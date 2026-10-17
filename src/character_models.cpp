#include "game.h"

uint32 HideAlternateCharacterModel(void)
{
    CharacterModelRecord *object_record;
    FUNCTION_MARKER(0x80044474, "SLUS_005.78");
    uint32 index, object, result;
    if ((sint32)character_controller.flags >= 0)
        return 1;
    index = 1 - ((uint16)(character_controller.motion.form_flags >> 16) & 15);
    object = character_controller.model_slots[index];
    object_record = (CharacterModelRecord *)psx_addr(object, 240);
    object = object_record->object_offset;
    result = r_u8(object + 13) | 1;
    w_u8(object + 13, result);
    return result;
}

uint32 ReloadBaseCharacterModels(uint32 mode)
{
    CharacterModelRecord *object_record;
    FUNCTION_MARKER(0x80044508, "SLUS_005.78");
    uint32 saved_stack, objects, selector, base, first_asset, second_asset, asset;
    uint32 slot, index, object, header, model, pointer, value, selection, result;
    sint32 remaining;
    saved_stack = p2_guest_stack_pointer;
    p2_guest_stack_pointer -= 16;
    objects = p2_guest_stack_pointer;
    selector = objects + 8;
    base = r_u32(0x8008A424);
    value = game_flow.mode_flags;
    first_asset = 23;
    second_asset = 29;
    remaining = 1;
    slot = 0;
    index = 0;
    if ((value & 0x200) != 0)
    {
        first_asset = 41;
        second_asset = 35;
    }
    w_u32(0x8008A4FC, mode);
    asset = first_asset + 1;
    selection = p2_func_80044780();
    if (mode == 1)
    {
        pointer = r_u32(0x8008A424);
        w_u32(selector, 0);
        --remaining;
        ReadAssetToGuest(first_asset, pointer, 1);
    }
    else if ((sint32)mode < 2)
    {
        --remaining;
        if (mode == 0)
        {
            result = 1;
            goto finish;
        }
    }
    else if (mode == 2)
    {
        pointer = r_u32(0x8008A424);
        asset = second_asset + 1;
        slot = 1;
        w_u32(selector, 0);
        --remaining;
        ReadAssetToGuest(second_asset, pointer, 1);
    }
    else if (mode == 3)
    {
        ReadAssetToGuest(first_asset, r_u32(0x8008A424), 1);
        value = resource_archive.Size(first_asset);
        pointer = r_u32(0x8008A424);
        remaining = 2;
        w_u32(selector, selection);
        --remaining;
        ReadAssetToGuest(second_asset, pointer + value, 1);
    }
    else
        --remaining;
    while (remaining >= 0)
    {
        object = base + resource_archive.Offset(asset);
        object_record = (CharacterModelRecord *)psx_addr(object, 240);
        character_controller.model_slots[slot] =  object;
        if ((sint32)resource_archive.Size(asset + 4) > 0)
        {
            pointer = base + resource_archive.Offset(asset + 4);
            value = p2_func_8006aa48(pointer);
            object_record->sound_table_offset =  value;
        }
        else
            object_record->sound_table_offset =  0;
        header = base + resource_archive.Offset(asset + 3);
        RelocateMaterialResource(header, -1);
        model = base + resource_archive.Offset(asset + 1);
        object_record->object_offset =  model;
        w_u32(objects + index * 4, model);
        pointer = object_record->object_offset;
        w_u32(pointer + 28, r_u32(pointer + 28) | 0x20034);
        BindModelMaterials(object_record->object_offset, header);
        pointer = object_record->object_offset;
        value = (uint32)object_record->extent[0];
        pointer = r_u32(pointer + 156);
        w_u16(pointer + 40, (uint32)((sint32)value >> 5));
        pointer = object_record->object_offset;
        value = (uint32)object_record->extent[1];
        pointer = r_u32(pointer + 156);
        w_u16(pointer + 42, (uint32)((sint32)value >> 5));
        value = resource_archive.Offset(asset + 2);
        pointer = object_record->object_offset;
        ++index;
        LoadAnimationTable(pointer, base + value, object_record->animation_offsets);
        value = resource_archive.Size(first_asset);
        pointer = r_u32(0x8008A424);
        ++slot;
        --remaining;
        asset = second_asset + 1;
        base = pointer + value;
    }
    selection = r_u32(selector);
    result = r_u32(objects + (selection << 2));
    w_u32(0x8008A340, result);
finish:
    p2_guest_stack_pointer = saved_stack;
    return result;
}

uint32 RestoreCharacterModelAnimations(uint32 mode, uint32 scene_offset)
{
    CharacterModelRecord *result_record;
    CharacterModelRecord *target_record;
    FUNCTION_MARKER(0x80055280, "SLUS_005.78");
    uint32 character, target, first, second, result;
    character = 0x800ABD68 + mode * 928;
    character_controller.ClearTransientFlags();
    result = character_controller.secondary_flags & 0x40;
    if (result != 0)
    {
        target = character_controller.model_offset;
        target_record = (CharacterModelRecord *)psx_addr(target, 240);
        first = target_record->animation_offsets[22];
        second = target_record->animation_offsets[2];
        target_record->animation_offsets[2] =  first;
        result = character_controller.model_offset;
        result_record = (CharacterModelRecord *)psx_addr(result, 240);
        result_record->animation_offsets[22] =  second;
    }
    return result;
}

uint32 LoadCharacterModel(uint32 index)
{
    CharacterModelRecord *object_record;
    uint32 offset;
    uint32 object;
    uint32 header;
    uint32 model;
    uint32 result;
    FUNCTION_MARKER(0x800552FC, "SLUS_005.78");
    offset = resource_archive.Offset(r_u32(0x8008A46C));
    object = r_u32(0x8008A488) + offset;
    object_record = (CharacterModelRecord *)psx_addr(object, 240);
    character_controller.model_slots[index] =  object;
    if ((sint32)resource_archive.Size(r_u32(0x8008A46C) + 4) <= 0)
        object_record->sound_table_offset =  0;
    else
    {
        offset = resource_archive.Offset(r_u32(0x8008A46C) + 4);
        object_record->sound_table_offset =  p2_func_8006aa48(r_u32(0x8008A488) + offset);
    }
    offset = resource_archive.Offset(r_u32(0x8008A46C) + 3);
    header = r_u32(0x8008A488) + offset;
    RelocateMaterialResource(header, -1);
    offset = resource_archive.Offset(r_u32(0x8008A46C) + 1);
    model = r_u32(0x8008A488) + offset;
    object_record->object_offset =  model;
    w_u32(model + 28, r_u32(model + 28) | 0x20034);
    BindModelMaterials(object_record->object_offset, header);
    w_u16(r_u32(object_record->object_offset + 156) + 40, (uint32)(object_record->extent[0] >> 5));
    w_u16(r_u32(object_record->object_offset + 156) + 42, (uint32)(object_record->extent[1] >> 5));
    offset = resource_archive.Offset(r_u32(0x8008A46C) + 2);
    LoadAnimationTable(object_record->object_offset, r_u32(0x8008A488) + offset, object_record->animation_offsets);
    result = r_u32(0x8008A46C) + 5;
    w_u32(0x8008A46C, result);
    return result;
}



uint32 ResetCharacterModels(void)
{
    FUNCTION_MARKER(0x800556E0, "SLUS_005.78");
    uint32 object, flags, value, index;
    uint32 *entry;
    object = r_u32(0x8008A340);
    if (object != 0)
    {
        character_controller.ClearTransientFlags();
        character_controller.flags = character_controller.flags & 0xC0000000;
        flags = r_u32(object + 28);
        value = r_u32(object + 228);
        w_u32(object + 28, flags & 0xC0F793FF);
        w_u32(object + 228, value & 0xFC000000);
        flags = character_controller.secondary_flags;
        character_controller.motion.jump_phase = 0;
        character_controller.motion.vertical_speed = 0;
        character_controller.motion.horizontal_speed = 0;
        character_controller.secondary_flags = flags & 0xFFFFF932;
        w_u32(object + 100, 0);
        w_u32(object + 96, 0);
        w_u32(object + 92, 0);
        character_controller.timers.special_move_ticks = 0;
        character_controller.timers.transition_ticks = 0;
        character_controller.timers.score_bonus_ticks = 0;
        character_controller.timers.action_cooldown_ticks = 0;
        character_controller.vitals.damage_ticks = 0;
        character_controller.timers.state_ticks = 0;
        w_u32(0x800AC078, 0);
        character_controller.timers.contact_impulse_ticks = 0;
        w_u32(0x800ABF90, 0);
    }
    entry = character_controller.model_slots + 2;
    index = 2;
    do
    {
        if (*entry != 0)
            *entry = 0;
        value = character_controller.motion.form_flags;
        if (index == ((value >> 16) & 15))
            AssignCharacterModel(0x8008A340, 0x800ABD68, (value >> 20) & 15);
        ++index;
        ++entry;
    } while (index < 15);
    return 0;
}

uint32 SelectCharacterForm(uint32 mode)
{
    FUNCTION_MARKER(0x800558AC, "SLUS_005.78");
    uint32 flags = character_controller.motion.form_flags;
    uint32 value = ((flags & 0xFFF0FFFF) | (mode << 16)) & 0xFF0FFFFF;
    character_controller.motion.form_flags = value | (mode << 20);
    return (flags >> 20) & 0xF;
}

uint32 InitializeCharacterModels(void)
{
    uint32 index;
    uint32 offset;
    uint32 object;
    uint32 header;
    uint32 flags;
    uint32 next;
    uint8 byte;
    FUNCTION_MARKER(0x80055AC4, "SLUS_005.78");
    {
        ObjectCallback operation = {UpdateCharacterMovement, 0, 0};
        object_operations.motions[1] = operation;
    }
    {
        ObjectCallback operation = {p2_func_800530a0, 0, 0};
        object_operations.behaviors[1] = operation;
    }
    {
        ObjectCallback operation = {p2_func_80053704, 0, 0};
        object_operations.behaviors[33] = operation;
    }
    {
        ObjectCallback operation = {p2_func_80048db4, 0, 0};
        object_operations.variants[1] = operation;
    }
    object_operations.collisions[1] = &CollectCharacterContact;
    w_u8(0x8008A45C, 0);
    w_u16(0x8008A532, 0);
    {
        ObjectCallback operation = {p2_func_80045ed8, 0, 0};
        object_operations.behaviors[2] = operation;
    }
    InitializeContactPool();
    game_flow.transition_flags = game_flow.transition_flags | 0x10000;
    for (index = 0; index < 15; ++index)
    {
        if ((sint32)resource_archive.Size(r_u32(0x8008A46C)) > 0)
        {
            offset = resource_archive.Offset(r_u32(0x8008A46C));
            next = r_u32(0x8008A46C) + 2;
            object = r_u32(0x8008A488) + offset;
            action_effect_templates[index] = object;
            offset = resource_archive.Offset(next);
            header = r_u32(0x8008A488) + offset;
            RelocateMaterialResource(header, -1);
            flags = r_u32(object + 28);
            byte = r_u8(object + 13);
            w_u32(object + 28, flags | 0x20024);
            w_u8(object + 13, byte | 0x80);
            BindModelMaterials(object, header);
            offset = resource_archive.Offset(r_u32(0x8008A46C) + 1);
            AttachObjectAnimation(object, r_u32(0x8008A488) + offset);
        }
        next = r_u32(0x8008A46C) + 3;
        w_u32(0x8008A46C, next);
    }
    offset = resource_archive.Offset(next);
    w_u32(0x800B7A30, 0);
    w_u32(0x8008A424, r_u32(0x8008A488) + offset);
    LoadCharacterModel(0);
    if (game_flow.mode_flags & 0x200)
    {
        offset = resource_archive.Size(41);
        next = 35;
    }
    else
    {
        offset = resource_archive.Size(23);
        next = 29;
    }
    w_u32(0x800B7A34, offset);
    w_u32(0x800B7A38, resource_archive.Size(next));
    LoadCharacterModel(1);
    w_u32(0x8008A4FC, 3);
    w_u32(0x800B7A3C, r_u32(0x800B7A34) + r_u32(0x800B7A38));
    p2_func_800558f0();
    saved_material_registry = material_registry;
    {
        ObjectCallback operation = {p2_func_8002995c, 0, 0};
        object_operations.behaviors[26] = operation;
    }
    return 0x8002995C;
}

static void ClearPrimaryModelSlots(void)
{
    FUNCTION_MARKER(0x80055D6C, "SLUS_005.78");
    uint32 index;
    for (index = 0; index < 2; ++index)
    {
        uint32 *slot = character_controller.model_slots + index;
        if (*slot != 0)
            *slot = 0;
    }
    w_u32(0x8008A340, 0);
}

uint32 AssignCharacterModel(uint32 object_slot_offset, uint32 character_offset, uint32 kind)
{
    CharacterModelRecord *model_record;
    CharacterModelRecord *table_record;
    FUNCTION_MARKER(0x80055DC0, "SLUS_005.78");
    uint32 model = character_controller.model_slots[kind];
    model_record = (CharacterModelRecord *)psx_addr(model, 240);
    uint32 flags, current_kind, value, source, destination, table, index = 0;
    uint32 x, y, z;
    {
        ObjectCallback operation = {kind == 5 ? UpdateForm5Movement : UpdateCharacterMovement, 0, 0};
        object_operations.motions[1] = operation;
    }
    value = (character_controller.motion.form_flags & 0xFFF0FFFF) | (kind << 16);
    character_controller.motion.form_flags = (value & 0xF0FFFFFF) | 0x0F000000;
    flags = character_controller.flags;
    current_kind = (uint16)(character_controller.motion.form_flags >> 16) & 0xF;
    character_controller.flags = flags & 0xBFFFFFFF;
    switch (current_kind)
    {
        case 0:
            if ((character_controller.secondary_flags & 0x20) != 0)
            {
                value = player_interaction.flags;
                flags = character_controller.secondary_flags;
                player_interaction.flags = value & 0xFFFFFFDB;
                character_controller.secondary_flags = flags & 0xFFFFFFDF;
                if (r_u32(object_slot_offset) != 0)
                {
                    SpawnActionEffect(r_u32(object_slot_offset), character_offset, 6);
                    source = r_u32(object_slot_offset);
                }
                else
                {
                    SpawnActionEffect(model_record->object_offset, character_offset, 6);
                    source = model_record->object_offset;
                }
                BeginCharacterInteraction(source, character_offset);
            }
            break;
        case 1:
            if ((character_controller.secondary_flags & 0x100) != 0)
            {
                character_controller.RemoveAction(6);
                character_controller.EndInteraction();
                character_controller.secondary_flags = character_controller.secondary_flags | 0x20;
            }
            goto clear_action;
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
                value = player_interaction.flags;
                flags = character_controller.secondary_flags;
                player_interaction.flags = value & 0xFFFFFFDB;
                character_controller.secondary_flags = flags & 0xFFFFFFDF;
            }
            if ((character_controller.secondary_flags & 0x100) != 0)
            {
                character_controller.RemoveAction(6);
                character_controller.EndInteraction();
            }
        clear_action:
            value = r_u32(character_offset + 20);
            if ((value & 0xFFFF) != 0)
                character_controller.RemoveAction((value >> 16) != 0 ? (value >> 16) - 7 : 3);
            w_u32(character_offset + 20, 0);
            break;
        default:
            break;
    }
    source = r_u32(object_slot_offset);
    if (source == 0)
    {
        character_controller.timers.idle_ticks = 900;
        goto assign_model;
    }
    table = character_controller.model_offset;
    if (table != 0)
    {
        table_record = (CharacterModelRecord *)psx_addr(table, 240);
        value = r_u32(source + 104);
        while (value != table_record->animation_offsets[index])
        {
            ++index;
            if (index >= 38)
                break;
        }
        if (index >= 7)
            index = (r_u32(source + 28) & 0x400) != 0 ? 6 : 2;
        if (index == 38)
        {
            index = 0;
            character_controller.timers.idle_ticks = 900;
        }
    }
    else
        character_controller.timers.idle_ticks = 900;
    source = r_u32(object_slot_offset);
    destination = model_record->object_offset;
    w_u8(destination + 13, r_u8(source + 13));
    x = r_u32(source + 16);
    y = r_u32(source + 20);
    z = r_u32(source + 24);
    w_u32(destination + 16, x);
    w_u32(destination + 20, y);
    w_u32(destination + 24, z);
    w_u32(destination + 28, r_u32(source + 28));
    w_u32(destination + 228, r_u32(source + 228));
    x = r_u32(source + 60);
    y = r_u32(source + 64);
    z = r_u32(source + 68);
    w_u32(destination + 60, x);
    w_u32(destination + 64, y);
    w_u32(destination + 68, z);
    x = r_u32(source + 72);
    y = r_u32(source + 76);
    z = r_u32(source + 80);
    w_u32(destination + 72, x);
    w_u32(destination + 76, y);
    w_u32(destination + 80, z);
    x = r_u32(source + 84);
    y = r_u32(source + 88);
    w_u32(destination + 84, x);
    w_u32(destination + 88, y);
    x = r_u32(source + 92);
    y = r_u32(source + 96);
    z = r_u32(source + 100);
    w_u32(destination + 92, x);
    w_u32(destination + 96, y);
    w_u32(destination + 100, z);
    p2_func_8006637c((uint8 *)psx_addr(destination + 108, 12), (const uint8 *)psx_addr(source + 108, 12), 12);
    w_u8(destination + 124, r_u8(source + 124));
    w_u8(destination + 125, r_u8(source + 125));
    w_u8(destination + 126, r_u8(source + 126));
    w_u8(destination + 127, r_u8(source + 127));
    w_u16(destination + 128, r_u16(source + 128));
    w_u16(destination + 130, r_u16(source + 130));
    w_u32(destination + 132, r_u32(source + 132));
    w_u16(destination + 136, r_u16(source + 136));
    w_u16(destination + 148, r_u16(source + 148));
    w_u16(destination + 150, r_u16(source + 150));
    w_u16(destination + 152, r_u16(source + 152));
    w_u16(destination + 154, r_u16(source + 154));
    w_u32(destination + 144, r_u32(source + 144));
    p2_func_8006637c((uint8 *)psx_addr(destination + 160, 64), (const uint8 *)psx_addr(source + 160, 64), 64);
assign_model:
    table = character_controller.model_slots[kind];
    table_record = (CharacterModelRecord *)psx_addr(table, 240);
    character_controller.model_offset = table;
    destination = model_record->object_offset;
    w_u32(destination + 104, table_record->animation_offsets[index]);
    destination = model_record->object_offset;
    w_u32(destination + 28, r_u32(destination + 28) | 0x20000);
    destination = model_record->object_offset;
    w_u32(destination + 28, r_u32(destination + 28) & 0xFBFEFFFF);
    w_u32(object_slot_offset, model_record->object_offset);
    w_u32(0x8008A340, model_record->object_offset);
    p2_func_8006637c((uint8 *)&character_controller.movement, (const uint8 *)&table_record->movement, 60);
    value = (uint32)character_controller.movement.turn_frames;
    if ((sint32)value > 0)
        character_controller.turn_step =  2048 / (sint32)value;
    else
        character_controller.turn_step =  4096;
    return index;
}

uint32 GetCharacterModelObject(void)
{
    FUNCTION_MARKER(0x800562F8, "SLUS_005.78");
    uint32 offset = (character_controller.motion.form_flags >> 18) & 0x3C;
    return ((CharacterModelRecord *)psx_addr(character_controller.model_slots[(offset) / 4], 240))->object_offset;
}

void ClearCharacterModels(void)
{
    FUNCTION_MARKER(0x80060EBC, "SLUS_005.78");
    ClearPrimaryModelSlots();
    /* 80034BA0 returns without effects */
    w_u32(0x8008A320, 0);
}
