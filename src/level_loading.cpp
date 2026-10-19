#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "game.h"
#include "original_defaults.h"

LevelLoader level_loader = {0};

uint32 LoadLevel(sint32 level)
{
    uint32 player;
    uint32 previous;
    CharacterPreview preview;
    FramePacer pacer = {30};
    uint32 result;
    const TextureDescriptor *sprite;
    uint32 position;
    uint32 first_x;
    uint32 first_y;
    uint32 last_x;
    uint32 last_y;
    uint32 copy;
    FUNCTION_MARKER(0x8002DCD0, "SLUS_005.78");
    ResetHud();
    ResetCutsceneCameras();
    InitializeContactPool();
    player = OriginalDefaults::SceneContextIdentity;
    camera_motion.target_ticks = 0;
    camera_motion.smooth_ticks[2] = (uint8)(0);
    camera_motion.smooth_ticks[1] = (uint8)(0);
    camera_motion.smooth_ticks[0] = (uint8)(0);
    previous = frame_clock.milliseconds;
    preview.velocity[0] = (uint32)-4096;
    preview_playback.tick = 0xFFFFFFFF;
    player_interaction.flags = 0;
    player_interaction.target = 0;
    preview.position[0] = 143360;
    preview.position[1] = (uint32)-61440;
    preview.position[2] = 122880;
    preview.velocity[2] = 0;
    preview.velocity[1] = 0;
    while ((PollLevelLoad(level) << 24) == 0)
    {
        preview.Update();
        BeginDraw();
        preview.Draw();
        ClearSceneOrderingTable();
        DrawBackgroundRectangle(0, 0xF00140, 0);
        sprite = resource_groups[2].texture;
        for (copy = 0; copy < 2; ++copy)
        {
            last_x = sprite->payload.bytes[2];
            first_x = sprite->payload.bytes[0];
            last_y = sprite->payload.bytes[3];
            first_y = sprite->payload.bytes[1];
            position = (uint32)((319 - ((sint32)last_x - (sint32)first_x)) >> 1);
            position |= (uint32)((239 - ((sint32)last_y - (sint32)first_y)) >> 1) << 16;
            DrawTexturedQuad(position, 0, sprite, 0);
        }
        UpdateResourceMaterialAnimations();
        EndDraw();
        previous = pacer.Wait(previous);
    }
    result = ActivateSceneSection();
    return result;
}

uint32 PollLevelLoad(sint32 level)
{
    FUNCTION_MARKER(0x8002D7F8, "SLUS_005.78");
    const LevelDefinition *table;
    uint32 entry;
    uint32 active;
    uint32 asset;
    uint32 other;
    uint32 destination;
    uint32 bytes;
    uint32 header_bytes;
    uint8 *header_input;
    uint32 index;
    LevelResource *resource;
    const uint8 *prefix;
    const uint8 *section_data;
    SectionDefinition *section;
    sint32 state;
    state = level_loader.phase;
    switch (state)
    {
        case 0:
            while (PollSceneResourceLoad() != 0)
                ;
            if (level == 0)
            {
                game_flow.progress.hub_spawn_consumed = 0;
                gameplay_input.prev_buttons |= 0x40;
            }
            else if ((game_flow.transition_flags & 2) == 0 && (uint32)level - 96 >= 2)
                game_flow.progress.selected_stage = (uint8)MapLevelProgression((uint32)level, 1);
            sound_effects.Stop(1);
            entry = OriginalDefaults::Levels[0].level_id;
            level_loader.definition = OriginalDefaults::Levels;
            if (entry != (uint32)level)
            {
                do
                {
                    table = level_loader.definition;
                    entry = table[1].level_id;
                    level_loader.definition = table + 1;
                } while (entry != 0xFFFF && entry != (uint32)level);
            }
            active = 3;
            if (level_loader.definition->preload_both_characters == 0)
            {
                active = 2;
                if (GetCurrentCharacterForm() == 0)
                    active = 1;
            }
            if (active != character_controller.loaded_model_mask)
            {
                ReloadBaseCharacterModels(active);
                character_controller.loaded_model_mask = active;
            }
            ResetSceneStreams();
            table = level_loader.definition;
            asset = table->resource_asset;
            header_bytes = resource_archive.Size(asset + 1);
            if (header_bytes < 280)
            {
                fprintf(stderr, "P2: Truncated level resource header\n");
                abort();
            }
            header_input = (uint8 *)malloc(header_bytes);
            if (header_input == 0)
            {
                fprintf(stderr, "P2: Cannot allocate level resource header\n");
                abort();
            }
            resource_archive.ReadAsset(asset + 1, header_input, 0, asset);
            asset = level_loader.definition->resource_asset;
            prefix = header_input;
            for (index = 0; index < 10; ++index)
            {
                section = level_resource.sections + index;
                section_data = prefix + index * 24;
                section->flags = section_data[0];
                section->metadata = section_data[1];
                section->transition_ticks = xport_load_le16(section_data + 2);
                section->checkpoint = xport_load_le32(section_data + 4);
                section->origin[0] = xport_load_le32(section_data + 8);
                section->origin[1] = xport_load_le32(section_data + 12);
                section->origin[2] = xport_load_le32(section_data + 16);
                section->metadata_word = xport_load_le32(section_data + 20);
            }
            level_resource.asset_base = xport_load_le32(prefix + 240);
            level_resource.actor_masks[0] = xport_load_le32(prefix + 244);
            level_resource.actor_masks[1] = xport_load_le32(prefix + 248);
            level_resource.animated_model_mask = xport_load_le32(prefix + 252);
            level_resource.model_group_mask = xport_load_le32(prefix + 256);
            level_resource.character_mask = xport_load_le16(prefix + 260);
            level_resource.level_id = xport_load_le16(prefix + 262);
            level_resource.active_sections = prefix[264];
            level_resource.section_count = prefix[265];
            level_resource.checkpoint = xport_load_le16(prefix + 266);
            level_resource.transition_ticks = xport_load_le16(prefix + 268);
            level_resource.treasure_total = xport_load_le16(prefix + 270);
            level_resource.collected_total = xport_load_le16(prefix + 272);
            level_resource.metadata_halfword = xport_load_le16(prefix + 274);
            level_resource.audio_mode = xport_load_le32(prefix + 276);
            free(header_input);
            level_loader.phase = 1;
            level_resource.asset_base = asset;
            break;
        case 1:
            if (((uint32)resource_archive.Poll() << 24) == 0)
                break;
            table = level_loader.definition;
            asset = table->sound_asset;
            other = table->resource_asset;
            resource_load_cursor.container_asset = other;
            resource_load_cursor.asset = other + 2;
            SetSoundTransferAddress(asset, 1);
            level_resource.LoadAnimatedModels();
            level_resource.LoadModelGroups();
            level_resource.LoadActorModels();
            level_resource.LoadCharacterModels();
            if (level_resource.level_id != 0 && (sint32)resource_archive.Size(level_loader.definition->image_asset) > 0)
            {
                asset = level_loader.definition->image_asset;
                level_loader.transfer_bytes = resource_archive.Size(asset);
                if (level_loader.transfer_bytes > 0xFFFFF800u)
                {
                    fprintf(stderr, "P2: Level transfer asset exceeds native capacity\n");
                    abort();
                }
                uint32 transfer_capacity = (level_loader.transfer_bytes + 2047u) & ~2047u;
                level_loader.transfer_data = (uint8 *)malloc(transfer_capacity != 0 ? transfer_capacity : 1);
                if (level_loader.transfer_data == 0)
                {
                    fprintf(stderr, "P2: Cannot allocate level transfer input\n");
                    abort();
                }
                resource_archive.ReadAsset(asset, level_loader.transfer_data, 0);
                level_loader.phase = 2;
                break;
            }
            goto load_sound;
        case 2:
            if (((uint32)resource_archive.Poll() << 24) != 0)
            {
                {
                    uint32 image_index = 1;
                    uint32 image_bytes = (uint32)OriginalDefaults::TextureImageRects[image_index].w * (uint32)OriginalDefaults::TextureImageRects[image_index].h * 2;
                    if (image_index == 0)
                    {
                        uint32 second_image_bytes = resource_archive.Offset(252) + (uint32)OriginalDefaults::TextureImageRects[8].w * (uint32)OriginalDefaults::TextureImageRects[8].h * 2;
                        if (image_bytes < second_image_bytes)
                            image_bytes = second_image_bytes;
                    }
                    if (image_bytes > level_loader.transfer_bytes)
                    {
                        fprintf(stderr, "P2: Truncated level texture input\n");
                        abort();
                    }
                    UploadTextureImage(level_loader.transfer_data, image_index, 0);
                }
                level_loader.phase = 3;
            }
            break;
        case 3:
            if (DrawSync(1) > 0)
                break;
        load_sound:
        {
            free(level_loader.transfer_data);
            level_loader.transfer_data = 0;
            level_loader.transfer_bytes = 0;
            asset = level_loader.definition->sound_asset;
            level_loader.transfer_bytes = resource_archive.Size(asset);
            if (level_loader.transfer_bytes > 0xFFFFF800u)
            {
                fprintf(stderr, "P2: Level transfer asset exceeds native capacity\n");
                abort();
            }
            uint32 transfer_capacity = (level_loader.transfer_bytes + 2047u) & ~2047u;
            level_loader.transfer_data = (uint8 *)malloc(transfer_capacity != 0 ? transfer_capacity : 1);
            if (level_loader.transfer_data == 0)
            {
                fprintf(stderr, "P2: Cannot allocate level transfer input\n");
                abort();
            }
            resource_archive.ReadAsset(asset, level_loader.transfer_data, 0);
            level_loader.phase = 4;
        }
        break;
        case 4:
            if (((uint32)resource_archive.Poll() << 24) != 0)
            {
                asset = level_loader.definition->sound_asset;
                bytes = resource_archive.Size(asset);
                UploadSoundSamples(level_loader.transfer_data, bytes);
                level_loader.phase = 5;
            }
            break;
        case 5:
            if (p2_func_8006aaa8() == 0)
                break;
            free(level_loader.transfer_data);
            level_loader.transfer_data = 0;
            level_loader.transfer_bytes = 0;
            if (level_resource.level_id == 0)
            {
                index = resource_load_cursor.asset;
                resource_load_cursor.asset = index + 1;
                uint32 name_bytes = resource_archive.Size(index);
                if (name_bytes < 40 * LevelResource::WorldNameCount)
                {
                    fprintf(stderr, "P2: Truncated world name table\n");
                    abort();
                }
                uint8 *name_input = (uint8 *)malloc(name_bytes);
                if (name_input == 0)
                {
                    fprintf(stderr, "P2: Cannot allocate world name table\n");
                    abort();
                }
                resource_archive.ReadAsset(index, name_input, 1, resource_load_cursor.container_asset);
                CopyBytes((uint8 *)level_resource.world_names, name_input, 40 * LevelResource::WorldNameCount);
                free(name_input);
            }
            if ((sint32)resource_archive.Offset(resource_load_cursor.asset) > 0)
            {
                uint32 bytes = resource_archive.Size(resource_load_cursor.asset);
                if (bytes < sizeof(SoundTableHeader))
                {
                    fprintf(stderr, "P2: Truncated scene sound table\n");
                    abort();
                }
                uint8 *sound_input = (uint8 *)malloc(bytes);
                if (sound_input == 0)
                {
                    fprintf(stderr, "P2: Cannot allocate native sound table input\n");
                    abort();
                }
                resource_archive.ReadAsset(resource_load_cursor.asset, sound_input, 1, resource_load_cursor.container_asset);
                const uint8 *source = sound_input;
                uint32 count = xport_load_le32(source);
                if ((sint32)count > 0 && (count > SoundBankStorage::SampleCapacity || count > (bytes - sizeof(SoundTableHeader)) / sizeof(SoundSample)))
                {
                    fprintf(stderr, "P2: Scene sound samples exceed table capacity\n");
                    abort();
                }
                sound_resources.scene_bank.header.count = count;
                if ((sint32)count > 0)
                {
                    for (uint32 sound_index = 0; sound_index < count; ++sound_index)
                    {
                        const uint8 *record = source + sizeof(SoundTableHeader) + sound_index * sizeof(SoundSample);
                        SoundSample *sample = sound_resources.scene_bank.samples + sound_index;
                        sample->spu_address = xport_load_le32(record);
                        sample->pitch = xport_load_le16(record + 4);
                        sample->metadata = xport_load_le16(record + 6);
                    }
                }
                sound_resources.scene_table = sound_resources.scene_bank.header.Relocate(bytes, sound_resources.scene_bank.samples) != 0 ? &sound_resources.scene_bank : 0;
                free(sound_input);
            }
            table = level_loader.definition;
            index = resource_load_cursor.asset;
            resource = &level_resource;
            asset = table->sound_asset;
            resource_load_cursor.asset = index + 1;
            resource->collected_total = 0;
            resource->active_sections = 0;
            resource->asset_base = asset + 1;
            FillBytes(scene_events.script_values + 50, 0, 25);
            destination = 0;
            if (level_resource.level_id == 0)
                destination = OriginalDefaults::HubSections[(sint8)game_flow.progress.selected_stage];
            RequestSceneSection(destination);
            level_loader.phase = 6;
            break;
        default:
            if (state == 6 && PollSceneResourceLoad() == 0)
                level_loader.phase = 0;
            break;
    }
    return level_loader.phase == 0;
}

sint32 PollLevelLoadResult(sint32 level)
{
    FUNCTION_MARKER(0x8002DEB8, "SLUS_005.78");
    return (sint8)PollLevelLoad(level);
}

uint32 UnloadLevelResources(void)
{
    FUNCTION_MARKER(0x8002DEDC, "SLUS_005.78");
    scene_events.world_name_index = 0;
    ReleaseSceneObjects();
    scene_stream.active_section = 0;
    scene_lighting.lights = 0;
    sound_effects.Stop(1);
    if (sequence_playback.current_handle != 0)
    {
        StopCurrentSequence();
        CloseSequence(sequence_playback.current_handle);
    }
    sequence_playback.current_handle = 0;
    if (sequence_playback.loading_phase != 0)
    {
        if (sequence_playback.loading_phase == 1 && SpuIsTransferCompleted(1) != 1)
        {
            fprintf(stderr, "P2: Cannot complete cancelled sequence transfer\n");
            abort();
        }
        p2_sdk_80079b14((sint16)sequence_playback.loading_bank);
        free(sequence_playback.loading_samples);
        sequence_playback.loading_samples = NULL;
        sequence_playback.loading_sample_bytes = 0;
        sequence_playback.loading_bank = (uint16)OriginalDefaults::SequenceLoadingBank;
        sequence_playback.loading_phase = 0;
    }
    if (level_loader.transfer_data != 0)
    {
        if (level_loader.phase == 3)
        {
            while (DrawSync(1) > 0)
                ;
        }
        if (level_loader.phase == 5 && SpuIsTransferCompleted(1) != 1)
        {
            fprintf(stderr, "P2: Cannot complete cancelled level transfer\n");
            abort();
        }
        free(level_loader.transfer_data);
        level_loader.transfer_data = 0;
        level_loader.transfer_bytes = 0;
    }
    if (scene_loader.phase != 0 || resource_archive.transfer_pending)
        resource_archive.Poll();
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
    scene_loader.phase = 0;
    scene_loader.sequence_bundle = 0;
    scene_loader.destination_buffer = 0;
    scene_loader.background = 0;
    for (uint32 slot = 0; slot < SceneStream::SequenceBufferCount; ++slot)
    {
        free(scene_stream.sequence_bundles[slot].data);
        scene_stream.sequence_bundles[slot].data = NULL;
        scene_stream.sequence_bundles[slot].bytes = 0;
    }
    for (uint32 slot = 0; slot < 2; ++slot)
    {
        BackgroundResource *background = &scene_stream.backgrounds[slot];
        free(background->data);
        free(background->tiles);
        memset(background, 0, sizeof(*background));
        free(scene_stream.buffers[slot].section.lighting_records);
        scene_stream.buffers[slot].section.lighting_records = NULL;
        scene_stream.buffers[slot].section.lighting_count = 0;
        scene_stream.buffers[slot].section.data_bytes = 0;
    }
    render_view.lighting_cache = NULL;
    render_view.lighting_enabled = false;
    ResetCharacterModels();
    ResetActorModels();
    ResetAnimatedModels();
    RetainCommonModelGroups();
    material_registry = saved_material_registry;
    return material_registry.reserved[4];
}
