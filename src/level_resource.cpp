#include <stdio.h>
#include <stdlib.h>

#include "game.h"

LevelResource level_resource = {0};

void LevelResource::LoadAnimatedModels() const
{
    FUNCTION_MARKER(0x800440A8, "SLUS_005.78");
    uint32 mask = animated_model_mask;
    uint32 bit = 0;
    while (mask != 0)
    {
        if ((mask & 1) != 0)
        {
            uint32 metadata_bytes = resource_archive.Size(resource_load_cursor.asset);
            if (metadata_bytes < 52)
            {
                fprintf(stderr, "P2: Truncated animated model metadata\n");
                abort();
            }
            uint8 *metadata_input = (uint8 *)malloc(metadata_bytes);
            if (metadata_input == 0)
            {
                fprintf(stderr, "P2: Cannot allocate animated model metadata\n");
                abort();
            }
            resource_archive.ReadAsset(resource_load_cursor.asset, metadata_input, 1, resource_load_cursor.container_asset);
            uint32 count = animated_models.count;
            AnimatedModelDefinition *definition = animated_models.records + count;
            const uint8 *record = metadata_input;
            definition->flags = xport_load_le16(record);
            definition->kind_id = (sint8)record[2];
            definition->metadata3 = record[3];
            definition->initial_speed = xport_load_le32(record + 4);
            definition->speed_limit = xport_load_le32(record + 8);
            definition->gravity = xport_load_le32(record + 12);
            definition->damping = xport_load_le32(record + 16);
            definition->collision_scale = xport_load_le32(record + 20);
            definition->impulse = xport_load_le32(record + 24);
            definition->metadata28[0] = xport_load_le32(record + 28);
            definition->metadata28[1] = xport_load_le32(record + 32);
            for (uint32 animation_index = 0; animation_index < 2; ++animation_index)
            {
                if (xport_load_le32(record + 36 + animation_index * 4) != 0)
                {
                    fprintf(stderr, "P2: Animated model metadata contains a prebound animation address\n");
                    abort();
                }
                definition->animations[animation_index] = 0;
            }
            definition->sound_radius = xport_load_le16(record + 44);
            definition->metadata46 = xport_load_le16(record + 46);
            animated_models.count = count + 1;
            animated_models.definitions[count] = definition;
            definition->kind_id = (sint8)bit;
            if ((sint32)resource_archive.Size(resource_load_cursor.asset + 2) > 0)
            {
                uint32 bytes = resource_archive.Size(resource_load_cursor.asset + 2);
                if (bytes < sizeof(SoundTableHeader))
                {
                    fprintf(stderr, "P2: Truncated animated model sound table\n");
                    abort();
                }
                uint8 *sound_input = (uint8 *)malloc(bytes);
                if (sound_input == 0)
                {
                    fprintf(stderr, "P2: Cannot allocate native sound table input\n");
                    abort();
                }
                resource_archive.ReadAsset(resource_load_cursor.asset + 2, sound_input, 1, resource_load_cursor.container_asset);
                const uint8 *encoded = sound_input;
                uint32 sample_count = xport_load_le32(encoded);
                definition->sound_bank.header.count = sample_count;
                if ((sint32)sample_count > 0)
                {
                    if (sample_count > SoundBankStorage::SampleCapacity || sample_count > (bytes - sizeof(SoundTableHeader)) / sizeof(SoundSample))
                    {
                        fprintf(stderr, "P2: Animated model sound samples exceed table capacity\n");
                        abort();
                    }
                    for (uint32 sample_index = 0; sample_index < sample_count; ++sample_index)
                    {
                        const uint8 *record = encoded + sizeof(SoundTableHeader) + sample_index * sizeof(SoundSample);
                        SoundSample *sample = definition->sound_bank.samples + sample_index;
                        sample->spu_address = xport_load_le32(record);
                        sample->pitch = xport_load_le16(record + 4);
                        sample->metadata = xport_load_le16(record + 6);
                    }
                }
                definition->sound_table = definition->sound_bank.header.Relocate(bytes, definition->sound_bank.samples) != 0 ? &definition->sound_bank : 0;
                free(sound_input);
            }
            else
                definition->sound_table = 0;
            if ((sint32)resource_archive.Size(resource_load_cursor.asset + 1) > 0)
            {
                uint32 animation_bytes = resource_archive.Size(resource_load_cursor.asset + 1);
                if (animation_bytes < 4)
                {
                    fprintf(stderr, "P2: Invalid model animation asset extent\n");
                    abort();
                }
                uint8 *animation_data = (uint8 *)malloc(animation_bytes);
                AnimationHeader *header_storage = (AnimationHeader *)calloc(2, sizeof(AnimationHeader));
                if (animation_data == 0 || header_storage == 0)
                {
                    fprintf(stderr, "P2: Cannot allocate native model animation bank\n");
                    abort();
                }
                resource_archive.ReadAsset(resource_load_cursor.asset + 1, animation_data, 1, resource_load_cursor.container_asset);
                free(definition->animation_data);
                if (definition->animation_headers != 0)
                {
                    for (uint32 header_index = 0; header_index < 2; ++header_index)
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
                LoadAnimationTable(0, animation_data, animation_bytes, definition->animations, 2, header_storage, &definition->morph_restoration);
            }
            free(metadata_input);
            resource_load_cursor.asset = resource_load_cursor.asset + 3;
        }
        mask >>= 1;
        ++bit;
    }
}

void LevelResource::LoadModelGroups() const
{
    FUNCTION_MARKER(0x8002D468, "SLUS_005.78");
    uint32 mask = model_group_mask;
    uint32 bit = 0;
    while (mask != 0)
    {
        if ((mask & 1) != 0)
            LoadModelGroup((uint8)bit);
        mask >>= 1;
        ++bit;
    }
}

uint32 LevelResource::LoadActorModels() const
{
    FUNCTION_MARKER(0x80034A9C, "SLUS_005.78");
    uint32 result = ResetActorLoadWorkspace();
    uint32 mask = actor_masks[0];
    uint32 bit = 0;
    while (mask != 0)
    {
        result = mask & 1;
        if (result != 0)
            result = LoadActorModel(bit);
        mask >>= 1;
        ++bit;
    }
    mask = actor_masks[1];
    bit = 32;
    while (mask != 0)
    {
        result = mask & 1;
        if (result != 0)
            result = LoadActorModel(bit);
        mask >>= 1;
        ++bit;
    }
    return result;
}

uint32 LevelResource::LoadCharacterModels() const
{
    FUNCTION_MARKER(0x80055468, "SLUS_005.78");
    uint32 index;
    for (index = 0; index < 15; ++index)
    {
        if (index >= 2)
        {
            if (((character_mask >> index) & 1) != 0)
                LoadCharacterModel(index);
        }
        else if (index == 0)
        {
            uint32 flags = character_controller.motion.form_flags;
            AnimatedObject *model;
            uint32 state;
            character_controller.sound_voice = (sint8)(0xFF);
            character_controller.section_height_offset = 0;
            character_controller.model_load_reset = 0;
            character_controller.motion.form_flags = (flags & 0xF0FFFFFF) | 0x0F000000;
            {
                CharacterModelRecord *selected_model;
                {
                    uint32 model_index = 0;
                    CharacterModelRecord *model_binding;
                    if (model_index < CharacterController::ModelCount)
                        model_binding = character_controller.model_slots[model_index];
                    else
                    {
                        fprintf(stderr, "P2: Invalid character model index %u\n", model_index);
                        abort();
                    }
                    if (model_binding == 0)
                    {
                        fprintf(stderr, "P2: Character model %u is not loaded\n", model_index);
                        abort();
                    }
                    selected_model = model_binding;
                }
                model = (AnimatedObject *)(selected_model->object);
            }
            model->flags |= 0x10;
            state = (uint16)(character_controller.motion.form_flags >> 16) & 15;
            switch (state)
            {
                case 0:
                    if ((character_controller.secondary_flags & 0x20) != 0)
                    {
                        uint32 global_flags = player_interaction.flags;
                        flags = character_controller.secondary_flags;
                        player_interaction.flags = global_flags & 0xFFFFFFDB;
                        character_controller.secondary_flags = (flags & 0xFFFFFFDF) | 0x100;
                    }
                    break;
                case 1:
                    if ((character_controller.secondary_flags & 0x100) != 0)
                    {
                        character_controller.RemoveAction(6);
                        character_controller.EndInteraction();
                        character_controller.secondary_flags = character_controller.secondary_flags | 0x20;
                    }
                    goto reset_action;
                case 4:
                case 5:
                case 6:
                case 7:
                case 8:
                case 9:
                case 10:
                case 11:
                case 13:
                case 14:
                    if ((character_controller.secondary_flags & 0x20) != 0)
                    {
                        uint32 global_flags = player_interaction.flags;
                        flags = character_controller.secondary_flags;
                        player_interaction.flags = global_flags & 0xFFFFFFDB;
                        character_controller.secondary_flags = flags & 0xFFFFFFDF;
                    }
                    if ((game_flow.mode_flags & 0x200) == 0 && ((uint16)(character_controller.motion.form_flags >> 16) & 15) != 0 && (character_controller.secondary_flags & 0x100) != 0)
                    {
                        character_controller.RemoveAction(6);
                        character_controller.EndInteraction();
                    }
                reset_action:
                    flags = character_controller.weapon_state;
                    if ((flags & 0xFFFF) != 0)
                        character_controller.RemoveAction((flags >> 16) != 0 ? (flags >> 16) - 7 : 3);
                    character_controller.weapon_state = 0;
                    break;
                default:
                    break;
            }
            gameplay_input.prev_buttons = 0;
            gameplay_input.buttons = 0;
            character_controller.special_move_backup[1] = 0;
        }
    }
    return 0;
}
