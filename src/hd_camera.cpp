#include "hd_internal.h"

static VECTORF hd_camera_transformed(const AnimationTrack *track, const MATRIX *matrix, float frame)
{
    VECTORF value = hd_animation_vector(track, frame);
    if (matrix != NULL)
    {
        VECTORF transformed = {
            (matrix->m[0][0] * value.vx + matrix->m[0][1] * value.vy + matrix->m[0][2] * value.vz) / 4096.0f + matrix->t[0] / 32.0f,
            (matrix->m[1][0] * value.vx + matrix->m[1][1] * value.vy + matrix->m[1][2] * value.vz) / 4096.0f + matrix->t[1] / 32.0f,
            (matrix->m[2][0] * value.vx + matrix->m[2][1] * value.vy + matrix->m[2][2] * value.vz) / 4096.0f + matrix->t[2] / 32.0f};
        value = transformed;
    }
    return value;
}

void hd_camera_sample(const AnimationTrack *track, const MATRIX *matrix, float frame, const Vec3 *dst)
{
    uint32 slot;
    if (dst == &camera_motion.pos)
        slot = 0;
    else if (dst == &camera_motion.focus_pos)
        slot = 1;
    else
        return;
    VECTORF value = hd_camera_transformed(track, matrix, frame);
    hd_state.precise_camera.source[slot] = *dst;
    hd_state.precise_camera.value[slot] = value;
    hd_state.precise_camera.valid[slot] = true;
    hd_state.precise_camera.aim_valid = false;
}

void hd_camera_path(const ScenePath *path, float frame, uint32 dir)
{
    float pos_frame = (path->node.state_flags & (dir | 8)) == 0 ? path->definition->pos_track.frame_count - frame - 1.0f : frame;
    float focus_frame = dir == 0 ? path->definition->pos_track.frame_count - frame - 1.0f : frame;
    hd_camera_sample(&path->offset_track, path->offset_matrix, pos_frame, &camera_motion.pos);
    hd_camera_sample(&path->definition->pos_track, path->definition->transform != NULL ? &path->definition->transform->matrix : NULL,
        focus_frame, &camera_motion.focus_pos);
}

static VECTORF hd_camera_point(uint32 slot, const Vec3 *source)
{
    if (hd_state.precise_camera.valid[slot] && memcmp(source, hd_state.precise_camera.source + slot, sizeof(*source)) == 0)
        return hd_state.precise_camera.value[slot];
    VECTORF value = {source->x / 32.0f, source->y / 32.0f, source->z / 32.0f};
    return value;
}

static VECTORF hd_camera_path_point(const ScenePath *path, float frame, uint32 dir, bool offset)
{
    if (offset)
    {
        if ((path->node.state_flags & (dir | 8)) == 0)
            frame = path->definition->pos_track.frame_count - frame - 1.0f;
        return hd_camera_transformed(&path->offset_track, path->offset_matrix, frame);
    }
    if (dir == 0)
        frame = path->definition->pos_track.frame_count - frame - 1.0f;
    return hd_camera_transformed(&path->definition->pos_track,
        path->definition->transform != NULL ? &path->definition->transform->matrix : NULL, frame);
}

void hd_camera_project(const ScenePath *path, sint32 frame, uint32 dir, const Vec3 *dst)
{
    if (dst != &camera_motion.pos)
        return;
    float phase = (float)frame;
    sint32 count = path->definition->pos_track.frame_count;
    VECTORF focus = hd_camera_point(1, &camera_motion.focus_pos);
    VECTORF floor = hd_camera_path_point(path, fminf((float)(count - 1), fmaxf(0.0f, phase)), dir, false);
    if (frame > 0 && frame < count)
    {
        VECTORF prev = hd_camera_path_point(path, (float)(frame - 1), dir, false);
        VECTORF next = hd_camera_path_point(path, (float)frame, dir, false);
        float x = next.vx - prev.vx, z = next.vz - prev.vz;
        float sq = x * x + z * z;
        if (sq > 0.0f)
        {
            // Original path projection uses XZ; jump height must not move the camera along the path
            float fraction = ((focus.vx - prev.vx) * x + (focus.vz - prev.vz) * z) / sq;
            fraction = fminf(1.0f, fmaxf(0.0f, fraction));
            phase = frame - 1.0f + fraction;
            floor.vy = prev.vy + (next.vy - prev.vy) * fraction;
        }
    }
    else
        phase = frame <= 0 ? 0.0f : (float)(count - 1);
    VECTORF value = hd_camera_path_point(path, phase, dir, true);
    float gap = focus.vy - floor.vy;
    if ((path->node.state_flags & 1) != 0 && gap > 0)
        value.vy += fminf(gap * 1.25f, gap + 1920.0f);
    hd_state.precise_camera.source[0] = *dst;
    hd_state.precise_camera.value[0] = value;
    hd_state.precise_camera.valid[0] = true;
    hd_state.precise_camera.aim_valid = false;
}

void hd_camera_aim(void)
{
    VECTORF position = hd_camera_point(0, &camera_motion.pos);
    VECTORF focus = hd_camera_point(1, &camera_motion.focus_pos);
    float x = focus.vx - position.vx, y = focus.vy - position.vy, z = focus.vz - position.vz;
    float horizontal = sqrtf(x * x + z * z);
    if (horizontal == 0.0f && y == 0.0f)
    {
        hd_state.precise_camera.aim_valid = false;
        return;
    }
    hd_state.precise_camera.angle[0] = -atan2f(y, horizontal) / RAD12;
    hd_state.precise_camera.angle[1] = horizontal != 0.0f ? atan2f(x, z) / RAD12 : (float)camera_proj.rotation[1];
    hd_state.precise_camera.source_angle[0] = camera_proj.rotation[0];
    hd_state.precise_camera.source_angle[1] = camera_proj.rotation[1];
    hd_state.precise_camera.aim_source[0] = camera_motion.pos;
    hd_state.precise_camera.aim_source[1] = camera_motion.focus_pos;
    hd_state.precise_camera.aim_valid = true;
}

bool hd_camera_aim_valid(void)
{
    return hd_state.precise_camera.aim_valid
        && camera_proj.rotation[0] == hd_state.precise_camera.source_angle[0]
        && camera_proj.rotation[1] == hd_state.precise_camera.source_angle[1]
        && memcmp(&camera_motion.pos, hd_state.precise_camera.aim_source, sizeof(Vec3)) == 0
        && memcmp(&camera_motion.focus_pos, hd_state.precise_camera.aim_source + 1, sizeof(Vec3)) == 0;
}

static bool hd_camera_float(const VECTORF *position, const float *rotation, const CameraProjection *projection, float near_z, HdCamera *camera)
{
    float focal = projection->scale / 4096.0f;
    float far_z = projection->far_distance / 32.0f;
    if (near_z <= 0.0f || far_z <= near_z || focal <= 0.0f)
        return false;
    MATRIXF x_basis, y_basis;
    IdentityMatrixf(&x_basis);
    IdentityMatrixf(&y_basis);
    RotMatrixXf(-rotation[0] * RAD12, &x_basis);
    RotMatrixYf(-rotation[1] * RAD12, &y_basis);
    MulMatrix0f(&x_basis, &y_basis, &camera->view);
    RotMatrixZf((2048.0f - rotation[2]) * RAD12, &camera->view);
    // World positions use 32 units per mesh coordinate; rotation coefficients do not scale translation
    VECTORF rotated;
    ApplyMatrixf(&camera->view, position, &rotated);
    camera->view.t[0] = -rotated.vx;
    camera->view.t[1] = -rotated.vy;
    camera->view.t[2] = -rotated.vz;
    camera->focal = focal;
    camera->near_z = near_z;
    camera->far_z = far_z;
    return true;
}

bool hd_camera(const Vec3 *pos, const CameraProjection *projection, float near_z, HdCamera *camera)
{
    VECTORF position = {pos->x / 32.0f, pos->y / 32.0f, pos->z / 32.0f};
    float rotation[3] = {(float)projection->rotation[0], (float)projection->rotation[1], (float)projection->rotation[2]};
    return hd_camera_float(&position, rotation, projection, near_z, camera);
}

bool hd_visible(const HdCamera *camera, const VECTORF *pos, float radius, float aspect)
{
    if (aspect <= 0.0f || radius < 0.0f)
        return false;
    VECTORF point;
    RotTransf(&camera->view, pos, &point);
    if (point.vz + radius < camera->near_z || point.vz - radius > camera->far_z)
        return false;
    float y_focal = camera->focal / 120.0f;
    float x_focal = y_focal / aspect;
    float x_extent = radius * sqrtf(x_focal * x_focal + 1.0f);
    float y_extent = radius * sqrtf(y_focal * y_focal + 1.0f);
    return fabsf(point.vx * x_focal) <= point.vz + x_extent && fabsf(point.vy * y_focal) <= point.vz + y_extent;
}

void hd_camera_cut(void)
{
    ++hd_state.camera_epoch;
    memset(&hd_state.precise_camera, 0, sizeof(hd_state.precise_camera));
}

float hd_angle_delta(float from, float to)
{
    float angle = fmodf(to - from + 2048.0f, 4096.0f);
    if (angle < 0.0f)
        angle += 4096.0f;
    return angle - 2048.0f;
}

HdCamera hd_mix_camera(const HdCamera *previous, const HdCamera *current, float fract)
{
    if (fract <= 0.0f)
        return *previous;
    if (fract >= 1.0f)
        return *current;
    HdCamera camera = *current;
    MATRIXF old_world, new_world, world;
    TransposeMatrixf(&previous->view, &old_world);
    TransposeMatrixf(&current->view, &new_world);
    QUATF old_rotation, new_rotation, rotation;
    MatrixQuatf(&old_world, &old_rotation);
    MatrixQuatf(&new_world, &new_rotation);
    SlerpQuatf(&old_rotation, &new_rotation, fract, &rotation);
    QuatMatrixf(&rotation, &world);
    TransposeMatrixf(&world, &camera.view);
    VECTORF old_pos = {-previous->view.t[0], -previous->view.t[1], -previous->view.t[2]};
    VECTORF new_pos = {-current->view.t[0], -current->view.t[1], -current->view.t[2]};
    ApplyMatrixf(&old_world, &old_pos, &old_pos);
    ApplyMatrixf(&new_world, &new_pos, &new_pos);
    VECTORF pos = {old_pos.vx + (new_pos.vx - old_pos.vx) * fract,
        old_pos.vy + (new_pos.vy - old_pos.vy) * fract, old_pos.vz + (new_pos.vz - old_pos.vz) * fract};
    ApplyMatrixf(&camera.view, &pos, &pos);
    camera.view.t[0] = -pos.vx;
    camera.view.t[1] = -pos.vy;
    camera.view.t[2] = -pos.vz;
    return camera;
}

void hd_frame_camera(const Vec3 *pos, const CameraProjection *projection, const SceneSection *scene)
{
    if (!hd_frame.collecting || hd_frame.failed)
        return;
    if (pos == &camera_motion.pos && projection == &camera_proj)
    {
        VECTORF position = hd_camera_point(0, pos);
        float rotation[3] = {(float)projection->rotation[0], (float)projection->rotation[1], (float)projection->rotation[2]};
        if (hd_camera_aim_valid())
        {
            rotation[0] = hd_state.precise_camera.angle[0];
            rotation[1] = hd_state.precise_camera.angle[1];
        }
        hd_frame.camera_valid = hd_camera_float(&position, rotation, projection, 1.0f, &hd_frame.camera);
    }
    else
        hd_frame.camera_valid = hd_camera(pos, projection, 1.0f, &hd_frame.camera);
    hd_frame.aspect = hd_state.ui_half_width / 120.0f;
    if (hd_frame.camera_valid)
        hd_source_clip(scene, &hd_frame.camera);
    hd_state.camera_sample_epoch = hd_state.camera_epoch;
    hd_state.scene = scene;
    if (!hd_frame.camera_valid)
        hd_frame.failed = true;
}
