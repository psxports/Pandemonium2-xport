#include "hd_internal.h"

HdState hd_state;
HdFrameState hd_frame;

void hd_invalidate_geometry(void)
{
    // Geometry replacement does not end the lifetime of live scene objects
    while (hd_state.meshes != NULL)
    {
        HdMesh *mesh = hd_state.meshes;
        hd_state.meshes = mesh->next;
        if (mesh->queued || mesh->sample_refs != 0)
        {
            mesh->next = hd_state.retired_meshes;
            hd_state.retired_meshes = mesh;
        }
        else
            hd_mesh_free(mesh);
    }
}

void hd_reset(void)
{
    hd_frame.collecting = false;
    if (hd_state.frame_active)
        vid_cancel();
    hd_state.frame_active = false;
    hd_state.pose_owner = NULL;
    hd_state.pose_active = false;
    hd_state.graph_depth = 0;
    hd_state.restore_image = false;
    hd_state.ui_count = 0;
    hd_og_release(hd_state.og_draws, hd_state.og_count);
    hd_state.og_count = 0;
    hd_state.og_background_skipped = false;
    hd_state.pass_count = 0;
    memset(&hd_state.backdrop, 0, sizeof(hd_state.backdrop));
    hd_state.sprite_count = 0;
    for (uint32 idx = 0; idx < hd_state.world_count; ++idx)
        if (hd_state.world_draws[idx].mesh != NULL)
            hd_state.world_draws[idx].mesh->queued = false;
    hd_state.world_count = 0;
    hd_animation_nodes_release(hd_state.nodes, hd_state.node_count);
    hd_state.node_count = 0;
    hd_free_retired();
    hd_state.vram_valid = false;
}

void hd_level_reset(void)
{
    memset(&hd_state.precise_camera, 0, sizeof(hd_state.precise_camera));
    if (hd_state.animation_capacity != 0)
    {
        for (uint32 idx = 0; idx < hd_state.animation_capacity; ++idx)
        {
            if (hd_state.animations[idx].node != NULL)
            {
                hd_animation_release(&hd_state.animations[idx].pose.position_channel);
                hd_animation_release(&hd_state.animations[idx].pose.rotation_channel);
            }
        }
        memset(hd_state.animations, 0, hd_state.animation_capacity * sizeof(HdAnimationEntry));
    }
    hd_state.animation_count = 0;
    hd_state.pause_scene = NULL;
    hd_state.pause_redraw = NULL;
    hd_state.pause_valid = false;
    hd_state.paused = false;
    hd_state.pause_collect = false;
    if (hd_state.graph_work_capacity != 0)
        memset(hd_state.graph_work, 0, hd_state.graph_work_capacity * sizeof(HdGraphWork));
    hd_state.graph_work_count = 0;
    hd_reset();
    hd_clear_samples();
    while (hd_state.scroll_clocks != NULL)
    {
        HdScrollClock *clock = hd_state.scroll_clocks;
        hd_state.scroll_clocks = clock->next;
        free(clock);
    }
    hd_scroll_reset();
    hd_invalidate_geometry();
    while (hd_state.pool_ranges != NULL)
    {
        HdPoolRange *range = hd_state.pool_ranges;
        hd_state.pool_ranges = range->next;
        free(range);
    }
    hd_state.world_vertex_count = 0;
    hd_state.world_pool_dirty = true;
}

void hd_free(void)
{
    hd_level_reset();
    vid_mesh_free(hd_state.world_mesh);
    vid_index_free(hd_state.world_indices);
    vid_index_free(hd_state.visible_indices);
    vid_mat_free(hd_state.pooled_material);
    vid_shader_free(hd_state.pooled_shader);
    vid_tex_free(hd_state.world_params);
    vid_tex_free(hd_state.world_groups);
    vid_shader_free(hd_state.world_pool_shader);
    for (uint32 idx = 0; idx < 4; ++idx)
        vid_mat_free(hd_state.world_pool_materials[idx]);
    free(hd_state.visible_draws);
    for (uint32 idx = 0; idx < 4; ++idx)
    {
        if (hd_state.ui_materials[idx] != NULL)
            vid_mat_free(hd_state.ui_materials[idx]);
        if (hd_state.world_materials[idx] != NULL)
            vid_mat_free(hd_state.world_materials[idx]);
        if (hd_state.sprite_materials[idx] != NULL)
            vid_mat_free(hd_state.sprite_materials[idx]);
    }
    if (hd_state.ui_mesh != NULL)
        vid_mesh_free(hd_state.ui_mesh);
    if (hd_state.vram != NULL)
        vid_tex_free(hd_state.vram);
    vid_tex_free(hd_state.empty_morph);
    if (hd_state.ui_shader != NULL)
        vid_shader_free(hd_state.ui_shader);
    if (hd_state.world_shader != NULL)
        vid_shader_free(hd_state.world_shader);
    if (hd_state.sprite_shader != NULL)
        vid_shader_free(hd_state.sprite_shader);
    if (hd_state.sprite_mesh != NULL)
        vid_mesh_free(hd_state.sprite_mesh);
    if (hd_state.image_material != NULL)
        vid_mat_free(hd_state.image_material);
    if (hd_state.image_shader != NULL)
        vid_shader_free(hd_state.image_shader);
    if (hd_state.image_mesh != NULL)
        vid_mesh_free(hd_state.image_mesh);
    vid_target_free(hd_state.live_image);
    vid_target_free(hd_state.saved_image);
    free(hd_state.sprite_vertices);
    free(hd_state.ui_vertices);
    free(hd_state.og_draws);
    hd_og_release(hd_state.frozen_og_draws, hd_state.frozen_og_count);
    free(hd_state.frozen_og_draws);
    free(hd_state.ui_passes);
    free(hd_state.world_draws);
    free(hd_state.poses);
    free(hd_state.paths);
    free(hd_state.nodes);
    free(hd_state.identities);
    free(hd_state.animations);
    free(hd_state.graph_work);
    free(hd_state.mixed_draws);
    free(hd_state.mixed_sprites);
    free(hd_state.mixed_poses);
    free(hd_state.previous_index);
    free(hd_state.current_index);
    for (uint32 idx = 0; idx < 2; ++idx)
    {
        free(hd_state.samples[idx].world_draws);
        free(hd_state.samples[idx].ui_vertices);
        free(hd_state.samples[idx].og_draws);
        free(hd_state.samples[idx].sprite_vertices);
        free(hd_state.samples[idx].ui_passes);
        free(hd_state.samples[idx].nodes);
    }
    memset(&hd_state, 0, sizeof(hd_state));
}

bool hd_init(void)
{
    if (hd_state.ui_shader != NULL)
        return true;
    hd_filter_keys();
    hd_state.ui_shader = vid_shader_create(hd_ui_vertex, hd_texture_fragment);
    if (hd_state.ui_shader == NULL)
        return false;
    hd_state.world_shader = hd_variant_shader("#define HD_LOCAL_MATERIAL\n");
    if (hd_state.world_shader == NULL)
    {
        hd_free();
        return false;
    }
    hd_state.pooled_shader = hd_variant_shader("#define HD_POOL\n");
    hd_state.world_pool_shader = hd_variant_shader("#define HD_POOL_MATERIAL\n");
    if (hd_state.pooled_shader == NULL || hd_state.world_pool_shader == NULL)
    {
        hd_free();
        return false;
    }
    hd_state.sprite_shader = vid_shader_create(hd_sprite_vertex, hd_texture_fragment);
    if (hd_state.sprite_shader == NULL)
    {
        hd_free();
        return false;
    }
    hd_state.vram = vid_tex_create_fmt(1024, 512, VID_RGBA5551);
    if (hd_state.vram == NULL)
    {
        hd_free();
        return false;
    }
    hd_state.empty_morph = vid_tex_create_fmt(1, 1, VID_RGBA16UI);
    if (hd_state.empty_morph == NULL)
    {
        hd_free();
        return false;
    }
    memset(vid_tex_data(hd_state.empty_morph), 0, 8);
    vid_tex_commit(hd_state.empty_morph);
    for (uint32 idx = 0; idx < 4; ++idx)
    {
        hd_state.ui_materials[idx] = vid_mat_create();
        if (hd_state.ui_materials[idx] == NULL)
        {
            hd_free();
            return false;
        }
        VID_MATERIAL_DATA *material = vid_mat_data(hd_state.ui_materials[idx]);
        material->texture = hd_state.vram;
        material->shader = hd_state.ui_shader;
        material->blend = idx;
        material->depth_test = 0;
        material->depth_write = 0;
        material->filter = 0;
        material->repeat = 0;
        material->cull = 0;
        vid_mat_commit(hd_state.ui_materials[idx]);
        hd_state.world_materials[idx] = vid_mat_create();
        if (hd_state.world_materials[idx] == NULL)
        {
            hd_free();
            return false;
        }
        material = vid_mat_data(hd_state.world_materials[idx]);
        material->aux[0] = hd_state.empty_morph;
        material->texture = hd_state.vram;
        material->shader = hd_state.world_shader;
        material->blend = idx;
        material->depth_test = 1;
        material->depth_write = idx == VID_BLEND_NONE;
        material->filter = 0;
        material->repeat = 0;
        material->cull = 0;
        vid_mat_commit(hd_state.world_materials[idx]);
        hd_state.sprite_materials[idx] = vid_mat_create();
        if (hd_state.sprite_materials[idx] == NULL)
        {
            hd_free();
            return false;
        }
        material = vid_mat_data(hd_state.sprite_materials[idx]);
        *material = *vid_mat_data(hd_state.world_materials[idx]);
        material->shader = hd_state.sprite_shader;
        vid_mat_commit(hd_state.sprite_materials[idx]);
    }
    return true;
}

bool hd_begin(uint32 color)
{
    if (!hd_init())
        return false;
    if (hd_state.frame_active)
        vid_cancel();
    for (uint32 idx = 0; idx < hd_state.world_count; ++idx)
        if (hd_state.world_draws[idx].mesh != NULL)
            hd_state.world_draws[idx].mesh->queued = false;
    hd_state.world_count = 0;
    hd_animation_nodes_release(hd_state.nodes, hd_state.node_count);
    hd_state.node_count = 0;
    hd_state.scene = NULL;
    hd_state.draw_pose = HD_NO_POSE;
    memset(hd_state.pool_valid, 1, sizeof(hd_state.pool_valid));
    hd_state.pose_owner = NULL;
    hd_state.pose_active = false;
    hd_state.graph_depth = 0;
    hd_free_retired();
    hd_state.sprite_count = 0;
    sint32 width, height;
    vid_resize(&width, &height);
    hd_state.ui_half_width = height > 0 ? 120.0f * width / height : 160.0f;
    hd_state.ui_count = 0;
    hd_og_release(hd_state.og_draws, hd_state.og_count);
    hd_state.og_count = 0;
    hd_state.pass_count = 0;
    hd_state.og_background_skipped = false;
    memset(&hd_state.backdrop, 0, sizeof(hd_state.backdrop));
    hd_state.restore_image = false;
    hd_state.clear_color = color;
    hd_state.content_version = ++hd_state.next_content_version;
    hd_state.sprite_content_version = hd_state.content_version;
    hd_state.frame_active = vid_begin((color & 255) / 255.0f, ((color >> 8) & 255) / 255.0f, ((color >> 16) & 255) / 255.0f) != 0;
    return hd_state.frame_active;
}

bool hd_end(void)
{
    if (!hd_state.frame_active)
        return false;
    hd_upload_vram();
    for (uint32 idx = 0; idx < 4; ++idx)
    {
        hd_filter_material(hd_state.ui_materials[idx]);
        hd_filter_material(hd_state.world_materials[idx]);
        hd_filter_material(hd_state.sprite_materials[idx]);
    }
    if (hd_state.sprite_count != 0)
    {
        if (hd_state.sprite_mesh_capacity < hd_state.sprite_count)
        {
            VID_GEOMETRY *mesh = vid_mesh_create_2d(hd_state.sprite_capacity);
            if (mesh == NULL)
            {
                hd_reset();
                return false;
            }
            if (hd_state.sprite_mesh != NULL)
                vid_mesh_free(hd_state.sprite_mesh);
            hd_state.sprite_mesh = mesh;
            hd_state.sprite_mesh_capacity = hd_state.sprite_capacity;
        }
        if (hd_state.sprite_version != hd_state.sprite_content_version)
        {
            memcpy(vid_mesh_data_2d(hd_state.sprite_mesh), hd_state.sprite_vertices, hd_state.sprite_count * sizeof(VID_VERTEX2D));
            vid_mesh_commit(hd_state.sprite_mesh);
            hd_state.sprite_version = hd_state.sprite_content_version;
        }
    }
    if (hd_state.ui_count != 0)
    {
        if (hd_state.mesh_capacity < hd_state.ui_count)
        {
            VID_GEOMETRY *mesh = vid_mesh_create_2d(hd_state.ui_capacity);
            if (mesh == NULL)
            {
                hd_reset();
                return false;
            }
            if (hd_state.ui_mesh != NULL)
                vid_mesh_free(hd_state.ui_mesh);
            hd_state.ui_mesh = mesh;
            hd_state.mesh_capacity = hd_state.ui_capacity;
        }
        if (hd_state.ui_version != hd_state.content_version)
        {
            memcpy(vid_mesh_data_2d(hd_state.ui_mesh), hd_state.ui_vertices, (size_t)hd_state.ui_count * sizeof(VID_VERTEX2D));
            vid_mesh_commit(hd_state.ui_mesh);
            hd_state.ui_version = hd_state.content_version;
        }
    }
    for (uint32 layer = HD_BACKGROUND; layer < HD_UI_LAYERS; ++layer)
    {
        float layout[4] = {hd_state.ui_half_width, 0.0f, 0.0f, 0.0f};
        if (layer == HD_BACKGROUND)
        {
            layout[1] = hd_state.backdrop.offset[0];
            layout[2] = hd_state.backdrop.offset[1];
        }
        vid_uniforms(layout, 1);
        for (uint32 idx = 0; idx < hd_state.pass_count; ++idx)
        {
            const HdUiPass *pass = hd_state.ui_passes + idx;
            if (pass->layer != layer)
                continue;
            vid_pass(hd_state.ui_materials[pass->blend]);
            if (!vid_submit(hd_state.ui_mesh, NULL, pass->first, pass->count))
            {
                hd_reset();
                return false;
            }
        }
        if (layer == HD_BACKGROUND && hd_state.restore_image)
        {
            vid_uniforms(NULL, 0);
            if (!vid_draw(hd_state.image_mesh, 0, 6, hd_state.image_material))
            {
                hd_reset();
                return false;
            }
        }
        if (layer == HD_BACKGROUND && !hd_submit_world())
        {
            hd_reset();
            return false;
        }
    }
    hd_state.frame_active = false;
    bool result = vid_end() != 0;
    for (uint32 idx = 0; idx < hd_state.world_count; ++idx)
        if (hd_state.world_draws[idx].mesh != NULL)
            hd_state.world_draws[idx].mesh->queued = false;
    hd_state.world_count = 0;
    hd_free_retired();
    return result;
}

void hd_frame_begin(void)
{
    bool requested = xport_hd != 0;
    if (requested != hd_state.last_hd)
        hd_clear_samples();
    hd_frame.camera_valid = false;
    hd_frame.failed = false;
    hd_frame.collecting = hd_begin(0);
    hd_frame.active = requested && hd_frame.collecting;
    hd_state.restore_image = false;
    if (hd_frame.collecting)
    {
        ++hd_state.collection;
        hd_state.clock_index = game_field & ~(uint64)1;
    }
    if (requested && !hd_frame.active)
        fprintf(stderr, "HD frame initialization failed\n");
    hd_frame.returning_og = hd_state.last_hd && !hd_frame.active;
}

bool hd_frame_end(void)
{
    if (hd_state.pause_collect && hd_frame.active)
        return hd_frame.collecting && !hd_frame.failed;
    bool active = hd_frame.active;
    if (!hd_frame.collecting)
    {
        hd_state.last_hd = false;
        return true;
    }
    hd_frame.active = false;
    hd_frame.collecting = false;
    uint32 incoming = 1 - hd_state.latest_sample;
    HdSnapshot *sample = hd_state.samples + incoming;
    bool captured = !hd_frame.failed && hd_capture_sample(sample);
    bool result = captured;
    if (active)
        result = captured && hd_render_sample(sample, hd_state.samples + hd_state.latest_sample);
    else
    {
        // Keep the collected source available for OG-to-HD transitions
        vid_cancel();
        hd_state.frame_active = false;
        for (uint32 idx = 0; idx < hd_state.world_count; ++idx)
            if (hd_state.world_draws[idx].mesh != NULL)
                hd_state.world_draws[idx].mesh->queued = false;
    }
    if (result)
    {
        sample->valid = true;
        hd_state.latest_sample = incoming;
        hd_state.world_count = 0;
    }
    else if (captured)
        hd_sample_clear(sample);
    hd_state.last_hd = active && result;
    if (!result)
    {
        hd_reset();
        if (active)
            fprintf(stderr, "HD frame collection or submission failed\n");
    }
    hd_free_retired();
    return active ? result : true;
}

void hd_save_image(void)
{
    const HdSnapshot *completed = hd_state.samples + hd_state.latest_sample;
    if (hd_state.last_hd && completed->valid && completed->collection == hd_state.collection &&
        !hd_state.frame_active && render_commands.active_buffer < 2)
    {
        const RenderBuffer *buffer = render_buffers + render_commands.active_buffer;
        if (render_commands.scene_ordering_table_offset == buffer->scene_ordering_table_offset &&
            render_commands.submitted_chain_offset == buffer->scene_ordering_table_offset + 2400)
            hd_og_rebuild_background(completed, buffer->aux_packet_base, buffer->scene_ordering_table_offset);
    }
    hd_og_save_ui(hd_state.samples + hd_state.latest_sample);
    ++hd_state.image_version;
    hd_state.image_valid = false;
    if (hd_state.paused)
        return;
    {
        const HdSnapshot *sample = hd_state.samples + hd_state.latest_sample;
        if (!sample->valid || sample->collection != hd_state.collection || sample->vram_version != gpu_vram_version() || hd_state.frame_active)
            return;
        if (hd_state.live_image == NULL)
            hd_state.live_image = vid_target_create();
        if (hd_state.live_image == NULL || !hd_begin(sample->clear_color))
            return;
        if (!vid_target_select_offscreen(hd_state.live_image) || !hd_render_sample(sample, sample))
        {
            hd_reset();
            return;
        }
    }
    if (hd_state.live_image == NULL)
        return;
    if (hd_state.saved_image == NULL)
        hd_state.saved_image = vid_target_create();
    if (hd_state.saved_image == NULL || !vid_target_copy(hd_state.saved_image, hd_state.live_image))
        return;
    if (hd_state.image_mesh == NULL)
    {
        hd_state.image_mesh = vid_mesh_create(6);
        if (hd_state.image_mesh == NULL)
            return;
        static const uint8 corners[6] = {0, 1, 2, 2, 1, 3};
        VID_VERTEX *vertices = vid_mesh_data(hd_state.image_mesh);
        for (uint32 idx = 0; idx < 6; ++idx)
        {
            uint8 corner = corners[idx];
            vertices[idx].pos[0] = corner & 1 ? 1.0f : -1.0f;
            vertices[idx].pos[1] = corner & 2 ? -1.0f : 1.0f;
            vertices[idx].pos[3] = 1.0f;
            vertices[idx].uv[0] = corner & 1 ? 1.0f : 0.0f;
            vertices[idx].uv[1] = corner & 2 ? 0.0f : 1.0f;
            for (uint32 channel = 0; channel < 4; ++channel)
                vertices[idx].color[channel] = 1.0f;
        }
        vid_mesh_commit(hd_state.image_mesh);
    }
    if (hd_state.image_material == NULL)
    {
        if (hd_state.image_shader == NULL)
            hd_state.image_shader = vid_shader_create(NULL, "void main(){outColor=vec4(texture(uTex,vUV).rgb,1.0);}\n");
        if (hd_state.image_shader == NULL)
            return;
        hd_state.image_material = vid_mat_create();
        if (hd_state.image_material == NULL)
            return;
        vid_mat_data(hd_state.image_material)->texture = vid_target_texture(hd_state.saved_image);
        // Display captures stay opaque after additive or subtractive overlays
        vid_mat_data(hd_state.image_material)->shader = hd_state.image_shader;
        vid_mat_commit(hd_state.image_material);
    }
    hd_state.image_valid = true;
}

void hd_restore_image(const PSX_RECT *source)
{
    if (hd_frame.collecting && hd_state.image_valid)
        hd_state.restore_image = true;
    else if (hd_frame.collecting && source != NULL && !hd_vram_image(source))
        hd_frame.failed = true;
}

void hd_menu_image_changed(void)
{
    hd_og_release(hd_state.frozen_og_draws, hd_state.frozen_og_count);
    hd_state.frozen_og_ui_valid = false;
    hd_state.frozen_og_count = 0;
    ++hd_state.image_version;
    hd_state.image_valid = false;
}

static void hd_drop_image(void)
{
    vid_mat_free(hd_state.image_material);
    vid_shader_free(hd_state.image_shader);
    vid_mesh_free(hd_state.image_mesh);
    vid_target_free(hd_state.saved_image);
    vid_target_free(hd_state.live_image);
    hd_state.image_material = NULL;
    hd_state.image_shader = NULL;
    hd_state.image_mesh = NULL;
    hd_state.saved_image = NULL;
    hd_state.live_image = NULL;
    hd_state.image_valid = false;
    ++hd_state.image_version;
}

void hd_pause_begin(SceneSection *scene, HdSceneRedraw redraw)
{
    hd_clear_samples();
    hd_drop_image();
    hd_state.paused = true;
    hd_state.pause_scene = scene;
    hd_state.pause_redraw = redraw;
    hd_state.pause_valid = false;
    hd_state.pause_collect = false;
}

void hd_pause_end(void)
{
    hd_state.paused = false;
    hd_state.pause_scene = NULL;
    hd_state.pause_redraw = NULL;
    hd_state.pause_valid = false;
    hd_state.pause_collect = false;
    // Keep the displayed frame available for the exit fade after scene teardown
    hd_clear_identities();
    for (uint32 idx = 0; idx < hd_state.pose_capacity; ++idx)
        hd_state.poses[idx].valid = false;
    hd_state.pose_owner = NULL;
    hd_state.pose_active = false;
    hd_state.graph_depth = 0;
    hd_state.path_count = 0;
}

static bool hd_rebuild_paused_scene(sint32 width, sint32 height)
{
    bool requested = xport_hd != 0;
    hd_state.image_valid = false;
    ++hd_state.image_version;
    hd_state.pause_collect = requested;
    hd_frame.capture_only = true;
    hd_state.pause_redraw(hd_state.pause_scene, false);
    hd_frame.capture_only = false;
    hd_state.pause_collect = false;
    DrawSync(0);
    render_commands.pending_disp_env = NULL;
    render_commands.submitted_disp_env = NULL;
    if (requested)
    {
        if (!hd_frame.active || !hd_frame.collecting || hd_frame.failed)
            return false;
    }
    hd_state.pause_hd = requested;
    hd_state.pause_width = width;
    hd_state.pause_height = height;
    hd_state.pause_valid = true;
    return true;
}

void hd_overlay_begin(void)
{
    bool scene_collected = false;
    if (hd_state.paused && hd_state.pause_redraw != NULL)
    {
        sint32 width, height;
        vid_resize(&width, &height);
        bool requested = xport_hd != 0;
        if (width > 0 && height > 0 && (requested || !hd_state.pause_valid || requested != hd_state.pause_hd))
        {
            hd_state.pause_valid = false;
            if (!hd_rebuild_paused_scene(width, height))
                fprintf(stderr, "Paused scene reconstruction failed\n");
            else
                scene_collected = requested;
        }
    }
    if (!scene_collected)
        hd_frame_begin();
    if (hd_frame.collecting)
    {
        // Scene redraw selected its packet arena; the overlay uses the other arena
        render_commands.packet_limit = render_commands.other_packet_base + 41600;
        gpu_frame_begin();
        gpu_set_raster(!hd_frame.active);
        if (!scene_collected)
            hd_restore_image();
    }
}

void hd_frame_rect(uint32 pos, uint32 extent, uint32 color, uint32 blend, HdUiLayer layer)
{
    if (!hd_frame.collecting || hd_frame.failed)
        return;
    HdUiAnchor anchor = (extent & 65535) == 320 ? HD_ANCHOR_FILL : HD_ANCHOR_CENTER;
    if (!hd_rect((sint16)pos, (sint16)(pos >> 16), extent & 65535, extent >> 16, color, blend, anchor, layer))
        hd_frame.failed = true;
}
