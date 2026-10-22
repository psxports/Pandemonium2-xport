#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "game.h"
#include "psx.h"
#include "psx_gpu.h"

// Zero selects the shared native standard GPU
const uint32 xport_gpu_graph_type_address = 0u;

// Direct DATA and native CD callbacks need no guest CD bindings
const uint32 xport_cd_sync_callback_address = 0u;

const uint32 xport_cd_ready_callback_address = 0u;

const uint32 xport_cd_status_address = 0u;

const uint32 xport_cd_setloc_table_address = 0u;

static void GameEntry(void)
{
    FUNCTION_MARKER(0x8001127C, "SLUS_005.78");

    // Only GPU packet pools and ordering tables retain device RAM storage
    memset(psx_addr(0x8008B720u, 0x1A504u), 0, 0x1A504u);
    // Retain the BIOS interrupt setup formerly reached through the guest heap
    if (xport_memory_read(0xA0000100u, 4u) == 0)
        SetConf(16, 4, 0x801FFF00u);
    GameFlowState::RunGame();
    trap(1);
}

void xport_main(void)
{
    // The game paces frames with root-counter events
    gpu_set_frame_skip(0);
    SsInit();
    memset(&psx_callback_driver, 0, sizeof(psx_callback_driver));
    SetVideoMode(OriginalDefaults::VideoMode);
    for (uint32 callback_slot = 0; callback_slot < 8; ++callback_slot)
        setIntrVSync(callback_slot, NULL);
    SetGraphDebug(0);
    uint32 *gpu_terminator = (uint32 *)psx_addr(0x8008A040u, sizeof(gpu_dma_nop_terminator));
    for (uint32 word = 0; word < sizeof(gpu_dma_nop_terminator) / sizeof(gpu_dma_nop_terminator[0]); ++word)
        gpu_terminator[word] = gpu_dma_nop_terminator[word];
    memset(&gpu_graph_control, 0, sizeof(gpu_graph_control));
    *spu_native_transfer_state = OriginalDefaults::spu_transfer_startup;
    memset(spu_key_control, 0, sizeof(*spu_key_control));
    memset(&spu_dma_event_state, 0, sizeof(spu_dma_event_state));
    memset(sequence_track_storage, 0, sizeof(sequence_track_storage));
    spu_sequence_scheduler = OriginalDefaults::sequence_scheduler_startup;
    *spu_sequence_clock() = OriginalDefaults::sequence_clock_startup;
    spu_sequence_clock()->tick = PumpSequences;
    memset(spu_tone_commands, 0, sizeof(spu_tone_commands));
    memset(&spu_sequence_handlers, 0, sizeof(spu_sequence_handlers));
    memset(&spu_voice_modulation_callbacks, 0, sizeof(spu_voice_modulation_callbacks));
    spu_sequence_clear_marks();
    spu_vm_reset();
    memset(&spu_reverb_attributes, 0, sizeof(spu_reverb_attributes));
    random_generator.seed = OriginalDefaults::RandomSeed;
    memset(&frame_clock, 0, sizeof(frame_clock));
    {
        RenderMatrixSlot *slot = render_view.matrix_root.next;
        while (slot != 0)
        {
            RenderMatrixSlot *next_slot = slot->next;
            free(slot);
            slot = next_slot;
        }
    }
    memset(&render_view, 0, sizeof(render_view));
    render_view.matrix_cursor = &render_view.matrix_root;
    CopyBytes((uint8 *)&render_view.matrix_root.matrix, OriginalDefaults::scratchpad_startup + (OriginalDefaults::RenderMatrixCursorOffset - 0x1F800000u), sizeof(MATRIX));
    {
        RenderMatrixSlot *reserve_slot = &render_view.matrix_root;
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
    memset(&path_routing, 0, sizeof(path_routing));
    path_routing.reject_repeated = 1;
    path_transition_rules = path_transition_defaults;
    memset(&camera_proj, 0, sizeof(camera_proj));
    memset(&camera_motion, 0, sizeof(camera_motion));
    camera_proj.scale = OriginalDefaults::CameraProjectionScale;
    memset(&scene_lighting, 0, sizeof(scene_lighting));
    memset(&preview_playback, 0, sizeof(preview_playback));
    preview_playback.first_step = preview_animation_steps;
    memset(&render_commands, 0, sizeof(render_commands));
    render_buffers[0] = render_buffer_defaults[0];
    render_buffers[1] = render_buffer_defaults[1];
    memset(&scr_render, 0, sizeof(scr_render));
    game_flow.transition_flags = OriginalDefaults::GameTransitionFlags;
    game_flow.mode_flags = OriginalDefaults::GameModeFlags;
    game_flow.attract_phase = 0;
    game_flow.requested_level = 0;
    memset(&game_flow.progress, 0, sizeof(game_flow.progress));
    player_interaction.flags = OriginalDefaults::InteractionFlags;
    player_interaction.target = 0;
    memset(&player_interaction.pos, 0, sizeof(player_interaction.pos));
    memset(&resource_archive, 0, sizeof(resource_archive));
    memset(&resource_load_cursor, 0, sizeof(resource_load_cursor));
    resource_memory.allocated_bytes = OriginalDefaults::resource_memory_words[0];
    resource_memory.arena = 0;
    memset(&object_speed_definitions, 0, sizeof(object_speed_definitions));
    memset(&volley_enemy, 0, sizeof(volley_enemy));
    volley_enemy.reset_pending = OriginalDefaults::VolleyResetPending;
    volley_enemy.turn_pending = OriginalDefaults::VolleyTurnPending;
    memset(&attack_enemy, 0, sizeof(attack_enemy));
    attack_enemy.special_pending = OriginalDefaults::AttackSpecialPending;
    attack_enemy.event_pending = OriginalDefaults::AttackEventPending;
    {
        ObjectOperations empty_operations = {0};
        object_operations = empty_operations;
    }
    character_action_flags = OriginalDefaults::CharacterActionFlags;
    character_action_owner = 0;
    character_action_target = 0;
    character_action_target_score = OriginalDefaults::CharacterActionTargetScore;
    character_sound_resume_flags = OriginalDefaults::CharacterSoundResumeFlags;
    {
        CharacterController empty_pad = {0};
        character_pad = empty_pad;
    }
    character_pad.model_rebind_pending = OriginalDefaults::CharacterModelRebindPending;
    memset(&character_geometry, 0, sizeof(character_geometry));
    memset(&col_transform, 0, sizeof(col_transform));
    memset(&trigger_contact_motion, 0, sizeof(trigger_contact_motion));
    {
        CharacterContactPool empty_contacts = {0};
        character_contact_pool = empty_contacts;
    }
    {
        uint32 effect_idx;
        for (effect_idx = 0; effect_idx < ActionEffectResources::Count; ++effect_idx)
        {
            free(action_effect_resources.material_resources[effect_idx].texs);
            free(action_effect_resources.material_resources[effect_idx].materials);
            free(action_effect_resources.material_resources[effect_idx].data);
            memset(&action_effect_resources.material_resources[effect_idx], 0, sizeof(action_effect_resources.material_resources[effect_idx]));
            free(action_effect_resources.model_vertices[effect_idx]);
            action_effect_resources.model_vertices[effect_idx] = 0;
            free(action_effect_resources.model_polygons[effect_idx]);
            action_effect_resources.model_polygons[effect_idx] = 0;
            free(action_effect_resources.model_data[effect_idx]);
            action_effect_resources.model_data[effect_idx] = 0;
            action_effect_resources.model_bytes[effect_idx] = 0;
            free(action_effect_resources.animation_data[effect_idx]);
            if (action_effect_resources.animation_headers[effect_idx] != 0)
            {
                for (uint32 header_idx = 0; header_idx < 1; ++header_idx)
                {
                    for (uint32 track_grp = 0; track_grp < 4; ++track_grp)
                        free(action_effect_resources.animation_headers[effect_idx][header_idx].tracks[track_grp].bytes);
                }
            }
            free(action_effect_resources.animation_headers[effect_idx]);
            action_effect_resources.animation_data[effect_idx] = 0;
            action_effect_resources.animation_headers[effect_idx] = 0;
            action_effect_resources.animation_bytes[effect_idx] = 0;
            action_effect_resources.templates[effect_idx] = 0;
        }
    }
    col_contact.selection = OriginalDefaults::ContactSelection;
    col_contact.fraction = OriginalDefaults::ContactFraction;
    col_contact.other_selection = OriginalDefaults::ContactOtherSelection;
    col_contact.flags = OriginalDefaults::ContactFlags;
    memset(&sound_effects, 0, sizeof(sound_effects));
    memset(&sound_resources, 0, sizeof(sound_resources));
    for (uint32 spu_voice = 0; spu_voice < 24; ++spu_voice)
        spu_sample_notes[spu_voice] = SPU_INITIAL_SAMPLE_NOTE;
    memset(&sequence_playback, 0, sizeof(sequence_playback));
    sequence_playback.loading_bank = OriginalDefaults::SequenceLoadingBank;
    for (uint32 voice = 0; voice < 8; ++voice)
        sound_effects.voice_order[voice] = OriginalDefaults::sound_voice_order[voice];
    sound_settings.effects_volume = OriginalDefaults::EffectsVolume;
    sound_settings.music_volume = OriginalDefaults::MusicVolume;
    memset(&hud, 0, sizeof(hud));
    free(menu_archive.data);
    memset(&menu_archive, 0, sizeof(menu_archive));
    free(menu_state.resource_view.header.texs);
    free(menu_state.resource_view.header.materials);
    free(menu_state.resource_view.header.data);
    memset(&menu_state.resource_view.header, 0, sizeof(menu_state.resource_view.header));
    menu_state = MenuState();
    menu_state.image_asset = OriginalDefaults::MenuImageAsset;
    menu_state.highlight_step = OriginalDefaults::MenuHighlightStep;
    menu_state.password.reset_pending = OriginalDefaults::PasswordResetPending != 0;
    control_bindings.active_scheme = &OriginalDefaults::control_schemes[0];
    control_bindings.sel_idx = 0;
    gameplay_input = GameplayInput();
    memset(resource_grps, 0, sizeof(resource_grps));
    free(scene_loader.image_data);
    scene_loader.image_data = 0;
    scene_loader.image_bytes = 0;
    free(scene_loader.geometry_data);
    scene_loader.geometry_data = 0;
    scene_loader.geometry_bytes = 0;
    for (uint32 src_slot = 0; src_slot < 2; ++src_slot)
    {
        free(scene_stream.buffers[src_slot].resource_data);
        scene_stream.buffers[src_slot].resource_data = 0;
        scene_stream.buffers[src_slot].resource_bytes = 0;
        scene_stream.buffers[src_slot].section.data = 0;
        scene_stream.buffers[src_slot].section.data_bytes = 0;
    }
    free(scene_loader.materials_data);
    scene_loader.materials_data = 0;
    scene_loader.materials_bytes = 0;
    memset(&scene_loader, 0, sizeof(scene_loader));
    free(level_loader.transfer_data);
    memset(&level_loader, 0, sizeof(level_loader));
    memset(&level_resource, 0, sizeof(level_resource));
    path_heading_delta.x = 0;
    path_heading_delta.y = 0;
    path_heading_delta.z = 0;
    for (uint32 grp_idx = 0; grp_idx < ModelGroupRegistry::Capacity; ++grp_idx)
    {
        for (uint32 model_idx = 0; model_idx < 4; ++model_idx)
        {
            free(model_grps.recs[grp_idx].material_resources[model_idx].texs);
            free(model_grps.recs[grp_idx].material_resources[model_idx].materials);
            free(model_grps.recs[grp_idx].material_resources[model_idx].data);
            memset(&model_grps.recs[grp_idx].material_resources[model_idx], 0, sizeof(model_grps.recs[grp_idx].material_resources[model_idx]));
            free(model_grps.recs[grp_idx].model_vertices[model_idx]);
            model_grps.recs[grp_idx].model_vertices[model_idx] = 0;
            free(model_grps.recs[grp_idx].model_polygons[model_idx]);
            model_grps.recs[grp_idx].model_polygons[model_idx] = 0;
            free(model_grps.recs[grp_idx].model_data[model_idx]);
            model_grps.recs[grp_idx].model_data[model_idx] = 0;
            model_grps.recs[grp_idx].model_bytes[model_idx] = 0;
            free(model_grps.recs[grp_idx].animation_data[model_idx]);
            if (model_grps.recs[grp_idx].animation_headers[model_idx] != 0)
            {
                for (uint32 header_idx = 0; header_idx < 1; ++header_idx)
                {
                    for (uint32 track_grp = 0; track_grp < 4; ++track_grp)
                        free(model_grps.recs[grp_idx].animation_headers[model_idx][header_idx].tracks[track_grp].bytes);
                }
            }
            free(model_grps.recs[grp_idx].animation_headers[model_idx]);
            model_grps.recs[grp_idx].animation_data[model_idx] = 0;
            model_grps.recs[grp_idx].animation_headers[model_idx] = 0;
            model_grps.recs[grp_idx].animation_bytes[model_idx] = 0;
        }
    }
    memset(&model_grps, 0, sizeof(model_grps));
    for (uint32 model_idx = 0; model_idx < CharacterController::ModelCount; ++model_idx)
    {
        free(character_model_headers[model_idx].material_resource.texs);
        free(character_model_headers[model_idx].material_resource.materials);
        free(character_model_headers[model_idx].material_resource.data);
        memset(&character_model_headers[model_idx].material_resource, 0, sizeof(character_model_headers[model_idx].material_resource));
        free(character_model_headers[model_idx].model_vertices);
        character_model_headers[model_idx].model_vertices = 0;
        free(character_model_headers[model_idx].model_polygons);
        character_model_headers[model_idx].model_polygons = 0;
        free(character_model_headers[model_idx].model_data);
        character_model_headers[model_idx].model_data = 0;
        character_model_headers[model_idx].model_bytes = 0;
        free(character_model_headers[model_idx].animation_data);
        if (character_model_headers[model_idx].animation_headers != 0)
        {
            for (uint32 header_idx = 0; header_idx < 38; ++header_idx)
            {
                for (uint32 track_grp = 0; track_grp < 4; ++track_grp)
                    free(character_model_headers[model_idx].animation_headers[header_idx].tracks[track_grp].bytes);
            }
        }
        free(character_model_headers[model_idx].animation_headers);
        free(character_model_headers[model_idx].morph_restoration);
        character_model_headers[model_idx].animation_data = 0;
        character_model_headers[model_idx].animation_headers = 0;
        character_model_headers[model_idx].morph_restoration = 0;
        character_model_headers[model_idx].animation_bytes = 0;
    }
    for (uint32 model_idx = 0; model_idx < sizeof(actor_models.definitions) / sizeof(actor_models.definitions[0]); ++model_idx)
    {
        free(actor_models.definitions[model_idx].material_resource.texs);
        free(actor_models.definitions[model_idx].material_resource.materials);
        free(actor_models.definitions[model_idx].material_resource.data);
        memset(&actor_models.definitions[model_idx].material_resource, 0, sizeof(actor_models.definitions[model_idx].material_resource));
        free(actor_models.definitions[model_idx].model_vertices);
        actor_models.definitions[model_idx].model_vertices = 0;
        free(actor_models.definitions[model_idx].model_polygons);
        actor_models.definitions[model_idx].model_polygons = 0;
        free(actor_models.definitions[model_idx].model_data);
        actor_models.definitions[model_idx].model_data = 0;
        actor_models.definitions[model_idx].model_bytes = 0;
        free(actor_models.definitions[model_idx].animation_data);
        if (actor_models.definitions[model_idx].animation_headers != 0)
        {
            for (uint32 header_idx = 0; header_idx < 14; ++header_idx)
            {
                for (uint32 track_grp = 0; track_grp < 4; ++track_grp)
                    free(actor_models.definitions[model_idx].animation_headers[header_idx].tracks[track_grp].bytes);
            }
        }
        free(actor_models.definitions[model_idx].animation_headers);
        free(actor_models.definitions[model_idx].morph_restoration);
    }
    for (uint32 model_idx = 0; model_idx < sizeof(animated_models.recs) / sizeof(animated_models.recs[0]); ++model_idx)
    {
        free(animated_models.recs[model_idx].animation_data);
        if (animated_models.recs[model_idx].animation_headers != 0)
        {
            for (uint32 header_idx = 0; header_idx < 2; ++header_idx)
            {
                for (uint32 track_grp = 0; track_grp < 4; ++track_grp)
                    free(animated_models.recs[model_idx].animation_headers[header_idx].tracks[track_grp].bytes);
            }
        }
        free(animated_models.recs[model_idx].animation_headers);
        free(animated_models.recs[model_idx].morph_restoration);
    }
    memset(&actor_models, 0, sizeof(actor_models));
    memset(&animated_models, 0, sizeof(animated_models));
    for (uint32 slot = 0; slot < SceneStream::SequenceBufferCount; ++slot)
        free(scene_stream.sequence_bundles[slot].data);
    for (uint32 slot = 0; slot < 2; ++slot)
    {
        BackgroundResource *background = &scene_stream.backgrounds[slot];
        free(background->data);
        free(background->tiles);
        memset(background, 0, sizeof(*background));
        free(scene_stream.buffers[slot].section.materials.texs);
        free(scene_stream.buffers[slot].section.materials.materials);
        free(scene_stream.buffers[slot].section.materials.data);
        memset(&scene_stream.buffers[slot].section.materials, 0, sizeof(scene_stream.buffers[slot].section.materials));
        free(scene_stream.buffers[slot].section.lighting_recs);
        for (ActorBinding *binding = scene_stream.buffers[slot].section.actor_bindings; binding != 0; binding = binding->next)
            free(binding->rotation_track);
        free(scene_stream.buffers[slot].section.actor_bindings);
        free(scene_stream.buffers[slot].section.animated_paths);
        free(scene_stream.buffers[slot].section.lights);
        free(scene_stream.buffers[slot].section.meshes);
        free(scene_stream.buffers[slot].section.face_storage);
        free(scene_stream.buffers[slot].section.vertex_storage);
        free(scene_stream.buffers[slot].section.col_storage);
        free(scene_stream.buffers[slot].section.material_animation_words);
        free(scene_stream.buffers[slot].section.animation_payload_data);
        if (scene_stream.buffers[slot].section.animations != 0)
        {
            for (uint32 header_idx = 0; header_idx < scene_stream.buffers[slot].section.animation_count; ++header_idx)
            {
                for (uint32 track_grp = 0; track_grp < 4; ++track_grp)
                    free(scene_stream.buffers[slot].section.animations[header_idx].tracks[track_grp].bytes);
            }
        }
        free(scene_stream.buffers[slot].section.animations);
        free(scene_stream.buffers[slot].section.billboards);
        free(scene_stream.buffers[slot].section.mesh_parts);
        free(scene_stream.buffers[slot].section.sphere_nodes);
        free(scene_stream.buffers[slot].section.placements);
        free(scene_stream.buffers[slot].section.animated_objects);
        free(scene_stream.buffers[slot].section.sprs);
        for (uint32 track_idx = 0; track_idx < scene_stream.buffers[slot].section.spr_extent_track_count; ++track_idx)
            free(scene_stream.buffers[slot].section.spr_extent_tracks[track_idx]);
        free(scene_stream.buffers[slot].section.spr_extent_tracks);
        free(scene_stream.buffers[slot].section.triggers);
        free(scene_stream.buffers[slot].section.path_names);
        free(scene_stream.buffers[slot].section.paths);
        free(scene_stream.buffers[slot].section.path_key_data);
        free(scene_stream.buffers[slot].section.path_offset_matrices);
        free(scene_stream.buffers[slot].section.definition_transforms);
        scene_stream.buffers[slot].section.definition_transforms = 0;
        scene_stream.buffers[slot].section.path_offset_matrices = 0;
        scene_stream.buffers[slot].section.lights = 0;
        scene_stream.buffers[slot].section.light_count = 0;
        scene_stream.buffers[slot].section.meshes = 0;
        scene_stream.buffers[slot].section.mesh_count = 0;
        scene_stream.buffers[slot].section.face_storage = 0;
        scene_stream.buffers[slot].section.face_count = 0;
        scene_stream.buffers[slot].section.vertex_storage = 0;
        scene_stream.buffers[slot].section.vertex_count = 0;
        scene_stream.buffers[slot].section.col_storage = 0;
        scene_stream.buffers[slot].section.col_count = 0;
        scene_stream.buffers[slot].section.material_animation_words = 0;
        scene_stream.buffers[slot].section.material_animation_word_count = 0;
        scene_stream.buffers[slot].section.animation_payload_data = 0;
        scene_stream.buffers[slot].section.animation_payload_bytes = 0;
        scene_stream.buffers[slot].section.animations = 0;
        scene_stream.buffers[slot].section.animation_count = 0;
        scene_stream.buffers[slot].section.billboards = 0;
        scene_stream.buffers[slot].section.billboard_count = 0;
        scene_stream.buffers[slot].section.mesh_parts = 0;
        scene_stream.buffers[slot].section.mesh_part_count = 0;
        scene_stream.buffers[slot].section.sphere_nodes = 0;
        scene_stream.buffers[slot].section.sphere_node_count = 0;
        scene_stream.buffers[slot].section.placements = 0;
        scene_stream.buffers[slot].section.placement_count = 0;
        scene_stream.buffers[slot].section.animated_objects = 0;
        scene_stream.buffers[slot].section.animated_object_count = 0;
        scene_stream.buffers[slot].section.sprs = 0;
        scene_stream.buffers[slot].section.spr_count = 0;
        scene_stream.buffers[slot].section.spr_extent_tracks = 0;
        scene_stream.buffers[slot].section.spr_extent_track_count = 0;
        scene_stream.buffers[slot].section.triggers = 0;
        scene_stream.buffers[slot].section.trigger_count = 0;
        scene_stream.buffers[slot].section.path_names = 0;
        scene_stream.buffers[slot].section.path_name_count = 0;
        scene_stream.buffers[slot].section.paths = 0;
        scene_stream.buffers[slot].section.path_count = 0;
        scene_stream.buffers[slot].section.path_key_data = 0;
        scene_stream.buffers[slot].section.actor_bindings = 0;
        scene_stream.buffers[slot].section.animated_paths = 0;
        scene_stream.buffers[slot].section.lighting_recs = NULL;
        scene_stream.buffers[slot].section.lighting_count = 0;
    }
    memset(&scene_stream, 0, sizeof(scene_stream));
    {
        CutscenePlayback empty_cutscene = {0};
        cutscene = empty_cutscene;
    }
    memset(&scene_events, 0, sizeof(scene_events));
    scene_events.pending = OriginalDefaults::PendingSceneEvent;
    memset(cutscene_cameras, 0, sizeof(cutscene_cameras));
    if (interface_resources != 0)
    {
        free((*interface_resources).texs);
        free((*interface_resources).materials);
        free((*interface_resources).data);
        memset(&(*interface_resources), 0, sizeof((*interface_resources)));
    }
    interface_resources = 0;
    memset(&material_registry, 0, sizeof(material_registry));
    memset(&saved_material_registry, 0, sizeof(saved_material_registry));
    free(spr_pool.recs);
    memset(&spr_pool, 0, sizeof(spr_pool));
    registered_object_head = 0;
    GameEntry();
}

void InitializeGame(uint32 mode)
{
    uint32 startup_image[OriginalDefaults::StartupImageReadBytes / sizeof(uint32)];
    uint32 behavior_bundle[OriginalDefaults::BehaviorBundleReadBytes / sizeof(uint32)];
    uint32 asset;
    uint32 offset;
    const uint8 *sound_data;
    uint8 *startup_bundle;
    uint32 startup_bundle_bytes;
    uint32 startup_bundle_capacity;
    uint32 sound_size;
    uint8 *loop_bundle;
    uint32 loop_bytes;
    uint32 loop_capacity;
    FUNCTION_MARKER(0x80067400, "SLUS_005.78");
    game_flow.transition_flags = 0;
    FlushCache();
    ResetCallbackDriver();
    if (resource_memory.arena == 0)
    {
        EnterBiosCriticalSection();
        ExitBiosCriticalSection();
    }
    InitGeom();
    xport_gte_write_control(24, 160u << 16);
    xport_gte_write_control(25, 120u << 16);
    SequencePlayback::InitGameSound();
    RenderCommands::InitializeRenderBuffers();
    CdInit();
    FrameClock::InitializeGameClock();
    GameplayInput::InitializeGamepad();
    resource_memory.arena = AllocateGameMemory(1254400);
    resource_archive.LoadIdx();
    resource_archive.ReadAsset(22, startup_image, 1);
    scr_render.startup_image_loaded = 0;
    ScreenRenderState::DrawStartupImage(startup_image);
    ++scr_render.startup_image_loaded;
    resource_archive.ReadAsset(0, behavior_bundle, 1);
    resource_load_cursor.asset = 1;
    VolleyEnemyState::LoadVolleyScripts((const uint8 *)behavior_bundle);
    AttackEnemyState::LoadAttackScripts((const uint8 *)behavior_bundle);
    resource_archive.ReadAsset(3, behavior_bundle, 1);
    resource_load_cursor.asset = 4;
    ObjectSpeedDefinitions::LoadObjectSpeedDefinitions((const uint8 *)behavior_bundle);
    loop_bytes = resource_archive.Size(17);
    loop_capacity = (loop_bytes + 2047) & ~2047u;
    loop_bundle = (uint8 *)malloc(loop_capacity != 0 ? loop_capacity : 1);
    resource_archive.ReadAsset(17, loop_bundle, 1);
    SequencePlayback::LoadLoopSequence(loop_bundle, loop_bytes);
    free(loop_bundle);
    if (game_flow.mode_flags & 0x200)
    {
        resource_load_cursor.container_asset = 47;
        asset = 147;
    }
    else
    {
        resource_load_cursor.container_asset = 150;
        asset = 250;
    }
    startup_bundle_bytes = resource_archive.Size(asset);
    startup_bundle_capacity = (startup_bundle_bytes + 2047u) & ~2047u;
    startup_bundle = (uint8 *)malloc(startup_bundle_capacity);
    resource_archive.ReadAsset(asset, startup_bundle, 1);
    offset = resource_archive.Offset(251);
    {
        uint32 image_idx = 0;

        uint32 img1_offset = resource_archive.Offset(252);
        uint32 img1_extent = (uint32)OriginalDefaults::tex_img_rects[8].w * (uint32)OriginalDefaults::tex_img_rects[8].h * 2;
        img1_extent += img1_offset;

        ScreenRenderState::UploadTexImage(startup_bundle + offset, image_idx, 1);
    }
    if (game_flow.mode_flags & 0x200)
    {
        free(startup_bundle);
        startup_bundle_bytes = resource_archive.Size(250);
        startup_bundle_capacity = (startup_bundle_bytes + 2047u) & ~2047u;
        startup_bundle = (uint8 *)malloc(startup_bundle_capacity);
        resource_archive.ReadAsset(250, startup_bundle, 1);
    }
    SoundTableHeader::SetSoundTransferAddress(253, 0);
    offset = resource_archive.Offset(253);
    sound_size = resource_archive.Size(253);
    {
    }
    sound_data = startup_bundle + offset;
    SoundTableHeader::UploadSoundSamples(sound_data, sound_size);
    SpuIsTransferCompleted(1);
    free(startup_bundle);
    CameraProjection::ResetSceneRendering();
    ResourceGroup::LoadCommonGameResources();
    game_flow.mode_flags = 0;
    if ((mode << 24) != 0)
        MenuState::ReloadMenuArchive();
}
