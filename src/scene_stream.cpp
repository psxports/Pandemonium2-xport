#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "scene_stream.h"
#include "game.h"
#include "hd.h"

SceneStream scene_stream = {0};

void SceneStream::SetSceneTransition(SceneSection *src, SceneSection *scene, sint32 idx, uint32 flags)
{
    FUNCTION_MARKER(0x80025674, "SLUS_005.78");
    SceneStream *stream;
    SceneSection *section0, *section1;
    const SceneSection *scene_view;
    SceneSectionBuffer *buffer0, *buffer1, *replacement;
    uint32 val, coefficient, result;
    const SectionDefinition *rec;
    sint32 kind;
    stream = &scene_stream;
    stream->prev_section = scene;
    if (src != 0)
        stream->target_section = src;
    else
    {
        buffer0 = stream->cur_buffer;
        section0 = &buffer0->section;
        section0->state_flags = section0->state_flags & 0xFFF7;
        buffer0 = stream->cur_buffer;
        section0 = &buffer0->section;
        if ((section0->state_flags & 0x10) != 0)
        {
            buffer1 = stream->pending_buffer;
            section1 = buffer1 != 0 ? &buffer1->section : 0;
            if (buffer1 != 0)
            {
                kind = section1->section_idx;
                if (kind < section0->section_idx)
                {
                    section1->state_flags = section1->state_flags & 0xFFF7;
                    buffer1 = stream->pending_buffer;
                    section1 = buffer1 != 0 ? &buffer1->section : 0;
                    if ((section1->state_flags & 0x10) == 0)
                    {
                        buffer0 = stream->cur_buffer;
                        section0 = &buffer0->section;
                        stream->target_section = section1;
                        scene = stream->target_section;
                        replacement = buffer1;
                        stream->prev_section = section0;
                        stream->pending_buffer = buffer0;
                        stream->cur_buffer = replacement;
                    }
                }
            }
        }
        else
        {
            buffer1 = stream->pending_buffer;
            section1 = buffer1 != 0 ? &buffer1->section : 0;
            if (buffer1 != 0)
                section1->state_flags = section1->state_flags & 0xFFF7;
        }
    }
    stream->checkpoint = idx;
    if ((sint16)idx == -1)
        stream->transition_ticks = 10;
    else if ((sint16)idx == -2)
    {
        scene_view = scene;
        kind = scene_view->section_idx;
        rec = level_resource.sections + kind;
        val = rec->checkpoint;
        stream->checkpoint = val;
        coefficient = rec->transition_ticks;
        kind = scene_view->transition_scale;
        val = mul12(coefficient, kind);
        stream->transition_ticks = val;
    }
    else
        stream->transition_ticks = flags;
    result = game_flow.transition_flags;
    result |= 0x40;
    game_flow.transition_flags = result;
}

uint32 SceneStream::HasPendingSceneSection(void)
{
    FUNCTION_MARKER(0x80025B30, "SLUS_005.78");
    if (scene_stream.pending_buffer != 0)
        return 1;
    return level_resource.level_id != 0;
}

SceneSection *SceneStream::SwapSceneSections(void)
{
    FUNCTION_MARKER(0x80025B7C, "SLUS_005.78");
    SceneSectionBuffer *prev_buffer, *result;
    prev_buffer = scene_stream.cur_buffer;
    result = scene_stream.pending_buffer;
    scene_stream.pending_buffer = prev_buffer;
    scene_stream.cur_buffer = result;
    return result != 0 ? &result->section : 0;
}

uint32 SceneStream::ActivateSceneSection(void)
{
    FUNCTION_MARKER(0x80025BFC, "SLUS_005.78");
    SceneStream *stream = &scene_stream;
    SceneSectionBuffer *cur = stream->pending_buffer;
    SceneSectionBuffer *pending;
    sint32 idx = cur->section.section_idx;
    const SectionDefinition *parameters;
    uint32 val, flags, scale;
    sint32 factor;
    stream->cur_buffer = cur;
    parameters = level_resource.sections + idx;
    if (cur == &stream->buffers[0])
    {
        pending = &stream->buffers[1];
    }
    else
    {
        pending = &stream->buffers[0];
    }
    stream->pending_buffer = pending;
    pending->section.data = 0;
    pending->section.data_bytes = 0;
    pending->section.section_idx = -1;
    val = parameters->checkpoint;
    flags = game_flow.transition_flags;
    stream->checkpoint = val;
    scale = parameters->transition_ticks;
    factor = cur->section.transition_scale;
    game_flow.transition_flags = flags | 0x40;
    scale = mul12((sint32)scale, factor);
    stream->transition_ticks = scale;
    stream->prev_section = 0;
    stream->target_section = &cur->section;
    flags = cur->section.state_flags;
    cur->section.state_flags = flags & ~9u;
    scr_render.transition_ticks = 15;
    return 15;
}

void SceneStream::ReleaseSceneObjects(void)
{
    FUNCTION_MARKER(0x800260B0, "SLUS_005.78");
    SceneNode *node = registered_object_head;
    SceneNode *next;
    AnimatedObject *actor;

    while (node != 0)
    {
        next = node->links.registry_next;
        if (((const AnimatedObject *)node)->behavior == 6)
            ((AnimatedObject *)node)->StopModelGrpSound();
        node->Unregister();
        if (node->kind == 7)
            SpriteRecord::ReleaseSpr((SpriteRecord *)node);
        else
        {
            actor = (AnimatedObject *)node;
            if (actor->behavior_suspended == 2 || actor->movement_mode >= 4)
                ((ActorPlacement *)actor->path_definition)->ReleaseBoundActor();
            if ((actor->flags & 4) != 0)
                node->flags = (node->flags | 0x80) & 0xFC;
            else
                FreeGameMemory(node);
        }
        node = next;
    }
    if (scene_stream.active_section != 0)
    {
        uint32 grp;
        for (grp = 0; grp < 2; ++grp)
        {
            SceneSection *section = scene_stream.active_section;
            node = grp == 0 ? section->links.first_child : section->transient_links.first_child;
            while (node != 0)
            {
                next = node->links.sibling_next;
                if (grp != 0 || (node->kind == 7 && (node->state_flags & 2) != 0))
                {
                    section = scene_stream.active_section;
                    (grp == 0 ? section->links : section->transient_links).DetachChild(node);
                    if (node->kind == 7)
                        SpriteRecord::ReleaseSpr((SpriteRecord *)node);
                    else
                    {
                        actor = (AnimatedObject *)node;
                        if (actor->behavior_suspended == 2 || actor->movement_mode >= 4)
                            ((ActorPlacement *)actor->path_definition)->ReleaseBoundActor();
                        if ((actor->flags & 4) != 0)
                            node->flags = (node->flags | 0x80) & 0xFC;
                        else
                            FreeGameMemory(node);
                    }
                }
                node = next;
            }
        }
        CharacterModelRecord::RestoreCharacterModelAnimations();
        {
            SceneSection *section = scene_stream.active_section;
            section->state_flags &= 0xFFFE;
        }
        SoundEffects::StopSceneSounds();
    }
}

void SceneStream::ExchangeSceneResourceBuffers(SceneSection *scene, const SceneSection *prev)
{
    FUNCTION_MARKER(0x80026370, "SLUS_005.78");
    uint32 changed = 0;
    if (scene == 0)
        return;
    if (prev == 0 || scene->resource_identity != prev->resource_identity)
        changed = 1;
    if (scene == prev)
        return;
    if (scene->sequence_bundle != 0 && changed != 0)
    {
        SceneSequenceBundle *slot0, *slot1;
        if (sequence_playback.cur_handle != 0)
        {
            SequencePlayback::StopCurSequence();
            SequencePlayback::CloseSequence(sequence_playback.cur_handle);
            sequence_playback.cur_handle = 0;
        }
        slot0 = scene_stream.sequence_slots[0];
        slot1 = scene_stream.sequence_slots[1];
        scene_stream.sequence_slots[1] = slot0;
        scene_stream.sequence_slots[0] = slot1;
    }
    if (scene->background != 0)
    {
        BackgroundResource *slot0 = scene_stream.background_slots[0];
        BackgroundResource *slot1 = scene_stream.background_slots[1];
        scene_stream.background_slots[1] = slot0;
        scene_stream.background_slots[0] = slot1;
    }
    else if (prev != 0)
        scene->background = 0;
}

void SceneStream::ApplySceneSectionTransition(void)
{
    FUNCTION_MARKER(0x800264C8, "SLUS_005.78");
    SceneStream *stream;
    SceneSection *scene;
    SceneNode *node;
    uint32 flags, process_new, x, y;
    SceneStream::ReleaseSceneObjects();
    CharacterContactPool::InitializeContactPool();
    stream = &scene_stream;
    SceneStream::ExchangeSceneResourceBuffers(stream->target_section, stream->prev_section);
    scene = stream->target_section;
    stream->prev_section = 0;
    scene_stream.active_section = scene;
    scene_lighting.lights = (SceneLight *)scene->object_heads[14];
    for (uint32 color = 0; color < 4; ++color)
        scene_lighting.ambient[color] = scene->ambient[color];
    if (cutscene.active != 0 || CutscenePlayback::GetCutsceneMode() != 0)
        CutscenePlayback::StopCutscene();
    cutscene.trigger = 0;
    cutscene.path = 0;
    flags = stream->target_section->state_flags;
    process_new = ((flags >> 3) ^ 1) & 1;
    if (process_new != 0)
    {
        if ((flags & 0x10) == 0)
            FillBytes(scene_events.script_vals, 0, 50);
        scene = stream->target_section;
        node = scene->object_heads[14];
        while (node != 0)
        {
            SceneLight *light = (SceneLight *)node;
            flags = light->event_flags;
            light->event_flags = flags & 0xFB;
            if ((flags & 8) != 0)
                light->flags = light->flags | 1;
            else
                light->flags = light->flags & 0xFE;
            node = light->links.registry_next;
            if (node == 0)
                scene = stream->target_section;
        }
        node = scene->object_heads[4];
        while (node != 0)
        {
            SceneTrigger *trigger = (SceneTrigger *)node;
            uint32 kind;
            flags = trigger->node.state_flags & 0xFF;
            kind = trigger->node.state_flags >> 8;
            trigger->node.state_flags = (trigger->node.state_flags & 0xFF00) | (uint8)(flags & 0xB7);
            if (kind - 40 < 16)
            {
                uint32 word0 = trigger->parameters.event.command;
                uint32 word1 = trigger->parameters.event.schedule;
                trigger->parameters.event.command = (word0 & 0xFFF) | ((kind - 39) << 12) | (word1 & 0x1FE0000);
            }
            else if ((sint8)kind == 2)
                process_new = 0;
            else if ((sint8)kind == 5)
            {
                trigger->parameters.damage.deadline = 0;
            }
            if ((sint8)(trigger->node.state_flags >> 8) != 50 || (trigger->parameters.sound.volume_flags & 0x3FFF) != 0)
                trigger->node.flags = trigger->node.flags & 0x7F;
            node = trigger->node.links.registry_next;
        }
        node = stream->target_section->object_heads[5];
        while (node != 0)
        {
            SceneTrigger *trigger = (SceneTrigger *)node;
            uint32 kind;
            flags = trigger->node.flags;
            kind = trigger->node.state_flags >> 8;
            trigger->node.flags = flags & 0x7F;
            if (kind - 40 < 16)
            {
                uint32 word0 = trigger->parameters.event.command;
                uint32 word1 = trigger->parameters.event.schedule;
                sint32 cur_kind = (sint8)(trigger->node.state_flags >> 8);
                trigger->parameters.event.command = (word0 & 0xFFF) | ((kind - 39) << 12) | (word1 & 0x1FE0000);
                if (cur_kind == 50)
                    trigger->parameters.sound.voice = 255;
            }
            node = trigger->node.links.registry_next;
        }
        ActorBinding *binding = scene_stream.active_section->actor_bindings;
        while (binding != 0)
        {
            ActorPlacement *placement = binding->definition;
            placement->InitializeBinding();
            placement = binding->definition;
            placement->path.flags &= 0xFFFFFFFA;
            placement = binding->definition;
            if ((placement->path.flags & 0x20) != 0)
            {
                placement->path.node.flags |= 1;
                placement = binding->definition;
                placement->path.flags |= 1;
            }
            else
                placement->path.node.flags &= 0xFE;
            binding = binding->next;
        }
        CameraProjection::TraverseSceneObjects(stream->target_section, 4);
        scene = stream->target_section;
        scene->state_flags = scene->state_flags | 8;
        stream->target_section->collected_count = 0;
        scene = stream->target_section;
        x = scene->far_distance;
        camera_proj.far_distance_copy = x;
        camera_proj.far_distance = x;
    }
    scene = stream->target_section;
    x = (sint16)stream->checkpoint;
    y = (sint16)stream->transition_ticks;
#if defined(_DEBUG)
    if (level_resource.level_id == 1 && scene->section_idx == 1 && getenv("P2_PERF_REFERENCE_START") != NULL)
    {
        x = 0;
        y = 21;
    }
#endif
    CharacterController::PlaceCharacterOnPath(scene, x, y);
#if defined(_DEBUG)
    if (level_resource.level_id == 1 && scene->section_idx == 1 && getenv("P2_PERF_REFERENCE_START") != NULL)
    {
        // Temporary user-selected starting point for the ten-second Right test
        AnimatedObject *player = character_pad.object;
        player->pos.x = 2162269;
        player->pos.y = 54304;
        player->pos.z = 141410;
        player->world_pos.x = 2162269;
        player->world_pos.y = 68946;
        player->world_pos.z = 141410;
        player->flags = 0x34;
        player->path_flags = 0x400;
        player->yaw = player->target_yaw = 3040;
        player->velocity.x = player->velocity.y = player->velocity.z = 0;
        character_pad.motion.horizontal_speed = 0;
        character_pad.motion.vertical_speed = 0;
        camera_motion.pos.x = 2188016;
        camera_motion.pos.y = 138071;
        camera_motion.pos.z = 279655;
        camera_motion.focus_pos.x = 2162269;
        camera_motion.focus_pos.y = 68640;
        camera_motion.focus_pos.z = 141410;
        camera_motion.desired_pos.x = 2422324;
        camera_motion.desired_pos.y = 49922;
        camera_motion.desired_pos.z = 256522;
        camera_motion.flags = 1;
        camera_motion.smooth_ticks[0] = camera_motion.smooth_ticks[1] = camera_motion.smooth_ticks[2] = 0;
        camera_motion.target_ticks = 0;
        CutscenePlayback::StopCutscene();
        CameraMotion::UpdateCameraAim();
        hd_camera_cut();
        fprintf(stderr, "P2_PERF_REFERENCE_START level=1 section=1 path=0 frame=21\n");
    }
#endif
    scene = stream->target_section;
    flags = scene->state_flags;
    x = sequence_playback.cur_handle;
    scene->state_flags = flags | 1u;
    if (x == 0)
    {
        scene = stream->target_section;
        if (scene->sequence_bundle != 0)
        {
            uint32 idx = scene->section_idx;
            uint32 asset = level_resource.asset_base + idx * 9;
            if ((sint32)resource_archive.Offset(asset + 8) >= 0)
            {
                SceneSequenceBundle *bundle;
                scene = stream->target_section;
                idx = scene->section_idx;
                bundle = scene->sequence_bundle;
                asset = level_resource.asset_base;
                asset += idx * 9;
                x = SequencePlayback::LoadSequence(asset + 2, asset + 8, bundle->data, bundle->bytes, NULL, 0);
                sequence_playback.cur_handle = x;
                if ((x & 0xFFFF) != 0)
                    SequencePlayback::StartCurSequence();
                else
                    scene_loader.phase = 4;
            }
        }
    }
    if (level_resource.level_id != 0)
    {
        if (process_new == 0)
            return;
        flags = stream->target_section->state_flags & 0x10;
        if (flags != 0)
            return;
        SceneLoader::RequestSceneSection(0xFFFFFFFF);
        return;
    }
    if ((sint8)game_flow.progress.sel_stage > 0 && game_flow.progress.hub_spawn_consumed == 0)
    {
        uint32 idx = (sint8)game_flow.progress.sel_stage;
        uint32 next = OriginalDefaults::hub_sections[idx];
        next = (sint32)next < 3 ? next + 1 : next - 1;
        SceneLoader::RequestSceneSection(next);
    }
    {
        sint32 number = (sint8)game_flow.progress.sel_stage;
        sint32 high = ((sint64)number * 0x66666667) >> 32;
        sint32 quotient = (high >> 2) - (number >> 31);
        uint32 event;
        level_resource.checkpoint = stream->checkpoint;
        level_resource.transition_ticks = stream->transition_ticks;
        event = SceneEvents::EncodeObjectName((sint8)quotient + 48, (sint8)(number - quotient * 10) + 48);
        SceneEvents::QueueSceneEvent(event | 0x4000, 4);
        return;
    }
}

void SceneStream::ResetSceneStreams(void)
{
    uint32 idx;
    SceneSection *section;
    FUNCTION_MARKER(0x80026AA0, "SLUS_005.78");
    hd_invalidate_geometry();
    CutscenePlayback::StopCutscene();
    cutscene.trigger = 0;
    cutscene.path = 0;
    scene_stream.active_section = 0;
    scene_lighting.lights = 0;
    if (sequence_playback.loading_phase != 0)
    {
        CloseVabBank(sequence_playback.loading_bank);
        free(sequence_playback.loading_samples);
        sequence_playback.loading_samples = NULL;
        sequence_playback.loading_sample_bytes = 0;
        sequence_playback.loading_bank = OriginalDefaults::SequenceLoadingBank;
        sequence_playback.loading_phase = 0;
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
    for (idx = 0; idx < SceneStream::SequenceBufferCount; ++idx)
        free(scene_stream.sequence_bundles[idx].data);
    for (idx = 0; idx < 2; ++idx)
    {
        free(scene_stream.backgrounds[idx].data);
        free(scene_stream.backgrounds[idx].tiles);
        free(scene_stream.buffers[idx].section.materials.texs);
        free(scene_stream.buffers[idx].section.materials.materials);
        free(scene_stream.buffers[idx].section.materials.data);
        memset(&scene_stream.buffers[idx].section.materials, 0, sizeof(scene_stream.buffers[idx].section.materials));
        free(scene_stream.buffers[idx].section.lighting_recs);
        for (ActorBinding *binding = scene_stream.buffers[idx].section.actor_bindings; binding != 0; binding = binding->next)
            free(binding->rotation_track);
        free(scene_stream.buffers[idx].section.actor_bindings);
        free(scene_stream.buffers[idx].section.animated_paths);
        free(scene_stream.buffers[idx].section.lights);
        free(scene_stream.buffers[idx].section.meshes);
        free(scene_stream.buffers[idx].section.face_storage);
        free(scene_stream.buffers[idx].section.vertex_storage);
        free(scene_stream.buffers[idx].section.col_storage);
        free(scene_stream.buffers[idx].section.material_animation_words);
        free(scene_stream.buffers[idx].section.animation_payload_data);
        if (scene_stream.buffers[idx].section.animations != 0)
        {
            for (uint32 header_idx = 0; header_idx < scene_stream.buffers[idx].section.animation_count; ++header_idx)
            {
                for (uint32 track_grp = 0; track_grp < 4; ++track_grp)
                    free(scene_stream.buffers[idx].section.animations[header_idx].tracks[track_grp].bytes);
            }
        }
        free(scene_stream.buffers[idx].section.animations);
        free(scene_stream.buffers[idx].section.billboards);
        free(scene_stream.buffers[idx].section.mesh_parts);
        free(scene_stream.buffers[idx].section.sphere_nodes);
        free(scene_stream.buffers[idx].section.placements);
        free(scene_stream.buffers[idx].section.animated_objects);
        free(scene_stream.buffers[idx].section.sprs);
        for (uint32 track_idx = 0; track_idx < scene_stream.buffers[idx].section.spr_extent_track_count; ++track_idx)
            free(scene_stream.buffers[idx].section.spr_extent_tracks[track_idx]);
        free(scene_stream.buffers[idx].section.spr_extent_tracks);
        free(scene_stream.buffers[idx].section.triggers);
        free(scene_stream.buffers[idx].section.path_names);
        free(scene_stream.buffers[idx].section.paths);
        free(scene_stream.buffers[idx].section.path_key_data);
        free(scene_stream.buffers[idx].section.path_offset_matrices);
        hd_invalidate_paths();
        free(scene_stream.buffers[idx].section.definition_transforms);
        scene_stream.buffers[idx].section.definition_transforms = 0;
        scene_stream.buffers[idx].section.path_offset_matrices = 0;
        scene_stream.buffers[idx].section.lights = 0;
        scene_stream.buffers[idx].section.light_count = 0;
        scene_stream.buffers[idx].section.meshes = 0;
        scene_stream.buffers[idx].section.mesh_count = 0;
        scene_stream.buffers[idx].section.face_storage = 0;
        scene_stream.buffers[idx].section.face_count = 0;
        scene_stream.buffers[idx].section.vertex_storage = 0;
        scene_stream.buffers[idx].section.vertex_count = 0;
        scene_stream.buffers[idx].section.col_storage = 0;
        scene_stream.buffers[idx].section.col_count = 0;
        scene_stream.buffers[idx].section.material_animation_words = 0;
        scene_stream.buffers[idx].section.material_animation_word_count = 0;
        scene_stream.buffers[idx].section.animation_payload_data = 0;
        scene_stream.buffers[idx].section.animation_payload_bytes = 0;
        scene_stream.buffers[idx].section.animations = 0;
        scene_stream.buffers[idx].section.animation_count = 0;
        scene_stream.buffers[idx].section.billboards = 0;
        scene_stream.buffers[idx].section.billboard_count = 0;
        scene_stream.buffers[idx].section.mesh_parts = 0;
        scene_stream.buffers[idx].section.mesh_part_count = 0;
        scene_stream.buffers[idx].section.sphere_nodes = 0;
        scene_stream.buffers[idx].section.sphere_node_count = 0;
        scene_stream.buffers[idx].section.placements = 0;
        scene_stream.buffers[idx].section.placement_count = 0;
        scene_stream.buffers[idx].section.animated_objects = 0;
        scene_stream.buffers[idx].section.animated_object_count = 0;
        scene_stream.buffers[idx].section.sprs = 0;
        scene_stream.buffers[idx].section.spr_count = 0;
        scene_stream.buffers[idx].section.spr_extent_tracks = 0;
        scene_stream.buffers[idx].section.spr_extent_track_count = 0;
        scene_stream.buffers[idx].section.triggers = 0;
        scene_stream.buffers[idx].section.trigger_count = 0;
        scene_stream.buffers[idx].section.path_names = 0;
        scene_stream.buffers[idx].section.path_name_count = 0;
        scene_stream.buffers[idx].section.paths = 0;
        scene_stream.buffers[idx].section.path_count = 0;
        scene_stream.buffers[idx].section.path_key_data = 0;
        scene_stream.buffers[idx].section.actor_bindings = 0;
        scene_stream.buffers[idx].section.animated_paths = 0;
        scene_stream.buffers[idx].section.lighting_recs = NULL;
        scene_stream.buffers[idx].section.lighting_count = 0;
    }
    render_view.lighting_cache = NULL;
    render_view.lighting_enabled = false;
    memset(&scene_stream, 0, sizeof(scene_stream));
    scene_stream.background_slots[0] = &scene_stream.backgrounds[0];
    scene_stream.background_slots[1] = &scene_stream.backgrounds[1];
    scene_stream.sequence_slots[1] = &scene_stream.sequence_bundles[0];
    for (idx = 0; idx < 2; ++idx)
    {
        section = &scene_stream.buffers[idx].section;
        section->data = 0;
        section->data_bytes = 0;
        section->state_flags = 0;
    }
    scene_stream.pending_buffer = &scene_stream.buffers[1];
    scene_stream.pending_buffer->section.section_idx = -1;
}

void SceneStream::EmptyStreamingOperation(void)
{
    FUNCTION_MARKER(0x80026B64, "SLUS_005.78");
    /* Original function returns without side effects */
}
