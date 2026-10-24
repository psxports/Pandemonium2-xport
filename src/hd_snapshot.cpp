#include "hd_internal.h"

void hd_sample_clear(HdSnapshot *sample)
{
    hd_og_release(sample->og_draws, sample->og_count);
    sample->og_count = 0;
    for (uint32 idx = 0; idx < sample->world_count; ++idx)
        if (sample->world_draws[idx].mesh != NULL)
            --sample->world_draws[idx].mesh->sample_refs;
    sample->world_count = 0;
    sample->ui_count = 0;
    sample->sprite_count = 0;
    sample->pass_count = 0;
    hd_animation_nodes_release(sample->nodes, sample->node_count);
    sample->node_count = 0;
    sample->valid = false;
    memset(sample->pool_valid, 0, sizeof(sample->pool_valid));
}

void hd_clear_samples(void)
{
    hd_clear_identities();
    hd_sample_clear(hd_state.samples);
    hd_sample_clear(hd_state.samples + 1);
    for (uint32 idx = 0; idx < hd_state.pose_capacity; ++idx)
        hd_state.poses[idx].valid = false;
    hd_state.pose_owner = NULL;
    hd_state.pose_active = false;
    hd_state.graph_depth = 0;
    hd_state.path_count = 0;
    hd_free_retired();
}

void *hd_grow_sample(void *data, uint32 *capacity, uint32 count, size_t stride)
{
    if (count <= *capacity)
        return data;
    uint32 grown = *capacity != 0 ? *capacity : 64;
    while (grown < count && grown <= 0x7FFFFFFFu)
        grown *= 2;
    if (grown < count)
        grown = count;
    if (grown > (size_t)-1 / stride)
        return NULL;
    void *result = realloc(data, grown * stride);
    if (result != NULL)
        *capacity = grown;
    return result;
}

bool hd_capture_sample(HdSnapshot *sample)
{
    if (sample->og_capacity < hd_state.og_count)
    {
        HdOgDraw *quads = (HdOgDraw *)hd_grow_sample(sample->og_draws, &sample->og_capacity, hd_state.og_count, sizeof(HdOgDraw));
        if (quads == NULL)
            return false;
        sample->og_draws = quads;
    }
    if (sample->node_capacity < hd_state.node_count)
    {
        HdPoseSample *nodes = (HdPoseSample *)hd_grow_sample(sample->nodes, &sample->node_capacity, hd_state.node_count, sizeof(HdPoseSample));
        if (nodes == NULL)
            return false;
        sample->nodes = nodes;
    }
    if (sample->world_capacity < hd_state.world_count)
    {
        HdWorldDraw *draws = (HdWorldDraw *)hd_grow_sample(sample->world_draws, &sample->world_capacity, hd_state.world_count, sizeof(HdWorldDraw));
        if (draws == NULL)
            return false;
        sample->world_draws = draws;
    }
    if (sample->ui_capacity < hd_state.ui_count)
    {
        VID_VERTEX2D *vertices = (VID_VERTEX2D *)hd_grow_sample(sample->ui_vertices, &sample->ui_capacity, hd_state.ui_count, sizeof(VID_VERTEX2D));
        if (vertices == NULL)
            return false;
        sample->ui_vertices = vertices;
    }
    if (sample->sprite_capacity < hd_state.sprite_count)
    {
        VID_VERTEX2D *vertices = (VID_VERTEX2D *)hd_grow_sample(sample->sprite_vertices, &sample->sprite_capacity, hd_state.sprite_count, sizeof(VID_VERTEX2D));
        if (vertices == NULL)
            return false;
        sample->sprite_vertices = vertices;
    }
    if (sample->pass_capacity < hd_state.pass_count)
    {
        HdUiPass *passes = (HdUiPass *)hd_grow_sample(sample->ui_passes, &sample->pass_capacity, hd_state.pass_count, sizeof(HdUiPass));
        if (passes == NULL)
            return false;
        sample->ui_passes = passes;
    }
    hd_sample_clear(sample);
    sample->og_count = hd_state.og_count;
    if (sample->og_count != 0)
    {
        memcpy(sample->og_draws, hd_state.og_draws, sample->og_count * sizeof(HdOgDraw));
        hd_og_retain(sample->og_draws, sample->og_count);
    }
    memcpy(sample->pool_valid, hd_state.pool_valid, sizeof(sample->pool_valid));
    sample->node_count = hd_state.node_count;
    if (sample->node_count != 0)
    {
        memcpy(sample->nodes, hd_state.nodes, sample->node_count * sizeof(HdPoseSample));
        for (uint32 idx = 0; idx < sample->node_count; ++idx)
        {
            hd_animation_retain(&sample->nodes[idx].position_channel);
            hd_animation_retain(&sample->nodes[idx].rotation_channel);
        }
    }
    sample->camera = hd_frame.camera;
    sample->camera_valid = hd_frame.camera_valid;
    sample->scene = hd_state.scene;
    sample->epoch = hd_state.pose_epoch;
    sample->camera_epoch = hd_state.camera_sample_epoch;
    sample->collection = hd_state.collection;
    sample->clock_index = hd_state.clock_index;
    sample->world_count = hd_state.world_count;
    sample->ui_count = hd_state.ui_count;
    sample->sprite_count = hd_state.sprite_count;
    sample->pass_count = hd_state.pass_count;
    if (sample->world_count != 0)
        memcpy(sample->world_draws, hd_state.world_draws, sample->world_count * sizeof(HdWorldDraw));
    if (sample->ui_count != 0)
        memcpy(sample->ui_vertices, hd_state.ui_vertices, sample->ui_count * sizeof(VID_VERTEX2D));
    if (sample->sprite_count != 0)
        memcpy(sample->sprite_vertices, hd_state.sprite_vertices, sample->sprite_count * sizeof(VID_VERTEX2D));
    if (sample->pass_count != 0)
        memcpy(sample->ui_passes, hd_state.ui_passes, sample->pass_count * sizeof(HdUiPass));
    for (uint32 idx = 0; idx < sample->world_count; ++idx)
        if (sample->world_draws[idx].mesh != NULL)
            ++sample->world_draws[idx].mesh->sample_refs;
    sample->clear_color = hd_state.clear_color;
    sample->vram_version = gpu_vram_version();
    sample->image_version = hd_state.image_version;
    sample->content_version = hd_state.content_version;
    sample->sprite_content_version = hd_state.sprite_content_version;
    sample->ui_half_width = hd_state.ui_half_width;
    sample->restore_image = hd_state.restore_image;
    sample->backdrop = hd_state.backdrop;
    return true;
}

static void hd_work_view(HdSnapshot *view)
{
    view->og_draws = hd_state.og_draws;
    view->og_count = hd_state.og_count;
    view->og_capacity = hd_state.og_capacity;
    view->world_draws = hd_state.world_draws;
    view->world_count = hd_state.world_count;
    view->world_capacity = hd_state.world_capacity;
    view->ui_vertices = hd_state.ui_vertices;
    view->ui_count = hd_state.ui_count;
    view->ui_capacity = hd_state.ui_capacity;
    view->sprite_vertices = hd_state.sprite_vertices;
    view->sprite_count = hd_state.sprite_count;
    view->sprite_capacity = hd_state.sprite_capacity;
    view->ui_passes = hd_state.ui_passes;
    view->pass_count = hd_state.pass_count;
    view->pass_capacity = hd_state.pass_capacity;
    view->clear_color = hd_state.clear_color;
    view->content_version = hd_state.content_version;
    view->sprite_content_version = hd_state.sprite_content_version;
    view->ui_half_width = hd_state.ui_half_width;
    view->restore_image = hd_state.restore_image;
    view->backdrop = hd_state.backdrop;
}

static void hd_use_sample(const HdSnapshot *view)
{
    // OG sources remain owned by the collector while HD consumes a borrowed view
    hd_state.world_draws = view->world_draws;
    hd_state.world_count = view->world_count;
    hd_state.world_capacity = view->world_capacity;
    hd_state.ui_vertices = view->ui_vertices;
    hd_state.ui_count = view->ui_count;
    hd_state.ui_capacity = view->ui_capacity;
    hd_state.sprite_vertices = view->sprite_vertices;
    hd_state.sprite_count = view->sprite_count;
    hd_state.sprite_capacity = view->sprite_capacity;
    hd_state.ui_passes = view->ui_passes;
    hd_state.pass_count = view->pass_count;
    hd_state.pass_capacity = view->pass_capacity;
    hd_state.clear_color = view->clear_color;
    hd_state.content_version = view->content_version;
    hd_state.sprite_content_version = view->sprite_content_version;
    hd_state.ui_half_width = view->ui_half_width;
    hd_state.restore_image = view->restore_image;
    hd_state.backdrop = view->backdrop;
}

static bool hd_mixed_view(const HdSnapshot *previous, const HdSnapshot *current, float fract, HdSnapshot *view)
{
    *view = *current;
    if (fract >= 1.0f)
        return true;
    if (!hd_mix_poses(previous, current, fract))
        return false;
    if (hd_state.mixed_draw_capacity < current->world_count)
    {
        HdWorldDraw *draws = (HdWorldDraw *)hd_grow_sample(hd_state.mixed_draws, &hd_state.mixed_draw_capacity, current->world_count, sizeof(HdWorldDraw));
        if (draws == NULL)
            return false;
        hd_state.mixed_draws = draws;
    }
    view->world_draws = hd_state.mixed_draws;
    view->world_capacity = hd_state.mixed_draw_capacity;
    if (current->world_count != 0)
        memcpy(view->world_draws, current->world_draws, current->world_count * sizeof(HdWorldDraw));
    if (current->backdrop.source != NULL && previous->backdrop.source == current->backdrop.source)
    {
        view->backdrop.offset[0] = -hd_angle_delta(previous->backdrop.yaw, current->backdrop.yaw) * current->backdrop.width / 256.0f * (1.0f - fract);
        view->backdrop.offset[1] = hd_angle_delta(previous->backdrop.pitch, current->backdrop.pitch) * 640.0f / 2048.0f * (1.0f - fract);
    }
    HdCamera camera = hd_mix_camera(&previous->camera, &current->camera, fract);
    for (uint32 idx = 0; idx < current->world_count; ++idx)
    {
        HdWorldDraw *draw = view->world_draws + idx;
        const MATRIXF *model = draw->pose < current->node_count ? &hd_state.mixed_poses[draw->pose].model : &draw->model;
        hd_mix_draw(draw, model, &camera);
        if (draw->mesh != NULL)
        {
            uint32 old_pose = draw->pose < current->node_count ? hd_state.mixed_poses[draw->pose].previous : HD_NO_POSE;
            if (draw->pose == HD_NO_POSE || old_pose != HD_NO_POSE)
            {
                draw->scroll = hd_previous_source(previous, draw, old_pose);
                draw->morph = hd_previous_mesh(previous, draw, old_pose);
            }
            draw->scroll_fract = fract;
            if (draw->morph != NULL)
            {
                if (!hd_prepare_morph(draw->morph))
                    return false;
                draw->uniforms[70] = fract;
                draw->uniforms[71] = draw->morph->morph_width;
            }
        }
    }
    return hd_mix_sprites(previous, current, fract, view);
}

bool hd_render_sample(const HdSnapshot *sample, const HdSnapshot *previous)
{
    HdSnapshot view = *sample;
    bool paired = previous->valid && previous->collection + 1 == sample->collection &&
        previous->epoch == sample->epoch && sample->epoch == hd_state.pose_epoch && previous->scene == sample->scene &&
        previous->camera_epoch == sample->camera_epoch && sample->camera_epoch == hd_state.camera_epoch &&
        previous->camera_valid && sample->camera_valid && !previous->restore_image && !sample->restore_image && !hd_state.paused &&
        previous->ui_half_width == sample->ui_half_width &&
        previous->camera.focal == sample->camera.focal && previous->camera.near_z == sample->camera.near_z && previous->camera.far_z == sample->camera.far_z;
    uint64 field;
    float fract = xport_frame_fract(&field);
    paired = paired && previous->clock_index + 2 == sample->clock_index && sample->clock_index == (field & ~(uint64)1);
    float phase = paired ? ((field & 1) + fract) * 0.5f : 1.0f;
    if (paired && !hd_mixed_view(previous, sample, phase, &view))
        return false;
    HdSnapshot work;
    hd_work_view(&work);
    hd_use_sample(&view);
    for (uint32 idx = 0; idx < view.world_count; ++idx)
        if (view.world_draws[idx].mesh != NULL)
            view.world_draws[idx].mesh->queued = true;
    bool result = hd_end();
    if (result)
        hd_state.live_collection = sample->collection;
    hd_use_sample(&work);
    return result;
}

static const HdSnapshot *hd_ready_sample(void)
{
    const HdSnapshot *sample = hd_state.samples + hd_state.latest_sample;
    if (hd_frame.active || hd_state.frame_active || !hd_state.last_hd || !sample->valid ||
        sample->vram_version != gpu_vram_version() ||
        (sample->restore_image && (sample->image_version != hd_state.image_version || !hd_state.image_valid)))
        return NULL;
    return sample;
}

bool hd_replay(void)
{
    const HdSnapshot *sample = hd_ready_sample();
    if (sample == NULL)
        return false;
    sint32 width, height;
    vid_resize(&width, &height);
    if (hd_state.paused && hd_state.pause_redraw != NULL &&
        (!hd_state.pause_valid || !xport_hd || width != hd_state.pause_width || height != hd_state.pause_height))
        return false;
    float half_width = height > 0 ? 120.0f * width / height : 160.0f;
    if (half_width != sample->ui_half_width || !hd_begin(sample->clear_color))
        return false;
    bool result = hd_render_sample(sample, hd_state.samples + 1 - hd_state.latest_sample);
    return result;
}

void hd_wait(uint64 deadline)
{
    for (;;)
    {
        psx_native_poll_events();
        if (CurrentField() >= deadline)
            break;
        if (hd_ready_sample() != NULL)
            hd_replay();
    }
}
