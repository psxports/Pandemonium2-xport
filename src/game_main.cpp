#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "game.h"
#include "psx.h"
#include "psx_gpu.h"

/* Unresolved until the original PsyQ graph type storage is audited */
const uint32 xport_gpu_graph_type_address = 0u;

/* Unresolved CD bindings until original storage is audited */
const uint32 xport_cd_sync_callback_address = 0u;
const uint32 xport_cd_ready_callback_address = 0u;
const uint32 xport_cd_status_address = 0u;
const uint32 xport_cd_setloc_table_address = 0u;

static void GameEntry(void)
{
    uint32 address = 0x8008A2D0;
    uint32 heap_bytes;

    FUNCTION_MARKER(0x8001127C, "SLUS_005.78");

    // Clear remaining guest BSS until native migration is complete
    do
    {
        w_u32(address, 0);
        address += 4;
    } while (address < 0x800BA520);

    heap_bytes = OriginalDefaults::GuestStackTop - OriginalDefaults::GuestStackReserve - 0xBA520;
    psx_bios_init_user_heap(0x800BA524, heap_bytes);
    RunGame();
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
    uint8 *gpu_terminator = (uint8 *)psx_addr(0x8008A040u, sizeof(gpu_dma_nop_terminator));
    for (uint32 word = 0; word < sizeof(gpu_dma_nop_terminator) / sizeof(gpu_dma_nop_terminator[0]); ++word)
        xport_store_le32(gpu_terminator + word * sizeof(uint32), gpu_dma_nop_terminator[word]);
    memset(&gpu_graph_control, 0, sizeof(gpu_graph_control));
    *spu_native_transfer_state = OriginalDefaults::SpuTransferStartup;
    memset(spu_key_control, 0, sizeof(*spu_key_control));
    memset(&spu_dma_event_state, 0, sizeof(spu_dma_event_state));
    memset(sequence_track_storage, 0, sizeof(sequence_track_storage));
    spu_sequence_scheduler = OriginalDefaults::SequenceSchedulerStartup;
    *spu_sequence_clock() = OriginalDefaults::SequenceClockStartup;
    spu_sequence_clock()->tick = p2_sdk_80074820;
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
    CopyBytes((uint8 *)&render_view.matrix_root.matrix, OriginalDefaults::ScratchpadStartup + (OriginalDefaults::RenderMatrixCursorOffset - 0x1F800000u), sizeof(MATRIX));
    {
        RenderMatrixSlot *reserve_slot = &render_view.matrix_root;
        uint32 reserve_index;
        for (reserve_index = 0; reserve_index < 3; ++reserve_index)
        {
            if (reserve_slot->next == 0)
            {
                RenderMatrixSlot *next_slot = (RenderMatrixSlot *)malloc(sizeof(RenderMatrixSlot));
                uint32 default_offset, default_bytes;
                if (next_slot == 0)
                {
                    fprintf(stderr, "P2: Cannot allocate native render matrix slot\n");
                    abort();
                }
                memset(next_slot, 0, sizeof(RenderMatrixSlot));
                next_slot->prev = reserve_slot;
                next_slot->depth = reserve_slot->depth + 1;
                default_offset = OriginalDefaults::RenderMatrixCursorOffset - 0x1F800000u + next_slot->depth * 32;
                if (default_offset < sizeof(OriginalDefaults::ScratchpadStartup))
                {
                    default_bytes = sizeof(OriginalDefaults::ScratchpadStartup) - default_offset;
                    if (default_bytes > sizeof(MATRIX))
                        default_bytes = sizeof(MATRIX);
                    CopyBytes((uint8 *)&next_slot->matrix, OriginalDefaults::ScratchpadStartup + default_offset, default_bytes);
                }
                reserve_slot->next = next_slot;
            }
            reserve_slot = reserve_slot->next;
        }
    }
    memset(&path_routing, 0, sizeof(path_routing));
    path_routing.reject_repeated = 1;
    path_transition_rules = path_transition_defaults;
    memset(&camera_projection, 0, sizeof(camera_projection));
    memset(&camera_motion, 0, sizeof(camera_motion));
    camera_projection.scale = OriginalDefaults::CameraProjectionScale;
    memset(&scene_lighting, 0, sizeof(scene_lighting));
    memset(&preview_playback, 0, sizeof(preview_playback));
    preview_playback.first_step = preview_animation_steps;
    memset(&render_commands, 0, sizeof(render_commands));
    render_buffers[0] = render_buffer_defaults[0];
    render_buffers[1] = render_buffer_defaults[1];
    memset(&screen_render, 0, sizeof(screen_render));
    game_flow.transition_flags = OriginalDefaults::GameTransitionFlags;
    game_flow.mode_flags = OriginalDefaults::GameModeFlags;
    game_flow.attract_phase = 0;
    game_flow.requested_level = 0;
    memset(&game_flow.progress, 0, sizeof(game_flow.progress));
    player_interaction.flags = OriginalDefaults::InteractionFlags;
    player_interaction.target = 0;
    memset(&player_interaction.position, 0, sizeof(player_interaction.position));
    memset(&resource_archive, 0, sizeof(resource_archive));
    memset(&resource_load_cursor, 0, sizeof(resource_load_cursor));
    resource_memory.allocated_bytes = OriginalDefaults::ResourceMemoryWords[0];
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
        CharacterController empty_controller = {0};
        character_controller = empty_controller;
    }
    character_controller.model_rebind_pending = OriginalDefaults::CharacterModelRebindPending;
    memset(&character_geometry, 0, sizeof(character_geometry));
    memset(&collision_transform, 0, sizeof(collision_transform));
    memset(&trigger_contact_motion, 0, sizeof(trigger_contact_motion));
    {
        CharacterContactPool empty_contacts = {0};
        character_contact_pool = empty_contacts;
    }
    {
        uint32 effect_index;
        for (effect_index = 0; effect_index < ActionEffectResources::Count; ++effect_index)
        {
            free(action_effect_resources.material_resources[effect_index].textures);
            free(action_effect_resources.material_resources[effect_index].materials);
            free(action_effect_resources.material_resources[effect_index].data);
            memset(&action_effect_resources.material_resources[effect_index], 0, sizeof(action_effect_resources.material_resources[effect_index]));
            free(action_effect_resources.model_vertices[effect_index]);
            action_effect_resources.model_vertices[effect_index] = 0;
            free(action_effect_resources.model_polygons[effect_index]);
            action_effect_resources.model_polygons[effect_index] = 0;
            free(action_effect_resources.model_data[effect_index]);
            action_effect_resources.model_data[effect_index] = 0;
            action_effect_resources.model_bytes[effect_index] = 0;
            free(action_effect_resources.animation_data[effect_index]);
            if (action_effect_resources.animation_headers[effect_index] != 0)
            {
                for (uint32 header_index = 0; header_index < 1; ++header_index)
                {
                    for (uint32 track_group = 0; track_group < 4; ++track_group)
                        free(action_effect_resources.animation_headers[effect_index][header_index].tracks[track_group].bytes);
                }
            }
            free(action_effect_resources.animation_headers[effect_index]);
            action_effect_resources.animation_data[effect_index] = 0;
            action_effect_resources.animation_headers[effect_index] = 0;
            action_effect_resources.animation_bytes[effect_index] = 0;
            action_effect_resources.templates[effect_index] = 0;
        }
    }
    collision_contact.selection = OriginalDefaults::ContactSelection;
    collision_contact.fraction = OriginalDefaults::ContactFraction;
    collision_contact.other_selection = OriginalDefaults::ContactOtherSelection;
    collision_contact.flags = OriginalDefaults::ContactFlags;
    memset(&sound_effects, 0, sizeof(sound_effects));
    memset(&sound_resources, 0, sizeof(sound_resources));
    for (uint32 spu_voice = 0; spu_voice < 24; ++spu_voice)
        spu_sample_notes[spu_voice] = SPU_INITIAL_SAMPLE_NOTE;
    memset(&sequence_playback, 0, sizeof(sequence_playback));
    sequence_playback.loading_bank = OriginalDefaults::SequenceLoadingBank;
    for (unsigned int voice = 0; voice < 8; ++voice)
        sound_effects.voice_order[voice] = OriginalDefaults::SoundVoiceOrder[voice];
    sound_settings.effects_volume = OriginalDefaults::EffectsVolume;
    sound_settings.music_volume = OriginalDefaults::MusicVolume;
    memset(&hud, 0, sizeof(hud));
    free(menu_archive.data);
    memset(&menu_archive, 0, sizeof(menu_archive));
    free(menu_state.resource_view.header.textures);
    free(menu_state.resource_view.header.materials);
    free(menu_state.resource_view.header.data);
    memset(&menu_state.resource_view.header, 0, sizeof(menu_state.resource_view.header));
    menu_state = MenuState();
    menu_state.image_asset = OriginalDefaults::MenuImageAsset;
    menu_state.highlight_step = OriginalDefaults::MenuHighlightStep;
    menu_state.password.reset_pending = OriginalDefaults::PasswordResetPending != 0;
    control_bindings.active_scheme = &OriginalDefaults::ControlSchemes[0];
    control_bindings.selected_index = 0;
    gameplay_input = GameplayInput();
    memset(resource_groups, 0, sizeof(resource_groups));
    free(scene_loader.image_data);
    scene_loader.image_data = 0;
    scene_loader.image_bytes = 0;
    free(scene_loader.geometry_data);
    scene_loader.geometry_data = 0;
    scene_loader.geometry_bytes = 0;
    for (uint32 source_slot = 0; source_slot < 2; ++source_slot)
    {
        free(scene_stream.buffers[source_slot].resource_data);
        scene_stream.buffers[source_slot].resource_data = 0;
        scene_stream.buffers[source_slot].resource_bytes = 0;
        scene_stream.buffers[source_slot].section.data = 0;
        scene_stream.buffers[source_slot].section.data_bytes = 0;
    }
    free(scene_loader.materials_data);
    scene_loader.materials_data = 0;
    scene_loader.materials_bytes = 0;
    memset(&scene_loader, 0, sizeof(scene_loader));
    free(level_loader.transfer_data);
    memset(&level_loader, 0, sizeof(level_loader));
    memset(&level_resource, 0, sizeof(level_resource));
    memset(path_heading_delta, 0, sizeof(path_heading_delta));
    for (uint32 group_index = 0; group_index < ModelGroupRegistry::Capacity; ++group_index)
    {
        for (uint32 model_index = 0; model_index < 4; ++model_index)
        {
            free(model_groups.records[group_index].material_resources[model_index].textures);
            free(model_groups.records[group_index].material_resources[model_index].materials);
            free(model_groups.records[group_index].material_resources[model_index].data);
            memset(&model_groups.records[group_index].material_resources[model_index], 0, sizeof(model_groups.records[group_index].material_resources[model_index]));
            free(model_groups.records[group_index].model_vertices[model_index]);
            model_groups.records[group_index].model_vertices[model_index] = 0;
            free(model_groups.records[group_index].model_polygons[model_index]);
            model_groups.records[group_index].model_polygons[model_index] = 0;
            free(model_groups.records[group_index].model_data[model_index]);
            model_groups.records[group_index].model_data[model_index] = 0;
            model_groups.records[group_index].model_bytes[model_index] = 0;
            free(model_groups.records[group_index].animation_data[model_index]);
            if (model_groups.records[group_index].animation_headers[model_index] != 0)
            {
                for (uint32 header_index = 0; header_index < 1; ++header_index)
                {
                    for (uint32 track_group = 0; track_group < 4; ++track_group)
                        free(model_groups.records[group_index].animation_headers[model_index][header_index].tracks[track_group].bytes);
                }
            }
            free(model_groups.records[group_index].animation_headers[model_index]);
            model_groups.records[group_index].animation_data[model_index] = 0;
            model_groups.records[group_index].animation_headers[model_index] = 0;
            model_groups.records[group_index].animation_bytes[model_index] = 0;
        }
    }
    memset(&model_groups, 0, sizeof(model_groups));
    for (uint32 model_index = 0; model_index < CharacterController::ModelCount; ++model_index)
    {
        free(character_model_headers[model_index].material_resource.textures);
        free(character_model_headers[model_index].material_resource.materials);
        free(character_model_headers[model_index].material_resource.data);
        memset(&character_model_headers[model_index].material_resource, 0, sizeof(character_model_headers[model_index].material_resource));
        free(character_model_headers[model_index].model_vertices);
        character_model_headers[model_index].model_vertices = 0;
        free(character_model_headers[model_index].model_polygons);
        character_model_headers[model_index].model_polygons = 0;
        free(character_model_headers[model_index].model_data);
        character_model_headers[model_index].model_data = 0;
        character_model_headers[model_index].model_bytes = 0;
        free(character_model_headers[model_index].animation_data);
        if (character_model_headers[model_index].animation_headers != 0)
        {
            for (uint32 header_index = 0; header_index < 38; ++header_index)
            {
                for (uint32 track_group = 0; track_group < 4; ++track_group)
                    free(character_model_headers[model_index].animation_headers[header_index].tracks[track_group].bytes);
            }
        }
        free(character_model_headers[model_index].animation_headers);
        free(character_model_headers[model_index].morph_restoration);
        character_model_headers[model_index].animation_data = 0;
        character_model_headers[model_index].animation_headers = 0;
        character_model_headers[model_index].morph_restoration = 0;
        character_model_headers[model_index].animation_bytes = 0;
    }
    for (uint32 model_index = 0; model_index < sizeof(actor_models.definitions) / sizeof(actor_models.definitions[0]); ++model_index)
    {
        free(actor_models.definitions[model_index].material_resource.textures);
        free(actor_models.definitions[model_index].material_resource.materials);
        free(actor_models.definitions[model_index].material_resource.data);
        memset(&actor_models.definitions[model_index].material_resource, 0, sizeof(actor_models.definitions[model_index].material_resource));
        free(actor_models.definitions[model_index].model_vertices);
        actor_models.definitions[model_index].model_vertices = 0;
        free(actor_models.definitions[model_index].model_polygons);
        actor_models.definitions[model_index].model_polygons = 0;
        free(actor_models.definitions[model_index].model_data);
        actor_models.definitions[model_index].model_data = 0;
        actor_models.definitions[model_index].model_bytes = 0;
        free(actor_models.definitions[model_index].animation_data);
        if (actor_models.definitions[model_index].animation_headers != 0)
        {
            for (uint32 header_index = 0; header_index < 14; ++header_index)
            {
                for (uint32 track_group = 0; track_group < 4; ++track_group)
                    free(actor_models.definitions[model_index].animation_headers[header_index].tracks[track_group].bytes);
            }
        }
        free(actor_models.definitions[model_index].animation_headers);
        free(actor_models.definitions[model_index].morph_restoration);
    }
    for (uint32 model_index = 0; model_index < sizeof(animated_models.records) / sizeof(animated_models.records[0]); ++model_index)
    {
        free(animated_models.records[model_index].animation_data);
        if (animated_models.records[model_index].animation_headers != 0)
        {
            for (uint32 header_index = 0; header_index < 2; ++header_index)
            {
                for (uint32 track_group = 0; track_group < 4; ++track_group)
                    free(animated_models.records[model_index].animation_headers[header_index].tracks[track_group].bytes);
            }
        }
        free(animated_models.records[model_index].animation_headers);
        free(animated_models.records[model_index].morph_restoration);
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
        free(scene_stream.buffers[slot].section.materials.textures);
        free(scene_stream.buffers[slot].section.materials.materials);
        free(scene_stream.buffers[slot].section.materials.data);
        memset(&scene_stream.buffers[slot].section.materials, 0, sizeof(scene_stream.buffers[slot].section.materials));
        free(scene_stream.buffers[slot].section.lighting_records);
        for (ActorBinding *binding = scene_stream.buffers[slot].section.actor_bindings; binding != 0; binding = binding->next)
            free(binding->rotation_track);
        free(scene_stream.buffers[slot].section.actor_bindings);
        free(scene_stream.buffers[slot].section.animated_paths);
        free(scene_stream.buffers[slot].section.lights);
        free(scene_stream.buffers[slot].section.meshes);
        free(scene_stream.buffers[slot].section.face_storage);
        free(scene_stream.buffers[slot].section.vertex_storage);
        free(scene_stream.buffers[slot].section.collision_storage);
        free(scene_stream.buffers[slot].section.material_animation_words);
        free(scene_stream.buffers[slot].section.animation_payload_data);
        if (scene_stream.buffers[slot].section.animations != 0)
        {
            for (uint32 header_index = 0; header_index < scene_stream.buffers[slot].section.animation_count; ++header_index)
            {
                for (uint32 track_group = 0; track_group < 4; ++track_group)
                    free(scene_stream.buffers[slot].section.animations[header_index].tracks[track_group].bytes);
            }
        }
        free(scene_stream.buffers[slot].section.animations);
        free(scene_stream.buffers[slot].section.billboards);
        free(scene_stream.buffers[slot].section.mesh_parts);
        free(scene_stream.buffers[slot].section.sphere_nodes);
        free(scene_stream.buffers[slot].section.placements);
        free(scene_stream.buffers[slot].section.animated_objects);
        free(scene_stream.buffers[slot].section.sprites);
        for (uint32 track_index = 0; track_index < scene_stream.buffers[slot].section.sprite_extent_track_count; ++track_index)
            free(scene_stream.buffers[slot].section.sprite_extent_tracks[track_index]);
        free(scene_stream.buffers[slot].section.sprite_extent_tracks);
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
        scene_stream.buffers[slot].section.collision_storage = 0;
        scene_stream.buffers[slot].section.collision_count = 0;
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
        scene_stream.buffers[slot].section.sprites = 0;
        scene_stream.buffers[slot].section.sprite_count = 0;
        scene_stream.buffers[slot].section.sprite_extent_tracks = 0;
        scene_stream.buffers[slot].section.sprite_extent_track_count = 0;
        scene_stream.buffers[slot].section.triggers = 0;
        scene_stream.buffers[slot].section.trigger_count = 0;
        scene_stream.buffers[slot].section.path_names = 0;
        scene_stream.buffers[slot].section.path_name_count = 0;
        scene_stream.buffers[slot].section.paths = 0;
        scene_stream.buffers[slot].section.path_count = 0;
        scene_stream.buffers[slot].section.path_key_data = 0;
        scene_stream.buffers[slot].section.actor_bindings = 0;
        scene_stream.buffers[slot].section.animated_paths = 0;
        scene_stream.buffers[slot].section.lighting_records = NULL;
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
        free((*interface_resources).textures);
        free((*interface_resources).materials);
        free((*interface_resources).data);
        memset(&(*interface_resources), 0, sizeof((*interface_resources)));
    }
    interface_resources = 0;
    memset(&material_registry, 0, sizeof(material_registry));
    memset(&saved_material_registry, 0, sizeof(saved_material_registry));
    free(sprite_pool.records);
    memset(&sprite_pool, 0, sizeof(sprite_pool));
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
    p2_reset_callback();
    if (resource_memory.arena == 0)
    {
        uint32 heap_start = mallocPSX(4);
        freePSX(heap_start);
        p2_func_8008346c();
        psx_bios_init_user_heap(heap_start, 0x801FF000u - heap_start);
        p2_func_8008347c();
    }
    InitGeom();
    xport_gte_write_control(24, 160u << 16);
    xport_gte_write_control(25, 120u << 16);
    InitGameSound();
    InitializeRenderBuffers();
    CdInit();
    InitializeGameClock();
    InitializeGamepad();
    resource_memory.arena = AllocateGameMemory(1254400);
    resource_archive.LoadIndex();
    resource_archive.ReadAsset(22, startup_image, 1);
    screen_render.startup_image_loaded = 0;
    DrawStartupImage(startup_image);
    ++screen_render.startup_image_loaded;
    resource_archive.ReadAsset(0, behavior_bundle, 1);
    resource_load_cursor.asset = 1;
    LoadVolleyScripts((const uint8 *)behavior_bundle);
    LoadAttackScripts((const uint8 *)behavior_bundle);
    resource_archive.ReadAsset(3, behavior_bundle, 1);
    resource_load_cursor.asset = 4;
    LoadObjectSpeedDefinitions((const uint8 *)behavior_bundle);
    loop_bytes = resource_archive.Size(17);
    if (loop_bytes > 0xFFFFF800u)
    {
        fprintf(stderr, "P2: Invalid loop bundle size %u\n", loop_bytes);
        abort();
    }
    loop_capacity = (loop_bytes + 2047) & ~2047u;
    loop_bundle = (uint8 *)malloc(loop_capacity != 0 ? loop_capacity : 1);
    if (loop_bundle == NULL)
    {
        fprintf(stderr, "P2: Cannot allocate loop bundle\n");
        abort();
    }
    resource_archive.ReadAsset(17, loop_bundle, 1);
    LoadLoopSequence(loop_bundle, loop_bytes);
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
    if (startup_bundle_bytes == 0 || startup_bundle_bytes > 0xFFFFF800u)
    {
        fprintf(stderr, "P2: Invalid startup bundle size %u\n", startup_bundle_bytes);
        abort();
    }
    startup_bundle_capacity = (startup_bundle_bytes + 2047u) & ~2047u;
    startup_bundle = (uint8 *)malloc(startup_bundle_capacity);
    if (startup_bundle == NULL)
    {
        fprintf(stderr, "P2: Cannot allocate startup bundle\n");
        abort();
    }
    resource_archive.ReadAsset(asset, startup_bundle, 1);
    offset = resource_archive.Offset(251);
    {
        uint32 image_index = 0;
        uint32 image_bytes = (uint32)OriginalDefaults::TextureImageRects[0].w * (uint32)OriginalDefaults::TextureImageRects[0].h * 2;
        uint32 second_offset = resource_archive.Offset(252);
        uint32 second_image_bytes = (uint32)OriginalDefaults::TextureImageRects[8].w * (uint32)OriginalDefaults::TextureImageRects[8].h * 2;
        if (offset > startup_bundle_bytes || second_offset > startup_bundle_bytes - offset || second_image_bytes > startup_bundle_bytes - offset - second_offset)
        {
            fprintf(stderr, "P2: Startup secondary image is outside bundle\n");
            free(startup_bundle);
            abort();
        }
        second_image_bytes += second_offset;
        if (image_bytes < second_image_bytes)
            image_bytes = second_image_bytes;
        if (image_bytes > startup_bundle_bytes - offset)
        {
            fprintf(stderr, "P2: Startup image is outside bundle\n");
            free(startup_bundle);
            abort();
        }
        UploadTextureImage(startup_bundle + offset, image_index, 1);
    }
    if (game_flow.mode_flags & 0x200)
    {
        free(startup_bundle);
        startup_bundle_bytes = resource_archive.Size(250);
        if (startup_bundle_bytes == 0 || startup_bundle_bytes > 0xFFFFF800u)
        {
            fprintf(stderr, "P2: Invalid startup sound bundle size %u\n", startup_bundle_bytes);
            abort();
        }
        startup_bundle_capacity = (startup_bundle_bytes + 2047u) & ~2047u;
        startup_bundle = (uint8 *)malloc(startup_bundle_capacity);
        if (startup_bundle == NULL)
        {
            fprintf(stderr, "P2: Cannot allocate startup sound bundle\n");
            abort();
        }
        resource_archive.ReadAsset(250, startup_bundle, 1);
    }
    SetSoundTransferAddress(253, 0);
    offset = resource_archive.Offset(253);
    sound_size = resource_archive.Size(253);
    {
        uint32 transferred_bytes = sound_size > 0x7EFF0 ? 0x7EFF0 : sound_size;
        if (offset > startup_bundle_bytes || transferred_bytes > startup_bundle_bytes - offset)
        {
            fprintf(stderr, "P2: Startup VB is outside sound bundle\n");
            free(startup_bundle);
            abort();
        }
    }
    sound_data = startup_bundle + offset;
    UploadSoundSamples(sound_data, sound_size);
    SpuIsTransferCompleted(1);
    free(startup_bundle);
    ResetSceneRendering();
    LoadCommonGameResources();
    game_flow.mode_flags = 0;
    if ((mode << 24) != 0)
        ReloadMenuArchive();
}
