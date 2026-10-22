#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "level_loader.h"
#include "game.h"

LevelLoader level_loader = {0};

uint32 LevelLoader::PollLevelLoad(sint32 level)
{
    FUNCTION_MARKER(0x8002D7F8, "SLUS_005.78");
    const LevelDefinition *table;
    uint32 entry;
    uint32 active;
    uint32 asset;
    uint32 other;
    uint32 dst;
    uint32 bytes;
    uint32 header_bytes;
    uint8 *header_input;
    uint32 idx;
    LevelResource *resource;
    const EncodedLevelHeader *prefix;
    const SectionDefinition *section_data;
    SectionDefinition *section;
    sint32 state;
    state = level_loader.phase;
    switch (state)
    {
        case 0:
            while (SceneLoader::PollSceneResourceLoad() != 0)
                ;
            if (level == 0)
            {
                game_flow.progress.hub_spawn_consumed = 0;
                gameplay_input.prev_buttons |= 0x40;
            }
            else if ((game_flow.transition_flags & 2) == 0 && (uint32)level - 96 >= 2)
                game_flow.progress.sel_stage = GameFlowState::MapLevelProgression(level, 1);
            sound_effects.Stop(1);
            entry = OriginalDefaults::levels[0].level_id;
            level_loader.definition = OriginalDefaults::levels;
            if (entry != level)
            {
                do
                {
                    table = level_loader.definition;
                    entry = table[1].level_id;
                    level_loader.definition = table + 1;
                } while (entry != 0xFFFF && entry != level);
            }
            active = 3;
            if (level_loader.definition->preload_both_characters == 0)
            {
                active = 2;
                if (CharacterController::GetCurCharacterForm() == 0)
                    active = 1;
            }
            if (active != character_pad.loaded_model_mask)
            {
                CharacterModelRecord::ReloadBaseCharacterModels(active);
                character_pad.loaded_model_mask = active;
            }
            SceneStream::ResetSceneStreams();
            table = level_loader.definition;
            asset = table->resource_asset;
            header_bytes = resource_archive.Size(asset + 1);
            header_input = (uint8 *)malloc(header_bytes);
            resource_archive.ReadAsset(asset + 1, header_input, 0, asset);
            asset = level_loader.definition->resource_asset;
            prefix = (const EncodedLevelHeader *)header_input;
            for (idx = 0; idx < 10; ++idx)
            {
                section = level_resource.sections + idx;
                section_data = prefix->sections + idx;
                section->flags = section_data->flags;
                section->respawn_form = section_data->respawn_form;
                section->transition_ticks = section_data->transition_ticks;
                section->checkpoint = section_data->checkpoint;
                section->far_distance = section_data->far_distance;
                section->fog_near = section_data->fog_near;
                section->fog_far = section_data->fog_far;
                section->metadata_word = section_data->metadata_word;
            }
            level_resource.asset_base = prefix->asset_base;
            level_resource.actor_masks[0] = prefix->actor_masks[0];
            level_resource.actor_masks[1] = prefix->actor_masks[1];
            level_resource.animated_model_mask = prefix->animated_model_mask;
            level_resource.model_grp_mask = prefix->model_grp_mask;
            level_resource.character_mask = prefix->character_mask;
            level_resource.level_id = prefix->level_id;
            level_resource.active_sections = prefix->active_sections;
            level_resource.section_count = prefix->section_count;
            level_resource.checkpoint = prefix->checkpoint;
            level_resource.transition_ticks = prefix->transition_ticks;
            level_resource.treasure_total = prefix->treasure_total;
            level_resource.collected_total = prefix->collected_total;
            level_resource.metadata_halfword = prefix->metadata_halfword;
            level_resource.audio_mode = prefix->audio_mode;
            free(header_input);
            level_loader.phase = 1;
            level_resource.asset_base = asset;
            break;
        case 1:
            if (!resource_archive.Poll())
                break;
            table = level_loader.definition;
            asset = table->sound_asset;
            other = table->resource_asset;
            resource_load_cursor.container_asset = other;
            resource_load_cursor.asset = other + 2;
            SoundTableHeader::SetSoundTransferAddress(asset, 1);
            level_resource.LoadAnimatedModels();
            level_resource.LoadModelGrps();
            level_resource.LoadActorModels();
            level_resource.LoadCharacterModels();
            if (level_resource.level_id != 0 && (sint32)resource_archive.Size(level_loader.definition->image_asset) > 0)
            {
                asset = level_loader.definition->image_asset;
                level_loader.transfer_bytes = resource_archive.Size(asset);
                uint32 transfer_capacity = (level_loader.transfer_bytes + 2047u) & ~2047u;
                level_loader.transfer_data = (uint8 *)malloc(transfer_capacity != 0 ? transfer_capacity : 1);
                resource_archive.ReadAsset(asset, level_loader.transfer_data, 0);
                level_loader.phase = 2;
                break;
            }
            goto load_sound;
        case 2:
            if (resource_archive.Poll())
            {
                ScreenRenderState::UploadTexImage(level_loader.transfer_data, 1, 0);
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
            uint32 transfer_capacity = (level_loader.transfer_bytes + 2047u) & ~2047u;
            level_loader.transfer_data = (uint8 *)malloc(transfer_capacity != 0 ? transfer_capacity : 1);
            resource_archive.ReadAsset(asset, level_loader.transfer_data, 0);
            level_loader.phase = 4;
        }
        break;
        case 4:
            if (resource_archive.Poll())
            {
                asset = level_loader.definition->sound_asset;
                bytes = resource_archive.Size(asset);
                SoundTableHeader::UploadSoundSamples(level_loader.transfer_data, bytes);
                level_loader.phase = 5;
            }
            break;
        case 5:
            if (IsSoundTransferComplete() == 0)
                break;
            free(level_loader.transfer_data);
            level_loader.transfer_data = 0;
            level_loader.transfer_bytes = 0;
            if (level_resource.level_id == 0)
            {
                idx = resource_load_cursor.asset;
                resource_load_cursor.asset = idx + 1;
                uint32 name_bytes = resource_archive.Size(idx);
                uint8 *name_input = (uint8 *)malloc(name_bytes);
                resource_archive.ReadAsset(idx, name_input, 1, resource_load_cursor.container_asset);
                CopyBytes((uint8 *)level_resource.world_names, name_input, 40 * LevelResource::WorldNameCount);
                free(name_input);
            }
            if ((sint32)resource_archive.Offset(resource_load_cursor.asset) > 0)
            {
                uint32 bytes = resource_archive.Size(resource_load_cursor.asset);
                uint8 *sound_input = (uint8 *)malloc(bytes);
                resource_archive.ReadAsset(resource_load_cursor.asset, sound_input, 1, resource_load_cursor.container_asset);
                const uint8 *src = sound_input;
                uint32 count = ((const EncodedSoundHeader *)src)->count;
                sound_resources.scene_bank.header.count = count;
                if ((sint32)count > 0)
                {
                    for (uint32 sound_idx = 0; sound_idx < count; ++sound_idx)
                    {
                        const uint8 *rec = src + sizeof(SoundTableHeader) + sound_idx * sizeof(SoundSample);
                        SoundSample *sample = sound_resources.scene_bank.samples + sound_idx;
                        const EncodedSoundSample *input_sample = (const EncodedSoundSample *)rec;
                        sample->spu_address = input_sample->spu_address;
                        sample->pitch = input_sample->pitch;
                        sample->metadata = input_sample->metadata;
                    }
                }
                sound_resources.scene_table = sound_resources.scene_bank.header.Relocate(bytes, sound_resources.scene_bank.samples) != 0 ? &sound_resources.scene_bank : 0;
                free(sound_input);
            }
            table = level_loader.definition;
            idx = resource_load_cursor.asset;
            resource = &level_resource;
            asset = table->sound_asset;
            resource_load_cursor.asset = idx + 1;
            resource->collected_total = 0;
            resource->active_sections = 0;
            resource->asset_base = asset + 1;
            FillBytes(scene_events.script_vals + 50, 0, 25);
            dst = 0;
            if (level_resource.level_id == 0)
                dst = OriginalDefaults::hub_sections[(sint8)game_flow.progress.sel_stage];
            SceneLoader::RequestSceneSection(dst);
            level_loader.phase = 6;
            break;
        default:
            if (state == 6 && SceneLoader::PollSceneResourceLoad() == 0)
                level_loader.phase = 0;
            break;
    }
    return level_loader.phase == 0;
}

void LevelLoader::LoadLevel(sint32 level)
{
    uint32 prev;
    CharacterPreview preview;
    FramePacer pacer = {30};

    const TextureDescriptor *spr;
    uint32 pos;
    sint32 first_x;
    sint32 first_y;
    sint32 last_x;
    sint32 last_y;
    uint32 copy;
    FUNCTION_MARKER(0x8002DCD0, "SLUS_005.78");
    HudState::ResetHud();
    CutscenePlayback::ResetCutsceneCameras();
    CharacterContactPool::InitializeContactPool();

    camera_motion.target_ticks = 0;
    camera_motion.smooth_ticks[2] = 0;
    camera_motion.smooth_ticks[1] = 0;
    camera_motion.smooth_ticks[0] = 0;
    prev = frame_clock.milliseconds;
    preview.velocity.x = -4096;
    preview_playback.tick = 0xFFFFFFFF;
    player_interaction.flags = 0;
    player_interaction.target = 0;
    preview.pos.x = 143360;
    preview.pos.y = -61440;
    preview.pos.z = 122880;
    preview.velocity.z = 0;
    preview.velocity.y = 0;
    while ((LevelLoader::PollLevelLoad(level) << 24) == 0)
    {
        preview.Update();
        RenderCommands::BeginDraw();
        preview.Draw();
        RenderCommands::ClearSceneOrderingTable();
        ScreenRenderState::DrawBackgroundRect(0, 0xF00140, 0);
        spr = resource_grps[2].tex;
        for (copy = 0; copy < 2; ++copy)
        {
            last_x = spr->mapping.right;
            first_x = spr->mapping.left;
            last_y = spr->mapping.bottom;
            first_y = spr->mapping.top;
            pos = (319 - (last_x - first_x)) >> 1;
            pos |= (uint32)((239 - (last_y - first_y)) >> 1) << 16;
            MaterialCatalog::DrawTexturedQuad(pos, 0, spr, 0);
        }
        MaterialRegistry::UpdateResourceMaterialAnimations();
        RenderCommands::EndDraw();
        prev = pacer.Wait(prev);
    }
    SceneStream::ActivateSceneSection();
}

sint32 LevelLoader::PollLevelLoadResult(sint32 level)
{
    FUNCTION_MARKER(0x8002DEB8, "SLUS_005.78");
    return (sint8)LevelLoader::PollLevelLoad(level);
}

void LevelLoader::UnloadLevelResources(void)
{
    FUNCTION_MARKER(0x8002DEDC, "SLUS_005.78");
    scene_events.world_name_idx = 0;
    SceneStream::ReleaseSceneObjects();
    scene_stream.active_section = 0;
    scene_lighting.lights = 0;
    sound_effects.Stop(1);
    if (sequence_playback.cur_handle != 0)
    {
        SequencePlayback::StopCurSequence();
        SequencePlayback::CloseSequence(sequence_playback.cur_handle);
    }
    sequence_playback.cur_handle = 0;
    if (sequence_playback.loading_phase != 0)
    {
        CloseVabBank(sequence_playback.loading_bank);
        free(sequence_playback.loading_samples);
        sequence_playback.loading_samples = NULL;
        sequence_playback.loading_sample_bytes = 0;
        sequence_playback.loading_bank = OriginalDefaults::SequenceLoadingBank;
        sequence_playback.loading_phase = 0;
    }
    if (level_loader.transfer_data != 0)
    {
        if (level_loader.phase == 3)
        {
            while (DrawSync(1) > 0)
                ;
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
    scene_loader.phase = 0;
    scene_loader.sequence_bundle = 0;
    scene_loader.dst_buffer = 0;
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
        free(scene_stream.buffers[slot].section.lighting_recs);
        scene_stream.buffers[slot].section.lighting_recs = NULL;
        scene_stream.buffers[slot].section.lighting_count = 0;
        scene_stream.buffers[slot].section.data_bytes = 0;
    }
    render_view.lighting_cache = NULL;
    render_view.lighting_enabled = false;
    CharacterModelRecord::ResetCharacterModels();
    ActorModelRegistry::ResetActorModels();
    AnimatedModelRegistry::ResetAnimatedModels();
    ModelGroupRegistry::RetainCommonModelGrps();
    material_registry = saved_material_registry;
}
