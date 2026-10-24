#include "hd_internal.h"

static void hd_reset_sprite_track(const SceneNode *node, const AnimatedObject *owner, const AnimationTrack *track)
{
    for (; node != NULL; node = node->links.sibling_next)
    {
        // Nested actors own their animation bindings
        if (node->kind == 0)
            continue;
        if (node->kind == 7)
        {
            sint32 binding = ((const SpriteRecord *)node)->animation_binding.tracks[0];
            if (binding >= 0 && binding < owner->animation->counts[0] && &owner->animation->tracks[0].vectors[binding].track == track)
                hd_forget_pose(node, false);
        }
        hd_reset_sprite_track(node->links.first_child, owner, track);
    }
}

void hd_sprite_track_loop(const AnimatedObject *owner, const AnimationTrack *track)
{
    if (owner != NULL && owner->animation != NULL)
        hd_reset_sprite_track(owner->node.links.first_child, owner, track);
}

void hd_sprite_pool(const SpriteRecord *records)
{
    if (records != hd_state.sprite_pool)
    {
        if (hd_state.sprite_pool != NULL)
            for (uint32 idx = 0; idx < HD_POOL_SLOTS; ++idx)
                hd_forget_animation(&hd_state.sprite_pool[idx].node);
        hd_clear_samples();
    }
    hd_state.sprite_pool = records;
    memset(hd_state.pool_valid, 0, sizeof(hd_state.pool_valid));
    memset(hd_state.samples[0].pool_valid, 0, sizeof(hd_state.samples[0].pool_valid));
    memset(hd_state.samples[1].pool_valid, 0, sizeof(hd_state.samples[1].pool_valid));
}

static bool hd_sprite_pass(const HdCamera *camera, const VID_FOG *fog, uint32 first, uint32 blend, uint32 mask, float alpha, const HdTexture *texture, const HdLighting *lighting, const VECTORF *normal)
{
    HdWorldDraw source;
    memset(&source, 0, sizeof(source));
    source.pose = hd_state.draw_pose;
    source.first = first;
    source.count = 6;
    source.blend = blend;
    for (uint32 row = 0; row < 3; ++row)
    {
        memcpy(source.uniforms + row * 4, camera->view.m[row], sizeof(camera->view.m[row]));
        source.uniforms[row * 4 + 3] = camera->view.t[row];
    }
    source.uniforms[12] = camera->focal / hd_state.ui_half_width;
    source.uniforms[13] = camera->focal / 120.0f;
    source.uniforms[14] = (camera->far_z + camera->near_z) / (camera->far_z - camera->near_z);
    source.uniforms[15] = -2.0f * camera->far_z * camera->near_z / (camera->far_z - camera->near_z);
    source.uniforms[16] = alpha;
    source.uniforms[17] = mask;
    source.uniforms[18] = texture->left | (texture->top << 8);
    source.uniforms[19] = (texture->width - 1) | ((texture->height - 1) << 8);
    source.uniforms[19 * 4 + 1] = camera->near_z;
    if (lighting != NULL)
    {
        memcpy(source.uniforms + 5 * 4, normal, sizeof(*normal));
        source.uniforms[5 * 4 + 3] = 1.0f;
        for (uint32 idx = 0; idx < 3; ++idx)
        {
            const HdLight *light = lighting->lights + idx;
            memcpy(source.uniforms + (6 + idx) * 4, &light->dir, sizeof(light->dir));
            source.uniforms[(6 + idx) * 4 + 3] = light->point ? 1.0f : 0.0f;
            memcpy(source.uniforms + (9 + idx) * 4, &light->color, sizeof(light->color));
            source.uniforms[(9 + idx) * 4 + 3] = light->radius;
            memcpy(source.uniforms + (12 + idx) * 4, &light->pos, sizeof(light->pos));
            source.uniforms[(12 + idx) * 4 + 3] = light->attenuation_range;
            memcpy(source.uniforms + (15 + idx) * 4, &light->color_scale, sizeof(light->color_scale));
        }
        memcpy(source.uniforms + 18 * 4, &lighting->ambient, sizeof(lighting->ambient));
    }
    if (fog != NULL)
        source.fog = *fog;
    if (hd_state.world_count != 0)
    {
        HdWorldDraw *prev = hd_state.world_draws + hd_state.world_count - 1;
        if (source.pose == HD_NO_POSE && prev->mesh == NULL && prev->pose == source.pose && prev->first + prev->count == first && prev->blend == blend &&
            memcmp(prev->uniforms, source.uniforms, sizeof(source.uniforms)) == 0 && memcmp(&prev->fog, &source.fog, sizeof(source.fog)) == 0)
        {
            prev->count += 6;
            return true;
        }
    }
    HdWorldDraw *draw = hd_new_world_draw();
    if (draw == NULL)
        return false;
    *draw = source;
    return true;
}

bool hd_sprite(const HdSprite *sprite, const TextureDescriptor *texture, const HdCamera *camera, const VID_FOG *fog, const HdLighting *lighting, const VECTORF *normal)
{
    if (!hd_state.frame_active || sprite == NULL || texture == NULL || camera == NULL || camera->near_z <= 0 || camera->far_z <= camera->near_z ||
        sprite->mode < HD_BILLBOARD || sprite->mode > HD_FACING_SPRITE || hd_state.sprite_count > 0xFFFFFFFFu - 6 || (lighting != NULL && normal == NULL))
        return false;
    if (fog != NULL && !(fog->near_dist > 0 && fog->far_dist > fog->near_dist && fog->horizon_dist > fog->far_dist && fog->focal[0] > 0 && fog->focal[1] > 0))
        return false;
    HdTexture region;
    memset(&region, 0, sizeof(region));
    region.width = region.height = 256;
    if ((texture->flags & 4) != 0 && !hd_texture(texture, &region))
        return false;
    uint32 count = hd_state.sprite_count + 6;
    if (count > hd_state.sprite_capacity)
    {
        uint32 capacity = hd_state.sprite_capacity != 0 ? hd_state.sprite_capacity * 2 : 1536;
        if (capacity < count || capacity > (size_t)-1 / sizeof(VID_VERTEX2D))
            return false;
        VID_VERTEX2D *vertices = (VID_VERTEX2D *)realloc(hd_state.sprite_vertices, capacity * sizeof(VID_VERTEX2D));
        if (vertices == NULL)
            return false;
        hd_state.sprite_vertices = vertices;
        hd_state.sprite_capacity = capacity;
    }
    const TextureMapping *mapping = &texture->mapping;
    static const uint8 order[6] = {0, 1, 2, 2, 1, 3};
    static const uint8 corners[4][4] = {{0,1,2,3},{2,0,3,1},{3,2,1,0},{1,3,0,2}};
    for (uint32 idx = 0; idx < 6; ++idx)
    {
        uint32 corner = sprite->mode != HD_GROUND_SPRITE ? corners[sprite->orientation & 3][order[idx]] : order[idx];
        if (sprite->mode == HD_GROUND_SPRITE)
            corner = ((corner & 1) << 1) | ((corner & 2) >> 1);
        VID_VERTEX2D *vertex = hd_state.sprite_vertices + hd_state.sprite_count + idx;
        memset(vertex, 0, sizeof(*vertex));
        memcpy(vertex->pos, &sprite->pos, sizeof(sprite->pos));
        vertex->pos[3] = (corner & 1) != 0 ? sprite->width : -sprite->width;
        vertex->alpha = (corner & 2) != 0 ? sprite->height : -sprite->height;
        if (sprite->mode == HD_BILLBOARD)
            vertex->alpha = -vertex->alpha;
        vertex->uv[0] = (order[idx] & 1) != 0 ? mapping->right : mapping->left;
        vertex->uv[1] = (order[idx] & 2) != 0 ? mapping->bottom : mapping->top;
        memcpy(vertex->color, &mapping->color, sizeof(vertex->color));
        vertex->color[3] = texture->flags >> 24;
        vertex->tex[0] = mapping->tpage | ((texture->flags & 0x80) != 0 ? texture->flags & 0x60 : 0);
        vertex->tex[1] = mapping->clut;
        vertex->tex[2] = HD_TEXTURED | region.flags | ((mapping->color.code & 1) != 0 ? HD_RAW_TEXTURE : 0);
        vertex->tex[3] = sprite->pitch & 4095;
        vertex->rect[0] = sprite->yaw & 4095;
        vertex->rect[1] = sprite->mode;
    }
    uint32 first = hd_state.sprite_count;
    hd_state.sprite_count = count;
    if ((mapping->color.code & 2) == 0)
        return hd_sprite_pass(camera, fog, first, VID_BLEND_NONE, 0, 1.0f, &region, lighting, normal);
    uint32 mode = (hd_state.sprite_vertices[first].tex[0] >> 5) & 3;
    static const uint32 blends[4] = {VID_BLEND_ALPHA, VID_BLEND_ADD, VID_BLEND_SUBTRACT, VID_BLEND_ADD};
    static const float alpha[4] = {0.5f, 1.0f, 1.0f, 0.25f};
    return hd_sprite_pass(camera, fog, first, VID_BLEND_NONE, HD_OPAQUE_ONLY, 1.0f, &region, lighting, normal) &&
        hd_sprite_pass(camera, fog, first, blends[mode], HD_STP_ONLY, alpha[mode], &region, lighting, normal);
}

bool hd_sprite_record(const SpriteRecord *source, const MATRIXF *model, const HdCamera *camera, float scale, const TextureDescriptor *material, const VID_FOG *fog, const SceneLighting *lighting)
{
    HdSprite sprite;
    memset(&sprite, 0, sizeof(sprite));
    sprite.pos.vx = model->t[0];
    sprite.pos.vy = model->t[1];
    sprite.pos.vz = model->t[2];
    sprite.width = source->extent[0];
    sprite.height = source->extent[1];
    uint32 flags = source->node.state_flags;
    const TextureDescriptor *texture = source->tex;
    if ((flags & 0x200) != 0)
    {
        sprite.mode = HD_GROUND_SPRITE;
        sprite.pitch = source->angles[0];
        sprite.yaw = source->angles[1];
    }
    else
    {
        VECTORF offset = {(sint16)source->angles[0] * sprite.width / 128.0f, (sint16)source->angles[1] * sprite.height / 128.0f, 0.0f};
        MATRIXF inverse;
        TransposeMatrixf(&camera->view, &inverse);
        ApplyMatrixf(&inverse, &offset, &offset);
        sprite.pos.vx += offset.vx;
        sprite.pos.vy += offset.vy;
        sprite.pos.vz += offset.vz;
        sprite.orientation = (flags & 0x180) >> 7;
        if ((flags & 4) != 0)
        {
            sprite.mode = HD_FACING_SPRITE;
            sprite.height += 10.0f;
        }
        else
        {
            sprite.mode = HD_BILLBOARD;
            if (scale != 0.0f)
            {
                sprite.width *= scale;
                sprite.height *= scale;
            }
            if (material != NULL)
            {
                texture = material;
                sprite.orientation = 0;
            }
        }
    }
    if ((flags & 0x205) == 0)
    {
        if (lighting == NULL)
            return false;
        VECTORF origin = {model->t[0], model->t[1], model->t[2]};
        float radius = sprite.width > sprite.height ? sprite.width : sprite.height;
        SceneLighting selected;
        hd_select_lights(lighting, &origin, radius, &selected);
        HdLighting lights;
        hd_lighting(&selected, model, NULL, &lights);
        VECTORF normal = {-camera->view.m[2][0], -camera->view.m[2][1], -camera->view.m[2][2]};
        MATRIXF inverse;
        TransposeMatrixf(model, &inverse);
        ApplyMatrixf(&inverse, &normal, &normal);
        uint32 first = hd_state.world_count;
        bool result = hd_sprite(&sprite, texture, camera, fog, &lights, &normal);
        if (result)
            hd_draw_lights(first, &selected, model);
        return result;
    }
    return hd_sprite(&sprite, texture, camera, fog);
}

bool hd_billboard_node(const BillboardNode *source, const VECTORF *pos, const HdCamera *camera, const VID_FOG *fog)
{
    HdSprite sprite;
    memset(&sprite, 0, sizeof(sprite));
    sprite.pos = *pos;
    sprite.width = source->extent / 32.0f;
    sprite.height = sprite.width;
    sprite.mode = HD_BILLBOARD;
    return hd_sprite(&sprite, source->tex, camera, fog);
}

static const HdWorldDraw *hd_previous_sprite(const HdSnapshot *previous, const HdWorldDraw *draw, uint32 pose)
{
    const HdWorldDraw *match = NULL;
    for (uint32 idx = 0; idx < previous->world_count; ++idx)
    {
        const HdWorldDraw *candidate = previous->world_draws + idx;
        if (candidate->mesh == NULL && candidate->pose == pose && candidate->count == draw->count &&
            candidate->blend == draw->blend && candidate->uniforms[17] == draw->uniforms[17])
        {
            if (match != NULL)
                return NULL;
            match = candidate;
        }
    }
    return match;
}

bool hd_mix_sprites(const HdSnapshot *previous, const HdSnapshot *current, float fract, HdSnapshot *view)
{
    if (current->sprite_count == 0)
        return true;
    if (hd_state.mixed_sprite_capacity < current->sprite_count)
    {
        VID_VERTEX2D *vertices = (VID_VERTEX2D *)hd_grow_sample(hd_state.mixed_sprites, &hd_state.mixed_sprite_capacity, current->sprite_count, sizeof(VID_VERTEX2D));
        if (vertices == NULL)
            return false;
        hd_state.mixed_sprites = vertices;
    }
    memcpy(hd_state.mixed_sprites, current->sprite_vertices, current->sprite_count * sizeof(VID_VERTEX2D));
    for (uint32 idx = 0; idx < current->world_count; ++idx)
    {
        const HdWorldDraw *draw = current->world_draws + idx;
        if (draw->mesh != NULL || draw->pose >= current->node_count || draw->first > current->sprite_count || draw->count > current->sprite_count - draw->first)
            continue;
        const HdMixedPose *pose = hd_state.mixed_poses + draw->pose;
        const HdWorldDraw *old = pose->previous != HD_NO_POSE ? hd_previous_sprite(previous, draw, pose->previous) : NULL;
        if (old != NULL && (old->first > previous->sprite_count || old->count > previous->sprite_count - old->first))
            old = NULL;
        if (old != NULL && draw->count != 0)
        {
            const VID_VERTEX2D *src = current->sprite_vertices + draw->first;
            const VID_VERTEX2D *prev = previous->sprite_vertices + old->first;
            HdWorldDraw *mixed = view->world_draws + idx;
            if (prev->rect[1] == HD_GROUND_SPRITE && src->rect[1] == HD_GROUND_SPRITE)
            {
                // Fractional angular offsets preserve the compact source vertex layout
                mixed->uniforms[80] = -hd_angle_delta(prev->tex[3], src->tex[3]) * (1.0f - fract);
                mixed->uniforms[81] = -hd_angle_delta(prev->rect[0], src->rect[0]) * (1.0f - fract);
            }
            if (prev->rect[1] == src->rect[1] && prev->tex[2] == src->tex[2] && old->uniforms[23] == draw->uniforms[23])
            {
                mixed->uniforms[82] = (prev->color[0] - src->color[0]) * (1.0f - fract);
                mixed->uniforms[83] = (prev->color[1] - src->color[1]) * (1.0f - fract);
                mixed->uniforms[78] = (prev->color[2] - src->color[2]) * (1.0f - fract);
                mixed->uniforms[79] = (prev->color[3] - src->color[3]) * (1.0f - fract);
            }
        }
        // STP passes share vertices but retain their own interpolation uniforms
        if (idx != 0 && current->world_draws[idx - 1].mesh == NULL && current->world_draws[idx - 1].first == draw->first)
            continue;
        MATRIXF old_world;
        if (old != NULL && !hd_pose_world(previous, pose->previous, &old_world))
            return false;
        for (uint32 vertex = 0; vertex < draw->count; ++vertex)
        {
            VID_VERTEX2D *dst = hd_state.mixed_sprites + draw->first + vertex;
            const VID_VERTEX2D *src = current->sprite_vertices + draw->first + vertex;
            const VID_VERTEX2D *prev = old != NULL ? previous->sprite_vertices + old->first + vertex : NULL;
            for (uint32 axis = 0; axis < 3; ++axis)
            {
                float offset = src->pos[axis] - pose->source.t[axis];
                if (prev != NULL)
                {
                    float old_offset = prev->pos[axis] - old_world.t[axis];
                    offset = old_offset + (offset - old_offset) * fract;
                }
                dst->pos[axis] = pose->model.t[axis] + offset;
            }
            if (prev != NULL && memcmp(prev->tex, src->tex, 3 * sizeof(src->tex[0])) == 0 &&
                prev->rect[1] == src->rect[1] && memcmp(prev->uv, src->uv, sizeof(src->uv)) == 0)
            {
                dst->pos[3] = prev->pos[3] + (src->pos[3] - prev->pos[3]) * fract;
                dst->alpha = prev->alpha + (src->alpha - prev->alpha) * fract;
            }
        }
    }
    if (memcmp(hd_state.mixed_sprites, current->sprite_vertices, current->sprite_count * sizeof(VID_VERTEX2D)) != 0)
    {
        view->sprite_vertices = hd_state.mixed_sprites;
        view->sprite_capacity = hd_state.mixed_sprite_capacity;
        view->sprite_content_version = ++hd_state.next_content_version;
    }
    return true;
}

void hd_frame_sprite(const SpriteRecord *sprite, float scale, const TextureDescriptor *material, const SceneLighting *lighting, const MATRIX *scratch, float depth_bias)
{
    if (!hd_frame.collecting || hd_frame.failed)
        return;
    bool native_pose = hd_state.pose_active && hd_state.poses[hd_state.pose_depth].node == &sprite->node;
    if (!native_pose)
    {
        hd_frame.failed = true;
        return;
    }
    MATRIXF model = hd_state.poses[hd_state.pose_depth].model;
    hd_state.draw_pose = hd_state.poses[hd_state.pose_depth].sample;
    uint32 first = hd_state.world_count;
    VID_FOG source_fog;
    const VID_FOG *fog = hd_source_fog(hd_state.scene, &hd_frame.camera, &source_fog);
    if (!hd_frame.camera_valid || !hd_sprite_record(sprite, &model, &hd_frame.camera, scale, material, fog, lighting))
        hd_frame.failed = true;
    for (uint32 idx = first; idx < hd_state.world_count; ++idx)
        hd_state.world_draws[idx].uniforms[76] = depth_bias;
    hd_state.draw_pose = HD_NO_POSE;
    if (!native_pose || hd_frame.failed)
        return;
    uint32 depth = hd_state.pose_depth + 1;
    if (!hd_reserve_poses(depth))
    {
        hd_frame.failed = true;
        return;
    }
    HdPose *pose = hd_state.poses + depth;
    if (!pose->valid)
        hd_source_model(scratch, &pose->model);
    VECTORF pos = {model.t[0], model.t[1], model.t[2]};
    RotTransf(&hd_frame.camera.view, &pos, &pos);
    TransMatrixf(&pose->model, &pos);
    if ((sprite->node.state_flags & 0x200) != 0)
    {
        MATRIXF rotation;
        IdentityMatrixf(&rotation);
        RotMatrixXf((sprite->angles[0] & 4095) * RAD12, &rotation);
        RotMatrixYf((sprite->angles[1] & 4095) * RAD12, &rotation);
        MulMatrix0f(&hd_frame.camera.view, &rotation, &pose->model);
    }
    else
    {
        pose->model.t[0] += (sint16)sprite->angles[0] * sprite->extent[0] / 128.0f;
        pose->model.t[1] += (sint16)sprite->angles[1] * sprite->extent[1] / 128.0f;
        if ((sprite->node.state_flags & 4) != 0)
        {
            MATRIXF inverse, rotation;
            TransposeMatrixf(&hd_frame.camera.view, &inverse);
            pos.vx = pose->model.t[0];
            pos.vy = pose->model.t[1];
            pos.vz = pose->model.t[2];
            ApplyMatrixf(&inverse, &pos, &pos);
            IdentityMatrixf(&rotation);
            RotMatrixYf(atan2f(pos.vz, pos.vx), &rotation);
            MulMatrix0f(&hd_frame.camera.view, &rotation, &pose->model);
        }
    }
    pose->valid = true;
}

void hd_frame_billboard(const BillboardNode *billboard, const MATRIX *view, float depth_bias)
{
    if (!hd_frame.collecting || hd_frame.failed)
        return;
    if (!hd_frame.camera_valid)
    {
        hd_frame.failed = true;
        return;
    }
    bool native_pose = hd_state.pose_active && hd_state.poses[hd_state.pose_depth].node == &billboard->node;
    VECTORF pos;
    if (native_pose)
    {
        const MATRIXF *model = &hd_state.poses[hd_state.pose_depth].model;
        pos.vx = model->t[0];
        pos.vy = model->t[1];
        pos.vz = model->t[2];
    }
    else
    {
        pos.vx = view->t[0] / 32.0f - hd_frame.camera.view.t[0];
        pos.vy = view->t[1] / 32.0f - hd_frame.camera.view.t[1];
        pos.vz = view->t[2] / 32.0f - hd_frame.camera.view.t[2];
        MATRIXF inverse;
        TransposeMatrixf(&hd_frame.camera.view, &inverse);
        ApplyMatrixf(&inverse, &pos, &pos);
    }
    hd_state.draw_pose = native_pose ? hd_state.poses[hd_state.pose_depth].sample : HD_NO_POSE;
    uint32 first = hd_state.world_count;
    VID_FOG source_fog;
    const VID_FOG *fog = hd_source_fog(hd_state.scene, &hd_frame.camera, &source_fog);
    if (!hd_frame.camera_valid || !hd_billboard_node(billboard, &pos, &hd_frame.camera, fog))
        hd_frame.failed = true;
    for (uint32 idx = first; idx < hd_state.world_count; ++idx)
        hd_state.world_draws[idx].uniforms[76] = depth_bias;
    hd_state.draw_pose = HD_NO_POSE;
    if (native_pose && !hd_frame.failed)
    {
        RotTransf(&hd_frame.camera.view, &pos, &pos);
        TransMatrixf(&hd_state.poses[hd_state.pose_depth].model, &pos);
    }
}
