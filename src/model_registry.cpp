#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "game.h"

// Original constructor order includes the reserved slot before the attack actor
static const ActorModelDefinition::Constructor actor_constructors[48] = {&AnimatedObject::InitializeCommonActor,
                                                                         &AnimatedObject::InitializeActorFacingAttack,
                                                                         &AnimatedObject::InitializeCommonActor,
                                                                         &AnimatedObject::InitializeCommonActor,
                                                                         &AnimatedObject::InitializeActorProximityAttack,
                                                                         &AnimatedObject::InitializeActorStationaryShots,
                                                                         &AnimatedObject::InitializeActorRandomPatrol,
                                                                         &AnimatedObject::InitializeActorTargetPursuit,
                                                                         &AnimatedObject::InitializeActorVerticalShot,
                                                                         &AnimatedObject::InitializeCommonActor,
                                                                         &AnimatedObject::InitializeActorChaseAttack,
                                                                         &AnimatedObject::InitializeCommonActor,
                                                                         &AnimatedObject::InitializeActorVerticalShot,
                                                                         &AnimatedObject::InitializeCommonActor,
                                                                         &AnimatedObject::InitializeCommonActor,
                                                                         &AnimatedObject::InitializeActorAimedShot,
                                                                         &AnimatedObject::InitializeActorPatrolPause,
                                                                         &AnimatedObject::InitializeActorPathSegmentMotion,
                                                                         &AnimatedObject::InitializeActorVerticalPursuit,
                                                                         &AnimatedObject::InitializeActorPathDetour,
                                                                         &AnimatedObject::InitializeCommonActor,
                                                                         &AnimatedObject::InitializeCommonActor,
                                                                         &AnimatedObject::InitializeCommonActor,
                                                                         &AnimatedObject::InitializeCommonActor,
                                                                         &AnimatedObject::InitializeCommonActor,
                                                                         &AnimatedObject::InitializeCommonActor,
                                                                         &AnimatedObject::InitializeCommonActor,
                                                                         &AnimatedObject::InitializeCommonActor,
                                                                         &AnimatedObject::InitializeCommonActor,
                                                                         &AnimatedObject::InitializeCommonActor,
                                                                         &AnimatedObject::InitializeCommonActor,
                                                                         &AnimatedObject::InitializeCommonActor,
                                                                         &AnimatedObject::InitializeCommonActor,
                                                                         &AnimatedObject::InitializeCommonActor,
                                                                         &AnimatedObject::InitializeCommonActor,
                                                                         &AnimatedObject::InitializeCommonActor,
                                                                         &AnimatedObject::InitializeCommonActor,
                                                                         &AnimatedObject::InitializeCommonActor,
                                                                         &AnimatedObject::InitializeCommonActor,
                                                                         &AnimatedObject::InitializeCommonActor,
                                                                         &AnimatedObject::InitializeCommonActor,
                                                                         &AnimatedObject::InitializeCommonActor,
                                                                         &AnimatedObject::InitializeCommonActor,
                                                                         &AnimatedObject::InitializeCommonActor,
                                                                         &AnimatedObject::InitializeCommonActor,
                                                                         &AnimatedObject::InitializeVolleyEnemy,
                                                                         0,
                                                                         &AnimatedObject::InitializeAttackEnemy};

ModelGroupRegistry model_groups = {0};
ActorModelRegistry actor_models = {0};
AnimatedModelRegistry animated_models = {0};

uint32 ResetActorLoadWorkspace(void)
{
    FUNCTION_MARKER(0x80034F04, "SLUS_005.78");
    uint32 i;
    actor_models.load_reset_byte = 0;
    for (i = 0; i < ActorModelRegistry::LoadSlotCount; ++i)
    {
        actor_models.load_words[i][0] = 0;
        actor_models.load_words[i][1] = 0;
        actor_models.load_words[i][2] = 0;
    }
    return 0;
}

void ResetActorModels(void)
{
    FUNCTION_MARKER(0x80034B38, "SLUS_005.78");
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
        actor_models.definitions[model_index].animation_data = 0;
        actor_models.definitions[model_index].animation_headers = 0;
        actor_models.definitions[model_index].morph_restoration = 0;
        actor_models.definitions[model_index].animation_bytes = 0;
    }
    actor_models.count = 0;
}

void ResetAnimatedModels(void)
{
    FUNCTION_MARKER(0x800441DC, "SLUS_005.78");
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
        animated_models.records[model_index].animation_data = 0;
        animated_models.records[model_index].animation_headers = 0;
        animated_models.records[model_index].morph_restoration = 0;
        animated_models.records[model_index].animation_bytes = 0;
    }
    animated_models.count = 0;
}

uint32 RetainCommonModelGroups(void)
{
    FUNCTION_MARKER(0x8002D4C0, "SLUS_005.78");
    for (uint32 group_index = ModelGroupRegistry::CommonCount; group_index < ModelGroupRegistry::Capacity; ++group_index)
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
    // The common loader registers three groups
    model_groups.count = ModelGroupRegistry::CommonCount;
    return ModelGroupRegistry::CommonCount;
}

void ClearModelGroups(void)
{
    FUNCTION_MARKER(0x8002D510, "SLUS_005.78");
    for (uint32 group_index = ModelGroupRegistry::CommonCount; group_index < ModelGroupRegistry::Capacity; ++group_index)
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
    model_groups.count = 0;
}

uint32 LoadModelGroup(uint8 kind)
{
    uint32 offset;
    ModelGroupDefinition *object;
    const uint8 *record;
    uint32 index;
    uint32 source_offset;
    uint32 previous;
    uint32 slot;

    uint32 flags;
    uint8 byte;
    FUNCTION_MARKER(0x8002B058, "SLUS_005.78");
    offset = resource_archive.Offset(resource_load_cursor.asset);
    slot = resource_load_cursor.asset + 1;
    index = model_groups.count;
    source_offset = offset;
    uint32 metadata_bytes = resource_archive.Size(resource_load_cursor.asset);
    if (metadata_bytes < 48)
    {
        fprintf(stderr, "P2: Truncated model group metadata\n");
        abort();
    }
    uint8 *metadata_input = (uint8 *)malloc(metadata_bytes);
    if (metadata_input == 0)
    {
        fprintf(stderr, "P2: Cannot allocate model group metadata\n");
        abort();
    }
    resource_archive.ReadAsset(resource_load_cursor.asset, metadata_input, 1, resource_load_cursor.container_asset);
    record = metadata_input;
    object = model_groups.records + index;
    for (previous = 0; previous < index; ++previous)
    {
        if (model_groups.source_packages[previous] == resource_load_cursor.container_asset && model_groups.source_offsets[previous] == source_offset)
        {
            object = model_groups.definitions[previous];
            break;
        }
    }
    model_groups.source_packages[index] = resource_load_cursor.container_asset;
    model_groups.source_offsets[index] = source_offset;
    model_groups.definitions[index] = object;
    object->flags = record[0];
    object->kind = record[1];
    object->lifetime = xport_load_le16(record + 2);
    for (uint32 model_index = 0; model_index < 4; ++model_index)
    {
        uint32 address = xport_load_le32(record + 8 + model_index * 4);
        if (address != 0)
        {
            fprintf(stderr, "P2: Model group metadata contains a prebound model address\n");
            abort();
        }
        object->models[model_index] = 0;
    }
    object->launch_speed = xport_load_le32(record + 24);
    object->metadata_word = xport_load_le32(record + 28);
    object->gravity = xport_load_le32(record + 32);
    object->sound_radius = xport_load_le32(record + 36);
    object->doppler_radius = xport_load_le32(record + 40);
    object->contact_type = record[44];
    for (uint32 metadata_index = 0; metadata_index < 3; ++metadata_index)
        object->metadata_bytes[metadata_index] = record[45 + metadata_index];
    model_groups.count = index + 1;
    if ((sint32)resource_archive.Size(slot) <= 0)
        object->sound_table = 0;
    else
    {
        uint32 bytes = resource_archive.Size(resource_load_cursor.asset + 1);
        if (bytes < sizeof(SoundTableHeader))
        {
            fprintf(stderr, "P2: Truncated model group sound table\n");
            abort();
        }
        uint8 *sound_input = (uint8 *)malloc(bytes);
        if (sound_input == 0)
        {
            fprintf(stderr, "P2: Cannot allocate native sound table input\n");
            abort();
        }
        resource_archive.ReadAsset(resource_load_cursor.asset + 1, sound_input, 1, resource_load_cursor.container_asset);
        const uint8 *source = sound_input;
        uint32 count = xport_load_le32(source);
        if ((sint32)count > 0 && (count > SoundBankStorage::SampleCapacity || count > (bytes - sizeof(SoundTableHeader)) / sizeof(SoundSample)))
        {
            fprintf(stderr, "P2: Model group sound samples exceed table capacity\n");
            abort();
        }
        object->sound_bank.header.count = count;
        if ((sint32)count > 0)
        {
            for (uint32 sound_index = 0; sound_index < count; ++sound_index)
            {
                const uint8 *encoded = source + sizeof(SoundTableHeader) + sound_index * sizeof(SoundSample);
                SoundSample *sample = object->sound_bank.samples + sound_index;
                sample->spu_address = xport_load_le32(encoded);
                sample->pitch = xport_load_le16(encoded + 4);
                sample->metadata = xport_load_le16(encoded + 6);
            }
        }
        object->sound_table = object->sound_bank.header.Relocate(bytes, object->sound_bank.samples) != 0 ? &object->sound_bank : 0;
        free(sound_input);
    }
    object->kind = kind;
    resource_load_cursor.asset = resource_load_cursor.asset + 2;
    for (index = 0; index < 4; ++index)
    {
        offset = resource_archive.Offset(resource_load_cursor.asset);
        if (offset != 0xFFFFFFFF)
        {
            {
                uint32 material_bytes = resource_archive.Size(resource_load_cursor.asset + 2);
                if (material_bytes < sizeof(EncodedMaterialResourceHeader))
                {
                    fprintf(stderr, "P2: Truncated nested material asset\n");
                    abort();
                }
                uint8 *material_input = (uint8 *)malloc(material_bytes);
                if (material_input == 0)
                {
                    fprintf(stderr, "P2: Cannot allocate nested material input\n");
                    abort();
                }
                resource_archive.ReadAsset(resource_load_cursor.asset + 2, material_input, 1, resource_load_cursor.container_asset);
                RelocateMaterialResource(&object->material_resources[index], material_input, material_bytes, -1);
                free(material_input);
            }
            uint32 model_bytes = resource_archive.Size(resource_load_cursor.asset);
            if (model_bytes < 232u)
            {
                fprintf(stderr, "P2: Model prototype asset is shorter than its root\n");
                abort();
            }
            uint8 *model_data = (uint8 *)malloc(model_bytes);
            if (model_data == 0)
            {
                fprintf(stderr, "P2: Cannot allocate native model prototype\n");
                abort();
            }
            resource_archive.ReadAsset(resource_load_cursor.asset, model_data, 1, resource_load_cursor.container_asset);
            free(object->model_vertices[index]);
            object->model_vertices[index] = 0;
            free(object->model_polygons[index]);
            object->model_polygons[index] = 0;
            free(object->model_data[index]);
            object->model_data[index] = model_data;
            object->model_bytes[index] = model_bytes;
            object->models[index] = 0;
            AnimatedObject *model_object;
            flags = xport_load_le32(model_data + 28);
            byte = model_data[13];
            xport_store_le32(model_data + 28, flags | 0x20024);
            model_data[13] = byte | 0x80;
            BindModelMaterials(&object->models[index], &object->model_data[index], &object->material_resources[index], object->model_bytes[index], &object->model_polygons[index], &object->model_vertices[index]);
            model_object = object->models[index];
            if ((sint32)resource_archive.Size(resource_load_cursor.asset + 1) <= 0)
            {
                free(object->animation_data[index]);
                if (object->animation_headers[index] != 0)
                {
                    for (uint32 header_index = 0; header_index < 1; ++header_index)
                    {
                        for (uint32 track_group = 0; track_group < 4; ++track_group)
                            free(object->animation_headers[index][header_index].tracks[track_group].bytes);
                    }
                }
                free(object->animation_headers[index]);
                object->animation_data[index] = 0;
                object->animation_headers[index] = 0;
                object->animation_bytes[index] = 0;
                model_object->animation = 0;
                model_object->unbound_animation_word = 0;
            }
            else
            {
                slot = resource_archive.Offset(resource_load_cursor.asset + 1);
                uint32 animation_bytes = resource_archive.Size(resource_load_cursor.asset + 1);
                if (animation_bytes < sizeof(EncodedAnimationHeader))
                {
                    fprintf(stderr, "P2: Invalid attached animation asset extent\n");
                    abort();
                }
                uint8 *animation_data = (uint8 *)malloc(animation_bytes);
                AnimationHeader *header_storage = (AnimationHeader *)calloc(1, sizeof(AnimationHeader));
                if (animation_data == 0 || header_storage == 0)
                {
                    fprintf(stderr, "P2: Cannot allocate native attached animation\n");
                    abort();
                }
                resource_archive.ReadAsset(resource_load_cursor.asset + 1, animation_data, 1, resource_load_cursor.container_asset);
                free(object->animation_data[index]);
                if (object->animation_headers[index] != 0)
                {
                    for (uint32 header_index = 0; header_index < 1; ++header_index)
                    {
                        for (uint32 track_group = 0; track_group < 4; ++track_group)
                            free(object->animation_headers[index][header_index].tracks[track_group].bytes);
                    }
                }
                free(object->animation_headers[index]);
                object->animation_data[index] = animation_data;
                object->animation_headers[index] = header_storage;
                object->animation_bytes[index] = animation_bytes;
                model_object->AttachAnimation(animation_data, animation_bytes, header_storage);
            }
        }
        resource_load_cursor.asset = resource_load_cursor.asset + 3;
    }
    free(metadata_input);
    return 0;
}

uint32 LoadActorModel(uint32 index)
{
    FUNCTION_MARKER(0x8003420C, "SLUS_005.78");
    uint32 asset, counter, saved_flags, value;
    sint32 callback_index;
    asset = resource_load_cursor.asset;
    counter = actor_models.count;
    actor_models.count = counter + 1;
    ActorModelDefinition *definition = actor_models.definitions + (sint8)counter;
    value = resource_archive.Size(asset + 4);
    if ((sint32)value <= 0)
        definition->sound_table = 0;
    else
    {
        uint32 sample_count, sample_index;
        const uint8 *sound_data;
        SoundTableHeader *sound_table;
        if (value < sizeof(SoundTableHeader))
        {
            fprintf(stderr, "P2: Truncated actor sound table\n");
            abort();
        }
        uint8 *sound_input = (uint8 *)malloc(value);
        if (sound_input == 0)
        {
            fprintf(stderr, "P2: Cannot allocate native sound table input\n");
            abort();
        }
        resource_archive.ReadAsset(resource_load_cursor.asset + 4, sound_input, 1, resource_load_cursor.container_asset);
        sound_data = sound_input;
        definition->sound_bank.header.count = xport_load_le32(sound_data);
        sample_count = definition->sound_bank.header.count;
        if ((sint32)sample_count > 0)
        {
            if (sample_count > SoundBankStorage::SampleCapacity || sample_count > (value - sizeof(SoundTableHeader)) / sizeof(SoundSample))
            {
                fprintf(stderr, "P2: Actor sound table exceeds its native bank or resource\n");
                abort();
            }
            for (sample_index = 0; sample_index < sample_count; ++sample_index)
            {
                SoundSample *sample = definition->sound_bank.samples + sample_index;
                const uint8 *encoded = sound_data + sizeof(SoundTableHeader) + sample_index * sizeof(SoundSample);
                sample->spu_address = xport_load_le32(encoded);
                sample->pitch = xport_load_le16(encoded + 4);
                sample->metadata = xport_load_le16(encoded + 6);
            }
        }
        sound_table = definition->sound_bank.header.Relocate(value, definition->sound_bank.samples);
        definition->sound_table = sound_table != 0 ? &definition->sound_bank : 0;
        free(sound_input);
    }
    uint32 metadata_bytes = resource_archive.Size(asset);
    if (metadata_bytes < 672)
    {
        fprintf(stderr, "P2: Truncated actor model metadata\n");
        abort();
    }
    uint8 *metadata_input = (uint8 *)malloc(metadata_bytes);
    if (metadata_input == 0)
    {
        fprintf(stderr, "P2: Cannot allocate actor model metadata\n");
        abort();
    }
    resource_archive.ReadAsset(asset, metadata_input, 1, resource_load_cursor.container_asset);
    const uint8 *record = metadata_input;
    for (uint32 action_index = 0; action_index < 6; ++action_index)
    {
        const uint8 *encoded = record + 140 + action_index * 80;
        ActorActionDefinition *action = definition->actions + action_index;
        action->flags = xport_load_le16(encoded + 0);
        action->metadata2 = xport_load_le16(encoded + 2);
        action->distance_threshold = xport_load_le32(encoded + 4);
        action->priority_mode = encoded[8];
        action->trigger = encoded[9];
        action->priority_frame = encoded[10];
        action->metadata11 = encoded[11];
        action->trigger_ticks = (sint16)xport_load_le16(encoded + 12);
        action->metadata14 = xport_load_le16(encoded + 14);
        action->animation = (sint16)xport_load_le16(encoded + 16);
        action->path_motion_mode = (sint16)xport_load_le16(encoded + 18);
        action->hold_ticks = (sint16)xport_load_le16(encoded + 20);
        action->hold_animation = (sint16)xport_load_le16(encoded + 22);
        action->hold_speed = xport_load_le32(encoded + 24);
        action->hold_motion = (sint16)xport_load_le16(encoded + 28);
        action->motion = (sint16)xport_load_le16(encoded + 30);
        action->speed = xport_load_le32(encoded + 32);
        action->switch_frame = (sint16)xport_load_le16(encoded + 36);
        action->next_motion = (sint16)xport_load_le16(encoded + 38);
        action->next_speed = xport_load_le32(encoded + 40);
        action->projectile_kind = (sint16)xport_load_le16(encoded + 44);
        action->projectile_frame = (sint16)xport_load_le16(encoded + 46);
        action->projectile_height = xport_load_le32(encoded + 48);
        action->projectile_interval = xport_load_le16(encoded + 52);
        action->projectile_end_frame = xport_load_le16(encoded + 54);
        action->aim_mode = (sint16)xport_load_le16(encoded + 56);
        action->metadata58 = xport_load_le16(encoded + 58);
        action->spread_angle = xport_load_le32(encoded + 60);
        action->projectile_offset = xport_load_le32(encoded + 64);
        for (uint32 metadata_index = 0; metadata_index < 3; ++metadata_index)
            action->metadata68[metadata_index] = xport_load_le32(encoded + 68 + metadata_index * 4);
    }
    for (uint32 animation_index = 0; animation_index < 14; ++animation_index)
    {
        if (xport_load_le32(record + 72 + animation_index * 4) != 0)
        {
            fprintf(stderr, "P2: Actor metadata contains a prebound animation address\n");
            abort();
        }
        definition->animations[animation_index] = 0;
    }
    definition->kind = (sint8)record[61];
    definition->kind_override = (sint8)record[633];
    definition->initial_health = (sint8)record[60];
    definition->target_range[0] = (sint32)xport_load_le32(record + 4);
    definition->target_range[1] = (sint32)xport_load_le32(record + 8);
    definition->height_offset = (sint32)xport_load_le32(record + 16);
    definition->patrol_radius = (sint32)xport_load_le32(record + 20);
    definition->shot_interval = (sint16)xport_load_le16(record + 64);
    definition->shot_frame = (sint16)xport_load_le16(record + 66);
    definition->path_flag = (sint8)record[627];
    definition->fixed_pitch = (sint16)xport_load_le16(record + 640);
    definition->pitch_limit = (sint16)xport_load_le16(record + 642);
    definition->target_avoidance_mode = (sint16)xport_load_le16(record + 644);
    definition->health_restore_count = (sint16)xport_load_le16(record + 662);

    definition->contact_damage = record[622];
    definition->motion_speed[0] = (sint32)xport_load_le32(record + 28);
    definition->motion_speed[1] = (sint32)xport_load_le32(record + 32);
    definition->motion_accel = (sint32)xport_load_le32(record + 36);
    definition->sprite_extent[0] = (sint32)xport_load_le32(record + 40);
    definition->sprite_extent[1] = (sint32)xport_load_le32(record + 44);
    definition->turn_step = (sint16)xport_load_le16(record + 68);
    definition->reaction_motion_mode = (sint16)xport_load_le16(record + 636);
    definition->turn_motion_mode = (sint16)xport_load_le16(record + 652);
    definition->instance_cost = xport_load_le16(record + 70);
    definition->sound_radius = (sint16)xport_load_le16(record + 132);
    definition->contact_mode = xport_load_le16(record + 624);
    definition->fog_color[0] = record[628];
    definition->fog_color[1] = record[629];
    definition->fog_color[2] = record[630];
    definition->action_damage = xport_load_le16(record + 650);
    definition->sound_interval = xport_load_le16(record + 134);
    definition->interaction_limit_step = (sint16)xport_load_le16(record + 646);
    definition->interaction_delay_bias = xport_load_le16(record + 648);
    definition->interaction_animation_mode = (sint16)xport_load_le16(record + 654);
    definition->interaction_frame = xport_load_le16(record + 656);
    definition->route_action = (sint16)xport_load_le16(record + 658);
    definition->turnaround_range = (sint16)xport_load_le16(record + 660);
    definition->action_counter_flags = record[136];
    callback_index = definition->kind;
    definition->constructor = actor_constructors[callback_index < 44 ? callback_index : callback_index + 1];
    callback_index = definition->kind_override;
    if (callback_index != -1)
        definition->constructor = actor_constructors[callback_index];
    asset = resource_load_cursor.asset;
    saved_flags = xport_load_le32(record + 48);
    {
        uint32 material_bytes = resource_archive.Size(asset + 3);
        if (material_bytes < sizeof(EncodedMaterialResourceHeader))
        {
            fprintf(stderr, "P2: Truncated nested material asset\n");
            abort();
        }
        uint8 *material_input = (uint8 *)malloc(material_bytes);
        if (material_input == 0)
        {
            fprintf(stderr, "P2: Cannot allocate nested material input\n");
            abort();
        }
        resource_archive.ReadAsset(asset + 3, material_input, 1, resource_load_cursor.container_asset);
        RelocateMaterialResource(&definition->material_resource, material_input, material_bytes, -1);
        free(material_input);
    }
    uint32 model_bytes = resource_archive.Size(resource_load_cursor.asset + 1);
    if (model_bytes < 232u)
    {
        fprintf(stderr, "P2: Model prototype asset is shorter than its root\n");
        abort();
    }
    uint8 *model_data = (uint8 *)malloc(model_bytes);
    if (model_data == 0)
    {
        fprintf(stderr, "P2: Cannot allocate native model prototype\n");
        abort();
    }
    resource_archive.ReadAsset(resource_load_cursor.asset + 1, model_data, 1, resource_load_cursor.container_asset);
    free(definition->model_vertices);
    definition->model_vertices = 0;
    free(definition->model_polygons);
    definition->model_polygons = 0;
    free(definition->model_data);
    definition->model_data = model_data;
    definition->model_bytes = model_bytes;
    definition->prototype = 0;
    BindModelMaterials(&definition->prototype, &definition->model_data, &definition->material_resource, definition->model_bytes, &definition->model_polygons, &definition->model_vertices);
    uint32 animation_bytes = resource_archive.Size(resource_load_cursor.asset + 2);
    if (animation_bytes < 4)
    {
        fprintf(stderr, "P2: Invalid model animation asset extent\n");
        abort();
    }
    uint8 *animation_data = (uint8 *)malloc(animation_bytes);
    AnimationHeader *header_storage = (AnimationHeader *)calloc(14, sizeof(AnimationHeader));
    if (animation_data == 0 || header_storage == 0)
    {
        fprintf(stderr, "P2: Cannot allocate native model animation bank\n");
        abort();
    }
    resource_archive.ReadAsset(resource_load_cursor.asset + 2, animation_data, 1, resource_load_cursor.container_asset);
    free(definition->animation_data);
    if (definition->animation_headers != 0)
    {
        for (uint32 header_index = 0; header_index < 14; ++header_index)
        {
            for (uint32 track_group = 0; track_group < 4; ++track_group)
                free(definition->animation_headers[header_index].tracks[track_group].bytes);
        }
    }
    free(definition->animation_headers);
    free(definition->morph_restoration);
    definition->animation_data = animation_data;
    definition->animation_headers = header_storage;
    definition->animation_bytes = animation_bytes;
    LoadAnimationTable(definition->prototype, animation_data, animation_bytes, definition->animations, 14, header_storage, &definition->morph_restoration);
    AnimatedObject *prototype = (AnimatedObject *)(definition->prototype);
    prototype->node.flags |= 0x80;
    prototype = (AnimatedObject *)(definition->prototype);
    prototype->flags |= 0x20024 | saved_flags;
    prototype = (AnimatedObject *)(definition->prototype);
    asset = resource_load_cursor.asset;
    value = prototype->clone_bytes;
    resource_load_cursor.asset = asset + 5;
    definition->instance_cost = (uint16)value;
    free(metadata_input);
    return 1;
}

void RegisterAnimatedModelOperations(void)
{
    FUNCTION_MARKER(0x800441E8, "SLUS_005.78");
    {
        ObjectCallback operation = {0, &AnimatedObject::UpdateAnimatedModelPathMotion};
        object_operations.motions[3] = operation;
    }
    {
        ObjectCallback operation = {0, &AnimatedObject::UpdateAnimatedModelState};
        object_operations.behaviors[22] = operation;
    }
    {
        ObjectCallback operation = {0, &AnimatedObject::HideAnimatedModel};
        object_operations.variants[4] = operation;
    }
    object_operations.collisions[4] = &AnimatedObject::HandleModelCollision;
}

void LoadCommonModelGroups(void)
{
    FUNCTION_MARKER(0x8002D4D0, "SLUS_005.78");
    {
        ObjectCallback operation = {0, 0, UpdateModelGroup};
        object_operations.behaviors[6] = operation;
    }
    LoadModelGroup(0);
    LoadModelGroup(11);
    LoadModelGroup(19);
}

void InitializeActorModels(void)
{
    FUNCTION_MARKER(0x80034B44, "SLUS_005.78");
    object_operations.collisions[2] = &AnimatedObject::CollectActorContact;
    {
        ObjectCallback operation = {0, &AnimatedObject::UpdateActorPathMotion};
        object_operations.motions[4] = operation;
    }
    {
        ObjectCallback operation = {0, &AnimatedObject::UpdateActorDeath};
        object_operations.variants[2] = operation;
    }
    {
        ObjectCallback operation = {0, &AnimatedObject::UpdateActorReactionSpin};
        object_operations.behaviors[16] = operation;
    }
    {
        ObjectCallback operation = {0, 0, 0, &AnimatedObject::UpdateActorReactionRecovery};
        object_operations.behaviors[17] = operation;
    }
    actor_models.count = 0;
}

void RegisterActorVelocityOperations(void)
{
    FUNCTION_MARKER(0x80030994, "SLUS_005.78");
    {
        ObjectCallback operation = {0, &AnimatedObject::UpdateActorVelocity};
        object_operations.motions[2] = operation;
    }
    object_operations.collisions[3] = &AnimatedObject::CollectModelGroupContact;
}
