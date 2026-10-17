#include "game.h"

void LevelResource::LoadAnimatedModels() const
{
    FUNCTION_MARKER(0x800440A8, "SLUS_005.78");
    uint32 mask = animated_model_mask;
    uint32 bit = 0;
    while (mask != 0)
    {
        if ((mask & 1) != 0)
        {
            uint32 offset = resource_archive.Offset(r_u32(0x8008A46C));
            uint32 count = r_u32(0x8008A338);
            uint32 entry = r_u32(0x8008A488) + offset;
            w_u32(0x8008A338, count + 1);
            w_u32(0x8008B4A8 + (count << 2), entry);
            w_u8(entry + 2, bit);
            if ((sint32)resource_archive.Size(r_u32(0x8008A46C) + 2) > 0)
            {
                offset = resource_archive.Offset(r_u32(0x8008A46C) + 2);
                w_u32(entry + 48, p2_func_8006aa48(r_u32(0x8008A488) + offset));
            }
            else
                w_u32(entry + 48, 0);
            if ((sint32)resource_archive.Size(r_u32(0x8008A46C) + 1) > 0)
            {
                offset = resource_archive.Offset(r_u32(0x8008A46C) + 1);
                LoadAnimationTable(0, r_u32(0x8008A488) + offset, (uint32 *)psx_addr(entry + 36, (r_u32(r_u32(0x8008A488) + offset) - 1) * 4));
            }
            w_u32(0x8008A46C, r_u32(0x8008A46C) + 3);
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
            p2_func_8002b058((uint8)bit);
        mask >>= 1;
        ++bit;
    }
}

uint32 LevelResource::LoadActorModels() const
{
    FUNCTION_MARKER(0x80034A9C, "SLUS_005.78");
    uint32 result = p2_func_80034f04();
    uint32 mask = actor_masks[0];
    uint32 bit = 0;
    while (mask != 0)
    {
        result = mask & 1;
        if (result != 0)
            result = p2_func_8003420c(bit);
        mask >>= 1;
        ++bit;
    }
    mask = actor_masks[1];
    bit = 32;
    while (mask != 0)
    {
        result = mask & 1;
        if (result != 0)
            result = p2_func_8003420c(bit);
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
            uint32 character = 0x800ABD68;
            uint32 flags = character_controller.motion.form_flags;
            uint32 model;
            uint32 state;
            character_controller.sound_voice = (sint8)(0xFF);
            w_u32(character + 784, 0);
            w_u32(character + 916, 0);
            character_controller.motion.form_flags = (flags & 0xF0FFFFFF) | 0x0F000000;
            model = ((CharacterModelRecord *)psx_addr(character_controller.model_slots[0], 240))->object_offset;
            w_u32(model + 28, r_u32(model + 28) | 0x10);
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
            w_u32(0x1F800388, 0);
            gameplay_input.buttons = 0;
            character_controller.model_slots[16] =  0;
        }
    }
    return 0;
}
