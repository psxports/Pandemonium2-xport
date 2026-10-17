#include "game.h"

uint32 p2_func_8002e00c(uint32 light_offset)
{
    FUNCTION_MARKER(0x8002E00C, "SLUS_005.78");
    uint32 saved_stack, difference, cached_position, mark_all, scene, node, result;
    uint32 flags, radius, threshold, x, y, z, a, b, c;
    saved_stack = p2_guest_stack_pointer;
    p2_guest_stack_pointer -= 32;
    difference = p2_guest_stack_pointer;
    cached_position = difference + 16;
    mark_all = 0;
    if (light_offset == 0 || (r_u8(light_offset + 14) & 2) == 0)
        mark_all = 1;
    result = r_u32(0x8008A4E8);
    node = r_u32(result + 28);
    if (node != 0)
    {
        do
        {
            if (r_s16(node + 32) != 0)
            {
                flags = r_u32(node + 28);
                if ((flags & 1) == 0)
                {
                    result = flags | 1;
                    if (mark_all == 0)
                    {
                        x = r_u32(node + 60);
                        y = r_u32(node + 64);
                        z = r_u32(node + 68);
                        w_u32(cached_position, x);
                        w_u32(cached_position + 4, y);
                        w_u32(cached_position + 8, z);
                        radius = r_u32(node + 72);
                        x = r_u32(light_offset + 32);
                        a = r_u32(cached_position);
                        b = r_u32(cached_position + 4);
                        x -= a;
                        w_u32(difference, x);
                        y = r_u32(light_offset + 36);
                        c = r_u32(cached_position + 8);
                        y -= b;
                        w_u32(difference + 4, y);
                        z = r_u32(light_offset + 40) - c;
                        w_u32(difference + 8, z);
                        radius += r_u32(light_offset + 80) << 5;
                        threshold = (uint32)(((sint64)(sint32)radius * (sint32)radius) >> 12);
                        x = (uint32)((sint32)x >> 6);
                        y = (uint32)((sint32)y >> 6);
                        z = (uint32)((sint32)z >> 6);
                        if (x * x + y * y + z * z >= threshold)
                        {
                            x = r_u32(light_offset + 44) - a;
                            w_u32(cached_position, x);
                            y = r_u32(light_offset + 48) - b;
                            w_u32(cached_position + 4, y);
                            z = r_u32(light_offset + 52) - c;
                            w_u32(cached_position + 8, z);
                            x = (uint32)((sint32)x >> 6);
                            y = (uint32)((sint32)y >> 6);
                            z = (uint32)((sint32)z >> 6);
                            if (x * x + y * y + z * z >= threshold)
                                goto next_first;
                        }
                        result = r_u32(node + 28) | 1;
                    }
                    w_u32(node + 28, result);
                }
            }
        next_first:
            node = r_u32(node);
        } while (node != 0);
        result = r_u32(0x8008A4E8);
    }
    scene = result;
    node = r_u32(scene + 36);
    while (node != 0)
    {
        if (mark_all == 0)
        {
            result = r_u32(node + 28) & 1;
            if (result != 0)
                goto next_second;
            x = r_u32(light_offset + 32);
            x -= r_u32(node + 16);
            w_u32(difference, x);
            y = r_u32(light_offset + 36);
            y -= r_u32(node + 20);
            w_u32(difference + 4, y);
            z = r_u32(light_offset + 40);
            z -= r_u32(node + 24);
            w_u32(difference + 8, z);
            radius = r_u16(node + 14);
            radius += r_u32(light_offset + 80);
            threshold = (radius * radius) >> 2;
            x = (uint32)((sint32)x >> 6);
            y = (uint32)((sint32)y >> 6);
            z = (uint32)((sint32)z >> 6);
            result = x * x + y * y + z * z < threshold;
            if (result == 0)
            {
                // Retain the original cache, including untouched local memory
                x = r_u32(light_offset + 44);
                x -= r_u32(cached_position);
                w_u32(cached_position, x);
                y = r_u32(light_offset + 48);
                y -= r_u32(cached_position + 4);
                w_u32(cached_position + 4, y);
                z = r_u32(light_offset + 52);
                z -= r_u32(cached_position + 8);
                w_u32(cached_position + 8, z);
                x = (uint32)((sint32)x >> 6);
                y = (uint32)((sint32)y >> 6);
                z = (uint32)((sint32)z >> 6);
                result = x * x + y * y + z * z < threshold;
                if (result == 0)
                    goto next_second;
            }
        }
        result = r_u32(node + 28) | 1;
        w_u32(node + 28, result);
    next_second:
        node = r_u32(node);
    }
    p2_guest_stack_pointer = saved_stack;
    return result;
}



uint32 p2_func_80039b20(uint32 object_offset, uint32 other_offset)
{
    FUNCTION_MARKER(0x80039B20, "SLUS_005.78");
    CollisionHandler collision_callback = 0;
    uint32 saved_stack, delta, shape_flags, fraction, selected_index;
    uint32 value, flags, radius, first_filter, last_filter, best, side, index, result;
    sint32 kind;
    saved_stack = p2_guest_stack_pointer;
    p2_guest_stack_pointer -= 32;
    delta = p2_guest_stack_pointer;
    shape_flags = delta + 16;
    fraction = delta + 20;
    selected_index = delta + 24;
    value = r_u32(object_offset + 92);
    w_u32(delta, value - r_u32(other_offset + 48));
    value = r_u32(object_offset + 96);
    w_u32(delta + 4, value - r_u32(other_offset + 52));
    value = r_u32(object_offset + 100);
    w_u32(delta + 8, value - r_u32(other_offset + 56));
    radius = r_u16(other_offset + 66) << 5;
    result = 1;
    if ((p2_func_80039404(object_offset, other_offset + 36, delta, radius) & 0xFF) == 0)
        goto finish_contact;
    kind = r_s8(other_offset + 15);
    first_filter = 0;
    if (kind != 1)
    {
        flags = r_u32(object_offset + 228);
        if ((flags & 2) != 0 || (flags & 0x180) == 0x80)
        {
            last_filter = 1;
            goto select_contact;
        }
        kind = r_s8(other_offset + 15);
    }
    if (kind == 1 || kind == 24 || kind == 6 || ((uint8)(kind - 40) < 16 && ((r_u32(other_offset + 72) >> 12) & 31) == 15))
        first_filter = 1;
    last_filter = 0;
    if (first_filter == 0 && (uint32)r_u8(other_offset + 15) - 40 < 16)
        last_filter = ((r_u32(other_offset + 72) >> 12) & 31) == 16;
select_contact:
    side = 0xFFFFFFFF;
    best = r_u32(object_offset + 144) + 1;
    for (index = 0; index < 4; ++index)
    {
        if (r_u32(object_offset + 172 + index * 16) == 0xFFFFFFFF)
            break;
        p2_func_8001c69c(object_offset, index + 1, shape_flags, shape_flags + 1);
        if (first_filter != 0)
        {
            if (r_s8(shape_flags) != 0)
                goto test_contact;
        }
        else if (last_filter == 0)
            goto test_contact;
        if (last_filter == 0 || r_s8(shape_flags + 1) == 0)
            continue;
    test_contact:
        value = p2_func_80038990(object_offset + 160 + index * 16, other_offset, delta, fraction, 0);
        if (value != 0)
        {
            flags = r_u32(fraction);
            if ((sint32)flags < (sint32)best)
            {
                best = flags;
                side = value;
                w_u32(selected_index, index);
            }
        }
    }
    if (r_s32(object_offset + 144) >= (sint32)best)
    {
        value = r_u32(selected_index);
        collision_contact.selection = side | ((value + 1) << 12);
        collision_contact.fraction = best;
        collision_contact.flags = 0;
        kind = r_s8(object_offset + 126);
        collision_callback = object_operations.collisions[kind];
        if ((collision_callback(object_offset, other_offset, &collision_contact) & 0xFF) != 0)
        {
            flags = r_u32(object_offset + 28);
            w_u32(object_offset + 144, best);
            w_u32(object_offset + 28, flags & 0xFFFFBFFF);
        }
    }
    flags = r_u32(object_offset + 228);
    result = flags & 0x80;
    if ((flags & 2) != 0)
    {
        value = gameplay_input.buttons;
        w_u32(0x8008A480, 0);
        if ((value & 0x1000) != 0)
        {
            result = 1;
            if (r_s32(object_offset + 144) < (sint32)best)
                w_u32(0x8008A480, 1);
        }
        else
        {
            result = value & 0x4000;
            if (result != 0)
            {
                result = 0xFFFFFFFF;
                if (r_s32(object_offset + 144) < (sint32)best)
                    w_u32(0x8008A480, 0xFFFFFFFF);
            }
        }
    }
    else if (result != 0)
    {
        value = r_u32(object_offset + 144);
        w_u32(0x8008A448, 0);
        result = 1;
        if ((sint32)value < (sint32)best)
            w_u32(0x8008A448, 1);
    }
finish_contact:
    p2_guest_stack_pointer = saved_stack;
    return result;
}

uint32 p2_func_8005d1b0(uint32 object_offset)
{
    FUNCTION_MARKER(0x8005D1B0, "SLUS_005.78");
    uint32 saved_stack, floor, normal, velocity, position, input, first_extent, second_extent;
    uint32 model, owner, frame, direction, side, flags, value, parent, matrix, x, y, z, a, b, c;
    uint32 scene, walk_scene, node, first_seen, second_seen, next, candidate_owner, hit, best;
    uint32 radius, target, index, squared, magnitude, result;
    sint32 kind;
    saved_stack = p2_guest_stack_pointer;
    p2_guest_stack_pointer -= 96;
    floor = p2_guest_stack_pointer;
    normal = floor + 16;
    velocity = floor + 32;
    position = floor + 48;
    input = floor + 64;
    first_extent = floor + 80;
    second_extent = floor + 84;
    model = r_u32(object_offset + 156);
    if (r_s16(model + 40) == 0 || r_s16(model + 42) == 0)
        goto hide;
    flags = r_u32(object_offset + 228);
    if ((flags & 0x11) != 0 || (flags & 0x300) == 0x100)
        goto hide;
    flags = r_u32(object_offset + 28);
    w_u32(model + 36, 0);
    w_u32(model + 32, 0);
    w_u32(model + 28, 0);
    owner = r_u32(object_offset + 132);
    side = 0xFFFFFFFF;
    frame = r_u16(object_offset + 136);
    direction = flags & 0x10;
    if (owner == 0)
        goto no_path;
    if ((r_u32(owner + 16) & 8) != 0 && r_u32(r_u32(owner + 44) + 56) != 0)
        goto hide;
    if (r_u32(owner + 32) == 0)
        goto no_path;
    flags = r_u32(object_offset + 28);
    hit = 0;
    if ((flags & 0x2400) == 0)
        goto orient;
    best = 0x7FFFF000;
    if ((flags & 0x2000) != 0)
    {
        x = r_u32(object_offset + 16);
        y = r_u32(object_offset + 20);
        z = r_u32(object_offset + 24);
        w_u32(position, x);
        w_u32(position + 4, y);
        w_u32(position + 8, z);
        goto search_objects;
    }
    first_seen = 0;
    second_seen = 0;
    parent = r_u32(object_offset + 132);
    if (parent != 0 && r_u32(parent + 40) != 0)
    {
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
        p2_sdk_8007ec64(input, position);
        PopMatrix();
        for (index = 0; index < 3; ++index)
        {
            matrix = r_u32(r_u32(object_offset + 132) + 40);
            value = r_u32(position + index * 4);
            w_u32(position + index * 4, value + r_u32(matrix + 20 + index * 4));
        }
    }
    else
    {
        x = r_u32(object_offset + 16);
        y = r_u32(object_offset + 20);
        z = r_u32(object_offset + 24);
        w_u32(position, x);
        w_u32(position + 4, y);
        w_u32(position + 8, z);
    }
    scene = r_u32(0x8008A114);
    node = r_u32(scene + 136);
    if (node != 0)
    {
        walk_scene = scene;
        for (;;)
        {
            flags = r_u32(r_u32(node + 36) + 16);
            if ((flags & 0x51) < 0x41)
                break;
            if ((flags & 0x10) == 0)
            {
                if ((r_u16(node + 14) & (direction | 8)) != 0)
                    node = r_u32(node + 20);
                else
                    node = r_u32(node + 16);
                if (first_seen == 0)
                    first_seen = node;
                else if (second_seen == 0)
                    second_seen = node;
                else if (node == first_seen || node == second_seen)
                    node = 0;
            }
            else
            {
                next = r_u32(node + 28);
                if (next == 0)
                    node = 0;
                else if ((r_u16(node + 14) & (direction | 8)) != 0)
                    node = next;
                else
                {
                    value = r_u16(next + 14);
                    node = next;
                    if ((value & 8) != 0)
                        node = r_u32(next + 16);
                }
            }
            if (node == 0)
                break;
            next = r_u32(walk_scene + 136);
            candidate_owner = r_u32(node + 36);
            if (candidate_owner == r_u32(next + 36))
                node = 0;
            if (node == 0)
                break;
        }
        scene = r_u32(0x8008A114);
    }
    if (node != r_u32(scene + 136) && node != 0)
    {
        frame = FindPathHeightFrame(r_u32(node + 36), position, direction, -40960);
        if ((sint16)frame >= 0)
            owner = r_u32(node + 36);
    }
    frame = (uint32)ProjectPathFrame(owner, (sint16)frame, position, floor, direction, 4096);
    if ((sint16)frame < 0)
        owner = r_u32(object_offset + 132);
    else
    {
        value = r_u32(position + 4);
        value -= r_u32(floor + 4);
        best = (uint32)((sint32)value / 50);
    }
search_objects:
    value = r_u32(position + 4);
    scene = r_u32(0x8008A4E8);
    w_u32(position + 12, 341);
    w_u32(position + 4, value - 341);
    for (node = r_u32(scene + 44); node != 0; node = r_u32(node))
    {
        if ((r_u8(node + 13) & 0x81) != 0)
            continue;
        kind = r_s8(node + 15);
        if (kind != 6 && kind != 9)
            continue;
        radius = r_u16(node + 66) << 5;
        x = r_u32(position);
        a = r_u32(node + 36);
        radius += 20480;
        value = x - a + radius;
        w_u32(first_extent, radius);
        if (value > (radius << 1))
            continue;
        value = r_u32(position + 8);
        value = value - r_u32(node + 44) + radius;
        if (value > (radius << 1))
            continue;
        value = r_u32(position + 4);
        value = value - r_u32(node + 40) + 204800;
        if (value > 409600)
            continue;
        w_u32(velocity + 8, 0);
        w_u32(velocity, 0);
        w_u32(velocity + 4, (uint32)-204800);
        value = p2_func_80038990(position, node, velocity, first_extent, 0);
        if (value != 0)
        {
            radius = r_u32(first_extent);
            if ((sint32)radius < (sint32)best)
            {
                best = radius;
                hit = node;
                side = value;
            }
        }
    }
    scene = r_u32(0x8008A4E8);
    for (node = r_u32(scene + 52); node != 0; node = r_u32(node))
    {
        if ((r_u8(node + 13) & 0x81) != 0)
            continue;
        next = r_u32(node + 68);
        if (next == 0)
            continue;
        candidate_owner = r_u32(next + 36);
        if (candidate_owner == owner || (r_u32(candidate_owner + 16) & 0x11) != 0)
            continue;
        radius = r_u16(node + 66) << 5;
        x = r_u32(position);
        a = r_u32(node + 36);
        radius += 20480;
        value = x - a + radius;
        w_u32(second_extent, radius);
        if (value > (radius << 1))
            continue;
        value = r_u32(position + 8);
        value = value - r_u32(node + 44) + radius;
        if (value > (radius << 1))
            continue;
        value = r_u32(position + 4);
        value = value - r_u32(node + 40) + 204800;
        if (value > 409600)
            continue;
        w_u32(velocity + 8, 0);
        w_u32(velocity, 0);
        w_u32(velocity + 4, (uint32)-204800);
        if (p2_func_80038990(position, node, velocity, second_extent, 0) != 0)
        {
            radius = r_u32(second_extent);
            if ((sint32)radius < (sint32)best)
            {
                side = 0xFFFFFFFF;
                best = radius;
                hit = node;
                if (direction != 0)
                    next = r_u32(hit + 68);
                else
                    next = r_u32(hit + 72);
                owner = r_u32(next + 36);
            }
        }
    }
    if (hit != 0)
    {
        frame = FindPathHeightFrame(owner, position, direction, -40960);
        if ((sint16)frame >= 0)
            frame = (uint32)ProjectPathFrame(owner, (sint16)frame, position, floor, direction, 4096);
    }
    else if ((r_u32(object_offset + 28) & 0x2000) != 0 || (r_u32(owner + 16) & 0x11) != 0)
        goto hide;
    if (hit != 0)
    {
        kind = r_s8(hit + 15);
        if (kind != 1 && (uint8)(kind - 24) >= 2)
        {
            w_u32(model + 32, 0 - best * 50);
            --side;
            target = hit + 16 + (uint32)((sint32)side >> 1) * 6;
            value = (uint32)r_s16(target);
            w_u32(normal, (side & 1) != 0 ? value : 0 - value);
            value = (uint32)r_s16(target + 2);
            w_u32(normal + 4, (side & 1) != 0 ? value : 0 - value);
            value = (uint32)r_s16(target + 4);
            w_u32(normal + 8, (side & 1) != 0 ? value : 0 - value);
            goto orient;
        }
    }
    // Keep the original floor read when its filling query was skipped
    value = r_u32(floor + 4);
    w_u32(model + 32, value - r_u32(position + 4));
    goto orient;
no_path:
    owner = 0;
    w_u16(model + 44, 0);
orient:
    if ((sint32)side >= 0)
    {
        value = (uint32)AsinAngle(r_s32(normal + 4), 4096);
        w_u16(model + 44, (1024 - value) & 0xFFF);
    }
    else if (owner != 0)
    {
        if ((sint16)frame > 0)
            --frame;
        SampleWorldPath(owner, (sint16)frame, velocity, direction);
        do
        {
            ++frame;
            SampleWorldPath(owner, (sint16)frame, floor, direction);
            x = r_u32(floor);
            a = r_u32(velocity);
            y = r_u32(floor + 4);
            b = r_u32(velocity + 4);
            z = r_u32(floor + 8);
            c = r_u32(velocity + 8);
            x -= a;
            y -= b;
            z -= c;
            w_u32(normal, x);
            w_u32(normal + 4, y);
            w_u32(normal + 8, z);
            x = (uint32)((sint32)x >> 6);
            y = (uint32)((sint32)y >> 6);
            z = (uint32)((sint32)z >> 6);
            squared = x * x + y * y + z * z;
            if ((sint32)squared < 0)
                magnitude = p2_sdk_8007e794(squared >> 2) << 1;
            else
                magnitude = p2_sdk_8007e794(squared);
        } while ((sint32)magnitude < 1024 && (sint16)frame < r_s16(owner + 26) - 1);
        value = (uint32)AsinAngle(r_s32(normal + 4), (sint32)magnitude);
        w_u16(model + 44, value & 0xFFF);
    }
    w_u16(model + 46, r_u16(object_offset + 150));
    if (direction == 0)
    {
        value = r_u16(model + 44);
        flags = r_u16(model + 46);
        w_u16(model + 44, (0 - value) & 0xFFF);
        w_u16(model + 46, (flags + 2048) & 0xFFF);
    }
    result = r_u8(model + 13) & 0xFE;
    goto finish_model;
hide:
    result = r_u8(model + 13) | 1;
finish_model:
    w_u8(model + 13, result);
    p2_guest_stack_pointer = saved_stack;
    return result;
}

uint32 p2_func_8004e118(uint32 object_offset, uint32 support_offset)
{
    FUNCTION_MARKER(0x8004E118, "SLUS_005.78");
    uint32 saved_stack, position, floor, input, query_frame, transform, transpose, backup_frame, visited;
    uint32 flags, direction, start, current, backup, previous, next, owner, parent, matrix;
    uint32 x, y, z, a, b, value, frame, steps, index, height_delta, need_transition, result;
    sint32 travel_direction;
    saved_stack = p2_guest_stack_pointer;
    p2_guest_stack_pointer -= 176;
    position = p2_guest_stack_pointer;
    floor = position + 16;
    input = position + 32;
    query_frame = position + 48;
    transform = position + 64;
    transpose = position + 80;
    backup_frame = position + 112;
    visited = position + 120;
    flags = r_u32(object_offset + 228);
    steps = 0;
    if ((flags & 0x400000) != 0)
    {
        if (support_offset != 0)
        {
            parent = r_u32(0x800ABE68);
            a = r_u32(support_offset + 68);
            value = r_u32(parent + 68);
            b = r_u32(support_offset + 72);
            if (a == value || b == value)
                goto failed;
        }
    }
    else if ((flags & 0x20) != 0 && support_offset != 0 && r_s8(0x8008A210) == 1)
    {
        a = r_u32(support_offset + 68);
        value = r_u32(0x800ABE60);
        b = r_u32(support_offset + 72);
        if (a == value || b == value)
            goto failed;
    }
    flags = r_u32(object_offset + 28);
    direction = flags & 0x10;
    if (support_offset != 0)
    {
        flags &= 0x2000;
        if (direction != 0)
            start = r_u32(support_offset + 68);
        else
        {
            start = r_u32(support_offset + 72);
            flags = r_u32(object_offset + 28) & 0x2000;
        }
    }
    else
    {
        z = r_u32(object_offset + 100);
        x = r_u32(object_offset + 92);
        start = r_u32(0x800AC018);
        value = (uint32)(sint32)(sint16)Atan2Angle((sint32)z, (sint32)x);
        w_u32(query_frame, value);
        if ((sint32)value == 4096)
            travel_direction = 0;
        else
        {
            a = (uint32)r_s16(object_offset + 150);
            travel_direction = -1;
            if (((value - a - 1024) & 0xFFF) < 2048)
                travel_direction = 1;
        }
        flags = r_u32(object_offset + 28) & 0x2000;
    }
    if (flags == 0 && (parent = r_u32(object_offset + 132)) != 0 && r_u32(parent + 40) != 0)
    {
        x = r_u32(object_offset + 16);
        y = r_u32(object_offset + 20);
        z = r_u32(object_offset + 24);
        w_u32(input, x);
        w_u32(input + 4, y);
        w_u32(input + 8, z);
        current = start;
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
        p2_sdk_8007ec64(input, position);
        backup = 0;
        PopMatrix();
        for (index = 0; index < 3; ++index)
        {
            matrix = r_u32(r_u32(object_offset + 132) + 40);
            value = r_u32(position + index * 4);
            w_u32(position + index * 4, value + r_u32(matrix + 20 + index * 4));
        }
    }
    else
    {
        x = r_u32(object_offset + 16);
        y = r_u32(object_offset + 20);
        z = r_u32(object_offset + 24);
        w_u32(position, x);
        w_u32(position + 4, y);
        w_u32(position + 8, z);
        current = start;
        backup = 0;
    }
    previous = 0;
    w_u32(visited, 0);
    for (index = 6; index != 0; --index)
        w_u32(visited + index * 8, 0);
walk:
    value = r_u32(0x8008A190);
    ++steps;
    if ((sint32)steps >= (sint32)value)
        goto failed;
    for (index = 0; index < 7; ++index)
    {
        if (r_u32(visited + index * 8) == current)
            goto failed;
    }
    for (index = 0; index < 7; ++index)
    {
        if (r_u32(visited + index * 8) == 0 && current != start)
        {
            w_u32(visited + index * 8, current);
            break;
        }
    }
    frame = FindPathHeightFrame(r_u32(current + 36), position, direction, -40960);
    w_u32(query_frame, frame);
    if ((sint32)frame >= 0)
        frame = (uint32)ProjectPathFrame(r_u32(current + 36), (sint32)frame, position, floor, direction, 4096);
    owner = r_u32(current + 36);
    a = r_u32(floor + 4);
    b = r_u32(position + 4);
    value = (uint32)(r_s16(owner + 26) - 1);
    height_delta = a - b;
    need_transition = (sint32)(frame - 1) >= (sint32)value;
    if (need_transition == 0 && support_offset == 0)
    {
        flags = r_u32(owner + 16);
        if ((flags & 0x11) != 0 || (r_u32(object_offset + 28) & 0x2000) != 0)
        {
            if ((sint32)b < (sint32)a)
                need_transition = 1;
        }
        if (need_transition == 0 && current != start && (r_u32(r_u32(current + 36) + 16) & 0x11) == 0 && (r_u32(r_u32(previous + 36) + 16) & 0x11) != 0 && (sint32)height_delta >= 1366)
            need_transition = 1;
    }
    if (need_transition != 0)
    {
        if (support_offset != 0)
            goto failed;
        if ((r_u32(object_offset + 28) & 0x2000) != 0)
        {
            if ((sint32)frame < 0)
                goto failed;
            if ((r_u32(r_u32(current + 36) + 16) & 0x11) == 0)
            {
                a = r_u32(floor + 4);
                b = r_u32(position + 4);
                if ((sint32)b < (sint32)a)
                    goto failed;
            }
        }
        if ((sint32)frame > 0)
        {
            owner = r_u32(current + 36);
            value = (uint32)r_s16(owner + 26);
            if ((sint32)frame < (sint32)value && (r_u32(owner + 16) & 0x10) != 0)
            {
                next = r_u32(current + 28);
                if (next == 0)
                    goto accept;
                if ((r_u16(current + 14) & (direction | 8)) == 0 && (r_u16(next + 14) & 8) != 0)
                    next = r_u32(next + 16);
                if (next == 0 || next == previous || next == start)
                    goto accept;
                goto advance;
            }
        }
        if ((sint32)frame >= 0)
        {
            value = r_u16(r_u32(current + 36) + 26);
            travel_direction = -1;
            if (((sint32)(sint16)value >> 1) < (sint32)frame)
                travel_direction = 1;
        }
        w_u32(query_frame, (uint32)travel_direction);
        next = p2_func_8003ac94(current, query_frame, direction);
        if (next == 0 || next == previous || next == start)
        {
            current = backup;
            if (backup == 0)
                goto failed;
            frame = r_u32(backup_frame);
            ProjectPathFrame(r_u32(current + 36), (sint32)frame, position, floor, direction, 4096);
            goto accept;
        }
    advance:
        if ((sint32)frame >= 0 && backup == 0)
        {
            backup = current;
            w_u32(backup_frame, frame);
        }
        frame = 0xFFFFFFFF;
        previous = current;
        current = next;
    }
    if ((sint32)frame < 0)
        goto walk;
accept:
    owner = r_u32(current + 36);
    w_u16(object_offset + 136, frame);
    w_u32(object_offset + 132, owner);
    owner = r_u32(current + 36);
    a = r_u32(floor + 4);
    b = r_u32(position + 4);
    flags = r_u32(owner + 16);
    height_delta = a - b;
    if ((flags & 0x11) != 0 || (((r_u32(object_offset + 28) & 0x400) != 0 || current != start) && (sint32)height_delta < -681))
    {
        owner = r_u32(current + 36);
        if ((r_u32(owner + 16) & 0x10000) == 0)
            goto leave_ground;
        parent = r_u32(0x8008A114);
        parent = r_u32(parent + 136);
        parent = r_u32(parent + 36);
        if ((r_u32(parent + 16) & 0x10000) != 0)
            goto leave_ground;
    }
    w_u32(object_offset + 28, r_u32(object_offset + 28) | 0x4000);
    goto after_ground;
leave_ground:
    w_u32(object_offset + 28, (r_u32(object_offset + 28) & 0xFFFFBFFF) | 0x400);
    w_u32(floor + 4, r_u32(position + 4));
after_ground:
    flags = r_u32(object_offset + 228);
    if ((flags & 2) != 0)
    {
        value = r_u32(object_offset + 28);
        if ((value & 0x4000) != 0 && (flags & 4) == 0)
            w_u32(object_offset + 28, value & 0xFFFFBFFF);
    }
    if ((r_u32(object_offset + 228) & 0x20) != 0)
    {
        p2_func_800586e8(object_offset);
        p2_func_80058728(object_offset);
    }
    w_u32(object_offset + 28, r_u32(object_offset + 28) & 0xFFFFD7FF);
    if ((r_u32(r_u32(current + 36) + 16) & 0x10000) != 0)
    {
        x = r_u32(floor);
        z = r_u32(floor + 8);
        w_u32(position, x);
        w_u32(position + 8, z);
    }
    flags = r_u32(object_offset + 28);
    if ((flags & 0x4400) == 0x400 || (r_s8(object_offset + 126) == 1 && support_offset == 0 && (flags & 0x4000) == 0))
    {
        x = r_u32(position);
        z = r_u32(position + 8);
        w_u32(floor, x);
        w_u32(floor + 8, z);
    }
    parent = r_u32(object_offset + 132);
    if (parent != 0 && (matrix = r_u32(parent + 40)) != 0)
    {
        value = r_u32(floor);
        w_u32(transform, value - r_u32(matrix + 20));
        for (index = 1; index < 3; ++index)
        {
            matrix = r_u32(r_u32(object_offset + 132) + 40);
            value = r_u32(floor + index * 4);
            w_u32(transform + index * 4, value - r_u32(matrix + 20 + index * 4));
        }
        for (index = 0; index < 9; ++index)
        {
            matrix = r_u32(r_u32(object_offset + 132) + 40);
            w_u16(transpose + index * 2, r_u16(matrix + (index % 3) * 6 + (index / 3) * 2));
        }
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
        p2_sdk_8007ec64(transform, object_offset + 16);
        PopMatrix();
    }
    else
    {
        x = r_u32(floor);
        y = r_u32(floor + 4);
        z = r_u32(floor + 8);
        w_u32(object_offset + 16, x);
        w_u32(object_offset + 20, y);
        w_u32(object_offset + 24, z);
    }
    result = current;
    goto finish_support;
failed:
    result = 0;
finish_support:
    p2_guest_stack_pointer = saved_stack;
    return result;
}

uint32 p2_func_80057650(uint32 object_offset)
{
    FUNCTION_MARKER(0x80057650, "SLUS_005.78");
    uint32 saved_stack, world, anchor, scene_cache, floor, sample, local, target;
    uint32 input, anchor_input, inverse_input, transpose, forward_input, first_floor, backup_floor, final_transpose;
    uint32 result, flags, path, scene, owner, matrix, pointer, value, direction, frame, first_frame;
    uint32 x, y, z, index;
    sint32 old_frame;
    saved_stack = p2_guest_stack_pointer;
    p2_guest_stack_pointer -= 272;
    world = p2_guest_stack_pointer;
    anchor = world + 16;
    scene_cache = world + 32;
    floor = world + 48;
    sample = world + 64;
    local = world + 80;
    target = world + 96;
    input = world + 112;
    anchor_input = world + 128;
    inverse_input = world + 144;
    transpose = world + 160;
    forward_input = world + 192;
    first_floor = world + 208;
    backup_floor = world + 224;
    final_transpose = world + 240;
    result = r_u32(object_offset + 228) & 0x40000;
    if (result != 0)
        goto finish_transfer;
    result = 0xFFFFFBFF;
    if (r_s8(0x800ABE64) >= 4)
        goto finish_transfer;
    flags = r_u32(object_offset + 28);
    w_u32(object_offset + 100, 0);
    w_u32(object_offset + 96, 0);
    w_u32(object_offset + 92, 0);
    w_u32(object_offset + 28, flags & 0xFFFFFBFF);
    pointer = r_u32(0x800ABE68);
    scene = r_u32(0x8008A114);
    path = r_u32(pointer + 68);
    value = r_u32(scene + 136);
    if (path == value)
    {
        result = r_u32(object_offset + 28) & 0x2000;
        if (result == 0)
            goto finish_transfer;
    }
    if ((r_u32(object_offset + 28) & 0x2000) == 0 && (owner = r_u32(object_offset + 132)) != 0 && r_u32(owner + 40) != 0)
    {
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
        p2_sdk_8007ec64(input, world);
        PopMatrix();
        for (index = 0; index < 3; ++index)
        {
            matrix = r_u32(r_u32(object_offset + 132) + 40);
            value = r_u32(world + index * 4);
            w_u32(world + index * 4, value + r_u32(matrix + 20 + index * 4));
        }
    }
    else
    {
        x = r_u32(object_offset + 16);
        y = r_u32(object_offset + 20);
        z = r_u32(object_offset + 24);
        w_u32(world, x);
        w_u32(world + 4, y);
        w_u32(world + 8, z);
    }
    owner = r_u32(path + 36);
    if (owner != 0 && r_u32(owner + 40) != 0)
    {
        pointer = r_u32(0x800ABE68);
        x = r_u32(pointer + 88);
        y = r_u32(pointer + 92);
        z = r_u32(pointer + 96);
        w_u32(anchor_input, x);
        w_u32(anchor_input + 4, y);
        w_u32(anchor_input + 8, z);
        PushMatrix();
        matrix = r_u32(r_u32(path + 36) + 40);
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
        p2_sdk_8007ec64(anchor_input, anchor);
        PopMatrix();
        for (index = 0; index < 3; ++index)
        {
            matrix = r_u32(r_u32(path + 36) + 40);
            value = r_u32(anchor + index * 4);
            w_u32(anchor + index * 4, value + r_u32(matrix + 20 + index * 4));
        }
    }
    else
    {
        pointer = r_u32(0x800ABE68);
        x = r_u32(pointer + 88);
        y = r_u32(pointer + 92);
        z = r_u32(pointer + 96);
        w_u32(anchor, x);
        w_u32(anchor + 4, y);
        w_u32(anchor + 8, z);
    }
    scene = r_u32(0x8008A114);
    x = r_u32(scene + 16);
    y = r_u32(scene + 20);
    z = r_u32(scene + 24);
    w_u32(0x800ABEC0, x);
    w_u32(0x800ABEC4, y);
    w_u32(0x800ABEC8, z);
    x = r_u32(0x800ABEC0);
    y = r_u32(0x800ABEC4);
    z = r_u32(0x800ABEC8);
    w_u32(scene_cache, x);
    w_u32(scene_cache + 4, y);
    w_u32(scene_cache + 8, z);
    x = r_u32(world);
    y = r_u32(world + 4);
    z = r_u32(world + 8);
    w_u32(0x800ABE90, x);
    w_u32(0x800ABE94, y);
    w_u32(0x800ABE98, z);
    direction = (r_u32(object_offset + 228) & 0x400) == 0 ? 64 : 0;
    SamplePathPosition(path, 0, sample, direction);
    owner = r_u32(path + 36);
    if (owner != 0 && (matrix = r_u32(owner + 40)) != 0)
    {
        value = r_u32(world);
        w_u32(inverse_input, value - r_u32(matrix + 20));
        for (index = 1; index < 3; ++index)
        {
            matrix = r_u32(r_u32(path + 36) + 40);
            value = r_u32(world + index * 4);
            w_u32(inverse_input + index * 4, value - r_u32(matrix + 20 + index * 4));
        }
        for (index = 0; index < 9; ++index)
        {
            matrix = r_u32(r_u32(path + 36) + 40);
            w_u16(transpose + index * 2, r_u16(matrix + (index % 3) * 6 + (index / 3) * 2));
        }
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
        p2_sdk_8007ec64(inverse_input, local);
        PopMatrix();
    }
    else
    {
        x = r_u32(world);
        y = r_u32(world + 4);
        z = r_u32(world + 8);
        w_u32(local, x);
        w_u32(local + 4, y);
        w_u32(local + 8, z);
    }
    flags = r_u32(object_offset + 228);
    value = r_u32(local);
    if ((flags & 0x400) == 0)
        value += r_u32(0x8008A1C4);
    else
        value -= r_u32(0x8008A1C4);
    w_u32(local, value);
    owner = r_u32(path + 36);
    if (owner != 0 && r_u32(owner + 40) != 0)
    {
        x = r_u32(local);
        y = r_u32(local + 4);
        z = r_u32(local + 8);
        w_u32(forward_input, x);
        w_u32(forward_input + 4, y);
        w_u32(forward_input + 8, z);
        PushMatrix();
        matrix = r_u32(r_u32(path + 36) + 40);
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
        p2_sdk_8007ec64(forward_input, target);
        PopMatrix();
        for (index = 0; index < 3; ++index)
        {
            matrix = r_u32(r_u32(path + 36) + 40);
            value = r_u32(target + index * 4);
            w_u32(target + index * 4, value + r_u32(matrix + 20 + index * 4));
        }
    }
    else
    {
        x = r_u32(local);
        y = r_u32(local + 4);
        z = r_u32(local + 8);
        w_u32(target, x);
        w_u32(target + 4, y);
        w_u32(target + 8, z);
    }
    value = r_u32(anchor + 4);
    scene = r_u32(0x8008A114);
    w_u32(target + 4, value + 40960);
    x = r_u32(scene + 16);
    y = r_u32(scene + 20);
    z = r_u32(scene + 24);
    w_u32(0x800ABECC, x);
    w_u32(0x800ABED0, y);
    w_u32(0x800ABED4, z);
    x = r_u32(0x800ABECC);
    y = r_u32(0x800ABED0);
    z = r_u32(0x800ABED4);
    w_u32(0x800ABEE4, x);
    w_u32(0x800ABEE8, y);
    w_u32(0x800ABEEC, z);
    x = r_u32(0x800ABEE4);
    y = r_u32(0x800ABEE8);
    z = r_u32(0x800ABEEC);
    w_u32(0x800ABED8, x);
    w_u32(0x800ABEDC, y);
    w_u32(0x800ABEE0, z);
    value = r_u32(0x800ABED0);
    if ((sint32)character_controller.flags < 0)
        value -= 32768;
    else
        value -= 14336;
    w_u32(0x800ABED0, value);
    x = r_u32(0x800ABECC);
    y = r_u32(0x800ABED0);
    z = r_u32(0x800ABED4);
    w_u32(0x800ABEC0, x);
    w_u32(0x800ABEC4, y);
    w_u32(0x800ABEC8, z);
    frame = FindPathHeightFrame(r_u32(path + 36), target, direction, -40960);
    if ((sint32)frame >= 0)
        frame = (uint32)ProjectPathFrame(r_u32(path + 36), (sint32)frame, target, floor, direction, 8192);
    x = r_u32(floor);
    y = r_u32(floor + 4);
    z = r_u32(floor + 8);
    w_u32(first_floor, x);
    w_u32(first_floor + 4, y);
    w_u32(first_floor + 8, z);
    first_frame = frame;
    SamplePathPosition(path, 0, floor, direction);
    x = r_u32(floor);
    y = r_u32(floor + 4);
    z = r_u32(floor + 8);
    w_u32(backup_floor, x);
    w_u32(backup_floor + 4, y);
    w_u32(backup_floor + 8, z);
    x = r_u32(floor);
    y = r_u32(floor + 4);
    z = r_u32(floor + 8);
    w_u32(target, x);
    w_u32(target + 4, y);
    w_u32(target + 8, z);
    frame = (uint32)ProjectPathFrame(r_u32(path + 36), 0, target, floor, direction, 8192);
    if ((sint32)frame < 0)
    {
        frame = 0;
        x = r_u32(backup_floor);
        y = r_u32(backup_floor + 4);
        z = r_u32(backup_floor + 8);
        w_u32(floor, x);
        w_u32(floor + 4, y);
        w_u32(floor + 8, z);
    }
    value = r_u32(floor);
    value = value - r_u32(first_floor) + 24576;
    if (value > 49152)
        goto restore_first;
    value = r_u32(floor + 8);
    value = value - r_u32(first_floor + 8) + 24576;
    if (value > 49152)
    {
    restore_first:
        x = r_u32(first_floor);
        y = r_u32(first_floor + 4);
        z = r_u32(first_floor + 8);
        w_u32(floor, x);
        w_u32(floor + 4, y);
        w_u32(floor + 8, z);
        frame = first_frame;
    }
    owner = r_u32(path + 36);
    w_u16(object_offset + 136, frame);
    w_u32(object_offset + 132, owner);
    x = r_u32(floor);
    y = r_u32(floor + 4);
    z = r_u32(floor + 8);
    w_u32(0x800ABE9C, x);
    w_u32(0x800ABEA0, y);
    w_u32(0x800ABEA4, z);
    owner = r_u32(object_offset + 132);
    if (owner != 0 && (matrix = r_u32(owner + 40)) != 0)
    {
        value = r_u32(floor);
        w_u32(first_floor, value - r_u32(matrix + 20));
        for (index = 1; index < 3; ++index)
        {
            matrix = r_u32(r_u32(object_offset + 132) + 40);
            value = r_u32(floor + index * 4);
            w_u32(first_floor + index * 4, value - r_u32(matrix + 20 + index * 4));
        }
        for (index = 0; index < 9; ++index)
        {
            matrix = r_u32(r_u32(object_offset + 132) + 40);
            w_u16(final_transpose + index * 2, r_u16(matrix + (index % 3) * 6 + (index / 3) * 2));
        }
        PushMatrix();
        x = r_u32(final_transpose);
        y = r_u32(final_transpose + 4);
        xport_gte_write_control(0, x);
        xport_gte_write_control(1, y);
        x = r_u32(final_transpose + 8);
        y = r_u32(final_transpose + 12);
        z = r_u16(final_transpose + 16);
        xport_gte_write_control(2, x);
        xport_gte_write_control(3, y);
        xport_gte_write_control(4, z);
        p2_sdk_8007ec64(first_floor, object_offset + 16);
        PopMatrix();
    }
    else
    {
        x = r_u32(floor);
        y = r_u32(floor + 4);
        z = r_u32(floor + 8);
        w_u32(object_offset + 16, x);
        w_u32(object_offset + 20, y);
        w_u32(object_offset + 24, z);
    }
    scene = r_u32(0x8008A114);
    x = r_u32(world);
    y = r_u32(world + 4);
    z = r_u32(world + 8);
    w_u32(scene + 16, x);
    w_u32(scene + 20, y);
    w_u32(scene + 24, z);
    scene = r_u32(0x8008A114);
    flags = character_controller.flags;
    value = r_u32(scene + 20);
    if ((sint32)flags < 0)
        value += 32768;
    else
        value += 14336;
    w_u32(scene + 20, value);
    old_frame = r_s16(object_offset + 136);
    scene = r_u32(0x8008A114);
    pointer = r_u32(scene + 136);
    flags = r_u32(object_offset + 28);
    owner = r_u32(pointer + 36);
    value = FindPathHeightFrame(owner, scene + 16, flags & 0x10, -40960);
    w_u16(object_offset + 136, value);
    p2_func_80046510(object_offset, path, old_frame);
    w_u32(0x800ABF00, path);
    result = r_u32(object_offset + 28) & 0xFFFFDFFF;
    w_u32(object_offset + 28, result);
finish_transfer:
    p2_guest_stack_pointer = saved_stack;
    return result;
}



sint32 p2_func_80037f78(uint32 object_offset, uint32 position_offset, uint32 target_offset)
{
    FUNCTION_MARKER(0x80037F78, "SLUS_005.78");
    uint32 saved_stack = p2_guest_stack_pointer;
    uint32 sample, parent, flags, x, z, first_angle, second_angle;
    sint32 count, frame, next_frame, result;
    p2_guest_stack_pointer -= 16;
    sample = p2_guest_stack_pointer;
    parent = r_u32(object_offset + 132);
    if (r_u32(parent + 32) == 0)
        goto no_direction;
    x = r_u32(position_offset) - r_u32(target_offset) + 4096;
    if (x < 0x2001)
    {
        z = r_u32(position_offset + 8) - r_u32(target_offset + 8) + 4096;
        if (z < 0x2001)
            goto no_direction;
    }
    parent = r_u32(object_offset + 132);
    count = r_s16(parent + 26);
    frame = r_s16(object_offset + 136);
    next_frame = frame + count / 8;
    if (next_frame == frame || next_frame > count - 1)
        next_frame = count - 1;
    flags = r_u32(object_offset + 28);
    parent = r_u32(object_offset + 132);
    SampleWorldPath(parent, next_frame, sample, flags & 0x10);
    z = r_u32(sample + 8);
    x = r_u32(position_offset + 8);
    parent = r_u32(sample);
    flags = r_u32(position_offset);
    first_angle = Atan2Angle((sint32)(z - x), (sint32)(parent - flags));
    z = r_u32(target_offset + 8);
    x = r_u32(position_offset + 8);
    parent = r_u32(target_offset);
    flags = r_u32(position_offset);
    second_angle = Atan2Angle((sint32)(z - x), (sint32)(parent - flags));
    result = ((first_angle - second_angle + 1024) & 0xFFF) < 0x801 ? 1 : -1;
    p2_guest_stack_pointer = saved_stack;
    return result;
no_direction:
    p2_guest_stack_pointer = saved_stack;
    return 0;
}

uint32 p2_func_8003557c(uint32 object_offset, uint32 position_offset, uint32 target_offset, uint32 distance_offset, uint32 incoming_action_offset)
{
    FUNCTION_MARKER(0x8003557C, "SLUS_005.78");
    uint32 saved_stack = p2_guest_stack_pointer;
    uint32 local, definition, actions, action = incoming_action_offset, owner;
    uint32 value, flags, mode, state, animation, x, y, z, parent, matrix;
    uint32 scan, candidate, counter_offset, trigger, threshold, blocked, chosen;
    uint32 kind, speed, turn, delay, spawned, attack, count, iteration, input, cached_input;
    uint32 length, magnitude, angle, sine, cosine, result;
    sint32 index, limit, direction, frame, next_frame, parameter, signed_value, sign_x, sign_z;
    definition = r_u32(r_u32(object_offset + 132) + 44);
    result = 0xFFFFFFFE;
    if (r_s8(definition + 124) == -2)
        return result;
    if ((r_u8(definition + 38) & 2) != 0)
        return p2_func_800378a0(object_offset);
    actions = r_u32(definition + 72);
    owner = GetCharacterModelObject();
    if (r_s8(definition + 181) != 0)
        return p2_func_800378a0(object_offset);
    p2_guest_stack_pointer -= 192;
    local = p2_guest_stack_pointer;
    w_u32(local + 160, owner);
    chosen = 0;
restart_action:
    w_u16(local + 8, 0);
restart_mode:
    for (scan = 0; scan < 6; ++scan)
        w_u8(local + scan, 0);
    mode = r_u8(definition + 166);
    index = -1;
    limit = 6;
    blocked = 0;
    w_u8(local + 136, 0);
    w_u8(local + 144, 0);
    if (mode == 0)
    {
        signed_value = r_s8(definition + 124);
        if (signed_value != -1)
        {
            action = actions + (uint32)signed_value * 80 + 140;
            value = r_u8(action + 8);
            if (value == 1)
                limit = (uint16)signed_value;
            else
            {
                if (value == 2)
                {
                    animation = r_u32(actions + 72 + (uint32)r_s16(action + 16) * 4);
                    if (r_u32(object_offset + 104) == animation && r_u8(action + 10) >= r_s16(object_offset + 130))
                        goto retain_action_priority;
                    value = r_u8(action + 8);
                }
                if (value == 3 && r_u32(object_offset + 104) == r_u32(actions + 72 + (uint32)r_s16(action + 16) * 4) && r_s16(object_offset + 130) >= r_u8(action + 10))
                {
                retain_action_priority:
                    limit = (uint16)r_s8(definition + 124);
                }
                else
                    blocked = 1;
            }
        }
        scan = 0;
        candidate = actions + 140;
        counter_offset = definition + 140;
        while (r_u32(candidate + 4) != 0)
        {
            if ((sint8)index != -1)
                break;
            threshold = r_u32(candidate + 4);
            if (r_s32(distance_offset) < (sint32)threshold || (sint32)threshold < 0)
            {
                trigger = r_u8(candidate + 9);
                value = 0;
                w_u8(local + 136, 1);
                if (trigger == 7)
                    w_u8(local + scan, 1);
                if ((sint32)scan < limit && blocked == 0)
                {
                    switch (r_u8(candidate + 9))
                    {
                        case 0:
                            value = 1;
                            break;
                        case 1:
                            if ((r_u32(r_u32(local + 160) + 28) & 0x400) != 0)
                                value = 1;
                            break;
                        case 2:
                            flags = gameplay_input.buttons;
                            if ((flags & (*control_bindings.active_scheme)[0]) != 0)
                                value = 1;
                            break;
                        case 3:
                            if ((r_u32(definition + 44) & 8) != 0)
                            {
                                flags = r_u16(definition + 120);
                                if ((flags & 0x10) == 0)
                                {
                                    value = 1;
                                    w_u16(definition + 120, flags | 0x10);
                                }
                            }
                            break;
                        case 4:
                            flags = r_u32(definition + 128);
                            w_u8(local + 144, 1);
                            if (flags != 0xFFFFFFFF)
                            {
                                value = 1;
                                w_u32(definition + 128, 0xFFFFFFFF);
                            }
                            break;
                        case 5:
                            if (r_s16(candidate + 12) >= r_s16(object_offset + 128))
                                value = 1;
                            break;
                        case 6:
                            if (r_u32(definition + 136) >= (uint32)r_s16(candidate + 12))
                            {
                                value = 1;
                                w_u32(definition + 136, 0);
                            }
                            break;
                        case 7:
                            if (r_u32(counter_offset) >= (uint32)r_s16(candidate + 12))
                            {
                                value = 1;
                                w_u32(counter_offset, 0);
                            }
                            break;
                    }
                    if (value != 0)
                    {
                        index = (sint32)scan;
                        action = candidate;
                        w_u8(definition + 124, scan);
                        w_u8(definition + 125, 0);
                    }
                }
            }
            candidate += 80;
            counter_offset += 4;
            scan += 1;
        }
        goto selected_action;
    }
    direction = p2_func_80037f78(object_offset, position_offset, target_offset);
    w_u32(local + 152, (uint32)direction);
    if (r_u8(definition + 166) == 2 && r_s8(definition + 124) < 0)
        p2_func_800378a0(object_offset);
    mode = r_u8(definition + 166);
    if (mode == 1)
    {
        p2_func_800378a0(object_offset);
        if (r_s16(actions + 658) != 0)
        {
            w_u8(definition + 166, 2);
            w_u8(definition + 125, 0);
            w_u8(definition + 124, r_u8(actions + 658) - 1);
        }
        else
        {
            animation = r_u32(r_u32(definition + 72) + 72);
            w_u16(object_offset + 130, 0xFFFF);
            w_u32(object_offset + 104, animation);
            if (r_s32(local + 152) != -1)
                w_u8(definition + 166, 4);
            else
            {
                w_u8(definition + 166, 3);
                parent = r_u32(object_offset + 132);
                flags = r_u32(object_offset + 28);
                x = r_u16(object_offset + 136);
                y = r_u16(parent + 26);
                w_u32(object_offset + 28, (flags & 0xFFFFF7FF) ^ 0x10);
                w_u16(object_offset + 136, y - x);
                parameter = r_s16(actions + 652);
                w_u16(local + 8, 1);
                if (parameter != 0)
                    w_u8(definition + 132, 0);
                else
                {
                    parameter = r_s16(actions + 68);
                    if (parameter == 0)
                        xport_mips_break(7);
                    delay = (uint32)(2048 / parameter) - 2;
                    if ((delay & 0x80) != 0)
                        delay = 0;
                    w_u8(definition + 132, delay);
                }
            }
        }
    }
    else if (mode == 3)
    {
        value = (r_u8(definition + 132) - 1) & 0xFF;
        w_u8(definition + 132, value);
        if (value == 0)
            w_u8(definition + 166, 4);
    }
    mode = r_u8(definition + 166);
    if (mode == 4)
    {
        threshold = (uint32)r_s16(actions + 660) << 5;
        if (r_s32(local + 152) != -1 && r_s32(distance_offset) < (sint32)threshold)
        {
            w_u16(definition + 120, r_u16(definition + 120) | 0x1100);
            w_u32(object_offset + 28, (r_u32(object_offset + 28) & 0xFFFFF7BF) | 0x40);
            w_u32(definition + 168, 0);
            goto selected_action;
        }
        p2_func_800378a0(object_offset);
        w_u16(local + 8, 0);
        goto restart_mode;
    }
    if (mode == 2)
        action = actions + (uint32)r_s8(definition + 124) * 80 + 140;
selected_action:
    index = r_s8(definition + 124);
    w_u16(definition + 122, 0);
    if (index == -1)
        goto update_counters;
    direction = p2_func_80037f78(object_offset, position_offset, target_offset);
    w_u32(local + 152, (uint32)direction);
    if (r_u8(definition + 125) == 0)
    {
        flags = r_u16(definition + 120);
        w_u8(definition + 125, 1);
        w_u16(definition + 120, flags & 0xFFFE);
        parameter = r_s16(action + 30);
        turn = ((parameter == 3 || parameter == 6 || parameter == 8) && r_s32(local + 152) == -1);
        if (turn == 0)
        {
            value = r_u16(action + 30);
            turn = ((value - 4 < 2 || (sint16)value == 7) && r_s32(local + 152) == 1);
        }
        if (turn != 0)
        {
            parent = r_u32(object_offset + 132);
            flags = r_u32(object_offset + 28);
            x = r_u16(object_offset + 136);
            y = r_u16(parent + 26);
            w_u32(object_offset + 28, (flags & 0xFFFFF7FF) ^ 0x10);
            w_u16(object_offset + 136, y - x);
            parameter = r_s16(actions + 652);
            w_u16(local + 8, 1);
            if (parameter == 0)
            {
                parameter = r_s16(actions + 68);
                if (parameter == 0)
                    xport_mips_break(7);
                delay = (uint32)(2048 / parameter) - 2;
                if ((delay & 0x80) != 0)
                    delay = 0;
                w_u8(definition + 132, delay);
            }
            else
                w_u8(definition + 132, 0);
        }
        else
            w_u8(definition + 132, 0);
        if (r_s16(action + 44) == -1)
            w_u8(definition + 133, 0);
        else
        {
            flags = r_u16(definition + 120);
            w_u8(definition + 133, 1);
            w_u8(definition + 135, 0);
            w_u16(definition + 120, flags & 0xFFF3);
        }
    }
    state = r_u8(definition + 125);
    if (state == 1)
    {
        value = r_u8(definition + 132);
        if (value != 0)
        {
            w_u8(definition + 132, value - 1);
            w_u16(local + 8, 1);
            value = r_u8(definition + 132);
        }
        if (value == 0)
        {
            animation = r_u32(actions + 72 + (uint32)r_s16(action + 16) * 4);
            flags = r_u32(object_offset + 28);
            w_u16(object_offset + 130, 0xFFFF);
            w_u32(object_offset + 28, flags | 0x10000);
            w_u32(object_offset + 104, animation);
            w_u8(definition + 125, 4);
        }
        state = r_u8(definition + 125);
    }
    if (state == 2)
    {
        animation = r_u32(object_offset + 104);
        if (animation != r_u32(actions + 72 + (uint32)r_s16(action + 16) * 4))
            w_u8(definition + 125, 7);
        else if (r_s16(object_offset + 130) != r_s16(animation + 6) - 1)
            w_u8(definition + 125, 4);
        w_u16(local + 8, 8);
    }
    else if (state == 3)
    {
        if (r_u32(object_offset + 104) != r_u32(actions + 72 + (uint32)r_s16(action + 16) * 4))
            w_u8(definition + 125, 7);
        if (r_s16(object_offset + 130) == r_s16(r_u32(object_offset + 104) + 6) - 1)
            w_u8(definition + 125, r_s16(action + 20) != 0 ? 5 : 7);
        else
        {
            value = (r_u8(definition + 132) - 1) & 0xFF;
            w_u8(definition + 132, value);
            if (value == 0)
                w_u16(local + 8, 1);
            else
                w_u8(definition + 125, 4);
        }
    }
    if (r_u8(definition + 125) == 4)
    {
        flags = r_u16(definition + 120);
        w_u16(local + 8, r_u16(action + ((flags & 1) != 0 ? 38 : 30)));
        if (r_u32(object_offset + 104) != r_u32(actions + 72 + (uint32)r_s16(action + 16) * 4))
            w_u8(definition + 125, 7);
        else
        {
            flags = r_u16(definition + 120);
            if ((flags & 1) == 0 && r_s16(action + 36) != 0 && r_s16(object_offset + 130) >= r_s16(action + 36))
            {
                w_u16(definition + 120, flags | 1);
                parameter = r_s16(action + 38);
                turn = ((parameter == 3 || parameter == 8) && r_s32(local + 152) != 1);
                if (turn == 0)
                {
                    if (parameter == 3 || parameter == 8)
                        parameter = r_s16(action + 38);
                    turn = ((parameter == 4 || parameter == 5) && r_s32(local + 152) == 1) || (parameter == 7 && r_s16(action + 30) == 6);
                }
                if (turn == 0)
                    w_u16(local + 8, r_u16(action + 38));
                else
                    goto reverse_action;
            }
            else if (r_s16(object_offset + 130) == r_s16(r_u32(object_offset + 104) + 6) - 1)
                w_u8(definition + 125, r_s16(action + 20) != 0 ? 5 : 7);
            else if (r_u16(local + 8) == 8 && r_s32(local + 152) != 1)
            {
            reverse_action:
                w_u8(definition + 125, 3);
                parent = r_u32(object_offset + 132);
                flags = r_u32(object_offset + 28);
                x = r_u16(object_offset + 136);
                y = r_u16(parent + 26);
                w_u32(object_offset + 28, (flags & 0xFFFFF7FF) ^ 0x10);
                w_u16(object_offset + 136, y - x);
                parameter = r_s16(actions + 652);
                w_u16(local + 8, 1);
                if (parameter != 0)
                    w_u8(definition + 132, 0);
                else
                {
                    parameter = r_s16(actions + 68);
                    if (parameter == 0)
                        xport_mips_break(7);
                    delay = (uint32)(2048 / parameter) - 2;
                    if ((delay & 0x80) != 0)
                        delay = 0;
                    w_u8(definition + 132, delay);
                }
            }
        }
    }
    if (r_u8(definition + 125) == 5)
    {
        w_u8(definition + 125, 6);
        animation = r_u32(actions + 72 + (uint32)r_s16(action + 22) * 4);
        flags = r_u32(object_offset + 28);
        w_u16(object_offset + 130, 0xFFFF);
        w_u32(object_offset + 104, animation);
        w_u32(object_offset + 28, flags | 0x10000);
        w_u32(object_offset + 112, (uint32)r_s16(action + 20));
        parameter = r_s16(action + 28);
        turn = ((parameter == 3 || parameter == 8) && r_s32(local + 152) != 1);
        if (turn == 0)
        {
            if (parameter == 3 || parameter == 8)
                parameter = r_s16(action + 28);
            turn = ((parameter == 4 || parameter == 5) && r_s32(local + 152) == 1);
        }
        if (turn != 0)
        {
            parent = r_u32(object_offset + 132);
            flags = r_u32(object_offset + 28);
            x = r_u16(object_offset + 136);
            y = r_u16(parent + 26);
            w_u32(object_offset + 28, (flags & 0xFFFFF7FF) ^ 0x10);
            w_u16(object_offset + 136, y - x);
            parameter = r_s16(actions + 652);
            w_u16(local + 8, 1);
            if (parameter != 0)
                w_u8(definition + 132, 0);
            else
            {
                parameter = r_s16(actions + 68);
                if (parameter == 0)
                    xport_mips_break(7);
                delay = (uint32)(2048 / parameter) - 2;
                if ((delay & 0x80) != 0)
                    delay = 0;
                w_u8(definition + 132, delay);
            }
        }
    }
    if (r_u8(definition + 125) == 6)
    {
        value = r_u32(object_offset + 112) - 1;
        w_u32(object_offset + 112, value);
        if (value == 0)
            w_u8(definition + 125, 7);
        else
        {
            value = r_u8(definition + 132);
            if (value != 0)
            {
                w_u8(definition + 132, value - 1);
                w_u16(local + 8, 1);
                value = r_u8(definition + 132);
            }
            if (value == 0)
            {
                if (r_u32(action + 24) == 0)
                    w_u16(local + 8, 1);
                else if (r_s16(action + 28) == 4 && r_s32(local + 152) == 1)
                {
                    parent = r_u32(object_offset + 132);
                    flags = r_u32(object_offset + 28);
                    x = r_u16(object_offset + 136);
                    y = r_u16(parent + 26);
                    w_u32(object_offset + 28, (flags & 0xFFFFF7FF) ^ 0x10);
                    w_u16(object_offset + 136, y - x);
                    parameter = r_s16(actions + 652);
                    w_u16(local + 8, 1);
                    if (parameter != 0)
                        w_u8(definition + 132, 0);
                    else
                    {
                        parameter = r_s16(actions + 68);
                        if (parameter == 0)
                            xport_mips_break(7);
                        delay = (uint32)(2048 / parameter) - 2;
                        if ((delay & 0x80) != 0)
                            delay = 0;
                        w_u8(definition + 132, delay);
                    }
                }
                else
                    w_u16(local + 8, r_u16(action + 28));
            }
        }
    }
    if (r_u8(definition + 125) == 7)
    {
        animation = r_u32(actions + 72);
        flags = r_u32(object_offset + 28);
        w_u16(object_offset + 130, 0xFFFF);
        w_u32(object_offset + 104, animation);
        w_u32(object_offset + 28, flags | 0x10000);
        w_u8(definition + 124, 0xFF);
        w_u16(local + 8, 9);
        if (r_u8(definition + 166) != 0)
            w_u8(definition + 166, 0);
    }
    if (r_u8(definition + 125) == 6)
        speed = r_u32(action + 24);
    else if ((r_u16(definition + 120) & 1) != 0)
        speed = r_u32(action + 40);
    else
        speed = r_u32(action + 32);
    kind = r_u16(local + 8);
    if (kind == 0)
        goto free_motion;
    if (kind == 1)
    {
        w_u16(definition + 120, r_u16(definition + 120) & 0xFEFF);
        w_u32(object_offset + 28, r_u32(object_offset + 28) & 0xFFFFFFBF);
    }
    else if (kind >= 2 && kind <= 8)
    {
        parent = r_u32(object_offset + 132);
        threshold = (uint32)r_s16(parent + 26) - r_u32(0x8008A140);
        if (r_s16(object_offset + 136) >= (sint32)threshold)
        {
            value = 0;
            if (r_s8(0x8008A144) != 0 && r_s16(actions + 658) != 0 && r_u8(definition + 166) == 0 && (sint32)((uint32)r_s16(actions + 660) << 5) >= r_s32(distance_offset))
            {
                x = r_u32(target_offset) - r_u32(position_offset);
                w_u32(local + 16, x);
                y = r_u32(target_offset + 4) - r_u32(position_offset + 4);
                w_u32(local + 20, y);
                z = r_u32(target_offset + 8) - r_u32(position_offset + 8);
                w_u32(local + 24, z);
                angle = Atan2Angle((sint32)z, (sint32)x);
                value = ((r_u16(object_offset + 148) - angle + 1024) & 0xFFF) < 0x801;
            }
            if (value != 0)
                w_u8(definition + 166, 1);
            else
            {
                w_u16(definition + 120, r_u16(definition + 120) | 0x100);
                w_u32(object_offset + 28, (r_u32(object_offset + 28) & 0xFFFFF7BF) | 0x40);
                w_u32(definition + 168, kind <= 4 ? speed : 0);
            }
        }
        else
        {
            w_u16(definition + 120, r_u16(definition + 120) | 0x100);
            w_u32(object_offset + 28, (r_u32(object_offset + 28) & 0xFFFFF7BF) | 0x40);
            w_u32(definition + 168, speed);
        }
    }
    else
    {
        parent = r_u32(object_offset + 132);
        if (r_s16(object_offset + 136) < r_s16(parent + 26) - 1)
        {
        free_motion:
            w_u16(definition + 120, r_u16(definition + 120) & 0xFEFF);
            w_u32(object_offset + 28, (r_u32(object_offset + 28) & 0xFFFFF7BF) | 0x40);
        }
        else
        {
            w_u16(definition + 120, r_u16(definition + 120) & 0xFEFF);
            w_u32(object_offset + 28, r_u32(object_offset + 28) & 0xFFFFFFBF);
        }
    }
    if (r_u8(definition + 125) == 7)
    {
        value = chosen;
        chosen = 1;
        if (value == 0)
            goto restart_action;
    }
    if (r_u32(object_offset + 104) == r_u32(actions + 72 + (uint32)r_s16(action + 16) * 4))
    {
        state = r_u8(definition + 133);
        if (state == 2)
        {
            if (r_s16(action + 46) - 1 >= r_s16(object_offset + 130))
            {
                flags = r_u16(definition + 120);
                w_u8(definition + 133, 1);
                w_u8(definition + 135, 0);
                w_u16(definition + 120, flags & 0xFFF3);
            }
            state = r_u8(definition + 133);
        }
        if (state == 1)
        {
            flags = r_u16(definition + 120);
            trigger = 0;
            if ((flags & 4) == 0)
            {
                if (r_s16(object_offset + 130) >= r_s16(action + 46) - 1)
                {
                    w_u16(definition + 120, flags | 4);
                    x = r_u8(action + 46);
                    y = r_u8(action + 52);
                    w_u8(definition + 135, x + y);
                    trigger = 1;
                }
            }
            else
            {
                value = r_u16(action + 54);
                if (value != 0 && (sint32)(value - 1) < r_s16(object_offset + 130))
                    w_u8(definition + 133, 2);
                else
                {
                    value = r_u8(definition + 135);
                    if (r_s16(object_offset + 130) >= (sint32)value - 1)
                    {
                        w_u8(definition + 135, value + r_u8(action + 52));
                        trigger = 1;
                    }
                }
            }
            if (trigger == 1)
            {
                if (r_s32(local + 152) != 1)
                {
                    x = r_u32(target_offset);
                    y = r_u32(target_offset + 4);
                    z = r_u32(target_offset + 8);
                    w_u32(local + 16, x);
                    w_u32(local + 20, y);
                    w_u32(local + 24, z);
                    x = 0 - r_u32(target_offset);
                    z = 0 - r_u32(target_offset + 8);
                    w_u32(target_offset, x);
                    w_u32(target_offset + 8, z);
                }
                if (r_u16(action + 52) == 0)
                    w_u8(definition + 133, 2);
                w_u32(object_offset + 116, (uint32)r_s16(action + 44));
                w_u32(object_offset + 28, r_u32(object_offset + 28) | 0x100);
                spawned = object_operations.behaviors[6].value_handler(object_offset);
                flags = r_u32(object_offset + 28);
                w_u32(local + 48, spawned);
                w_u32(object_offset + 28, flags & 0xFFFFFEFF);
                x = r_u32(spawned + 16);
                y = r_u32(spawned + 20);
                z = r_u32(spawned + 24);
                w_u32(local + 32, x);
                w_u32(local + 36, y);
                w_u32(local + 40, z);
                spawned = r_u32(local + 48);
                value = r_u32(action + 48);
                value = r_u32(spawned + 20) + value;
                w_u32(spawned + 20, value);
                w_u32(local + 36, value);
                parameter = r_s16(object_offset + 148);
                speed = r_u32(action + 64);
                value = 0 - (uint32)(SinQ14((sint16)parameter) >> 2);
                value = (uint32)(((sint64)(sint32)speed * (sint32)value) >> 12);
                w_u32(spawned + 16, r_u32(spawned + 16) + value);
                parameter = (sint16)(1024 - r_u16(object_offset + 148));
                speed = r_u32(action + 64);
                spawned = r_u32(local + 48);
                value = 0 - (uint32)(SinQ14((sint16)parameter) >> 2);
                value = (uint32)(((sint64)(sint32)speed * (sint32)value) >> 12);
                w_u32(spawned + 24, r_u32(spawned + 24) + value);
                parameter = r_s16(action + 56);
                if (parameter < 0 || parameter > 3)
                    goto attack_finished;
                if (parameter > 0 && parameter < 3)
                {
                    p2_func_800536f0(0);
                    w_u32(local + 88, r_u32(action + 60));
                    parameter = r_s16(action + 56);
                    w_u16(local + 176, 0);
                    if (parameter == 1)
                        w_u8(local + 168, 1);
                    else
                    {
                        w_u32(local + 92, 0);
                        w_u32(local + 96, 4096 - r_u32(action + 60));
                        x = (uint32)(r_s32(definition + 196) >> 6);
                        y = (uint32)(r_s32(definition + 200) >> 6);
                        z = (uint32)(r_s32(definition + 204) >> 6);
                        value = x * x + y * y + z * z;
                        w_u8(local + 168, 3);
                        if ((sint32)value < 0)
                            length = p2_sdk_8007e794(value >> 2) << 1;
                        else
                            length = p2_sdk_8007e794(value);
                        if ((r_u32(object_offset + 28) & 0x10) == 0)
                            length = 0 - length;
                        angle = (uint32)AsinAngle(r_s32(definition + 200), (sint32)length);
                        w_u16(local + 176, angle);
                    }
                    iteration = 0;
                    if (r_u8(local + 168) == 0)
                        goto attack_finished;
                    attack = 1;
                    input = local + 104;
                    cached_input = local + 120;
                }
                else
                {
                    attack = parameter == 3 ? 3 : 0;
                    input = attack == 0 ? local + 56 : local + 104;
                    cached_input = local + 72;
                    iteration = 0;
                }
            next_projectile:
                if (attack == 1 && (sint32)iteration > 0)
                {
                    w_u32(object_offset + 28, r_u32(object_offset + 28) | 0x100);
                    spawned = object_operations.behaviors[6].value_handler(object_offset);
                    flags = r_u32(object_offset + 28);
                    w_u32(local + 48, spawned);
                    w_u32(object_offset + 28, flags & 0xFFFFFEFF);
                    value = r_u32(spawned + 20);
                    w_u32(spawned + 20, value + r_u32(action + 48));
                }
                spawned = r_u32(local + 48);
                parent = r_u32(spawned + 132);
                if (parent != 0 && r_u32(parent + 40) != 0)
                {
                    x = r_u32(spawned + 16);
                    y = r_u32(spawned + 20);
                    z = r_u32(spawned + 24);
                    w_u32(input, x);
                    w_u32(input + 4, y);
                    w_u32(input + 8, z);
                    PushMatrix();
                    spawned = r_u32(local + 48);
                    matrix = r_u32(r_u32(spawned + 132) + 40);
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
                    p2_sdk_8007ec64(input, spawned + 16);
                    PopMatrix();
                    spawned = r_u32(local + 48);
                    parent = r_u32(spawned + 132);
                    matrix = r_u32(parent + 40);
                    value = r_u32(spawned + 16) + r_u32(matrix + 20);
                    parent = r_u32(spawned + 132);
                    w_u32(spawned + 16, value);
                    matrix = r_u32(parent + 40);
                    value = r_u32(spawned + 20) + r_u32(matrix + 24);
                    parent = r_u32(spawned + 132);
                    w_u32(spawned + 20, value);
                    matrix = r_u32(parent + 40);
                    value = r_u32(matrix + 28);
                    w_u32(spawned + 24, r_u32(spawned + 24) + value);
                }
                else
                {
                    x = r_u32(spawned + 16);
                    y = r_u32(spawned + 20);
                    z = r_u32(spawned + 24);
                    w_u32(spawned + 16, x);
                    w_u32(spawned + 20, y);
                    w_u32(spawned + 24, z);
                }
                if (attack == 3)
                {
                    spawned = r_u32(local + 48);
                    flags = r_u32(spawned + 28);
                    w_u8(spawned + 124, 0);
                    spawned = r_u32(local + 48);
                    w_u32(spawned + 28, flags | 0x2000);
                    w_u32(spawned + 96, 0);
                    goto attack_finished;
                }
                spawned = r_u32(local + 48);
                parent = r_u32(spawned + 132);
                if (parent != 0 && r_u32(parent + 40) != 0)
                {
                    x = r_u32(local + 32);
                    y = r_u32(local + 36);
                    z = r_u32(local + 40);
                    w_u32(cached_input, x);
                    w_u32(cached_input + 4, y);
                    w_u32(cached_input + 8, z);
                    PushMatrix();
                    spawned = r_u32(local + 48);
                    matrix = r_u32(r_u32(spawned + 132) + 40);
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
                    p2_sdk_8007ec64(cached_input, local + 32);
                    PopMatrix();
                    spawned = r_u32(local + 48);
                    matrix = r_u32(r_u32(spawned + 132) + 40);
                    w_u32(local + 32, r_u32(local + 32) + r_u32(matrix + 20));
                    matrix = r_u32(r_u32(spawned + 132) + 40);
                    w_u32(local + 36, r_u32(local + 36) + r_u32(matrix + 24));
                    matrix = r_u32(r_u32(spawned + 132) + 40);
                    w_u32(local + 40, r_u32(local + 40) + r_u32(matrix + 28));
                }
                spawned = r_u32(local + 48);
                flags = r_u32(spawned + 28);
                w_u8(spawned + 124, 0);
                w_u32(spawned + 28, flags | 0x2000);
                if (attack == 0)
                    p2_func_800536f0(0);
                spawned = r_u32(local + 48);
                w_u32(spawned + 92, r_u32(target_offset) - r_u32(local + 32));
                w_u32(spawned + 96, r_u32(target_offset + 4) - r_u32(local + 36));
                z = r_u32(target_offset + 8) - r_u32(local + 40);
                w_u32(spawned + 100, z);
                if (attack == 0)
                {
                    magnitude = r_u32(spawned + 96);
                    if ((sint32)magnitude < 0)
                        magnitude = 0 - magnitude;
                }
                x = r_u32(spawned + 92);
                value = (uint32)(((sint64)(sint32)x * (sint32)x) >> 12) + (uint32)(((sint64)(sint32)z * (sint32)z) >> 12);
                if ((sint32)value < 0)
                {
                    x = r_u32(spawned + 92);
                    value = (uint32)(((sint64)(sint32)x * (sint32)x) >> 12) + (uint32)(((sint64)(sint32)z * (sint32)z) >> 12);
                    length = p2_sdk_8007e794(value >> 2) << 1;
                }
                else
                {
                    x = r_u32(spawned + 92);
                    value = (uint32)(((sint64)(sint32)x * (sint32)x) >> 12) + (uint32)(((sint64)(sint32)z * (sint32)z) >> 12);
                    length = p2_sdk_8007e794(value);
                }
                if (attack == 0)
                {
                    value = (uint32)((sint32)length >> 1);
                    w_u32(distance_offset, value);
                    if ((sint32)magnitude >= (sint32)value)
                    {
                        if (magnitude == 0)
                            xport_mips_break(7);
                        signed_value = 0x40000000 / (sint32)magnitude;
                        value = (uint32)(((sint64)(sint32)value * signed_value) >> 18);
                        spawned = r_u32(local + 48);
                        x = r_u32(spawned + 96);
                        w_u32(0x1F80030C, value);
                        w_u32(spawned + 96, (uint32)(((sint64)(sint32)x * (sint32)value) >> 12));
                    }
                    goto attack_finished;
                }
                sine = (uint32)SinQ14(r_s16(local + 88));
                spawned = r_u32(local + 48);
                signed_value = (sint16)((sint32)sine >> 2);
                w_u32(spawned + 96, (uint32)(((sint64)(sint32)length * signed_value) >> 12));
                parent = r_u32(object_offset + 132);
                value = 0;
                if (parent != 0 && (r_u32(parent + 16) & 8) != 0)
                {
                    value = r_u32(r_u32(parent + 44) + 56);
                    sign_x = 1;
                }
                if (value != 0)
                {
                    sign_z = 1;
                    x = r_u32(spawned + 20);
                    y = r_u32(action + 48);
                    z = r_u32(spawned + 92);
                    w_u32(spawned + 20, x - y);
                    if ((sint32)z < 0)
                    {
                        sign_x = -1;
                        w_u32(spawned + 92, 0 - z);
                    }
                    spawned = r_u32(local + 48);
                    z = r_u32(spawned + 100);
                    if ((sint32)z < 0)
                    {
                        sign_z = -1;
                        w_u32(spawned + 100, 0 - z);
                    }
                    signed_value = r_s8(0x8008A14C);
                    if (signed_value == -1)
                        signed_value = (sint32)iteration;
                    angle = (r_u32(local + 88 + (uint32)signed_value * 4) + r_u16(local + 176)) & 0xFFF;
                    sine = (uint32)SinQ14((sint16)angle);
                    cosine = (uint32)SinQ14((sint16)(1024 - angle));
                    sine = (uint32)((sint32)sine >> 2);
                    cosine = (uint32)((sint32)cosine >> 2);
                    spawned = r_u32(local + 48);
                    x = r_u32(spawned + 92);
                    z = r_u32(spawned + 100);
                    value = (uint32)(((sint64)(sint32)(x + z) * (sint32)cosine) >> 12);
                    x = r_u32(spawned + 92);
                    w_u32(spawned + 96, value);
                    value = (uint32)(((sint64)(sint32)x * (sint32)sine) >> 12);
                    z = r_u32(spawned + 100);
                    w_u32(spawned + 92, value);
                    w_u32(spawned + 100, (uint32)(((sint64)(sint32)z * (sint32)sine) >> 12));
                    if (sign_x == -1)
                        w_u32(spawned + 92, 0 - r_u32(spawned + 92));
                    if (sign_z == -1)
                    {
                        spawned = r_u32(local + 48);
                        w_u32(spawned + 100, 0 - r_u32(spawned + 100));
                    }
                }
                else if (iteration == 1)
                {
                    spawned = r_u32(local + 48);
                    w_u32(spawned + 92, 0);
                    w_u32(spawned + 100, 0);
                }
                else if (iteration == 2)
                {
                    spawned = r_u32(local + 48);
                    x = r_u32(spawned + 92);
                    z = r_u32(spawned + 100);
                    w_u32(spawned + 92, 0 - x);
                    w_u32(spawned + 100, 0 - z);
                }
                count = r_u8(local + 168);
                iteration += 1;
                if ((sint32)iteration < (sint32)count)
                    goto next_projectile;
            attack_finished:
                if (r_s32(local + 152) != 1)
                {
                    x = r_u32(local + 16);
                    y = r_u32(local + 20);
                    z = r_u32(local + 24);
                    w_u32(target_offset, x);
                    w_u32(target_offset + 4, y);
                    w_u32(target_offset + 8, z);
                }
                spawned = r_u32(local + 48);
                x = r_u32(spawned + 92);
                y = r_u32(spawned + 96);
                z = r_u32(spawned + 100);
                w_u32(definition + 184, x);
                w_u32(definition + 188, y);
                w_u32(definition + 192, z);
            }
        }
    }
    if (r_s16(action + 18) == 1 && r_u32(r_u32(object_offset + 132) + 32) != 0)
    {
        w_u16(definition + 122, r_u16(definition + 122) | 2);
        frame = r_s16(object_offset + 136);
        if ((r_u32(object_offset + 28) & 0x10) != 0)
        {
            parameter = r_s16(r_u32(object_offset + 132) + 26);
            next_frame = frame + 1;
            if (frame == parameter - 1)
            {
                frame = parameter - 2;
                next_frame = frame + 1;
            }
        }
        else
        {
            next_frame = frame - 1;
            if (frame == 0)
            {
                frame = 1;
                next_frame = 0;
            }
        }
        p2_func_80056340(r_u32(object_offset + 132), frame, local + 16, 64);
        p2_func_80056340(r_u32(object_offset + 132), next_frame, local + 32, 64);
        x = r_u32(local + 32);
        value = r_u32(local + 16);
        y = r_u32(local + 36);
        flags = r_u32(local + 20);
        z = r_u32(local + 40);
        threshold = r_u32(local + 24);
        x -= value;
        y -= flags;
        z -= threshold;
        w_u32(local + 56, x);
        w_u32(local + 60, y);
        w_u32(local + 64, z);
        x = (uint32)((sint32)x >> 6);
        y = (uint32)((sint32)y >> 6);
        z = (uint32)((sint32)z >> 6);
        value = x * x + y * y + z * z;
        if ((sint32)value < 0)
            length = p2_sdk_8007e794(value >> 2) << 1;
        else
            length = p2_sdk_8007e794(value);
        angle = (uint32)AsinAngle(r_s32(local + 60), (sint32)length);
        w_u16(object_offset + 86, (0 - angle) & 0xFFF);
    }
    if (r_s8(definition + 124) != -1)
    {
        if (r_u8(definition + 125) >= 5)
            value = r_u8(action) & 4;
        else if ((r_u16(definition + 120) & 1) != 0)
            value = r_u8(action) & 2;
        else
            value = r_u8(action) & 1;
        if (value != 0)
        {
            flags = r_u16(definition + 122);
            kind = r_u16(local + 8);
            w_u16(definition + 122, flags | 0x200);
            if (kind == 4 || kind == 7 || kind == 5)
            {
                w_u16(definition + 122, r_u16(definition + 122) | 0x400);
                goto check_motion_flag;
            }
        }
        else
        {
        check_motion_flag:
            if (r_u16(local + 8) == 5)
                w_u16(definition + 122, r_u16(definition + 122) | 0x800);
        }
        if (r_u8(local + 144) == 1)
            w_u16(definition + 122, r_u16(definition + 122) | 0x100);
        if ((r_u16(action) & 0x10) != 0)
            w_u16(definition + 122, r_u16(definition + 122) | 0x1000);
    }
update_counters:
    if ((r_u8(actions + 136) & 1) != 0)
    {
        if (r_u8(local + 136) == 1)
        {
            value = r_u32(definition + 136);
            flags = r_u16(definition + 120) & 0xFFFD;
            w_u32(definition + 136, value + 1);
            w_u16(definition + 120, flags);
        }
        else if (r_u32(definition + 136) != 0)
        {
            flags = r_u16(definition + 120);
            if ((flags & 2) != 0)
                w_u32(definition + 136, 0);
            else
                w_u16(definition + 120, flags | 2);
        }
        for (scan = 0; scan < 6; ++scan)
        {
            if (r_s8(local + scan) != 0)
                w_u32(definition + 140 + scan * 4, r_u32(definition + 140 + scan * 4) + 1);
            else
                w_u32(definition + 140 + scan * 4, 0);
        }
    }
    flags = r_u16(definition + 120);
    result = 16;
    if ((flags & 0x10) != 0)
    {
        result = flags & 0xFFEF;
        if (r_s8(object_offset + 125) != 16)
            w_u16(definition + 120, result);
    }
    p2_guest_stack_pointer = saved_stack;
    return result;
}
