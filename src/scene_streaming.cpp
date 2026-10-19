#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "game.h"
#include "original_defaults.h"

SceneStream scene_stream = {0};

uint32 ResetSceneStreams(void)
{
    uint32 index;
    SceneSection *section;
    FUNCTION_MARKER(0x80026AA0, "SLUS_005.78");
    StopCutscene();
    cutscene.trigger = 0;
    cutscene.path = 0;
    scene_stream.active_section = 0;
    scene_lighting.lights = 0;
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
    for (index = 0; index < SceneStream::SequenceBufferCount; ++index)
        free(scene_stream.sequence_bundles[index].data);
    for (index = 0; index < 2; ++index)
    {
        free(scene_stream.backgrounds[index].data);
        free(scene_stream.backgrounds[index].tiles);
        free(scene_stream.buffers[index].section.materials.textures);
        free(scene_stream.buffers[index].section.materials.materials);
        free(scene_stream.buffers[index].section.materials.data);
        memset(&scene_stream.buffers[index].section.materials, 0, sizeof(scene_stream.buffers[index].section.materials));
        free(scene_stream.buffers[index].section.lighting_records);
        for (ActorBinding *binding = scene_stream.buffers[index].section.actor_bindings; binding != 0; binding = binding->next)
            free(binding->rotation_track);
        free(scene_stream.buffers[index].section.actor_bindings);
        free(scene_stream.buffers[index].section.animated_paths);
        free(scene_stream.buffers[index].section.lights);
        free(scene_stream.buffers[index].section.meshes);
        free(scene_stream.buffers[index].section.face_storage);
        free(scene_stream.buffers[index].section.vertex_storage);
        free(scene_stream.buffers[index].section.collision_storage);
        free(scene_stream.buffers[index].section.material_animation_words);
        free(scene_stream.buffers[index].section.animation_payload_data);
        if (scene_stream.buffers[index].section.animations != 0)
        {
            for (uint32 header_index = 0; header_index < scene_stream.buffers[index].section.animation_count; ++header_index)
            {
                for (uint32 track_group = 0; track_group < 4; ++track_group)
                    free(scene_stream.buffers[index].section.animations[header_index].tracks[track_group].bytes);
            }
        }
        free(scene_stream.buffers[index].section.animations);
        free(scene_stream.buffers[index].section.billboards);
        free(scene_stream.buffers[index].section.mesh_parts);
        free(scene_stream.buffers[index].section.sphere_nodes);
        free(scene_stream.buffers[index].section.placements);
        free(scene_stream.buffers[index].section.animated_objects);
        free(scene_stream.buffers[index].section.sprites);
        for (uint32 track_index = 0; track_index < scene_stream.buffers[index].section.sprite_extent_track_count; ++track_index)
            free(scene_stream.buffers[index].section.sprite_extent_tracks[track_index]);
        free(scene_stream.buffers[index].section.sprite_extent_tracks);
        free(scene_stream.buffers[index].section.triggers);
        free(scene_stream.buffers[index].section.path_names);
        free(scene_stream.buffers[index].section.paths);
        free(scene_stream.buffers[index].section.path_key_data);
        free(scene_stream.buffers[index].section.path_offset_matrices);
        free(scene_stream.buffers[index].section.definition_transforms);
        scene_stream.buffers[index].section.definition_transforms = 0;
        scene_stream.buffers[index].section.path_offset_matrices = 0;
        scene_stream.buffers[index].section.lights = 0;
        scene_stream.buffers[index].section.light_count = 0;
        scene_stream.buffers[index].section.meshes = 0;
        scene_stream.buffers[index].section.mesh_count = 0;
        scene_stream.buffers[index].section.face_storage = 0;
        scene_stream.buffers[index].section.face_count = 0;
        scene_stream.buffers[index].section.vertex_storage = 0;
        scene_stream.buffers[index].section.vertex_count = 0;
        scene_stream.buffers[index].section.collision_storage = 0;
        scene_stream.buffers[index].section.collision_count = 0;
        scene_stream.buffers[index].section.material_animation_words = 0;
        scene_stream.buffers[index].section.material_animation_word_count = 0;
        scene_stream.buffers[index].section.animation_payload_data = 0;
        scene_stream.buffers[index].section.animation_payload_bytes = 0;
        scene_stream.buffers[index].section.animations = 0;
        scene_stream.buffers[index].section.animation_count = 0;
        scene_stream.buffers[index].section.billboards = 0;
        scene_stream.buffers[index].section.billboard_count = 0;
        scene_stream.buffers[index].section.mesh_parts = 0;
        scene_stream.buffers[index].section.mesh_part_count = 0;
        scene_stream.buffers[index].section.sphere_nodes = 0;
        scene_stream.buffers[index].section.sphere_node_count = 0;
        scene_stream.buffers[index].section.placements = 0;
        scene_stream.buffers[index].section.placement_count = 0;
        scene_stream.buffers[index].section.animated_objects = 0;
        scene_stream.buffers[index].section.animated_object_count = 0;
        scene_stream.buffers[index].section.sprites = 0;
        scene_stream.buffers[index].section.sprite_count = 0;
        scene_stream.buffers[index].section.sprite_extent_tracks = 0;
        scene_stream.buffers[index].section.sprite_extent_track_count = 0;
        scene_stream.buffers[index].section.triggers = 0;
        scene_stream.buffers[index].section.trigger_count = 0;
        scene_stream.buffers[index].section.path_names = 0;
        scene_stream.buffers[index].section.path_name_count = 0;
        scene_stream.buffers[index].section.paths = 0;
        scene_stream.buffers[index].section.path_count = 0;
        scene_stream.buffers[index].section.path_key_data = 0;
        scene_stream.buffers[index].section.actor_bindings = 0;
        scene_stream.buffers[index].section.animated_paths = 0;
        scene_stream.buffers[index].section.lighting_records = NULL;
        scene_stream.buffers[index].section.lighting_count = 0;
    }
    render_view.lighting_cache = NULL;
    render_view.lighting_enabled = false;
    memset(&scene_stream, 0, sizeof(scene_stream));
    scene_stream.background_slots[0] = &scene_stream.backgrounds[0];
    scene_stream.background_slots[1] = &scene_stream.backgrounds[1];
    scene_stream.sequence_slots[1] = &scene_stream.sequence_bundles[0];
    for (index = 0; index < 2; ++index)
    {
        section = &scene_stream.buffers[index].section;
        section->data = 0;
        section->data_bytes = 0;
        section->state_flags = 0;
    }
    scene_stream.pending_buffer = &scene_stream.buffers[1];
    scene_stream.pending_buffer->section.section_index = -1;
    return 0xFFFFFFFF;
}

uint32 ActivateSceneSection(void)
{
    FUNCTION_MARKER(0x80025BFC, "SLUS_005.78");
    SceneStream *stream = &scene_stream;
    SceneSectionBuffer *current = stream->pending_buffer;
    SceneSectionBuffer *pending;
    uint32 index = (uint32)current->section.section_index;
    const SectionDefinition *parameters;
    uint32 value, flags, scale;
    sint32 factor;
    stream->current_buffer = current;
    parameters = level_resource.sections + (sint32)index;
    if (current == &stream->buffers[0])
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
    pending->section.section_index = -1;
    value = parameters->checkpoint;
    flags = game_flow.transition_flags;
    stream->checkpoint = value;
    scale = parameters->transition_ticks;
    factor = current->section.transition_scale;
    game_flow.transition_flags = flags | 0x40;
    scale = (uint32)(((sint64)(sint32)scale * factor) >> 12);
    stream->transition_ticks = scale;
    stream->previous_section = 0;
    stream->target_section = &current->section;
    flags = current->section.state_flags;
    current->section.state_flags = flags & ~9u;
    screen_render.transition_ticks = 15;
    return 15;
}

uint32 ApplySceneSectionTransition(void)
{
    FUNCTION_MARKER(0x800264C8, "SLUS_005.78");
    SceneStream *stream;
    SceneSection *scene;
    SceneNode *node;
    uint32 flags, process_new, x, y;
    ReleaseSceneObjects();
    InitializeContactPool();
    stream = &scene_stream;
    ExchangeSceneResourceBuffers(stream->target_section, stream->previous_section);
    scene = stream->target_section;
    stream->previous_section = 0;
    scene_stream.active_section = scene;
    scene_lighting.lights = (SceneLight *)(scene)->object_heads[14];
    x = (scene)->runtime_words[1];
    y = (scene)->runtime_words[2];
    xport_store_le32((uint8 *)(scene_lighting.ambient + 0), x);
    xport_store_le32((uint8 *)(scene_lighting.ambient + 2), y);
    if (cutscene.active != 0 || GetCutsceneMode() != 0)
        StopCutscene();
    cutscene.trigger = 0;
    cutscene.path = 0;
    flags = (stream->target_section)->state_flags;
    process_new = ((flags >> 3) ^ 1) & 1;
    if (process_new != 0)
    {
        if ((flags & 0x10) == 0)
            FillBytes(scene_events.script_values, 0, 50);
        scene = stream->target_section;
        node = (scene)->object_heads[14];
        while (node != 0)
        {
            SceneLight *light = (SceneLight *)node;
            flags = light->event_flags;
            light->event_flags = (uint8)(flags & 0xFB);
            if ((flags & 8) != 0)
                light->flags = light->flags | 1;
            else
                light->flags = light->flags & 0xFE;
            node = light->links.registry_next;
            if (node == 0)
                scene = stream->target_section;
        }
        node = (scene)->object_heads[4];
        while (node != 0)
        {
            SceneTrigger *trigger = (SceneTrigger *)node;
            uint32 kind;
            flags = (trigger->node.state_flags & 0xFF);
            kind = (trigger->node.state_flags >> 8);
            trigger->node.state_flags = (trigger->node.state_flags & 0xFF00) | (uint8)(flags & 0xB7);
            if (kind - 40 < 16)
            {
                uint32 first = trigger->parameters.words[0];
                uint32 second = trigger->parameters.words[1];
                trigger->parameters.words[0] = (first & 0xFFF) | ((kind - 39) << 12) | (second & 0x1FE0000);
            }
            else if ((sint8)kind == 2)
                process_new = 0;
            else if ((sint8)kind == 5)
            {
                trigger->parameters.words[1] = 0;
                trigger->parameters.words[0] = 0;
            }
            if ((sint8)(trigger->node.state_flags >> 8) != 50 || ((trigger->parameters.words[2] >> 16) & 0x3FFF) != 0)
                trigger->node.flags = trigger->node.flags & 0x7F;
            node = trigger->node.links.registry_next;
        }
        node = (stream->target_section)->object_heads[5];
        while (node != 0)
        {
            SceneTrigger *trigger = (SceneTrigger *)node;
            uint32 kind;
            flags = trigger->node.flags;
            kind = (trigger->node.state_flags >> 8);
            trigger->node.flags = flags & 0x7F;
            if (kind - 40 < 16)
            {
                uint32 first = trigger->parameters.words[0];
                uint32 second = trigger->parameters.words[1];
                sint32 current_kind = (sint8)(trigger->node.state_flags >> 8);
                trigger->parameters.words[0] = (first & 0xFFF) | ((kind - 39) << 12) | (second & 0x1FE0000);
                if (current_kind == 50)
                    trigger->parameters.words[2] = trigger->parameters.words[2] | 0xFF;
            }
            node = trigger->node.links.registry_next;
        }
        ActorBinding *binding = scene_stream.active_section->actor_bindings;
        while (binding != 0)
        {
            ActorPlacement *placement = ((ActorPlacement *)(binding->definition));
            placement->InitializeBinding();
            placement = ((ActorPlacement *)(binding->definition));
            placement->path.flags &= 0xFFFFFFFA;
            placement = ((ActorPlacement *)(binding->definition));
            if ((placement->path.flags & 0x20) != 0)
            {
                placement->path.node.flags |= 1;
                placement = ((ActorPlacement *)(binding->definition));
                placement->path.flags |= 1;
            }
            else
                placement->path.node.flags &= 0xFE;
            binding = binding->next;
        }
        TraverseSceneObjects(stream->target_section, 4);
        scene = stream->target_section;
        (scene)->state_flags = (scene)->state_flags | 8;
        (stream->target_section)->collected_count = 0;
        scene = stream->target_section;
        x = (scene)->origin[0];
        camera_projection.far_distance_copy = x;
        camera_projection.far_distance = x;
    }
    scene = stream->target_section;
    x = (uint32)(sint16)stream->checkpoint;
    y = (uint32)(sint16)stream->transition_ticks;
    PlaceCharacterOnPath(scene, (sint32)x, (sint16)y);
    scene = stream->target_section;
    flags = (scene)->state_flags;
    x = sequence_playback.current_handle;
    (scene)->state_flags = flags | 1u;
    if (x == 0)
    {
        scene = stream->target_section;
        if ((scene)->sequence_bundle != 0)
        {
            uint32 index = (uint32)(scene)->section_index;
            uint32 asset = level_resource.asset_base + index * 9;
            if ((sint32)resource_archive.Offset(asset + 8) >= 0)
            {
                SceneSequenceBundle *bundle;
                scene = stream->target_section;
                index = (uint32)(scene)->section_index;
                bundle = scene->sequence_bundle;
                if (bundle != &stream->sequence_bundles[0] && bundle != &stream->sequence_bundles[1])
                {
                    fprintf(stderr, "P2: Invalid scene sequence bundle\n");
                    abort();
                }
                asset = level_resource.asset_base;
                asset += index * 9;
                x = LoadSequence(asset + 2, asset + 8, bundle->data, bundle->bytes, NULL, 0);
                sequence_playback.current_handle = x;
                if ((x & 0xFFFF) != 0)
                    StartCurrentSequence();
                else
                    scene_loader.phase = 4;
            }
        }
    }
    if (level_resource.level_id != 0)
    {
        if (process_new == 0)
            return 0;
        flags = (stream->target_section)->state_flags & 0x10;
        if (flags != 0)
            return flags;
        return RequestSceneSection(0xFFFFFFFF);
    }
    if ((sint8)game_flow.progress.selected_stage > 0 && game_flow.progress.hub_spawn_consumed == 0)
    {
        uint32 index = (uint32)(sint8)game_flow.progress.selected_stage;
        uint32 next = OriginalDefaults::HubSections[index];
        next = (sint32)next < 3 ? next + 1 : next - 1;
        RequestSceneSection(next);
    }
    {
        sint32 number = (sint8)game_flow.progress.selected_stage;
        sint32 high = (sint32)(((sint64)number * 0x66666667) >> 32);
        sint32 quotient = (high >> 2) - (number >> 31);
        uint32 event;
        level_resource.checkpoint = stream->checkpoint;
        level_resource.transition_ticks = stream->transition_ticks;
        event = EncodeObjectName((sint8)quotient + 48, (sint8)(number - quotient * 10) + 48);
        return QueueSceneEvent(event | 0x4000, 4);
    }
}

void ExchangeSceneResourceBuffers(SceneSection *scene, const SceneSection *previous)
{
    FUNCTION_MARKER(0x80026370, "SLUS_005.78");
    uint32 changed = 0;
    if (scene == 0)
        return;
    if (previous == 0 || (scene)->resource_identity != (previous)->resource_identity)
        changed = 1;
    if (scene == previous)
        return;
    if ((scene)->sequence_bundle != 0 && changed != 0)
    {
        SceneSequenceBundle *first, *second;
        if (sequence_playback.current_handle != 0)
        {
            StopCurrentSequence();
            CloseSequence(sequence_playback.current_handle);
            sequence_playback.current_handle = 0;
        }
        first = scene_stream.sequence_slots[0];
        second = scene_stream.sequence_slots[1];
        scene_stream.sequence_slots[1] = first;
        scene_stream.sequence_slots[0] = second;
    }
    if (scene->background != 0)
    {
        BackgroundResource *first = scene_stream.background_slots[0];
        BackgroundResource *second = scene_stream.background_slots[1];
        scene_stream.background_slots[1] = first;
        scene_stream.background_slots[0] = second;
    }
    else if (previous != 0)
        scene->background = 0;
}

uint32 HasPendingSceneSection(void)
{
    FUNCTION_MARKER(0x80025B30, "SLUS_005.78");
    if (scene_stream.pending_buffer != 0)
        return 1;
    return level_resource.level_id != 0;
}

SceneSection *SwapSceneSections(void)
{
    FUNCTION_MARKER(0x80025B7C, "SLUS_005.78");
    SceneSectionBuffer *first, *result;
    first = scene_stream.current_buffer;
    result = scene_stream.pending_buffer;
    scene_stream.pending_buffer = first;
    scene_stream.current_buffer = result;
    return result != 0 ? &result->section : 0;
}

uint32 SetSceneTransition(SceneSection *source, SceneSection *scene, sint32 index, uint32 flags)
{
    FUNCTION_MARKER(0x80025674, "SLUS_005.78");
    SceneStream *stream;
    SceneSection *first_section, *second_section;
    const SceneSection *scene_view;
    SceneSectionBuffer *first, *second, *replacement;
    uint32 value, coefficient, result;
    const SectionDefinition *record;
    sint32 kind;
    stream = &scene_stream;
    stream->previous_section = scene;
    if (source != 0)
        stream->target_section = source;
    else
    {
        first = stream->current_buffer;
        first_section = &first->section;
        first_section->state_flags = first_section->state_flags & 0xFFF7;
        first = stream->current_buffer;
        first_section = &first->section;
        if ((first_section->state_flags & 0x10) != 0)
        {
            second = stream->pending_buffer;
            second_section = second != 0 ? &second->section : 0;
            if (second != 0)
            {
                kind = second_section->section_index;
                if (kind < first_section->section_index)
                {
                    second_section->state_flags = second_section->state_flags & 0xFFF7;
                    second = stream->pending_buffer;
                    second_section = second != 0 ? &second->section : 0;
                    if ((second_section->state_flags & 0x10) == 0)
                    {
                        first = stream->current_buffer;
                        first_section = &first->section;
                        stream->target_section = second_section;
                        scene = stream->target_section;
                        replacement = second;
                        stream->previous_section = first_section;
                        stream->pending_buffer = first;
                        stream->current_buffer = replacement;
                    }
                }
            }
        }
        else
        {
            second = stream->pending_buffer;
            second_section = second != 0 ? &second->section : 0;
            if (second != 0)
                second_section->state_flags = second_section->state_flags & 0xFFF7;
        }
    }
    stream->checkpoint = index;
    if ((sint16)index == -1)
        stream->transition_ticks = 10;
    else if ((sint16)index == -2)
    {
        scene_view = scene;
        kind = scene_view->section_index;
        record = level_resource.sections + kind;
        value = record->checkpoint;
        stream->checkpoint = value;
        coefficient = record->transition_ticks;
        kind = scene_view->transition_scale;
        value = (uint32)(((sint64)coefficient * kind) >> 12);
        stream->transition_ticks = value;
    }
    else
        stream->transition_ticks = flags;
    result = game_flow.transition_flags;
    result |= 0x40;
    game_flow.transition_flags = result;
    return result;
}

void ReleaseSceneObjects(void)
{
    FUNCTION_MARKER(0x800260B0, "SLUS_005.78");
    SceneNode *node = registered_object_head;
    SceneNode *next;
    AnimatedObject *actor;

    while (node != 0)
    {
        next = node->links.registry_next;
        if (((const sint8 *)node)[125] == 6)
            ((AnimatedObject *)node)->StopModelGroupSound();
        node->Unregister();
        if (node->kind == 7)
            ReleaseSprite((SpriteRecord *)node);
        else
        {
            actor = (AnimatedObject *)node;
            if (actor->metadata126[1] == 2 || actor->movement_mode >= 4)
                ((ActorPlacement *)((const PathDefinition *)((actor->path_definition))))->ReleaseBoundActor();
            if ((actor->flags & 4) != 0)
                node->flags = (node->flags | 0x80) & 0xFC;
            else
                FreeGameMemory(node);
        }
        node = next;
    }
    if (scene_stream.active_section != 0)
    {
        uint32 group;
        for (group = 0; group < 2; ++group)
        {
            SceneSection *section = scene_stream.active_section;
            node = group == 0 ? section->links.first_child : section->transient_links.first_child;
            while (node != 0)
            {
                next = node->links.sibling_next;
                if (group != 0 || (node->kind == 7 && (node->state_flags & 2) != 0))
                {
                    section = scene_stream.active_section;
                    (group == 0 ? section->links : section->transient_links).DetachChild(node);
                    if (node->kind == 7)
                        ReleaseSprite((SpriteRecord *)node);
                    else
                    {
                        actor = (AnimatedObject *)node;
                        if (actor->metadata126[1] == 2 || actor->movement_mode >= 4)
                            ((ActorPlacement *)((const PathDefinition *)((actor->path_definition))))->ReleaseBoundActor();
                        if ((actor->flags & 4) != 0)
                            node->flags = (node->flags | 0x80) & 0xFC;
                        else
                            FreeGameMemory(node);
                    }
                }
                node = next;
            }
        }
        RestoreCharacterModelAnimations();
        {
            SceneSection *section = scene_stream.active_section;
            section->state_flags &= 0xFFFE;
        }
        StopSceneSounds();
    }
}

void NoOp80026B64(void)
{
    FUNCTION_MARKER(0x80026B64, "SLUS_005.78");
    /* Original function returns without side effects */
}
