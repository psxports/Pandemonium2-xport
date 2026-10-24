#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "animated_object.h"
#include "game.h"
#include "actor_model_registry.h"
#include "camera_projection.h"
#include "collision.h"
#include "global.h"
#include "psx_gte.h"
#include "hd.h"

static const sint8 action_start_mode = 1;

static const sint32 actor_target_height_tolerance = 8192;

static const sint32 actor_target_height_limit = 61440;

static const uint32 actor_path_height_tolerance = 8192;

static const sint8 actor_path_height_enabled = 1;

static const uint8 actor_target_avoidance_mode = 0;

static const uint32 actor_action_end_margin = 2;

static const sint8 actor_turnaround_enabled = 1;

static const sint8 actor_launch_angle_selector = -1;

AnimatedObject *character_action_owner = 0;

AnimatedObject *character_action_target = 0;

static const sint8 hold_interaction_timeout = 1;

static const sint8 raise_completed_target = 1;

static const sint8 pause_interaction_actions = 0;

static const sint8 pickup_scores[3] = {1, 5, 25};

static const uint32 min_impulse_vertical_speed = 4096;

static const uint32 support_search_limit = 20;

struct FallbackWorkspace
{
    Vec3 scratch;
    const ScenePath *chains[3][10];
    Vec3 probe;
    const ScenePath *candidates[5];
    const ScenePath *alternate[4];
    Vec3 displaced;
    const ScenePath *ranked[10];
    uint32 costs[10];
    uint32 frames[10];
    Vec3 endpoint;
};

struct FallbackDefaults
{
    const ScenePath *chains[10];
    const ScenePath *candidates[5];
    const ScenePath *alternate[4];
    const ScenePath *ranked[9];
    uint32 costs[9];
};

static const FallbackDefaults fallback_defaults = {{0}, {0}, {0}, {0}, {0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF}};

void AnimatedObject::DrawGeometry(const MorphSpan *span) const
{
    const SceneSection *section;
    const MeshVertex *vertices_val, *vertex;
    ProjectedVertex *proj;
    sint32 polygon_count;
    RenderMatrixSlot *matrix;
    MATRIX *cur_matrix, *output_matrix;
    const MATRIX *parent_matrix;
    uint32 point;
    uint32 remaining;
    const MeshFace *polygon;
    MeshLighting *lighting;
    uint32 idx;
    uint32 val;
    uint32 x;
    uint32 y;
    uint32 z;
    uint32 cur_flags;
    FUNCTION_MARKER(0x80011980, "SLUS_005.78");
    polygon_count = span->polygon_count;
    if (polygon_count == 0)
        return;
    hd_frame_actor(this, span, &scene_lighting, render_view.material_override, render_view.lighting_enabled ? 0.0f : render_view.object_depth_bias,
        !render_view.lighting_enabled && (game_flow.transition_flags & 0x20) != 0, render_view.fog_color);
    if (hd_frame.active)
    {
        SetRotMatrix(&render_view.matrix_cursor->matrix);
        return;
    }
    matrix = render_view.matrix_cursor;
    cur_matrix = &matrix->matrix;
    {
        RenderMatrixSlot *reserve_slot = matrix;
        uint32 reserve_idx;
        for (reserve_idx = 0; reserve_idx < 3; ++reserve_idx)
        {
            if (reserve_slot->next == 0)
            {
                RenderMatrixSlot *next_slot = (RenderMatrixSlot *)malloc(sizeof(RenderMatrixSlot));
                uint32 default_offset, default_bytes;
                memset(next_slot, 0, sizeof(RenderMatrixSlot));
                next_slot->prev = reserve_slot;
                next_slot->depth = reserve_slot->depth + 1;
                default_offset = OriginalDefaults::RenderMatrixCursorOffset - 0x1F800000u + next_slot->depth * 32;
                if (default_offset < sizeof(OriginalDefaults::scratchpad_startup))
                {
                    default_bytes = sizeof(OriginalDefaults::scratchpad_startup) - default_offset;
                    if (default_bytes > sizeof(MATRIX))
                        default_bytes = sizeof(MATRIX);
                    CopyBytes((uint8 *)&next_slot->matrix, OriginalDefaults::scratchpad_startup + default_offset, default_bytes);
                }
                reserve_slot->next = next_slot;
            }
            reserve_slot = reserve_slot->next;
        }
    }
    render_view.matrix_cursor = matrix->next;
    SetRotMatrix(&render_view.camera_transform);
    output_matrix = &render_view.matrix_cursor->matrix;
    {
        VECTOR rotation_input = {cur_matrix->t[0], cur_matrix->t[1], cur_matrix->t[2], 0};
        VECTOR rotation_output;
        ApplyRotMatrixLV(&rotation_input, &rotation_output);
        output_matrix->t[0] = rotation_output.vx;
        output_matrix->t[1] = rotation_output.vy;
        output_matrix->t[2] = rotation_output.vz;
    }
    matrix = render_view.matrix_cursor;
    cur_matrix = &matrix->matrix;
    val = render_view.camera_transform.t[0];
    x = cur_matrix->t[0];
    cur_matrix->t[0] = x + val;
    y = cur_matrix->t[1];
    val = render_view.camera_transform.t[1];
    cur_matrix->t[1] = y + val;
    z = cur_matrix->t[2];
    val = render_view.camera_transform.t[2];
    cur_matrix->t[2] = z + val;
    parent_matrix = matrix->prev != 0 ? &matrix->prev->matrix : 0;
    MulRotMatrix0(parent_matrix, cur_matrix);
    SetRotMatrix(cur_matrix);
    matrix = render_view.matrix_cursor;
    cur_matrix = &matrix->matrix;
    x = cur_matrix->t[0];
    y = cur_matrix->t[1];
    z = cur_matrix->t[2];
    xport_gte_write_control(5, ((sint32)x >> 5));
    xport_gte_write_control(6, ((sint32)y >> 5));
    xport_gte_write_control(7, ((sint32)z >> 5));
    remaining = span->vertex_count;
    point = span->first_vertex;
    vertices_val = vertices + point;
    vertex = vertices_val;
    proj = render_view.proj_vertices;
    while ((sint32)(remaining - 3) >= 0)
    {
        gte_ldv0(&vertex[0].pos);
        gte_ldv1(&vertex[1].pos);
        gte_ldv2(&vertex[2].pos);
        xport_gte_execute(0x280030);
        proj[0].xy = xport_gte_read_data(12);
        proj[1].xy = xport_gte_read_data(13);
        proj[2].xy = xport_gte_read_data(14);
        proj[0].depth = xport_gte_read_data(17);
        proj[1].depth = xport_gte_read_data(18);
        proj[2].depth = xport_gte_read_data(19);
        vertex += 3;
        remaining -= 3;
        proj += 3;
    }
    while (remaining != 0)
    {
        gte_ldv0(&vertex[0].pos);
        xport_gte_execute(0x180001);
        proj[0].xy = xport_gte_read_data(14);
        proj[0].depth = xport_gte_read_data(19);
        ++vertex;
        ++proj;
        --remaining;
    }
    point = (uint16)span->first_vertex;
    render_view.first_proj_vertex = point;
    cur_flags = game_flow.transition_flags;
    matrix = render_view.matrix_cursor;
    cur_matrix = &matrix->matrix;
    for (idx = 0; idx < 8; ++idx)
        render_view.inverse_transform.m[idx / 3][idx % 3] = cur_matrix->m[idx % 3][idx / 3];
    val = (uint16)cur_matrix->m[2][2];
    game_flow.transition_flags = cur_flags & 0xFFFFFFFE;
    render_view.inverse_transform.m[2][2] = val;
    SetRotMatrix(&render_view.inverse_transform);
    {
        VECTOR rotation_input = {cur_matrix->t[0], cur_matrix->t[1], cur_matrix->t[2], 0};
        VECTOR rotation_output;
        ApplyRotMatrixLV(&rotation_input, &rotation_output);
        render_view.inverse_transform.t[0] = rotation_output.vx;
        render_view.inverse_transform.t[1] = rotation_output.vy;
        render_view.inverse_transform.t[2] = rotation_output.vz;
    }
    x = render_view.inverse_transform.t[0];
    y = render_view.inverse_transform.t[1];
    z = render_view.inverse_transform.t[2];
    render_view.inverse_transform.t[0] = -x;
    render_view.inverse_transform.t[1] = -y;
    render_view.inverse_transform.t[2] = -z;
    if (render_view.lighting_enabled)
    {
        section = scene_stream.active_section;
        val = lighting_idx;
        point = span->first_polygon;
        lighting = section->lighting_recs + val / 4 + point;
        render_view.lighting_cache = lighting;
        if ((flags & 1) != 0)
        {
            point = span->first_polygon;
            polygon = polygons + point;
            MeshGeometryNode::DrawFacesWithFreshLighting(vertices_val, polygon, polygon_count);
        }
        else
        {
            point = span->first_polygon;
            polygon = polygons + point;
            MeshGeometryNode::DrawFacesWithCachedLighting(vertices_val, polygon, polygon_count);
        }
    }
    else
    {
        cur_flags = game_flow.transition_flags;
        point = span->first_polygon;
        polygon = polygons + point;
        if ((cur_flags & 0x20) != 0)
            MeshGeometryNode::DrawUniformColorFaces(polygon, polygon_count);
        else
            MeshGeometryNode::DrawFacesWithoutLightingCache(vertices_val, polygon, polygon_count);
    }
    matrix = render_view.matrix_cursor->prev;
    cur_matrix = &matrix->matrix;
    render_view.matrix_cursor = matrix;
    SetRotMatrix(cur_matrix);
}

void AnimatedObject::UpdateObjectBounds()
{
    CollisionSphere *spheres_val = spheres;
    CollisionSphere *sphere;
    uint32 cur_flags;
    const MATRIX *matrix;
    uint32 x;
    uint32 y;
    uint32 z;
    uint32 val;
    uint32 dx;
    uint32 dy;
    uint32 dz;
    uint32 idx;
    uint32 result;
    FUNCTION_MARKER(0x80012298, "SLUS_005.78");
    cur_flags = flags;
    if ((cur_flags & 0x20) != 0 && contact_kind != 0)
    {
        if ((cur_flags & 0x20200) != 0x200)
        {
            spheres_val[3].radius = 0xFFFFFFFF;
            spheres_val[2].radius = 0xFFFFFFFF;
            spheres_val[1].radius = 0xFFFFFFFF;
            spheres_val[0].radius = 0xFFFFFFFF;
            return;
        }
        matrix = &render_view.matrix_cursor->matrix;
        x = world_pos.x;
        val = matrix->t[0];
        dx = val - x;
        val = matrix->t[1];
        y = world_pos.y;
        dy = val - y;
        val = matrix->t[2];
        z = world_pos.z;
        dz = val - z;
        dy += bounds.height;
        world_pos.x = world_pos.x + dx;
        world_pos.y = world_pos.y + dy;
        world_pos.z = world_pos.z + dz;
        sphere = spheres_val;
        idx = 0;
        for (;;)
        {
            result = sphere->radius;
            ++idx;
            if ((sint32)result <= 0)
                return;
            sphere->pos.x = sphere->pos.x + dx;
            sphere->pos.y = sphere->pos.y + dy;
            sphere->pos.z = sphere->pos.z + dz;
            result = idx < 4;
            ++sphere;
            if (result == 0)
                return;
        }
    }
    matrix = &render_view.matrix_cursor->matrix;
    x = spheres_val[0].pos.x;
    val = matrix->t[0];
    world_pos.x = x + val;
    y = spheres_val[0].pos.y;
    val = matrix->t[1];
    world_pos.y = y + val;
    z = spheres_val[0].pos.z;
    val = matrix->t[2];
    world_pos.z = z + val;
}

sint16 AnimatedObject::ComputeObjectFacing(const Vec3 &input_dir_vector, Quat *axes) const
{
    // Q27 unity becomes Q12 after the coefficient shift
    const Vec3 *dir_vector = &input_dir_vector;
    FUNCTION_MARKER(0x80018144, "SLUS_005.78");
    Quat local_quat, parent_quat, composed;
    Quat *quat = axes != 0 ? axes : &local_quat;
    const Quat *src;

    Vec3 t;

    MATRIX matrix = {0};

    const PathDefinition *definition;
    const ActorPlacement *placement;
    const ActorBinding *binding;
    const MATRIX *parent_matrix;
    const PathTransform *matrix_transform;
    const AnimationTrack *track;
    sint32 val;
    uint32 result;
    uint32 xx, yy, zz, xy, xz, yz, wx, wy, wz, x, y, z;
    sint32 frame, q0, q1, q2, q3, p0, p1, p2, p3;
    definition = path_definition;
    if (definition != 0 && (definition->flags & 8) != 0)
    {
        placement = (const ActorPlacement *)definition;
        binding = (const ActorBinding *)placement->state;
        track = binding->rotation_track;
        if (track != 0)
        {
            if ((flags & 0x10) != 0)
                frame = path_frame;
            else
            {
                frame = definition->pos_track.frame_count;
                frame = (uint32)frame - path_frame - 1;
            }
            AnimationTrack::SampleQuatTrack(frame, track, *quat);
            matrix_transform = path_definition->transform;
            parent_matrix = matrix_transform != 0 ? &matrix_transform->matrix : 0;
            if (parent_matrix != 0)
            {
                Quat::MatrixToQuat(parent_matrix, &parent_quat);
                q0 = quat->w;
                src = 0;
                if (q0 == 0x4000)
                    src = &parent_quat;
                else
                {
                    p0 = parent_quat.w;
                    if (p0 == 0x4000)
                        src = quat;
                    else
                    {
                        q1 = quat->x;
                        p1 = parent_quat.x;
                        q2 = quat->y;
                        p2 = parent_quat.y;
                        q3 = quat->z;
                        p3 = parent_quat.z;
                        x = (q0 * p0) - ((q1 * p1) + (uint32)(q2 * p2) + (q3 * p3));
                        y = (q0 * p1) + (uint32)(p0 * q1) + (q2 * p3) - (q3 * p2);
                        z = (q0 * p2) + (uint32)(p0 * q2) + (q3 * p1) - (q1 * p3);
                        val = (q0 * p3) + (uint32)(p0 * q3) + (q1 * p2) - (q2 * p1);
                        composed.w = x >> 14;
                        composed.x = y >> 14;
                        composed.y = z >> 14;
                        composed.z = val >> 14;
                    }
                }
                if (src != 0)
                {
                    composed = *src;
                }
                *quat = composed;
            }
            q1 = quat->x;
            q2 = quat->y;
            q3 = quat->z;
            q0 = quat->w;
            xx = q1 * q1;
            yy = q2 * q2;
            zz = q3 * q3;
            xy = q1 * q2;
            xz = q1 * q3;
            yz = q2 * q3;
            wz = q0 * q3;
            wy = q0 * q2;
            wx = q0 * q1;
            matrix.m[0][0] = (sint32)((1u << 27) - yy - zz) >> 15;
            matrix.m[1][1] = (sint32)((1u << 27) - xx - zz) >> 15;
            matrix.m[2][2] = (sint32)((1u << 27) - xx - yy) >> 15;
            matrix.m[0][1] = (sint32)(xy - wz) >> 15;
            matrix.m[1][0] = (sint32)(xy + wz) >> 15;
            matrix.m[0][2] = (sint32)(xz + wy) >> 15;
            matrix.m[2][0] = (sint32)(xz - wy) >> 15;
            matrix.m[1][2] = (sint32)(yz - wx) >> 15;
            matrix.m[2][1] = (sint32)(yz + wx) >> 15;
            PushMatrix();
            SetRotMatrix(&matrix);
            {
                VECTOR rotation_input = {dir_vector->x, dir_vector->y, dir_vector->z, 0};
                {
                    VECTOR sdk_output;
                    ApplyRotMatrixLV(&rotation_input, &sdk_output);
                    t.x = sdk_output.vx;
                    t.y = sdk_output.vy;
                    t.z = sdk_output.vz;
                }
            }
            PopMatrix();
            dir_vector = &t;
        }
    }
    if ((uint32)dir_vector->x + 32 >= 65 || (uint32)dir_vector->z + 32 >= 65)
    {
        z = dir_vector->z;
        x = dir_vector->x;
        result = Atan2Angle((-z), (-x));
    }
    else
        result = 4096;
    return result;
}

sint16 AnimatedObject::ComputeObjectPitch(const Vec3 &dir, Quat *axes) const
{
    // Q27 unity becomes Q12 after the coefficient shift
    const Vec3 *dir_vector = &dir;
    FUNCTION_MARKER(0x800185C8, "SLUS_005.78");
    Quat local_quat, parent_quat, composed;
    Quat *quat = axes != 0 ? axes : &local_quat;
    const Quat *src;

    Vec3 t;

    MATRIX matrix = {0};

    const PathDefinition *definition;
    const ActorPlacement *placement;
    const ActorBinding *binding;
    const MATRIX *parent_matrix;
    const PathTransform *matrix_transform;
    const AnimationTrack *track;
    sint32 val;
    uint32 result;
    uint32 xx, yy, zz, xy, xz, yz, wx, wy, wz, x, y, z;
    uint32 rotated, sq, mag, height;
    sint32 frame, q0, q1, q2, q3, p0, p1, p2, p3;
    rotated = 0;
    definition = path_definition;
    if (definition != 0 && (definition->flags & 8) != 0)
    {
        placement = (const ActorPlacement *)definition;
        binding = (const ActorBinding *)placement->state;
        track = binding->rotation_track;
        if (track != 0)
        {
            if ((flags & 0x10) != 0)
                frame = path_frame;
            else
            {
                frame = definition->pos_track.frame_count;
                frame = (uint32)frame - path_frame - 1;
            }
            AnimationTrack::SampleQuatTrack(frame, track, *quat);
            matrix_transform = path_definition->transform;
            parent_matrix = matrix_transform != 0 ? &matrix_transform->matrix : 0;
            if (parent_matrix != 0)
            {
                Quat::MatrixToQuat(parent_matrix, &parent_quat);
                q0 = quat->w;
                src = 0;
                if (q0 == 0x4000)
                    src = &parent_quat;
                else
                {
                    p0 = parent_quat.w;
                    if (p0 == 0x4000)
                        src = quat;
                    else
                    {
                        q1 = quat->x;
                        p1 = parent_quat.x;
                        q2 = quat->y;
                        p2 = parent_quat.y;
                        q3 = quat->z;
                        p3 = parent_quat.z;
                        x = (q0 * p0) - ((q1 * p1) + (uint32)(q2 * p2) + (q3 * p3));
                        y = (q0 * p1) + (uint32)(p0 * q1) + (q2 * p3) - (q3 * p2);
                        z = (q0 * p2) + (uint32)(p0 * q2) + (q3 * p1) - (q1 * p3);
                        val = (q0 * p3) + (uint32)(p0 * q3) + (q1 * p2) - (q2 * p1);
                        composed.w = x >> 14;
                        composed.x = y >> 14;
                        composed.y = z >> 14;
                        composed.z = val >> 14;
                    }
                }
                if (src != 0)
                {
                    composed = *src;
                }
                *quat = composed;
            }
            q1 = quat->x;
            q2 = quat->y;
            q3 = quat->z;
            q0 = quat->w;
            xx = q1 * q1;
            yy = q2 * q2;
            zz = q3 * q3;
            xy = q1 * q2;
            xz = q1 * q3;
            yz = q2 * q3;
            wz = q0 * q3;
            wy = q0 * q2;
            wx = q0 * q1;
            matrix.m[0][0] = (sint32)((1u << 27) - yy - zz) >> 15;
            matrix.m[1][1] = (sint32)((1u << 27) - xx - zz) >> 15;
            matrix.m[2][2] = (sint32)((1u << 27) - xx - yy) >> 15;
            matrix.m[0][1] = (sint32)(xy - wz) >> 15;
            matrix.m[1][0] = (sint32)(xy + wz) >> 15;
            matrix.m[0][2] = (sint32)(xz + wy) >> 15;
            matrix.m[2][0] = (sint32)(xz - wy) >> 15;
            matrix.m[1][2] = (sint32)(yz - wx) >> 15;
            matrix.m[2][1] = (sint32)(yz + wx) >> 15;
            PushMatrix();
            SetRotMatrix(&matrix);
            {
                VECTOR rotation_input = {dir_vector->x, dir_vector->y, dir_vector->z, 0};
                {
                    VECTOR sdk_output;
                    ApplyRotMatrixLV(&rotation_input, &sdk_output);
                    t.x = sdk_output.vx;
                    t.y = sdk_output.vy;
                    t.z = sdk_output.vz;
                }
            }
            PopMatrix();
            dir_vector = &t;
            rotated = 1;
        }
    }
    if (rotated != 0)
    {
        x = dir_vector->x >> 6;
        y = dir_vector->y >> 6;
        z = dir_vector->z >> 6;
    }
    else
    {
        height = dir_vector->y;
        if (height + 32 < 65)
        {
            return 4096;
        }
        x = dir_vector->x >> 6;
        y = (sint32)height >> 6;
        z = dir_vector->z >> 6;
    }
    sq = x * x + y * y + z * z;
    if ((sint32)sq >= 0)
        mag = SquareRoot12(sq);
    else
        mag = SquareRoot12((sq >> 2)) << 1;
    if (rotated == 0)
    {
        height = dir_vector->y;
        if ((sint32)height < 0)
            height = -height;
        if ((sint32)mag < (sint32)height)
            mag = height;
    }
    height = dir_vector->y;
    result = AsinAngle(height, mag);
    if ((sint16)result != 4096)
        result = (-result) & 0xFFF;
    return result;
}

sint16 AnimatedObject::SampleObjectPitch(const Vec3 &dir_vector, Quat *quat) const
{
    uint32 dx, dy, dz;


    // Q27 unity becomes Q12 after the coefficient shift
    FUNCTION_MARKER(0x80018B40, "SLUS_005.78");
    Quat local_quat, object_rotation;

    MATRIX matrix = {0};

    Vec3 rotated;

    const ActorPlacement *placement;
    const ActorBinding *binding;
    const MATRIX *parent_matrix;
    const PathTransform *matrix_transform;
    const AnimationTrack *track;
    sint32 frame;
    uint32 sq = 0;
    uint32 len;
    sint32 angle;
    sint16 result;

    if (quat == 0)
        quat = &local_quat;
    if ((flags & 16) != 0)
    {
        placement = (const ActorPlacement *)path_definition;
        binding = (const ActorBinding *)placement->state;
        track = binding->rotation_track;
        frame = path_frame;
    }
    else
    {
        placement = (const ActorPlacement *)path_definition;
        binding = (const ActorBinding *)placement->state;
        track = binding->rotation_track;
        frame = placement->path.pos_track.frame_count - path_frame - 1;
    }
    AnimationTrack::SampleQuatTrack(frame, track, *quat);
    matrix_transform = path_definition->transform;
    parent_matrix = matrix_transform != 0 ? &matrix_transform->matrix : 0;
    if (parent_matrix != 0)
    {
        Quat::MatrixToQuat(parent_matrix, &object_rotation);
        if (quat->w == 16384)
            *quat = object_rotation;
        else if (object_rotation.w != 16384)
        {
            uint32 w = quat->w;
            uint32 x = quat->x;
            uint32 y = quat->y;
            uint32 z = quat->z;
            uint32 mw = object_rotation.w;
            uint32 mx = object_rotation.x;
            uint32 my = object_rotation.y;
            uint32 mz = object_rotation.z;
            uint32 combined_w = (w * mw - (x * mx + y * my + z * mz)) >> 14;
            uint32 combined_x = (w * mx + mw * x + y * mz - z * my) >> 14;
            uint32 combined_y = (w * my + mw * y + z * mx - x * mz) >> 14;
            uint32 combined_z = (sint32)(w * mz + mw * z + x * my - y * mx) >> 14;
            quat->w = combined_w;
            quat->x = combined_x;
            quat->y = combined_y;
            quat->z = combined_z;
        }
    }
    {
        uint32 x = quat->x;
        uint32 y = quat->y;
        uint32 z = quat->z;
        uint32 w = quat->w;
        uint32 xx = x * x;
        uint32 yy = y * y;
        uint32 zz = z * z;
        uint32 xy = x * y;
        uint32 xz = x * z;
        uint32 yz = y * z;
        uint32 wx = w * x;
        uint32 wy = w * y;
        uint32 wz = w * z;
        matrix.m[0][0] = (sint32)((1u << 27) - (yy + zz)) >> 15;
        matrix.m[1][1] = (sint32)((1u << 27) - (xx + zz)) >> 15;
        matrix.m[2][2] = (sint32)((1u << 27) - (xx + yy)) >> 15;
        matrix.m[0][1] = (sint32)(xy - wz) >> 15;
        matrix.m[1][0] = (sint32)(xy + wz) >> 15;
        matrix.m[0][2] = (sint32)(xz + wy) >> 15;
        matrix.m[2][0] = (sint32)(xz - wy) >> 15;
        matrix.m[1][2] = (sint32)(yz - wx) >> 15;
        matrix.m[2][1] = (sint32)(yz + wx) >> 15;
    }
    PushMatrix();
    {
        SetRotMatrix(&matrix);
    }
    {
        VECTOR rotation_input = {dir_vector.x, dir_vector.y, dir_vector.z, 0};
        {
            VECTOR sdk_output;
            ApplyRotMatrixLV(&rotation_input, &sdk_output);
            rotated.x = sdk_output.vx;
            rotated.y = sdk_output.vy;
            rotated.z = sdk_output.vz;
        }
    }
    PopMatrix();
    dx = rotated.x >> 6;
    dy = rotated.y >> 6;
    dz = rotated.z >> 6;
    sq += dx * dx + dy * dy + dz * dz;
    if ((sint32)sq >= 0)
        len = SquareRoot12(sq);
    else
        len = SquareRoot12((sq >> 2)) * 2;
    angle = AsinAngle(rotated.y, len);
    result = (sint16)angle == 4096 ? (uint32)angle : (-(uint32)angle) & 4095;
    return result;
}

void AnimatedObject::RebuildMorphNormals(const MorphSpan *span) const
{
    FUNCTION_MARKER(0x80019260, "SLUS_005.78");
    uint32 first_vertex = span->first_vertex;
    MeshVertex *vertices_val;
    MeshFace *first_polygon = polygons + span->first_polygon;
    MeshFace *polygon;
    uint32 remaining = span->polygon_count;
    Vec3 edge0;
    Vec3 edge1;
    Vec3 n;
    MeshVertex *vertex0, *vertex1, *vertex2, *vertex3;
    uint32 dot, plane, i;
    MATRIX *parent_matrix;
    vertices_val = vertices + first_vertex;
    if (remaining != 0)
    {
        polygon = first_polygon;
        do
        {
            vertex0 = vertices_val + (polygon->vertex_idxs[0] - first_vertex);
            vertex1 = vertices_val + (polygon->vertex_idxs[1] - first_vertex);
            vertex3 = vertices_val + (polygon->vertex_idxs[3] - first_vertex);
            vertex2 = vertices_val + (polygon->vertex_idxs[2] - first_vertex);
            edge0.x = vertex2->pos.vx - (uint32)vertex0->pos.vx;
            edge0.y = vertex2->pos.vy - (uint32)vertex0->pos.vy;
            edge0.z = vertex2->pos.vz - (uint32)vertex0->pos.vz;
            edge1.x = vertex3->pos.vx - (uint32)vertex1->pos.vx;
            edge1.y = vertex3->pos.vy - (uint32)vertex1->pos.vy;
            edge1.z = vertex3->pos.vz - (uint32)vertex1->pos.vz;
            // Keep small morph faces nonzero before normalizing
            sint64 nx = (sint64)edge0.y * edge1.z - (sint64)edge0.z * edge1.y;
            sint64 ny = (sint64)edge0.z * edge1.x - (sint64)edge0.x * edge1.z;
            sint64 nz = (sint64)edge0.x * edge1.y - (sint64)edge0.y * edge1.x;
            while (nx < -26754 || nx > 26754 || ny < -26754 || ny > 26754 || nz < -26754 || nz > 26754)
            {
                nx >>= 1;
                ny >>= 1;
                nz >>= 1;
            }
            n.x = nx;
            n.y = ny;
            n.z = nz;
            {
                VECTOR n_input = {n.x, n.y, n.z, 0};
                SVECTOR n_output;
                VectorNormalS(&n_input, &n_output);
                polygon->n.x = n_output.vx;
                polygon->n.y = n_output.vy;
                polygon->n.z = n_output.vz;
            }
            dot = vertex0->pos.vx * polygon->n.x;
            dot += (uint32)(vertex0->pos.vy * polygon->n.y);
            dot += (uint32)(vertex0->pos.vz * polygon->n.z);
            plane = -(uint32)((sint32)dot >> 7);
            polygon->plane = plane;
            if ((polygon->tex->flags & 1) != 0)
                polygon->plane = plane + 0x3FFFF800;
            for (i = 0; i < 4; ++i)
            {
                MeshVertex *vertex = i == 0 ? vertex0 : i == 1 ? vertex1 : i == 2 ? vertex2 : vertex3;
                vertex->n.vx = (uint16)vertex->n.vx + (uint16)polygon->n.x;
                vertex->n.vy = (uint16)vertex->n.vy + (uint16)polygon->n.y;
                vertex->n.vz = (uint16)vertex->n.vz + (uint16)polygon->n.z;
            }
            ++polygon;
            --remaining;
        } while (remaining != 0);
    }
    remaining = span->vertex_count;
    if (remaining != 0)
    {
        MeshVertex *vertex = vertices_val;
        do
        {
            VectorNormalSS(&vertex->n, &vertex->n);
            ++vertex;
            --remaining;
        } while (remaining != 0);
    }
    parent_matrix = render_view.matrix_cursor->prev != 0 ? &render_view.matrix_cursor->prev->matrix : 0;
    {
        SetRotMatrix(parent_matrix);
    }
}

void AnimatedObject::SampleNodeAnimation(SceneNode *node)
{
    const AnimationHeader *header;
    Vec3 tmp_vector;
    AnimationHeader *clip;
    uint32 kind;
    MeshPartNode *part = (MeshPartNode *)node;
    SpriteRecord *spr = (SpriteRecord *)node;
    SphereNode *sphere_node = (SphereNode *)node;
    Vec3 *pos;
    Quat *rotation;
    sint32 pos_track;
    const sint8 *rotation_binding;
    const AnimationTrack *track;
    const Vec3 *src;
    const uint8 *payload;
    sint32 weight;
    uint32 x;
    uint32 y;

    sint32 product;
    uint32 cur_flags;
    uint32 active;
    MorphRestorationHeader *restoration;
    MorphRestorationEntry *entry;
    const MorphAnimationTrack *morph_track;
    uint32 slot_flags;
    uint32 remaining;
    uint32 points;
    MeshVertex *vertex;
    const SVECTOR *src_vertex;
    uint32 point;
    sint32 track_idx;
    sint32 count;
    sint32 shifted;
    sint32 sample_frame;
    sint32 node_key;
    FUNCTION_MARKER(0x80019CF4, "SLUS_005.78");
    hd_animation_begin(this, node);
    clip = animation;
    if (clip == 0)
        goto finish;
    header = clip;
    kind = node->kind;
    sample_frame = frame;
    if (kind == 5)
    {
        pos_track = part->animation_binding.tracks[0];
        pos = &part->pos;
    }
    else if (kind == 6)
    {
        pos_track = (sint8)sphere_node->node.state_flags;
        pos = &sphere_node->pos;
    }
    else if (kind == 7)
    {
        pos_track = spr->animation_binding.tracks[0];
        pos = &spr->pos;
    }
    else if (kind == 2)
    {
        SceneTrigger *trigger = (SceneTrigger *)node;
        pos_track = trigger->animation_binding.tracks[0];
        pos = &trigger->pos;
    }
    else
        goto finish;
    track_idx = pos_track;
    if (track_idx >= 0)
    {
        track = &header->tracks[0].vectors[track_idx].track;
        payload = track->keys;
        if (payload != 0)
        {
            cur_flags = flags;
            if ((cur_flags & 0x10000) != 0)
            {
                weight = 1365 * ((uint32)sample_frame + 1);
                src = payload == (const uint8 *)-1 ? &track->constant : &((const VectorAnimationKey *)payload)->pos;
                x = src->x;
                y = pos->x;
                product = (sint64)(sint32)(x - y) * weight;
                pos->x = y + (product >> 12);
                x = src->y;
                y = pos->y;
                product = (sint64)(sint32)(x - y) * weight;
                pos->y = y + (product >> 12);
                x = src->z;
                y = pos->z;
                product = (sint64)(sint32)(x - y) * weight;
                pos->z = y + (product >> 12);
            }
            else if (payload == (const uint8 *)-1)
            {
                *pos = track->constant;
            }
            else
                AnimationTrack::SampleVectorTrack(sample_frame, track, *pos);
        }
    }
    if (kind == 6 || kind == 7)
    {
        track_idx = kind == 6 ? sphere_node->animation_binding.tracks[0] : spr->animation_binding.tracks[2];
        if (track_idx >= 0)
        {
            track = header->tracks[2].recs + track_idx;
            payload = track->keys;
            if (payload != 0)
            {
                if (payload == (const uint8 *)-1)
                {
                    tmp_vector = track->constant;
                }
                else
                    AnimationTrack::SampleVectorTrack(sample_frame, track, tmp_vector);
                if (kind == 6)
                    sphere_node->radius = tmp_vector.x;
                else
                {
                    x = tmp_vector.x;
                    spr->extent[0] = (sint32)x >> 5;
                    y = tmp_vector.y;
                    spr->extent[1] = (sint32)y >> 5;
                }
            }
        }
        goto finish;
    }
    if (kind == 5)
    {
        rotation_binding = part->animation_binding.tracks + 1;
        rotation = &part->quat;
    }
    else
    {
        SceneTrigger *trigger = (SceneTrigger *)node;
        rotation_binding = trigger->animation_binding.tracks + 1;
        rotation = &trigger->quat;
    }
    track_idx = *rotation_binding;
    if (track_idx >= 0)
    {
        track = header->tracks[1].recs + track_idx;
        payload = track->keys;
        if (payload != 0)
        {
            shifted = gameplay_input.rotation_track_shift;
            if (shifted != 0)
            {
                shifted += track_idx;
                count = header->counts[1];
                if (count == 0)
                    xport_mips_break(7);
                track_idx = count == 0 ? shifted : shifted % count;
                track = header->tracks[1].recs + track_idx;
            }
            cur_flags = flags;
            if ((cur_flags & 0x10000) != 0)
                AnimationTrack::BlendRotationTransition(sample_frame, track, *rotation);
            else if (track->keys == (const uint8 *)-1)
            {
                *rotation = track->constant_quat;
            }
            else
                AnimationTrack::SampleQuatTrack(sample_frame, track, *rotation);
        }
    }
    if (kind != 5 || (flags & 0x20) == 0)
        goto finish;
    restoration = morph_restoration;
    if (restoration == 0)
        goto finish;
    active = header->counts[3];
    node_key = part->animation_binding.tracks[0];
    if (active != 0)
    {
        morph_track = header->tracks[3].morphs;
        for (;;)
        {
            if (morph_track->track.node_key == node_key)
            {
                AnimationTrack::SampleMorphTrack(sample_frame, morph_track, this, &part->span);
                break;
            }
            --active;
            ++morph_track;
            if (active == 0)
                break;
        }
    }
    if (active == 0 && restoration->active_count == 0)
        goto finish;
    remaining = restoration->count;
    if (remaining == 0)
        goto finish;
    entry = restoration->entries;
    for (;;)
    {
        if (entry->node_key == node_key)
            break;
        --remaining;
        if (remaining == 0)
            goto finish;
        ++entry;
    }
    slot_flags = entry->flags;
    if (active != 0)
    {
        if ((slot_flags & 1) == 0)
        {
            entry->flags = slot_flags | 1;
            ++restoration->active_count;
        }
    }
    else if ((slot_flags & 1) != 0)
    {
        entry->flags = slot_flags & 0xFFFE;
        --restoration->active_count;
        point = part->span.first_vertex;
        points = part->span.vertex_count;
        vertex = vertices + point;
        src_vertex = entry->vertices;
        do
        {
            vertex->n.vz = 0;
            vertex->n.vy = 0;
            vertex->n.vx = 0;
            vertex->pos = *src_vertex;
            ++src_vertex;
            ++vertex;
            --points;
        } while (points != 0);
        RebuildMorphNormals(&part->span);
    }
finish:
    hd_animation_end(this, node);
    return;
}

uint32 AnimatedObject::ClampAnimationFrame()
{
    uint32 cur_flags;
    uint32 result;
    uint8 byte_flags;
    sint32 sample_frame;
    sint32 count;
    FUNCTION_MARKER(0x8001A6AC, "SLUS_005.78");
    cur_flags = flags;
    if ((cur_flags & 0x10000) != 0)
    {
        sample_frame = frame;
        if (sample_frame == 2 || sample_frame == animation->frame_count)
        {
            cur_flags = flags;
            frame = 0;
            flags = (cur_flags & 0xFFFEFFFF) | 0x20000;
            goto mark_changed;
        }
    }
    sample_frame = frame;
    if (sample_frame < 0)
        frame = 0;
    else
    {
        count = animation->frame_count;
        if (sample_frame >= count)
            frame = count - 1;
    }
mark_changed:
    result = flags;
    byte_flags = node.flags;
    result |= 1;
    flags = result;
    node.flags = byte_flags | 0x40;
    return result;
}

void AnimatedObject::ApplyNodeAnimation(AnimatedObject *parent, SceneNode *node)
{
    AnimatedObject *animated_owner;
    const AnimationHeader *header;
    Vec3 extent;
    MeshPartNode *part = (MeshPartNode *)node;
    SpriteRecord *spr = (SpriteRecord *)node;
    const sint8 *pos_binding;
    const sint8 *rotation_binding = 0;
    const sint8 *morph_binding = 0;
    Vec3 *pos;
    Quat *rotation;
    const MorphSpan *morph_span;
    FUNCTION_MARKER(0x8001AECC, "SLUS_005.78");
    AnimatedObject *owner = parent;
    uint32 category = node->kind;
    AnimationHeader *animation;
    uint32 flags;
    AnimationTrack *track;
    MorphAnimationTrack *morph_track;
    sint32 idx, frame;
    if (category == 0)
        owner = (AnimatedObject *)node;
    if (owner == 0)
        goto done;
    animated_owner = owner;
    animation = animated_owner->animation;
    if (animation == 0)
        goto done;
    header = animation;
    flags = animated_owner->flags;
    if ((flags & 0x20) != 0)
    {
        animated_owner->flags = flags & 0xFFFFFFDF;
        owner->SampleNodeAnimation(node);
        animated_owner->flags |= 0x20;
        goto done;
    }
    hd_animation_begin(owner, node, true);
    switch (category)
    {
        case 0:
            if (animated_owner->animation_binding.word == 0xFFFFFFFF)
                goto done;
            pos_binding = animated_owner->animation_binding.tracks;
            pos = &animated_owner->pos;
            rotation_binding = animated_owner->animation_binding.tracks + 1;
            morph_binding = animated_owner->animation_binding.tracks + 3;
            break;
        case 5:
            pos_binding = part->animation_binding.tracks;
            pos = &part->pos;
            rotation_binding = part->animation_binding.tracks + 1;
            morph_binding = part->animation_binding.tracks + 3;
            break;
        case 7:
            pos_binding = spr->animation_binding.tracks;
            pos = &spr->pos;
            break;
        case 8:
            pos_binding = (const sint8 *)&node->state_flags;
            pos = &((PositionedBillboardNode *)node)->pos;
            break;
        case 2:
        {
            SceneTrigger *trigger = (SceneTrigger *)node;
            pos_binding = trigger->animation_binding.tracks;
            pos = &trigger->pos;
            rotation_binding = trigger->animation_binding.tracks + 1;
            break;
        }
        case 4:
            pos_binding = (const sint8 *)&node->state_flags + 1;
            pos = &((SceneLight *)node)->pos;
            break;
        default:
            goto done;
    }
    idx = *pos_binding;
    if (idx >= 0)
    {
        track = &header->tracks[0].vectors[idx].track;
        flags = track->flags;
        if ((flags & 8) != 0)
        {
            frame = track->frame;
            track->flags = flags & 0xF7;
            AnimationTrack::SampleVectorTrack(frame, track, *pos);
        }
    }
    if (category == 7)
    {
        idx = spr->animation_binding.tracks[2];
        if (idx >= 0)
        {
            track = header->tracks[2].recs + idx;
            flags = track->flags;
            if ((flags & 8) != 0)
            {
                track->flags = flags & 0xF7;
                extent.x = (uint32)spr->extent[0] << 5;
                extent.y = (uint32)spr->extent[1] << 5;
                AnimationTrack::SampleVectorTrack(track->frame, track, extent);
                spr->extent[0] = extent.x >> 5;
                spr->extent[1] = extent.y >> 5;
            }
        }
        goto done;
    }
    if (rotation_binding != 0)
    {
        idx = *rotation_binding;
        if (idx >= 0)
        {
            track = header->tracks[1].recs + idx;
            flags = track->flags;
            if ((flags & 8) != 0)
            {
                frame = track->frame;
                track->flags = flags & 0xF7;
                if (category == 0)
                    rotation = &animated_owner->rotation;
                else if (category == 5)
                    rotation = &part->quat;
                else
                    rotation = &((SceneTrigger *)node)->quat;
                AnimationTrack::SampleQuatTrack(frame, track, *rotation);
            }
        }
    }
    if (morph_binding != 0)
    {
        idx = *morph_binding;
        if (idx >= 0)
        {
            morph_track = header->tracks[3].morphs + idx;
            track = &morph_track->track;
            flags = track->flags;
            if ((flags & 8) != 0)
            {
                frame = track->frame;
                track->flags = flags & 0xF7;
                morph_span = category == 0 ? &animated_owner->mesh_span : &part->span;
                AnimationTrack::SampleMorphTrack(frame, morph_track, animated_owner, morph_span);
            }
        }
    }
done:
    hd_animation_end(owner, node);
    return;
}

uint32 AnimatedObject::AdvanceTracks(sint32 step)
{
    FUNCTION_MARKER(0x8001B334, "SLUS_005.78");
    AnimationHeader *clip;
    uint32 grp, count, stride, first, cur_flags, result;
    uint8 *track;
    uint32 changed;
    if ((flags & 0x30000) != 0)
        step = 0;
    clip = animation;
    changed = step == 0;
    if (clip != 0)
    {
        const AnimationHeader *header = clip;
        for (grp = 0; grp < 4; ++grp)
        {
            count = header->counts[grp];
            if (count == 0)
                continue;
            stride = grp == 0 ? sizeof(VectorAnimationTrack) : (grp == 3 ? sizeof(MorphAnimationTrack) : sizeof(AnimationTrack));
            track = header->tracks[grp].bytes != 0 ? header->tracks[grp].bytes : NULL;
            if (grp < 2)
                track += count * stride - stride;
            first = 1;
            do
            {
                AnimationTrack *cur_track = (AnimationTrack *)track;
                if (first != 0 && grp < 2 && (cur_track->flags & 4) != 0 && step != 0)
                {
                    if (grp == 0)
                    {
                        if (cur_track->frame == 0 && velocity.x < 0)
                            changed = 1;
                        else if ((cur_track->AdvanceVector(pos, step, velocity.x) & 0xFF) != 0)
                            changed = 1;
                    }
                    else
                    {
                        cur_track->frame = header->tracks[0].vectors->track.frame;
                        changed = 1;
                        header->tracks[0].vectors->track.BlendRotation(cur_track, pos, rotation, step);
                    }
                    goto next_track;
                }
                if (step == 0)
                {
                    cur_flags = flags;
                    if ((cur_flags & 0x30000) != 0)
                    {
                        if ((cur_flags & 0x20000) != 0)
                            cur_track->frame = 0;
                        else if ((cur_track->flags & 2) != 0)
                            cur_track->frame = (uint16)cur_track->frame_limit - 1;
                        else
                            cur_track->frame = cur_track->frame_limit;
                    }
                    else if ((cur_track->flags & 2) != 0)
                    {
                        sint32 initial = frame;
                        sint32 limit = cur_track->frame_limit;

                        if (limit == 0)
                            xport_mips_break(7);
                        cur_track->frame = initial % limit;
                    }
                    else
                    {
                        sint32 initial = frame;
                        sint32 limit = cur_track->frame_limit;
                        cur_track->frame = initial < limit ? initial : limit;
                    }
                }
                else
                {
                    uint32 old = (uint16)cur_track->frame;
                    uint32 limit = (uint16)cur_track->frame_limit;
                    uint32 sum = old + step;
                    uint32 sample_frame = sum & 0xFFFF;
                    cur_track->frame = sum;
                    if (sample_frame >= limit)
                    {
                        if ((cur_track->flags & 2) != 0)
                        {
                            cur_track->frame = (sum & 0x8000) != 0 ? limit - 1 : 0;
                            if (grp == 0 && (cur_track->flags & 3) == 2 && cur_track->frame != (sint16)sum)
                                hd_sprite_track_loop(this, cur_track);
                        }
                        else if (limit < sample_frame)
                        {
                            cur_track->frame = sum - step;
                            goto next_track;
                        }
                    }
                    changed = 1;
                }
                cur_track->flags = cur_track->flags | 8;
            next_track:
                first = 0;
                --count;
                if (count != 0)
                {
                    if (grp < 2)
                        track -= stride;
                    else
                        track += stride;
                }
            } while (count != 0);
        }
    }
    cur_flags = flags;
    if (changed != 0)
        cur_flags = (cur_flags & 0xFFFCFFFF) | 1;
    else
        cur_flags &= 0xFFFBFFFF;
    flags = cur_flags;
    result = node.flags | 0x40;
    node.flags = result;
    return result;
}

CollisionSphere *AnimatedObject::BuildCharacterGeometry()
{
    FUNCTION_MARKER(0x8001BD30, "SLUS_005.78");
    MATRIX rotation;
    VECTOR world;
    VECTOR input;
    Vec3 minimum;
    Vec3 maximum;
    Vec3 low;
    Vec3 high;
    const PathDefinition *parent;
    uint32 count;
    uint32 offset;
    uint32 idx;
    uint32 val;
    uint32 x;
    uint32 y;
    uint32 z;
    uint32 max_x;
    sint32 max_y;
    uint32 max_z;
    uint32 radius, half_y, half_z, ratio, spacing, angle, c, s;
    CollisionSphere *output, *dst;
    const CollisionSphere *src;
    MATRIX *parent_matrix;
    const SceneNode *shape_node;
    sint32 quotient;
    if ((flags & 0x2000) == 0 && (parent = path_definition) != 0 && parent->transform != 0)
    {
        x = pos.x;
        y = pos.y;
        z = pos.z;
        input.vx = x;
        input.vy = y;
        input.vz = z;
        PushMatrix();
        parent_matrix = path_definition->transform != 0 ? &path_definition->transform->matrix : NULL;
        SetRotMatrix(parent_matrix);
        ApplyRotMatrixLV(&input, &world);
        PopMatrix();
        parent_matrix = path_definition->transform != 0 ? &path_definition->transform->matrix : NULL;
        val = world.vx;
        world.vx = val + parent_matrix->t[0];
        val = world.vy;
        world.vy = val + parent_matrix->t[1];
        val = world.vz;
        world.vz = val + parent_matrix->t[2];
    }
    else
    {
        x = pos.x;
        y = pos.y;
        z = pos.z;
        world.vx = x;
        world.vy = y;
        world.vz = z;
    }
    if ((character_pad.flags & 0x40000000) == 0)
    {
        character_geometry.world[3].radius = 0xFFFFFFFF;
        character_geometry.world[2].radius = 0xFFFFFFFF;
        character_geometry.world[1].radius = 0xFFFFFFFF;
        character_geometry.world[0].radius = 0xFFFFFFFF;
        shape_node = node.links.first_child;
        count = 0;
        offset = 0;
        output = character_geometry.world;
        while (shape_node != 0)
        {
            if (shape_node->kind == 6)
            {
                const SphereNode *sphere_node = (const SphereNode *)shape_node;

                output->pos = sphere_node->pos;
                radius = sphere_node->radius & 0xFFFFFFF8;
                output->radius = radius;
                high.z = radius;
                high.y = radius;
                high.x = radius;
                x = output->pos.x - radius;
                low.x = x;
                y = output->pos.y - radius;
                low.y = y;
                z = output->pos.z - radius;
                low.z = z;
                max_x = output->pos.x + radius;
                high.x = max_x;
                max_y = output->pos.y + radius;
                high.y = max_y;
                max_z = output->pos.z + radius;
                high.z = max_z;
                if (count == 0)
                {
                    minimum = low;

                    maximum = high;
                }
                else
                {
                    if ((sint32)x < minimum.x)
                        minimum.x = x;
                    if (maximum.x < (sint32)max_x)
                        maximum.x = max_x;
                    if ((sint32)y < minimum.y)
                        minimum.y = y;
                    if (maximum.y < max_y)
                        maximum.y = max_y;
                    if ((sint32)z < minimum.z)
                        minimum.z = z;
                    if (maximum.z < (sint32)max_z)
                        maximum.z = max_z;
                }
                ++output;
                offset += 16;
                ++count;
            }
            shape_node = shape_node->links.sibling_next;
        }
        if (count == 0)
            goto copy_geometry;
        x = minimum.x;
        max_x = maximum.x;
        val = -x;
        if ((sint32)val < (sint32)max_x)
            minimum.x = -max_x;
        else
            maximum.x = val;
        z = minimum.z;
        max_z = maximum.z;
        val = -z;
        if ((sint32)val < (sint32)max_z)
            minimum.z = -max_z;
        else
            maximum.z = val;
        max_y = maximum.y;
        y = minimum.y;
        max_z = maximum.z;
        z = minimum.z;
        half_y = (sint32)(max_y - y) >> 1;
        half_z = (sint32)(max_z - z) >> 1;
        if (half_z == 0)
            xport_mips_break(7);
        quotient = 0x40000000 / (sint32)half_z;
        world.vy = half_y;
        world.vz = half_z;
        ratio = ((sint64)(sint32)half_y * quotient) >> 18;
        fixed_math_result = ratio;
        character_geometry.world[0].pos.x = 0;
        world.vx = ratio;
        if (ratio - 3277 < 0x666)
        {
            radius = (sint32)ratio < 4096 ? half_z : half_y;
            character_geometry.world[0].pos.y = y + radius;
            character_geometry.world[0].radius = radius;
            character_geometry.world[0].pos.z = 0;
            character_geometry.world[3].radius = 0xFFFFFFFF;
            character_geometry.world[2].radius = 0xFFFFFFFF;
            character_geometry.world[1].radius = 0xFFFFFFFF;
        }
        else if ((sint32)half_z < (sint32)half_y)
        {
            spacing = (sint32)half_y / 4;
            world.vy = spacing;
            if ((sint32)spacing < (sint32)half_z)
            {
                val = (max_y - half_z) - (y + half_z);
                character_geometry.world[0].radius = half_z;
                spacing = (sint32)val / 6;
                world.vy = spacing;
            }
            else
                character_geometry.world[0].radius = spacing;
            spacing = world.vy;
            y = minimum.y;
            radius = character_geometry.world[0].radius;
            character_geometry.world[0].pos.z = 0;
            spacing <<= 1;
            world.vy = spacing;
            character_geometry.world[0].pos.y = y + radius;
            for (offset = 16; offset < 64; offset += 16)
            {
                if (character_geometry.world[offset / 16].radius != 0xFFFFFFFF)
                {
                    src = &character_geometry.world[offset / 16 - 1];
                    dst = &character_geometry.world[offset / 16];
                    x = src->pos.x;
                    y = src->pos.y;
                    z = src->pos.z;
                    radius = src->radius;
                    dst->pos.x = x;
                    dst->pos.y = y;
                    dst->pos.z = z;
                    dst->radius = radius;
                    val = dst->pos.y;
                    dst->pos.y = val + world.vy;
                }
            }
        }
        else
        {
            spacing = (sint32)half_z / 4;
            world.vz = spacing;
            if ((sint32)spacing < (sint32)half_y)
            {
                val = (max_z - half_y) - (z + half_y);
                character_geometry.world[0].radius = half_y;
                spacing = (sint32)val / 6;
                world.vz = spacing;
            }
            else
                character_geometry.world[0].radius = spacing;
            spacing = world.vz;
            y = minimum.y;
            radius = character_geometry.world[0].radius;
            z = minimum.z;
            spacing <<= 1;
            world.vz = spacing;
            character_geometry.world[0].pos.y = y + radius;
            character_geometry.world[0].pos.z = z + radius;
            for (offset = 16; offset < 64; offset += 16)
            {
                if (character_geometry.world[offset / 16].radius != 0xFFFFFFFF)
                {
                    src = &character_geometry.world[offset / 16 - 1];
                    dst = &character_geometry.world[offset / 16];
                    x = src->pos.x;
                    y = src->pos.y;
                    z = src->pos.z;
                    radius = src->radius;
                    dst->pos.x = x;
                    dst->pos.y = y;
                    dst->pos.z = z;
                    dst->radius = radius;
                    val = dst->pos.z;
                    dst->pos.z = val + world.vz;
                }
            }
        }
    copy_geometry:
        for (idx = 0; idx < 4; ++idx)
            character_geometry.local[idx] = character_geometry.world[idx];
        if ((flags & 0x10000) == 0)
            character_pad.flags = character_pad.flags | 0x40000000;
    }
    val = flags;
    angle = (uint16)target_yaw;
    if ((val & 0x10) != 0)
        angle += 2048;
    x = psx_identity_rotation_words[0];
    y = psx_identity_rotation_words[1];
    xport_gte_write_control(0, x);
    xport_gte_write_control(1, y);
    x = psx_identity_rotation_words[2];
    y = psx_identity_rotation_words[3];
    z = psx_identity_rotation_words[4];
    xport_gte_write_control(2, x);
    xport_gte_write_control(3, y);
    xport_gte_write_control(4, z);
    c = SinQ14((1024 - angle)) >> 2;
    s = SinQ14(angle) >> 2;
    rotation_y.m[0][0] = c;
    rotation_y.m[2][0] = -s;
    rotation_y.m[0][2] = s;
    rotation_y.m[2][2] = c;
    MulRotMatrix0(&rotation_y, &rotation);
    SetRotMatrix(&rotation);
    for (offset = 0; offset < 64; offset += 16)
    {
        val = character_geometry.local[offset / 16].radius;
        output = &character_geometry.world[offset / 16];
        output->radius = val;
        if (val != 0xFFFFFFFF)
        {
            VECTOR input_vector, output_vector;
            input_vector.vx = character_geometry.local[offset / 16].pos.x;
            input_vector.vy = character_geometry.local[offset / 16].pos.y;
            input_vector.vz = character_geometry.local[offset / 16].pos.z;
            ApplyRotMatrixLV(&input_vector, &output_vector);
            output->pos.x = output_vector.vx;
            output->pos.y = output_vector.vy;
            output->pos.z = output_vector.vz;
            val = output->pos.x;
            output->pos.x = val + world.vx;
            val = output->pos.y;
            output->pos.y = val + world.vy;
            val = output->pos.z;
            output->pos.z = val + world.vz;
        }
    }
    return character_geometry.world;
}

void AnimatedObject::GetCharacterSphereEnds(uint32 sphere, uint8 *first, uint8 *last_output) const
{
    FUNCTION_MARKER(0x8001C69C, "SLUS_005.78");
    uint32 result, last;
    if (contact_kind != 1 || character_geometry.local[0].pos.z != 0)
    {
        *last_output = 1;
        *first = 1;
        return;
    }
    *first = sphere == 1;

    if (sphere == 4)
        last = 1;
    else
    {
        last = character_geometry.world[sphere].radius;
        result = 0xFFFFFFFF;
        last = last == result;
    }
    *last_output = last;
}

uint32 AnimatedObject::AcceptVerticalFaceContact(const CollisionTransform *desc, uint32 selection) const
{
    const PathTransform *path_transform;
    FUNCTION_MARKER(0x8001C710, "SLUS_005.78");
    const PathDefinition *definition;
    uint32 dx, dy, dz, mode, upper, lower;
    dx = velocity.x - (uint32)desc->motion.x;
    dy = velocity.y - (uint32)desc->motion.y;
    dz = velocity.z - (uint32)desc->motion.z;
    definition = path_definition;
    if (definition != 0)
    {
        const PathTransform *captured_path_transform = definition->transform;
        if (captured_path_transform != 0 && (flags & 0x2000) == 0)
        {
            path_transform = captured_path_transform;
            dx += (uint32)path_transform->displacement.x;
            dy += (uint32)path_transform->displacement.y;
            dz += (uint32)path_transform->displacement.z;
        }
    }
    if (dx != 0 || dz != 0)
        return 0;
    mode = (selection & 0x7000) >> 12;
    if (contact_kind == 1 && character_geometry.local[0].pos.z == 0)
    {
        lower = mode == 1;
        upper = mode == 4 || character_geometry.world[mode].radius == 0xFFFFFFFF;
    }
    else
    {
        lower = 1;
        upper = 1;
    }
    if ((lower == 0 || (sint32)dy > 0) && upper == 0)
        return 1;
    if ((sint32)dy < 0 && lower == 0)
        return 1;
    return 0;
}

void AnimatedObject::ResolveSurfaceMotion(uint32 *move_flags)
{
    FUNCTION_MARKER(0x8001C8CC, "SLUS_005.78");
    CollisionTransform rec;
    MATRIX matrix_local;
    Vec3 n, relative, tangent;
    uint32 contact;
    const MeshFace *face;
    const SceneNode *owner;
    const MeshGeometryNode *owner_mesh, *pos_src;
    const PathDefinition *parent;
    uint32 x, y, z, a, b, c, d, e, f, g, h, i, val, cur_flags, up, down;
    const PathTransform *path_transform;
    uint32 dot, sq, mag, scaled_x, scaled_y, scaled_z, result;
    sint32 heading;
    uint32 angle;
    owner = character_pad.surface_interaction.sel;
    owner_mesh = (const MeshGeometryNode *)owner;
    contact = surface_contact & 0xFFFF8FFF;
    if (owner_mesh->node.kind == 1)
    {
        rec.rotation[0] = psx_identity_matrix.m[0][0];
        rec.rotation[1] = psx_identity_matrix.m[0][1];
        rec.rotation[2] = psx_identity_matrix.m[0][2];
        rec.rotation[3] = psx_identity_matrix.m[1][0];
        rec.rotation[4] = psx_identity_matrix.m[1][1];
        rec.rotation[5] = psx_identity_matrix.m[1][2];
        rec.rotation[6] = psx_identity_matrix.m[2][0];
        rec.rotation[7] = psx_identity_matrix.m[2][1];
        rec.rotation[8] = psx_identity_matrix.m[2][2];
        if (owner_mesh->vertex_count > 0)
            pos_src = owner_mesh;
        else
            pos_src = owner_mesh->src_mesh;

        rec.pos = pos_src->pos;
        rec.motion.z = 0;
        rec.motion.y = 0;
        rec.motion.x = 0;
    }
    else
    {
        const CollisionTransform *cached_transform = ((const AnimatedObject *)owner_mesh)->col_transforms;
        cached_transform = cached_transform + (contact >> 16);
        rec = *cached_transform;
    }
    rec.face_count = 1;
    rec.first_face = contact;
    face = owner_mesh->polygons;
    a = (uint16)rec.rotation[6];
    b = (uint16)rec.rotation[0];
    c = (uint16)rec.rotation[3];
    d = (uint16)rec.rotation[1];
    e = (uint16)rec.rotation[4];
    f = (uint16)rec.rotation[7];
    g = (uint16)rec.rotation[2];
    h = (uint16)rec.rotation[5];
    i = (uint16)rec.rotation[8];
    matrix_local.m[0][2] = a;
    matrix_local.m[0][1] = c;
    face += (uint16)contact;
    matrix_local.m[0][0] = b;
    matrix_local.m[1][0] = d;
    matrix_local.m[1][1] = e;
    matrix_local.m[1][2] = f;
    matrix_local.m[2][0] = g;
    matrix_local.m[2][1] = h;
    matrix_local.m[2][2] = i;
    PushMatrix();
    SetRotMatrix(&matrix_local);
    xport_gte_write_data(0, (uint16)face->n.x | ((uint32)face->n.y << 16));
    xport_gte_write_data(1, (uint16)face->n.z | ((uint32)face->n.depth_bias << 16));
    xport_gte_execute(0x486012);
    n.x = xport_gte_read_data(25);
    n.y = xport_gte_read_data(26);
    n.z = xport_gte_read_data(27);
    PopMatrix();
    x = velocity.x;
    a = rec.motion.x;
    x -= a;
    relative.x = x;
    y = velocity.y;
    b = rec.motion.y;
    y -= b;
    relative.y = y;
    z = velocity.z;
    c = rec.motion.z;
    z -= c;
    relative.z = z;
    parent = path_definition;
    if (parent != 0 && (path_transform = parent->transform) != 0 && (flags & 0x2000) == 0)
    {
        relative.x = x + path_transform->displacement.x;
        path_transform = path_definition->transform;
        relative.y = y + path_transform->displacement.y;
        path_transform = path_definition->transform;
        relative.z = z + path_transform->displacement.z;
    }
    val = surface_contact;
    heading = contact_kind;
    val = (val & 0x7000) >> 12;
    if (heading == 1 && character_geometry.local[0].pos.z == 0)
    {
        up = val == 1;
        down = val == 4 || character_geometry.world[val].radius == 0xFFFFFFFF;
    }
    else
    {
        up = 1;
        down = 1;
    }
    if ((uint32)n.y + 128 < 257 || (flags & 0x400) == 0 || (velocity.x == 0 && velocity.y == 0 && velocity.z == 0) || (up == 0 && down == 0))
        goto reject_contact;
    val = n.y;
    if ((sint32)val < 0 && (down == 0 || relative.y < 0))
        goto reject_contact;
    if ((sint32)val > 0 && up == 0)
        goto reject_contact;
    if (n.y >= 0 && up != 0)
        goto slide_contact;
    if ((character_pad.flags & 0x10000) != 0)
        goto reverse_speed;
    val = character_pad.movement.jump_ticks;
    cur_flags = character_pad.flags;
    character_pad.motion.jump_phase = -1;
    character_pad.motion.vertical_speed = 0;
    character_pad.timers.state_ticks = 0;
    character_pad.timers.jump_duration_ticks = val;
    character_pad.flags = cur_flags | 0x08000000;
    val = rec.motion.y;
    if ((sint32)val < 0)
        character_pad.motion.vertical_speed = val;
    if ((character_pad.secondary_flags & 0x80) == 0)
        character_pad.motion.state_code = 6;
    cur_flags = character_pad.flags;
    if ((cur_flags & 0x40) == 0 && ((uint16)(character_pad.motion.form_flags >> 16) & 15) == 5 && (sint32)((uint32)n.y - 128) < -4095)
    {
        character_pad.flags = (cur_flags & 0xFFFE7FBF) | 0x8000;
        goto save_contact;
    }
    result = (uint16)(character_pad.motion.form_flags >> 16) & 15;
    character_pad.motion.horizontal_speed = 0;
    if (result != 5)
    {
        result = *move_flags & 0xFFFFFFFE;
        *move_flags = result;
    }
    goto finish;
reject_contact:
    cur_flags = character_pad.flags;
    if ((cur_flags & 0x10000) != 0)
        goto reverse_speed;
    if ((cur_flags & 0x40) != 0)
    {
        goto finish;
    }
    character_pad.flags = (cur_flags & 0xFFFE7FBF) | 0x40;
    goto save_contact;
reverse_speed:
    x = character_pad.support.speed[0];
    y = character_pad.support.speed[1];
    result = -x;
    character_pad.support.speed[0] = result;
    character_pad.support.speed[1] = -y;
    goto finish;
slide_contact:
    val = character_pad.movement.jump_ticks;
    cur_flags = character_pad.flags;
    character_pad.motion.jump_phase = -1;
    character_pad.motion.vertical_speed = 0;
    character_pad.timers.state_ticks = 0;
    character_pad.motion.horizontal_speed = 0;
    character_pad.timers.jump_duration_ticks = val;
    character_pad.flags = cur_flags | 0x08000000;
    dot = mul12(velocity.x, n.x)
        + mul12(velocity.y, n.y)
        + mul12(velocity.z, n.z);
    x = n.x;
    scaled_x = mul12((sint32)x, (sint32)dot);
    y = n.y;
    tangent.x = scaled_x;
    scaled_y = mul12((sint32)y, (sint32)dot);
    z = n.z;
    tangent.y = scaled_y;
    scaled_z = mul12((sint32)z, (sint32)dot);
    tangent.z = scaled_z;
    x = velocity.x - scaled_x;
    tangent.x = x;
    y = velocity.y - scaled_y;
    tangent.y = y;
    z = velocity.z - scaled_z;
    tangent.z = z;
    sq = (uint32)mul12((sint32)x, (sint32)x) + (uint32)mul12((sint32)y, (sint32)y) + (uint32)mul12((sint32)z, (sint32)z);
    if ((sint32)sq < 16)
    {
        a = n.x;
        x = velocity.x;
        dot = mul12((sint32)a, (sint32)x);
        b = n.z;
        z = velocity.z;
        dot += (uint32)mul12((sint32)b, (sint32)z);
        x = n.x;
        scaled_x = mul12((sint32)x, (sint32)dot);
        y = n.y;
        tangent.x = scaled_x;
        scaled_y = mul12((sint32)y, (sint32)dot);
        z = n.z;
        tangent.y = scaled_y;
        scaled_z = mul12((sint32)z, (sint32)dot);
        tangent.z = scaled_z;
        x = velocity.x - scaled_x;
        y = -scaled_y;
        tangent.x = x;
        tangent.y = y;
        z = velocity.z - scaled_z;
        tangent.z = z;
        sq = (uint32)mul12((sint32)x, (sint32)x) + (uint32)mul12((sint32)y, (sint32)y) + (uint32)mul12((sint32)z, (sint32)z);
    }
    if ((sint32)sq < 2)
    {
        character_pad.support.speed[1] = 0;
        tangent.x = 6144;
        goto adjust_sign;
    }
    y = tangent.y;
    if ((sint32)y > 0)
    {
        x = tangent.x;
        z = tangent.z;
        tangent.y = -y;
        tangent.x = -x;
        tangent.z = -z;
    }
    if ((uint32)tangent.x + 26754 > 0xD103 || (uint32)tangent.y + 26754 > 0xD103 || (uint32)tangent.z + 26754 > 0xD103)
    {
        do
        {
            x = tangent.x >> 1;
            y = tangent.y >> 1;
            z = tangent.z >> 1;
            tangent.x = x;
            tangent.y = y;
            tangent.z = z;
        } while (x + 26754 > 0xD103 || y + 26754 > 0xD103 || z + 26754 > 0xD103);
    }
    {
        VECTOR sdk_input = {tangent.x, tangent.y, tangent.z, 0};
        VECTOR sdk_output;
        VectorNormal(&sdk_input, &sdk_output);
        tangent.x = sdk_output.vx;
        tangent.y = sdk_output.vy;
        tangent.z = sdk_output.vz;
    }
    x = tangent.x;
    scaled_x = mul12((sint32)x, 6144);
    y = tangent.y;
    tangent.x = scaled_x;
    scaled_y = mul12((sint32)y, 6144);
    z = tangent.z;
    tangent.y = scaled_y;
    scaled_z = mul12((sint32)z, 6144);
    tangent.z = scaled_z;
    character_pad.support.speed[1] = scaled_y;
    x = tangent.x >> 6;
    z = tangent.z >> 6;
    sq = x * x + z * z;
    tangent.y = 0;
    mag = (sint32)sq >= 0 ? SquareRoot12(sq) : SquareRoot12((sq >> 2)) << 1;
    z = velocity.z;
    x = velocity.x;
    heading = Atan2Angle(z, x);
    if (heading == 4096)
        heading = ((uint16)target_yaw + 2048) & 0xFFF;
    z = tangent.z;
    x = tangent.x;
    angle = Atan2Angle(z, x);
    val = (angle - (sint16)heading + 1024) & 0xFFF;
    tangent.x = val < 2048 ? mag : -mag;
adjust_sign:
    if ((sint32)character_pad.support.speed[0] < 0)
        tangent.x = -(uint32)tangent.x;
    if ((character_pad.flags & 0x10000) != 0)
    {
        x = character_pad.support.speed[0];
        y = tangent.x;
        if ((sint32)(x ^ y) < 0)
        {
            tangent.x = -y;
            character_pad.support.speed[1] = -character_pad.support.speed[1];
        }
    }
    if ((character_pad.flags & 0x40) != 0)
    {
        x = character_pad.support.speed[0];
        y = tangent.x;
        if ((sint32)(x ^ y) >= 0)
        {
            tangent.x = -y;
            character_pad.support.speed[1] = -character_pad.support.speed[1];
        }
    }
    cur_flags = character_pad.flags;
    val = tangent.x;
    character_pad.support.speed[0] = val;
    character_pad.flags = (cur_flags & 0xFFFE7FBF) | 0x10000;
    flags = flags | 0x400;
save_contact:
    result = surface_contact;
    owner = character_pad.surface_interaction.sel;
    result &= 0xFFFF8FFF;
    character_pad.support.face_ref.face_idx = result;
    character_pad.support.face_ref.transform_idx = result >> 16;
    character_pad.support.owner = (SceneNode *)owner;
finish:
    return;
}

void AnimatedObject::TranslateCharacterWithBounds(sint32 x, sint32 y, sint32 z)
{
    FUNCTION_MARKER(0x8001D3F0, "SLUS_005.78");
    CollisionSphere *spheres_val = spheres;
    uint32 idx;

    const PathDefinition *definition;
    for (idx = 0; idx < 4; ++idx)
    {
        spheres_val[idx].pos.x = spheres_val[idx].pos.x + ((uint32)x);
        spheres_val[idx].pos.y = spheres_val[idx].pos.y + ((uint32)y);
        spheres_val[idx].pos.z = spheres_val[idx].pos.z + ((uint32)z);
    }
    world_pos.x = (uint32)world_pos.x + x;
    world_pos.y = (uint32)world_pos.y + y;
    world_pos.z = (uint32)world_pos.z + z;
    if ((flags & 0x2000) == 0)
    {
        definition = path_definition;
        if (definition != 0 && definition->transform != 0)
        {
            VECTOR vector, t;
            MATRIX transpose = {0};
            memset((uint8 *)&transpose + sizeof(transpose.m), 0, offsetof(MATRIX, t) - sizeof(transpose.m));
            for (idx = 0; idx < 9; ++idx)
            {
                const PathTransform *path_transform = path_definition->transform;
                transpose.m[idx / 3][idx % 3] = path_transform->matrix.m[idx % 3][idx / 3];
            }
            vector.vx = x;
            vector.vy = y;
            vector.vz = z;
            PushMatrix();
            {
                SetRotMatrix(&transpose);
            }
            ApplyRotMatrixLV(&vector, &t);
            PopMatrix();
            x = t.vx;
            y = t.vy;
            z = t.vz;
        }
    }
    pos.x = (uint32)pos.x + x;
    pos.y = (uint32)pos.y + y;
    pos.z = (uint32)pos.z + z;
    character_pad.flags = character_pad.flags | 1;
    character_pad.support.candidate = 0;
    character_pad.support.fraction = 0;
    flags = flags & ~0x4000u;
}

void AnimatedObject::UpdateCharacterSupportMotion()
{
    const PathTransform *path_transform;
    uint32 axis_val;
    uint32 axis_val1;

    FUNCTION_MARKER(0x8001D698, "SLUS_005.78");
    Vec3 motion0;
    const SceneNode *other;
    const MeshGeometryNode *other_mesh;
    SupportContact support_contact;
    CollisionTransform col = {0};
    uint32 fraction_limit;
    uint32 result;

    ApplyCharacterPathMotion(character_pad.support.speed[0], character_pad.support.speed[1]);
    motion0 = velocity;
    velocity.x = 0;
    velocity.y = -40960;
    velocity.z = 0;
    other = character_pad.support.owner;
    other_mesh = (const MeshGeometryNode *)other;
    fraction_limit = 4096;
    {
        uint32 kind = other_mesh->node.kind;
        support_contact = character_pad.support.face_ref;
        if (kind == 1)
        {
            col.rotation[0] = psx_identity_matrix.m[0][0];
            col.rotation[1] = psx_identity_matrix.m[0][1];
            col.rotation[2] = psx_identity_matrix.m[0][2];
            col.rotation[3] = psx_identity_matrix.m[1][0];
            col.rotation[4] = psx_identity_matrix.m[1][1];
            col.rotation[5] = psx_identity_matrix.m[1][2];
            col.rotation[6] = psx_identity_matrix.m[2][0];
            col.rotation[7] = psx_identity_matrix.m[2][1];
            col.rotation[8] = psx_identity_matrix.m[2][2];
            const MeshGeometryNode *pos_src = other_mesh->vertex_count > 0 ? other_mesh : other_mesh->src_mesh;

            col.pos = pos_src->pos;
            col.motion.z = 0;
            col.motion.y = 0;
            col.motion.x = 0;
        }
        else
        {
            const CollisionTransform *cached_transform = ((const AnimatedObject *)other_mesh)->col_transforms;
            cached_transform = cached_transform + support_contact.transform_idx;

            col = *cached_transform;
        }
    }
    col.face_count = 1;
    col.first_face = support_contact.face_idx;
    result = FindSphereFaceCol(character_geometry.world, character_pad.support.owner, &col, &fraction_limit);
    velocity = motion0;
    if ((sint32)result < 0)
    {
        uint32 horizontal = character_pad.support.speed[0];
        uint32 vertical = character_pad.support.speed[1];
        character_pad.motion.horizontal_speed = horizontal;
        character_pad.motion.vertical_speed = vertical;
        if ((sint32)vertical > 0)
        {
            vertical = character_pad.motion.vertical_speed;
            character_pad.motion.horizontal_speed = 0;
            character_pad.motion.vertical_speed = -vertical;
        }
        result = character_pad.flags & ~0x10000u;
        character_pad.flags = result;
    }
    else
    {
        fraction_limit = 4096;
        result = FindSphereFaceCol(character_geometry.world, character_pad.support.owner, &col, &fraction_limit);
        if ((sint32)result >= 0)
        {
            const PathDefinition *definition;
            if ((flags & 0x2000) == 0 && (definition = path_definition) != 0 && definition->transform != 0)
            {
                path_transform = path_definition->transform;
                axis_val = col.motion.x;
                axis_val -= (uint32)path_transform->displacement.x;
                col.motion.x = axis_val;
                path_transform = path_definition->transform;
                axis_val = col.motion.y;
                axis_val -= (uint32)path_transform->displacement.y;
                col.motion.y = axis_val;
                path_transform = path_definition->transform;
                axis_val = col.motion.z;
                axis_val -= (uint32)path_transform->displacement.z;
                col.motion.z = axis_val;
            }
            if (col.motion.x != 0 || col.motion.y != 0 || col.motion.z != 0)
            {
                uint32 horizontal = velocity.x;
                uint32 brake = 0;
                sint32 factor;
                if (horizontal != 0 || velocity.z != 0)
                {
                    sint16 motion_angle = Atan2Angle(velocity.z, horizontal);
                    sint16 contact_angle = Atan2Angle(col.motion.z, col.motion.x);
                    brake = (((uint32)motion_angle - contact_angle + 1024) & 4095) < 2048;
                }
                factor = brake != 0 ? 3993 : 4198;
                axis_val1 = mul12(col.motion.x, factor);
                col.motion.x = axis_val1;
                axis_val1 = mul12(col.motion.y, factor);
                col.motion.y = axis_val1;
                axis_val1 = mul12(col.motion.z, factor);
                col.motion.z = axis_val1;
                if (brake != 0)
                {
                    velocity.x = velocity.x + (uint32)col.motion.x;
                    velocity.y = velocity.y + (uint32)col.motion.y;
                    velocity.z = velocity.z + (uint32)col.motion.z;
                }
                else
                    TranslateCharacterWithBounds(col.motion.x, col.motion.y, col.motion.z);
            }
            result = (uint32)velocity.y + 682;
            velocity.y = result;
        }
    }
}

void AnimatedObject::ResolveCharacterSupport(uint8 grounded)
{
    const PathDefinition *parent, *pad_definition;
    FUNCTION_MARKER(0x8001DB60, "SLUS_005.78");
    CollisionTransform rec = {0};
    Vec3 old_velocity, inverse, input;
    MATRIX matrix_local = {0};
    uint32 fraction;
    Vec3 world, floor;
    SupportContact support_contact;
    uint32 type, idx, row;
    const MeshFace *triangle;
    PathTransform *path_transform;
    const ScenePath *pad_path;
    const SceneNode *owner;
    const MeshGeometryNode *owner_mesh;
    uint32 x, y, z, a, b, c, d, e, f, g, h, i, cur_flags, val, aligned, factor, result;
    sint32 query;
    sint32 correction;
    uint32 heading;
    sint32 angle;
    memset((uint8 *)&matrix_local + sizeof(matrix_local.m), 0, offsetof(MATRIX, t) - sizeof(matrix_local.m));
    owner = character_pad.support.owner;
    owner_mesh = (const MeshGeometryNode *)owner;
    type = owner_mesh->node.kind;
    support_contact = character_pad.support.face_ref;
    if (type == 1)
    {
        rec.rotation[0] = psx_identity_matrix.m[0][0];
        rec.rotation[1] = psx_identity_matrix.m[0][1];
        rec.rotation[2] = psx_identity_matrix.m[0][2];
        rec.rotation[3] = psx_identity_matrix.m[1][0];
        rec.rotation[4] = psx_identity_matrix.m[1][1];
        rec.rotation[5] = psx_identity_matrix.m[1][2];
        rec.rotation[6] = psx_identity_matrix.m[2][0];
        rec.rotation[7] = psx_identity_matrix.m[2][1];
        rec.rotation[8] = psx_identity_matrix.m[2][2];
        const MeshGeometryNode *pos_src = owner_mesh->vertex_count > 0 ? owner_mesh : owner_mesh->src_mesh;

        rec.pos = pos_src->pos;
        rec.motion.z = 0;
        rec.motion.y = 0;
        rec.motion.x = 0;
    }
    else
    {
        const CollisionTransform *cached_transform = ((const AnimatedObject *)owner_mesh)->col_transforms;
        cached_transform = cached_transform + support_contact.transform_idx;
        rec = *cached_transform;
    }
    rec.face_count = 1;
    rec.first_face = support_contact.face_idx;
    fraction = 4096;
    triangle = owner_mesh->polygons + support_contact.face_idx;
    if (grounded != 0)
    {
        x = velocity.x;
        y = velocity.y;
        z = velocity.z;
        velocity.x = x << 2;
        velocity.y = y << 2;
        velocity.z = z << 2;
    }
    owner = character_pad.support.owner;
    owner_mesh = (const MeshGeometryNode *)owner;
    query = FindSphereFaceCol(character_geometry.world, owner, &rec, &fraction);
    if (grounded != 0)
    {
        x = velocity.x >> 2;
        y = velocity.y >> 2;
        z = velocity.z >> 2;
        velocity.x = x;
        velocity.y = y;
        velocity.z = z;
        if (query < 0)
            goto no_contact;
        if ((AcceptVerticalFaceContact(&rec, query) & 0xFF) != 0)
            query = -1;
    }
    if (query < 0)
        goto no_contact;
    cur_flags = character_pad.flags;
    pad_path = character_pad.path;
    character_pad.support.candidate = 0;
    character_pad.support.fraction = 0;
    character_pad.flags = cur_flags | 1;
    pad_definition = pad_path->definition;
    parent = path_definition;
    if (pad_definition != parent)
    {
        if (parent != 0 && parent->transform != 0)
        {
            world = pos;
            PushMatrix();
            path_transform = path_definition->transform;
            SetRotMatrix(&path_transform->matrix);
            {
                VECTOR rotation_output;
                {
                    VECTOR sdk_input = {world.x, world.y, world.z, 0};
                    ApplyRotMatrixLV(&sdk_input, &rotation_output);
                }
                pos.x = rotation_output.vx;
                pos.y = rotation_output.vy;
                pos.z = rotation_output.vz;
            }
            PopMatrix();
            path_transform = path_definition->transform;
            x = pos.x;
            val = path_transform->matrix.t[0];
            parent = path_definition;
            pos.x = x + val;
            path_transform = parent->transform;
            y = pos.y;
            val = path_transform->matrix.t[1];
            parent = path_definition;
            pos.y = y + val;
            path_transform = parent->transform;
            z = pos.z;
            val = path_transform->matrix.t[2];
            pos.z = z + val;
        }
        CameraMotion::SelectScenePath(character_pad.path);
        pad_path = character_pad.path;
        parent = pad_path->definition;
        path_definition = parent;
        if (parent != 0 && (path_transform = parent->transform) != 0)
        {
            x = pos.x;
            val = path_transform->matrix.t[0];
            inverse.x = x - val;
            path_transform = path_definition->transform;
            x = pos.y;
            val = path_transform->matrix.t[1];
            inverse.y = x - val;
            path_transform = path_definition->transform;
            x = pos.z;
            val = path_transform->matrix.t[2];
            inverse.z = x - val;
            for (idx = 0; idx < 9; ++idx)
            {
                path_transform = path_definition->transform;
                val = (uint16)path_transform->matrix.m[idx % 3][idx / 3];
                matrix_local.m[idx / 3][idx % 3] = val;
            }
            PushMatrix();
            SetRotMatrix(&matrix_local);
            {
                VECTOR rotation_output;
                {
                    VECTOR sdk_input = {inverse.x, inverse.y, inverse.z, 0};
                    ApplyRotMatrixLV(&sdk_input, &rotation_output);
                }
                pos.x = rotation_output.vx;
                pos.y = rotation_output.vy;
                pos.z = rotation_output.vz;
            }
            PopMatrix();
        }
        flags = flags & 0xFFFFBFFF;
    }

    old_velocity = velocity;
    velocity.z = 0;
    velocity.x = 0;
    if (grounded == 0)
    {
        if (((uint16)(character_pad.motion.form_flags >> 16) & 15) == 1 && (flags & 0x04000000) != 0)
            EndCharacterSpecialMove();
        cur_flags = flags;
        velocity.y = 0;
        flags = cur_flags & 0xFFFFFBFF;
        if ((cur_flags & 0x2000) == 0)
        {
            parent = path_definition;
            if (parent != 0 && parent->transform != 0)
            {
                input = pos;
                PushMatrix();
                path_transform = path_definition->transform;
                SetRotMatrix(&path_transform->matrix);
                {
                    VECTOR sdk_input = {input.x, input.y, input.z, 0};
                    VECTOR sdk_output;
                    ApplyRotMatrixLV(&sdk_input, &sdk_output);
                    world.x = sdk_output.vx;
                    world.y = sdk_output.vy;
                    world.z = sdk_output.vz;
                }
                PopMatrix();
                path_transform = path_definition->transform;
                x = world.x;
                val = path_transform->matrix.t[0];
                world.x = x + val;
                path_transform = path_definition->transform;
                x = world.y;
                val = path_transform->matrix.t[1];
                world.y = x + val;
                path_transform = path_definition->transform;
                x = world.z;
                val = path_transform->matrix.t[2];
                world.z = x + val;
            }
            else
            {
                world = pos;
            }
            parent = path_definition;
            cur_flags = flags;
            correction = parent->FindHeightFrame(world, cur_flags & 0x10, -40960);
            if (correction >= 0)
            {
                cur_flags = flags;
                parent = path_definition;
                correction = parent->ProjectFrame(correction, world, floor, cur_flags & 0x10, 4096);
                if (correction >= 0)
                {
                    camera_motion.frame = correction;
                    y = floor.y;
                    val = world.y;
                    velocity.y = y - val;
                }
            }
        }
    }
    else if (velocity.y > 0)
    {
        val = (query & 0x7000) >> 12;
        if (character_geometry.local[0].pos.z != 0 || val >= 4 || character_geometry.world[val].radius == 0xFFFFFFFF)
        {
            a = (uint16)rec.rotation[0];
            b = (uint16)rec.rotation[3];
            c = (uint16)rec.rotation[6];
            d = (uint16)rec.rotation[1];
            e = (uint16)rec.rotation[4];
            f = (uint16)rec.rotation[7];
            g = (uint16)rec.rotation[2];
            h = (uint16)rec.rotation[5];
            i = (uint16)rec.rotation[8];
            matrix_local.m[0][0] = a;
            matrix_local.m[0][1] = b;
            matrix_local.m[0][2] = c;
            matrix_local.m[1][0] = d;
            matrix_local.m[1][1] = e;
            matrix_local.m[1][2] = f;
            matrix_local.m[2][0] = g;
            matrix_local.m[2][1] = h;
            matrix_local.m[2][2] = i;
            PushMatrix();
            SetRotMatrix(&matrix_local);
            xport_gte_write_data(0, (uint16)triangle->n.x | ((uint32)triangle->n.y << 16));
            xport_gte_write_data(1, (uint16)triangle->n.z | ((uint32)triangle->n.depth_bias << 16));
            xport_gte_execute(0x486012);
            floor.x = xport_gte_read_data(25);
            floor.y = xport_gte_read_data(26);
            floor.z = xport_gte_read_data(27);
            PopMatrix();
            if (floor.y < -256)
            {
                val = character_pad.movement.jump_ticks;
                cur_flags = character_pad.flags;
                character_pad.motion.jump_phase = -1;
                character_pad.motion.vertical_speed = 0;
                character_pad.timers.state_ticks = 0;
                character_pad.timers.jump_duration_ticks = val;
                character_pad.flags = cur_flags | 0x08000000;
                if ((character_pad.secondary_flags & 0x80) == 0)
                    character_pad.motion.state_code = 6;
                velocity.y = 0;

                goto finish;
            }
        }
    }
    parent = path_definition;
    if (parent != 0 && (path_transform = parent->transform) != 0 && (flags & 0x2000) == 0)
    {
        x = rec.motion.x;
        val = path_transform->displacement.x;
        rec.motion.x = x - val;
        path_transform = path_definition->transform;
        x = rec.motion.y;
        val = path_transform->displacement.y;
        rec.motion.y = x - val;
        path_transform = path_definition->transform;
        x = rec.motion.z;
        val = path_transform->displacement.z;
        rec.motion.z = x - val;
    }
    if (rec.motion.x == 0 && rec.motion.z == 0)
    {
        if (character_pad.timers.contact_impulse_ticks == 0)
            character_pad.motion.horizontal_speed = 0;
        goto save_frame;
    }
    x = old_velocity.x;
    aligned = 0;
    if (x != 0 || old_velocity.z != 0)
    {
        z = old_velocity.z;
        heading = Atan2Angle(z, x);
        z = rec.motion.z;
        x = rec.motion.x;
        angle = Atan2Angle(z, x);
        aligned = ((heading - angle + 1024) & 0xFFF) < 2048;
    }
    if (fraction != 0)
    {
        old_velocity.z = 0;
        old_velocity.y = 0;
        old_velocity.x = 0;
        goto damp_velocity;
    }

    old_velocity = rec.motion;
    if (aligned != 0)
    {
        x = old_velocity.x;
        y = old_velocity.y;
        z = old_velocity.z;
        old_velocity.x = -x;
        old_velocity.y = -y;
        old_velocity.z = -z;
    }
    x = old_velocity.x;
    a = mul12((sint32)x, 40960);
    y = old_velocity.y;
    old_velocity.x = a;
    b = mul12((sint32)y, 40960);
    z = old_velocity.z;
    old_velocity.y = b;
    old_velocity.z = mul12((sint32)z, 40960);
    for (row = 0; row < 4; ++row)
    {
        x = spheres[row].pos.x;
        y = old_velocity.x;
        spheres[row].pos.x = x + y;
        x = spheres[row].pos.y;
        y = old_velocity.y;
        spheres[row].pos.y = x + y;
        x = spheres[row].pos.z;
        y = old_velocity.z;
        spheres[row].pos.z = x + y;
    }
    x = velocity.x;
    y = old_velocity.x;
    velocity.x = x - y;
    x = velocity.y;
    y = old_velocity.y;
    velocity.y = x - y;
    x = velocity.z;
    y = old_velocity.z;
    velocity.z = x - y;
    fraction = 4096;
    owner = character_pad.support.owner;
    owner_mesh = (const MeshGeometryNode *)owner;
    type = owner_mesh->node.kind;
    support_contact = character_pad.support.face_ref;
    if (type == 1)
    {
        rec.rotation[0] = psx_identity_matrix.m[0][0];
        rec.rotation[1] = psx_identity_matrix.m[0][1];
        rec.rotation[2] = psx_identity_matrix.m[0][2];
        rec.rotation[3] = psx_identity_matrix.m[1][0];
        rec.rotation[4] = psx_identity_matrix.m[1][1];
        rec.rotation[5] = psx_identity_matrix.m[1][2];
        rec.rotation[6] = psx_identity_matrix.m[2][0];
        rec.rotation[7] = psx_identity_matrix.m[2][1];
        rec.rotation[8] = psx_identity_matrix.m[2][2];
        const MeshGeometryNode *pos_src = owner_mesh->vertex_count > 0 ? owner_mesh : owner_mesh->src_mesh;

        rec.pos = pos_src->pos;
        rec.motion.z = 0;
        rec.motion.y = 0;
        rec.motion.x = 0;
    }
    else
    {
        const CollisionTransform *cached_transform = ((const AnimatedObject *)owner_mesh)->col_transforms;
        cached_transform = cached_transform + support_contact.transform_idx;
        rec = *cached_transform;
    }
    rec.face_count = 1;
    rec.first_face = support_contact.face_idx;
    owner = character_pad.support.owner;
    owner_mesh = (const MeshGeometryNode *)owner;
    query = FindSphereFaceCol(character_geometry.world, owner, &rec, &fraction);
    x = velocity.x;
    y = old_velocity.x;
    velocity.x = x + y;
    x = velocity.y;
    y = old_velocity.y;
    velocity.y = x + y;
    x = velocity.z;
    y = old_velocity.z;
    velocity.z = x + y;
    for (row = 0; row < 4; ++row)
    {
        x = spheres[row].pos.x;
        y = old_velocity.x;
        spheres[row].pos.x = x - y;
        x = spheres[row].pos.y;
        y = old_velocity.y;
        spheres[row].pos.y = x - y;
        x = spheres[row].pos.z;
        y = old_velocity.z;
        spheres[row].pos.z = x - y;
    }
    if (query >= 0 && (factor = fraction) != 0)
    {
        factor = 4096 - factor;
        fraction = factor;
        x = old_velocity.x;
        a = mul12((sint32)x, (sint32)factor);
        y = old_velocity.y;
        old_velocity.x = a;
        b = mul12((sint32)y, (sint32)factor);
        z = old_velocity.z;
        old_velocity.y = b;
        old_velocity.z = mul12((sint32)z, (sint32)factor);
    }
    else
    {
        old_velocity.z = 0;
        old_velocity.y = 0;
        old_velocity.x = 0;
    }
    if ((flags & 0x2000) == 0)
    {
        parent = path_definition;
        if (parent != 0 && (path_transform = parent->transform) != 0)
        {
            x = rec.motion.x;
            val = path_transform->displacement.x;
            rec.motion.x = x - val;
            path_transform = path_definition->transform;
            x = rec.motion.y;
            val = path_transform->displacement.y;
            rec.motion.y = x - val;
            path_transform = path_definition->transform;
            x = rec.motion.z;
            val = path_transform->displacement.z;
            rec.motion.z = x - val;
        }
    }
damp_velocity:
    factor = aligned != 0 ? 3993 : 4198;
    x = rec.motion.x;
    a = mul12((sint32)x, (sint32)factor);
    y = rec.motion.y;
    rec.motion.x = a;
    b = mul12((sint32)y, (sint32)factor);
    z = rec.motion.z;
    rec.motion.y = b;
    c = mul12((sint32)z, (sint32)factor);
    rec.motion.z = c;
    if (aligned != 0)
    {
        velocity.x = velocity.x + a;
        y = velocity.y;
        val = rec.motion.y;
        velocity.y = y + val;
        z = velocity.z;
        val = rec.motion.z;
        velocity.z = z + val;
    }
    else
    {
        x = old_velocity.x;
        y = old_velocity.y;
        z = old_velocity.z;
        old_velocity.x = a + x;
        old_velocity.y = b + y;
        old_velocity.z = c + z;
        if (character_pad.timers.contact_impulse_ticks == 0)
            character_pad.motion.horizontal_speed = 0;
    }
    z = old_velocity.z;
    x = old_velocity.x;
    y = old_velocity.y;
    TranslateCharacterWithBounds(x, y, z);
save_frame:

    cur_flags = flags;
    val = (uint16)camera_motion.frame;
    result = cur_flags & 0xFFFFF7FF;
    flags = result;
    path_frame = val;
    goto finish;
no_contact:
    result = (uint32)velocity.x;
    if (result == 0)
        result = velocity.z;
    if (result != 0)
    {
        result = character_pad.flags & 0xFFFFFFBF;
        character_pad.flags = result;
    }
finish:
    return;
}

void AnimatedObject::ProbeCharacterSupport()
{
    FUNCTION_MARKER(0x8001EB44, "SLUS_005.78");
    const SceneNode *other = character_pad.support.owner;
    const MeshGeometryNode *other_mesh = (const MeshGeometryNode *)other;
    uint32 kind = other_mesh->node.kind;
    SupportContact support_contact = character_pad.support.face_ref;
    CollisionTransform col = {0};
    uint32 fraction_limit;
    uint32 result;
    if (kind == 1)
    {
        col.rotation[0] = psx_identity_matrix.m[0][0];
        col.rotation[1] = psx_identity_matrix.m[0][1];
        col.rotation[2] = psx_identity_matrix.m[0][2];
        col.rotation[3] = psx_identity_matrix.m[1][0];
        col.rotation[4] = psx_identity_matrix.m[1][1];
        col.rotation[5] = psx_identity_matrix.m[1][2];
        col.rotation[6] = psx_identity_matrix.m[2][0];
        col.rotation[7] = psx_identity_matrix.m[2][1];
        col.rotation[8] = psx_identity_matrix.m[2][2];
        const MeshGeometryNode *pos_src = other_mesh->vertex_count > 0 ? other_mesh : other_mesh->src_mesh;

        col.pos = pos_src->pos;
        col.motion.z = 0;
        col.motion.y = 0;
        col.motion.x = 0;
    }
    else
    {
        const CollisionTransform *cached_transform = ((const AnimatedObject *)other_mesh)->col_transforms;
        cached_transform = cached_transform + support_contact.transform_idx;

        col = *cached_transform;
    }
    col.face_count = 1;
    col.first_face = support_contact.face_idx;
    fraction_limit = 4096;
    result = FindSphereFaceCol(character_geometry.world, character_pad.support.owner, &col, &fraction_limit);
    if ((sint32)result >= 0)
        velocity.y = 0;
    else
    {
        result = velocity.y;
        if (result != 0)
        {
            result = character_pad.flags & 0xFFFF7FFF;
            character_pad.flags = result;
        }
    }
}

intptr AnimatedObject::UpdateActorAimedShot()
{
    FUNCTION_MARKER(0x8001EDAC, "SLUS_005.78");
    ActorBinding *cur_state;
    ActorModelDefinition *resource_model;
    Vec3 origin;
    Vec3 target_pos;
    uint32 distance_output;
    Vec3 *target = &target_pos;
    uint32 cur_flags = flags, was_end = cur_flags & 0x800;
    uint32 mag, result;
    AnimationHeader *clip;
    AnimatedObject *spawned;
    sint32 distance, sample_frame, next, count, velocity_y, absolute_y, root, factor;
    uint32 radius, half;
    if (was_end != 0)
    {
        flags = cur_flags & 0xFFFFF7FF;
        path_frame = path_definition->pos_track.frame_count;
    }
    cur_state = (ActorBinding *)(((const ActorPlacement *)path_definition)->state);
    ActorBinding *binding = cur_state;
    resource_model = binding->model;
    UpdateActorStateAndTarget(origin, *target, &distance_output);
    if ((binding->flags & 2) != 0)
    {
        target = 0;
        goto finish;
    }
    if ((path_definition->flags & 0x40) == 0)
    {
        flags = flags & ~0x40u;
        goto choose_animation;
    }
    if (state_delay != 0)
        state_delay = state_delay - 1;
    distance = distance_output;
    if (animation != resource_model->animations[2])
    {
        if ((resource_model->target_range[0] >= distance && path_frame != 0) || resource_model->target_range[1] < distance)
            goto motion;
        if (state_delay != 0)
            goto finish;
    }
    if ((flags & 0x30000000) != 0 || behavior == 17)
        goto motion;
    if (animation != resource_model->animations[2])
    {
        flags = flags & ~0x40u;
        {
            AnimationHeader *animation_val = resource_model->animations[2];
            AnimatedObject *animation_object = this;
            uint32 animation_flags = animation_object->flags;
            animation_object->animation = animation_val;
            animation_object->frame = -1;
            animation_object->flags = animation_flags | 0x10000;
        }
    }
    clip = animation;
    next = clip != 0 && frame < clip->frame_count - 1 ? frame + 1 : 0;
    if (next == resource_model->shot_frame)
    {
        state = 23;
        flags = flags | 0x100;
        spawned = object_operations.behaviors[6].factory_handler(this);
        flags = flags & ~0x100u;
        if (spawned != 0)
        {
            {
                const Vec3 *point_src = &spawned->pos;
                Vec3 *point_dst = &spawned->pos;
                const PathDefinition *point_definition = spawned->path_definition;
                MATRIX *point_matrix;

                uint32 point_x = (uint32)point_src->x, point_y = (uint32)point_src->y, point_z = point_src->z;
                if (point_definition != 0 && point_definition->transform != 0)
                {
                    VECTOR point_input;

                    point_input.vx = point_x;
                    point_input.vy = point_y;
                    point_input.vz = point_z;
                    PushMatrix();
                    point_definition = spawned->path_definition;
                    point_matrix = point_definition->transform != 0 ? &point_definition->transform->matrix : NULL;
                    SetRotMatrix(point_matrix);

                    {
                        VECTOR rotation_output;
                        ApplyRotMatrixLV(&point_input, &rotation_output);
                        point_dst->x = rotation_output.vx;
                        point_dst->y = rotation_output.vy;
                        point_dst->z = rotation_output.vz;
                    }
                    PopMatrix();
                    point_definition = spawned->path_definition;
                    point_matrix = point_definition->transform != 0 ? &point_definition->transform->matrix : NULL;
                    point_dst->x = (uint32)point_dst->x + point_matrix->t[0];
                    point_definition = spawned->path_definition;
                    point_matrix = point_definition->transform != 0 ? &point_definition->transform->matrix : NULL;
                    point_dst->y = (uint32)point_dst->y + point_matrix->t[1];
                    point_definition = spawned->path_definition;
                    point_matrix = point_definition->transform != 0 ? &point_definition->transform->matrix : NULL;
                    point_dst->z = (uint32)point_dst->z + point_matrix->t[2];
                }
                else
                {
                    point_dst->x = point_x;
                    point_dst->y = point_y;
                    point_dst->z = point_z;
                }
            }
            spawned->movement_mode = 0;
            spawned->flags = spawned->flags | 0x2000;
            const AnimatedObject *projectile_player = CharacterController::FindPlayerObject(0);
            spawned->velocity.x = target->x - (uint32)spawned->pos.x;
            spawned->velocity.y = target->y - (uint32)spawned->pos.y;
            spawned->velocity.z = target->z - (uint32)spawned->pos.z;
            velocity_y = (uint32)spawned->velocity.y - 12288 + projectile_player->bounds.height;
            spawned->velocity.y = velocity_y;
            absolute_y = velocity_y < 0 ? (sint32)(-(uint32)velocity_y) : velocity_y;
            mag = (uint32)mul12(spawned->velocity.x, spawned->velocity.x) + (uint32)mul12(spawned->velocity.z, spawned->velocity.z);
            root = (sint32)mag < 0 ? (sint32)(2u * SquareRoot12((mag >> 2))) : SquareRoot12(mag);
            distance = root >> 1;
            distance_output = distance;
            if (absolute_y >= distance)
            {
                factor = ((sint64)distance * (0x40000000 / absolute_y)) >> 18;
                fixed_math_result = factor;
                spawned->velocity.y = mul12(velocity_y, factor);
            }
        }
        state_delay = resource_model->shot_interval;
        goto finish;
    }
    sample_frame = frame;
    if (sample_frame <= 0 || (clip != 0 && sample_frame < clip->frame_count - 1))
    {
        goto finish;
    }

    {
        AnimationHeader *animation_val = resource_model->animations[0];
        AnimatedObject *animation_object = this;
        uint32 animation_flags = animation_object->flags;
        animation_object->animation = animation_val;
        animation_object->frame = -1;
        animation_object->flags = animation_flags | 0x10000;
    }
    if (path_frame == 0 && path_definition->pos_track.keys != 0)
        path_frame = 1;
    goto finish;
motion:
    if (state_delay != 0)
        goto finish;
    if (resource_model->target_range[0] < distance && resource_model->target_range[1] >= distance)
        goto finish;
    if (distance < 10001)
        next = path_frame + ClassifyActorPathDir(origin, *target);
    else
    {
        if (was_end != 0)
        {
            uint32 sample_path_frame = (uint16)path_frame;
            uint32 path_count = (uint16)path_definition->pos_track.frame_count;
            AnimatedObject *path_object = this;
            path_object->flags = (path_object->flags & 0xFFFFF7FF) ^ 0x10;
            path_frame = path_count - sample_path_frame;
        }
        next = path_frame + 1;
    }
    sample_frame = path_frame;
    count = path_definition->pos_track.frame_count;
    if (next < sample_frame)
    {
        if (resource_model->target_range[1] < distance)
        {
            radius = resource_model->patrol_radius;
            half = resource_model->patrol_radius >> 1;
            flags = flags | 0x40;
            if (sample_frame < count)
            {
                if (radius * 2 >= target->y - (uint32)world_pos.y + radius && (half * 2 < origin.x - (uint32)target->x + half || half * 2 < origin.z - (uint32)target->z + half))
                {
                    uint32 sample_path_frame0 = (uint16)path_frame;
                    uint32 path_count = (uint16)path_definition->pos_track.frame_count;
                    AnimatedObject *path_object = this;
                    path_object->flags = (path_object->flags & 0xFFFFF7FF) ^ 0x10;
                    path_frame = path_count - sample_path_frame0;
                }
            }
            else if (radius * 2 < origin.x - (uint32)target->x + radius || radius * 2 < origin.z - (uint32)target->z + radius)
            {
                uint32 sample_path_frame1 = (uint16)path_frame;
                uint32 path_count = (uint16)path_definition->pos_track.frame_count;
                AnimatedObject *path_object = this;
                path_object->flags = (path_object->flags & 0xFFFFF7FF) ^ 0x10;
                path_frame = path_count - sample_path_frame1;
            }
            else
                flags = flags & ~0x40u;
        }
        else if (sample_frame >= count - 1)
        {
            path_frame = 0;
            flags = (flags ^ 0x10) & ~0x40u;
        }
        else
            flags = flags | 0x40;
    }
    else if (resource_model->target_range[1] >= distance)
    {
        if (sample_frame >= 2)
        {
            {
                uint32 sample_path_frame2 = (uint16)path_frame;
                uint32 path_count = (uint16)path_definition->pos_track.frame_count;
                AnimatedObject *path_object = this;
                path_object->flags = (path_object->flags & 0xFFFFF7FF) ^ 0x10;
                path_frame = path_count - sample_path_frame2;
            }
            flags = flags | 0x40;
        }
        else
        {
            path_frame = 0;
            flags = flags & ~0x40u;
        }
    }
    else
        flags = sample_frame >= count ? flags & ~0x40u : flags | 0x40;
choose_animation:
    clip = resource_model->animations[((flags & 0x40) != 0 ? 3 : 0)];
    if (animation != clip)
    {
        AnimationHeader *animation_val = clip;
        AnimatedObject *animation_object = this;
        uint32 animation_flags = animation_object->flags;
        animation_object->animation = animation_val;
        animation_object->frame = -1;
        animation_object->flags = animation_flags | 0x10000;
    }
finish:
    result = UpdateActorFacingAndAnimation(target, origin, resource_model);

    return result;
}

intptr AnimatedObject::InitializeActorAimedShot()
{
    FUNCTION_MARKER(0x8001F6AC, "SLUS_005.78");
    if (!object_operations.behaviors[18].plain_handler && !object_operations.behaviors[18].member_handler && !object_operations.behaviors[18].void_member_handler && !object_operations.behaviors[18].factory_handler)
    {
        ObjectCallback operation = {0, &AnimatedObject::UpdateActorAimedShot};
        object_operations.behaviors[18] = operation;
    }
    behavior = 18;
    return 18;
}

intptr AnimatedObject::UpdateVolleyEnemy()
{
    typedef uint32 (AnimatedObject::*VolleyHandler)(const Vec3 &, const Vec3 &, sint32, ActorModelDefinition *, const VolleyScript *);
    static const VolleyHandler volley_handlers[4] = {&AnimatedObject::UpdateVolleyFiring, &AnimatedObject::UpdateVolleyFiring, &AnimatedObject::UpdateVolleyPath, &AnimatedObject::UpdateVolleyFiring};
    FUNCTION_MARKER(0x8001F6DC, "SLUS_005.78");
    ActorModelDefinition *bound_model;
    bound_model = ((const ActorBinding *)(((const ActorPlacement *)path_definition)->state))->model;
    sint32 distance;
    uint32 result;
    AnimationHeader *clip;
    Vec3 origin;
    Vec3 target;
    uint32 distance_output;
    const VolleyScript *script;
    uint32 delta;
    sint32 kind;
    sint32 step;
    UpdateActorStateAndTarget(origin, target, &distance_output);
    delta = (sint32)(origin.x - (uint32)target.x) >> 12;
    distance = delta * delta;
    distance_output = distance;
    if ((path_definition->flags & 0x40) != 0 && volley_enemy.skip_update == 0)
    {
        if (state_delay != 0)
            state_delay = state_delay - 1;
        if (volley_enemy.delay_ticks != 0)
            volley_enemy.delay_ticks = volley_enemy.delay_ticks - 1;
        script = &volley_enemy.scripts[volley_enemy.script_idx];
        result = (this->*volley_handlers[script->mode])(origin, target, distance, bound_model, script);
        if (result == 1)
        {
            step = volley_enemy.script_idx;
            while (ticks < volley_enemy.scripts[step].health_threshold)
            {
                if (step >= (sint32)volley_enemy.script_count - 1)
                    break;
                volley_enemy.script_idx = (uint32)step + 1;
                VolleyEnemyState::ResetVolleyScript();
                step = volley_enemy.script_idx;
                kind = volley_enemy.scripts[step].mode;
                volley_enemy.delay_ticks = 30;
                if (kind == 2)
                    break;
            }
        }
    }
    else
    {
        flags = flags & 0xFFFFFFBF;
        if (animation != bound_model->animations[3])
        {
            AnimationHeader *animation_val = bound_model->animations[3];
            uint32 animation_flags = flags;
            animation = animation_val;
            frame = -1;
            flags = animation_flags | 0x10000;
        }
        volley_enemy.skip_update = 0;
    }
    script = &volley_enemy.scripts[volley_enemy.script_idx];
    kind = script->mode;
    if (kind >= 0 && kind < 2)
        target.x = origin.x;
    else if (kind == 2 && animation == bound_model->animations[1])
    {
        sint32 marker = script->frames[volley_enemy.frame_idx];
        if (frame == marker)
            volley_enemy.target_x = target.x;
        else if (marker < frame)
            target.x = volley_enemy.target_x;
    }
    clip = animation;
    if (clip == bound_model->animations[0])
        target.x = origin.x;
    result = UpdateActorFacingAndAnimation(&target, origin, bound_model);
    return result;
}

void AnimatedObject::SpawnVolleyProjectile(const VolleyScript *script, const Vec3 &target, sint32 idx)
{
    sint32 axis_component1;
    sint32 n;

    FUNCTION_MARKER(0x8001FA24, "SLUS_005.78");

    VECTOR input;

    AnimatedObject *spawned;
    uint32 type;
    uint32 mag;
    sint32 len;
    const PathDefinition *definition;
    PathTransform *path_transform;
    sint32 delta, degrees, angle, factor;

    if (scene_events.script_vals[15] >= 5)
        return;
    state = script->projectile_kind;
    flags = flags | 0x100;
    spawned = object_operations.behaviors[6].factory_handler(this);
    flags = flags & 0xFFFFFEFF;
    /* A failed factory must not write a projectile through address zero */
    if (spawned == 0)
        return;
    spawned->animation_binding.tracks[0] = -3;
    definition = spawned->path_definition;
    if (definition != 0 && definition->transform != 0)
    {
        input.vx = spawned->pos.x;
        input.vy = spawned->pos.y;
        input.vz = spawned->pos.z;
        PushMatrix();
        path_transform = spawned->path_definition->transform;
        SetRotMatrix(&path_transform->matrix);
        {
            VECTOR rotation_output;
            ApplyRotMatrixLV(&input, &rotation_output);
            spawned->pos.x = rotation_output.vx;
            spawned->pos.y = rotation_output.vy;
            spawned->pos.z = rotation_output.vz;
        }
        PopMatrix();
        path_transform = spawned->path_definition->transform;
        spawned->pos.x = (uint32)spawned->pos.x + path_transform->matrix.t[0];
        path_transform = spawned->path_definition->transform;
        spawned->pos.y = (uint32)spawned->pos.y + path_transform->matrix.t[1];
        path_transform = spawned->path_definition->transform;
        spawned->pos.z = (uint32)spawned->pos.z + path_transform->matrix.t[2];
    }
    else
    {
        uint32 x = (uint32)spawned->pos.x, y = (uint32)spawned->pos.y, z = spawned->pos.z;
        spawned->pos.x = x;
        spawned->pos.y = y;
        spawned->pos.z = z;
    }
    type = script->mode;
    if (type == 0 || type == 1 || type == 3)
    {
        uint32 y = (uint32)spawned->pos.y, x = spawned->pos.x;
        spawned->pos.y = y + 172032;
        spawned->pos.x = x + 61440;
    }
    spawned->movement_mode = 0;
    spawned->flags = spawned->flags | 0x2000;
    type = script->mode;
    if (type < 2)
    {
        spawned->velocity.x = 0;
        spawned->velocity.y = 24576;
        spawned->velocity.z = target.z - (uint32)spawned->pos.z;
    }
    else if (type == 3)
    {
        delta = target.z - (uint32)spawned->pos.z;
        degrees = script->angles[idx];
        angle = (sint32)((degrees < 0 ? -(uint32)degrees : (uint32)degrees) << 12) / 360;
        factor = SinQ14(angle) >> 2;
        mag = mul12(factor, delta);
        spawned->velocity.x = degrees < 0 ? -mag : mag;
        spawned->velocity.y = 24576;
        factor = SinQ14((1024 - angle)) >> 2;
        spawned->velocity.z = mul12(factor, delta);
    }
    mag = mul12(spawned->velocity.x, spawned->velocity.x)
        + mul12(spawned->velocity.y, spawned->velocity.y)
        + mul12(spawned->velocity.z, spawned->velocity.z);
    len = (sint32)mag >= 0 ? SquareRoot12(mag) : SquareRoot12((mag >> 2)) << 1;
    len = len >> 1;
    factor = 0x40000000 / len;
    axis_component1 = spawned->velocity.x;
    n = ((sint64)axis_component1 * factor) >> 18;
    fixed_math_result = n;
    delta = mul12(n, 122880);
    spawned->pos.x = (uint32)spawned->pos.x + delta;
    axis_component1 = spawned->velocity.y;
    n = ((sint64)axis_component1 * factor) >> 18;
    fixed_math_result = n;
    delta = mul12(n, 122880);
    spawned->pos.y = (uint32)spawned->pos.y + delta;
    axis_component1 = spawned->velocity.z;
    n = ((sint64)axis_component1 * factor) >> 18;
    fixed_math_result = n;
    delta = mul12(n, 122880);
    spawned->pos.z = (uint32)spawned->pos.z + delta;
    state_delay = script->shot_delay;
}

uint32 AnimatedObject::UpdateVolleyPath(const Vec3 &origin, const Vec3 &target, sint32 distance, ActorModelDefinition *resource_model, const VolleyScript *script)
{
    FUNCTION_MARKER(0x80020094, "SLUS_005.78");
    AnimationHeader *clip, *attack, *next_animation;
    uint32 cur_flags;
    sint32 sample_frame, cur, next;
    if (volley_enemy.reset_pending != 0)
    {
        flags = flags & 0xFFFFFFBF;
        {
            AnimationHeader *animation_val = resource_model->animations[0];
            uint32 animation_flags = flags;
            animation = animation_val;
            frame = -1;
            flags = animation_flags | 0x10000;
        }
        volley_enemy.reset_pending = 0;
        volley_enemy.path_active = 0;
        volley_enemy.event_started = 0;
        return 0;
    }
    clip = animation;
    if (clip == resource_model->animations[0])
    {
        sample_frame = frame;
        if (clip != 0 && sample_frame < clip->frame_count - 1 && sample_frame + 1 >= 40 && volley_enemy.event_started == 0)
        {
            volley_enemy.event_started = 1;
            scene_events.pending = volley_enemy.event_id | 0x1000;
        }
        if (frame >= animation->frame_count - 1)
        {
            next = script->frames[volley_enemy.frame_idx];
            if ((flags & 0x10) != 0)
                next = (uint32)path_definition->pos_track.frame_count - next;
            volley_enemy.target_path_frame = next;
            volley_enemy.path_active = 1;
            volley_enemy.prev_path_frame = path_frame;
        }
    }
    if (volley_enemy.path_active != 0)
    {
        clip = animation;
        attack = resource_model->animations[1];
        if (clip == attack)
        {
            if (frame >= attack->frame_count - 1)
            {
                volley_enemy.frame_idx = volley_enemy.frame_idx + 1;
                next_animation = resource_model->animations[((flags & 0x10) != 0 ? 4 : 3)];
                {
                    AnimationHeader *animation_val = next_animation;
                    uint32 animation_flags = flags;
                    animation = animation_val;
                    frame = -1;
                    flags = animation_flags | 0x10000;
                }
            }
            next = script->frames[volley_enemy.frame_idx];
            if ((flags & 0x10) != 0)
                next = (uint32)path_definition->pos_track.frame_count - next;
            volley_enemy.target_path_frame = next;
        }
        else
        {
            if (distance < script->distance_sq || volley_enemy.approach_ticks == script->action_limit)
            {
                frame = -1;
                animation = attack;
                cur = path_frame;
                flags = (flags & 0xFFFEFFBF) | 0x10000;
                volley_enemy.approach_ticks = 0;
                volley_enemy.prev_path_frame = cur;
                return 0;
            }
            volley_enemy.approach_ticks = volley_enemy.approach_ticks + 1;
            cur_flags = flags;
            cur = path_frame;
            if (((cur_flags & 0x10) != 0 && target.x < origin.x) || ((cur_flags & 0x10) == 0 && origin.x < target.x))
                next = cur + 1;
            else
                next = cur - 1;
            volley_enemy.target_path_frame = next;
            cur = path_frame;
            if (next < cur)
            {
                if (cur > 0)
                {
                    flags = ((flags & 0xFFFFF7FF) ^ 0x10) | 0x40;
                    path_frame = (uint16)path_definition->pos_track.frame_count - cur;
                }
                else
                    flags = flags & 0xFFFFFFBF;
            }
            else if (cur < path_definition->pos_track.frame_count)
                flags = flags | 0x40;
            else
                flags = flags & 0xFFFFFFBF;
            next_animation = resource_model->animations[((flags & 0x10) != 0 ? 4 : 3)];
            if (animation != next_animation)
            {
                AnimationHeader *animation_val = next_animation;
                uint32 animation_flags = flags;
                animation = animation_val;
                frame = -1;
                flags = animation_flags | 0x10000;
            }
        }
        volley_enemy.prev_path_frame = path_frame;
    }
    if ((sint32)volley_enemy.frame_idx < script->frame_count)
        return 0;
    volley_enemy.frame_idx = 0;
    volley_enemy.path_active = 0;
    volley_enemy.event_started = 0;
    volley_enemy.reset_pending = 1;
    scene_events.pending = volley_enemy.event_id | 0x2000;
    return 1;
}

uint32 AnimatedObject::UpdateVolleyFiring(const Vec3 &origin, const Vec3 &target, sint32 distance, ActorModelDefinition *resource_model, const VolleyScript *script)
{
    FUNCTION_MARKER(0x800204CC, "SLUS_005.78");
    AnimationHeader *clip, *next_animation;
    uint32 kind, cur_flags;
    sint32 sample_frame, next_frame, count, idx;
    if (state_delay != 0 || volley_enemy.delay_ticks != 0)
        return 1;
    resource_model->motion_speed[0] = (sint32)(script->move_speed << 12) / 30;
    kind = script->mode;
    clip = animation;
    if (kind == 1 && (clip == resource_model->animations[5] || clip == resource_model->animations[6]))
    {
        next_frame = clip != 0 && frame < clip->frame_count - 1 ? frame + 1 : 0;
        count = script->frame_count;
        for (idx = 0; idx < count; ++idx)
        {
            if (script->frames[idx] == next_frame)
            {
                SpawnVolleyProjectile(script, target, idx);
                return 1;
            }
        }
    }
    else if (kind == 0 || kind == 3)
    {
        if (clip == resource_model->animations[2] || script->distance_sq < distance || volley_enemy.motion_ticks == script->action_limit)
        {
            if (clip != resource_model->animations[2])
            {
                flags = flags & 0xFFFFFFBF;
                {
                    AnimationHeader *animation_val = resource_model->animations[2];
                    uint32 animation_flags = flags;
                    animation = animation_val;
                    frame = -1;
                    flags = animation_flags | 0x10000;
                }
            }
            clip = animation;
            next_frame = clip != 0 && frame < clip->frame_count - 1 ? frame + 1 : 0;
            count = script->frame_count;
            for (idx = 0; idx < count; ++idx)
            {
                if (script->frames[idx] == next_frame)
                {
                    SpawnVolleyProjectile(script, target, idx);
                    volley_enemy.turn_pending = 1;
                    volley_enemy.motion_ticks = 0;
                    return 1;
                }
            }
            clip = animation;
            sample_frame = frame;
            if ((clip == 0 || sample_frame >= clip->frame_count - 1) && sample_frame > 0)
            {
                {
                    AnimationHeader *animation_val = resource_model->animations[0];
                    uint32 animation_flags = flags;
                    animation = animation_val;
                    frame = -1;
                    flags = animation_flags | 0x10000;
                }
                if (path_frame == 0 && path_definition->pos_track.keys != 0)
                    path_frame = 1;
            }
            return 1;
        }
    }
    if (state_delay != 0)
        return 1;
    kind = script->mode;
    if (kind == 1)
    {
        flags = flags | 0x40;
        count = volley_enemy.motion_ticks + 1;
        volley_enemy.motion_ticks = count;
        if (count > script->turn_ticks)
        {
            uint32 random = randPSX() & 1;
            volley_enemy.turn_pending = 0;
            cur_flags = flags;
            if ((random != 0 && (cur_flags & 0x10) != 0) || (random == 0 && (cur_flags & 0x10) == 0))
            {
                uint32 sample_path_frame = (uint16)path_frame;
                uint32 path_count = (uint16)path_definition->pos_track.frame_count;
                flags = (flags & 0xFFFFF7FF) ^ 0x10;
                path_frame = path_count - sample_path_frame;
            }
            volley_enemy.motion_ticks = 0;
        }
        sample_frame = path_frame;
        count = path_definition->pos_track.frame_count;
        if (sample_frame >= count - 1)
        {
            path_frame = count - 1;
            {
                uint32 sample_path_frame0 = (uint16)path_frame;
                uint32 path_count = (uint16)path_definition->pos_track.frame_count;
                flags = (flags & 0xFFFFF7FF) ^ 0x10;
                path_frame = path_count - sample_path_frame0;
            }
        }
        next_animation = resource_model->animations[((flags & 0x10) != 0 ? 6 : 5)];
        if (animation != next_animation)
        {
            AnimationHeader *animation_val = next_animation;
            uint32 animation_flags = flags;
            animation = animation_val;
            frame = -1;
            flags = animation_flags | 0x10000;
        }
    }
    else if ((kind == 0 || kind == 3) && script->distance_sq >= distance)
    {
        count = volley_enemy.motion_ticks + 1;
        volley_enemy.motion_ticks = count;
        if (count >= (sint32)script->action_limit)
            return 1;
        flags = flags | 0x40;
        if (volley_enemy.turn_pending != 0)
        {
            uint32 random = randPSX() & 1;
            volley_enemy.turn_pending = 0;
            cur_flags = flags;
            if ((random != 0 && (cur_flags & 0x10) != 0) || (random == 0 && (cur_flags & 0x10) == 0))
            {
                uint32 sample_path_frame1 = (uint16)path_frame;
                uint32 path_count = (uint16)path_definition->pos_track.frame_count;
                flags = (flags & 0xFFFFF7FF) ^ 0x10;
                path_frame = path_count - sample_path_frame1;
            }
        }
        count = path_definition->pos_track.frame_count;
        if (path_frame >= count - 1)
        {
            path_frame = count - 1;
            {
                uint32 sample_path_frame2 = (uint16)path_frame;
                uint32 path_count = (uint16)path_definition->pos_track.frame_count;
                flags = (flags & 0xFFFFF7FF) ^ 0x10;
                path_frame = path_count - sample_path_frame2;
            }
        }
        if (path_frame == 0)
        {
            uint32 sample_path_frame3 = (uint16)path_frame;
            uint32 path_count = (uint16)path_definition->pos_track.frame_count;
            flags = (flags & 0xFFFFF7FF) ^ 0x10;
            path_frame = path_count - sample_path_frame3;
        }
        next_animation = resource_model->animations[((flags & 0x10) != 0 ? 4 : 3)];
        if (animation != next_animation)
        {
            AnimationHeader *animation_val = next_animation;
            uint32 animation_flags = flags;
            animation = animation_val;
            frame = -1;
            flags = animation_flags | 0x10000;
        }
    }
    return 1;
}

intptr AnimatedObject::InitializeVolleyEnemy()
{
    FUNCTION_MARKER(0x80020C08, "SLUS_005.78");
    {
        ObjectCallback operation = {0, &AnimatedObject::UpdateVolleyEnemy};
        object_operations.behaviors[30] = operation;
    }
    behavior = 30;
    volley_enemy.event_id = SceneEvents::EncodeObjectName(74, 80);
    volley_enemy.delay_ticks = 30;
    volley_enemy.script_idx = 0;
    volley_enemy.motion_ticks = 0;
    volley_enemy.approach_ticks = 0;
    volley_enemy.frame_idx = 0;
    volley_enemy.reset_pending = 1;
    volley_enemy.skip_update = 1;
    return 1;
}

intptr AnimatedObject::UpdateAttackEnemy()
{
    FUNCTION_MARKER(0x80020C6C, "SLUS_005.78");
    ActorModelDefinition *bound_model;
    bound_model = ((const ActorBinding *)(((const ActorPlacement *)path_definition)->state))->model;
    uint32 result;
    Vec3 origin;
    Vec3 target;
    uint32 distance_output;
    const AttackScript *script;
    uint32 delta;
    sint32 state;
    if ((flags & 0x10) == 0 && ticks > 0)
    {
        uint32 x = (uint32)velocity.x, y = (uint32)velocity.y, z = velocity.z;
        velocity.x = -x;
        velocity.y = -y;
        velocity.z = -z;
    }
    UpdateActorStateAndTarget(origin, target, &distance_output);
    if (ticks <= 0)
    {
        frame = 40;
        velocity.z = 0;
        velocity.x = -4096;
        animation = bound_model->animations[9];
        flags = (flags | 0x10) & 0xFFF7FFFF;
    }
    else
    {
        delta = (sint32)(origin.x - (uint32)target.x) >> 12;
        distance_output = delta * delta;
        if (state_delay != 0)
            state_delay = state_delay - 1;
        script = &attack_enemy.scripts[attack_enemy.script_idx];
        state = script->state;
        if (state == 0)
            result = UpdateAttackRepeat(origin, target, distance_output, bound_model, script);
        else if (state == 1)
            result = UpdateAttackAnimation(bound_model, script);
        else
            result = UpdateAttackApproach(origin, target, distance_output, bound_model, script);
        if (result == 1)
        {
            state = attack_enemy.script_idx;
            attack_enemy.script_idx = state < (sint32)attack_enemy.script_count - 1 ? (uint32)state + 1 : 0;
        }
        script = &attack_enemy.scripts[attack_enemy.script_idx];
        if (script->state != 0)
            target.x = origin.x;
        else if (animation == bound_model->animations[script->animation_idx])
        {
            if (frame == script->marker_frame)
                attack_enemy.target_x = target.x;
            else if (script->marker_frame < frame)
                target.x = attack_enemy.target_x;
        }
    }
    target.x = origin.x;
    result = UpdateActorFacingAndAnimation(&target, origin, bound_model);
    return result;
}

uint32 AnimatedObject::UpdateAttackRepeat(const Vec3 &origin, const Vec3 &target, sint32 distance, ActorModelDefinition *resource_model, const AttackScript *script)
{
    FUNCTION_MARKER(0x80020F44, "SLUS_005.78");
    AnimationHeader *clip, *attack, *next_animation;
    uint32 cur_flags;
    sint32 sample_frame, next;
    if (script->animation_idx == 6 && attack_enemy.special_pending != 0)
    {
        flags = flags & 0xFFFFFFBF;
        {
            AnimationHeader *animation_val = resource_model->animations[5];
            uint32 animation_flags = flags;
            animation = animation_val;
            frame = -1;
            flags = animation_flags | 0x10000;
        }
        attack_enemy.special_pending = 0;
        scene_events.pending = attack_enemy.special_event_id | 0x1000;
        return 0;
    }
    clip = animation;
    attack = resource_model->animations[script->animation_idx];
    if (clip == attack)
    {
        if (frame >= clip->frame_count - 1)
        {
            attack_enemy.repeat_count = attack_enemy.repeat_count + 1;
            next_animation = resource_model->animations[((flags & 0x10) != 0 ? 8 : 7)];
            if (animation != next_animation)
            {
                AnimationHeader *animation_val = next_animation;
                uint32 animation_flags = flags;
                animation = animation_val;
                frame = -1;
                flags = animation_flags | 0x10000;
            }
        }
    }
    else
    {
        if ((clip == resource_model->animations[5] || clip == resource_model->animations[10]) && frame < clip->frame_count - 1)
            return 0;
        if (distance < script->distance_sq || attack_enemy.approach_ticks == script->approach_ticks)
        {
            frame = -1;
            animation = attack;
            flags = (flags & 0xFFFEFFBF) | 0x10000;
            attack_enemy.approach_ticks = 0;
            return 0;
        }
        attack_enemy.approach_ticks = attack_enemy.approach_ticks + 1;
        cur_flags = flags | 0x40;
        flags = cur_flags;
        sample_frame = path_frame;
        next = (((cur_flags & 0x10) != 0 && target.x < origin.x) || ((cur_flags & 0x10) == 0 && origin.x < target.x)) ? sample_frame + 1 : sample_frame - 1;
        attack_enemy.next_path_frame = next;
        if (path_frame > 0 && next < path_frame)
        {
            uint32 sample_path_frame = (uint16)path_frame;
            uint32 path_count = (uint16)path_definition->pos_track.frame_count;
            flags = (flags & 0xFFFFF7FF) ^ 0x10;
            path_frame = path_count - sample_path_frame;
        }
        next_animation = resource_model->animations[((flags & 0x10) != 0 ? 8 : 7)];
        if (animation != next_animation)
        {
            AnimationHeader *animation_val = next_animation;
            uint32 animation_flags = flags;
            animation = animation_val;
            frame = -1;
            flags = animation_flags | 0x10000;
        }
    }
    if ((sint32)attack_enemy.repeat_count < script->repetitions)
        return 0;
    attack_enemy.repeat_count = 0;
    if (script->animation_idx == 6)
    {
        attack_enemy.special_pending = 1;
        flags = flags & 0xFFFFFFBF;
        {
            AnimationHeader *animation_val = resource_model->animations[5];
            uint32 animation_flags = flags;
            animation = animation_val;
            frame = -1;
            flags = animation_flags | 0x10000;
        }
        scene_events.pending = attack_enemy.special_event_id | 0x2000;
    }
    return 1;
}

uint32 AnimatedObject::UpdateAttackApproach(const Vec3 &origin, const Vec3 &target, sint32 distance, ActorModelDefinition *resource_model, const AttackScript *script)
{
    FUNCTION_MARKER(0x80021254, "SLUS_005.78");
    sint32 timer = attack_enemy.retreat_ticks, sample_frame, next;
    AnimationHeader *clip;
    uint32 cur_flags;
    attack_enemy.retreat_ticks = (uint32)timer + 1;
    if (timer >= (sint32)script->approach_ticks)
    {
        attack_enemy.retreat_ticks = 0;
        return 1;
    }
    cur_flags = flags | 0x40;
    flags = cur_flags;
    if (distance < script->distance_sq)
    {
        flags = cur_flags & 0xFFFFFFBF;
        {
            AnimationHeader *animation_val = resource_model->animations[0];
            uint32 animation_flags = flags;
            animation = animation_val;
            frame = -1;
            flags = animation_flags | 0x10000;
        }
        return 0;
    }
    sample_frame = path_frame;
    next = (((cur_flags & 0x10) != 0 && target.x < origin.x) || ((cur_flags & 0x10) == 0 && origin.x < target.x)) ? sample_frame + 1 : sample_frame - 1;
    if (sample_frame > 0 && next < sample_frame)
    {
        uint32 sample_path_frame = (uint16)path_frame;
        uint32 path_count = (uint16)path_definition->pos_track.frame_count;
        flags = (flags & 0xFFFFF7FF) ^ 0x10;
        path_frame = path_count - sample_path_frame;
    }
    clip = resource_model->animations[((flags & 0x10) != 0 ? 8 : 7)];
    if (animation != clip)
    {
        AnimationHeader *animation_val = clip;
        uint32 animation_flags = flags;
        animation = animation_val;
        frame = -1;
        flags = animation_flags | 0x10000;
    }
    return 0;
}

uint32 AnimatedObject::UpdateAttackAnimation(ActorModelDefinition *resource_model, const AttackScript *script)
{
    FUNCTION_MARKER(0x800213D8, "SLUS_005.78");
    AnimationHeader *clip, *sel;
    if (attack_enemy.event_pending != 0)
    {
        flags = flags & 0xFFFFFFBF;
        {
            AnimationHeader *animation_val = resource_model->animations[11];
            uint32 animation_flags = flags;
            animation = animation_val;
            frame = -1;
            flags = animation_flags | 0x10000;
        }
        attack_enemy.event_pending = 0;
        scene_events.pending = attack_enemy.event_id | 0x1000;
        return 0;
    }
    clip = animation;
    sel = resource_model->animations[attack_enemy.scripts[attack_enemy.script_idx].animation_idx];
    if (clip == sel)
    {
        if (frame >= clip->frame_count - 1)
            attack_enemy.repeat_count = attack_enemy.repeat_count + 1;
    }
    else if (frame == clip->frame_count - 1)
    {
        AnimationHeader *animation_val = sel;
        uint32 animation_flags = flags;
        animation = animation_val;
        frame = -1;
        flags = animation_flags | 0x10000;
    }
    if ((sint32)attack_enemy.repeat_count < script->repetitions)
        return 0;
    flags = flags & 0xFFFFFFBF;
    sel = resource_model->animations[10];
    attack_enemy.repeat_count = 0;
    attack_enemy.event_pending = 1;
    frame = -1;
    scene_events.pending = attack_enemy.event_id | 0x2000;
    animation = sel;
    flags = flags | 0x10000;
    return 1;
}

intptr AnimatedObject::InitializeAttackEnemy()
{
    FUNCTION_MARKER(0x80021678, "SLUS_005.78");
    uint32 result;
    {
        ObjectCallback operation = {0, &AnimatedObject::UpdateAttackEnemy};
        object_operations.behaviors[30] = operation;
    }
    behavior = 30;
    attack_enemy.event_id = SceneEvents::EncodeObjectName(70, 71);
    result = SceneEvents::EncodeObjectName(117, 112);
    attack_enemy.special_event_id = result;
    attack_enemy.script_idx = 0;
    return result;
}

intptr AnimatedObject::UpdateActorChaseSpeed()
{
    FUNCTION_MARKER(0x800216CC, "SLUS_005.78");
    const ActorModelDefinition *speed_model;
    ActorBinding *binding;
    ActorBinding *state = (ActorBinding *)(((const ActorPlacement *)path_definition)->state);
    binding = state;
    uint32 cur_flags, dir, result;
    sint32 maximum, step, speed;
    if ((binding->flags & 10) != 0 || (flags & 0x40) == 0)
        return (this->*object_operations.motions[4].member_handler)();
    cur_flags = flags;
    dir = (cur_flags >> 4) & 1;
    speed_model = binding->model;
    if (animation == speed_model->animations[7])
    {
        maximum = speed_model->motion_speed[0];
        step = speed_model->motion_accel >> 2;
    }
    else
    {
        maximum = speed_model->motion_speed[1];
        step = speed_model->motion_accel;
    }
    speed = binding->motion_speed;
    if ((flags & 0x40000000) != 0)
    {
        speed = (uint32)speed - step;
        binding->motion_speed = speed;
        if (speed <= 0)
        {
            binding->motion_speed = 0;
            cur_flags = flags & 0xBFFFFFBF;
            flags = cur_flags;
            path_frame = (uint16)path_definition->pos_track.frame_count - (uint16)path_frame;
            flags = (flags & 0xFFFFF7FF) ^ 0x10;
        }
    }
    else if (speed < maximum)
    {
        speed = (uint32)speed + step;
        binding->motion_speed = speed;
        if (speed > maximum)
            binding->motion_speed = maximum;
    }
    (this->*object_operations.motions[4].member_handler)();
    cur_flags = flags;
    result = cur_flags & 0x40;
    if (result != 0)
    {
        state = (ActorBinding *)(((const ActorPlacement *)path_definition)->state);
        binding = state;
        result = 0x40000000;
        if (path_frame >= binding->path_parameters[dir != 0 ? 0 : 1])
        {
            result = cur_flags | 0x40000000;
            flags = result;
        }
    }
    return result;
}

intptr AnimatedObject::UpdateActorChaseAttack()
{
    FUNCTION_MARKER(0x80021888, "SLUS_005.78");
    ActorModelDefinition *resource_model;

    ActorBinding *binding = (ActorBinding *)(((const ActorPlacement *)path_definition)->state);

    resource_model = binding->model;
    uint32 result, event, cur_flags;
    AnimationHeader *clip;
    Vec3 origin;
    Vec3 target_pos;
    uint32 distance_output;
    Vec3 *target = &target_pos;
    sint32 close, sample_frame, proj;
    event = UpdateActorStateAndTarget(origin, *target, &distance_output);
    if ((binding->flags & 2) != 0)
    {
        target = 0;
        goto finish;
    }
    if ((binding->flags & 8) != 0)
        goto finish;
    close = resource_model->target_range[0] >= (sint32)distance_output && ((event & 0x1800000) != 0 || (binding->reaction_flags & 2) != 0);
    if ((path_definition->flags & 0x40) == 0)
    {
        if ((flags & 0x40) != 0)
            flags = flags & 0xFFFFFFBF;
        if (animation != resource_model->animations[0])
        {
            AnimationHeader *animation_val = resource_model->animations[0];
            AnimatedObject *animation_object = this;
            uint32 animation_flags = animation_object->flags;
            animation_object->animation = animation_val;
            animation_object->frame = -1;
            animation_object->flags = animation_flags | 0x10000;
        }
        goto finish;
    }
    clip = animation;
    if (close && clip != resource_model->animations[3] && clip != resource_model->animations[0] && clip != resource_model->animations[8])
        goto start_chase;
    if (clip == resource_model->animations[0])
    {
        if (close)
        {
            AnimationHeader *animation_val = resource_model->animations[3];
            AnimatedObject *animation_object = this;
            uint32 animation_flags = animation_object->flags;
            animation_object->animation = animation_val;
            animation_object->frame = -1;
            animation_object->flags = animation_flags | 0x10000;
        }
        else
        {
            sample_frame = clip != 0 && frame < clip->frame_count - 1 ? frame + 1 : 0;
            frame = sample_frame;
        }
        target = &origin;
        goto finish;
    }
    if (clip == resource_model->animations[2])
    {
        uint32 timer = state_delay - 1;
        state_delay = timer;
        if (timer == 0)
        {
            if ((flags & 0x40) == 0)
                flags = flags | 0x40;
            flags = flags | 0x40000000;
            {
                AnimationHeader *animation_val = resource_model->animations[7];
                AnimatedObject *animation_object = this;
                uint32 animation_flags = animation_object->flags;
                animation_object->animation = animation_val;
                animation_object->frame = -1;
                animation_object->flags = animation_flags | 0x10000;
            }
            goto finish;
        }
        goto chase;
    }
    if (clip == resource_model->animations[7])
    {
        if (binding->motion_speed == 0)
            goto stop_chase;
        if ((flags & 0x40) == 0)
            flags = flags | 0x40;
        goto finish;
    }
    if (clip == resource_model->animations[8])
    {
        sample_frame = frame;
        if ((clip == 0 || sample_frame >= clip->frame_count - 1) && sample_frame > 0)
        {
            AnimationHeader *animation_val = resource_model->animations[0];
            AnimatedObject *animation_object = this;
            uint32 animation_flags = animation_object->flags;
            animation_object->animation = animation_val;
            animation_object->frame = -1;
            animation_object->flags = animation_flags | 0x10000;
        }
        else
        {
            clip = animation;
            sample_frame = clip != 0 && frame < clip->frame_count - 1 ? frame + 1 : 0;
            frame = sample_frame;
        }
        target = &origin;
        goto finish;
    }
    if (clip == resource_model->animations[3])
    {
        sample_frame = frame;
        if ((clip == 0 || sample_frame >= clip->frame_count - 1) && sample_frame > 0)
            goto start_chase;
    }
    goto finish;
start_chase:
    state_delay = 240;
    if (animation != resource_model->animations[2])
    {
        AnimationHeader *animation_val = resource_model->animations[2];
        AnimatedObject *animation_object = this;
        uint32 animation_flags = animation_object->flags;
        animation_object->animation = animation_val;
        animation_object->frame = -1;
        animation_object->flags = animation_flags | 0x10000;
    }
chase:
    if ((sint32)distance_output < 10001)
        proj = path_definition->FindHeightFrame(*target, flags & 0x10, 409600);
    else
        proj = path_definition->pos_track.frame_count;
    sample_frame = path_frame;
    cur_flags = flags;
    if (proj >= sample_frame)
        flags = cur_flags & 0xBFFFFFFF;
    else if ((cur_flags & 0x40) != 0)
        flags = cur_flags | 0x40000000;
    else
    {
        flags = ((cur_flags & 0xFFFFF7FF) ^ 0x10) & 0xBFFFFFFF;
        path_frame = (uint16)path_definition->pos_track.frame_count - sample_frame;
    }
    cur_flags = flags;
    sample_frame = path_frame;
    if ((cur_flags & 0x40000000) == 0)
    {
        const sint16 *limit;
        if (sample_frame == 0 && proj == 0)
            goto stop_chase;
        limit = ((ActorBinding *)(((const ActorPlacement *)path_definition)->state))->path_parameters + ((cur_flags & 0x10) != 0 ? 0 : 1);
        if (sample_frame >= *limit && proj >= sample_frame)
            goto stop_chase;
    }
    flags = flags | 0x40;
    goto finish;
stop_chase:
    flags = flags & 0xFFFFFFBF;
    {
        AnimationHeader *animation_val = resource_model->animations[8];
        AnimatedObject *animation_object = this;
        uint32 animation_flags = animation_object->flags;
        animation_object->animation = animation_val;
        animation_object->frame = -1;
        animation_object->flags = animation_flags | 0x10000;
    }
    target_yaw = ((uint16)target_yaw + 1024) & 4095;
    target = &origin;
finish:
    result = UpdateActorFacingAndAnimation(target, origin, resource_model);

    return result;
}

intptr AnimatedObject::InitializeActorChaseAttack()
{
    FUNCTION_MARKER(0x80021E28, "SLUS_005.78");
    if (!object_operations.behaviors[19].plain_handler && !object_operations.behaviors[19].member_handler && !object_operations.behaviors[19].void_member_handler && !object_operations.behaviors[19].factory_handler)
    {
        {
            ObjectCallback operation = {0, &AnimatedObject::UpdateActorChaseSpeed};
            object_operations.motions[6] = operation;
        }
        {
            ObjectCallback operation = {0, &AnimatedObject::UpdateActorChaseAttack};
            object_operations.behaviors[19] = operation;
        }
    }
    behavior = 19;
    movement_mode = 6;
    return 6;
}

intptr AnimatedObject::UpdateActorVerticalShot()
{

    FUNCTION_MARKER(0x80021E70, "SLUS_005.78");
    ActorModelDefinition *resource_model;
    ActorBinding *binding = (ActorBinding *)(((const ActorPlacement *)path_definition)->state);

    resource_model = binding->model;
    uint32 result, timer;
    AnimationHeader *clip;
    AnimatedObject *spawned;
    Vec3 origin;
    Vec3 target_pos;
    uint32 distance_output;
    Vec3 *target = &target_pos;
    sint32 next_frame, sample_frame, next, forward_distance, backward_distance;
    UpdateActorStateAndTarget(origin, *target, &distance_output);
    if ((binding->flags & 2) != 0)
    {
        target = 0;
        goto finish;
    }
    timer = state_delay;
    if (timer != 0)
    {
        state_delay = --timer;
        if (timer != 0)
            goto finish;
    }
    clip = animation;
    next_frame = clip != 0 && frame < clip->frame_count - 1 ? frame + 1 : 0;
    if (next_frame != resource_model->shot_frame || (path_definition->flags & 0x40) == 0 || (flags & 0x30000000) != 0 || behavior == 17 || (sint32)distance_output >= 10001)
        goto finish;
    next = ClassifyActorPathDir(origin, *target);
    sample_frame = path_frame;
    next += sample_frame;
    if (next < sample_frame)
    {
        uint32 sample_path_frame = (uint16)path_frame;
        uint32 path_count = (uint16)path_definition->pos_track.frame_count;
        AnimatedObject *path_object = this;
        path_object->flags = (path_object->flags & 0xFFFFF7FF) ^ 0x10;
        path_frame = path_count - sample_path_frame;
    }
    else if (next == sample_frame)
    {
        if (next >= path_definition->pos_track.frame_count - 1)
            forward_distance = 0x80001000;
        else
        {
            Vec3 probe;
            uint32 probe_sum = 0;
            sint32 probe_frame = next + 1;
            path_definition->SamplePos(probe_frame, probe, flags & 0x10);
            probe.x = probe.x - (uint32)target->x;
            probe.y = probe.y - (uint32)target->y;
            probe.z = probe.z - (uint32)target->z;
            probe.x >>= 12;
            probe.y >>= 12;
            probe.z >>= 12;
            probe_sum += (uint32)probe.x * probe.x + (uint32)probe.y * probe.y + (uint32)probe.z * probe.z;
            forward_distance = probe_sum;
        }
        if (next <= 0)
            backward_distance = 0x7FFFF000;
        else
        {
            Vec3 probe;
            uint32 probe_sum = 0;
            sint32 probe_frame = next - 1;
            path_definition->SamplePos(probe_frame, probe, flags & 0x10);
            probe.x = probe.x - (uint32)target->x;
            probe.y = probe.y - (uint32)target->y;
            probe.z = probe.z - (uint32)target->z;
            probe.x >>= 12;
            probe.y >>= 12;
            probe.z >>= 12;
            probe_sum += (uint32)probe.x * probe.x + (uint32)probe.y * probe.y + (uint32)probe.z * probe.z;
            backward_distance = probe_sum;
        }
        if (backward_distance < (sint32)distance_output || (sint32)distance_output < forward_distance)
        {
            uint32 sample_path_frame0 = (uint16)path_frame;
            uint32 path_count = (uint16)path_definition->pos_track.frame_count;
            AnimatedObject *path_object = this;
            path_object->flags = (path_object->flags & 0xFFFFF7FF) ^ 0x10;
            path_frame = path_count - sample_path_frame0;
        }
    }
    state = ((const ActorBinding *)(((const ActorPlacement *)path_definition)->state))->model_kind == 8 ? 28 : 26;
    flags = flags | 0x100;
    spawned = object_operations.behaviors[6].factory_handler(this);
    flags = flags & 0xFFFFFEFF;
    if (spawned != 0)
    {
        if (((const ActorBinding *)(((const ActorPlacement *)path_definition)->state))->model_kind != 8)
        {
            {
                const Vec3 *point_src = &spawned->pos;
                Vec3 *point_dst = &spawned->pos;
                const PathDefinition *point_definition = spawned->path_definition;
                MATRIX *point_matrix;

                uint32 point_x = (uint32)point_src->x, point_y = (uint32)point_src->y, point_z = point_src->z;
                if (point_definition != 0 && point_definition->transform != 0)
                {
                    VECTOR point_input;

                    point_input.vx = point_x;
                    point_input.vy = point_y;
                    point_input.vz = point_z;
                    PushMatrix();
                    point_definition = spawned->path_definition;
                    point_matrix = point_definition->transform != 0 ? &point_definition->transform->matrix : NULL;
                    SetRotMatrix(point_matrix);

                    {
                        VECTOR rotation_output;
                        ApplyRotMatrixLV(&point_input, &rotation_output);
                        point_dst->x = rotation_output.vx;
                        point_dst->y = rotation_output.vy;
                        point_dst->z = rotation_output.vz;
                    }
                    PopMatrix();
                    point_definition = spawned->path_definition;
                    point_matrix = point_definition->transform != 0 ? &point_definition->transform->matrix : NULL;
                    point_dst->x = (uint32)point_dst->x + point_matrix->t[0];
                    point_definition = spawned->path_definition;
                    point_matrix = point_definition->transform != 0 ? &point_definition->transform->matrix : NULL;
                    point_dst->y = (uint32)point_dst->y + point_matrix->t[1];
                    point_definition = spawned->path_definition;
                    point_matrix = point_definition->transform != 0 ? &point_definition->transform->matrix : NULL;
                    point_dst->z = (uint32)point_dst->z + point_matrix->t[2];
                }
                else
                {
                    point_dst->x = point_x;
                    point_dst->y = point_y;
                    point_dst->z = point_z;
                }
            }
            spawned->movement_mode = 0;
            spawned->flags = spawned->flags | 0x2000;
            spawned->velocity.z = 0;
            spawned->velocity.x = 0;
            spawned->velocity.y = spawned->GetModelGrpLaunchSpeed();
        }
        spawned->animation_binding.tracks[0] = -2;
        spawned->animation_binding.tracks[1] = -3;
    }
    state_delay = resource_model->shot_interval;
finish:
    result = UpdateActorFacingAndAnimation(target, origin, resource_model);

    return result;
}

intptr AnimatedObject::InitializeActorVerticalShot()
{
    FUNCTION_MARKER(0x80022358, "SLUS_005.78");
    if (!object_operations.behaviors[14].plain_handler && !object_operations.behaviors[14].member_handler && !object_operations.behaviors[14].void_member_handler && !object_operations.behaviors[14].factory_handler)
    {
        ObjectCallback operation = {0, &AnimatedObject::UpdateActorVerticalShot};
        object_operations.behaviors[14] = operation;
    }
    behavior = 14;
    return 14;
}

sint32 AnimatedObject::FindActorPathSeg() const
{
    FUNCTION_MARKER(0x80022388, "SLUS_005.78");
    const PathDefinition *definition = path_definition;
    const VectorAnimationKey *path, *seg;
    sint32 frame;
    if (definition == 0)
        return 0;
    path = (const VectorAnimationKey *)definition->pos_track.keys;
    if (path == 0)
        return 0;
    frame = (uint16)path_frame;
    if ((flags & 0x10) == 0)
        frame = (sint16)((uint16)definition->pos_track.frame_count - (frame + 1) - 1);
    else
        frame = (sint16)frame;
    seg = path + definition->pos_track.key_count - 1;
    if (frame >= definition->pos_track.frame_limit)
    {
        if ((definition->pos_track.flags & 3) == 3)
            return 0;
    }
    else if ((uint32)frame < (seg->time >> 20))
    {
        seg = path;
        if ((uint32)frame >= (path[1].time >> 20))
        {
            ++seg;
            do
            {
                uint32 next_start = seg[1].time >> 20;
                ++seg;
                if ((uint32)frame < next_start)
                    break;
            } while (1);
            --seg;
        }
    }
    uint32 byte_delta = (const uint8 *)seg - definition->pos_track.keys;
    return (sint32)(byte_delta * 0xCCCCCCCDu) >> 3;
}

void AnimatedObject::UpdateActorPathSegMotion()
{
    FUNCTION_MARKER(0x800224AC, "SLUS_005.78");
    ActorBinding *state;
    const ActorModelDefinition *resource_model;
    uint32 timer = state_delay - 1, result;
    AnimationHeader *clip;
    const PathDefinition *definition;

    Vec3 world_pos;


    sint32 prev, cur;
    state_delay = timer;
    if (timer != 0xFFFFFFFF || ticks == 0)
    {
        (this->*object_operations.motions[4].member_handler)();
        return;
    }
    definition = path_definition;
    state = (ActorBinding *)(((const ActorPlacement *)definition)->state);
    ActorBinding *binding = state;
    resource_model = binding->model;
    state_delay = 0;
    ((PathDefinition *)definition)->flags = definition->flags | 0x40;
    prev = FindActorPathSeg();
    (this->*object_operations.motions[4].member_handler)();
    result = binding->flags & 2;
    if (result != 0)
        return;
    cur = FindActorPathSeg();
    if (cur == prev && path_frame < path_definition->pos_track.frame_count - 1)
        clip = resource_model->animations[2];
    else
    {
        if (cur != prev)
        {
            definition = path_definition;
            if ((flags & 0x10) != 0)
            {
                ++prev;
                if (prev == definition->pos_track.key_count)
                    prev = 0;
            }
            else
            {
                if (prev < 0)
                    prev = definition->pos_track.key_count - 1;
                path_frame = (uint16)definition->pos_track.frame_count - (((const VectorAnimationKey *)definition->pos_track.keys)[prev].time >> 20) - 1;
            }
            {
                const Vec3 *point_src = &pos;
                Vec3 *point_dst = &world_pos;
                const PathDefinition *point_definition = path_definition;
                MATRIX *point_matrix;
                uint32 point_axis;
                uint32 point_x = (uint32)point_src->x, point_y = (uint32)point_src->y, point_z = point_src->z;
                if (point_definition != 0 && point_definition->transform != 0)
                {
                    VECTOR point_input;

                    point_input.vx = point_x;
                    point_input.vy = point_y;
                    point_input.vz = point_z;
                    PushMatrix();
                    point_definition = path_definition;
                    point_matrix = point_definition->transform != 0 ? &point_definition->transform->matrix : NULL;
                    SetRotMatrix(point_matrix);
                    point_axis = 5;

                    {
                        VECTOR rotation_output;
                        ApplyRotMatrixLV(&point_input, &rotation_output);
                        point_dst->x = rotation_output.vx;
                        point_dst->y = rotation_output.vy;
                        point_dst->z = rotation_output.vz;
                    }
                    PopMatrix();
                    point_definition = path_definition;
                    point_matrix = point_definition->transform != 0 ? &point_definition->transform->matrix : NULL;
                    point_dst->x = (uint32)point_dst->x + point_matrix->t[0];
                    point_definition = path_definition;
                    point_matrix = point_definition->transform != 0 ? &point_definition->transform->matrix : NULL;
                    point_dst->y = (uint32)point_dst->y + point_matrix->t[1];
                    point_definition = path_definition;
                    point_matrix = point_definition->transform != 0 ? &point_definition->transform->matrix : NULL;
                    point_dst->z = (uint32)point_dst->z + point_matrix->t[2];
                }
                else
                {
                    point_dst->x = point_x;
                    point_dst->y = point_y;
                    point_dst->z = point_z;
                }
            }
            definition = path_definition;
            {
                const Vec3 *point_src = &((const VectorAnimationKey *)definition->pos_track.keys)[prev].pos;
                Vec3 *point_dst = &velocity;
                const PathDefinition *point_definition = path_definition;
                MATRIX *point_matrix;
                uint32 point_axis;
                uint32 point_x = (uint32)point_src->x, point_y = (uint32)point_src->y, point_z = point_src->z;
                if (point_definition != 0 && point_definition->transform != 0)
                {
                    VECTOR point_input;

                    point_input.vx = point_x;
                    point_input.vy = point_y;
                    point_input.vz = point_z;
                    PushMatrix();
                    point_definition = path_definition;
                    point_matrix = point_definition->transform != 0 ? &point_definition->transform->matrix : NULL;
                    SetRotMatrix(point_matrix);
                    point_axis = 5;

                    {
                        VECTOR rotation_output;
                        ApplyRotMatrixLV(&point_input, &rotation_output);
                        point_dst->x = rotation_output.vx;
                        point_dst->y = rotation_output.vy;
                        point_dst->z = rotation_output.vz;
                    }
                    PopMatrix();
                    point_definition = path_definition;
                    point_matrix = point_definition->transform != 0 ? &point_definition->transform->matrix : NULL;
                    point_dst->x = (uint32)point_dst->x + point_matrix->t[0];
                    point_definition = path_definition;
                    point_matrix = point_definition->transform != 0 ? &point_definition->transform->matrix : NULL;
                    point_dst->y = (uint32)point_dst->y + point_matrix->t[1];
                    point_definition = path_definition;
                    point_matrix = point_definition->transform != 0 ? &point_definition->transform->matrix : NULL;
                    point_dst->z = (uint32)point_dst->z + point_matrix->t[2];
                }
                else
                {
                    point_dst->x = point_x;
                    point_dst->y = point_y;
                    point_dst->z = point_z;
                }
            }
            velocity.x = velocity.x - (uint32)world_pos.x;
            velocity.y = velocity.y - (uint32)world_pos.y;
            velocity.z = velocity.z - (uint32)world_pos.z;
        }
        if (path_frame < path_definition->pos_track.frame_count - 1)
            flags = flags & 0xFFFFF7FF;
        else
            flags = flags | 0x800;
        state_delay = 15;
        definition = path_definition;
        ((PathDefinition *)definition)->flags = definition->flags & 0xFFFFFFBF;
        clip = resource_model->animations[0];
    }
    if (animation != clip)
    {
        {
            AnimationHeader *animation_val = clip;
            AnimatedObject *animation_object = this;
            uint32 animation_flags = animation_object->flags;
            animation_object->animation = animation_val;
            animation_object->frame = -1;
            animation_object->flags = animation_flags | 0x10000;
        }
    }
}

intptr AnimatedObject::InitializeActorPathSegMotion()
{
    FUNCTION_MARKER(0x8002293C, "SLUS_005.78");
    intptr result;
    const PathDefinition *definition;
    if (!object_operations.behaviors[21].plain_handler && !object_operations.behaviors[21].member_handler && !object_operations.behaviors[21].void_member_handler && !object_operations.behaviors[21].factory_handler)
    {
        {
            ObjectCallback operation = {0, 0, 0, &AnimatedObject::UpdateActorPathSegMotion};
            object_operations.motions[5] = operation;
        }
        {
            ObjectCallback operation = {0, &AnimatedObject::UpdateBasicActor};
            object_operations.behaviors[21] = operation;
        }
    }
    definition = path_definition;
    behavior = 21;
    result = (intptr)definition->pos_track.keys;
    if (result != 0)
    {
        result = flags;
        movement_mode = 5;
        result |= 0x40;
        flags = result;
    }
    state_delay = 0;
    return result;
}

intptr AnimatedObject::UpdateBasicActor()
{
    FUNCTION_MARKER(0x800229A4, "SLUS_005.78");
    Vec3 origin;
    Vec3 target;
    uint32 distance;
    uint32 result;
    UpdateActorStateAndTarget(origin, target, &distance);
    ActorBinding *binding = (ActorBinding *)(((const ActorPlacement *)path_definition)->state);
    result = UpdateActorFacingAndAnimation(0, origin, binding->model);
    return result;
}

intptr AnimatedObject::UpdateActorPatrolPause()
{
    FUNCTION_MARKER(0x800278E8, "SLUS_005.78");
    ActorModelDefinition *resource_model;
    const PathDefinition *definition = path_definition;
    ActorBinding *binding = (ActorBinding *)(((const ActorPlacement *)definition)->state);

    resource_model = binding->model;
    uint32 result;
    Vec3 origin;
    Vec3 target;
    uint32 distance;
    UpdateActorStateAndTarget(origin, target, &distance);
    if ((binding->flags & 10) == 0)
    {
        if ((path_definition->flags & 0x40) != 0)
        {
            uint32 timer = state_delay;
            if (timer != 0)
                state_delay = --timer;
            if (timer == 0)
            {
                AnimationHeader *clip;
                uint32 delay;
                if ((flags & 0x40) != 0)
                {
                    clip = resource_model->animations[0];
                    delay = 60;
                }
                else
                {
                    ((ActorBinding *)(((const ActorPlacement *)path_definition)->state))->motion_speed = 0;
                    clip = resource_model->animations[2];
                    delay = 0;
                }
                animation = clip;
                frame = -1;
                flags = flags | 0x10000;
                if (delay == 0)
                    delay = random_generator.Next() % 60 + 90;
                state_delay = delay;
                flags = flags ^ 0x40;
            }
        }
        else if (animation != resource_model->animations[0])
        {
            state_delay = 0;
            flags = flags & 0xFFFFFFBF;
            animation = resource_model->animations[0];
            frame = -1;
            flags = flags | 0x10000;
        }
    }
    result = UpdateActorFacingAndAnimation(0, origin, resource_model);
    return result;
}

intptr AnimatedObject::InitializeActorPatrolPause()
{
    FUNCTION_MARKER(0x80027A8C, "SLUS_005.78");
    if (!object_operations.behaviors[20].plain_handler && !object_operations.behaviors[20].member_handler && !object_operations.behaviors[20].void_member_handler && !object_operations.behaviors[20].factory_handler)
    {
        ObjectCallback operation = {0, &AnimatedObject::UpdateActorPatrolPause};
        object_operations.behaviors[20] = operation;
    }
    behavior = 20;
    state_delay = 0;
    return 20;
}

void AnimatedObject::SpawnActionEffect(AnimatedObject *object, const void *context, uint32 action)
{
    FUNCTION_MARKER(0x80027B1C, "SLUS_005.78");
    uint32 output;
    uint16 heading;
    Vec3 dir;
    Vec3 world, other_world, input, inverse;
    Vec3 *dst;

    MATRIX matrix_local = {0};

    const PathDefinition *parent;
    uint32 flags, val, x, y, z, a, b, c;
    PathTransform *path_transform;
    const AnimatedObject *owner;
    uint32 pass, passes, global_src, idx;
    sint32 mode;

    AnimatedObject *effect = action_effect_resources.templates[action];
    if (effect == 0)
        return;
    if ((effect->node.flags & 0x80) != 0)
    {
        val = effect->node.flags;
        effect->node.links.sibling_next = 0;
        effect->node.flags = val & 0x7F;
    }
    else
    {
        AnimatedObject *created = AnimatedObject::CloneObjectResource(effect);
        if (created == 0)
            return;
        effect = created;
        created->flags &= 0xFFFFFFFB;
    }
    flags = effect->flags;
    effect->frame = 0;
    effect->action_context = 0;
    effect->state = action;
    effect->flags = flags | 0x20000;
    if (action == 10)
    {
        x = object->action_ox;
        y = object->action_oy;
        z = object->action_oz;
        effect->pos.x = x;
        effect->pos.y = y;
        effect->pos.z = z;
    }
    else
    {
        effect->pos = object->world_pos;
        if (context != 0 && action != 2)
        {
            AnimatedObject::EmptyActionOperation0();
            mode = character_pad.motion.state_code;
            if (mode >= 10 && mode < 13)
                AnimatedObject::EmptyActionOperation2();
            else if (mode == 20)
                AnimatedObject::EmptyActionOperation1();
        }
        switch (action)
        {
            case 0:
                x = (uint32)effect->pos.y;
                val = object->bounds.height;
                effect->pos.y = x + val;
                break;
            case 1:
                x = (uint32)effect->pos.y;
                val = object->bounds.height;
                effect->pos.y = x - val;
                break;
            case 2:
                effect->action_context = (const ActorBinding *)context;
                val = random_generator.Next() & 0x7FF;
                effect->velocity.x = val - 1024;
                effect->velocity.x = val >= 1024 ? val + 1024 : val - 3072;
                val = (random_generator.Next() & 0x3FF) + 2048;
                effect->velocity.y = val;
                val = random_generator.Next() & 0x7FF;
                effect->velocity.z = val - 1024;
                effect->velocity.z = val >= 1024 ? val + 1024 : val - 3072;
                break;
            case 5:
            case 8:
                break;
            case 9:
            case 12:
                x = (uint32)effect->pos.x;
                y = effect->pos.y;
                z = effect->pos.z;
                effect->world_pos.x = x;
                effect->world_pos.y = y;
                effect->world_pos.z = z;
                effect->bounds.radius = 12288;
                effect->frame = 0;
                effect->path_frame = 0;
                if (action == 12)
                {
                    character_pad.action_effects[action] = effect;
                    effect->flags = effect->flags | 0x8000;
                }
                break;
            case 11:
                mode = action_start_mode;
                effect->velocity.z = 0;
                if (mode == 1)
                {
                    owner = CharacterController::FindPlayerObject(0);
                    flags = owner->flags;
                    mode = owner->path_frame;
                    AnimatedObject::SampleObjectPathDir(&heading, dir, owner, mode, flags & 0x10);
                    effect->TurnObjectFacing(dir, 4096);
                }
                else if (mode == 2)
                {
                    MATRIX *cur_matrix = &render_view.matrix_cursor->matrix;
                    {
                        VECTOR rotation_input = {((const SpriteRecord *)effect)->pos.x, ((const SpriteRecord *)effect)->pos.y, ((const SpriteRecord *)effect)->pos.z, 0};
                        VECTOR rotation_output;
                        ApplyRotMatrixLV(&rotation_input, &rotation_output);
                        cur_matrix->t[0] = rotation_output.vx;
                        cur_matrix->t[1] = rotation_output.vy;
                        cur_matrix->t[2] = rotation_output.vz;
                    }
                    cur_matrix = &render_view.matrix_cursor->matrix;
                    MATRIX *parent_matrix = render_view.matrix_cursor->prev != 0 ? &render_view.matrix_cursor->prev->matrix : 0;
                    x = cur_matrix->t[0];
                    a = parent_matrix->t[0];
                    y = cur_matrix->t[1];
                    b = parent_matrix->t[1];
                    z = cur_matrix->t[2];
                    c = parent_matrix->t[2];
                    cur_matrix->t[0] = x + a;
                    cur_matrix->t[1] = y + b;
                    cur_matrix->t[2] = z + c;
                    effect->node.DrawBillboard();
                }
                break;
            case 13:
            case 14:
                owner = CharacterController::FindPlayerObject(0);
                passes = action == 13 ? 2 : 1;
                for (pass = 0; pass < passes; ++pass)
                {
                    global_src = action == 14 || pass != 0;
                    const AnimatedObject *src_object = global_src != 0 ? character_action_target : owner;
                    dst = pass == 0 ? &world : &other_world;
                    parent = src_object->path_definition;
                    if (parent != 0 && parent->transform != 0)
                    {
                        input = src_object->pos;
                        PushMatrix();
                        if (global_src != 0)
                        {
                            src_object = character_action_target;
                        }
                        path_transform = src_object->path_definition->transform;
                        SetRotMatrix(&path_transform->matrix);
                        {
                            VECTOR sdk_result;
                            {
                                VECTOR sdk_input = {input.x, input.y, input.z, 0};
                                ApplyRotMatrixLV(&sdk_input, &sdk_result);
                            }
                            dst->x = sdk_result.vx;
                            dst->y = sdk_result.vy;
                            dst->z = sdk_result.vz;
                        }
                        PopMatrix();
                        if (global_src != 0)
                        {
                            src_object = character_action_target;
                        }
                        path_transform = src_object->path_definition->transform;
                        val = dst->x;
                        x = path_transform->matrix.t[0];
                        dst->x = val + x;
                        path_transform = src_object->path_definition->transform;
                        val = dst->y;
                        x = path_transform->matrix.t[1];
                        dst->y = val + x;
                        path_transform = src_object->path_definition->transform;
                        val = dst->z;
                        x = path_transform->matrix.t[2];
                        dst->z = val + x;
                    }
                    else
                    {
                        if (global_src != 0)
                        {
                            src_object = character_action_target;
                        }

                        *dst = src_object->pos;
                    }
                    if (pass == 0)
                    {
                        parent = effect->path_definition;
                        path_transform = parent != 0 ? parent->transform : 0;
                        if (path_transform != 0)
                        {
                            val = world.x;
                            x = path_transform->matrix.t[0];
                            inverse.x = val - x;
                            path_transform = effect->path_definition->transform;
                            val = world.y;
                            x = path_transform->matrix.t[1];
                            inverse.y = val - x;
                            path_transform = effect->path_definition->transform;
                            val = world.z;
                            x = path_transform->matrix.t[2];
                            inverse.z = val - x;
                            for (idx = 0; idx < 9; ++idx)
                            {
                                path_transform = effect->path_definition->transform;
                                val = (uint16)path_transform->matrix.m[idx % 3][idx / 3];
                                matrix_local.m[idx / 3][idx % 3] = val;
                            }
                            PushMatrix();
                            SetRotMatrix(&matrix_local);
                            {
                                VECTOR rotation_output;
                                {
                                    VECTOR sdk_input = {inverse.x, inverse.y, inverse.z, 0};
                                    ApplyRotMatrixLV(&sdk_input, &rotation_output);
                                }
                                effect->pos.x = rotation_output.vx;
                                effect->pos.y = rotation_output.vy;
                                effect->pos.z = rotation_output.vz;
                            }
                            PopMatrix();
                        }
                        else
                        {
                            effect->pos = world;
                        }
                        effect->pos.y = (uint32)effect->pos.y + 0x4000;
                    }
                }
                val = AnimatedObject::ActionDistance(action == 13 ? other_world : world, effect->pos);
                effect->velocity.x = val;
                val = action == 13 ? character_pad.action13_scale : character_pad.action14_scale;
                effect->AdvanceActionEffect(owner, action, val << 1, &output);

                effect->world_pos = effect->pos;
                effect->bounds.radius = 12288;
                effect->frame = 0;
                effect->path_frame = 0;
                character_pad.action_effects[action] = effect;
                flags = effect->flags;
                if (action == 14)
                    effect->velocity.z = 0;
                effect->flags = flags | 0x8000;
                if (action == 13)
                    character_pad.action13_count += 1;
                else
                    character_pad.action14_count += 1;
                if (action == 13)
                    effect->velocity.z = 0;
                break;
            default:
                character_pad.action_effects[action] = effect;
                effect->flags = effect->flags | 0x8000;
                break;
        }
    }
    effect->behavior = 26;
    {
        SceneSection *section = scene_stream.active_section;
        SceneNode *prev = section->transient_links.first_child;

        effect->node.links.sibling_next = prev;
        section->transient_links.first_child = &effect->node;
    }
}

uint32 AnimatedObject::UpdateInteractionChain(const AnimatedObject *owner)
{
    sint32 difference;

    FUNCTION_MARKER(0x80028720, "SLUS_005.78");
    uint32 cur_flags = player_interaction.flags;
    uint32 amplitude = 8192;
    Vec3 step;
    Vec3 sample_pos = {0};
    SceneNode *child_node;
    SpriteRecord *child;
    uint32 idx;
    if ((cur_flags & 32) == 0 || (flags & 0x20200) == 512)
    {
        uint8 object_flags;
        player_interaction.flags = cur_flags & ~36u;
        player_interaction.target = 0;
        object_flags = node.flags;
        behavior = 0;
        node.flags = object_flags | 2;
        return object_flags | 2;
    }
    if ((cur_flags & 8) != 0)
        path_frame = 4;
    else
    {
        sint16 countdown = path_frame;
        if (countdown != 0)
        {
            countdown = countdown - 1;
            path_frame = countdown;
            if (countdown >= 3)
                amplitude = (4 - countdown) * 2048 + 8192;
            else if (countdown >= 0)
            {
                amplitude = 12288;
                if (countdown == 0)
                    path_frame = -1;
            }
            else if (countdown == -5)
                path_frame = 0;
            else
                amplitude = ((countdown + 6) * 4096) / 5 + 8192;
        }
    }
    difference = owner->world_pos.x - (uint32)pos.x;
    step.x = mul12(difference, 256);
    difference = owner->world_pos.y - (uint32)pos.y;
    step.y = mul12(difference, 256);
    difference = owner->world_pos.z - (uint32)pos.z;
    step.z = mul12(difference, 256);
    child_node = node.links.first_child;
    for (idx = 0; idx < 16; ++idx)
    {
        child = child_node != 0 ? (SpriteRecord *)child_node : NULL;
        uint32 weight = (idx < 9 ? idx : 15 - idx) * 1024;
        sint16 phase;
        sint32 wave;
        uint32 height;
        child->pos = sample_pos;
        phase = (idx + frame) << 9;
        wave = SinQ14(phase) >> 2;
        height = child->pos.y;
        child->extent[1] = amplitude >> 5;
        child->extent[0] = amplitude >> 5;
        child->pos.y = height + (uint32)mul12(wave, weight);
        sample_pos.x = sample_pos.x + ((uint32)step.x);
        sample_pos.y = sample_pos.y + ((uint32)step.y);
        sample_pos.z = sample_pos.z + ((uint32)step.z);
        child_node = child->node.links.sibling_next;
    }
    idx = (uint16)frame + 1;
    frame = idx;
    return idx;
}

uint32 AnimatedObject::UpdateInteractionWave(const AnimatedObject *owner)
{
    sint32 difference;

    FUNCTION_MARKER(0x800289D4, "SLUS_005.78");
    uint32 cur_flags = character_action_flags;
    uint32 amplitude = 8192;
    Vec3 step;
    Vec3 sample_pos = {0};
    SceneNode *child_node;
    SpriteRecord *child;
    uint32 idx;
    sint16 countdown;
    if ((cur_flags & 8) == 0 || (flags & 0x20200) == 512)
    {
        AnimatedObject *active = character_action_target;
        uint8 object_flags;
        character_action_flags = (cur_flags | 0x800) & ~8u;
        if (active != 0)
        {
            ActorBinding *binding = (ActorBinding *)(((const ActorPlacement *)active->path_definition)->state);

            binding->flags &= 0xF7;
        }
        object_flags = node.flags;
        behavior = 0;
        node.flags = object_flags | 2;
        return object_flags | 2;
    }
    countdown = path_frame;
    if (countdown != 0)
    {
        countdown = countdown - 1;
        path_frame = countdown;
        if (countdown >= 3)
            amplitude = (4 - countdown) * 2048 + 8192;
        else if (countdown >= 0)
        {
            amplitude = 12288;
            if (countdown == 0)
                path_frame = -1;
        }
        else if (countdown == -5)
            path_frame = 0;
        else
            amplitude = ((countdown + 6) * 4096) / 5 + 8192;
    }
    difference = owner->world_pos.x - (uint32)pos.x;
    step.x = mul12(difference, 256);
    difference = owner->world_pos.y - (uint32)pos.y;
    step.y = mul12(difference, 256);
    difference = owner->world_pos.z - (uint32)pos.z;
    step.z = mul12(difference, 256);
    {
        sint8 next_phase = character_pad.action_wave_phase + 1;
        character_pad.action_wave_phase = next_phase;
        if (next_phase >= 16)
            character_pad.action_wave_phase = 0;
    }
    child_node = node.links.first_child;
    for (idx = 0; idx < 16; ++idx)
    {
        child = child_node != 0 ? (SpriteRecord *)child_node : NULL;
        uint32 wave_scale = (idx < 9 ? idx : 15 - idx) * 1280;
        sint32 phase;
        uint32 wave;
        uint32 height;
        child->pos = sample_pos;
        phase = (character_pad.action_wave_phase + (sint32)idx) % 16;
        wave = mul12(OriginalDefaults::interaction_wave[phase], wave_scale);
        height = child->pos.y;
        child->extent[1] = amplitude >> 5;
        child->extent[0] = amplitude >> 5;
        child->pos.y = height + wave;
        sample_pos.x = sample_pos.x + ((uint32)step.x);
        sample_pos.y = sample_pos.y + ((uint32)step.y);
        sample_pos.z = sample_pos.z + ((uint32)step.z);
        child_node = child->node.links.sibling_next;
    }
    idx = (uint16)frame + 1;
    frame = idx;
    return idx;
}

uint32 AnimatedObject::ActionDistance(const Vec3 &v0, const Vec3 &v1)
{
    FUNCTION_MARKER(0x80028CC0, "SLUS_005.78");
    uint32 x = v1.x - (uint32)v0.x;
    uint32 y = v1.y - (uint32)v0.y;
    uint32 z = v1.z - (uint32)v0.z;
    if ((sint32)x < 0)
        x = -x;
    if ((sint32)y < 0)
        y = -y;
    if ((sint32)z < 0)
        z = -z;
    return 2 * (x + z) + y;
}

uint32 AnimatedObject::AdvanceActionEffect(const AnimatedObject *owner, uint32 action, uint32 distance, uint32 *progress_output)
{
    FUNCTION_MARKER(0x80028D60, "SLUS_005.78");
    Vec3 world[3], input, relative;

    MATRIX transpose = {0};

    uint32 src_idx, idx;
    const PathDefinition *definition;
    PathTransform *path_transform;
    Vec3 difference;
    Vec3 absolute;
    Vec3 interpolated;
    uint32 len;
    uint32 progress;
    sint32 coefficient;
    uint32 initialized = 0;
    uint32 result;
    sint32 rcp;
    CharacterController *character;
    for (src_idx = 0; src_idx < 3; ++src_idx)
    {
        const AnimatedObject *src = src_idx == 0 ? owner : (src_idx == 1 ? character_action_target : this);
        Vec3 *dst = &world[src_idx];
        definition = src->path_definition;
        if (definition != 0 && definition->transform != 0)
        {
            input.x = src->pos.x;
            input.y = src->pos.y;
            input.z = src->pos.z;
            PushMatrix();
            if (src_idx == 1)
            {
                src = character_action_target;
            }
            path_transform = src->path_definition->transform;
            {
                SetRotMatrix(&path_transform->matrix);
            }
            {
                VECTOR sdk_input = {input.x, input.y, input.z, 0};
                {
                    VECTOR sdk_result;
                    ApplyRotMatrixLV(&sdk_input, &sdk_result);
                    dst->x = sdk_result.vx;
                    dst->y = sdk_result.vy;
                    dst->z = sdk_result.vz;
                }
            }
            PopMatrix();
            if (src_idx == 1)
            {
                src = character_action_target;
            }
            path_transform = src->path_definition->transform;
            dst->x = (uint32)dst->x + path_transform->matrix.t[0];
            path_transform = src->path_definition->transform;
            dst->y = (uint32)dst->y + path_transform->matrix.t[1];
            path_transform = src->path_definition->transform;
            dst->z = (uint32)dst->z + path_transform->matrix.t[2];
        }
        else
        {
            if (src_idx == 1)
            {
                src = character_action_target;
            }
            dst->x = src->pos.x;
            dst->y = src->pos.y;
            dst->z = src->pos.z;
        }
    }
    world[2].y = (uint32)world[2].y - 16384;
    difference.x = world[0].x - (uint32)world[1].x;
    relative.x = difference.x;
    absolute.x = difference.x < 0 ? -(uint32)difference.x : (uint32)difference.x;
    difference.y = world[0].y - (uint32)world[1].y;
    relative.y = difference.y;
    absolute.y = difference.y < 0 ? -(uint32)difference.y : (uint32)difference.y;
    difference.z = world[0].z - (uint32)world[1].z;
    relative.z = difference.z;
    absolute.z = difference.z < 0 ? -(uint32)difference.z : (uint32)difference.z;
    len = 2u * (absolute.x + (uint32)absolute.z) + absolute.y;

    progress = velocity.x;
    progress = action == 13 ? progress - distance : progress + distance;
    if (len == 0)
        trap(7);
    rcp = 0x40000000 / (sint32)len;
    velocity.x = progress;
    coefficient = ((sint64)(sint32)progress * rcp) >> 18;
    fixed_math_result = coefficient;
    character = &character_pad;
    if (character_pad.interaction_phase == 4 && character_pad.action_mode == 1)
    {
        uint32 limit = character_pad.action_shared_limit;
        if (limit == -1)
        {
            if (action == 13)
            {
                uint32 endpoint = character_pad.action14_endpoint;
                if ((sint32)progress < (sint32)endpoint && endpoint != -1)
                {
                    initialized = 1;
                    character_pad.action_shared_limit = endpoint;
                    character_pad.action13_limit_reached = 1;
                    velocity.z = 1;
                    progress = character_pad.action_shared_limit;
                    character_pad.action13_endpoint = progress;
                }
            }
            else
            {
                uint32 endpoint = character_pad.action13_endpoint;
                if ((sint32)endpoint < (sint32)progress && character_pad.action14_endpoint != -1)
                {
                    initialized = 1;
                    character_pad.action_shared_limit = endpoint;
                    character_pad.action14_limit_reached = 1;
                    velocity.z = 1;
                    progress = character_pad.action_shared_limit;
                    character_pad.action14_endpoint = progress;
                }
            }
        }
        else if (velocity.z == 1)
        {
            if (action == 13 ? (sint32)progress < (sint32)limit : (sint32)limit < (sint32)progress)
                progress = limit;
        }
        else if (action == 13)
        {
            if ((sint32)progress < (sint32)limit)
            {
                if (character_pad.action13_limit_reached != 0)
                {
                    result = -2;
                    goto finished;
                }
                velocity.z = 1;
                progress = character_pad.action_shared_limit;
                character_pad.action13_limit_reached = 1;
            }
        }
        else if ((sint32)limit < (sint32)progress)
        {
            if (character_pad.action14_limit_reached != 0)
            {
                result = -2;
                goto finished;
            }
            velocity.z = 1;
            progress = character_pad.action_shared_limit;
            character_pad.action14_limit_reached = 1;
        }
    }
    if ((sint32)progress < (sint32)len && (sint32)progress >= 0 && initialized == 0)
    {
        coefficient = ((sint64)(sint32)progress * rcp) >> 18;
        fixed_math_result = coefficient;
        interpolated.x = mul12(coefficient, difference.x);
        interpolated.y = mul12(coefficient, difference.y);
        interpolated.z = mul12(coefficient, difference.z);
        interpolated.y = (uint32)interpolated.y + 16384;
        interpolated.x = interpolated.x + ((uint32)world[1].x);
        interpolated.y = interpolated.y + ((uint32)world[1].y);
        interpolated.z = interpolated.z + ((uint32)world[1].z);
        definition = path_definition;
        if (definition != 0 && definition->transform != 0)
        {
            path_transform = path_definition->transform;
            relative.x = (uint32)interpolated.x - path_transform->matrix.t[0];
            path_transform = path_definition->transform;
            relative.y = (uint32)interpolated.y - path_transform->matrix.t[1];
            path_transform = path_definition->transform;
            relative.z = (uint32)interpolated.z - path_transform->matrix.t[2];
            for (idx = 0; idx < 9; ++idx)
            {
                path_transform = path_definition->transform;
                transpose.m[idx / 3][idx % 3] = path_transform->matrix.m[idx % 3][idx / 3];
            }
            PushMatrix();
            {
                SetRotMatrix(&transpose);
            }
            {
                VECTOR rotation_output;
                {
                    VECTOR sdk_input = {relative.x, relative.y, relative.z, 0};
                    ApplyRotMatrixLV(&sdk_input, &rotation_output);
                }
                pos.x = rotation_output.vx;
                pos.y = rotation_output.vy;
                pos.z = rotation_output.vz;
            }
            PopMatrix();
        }
        else
        {
            pos.x = interpolated.x;
            pos.y = interpolated.y;
            pos.z = interpolated.z;
        }
    }
    *progress_output = progress;
    if (action == 13)
        result = (sint32)progress < 0 ? (uint32)-1 : progress;
    else
        result = (sint32)progress < (sint32)len ? len - progress : (uint32)-1;
finished:
    return result;
}

void AnimatedObject::UpdateActionEffect(const AnimatedObject *owner, uint32 action)
{
    FUNCTION_MARKER(0x800296A4, "SLUS_005.78");
    uint32 output, result, distance;
    if ((character_action_flags & 0x2008) != 8)
        goto remove_object;
    if (character_pad.interaction_phase != 4)
    {
        AdvanceActionEffect(owner, action, 20480, &output);

        if ((sint32)output >= 2048)
            goto finished;
        character_pad.action_completion = 1;
        goto remove_primary;
    }
    distance = AdvanceActionEffect(owner, action, action == 13 ? 20480 : 4096, &output);
    if (action == 13)
    {
        uint32 prev = character_pad.action_shared_limit;
        uint32 check_status = prev != -1;
        if (prev == -1)
        {
            uint32 sample = output;
            uint32 cur = character_pad.action_shared_limit;
            character_pad.action13_endpoint = sample;
            check_status = cur != prev;
        }
        if (check_status != 0 && distance == -2)
        {
            uint32 flags;
            character_pad.action_shared_limit = character_pad.action_shared_limit - 8192;
            flags = node.flags;
            behavior = 0;
            node.flags = flags | 2;
            character_pad.action13_count = (uint8)character_pad.action13_count - 1u;
        }
        if ((sint32)distance < 2048 && (node.flags & 2) == 0)
            character_pad.action_completion = 1;
    }
    else
    {
        if (character_pad.action_shared_limit == -1)
            character_pad.action14_endpoint = output;
        if (character_pad.action_mode == 1 && distance == -2 && velocity.z != 1)
        {
            uint32 flags;
            sint32 displacement = character_pad.action_limit_step * 4096;
            character_pad.action_shared_limit = character_pad.action_shared_limit + displacement;
            flags = node.flags;
            behavior = 0;
            node.flags = flags | 2;
            character_pad.action14_count = (uint8)character_pad.action14_count - 1u;
        }
        if ((sint32)distance < 2048 && (node.flags & 2) == 0)
        {
            uint32 flags;
            character_pad.action_completion = 2;
            flags = node.flags;
            behavior = 0;
            node.flags = flags | 2;
            character_pad.action14_count = (uint8)character_pad.action14_count - 1u;
        }
    }
    result = distance + 2u < 2;
    if (result == 0)
        goto finished;
    result = node.flags & 2;
    if (result != 0)
        goto finished;
remove_object:
    if (action != 13)
    {
        uint32 flags = node.flags;
        behavior = 0;
        node.flags = flags | 2;
        --character_pad.action14_count;
        goto finished;
    }
remove_primary:
{
    uint32 flags = node.flags;
    behavior = 0;
    node.flags = flags | 2;
    --character_pad.action13_count;
}
finished:
    return;
}

intptr AnimatedObject::UpdateActionObject()
{
    FUNCTION_MARKER(0x8002995C, "SLUS_005.78");
    const AnimatedObject *owner;
    const ActorBinding *context;
    const PathDefinition *parent;
    uint32 pointer, cur_state, val, cur_flags, x, y, z, result;
    AnimationHeader *clip;
    PathTransform *path_transform;
    Vec3 world, local, relative;

    MATRIX inverse = {0};

    sint32 sample_frame, prev, len, timer;
    context = action_context;
    if (context != 0)
        owner = context->instance;
    else
        owner = CharacterController::FindPlayerObject(0);
    cur_state = state;
    if (cur_state == 9)
        return UpdateInteractionChain(owner);
    if (cur_state == 12)
        return UpdateInteractionWave(owner);
    if (cur_state == 13 || cur_state == 14)
    {
        const AnimatedObject *player = CharacterController::FindPlayerObject(0);
        clip = animation;
        sample_frame = 0;
        if (clip)
        {
            prev = frame;
            len = clip->frame_count;
            if (prev < len - 1)
                sample_frame = prev + 1;
        }
        const AnimatedObject *sel = character_action_target;
        frame = sample_frame;
        if (sel != 0)
            UpdateActionEffect(player, cur_state);
        goto finish;
    }
    if (cur_state == 11)
    {
        const AnimatedObject *src = owner;
        clip = animation;
        sample_frame = 0;
        if (clip)
        {
            prev = frame;
            len = clip->frame_count;
            if (prev < len - 1)
                sample_frame = prev + 1;
        }
        val = velocity.z;
        frame = sample_frame;
        ++val;
        velocity.z = val;
        if ((sint32)val >= 4)
            goto expire;
        parent = src->path_definition;
        path_transform = parent != 0 ? parent->transform : 0;
        if (path_transform)
        {
            local = src->pos;
            PushMatrix();
            path_transform = src->path_definition->transform;
            SetRotMatrix(&path_transform->matrix);
            {
                VECTOR sdk_input = {local.x, local.y, local.z, 0};
                VECTOR sdk_output;
                ApplyRotMatrixLV(&sdk_input, &sdk_output);
                world.x = sdk_output.vx;
                world.y = sdk_output.vy;
                world.z = sdk_output.vz;
            }
            PopMatrix();
            path_transform = src->path_definition->transform;
            x = world.x;
            y = path_transform->matrix.t[0];
            world.x = x + y;
            path_transform = src->path_definition->transform;
            x = world.y;
            y = path_transform->matrix.t[1];
            world.y = x + y;
            path_transform = src->path_definition->transform;
            x = world.z;
            y = path_transform->matrix.t[2];
            world.z = x + y;
        }
        else
        {
            world = src->pos;
        }
        parent = path_definition;
        path_transform = parent != 0 ? parent->transform : 0;
        if (path_transform)
        {
            x = world.x;
            y = path_transform->matrix.t[0];
            relative.x = x - y;
            path_transform = path_definition->transform;
            x = world.y;
            y = path_transform->matrix.t[1];
            relative.y = x - y;
            path_transform = path_definition->transform;
            x = world.z;
            y = path_transform->matrix.t[2];
            relative.z = x - y;
            path_transform = path_definition->transform;
            inverse.m[0][0] = path_transform->matrix.m[0][0];
            path_transform = path_definition->transform;
            inverse.m[0][1] = path_transform->matrix.m[1][0];
            path_transform = path_definition->transform;
            inverse.m[0][2] = path_transform->matrix.m[2][0];
            path_transform = path_definition->transform;
            inverse.m[1][0] = path_transform->matrix.m[0][1];
            path_transform = path_definition->transform;
            inverse.m[1][1] = path_transform->matrix.m[1][1];
            path_transform = path_definition->transform;
            inverse.m[1][2] = path_transform->matrix.m[2][1];
            path_transform = path_definition->transform;
            inverse.m[2][0] = path_transform->matrix.m[0][2];
            path_transform = path_definition->transform;
            inverse.m[2][1] = path_transform->matrix.m[1][2];
            path_transform = path_definition->transform;
            inverse.m[2][2] = path_transform->matrix.m[2][2];
            PushMatrix();
            SetRotMatrix(&inverse);
            {
                VECTOR rotation_output;
                {
                    VECTOR sdk_input = {relative.x, relative.y, relative.z, 0};
                    ApplyRotMatrixLV(&sdk_input, &rotation_output);
                }
                pos.x = rotation_output.vx;
                pos.y = rotation_output.vy;
                pos.z = rotation_output.vz;
            }
            PopMatrix();
        }
        else
        {
            pos = world;
        }
        goto finish;
    }
    clip = animation;
    sample_frame = 0;
    if (clip)
    {
        len = clip->frame_count;
        prev = frame;
        if (prev < len - 1)
            sample_frame = prev + 1;
    }
    cur_state = state;
    if (cur_state == 2)
    {
        x = pos.x;
        val = velocity.x;
        y = pos.y;
        cur_flags = velocity.y;
        z = pos.z;
        pointer = velocity.z;
        pos.x = x + val;
        pos.y = y + cur_flags;
        pos.z = z + pointer;
    }
    else if (cur_state == 8 && sample_frame == 15)
        AnimatedObject::QueueCharacterActionSound(11, 3);
    if ((flags & 0x8000) && owner)
    {
        node.state_flags = owner->node.state_flags + 64;
        path_transform = 0;
        if (!(owner->flags & 0x2000))
        {
            parent = owner->path_definition;
            if (parent)
                path_transform = parent->transform;
        }
        if (path_transform)
        {
            local = owner->pos;
            PushMatrix();
            path_transform = owner->path_definition->transform;
            SetRotMatrix(&path_transform->matrix);
            {
                VECTOR rotation_output;
                {
                    VECTOR sdk_input = {local.x, local.y, local.z, 0};
                    ApplyRotMatrixLV(&sdk_input, &rotation_output);
                }
                pos.x = rotation_output.vx;
                pos.y = rotation_output.vy;
                pos.z = rotation_output.vz;
            }
            PopMatrix();
            path_transform = owner->path_definition->transform;
            x = pos.x;
            y = path_transform->matrix.t[0];
            pos.x = x + y;
            path_transform = owner->path_definition->transform;
            x = pos.y;
            y = path_transform->matrix.t[1];
            pos.y = x + y;
            path_transform = owner->path_definition->transform;
            x = pos.z;
            y = path_transform->matrix.t[2];
            pos.z = x + y;
        }
        else
        {
            pos = owner->pos;
        }
        x = pos.y;
        y = owner->bounds.height;
        context = action_context;
        pos.y = x + y;
        if (context == 0)
        {
            cur_flags = owner->path_flags;
            if (cur_flags & 1)
                AnimatedObject::ApplyHeadingPosCorrection(owner, this);
            else if (cur_flags & 0x100)
                AnimatedObject::ApplyCameraPosCorrection(owner, this);
        }
        if ((owner->node.flags & 1) || (action_context == 0 && ((uint16)(character_pad.motion.form_flags >> 16) & 15) >= 2 && state - 3 < 2))
            node.flags = node.flags | 1;
        else
            node.flags = node.flags & 0xFE;
    }
    else if (sample_frame < frame)
        goto expire;
    cur_state = state;
    frame = sample_frame;
    if (cur_state == 7)
    {
        timer = (sint16)character_pad.timers.model_swap_ticks;
        if (timer < 64)
        {
            val = node.flags;
            node.flags = (timer & 4) ? val & 0xFE : val | 1;
        }
    }
finish:
    result = node.flags & 1;
    if (!result)
        result = ClampAnimationFrame();
    return result;
expire:
    val = node.flags;
    behavior = 0;
    result = val | 2;
    node.flags = result;
    return result;
}

uint32 AnimatedObject::ComputePathBoundaryExcess(const Vec3 &pos) const
{
    uint32 dx, dy, dz;

    sint32 sum;

    FUNCTION_MARKER(0x8002AB84, "SLUS_005.78");
    const PathDefinition *definition = path_definition;
    Vec3 start_pos, end_pos;
    uint32 sq = 0;
    sint32 len;
    Vec3 midpoint;

    uint32 radius;
    uint32 result;
    if (definition == 0)
        return 0;
    definition->SamplePos(0, start_pos, 1);
    definition = path_definition;
    definition->SamplePos(definition->pos_track.frame_count - 1, end_pos, 1);
    dx = (sint32)(start_pos.x - (uint32)end_pos.x) >> 6;
    dy = (sint32)(start_pos.y - (uint32)end_pos.y) >> 6;
    dz = (sint32)(start_pos.z - (uint32)end_pos.z) >> 6;
    sq += dx * dx + dy * dy + dz * dz;
    if ((sint32)sq >= 0)
        len = SquareRoot12(sq);
    else
        len = SquareRoot12((sq >> 2)) * 2;
    radius = len >> 1;
    sum = start_pos.x + (uint32)end_pos.x;
    midpoint.x = sum >> 1;
    sum = start_pos.y + (uint32)end_pos.y;
    midpoint.y = sum >> 1;
    sum = start_pos.z + (uint32)end_pos.z;
    midpoint.z = sum >> 1;
    sq = 0;
    dx = (sint32)(pos.x - (uint32)midpoint.x) >> 6;
    dy = (sint32)(pos.y - (uint32)midpoint.y) >> 6;
    dz = (sint32)(pos.z - (uint32)midpoint.z) >> 6;
    sq += dx * dx + dy * dy + dz * dz;
    radius -= 40960;
    if ((sint32)sq >= 0)
        result = SquareRoot12(sq) - radius;
    else
        result = SquareRoot12((sq >> 2)) * 2 - radius;
    return result;
}

intptr AnimatedObject::UpdateForm5Movement()
{
    FUNCTION_MARKER(0x8002AD4C, "SLUS_005.78");

    uint32 input, cur_flags, result, speed, acceleration, limit;
    if ((character_pad.secondary_flags & 0x400) != 0 && frame == animation->frame_count - 1)
        character_pad.secondary_flags = character_pad.secondary_flags & 0xFFFFFBFF;
    flags = flags | 0x400;
    GameplayInput::UpdateGameplayInput();
    input = gameplay_input.buttons;
    cur_flags = 0;
    speed = character_pad.motion.horizontal_speed;
    acceleration = character_pad.movement.acceleration0;
    if ((input & 0x2000) != 0)
        speed = (flags & 0x10) != 0 ? speed + acceleration : speed - acceleration;
    else if ((input & 0x8000) != 0)
        speed = (flags & 0x10) != 0 ? speed - acceleration : speed + acceleration;
    else
        speed = (sint32)(speed * 7) / 8;
    character_pad.motion.horizontal_speed = speed;
    if ((sint32)speed < 0)
        cur_flags |= 2;
    limit = (uint32)character_pad.movement.acceleration0 << 2;
    if ((sint32)limit < (sint32)speed)
        character_pad.motion.horizontal_speed = limit;
    speed = character_pad.motion.vertical_speed;
    if ((input & 0x1000) != 0)
    {
        speed += (uint32)character_pad.movement.acceleration0;
        character_pad.motion.vertical_speed = speed;
        limit = (uint32)character_pad.movement.acceleration0 << 2;
        if ((sint32)limit < (sint32)speed)
            character_pad.motion.vertical_speed = limit;
    }
    else if ((input & 0x4000) != 0)
    {
        speed -= (uint32)character_pad.movement.acceleration0;
        character_pad.motion.vertical_speed = speed;
        limit = (-(uint32)character_pad.movement.acceleration0) << 2;
        if ((sint32)speed < (sint32)limit)
            character_pad.motion.vertical_speed = limit;
    }
    else
        character_pad.motion.vertical_speed = (sint32)(speed * 7) / 8;
    character_pad.motion.jump_phase = 0;
    if ((input & (*control_bindings.active_scheme)[0]) != 0)
        cur_flags |= 0x40;
    UpdateMovementImpulse(&cur_flags);
    UpdateActionInput(&cur_flags);
    result = IntegrateCharacterMovement(cur_flags);
    return result;
}

intptr AnimatedObject::InitializeCommonActor()
{
    FUNCTION_MARKER(0x8002AFA8, "SLUS_005.78");
    uint32 result;
    const PathDefinition *parent;
    if ((!object_operations.behaviors[7].plain_handler && !object_operations.behaviors[7].member_handler && !object_operations.behaviors[7].void_member_handler && !object_operations.behaviors[7].factory_handler))
    {
        ObjectCallback operation = {0, &AnimatedObject::UpdateCommonActor};
        object_operations.behaviors[7] = operation;
    }
    parent = path_definition;
    behavior = 7;
    result = parent->pos_track.keys != 0;
    if (result != 0)
    {
        result = flags | 0x40;
        flags = result;
    }
    return result;
}

intptr AnimatedObject::UpdateCommonActor()
{
    FUNCTION_MARKER(0x8002B000, "SLUS_005.78");
    ActorModelDefinition *resource_model;
    Vec3 origin;
    Vec3 target;
    uint32 distance;
    uint32 result;
    UpdateActorStateAndTarget(origin, target, &distance);
    ActorBinding *binding = (ActorBinding *)(((const ActorPlacement *)path_definition)->state);
    resource_model = binding->model;
    result = UpdateActorFacingAndAnimation(0, origin, resource_model);
    return result;
}

uint32 AnimatedObject::GetModelGrpLaunchSpeed() const
{
    FUNCTION_MARKER(0x8002B220, "SLUS_005.78");
    uint32 count = model_grps.count;
    uint32 type = state;
    ModelGroupDefinition *const *slot = model_grps.definitions;
    while (count != 0)
    {
        const ModelGroupDefinition *object = *slot;
        --count;
        if (object != 0 && object->kind == type)
            return object->launch_speed;
        ++slot;
    }
    /* Return no match instead of the original read through address zero */
    return 0;
}

uint32 AnimatedObject::StopModelGrpSound()
{
    FUNCTION_MARKER(0x8002C510, "SLUS_005.78");
    uint32 result = 6;
    if (behavior == 6)
    {
        result = flags & 0x40000020;
        if (result == 0x40000020)
        {
            uint32 remaining = model_grps.count;
            ModelGroupDefinition *const *slot = model_grps.definitions;
            uint32 kind = state;
            const ModelGroupDefinition *entry = 0;
            while (remaining != 0)
            {
                const ModelGroupDefinition *candidate = *slot;
                --remaining;
                if (candidate != 0 && candidate->kind == kind)
                {
                    entry = candidate;
                    break;
                }
                ++slot;
            }
            result = entry->doppler_radius;
            if (result != 0)
            {
                const SoundSample *sound = 0;
                const SoundBankStorage *sounds = entry->sound_table;
                if (sounds != 0 && sounds->header.count >= 3)
                    sound = sounds->samples + 2;
                if ((uint8)sound_effects.IsPlaying(sound, (sint16)sound_voice) != 0)
                    SoundEffects::StopSoundVoice(sound_voice);
                result = -1;
                sound_voice = result;
            }
        }
    }
    return result;
}

uint32 AnimatedObject::CanModelGrpContact(const SceneNode *other_node) const
{
    FUNCTION_MARKER(0x8002F75C, "SLUS_005.78");
    const SceneNode *other = other_node;
    uint32 kind, variant;
    const ScenePath *context;
    sint32 subtype, cur_state, facing, other_facing;
    kind = other->kind;
    if (kind == 2)
    {
        subtype = (sint8)(other->state_flags >> 8);
        if (subtype == 9 || subtype == 14 || subtype == 29)
            return 1;
        if ((uint8)((uint32)subtype - 40) < 16 && (other->flags & 0x80) == 0 && ((((const SceneTrigger *)other)->parameters.event.command >> 24) & 0xFE) == 0)
        {
            if (((((const SceneTrigger *)other)->parameters.event.schedule >> 12) & 31) == 4)
                return 1;
            subtype = (sint8)(other->state_flags >> 8);
        }
        if (subtype != 1)
            return 0;
        cur_state = contact_kind;
        if (cur_state == 1)
        {
            if (behavior == 33)
                context = scene_path;
            else
            {
                context = camera_motion.path;
            }
        }
        else
        {
            context = 0;
            if (cur_state == 3 && animation_binding.tracks[0] == -1)
                context = scene_path;
        }
        return (CanObjectChangePath((const SceneTrigger *)other_node, context) & 0xFF) != 0;
    }
    if (kind >= 3)
        return kind == 8;
    if (kind != 0)
        return 0;
    cur_state = animation_binding.tracks[1];
    variant = (uint8)((const AnimatedObject *)other)->contact_kind;
    if (cur_state != -3)
        return 0;
    if (variant - 1 >= 2 || animation_binding.tracks[0] == -(sint8)variant)
    {
        if ((sint8)variant != 3 || (sint8)(((const AnimatedObject *)other)->animation_binding.word >> 8) != cur_state)
            return 0;
        facing = animation_binding.tracks[0];
        other_facing = (sint8)((const AnimatedObject *)other)->animation_binding.word;
        if (facing == other_facing)
            return 0;
    }
    if ((sint8)variant == 2 && state == 11)
        return ((const AnimatedObject *)other)->behavior == 17;
    return 1;
}

uint32 AnimatedObject::CollectModelGrpContact(SceneNode *other_node, CollisionContact *contact_data)
{
    FUNCTION_MARKER(0x8002F944, "SLUS_005.78");
    SceneNode *other = other_node;
    SceneTrigger *prev;
    uint32 kind, contact, val, variant;
    sint32 subtype;
    if (contact_data == 0)
        return (sint8)CanModelGrpContact(other_node);
    kind = other->kind;
    contact = contact_data->fraction;
    if (kind == 2)
    {
        subtype = (sint8)(other->state_flags >> 8);
        if (subtype == 9 || subtype == 14 || subtype == 29)
        {
            model_contacts.blocker = other_node;
            return 1;
        }
        if (subtype == 1)
        {
            prev = model_contacts.surface0;
            if (prev != 0 && (sint8)(prev->node.state_flags >> 8) != 1)
            {
                if (model_contacts.surface1 == 0)
                {
                    val = model_contacts.fraction0;
                    model_contacts.surface1 = prev;
                    model_contacts.fraction1 = val;
                }
                else if (model_contacts.surface2 == 0)
                {
                    val = model_contacts.fraction0;
                    model_contacts.surface2 = prev;
                    model_contacts.fraction2 = val;
                }
                model_contacts.surface0 = 0;
            }
            if ((TestCharacterSurfaceContact((const SceneTrigger *)other_node, contact) & 0xFF) == 0)
                return 0;
            prev = model_contacts.surface0;
            if (prev != 0 && (sint8)(prev->node.state_flags >> 8) == 1 && (sint32)contact < (sint32)model_contacts.fraction0)
                return 0;
            model_contacts.surface0 = (SceneTrigger *)other_node;
            model_contacts.fraction0 = contact;
        }
        else
        {
            if ((uint8)((uint32)subtype - 40) >= 16)
                return 0;
            if (model_contacts.surface0 == 0)
            {
                model_contacts.surface0 = (SceneTrigger *)other_node;
                model_contacts.fraction0 = contact;
            }
            else if (model_contacts.surface1 == 0)
            {
                model_contacts.surface1 = (SceneTrigger *)other_node;
                model_contacts.fraction1 = contact;
            }
            else
            {
                if (model_contacts.surface2 != 0)
                    return 0;
                model_contacts.surface2 = (SceneTrigger *)other_node;
                model_contacts.fraction2 = contact;
            }
        }
        return 0;
    }
    if (kind < 3)
    {
        model_contacts.blocker = other_node;
        return 1;
    }
    if (kind != 8)
        return 0;
    CharacterContactPool::QueueCharacterPickup(other_node, 0);
    variant = ((const BillboardNode *)other)->playback_flags & 0x7F;
    if (variant - 3 < 3)
        ticks = (uint16)ticks + 30;
    if (variant - 6 < 2)
        return 1;
    return variant == 10;
}

intptr AnimatedObject::UpdateActorVelocity()
{
    FUNCTION_MARKER(0x8002FB94, "SLUS_005.78");
    uint32 result, speed, cur_state, mag, x, y, z, sq;
    if (state == 19 && (player_interaction.flags & 3) != 3)
    {
        result = (uint16)ticks + 1;
        ticks = result;
        return result;
    }
    result = ComputeObjectFacing(velocity, 0);
    target_yaw = result;
    orientation[2] = (result + 2048) & 0xFFF;
    x = velocity.x;
    y = velocity.y;
    z = velocity.z;
    if (flags & 0x400)
        y = 0;
    if (x || z)
    {
        x = (sint32)x >> 6;
        y = (sint32)y >> 6;
        z = (sint32)z >> 6;
        sq = x * x + y * y + z * z;
        if ((sint32)sq >= 0)
            result = SquareRoot12(sq);
        else
            result = SquareRoot12((sq >> 2)) << 1;
        speed = result;
    }
    else
        speed = 0;
    cur_state = state;
    if (cur_state == 19)
        ApplyInteractionDirectionalInput(&speed);
    else if (!cur_state)
        ApplyActorDirectionalInput(&speed);
    result = flags & 0x2000;
    if (!result)
    {
        mag = speed;
        if (mag || (uint32)velocity.y)
            result = ComputePathMotion(mag, velocity.y, NULL);
        else
        {
            velocity.z = 0;
            velocity.y = 0;
            velocity.x = 0;
        }
    }
    return result;
}

void AnimatedObject::ApplyModelGrpContactMotion()
{
    const PathDefinition *parent;
    const PathDefinition *definition;
    FUNCTION_MARKER(0x8002FD54, "SLUS_005.78");

    Vec3 dir, motion, translation;

    MATRIX matrix_local = {0};

    const AnimatedObject *owner;
    uint32 idx;
    PathTransform *path_transform;
    uint32 cur_flags;
    uint32 val;
    uint32 x;
    uint32 y;
    uint32 z;
    uint32 a;
    uint32 b;
    uint32 c;
    sint32 factor;
    uint32 scaled_x;
    uint32 scaled_y;
    uint32 scaled_z;
    uint32 sq;
    uint32 mag;
    const ScenePath *support;
    SceneTrigger *contact;
    uint32 result;
    sint32 speed, cur_state, timer;
    if (state == 19 && (player_interaction.flags & 3) != 3)
    {
        owner = CharacterController::FindPlayerObject(0);
        cur_flags = player_interaction.flags;
        speed = 40960;
        if ((cur_flags & 0x24) == 0x24)
        {
            result = node.flags | 1;
            node.flags = result;
            goto finish;
        }
        node.flags = node.flags & 0xFE;
        cur_flags = owner->flags;
        if ((cur_flags & 0x2000) == 0 && (parent = owner->path_definition) != 0 && parent->transform != 0)
        {
            motion = owner->pos;
            PushMatrix();
            definition = owner->path_definition;
            path_transform = definition->transform;
            SetRotMatrix(&path_transform->matrix);
            {
                VECTOR rotation_output;
                {
                    VECTOR sdk_input = {motion.x, motion.y, motion.z, 0};
                    ApplyRotMatrixLV(&sdk_input, &rotation_output);
                }
                velocity.x = rotation_output.vx;
                velocity.y = rotation_output.vy;
                velocity.z = rotation_output.vz;
            }
            PopMatrix();
            definition = owner->path_definition;
            path_transform = definition->transform;
            val = velocity.x;
            x = path_transform->matrix.t[0];
            velocity.x = val + x;
            definition = owner->path_definition;
            path_transform = definition->transform;
            val = velocity.y;
            x = path_transform->matrix.t[1];
            velocity.y = val + x;
            definition = owner->path_definition;
            path_transform = definition->transform;
            val = velocity.z;
            x = path_transform->matrix.t[2];
            velocity.z = val + x;
        }
        else
        {
            velocity = owner->pos;
        }
        velocity.y = (uint32)velocity.y + 12288;
        x = velocity.x;
        a = pos.x;
        y = velocity.y;
        b = pos.y;
        z = velocity.z;
        c = pos.z;
        x -= a;
        y -= b;
        z -= c;
        velocity.x = x;
        velocity.y = y;
        velocity.z = z;
        x = velocity.x >> 6;
        y = velocity.y >> 6;
        z = (sint32)z >> 6;
        sq = x * x + y * y + z * z;
        mag = (sint32)sq >= 0 ? SquareRoot12(sq) : SquareRoot12((sq >> 2)) << 1;
        if ((player_interaction.flags & 0x40) != 0)
            speed *= 16;
        if ((sint32)mag < speed + speed / 4)
            ticks = 1;
        if ((uint32)velocity.x + 26754 > 0xD103 || (uint32)velocity.y + 26754 > 0xD103 || (uint32)velocity.z + 26754 > 0xD103)
        {
            for (;;)
            {
                x = velocity.x;
                z = velocity.z;
                velocity.x = (sint32)x >> 1;
                y = velocity.y;
                x = velocity.x;
                z = (sint32)z >> 1;
                velocity.z = z;
                velocity.y = (sint32)y >> 1;
                if (x + 26754 > 0xD103)
                    continue;
                if ((uint32)velocity.y + 26754 > 0xD103 || z + 26754 > 0xD103)
                    continue;
                break;
            }
        }
        {
            VECTOR n_input = {velocity.x, velocity.y, velocity.z, 0};
            VECTOR n_output;
            VectorNormal(&n_input, &n_output);
            velocity.x = n_output.vx;
            velocity.y = n_output.vy;
            velocity.z = n_output.vz;
        }
        x = velocity.x;
        scaled_x = mul12((sint32)x, speed);
        y = velocity.y;
        velocity.x = scaled_x;
        scaled_y = mul12((sint32)y, speed);
        z = velocity.z;
        velocity.y = scaled_y;
        velocity.z = mul12((sint32)z, speed);
        result = ComputeObjectPitch(velocity, 0);
        pitch_scale = result;
        orientation[1] = result & 0xFFF;
        result = ComputeObjectFacing(velocity, 0);
        target_yaw = result;
        orientation[2] = (result + 2048) & 0xFFF;

        dir = velocity;
        x = dir.x;
        if (x == 0 && dir.z == 0)
            mag = 0;
        else
        {
            x = (sint32)x >> 6;
            y = dir.y >> 6;
            z = dir.z >> 6;
            sq = x * x + y * y + z * z;
            mag = (sint32)sq >= 0 ? SquareRoot12(sq) : SquareRoot12((sq >> 2)) << 1;
        }
        ComputePathMotion(mag, velocity.y, &dir);
        val = pos.x;
        x = dir.x;
        result = val + x;
        pos.x = result;
        val = pos.y;
        x = dir.y;
        result = val + x;
        pos.y = result;
        val = pos.z;
        x = dir.z;
        result = val + x;
        pos.z = result;
        goto finish;
    }
    if (path_motion_scale != 0 && (velocity.x != 0 || velocity.y != 0 || velocity.z != 0))
    {
        motion = velocity;
        if ((sint32)path_motion_scale < 4096)
        {
            translation.z = 0;
            translation.y = 0;
            translation.x = 0;
            cur_flags = flags;
            flags = cur_flags & 0xFFFFF7FF;
            if ((cur_flags & 0x6000) == 0)
            {
                parent = path_definition;
                if (parent != 0 && (path_transform = parent->transform) != 0)
                {
                    translation = path_transform->displacement;
                }
            }
            x = motion.x;
            a = translation.x;
            y = motion.y;
            b = translation.y;
            z = motion.z;
            c = translation.z;
            x += a;
            y += b;
            z += c;
            motion.x = x;
            motion.y = y;
            motion.z = z;
            factor = path_motion_scale;
            scaled_x = mul12((sint32)x, factor);
            motion.x = scaled_x;
            scaled_y = mul12((sint32)y, factor);
            motion.y = scaled_y;
            scaled_z = mul12((sint32)z, factor);
            motion.z = scaled_z;
            motion.x = scaled_x - a;
            motion.y = scaled_y - b;
            motion.z = scaled_z - c;
            if (state == 19)
                ticks = 0;
        }
        if ((flags & 0x2000) == 0)
        {
            parent = path_definition;
            if (parent != 0 && (path_transform = parent->transform) != 0)
            {
                for (idx = 0; idx < 9; ++idx)
                {
                    if (idx != 0)
                    {
                        definition = path_definition;
                        path_transform = definition->transform;
                    }
                    val = (uint16)path_transform->matrix.m[idx % 3][idx / 3];
                    matrix_local.m[idx / 3][idx % 3] = val;
                }

                translation = motion;
                PushMatrix();
                SetRotMatrix(&matrix_local);
                {
                    VECTOR sdk_input = {translation.x, translation.y, translation.z, 0};
                    VECTOR sdk_output;
                    ApplyRotMatrixLV(&sdk_input, &sdk_output);
                    motion.x = sdk_output.vx;
                    motion.y = sdk_output.vy;
                    motion.z = sdk_output.vz;
                }
                PopMatrix();
            }
        }
        val = pos.x;
        x = motion.x;
        pos.x = val + x;
        val = pos.y;
        x = motion.y;
        pos.y = val + x;
        val = pos.z;
        x = motion.z;
        pos.z = val + x;
    }
    contact = model_contacts.surface0;
    if (contact != 0)
    {
        val = model_contacts.fraction0;
        factor = path_motion_scale;
        if (factor >= (sint32)val)
        {
            if ((sint8)(contact->node.state_flags >> 8) == 1)
            {
                support = FindCharacterSupportPath(contact);
                if (support != 0)
                {
                    const PathDefinition *support_definition = support->definition;
                    ApplyPathVelocityChange(support_definition);
                    scene_path = support;
                }
            }
            else
                contact->node.state_flags = contact->node.state_flags | 8;
        }
    }
    contact = model_contacts.surface1;
    if (contact != 0)
    {
        val = model_contacts.fraction1;
        factor = path_motion_scale;
        if (factor >= (sint32)val)
            contact->node.state_flags = contact->node.state_flags | 8;
    }
    contact = model_contacts.surface2;
    if (contact != 0)
    {
        val = model_contacts.fraction2;
        factor = path_motion_scale;
        if (factor >= (sint32)val)
            contact->node.state_flags = contact->node.state_flags | 8;
    }
    if (model_contacts.blocker != 0)
    {
        ticks = 0;
        goto clear_contacts;
    }
    cur_flags = flags;
    result = cur_flags & 0x800;
    if ((cur_flags & 0x4000) != 0)
    {
        ticks = 0;
        goto clear_contacts;
    }
    if (result == 0)
        goto clear_contacts;
    cur_state = animation_binding.tracks[0];
    flags = cur_flags & 0xFFFFF7FF;
    if (cur_state == -2)
    {
        timer = ticks;

        if (timer <= 0)
        {
            ticks = 0;
            goto clear_contacts;
        }
    }
    parent = path_definition;
    if (parent != 0 && parent->transform != 0)
    {
        translation = pos;
        PushMatrix();
        definition = path_definition;
        path_transform = definition->transform;
        SetRotMatrix(&path_transform->matrix);
        {
            VECTOR rotation_output;
            {
                VECTOR sdk_input = {translation.x, translation.y, translation.z, 0};
                ApplyRotMatrixLV(&sdk_input, &rotation_output);
            }
            pos.x = rotation_output.vx;
            pos.y = rotation_output.vy;
            pos.z = rotation_output.vz;
        }
        PopMatrix();
        definition = path_definition;
        path_transform = definition->transform;
        x = pos.x;
        val = path_transform->matrix.t[0];
        parent = path_definition;
        pos.x = x + val;
        path_transform = parent->transform;
        y = pos.y;
        val = path_transform->matrix.t[1];
        parent = path_definition;
        pos.y = y + val;
        path_transform = parent->transform;
        z = pos.z;
        val = path_transform->matrix.t[2];
        pos.z = z + val;
    }
    parent = path_definition;
    if (parent != 0 && (path_transform = parent->transform) != 0)
    {
        x = velocity.x;
        val = path_transform->displacement.x;
        parent = path_definition;
        velocity.x = x + val;
        path_transform = parent->transform;
        y = velocity.y;
        val = path_transform->displacement.y;
        parent = path_definition;
        velocity.y = y + val;
        path_transform = parent->transform;
        z = velocity.z;
        val = path_transform->displacement.z;
        velocity.z = z + val;
    }
    cur_flags = flags;
    val = state;
    flags = cur_flags | 0x2000;
    if (val != 19)
        movement_mode = 0;
    result = flags & 0x400;
    if (result != 0)
        goto clear_contacts;
    definition = path_definition;
    result = definition->flags & 8;
    if (result == 0)
        goto clear_contacts;
    if ((uint32)velocity.x + 26754 > 0xD103 || (uint32)velocity.y + 26754 > 0xD103 || (uint32)velocity.z + 26754 > 0xD103)
    {
        for (;;)
        {
            x = velocity.x;
            z = velocity.z;
            velocity.x = (sint32)x >> 1;
            y = velocity.y;
            x = velocity.x;
            z = (sint32)z >> 1;
            velocity.z = z;
            velocity.y = (sint32)y >> 1;
            if (x + 26754 > 0xD103)
                continue;
            if ((uint32)velocity.y + 26754 > 0xD103 || z + 26754 > 0xD103)
                continue;
            break;
        }
    }
    {
        VECTOR n_input = {velocity.x, velocity.y, velocity.z, 0};
        VECTOR n_output;
        VectorNormal(&n_input, &n_output);
        velocity.x = n_output.vx;
        velocity.y = n_output.vy;
        velocity.z = n_output.vz;
    }
    x = velocity.x;
    factor = state;
    scaled_x = mul12((sint32)x, factor);
    factor = state;
    y = velocity.y;
    velocity.x = scaled_x;
    scaled_y = mul12((sint32)y, factor);
    factor = state;
    result = velocity.z;
    velocity.y = scaled_y;
    velocity.z = mul12((sint32)result, factor);
clear_contacts:
    model_contacts.surface2 = 0;
    model_contacts.surface1 = 0;
    model_contacts.surface0 = 0;
    model_contacts.blocker = 0;
finish:
    return;
}

void AnimatedObject::RegisterActorVelocityOperations(void)
{
    FUNCTION_MARKER(0x80030994, "SLUS_005.78");
    {
        ObjectCallback operation = {0, &AnimatedObject::UpdateActorVelocity};
        object_operations.motions[2] = operation;
    }
    object_operations.collisions[3] = &AnimatedObject::CollectModelGrpContact;
}

sint16 AnimatedObject::SampleActorPathHeading() const
{
    const PathDefinition *definition;
    FUNCTION_MARKER(0x800309BC, "SLUS_005.78");
    sint32 frame0 = path_frame, frame1;
    definition = path_definition;
    if ((flags & 0x10) != 0)
    {
        frame1 = frame0 + 1;
        if (frame0 >= definition->pos_track.frame_count)
        {
            frame1 = frame0;
            --frame0;
        }
    }
    else
    {
        frame1 = frame0 - 1;
        if (frame0 <= 0)
        {
            frame1 = frame0;
            ++frame0;
        }
    }
    return PathDefinition::SampleDefinitionHeading(frame0, frame1, definition);
}

uint32 AnimatedObject::ReadObjectImpulseScale() const
{
    FUNCTION_MARKER(0x80030A34, "SLUS_005.78");
    void *data;
    const ActorModelDefinition *bound_model;
    const PathDefinition *parent;
    AnimationHeader *clip;
    parent = path_definition;
    if (parent == 0)
        return 0;
    data = ((const ActorPlacement *)parent)->state;
    if ((((const ActorBinding *)data)->flags & 2) == 0)
        return 0;
    bound_model = ((const ActorBinding *)data)->model;
    clip = animation;
    return clip == bound_model->animations[4] ? 5324 : 5734;
}

intptr AnimatedObject::UpdateActorReactionSpin()
{
    const PathDefinition *parent;
    FUNCTION_MARKER(0x80030AC8, "SLUS_005.78");
    const ActorModelDefinition *bound_model;
    ActorBinding *binding = (ActorBinding *)(((const ActorPlacement *)path_definition)->state);
    uint32 val = binding->reaction_state - 1;
    uint32 cur_flags, angle, result;
    binding->reaction_state = val;
    if ((val & 0xFF) == 0)
    {
        flags &= 0xFDFFFFFF;
        val = binding->reaction_state;
        contact_kind = 2;
        behavior = (sint32)val >> 8;
        angle = (binding->reaction_state >> 16) & 0xFFF;
        yaw = angle;
        target_yaw = angle;
    }
    else
    {
        parent = path_definition;
        cur_flags = parent == 0 ? 0 : ((const ActorBinding *)(((const ActorPlacement *)parent)->state))->flags & 8;
        if (cur_flags != 0)
            angle = (uint16)target_yaw - 6553;
        else
        {
            bound_model = ((const ActorBinding *)(((const ActorPlacement *)path_definition)->state))->model;
            if (bound_model->reaction_motion_mode == 1)
                goto finish;
            angle = (uint16)target_yaw - 3276;
        }
        target_yaw = angle;
        yaw = (angle << 17) >> 20;
    }
finish:
    result = (uint16)yaw;
    orientation[0] = 0x7FFF;
    orientation[1] = 0;
    result = (result + 2048) & 0xFFF;
    orientation[2] = result;
    return result;
}

void AnimatedObject::UpdateActorShrinkEffect()
{
    FUNCTION_MARKER(0x80030C2C, "SLUS_005.78");
    const ActorModelDefinition *bound_model;
    uint32 cur_state = state;
    sint32 countdown = (sint32)cur_state >> 16;
    sint32 scale = cur_state & 0xFFFF;
    SpriteRecord *target;
    if (countdown != 0)
    {
        --countdown;
        if (countdown == 0)
        {
            uint32 cur_flags = flags;
            const SoundBankStorage *sounds = sound_resources.common_table;
            const SoundSample *sound = 0;
            flags = cur_flags | 0x20000000;
            if (sounds != 0 && sounds->header.count >= 14)
                sound = sounds->samples + 13;
            sound_effects.Queue(sound, 0, 0, (uint16)sound_settings.effects_volume, 0, 3);
            flags = flags & ~0x10000000u;
        }
    }
    else
    {
        uint32 cur_flags0 = flags;
        if ((cur_flags0 & 0x10000000) != 0)
        {
            if (scale >= 1366)
            {
                scale -= 182;
                if (scale < 1366)
                {
                    scale = 1365;
                    countdown = 150;
                }
            }
        }
        else if (scale < 4096)
        {
            scale += 182;
            if (scale >= 4096)
            {
                scale = 4096;
                flags = cur_flags0 & ~0x20000000u;
            }
        }
    }
    target = spr;
    if (target != 0)
    {
        bound_model = ((const ActorBinding *)(((const ActorPlacement *)path_definition)->state))->model;
        sint32 product = mul12(bound_model->spr_extent[0], scale);
        target->extent[0] = product >> 5;
        product = mul12(bound_model->spr_extent[1], scale);
        target = spr;
        target->extent[1] = product >> 5;
    }
    state = scale | ((uint32)countdown << 16);
}

uint32 AnimatedObject::UpdateActorStateAndTarget(Vec3 &sample_pos, Vec3 &target, uint32 *distance)
{
    const PathDefinition *parent;
    FUNCTION_MARKER(0x80030DCC, "SLUS_005.78");
    void *definition;
    const ActorModelDefinition *motion_model;
    const ActorModelDefinition *animation_model;
    const ActorModelDefinition *resource_model;

    VECTOR delta, input;

    MATRIX matrix_local = {0};

    ActorBinding *interaction_target;
    const AnimatedObject *player;
    uint32 cur_flags, val, x, y, z, amount, repetitions;
    PathTransform *path_transform;
    uint32 heading, candidate, angle, pitch;
    AnimationHeader *clip, *alternate;
    const SoundBankStorage *sound_table;
    const SoundSample *sound;
    uint32 sq, result, idx, next;
    uint32 finish_early = 0;
    sint32 kind, sample_frame, limit, timer;
    definition = ((const ActorPlacement *)path_definition)->state;
    ActorBinding *binding = (ActorBinding *)definition;
    cur_flags = binding->reaction_flags;
    resource_model = binding->model;
    if ((cur_flags & 0x20000) != 0)
    {
        AnimatedObject::SpawnActionEffect(this, 0, 8);
        ticks = 0;
    }
    if ((binding->reaction_flags & 4) != 0)
    {
        limit = binding->respawn_ticks;
        if ((sint32)path_motion_scale >= limit)
        {
            SceneNode *event_node = binding->event_node;
            event_node->state_flags |= 8;
        }
    }
    if ((binding->reaction_flags & 0x28) != 0)
    {
        kind = resource_model->kind_override;
        if (kind == -1)
            kind = binding->model_kind;
        if (kind < 44)
        {
            sound = binding->sound;
            val = binding->sound_voice;
            if ((sound_effects.IsPlaying(sound, val) & 0xFF) != 0)
                SoundEffects::StopSoundVoice(binding->sound_voice);
            binding->sound = 0;
            binding->sound_voice = -1;
        }
        character_pad.support.release_pending = 0;
        kind = resource_model->kind_override;
        if (kind == -1)
            kind = binding->model_kind;
        if (kind != 30)
        {
            kind = resource_model->kind_override;
            if (kind == -1)
                kind = binding->model_kind;
            if (kind != 31)
                goto consume_timer;
        }
        cur_flags = binding->reaction_flags & 0xFFFFFFDF;
        binding->reaction_flags = cur_flags;
        sample_frame = frame;
        if (sample_frame == 0 || sample_frame == animation->frame_count - 1)
        {
            binding->reaction_flags = cur_flags | 0x20;
            character_pad.support.release_pending = 1;
        }
    consume_timer:
        cur_flags = binding->reaction_flags;
        amount = cur_flags >> 9;
        if ((cur_flags & 0x20) != 0 || ticks < (sint32)amount)
            amount = ticks;
        if (amount != 0)
        {
            ticks = (uint16)ticks - amount;
            repetitions = resource_model->health_restore_count;
            if (repetitions != 0)
            {
                --repetitions;
                do
                {
                    if (ticks == 0)
                    {
                        AnimatedObject::SpawnActionEffect(this, binding, 2);
                        CharacterController::RestoreCharacterHealthPoint();
                    }
                    --repetitions;
                } while (repetitions != 0xFFFFFFFF);
            }
            if (ticks == 0)
            {
                finish_early = 1;
                goto sample_target;
            }
        }
        kind = resource_model->kind_override;
        if (kind == -1)
            kind = binding->model_kind;
        if (kind != 19)
        {
            parent = path_definition;
            ((PathDefinition *)parent)->flags |= 0x40;
        }
    }
    if (path_motion_scale != 0 && (velocity.x != 0 || velocity.y != 0 || velocity.z != 0))
    {
        heading = ComputeObjectFacing(velocity, 0);
        if ((sint16)heading == 4096)
            candidate = 0;
        else
        {
            candidate = (uint16)yaw - heading;
            if ((sint16)candidate < 0)
                candidate = -candidate;
            if ((((uint32)(sint16)candidate - 1024) & 0xFFF) < 2048 && path_definition != 0 && binding->rotation_track != 0)
            {
                z = velocity.z;
                x = velocity.x;
                angle = Atan2Angle((-z), (-x));
                if (((((uint32)(sint16)heading - (sint16)angle) + 128) & 0xFFF) >= 257)
                    candidate = 0;
            }
        }
        motion_model = binding->model;
        if (motion_model->turn_step < (sint16)candidate && motion_model->turn_motion_mode == 0)
        {
            val = (uint16)binding->motion_frame;
            binding->motion_speed = 0;
            path_frame = val;
            goto after_motion;
        }
        parent = path_definition;
        if (parent != 0 && (path_transform = parent->transform) != 0)
        {
            matrix_local.m[0][0] = path_transform->matrix.m[0][0];
            for (idx = 1; idx < 9; ++idx)
            {
                path_transform = path_definition->transform;
                matrix_local.m[idx / 3][idx % 3] = path_transform->matrix.m[idx % 3][idx / 3];
            }
            x = velocity.x;
            y = velocity.y;
            z = velocity.z;
            input.vx = x;
            input.vy = y;
            input.vz = z;
            PushMatrix();
            SetRotMatrix(&matrix_local);
            ApplyRotMatrixLV(&input, &delta);
            PopMatrix();
        }
        else
        {
            x = velocity.x;
            y = velocity.y;
            z = velocity.z;
            delta.vx = x;
            delta.vy = y;
            delta.vz = z;
        }
        x = mul12(delta.vx, (sint32)path_motion_scale);
        val = delta.vy;
        delta.vx = x;
        y = mul12((sint32)val, (sint32)path_motion_scale);
        val = delta.vz;
        delta.vy = y;
        z = mul12((sint32)val, (sint32)path_motion_scale);
        delta.vz = z;
        kind = behavior;
        if ((object_operations.behaviors[kind].member_handler == &AnimatedObject::UpdateAttackEnemy) && (flags & 0x10) == 0)
        {
            delta.vx = -x;
            delta.vy = -y;
            delta.vz = -z;
        }
        x = pos.x;
        pos.x = x + delta.vx;
        y = pos.y;
        pos.y = y + delta.vy;
        z = pos.z;
        pos.z = z + delta.vz;
    }
    else
        path_frame = binding->motion_frame;
after_motion:
    cur_flags = binding->reaction_flags;
    if ((cur_flags & 1) != 0)
    {
        kind = behavior;
        behavior = 17;
        state = ((uint32)kind << 8) | 0x96;
        goto after_reaction;
    }
    if ((cur_flags & 8) != 0)
    {
        kind = resource_model->kind_override;
        if (kind == -1)
        {
            if (binding->model_kind >= 44)
                goto large_kind_reaction;
            kind = resource_model->kind_override;
        }
        else if (kind >= 44)
            goto large_kind_reaction;
        if (kind == -1)
            kind = binding->model_kind;
        if (kind == 10)
            goto blink;
        if (resource_model->reaction_motion_mode != 1 || ticks == 0)
        {
            cur_flags = flags;
            angle = yaw;
            kind = behavior;
            flags = cur_flags | 0x2000000;
            binding->reaction_state = (angle << 16) | ((uint32)kind << 8) | 10;
            angle = (uint16)yaw;
            contact_kind = 0;
            behavior = 16;
            target_yaw = angle << 3;
        }
        goto blink;
    large_kind_reaction:
        if ((binding->reaction_flags & 8) == 0)
            goto after_reaction;
        kind = resource_model->kind_override;
        if (kind == -1)
            kind = binding->model_kind;
        if (kind >= 47)
            goto after_reaction;
    blink:
        binding->blink_active = 1;
        binding->blink_ticks = 0;
        flags = flags | 0x80000;
    }
after_reaction:
    if ((flags & 0x30000000) != 0)
        UpdateActorShrinkEffect();
    cur_flags = binding->reaction_flags;
    if ((cur_flags & 0x10) != 0)
    {
        if ((cur_flags & 0x80) != 0)
            goto active_route;
        if (path_definition->pos_track.keys != 0 && (velocity.x != 0 || velocity.y != 0 || velocity.z != 0))
        {
            if (binding->route_dir_mode == 0)
            {
                binding->reaction_flags = binding->reaction_flags | 0x80;
                AnimatedObject::CaptureActorRouteDir(&binding->route_heading, binding->route_dir, this);
                binding->route_dir_mode = 1;
                if ((flags & 0x10) != 0)
                    val = binding->flags | 0x20;
                else
                    val = binding->flags & 0xDF;
                binding->flags = val;
            }
            goto active_route;
        }
        parent = path_definition;
        cur_flags = flags;
        sample_frame = (uint16)path_frame;
        val = (uint16)parent->pos_track.frame_count;
        flags = (cur_flags & 0xFFFFF7FF) ^ 0x10;
        path_frame = val - sample_frame;
        val = binding->reaction_flags | 0x80;
    }
    else
        val = cur_flags & 0xFFFFFF7F;
    binding->reaction_flags = val;
active_route:
    if ((flags & 0x800) != 0)
    {
        cur_flags = flags;
        parent = path_definition;
        path_frame = 0;
        cur_flags &= 0xFFFFF7FF;
        flags = cur_flags;
        if ((parent->pos_track.flags & 2) == 0)
            flags = cur_flags ^ 0x10;
    }
    if ((binding->flags & 2) == 0)
        goto animation_done;
    if ((player_interaction.flags & 4) == 0 && player_interaction.target == (definition != 0 ? binding : 0))
    {
        AnimatedObject::SpawnActionEffect(this, 0, 9);
        player_interaction.flags = player_interaction.flags | 12;
    }
    timer = binding->interaction_timer;
    if (timer == 0)
    {
        sound = binding->sound;
        val = binding->sound_voice;
        if ((sound_effects.IsPlaying(sound, val) & 0xFF) != 0)
            SoundEffects::StopSoundVoice(binding->sound_voice);
        sound_table = sound_resources.common_table;
        binding->sound = 0;
        binding->sound_voice = -1;
        sound = 0;
        if (sound_table != 0 && sound_table->header.count >= 13)
            sound = sound_table->samples + 12;
        pitch = sound->pitch;
        SoundEffects::PlaySpatialSound(sound, 8192, 0, pitch, world_pos.comp, 0x4B000, 0xFFFFFFFF, 0, 0);
        clip = binding->model->animations[4];
        animation = clip;
        clip = animation;
        velocity.z = 0;
        velocity.y = 0;
        velocity.x = 0;
        frame = -1;
        val = (uint16)clip->frame_count;
        binding->interaction_timer = ((sint16)val >> 1) + 45;
        goto animation_done;
    }
    if (timer < 0)
    {
        next = (uint32)timer + 1;
        binding->interaction_timer = next;
        if (next == 0)
        {
            animation_model = binding->model;
            clip = animation;
            alternate = animation_model->animations[5];
            if (clip == alternate)
            {
                clip = animation_model->animations[4];
                animation = clip;
                val = (uint16)clip->frame_count;
                frame = val;
                binding->interaction_timer = -(sint16)val;
                goto animation_done;
            }
            clip = animation_model->animations[0];
            frame = -1;
            animation = clip;
            kind = resource_model->kind_override;
            if (kind == -1)
                kind = binding->model_kind;
            if (kind == 10)
                flags = flags & 0xFFFFFFBF;
            binding->flags = binding->flags ^ 2;
            interaction_target = player_interaction.target;
            binding->reaction_flags = 0;
            if (interaction_target == (definition != 0 ? binding : 0))
                player_interaction.flags = player_interaction.flags & 0xFFFFFFDB;
            flags = flags | 0x20000;
            goto animation_done;
        }
        if (next == -45)
        {
            clip = binding->model->animations[4];
            animation = clip;
            frame = (uint32)(uint16)clip->frame_count - 1;
            binding->interaction_timer = 45;
            goto animation_done;
        }
        if ((player_interaction.flags & 8) == 0 || player_interaction.target != (definition != 0 ? binding : 0))
            goto animation_done;
        clip = animation;
        sample_frame = frame;
        limit = clip->frame_count;
        sound_table = sound_resources.common_table;
        binding->interaction_timer = ((limit - sample_frame) >> 1) + 45;
        sound = 0;
        if (sound_table != 0 && sound_table->header.count >= 13)
            sound = sound_table->samples + 12;
        animation_model = binding->model;
        clip = animation;
        alternate = animation_model->animations[5];
        pitch = sound->pitch;
        if (clip == alternate)
            pitch += pitch >> 3;
        goto play_state_sound;
    }
    next = (uint32)timer - 1;
    binding->interaction_timer = next;
    if ((sint32)next < 45 && (player_interaction.flags & 8) != 0 && player_interaction.target == (definition != 0 ? binding : 0))
    {
        animation_model = binding->model;
        clip = animation;
        alternate = animation_model->animations[5];
        if (clip == alternate)
        {
            sound_table = sound_resources.common_table;
            sound = 0;
            if (sound_table != 0 && sound_table->header.count >= 15)
                sound = sound_table->samples + 14;
            pitch = sound->pitch;
            SoundEffects::PlaySpatialSound(sound, 8192, 0, pitch, world_pos.comp, 0x4B000, 0xFFFFFFFF, 0, 0);
            ticks = 0;
            goto animation_done;
        }
        if (clip != animation_model->animations[4])
            goto animation_done;
        limit = clip->frame_count;
        sample_frame = frame;
        if (sample_frame != limit - 1)
            goto animation_done;
        animation = alternate;
        clip = animation;
        frame = -1;
        val = (uint16)clip->frame_count;
        sound_table = sound_resources.common_table;
        binding->interaction_timer = ((sint16)val >> 1) + 45;
        sound = 0;
        if (sound_table != 0 && sound_table->header.count >= 13)
            sound = sound_table->samples + 12;
        pitch = sound->pitch;
        pitch += pitch >> 3;
        goto play_state_sound;
    }
    if (binding->interaction_timer != 0)
        goto animation_done;
    clip = animation;
    limit = clip->frame_count;
    sound_table = sound_resources.common_table;
    binding->interaction_timer = -(uint32)limit;
    sound = 0;
    if (sound_table != 0 && sound_table->header.count >= 14)
        sound = sound_table->samples + 13;
    animation_model = binding->model;
    clip = animation;
    alternate = animation_model->animations[5];
    pitch = sound->pitch;
    if (clip == alternate)
    {
        val = binding->interaction_timer;
        pitch += pitch >> 3;
        binding->interaction_timer = val - 45;
    }
play_state_sound:
    SoundEffects::PlaySpatialSound(sound, 8192, 0, pitch, world_pos.comp, 0x4B000, 0xFFFFFFFF, 0, 0);
animation_done:
    if (binding->blink_active != 0)
    {
        val = binding->blink_ticks;
        binding->blink_ticks = val + 1;
        if ((sint16)val >= 5)
        {
            binding->blink_ticks = 0;
            flags = flags & 0xFFF7FFFF;
            binding->blink_active = 0;
        }
    }
sample_target:
    player = finish_early == 0 ? CharacterController::FindPlayerObject(0) : 0;
    distance[0] = 0x7FFFF000;
    parent = path_definition;
    if (parent != 0 && parent->transform != 0)
    {
        x = pos.x;
        y = pos.y;
        z = pos.z;
        input.vx = x;
        input.vy = y;
        input.vz = z;
        PushMatrix();
        path_transform = path_definition->transform;
        SetRotMatrix(&path_transform->matrix);
        {
            VECTOR rotation_output;
            ApplyRotMatrixLV(&input, &rotation_output);
            sample_pos.x = rotation_output.vx;
            sample_pos.y = rotation_output.vy;
            sample_pos.z = rotation_output.vz;
        }
        PopMatrix();
        path_transform = path_definition->transform;
        val = sample_pos.x;
        sample_pos.x = val + path_transform->matrix.t[0];
        path_transform = path_definition->transform;
        val = sample_pos.y;
        sample_pos.y = val + path_transform->matrix.t[1];
        path_transform = path_definition->transform;
        val = sample_pos.z;
        sample_pos.z = val + path_transform->matrix.t[2];
    }
    else
    {
        sample_pos = pos;
    }

    x = sample_pos.x;
    x -= (uint32)camera_motion.focus_pos.x;
    y = sample_pos.y;
    y -= (uint32)camera_motion.focus_pos.y;
    z = sample_pos.z;
    z -= (uint32)camera_motion.focus_pos.z;
    x = (sint32)x >> 12;
    y = (sint32)y >> 12;
    z = (sint32)z >> 12;
    sq = x * x + y * y + z * z;

    target = camera_motion.focus_pos;
    distance[0] = sq;
    if (finish_early != 0)
        return 0;
    binding->distance_sq = sq;
    if ((player->path_flags & 0x100) != 0 && resource_model->target_avoidance_mode == 0 && (sint32)distance[0] <= 409599)
    {
        y = camera_motion.focus_pos.y;
        z = sample_pos.y;
        if ((sint32)z >= (sint32)(y - 8192))
        {
            kind = actor_target_avoidance_mode;
            if (kind == 0 || kind == 2)
                distance[0] = 409600;
            if (actor_target_avoidance_mode < 2)
            {
                x = target.x;
                val = sample_pos.x;
                target.x = x + ((sint32)val < (sint32)x ? 409600 : (uint32)-409600);
                z = target.z;
                val = sample_pos.z;
                target.z = z + ((sint32)val < (sint32)z ? 409600 : (uint32)-409600);
            }
        }
    }
    if ((binding->flags & 10) == 0)
        UpdateActorAction(sample_pos, target, distance);
    angle = binding->desired_pitch;
    if (angle != 0 && (limit = resource_model->pitch_limit) != 0)
    {
        val = angle << 2;
        sample_frame = (sint16)val;
        if (sample_frame > 0)
        {
            if (limit < sample_frame)
                val = limit;
        }
        else if (sample_frame < -limit)
            val = -(uint32)limit;
        orientation[1] = val & 0xFFF;
    }
    else
    {
        val = resource_model->fixed_pitch;
        if (val != 0)
            orientation[1] = val & 0xFFF;
    }
    result = CharacterController::ReadCharacterMovementFlags();
    return result;
}

uint32 AnimatedObject::UpdateActorFacingAndAnimation(const Vec3 *target, const Vec3 &origin, ActorModelDefinition *resource_model)
{
    const PathDefinition *parent;
    FUNCTION_MARKER(0x80032248, "SLUS_005.78");
    void *definition;
    const SoundBankStorage *actor_sounds;
    const ActorModelDefinition *sound_model;
    const SoundSample *selection;
    const SoundSample *prev_sound;

    Vec3 world, delta_vector, sample0, sample1, dir_vector, target_vector;

    const Vec3 *dir;
    const AnimatedObject *owner;
    uint32 x, y, z, a, b, c, val, cur_flags, idx;
    PathTransform *path_transform;
    uint32 slot;
    sint32 minimum;
    uint32 frequency_zero;
    uint32 random;
    uint32 divisor;
    uint32 pitch;
    uint32 scale;
    sint32 ratio;
    uint32 radius;
    uint32 option;
    uint32 result;
    AnimationHeader *clip, *sel_animation;
    sint32 angle, step, sample_frame, next_frame, end_frame, mode, limit, voice;
    parent = path_definition;
    definition = ((const ActorPlacement *)parent)->state;
    ActorBinding *binding = (ActorBinding *)definition;
    if (parent != 0 && (parent->flags & 8) != 0 && binding->rotation_track != 0)
    {
        if (velocity.x != 0 || velocity.y != 0 || velocity.z != 0)
        {
            binding->sampled_velocity = velocity;
        }
        result = SampleObjectPitch(binding->sampled_velocity, 0);
        pitch_scale = result;
        angle = (sint16)result;
        binding->sampled_pitch = angle != 0 && angle != 4096 ? (uint32)angle : 0;
    }
    else
        binding->sampled_pitch = 0;
    if (resource_model->turn_motion_mode != 0 && (flags & 0x02000000) == 0)
    {
        dir = &binding->path_dir;
        step = 4096;
        goto apply_dir;
    }
    if ((binding->action_flags & 0x1000) != 0)
    {
        owner = CharacterModelRecord::GetCharacterModelObject();
        parent = owner->path_definition;
        if (parent != 0 && parent->transform != 0)
        {
            sample0 = owner->pos;
            PushMatrix();
            path_transform = owner->path_definition->transform;
            SetRotMatrix(&path_transform->matrix);
            {
                VECTOR sdk_input = {sample0.x, sample0.y, sample0.z, 0};
                VECTOR sdk_output;
                ApplyRotMatrixLV(&sdk_input, &sdk_output);
                world.x = sdk_output.vx;
                world.y = sdk_output.vy;
                world.z = sdk_output.vz;
            }
            PopMatrix();
            path_transform = owner->path_definition->transform;
            val = world.x;
            x = path_transform->matrix.t[0];
            world.x = val + x;
            path_transform = owner->path_definition->transform;
            val = world.y;
            x = path_transform->matrix.t[1];
            world.y = val + x;
            path_transform = owner->path_definition->transform;
            val = world.z;
            x = path_transform->matrix.t[2];
            world.z = val + x;
        }
        else
        {
            world = owner->pos;
        }
        val = world.x;
        x = origin.x;
        delta_vector.x = val - x;
        val = world.y;
        x = origin.y;
        delta_vector.y = val - x;
        val = world.z;
        x = origin.z;
        delta_vector.z = val - x;
        dir = &delta_vector;
        step = 4096;
        goto apply_dir;
    }
    if ((binding->action_effect_flags & 0x800) != 0 && (velocity.x != 0 || velocity.y != 0 || velocity.z != 0))
    {
        ReverseObjectFacingFromVelocity();
        goto after_dir;
    }
    mode = behavior;
    if ((object_operations.behaviors[mode].member_handler == &AnimatedObject::UpdateAttackEnemy))
    {
        resource_model->turn_step = 4096;
        dir = &velocity;
        step = 4096;
        goto apply_dir;
    }
    mode = binding->route_dir_mode;
    if (mode != 0)
    {
        if (mode == 1)
        {
            step = resource_model->turn_step;
            dir = &binding->route_dir;
        }
        else
        {
            sample0 = binding->route_dir;
            x = sample0.x;
            z = sample0.z;
            sample0.x = -x;
            sample0.z = -z;
            step = resource_model->turn_step;
            dir = &sample0;
        }
        goto apply_dir;
    }
    if ((binding->action_flags & 0x100) != 0 && binding->action_speed == 0)
    {
        parent = path_definition;
        if (parent->pos_track.keys != 0)
        {
            cur_flags = flags;
            sample_frame = path_frame;
            if ((cur_flags & 0x10) != 0)
            {
                next_frame = sample_frame + 1;
                if (sample_frame >= parent->pos_track.frame_count)
                {
                    next_frame = sample_frame;
                    --sample_frame;
                }
            }
            else
            {
                next_frame = sample_frame - 1;
                if (sample_frame <= 0)
                {
                    next_frame = sample_frame;
                    ++sample_frame;
                }
            }
            parent = path_definition;
            result = PathDefinition::SampleDefinitionHeading(sample_frame, next_frame, parent);
            parent = path_definition;
            target_yaw = result;
            parent->SampleActorPos(sample_frame, sample0, 64);
            path_definition->SampleActorPos(next_frame, sample1, 64);
            x = sample1.x;
            a = sample0.x;
            y = sample1.y;
            b = sample0.y;
            z = sample1.z;
            c = sample0.z;
            dir_vector.x = x - a;
            dir_vector.y = y - b;
            dir_vector.z = z - c;
            step = resource_model->turn_step;
            dir = &dir_vector;
            goto apply_dir;
        }
    }
    if ((flags & 0x40) != 0 && (velocity.x != 0 || velocity.y != 0 || velocity.z != 0))
    {
        step = resource_model->turn_step;
        dir = &velocity;
        goto apply_dir;
    }
    binding->motion_speed = 0;
    if (target == 0)
        goto after_dir;
    parent = path_definition;
    if ((parent->flags & 0x40) == 0)
    {
        mode = resource_model->kind_override;
        if (mode == -1)
        {
            if (binding->model_kind != 19)
                goto after_dir;
        }
        else if (mode != 19)
            goto after_dir;
    }
    val = target->x;
    x = origin.x;
    target_vector.x = val - x;
    val = target->y;
    x = origin.y;
    target_vector.y = val - x;
    val = target->z;
    x = origin.z;
    target_vector.z = val - x;
    step = resource_model->turn_step;
    dir = &target_vector;
apply_dir:
    TurnObjectFacing(*dir, step);
after_dir:
    cur_flags = binding->flags;
    if ((cur_flags & 2) != 0)
    {
        if (binding->interaction_timer > 0)
        {
            sample_frame = frame;
            if (sample_frame != -1)
            {
                clip = animation;
                val = (uint32)sample_frame + 2;
                frame = val;
                end_frame = clip->frame_count;
                if ((sint16)val >= end_frame)
                    frame = (uint32)end_frame - 1;
            }
        }
        else
        {
            sample_frame = frame;
            if (sample_frame != 0)
                frame = (uint32)sample_frame - 1;
        }
        goto finish;
    }
    if ((cur_flags & 8) != 0)
    {
        if (resource_model->interaction_animation_mode == -1)
        {
            sample_frame = frame;
            if (sample_frame != -1)
            {
                clip = animation;
                val = (uint32)sample_frame + 2;
                frame = val;
                end_frame = clip->frame_count;
                if ((sint16)val >= end_frame)
                    frame = (uint32)end_frame - 1;
            }
        }
        else
            frame = resource_model->interaction_frame;
        mode = character_pad.action_disp_phase;
        if (mode < 0)
            mode = -mode;
        idx = mode % 16;
        sample_frame = resource_model->interaction_limit_step;
        val = character_pad.action_heading;
        limit = 1;
        if (sample_frame != 0 && (character_action_flags & 0x1000) == 0)
            limit = 10;
        x = binding->interaction_heading_ticks + 1;
        binding->interaction_heading_ticks = x;
        if ((sint32)x >= limit)
        {
            binding->interaction_heading_ticks = 0;
            x = OriginalDefaults::interaction_wave[idx];
            val = (val + ((sint32)(x << 16) >> 19)) & 0xFFF;
        }
        sample0.y = 0;
        x = SinQ14(val);
        sample0.x = -(uint32)((sint32)x >> 2);
        z = SinQ14((1024 - val));
        sample0.z = -(uint32)((sint32)z >> 2);
        TurnObjectFacing(sample0, 4096);
        goto finish;
    }
    if (resource_model->kind >= 47)
        goto finish;
    if (resource_model->sound_table == 0)
        goto advance_frame;
    frequency_zero = resource_model->sound_interval == 0;
    sample_frame = frame;
    selection = 0;
    if (sample_frame == -1 && frequency_zero != 0)
    {
        prev_sound = binding->sound;
        voice = binding->sound_voice;
        if ((sound_effects.IsPlaying(prev_sound, voice) & 0xFF) != 0)
            SoundEffects::StopSoundVoice(binding->sound_voice);
        binding->sound = 0;
        binding->sound_voice = -1;
    }
    mode = resource_model->kind;
    if (mode == 17)
    {
        clip = animation;
        sel_animation = resource_model->animations[1];
        if (clip == sel_animation)
        {
            if (frame == 19)
            {
                actor_sounds = resource_model->sound_table;
                if (actor_sounds != 0 && actor_sounds->header.count >= 3)
                    selection = actor_sounds->samples + 2;
                binding->sound_voice = -1;
                goto sound_sel;
            }
            clip = animation;
        }
    }
    else
        clip = animation;
    if (clip == resource_model->animations[0])
    {
        slot = 4;
        minimum = 1;
        goto random_sound_gate;
    }
    if (clip == resource_model->animations[2] && resource_model->kind != 17)
    {
        slot = 20;
        minimum = 3;
        goto random_sound_gate;
    }
    if (frame > 0)
        goto sound_sel;
    clip = animation;
    sel_animation = resource_model->animations[3];
    if (clip != sel_animation)
        goto sound_sel;
    slot = 12;
    minimum = 2;
    goto lookup_sound;
random_sound_gate:
    voice = binding->sound_voice;
    if (voice < 0 && frequency_zero == 0)
    {
        random = random_generator.Next();
        divisor = resource_model->sound_interval;
        if (divisor == 0)
            xport_mips_break(7);
        if (random % divisor != 0)
            goto sound_sel;
    }
lookup_sound:
    actor_sounds = resource_model->sound_table;
    if (actor_sounds != 0 && actor_sounds->header.count >= minimum)
        selection = actor_sounds->samples + ((slot - 4) / 8);
sound_sel:
    if (selection == 0)
        goto advance_frame;
    prev_sound = binding->sound;
    option = 0;
    if (selection != prev_sound && prev_sound != 0)
    {
        voice = binding->sound_voice;
        if ((sound_effects.IsPlaying(prev_sound, voice) & 0xFF) != 0)
            SoundEffects::StopSoundVoice(binding->sound_voice);
        binding->sound = 0;
        binding->sound_voice = -1;
    }
    voice = binding->sound_voice;
    scale = 4096;
    if (voice >= 0)
        pitch = binding->sound_pitch;
    else
    {
        pitch = selection->pitch;
        if (pitch == 0)
        {
            mode = binding->model_kind;
            PrintSdkDiagnostic("Monster type %d bad SFX rate.\n", mode);
            goto advance_frame;
        }
        random = random_generator.Next();
        divisor = pitch >> 2;
        if (divisor == 0)
            xport_mips_break(7);
        pitch = pitch + random % divisor - (pitch >> 3);
        binding->sound_pitch = pitch;
    }
    if ((flags & 0x30000000) != 0)
    {
        val = (uint16)state;
        if ((sint32)val < 2048)
            val = 2048;
        scale = mul12((sint32)scale, (sint32)val);
        if (val == 0)
            xport_mips_break(7);
        ratio = 0x01000000 / (sint32)val;
        pitch = mul12((sint32)pitch, ratio);
    }
    if (frequency_zero != 0)
        option = 1;
    binding->sound = selection;
    sound_model = binding->model;
    radius = (uint32)sound_model->sound_radius << 5;
    voice = binding->sound_voice;
    result = SoundEffects::PlaySpatialSound(selection, (scale + 4096), 0, pitch, world_pos.comp, radius, voice, frequency_zero, option);
    binding->sound_voice = result;
advance_frame:
    clip = animation;
    val = 0;
    if (clip != 0)
    {
        sample_frame = frame;
        end_frame = clip->frame_count - 1;
        if (sample_frame < end_frame)
            val = (uint32)sample_frame + 1;
    }
    frame = val;
finish:
    result = ClampAnimationFrame();
    return result;
}

uint32 AnimatedObject::CollectActorContact(SceneNode *other_node, CollisionContact *contact_data)
{
    FUNCTION_MARKER(0x80032DDC, "SLUS_005.78");
    SceneNode *other = other_node;
    uint32 val, cur_flags, contact_distance, category, resource, movement, angle, x, z, timeout;
    ActorBinding *binding;
    sint32 kind, variant, result, auxiliary;
    if (!contact_data)
    {
        category = other->kind;
        if (category)
        {
            if (category != 2)
                return 0;
            kind = (sint8)(other->state_flags >> 8);
            if (kind == 13)
            {
                val = game_tick;
                timeout = ((const SceneTrigger *)other)->parameters.reaction.deadline;
                if (val >= timeout)
                    return 1;
            }
            if (kind == 14 || (uint8)(kind - 22) < 2)
                return 1;
            if ((uint8)(kind - 40) >= 16 || (other->flags & 0x80))
                return 0;
            if ((((const SceneTrigger *)other)->parameters.event.command >> 24) & 0xFE)
                return 0;
            return ((((const SceneTrigger *)other)->parameters.event.schedule >> 12) & 31) == 2;
        }
        kind = ((const AnimatedObject *)other)->contact_kind;
        if (kind == 3)
        {
            variant = (sint8)((const AnimatedObject *)other)->animation_binding.word;
            if (variant == -2)
                return 0;
            binding = (ActorBinding *)(((const ActorPlacement *)path_definition)->state);
            if (binding->model_kind < 47 || variant != -1)
                return 1;
        }
        if (kind == 1)
        {
            if (behavior != 24)
                return 1;
            if (path_definition->flags & 0x40)
                return 1;
        }
        if (kind != 2 || ((const AnimatedObject *)other)->behavior != 19)
            return 0;
        if (behavior == 19)
            return 0;
        return (((const AnimatedObject *)other)->flags & 0x40) != 0;
    }
    binding = (ActorBinding *)(((const ActorPlacement *)path_definition)->state);
    category = other->kind;
    contact_distance = contact_data->fraction;
    if (category)
    {
        kind = (sint8)(other->state_flags >> 8);
        if (kind == 13)
        {
            cur_flags = binding->reaction_flags | 8;
            binding->reaction_flags = cur_flags;
            val = ((const SceneTrigger *)other)->parameters.reaction.flags;
            binding->reaction_flags = (cur_flags & 0xFFFF01FF) | ((val & 0x7F) << 9);
            timeout = ((const SceneTrigger *)other)->parameters.reaction.flags >> 16;
            val = game_tick;
            ((SceneTrigger *)other)->parameters.reaction.deadline = val + timeout;
        }
        else if (kind == 14)
        {
            cur_flags = binding->reaction_flags | 0x10;
            binding->reaction_flags = cur_flags;
            return (cur_flags & 0x80) == 0;
        }
        else if (kind == 22)
        {
            if (contact_distance)
                return 0;
            flags = flags | 0x40000;
        }
        else if (kind == 23)
        {
            if (contact_distance)
                return 0;
            node.state_flags = ((const SceneTrigger *)other)->parameters.state_flags;
        }
        else if (!contact_distance)
            other->state_flags = (other->state_flags & 0xFF00) | (uint8)((other->state_flags & 0xFF) | 8);
        else
        {
            if ((binding->reaction_flags & 4) && (sint32)contact_distance >= binding->respawn_ticks)
                return 0;
            cur_flags = binding->reaction_flags;
            binding->event_node = other_node;
            binding->respawn_ticks = contact_distance;
            binding->reaction_flags = cur_flags | 4;
        }
        return 0;
    }
    kind = ((const AnimatedObject *)other)->contact_kind;
    if (kind == 3)
    {
        resource = ((const AnimatedObject *)other)->state;
        if (resource == 19)
        {
            if ((player_interaction.flags & 0x20) && binding->model->contact_mode)
            {
                val = binding->flags;
                if (!(val & 2))
                {
                    binding->flags = val | 2;
                    binding->interaction_timer = 0;
                }
                player_interaction.target = binding;
                return 1;
            }
        }
        else if (resource == 11)
        {
            if (flags & 0x30000000)
                return 1;
            binding->reaction_flags = binding->reaction_flags | 1;
            return 1;
        }
        else if (behavior == 17 || (flags & 0x30000000))
        {
            binding->reaction_flags = binding->reaction_flags | 0x20;
            return 1;
        }
        movement = flags & 0x05800000;
        if (movement != 0x01800000)
        {
            if (!movement || (movement & 0x04000000))
                goto mark_push;
            z = ((const AnimatedObject *)other)->velocity.z;
            x = ((const AnimatedObject *)other)->velocity.x;
            angle = Atan2Angle(z, x);
            val = (uint16)yaw - angle;
            if (movement & 0x00800000)
            {
                if ((uint16)(val + 1024) >= 2049)
                    goto mark_push;
            }
            else if ((movement & 0x01000000) && (uint16)(val + 1023) < 2047)
                goto mark_push;
        }
        goto finish_contact;
    }
    if (kind != 1)
    {
        binding->reaction_flags = binding->reaction_flags | 0x60;
        goto finish_contact;
    }
    if (behavior == 17 || (flags & 0x10000000))
    {
        binding->reaction_flags = binding->reaction_flags | 0x22;
        return 1;
    }
    if (!contact_distance && (((const AnimatedObject *)other)->flags & 0x02000000))
        goto mark_contact;
    result = ((const AnimatedObject *)other)->ClassifyCharacterContactResponse();
    val = ((const AnimatedObject *)other)->path_flags;
    auxiliary = 0;
    if (val & 1)
        auxiliary = ((const AnimatedObject *)other)->ClassifyConstrainedContact(this);
    if (result <= 0 && ((behavior == 20 && (flags & 0x40)) || (contact_data->selection < 2048 && result == 0) || (((const AnimatedObject *)other)->flags & 0x18000000) || (flags & 0x04400000) == 0x00400000))
    {
        if (auxiliary && behavior != 20)
            binding->reaction_flags = binding->reaction_flags | 8;
    }
    else
    {
        cur_flags = binding->reaction_flags;
        binding->reaction_flags = cur_flags | (result > 0 ? 0x20 : 8);
    }
    if (character_pad.path_transition.contact_delay > 0)
        binding->reaction_flags = binding->reaction_flags & 0xFFFFFFD7;
mark_contact:
    cur_flags = character_pad.secondary_flags;
    if (cur_flags & 0x40)
        binding->reaction_flags = binding->reaction_flags | 0x20000;
    else if (cur_flags & 0x200)
        binding->reaction_flags = (binding->reaction_flags | 1) & 0xFFFFFFDF;
    binding->reaction_flags = binding->reaction_flags | 2;
    goto finish_contact;
mark_push:
    binding->reaction_flags = binding->reaction_flags | 8;
finish_contact:
    cur_flags = binding->reaction_flags;
    if (cur_flags & 8)
    {
        if ((contact_data->flags & 1) || (behavior == 20 && !(flags & 0x40)))
        {
            val = binding->flags;

            cur_flags = (val & 2) ? cur_flags & 0xFFFF01F7 : (cur_flags & 0xFFFF01FF) | 0x200;
        }
        else
            cur_flags &= 0xFFFFFFF7;
        binding->reaction_flags = cur_flags;
        if ((binding->reaction_flags & 8) && !(character_pad.secondary_flags & 0x40))
            character_pad.path_transition.contact_delay = 5;
    }
    return kind != 1;
}

intptr AnimatedObject::UpdateActorPathMotion()
{
    const PathDefinition *parent;
    AnimationHeader *sel_animation;
    FUNCTION_MARKER(0x80033564, "SLUS_005.78");
    void *data;
    const ActorModelDefinition *speed_model;
    const ActorModelDefinition *resource_model;
    const SoundSample *prev_sound;

    VECTOR sample_pos, local;

    uint32 output_distance, output_height;
    uint16 output_angle;
    uint32 cur_flags, val, result;
    PathTransform *path_transform;
    uint32 x;
    uint32 y;
    uint32 z;
    uint32 initial;
    uint32 scene;
    uint32 counter;
    uint32 eligible;
    uint32 distance;
    uint32 height;
    sint32 target;
    uint32 acceleration;
    sint32 kind;
    parent = path_definition;
    data = ((const ActorPlacement *)parent)->state;
    ActorBinding *binding = (ActorBinding *)data;
    val = (uint16)ticks;
    resource_model = binding->model;
    initial = val == 0;
    if (initial != 0)
    {
        path_motion_scale = 15;
        kind = resource_model->kind_override;
        if (kind == -1)
            kind = binding->model_kind;
        if (kind < 44)
        {
            prev_sound = binding->sound;
            x = binding->sound_voice;
            result = sound_effects.IsPlaying(prev_sound, x);
            if ((result << 24) != 0)
                SoundEffects::StopSoundVoice(binding->sound_voice);
            binding->sound = 0;
            binding->sound_voice = -1;
        }
        else
        {
            sel_animation = binding->model->animations[1];
            animation = sel_animation;
            x = sel_animation->frame_count;
            val = path_motion_scale;
            frame = -1;
            path_motion_scale = val + x;
        }
        cur_flags = binding->flags;
        if ((cur_flags & 2) != 0)
        {
            const ActorModelDefinition *reaction_model = binding->model;
            binding->flags = cur_flags ^ 2;
            sel_animation = reaction_model->animations[6];
            cur_flags = player_interaction.flags;
            frame = -1;
            player_interaction.flags = cur_flags & 0xFFFFFFF3;
            velocity.x = 4096;
            animation = sel_animation;
            AnimatedObject::SpawnActionEffect(this, 0, 5);
            path_motion_scale = (uint32)animation->frame_count + 15;
        }
        kind = behavior;
        movement_mode = 0;
        contact_kind = 0;
        behavior_suspended = (object_operations.behaviors[kind].member_handler == &AnimatedObject::UpdateAttackEnemy) ? 0 : 2;
        val = node.flags;
        velocity.x = 4096;
        node.flags = val & 0xFE;
        goto world_pos;
    }
    val = binding->reaction_flags;
    cur_flags = binding->flags;
    binding->reaction_flags = val & 0x80;
    binding->motion_frame = path_frame;
    result = cur_flags & 8;
    if ((cur_flags & 2) != 0)
        goto done;
    if (result != 0)
    {
        velocity.z = 0;
        velocity.y = 0;
        velocity.x = 0;
    }
    if ((binding->flags & 0x10) == 0)
        goto movement;
world_pos:
    parent = path_definition;
    if (parent != 0 && parent->transform != 0)
    {
        x = pos.x;
        y = pos.y;
        z = pos.z;
        local.vx = x;
        local.vy = y;
        local.vz = z;
        PushMatrix();
        path_transform = path_definition->transform;
        SetRotMatrix(&path_transform->matrix);
        ApplyRotMatrixLV(&local, &sample_pos);
        PopMatrix();
        path_transform = path_definition->transform;
        x = sample_pos.vx;
        x += (uint32)path_transform->matrix.t[0];
        sample_pos.vx = x;
        path_transform = path_definition->transform;
        y = sample_pos.vy;
        y += (uint32)path_transform->matrix.t[1];
        sample_pos.vy = y;
        path_transform = path_definition->transform;
        z = sample_pos.vz;
        z += (uint32)path_transform->matrix.t[2];
        sample_pos.vz = z;
    }
    else
    {
        x = pos.x;
        y = pos.y;
        z = pos.z;
        sample_pos.vx = x;
        sample_pos.vy = y;
        sample_pos.vz = z;
    }
    if (initial != 0)
    {
        x = world_pos.x;
        x -= (uint32)sample_pos.vx;
        spheres[0].pos.x = x;
        y = world_pos.y;
        y -= (uint32)sample_pos.vy;
        spheres[0].pos.y = y;
        z = world_pos.z;
        z -= (uint32)sample_pos.vz;
        spheres[0].pos.z = z;
        result = binding->event_command;
        if (result != 0)
        {
            scene = OriginalDefaults::scene_context_identity;
            counter = scene_events.deferred_count;
            scene_events.deferred_count = counter + 1;
            val = binding->event_command;
            result = scene + ((uint32)(sint8)counter << 2);
            scene_events.deferred_entries[(sint8)counter] = (val & 0xFFF) | 0x4000;
        }
        goto done;
    }
    x = sample_pos.vy;
    val = binding->interaction_height;
    val = x - val;
    if (val == 0)
    {
        binding->flags = binding->flags & 0xEF;
        velocity.y = 0;
    }
    else
    {
        val = (sint32)val >= 4097 ? 0xFFFFF000 : -val;
        velocity.y = val;
        cur_flags = flags;
        height = velocity.y;
        flags = cur_flags | 0x400;
        ComputePathMotion(0, height, NULL);
        flags = flags & 0xFFFFFBFF;
    }
movement:
    result = binding->flags & 8;
    if (result != 0)
        goto done;
    result = flags & 0x02000040;
    eligible = result == 0x40;
    if (result == 0x02000040)
    {
        val = resource_model->reaction_motion_mode;
        result = 1;
        if (val == 1)
        {
            result = ticks;
            if (result != 0)
                eligible = 1;
        }
    }
    if (eligible == 0)
        goto zero_velocity;
    result = path_definition->pos_track.keys != 0;
    if (result != 0 && binding->route_dir_mode != 0)
    {
        flags = flags & 0xFFFFF7FF;
        result = UpdateActorTurnaround(binding, resource_model);
        if (result == 0)
            goto zero_velocity;
        result = ComputePathMotion(result, binding->path_pitch_speed, NULL);
        goto done;
    }
    parent = path_definition;
    result = parent->pos_track.keys != 0;
    if (result == 0)
        goto zero_velocity;
    if ((parent->flags & 0x40) == 0)
    {
        kind = resource_model->kind_override;
        result = 19;
        if (kind == -1)
        {
            if (binding->model_kind != 19)
                goto zero_velocity;
        }
        else if (kind != 19)
            goto zero_velocity;
    }
    flags = flags & 0xFFFFF7FF;
    kind = resource_model->kind_override;
    if (kind == -1)
        kind = binding->model_kind;
    if (kind == 10)
        distance = binding->motion_speed;
    else
    {
        speed_model = binding->model;
        cur_flags = flags;
        val = binding->action_flags;
        if ((cur_flags & 0x08000000) != 0)
            target = speed_model->motion_speed[1];
        else
            target = speed_model->motion_speed[0];
        if ((val & 0x100) != 0)
            distance = binding->action_speed;
        else
        {
            acceleration = speed_model->motion_accel;
            distance = target;
            if (acceleration != 0)
            {
                distance = binding->motion_speed;
                if ((sint32)distance < target)
                {
                    distance += acceleration;
                    if (target < (sint32)distance)
                        distance = target;
                }
            }
        }
        binding->motion_speed = distance;
    }
    result = binding->action_effect_flags & 0x200;
    binding->desired_pitch = 0;
    if (result != 0)
        CalculateActorTargetMotion(distance, &output_distance, &output_height, &output_angle);
    else
    {
        result = binding->action_flags & 0x800;
        if (result == 0)
            goto direct_movement;
        result = binding->flags & 8;
        if (result != 0)
            goto direct_movement;
        CalculateActorPathHeightMotion(distance, &output_distance, &output_height, &output_angle);
    }
    val = output_height;
    binding->path_pitch_speed = val;
    result = resource_model->pitch_limit;
    distance = output_distance;
    if (result != 0)
    {
        result = output_angle;
        binding->desired_pitch = result;
    }
    else
        binding->desired_pitch = 0;
    if (distance == 0)
        goto zero_velocity;
    flags = flags | 0x400;
    ComputePathMotion(distance, binding->path_pitch_speed, NULL);
    result = flags & 0xFFFFFBFF;
    flags = result;
    goto done;
direct_movement:
    if (distance == 0)
        goto zero_velocity;
    result = ComputePathMotion(distance, binding->path_pitch_speed, NULL);
    goto done;
zero_velocity:
    velocity.z = 0;
    velocity.y = 0;
    velocity.x = 0;
done:
    return result;
}

intptr AnimatedObject::ResetActorInstance()
{
    AnimationHeader *initial_animation;
    FUNCTION_MARKER(0x80033D7C, "SLUS_005.78");
    void *data;
    ActorModelDefinition *table;
    uint32 cur_flags, count, val, x, y, z;
    const PathDefinition *parent;
    const ActorBinding *kind_binding;
    Vec3 pos0;
    Vec3 pos1;
    intptr result;
    ActorModelDefinition *definition = 0;
    sint32 kind, sample_frame;
    parent = path_definition;
    cur_flags = flags;
    data = ((const ActorPlacement *)parent)->state;
    ActorBinding *binding = (ActorBinding *)data;
    parent = path_definition;
    flags = (cur_flags & 0x01C08024) | 0x20010;
    kind_binding = (const ActorBinding *)(((const ActorPlacement *)parent)->state);
    count = actor_models.count;
    kind = kind_binding->model_kind;
    table = actor_models.definitions;
    while (count != 0)
    {
        val = table->kind;
        --count;
        if ((sint32)val == kind)
        {
            definition = table;
            break;
        }
        ++table;
    }
    initial_animation = definition->animations[0];
    frame = -1;
    movement_mode = 4;
    contact_kind = 2;
    behavior_suspended = 0;
    animation = initial_animation;
    unbound_animation_word = 0;
    val = (uint8)definition->initial_health;
    parent = path_definition;
    ticks = (sint8)val;
    if (parent->pos_track.keys != 0)
    {
        val = PathDefinition::SampleDefinitionHeading(0, 1, parent);
        target_yaw = val;
        yaw = val;
        binding->path_heading = val;
        parent = path_definition;
        parent->SampleActorPos(0, pos0, 64);
        parent = path_definition;
        parent->SampleActorPos(1, pos1, 64);
        x = pos1.x;
        x -= (uint32)pos0.x;
        y = pos1.y;
        y -= (uint32)pos0.y;
        z = pos1.z;
        z -= (uint32)pos0.z;
        binding->path_dir.x = x;
        binding->path_dir.y = y;
        binding->path_dir.z = z;
    }
    else
    {
        yaw = 0;
        binding->path_heading = 0;
        binding->path_dir.z = 0;
        binding->path_dir.y = 0;
        binding->path_dir.x = 0;
        binding->path_dir.z = ONE;
    }
    if (definition->turn_motion_mode == 0)
        yaw = random_generator.Next() & 0xFFF;
    parent = path_definition;
    if (parent->pos_track.keys != 0)
    {
        const ActorBinding *frame_binding = (const ActorBinding *)(((const ActorPlacement *)parent)->state);
        const PathDefinition *definition0 = path_definition;
        sample_frame = frame_binding->initial_path_frame;
        path_frame = sample_frame;
        AnimationTrack::SampleVectorTrack(sample_frame, &definition0->pos_track, pos);
    }
    else
    {
        const PathDefinition *definition1 = parent;
        const PathTransform *captured_path_matrix = definition1->transform;
        if (captured_path_matrix != 0)
        {
            x = captured_path_matrix->matrix.t[0];
            y = captured_path_matrix->matrix.t[1];
            z = captured_path_matrix->matrix.t[2];
        }
        else
        {
            x = definition1->static_pos.x;
            y = definition1->static_pos.y;
            z = definition1->static_pos.z;
        }
        pos.x = x;
        pos.y = y;
        pos.z = z;
    }
    val = node.flags;
    spheres[3].radius = 0xFFFFFFFF;
    spheres[2].radius = 0xFFFFFFFF;
    spheres[1].radius = 0xFFFFFFFF;
    spheres[0].radius = 0xFFFFFFFF;
    node.flags = (val & 0x7D) | 0x40;
    if (definition->path_flag != 0)
        path_flags = path_flags | 0x4000;
    val = binding->flags;
    binding->interaction_timer = 0;
    binding->interaction_height = 0;
    binding->route_dir_mode = 0;
    binding->sampled_velocity.z = 0;
    binding->sampled_velocity.y = 0;
    binding->sampled_velocity.x = 0;
    binding->flags = val & 0xE5;
    InitializeActorAction(binding);
    result = (this->*definition->constructor)();
    return result;
}

intptr AnimatedObject::UpdateActorDeath()
{
    FUNCTION_MARKER(0x80034044, "SLUS_005.78");
    void *data;
    const ActorModelDefinition *animation_model;
    uint32 timer, val, count, sample_frame, result;
    AnimationHeader *clip, *sel_animation;
    const PathDefinition *resource;
    data = ((const ActorPlacement *)path_definition)->state;
    ActorBinding *binding = (ActorBinding *)data;
    if (this == character_action_target)
        character_action_flags = character_action_flags | 0x800;
    timer = death_ticks - 1;
    death_ticks = timer;
    if (timer == 0)
    {
        if ((game_flow.mode_flags & 0x100) && binding->model_kind < 44)
            return ResetActorInstance();
        val = binding->respawn_delay;
        AnimatedObject *action_target = character_action_target;
        binding->respawn_ticks = val;
        if (this == action_target)
            character_action_target = 0;
        resource = path_definition;
        ((PathDefinition *)resource)->node.flags |= 0x80;
        result = node.flags | 2;
        node.flags = result;
        return result;
    }
    if (behavior == 17)
    {
        result = (uint16)frame + 1;
        frame = result;
        render_view.material_override = resource_grps[25].tex;
        return result;
    }
    if ((sint32)timer < 15)
    {
        animation_model = binding->model;
        clip = animation;
        sel_animation = animation_model->animations[6];
        if (clip != sel_animation)
        {
            flags = flags ^ 0x80000;
            animation_model = binding->model;
            clip = animation;
            sel_animation = animation_model->animations[6];
            if (clip != sel_animation)
                velocity.x = mul12(velocity.x, 3513);
            return ClampAnimationFrame();
        }
    }
    sample_frame = (uint16)frame + 1;
    clip = animation;
    frame = sample_frame;
    count = clip->frame_count;
    if ((sint16)sample_frame >= (sint32)count)
        frame = count - 1;
    return ClampAnimationFrame();
}

sint32 AnimatedObject::ClassifyActorPathDir(const Vec3 &pos, const Vec3 &target) const
{
    FUNCTION_MARKER(0x80034958, "SLUS_005.78");
    const PathDefinition *definition = path_definition;

    Vec3 sample;

    sint32 count, frame, lookahead;
    sint16 path_angle, target_angle;
    if (definition->pos_track.keys == 0)
        return 0;
    if (pos.x - (uint32)target.x + 4096 < 8193 && pos.z - (uint32)target.z + 4096 < 8193)
        return 0;
    count = definition->pos_track.frame_count;
    frame = path_frame;
    lookahead = frame + count / 8;
    if (lookahead == frame || lookahead > count - 1)
        lookahead = count - 1;
    definition->SamplePos(lookahead, sample, flags & 0x10);
    path_angle = Atan2Angle((sample.z - (uint32)pos.z), (sample.x - (uint32)pos.x));
    target_angle = Atan2Angle((target.z - (uint32)pos.z), (target.x - (uint32)pos.x));
    return (((path_angle - target_angle + 1024) & 4095) < 2049) ? 1 : -1;
}

uint32 AnimatedObject::UpdateActorTurnaround(ActorBinding *binding, const ActorModelDefinition *resource_model)
{
    FUNCTION_MARKER(0x80034BA8, "SLUS_005.78");

    Vec3 input, output;

    Vec3 target_pos;
    Vec3 world_pos;
    Vec3 difference;
    const PathDefinition *definition;

    uint32 angle;
    sint32 state;
    target_pos = camera_motion.focus_pos;
    definition = path_definition;
    if (definition != 0 && definition->transform != 0)
    {
        PathTransform *path_transform;
        input.x = pos.x;
        input.y = pos.y;
        input.z = pos.z;
        PushMatrix();
        path_transform = path_definition->transform;
        {
            SetRotMatrix(&path_transform->matrix);
        }
        {
            VECTOR sdk_input = {input.x, input.y, input.z, 0};
            VECTOR sdk_output;
            ApplyRotMatrixLV(&sdk_input, &sdk_output);
            output.x = sdk_output.vx;
            output.y = sdk_output.vy;
            output.z = sdk_output.vz;
        }
        PopMatrix();
        path_transform = path_definition->transform;
        world_pos.x = output.x + (uint32)path_transform->matrix.t[0];
        output.x = world_pos.x;
        path_transform = path_definition->transform;
        world_pos.y = output.y + (uint32)path_transform->matrix.t[1];
        output.y = world_pos.y;
        path_transform = path_definition->transform;
        world_pos.z = output.z + (uint32)path_transform->matrix.t[2];
        output.z = world_pos.z;
    }
    else
    {
        world_pos.x = pos.x;
        world_pos.y = pos.y;
        world_pos.z = pos.z;
    }
    difference.x = target_pos.x - (uint32)world_pos.x;
    difference.y = target_pos.y - (uint32)world_pos.y;
    difference.z = target_pos.z - (uint32)world_pos.z;
    angle = Atan2Angle((-(uint32)difference.z), (-(uint32)difference.x));
    angle = (binding->route_heading - angle) & 4095;
    if (angle > 2048)
        angle = (4096 - angle) & 4095;
    state = binding->route_dir_mode;
    if (state == 2)
    {
        uint32 countdown = binding->turn_ticks - 1u;
        binding->turn_ticks = countdown;
        if ((sint8)countdown <= 0)
        {
            binding->route_dir_mode = 3;
            binding->turn_ticks = 4;
        }
    }
    else if (state == 1 && angle < 512)
    {
        definition = path_definition;
        uint32 cur_flags = flags;
        uint16 frame = path_frame;
        uint16 count = definition->pos_track.frame_count;
        sint32 divisor;
        flags = (cur_flags & ~0x800u) ^ 16;
        path_frame = (uint32)count - frame;
        binding->route_dir_mode = 2;
        divisor = resource_model->turn_step;
        if (divisor == 0)
            trap(7);
        binding->turn_ticks = (uint32)(2048 / divisor) + 5;
    }
    else if (state == 3 && angle < 512)
    {
        uint32 countdown;
        if ((binding->reaction_flags & 16) != 0)
            return 4096;
        countdown = binding->turn_ticks - 1u;
        binding->turn_ticks = countdown;
        if ((sint8)countdown > 0)
            return 4096;
        binding->route_dir_mode = 0;
    }
    return 0;
}

uint32 AnimatedObject::GetActorContactDamage() const
{
    FUNCTION_MARKER(0x80034EAC, "SLUS_005.78");
    void *data;
    ActorModelDefinition *table;
    uint32 count;
    sint32 kind;
    ActorModelDefinition *definition = 0;
    data = ((const ActorPlacement *)path_definition)->state;
    ActorBinding *binding = (ActorBinding *)data;
    count = actor_models.count;
    kind = (uint8)binding->model_kind;
    table = actor_models.definitions;
    while (count != 0)
    {
        if (table->kind == kind)
        {
            definition = table;
            break;
        }
        --count;
        ++table;
    }
    return definition->contact_damage;
}

void AnimatedObject::SampleObjectPathDir(uint16 *angle, Vec3 &dir_vector, const AnimatedObject *object, sint32 frame, uint32 reverse)
{
    FUNCTION_MARKER(0x80035058, "SLUS_005.78");
    sint32 frame0 = frame;
    sint32 frame1 = (uint32)frame + 1;
    Vec3 sample0;
    Vec3 sample1;
    uint32 x;
    uint32 y;
    uint32 z;
    uint32 result;
    /* Initialize the adjacent sample for the original undefined reverse-zero branch */
    if (reverse != 0 && object->path_frame >= object->path_definition->pos_track.frame_count)
    {
        frame1 = frame;
        frame0 = (uint32)frame - 1;
    }
    object->path_definition->SampleActorPos(frame0, sample0, reverse);
    object->path_definition->SampleActorPos(frame1, sample1, reverse);
    x = sample1.x - (uint32)sample0.x;
    y = sample1.y - (uint32)sample0.y;
    z = sample1.z - (uint32)sample0.z;
    dir_vector.x = x;
    dir_vector.y = y;
    dir_vector.z = z;
    result = Atan2Angle(z, x);
    *angle = result;
}

void AnimatedObject::CaptureActorRouteDir(uint16 *angle, Vec3 &dir, const AnimatedObject *object)
{
    FUNCTION_MARKER(0x80035154, "SLUS_005.78");
    uint32 x = object->velocity.x;
    uint32 y = object->velocity.y;
    uint32 z = object->velocity.z;
    uint32 result;
    dir.x = x;
    dir.y = y;
    dir.z = z;
    result = Atan2Angle(object->velocity.z, object->velocity.x);
    *angle = result;
}

void AnimatedObject::UpdateActorReactionRecovery()
{
    FUNCTION_MARKER(0x800351A4, "SLUS_005.78");
    ActorBinding *binding = (ActorBinding *)(((const ActorPlacement *)path_definition)->state);
    uint32 flags, val;
    const SoundBankStorage *sound_table;
    const SoundSample *sound;
    if ((sound_effects.IsPlaying(binding->sound, binding->sound_voice) & 0xFF) != 0)
        SoundEffects::StopSoundVoice(binding->sound_voice);
    flags = binding->reaction_flags;
    binding->sound = 0;
    binding->sound_voice = -1;
    if ((flags & 0x28) != 0)
    {
        sound_table = sound_resources.common_table;
        sound = sound_table != 0 && sound_table->header.count >= 23 ? sound_table->samples + 22 : 0;
        sound_effects.Queue(sound, 0, 0, (uint16)sound_settings.effects_volume, 0, 3);
        ticks = 0;
    }
    else if ((flags & 1) != 0)
        state = (state & 0xFFFFFF00) | 0x96;
    else
    {
        val = state - 1;
        state = val;
        if ((val & 0xFF) == 0)
        {
            behavior = (sint32)val >> 8;
            return;
        }
    }
    render_view.material_override = resource_grps[25].tex;
}

void AnimatedObject::InitializeActorAction(ActorBinding *binding)
{
    const PathDefinition *parent;
    FUNCTION_MARKER(0x800352F4, "SLUS_005.78");
    const ActorModelDefinition *bound_model;
    uint32 result, x, y, z;
    Vec3 pos0;
    Vec3 pos1;
    Vec3 delta;
    bound_model = ((const ActorBinding *)(((const ActorPlacement *)path_definition)->state))->model;
    yaw = 0;
    if (bound_model->actions[0].distance_threshold == 0)
    {
        binding->action_idx = -2;
        return;
    }
    ResetActorAction();
    result = bound_model->turn_motion_mode;
    if (result != 0)
        return;
    parent = path_definition;
    result = parent->pos_track.keys != 0;
    if (result == 0)
        return;
    parent->SampleActorPos(0, pos0, 64);
    parent = path_definition;
    parent->SampleActorPos(1, pos1, 64);
    x = pos1.x;
    x -= (uint32)pos0.x;
    y = pos1.y;
    y -= (uint32)pos0.y;
    z = pos1.z;
    z -= (uint32)pos0.z;
    delta.x = x;
    delta.y = y;
    delta.z = z;
    TurnObjectFacing(delta, bound_model->turn_step);
}

uint32 AnimatedObject::CanActorTurnTowardTarget(const Vec3 &pos, const Vec3 &target, const uint32 *distance, const ActorBinding *binding, const ActorModelDefinition *model, uint8 allowed) const
{
    FUNCTION_MARKER(0x80035494, "SLUS_005.78");
    sint32 range;
    uint32 angle;
    if (actor_turnaround_enabled == 0 || model->route_action == 0)
        return 0;
    if (binding->route_mode != 0 && allowed == 0)
        return 0;
    range = (uint32)model->turnaround_range << 5;
    if (range < (sint32)distance[0])
        return 0;
    angle = Atan2Angle((target.z - (uint32)pos.z), (target.x - (uint32)pos.x));
    return (((uint16)yaw - angle + 1024) & 4095) < 2049;
}

void AnimatedObject::UpdateActorAction(const Vec3 &pos, Vec3 &target, uint32 *distance)
{
    const PathDefinition *parent;
    FUNCTION_MARKER(0x8003557C, "SLUS_005.78");
    void *definition;
    const ActorModelDefinition *actions_model;
    const ActorActionDefinition *action, *candidate;
    const AnimatedObject *owner;
    uint8 candidate_flags[6];
    uint32 launch_angles[3];
    uint32 val, cur_flags, mode, cur_state, x, y, z;
    AnimationHeader *clip;
    PathTransform *path_transform;
    uint32 *counter;
    uint32 scan, trigger, threshold, blocked, chosen;

    Vec3 target_saved, spawn_pos, path_delta, input1, cached0, cached1;

    Vec3 *input, *cached_input;
    AnimatedObject *spawned, *saved_spawn;
    uint16 motion_kind, pitch_angle;
    uint8 in_range, event_trigger_seen, projectile_count;
    sint32 target_dir;
    uint32 kind;
    uint32 speed;
    uint32 turn;
    uint32 delay;
    uint32 attack;
    sint32 count;
    uint32 i;
    uint32 len;
    uint32 mag;
    uint32 angle;
    sint32 s;
    sint32 c;
    uint32 result;
    sint32 idx, limit, dir, sample_frame, next_frame, parameter, signed_val, sign_x, sign_z;
    definition = ((const ActorPlacement *)path_definition)->state;
    ActorBinding *binding = (ActorBinding *)definition;

    if (binding->action_idx == -2)
        return;
    if ((binding->flags & 2) != 0)
    {
        ResetActorAction();
        return;
    }
    actions_model = binding->model;
    owner = CharacterModelRecord::GetCharacterModelObject();
    if (binding->turn_ticks != 0)
    {
        ResetActorAction();
        return;
    }
    chosen = 0;
restart_action:
    motion_kind = 0;
restart_mode:
    for (scan = 0; scan < 6; ++scan)
        candidate_flags[scan] = 0;
    mode = binding->route_mode;
    idx = -1;
    limit = 6;
    blocked = 0;
    in_range = 0;
    event_trigger_seen = 0;
    if (mode == 0)
    {
        signed_val = binding->action_idx;
        if (signed_val != -1)
        {
            action = actions_model->actions + signed_val;
            val = action->priority_mode;
            if (val == 1)
                limit = (uint16)signed_val;
            else
            {
                if (val == 2)
                {
                    clip = actions_model->animations[(uint32)action->animation];
                    if (animation == clip && action->priority_frame >= frame)
                        goto retain_action_priority;
                    val = action->priority_mode;
                }
                if (val == 3 && animation == actions_model->animations[(uint32)action->animation] && frame >= action->priority_frame)
                {
                retain_action_priority:
                    limit = (uint16)binding->action_idx;
                }
                else
                    blocked = 1;
            }
        }
        scan = 0;
        candidate = actions_model->actions;
        counter = binding->trigger_ticks;
        while (candidate->distance_threshold != 0)
        {
            if ((sint8)idx != -1)
                break;
            threshold = candidate->distance_threshold;
            if ((sint32)distance[0] < (sint32)threshold || (sint32)threshold < 0)
            {
                trigger = candidate->trigger;
                val = 0;
                in_range = 1;
                if (trigger == 7)
                    candidate_flags[scan] = 1;
                if ((sint32)scan < limit && blocked == 0)
                {
                    switch (candidate->trigger)
                    {
                        case 0:
                            val = 1;
                            break;
                        case 1:
                            if ((owner->flags & 0x400) != 0)
                                val = 1;
                            break;
                        case 2:
                            cur_flags = gameplay_input.buttons;
                            if ((cur_flags & (*control_bindings.active_scheme)[0]) != 0)
                                val = 1;
                            break;
                        case 3:
                            if ((binding->reaction_flags & 8) != 0)
                            {
                                cur_flags = binding->action_flags;
                                if ((cur_flags & 0x10) == 0)
                                {
                                    val = 1;
                                    binding->action_flags = cur_flags | 0x10;
                                }
                            }
                            break;
                        case 4:
                            cur_flags = binding->pending_event;
                            event_trigger_seen = 1;
                            if (cur_flags != 0xFFFFFFFF)
                            {
                                val = 1;
                                binding->pending_event = 0xFFFFFFFF;
                            }
                            break;
                        case 5:
                            if (candidate->trigger_ticks >= ticks)
                                val = 1;
                            break;
                        case 6:
                            if (binding->range_ticks >= (uint32)candidate->trigger_ticks)
                            {
                                val = 1;
                                binding->range_ticks = 0;
                            }
                            break;
                        case 7:
                            if (*counter >= (uint32)candidate->trigger_ticks)
                            {
                                val = 1;
                                *counter = 0;
                            }
                            break;
                    }
                    if (val != 0)
                    {
                        idx = scan;
                        action = candidate;
                        binding->action_idx = scan;
                        binding->action_phase = 0;
                    }
                }
            }
            ++candidate;
            ++counter;
            scan += 1;
        }
        goto sel_action;
    }
    dir = ClassifyActorTargetDir(pos, target);
    target_dir = dir;
    if (binding->route_mode == 2 && binding->action_idx < 0)
        ResetActorAction();
    mode = binding->route_mode;
    if (mode == 1)
    {
        ResetActorAction();
        if (actions_model->route_action != 0)
        {
            binding->route_mode = 2;
            binding->action_phase = 0;
            binding->action_idx = ((uint8)actions_model->route_action) - 1;
        }
        else
        {
            clip = binding->model->animations[0];
            frame = -1;
            animation = clip;
            if (target_dir != -1)
                binding->route_mode = 4;
            else
            {
                binding->route_mode = 3;
                parent = path_definition;
                cur_flags = flags;
                x = (uint16)path_frame;
                y = (uint16)parent->pos_track.frame_count;
                flags = (cur_flags & 0xFFFFF7FF) ^ 0x10;
                path_frame = y - x;
                parameter = actions_model->turn_motion_mode;
                motion_kind = 1;
                if (parameter != 0)
                    binding->turn_delay = 0;
                else
                {
                    parameter = actions_model->turn_step;
                    if (parameter == 0)
                        xport_mips_break(7);
                    delay = (uint32)(2048 / parameter) - 2;
                    if ((delay & 0x80) != 0)
                        delay = 0;
                    binding->turn_delay = delay;
                }
            }
        }
    }
    else if (mode == 3)
    {
        val = (binding->turn_delay - 1) & 0xFF;
        binding->turn_delay = val;
        if (val == 0)
            binding->route_mode = 4;
    }
    mode = binding->route_mode;
    if (mode == 4)
    {
        threshold = (uint32)actions_model->turnaround_range << 5;
        if (target_dir != -1 && (sint32)distance[0] < (sint32)threshold)
        {
            binding->action_flags = binding->action_flags | 0x1100;
            flags = (flags & 0xFFFFF7BF) | 0x40;
            binding->action_speed = 0;
            goto sel_action;
        }
        ResetActorAction();
        motion_kind = 0;
        goto restart_mode;
    }
    if (mode == 2)
        action = actions_model->actions + binding->action_idx;
sel_action:
    idx = binding->action_idx;
    binding->action_effect_flags = 0;
    if (idx == -1)
        goto update_counters;
    action = actions_model->actions + idx;
    dir = ClassifyActorTargetDir(pos, target);
    target_dir = dir;
    if (binding->action_phase == 0)
    {
        cur_flags = binding->action_flags;
        binding->action_phase = 1;
        binding->action_flags = cur_flags & 0xFFFE;
        parameter = action->motion;
        turn = (parameter == 3 || parameter == 6 || parameter == 8) && target_dir == -1;
        if (turn == 0)
        {
            val = (uint16)action->motion;
            turn = (val - 4 < 2 || (sint16)val == 7) && target_dir == 1;
        }
        if (turn != 0)
        {
            parent = path_definition;
            cur_flags = flags;
            x = (uint16)path_frame;
            y = (uint16)parent->pos_track.frame_count;
            flags = (cur_flags & 0xFFFFF7FF) ^ 0x10;
            path_frame = y - x;
            parameter = actions_model->turn_motion_mode;
            motion_kind = 1;
            if (parameter == 0)
            {
                parameter = actions_model->turn_step;
                if (parameter == 0)
                    xport_mips_break(7);
                delay = (uint32)(2048 / parameter) - 2;
                if ((delay & 0x80) != 0)
                    delay = 0;
                binding->turn_delay = delay;
            }
            else
                binding->turn_delay = 0;
        }
        else
            binding->turn_delay = 0;
        if (action->projectile_kind == -1)
            binding->projectile_phase = 0;
        else
        {
            cur_flags = binding->action_flags;
            binding->projectile_phase = 1;
            binding->next_projectile_frame = 0;
            binding->action_flags = cur_flags & 0xFFF3;
        }
    }
    cur_state = binding->action_phase;
    if (cur_state == 1)
    {
        val = binding->turn_delay;
        if (val != 0)
        {
            binding->turn_delay = val - 1;
            motion_kind = 1;
            val = binding->turn_delay;
        }
        if (val == 0)
        {
            clip = actions_model->animations[(uint32)action->animation];
            cur_flags = flags;
            frame = -1;
            flags = cur_flags | 0x10000;
            animation = clip;
            binding->action_phase = 4;
        }
        cur_state = binding->action_phase;
    }
    if (cur_state == 2)
    {
        clip = animation;
        if (clip != actions_model->animations[(uint32)action->animation])
            binding->action_phase = 7;
        else if (frame != clip->frame_count - 1)
            binding->action_phase = 4;
        motion_kind = 8;
    }
    else if (cur_state == 3)
    {
        if (animation != actions_model->animations[(uint32)action->animation])
            binding->action_phase = 7;
        if (frame == animation->frame_count - 1)
            binding->action_phase = action->hold_ticks != 0 ? 5 : 7;
        else
        {
            val = (binding->turn_delay - 1) & 0xFF;
            binding->turn_delay = val;
            if (val == 0)
                motion_kind = 1;
            else
                binding->action_phase = 4;
        }
    }
    if (binding->action_phase == 4)
    {
        cur_flags = binding->action_flags;
        motion_kind = (cur_flags & 1) != 0 ? (uint16)action->next_motion : (uint16)action->motion;
        if (animation != actions_model->animations[(uint32)action->animation])
            binding->action_phase = 7;
        else
        {
            cur_flags = binding->action_flags;
            if ((cur_flags & 1) == 0 && action->switch_frame != 0 && frame >= action->switch_frame)
            {
                binding->action_flags = cur_flags | 1;
                parameter = action->next_motion;
                turn = (parameter == 3 || parameter == 8) && target_dir != 1;
                if (turn == 0)
                {
                    if (parameter == 3 || parameter == 8)
                        parameter = action->next_motion;
                    turn = ((parameter == 4 || parameter == 5) && target_dir == 1) || (parameter == 7 && action->motion == 6);
                }
                if (turn == 0)
                    motion_kind = action->next_motion;
                else
                    goto reverse_action;
            }
            else if (frame == animation->frame_count - 1)
                binding->action_phase = action->hold_ticks != 0 ? 5 : 7;
            else if (motion_kind == 8 && target_dir != 1)
            {
            reverse_action:
                binding->action_phase = 3;
                parent = path_definition;
                cur_flags = flags;
                x = (uint16)path_frame;
                y = (uint16)parent->pos_track.frame_count;
                flags = (cur_flags & 0xFFFFF7FF) ^ 0x10;
                path_frame = y - x;
                parameter = actions_model->turn_motion_mode;
                motion_kind = 1;
                if (parameter != 0)
                    binding->turn_delay = 0;
                else
                {
                    parameter = actions_model->turn_step;
                    if (parameter == 0)
                        xport_mips_break(7);
                    delay = (uint32)(2048 / parameter) - 2;
                    if ((delay & 0x80) != 0)
                        delay = 0;
                    binding->turn_delay = delay;
                }
            }
        }
    }
    if (binding->action_phase == 5)
    {
        binding->action_phase = 6;
        clip = actions_model->animations[(uint32)action->hold_animation];
        cur_flags = flags;
        frame = -1;
        animation = clip;
        flags = cur_flags | 0x10000;
        state_delay = action->hold_ticks;
        parameter = action->hold_motion;
        turn = (parameter == 3 || parameter == 8) && target_dir != 1;
        if (turn == 0)
        {
            if (parameter == 3 || parameter == 8)
                parameter = action->hold_motion;
            turn = (parameter == 4 || parameter == 5) && target_dir == 1;
        }
        if (turn != 0)
        {
            parent = path_definition;
            cur_flags = flags;
            x = (uint16)path_frame;
            y = (uint16)parent->pos_track.frame_count;
            flags = (cur_flags & 0xFFFFF7FF) ^ 0x10;
            path_frame = y - x;
            parameter = actions_model->turn_motion_mode;
            motion_kind = 1;
            if (parameter != 0)
                binding->turn_delay = 0;
            else
            {
                parameter = actions_model->turn_step;
                if (parameter == 0)
                    xport_mips_break(7);
                delay = (uint32)(2048 / parameter) - 2;
                if ((delay & 0x80) != 0)
                    delay = 0;
                binding->turn_delay = delay;
            }
        }
    }
    if (binding->action_phase == 6)
    {
        val = state_delay - 1;
        state_delay = val;
        if (val == 0)
            binding->action_phase = 7;
        else
        {
            val = binding->turn_delay;
            if (val != 0)
            {
                binding->turn_delay = val - 1;
                motion_kind = 1;
                val = binding->turn_delay;
            }
            if (val == 0)
            {
                if (action->hold_speed == 0)
                    motion_kind = 1;
                else if (action->hold_motion == 4 && target_dir == 1)
                {
                    parent = path_definition;
                    cur_flags = flags;
                    x = (uint16)path_frame;
                    y = (uint16)parent->pos_track.frame_count;
                    flags = (cur_flags & 0xFFFFF7FF) ^ 0x10;
                    path_frame = y - x;
                    parameter = actions_model->turn_motion_mode;
                    motion_kind = 1;
                    if (parameter != 0)
                        binding->turn_delay = 0;
                    else
                    {
                        parameter = actions_model->turn_step;
                        if (parameter == 0)
                            xport_mips_break(7);
                        delay = (uint32)(2048 / parameter) - 2;
                        if ((delay & 0x80) != 0)
                            delay = 0;
                        binding->turn_delay = delay;
                    }
                }
                else
                    motion_kind = action->hold_motion;
            }
        }
    }
    if (binding->action_phase == 7)
    {
        clip = actions_model->animations[0];
        cur_flags = flags;
        frame = -1;
        animation = clip;
        flags = cur_flags | 0x10000;
        binding->action_idx = -1;
        motion_kind = 9;
        if (binding->route_mode != 0)
            binding->route_mode = 0;
    }
    if (binding->action_phase == 6)
        speed = action->hold_speed;
    else if ((binding->action_flags & 1) != 0)
        speed = action->next_speed;
    else
        speed = action->speed;
    kind = motion_kind;
    if (kind == 0)
        goto free_motion;
    if (kind == 1)
    {
        binding->action_flags = binding->action_flags & 0xFEFF;
        flags = flags & 0xFFFFFFBF;
    }
    else if (kind >= 2 && kind <= 8)
    {
        parent = path_definition;
        threshold = (uint32)parent->pos_track.frame_count - actor_action_end_margin;
        if (path_frame >= (sint32)threshold)
        {
            val = 0;
            if (actor_turnaround_enabled != 0 && actions_model->route_action != 0 && binding->route_mode == 0 && (sint32)((uint32)actions_model->turnaround_range << 5) >= (sint32)distance[0])
            {
                x = target.x - (uint32)pos.x;
                target_saved.x = x;
                y = target.y - (uint32)pos.y;
                target_saved.y = y;
                z = target.z - (uint32)pos.z;
                target_saved.z = z;
                angle = Atan2Angle(z, x);
                val = (((uint16)yaw - angle + 1024) & 0xFFF) < 0x801;
            }
            if (val != 0)
                binding->route_mode = 1;
            else
            {
                binding->action_flags = binding->action_flags | 0x100;
                flags = (flags & 0xFFFFF7BF) | 0x40;
                binding->action_speed = kind <= 4 ? speed : 0;
            }
        }
        else
        {
            binding->action_flags = binding->action_flags | 0x100;
            flags = (flags & 0xFFFFF7BF) | 0x40;
            binding->action_speed = speed;
        }
    }
    else
    {
        parent = path_definition;
        if (path_frame < parent->pos_track.frame_count - 1)
        {
        free_motion:
            binding->action_flags = binding->action_flags & 0xFEFF;
            flags = (flags & 0xFFFFF7BF) | 0x40;
        }
        else
        {
            binding->action_flags = binding->action_flags & 0xFEFF;
            flags = flags & 0xFFFFFFBF;
        }
    }
    if (binding->action_phase == 7)
    {
        val = chosen;
        chosen = 1;
        if (val == 0)
            goto restart_action;
    }
    if (animation == actions_model->animations[(uint32)action->animation])
    {
        cur_state = binding->projectile_phase;
        if (cur_state == 2)
        {
            if (action->projectile_frame - 1 >= frame)
            {
                cur_flags = binding->action_flags;
                binding->projectile_phase = 1;
                binding->next_projectile_frame = 0;
                binding->action_flags = cur_flags & 0xFFF3;
            }
            cur_state = binding->projectile_phase;
        }
        if (cur_state == 1)
        {
            cur_flags = binding->action_flags;
            trigger = 0;
            if ((cur_flags & 4) == 0)
            {
                if (frame >= action->projectile_frame - 1)
                {
                    binding->action_flags = cur_flags | 4;
                    x = (uint8)action->projectile_frame;
                    y = (uint8)action->projectile_interval;
                    binding->next_projectile_frame = x + y;
                    trigger = 1;
                }
            }
            else
            {
                val = action->projectile_end_frame;
                if (val != 0 && (sint32)(val - 1) < frame)
                    binding->projectile_phase = 2;
                else
                {
                    val = binding->next_projectile_frame;
                    if (frame >= (sint32)val - 1)
                    {
                        binding->next_projectile_frame = val + (uint8)action->projectile_interval;
                        trigger = 1;
                    }
                }
            }
            if (trigger == 1)
            {
                if (target_dir != 1)
                {
                    target_saved = target;
                    x = -(uint32)target.x;
                    z = -(uint32)target.z;
                    target.x = x;
                    target.z = z;
                }
                if (action->projectile_interval == 0)
                    binding->projectile_phase = 2;
                state = action->projectile_kind;
                flags = flags | 0x100;
                spawned = object_operations.behaviors[6].factory_handler(this);
                cur_flags = flags;
                saved_spawn = spawned;
                flags = cur_flags & 0xFFFFFEFF;

                spawn_pos = spawned->pos;
                spawned = saved_spawn;
                val = action->projectile_height;
                val = spawned->pos.y + val;
                spawned->pos.y = val;
                spawn_pos.y = val;
                parameter = yaw;
                speed = action->projectile_offset;
                val = -(uint32)(SinQ14(parameter) >> 2);
                val = mul12((sint32)speed, (sint32)val);
                spawned->pos.x = spawned->pos.x + val;
                parameter = (sint16)(1024 - (uint16)yaw);
                speed = action->projectile_offset;
                spawned = saved_spawn;
                val = -(uint32)(SinQ14(parameter) >> 2);
                val = mul12((sint32)speed, (sint32)val);
                spawned->pos.z = spawned->pos.z + val;
                parameter = action->aim_mode;
                if (parameter < 0 || parameter > 3)
                    goto attack_finished;
                if (parameter > 0 && parameter < 3)
                {
                    CharacterController::FindPlayerObject(0);
                    launch_angles[0] = action->spread_angle;
                    parameter = action->aim_mode;
                    pitch_angle = 0;
                    if (parameter == 1)
                        projectile_count = 1;
                    else
                    {
                        launch_angles[1] = 0;
                        launch_angles[2] = 4096 - action->spread_angle;
                        x = binding->sampled_velocity.x >> 6;
                        y = binding->sampled_velocity.y >> 6;
                        z = binding->sampled_velocity.z >> 6;
                        val = x * x + y * y + z * z;
                        projectile_count = 3;
                        if ((sint32)val < 0)
                            len = SquareRoot12((val >> 2)) << 1;
                        else
                            len = SquareRoot12(val);
                        if ((flags & 0x10) == 0)
                            len = -len;
                        angle = AsinAngle(binding->sampled_velocity.y, len);
                        pitch_angle = angle;
                    }
                    i = 0;
                    if (projectile_count == 0)
                        goto attack_finished;
                    attack = 1;
                    input = &input1;
                    cached_input = &cached1;
                }
                else
                {
                    attack = parameter == 3 ? 3 : 0;
                    input = attack == 0 ? &path_delta : &input1;
                    cached_input = &cached0;
                    i = 0;
                }
            next_projectile:
                if (attack == 1 && (sint32)i > 0)
                {
                    flags = flags | 0x100;
                    spawned = object_operations.behaviors[6].factory_handler(this);
                    cur_flags = flags;
                    saved_spawn = spawned;
                    flags = cur_flags & 0xFFFFFEFF;
                    val = spawned->pos.y;
                    spawned->pos.y = val + action->projectile_height;
                }
                spawned = saved_spawn;
                parent = spawned->path_definition;
                if (parent != 0 && parent->transform != 0)
                {
                    *input = spawned->pos;
                    PushMatrix();
                    spawned = saved_spawn;
                    path_transform = spawned->path_definition->transform;
                    SetRotMatrix(&path_transform->matrix);
                    {
                        VECTOR rotation_input = {input->x, input->y, input->z, 0};
                        VECTOR rotation_output;
                        ApplyRotMatrixLV(&rotation_input, &rotation_output);
                        spawned->pos.x = rotation_output.vx;
                        spawned->pos.y = rotation_output.vy;
                        spawned->pos.z = rotation_output.vz;
                    }
                    PopMatrix();
                    spawned = saved_spawn;
                    parent = spawned->path_definition;
                    path_transform = parent->transform;
                    val = (uint32)spawned->pos.x + path_transform->matrix.t[0];
                    parent = spawned->path_definition;
                    spawned->pos.x = val;
                    path_transform = parent->transform;
                    val = (uint32)spawned->pos.y + path_transform->matrix.t[1];
                    parent = spawned->path_definition;
                    spawned->pos.y = val;
                    path_transform = parent->transform;
                    val = path_transform->matrix.t[2];
                    spawned->pos.z = spawned->pos.z + val;
                }
                if (attack == 3)
                {
                    spawned = saved_spawn;
                    cur_flags = spawned->flags;
                    spawned->movement_mode = 0;
                    spawned = saved_spawn;
                    spawned->flags = cur_flags | 0x2000;
                    spawned->velocity.y = 0;
                    goto attack_finished;
                }
                spawned = saved_spawn;
                parent = spawned->path_definition;
                if (parent != 0 && parent->transform != 0)
                {
                    *cached_input = spawn_pos;
                    PushMatrix();
                    spawned = saved_spawn;
                    path_transform = spawned->path_definition->transform;
                    SetRotMatrix(&path_transform->matrix);
                    {
                        VECTOR rotation_input = {cached_input->x, cached_input->y, cached_input->z, 0};
                        {
                            VECTOR sdk_output;
                            ApplyRotMatrixLV(&rotation_input, &sdk_output);
                            spawn_pos.x = sdk_output.vx;
                            spawn_pos.y = sdk_output.vy;
                            spawn_pos.z = sdk_output.vz;
                        }
                    }
                    PopMatrix();
                    spawned = saved_spawn;
                    path_transform = spawned->path_definition->transform;
                    spawn_pos.x = spawn_pos.x + (uint32)path_transform->matrix.t[0];
                    path_transform = spawned->path_definition->transform;
                    spawn_pos.y = spawn_pos.y + (uint32)path_transform->matrix.t[1];
                    path_transform = spawned->path_definition->transform;
                    spawn_pos.z = spawn_pos.z + (uint32)path_transform->matrix.t[2];
                }
                spawned = saved_spawn;
                cur_flags = spawned->flags;
                spawned->movement_mode = 0;
                spawned->flags = cur_flags | 0x2000;
                if (attack == 0)
                    CharacterController::FindPlayerObject(0);
                spawned = saved_spawn;
                spawned->velocity.x = target.x - (uint32)spawn_pos.x;
                spawned->velocity.y = target.y - (uint32)spawn_pos.y;
                z = target.z - (uint32)spawn_pos.z;
                spawned->velocity.z = z;
                if (attack == 0)
                {
                    mag = spawned->velocity.y;
                    if ((sint32)mag < 0)
                        mag = -mag;
                }
                x = spawned->velocity.x;
                val = (uint32)mul12((sint32)x, (sint32)x) + (uint32)mul12((sint32)z, (sint32)z);
                if ((sint32)val < 0)
                {
                    x = spawned->velocity.x;
                    val = (uint32)mul12((sint32)x, (sint32)x) + (uint32)mul12((sint32)z, (sint32)z);
                    len = SquareRoot12((val >> 2)) << 1;
                }
                else
                {
                    x = spawned->velocity.x;
                    val = (uint32)mul12((sint32)x, (sint32)x) + (uint32)mul12((sint32)z, (sint32)z);
                    len = SquareRoot12(val);
                }
                if (attack == 0)
                {
                    val = (sint32)len >> 1;
                    distance[0] = val;
                    if ((sint32)mag >= (sint32)val)
                    {
                        if (mag == 0)
                            xport_mips_break(7);
                        signed_val = 0x40000000 / (sint32)mag;
                        val = ((sint64)(sint32)val * signed_val) >> 18;
                        spawned = saved_spawn;
                        x = spawned->velocity.y;
                        fixed_math_result = val;
                        spawned->velocity.y = mul12((sint32)x, (sint32)val);
                    }
                    goto attack_finished;
                }
                s = SinQ14(launch_angles[0]);
                spawned = saved_spawn;
                signed_val = (sint16)(s >> 2);
                spawned->velocity.y = mul12((sint32)len, signed_val);
                parent = path_definition;
                const AnimationTrack *rotation_track;
                rotation_track = 0;
                if (parent != 0 && (parent->flags & 8) != 0)
                {
                    rotation_track = ((const ActorBinding *)(((const ActorPlacement *)parent)->state))->rotation_track;
                    sign_x = 1;
                }
                if (rotation_track != 0)
                {
                    sign_z = 1;
                    x = spawned->pos.y;
                    y = action->projectile_height;
                    z = spawned->velocity.x;
                    spawned->pos.y = x - y;
                    if ((sint32)z < 0)
                    {
                        sign_x = -1;
                        spawned->velocity.x = -z;
                    }
                    spawned = saved_spawn;
                    z = spawned->velocity.z;
                    if ((sint32)z < 0)
                    {
                        sign_z = -1;
                        spawned->velocity.z = -z;
                    }
                    signed_val = actor_launch_angle_selector;
                    if (signed_val == -1)
                        signed_val = i;
                    angle = (launch_angles[signed_val] + pitch_angle) & 0xFFF;
                    s = SinQ14(angle);
                    c = SinQ14((1024 - angle));
                    s = s >> 2;
                    c = c >> 2;
                    spawned = saved_spawn;
                    x = spawned->velocity.x;
                    z = spawned->velocity.z;
                    val = mul12((sint32)(x + z), c);
                    x = spawned->velocity.x;
                    spawned->velocity.y = val;
                    val = mul12((sint32)x, s);
                    z = spawned->velocity.z;
                    spawned->velocity.x = val;
                    spawned->velocity.z = mul12((sint32)z, s);
                    if (sign_x == -1)
                        spawned->velocity.x = -(uint32)spawned->velocity.x;
                    if (sign_z == -1)
                    {
                        spawned = saved_spawn;
                        spawned->velocity.z = -(uint32)spawned->velocity.z;
                    }
                }
                else if (i == 1)
                {
                    spawned = saved_spawn;
                    spawned->velocity.x = 0;
                    spawned->velocity.z = 0;
                }
                else if (i == 2)
                {
                    spawned = saved_spawn;
                    x = spawned->velocity.x;
                    z = spawned->velocity.z;
                    spawned->velocity.x = -x;
                    spawned->velocity.z = -z;
                }
                count = projectile_count;
                i += 1;
                if ((sint32)i < count)
                    goto next_projectile;
            attack_finished:
                if (target_dir != 1)
                {
                    target = target_saved;
                }
                spawned = saved_spawn;

                binding->projectile_velocity = spawned->velocity;
            }
        }
    }
    if (action->path_motion_mode == 1 && path_definition->pos_track.keys != 0)
    {
        binding->action_effect_flags = binding->action_effect_flags | 2;
        sample_frame = path_frame;
        if ((flags & 0x10) != 0)
        {
            parameter = path_definition->pos_track.frame_count;
            next_frame = sample_frame + 1;
            if (sample_frame == parameter - 1)
            {
                sample_frame = parameter - 2;
                next_frame = sample_frame + 1;
            }
        }
        else
        {
            next_frame = sample_frame - 1;
            if (sample_frame == 0)
            {
                sample_frame = 1;
                next_frame = 0;
            }
        }
        path_definition->SampleActorPos(sample_frame, target_saved, 64);
        path_definition->SampleActorPos(next_frame, spawn_pos, 64);
        x = spawn_pos.x;
        val = target_saved.x;
        y = spawn_pos.y;
        cur_flags = target_saved.y;
        z = spawn_pos.z;
        threshold = target_saved.z;
        x -= val;
        y -= cur_flags;
        z -= threshold;
        path_delta.x = x;
        path_delta.y = y;
        path_delta.z = z;
        x = (sint32)x >> 6;
        y = (sint32)y >> 6;
        z = (sint32)z >> 6;
        val = x * x + y * y + z * z;
        if ((sint32)val < 0)
            len = SquareRoot12((val >> 2)) << 1;
        else
            len = SquareRoot12(val);
        angle = AsinAngle(path_delta.y, len);
        orientation[1] = (-angle) & 0xFFF;
    }
    if (binding->action_idx != -1)
    {
        if (binding->action_phase >= 5)
            val = (uint8)action->flags & 4;
        else if ((binding->action_flags & 1) != 0)
            val = (uint8)action->flags & 2;
        else
            val = (uint8)action->flags & 1;
        if (val != 0)
        {
            cur_flags = binding->action_effect_flags;
            kind = motion_kind;
            binding->action_effect_flags = cur_flags | 0x200;
            if (kind == 4 || kind == 7 || kind == 5)
            {
                binding->action_effect_flags = binding->action_effect_flags | 0x400;
                goto check_motion_flag;
            }
        }
        else
        {
        check_motion_flag:
            if (motion_kind == 5)
                binding->action_effect_flags = binding->action_effect_flags | 0x800;
        }
        if (event_trigger_seen == 1)
            binding->action_effect_flags = binding->action_effect_flags | 0x100;
        if ((action->flags & 0x10) != 0)
            binding->action_effect_flags = binding->action_effect_flags | 0x1000;
    }
update_counters:
    if ((actions_model->action_counter_flags & 1) != 0)
    {
        if (in_range == 1)
        {
            val = binding->range_ticks;
            cur_flags = binding->action_flags & 0xFFFD;
            binding->range_ticks = val + 1;
            binding->action_flags = cur_flags;
        }
        else if (binding->range_ticks != 0)
        {
            cur_flags = binding->action_flags;
            if ((cur_flags & 2) != 0)
                binding->range_ticks = 0;
            else
                binding->action_flags = cur_flags | 2;
        }
        for (scan = 0; scan < 6; ++scan)
        {
            if (candidate_flags[scan] != 0)
                binding->trigger_ticks[scan] = binding->trigger_ticks[scan] + 1;
            else
                binding->trigger_ticks[scan] = 0;
        }
    }
    cur_flags = binding->action_flags;

    if ((cur_flags & 0x10) != 0)
    {
        result = cur_flags & 0xFFEF;
        if (behavior != 16)
            binding->action_flags = result;
    }
}

ActorBinding *AnimatedObject::ResetActorAction()
{
    FUNCTION_MARKER(0x800378A0, "SLUS_005.78");
    void *data = ((const ActorPlacement *)path_definition)->state;
    ActorBinding *binding = (ActorBinding *)data;
    binding->action_flags = 0;
    binding->action_effect_flags = 0;
    binding->action_idx = -1;
    binding->action_reset_half = 0;
    binding->route_mode = 0;
    return data != 0 ? binding : 0;
}

void AnimatedObject::CalculateActorTargetMotion(uint32 distance, uint32 *distance_ret, uint32 *height_ret, uint16 *angle_ret) const
{
    FUNCTION_MARKER(0x800378C8, "SLUS_005.78");
    void *data = ((const ActorPlacement *)path_definition)->state;
    ActorBinding *binding = (ActorBinding *)data;
    const AnimatedObject *owner = CharacterModelRecord::GetCharacterModelObject();

    Vec3 input, t;

    Vec3 target_pos;
    Vec3 sample_pos;
    Vec3 difference;
    uint32 result, angle = 0;
    const PathDefinition *definition;
    PathTransform *path_transform;
    uint32 horizontal = distance, vertical = 0;
    target_pos.x = camera_motion.focus_pos.x;
    target_pos.y = camera_motion.focus_pos.y;
    target_pos.z = camera_motion.focus_pos.z;
    definition = path_definition;
    if (definition != 0 && definition->transform != 0)
    {
        input.x = pos.x;
        input.y = pos.y;
        input.z = pos.z;
        PushMatrix();
        path_transform = path_definition->transform;
        {
            SetRotMatrix(&path_transform->matrix);
        }
        {
            VECTOR sdk_input = {input.x, input.y, input.z, 0};
            VECTOR sdk_output;
            ApplyRotMatrixLV(&sdk_input, &sdk_output);
            t.x = sdk_output.vx;
            t.y = sdk_output.vy;
            t.z = sdk_output.vz;
        }
        PopMatrix();
        path_transform = path_definition->transform;
        sample_pos.x = t.x + (uint32)path_transform->matrix.t[0];
        t.x = sample_pos.x;
        path_transform = path_definition->transform;
        sample_pos.y = t.y + (uint32)path_transform->matrix.t[1];
        t.y = sample_pos.y;
        path_transform = path_definition->transform;
        sample_pos.z = t.z + (uint32)path_transform->matrix.t[2];
        t.z = sample_pos.z;
    }
    else
    {
        sample_pos.x = pos.x;
        sample_pos.y = pos.y;
        sample_pos.z = pos.z;
    }
    difference.x = sample_pos.x - (uint32)target_pos.x;
    difference.y = sample_pos.y - (uint32)target_pos.y;
    difference.z = sample_pos.z - (uint32)target_pos.z;
    if ((binding->action_effect_flags & 0x400) == 0)
    {
        sint32 threshold = actor_target_height_tolerance;
        if (difference.y < -threshold)
            vertical = distance;
        result = threshold < difference.y;
        if (result != 0)
            vertical = -distance;
    }
    else if (pos.y < target_pos.y && difference.y < actor_target_height_limit)
    {
        if (owner->velocity.y < 0)
            vertical = -distance;
        else
        {
            result = difference.y < -40960;
            if (result != 0)
                vertical = -distance;
        }
    }
    else
    {
        result = -actor_target_height_limit < difference.y;
        if (result != 0)
        {
            sint32 velocity = owner->velocity.y;

            if (velocity >= 0)
                vertical = distance;
            else
            {
                result = difference.y > 40960;
                if (result != 0)
                    vertical = distance;
            }
        }
    }
    if (vertical != 0)
    {
        uint32 height = difference.y;
        uint32 reach;
        sint32 wave;
        uint32 flags = binding->action_flags;
        binding->desired_pitch = 0;
        binding->action_flags = flags | 0x800;
        if ((sint32)height < 0)
            height = -height;
        if (difference.z < difference.x)
            reach = (uint32)difference.x + (difference.z / 2);
        else
            reach = (difference.x / 2) + (uint32)difference.z;
        angle = Atan2Angle(reach, height);
        if ((sint32)vertical < 0)
            angle = -angle;
        wave = SinQ14(angle) >> 2;
        vertical = mul12((sint32)distance, wave);
        wave = SinQ14((1024u - angle)) >> 2;
        horizontal = mul12((sint32)distance, wave);
    }
    *height_ret = vertical;
    *distance_ret = horizontal;
    *angle_ret = angle;
}

void AnimatedObject::CalculateActorPathHeightMotion(uint32 distance, uint32 *distance_ret, uint32 *height_ret, uint16 *angle_ret) const
{
    FUNCTION_MARKER(0x80037C50, "SLUS_005.78");
    const PathDefinition *definition = path_definition;
    void *data = ((const ActorPlacement *)definition)->state;
    ActorBinding *binding = (ActorBinding *)data;
    uint32 vertical = 0, result;
    if (actor_path_height_enabled == 0)
    {
        result = binding->action_flags & 0xF7FF;
        binding->action_flags = result;
    }
    else
    {
        Vec3 sample_pos, input;

        Vec3 floor;

        PathTransform *path_transform;
        if (definition != 0 && definition->transform != 0)
        {
            input.x = pos.x;
            input.y = pos.y;
            input.z = pos.z;
            PushMatrix();
            path_transform = path_definition->transform;
            {
                SetRotMatrix(&path_transform->matrix);
            }
            {
                VECTOR sdk_input = {input.x, input.y, input.z, 0};
                VECTOR sdk_output;
                ApplyRotMatrixLV(&sdk_input, &sdk_output);
                sample_pos.x = sdk_output.vx;
                sample_pos.y = sdk_output.vy;
                sample_pos.z = sdk_output.vz;
            }
            PopMatrix();
            path_transform = path_definition->transform;
            sample_pos.x = sample_pos.x + (uint32)path_transform->matrix.t[0];
            path_transform = path_definition->transform;
            sample_pos.y = sample_pos.y + (uint32)path_transform->matrix.t[1];
            path_transform = path_definition->transform;
            sample_pos.z = sample_pos.z + (uint32)path_transform->matrix.t[2];
        }
        else
        {
            sample_pos.x = pos.x;
            sample_pos.y = pos.y;
            sample_pos.z = pos.z;
        }
        result = path_definition->FindHeightFrame(sample_pos, flags & 16, 4096000);
        if (result != -1)
        {
            uint32 dir = flags & 16;
            sint32 pos_y;
            uint32 floor_y;
            sint32 difference;
            result = path_definition->ProjectFrame(result, sample_pos, floor, dir, 4096);
            if (result != -1)
            {
                uint32 tolerance = actor_path_height_tolerance;
                floor_y = floor.y;
                pos_y = sample_pos.y;
                difference = floor_y - pos_y;
                if ((difference < (sint32)tolerance && (sint32)(-tolerance) < difference) || (sint16)binding->height_dir_changes >= 5)
                {
                    result = binding->action_flags & 0xF7FF;
                    binding->action_flags = result;
                }
                else
                {
                    uint32 cur_flags = binding->action_flags;
                    result = cur_flags & 32;
                    if (pos_y < (sint32)floor_y)
                    {
                        vertical = distance;
                        if (result == 0)
                        {
                            result = binding->height_dir_changes;
                            binding->action_flags = cur_flags | 32;
                            ++result;
                            binding->height_dir_changes = result;
                        }
                        else
                            binding->height_dir_changes = 0;
                    }
                    else
                    {
                        vertical = -distance;
                        if (result != 0)
                        {
                            result = binding->height_dir_changes;
                            binding->action_flags = cur_flags & 0xFFDF;
                            ++result;
                            binding->height_dir_changes = result;
                        }
                        else
                            binding->height_dir_changes = 0;
                    }
                }
            }
        }
    }
    *height_ret = vertical;
    *distance_ret = distance;
    *angle_ret = 0;
}

void AnimatedObject::ReverseObjectFacingFromVelocity()
{
    FUNCTION_MARKER(0x80037F08, "SLUS_005.78");
    Vec3 vector;
    uint32 result;
    vector.x = -(uint32)velocity.x;
    vector.z = -(uint32)velocity.z;
    vector.y = velocity.y;
    TurnObjectFacing(vector, 4096);
    result = ((uint16)yaw + 2048) & 4095;
    target_yaw = result;
    yaw = result;
}

sint32 AnimatedObject::ClassifyActorTargetDir(const Vec3 &pos, const Vec3 &target) const
{
    FUNCTION_MARKER(0x80037F78, "SLUS_005.78");

    Vec3 sample;

    const PathDefinition *parent;
    uint32 point_x, cur_flags, x, z, angle0, angle1;
    sint32 count, frame, next_frame, result;
    parent = path_definition;
    if (parent->pos_track.keys == 0)
        goto no_dir;
    x = pos.x - (uint32)target.x + 4096;
    if (x < 0x2001)
    {
        z = pos.z - (uint32)target.z + 4096;
        if (z < 0x2001)
            goto no_dir;
    }
    parent = path_definition;
    count = parent->pos_track.frame_count;
    frame = path_frame;
    next_frame = frame + count / 8;
    if (next_frame == frame || next_frame > count - 1)
        next_frame = count - 1;
    cur_flags = flags;
    parent = path_definition;
    parent->SamplePos(next_frame, sample, cur_flags & 0x10);
    z = sample.z;
    x = pos.z;
    point_x = sample.x;
    cur_flags = pos.x;
    angle0 = Atan2Angle((z - x), (point_x - cur_flags));
    z = target.z;
    x = pos.z;
    point_x = target.x;
    cur_flags = pos.x;
    angle1 = Atan2Angle((z - x), (point_x - cur_flags));
    result = ((angle0 - angle1 + 1024) & 0xFFF) < 0x801 ? 1 : -1;
    return result;
no_dir:
    return 0;
}

sint32 AnimatedObject::FindSphereFaceCol(const CollisionSphere *geometry, const SceneNode *other, const CollisionTransform *transform, uint32 *fraction_limit) const
{
    const PathTransform *path_transform;
    FUNCTION_MARKER(0x800380AC, "SLUS_005.78");
    VECTOR start, end, point;
    const MeshFace *polygon;
    const MeshVertex *vertex;
    const MeshVertex *vertex_base;
    const MeshFace *face_base, *face, *sel_face = 0;
    uint32 shape, edge, i, radius, radius_sq, margin, val;
    sint32 face_idx;
    uint32 motion_x, motion_y, motion_z, x, y, z, sx, sy, sz, ex, ey, ez;
    uint32 vx;
    uint32 vy;
    uint32 vz;
    uint32 nx;
    uint32 ny;
    uint32 nz;
    sint32 dx;
    sint32 dy;
    sint32 dz;
    sint32 cx;
    sint32 cy;
    sint32 cz;
    uint32 px;
    uint32 py;
    uint32 pz;
    uint32 dot;
    sint32 start_distance;
    uint32 end_distance;
    uint32 fraction;
    sint32 edge_radius;
    uint32 edge_sq;
    uint32 len_sq;
    uint32 bound;
    sint32 face_count = transform->face_count;
    const CollisionSphere *sphere, *sel_sphere = 0;
    sint32 rcp, angle, result;
    if (face_count <= 0)
        return -1;
    const MeshGeometryNode *mesh = other != 0 ? (const MeshGeometryNode *)other : NULL;
    const MeshGeometryNode *vertices_owner = mesh;
    face_base = mesh->polygons + transform->first_face;
    if (mesh->vertex_count <= 0)
        vertices_owner = mesh->src_mesh;
    vertex_base = vertices_owner->vertices;
    motion_x = velocity.x - (uint32)transform->motion.x;
    motion_y = velocity.y - (uint32)transform->motion.y;
    motion_z = velocity.z - (uint32)transform->motion.z;
    const PathDefinition *definition = path_definition;
    if (definition != 0)
    {
        const PathTransform *captured_path_transform = definition->transform;
        if (captured_path_transform != 0 && (flags & 0x2000) == 0)
        {
            path_transform = captured_path_transform;
            motion_x += (uint32)path_transform->displacement.x;
            motion_y += (uint32)path_transform->displacement.y;
            motion_z += (uint32)path_transform->displacement.z;
        }
    }
    for (shape = 0; shape < 4; ++shape)
    {
        sphere = &geometry[shape];
        radius = sphere->radius;
        if (radius == 0xFFFFFFFF)
            continue;
        val = mul12((sint32)radius, 4014);
        radius_sq = mul12((sint32)val, (sint32)val);
        margin = mul12((sint32)radius, 3686);
        x = sphere->pos.x - (uint32)transform->pos.x;
        end.vx = x;
        y = sphere->pos.y - (uint32)transform->pos.y;
        end.vy = y;
        z = sphere->pos.z - (uint32)transform->pos.z;
        end.vz = z;
        PushMatrix();
        {
            MATRIX rotation = {0};
            for (i = 0; i < 9; ++i)
                rotation.m[i / 3][i % 3] = transform->rotation[i];
            SetRotMatrix(&rotation);
        }
        ApplyRotMatrixLV(&end, &start);
        x = start.vx >> 5;
        y = start.vy >> 5;
        z = start.vz >> 5;
        start.vx = x;
        start.vy = y;
        start.vz = z;
        x = end.vx + motion_x;
        y = end.vy + motion_y;
        z = end.vz + motion_z;
        point.vx = x;
        point.vy = y;
        point.vz = z;
        ApplyRotMatrixLV(&point, &end);
        PopMatrix();
        x = end.vx >> 5;
        y = end.vy >> 5;
        z = end.vz >> 5;
        end.vx = x;
        end.vy = y;
        end.vz = z;
        for (face_idx = 0; face_idx < face_count; ++face_idx)
        {
            face = face_base + face_idx;
            polygon = face;
            if (((uint16)polygon->n.depth_bias & 1) == 0)
                continue;
            vertex = vertex_base + polygon->vertex_idxs[3];
            vx = vertex->pos.vx;
            vy = vertex->pos.vy;
            vz = vertex->pos.vz;
            sx = start.vx;
            sy = start.vy;
            sz = start.vz;
            dx = sx - vx;
            dy = sy - vy;
            dz = sz - vz;
            nx = polygon->n.x;
            ny = polygon->n.y;
            nz = polygon->n.z;
            dot = (sint32)(dx * nx + dy * ny + dz * nz) >> 7;
            if ((sint32)(dot + margin) <= 0)
                continue;
            ex = end.vx;
            ey = end.vy;
            ez = end.vz;
            dx = ex - vx;
            dy = ey - vy;
            dz = ez - vz;
            nx = polygon->n.x;
            ny = polygon->n.y;
            nz = polygon->n.z;
            end_distance = ((sint32)(dx * nx + dy * ny + dz * nz) >> 7) - radius;
            start_distance = dot - radius;
            if ((sint32)end_distance >= 0)
                continue;
            if (start_distance > 0)
            {
                val = start_distance - end_distance;
                if (val == 0)
                    xport_mips_break(7);
                rcp = 0x40000000 / (sint32)val;
                fraction = ((sint64)start_distance * rcp) >> 18;
                fixed_math_result = fraction;
                dx = ex - sx;
                dy = ey - sy;
                dz = ez - sz;
                point.vx = dx;
                point.vy = dy;
                point.vz = dz;
                if ((sint32)*fraction_limit < (sint32)fraction)
                    continue;
                px = mul12(dx, (sint32)fraction);
                py = mul12(dy, (sint32)fraction);
                pz = mul12(dz, (sint32)fraction);
                point.vx = px;
                point.vy = py;
                point.vz = pz;
                px += sx;
                py += sy;
                pz += sz;
                point.vx = px;
                point.vy = py;
                point.vz = pz;
                edge_radius = radius_sq;
            }
            else
            {
                if ((sint32)end_distance >= start_distance)
                    continue;
                dot = start_distance + radius;
                if ((sint32)dot < 0)
                {
                    angle = AcosAngle((-dot), radius);
                    angle = SinQ14((1024 - (uint32)angle)) >> 2;
                    val = mul12((sint32)radius, angle);
                    edge_radius = mul12((sint32)val, (sint32)val);
                }
                else
                    edge_radius = radius_sq;
                px = start.vx;
                py = start.vy;
                pz = start.vz;
                point.vx = px;
                point.vy = py;
                point.vz = pz;
                fraction = 0;
            }
            for (edge = 0; edge < 4; ++edge)
            {
                x = vx;
                y = vy;
                z = vz;
                vertex = vertex_base + polygon->vertex_idxs[edge];
                vx = vertex->pos.vx;
                vy = vertex->pos.vy;
                vz = vertex->pos.vz;
                dx = x - vx;
                dy = y - vy;
                dz = z - vz;
                nx = polygon->n.x;
                ny = polygon->n.y;
                nz = polygon->n.z;
                cx = (sint32)(ny * dz - dy * nz) >> 7;
                cy = (sint32)(nz * dx - dz * nx) >> 7;
                cz = (sint32)(nx * dy - dx * ny) >> 7;
                dx = (point.vx - vx) << 5;
                dy = (point.vy - vy) << 5;
                dz = (point.vz - vz) << 5;
                dot = mul12(cx, dx);
                dot += (uint32)mul12(cy, dy);
                dot += (uint32)mul12(cz, dz);
                if ((sint32)dot >= 0)
                {
                    if ((sint32)dot > 0x200000)
                        break;
                    edge_sq = mul12((sint32)dot, (sint32)dot);
                    len_sq = mul12(cx, cx);
                    len_sq += (uint32)mul12(cy, cy);
                    len_sq += (uint32)mul12(cz, cz);
                    bound = mul12(edge_radius, (sint32)len_sq);
                    if (bound < edge_sq)
                        break;
                }
            }
            if (edge == 4)
            {
                *fraction_limit = fraction;
                sel_face = face;
                sel_sphere = sphere;
            }
        }
    }
    if (sel_face == 0)
        result = -1;
    else
    {
        val = sel_sphere - geometry + 1;
        x = sel_face - face_base;
        result = x | (val << 12);
    }
    return result;
}

uint32 AnimatedObject::SweepSphereAgainstObject(const CollisionSphere *bounds, const SceneNode *object, const Vec3 &delta, uint32 *fraction, const CollisionSphere *geometry)
{
    FUNCTION_MARKER(0x80038990, "SLUS_005.78");
    VECTOR start, end, relative;
    uint32 category;
    uint32 hx;
    uint32 hy;
    uint32 hz;
    uint32 radius;
    uint32 val;
    uint32 old;
    sint32 rcp;
    uint32 sx, sy, sz, ex, ey, ez, dx, dy, dz, lower_x, upper_x, lower_y, upper_y, lower_z, upper_z;
    uint32 enter_x;
    uint32 exit_x;
    uint32 enter_y;
    sint32 exit_y;
    uint32 enter_z;
    sint32 exit_z;
    uint32 time;
    sint32 leave;
    uint32 side;
    uint32 best;
    uint32 nx;
    uint32 ny;
    uint32 nz;
    uint32 dot;
    uint32 i;
    uint32 result = 0;
    const SceneTrigger *trigger = 0;
    const sint16 *n;
    category = object->kind;
    if (category == 2)
    {
        trigger = (const SceneTrigger *)object;
        hx = trigger->extent[0] << 5;
        hy = trigger->extent[1] << 5;
        hz = trigger->extent[2] << 5;
        relative.vx = (uint32)bounds->pos.x - trigger->transform.t[0];
        relative.vy = (uint32)bounds->pos.y - trigger->transform.t[1];
        relative.vz = (uint32)bounds->pos.z - trigger->transform.t[2];
        PushMatrix();
        {
            SetRotMatrix((MATRIX *)&trigger->transform);
        }
        ApplyRotMatrixLV(&relative, &start);
        relative.vx = relative.vx + (uint32)delta.x;
        relative.vy = relative.vy + (uint32)delta.y;
        relative.vz = relative.vz + (uint32)delta.z;
        ApplyRotMatrixLV(&relative, &end);
        PopMatrix();
    }
    else
    {
        if (category == 8)
        {
            const PositionedBillboardNode *billboard = (const PositionedBillboardNode *)object;
            sx = bounds->pos.x - (uint32)billboard->world_pos.x;
            start.vx = sx;
            sy = bounds->pos.y - (uint32)billboard->world_pos.y;
            start.vy = sy;
            sz = bounds->pos.z - (uint32)billboard->world_pos.z;
            start.vz = sz;
            hz = billboard->billboard.extent;
        }
        else if (category == 0)
        {
            sx = bounds->pos.x - (uint32)geometry->pos.x;
            start.vx = sx;
            sy = bounds->pos.y - (uint32)geometry->pos.y;
            start.vy = sy;
            sz = bounds->pos.z - (uint32)geometry->pos.z;
            start.vz = sz;
            hz = geometry->radius;
        }
        else
            goto done;
        hy = hz;
        hx = hz;
        end.vx = sx + delta.x;
        end.vy = sy + delta.y;
        end.vz = sz + delta.z;
    }
    radius = bounds->radius;
    sx = start.vx;
    ex = end.vx;
    val = (-hx) - (sx - radius);
    lower_x = val - (radius << 1);
    upper_x = val + (hx << 1);
    dx = ex - sx;
    if (!((sint32)lower_x > 0 ? (sint32)dx >= (sint32)lower_x : (sint32)upper_x >= 0))
    {
        if ((sint32)upper_x >= 0 || (sint32)upper_x < (sint32)dx)
            goto done;
    }
    sy = start.vy;
    ey = end.vy;
    val = (-hy) - (sy - radius);
    lower_y = val - (radius << 1);
    upper_y = val + (hy << 1);
    dy = ey - sy;
    if (!((sint32)lower_y > 0 ? (sint32)dy >= (sint32)lower_y : (sint32)upper_y >= 0))
    {
        if ((sint32)upper_y >= 0 || (sint32)upper_y < (sint32)dy)
            goto done;
    }
    sz = start.vz;
    ez = end.vz;
    val = (-hz) - (sz - radius);
    lower_z = val - (radius << 1);
    upper_z = val + (hz << 1);
    dz = ez - sz;
    if (!((sint32)lower_z > 0 ? (sint32)dz >= (sint32)lower_z : (sint32)upper_z >= 0))
    {
        if ((sint32)upper_z >= 0 || (sint32)upper_z < (sint32)dz)
            goto done;
    }
    if (dx + 256 < 0x201)
    {
        enter_x = 0;
        exit_x = 4096;
    }
    else
    {
        old = lower_x;
        if ((sint32)upper_x < 0)
        {
            lower_x = -upper_x;
            upper_x = -old;
        }
        else if ((sint32)lower_x <= 0)
        {
            if ((sint32)dx < 0)
                upper_x = -lower_x;
            lower_x = 0;
        }
        if ((sint32)dx < 0)
            dx = -dx;
        if ((sint32)dx < (sint32)upper_x)
            upper_x = dx;
        if (dx == 0)
            xport_mips_break(7);
        rcp = dx == 0 ? 0xFFFFFFFF : (uint32)(0x1000000 / (sint32)dx);
        enter_x = mul12((sint32)lower_x, rcp);
        exit_x = mul12((sint32)upper_x, rcp);
    }
    time = enter_x;
    leave = exit_x;
    if (dy + 256 < 0x201)
    {
        enter_y = 0;
    }
    else
    {
        old = lower_y;
        if ((sint32)upper_y < 0)
        {
            lower_y = -upper_y;
            upper_y = -old;
        }
        else if ((sint32)lower_y <= 0)
        {
            if ((sint32)dy < 0)
                upper_y = -lower_y;
            lower_y = 0;
        }
        if ((sint32)dy < 0)
            dy = -dy;
        if ((sint32)dy < (sint32)upper_y)
            upper_y = dy;
        if (dy == 0)
            xport_mips_break(7);
        rcp = dy == 0 ? 0xFFFFFFFF : (uint32)(0x1000000 / (sint32)dy);
        enter_y = mul12((sint32)lower_y, rcp);
        exit_y = mul12((sint32)upper_y, rcp);
        if ((sint32)time < (sint32)enter_y)
            time = enter_y;
        if (exit_y < leave)
            leave = exit_y;
    }
    if (dz + 256 < 0x201)
    {
        enter_z = 0;
    }
    else
    {
        old = lower_z;
        if ((sint32)upper_z < 0)
        {
            lower_z = -upper_z;
            upper_z = -old;
        }
        else if ((sint32)lower_z <= 0)
        {
            if ((sint32)dz < 0)
                upper_z = -lower_z;
            lower_z = 0;
        }
        if ((sint32)dz < 0)
            dz = -dz;
        if ((sint32)dz < (sint32)upper_z)
            upper_z = dz;
        if (dz == 0)
            xport_mips_break(7);
        rcp = dz == 0 ? 0xFFFFFFFF : (uint32)(0x1000000 / (sint32)dz);
        enter_z = mul12((sint32)lower_z, rcp);
        exit_z = mul12((sint32)upper_z, rcp);
        if ((sint32)time < (sint32)enter_z)
            time = enter_z;
        if (exit_z < leave)
            leave = exit_z;
    }
    if (leave < (sint32)time)
        goto done;
    result = 1;
    if (object->kind != 2)
        goto hit;
    val = (sint8)(object->state_flags >> 8);
    if (val != 9 && val != 6)
        goto hit;
    if (time != 0)
    {
        if (time == enter_y)
            side = (sint32)sy < 0 ? 2 : 3;
        else if (time == enter_z)
            side = (sint32)sz < 0 ? 4 : 5;
        else
            side = (sint32)sx >= 0;
        goto side_hit;
    }
    val = (sint32)sx < 0 ? sx + hx : sx - hx;
    end.vx = val;
    if ((sint32)val < 0)
        end.vx = -val;
    val = (sint32)sy < 0 ? sy + hy : sy - hy;
    end.vy = val;
    if ((sint32)val < 0)
        end.vy = -val;
    val = (sint32)sz < 0 ? sz + hz : sz - hz;
    end.vz = val;
    if ((sint32)val < 0)
        end.vz = -val;
    val = end.vy;
    if (end.vx >= (sint32)val && end.vz >= (sint32)val)
        side = (sint32)sy < 0 ? 2 : 3;
    else
    {
        val = end.vz;
        if (end.vx >= (sint32)val && end.vy >= (sint32)val)
            side = (sint32)sz < 0 ? 4 : 5;
        else
            side = (sint32)sx >= 0;
    }
    if (delta.x == 0 && delta.y == 0 && delta.z == 0)
        goto side_hit;
    best = enter_z;
    for (i = 0; i < 6; ++i)
    {
        n = trigger->transform.m[i >> 1];
        nx = n[0];
        ny = n[1];
        nz = n[2];
        if ((i & 1) == 0)
        {
            nx = -nx;
            ny = -ny;
            nz = -nz;
        }
        dot = mul12(delta.x, (sint32)nx);
        dot += (uint32)mul12(delta.y, (sint32)ny);
        dot += (uint32)mul12(delta.z, (sint32)nz);
        if ((sint32)dot < (sint32)time)
        {
            time = dot;
            best = i;
        }
    }
    if (best != side)
    {
        uint32 row = (sint32)best >> 1;
        n = row < 3 ? trigger->transform.m[row] : (const sint16 *)((const uint8 *)trigger + 16 + row * 6);

        ny = n[1];

        if ((best & 1) == 0)
        {
            ny = -ny;
        }
        if (ny + 128 >= 0x101)
        {
            n = trigger->transform.m[side >> 1];

            ny = n[1];

            if ((side & 1) == 0)
            {
                ny = -ny;
            }
            if (ny + 128 < 0x101)
                best = side;
        }
    }
    side = best;
    time = 0;
side_hit:
    result = side + 1;
hit:
    *fraction = time;
done:
    return result;
}

uint32 AnimatedObject::TestSweptObjectBounds(const sint32 *pos, const sint32 *velocity, uint32 radius) const
{
    FUNCTION_MARKER(0x80039404, "SLUS_005.78");
    sint32 combined;
    sint32 radius_sq;
    sint32 start_sq;
    uint32 velocity_sq;
    sint32 end_sq;
    uint32 x;
    uint32 y;
    uint32 z;
    sint32 vx;
    sint32 vy;
    sint32 vz;
    sint32 scaled;
    sint32 difference;
    sint32 left;
    uint32 right;
    combined = radius + bounds.radius;
    radius_sq = mul12(combined, combined);
    x = pos[0] - (uint32)world_pos.x;
    y = pos[1] - (uint32)world_pos.y;
    z = pos[2] - (uint32)world_pos.z;
    x = (sint32)x >> 6;
    y = (sint32)y >> 6;
    z = (sint32)z >> 6;
    start_sq = x * x + y * y + z * z;
    if (radius_sq >= start_sq)
        return 1;
    vx = velocity[0];
    x = vx >> 6;
    vy = velocity[1];
    y = vy >> 6;
    vz = velocity[2];
    z = vz >> 6;
    velocity_sq = x * x + y * y + z * z;
    if (velocity_sq == 0)
        return 0;
    x = (uint32)world_pos.x + vx;
    y = (uint32)world_pos.y + velocity[1];
    z = (uint32)world_pos.z + velocity[2];
    x = pos[0] - x;
    y = pos[1] - y;
    z = pos[2] - z;
    x = (sint32)x >> 6;
    y = (sint32)y >> 6;
    z = (sint32)z >> 6;
    end_sq = x * x + y * y + z * z;
    if (radius_sq >= end_sq)
        return 1;
    difference = end_sq - velocity_sq - start_sq;
    scaled = velocity_sq << 2;
    right = mul12(start_sq, scaled);
    right -= (uint32)mul12(difference, difference);
    left = mul12(radius_sq, scaled);
    return left >= (sint32)right;
}

void AnimatedObject::CollideObjectSpheres(AnimatedObject *other_object)
{
    const PathTransform *path_transform;
    FUNCTION_MARKER(0x80039614, "SLUS_005.78");
    AnimatedObject *other = other_object;
    CollisionHandler col_callback = 0;
    Vec3 difference;
    uint32 extent;
    const PathDefinition *definition;
    uint32 x, y, val, cur_flags, best, result;
    const CollisionSphere *own_shape, *sel_own = 0;
    const CollisionSphere *other_shape, *sel_other = 0;
    uint32 own_idx, other_idx, sel = 0;
    sint32 kind, other_kind;

    difference = other->velocity;
    definition = other->path_definition;
    if (definition != 0)
    {
        const PathTransform *captured_path_transform = definition->transform;
        if (captured_path_transform != 0 && (other->flags & 0x2000) == 0)
        {
            path_transform = captured_path_transform;
            difference.x = difference.x + ((uint32)path_transform->displacement.x);
            difference.y = difference.y + ((uint32)path_transform->displacement.y);
            difference.z = difference.z + ((uint32)path_transform->displacement.z);
        }
    }
    x = velocity.x - (uint32)difference.x;
    difference.x = x;
    y = velocity.y - (uint32)difference.y;
    difference.y = y;
    difference.z = velocity.z - (uint32)difference.z;
    result = TestSweptObjectBounds(other->world_pos.comp, difference.comp, other->bounds.radius) << 24;
    if (result == 0)
        goto finish;
    best = path_motion_scale;
    own_shape = spheres;
    for (own_idx = 0; own_idx < 4; ++own_idx, ++own_shape)
    {
        if (own_shape->radius == 0xFFFFFFFF)
            break;
        other_shape = other->spheres;
        for (other_idx = 0; other_idx < 4; ++other_idx, ++other_shape)
        {
            if (other_shape->radius == 0xFFFFFFFF)
                break;
            if (AnimatedObject::SweepSphereAgainstObject(own_shape, &other->node, difference, &extent, other_shape) != 0)
            {
                val = extent;
                if ((sint32)val <= (sint32)best && (sel == 0 || (sint32)val < (sint32)best))
                {
                    best = val;
                    sel_other = other_shape;
                    sel_own = own_shape;
                    sel = 1;
                }
            }
        }
    }

    if (sel == 0 || (sint32)path_motion_scale < (sint32)best)
        goto finish;
    kind = contact_kind;
    if (kind == 1)
    {
        other_kind = other->contact_kind;
        if (other_kind == 2)
        {
            col_contact.other_selection = 0;
            if (other->ReadObjectImpulseScale() != 0)
                best = 4096;
            if ((flags & 0x2400) != 0 || (path_flags & 2) != 0)
            {
                x = world_pos.y;
                y = bounds.height;
                val = velocity.y;
                x -= y;
                val = mul12((sint32)val, (sint32)best);
                y = other->velocity.y;
                x += val + 6144;
                y = mul12((sint32)y, (sint32)best);
                y += (uint32)other->world_pos.y;
                if ((sint32)x >= (sint32)y)
                    col_contact.other_selection = 4096;
            }
            goto prepare_callback;
        }
    }
    else
        other_kind = other->contact_kind;
    if (other_kind == 1 && contact_kind == 2)
    {
        col_contact.selection = 0;
        if ((other->flags & 0x2400) != 0 || (other->path_flags & 2) != 0)
        {
            x = other->world_pos.y;
            y = other->bounds.height;
            val = other->velocity.y;
            x -= y;
            val = mul12((sint32)val, (sint32)best);
            y = velocity.y;
            x += val + 6144;
            y = mul12((sint32)y, (sint32)best);
            y += (uint32)world_pos.y;
            if ((sint32)x >= (sint32)y)
                col_contact.selection = 4096;
        }
    }
prepare_callback:
    col_contact.fraction = best;
    kind = contact_kind;
    if (kind == 1)
        val = sel_other->radius & 7;
    else if (kind == 2)
        val = sel_own->radius & 7;
    else
        val = 0;
    col_contact.flags = val;
    kind = contact_kind;
    col_callback = object_operations.collisions[kind];
    result = (this->*col_callback)((SceneNode *)other_object, &col_contact) << 24;
    if (result != 0)
    {
        cur_flags = flags;
        path_motion_scale = best;
        result = cur_flags & 0xFFFFBFFF;
        flags = result;
    }
finish:
    return;
}

uint32 AnimatedObject::CollideObjectWithTrigger(const SceneTrigger *trigger)
{
    FUNCTION_MARKER(0x80039B20, "SLUS_005.78");
    CollisionHandler col_callback = 0;
    Vec3 delta;
    uint32 fraction;
    uint32 sel_idx;
    uint8 shape_flags[2];
    uint32 val, cur_flags, radius, first_filter, last_filter, best, side, idx, result;
    sint32 kind;
    const SceneTrigger *other = trigger;
    val = velocity.x;
    delta.x = val - other->delta.x;
    val = velocity.y;
    delta.y = val - other->delta.y;
    val = velocity.z;
    delta.z = val - other->delta.z;
    radius = other->radius << 5;
    result = 1;
    if ((TestSweptObjectBounds(other->transform.t, delta.comp, radius) & 0xFF) == 0)
        goto finish_contact;
    kind = (sint8)(other->node.state_flags >> 8);
    first_filter = 0;
    if (kind != 1)
    {
        cur_flags = path_flags;
        if ((cur_flags & 2) != 0 || (cur_flags & 0x180) == 0x80)
        {
            last_filter = 1;
            goto select_contact;
        }
        kind = (sint8)(other->node.state_flags >> 8);
    }
    if (kind == 1 || kind == 24 || kind == 6 || ((uint8)(kind - 40) < 16 && ((other->parameters.event.schedule >> 12) & 31) == 15))
        first_filter = 1;
    last_filter = 0;
    if (first_filter == 0 && (uint32)(other->node.state_flags >> 8) - 40 < 16)
        last_filter = ((other->parameters.event.schedule >> 12) & 31) == 16;
select_contact:
    side = 0xFFFFFFFF;
    best = path_motion_scale + 1;
    for (idx = 0; idx < 4; ++idx)
    {
        if (spheres[idx].radius == 0xFFFFFFFF)
            break;
        GetCharacterSphereEnds(idx + 1, shape_flags, shape_flags + 1);
        if (first_filter != 0)
        {
            if (shape_flags[0] != 0)
                goto test_contact;
        }
        else if (last_filter == 0)
            goto test_contact;
        if (last_filter == 0 || shape_flags[1] == 0)
            continue;
    test_contact:
        val = AnimatedObject::SweepSphereAgainstObject(spheres + idx, &other->node, delta, &fraction, 0);
        if (val != 0)
        {
            cur_flags = fraction;
            if ((sint32)cur_flags < (sint32)best)
            {
                best = cur_flags;
                side = val;
                sel_idx = idx;
            }
        }
    }
    if ((sint32)path_motion_scale >= (sint32)best)
    {
        col_contact.selection = side;
        if (side != 0xFFFFFFFF)
        {
            val = sel_idx;
            col_contact.selection |= (val + 1) << 12;
        }
        col_contact.fraction = best;
        col_contact.flags = 0;
        kind = contact_kind;
        col_callback = object_operations.collisions[kind];
        if (((this->*col_callback)(trigger != 0 ? (SceneNode *)&trigger->node : 0, &col_contact) & 0xFF) != 0)
        {
            cur_flags = flags;
            path_motion_scale = best;
            flags = cur_flags & 0xFFFFBFFF;
        }
    }
    cur_flags = path_flags;
    result = cur_flags & 0x80;
    if ((cur_flags & 2) != 0)
    {
        val = gameplay_input.buttons;
        trigger_contact_motion.dir = 0;
        if ((val & 0x1000) != 0)
        {
            result = 1;
            if ((sint32)path_motion_scale < (sint32)best)
                trigger_contact_motion.dir = 1;
        }
        else
        {
            result = val & 0x4000;
            if (result != 0)
            {
                result = 0xFFFFFFFF;
                if ((sint32)path_motion_scale < (sint32)best)
                    trigger_contact_motion.dir = 0xFFFFFFFF;
            }
        }
    }
    else if (result != 0)
    {
        val = path_motion_scale;
        trigger_contact_motion.blocked = 0;
        result = 1;
        if ((sint32)val < (sint32)best)
            trigger_contact_motion.blocked = 1;
    }
finish_contact:
    return result;
}

void AnimatedObject::FindObjectGeometryCol()
{
    SceneNode *scene_node, *sel = 0;

    const SceneSection *section;
    FUNCTION_MARKER(0x80039EA4, "SLUS_005.78");
    CollisionHandler col_callback = 0;
    const CollisionSphere *geometry;
    const MeshGeometryNode *mesh, *pos_src;
    const AnimatedObject *geometry_object;
    const CollisionTransform *cached_transform;
    uint32 fraction_limit;
    uint32 mask, x, y, z, radius, limit, val, result;
    sint32 idx;
    sint32 kind, hit, selection;
    kind = contact_kind;
    result = 4;
    if (kind == 1)
        goto search;
    if (kind == 3)
    {
        if (animation_binding.tracks[0] != -2)
            goto search;
        val = state;
        if (val == 23 || val == 42)
            goto search;
    }
    if (kind != 4)
        return;
    col_callback = object_operations.collisions[4];
    result = (this->*col_callback)(0, 0) << 24;
    if (result == 0)
        return;
search:
    geometry = spheres;
    if (contact_kind == 1)
    {
        geometry = BuildCharacterGeometry();
        CharacterController::DisableCharacterSupportFace();
    }
    mask = (flags & 0x200) != 0 ? 0x100000 : 0x100200;
    section = scene_stream.active_section;

    col_transform.rotation[0] = psx_identity_matrix.m[0][0];
    col_transform.rotation[1] = psx_identity_matrix.m[0][1];
    col_transform.rotation[2] = psx_identity_matrix.m[0][2];
    col_transform.rotation[3] = psx_identity_matrix.m[1][0];
    col_transform.rotation[4] = psx_identity_matrix.m[1][1];
    col_transform.rotation[5] = psx_identity_matrix.m[1][2];
    col_transform.rotation[6] = psx_identity_matrix.m[2][0];
    col_transform.rotation[7] = psx_identity_matrix.m[2][1];
    col_transform.rotation[8] = psx_identity_matrix.m[2][2];
    col_transform.motion.z = 0;
    col_transform.motion.y = 0;
    col_transform.motion.x = 0;
    col_transform.first_face = 0;
    scene_node = section->object_heads[2];
    val = path_motion_scale;
    selection = -1;
    mask |= 0x2000;
    fraction_limit = val;
    for (; scene_node != 0; scene_node = mesh->node.links.registry_next)
    {
        mesh = (const MeshGeometryNode *)scene_node;
        if ((mesh->flags & mask) != 0x100000 || (mesh->node.flags & 1) != 0)
            continue;
        if (mesh->vertex_count > 0)
        {
            x = mesh->pos.x;
            y = mesh->pos.y;
            z = mesh->pos.z;
        }
        else
        {
            pos_src = mesh->src_mesh;
            x = pos_src->pos.x;
            y = pos_src->pos.y;
            z = pos_src->pos.z;
        }
        col_transform.pos.x = x;
        col_transform.pos.y = y;
        col_transform.pos.z = z;
        radius = mesh->node.state_flags << 5;
        limit = (radius + 61440) << 1;
        val = world_pos.x;
        val = val - mesh->pos.x + radius + 61440;
        if (val > limit)
            continue;
        val = world_pos.y;
        val = val - mesh->pos.y + radius + 61440;
        if (val > limit)
            continue;
        val = world_pos.z;
        val = val - mesh->pos.z + radius + 61440;
        if (val > limit)
            continue;
        if ((TestSweptObjectBounds(mesh->pos.comp, velocity.comp, radius) & 0xFF) == 0)
            continue;
        col_transform.face_count = mesh->polygon_count;
        hit = FindSphereFaceCol(geometry, scene_node, &col_transform, &fraction_limit);
        if (hit >= 0)
        {
            selection = (uint32)hit & 0xFFFF;
            sel = scene_node;
        }
    }
    section = scene_stream.active_section;
    scene_node = section->object_heads[0];
    mask &= 0xFFFFDFFF;
    for (; scene_node != 0; scene_node = geometry_object->node.links.registry_next)
    {
        geometry_object = (const AnimatedObject *)scene_node;
        if ((geometry_object->node.flags & 0x81) != 0 || (geometry_object->flags & mask) != 0x100000)
            continue;
        if (contact_kind == 4 && scene_node == node.links.first_child)
            continue;
        radius = geometry_object->bounds.radius + 61440;
        limit = radius << 1;
        val = world_pos.x;
        val = val - geometry_object->world_pos.x + radius;
        if (val > limit)
            continue;
        val = world_pos.y;
        val = val - geometry_object->world_pos.y + radius;
        if (val > limit)
            continue;
        val = world_pos.z;
        val = val - geometry_object->world_pos.z + radius;
        if (val > limit)
            continue;
        for (idx = 0; idx < geometry_object->col_transform_count; ++idx)
        {
            cached_transform = geometry_object->col_transforms;
            cached_transform = cached_transform + idx;
            col_transform = *cached_transform;

            hit = FindSphereFaceCol(geometry, scene_node, &col_transform, &fraction_limit);
            if (hit >= 0)
            {
                sel = scene_node;
                val = (uint32)hit + col_transform.first_face;
                selection = (idx << 16) | (val & 0xFFFF);
            }
        }
    }
    if (contact_kind == 1)
        CharacterController::RestoreCharacterSupportFace();
    result = 0x7FFF0000;
    if (selection >= 0)
    {
        val = fraction_limit;
        col_contact.selection = selection;
        col_contact.flags = 0x7FFFF000;
        col_contact.fraction = val;
        col_callback = object_operations.collisions[contact_kind];
        result = (this->*col_callback)(sel, &col_contact) << 24;
        if (result != 0)
        {
            val = fraction_limit;
            mask = flags;
            path_motion_scale = val;
            result = 0xFFFFBFFF;
            flags = mask & result;
        }
    }
}

void AnimatedObject::ProcessObjectTriggerContacts()
{
    SceneNode *node;

    FUNCTION_MARKER(0x8003A384, "SLUS_005.78");
    CollisionHandler col_callback = 0;
    const SceneTrigger *trigger;
    PositionedBillboardNode *billboard;
    const ScenePath *context;
    Vec3 motion;
    uint32 extent;
    uint32 kind;
    uint32 radius;
    uint32 limit;
    uint32 val;

    uint32 shape;
    uint32 nearest;
    uint32 result;
    const SceneSection *section;
    kind = contact_kind;
    result = 4;
    if (kind == 2)
    {
        section = scene_stream.active_section;
        for (node = section->object_heads[5]; node != 0; node = trigger->node.links.registry_next)
        {
            trigger = (const SceneTrigger *)node;
            result = 61440;
            if ((trigger->node.flags & 0x81) != 0)
                continue;
            radius = (trigger->radius << 5) + 61440;
            limit = radius << 1;
            val = world_pos.x;
            val = val - trigger->transform.t[0] + radius;
            if (val > limit)
                continue;
            val = world_pos.y;
            val = val - trigger->transform.t[1] + radius;
            result = val > limit;
            if (result != 0)
                continue;
            val = world_pos.z;
            val = val - trigger->transform.t[2] + radius;
            result = val > limit;
            if (result != 0)
                continue;
            col_callback = object_operations.collisions[contact_kind];
            result = (this->*col_callback)((SceneNode *)&trigger->node, 0) << 24;
            if (result != 0)
                result = CollideObjectWithTrigger(trigger);
        }
        goto done;
    }
    if (kind == 4)
        goto done;
    section = scene_stream.active_section;
    for (node = section->object_heads[4]; node != 0; node = trigger->node.links.registry_next)
    {
        trigger = (const SceneTrigger *)node;
        if ((trigger->node.flags & 0x85) != 0)
            continue;
        radius = (trigger->radius << 5) + 61440;
        limit = radius << 1;
        val = world_pos.x;
        val = val - trigger->transform.t[0] + radius;
        if (val > limit)
            continue;
        val = world_pos.y;
        val = val - trigger->transform.t[1] + radius;
        if (val > limit)
            continue;
        val = world_pos.z;
        val = val - trigger->transform.t[2] + radius;
        if (val > limit)
            continue;
        col_callback = object_operations.collisions[contact_kind];
        if (((this->*col_callback)((SceneNode *)&trigger->node, 0) & 0xFF) != 0)
            CollideObjectWithTrigger(trigger);
    }
    kind = contact_kind;
    context = 0;
    if (kind == 1)
    {
        if (behavior == 33)
            context = scene_path;
        else
            context = camera_motion.path;
    }
    else if (kind == 3 && animation_binding.tracks[0] == -1)
        context = scene_path;
    if ((CanObjectChangePath(0, context) & 0xFF) != 0)
    {
        section = scene_stream.active_section;
        for (node = section->object_heads[6]; node != 0; node = trigger->node.links.registry_next)
        {
            trigger = (const SceneTrigger *)node;
            if ((trigger->node.flags & 0x85) != 0)
                continue;
            radius = (trigger->radius << 5) + 61440;
            limit = radius << 1;
            val = spheres[0].pos.x;
            val = val - trigger->transform.t[0] + radius;
            if (val > limit)
                continue;
            val = spheres[0].pos.y;
            val = val - trigger->transform.t[1] + radius;
            if (val > limit)
                continue;
            val = spheres[0].pos.z;
            val = val - trigger->transform.t[2] + radius;
            if (val > limit)
                continue;
            if ((CanObjectChangePath(trigger, context) & 0xFF) != 0)
                CollideObjectWithTrigger(trigger);
        }
    }
    kind = contact_kind;
    result = 19;
    if (kind != 1 && (kind != 3 || state != 19))
        goto done;
    section = scene_stream.active_section;
    for (node = section->object_heads[8]; node != 0; node = billboard->billboard.node.links.registry_next)
    {
        billboard = (PositionedBillboardNode *)node;
        if ((billboard->billboard.playback_flags & 0x8000) != 0)
        {
            SceneEvents::QueueSceneEvent(billboard->billboard.event_id | 0x4000, 4);
            billboard->billboard.playback_flags = billboard->billboard.playback_flags & 0x7FFF;
        }
        result = billboard->billboard.node.flags & 0x85;
        if (result != 0)
            continue;
        val = world_pos.x;
        val = val - billboard->world_pos.x + 61440;
        result = val > 0x1E000;
        if (result != 0)
            continue;
        val = world_pos.y;
        val = val - billboard->world_pos.y + 61440;
        result = val > 0x1E000;
        if (result != 0)
            continue;
        val = world_pos.z;
        val = val - billboard->world_pos.z + 61440;
        result = val > 0x1E000;
        if (result != 0)
            continue;
        col_callback = object_operations.collisions[contact_kind];
        result = (this->*col_callback)(&billboard->billboard.node, 0) << 24;
        if (result == 0)
            continue;

        motion = velocity;
        nearest = path_motion_scale + 1;
        for (shape = 0; shape < 4; ++shape)
        {
            if (spheres[shape].radius == 0xFFFFFFFF)
                break;
            if (AnimatedObject::SweepSphereAgainstObject(spheres + shape, &billboard->billboard.node, motion, &extent, 0) != 0)
            {
                val = extent;
                if ((sint32)val < (sint32)nearest)
                    nearest = val;
            }
        }
        result = (sint32)path_motion_scale < (sint32)nearest;
        if (result != 0)
            continue;
        col_contact.fraction = nearest;
        col_contact.flags = 0;
        col_callback = object_operations.collisions[contact_kind];
        val = (this->*col_callback)(&billboard->billboard.node, &col_contact) << 24;
        result = 0xFFFFBFFF;
        if (val != 0)
        {
            val = flags;
            path_motion_scale = nearest;
            flags = val & result;
        }
    }
done:
    return;
}

void AnimatedObject::UpdateObjectCollisions()
{
    const SceneSection *section;
    const PathTransform *path_transform;
    FUNCTION_MARKER(0x8003A930, "SLUS_005.78");
    SceneNode *other_node;
    AnimatedObject *other;
    CollisionHandler col_callback = 0;
    const PathDefinition *parent, *restore_definition;
    uint32 val, result, radius, limit;
    const PathTransform *captured_path_transform;
    FindObjectGeometryCol();
    parent = path_definition;
    if (parent != 0)
    {
        captured_path_transform = parent->transform;
        if (captured_path_transform != 0 && (flags & 0x2000) == 0)
        {
            path_transform = captured_path_transform;
            val = velocity.x + (uint32)path_transform->displacement.x;
            parent = path_definition;
            velocity.x = val;
            path_transform = parent->transform;
            val = velocity.y + (uint32)path_transform->displacement.y;
            parent = path_definition;
            velocity.y = val;
            path_transform = parent->transform;
            val = velocity.z + (uint32)path_transform->displacement.z;
            velocity.z = val;
        }
    }
    ProcessObjectTriggerContacts();
    if ((node.flags & 1) != 0)
        goto restore;
    for (other_node = registered_object_head; other_node != 0; other_node = other_node->links.registry_next)
    {
        other = (AnimatedObject *)other_node;
        if (other_node == &node || (other_node->flags & 0x81) != 0 || other->contact_kind == 0)
            continue;
        val = behavior;
        radius = other->bounds.radius + (val == 30 ? 184320 : 61440);
        limit = radius << 1;
        val = world_pos.x;
        val = val - other->world_pos.x + radius;
        if (val > limit)
            continue;
        val = world_pos.y;
        val = val - other->world_pos.y + radius;
        if (val > limit)
            continue;
        val = world_pos.z;
        val = val - other->world_pos.z + radius;
        if (val > limit)
            continue;
        col_callback = object_operations.collisions[contact_kind];
        if (((this->*col_callback)(other_node, 0) & 0xFF) != 0)
            CollideObjectSpheres(other);
    }
    if (contact_kind == 4)
    {
        section = scene_stream.active_section;
        for (other_node = section->object_heads[1]; other_node != 0; other_node = other_node->links.registry_next)
        {
            other = (AnimatedObject *)other_node;
            if (other_node == &node || (other_node->flags & 0x81) != 0 || other->contact_kind == 0)
                continue;
            val = world_pos.x;
            val = val - other->world_pos.x + 81920;
            if (val > 163840)
                continue;
            val = world_pos.y;
            val = val - other->world_pos.y + 81920;
            if (val > 163840)
                continue;
            val = world_pos.z;
            val = val - other->world_pos.z + 81920;
            if (val > 163840)
                continue;
            col_callback = object_operations.collisions[contact_kind];
            if (((this->*col_callback)(other_node, 0) & 0xFF) != 0)
                CollideObjectSpheres(other);
        }
    }
restore:
    restore_definition = path_definition;
    if (restore_definition != 0)
    {
        captured_path_transform = restore_definition->transform;
        if (captured_path_transform != 0)
        {
            result = flags & 0x2000;
            if (result == 0)
            {
                path_transform = captured_path_transform;
                result = velocity.x - (uint32)path_transform->displacement.x;
                parent = path_definition;
                velocity.x = result;
                path_transform = parent->transform;
                result = velocity.y - (uint32)path_transform->displacement.y;
                parent = path_definition;
                velocity.y = result;
                path_transform = parent->transform;
                result = velocity.z - (uint32)path_transform->displacement.z;
                velocity.z = result;
            }
        }
    }
}

uint32 AnimatedObject::ChangeActorPath(const ScenePath *path, const Vec3 &sample_pos, Vec3 &floor)
{
    const ScenePath *adjacent_path;
    FUNCTION_MARKER(0x8003AF18, "SLUS_005.78");
    MATRIX *matrix;
    const PathDefinition *parent;
    const ScenePath *path_view;
    const AnimatedObject *player;
    uint32 frame_word;
    Vec3 query_floor, input, inverse;
    MATRIX transpose = {0};
    const ScenePath *visited[6];
    /* No candidate exists until this function selects a path */
    const ScenePath *candidate = 0, *visited_path;
    const ScenePath *initial_path = path;
    uint32 dir, cur_flags;
    const PathDefinition *definition;
    PathTransform *matrix_transform;
    uint32 kind_one_probe = 0, restrict_paths = 0, allow_special = 0, allow_link = 0;
    uint32 fallback_used = 0, result = 0, x, y, z, val, idx, repeated;
    sint32 kind, frame, probe_frame, retries = 2, prev_retries, gap;
    sint32 probe;
    memset((uint8 *)&transpose + sizeof(transpose.m), 0, offsetof(MATRIX, t) - sizeof(transpose.m));
    kind = contact_kind;
    if (kind == 1)
        kind_one_probe = (character_pad.flags & 0x60000) != 0;
    else if (movement_mode == 2 && (state == 19 || path_routing.flags != 0))
    {
        player = CharacterController::FindPlayerObject(0);
        parent = player->path_definition;
        val = parent->flags & 0x80000;
        restrict_paths = 1;
        if (val != 0)
            allow_special = 1;
        if ((path_definition->flags & 0x10000) != 0)
        {
            allow_link = 1;
        }
    }
    dir = flags & 0x10;
    if ((path_flags & 0x26) != 0)
    {
        probe = CanChangePath(sample_pos, dir);
        if (((uint32)probe & 0xFF) == 0)
            goto done;
    }
    for (idx = 0; idx < 6; ++idx)
        visited[idx] = 0;
select_path:
    path_view = path;
    frame = -1;
    if ((flags & 0x40) != 0)
        frame = path_frame;
    frame_word = frame;
    kind = contact_kind;
    if (kind == 1 || (kind == 3 && animation_binding.tracks[0] == -1))
    {
        if ((path_view->definition->flags & 0x50) >= 0x41 && ((flags & 0x4800) == 0 || retries < 2))
        {
            candidate = path_view->fallback;
            if (candidate != 0)
            {
                if ((path_view->node.state_flags & (dir | 8)) == 0 && (candidate->node.state_flags & 8) != 0)
                {
                    adjacent_path = candidate->adjacent[0];
                    candidate = adjacent_path;
                }
                if (candidate != 0)
                    goto check_candidate;
            }
        }
        candidate = path_view->SelectAdjacent(&frame_word, dir, path_flags);
        if (candidate != 0)
            goto check_candidate;
    }
    if (candidate == 0)
        goto fallback;
    if (behavior != 6 || state != 0 || path_routing.flags == 0 || fallback_used != 0)
        goto done;
    if ((path_definition->flags & 0x10000) == 0)
        goto done;
    adjacent_path = camera_motion.path->adjacent[1];
    fallback_used = 1;
    if (adjacent_path == 0)
        goto done;
    candidate = adjacent_path;
    goto check_candidate;
fallback:
    if (fallback_used == 1)
        goto done;
    fallback_used = 1;
    candidate = FindFallbackPath(sample_pos, dir);
    if (candidate == 0)
        goto done;
check_candidate:
    cur_flags = path_flags;
    path_flags = cur_flags & 0xFDFFFFFF;
    if ((cur_flags & 0x20) != 0 && camera_motion.path == candidate)
        goto fallback;
    if (candidate != initial_path && candidate != 0)
    {
        repeated = 0;
        for (idx = 0; idx < 6; ++idx)
        {
            visited_path = visited[idx];
            if (candidate == visited_path)
            {
                repeated = 1;
                break;
            }
        }
        if (repeated != 0 && path_routing.reject_repeated != 0)
            goto fallback;
        for (idx = 0; idx < 6; ++idx)
        {
            visited_path = visited[idx];
            if (visited_path == 0)
            {
                visited[idx] = candidate;
                break;
            }
        }
    }
    cur_flags = path_view->definition->flags;
    if ((cur_flags & 0x50) == 0 && retries >= 2)
        goto accept_path;
    if ((flags & 0x6000) == 0x4000)
    {
        probe = candidate->definition->ProjectFrame(frame_word, sample_pos, query_floor, dir, 4096);
        frame_word = probe;
        if (probe < 0)
            goto done;
        goto check_floor;
    }
    if (((cur_flags & 0x40) == 0 || candidate == path) && retries == 2)
    {
        candidate->SamplePos(frame_word, query_floor, dir);
        goto floor_ready;
    }
    if ((flags & 0x2000) == 0 || (path_flags & 0x40000) != 0)
    {
        probe = candidate->definition->FindHeightFrame(sample_pos, dir, -40960);
        frame_word = probe;
    }
    if ((path_flags & 0x24) != 0 && (sint32)frame_word == -1)
    {
        probe = candidate->definition->FindHeightFrame(sample_pos, dir, -40960);
        if (probe != -1)
        {
            probe = candidate->definition->ProjectFrame(probe, sample_pos, query_floor, dir, 8192);
            frame_word = probe;
        }
    }
    if (kind_one_probe != 0 && (sint32)frame_word == -1)
    {
        probe = candidate->definition->FindHeightFrame(sample_pos, dir, -40960);
        if (probe != -1)
        {
            probe = candidate->definition->ProjectFrame(probe, sample_pos, query_floor, dir, 20480);
            frame_word = probe;
        }
    }
    frame = frame_word;
    if (frame < 0)
        probe_frame = -1;
    else
    {
        probe = candidate->definition->ProjectFrame(frame, sample_pos, query_floor, dir, 4096);
        probe_frame = probe;
        if ((path_flags & 0x24) != 0 && probe_frame == -1)
        {
            probe = candidate->definition->FindHeightFrame(sample_pos, dir, -40960);
            if (probe != -1)
            {
                probe = candidate->definition->ProjectFrame(probe, sample_pos, query_floor, dir, 8192);
                probe_frame = probe;
            }
        }
        if (probe_frame >= 0 && (query_floor.x - (uint32)sample_pos.x + 24576 > 0xC000 || query_floor.z - (uint32)sample_pos.z + 24576 > 0xC000))
            probe_frame = -1;
    }
    frame_word = probe_frame;
floor_ready:
    if ((sint32)frame_word < 0)
        goto retry_path;
check_floor:
    if ((candidate->definition->flags & 0x10000) != 0)
        goto retry_path;
    gap = sample_pos.y - (uint32)query_floor.y;
    if (gap < -1365 && ((path_view->definition->flags & 0x40) == 0 || gap < -4096))
        goto retry_path;

    floor = query_floor;
    goto accept_path;
retry_path:
    if (retries != 0)
    {
        cur_flags = flags;
        if ((cur_flags & 0x4000) == 0)
        {
            prev_retries = retries--;
            if (prev_retries != 2 || (path_view->definition->flags & 0x10) == 0 || (cur_flags & 0x800) == 0 || path_view->fallback == 0)
                path = candidate;
            goto select_path;
        }
    }
    if (fallback_used == 0)
        goto fallback;
    goto done;
accept_path:
    if (candidate == initial_path && ((flags & 0x800) == 0 || retries != 2))
        goto done;
    cur_flags = candidate->definition->flags;
    if ((cur_flags & 0x10000) != 0)
        goto done;
    if (restrict_paths != 0)
    {
        if ((cur_flags & 0x80000) != 0 && allow_special == 0)
            goto done;
        if (allow_link == 0 && (candidate->definition->flags & 0x10) != 0)
            goto done;
    }
    cur_flags = flags;
    kind = contact_kind;
    flags = cur_flags & 0xFFFF9FFF;
    if (kind == 1 && behavior != 33)
    {
        camera_motion.focus_pos = sample_pos;
        UpdateObjectScenePath(candidate, frame_word);
    }
    else
    {
        if (kind != 1)
            ApplyPathVelocityChange(candidate->definition);
        scene_path = candidate;
    }
    path_frame = frame_word;
    parent = path_definition;
    if (parent != 0 && parent->transform != 0)
    {
        input = pos;
        PushMatrix();
        matrix = path_definition->transform != 0 ? &path_definition->transform->matrix : NULL;
        SetRotMatrix(matrix);
        {
            VECTOR rotation_output;
            {
                VECTOR sdk_input = {input.x, input.y, input.z, 0};
                ApplyRotMatrixLV(&sdk_input, &rotation_output);
            }
            pos.x = rotation_output.vx;
            pos.y = rotation_output.vy;
            pos.z = rotation_output.vz;
        }
        PopMatrix();
        parent = path_definition;
        matrix = parent->transform != 0 ? &parent->transform->matrix : NULL;
        x = pos.x;
        val = matrix->t[0];
        parent = path_definition;
        pos.x = x + val;
        matrix = parent->transform != 0 ? &parent->transform->matrix : NULL;
        y = pos.y;
        val = matrix->t[1];
        parent = path_definition;
        pos.y = y + val;
        matrix = parent->transform != 0 ? &parent->transform->matrix : NULL;
        z = pos.z;
        val = matrix->t[2];
        pos.z = z + val;
    }
    definition = candidate->definition;
    path_definition = definition;
    if (definition != 0 && (matrix_transform = definition->transform) != 0)
    {
        matrix = matrix_transform != 0 ? &matrix_transform->matrix : NULL;
        x = pos.x;
        inverse.x = x - matrix->t[0];
        matrix = path_definition->transform != 0 ? &path_definition->transform->matrix : NULL;
        y = pos.y;
        inverse.y = y - matrix->t[1];
        matrix = path_definition->transform != 0 ? &path_definition->transform->matrix : NULL;
        z = pos.z;
        inverse.z = z - matrix->t[2];
        for (idx = 0; idx < 9; ++idx)
        {
            matrix = path_definition->transform != 0 ? &path_definition->transform->matrix : NULL;
            val = (uint16)matrix->m[idx % 3][idx / 3];
            transpose.m[idx / 3][idx % 3] = val;
        }
        PushMatrix();
        SetRotMatrix(&transpose);
        {
            VECTOR rotation_output;
            {
                VECTOR sdk_input = {inverse.x, inverse.y, inverse.z, 0};
                ApplyRotMatrixLV(&sdk_input, &rotation_output);
            }
            pos.x = rotation_output.vx;
            pos.y = rotation_output.vy;
            pos.z = rotation_output.vz;
        }
        PopMatrix();
    }
    result = 1;
done:
    return result;
}

const ScenePath *AnimatedObject::FindFallbackPath(const Vec3 &pos, uint32 dir)
{
    const ScenePath *adjacent_path;
    FUNCTION_MARKER(0x8003BC08, "SLUS_005.78");
    const ScenePath *anchor_path;
    FallbackWorkspace work;
    const ScenePath *cur, *node, *root, *candidate, *sel = 0;
    uint32 flags;
    const PathDefinition *parent;
    uint32 same_definition;
    const ScenePath **chain0, **chain1, **chain2;
    uint32 offset, scan, slot, phase, end, val, x, y, z;
    /* A skipped same-path probe has no sampled frame or output vector */
    uint32 sel_frame = 0xFFFFFFFF, prev_frame = 0xFFFFFFFF, next_frame = 0xFFFFFFFF;
    uint32 query, displacement;
    sint32 status = -1, best, signed_y;
    flags = path_flags;
    if ((flags & 0x440000) != 0)
    {
        for (phase = 0; phase < 3; ++phase)
        {
            for (offset = 0; offset < 10; ++offset)
                work.chains[phase][offset] = fallback_defaults.chains[offset];
        }

        cur = camera_motion.path;
        node = cur->adjacent[0];
        if (node != 0 && (node->definition->flags & 0x10) != 0)
        {
            sel = node->fallback;
            if (sel != 0)
            {
                root = sel;
                if (sel != cur)
                {
                    sel_frame = sel->definition->FindHeightFrame(pos, dir, 0xFFFB0000);
                    if (sel_frame != 0xFFFFFFFF)
                        sel_frame = sel->definition->ProjectFrame(sel_frame, pos, work.probe, dir, 4096);
                }
                if (sel_frame != 0xFFFFFFFF)
                {
                    val = work.probe.x;
                    x = pos.x;
                    y = work.probe.y;
                    work.probe.x = val - x;
                    x = pos.y;
                    y -= x;
                    z = work.probe.z;
                    work.probe.y = y;
                    x = pos.z;
                    work.probe.z = z - x;
                    if ((sint32)y > 0)
                        sel_frame = 0xFFFFFFFF;
                    if (sel_frame != 0xFFFFFFFF)
                        goto return_sel;
                }
                node = sel->adjacent[1];
                if (node != 0)
                {
                    cur = camera_motion.path;
                    sel = node;
                    if (sel != cur)
                    {
                        prev_frame = sel->definition->FindHeightFrame(pos, dir, 0xFFFB0000);
                        if (prev_frame != 0xFFFFFFFF)
                            prev_frame = sel->definition->ProjectFrame(prev_frame, pos, work.probe, dir, 4096);
                    }
                    if (prev_frame != 0xFFFFFFFF)
                    {
                        val = work.probe.x;
                        x = pos.x;
                        y = work.probe.y;
                        work.probe.x = val - x;
                        x = pos.y;
                        y -= x;
                        z = work.probe.z;
                        work.probe.y = y;
                        x = pos.z;
                        work.probe.z = z - x;
                        if ((sint32)y > 0)
                            prev_frame = 0xFFFFFFFF;
                        if (prev_frame != 0xFFFFFFFF)
                            goto return_sel;
                    }
                }
                if ((root->node.state_flags & 0x10) != 0)
                {
                    node = root->fallback;
                    if (node != 0)
                    {
                        cur = camera_motion.path;
                        sel = node;
                        if (sel != cur)
                        {
                            next_frame = sel->definition->FindHeightFrame(pos, dir, 0xFFFB0000);
                            if (next_frame != 0xFFFFFFFF)
                            {
                                next_frame = sel->definition->ProjectFrame(next_frame, pos, work.probe, dir, 4096);
                            }
                        }
                        if (next_frame != 0xFFFFFFFF)
                        {
                            val = work.probe.x;
                            x = pos.x;
                            y = work.probe.y;
                            work.probe.x = val - x;
                            x = pos.y;
                            y -= x;
                            z = work.probe.z;
                            work.probe.y = y;
                            x = pos.z;
                            work.probe.z = z - x;
                            if ((sint32)y > 0)
                                next_frame = 0xFFFFFFFF;
                            if (next_frame != 0xFFFFFFFF)
                                goto return_sel;
                        }
                    }
                }
            }
        }
        work.chains[0][0] = camera_motion.path;
        work.chains[1][0] = camera_motion.path;
        for (scan = 1; scan < 10; ++scan)
        {
            chain0 = work.chains[0] + scan;
            chain1 = work.chains[1] + scan;
            chain2 = work.chains[2] + scan;
            node = chain0[-1];
            if (node != 0)
                *chain0 = node->adjacent[1];
            cur = camera_motion.path;
            if (*chain0 == cur)
                *chain0 = 0;
            node = chain1[-1];
            if (node != 0)
                *chain1 = node->adjacent[0];
            cur = camera_motion.path;
            if (*chain1 == cur)
            {
                node = *chain0;
                if ((node->definition->flags & 0x100011) != 0)
                    *chain1 = (node->definition->flags & 0x100010) != 0 ? node->fallback : 0;
                else
                    *chain1 = 0;
            }
            if (*chain1 != 0)
            {
                node = chain1[-1];
                if ((node->definition->flags & 0x100011) != 0)
                    *chain2 = (node->definition->flags & 0x100010) != 0 ? node->fallback : 0;
            }
            slot = 0;
            while (*chain0 != 0 || *chain1 != 0)
            {
                candidate = *(slot == 0 ? chain0 : slot == 1 ? chain1 : chain2);
                if (candidate != 0)
                {
                    query = candidate->definition->FindHeightFrame(pos, dir, 0xFFFB0000);
                    if (query != 0xFFFFFFFF)
                    {
                        status = candidate->definition->ProjectFrame(query, pos, work.scratch, dir, 4096);
                        // Keep the copied query result separate from later status rejection
                        query = status;
                    }
                    if (status >= 0)
                    {
                        x = work.scratch.x - (uint32)pos.x + 24576;
                        if (x > 0xC000)
                            status = -1;
                        else
                        {
                            z = work.scratch.z - (uint32)pos.z + 24576;
                            if (z > 0xC000)
                                status = -1;
                        }
                    }
                }
                else
                    query = 0xFFFFFFFF;
                if (query != 0xFFFFFFFF)
                {
                    x = work.scratch.x - (uint32)pos.x;
                    work.scratch.x = x;
                    y = work.scratch.y - (uint32)pos.y;
                    z = work.scratch.z;
                    work.scratch.y = y;
                    z -= (uint32)pos.z;
                    work.scratch.z = z;
                    if ((sint32)y > 0)
                        query = 0xFFFFFFFF;
                    if (query != 0xFFFFFFFF)
                    {
                        sel = candidate;
                        goto return_sel;
                    }
                }
                slot += 1;
                if (slot >= 3)
                    break;
            }
        }
        for (offset = 0; offset < 5; ++offset)
            work.candidates[offset] = fallback_defaults.candidates[offset];
        if (work.chains[1][1] != 0)
        {
            if (work.chains[1][2] != 0)
            {
                cur = camera_motion.path;
                adjacent_path = cur->adjacent[0];
                adjacent_path = adjacent_path->adjacent[0];
                adjacent_path = adjacent_path->adjacent[1];
                work.candidates[0] = adjacent_path;
                cur = camera_motion.path;
                if (adjacent_path == cur)
                    work.candidates[0] = 0;
            }

            cur = camera_motion.path;
            adjacent_path = cur->adjacent[0];
            node = adjacent_path->adjacent[1];
            if (node != 0)
            {
                adjacent_path = node->adjacent[1];
                work.candidates[1] = adjacent_path;
                cur = camera_motion.path;
                if (adjacent_path == cur)
                {
                    work.candidates[1] = 0;
                    cur = camera_motion.path;
                }
                node = cur->adjacent[0];
                adjacent_path = node->adjacent[1];
                if (adjacent_path != 0)
                {
                    work.candidates[2] = adjacent_path;
                    if (adjacent_path == camera_motion.path)
                        work.candidates[2] = 0;
                }
            }
        }
        node = work.chains[0][3];
        if (node != 0)
        {
            adjacent_path = node->adjacent[0];
            work.candidates[3] = adjacent_path;
            if (adjacent_path == camera_motion.path)
                work.candidates[3] = 0;
        }
        node = work.chains[1][1];
        if (node != 0 && (node->definition->flags & 0x10) != 0)
        {
            node = node->fallback;
            if (node != 0)
            {
                adjacent_path = node->adjacent[1];
                work.candidates[4] = adjacent_path;
                if (adjacent_path == camera_motion.path)
                    work.candidates[4] = 0;
            }
        }
        for (scan = 0; scan < 5; ++scan)
        {
            sel = work.candidates[scan];
            query = 0xFFFFFFFF;
            if (sel != 0)
            {
                query = sel->definition->FindHeightFrame(pos, dir, 0xFFFB0000);
                if (query != 0xFFFFFFFF)
                    query = sel->definition->ProjectFrame(query, pos, work.scratch, dir, 4096);
            }
            if (query != 0xFFFFFFFF)
            {
                x = work.scratch.x - (uint32)pos.x;
                work.scratch.x = x;
                y = work.scratch.y - (uint32)pos.y;
                z = work.scratch.z;
                work.scratch.y = y;
                z -= (uint32)pos.z;
                work.scratch.z = z;
                if ((sint32)y > 0)
                    query = 0xFFFFFFFF;
                if (query != 0xFFFFFFFF)
                    goto return_sel;
            }
        }
        sel = 0;
        goto return_sel;
    }
    if ((flags & 4) != 0)
    {
        status = -1;
        cur = camera_motion.path;
        if (cur->adjacent[1] != 0)
        {
            for (offset = 0; offset < 4; ++offset)
                work.alternate[offset] = fallback_defaults.alternate[offset];
            cur = camera_motion.path;
            adjacent_path = cur->adjacent[1];
            work.alternate[0] = adjacent_path;
            if (adjacent_path != 0)
            {
                cur = camera_motion.path;
                adjacent_path = cur->adjacent[1];
                adjacent_path = adjacent_path->adjacent[1];
                work.alternate[1] = adjacent_path;
                if (adjacent_path != 0)
                {
                    cur = camera_motion.path;
                    adjacent_path = cur->adjacent[1];
                    adjacent_path = adjacent_path->adjacent[1];
                    adjacent_path = adjacent_path->adjacent[1];
                    work.alternate[2] = adjacent_path;
                    if (adjacent_path != 0)
                    {
                        cur = camera_motion.path;
                        adjacent_path = cur->adjacent[1];
                        adjacent_path = adjacent_path->adjacent[1];
                        adjacent_path = adjacent_path->adjacent[1];
                        work.alternate[3] = adjacent_path->adjacent[1];
                    }
                }
            }
            for (scan = 0; scan < 4; ++scan)
            {
                if (status != -1)
                    goto return_sel;
                sel = work.alternate[scan];
                query = 0xFFFFFFFF;
                if (sel != 0)
                    query = sel->definition->FindHeightFrame(pos, dir, 0xFFFB0000);
                if (query != 0xFFFFFFFF)
                    status = sel->definition->ProjectFrame(query, pos, work.probe, dir, 4096);
                if (status != -1)
                {
                    x = work.probe.x - (uint32)pos.x;
                    work.probe.x = x;
                    y = work.probe.y - (uint32)pos.y;
                    z = work.probe.z;
                    work.probe.y = y;
                    z -= (uint32)pos.z;
                    work.probe.z = z;
                    if ((sint32)y > 0)
                        status = -1;
                    if (status != -1 && (sel->definition->flags & 0x10) != 0)
                    {
                        val = SinQ14(yaw);
                        displacement = mul12(((sint32)val >> 2), 20480);
                        val = SinQ14((1024 - (uint16)yaw));
                        val = mul12(((sint32)val >> 2), 20480);

                        work.displaced = pos;
                        x = work.displaced.x;
                        z = work.displaced.z;
                        work.displaced.x = x - displacement;
                        work.displaced.z = z - val;
                        query = sel->definition->FindHeightFrame(work.displaced, dir, 0xFFFB0000);
                        if (query == 0xFFFFFFFF)
                            status = -1;
                        else
                            status = sel->definition->ProjectFrame(query, work.displaced, work.probe, dir, 8192);
                    }
                }
            }
        }
        if (status == -1)
            sel = 0;
        goto return_sel;
    }
    if ((flags & 0x20) == 0)
    {
        signed_y = (sint8)path_routing.fallback_phase;
        if (signed_y == 1)
        {
            cur = camera_motion.path;
            path_routing.fallback_phase = 2;
            sel = cur->fallback;
            goto return_sel;
        }
        sel = 0;
        if (movement_mode != 2 || state != 19)
            goto return_sel;
        cur = camera_motion.path;
        if ((cur->definition->flags & 0x90010) == 0)
            goto return_sel;
        sel = cur->adjacent[1];
        status = -1;
        // Original stack-pointer frame argument is negative and clamps to zero
        if (sel != 0)
            status = sel->definition->ProjectFrame(0, pos, work.endpoint, dir, 4096);
        if (status == -1)
            sel = 0;
        goto return_sel;
    }
    for (offset = 0; offset < 9; ++offset)
        work.ranked[offset] = fallback_defaults.ranked[offset];
    for (offset = 0; offset < 9; ++offset)
        work.costs[offset] = fallback_defaults.costs[offset];
    cur = camera_motion.path;
    sel = cur->fallback;
    best = -1;
    status = -1;
    if (sel != 0 && (cur->definition->flags & 0x100000) != 0)
    {
        query = sel->definition->FindHeightFrame(pos, dir, 0xFFFB0000);
        if (query != 0xFFFFFFFF)
            status = sel->definition->ProjectFrame(query, pos, work.displaced, dir, 81920);
        if (status != -1)
        {
            x = work.displaced.x - (uint32)pos.x;
            work.displaced.x = x;
            y = work.displaced.y - (uint32)pos.y;
            z = work.displaced.z;
            work.displaced.y = y;
            z -= (uint32)pos.z;
            work.displaced.z = z;
            if ((sint32)y > 0)
                status = -1;
        }
    }
    anchor_path = character_pad.path_transition.anchor_path;
    node = anchor_path->adjacent[1];
    if (node != 0)
    {
        cur = camera_motion.path;
        if (cur == node)
        {
            sel = cur;
            query = cur->definition->FindHeightFrame(pos, dir, 0xFFFB0000);
            status = sel->definition->ProjectFrame(query, pos, work.displaced, dir, 4096);
            if (status >= 0)
            {
                x = work.displaced.x - (uint32)pos.x + 24576;
                if (x > 0xC000)
                    goto ranked_shortcut_finished;
                z = work.displaced.z - (uint32)pos.z + 24576;
                if (z > 0xC000)
                    goto ranked_shortcut_finished;
            }
            x = work.displaced.x - (uint32)pos.x;
            work.displaced.x = x;
            y = work.displaced.y - (uint32)pos.y;
            z = work.displaced.z;
            work.displaced.y = y;
            z -= (uint32)pos.z;
            work.displaced.z = z;
            if ((sint32)y >= 4097)
                status = -1;
        }
    }
ranked_shortcut_finished:
    if (status != -1)
        goto ranked_finished;

    cur = camera_motion.path;
    adjacent_path = cur->adjacent[1];
    work.ranked[0] = adjacent_path;
    if (adjacent_path != 0)
    {
        cur = camera_motion.path;
        adjacent_path = cur->adjacent[1];
        work.ranked[1] = adjacent_path->adjacent[1];
    }
    cur = camera_motion.path;
    node = cur->adjacent[0];
    if (node != 0)
    {
        work.ranked[2] = node->adjacent[1];
        cur = camera_motion.path;
        adjacent_path = cur->adjacent[0];
        node = adjacent_path->adjacent[0];
        if (node != 0)
            work.ranked[3] = node->adjacent[1];
    }
    for (phase = 0; phase < 3; ++phase)
    {
        if (phase == 0)
        {
            scan = 0;
            end = 4;
        }
        else if (phase == 1)
        {
            if (work.ranked[1] != 0)
            {
                cur = camera_motion.path;
                adjacent_path = cur->adjacent[1];
                adjacent_path = adjacent_path->adjacent[1];
                work.ranked[4] = adjacent_path->adjacent[1];
            }
            cur = camera_motion.path;
            node = cur->adjacent[0];
            if (node != 0)
            {
                node = node->adjacent[0];
                if (node != 0)
                {
                    node = node->adjacent[0];
                    if (node != 0)
                        work.ranked[5] = node->adjacent[1];
                }
            }
            scan = 4;
            end = 6;
        }
        else
        {
            if (work.ranked[0] != 0)
            {
                cur = camera_motion.path;
                adjacent_path = cur->adjacent[1];
                work.ranked[6] = adjacent_path->adjacent[0];
            }

            cur = camera_motion.path;
            node = cur->adjacent[0];
            if (node != 0)
                work.ranked[7] = node->adjacent[0];
            cur = camera_motion.path;
            work.ranked[8] = cur->adjacent[0];
            scan = 6;
            end = 9;
        }
        for (; scan < end; ++scan)
        {
            candidate = work.ranked[scan];
            if (candidate == 0)
                continue;
            cur = camera_motion.path;
            parent = candidate->definition;
            same_definition = parent == cur->definition;
            if (same_definition != 0 || candidate == cur)
                continue;
            query = parent->FindHeightFrame(pos, dir, 0xFFFB0000);
            work.frames[scan] = query;
            if (query == 0xFFFFFFFF)
                continue;
            candidate = work.ranked[scan];
            status = candidate->definition->ProjectFrame(work.frames[scan], pos, work.displaced, dir, 4096);
            if (status >= 0)
            {
                x = work.displaced.x - (uint32)pos.x + 24576;
                if (x > 0xC000)
                    status = -1;
                else
                {
                    z = work.displaced.z - (uint32)pos.z + 24576;
                    if (z > 0xC000)
                        status = -1;
                }
            }
            if (status != -1)
            {
                x = work.displaced.x - (uint32)pos.x;
                work.displaced.x = x;
                y = work.displaced.y - (uint32)pos.y;
                z = work.displaced.z;
                work.displaced.y = y;
                z -= (uint32)pos.z;
                work.displaced.z = z;
                if ((sint32)y > 0)
                    status = -1;
                else
                {
                    val = -y;
                    work.costs[scan] = val;
                    if (best == -1 || (sint32)val < (sint32)work.costs[(uint32)best])
                        best = scan;
                }
            }
        }
        if (best != -1)
        {
            if (phase == 0)
                status = work.frames[(uint32)best];
            sel = work.ranked[(uint32)best];
        }
        if (phase < 2 && status != -1)
            goto ranked_finished;
    }
ranked_finished:
    if (status < 0)
    {
        MarkPathSearchFailed();
        sel = 0;
    }
return_sel:
    return sel;
}

uint32 AnimatedObject::CanChangePath(const Vec3 &pos, uint32 dir) const
{
    FUNCTION_MARKER(0x8003D190, "SLUS_005.78");
    Vec3 floor;
    const ScenePath *path, *path_view;
    uint32 flags, x, y, z, result;
    sint32 frame;

    flags = path_flags;
    path = camera_motion.path;
    path_view = path;
    if ((flags & 6) == 2)
    {
        return 0;
    }
    if ((flags & 4) && CharacterController::ReadCharacterConstrainedPath() != path)
    {
        frame = path_view->definition->FindHeightFrame(pos, dir, -40960);
        if (frame != -1)
        {
            frame = path_view->definition->ProjectFrame(frame, pos, floor, dir, 4096);
        }
        if (frame >= 0)
        {
            x = floor.x;
            x -= (uint32)pos.x;
            if (x + 24576 > 0xC000)
                frame = -1;
            else
            {
                z = floor.z;
                z -= (uint32)pos.z;
                if (z + 24576 > 0xC000)
                    frame = -1;
            }
        }
        if (frame != -1)
        {
            x = floor.x;
            x -= (uint32)pos.x;
            y = floor.y;
            floor.x = x;
            y -= (uint32)pos.y;
            z = floor.z;
            floor.y = y;
            z -= (uint32)pos.z;
            floor.z = z;
            if ((sint32)y <= 0)
            {
                return 0;
            }
        }
    }
    result = 1;
    if ((path_flags & 0x20) && (CharacterController::IsCharacterAnchorPathActive() & 0xFF) == 0)
    {
        frame = path_view->definition->FindHeightFrame(pos, dir, -40960);
        if (frame != -1)
        {
            frame = path_view->definition->ProjectFrame(frame, pos, floor, dir, 4096);
        }
        if (frame >= 0)
        {
            x = floor.x;
            x -= (uint32)pos.x;
            if (x + 24576 > 0xC000)
                frame = -1;
            else
            {
                z = floor.z;
                z -= (uint32)pos.z;
                if (z + 24576 > 0xC000)
                    frame = -1;
            }
        }
        if (frame != -1)
        {
            x = floor.x;
            x -= (uint32)pos.x;
            y = floor.y;
            floor.x = x;
            y -= (uint32)pos.y;
            z = floor.z;
            floor.y = y;
            z -= (uint32)pos.z;
            floor.z = z;
            if ((sint32)y <= 0)
                result = 0;
        }
    }
    return result;
}

uint32 AnimatedObject::ComputePathMotion(sint32 distance, sint32 height, const Vec3 *dir_vector)
{
    FUNCTION_MARKER(0x8003DB7C, "SLUS_005.78");
    Vec3 working, original, next, cur, tmp, motion;
    const ScenePath *initial_path = 0, *path, *path_view, *candidate, *candidate_view, *cur_path;
    uint32 character = 0, special = 0, special_path = 0;
    const PathDefinition *definition;
    MATRIX *matrix;
    uint32 cur_flags, val, x, y, z, sq, dir, track_dir;
    uint32 stage, stop, valid, first_seg;
    uint32 prev_x, prev_y, prev_z, result;
    sint32 category, frame, limit, drop = 0, base_height, initial_distance, rcp, factor, len, height_step;
    category = contact_kind;
    if (category == 1)
    {
        if (behavior != 33)
            initial_path = camera_motion.path;
        else
            initial_path = scene_path;
    }
    else if (category == 3 && animation_binding.tracks[0] == -1)
        initial_path = scene_path;
    path_routing.fallback_phase = 0;
    category = contact_kind;
    if (category == 1)
        character = 0x800ABD68;
    else if (movement_mode == 2 && (state == 19 || path_routing.flags != 0))
    {
        CharacterController::FindPlayerObject(0);
        special = 1;
        if ((path_definition->flags & 0x80000) != 0)
            special_path = 1;
    }
    if (special != 0 && (path_definition->flags & 0x10000) != 0)
    {
        definition = path_definition;
        if (definition != 0 && definition->transform != 0)
        {
            tmp = pos;
            PushMatrix();
            matrix = path_definition->transform != 0 ? &path_definition->transform->matrix : NULL;
            {
                SetRotMatrix(matrix);
            }
            {
                VECTOR sdk_input = {tmp.x, tmp.y, tmp.z, 0};
                VECTOR sdk_output;
                ApplyRotMatrixLV(&sdk_input, &sdk_output);
                original.x = sdk_output.vx;
                original.y = sdk_output.vy;
                original.z = sdk_output.vz;
            }
            PopMatrix();
            matrix = path_definition->transform != 0 ? &path_definition->transform->matrix : NULL;
            original.x = (uint32)original.x + matrix->t[0];
            matrix = path_definition->transform != 0 ? &path_definition->transform->matrix : NULL;
            original.y = (uint32)original.y + matrix->t[1];
            matrix = path_definition->transform != 0 ? &path_definition->transform->matrix : NULL;
            original.z = (uint32)original.z + matrix->t[2];
        }
        else
        {
            original = pos;
        }

        cur = original;

        working = cur;
        ChangeActorPath(camera_motion.path, working, cur);
    }
    initial_distance = distance;
    base_height = height;
    if (distance < 0)
    {
        initial_distance = 0;
        distance = 0;
    }
    cur_flags = path_flags;
    if ((cur_flags & 0x440000) != 0)
    {
        CompleteCharacterPathTransfer(character != 0 ? &character_pad : 0);
        goto check_direct;
    }
    if ((flags & 0x2000) != 0 && (cur_flags & 4) != 0)
        goto prepare_contact;
    if ((path_flags & 0x20) == 0)
        goto check_direct;
    if ((flags & 0x2000) == 0)
        goto prepare_pos;
prepare_contact:
    definition = path_definition;
    if (definition != 0 && definition->transform != 0)
    {
        tmp = pos;
        PushMatrix();
        matrix = path_definition->transform != 0 ? &path_definition->transform->matrix : NULL;
        {
            SetRotMatrix(matrix);
        }
        {
            VECTOR sdk_input = {tmp.x, tmp.y, tmp.z, 0};
            VECTOR sdk_output;
            ApplyRotMatrixLV(&sdk_input, &sdk_output);
            original.x = sdk_output.vx;
            original.y = sdk_output.vy;
            original.z = sdk_output.vz;
        }
        PopMatrix();
        matrix = path_definition->transform != 0 ? &path_definition->transform->matrix : NULL;
        original.x = (uint32)original.x + matrix->t[0];
        matrix = path_definition->transform != 0 ? &path_definition->transform->matrix : NULL;
        original.y = (uint32)original.y + matrix->t[1];
        matrix = path_definition->transform != 0 ? &path_definition->transform->matrix : NULL;
        original.z = (uint32)original.z + matrix->t[2];
    }
    else
    {
        original = pos;
    }

    cur = original;

    working = cur;
    ChangeActorPath(camera_motion.path, working, cur);
check_direct:
    if ((flags & 0x2000) != 0)
        goto direct_vector;
prepare_pos:
    definition = path_definition;
    if (definition != 0 && definition->transform != 0)
    {
        tmp = pos;
        PushMatrix();
        matrix = path_definition->transform != 0 ? &path_definition->transform->matrix : NULL;
        {
            SetRotMatrix(matrix);
        }
        {
            VECTOR sdk_input = {tmp.x, tmp.y, tmp.z, 0};
            VECTOR sdk_output;
            ApplyRotMatrixLV(&sdk_input, &sdk_output);
            original.x = sdk_output.vx;
            original.y = sdk_output.vy;
            original.z = sdk_output.vz;
        }
        PopMatrix();
        matrix = path_definition->transform != 0 ? &path_definition->transform->matrix : NULL;
        original.x = (uint32)original.x + matrix->t[0];
        matrix = path_definition->transform != 0 ? &path_definition->transform->matrix : NULL;
        original.y = (uint32)original.y + matrix->t[1];
        matrix = path_definition->transform != 0 ? &path_definition->transform->matrix : NULL;
        original.z = (uint32)original.z + matrix->t[2];
    }
    else
    {
        original = pos;
    }

    cur = original;

    working = cur;
    dir = flags;
    dir &= 0x10;
    if ((flags & 0x400) == 0 && (path_flags & 0x1222) == 0)
        goto prepare_next;
    for (;;)
    {
        result = path_definition->ProjectFrame(path_frame, working, cur, dir, 4096);
        if ((sint32)result >= 0)
            break;
        frame = path_frame;
        if (frame != path_definition->pos_track.frame_count - 1)
        {
            dir_vector = &working;
            if (contact_kind == 1)
            {
                DetachCharacterFromPath(&working);
                goto direct_vector;
            }
            break;
        }
        path_frame = (uint32)frame - 1;
    }
    if (working.y < cur.y && base_height >= 0)
        cur.y = working.y;
    if (distance == 0 && base_height < 0 && (path_flags & 0x20) == 0)
    {
        x = cur.x - (uint32)working.x;
        y = cur.y - (uint32)working.y;
        z = cur.z - (uint32)working.z;
        motion.x = x;
        motion.y = y;
        motion.z = z;
        if ((sint32)y < 0)
        {
            motion.y = 0;
            rcp = 0x40000000 / (sint32)y;
            if (x + 26754 > 0xD103 || z + 26754 > 0xD103)
            {
                x = motion.x;
                y = motion.y;
                z = motion.z;
                sq = mul12((sint32)x, (sint32)x);
                sq += (uint32)mul12((sint32)y, (sint32)y);
                sq += (uint32)mul12((sint32)z, (sint32)z);
            }
            else
                sq = (sint32)(x * x + z * z) >> 12;
            if ((sint32)sq < 0)
                val = SquareRoot12((sq >> 2)) << 1;
            else
                val = SquareRoot12(sq);
            factor = mul12((sint32)val, base_height);
            distance = ((sint64)factor * rcp) >> 18;
            fixed_math_result = distance;
            initial_distance = distance;
            if (distance < 0)
            {
                distance = 0;
                initial_distance = 0;
            }
        }
    }
    drop = working.y - (uint32)cur.y;
    cur.x = working.x;
    cur.z = working.z;
    if (drop < -1365 && (path_flags & 0x20) == 0 && (special_path & special) == 0 && (character == 0 || ((uint16)(character_pad.motion.form_flags >> 16) & 0xF) != 5) && (path_definition->flags & 0x11) == 0)
    {
        working = cur;
        initial_distance = 0;
        base_height = 0;
        first_seg = 0;
        goto finish_fall;
    }
prepare_next:
    x = (uint32)cur.x;
    y = cur.y;
    z = cur.z;
    next.x = x;
    next.y = y;
    next.z = z;
restart_path:
    frame = path_frame;
    first_seg = frame < 1;
    if (frame >= 1 && (flags & 0x40) != 0)
        path_frame = (uint32)frame - 1;
    category = contact_kind;
    path = 0;
    if (category == 1)
    {
        if (behavior != 33)
            path = camera_motion.path;
        else
            path = scene_path;
    }
    else if (category == 3 && animation_binding.tracks[0] == -1)
        path = scene_path;
    path_view = path;
    stop = 0;
    if (base_height <= 0)
    {
        cur_flags = path_definition->flags;
        if ((cur_flags & 0x51) >= 0x41 && ((cur_flags & 0x10) == 0 || (path_view != 0 && (path_view->fallback != 0 || path_view->slope_scale != 0))))
            stop = 1;
    }
next_seg:
    track_dir = dir;
    candidate = path;
    frame = path_frame;
    if ((flags & 0x40) != 0)
        frame += 1;
    else
        frame -= 1;
    definition = path_definition;
    path_frame = frame;
    valid = 1;
    if (((uint32)frame & 0xFFFF) < (uint16)definition->pos_track.frame_count)
    {
        definition->SamplePos((sint16)frame, next, track_dir);
        if (path != 0 && ((flags & 0x400) != 0 || (path_flags & 0x1022) != 0))
        {
            val = path_frame;
            if ((flags & 0x40) == 0)
            {
                if ((candidate->node.state_flags & 8) != 0)
                {
                    candidate = candidate->adjacent[0];
                }
                track_dir ^= 0x10;
                val = path_definition->pos_track.frame_count - val - 1;
            }
            candidate_view = candidate;
            if ((candidate_view->node.state_flags & (track_dir | 8)) != 0)
                limit = candidate_view->motion_limits[0];
            else
                limit = candidate_view->motion_limits[1];
            if (val >= (uint32)limit && limit != -1)
            {
                if ((uint32)limit < val)
                    flags = flags | 0x2000;
                valid = 0;
                flags = flags | 0x4000;
            }
        }
    }
    else
    {
        valid = 0;
        flags = flags | 0x800;
    }
    if (valid != 0 && (stop == 0 || drop >= 0) && (path_flags & 8) == 0)
        goto advance;
    cur_flags = path_flags;
    if ((cur_flags & 0x80000) != 0)
    {
        result = (cur_flags & 0xFFF7FFFF) | 0x100000;
        path_flags = result;
        velocity.x = 0;
        velocity.y = 0;
        goto done;
    }
    cur_flags = flags;
    if ((cur_flags & 0x6000) != 0 && drop < 0)
    {
        flags = cur_flags & 0xFFFF9FFF;
        if ((cur_flags & 0x800) == 0 && stop == 0)
            goto advance;
    }
    if (distance == 0 && (path_flags & 8) == 0)
    {
        next = cur;
        cur_flags = flags;
        flags = cur_flags & 0xFFFFF7FF;
        if ((cur_flags & 0x6000) == 0 && (stop == 0 || drop >= 0))
            goto advance;
    }
    prev_x = velocity.x;
    prev_y = velocity.y;
    prev_z = velocity.z;
    if ((path_flags & 2) != 0 && (camera_motion.path->definition->flags & 0x10000) == 0 && CharacterController::CanContinuePathStep() != 0)
        goto end_contact;
    if (character != 0 && character_pad.motion.slope_scale != 0 && (uint32)initial_distance - 1 < 0x3FF && base_height < 512)
        goto end_contact;
    if ((ChangeActorPath(path, working, cur) & 0xFF) == 0)
        goto end_contact;
    if ((path_view->definition->flags & 0x40) != 0)
    {
        drop = working.y - (uint32)cur.y;
        if (drop < 1366)
        {
            if ((flags & 0x400) != 0)
                goto adjust_velocity;
            if ((path_flags & 0x1022) == 0)
            {
                working = cur;
            }
        }
        else
            flags = flags | 0x400;
    }
    if ((flags & 0x400) == 0 && (path_flags & 0x1022) == 0)
        goto check_path_change;
adjust_velocity:
    x = velocity.x - prev_x;
    motion.x = x;
    y = velocity.y - prev_y;
    motion.y = y;
    z = velocity.z - prev_z;
    motion.z = z;
    base_height = base_height + y;
    motion.y = 0;
    height = height + y;
    if (x + 26754 > 0xD103 || z + 26754 > 0xD103)
    {
        x = motion.x;
        y = motion.y;
        z = motion.z;
        sq = mul12((sint32)x, (sint32)x);
        sq += (uint32)mul12((sint32)y, (sint32)y);
        sq += (uint32)mul12((sint32)z, (sint32)z);
    }
    else
        sq = (sint32)(x * x + z * z) >> 12;
    if ((sint32)sq < 0)
        motion.y = SquareRoot12((sq >> 2)) << 1;
    else
        motion.y = SquareRoot12(sq);
    val = Atan2Angle(motion.z, motion.x);
    if ((((uint32)(sint16)val - target_yaw - 1024) & 0xFFF) >= 0x800)
        motion.y = -(uint32)motion.y;
    if ((sint32)(distance + (uint32)motion.y) <= 0)
        motion.y = 1 - (uint32)distance;
    initial_distance = initial_distance + (uint32)motion.y;
    distance = distance + (uint32)motion.y;
check_path_change:
    for (stage = 2; stage <= 5; ++stage)
    {
        if (path == initial_path)
            break;
        category = contact_kind;
        cur_path = 0;
        if (category == 1)
        {
            if (behavior != 33)
                cur_path = camera_motion.path;
            else
                cur_path = scene_path;
        }
        else if (category == 3 && animation_binding.tracks[0] == -1)
            cur_path = scene_path;
        if (cur_path != initial_path)
            continue;
        if (stage == 2 && (path_view->definition->flags & 0x10) == 0)
            continue;
        if (stage == 3 && (camera_motion.path->definition->flags & 0x80000) == 0)
            continue;
        if (stage == 4 && (initial_path->definition->flags & 0x10) == 0)
            continue;
        if (stage == 5 && special == 0)
            continue;
        if (path_routing.fallback_phase == 0)
        {
            cur_flags = flags;
            path_routing.fallback_phase = stage;
            flags = cur_flags | 0x800;
            goto end_contact;
        }
    }
    flags = flags & 0xFFFFF7FF;
    goto restart_path;
end_contact:
    cur_flags = flags;
    flags = cur_flags & 0xFFFF9FFF;
    if ((cur_flags & 0x800) == 0)
        goto advance;
    category = contact_kind;
    if (category == 1 || (((uint32)category - 3) & 0xFF) < 2)
    {
        {
            VECTOR sdk_output;
            path_definition->SampleTangent(&sdk_output, dir);
            motion.x = sdk_output.vx;
            motion.y = sdk_output.vy;
            motion.z = sdk_output.vz;
        }
        if ((flags & 0x400) != 0 || (path_flags & 0x1002) != 0)
        {
            x = motion.x;
            motion.y = 0;
            if (x + 26754 > 0xD103 || (uint32)motion.z + 26754 > 0xD103)
            {
                do
                {
                    x = motion.x >> 1;
                    y = motion.y >> 1;
                    z = motion.z >> 1;
                    motion.x = x;
                    motion.y = y;
                    motion.z = z;
                } while (x + 26754 > 0xD103 || y + 26754 > 0xD103 || z + 26754 > 0xD103);
            }
            {
                VECTOR sdk_input = {motion.x, motion.y, motion.z, 0};
                VECTOR sdk_output;
                VectorNormal(&sdk_input, &sdk_output);
                motion.x = sdk_output.vx;
                motion.y = sdk_output.vy;
                motion.z = sdk_output.vz;
            }
        }
        if ((flags & 0x40) == 0)
            distance = -(uint32)distance;
        motion.x = mul12(motion.x, distance);
        motion.y = mul12(motion.y, distance);
        motion.z = mul12(motion.z, distance);
        x = working.x + (uint32)motion.x;
        y = working.y + (uint32)motion.y;
        z = working.z + (uint32)motion.z;
        working.x = x;
        working.y = y;
        working.z = z;
        working.y = y + height;
    }
    velocity.x = working.x - (uint32)original.x;
    velocity.y = working.y - (uint32)original.y;
    result = original.z;
    velocity.z = working.z - result;
    goto done;
advance:
    if ((flags & 0x400) != 0 || (path_flags & 0x1222) != 0)
    {
        x = next.x - (uint32)cur.x;
        y = next.y - (uint32)cur.y;
        z = next.z - (uint32)cur.z;
        motion.x = x;
        motion.y = y;
        motion.z = z;

        cur = next;
        val = motion.y;
        x = motion.x;
        motion.y = 0;
        next.y = val;
        z = motion.z;
        if (x + 26754 > 0xD103 || z + 26754 > 0xD103)
        {
            x = motion.x;
            y = motion.y;
            z = motion.z;
            sq = mul12((sint32)x, (sint32)x);
            sq += (uint32)mul12((sint32)y, (sint32)y);
            sq += (uint32)mul12((sint32)z, (sint32)z);
        }
        else
            sq = (sint32)(x * x + z * z) >> 12;
        if ((sint32)sq < 0)
            len = SquareRoot12((sq >> 2)) << 1;
        else
            len = SquareRoot12(sq);
        motion.y = next.y;
    }
    else
    {
        x = next.x - (uint32)working.x;
        y = next.y - (uint32)working.y;
        z = next.z - (uint32)working.z;
        motion.x = x;
        motion.y = y;
        motion.z = z;
        if (x + 26754 > 0xD103 || y + 26754 > 0xD103 || z + 26754 > 0xD103)
        {
            x = motion.x;
            y = motion.y;
            z = motion.z;
            sq = mul12((sint32)x, (sint32)x);
            sq += (uint32)mul12((sint32)y, (sint32)y);
            sq += (uint32)mul12((sint32)z, (sint32)z);
        }
        else
            sq = (sint32)(x * x + y * y + z * z) >> 12;
        if ((sint32)sq < 0)
            len = SquareRoot12((sq >> 2)) << 1;
        else
            len = SquareRoot12(sq);
    }
    if (distance < len)
    {
        if (len == 0)
        {
            xport_mips_break(7);
            rcp = -1;
        }
        else
            rcp = 0x40000000 / len;
        factor = ((sint64)distance * rcp) >> 18;
        cur_flags = flags;
        fixed_math_result = factor;
        distance = 0;
        if ((cur_flags & 0x40) == 0)
            path_frame = (uint16)path_frame + 1;
        factor = fixed_math_result;
        motion.x = mul12(motion.x, factor);
        motion.y = mul12(motion.y, factor);
        motion.z = mul12(motion.z, factor);
        height_step = height;
    }
    else if (height != 0)
    {
        if (distance != 0)
        {
            rcp = 0x40000000 / distance;
            factor = ((sint64)len * rcp) >> 18;
            distance = (uint32)distance - len;
            fixed_math_result = factor;
            height_step = mul12(height, factor);
        }
        else
            height_step = height;
    }
    else
    {
        distance = (uint32)distance - len;
        height_step = 0;
    }
    x = working.x + (uint32)motion.x;
    y = working.y + (uint32)motion.y;
    z = working.z + (uint32)motion.z;
    working.x = x;
    working.y = y;
    working.z = z;
    if ((flags & 0x400) != 0 || (path_flags & 0x1222) != 0)
    {
        height = (uint32)height - height_step;
        val = height_step - (uint32)motion.y;
        y += val;
        drop = drop + val;
        working.y = y;
        if (drop < -169 && (base_height <= 0 || (first_seg == 0 && (uint32)drop + 5461 >= 0x1001)) && (character == 0 || ((uint16)(character_pad.motion.form_flags >> 16) & 0xF) != 5) && (path_definition->flags & 0x11) == 0 && ((camera_motion.path->definition->flags & 0x80000) == 0 || (path_flags & 0x20) == 0) && (special_path & special) == 0)
        {
            working.y = y - drop;
            goto finish_fall;
        }
    }
    if (distance != 0)
        goto next_seg;
    if (drop <= 0 && behavior != 1)
    {
        cur_flags = flags;
        if ((cur_flags & 0x400) != 0 && (special == 0 || (path_definition->flags & 0x80010) == 0))
            flags = cur_flags | 0x4000;
    }
    velocity.x = working.x - (uint32)original.x;
    velocity.y = working.y - (uint32)original.y;
    velocity.z = working.z - (uint32)original.z;
    if ((path_flags & 1) != 0 && (camera_motion.path->definition->flags & 0x10000) == 0)
    {
        velocity.z = 0;
        velocity.x = 0;
        character_pad.motion.horizontal_speed = 0;
    }
    goto finish_seg;
finish_fall:
    if ((path_flags & 0x800) != 0)
        ClearCharacterPathStep();
    flags = flags | 0x4000;
    velocity.x = working.x - (uint32)original.x;
    velocity.y = working.y - (uint32)original.y;
    velocity.z = working.z - (uint32)original.z;
    motion.x = initial_distance;
    motion.z = 0;
    motion.y = base_height;
    sq = mul12(initial_distance, initial_distance);
    sq += (uint32)mul12(base_height, base_height);
    if (sq != 0)
    {
        x = velocity.x;
        rcp = 0x40000000 / (sint32)sq;
        val = mul12((sint32)x, (sint32)x);
        y = velocity.y;
        val += (uint32)mul12((sint32)y, (sint32)y);
        z = velocity.z;
        val += (uint32)mul12((sint32)z, (sint32)z);
        factor = ((sint64)(sint32)val * rcp) >> 18;
        fixed_math_result = factor;
        if (factor >= 0)
            result = SquareRoot12(((uint32)factor));
        else
            result = SquareRoot12(((uint32)factor >> 2)) << 1;
    }
    else
        result = 4095;
    path_motion_scale = result;
    val = path_motion_scale;
    if (val != 0)
    {
        rcp = 0x1000000 / (sint32)val;
        x = mul12(velocity.x, rcp);
        val = path_motion_scale;
        if (val == 0)
        {
            xport_mips_break(7);
            rcp = -1;
        }
        else
            rcp = 0x1000000 / (sint32)val;
        velocity.x = x;
        y = mul12(velocity.y, rcp);
        val = path_motion_scale;
        z = velocity.z;
        velocity.y = y;
        if (val == 0)
        {
            xport_mips_break(7);
            rcp = -1;
        }
        else
            rcp = 0x1000000 / (sint32)val;
        velocity.z = mul12((sint32)z, rcp);
    }
finish_seg:
    result = 1;
    if (first_seg != 0)
    {
        result = 4096;
        if (path_frame == 1)
        {
            result = path_definition->ProjectFrame(0, working, cur, dir, 4096);
            path_frame = result;
        }
    }
    goto done;
direct_vector:
    if ((flags & 0x40) == 0)
        distance = -(uint32)distance;
    velocity.x = mul12(dir_vector->x, distance);
    velocity.y = mul12(dir_vector->y, distance);
    velocity.z = mul12(dir_vector->z, distance);
    result = (uint32)velocity.y + height;
    velocity.y = result;
done:
    return result;
}

void AnimatedObject::WrapObjectSpeed()
{
    FUNCTION_MARKER(0x8003FBB0, "SLUS_005.78");
    sint32 count = object_speed_definitions.count;
    uint32 desc = state;
    const ObjectSpeedDefinition *entry = 0;
    uint32 center;
    uint32 extent;
    sint32 pos;
    uint32 result;
    sint32 idx;
    for (idx = 0; idx < count; ++idx)
    {
        const ObjectSpeedDefinition *candidate = &object_speed_definitions.recs[idx];
        if (candidate->kind == desc)
        {
            entry = candidate;
            break;
        }
    }
    center = entry->wrap_center;
    extent = entry->wrap_step;
    pos = velocity.x;
    result = pos - extent;
    if ((sint32)(center + extent) < pos)
        velocity.x = result;
    else
    {
        result = pos + extent;
        if (pos < (sint32)(center - extent))
            velocity.x = result;
    }
}

void AnimatedObject::ApplyObjectSpeedAction(uint32 action)
{
    FUNCTION_MARKER(0x8003FC44, "SLUS_005.78");
    sint32 count = object_speed_definitions.count;
    uint32 desc = state;
    const ObjectSpeedDefinition *entry = 0;
    sint32 idx;
    uint32 result;
    for (idx = 0; idx < count; ++idx)
    {
        const ObjectSpeedDefinition *candidate = &object_speed_definitions.recs[idx];
        if (candidate->kind == desc)
        {
            entry = candidate;
            break;
        }
    }
    result = entry->action_count;
    if (result != 0)
    {
        const ObjectSpeedAction *slot = entry->actions;
        uint32 pos = 0;
        do
        {
            uint32 found = slot->action;
            ++pos;
            if (action == found)
            {
                sint32 val = (uint32)velocity.x + slot->speed_delta;
                sint32 limit;
                velocity.x = val;
                limit = entry->max_speed;
                if (limit < val)
                    velocity.x = limit;
                val = velocity.x;
                limit = entry->min_speed;
                result = val < limit;
                if (result != 0)
                    velocity.x = limit;
                return;
            }
            result = pos < entry->action_count;
            ++slot;
        } while (result != 0);
    }
}

intptr AnimatedObject::UpdateActorTargetPursuit()
{
    FUNCTION_MARKER(0x8003FDC8, "SLUS_005.78");
    ActorBinding *state;
    ActorModelDefinition *resource_model;
    Vec3 origin;
    Vec3 target;
    uint32 distance;
    uint32 result;
    const PathDefinition *definition;
    UpdateActorStateAndTarget(origin, target, &distance);
    definition = path_definition;
    state = (ActorBinding *)(((const ActorPlacement *)definition)->state);
    ActorBinding *binding = state;
    resource_model = binding->model;
    if ((binding->flags & 2) == 0)
    {
        if (resource_model->target_range[0] < (sint32)distance || (definition->flags & 0x40) == 0)
        {
            if ((flags & 0x40) != 0)
            {
                flags = flags & 0xFFFFFFBF;
                animation = resource_model->animations[0];
                frame = -1;
                flags = flags | 0x10000;
            }
        }
        else if ((flags & 0x40) == 0)
        {
            sint32 sample_frame, prev;
            flags = flags | 0x40;
            animation = resource_model->animations[2];
            frame = -1;
            flags = flags | 0x10000;
            sample_frame = path_definition->FindHeightFrame(target, flags & 0x10, resource_model->height_offset);
            prev = path_frame;
            if (sample_frame < prev)
            {
                uint32 count = (uint16)path_definition->pos_track.frame_count;
                flags = (flags & 0xFFFFF7FF) ^ 0x10;
                path_frame = count - prev;
            }
        }
    }
    result = UpdateActorFacingAndAnimation(0, origin, resource_model);
    return result;
}

intptr AnimatedObject::InitializeActorTargetPursuit()
{
    FUNCTION_MARKER(0x8003FF3C, "SLUS_005.78");
    if (!object_operations.behaviors[13].plain_handler && !object_operations.behaviors[13].member_handler && !object_operations.behaviors[13].void_member_handler && !object_operations.behaviors[13].factory_handler)
    {
        ObjectCallback operation = {0, &AnimatedObject::UpdateActorTargetPursuit};
        object_operations.behaviors[13] = operation;
    }
    behavior = 13;
    return 13;
}

AnimatedObject *AnimatedObject::CloneObjectResource(const AnimatedObject *src)
{
    FUNCTION_MARKER(0x8004002C, "SLUS_005.78");
    uint8 *dst;
    uint32 bytes;
    SceneNode *scene_node, *child_node, *reference;
    AnimatedObject *clone;
    SpriteRecord *clone_spr;
    if (!src)
        return 0;
    dst = AllocateGameMemory(src->clone_bytes, src->clone_storage_bytes);
    if (!dst)
        return 0;
    bytes = src->clone_storage_bytes;
    CopyBytes(dst, (const uint8 *)src, bytes);
    clone = (AnimatedObject *)dst;
    if (clone->material_animation_entries != 0)
    {
        clone->material_animation_entries = (uint32 *)((uint8 *)clone + ((const uint8 *)src->material_animation_entries - (const uint8 *)src));
        MaterialAnimationList::ResetMaterialAnimationList(clone->material_animation_entries);
    }
    ObjectLinks *root_links = &clone->node.links;
    reference = root_links->first_child;
    root_links->registry_next = 0;
    clone->scale = 4096;
    if (reference != 0)
    {
        root_links->first_child = (SceneNode *)((uint8 *)clone + ((const uint8 *)reference - (const uint8 *)src));
    }
    clone_spr = clone->spr;
    if (clone_spr != 0)
    {
        clone->spr = (SpriteRecord *)((uint8 *)clone + ((const uint8 *)clone_spr - (const uint8 *)src));
    }
    scene_node = root_links->first_child;
    root_links->sibling_next = 0;
    while (scene_node != 0)
    {
        reference = scene_node->links.first_child;
        if (reference != 0)
        {
            scene_node->links.first_child = (SceneNode *)((uint8 *)clone + ((const uint8 *)reference - (const uint8 *)src));
        }
        reference = scene_node->links.sibling_next;
        if (reference != 0)
        {
            scene_node->links.sibling_next = (SceneNode *)((uint8 *)clone + ((const uint8 *)reference - (const uint8 *)src));
        }
        if (scene_node->kind == 7)
        {
            SpriteRecord *spr = (SpriteRecord *)scene_node;
            uint32 *entries = spr->animation_entries;
            if (entries != 0)
            {
                spr->animation_entries = (uint32 *)((uint8 *)clone + ((const uint8 *)entries - (const uint8 *)src));
                MaterialAnimationList::ResetMaterialAnimationList(spr->animation_entries);
            }
        }
        child_node = scene_node->links.first_child;
        while (child_node != 0)
        {
            child_node->RelocateClone((uint8 *)clone, (const uint8 *)src);
            child_node = child_node->links.sibling_next;
        }
        scene_node = scene_node->links.sibling_next;
    }
    return clone;
}

uint32 AnimatedObject::TurnObjectYaw(uint32 target, sint16 step)
{
    FUNCTION_MARKER(0x800401F0, "SLUS_005.78");
    sint32 prev = yaw;
    sint32 difference;
    target_yaw = target;
    if (prev == 4096)
    {
        yaw = target;
        return 4096;
    }
    difference = (sint16)(target - prev);
    if (difference < -2048)
        difference = (sint16)(difference + 4096);
    else if (difference > 2048)
        difference = (sint16)(difference - 4096);
    if (difference < step && difference > -step)
        yaw = target;
    else
        yaw = (uint16)yaw + (difference < 0 ? -step : step);
    target = (uint16)yaw & 4095;
    yaw = target;
    return target;
}

uint32 AnimatedObject::TurnObjectPitch(uint32 target, sint16 step)
{
    FUNCTION_MARKER(0x800402BC, "SLUS_005.78");
    sint32 prev = pitch;
    sint32 difference;
    if (prev == 4096)
    {
        pitch = target;
        return 4096;
    }
    difference = (sint16)(target - prev);
    if (difference < -2048)
        difference = (sint16)(difference + 4096);
    else if (difference > 2048)
        difference = (sint16)(difference - 4096);
    if (difference < step && difference > -step)
        pitch = target;
    else
        pitch = (uint16)pitch + (difference < 0 ? -step : step);
    target = (uint16)pitch & 4095;
    pitch = target;
    return target;
}

uint32 AnimatedObject::TurnObjectFacing(const Vec3 &dir_vector, sint16 step)
{
    const PathDefinition *definition;
    FUNCTION_MARKER(0x80040384, "SLUS_005.78");
    Quat axes;
    uint32 target, val, kind, x, y, z, sq, fast_turn, result;
    sint32 old_yaw = yaw;
    sint32 old_pitch = pitch;
    sint32 cur_angle, delta, yaw_step, pitch_step, len, vertical;
    definition = path_definition;
    if (definition != 0 && (definition->flags & 8) != 0 && ((const ActorBinding *)(((const ActorPlacement *)definition)->state))->rotation_track != 0)
    {
        target = ComputeObjectFacing(dir_vector, &axes);
        cur_angle = yaw;
        target_yaw = target;
        if (cur_angle == 4096)
            yaw = target;
        else
        {
            delta = (sint16)(target - cur_angle);
            if (delta < -2048)
                delta += 4096;
            else if (delta > 2048)
                delta -= 4096;
            delta = (sint16)delta;
            if (delta < step && -step < delta)
                yaw = target;
            else if (delta < 0)
                yaw = (uint16)yaw - (uint32)step;
            else
                yaw = (uint16)yaw + (uint32)step;
            yaw = (uint16)yaw & 0xFFF;
        }
        Quat::ComposeHeadingQuat(yaw, &axes, &rotation);
        goto finish;
    }
    if ((flags & 0x2000) != 0 || ((uint32)dir_vector.x + 32 < 65 && (uint32)dir_vector.z + 32 < 65))
        val = (uint16)target_yaw;
    else
        val = Atan2Angle((-(uint32)dir_vector.z), (-(uint32)dir_vector.x));
    fast_turn = 0;
    if (contact_kind == 1)
    {
        kind = (uint16)(character_pad.motion.form_flags >> 16) & 0xF;
        fast_turn = kind - 5 < 2 || kind == 7 || kind - 10 < 2;
    }
    cur_angle = yaw;
    if (fast_turn != 0)
    {
        target = (val + ((flags & 0x10) != 0 ? 1024 : (uint32)-1024)) & 0xFFF;
        yaw_step = 4096;
    }
    else
    {
        if ((sint16)val == cur_angle)
            goto pitch;
        target = val & 0xFFF;
        yaw_step = step;
    }
    target_yaw = target;
    if (cur_angle == 4096)
        yaw = target;
    else
    {
        delta = (sint16)(target - cur_angle);
        if (delta < -2048)
            delta += 4096;
        else if (delta > 2048)
            delta -= 4096;
        delta = (sint16)delta;
        if (delta < yaw_step && -yaw_step < delta)
            yaw = target;
        else if (delta < 0)
            yaw = (uint16)yaw - (uint32)yaw_step;
        else
            yaw = (uint16)yaw + (uint32)yaw_step;
        yaw = (uint16)yaw & 0xFFF;
    }
    orientation[2] = ((uint16)yaw + 2048) & 0xFFF;
pitch:
    if (pitch_scale == 4096)
        orientation[1] = 0;
    else
    {
        x = dir_vector.x >> 6;
        y = dir_vector.y >> 6;
        z = dir_vector.z >> 6;
        sq = x * x + y * y + z * z;
        if ((sint32)sq >= 0)
            len = SquareRoot12(sq);
        else
            len = SquareRoot12((sq >> 2)) << 1;
        vertical = dir_vector.y;
        if (vertical < 0)
            vertical = -(uint32)vertical;
        if (len < vertical)
            len = vertical;
        val = AsinAngle(dir_vector.y, len);
        target = (sint16)val == 4096 ? 0 : (-val) & 0xFFF;
        cur_angle = pitch;
        if ((sint16)target != cur_angle)
        {
            pitch_step = step >> 1;
            target &= 0xFFF;
            if (cur_angle == 4096)
                pitch = target;
            else
            {
                delta = (sint16)(target - cur_angle);
                if (delta < -2048)
                    delta += 4096;
                else if (delta > 2048)
                    delta -= 4096;
                delta = (sint16)delta;
                if (delta < pitch_step && -pitch_step < delta)
                    pitch = target;
                else if (delta < 0)
                    pitch = (uint16)pitch - (uint32)pitch_step;
                else
                    pitch = (uint16)pitch + (uint32)pitch_step;
                pitch = (uint16)pitch & 0xFFF;
            }
            orientation[1] = (uint16)pitch & 0xFFF;
        }
    }
    orientation[0] = 0x7FFF;
finish:
    val = flags;
    cur_angle = yaw;
    flags = val | 1;
    result = cur_angle == old_yaw && pitch == old_pitch;
    return result;
}

void AnimatedObject::ComputeColResponse(const SceneNode *other_node, uint32 mode, uint32 val)
{
    FUNCTION_MARKER(0x80040A1C, "SLUS_005.78");
    const SceneNode *other = other_node;
    Vec3 delta, n, other_velocity;
    MATRIX rotation = {0};
    CollisionTransform desc = {0};
    const MeshFace *polygon;
    Vec3 *response_velocity;
    AnimatedPathState *path_state;
    uint32 idx, x, y, z, component0, component1, sum, mag, len, factor;
    sint32 quotient;
    memset((uint8 *)&rotation + sizeof(rotation.m), 0, offsetof(MATRIX, t) - sizeof(rotation.m));
    if (val == 0xFFFFFFFF)
    {
        x = ((const AnimatedObject *)other)->velocity.x;
        y = ((const AnimatedObject *)other)->velocity.y;
        z = ((const AnimatedObject *)other)->velocity.z;
        other_velocity.x = x;
        other_velocity.y = y;
        other_velocity.z = z;
        component0 = mul12(velocity.x, (sint32)mode);
        delta.x = component0;
        component0 = mul12(velocity.y, (sint32)mode);
        delta.y = component0;
        component0 = mul12(velocity.z, (sint32)mode);
        delta.z = component0;
        delta.x = delta.x + (uint32)spheres[0].pos.x;
        delta.y = delta.y + (uint32)spheres[0].pos.y;
        delta.z = delta.z + (uint32)spheres[0].pos.z;
        component0 = mul12(other_velocity.x, (sint32)mode);
        n.x = component0;
        component0 = mul12(other_velocity.y, (sint32)mode);
        n.y = component0;
        component0 = mul12(other_velocity.z, (sint32)mode);
        n.z = component0;
        n.x = n.x + (uint32)((const AnimatedObject *)other)->spheres[0].pos.x;
        n.y = n.y + (uint32)((const AnimatedObject *)other)->spheres[0].pos.y;
        n.z = n.z + (uint32)((const AnimatedObject *)other)->spheres[0].pos.z;
        x = delta.x - (uint32)n.x;
        y = delta.y - (uint32)n.y;
        z = delta.z - (uint32)n.z;
        n.x = x;
        n.y = y;
        n.z = z;
        while (x + 26754 > 0xD103 || y + 26754 > 0xD103 || z + 26754 > 0xD103)
        {
            x = (sint32)x >> 1;
            y = (sint32)y >> 1;
            z = (sint32)z >> 1;
            n.x = x;
            n.y = y;
            n.z = z;
        }
        {
            VECTOR sdk_input = {n.x, n.y, n.z, 0};
            VECTOR sdk_output;
            VectorNormal(&sdk_input, &sdk_output);
            n.x = sdk_output.vx;
            n.y = sdk_output.vy;
            n.z = sdk_output.vz;
        }
    }
    else
    {
        polygon = SceneNode::SelectColFace(other_node, val & 0xFFFF8FFF, &desc);

        other_velocity = desc.motion;
        for (idx = 0; idx < 9; ++idx)
            rotation.m[idx / 3][idx % 3] = desc.rotation[(idx % 3) * 3 + idx / 3];
        PushMatrix();
        {
            SetRotMatrix(&rotation);
        }
        gte_ldv0(&polygon->n);
        xport_gte_mvmva(0x486012);
        n.x = xport_gte_read_data(25);
        n.y = xport_gte_read_data(26);
        n.z = xport_gte_read_data(27);
        PopMatrix();
    }
    delta.x = velocity.x - (uint32)other_velocity.x;
    delta.y = velocity.y - (uint32)other_velocity.y;
    delta.z = velocity.z - (uint32)other_velocity.z;
    if (val == 0xFFFFFFFF)
    {
        sum = 0;
        sum += mul12(delta.x, n.x)
            + mul12(delta.y, n.y)
            + mul12(delta.z, n.z);
        mag = -sum;
        if (mag != 0)
        {
            x = delta.x >> 6;
            y = delta.y >> 6;
            z = delta.z >> 6;
            sum = x * x + y * y + z * z;
            if ((sint32)sum < 0)
                len = SquareRoot12((sum >> 2)) << 1;
            else
                len = SquareRoot12(sum);
            if (len != 0)
            {
                quotient = 0x40000000 / (sint32)len;
                factor = ((sint64)(sint32)mag * quotient) >> 18;
                fixed_math_result = factor;
                factor = 4096 - factor;
                component0 = mul12(delta.x, (sint32)factor);
                delta.x = component0;
                component0 = mul12(delta.y, (sint32)factor);
                delta.y = component0;
                component0 = mul12(delta.z, (sint32)factor);
                delta.z = component0;
            }
        }
    }
    sum = 0;
    sum += mul12(delta.x, n.x)
            + mul12(delta.y, n.y)
            + mul12(delta.z, n.z);
    path_state = (AnimatedPathState *)(((const ActorPlacement *)path_definition)->state);
    response_velocity = &path_state->dir;
    factor = -(sum << 1);
    component0 = mul12(n.x, (sint32)factor);
    response_velocity->x = component0;
    component0 = mul12(n.y, (sint32)factor);
    response_velocity->y = component0;
    component0 = mul12(n.z, (sint32)factor);
    response_velocity->z = component0;
    component0 = response_velocity->x;
    component1 = delta.x;
    response_velocity->x = component0 + component1;
    component0 = response_velocity->y;
    component1 = delta.y;
    response_velocity->y = component0 + component1;
    component0 = response_velocity->z;
    component1 = delta.z;
    response_velocity->z = component0 + component1;
    component0 = response_velocity->x;
    component1 = other_velocity.x;
    response_velocity->x = component0 + component1;
    component0 = response_velocity->y;
    component1 = other_velocity.y;
    response_velocity->y = component0 + component1;
    component0 = response_velocity->z;
    component1 = other_velocity.z;
    response_velocity->z = component0 + component1;
}

uint32 AnimatedObject::HandleModelCol(SceneNode *other_node, CollisionContact *contact_data)
{
    FUNCTION_MARKER(0x80041060, "SLUS_005.78");
    SceneNode *other = other_node;
    const AnimatedModelDefinition *rule;
    Vec3 *response_velocity;
    AnimatedModelDefinition *const *cursor = animated_models.definitions;
    uint32 count = animated_models.count;
    const AnimatedModelDefinition *entry = 0;
    uint32 kind_id = state, cur_flags, mask, mode, val;
    AnimatedPathState *path_state;
    uint32 x;
    uint32 y;
    uint32 z;
    uint32 distance;
    uint32 speed;
    uint32 mag;
    sint32 scale;

    sint32 kind, quotient;
    while (count != 0)
    {
        entry = *cursor;
        ++cursor;
        rule = entry;
        if (rule->kind_id == kind_id)
            break;
        --count;
    }
    if (count == 0)
        entry = 0;
    rule = entry;
    if (contact_data == 0)
    {
        if (other_node == 0)
            return (rule->flags & 0x180) != 0;
        if (other->kind != 0)
            return 0;
        kind = ((const AnimatedObject *)other)->contact_kind;
        switch (kind)
        {
            case 1:
                mask = 0x18;
                break;
            case 2:
                mask = 0x60;
                break;
            case 3:
                mask = 0x600;
                break;
            case 4:
                mask = 0x1800;
                break;
            default:
                return 0;
        }
        return (rule->flags & mask) != 0;
    }
    cur_flags = flags & 0xFF3FFFFF;
    flags = cur_flags;
    val = contact_data->flags;
    mode = contact_data->fraction;
    if (val == 0x7FFFF000)
    {
        mask = (rule->flags & 0x80) != 0 ? 0x400000 : 0x800000;
        flags = cur_flags | mask;
        val = contact_data->selection;
    }
    else
    {
        kind = ((const AnimatedObject *)other)->contact_kind;
        switch (kind)
        {
            case 1:
                mask = 8;
                break;
            case 2:
                mask = 0x20;
                break;
            case 3:
                mask = 0x200;
                break;
            case 4:
                mask = 0x800;
                break;
            default:
                return 0;
        }
        cur_flags |= (rule->flags & mask) != 0 ? 0x400000 : 0x800000;
        flags = cur_flags;
        val = 0xFFFFFFFF;
    }
    cur_flags = flags;
    if ((cur_flags & 0x400000) == 0)
        return 1;
    if (mode != 0 || val != 0xFFFFFFFF)
    {
        ComputeColResponse(other_node, mode, val);
        if (mode != 0 || (flags & 0x1000000) == 0)
            return 1;
        return 0;
    }
    if ((cur_flags & 0x1000000) == 0)
        return 0;
    path_state = (AnimatedPathState *)(((const ActorPlacement *)path_definition)->state);
    response_velocity = &path_state->dir;
    x = spheres[0].pos.x - (uint32)((const AnimatedObject *)other)->spheres[0].pos.x;
    response_velocity->x = x;
    y = spheres[0].pos.y - (uint32)((const AnimatedObject *)other)->spheres[0].pos.y;
    response_velocity->y = y;
    z = spheres[0].pos.z - (uint32)((const AnimatedObject *)other)->spheres[0].pos.z;
    response_velocity->z = z;
    x = response_velocity->x >> 6;
    y = response_velocity->y >> 6;
    distance = x * x + y * y;
    x = ((const AnimatedObject *)other)->velocity.x >> 6;
    y = ((const AnimatedObject *)other)->velocity.y >> 6;
    speed = x * x + y * y;
    x = ((const AnimatedObject *)other)->velocity.z >> 6;
    z = (sint32)z >> 6;
    speed += x * x;
    distance += z * z;
    if (distance == 0)
        xport_mips_break(7);
    quotient = 0x40000000 / (sint32)distance;
    mag = ((sint64)(sint32)speed * quotient) >> 18;
    fixed_math_result = mag;
    if ((sint32)mag < 0)
        scale = SquareRoot12((mag >> 2)) << 1;
    else
        scale = SquareRoot12(mag);
    if (scale < 64)
        scale = 64;
    val = mul12(response_velocity->x, scale);
    response_velocity->x = val;
    val = mul12(response_velocity->y, scale);
    response_velocity->y = val;
    val = mul12(response_velocity->z, scale);
    response_velocity->z = val;
    return 0;
}

void AnimatedObject::CapturePathVelocity()
{
    FUNCTION_MARKER(0x800414D0, "SLUS_005.78");
    AnimatedPathState *path_state = (AnimatedPathState *)(((const ActorPlacement *)path_definition)->state);

    VECTOR motion;

    uint32 x, y, z, sum, result;
    x = velocity.x;
    y = velocity.y;
    z = velocity.z;
    motion.vx = x;
    motion.vy = y;
    motion.vz = z;
    path_state->speed[1] = y;
    x = motion.vx >> 6;
    z = motion.vz >> 6;
    sum = x * x + z * z;
    motion.vy = 0;
    if ((sint32)sum < 0)
        result = SquareRoot12((sum >> 2)) << 1;
    else
        result = SquareRoot12(sum);
    path_state->speed[0] = result;
    if ((flags & 0x2000) != 0)
    {
        x = motion.vx;
        y = motion.vy;
        z = motion.vz;
        while (x + 26754 > 0xD103 || y + 26754 > 0xD103 || z + 26754 > 0xD103)
        {
            x = (sint32)x >> 1;
            y = (sint32)y >> 1;
            z = (sint32)z >> 1;
            motion.vx = x;
            motion.vy = y;
            motion.vz = z;
        }
        VectorNormal(&motion, &motion);
        x = motion.vx;
        y = motion.vy;
        z = motion.vz;
        path_state->dir.x = x;
        path_state->dir.y = y;
        path_state->dir.z = z;
    }
    flags = flags | 0x400;
}

intptr AnimatedObject::UpdateAnimatedModelPathMotion()
{
    FUNCTION_MARKER(0x8004165C, "SLUS_005.78");
    AnimatedPathState *path_state = (AnimatedPathState *)(((const ActorPlacement *)path_definition)->state);
    uint32 *speed = path_state->speed;
    AnimatedModelDefinition *const *cursor = animated_models.definitions;
    uint32 count = animated_models.count;
    const AnimatedModelDefinition *definition = NULL;
    AnimatedObject *owner;
    SceneNode *child_reference;
    uint32 kind = state, cur_flags, angle, reverse, val;
    uint32 idx;
    uint32 x;
    uint32 y;
    uint32 z;
    uint32 sum;
    uint32 len;
    uint32 scaled_len;
    uint32 relative_y;
    sint32 gravity;
    sint32 factor;
    uint32 result;
    sint32 heading, quotient;
    while (count != 0)
    {
        definition = *cursor;
        ++cursor;
        if (definition->kind_id == kind)
            break;
        --count;
    }
    if (count == 0)
        definition = NULL;
    cur_flags = flags;
    child_reference = node.links.first_child;
    owner = child_reference != 0 ? (AnimatedObject *)child_reference : NULL;
    if ((cur_flags & 0x400000) != 0)
    {
        angle = (uint16)target_yaw;
        if ((cur_flags & 0x10) != 0)
            angle += 2048;
        reverse = 0;
        heading = Atan2Angle(path_state->dir.z, path_state->dir.x);
        if (heading != 4096)
            reverse = (((uint32)(sint16)angle - heading + 512) & 0xFFF) >= 1025;
        val = mul12(path_state->dir.x, (sint32)definition->col_scale);
        path_state->dir.x = val;
        val = mul12(path_state->dir.y, (sint32)definition->col_scale);
        path_state->dir.y = val;
        val = mul12(path_state->dir.z, (sint32)definition->col_scale);
        path_state->dir.z = val;
        x = path_state->dir.x;
        y = path_state->dir.y;
        z = path_state->dir.z;
        velocity = path_state->dir;
        if ((flags & 0x400) != 0)
        {
            CapturePathVelocity();
            speed[1] = speed[1] + definition->gravity;
        }
        else
        {
            {
                uint32 x = velocity.x >> 6;
                uint32 y = velocity.y >> 6;
                uint32 z = velocity.z >> 6;
                uint32 sum = x * x + y * y + z * z;
                if ((sint32)sum < 0)
                    speed[0] = SquareRoot12((sum >> 2)) << 1;
                else
                    speed[0] = SquareRoot12(sum);
            }
            speed[1] = 0;
        }
        if (reverse != 0)
        {
            speed[0] = -speed[0];
            x = path_state->dir.x;
            y = path_state->dir.y;
            z = path_state->dir.z;
            path_state->dir.x = -x;
            path_state->dir.y = -y;
            path_state->dir.z = -z;
            {
                AnimatedPathState *path_state = (AnimatedPathState *)(((const ActorPlacement *)path_definition)->state);
                uint32 cur_flags0;
                const PathDefinition *parent;
                sint32 count, frame;
                path_state->speed[0] = -path_state->speed[0];
                cur_flags0 = flags;
                parent = path_definition;
                flags = cur_flags0 ^ 0x10;
                count = parent->pos_track.frame_count;
                frame = count - path_frame;
                if (frame >= count)
                    frame = count - 1;
                else if (frame < 0)
                    frame = 0;
                path_frame = frame;
            }
            owner->pitch_scale = -(uint16)owner->pitch_scale;
        }
        if ((sint32)speed[0] < 256)
            speed[0] = 0;
        flags = (flags & 0xFFBFFFFF) | 0x1000000;
    }
    cur_flags = flags;
    if ((cur_flags & 0x400) != 0)
        speed[1] = speed[1] - definition->gravity;
    else
    {
        angle = (uint16)owner->pitch_scale;
        if ((cur_flags & 0x4000) != 0)
        {
            flags = cur_flags & 0xFFFFBFFF;
            {
                uint32 x = (sint32)speed[0] >> 6;
                uint32 y = (sint32)speed[1] >> 6;
                uint32 z = (sint32)speed[2] >> 6;
                uint32 sum = x * x + y * y + z * z;
                if ((sint32)sum < 0)
                    len = SquareRoot12((sum >> 2)) << 1;
                else
                    len = SquareRoot12(sum);
            }
            scaled_len = mul12((sint32)len, (sint32)definition->col_scale);
            if (scaled_len != 0)
            {
                heading = AsinAngle(speed[1], len);
                val = (angle - heading) & 0xFFF;
                x = mul12((SinQ14((1024 - val)) >> 2), (sint32)scaled_len);
                velocity.x = x;
                y = mul12((SinQ14(val) >> 2), (sint32)scaled_len);
                x = velocity.x;
                velocity.y = y;
                velocity.z = 0;
                speed[0] = x;
            }
            speed[1] = 0;
        }
        if ((sint32)speed[0] < 0)
        {
            {
                AnimatedPathState *path_state = (AnimatedPathState *)(((const ActorPlacement *)path_definition)->state);
                uint32 cur_flags1;
                const PathDefinition *parent;
                sint32 count, frame;
                path_state->speed[0] = -path_state->speed[0];
                cur_flags1 = flags;
                parent = path_definition;
                flags = cur_flags1 ^ 0x10;
                count = parent->pos_track.frame_count;
                frame = count - path_frame;
                if (frame >= count)
                    frame = count - 1;
                else if (frame < 0)
                    frame = 0;
                path_frame = frame;
            }
            angle = -angle;
        }
        x = velocity.x;
        y = velocity.y;
        z = velocity.z;
        relative_y = y - definition->gravity;
        reverse = 0;
        if ((definition->flags & 4) != 0)
        {
            x = velocity.x >> 6;
            y = velocity.y >> 6;
            z = velocity.z >> 6;
            sum = x * x + y * y + z * z;
            if ((sint32)sum >= 89)
            {
                x = velocity.x >> 6;
                y = (sint32)relative_y >> 6;
                z = velocity.z >> 6;
                sum = x * x + y * y + z * z;
                if ((sint32)sum < 0)
                    len = SquareRoot12((sum >> 2)) << 1;
                else
                    len = SquareRoot12(sum);
                heading = AsinAngle(relative_y, len);
                reverse = (((uint32)heading - (sint16)angle) & 0xFFF) < 2048;
            }
        }
        if (reverse != 0)
            CapturePathVelocity();
        else if (path_frame > 0 || (flags & 0x1000000) == 0)
        {
            gravity = definition->gravity;
            val = mul12(gravity, (SinQ14(angle) >> 2));
            val = speed[0] - val;
            speed[0] = val;
            speed[0] = mul12((sint32)definition->damping, (sint32)val);
        }
    }
    if ((sint32)speed[0] < 0)
    {
        AnimatedPathState *path_state = (AnimatedPathState *)(((const ActorPlacement *)path_definition)->state);
        uint32 cur_flags2;
        const PathDefinition *parent;
        sint32 count, frame;
        path_state->speed[0] = -path_state->speed[0];
        cur_flags2 = flags;
        parent = path_definition;
        flags = cur_flags2 ^ 0x10;
        count = parent->pos_track.frame_count;
        frame = count - path_frame;
        if (frame >= count)
            frame = count - 1;
        else if (frame < 0)
            frame = 0;
        path_frame = frame;
    }
    {
        uint32 x = (sint32)speed[0] >> 6;
        uint32 y = (sint32)speed[1] >> 6;
        uint32 z = (sint32)speed[2] >> 6;
        uint32 sum = x * x + y * y + z * z;
        if ((sint32)sum < 0)
            len = SquareRoot12((sum >> 2)) << 1;
        else
            len = SquareRoot12(sum);
    }
    val = definition->speed_limit;
    if ((sint32)val < (sint32)len)
    {
        if (len == 0)
        {
            trap(7);
        }
        quotient = 0x40000000 / (sint32)len;
        factor = ((sint64)(sint32)val * quotient) >> 18;
        fixed_math_result = factor;
        for (idx = 0; idx < 3; ++idx)
            speed[idx] = mul12((sint32)speed[idx], factor);
    }
    flags = flags | 0x40;
    ComputePathMotion(speed[0], speed[1], &path_state->dir);
    result = flags & 0xFFFFFFBF;
    flags = result;
    return result;
}

uint32 AnimatedObject::ApplyModelTravelRotation(SceneNode *object, uint32 distance)
{
    FUNCTION_MARKER(0x80041EDC, "SLUS_005.78");
    SceneNode *root = object;
    SceneNode *node = root->links.first_child;
    uint32 changed = 0;
    sint32 factor;
    uint32 result;
    sint32 denominator, quotient, angle;

    while (node != 0)
    {
        if (node->kind == 6 && root->kind == 5)
        {
            denominator = mul12((sint32)((const SphereNode *)node)->radius, 25735);
            if (denominator == 0)
            {
                trap(7);
            }
            quotient = 0x40000000 / denominator;
            factor = ((sint64)(sint32)distance * quotient) >> 18;
            angle = factor >> 1;
            fixed_math_result = factor;
            ((MeshPartNode *)root)->animation_binding.tracks[1] = -1;
            result = SinQ14((1024 - (uint32)angle));
            ((MeshPartNode *)root)->rotation[0] = result;
            result = SinQ14(angle);
            changed = 1;
            ((MeshPartNode *)root)->rotation[1] = result;
            ((MeshPartNode *)root)->rotation[3] = 0;
            ((MeshPartNode *)root)->rotation[2] = 0;
        }
        result = AnimatedObject::ApplyModelTravelRotation(node, distance);
        node = node->links.sibling_next;
        changed |= result;
    }
    return (sint8)changed;
}

void AnimatedObject::ApplyEvents()
{
    FUNCTION_MARKER(0x80042024, "SLUS_005.78");
    uint32 result;
    uint32 remaining = scene_events.count;
    const uint32 *events;
    uint32 count, kind, object_id, entry, action, subtype, impulse, cur_flags;
    uint32 *speed;
    const AnimatedModelDefinition *definition = NULL;
    if (remaining == 0)
        return;
    count = animated_models.count;
    AnimatedModelDefinition *const *definitions = animated_models.definitions;
    kind = state;
    while (count != 0)
    {
        definition = *definitions;
        if (definition->kind_id == kind)
            break;
        ++definitions;
        --count;
    }
    if (count == 0)
        definition = NULL;
    object_id = event_word & 0xFFF;
    events = scene_events.entries;
    speed = ((AnimatedPathState *)(((const ActorPlacement *)path_definition)->state))->speed;
    do
    {
        entry = *events;
        ++events;
        result = entry & 0xFFF;
        if (object_id == result)
        {
            action = ((entry >> 12) & 0x1F) + 39;
            subtype = (entry >> 17) & 0xFF;
            if ((node.flags & 1) != 0 && action != 43)
                action = 42;
            result = action - 40 < 15;
            if (result != 0)
            {
                switch (action)
                {
                    case 40:
                        result = movement_mode;
                        if (result != 3)
                        {
                            result = definition->initial_speed;
                            speed[0] = result;
                            goto moving;
                        }
                        break;
                    case 42:
                    case 53:
                        result = flags | 0x4000;
                        cur_flags = node.flags | 1;
                        flags = result;
                        node.flags = cur_flags;
                        goto stop_motion;
                    case 44:
                    stop_motion:
                        result = movement_mode;
                        if (result != 0)
                        {
                            velocity.z = 0;
                            velocity.y = 0;
                            velocity.x = 0;
                            speed[2] = 0;
                            speed[1] = 0;
                            speed[0] = 0;
                            movement_mode = 0;
                        }
                        break;
                    case 46:
                        impulse = definition->impulse;

                        if (impulse == 0)
                            break;
                        if (subtype == 11)
                        {
                            cur_flags = flags;
                            result = cur_flags & 0x400;
                            if ((cur_flags & 0x4000) == 0)
                            {
                                if (result == 0)
                                    CapturePathVelocity();
                                result = speed[1] + definition->impulse;
                                speed[1] = result;
                            }
                        }
                        else
                        {
                            result = (flags & 0x10) != 0 ? speed[0] + impulse : speed[0] - impulse;
                            speed[0] = result;
                        }
                        goto moving;
                    case 48:
                        impulse = definition->impulse;

                        if (impulse == 0)
                            break;
                        if (subtype == 10)
                        {
                            result = flags & 0x400;
                            if (result != 0)
                            {
                                result = speed[1] - impulse;
                                speed[1] = result;
                            }
                        }
                        else
                        {
                            result = (flags & 0x10) != 0 ? speed[0] - impulse : speed[0] + impulse;
                            speed[0] = result;
                        }
                    moving:
                        movement_mode = 3;
                        break;
                    case 51:
                        if ((node.flags & 1) == 0)
                        {
                            result = flags | 0x4000;
                            cur_flags = node.flags | 1;
                            flags = result;
                            node.flags = cur_flags;
                            break;
                        }
                        goto activate;
                    case 43:
                    case 54:
                    activate:
                        flags = flags | 0x80000;
                        result = node.flags;
                        cur_flags = flags;
                        result &= 0xFE;
                        cur_flags &= 0xFFDFFFFF;
                        node.flags = result;
                        flags = cur_flags;
                        break;
                }
            }
        }
        --remaining;
    } while (remaining != 0);
}

void AnimatedObject::UpdateAnimatedModelOrientation(const AnimatedModelDefinition *definition, AnimatedObject *object)
{
    FUNCTION_MARKER(0x80042320, "SLUS_005.78");
    Vec3 sample_pos, delta;
    AnimatedObject *owner;
    SceneNode *sibling;
    uint32 dir, sample_frame, x, y, z, len, val, result;
    sint32 frame, heading, half_angle, complement;
    frame = object->path_frame;
    dir = object->flags & 0x10;
    owner = (AnimatedObject *)object->node.links.first_child;
    if (frame > 0)
        --frame;
    sample_frame = frame;
    object->path_definition->SamplePos((sint16)sample_frame, sample_pos, dir);
    do
    {
        ++sample_frame;
        frame = (sint16)sample_frame;
        object->path_definition->SamplePos(frame, delta, dir);
        x = sample_pos.x - (uint32)delta.x;
        y = sample_pos.y - (uint32)delta.y;
        z = sample_pos.z - (uint32)delta.z;
        delta.x = x;
        delta.y = y;
        delta.z = z;
        heading = Atan2Angle(z, x);
        object->target_yaw = heading;
        if (heading != 4096)
            break;
    } while (frame < object->path_definition->pos_track.frame_count - 1);
    if (dir == 0)
    {
        if (heading == 4096)
            goto update_pitch;
        object->target_yaw = ((uint32)heading + 2048) & 0xFFF;
    }
    heading = object->target_yaw;
    if (heading != 4096 && object->yaw != heading)
    {
        half_angle = (sint32)((2048 - (uint32)heading) << 16) >> 17;
        object->yaw = heading;
        result = SinQ14((1024 - (uint32)half_angle));
        object->orientation[0] = result;
        object->orientation[3] = 0;
        object->orientation[1] = 0;
        result = SinQ14(half_angle);
        object->orientation[2] = result;
    }
update_pitch:
    if ((object->flags & 0x400) == 0)
    {
        if ((uint32)delta.y + 32 < 65)
            owner->pitch_scale = 0;
        else
        {
            x = delta.x >> 6;
            y = delta.y >> 6;
            z = delta.z >> 6;
            val = x * x + y * y + z * z;
            if ((sint32)val < 0)
                len = SquareRoot12((val >> 2)) << 1;
            else
                len = SquareRoot12(val);
            result = -AsinAngle(delta.y, len);
            owner->pitch_scale = result & 0xFFF;
        }
    }
    result = definition->flags & 2;
    if (result != 0)
    {
        val = (uint16)owner->pitch_scale;
        if ((object->flags & 0x10) == 0)
            val = -val;
        half_angle = (sint32)(val << 16) >> 17;
        complement = 1024 - (uint32)half_angle;
        result = SinQ14(complement);
        owner->orientation[0] = result;
        result = SinQ14(half_angle);
        owner->orientation[1] = result;
        owner->orientation[3] = 0;
        owner->orientation[2] = 0;
        sibling = owner->node.links.sibling_next;
        if (sibling != 0)
        {
            owner = (AnimatedObject *)sibling;
            result = SinQ14(complement);
            owner->orientation[0] = result;
            result = SinQ14(half_angle);
            owner->orientation[1] = result;
            owner->orientation[3] = 0;
            owner->orientation[2] = 0;
        }
    }
}

intptr AnimatedObject::UpdateAnimatedModelState()
{
    const AnimationHeader *active_animation;
    FUNCTION_MARKER(0x8004264C, "SLUS_005.78");
    Vec3 vector;
    Vec3 motion;
    Vec3 floor;
    Vec3 input;
    Vec3 world_input;
    Vec3 inverse_input;
    Vec3 event_input;
    Vec3 sample_input;
    Vec3 final_input;
    MATRIX rotation, inverse_rotation, final_rotation;
    AnimatedModelDefinition *const *cursor;
    AnimatedPathState *path_state;
    uint32 *speed;
    AnimatedObject *owner;
    SceneNode *child_reference;
    uint32 count, kind_id, initial_kind;
    const AnimatedModelDefinition *definition = 0;
    uint32 animation_idx;
    uint32 cur_flags, mode, val, x, y, z, result;
    const PathDefinition *parent;
    const SoundBankStorage *sound_bank;
    const SoundSample *sound, *prev;
    SceneNode *scene_node;
    uint32 len;
    uint32 option = 0;
    uint32 volume = 4096;
    sint32 base;
    uint32 factor;
    uint32 distance;
    uint32 changed;
    sint32 variant, frame, pitch, denominator, quotient;
    variant = behavior_suspended;
    if (variant != 0)
    {
        const ObjectCallback operation = object_operations.variants[variant];
        return (this->*operation.member_handler)();
    }
    initial_kind = (uint8)movement_mode;
    child_reference = node.links.first_child;
    owner = child_reference != 0 ? (AnimatedObject *)child_reference : NULL;
    path_state = (AnimatedPathState *)(((const ActorPlacement *)path_definition)->state);
    speed = path_state->speed;
    count = animated_models.count;
    cursor = animated_models.definitions;
    motion.z = 0;
    motion.y = 0;
    motion.x = 0;
    kind_id = state;
    while (count != 0)
    {
        definition = *cursor;
        ++cursor;
        if (definition->kind_id == kind_id)
            break;
        --count;
    }
    if (count == 0)
        definition = 0;
    if (movement_mode == 3)
    {
        if (path_motion_scale != 0)
            flags = flags & 0xFEFFFFFF;
        cur_flags = flags;
        if ((cur_flags & 0x4000) != 0)
            flags = cur_flags & 0xFFFFFBFF;
        if ((sint32)path_motion_scale < 4096)
        {
            if ((flags & 0x800000) != 0)
            {
                mode = definition->flags >> 13;
                if (mode - 4 < 2)
                {
                    if (mode == 5)
                        ticks = -2;
                    path_motion_scale = 4096;
                }
                else
                    ticks = 0;
                flags = flags & 0xFF7FFFFF;
            }
            val = (sint32)(3686 * path_motion_scale) >> 12;
            path_motion_scale = val;
            val = mul12(velocity.x, (sint32)path_motion_scale);
            vector.x = val;
            val = mul12(velocity.y, (sint32)path_motion_scale);
            vector.y = val;
            val = mul12(velocity.z, (sint32)path_motion_scale);
            vector.z = val;
        }
        else
        {
            x = velocity.x;
            y = velocity.y;
            z = velocity.z;
            vector = velocity;
        }
        x = vector.x;
        y = vector.y;
        z = vector.z;
        motion = vector;
        if ((flags & 0x2000) != 0)
        {
            frame = ticks;
            if (frame != 0)
                ticks = frame - 1;
        }
        else
        {
            parent = path_definition;
            if (parent != 0 && parent->transform != 0)
            {
                {
                    MATRIX *matrix;
                    uint32 idx;
                    for (idx = 0; idx < 9; ++idx)
                    {
                        matrix = path_definition->transform != 0 ? &path_definition->transform->matrix : NULL;
                        rotation.m[idx / 3][idx % 3] = matrix->m[idx % 3][idx / 3];
                    }
                }
                x = vector.x;
                y = vector.y;
                z = vector.z;
                input = vector;
                {
                    uint32 idx;
                    PushMatrix();
                    SetRotMatrix(&rotation);
                    idx = 4;
                    {
                        VECTOR rotation_input = {input.x, input.y, input.z, 0};
                        VECTOR rotation_output;
                        ApplyRotMatrixLV(&rotation_input, &rotation_output);
                        vector.x = rotation_output.vx;
                        vector.y = rotation_output.vy;
                        vector.z = rotation_output.vz;
                    }
                    PopMatrix();
                }
            }
        }
        pos.x = pos.x + (uint32)vector.x;
        pos.y = pos.y + (uint32)vector.y;
        pos.z = pos.z + (uint32)vector.z;
        if ((sint32)path_motion_scale < 4096 && (flags & 0x2000) == 0)
        {
            {
                const PathDefinition *parent = path_definition;
                MATRIX *matrix;
                uint32 idx, x, y, z;
                if (parent != 0 && parent->transform != 0)
                {
                    x = pos.x;
                    y = pos.y;
                    z = pos.z;
                    world_input = pos;
                    PushMatrix();
                    matrix = path_definition->transform != 0 ? &path_definition->transform->matrix : NULL;
                    SetRotMatrix(matrix);
                    idx = 5;

                    {
                        VECTOR rotation_input = {world_input.x, world_input.y, world_input.z, 0};
                        VECTOR rotation_output;
                        ApplyRotMatrixLV(&rotation_input, &rotation_output);
                        vector.x = rotation_output.vx;
                        vector.y = rotation_output.vy;
                        vector.z = rotation_output.vz;
                    }
                    PopMatrix();
                    matrix = path_definition->transform != 0 ? &path_definition->transform->matrix : NULL;
                    vector.x = (uint32)vector.x + matrix->t[0];
                    matrix = path_definition->transform != 0 ? &path_definition->transform->matrix : NULL;
                    vector.y = (uint32)vector.y + matrix->t[1];
                    matrix = path_definition->transform != 0 ? &path_definition->transform->matrix : NULL;
                    vector.z = (uint32)vector.z + matrix->t[2];
                }
                else
                {
                    x = pos.x;
                    y = pos.y;
                    z = pos.z;
                    vector = pos;
                }
            }
            frame = (sint16)path_definition->FindHeightFrame(vector, flags & 0x10, -61440);
            path_frame = frame;
            if (frame >= 0)
            {
                path_frame = path_definition->ProjectFrame(frame, vector, floor, flags & 0x10, 4096);
                cur_flags = flags;
                if ((cur_flags & 0x400) != 0)
                {
                    if (floor.y >= (sint32)((uint32)vector.y - 682))
                        flags = cur_flags ^ 0x4400;
                    else
                        floor.y = vector.y;
                }
                {
                    const PathDefinition *parent = path_definition;
                    MATRIX *matrix;
                    uint32 x, y, z;
                    if (parent != 0 && parent->transform != 0)
                    {
                        matrix = path_definition->transform != 0 ? &path_definition->transform->matrix : NULL;
                        inverse_input.x = (uint32)floor.x - matrix->t[0];
                        matrix = path_definition->transform != 0 ? &path_definition->transform->matrix : NULL;
                        inverse_input.y = (uint32)floor.y - matrix->t[1];
                        matrix = path_definition->transform != 0 ? &path_definition->transform->matrix : NULL;
                        inverse_input.z = (uint32)floor.z - matrix->t[2];
                        {
                            MATRIX *matrix;
                            uint32 idx;
                            for (idx = 0; idx < 9; ++idx)
                            {
                                matrix = path_definition->transform != 0 ? &path_definition->transform->matrix : NULL;
                                inverse_rotation.m[idx / 3][idx % 3] = matrix->m[idx % 3][idx / 3];
                            }
                        }
                        {
                            uint32 idx;
                            PushMatrix();
                            SetRotMatrix(&inverse_rotation);
                            idx = 4;
                            {
                                VECTOR rotation_input = {inverse_input.x, inverse_input.y, inverse_input.z, 0};
                                VECTOR rotation_output;
                                ApplyRotMatrixLV(&rotation_input, &rotation_output);
                                pos.x = rotation_output.vx;
                                pos.y = rotation_output.vy;
                                pos.z = rotation_output.vz;
                            }
                            PopMatrix();
                        }
                    }
                    else
                    {
                        x = floor.x;
                        y = floor.y;
                        z = floor.z;
                        pos = floor;
                    }
                }
            }
            if ((sint32)speed[0] < 0)
            {
                AnimatedPathState *path_state = (AnimatedPathState *)(((const ActorPlacement *)path_definition)->state);
                uint32 cur_flags0;
                const PathDefinition *parent;
                sint32 count, frame;
                path_state->speed[0] = -path_state->speed[0];
                cur_flags0 = flags;
                parent = path_definition;
                flags = cur_flags0 ^ 0x10;
                count = parent->pos_track.frame_count;
                frame = count - path_frame;
                if (frame >= count)
                    frame = count - 1;
                else if (frame < 0)
                    frame = 0;
                path_frame = frame;
            }
        }
        cur_flags = flags;
        if ((cur_flags & 0x800) != 0)
        {
            mode = definition->flags >> 13;
            flags = cur_flags & 0xFFFFF7FF;
            switch (mode)
            {
                case 0:
                case 1:
                    if ((path_definition->pos_track.flags & 2) != 0)
                        break;
                    path_frame = 0;
                    flags = flags ^ 0x10;
                    speed[0] = 0;
                    if (mode == 1)
                    {
                        velocity.z = 0;
                        velocity.x = 0;
                    }
                    else
                    {
                        x = velocity.x;
                        y = velocity.y;
                        z = velocity.z;
                        velocity.x = -x;
                        velocity.y = -y;
                        velocity.z = -z;
                        if ((flags & 0x400) != 0)
                        {
                            velocity.y = -(uint32)velocity.y;
                            {
                                const PathDefinition *parent = path_definition;
                                MATRIX *matrix;
                                uint32 idx, x, y, z;
                                if (parent != 0 && parent->transform != 0)
                                {
                                    x = pos.x;
                                    y = pos.y;
                                    z = pos.z;
                                    event_input = pos;
                                    PushMatrix();
                                    matrix = path_definition->transform != 0 ? &path_definition->transform->matrix : NULL;
                                    SetRotMatrix(matrix);
                                    idx = 5;

                                    {
                                        VECTOR rotation_input = {event_input.x, event_input.y, event_input.z, 0};
                                        VECTOR rotation_output;
                                        ApplyRotMatrixLV(&rotation_input, &rotation_output);
                                        vector.x = rotation_output.vx;
                                        vector.y = rotation_output.vy;
                                        vector.z = rotation_output.vz;
                                    }
                                    PopMatrix();
                                    matrix = path_definition->transform != 0 ? &path_definition->transform->matrix : NULL;
                                    vector.x = (uint32)vector.x + matrix->t[0];
                                    matrix = path_definition->transform != 0 ? &path_definition->transform->matrix : NULL;
                                    vector.y = (uint32)vector.y + matrix->t[1];
                                    matrix = path_definition->transform != 0 ? &path_definition->transform->matrix : NULL;
                                    vector.z = (uint32)vector.z + matrix->t[2];
                                }
                                else
                                {
                                    x = pos.x;
                                    y = pos.y;
                                    z = pos.z;
                                    vector = pos;
                                }
                            }
                        }
                        {
                            uint32 velocity_len;
                            {
                                uint32 x = velocity.x >> 6;
                                uint32 y = velocity.y >> 6;
                                uint32 z = velocity.z >> 6;
                                uint32 sum = x * x + y * y + z * z;
                                if ((sint32)sum < 0)
                                    velocity_len = SquareRoot12((sum >> 2)) << 1;
                                else
                                    velocity_len = SquareRoot12(sum);
                            }
                            if (velocity_len != 0)
                            {
                                x = velocity.x;
                                y = velocity.y;
                                z = velocity.z;
                                path_state->dir = velocity;
                                flags = flags | 0x400000;
                            }
                        }
                    }
                    {
                        const PathDefinition *parent = path_definition;
                        MATRIX *matrix;
                        uint32 idx, x, y, z;
                        if (parent != 0 && parent->transform != 0)
                        {
                            x = pos.x;
                            y = pos.y;
                            z = pos.z;
                            event_input = pos;
                            PushMatrix();
                            matrix = path_definition->transform != 0 ? &path_definition->transform->matrix : NULL;
                            SetRotMatrix(matrix);
                            idx = 5;

                            {
                                VECTOR rotation_input = {event_input.x, event_input.y, event_input.z, 0};
                                VECTOR rotation_output;
                                ApplyRotMatrixLV(&rotation_input, &rotation_output);
                                floor.x = rotation_output.vx;
                                floor.y = rotation_output.vy;
                                floor.z = rotation_output.vz;
                            }
                            PopMatrix();
                            matrix = path_definition->transform != 0 ? &path_definition->transform->matrix : NULL;
                            floor.x = (uint32)floor.x + matrix->t[0];
                            matrix = path_definition->transform != 0 ? &path_definition->transform->matrix : NULL;
                            floor.y = (uint32)floor.y + matrix->t[1];
                            matrix = path_definition->transform != 0 ? &path_definition->transform->matrix : NULL;
                            floor.z = (uint32)floor.z + matrix->t[2];
                        }
                        else
                        {
                            x = pos.x;
                            y = pos.y;
                            z = pos.z;
                            floor = pos;
                        }
                    }
                    x = motion.x - (uint32)floor.x;
                    y = motion.y - (uint32)floor.y;
                    z = motion.z - (uint32)floor.z;
                    motion.x = x;
                    motion.y = y;
                    motion.z = z;
                    parent = path_definition;
                    frame = path_frame;
                    if ((flags & 0x10) == 0)
                        frame = parent->pos_track.frame_count - frame - 1;
                    AnimationTrack::SampleVectorTrack(frame, &parent->pos_track, pos);
                    {
                        const PathDefinition *parent = path_definition;
                        MATRIX *matrix;
                        uint32 idx, x, y, z;
                        if (parent != 0 && parent->transform != 0)
                        {
                            x = pos.x;
                            y = pos.y;
                            z = pos.z;
                            sample_input = pos;
                            PushMatrix();
                            matrix = path_definition->transform != 0 ? &path_definition->transform->matrix : NULL;
                            SetRotMatrix(matrix);
                            idx = 5;

                            {
                                VECTOR rotation_input = {sample_input.x, sample_input.y, sample_input.z, 0};
                                VECTOR rotation_output;
                                ApplyRotMatrixLV(&rotation_input, &rotation_output);
                                floor.x = rotation_output.vx;
                                floor.y = rotation_output.vy;
                                floor.z = rotation_output.vz;
                            }
                            PopMatrix();
                            matrix = path_definition->transform != 0 ? &path_definition->transform->matrix : NULL;
                            floor.x = (uint32)floor.x + matrix->t[0];
                            matrix = path_definition->transform != 0 ? &path_definition->transform->matrix : NULL;
                            floor.y = (uint32)floor.y + matrix->t[1];
                            matrix = path_definition->transform != 0 ? &path_definition->transform->matrix : NULL;
                            floor.z = (uint32)floor.z + matrix->t[2];
                        }
                        else
                        {
                            x = pos.x;
                            y = pos.y;
                            z = pos.z;
                            floor = pos;
                        }
                    }
                    x = motion.x + (uint32)floor.x;
                    y = motion.y + (uint32)floor.y;
                    z = motion.z + (uint32)floor.z;
                    motion.x = x;
                    motion.y = y;
                    motion.z = z;
                    if ((flags & 0x400) != 0)
                    {
                        floor.y = vector.y;
                        {
                            const PathDefinition *parent = path_definition;
                            MATRIX *matrix;
                            uint32 x, y, z;
                            if (parent != 0 && parent->transform != 0)
                            {
                                matrix = path_definition->transform != 0 ? &path_definition->transform->matrix : NULL;
                                final_input.x = (uint32)floor.x - matrix->t[0];
                                matrix = path_definition->transform != 0 ? &path_definition->transform->matrix : NULL;
                                final_input.y = (uint32)floor.y - matrix->t[1];
                                matrix = path_definition->transform != 0 ? &path_definition->transform->matrix : NULL;
                                final_input.z = (uint32)floor.z - matrix->t[2];
                                {
                                    MATRIX *matrix;
                                    uint32 idx;
                                    for (idx = 0; idx < 9; ++idx)
                                    {
                                        matrix = path_definition->transform != 0 ? &path_definition->transform->matrix : NULL;
                                        final_rotation.m[idx / 3][idx % 3] = matrix->m[idx % 3][idx / 3];
                                    }
                                }
                                {
                                    uint32 idx;
                                    PushMatrix();
                                    SetRotMatrix(&final_rotation);
                                    idx = 4;
                                    {
                                        VECTOR rotation_input = {final_input.x, final_input.y, final_input.z, 0};
                                        VECTOR rotation_output;
                                        ApplyRotMatrixLV(&rotation_input, &rotation_output);
                                        pos.x = rotation_output.vx;
                                        pos.y = rotation_output.vy;
                                        pos.z = rotation_output.vz;
                                    }
                                    PopMatrix();
                                }
                            }
                            else
                            {
                                x = floor.x;
                                y = floor.y;
                                z = floor.z;
                                pos = floor;
                            }
                        }
                    }
                    break;
                case 2:
                    ticks = 0;
                    break;
                case 3:
                {
                    const PathDefinition *parent = path_definition;
                    MATRIX *matrix;
                    uint32 idx, x, y, z;
                    if (parent != 0 && parent->transform != 0)
                    {
                        x = pos.x;
                        y = pos.y;
                        z = pos.z;
                        world_input = pos;
                        PushMatrix();
                        matrix = path_definition->transform != 0 ? &path_definition->transform->matrix : NULL;
                        SetRotMatrix(matrix);
                        idx = 5;

                        {
                            VECTOR rotation_input = {world_input.x, world_input.y, world_input.z, 0};
                            VECTOR rotation_output;
                            ApplyRotMatrixLV(&rotation_input, &rotation_output);
                            pos.x = rotation_output.vx;
                            pos.y = rotation_output.vy;
                            pos.z = rotation_output.vz;
                        }
                        PopMatrix();
                        matrix = path_definition->transform != 0 ? &path_definition->transform->matrix : NULL;
                        pos.x = (uint32)pos.x + matrix->t[0];
                        matrix = path_definition->transform != 0 ? &path_definition->transform->matrix : NULL;
                        pos.y = (uint32)pos.y + matrix->t[1];
                        matrix = path_definition->transform != 0 ? &path_definition->transform->matrix : NULL;
                        pos.z = (uint32)pos.z + matrix->t[2];
                    }
                    else
                    {
                        x = pos.x;
                        y = pos.y;
                        z = pos.z;
                    }
                }
                    flags = flags | 0x2000;
                    CapturePathVelocity();
                    ticks = 150;
                    break;
                case 4:
                case 5:
                    if (ticks == 0)
                        goto expire;
                    cur_flags = flags;
                    val = node.flags;
                    ticks = -1;
                    movement_mode = 0;
                    result = cur_flags | 0x4000;
                    flags = result;
                    node.flags = val | 1;
                    goto finish;
            }
        }
    }
    if (ticks > 0)
        ApplyEvents();
    else
    {
    expire:
        if ((sound_effects.IsPlaying(path_state->sound, path_state->sound_voice) & 0xFF) != 0)
            SoundEffects::StopSoundVoice(path_state->sound_voice);
        path_state->sound = 0;
        path_state->sound_voice = -1;
        frame = ticks;
        val = node.flags | 1;
        flags = flags | 0x4000;
        node.flags = val;
        if (frame != -2)
        {
            frame = ticks;
            movement_mode = 0;
            if (frame != 0)
            {
                parent = path_definition;
                ticks = 1;
                path_frame = 0;
                AnimationTrack::SampleVectorTrack(0, &parent->pos_track, pos);
                result = flags | 0x8000;
                val = node.flags & 0xFE;
                flags = result;
                node.flags = val;
            }
            else
            {
                result = 4;
                contact_kind = 0;
                behavior_suspended = 4;
            }
            goto finish;
        }
    }
    if ((flags & 0x200000) == 0)
    {
        cur_flags = owner->flags;
        owner->behavior = 0;
        owner->flags = cur_flags | 0x20020;
        motion.z = 0;
        motion.y = 0;
        motion.x = 0;
        velocity.z = 0;
        velocity.y = 0;
        velocity.x = 0;
        speed[2] = 0;
        speed[1] = 0;
        speed[0] = 0;
        owner->path_motion_scale = 0;
        cur_flags = flags;
        yaw = 4096;
        if ((cur_flags & 0x40000) != 0)
        {
            speed[0] = definition->initial_speed;
            movement_mode = 3;
        }
        else
            movement_mode = 0;
        initial_kind = 3;
        flags = flags | 0x200010;
    }
    animation_idx = movement_mode == 3 ? 1 : 0;
    if (owner->animation != definition->animations[animation_idx])
    {
        if ((sound_effects.IsPlaying(path_state->sound, path_state->sound_voice) & 0xFF) != 0)
            SoundEffects::StopSoundVoice(path_state->sound_voice);
        path_state->sound = 0;
        path_state->sound_voice = -1;
        cur_flags = owner->flags;
        owner->animation = definition->animations[animation_idx];
        owner->frame = -1;
        owner->flags = cur_flags | 0x10000;
    }
    if (initial_kind == 3 && (flags & 0x2000) == 0)
        AnimatedObject::UpdateAnimatedModelOrientation(definition, this);
    val = node.flags;
    result = val | 0x40;
    if ((val & 1) != 0)
        goto finish;
    node.flags = result;
    active_animation = owner->animation;
    if (active_animation != 0)
    {
        frame = owner->frame;
        frame = frame < active_animation->frame_count - 1 ? frame + 1 : 0;
        owner->frame = frame;
        owner->ClampAnimationFrame();
    }
    if ((flags & 0x400) != 0)
    {
        motion.y = 0;
    }

    {
        uint32 x = motion.x >> 6;
        uint32 y = motion.y >> 6;
        uint32 z = motion.z >> 6;
        uint32 sum = x * x + y * y + z * z;
        if ((sint32)sum < 0)
            len = SquareRoot12((sum >> 2)) << 1;
        else
            len = SquareRoot12(sum);
    }
    vector.x = len;
    if (len == 0)
    {
        sound_bank = definition->sound_table;
        sound = sound_bank != 0 && sound_bank->header.count >= 1 ? sound_bank->samples + 0 : 0;
        pitch = sound->pitch;
        if (state == 4)
            option = 1;
    }
    else
    {
        if ((flags & 0x10) != 0)
            vector.x = -len;
        owner->path_motion_scale = owner->path_motion_scale + vector.x;
        owner->flags = owner->flags | 1;
        owner->node.flags = owner->node.flags | 0x40;
        sound_bank = definition->sound_table;
        sound = sound_bank != 0 && sound_bank->header.count >= 2 ? sound_bank->samples + 1 : 0;
        if (state - 30 < 2)
            option = 1;
        base = sound->pitch << 12;
        val = ApproxVectorLen(motion.x, 0, motion.z);
        len = definition->speed_limit;
        if ((sint32)val < (sint32)len)
        {
            if (len == 0)
            {
                trap(7);
            }
            quotient = 0x40000000 / (sint32)len;
            factor = ((sint64)(sint32)val * quotient) >> 18;
            fixed_math_result = factor;
            base = mul12(base, 3072);
            pitch = (sint32)(base + (uint32)mul12((sint32)factor, base)) >> 12;
            volume = (uint32)((sint32)(factor * 3072) >> 12) + 1024;
        }
        else
            pitch = (sint32)mul12(base, 6144) >> 12;
    }
    result = owner->flags & 0x2000000;
    if (result != 0)
    {
        scene_node = owner->node.links.first_child;
        distance = owner->path_motion_scale;
        changed = 0;
        while (scene_node != 0)
        {
            if (scene_node->kind == 6 && owner->node.kind == 5)
            {
                denominator = mul12(25735, (sint32)((const SphereNode *)scene_node)->radius);
                if (denominator == 0)
                {
                    trap(7);
                }
                quotient = 0x40000000 / denominator;
                factor = ((sint64)(sint32)distance * quotient) >> 18;
                frame = (sint32)factor >> 1;
                fixed_math_result = factor;
                ((MeshPartNode *)owner)->animation_binding.tracks[1] = -1;
                ((MeshPartNode *)owner)->rotation[0] = SinQ14(1024 - (uint32)frame);
                ((MeshPartNode *)owner)->rotation[1] = SinQ14(frame);
                changed = 1;
                ((MeshPartNode *)owner)->rotation[3] = 0;
                ((MeshPartNode *)owner)->rotation[2] = 0;
            }
            val = AnimatedObject::ApplyModelTravelRotation(scene_node, distance);
            scene_node = scene_node->links.sibling_next;
            changed |= val;
        }
        result = changed << 24;
        if (result == 0)
        {
            result = owner->flags & 0xFDFFFFFF;
            owner->flags = result;
        }
    }
    if (sound != 0)
    {
        prev = path_state->sound;
        if (sound != prev && prev != 0)
        {
            if ((sound_effects.IsPlaying(prev, path_state->sound_voice) & 0xFF) != 0)
                SoundEffects::StopSoundVoice(path_state->sound_voice);
            path_state->sound = 0;
            path_state->sound_voice = -1;
        }
        path_state->sound = sound;
        result = SoundEffects::PlaySpatialSound(sound, (volume + 4096), 0, pitch, world_pos.comp, (uint32)definition->sound_radius << 5, path_state->sound_voice, 1, option);
        path_state->sound_voice = result;
    }
finish:
    return result;
}

void AnimatedObject::ResetAnimatedModel()
{
    FUNCTION_MARKER(0x80043DE4, "SLUS_005.78");
    uint32 count = animated_models.count;
    uint32 kind = state;
    AnimatedModelDefinition *const *table = animated_models.definitions;
    const AnimatedModelDefinition *sel = NULL;
    AnimatedPathState *path_state;
    AnimatedObject *other;
    SceneNode *child_reference;
    uint32 cur_flags;
    const PathDefinition *definition;
    ticks = 1;
    while (count-- != 0)
    {
        const AnimatedModelDefinition *candidate = *table;
        ++table;
        if (candidate->kind_id == kind)
        {
            sel = candidate;
            break;
        }
    }
    path_state = (AnimatedPathState *)(((const ActorPlacement *)path_definition)->state);
    if ((sound_effects.IsPlaying(path_state->sound, path_state->sound_voice) & 255) != 0)
        SoundEffects::StopSoundVoice(path_state->sound_voice);
    path_state->sound = 0;
    path_state->sound_voice = -1;
    cur_flags = flags;
    movement_mode = 0;
    behavior_suspended = 0;
    contact_kind = 4;
    if ((cur_flags & 0x8000) == 0)
    {
        uint32 byte_flags = node.flags & 254;
        uint32 cur_state = playback.flags;
        uint32 frame;
        cur_flags = flags & 0xFFF3BEFF;
        node.flags = byte_flags;
        definition = path_definition;
        node.flags = byte_flags | (cur_state & 1);
        flags = cur_flags;
        flags = cur_flags | (cur_state & 0x40000);
        frame = ((const AnimatedPathState *)(((const ActorPlacement *)definition)->state))->start_frame;
        definition = path_definition;
        path_frame = frame;
        AnimationTrack::SampleVectorTrack((sint16)frame, &definition->pos_track, pos);
    }
    cur_flags = flags;
    spheres[3].radius = -1;
    spheres[2].radius = -1;
    spheres[1].radius = -1;
    spheres[0].radius = -1;
    yaw = 4096;
    velocity.z = 0;
    velocity.y = 0;
    velocity.x = 0;
    flags = cur_flags & 0xFE1F13FF;
    definition = path_definition;
    if (definition != 0 && definition->transform != 0)
    {
        Vec3 input;

        MATRIX *matrix;
        input.x = pos.x;
        input.y = pos.y;
        input.z = pos.z;
        PushMatrix();
        matrix = path_definition->transform != 0 ? &path_definition->transform->matrix : NULL;
        SetRotMatrix(matrix);

        {
            VECTOR rotation_output;
            {
                VECTOR sdk_input = {input.x, input.y, input.z, 0};
                ApplyRotMatrixLV(&sdk_input, &rotation_output);
            }
            spheres[1].pos.x = rotation_output.vx;
            spheres[1].pos.y = rotation_output.vy;
            spheres[1].pos.z = rotation_output.vz;
        }
        PopMatrix();
        matrix = path_definition->transform != 0 ? &path_definition->transform->matrix : NULL;
        spheres[1].pos.x = (uint32)spheres[1].pos.x + matrix->t[0];
        matrix = path_definition->transform != 0 ? &path_definition->transform->matrix : NULL;
        spheres[1].pos.y = (uint32)spheres[1].pos.y + matrix->t[1];
        matrix = path_definition->transform != 0 ? &path_definition->transform->matrix : NULL;
        spheres[1].pos.z = (uint32)spheres[1].pos.z + matrix->t[2];
    }
    else
    {
        spheres[1].pos.x = pos.x;
        spheres[1].pos.y = pos.y;
        spheres[1].pos.z = pos.z;
    }
    world_pos.x = spheres[1].pos.x;
    world_pos.y = spheres[1].pos.y;
    world_pos.z = spheres[1].pos.z;
    child_reference = node.links.first_child;
    other = child_reference != 0 ? (AnimatedObject *)child_reference : NULL;
    cur_flags = other->flags;
    other->behavior = 0;
    other->path_motion_scale = 0;
    other->flags = cur_flags | 0x2020020;
    AnimatedObject::UpdateAnimatedModelOrientation(sel, this);
}

intptr AnimatedObject::HideAnimatedModel()
{
    FUNCTION_MARKER(0x80044230, "SLUS_005.78");
    SceneNode *parent = owner_node;
    uint32 result;
    parent->flags = parent->flags | 0x80;
    result = node.flags | 0x80;
    node.flags = result;
    return result;
}

intptr AnimatedObject::UpdateActorFacingAttack()
{
    FUNCTION_MARKER(0x8004425C, "SLUS_005.78");
    ActorModelDefinition *resource_model;
    ActorBinding *binding = (ActorBinding *)(((const ActorPlacement *)path_definition)->state);

    resource_model = binding->model;
    Vec3 origin;
    Vec3 target_pos;
    uint32 distance;
    uint32 result;
    AnimationHeader *clip;
    const Vec3 *target;
    sint32 sample_frame;
    UpdateActorStateAndTarget(origin, target_pos, &distance);
    target = &target_pos;
    if ((binding->flags & 2) != 0)
        target = 0;
    else
    {
        if ((path_definition->flags & 0x40) == 0 && animation != resource_model->animations[0])
        {
            animation = resource_model->animations[0];
            frame = -1;
            flags = flags | 0x10000;
        }
        clip = animation;
        sample_frame = frame;
        if (clip == resource_model->animations[2])
        {
            if ((clip == 0 || sample_frame >= clip->frame_count - 1) && sample_frame > 0)
            {
                frame = -1;
                animation = resource_model->animations[0];
                flags = flags | 0x10000;
            }
        }
        else if ((path_definition->flags & 0x40) != 0 && resource_model->target_range[0] >= (sint32)distance && resource_model->target_range[1] >= (sint32)distance)
        {
            sint16 angle = (uint16)yaw - Atan2Angle((origin.z - (uint32)target->z), (origin.x - (uint32)target->x));
            if (angle >= -63 && angle < 64)
            {
                frame = -1;
                animation = resource_model->animations[2];
                flags = flags | 0x10000;
            }
        }
    }
    result = UpdateActorFacingAndAnimation(target, origin, resource_model);
    return result;
}

intptr AnimatedObject::InitializeActorFacingAttack()
{
    FUNCTION_MARKER(0x80044444, "SLUS_005.78");
    if (!object_operations.behaviors[8].plain_handler && !object_operations.behaviors[8].member_handler && !object_operations.behaviors[8].void_member_handler && !object_operations.behaviors[8].factory_handler)
    {
        ObjectCallback operation = {0, &AnimatedObject::UpdateActorFacingAttack};
        object_operations.behaviors[8] = operation;
    }
    behavior = 8;
    return 8;
}

uint32 AnimatedObject::ReleaseActorInteraction()
{
    const PathDefinition *parent;
    FUNCTION_MARKER(0x8004483C, "SLUS_005.78");

    VECTOR sample_pos;

    uint32 global_flags, cur_flags, x, y, z, val, result;
    PathTransform *path_transform;
    global_flags = player_interaction.flags;
    cur_flags = flags;
    ticks = 2;
    result = 0xFFFFFFFE;
    player_interaction.flags = global_flags & result;
    contact_kind = 0;
    if ((cur_flags & 0x2000) != 0)
        return result;
    parent = path_definition;
    if (parent != 0 && parent->transform != 0)
    {
        x = pos.x;
        y = pos.y;
        z = pos.z;
        sample_pos.vx = x;
        sample_pos.vy = y;
        sample_pos.vz = z;
        PushMatrix();
        path_transform = path_definition->transform;
        SetRotMatrix(&path_transform->matrix);
        {
            VECTOR rotation_output;
            ApplyRotMatrixLV(&sample_pos, &rotation_output);
            pos.x = rotation_output.vx;
            pos.y = rotation_output.vy;
            pos.z = rotation_output.vz;
        }
        PopMatrix();
        path_transform = path_definition->transform;
        x = pos.x;
        val = path_transform->matrix.t[0];
        parent = path_definition;
        pos.x = x + val;
        path_transform = parent->transform;
        y = pos.y;
        val = path_transform->matrix.t[1];
        parent = path_definition;
        pos.y = y + val;
        path_transform = parent->transform;
        z = pos.z;
        val = path_transform->matrix.t[2];
        pos.z = z + val;
    }
    result = flags | 0x2000;
    flags = result;
    return result;
}

void AnimatedObject::ApplyInteractionDirectionalInput(uint32 *speed)
{
    const PathDefinition *parent;
    FUNCTION_MARKER(0x80044990, "SLUS_005.78");

    VECTOR tangent;

    uint32 kind, cur_flags, buttons, mask, result;
    uint32 val;
    uint32 frame;
    uint32 x;
    uint32 y;
    uint32 z;
    sint32 multiplier;
    uint32 scaled_x;
    sint32 timer, limit, delta;
    CharacterController::FindPlayerObject(0);
    kind = state;
    timer = ticks;
    limit = (sint16)ModelGroupRegistry::GetModelGrpLifetime(kind);
    if (timer < limit - 15 && (player_interaction.flags & 0x10) == 0)
    {
        ReleaseActorInteraction();
        return;
    }
    cur_flags = flags;
    if ((cur_flags & 0x20200) == 0x200 || character_pad.motion.state_code == 4 || (character_pad.secondary_flags & 0x240) != 0 || (character_pad.flags & 0x04000000) != 0)
    {
        ReleaseActorInteraction();
        return;
    }
    buttons = gameplay_input.buttons;
    mask = (*control_bindings.active_scheme)[0];
    if ((buttons & mask) == 0)
    {
        ReleaseActorInteraction();
        return;
    }
    val = player_interaction.flags;

    if ((val & 0x10) == 0)
    {
        return;
    }
    delta = 0;
    if ((buttons & 0x8000) != 0)
        delta = (cur_flags & 0x10) != 0 ? -819 : 819;
    else if ((buttons & 0x2000) != 0)
        delta = (cur_flags & 0x10) != 0 ? 819 : -819;
    val = *speed + delta;
    *speed = val;
    if ((sint32)val >= 6145)
        *speed = 6144;
    if (delta != 0)
    {
        val = *speed;
        if ((sint32)val < 0)
        {
            *speed = -val;
            cur_flags = flags;
            parent = path_definition;
            flags = cur_flags ^ 0x10;
            timer = parent->pos_track.frame_count;
            frame = (uint32)timer - path_frame;
            if ((sint32)frame >= timer)
                frame = (uint32)timer - 1;
            else if ((sint32)frame < 0)
                frame = 0;
            path_frame = frame;
        }
        cur_flags = flags;
        if ((cur_flags & 0x2000) != 0)
        {
            parent = path_definition;
            parent->SampleTangent(&tangent, (cur_flags & 0x10));
            x = tangent.vx;
            tangent.vy = 0;
            if (x + 26754 > 0xD103 || (uint32)tangent.vz + 26754 > 0xD103)
            {
                do
                {
                    x = tangent.vx >> 1;
                    y = tangent.vy >> 1;
                    z = tangent.vz >> 1;
                    tangent.vx = x;
                    tangent.vy = y;
                    tangent.vz = z;
                } while (x + 26754 > 0xD103 || y + 26754 > 0xD103 || z + 26754 > 0xD103);
            }
            VectorNormal(&tangent, &tangent);
            x = tangent.vx;
            multiplier = *speed;
            scaled_x = mul12((sint32)x, multiplier);
            tangent.vx = scaled_x;
            multiplier = *speed;
            y = tangent.vy;
            tangent.vy = mul12((sint32)y, multiplier);
            multiplier = *speed;
            z = tangent.vz;
            tangent.vz = mul12((sint32)z, multiplier);
            velocity.x = scaled_x;
            z = tangent.vz;
            velocity.z = z;
        }
    }
    buttons = gameplay_input.buttons;
    delta = 0;
    if ((buttons & 0x1000) != 0)
        delta = 819;
    else if ((buttons & 0x4000) != 0)
        delta = -819;
    val = (uint32)velocity.y + delta;
    velocity.y = val;
    result = (sint32)val >= 6145 ? 6144 : (uint32)-6144;
    if ((sint32)val >= 6145 || (sint32)val < -6144)
        velocity.y = result;
}

void AnimatedObject::ApplyActorDirectionalInput(uint32 *speed)
{
    FUNCTION_MARKER(0x80044CFC, "SLUS_005.78");
    uint32 cur_flags, input, val;
    sint32 delta = 0;
    CharacterController::FindPlayerObject(0);
    if (((uint16)(character_pad.motion.form_flags >> 16) & 15) != 0)
        return;
    cur_flags = flags;
    if ((cur_flags & 0x20200) == 512 || character_pad.motion.state_code == 4)
        return;
    if ((path_routing.flags & 16) == 0)
        return;
    input = gameplay_input.buttons;
    if ((input & 0x8000) != 0)
    {
        delta = (cur_flags & 16) != 0 ? -819 : 819;
        gameplay_input.buttons = input & ~0x8000u;
    }
    else if ((input & 0x2000) != 0)
    {
        delta = (cur_flags & 16) != 0 ? 819 : -819;
        gameplay_input.buttons = input & ~0x2000u;
    }
    val = *speed + delta;
    *speed = val;
    if ((sint32)val >= 12289)
        *speed = 12288;
    if (delta != 0)
    {
        if ((sint32)*speed < 0)
        {
            const PathDefinition *definition;
            sint32 count, frame;
            *speed = -(*speed);
            cur_flags = flags;
            definition = path_definition;
            flags = cur_flags ^ 16;
            count = definition->pos_track.frame_count;
            frame = count - path_frame;
            if (frame >= count)
                frame = count - 1;
            else if (frame < 0)
                frame = 0;
            path_frame = frame;
        }
        cur_flags = flags;
        if ((cur_flags & 0x2000) != 0)
        {
            Vec3 tangent;


            {
                VECTOR sdk_output;
                path_definition->SampleTangent(&sdk_output, cur_flags & 16);
                tangent.x = sdk_output.vx;
                tangent.y = sdk_output.vy;
                tangent.z = sdk_output.vz;
            }
            tangent.y = 0;
            if (tangent.x + 26754u > 0xD103 || tangent.z + 26754u > 0xD103)
            {
                do
                {
                    tangent.x >>= 1;
                    tangent.y >>= 1;
                    tangent.z >>= 1;
                } while (tangent.x + 26754u > 0xD103 || tangent.y + 26754u > 0xD103 || tangent.z + 26754u > 0xD103);
            }
            {
                VECTOR sdk_input = {tangent.x, tangent.y, tangent.z, 0};
                VECTOR sdk_output;
                VectorNormal(&sdk_input, &sdk_output);
                tangent.x = sdk_output.vx;
                tangent.y = sdk_output.vy;
                tangent.z = sdk_output.vz;
            }
            tangent.x = mul12(tangent.x, (sint32)*speed);
            tangent.y = mul12(tangent.y, (sint32)*speed);
            tangent.z = mul12(tangent.z, (sint32)*speed);
            velocity.x = tangent.x;
            velocity.z = tangent.z;
        }
    }
    input = gameplay_input.buttons;
    delta = (input & 0x1000) != 0 ? 819 : ((input & 0x4000) != 0 ? -819 : 0);
    val = (uint32)velocity.y + delta;
    velocity.y = val;
    if ((sint32)val >= 12289)
    {
        velocity.y = 12288;
        return;
    }
    if ((sint32)val < -12288)
        velocity.y = -12288;
}

void AnimatedObject::QueueCharacterActionSound(sint32 action, uint32 priority)
{
    FUNCTION_MARKER(0x80044FFC, "SLUS_005.78");
    uint32 mode, idx, minimum;
    const SoundBankStorage *table;
    const SoundSample *sound;
    if (action == 2)
    {
        mode = (uint16)(character_pad.motion.form_flags >> 16) & 15;
        if (mode == 1 || mode == 7 || mode == 9 || mode == 14 || mode == 11)
            action = 3;
    }
    table = sound_resources.common_table;
    if (table != 0 && action < table->header.count)
        sound = table->samples + action;
    else
        sound = 0;
    mode = (uint16)(character_pad.motion.form_flags >> 16) & 15;
    minimum = 0;
    idx = 0;
    if (mode == 4)
    {
        if (action == 0)
        {
            minimum = 12;
            idx = 11;
        }
        else if (action == 1)
        {
            minimum = 13;
            idx = 12;
        }
        else if (action == 2)
        {
            minimum = 14;
            idx = 13;
        }
        else if (action == 7)
        {
            minimum = 4;
            idx = 3;
        }
    }
    else if (mode == 5 && action == 2)
    {
        minimum = 14;
        idx = 13;
    }
    if (minimum != 0)
    {
        table = sound_resources.scene_table;
        sound = 0;
        if (table != 0 && table->header.count >= (sint32)minimum)
            sound = table->samples + idx;
    }
    mode = (uint16)sound_settings.effects_volume;
    sound_effects.Queue(sound, 0, 0, mode, 0, priority);
}

void AnimatedObject::ApplyCharacterPathMotion(sint32 horizontal_speed, sint32 vertical_speed)
{
    FUNCTION_MARKER(0x800455DC, "SLUS_005.78");
    const PathDefinition *parent;
    uint32 distance;
    sint32 result;
    PathTransform *path_transform;
    Vec3 sample_pos, proj, src;
    distance = horizontal_speed;
    if (horizontal_speed >= 0)
        flags = flags | 0x40;
    else
        distance = -distance;
    ComputePathMotion(distance, vertical_speed, &character_pad.path_tangent_coord);
    if (((uint16)(character_pad.motion.form_flags >> 16) & 15) == 5)
    {
        parent = path_definition;
        if (parent != 0 && parent->transform != 0)
        {
            src = pos;
            PushMatrix();
            path_transform = path_definition->transform;
            SetRotMatrix(&path_transform->matrix);
            {
                VECTOR sdk_input = {src.x, src.y, src.z, 0};
                VECTOR sdk_output;
                ApplyRotMatrixLV(&sdk_input, &sdk_output);
                sample_pos.x = sdk_output.vx;
                sample_pos.y = sdk_output.vy;
                sample_pos.z = sdk_output.vz;
            }
            PopMatrix();
            path_transform = path_definition->transform;
            sample_pos.x = (uint32)sample_pos.x + path_transform->matrix.t[0];
            path_transform = path_definition->transform;
            sample_pos.y = (uint32)sample_pos.y + path_transform->matrix.t[1];
            path_transform = path_definition->transform;
            sample_pos.z = (uint32)sample_pos.z + path_transform->matrix.t[2];
        }
        else
        {
            sample_pos = pos;
        }
        proj.x = sample_pos.x + (uint32)velocity.x;
        proj.y = sample_pos.y + (uint32)velocity.y;
        proj.z = sample_pos.z + (uint32)velocity.z;
        result = ComputePathBoundaryExcess(proj);
        if (result > 0)
            ComputePathMotion(0, 0, &character_pad.path_tangent_coord);
    }
    flags = flags & 0xFFFFFFBF;
}

void AnimatedObject::UpdateCharacterCameraPath()
{
    const PathDefinition *parent;
    FUNCTION_MARKER(0x800457C8, "SLUS_005.78");

    Vec3 delta, sample_pos;

    uint32 dir, cur_flags, x, y, z, result;
    PathTransform *path_transform;
    const ScenePath *cur;
    Vec3 *dst;
    sint32 frame;

    cur_flags = flags;
    dir = cur_flags & 0x10;
    camera_motion.flags = camera_motion.flags & 0xFE;
    if (CutscenePlayback::GetCutsceneMode() != 0)
    {
        if (CutscenePlayback::UpdateCutscene() == 0)
        {
            character_pad.path_velocity.vz = 0;
            character_pad.path_velocity.vy = 0;
            character_pad.path_velocity.vx = 0;
            return;
        }
        character_pad.motion.pos_changed = 1;
    }
    cur_flags = path_flags;
    if (((cur_flags & 0x100) != 0 && path_transition_rules.camera_duration != 0) || (cur_flags & 0x40000) != 0)
    {
        cur_flags = character_pad.flags;
        y = camera_motion.focus_pos.y;
        camera_motion.focus_pos.y = y - ((sint32)cur_flags >= 0 ? 14336 : 32768);
        AnimatedObject::UpdateCharacterCameraTarget(&sample_pos, this);

        camera_motion.focus_pos = sample_pos;
        goto select_pos;
    }
    if ((flags & 0x2000) != 0)
        goto local_pos;
    parent = path_definition;
    if (parent == 0 || parent->transform == 0)
        goto local_pos;

    sample_pos = pos;
    PushMatrix();
    path_transform = path_definition->transform;
    SetRotMatrix(&path_transform->matrix);

    {
        VECTOR rotation_output;
        {
            VECTOR sdk_input = {sample_pos.x, sample_pos.y, sample_pos.z, 0};
            ApplyRotMatrixLV(&sdk_input, &rotation_output);
        }
        camera_motion.focus_pos.x = rotation_output.vx;
        camera_motion.focus_pos.y = rotation_output.vy;
        camera_motion.focus_pos.z = rotation_output.vz;
    }
    PopMatrix();

    path_transform = path_definition->transform;
    x = camera_motion.focus_pos.x;
    camera_motion.focus_pos.x = x + path_transform->matrix.t[0];
    path_transform = path_definition->transform;
    y = camera_motion.focus_pos.y;
    camera_motion.focus_pos.y = y + path_transform->matrix.t[1];
    path_transform = path_definition->transform;
    z = camera_motion.focus_pos.z;
    camera_motion.focus_pos.z = z + path_transform->matrix.t[2];
    goto select_pos;
local_pos:

    camera_motion.focus_pos = pos;
select_pos:

    dst = (camera_motion.smooth_ticks[0] | camera_motion.smooth_ticks[1] | camera_motion.smooth_ticks[2]) != 0 ? &camera_motion.desired_pos : &camera_motion.pos;
    if (character_pad.motion.pos_changed != 0)
        goto update_path;

    if (dst == &camera_motion.desired_pos)
        goto update_path;
    cur = camera_motion.path;
    if (cur->offset_matrix != 0)
        goto update_path;
    if ((cur->node.state_flags & 1) != 0 && (flags & 0x400) != 0)
        goto update_path;
    if ((path_flags & 0x212) == 0)
        goto compute_delta;
update_path:

    cur = camera_motion.path;
    frame = camera_motion.frame;
    cur->ProjectOffset(frame, *dst, dir);
compute_delta:
    if ((flags & 0x2000) != 0)
    {
        cur_flags = character_pad.support.speed[0];

        delta = velocity;
        if ((sint32)cur_flags < 0)
        {
            x = delta.x;
            y = delta.y;
            z = delta.z;
            delta.x = -x;
            delta.y = -y;
            delta.z = -z;
        }
    }
    else
    {
        cur = camera_motion.path;
        frame = camera_motion.frame;
        cur->SamplePos(frame, sample_pos, dir);

        frame = camera_motion.frame;
        if (frame == 0)
        {
            delta = sample_pos;
            frame = camera_motion.frame;
            cur = camera_motion.path;
            cur->SamplePos(frame + 1, sample_pos, dir);
        }
        else
        {
            cur = camera_motion.path;
            cur->SamplePos(frame - 1, delta, dir);
        }
        x = sample_pos.x - (uint32)delta.x;
        y = sample_pos.y - (uint32)delta.y;
        z = sample_pos.z - (uint32)delta.z;
        delta.x = x;
        delta.y = y;
        delta.z = z;
    }
    if ((character_pad.flags & 0x180000) != 0)
    {
        x = (uint16)yaw;
        cur_flags = flags;
        x = (x + 1024) & 0xFFF;
        yaw = x;
        orientation[2] = (x + 2048) & 0xFFF;
        flags = cur_flags | 1;
    }
    else if ((path_flags & 1) != 0)
        orientation[2] = ((uint16)yaw + 2048) & 0xFFF;
    else
    {
        x = delta.x;
        y = delta.y;
        z = delta.z;
        character_pad.path_velocity.vx = -x;
        character_pad.path_velocity.vy = -y;
        character_pad.path_velocity.vz = -z;
        TurnObjectFacing(delta, character_pad.turn_step);
    }
    if ((path_flags & 0x100) != 0)
        CameraMotion::EmptyCameraOperation();

    cur_flags = character_pad.flags;
    y = camera_motion.focus_pos.y;
    result = y + ((sint32)cur_flags >= 0 ? 14336 : 32768);
    camera_motion.focus_pos.y = result;
}

void AnimatedObject::FinishCharacterCameraTransition()
{
    const PathDefinition *parent;
    FUNCTION_MARKER(0x80045D0C, "SLUS_005.78");
    hd_forget_pose(&node);
    hd_camera_cut();

    VECTOR vector;

    uint32 x;
    uint32 y;
    uint32 z;
    sint32 result;
    PathTransform *path_transform;

    camera_motion.pos = camera_motion.desired_pos;
    parent = path_definition;
    if (parent != 0 && parent->transform != 0)
    {
        x = pos.x;
        y = pos.y;
        z = pos.z;
        vector.vx = x;
        vector.vy = y;
        vector.vz = z;
        PushMatrix();
        path_transform = path_definition->transform;
        SetRotMatrix(&path_transform->matrix);
        {
            VECTOR rotation_output;
            ApplyRotMatrixLV(&vector, &rotation_output);
            camera_motion.focus_pos.x = rotation_output.vx;
            camera_motion.focus_pos.y = rotation_output.vy;
            camera_motion.focus_pos.z = rotation_output.vz;
        }
        PopMatrix();
        path_transform = path_definition->transform;
        camera_motion.focus_pos.x = (uint32)camera_motion.focus_pos.x + path_transform->matrix.t[0];
        path_transform = path_definition->transform;
        camera_motion.focus_pos.y = (uint32)camera_motion.focus_pos.y + path_transform->matrix.t[1];
        path_transform = path_definition->transform;
        camera_motion.focus_pos.z = (uint32)camera_motion.focus_pos.z + path_transform->matrix.t[2];
    }
    else
    {
        camera_motion.focus_pos = pos;
    }
    result = character_pad.flags;
    y = camera_motion.focus_pos.y;
    camera_motion.focus_pos.y = y + (result < 0 ? 0x8000 : 0x3800);
    character_pad.motion.jump_phase = 0;
    character_pad.motion.vertical_speed = 0;
    character_pad.motion.horizontal_speed = 0;
    velocity.z = 0;
    velocity.y = 0;
    velocity.x = 0;
    movement_mode = 1;
    character_pad.flags = character_pad.flags & 0xF7FFFFFF;
}

intptr AnimatedObject::UpdateCharacterCameraTransition()
{
    const PathDefinition *parent;
    FUNCTION_MARKER(0x80045ED8, "SLUS_005.78");

    VECTOR vector;

    uint32 val, result;
    PathTransform *path_transform;
    uint32 x, y, z;
    sint32 divisor, coefficient;
    camera_motion.path->SampleOffset(camera_motion.frame, camera_motion.desired_pos, flags & 0x10);
    val = character_pad.timers.transition_ticks - 1;
    character_pad.timers.transition_ticks = val;
    if ((val & 0xFFFF) == 0)
        FinishCharacterCameraTransition();
    else
    {
        parent = path_definition;
        if (parent != 0 && parent->transform != 0)
        {
            x = pos.x;
            y = pos.y;
            z = pos.z;
            vector.vx = x;
            vector.vy = y;
            vector.vz = z;
            PushMatrix();
            path_transform = path_definition->transform;
            SetRotMatrix(&path_transform->matrix);

            {
                VECTOR rotation_output;
                ApplyRotMatrixLV(&vector, &rotation_output);
                velocity.x = rotation_output.vx;
                velocity.y = rotation_output.vy;
                velocity.z = rotation_output.vz;
            }
            PopMatrix();
            path_transform = path_definition->transform;
            val = (uint32)velocity.x + path_transform->matrix.t[0];
            velocity.x = val;
            path_transform = path_definition->transform;
            val = (uint32)velocity.y + path_transform->matrix.t[1];
            velocity.y = val;
            path_transform = path_definition->transform;
            val = (uint32)velocity.z + path_transform->matrix.t[2];
            velocity.z = val;
        }
        else
        {
            velocity = pos;
        }
        val = velocity.y;
        val += (sint32)character_pad.flags < 0 ? 0x8000 : 14336;
        velocity.y = val;
        val = velocity.x - (uint32)camera_motion.focus_pos.x;
        velocity.x = val;
        val = velocity.y - (uint32)camera_motion.focus_pos.y;
        velocity.y = val;
        val = velocity.z - (uint32)camera_motion.focus_pos.z;
        velocity.z = val;
        divisor = (sint16)character_pad.timers.transition_ticks * 4096;
        if (divisor == 0)
        {
            trap(7);
        }
        coefficient = 0x1000000 / divisor;
        val = mul12(velocity.x, coefficient);
        velocity.x = val;
        divisor = (sint16)character_pad.timers.transition_ticks * 4096;
        if (divisor == 0)
        {
            trap(7);
        }
        coefficient = 0x1000000 / divisor;
        val = mul12(velocity.y, coefficient);
        velocity.y = val;
        divisor = (sint16)character_pad.timers.transition_ticks * 4096;
        if (divisor == 0)
        {
            trap(7);
        }
        coefficient = 0x1000000 / divisor;
        val = mul12(velocity.z, coefficient);
        velocity.z = val;
        val = camera_motion.focus_pos.x + (uint32)velocity.x;
        camera_motion.focus_pos.x = val;
        val = camera_motion.focus_pos.y + (uint32)velocity.y;
        camera_motion.focus_pos.y = val;
        val = camera_motion.focus_pos.z + (uint32)velocity.z;
        camera_motion.focus_pos.z = val;
    }
    result = camera_motion.flags & 0xFE;
    camera_motion.flags = result;
    return result;
}

void AnimatedObject::ApplyPathVelocityChange(const PathDefinition *definition)
{
    FUNCTION_MARKER(0x80046228, "SLUS_005.78");
    const PathDefinition *prev_definition;
    const PathTransform *transform;
    const ScenePath *parent;
    uint32 x, y, z, val, speed, angle, cur_flags, secondary;
    const PathTransform *matrix_transform;
    sint32 kind;

    if ((flags & 0x2000) != 0)
    {
        return;
    }
    kind = contact_kind;
    parent = 0;
    if (kind == 1)
    {
        if (behavior != 33)
        {
            parent = camera_motion.path;
        }
        else
            parent = scene_path;
    }
    else if (kind == 3)
    {
        if (animation_binding.tracks[0] == -1)
            parent = scene_path;
    }
    if (parent == 0)
    {
        return;
    }
    prev_definition = parent->definition;
    matrix_transform = prev_definition->transform;
    if (matrix_transform == 0)
    {
        return;
    }
    transform = matrix_transform;
    x = transform->displacement.x;
    y = transform->displacement.y;
    z = transform->displacement.z;
    if (definition != 0)
    {
        matrix_transform = definition->transform;
        if (matrix_transform != 0)
        {
            transform = matrix_transform;
            x -= (uint32)transform->displacement.x;
            matrix_transform = definition->transform;
            transform = matrix_transform;
            y -= (uint32)transform->displacement.y;
            matrix_transform = definition->transform;
            transform = matrix_transform;
            z -= (uint32)transform->displacement.z;
        }
    }
    velocity.x = velocity.x + x;
    velocity.y = velocity.y + y;
    velocity.z = velocity.z + z;
    if (contact_kind != 1)
    {
        return;
    }
    if (x == 0 && y == 0 && z == 0)
    {
        return;
    }
    if (y != 0)
    {
        val = character_pad.motion.vertical_speed;
        val += y;
        y = 0;
        character_pad.motion.vertical_speed = val;
        flags = flags | 0x400;
    }
    val = (sint32)x >> 6;
    val *= val;
    speed = (sint32)y >> 6;
    val += speed * speed;
    speed = (sint32)z >> 6;
    val += speed * speed;
    if ((sint32)val < 0)
    {
        speed = SquareRoot12((val >> 2)) << 1;
    }
    else
    {
        speed = SquareRoot12(val);
    }

    if (speed != 0)
    {
        angle = Atan2Angle(z, x);
        val = target_yaw;
        angle = (sint16)angle - val - 1024;
        if ((angle & 0xFFF) < 2048)
        {
            if (speed != 0 && (flags & 0x400) == 0)
                speed += 4096;
        }
        else
            speed = -speed;
    }
    val = character_pad.motion.horizontal_speed;
    cur_flags = character_pad.flags;
    secondary = character_pad.secondary_flags;
    character_pad.motion.horizontal_speed = val + speed;
    character_pad.flags = cur_flags & 0xFF7FFFFF;
    character_pad.secondary_flags = secondary & 0xFFFFFF7F;
}

void AnimatedObject::UpdateObjectScenePath(const ScenePath *path, sint32 frame)
{
    FUNCTION_MARKER(0x80046510, "SLUS_005.78");
    const ScenePath *path_view, *prev;
    uint32 dir;
    uint32 same_definition;
    const PathDefinition *definition;
    sint32 prev_frame;
    prev = camera_motion.path;
    if (path != prev)
    {
        path_view = path;
        dir = flags & 0x10;
        path_view->ProjectOffset(frame, camera_motion.desired_pos, dir);
        prev = camera_motion.path;
        definition = path_view->definition;
        same_definition = definition == prev->definition;
        if (same_definition != 0)
            dir ^= 0x10;
        else
            ApplyPathVelocityChange(definition);
        if ((camera_motion.smooth_ticks[0] | camera_motion.smooth_ticks[1] | camera_motion.smooth_ticks[2]) == 0)
        {
            prev = camera_motion.path;
            if (prev == character_pad.path && (flags & 0x2000) == 0)
            {
                prev_frame = path_frame;
                if (prev_frame >= 0)
                    (prev)->ProjectOffset(prev_frame, camera_motion.pos, dir);
            }
        }
        CameraMotion::SelectScenePath(path);
    }
    path_frame = frame;
}

void AnimatedObject::BeginCharacterCameraTransition(const ScenePath *input_path, uint32 frame_word, uint32 duration_ticks, uint32 kind)
{
    FUNCTION_MARKER(0x80046628, "SLUS_005.78");

    VECTOR initial, target;

    const PathDefinition *parent, *definition;
    uint32 x, y, z, val, dir, frame;
    PathTransform *path_transform;
    const ScenePath *path, *path_view;
    uint32 cur_flags, duration, denominator;
    sint32 coefficient, sample_frame, rcp;
    if (kind == 87)
    {
        scr_render.brighten_duration = 30;
        scr_render.brighten_ticks = 15;
    }
    else if (kind == 66)
    {
        scr_render.darken_duration = 30;
        scr_render.darken_ticks = 15;
    }
    val = player_interaction.flags;
    if (val != 0)
        player_interaction.flags = val | 0x40;
    character_pad.ClearTransientFlags();
    if (character_action_flags != 0)
    {
        character_pad.interaction_phase = 0;
        character_pad.ClearInteractionActions();
    }
    if ((flags & 0x2000) == 0 && (parent = path_definition) != 0 && parent->transform != 0)
    {
        x = pos.x;
        y = pos.y;
        z = pos.z;
        initial.vx = x;
        initial.vy = y;
        initial.vz = z;
        PushMatrix();
        path_transform = path_definition->transform;
        SetRotMatrix(&path_transform->matrix);
        {
            VECTOR rotation_output;
            ApplyRotMatrixLV(&initial, &rotation_output);
            camera_motion.focus_pos.x = rotation_output.vx;
            camera_motion.focus_pos.y = rotation_output.vy;
            camera_motion.focus_pos.z = rotation_output.vz;
        }
        PopMatrix();
        path_transform = path_definition->transform;
        camera_motion.focus_pos.x = (uint32)camera_motion.focus_pos.x + path_transform->matrix.t[0];
        path_transform = path_definition->transform;
        camera_motion.focus_pos.y = (uint32)camera_motion.focus_pos.y + path_transform->matrix.t[1];
        path_transform = path_definition->transform;
        camera_motion.focus_pos.z = (uint32)camera_motion.focus_pos.z + path_transform->matrix.t[2];
    }
    else
    {
        camera_motion.focus_pos = pos;
    }
    flags = flags & 0xEFFF93EF;
    x = character_pad.motion.vertical_speed;
    y = character_pad.movement.fall_speed_limit;
    if ((sint32)y >= (sint32)x)
        character_pad.timers.state_ticks = 0;
    path = input_path;
    path_view = path;
    frame = frame_word;
    duration = duration_ticks;
    cur_flags = path_view->node.state_flags;
    dir = cur_flags & 0x10;
    if ((sint32)frame < 0)
    {
        frame = ~frame;
        if ((cur_flags & 8) != 0)
        {
            path = path_view->adjacent[0];
            path_view = path;
        }
        dir ^= 0x10;
    }
    flags = flags | 0x20000 | dir;
    if ((path_view->definition->flags & 0x11) != 0)
    {
        x = character_pad.movement.jump_ticks;
        cur_flags = character_pad.flags;
        character_pad.motion.jump_phase = -1;
        character_pad.motion.vertical_speed = 0;
        character_pad.timers.state_ticks = 0;
        character_pad.timers.jump_duration_ticks = x;
        character_pad.flags = cur_flags | 0x08000000;
        if ((character_pad.secondary_flags & 0x80) == 0)
            character_pad.motion.state_code = 6;
        flags = (flags | 0x400) & 0xF7FFFFFF;
    }
    path_definition = path_view->definition;
    coefficient = path_view->definition->frame_scale;
    if (coefficient != 0)
        frame = (sint32)(coefficient * frame) >> 12;
    camera_motion.frame = frame;
    path_frame = frame;
    yaw = 4096;
    character_pad.flags = character_pad.flags & 0xFCC5F459;
    CameraMotion::SelectScenePath(path);
    path_view->SampleOffset(frame, camera_motion.desired_pos, dir);
    definition = camera_motion.path->definition;
    if (dir != 0)
        sample_frame = camera_motion.frame;
    else
        sample_frame = (uint32)definition->pos_track.frame_count - camera_motion.frame - 1;
    AnimationTrack::SampleVectorTrack(sample_frame, &definition->pos_track, pos);
    path_frame = camera_motion.frame;
    parent = path_definition;
    if (parent != 0 && parent->transform != 0)
    {
        x = pos.x;
        y = pos.y;
        z = pos.z;
        target.vx = x;
        target.vy = y;
        target.vz = z;
        PushMatrix();
        path_transform = path_definition->transform;
        SetRotMatrix(&path_transform->matrix);
        {
            VECTOR rotation_output;
            ApplyRotMatrixLV(&target, &rotation_output);
            velocity.x = rotation_output.vx;
            velocity.y = rotation_output.vy;
            velocity.z = rotation_output.vz;
        }
        PopMatrix();
        parent = path_definition;
        path_transform = parent->transform;
        x = velocity.x;
        x += (uint32)path_transform->matrix.t[0];
        parent = path_definition;
        velocity.x = x;
        path_transform = parent->transform;
        y = velocity.y;
        y += (uint32)path_transform->matrix.t[1];
        parent = path_definition;
        velocity.y = y;
        path_transform = parent->transform;
        velocity.z = (uint32)velocity.z + path_transform->matrix.t[2];
    }
    else
    {
        velocity = pos;
    }
    velocity.x = velocity.x - (uint32)camera_motion.focus_pos.x;
    velocity.y = velocity.y - (uint32)camera_motion.focus_pos.y;
    velocity.z = velocity.z - (uint32)camera_motion.focus_pos.z;
    camera_motion.smooth_ticks[2] = duration;
    camera_motion.smooth_ticks[1] = duration;
    camera_motion.smooth_ticks[0] = duration;
    if (duration == 0)
    {
        x = (uint32)velocity.x << 1;
        y = (uint32)velocity.y << 1;
        z = (uint32)velocity.z << 1;
        world_pos.x = x;
        world_pos.y = y;
        world_pos.z = z;
        FinishCharacterCameraTransition();
        camera_motion.flags = camera_motion.flags & 0xFE;
    }
    else
    {
        character_pad.timers.transition_ticks = duration;
        x = velocity.x;
        denominator = duration << 12;
        if (denominator == 0)
            xport_mips_break(7);
        rcp = 0x01000000 / (sint32)denominator;
        x = mul12((sint32)x, rcp);
        y = velocity.y;
        velocity.x = x;
        y = mul12((sint32)y, rcp);
        z = velocity.z;
        velocity.y = y;
        velocity.z = mul12((sint32)z, rcp);
        cur_flags = character_pad.flags;
        y = camera_motion.focus_pos.y;
        camera_motion.focus_pos.y = y + ((sint32)cur_flags < 0 ? 0x8000 : 0x3800);
        node.flags = node.flags | 1;
        movement_mode = 0;
        character_pad.flags = character_pad.flags | 0x04000000;
    }
    path_definition->SampleTangent(&character_pad.path_tangent, dir);
}

void AnimatedObject::ReverseScenePath()
{
    uint32 *support_speed;
    FUNCTION_MARKER(0x80046CBC, "SLUS_005.78");
    MATRIX *matrix;
    const PathDefinition *parent;
    const ScenePath *path, *path_view, *prev;
    uint32 result, cur_flags, x, y, z, frame, old_frame, heading;
    Vec3 local, delta;
    uint32 dir;
    const PathDefinition *definition, *captured_parent, *prev_definition;
    sint32 signed_frame;

    path = camera_motion.path;
    path_view = path;
    result = path_view->node.state_flags & 8;
    if (result != 0)
    {
        path = path_view->adjacent[0];
    }
    if (path == 0)
    {
        return;
    }
    result = character_pad.secondary_flags & 0x80;
    if (result != 0)
    {
        return;
    }
    flags = flags ^ 0x10;
    support_speed = character_pad.support.speed;
    x = support_speed[0];
    y = character_pad.motion.horizontal_speed;
    support_speed[0] = -x;
    character_pad.motion.horizontal_speed = -y;
    heading = (uint16)target_yaw;
    captured_parent = path_definition;
    parent = captured_parent;
    old_frame = (uint16)path_frame;
    target_yaw = (heading + 2048) & 0xFFF;
    frame = (uint16)parent->pos_track.frame_count;
    cur_flags = flags;
    frame -= old_frame;
    if ((cur_flags & 0x2000) != 0)
    {
        x = character_pad.path_tangent.vx;
        y = character_pad.path_tangent.vy;
        z = character_pad.path_tangent.vz;
        character_pad.path_tangent.vx = -x;
        character_pad.path_tangent.vy = -y;
        character_pad.path_tangent.vz = -z;

        camera_motion.focus_pos = pos;
    }
    else if (captured_parent != 0 && parent->transform != 0)
    {
        local = pos;
        PushMatrix();
        matrix = path_definition->transform != 0 ? &path_definition->transform->matrix : NULL;
        SetRotMatrix(matrix);

        {
            VECTOR rotation_output;
            {
                VECTOR sdk_input = {local.x, local.y, local.z, 0};
                ApplyRotMatrixLV(&sdk_input, &rotation_output);
            }
            camera_motion.focus_pos.x = rotation_output.vx;
            camera_motion.focus_pos.y = rotation_output.vy;
            camera_motion.focus_pos.z = rotation_output.vz;
        }
        PopMatrix();

        matrix = path_definition->transform != 0 ? &path_definition->transform->matrix : NULL;
        x = camera_motion.focus_pos.x;
        x += matrix->t[0];
        camera_motion.focus_pos.x = x;
        matrix = path_definition->transform != 0 ? &path_definition->transform->matrix : NULL;
        y = camera_motion.focus_pos.y;
        y += matrix->t[1];
        camera_motion.focus_pos.y = y;
        matrix = path_definition->transform != 0 ? &path_definition->transform->matrix : NULL;
        z = camera_motion.focus_pos.z;
        z += matrix->t[2];
        camera_motion.focus_pos.z = z;
    }
    else
    {
        camera_motion.focus_pos = pos;
    }

    prev = camera_motion.path;
    signed_frame = (sint16)frame;
    if (path != prev)
    {
        dir = flags & 0x10;
        path->ProjectOffset(signed_frame, camera_motion.desired_pos, dir);

        prev = camera_motion.path;
        definition = path->definition;
        prev_definition = prev->definition;
        if (definition == prev_definition)
            dir ^= 0x10;
        else
            ApplyPathVelocityChange(definition);

        if ((camera_motion.smooth_ticks[0] | camera_motion.smooth_ticks[1] | camera_motion.smooth_ticks[2]) == 0)
        {
            prev = camera_motion.path;
            if (prev == character_pad.path && (flags & 0x2000) == 0)
            {
                signed_frame = path_frame;
                if (signed_frame >= 0)
                    (prev)->ProjectOffset(signed_frame, camera_motion.pos, dir);
            }
        }
        CameraMotion::SelectScenePath(path);
    }
    path_frame = frame;
    if (path != character_pad.path)
    {
        character_pad.path = path;

        x = camera_motion.desired_pos.x;
        x -= (uint32)camera_motion.pos.x;
        delta.x = x;
        y = camera_motion.desired_pos.y;
        y -= (uint32)camera_motion.pos.y;
        delta.y = y;
        z = camera_motion.desired_pos.z;
        z -= (uint32)camera_motion.pos.z;
        delta.z = z;
        CameraMotion::SetCameraTransition(delta);
    }

    camera_motion.frame = frame;
}

uint32 AnimatedObject::CanObjectChangePath(const SceneTrigger *trigger, const ScenePath *context) const
{
    FUNCTION_MARKER(0x80047058, "SLUS_005.78");
    uint32 cur_flags, mode;
    const PathDefinition *target, *other_target;
    const ScenePath *pad_path;
    const ScenePath *node;
    if (movement_mode == 2)
    {
        mode = state;
        if ((mode == 19 || (mode == 0 && path_routing.flags != 0)) && trigger != 0)
        {
            if ((flags & 0x10) != 0)
                node = trigger->parameters.path_pair.targets[0];
            else
                node = trigger->parameters.path_pair.targets[1];
            if ((node->definition->flags & 0x90000) != 0)
                return 0;
        }
    }
    cur_flags = flags;
    if ((cur_flags & 0xC00) == 0 && (path_flags & 0xA0) == 0)
        return 0;
    if (context == 0)
        return 0;
    if (trigger == 0)
        return 1;
    if ((cur_flags & 0x10) != 0)
        node = trigger->parameters.path_pair.targets[0];
    else
        node = trigger->parameters.path_pair.targets[1];
    mode = path_flags;
    if ((mode & 0x20) != 0)
    {
        const ScenePath *anchor = character_pad.path_transition.anchor_path;
        target = node->definition;
        other_target = anchor->definition;
        if (target != other_target || (mode & 0x8000) != 0)
            return 1;
        return (flags & 0x2000) != 0;
    }
    target = node->definition;
    other_target = context->definition;
    if (target != other_target)
    {
        if (contact_kind != 1)
            return 1;
        pad_path = character_pad.path;
        other_target = pad_path->definition;
        if (target != other_target)
            return 1;
    }
    return (flags & 0x2000) != 0;
}

uint32 AnimatedObject::TestCharacterSurfaceContact(const SceneTrigger *trigger, uint32 contact) const
{
    FUNCTION_MARKER(0x800471F4, "SLUS_005.78");
    const SceneTrigger *other = trigger;
    CollisionSphere bounds;
    Vec3 delta;
    uint32 extent;
    const PathDefinition *parent;
    uint32 x, y, z, result, val;
    const PathTransform *path_transform;
    if ((sint8)(other->node.state_flags >> 8) == 24 && (path_flags & 4) != 0 && character_pad.surface_interaction.prev_surface == trigger)
    {
        val = path_transition_rules.early_movement_ticks;
        if (character_pad.path_transition.movement_ticks < val)
            return 0;
    }
    if (contact != 0)
        return 1;
    if ((flags & 0x2800) != 0)
        return 1;
    x = -(uint32)other->delta.x;
    y = -(uint32)other->delta.y;
    z = -(uint32)other->delta.z;
    parent = path_definition;
    if (parent != 0)
    {
        path_transform = parent->transform;
        if (path_transform != 0 && (flags & 0x2000) == 0)
        {
            x += (uint32)path_transform->displacement.x;
            path_transform = path_definition->transform;
            y += (uint32)path_transform->displacement.y;
            path_transform = path_definition->transform;
            z += (uint32)path_transform->displacement.z;
        }
    }
    if (x != 0 || y != 0 || z != 0)
        return 1;
    extent = contact;

    bounds.pos = spheres[0].pos;
    bounds.radius = 0;
    x = velocity.x;
    x -= (uint32)other->delta.x;
    delta.x = x;
    y = velocity.y;
    y -= (uint32)other->delta.y;
    delta.y = y;
    z = velocity.z;
    z -= (uint32)other->delta.z;
    delta.z = z;
    result = AnimatedObject::SweepSphereAgainstObject(&bounds, &other->node, delta, &extent, 0) != 0;
    return result;
}

uint32 AnimatedObject::CanCharacterContact(const SceneNode *other_node) const
{
    const ScenePath *trigger_path;
    const PathDefinition *parent;
    const PathDefinition *trigger_definition, *pad_definition;
    FUNCTION_MARKER(0x80047524, "SLUS_005.78");
    const SceneNode *other = other_node;
    const ScenePath *pad_path;

    uint32 category, event, mode, cur_flags, val, x, y, z;
    const PathTransform *path_transform;
    sint32 kind;
    if ((character_pad.flags & 0x4000000) != 0)
        return 0;
    category = other->kind;
    if (category == 2)
    {
        event = other->state_flags >> 8;
        switch (event)
        {
            case 2:
            case 4:
            case 8:
            case 9:
            case 18:
            case 19:
            case 20:
            case 22:
            case 23:
            case 26:
            case 27:
            case 28:
            case 30:
                return 1;
            case 3:
                return (SceneStream::HasPendingSceneSection() & 0xFF) != 0;
            case 5:
                cur_flags = flags;
                if ((cur_flags & 0x22000000) != 0)
                    return 0;
                if ((cur_flags & 0x8000000) != 0 && ((uint16)(character_pad.motion.form_flags >> 16) & 0xF) == 4)
                    return 0;
                val = game_tick;
                return val >= ((const SceneTrigger *)other)->parameters.damage.deadline;
            case 6:
                return (flags & 0x400) != 0 && character_pad.motion.vertical_speed < 0;
            case 7:
                x = (uint32)velocity.x;
                y = velocity.y;
                z = velocity.z;
                parent = path_definition;
                if (parent != 0)
                {
                    path_transform = parent->transform;
                    if (path_transform != 0 && (flags & 0x2000) == 0)
                    {
                        x -= (uint32)path_transform->displacement.x;
                        path_transform = path_definition->transform;
                        y -= (uint32)path_transform->displacement.y;
                        path_transform = path_definition->transform;
                        z -= (uint32)path_transform->displacement.z;
                    }
                }
                trigger_path = ((const SceneTrigger *)other)->parameters.path.target;
                trigger_path = trigger_path;
                pad_path = character_pad.path;
                trigger_definition = trigger_path->definition;
                pad_definition = pad_path->definition;
                return trigger_definition != pad_definition || x != 0 || y != 0 || z != 0;
            case 10:
            case 11:
                return character_pad.timers.contact_impulse_ticks == 0;
            case 15:
                return (node.flags & 1) == 0;
            case 16:
                return (node.flags & 1) != 0;
            case 17:
                if (scr_render.transition_ticks != 0)
                    return 0;
                return sequence_playback.cur_handle != 0;
            case 21:
                val = ((const SceneTrigger *)other)->parameters.form + 1;
                if ((sint32)val < 2)
                    val = (character_pad.motion.form_flags >> 20) & 0xF;
                val = CharacterContactPool::ResolveCharacterFormVariant(val);
                return val != ((uint16)(character_pad.motion.form_flags >> 16) & 0xF);
            default:
                mode = (((const SceneTrigger *)other)->parameters.event.schedule >> 12) & 0x1F;
                if ((sint32)mode - 1 < 3 || (uint32)(other->state_flags >> 8) - 40 >= 16)
                    return 0;
                if ((other->flags & 0x80) != 0 || ((((const SceneTrigger *)other)->parameters.event.command >> 24) & 0xFE) != 0)
                    return 0;
                switch (mode)
                {
                    case 4:
                        return character_pad.secondary_flags & 0x40;
                    case 5:
                    case 6:
                    case 15:
                    case 16:
                        return 1;
                    case 7:
                        return character_pad.motion.horizontal_speed != 0;
                    case 8:
                        return (gameplay_input.buttons & 0x8000) != 0 && (flags & 0x8000000) == 0;
                    case 9:
                        return (gameplay_input.buttons & 0x2000) != 0 && (flags & 0x8000000) == 0;
                    case 10:
                        if ((sint32)character_pad.flags >= 0)
                        {
                            val = (*control_bindings.active_scheme)[1];
                            return (gameplay_input.buttons & val) != 0;
                        }
                        return (gameplay_input.buttons & 0x40) != 0 && (gameplay_input.prev_buttons & 0x40) == 0;
                    case 11:
                        if ((sint32)character_pad.flags < 0)
                            return 0;
                        return (gameplay_input.buttons >> 14) & 1;
                    case 12:
                        return (gameplay_input.buttons >> 12) & 1;
                    case 13:
                        return (gameplay_input.buttons >> 2) & 1;
                    case 14:
                        return (gameplay_input.buttons >> 3) & 1;
                    case 17:
                        val = (*control_bindings.active_scheme)[0];
                        return (gameplay_input.buttons & val) != 0;
                    case 18:
                        if (((uint16)(character_pad.motion.form_flags >> 16) & 0xF) != 1)
                            return 0;
                        return character_pad.motion.state_code == 9;
                    default:
                        return 0;
                }
        }
    }
    if (category == 0)
    {
        kind = ((const AnimatedObject *)other)->contact_kind;
        if (kind == 2)
        {
            if (((const AnimatedObject *)other)->behavior != 24)
                return 1;
            if ((((const AnimatedObject *)other)->path_definition->flags & 0x40) != 0)
                return 1;
        }
        if (kind == 3)
            return (sint8)((const AnimatedObject *)other)->animation_binding.word != -1;
        return 0;
    }
    if (category != 8)
        return 0;
    if ((character_pad.secondary_flags & 0x240) != 0)
    {
        if (character_pad.pickup_count >= 12)
            return 0;
        val = ((const BillboardNode *)other)->playback_flags & 0x7F;
        return val < 6 || val == 8;
    }
    if ((flags & 0x8000000) != 0 && ((uint16)(character_pad.motion.form_flags >> 16) & 0xF) == 4)
        return 0;
    return character_pad.pickup_count < 12;
}

sint32 AnimatedObject::ClassifyCharacterContactResponse() const
{
    CharacterController *character = &character_pad;
    FUNCTION_MARKER(0x80047B1C, "SLUS_005.78");
    uint32 cur_flags = flags;
    uint32 character_flags, val;
    sint32 timer;
    if ((cur_flags & 0x20000000) != 0 || (character->secondary_flags & 0x240) != 0)
    {
        character_flags = character->secondary_flags;
        if ((character_flags & 0x40) != 0)
            return 4;
        if ((character_flags & 0x200) != 0)
            return 5;
        timer = (sint16)character_pad.vitals.damage_ticks;
        if (timer >= 150)
            return 3;
        if (timer >= 75)
            val = timer & 7;
        else if (timer >= 37)
            val = timer & 3;
        else
            val = timer & 1;
        return val == 0 ? 2 : 3;
    }
    if ((cur_flags & 0x4000000) != 0 && ((character_pad.motion.form_flags >> 16) & 0xF) == 1)
        return -1;
    if (character_pad.motion.prev_state_code == 9)
        return ((character_pad.motion.form_flags >> 16) & 0xF) == 1 ? -1 : 0;
    return 0;
}

uint32 AnimatedObject::CollectCharacterContact(SceneNode *other_node, CollisionContact *contact_data)
{
    const ScenePath *trigger_path;
    const PathDefinition *parent;
    const PathDefinition *object_definition;
    FUNCTION_MARKER(0x80047D6C, "SLUS_005.78");
    SceneNode *other = other_node;
    CollisionTransform desc = {0};

    uint32 contact;
    uint32 category;
    uint32 event;
    uint32 cur_flags;
    uint32 val;
    uint32 mask;
    uint32 x;
    uint32 y;
    uint32 z;
    sint32 idx;
    uint32 count;
    uint32 result = 0;
    const PathTransform *path_transform;
    sint32 status, kind;
    if (contact_data == 0)
        return (sint8)CanCharacterContact(other_node);
    category = other->kind;
    contact = contact_data->fraction;
    if (category == 1)
        goto surface_desc;
    if (category == 0)
        goto ordinary_contact;
    if (category == 8)
    {
        idx = character_pad.pickup_count;
        if (idx < 12)
        {
            character_pad.pickup_objects[idx] = other_node;
            count = (uint8)character_pad.pickup_count;
            character_pad.pickup_count = count + 1;
            character_pad.pickup_fractions[(sint8)count] = contact;
        }
        goto done;
    }
    if (category != 2)
        goto done;
    CharacterContactPool::QueueContact(&character_contact_pool.cur, other_node);
    if (CharacterContactPool::ContainsContact(character_contact_pool.prev, other_node) != 0 && ((((const SceneTrigger *)other)->parameters.event.schedule >> 12) & 0x1F) == 6)
        goto done;
    event = other->state_flags >> 8;
    switch (event)
    {
        case 1:
        case 24:
            if ((TestCharacterSurfaceContact((const SceneTrigger *)other_node, contact) & 0xFF) == 0)
                goto done;
            cur_flags = character_pad.flags;
            if ((cur_flags & 1) != 0 && (sint32)contact < (sint32)character_pad.support.fraction)
                goto done;
            val = character_pad.secondary_flags;
            character_pad.flags = cur_flags | 1;
            character_pad.support.candidate = (SceneTrigger *)other_node;
            character_pad.support.fraction = contact;
            if (event == 1)
                character_pad.secondary_flags = val & 0xFFFFFFF6;
            else
            {
                character_pad.secondary_flags = (val | 1) & 0xFFFFFFF7;
                if ((path_flags & 6) != 2)
                    character_pad.surface_interaction.surface = (SceneTrigger *)other_node;
                if ((character_pad.secondary_flags & 4) != 0)
                    character_pad.surface_interaction.prev_surface_step = character_pad.surface_interaction.surface_step;
            }
            goto done;
        case 2:
            cur_flags = character_pad.flags;
            character_pad.contacts.section_trigger = (SceneTrigger *)other_node;
            character_pad.contacts.section_fraction = contact;
            character_pad.flags = cur_flags | 0x4000;
            goto done;
        case 3:
        case 17:
            val = ((const SceneTrigger *)other)->control_word;
            if (val == 87)
            {
                scr_render.brighten_duration = 30;
                scr_render.brighten_ticks = 15;
            }
            else if (val == 66)
            {
                scr_render.darken_duration = 30;
                scr_render.darken_ticks = 15;
            }
            cur_flags = character_pad.flags | 2;
            mask = 0xFCC5F45B;
            goto moving_event;
        case 4:
            cur_flags = character_pad.flags;
            character_pad.surface_interaction.environment = (SceneTrigger *)other_node;
            character_pad.motion.environment_acceleration = contact;
            character_pad.flags = cur_flags | 8;
            goto done;
        case 5:
        case 18:
            cur_flags = character_pad.flags;
            character_pad.support.contact = other_node;
            character_pad.contacts.damage_fraction = contact;
            character_pad.flags = cur_flags | 0x400;
            goto done;
        case 6:
            character_pad.flags = (character_pad.flags | 0x10) & 0xFCC5F459;
            character_pad.support.impulse_surface = (SceneTrigger *)other_node;
            val = contact_data->selection & 0x8FFF;
            goto indexed_event;
        case 7:
            cur_flags = character_pad.flags | 0x20;
            mask = 0xFCC5F479;
            goto moving_event;
        case 8:
            CutscenePlayback::QueueCutsceneTrigger((SceneTrigger *)other_node, contact);
            goto done;
        case 9:
            cur_flags = character_pad.flags;
            if (contact == 0 && (cur_flags & 0x40000) != 0)
            {
                character_pad.flags = cur_flags | 0x20000;
                goto done;
            }
            character_pad.flags = ((cur_flags | 0x20000) & 0xFCC7F459) & 0xFFFBFFFF;
            character_pad.surface_interaction.sel = other_node;
            val = contact_data->selection & 0x8FFF;
            goto indexed_event;
        case 10:
            cur_flags = character_pad.flags;
            character_pad.surface_interaction.jump_scale = (SceneTrigger *)other_node;
            character_pad.contacts.jump_scale_fraction = contact;
            character_pad.flags = cur_flags | 0x800000;
            goto done;
        case 11:
            cur_flags = character_pad.flags;
            val = character_pad.secondary_flags;
            character_pad.surface_interaction.jump_scale = (SceneTrigger *)other_node;
            character_pad.contacts.jump_scale_fraction = contact;
            character_pad.flags = cur_flags | 0x800000;
            character_pad.secondary_flags = val | 0x80;
            goto done;
        case 15:
            cur_flags = character_pad.flags;
            character_pad.contacts.hide_trigger = (SceneTrigger *)other_node;
            character_pad.contacts.hide_fraction = contact;
            character_pad.flags = cur_flags | 0x1000000;
            goto done;
        case 16:
            cur_flags = character_pad.flags;
            character_pad.contacts.show_trigger = (SceneTrigger *)other_node;
            character_pad.contacts.show_fraction = contact;
            character_pad.flags = cur_flags | 0x2000000;
            goto done;
        case 19:
            if (contact != 0)
                goto queue_event;
            val = ((const SceneTrigger *)other)->parameters.far_distance;
            camera_proj.far_distance_copy = val;
            camera_proj.far_distance = val;
            goto done;
        case 20:
            if (contact != 0)
                goto queue_event;
            character_pad.motion.trail_effect = ((const SceneTrigger *)other)->parameters.trail_effect;
            goto done;
        case 21:
            if (contact != 0)
                goto queue_event;
            val = ((const SceneTrigger *)other)->parameters.form + 1;
            if ((sint32)val < 2)
                val = (character_pad.motion.form_flags >> 20) & 0xF;
            val = CharacterContactPool::ResolveCharacterFormVariant(val) << 24;
            character_pad.motion.form_flags = (character_pad.motion.form_flags & 0xF0FFFFFF) | val;
            goto done;
        case 22:
            if (contact == 0)
                flags = flags | 0x40000;
            goto done;
        case 23:
            if (contact != 0)
                goto queue_event;
            node.state_flags = ((const SceneTrigger *)other)->parameters.state_flags;
            goto done;
        case 26:
        case 27:
            if (character_pad.path_transition.reentry_ticks != 0)
                goto done;
            cur_flags = path_flags;
            if ((cur_flags & 0x80) != 0 || (character_pad.flags & 0x60000) != 0)
                goto done;
            if (((flags & 0x400) == 0 || character_pad.path_transition.movement_ticks < 10) && (cur_flags & 1) == 0)
                goto done;
            trigger_path = ((const SceneTrigger *)other)->parameters.path.target;
            trigger_path = trigger_path;
            parent = trigger_path->definition;
            object_definition = path_definition;
            if (parent == object_definition && (flags & 0x2000) == 0)
                goto done;
            cur_flags = path_flags;
            if (event == 26)
                cur_flags = (cur_flags | 0x80) & 0xFFFFFBFF;
            else
                cur_flags |= 0x480;
            path_flags = cur_flags;
            character_pad.path_transition.anchor = (SceneTrigger *)other_node;
            character_pad.path_transition.anchor_fraction = contact;
            character_pad.path_transition.phase = 0;
            goto done;
        case 28:
            goto queue_event;
        case 30:
        {
            cur_flags = character_pad.secondary_flags | 0x400;
            character_pad.secondary_flags = cur_flags;
            val = ((const SceneTrigger *)other)->parameters.animation;
            const CharacterModelRecord *event_model = character_pad.model;
            AnimationHeader *active_animation = animation;
            if (active_animation == event_model->animations[val])
                frame = 0;
            else if ((cur_flags & 0x80) == 0)
                character_pad.motion.state_code = val;
            goto done;
        }
        default:
            if (contact != 0)
                goto queue_event;
            other->state_flags = (other->state_flags & 0xFF00) | (uint8)((other->state_flags & 0xFF) | 8);
            goto done;
    }
moving_event:
    character_pad.flags = cur_flags & mask;
    character_pad.surface_interaction.sel = other_node;
    x = ((const SceneTrigger *)other)->delta.x;
    y = ((const SceneTrigger *)other)->delta.y;
    z = ((const SceneTrigger *)other)->delta.z;
    character_pad.support.motion.x = x;
    character_pad.support.motion.y = y;
    character_pad.support.motion.z = z;
    result = 1;
    goto done;
indexed_event:
    character_pad.support.contact_dir = val;
    x = ((const SceneTrigger *)other)->delta.x;
    y = ((const SceneTrigger *)other)->delta.y;
    z = ((const SceneTrigger *)other)->delta.z;
    character_pad.support.motion.x = x;
    character_pad.support.motion.y = y;
    character_pad.support.motion.z = z;
    result = 1;
    goto done;
queue_event:
    idx = character_pad.trigger_count;
    if (idx < 12)
    {
        character_pad.trigger_objects[idx] = other_node;
        count = (uint8)character_pad.trigger_count;
        character_pad.trigger_count = count + 1;
        character_pad.trigger_fractions[(sint8)count] = contact;
    }
    goto done;
surface_desc:
    SceneNode::SelectColFace(other_node, contact_data->selection & 0xFFFF8FFF, &desc);
    if ((AcceptVerticalFaceContact(&desc, contact_data->selection) & 0xFF) != 0)
        goto done;
    character_pad.flags = (character_pad.flags | 4) & 0xFCC5F45D;
    character_pad.surface_interaction.sel = other_node;
    surface_contact = contact_data->selection;

    character_pad.support.motion = desc.motion;
    result = 1;
    goto done;
ordinary_contact:
    cur_flags = ((const AnimatedObject *)other)->flags;
    if ((cur_flags & 0x100000) != 0)
        goto surface_desc;
    if ((cur_flags & 0x200) != 0 && ((const AnimatedObject *)other)->contact_kind == 2)
        goto done;
    if (((const AnimatedObject *)other)->behavior == 17)
        goto moving_platform;
    kind = ((const AnimatedObject *)other)->contact_kind;
    if (kind == 2)
    {
        if ((((const AnimatedObject *)other)->flags & 0x30000000) != 0)
            goto moving_platform;
        kind = ((const AnimatedObject *)other)->contact_kind;
    }
    if (kind == 3 && ((const AnimatedObject *)other)->state == 30)
    {
        character_pad.motion.slope_scale = 4096;
        goto done;
    }
    status = ClassifyCharacterContactResponse();
    kind = ((const AnimatedObject *)other)->contact_kind;
    if (kind != 2)
        goto fallback;
    if (status == 0 && (character_pad.secondary_flags & 0x100) != 0 && character_action_target == (const AnimatedObject *)other_node && (character_action_flags & 8) != 0)
    {
        status = -1;
        kind = ((const AnimatedObject *)other)->contact_kind;
    }
    if (kind != 2)
        goto fallback;
    if (status == 0 && (path_flags & 1) != 0)
        status = ClassifyConstrainedContact((const AnimatedObject *)other);
    if (((const AnimatedObject *)other)->contact_kind != 2)
        goto fallback;
    if (status == 0 && ((const AnimatedObject *)other)->ReadObjectImpulseScale() != 0)
        status = -1;
    if (contact == 0 && (flags & 0x2000000) != 0)
        goto fallback;
    if ((contact_data->flags & 1) == 0 && (((const AnimatedObject *)other)->behavior != 20 || (((const AnimatedObject *)other)->flags & 0x40) != 0))
        goto fallback;
    cur_flags = flags;
    if ((cur_flags & 0x8000000) != 0 && ((uint16)(character_pad.motion.form_flags >> 16) & 0xF) == 4)
        goto fallback;
    if (status > 0)
        goto slide_contact;
    if (contact_data->other_selection < 0x800 && status == 0)
        goto attach_contact;
    if ((cur_flags & 0x10000000) != 0 || (((const AnimatedObject *)other)->flags & 0x4400000) == 0x400000)
        goto fallback;
slide_contact:
    x = (uint32)((const AnimatedObject *)other)->velocity.x;
    y = ((const AnimatedObject *)other)->velocity.y;
    z = ((const AnimatedObject *)other)->velocity.z;
    character_pad.support.motion.x = x;
    character_pad.support.motion.y = y;
    character_pad.support.motion.z = z;
    if ((((const AnimatedObject *)other)->flags & 0x2000) == 0)
    {
        parent = ((const AnimatedObject *)other)->path_definition;
        if (parent != 0)
        {
            path_transform = parent->transform;
            if (path_transform != 0)
            {
                character_pad.support.motion.x = character_pad.support.motion.x + (uint32)path_transform->displacement.x;
                path_transform = ((const AnimatedObject *)other)->path_definition->transform;
                character_pad.support.motion.y = character_pad.support.motion.y + (uint32)path_transform->displacement.y;
                path_transform = ((const AnimatedObject *)other)->path_definition->transform;
                character_pad.support.motion.z = character_pad.support.motion.z + (uint32)path_transform->displacement.z;
            }
        }
    }
    if ((flags & 0x400) != 0)
    {
        character_pad.flags = (character_pad.flags | 0x800) & 0xFCC5FC59;
        character_pad.support.impulse_scale = ((const AnimatedObject *)other)->ReadObjectImpulseScale();
        result = contact != 0;
        goto done;
    }
    if (((const AnimatedObject *)other)->ReadObjectImpulseScale() == 0)
    {
        if (character_pad.path_transition.contact_delay <= 0)
            AnimatedObject::QueueCharacterActionSound(7, 3);
        if (status >= 0 || ((const AnimatedObject *)other)->ticks == 1)
        {
            result = status == 0;
            goto done;
        }
    }
    contact_data->flags = contact_data->flags & 0xFFFFFFFD;
fallback:
    if (status > 0)
        goto done;
attach_contact:
    if ((contact_data->flags & 2) == 0)
        goto ordinary_attachment;
    cur_flags = flags;
    if ((cur_flags & 0x2000000) != 0 || ((cur_flags & 0x8000000) != 0 && ((uint16)(character_pad.motion.form_flags >> 16) & 0xF) == 4))
        goto ordinary_attachment;
    character_pad.flags = (character_pad.flags | 0x400) & 0xFCC5F459;
    character_pad.support.contact = other_node;
    kind = ((const AnimatedObject *)other)->contact_kind;
    if (kind == 2)
        character_pad.vitals.damage_amount = ((const AnimatedObject *)other)->GetActorContactDamage();
    else if (kind == 3)
        character_pad.vitals.damage_amount = ModelGroupRegistry::GetModelGrpContactType(((const AnimatedObject *)other)->state);
    goto copy_attachment;
ordinary_attachment:
    cur_flags = character_pad.flags;
    if ((cur_flags & 0x40000) != 0 || (flags & 0x2000000) != 0)
        goto done;
    character_pad.flags = ((cur_flags | 0x20000) & 0xFCC7F459) & 0xFFFBFFFF;
    character_pad.support.contact = other_node;
copy_attachment:
    x = (uint32)((const AnimatedObject *)other)->velocity.x;
    y = ((const AnimatedObject *)other)->velocity.y;
    z = ((const AnimatedObject *)other)->velocity.z;
    character_pad.support.motion.x = x;
    character_pad.support.motion.y = y;
    character_pad.support.motion.z = z;
    parent = ((const AnimatedObject *)other)->path_definition;
    if (parent != 0)
    {
        path_transform = parent->transform;
        if (path_transform != 0 && (((const AnimatedObject *)other)->flags & 0x2000) == 0)
        {
            character_pad.support.motion.x = character_pad.support.motion.x + (uint32)path_transform->displacement.x;
            path_transform = ((const AnimatedObject *)other)->path_definition->transform;
            character_pad.support.motion.y = character_pad.support.motion.y + (uint32)path_transform->displacement.y;
            path_transform = ((const AnimatedObject *)other)->path_definition->transform;
            character_pad.support.motion.z = character_pad.support.motion.z + (uint32)path_transform->displacement.z;
        }
    }
    result = ((const AnimatedObject *)other)->contact_kind != 3;
    goto done;
moving_platform:
    if ((flags & 0x400) == 0)
        goto done;
    character_pad.flags = (character_pad.flags | 0x200800) & 0xFCE5FC59;
    path_transform = ((const AnimatedObject *)other)->path_definition->transform;
    result = 1;
    if (path_transform != 0)
    {
        character_pad.support.motion.x = character_pad.support.motion.x + (uint32)path_transform->displacement.x;
        path_transform = ((const AnimatedObject *)other)->path_definition->transform;
        character_pad.support.motion.y = character_pad.support.motion.y + (uint32)path_transform->displacement.y;
        path_transform = ((const AnimatedObject *)other)->path_definition->transform;
        character_pad.support.motion.z = character_pad.support.motion.z + (uint32)path_transform->displacement.z;
    }
done:
    return result;
}

uint32 AnimatedObject::ResetCharacterForScene(sint32 mode)
{
    CharacterModelRecord *model_rec;
    AnimatedObject *model_object;
    SpriteRecord *spr;
    FUNCTION_MARKER(0x80048AF0, "SLUS_005.78");
    hd_forget_pose(&node);
    hd_camera_cut();
    uint32 cur_flags, val, packed, category, result = 1;
    sint32 sel;
    HudState::ResetHud();
    category = (character_pad.motion.form_flags >> 16) & 0xF;
    character_pad.motion.slope_scale = 0;
    {
        ObjectCallback operation = {0, category == 5 ? &AnimatedObject::UpdateForm5Movement : &AnimatedObject::UpdateCharacterMovement};
        object_operations.motions[1] = operation;
    }
    {
        ObjectCallback operation = {0, &AnimatedObject::UpdateCharacterBehavior};
        object_operations.behaviors[1] = operation;
    }
    model_rec = character_pad.model;
    character_pad.motion.trail_effect = 0;
    model_object = model_rec->object;
    spr = model_object->spr;
    spr->extent[0] = model_rec->extent[0] >> 5;
    model_rec = character_pad.model;
    model_object = model_rec->object;
    spr = model_object->spr;
    spr->extent[1] = model_rec->extent[1] >> 5;
    val = game_flow.transition_flags;
    cur_flags = character_pad.flags;
    game_flow.transition_flags = val & 0xFFFFFEFF;
    character_pad.flags = cur_flags & 0xBFFFFFFF;
    character_pad.ClearTransientFlags();
    character_pad.flags = character_pad.flags & 0xC0000000;
    cur_flags = flags;
    val = path_flags;
    flags = cur_flags & 0xC0F793FF;
    path_flags = val & 0xFC000000;
    cur_flags = character_pad.secondary_flags;
    val = game_flow.mode_flags;
    character_pad.motion.jump_phase = 0;
    character_pad.motion.vertical_speed = 0;
    character_pad.motion.horizontal_speed = 0;
    character_pad.secondary_flags = cur_flags & 0xFFFFF932;
    velocity.z = 0;
    velocity.y = 0;
    velocity.x = 0;
    character_pad.timers.special_move_ticks = 0;
    character_pad.timers.transition_ticks = 0;
    character_pad.timers.score_bonus_ticks = 0;
    character_pad.timers.action_cooldown_ticks = 0;
    character_pad.vitals.damage_ticks = 0;
    character_pad.timers.state_ticks = 0;
    character_pad.section_height_offset = 0;
    character_pad.timers.contact_impulse_ticks = 0;
    character_pad.motion.environment_acceleration = 0;
    if ((val & 0x10) == 0 && mode != 0)
    {
        character_pad.weapon_state = 0;
    }
    if (mode == 2)
    {
        character_pad.vitals.lives = 3;
        character_pad.vitals.health_limit = 4;
        character_pad.vitals.health = 4;
        character_pad.vitals.score = 0;
        if ((character_pad.secondary_flags & 0x80) == 0)
            character_pad.motion.state_code = 0;
        result = game_flow.transition_flags | 0x20000;
        game_flow.transition_flags = result;
    }
    else if (mode == 1)
    {
        val = SceneSection::GetSectionRespawnForm(scene_stream.active_section);
        character_pad.vitals.health = val;
        sel = (sint8)val;
        if (sel == -1)
        {
            packed = character_pad.motion.form_flags & 0xFFF0FFFF;
            category = ((packed >> 20) & 0xF) << 16;
        }
        else
        {
            val = CharacterContactPool::ResolveCharacterFormVariant(sel);
            character_pad.vitals.health = val;
            packed = character_pad.motion.form_flags;
            sel = (sint8)character_pad.vitals.health;
            packed &= 0xFFF0FFFF;
            category = (uint32)sel << 16;
            character_pad.motion.form_flags = packed;
        }
        character_pad.motion.form_flags = packed | category;
        flags = flags | 0x10;
        character_pad.vitals.health = character_pad.vitals.health_limit;
        if ((character_pad.secondary_flags & 0x80) == 0)
            character_pad.motion.state_code = 0;
        SceneStream::SetSceneTransition(0, scene_stream.active_section, -2, 0);
        result = 15;
        scr_render.transition_ticks = result;
    }
    return result;
}

intptr AnimatedObject::UpdateCharacterDeathTransition()
{
    FUNCTION_MARKER(0x80048DB4, "SLUS_005.78");
    ActorBinding *binding;
    const AnimationHeader *clip;
    uint32 val, scene, fade, sample_frame, buttons, result;
    sint32 step, sample_pos, limit;
    if (player_interaction.flags != 0)
    {
        player_interaction.flags = 0;
        player_interaction.target = 0;
    }
    if (character_action_flags != 0)
    {
        AnimatedObject *sel = character_action_target;
        character_action_flags = 0;
        if (sel != 0)
        {
            binding = (ActorBinding *)(((const ActorPlacement *)sel->path_definition)->state);
            val = binding->flags;
            binding->flags = val & 0xF7;
        }
    }
    if (((character_pad.motion.form_flags >> 16) & 15) == 1)
        character_pad.secondary_flags = character_pad.secondary_flags & 0xFFFFFFDF;
    else if ((game_flow.mode_flags & 0x200) == 0)
        character_pad.secondary_flags = character_pad.secondary_flags & 0xFFFFFEFF;

    camera_motion.smooth_ticks[2] = 0;
    camera_motion.smooth_ticks[1] = 0;
    camera_motion.smooth_ticks[0] = 0;

    camera_motion.desired_pos = camera_motion.pos;
    if (animation == 0)
    {
        val = character_pad.timers.state_ticks - 1;
        character_pad.timers.state_ticks = val;
        if ((uint16)val != 0)
            return 0xE0000;
        camera_proj.scale = 0xE9555;
        CameraProjection::SetCameraProj(0xE9555);
        return ResetCharacterForScene(1);
    }
    fade = scr_render.fade_level;
    if (fade < 4096)
    {
        fade += 409;
        scr_render.fade_level = fade;
        if ((uint16)fade >= 4096)
        {
            scr_render.fade_level = 4096;
            SceneStream::ReleaseSceneObjects();
            node.Register();
        }
    }
    if ((character_pad.flags & 0x80) != 0)
    {
        if (scr_render.fade_level < 4096)
        {
            val = pos.y;

            pos.y = val - 10240;

            camera_motion.focus_pos = pos;
            CameraMotion::UpdateCameraAim();

            camera_motion.flags = camera_motion.flags | 1;
        }
        if (character_pad.timers.state_ticks != 0)
        {
            step = character_pad.path_velocity.vz;
            if (step > 0)
            {
                sample_pos = camera_proj.scale;
                if (sample_pos <= 2457599)
                {
                    val = (uint32)sample_pos + step;
                    camera_proj.scale = val;
                    if ((sint32)val > 2457600)
                        camera_proj.scale = 2457600;
                    CameraProjection::SetCameraProj(camera_proj.scale);
                }
                if (camera_proj.scale == 2457600)
                    character_pad.path_velocity.vz = -(uint32)character_pad.path_velocity.vz;
            }
            else
            {
                sample_pos = camera_proj.scale;
                if (sample_pos > 0)
                {
                    val = (uint32)sample_pos + step;
                    camera_proj.scale = val;
                    if ((sint32)val < 0)
                        camera_proj.scale = 0;
                    CameraProjection::SetCameraProj(camera_proj.scale);
                }
            }
            character_pad.timers.state_ticks = character_pad.timers.state_ticks - 1;
            clip = animation;
            sample_frame = 0;
            if (clip != 0)
            {
                sample_pos = frame;
                limit = clip->frame_count - 1;
                if (sample_pos < limit)
                    sample_frame = (uint32)sample_pos + 1;
            }
            frame = sample_frame;
            return ClampAnimationFrame();
        }
        character_pad.flags = character_pad.flags & 0xFFFFFF7F;
    }
    else
    {
        sample_pos = camera_proj.scale;
        if (sample_pos <= 2457599)
        {
            val = (uint32)sample_pos + 40960;
            camera_proj.scale = val;
            if ((sint32)val > 2457600)
                camera_proj.scale = 2457600;
            CameraProjection::SetCameraProj(camera_proj.scale);
        }
        sample_frame = (uint16)frame;
        clip = animation;
        ++sample_frame;
        frame = sample_frame;
        limit = clip->frame_count;
        if ((sint16)sample_frame < limit)
            return ClampAnimationFrame();
    }
    animation = 0;
    SequencePlayback::StopCurSequence();
    result = SequencePlayback::PrepareSavedSequence();
    buttons = game_flow.transition_flags;
    sequence_playback.cur_handle = result;
    if ((buttons & 0x10000) != 0)
    {
        val = level_resource.level_id;
        if (val - 96 >= 2)
        {
            val = character_pad.vitals.lives - 1;
            character_pad.vitals.lives = val;
            if ((uint8)val == 0)
                return ResetCharacterForScene(2);
        }
    }
    SequencePlayback::StartCurSequence();
    scene = OriginalDefaults::scene_context_identity;
    character_pad.timers.state_ticks = 15;
    scr_render.transition_ticks = -15;
    return scene;
}

void AnimatedObject::UpdateCharacterHorizontalSpeed(uint32 decelerate)
{
    CharacterModelRecord *parent_rec;
    FUNCTION_MARKER(0x8004922C, "SLUS_005.78");
    uint32 cur_flags;
    uint32 object_flags;
    uint32 speed;
    uint32 cap;
    sint32 minimum;
    uint32 acceleration = 0;
    uint32 slope = 0;
    uint32 x;
    uint32 y;
    uint32 z;
    uint32 len;
    uint32 angle;
    sint32 folded;
    uint32 val;
    uint32 proposed;
    uint32 result;
    object_flags = flags;
    speed = character_pad.motion.horizontal_speed;
    decelerate &= 0xFF;
    if ((object_flags & 0x400) != 0)
        goto ground_speed;
    cur_flags = path_flags;
    if ((cur_flags & 0x1024) != 0)
    {
    ground_speed:
        /* Braking uses the same ground speed limit as acceleration */
        cap = character_pad.movement.speed_limits[3];
        if (decelerate != 0)
        {
            if ((character_pad.flags & 0x20000000) == 0)
            {
                if ((sint32)speed < 0)
                    acceleration = character_pad.movement.braking0;
                else
                    acceleration = -(uint32)character_pad.movement.braking0;
            }
        }
        else
        {
            minimum = character_pad.movement.speed_limits[2];
            acceleration = character_pad.movement.acceleration0;
            if ((sint32)speed < 0)
                acceleration += (uint32)character_pad.movement.braking0;
        }
        goto apply_speed;
    }
    cap = character_pad.movement.speed_limits[1];
    if ((cur_flags & 0x10) != 0)
    {
        parent_rec = character_pad.model;
        cap = mul12((sint32)cap, parent_rec->motion_scale.z);
    }
    else if ((cur_flags & 1) != 0)
    {
        parent_rec = character_pad.model;
        cap = mul12((sint32)cap, parent_rec->motion_scale.y);
    }
    else if ((cur_flags & 4) != 0)
        cap = mul12((sint32)cap, path_transition_rules.axis_speed_limit_scale);
    if (decelerate != 0)
    {
        if ((sint32)speed <= 0)
            acceleration = character_pad.movement.braking1;
        else
            acceleration = -(uint32)character_pad.movement.braking1;
    }
    else
    {
        minimum = character_pad.movement.speed_limits[0];
        acceleration = character_pad.movement.acceleration1;
        if ((sint32)speed < 0)
            acceleration += (uint32)character_pad.movement.braking1;
    }
    if (character_pad.motion.slope_scale != 0 && (path_flags & 0x1440100) == 0)
    {
        x = character_pad.path_velocity.vx >> 6;
        y = character_pad.path_velocity.vy >> 6;
        z = character_pad.path_velocity.vz >> 6;
        val = x * x + y * y + z * z;
        if ((sint32)val < 0)
            len = SquareRoot12((val >> 2)) << 1;
        else
            len = SquareRoot12(val);
        angle = (-AsinAngle(character_pad.path_velocity.vy, len)) & 0xFFF;
        if (angle != 0)
        {
            folded = angle;
            if (angle >= 1025)
                folded = 4096 - angle;
            if ((sint32)character_pad.motion.slope_scale >= folded)
            {
                val = SinQ14(angle);
                slope = -(uint32)mul12(1024, ((sint32)val >> 2));
            }
        }
        if (decelerate != 0)
        {
            val = 4096 - character_pad.motion.slope_scale;
            acceleration = mul12((sint32)acceleration, (sint32)val);
        }
        else if ((sint32)slope < 0 && (sint32)cap < (sint32)speed)
            acceleration = 0;
        val = 15360 - cap;
        val = mul12((sint32)val, (sint32)character_pad.motion.slope_scale);
        cap += val;
        if (decelerate == 0)
        {
            val = 4096 - character_pad.motion.slope_scale;
            minimum = mul12(minimum, (sint32)val);
        }
        else if ((sint32)speed < 0)
            cap = -cap;
        else if (speed == 0 && (sint32)(acceleration ^ slope) < 0)
            acceleration = -acceleration;
    }
apply_speed:
    if (decelerate != 0)
    {
        if (speed != 0)
        {
            acceleration += slope;
            proposed = speed + acceleration;
            if ((sint32)(speed ^ proposed) < 0)
                speed = 0;
            else
            {
                speed = proposed;
                if ((flags & 0x400) == 0)
                {
                    if ((sint32)proposed < 0 && (sint32)cap < 0 && (sint32)proposed < (sint32)cap)
                        speed = cap;
                    else if ((sint32)proposed > 0 && (sint32)cap > 0 && (sint32)cap < (sint32)proposed)
                        speed = cap;
                }
            }
        }
        else
        {
            proposed = slope - acceleration;
            if (slope != 0 && (sint32)(slope ^ proposed) >= 0)
                speed = proposed;
        }
    }
    else
    {
        acceleration += slope;
        if ((sint32)speed < 0)
            speed += acceleration;
        else if ((sint32)speed < minimum)
            speed = minimum;
        else if ((sint32)speed < (sint32)cap)
        {
            speed += acceleration;
            if ((sint32)cap < (sint32)speed)
                speed = cap;
        }
        else if ((flags & 0x400) == 0)
            speed = cap;
    }
    result = character_pad.flags & 0x800000;
    character_pad.motion.horizontal_speed = speed;
    if (result != 0)
    {
        const SceneTrigger *jump_scale = character_pad.surface_interaction.jump_scale;
        val = jump_scale->parameters.jump.motion_scale;
        speed = mul12((sint32)speed, (sint32)val);
        jump_scale = character_pad.surface_interaction.jump_scale;
        character_pad.motion.horizontal_speed = speed;
        val = jump_scale->parameters.jump.motion_scale;
        result = character_pad.motion.vertical_speed;
        character_pad.motion.vertical_speed = mul12((sint32)result, (sint32)val);
    }
}

void AnimatedObject::ApplyEnvironmentMotion(uint32 *horizontal, uint32 *vertical)
{
    FUNCTION_MARKER(0x80049730, "SLUS_005.78");
    uint32 val, result, acceleration, input, sum, cur_flags, upper, lower;
    if ((character_pad.flags & 0x80000) != 0)
    {
        val = *vertical + 101;
        *vertical = val;
        if ((sint32)val > 0)
            flags = flags | 0x400;
        val = character_pad.movement.jump_ticks;
        character_pad.motion.jump_phase = 1;
        character_pad.timers.state_ticks = val;
        *horizontal = 0;
        character_pad.motion.horizontal_speed = 0;
        character_pad.motion.vertical_speed = *vertical;
        return;
    }
    acceleration = character_pad.surface_interaction.environment->parameters.motion.vertical_accel;
    if (acceleration != 0)
    {
        input = gameplay_input.buttons;
        if ((input & 0x1000) != 0)
            acceleration += 40;
        else if ((input & 0x4000) != 0)
            acceleration -= 40;
        sum = *vertical + acceleration;
        *vertical = sum;
        if ((sint32)acceleration > 0)
        {
            if ((sint32)sum > 0)
            {
                cur_flags = flags;
                if ((cur_flags & 0x400) == 0)
                {
                    flags = cur_flags | 0x400;
                    if ((character_pad.secondary_flags & 0x80) == 0)
                        character_pad.motion.state_code = 3;
                }
            }
            val = character_pad.movement.jump_ticks;
            character_pad.motion.jump_phase = 1;
            character_pad.timers.state_ticks = val;
        }
        else if ((flags & 0x400) != 0 && (sint32)sum < 0 && character_pad.motion.jump_phase >= 0)
            character_pad.motion.jump_phase = -1;
        character_pad.motion.environment_acceleration = 0;
        character_pad.motion.vertical_speed = *vertical;
    }
    acceleration = character_pad.surface_interaction.environment->parameters.motion.horizontal_accel;
    if (acceleration == 0)
        return;
    if ((flags & 0x10) == 0)
        acceleration = -acceleration;
    val = character_pad.surface_interaction.environment->parameters.motion.horizontal_accel;
    character_pad.motion.environment_acceleration = val;
    *horizontal = *horizontal + acceleration;
    if ((flags & 0x400) != 0)
        val = character_pad.movement.speed_limits[3];
    else
        val = character_pad.movement.speed_limits[1];
    lower = -val;
    sum = *horizontal;
    upper = val + acceleration;
    result = (sint32)upper < (sint32)sum;
    lower += acceleration;
    if (result != 0)
        *horizontal = upper;
    else
    {
        result = (sint32)sum < (sint32)lower;
        if (result != 0)
            *horizontal = lower;
    }
}

void AnimatedObject::ApplyDirectionalImpulse(uint32 *move_flags, uint32 mode, const Vec3 &dir_vector, sint32 strength, sint32 limit)
{
    FUNCTION_MARKER(0x80049928, "SLUS_005.78");
    const PathDefinition *parent;
    uint32 val;
    uint32 x;
    uint32 y;
    uint32 z;
    uint32 dot;
    uint32 rx;
    uint32 ry;
    uint32 rz;
    uint32 nx;
    uint32 ny;
    uint32 nz;
    uint32 speed;
    sint32 coefficient;
    const PathTransform *path_transform;
    uint32 cur_flags;
    sint32 vertical;
    uint32 minimum;
    uint32 angle;

    Vec3 vector;
    sint64 product;
    val = AsinAngle(dir_vector.y, 4096);
    cur_flags = character_pad.flags;
    if ((val & 0x3FF) - 64 < 0x380)
        cur_flags |= 0x20000000;
    else
        cur_flags &= 0xDFFFFFFF;
    character_pad.flags = cur_flags;
    x = velocity.x;
    y = velocity.y;
    z = velocity.z;
    if ((*move_flags & 8) == 0)
    {
        x = mul12((sint32)x, 3276);
        y = mul12((sint32)y, 3276);
        z = mul12((sint32)z, 3276);
    }
    parent = path_definition;
    if (parent != 0)
    {
        path_transform = parent->transform;
        if (path_transform != 0 && (flags & 0x2000) == 0)
        {
            x += (uint32)path_transform->displacement.x;
            path_transform = path_definition->transform;
            y += (uint32)path_transform->displacement.y;
            path_transform = path_definition->transform;
            z += (uint32)path_transform->displacement.z;
        }
    }
    x -= (uint32)character_pad.support.motion.x;
    y -= (uint32)character_pad.support.motion.y;
    z -= (uint32)character_pad.support.motion.z;
    dot = mul12((sint32)x, dir_vector.x);
    dot += (uint32)mul12((sint32)y, dir_vector.y);
    dot += (uint32)mul12((sint32)z, dir_vector.z);
    dot = -(dot << 1);
    rx = mul12(dir_vector.x, (sint32)dot);
    ry = mul12(dir_vector.y, (sint32)dot);
    rz = mul12(dir_vector.z, (sint32)dot);
    rx += x;
    ry += y;
    rz += z;
    nx = mul12(dir_vector.x, strength);
    ny = mul12(dir_vector.y, strength);
    nz = mul12(dir_vector.z, strength);
    rx += nx;
    ry += ny;
    rz += nz;
    rx += (uint32)character_pad.support.motion.x;
    ry += (uint32)character_pad.support.motion.y;
    rz += (uint32)character_pad.support.motion.z;
    x = (sint32)rx >> 6;
    y = (sint32)ry >> 6;
    z = (sint32)rz >> 6;
    val = x * x + y * y + z * z;
    if ((sint32)val < 0)
        speed = SquareRoot12((val >> 2)) << 1;
    else
        speed = SquareRoot12(val);
    if (limit < (sint32)speed)
    {
        if (mode == 0xFFFFFFFF && ry + 12288 >= 0x6001)
        {
            val = (sint32)ry < 0 ? -ry : ry;
            if (val == 0)
                xport_mips_break(7);
            coefficient = 0x40000000 / (sint32)val;
            product = (sint64)12288 * coefficient;
            coefficient = (uint64)product >> 18;
            fixed_math_result = coefficient;
            rx = mul12((sint32)rx, coefficient);
            ry = mul12((sint32)ry, coefficient);
            rz = mul12((sint32)rz, coefficient);
        }
        if (speed == 0)
            xport_mips_break(7);
        coefficient = 0x40000000 / (sint32)speed;
        product = (sint64)limit * coefficient;
        coefficient = (uint64)product >> 18;
        fixed_math_result = coefficient;
        rx = mul12((sint32)rx, coefficient);
        ry = mul12((sint32)ry, coefficient);
        rz = mul12((sint32)rz, coefficient);
    }
    *move_flags = *move_flags & 0xFFFFFFF7;
    val = (uint16)(character_pad.motion.form_flags >> 16) & 15;
    character_pad.timers.state_ticks = 0;
    character_pad.motion.jump_phase = 0;
    if (val != 5 && (character_pad.secondary_flags & 0x80) == 0)
        character_pad.motion.state_code = 3;
    character_pad.motion.vertical_speed = ry;
    character_pad.motion.jump_phase = (sint32)ry < 0 ? 0xFFFFFFFF : 1;
    ry = 0;
    if (((uint16)(character_pad.motion.form_flags >> 16) & 15) != 4 && ((flags & 0x400) != 0 || (path_flags & 4) != 0))
    {
        vertical = character_pad.motion.vertical_speed;
        minimum = min_impulse_vertical_speed;
        if (vertical < (sint32)minimum && vertical > 0)
        {
            val = character_pad.motion.horizontal_speed;
            coefficient = 0x40000000 / vertical;
            val = mul12((sint32)val, (sint32)minimum);
            product = (sint64)(sint32)val * coefficient;
            val = (uint64)product >> 18;
            fixed_math_result = val;
            character_pad.motion.horizontal_speed = val;
            character_pad.motion.vertical_speed = minimum;
        }
    }
    if (((uint16)(character_pad.motion.form_flags >> 16) & 15) == 5)
        rz = 0;
    x = (sint32)rx >> 6;
    y = (sint32)ry >> 6;
    z = (sint32)rz >> 6;
    val = x * x + y * y + z * z;
    if ((sint32)val < 0)
        speed = SquareRoot12((val >> 2)) << 1;
    else
        speed = SquareRoot12(val);
    character_pad.motion.horizontal_speed = speed;
    if (speed != 0)
    {
        angle = Atan2Angle(rz, rx);
        val = target_yaw;
        angle = (sint16)angle - val - 1024;
        if ((angle & 0xFFF) >= 2048)
        {
            character_pad.motion.horizontal_speed = -(uint32)character_pad.motion.horizontal_speed;
            rx = -rx;
            ry = -ry;
            rz = -rz;
        }
        cur_flags = flags;
        if ((cur_flags & 0x2000) != 0)
        {
            while (rx + 26754 > 0xD103 || ry + 26754 > 0xD103 || rz + 26754 > 0xD103)
            {
                rx = (sint32)rx >> 1;
                ry = (sint32)ry >> 1;
                rz = (sint32)rz >> 1;
            }
            vector.x = rx;
            vector.y = ry;
            vector.z = rz;
            {
                VECTOR sdk_input = {vector.x, vector.y, vector.z, 0};
                VECTOR sdk_output;
                VectorNormal(&sdk_input, &sdk_output);
                vector.x = sdk_output.vx;
                vector.y = sdk_output.vy;
                vector.z = sdk_output.vz;
            }
            x = vector.x;
            y = vector.y;
            z = vector.z;
            character_pad.path_tangent.vx = x;
            character_pad.path_tangent.vy = y;
            character_pad.path_tangent.vz = z;
        }
        else
        {
            vertical = character_pad.motion.vertical_speed;
            val = character_pad.movement.jump_speed;
            if ((sint32)val >= vertical && (cur_flags & 0x400) == 0)
            {
                character_pad.motion.jump_phase = 1;
                character_pad.motion.vertical_speed = val;
                character_pad.timers.state_ticks = 0;
                frame = -1;
            }
        }
    }
    flags = flags | 0x400;
}

uint32 AnimatedObject::ApplyContactImpulse(uint32 *move_flags)
{
    FUNCTION_MARKER(0x8004A218, "SLUS_005.78");
    const ActorModelDefinition *bound_model;
    Vec3 vector;
    const PathDefinition *parent;
    uint32 cur_flags, auxiliary_flags, parent_flag, special;
    const ActorBinding *contact_binding;
    const SceneNode *other;
    const SceneTrigger *impulse_trigger;
    const AnimatedObject *contact_object, *contact_root;
    uint32 volume, x, z, relative_x, relative_y, relative_z, strength, val, mode, idx;
    sint32 kind, dir, angle;
    cur_flags = character_pad.flags;
    auxiliary_flags = character_pad.secondary_flags;
    other = character_pad.support.contact;
    character_pad.flags = (cur_flags | 0x40000) & 0xFF7FFFFF;
    character_pad.secondary_flags = auxiliary_flags & 0xFFFFFF7F;
    if (other == 0 || other->kind != 0)
    {
        const SceneTrigger *sel = character_pad.surface_interaction.sel != 0 ? (const SceneTrigger *)character_pad.surface_interaction.sel : NULL;
        strength = sel->parameters.impulse_strength;
        relative_x = sel->transform.t[0];

        relative_z = sel->transform.t[2];
        dir = (sint16)character_pad.support.contact_dir;
        goto resolve_dir;
    }
    contact_object = (const AnimatedObject *)other;
    parent = contact_object->path_definition;
    parent_flag = 0;
    contact_binding = 0;
    if (parent)
    {
        contact_binding = (const ActorBinding *)(((const ActorPlacement *)parent)->state);
        parent_flag = contact_binding->action_effect_flags & 0x1000;
    }
    kind = contact_object->behavior;
    special = 0;
    if (kind == 20 && (contact_object->flags & 0x40))
        special = 1;
    if (!special && parent_flag && (contact_object->flags & 0x40))
    {
        kind = contact_object->behavior;
        special = 1;
    }
    if (special)
    {
        if (kind == 20 || (contact_binding->action_effect_flags & 0x1000))
        {
            bound_model = ((const ActorBinding *)(((const ActorPlacement *)contact_object->path_definition)->state))->model;
            const SoundBankStorage *actor_sounds = bound_model->sound_table;
            const SoundSample *sample = actor_sounds != 0 && actor_sounds->header.count >= 2 ? actor_sounds->samples + 1 : 0;
            volume = (uint16)sound_settings.effects_volume;
            sound_effects.Queue(sample, sample, 0, volume, 0, 3);
            character_pad.flags = character_pad.flags | 0x08000000;
        }
        if ((flags & 0x400) && contact_object->behavior != 30)
        {
            vector.z = 0;
            vector.x = 0;
            vector.y = 4096;
            velocity.y = 0;
        }
        else
        {
            x = world_pos.x;

            z = world_pos.z;
            vector = world_pos;
            contact_root = character_pad.support.contact != 0 ? (const AnimatedObject *)character_pad.support.contact : NULL;
            relative_x = contact_root->world_pos.x;

            relative_z = contact_root->world_pos.z;
            relative_x = x - relative_x;

            relative_z = z - relative_z;
            if (contact_object->behavior == 30)
            {
                vector.x = (sint32)relative_x < 0 ? (uint32)-1672 : 1672;
                vector.z = (sint32)relative_z < 0 ? (uint32)-1672 : 1672;
                vector.y = 3344;
                AnimatedObject::SpawnActionEffect(this, &character_pad, 1);
            }
            else
            {
                vector.x = (sint32)relative_x < 0 ? (uint32)-2364 : 2364;
                vector.z = (sint32)relative_z < 0 ? (uint32)-2364 : 2364;
                vector.y = 2364;
            }
        }
        if (contact_object->behavior == 30)
            strength = 409;
        else
            strength = (flags & 0x400) ? 16384 : 40960;
        mode = 0xFFFFFFFF;
        character_pad.support.motion.z = 0;
        character_pad.support.motion.y = 0;
        character_pad.support.motion.x = 0;
        goto apply_vector;
    }
    contact_root = character_pad.support.contact != 0 ? (const AnimatedObject *)character_pad.support.contact : NULL;
    relative_x = contact_root->world_pos.x;
    relative_y = contact_root->world_pos.y;
    relative_z = contact_root->world_pos.z;
    kind = contact_object->behavior;
    if (kind == 19 && (contact_object->flags & 0x40))
    {
        strength = 40960;
        dir = -1;
        goto resolve_dir;
    }
    if (kind == 23)
    {
        bound_model = ((const ActorBinding *)(((const ActorPlacement *)contact_object->path_definition)->state))->model;
        const SoundBankStorage *actor_sounds = bound_model->sound_table;
        const SoundSample *sample = actor_sounds != 0 && actor_sounds->header.count >= 2 ? actor_sounds->samples + 1 : 0;
        volume = (uint16)sound_settings.effects_volume;
        sound_effects.Queue(sample, sample, 0, volume, 0, 3);
    }
    strength = contact_object->behavior == 19 ? 12288 : 8192;
    dir = -1;
    if (contact_object->contact_kind == 2 && (contact_object->flags & 0x40))
    {
        x = world_pos.x;

        z = world_pos.z;
        vector = world_pos;
        val = Atan2Angle((relative_z - z), (relative_x - x));
        angle = contact_object->yaw - (sint16)val;
        vector.x = angle;
        if (angle < 0)
            vector.x = -(uint32)angle;
        if (vector.x < 1024)
            strength += ((const ActorBinding *)(((const ActorPlacement *)contact_object->path_definition)->state))->motion_speed;
        relative_x += (uint32)contact_object->velocity.x;
        relative_y += (uint32)contact_object->velocity.y;
        relative_z += (uint32)contact_object->velocity.z;
    }
resolve_dir:
    mode = (uint32)dir - 1;
    if (dir < 0)
    {
        x = camera_motion.focus_pos.x;
        relative_x = x - relative_x;

        z = camera_motion.focus_pos.z;
        relative_z = z - relative_z;
        val = Atan2Angle(relative_z, relative_x);
        angle = target_yaw;
        val = ((uint32)(sint16)val - angle - 1024) & 0xFFF;
        character_pad.motion.horizontal_speed = val < 2048 ? strength : -strength;
        goto finish;
    }
    idx = (sint32)mode >> 1;
    impulse_trigger = character_pad.surface_interaction.sel != 0 ? (const SceneTrigger *)character_pad.surface_interaction.sel : NULL;
    val = impulse_trigger->transform.m[idx][0];
    vector.x = (mode & 1) ? val : -val;
    impulse_trigger = character_pad.surface_interaction.sel != 0 ? (const SceneTrigger *)character_pad.surface_interaction.sel : NULL;
    val = impulse_trigger->transform.m[idx][1];
    vector.y = (mode & 1) ? val : -val;
    impulse_trigger = character_pad.surface_interaction.sel != 0 ? (const SceneTrigger *)character_pad.surface_interaction.sel : NULL;
    val = impulse_trigger->transform.m[idx][2];
    vector.z = (mode & 1) ? val : -val;
apply_vector:
    *move_flags = *move_flags | 8;
    ApplyDirectionalImpulse(move_flags, mode, vector, strength, 36044);
finish:
    val = (uint16)(character_pad.motion.form_flags >> 16);
    character_pad.timers.contact_impulse_ticks = 10;
    cur_flags = *move_flags;
    val = cur_flags & ((val & 15) == 3 ? 0xFFFFFFBE : 0xFFFFFF3E);
    *move_flags = val;
    return val;
}

void AnimatedObject::EndCharacterSpecialMove()
{
    FUNCTION_MARKER(0x8004A888, "SLUS_005.78");
    flags = flags & ~0x4000000;
    if (character_pad.motion.state_code == 9 && (character_pad.secondary_flags & 128) == 0)
        character_pad.motion.state_code = 2;
    character_pad.movement.speed_limits[0] = character_pad.special_move_backup[0];
    character_pad.movement.speed_limits[1] = character_pad.special_move_backup[1];
    character_pad.movement.speed_limits[2] = character_pad.special_move_backup[2];
    character_pad.movement.speed_limits[3] = character_pad.special_move_backup[3];
    character_pad.motion.horizontal_speed = character_pad.special_move_backup[1];
    character_pad.special_move_backup[1] = -15;
}

uint32 AnimatedObject::BuildMovementFlags()
{
    FUNCTION_MARKER(0x8004AA68, "SLUS_005.78");
    uint32 input, result = 0, prev, cur_flags, val, mask, filtered, x, y, z;
    const PathTransform *path_transform;
    sint32 timer;
    sint32 state;
    uint32 limit;
    sint32 response;
    if ((character_pad.flags & 0x4000000) != 0)
    {
        node.flags = node.flags & 0xFE;
        character_pad.flags = character_pad.flags & 0xFBFFFFFF;
        gameplay_input.buttons = 0;
        goto finish;
    }
    if (character_pad.timers.transition_ticks != 0 || (node.flags & 1) != 0)
        goto finish;
    input = gameplay_input.buttons;
    prev = gameplay_input.prev_buttons;
    timer = character_pad.path_transition.contact_delay;
    if (timer > 0)
        character_pad.path_transition.contact_delay = (uint32)timer - 1;
    state = character_pad.path_input.routing_phase;
    if (state == 0)
    {
        if ((player_interaction.flags & 0x10) != 0 || (path_routing.flags & 0x10) != 0)
            character_pad.path_input.routing_phase = 1;
    }
    else if (state == 1)
    {
        if ((player_interaction.flags & 0x10) == 0 && (path_routing.flags & 0x10) == 0)
            character_pad.path_input.routing_phase = 2;
    }
    else
    {
        character_pad.path_input.routing_phase = 2;
        mask = (*control_bindings.active_scheme)[0];
        if ((gameplay_input.buttons & mask) == 0)
            character_pad.path_input.routing_phase = 0;
    }
    if ((path_flags & 4) != 0)
        UpdateCharacterPathTimeout();
    if ((flags & 0x400) != 0)
        character_pad.path_transition.movement_ticks = character_pad.path_transition.movement_ticks + 1;
    else if ((path_flags & 4) == 0)
        character_pad.path_transition.movement_ticks = 0;
    val = character_pad.path_transition.reentry_ticks;
    if (val != 0)
        character_pad.path_transition.reentry_ticks = val - 1;
    if ((path_flags & 0x40000) != 0 && character_pad.path_transition.phase == 8)
    {
        val = character_pad.path_transition.phase_ticks + 1;
        limit = path_transition_rules.exit_ticks;
        character_pad.path_transition.phase_ticks = val;
        if (limit < val)
            path_flags = path_flags & 0xFFFBFFFF;
    }
    if ((path_flags & 0x80) != 0 && character_pad.path_transition.phase == 0)
    {
        response = EvaluatePathTransition(&input);
        if (response == -1)
            ClearCharacterPathTransitionFlags();
        else if (response == 1)
        {
            cur_flags = path_flags;
            if ((cur_flags & 1) != 0)
            {
                if ((cur_flags & 0x400) != 0)
                {
                    val = character_pad.path_transition.motion_heading;
                    result = 2;
                    yaw = val;
                    target_yaw = val;
                }
                EndCharacterConstrainedPath();
            }
            if ((path_flags & 0x820) != 0)
            {
                ClearCharacterPathRelease();
                ClearCharacterPathStep();
            }
            BeginCharacterPathTransition();
        }
    }
    if ((path_flags & 0x1000000) != 0 && ((input & ((*control_bindings.active_scheme)[1] | 0xA000)) != 0 || (character_pad.flags & 0x60000) != 0))
        ClearCharacterPathExitLatch();
    if ((path_flags & 0x20) != 0)
    {
        timer = character_pad.path_transition.hold_motion_ticks;
        if (timer < path_transition_rules.hold_motion_limit)
        {
            val = (uint32)timer + 1;
            character_pad.path_transition.hold_motion_ticks = val;
            if ((sint8)val < path_transition_rules.hold_motion_limit)
                path_flags = path_flags | 0x8000;
        }
    }
    if ((character_pad.secondary_flags & 0x240) != 0 || ((character_pad.motion.form_flags >> 16) & 0xF) - 13 < 2)
        result |= 1;
    if ((path_flags & 0x3C8) == 0)
        character_pad.path_transition.path = 0;
    cur_flags = path_flags;
    if ((cur_flags & 0x100) != 0 || ((cur_flags & 0x80) != 0 && character_pad.path_transition.phase > 0))
    {
        if ((character_pad.secondary_flags & 0x240) != 0 && character_pad.path_transition.phase < 4)
            result &= 0xFFFFFFFE;
        cur_flags = path_flags;
        if ((cur_flags & 0x640000) != 0 || camera_motion.path != character_pad.path_transition.path)
            goto post_transition;
        if (character_pad.path_transition.phase >= 4)
        {
            CancelCharacterPathTransition();
            goto post_transition;
        }
        if (character_pad.path_transition.hold_ticks >= path_transition_rules.exit_hold_ticks)
        {
            if (((cur_flags & 0x400) != 0 && (input & 0x2000) != 0) || ((cur_flags & 0x400) == 0 && (input & 0x8000) != 0))
            {
                ExitCharacterPathTransition();
                if ((input & 0x2000) != 0)
                    path_flags = path_flags | 0x10000;
                else
                    path_flags = path_flags | 0x20000;
                goto post_transition;
            }
        }
        if (character_pad.path_transition.phase < 4 && (character_pad.secondary_flags & 0x240) == 0 && (input & 0x4000) != 0 && (prev & 0x4000) == 0)
            RequestCharacterPathExit();
    post_transition:
        if ((path_flags & 0x40100) != 0)
            result |= 0x20000;
    }
    if ((camera_motion.path->definition->flags & 0x80000) != 0 && (path_flags & 0x30) == 0)
        ArmCharacterPathHold();
    cur_flags = path_flags;
    if ((cur_flags & 0x10) != 0)
    {
        if ((input & 0x4000) != 0 && (prev & 0x4000) == 0 && character_pad.path_input.routing_phase == 0 && (input & 0xA000) == 0 && (flags & 0x400) == 0 && (character_pad.flags & 0x800000) == 0)
        {
            if ((cur_flags & 0x20) == 0)
                BeginCharacterPathRelease();
        }
        else if ((path_flags & 0x20) == 0)
        {
            if ((camera_motion.path->definition->flags & 0x80000) != 0)
                character_pad.path_transition.hold_motion_ticks = 0;
            else
            {
                timer = character_pad.path_transition.hold_motion_ticks;
                if (timer < 4)
                    character_pad.path_transition.hold_motion_ticks = (uint32)timer + 1;
                else
                    EndCharacterPathHold();
            }
        }
    }
    if ((path_flags & 0x20) != 0)
        result |= 0x10000;
    if ((path_flags & 0x1081) == 0 && (camera_motion.path->definition->flags & 0x10000) != 0)
    {
        BeginCharacterConstrainedPath();
        if ((input & 0xA000) != 0)
            val = (input & 0x2000) != 0 ? 16 : 32;
        else
            val = (flags & 0x10) != 0 ? 16 : 32;
        character_pad.path_input.dir = val;
    }
    if ((camera_motion.path->definition->flags & 0x10000) == 0)
    {
        cur_flags = path_flags;
        if ((cur_flags & 1) == 0)
            goto ungrounded;
        if ((cur_flags & 0x80) == 0 || (result & 0x800) == 0)
        {
            timer = character_pad.path_input.settle_ticks;
            if (timer < 4)
                character_pad.path_input.settle_ticks = (uint32)timer + 1;
            else
                EndCharacterConstrainedPath();
        }
    }
    if ((path_flags & 1) != 0)
    {
        mask = (*control_bindings.active_scheme)[1];
        if ((input & mask) != 0 && (prev & mask) == 0)
        {
            BeginCharacterSurfaceMotion();
            state = character_pad.path_input.dir;
            character_pad.motion.jump_phase = 1;
            character_pad.motion.vertical_speed = 0;
            character_pad.timers.state_ticks = 0;
            frame = -1;
            result |= 0x2000;
            if ((state == 16 && (flags & 0x10) != 0) || (state == 32 && (flags & 0x10) == 0))
            {
                ReverseScenePath();
                val = (uint16)target_yaw;
                yaw = val;
                orientation[2] = (val + 2048) & 0xFFF;
            }
            goto movement;
        }
        if (character_pad.path_transition.input_ready == 0)
        {
            character_pad.path_transition.input_ready = 1;
            goto movement;
        }
        if (character_pad.path_input.routing_phase != 0)
            goto movement;
        path_transform = path_definition->transform;
        filtered = input;
        val = path_transform != 0 && path_transform->displacement.y != 0;
        if (val == 0 && ((character_pad.flags & 0x40) != 0 || character_pad.path_input.latch_phase != 0))
        {
            if (character_pad.path_input.latch_phase != 0)
            {
                character_pad.path_input.latch_phase = 2;
                filtered &= 0xFFFFEFFF;
            }
            else
            {
                val = character_pad.path_input.prev_motion;
                if ((val & 0x800) != 0)
                {
                    filtered &= 0xFFFFEFFF;
                    character_pad.path_input.latched_motion = val;
                    character_pad.path_input.latch_phase = 1;
                    character_pad.path_input.release_ticks = 0;
                }
            }
            if (character_pad.path_input.latch_phase != 0 && (input & 0x4000) != 0)
            {
                val = character_pad.path_input.release_ticks + 1;
                character_pad.path_input.release_ticks = val;
                if ((sint32)val >= 2)
                    character_pad.path_input.latch_phase = 0;
            }
        }
        if ((flags & 0x10) != 0)
        {
            if ((filtered & 0x1000) != 0)
                result |= 0x801;
            else if ((filtered & 0x4000) != 0)
                result |= 2;
        }
        else
        {
            if ((filtered & 0x4000) != 0)
                result |= 0x1001;
            else if ((filtered & 0x1000) != 0)
                result |= 2;
        }
        val = result & 0x1800;
        if (val != 0)
            character_pad.path_input.prev_motion = val;
        if ((input & 0x2000) != 0)
            character_pad.path_input.dir = 32;
        if ((input & 0x8000) != 0)
            character_pad.path_input.dir = 16;
        result |= character_pad.path_input.dir;
        goto movement;
    }
ungrounded:
    if ((path_flags & 4) != 0 && (flags & 0x400) == 0)
        EndCharacterConstrainedPath();
movement:
    cur_flags = path_flags;
    if ((cur_flags & 6) != 2 && (cur_flags & 0x101) == 0)
    {
        if ((flags & 0x10) != 0)
        {
            if ((input & 0x2000) != 0)
                result |= 1;
            else if ((input & 0x8000) != 0)
                result |= 2;
        }
        else
        {
            if ((input & 0x8000) != 0)
                result |= 1;
            else if ((input & 0x2000) != 0)
                result |= 2;
        }
    }
    if ((path_flags & 0x10) != 0)
    {
        result |= 0x8000;
        if ((input & 0x1000) != 0 && (flags & 0x400) == 0)
            result = (result | 0x800) & 0xFFFFFFFC;
    }
    if ((sint32)character_pad.flags < 0)
        goto finish;
    cur_flags = path_flags;
    if ((cur_flags & 0x1120) != 0)
        goto finish;
    if ((character_pad.secondary_flags & 0x240) != 0)
        goto button_actions;
    if ((input & 0x4000) != 0 && (cur_flags & 0x15) == 0)
        result |= 0x80;
    mask = (*control_bindings.active_scheme)[0];
    if ((input & mask) != 0)
    {
        if ((prev & mask) == 0)
            result |= 0x40;
        else
        {
            val = (uint16)(character_pad.motion.form_flags >> 16) & 0xF;
            if (val == 4 || val == 5 || val - 10 < 2)
                result |= 0x40;
        }
    }
    if ((result & 0x40) != 0)
    {
        if ((character_pad.secondary_flags & 0x100) != 0)
            flags = flags | 0x1000000;
        cur_flags = player_interaction.flags;
        if ((cur_flags & 4) != 0)
        {
            player_interaction.flags = cur_flags | 8;
            result ^= 0x40;
        }
    }
    else
    {
        cur_flags = player_interaction.flags;
        val = cur_flags & 0xFFFFFFF7;
        player_interaction.flags = val;
        mask = (*control_bindings.active_scheme)[0];
        if ((input & mask) != 0)
        {
            if ((cur_flags & 0x10) != 0)
                result &= 0xFFFFFFFD;
        }
        else
        {
            x = path_routing.flags;
            player_interaction.flags = val & 0xFFFFFFEF;
            path_routing.flags = x & 0xFFFFFFEF;
        }
    }
    val = (uint16)(character_pad.motion.form_flags >> 16) & 0xF;
    if (val == 5 || val - 10 < 2)
    {
        if ((result & 0x80) != 0)
        {
            cur_flags = flags;
            result &= 0xFFFFFF7F;
            if ((cur_flags & 0x400) != 0)
                result |= 0x200;
        }
        else if ((input & 0x1000) != 0)
            result |= 0x100;
    }
    else if (val == 1)
    {
        if ((flags & 0x4000000) != 0)
            result = 1024;
        else
        {
            mask = (*control_bindings.active_scheme)[2];
            val = input & mask;
            if (val == mask && (prev & val) != val)
                result |= 0x400;
        }
    }
button_actions:
    mask = (*control_bindings.active_scheme)[1];
    if ((input & mask) != 0)
    {
        result |= 8;
        if ((prev & mask) == 0)
        {
            if (character_pad.motion.buffered_rise_limit != 0)
            {
                if (character_pad.timers.jump_input_ticks < 6)
                {
                    x = character_pad.motion.buffered_horizontal_speed;
                    y = character_pad.motion.buffered_vertical_speed;
                    z = character_pad.motion.buffered_rise_limit;
                    cur_flags = character_pad.flags;
                    character_pad.motion.jump_phase = 1;
                    character_pad.timers.state_ticks = 0;
                    character_pad.motion.horizontal_speed = x;
                    character_pad.motion.vertical_speed = y;
                    character_pad.movement.rise_speed_limit = z;
                    character_pad.flags = cur_flags | 0x10000000;
                }
                character_pad.motion.buffered_rise_limit = 0;
            }
            character_pad.timers.jump_input_ticks = -1;
        }
    }
    timer = character_pad.timers.jump_input_ticks;
    if (timer < 6)
        character_pad.timers.jump_input_ticks = (uint32)timer + 1;
finish:
    if (character_pad.support.release_pending == 1)
    {
        cur_flags = character_pad.flags;
        character_pad.support.release_pending = 0;
        character_pad.flags = cur_flags & 0xFFFFF7EF;
    }
    return result;
}

void AnimatedObject::UpdateJumpMotion(uint32 *move_flags)
{
    FUNCTION_MARKER(0x8004B8F8, "SLUS_005.78");
    uint32 cur_flags, val, mask, x, y, z, sq, angle, result;
    sint32 old_frame, speed, len, c, duration, timer;
    if (((*move_flags & 8) == 0 && character_pad.motion.jump_phase < 2 && (character_pad.flags & 0x10000000) == 0) || (character_pad.flags & 0x10000) != 0 || (path_flags & 0x200) != 0)
    {
        if ((flags & 0x400) != 0)
        {
            if (character_pad.motion.jump_phase > 0)
            {
                if ((character_pad.flags & 0x10000000) == 0)
                {
                    character_pad.timers.state_ticks = 0;
                    character_pad.motion.jump_phase = 0;
                }
            }
            else
                character_pad.flags = character_pad.flags & 0xEFFFFFFF;
        }
        goto done;
    }
    cur_flags = character_pad.flags;
    if ((cur_flags & 0x2000) != 0 && (flags & 0x400) == 0)
        goto done;
    if ((cur_flags & 0x10000000) == 0)
        character_pad.timers.jump_duration_ticks = character_pad.movement.jump_ticks;
    *move_flags = *move_flags | 8;
    if (((uint16)(character_pad.motion.form_flags >> 16) & 0xF) == 5 && (character_pad.secondary_flags & 0x80) == 0)
        character_pad.motion.state_code = 3;
    if ((flags & 0x400) == 0)
    {
        character_pad.flags = character_pad.flags | 0x2000;
        if ((flags & 0x8000000) != 0)
        {
            *move_flags = *move_flags & 0xFFFFFBFE;
            flags = flags & 0xF7FFFFFF;
        }
        if (character_pad.motion.vertical_speed <= 0)
        {
            old_frame = frame;
            val = character_pad.movement.jump_speed;
            character_pad.motion.jump_phase = 1;
            character_pad.timers.state_ticks = 0;
            character_pad.motion.vertical_speed = val;
            frame = -1;
            if ((character_pad.flags & 0x800000) != 0)
            {
                val = character_pad.motion.vertical_speed;
                speed = character_pad.surface_interaction.jump_scale->parameters.jump.rise_scale;
                speed = mul12((sint32)val, speed);
                character_pad.motion.vertical_speed = speed;
                if (speed <= 0)
                {
                    *move_flags = *move_flags & 0xFFFFFFF7;
                    val = character_pad.movement.jump_ticks;
                    cur_flags = character_pad.flags;
                    character_pad.motion.vertical_speed = 0;
                    character_pad.motion.jump_phase = -1;
                    character_pad.timers.state_ticks = 0;
                    character_pad.timers.jump_duration_ticks = val;
                    character_pad.flags = cur_flags | 0x8000000;
                    frame = old_frame;
                }
            }
            if (character_pad.motion.vertical_speed == 0)
                goto done;
        }
        character_pad.movement_flags = character_pad.movement_flags | 0x400000;
        cur_flags = flags | 0x400;
        flags = cur_flags;
        if ((cur_flags & 0x2000) == 0)
        {
            x = character_pad.path_velocity.vx >> 6;
            y = character_pad.path_velocity.vy >> 6;
            z = character_pad.path_velocity.vz >> 6;
            sq = x * x + y * y + z * z;
            if ((sint32)sq >= 0)
                len = SquareRoot12(sq);
            else
                len = SquareRoot12((sq >> 2)) << 1;
            angle = (-AsinAngle(character_pad.path_velocity.vy, len)) & 0xFFF;
            if (character_pad.motion.horizontal_speed < 0)
                angle = (-angle) & 0xFFF;
            if ((sint16)angle >= 1025)
                angle += (uint32)((4096 - (sint16)angle) >> 1);
            else
            {
                angle <<= 1;
                if ((sint16)angle >= 1025)
                    angle = 1024;
            }
            if ((path_flags & 4) != 0)
            {
                angle = 3584;
                character_pad.motion.horizontal_speed = character_pad.movement.speed_limits[1];
            }
            if ((sint16)angle >= 2048)
            {
                c = SinQ14((1024 - angle)) >> 2;
                speed = character_pad.motion.horizontal_speed;
                speed = mul12(c, speed);
                if ((speed ^ (uint32)character_pad.motion.horizontal_speed) >= 0x80000000)
                    speed = 0;
                character_pad.motion.horizontal_speed = speed;
            }
            else if ((sint16)angle >= 512)
            {
                character_pad.motion.horizontal_speed = 0;
                *move_flags = *move_flags & 0xFFFFFBFE;
            }
        }
        cur_flags = flags;
        if ((cur_flags & 0x8000000) != 0 && ((uint16)(character_pad.motion.form_flags >> 16) & 0xF) == 4)
            goto done;
        val = (character_pad.motion.form_flags >> 16) & 0xF;
        if (val - 6 < 2)
        {
            if (character_pad.motion.horizontal_speed != 0)
                val = (cur_flags & 0x10) != 0 ? 34 : 33;
            else
                val = 3;
        }
        else
            val = 3;
        if ((character_pad.secondary_flags & 0x80) == 0)
            character_pad.motion.state_code = val;
        AnimatedObject::QueueCharacterActionSound(0, 3);
        goto done;
    }
    duration = character_pad.timers.jump_duration_ticks;
    if (character_pad.motion.jump_phase > 0)
    {
        val = character_pad.timers.state_ticks + 1;
        character_pad.timers.state_ticks = val;
        timer = (sint16)val;
        if (duration < timer || ((character_pad.flags & 0x8000000) != 0 && (duration >> 1) < timer))
        {
            cur_flags = character_pad.flags;
            character_pad.timers.state_ticks = 0;
            character_pad.motion.jump_phase = 0;
            character_pad.flags = cur_flags & 0xEFFFFFFF;
        }
    }
    if (((uint16)(character_pad.motion.form_flags >> 16) & 0xF) == 0)
    {
        mask = (*control_bindings.active_scheme)[1];
        if ((gameplay_input.prev_buttons & mask) == 0 && (gameplay_input.buttons & mask) != 0 && (character_pad.flags & 0x8000000) == 0 && (uint32)character_pad.motion.vertical_speed + 4504 < 0x4198)
            goto reactivate;
    }
    if ((game_flow.mode_flags & 0x400) == 0)
        goto done;
    mask = (*control_bindings.active_scheme)[1];
    if ((gameplay_input.prev_buttons & mask) != 0 || (gameplay_input.buttons & mask) == 0)
        goto done;
reactivate:
    val = character_pad.model->movement.rise_speed_limit;
    cur_flags = character_pad.movement.jump_ticks;
    x = character_pad.movement.jump_speed;
    character_pad.timers.jump_duration_ticks = cur_flags;
    character_pad.timers.state_ticks = 0;
    character_pad.motion.vertical_speed = x;
    character_pad.motion.jump_phase = 1;
    character_pad.movement.rise_speed_limit = val;
    frame = -1;
    if ((character_pad.secondary_flags & 0x80) == 0)
        character_pad.motion.state_code = 9;
    AnimatedObject::QueueCharacterActionSound(5, 3);
    character_pad.flags = (character_pad.flags | 0x8002000) & 0xEFFFFFFF;
done:
    result = *move_flags & 8;
    if (result == 0)
    {
        result = character_pad.flags & 0xFFFFDFFF;
        character_pad.flags = result;
    }
}

void AnimatedObject::UpdateSpecialMovement(uint32 *move_flags)
{
    FUNCTION_MARKER(0x8004BFD0, "SLUS_005.78");
    uint32 cur_flags, val, kind, i, result;
    sint32 timer;
    if ((flags & 0x8000000) != 0 && (((character_pad.motion.form_flags >> 16) & 0xF) - 13 >= 2))
    {
        *move_flags = *move_flags & 0xFFFFFBFE;
        goto secondary;
    }
    if (((uint16)(character_pad.motion.form_flags >> 16) & 0xF) != 1)
    {
        flags = flags & 0xFBFFFFFF;
        goto secondary;
    }
    if ((flags & 0x4000000) != 0)
    {
        if (((*move_flags & 0x400) == 0 && (node.flags & 1) == 0) || (character_pad.flags & 0x18040) != 0 || character_pad.motion.state_code != 9)
            goto restore;
        val = character_pad.timers.special_move_ticks - 1;
        character_pad.timers.special_move_ticks = val;
        if ((val & 0xFFFF) != 0)
            goto secondary;
    restore:
        flags = flags & 0xFBFFFFFF;
        if (character_pad.motion.state_code == 9 && (character_pad.secondary_flags & 0x80) == 0)
            character_pad.motion.state_code = 2;
        for (i = 0; i < 4; ++i)
            character_pad.movement.speed_limits[i] = character_pad.special_move_backup[i];
        character_pad.motion.horizontal_speed = character_pad.special_move_backup[1];
        character_pad.special_move_backup[1] = -15;
    }
    else
    {
        timer = character_pad.special_move_backup[1];
        if (timer > 0)
        {
            character_pad.special_move_backup[1] = 0;
            timer = character_pad.special_move_backup[1];
        }
        val = 0;
        if (timer == 0)
            val = 1;
        else
        {
            timer = (uint32)timer + 1;
            character_pad.special_move_backup[1] = timer;
            if (timer == 0)
                val = 1;
        }
        if (val == 0 || (*move_flags & 0x400) == 0 || (flags & 0x400) != 0)
        {
            *move_flags = *move_flags & 0xFFFFFBFF;
            goto secondary;
        }
        flags = flags | 0x4000000;
        if ((character_pad.secondary_flags & 0x80) == 0)
            character_pad.motion.state_code = 9;
        val = character_pad.movement.speed_limits[0];
        character_pad.timers.special_move_ticks = 10;
        character_pad.special_move_backup[0] = val;
        val = character_pad.movement.speed_limits[1];
        character_pad.special_move_backup[1] = val;
        val = mul12((sint32)val, 5120);
        cur_flags = character_pad.movement.speed_limits[2];
        character_pad.movement.speed_limits[1] = val;
        character_pad.movement.speed_limits[0] = val;
        character_pad.special_move_backup[2] = cur_flags;
        val = character_pad.movement.speed_limits[3];
        character_pad.special_move_backup[3] = val;
        val = mul12((sint32)val, 5120);
        character_pad.movement.speed_limits[3] = val;
        character_pad.movement.speed_limits[2] = val;
    }
secondary:
    if ((*move_flags & 0x80) != 0 && character_pad.model->animations[7] != 0)
    {
        cur_flags = flags;
        if ((cur_flags & 0x5000000) == 0 && (((uint16)(character_pad.motion.form_flags >> 16) & 0xF) == 3 || (cur_flags & 0x400) == 0))
            flags = cur_flags | 0x8000000;
        result = flags & 0x8000000;
        if (result != 0)
        {
            if ((character_pad.secondary_flags & 0x80) == 0)
                character_pad.motion.state_code = 7;
            result = character_pad.motion.slope_scale;
            if (result == 0)
                character_pad.motion.horizontal_speed = 0;
        }
    }
    else
    {
        result = flags & 0x8010000;
        if (result == 0x8000000)
        {
            kind = (uint16)(character_pad.motion.form_flags >> 16) & 0xF;
            if (kind == 4)
            {
                result = character_pad.flags & 0x60000;
                if (result != 0)
                    return;
            }
            if (character_pad.motion.state_code == 7 && (character_pad.secondary_flags & 0x80) == 0)
                character_pad.motion.state_code = 0;
            result = flags & 0xF7FFFFFF;
            flags = result;
        }
    }
}

void AnimatedObject::UpdateMovementState(uint32 *move_flags)
{
    FUNCTION_MARKER(0x8004C3B0, "SLUS_005.78");
    uint32 cur_flags, val, kind, animation, i, result, decelerate;
    sint32 speed, state;
    if ((character_pad.flags & 0x10000) != 0)
        goto input_actions;
    if ((path_flags & 0x30000) != 0)
    {
        val = character_pad.path_transition.phase_ticks + 1;
        kind = path_transition_rules.special_ticks;
        character_pad.path_transition.phase_ticks = val;
        if (val >= kind)
            path_flags = path_flags & 0xFFFCFFFF;
    }
    decelerate = 0;
    if ((*move_flags & 0x401) == 0)
        decelerate = (path_flags & 0x30000) == 0;
    UpdateCharacterHorizontalSpeed(decelerate);
    if ((*move_flags & 0x10000) != 0)
    {
        if ((character_pad.secondary_flags & 0x80) == 0)
            character_pad.motion.state_code = 6;
    }
    else if ((*move_flags & 0x8000) != 0 && (flags & 0x400) == 0)
    {
        if ((*move_flags & 0x800) != 0)
            animation = 17;
        else if ((character_pad.motion.slope_scale != 0 && velocity.y < 0) || character_pad.motion.horizontal_speed <= 0)
            animation = 16;
        else
            animation = 15;
        if ((character_pad.secondary_flags & 0x80) == 0)
            character_pad.motion.state_code = animation;
    }
    else if ((path_flags & 1) != 0)
    {
        if (character_pad.motion.horizontal_speed != 0)
            animation = (gameplay_input.buttons & 0x1000) != 0 ? 10 : 12;
        else
            animation = 11;
        if ((character_pad.secondary_flags & 0x80) == 0)
            character_pad.motion.state_code = animation;
        if ((*move_flags & 0x800) != 0)
            character_pad.motion.horizontal_speed = mul12(character_pad.movement.speed_limits[1], character_pad.model->motion_scale.x);
        else if ((*move_flags & 0x1000) != 0)
            character_pad.motion.horizontal_speed = mul12(character_pad.movement.speed_limits[1], character_pad.model->motion_scale.y);
        else
            character_pad.motion.horizontal_speed = 0;
    }
    cur_flags = flags;
    if ((cur_flags & 0x9000400) != 0 || (path_flags & 0x1133) != 0)
        goto input_actions;
    speed = character_pad.motion.horizontal_speed;
    if (speed != 0)
    {
        if ((*move_flags & 0x401) != 0)
        {
            kind = (uint16)(character_pad.motion.form_flags >> 16) & 0xF;
            if (kind == 1 && (cur_flags & 0x4000000) != 0)
                goto input_actions;
            goto moving_animation;
        }
        animation = (uint8)character_pad.motion.state_code;
        if (animation - 4 < 2)
            goto input_actions;
        if (character_pad.motion.slope_scale != 0 || (character_pad.flags & 8) != 0)
        {
            animation = 8;
            goto set_animation;
        }
        if (speed > 0)
        {
            kind = (uint16)(character_pad.motion.form_flags >> 16) & 0xF;
            goto moving_animation;
        }
        goto idle_animation;
    }
    if (((uint16)(character_pad.motion.form_flags >> 16) & 0xF) == 1 && (cur_flags & 0x4000000) != 0)
    {
        flags = cur_flags & 0xFBFFFFFF;
        if (character_pad.motion.state_code == 9 && (character_pad.secondary_flags & 0x80) == 0)
            character_pad.motion.state_code = 2;
        for (i = 0; i < 4; ++i)
            character_pad.movement.speed_limits[i] = character_pad.special_move_backup[i];
        character_pad.motion.horizontal_speed = character_pad.special_move_backup[1];
        character_pad.special_move_backup[1] = -15;
        character_pad.motion.horizontal_speed = 0;
    }
    animation = (uint8)character_pad.motion.state_code;
    if (animation - 4 < 2)
        goto input_actions;
    if ((character_pad.flags & 8) != 0)
    {
        animation = 8;
        goto set_animation;
    }
    if ((*move_flags & 1) != 0)
    {
        kind = (uint16)(character_pad.motion.form_flags >> 16) & 0xF;
        cur_flags = flags;
        goto moving_animation;
    }
idle_animation:
    if ((uint32)(sint8)animation - 1 < 34)
    {
        animation = 0;
        goto set_animation;
    }
    goto input_actions;
moving_animation:
    if (kind - 6 < 2)
        animation = (cur_flags & 0x10) != 0 ? 32 : 31;
    else
        animation = 2;
set_animation:
    if ((character_pad.secondary_flags & 0x80) == 0)
        character_pad.motion.state_code = animation;
input_actions:
    if ((*move_flags & 0x100) != 0)
    {
        state = character_pad.motion.state_code;
        character_pad.motion.jump_phase = 1;
        character_pad.timers.state_ticks = 0;
        if (state != 9 && state != 5)
        {
            if ((character_pad.secondary_flags & 0x80) == 0)
                character_pad.motion.state_code = 9;
            flags = flags | 0x400;
            if ((character_pad.flags & 0x10000) != 0)
            {
                val = character_pad.support.speed[1];
                character_pad.motion.horizontal_speed = 0;
                character_pad.motion.vertical_speed = val;
                if ((sint32)val < 0)
                    character_pad.motion.vertical_speed = -val;
                character_pad.ClearTransientFlags();
            }
        }
    }

    if ((path_flags & 1) != 0)
    {
        if (character_pad.path_input.dir == 16)
            result = character_pad.path_transition.motion_heading;
        else
            result = (character_pad.path_transition.motion_heading + 2048) & 0xFFF;
        yaw = result;
        target_yaw = result;
    }
}

void AnimatedObject::UpdateActionInput(uint32 *move_flags)
{
    FUNCTION_MARKER(0x8004CA00, "SLUS_005.78");
    uint32 kind, cur_flags, result;
    if ((uint16)character_pad.weapon_state != 0)
        goto action;
    kind = (uint16)(character_pad.motion.form_flags >> 16) & 0xF;
    if (kind == 5 || kind - 10 < 2)
        goto action;
    if (kind == 1 && player_interaction.flags == 0 && (flags & 0x8000000) == 0 && character_pad.motion.state_code != 4)
        goto action;
    result = (uint16)(character_pad.motion.form_flags >> 16) & 0xF;
    if (result != 4)
        return;
action:
    result = *move_flags & 0x40;
    if (result == 0)
        return;
    if (character_pad.timers.action_cooldown_ticks != 0)
        return;
    cur_flags = flags;
    result = cur_flags & 0x5000000;
    if (result != 0)
        return;
    result = node.flags & 1;
    if (result != 0)
        return;
    if (character_pad.model->animations[5] == 0)
        return;
    cur_flags |= 0x1000000;
    flags = cur_flags;
    kind = (uint16)(character_pad.motion.form_flags >> 16) & 0xF;
    if (kind == 1)
    {
        if ((character_pad.secondary_flags & 0x80) == 0)
            character_pad.motion.state_code = 5;
        frame = -1;
        character_pad.timers.action_launch_ticks = 5;
        return;
    }
    if (kind != 4 && kind != 5 && kind - 10 >= 2)
    {
        if ((cur_flags & 0x8000400) != 0 || character_pad.motion.state_code == 2)
            return;
    }
    if ((character_pad.secondary_flags & 0x80) == 0)
        character_pad.motion.state_code = 5;
    frame = -1;
}

uint32 AnimatedObject::IntegrateCharacterMovement(uint32 input_flags)
{
    FUNCTION_MARKER(0x8004CBA4, "SLUS_005.78");
    const ScenePath *camera_path;
    uint32 horizontal, vertical, cur_flags, val, kind, state, grounded, result;
    const PathDefinition *definition;
    PathTransform *path_transform;
    Vec3 sample_pos, floor, original;
    sint32 lower_limit, acceleration, upper_limit, rise, speed, dir, frame;
    if ((input_flags & 2) != 0 && (flags & 0x9000000) == 0 && (((character_pad.motion.form_flags >> 16) & 0xF) - 13 >= 2))
        goto prepare;
    if ((input_flags & 0x20) != 0 && (path_flags & 2) != 0)
    {
    prepare:
        ReverseScenePath();
    }
    cur_flags = character_pad.flags;
    if ((cur_flags & 0x10000) != 0)
    {
        UpdateCharacterSupportMotion();
        goto finish_motion;
    }
    if ((flags & 0x400) == 0 && character_pad.motion.horizontal_speed == 0 && (cur_flags & 0x99048) == 0 && character_pad.motion.environment_acceleration == 0 && (path_flags & 0x122) == 0)
    {
        velocity.z = 0;
        velocity.y = 0;
        velocity.x = 0;
        goto finish_motion;
    }
    cur_flags = flags;
    val = character_pad.motion.horizontal_speed;
    horizontal = val;
    if ((cur_flags & 0x400) == 0 && (path_flags & 0x122) == 0)
    {
        vertical = 0;
        goto adjust_horizontal;
    }
    lower_limit = character_pad.movement.fall_speed_limit;
    acceleration = character_pad.movement.fall_acceleration;
    if ((input_flags & 0x20000) != 0 || (path_flags & 0x200000) != 0)
    {
        UpdateTransitionMotion(input_flags, 0, &horizontal, &vertical);
        goto integrate_vertical;
    }
    if ((input_flags & 0x10000) != 0 && path_transition_rules.release_mode == 0)
    {
        val = mul12(path_transition_rules.release_motion_scale, character_pad.movement.speed_limits[1]);
        character_pad.motion.vertical_speed = val;
        cur_flags = character_pad.motion.vertical_speed;
        vertical = val;
        val = -cur_flags;
        character_pad.motion.vertical_speed = val;
        vertical = val;
        goto integrate_vertical;
    }
    if ((input_flags & 0x2000) != 0 || (path_flags & 4) != 0)
    {
        UpdateSpecialAxisMotion(input_flags, (input_flags & 0x2000) != 0 ? 8 : 0, &horizontal, &vertical);
        goto integrate_vertical;
    }
    if ((input_flags & 0x200) != 0)
    {
        state = character_pad.motion.state_code;
        character_pad.motion.jump_phase = -1;
        character_pad.timers.state_ticks = 0;
        if (state != 5 && (character_pad.secondary_flags & 0x80) == 0)
            character_pad.motion.state_code = 7;
        goto integrate_vertical;
    }
    kind = (uint16)(character_pad.motion.form_flags >> 16) & 0xF;
    if ((kind != 5 && kind - 10 >= 2) || (input_flags & 0x100) != 0)
        goto integrate_vertical;
    if (character_pad.motion.vertical_speed < lower_limit)
    {
        character_pad.motion.jump_phase = 1;
        input_flags |= 0x100;
    }
    state = character_pad.motion.state_code;
    if (state != 3 || (input_flags & 8) != 0)
        goto check_animation;
    if ((character_pad.flags & 0x60000) != 0)
    {
        state = character_pad.motion.state_code;
    check_animation:
        if (state == 5 || state == 3)
            goto integrate_vertical;
    }
    cur_flags = character_pad.secondary_flags;
    if ((cur_flags & 0x400) == 0 && (cur_flags & 0x80) == 0)
        character_pad.motion.state_code = (input_flags & 3) != 0 ? 9 : 6;
integrate_vertical:
    if ((character_pad.flags & 0x800000) != 0)
    {
        const SceneTrigger *jump_scale = character_pad.surface_interaction.jump_scale;
        upper_limit = mul12(character_pad.movement.rise_speed_limit, jump_scale->parameters.jump.rise_scale);
        rise = mul12(character_pad.movement.rise_acceleration, jump_scale->parameters.jump.rise_scale);
    }
    else
    {
        upper_limit = character_pad.movement.rise_speed_limit;
        rise = character_pad.movement.rise_acceleration;
    }
    cur_flags = path_flags;
    if ((cur_flags & 2) != 0 && (cur_flags & 4) == 0)
    {
        dir = character_pad.motion.jump_phase;
        if (dir > 0)
        {
            speed = character_pad.motion.vertical_speed;
            if (speed < upper_limit)
                character_pad.motion.vertical_speed = speed;
            dir = character_pad.motion.jump_phase;
        }
        if (dir < 0)
        {
            speed = character_pad.motion.vertical_speed;
            if (lower_limit < speed)
            {
                character_pad.motion.vertical_speed = speed;
                if (speed < lower_limit)
                    goto clamp_lower;
            }
        }
        goto save_vertical;
    }
    dir = character_pad.motion.jump_phase;
    if (dir > 0)
    {
        speed = character_pad.motion.vertical_speed;
        if (speed < upper_limit)
        {
            character_pad.motion.vertical_speed = speed;
            if (((cur_flags & 2) != 0 && (input_flags & 0x4000) != 0) || ((cur_flags & 2) == 0 && (input_flags & 0x108) != 0))
                character_pad.motion.vertical_speed = (uint32)speed + rise;
            speed = character_pad.motion.vertical_speed;
            character_pad.motion.vertical_speed = speed;
            if (speed >= upper_limit)
            {
                character_pad.motion.vertical_speed = upper_limit;
                vertical = upper_limit;
                goto adjust_horizontal;
            }
        }
        dir = character_pad.motion.jump_phase;
    }
    if (dir < 0)
    {
        speed = character_pad.motion.vertical_speed;
        if (lower_limit < speed)
        {
            speed = (uint32)speed + acceleration;
            character_pad.motion.vertical_speed = speed;
            if (speed < lower_limit)
            {
            clamp_lower:
                character_pad.motion.vertical_speed = (uint32)lower_limit;
                vertical = lower_limit;
                goto adjust_horizontal;
            }
        }
    }
save_vertical:
    vertical = character_pad.motion.vertical_speed;
adjust_horizontal:
    if ((character_pad.flags & 0x80008) != 0)
        ApplyEnvironmentMotion(&horizontal, &vertical);
    else
    {
        val = character_pad.motion.environment_acceleration;
        if (val != 0)
        {
            if ((flags & 0x10) != 0)
                val = character_pad.motion.horizontal_speed + val;
            else
                val = character_pad.motion.horizontal_speed - val;
            character_pad.motion.horizontal_speed = val;
            val = character_pad.motion.horizontal_speed;
            character_pad.motion.environment_acceleration = 0;
            horizontal = val;
        }
    }
    speed = horizontal;
    if (speed > 15360)
        horizontal = 15360;
    else if (speed < -15360)
        horizontal = -15360;
    val = horizontal;
    cur_flags = vertical;
    character_pad.support.speed[0] = val;
    character_pad.support.speed[1] = cur_flags;
    cur_flags = flags;
    grounded = (cur_flags >> 10) & 1;
    if (grounded != 0)
        character_pad.flags = character_pad.flags & 0xFFFFEFFF;
    else if ((character_pad.flags & 0x1000) != 0)
        flags = cur_flags | 0x400;
    ApplyCharacterPathMotion(horizontal, vertical);
    cur_flags = character_pad.flags;
    if ((cur_flags & 0x40) != 0)
        ResolveCharacterSupport(grounded);
    else if ((cur_flags & 0x8000) != 0)
        ProbeCharacterSupport();
    if (grounded != 0)
        goto finish_motion;
    if ((path_definition->flags & 0x11) != 0)
        flags = flags | 0x400;
    if ((flags & 0x400) == 0)
        goto finish;
    if ((sint32)horizontal <= 0 || (character_pad.flags & 0x1000) != 0)
        goto stop_vertical;
    definition = character_pad.path->definition;
    if ((definition->flags & 0x11) != 0)
    {
    stop_vertical:
        val = character_pad.movement.jump_ticks;
        cur_flags = character_pad.flags;
        kind = (uint16)(character_pad.motion.form_flags >> 16) & 0xF;
        character_pad.motion.jump_phase = -1;
        character_pad.motion.vertical_speed = 0;
        character_pad.timers.state_ticks = 0;
        character_pad.timers.jump_duration_ticks = val;
        character_pad.flags = cur_flags | 0x8000000;
        if ((kind != 1 || (flags & 0x4000000) == 0) && (path_flags & 0x100) == 0 && (character_pad.secondary_flags & 0x80) == 0)
            character_pad.motion.state_code = 6;
        goto finish_motion;
    }
    horizontal = definition->pos_track.frame_count;
    definition = path_definition;
    if (definition != 0 && definition->transform != 0)
    {
        cur_flags = pos.y;
        kind = pos.z;
        original = pos;
        PushMatrix();
        path_transform = path_definition->transform;
        {
            SetRotMatrix(&path_transform->matrix);
        }
        {
            VECTOR sdk_input = {original.x, original.y, original.z, 0};
            VECTOR sdk_output;
            ApplyRotMatrixLV(&sdk_input, &sdk_output);
            sample_pos.x = sdk_output.vx;
            sample_pos.y = sdk_output.vy;
            sample_pos.z = sdk_output.vz;
        }
        PopMatrix();
        path_transform = path_definition->transform;
        val = sample_pos.x;
        val += (uint32)path_transform->matrix.t[0];
        sample_pos.x = val;
        path_transform = path_definition->transform;
        val = sample_pos.y;
        val += (uint32)path_transform->matrix.t[1];
        sample_pos.y = val;
        path_transform = path_definition->transform;
        val = sample_pos.z;
        val += (uint32)path_transform->matrix.t[2];
        sample_pos.z = val;
    }
    else
    {
        cur_flags = pos.y;
        kind = pos.z;
        sample_pos = pos;
    }
    val = sample_pos.x;
    val += (uint32)velocity.x;
    sample_pos.x = val;
    val = sample_pos.y;
    val += (uint32)velocity.y;
    sample_pos.y = val;
    val = sample_pos.z;
    val += (uint32)velocity.z;
    sample_pos.z = val;
    definition = character_pad.path->definition;
    frame = horizontal;
    dir = definition->ProjectFrame(frame, sample_pos, floor, flags & 0x10, 6144);
    if (dir == (sint32)horizontal)
    {
        flags = flags & 0xFFFFFBFF;
        character_pad.flags = character_pad.flags | 0x1000;
    }
finish_motion:
    if ((flags & 0x400) != 0 && character_pad.motion.jump_phase == 0)
    {
        cur_flags = character_pad.flags;
        character_pad.motion.jump_phase = -1;
        character_pad.flags = cur_flags & 0xEFFFFFFF;
    }
finish:

    character_pad.support.contact = 0;
    camera_path = camera_motion.path;
    if ((camera_path->definition->flags & 0x100011) != 0)
        character_pad.motion.slope_scale = 0;
    else
        character_pad.motion.slope_scale = camera_path->slope_scale;
    result = character_pad.flags & 0xF855B041;
    character_pad.flags = result;
    return result;
}

void AnimatedObject::UpdateMovementImpulse(uint32 *move_flags)
{
    const SceneTrigger *surface;

    FUNCTION_MARKER(0x8004D5C0, "SLUS_005.78");
    Vec3 vector;
    uint32 cur_flags, val, mask, mode, snapshot, i, result;
    sint32 idx = 0, strength, applied;
    if ((character_pad.flags & 4) != 0)
    {
        ResolveSurfaceMotion(move_flags);
        character_pad.flags = character_pad.flags & 0xFFFFFFFB;
    }
    cur_flags = character_pad.flags;
    if ((cur_flags & 0x18040) != 0)
    {
        if ((character_pad.support.owner->flags & 1) != 0)
            character_pad.ClearTransientFlags();
        else if ((cur_flags & 0x10000) != 0)
        {
            if ((character_pad.secondary_flags & 0x80) == 0)
                character_pad.motion.state_code = 8;
            *move_flags = *move_flags & 0xFFFFFBFE;
        }
    }
    mode = character_pad.flags & 0x60000;
    if (mode == 0x40000)
    {
        val = character_pad.timers.contact_impulse_ticks;
        character_pad.timers.contact_impulse_ticks = val - 1;
        if (val == 0)
        {
            cur_flags = character_pad.flags;
            character_pad.timers.contact_impulse_ticks = 0;
            character_pad.flags = cur_flags & 0xFFFBFFFF;
            flags = flags & 0xEFFFFFFF;
            character_pad.flags = character_pad.flags & 0xFFEFFFFF;
        }
        else
            goto suppress;
    }
    else if (mode == 0x60000 && (flags & 0x400) != 0)
    {
    suppress:
        *move_flags = 0;
        mask = (*control_bindings.active_scheme)[1];
        gameplay_input.buttons = gameplay_input.buttons & ~mask;
        mask = (*control_bindings.active_scheme)[1];
        gameplay_input.prev_buttons &= ~mask;
    }
    if ((character_pad.flags & 0x80000) != 0)
        *move_flags = 0;
    if ((path_flags & 0x101) != 0 && (character_pad.flags & 0x20000) != 0)
        character_pad.flags = character_pad.flags & 0xFFFDFFFF;
    cur_flags = character_pad.flags;
    if ((cur_flags & 0x810) == 0 && (cur_flags & 0x60000) != 0x20000)
    {
        result = flags & 0x400;
        if (result == 0)
        {
            result = path_flags & 0x1220;
            if (result == 0)
            {
                result = character_pad.model->movement.rise_speed_limit;
                character_pad.motion.vertical_speed = 0;
                character_pad.movement.rise_speed_limit = result;
            }
        }
        goto done;
    }
    if ((flags & 0x400) == 0)
        *move_flags = *move_flags | 8;
    val = (uint16)(character_pad.motion.form_flags >> 16);
    character_pad.motion.jump_phase = 1;
    character_pad.timers.state_ticks = 0;
    if ((val & 0xF) != 4 || (flags & 0x8000000) == 0)
        frame = -1;
    if (((uint16)(character_pad.motion.form_flags >> 16) & 0xF) == 1)
    {
        cur_flags = flags;
        if ((cur_flags & 0x4000000) != 0)
        {
            flags = cur_flags & 0xFBFFFFFF;
            if (character_pad.motion.state_code == 9 && (character_pad.secondary_flags & 0x80) == 0)
                character_pad.motion.state_code = 2;
            for (i = 0; i < 4; ++i)
                character_pad.movement.speed_limits[i] = character_pad.special_move_backup[i];
            character_pad.motion.horizontal_speed = character_pad.special_move_backup[1];
            character_pad.special_move_backup[1] = -15;
        }
    }
    val = character_pad.movement.jump_ticks;
    cur_flags = character_pad.flags;
    character_pad.timers.jump_duration_ticks = val;
    if ((cur_flags & 0x60000) == 0x20000)
    {
        *move_flags = *move_flags | 8;
        character_pad.flags = character_pad.flags | 0x2000;
        ApplyContactImpulse(move_flags);
        goto done;
    }
    val = character_pad.model->movement.rise_speed_limit;
    cur_flags = character_pad.flags;
    character_pad.timers.jump_duration_ticks = 7;
    character_pad.movement.rise_speed_limit = val;
    if ((cur_flags & 0x10) != 0)
    {
        idx = (sint16)character_pad.support.contact_dir - 1;
        surface = character_pad.support.impulse_surface;
        val = surface->transform.m[idx >> 1][0];
        if ((idx & 1) == 0)
            val = -val;
        vector.x = val;
        surface = character_pad.support.impulse_surface;
        val = surface->transform.m[idx >> 1][1];
        if ((idx & 1) == 0)
            val = -val;
        vector.y = val;
        surface = character_pad.support.impulse_surface;
        val = surface->transform.m[idx >> 1][2];
        if ((idx & 1) == 0)
            val = -val;
        vector.z = val;
        strength = character_pad.support.impulse_surface->parameters.impulse_strength;
    }
    else
    {
        vector.z = 0;
        vector.x = 0;
        vector.y = 4096;
        velocity.y = 0;
        if ((character_pad.flags & 0x200000) == 0 && character_pad.support.impulse_scale == 0 && character_pad.path_transition.contact_delay == 4)
        {
            AnimatedObject::QueueCharacterActionSound(7, 3);
            AnimatedObject::SpawnActionEffect(this, &character_pad, 1);
        }
        strength = character_pad.movement.jump_speed;
        val = character_pad.support.impulse_scale;
        character_pad.support.motion.z = 0;
        character_pad.support.motion.y = 0;
        character_pad.support.motion.x = 0;
        if (val != 0)
            strength = mul12(strength, (sint32)val);
    }
    val = character_pad.timers.jump_input_ticks < 3;
    character_pad.motion.buffered_rise_limit = 0;
    if (val != 0)
    {
        character_pad.timers.jump_input_ticks = 6;
        *move_flags = *move_flags | 8;
    }
    else if ((*move_flags & 8) == 0)
    {
        character_pad.timers.jump_input_ticks = 3;
        *move_flags = *move_flags | 8;
        ApplyDirectionalImpulse(move_flags, idx, vector, strength, ((uint32)strength << 1));
        if ((character_pad.flags & 0x10) != 0 && character_pad.motion.vertical_speed < character_pad.movement.jump_speed)
            character_pad.motion.vertical_speed = character_pad.movement.jump_speed;
        cur_flags = character_pad.movement.rise_speed_limit;
        val = character_pad.motion.horizontal_speed;
        mode = character_pad.motion.vertical_speed;
        snapshot = character_pad.motion.vertical_speed;
        character_pad.motion.buffered_rise_limit = cur_flags;
        character_pad.motion.buffered_horizontal_speed = val;
        character_pad.motion.buffered_vertical_speed = mode;
        if ((sint32)cur_flags < (sint32)snapshot)
            character_pad.motion.buffered_rise_limit = snapshot;
    }
    snapshot = *move_flags;
    applied = strength;
    if ((*move_flags & 8) == 0)
        applied = strength >> 2;
    ApplyDirectionalImpulse(move_flags, snapshot, vector, applied, ((uint32)strength << 1));
    if ((character_pad.flags & 0x10) != 0 && character_pad.motion.vertical_speed < character_pad.movement.jump_speed)
        character_pad.motion.vertical_speed = character_pad.movement.jump_speed;
    val = character_pad.motion.vertical_speed;
    if (character_pad.movement.rise_speed_limit < (sint32)val)
        character_pad.movement.rise_speed_limit = val;
    if ((snapshot & 8) != 0)
    {
        if (character_pad.motion.jump_phase == 1)
            character_pad.motion.jump_phase = 2;
        character_pad.flags = character_pad.flags | 0x10000000;
    }
    *move_flags = *move_flags | 8;
    result = (character_pad.flags | 0x2000) & 0xF7FFFFFF;
    character_pad.flags = result;
done:
    return;
}

intptr AnimatedObject::UpdateCharacterMovement()
{
    FUNCTION_MARKER(0x8004DCD0, "SLUS_005.78");
    uint32 prev_kind = (uint8)character_pad.motion.state_code;
    sint32 timer = (sint16)character_pad.timers.score_bonus_ticks;
    const ScenePath *path;
    uint32 move_flags, cur_flags, val, input, mask, result;
    character_pad.motion.prev_state_code = prev_kind;
    cur_flags = character_pad.movement_flags;
    path = camera_motion.path;
    character_pad.movement_flags = cur_flags & 0xF83FFFFF;
    character_pad.path = path;
    if (timer != 0)
    {
        val = (uint32)timer - 1;
        character_pad.timers.score_bonus_ticks = val;
        if ((val & 0xFFFF) == 0)
            character_pad.flags = character_pad.flags & 0xFFBFFFFF;
    }
    cur_flags = game_flow.transition_flags;
    input = gameplay_input.buttons;
    gameplay_input.prev_buttons = input;
    if ((cur_flags & 0x102) != 0)
    {
        gameplay_input.buttons = 0;
        GameplayInput::ReadGamepadButtons();
    }
    else
    {
        val = GameplayInput::ReadGamepadButtons();
        gameplay_input.buttons = GameplayInput::FilterDebugInput(val);
    }
    mask = gameplay_input.release_mask;
    if (mask != 0)
    {
        input = gameplay_input.buttons;
        gameplay_input.release_mask = mask & input;
        gameplay_input.buttons = input & ~mask;
    }
    if ((sint32)CutscenePlayback::GetCutsceneMode() < 0)
        gameplay_input.buttons = gameplay_input.buttons & 0x900;
    gameplay_input.ApplyDemo();
    cur_flags = character_pad.secondary_flags;
    if ((cur_flags & 0x400) != 0 && frame == animation->frame_count - 1)
        character_pad.secondary_flags = cur_flags & 0xFFFFFBFF;
    val = BuildMovementFlags();
    cur_flags = flags;
    move_flags = val;
    if ((cur_flags & 0x8000000) != 0 && ((uint16)(character_pad.motion.form_flags >> 16) & 0xF) == 4)
        move_flags = val & 0xFFFFFFF7;
    UpdateMovementImpulse(&move_flags);
    UpdateJumpMotion(&move_flags);
    UpdateSpecialMovement(&move_flags);
    UpdateMovementState(&move_flags);
    UpdateActionInput(&move_flags);
    result = IntegrateCharacterMovement(move_flags);
    return result;
}

void AnimatedObject::LeaveCharacterSupport()
{
    FUNCTION_MARKER(0x8004DF14, "SLUS_005.78");
    Vec3 vector;
    const PathDefinition *parent;
    uint32 result, x, y, z;
    PathTransform *path_transform;
    result = flags & 0x2000;
    if (result)
        return;
    parent = path_definition;
    path_transform = parent != 0 ? parent->transform : 0;
    if (path_transform)
    {
        vector = pos;
        PushMatrix();
        path_transform = path_definition->transform;
        SetRotMatrix(&path_transform->matrix);
        {
            VECTOR rotation_output;
            {
                VECTOR sdk_input = {vector.x, vector.y, vector.z, 0};
                ApplyRotMatrixLV(&sdk_input, &rotation_output);
            }
            pos.x = rotation_output.vx;
            pos.y = rotation_output.vy;
            pos.z = rotation_output.vz;
        }
        PopMatrix();
        path_transform = path_definition->transform;
        x = pos.x;
        y = path_transform->matrix.t[0];
        parent = path_definition;
        pos.x = x + y;
        path_transform = parent->transform;
        x = pos.y;
        y = path_transform->matrix.t[1];
        parent = path_definition;
        pos.y = x + y;
        path_transform = parent->transform;
        x = pos.z;
        y = path_transform->matrix.t[2];
        pos.z = x + y;
    }
    flags = flags | 0x2400;
    x = character_pad.path_velocity.vx;
    y = character_pad.path_velocity.vy;
    z = character_pad.path_velocity.vz;
    character_pad.path_tangent.vx = -x;
    character_pad.path_tangent.vy = -y;
    character_pad.path_tangent.vz = -z;
    character_pad.path_tangent.vy = 0;
    if ((uint32)character_pad.path_tangent.vx + 26754 > 0xD103 || (uint32)character_pad.path_tangent.vz + 26754 > 0xD103)
    {
        for (;;)
        {
            x = character_pad.path_tangent.vx;
            z = character_pad.path_tangent.vz;
            character_pad.path_tangent.vx = (sint32)x >> 1;
            y = character_pad.path_tangent.vy;
            x = character_pad.path_tangent.vx;
            z = (sint32)z >> 1;
            character_pad.path_tangent.vz = z;
            character_pad.path_tangent.vy = (sint32)y >> 1;
            if (x + 26754 > 0xD103)
                continue;
            if ((uint32)character_pad.path_tangent.vy + 26754 > 0xD103)
                continue;
            if (z + 26754 <= 0xD103)
                break;
        }
    }
    VectorNormal(&character_pad.path_tangent, &character_pad.path_tangent);
}

const ScenePath *AnimatedObject::FindCharacterSupportPath(const SceneTrigger *support)
{
    const ScenePath *support_first, *support_second, *anchor_target;

    FUNCTION_MARKER(0x8004E118, "SLUS_005.78");
    const ScenePath *camera_path;
    const ScenePath *start, *cur, *backup, *prev, *next, *result;
    Vec3 floor;
    uint32 query_frame;
    uint32 backup_frame;
    Vec3 sample_pos, input, transform;
    const ScenePath *visited[7];

    MATRIX transpose = {0};

    const PathDefinition *owner, *parent;
    uint32 cur_flags, dir;
    PathTransform *path_transform;
    uint32 x;

    uint32 z;
    uint32 a;
    sint32 b;
    uint32 val;
    uint32 frame;
    uint32 steps;
    uint32 idx;
    sint32 height_delta;
    uint32 need_transition;
    sint32 travel_dir;
    cur_flags = path_flags;
    steps = 0;
    if ((cur_flags & 0x400000) != 0)
    {
        if (support != 0)
        {
            const SceneTrigger *anchor = character_pad.path_transition.anchor;
            support_first = support->parameters.path_pair.targets[0];
            anchor_target = anchor->parameters.path.target;
            support_second = support->parameters.path_pair.targets[1];
            if (support_first == anchor_target || support_second == anchor_target)
                goto failed;
        }
    }
    else if ((cur_flags & 0x20) != 0 && support != 0 && path_transition_rules.release_mode == 1)
    {
        support_first = support->parameters.path_pair.targets[0];
        const ScenePath *anchor = character_pad.path_transition.anchor_path;
        support_second = support->parameters.path_pair.targets[1];
        if (support_first == anchor || support_second == anchor)
            goto failed;
    }
    cur_flags = flags;
    dir = cur_flags & 0x10;
    if (support != 0)
    {
        cur_flags &= 0x2000;
        if (dir != 0)
        {
            start = support->parameters.path_pair.targets[0];
        }
        else
        {
            start = support->parameters.path_pair.targets[1];
            cur_flags = flags & 0x2000;
        }
    }
    else
    {
        z = velocity.z;
        x = velocity.x;
        start = character_pad.path;
        val = Atan2Angle(z, x);
        query_frame = val;
        if (val == 4096)
            travel_dir = 0;
        else
        {
            a = target_yaw;
            travel_dir = -1;
            if (((val - a - 1024) & 0xFFF) < 2048)
                travel_dir = 1;
        }
        cur_flags = flags & 0x2000;
    }
    if (cur_flags == 0 && (parent = path_definition) != 0 && parent->transform != 0)
    {
        input = pos;
        cur = start;
        PushMatrix();
        path_transform = path_definition->transform;
        SetRotMatrix(&path_transform->matrix);
        {
            VECTOR sdk_input = {input.x, input.y, input.z, 0};
            VECTOR sdk_output;
            ApplyRotMatrixLV(&sdk_input, &sdk_output);
            sample_pos.x = sdk_output.vx;
            sample_pos.y = sdk_output.vy;
            sample_pos.z = sdk_output.vz;
        }
        backup = 0;
        PopMatrix();
        path_transform = path_definition->transform;
        val = sample_pos.x;
        sample_pos.x = val + path_transform->matrix.t[0];
        path_transform = path_definition->transform;
        val = sample_pos.y;
        sample_pos.y = val + path_transform->matrix.t[1];
        path_transform = path_definition->transform;
        val = sample_pos.z;
        sample_pos.z = val + path_transform->matrix.t[2];
    }
    else
    {
        sample_pos = pos;
        cur = start;
        backup = 0;
    }
    prev = 0;
    visited[0] = 0;
    for (idx = 6; idx != 0; --idx)
        visited[idx] = 0;
walk:
    val = support_search_limit;
    ++steps;
    if ((sint32)steps >= (sint32)val)
        goto failed;
    for (idx = 0; idx < 7; ++idx)
    {
        if (visited[idx] == cur)
            goto failed;
    }
    for (idx = 0; idx < 7; ++idx)
    {
        if (visited[idx] == 0 && cur != start)
        {
            visited[idx] = cur;
            break;
        }
    }
    frame = cur->definition->FindHeightFrame(sample_pos, dir, -40960);
    query_frame = frame;
    if ((sint32)frame < 0)
        goto failed;
    frame = cur->definition->ProjectFrame(frame, sample_pos, floor, dir, 4096);
    owner = cur->definition;
    a = floor.y;
    b = sample_pos.y;
    val = owner->pos_track.frame_count - 1;
    height_delta = a - b;
    need_transition = (sint32)(frame - 1) >= (sint32)val;
    if (need_transition == 0 && support == 0)
    {
        cur_flags = owner->flags;
        if ((cur_flags & 0x11) != 0 || (flags & 0x2000) != 0)
        {
            if (b < (sint32)a)
                need_transition = 1;
        }
        if (need_transition == 0 && cur != start && (cur->definition->flags & 0x11) == 0 && (prev->definition->flags & 0x11) != 0 && height_delta >= 1366)
            need_transition = 1;
    }
    if (need_transition != 0)
    {
        if (support != 0)
            goto failed;
        if ((flags & 0x2000) != 0)
        {
            if ((sint32)frame < 0)
                goto failed;
            if ((cur->definition->flags & 0x11) == 0)
            {
                a = floor.y;
                b = sample_pos.y;
                if (b < (sint32)a)
                    goto failed;
            }
        }
        if ((sint32)frame > 0)
        {
            owner = cur->definition;
            val = owner->pos_track.frame_count;
            if ((sint32)frame < (sint32)val && (owner->flags & 0x10) != 0)
            {
                next = cur->fallback;
                if (next == 0)
                    goto accept;
                if ((cur->node.state_flags & (dir | 8)) == 0 && (next->node.state_flags & 8) != 0)
                {
                    next = next->adjacent[0];
                }
                if (next == 0 || next == prev || next == start)
                    goto accept;
                goto advance;
            }
        }
        if ((sint32)frame >= 0)
        {
            val = (uint16)cur->definition->pos_track.frame_count;
            travel_dir = -1;
            if (((sint16)val >> 1) < (sint32)frame)
                travel_dir = 1;
        }
        query_frame = travel_dir;
        next = cur->Step(&query_frame, dir);
        if (next == 0 || next == prev || next == start)
        {
            cur = backup;
            if (backup == 0)
                goto failed;
            frame = backup_frame;
            cur->definition->ProjectFrame(frame, sample_pos, floor, dir, 4096);
            goto accept;
        }
    advance:
        if ((sint32)frame >= 0 && backup == 0)
        {
            backup = cur;
            backup_frame = frame;
        }
        frame = 0xFFFFFFFF;
        prev = cur;
        cur = next;
    }
    if ((sint32)frame < 0)
        goto walk;
accept:
    owner = cur->definition;
    path_frame = frame;
    path_definition = owner;
    owner = cur->definition;
    a = floor.y;
    b = sample_pos.y;
    cur_flags = owner->flags;
    height_delta = a - b;
    if ((cur_flags & 0x11) != 0 || (((flags & 0x400) != 0 || cur != start) && height_delta < -681))
    {
        owner = cur->definition;
        if ((owner->flags & 0x10000) == 0)
            goto leave_ground;
        camera_path = camera_motion.path;
        parent = camera_path->definition;
        if ((parent->flags & 0x10000) != 0)
            goto leave_ground;
    }
    flags = flags | 0x4000;
    goto after_ground;
leave_ground:
    flags = (flags & 0xFFFFBFFF) | 0x400;
    floor.y = sample_pos.y;
after_ground:
    cur_flags = path_flags;
    if ((cur_flags & 2) != 0)
    {
        val = flags;
        if ((val & 0x4000) != 0 && (cur_flags & 4) == 0)
            flags = val & 0xFFFFBFFF;
    }
    if ((path_flags & 0x20) != 0)
    {
        ClearCharacterPathRelease();
        ClearCharacterPathStep();
    }
    flags = flags & 0xFFFFD7FF;
    if ((cur->definition->flags & 0x10000) != 0)
    {
        x = floor.x;
        z = floor.z;
        sample_pos.x = x;
        sample_pos.z = z;
    }
    cur_flags = flags;
    if ((cur_flags & 0x4400) == 0x400 || (contact_kind == 1 && support == 0 && (cur_flags & 0x4000) == 0))
    {
        x = sample_pos.x;
        z = sample_pos.z;
        floor.x = x;
        floor.z = z;
    }
    parent = path_definition;
    if (parent != 0 && (path_transform = parent->transform) != 0)
    {
        val = floor.x;
        transform.x = val - path_transform->matrix.t[0];
        path_transform = path_definition->transform;
        val = floor.y;
        transform.y = val - path_transform->matrix.t[1];
        path_transform = path_definition->transform;
        val = floor.z;
        transform.z = val - path_transform->matrix.t[2];
        for (idx = 0; idx < 9; ++idx)
        {
            path_transform = path_definition->transform;
            transpose.m[idx / 3][idx % 3] = path_transform->matrix.m[idx % 3][idx / 3];
        }
        PushMatrix();
        SetRotMatrix(&transpose);
        {
            VECTOR rotation_output;
            {
                VECTOR sdk_input = {transform.x, transform.y, transform.z, 0};
                ApplyRotMatrixLV(&sdk_input, &rotation_output);
            }
            pos.x = rotation_output.vx;
            pos.y = rotation_output.vy;
            pos.z = rotation_output.vz;
        }
        PopMatrix();
    }
    else
    {
        pos = floor;
    }
    result = cur;
    goto finish_support;
failed:
    result = 0;
finish_support:
    return result;
}

void AnimatedObject::ApplyCharacterContactMotion()
{
    FUNCTION_MARKER(0x8004EB74, "SLUS_005.78");
    const ScenePath *support, *cur, *saved_path;
    Vec3 motion0, parent_velocity, input, tmp;

    MATRIX transpose = {0};

    const PathDefinition *parent, *definition, *other_definition;
    uint32 x, y, z, cur_flags, fraction, dir, kind, moved, was_grounded, was_detached, special;
    uint32 same_definition;
    PathTransform *path_transform;
    uint32 saved_ticks0, saved_ticks1, saved_ticks2;
    sint32 frame;
    if ((path_flags & 0x100) != 0)
        BeginCharacterPathTransfer();

    motion0 = velocity;
    if ((sint32)path_motion_scale < 4096)
    {
        character_pad.flags = character_pad.flags & 0xFFFFEFFF;
        parent_velocity.z = 0;
        parent_velocity.y = 0;
        parent_velocity.x = 0;
        cur_flags = flags;
        flags = cur_flags & 0xFFFFF7FF;
        if ((cur_flags & 0x4000) != 0)
        {
            if ((path_flags & 2) != 0 && (cur_flags & 0x400) != 0)
            {
                cur = camera_motion.path;
                definition = cur->definition;
                if ((definition->flags & 0x10000) == 0)
                    ClearCharacterSurfaceMotion();
            }

            cur = camera_motion.path;
            definition = cur->definition;
            if ((definition->flags & 0x80000) == 0)
            {
                if ((path_flags & 0x20) != 0)
                    ClearCharacterPathRelease();
                if ((path_flags & 0x10) != 0)
                    EndCharacterPathHold();
            }
            if ((path_flags & 0x200) != 0)
                CancelCharacterPathTransition();
            if ((path_flags & 0x1000) != 0)
                ClearCharacterPathAuxiliaryMode();
            character_pad.support.motion.z = 0;
            character_pad.support.motion.y = 0;
            character_pad.support.motion.x = 0;
            flags = flags & 0xFFFFFBFF;
        }
        else
        {
            parent = path_definition;
            if (parent != 0)
            {
                path_transform = parent->transform;
                if (path_transform != 0 && (cur_flags & 0x2000) == 0)
                {
                    parent_velocity = path_transform->displacement;
                }
            }
        }
        x = motion0.x + (uint32)parent_velocity.x;
        y = motion0.y + (uint32)parent_velocity.y;
        z = motion0.z + (uint32)parent_velocity.z;
        motion0.x = x;
        motion0.y = y;
        motion0.z = z;
        fraction = path_motion_scale;
        if (fraction != 0 || (path_flags & 6) != 2)
        {
            motion0.x = mul12((sint32)x, (sint32)fraction);
            motion0.y = mul12((sint32)y, (sint32)fraction);
            motion0.z = mul12((sint32)z, (sint32)fraction);
        }
        x = motion0.x - (uint32)parent_velocity.x;
        y = motion0.y - (uint32)parent_velocity.y;
        z = motion0.z - (uint32)parent_velocity.z;
        motion0.x = x;
        motion0.y = y;
        motion0.z = z;
        fraction = 4096 - path_motion_scale;
        if ((character_pad.flags & 0x18040) != 0)
        {
            cur_flags = character_pad.flags;
            character_pad.support.motion.z = 0;
            character_pad.support.motion.x = 0;
            if ((cur_flags & 0x18000) != 0)
                character_pad.support.motion.y = 0;
        }
        x = mul12(character_pad.support.motion.x, (sint32)fraction);
        y = character_pad.support.motion.y;
        character_pad.support.motion.x = x;
        y = mul12((sint32)y, (sint32)fraction);
        z = character_pad.support.motion.z;
        character_pad.support.motion.y = y;
        character_pad.support.motion.z = mul12((sint32)z, (sint32)fraction);
        motion0.x = motion0.x + (uint32)character_pad.support.motion.x;
        motion0.y = motion0.y + (uint32)character_pad.support.motion.y;
        motion0.z = motion0.z + (uint32)character_pad.support.motion.z;
    }
    moved = motion0.x != 0 || motion0.z != 0;
    if (moved != 0)
        character_pad.movement_flags = character_pad.movement_flags | 0x2000000;
    else if (motion0.y <= 0)
        goto rotate_velocity;
    if ((game_tick & 1) != 0 && character_pad.motion.trail_effect >= 0 && (path_flags & 0x400011) == 0)
    {
        cur = camera_motion.path;
        definition = cur->definition;
        if ((definition->flags & 0x80000) == 0)
        {
            if ((flags & 0x2000) != 0)
                goto effect_pos;
            parent = path_definition;
            if (parent == 0 || parent->transform == 0)
                goto effect_pos;

            input = pos;
            PushMatrix();
            path_transform = path_definition->transform;
            SetRotMatrix(&path_transform->matrix);
            {
                VECTOR sdk_input = {input.x, input.y, input.z, 0};
                VECTOR sdk_output;
                ApplyRotMatrixLV(&sdk_input, &sdk_output);
                parent_velocity.x = sdk_output.vx;
                parent_velocity.y = sdk_output.vy;
                parent_velocity.z = sdk_output.vz;
            }
            PopMatrix();
            path_transform = path_definition->transform;
            x = parent_velocity.x;
            parent_velocity.x = x + path_transform->matrix.t[0];
            path_transform = path_definition->transform;
            y = parent_velocity.y;
            parent_velocity.y = y + path_transform->matrix.t[1];
            path_transform = path_definition->transform;
            z = parent_velocity.z;
            parent_velocity.z = z + path_transform->matrix.t[2];
            goto emit_effect;
        effect_pos:
            x = (uint32)pos.x;
            y = pos.y;
            z = pos.z;
            parent_velocity.x = x;
            parent_velocity.y = y;
            parent_velocity.z = z;
        emit_effect:
            kind = (uint16)(character_pad.motion.form_flags >> 16) & 15;
            if (kind != 5 && kind - 10 >= 2)
                SpriteRecord::SpawnEffectSpr(parent_velocity.comp, 4096, character_pad.motion.trail_effect + 7);
        }
    }
rotate_velocity:
    if ((flags & 0x2000) != 0)
    {
        if (path_motion_scale == 4096)
            path_motion_scale = 4095;
    }
    else
    {
        parent = path_definition;
        if (parent != 0)
        {
            path_transform = parent->transform;
            if (path_transform != 0)
            {
                transpose.m[0][0] = path_transform->matrix.m[0][0];
                transpose.m[0][1] = path_transform->matrix.m[1][0];
                transpose.m[0][2] = path_transform->matrix.m[2][0];
                transpose.m[1][0] = path_transform->matrix.m[0][1];
                transpose.m[1][1] = path_transform->matrix.m[1][1];
                transpose.m[1][2] = path_transform->matrix.m[2][1];
                transpose.m[2][0] = path_transform->matrix.m[0][2];
                transpose.m[2][1] = path_transform->matrix.m[1][2];
                transpose.m[2][2] = path_transform->matrix.m[2][2];

                input = motion0;
                PushMatrix();
                SetRotMatrix(&transpose);
                {
                    VECTOR sdk_input = {input.x, input.y, input.z, 0};
                    VECTOR sdk_output;
                    ApplyRotMatrixLV(&sdk_input, &sdk_output);
                    motion0.x = sdk_output.vx;
                    motion0.y = sdk_output.vy;
                    motion0.z = sdk_output.vz;
                }
                PopMatrix();
            }
        }
    }
    pos.x = pos.x + (uint32)motion0.x;
    pos.y = pos.y + (uint32)motion0.y;
    pos.z = pos.z + (uint32)motion0.z;
    if ((flags & 0x4000) != 0 || (path_flags & 0x100) != 0)
        goto final_pos;
    cur_flags = character_pad.flags;
    if ((cur_flags & 0x22) != 0)
        goto final_pos;
    fraction = path_motion_scale;
    if ((sint32)fraction >= 4096 && ((cur_flags & 1) == 0 || (sint32)fraction < (sint32)character_pad.support.fraction))
        goto final_pos;

    camera_motion.frame = path_frame;
    cur_flags = flags;
    x = character_pad.flags;
    was_grounded = (cur_flags >> 10) & 1;
    was_detached = (cur_flags >> 13) & 1;
    if ((x & 1) == 0 || (sint32)path_motion_scale < (sint32)character_pad.support.fraction)
        character_pad.support.candidate = 0;
    moved = 1;
    support = FindCharacterSupportPath(character_pad.support.candidate);
    if (support == 0)
    {
        if ((flags & 0x2000) != 0 || character_pad.support.candidate != 0)
            goto grounded_transition;
        parent = path_definition;
        if (parent != 0)
        {
            path_transform = parent->transform;
            if (path_transform != 0)
            {
                x = velocity.x;
                y = path_transform->displacement.x;
                parent = path_definition;
                velocity.x = x + y;
                path_transform = parent->transform;
                x = velocity.y;
                y = path_transform->displacement.y;
                parent = path_definition;
                velocity.y = x + y;
                path_transform = parent->transform;
                x = velocity.z;
                velocity.z = x + path_transform->displacement.z;
            }
        }
        LeaveCharacterSupport();
        goto grounded_transition;
    }
    cur_flags = flags;
    if ((cur_flags & 0x4000) != 0)
    {
        if (character_pad.support.candidate != 0)
            character_pad.flags = character_pad.flags & 0xFCC5F459;
        else
        {
            flags = cur_flags & 0xFFFFBBFF;
            character_pad.flags = character_pad.flags & 0xF7FFFFFF;
        }
    }
    special = 0;
    if ((path_flags & 0x20) != 0 && support == character_pad.path_transition.anchor_path)
        special = 1;

    if (support == camera_motion.path || special != 0)
        goto grounded_transition;
    parent = path_definition;
    if (parent != 0 && parent->transform != 0)
    {
        tmp = pos;
        PushMatrix();
        path_transform = path_definition->transform;
        SetRotMatrix(&path_transform->matrix);

        {
            VECTOR rotation_output;
            {
                VECTOR sdk_input = {tmp.x, tmp.y, tmp.z, 0};
                ApplyRotMatrixLV(&sdk_input, &rotation_output);
            }
            camera_motion.focus_pos.x = rotation_output.vx;
            camera_motion.focus_pos.y = rotation_output.vy;
            camera_motion.focus_pos.z = rotation_output.vz;
        }
        PopMatrix();

        path_transform = path_definition->transform;
        x = camera_motion.focus_pos.x;
        camera_motion.focus_pos.x = x + path_transform->matrix.t[0];
        path_transform = path_definition->transform;
        y = camera_motion.focus_pos.y;
        camera_motion.focus_pos.y = y + path_transform->matrix.t[1];
        path_transform = path_definition->transform;
        z = camera_motion.focus_pos.z;
        camera_motion.focus_pos.z = z + path_transform->matrix.t[2];
    }
    else
    {
        camera_motion.focus_pos = pos;
    }
    frame = path_frame;
    if (was_detached != 0)
        flags = flags | 0x2000;
    else
    {
        cur = camera_motion.path;
        cur_flags = flags;
        definition = cur->definition;
        path_frame = definition->FindHeightFrame(camera_motion.focus_pos, cur_flags & 0x10, -40960);
    }

    if (support != camera_motion.path)
    {
        dir = flags & 0x10;
        support->ProjectOffset(frame, camera_motion.desired_pos, dir);

        cur = camera_motion.path;
        definition = support->definition;
        other_definition = cur->definition;
        same_definition = definition == other_definition;
        if (same_definition != 0)
            dir ^= 0x10;
        else
            ApplyPathVelocityChange(definition);

        if ((camera_motion.smooth_ticks[0] | camera_motion.smooth_ticks[1] | camera_motion.smooth_ticks[2]) == 0)
        {
            cur = camera_motion.path;
            if (cur == character_pad.path && (flags & 0x2000) == 0)
            {
                kind = path_frame;
                if ((sint32)kind >= 0)
                    (cur)->ProjectOffset(kind, camera_motion.pos, dir);
            }
        }
        CameraMotion::SelectScenePath(support);
    }
    cur_flags = flags;
    path_frame = frame;
    flags = cur_flags & 0xFFFFDFFF;
    x = character_pad.motion.vertical_speed;
    if (character_pad.movement.fall_speed_limit >= (sint32)x)
        character_pad.timers.state_ticks = 0;
    if (character_pad.support.candidate != 0 && (character_pad.flags & 0x8040) != 0)
        character_pad.ClearTransientFlags();
grounded_transition:
    if ((flags & 0x400) != 0 && was_grounded == 0)
    {
        x = character_pad.movement.jump_ticks;
        cur_flags = character_pad.flags;
        kind = (uint16)(character_pad.motion.form_flags >> 16);
        character_pad.motion.jump_phase = -1;
        character_pad.motion.vertical_speed = 0;
        character_pad.timers.state_ticks = 0;
        character_pad.timers.jump_duration_ticks = x;
        character_pad.flags = cur_flags | 0x8000000;
        if (((kind & 15) != 1 || (flags & 0x4000000) == 0) && (character_pad.secondary_flags & 0x80) == 0)
            character_pad.motion.state_code = 6;
    }
final_pos:

    cur = character_pad.path;
    if (camera_motion.path != cur)
    {
        tmp.x = camera_motion.desired_pos.x - (uint32)camera_motion.pos.x;
        tmp.y = camera_motion.desired_pos.y - (uint32)camera_motion.pos.y;
        tmp.z = camera_motion.desired_pos.z - (uint32)camera_motion.pos.z;
        CameraMotion::SetCameraTransition(tmp);

        cur = camera_motion.path;
        saved_path = character_pad.path;
        definition = cur->definition;
        other_definition = saved_path->definition;
        same_definition = definition == other_definition;
        if (same_definition == 0 && (camera_motion.smooth_ticks[0] | camera_motion.smooth_ticks[1] | camera_motion.smooth_ticks[2]) != 0)
        {
            saved_ticks0 = camera_motion.smooth_ticks[0];
            saved_ticks1 = camera_motion.smooth_ticks[1];
            saved_ticks2 = camera_motion.smooth_ticks[2];
            x = (uint32)velocity.x * 2;
            y = (uint32)velocity.y * 2;
            z = (uint32)velocity.z * 2;
            tmp.x = tmp.x - x;
            tmp.y = tmp.y - y;
            tmp.z = tmp.z - z;
            CameraMotion::SetCameraTransition(tmp);

            if (saved_ticks0 < camera_motion.smooth_ticks[0])
            {
                camera_motion.smooth_ticks[0] = saved_ticks0;
            }
            if (saved_ticks1 < camera_motion.smooth_ticks[1])
                camera_motion.smooth_ticks[1] = saved_ticks1;

            if (saved_ticks2 < camera_motion.smooth_ticks[2])
            {
                camera_motion.smooth_ticks[2] = saved_ticks2;
            }
        }
        moved = 1;
        camera_motion.flags = camera_motion.flags & 0xFE;
    }

    camera_motion.frame = path_frame;
    character_pad.motion.pos_changed = moved != 0 ? 1 : 0;
    cur_flags = character_pad.flags;
    x = character_pad.secondary_flags;
    character_pad.flags = cur_flags & 0xFFFFFFFE;
    character_pad.secondary_flags = x & 0xFFFFFFF6;
}

void AnimatedObject::DetachCharacterFromPath(Vec3 *dir)
{
    FUNCTION_MARKER(0x8004FA54, "SLUS_005.78");
    Vec3 vector;
    const PathDefinition *parent;
    MATRIX *matrix;
    uint32 cur_flags;
    uint32 x;
    sint32 y;
    uint32 z;
    uint32 frame;
    uint32 forward;
    uint32 reverse;
    uint32 val;
    uint32 clock;
    uint32 mode;
    uint32 result;
    ApplyPathVelocityChange(0);
    flags = (flags & 0xFFFFF7FF) | 0x2000;
    parent = path_definition;
    if (parent != 0 && parent->transform != 0)
    {
        vector = pos;
        PushMatrix();
        matrix = path_definition->transform != 0 ? &path_definition->transform->matrix : NULL;
        SetRotMatrix(matrix);
        {
            VECTOR rotation_output;
            {
                VECTOR sdk_input = {vector.x, vector.y, vector.z, 0};
                ApplyRotMatrixLV(&sdk_input, &rotation_output);
            }
            pos.x = rotation_output.vx;
            pos.y = rotation_output.vy;
            pos.z = rotation_output.vz;
        }
        PopMatrix();
        matrix = path_definition->transform != 0 ? &path_definition->transform->matrix : NULL;
        x = pos.x;
        y = matrix->t[0];
        parent = path_definition;
        pos.x = x + y;
        matrix = parent->transform != 0 ? &parent->transform->matrix : NULL;
        x = pos.y;
        y = matrix->t[1];
        parent = path_definition;
        pos.y = x + y;
        matrix = parent->transform != 0 ? &parent->transform->matrix : NULL;
        x = pos.z;
        y = matrix->t[2];
        pos.z = x + y;
    }
    frame = (uint16)path_frame;
    cur_flags = flags;
    forward = (cur_flags >> 4) & 1;
    reverse = (frame >> 15) ^ forward;
    path_definition->SampleTangent(&character_pad.path_tangent, (reverse << 4));
    if (forward != reverse)
    {
        x = character_pad.path_tangent.vx;
        z = character_pad.path_tangent.vz;
        character_pad.path_tangent.vx = -x;
        character_pad.path_tangent.vz = -z;
    }
    cur_flags = flags;
    if (cur_flags & 0x400)
    {
        x = character_pad.path_tangent.vx;
        character_pad.path_tangent.vy = 0;
        if (x + 26754 > 0xD103 || (uint32)character_pad.path_tangent.vz + 26754 > 0xD103)
        {
            for (;;)
            {
                x = character_pad.path_tangent.vx;
                z = character_pad.path_tangent.vz;
                character_pad.path_tangent.vx = (sint32)x >> 1;
                y = character_pad.path_tangent.vy;
                x = character_pad.path_tangent.vx;
                z = (sint32)z >> 1;
                character_pad.path_tangent.vz = z;
                character_pad.path_tangent.vy = y >> 1;
                if (x + 26754 > 0xD103)
                    continue;
                if ((uint32)character_pad.path_tangent.vy + 26754 > 0xD103)
                    continue;
                if (z + 26754 <= 0xD103)
                    break;
            }
        }
        VectorNormal(&character_pad.path_tangent, &character_pad.path_tangent);
    }
    else
    {
        flags = cur_flags | 0x400;
        clock = character_pad.movement.jump_ticks;
        val = character_pad.flags;
        mode = (uint16)(character_pad.motion.form_flags >> 16) & 15;
        character_pad.motion.jump_phase = -1;
        character_pad.motion.vertical_speed = 0;
        character_pad.timers.state_ticks = 0;
        character_pad.timers.jump_duration_ticks = clock;
        character_pad.flags = val | 0x08000000;
        result = 0;
        if (mode == 1)
            result = flags & 0x04000000;
        if (!result)
        {
            val = character_pad.secondary_flags;

            if (!(val & 0x80))
                character_pad.motion.state_code = 6;
        }
    }
    if (dir)
    {
        result = character_pad.path_tangent.vx;
        y = character_pad.path_tangent.vy;
        z = character_pad.path_tangent.vz;
        dir->x = result;
        dir->y = y;
        dir->z = z;
    }
}

void AnimatedObject::UpdateCharacterForm()
{
    FUNCTION_MARKER(0x8004FD44, "SLUS_005.78");
    const ScenePath *camera_path, *object_path;

    Vec3 pos, input;

    AnimatedObject *object_slot = this;
    AnimatedObject *cur;
    const PathDefinition *definition;
    MATRIX *matrix;
    uint32 val, flags, x, y, z, result, kind;
    if ((sint32)character_pad.flags >= 0 || (gameplay_input.buttons & 0x10) == 0 || (gameplay_input.prev_buttons & 0x10) != 0)
        goto check_form;
    val = character_pad.motion.form_flags;
    if (((val >> 16) & 15) >= 2)
        goto check_form;
    if (((val >> 24) & 15) != 15)
        goto change_form;
    if ((game_flow.transition_flags & 0x48) != 0)
        goto check_form;
    val &= 0xFF0FFFFF;
    val |= (1 - ((val >> 16) & 15)) << 20;
    val &= 0xF0FFFFFF;
    flags = character_pad.flags;
    val |= (val >> 20) << 24;
    character_pad.motion.form_flags = val;
    if ((flags & 0x80000000) == 0)
        goto check_form;
    character_pad.ClearTransientFlags();
    val = character_pad.motion.form_flags;
    character_pad.motion.jump_phase = 0;
    character_pad.motion.vertical_speed = 0;
    character_pad.motion.horizontal_speed = 0;
    val = ((val & 0xF0FFFFFF) | 0xF000000) & 0xFFF0FFFF;
    cur = object_slot;
    val |= ((val >> 20) & 15) << 16;
    character_pad.motion.form_flags = val;
    cur->movement_mode = 0;
    cur = object_slot;
    cur->contact_kind = 0;

    object_path = camera_motion.path;
    cur = object_slot;
    cur->scene_path = object_path;
    val = (character_pad.motion.form_flags >> 18) & 0x3C;
    {
        CharacterModelRecord *sel_model;
        {
            uint32 model_idx = val / 4;
            CharacterModelRecord *model_binding;
            model_binding = character_pad.model_slots[model_idx];
            sel_model = model_binding;
        }
        object_slot = sel_model->object;
    }
    cur = object_slot;
    cur->movement_mode = 1;
    cur = object_slot;
    cur->contact_kind = 1;
    cur = object_slot;
    cur->node.flags = cur->node.flags & 0xFE;
    cur = object_slot;
    if ((cur->flags & 0x2000) != 0)
        goto local_pos;
    definition = cur->path_definition;
    if (definition == 0 || definition->transform == 0)
        goto local_pos;

    input = cur->pos;
    PushMatrix();
    cur = object_slot;
    definition = cur->path_definition;
    matrix = definition->transform != 0 ? &definition->transform->matrix : NULL;
    SetRotMatrix(matrix);
    {
        VECTOR sdk_input = {input.x, input.y, input.z, 0};
        VECTOR sdk_output;
        ApplyRotMatrixLV(&sdk_input, &sdk_output);
        pos.x = sdk_output.vx;
        pos.y = sdk_output.vy;
        pos.z = sdk_output.vz;
    }
    PopMatrix();
    cur = object_slot;
    definition = cur->path_definition;
    matrix = definition->transform != 0 ? &definition->transform->matrix : NULL;
    x = pos.x;
    pos.x = x + matrix->t[0];
    definition = cur->path_definition;
    matrix = definition->transform != 0 ? &definition->transform->matrix : NULL;
    y = pos.y;
    pos.y = y + matrix->t[1];
    definition = cur->path_definition;
    matrix = definition->transform != 0 ? &definition->transform->matrix : NULL;
    z = pos.z;
    pos.z = z + matrix->t[2];
    goto pos_ready;
local_pos:
    cur = object_slot;

    pos = cur->pos;
pos_ready:
    pos.y = (uint32)pos.y + 12288;
    SpriteRecord::SpawnEffectSpr(pos.comp, 10240, 7);
    AnimatedObject::QueueCharacterActionSound(14, 3);
    cur = object_slot;
    flags = cur->flags;
    object_path = cur->scene_path;
    cur->flags = flags & 0xFFFFF7FF;
    CameraMotion::SelectScenePath(object_path);

    camera_motion.target = camera_motion.focus_pos;

    cur = object_slot;

    camera_motion.focus_pos = cur->pos;

    cur = object_slot;
    camera_path = camera_motion.path;
    flags = cur->flags;
    camera_path->ProjectOffset(cur->path_frame, camera_motion.desired_pos, flags & 0x10);

    camera_motion.focus_pos = camera_motion.target;

    camera_motion.smooth_ticks[2] = 15;
    camera_motion.smooth_ticks[1] = 15;
    camera_motion.smooth_ticks[0] = 15;
    cur = object_slot;

    camera_motion.target = cur->pos;
    flags = character_pad.flags;
    y = camera_motion.target.y;
    camera_motion.target.y = y + ((sint32)flags >= 0 ? 14336 : 32768);
    val = game_flow.transition_flags;
    flags = character_pad.secondary_flags;
    camera_motion.target_ticks = 15;
    game_flow.transition_flags = val | 0x100;
    if ((flags & 0x20) != 0)
    {
        AnimatedObject::BeginCharacterInteraction(object_slot);
        flags = character_pad.secondary_flags;
        val = game_flow.mode_flags;
        character_pad.secondary_flags = flags & 0xFFFFFFDF;
        if ((val & 0x200) != 0)
            goto ensure_form;
        AnimatedObject::SpawnActionEffect(object_slot, &character_pad, 6);
    }
    else if ((flags & 0x100) != 0)
    {
        character_pad.secondary_flags = (flags | 0x20) & 0xFFFFFEFF;
        character_pad.RemoveAction(6);
        character_pad.EndInteraction();
    }
    result = game_flow.mode_flags & 0x200;
    if (result == 0)
        goto done;
ensure_form:
    result = (uint16)(character_pad.motion.form_flags >> 16) & 15;
    if (result == 0)
    {
        result = character_pad.secondary_flags & 0x100;
        if (result == 0)
            AnimatedObject::BeginCharacterInteraction(object_slot);
    }
    goto done;
check_form:
    result = (uint8)(character_pad.motion.form_flags >> 24) & 15;
    if (result == 15)
        goto done;
change_form:
    val = character_pad.motion.form_flags;
    kind = (val >> 24) & 15;
    result = (val >> 16) & 15;
    if (kind == result)
        goto done;
    (object_slot != 0 ? &object_slot->node : 0)->Unregister();
    kind = (uint8)(character_pad.motion.form_flags >> 24) & 15;
    kind = CharacterModelRecord::AssignCharacterModel(&object_slot, kind);
    (object_slot != 0 ? &object_slot->node : 0)->Register();
    cur = object_slot;
    flags = cur->flags;
    if ((flags & 0x8000000) != 0 && character_pad.model->animations[7] == 0)
    {
        cur->flags = flags & 0xF7FFFFFF;
        cur = object_slot;
        flags = cur->flags;
    }
    if ((flags & 0x1000000) != 0 && character_pad.model->animations[5] == 0)
        cur->flags = flags & 0xFEFFFFFF;
    cur = object_slot;
    if ((cur->flags & 0x400) != 0 && character_pad.model->animations[3] == 0)
    {
        val = character_pad.movement.jump_ticks;
        flags = character_pad.flags;
        character_pad.motion.jump_phase = -1;
        character_pad.motion.vertical_speed = 0;
        character_pad.timers.state_ticks = 0;
        character_pad.timers.jump_duration_ticks = val;
        character_pad.flags = flags | 0x8000000;
    }
    character_pad.motion.prev_state_code = 38;
    cur = object_slot;
    if (cur->animation != 0)
        goto done;
    switch (kind)
    {
        case 3:
        case 10:
        case 11:
        case 12:
            if ((character_pad.secondary_flags & 0x80) == 0)
                character_pad.motion.state_code = 6;
            break;
        case 5:
        case 7:
            cur = object_slot;
            if ((cur->flags & 0x400) != 0 && (character_pad.secondary_flags & 0x80) == 0)
                character_pad.motion.state_code = 3;
            break;
        case 9:
            if ((character_pad.secondary_flags & 0x80) == 0)
                character_pad.motion.state_code = 2;
            break;
    }
    cur = object_slot;
    if (cur->animation == 0)
    {
        result = character_pad.secondary_flags & 0x80;
        if (result == 0)
            character_pad.motion.state_code = 0;
    }
done:
    return;
}

void AnimatedObject::PauseCharacterSound(void)
{
    FUNCTION_MARKER(0x800504E4, "SLUS_005.78");
    uint32 result;
    const SoundBankStorage *table;
    const SoundSample *sound;
    table = sound_resources.common_table;
    character_sound_resume_flags = 0;
    sound = 0;
    if (table && table->header.count >= 7)
        sound = table->samples + 6;
    if (sound_effects.IsPlaying(sound, character_pad.sound_voice) & 0xFF)
        character_sound_resume_flags = character_sound_resume_flags | 1;
    table = sound_resources.common_table;
    sound = 0;
    if (table && table->header.count >= 11)
        sound = table->samples + 10;
    if (sound_effects.IsPlaying(sound, character_pad.sound_voice) & 0xFF)
        character_sound_resume_flags = character_sound_resume_flags | 2;
    result = character_sound_resume_flags;
    if (result)
    {
        SoundEffects::StopSoundVoice(character_pad.sound_voice);
        result = 0xFFFFFFFF;
        character_pad.sound_voice = result;
    }
}

void AnimatedObject::ResumeCharacterSound(void)
{
    FUNCTION_MARKER(0x800505E4, "SLUS_005.78");
    uint32 flags = character_sound_resume_flags;
    const SoundSample *sound = 0;
    const SoundBankStorage *sounds;
    sint32 voice;
    if ((flags & 1) != 0)
    {
        sounds = sound_resources.common_table;
        if (sounds != 0 && sounds->header.count >= 7)
            sound = sounds->samples + 6;
    }
    else
    {
        if ((flags & 2) == 0)
            return;
        sounds = sound_resources.common_table;
        if (sounds != 0 && sounds->header.count >= 11)
            sound = sounds->samples + 10;
    }
    voice = sound_effects.SelectVoice(-1, sound, 1);
    character_pad.sound_voice = voice;
    voice = (sint8)voice;
    if (voice >= 0 && sound != 0)
    {
        const SoundSample *sample = sound;
        uint16 metadata = sample->metadata;
        uint16 pitch = sample->pitch;
        sint16 volume = sound_settings.effects_volume;
        SoundEffects::PlaySoundEffect(sound, voice, 0, metadata, pitch, volume, 0);
    }
}

uint32 AnimatedObject::ProcessCharacterPickups()
{
    SceneSection *section;
    CharacterModelRecord *state_rec;
    CharacterController *character = &character_pad;
    FUNCTION_MARKER(0x800506D4, "SLUS_005.78");
    SceneNode *event;
    BillboardNode *billboard;
    uint32 kind, marker, result, idx, val, cur_flags, total, action, prev, form;
    sint32 amount, count, health, health_limit;
    count = character_pad.pickup_count;
    val = (uint8)character_pad.pickup_count;
    if (count == 0)
        return 0;
    result = val - 1;
    do
    {
        character_pad.pickup_count = result;
        idx = (sint8)result;
        val = character_pad.pickup_fractions[(sint32)idx];
        if ((sint32)path_motion_scale < (sint32)val)
            goto next_event;
        idx = character_pad.pickup_count;
        event = character_pad.pickup_objects[(sint32)idx];
        billboard = (BillboardNode *)event;
        kind = billboard->playback_flags & 0x7F;
        val = event->flags;
        event->flags = val | 0x80;
        marker = kind;
        switch (kind)
        {
            case 0:
                val = character_pad.vitals.lives;
                if (val < 32)
                    character_pad.vitals.lives = val + 1;
                break;
            case 1:
                health = (sint8)character_pad.vitals.health;
                health_limit = (sint8)character_pad.vitals.health_limit;
                if (health < health_limit)
                    character_pad.vitals.health = health + 1;
                break;
            case 2:
                if ((sint8)character_pad.vitals.health_limit < 21)
                {
                    val = level_resource.level_id;
                    if (val == 75)
                        character_pad.vitals.health_pickups = character_pad.vitals.health_pickups | 1;
                    else if (val == 10)
                        character_pad.vitals.health_pickups = character_pad.vitals.health_pickups | 2;
                    else if (val == 20)
                        character_pad.vitals.health_pickups = character_pad.vitals.health_pickups | 4;
                    character_pad.vitals.health_limit = character_pad.vitals.health_limit + 4;
                }
                health = (sint8)character_pad.vitals.health;
                health_limit = (sint8)character_pad.vitals.health_limit;
                if (health_limit - 4 >= health)
                    character_pad.vitals.health = health + 4;
                break;
            case 3:
            case 4:
            case 5:
                idx = ((const PositionedBillboardNode *)event)->billboard.playback_flags & 0x7F;
                section = scene_stream.active_section;
                amount = pickup_scores[idx - 3];
                section->collected_count = section->collected_count + amount;
                cur_flags = character_pad.flags;
                total = character_pad.vitals.score;
                total += (cur_flags & 0x400000) != 0 ? (uint32)amount * 2 : (uint32)amount;
                idx = character_pad.pickup_count;
                character_pad.vitals.score = total;
                if (character_pad.pickup_fractions[(sint32)idx] == 0 && (player_interaction.flags & 1) != 0)
                {
                    cur_flags = character_pad.flags;
                    total += (cur_flags & 0x400000) != 0 ? (uint32)amount * 2 : (uint32)amount;
                    character_pad.vitals.score = total;
                }
                break;
            case 6:
                if (((uint16)(character_pad.motion.form_flags >> 16) & 15) == 1)
                {
                    state_rec = character_pad.model;
                    AnimationHeader *animation22 = state_rec->animations[22];
                    AnimationHeader *animation2 = state_rec->animations[2];
                    state_rec->animations[2] = animation22;
                    state_rec = character_pad.model;
                    state_rec->animations[22] = animation2;
                    AnimatedObject::SpawnActionEffect(this, &character_pad, 7);
                    character_pad.secondary_flags = character_pad.secondary_flags | 0x40;
                    flags = flags | 0x20000000;
                    character_pad.timers.model_swap_ticks = 450;
                    break;
                }
                goto change_form;
            case 7:
                if (((uint16)(character_pad.motion.form_flags >> 16) & 15) == 1)
                {
                    character_pad.secondary_flags = character_pad.secondary_flags | 0x200;
                    flags = flags | 0x20000000;
                    character_pad.timers.color_effect_ticks = 450;
                    break;
                }
            change_form:
                kind = (uint16)(character_pad.motion.form_flags >> 16) & 15;
                if (kind == 4 || kind == 5 || kind - 10 < 2)
                {
                    marker = 0xFFFFFFFF;
                    break;
                }
                form = marker == 7 ? 11 : 0;
                marker = 6;
                if ((character_pad.secondary_flags & 0x100) != 0)
                {
                    character->EndInteraction();
                    character_pad.RemoveAction(6);
                }
                else
                {
                    val = character_pad.weapon_state;
                    if ((val & 0xFFFF) != 0)
                    {
                        prev = val >> 16;
                        if (prev == form)
                            goto store_form;
                        character_pad.RemoveAction(prev == 0 ? 3 : prev - 7);
                    }
                }
                AnimatedObject::SpawnActionEffect(this, &character_pad, form == 0 ? 3 : form - 7);
            store_form:
                character_pad.weapon_state = (form << 16) | 1;
                break;
            case 8:
                marker -= 2;
                break;
            case 10:
                cur_flags = character_pad.secondary_flags;
                if ((cur_flags & 0x120) == 0)
                {
                    if (((uint16)(character_pad.motion.form_flags >> 16) & 15) == 1)
                        character_pad.secondary_flags = cur_flags | 0x20;
                    else
                    {
                        AnimatedObject::SpawnActionEffect(this, &character_pad, 6);
                        AnimatedObject::BeginCharacterInteraction(this);
                        val = character_pad.weapon_state;
                        if ((val & 0xFFFF) != 0)
                        {
                            prev = val >> 16;
                            character_pad.RemoveAction(prev == 0 ? 3 : prev - 7);
                        }
                        character_pad.weapon_state = 0;
                    }
                }
                break;
            case 11:
                AnimatedObject::SpawnActionEffect(this, &character_pad, 3);
                character_pad.weapon_state = 1;
                break;
        }
        while (character_pad.vitals.score >= 500)
        {
            val = character_pad.vitals.lives;
            if (val < 32)
                character_pad.vitals.lives = val + 1;
            val = character_pad.vitals.score;
            character_pad.vitals.score = val - 500;
            AnimatedObject::QueueCharacterActionSound(17, 3);
        }
        SpriteRecord::SpawnEffectSpr(((const PositionedBillboardNode *)event)->world_pos.comp, ((const PositionedBillboardNode *)event)->billboard.extent, 6);
        if ((sint8)marker != -1)
        {
            kind = ((const PositionedBillboardNode *)event)->billboard.playback_flags & 0x7F;
            switch (kind)
            {
                case 0:
                    action = 17;
                    break;
                case 1:
                case 2:
                    action = 18;
                    break;
                case 3:
                case 4:
                case 5:
                    action = 19;
                    break;
                case 6:
                case 7:
                    action = 20;
                    break;
                case 8:
                    action = 21;
                    break;
                default:
                    action = ((uint16)(character_pad.motion.form_flags >> 16) & 15) == 1 ? 11 : 15;
                    break;
            }
            AnimatedObject::QueueCharacterActionSound(action, 3);
        }
        if (billboard->event_id != 0)
            billboard->playback_flags |= 0x8000;
    next_event:
        count = character_pad.pickup_count;
        val = (uint8)character_pad.pickup_count;
        result = val - 1;
    } while (count != 0);
    return result;
}

uint32 AnimatedObject::ProcessCharacterSceneEvent()
{
    CharacterModelRecord *parent_rec;
    FUNCTION_MARKER(0x80050C94, "SLUS_005.78");
    SceneSection *next_section, *cur_section;
    Vec3 sample_pos, floor, input;
    const PathDefinition *parent;
    uint32 cur_flags, val, x, y, z;
    PathTransform *path_transform;
    const SceneTrigger *event;
    sint32 sample_frame, idx;
    if ((character_pad.flags & 2) == 0)
        return 0;
    event = character_pad.surface_interaction.sel != 0 ? (const SceneTrigger *)character_pad.surface_interaction.sel : NULL;
    if ((sint8)(event->node.state_flags >> 8) == 17)
    {
        ResetCharacterForScene(0);
        event = character_pad.surface_interaction.sel != 0 ? (const SceneTrigger *)character_pad.surface_interaction.sel : NULL;
        val = GameFlowState::MapLevelProgression(event->parameters.level, 0);
        cur_flags = game_flow.transition_flags;
        game_flow.requested_level = val;
        game_flow.transition_flags = cur_flags | 8;
        goto finish;
    }
    character_pad.section_height_offset = 0;
    if ((flags & 0x2400) == 0x400)
    {
        parent = path_definition;
        if (parent != 0 && parent->transform != 0)
        {
            input = pos;
            PushMatrix();
            path_transform = path_definition->transform;
            SetRotMatrix(&path_transform->matrix);
            {
                VECTOR sdk_input = {input.x, input.y, input.z, 0};
                VECTOR sdk_output;
                ApplyRotMatrixLV(&sdk_input, &sdk_output);
                sample_pos.x = sdk_output.vx;
                sample_pos.y = sdk_output.vy;
                sample_pos.z = sdk_output.vz;
            }
            PopMatrix();
            path_transform = path_definition->transform;
            x = sample_pos.x;
            sample_pos.x = x + path_transform->matrix.t[0];
            path_transform = path_definition->transform;
            y = sample_pos.y;
            sample_pos.y = y + path_transform->matrix.t[1];
            path_transform = path_definition->transform;
            z = sample_pos.z;
            sample_pos.z = z + path_transform->matrix.t[2];
        }
        else
        {
            sample_pos = pos;
        }
        sample_frame = path_frame;
        cur_flags = flags;
        parent = path_definition;
        if (parent->ProjectFrame(sample_frame, sample_pos, floor, cur_flags & 0x10, 4096) >= 0)
            character_pad.section_height_offset = sample_pos.y - (uint32)floor.y;
    }
    event = character_pad.surface_interaction.sel != 0 ? (const SceneTrigger *)character_pad.surface_interaction.sel : NULL;
    if (event->parameters.exit.swap != 0)
    {
        while (SceneLoader::PollSceneResourceLoad() != 0)
        {
        }
        next_section = SceneStream::SwapSceneSections();
        event = character_pad.surface_interaction.sel != 0 ? (const SceneTrigger *)character_pad.surface_interaction.sel : NULL;
        idx = next_section->FindObjectId(&event->parameters.exit.target);
        event = character_pad.surface_interaction.sel != 0 ? (const SceneTrigger *)character_pad.surface_interaction.sel : NULL;
        cur_section = scene_stream.active_section;
        val = event->parameters.exit.flags;
        SceneStream::SetSceneTransition(next_section, cur_section, idx, val);
    }
    else
    {
        parent_rec = character_pad.model;
        cur_flags = flags;
        AnimationHeader *sel_animation = parent_rec->animations[2];
        frame = -1;
        flags = cur_flags | 0x10000;
        animation = sel_animation;
        game_flow.transition_flags = game_flow.transition_flags | 0x1000;
    }
finish:
    character_pad.ClearTransientFlags();
    character_pad.flags = character_pad.flags & 0xFFFFFFDD;
    return 1;
}

void AnimatedObject::ProcessCharacterTriggers()
{
    SceneSection *section;
    CharacterController *character = &character_pad;
    FUNCTION_MARKER(0x80050F60, "SLUS_005.78");
    const ActorModelDefinition *bound_model;
    SceneNode *event;
    uint32 cur_flags;
    uint32 object_flags;
    uint32 val;
    uint32 prev;
    sint32 idx;
    uint32 count;

    uint32 result;
    sint32 kind, health, active;
    cur_flags = character_action_flags;
    if ((cur_flags & 0x100) != 0)
    {
        const PathDefinition *hit_definition = character_action_target->path_definition;
        const void *hit_state = ((const ActorPlacement *)hit_definition)->state;
        val = character_pad.flags;
        bound_model = ((const ActorBinding *)hit_state)->model;
        character_pad.flags = val | 0x400;
        val = bound_model->action_damage;
        character_action_flags = cur_flags & 0xFFFFFEFF;
        character_pad.vitals.damage_amount = val;
        goto take_damage;
    }
    if ((character_pad.flags & 0x80000) != 0)
    {
        object_flags = flags;
        if ((object_flags & 0x2000000) == 0 && (object_flags & 0x4400) != 0x400)
        {
            character_pad.vitals.damage_amount = 1;
            goto take_damage;
        }
    }
    cur_flags = character_pad.flags;
    if ((cur_flags & 0x400) == 0)
        goto timed_flags;
    event = character_pad.support.contact;
    if (event->kind != 0)
    {
        val = character_pad.contacts.damage_fraction;
        if ((sint32)path_motion_scale < (sint32)val)
            goto timed_flags;
        if ((sint8)(event->state_flags >> 8) == 18)
        {
            cur_flags = character_pad.flags;
            character_pad.vitals.health = 0;
            character_pad.flags = cur_flags | 0x80;
        }
        else
        {
            val = ((const SceneTrigger *)event)->parameters.damage.flags;
            event = character_pad.support.contact;
            character_pad.vitals.damage_amount = val;
            val = (((const SceneTrigger *)event)->parameters.damage.flags >> 16);
            val += game_tick;
            ((SceneTrigger *)event)->parameters.damage.deadline = val;
        }
    }
    else if ((flags & 0x2000400) != 0x2000000)
        character_pad.flags = cur_flags | 0x20000;
    if ((sint8)character_pad.vitals.health <= 0 || (flags & 0x2000000) != 0)
        goto timed_flags;
take_damage:
    if ((game_flow.transition_flags & 0x10000) != 0 && (uint32)level_resource.level_id - 96 >= 2 && (flags & 0x22000000) == 0)
    {
        val = character_pad.vitals.damage_amount;
        health = (sint8)character_pad.vitals.health;
        if ((sint32)val >= health)
        {
            val = health;
            if (health >= 2)
                --val;
        }
        character_pad.vitals.health = character_pad.vitals.health - val;
    }
    AnimatedObject::QueueCharacterActionSound(2, 3);
    val = character_pad.movement_flags;
    cur_flags = game_flow.mode_flags;
    character_pad.movement_flags = val | 0x4000000;
    if ((cur_flags & 0x10) == 0)
    {
        if ((character_pad.secondary_flags & 0x20) != 0)
        {
            val = player_interaction.flags;
            cur_flags = character_pad.secondary_flags;
            player_interaction.flags = val & 0xFFFFFFDB;
            character_pad.secondary_flags = cur_flags & 0xFFFFFFDF;
        }
        if (((game_flow.mode_flags & 0x200) == 0 || ((uint16)(character_pad.motion.form_flags >> 16) & 15) != 0) && (character_pad.secondary_flags & 0x100) != 0)
        {
            character_pad.RemoveAction(6);
            character->EndInteraction();
        }
        val = character_pad.weapon_state;
        if ((val & 0xFFFF) != 0)
        {
            prev = val >> 16;
            character_pad.RemoveAction(prev == 0 ? 3 : prev - 7);
        }
        character_pad.weapon_state = 0;
    }
    if ((flags & 0x2000000) == 0)
    {
        character_pad.vitals.damage_ticks = 60;
        flags = flags | 0x2080000;
    }
    if ((character_pad.flags & 0x80000) == 0)
    {
        AnimatedObject::SpawnActionEffect(this, &character_pad, 0);
        if ((path_flags & 0x101) == 0 && (character_pad.secondary_flags & 0x80) == 0)
            character_pad.motion.state_code = 4;
    }
    character_pad.flags = character_pad.flags & 0xEFFFFFFF;
timed_flags:
    if ((character_pad.flags & 0x800000) != 0)
    {
        val = character_pad.contacts.jump_scale_fraction;
        if ((sint32)path_motion_scale < (sint32)val)
        {
            val = character_pad.secondary_flags;
            cur_flags = character_pad.flags;
            character_pad.secondary_flags = val & 0xFFFFFF7F;
            character_pad.flags = cur_flags & 0xFF7FFFFF;
        }
    }
    if ((character_pad.flags & 8) != 0)
    {
        val = character_pad.motion.environment_acceleration;
        if ((sint32)path_motion_scale < (sint32)val)
        {
            cur_flags = character_pad.flags;
            character_pad.motion.environment_acceleration = 0;
            character_pad.flags = cur_flags & 0xFFFFFFF7;
        }
    }
    cur_flags = character_pad.flags;
    if ((cur_flags & 0x3000000) == 0x3000000)
    {
        val = character_pad.contacts.hide_fraction;
        prev = character_pad.contacts.show_fraction;
        if ((sint32)prev >= (sint32)val && (sint32)path_motion_scale >= (sint32)val)
            character_pad.flags = cur_flags & 0xFDFFFFFF;
        else
        {
            prev = character_pad.contacts.show_fraction;
            val = character_pad.contacts.hide_fraction;
            if ((sint32)val >= (sint32)prev && (sint32)path_motion_scale >= (sint32)prev)
                character_pad.flags = character_pad.flags & 0xFEFFFFFF;
        }
    }
    if ((character_pad.flags & 0x1000000) != 0)
    {
        val = character_pad.contacts.hide_fraction;
        if ((sint32)path_motion_scale >= (sint32)val)
        {
            node.flags = node.flags | 1;
            goto timed_scene;
        }
    }
    if ((character_pad.flags & 0x2000000) != 0)
    {
        val = character_pad.contacts.show_fraction;
        if ((sint32)path_motion_scale >= (sint32)val)
            node.flags = node.flags & 0xFE;
    }
timed_scene:
    if ((character_pad.flags & 0x4000) != 0)
    {
        val = character_pad.contacts.section_fraction;
        if ((sint32)path_motion_scale >= (sint32)val && (SceneLoader::RequestSceneSection(character_pad.contacts.section_trigger->parameters.section_id) << 24) != 0)
        {
            section = scene_stream.active_section;
            section->state_flags = section->state_flags & 0xFFEF;
        }
    }
    active = character_pad.trigger_count;
    count = (uint8)character_pad.trigger_count;
    while (active != 0)
    {
        val = count - 1;
        character_pad.trigger_count = val;
        idx = (sint8)val;
        val = character_pad.trigger_fractions[idx];
        if ((sint32)path_motion_scale >= (sint32)val)
        {
            event = character_pad.trigger_objects[idx];
            if (event != 0)
            {
                kind = (sint8)(event->state_flags >> 8);
                if (kind >= 40)
                    event->state_flags = event->state_flags | 8;
                else
                {
                    switch (kind)
                    {
                        case 19:

                            val = ((const SceneTrigger *)event)->parameters.far_distance;
                            camera_proj.far_distance_copy = val;
                            camera_proj.far_distance = val;
                            break;
                        case 20:
                            character_pad.motion.trail_effect = ((const SceneTrigger *)event)->parameters.trail_effect;
                            break;
                        case 23:
                            node.state_flags = ((const SceneTrigger *)event)->parameters.state_flags;
                            break;
                        case 21:
                            val = ((const SceneTrigger *)event)->parameters.form + 1;
                            if ((sint32)val < 2)
                                val = (character_pad.motion.form_flags >> 20) & 15;
                            result = CharacterContactPool::ResolveCharacterFormVariant(val);
                            val = character_pad.motion.form_flags;
                            character_pad.motion.form_flags = (val & 0xF0FFFFFF) | (result << 24);
                            break;
                    }
                }
            }
        }
        active = character_pad.trigger_count;
        count = (uint8)character_pad.trigger_count;
    }
    CutscenePlayback::StartPendingCutscene(this);
}

void AnimatedObject::UpdateCharacterActionEffects()
{
    CharacterModelRecord *parent_rec;
    AnimatedObject *created_object;
    FUNCTION_MARKER(0x800515F0, "SLUS_005.78");
    VECTOR input, relative;

    MATRIX transpose = {0};

    const PathDefinition *parent;
    uint32 cur_flags, original_flags, val, x, y, z, result, kind, eject, angle, pitch;
    PathTransform *path_transform;
    const SoundBankStorage *sound_table;
    const SoundSample *sound;
    sint32 timer, voice, volume, wave;
    if ((character_pad.secondary_flags & 0x100) != 0)
    {
        if ((flags & 0x1000000) != 0)
        {
            parent = path_definition;
            if (parent != 0 && parent->transform != 0)
            {
                x = pos.x;
                y = pos.y;
                z = pos.z;
                input.vx = x;
                input.vy = y;
                input.vz = z;
                PushMatrix();
                path_transform = path_definition->transform;
                SetRotMatrix(&path_transform->matrix);
                ApplyRotMatrixLV(&input, &player_interaction.pos);
                PopMatrix();
                path_transform = path_definition->transform;
                x = player_interaction.pos.vx;
                player_interaction.pos.vx = x + path_transform->matrix.t[0];
                path_transform = path_definition->transform;
                y = player_interaction.pos.vy;
                player_interaction.pos.vy = y + path_transform->matrix.t[1];
                path_transform = path_definition->transform;
                z = player_interaction.pos.vz;
                player_interaction.pos.vz = z + path_transform->matrix.t[2];
            }
            else
            {
                x = pos.x;
                y = pos.y;
                z = pos.z;
                player_interaction.pos.vx = x;
                player_interaction.pos.vy = y;
                player_interaction.pos.vz = z;
            }
        }
        UpdateCharacterInteraction();
    }
    cur_flags = character_pad.secondary_flags;
    if ((cur_flags & 0x40) != 0)
    {
        --character_pad.timers.model_swap_ticks;
        if (character_pad.timers.model_swap_ticks == 435)
        {
            sound_table = sound_resources.common_table;
            sound = sound_table != 0 && sound_table->header.count >= 7 ? sound_table->samples + 6 : 0;
            result = sound_effects.IsPlaying(sound, character_pad.sound_voice);
            if ((result << 24) == 0)
            {
                sound_table = sound_resources.common_table;
                sound = sound_table != 0 && sound_table->header.count >= 11 ? sound_table->samples + 10 : 0;
                result = sound_effects.IsPlaying(sound, character_pad.sound_voice);
            }
            if ((result << 24) != 0)
            {
                SoundEffects::StopSoundVoice(character_pad.sound_voice);
                character_pad.sound_voice = -1;
            }
            AnimatedObject::QueueCharacterActionSound(9, 3);
            sound_table = sound_resources.common_table;
            sound = sound_table != 0 && sound_table->header.count >= 11 ? sound_table->samples + 10 : 0;
            voice = sound_effects.SelectVoice(0xFFFFFFFF, sound, 1);
            character_pad.sound_voice = voice;
            voice = (sint8)voice;
            if (voice >= 0)
            {
                const SoundSample *sample = sound;
                val = sample->metadata;
                pitch = sample->pitch;
                volume = sound_settings.effects_volume;
                SoundEffects::PlaySoundEffect(sound, voice, 0, val, pitch, volume, 0);
            }
        }
        if ((sint16)character_pad.timers.model_swap_ticks < 150)
        {
            val = character_pad.appearance.blink_ticks;
            character_pad.appearance.blink_ticks = val + 1;
            if ((sint32)val >= 15)
            {
                cur_flags = flags;
                character_pad.appearance.blink_ticks = 0;
                flags = cur_flags ^ 0x80000;
            }
        }
        if (character_pad.timers.model_swap_ticks == 0)
        {
            parent_rec = character_pad.model;
            AnimationHeader *animation22 = parent_rec->animations[22];
            AnimationHeader *animation2 = parent_rec->animations[2];
            parent_rec->animations[2] = animation22;
            parent_rec = character_pad.model;
            parent_rec->animations[22] = animation2;
            character_pad.RemoveAction(7);
            flags = flags & 0xDFFFFFFF;
            character_pad.secondary_flags = character_pad.secondary_flags & 0xFFFFFFBF;
            flags = flags & 0xFFF7FFFF;
        }
    }
    else if ((cur_flags & 0x200) != 0)
    {
        val = (uint32)character_pad.timers.color_effect_ticks - 1;
        character_pad.timers.color_effect_ticks = val;
        timer = (sint16)character_pad.timers.color_effect_ticks;
        character_pad.appearance.tint = (((16 * (val & 15) + 64) & 0xFF) << 16) | 0x2020;
        if (timer == 0)
        {
            flags = flags & 0xDFF7FFFF;
            character_pad.secondary_flags = character_pad.secondary_flags & 0xFFFFFDFF;
        }
    }
    cur_flags = flags;
    if ((cur_flags & 0x1000000) == 0)
    {
        timer = (sint16)character_pad.timers.action_cooldown_ticks;

        if (timer != 0)
        {
            result = (uint32)timer - 1;
            character_pad.timers.action_cooldown_ticks = result;
        }
        goto done;
    }
    timer = character_pad.timers.action_launch_ticks;
    result = (uint32)timer - 1;
    if (timer > 0)
    {
        character_pad.timers.action_launch_ticks = result;
        goto done;
    }
    flags = cur_flags & 0xFEFFFFFF;
    cur_flags = character_pad.movement_flags;
    kind = (uint16)(character_pad.motion.form_flags >> 16);
    character_pad.timers.action_cooldown_ticks = 15;
    character_pad.movement_flags = cur_flags | 0x1000000;
    switch (kind & 15)
    {
        case 1:
            state = 19;
            cur_flags = character_pad.secondary_flags;
            player_interaction.flags = 19;
            if ((cur_flags & 0x20) != 0)
                player_interaction.flags = 51;
            break;
        case 4:
            state = 21;
            break;
        case 5:
        case 10:
        case 11:
            state = 25;
            break;
        default:
            state = (uint16)(character_pad.weapon_state >> 16);
            if (((uint16)(character_pad.motion.form_flags >> 16) & 15) == 0)
                path_routing.flags = path_routing.flags | 0x10;
            break;
    }
    val = state;
    original_flags = flags;
    if ((val == 19 || path_routing.flags != 0) && (path_definition->flags & 0x10000) != 0)
    {
        timer = yaw;
        wave = (sint16)character_pad.path_transition.motion_heading;
        flags = timer == wave ? original_flags & 0xFFFFFFEF : original_flags | 0x10;
    }
    flags = flags | 0x100;
    created_object = object_operations.behaviors[6].factory_handler(this);
    flags = original_flags;
    kind = (uint16)(character_pad.motion.form_flags >> 16) & 15;
    eject = kind - 5 < 2 || kind == 7 || kind - 10 < 2;
    if (eject == 0)
    {
        if (kind == 4)
        {
            result = (uint32)created_object->pos.y + ((original_flags & 0x8000000) != 0 ? 4096 : 20480);
            created_object->pos.y = result;
            goto done;
        }
        result = original_flags & 0x8000000;
        if (result == 0)
            goto done;
    }
    parent = created_object->path_definition;
    if (parent != 0 && parent->transform != 0)
    {
        x = created_object->pos.x;
        y = created_object->pos.y;
        z = created_object->pos.z;
        input.vx = x;
        input.vy = y;
        input.vz = z;
        PushMatrix();
        path_transform = created_object->path_definition->transform;
        SetRotMatrix(&path_transform->matrix);
        {
            VECTOR rotation_output;
            ApplyRotMatrixLV(&input, &rotation_output);
            created_object->pos.x = rotation_output.vx;
            created_object->pos.y = rotation_output.vy;
            created_object->pos.z = rotation_output.vz;
        }
        PopMatrix();
        parent = created_object->path_definition;
        path_transform = parent->transform;
        x = created_object->pos.x;
        y = path_transform->matrix.t[0];
        parent = created_object->path_definition;
        created_object->pos.x = x + y;
        path_transform = parent->transform;
        x = created_object->pos.y;
        y = path_transform->matrix.t[1];
        parent = created_object->path_definition;
        created_object->pos.y = x + y;
        path_transform = parent->transform;
        z = created_object->pos.z;
        created_object->pos.z = z + path_transform->matrix.t[2];
    }
    if (eject != 0)
    {
        angle = ((uint16)target_yaw + 2048) & 0xFFF;
        wave = SinQ14(angle) >> 2;
        x = created_object->pos.x;
        created_object->pos.x = x + (uint32)mul12(12288, wave);
        wave = SinQ14((1024 - angle)) >> 2;
        z = created_object->pos.z;
        created_object->pos.z = z + (uint32)mul12(12288, wave);
        created_object->pos.x = created_object->pos.x + (uint32)velocity.x;
        created_object->pos.y = created_object->pos.y + (uint32)velocity.y;
        created_object->pos.z = created_object->pos.z + (uint32)velocity.z;
        result = created_object->flags | 0x2000;
        created_object->flags = result;
        created_object->movement_mode = 0;
        goto done;
    }
    created_object->pos.y = (uint32)created_object->pos.y - 8192;
    parent = created_object->path_definition;
    if (parent == 0)
    {
        goto done;
    }
    path_transform = parent->transform;
    if (path_transform == 0)
    {
        goto done;
    }
    relative.vx = (uint32)created_object->pos.x - path_transform->matrix.t[0];
    path_transform = created_object->path_definition->transform;
    relative.vy = (uint32)created_object->pos.y - path_transform->matrix.t[1];
    path_transform = created_object->path_definition->transform;
    relative.vz = (uint32)created_object->pos.z - path_transform->matrix.t[2];
    transpose.m[0][0] = path_transform->matrix.m[0][0];
    transpose.m[0][1] = path_transform->matrix.m[1][0];
    transpose.m[0][2] = path_transform->matrix.m[2][0];
    transpose.m[1][0] = path_transform->matrix.m[0][1];
    transpose.m[1][1] = path_transform->matrix.m[1][1];
    transpose.m[1][2] = path_transform->matrix.m[2][1];
    transpose.m[2][0] = path_transform->matrix.m[0][2];
    transpose.m[2][1] = path_transform->matrix.m[1][2];
    transpose.m[2][2] = path_transform->matrix.m[2][2];
    PushMatrix();
    SetRotMatrix(&transpose);
    {
        VECTOR rotation_output;
        ApplyRotMatrixLV(&relative, &rotation_output);
        created_object->pos.x = rotation_output.vx;
        created_object->pos.y = rotation_output.vy;
        created_object->pos.z = rotation_output.vz;
    }
    PopMatrix();
done:
    return;
}

void AnimatedObject::UpdateCharacterContactState()
{
    FUNCTION_MARKER(0x800521BC, "SLUS_005.78");
    uint32 cur_flags, val, kind;
    sint32 animation, limit;
    if ((flags & 0x4000) != 0)
    {
        if (character_pad.motion.vertical_speed < 0)
            AnimatedObject::QueueCharacterActionSound(1, 3);
        cur_flags = character_pad.movement_flags;
        character_pad.motion.vertical_speed = 0;
        character_pad.timers.contact_impulse_ticks = 0;
        character_pad.movement_flags = cur_flags | 0x800000;
        flags = flags & 0xEFFFBBFF;
        cur_flags = character_pad.flags;
        if ((cur_flags & 0x18000) != 0)
            character_pad.flags = (cur_flags | 0x40) & 0xFFFE7FFF;
        cur_flags = character_pad.flags;
        val = character_pad.motion.horizontal_speed;
        limit = character_pad.movement.speed_limits[1];
        character_pad.flags = cur_flags & 0xC7F9FFFF;
        if (limit < (sint32)val)
            character_pad.motion.horizontal_speed = limit;
    }
    if ((path_flags & 0x100) != 0)
        BeginCharacterPathTransfer();
    cur_flags = flags;
    if ((cur_flags & 0x800) != 0)
    {
        DetachCharacterFromPath(0);
        return;
    }
    if ((path_flags & 0x800) != 0)
    {
        flags = cur_flags | 0x2000;
        DetachCharacterFromPath(0);
        return;
    }
    if ((cur_flags & 0x400) == 0)
    {
        if ((path_definition->flags & 0x11) != 0)
        {
            val = character_pad.movement.jump_ticks;
            cur_flags = character_pad.flags;
            character_pad.motion.jump_phase = -1;
            character_pad.motion.vertical_speed = 0;
            character_pad.timers.state_ticks = 0;
            character_pad.timers.jump_duration_ticks = val;
            character_pad.flags = cur_flags | 0x8000000;
            flags = flags | 0x400;
        }
        val = flags & 0x400;
        if (val == 0)
            return;
    }
    animation = character_pad.motion.state_code;
    if (animation == 6)
        return;
    if (animation == 3)
        return;
    if ((uint8)(animation - 33) < 2)
        return;
    if (animation == 15 || animation == 23)
        return;
    kind = (uint16)(character_pad.motion.form_flags >> 16) & 15;
    if ((kind == 0 || kind == 8) && animation == 9)
        return;
    cur_flags = flags;
    if ((cur_flags & 0x8000000) != 0 && ((uint16)(character_pad.motion.form_flags >> 16) & 15) == 4)
        return;
    if (((uint16)(character_pad.motion.form_flags >> 16) & 15) == 5 && character_pad.motion.state_code == 7)
        return;
    if ((character_pad.secondary_flags & 0x400) != 0)
        return;
    flags = cur_flags & 0xF7FFFFFF;
    animation = character_pad.motion.state_code;
    if (animation == 4)
        return;
    kind = (uint16)(character_pad.motion.form_flags >> 16) & 15;
    if ((kind == 5 || kind - 10 < 2) && (animation == 5 || animation == 9))
        return;
    if (((uint16)(character_pad.motion.form_flags >> 16) & 15) == 1)
    {
        val = flags & 0x4000000;
        if (val != 0)
            return;
    }
    if ((character_pad.secondary_flags & 0x80) == 0)
        character_pad.motion.state_code = 6;
}

void AnimatedObject::UpdateCharacterFallTimeout()
{
    FUNCTION_MARKER(0x800524AC, "SLUS_005.78");
    const PathDefinition *parent;
    uint32 result, cur_flags, mask, x, y, z;
    sint16 ticks;
    PathTransform *path_transform;

    Vec3 sample_pos, floor, input;

    sint32 elapsed;
    result = (sint8)character_pad.vitals.health;
    if (result == 0)
        return;
    if (character_pad.movement.fall_speed_limit < character_pad.motion.vertical_speed)
        return;
    result = (uint16)(character_pad.motion.form_flags >> 16) & 15;
    if (result == 5)
        return;
    if ((flags & 0x2000) != 0)
        goto inc;
    parent = path_definition;
    cur_flags = parent->flags;
    mask = cur_flags & 0x11;
    result = cur_flags & 0x10;
    if (mask != 1)
    {
        if (result == 0)
            goto done;
        const ScenePath *camera_path = camera_motion.path;
        const PathDefinition *camera_definition = camera_path->definition;
        if ((camera_definition->flags & 0x100010) != 0)
        {
            const ScenePath *fallback = camera_path->fallback;
            result = fallback != 0;
        }
        else
            result = camera_path->slope_scale;
        if (result != 0)
            goto done;
    }
    if (mask == 0)
        goto inc;
    if (parent != 0 && parent->transform != 0)
    {
        input = pos;
        PushMatrix();
        path_transform = path_definition->transform;
        SetRotMatrix(&path_transform->matrix);
        {
            VECTOR sdk_input = {input.x, input.y, input.z, 0};
            VECTOR sdk_output;
            ApplyRotMatrixLV(&sdk_input, &sdk_output);
            sample_pos.x = sdk_output.vx;
            sample_pos.y = sdk_output.vy;
            sample_pos.z = sdk_output.vz;
        }
        PopMatrix();
        path_transform = path_definition->transform;
        x = sample_pos.x;
        sample_pos.x = x + path_transform->matrix.t[0];
        path_transform = path_definition->transform;
        y = sample_pos.y;
        sample_pos.y = y + path_transform->matrix.t[1];
        path_transform = path_definition->transform;
        z = sample_pos.z;
        sample_pos.z = z + path_transform->matrix.t[2];
    }
    else
    {
        sample_pos = pos;
    }
    elapsed = path_frame;
    cur_flags = flags;
    parent = path_definition;
    if (parent->ProjectFrame(elapsed, sample_pos, floor, cur_flags & 0x10, 4096) >= 0 && sample_pos.y >= floor.y)
        character_pad.timers.state_ticks = (uint32)character_pad.timers.state_ticks - 1;
inc:
    ticks = character_pad.timers.state_ticks + 1;
    character_pad.timers.state_ticks = ticks;
    result = ticks < 16;
    if (result == 0)
        character_pad.vitals.health = 0;
done:
    return;
}

uint32 AnimatedObject::UpdateCharacterDeathState()
{
    FUNCTION_MARKER(0x800526F8, "SLUS_005.78");
    const PathDefinition *parent;
    uint32 cur_flags, other_flags, val, result, x, y, z;
    PathTransform *path_transform;
    const SoundBankStorage *sound_table;
    const SoundSample *sound;

    VECTOR sample_pos, input, death_input;

    if ((game_flow.transition_flags & 0x100) == 0)
        UpdateCharacterCameraPath();
    if ((sint8)character_pad.vitals.health > 0)
        return 1;
    cur_flags = character_pad.flags;
    other_flags = character_pad.secondary_flags;
    character_pad.trigger_count = 0;
    character_pad.vitals.health = 0;
    character_pad.flags = cur_flags & 0xFEFFBFDD;
    character_pad.secondary_flags = other_flags & 0xFFFFFF7F;
    val = node.flags;
    cur_flags = flags;
    node.flags = val & 0xFE;
    flags = cur_flags & 0xFDF7FFFF;
    character_pad.timers.state_ticks = 30;
    movement_mode = 0;
    contact_kind = 0;
    behavior_suspended = 1;
    SequencePlayback::StopCurSequence();

    sound_table = sound_resources.common_table;
    sound = sound_table != 0 && sound_table->header.count >= 7 ? sound_table->samples + 6 : 0;
    result = sound_effects.IsPlaying(sound, character_pad.sound_voice);
    if ((result << 24) == 0)
    {
        sound_table = sound_resources.common_table;
        sound = sound_table != 0 && sound_table->header.count >= 11 ? sound_table->samples + 10 : 0;
        result = sound_effects.IsPlaying(sound, character_pad.sound_voice);
    }
    if ((result << 24) != 0)
    {
        SoundEffects::StopSoundVoice(character_pad.sound_voice);
        character_pad.sound_voice = -1;
    }
    SequencePlayback::PlayLoopSequence();
    val = game_flow.transition_flags;
    cur_flags = flags;
    scr_render.fade_level = 0;
    game_flow.transition_flags = val | 0x100;
    if ((cur_flags & 0x2000) != 0)
        goto local_pos;
    parent = path_definition;
    if (parent == 0 || parent->transform == 0)
        goto local_pos;
    x = pos.x;
    y = pos.y;
    z = pos.z;
    input.vx = x;
    input.vy = y;
    input.vz = z;
    PushMatrix();
    path_transform = path_definition->transform;
    SetRotMatrix(&path_transform->matrix);
    ApplyRotMatrixLV(&input, &sample_pos);
    PopMatrix();
    path_transform = path_definition->transform;
    x = sample_pos.vx;
    sample_pos.vx = x + path_transform->matrix.t[0];
    path_transform = path_definition->transform;
    y = sample_pos.vy;
    sample_pos.vy = y + path_transform->matrix.t[1];
    path_transform = path_definition->transform;
    z = sample_pos.vz;
    sample_pos.vz = z + path_transform->matrix.t[2];
    goto pos_ready;
local_pos:
    x = (uint32)pos.x;
    y = pos.y;
    z = pos.z;
    sample_pos.vx = x;
    sample_pos.vy = y;
    sample_pos.vz = z;
pos_ready:
    spheres[0].pos.x = world_pos.x - (uint32)sample_pos.vx;
    spheres[0].pos.y = world_pos.y - (uint32)sample_pos.vy;
    spheres[0].pos.z = world_pos.z - (uint32)sample_pos.vz;
    if ((character_pad.flags & 0x80) != 0)
    {
        character_pad.timers.state_ticks = 70;
        character_pad.path_velocity.vz = 40960;
        if ((character_pad.secondary_flags & 0x80) == 0)
            character_pad.motion.state_code = 6;
        if ((flags & 0x2000) == 0)
        {
            parent = path_definition;
            if (parent != 0 && parent->transform != 0)
            {
                x = pos.x;
                y = pos.y;
                z = pos.z;
                death_input.vx = x;
                death_input.vy = y;
                death_input.vz = z;
                PushMatrix();
                path_transform = path_definition->transform;
                SetRotMatrix(&path_transform->matrix);
                {
                    VECTOR rotation_output;
                    ApplyRotMatrixLV(&death_input, &rotation_output);
                    pos.x = rotation_output.vx;
                    pos.y = rotation_output.vy;
                    pos.z = rotation_output.vz;
                }
                PopMatrix();
                parent = path_definition;
                path_transform = parent->transform;
                x = pos.x;
                y = path_transform->matrix.t[0];
                parent = path_definition;
                pos.x = x + y;
                path_transform = parent->transform;
                x = pos.y;
                y = path_transform->matrix.t[1];
                parent = path_definition;
                pos.y = x + y;
                path_transform = parent->transform;
                z = pos.z;
                pos.z = z + path_transform->matrix.t[2];
            }
            flags = flags | 0x2000;
        }
    }
    else if ((character_pad.secondary_flags & 0x80) == 0)
        character_pad.motion.state_code = 1;
    camera_motion.smooth_ticks[2] = 0;
    camera_motion.smooth_ticks[1] = 0;
    camera_motion.smooth_ticks[0] = 0;

    camera_motion.desired_pos = camera_motion.pos;
    return 0;
}

void AnimatedObject::UpdateCharacterAnimationState()
{
    CharacterModelRecord *animation_data_rec;
    AnimationHeader *sel_animation;
    FUNCTION_MARKER(0x80052BA4, "SLUS_005.78");
    uint32 cur_flags, val, result, kind, pitch;
    const SoundBankStorage *sound_table;
    const SoundSample *sound;
    sint32 prev, clip, voice, volume;
    if (character_pad.vitals.health != 0 && (character_pad.flags & 0x180000) != 0 && (character_pad.secondary_flags & 0x80) == 0)
        character_pad.motion.state_code = 8;
    prev = character_pad.motion.prev_state_code;
    clip = character_pad.motion.state_code;
    if (prev == clip)
    {
        if ((uint32)prev - 1 >= 34)
            character_pad.timers.idle_ticks = (uint32)character_pad.timers.idle_ticks - 1;
        if (character_pad.motion.state_code != 7)
            return;
        result = character_pad.motion.horizontal_speed;
        if (result != 0)
            return;
        result = character_pad.secondary_flags & 0x40;
        if (result != 0)
            return;

        sound_table = sound_resources.common_table;
        sound = sound_table != 0 && sound_table->header.count >= 7 ? sound_table->samples + 6 : 0;
        result = sound_effects.IsPlaying(sound, character_pad.sound_voice) << 24;
        if (result == 0)
        {
            sound_table = sound_resources.common_table;
            sound = sound_table != 0 && sound_table->header.count >= 11 ? sound_table->samples + 10 : 0;
            result = sound_effects.IsPlaying(sound, character_pad.sound_voice) << 24;
            if (result == 0)
                return;
        }
        SoundEffects::StopSoundVoice(character_pad.sound_voice);
        character_pad.sound_voice = -1;
        return;
    }
    if (prev == 7 && (flags & 0x8000000) != 0 && ((uint16)(character_pad.motion.form_flags >> 16) & 15) == 4)
    {
        character_pad.motion.state_code = 7;
        goto finish;
    }
    clip = character_pad.motion.state_code;
    if (clip == 7)
    {
        kind = (uint16)(character_pad.motion.form_flags >> 16) & 15;
        if (kind != 5 && kind - 10 >= 2)
        {
            animation_data_rec = character_pad.model;
            cur_flags = flags;
            sel_animation = animation_data_rec->animations[7];
            frame = 4;
            flags = (cur_flags & 0xFFFEFFFF) | 0x20000;
            animation = sel_animation;
            goto finish;
        }
        clip = character_pad.motion.state_code;
    }
    if (clip == 20)
    {
        animation_data_rec = character_pad.model;
        cur_flags = flags;
        sel_animation = animation_data_rec->animations[20];
        goto set_animation_data;
    }
    if (character_pad.path_transition.reentry_ticks != 0)
    {
        animation_data_rec = character_pad.model;
        cur_flags = flags;
        sel_animation = animation_data_rec->animations[3];
        goto set_animation_data;
    }
    if ((character_pad.secondary_flags & 0x40) == 0)
    {
        sound_table = sound_resources.common_table;
        sound = sound_table != 0 && sound_table->header.count >= 7 ? sound_table->samples + 6 : 0;
        result = sound_effects.IsPlaying(sound, character_pad.sound_voice) << 24;
        if (result == 0)
        {
            sound_table = sound_resources.common_table;
            sound = sound_table != 0 && sound_table->header.count >= 11 ? sound_table->samples + 10 : 0;
            result = sound_effects.IsPlaying(sound, character_pad.sound_voice) << 24;
        }
        if (result != 0)
        {
            SoundEffects::StopSoundVoice(character_pad.sound_voice);
            character_pad.sound_voice = -1;
        }
    }
    clip = character_pad.motion.state_code;
    if (clip == 0)
    {
        prev = character_pad.motion.prev_state_code;
        if ((uint32)prev - 1 < 34)
            character_pad.timers.idle_ticks = 900;
        clip = character_pad.motion.state_code;
    }
    animation_data_rec = character_pad.model;
    sel_animation = animation_data_rec->animations[clip];
    cur_flags = flags;
    frame = -1;
    flags = cur_flags | 0x10000;
    animation = sel_animation;
    clip = character_pad.motion.state_code;
    if (clip == 8)
    {
        if ((character_pad.secondary_flags & 0x40) == 0)
        {
            sound_table = sound_resources.common_table;
            sound = sound_table != 0 && sound_table->header.count >= 7 ? sound_table->samples + 6 : 0;
            voice = sound_effects.SelectVoice(0xFFFFFFFF, sound, 1);
            character_pad.sound_voice = voice;
            voice = (sint8)voice;
            if (voice >= 0)
            {
                const SoundSample *sample = sound;
                val = sample->metadata;
                pitch = sample->pitch;
                volume = sound_settings.effects_volume;
                SoundEffects::PlaySoundEffect(sound, voice, 0, val, pitch, volume, 0);
            }
        }
    }
    else if (clip == 9 && ((uint16)(character_pad.motion.form_flags >> 16) & 15) == 1)
        AnimatedObject::QueueCharacterActionSound(4, 3);
    else if ((path_flags & 0x40000) != 0 && character_pad.path_transition.phase == 8)
        flags = flags & 0xFFFEFFFF;
    goto finish;
set_animation_data:
    flags = cur_flags & 0xFFFEFFFF;
    animation = sel_animation;
finish:
    cur_flags = path_flags;

    if ((cur_flags & 0x200000) != 0)
    {
        path_flags = cur_flags & 0xFFDFFFFF;
        result = flags;
        cur_flags = path_flags;
        result &= 0xFFFEFFFF;
        flags = result;
        path_flags = cur_flags & 0xFFBFFFFF;
    }
}

intptr AnimatedObject::UpdateCharacterBehavior()
{
    FUNCTION_MARKER(0x800530A0, "SLUS_005.78");

    uint32 cur_flags, val, path_val0, path_val1, transition_control, clip, kind, result;
    AnimationHeader *src;
    sint32 sample_frame, next = 0;
    if (behavior_suspended == 1)
        return 1;
    if ((character_pad.flags & 0x4000000) != 0)
        return UpdateCharacterCameraTransition();
    ApplyCharacterContactMotion();
    if ((flags & 0x2000000) != 0)
    {
        val = character_pad.vitals.damage_ticks - 1;
        character_pad.vitals.damage_ticks = val;
        if ((sint16)val <= 0)
        {
            flags = flags & 0xEDF7FFFF;
            character_pad.vitals.damage_ticks = 0;
        }
        else
            flags = flags ^ 0x80000;
    }
    ProcessCharacterPickups();
    ProcessCharacterTriggers();
    UpdateCharacterContactState();
    UpdateCharacterFallTimeout();
    if ((UpdateCharacterDeathState() & 0xFF) == 0)
        goto finish;
    result = ProcessCharacterSceneEvent() << 24;
    if (result != 0)
        return result;
    if ((character_pad.flags & 0x20) != 0)
    {
        const SceneTrigger *camera_trigger = (const SceneTrigger *)character_pad.surface_interaction.sel;
        transition_control = camera_trigger->control_word;
        const ScenePath *camera_path = camera_trigger->parameters.path.target;
        path_val0 = camera_trigger->parameters.path.frame_word;
        path_val1 = camera_trigger->parameters.path.duration_ticks;
        BeginCharacterCameraTransition(camera_path, path_val0, path_val1, transition_control);
        character_pad.flags = character_pad.flags & 0xFFFFFFDD;
        goto finish;
    }
    UpdateCharacterActionEffects();
    if (character_pad.motion.state_code == 23)
        goto finish;
    src = animation;
    if (src != 0)
    {
        val = src->frame_count;
        sample_frame = frame;
        if (sample_frame < (sint32)val - 1)
            next = sample_frame + 1;
    }
    sample_frame = frame;
    if (next < sample_frame)
    {
        if ((path_flags & 0x40000) != 0)
            goto reverse;
        clip = character_pad.motion.state_code;
        if (clip - 1 >= 34 && ((uint16)(character_pad.motion.form_flags >> 16) & 0xF) < 2)
        {
            frame = next;
            val = 0;
            if (character_pad.motion.prev_state_code == 0 && (sint16)character_pad.timers.idle_ticks <= 0)
            {
                val = 37;
                if ((sint16)character_pad.timers.idle_ticks < -4499)
                    character_pad.timers.idle_ticks = -4500;
                else
                {
                    cur_flags = random_generator.Next() & 0x7F;
                    val = cur_flags < 50 ? 0 : (cur_flags < 101 ? 35 : 36);
                }
            }
            goto set_animation;
        }
        clip = (uint8)character_pad.motion.state_code;
        if (clip - 4 < 2)
        {
            if ((flags & 0x400) != 0)
                val = 6;
            else
                val = (sint8)character_pad.vitals.health <= 0 ? 1 : 0;
            goto set_animation;
        }
        if ((sint8)clip == 3)
        {
            val = 6;
            goto set_animation;
        }
        kind = (uint16)(character_pad.motion.form_flags >> 16) & 0xF;
        if ((kind == 0 || kind == 8) && (sint8)clip == 9)
        {
            val = 6;
            goto set_animation;
        }
        clip = character_pad.motion.state_code;
        if (clip == 7)
        {
            if (((uint16)(character_pad.motion.form_flags >> 16) & 0xF) != 5)
                goto last_frame;
            clip = character_pad.motion.state_code;
        }
        if (clip == 17 || clip == 20)
        {
        last_frame:
            src = animation;
            frame = (uint32)(uint16)src->frame_count - 1;
            goto finish;
        }
    }
    else
    {
        if ((path_flags & 0x40000) != 0)
            goto reverse;
        if (character_pad.motion.state_code == 20 && character_pad.path_transition.phase == 1)
        {
            frame = 0;
            goto finish;
        }
    }
    frame = next;
    goto finish;
reverse:
    sample_frame = frame;
    if (sample_frame > 0)
        frame = (uint32)sample_frame - 1;
    goto finish;
set_animation:
    if ((character_pad.secondary_flags & 0x80) == 0)
        character_pad.motion.state_code = val;
finish:
    UpdateCharacterAnimationState();
    UpdateCharacterForm();
    return ClampAnimationFrame();
}

intptr AnimatedObject::UpdateAlternateCharacterFollow()
{
    CharacterModelRecord *model_rec;
    FUNCTION_MARKER(0x80053704, "SLUS_005.78");
    AnimatedObject *parent = follow_target;
    Vec3 vector;
    const ScenePath *path;
    const PathDefinition *captured_definition;
    uint32 cur_flags, x, y, z, sq, result;
    AnimationHeader *definition, *clip;
    sint32 distance, speed, cap, target_frame, sample_frame;
    cur_flags = flags;
    if ((cur_flags & 0x10) != (parent->flags & 0x10))
    {
        path = scene_path;
        flags = cur_flags ^ 0x10;
        if ((path->node.state_flags & 8) != 0)
        {
            path = path->adjacent[0];
        }
        captured_definition = path_definition;
        scene_path = path;
        sample_frame = (uint16)path_frame;
        path_frame = (uint16)captured_definition->pos_track.frame_count - (uint32)sample_frame;
    }
    if (movement_mode == 1)
    {
        behavior = 1;
        character_pad.object = this;
        parent->behavior = 33;
        parent->follow_target = this;
        {
            uint32 model_idx = ((character_pad.motion.form_flags >> 18) & 0x3C) / 4;
            CharacterModelRecord *model_binding;
            model_binding = character_pad.model_slots[model_idx];
            model_rec = model_binding;
            character_pad.model = model_binding;
        }
        velocity.z = 0;
        velocity.y = 0;
        velocity.x = 0;
        parent->velocity.z = 0;
        parent->velocity.y = 0;
        parent->velocity.x = 0;
        character_pad.motion.state_code = 0;
        goto update_animation;
    }
    {
        uint32 model_idx = 1 - ((character_pad.motion.form_flags >> 16) & 0xF);
        CharacterModelRecord *model_binding;
        model_binding = character_pad.model_slots[model_idx];
        model_rec = model_binding;
    }
    vector.x = parent->pos.x - (uint32)pos.x;
    vector.y = parent->pos.y - (uint32)pos.y;
    vector.z = parent->pos.z - (uint32)pos.z;
    if (parent->velocity.x == 0 && parent->velocity.y == 0 && parent->velocity.z == 0 && velocity.x == 0 && velocity.y == 0 && velocity.z == 0)
        goto orient;
    x = vector.x >> 6;
    y = vector.y >> 6;
    z = vector.z >> 6;
    sq = x * x + y * y + z * z;
    if ((sint32)sq >= 0)
        distance = SquareRoot12(sq);
    else
        distance = SquareRoot12((sq >> 2)) << 1;
    vector.y = distance;
    if (distance <= 49151 && (distance <= 36863 || (velocity.x == 0 && velocity.y == 0 && velocity.z == 0)))
        goto stop;
    target_frame = parent->path_frame;
    if (scene_path != camera_motion.path)
    {
        if (target_frame < (parent->path_definition->pos_track.frame_count >> 1))
            target_frame = path_frame + 1;
        else
            target_frame = path_frame - 1;
    }
    if (path_frame >= target_frame)
        goto stop;
    x = parent->velocity.x >> 6;
    y = parent->velocity.y >> 6;
    z = parent->velocity.z >> 6;
    sq = x * x + y * y + z * z;
    if ((sint32)sq >= 0)
        speed = SquareRoot12(sq);
    else
        speed = SquareRoot12((sq >> 2)) << 1;
    vector.z = speed;
    vector.x = speed;
    if (vector.y > 49152)
    {
        cap = model_rec->movement.speed_limits[1];
        vector.x = cap;
        if (cap < speed)
            vector.x = speed + (uint32)(speed >> 3);
    }
    cur_flags = flags;
    contact_kind = 1;
    flags = cur_flags | 0x40;
    ComputePathMotion(vector.x, 0, NULL);
    cur_flags = flags;
    contact_kind = 0;
    flags = cur_flags & 0xFFFFFFBF;

    vector = velocity;
    goto orient;
stop:
    velocity.z = 0;
    velocity.y = 0;
    velocity.x = 0;
orient:
    TurnObjectFacing(vector, character_pad.turn_step);
update_animation:
    if (velocity.x != 0 || velocity.y != 0 || velocity.z != 0)
    {
        x = pos.x + (uint32)velocity.x;
        y = pos.y + (uint32)velocity.y;
        z = pos.z + (uint32)velocity.z;
        pos.x = x;
        pos.y = y;
        pos.z = z;
        clip = animation;
        definition = model_rec->animations[2];
    }
    else
    {
        clip = animation;
        definition = model_rec->animations[0];
    }
    if (clip != definition)
    {
        cur_flags = flags;
        animation = definition;
        frame = -1;
        flags = cur_flags | 0x10000;
    }
    clip = animation;
    if (clip != 0)
    {
        sample_frame = frame;
        if (sample_frame < clip->frame_count - 1)
            sample_frame += 1;
        else
            sample_frame = 0;
    }
    else
        sample_frame = 0;
    frame = sample_frame;
    result = ClampAnimationFrame();
    return result;
}

void AnimatedObject::BeginCharacterPathTransition()
{
    FUNCTION_MARKER(0x80056700, "SLUS_005.78");
    uint32 cur_flags, state;
    cur_flags = path_flags;
    state = flags;
    path_flags = cur_flags | 0x100;
    flags = state & 0xFFFFFBFF;
    cur_flags = character_pad.flags;
    character_pad.path_transition.phase = 1;
    character_pad.path_transition.approach_distance = 0xFFFFFFFF;
    character_pad.path_transition.camera_ticks = 0;
    character_pad.path_transition.hold_ticks = 0;
    character_pad.path_transition.release_pending = 0;
    character_pad.motion.jump_phase = -1;
    character_pad.flags = cur_flags & 0xE7FFFFBF;
}

void AnimatedObject::CancelCharacterPathTransition()
{
    FUNCTION_MARKER(0x80056774, "SLUS_005.78");
    uint32 result, trigger;
    path_flags = path_flags & 0xFFFFFC7F;
    result = character_pad.flags;
    trigger = character_pad.motion.slope_scale;
    result &= 0xF7FFFFFF;
    character_pad.flags = result;
    if (trigger != 0)
        SetCharacterPathExitLatch();
}

uint32 AnimatedObject::SetCharacterPathExitLatch()
{
    FUNCTION_MARKER(0x800567D0, "SLUS_005.78");
    uint32 result = path_flags | 0x1000000;
    path_flags = result;
    return result;
}

void AnimatedObject::ClearCharacterPathExitLatch()
{
    FUNCTION_MARKER(0x800567E4, "SLUS_005.78");
    uint32 result = path_flags & 0xFEFFFFFF;
    path_flags = result;
}

void AnimatedObject::ClearCharacterPathTransitionFlags()
{
    FUNCTION_MARKER(0x800567FC, "SLUS_005.78");

    path_flags = path_flags & 0xFFFFFC37;
}

void AnimatedObject::RequestCharacterPathExit()
{
    FUNCTION_MARKER(0x80056810, "SLUS_005.78");
    path_flags = path_flags | 0x40000;
    character_pad.path_transition.phase = 6;
}

void AnimatedObject::CompleteCharacterPathTransfer(CharacterController *pad)
{
    const PathDefinition *definition;
    FUNCTION_MARKER(0x80056884, "SLUS_005.78");
    uint32 saved_path_matches;
    Vec3 world, input, pos_vector, floor_vector;
    MATRIX inverse = {0};
    Vec3 *start;
    uint32 *phase_ticks_slot;
    sint8 *phase_slot;
    const Vec3 *end;
    uint32 cur_flags = path_flags;
    uint32 special = cur_flags & 0x400000;
    const ScenePath *path, *path_view;
    uint32 idx, result;
    MATRIX *matrix;
    Vec3 sample_pos;
    Vec3 delta;
    if (pad == 0)
        return;
    phase_slot = &pad->path_transition.phase;
    if (special == 0)
    {
        if (*phase_slot != 7)
            return;
        result = flags;
        path_flags = cur_flags & 0xFFFFFC37;
        flags = result | 0x400;
        pad->flags = pad->flags | 0x2000;
    }
    else if (*phase_slot != 10)
        return;
    start = &pad->path_transition.start;
    if (path_definition->transform != 0)
    {
        path = camera_motion.path;
        path_view = path;
        definition = path_view->definition;
        if (definition != 0 && definition->transform != 0)
        {
            input.x = pos.x;
            input.y = pos.y;
            input.z = pos.z;
            PushMatrix();
            path = camera_motion.path;
            path_view = path;
            matrix = path_view->definition->transform != 0 ? &path_view->definition->transform->matrix : NULL;
            {
                SetRotMatrix(matrix);
            }
            {
                VECTOR sdk_input = {input.x, input.y, input.z, 0};
                VECTOR sdk_output;
                ApplyRotMatrixLV(&sdk_input, &sdk_output);
                world.x = sdk_output.vx;
                world.y = sdk_output.vy;
                world.z = sdk_output.vz;
            }
            PopMatrix();
            path = camera_motion.path;
            path_view = path;
            matrix = path_view->definition->transform != 0 ? &path_view->definition->transform->matrix : NULL;
            world.x = (uint32)world.x + matrix->t[0];
            path = camera_motion.path;
            path_view = path;
            matrix = path_view->definition->transform != 0 ? &path_view->definition->transform->matrix : NULL;
            world.y = (uint32)world.y + matrix->t[1];
            path = camera_motion.path;
            path_view = path;
            matrix = path_view->definition->transform != 0 ? &path_view->definition->transform->matrix : NULL;
            world.z = (uint32)world.z + matrix->t[2];
        }
        else
        {
            world.x = pos.x;
            world.y = pos.y;
            world.z = pos.z;
        }
        end = &pad->path_transition.end;
        delta.x = end->x - (uint32)start->x;
        delta.y = end->y - (uint32)start->y;
        delta.z = end->z - (uint32)start->z;
        start->x = world.x - (uint32)delta.x;
        start->y = world.y - (uint32)delta.y;
        start->z = world.z - (uint32)delta.z;
    }
    definition = path_definition;
    if (definition != 0 && definition->transform != 0)
    {
        matrix = path_definition->transform != 0 ? &path_definition->transform->matrix : NULL;
        input.x = (uint32)start->x - matrix->t[0];
        matrix = path_definition->transform != 0 ? &path_definition->transform->matrix : NULL;
        input.y = (uint32)start->y - matrix->t[1];
        matrix = path_definition->transform != 0 ? &path_definition->transform->matrix : NULL;
        input.z = (uint32)start->z - matrix->t[2];
        for (idx = 0; idx < 9; ++idx)
        {
            matrix = path_definition->transform != 0 ? &path_definition->transform->matrix : NULL;
            inverse.m[idx / 3][idx % 3] = matrix->m[idx % 3][idx / 3];
        }
        PushMatrix();
        SetRotMatrix(&inverse);

        {
            VECTOR rotation_output;
            {
                VECTOR sdk_input = {input.x, input.y, input.z, 0};
                ApplyRotMatrixLV(&sdk_input, &rotation_output);
            }
            pos.x = rotation_output.vx;
            pos.y = rotation_output.vy;
            pos.z = rotation_output.vz;
        }
        PopMatrix();
    }
    else
    {
        pos.x = start->x;
        pos.y = start->y;
        pos.z = start->z;
    }
    sample_pos.x = start->x;
    sample_pos.y = start->y;
    sample_pos.z = start->z;
    pos_vector.x = sample_pos.x;
    pos_vector.y = sample_pos.y;
    pos_vector.z = sample_pos.z;
    floor_vector.x = sample_pos.x;
    floor_vector.y = sample_pos.y;
    floor_vector.z = sample_pos.z;
    if (special == 0)
    {
        ReverseScenePath();
        if (OriginalDefaults::MarkTransferredPath != 0)
            path_flags = path_flags | 0x2000000;
    }
    else
        path_flags = path_flags | 0x2200000;
    path = camera_motion.path;
    path_view = path;
    ChangeActorPath(path, pos_vector, floor_vector);
    if (special == 0)
    {
        ReverseScenePath();
        /* The original normal branch keeps its transition flag zero */
        path_flags = path_flags & ~0x40000u;
    }
    path = camera_motion.path;
    path_view = path;
    const ScenePath *saved_path = pad->path_transition.path;
    saved_path_matches = path == saved_path;
    if (saved_path_matches != 0)
    {
        result = flags | 0x2000;
        flags = result;
    }
    if (special != 0)
    {
        result = 11;
        *phase_slot = result;
        phase_ticks_slot = &pad->path_transition.phase_ticks;
        *phase_ticks_slot = 0;
    }
}

void AnimatedObject::ExitCharacterPathTransition()
{
    const PathDefinition *definition;
    FUNCTION_MARKER(0x80057208, "SLUS_005.78");
    const ScenePath *camera_path;
    Vec3 sample_pos, floor, local;
    uint32 cur_flags, state, x, y, z;
    MATRIX *matrix;
    cur_flags = path_flags;
    state = flags;
    path_flags = cur_flags & 0xFFFFFC37;
    flags = state | 0x400;
    if (character_pad.motion.state_code == 20 && frame == 0)
    {
        path_flags = path_flags | 0x400000;
        character_pad.path_transition.phase = 10;
    }
    else
    {
        ReverseScenePath();
        definition = camera_motion.path->definition;
        if (definition != 0 && definition->transform != 0)
        {
            local = pos;
            PushMatrix();
            matrix = &camera_motion.path->definition->transform->matrix;
            SetRotMatrix(matrix);
            {
                VECTOR sdk_input = {local.x, local.y, local.z, 0};
                VECTOR sdk_output;
                ApplyRotMatrixLV(&sdk_input, &sdk_output);
                sample_pos.x = sdk_output.vx;
                sample_pos.y = sdk_output.vy;
                sample_pos.z = sdk_output.vz;
            }
            PopMatrix();
            matrix = &camera_motion.path->definition->transform->matrix;
            x = sample_pos.x;
            x += (uint32)matrix->t[0];
            sample_pos.x = x;
            matrix = &camera_motion.path->definition->transform->matrix;
            y = sample_pos.y;
            y += (uint32)matrix->t[1];
            sample_pos.y = y;
            matrix = &camera_motion.path->definition->transform->matrix;
            z = sample_pos.z;
            z += (uint32)matrix->t[2];
            sample_pos.z = z;
        }
        else
        {
            sample_pos = pos;
        }

        floor = sample_pos;
        sample_pos.y = (uint32)sample_pos.y - 0x4000;
        cur_flags = path_flags;
        path_flags = cur_flags | 0x02200000;
        ChangeActorPath(camera_motion.path, sample_pos, floor);
        camera_path = camera_motion.path;
        if (camera_path == character_pad.path_transition.path)
            flags = flags | 0x2000;
    }
    character_pad.path_transition.reentry_ticks = path_transition_rules.reentry_ticks;
}

void AnimatedObject::SampleActorPathStart(const ScenePath *path, Vec3 &dst) const
{
    FUNCTION_MARKER(0x80057480, "SLUS_005.78");
    path->SamplePos(0, dst, (path_flags & 0x400) == 0 ? 64 : 0);
}

void AnimatedObject::BeginCharacterPathTransfer()
{
    const PathDefinition *owner;
    FUNCTION_MARKER(0x80057650, "SLUS_005.78");
    Vec3 world, anchor, scene_cache, floor, sample, local, target;
    Vec3 input, anchor_input, inverse_input, forward_input, first_floor, backup_floor;
    MATRIX transpose = {0}, final_transpose = {0};
    uint32 first_valid;
    const SceneTrigger *anchor_trigger;
    const ScenePath *path, *path_view, *cur;
    uint32 result, cur_flags, val, dir, frame, first_frame;
    MATRIX *matrix;
    uint32 x, y, z, idx;
    sint32 old_frame;
    result = path_flags & 0x40000;
    if (result != 0)
        goto finish_transfer;

    if (character_pad.path_transition.phase >= 4)
        goto finish_transfer;
    cur_flags = flags;
    velocity.z = 0;
    velocity.y = 0;
    velocity.x = 0;
    flags = cur_flags & 0xFFFFFBFF;
    anchor_trigger = character_pad.path_transition.anchor;
    path = anchor_trigger->parameters.path.target;
    path_view = path;
    cur = camera_motion.path;
    if (path == cur)
    {
        result = flags & 0x2000;
        if (result == 0)
            goto finish_transfer;
    }
    if ((flags & 0x2000) == 0 && (owner = path_definition) != 0 && owner->transform != 0)
    {
        input = pos;
        PushMatrix();
        matrix = path_definition->transform != 0 ? &path_definition->transform->matrix : NULL;
        SetRotMatrix(matrix);
        {
            VECTOR sdk_input = {input.x, input.y, input.z, 0};
            VECTOR sdk_output;
            ApplyRotMatrixLV(&sdk_input, &sdk_output);
            world.x = sdk_output.vx;
            world.y = sdk_output.vy;
            world.z = sdk_output.vz;
        }
        PopMatrix();
        matrix = path_definition->transform != 0 ? &path_definition->transform->matrix : NULL;
        val = world.x;
        world.x = val + matrix->t[0];
        matrix = path_definition->transform != 0 ? &path_definition->transform->matrix : NULL;
        val = world.y;
        world.y = val + matrix->t[1];
        matrix = path_definition->transform != 0 ? &path_definition->transform->matrix : NULL;
        val = world.z;
        world.z = val + matrix->t[2];
    }
    else
    {
        world = pos;
    }
    owner = path_view->definition;
    if (owner != 0 && owner->transform != 0)
    {
        anchor_trigger = character_pad.path_transition.anchor;

        anchor_input = anchor_trigger->pos;
        PushMatrix();
        matrix = path_view->definition->transform != 0 ? &path_view->definition->transform->matrix : NULL;
        SetRotMatrix(matrix);
        {
            VECTOR sdk_input = {anchor_input.x, anchor_input.y, anchor_input.z, 0};
            VECTOR sdk_output;
            ApplyRotMatrixLV(&sdk_input, &sdk_output);
            anchor.x = sdk_output.vx;
            anchor.y = sdk_output.vy;
            anchor.z = sdk_output.vz;
        }
        PopMatrix();
        matrix = path_view->definition->transform != 0 ? &path_view->definition->transform->matrix : NULL;
        val = anchor.x;
        anchor.x = val + matrix->t[0];
        matrix = path_view->definition->transform != 0 ? &path_view->definition->transform->matrix : NULL;
        val = anchor.y;
        anchor.y = val + matrix->t[1];
        matrix = path_view->definition->transform != 0 ? &path_view->definition->transform->matrix : NULL;
        val = anchor.z;
        anchor.z = val + matrix->t[2];
    }
    else
    {
        anchor_trigger = character_pad.path_transition.anchor;

        anchor = anchor_trigger->pos;
    }

    character_pad.path_transition.camera_samples[0] = camera_motion.focus_pos;

    scene_cache = character_pad.path_transition.camera_samples[0];

    character_pad.path_transition.start = world;
    dir = (path_flags & 0x400) == 0 ? 64 : 0;
    path_view->SamplePos(0, sample, dir);
    owner = path_view->definition;
    if (owner != 0 && owner->transform != 0)
    {
        matrix = owner->transform != 0 ? &owner->transform->matrix : NULL;
        val = world.x;
        inverse_input.x = val - matrix->t[0];
        matrix = path_view->definition->transform != 0 ? &path_view->definition->transform->matrix : NULL;
        val = world.y;
        inverse_input.y = val - matrix->t[1];
        matrix = path_view->definition->transform != 0 ? &path_view->definition->transform->matrix : NULL;
        val = world.z;
        inverse_input.z = val - matrix->t[2];
        for (idx = 0; idx < 9; ++idx)
        {
            matrix = path_view->definition->transform != 0 ? &path_view->definition->transform->matrix : NULL;
            transpose.m[idx / 3][idx % 3] = matrix->m[idx % 3][idx / 3];
        }
        PushMatrix();
        SetRotMatrix(&transpose);
        {
            VECTOR sdk_input = {inverse_input.x, inverse_input.y, inverse_input.z, 0};
            VECTOR sdk_output;
            ApplyRotMatrixLV(&sdk_input, &sdk_output);
            local.x = sdk_output.vx;
            local.y = sdk_output.vy;
            local.z = sdk_output.vz;
        }
        PopMatrix();
    }
    else
    {
        local = world;
    }
    cur_flags = path_flags;
    val = local.x;
    if ((cur_flags & 0x400) == 0)
        val += path_transition_rules.side_step;
    else
        val -= path_transition_rules.side_step;
    local.x = val;
    owner = path_view->definition;
    if (owner != 0 && owner->transform != 0)
    {
        forward_input = local;
        PushMatrix();
        matrix = path_view->definition->transform != 0 ? &path_view->definition->transform->matrix : NULL;
        SetRotMatrix(matrix);
        {
            VECTOR sdk_input = {forward_input.x, forward_input.y, forward_input.z, 0};
            VECTOR sdk_output;
            ApplyRotMatrixLV(&sdk_input, &sdk_output);
            target.x = sdk_output.vx;
            target.y = sdk_output.vy;
            target.z = sdk_output.vz;
        }
        PopMatrix();
        matrix = path_view->definition->transform != 0 ? &path_view->definition->transform->matrix : NULL;
        val = target.x;
        target.x = val + matrix->t[0];
        matrix = path_view->definition->transform != 0 ? &path_view->definition->transform->matrix : NULL;
        val = target.y;
        target.y = val + matrix->t[1];
        matrix = path_view->definition->transform != 0 ? &path_view->definition->transform->matrix : NULL;
        val = target.z;
        target.z = val + matrix->t[2];
    }
    else
    {
        target = local;
    }
    val = anchor.y;
    target.y = val + 40960;

    character_pad.path_transition.camera_samples[1] = camera_motion.focus_pos;

    character_pad.path_transition.camera_samples[3] = character_pad.path_transition.camera_samples[1];

    character_pad.path_transition.camera_samples[2] = character_pad.path_transition.camera_samples[3];
    val = character_pad.path_transition.camera_samples[1].y;
    if ((sint32)character_pad.flags < 0)
        val -= 32768;
    else
        val -= 14336;
    character_pad.path_transition.camera_samples[1].y = val;

    character_pad.path_transition.camera_samples[0] = character_pad.path_transition.camera_samples[1];
    frame = path_view->definition->FindHeightFrame(target, dir, -40960);
    if ((sint32)frame >= 0)
        frame = path_view->definition->ProjectFrame(frame, target, floor, dir, 8192);
    first_valid = (sint32)frame >= 0;
    if (first_valid)
    {
        first_floor = floor;
        first_frame = frame;
    }
    path_view->SamplePos(0, floor, dir);

    backup_floor = floor;

    target = floor;
    frame = path_view->definition->ProjectFrame(0, target, floor, dir, 8192);
    if ((sint32)frame < 0)
    {
        frame = 0;

        floor = backup_floor;
    }
    // Use the existing endpoint candidate when the height probe fails
    if (first_valid)
    {
        val = floor.x;
        val = val - first_floor.x + 24576;
        if (val > 49152)
            goto restore_first;
        val = floor.z;
        val = val - first_floor.z + 24576;
        if (val > 49152)
        {
        restore_first:
            x = (uint32)first_floor.x;
            y = first_floor.y;
            z = first_floor.z;
            floor.x = x;
            floor.y = y;
            floor.z = z;
            frame = first_frame;
        }
    }
    owner = path_view->definition;
    path_frame = frame;
    path_definition = owner;

    character_pad.path_transition.end = floor;
    owner = path_definition;
    if (owner != 0 && owner->transform != 0)
    {
        matrix = path_definition->transform != 0 ? &path_definition->transform->matrix : NULL;
        val = floor.x;
        first_floor.x = val - matrix->t[0];
        matrix = path_definition->transform != 0 ? &path_definition->transform->matrix : NULL;
        val = floor.y;
        first_floor.y = val - matrix->t[1];
        matrix = path_definition->transform != 0 ? &path_definition->transform->matrix : NULL;
        val = floor.z;
        first_floor.z = val - matrix->t[2];
        for (idx = 0; idx < 9; ++idx)
        {
            matrix = path_definition->transform != 0 ? &path_definition->transform->matrix : NULL;
            final_transpose.m[idx / 3][idx % 3] = matrix->m[idx % 3][idx / 3];
        }
        PushMatrix();
        SetRotMatrix(&final_transpose);
        {
            VECTOR rotation_output;
            {
                VECTOR sdk_input = {first_floor.x, first_floor.y, first_floor.z, 0};
                ApplyRotMatrixLV(&sdk_input, &rotation_output);
            }
            pos.x = rotation_output.vx;
            pos.y = rotation_output.vy;
            pos.z = rotation_output.vz;
        }
        PopMatrix();
    }
    else
    {
        pos = floor;
    }

    camera_motion.focus_pos = world;
    cur_flags = character_pad.flags;
    val = camera_motion.focus_pos.y;
    if ((sint32)cur_flags < 0)
        val += 32768;
    else
        val += 14336;
    camera_motion.focus_pos.y = val;
    old_frame = path_frame;
    cur = camera_motion.path;
    cur_flags = flags;
    owner = cur->definition;
    val = owner->FindHeightFrame(camera_motion.focus_pos, cur_flags & 0x10, -40960);
    path_frame = val;
    UpdateObjectScenePath(path, old_frame);
    character_pad.path_transition.path = path;
    result = flags & 0xFFFFDFFF;
    flags = result;
finish_transfer:
    return;
}

sint32 AnimatedObject::EvaluatePathTransition(const uint32 *input) const
{
    FUNCTION_MARKER(0x8005814C, "SLUS_005.78");
    Vec3 cur, target, sample0, sample1;
    const ScenePath *path;
    const PathDefinition *definition;
    uint32 x, y, z, cur_flags, val;
    PathTransform *path_transform;
    const SceneTrigger *anchor;
    sint32 height;
    uint32 distance;
    uint32 dx;
    uint32 dz;
    uint32 angle;
    uint32 difference;
    uint32 result;
    sint32 frame0, frame1;
    anchor = character_pad.path_transition.anchor;
    path = anchor->parameters.path.target;
    result = 0;
    definition = path->definition;
    path_transform = definition->transform;
    if (path_transform != 0)
    {
        cur_flags = path_flags;
        path->SamplePos(0, target, (cur_flags & 0x400) == 0 ? 64 : 0);
    }
    else
    {
        target = anchor->pos;
    }
    definition = path_definition;
    if (definition != 0 && definition->transform != 0)
    {
        sample0 = pos;
        PushMatrix();
        definition = path_definition;
        path_transform = definition->transform;
        SetRotMatrix(&path_transform->matrix);
        {
            VECTOR sdk_input = {sample0.x, sample0.y, sample0.z, 0};
            VECTOR sdk_output;
            ApplyRotMatrixLV(&sdk_input, &sdk_output);
            cur.x = sdk_output.vx;
            cur.y = sdk_output.vy;
            cur.z = sdk_output.vz;
        }
        PopMatrix();
        definition = path_definition;
        path_transform = definition->transform;
        x = cur.x;
        x += (uint32)path_transform->matrix.t[0];
        cur.x = x;
        definition = path_definition;
        path_transform = definition->transform;
        y = cur.y;
        y += (uint32)path_transform->matrix.t[1];
        cur.y = y;
        definition = path_definition;
        path_transform = definition->transform;
        z = cur.z;
        z += (uint32)path_transform->matrix.t[2];
        cur.z = z;
    }
    else
    {
        cur = pos;
    }
    y = target.y;
    y -= (uint32)cur.y;
    val = path_transition_rules.height_bias;
    x = cur.x;
    height = y - val;
    x -= (uint32)target.x;
    z = cur.z;
    val = target.z;
    if ((sint32)x < 0)
        x = -x;
    y = path_transition_rules.approach_limit;
    z -= val;
    if ((sint32)z < 0)
        z = -z;
    distance = x + z;
    if ((sint32)y < (sint32)distance)
    {
        result = 0xFFFFFFFF;
        goto done;
    }
    if ((path_flags & 1) == 0)
    {
        if (character_pad.motion.vertical_speed < 0 && (sint32)path_transition_rules.max_height < height)
        {
            result = 0xFFFFFFFF;
            goto done;
        }
        if ((path_flags & 1) != 0 && (sint32)path_transition_rules.max_height < height)
        {
            result = 0xFFFFFFFF;
            goto done;
        }
    }
    else if ((sint32)path_transition_rules.max_height < height)
    {
        result = 0xFFFFFFFF;
        goto done;
    }
    frame1 = 1;
    if ((path_flags & 0x400) != 0)
    {
        definition = path->definition;
        frame0 = definition->pos_track.frame_count;
        frame1 = frame0 - 2;
        --frame0;
    }
    else
        frame0 = 0;
    path->SamplePos(frame0, sample0, 64);
    path->SamplePos(frame1, sample1, 64);
    x = sample1.x;
    dx = x - sample0.x;
    y = sample1.y;
    y -= (uint32)sample0.y;
    z = sample1.z;
    dz = z - sample0.z;
    angle = Atan2Angle((-dz), (-dx));
    character_pad.path_transition.heading = angle;

    character_pad.path_transition.probe_pos = path_heading_delta;
    val = yaw;
    difference = val - (sint16)angle;
    if ((sint32)difference < 0)
        difference = -difference;
    if ((sint16)difference >= 2049)
        difference = 4096 - difference;
    character_pad.path_transition.approach_distance = distance;
    cur_flags = path_flags;
    if ((cur_flags & 1) != 0)
    {
        if ((sint32)path_transition_rules.air_height_limit >= height)
            result = 1;
        goto done;
    }
    if (character_pad.motion.vertical_speed > 0)
        goto done;
    if ((cur_flags & 0x400) != 0)
    {
        if ((flags & 0x10) != 0)
            goto done;
    }
    else if ((flags & 0x10) == 0)
        goto done;
    if ((sint16)difference >= (sint16)path_transition_rules.angle_limit)
        goto done;
    if (height < (sint32)path_transition_rules.min_height || (sint32)path_transition_rules.max_height < height)
        goto done;
    if ((path_flags & 0x400) != 0)
    {
        if ((*input & 0x2000) == 0)
            result = 1;
    }
    else if ((*input & 0x8000) == 0)
        result = 1;
done:
    return result;
}

const ScenePath *AnimatedObject::ArmCharacterPathHold()
{
    FUNCTION_MARKER(0x800585E0, "SLUS_005.78");
    uint32 flags;
    const ScenePath *result;
    flags = path_flags;

    path_flags = (flags | 0x10) & 0xFFFFFF91;
    character_pad.path_transition.hold_motion_ticks = 0;
    result = camera_motion.path;
    character_pad.path_transition.anchor_path = result;
    return result;
}

void AnimatedObject::EndCharacterPathHold()
{
    FUNCTION_MARKER(0x80058618, "SLUS_005.78");

    path_flags = path_flags & 0xFFFFFFEF;
    character_pad.path_transition.hold_motion_ticks = 0;
    character_pad.path_transition.movement_ticks = 0;
}

void AnimatedObject::BeginCharacterPathRelease()
{
    FUNCTION_MARKER(0x80058640, "SLUS_005.78");
    const ScenePath *path;
    uint32 flags;
    sint32 mode, speed_limit, speed_scale;
    path_flags = path_flags & 0xFFFFFFEF;
    character_pad.path_transition.hold_motion_ticks = 0;
    character_pad.path_transition.movement_ticks = 0;
    flags = path_flags;

    path_flags = (flags | 0x68) & 0xFFFF7FFF;
    path = camera_motion.path;
    mode = path_transition_rules.release_mode;
    character_pad.path_transition.hold_motion_ticks = 0;
    character_pad.path_transition.anchor_path = path;
    if (mode == 1)
    {
        speed_limit = character_pad.movement.speed_limits[1];
        speed_scale = path_transition_rules.release_speed_scale;
        character_pad.motion.vertical_speed = mul12(speed_scale, speed_limit);
    }
    character_pad.motion.jump_phase = -1;
}

void AnimatedObject::ClearCharacterPathRelease()
{
    FUNCTION_MARKER(0x800586E8, "SLUS_005.78");

    path_flags = path_flags & 0xFFFF7F97;
    character_pad.path_transition.anchor_path = 0;
    character_pad.path_transition.hold_motion_ticks = 0;
}

void AnimatedObject::MarkPathSearchFailed()
{
    FUNCTION_MARKER(0x80058714, "SLUS_005.78");
    uint32 flags = path_flags | 0x800;
    path_flags = flags;
}

void AnimatedObject::ClearCharacterPathStep()
{
    FUNCTION_MARKER(0x80058728, "SLUS_005.78");
    uint32 result = path_flags & 0xFFFFF7F7;
    path_flags = result;
}

void AnimatedObject::BeginCharacterConstrainedPath()
{
    FUNCTION_MARKER(0x8005873C, "SLUS_005.78");
    Vec3 world, prev_pos, cur_pos, delta, sample, dst, tmp, relative;
    MATRIX inverse;
    const PathDefinition *definition;
    MATRIX *matrix;
    const ScenePath *sel, *route;
    uint32 cur_flags, val, x, y, z, a, b, c, idx;
    sint32 frame, prev, count;
    character_pad.flags = character_pad.flags & 0xF7FFFFFF;
    path_flags = path_flags | 1;
    cur_flags = flags;
    val = path_flags;
    flags = cur_flags & 0xFFFFFBFF;
    path_flags = val & 0xFFFFFFB1;
    SceneTrigger *surface = character_pad.surface_interaction.surface;
    character_pad.surface_interaction.surface_phase = 0;
    character_pad.surface_interaction.constrained_reset = 0;
    character_pad.path_transition.input_ready = 0;
    character_pad.surface_interaction.prev_surface = surface;
    character_pad.path_transition.entry_flags = flags;
    sel = camera_motion.path;
    route = sel->adjacent[1];
    character_pad.path_input.dir = 0;
    character_pad.path_input.settle_ticks = 0;
    character_pad.path_input.latch_phase = 0;
    character_pad.path_input.release_ticks = 0;
    character_pad.path_input.prev_motion = 0;
    character_pad.path_input.latched_motion = 0;
    definition = path_definition;
    matrix = definition != 0 && definition->transform != 0 ? &definition->transform->matrix : NULL;
    if (matrix)
    {
        tmp = pos;
        PushMatrix();
        matrix = path_definition->transform != 0 ? &path_definition->transform->matrix : NULL;
        SetRotMatrix(matrix);
        {
            VECTOR sdk_input = {tmp.x, tmp.y, tmp.z, 0};
            VECTOR sdk_output;
            ApplyRotMatrixLV(&sdk_input, &sdk_output);
            world.x = sdk_output.vx;
            world.y = sdk_output.vy;
            world.z = sdk_output.vz;
        }
        PopMatrix();
        matrix = path_definition->transform != 0 ? &path_definition->transform->matrix : NULL;
        x = world.x;
        y = matrix->t[0];
        world.x = x + y;
        matrix = path_definition->transform != 0 ? &path_definition->transform->matrix : NULL;
        x = world.y;
        y = matrix->t[1];
        world.y = x + y;
        matrix = path_definition->transform != 0 ? &path_definition->transform->matrix : NULL;
        x = world.z;
        y = matrix->t[2];
        world.z = x + y;
    }
    else
    {
        world = pos;
    }
    frame = route->definition->FindHeightFrame(world, 64, -40960);
    if (frame != 0)
    {
        count = route->definition->pos_track.frame_count;
        val = (uint32)count - frame - 1;
        if (frame != val)
            frame = (uint32)frame + 1;
        prev = (uint32)frame - 1;
    }
    else
    {
        count = route->definition->pos_track.frame_count;
        frame = count - 1;
        prev = count - 2;
    }
    route->SamplePos(prev, prev_pos, 64);
    route->SamplePos(frame, cur_pos, 64);
    x = cur_pos.x;
    a = prev_pos.x;
    y = cur_pos.y;
    b = prev_pos.y;
    z = cur_pos.z;
    c = prev_pos.z;
    x -= a;
    y -= b;
    z -= c;
    delta.x = x;
    delta.y = y;
    delta.z = z;
    val = Atan2Angle((-z), (-x));
    sel = camera_motion.path;
    character_pad.path_transition.tangent_heading = val;
    character_pad.path_transition.motion_heading = val;
    character_pad.path_transition.constrained_path = sel;
    sel = camera_motion.path;
    sel->SamplePos(0, sample, 64);
    definition = camera_motion.path->definition;
    matrix = definition != 0 && definition->transform != 0 ? &definition->transform->matrix : NULL;
    if (matrix)
    {
        x = sample.x;
        y = matrix->t[0];
        relative.x = x - y;
        matrix = &camera_motion.path->definition->transform->matrix;
        x = sample.y;
        y = matrix->t[1];
        relative.y = x - y;
        matrix = &camera_motion.path->definition->transform->matrix;
        x = sample.z;
        y = matrix->t[2];
        relative.z = x - y;
        for (idx = 0; idx < 9; ++idx)
        {
            matrix = &camera_motion.path->definition->transform->matrix;
            inverse.m[idx / 3][idx % 3] = matrix->m[idx % 3][idx / 3];
        }
        PushMatrix();
        SetRotMatrix(&inverse);
        {
            VECTOR sdk_input = {relative.x, relative.y, relative.z, 0};
            VECTOR sdk_output;
            ApplyRotMatrixLV(&sdk_input, &sdk_output);
            dst.x = sdk_output.vx;
            dst.y = sdk_output.vy;
            dst.z = sdk_output.vz;
        }
        PopMatrix();
    }
    else
    {
        dst = sample;
    }
    pos.x = dst.x;
    pos.z = dst.z;
}

void AnimatedObject::EndCharacterConstrainedPath()
{
    FUNCTION_MARKER(0x80058C78, "SLUS_005.78");
    uint32 result = path_flags & 0xFFFFFFB0;
    path_flags = result;
}

void AnimatedObject::ClearCharacterSurfaceMotion()
{
    FUNCTION_MARKER(0x80058C8C, "SLUS_005.78");
    uint32 result = path_flags & 0xFFFFFFB1;
    path_flags = result;
}

const ScenePath *AnimatedObject::BeginCharacterSurfaceMotion()
{
    FUNCTION_MARKER(0x80058CA0, "SLUS_005.78");
    uint32 cur_flags, object_flags, character_flags;
    const ScenePath *result;
    cur_flags = path_flags;
    object_flags = flags;
    path_flags = (cur_flags & 0xFFFFFFB0) | 0x42;
    flags = object_flags & 0xFFFFFBFF;
    SceneTrigger *surface = character_pad.surface_interaction.surface;
    character_flags = character_pad.secondary_flags;
    character_pad.surface_interaction.surface_phase = 0;
    character_pad.surface_interaction.prev_surface_step = 0;
    character_pad.surface_interaction.surface_step = 0;
    character_pad.surface_interaction.prev_surface = surface;
    character_pad.secondary_flags = character_flags | 4;
    cur_flags = path_flags;

    path_flags = cur_flags | 12;
    character_pad.path_transition.movement_ticks = 0;
    result = camera_motion.path;
    character_pad.path_transition.anchor_path = result;
    return result;
}

void AnimatedObject::UpdateTransitionMotion(uint32 input_flags, uint32 mode, uint32 *horizontal, uint32 *vertical)
{
    FUNCTION_MARKER(0x80058D08, "SLUS_005.78");
    uint32 flags, input, mask;
    AnimationHeader *clip;
    sint32 state, count, threshold;
    flags = path_flags;
    input = gameplay_input.buttons;
    if ((flags & 0x200000) != 0 && (character_pad.secondary_flags & 0x80) == 0)
        character_pad.motion.state_code = 3;
    flags = path_flags;
    if ((flags & 0x40000) != 0)
    {
        if (character_pad.path_transition.phase != 6)
            return;
        if (frame != 0)
            return;
        character_pad.path_transition.phase = 7;
        if ((character_pad.secondary_flags & 0x80) == 0)
            character_pad.motion.state_code = 3;
        path_flags = path_flags | 0x200000;
        return;
    }
    if ((flags & 0x200000) != 0)
        return;
    state = character_pad.path_transition.phase;
    if (state == 1)
    {
        character_pad.motion.vertical_speed = 0;
        *vertical = 0;
        if ((character_pad.secondary_flags & 0x240) != 0)
            character_pad.path_transition.release_pending = 0;
        count = character_pad.path_transition.hold_ticks;
        threshold = path_transition_rules.release_ticks;
        if (count < threshold)
        {
            character_pad.path_transition.hold_ticks = count + 1;
            mask = (path_flags & 0x400) != 0 ? 0x8000 : 0x2000;
            if ((input & mask) == 0)
                character_pad.path_transition.release_pending = 1;
        }
        else if (count == threshold)
        {
            if (character_pad.path_transition.release_pending == 0)
                character_pad.path_transition.phase = 2;
            else
            {
                mask = (path_flags & 0x400) != 0 ? 0x8000 : 0x2000;
                if ((input & mask) != 0)
                    character_pad.path_transition.phase = 2;
            }
        }
        state = character_pad.path_transition.phase;
    }
    if (state == 2)
    {
        character_pad.motion.vertical_speed = 0;
        *vertical = 0;
        character_pad.path_transition.phase = 3;
    }
    else if (state >= 3)
    {
        if (state == 3)
        {
            if (character_pad.motion.state_code == 20)
            {
                clip = animation;
                count = clip->frame_count;
                threshold = frame;
                if (threshold != count - 1)
                {
                    *vertical = 0;
                    character_pad.motion.vertical_speed = 0;
                    goto clip;
                }
            }
            *horizontal = 0;
            character_pad.motion.horizontal_speed = 0;
            *vertical = 0;
            character_pad.motion.vertical_speed = 0;
        }
        character_pad.path_transition.phase = 4;
    }
    else if (state < 0)
        character_pad.path_transition.phase = 4;
clip:
    if ((character_pad.secondary_flags & 0x80) == 0)
        character_pad.motion.state_code = 20;
}

void AnimatedObject::UpdateSpecialAxisMotion(uint32 input_flags, uint32 mode, uint32 *horizontal, uint32 *vertical)
{
    CharacterModelRecord *pointer_rec;
    FUNCTION_MARKER(0x80058F50, "SLUS_005.78");
    uint32 flags, result, delta, val, limit, count;
    sint32 coefficient, scale;
    flags = path_flags;
    if (!(flags & 4))
    {
        result = mode & 8;
        if (result)
            return;
        if (!(character_pad.flags & 0x40000))
        {
            delta = 0;
            if (input_flags & 0x800)
            {
                pointer_rec = character_pad.model;
                coefficient = pointer_rec->motion_scale.x;
                scale = character_pad.movement.speed_limits[1];
                delta = mul12(scale, coefficient);
            }
            else if (input_flags & 0x1000)
            {
                pointer_rec = character_pad.model;
                scale = character_pad.movement.speed_limits[1];
                coefficient = pointer_rec->motion_scale.y;
                delta = -(uint32)mul12(scale, coefficient);
            }
            result = character_pad.motion.vertical_speed + delta;
            character_pad.motion.vertical_speed = result;
            *vertical = result;
        }
        result = (input_flags & 0x800) ? 10 : ((input_flags & 0x1000) ? 12 : 11);
        if (!(character_pad.secondary_flags & 0x80))
            character_pad.motion.state_code = result;
        return;
    }
    result = character_pad.flags & 0x40000;
    if (result)
        return;
    limit = path_transition_rules.axis_input_ticks;
    count = character_pad.path_transition.movement_ticks;

    if (count >= limit)
        return;
    coefficient = path_transition_rules.axis_horizontal_acceleration[(input_flags & 0x30) != 0];
    scale = character_pad.movement.speed_limits[1];
    delta = mul12(coefficient, scale);
    result = character_pad.motion.horizontal_speed + delta;
    character_pad.motion.horizontal_speed = result;
    *horizontal = result;
    coefficient = path_transition_rules.axis_vertical_limits[0];
    scale = character_pad.movement.speed_limits[1];
    val = mul12(coefficient, scale);
    result = character_pad.motion.vertical_speed;
    if ((sint32)val >= (sint32)result)
        return;
    coefficient = path_transition_rules.axis_vertical_limits[(input_flags & 0x30) != 0];
    scale = character_pad.movement.speed_limits[1];
    val = mul12(coefficient, scale);

    character_pad.motion.vertical_speed = val;
    *vertical = val;
}

void AnimatedObject::ClearCharacterPathAuxiliaryMode()
{
    FUNCTION_MARKER(0x80059268, "SLUS_005.78");
    uint32 result = path_flags & 0xFFFFEFBF;
    path_flags = result;
}

void AnimatedObject::EmptyActionOperation0(void)
{
    FUNCTION_MARKER(0x8005927C, "SLUS_005.78");
}

void AnimatedObject::ApplyCameraPosCorrection(const AnimatedObject *owner, AnimatedObject *object)
{
    FUNCTION_MARKER(0x80059284, "SLUS_005.78");
    Vec3 world, input;
    MATRIX inverse = {0};
    Vec3 movement;
    const PathDefinition *definition;
    MATRIX *matrix;
    uint32 idx;
    if (path_transition_rules.follow_enabled == 0)
        return;
    if (path_transition_rules.follow_limit < character_pad.path_transition.camera_ticks)
        return;
    character_pad.path_transition.correction_mode = 1;
    definition = object->path_definition;
    if (definition != 0 && definition->transform != 0)
    {
        input.x = object->pos.x;
        input.y = object->pos.y;
        input.z = object->pos.z;
        PushMatrix();
        matrix = object->path_definition->transform != 0 ? &object->path_definition->transform->matrix : NULL;
        {
            SetRotMatrix(matrix);
        }
        {
            VECTOR sdk_input = {input.x, input.y, input.z, 0};
            VECTOR sdk_output;
            ApplyRotMatrixLV(&sdk_input, &sdk_output);
            world.x = sdk_output.vx;
            world.y = sdk_output.vy;
            world.z = sdk_output.vz;
        }
        PopMatrix();
        matrix = object->path_definition->transform != 0 ? &object->path_definition->transform->matrix : NULL;
        world.x = (uint32)world.x + matrix->t[0];
        matrix = object->path_definition->transform != 0 ? &object->path_definition->transform->matrix : NULL;
        world.y = (uint32)world.y + matrix->t[1];
        matrix = object->path_definition->transform != 0 ? &object->path_definition->transform->matrix : NULL;
        world.z = (uint32)world.z + matrix->t[2];
    }
    else
    {
        world.x = object->pos.x;
        world.y = object->pos.y;
        world.z = object->pos.z;
    }
    movement.x = character_pad.path_transition.camera_samples[1].x - (uint32)world.x;
    movement.y = character_pad.path_transition.camera_samples[1].y - (uint32)world.y;
    movement.z = character_pad.path_transition.camera_samples[1].z - (uint32)world.z;
    movement.y = movement.y + path_transition_rules.follow_height;
    if (path_transition_rules.follow_side_ticks >= character_pad.path_transition.camera_ticks)
    {
        uint32 heading = character_pad.path_transition.heading;
        if (heading - 512u >= 0xC01)
            movement.z = movement.z - path_transition_rules.follow_side_offset;
        else if ((sint16)heading < 1536)
            movement.x = movement.x + path_transition_rules.follow_side_offset;
        else if ((sint16)heading < 2560)
            movement.z = movement.z + path_transition_rules.follow_side_offset;
        else
            movement.x = movement.x - path_transition_rules.follow_side_offset;
    }
    world.x = world.x + (uint32)movement.x;
    world.y = world.y + (uint32)movement.y;
    world.z = world.z + (uint32)movement.z;
    character_pad.path_transition.correction.x = movement.x;
    character_pad.path_transition.correction.y = movement.y;
    character_pad.path_transition.correction.z = movement.z;
    definition = object->path_definition;
    if (definition != 0 && definition->transform != 0)
    {
        matrix = object->path_definition->transform != 0 ? &object->path_definition->transform->matrix : NULL;
        input.x = (uint32)world.x - matrix->t[0];
        matrix = object->path_definition->transform != 0 ? &object->path_definition->transform->matrix : NULL;
        input.y = (uint32)world.y - matrix->t[1];
        matrix = object->path_definition->transform != 0 ? &object->path_definition->transform->matrix : NULL;
        input.z = (uint32)world.z - matrix->t[2];
        for (idx = 0; idx < 9; ++idx)
        {
            matrix = object->path_definition->transform != 0 ? &object->path_definition->transform->matrix : NULL;
            inverse.m[idx / 3][idx % 3] = matrix->m[idx % 3][idx / 3];
        }
        PushMatrix();
        SetRotMatrix(&inverse);

        {
            VECTOR rotation_output;
            {
                VECTOR sdk_input = {input.x, input.y, input.z, 0};
                ApplyRotMatrixLV(&sdk_input, &rotation_output);
            }
            object->pos.x = rotation_output.vx;
            object->pos.y = rotation_output.vy;
            object->pos.z = rotation_output.vz;
        }
        PopMatrix();
    }
    else
    {
        object->pos.x = world.x;
        object->pos.y = world.y;
        object->pos.z = world.z;
    }
}

void AnimatedObject::ApplyHeadingPosCorrection(const AnimatedObject *owner, AnimatedObject *object)
{
    FUNCTION_MARKER(0x800596CC, "SLUS_005.78");
    Vec3 world, input;
    MATRIX inverse = {0};
    Vec3 pos;
    const PathDefinition *definition;
    MATRIX *matrix;
    uint32 idx, amplitude, step_x, step_z;
    sint16 heading;
    sint32 state;
    if (path_transition_rules.follow_enabled == 0)
        return;
    character_pad.path_transition.correction_mode = 2;
    definition = object->path_definition;
    if (definition != 0 && definition->transform != 0)
    {
        input.x = object->pos.x;
        input.y = object->pos.y;
        input.z = object->pos.z;
        PushMatrix();
        matrix = object->path_definition->transform != 0 ? &object->path_definition->transform->matrix : NULL;
        {
            SetRotMatrix(matrix);
        }
        {
            VECTOR sdk_input = {input.x, input.y, input.z, 0};
            VECTOR sdk_output;
            ApplyRotMatrixLV(&sdk_input, &sdk_output);
            world.x = sdk_output.vx;
            world.y = sdk_output.vy;
            world.z = sdk_output.vz;
        }
        PopMatrix();
        matrix = object->path_definition->transform != 0 ? &object->path_definition->transform->matrix : NULL;
        world.x = (uint32)world.x + matrix->t[0];
        matrix = object->path_definition->transform != 0 ? &object->path_definition->transform->matrix : NULL;
        world.y = (uint32)world.y + matrix->t[1];
        matrix = object->path_definition->transform != 0 ? &object->path_definition->transform->matrix : NULL;
        world.z = (uint32)world.z + matrix->t[2];
    }
    else
    {
        world.x = object->pos.x;
        world.y = object->pos.y;
        world.z = object->pos.z;
    }
    state = character_pad.motion.state_code;
    amplitude = state == 11 ? 4096 : (state == 10 ? 2048 : 3072);
    heading = owner->yaw;
    step_x = mul12((SinQ14(heading) >> 2), amplitude);
    step_z = mul12((SinQ14((1024 - heading)) >> 2), amplitude);
    pos.x = world.x + step_x;
    pos.y = world.y;
    pos.z = world.z + step_z;
    world.x = pos.x;
    world.y = pos.y;
    world.z = pos.z;
    definition = object->path_definition;
    if (definition != 0 && definition->transform != 0)
    {
        matrix = object->path_definition->transform != 0 ? &object->path_definition->transform->matrix : NULL;
        input.x = (uint32)pos.x - matrix->t[0];
        matrix = object->path_definition->transform != 0 ? &object->path_definition->transform->matrix : NULL;
        input.y = (uint32)pos.y - matrix->t[1];
        matrix = object->path_definition->transform != 0 ? &object->path_definition->transform->matrix : NULL;
        input.z = (uint32)pos.z - matrix->t[2];
        for (idx = 0; idx < 9; ++idx)
        {
            matrix = object->path_definition->transform != 0 ? &object->path_definition->transform->matrix : NULL;
            inverse.m[idx / 3][idx % 3] = matrix->m[idx % 3][idx / 3];
        }
        PushMatrix();
        SetRotMatrix(&inverse);

        {
            VECTOR rotation_output;
            {
                VECTOR sdk_input = {input.x, input.y, input.z, 0};
                ApplyRotMatrixLV(&sdk_input, &rotation_output);
            }
            object->pos.x = rotation_output.vx;
            object->pos.y = rotation_output.vy;
            object->pos.z = rotation_output.vz;
        }
        PopMatrix();
    }
    else
    {
        object->pos.x = pos.x;
        object->pos.y = pos.y;
        object->pos.z = pos.z;
    }
}

void AnimatedObject::EmptyActionOperation1(void)
{
    FUNCTION_MARKER(0x80059AC4, "SLUS_005.78");
}

void AnimatedObject::EmptyActionOperation2(void)
{
    FUNCTION_MARKER(0x80059ACC, "SLUS_005.78");
}

void AnimatedObject::EmptyActionOperation3(void)
{
    FUNCTION_MARKER(0x80059AD4, "SLUS_005.78");
    /* Original function returns without side effects */
}

Vec3 *AnimatedObject::UpdateCharacterCameraTarget(Vec3 *dst, const AnimatedObject *object)
{
    const PathDefinition *parent;
    const PathDefinition *object_definition;
    PathTransform *object_transform;
    const AnimationHeader *active_animation;
    FUNCTION_MARKER(0x80059AEC, "SLUS_005.78");
    const ScenePath *camera_path;
    const PathDefinition *camera_definition;
    Vec3 local, world;
    uint32 operation, val, counter;
    MATRIX *matrix;
    uint32 x, y, z, prev_x, prev_y, prev_z, dx, dy, dz, enabled;
    sint32 duration, threshold;
    operation = 0;
    if ((object->path_flags & 0x40000) != 0)
    {
        if (character_pad.motion.state_code == 20)
        {
            duration = object->frame + 1;
            operation = 3;
        }
        else if ((object->flags & 0x2000) != 0)
        {
            x = object->pos.x;
            y = object->pos.y;
            z = object->pos.z;
            goto publish;
        }
    }
    else
    {
        counter = character_pad.path_transition.camera_ticks;
        threshold = path_transition_rules.camera_duration;
        if (counter < (uint32)threshold)
        {
            enabled = 0;
            if (path_transition_rules.wait_animation_start == 0)
                enabled = 1;
            else if (character_pad.motion.state_code == 20 && object->frame != 0)
            {
                active_animation = object->animation;
                counter = path_transition_rules.animation_tail_frames;
                val = (uint8)active_animation->frame_count;
                path_transition_rules.camera_duration = val - counter;
                enabled = 1;
            }
            if (enabled != 0)
            {
                threshold = path_transition_rules.camera_duration;
                counter = character_pad.path_transition.camera_ticks;
                prev_x = character_pad.path_transition.camera_samples[0].x;
                prev_y = character_pad.path_transition.camera_samples[0].y;
                prev_z = character_pad.path_transition.camera_samples[0].z;
                duration = threshold - (sint32)counter + 1;
                if (duration <= 0)
                    duration = 1;
                ++counter;
                character_pad.path_transition.camera_ticks = counter;
                if ((counter & 0xFF) != 0)
                {
                    prev_x = camera_motion.focus_pos.x;
                    prev_y = camera_motion.focus_pos.y;
                    prev_z = camera_motion.focus_pos.z;
                }
                operation = 1;
            }
            else
            {
                camera_path = camera_motion.path;
                camera_definition = camera_path->definition;
                if (camera_definition->transform == 0)
                {
                    x = character_pad.path_transition.camera_samples[1].x;
                    y = character_pad.path_transition.camera_samples[1].y;
                    z = character_pad.path_transition.camera_samples[1].z;
                    goto publish;
                }
                operation = 2;
            }
        }
    }
    parent = object->path_definition;
    if (parent != 0 && parent->transform != 0)
    {
        local = object->pos;
        PushMatrix();
        object_definition = object->path_definition;
        object_transform = object_definition->transform;
        matrix = object_transform != 0 ? &object_transform->matrix : NULL;
        SetRotMatrix(matrix);
        {
            VECTOR sdk_input = {local.x, local.y, local.z, 0};
            VECTOR sdk_output;
            ApplyRotMatrixLV(&sdk_input, &sdk_output);
            world.x = sdk_output.vx;
            world.y = sdk_output.vy;
            world.z = sdk_output.vz;
        }
        PopMatrix();
        object_definition = object->path_definition;
        object_transform = object_definition->transform;
        matrix = object_transform != 0 ? &object_transform->matrix : NULL;
        x = world.x;
        x += (uint32)matrix->t[0];
        world.x = x;
        object_definition = object->path_definition;
        object_transform = object_definition->transform;
        matrix = object_transform != 0 ? &object_transform->matrix : NULL;
        y = world.y;
        y += (uint32)matrix->t[1];
        world.y = y;
        object_definition = object->path_definition;
        object_transform = object_definition->transform;
        matrix = object_transform != 0 ? &object_transform->matrix : NULL;
        z = world.z;
        z += (uint32)matrix->t[2];
        world.z = z;
    }
    else
    {
        x = object->pos.x;
        y = object->pos.y;
        z = object->pos.z;
    }
    if (operation == 1)
    {
        dx = x - prev_x;
        dy = y - prev_y;
        dz = z - prev_z;
        x = prev_x + ((sint32)dx / duration);
        y = prev_y + ((sint32)dy / duration);
        z = prev_z + ((sint32)dz / duration);
    }
    else if (operation == 2)
    {
        dx = x - character_pad.path_transition.end.x;
        dy = y - character_pad.path_transition.end.y;
        dz = z - character_pad.path_transition.end.z;
        x = character_pad.path_transition.camera_samples[2].x;
        y = character_pad.path_transition.camera_samples[2].y;
        z = character_pad.path_transition.camera_samples[2].z;
        x += dx;
        y += dy;
        z += dz;
    }
    else if (operation == 3)
    {
        if (duration == 0)
            xport_mips_break(7);
        y += (uint32)(-32768 / duration);
    }
publish:
    character_pad.path_transition.camera_samples[1].x = x;
    character_pad.path_transition.camera_samples[1].y = y;
    character_pad.path_transition.camera_samples[1].z = z;
    dst->x = x;
    dst->y = y;
    dst->z = z;
    return dst;
}

uint32 AnimatedObject::BeginCharacterInteraction(AnimatedObject *object)
{
    FUNCTION_MARKER(0x8005A440, "SLUS_005.78");
    CharacterController *character = &character_pad;
    uint32 flags = character->secondary_flags;
    character_action_flags = 4;
    character_pad.timers.interaction_ticks = 900;
    character_pad.action_ticks = 0;
    character_pad.phase_ticks = 0;
    character_pad.interaction_phase = 0;
    character_action_owner = object;
    character_action_target = 0;
    character_pad.action13_count = 0;
    character_pad.action14_count = 0;
    character->secondary_flags = flags | 0x100;
    return 900;
}

void AnimatedObject::UpdateCharacterInteraction()
{
    FUNCTION_MARKER(0x8005A7F4, "SLUS_005.78");
    const ActorModelDefinition *bound_model;
    uint32 cur_flags, result;
    uint32 cleanup_flags;
    ActorBinding *cleanup_binding;
    AnimatedObject *cleanup_target;
    sint32 action_idx;
    CharacterController *character = &character_pad;
    if (hold_interaction_timeout == 0)
        character_pad.timers.interaction_ticks = character_pad.timers.interaction_ticks - 1u;
    character_pad.phase_ticks = (uint8)character_pad.phase_ticks + 1u;
    cur_flags = character_action_flags;
    if ((cur_flags & 0x800) != 0)
    {
        character_action_flags = cur_flags & ~0x800u;
        character_pad.phase_ticks = 0;
        character_pad.interaction_phase = 0;
        cleanup_flags = character_action_flags;
        if (cleanup_flags != 0)
        {
            cleanup_target = character_action_target;
            if (cleanup_target != 0)
            {
                cleanup_binding = (ActorBinding *)(((const ActorPlacement *)cleanup_target->path_definition)->state);
                cleanup_binding->flags &= 0xF7;
            }
            cleanup_flags = character_action_flags;
        }
        if ((cleanup_flags & 8) != 0)
            character_pad.RemoveAction(12);
        cleanup_flags = character_action_flags;
        character_action_flags = cleanup_flags & 0xFFFFEFF7;
        if ((cleanup_flags & 16) != 0)
        {
            action_idx = 0;
            while (action_idx < character_pad.action13_count)
            {
                character_pad.RemoveAction(13);
                ++action_idx;
            }
            character_pad.action13_count = 0;
        }
        if ((character_action_flags & 32) != 0)
        {
            action_idx = 0;
            while (action_idx < character_pad.action14_count)
            {
                character_pad.RemoveAction(14);
                ++action_idx;
            }
            character_pad.action14_count = 0;
        }
        character_action_flags = character_action_flags & ~48u;
    }
    switch (character_pad.interaction_phase)
    {
        case 0:
            if ((flags & 0x1000400) == 0x1000000)
            {
                character_pad.phase_ticks = 0;
                character_pad.interaction_phase = 1;
            }
            break;
        case 1:
        {
            sint32 tick = character_pad.phase_ticks;
            if (tick == 1)
            {
                AnimatedObject::QueueCharacterActionSound(16, 3);
                AnimatedObject::SpawnActionEffect(this, &character_pad, 11);
            }
            else if (tick == 4)
            {
                cur_flags = character_action_flags;
                character_action_target = 0;
                character_action_flags = cur_flags | 0x200;
            }
            else if (tick == 5 || tick == 6)
            {
                AnimatedObject *target = character_action_target;

                const PathDefinition *definition;
                PathTransform *path_transform;
                ActorBinding *data;

                Vec3 world, input, other;

                if (tick == 5 && target == 0)
                {
                    character_action_flags = character_action_flags | 0x200;
                    break;
                }
                character_pad.phase_ticks = 0;
                target = character_action_target;
                if (target == 0)
                {
                    character_pad.interaction_phase = 0;
                    break;
                }
                data = (ActorBinding *)(((const ActorPlacement *)target->path_definition)->state);
                cur_flags = character_action_flags;
                bound_model = data->model;
                character_action_flags = cur_flags & ~0x200u;
                definition = target->path_definition;
                if (definition != 0 && definition->transform != 0)
                {
                    input.x = target->pos.x;
                    input.y = target->pos.y;
                    input.z = target->pos.z;
                    PushMatrix();
                    target = character_action_target;
                    path_transform = target->path_definition->transform;
                    {
                        SetRotMatrix(&path_transform->matrix);
                    }
                    {
                        VECTOR sdk_input = {input.x, input.y, input.z, 0};
                        VECTOR sdk_output;
                        ApplyRotMatrixLV(&sdk_input, &sdk_output);
                        world.x = sdk_output.vx;
                        world.y = sdk_output.vy;
                        world.z = sdk_output.vz;
                    }
                    PopMatrix();
                    target = character_action_target;
                    path_transform = target->path_definition->transform;
                    world.x = world.x + (uint32)path_transform->matrix.t[0];
                    path_transform = target->path_definition->transform;
                    world.y = world.y + (uint32)path_transform->matrix.t[1];
                    path_transform = target->path_definition->transform;
                    world.z = world.z + (uint32)path_transform->matrix.t[2];
                }
                else
                {
                    target = character_action_target;
                    world.x = target->pos.x;
                    world.y = target->pos.y;
                    world.z = target->pos.z;
                }
                data->interaction_height = world.y;
                if (bound_model->interaction_animation_mode != -1 || bound_model->interaction_limit_step != 0)
                {
                    definition = path_definition;
                    if (definition != 0 && definition->transform != 0)
                    {
                        input.x = pos.x;
                        input.y = pos.y;
                        input.z = pos.z;
                        PushMatrix();
                        path_transform = path_definition->transform;
                        {
                            SetRotMatrix(&path_transform->matrix);
                        }
                        {
                            VECTOR sdk_input = {input.x, input.y, input.z, 0};
                            VECTOR sdk_output;
                            ApplyRotMatrixLV(&sdk_input, &sdk_output);
                            other.x = sdk_output.vx;
                            other.y = sdk_output.vy;
                            other.z = sdk_output.vz;
                        }
                        PopMatrix();
                        path_transform = path_definition->transform;
                        other.x = other.x + (uint32)path_transform->matrix.t[0];
                        path_transform = path_definition->transform;
                        other.y = other.y + (uint32)path_transform->matrix.t[1];
                        path_transform = path_definition->transform;
                        other.z = other.z + (uint32)path_transform->matrix.t[2];
                    }
                    else
                    {
                        other.x = pos.x;
                        other.y = pos.y;
                        other.z = pos.z;
                    }
                    result = Atan2Angle((world.z - (uint32)other.z), (world.x - (uint32)other.x));
                }
                else
                    result = (uint16)character_action_target->yaw;
                character_pad.action_heading = result;
                character_action_flags = character_action_flags | 8;
                data->flags |= 8;
                AnimatedObject::SpawnActionEffect(character_action_target, &character_pad, 12);
                if ((character_action_flags & 0x1000) != 0 || bound_model->interaction_limit_step == 0)
                    character_pad.interaction_phase = 2;
                else
                {
                    character_pad.interaction_phase = 4;
                    character_pad.action13_scale = 4096;
                    character_pad.action_pending = 0;
                    character_pad.action_delay = 0;
                    character_pad.action_mode = 0;
                    character_pad.action_latched = 0;
                    character_pad.action13_count = 0;
                    character_pad.action14_count = 0;
                    character_pad.action13_limit_reached = 0;
                    character_pad.action14_limit_reached = 0;
                    character_pad.action_completion = 0;
                    character_pad.action13_endpoint = -1;
                    character_pad.action14_endpoint = -1;
                    character_pad.action_shared_limit = -1;
                    character_pad.action_limit_step = bound_model->interaction_limit_step;
                    character_pad.action_delay_bias = bound_model->interaction_delay_bias;
                }
            }
            break;
        }
        case 2:
            if (character_action_target == 0 || character_action_target->ticks <= 0)
            {
                if ((character_action_flags & 8) != 0)
                    character_pad.RemoveAction(12);
                character_pad.interaction_phase = 0;
                cleanup_flags = character_action_flags;
                if (cleanup_flags != 0)
                {
                    cleanup_target = character_action_target;
                    if (cleanup_target != 0)
                    {
                        cleanup_binding = (ActorBinding *)(((const ActorPlacement *)cleanup_target->path_definition)->state);
                        cleanup_binding->flags &= 0xF7;
                    }
                    cleanup_flags = character_action_flags;
                }
                if ((cleanup_flags & 8) != 0)
                    character_pad.RemoveAction(12);
                cleanup_flags = character_action_flags;
                character_action_flags = cleanup_flags & 0xFFFFEFF7;
                if ((cleanup_flags & 16) != 0)
                {
                    action_idx = 0;
                    while (action_idx < character_pad.action13_count)
                    {
                        character_pad.RemoveAction(13);
                        ++action_idx;
                    }
                    character_pad.action13_count = 0;
                }
                if ((character_action_flags & 32) != 0)
                {
                    action_idx = 0;
                    while (action_idx < character_pad.action14_count)
                    {
                        character_pad.RemoveAction(14);
                        ++action_idx;
                    }
                    character_pad.action14_count = 0;
                }
                character_action_flags = character_action_flags & ~48u;
            }
            else
            {
                if ((flags & 0x1000400) == 0x1000000)
                {
                    AnimatedObject::QueueCharacterActionSound(16, 3);
                    AnimatedObject::SpawnActionEffect(this, &character_pad, 11);
                    character_pad.phase_ticks = 0;
                    AnimatedObject::SpawnActionEffect(this, &character_pad, 13);
                    character_action_flags = character_action_flags | 16;
                }
                if (character_pad.action_completion == 1)
                {
                    AnimatedObject *target;
                    ActorBinding *data;
                    character_pad.action_completion = 0;
                    target = character_action_target;
                    if (target->ticks != 0)
                    {
                        data = (ActorBinding *)(((const ActorPlacement *)target->path_definition)->state);
                        cur_flags = data->reaction_flags;
                        result = raise_completed_target;
                        data->reaction_flags = (cur_flags & 0xFFFF01FF) | 0x208;
                        if (result != 0)
                        {
                            target->pos.y = target->pos.y + 3072u;
                            data->flags |= 16;
                        }
                    }
                }
            }
            break;
        case 4:
            if (character_action_target == 0 || character_action_target->ticks <= 0)
            {
                character_pad.phase_ticks = 0;
                character_pad.interaction_phase = 0;
                cleanup_flags = character_action_flags;
                if (cleanup_flags != 0)
                {
                    cleanup_target = character_action_target;
                    if (cleanup_target != 0)
                    {
                        cleanup_binding = (ActorBinding *)(((const ActorPlacement *)cleanup_target->path_definition)->state);
                        cleanup_binding->flags &= 0xF7;
                    }
                    cleanup_flags = character_action_flags;
                }
                if ((cleanup_flags & 8) != 0)
                    character_pad.RemoveAction(12);
                cleanup_flags = character_action_flags;
                character_action_flags = cleanup_flags & 0xFFFFEFF7;
                if ((cleanup_flags & 16) != 0)
                {
                    action_idx = 0;
                    while (action_idx < character_pad.action13_count)
                    {
                        character_pad.RemoveAction(13);
                        ++action_idx;
                    }
                    character_pad.action13_count = 0;
                }
                if ((character_action_flags & 32) != 0)
                {
                    action_idx = 0;
                    while (action_idx < character_pad.action14_count)
                    {
                        character_pad.RemoveAction(14);
                        ++action_idx;
                    }
                    character_pad.action14_count = 0;
                }
                character_action_flags = character_action_flags & ~48u;
                break;
            }
            if ((flags & 0x1000400) == 0x1000000)
            {
                AnimatedObject::QueueCharacterActionSound(16, 3);
                AnimatedObject::SpawnActionEffect(this, &character_pad, 11);
                character_pad.phase_ticks = 0;
                AnimatedObject::SpawnActionEffect(this, &character_pad, 13);
                character_action_flags = character_action_flags | 16;
                character_pad.action_mode = 1;
            }
            if (character_pad.action_pending != 0)
            {
                sint32 table_idx, delay;
                character_pad.action14_scale = character_pad.action_mode == 1 ? 4096 : 2048;
                AnimatedObject::QueueCharacterActionSound(16, 0);
                AnimatedObject::SpawnActionEffect(this, &character_pad, 14);
                character_action_flags = character_action_flags | 32;
                table_idx = character_pad.action_ticks % 16;
                character_pad.action_pending = 0;
                delay = OriginalDefaults::interaction_wave[table_idx] / 2048;
                character_pad.action_delay = delay;
                delay += (uint8)character_pad.action_delay_bias;
                character_pad.action_delay = (sint8)delay * 4;
            }
            else
            {
                if (character_pad.action_latched != 0 && character_pad.action_mode != 1)
                    goto interaction_result;
                result = (uint8)character_pad.action_delay - 1u;
                character_pad.action_delay = result;
                if ((sint8)result > 0 || pause_interaction_actions != 0 || (character_pad.action_latched != 0 && character_pad.action_mode != 1))
                {
                    goto interaction_result;
                }

                {
                    uint32 pending = (uint8)character_pad.action_pending;
                    sint32 table_idx = character_pad.action_ticks % 16;
                    sint32 delay;
                    character_pad.action_latched = 1;
                    character_pad.action_pending = pending + 1u;
                    delay = OriginalDefaults::interaction_wave[table_idx] / 2048;
                    result = (uint8)character_pad.action_delay_bias;
                    character_pad.action_delay = delay;
                    character_pad.action_delay = delay + result;
                }
            }
        interaction_result:
            if (character_pad.action_completion == 1)
            {
                character_pad.action_completion = 0;
                if ((character_action_flags & 16) != 0)
                {
                    action_idx = 0;
                    while (action_idx < character_pad.action13_count)
                    {
                        character_pad.RemoveAction(13);
                        ++action_idx;
                    }
                    character_pad.action13_count = 0;
                }
                if ((character_action_flags & 32) != 0)
                {
                    action_idx = 0;
                    while (action_idx < character_pad.action14_count)
                    {
                        character_pad.RemoveAction(14);
                        ++action_idx;
                    }
                    character_pad.action14_count = 0;
                }
                character_action_flags = (character_action_flags & ~48u) | 0x3000;
                character_pad.interaction_phase = 5;
                character_pad.action_ticks = 0;
            }
            else if (character_pad.action_completion == 2)
            {
                character_pad.action_completion = 0;
                cur_flags = character_action_flags;
                character_pad.support.contact = 0;
                character_pad.contacts.damage_fraction = 0;
                character_action_flags = cur_flags | 0x2100;
            }
            break;
        case 5:
            result = (uint8)character_pad.action_ticks + 1u;
            character_pad.action_ticks = result;
            if ((sint8)result >= 3)
            {
                character_action_flags = character_action_flags & ~0x2000u;
                character_pad.action_ticks = 0;
                character_pad.interaction_phase = 2;
            }
            break;
        default:
            character_pad.phase_ticks = 0;
            character_pad.action_ticks = 0;
            character_pad.interaction_phase = 0;
            break;
    }
    if (character_pad.timers.interaction_ticks == 0)
    {
        cleanup_flags = character_action_flags;
        if (cleanup_flags != 0)
        {
            cleanup_target = character_action_target;
            if (cleanup_target != 0)
            {
                cleanup_binding = (ActorBinding *)(((const ActorPlacement *)cleanup_target->path_definition)->state);
                cleanup_binding->flags &= 0xF7;
            }
            cleanup_flags = character_action_flags;
        }
        if ((cleanup_flags & 8) != 0)
            character_pad.RemoveAction(12);
        cleanup_flags = character_action_flags;
        character_action_flags = cleanup_flags & 0xFFFFEFF7;
        if ((cleanup_flags & 16) != 0)
        {
            action_idx = 0;
            while (action_idx < character_pad.action13_count)
            {
                character_pad.RemoveAction(13);
                ++action_idx;
            }
            character_pad.action13_count = 0;
        }
        if ((character_action_flags & 32) != 0)
        {
            action_idx = 0;
            while (action_idx < character_pad.action14_count)
            {
                character_pad.RemoveAction(14);
                ++action_idx;
            }
            character_pad.action14_count = 0;
        }
        cur_flags = character->secondary_flags;
        character_action_flags = 0;
        character->secondary_flags = cur_flags & ~0x100u;
    }
    cur_flags = flags;
    result = (uint8)character_pad.phase_ticks;
    character_pad.action_disp_phase = result;
    flags = cur_flags & ~0x1000000u;
}

void AnimatedObject::UpdateCharacterPathTimeout()
{
    FUNCTION_MARKER(0x8005B588, "SLUS_005.78");
    uint32 result, limit, count;
    if (!character_pad.path_transition.movement_ticks)
        character_pad.timers.jump_input_ticks = -1;
    result = (uint16)character_pad.timers.jump_input_ticks;
    limit = path_transition_rules.early_movement_ticks;
    count = character_pad.path_transition.movement_ticks;
    ++result;
    character_pad.timers.jump_input_ticks = result;
    if (count >= limit)
    {
        path_flags = path_flags & 0xFFFFFFB1;
        character_pad.secondary_flags = character_pad.secondary_flags & 0xFFFFFFFB;
        flags = flags | 0x400;
        result = character_pad.flags | 0x2000;
        character_pad.flags = result;
    }
}

sint32 AnimatedObject::ClassifyConstrainedContact(const AnimatedObject *other) const
{
    FUNCTION_MARKER(0x8005B604, "SLUS_005.78");

    Vec3 pos_track[2], input;

    MATRIX inverse;
    uint32 side, idx;
    sint32 result = 0;
    for (side = 0; side < 2; ++side)
    {
        const AnimatedObject *object = side == 0 ? this : other;
        Vec3 *dst = &pos_track[side];
        const PathDefinition *definition = object->path_definition;
        const PathTransform *path_transform;
        if (definition != 0 && definition->transform != 0)
        {
            path_transform = object->path_definition->transform;
            input.x = (uint32)object->pos.x - path_transform->matrix.t[0];
            path_transform = object->path_definition->transform;
            input.y = (uint32)object->pos.y - path_transform->matrix.t[1];
            path_transform = object->path_definition->transform;
            input.z = (uint32)object->pos.z - path_transform->matrix.t[2];
            for (idx = 0; idx < 9; ++idx)
            {
                path_transform = object->path_definition->transform;
                inverse.m[idx / 3][idx % 3] = path_transform->matrix.m[idx % 3][idx / 3];
            }
            PushMatrix();
            SetRotMatrix(&inverse);

            {
                VECTOR sdk_input = {input.x, input.y, input.z, 0};
                VECTOR sdk_output;
                ApplyRotMatrixLV(&sdk_input, &sdk_output);
                pos_track[side].x = sdk_output.vx;
                pos_track[side].y = sdk_output.vy;
                pos_track[side].z = sdk_output.vz;
            }
            PopMatrix();
        }
        else
        {
            dst->x = object->pos.x;
            dst->y = object->pos.y;
            dst->z = object->pos.z;
        }
    }
    if (velocity.y < 0 && pos_track[1].y < pos_track[0].y && pos_track[0].x - (uint32)pos_track[1].x + 6144u < 0x3001 && pos_track[0].z - (uint32)pos_track[1].z + 6144u < 0x3001)
        result = -1;
    return result;
}

void AnimatedObject::UpdateGroundShadow()
{
    const PathDefinition *parent;

    FUNCTION_MARKER(0x8005D1B0, "SLUS_005.78");
    Vec3 floor;
    Vec3 n;
    Vec3 velocity;
    Vec3 input;
    uint32 extent0;
    uint32 extent1;
    uint32 floor_valid = 0;
    CollisionSphere sample_pos;
    SpriteRecord *shadow;
    SceneTrigger *trigger;
    const SceneTrigger *hit;
    const SceneSection *section;
    MATRIX *matrix;
    const PathDefinition *owner, *candidate_owner, *next_definition;
    uint32 frame, dir, side, cur_flags, val, x, y, z, a, b, c;
    const ScenePath *path, *first_seen, *second_seen, *next_path;
    const ScenePath *next;
    uint32 best;
    uint32 radius, sq, mag, result;
    const sint16 *n_axis;
    sint32 kind;
    shadow = spr;
    if (shadow->extent[0] == 0 || shadow->extent[1] == 0)
        goto hide;
    cur_flags = path_flags;
    if ((cur_flags & 0x11) != 0 || (cur_flags & 0x300) == 0x100)
        goto hide;
    cur_flags = flags;
    shadow->pos.z = 0;
    shadow->pos.y = 0;
    shadow->pos.x = 0;
    owner = path_definition;
    side = 0xFFFFFFFF;
    frame = (uint16)path_frame;
    dir = cur_flags & 0x10;
    if (owner == 0)
        goto no_path;
    if ((owner->flags & 8) != 0 && ((const ActorBinding *)(((const ActorPlacement *)owner)->state))->rotation_track != 0)
        goto hide;
    if (owner->pos_track.keys == 0)
        goto no_path;
    cur_flags = flags;
    hit = 0;
    if ((cur_flags & 0x2400) == 0)
        goto orient;
    best = 0x7FFFF000;
    if ((cur_flags & 0x2000) != 0)
    {
        sample_pos.pos = pos;
        goto search_objects;
    }
    first_seen = 0;
    second_seen = 0;
    parent = path_definition;
    if (parent != 0 && parent->transform != 0)
    {
        input = pos;
        PushMatrix();
        matrix = path_definition->transform != 0 ? &path_definition->transform->matrix : NULL;
        SetRotMatrix(matrix);
        {
            VECTOR rotation_input = {input.x, input.y, input.z, 0};
            VECTOR rotation_output;
            ApplyRotMatrixLV(&rotation_input, &rotation_output);
            sample_pos.pos.x = rotation_output.vx;
            sample_pos.pos.y = rotation_output.vy;
            sample_pos.pos.z = rotation_output.vz;
        }
        PopMatrix();
        matrix = path_definition->transform != 0 ? &path_definition->transform->matrix : NULL;
        val = sample_pos.pos.x;
        sample_pos.pos.x = val + matrix->t[0];
        matrix = path_definition->transform != 0 ? &path_definition->transform->matrix : NULL;
        val = sample_pos.pos.y;
        sample_pos.pos.y = val + matrix->t[1];
        matrix = path_definition->transform != 0 ? &path_definition->transform->matrix : NULL;
        val = sample_pos.pos.z;
        sample_pos.pos.z = val + matrix->t[2];
    }
    else
    {
        sample_pos.pos = pos;
    }

    path = camera_motion.path;
    if (path != 0)
    {
        for (;;)
        {
            cur_flags = path->definition->flags;
            if ((cur_flags & 0x51) < 0x41)
                break;
            if ((cur_flags & 0x10) == 0)
            {
                if ((path->node.state_flags & (dir | 8)) != 0)
                    path = path->adjacent[1];
                else
                    path = path->adjacent[0];
                if (first_seen == 0)
                    first_seen = path;
                else if (second_seen == 0)
                    second_seen = path;
                else if (path == first_seen || path == second_seen)
                    path = 0;
            }
            else
            {
                next_path = path->fallback;
                if (next_path == 0)
                    path = 0;
                else if ((path->node.state_flags & (dir | 8)) != 0)
                    path = next_path;
                else
                {
                    val = next_path->node.state_flags;
                    path = next_path;
                    if ((val & 8) != 0)
                        path = next_path->adjacent[0];
                }
            }
            if (path == 0)
                break;
            next_path = camera_motion.path;
            candidate_owner = path->definition;
            next_definition = next_path->definition;
            if (candidate_owner == next_definition)
                path = 0;
            if (path == 0)
                break;
        }
    }
    if (path != camera_motion.path && path != 0)
    {
        frame = path->definition->FindHeightFrame(sample_pos.pos, dir, -40960);
        if ((sint16)frame >= 0)
            owner = path->definition;
    }
    frame = owner->ProjectFrame((sint16)frame, sample_pos.pos, floor, dir, 4096);
    floor_valid = 1;
    if ((sint16)frame < 0)
        owner = path_definition;
    else
    {
        val = sample_pos.pos.y;
        val -= (uint32)floor.y;
        best = (sint32)val / 50;
    }
search_objects:
    val = (uint32)sample_pos.pos.y;
    section = scene_stream.active_section;
    sample_pos.radius = 341;
    sample_pos.pos.y = val - 341;
    for (trigger = (SceneTrigger *)section->object_heads[4]; trigger != 0; trigger = (SceneTrigger *)trigger->node.links.registry_next)
    {
        if ((trigger->node.flags & 0x81) != 0)
            continue;
        kind = (sint8)(trigger->node.state_flags >> 8);
        if (kind != 6 && kind != 9)
            continue;
        radius = trigger->radius << 5;
        x = sample_pos.pos.x;
        a = trigger->transform.t[0];
        radius += 20480;
        val = x - a + radius;
        extent0 = radius;
        if (val > (radius << 1))
            continue;
        val = sample_pos.pos.z;
        val = val - trigger->transform.t[2] + radius;
        if (val > (radius << 1))
            continue;
        val = sample_pos.pos.y;
        val = val - trigger->transform.t[1] + 204800;
        if (val > 409600)
            continue;
        velocity.z = 0;
        velocity.x = 0;
        velocity.y = -204800;
        val = AnimatedObject::SweepSphereAgainstObject(&sample_pos, &trigger->node, velocity, &extent0, 0);
        if (val != 0)
        {
            radius = extent0;
            if ((sint32)radius < (sint32)best)
            {
                best = radius;
                hit = trigger;
                side = val;
            }
        }
    }
    section = scene_stream.active_section;
    for (trigger = (SceneTrigger *)section->object_heads[6]; trigger != 0; trigger = (SceneTrigger *)trigger->node.links.registry_next)
    {
        if ((trigger->node.flags & 0x81) != 0)
            continue;
        next = trigger->parameters.path_pair.targets[0];
        if (next == 0)
            continue;
        candidate_owner = next->definition;
        if (candidate_owner == owner || (candidate_owner->flags & 0x11) != 0)
            continue;
        radius = trigger->radius << 5;
        x = sample_pos.pos.x;
        a = trigger->transform.t[0];
        radius += 20480;
        val = x - a + radius;
        extent1 = radius;
        if (val > (radius << 1))
            continue;
        val = sample_pos.pos.z;
        val = val - trigger->transform.t[2] + radius;
        if (val > (radius << 1))
            continue;
        val = sample_pos.pos.y;
        val = val - trigger->transform.t[1] + 204800;
        if (val > 409600)
            continue;
        velocity.z = 0;
        velocity.x = 0;
        velocity.y = -204800;
        if (AnimatedObject::SweepSphereAgainstObject(&sample_pos, &trigger->node, velocity, &extent1, 0) != 0)
        {
            radius = extent1;
            if ((sint32)radius < (sint32)best)
            {
                side = 0xFFFFFFFF;
                best = radius;
                hit = trigger;
                if (dir != 0)
                    next = hit->parameters.path_pair.targets[0];
                else
                    next = hit->parameters.path_pair.targets[1];
                owner = next->definition;
            }
        }
    }
    if (hit != 0)
    {
        frame = owner->FindHeightFrame(sample_pos.pos, dir, -40960);
        floor_valid = 0;
        if ((sint16)frame >= 0)
        {
            frame = owner->ProjectFrame((sint16)frame, sample_pos.pos, floor, dir, 4096);
            floor_valid = 1;
        }
    }
    else if ((flags & 0x2000) != 0 || (owner->flags & 0x11) != 0)
        goto hide;
    if (hit != 0)
    {
        kind = (sint8)(hit->node.state_flags >> 8);
        if (kind != 1 && (uint8)(kind - 24) >= 2)
        {
            shadow->pos.y = -(best * 50);
            --side;
            n_axis = hit->transform.m[(sint32)side >> 1];
            val = n_axis[0];
            n.x = (side & 1) != 0 ? val : -val;
            val = n_axis[1];
            n.y = (side & 1) != 0 ? val : -val;
            val = n_axis[2];
            n.z = (side & 1) != 0 ? val : -val;
            goto orient;
        }
    }
    if (floor_valid == 0)
        goto hide;
    val = floor.y;
    shadow->pos.y = val - sample_pos.pos.y;
    goto orient;
no_path:
    owner = 0;
    shadow->angles[0] = 0;
orient:
    if ((sint32)side >= 0)
    {
        val = AsinAngle(n.y, 4096);
        shadow->angles[0] = (1024 - val) & 0xFFF;
    }
    else if (owner != 0)
    {
        if ((sint16)frame > 0)
            --frame;
        owner->SamplePos((sint16)frame, velocity, dir);
        do
        {
            ++frame;
            owner->SamplePos((sint16)frame, floor, dir);
            x = floor.x;
            a = velocity.x;
            y = floor.y;
            b = velocity.y;
            z = floor.z;
            c = velocity.z;
            x -= a;
            y -= b;
            z -= c;
            n.x = x;
            n.y = y;
            n.z = z;
            x = (sint32)x >> 6;
            y = (sint32)y >> 6;
            z = (sint32)z >> 6;
            sq = x * x + y * y + z * z;
            if ((sint32)sq < 0)
                mag = SquareRoot12((sq >> 2)) << 1;
            else
                mag = SquareRoot12(sq);
        } while ((sint32)mag < 1024 && (sint16)frame < owner->pos_track.frame_count - 1);
        val = AsinAngle(n.y, mag);
        shadow->angles[0] = val & 0xFFF;
    }
    shadow->angles[1] = target_yaw;
    if (dir == 0)
    {
        val = shadow->angles[0];
        cur_flags = shadow->angles[1];
        shadow->angles[0] = (-val) & 0xFFF;
        shadow->angles[1] = (cur_flags + 2048) & 0xFFF;
    }
    result = shadow->node.flags & 0xFE;
    goto finish_model;
hide:
    result = shadow->node.flags | 1;
finish_model:
    shadow->node.flags = result;
}

intptr AnimatedObject::UpdateActorStationaryShots()
{
    FUNCTION_MARKER(0x8005DE30, "SLUS_005.78");
    ActorModelDefinition *resource_model;

    ActorBinding *binding = (ActorBinding *)(((const ActorPlacement *)path_definition)->state);

    resource_model = binding->model;
    Vec3 origin;
    Vec3 target_pos;
    uint32 distance_output;
    uint32 result;
    UpdateActorStateAndTarget(origin, target_pos, &distance_output);
    if ((binding->flags & 2) == 0 && (path_definition->flags & 0x40) != 0)
    {
        uint32 facing = TurnObjectFacing(velocity, resource_model->turn_step);
        uint32 timer = state_delay;
        if (timer != 0)
            state_delay = --timer;
        if (timer == 0 && (facing & 255) != 0 && (flags & 0x30000000) == 0 && behavior != 17)
        {
            AnimatedObject *spawned;
            state = 30;
            flags = flags | 0x100;
            spawned = object_operations.behaviors[6].factory_handler(this);
            flags = flags & 0xFFFFFEFF;
            if (spawned != 0)
            {
                uint32 cur_flags = spawned->flags;
                spawned->velocity.z = 0;
                spawned->velocity.y = 0;
                spawned->velocity.x = 0;
                spawned->flags = cur_flags & 0xFFFFFBFF;
                spawned->rotation = rotation;
                spawned->animation_binding.tracks[0] = -2;
            }
            state_delay = resource_model->shot_interval;
        }
    }
    result = UpdateActorFacingAndAnimation(0, origin, resource_model);

    return result;
}

intptr AnimatedObject::InitializeActorStationaryShots()
{
    FUNCTION_MARKER(0x8005DFB8, "SLUS_005.78");
    intptr result;
    const PathDefinition *definition;
    if (!object_operations.behaviors[15].plain_handler && !object_operations.behaviors[15].member_handler && !object_operations.behaviors[15].void_member_handler && !object_operations.behaviors[15].factory_handler)
    {
        ObjectCallback operation = {0, &AnimatedObject::UpdateActorStationaryShots};
        object_operations.behaviors[15] = operation;
    }
    definition = path_definition;
    behavior = 15;
    result = (intptr)definition->pos_track.keys;
    if (result != 0)
    {
        result = flags | 0x40;
        flags = result;
    }
    state_delay = 0;
    return result;
}

intptr AnimatedObject::UpdateActorProximityAttack()
{
    FUNCTION_MARKER(0x8005E010, "SLUS_005.78");
    ActorBinding *state;
    ActorModelDefinition *resource_model;
    ActorBinding *binding;
    uint32 timer, result;
    AnimationHeader *clip;
    Vec3 origin;
    Vec3 target_pos;
    uint32 distance_output;
    Vec3 *target = &target_pos;
    sint32 sample_frame;
    UpdateActorStateAndTarget(origin, *target, &distance_output);
    state = (ActorBinding *)(((const ActorPlacement *)path_definition)->state);
    binding = state;
    resource_model = binding->model;
    if ((binding->flags & 2) != 0)
    {
        target = 0;
        goto finish;
    }
    if ((binding->flags & 8) != 0)
        goto finish;
    if ((binding->reaction_flags & 8) != 0)
        state_delay = 30;
    if ((path_definition->flags & 0x40) == 0)
    {
        if (animation != resource_model->animations[0])
        {
            flags = flags & 0xFBFFFFFF;
            {
                AnimationHeader *animation_val = resource_model->animations[0];
                AnimatedObject *animation_object = this;
                uint32 animation_flags = animation_object->flags;
                animation_object->animation = animation_val;
                animation_object->frame = -1;
                animation_object->flags = animation_flags | 0x10000;
            }
        }
        goto finish;
    }
    timer = state_delay;
    if (timer != 0)
    {
        state_delay = --timer;
        if (timer != 0)
        {
            distance_output = 0x7FFFF000;
            if (animation != resource_model->animations[2])
            {
                uint32 count;
                flags = (flags & 0xF7FFFFFF) | 0x40;
                {
                    AnimationHeader *animation_val = resource_model->animations[2];
                    AnimatedObject *animation_object = this;
                    uint32 animation_flags = animation_object->flags;
                    animation_object->animation = animation_val;
                    animation_object->frame = -1;
                    animation_object->flags = animation_flags | 0x10000;
                }
                count = (uint16)path_definition->pos_track.frame_count;
                if (((sint32)(count << 16) >> 17) < path_frame)
                {
                    uint32 sample_path_frame = (uint16)path_frame;
                    uint32 path_count = (uint16)path_definition->pos_track.frame_count;
                    AnimatedObject *path_object = this;
                    path_object->flags = (path_object->flags & 0xFFFFF7FF) ^ 0x10;
                    path_frame = path_count - sample_path_frame;
                }
            }
        }
    }
    clip = animation;
    sample_frame = frame;
    if (clip == resource_model->animations[3] && ((clip != 0 && sample_frame < clip->frame_count - 1) || sample_frame < 0))
    {
        target = 0;
        goto finish;
    }
    flags = flags & 0xFBFFFFFF;
    if (resource_model->target_range[0] >= (sint32)distance_output && target->y - (uint32)world_pos.y + 12288 < 24577)
    {
        sint16 angle = 64;
        if (resource_model->target_range[1] >= (sint32)distance_output)
            angle = (uint16)yaw - Atan2Angle((origin.z - (uint32)target->z), (origin.x - (uint32)target->x));
        if (angle >= -63 && angle < 64)
        {
            flags = flags | 0x4000000;
            if (animation != resource_model->animations[3])
            {
                flags = flags & 0xFFFFFFBF;
                {
                    AnimationHeader *animation_val = resource_model->animations[3];
                    AnimatedObject *animation_object = this;
                    uint32 animation_flags = animation_object->flags;
                    animation_object->animation = animation_val;
                    animation_object->frame = -1;
                    animation_object->flags = animation_flags | 0x10000;
                }
            }
        }
        else if (animation != resource_model->animations[2])
        {
            flags = flags | 0x8000040;
            {
                AnimationHeader *animation_val = resource_model->animations[2];
                AnimatedObject *animation_object = this;
                uint32 animation_flags = animation_object->flags;
                animation_object->animation = animation_val;
                animation_object->frame = -1;
                animation_object->flags = animation_flags | 0x10000;
            }
        }
        if (ClassifyActorPathDir(origin, *target) < 0)
        {
            uint32 sample_path_frame0 = (uint16)path_frame;
            uint32 path_count = (uint16)path_definition->pos_track.frame_count;
            AnimatedObject *path_object = this;
            path_object->flags = (path_object->flags & 0xFFFFF7FF) ^ 0x10;
            path_frame = path_count - sample_path_frame0;
        }
    }
    else
    {
        const PathDefinition *definition = path_definition;
        const sint16 *limit = ((ActorBinding *)(((const ActorPlacement *)definition)->state))->path_parameters + ((flags & 0x10) != 0 ? 0 : 1);
        if (path_frame >= *limit)
        {
            uint32 cur_flags = flags;
            path_frame = (uint16)definition->pos_track.frame_count - path_frame;
            flags = (cur_flags & 0xFFFFF7FF) ^ 0x10;
        }
        if (state_delay == 0 && animation != resource_model->animations[0] && (binding->flags & 8) == 0)
        {
            flags = flags | 0x40;
            {
                AnimationHeader *animation_val = resource_model->animations[0];
                AnimatedObject *animation_object = this;
                uint32 animation_flags = animation_object->flags;
                animation_object->animation = animation_val;
                animation_object->frame = -1;
                animation_object->flags = animation_flags | 0x10000;
            }
        }
    }
finish:
    result = UpdateActorFacingAndAnimation(target, origin, resource_model);

    return result;
}

intptr AnimatedObject::InitializeActorProximityAttack()
{
    FUNCTION_MARKER(0x8005E418, "SLUS_005.78");
    intptr result;
    const PathDefinition *definition;
    if (!object_operations.behaviors[10].plain_handler && !object_operations.behaviors[10].member_handler && !object_operations.behaviors[10].void_member_handler && !object_operations.behaviors[10].factory_handler)
    {
        ObjectCallback operation = {0, &AnimatedObject::UpdateActorProximityAttack};
        object_operations.behaviors[10] = operation;
    }
    definition = path_definition;
    behavior = 10;
    result = (intptr)definition->pos_track.keys;
    if (result != 0)
    {
        result = flags | 0x40;
        flags = result;
    }
    return result;
}

intptr AnimatedObject::UpdateActorRandomPatrol()
{
    FUNCTION_MARKER(0x8005F154, "SLUS_005.78");
    ActorBinding *state;
    ActorModelDefinition *resource_model;
    ActorBinding *binding;
    uint32 event, result;
    AnimationHeader *clip;
    const PathDefinition *definition;
    Vec3 origin;
    Vec3 target_pos;
    uint32 distance_output;
    Vec3 *target = &target_pos;
    sint32 sample_frame, dir;
    event = UpdateActorStateAndTarget(origin, *target, &distance_output);
    definition = path_definition;
    state = (ActorBinding *)(((const ActorPlacement *)definition)->state);
    binding = state;
    resource_model = binding->model;
    if ((binding->flags & 2) != 0)
        target = 0;
    else
    {
        if ((definition->flags & 0x40) == 0)
        {
            flags = flags & 0xFFFFFFBF;
            if (animation != resource_model->animations[0])
            {
                AnimationHeader *animation_val = resource_model->animations[0];
                AnimatedObject *animation_object = this;
                uint32 animation_flags = animation_object->flags;
                animation_object->animation = animation_val;
                animation_object->frame = -1;
                animation_object->flags = animation_flags | 0x10000;
            }
        }
        else
        {
            if ((event & 0x400000) != 0 && animation != resource_model->animations[2])
            {
                frame = -1;
                animation = resource_model->animations[2];
                flags = (flags | 0x10000) & 0xFFFFFFBF;
            }
            else if (animation == resource_model->animations[2])
            {
                clip = animation;
                sample_frame = frame;
                if ((clip == 0 || sample_frame >= clip->frame_count - 1) && sample_frame > 0)
                {
                    clip = resource_model->animations[((random_generator.Next() & 1) != 0 ? 0 : 3)];
                    {
                        AnimationHeader *animation_val = clip;
                        AnimatedObject *animation_object = this;
                        uint32 animation_flags = animation_object->flags;
                        animation_object->animation = animation_val;
                        animation_object->frame = -1;
                        animation_object->flags = animation_flags | 0x10000;
                    }
                    state_delay = 1;
                }
            }
            else
            {
                uint32 timer = state_delay - 1;
                state_delay = timer;
                if (timer == 0)
                {
                    state_delay = random_generator.Next() % 75 + 30;
                    if (animation == resource_model->animations[3])
                    {
                        {
                            AnimationHeader *animation_val = resource_model->animations[0];
                            AnimatedObject *animation_object = this;
                            uint32 animation_flags = animation_object->flags;
                            animation_object->animation = animation_val;
                            animation_object->frame = -1;
                            animation_object->flags = animation_flags | 0x10000;
                        }
                        flags = flags & 0xFFFFFFBF;
                    }
                    else
                    {
                        {
                            AnimationHeader *animation_val = resource_model->animations[3];
                            AnimatedObject *animation_object = this;
                            uint32 animation_flags = animation_object->flags;
                            animation_object->animation = animation_val;
                            animation_object->frame = -1;
                            animation_object->flags = animation_flags | 0x10000;
                        }
                        flags = flags | 0x40;
                    }
                }
                if (resource_model->target_range[0] >= (sint32)distance_output)
                {
                    dir = ClassifyActorPathDir(origin, *target);
                    if (dir < 0)
                    {
                        uint32 sample_path_frame = (uint16)path_frame;
                        uint32 path_count = (uint16)path_definition->pos_track.frame_count;
                        AnimatedObject *path_object = this;
                        path_object->flags = (path_object->flags & 0xFFFFF7FF) ^ 0x10;
                        path_frame = path_count - sample_path_frame;
                    }
                    else if (dir == 0 && animation == resource_model->animations[3])
                    {
                        flags = flags & 0xFFFFFFBF;
                        {
                            AnimationHeader *animation_val = resource_model->animations[0];
                            AnimatedObject *animation_object = this;
                            uint32 animation_flags = animation_object->flags;
                            animation_object->animation = animation_val;
                            animation_object->frame = -1;
                            animation_object->flags = animation_flags | 0x10000;
                        }
                    }
                }
            }
        }
        if (resource_model->target_range[0] < (sint32)distance_output)
            target = 0;
    }
    result = UpdateActorFacingAndAnimation(target, origin, resource_model);

    return result;
}

intptr AnimatedObject::InitializeActorRandomPatrol()
{
    FUNCTION_MARKER(0x8005F490, "SLUS_005.78");
    uint32 result;
    if (!object_operations.behaviors[12].plain_handler && !object_operations.behaviors[12].member_handler && !object_operations.behaviors[12].void_member_handler && !object_operations.behaviors[12].factory_handler)
    {
        ObjectCallback operation = {0, &AnimatedObject::UpdateActorRandomPatrol};
        object_operations.behaviors[12] = operation;
    }
    behavior = 12;
    result = random_generator.Next() % 75 + 15;
    state_delay = result;
    return result;
}

intptr AnimatedObject::UpdateActorPathDetour()
{
    uint32 dx, dy;

    FUNCTION_MARKER(0x8005F510, "SLUS_005.78");
    ActorModelDefinition *resource_model;

    ActorBinding *binding = (ActorBinding *)(((const ActorPlacement *)path_definition)->state);

    resource_model = binding->model;
    uint32 result, sum;
    Vec3 origin;
    Vec3 target_pos;
    uint32 distance_output;
    Vec3 *target = &target_pos;
    Vec3 sel;
    sint32 frame, cur, next, difference, velocity;
    UpdateActorStateAndTarget(origin, *target, &distance_output);
    if (binding->action_idx >= 0 || binding->route_dir_mode != 0)
    {
        result = UpdateActorFacingAndAnimation(target, origin, resource_model);

        return result;
    }
    if ((path_definition->flags & 0x40) == 0)
    {
        uint32 packed = detour_target;
        if ((packed & 0x8000) != 0)
        {
            sel.x = (uint32)((sint32)packed >> 16) << 12;
            sel.y = (uint32)binding->path_parameters[0] << 12;
            sel.z = (uint32)binding->path_parameters[1] << 12;
            frame = packed & 0x7FFF;
        }
        else
        {
            sel.x = target->x;
            sel.y = target->y;
            sel.z = target->z;
            frame = path_definition->FindHeightFrame(sel, 1, 4096000);
            packed = ((uint32)((sel.x >> 12) - 3 + (sint32)(random_generator.Next() & 7)) << 16) | frame | 0x8000;
            detour_target = packed;
            binding->path_parameters[0] = (sel.y >> 12) + (random_generator.Next() & 7);
            binding->path_parameters[1] = (sel.z >> 12) + (random_generator.Next() & 7) - 3;
        }
        if ((flags & 0x10) == 0)
            frame = path_definition->pos_track.frame_count - frame - 1;
        sum = 0;
        dx = (sint32)(origin.x - (uint32)sel.x) >> 12;
        dy = (sint32)(origin.y - (uint32)sel.y) >> 12;
        difference = (sint32)(origin.z - (uint32)sel.z) >> 12;
        sum += dx * dx + dy * dy + (uint32)difference * difference;
        if (resource_model->target_range[0] < (sint32)sum)
            goto stop_motion;
        cur = path_frame;
        if (frame < cur)
        {
            if (cur < path_definition->pos_track.frame_count - 1)
                flags = flags | 0x40;
            else
            {
                path_frame = 0;
                flags = (flags ^ 0x10) & 0xFFFFFFBF;
            }
        }
        else if (cur == 0)
            flags = flags & 0xFFFFFFBF;
        else
        {
            flags = ((flags & 0xFFFFF7FF) ^ 0x10) | 0x40;
            path_frame = (uint16)path_definition->pos_track.frame_count - cur;
        }
        flags = flags | 0x8000000;
        if (animation != resource_model->animations[3])
        {
            AnimationHeader *animation_val = resource_model->animations[3];
            AnimatedObject *animation_object = this;
            uint32 animation_flags = animation_object->flags;
            animation_object->animation = animation_val;
            animation_object->frame = -1;
            animation_object->flags = animation_flags | 0x10000;
        }
        goto finish_motion;
    }
    if ((detour_target & 0x8000) != 0)
    {
        detour_target = ((random_generator.Next() & 7) - 3) << 16;
        binding->path_parameters[0] = random_generator.Next() & 3;
        random_generator.Next();
        binding->path_parameters[1] = 0;
        sel.x = (uint32)(sint16)(detour_target >> 16) << 12;
        sel.y = (uint32)binding->path_parameters[0] << 12;
        sel.z = (uint32)binding->path_parameters[1] << 12;
        target->x = target->x + (uint32)sel.x;
        target->y = target->y + (uint32)sel.y;
        target->z = target->z + (uint32)sel.z;
    }
    if ((sint32)distance_output >= 10000)
        goto stop_motion;
    next = ClassifyActorPathDir(origin, *target);
    flags = (flags & 0xF7FFFFFF) | 0x40;
    cur = path_frame;
    next += cur;
    if (animation != resource_model->animations[2])
    {
        AnimationHeader *animation_val = resource_model->animations[2];
        AnimatedObject *animation_object = this;
        uint32 animation_flags = animation_object->flags;
        animation_object->animation = animation_val;
        animation_object->frame = -1;
        animation_object->flags = animation_flags | 0x10000;
    }
    cur = path_frame;
    if (next < cur)
    {
        if (cur == 0)
            flags = flags & 0xFFFFFFBF;
        else if ((binding->reaction_flags & 0x80) == 0)
        {
            flags = ((flags & 0xFFFFF7FF) ^ 0x10) | 0x40;
            path_frame = (uint16)path_definition->pos_track.frame_count - cur;
        }
    }
    else if (cur < path_definition->pos_track.frame_count - 1)
        flags = flags | 0x40;
    else
    {
        path_frame = 0;
        flags = (flags ^ 0x10) & 0xFFFFFFBF;
    }
    difference = target->y - (uint32)origin.y;
    velocity = difference;
    if (difference != 0)
    {
        velocity = ((ActorBinding *)(((const ActorPlacement *)path_definition)->state))->path_pitch_speed + (difference < 0 ? -204 : 204);
        if (velocity < -4096)
            velocity = -4096;
        if (velocity > 4096)
            velocity = 4096;
    }
    if ((flags & 0x4000) != 0 && velocity < 0)
        velocity = 0;
    ((ActorBinding *)(((const ActorPlacement *)path_definition)->state))->path_pitch_speed = velocity;
    goto finish_motion;
stop_motion:
    flags = flags & 0xFFFFFFBF;
finish_motion:
    UpdateActorFacingAndAnimation(target, origin, resource_model);
    flags = flags & 0xFFFFBFFF;
    result = (camera_proj.rotation[1] + 2048) & 4095;
    orientation[2] = result;

    return result;
}

intptr AnimatedObject::InitializeActorPathDetour()
{
    FUNCTION_MARKER(0x8005FADC, "SLUS_005.78");
    uint32 result;
    const PathDefinition *definition;
    if (!object_operations.behaviors[24].plain_handler && !object_operations.behaviors[24].member_handler && !object_operations.behaviors[24].void_member_handler && !object_operations.behaviors[24].factory_handler)
    {
        ObjectCallback operation = {0, &AnimatedObject::UpdateActorPathDetour};
        object_operations.behaviors[24] = operation;
    }
    definition = path_definition;
    behavior = 24;
    result = definition->pos_track.keys != 0;
    if (result != 0)
    {
        result = flags | 0x400;
        flags = result;
    }
    void *state = ((const ActorPlacement *)path_definition)->state;
    ActorBinding *binding = (ActorBinding *)state;
    binding->path_pitch_speed = 0;
    return (intptr)state;
}

intptr AnimatedObject::UpdateActorVerticalPursuit()
{
    FUNCTION_MARKER(0x80060360, "SLUS_005.78");
    ActorModelDefinition *resource_model;
    ActorBinding *binding;
    ActorBinding *state = (ActorBinding *)(((const ActorPlacement *)path_definition)->state);
    binding = state;
    resource_model = binding->model;
    uint32 result;
    Vec3 origin;
    Vec3 target_pos;
    uint32 distance_output;
    Vec3 *target = &target_pos;
    sint32 frame, prev, difference, velocity, angle;
    UpdateActorStateAndTarget(origin, *target, &distance_output);
    if ((binding->flags & 2) != 0)
    {
        result = UpdateActorFacingAndAnimation(0, origin, resource_model);

        return result;
    }
    target->y = (uint32)target->y + 14336;
    if ((path_definition->flags & 0x40) != 0)
    {
        flags = flags | 0x40;
        if ((sint32)distance_output < 10001)
            frame = path_frame + ClassifyActorPathDir(origin, *target);
        else
            frame = path_definition->pos_track.frame_count;
        prev = path_frame;
        if (frame < prev)
        {
            uint32 count = (uint16)path_definition->pos_track.frame_count;
            flags = (flags & 0xFFFFF7FF) ^ 0x10;
            path_frame = count - prev;
        }
        difference = target->y - (uint32)origin.y;
        if ((uint32)difference + 2048 < 4096)
            difference = 0;
        state = (ActorBinding *)(((const ActorPlacement *)path_definition)->state);
        binding = state;
        velocity = binding->path_pitch_speed;
        angle = pitch;
        if (difference != 0)
        {
            if ((angle == 0 && difference >= 0) || (angle > 1024 && difference >= 0))
            {
                velocity += 409;
                if (velocity > 2048)
                    velocity = 2048;
            }
            else if (angle > 1024 && difference < 0)
            {
                velocity -= 409;
                if (velocity < 0)
                    velocity = 0;
            }
            else if (difference <= 0)
            {
                velocity -= 409;
                if (velocity < -2048)
                    velocity = -2048;
            }
            else
            {
                velocity += 409;
                if (velocity > 0)
                    velocity = 0;
            }
        }
        else if (velocity > 0)
        {
            velocity -= 409;
            if (velocity < 0)
                velocity = 0;
        }
        else if (velocity < 0)
        {
            velocity += 409;
            if (velocity > 0)
                velocity = 0;
        }
        if ((flags & 0x4000) != 0 && velocity < 0)
            velocity = 0;
        ((ActorBinding *)(((const ActorPlacement *)path_definition)->state))->path_pitch_speed = velocity;
    }
    else if ((flags & 0x40) != 0)
    {
        flags = flags & 0xFFFFFFBF;
        ((ActorBinding *)(((const ActorPlacement *)path_definition)->state))->path_pitch_speed = 0;
    }
    UpdateActorFacingAndAnimation(target, origin, resource_model);
    result = flags & 0xFFFFBFFF;
    flags = result;

    return result;
}

intptr AnimatedObject::InitializeActorVerticalPursuit()
{
    FUNCTION_MARKER(0x80060688, "SLUS_005.78");
    uint32 result;
    const PathDefinition *definition;
    if (!object_operations.behaviors[23].plain_handler && !object_operations.behaviors[23].member_handler && !object_operations.behaviors[23].void_member_handler && !object_operations.behaviors[23].factory_handler)
    {
        ObjectCallback operation = {0, &AnimatedObject::UpdateActorVerticalPursuit};
        object_operations.behaviors[23] = operation;
    }
    definition = path_definition;
    behavior = 23;
    result = definition->pos_track.keys != 0;
    if (result != 0)
    {
        result = flags | 0x440;
        flags = result;
    }
    void *state = ((const ActorPlacement *)path_definition)->state;
    ActorBinding *binding = (ActorBinding *)state;
    binding->path_pitch_speed = 0;
    pitch_scale = 0;
    return (intptr)state;
}

sint32 AnimatedObject::ResolveObjectSpeed() const
{
    FUNCTION_MARKER(0x800606FC, "SLUS_005.78");
    const ObjectSpeedDefinition *definition = ObjectSpeedDefinitions::FindObjectSpeedDefinition(state);
    const SceneNode *child_node = node.links.first_child;
    sint32 result = 0;
    while (child_node != 0)
    {
        if (child_node->kind == 0)
        {
            const AnimatedObject *child_object = (const AnimatedObject *)child_node;
            if ((child_object->path_flags & 0x2000) != 0)
            {
                sint32 pos = child_object->velocity.x;
                if (pos != 0)
                {
                    result = pos;
                    break;
                }
                pos = child_object->ResolveObjectSpeed();
                if (pos != child_object->velocity.x)
                {
                    result = pos;
                    break;
                }
            }
        }
        child_node = child_node->links.sibling_next;
    }
    return result != 0 ? result : definition->default_speed;
}

intptr AnimatedObject::UpdateAnimatedObjectCommands()
{
    SceneNode *scene_node;
    FUNCTION_MARKER(0x800607BC, "SLUS_005.78");
    const ObjectSpeedDefinition *definition;
    AnimatedObject *child;
    uint32 *commands[3] = {&event_word, &event_command, &state};
    uint32 cur_flags, val, idx, command, result;
    sint32 sel, step;
    cur_flags = flags;
    val = path_flags;
    flags = cur_flags & 0xFC7FFFFF;
    if ((val & 0x2000) != 0)
        WrapObjectSpeed();
    cur_flags = flags;
    if ((cur_flags & 0x100) != 0)
    {
        flags = cur_flags & 0xFFFFFEFF;
        for (idx = 0; idx < 3; ++idx)
        {
            val = *commands[idx];
            if ((val & 0xFFF) == 0)
                continue;
            command = (val >> 12) & 31;
            switch (command)
            {
                case 1:
                    cur_flags = flags;
                    if ((cur_flags & 0x240000) != 0x40000)
                    {
                        if ((cur_flags & 0x440000) != 0)
                            flags = cur_flags & 0xFFBFFFFF;
                        else
                            flags = cur_flags | 0x1020000;
                        flags = (flags & 0xFFDFFFFF) | 0x40000;
                    }
                    if ((path_flags & 0x2000) == 0)
                        break;
                    definition = ObjectSpeedDefinitions::FindObjectSpeedDefinition(state);
                    sel = 0;
                    scene_node = node.links.first_child;
                    while (scene_node != 0)
                    {
                        const SceneNode *child_node = scene_node;
                        if (child_node->kind == 0)
                        {
                            child = (AnimatedObject *)scene_node;
                            if ((child->path_flags & 0x2000) != 0)
                            {
                                sint32 child_speed = child->velocity.x;
                                if (child_speed != 0)
                                {
                                    sel = child_speed;
                                    break;
                                }
                                child_speed = child->ResolveObjectSpeed();
                                if (child_speed != child->velocity.x)
                                {
                                    sel = child_speed;
                                    break;
                                }
                            }
                        }
                        scene_node = child_node->links.sibling_next;
                    }
                    if (sel == 0)
                        sel = definition->default_speed;
                    velocity.x = sel;
                    break;
                case 2:
                    if ((path_flags & 0x2000) != 0)
                        ApplyObjectSpeedAction(41);
                    else
                    {
                        cur_flags = flags;
                        if ((cur_flags & 0x240000) != 0x240000)
                        {
                            val = (cur_flags & 0x440000) != 0 ? cur_flags & 0xFFBFFFFF : cur_flags | 0x2010000;
                            flags = val;
                            flags = flags | 0x240000;
                        }
                    }
                    break;
                case 3:
                case 14:
                    goto hide_object;
                case 4:
                case 15:
                    goto show_object;
                case 5:
                    cur_flags = flags;
                    if ((cur_flags & 0x40000) != 0)
                        flags = (cur_flags & 0xFFFBFFFF) | 0x400000;
                    break;
                case 6:
                    if ((path_flags & 0x2000) != 0)
                        ApplyObjectSpeedAction(45);
                    else
                        flags = (flags | 0x40000) & 0xFFBFFFFF;
                    break;
                case 7:
                    if ((path_flags & 0x2000) != 0)
                        ApplyObjectSpeedAction(46);
                    else
                        flags = (flags & 0xFFDFFFFF) | 0x800000;
                    break;
                case 8:
                    flags = (flags | 0x8000) & 0xFC1BFFFF;
                    goto process_mode;
                case 9:
                    if ((path_flags & 0x2000) != 0)
                        ApplyObjectSpeedAction(48);
                    else
                        flags = flags | 0xA00000;
                    break;
                case 10:
                process_mode:
                    if ((path_flags & 0x2000) == 0)
                        goto finish_commands;
                    ApplyObjectSpeedAction(49);
                    break;
                case 12:
                    if ((node.flags & 1) != 0)
                        goto show_object;
                hide_object:
                    cur_flags = flags;
                    val = node.flags;
                    flags = cur_flags | 0x4000;
                    node.flags = val | 1;
                    break;
                show_object:
                    cur_flags = flags;
                    val = node.flags;
                    flags = cur_flags | 0x80000;
                    node.flags = val & 0xFE;
                    break;
                case 13:
                    cur_flags = flags;
                    if ((cur_flags & 0x40000) != 0)
                        flags = (cur_flags & 0xFFFBFFFF) | 0x400000;
                    else
                        flags = (cur_flags | 0x40000) & 0xFFBFFFFF;
                    break;
            }
        }
        cur_flags = flags;
    }
    path_frame = 0;
    if ((cur_flags & 0x840000) != 0 && (node.flags & 1) == 0)
    {
        step = (cur_flags & 0x200000) != 0 ? -1 : 1;
        path_frame = step;
        AdvanceTracks(step);
    }
finish_commands:
    result = SceneEvents::ApplySceneEvents(commands, 3) << 24;
    if (result != 0)
    {
        result = flags | 0x100;
        flags = result;
    }
    return result;
}

intptr AnimatedObject::UpdateSceneScriptVariable()
{
    FUNCTION_MARKER(0x80060C0C, "SLUS_005.78");
    uint32 *commands[1] = {&event_word};
    uint32 val, cur_flags, idx, byte, condition;
    sint32 mode, threshold, step;
    if (event_word != 0)
    {
        if ((SceneEvents::ApplySceneEvents(commands, 1) << 24) != 0)
        {
            if (script_mode != 0)
            {
                idx = ticks;
                if (idx == 98)
                    val = game_flow.progress.unlocked_stage;
                else if (idx == 99)
                    val = scene_events.world_name_idx;
                else
                    val = scene_events.script_vals[idx];
                val += state;
                if ((sint32)val < 0)
                    val = 0;
                else if ((sint32)val >= 100)
                    val = 99;
            }
            else
                val = state;
            idx = ticks;
            if (idx == 98)
                game_flow.progress.unlocked_stage = val;
            else if (idx == 99)
                scene_events.world_name_idx = val;
            else
                scene_events.script_vals[idx] = val;
        }
    }
    else
    {
        mode = script_mode;
        idx = ticks;
        if (idx == 98)
            val = game_flow.progress.unlocked_stage;
        else if (idx == 99)
            val = scene_events.world_name_idx;
        else
            val = scene_events.script_vals[idx];
        threshold = state;
        if (mode == 0)
            condition = val == threshold;
        else if (mode < 0)
            condition = (sint32)val < threshold;
        else
            condition = threshold < (sint32)val;
        if (condition != 0)
        {
            if ((node.flags & 1) != 0)
            {
                cur_flags = flags;
                byte = node.flags;
                flags = cur_flags | 0x80000;
                node.flags = byte & 0xFE;
            }
        }
        else if ((node.flags & 1) == 0)
        {
            cur_flags = flags;
            byte = node.flags;
            flags = cur_flags | 0x4000;
            node.flags = byte | 1;
        }
    }
    cur_flags = flags;
    val = cur_flags & 0x840000;
    if (val == 0)
        return val;
    if ((node.flags & 1) != 0)
        return 0x200000;
    step = (cur_flags & 0x200000) != 0 ? -1 : 1;
    path_frame = step;
    return AdvanceTracks(step);
}

void AnimatedObject::EmptyObjectBehavior(void)
{
    FUNCTION_MARKER(0x80061174, "SLUS_005.78");
}

intptr AnimatedObject::UpdateSceneAnimation()
{
    FUNCTION_MARKER(0x8006117C, "SLUS_005.78");
    uint32 cur_flags, result;
    sint32 step;
    cur_flags = flags;
    result = cur_flags & 0x840000;
    if (result == 0)
        return result;
    if ((node.flags & 1) != 0)
        return 0x200000;
    step = (cur_flags & 0x200000) != 0 ? -1 : 1;
    path_frame = step;
    return AdvanceTracks(step);
}

intptr AnimatedObject::FollowSceneAnimation()
{
    FUNCTION_MARKER(0x800611E8, "SLUS_005.78");
    AnimatedObject *parent = linked_object;
    uint32 result = parent->path_frame;
    uint32 cur_flags, parent_flags, frame;
    if (result == 0)
        return result;
    result = 0x40000;
    if ((node.flags & 1) != 0)
        return result;
    cur_flags = flags | 0x40000;
    flags = cur_flags;
    parent_flags = parent->flags;
    if ((parent_flags & 0x1000000) != 0)
        flags = cur_flags | 0x20000;
    else if ((parent_flags & 0x2000000) != 0)
        flags = cur_flags | 0x10000;
    frame = (uint16)parent->path_frame;
    path_frame = frame;
    AdvanceTracks((sint16)frame);
    parent_flags = parent->flags;
    result = parent_flags & 0x840000;
    if (result == 0)
    {
        cur_flags = flags;
        result = parent_flags | 0x40000;
        if ((cur_flags & 0x40000) != 0)
            parent->flags = result;
    }
    return result;
}

void AnimatedObject::RelocateResourceMaterials(const MaterialCatalog *resource, const ModelDecodeLayout *layout, MeshFace **polygon_owner, MeshVertex **vertex_owner)
{
    const uint8 *input = layout->input;

    uint8 *object = (uint8 *)this;
    SceneNode *child_node;
    SpriteRecord *spr_val;
    uint32 *pointer;
    uint32 offset;
    uint32 remaining;
    uint32 idx;
    uint32 kind;
    uint32 val;
    FUNCTION_MARKER(0x80068570, "SLUS_005.78");
    hd_invalidate_geometry();
    free(*polygon_owner);
    *polygon_owner = 0;
    free(*vertex_owner);
    *vertex_owner = 0;
    EncodedAnimatedObject encoded;
    memcpy(&encoded, input, sizeof(encoded));
    const EncodedAnimatedObject *root = &encoded;
    node.kind = root->node.kind;
    node.flags = root->node.flags;
    node.state_flags = root->node.state_flags;
    flags = root->flags;
    face_count = root->geometry_counts;
    vertex_count = root->geometry_counts >> 16;
    event_word = root->event_word;
    behavior_data = root->behavior_data;
    state = root->state_word;
    animation_binding.word = root->animation_binding.word;
    path_motion_scale = root->motion_scale;
    clone_bytes = root->clone_bytes;
    clone_storage_bytes = layout->storage_bytes;
    path_flags = root->path_flags;
    lighting_idx = root->lighting_idx;
    resource_halfword58 = root->resource_halfword58;
    sound_voice = root->sound_voice;
    mesh_span.first_vertex = root->mesh_span.first_vertex;
    mesh_span.vertex_count = root->mesh_span.vertex_count;
    mesh_span.first_polygon = root->mesh_span.first_polygon;
    mesh_span.polygon_count = root->mesh_span.polygon_count;
    ticks = root->ticks;
    frame = root->frame;
    path_frame = root->path_frame;
    yaw = root->yaw;
    target_yaw = root->target_yaw;
    pitch = root->pitch;
    pitch_scale = root->pitch_scale;
    pos.x = root->pos.x;
    pos.y = root->pos.y;
    pos.z = root->pos.z;
    world_pos.x = root->world_pos.x;
    world_pos.y = root->world_pos.y;
    world_pos.z = root->world_pos.z;
    bounds.maximum.x = root->bounds.maximum.x;
    bounds.maximum.y = root->bounds.maximum.y;
    bounds.maximum.z = root->bounds.maximum.z;
    velocity.x = root->velocity.x;
    velocity.y = root->velocity.y;
    velocity.z = root->velocity.z;
    model_contacts.blocker = 0;
    model_contacts.surface0 = 0;
    model_contacts.surface1 = 0;
    model_contacts.surface2 = 0;
    model_contacts.fraction0 = 0;
    model_contacts.fraction1 = 0;
    model_contacts.fraction2 = 0;
    const CollisionSphere *src_spheres = root->spheres;
    for (uint32 sphere = 0; sphere < 4; ++sphere)
        spheres[sphere] = src_spheres[sphere];
    orientation[0] = root->rotation.w;
    orientation[1] = root->rotation.x;
    orientation[2] = root->rotation.y;
    orientation[3] = root->rotation.z;
    movement_mode = root->movement_mode;
    behavior = root->behavior;
    contact_kind = root->contact_kind;
    behavior_suspended = root->behavior_suspended;
    unbound_animation_word = root->animation_offset;
    animation = 0;

    path_definition = 0;
    mesh_state = 0;
    vertices = 0;
    polygons = 0;
    material_animation_entries = 0;
    node.links.first_child = 0;
    node.links.sibling_next = 0;
    spr = 0;
    offset = root->vertices_offset;
    if (offset != 0)
    {
        sint32 vertex_count_val = vertex_count;
        if (vertex_count_val > 0)
        {
            uint32 count = vertex_count_val;
            *vertex_owner = (MeshVertex *)malloc(count * sizeof(MeshVertex));
            vertices = *vertex_owner;
            for (uint32 vertex_idx = 0; vertex_idx < count; ++vertex_idx)
            {
                const uint8 *vertex_input = input + offset + vertex_idx * sizeof(EncodedMeshVertex);
                MeshVertex *vertex = *vertex_owner + vertex_idx;
                vertex->pos.vx = xport_load_le16(vertex_input);
                vertex->pos.vy = xport_load_le16(vertex_input + 2);
                vertex->pos.vz = xport_load_le16(vertex_input + 4);
                vertex->pos.pad = xport_load_le16(vertex_input + 6);
                vertex->n.vx = xport_load_le16(vertex_input + 8);
                vertex->n.vy = xport_load_le16(vertex_input + 10);
                vertex->n.vz = xport_load_le16(vertex_input + 12);
                vertex->n.pad = xport_load_le16(vertex_input + 14);
            }
        }
        else
        {
            vertices = (MeshVertex *)(layout->legacy_data + offset);
        }
    }
    offset = root->polygon_word;
    if (offset != 0)
    {
        if ((flags & 12) == 8)
        {
            polygons = (MeshFace *)(layout->legacy_data + offset);
            PrintSdkDiagnostic("Share\n");
        }
        else
        {
            remaining = face_count;
            *polygon_owner = (MeshFace *)malloc(remaining * sizeof(MeshFace));
            polygons = *polygon_owner;
            MeshFace *face = *polygon_owner;
            const uint8 *face_input = input + offset;
            do
            {
                idx = ((const EncodedMeshFace *)face_input)->tex_word;
                --remaining;
                face->tex = resource->texs + idx;
                const EncodedMeshFace *src = (const EncodedMeshFace *)face_input;
                for (uint32 vertex = 0; vertex < 4; ++vertex)
                    face->vertex_idxs[vertex] = src->vertex_idxs[vertex];
                face->plane = src->plane;
                face->n = src->n;
                ++face;
                face_input += sizeof(EncodedMeshFace);
            } while (remaining != 0);
        }
    }
    node.links.src_registry_word = root->node.registry_offset;
    offset = root->node.child_offset;
    if (offset != 0)
    {
        uint32 mapping_idx = 0;
        while (mapping_idx < layout->rec_count && !(layout->recs[mapping_idx].src_offset == offset && layout->recs[mapping_idx].kind < ModelStorageRoot))
            ++mapping_idx;
        node.links.first_child = (SceneNode *)(layout->storage + layout->recs[mapping_idx].storage_offset);
    }
    offset = root->node.sibling_offset;
    if (offset != 0)
    {
        uint32 mapping_idx = 0;
        while (mapping_idx < layout->rec_count && !(layout->recs[mapping_idx].src_offset == offset && layout->recs[mapping_idx].kind < ModelStorageRoot))
            ++mapping_idx;
        node.links.sibling_next = (SceneNode *)(layout->storage + layout->recs[mapping_idx].storage_offset);
    }
    offset = root->sprite_offset;
    if (offset != 0)
    {
        uint32 mapping_idx = 0;
        while (mapping_idx < layout->rec_count && !(layout->recs[mapping_idx].src_offset == offset && layout->recs[mapping_idx].kind == 7))
            ++mapping_idx;
        spr = (SpriteRecord *)(layout->storage + layout->recs[mapping_idx].storage_offset);
    }
    child_node = node.links.first_child;
    while (child_node != 0)
    {
        child_node->RelocateResourceLinks(layout, resource, spr);
        child_node = child_node->links.sibling_next;
    }
    spr_val = spr;
    if (spr_val != 0)
        spr_val->tex = resource_grps[0].tex;
    offset = root->materials_offset;
    if (offset != 0)
    {
        uint32 mapping_idx = 0;
        while (mapping_idx < layout->rec_count && !(layout->recs[mapping_idx].src_offset == offset - 4 && layout->recs[mapping_idx].kind == ModelStorageMaterialAnimations))
            ++mapping_idx;
        pointer = (uint32 *)(layout->storage + layout->recs[mapping_idx].storage_offset) + 1;
        material_animation_entries = pointer;
        remaining = (uint8)((const MaterialAnimationListHeader *)pointer - 1)->count;
        if (remaining != 0)
        {
            do
            {
                uint32 *entry = pointer;
                val = *entry;
                if (val != 0xFFFFFFFF)
                {
                    idx = (val >> 1) & 0x1FF;
                    kind = resource->materials[idx].kind;
                    idx += material_registry.count;
                    if (kind == 0)
                        *entry = 2 * idx;
                    else if (kind == 2)
                        *entry = 2 * idx | (val & 0x3FC00) | (val & 0x3FC0000) | (val & 0x3C000000);
                }
                ++pointer;
                --remaining;
            } while (remaining != 0);
        }
    }
}

void AnimatedObject::DecodeModelResource(AnimatedObject **object_slot, uint8 **model_owner, const MaterialCatalog *resource, uint32 bytes, MeshFace **polygon_owner, MeshVertex **vertex_owner)
{
    FUNCTION_MARKER(0x80069F64, "SLUS_005.78");
    uint8 *input = *model_owner;
    const EncodedAnimatedObject *root = (const EncodedAnimatedObject *)input;
    uint32 clone_bytes = root->clone_bytes;
    uint32 capacity = clone_bytes / 4 + 1;
    ModelStorageRecord *recs = (ModelStorageRecord *)malloc(capacity * sizeof(ModelStorageRecord));
    uint32 *pending = (uint32 *)malloc(capacity * sizeof(uint32));
    recs[0].src_offset = 0;
    recs[0].src_bytes = 232;
    recs[0].storage_offset = 0;
    recs[0].storage_bytes = sizeof(AnimatedObject);
    recs[0].kind = ModelStorageRoot;
    uint32 rec_count = 1;
    uint32 pending_count = 1;
    uint32 pending_idx = 0;
    pending[0] = root->node.child_offset;
    while (pending_idx < pending_count)
    {
        uint32 offset = pending[pending_idx++];
        if (offset == 0)
            continue;
        uint32 found = 0;
        for (uint32 idx = 0; idx < rec_count; ++idx)
        {
            if (recs[idx].src_offset == offset)
            {
                found = 1;
                break;
            }
        }
        if (found != 0)
            continue;
        const EncodedSceneNode *encoded_node = (const EncodedSceneNode *)(input + offset);
        uint32 kind = encoded_node->kind;
        uint32 src_bytes = kind == 5 || kind == 7 ? 48 : (kind == 6 ? 36 : 16);
        uint32 storage_bytes = kind == 5 ? sizeof(MeshPartNode) : (kind == 6 ? sizeof(SphereNode) : (kind == 7 ? sizeof(SpriteRecord) : sizeof(SceneNode)));
        recs[rec_count].src_offset = offset;
        recs[rec_count].src_bytes = src_bytes;
        recs[rec_count].storage_offset = 0;
        recs[rec_count].storage_bytes = storage_bytes;
        recs[rec_count].kind = kind;
        ++rec_count;
        pending[pending_count++] = encoded_node->child_offset;
        pending[pending_count++] = encoded_node->sibling_offset;
    }
    uint32 node_count = rec_count;
    for (uint32 node_idx = 0; node_idx < node_count; ++node_idx)
    {
        const ModelStorageRecord *node = recs + node_idx;
        uint32 offset = node->kind == ModelStorageRoot ? root->materials_offset : (node->kind == 7 ? ((const EncodedSpriteRecord *)(input + node->src_offset))->animation_offset : 0);
        if (offset == 0)
            continue;
        uint32 count = ((const MaterialAnimationListHeader *)(input + offset) - 1)->count & 255;
        uint32 list_bytes = (count + 1) * sizeof(uint32);
        uint32 found = 0;
        for (uint32 idx = node_count; idx < rec_count; ++idx)
        {
            if (recs[idx].src_offset == offset - 4)
            {
                found = 1;
                break;
            }
        }
        if (found != 0)
            continue;
        recs[rec_count].src_offset = offset - 4;
        recs[rec_count].src_bytes = list_bytes;
        recs[rec_count].storage_offset = 0;
        recs[rec_count].storage_bytes = list_bytes;
        recs[rec_count].kind = ModelStorageMaterialAnimations;
        ++rec_count;
    }
    for (uint32 idx = 1; idx < rec_count; ++idx)
    {
        ModelStorageRecord rec = recs[idx];
        uint32 insert = idx;
        while (insert != 0 && recs[insert - 1].src_offset > rec.src_offset)
        {
            recs[insert] = recs[insert - 1];
            --insert;
        }
        recs[insert] = rec;
    }
    uint32 src_cursor = 0;
    uint32 rec_idx = 0;
    while (rec_idx < rec_count)
    {
        if (recs[rec_idx].src_offset > src_cursor)
        {
            uint32 gap = recs[rec_idx].src_offset - src_cursor;
            memmove(recs + rec_idx + 1, recs + rec_idx, (rec_count - rec_idx) * sizeof(ModelStorageRecord));
            recs[rec_idx].src_offset = src_cursor;
            recs[rec_idx].src_bytes = gap;
            recs[rec_idx].storage_offset = 0;
            recs[rec_idx].storage_bytes = gap;
            recs[rec_idx].kind = ModelStorageBytes;
            ++rec_count;
        }
        src_cursor = recs[rec_idx].src_offset + recs[rec_idx].src_bytes;
        ++rec_idx;
    }
    if (src_cursor < clone_bytes)
    {
        recs[rec_count].src_offset = src_cursor;
        recs[rec_count].src_bytes = clone_bytes - src_cursor;
        recs[rec_count].storage_offset = 0;
        recs[rec_count].storage_bytes = clone_bytes - src_cursor;
        recs[rec_count].kind = ModelStorageBytes;
        ++rec_count;
    }
    uint32 storage_bytes = 0;
    for (uint32 idx = 0; idx < rec_count; ++idx)
    {
        storage_bytes = (storage_bytes + sizeof(void *) - 1u) & ~(sizeof(void *) - 1u);
        recs[idx].storage_offset = storage_bytes;
        storage_bytes += recs[idx].storage_bytes;
    }
    storage_bytes = (storage_bytes + sizeof(void *) - 1u) & ~(sizeof(void *) - 1u);
    uint32 root_flags = root->flags;
    uint32 packed_counts = root->geometry_counts;
    uint32 face_offset = root->polygon_word;
    uint32 vertex_offset = root->vertices_offset;
    uint32 legacy_bytes = (((root_flags & 12) == 8 && face_offset != 0) || ((sint16)(packed_counts >> 16) <= 0 && vertex_offset != 0)) ? bytes : 0;
    uint8 *storage = (uint8 *)calloc(storage_bytes + legacy_bytes, 1);
    for (uint32 idx = 0; idx < rec_count; ++idx)
    {
        const ModelStorageRecord *rec = recs + idx;
        if (rec->kind == ModelStorageMaterialAnimations)
        {
            memcpy(storage + rec->storage_offset, input + rec->src_offset, rec->src_bytes);
        }
        else if (rec->kind == ModelStorageBytes)
            memcpy(storage + rec->storage_offset, input + rec->src_offset, rec->src_bytes);
    }
    if (legacy_bytes != 0)
        memcpy(storage + storage_bytes, input, bytes);
    ModelDecodeLayout layout = {0};
    layout.input = input;
    layout.input_bytes = bytes;
    layout.src_clone_bytes = clone_bytes;
    layout.storage = storage;
    layout.storage_bytes = storage_bytes;
    layout.legacy_data = legacy_bytes != 0 ? storage + storage_bytes : 0;
    layout.recs = recs;
    layout.rec_count = rec_count;
    AnimatedObject *object = (AnimatedObject *)storage;
    object->RelocateResourceMaterials(resource, &layout, polygon_owner, vertex_owner);
    *object_slot = object;
    *model_owner = storage;
    free(input);
    free(pending);
    free(recs);
    uint32 count = resource->material_count;
    if ((sint32)count > 0)
    {
        MaterialRecord *dst = material_registry.recs + material_registry.count;
        for (uint32 idx = 0; idx < count; ++idx)
            dst[idx] = resource->materials[idx];
        material_registry.count = material_registry.count + resource->material_count;
    }
}

void AnimatedObject::AttachAnimation(uint8 *animation_data, uint32 bytes, AnimationHeader *header)
{
    FUNCTION_MARKER(0x8006A170, "SLUS_005.78");
    header->Relocate(animation_data, bytes, (const EncodedAnimationHeader *)animation_data);
    animation = header;
    unbound_animation_word = 0;
}
