#include "game.h"

unsigned int character_prev_contacts = 0;
unsigned int character_cur_contacts = 0;
unsigned int contact_free_head = 0;

uint32 InitializeContactPool(void)
{
    uint32 pointer = 0x8008B600;
    sint32 offset;
    FUNCTION_MARKER(0x8005BA34, "SLUS_005.78");
    for (offset = 240; offset >= 0; offset -= 8)
    {
        w_u32(0x8008B50C + (uint32)offset, pointer);
        pointer -= 8;
    }
    w_u32(0x8008B604, 0);
    contact_free_head = 0x8008B508;
    return 0x8008B508;
}

uint32 ContainsContact(uint32 head_offset, uint32 object_offset)
{
    FUNCTION_MARKER(0x8005BAAC, "SLUS_005.78");
    while (head_offset != 0)
    {
        if (r_u32(head_offset) == object_offset)
            return 1;
        head_offset = r_u32(head_offset + 4);
    }
    return 0;
}

uint32 QueueContact(uint32 *head, uint32 object_offset, uint32 empty_pool_node)
{
    FUNCTION_MARKER(0x8005BADC, "SLUS_005.78");
    uint32 free_head = contact_free_head;
    uint32 node = empty_pool_node;
    uint32 result;
    if (free_head != 0)
    {
        node = free_head;
        contact_free_head = r_u32(node + 4);
    }
    w_u32(node, object_offset);
    result = *head;
    w_u32(node + 4, result);
    *head = node;
    return result;
}

void ReleaseContacts(uint32 *head)
{
    FUNCTION_MARKER(0x8005BB14, "SLUS_005.78");
    uint32 node = *head;
    uint32 next, free_head;
    while (node != 0)
    {
        next = r_u32(node + 4);
        free_head = contact_free_head;
        contact_free_head = node;
        w_u32(node + 4, free_head);
        node = next;
    }
    *head = 0;
}

uint32 FinishCharacterContacts(void)
{
    FUNCTION_MARKER(0x80047C1C, "SLUS_005.78");
    uint32 node, object, result, value;
    sint32 kind;
    for (node = character_prev_contacts; node != 0; node = r_u32(node + 4))
    {
        value = character_cur_contacts;
        object = r_u32(node);
        if (ContainsContact(value, object) == 0)
        {
            object = r_u32(node);
            kind = r_s8(object + 15);
            if (kind == 53)
                p2_func_8005bb48((r_u32(object + 68) & 0xFFF) | 0x4000, 4);
            else if (kind == 54)
                p2_func_8005bb48((r_u32(object + 68) & 0xFFF) | 0x3000, 4);
        }
    }
    ReleaseContacts(&character_prev_contacts);
    result = character_cur_contacts;
    character_cur_contacts = 0;
    character_prev_contacts = result;
    return result;
}

uint32 CanCharacterContact(uint32 object_offset, uint32 other_offset)
{
    FUNCTION_MARKER(0x80047524, "SLUS_005.78");
    uint32 state = 0x800ABD68;
    uint32 category, event, mode, flags, value, parent, matrix, x, y, z;
    sint32 kind;
    if ((character_controller.flags & 0x4000000) != 0)
        return 0;
    category = r_u8(other_offset + 12);
    if (category == 2)
    {
        event = r_u8(other_offset + 15);
        switch (event)
        {
            case 2:
            case 4:
            case 8:
            case 9:
            case 18:
            case 19:
            case 20:
            case 22:
            case 23:
            case 26:
            case 27:
            case 28:
            case 30:
                return 1;
            case 3:
                return (HasPendingSceneSection(0) & 0xFF) != 0;
            case 5:
                flags = r_u32(object_offset + 28);
                if ((flags & 0x22000000) != 0)
                    return 0;
                if ((flags & 0x8000000) != 0 && ((uint16)(character_controller.motion.form_flags >> 16) & 0xF) == 4)
                    return 0;
                value = frame_clock.render_tick;
                return value >= r_u32(other_offset + 68);
            case 6:
                return (r_u32(object_offset + 28) & 0x400) != 0 && character_controller.motion.vertical_speed < 0;
            case 7:
                x = r_u32(object_offset + 92);
                y = r_u32(object_offset + 96);
                z = r_u32(object_offset + 100);
                parent = r_u32(object_offset + 132);
                if (parent != 0)
                {
                    matrix = r_u32(parent + 40);
                    if (matrix != 0 && (r_u32(object_offset + 28) & 0x2000) == 0)
                    {
                        x -= r_u32(matrix + 32);
                        matrix = r_u32(r_u32(object_offset + 132) + 40);
                        y -= r_u32(matrix + 36);
                        matrix = r_u32(r_u32(object_offset + 132) + 40);
                        z -= r_u32(matrix + 40);
                    }
                }
                parent = r_u32(other_offset + 68);
                matrix = r_u32(state + 688);
                return r_u32(parent + 36) != r_u32(matrix + 36) || x != 0 || y != 0 || z != 0;
            case 10:
            case 11:
                return character_controller.timers.contact_impulse_ticks == 0;
            case 15:
                return (r_u8(object_offset + 13) & 1) == 0;
            case 16:
                return (r_u8(object_offset + 13) & 1) != 0;
            case 17:
                if (r_s16(r_u32(0x8008A114) + 46) != 0)
                    return 0;
                return r_u16(0x8008A530) != 0;
            case 21:
                value = r_u32(other_offset + 68) + 1;
                if ((sint32)value < 2)
                    value = (character_controller.motion.form_flags >> 20) & 0xF;
                value = ResolveCharacterFormVariant((sint32)value, state);
                return value != ((uint16)(character_controller.motion.form_flags >> 16) & 0xF);
            default:
                mode = (r_u32(other_offset + 72) >> 12) & 0x1F;
                if ((sint32)mode - 1 < 3 || (uint32)r_u8(other_offset + 15) - 40 >= 16)
                    return 0;
                if ((r_u8(other_offset + 13) & 0x80) != 0 || (r_u8(other_offset + 71) & 0xFE) != 0)
                    return 0;
                switch (mode)
                {
                    case 4:
                        return character_controller.secondary_flags & 0x40;
                    case 5:
                    case 6:
                    case 15:
                    case 16:
                        return 1;
                    case 7:
                        return character_controller.motion.horizontal_speed != 0;
                    case 8:
                        return (gameplay_input.buttons & 0x8000) != 0 && (r_u32(object_offset + 28) & 0x8000000) == 0;
                    case 9:
                        return (gameplay_input.buttons & 0x2000) != 0 && (r_u32(object_offset + 28) & 0x8000000) == 0;
                    case 10:
                        if ((sint32)character_controller.flags >= 0)
                        {
                            value = (*control_bindings.active_scheme)[1];
                            return (gameplay_input.buttons & value) != 0;
                        }
                        return (gameplay_input.buttons & 0x40) != 0 && (r_u32(0x1F800388) & 0x40) == 0;
                    case 11:
                        if ((sint32)character_controller.flags < 0)
                            return 0;
                        return (gameplay_input.buttons >> 14) & 1;
                    case 12:
                        return (gameplay_input.buttons >> 12) & 1;
                    case 13:
                        return (gameplay_input.buttons >> 2) & 1;
                    case 14:
                        return (gameplay_input.buttons >> 3) & 1;
                    case 17:
                        value = (*control_bindings.active_scheme)[0];
                        return (gameplay_input.buttons & value) != 0;
                    case 18:
                        if (((uint16)(character_controller.motion.form_flags >> 16) & 0xF) != 1)
                            return 0;
                        return (sint8)character_controller.motion.state_code == 9;
                    default:
                        return 0;
                }
        }
    }
    if (category == 0)
    {
        kind = r_s8(other_offset + 126);
        if (kind == 2)
        {
            if (r_s8(other_offset + 125) != 24)
                return 1;
            if ((r_u32(r_u32(other_offset + 132) + 16) & 0x40) != 0)
                return 1;
        }
        if (kind == 3)
            return r_s8(other_offset + 120) != -1;
        return 0;
    }
    if (category != 8)
        return 0;
    if ((character_controller.secondary_flags & 0x240) != 0)
    {
        if (character_controller.pickup_count >= 12)
            return 0;
        value = r_u16(other_offset + 18) & 0x7F;
        return value < 6 || value == 8;
    }
    if ((r_u32(object_offset + 28) & 0x8000000) != 0 && ((uint16)(character_controller.motion.form_flags >> 16) & 0xF) == 4)
        return 0;
    return character_controller.pickup_count < 12;
}

void QueueCharacterPickup(uint32 object_offset, uint32 context_offset)
{
    FUNCTION_MARKER(0x80047D18, "SLUS_005.78");
    sint32 index = character_controller.pickup_count;
    if (index < 12)
    {
        uint8 old_count;
        character_controller.pickup_objects[index] = object_offset;
        old_count = (uint8)character_controller.pickup_count;
        character_controller.pickup_count = old_count + 1;
        character_controller.pickup_fractions[(sint8)old_count] = context_offset;
    }
}

uint32 CollectCharacterContact(uint32 object_offset, uint32 other_offset, CollisionContact *contact_data)
{
    FUNCTION_MARKER(0x80047D6C, "SLUS_005.78");
    uint32 saved_stack = p2_guest_stack_pointer;
    uint32 state = 0x800ABD68;
    uint32 descriptor, contact, category, event, flags, value, mask, parent, matrix, x, y, z, index, count, result = 0;
    sint32 status, kind;
    if (contact_data == 0)
        return (uint32)(sint32)(sint8)CanCharacterContact(object_offset, other_offset);
    p2_guest_stack_pointer -= 0x40;
    descriptor = p2_guest_stack_pointer + 0x10;
    category = r_u8(other_offset + 12);
    contact = contact_data->fraction;
    if (category == 1)
        goto surface_descriptor;
    if (category == 0)
        goto ordinary_contact;
    if (category == 8)
    {
        index = (uint32)(sint32)character_controller.pickup_count;
        if ((sint32)index < 12)
        {
            character_controller.pickup_objects[(sint32)index] = other_offset;
            count = (uint8)character_controller.pickup_count;
            character_controller.pickup_count = count + 1;
            character_controller.pickup_fractions[(sint8)count] = contact;
        }
        goto done;
    }
    if (category != 2)
        goto done;
    QueueContact(&character_cur_contacts, other_offset, 2);
    if (ContainsContact(character_prev_contacts, other_offset) != 0 && ((r_u32(other_offset + 72) >> 12) & 0x1F) == 6)
        goto done;
    event = r_u8(other_offset + 15);
    switch (event)
    {
        case 1:
        case 24:
            if ((TestCharacterSurfaceContact(object_offset, other_offset, contact) & 0xFF) == 0)
                goto done;
            flags = character_controller.flags;
            if ((flags & 1) != 0 && (sint32)contact < r_s32(state + 672))
                goto done;
            value = character_controller.secondary_flags;
            character_controller.flags = flags | 1;
            w_u32(state + 188, other_offset);
            w_u32(state + 672, contact);
            if (event == 1)
                character_controller.secondary_flags = value & 0xFFFFFFF6;
            else
            {
                character_controller.secondary_flags = (value | 1) & 0xFFFFFFF7;
                if ((r_u32(object_offset + 228) & 6) != 2)
                    w_u32(state + 200, other_offset);
                if ((character_controller.secondary_flags & 4) != 0)
                    w_u8(state + 242, r_u8(state + 241));
            }
            goto done;
        case 2:
            flags = character_controller.flags;
            w_u32(state + 176, other_offset);
            w_u32(state + 652, contact);
            character_controller.flags = flags | 0x4000;
            goto done;
        case 3:
        case 17:
            value = r_u32(other_offset + 80);
            if (value == 87)
            {
                w_u32(0x8008A47C, 30);
                w_u32(0x8008A508, 15);
            }
            else if (value == 66)
            {
                w_u32(0x8008A470, 30);
                w_u32(0x8008A4D4, 15);
            }
            flags = character_controller.flags | 2;
            mask = 0xFCC5F45B;
            goto moving_event;
        case 4:
            flags = character_controller.flags;
            w_u32(state + 120, other_offset);
            w_u32(state + 552, contact);
            character_controller.flags = flags | 8;
            goto done;
        case 5:
        case 18:
            flags = character_controller.flags;
            w_u32(state + 180, other_offset);
            w_u32(state + 668, contact);
            character_controller.flags = flags | 0x400;
            goto done;
        case 6:
            character_controller.flags = (character_controller.flags | 0x10) & 0xFCC5F459;
            w_u32(state + 184, other_offset);
            value = contact_data->selection & 0x8FFF;
            parent = r_u32(state + 184);
            goto indexed_event;
        case 7:
            flags = character_controller.flags | 0x20;
            mask = 0xFCC5F479;
            goto moving_event;
        case 8:
            QueueCutsceneTrigger(other_offset, contact);
            goto done;
        case 9:
            flags = character_controller.flags;
            if (contact == 0 && (flags & 0x40000) != 0)
            {
                character_controller.flags = flags | 0x20000;
                goto done;
            }
            character_controller.flags = ((flags | 0x20000) & 0xFCC7F459) & 0xFFFBFFFF;
            w_u32(state + 116, other_offset);
            value = contact_data->selection & 0x8FFF;
            parent = r_u32(state + 116);
            goto indexed_event;
        case 10:
            flags = character_controller.flags;
            w_u32(state + 124, other_offset);
            w_u32(state + 656, contact);
            character_controller.flags = flags | 0x800000;
            goto done;
        case 11:
            flags = character_controller.flags;
            value = character_controller.secondary_flags;
            w_u32(state + 124, other_offset);
            w_u32(state + 656, contact);
            character_controller.flags = flags | 0x800000;
            character_controller.secondary_flags = value | 0x80;
            goto done;
        case 15:
            flags = character_controller.flags;
            w_u32(state + 192, other_offset);
            w_u32(state + 660, contact);
            character_controller.flags = flags | 0x1000000;
            goto done;
        case 16:
            flags = character_controller.flags;
            w_u32(state + 196, other_offset);
            w_u32(state + 664, contact);
            character_controller.flags = flags | 0x2000000;
            goto done;
        case 19:
            if (contact != 0)
                goto queue_event;
            parent = r_u32(0x8008A114);
            value = r_u32(other_offset + 68);
            w_u32(parent + 344, value);
            w_u32(parent + 340, value);
            goto done;
        case 20:
            if (contact != 0)
                goto queue_event;
            w_u8(state + 714, r_u32(other_offset + 68));
            goto done;
        case 21:
            if (contact != 0)
                goto queue_event;
            value = r_u32(other_offset + 68) + 1;
            if ((sint32)value < 2)
                value = (character_controller.motion.form_flags >> 20) & 0xF;
            value = ResolveCharacterFormVariant((sint32)value, state) << 24;
            character_controller.motion.form_flags = (character_controller.motion.form_flags & 0xF0FFFFFF) | value;
            goto done;
        case 22:
            if (contact == 0)
                w_u32(object_offset + 28, r_u32(object_offset + 28) | 0x40000);
            goto done;
        case 23:
            if (contact != 0)
                goto queue_event;
            w_u16(object_offset + 14, r_u32(other_offset + 68));
            goto done;
        case 26:
        case 27:
            if (r_u8(state + 294) != 0)
                goto done;
            flags = r_u32(object_offset + 228);
            if ((flags & 0x80) != 0 || (character_controller.flags & 0x60000) != 0)
                goto done;
            if (((r_u32(object_offset + 28) & 0x400) == 0 || r_u32(state + 272) < 10) && (flags & 1) == 0)
                goto done;
            parent = r_u32(r_u32(other_offset + 68) + 36);
            if (parent == r_u32(object_offset + 132) && (r_u32(object_offset + 28) & 0x2000) == 0)
                goto done;
            flags = r_u32(object_offset + 228);
            if (event == 26)
                flags = (flags | 0x80) & 0xFFFFFBFF;
            else
                flags |= 0x480;
            w_u32(object_offset + 228, flags);
            w_u32(state + 256, other_offset);
            w_u32(state + 260, contact);
            w_u8(state + 252, 0);
            goto done;
        case 28:
            goto queue_event;
        case 30:
            flags = character_controller.secondary_flags | 0x400;
            character_controller.secondary_flags = flags;
            value = r_u32(other_offset + 68);
            parent = character_controller.model_offset + value * 4;
            x = r_u32(object_offset + 104);
            if (x == r_u32(parent + 80))
                w_u16(object_offset + 130, 0);
            else if ((flags & 0x80) == 0)
                character_controller.motion.state_code = value;
            goto done;
        default:
            if (contact != 0)
                goto queue_event;
            w_u8(other_offset + 14, r_u8(other_offset + 14) | 8);
            goto done;
    }
moving_event:
    character_controller.flags = flags & mask;
    w_u32(state + 116, other_offset);
    x = r_u32(other_offset + 48);
    y = r_u32(other_offset + 52);
    z = r_u32(other_offset + 56);
    w_u32(state + 36, x);
    w_u32(state + 40, y);
    w_u32(state + 44, z);
    result = 1;
    goto done;
indexed_event:
    w_u16(state + 678, value);
    x = r_u32(parent + 48);
    y = r_u32(parent + 52);
    z = r_u32(parent + 56);
    w_u32(state + 36, x);
    w_u32(state + 40, y);
    w_u32(state + 44, z);
    result = 1;
    goto done;
queue_event:
    index = (uint32)(sint32)character_controller.trigger_count;
    if ((sint32)index < 12)
    {
        character_controller.trigger_objects[(sint32)index] = other_offset;
        count = (uint8)character_controller.trigger_count;
        character_controller.trigger_count = count + 1;
        character_controller.trigger_fractions[(sint8)count] = contact;
    }
    goto done;
surface_descriptor:
    p2_func_8001c544(other_offset, contact_data->selection & 0xFFFF8FFF, descriptor);
    if ((p2_func_8001c710(object_offset, descriptor, contact_data->selection) & 0xFF) != 0)
        goto done;
    character_controller.flags = (character_controller.flags | 4) & 0xFCC5F45D;
    w_u32(state + 116, other_offset);
    w_u32(object_offset + 112, contact_data->selection);
    x = r_u32(descriptor + 36);
    y = r_u32(descriptor + 40);
    z = r_u32(descriptor + 44);
    w_u32(state + 36, x);
    w_u32(state + 40, y);
    w_u32(state + 44, z);
    result = 1;
    goto done;
ordinary_contact:
    flags = r_u32(other_offset + 28);
    if ((flags & 0x100000) != 0)
        goto surface_descriptor;
    if ((flags & 0x200) != 0 && r_s8(other_offset + 126) == 2)
        goto done;
    if (r_s8(other_offset + 125) == 17)
        goto moving_platform;
    kind = r_s8(other_offset + 126);
    if (kind == 2)
    {
        if ((r_u32(other_offset + 28) & 0x30000000) != 0)
            goto moving_platform;
        kind = r_s8(other_offset + 126);
    }
    if (kind == 3 && r_u32(other_offset + 116) == 30)
    {
        w_u32(state + 500, 4096);
        goto done;
    }
    status = ClassifyCharacterContactResponse(object_offset);
    kind = r_s8(other_offset + 126);
    if (kind != 2)
        goto fallback;
    if (status == 0 && (character_controller.secondary_flags & 0x100) != 0 && character_action_target_offset == other_offset && (character_action_flags & 8) != 0)
    {
        status = -1;
        kind = r_s8(other_offset + 126);
    }
    if (kind != 2)
        goto fallback;
    if (status == 0 && (r_u32(object_offset + 228) & 1) != 0)
        status = p2_func_8005b604(object_offset, other_offset);
    if (r_s8(other_offset + 126) != 2)
        goto fallback;
    if (status == 0 && p2_func_80030a34(other_offset) != 0)
        status = -1;
    if (contact == 0 && (r_u32(object_offset + 28) & 0x2000000) != 0)
        goto fallback;
    if ((contact_data->flags & 1) == 0 && (r_s8(other_offset + 125) != 20 || (r_u32(other_offset + 28) & 0x40) != 0))
        goto fallback;
    flags = r_u32(object_offset + 28);
    if ((flags & 0x8000000) != 0 && ((uint16)(character_controller.motion.form_flags >> 16) & 0xF) == 4)
        goto fallback;
    if (status > 0)
        goto slide_contact;
    if (contact_data->other_selection < 0x800 && status == 0)
        goto attach_contact;
    if ((flags & 0x10000000) != 0 || (r_u32(other_offset + 28) & 0x4400000) == 0x400000)
        goto fallback;
slide_contact:
    x = r_u32(other_offset + 92);
    y = r_u32(other_offset + 96);
    z = r_u32(other_offset + 100);
    w_u32(state + 36, x);
    w_u32(state + 40, y);
    w_u32(state + 44, z);
    if ((r_u32(other_offset + 28) & 0x2000) == 0)
    {
        parent = r_u32(other_offset + 132);
        if (parent != 0)
        {
            matrix = r_u32(parent + 40);
            if (matrix != 0)
            {
                w_u32(state + 36, r_u32(state + 36) + r_u32(matrix + 32));
                matrix = r_u32(r_u32(other_offset + 132) + 40);
                w_u32(state + 40, r_u32(state + 40) + r_u32(matrix + 36));
                matrix = r_u32(r_u32(other_offset + 132) + 40);
                w_u32(state + 44, r_u32(state + 44) + r_u32(matrix + 40));
            }
        }
    }
    if ((r_u32(object_offset + 28) & 0x400) != 0)
    {
        character_controller.flags = (character_controller.flags | 0x800) & 0xFCC5FC59;
        w_u32(0x8008A180, p2_func_80030a34(other_offset));
        result = contact != 0;
        goto done;
    }
    if (p2_func_80030a34(other_offset) == 0)
    {
        if (r_s8(state + 276) <= 0)
            p2_func_80044ffc(7, 3);
        if (status >= 0 || r_s16(other_offset + 128) == 1)
        {
            result = status == 0;
            goto done;
        }
    }
    contact_data->flags = contact_data->flags & 0xFFFFFFFD;
fallback:
    if (status > 0)
        goto done;
attach_contact:
    if ((contact_data->flags & 2) == 0)
        goto ordinary_attachment;
    flags = r_u32(object_offset + 28);
    if ((flags & 0x2000000) != 0 || ((flags & 0x8000000) != 0 && ((uint16)(character_controller.motion.form_flags >> 16) & 0xF) == 4))
        goto ordinary_attachment;
    character_controller.flags = (character_controller.flags | 0x400) & 0xFCC5F459;
    w_u32(state + 180, other_offset);
    kind = r_s8(other_offset + 126);
    if (kind == 2)
        w_u8(state + 496, p2_func_80034eac(other_offset));
    else if (kind == 3)
        w_u8(state + 496, p2_func_8002d750(r_u32(other_offset + 116)));
    goto copy_attachment;
ordinary_attachment:
    flags = character_controller.flags;
    if ((flags & 0x40000) != 0 || (r_u32(object_offset + 28) & 0x2000000) != 0)
        goto done;
    character_controller.flags = ((flags | 0x20000) & 0xFCC7F459) & 0xFFFBFFFF;
    w_u32(state + 180, other_offset);
copy_attachment:
    x = r_u32(other_offset + 92);
    y = r_u32(other_offset + 96);
    z = r_u32(other_offset + 100);
    w_u32(state + 36, x);
    w_u32(state + 40, y);
    w_u32(state + 44, z);
    parent = r_u32(other_offset + 132);
    if (parent != 0)
    {
        matrix = r_u32(parent + 40);
        if (matrix != 0 && (r_u32(other_offset + 28) & 0x2000) == 0)
        {
            w_u32(state + 36, r_u32(state + 36) + r_u32(matrix + 32));
            matrix = r_u32(r_u32(other_offset + 132) + 40);
            w_u32(state + 40, r_u32(state + 40) + r_u32(matrix + 36));
            matrix = r_u32(r_u32(other_offset + 132) + 40);
            w_u32(state + 44, r_u32(state + 44) + r_u32(matrix + 40));
        }
    }
    result = r_s8(other_offset + 126) != 3;
    goto done;
moving_platform:
    if ((r_u32(object_offset + 28) & 0x400) == 0)
        goto done;
    character_controller.flags = (character_controller.flags | 0x200800) & 0xFCE5FC59;
    matrix = r_u32(r_u32(other_offset + 132) + 40);
    result = 1;
    if (matrix != 0)
    {
        w_u32(state + 36, r_u32(state + 36) + r_u32(matrix + 32));
        matrix = r_u32(r_u32(other_offset + 132) + 40);
        w_u32(state + 40, r_u32(state + 40) + r_u32(matrix + 36));
        matrix = r_u32(r_u32(other_offset + 132) + 40);
        w_u32(state + 44, r_u32(state + 44) + r_u32(matrix + 40));
    }
done:
    p2_guest_stack_pointer = saved_stack;
    return result;
}

uint32 ProcessCharacterPickups(uint32 object_offset, uint32 character_offset)
{
    CharacterModelRecord *state_record;
    CharacterController *character = &character_controller;
    FUNCTION_MARKER(0x800506D4, "SLUS_005.78");
    uint32 saved_stack, values, event, kind, marker, result, index, value, flags, total, state, action, previous, form;
    sint32 amount, first, second, third;
    first = character_controller.pickup_count;
    value = (uint8)character_controller.pickup_count;
    if (first == 0)
        return 0;
    saved_stack = p2_guest_stack_pointer;
    p2_guest_stack_pointer -= 0x20;
    values = p2_guest_stack_pointer + 0x10;
    result = value - 1;
    do
    {
        character_controller.pickup_count = result;
        index = (uint32)(sint32)(sint8)result;
        value = character_controller.pickup_fractions[(sint32)index];
        if (r_s32(object_offset + 144) < (sint32)value)
            goto next_event;
        first = r_s8(0x8008A194);
        second = r_s8(0x8008A195);
        third = r_s8(0x8008A196);
        w_u8(values, first);
        w_u8(values + 1, second);
        w_u8(values + 2, third);
        index = (uint32)(sint32)character_controller.pickup_count;
        event = character_controller.pickup_objects[(sint32)index];
        kind = r_u8(event + 18) & 0x7F;
        value = r_u8(event + 13);
        w_u8(event + 13, value | 0x80);
        marker = kind;
        switch (kind)
        {
            case 0:
                value = character_controller.vitals.lives;
                if (value < 32)
                    character_controller.vitals.lives = value + 1;
                break;
            case 1:
                first = (sint8)character_controller.vitals.health;
                second = (sint8)character_controller.vitals.health_limit;
                if (first < second)
                    character_controller.vitals.health = first + 1;
                break;
            case 2:
                if ((sint8)character_controller.vitals.health_limit < 21)
                {
                    value = r_u16(r_u32(0x8008A4E4) + 262);
                    if (value == 75)
                        character_controller.vitals.health_pickups = character_controller.vitals.health_pickups | 1;
                    else if (value == 10)
                        character_controller.vitals.health_pickups = character_controller.vitals.health_pickups | 2;
                    else if (value == 20)
                        character_controller.vitals.health_pickups = character_controller.vitals.health_pickups | 4;
                    character_controller.vitals.health_limit = character_controller.vitals.health_limit + 4;
                }
                first = (sint8)character_controller.vitals.health;
                second = (sint8)character_controller.vitals.health_limit;
                if (second - 4 >= first)
                    character_controller.vitals.health = first + 4;
                break;
            case 3:
            case 4:
            case 5:
                index = r_u16(event + 18) & 0x7F;
                state = r_u32(0x8008A4E8);
                amount = r_s8(values + index - 3);
                w_u16(state + 96, r_u16(state + 96) + amount);
                flags = character_controller.flags;
                total = character_controller.vitals.score;
                total += (flags & 0x400000) != 0 ? (uint32)amount * 2 : (uint32)amount;
                index = (uint32)(sint32)character_controller.pickup_count;
                character_controller.vitals.score = total;
                if (character_controller.pickup_fractions[(sint32)index] == 0 && (player_interaction.flags & 1) != 0)
                {
                    flags = character_controller.flags;
                    total += (flags & 0x400000) != 0 ? (uint32)amount * 2 : (uint32)amount;
                    character_controller.vitals.score = total;
                }
                break;
            case 6:
                if (((uint16)(character_controller.motion.form_flags >> 16) & 15) == 1)
                {
                    state = character_controller.model_offset;
                    state_record = (CharacterModelRecord *)psx_addr(state, 240);
                    value = state_record->animation_offsets[22];
                    previous = state_record->animation_offsets[2];
                    state_record->animation_offsets[2] =  value;
                    state = character_controller.model_offset;
                    state_record = (CharacterModelRecord *)psx_addr(state, 240);
                    state_record->animation_offsets[22] =  previous;
                    SpawnActionEffect(object_offset, character_offset, 7);
                    character_controller.secondary_flags = character_controller.secondary_flags | 0x40;
                    w_u32(object_offset + 28, r_u32(object_offset + 28) | 0x20000000);
                    character_controller.timers.model_swap_ticks = 450;
                    break;
                }
                goto change_form;
            case 7:
                if (((uint16)(character_controller.motion.form_flags >> 16) & 15) == 1)
                {
                    character_controller.secondary_flags = character_controller.secondary_flags | 0x200;
                    w_u32(object_offset + 28, r_u32(object_offset + 28) | 0x20000000);
                    character_controller.timers.color_effect_ticks = 450;
                    break;
                }
            change_form:
                kind = (uint16)(character_controller.motion.form_flags >> 16) & 15;
                if (kind == 4 || kind == 5 || kind - 10 < 2)
                {
                    marker = 0xFFFFFFFF;
                    break;
                }
                form = marker == 7 ? 11 : 0;
                marker = 6;
                if ((character_controller.secondary_flags & 0x100) != 0)
                {
                    character->EndInteraction();
                    character_controller.RemoveAction(6);
                }
                else
                {
                    value = character_controller.weapon_state;
                    if ((value & 0xFFFF) != 0)
                    {
                        previous = value >> 16;
                        if (previous == form)
                            goto store_form;
                        character_controller.RemoveAction(previous == 0 ? 3 : previous - 7);
                    }
                }
                SpawnActionEffect(object_offset, character_offset, form == 0 ? 3 : form - 7);
            store_form:
                character_controller.weapon_state = (form << 16) | 1;
                break;
            case 8:
                marker -= 2;
                break;
            case 10:
                flags = character_controller.secondary_flags;
                if ((flags & 0x120) == 0)
                {
                    if (((uint16)(character_controller.motion.form_flags >> 16) & 15) == 1)
                        character_controller.secondary_flags = flags | 0x20;
                    else
                    {
                        SpawnActionEffect(object_offset, character_offset, 6);
                        BeginCharacterInteraction(object_offset, character_offset);
                        value = character_controller.weapon_state;
                        if ((value & 0xFFFF) != 0)
                        {
                            previous = value >> 16;
                            character_controller.RemoveAction(previous == 0 ? 3 : previous - 7);
                        }
                        character_controller.weapon_state = 0;
                    }
                }
                break;
            case 11:
                SpawnActionEffect(object_offset, character_offset, 3);
                character_controller.weapon_state = 1;
                break;
        }
        while (character_controller.vitals.score >= 500)
        {
            value = character_controller.vitals.lives;
            if (value < 32)
                character_controller.vitals.lives = value + 1;
            value = character_controller.vitals.score;
            character_controller.vitals.score = value - 500;
            p2_func_80044ffc(17, 3);
        }
        p2_func_8002a80c(event + 40, r_u32(event + 24), 6);
        if ((sint8)marker != -1)
        {
            kind = r_u16(event + 18) & 0x7F;
            switch (kind)
            {
                case 0:
                    action = 17;
                    break;
                case 1:
                case 2:
                    action = 18;
                    break;
                case 3:
                case 4:
                case 5:
                    action = 19;
                    break;
                case 6:
                case 7:
                    action = 20;
                    break;
                case 8:
                    action = 21;
                    break;
                default:
                    action = ((uint16)(character_controller.motion.form_flags >> 16) & 15) == 1 ? 11 : 15;
                    break;
            }
            p2_func_80044ffc(action, 3);
        }
        if (r_u16(event + 16) != 0)
            w_u16(event + 18, r_u16(event + 18) | 0x8000);
    next_event:
        first = character_controller.pickup_count;
        value = (uint8)character_controller.pickup_count;
        result = value - 1;
    } while (first != 0);
    p2_guest_stack_pointer = saved_stack;
    return result;
}

uint32 ProcessCharacterTriggers(uint32 object_offset, uint32 character_offset)
{
    CharacterController *character = &character_controller;
    FUNCTION_MARKER(0x80050F60, "SLUS_005.78");
    uint32 flags, object_flags, event, state, value, previous, index, count, scene, result;
    sint32 kind, health, active;
    flags = character_action_flags;
    if ((flags & 0x100) != 0)
    {
        state = character_action_target_offset;
        state = r_u32(state + 132);
        state = r_u32(state + 44);
        value = character_controller.flags;
        state = r_u32(state + 72);
        character_controller.flags = value | 0x400;
        value = r_u16(state + 650);
        character_action_flags = flags & 0xFFFFFEFF;
        w_u8(character_offset + 496, value);
        goto take_damage;
    }
    if ((character_controller.flags & 0x80000) != 0)
    {
        object_flags = r_u32(object_offset + 28);
        if ((object_flags & 0x2000000) == 0 && (object_flags & 0x4400) != 0x400)
        {
            w_u8(character_offset + 496, 1);
            goto take_damage;
        }
    }
    flags = character_controller.flags;
    if ((flags & 0x400) == 0)
        goto timed_flags;
    event = r_u32(character_offset + 180);
    if (r_u8(event + 12) != 0)
    {
        value = r_u32(character_offset + 668);
        if (r_s32(object_offset + 144) < (sint32)value)
            goto timed_flags;
        if (r_s8(event + 15) == 18)
        {
            flags = character_controller.flags;
            character_controller.vitals.health = 0;
            character_controller.flags = flags | 0x80;
        }
        else
        {
            value = r_u32(event + 76);
            event = r_u32(character_offset + 180);
            w_u8(character_offset + 496, value);
            value = r_u16(event + 78);
            value += frame_clock.render_tick;
            w_u32(event + 68, value);
        }
    }
    else if ((r_u32(object_offset + 28) & 0x2000400) != 0x2000000)
        character_controller.flags = flags | 0x20000;
    if ((sint8)character_controller.vitals.health <= 0 || (r_u32(object_offset + 28) & 0x2000000) != 0)
        goto timed_flags;
take_damage:
    if ((game_flow.transition_flags & 0x10000) != 0 && (uint32)r_u16(r_u32(0x8008A4E4) + 262) - 96 >= 2 && (r_u32(object_offset + 28) & 0x22000000) == 0)
    {
        value = r_u8(character_offset + 496);
        health = (sint8)character_controller.vitals.health;
        if ((sint32)value >= health)
        {
            value = (uint32)health;
            if (health >= 2)
                --value;
        }
        character_controller.vitals.health = character_controller.vitals.health - value;
    }
    p2_func_80044ffc(2, 3);
    value = r_u32(character_offset + 8);
    flags = game_flow.mode_flags;
    w_u32(character_offset + 8, value | 0x4000000);
    if ((flags & 0x10) == 0)
    {
        if ((character_controller.secondary_flags & 0x20) != 0)
        {
            value = player_interaction.flags;
            flags = character_controller.secondary_flags;
            player_interaction.flags = value & 0xFFFFFFDB;
            character_controller.secondary_flags = flags & 0xFFFFFFDF;
        }
        if (((game_flow.mode_flags & 0x200) == 0 || ((uint16)(character_controller.motion.form_flags >> 16) & 15) != 0) && (character_controller.secondary_flags & 0x100) != 0)
        {
            character_controller.RemoveAction(6);
            character->EndInteraction();
        }
        value = character_controller.weapon_state;
        if ((value & 0xFFFF) != 0)
        {
            previous = value >> 16;
            character_controller.RemoveAction(previous == 0 ? 3 : previous - 7);
        }
        character_controller.weapon_state = 0;
    }
    if ((r_u32(object_offset + 28) & 0x2000000) == 0)
    {
        character_controller.vitals.damage_ticks = 60;
        w_u32(object_offset + 28, r_u32(object_offset + 28) | 0x2080000);
    }
    if ((character_controller.flags & 0x80000) == 0)
    {
        SpawnActionEffect(object_offset, character_offset, 0);
        if ((r_u32(object_offset + 228) & 0x101) == 0 && (character_controller.secondary_flags & 0x80) == 0)
            character_controller.motion.state_code = 4;
    }
    character_controller.flags = character_controller.flags & 0xEFFFFFFF;
timed_flags:
    if ((character_controller.flags & 0x800000) != 0)
    {
        value = r_u32(character_offset + 656);
        if (r_s32(object_offset + 144) < (sint32)value)
        {
            value = character_controller.secondary_flags;
            flags = character_controller.flags;
            character_controller.secondary_flags = value & 0xFFFFFF7F;
            character_controller.flags = flags & 0xFF7FFFFF;
        }
    }
    if ((character_controller.flags & 8) != 0)
    {
        value = r_u32(character_offset + 552);
        if (r_s32(object_offset + 144) < (sint32)value)
        {
            flags = character_controller.flags;
            w_u32(character_offset + 552, 0);
            character_controller.flags = flags & 0xFFFFFFF7;
        }
    }
    flags = character_controller.flags;
    if ((flags & 0x3000000) == 0x3000000)
    {
        value = r_u32(character_offset + 660);
        previous = r_u32(character_offset + 664);
        if ((sint32)previous >= (sint32)value && r_s32(object_offset + 144) >= (sint32)value)
            character_controller.flags = flags & 0xFDFFFFFF;
        else
        {
            previous = r_u32(character_offset + 664);
            value = r_u32(character_offset + 660);
            if ((sint32)value >= (sint32)previous && r_s32(object_offset + 144) >= (sint32)previous)
                character_controller.flags = character_controller.flags & 0xFEFFFFFF;
        }
    }
    if ((character_controller.flags & 0x1000000) != 0)
    {
        value = r_u32(character_offset + 660);
        if (r_s32(object_offset + 144) >= (sint32)value)
        {
            w_u8(object_offset + 13, r_u8(object_offset + 13) | 1);
            goto timed_scene;
        }
    }
    if ((character_controller.flags & 0x2000000) != 0)
    {
        value = r_u32(character_offset + 664);
        if (r_s32(object_offset + 144) >= (sint32)value)
            w_u8(object_offset + 13, r_u8(object_offset + 13) & 0xFE);
    }
timed_scene:
    if ((character_controller.flags & 0x4000) != 0)
    {
        value = r_u32(character_offset + 652);
        if (r_s32(object_offset + 144) >= (sint32)value && (RequestSceneSection(r_u32(r_u32(character_offset + 176) + 68), 0) << 24) != 0)
        {
            scene = r_u32(0x8008A4E8);
            w_u16(scene + 18, r_u16(scene + 18) & 0xFFEF);
        }
    }
    active = character_controller.trigger_count;
    count = (uint8)character_controller.trigger_count;
    while (active != 0)
    {
        value = count - 1;
        character_controller.trigger_count = value;
        index = (uint32)(sint32)(sint8)value;
        value = character_controller.trigger_fractions[(sint32)index];
        if (r_s32(object_offset + 144) >= (sint32)value)
        {
            event = character_controller.trigger_objects[(sint32)index];
            if (event != 0)
            {
                kind = r_s8(event + 15);
                if (kind >= 40)
                    w_u8(event + 14, r_u8(event + 14) | 8);
                else
                {
                    switch (kind)
                    {
                        case 19:
                            scene = r_u32(0x8008A114);
                            value = r_u32(event + 68);
                            w_u32(scene + 344, value);
                            w_u32(scene + 340, value);
                            break;
                        case 20:
                            w_u8(character_offset + 714, r_u32(event + 68));
                            break;
                        case 23:
                            w_u16(object_offset + 14, r_u32(event + 68));
                            break;
                        case 21:
                            value = r_u32(event + 68) + 1;
                            if ((sint32)value < 2)
                                value = (character_controller.motion.form_flags >> 20) & 15;
                            result = ResolveCharacterFormVariant((sint32)value, character_offset);
                            value = character_controller.motion.form_flags;
                            character_controller.motion.form_flags = (value & 0xF0FFFFFF) | (result << 24);
                            break;
                    }
                }
            }
        }
        active = character_controller.trigger_count;
        count = (uint8)character_controller.trigger_count;
    }
    return StartPendingCutscene(object_offset);
}

uint32 TestCharacterSurfaceContact(uint32 object_offset, uint32 other_offset, uint32 contact)
{
    FUNCTION_MARKER(0x800471F4, "SLUS_005.78");
    uint32 x, y, z, parent, matrix, saved_stack, bounds, delta, extent, result, value;
    if (r_s8(other_offset + 15) == 24 && (r_u32(object_offset + 228) & 4) != 0 && r_u32(0x800ABE34) == other_offset)
    {
        value = r_u8(0x8008A230);
        if (r_u32(0x800ABE78) < value)
            return 0;
    }
    if (contact != 0)
        return 1;
    if ((r_u32(object_offset + 28) & 0x2800) != 0)
        return 1;
    x = 0 - r_u32(other_offset + 48);
    y = 0 - r_u32(other_offset + 52);
    z = 0 - r_u32(other_offset + 56);
    parent = r_u32(object_offset + 132);
    if (parent != 0)
    {
        matrix = r_u32(parent + 40);
        if (matrix != 0 && (r_u32(object_offset + 28) & 0x2000) == 0)
        {
            x += r_u32(matrix + 32);
            matrix = r_u32(r_u32(object_offset + 132) + 40);
            y += r_u32(matrix + 36);
            matrix = r_u32(r_u32(object_offset + 132) + 40);
            z += r_u32(matrix + 40);
        }
    }
    if (x != 0 || y != 0 || z != 0)
        return 1;
    saved_stack = p2_guest_stack_pointer;
    p2_guest_stack_pointer -= 48;
    bounds = p2_guest_stack_pointer;
    delta = bounds + 16;
    extent = bounds + 32;
    w_u32(extent, contact);
    x = r_u32(object_offset + 160);
    y = r_u32(object_offset + 164);
    z = r_u32(object_offset + 168);
    w_u32(bounds, x);
    w_u32(bounds + 4, y);
    w_u32(bounds + 8, z);
    w_u32(bounds + 12, 0);
    x = r_u32(object_offset + 92);
    x -= r_u32(other_offset + 48);
    w_u32(delta, x);
    y = r_u32(object_offset + 96);
    y -= r_u32(other_offset + 52);
    w_u32(delta + 4, y);
    z = r_u32(object_offset + 100);
    z -= r_u32(other_offset + 56);
    w_u32(delta + 8, z);
    result = p2_func_80038990(bounds, other_offset, delta, extent, 0) != 0;
    p2_guest_stack_pointer = saved_stack;
    return result;
}

uint32 ResolveCharacterFormVariant(sint32 kind, uint32 character_offset)
{
    FUNCTION_MARKER(0x800473F0, "SLUS_005.78");
    uint32 current;
    uint32 compatible;
    if (kind != 6 && kind != 7 && kind != 8 && kind != 13)
        return (uint32)kind;
    current = (character_controller.motion.form_flags >> 16) & 15;
    compatible = current == 0 || current == 6 || current == 8 || current == 13 || current == 10;
    switch (kind)
    {
        case 6:
            return compatible ? 6 : 7;
        case 7:
            return compatible ? 8 : 9;
        case 8:
            return compatible ? 10 : 11;
        default:
            return compatible ? 13 : 14;
    }
}

sint32 ClassifyCharacterContactResponse(uint32 object_offset)
{
    CharacterController *character = &character_controller;
    FUNCTION_MARKER(0x80047B1C, "SLUS_005.78");
    uint32 flags = r_u32(object_offset + 28);
    uint32 character_flags, value;
    sint32 timer;
    if ((flags & 0x20000000) != 0 || (character->secondary_flags & 0x240) != 0)
    {
        character_flags = character->secondary_flags;
        if ((character_flags & 0x40) != 0)
            return 4;
        if ((character_flags & 0x200) != 0)
            return 5;
        timer = (sint16)character_controller.vitals.damage_ticks;
        if (timer >= 150)
            return 3;
        if (timer >= 75)
            value = (uint32)timer & 7;
        else if (timer >= 37)
            value = (uint32)timer & 3;
        else
            value = (uint32)timer & 1;
        return value == 0 ? 2 : 3;
    }
    if ((flags & 0x4000000) != 0 && ((character_controller.motion.form_flags >> 16) & 0xF) == 1)
        return -1;
    if ((sint8)character_controller.motion.prev_state_code == 9)
        return ((character_controller.motion.form_flags >> 16) & 0xF) == 1 ? -1 : 0;
    return 0;
}
