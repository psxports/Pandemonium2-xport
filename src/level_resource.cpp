#include <stdio.h>
#include <stdlib.h>

#include "level_resource.h"
#include "game.h"

LevelResource level_resource = {0};

void LevelResource::LoadModelGrps() const
{
    FUNCTION_MARKER(0x8002D468, "SLUS_005.78");
    uint32 mask = model_grp_mask;
    uint32 bit = 0;
    while (mask != 0)
    {
        if ((mask & 1) != 0)
            ModelGroupRegistry::LoadModelGrp(bit);
        mask >>= 1;
        ++bit;
    }
}

void LevelResource::LoadActorModels() const
{
    FUNCTION_MARKER(0x80034A9C, "SLUS_005.78");
    uint32 result = ActorModelRegistry::ResetActorLoadState();
    uint32 mask = actor_masks[0];
    uint32 bit = 0;
    while (mask != 0)
    {
        result = mask & 1;
        if (result != 0)
            ActorModelRegistry::LoadActorModel(bit);
        mask >>= 1;
        ++bit;
    }
    mask = actor_masks[1];
    bit = 32;
    while (mask != 0)
    {
        result = mask & 1;
        if (result != 0)
            ActorModelRegistry::LoadActorModel(bit);
        mask >>= 1;
        ++bit;
    }
}

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
            uint8 *metadata_input = (uint8 *)malloc(metadata_bytes);
            resource_archive.ReadAsset(resource_load_cursor.asset, metadata_input, 1, resource_load_cursor.container_asset);
            uint32 count = animated_models.count;
            AnimatedModelDefinition *definition = animated_models.recs + count;
            const uint8 *rec = metadata_input;
            const EncodedAnimatedModelMetadata *metadata = (const EncodedAnimatedModelMetadata *)rec;
            definition->flags = metadata->flags;
            definition->kind_id = metadata->kind_id;
            definition->resource_byte3 = metadata->resource_byte3;
            definition->initial_speed = metadata->initial_speed;
            definition->speed_limit = metadata->speed_limit;
            definition->gravity = metadata->gravity;
            definition->damping = metadata->damping;
            definition->col_scale = metadata->col_scale;
            definition->impulse = metadata->impulse;
            for (uint32 animation_idx = 0; animation_idx < 2; ++animation_idx)
            {
                definition->animations[animation_idx] = 0;
            }
            definition->sound_radius = metadata->sound_radius;
            definition->resource_halfword46 = metadata->resource_halfword46;
            animated_models.count = count + 1;
            animated_models.definitions[count] = definition;
            definition->kind_id = bit;
            if ((sint32)resource_archive.Size(resource_load_cursor.asset + 2) > 0)
            {
                uint32 bytes = resource_archive.Size(resource_load_cursor.asset + 2);
                uint8 *sound_input = (uint8 *)malloc(bytes);
                resource_archive.ReadAsset(resource_load_cursor.asset + 2, sound_input, 1, resource_load_cursor.container_asset);
                const uint8 *encoded = sound_input;
                uint32 sample_count = ((const EncodedSoundHeader *)encoded)->count;
                definition->sound_bank.header.count = sample_count;
                if ((sint32)sample_count > 0)
                {
                    for (uint32 sample_idx = 0; sample_idx < sample_count; ++sample_idx)
                    {
                        const uint8 *rec = encoded + sizeof(SoundTableHeader) + sample_idx * sizeof(SoundSample);
                        SoundSample *sample = definition->sound_bank.samples + sample_idx;
                        const EncodedSoundSample *input_sample = (const EncodedSoundSample *)rec;
                        sample->spu_address = input_sample->spu_address;
                        sample->pitch = input_sample->pitch;
                        sample->metadata = input_sample->metadata;
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
                uint8 *animation_data = (uint8 *)malloc(animation_bytes);
                AnimationHeader *header_storage = (AnimationHeader *)calloc(2, sizeof(AnimationHeader));
                resource_archive.ReadAsset(resource_load_cursor.asset + 1, animation_data, 1, resource_load_cursor.container_asset);
                free(definition->animation_data);
                if (definition->animation_headers != 0)
                {
                    for (uint32 header_idx = 0; header_idx < 2; ++header_idx)
                    {
                        for (uint32 track_grp = 0; track_grp < 4; ++track_grp)
                            free(definition->animation_headers[header_idx].tracks[track_grp].bytes);
                    }
                }
                free(definition->animation_headers);
                free(definition->morph_restoration);
                definition->animation_data = animation_data;
                definition->animation_headers = header_storage;
                definition->animation_bytes = animation_bytes;
                AnimationHeader::LoadAnimationTable(0, animation_data, animation_bytes, definition->animations, 2, header_storage, &definition->morph_restoration);
            }
            free(metadata_input);
            resource_load_cursor.asset = resource_load_cursor.asset + 3;
        }
        mask >>= 1;
        ++bit;
    }
}

void LevelResource::LoadCharacterModels() const
{
    FUNCTION_MARKER(0x80055468, "SLUS_005.78");
    uint32 idx;
    for (idx = 0; idx < 15; ++idx)
    {
        if (idx >= 2)
        {
            if (((character_mask >> idx) & 1) != 0)
                CharacterModelRecord::LoadCharacterModel(idx);
        }
        else if (idx == 0)
        {
            uint32 flags = character_pad.motion.form_flags;
            AnimatedObject *model;
            uint32 state;
            character_pad.sound_voice = -1;
            character_pad.section_height_offset = 0;
            character_pad.model_load_reset = 0;
            character_pad.motion.form_flags = (flags & 0xF0FFFFFF) | 0x0F000000;
            {
                CharacterModelRecord *sel_model;
                {
                    uint32 model_idx = 0;
                    CharacterModelRecord *model_binding;
                    model_binding = character_pad.model_slots[model_idx];
                    sel_model = model_binding;
                }
                model = sel_model->object;
            }
            model->flags |= 0x10;
            state = (uint16)(character_pad.motion.form_flags >> 16) & 15;
            switch (state)
            {
                case 0:
                    if ((character_pad.secondary_flags & 0x20) != 0)
                    {
                        uint32 global_flags = player_interaction.flags;
                        flags = character_pad.secondary_flags;
                        player_interaction.flags = global_flags & 0xFFFFFFDB;
                        character_pad.secondary_flags = (flags & 0xFFFFFFDF) | 0x100;
                    }
                    break;
                case 1:
                    if ((character_pad.secondary_flags & 0x100) != 0)
                    {
                        character_pad.RemoveAction(6);
                        character_pad.EndInteraction();
                        character_pad.secondary_flags = character_pad.secondary_flags | 0x20;
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
                    if ((character_pad.secondary_flags & 0x20) != 0)
                    {
                        uint32 global_flags = player_interaction.flags;
                        flags = character_pad.secondary_flags;
                        player_interaction.flags = global_flags & 0xFFFFFFDB;
                        character_pad.secondary_flags = flags & 0xFFFFFFDF;
                    }
                    if ((game_flow.mode_flags & 0x200) == 0 && ((uint16)(character_pad.motion.form_flags >> 16) & 15) != 0 && (character_pad.secondary_flags & 0x100) != 0)
                    {
                        character_pad.RemoveAction(6);
                        character_pad.EndInteraction();
                    }
                reset_action:
                    flags = character_pad.weapon_state;
                    if ((flags & 0xFFFF) != 0)
                        character_pad.RemoveAction((flags >> 16) != 0 ? (flags >> 16) - 7 : 3);
                    character_pad.weapon_state = 0;
                    break;
                default:
                    break;
            }
            gameplay_input.prev_buttons = 0;
            gameplay_input.buttons = 0;
            character_pad.special_move_backup[1] = 0;
        }
    }
}
