#include "game.h"

uint32 p2_func_80047d6c(uint32 object_offset, uint32 other_offset, uint32 collision_offset)
{
    uint32 saved_stack = p2_guest_stack_pointer;
    uint32 state = 0x800ABD68;
    uint32 descriptor, contact, category, event, flags, value, mask, parent, matrix, x, y, z, index, count, result = 0;
    sint32 status, kind;
    if (collision_offset == 0)
        return (uint32)(sint32)(sint8)p2_func_80047524(object_offset, other_offset);
    p2_guest_stack_pointer -= 0x40;
    descriptor = p2_guest_stack_pointer + 0x10;
    category = r_u8(other_offset + 12);
    contact = r_u32(collision_offset + 4);
    if (category == 1)
        goto surface_descriptor;
    if (category == 0)
        goto ordinary_contact;
    if (category == 8)
    {
        index = (uint32)(sint32)r_s8(0x800AC011);
        if ((sint32)index < 12)
        {
            w_u32(state + 504 + index * 4, other_offset);
            count = r_u8(0x800AC011);
            w_u8(0x800AC011, count + 1);
            w_u32(state + 604 + (uint32)(sint32)(sint8)count * 4, contact);
        }
        goto done;
    }
    if (category != 2)
        goto done;
    p2_func_8005badc(0x800AC104, other_offset, 2);
    if (p2_func_8005baac(r_u32(0x800AC100), other_offset) != 0 && ((r_u32(other_offset + 72) >> 12) & 0x1F) == 6)
        goto done;
    event = r_u8(other_offset + 15);
    switch (event)
    {
        case 1:
        case 24:
            if ((p2_func_800471f4(object_offset, other_offset, contact) & 0xFF) == 0)
                goto done;
            flags = r_u32(state);
            if ((flags & 1) != 0 && (sint32)contact < r_s32(state + 672))
                goto done;
            value = r_u32(state + 4);
            w_u32(state, flags | 1);
            w_u32(state + 188, other_offset);
            w_u32(state + 672, contact);
            if (event == 1)
                w_u32(state + 4, value & 0xFFFFFFF6);
            else
            {
                w_u32(state + 4, (value | 1) & 0xFFFFFFF7);
                if ((r_u32(object_offset + 228) & 6) != 2)
                    w_u32(state + 200, other_offset);
                if ((r_u32(state + 4) & 4) != 0)
                    w_u8(state + 242, r_u8(state + 241));
            }
            goto done;
        case 2:
            flags = r_u32(state);
            w_u32(state + 176, other_offset);
            w_u32(state + 652, contact);
            w_u32(state, flags | 0x4000);
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
            flags = r_u32(state) | 2;
            mask = 0xFCC5F45B;
            goto moving_event;
        case 4:
            flags = r_u32(state);
            w_u32(state + 120, other_offset);
            w_u32(state + 552, contact);
            w_u32(state, flags | 8);
            goto done;
        case 5:
        case 18:
            flags = r_u32(state);
            w_u32(state + 180, other_offset);
            w_u32(state + 668, contact);
            w_u32(state, flags | 0x400);
            goto done;
        case 6:
            w_u32(state, (r_u32(state) | 0x10) & 0xFCC5F459);
            w_u32(state + 184, other_offset);
            value = r_u32(collision_offset) & 0x8FFF;
            parent = r_u32(state + 184);
            goto indexed_event;
        case 7:
            flags = r_u32(state) | 0x20;
            mask = 0xFCC5F479;
            goto moving_event;
        case 8:
            p2_func_80026c54(other_offset, contact);
            goto done;
        case 9:
            flags = r_u32(state);
            if (contact == 0 && (flags & 0x40000) != 0)
            {
                w_u32(state, flags | 0x20000);
                goto done;
            }
            w_u32(state, ((flags | 0x20000) & 0xFCC7F459) & 0xFFFBFFFF);
            w_u32(state + 116, other_offset);
            value = r_u32(collision_offset) & 0x8FFF;
            parent = r_u32(state + 116);
            goto indexed_event;
        case 10:
            flags = r_u32(state);
            w_u32(state + 124, other_offset);
            w_u32(state + 656, contact);
            w_u32(state, flags | 0x800000);
            goto done;
        case 11:
            flags = r_u32(state);
            value = r_u32(state + 4);
            w_u32(state + 124, other_offset);
            w_u32(state + 656, contact);
            w_u32(state, flags | 0x800000);
            w_u32(state + 4, value | 0x80);
            goto done;
        case 15:
            flags = r_u32(state);
            w_u32(state + 192, other_offset);
            w_u32(state + 660, contact);
            w_u32(state, flags | 0x1000000);
            goto done;
        case 16:
            flags = r_u32(state);
            w_u32(state + 196, other_offset);
            w_u32(state + 664, contact);
            w_u32(state, flags | 0x2000000);
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
                value = (r_u32(state + 704) >> 20) & 0xF;
            value = p2_func_800473f0((sint32)value, state) << 24;
            w_u32(state + 704, (r_u32(state + 704) & 0xF0FFFFFF) | value);
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
            if ((flags & 0x80) != 0 || (r_u32(state) & 0x60000) != 0)
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
            flags = r_u32(state + 4) | 0x400;
            w_u32(state + 4, flags);
            value = r_u32(other_offset + 68);
            parent = r_u32(state + 780) + value * 4;
            x = r_u32(object_offset + 104);
            if (x == r_u32(parent + 80))
                w_u16(object_offset + 130, 0);
            else if ((flags & 0x80) == 0)
                w_u8(state + 712, value);
            goto done;
        default:
            if (contact != 0)
                goto queue_event;
            w_u8(other_offset + 14, r_u8(other_offset + 14) | 8);
            goto done;
    }
moving_event:
    w_u32(state, flags & mask);
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
    index = (uint32)(sint32)r_s8(state + 680);
    if ((sint32)index < 12)
    {
        w_u32(state + 128 + index * 4, other_offset);
        count = r_u8(state + 680);
        w_u8(state + 680, count + 1);
        w_u32(state + 556 + (uint32)(sint32)(sint8)count * 4, contact);
    }
    goto done;
surface_descriptor:
    p2_func_8001c544(other_offset, r_u32(collision_offset) & 0xFFFF8FFF, descriptor);
    if ((p2_func_8001c710(object_offset, descriptor, r_u32(collision_offset)) & 0xFF) != 0)
        goto done;
    w_u32(state, (r_u32(state) | 4) & 0xFCC5F45D);
    w_u32(state + 116, other_offset);
    w_u32(object_offset + 112, r_u32(collision_offset));
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
    status = p2_func_80047b1c(object_offset);
    kind = r_s8(other_offset + 126);
    if (kind != 2)
        goto fallback;
    if (status == 0 && (r_u32(state + 4) & 0x100) != 0 && r_u32(0x8008A510) == other_offset && (r_u32(0x8008A1A4) & 8) != 0)
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
    if ((r_u32(collision_offset + 12) & 1) == 0 && (r_s8(other_offset + 125) != 20 || (r_u32(other_offset + 28) & 0x40) != 0))
        goto fallback;
    flags = r_u32(object_offset + 28);
    if ((flags & 0x8000000) != 0 && (r_u16(state + 706) & 0xF) == 4)
        goto fallback;
    if (status > 0)
        goto slide_contact;
    if (r_u32(collision_offset + 8) < 0x800 && status == 0)
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
        w_u32(state, (r_u32(state) | 0x800) & 0xFCC5FC59);
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
    w_u32(collision_offset + 12, r_u32(collision_offset + 12) & 0xFFFFFFFD);
fallback:
    if (status > 0)
        goto done;
attach_contact:
    if ((r_u32(collision_offset + 12) & 2) == 0)
        goto ordinary_attachment;
    flags = r_u32(object_offset + 28);
    if ((flags & 0x2000000) != 0 || ((flags & 0x8000000) != 0 && (r_u16(state + 706) & 0xF) == 4))
        goto ordinary_attachment;
    w_u32(state, (r_u32(state) | 0x400) & 0xFCC5F459);
    w_u32(state + 180, other_offset);
    kind = r_s8(other_offset + 126);
    if (kind == 2)
        w_u8(state + 496, p2_func_80034eac(other_offset));
    else if (kind == 3)
        w_u8(state + 496, p2_func_8002d750(r_u32(other_offset + 116)));
    goto copy_attachment;
ordinary_attachment:
    flags = r_u32(state);
    if ((flags & 0x40000) != 0 || (r_u32(object_offset + 28) & 0x2000000) != 0)
        goto done;
    w_u32(state, ((flags | 0x20000) & 0xFCC7F459) & 0xFFFBFFFF);
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
    w_u32(state, (r_u32(state) | 0x200800) & 0xFCE5FC59);
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

uint32 p2_func_80047524(uint32 object_offset, uint32 other_offset)
{
    uint32 state = 0x800ABD68;
    uint32 category, event, mode, flags, value, parent, matrix, x, y, z;
    sint32 kind;
    if ((r_u32(state) & 0x4000000) != 0)
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
                return (p2_func_80025b30(0) & 0xFF) != 0;
            case 5:
                flags = r_u32(object_offset + 28);
                if ((flags & 0x22000000) != 0)
                    return 0;
                if ((flags & 0x8000000) != 0 && (r_u16(state + 706) & 0xF) == 4)
                    return 0;
                value = r_u32(0x8008A518);
                return value >= r_u32(other_offset + 68);
            case 6:
                return (r_u32(object_offset + 28) & 0x400) != 0 && r_s32(state + 28) < 0;
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
                return r_s16(state + 676) == 0;
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
                    value = (r_u32(state + 704) >> 20) & 0xF;
                value = p2_func_800473f0((sint32)value, state);
                return value != (r_u16(state + 706) & 0xF);
            default:
                mode = (r_u32(other_offset + 72) >> 12) & 0x1F;
                if ((sint32)mode - 1 < 3 || (uint32)r_u8(other_offset + 15) - 40 >= 16)
                    return 0;
                if ((r_u8(other_offset + 13) & 0x80) != 0 || (r_u8(other_offset + 71) & 0xFE) != 0)
                    return 0;
                switch (mode)
                {
                    case 4:
                        return r_u32(state + 4) & 0x40;
                    case 5:
                    case 6:
                    case 15:
                    case 16:
                        return 1;
                    case 7:
                        return r_u32(state + 24) != 0;
                    case 8:
                        return (r_u32(0x1F800384) & 0x8000) != 0 && (r_u32(object_offset + 28) & 0x8000000) == 0;
                    case 9:
                        return (r_u32(0x1F800384) & 0x2000) != 0 && (r_u32(object_offset + 28) & 0x8000000) == 0;
                    case 10:
                        if (r_s32(state) >= 0)
                        {
                            value = r_u8(r_u32(0x8008A2B4) + 1);
                            return (r_u32(0x1F800384) & value) != 0;
                        }
                        return (r_u32(0x1F800384) & 0x40) != 0 && (r_u32(0x1F800388) & 0x40) == 0;
                    case 11:
                        if (r_s32(state) < 0)
                            return 0;
                        return (r_u32(0x1F800384) >> 14) & 1;
                    case 12:
                        return (r_u32(0x1F800384) >> 12) & 1;
                    case 13:
                        return (r_u32(0x1F800384) >> 2) & 1;
                    case 14:
                        return (r_u32(0x1F800384) >> 3) & 1;
                    case 17:
                        value = r_u8(r_u32(0x8008A2B4));
                        return (r_u32(0x1F800384) & value) != 0;
                    case 18:
                        if ((r_u16(state + 706) & 0xF) != 1)
                            return 0;
                        return r_s8(state + 712) == 9;
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
    if ((r_u32(state + 4) & 0x240) != 0)
    {
        if (r_s8(0x800AC011) >= 12)
            return 0;
        value = r_u16(other_offset + 18) & 0x7F;
        return value < 6 || value == 8;
    }
    if ((r_u32(object_offset + 28) & 0x8000000) != 0 && (r_u16(0x800AC02A) & 0xF) == 4)
        return 0;
    return r_s8(0x800AC011) < 12;
}

uint32 p2_func_8001c69c(uint32 object_offset, uint32 sphere, uint32 first_offset, uint32 last_offset)
{
    uint32 result, last = 0;
    if (r_s8(object_offset + 126) != 1 || r_u32(0x800AC084) != 0)
    {
        w_u8(last_offset, 1);
        w_u8(first_offset, 1);
        return 1;
    }
    w_u8(first_offset, sphere == 1);
    result = 4;
    if (sphere == 4)
        last = 1;
    else
    {
        last = r_u32(0x8008A544 + sphere * 16);
        result = 0xFFFFFFFF;
        last = last == result;
    }
    w_u8(last_offset, last);
    return result;
}

uint32 p2_func_80038990(uint32 bounds_offset, uint32 object_offset, uint32 delta_offset, uint32 extent_offset, uint32 geometry_offset)
{
    uint32 saved_stack = p2_guest_stack_pointer;
    uint32 start, end, relative, category, hx, hy, hz, radius, value, old, reciprocal;
    uint32 sx, sy, sz, ex, ey, ez, dx, dy, dz, lower_x, upper_x, lower_y, upper_y, lower_z, upper_z;
    uint32 enter_x, exit_x, enter_y, exit_y, enter_z, exit_z, time, leave, side, best, normal, nx, ny, nz, dot, i, result = 0;
    p2_guest_stack_pointer -= 0x50;
    start = p2_guest_stack_pointer + 0x10;
    end = p2_guest_stack_pointer + 0x20;
    relative = p2_guest_stack_pointer + 0x40;
    category = r_u8(object_offset + 12);
    if (category == 2)
    {
        hx = r_u16(object_offset + 60) << 5;
        hy = r_u16(object_offset + 62) << 5;
        hz = r_u16(object_offset + 64) << 5;
        w_u32(relative, r_u32(bounds_offset) - r_u32(object_offset + 36));
        w_u32(relative + 4, r_u32(bounds_offset + 4) - r_u32(object_offset + 40));
        w_u32(relative + 8, r_u32(bounds_offset + 8) - r_u32(object_offset + 44));
        PushMatrix();
        for (i = 0; i < 5; ++i)
            xport_gte_write_control(i, r_u32(object_offset + 16 + i * 4));
        p2_sdk_8007ec64(relative, start);
        w_u32(relative, r_u32(relative) + r_u32(delta_offset));
        w_u32(relative + 4, r_u32(relative + 4) + r_u32(delta_offset + 4));
        w_u32(relative + 8, r_u32(relative + 8) + r_u32(delta_offset + 8));
        p2_sdk_8007ec64(relative, end);
        PopMatrix();
    }
    else
    {
        if (category == 8)
        {
            sx = r_u32(bounds_offset) - r_u32(object_offset + 40);
            w_u32(start, sx);
            sy = r_u32(bounds_offset + 4) - r_u32(object_offset + 44);
            w_u32(start + 4, sy);
            sz = r_u32(bounds_offset + 8) - r_u32(object_offset + 48);
            w_u32(start + 8, sz);
            hz = r_u32(object_offset + 24);
        }
        else if (category == 0)
        {
            sx = r_u32(bounds_offset) - r_u32(geometry_offset);
            w_u32(start, sx);
            sy = r_u32(bounds_offset + 4) - r_u32(geometry_offset + 4);
            w_u32(start + 4, sy);
            sz = r_u32(bounds_offset + 8) - r_u32(geometry_offset + 8);
            w_u32(start + 8, sz);
            hz = r_u32(geometry_offset + 12);
        }
        else
            goto done;
        hy = hz;
        hx = hz;
        w_u32(end, sx + r_u32(delta_offset));
        w_u32(end + 4, sy + r_u32(delta_offset + 4));
        w_u32(end + 8, sz + r_u32(delta_offset + 8));
    }
    radius = r_u32(bounds_offset + 12);
    sx = r_u32(start);
    ex = r_u32(end);
    value = (0 - hx) - (sx - radius);
    lower_x = value - (radius << 1);
    upper_x = value + (hx << 1);
    dx = ex - sx;
    if (!((sint32)lower_x > 0 ? (sint32)dx >= (sint32)lower_x : (sint32)upper_x >= 0))
    {
        if ((sint32)upper_x >= 0 || (sint32)upper_x < (sint32)dx)
            goto done;
    }
    sy = r_u32(start + 4);
    ey = r_u32(end + 4);
    value = (0 - hy) - (sy - radius);
    lower_y = value - (radius << 1);
    upper_y = value + (hy << 1);
    dy = ey - sy;
    if (!((sint32)lower_y > 0 ? (sint32)dy >= (sint32)lower_y : (sint32)upper_y >= 0))
    {
        if ((sint32)upper_y >= 0 || (sint32)upper_y < (sint32)dy)
            goto done;
    }
    sz = r_u32(start + 8);
    ez = r_u32(end + 8);
    value = (0 - hz) - (sz - radius);
    lower_z = value - (radius << 1);
    upper_z = value + (hz << 1);
    dz = ez - sz;
    if (!((sint32)lower_z > 0 ? (sint32)dz >= (sint32)lower_z : (sint32)upper_z >= 0))
    {
        if ((sint32)upper_z >= 0 || (sint32)upper_z < (sint32)dz)
            goto done;
    }
    if (dx + 256 < 0x201)
    {
        enter_x = 0;
        exit_x = 4096;
    }
    else
    {
        old = lower_x;
        if ((sint32)upper_x < 0)
        {
            lower_x = 0 - upper_x;
            upper_x = 0 - old;
        }
        else if ((sint32)lower_x <= 0)
        {
            if ((sint32)dx < 0)
                upper_x = 0 - lower_x;
            lower_x = 0;
        }
        if ((sint32)dx < 0)
            dx = 0 - dx;
        if ((sint32)dx < (sint32)upper_x)
            upper_x = dx;
        if (dx == 0)
            xport_mips_break(7);
        reciprocal = dx == 0 ? 0xFFFFFFFF : (uint32)(0x1000000 / (sint32)dx);
        enter_x = (uint32)(((sint64)(sint32)lower_x * (sint32)reciprocal) >> 12);
        exit_x = (uint32)(((sint64)(sint32)upper_x * (sint32)reciprocal) >> 12);
    }
    time = enter_x;
    leave = exit_x;
    if (dy + 256 < 0x201)
    {
        enter_y = 0;
    }
    else
    {
        old = lower_y;
        if ((sint32)upper_y < 0)
        {
            lower_y = 0 - upper_y;
            upper_y = 0 - old;
        }
        else if ((sint32)lower_y <= 0)
        {
            if ((sint32)dy < 0)
                upper_y = 0 - lower_y;
            lower_y = 0;
        }
        if ((sint32)dy < 0)
            dy = 0 - dy;
        if ((sint32)dy < (sint32)upper_y)
            upper_y = dy;
        if (dy == 0)
            xport_mips_break(7);
        reciprocal = dy == 0 ? 0xFFFFFFFF : (uint32)(0x1000000 / (sint32)dy);
        enter_y = (uint32)(((sint64)(sint32)lower_y * (sint32)reciprocal) >> 12);
        exit_y = (uint32)(((sint64)(sint32)upper_y * (sint32)reciprocal) >> 12);
        if ((sint32)time < (sint32)enter_y)
            time = enter_y;
        if ((sint32)exit_y < (sint32)leave)
            leave = exit_y;
    }
    if (dz + 256 < 0x201)
    {
        enter_z = 0;
    }
    else
    {
        old = lower_z;
        if ((sint32)upper_z < 0)
        {
            lower_z = 0 - upper_z;
            upper_z = 0 - old;
        }
        else if ((sint32)lower_z <= 0)
        {
            if ((sint32)dz < 0)
                upper_z = 0 - lower_z;
            lower_z = 0;
        }
        if ((sint32)dz < 0)
            dz = 0 - dz;
        if ((sint32)dz < (sint32)upper_z)
            upper_z = dz;
        if (dz == 0)
            xport_mips_break(7);
        reciprocal = dz == 0 ? 0xFFFFFFFF : (uint32)(0x1000000 / (sint32)dz);
        enter_z = (uint32)(((sint64)(sint32)lower_z * (sint32)reciprocal) >> 12);
        exit_z = (uint32)(((sint64)(sint32)upper_z * (sint32)reciprocal) >> 12);
        if ((sint32)time < (sint32)enter_z)
            time = enter_z;
        if ((sint32)exit_z < (sint32)leave)
            leave = exit_z;
    }
    if ((sint32)leave < (sint32)time)
        goto done;
    result = 1;
    if (r_u8(object_offset + 12) != 2)
        goto hit;
    value = (uint32)r_s8(object_offset + 15);
    if (value != 9 && value != 6)
        goto hit;
    if (time != 0)
    {
        if (time == enter_y)
            side = (sint32)sy < 0 ? 2 : 3;
        else if (time == enter_z)
            side = (sint32)sz < 0 ? 4 : 5;
        else
            side = (sint32)sx >= 0;
        goto side_hit;
    }
    value = (sint32)sx < 0 ? sx + hx : sx - hx;
    w_u32(end, value);
    if ((sint32)value < 0)
        w_u32(end, 0 - value);
    value = (sint32)sy < 0 ? sy + hy : sy - hy;
    w_u32(end + 4, value);
    if ((sint32)value < 0)
        w_u32(end + 4, 0 - value);
    value = (sint32)sz < 0 ? sz + hz : sz - hz;
    w_u32(end + 8, value);
    if ((sint32)value < 0)
        w_u32(end + 8, 0 - value);
    value = r_u32(end + 4);
    if (r_s32(end) >= (sint32)value && r_s32(end + 8) >= (sint32)value)
        side = (sint32)sy < 0 ? 2 : 3;
    else
    {
        value = r_u32(end + 8);
        if (r_s32(end) >= (sint32)value && r_s32(end + 4) >= (sint32)value)
            side = (sint32)sz < 0 ? 4 : 5;
        else
            side = (sint32)sx >= 0;
    }
    if (r_u32(delta_offset) == 0 && r_u32(delta_offset + 4) == 0 && r_u32(delta_offset + 8) == 0)
        goto side_hit;
    best = enter_z;
    for (i = 0; i < 6; ++i)
    {
        normal = object_offset + 16 + (i >> 1) * 6;
        nx = (uint32)(sint32)r_s16(normal);
        ny = (uint32)(sint32)r_s16(normal + 2);
        nz = (uint32)(sint32)r_s16(normal + 4);
        if ((i & 1) == 0)
        {
            nx = 0 - nx;
            ny = 0 - ny;
            nz = 0 - nz;
        }
        dot = (uint32)(((sint64)r_s32(delta_offset) * (sint32)nx) >> 12);
        dot += (uint32)(((sint64)r_s32(delta_offset + 4) * (sint32)ny) >> 12);
        dot += (uint32)(((sint64)r_s32(delta_offset + 8) * (sint32)nz) >> 12);
        if ((sint32)dot < (sint32)time)
        {
            time = dot;
            best = i;
        }
    }
    if (best != side)
    {
        normal = object_offset + 16 + (uint32)((sint32)best >> 1) * 6;
        nx = (uint32)(sint32)r_s16(normal);
        ny = (uint32)(sint32)r_s16(normal + 2);
        nz = (uint32)(sint32)r_s16(normal + 4);
        if ((best & 1) == 0)
        {
            nx = 0 - nx;
            ny = 0 - ny;
            nz = 0 - nz;
        }
        if (ny + 128 >= 0x101)
        {
            normal = object_offset + 16 + (side >> 1) * 6;
            nx = (uint32)(sint32)r_s16(normal);
            ny = (uint32)(sint32)r_s16(normal + 2);
            nz = (uint32)(sint32)r_s16(normal + 4);
            if ((side & 1) == 0)
            {
                nx = 0 - nx;
                ny = 0 - ny;
                nz = 0 - nz;
            }
            if (ny + 128 < 0x101)
                best = side;
        }
    }
    side = best;
    time = 0;
side_hit:
    result = side + 1;
hit:
    w_u32(extent_offset, time);
done:
    p2_guest_stack_pointer = saved_stack;
    return result;
}

uint32 p2_func_8005badc(uint32 head_offset, uint32 object_offset, uint32 empty_pool_node)
{
    uint32 free_head = r_u32(0x8008A34C);
    uint32 node = empty_pool_node;
    uint32 result;
    if (free_head != 0)
    {
        node = free_head;
        w_u32(0x8008A34C, r_u32(node + 4));
    }
    w_u32(node, object_offset);
    result = r_u32(head_offset);
    w_u32(node + 4, result);
    w_u32(head_offset, node);
    return result;
}

uint32 p2_func_80047058(uint32 object_offset, uint32 other_offset, uint32 context_offset)
{
    uint32 flags, mode, node, target, other_target;
    if (r_s8(object_offset + 124) == 2)
    {
        mode = r_u32(object_offset + 116);
        if ((mode == 19 || (mode == 0 && r_u32(0x8008A498) != 0)) && other_offset != 0)
        {
            if ((r_u32(object_offset + 28) & 0x10) != 0)
                node = r_u32(other_offset + 68);
            else
                node = r_u32(other_offset + 72);
            if ((r_u32(r_u32(node + 36) + 16) & 0x90000) != 0)
                return 0;
        }
    }
    flags = r_u32(object_offset + 28);
    if ((flags & 0xC00) == 0 && (r_u32(object_offset + 228) & 0xA0) == 0)
        return 0;
    if (context_offset == 0)
        return 0;
    if (other_offset == 0)
        return 1;
    if ((flags & 0x10) != 0)
        node = r_u32(other_offset + 68);
    else
        node = r_u32(other_offset + 72);
    mode = r_u32(object_offset + 228);
    if ((mode & 0x20) != 0)
    {
        other_target = r_u32(0x800ABE60);
        target = r_u32(node + 36);
        if (target != r_u32(other_target + 36) || (mode & 0x8000) != 0)
            return 1;
        return (r_u32(object_offset + 28) & 0x2000) != 0;
    }
    target = r_u32(node + 36);
    other_target = r_u32(context_offset + 36);
    if (target != other_target)
    {
        if (r_s8(object_offset + 126) != 1)
            return 1;
        if (target != r_u32(r_u32(0x800AC018) + 36))
            return 1;
    }
    return (r_u32(object_offset + 28) & 0x2000) != 0;
}

uint32 p2_func_8005bff4(void)
{
    uint32 saved_stack = p2_guest_stack_pointer;
    uint32 record, limit, scene, source, count, value, node, object, score, head = 0, cursor, previous;
    uint32 x, y, z, flags, result, slots, budget, cost, rank;
    p2_guest_stack_pointer -= 0x280;
    record = p2_guest_stack_pointer + 0x10;
    limit = p2_guest_stack_pointer + 0x268;
    scene = r_u32(0x8008A114);
    if (r_s8(scene + 75) != 0)
    {
        source = scene + 76;
        do
        {
            scene = r_u32(0x8008A114);
            count = (uint32)(sint32)r_s8(scene + 1);
            value = r_u32(source);
            source += 4;
            if ((sint32)count < 40)
            {
                w_u8(scene + 1, count + 1);
                w_u32(scene + count * 4 + 140, value | 0x20000);
            }
            scene = r_u32(0x8008A114);
            count = r_u8(scene + 75) - 1;
            w_u8(scene + 75, count);
        } while ((count & 0xFF) != 0);
    }
    result = r_u32(0x8008A4E8);
    for (node = r_u32(result + 68); node != 0; node = r_u32(node))
    {
        object = r_u32(node + 60);
        if ((r_u8(object + 13) & 0x80) != 0)
        {
            result = r_u16(node + 48);
            if (result == 0)
                continue;
            value = r_u16(node + 40) - 1;
            w_u16(node + 40, value);
            result = value & 0xFFFF;
            if (result != 0)
                continue;
            value = r_u16(node + 26);
            object = r_u32(node + 60);
            w_u16(node + 24, value);
            w_u32(object + 16, r_u32(object + 16) & 0xFFFFFFFB);
            object = r_u32(node + 60);
            w_u8(object + 13, r_u8(object + 13) & 0x7F);
        }
        object = r_u32(node + 60);
        if ((r_u8(object + 13) & 1) != 0)
        {
            result = p2_func_8005bcfc(object);
            continue;
        }
        value = r_u32(node + 12);
        if (value == 0x7FFFF000)
            value = p2_func_800343f0(node);
        w_u32(record + 8, value);
        if ((r_u32(0x8008A1A4) & 0x200) != 0)
        {
            object = r_u32(node + 4);
            if (object != 0 && (r_s8(node + 37) != 12 || r_s8(node + 124) != 0) && (r_u32(0x8008A510) == 0 || r_s32(record + 8) < r_s32(0x8008A41C)))
            {
                x = r_u32(0x800B1510) - r_u32(object + 16);
                w_u32(limit, x);
                object = r_u32(node + 4);
                y = r_u32(0x800B1514) - r_u32(object + 20);
                w_u32(limit + 4, y);
                object = r_u32(node + 4);
                z = r_u32(0x800B1518) - r_u32(object + 24);
                w_u32(limit + 8, z);
                value = p2_func_8005dc00((sint32)z, (sint32)x);
                value = (uint32)(sint32)(sint16)value - (uint32)(sint32)r_s16(r_u32(0x8008A494) + 148);
                if ((sint32)value < 0)
                    value = 0 - value;
                if ((sint16)value < 512)
                {
                    object = r_u32(node + 4);
                    value = r_u32(record + 8);
                    w_u32(0x8008A510, object);
                    w_u32(0x8008A41C, value);
                }
            }
        }
        cursor = head;
        w_u32(record + 4, node);
        if (cursor == 0)
        {
            head = record;
            w_u32(record, 0);
        }
        else
        {
            score = r_u32(record + 8);
            if (r_s32(cursor + 8) < (sint32)score)
            {
                do
                {
                    previous = cursor;
                    cursor = r_u32(cursor);
                } while (cursor != 0 && r_s32(cursor + 8) < (sint32)score);
            }
            if (cursor == head)
                head = record;
            else
                w_u32(previous, record);
            w_u32(record, cursor);
        }
        record += 12;
        result = limit;
        if (record == limit)
            break;
    }
    rank = 0xFFFFFFFF;
    slots = 12;
    budget = 102400;
    for (cursor = head; cursor != 0; cursor = r_u32(cursor))
    {
        if (slots != 0)
        {
            node = r_u32(cursor + 4);
            object = r_u32(node + 72);
            if (object != 0)
            {
                cost = r_u16(object + 70);
                if ((sint32)budget >= (sint32)cost)
                {
                    budget -= cost;
                    --rank;
                    w_u32(node + 12, rank);
                    --slots;
                }
            }
        }
        node = r_u32(cursor + 4);
        p2_func_8005bcfc(r_u32(node + 60));
    }
    if (head != 0)
    {
        flags = r_u32(0x8008A1A4);
        result = flags ^ 0x200;
        if ((flags & 0x200) != 0)
        {
            result |= 0x400;
            w_u32(0x8008A1A4, result);
        }
    }
    p2_guest_stack_pointer = saved_stack;
    return result;
}

uint32 p2_func_80016d0c(void)
{
    uint32 object = p2_func_800536f0(0);
    uint32 target, value, scene, matrix, x, y, z, i;
    sint32 kind;
    if (object != 0)
    {
        kind = r_s8(object + 127);
        if (kind != 0)
            target = r_u32(0x800ABD50 + (uint32)kind * 4);
        else
            target = r_u32(0x800BA380 + (uint32)(sint32)r_s8(object + 125) * 4);
        p2_dispatch_object_callback(target, object);
        value = r_u32(0x1F800320);
        if ((value & 0x48) != 0)
        {
            if ((value & 8) != 0)
                return 0;
            scene = r_u32(0x8008A4E8);
            if ((r_u16(scene + 18) & 8) == 0)
                return 0;
            if (r_u16(0x8008A532) < 0x1000)
            {
                w_u16(0x1F8002D8, 0);
                w_u16(0x1F8002D6, 0);
                w_u16(0x1F8002D4, 0);
                w_u32(0x1F8002D0, 0x80001000);
                for (i = 0; i < 5; ++i)
                    xport_gte_write_control(i, r_u32(0x80010180 + i * 4));
                matrix = r_u32(0x8008A0C8);
                x = r_u32(0x80010180);
                y = r_u32(0x80010184);
                z = r_u32(0x80010188);
                w_u32(matrix, x);
                w_u32(matrix + 4, y);
                w_u32(matrix + 8, z);
                value = r_u32(0x8001018C);
                x = (uint32)r_s16(0x80010190);
                w_u32(matrix + 12, value);
                w_u16(matrix + 16, x);
                matrix = r_u32(0x8008A0C8);
                w_u32(matrix + 28, 0);
                w_u32(matrix + 24, 0);
                w_u32(matrix + 20, 0);
                p2_func_800140f8(scene, 0, 0);
            }
            return 0;
        }
    }
    if ((r_u32(r_u32(0x8008A114) + 72) & 0xFFFFFF) != 0)
        p2_func_80024890();
    if (r_u32(0x8008A4F4) != 0)
        p2_func_80024c44();
    return 1;
}

uint32 p2_func_800536f0(uint32 index)
{
    if (index == 0)
        return r_u32(0x8008A340);
    return 0;
}

uint32 p2_func_800530a0(uint32 object_offset)
{
    uint32 state = 0x800ABD68;
    uint32 flags, value, source, first, second, third, fourth, animation, kind, result;
    sint32 frame, next = 0;
    if (r_s8(object_offset + 127) == 1)
        return 1;
    if ((r_u32(state) & 0x4000000) != 0)
        return p2_func_80045ed8(object_offset);
    p2_func_8004eb74(object_offset, state);
    if ((r_u32(object_offset + 28) & 0x2000000) != 0)
    {
        value = r_u16(0x800ABD9A) - 1;
        w_u16(0x800ABD9A, value);
        if ((sint16)value <= 0)
        {
            w_u32(object_offset + 28, r_u32(object_offset + 28) & 0xEDF7FFFF);
            w_u16(0x800ABD9A, 0);
        }
        else
            w_u32(object_offset + 28, r_u32(object_offset + 28) ^ 0x80000);
    }
    p2_func_800506d4(object_offset, state);
    p2_func_80050f60(object_offset, state);
    p2_func_800521bc(object_offset, state);
    p2_func_800524ac(object_offset, state);
    if ((p2_func_800526f8(object_offset, state) & 0xFF) == 0)
        goto finish;
    result = p2_func_80050c94(object_offset, state) << 24;
    if (result != 0)
        return result;
    if ((r_u32(state) & 0x20) != 0)
    {
        source = r_u32(state + 116);
        fourth = r_u32(source + 80);
        first = r_u32(source + 68);
        second = r_u32(source + 72);
        third = r_u32(source + 76);
        p2_func_80046628(object_offset, first, second, third, fourth);
        w_u32(state, r_u32(state) & 0xFFFFFFDD);
        goto finish;
    }
    p2_func_800515f0(object_offset, state);
    if (r_s8(state + 712) == 23)
        goto finish;
    source = r_u32(object_offset + 104);
    if (source != 0)
    {
        value = (uint32)(sint32)r_s16(source + 6);
        frame = r_s16(object_offset + 130);
        if (frame < (sint32)value - 1)
            next = frame + 1;
    }
    frame = r_s16(object_offset + 130);
    if (next < frame)
    {
        if ((r_u32(object_offset + 228) & 0x40000) != 0)
            goto reverse;
        animation = (uint32)(sint32)r_s8(state + 712);
        if (animation - 1 >= 34 && (r_u16(state + 706) & 0xF) < 2)
        {
            w_u16(object_offset + 130, (uint32)next);
            value = 0;
            if (r_s8(state + 713) == 0 && r_s16(state + 62) <= 0)
            {
                value = 37;
                if (r_s16(state + 62) < -4499)
                    w_u16(state + 62, (uint32)-4500);
                else
                {
                    flags = p2_func_80068198() & 0x7F;
                    value = flags < 50 ? 0 : (flags < 101 ? 35 : 36);
                }
            }
            goto set_animation;
        }
        animation = r_u8(state + 712);
        if (animation - 4 < 2)
        {
            if ((r_u32(object_offset + 28) & 0x400) != 0)
                value = 6;
            else
                value = r_s8(state + 14) <= 0 ? 1 : 0;
            goto set_animation;
        }
        if ((sint8)animation == 3)
        {
            value = 6;
            goto set_animation;
        }
        kind = r_u16(state + 706) & 0xF;
        if ((kind == 0 || kind == 8) && (sint8)animation == 9)
        {
            value = 6;
            goto set_animation;
        }
        animation = (uint32)(sint32)r_s8(state + 712);
        if (animation == 7)
        {
            if ((r_u16(state + 706) & 0xF) != 5)
                goto last_frame;
            animation = (uint32)(sint32)r_s8(state + 712);
        }
        if (animation == 17 || animation == 20)
        {
        last_frame:
            source = r_u32(object_offset + 104);
            w_u16(object_offset + 130, r_u16(source + 6) - 1);
            goto finish;
        }
    }
    else
    {
        if ((r_u32(object_offset + 228) & 0x40000) != 0)
            goto reverse;
        if (r_s8(state + 712) == 20 && r_s8(state + 252) == 1)
        {
            w_u16(object_offset + 130, 0);
            goto finish;
        }
    }
    w_u16(object_offset + 130, (uint32)next);
    goto finish;
reverse:
    frame = r_s16(object_offset + 130);
    if (frame > 0)
        w_u16(object_offset + 130, (uint32)frame - 1);
    goto finish;
set_animation:
    if ((r_u32(state + 4) & 0x80) == 0)
        w_u8(state + 712, value);
finish:
    p2_func_80052ba4(object_offset, state);
    p2_func_8004fd44(object_offset, state);
    return p2_func_8001a6ac(object_offset);
}

uint32 p2_func_800506d4(uint32 object_offset, uint32 character_offset)
{
    uint32 saved_stack, values, event, kind, marker, result, index, value, flags, total, state, action, previous, form;
    sint32 amount, first, second, third;
    first = r_s8(character_offset + 681);
    value = r_u8(character_offset + 681);
    if (first == 0)
        return 0;
    saved_stack = p2_guest_stack_pointer;
    p2_guest_stack_pointer -= 0x20;
    values = p2_guest_stack_pointer + 0x10;
    result = value - 1;
    do
    {
        w_u8(character_offset + 681, result);
        index = (uint32)(sint32)(sint8)result * 4;
        value = r_u32(character_offset + 604 + index);
        if (r_s32(object_offset + 144) < (sint32)value)
            goto next_event;
        first = r_s8(0x8008A194);
        second = r_s8(0x8008A195);
        third = r_s8(0x8008A196);
        w_u8(values, first);
        w_u8(values + 1, second);
        w_u8(values + 2, third);
        index = (uint32)(sint32)r_s8(character_offset + 681) * 4;
        event = r_u32(character_offset + 504 + index);
        kind = r_u8(event + 18) & 0x7F;
        value = r_u8(event + 13);
        w_u8(event + 13, value | 0x80);
        marker = kind;
        switch (kind)
        {
            case 0:
                value = r_u8(character_offset + 12);
                if (value < 32)
                    w_u8(character_offset + 12, value + 1);
                break;
            case 1:
                first = r_s8(0x800ABD76);
                second = r_s8(0x800ABD75);
                if (first < second)
                    w_u8(0x800ABD76, first + 1);
                break;
            case 2:
                if (r_s8(character_offset + 13) < 21)
                {
                    value = r_u16(r_u32(0x8008A4E4) + 262);
                    if (value == 75)
                        w_u8(0x8008A344, r_u8(0x8008A344) | 1);
                    else if (value == 10)
                        w_u8(0x8008A344, r_u8(0x8008A344) | 2);
                    else if (value == 20)
                        w_u8(0x8008A344, r_u8(0x8008A344) | 4);
                    w_u8(character_offset + 13, r_u8(character_offset + 13) + 4);
                }
                first = r_s8(character_offset + 14);
                second = r_s8(character_offset + 13);
                if (second - 4 >= first)
                    w_u8(character_offset + 14, first + 4);
                break;
            case 3:
            case 4:
            case 5:
                index = r_u16(event + 18) & 0x7F;
                state = r_u32(0x8008A4E8);
                amount = r_s8(values + index - 3);
                w_u16(state + 96, r_u16(state + 96) + amount);
                flags = r_u32(character_offset);
                total = r_u32(character_offset + 16);
                total += (flags & 0x400000) != 0 ? (uint32)amount * 2 : (uint32)amount;
                index = (uint32)(sint32)r_s8(character_offset + 681) * 4;
                w_u32(character_offset + 16, total);
                if (r_u32(character_offset + 604 + index) == 0 && (r_u32(0x8008A178) & 1) != 0)
                {
                    flags = r_u32(character_offset);
                    total += (flags & 0x400000) != 0 ? (uint32)amount * 2 : (uint32)amount;
                    w_u32(character_offset + 16, total);
                }
                break;
            case 6:
                if ((r_u16(character_offset + 706) & 15) == 1)
                {
                    state = r_u32(character_offset + 780);
                    value = r_u32(state + 168);
                    previous = r_u32(state + 88);
                    w_u32(state + 88, value);
                    state = r_u32(character_offset + 780);
                    w_u32(state + 168, previous);
                    p2_func_80027b1c(object_offset, character_offset, 7);
                    w_u32(character_offset + 4, r_u32(character_offset + 4) | 0x40);
                    w_u32(object_offset + 28, r_u32(object_offset + 28) | 0x20000000);
                    w_u16(character_offset + 64, 450);
                    break;
                }
                goto change_form;
            case 7:
                if ((r_u16(character_offset + 706) & 15) == 1)
                {
                    w_u32(character_offset + 4, r_u32(character_offset + 4) | 0x200);
                    w_u32(object_offset + 28, r_u32(object_offset + 28) | 0x20000000);
                    w_u16(character_offset + 68, 450);
                    break;
                }
            change_form:
                kind = r_u16(character_offset + 706) & 15;
                if (kind == 4 || kind == 5 || kind - 10 < 2)
                {
                    marker = 0xFFFFFFFF;
                    break;
                }
                form = marker == 7 ? 11 : 0;
                marker = 6;
                if ((r_u32(character_offset + 4) & 0x100) != 0)
                {
                    p2_func_8005a668(character_offset);
                    p2_func_80027ac0(character_offset, 6);
                }
                else
                {
                    value = r_u32(character_offset + 20);
                    if ((value & 0xFFFF) != 0)
                    {
                        previous = value >> 16;
                        if (previous == form)
                            goto store_form;
                        p2_func_80027ac0(character_offset, previous == 0 ? 3 : previous - 7);
                    }
                }
                p2_func_80027b1c(object_offset, character_offset, form == 0 ? 3 : form - 7);
            store_form:
                w_u32(character_offset + 20, (form << 16) | 1);
                break;
            case 8:
                marker -= 2;
                break;
            case 10:
                flags = r_u32(character_offset + 4);
                if ((flags & 0x120) == 0)
                {
                    if ((r_u16(character_offset + 706) & 15) == 1)
                        w_u32(character_offset + 4, flags | 0x20);
                    else
                    {
                        p2_func_80027b1c(object_offset, character_offset, 6);
                        p2_func_8005a440(object_offset, character_offset);
                        value = r_u32(character_offset + 20);
                        if ((value & 0xFFFF) != 0)
                        {
                            previous = value >> 16;
                            p2_func_80027ac0(character_offset, previous == 0 ? 3 : previous - 7);
                        }
                        w_u32(character_offset + 20, 0);
                    }
                }
                break;
            case 11:
                p2_func_80027b1c(object_offset, character_offset, 3);
                w_u32(character_offset + 20, 1);
                break;
        }
        while (r_u32(character_offset + 16) >= 500)
        {
            value = r_u8(character_offset + 12);
            if (value < 32)
                w_u8(character_offset + 12, value + 1);
            value = r_u32(character_offset + 16);
            w_u32(character_offset + 16, value - 500);
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
                    action = (r_u16(character_offset + 706) & 15) == 1 ? 11 : 15;
                    break;
            }
            p2_func_80044ffc(action, 3);
        }
        if (r_u16(event + 16) != 0)
            w_u16(event + 18, r_u16(event + 18) | 0x8000);
    next_event:
        first = r_s8(character_offset + 681);
        value = r_u8(character_offset + 681);
        result = value - 1;
    } while (first != 0);
    p2_guest_stack_pointer = saved_stack;
    return result;
}

uint32 p2_func_80050f60(uint32 object_offset, uint32 character_offset)
{
    uint32 flags, object_flags, event, state, value, previous, index, count, scene, result;
    sint32 kind, health, active;
    flags = r_u32(0x8008A1A4);
    if ((flags & 0x100) != 0)
    {
        state = r_u32(0x8008A510);
        state = r_u32(state + 132);
        state = r_u32(state + 44);
        value = r_u32(character_offset);
        state = r_u32(state + 72);
        w_u32(character_offset, value | 0x400);
        value = r_u16(state + 650);
        w_u32(0x8008A1A4, flags & 0xFFFFFEFF);
        w_u8(character_offset + 496, value);
        goto take_damage;
    }
    if ((r_u32(character_offset) & 0x80000) != 0)
    {
        object_flags = r_u32(object_offset + 28);
        if ((object_flags & 0x2000000) == 0 && (object_flags & 0x4400) != 0x400)
        {
            w_u8(character_offset + 496, 1);
            goto take_damage;
        }
    }
    flags = r_u32(character_offset);
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
            flags = r_u32(character_offset);
            w_u8(character_offset + 14, 0);
            w_u32(character_offset, flags | 0x80);
        }
        else
        {
            value = r_u32(event + 76);
            event = r_u32(character_offset + 180);
            w_u8(character_offset + 496, value);
            value = r_u16(event + 78);
            value += r_u32(0x8008A518);
            w_u32(event + 68, value);
        }
    }
    else if ((r_u32(object_offset + 28) & 0x2000400) != 0x2000000)
        w_u32(character_offset, flags | 0x20000);
    if (r_s8(character_offset + 14) <= 0 || (r_u32(object_offset + 28) & 0x2000000) != 0)
        goto timed_flags;
take_damage:
    if ((r_u32(0x1F800320) & 0x10000) != 0 && (uint32)r_u16(r_u32(0x8008A4E4) + 262) - 96 >= 2 && (r_u32(object_offset + 28) & 0x22000000) == 0)
    {
        value = r_u8(character_offset + 496);
        health = r_s8(character_offset + 14);
        if ((sint32)value >= health)
        {
            value = (uint32)health;
            if (health >= 2)
                --value;
        }
        w_u8(character_offset + 14, r_u8(character_offset + 14) - value);
    }
    p2_func_80044ffc(2, 3);
    value = r_u32(character_offset + 8);
    flags = r_u32(0x8008A52C);
    w_u32(character_offset + 8, value | 0x4000000);
    if ((flags & 0x10) == 0)
    {
        if ((r_u32(character_offset + 4) & 0x20) != 0)
        {
            value = r_u32(0x8008A178);
            flags = r_u32(character_offset + 4);
            w_u32(0x8008A178, value & 0xFFFFFFDB);
            w_u32(character_offset + 4, flags & 0xFFFFFFDF);
        }
        if (((r_u32(0x8008A52C) & 0x200) == 0 || (r_u16(character_offset + 706) & 15) != 0) && (r_u32(character_offset + 4) & 0x100) != 0)
        {
            p2_func_80027ac0(character_offset, 6);
            p2_func_8005a668(character_offset);
        }
        value = r_u32(character_offset + 20);
        if ((value & 0xFFFF) != 0)
        {
            previous = value >> 16;
            p2_func_80027ac0(character_offset, previous == 0 ? 3 : previous - 7);
        }
        w_u32(character_offset + 20, 0);
    }
    if ((r_u32(object_offset + 28) & 0x2000000) == 0)
    {
        w_u16(character_offset + 50, 60);
        w_u32(object_offset + 28, r_u32(object_offset + 28) | 0x2080000);
    }
    if ((r_u32(character_offset) & 0x80000) == 0)
    {
        p2_func_80027b1c(object_offset, character_offset, 0);
        if ((r_u32(object_offset + 228) & 0x101) == 0 && (r_u32(character_offset + 4) & 0x80) == 0)
            w_u8(character_offset + 712, 4);
    }
    w_u32(character_offset, r_u32(character_offset) & 0xEFFFFFFF);
timed_flags:
    if ((r_u32(character_offset) & 0x800000) != 0)
    {
        value = r_u32(character_offset + 656);
        if (r_s32(object_offset + 144) < (sint32)value)
        {
            value = r_u32(character_offset + 4);
            flags = r_u32(character_offset);
            w_u32(character_offset + 4, value & 0xFFFFFF7F);
            w_u32(character_offset, flags & 0xFF7FFFFF);
        }
    }
    if ((r_u32(character_offset) & 8) != 0)
    {
        value = r_u32(character_offset + 552);
        if (r_s32(object_offset + 144) < (sint32)value)
        {
            flags = r_u32(character_offset);
            w_u32(character_offset + 552, 0);
            w_u32(character_offset, flags & 0xFFFFFFF7);
        }
    }
    flags = r_u32(character_offset);
    if ((flags & 0x3000000) == 0x3000000)
    {
        value = r_u32(character_offset + 660);
        previous = r_u32(character_offset + 664);
        if ((sint32)previous >= (sint32)value && r_s32(object_offset + 144) >= (sint32)value)
            w_u32(character_offset, flags & 0xFDFFFFFF);
        else
        {
            previous = r_u32(character_offset + 664);
            value = r_u32(character_offset + 660);
            if ((sint32)value >= (sint32)previous && r_s32(object_offset + 144) >= (sint32)previous)
                w_u32(character_offset, r_u32(character_offset) & 0xFEFFFFFF);
        }
    }
    if ((r_u32(character_offset) & 0x1000000) != 0)
    {
        value = r_u32(character_offset + 660);
        if (r_s32(object_offset + 144) >= (sint32)value)
        {
            w_u8(object_offset + 13, r_u8(object_offset + 13) | 1);
            goto timed_scene;
        }
    }
    if ((r_u32(character_offset) & 0x2000000) != 0)
    {
        value = r_u32(character_offset + 664);
        if (r_s32(object_offset + 144) >= (sint32)value)
            w_u8(object_offset + 13, r_u8(object_offset + 13) & 0xFE);
    }
timed_scene:
    if ((r_u32(character_offset) & 0x4000) != 0)
    {
        value = r_u32(character_offset + 652);
        if (r_s32(object_offset + 144) >= (sint32)value && (p2_func_80025d3c(r_u32(r_u32(character_offset + 176) + 68), 0) << 24) != 0)
        {
            scene = r_u32(0x8008A4E8);
            w_u16(scene + 18, r_u16(scene + 18) & 0xFFEF);
        }
    }
    active = r_s8(character_offset + 680);
    count = r_u8(character_offset + 680);
    while (active != 0)
    {
        value = count - 1;
        w_u8(character_offset + 680, value);
        index = character_offset + (uint32)(sint32)(sint8)value * 4;
        value = r_u32(index + 556);
        if (r_s32(object_offset + 144) >= (sint32)value)
        {
            event = r_u32(index + 128);
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
                                value = (r_u32(character_offset + 704) >> 20) & 15;
                            result = p2_func_800473f0((sint32)value, character_offset);
                            value = r_u32(character_offset + 704);
                            w_u32(character_offset + 704, (value & 0xF0FFFFFF) | (result << 24));
                            break;
                    }
                }
            }
        }
        active = r_s8(character_offset + 680);
        count = r_u8(character_offset + 680);
    }
    return p2_func_80026c78(object_offset);
}

uint32 p2_func_80026c78(uint32 object_offset)
{
    uint32 result, event, scene, value;
    sint32 threshold;
    if (object_offset != 0)
    {
        threshold = r_s32(0x8008B214);
        if (threshold >= r_s32(object_offset + 144))
            return 0;
    }
    result = r_u32(0x8008B1FC);
    if (result == 0)
        return result;
    result = r_u32(0x8008B200);
    if (result != 0)
        return result;
    event = r_u32(0x8008B218);
    scene = r_u32(0x8008A4E8);
    value = r_u32(event + 76);
    w_u32(0x8008B208, value);
    w_u32(0x8008B200, value);
    w_u32(0x8008B21C, r_u32(event + 72));
    value = r_u16(event + 80);
    w_u32(0x8008B210, 0);
    w_u32(0x8008B204, 0);
    w_u32(0x8008B1F8, value);
    result = p2_func_800229fc(scene, r_u16(event + 68));
    event = r_u32(0x8008B218);
    w_u32(0x8008B20C, result);
    w_u8(event + 13, r_u8(event + 13) | 0x80);
    result = r_u32(0x8008B1F8) & 2;
    if (result != 0)
    {
        result = p2_func_800536f0(0);
        w_u8(result + 13, r_u8(result + 13) | 1);
    }
    return result;
}

uint32 p2_func_8004eb74(uint32 object_offset, uint32 character_offset)
{
    uint32 saved_stack = p2_guest_stack_pointer;
    uint32 velocity, parent_velocity, input, transpose, temporary;
    uint32 x, y, z, flags, fraction, parent, matrix, scene, current, support, definition, direction, kind, moved, was_grounded, was_detached, special;
    uint32 first, second, third;
    sint32 frame;
    p2_guest_stack_pointer -= 0x70;
    velocity = p2_guest_stack_pointer + 0x10;
    parent_velocity = p2_guest_stack_pointer + 0x20;
    input = p2_guest_stack_pointer + 0x30;
    transpose = p2_guest_stack_pointer + 0x40;
    temporary = p2_guest_stack_pointer + 0x58;
    if ((r_u32(object_offset + 228) & 0x100) != 0)
        p2_func_80057650(object_offset);
    x = r_u32(object_offset + 92);
    y = r_u32(object_offset + 96);
    z = r_u32(object_offset + 100);
    w_u32(velocity, x);
    w_u32(velocity + 4, y);
    w_u32(velocity + 8, z);
    if (r_s32(object_offset + 144) < 4096)
    {
        w_u32(character_offset, r_u32(character_offset) & 0xFFFFEFFF);
        w_u32(parent_velocity + 8, 0);
        w_u32(parent_velocity + 4, 0);
        w_u32(parent_velocity, 0);
        flags = r_u32(object_offset + 28);
        w_u32(object_offset + 28, flags & 0xFFFFF7FF);
        if ((flags & 0x4000) != 0)
        {
            if ((r_u32(object_offset + 228) & 2) != 0 && (flags & 0x400) != 0)
            {
                scene = r_u32(0x8008A114);
                current = r_u32(scene + 136);
                definition = r_u32(current + 36);
                if ((r_u32(definition + 16) & 0x10000) == 0)
                    p2_func_80058c8c(object_offset);
            }
            scene = r_u32(0x8008A114);
            current = r_u32(scene + 136);
            definition = r_u32(current + 36);
            if ((r_u32(definition + 16) & 0x80000) == 0)
            {
                if ((r_u32(object_offset + 228) & 0x20) != 0)
                    p2_func_800586e8(object_offset);
                if ((r_u32(object_offset + 228) & 0x10) != 0)
                    p2_func_80058618(object_offset);
            }
            if ((r_u32(object_offset + 228) & 0x200) != 0)
                p2_func_80056774(object_offset);
            if ((r_u32(object_offset + 228) & 0x1000) != 0)
                p2_func_80059268(object_offset);
            w_u32(character_offset + 44, 0);
            w_u32(character_offset + 40, 0);
            w_u32(character_offset + 36, 0);
            w_u32(object_offset + 28, r_u32(object_offset + 28) & 0xFFFFFBFF);
        }
        else
        {
            parent = r_u32(object_offset + 132);
            if (parent != 0)
            {
                matrix = r_u32(parent + 40);
                if (matrix != 0 && (flags & 0x2000) == 0)
                {
                    x = r_u32(matrix + 32);
                    y = r_u32(matrix + 36);
                    z = r_u32(matrix + 40);
                    w_u32(parent_velocity, x);
                    w_u32(parent_velocity + 4, y);
                    w_u32(parent_velocity + 8, z);
                }
            }
        }
        x = r_u32(velocity) + r_u32(parent_velocity);
        y = r_u32(velocity + 4) + r_u32(parent_velocity + 4);
        z = r_u32(velocity + 8) + r_u32(parent_velocity + 8);
        w_u32(velocity, x);
        w_u32(velocity + 4, y);
        w_u32(velocity + 8, z);
        fraction = r_u32(object_offset + 144);
        if (fraction != 0 || (r_u32(object_offset + 228) & 6) != 2)
        {
            w_u32(velocity, (uint32)(((sint64)(sint32)x * (sint32)fraction) >> 12));
            w_u32(velocity + 4, (uint32)(((sint64)(sint32)y * (sint32)fraction) >> 12));
            w_u32(velocity + 8, (uint32)(((sint64)(sint32)z * (sint32)fraction) >> 12));
        }
        x = r_u32(velocity) - r_u32(parent_velocity);
        y = r_u32(velocity + 4) - r_u32(parent_velocity + 4);
        z = r_u32(velocity + 8) - r_u32(parent_velocity + 8);
        w_u32(velocity, x);
        w_u32(velocity + 4, y);
        w_u32(velocity + 8, z);
        fraction = 4096 - r_u32(object_offset + 144);
        if ((r_u32(character_offset) & 0x18040) != 0)
        {
            flags = r_u32(character_offset);
            w_u32(character_offset + 44, 0);
            w_u32(character_offset + 36, 0);
            if ((flags & 0x18000) != 0)
                w_u32(character_offset + 40, 0);
        }
        x = (uint32)(((sint64)r_s32(character_offset + 36) * (sint32)fraction) >> 12);
        y = r_u32(character_offset + 40);
        w_u32(character_offset + 36, x);
        y = (uint32)(((sint64)(sint32)y * (sint32)fraction) >> 12);
        z = r_u32(character_offset + 44);
        w_u32(character_offset + 40, y);
        w_u32(character_offset + 44, (uint32)(((sint64)(sint32)z * (sint32)fraction) >> 12));
        w_u32(velocity, r_u32(velocity) + r_u32(character_offset + 36));
        w_u32(velocity + 4, r_u32(velocity + 4) + r_u32(character_offset + 40));
        w_u32(velocity + 8, r_u32(velocity + 8) + r_u32(character_offset + 44));
    }
    moved = r_u32(velocity) != 0 || r_u32(velocity + 8) != 0;
    if (moved != 0)
        w_u32(character_offset + 8, r_u32(character_offset + 8) | 0x2000000);
    else if (r_s32(velocity + 4) <= 0)
        goto rotate_velocity;
    if ((r_u32(0x8008A518) & 1) != 0 && r_s8(character_offset + 714) >= 0 && (r_u32(object_offset + 228) & 0x400011) == 0)
    {
        scene = r_u32(0x8008A114);
        current = r_u32(scene + 136);
        definition = r_u32(current + 36);
        if ((r_u32(definition + 16) & 0x80000) == 0)
        {
            if ((r_u32(object_offset + 28) & 0x2000) != 0)
                goto effect_position;
            parent = r_u32(object_offset + 132);
            if (parent == 0 || r_u32(parent + 40) == 0)
                goto effect_position;
            x = r_u32(object_offset + 16);
            y = r_u32(object_offset + 20);
            z = r_u32(object_offset + 24);
            w_u32(input, x);
            w_u32(input + 4, y);
            w_u32(input + 8, z);
            PushMatrix();
            matrix = r_u32(r_u32(object_offset + 132) + 40);
            x = r_u32(matrix);
            y = r_u32(matrix + 4);
            xport_gte_write_control(0, x);
            xport_gte_write_control(1, y);
            x = r_u32(matrix + 8);
            y = r_u32(matrix + 12);
            z = r_u32(matrix + 16);
            xport_gte_write_control(2, x);
            xport_gte_write_control(3, y);
            xport_gte_write_control(4, z);
            p2_sdk_8007ec64(input, parent_velocity);
            PopMatrix();
            matrix = r_u32(r_u32(object_offset + 132) + 40);
            x = r_u32(parent_velocity);
            w_u32(parent_velocity, x + r_u32(matrix + 20));
            matrix = r_u32(r_u32(object_offset + 132) + 40);
            y = r_u32(parent_velocity + 4);
            w_u32(parent_velocity + 4, y + r_u32(matrix + 24));
            matrix = r_u32(r_u32(object_offset + 132) + 40);
            z = r_u32(parent_velocity + 8);
            w_u32(parent_velocity + 8, z + r_u32(matrix + 28));
            goto emit_effect;
        effect_position:
            x = r_u32(object_offset + 16);
            y = r_u32(object_offset + 20);
            z = r_u32(object_offset + 24);
            w_u32(parent_velocity, x);
            w_u32(parent_velocity + 4, y);
            w_u32(parent_velocity + 8, z);
        emit_effect:
            kind = r_u16(character_offset + 706) & 15;
            if (kind != 5 && kind - 10 >= 2)
                p2_func_8002a80c(parent_velocity, 4096, r_s8(character_offset + 714) + 7);
        }
    }
rotate_velocity:
    if ((r_u32(object_offset + 28) & 0x2000) != 0)
    {
        if (r_u32(object_offset + 144) == 4096)
            w_u32(object_offset + 144, 4095);
    }
    else
    {
        parent = r_u32(object_offset + 132);
        if (parent != 0)
        {
            matrix = r_u32(parent + 40);
            if (matrix != 0)
            {
                w_u16(transpose, r_u16(matrix));
                w_u16(transpose + 2, r_u16(r_u32(r_u32(object_offset + 132) + 40) + 6));
                w_u16(transpose + 4, r_u16(r_u32(r_u32(object_offset + 132) + 40) + 12));
                w_u16(transpose + 6, r_u16(r_u32(r_u32(object_offset + 132) + 40) + 2));
                w_u16(transpose + 8, r_u16(r_u32(r_u32(object_offset + 132) + 40) + 8));
                w_u16(transpose + 10, r_u16(r_u32(r_u32(object_offset + 132) + 40) + 14));
                w_u16(transpose + 12, r_u16(r_u32(r_u32(object_offset + 132) + 40) + 4));
                w_u16(transpose + 14, r_u16(r_u32(r_u32(object_offset + 132) + 40) + 10));
                w_u16(transpose + 16, r_u16(r_u32(r_u32(object_offset + 132) + 40) + 16));
                x = r_u32(velocity);
                y = r_u32(velocity + 4);
                z = r_u32(velocity + 8);
                w_u32(input, x);
                w_u32(input + 4, y);
                w_u32(input + 8, z);
                PushMatrix();
                x = r_u32(transpose);
                y = r_u32(transpose + 4);
                xport_gte_write_control(0, x);
                xport_gte_write_control(1, y);
                x = r_u32(transpose + 8);
                y = r_u32(transpose + 12);
                z = r_u16(transpose + 16);
                xport_gte_write_control(2, x);
                xport_gte_write_control(3, y);
                xport_gte_write_control(4, z);
                p2_sdk_8007ec64(input, velocity);
                PopMatrix();
            }
        }
    }
    w_u32(object_offset + 16, r_u32(object_offset + 16) + r_u32(velocity));
    w_u32(object_offset + 20, r_u32(object_offset + 20) + r_u32(velocity + 4));
    w_u32(object_offset + 24, r_u32(object_offset + 24) + r_u32(velocity + 8));
    if ((r_u32(object_offset + 28) & 0x4000) != 0 || (r_u32(object_offset + 228) & 0x100) != 0)
        goto final_position;
    flags = r_u32(character_offset);
    if ((flags & 0x22) != 0)
        goto final_position;
    fraction = r_u32(object_offset + 144);
    if ((sint32)fraction >= 4096 && ((flags & 1) == 0 || (sint32)fraction < r_s32(character_offset + 672)))
        goto final_position;
    scene = r_u32(0x8008A114);
    w_u16(scene + 2, r_u16(object_offset + 136));
    flags = r_u32(object_offset + 28);
    x = r_u32(character_offset);
    was_grounded = (flags >> 10) & 1;
    was_detached = (flags >> 13) & 1;
    if ((x & 1) == 0 || r_s32(object_offset + 144) < r_s32(character_offset + 672))
        w_u32(character_offset + 188, 0);
    moved = 1;
    support = p2_func_8004e118(object_offset, r_u32(character_offset + 188));
    if (support == 0)
    {
        if ((r_u32(object_offset + 28) & 0x2000) != 0 || r_u32(character_offset + 188) != 0)
            goto grounded_transition;
        parent = r_u32(object_offset + 132);
        if (parent != 0)
        {
            matrix = r_u32(parent + 40);
            if (matrix != 0)
            {
                x = r_u32(object_offset + 92);
                y = r_u32(matrix + 32);
                parent = r_u32(object_offset + 132);
                w_u32(object_offset + 92, x + y);
                matrix = r_u32(parent + 40);
                x = r_u32(object_offset + 96);
                y = r_u32(matrix + 36);
                parent = r_u32(object_offset + 132);
                w_u32(object_offset + 96, x + y);
                matrix = r_u32(parent + 40);
                x = r_u32(object_offset + 100);
                w_u32(object_offset + 100, x + r_u32(matrix + 40));
            }
        }
        p2_func_8004df14(object_offset, character_offset);
        goto grounded_transition;
    }
    flags = r_u32(object_offset + 28);
    if ((flags & 0x4000) != 0)
    {
        if (r_u32(character_offset + 188) != 0)
            w_u32(character_offset, r_u32(character_offset) & 0xFCC5F459);
        else
        {
            w_u32(object_offset + 28, flags & 0xFFFFBBFF);
            w_u32(character_offset, r_u32(character_offset) & 0xF7FFFFFF);
        }
    }
    special = 0;
    if ((r_u32(object_offset + 228) & 0x20) != 0 && support == r_u32(character_offset + 248))
        special = 1;
    scene = r_u32(0x8008A114);
    if (support == r_u32(scene + 136) || special != 0)
        goto grounded_transition;
    parent = r_u32(object_offset + 132);
    if (parent != 0 && r_u32(parent + 40) != 0)
    {
        x = r_u32(object_offset + 16);
        y = r_u32(object_offset + 20);
        z = r_u32(object_offset + 24);
        w_u32(temporary, x);
        w_u32(temporary + 4, y);
        w_u32(temporary + 8, z);
        PushMatrix();
        matrix = r_u32(r_u32(object_offset + 132) + 40);
        x = r_u32(matrix);
        y = r_u32(matrix + 4);
        xport_gte_write_control(0, x);
        xport_gte_write_control(1, y);
        x = r_u32(matrix + 8);
        y = r_u32(matrix + 12);
        z = r_u32(matrix + 16);
        xport_gte_write_control(2, x);
        xport_gte_write_control(3, y);
        xport_gte_write_control(4, z);
        scene = r_u32(0x8008A114);
        p2_sdk_8007ec64(temporary, scene + 16);
        PopMatrix();
        scene = r_u32(0x8008A114);
        matrix = r_u32(r_u32(object_offset + 132) + 40);
        x = r_u32(scene + 16);
        w_u32(scene + 16, x + r_u32(matrix + 20));
        matrix = r_u32(r_u32(object_offset + 132) + 40);
        y = r_u32(scene + 20);
        w_u32(scene + 20, y + r_u32(matrix + 24));
        matrix = r_u32(r_u32(object_offset + 132) + 40);
        z = r_u32(scene + 24);
        w_u32(scene + 24, z + r_u32(matrix + 28));
    }
    else
    {
        scene = r_u32(0x8008A114);
        x = r_u32(object_offset + 16);
        y = r_u32(object_offset + 20);
        z = r_u32(object_offset + 24);
        w_u32(scene + 16, x);
        w_u32(scene + 20, y);
        w_u32(scene + 24, z);
    }
    frame = r_s16(object_offset + 136);
    if (was_detached != 0)
        w_u32(object_offset + 28, r_u32(object_offset + 28) | 0x2000);
    else
    {
        scene = r_u32(0x8008A114);
        current = r_u32(scene + 136);
        flags = r_u32(object_offset + 28);
        definition = r_u32(current + 36);
        w_u16(object_offset + 136, p2_func_800231dc(definition, scene + 16, flags & 0x10, -40960));
    }
    scene = r_u32(0x8008A114);
    if (support != r_u32(scene + 136))
    {
        direction = r_u32(object_offset + 28) & 0x10;
        p2_func_800451cc(support, frame, scene + 28, direction);
        scene = r_u32(0x8008A114);
        current = r_u32(scene + 136);
        definition = r_u32(support + 36);
        if (definition == r_u32(current + 36))
            direction ^= 0x10;
        else
            p2_func_80046228(object_offset, definition);
        scene = r_u32(0x8008A114);
        if ((r_u32(scene + 72) & 0xFFFFFF) == 0)
        {
            current = r_u32(scene + 136);
            if (current == r_u32(0x800AC018) && (r_u32(object_offset + 28) & 0x2000) == 0)
            {
                kind = (uint32)(sint32)r_s16(object_offset + 136);
                if ((sint32)kind >= 0)
                    p2_func_800451cc(current, (sint32)kind, scene + 4, direction);
            }
        }
        p2_func_80024844(support);
    }
    flags = r_u32(object_offset + 28);
    w_u16(object_offset + 136, frame);
    w_u32(object_offset + 28, flags & 0xFFFFDFFF);
    x = r_u32(character_offset + 28);
    if (r_s32(character_offset + 732) >= (sint32)x)
        w_u16(character_offset + 48, 0);
    if (r_u32(character_offset + 188) != 0 && (r_u32(character_offset) & 0x8040) != 0)
        p2_func_8001c684(character_offset);
grounded_transition:
    if ((r_u32(object_offset + 28) & 0x400) != 0 && was_grounded == 0)
    {
        x = r_u32(character_offset + 768);
        flags = r_u32(character_offset);
        kind = r_u16(character_offset + 706);
        w_u32(character_offset + 32, 0xFFFFFFFF);
        w_u32(character_offset + 28, 0);
        w_u16(character_offset + 48, 0);
        w_u16(character_offset + 684, x);
        w_u32(character_offset, flags | 0x8000000);
        if (((kind & 15) != 1 || (r_u32(object_offset + 28) & 0x4000000) == 0) && (r_u32(character_offset + 4) & 0x80) == 0)
            w_u8(character_offset + 712, 6);
    }
final_position:
    scene = r_u32(0x8008A114);
    current = r_u32(character_offset + 688);
    if (r_u32(scene + 136) != current)
    {
        w_u32(temporary, r_u32(scene + 28) - r_u32(scene + 4));
        w_u32(temporary + 4, r_u32(scene + 32) - r_u32(scene + 8));
        w_u32(temporary + 8, r_u32(scene + 36) - r_u32(scene + 12));
        p2_func_80024b3c(temporary);
        scene = r_u32(0x8008A114);
        current = r_u32(scene + 136);
        parent = r_u32(character_offset + 688);
        definition = r_u32(current + 36);
        if (definition != r_u32(parent + 36) && (r_u32(scene + 72) & 0xFFFFFF) != 0)
        {
            first = r_u8(scene + 72);
            second = r_u8(scene + 73);
            third = r_u8(scene + 74);
            x = r_u32(object_offset + 92) * 2;
            y = r_u32(object_offset + 96) * 2;
            z = r_u32(object_offset + 100) * 2;
            w_u32(temporary, r_u32(temporary) - x);
            w_u32(temporary + 4, r_u32(temporary + 4) - y);
            w_u32(temporary + 8, r_u32(temporary + 8) - z);
            p2_func_80024b3c(temporary);
            scene = r_u32(0x8008A114);
            if (first < r_u8(scene + 72))
            {
                w_u8(scene + 72, first);
                scene = r_u32(0x8008A114);
            }
            if (second < r_u8(scene + 73))
                w_u8(scene + 73, second);
            scene = r_u32(0x8008A114);
            if (third < r_u8(scene + 74))
            {
                w_u8(scene + 74, third);
                scene = r_u32(0x8008A114);
            }
        }
        else
            scene = r_u32(0x8008A114);
        moved = 1;
        w_u8(scene, r_u8(scene) & 0xFE);
    }
    scene = r_u32(0x8008A114);
    w_u16(scene + 2, r_u16(object_offset + 136));
    w_u16(character_offset + 682, moved != 0 ? 1 : 0);
    flags = r_u32(character_offset);
    x = r_u32(character_offset + 4);
    w_u32(character_offset, flags & 0xFFFFFFFE);
    w_u32(character_offset + 4, x & 0xFFFFFFF6);
    p2_guest_stack_pointer = saved_stack;
    return 0xFFFFFFF6;
}
