#include "game.h"


uint32 p2_func_8005bb90(sint32 first, sint32 second)
{
    FUNCTION_MARKER(0x8005BB90, "SLUS_005.78");
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
    FUNCTION_MARKER(0x8005BB48, "SLUS_005.78");
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
    FUNCTION_MARKER(0x8005C588, "SLUS_005.78");
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
        result = random_generator.Next();
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
            game_flow.transition_flags = game_flow.transition_flags | 0x8000;
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
                volume = (sint32)((uint32)volume - random_generator.Next() % divisor);
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
        volume = (sint32)((uint32)volume * (uint32)(sint32)(sint16)(sound_settings.effects_volume)) >> 12;
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
                result = random_generator.Next();
                pitch += result % divisor - (pitch >> 3);
            }
        }
        value = r_u16(pan);
        sound_effects.Queue(sound, 0, pitch, (uint32)volume & 0xFFFF, value, 0);
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
        if ((sound_effects.IsPlaying(sound, voice) & 0xFF) != 0)
            p2_func_8006a970((uint16)voice);
        w_u32(node + 76, r_u32(node + 76) | 0xFF);
    }
    p2_guest_stack_pointer = saved_stack;
}

uint32 p2_func_8003a930(uint32 object_offset)
{
    FUNCTION_MARKER(0x8003A930, "SLUS_005.78");
    CollisionHandler collision_callback = 0;
    uint32 node, matrix, value, parent, result, radius, limit;
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
    for (node = registered_object_head; node != 0; node = r_u32(node))
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
        collision_callback = object_operations.collisions[r_s8(object_offset + 126)];
        if ((collision_callback(object_offset, node, 0) & 0xFF) != 0)
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
            collision_callback = object_operations.collisions[r_s8(object_offset + 126)];
            if ((collision_callback(object_offset, node, 0) & 0xFF) != 0)
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
    FUNCTION_MARKER(0x80039EA4, "SLUS_005.78");
    CollisionHandler collision_callback = 0;
    uint32 saved_stack = p2_guest_stack_pointer;
    uint32 state, geometry, mask, node, selected, matrix, index, source, destination, x, y, z, radius, limit, value, result;
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
    collision_callback = object_operations.collisions[4];
    result = collision_callback(object_offset, 0, 0) << 24;
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
        collision_contact.selection = (uint32)selection;
        collision_contact.flags = 0x7FFFF000;
        collision_contact.fraction = value;
        collision_callback = object_operations.collisions[r_s8(object_offset + 126)];
        result = collision_callback(object_offset, selected, &collision_contact) << 24;
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
    FUNCTION_MARKER(0x8001ECEC, "SLUS_005.78");
    uint32 result = character_controller.flags & 0x18040;
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
    FUNCTION_MARKER(0x80039404, "SLUS_005.78");
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
    FUNCTION_MARKER(0x8001ED4C, "SLUS_005.78");
    uint32 result = character_controller.flags & 0x18040;
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
    FUNCTION_MARKER(0x800380AC, "SLUS_005.78");
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
                    angle = AcosAngle((sint32)(0 - dot), (sint32)radius);
                    angle = SinQ14((sint16)(1024 - (uint32)angle)) >> 2;
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
    FUNCTION_MARKER(0x8003A384, "SLUS_005.78");
    CollisionHandler collision_callback = 0;
    uint32 saved_stack = p2_guest_stack_pointer;
    uint32 velocity, extent, kind, node, radius, limit, value, context, x, y, z, shape, nearest, result;
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
            collision_callback = object_operations.collisions[r_s8(object_offset + 126)];
            result = collision_callback(object_offset, node, 0) << 24;
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
        collision_callback = object_operations.collisions[r_s8(object_offset + 126)];
        if ((collision_callback(object_offset, node, 0) & 0xFF) != 0)
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
        collision_callback = object_operations.collisions[r_s8(object_offset + 126)];
        result = collision_callback(object_offset, node, 0) << 24;
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
        collision_contact.fraction = nearest;
        collision_contact.flags = 0;
        collision_callback = object_operations.collisions[r_s8(object_offset + 126)];
        value = collision_callback(object_offset, node, &collision_contact) << 24;
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
