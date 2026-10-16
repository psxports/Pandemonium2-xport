#include "game.h"

uint32 p2_func_8004d5c0(uint32 object_offset, uint32 character_offset, uint32 flags_offset)
{
    uint32 saved_stack = p2_guest_stack_pointer;
    uint32 vector, flags, value, mask, mapping, mode, snapshot, table, i, result;
    sint32 index = 0, strength, applied;
    p2_guest_stack_pointer -= 0x30;
    vector = p2_guest_stack_pointer + 0x20;
    if ((r_u32(character_offset) & 4) != 0)
    {
        p2_func_8001c8cc(object_offset, character_offset, flags_offset);
        w_u32(character_offset, r_u32(character_offset) & 0xFFFFFFFB);
    }
    flags = r_u32(character_offset);
    if ((flags & 0x18040) != 0)
    {
        if ((r_u8(r_u32(character_offset + 112) + 13) & 1) != 0)
            p2_func_8001c684(character_offset);
        else if ((flags & 0x10000) != 0)
        {
            if ((r_u32(character_offset + 4) & 0x80) == 0)
                w_u8(character_offset + 712, 8);
            w_u32(flags_offset, r_u32(flags_offset) & 0xFFFFFBFE);
        }
    }
    mode = r_u32(character_offset) & 0x60000;
    if (mode == 0x40000)
    {
        value = r_u16(character_offset + 676);
        w_u16(character_offset + 676, value - 1);
        if (value == 0)
        {
            flags = r_u32(character_offset);
            w_u16(character_offset + 676, 0);
            w_u32(character_offset, flags & 0xFFFBFFFF);
            w_u32(object_offset + 28, r_u32(object_offset + 28) & 0xEFFFFFFF);
            w_u32(character_offset, r_u32(character_offset) & 0xFFEFFFFF);
        }
        else
            goto suppress;
    }
    else if (mode == 0x60000 && (r_u32(object_offset + 28) & 0x400) != 0)
    {
    suppress:
        w_u32(flags_offset, 0);
        mapping = r_u32(0x8008A2B4);
        mask = r_u8(mapping + 1);
        w_u32(0x1F800384, r_u32(0x1F800384) & ~mask);
        mask = r_u8(mapping + 1);
        w_u32(0x1F800388, r_u32(0x1F800388) & ~mask);
    }
    if ((r_u32(character_offset) & 0x80000) != 0)
        w_u32(flags_offset, 0);
    if ((r_u32(object_offset + 228) & 0x101) != 0 && (r_u32(character_offset) & 0x20000) != 0)
        w_u32(character_offset, r_u32(character_offset) & 0xFFFDFFFF);
    flags = r_u32(character_offset);
    if ((flags & 0x810) == 0 && (flags & 0x60000) != 0x20000)
    {
        result = r_u32(object_offset + 28) & 0x400;
        if (result == 0)
        {
            result = r_u32(object_offset + 228) & 0x1220;
            if (result == 0)
            {
                result = r_u32(r_u32(character_offset + 780) + 12);
                w_u32(character_offset + 28, 0);
                w_u32(character_offset + 728, result);
            }
        }
        goto done;
    }
    if ((r_u32(object_offset + 28) & 0x400) == 0)
        w_u32(flags_offset, r_u32(flags_offset) | 8);
    value = r_u16(character_offset + 706);
    w_u32(character_offset + 32, 1);
    w_u16(character_offset + 48, 0);
    if ((value & 0xF) != 4 || (r_u32(object_offset + 28) & 0x8000000) == 0)
        w_u16(object_offset + 130, 0xFFFF);
    if ((r_u16(character_offset + 706) & 0xF) == 1)
    {
        flags = r_u32(object_offset + 28);
        if ((flags & 0x4000000) != 0)
        {
            w_u32(object_offset + 28, flags & 0xFBFFFFFF);
            if (r_s8(character_offset + 712) == 9 && (r_u32(character_offset + 4) & 0x80) == 0)
                w_u8(character_offset + 712, 2);
            for (i = 0; i < 4; ++i)
                w_u32(character_offset + 736 + i * 4, r_u32(0x8008B4F4 + i * 4));
            w_u32(character_offset + 24, r_u32(0x8008B4F8));
            w_u32(0x8008B4F8, (uint32)-15);
        }
    }
    value = r_u32(character_offset + 768);
    flags = r_u32(character_offset);
    w_u16(character_offset + 684, value);
    if ((flags & 0x60000) == 0x20000)
    {
        w_u32(flags_offset, r_u32(flags_offset) | 8);
        w_u32(character_offset, r_u32(character_offset) | 0x2000);
        result = p2_func_8004a218(object_offset, character_offset, flags_offset);
        goto done;
    }
    value = r_u32(r_u32(character_offset + 780) + 12);
    flags = r_u32(character_offset);
    w_u16(character_offset + 684, 7);
    w_u32(character_offset + 728, value);
    if ((flags & 0x10) != 0)
    {
        index = r_s16(character_offset + 678) - 1;
        for (i = 0; i < 3; ++i)
        {
            table = r_u32(character_offset + 184) + (uint32)(index >> 1) * 6;
            value = (uint32)r_s16(table + 16 + i * 2);
            if (((uint32)index & 1) == 0)
                value = 0 - value;
            w_u32(vector + i * 4, value);
        }
        strength = r_s32(r_u32(character_offset + 184) + 68);
    }
    else
    {
        w_u32(vector + 8, 0);
        w_u32(vector, 0);
        w_u32(vector + 4, 4096);
        w_u32(object_offset + 96, 0);
        if ((r_u32(character_offset) & 0x200000) == 0 && r_u32(0x8008A180) == 0 && r_s8(character_offset + 276) == 4)
        {
            p2_func_80044ffc(7, 3);
            p2_func_80027b1c(object_offset, character_offset, 1);
        }
        strength = r_s32(character_offset + 716);
        value = r_u32(0x8008A180);
        w_u32(character_offset + 44, 0);
        w_u32(character_offset + 40, 0);
        w_u32(character_offset + 36, 0);
        if (value != 0)
            strength = (sint32)(uint32)(((sint64)strength * (sint32)value) >> 12);
    }
    value = r_s16(character_offset + 58) < 3;
    w_u32(character_offset + 104, 0);
    if (value != 0)
    {
        w_u16(character_offset + 58, 6);
        w_u32(flags_offset, r_u32(flags_offset) | 8);
    }
    else if ((r_u32(flags_offset) & 8) == 0)
    {
        w_u16(character_offset + 58, 3);
        w_u32(flags_offset, r_u32(flags_offset) | 8);
        p2_func_80049928(object_offset, character_offset, flags_offset, (uint32)index, vector, strength, (sint32)((uint32)strength << 1));
        if ((r_u32(character_offset) & 0x10) != 0 && r_s32(character_offset + 28) < r_s32(character_offset + 716))
            w_u32(character_offset + 28, r_u32(character_offset + 716));
        flags = r_u32(character_offset + 728);
        value = r_u32(character_offset + 24);
        mode = r_u32(character_offset + 28);
        snapshot = r_u32(character_offset + 28);
        w_u32(character_offset + 104, flags);
        w_u32(character_offset + 96, value);
        w_u32(character_offset + 100, mode);
        if ((sint32)flags < (sint32)snapshot)
            w_u32(character_offset + 104, snapshot);
    }
    snapshot = r_u32(flags_offset);
    applied = strength;
    if ((r_u32(flags_offset) & 8) == 0)
        applied = strength >> 2;
    p2_func_80049928(object_offset, character_offset, flags_offset, snapshot, vector, applied, (sint32)((uint32)strength << 1));
    if ((r_u32(character_offset) & 0x10) != 0 && r_s32(character_offset + 28) < r_s32(character_offset + 716))
        w_u32(character_offset + 28, r_u32(character_offset + 716));
    value = r_u32(character_offset + 28);
    if (r_s32(character_offset + 728) < (sint32)value)
        w_u32(character_offset + 728, value);
    if ((snapshot & 8) != 0)
    {
        if (r_u32(character_offset + 32) == 1)
            w_u32(character_offset + 32, 2);
        w_u32(character_offset, r_u32(character_offset) | 0x10000000);
    }
    w_u32(flags_offset, r_u32(flags_offset) | 8);
    result = (r_u32(character_offset) | 0x2000) & 0xF7FFFFFF;
    w_u32(character_offset, result);
done:
    p2_guest_stack_pointer = saved_stack;
    return result;
}

uint32 p2_func_8004b8f8(uint32 object_offset, uint32 character_offset, uint32 flags_offset)
{
    uint32 flags, value, mask, x, y, z, squared, angle, result;
    sint32 old_frame, speed, length, cosine, duration, timer;
    if (((r_u32(flags_offset) & 8) == 0 && r_s32(character_offset + 32) < 2 && (r_u32(character_offset) & 0x10000000) == 0) || (r_u32(character_offset) & 0x10000) != 0 || (r_u32(object_offset + 228) & 0x200) != 0)
    {
        if ((r_u32(object_offset + 28) & 0x400) != 0)
        {
            if (r_s32(character_offset + 32) > 0)
            {
                if ((r_u32(character_offset) & 0x10000000) == 0)
                {
                    w_u16(character_offset + 48, 0);
                    w_u32(character_offset + 32, 0);
                }
            }
            else
                w_u32(character_offset, r_u32(character_offset) & 0xEFFFFFFF);
        }
        goto done;
    }
    flags = r_u32(character_offset);
    if ((flags & 0x2000) != 0 && (r_u32(object_offset + 28) & 0x400) == 0)
        goto done;
    if ((flags & 0x10000000) == 0)
        w_u16(character_offset + 684, r_u32(character_offset + 768));
    w_u32(flags_offset, r_u32(flags_offset) | 8);
    if ((r_u16(character_offset + 706) & 0xF) == 5 && (r_u32(character_offset + 4) & 0x80) == 0)
        w_u8(character_offset + 712, 3);
    if ((r_u32(object_offset + 28) & 0x400) == 0)
    {
        w_u32(character_offset, r_u32(character_offset) | 0x2000);
        if ((r_u32(object_offset + 28) & 0x8000000) != 0)
        {
            w_u32(flags_offset, r_u32(flags_offset) & 0xFFFFFBFE);
            w_u32(object_offset + 28, r_u32(object_offset + 28) & 0xF7FFFFFF);
        }
        if (r_s32(character_offset + 28) <= 0)
        {
            old_frame = r_s16(object_offset + 130);
            value = r_u32(character_offset + 716);
            w_u32(character_offset + 32, 1);
            w_u16(character_offset + 48, 0);
            w_u32(character_offset + 28, value);
            w_u16(object_offset + 130, 0xFFFF);
            if ((r_u32(character_offset) & 0x800000) != 0)
            {
                value = r_u32(character_offset + 28);
                speed = r_s32(r_u32(character_offset + 124) + 72);
                speed = (sint32)(uint32)(((sint64)(sint32)value * speed) >> 12);
                w_u32(character_offset + 28, (uint32)speed);
                if (speed <= 0)
                {
                    w_u32(flags_offset, r_u32(flags_offset) & 0xFFFFFFF7);
                    value = r_u32(character_offset + 768);
                    flags = r_u32(character_offset);
                    w_u32(character_offset + 28, 0);
                    w_u32(character_offset + 32, 0xFFFFFFFF);
                    w_u16(character_offset + 48, 0);
                    w_u16(character_offset + 684, value);
                    w_u32(character_offset, flags | 0x8000000);
                    w_u16(object_offset + 130, (uint32)old_frame);
                }
            }
            if (r_u32(character_offset + 28) == 0)
                goto done;
        }
        w_u32(character_offset + 8, r_u32(character_offset + 8) | 0x400000);
        flags = r_u32(object_offset + 28) | 0x400;
        w_u32(object_offset + 28, flags);
        if ((flags & 0x2000) == 0)
        {
            x = (uint32)(r_s32(character_offset + 692) >> 6);
            y = (uint32)(r_s32(character_offset + 696) >> 6);
            z = (uint32)(r_s32(character_offset + 700) >> 6);
            squared = x * x + y * y + z * z;
            if ((sint32)squared >= 0)
                length = (sint32)p2_sdk_8007e794(squared);
            else
                length = (sint32)(p2_sdk_8007e794(squared >> 2) << 1);
            angle = (0 - (uint32)p2_func_8005dcd4(r_s32(character_offset + 696), length)) & 0xFFF;
            if (r_s32(character_offset + 24) < 0)
                angle = (0 - angle) & 0xFFF;
            if ((sint16)angle >= 1025)
                angle += (uint32)((4096 - (sint16)angle) >> 1);
            else
            {
                angle <<= 1;
                if ((sint16)angle >= 1025)
                    angle = 1024;
            }
            if ((r_u32(object_offset + 228) & 4) != 0)
            {
                angle = 3584;
                w_u32(character_offset + 24, r_u32(character_offset + 740));
            }
            if ((sint16)angle >= 2048)
            {
                cosine = p2_func_8005dbb8((sint16)(1024 - angle)) >> 2;
                speed = r_s32(character_offset + 24);
                speed = (sint32)(uint32)(((sint64)cosine * speed) >> 12);
                if (((uint32)speed ^ r_u32(character_offset + 24)) >= 0x80000000)
                    speed = 0;
                w_u32(character_offset + 24, (uint32)speed);
            }
            else if ((sint16)angle >= 512)
            {
                w_u32(character_offset + 24, 0);
                w_u32(flags_offset, r_u32(flags_offset) & 0xFFFFFBFE);
            }
        }
        flags = r_u32(object_offset + 28);
        if ((flags & 0x8000000) != 0 && (r_u16(character_offset + 706) & 0xF) == 4)
            goto done;
        value = (uint32)r_u16(character_offset + 706) & 0xF;
        if (value - 6 < 2)
        {
            if (r_u32(character_offset + 24) != 0)
                value = (flags & 0x10) != 0 ? 34 : 33;
            else
                value = 3;
        }
        else
            value = 3;
        if ((r_u32(character_offset + 4) & 0x80) == 0)
            w_u8(character_offset + 712, value);
        p2_func_80044ffc(0, 3);
        goto done;
    }
    duration = r_s16(character_offset + 684);
    if (r_s32(character_offset + 32) > 0)
    {
        value = r_u16(character_offset + 48) + 1;
        w_u16(character_offset + 48, value);
        timer = (sint16)value;
        if (duration < timer || ((r_u32(character_offset) & 0x8000000) != 0 && (duration >> 1) < timer))
        {
            flags = r_u32(character_offset);
            w_u16(character_offset + 48, 0);
            w_u32(character_offset + 32, 0);
            w_u32(character_offset, flags & 0xEFFFFFFF);
        }
    }
    if ((r_u16(character_offset + 706) & 0xF) == 0)
    {
        mask = r_u8(r_u32(0x8008A2B4) + 1);
        if ((r_u32(0x1F800388) & mask) == 0 && (r_u32(0x1F800384) & mask) != 0 && (r_u32(character_offset) & 0x8000000) == 0 && r_u32(character_offset + 28) + 4504 < 0x4198)
            goto reactivate;
    }
    if ((r_u32(0x8008A52C) & 0x400) == 0)
        goto done;
    mask = r_u8(r_u32(0x8008A2B4) + 1);
    if ((r_u32(0x1F800388) & mask) != 0 || (r_u32(0x1F800384) & mask) == 0)
        goto done;
reactivate:
    value = r_u32(r_u32(character_offset + 780) + 12);
    flags = r_u32(character_offset + 768);
    x = r_u32(character_offset + 716);
    w_u16(character_offset + 684, flags);
    w_u16(character_offset + 48, 0);
    w_u32(character_offset + 28, x);
    w_u32(character_offset + 32, 1);
    w_u32(character_offset + 728, value);
    w_u16(object_offset + 130, 0xFFFF);
    if ((r_u32(character_offset + 4) & 0x80) == 0)
        w_u8(character_offset + 712, 9);
    p2_func_80044ffc(5, 3);
    w_u32(character_offset, (r_u32(character_offset) | 0x8002000) & 0xEFFFFFFF);
done:
    result = r_u32(flags_offset) & 8;
    if (result == 0)
    {
        result = r_u32(character_offset) & 0xFFFFDFFF;
        w_u32(character_offset, result);
    }
    return result;
}

uint32 p2_func_8004bfd0(uint32 object_offset, uint32 character_offset, uint32 flags_offset)
{
    uint32 flags, value, kind, i, result;
    sint32 timer;
    if ((r_u32(object_offset + 28) & 0x8000000) != 0 && (((uint32)r_u16(character_offset + 706) & 0xF) - 13 >= 2))
    {
        w_u32(flags_offset, r_u32(flags_offset) & 0xFFFFFBFE);
        goto secondary;
    }
    if ((r_u16(character_offset + 706) & 0xF) != 1)
    {
        w_u32(object_offset + 28, r_u32(object_offset + 28) & 0xFBFFFFFF);
        goto secondary;
    }
    if ((r_u32(object_offset + 28) & 0x4000000) != 0)
    {
        if (((r_u32(flags_offset) & 0x400) == 0 && (r_u8(object_offset + 13) & 1) == 0) || (r_u32(character_offset) & 0x18040) != 0 || r_s8(character_offset + 712) != 9)
            goto restore;
        value = r_u16(character_offset + 60) - 1;
        w_u16(character_offset + 60, value);
        if ((value & 0xFFFF) != 0)
            goto secondary;
    restore:
        w_u32(object_offset + 28, r_u32(object_offset + 28) & 0xFBFFFFFF);
        if (r_s8(character_offset + 712) == 9 && (r_u32(character_offset + 4) & 0x80) == 0)
            w_u8(character_offset + 712, 2);
        for (i = 0; i < 4; ++i)
            w_u32(character_offset + 736 + i * 4, r_u32(0x8008B4F4 + i * 4));
        w_u32(character_offset + 24, r_u32(0x8008B4F8));
        w_u32(0x8008B4F8, (uint32)-15);
    }
    else
    {
        timer = r_s32(0x8008B4F8);
        if (timer > 0)
        {
            w_u32(0x8008B4F8, 0);
            timer = r_s32(0x8008B4F8);
        }
        value = 0;
        if (timer == 0)
            value = 1;
        else
        {
            timer = (sint32)((uint32)timer + 1);
            w_u32(0x8008B4F8, (uint32)timer);
            if (timer == 0)
                value = 1;
        }
        if (value == 0 || (r_u32(flags_offset) & 0x400) == 0 || (r_u32(object_offset + 28) & 0x400) != 0)
        {
            w_u32(flags_offset, r_u32(flags_offset) & 0xFFFFFBFF);
            goto secondary;
        }
        w_u32(object_offset + 28, r_u32(object_offset + 28) | 0x4000000);
        if ((r_u32(character_offset + 4) & 0x80) == 0)
            w_u8(character_offset + 712, 9);
        value = r_u32(character_offset + 736);
        w_u16(character_offset + 60, 10);
        w_u32(0x8008B4F4, value);
        value = r_u32(character_offset + 740);
        w_u32(0x8008B4F8, value);
        value = (uint32)(((sint64)(sint32)value * 5120) >> 12);
        flags = r_u32(character_offset + 744);
        w_u32(character_offset + 740, value);
        w_u32(character_offset + 736, value);
        w_u32(0x8008B4FC, flags);
        value = r_u32(character_offset + 748);
        w_u32(0x8008B500, value);
        value = (uint32)(((sint64)(sint32)value * 5120) >> 12);
        w_u32(character_offset + 748, value);
        w_u32(character_offset + 744, value);
    }
secondary:
    if ((r_u32(flags_offset) & 0x80) != 0 && r_u32(r_u32(character_offset + 780) + 108) != 0)
    {
        flags = r_u32(object_offset + 28);
        if ((flags & 0x5000000) == 0 && ((r_u16(character_offset + 706) & 0xF) == 3 || (flags & 0x400) == 0))
            w_u32(object_offset + 28, flags | 0x8000000);
        result = r_u32(object_offset + 28) & 0x8000000;
        if (result != 0)
        {
            if ((r_u32(character_offset + 4) & 0x80) == 0)
                w_u8(character_offset + 712, 7);
            result = r_u32(character_offset + 500);
            if (result == 0)
                w_u32(character_offset + 24, 0);
        }
    }
    else
    {
        result = r_u32(object_offset + 28) & 0x8010000;
        if (result == 0x8000000)
        {
            kind = r_u16(character_offset + 706) & 0xF;
            if (kind == 4)
            {
                result = r_u32(character_offset) & 0x60000;
                if (result != 0)
                    return result;
            }
            if (r_s8(character_offset + 712) == 7 && (r_u32(character_offset + 4) & 0x80) == 0)
                w_u8(character_offset + 712, 0);
            result = r_u32(object_offset + 28) & 0xF7FFFFFF;
            w_u32(object_offset + 28, result);
        }
    }
    return result;
}

uint32 p2_func_8004c3b0(uint32 object_offset, uint32 character_offset, uint32 flags_offset)
{
    uint32 flags, value, kind, animation, i, result, decelerate;
    sint32 speed, state;
    if ((r_u32(character_offset) & 0x10000) != 0)
        goto input_actions;
    if ((r_u32(object_offset + 228) & 0x30000) != 0)
    {
        value = r_u32(character_offset + 288) + 1;
        kind = r_u8(0x8008A1BE);
        w_u32(character_offset + 288, value);
        if (value >= kind)
            w_u32(object_offset + 228, r_u32(object_offset + 228) & 0xFFFCFFFF);
    }
    decelerate = 0;
    if ((r_u32(flags_offset) & 0x401) == 0)
        decelerate = (r_u32(object_offset + 228) & 0x30000) == 0;
    p2_func_8004922c(object_offset, character_offset, decelerate);
    if ((r_u32(flags_offset) & 0x10000) != 0)
    {
        if ((r_u32(character_offset + 4) & 0x80) == 0)
            w_u8(character_offset + 712, 6);
    }
    else if ((r_u32(flags_offset) & 0x8000) != 0 && (r_u32(object_offset + 28) & 0x400) == 0)
    {
        if ((r_u32(flags_offset) & 0x800) != 0)
            animation = 17;
        else if ((r_u32(character_offset + 500) != 0 && r_s32(object_offset + 96) < 0) || r_s32(character_offset + 24) <= 0)
            animation = 16;
        else
            animation = 15;
        if ((r_u32(character_offset + 4) & 0x80) == 0)
            w_u8(character_offset + 712, animation);
    }
    else if ((r_u32(object_offset + 228) & 1) != 0)
    {
        if (r_u32(character_offset + 24) != 0)
            animation = (r_u32(0x1F800384) & 0x1000) != 0 ? 10 : 12;
        else
            animation = 11;
        if ((r_u32(character_offset + 4) & 0x80) == 0)
            w_u8(character_offset + 712, animation);
        if ((r_u32(flags_offset) & 0x800) != 0)
            w_u32(character_offset + 24, (uint32)(((sint64)r_s32(character_offset + 740) * r_s32(r_u32(character_offset + 780) + 68)) >> 12));
        else if ((r_u32(flags_offset) & 0x1000) != 0)
            w_u32(character_offset + 24, (uint32)(((sint64)r_s32(character_offset + 740) * r_s32(r_u32(character_offset + 780) + 72)) >> 12));
        else
            w_u32(character_offset + 24, 0);
    }
    flags = r_u32(object_offset + 28);
    if ((flags & 0x9000400) != 0 || (r_u32(object_offset + 228) & 0x1133) != 0)
        goto input_actions;
    speed = r_s32(character_offset + 24);
    if (speed != 0)
    {
        if ((r_u32(flags_offset) & 0x401) != 0)
        {
            kind = r_u16(character_offset + 706) & 0xF;
            if (kind == 1 && (flags & 0x4000000) != 0)
                goto input_actions;
            goto moving_animation;
        }
        animation = r_u8(character_offset + 712);
        if (animation - 4 < 2)
            goto input_actions;
        if (r_u32(character_offset + 500) != 0 || (r_u32(character_offset) & 8) != 0)
        {
            animation = 8;
            goto set_animation;
        }
        if (speed > 0)
        {
            kind = r_u16(character_offset + 706) & 0xF;
            goto moving_animation;
        }
        goto idle_animation;
    }
    if ((r_u16(character_offset + 706) & 0xF) == 1 && (flags & 0x4000000) != 0)
    {
        w_u32(object_offset + 28, flags & 0xFBFFFFFF);
        if (r_s8(character_offset + 712) == 9 && (r_u32(character_offset + 4) & 0x80) == 0)
            w_u8(character_offset + 712, 2);
        for (i = 0; i < 4; ++i)
            w_u32(character_offset + 736 + i * 4, r_u32(0x8008B4F4 + i * 4));
        w_u32(character_offset + 24, r_u32(0x8008B4F8));
        w_u32(0x8008B4F8, (uint32)-15);
        w_u32(character_offset + 24, 0);
    }
    animation = r_u8(character_offset + 712);
    if (animation - 4 < 2)
        goto input_actions;
    if ((r_u32(character_offset) & 8) != 0)
    {
        animation = 8;
        goto set_animation;
    }
    if ((r_u32(flags_offset) & 1) != 0)
    {
        kind = r_u16(character_offset + 706) & 0xF;
        flags = r_u32(object_offset + 28);
        goto moving_animation;
    }
idle_animation:
    if ((uint32)(sint32)(sint8)animation - 1 < 34)
    {
        animation = 0;
        goto set_animation;
    }
    goto input_actions;
moving_animation:
    if (kind - 6 < 2)
        animation = (flags & 0x10) != 0 ? 32 : 31;
    else
        animation = 2;
set_animation:
    if ((r_u32(character_offset + 4) & 0x80) == 0)
        w_u8(character_offset + 712, animation);
input_actions:
    if ((r_u32(flags_offset) & 0x100) != 0)
    {
        state = r_s8(character_offset + 712);
        w_u32(character_offset + 32, 1);
        w_u16(character_offset + 48, 0);
        if (state != 9 && state != 5)
        {
            if ((r_u32(character_offset + 4) & 0x80) == 0)
                w_u8(character_offset + 712, 9);
            w_u32(object_offset + 28, r_u32(object_offset + 28) | 0x400);
            if ((r_u32(character_offset) & 0x10000) != 0)
            {
                value = r_u32(character_offset + 88);
                w_u32(character_offset + 24, 0);
                w_u32(character_offset + 28, value);
                if ((sint32)value < 0)
                    w_u32(character_offset + 28, 0 - value);
                p2_func_8001c684(character_offset);
            }
        }
    }
    result = 16;
    if ((r_u32(object_offset + 228) & 1) != 0)
    {
        if (r_s8(0x8008A490) == 16)
            result = r_u16(character_offset + 266);
        else
            result = (r_u16(character_offset + 266) + 2048) & 0xFFF;
        w_u16(object_offset + 148, result);
        w_u16(object_offset + 150, result);
    }
    return result;
}

uint32 p2_func_8004ca00(uint32 object_offset, uint32 character_offset, uint32 flags_offset)
{
    uint32 kind, flags, result;
    if (r_u16(character_offset + 20) != 0)
        goto action;
    kind = r_u16(character_offset + 706) & 0xF;
    if (kind == 5 || kind - 10 < 2)
        goto action;
    if (kind == 1 && r_u32(0x8008A178) == 0 && (r_u32(object_offset + 28) & 0x8000000) == 0 && r_s8(character_offset + 712) != 4)
        goto action;
    result = r_u16(character_offset + 706) & 0xF;
    if (result != 4)
        return result;
action:
    result = r_u32(flags_offset) & 0x40;
    if (result == 0)
        return result;
    if (r_s16(character_offset + 52) != 0)
        return 0x5000000;
    flags = r_u32(object_offset + 28);
    result = flags & 0x5000000;
    if (result != 0)
        return result;
    result = r_u8(object_offset + 13) & 1;
    if (result != 0)
        return result;
    if (r_u32(r_u32(character_offset + 780) + 100) == 0)
        return 0x1000000;
    flags |= 0x1000000;
    w_u32(object_offset + 28, flags);
    kind = r_u16(character_offset + 706) & 0xF;
    if (kind == 1)
    {
        if ((r_u32(character_offset + 4) & 0x80) == 0)
            w_u8(character_offset + 712, 5);
        w_u16(object_offset + 130, 0xFFFF);
        w_u32(0x8008A4CC, 5);
        return 5;
    }
    if (kind != 4 && kind != 5 && kind - 10 >= 2)
    {
        if ((flags & 0x8000400) != 0 || r_s8(character_offset + 712) == 2)
            return 2;
    }
    if ((r_u32(character_offset + 4) & 0x80) == 0)
        w_u8(character_offset + 712, 5);
    w_u16(object_offset + 130, 0xFFFF);
    return 0xFFFFFFFF;
}

uint32 p2_func_8004cba4(uint32 object_offset, uint32 character_offset, uint32 input_flags)
{
    uint32 saved_stack = p2_guest_stack_pointer;
    uint32 horizontal, vertical, position, floor, original, flags, value, kind, state, matrix, node, i, grounded, result;
    sint32 lower_limit, acceleration, upper_limit, rise, speed, direction, frame;
    p2_guest_stack_pointer -= 0x50;
    horizontal = p2_guest_stack_pointer + 0x18;
    vertical = p2_guest_stack_pointer + 0x1C;
    position = p2_guest_stack_pointer + 0x20;
    floor = p2_guest_stack_pointer + 0x30;
    original = p2_guest_stack_pointer + 0x40;
    if ((input_flags & 2) != 0 && (r_u32(object_offset + 28) & 0x9000000) == 0 && (((uint32)r_u16(character_offset + 706) & 0xF) - 13 >= 2))
        goto prepare;
    if ((input_flags & 0x20) != 0 && (r_u32(object_offset + 228) & 2) != 0)
    {
    prepare:
        p2_func_80046cbc(object_offset, character_offset);
    }
    flags = r_u32(character_offset);
    if ((flags & 0x10000) != 0)
    {
        p2_func_8001d698(object_offset, character_offset);
        goto finish_motion;
    }
    if ((r_u32(object_offset + 28) & 0x400) == 0 && r_u32(character_offset + 24) == 0 && (flags & 0x99048) == 0 && r_u32(character_offset + 552) == 0 && (r_u32(object_offset + 228) & 0x122) == 0)
    {
        w_u32(object_offset + 100, 0);
        w_u32(object_offset + 96, 0);
        w_u32(object_offset + 92, 0);
        goto finish_motion;
    }
    flags = r_u32(object_offset + 28);
    value = r_u32(character_offset + 24);
    w_u32(horizontal, value);
    if ((flags & 0x400) == 0 && (r_u32(object_offset + 228) & 0x122) == 0)
    {
        w_u32(vertical, 0);
        goto adjust_horizontal;
    }
    lower_limit = r_s32(character_offset + 732);
    acceleration = r_s32(character_offset + 724);
    if ((input_flags & 0x20000) != 0 || (r_u32(object_offset + 228) & 0x200000) != 0)
    {
        p2_func_80058d08(object_offset, character_offset, input_flags, 0, horizontal, vertical);
        goto integrate_vertical;
    }
    if ((input_flags & 0x10000) != 0 && r_s8(0x8008A210) == 0)
    {
        value = (uint32)(((sint64)r_s32(0x8008A214) * r_s32(character_offset + 740)) >> 12);
        w_u32(character_offset + 28, value);
        flags = r_u32(character_offset + 28);
        w_u32(vertical, value);
        value = 0 - flags;
        w_u32(character_offset + 28, value);
        w_u32(vertical, value);
        goto integrate_vertical;
    }
    if ((input_flags & 0x2000) != 0 || (r_u32(object_offset + 228) & 4) != 0)
    {
        p2_func_80058f50(object_offset, character_offset, input_flags, (input_flags & 0x2000) != 0 ? 8 : 0, horizontal, vertical);
        goto integrate_vertical;
    }
    if ((input_flags & 0x200) != 0)
    {
        state = (uint32)r_s8(character_offset + 712);
        w_u32(character_offset + 32, 0xFFFFFFFF);
        w_u16(character_offset + 48, 0);
        if (state != 5 && (r_u32(character_offset + 4) & 0x80) == 0)
            w_u8(character_offset + 712, 7);
        goto integrate_vertical;
    }
    kind = r_u16(character_offset + 706) & 0xF;
    if ((kind != 5 && kind - 10 >= 2) || (input_flags & 0x100) != 0)
        goto integrate_vertical;
    if (r_s32(character_offset + 28) < lower_limit)
    {
        w_u32(character_offset + 32, 1);
        input_flags |= 0x100;
    }
    state = (uint32)r_s8(character_offset + 712);
    if (state != 3 || (input_flags & 8) != 0)
        goto check_animation;
    if ((r_u32(character_offset) & 0x60000) != 0)
    {
        state = (uint32)r_s8(character_offset + 712);
    check_animation:
        if (state == 5 || state == 3)
            goto integrate_vertical;
    }
    flags = r_u32(character_offset + 4);
    if ((flags & 0x400) == 0 && (flags & 0x80) == 0)
        w_u8(character_offset + 712, (input_flags & 3) != 0 ? 9 : 6);
integrate_vertical:
    if ((r_u32(character_offset) & 0x800000) != 0)
    {
        node = r_u32(character_offset + 124);
        upper_limit = (sint32)(((sint64)r_s32(character_offset + 728) * r_s32(node + 72)) >> 12);
        rise = (sint32)(((sint64)r_s32(character_offset + 720) * r_s32(node + 72)) >> 12);
    }
    else
    {
        upper_limit = r_s32(character_offset + 728);
        rise = r_s32(character_offset + 720);
    }
    flags = r_u32(object_offset + 228);
    if ((flags & 2) != 0 && (flags & 4) == 0)
    {
        direction = r_s32(character_offset + 32);
        if (direction > 0)
        {
            speed = r_s32(character_offset + 28);
            if (speed < upper_limit)
                w_u32(character_offset + 28, (uint32)speed);
            direction = r_s32(character_offset + 32);
        }
        if (direction < 0)
        {
            speed = r_s32(character_offset + 28);
            if (lower_limit < speed)
            {
                w_u32(character_offset + 28, (uint32)speed);
                if (speed < lower_limit)
                    goto clamp_lower;
            }
        }
        goto save_vertical;
    }
    direction = r_s32(character_offset + 32);
    if (direction > 0)
    {
        speed = r_s32(character_offset + 28);
        if (speed < upper_limit)
        {
            w_u32(character_offset + 28, (uint32)speed);
            if (((flags & 2) != 0 && (input_flags & 0x4000) != 0) || ((flags & 2) == 0 && (input_flags & 0x108) != 0))
                w_u32(character_offset + 28, (uint32)speed + (uint32)rise);
            speed = r_s32(character_offset + 28);
            w_u32(character_offset + 28, (uint32)speed);
            if (speed >= upper_limit)
            {
                w_u32(character_offset + 28, (uint32)upper_limit);
                w_u32(vertical, (uint32)upper_limit);
                goto adjust_horizontal;
            }
        }
        direction = r_s32(character_offset + 32);
    }
    if (direction < 0)
    {
        speed = r_s32(character_offset + 28);
        if (lower_limit < speed)
        {
            speed = (sint32)((uint32)speed + (uint32)acceleration);
            w_u32(character_offset + 28, (uint32)speed);
            if (speed < lower_limit)
            {
            clamp_lower:
                w_u32(character_offset + 28, (uint32)lower_limit);
                w_u32(vertical, (uint32)lower_limit);
                goto adjust_horizontal;
            }
        }
    }
save_vertical:
    w_u32(vertical, r_u32(character_offset + 28));
adjust_horizontal:
    if ((r_u32(character_offset) & 0x80008) != 0)
        p2_func_80049730(object_offset, character_offset, horizontal, vertical);
    else
    {
        value = r_u32(character_offset + 552);
        if (value != 0)
        {
            if ((r_u32(object_offset + 28) & 0x10) != 0)
                value = r_u32(character_offset + 24) + value;
            else
                value = r_u32(character_offset + 24) - value;
            w_u32(character_offset + 24, value);
            value = r_u32(character_offset + 24);
            w_u32(character_offset + 552, 0);
            w_u32(horizontal, value);
        }
    }
    speed = r_s32(horizontal);
    if (speed > 15360)
        w_u32(horizontal, 15360);
    else if (speed < -15360)
        w_u32(horizontal, (uint32)-15360);
    value = r_u32(horizontal);
    flags = r_u32(vertical);
    w_u32(character_offset + 84, value);
    w_u32(character_offset + 88, flags);
    flags = r_u32(object_offset + 28);
    grounded = (flags >> 10) & 1;
    if (grounded != 0)
        w_u32(character_offset, r_u32(character_offset) & 0xFFFFEFFF);
    else if ((r_u32(character_offset) & 0x1000) != 0)
        w_u32(object_offset + 28, flags | 0x400);
    p2_func_800455dc(object_offset, character_offset, r_s32(horizontal), r_s32(vertical));
    flags = r_u32(character_offset);
    if ((flags & 0x40) != 0)
        p2_func_8001db60(object_offset, character_offset, grounded);
    else if ((flags & 0x8000) != 0)
        p2_func_8001eb44(object_offset, character_offset);
    if (grounded != 0)
        goto finish_motion;
    if ((r_u32(r_u32(object_offset + 132) + 16) & 0x11) != 0)
        w_u32(object_offset + 28, r_u32(object_offset + 28) | 0x400);
    if ((r_u32(object_offset + 28) & 0x400) == 0)
        goto finish;
    if (r_s32(horizontal) <= 0 || (r_u32(character_offset) & 0x1000) != 0)
        goto stop_vertical;
    node = r_u32(r_u32(character_offset + 688) + 36);
    if ((r_u32(node + 16) & 0x11) != 0)
    {
    stop_vertical:
        value = r_u32(character_offset + 768);
        flags = r_u32(character_offset);
        kind = r_u16(character_offset + 706) & 0xF;
        w_u32(character_offset + 32, 0xFFFFFFFF);
        w_u32(character_offset + 28, 0);
        w_u16(character_offset + 48, 0);
        w_u16(character_offset + 684, value);
        w_u32(character_offset, flags | 0x8000000);
        if ((kind != 1 || (r_u32(object_offset + 28) & 0x4000000) == 0) && (r_u32(object_offset + 228) & 0x100) == 0 && (r_u32(character_offset + 4) & 0x80) == 0)
            w_u8(character_offset + 712, 6);
        goto finish_motion;
    }
    w_u32(horizontal, (uint32)r_s16(node + 26));
    node = r_u32(object_offset + 132);
    if (node != 0 && r_u32(node + 40) != 0)
    {
        value = r_u32(object_offset + 16);
        flags = r_u32(object_offset + 20);
        kind = r_u32(object_offset + 24);
        w_u32(original, value);
        w_u32(original + 4, flags);
        w_u32(original + 8, kind);
        PushMatrix();
        matrix = r_u32(r_u32(object_offset + 132) + 40);
        for (i = 0; i < 5; ++i)
            xport_gte_write_control(i, r_u32(matrix + i * 4));
        p2_sdk_8007ec64(original, position);
        PopMatrix();
        for (i = 0; i < 3; ++i)
        {
            matrix = r_u32(r_u32(object_offset + 132) + 40);
            value = r_u32(position + i * 4);
            value += r_u32(matrix + 20 + i * 4);
            w_u32(position + i * 4, value);
        }
    }
    else
    {
        value = r_u32(object_offset + 16);
        flags = r_u32(object_offset + 20);
        kind = r_u32(object_offset + 24);
        w_u32(position, value);
        w_u32(position + 4, flags);
        w_u32(position + 8, kind);
    }
    for (i = 0; i < 3; ++i)
    {
        value = r_u32(position + i * 4);
        value += r_u32(object_offset + 92 + i * 4);
        w_u32(position + i * 4, value);
    }
    node = r_u32(r_u32(character_offset + 688) + 36);
    frame = r_s32(horizontal);
    direction = p2_func_8003d3fc(node, frame, position, floor, r_u32(object_offset + 28) & 0x10, 6144);
    if (direction == r_s32(horizontal))
    {
        w_u32(object_offset + 28, r_u32(object_offset + 28) & 0xFFFFFBFF);
        w_u32(character_offset, r_u32(character_offset) | 0x1000);
    }
finish_motion:
    if ((r_u32(object_offset + 28) & 0x400) != 0 && r_u32(character_offset + 32) == 0)
    {
        flags = r_u32(character_offset);
        w_u32(character_offset + 32, 0xFFFFFFFF);
        w_u32(character_offset, flags & 0xEFFFFFFF);
    }
finish:
    node = r_u32(0x8008A114);
    w_u32(character_offset + 180, 0);
    node = r_u32(node + 136);
    if ((r_u32(r_u32(node + 36) + 16) & 0x100011) != 0)
        w_u32(character_offset + 500, 0);
    else
        w_u32(character_offset + 500, r_u32(node + 28));
    result = r_u32(character_offset) & 0xF855B041;
    w_u32(character_offset, result);
    p2_guest_stack_pointer = saved_stack;
    return result;
}

uint32 p2_func_80022ec4(uint32 source_offset, uint32 destination_offset, sint32 position)
{
    uint32 saved_stack = p2_guest_stack_pointer;
    uint32 vector, node, index, value, i, result;
    sint32 x, y, z;
    p2_guest_stack_pointer -= 0x20;
    vector = p2_guest_stack_pointer + 0x10;
    for (i = 0; i < 3; ++i)
    {
        if (position != 0)
        {
            index = r_u8(source_offset + 20);
            node = r_u32(source_offset + 32) + index * 40;
            value = r_u32(node - 36 + i * 4);
            value -= r_u32(node - 76 + i * 4);
        }
        else
        {
            node = r_u32(source_offset + 32);
            value = r_u32(node + 4 + i * 4);
            value -= r_u32(node + 44 + i * 4);
        }
        w_u32(vector + i * 4, value);
    }
    while (r_u32(vector) + 26754 > 0xD103 || r_u32(vector + 4) + 26754 > 0xD103 || r_u32(vector + 8) + 26754 > 0xD103)
    {
        x = r_s32(vector) >> 1;
        y = r_s32(vector + 4) >> 1;
        z = r_s32(vector + 8) >> 1;
        w_u32(vector, (uint32)x);
        w_u32(vector + 4, (uint32)y);
        w_u32(vector + 8, (uint32)z);
    }
    VectorNormal((VECTOR *)psx_addr(vector, 12), (VECTOR *)psx_addr(vector, 12));
    if (r_u32(source_offset + 40) != 0)
    {
        PushMatrix();
        node = r_u32(source_offset + 40);
        for (i = 0; i < 5; ++i)
            xport_gte_write_control(i, r_u32(node + i * 4));
        result = p2_sdk_8007ec64(vector, destination_offset);
        PopMatrix();
    }
    else
    {
        result = r_u32(vector);
        value = r_u32(vector + 4);
        node = r_u32(vector + 8);
        w_u32(destination_offset, result);
        w_u32(destination_offset + 4, value);
        w_u32(destination_offset + 8, node);
    }
    p2_guest_stack_pointer = saved_stack;
    return result;
}

uint32 p2_func_8005bb90(sint32 first, sint32 second)
{
    uint32 high, low;
    if (first == 95 || second == 95)
        return 0;
    if ((uint32)first - 48 < 10)
        high = (uint32)first - 48;
    else if ((uint32)first - 97 < 26)
        high = (uint32)first - 87;
    else if ((uint32)first - 65 < 26)
        high = (uint32)first - 29;
    else
        return 0xFFFFFFFF;
    if ((uint32)second - 48 < 10)
        low = (uint32)second - 48;
    else if ((uint32)second - 97 < 26)
        low = (uint32)second - 87;
    else if ((uint32)second - 65 < 26)
        low = (uint32)second - 29;
    else
        return 0xFFFFFFFF;
    return high * 62 + low + 1;
}

uint32 p2_func_8005bb48(uint32 event, uint32 kind)
{
    uint32 queue = r_u32(0x8008A114);
    sint32 count = r_s8(queue + 1);
    uint32 result = count < 40;
    if (result != 0)
    {
        w_u8(queue + 1, (uint32)count + 1);
        result = event | (kind << 17);
        w_u32(queue + (uint32)count * 4 + 140, result);
    }
    return result;
}

void p2_func_8005c588(uint32 object_offset, uint32 auxiliary_offset)
{
    uint32 saved_stack = p2_guest_stack_pointer;
    uint32 bounds, delta, extent, attenuation, pan, node, candidate, flags, active, value, x, y, z, radius, limit, packed, delay, count, kind, table, sound, index, voice, divisor, pitch, result, queue;
    sint32 volume, queue_count;
    p2_guest_stack_pointer -= 0x60;
    bounds = p2_guest_stack_pointer + 0x28;
    delta = p2_guest_stack_pointer + 0x38;
    extent = p2_guest_stack_pointer + 0x48;
    attenuation = p2_guest_stack_pointer + 0x4C;
    pan = p2_guest_stack_pointer + 0x50;
    for (node = auxiliary_offset; node != 0; node = r_u32(node))
    {
        if ((r_u8(node + 13) & 0x81) != 0)
            continue;
        flags = r_u8(node + 14);
        w_u8(node + 14, flags & 0xBF);
        x = r_u32(node + 36);
        y = r_u32(node + 40);
        z = r_u32(node + 44);
        w_u32(bounds, x);
        w_u32(bounds + 4, y);
        w_u32(bounds + 8, z);
        value = r_u16(node + 66);
        table = r_u32(0x8008A4E8);
        w_u32(bounds + 12, value << 5);
        candidate = r_u32(table + 44);
        active = (flags >> 6) & 1;
        for (; candidate != 0; candidate = r_u32(candidate))
        {
            kind = (r_u32(candidate + 72) >> 12) & 0x1F;
            if (kind - 3 >= 2)
                break;
            if ((r_u8(candidate + 13) & 0x81) != 0 || (r_u8(candidate + 71) & 0xFE) != 0)
                continue;
            if (active == 0 && (r_u8(candidate + 14) & 0x40) == 0)
                continue;
            radius = r_u16(candidate + 66);
            radius = r_u32(bounds + 12) + (radius << 5) + 20480;
            value = r_u32(bounds);
            value = value - r_u32(candidate + 36) + radius;
            limit = radius << 1;
            w_u32(extent, radius);
            if (value > limit)
                continue;
            value = r_u32(bounds + 4);
            value = value - r_u32(candidate + 40) + radius;
            if (value > limit)
                continue;
            value = r_u32(bounds + 8);
            value = value - r_u32(candidate + 44) + radius;
            if (value > limit)
                continue;
            for (index = 0; index < 3; ++index)
            {
                value = r_u32(node + 48 + index * 4);
                value -= r_u32(candidate + 48 + index * 4);
                w_u32(delta + index * 4, value);
            }
            if (p2_func_80038990(bounds, candidate, delta, extent, 0) != 0)
            {
                w_u8(candidate + 14, r_u8(candidate + 14) | 8);
                w_u8(node + 14, r_u8(node + 14) | 0x40);
            }
        }
    }
    for (node = object_offset; node != 0; node = r_u32(node))
    {
        flags = r_u8(node + 14);
        kind = r_u8(node + 15);
        flags &= 0xBF;
        w_u8(node + 14, flags);
        if (kind - 40 >= 16)
            break;
        packed = r_u32(node + 68);
        delay = (packed >> 25) * 2;
        count = (packed >> 17) & 0xFF;
        if (delay != 0)
        {
            --delay;
            if (delay == 0)
                w_u8(node + 14, flags | 0x40);
            value = r_u32(node + 68) & 0x1FFFF;
            w_u32(node + 68, value | (count << 17) | ((delay >> 1) << 25));
        }
        flags = r_u8(node + 14);
        if ((flags & 8) != 0)
            goto activate;
        packed = r_u32(node + 72);
        if (((packed >> 12) & 0x1F) != 1 || (r_u8(node + 13) & 0x81) != 0 || delay != 0)
            goto stop_voice;
        if ((flags & 0x10) == 0)
            goto activate;
        count = (packed >> 17) & 0xFF;
        if (count == 0)
            goto activate;
        result = p2_func_80068198();
        count = result % (count * 15);
        if (count != 0)
            continue;
    activate:
        packed = r_u32(node + 72);
        flags = r_u8(node + 14) | 0x48;
        delay = (packed >> 25) * 2;
        w_u8(node + 14, flags);
        if (count != 0)
        {
            --count;
            value = r_u32(node + 68) & 0x1FFFF;
            w_u32(node + 68, value | (count << 17) | (delay << 24));
            if (delay == 254)
                w_u8(node + 13, r_u8(node + 13) | 0x80);
            continue;
        }
        kind = (uint32)r_s8(node + 15);
        count = (packed >> 17) & 0xFF;
        w_u8(node + 14, flags & 0xF7);
        if (kind == 55)
        {
            w_u32(0x1F800320, r_u32(0x1F800320) | 0x8000);
            goto reset;
        }
        if (kind != 50)
        {
            queue = r_u32(0x8008A114);
            packed = r_u32(node + 72);
            value = r_u32(node + 68);
            queue_count = r_s8(queue + 1);
            kind = ((packed >> 12) & 0x1F) - 1;
            if (queue_count < 40)
            {
                w_u8(queue + 1, (uint32)queue_count + 1);
                w_u32(queue + (uint32)queue_count * 4 + 140, value | (kind << 17));
            }
            goto reset;
        }
        if (delay == 0 && count == 0)
        {
            if ((flags & 2) == 0 && r_s8(node + 86) >= 0)
                p2_func_8001bb58(node);
            table = r_u32(0x8008A460);
            sound = 0;
            if (table != 0)
            {
                index = r_u32(node + 68) & 0xFFF;
                if (index < r_u32(table))
                    sound = table + index * 8 + 4;
            }
            packed = r_u32(node + 76);
            pitch = r_u16(sound + 4);
            radius = r_u16(node + 66) << 5;
            result = p2_func_8005eb18(sound, (sint32)((packed >> 16) & 0x3FFF), (packed >> 8) & 0xFF, pitch, node + 36, radius, packed & 0xFF, 1, 1);
            w_u32(node + 76, (r_u32(node + 76) & 0xFFFFFF00) | (result & 0xFF));
            goto reset;
        }
        packed = r_u32(node + 76);
        volume = (sint32)((packed >> 16) & 0x3FFF);
        if (volume == 0)
            goto reset;
        if ((packed & 0x40000000) != 0)
        {
            divisor = (((uint32)volume - 1) & 0xFFF) >> 1;
            if (divisor != 0)
                volume = (sint32)((uint32)volume - p2_func_80068198() % divisor);
        }
        value = r_u8(node + 77);
        w_u32(pan, value);
        if (value == 0)
        {
            if ((r_u8(node + 14) & 2) == 0 && r_s8(node + 86) >= 0)
                p2_func_8001bb58(node);
            radius = r_u16(node + 66) << 5;
            p2_func_8005e8bc(node + 36, radius, volume >= 4097 ? attenuation : 0, pan);
            if (volume >= 4097)
            {
                if (volume >= 8193)
                {
                    volume -= 4096;
                    w_u32(pan, 5);
                }
                value = r_u32(attenuation) * ((uint32)volume - 4096);
                volume = (sint32)value >> 12;
            }
        }
        volume = (sint32)((uint32)volume * (uint32)(sint32)r_s16(0x8008A24C)) >> 12;
        if (volume == 0)
            goto reset;
        table = r_u32(0x8008A460);
        sound = 0;
        if (table != 0)
        {
            index = r_u32(node + 68) & 0xFFF;
            if (index < r_u32(table))
                sound = table + index * 8 + 4;
        }
        pitch = 0;
        if (r_s32(node + 76) < 0)
        {
            pitch = r_u16(sound + 4);
            divisor = pitch >> 2;
            if (divisor != 0)
            {
                result = p2_func_80068198();
                pitch += result % divisor - (pitch >> 3);
            }
        }
        value = r_u16(pan);
        p2_func_8005e538(sound, 0, pitch, (uint32)volume & 0xFFFF, value, 0);
    reset:
        if (delay == 254)
            w_u8(node + 13, r_u8(node + 13) | 0x80);
        else
            w_u32(node + 68, (r_u32(node + 68) & 0x1FFFF) | (count << 17) | ((delay >> 1) << 25));
        continue;
    stop_voice:
        if (r_s8(node + 15) != 50)
            continue;
        packed = r_u32(node + 72);
        if (((packed >> 17) & 0xFF) != 0 || (packed >> 25) != 0)
            continue;
        voice = r_u8(node + 76);
        if (voice >= 8)
            continue;
        table = r_u32(0x8008A460);
        sound = 0;
        if (table != 0)
        {
            index = r_u32(node + 68) & 0xFFF;
            if (index < r_u32(table))
                sound = table + index * 8 + 4;
        }
        if ((p2_func_8005e868(sound, voice) & 0xFF) != 0)
            p2_func_8006a970((uint16)voice);
        w_u32(node + 76, r_u32(node + 76) | 0xFF);
    }
    p2_guest_stack_pointer = saved_stack;
}

uint32 p2_func_80047c1c(void)
{
    uint32 node, object, result, value;
    sint32 kind;
    for (node = r_u32(0x800AC100); node != 0; node = r_u32(node + 4))
    {
        value = r_u32(0x800AC104);
        object = r_u32(node);
        if (p2_func_8005baac(value, object) == 0)
        {
            object = r_u32(node);
            kind = r_s8(object + 15);
            if (kind == 53)
                p2_func_8005bb48((r_u32(object + 68) & 0xFFF) | 0x4000, 4);
            else if (kind == 54)
                p2_func_8005bb48((r_u32(object + 68) & 0xFFF) | 0x3000, 4);
        }
    }
    p2_func_8005bb14(0x800AC100);
    result = r_u32(0x800AC104);
    w_u32(0x800AC104, 0);
    w_u32(0x800AC100, result);
    return result;
}

void p2_func_8005bb14(uint32 head_offset)
{
    uint32 node = r_u32(head_offset);
    uint32 next, free_head;
    while (node != 0)
    {
        next = r_u32(node + 4);
        free_head = r_u32(0x8008A34C);
        w_u32(0x8008A34C, node);
        w_u32(node + 4, free_head);
        node = next;
    }
    w_u32(head_offset, 0);
}

uint32 p2_func_8005baac(uint32 head_offset, uint32 object_offset)
{
    while (head_offset != 0)
    {
        if (r_u32(head_offset) == object_offset)
            return 1;
        head_offset = r_u32(head_offset + 4);
    }
    return 0;
}

uint32 p2_func_8003a930(uint32 object_offset)
{
    uint32 node, matrix, value, parent, result, radius, limit, target;
    p2_func_80039ea4(object_offset);
    parent = r_u32(object_offset + 132);
    if (parent != 0)
    {
        matrix = r_u32(parent + 40);
        if (matrix != 0 && (r_u32(object_offset + 28) & 0x2000) == 0)
        {
            value = r_u32(object_offset + 92) + r_u32(matrix + 32);
            parent = r_u32(object_offset + 132);
            w_u32(object_offset + 92, value);
            matrix = r_u32(parent + 40);
            value = r_u32(object_offset + 96) + r_u32(matrix + 36);
            parent = r_u32(object_offset + 132);
            w_u32(object_offset + 96, value);
            matrix = r_u32(parent + 40);
            value = r_u32(object_offset + 100) + r_u32(matrix + 40);
            w_u32(object_offset + 100, value);
        }
    }
    p2_func_8003a384(object_offset);
    if ((r_u8(object_offset + 13) & 1) != 0)
        goto restore;
    for (node = r_u32(0x8008A4EC); node != 0; node = r_u32(node))
    {
        if (node == object_offset || (r_u8(node + 13) & 0x81) != 0 || r_s8(node + 126) == 0)
            continue;
        value = (uint32)r_s8(object_offset + 125);
        radius = r_u32(node + 72) + (value == 30 ? 184320 : 61440);
        limit = radius << 1;
        value = r_u32(object_offset + 60);
        value = value - r_u32(node + 60) + radius;
        if (value > limit)
            continue;
        value = r_u32(object_offset + 64);
        value = value - r_u32(node + 64) + radius;
        if (value > limit)
            continue;
        value = r_u32(object_offset + 68);
        value = value - r_u32(node + 68) + radius;
        if (value > limit)
            continue;
        target = r_u32(0x800B7A48 + (uint32)(sint32)r_s8(object_offset + 126) * 4);
        if ((p2_dispatch_collision_callback(target, object_offset, node, 0) & 0xFF) != 0)
            p2_func_80039614(object_offset, node);
    }
    if (r_s8(object_offset + 126) == 4)
    {
        for (node = r_u32(r_u32(0x8008A4E8) + 32); node != 0; node = r_u32(node))
        {
            if (node == object_offset || (r_u8(node + 13) & 0x81) != 0 || r_s8(node + 126) == 0)
                continue;
            value = r_u32(object_offset + 60);
            value = value - r_u32(node + 60) + 81920;
            if (value > 163840)
                continue;
            value = r_u32(object_offset + 64);
            value = value - r_u32(node + 64) + 81920;
            if (value > 163840)
                continue;
            value = r_u32(object_offset + 68);
            value = value - r_u32(node + 68) + 81920;
            if (value > 163840)
                continue;
            target = r_u32(0x800B7A48 + (uint32)(sint32)r_s8(object_offset + 126) * 4);
            if ((p2_dispatch_collision_callback(target, object_offset, node, 0) & 0xFF) != 0)
                p2_func_80039614(object_offset, node);
        }
    }
restore:
    result = r_u32(object_offset + 132);
    if (result != 0)
    {
        matrix = r_u32(result + 40);
        if (matrix != 0)
        {
            result = r_u32(object_offset + 28) & 0x2000;
            if (result == 0)
            {
                result = r_u32(object_offset + 92) - r_u32(matrix + 32);
                parent = r_u32(object_offset + 132);
                w_u32(object_offset + 92, result);
                matrix = r_u32(parent + 40);
                result = r_u32(object_offset + 96) - r_u32(matrix + 36);
                parent = r_u32(object_offset + 132);
                w_u32(object_offset + 96, result);
                matrix = r_u32(parent + 40);
                result = r_u32(object_offset + 100) - r_u32(matrix + 40);
                w_u32(object_offset + 100, result);
            }
        }
    }
    return result;
}

uint32 p2_func_80039ea4(uint32 object_offset)
{
    uint32 saved_stack = p2_guest_stack_pointer;
    uint32 state, geometry, mask, node, selected, matrix, index, source, destination, x, y, z, radius, limit, value, target, result;
    sint32 kind, hit, selection;
    kind = r_s8(object_offset + 126);
    result = 4;
    if (kind == 1)
        goto search;
    if (kind == 3)
    {
        if (r_s8(object_offset + 120) != -2)
            goto search;
        value = r_u32(object_offset + 116);
        if (value == 23 || value == 42)
            goto search;
    }
    if (kind != 4)
        return result;
    target = r_u32(0x800B7A58);
    result = p2_dispatch_collision_callback(target, object_offset, 0, 0) << 24;
    if (result == 0)
        return result;
search:
    p2_guest_stack_pointer -= 0x30;
    state = p2_guest_stack_pointer + 0x28;
    geometry = object_offset + 160;
    if (r_s8(object_offset + 126) == 1)
    {
        geometry = p2_func_8001bd30(object_offset);
        p2_func_8001ecec();
    }
    mask = (r_u32(object_offset + 28) & 0x200) != 0 ? 0x100000 : 0x100200;
    node = r_u32(0x8008A4E8);
    x = r_u32(0x80010180);
    y = r_u32(0x80010184);
    z = r_u32(0x80010188);
    w_u32(0x1F8003A0, x);
    w_u32(0x1F8003A4, y);
    w_u32(0x1F8003A8, z);
    value = r_u32(0x8001018C);
    x = (uint32)r_s16(0x80010190);
    w_u32(0x1F8003AC, value);
    w_u16(0x1F8003B0, x);
    w_u32(0x1F8003C8, 0);
    w_u32(0x1F8003C4, 0);
    w_u32(0x1F8003C0, 0);
    w_u16(0x1F80039C, 0);
    node = r_u32(node + 36);
    value = r_u32(object_offset + 144);
    selection = -1;
    mask |= 0x2000;
    w_u32(state, value);
    for (; node != 0; node = r_u32(node))
    {
        if ((r_u32(node + 28) & mask) != 0x100000 || (r_u8(node + 13) & 1) != 0)
            continue;
        if (r_s16(node + 34) > 0)
        {
            x = r_u32(node + 16);
            y = r_u32(node + 20);
            z = r_u32(node + 24);
        }
        else
        {
            matrix = r_u32(node + 40);
            x = r_u32(matrix + 16);
            y = r_u32(matrix + 20);
            z = r_u32(matrix + 24);
        }
        w_u32(0x1F8003B4, x);
        w_u32(0x1F8003B8, y);
        w_u32(0x1F8003BC, z);
        radius = r_u16(node + 14) << 5;
        limit = (radius + 61440) << 1;
        value = r_u32(object_offset + 60);
        value = value - r_u32(node + 16) + radius + 61440;
        if (value > limit)
            continue;
        value = r_u32(object_offset + 64);
        value = value - r_u32(node + 20) + radius + 61440;
        if (value > limit)
            continue;
        value = r_u32(object_offset + 68);
        value = value - r_u32(node + 24) + radius + 61440;
        if (value > limit)
            continue;
        if ((p2_func_80039404(object_offset, node + 16, object_offset + 92, radius) & 0xFF) == 0)
            continue;
        w_u16(0x1F80039E, r_u16(node + 32));
        hit = p2_func_800380ac(object_offset, geometry, node, 0x1F80039C, state);
        if (hit >= 0)
        {
            selection = (sint32)((uint32)hit & 0xFFFF);
            selected = node;
        }
    }
    node = r_u32(r_u32(0x8008A4E8) + 28);
    mask &= 0xFFFFDFFF;
    for (; node != 0; node = r_u32(node))
    {
        if ((r_u8(node + 13) & 0x81) != 0 || (r_u32(node + 28) & mask) != 0x100000)
            continue;
        if (r_s8(object_offset + 126) == 4 && node == r_u32(object_offset + 8))
            continue;
        radius = r_u32(node + 72) + 61440;
        limit = radius << 1;
        value = r_u32(object_offset + 60);
        value = value - r_u32(node + 60) + radius;
        if (value > limit)
            continue;
        value = r_u32(object_offset + 64);
        value = value - r_u32(node + 64) + radius;
        if (value > limit)
            continue;
        value = r_u32(object_offset + 68);
        value = value - r_u32(node + 68) + radius;
        if (value > limit)
            continue;
        for (index = 0; (sint32)index < r_s16(node + 138); ++index)
        {
            source = r_u32(node + 140) + index * 48;
            for (destination = 0x1F80039C; destination != 0x1F8003CC; destination += 16)
            {
                value = r_u32(source);
                x = r_u32(source + 4);
                y = r_u32(source + 8);
                z = r_u32(source + 12);
                w_u32(destination, value);
                w_u32(destination + 4, x);
                w_u32(destination + 8, y);
                w_u32(destination + 12, z);
                source += 16;
            }
            hit = p2_func_800380ac(object_offset, geometry, node, 0x1F80039C, state);
            if (hit >= 0)
            {
                selected = node;
                value = (uint32)hit + (uint32)(sint32)r_s16(0x1F80039C);
                selection = (sint32)((index << 16) | (value & 0xFFFF));
            }
        }
    }
    if (r_s8(object_offset + 126) == 1)
        p2_func_8001ed4c();
    result = 0x7FFF0000;
    if (selection >= 0)
    {
        value = r_u32(state);
        w_u32(0x1F800310, (uint32)selection);
        w_u32(0x1F80031C, 0x7FFFF000);
        w_u32(0x1F800314, value);
        target = r_u32(0x800B7A48 + (uint32)(sint32)r_s8(object_offset + 126) * 4);
        result = p2_dispatch_collision_callback(target, object_offset, selected, 0x1F800310) << 24;
        if (result != 0)
        {
            value = r_u32(state);
            mask = r_u32(object_offset + 28);
            w_u32(object_offset + 144, value);
            result = 0xFFFFBFFF;
            w_u32(object_offset + 28, mask & result);
        }
    }
    p2_guest_stack_pointer = saved_stack;
    return result;
}

uint32 p2_func_8001ecec(void)
{
    uint32 result = r_u32(0x800ABD68) & 0x18040;
    uint32 node, index;
    if (result != 0)
    {
        node = r_u32(0x800ABDD8);
        index = r_u32(0x800ABDD4) & 0xFFFF;
        node = r_u32(node + 36) + index * 24;
        result = r_u16(node + 22) & 0xFFFE;
        w_u16(node + 22, result);
    }
    return result;
}

uint32 p2_func_80039404(uint32 object_offset, uint32 position_offset, uint32 velocity_offset, uint32 radius)
{
    uint32 combined, radius_square, start_square, velocity_square, end_square, x, y, z, vx, vy, vz, scaled, difference, left, right;
    combined = radius + r_u32(object_offset + 72);
    radius_square = (uint32)(((sint64)(sint32)combined * (sint32)combined) >> 12);
    x = r_u32(position_offset) - r_u32(object_offset + 60);
    y = r_u32(position_offset + 4) - r_u32(object_offset + 64);
    z = r_u32(position_offset + 8) - r_u32(object_offset + 68);
    x = (uint32)((sint32)x >> 6);
    y = (uint32)((sint32)y >> 6);
    z = (uint32)((sint32)z >> 6);
    start_square = x * x + y * y + z * z;
    if ((sint32)radius_square >= (sint32)start_square)
        return 1;
    vx = r_u32(velocity_offset);
    x = (uint32)((sint32)vx >> 6);
    vy = r_u32(velocity_offset + 4);
    y = (uint32)((sint32)vy >> 6);
    vz = r_u32(velocity_offset + 8);
    z = (uint32)((sint32)vz >> 6);
    velocity_square = x * x + y * y + z * z;
    if (velocity_square == 0)
        return 0;
    x = r_u32(object_offset + 60) + vx;
    y = r_u32(object_offset + 64) + r_u32(velocity_offset + 4);
    z = r_u32(object_offset + 68) + r_u32(velocity_offset + 8);
    x = r_u32(position_offset) - x;
    y = r_u32(position_offset + 4) - y;
    z = r_u32(position_offset + 8) - z;
    x = (uint32)((sint32)x >> 6);
    y = (uint32)((sint32)y >> 6);
    z = (uint32)((sint32)z >> 6);
    end_square = x * x + y * y + z * z;
    if ((sint32)radius_square >= (sint32)end_square)
        return 1;
    difference = end_square - velocity_square - start_square;
    scaled = velocity_square << 2;
    right = (uint32)(((sint64)(sint32)start_square * (sint32)scaled) >> 12);
    right -= (uint32)(((sint64)(sint32)difference * (sint32)difference) >> 12);
    left = (uint32)(((sint64)(sint32)radius_square * (sint32)scaled) >> 12);
    return (sint32)left >= (sint32)right;
}

uint32 p2_func_8001ed4c(void)
{
    uint32 result = r_u32(0x800ABD68) & 0x18040;
    uint32 node, index;
    if (result != 0)
    {
        node = r_u32(0x800ABDD8);
        index = r_u32(0x800ABDD4) & 0xFFFF;
        node = r_u32(node + 36) + index * 24;
        result = r_u16(node + 22) | 1;
        w_u16(node + 22, result);
    }
    return result;
}

sint32 p2_func_800380ac(uint32 object_offset, uint32 geometry_offset, uint32 other_offset, uint32 collision_offset, uint32 state_offset)
{
    uint32 saved_stack = p2_guest_stack_pointer;
    uint32 start, end, point, vertex_base, face_base, face, sphere, selected_face = 0, selected_sphere;
    uint32 shape, face_index, edge, matrix, i, radius, radius_square, margin, value;
    uint32 motion_x, motion_y, motion_z, x, y, z, sx, sy, sz, ex, ey, ez;
    uint32 vx, vy, vz, nx, ny, nz, dx, dy, dz, cx, cy, cz, px, py, pz;
    uint32 dot, start_distance, end_distance, fraction, edge_radius, edge_square, length_square, bound;
    sint32 face_count = r_s16(collision_offset + 2);
    sint32 reciprocal, angle, result;
    if (face_count <= 0)
        return -1;
    p2_guest_stack_pointer -= 0x50;
    start = p2_guest_stack_pointer + 0x20;
    end = p2_guest_stack_pointer + 0x30;
    point = p2_guest_stack_pointer + 0x40;
    vertex_base = r_u32(other_offset + 40);
    face_base = r_u32(other_offset + 36) + (uint32)(sint32)r_s16(collision_offset) * 24;
    if (r_s16(other_offset + 34) <= 0)
        vertex_base = r_u32(vertex_base + 40);
    motion_x = r_u32(object_offset + 92) - r_u32(collision_offset + 36);
    motion_y = r_u32(object_offset + 96) - r_u32(collision_offset + 40);
    motion_z = r_u32(object_offset + 100) - r_u32(collision_offset + 44);
    matrix = r_u32(object_offset + 132);
    if (matrix != 0)
    {
        matrix = r_u32(matrix + 40);
        if (matrix != 0 && (r_u32(object_offset + 28) & 0x2000) == 0)
        {
            motion_x += r_u32(matrix + 32);
            matrix = r_u32(r_u32(object_offset + 132) + 40);
            motion_y += r_u32(matrix + 36);
            matrix = r_u32(r_u32(object_offset + 132) + 40);
            motion_z += r_u32(matrix + 40);
        }
    }
    for (shape = 0; shape < 4; ++shape)
    {
        sphere = geometry_offset + shape * 16;
        radius = r_u32(sphere + 12);
        if (radius == 0xFFFFFFFF)
            continue;
        value = (uint32)(((sint64)(sint32)radius * 4014) >> 12);
        radius_square = (uint32)(((sint64)(sint32)value * (sint32)value) >> 12);
        margin = (uint32)(((sint64)(sint32)radius * 3686) >> 12);
        x = r_u32(sphere) - r_u32(collision_offset + 24);
        w_u32(end, x);
        y = r_u32(sphere + 4) - r_u32(collision_offset + 28);
        w_u32(end + 4, y);
        z = r_u32(sphere + 8) - r_u32(collision_offset + 32);
        w_u32(end + 8, z);
        PushMatrix();
        for (i = 0; i < 5; ++i)
            xport_gte_write_control(i, r_u32(collision_offset + 4 + i * 4));
        p2_sdk_8007ec64(end, start);
        x = (uint32)(r_s32(start) >> 5);
        y = (uint32)(r_s32(start + 4) >> 5);
        z = (uint32)(r_s32(start + 8) >> 5);
        w_u32(start, x);
        w_u32(start + 4, y);
        w_u32(start + 8, z);
        x = r_u32(end) + motion_x;
        y = r_u32(end + 4) + motion_y;
        z = r_u32(end + 8) + motion_z;
        w_u32(point, x);
        w_u32(point + 4, y);
        w_u32(point + 8, z);
        p2_sdk_8007ec64(point, end);
        PopMatrix();
        x = (uint32)(r_s32(end) >> 5);
        y = (uint32)(r_s32(end + 4) >> 5);
        z = (uint32)(r_s32(end + 8) >> 5);
        w_u32(end, x);
        w_u32(end + 4, y);
        w_u32(end + 8, z);
        for (face_index = 0; face_index < (uint32)face_count; ++face_index)
        {
            face = face_base + face_index * 24;
            if ((r_u16(face + 22) & 1) == 0)
                continue;
            vx = (uint32)(sint32)r_s16(vertex_base + r_u16(face + 10) * 16);
            vy = (uint32)(sint32)r_s16(vertex_base + r_u16(face + 10) * 16 + 2);
            vz = (uint32)(sint32)r_s16(vertex_base + r_u16(face + 10) * 16 + 4);
            sx = r_u32(start);
            sy = r_u32(start + 4);
            sz = r_u32(start + 8);
            dx = sx - vx;
            dy = sy - vy;
            dz = sz - vz;
            nx = (uint32)(sint32)r_s16(face + 16);
            ny = (uint32)(sint32)r_s16(face + 18);
            nz = (uint32)(sint32)r_s16(face + 20);
            dot = (uint32)((sint32)(dx * nx + dy * ny + dz * nz) >> 7);
            if ((sint32)(dot + margin) <= 0)
                continue;
            ex = r_u32(end);
            ey = r_u32(end + 4);
            ez = r_u32(end + 8);
            dx = ex - vx;
            dy = ey - vy;
            dz = ez - vz;
            nx = (uint32)(sint32)r_s16(face + 16);
            ny = (uint32)(sint32)r_s16(face + 18);
            nz = (uint32)(sint32)r_s16(face + 20);
            end_distance = (uint32)((sint32)(dx * nx + dy * ny + dz * nz) >> 7) - radius;
            start_distance = dot - radius;
            if ((sint32)end_distance >= 0)
                continue;
            if ((sint32)start_distance > 0)
            {
                value = start_distance - end_distance;
                if (value == 0)
                    xport_mips_break(7);
                reciprocal = value == 0 ? -1 : 0x40000000 / (sint32)value;
                fraction = (uint32)(((sint64)(sint32)start_distance * reciprocal) >> 18);
                w_u32(0x1F80030C, fraction);
                dx = ex - sx;
                dy = ey - sy;
                dz = ez - sz;
                w_u32(point, dx);
                w_u32(point + 4, dy);
                w_u32(point + 8, dz);
                if (r_s32(state_offset) < (sint32)fraction)
                    continue;
                px = (uint32)(((sint64)(sint32)dx * (sint32)fraction) >> 12);
                py = (uint32)(((sint64)(sint32)dy * (sint32)fraction) >> 12);
                pz = (uint32)(((sint64)(sint32)dz * (sint32)fraction) >> 12);
                w_u32(point, px);
                w_u32(point + 4, py);
                w_u32(point + 8, pz);
                px += sx;
                py += sy;
                pz += sz;
                w_u32(point, px);
                w_u32(point + 4, py);
                w_u32(point + 8, pz);
                edge_radius = radius_square;
            }
            else
            {
                if ((sint32)end_distance >= (sint32)start_distance)
                    continue;
                dot = start_distance + radius;
                if ((sint32)dot < 0)
                {
                    angle = p2_func_8005dd68((sint32)(0 - dot), (sint32)radius);
                    angle = p2_func_8005dbb8((sint16)(1024 - (uint32)angle)) >> 2;
                    value = (uint32)(((sint64)(sint32)radius * angle) >> 12);
                    edge_radius = (uint32)(((sint64)(sint32)value * (sint32)value) >> 12);
                }
                else
                    edge_radius = radius_square;
                px = r_u32(start);
                py = r_u32(start + 4);
                pz = r_u32(start + 8);
                w_u32(point, px);
                w_u32(point + 4, py);
                w_u32(point + 8, pz);
                fraction = 0;
            }
            for (edge = 0; edge < 4; ++edge)
            {
                x = vx;
                y = vy;
                z = vz;
                vx = (uint32)(sint32)r_s16(vertex_base + r_u16(face + edge * 2 + 4) * 16);
                vy = (uint32)(sint32)r_s16(vertex_base + r_u16(face + edge * 2 + 4) * 16 + 2);
                vz = (uint32)(sint32)r_s16(vertex_base + r_u16(face + edge * 2 + 4) * 16 + 4);
                dx = x - vx;
                dy = y - vy;
                dz = z - vz;
                nx = (uint32)(sint32)r_s16(face + 16);
                ny = (uint32)(sint32)r_s16(face + 18);
                nz = (uint32)(sint32)r_s16(face + 20);
                cx = (uint32)((sint32)(ny * dz - dy * nz) >> 7);
                cy = (uint32)((sint32)(nz * dx - dz * nx) >> 7);
                cz = (uint32)((sint32)(nx * dy - dx * ny) >> 7);
                dx = (r_u32(point) - vx) << 5;
                dy = (r_u32(point + 4) - vy) << 5;
                dz = (r_u32(point + 8) - vz) << 5;
                dot = (uint32)(((sint64)(sint32)cx * (sint32)dx) >> 12);
                dot += (uint32)(((sint64)(sint32)cy * (sint32)dy) >> 12);
                dot += (uint32)(((sint64)(sint32)cz * (sint32)dz) >> 12);
                if ((sint32)dot >= 0)
                {
                    if ((sint32)dot > 0x200000)
                        break;
                    edge_square = (uint32)(((sint64)(sint32)dot * (sint32)dot) >> 12);
                    length_square = (uint32)(((sint64)(sint32)cx * (sint32)cx) >> 12);
                    length_square += (uint32)(((sint64)(sint32)cy * (sint32)cy) >> 12);
                    length_square += (uint32)(((sint64)(sint32)cz * (sint32)cz) >> 12);
                    bound = (uint32)(((sint64)(sint32)edge_radius * (sint32)length_square) >> 12);
                    if (bound < edge_square)
                        break;
                }
            }
            if (edge == 4)
            {
                w_u32(state_offset, fraction);
                selected_face = face;
                selected_sphere = sphere;
            }
        }
    }
    if (selected_face == 0)
        result = -1;
    else
    {
        value = (uint32)((sint32)(selected_sphere - geometry_offset) >> 4) + 1;
        x = (uint32)((sint32)((selected_face - face_base) * 0xAAAAAAAB) >> 3);
        result = (sint32)(x | (value << 12));
    }
    p2_guest_stack_pointer = saved_stack;
    return result;
}

uint32 p2_func_8003a384(uint32 object_offset)
{
    uint32 saved_stack = p2_guest_stack_pointer;
    uint32 velocity, extent, kind, node, radius, limit, value, target, context, x, y, z, shape, nearest, result;
    p2_guest_stack_pointer -= 0x30;
    velocity = p2_guest_stack_pointer + 0x18;
    extent = p2_guest_stack_pointer + 0x28;
    kind = (uint32)r_s8(object_offset + 126);
    result = 4;
    if (kind == 2)
    {
        result = r_u32(0x8008A4E8);
        for (node = r_u32(result + 48); node != 0; node = r_u32(node))
        {
            result = 61440;
            if ((r_u8(node + 13) & 0x81) != 0)
                continue;
            radius = (r_u16(node + 66) << 5) + 61440;
            limit = radius << 1;
            value = r_u32(object_offset + 60);
            value = value - r_u32(node + 36) + radius;
            if (value > limit)
                continue;
            value = r_u32(object_offset + 64);
            value = value - r_u32(node + 40) + radius;
            result = value > limit;
            if (result != 0)
                continue;
            value = r_u32(object_offset + 68);
            value = value - r_u32(node + 44) + radius;
            result = value > limit;
            if (result != 0)
                continue;
            target = r_u32(0x800B7A48 + (uint32)(sint32)r_s8(object_offset + 126) * 4);
            result = p2_dispatch_collision_callback(target, object_offset, node, 0) << 24;
            if (result != 0)
                result = p2_func_80039b20(object_offset, node);
        }
        goto done;
    }
    if (kind == 4)
        goto done;
    for (node = r_u32(r_u32(0x8008A4E8) + 44); node != 0; node = r_u32(node))
    {
        if ((r_u8(node + 13) & 0x85) != 0)
            continue;
        radius = (r_u16(node + 66) << 5) + 61440;
        limit = radius << 1;
        value = r_u32(object_offset + 60);
        value = value - r_u32(node + 36) + radius;
        if (value > limit)
            continue;
        value = r_u32(object_offset + 64);
        value = value - r_u32(node + 40) + radius;
        if (value > limit)
            continue;
        value = r_u32(object_offset + 68);
        value = value - r_u32(node + 44) + radius;
        if (value > limit)
            continue;
        target = r_u32(0x800B7A48 + (uint32)(sint32)r_s8(object_offset + 126) * 4);
        if ((p2_dispatch_collision_callback(target, object_offset, node, 0) & 0xFF) != 0)
            p2_func_80039b20(object_offset, node);
    }
    kind = (uint32)r_s8(object_offset + 126);
    context = 0;
    if (kind == 1)
    {
        if (r_s8(object_offset + 125) == 33)
            context = r_u32(object_offset + 112);
        else
            context = r_u32(r_u32(0x8008A114) + 136);
    }
    else if (kind == 3 && r_s8(object_offset + 120) == -1)
        context = r_u32(object_offset + 112);
    if ((p2_func_80047058(object_offset, 0, context) & 0xFF) != 0)
    {
        for (node = r_u32(r_u32(0x8008A4E8) + 52); node != 0; node = r_u32(node))
        {
            if ((r_u8(node + 13) & 0x85) != 0)
                continue;
            radius = (r_u16(node + 66) << 5) + 61440;
            limit = radius << 1;
            value = r_u32(object_offset + 160);
            value = value - r_u32(node + 36) + radius;
            if (value > limit)
                continue;
            value = r_u32(object_offset + 164);
            value = value - r_u32(node + 40) + radius;
            if (value > limit)
                continue;
            value = r_u32(object_offset + 168);
            value = value - r_u32(node + 44) + radius;
            if (value > limit)
                continue;
            if ((p2_func_80047058(object_offset, node, context) & 0xFF) != 0)
                p2_func_80039b20(object_offset, node);
        }
    }
    kind = (uint32)r_s8(object_offset + 126);
    result = 19;
    if (kind != 1 && (kind != 3 || r_u32(object_offset + 116) != 19))
        goto done;
    result = r_u32(0x8008A4E8);
    for (node = r_u32(result + 60); node != 0; node = r_u32(node))
    {
        if ((r_u16(node + 18) & 0x8000) != 0)
        {
            p2_func_8005bb48(r_u16(node + 16) | 0x4000, 4);
            w_u16(node + 18, r_u16(node + 18) & 0x7FFF);
        }
        result = r_u8(node + 13) & 0x85;
        if (result != 0)
            continue;
        value = r_u32(object_offset + 60);
        value = value - r_u32(node + 40) + 61440;
        result = value > 0x1E000;
        if (result != 0)
            continue;
        value = r_u32(object_offset + 64);
        value = value - r_u32(node + 44) + 61440;
        result = value > 0x1E000;
        if (result != 0)
            continue;
        value = r_u32(object_offset + 68);
        value = value - r_u32(node + 48) + 61440;
        result = value > 0x1E000;
        if (result != 0)
            continue;
        target = r_u32(0x800B7A48 + (uint32)(sint32)r_s8(object_offset + 126) * 4);
        result = p2_dispatch_collision_callback(target, object_offset, node, 0) << 24;
        if (result == 0)
            continue;
        x = r_u32(object_offset + 92);
        y = r_u32(object_offset + 96);
        z = r_u32(object_offset + 100);
        w_u32(velocity, x);
        w_u32(velocity + 4, y);
        w_u32(velocity + 8, z);
        nearest = r_u32(object_offset + 144) + 1;
        for (shape = 0; shape < 4; ++shape)
        {
            if (r_u32(object_offset + 172 + shape * 16) == 0xFFFFFFFF)
                break;
            if (p2_func_80038990(object_offset + 160 + shape * 16, node, velocity, extent, 0) != 0)
            {
                value = r_u32(extent);
                if ((sint32)value < (sint32)nearest)
                    nearest = value;
            }
        }
        result = r_s32(object_offset + 144) < (sint32)nearest;
        if (result != 0)
            continue;
        w_u32(0x1F800314, nearest);
        w_u32(0x1F80031C, 0);
        target = r_u32(0x800B7A48 + (uint32)(sint32)r_s8(object_offset + 126) * 4);
        value = p2_dispatch_collision_callback(target, object_offset, node, 0x1F800310) << 24;
        result = 0xFFFFBFFF;
        if (value != 0)
        {
            value = r_u32(object_offset + 28);
            w_u32(object_offset + 144, nearest);
            w_u32(object_offset + 28, value & result);
        }
    }
done:
    p2_guest_stack_pointer = saved_stack;
    return result;
}
