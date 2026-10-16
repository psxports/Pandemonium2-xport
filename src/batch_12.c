#include "game.h"

uint32 p2_func_8004a218(uint32 object_offset, uint32 character_offset, uint32 flags_offset)
{
    uint32 saved_stack, vector, flags, auxiliary_flags, other, parent, data, parent_flag, special;
    uint32 sound, volume, pointer, x, y, z, relative_x, relative_y, relative_z, strength, value, mode, index;
    sint32 kind, direction, angle;
    saved_stack = p2_guest_stack_pointer;
    p2_guest_stack_pointer -= 16;
    vector = p2_guest_stack_pointer;
    flags = r_u32(character_offset);
    auxiliary_flags = r_u32(character_offset + 4);
    other = r_u32(character_offset + 180);
    w_u32(character_offset, (flags | 0x40000) & 0xFF7FFFFF);
    w_u32(character_offset + 4, auxiliary_flags & 0xFFFFFF7F);
    if (!other || r_u8(other + 12))
    {
        pointer = r_u32(character_offset + 116);
        strength = r_u32(pointer + 68);
        relative_x = r_u32(pointer + 36);
        relative_y = r_u32(pointer + 40);
        relative_z = r_u32(pointer + 44);
        direction = r_s16(character_offset + 678);
        goto resolve_direction_8004a218;
    }
    parent = r_u32(other + 132);
    parent_flag = 0;
    data = 0;
    if (parent)
    {
        data = r_u32(parent + 44);
        parent_flag = r_u16(data + 122) & 0x1000;
    }
    kind = r_s8(other + 125);
    special = 0;
    if (kind == 20 && (r_u32(other + 28) & 0x40))
        special = 1;
    if (!special && parent_flag && (r_u32(other + 28) & 0x40))
    {
        kind = r_s8(other + 125);
        special = 1;
    }
    if (special)
    {
        if (kind == 20 || (r_u16(data + 122) & 0x1000))
        {
            pointer = r_u32(r_u32(r_u32(other + 132) + 44) + 72);
            pointer = r_u32(pointer + 128);
            sound = 0;
            if (pointer && r_s32(pointer) >= 2)
                sound = pointer + 12;
            volume = r_u16(0x8008A24C);
            p2_func_8005e538(sound, sound, 0, volume, 0, 3);
            w_u32(character_offset, r_u32(character_offset) | 0x08000000);
        }
        if ((r_u32(object_offset + 28) & 0x400) && r_s8(other + 125) != 30)
        {
            w_u32(vector + 8, 0);
            w_u32(vector, 0);
            w_u32(vector + 4, 4096);
            w_u32(object_offset + 96, 0);
        }
        else
        {
            x = r_u32(object_offset + 60);
            y = r_u32(object_offset + 64);
            z = r_u32(object_offset + 68);
            w_u32(vector, x);
            w_u32(vector + 4, y);
            w_u32(vector + 8, z);
            pointer = r_u32(character_offset + 180);
            relative_x = r_u32(pointer + 60);
            relative_y = r_u32(pointer + 64);
            relative_z = r_u32(pointer + 68);
            relative_x = x - relative_x;
            relative_y = y - relative_y;
            relative_z = z - relative_z;
            if (r_s8(other + 125) == 30)
            {
                w_u32(vector, (sint32)relative_x < 0 ? (uint32)-1672 : 1672);
                w_u32(vector + 8, (sint32)relative_z < 0 ? (uint32)-1672 : 1672);
                w_u32(vector + 4, 3344);
                p2_func_80027b1c(object_offset, character_offset, 1);
            }
            else
            {
                w_u32(vector, (sint32)relative_x < 0 ? (uint32)-2364 : 2364);
                w_u32(vector + 8, (sint32)relative_z < 0 ? (uint32)-2364 : 2364);
                w_u32(vector + 4, 2364);
            }
        }
        if (r_s8(other + 125) == 30)
            strength = 409;
        else
            strength = (r_u32(object_offset + 28) & 0x400) ? 16384 : 40960;
        mode = 0xFFFFFFFF;
        w_u32(character_offset + 44, 0);
        w_u32(character_offset + 40, 0);
        w_u32(character_offset + 36, 0);
        goto apply_vector_8004a218;
    }
    pointer = r_u32(character_offset + 180);
    relative_x = r_u32(pointer + 60);
    relative_y = r_u32(pointer + 64);
    relative_z = r_u32(pointer + 68);
    kind = r_s8(other + 125);
    if (kind == 19 && (r_u32(other + 28) & 0x40))
    {
        strength = 40960;
        direction = -1;
        goto resolve_direction_8004a218;
    }
    if (kind == 23)
    {
        pointer = r_u32(r_u32(r_u32(other + 132) + 44) + 72);
        pointer = r_u32(pointer + 128);
        sound = 0;
        if (pointer && r_s32(pointer) >= 2)
            sound = pointer + 12;
        volume = r_u16(0x8008A24C);
        p2_func_8005e538(sound, sound, 0, volume, 0, 3);
    }
    strength = r_s8(other + 125) == 19 ? 12288 : 8192;
    direction = -1;
    if (r_s8(other + 126) == 2 && (r_u32(other + 28) & 0x40))
    {
        x = r_u32(object_offset + 60);
        y = r_u32(object_offset + 64);
        z = r_u32(object_offset + 68);
        w_u32(vector, x);
        w_u32(vector + 4, y);
        w_u32(vector + 8, z);
        value = p2_func_8005dc00((sint32)(relative_z - z), (sint32)(relative_x - x));
        angle = r_s16(other + 148) - (sint16)value;
        w_u32(vector, (uint32)angle);
        if (angle < 0)
            w_u32(vector, 0 - (uint32)angle);
        if (r_s32(vector) < 1024)
            strength += r_u32(r_u32(r_u32(other + 132) + 44) + 16);
        relative_x += r_u32(other + 92);
        relative_y += r_u32(other + 96);
        relative_z += r_u32(other + 100);
    }
resolve_direction_8004a218:
    mode = (uint32)direction - 1;
    if (direction < 0)
    {
        pointer = r_u32(0x8008A114);
        x = r_u32(pointer + 16);
        relative_x = x - relative_x;
        y = r_u32(pointer + 20);
        relative_y = y - relative_y;
        z = r_u32(pointer + 24);
        relative_z = z - relative_z;
        value = p2_func_8005dc00((sint32)relative_z, (sint32)relative_x);
        angle = r_s16(object_offset + 150);
        value = ((uint32)(sint32)(sint16)value - (uint32)angle - 1024) & 0xFFF;
        w_u32(character_offset + 24, value < 2048 ? strength : 0 - strength);
        goto finish_8004a218;
    }
    index = (uint32)((sint32)mode >> 1) * 6;
    pointer = r_u32(character_offset + 116);
    value = (uint32)(sint32)r_s16(pointer + index + 16);
    w_u32(vector, (mode & 1) ? value : 0 - value);
    pointer = r_u32(character_offset + 116);
    value = (uint32)(sint32)r_s16(pointer + index + 18);
    w_u32(vector + 4, (mode & 1) ? value : 0 - value);
    pointer = r_u32(character_offset + 116);
    value = (uint32)(sint32)r_s16(pointer + index + 20);
    w_u32(vector + 8, (mode & 1) ? value : 0 - value);
apply_vector_8004a218:
    w_u32(flags_offset, r_u32(flags_offset) | 8);
    p2_func_80049928(object_offset, character_offset, flags_offset, mode, vector, (sint32)strength, 36044);
finish_8004a218:
    value = r_u16(character_offset + 706);
    w_u16(character_offset + 676, 10);
    flags = r_u32(flags_offset);
    value = flags & ((value & 15) == 3 ? 0xFFFFFFBE : 0xFFFFFF3E);
    w_u32(flags_offset, value);
    p2_guest_stack_pointer = saved_stack;
    return value;
}

uint32 p2_func_80024de0(uint32 definition_offset, uint32 position_offset, uint32 direction, sint32 height_offset)
{
    return (uint32)p2_query_link_height_with_selection(definition_offset, position_offset, direction, height_offset, 0).value;
}

P2PathProbeResult p2_query_link_height_with_selection(uint32 definition_offset, uint32 position_offset, uint32 direction, sint32 height_offset, uint32 fallback_frame)
{
    P2PathProbeResult output, probe;
    uint32 saved_stack, delta, first, second, sample, temporary, matrix, value, x, y, z;
    uint32 first_distance, second_distance, distance, best_distance, radius, extent, effective_direction;
    sint32 frames, low, high, frame, best_frame, nearer_first, choose_high;
    sint32 c0, c1, c2, c3, c4, c5, c6, c7, c8;
    if (!r_u32(definition_offset + 32))
    {
        output.value = 0;
        output.fallback_frame = fallback_frame;
        return output;
    }
    saved_stack = p2_guest_stack_pointer;
    p2_guest_stack_pointer -= 80;
    delta = p2_guest_stack_pointer;
    first = delta + 16;
    second = delta + 32;
    sample = delta + 48;
    temporary = delta + 64;
    effective_direction = direction ? direction : 16;
    frames = r_s16(definition_offset + 26);
    if (height_offset < 0)
    {
        low = 0;
        matrix = r_u32(definition_offset + 40);
        high = frames - 1;
        if (matrix)
        {
            probe = p2_sample_track_with_selection(0, definition_offset + 20, temporary, fallback_frame);
            fallback_frame = probe.fallback_frame;
            PushMatrix();
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
            p2_sdk_8007ec64(temporary, first);
            PopMatrix();
            /* TODO: Audit PopMatrix diagnostic exits for the secondary output */
            fallback_frame = xport_gte_read_translation(0);
            x = r_u32(first);
            y = r_u32(matrix + 20);
            w_u32(first, x + y);
            x = r_u32(first + 4);
            y = r_u32(matrix + 24);
            w_u32(first + 4, x + y);
            x = r_u32(first + 8);
            y = r_u32(matrix + 28);
            w_u32(first + 8, x + y);
        }
        else
        {
            probe = p2_sample_track_with_selection(0, definition_offset + 20, first, fallback_frame);
            fallback_frame = probe.fallback_frame;
        }
        x = r_u32(first);
        x -= r_u32(position_offset);
        y = r_u32(first + 4);
        w_u32(delta, x);
        y -= r_u32(position_offset + 4);
        z = r_u32(first + 8);
        w_u32(delta + 4, y);
        z -= r_u32(position_offset + 8);
        w_u32(delta + 8, z);
        w_u32(delta + 4, 0);
        x = (uint32)((sint32)x >> 6);
        z = (uint32)((sint32)z >> 6);
        first_distance = x * x + z * z;
        matrix = r_u32(definition_offset + 40);
        if (matrix)
        {
            probe = p2_sample_track_with_selection(high, definition_offset + 20, temporary, fallback_frame);
            fallback_frame = probe.fallback_frame;
            PushMatrix();
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
            p2_sdk_8007ec64(temporary, second);
            PopMatrix();
            /* TODO: Audit PopMatrix diagnostic exits for the secondary output */
            fallback_frame = xport_gte_read_translation(0);
            x = r_u32(second);
            y = r_u32(matrix + 20);
            w_u32(second, x + y);
            x = r_u32(second + 4);
            y = r_u32(matrix + 24);
            w_u32(second + 4, x + y);
            x = r_u32(second + 8);
            y = r_u32(matrix + 28);
            w_u32(second + 8, x + y);
        }
        else
        {
            probe = p2_sample_track_with_selection(high, definition_offset + 20, second, fallback_frame);
            fallback_frame = probe.fallback_frame;
        }
        x = r_u32(second);
        x -= r_u32(position_offset);
        y = r_u32(second + 4);
        w_u32(delta, x);
        y -= r_u32(position_offset + 4);
        z = r_u32(second + 8);
        w_u32(delta + 4, y);
        z -= r_u32(position_offset + 8);
        w_u32(delta + 8, z);
        w_u32(delta + 4, 0);
        x = (uint32)((sint32)x >> 6);
        z = (uint32)((sint32)z >> 6);
        second_distance = x * x + z * z;
        nearer_first = first_distance < second_distance;
        if (nearer_first)
        {
            frame = high >> 1;
            if ((sint32)(first_distance << 2) >= (sint32)second_distance)
                frame = (sint32)((uint32)(high >> 2) + (uint32)(high >> 1));
        }
        else
        {
            frame = high >> 1;
            if ((sint32)(second_distance << 2) >= (sint32)first_distance)
                frame = high >> 2;
        }
        value = r_u32(position_offset + 4);
        if ((sint32)value < r_s32(first + 4) || r_s32(second + 4) < (sint32)value)
        {
            p2_guest_stack_pointer = saved_stack;
            output.value = -1;
            output.fallback_frame = fallback_frame;
            return output;
        }
        for (;;)
        {
            matrix = r_u32(definition_offset + 40);
            if (matrix)
            {
                probe = p2_sample_track_with_selection(frame, definition_offset + 20, temporary, fallback_frame);
                fallback_frame = probe.fallback_frame;
                PushMatrix();
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
                p2_sdk_8007ec64(temporary, sample);
                PopMatrix();
                /* TODO: Audit PopMatrix diagnostic exits for the secondary output */
                fallback_frame = xport_gte_read_translation(0);
                x = r_u32(sample);
                y = r_u32(matrix + 20);
                w_u32(sample, x + y);
                x = r_u32(sample + 4);
                y = r_u32(matrix + 24);
                w_u32(sample + 4, x + y);
                x = r_u32(sample + 8);
                y = r_u32(matrix + 28);
                w_u32(sample + 8, x + y);
            }
            else
            {
                probe = p2_sample_track_with_selection(frame, definition_offset + 20, sample, fallback_frame);
                fallback_frame = probe.fallback_frame;
            }
            value = r_u32(position_offset + 4);
            y = r_u32(sample + 4);
            choose_high = nearer_first ? (sint32)value < (sint32)y : (sint32)y >= (sint32)value;
            x = r_u32(sample);
            y = r_u32(sample + 4);
            z = r_u32(sample + 8);
            if (choose_high)
            {
                w_u32(second, x);
                w_u32(second + 4, y);
                w_u32(second + 8, z);
                high = frame;
            }
            else
            {
                w_u32(first, x);
                w_u32(first + 4, y);
                w_u32(first + 8, z);
                low = frame;
            }
            frame = (sint32)((uint32)low + (uint32)high) >> 1;
            if (low == frame)
            {
                if (!direction)
                    frame = (sint32)((uint32)(sint32)r_s16(definition_offset + 26) - (uint32)frame - 1);
                p2_guest_stack_pointer = saved_stack;
                output.value = frame;
                output.fallback_frame = fallback_frame;
                return output;
            }
        }
    }
    radius = (uint32)height_offset;
    extent = radius << 1;
    matrix = r_u32(definition_offset + 40);
    if (matrix)
    {
        value = r_u16(matrix);
        c0 = (sint16)value;
        w_u16(second, value);
        value = r_u16(r_u32(definition_offset + 40) + 6);
        c1 = (sint16)value;
        w_u16(second + 2, value);
        value = r_u16(r_u32(definition_offset + 40) + 12);
        c2 = (sint16)value;
        w_u16(second + 4, value);
        value = r_u16(r_u32(definition_offset + 40) + 2);
        c3 = (sint16)value;
        w_u16(second + 6, value);
        value = r_u16(r_u32(definition_offset + 40) + 8);
        c4 = (sint16)value;
        w_u16(second + 8, value);
        value = r_u16(r_u32(definition_offset + 40) + 14);
        c5 = (sint16)value;
        w_u16(second + 10, value);
        value = r_u16(r_u32(definition_offset + 40) + 4);
        c6 = (sint16)value;
        w_u16(second + 12, value);
        value = r_u16(r_u32(definition_offset + 40) + 10);
        c7 = (sint16)value;
        w_u16(second + 14, value);
        value = r_u16(r_u32(definition_offset + 40) + 16);
        c8 = (sint16)value;
        w_u16(second + 16, value);
        x = 0 - r_u32(r_u32(definition_offset + 40) + 20);
        w_u32(delta, x);
        y = 0 - r_u32(r_u32(definition_offset + 40) + 24);
        w_u32(delta + 4, y);
        z = 0 - r_u32(r_u32(definition_offset + 40) + 28);
        w_u32(delta + 8, z);
        x += r_u32(position_offset);
        w_u32(sample + 8, x);
        y += r_u32(position_offset + 4);
        w_u32(sample + 12, y);
        z += r_u32(position_offset + 8);
        w_u32(temporary, z);
        value = (uint32)(((sint64)(sint32)x * c0) >> 12);
        fallback_frame = (uint32)((uint64)((sint64)(sint32)y * c1) >> 32) << 20;
        value += (uint32)(((sint64)(sint32)y * c1) >> 12);
        value += (uint32)(((sint64)(sint32)z * c2) >> 12);
        w_u32(first, value);
        value = (uint32)(((sint64)(sint32)x * c3) >> 12);
        value += (uint32)(((sint64)(sint32)y * c4) >> 12);
        value += (uint32)(((sint64)(sint32)z * c5) >> 12);
        w_u32(first + 4, value);
        value = (uint32)(((sint64)(sint32)x * c6) >> 12);
        value += (uint32)(((sint64)(sint32)y * c7) >> 12);
        value += (uint32)(((sint64)(sint32)z * c8) >> 12);
        w_u32(first + 8, value);
        position_offset = first;
    }
    frame = 0;
    best_frame = -1;
    best_distance = 0x7FFFF000;
    do
    {
        probe = p2_sample_track_with_selection(frame, definition_offset + 20, delta, fallback_frame);
        fallback_frame = probe.fallback_frame;
        x = r_u32(delta);
        x -= r_u32(position_offset);
        w_u32(delta, x);
        if (x + radius < extent)
        {
            y = r_u32(delta + 4);
            y -= r_u32(position_offset + 4);
            w_u32(delta + 4, y);
            if (y + radius < extent)
            {
                z = r_u32(delta + 8);
                z -= r_u32(position_offset + 8);
                w_u32(delta + 8, z);
                if (z + radius < extent)
                {
                    x = (uint32)((sint32)x >> 6);
                    y = (uint32)((sint32)y >> 6);
                    z = (uint32)((sint32)z >> 6);
                    distance = x * x + y * y + z * z;
                    if ((sint32)distance < (sint32)best_distance)
                    {
                        best_distance = distance;
                        best_frame = frame;
                    }
                }
            }
        }
        --frames;
        frame = (sint32)((uint32)frame + 1);
    } while (frames > 0);
    if (best_frame >= 0 && !effective_direction)
        best_frame = (sint32)((uint32)(sint32)r_s16(definition_offset + 26) - (uint32)best_frame - 1);
    p2_guest_stack_pointer = saved_stack;
    output.value = best_frame;
    output.fallback_frame = fallback_frame;
    return output;
}

uint32 p2_func_80062e3c(uint32 vertices_offset, uint32 polygons_offset, sint32 count)
{
    uint32 result, saved_stack, local, color, packet, polygon, remaining, value, sum, limit;
    uint32 base, first, second, third, fourth, first_index, second_index, third_index, fourth_index;
    uint32 camera, far_distance, near_distance, distance, slot, entry, x, y, z;
    sint32 coefficient, normal;
    sint64 product;
    if ((r_u32(0x1F800320) & 4) && (r_u32(r_u32(polygons_offset)) & 0x40000))
    {
        result = r_u32(0x8008A178);
        if (result)
            return result;
    }
    saved_stack = p2_guest_stack_pointer;
    p2_guest_stack_pointer -= 8;
    local = p2_guest_stack_pointer;
    color = r_u32(0x1F8002B0);
    packet = r_u32(0x8008A4F8);
    w_u32(local + 4, color | 0x28000000);
    xport_gte_write_data(8, 0);
    polygon = polygons_offset + 22;
    remaining = (uint32)count;
    do
    {
        value = r_u32(0x8008A4F8);
        limit = r_u32(0x8008A4A0);
        result = value < limit;
        if (!result)
        {
            p2_guest_stack_pointer = saved_stack;
            return result;
        }
        coefficient = r_s32(0x1F800258) >> 6;
        normal = r_s16(polygon - 6);
        x = (uint32)((sint64)coefficient * normal);
        coefficient = r_s32(0x1F80025C) >> 6;
        normal = r_s16(polygon - 4);
        y = (uint32)((sint64)coefficient * normal);
        coefficient = r_s32(0x1F800260) >> 6;
        normal = r_s16(polygon - 2);
        z = (uint32)((sint64)coefficient * normal);
        value = r_u32(polygon - 10);
        result = (uint32)((sint32)(x + y + z) >> 6) + value;
        if ((sint32)result <= 0)
            goto next_polygon_80062e3c;
        first_index = r_u16(polygon - 18);
        base = r_u32(0x1F8002C8);
        second_index = r_u16(polygon - 16);
        fourth_index = r_u16(polygon - 12);
        third_index = r_u16(polygon - 14);
        first = base + first_index * 8;
        second = base + second_index * 8;
        third = base + third_index * 8;
        fourth = base + fourth_index * 8;
        result = r_u16(first) < 320;
        if (!result)
            result = r_u16(second) < 320;
        if (!result)
            result = r_u16(third) < 320;
        if (!result)
            result = r_u16(fourth) < 320;
        if (!result)
            goto next_polygon_80062e3c;
        result = r_u16(first + 2) < 240;
        if (!result)
            result = r_u16(second + 2) < 240;
        if (!result)
            result = r_u16(third + 2) < 240;
        if (!result)
            result = r_u16(fourth + 2) < 240;
        if (!result)
            goto next_polygon_80062e3c;
        x = r_u32(first + 4);
        y = r_u32(second + 4);
        z = r_u32(third + 4);
        value = r_u32(fourth + 4);
        limit = r_u32(0x1F8002B4);
        sum = x + y + z + value;
        result = (sint32)(sum << 1) < (sint32)limit;
        w_u32(local, sum);
        if (!result)
            goto next_polygon_80062e3c;
        value = r_u16(polygon);
        x = (uint32)(sint32)r_s16(0x1F8002A6);
        y = r_u32(0x1F8002B8);
        value = (uint32)((sint32)(value << 16) >> 17) + x;
        sum = (uint32)((sint32)sum >> 2) + value;
        value = (uint32)((sint64)(sint32)sum * (sint32)y);
        slot = (uint32)((sint32)value >> 19);
        result = slot < 1024;
        w_u32(local, slot);
        if (!result)
        {
            result = 1023;
            if ((sint32)slot < 0)
                w_u32(local, 0);
            else
                w_u32(local, result);
        }
        camera = r_u32(0x8008A4E8);
        far_distance = r_u32(camera + 152);
        if (far_distance)
        {
            slot = r_u32(local);
            limit = r_u32(0x1F8002B4);
            product = (sint64)(sint32)limit * (sint32)(slot << 2);
            distance = (uint32)(product >> 12);
            near_distance = r_u32(camera + 148);
            result = (sint32)near_distance < (sint32)distance;
            if (result)
            {
                result = distance - near_distance;
                if ((sint32)distance >= (sint32)far_distance)
                    goto next_polygon_80062e3c;
                value = r_u32(0x1F8002BC);
                product = (sint64)(sint32)value * (sint32)result;
                value = (uint32)((sint32)(uint32)(product >> 12) >> 8);
                xport_gte_write_data(8, value);
            }
            else
                xport_gte_write_data(8, 0);
        }
        else
            xport_gte_write_data(8, 0);
        slot = r_u32(local);
        entry = r_u32(0x8008A44C) + (slot << 2);
        value = r_u32(entry);
        w_u32(packet, (value & 0xFFFFFF) | 0x05000000);
        value = r_u32(entry);
        w_u32(entry, (value & 0xFF000000) | (packet & 0xFFFFFF));
        xport_gte_write_data(6, r_u32(local + 4));
        xport_gte_execute(0x780010);
        w_u32(packet + 4, xport_gte_read_data(22));
        w_u32(packet + 8, r_u32(first));
        w_u32(packet + 12, r_u32(second));
        w_u32(packet + 16, r_u32(fourth));
        result = r_u32(third);
        w_u32(packet + 20, result);
        packet += 24;
    next_polygon_80062e3c:
        --remaining;
        polygon += 24;
    } while (remaining);
    w_u32(0x8008A4F8, packet);
    p2_guest_stack_pointer = saved_stack;
    return result;
}

uint32 p2_func_80034044(uint32 object_offset)
{
    uint32 data, timer, value, resource, animation, count, frame, result;
    data = r_u32(r_u32(object_offset + 132) + 44);
    if (object_offset == r_u32(0x8008A510))
        w_u32(0x8008A1A4, r_u32(0x8008A1A4) | 0x800);
    timer = r_u32(object_offset + 144) - 1;
    w_u32(object_offset + 144, timer);
    if (timer == 0)
    {
        if ((r_u32(0x8008A52C) & 0x100) && r_s8(data + 37) < 44)
            return p2_func_80033d7c(object_offset);
        value = r_u16(data + 48);
        resource = r_u32(0x8008A510);
        w_u16(data + 40, value);
        if (object_offset == resource)
            w_u32(0x8008A510, 0);
        resource = r_u32(object_offset + 132);
        w_u8(resource + 13, r_u8(resource + 13) | 0x80);
        result = r_u8(object_offset + 13) | 2;
        w_u8(object_offset + 13, result);
        return result;
    }
    if (r_s8(object_offset + 125) == 17)
    {
        result = r_u16(object_offset + 130) + 1;
        value = r_u32(0x8008A520);
        w_u16(object_offset + 130, result);
        w_u32(0x1F8002A8, value);
        return result;
    }
    if ((sint32)timer < 15)
    {
        resource = r_u32(data + 72);
        animation = r_u32(object_offset + 104);
        value = r_u32(resource + 96);
        if (animation != value)
        {
            w_u32(object_offset + 28, r_u32(object_offset + 28) ^ 0x80000);
            resource = r_u32(data + 72);
            animation = r_u32(object_offset + 104);
            value = r_u32(resource + 96);
            if (animation != value)
                w_u32(object_offset + 92, (uint32)(((sint64)r_s32(object_offset + 92) * 3513) >> 12));
            return p2_func_8001a6ac(object_offset);
        }
    }
    frame = r_u16(object_offset + 130) + 1;
    animation = r_u32(object_offset + 104);
    w_u16(object_offset + 130, frame);
    count = (uint32)r_s16(animation + 6);
    if ((sint32)(sint16)frame >= (sint32)count)
        w_u16(object_offset + 130, count - 1);
    return p2_func_8001a6ac(object_offset);
}

uint32 p2_func_80018fec(sint16 angle, uint32 quaternion_offset, uint32 destination_offset)
{
    uint32 saved_stack, quaternion, source, result, index, address, upper, lower, shift, value, mask, old;
    uint32 first_word, second_word, x, y, z;
    sint32 half_angle, sine, q0, q1, q2, q3, p0, p1, p2, p3;
    half_angle = (sint32)(((uint32)2048 - (uint32)(sint32)angle) << 16) >> 17;
    if (!quaternion_offset)
    {
        result = (uint32)p2_func_8005dbb8((sint16)(1024 - half_angle));
        w_u16(destination_offset, result);
        w_u16(destination_offset + 6, 0);
        w_u16(destination_offset + 2, 0);
        result = (uint32)p2_func_8005dbb8((sint16)half_angle);
        w_u16(destination_offset + 4, result);
        return result;
    }
    saved_stack = p2_guest_stack_pointer;
    p2_guest_stack_pointer -= 8;
    quaternion = p2_guest_stack_pointer;
    result = (uint32)p2_func_8005dbb8((sint16)(1024 - half_angle));
    w_u16(quaternion, result);
    w_u16(quaternion + 6, 0);
    w_u16(quaternion + 2, 0);
    sine = p2_func_8005dbb8((sint16)half_angle);
    w_u16(quaternion + 4, (uint32)sine);
    q0 = r_s16(quaternion);
    source = 0;
    if (q0 == 0x4000)
        source = quaternion_offset;
    else
    {
        p0 = r_s16(quaternion_offset);
        if (p0 == 0x4000)
            source = quaternion;
        else
        {
            q1 = r_s16(quaternion + 2);
            p1 = r_s16(quaternion_offset + 2);
            p2 = r_s16(quaternion_offset + 4);
            q2 = (sint16)sine;
            q3 = r_s16(quaternion + 6);
            p3 = r_s16(quaternion_offset + 6);
            x = (uint32)(q0 * p0) - ((uint32)(q1 * p1) + (uint32)(q2 * p2) + (uint32)(q3 * p3));
            y = (uint32)(q0 * p1) + (uint32)(p0 * q1) + (uint32)(q2 * p3) - (uint32)(q3 * p2);
            z = (uint32)(q0 * p2) + (uint32)(p0 * q2) + (uint32)(q3 * p1) - (uint32)(q1 * p3);
            result = (uint32)(q0 * p3) + (uint32)(p0 * q3) + (uint32)(q1 * p2) - (uint32)(q2 * p1);
            w_u16(destination_offset, x >> 14);
            w_u16(destination_offset + 2, y >> 14);
            w_u16(destination_offset + 4, z >> 14);
            result = (uint32)((sint32)result >> 14);
            w_u16(destination_offset + 6, result);
        }
    }
    if (source)
    {
        for (index = 0; index < 2; ++index)
        {
            address = source + index * 4;
            upper = r_u32((address + 3) & 0xFFFFFFFC);
            lower = r_u32(address & 0xFFFFFFFC);
            shift = (address & 3) * 8;
            value = shift == 0 ? lower : (upper << (32 - shift)) | (lower >> shift);
            if (index == 0)
                first_word = value;
            else
                second_word = value;
        }
        result = first_word;
        for (index = 0; index < 2; ++index)
        {
            value = index == 0 ? first_word : second_word;
            address = destination_offset + index * 4;
            shift = ((address + 3) & 3) * 8;
            mask = 0xFFFFFFFF >> (24 - shift);
            old = r_u32((address + 3) & 0xFFFFFFFC);
            w_u32((address + 3) & 0xFFFFFFFC, (old & ~mask) | (value >> (24 - shift)));
            shift = (address & 3) * 8;
            mask = 0xFFFFFFFF << shift;
            old = r_u32(address & 0xFFFFFFFC);
            w_u32(address & 0xFFFFFFFC, (old & ~mask) | (value << shift));
        }
    }
    p2_guest_stack_pointer = saved_stack;
    return result;
}

uint32 p2_func_8004002c(uint32 source_offset)
{
    uint32 destination, bytes, value, node, child;
    if (!source_offset)
        return 0;
    destination = p2_func_80064d28(r_u32(source_offset + 224));
    if (!destination)
        return 0;
    bytes = r_u32(source_offset + 224);
    p2_func_8006637c((uint8 *)psx_addr(destination, bytes), (const uint8 *)psx_addr(source_offset, bytes), bytes);
    if (r_u32(destination + 44))
    {
        value = destination + (r_u32(source_offset + 44) - source_offset);
        w_u32(destination + 44, value);
        p2_func_80027594(value);
    }
    value = r_u32(destination + 8);
    w_u32(destination, 0);
    w_u16(destination + 148, 4096);
    if (value)
        w_u32(destination + 8, destination + (value - source_offset));
    value = r_u32(destination + 156);
    if (value)
        w_u32(destination + 156, destination + (value - source_offset));
    node = r_u32(destination + 8);
    w_u32(destination + 4, 0);
    while (node)
    {
        value = r_u32(node + 8);
        if (value)
            w_u32(node + 8, destination + (value - source_offset));
        value = r_u32(node + 4);
        if (value)
            w_u32(node + 4, destination + (value - source_offset));
        if (r_u8(node + 12) == 7)
        {
            value = r_u32(node + 20);
            if (value)
            {
                value = destination + (value - source_offset);
                w_u32(node + 20, value);
                p2_func_80027594(value);
            }
        }
        child = r_u32(node + 8);
        while (child)
        {
            p2_func_8003ff6c(child, destination, source_offset);
            child = r_u32(child + 4);
        }
        node = r_u32(node + 4);
    }
    return destination;
}

void p2_func_80034848(uint32 allocation_offset)
{
    uint32 data, object, sound;
    sint32 frame, count;
    data = r_u32(allocation_offset + 44);
    object = r_u32(data + 4);
    if (!object)
        return;
    sound = r_u32(data + 68);
    if ((p2_func_8005e868(sound, (uint32)r_s16(data + 66)) & 0xFF) != 0)
        p2_func_8006a970(r_u16(data + 66));
    w_u32(data + 68, 0);
    w_u16(data + 66, 0xFFFF);
    w_u32(data + 4, 0);
    if (r_u32(object + 28) & 0x10)
        w_u16(data + 24, r_u16(object + 136));
    else
    {
        count = r_u16(allocation_offset + 26);
        frame = r_u16(object + 136);
        w_u16(data + 24, (uint32)count - (uint32)frame - 1);
    }
    frame = r_s16(data + 24);
    if (frame < 0)
        w_u16(data + 24, 0);
    else
    {
        count = r_s16(allocation_offset + 26);
        if (frame >= count)
            w_u16(data + 24, (uint32)count - 1);
    }
    w_u32(data + 12, 0x7FFFF000);
    w_u8(object + 13, r_u8(object + 13) | 2);
}

uint32 p2_func_80034658(uint32 object_offset)
{
    uint32 value, data, count, frame, table, result;
    sint32 kind;
    value = r_u8(object_offset + 13);
    data = r_u32(object_offset + 44);
    w_u8(object_offset + 13, value & 0x7F);
    count = (uint32)(sint32)r_s8(0x8008A330);
    frame = r_u16(data + 26);
    kind = r_s8(data + 37);
    w_u32(data + 4, 0);
    w_u32(data + 12, 0x7FFFF000);
    w_u32(data + 44, 0);
    w_u16(data + 24, frame);
    result = 0;
    table = 0x8008B450;
    while (count != 0)
    {
        result = r_u32(table);
        if (r_s8(result + 61) == kind)
            break;
        --count;
        table += 4;
        result = 0;
    }
    w_u32(data + 72, result);
    return result;
}

uint32 p2_func_8005bcfc(uint32 object_offset)
{
    uint32 flags, data, command, kind, value, target, scene, found, key, table, count, match;
    flags = r_u32(object_offset + 16);
    data = r_u32(object_offset + 44);
    if ((flags & 4) != 0)
    {
        w_u32(object_offset + 16, flags & 0xFFFFFFFB);
        command = r_u32(data + 8);
        if ((command & 0xFFF) != 0)
        {
            kind = (command >> 12) & 31;
            if ((r_u16(data + 122) & 0x100) != 0)
                w_u32(data + 128, kind + 39);
            switch (kind)
            {
                case 1:
                    w_u32(object_offset + 16, r_u32(object_offset + 16) | 0x40);
                    break;
                case 3:
                    if ((r_u8(object_offset + 13) & 1) == 0)
                    {
                        p2_func_80034848(object_offset);
                        value = r_u8(object_offset + 13);
                        flags = r_u32(object_offset + 16);
                        w_u8(object_offset + 13, value | 1);
                        w_u32(object_offset + 16, flags | 1);
                    }
                    break;
                case 5:
                    w_u32(object_offset + 16, r_u32(object_offset + 16) & 0xFFFFFFBF);
                    break;
                case 8:
                    w_u16(data + 24, r_u16(data + 26));
                    p2_func_80034848(object_offset);
                    value = r_u8(object_offset + 13);
                    flags = r_u32(object_offset + 16);
                    w_u8(object_offset + 13, value | 1);
                    if ((flags & 0x20) != 0)
                    {
                        w_u32(object_offset + 16, flags | 1);
                        break;
                    }
                case 4:
                    if ((r_u8(object_offset + 13) & 1) != 0)
                    {
                        p2_func_800346c8(object_offset);
                        if (kind == 8)
                        {
                            target = r_u32(data + 4);
                            if (target != 0)
                            {
                                w_u16(target + 148, 4096);
                                target = r_u32(data + 4);
                                w_u16(target + 84, 0x4000);
                                target = r_u32(data + 4);
                                w_u16(target + 90, 0);
                                w_u16(target + 88, 0);
                                w_u16(target + 86, 0);
                            }
                        }
                        value = r_u8(object_offset + 13);
                        flags = r_u32(object_offset + 16);
                        w_u8(object_offset + 13, value & 0xFE);
                        w_u32(object_offset + 16, flags & 0xFFFFFFFE);
                    }
                    break;
            }
        }
        return 4;
    }
    scene = r_u32(0x8008A114);
    found = 0;
    if (r_s8(scene + 1) != 0)
    {
        key = r_u32(data + 8) & 0xFFF;
        if (key != 0)
        {
            w_u32(data + 8, key);
            command = r_u32(scene + 140);
            count = (uint32)(sint32)r_s8(scene + 1);
            table = scene + 144;
            for (;;)
            {
                match = command & 0xFFF;
                --count;
                if (key == match)
                {
                    w_u32(data + 8, command);
                    found = 1;
                    break;
                }
                if (count == 0)
                    break;
                command = r_u32(table);
                table += 4;
            }
        }
    }
    if (found != 0)
        w_u32(object_offset + 16, r_u32(object_offset + 16) | 4);
    else if ((r_u32(object_offset + 16) & 1) != 0 || (sint32)(0 - (r_u32(data + 12) + 2)) < 0)
    {
        if (r_u32(data + 4) != 0)
            p2_func_80034848(object_offset);
    }
    else if (r_u32(data + 4) == 0)
        p2_func_800346c8(object_offset);
    w_u32(data + 12, 0x7FFFF000);
    return 0x7FFFF000;
}

void p2_func_800346c8(uint32 object_offset)
{
    uint32 data, resource, source, object, value, node, scene;
    data = r_u32(object_offset + 44);
    resource = r_u32(data + 72);
    source = r_u32(resource + 48);
    if (source == 0 || r_u32(data + 4) != 0)
        return;
    if ((r_u8(source + 13) & 0x80) != 0)
    {
        object = source;
        w_u32(object + 4, 0);
    }
    else
    {
        object = p2_func_8004002c(source);
        if (object == 0)
            return;
        w_u32(object + 28, r_u32(object + 28) & 0xFFFFFFFB);
    }
    if ((r_u32(object + 28) & 0x8000) == 0)
    {
        if ((r_u8(r_u32(object_offset + 44) + 38) & 1) != 0)
        {
            node = r_u32(object + 156);
            w_u16(node + 42, 0);
            w_u16(node + 40, 0);
            node = r_u32(object + 156);
            value = r_u8(node + 13) | 1;
        }
        else
        {
            value = r_u32(resource + 40);
            node = r_u32(object + 156);
            w_u16(node + 40, (uint32)((sint32)value >> 5));
            value = r_u32(resource + 44);
            node = r_u32(object + 156);
            w_u16(node + 42, (uint32)((sint32)value >> 5));
            node = r_u32(object + 156);
            value = r_u8(node + 13) & 0xFE;
        }
        w_u8(node + 13, value);
    }
    w_u32(object + 132, object_offset);
    p2_func_80033d7c(object);
    w_u32(data + 4, object);
    w_u16(data + 66, 0xFFFF);
    scene = r_u32(0x8008A4E8);
    value = r_u32(scene + 108);
    w_u32(object + 4, value);
    w_u32(scene + 108, object);
}

uint32 p2_func_8006117c(uint32 object_offset)
{
    uint32 flags, result;
    sint32 step;
    flags = r_u32(object_offset + 28);
    result = flags & 0x840000;
    if (result == 0)
        return result;
    if ((r_u8(object_offset + 13) & 1) != 0)
        return 0x200000;
    step = (flags & 0x200000) != 0 ? -1 : 1;
    w_u16(object_offset + 136, step);
    return p2_func_8001b334(object_offset, step);
}

uint32 p2_func_80049730(uint32 object_offset, uint32 character_offset, uint32 horizontal_offset, uint32 vertical_offset)
{
    uint32 value, result, controller, acceleration, input, sum, flags, upper, lower;
    if ((r_u32(character_offset) & 0x80000) != 0)
    {
        value = r_u32(vertical_offset) + 101;
        w_u32(vertical_offset, value);
        if ((sint32)value > 0)
            w_u32(object_offset + 28, r_u32(object_offset + 28) | 0x400);
        value = r_u32(character_offset + 768);
        w_u32(character_offset + 32, 1);
        w_u16(character_offset + 48, value);
        w_u32(horizontal_offset, 0);
        w_u32(character_offset + 24, 0);
        result = r_u32(vertical_offset);
        w_u32(character_offset + 28, result);
        return result;
    }
    controller = r_u32(character_offset + 120);
    acceleration = r_u32(controller + 72);
    if (acceleration != 0)
    {
        input = r_u32(0x1F800384);
        if ((input & 0x1000) != 0)
            acceleration += 40;
        else if ((input & 0x4000) != 0)
            acceleration -= 40;
        sum = r_u32(vertical_offset) + acceleration;
        w_u32(vertical_offset, sum);
        if ((sint32)acceleration > 0)
        {
            if ((sint32)sum > 0)
            {
                flags = r_u32(object_offset + 28);
                if ((flags & 0x400) == 0)
                {
                    w_u32(object_offset + 28, flags | 0x400);
                    if ((r_u32(character_offset + 4) & 0x80) == 0)
                        w_u8(character_offset + 712, 3);
                }
            }
            value = r_u32(character_offset + 768);
            w_u32(character_offset + 32, 1);
            w_u16(character_offset + 48, value);
        }
        else if ((r_u32(object_offset + 28) & 0x400) != 0 && (sint32)sum < 0 && r_s32(character_offset + 32) >= 0)
            w_u32(character_offset + 32, 0xFFFFFFFF);
        w_u32(character_offset + 552, 0);
        w_u32(character_offset + 28, r_u32(vertical_offset));
        controller = r_u32(character_offset + 120);
    }
    result = controller;
    acceleration = r_u32(controller + 68);
    if (acceleration == 0)
        return result;
    if ((r_u32(object_offset + 28) & 0x10) == 0)
        acceleration = 0 - acceleration;
    value = r_u32(r_u32(character_offset + 120) + 68);
    w_u32(character_offset + 552, value);
    w_u32(horizontal_offset, r_u32(horizontal_offset) + acceleration);
    if ((r_u32(object_offset + 28) & 0x400) != 0)
        value = r_u32(character_offset + 748);
    else
        value = r_u32(character_offset + 740);
    lower = 0 - value;
    sum = r_u32(horizontal_offset);
    upper = value + acceleration;
    result = (sint32)upper < (sint32)sum;
    lower += acceleration;
    if (result != 0)
        w_u32(horizontal_offset, upper);
    else
    {
        result = (sint32)sum < (sint32)lower;
        if (result != 0)
            w_u32(horizontal_offset, lower);
    }
    return result;
}

uint32 p2_func_80056340(uint32 definition_offset, sint32 frame, uint32 destination_offset, uint32 direction)
{
    uint32 matrix, saved_stack, vector, x, y, z, result;
    if (direction == 0)
        frame = (sint32)((uint32)(sint32)r_s16(definition_offset + 26) - (uint32)frame - 1);
    matrix = r_u32(definition_offset + 40);
    if (matrix == 0)
        return p2_func_80017364(frame, definition_offset + 20, destination_offset);
    saved_stack = p2_guest_stack_pointer;
    p2_guest_stack_pointer -= 16;
    vector = p2_guest_stack_pointer;
    p2_func_80017364(frame, definition_offset + 20, vector);
    PushMatrix();
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
    p2_sdk_8007ec64(vector, destination_offset);
    PopMatrix();
    x = r_u32(destination_offset);
    x += r_u32(matrix + 20);
    w_u32(destination_offset, x);
    y = r_u32(destination_offset + 4);
    y += r_u32(matrix + 24);
    w_u32(destination_offset + 4, y);
    result = r_u32(destination_offset + 8);
    result += r_u32(matrix + 28);
    w_u32(destination_offset + 8, result);
    p2_guest_stack_pointer = saved_stack;
    return result;
}

sint32 p2_func_8005642c(sint32 first_frame, sint32 second_frame, uint32 definition_offset)
{
    uint32 saved_stack, first, second, local, sample, destination, matrix, x, y, z, dx, dy, dz, result;
    sint32 frame;
    saved_stack = p2_guest_stack_pointer;
    p2_guest_stack_pointer -= 48;
    first = p2_guest_stack_pointer;
    second = first + 16;
    local = first + 32;
    for (sample = 0; sample < 2; ++sample)
    {
        frame = sample == 0 ? first_frame : second_frame;
        destination = sample == 0 ? first : second;
        matrix = r_u32(definition_offset + 40);
        if (matrix != 0)
        {
            p2_func_80017364(frame, definition_offset + 20, local);
            PushMatrix();
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
            p2_sdk_8007ec64(local, destination);
            PopMatrix();
            x = r_u32(destination);
            x += r_u32(matrix + 20);
            w_u32(destination, x);
            y = r_u32(destination + 4);
            y += r_u32(matrix + 24);
            w_u32(destination + 4, y);
            z = r_u32(destination + 8);
            z += r_u32(matrix + 28);
            w_u32(destination + 8, z);
        }
        else
            p2_func_80017364(frame, definition_offset + 20, destination);
    }
    x = r_u32(second);
    dx = x - r_u32(first);
    y = r_u32(second + 4);
    dy = y - r_u32(first + 4);
    z = r_u32(second + 8);
    dz = z - r_u32(first + 8);
    w_u32(0x800B14A0, dx);
    w_u32(0x800B14A4, dy);
    w_u32(0x800B14A8, dz);
    if (dx == 0)
        result = (sint32)dz >= 0 ? 2048 : 0;
    else
        result = p2_func_8005dc00((sint32)(0 - dz), (sint32)(0 - dx));
    p2_guest_stack_pointer = saved_stack;
    return (sint16)result;
}

uint32 p2_func_80068198(void)
{
    uint32 seed, high, quotient, remainder, result;
    seed = r_u32(0x8008A298);
    high = (uint32)(((uint64)seed * 0x069C16BD) >> 32);
    quotient = (high + ((seed - high) >> 1)) >> 16;
    remainder = seed - quotient * 127773;
    result = remainder * 16807 - quotient * 2836;
    w_u32(0x8008A298, result);
    if ((sint32)result < 0)
    {
        result -= 0x7FFFFFFF;
        w_u32(0x8008A298, result);
    }
    else if (result == 0)
    {
        result = 1;
        w_u32(0x8008A298, result);
    }
    return result;
}

uint32 p2_func_80033d7c(uint32 object_offset)
{
    uint32 parent, data, flags, kind_offset, count, table, resource, node, value, x, y, z;
    uint32 saved_stack, first, second, result, target;
    sint32 kind, frame;
    parent = r_u32(object_offset + 132);
    flags = r_u32(object_offset + 28);
    data = r_u32(parent + 44);
    parent = r_u32(object_offset + 132);
    w_u32(object_offset + 28, (flags & 0x01C08024) | 0x20010);
    kind_offset = r_u32(parent + 44);
    count = (uint32)(sint32)r_s8(0x8008A330);
    kind = r_s8(kind_offset + 37);
    resource = 0;
    table = 0x8008B450;
    while (count != 0)
    {
        node = r_u32(table);
        value = (uint32)(sint32)r_s8(node + 61);
        --count;
        if ((sint32)value == kind)
        {
            resource = node;
            break;
        }
        table += 4;
    }
    value = r_u32(resource + 72);
    w_u16(object_offset + 130, 0xFFFF);
    w_u8(object_offset + 124, 4);
    w_u8(object_offset + 126, 2);
    w_u8(object_offset + 127, 0);
    w_u32(object_offset + 104, value);
    value = r_u8(resource + 60);
    parent = r_u32(object_offset + 132);
    w_u16(object_offset + 128, (sint8)value);
    if (r_u32(parent + 32) != 0)
    {
        value = (uint32)p2_func_8005642c(0, 1, parent);
        w_u16(object_offset + 150, value);
        w_u16(object_offset + 148, value);
        w_u16(data + 88, value);
        saved_stack = p2_guest_stack_pointer;
        p2_guest_stack_pointer -= 32;
        first = p2_guest_stack_pointer;
        second = first + 16;
        parent = r_u32(object_offset + 132);
        p2_func_80056340(parent, 0, first, 64);
        parent = r_u32(object_offset + 132);
        p2_func_80056340(parent, 1, second, 64);
        x = r_u32(second);
        x -= r_u32(first);
        y = r_u32(second + 4);
        y -= r_u32(first + 4);
        z = r_u32(second + 8);
        z -= r_u32(first + 8);
        w_u32(data + 92, x);
        w_u32(data + 96, y);
        w_u32(data + 100, z);
        p2_guest_stack_pointer = saved_stack;
    }
    else
    {
        w_u16(object_offset + 148, 0);
        w_u16(data + 88, 0);
        w_u32(data + 100, 0);
        w_u32(data + 96, 0);
        w_u32(data + 92, 0);
        w_u32(data + 100, 4096);
    }
    if (r_s16(resource + 652) == 0)
        w_u16(object_offset + 148, p2_func_80068198() & 0xFFF);
    parent = r_u32(object_offset + 132);
    if (r_u32(parent + 32) != 0)
    {
        value = r_u32(parent + 44);
        node = r_u32(object_offset + 132);
        value = r_u16(value + 24);
        frame = (sint16)value;
        w_u16(object_offset + 136, value);
        p2_func_80017364(frame, node + 20, object_offset + 16);
    }
    else
    {
        node = r_u32(parent + 40);
        if (node == 0)
            node = parent;
        x = r_u32(node + 20);
        y = r_u32(node + 24);
        z = r_u32(node + 28);
        w_u32(object_offset + 16, x);
        w_u32(object_offset + 20, y);
        w_u32(object_offset + 24, z);
    }
    value = r_u8(object_offset + 13);
    w_u32(object_offset + 220, 0xFFFFFFFF);
    w_u32(object_offset + 204, 0xFFFFFFFF);
    w_u32(object_offset + 188, 0xFFFFFFFF);
    w_u32(object_offset + 172, 0xFFFFFFFF);
    w_u8(object_offset + 13, (value & 0x7D) | 0x40);
    if (r_s8(resource + 627) != 0)
        w_u32(object_offset + 228, r_u32(object_offset + 228) | 0x4000);
    value = r_u8(data + 38);
    w_u32(data + 76, 0);
    w_u32(data + 84, 0);
    w_u8(data + 180, 0);
    w_u32(data + 204, 0);
    w_u32(data + 200, 0);
    w_u32(data + 196, 0);
    w_u8(data + 38, value & 0xE5);
    p2_func_800352f4(object_offset, data);
    target = r_u32(resource);
    result = p2_dispatch_object_factory(target, object_offset);
    return result;
}

uint32 p2_func_800352f4(uint32 object_offset, uint32 data_offset)
{
    uint32 resource, result, parent, saved_stack, first, second, delta, x, y, z;
    resource = r_u32(r_u32(r_u32(object_offset + 132) + 44) + 72);
    w_u16(object_offset + 148, 0);
    if (r_u32(resource + 144) == 0)
    {
        w_u8(data_offset + 124, 0xFE);
        return 0xFFFFFFFE;
    }
    p2_func_800378a0(object_offset);
    result = (uint32)(sint32)r_s16(resource + 652);
    if (result != 0)
        return result;
    parent = r_u32(object_offset + 132);
    result = r_u32(parent + 32);
    if (result == 0)
        return result;
    saved_stack = p2_guest_stack_pointer;
    p2_guest_stack_pointer -= 48;
    first = p2_guest_stack_pointer;
    second = first + 16;
    delta = first + 32;
    p2_func_80056340(parent, 0, first, 64);
    parent = r_u32(object_offset + 132);
    p2_func_80056340(parent, 1, second, 64);
    x = r_u32(second);
    x -= r_u32(first);
    y = r_u32(second + 4);
    y -= r_u32(first + 4);
    z = r_u32(second + 8);
    z -= r_u32(first + 8);
    w_u32(delta, x);
    w_u32(delta + 4, y);
    w_u32(delta + 8, z);
    result = p2_func_80040384(object_offset, delta, r_s16(resource + 68));
    p2_guest_stack_pointer = saved_stack;
    return result;
}

uint32 p2_func_8002afa8(uint32 object_offset)
{
    uint32 parent, result;
    if (r_u32(0x800BA39C) == 0)
        w_u32(0x800BA39C, 0x8002B000);
    parent = r_u32(object_offset + 132);
    w_u8(object_offset + 125, 7);
    result = r_u32(parent + 32);
    if (result != 0)
    {
        result = r_u32(object_offset + 28) | 0x40;
        w_u32(object_offset + 28, result);
    }
    return result;
}

uint32 p2_func_800343f0(uint32 object_offset)
{
    uint32 saved_stack, position, local, object, definition, matrix, scene, x, y, z;
    w_u32(object_offset + 12, 0x7FFFF000);
    if (p2_func_800536f0(0) == 0)
        return r_u32(object_offset + 12);
    saved_stack = p2_guest_stack_pointer;
    p2_guest_stack_pointer -= 32;
    position = p2_guest_stack_pointer;
    local = position + 16;
    object = r_u32(object_offset + 4);
    if (object != 0)
    {
        definition = r_u32(object_offset + 60);
        if (definition != 0 && r_u32(definition + 40) != 0)
        {
            x = r_u32(object + 16);
            y = r_u32(object + 20);
            z = r_u32(object + 24);
            w_u32(local, x);
            w_u32(local + 4, y);
            w_u32(local + 8, z);
            PushMatrix();
            matrix = r_u32(r_u32(object_offset + 60) + 40);
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
            p2_sdk_8007ec64(local, position);
            PopMatrix();
            matrix = r_u32(r_u32(object_offset + 60) + 40);
            x = r_u32(position);
            x += r_u32(matrix + 20);
            w_u32(position, x);
            matrix = r_u32(r_u32(object_offset + 60) + 40);
            y = r_u32(position + 4);
            y += r_u32(matrix + 24);
            w_u32(position + 4, y);
            matrix = r_u32(r_u32(object_offset + 60) + 40);
            z = r_u32(position + 8);
            z += r_u32(matrix + 28);
            w_u32(position + 8, z);
        }
        else
        {
            object = r_u32(object_offset + 4);
            x = r_u32(object + 16);
            y = r_u32(object + 20);
            z = r_u32(object + 24);
            w_u32(position, x);
            w_u32(position + 4, y);
            w_u32(position + 8, z);
        }
    }
    else
    {
        definition = r_u32(object_offset + 60);
        if (r_u32(definition + 32) != 0)
            p2_func_800230f0(definition, r_s16(object_offset + 24), position, 1);
        else
        {
            object = r_u32(definition + 40);
            if (object == 0)
                object = definition;
            x = r_u32(object + 20);
            y = r_u32(object + 24);
            z = r_u32(object + 28);
            w_u32(position, x);
            w_u32(position + 4, y);
            w_u32(position + 8, z);
        }
    }
    scene = r_u32(0x8008A114);
    x = r_u32(position);
    x -= r_u32(scene + 16);
    y = r_u32(position + 4);
    y -= r_u32(scene + 20);
    z = r_u32(position + 8);
    z -= r_u32(scene + 24);
    x = (uint32)((sint32)x >> 12);
    y = (uint32)((sint32)y >> 12);
    z = (uint32)((sint32)z >> 12);
    w_u32(object_offset + 12, x * x + y * y + z * z);
    p2_guest_stack_pointer = saved_stack;
    return r_u32(object_offset + 12);
}

uint32 p2_func_8002b000(uint32 object_offset)
{
    uint32 saved_stack, first, second, third, resource, result;
    saved_stack = p2_guest_stack_pointer;
    p2_guest_stack_pointer -= 48;
    first = p2_guest_stack_pointer;
    second = first + 16;
    third = first + 32;
    p2_func_80030dcc(object_offset, first, second, third);
    resource = r_u32(r_u32(r_u32(object_offset + 132) + 44) + 72);
    result = p2_func_80032248(object_offset, 0, first, resource);
    p2_guest_stack_pointer = saved_stack;
    return result;
}

uint32 p2_func_80033564(uint32 object_offset)
{
    uint32 saved_stack, position, local, output, data, resource, parent, matrix, flags, value, result;
    uint32 x, y, z, initial, scene, counter, eligible, distance, height, speed_table, speed_pointer, target, acceleration;
    sint32 kind;
    saved_stack = p2_guest_stack_pointer;
    p2_guest_stack_pointer -= 48;
    position = p2_guest_stack_pointer;
    local = position + 16;
    output = position + 32;
    parent = r_u32(object_offset + 132);
    data = r_u32(parent + 44);
    value = r_u16(object_offset + 128);
    resource = r_u32(data + 72);
    initial = value == 0;
    if (initial != 0)
    {
        w_u32(object_offset + 144, 15);
        kind = r_s8(resource + 633);
        if (kind == -1)
            kind = r_s8(data + 37);
        if (kind < 44)
        {
            value = r_u32(data + 68);
            x = (uint32)(sint32)r_s16(data + 66);
            result = p2_func_8005e868(value, x);
            if ((result << 24) != 0)
                p2_func_8006a970(r_u16(data + 66));
            w_u32(data + 68, 0);
            w_u16(data + 66, 0xFFFF);
        }
        else
        {
            value = r_u32(r_u32(data + 72) + 76);
            w_u32(object_offset + 104, value);
            x = (uint32)(sint32)r_s16(value + 6);
            value = r_u32(object_offset + 144);
            w_u16(object_offset + 130, 0xFFFF);
            w_u32(object_offset + 144, value + x);
        }
        flags = r_u8(data + 38);
        if ((flags & 2) != 0)
        {
            value = r_u32(data + 72);
            w_u8(data + 38, flags ^ 2);
            value = r_u32(value + 96);
            flags = r_u32(0x8008A178);
            w_u16(object_offset + 130, 0xFFFF);
            w_u32(0x8008A178, flags & 0xFFFFFFF3);
            w_u32(object_offset + 92, 4096);
            w_u32(object_offset + 104, value);
            p2_func_80027b1c(object_offset, 0, 5);
            w_u32(object_offset + 144, (uint32)(sint32)r_s16(r_u32(object_offset + 104) + 6) + 15);
        }
        kind = r_s8(object_offset + 125);
        w_u8(object_offset + 124, 0);
        w_u8(object_offset + 126, 0);
        value = r_u32(0x800BA380 + ((uint32)kind << 2));
        w_u8(object_offset + 127, value == 0x80020C6C ? 0 : 2);
        value = r_u8(object_offset + 13);
        w_u32(object_offset + 92, 4096);
        w_u8(object_offset + 13, value & 0xFE);
        goto world_position;
    }
    value = r_u32(data + 44);
    flags = r_u8(data + 38);
    w_u32(data + 44, value & 0x80);
    w_u16(data + 28, r_u16(object_offset + 136));
    result = flags & 8;
    if ((flags & 2) != 0)
        goto done;
    if (result != 0)
    {
        w_u32(object_offset + 100, 0);
        w_u32(object_offset + 96, 0);
        w_u32(object_offset + 92, 0);
    }
    if ((r_u8(data + 38) & 0x10) == 0)
        goto movement;
world_position:
    parent = r_u32(object_offset + 132);
    if (parent != 0 && r_u32(parent + 40) != 0)
    {
        x = r_u32(object_offset + 16);
        y = r_u32(object_offset + 20);
        z = r_u32(object_offset + 24);
        w_u32(local, x);
        w_u32(local + 4, y);
        w_u32(local + 8, z);
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
        p2_sdk_8007ec64(local, position);
        PopMatrix();
        matrix = r_u32(r_u32(object_offset + 132) + 40);
        x = r_u32(position);
        x += r_u32(matrix + 20);
        w_u32(position, x);
        matrix = r_u32(r_u32(object_offset + 132) + 40);
        y = r_u32(position + 4);
        y += r_u32(matrix + 24);
        w_u32(position + 4, y);
        matrix = r_u32(r_u32(object_offset + 132) + 40);
        z = r_u32(position + 8);
        z += r_u32(matrix + 28);
        w_u32(position + 8, z);
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
    if (initial != 0)
    {
        x = r_u32(object_offset + 60);
        x -= r_u32(position);
        w_u32(object_offset + 160, x);
        y = r_u32(object_offset + 64);
        y -= r_u32(position + 4);
        w_u32(object_offset + 164, y);
        z = r_u32(object_offset + 68);
        z -= r_u32(position + 8);
        w_u32(object_offset + 168, z);
        result = r_u32(data + 8);
        if (result != 0)
        {
            scene = r_u32(0x8008A114);
            counter = r_u8(scene + 75);
            w_u8(scene + 75, counter + 1);
            value = r_u32(data + 8);
            result = scene + ((uint32)(sint32)(sint8)counter << 2);
            w_u32(result + 76, (value & 0xFFF) | 0x4000);
        }
        goto done;
    }
    x = r_u32(position + 4);
    value = r_u32(data + 84);
    value = x - value;
    if (value == 0)
    {
        w_u8(data + 38, r_u8(data + 38) & 0xEF);
        w_u32(object_offset + 96, 0);
    }
    else
    {
        value = (sint32)value >= 4097 ? 0xFFFFF000 : 0 - value;
        w_u32(object_offset + 96, value);
        flags = r_u32(object_offset + 28);
        height = r_u32(object_offset + 96);
        w_u32(object_offset + 28, flags | 0x400);
        p2_func_8003db7c(object_offset, 0, (sint32)height, 0);
        w_u32(object_offset + 28, r_u32(object_offset + 28) & 0xFFFFFBFF);
    }
movement:
    result = r_u8(data + 38) & 8;
    if (result != 0)
        goto done;
    result = r_u32(object_offset + 28) & 0x02000040;
    eligible = result == 0x40;
    if (result == 0x02000040)
    {
        value = (uint32)(sint32)r_s16(resource + 636);
        result = 1;
        if (value == 1)
        {
            result = (uint32)(sint32)r_s16(object_offset + 128);
            if (result != 0)
                eligible = 1;
        }
    }
    if (eligible == 0)
        goto zero_velocity;
    result = r_u32(r_u32(object_offset + 132) + 32);
    if (result != 0 && r_s8(data + 180) != 0)
    {
        w_u32(object_offset + 28, r_u32(object_offset + 28) & 0xFFFFF7FF);
        result = p2_func_80034ba8(object_offset, data, resource);
        if (result == 0)
            goto zero_velocity;
        result = p2_func_8003db7c(object_offset, (sint32)result, r_s16(data + 30), 0);
        goto done;
    }
    parent = r_u32(object_offset + 132);
    result = r_u32(parent + 32);
    if (result == 0)
        goto zero_velocity;
    if ((r_u32(parent + 16) & 0x40) == 0)
    {
        kind = r_s8(resource + 633);
        result = 19;
        if (kind == -1)
        {
            if (r_s8(data + 37) != 19)
                goto zero_velocity;
        }
        else if (kind != 19)
            goto zero_velocity;
    }
    w_u32(object_offset + 28, r_u32(object_offset + 28) & 0xFFFFF7FF);
    kind = r_s8(resource + 633);
    if (kind == -1)
        kind = r_s8(data + 37);
    if (kind == 10)
        distance = r_u32(data + 16);
    else
    {
        speed_table = r_u32(data + 72);
        flags = r_u32(object_offset + 28);
        speed_pointer = speed_table + ((flags & 0x08000000) != 0 ? 32 : 28);
        value = r_u16(data + 120);
        target = r_u32(speed_pointer);
        if ((value & 0x100) != 0)
            distance = r_u32(data + 168);
        else
        {
            acceleration = r_u32(speed_table + 36);
            distance = target;
            if (acceleration != 0)
            {
                distance = r_u32(data + 16);
                if ((sint32)distance < (sint32)target)
                {
                    distance += acceleration;
                    if ((sint32)target < (sint32)distance)
                        distance = target;
                }
            }
        }
        w_u32(data + 16, distance);
    }
    result = r_u16(data + 122) & 0x200;
    w_u16(data + 176, 0);
    if (result != 0)
        p2_func_800378c8(object_offset, distance, output, output + 4, output + 8);
    else
    {
        result = r_u16(data + 120) & 0x800;
        if (result == 0)
            goto direct_movement;
        result = r_u8(data + 38) & 8;
        if (result != 0)
            goto direct_movement;
        p2_func_80037c50(object_offset, distance, output, output + 4, output + 8);
    }
    value = r_u32(output + 4);
    w_u16(data + 30, value);
    result = (uint32)(sint32)r_s16(resource + 642);
    distance = r_u32(output);
    if (result != 0)
    {
        result = r_u16(output + 8);
        w_u16(data + 176, result);
    }
    else
        w_u16(data + 176, 0);
    if (distance == 0)
        goto zero_velocity;
    w_u32(object_offset + 28, r_u32(object_offset + 28) | 0x400);
    p2_func_8003db7c(object_offset, (sint32)distance, r_s16(data + 30), 0);
    result = r_u32(object_offset + 28) & 0xFFFFFBFF;
    w_u32(object_offset + 28, result);
    goto done;
direct_movement:
    if (distance == 0)
        goto zero_velocity;
    result = p2_func_8003db7c(object_offset, (sint32)distance, r_s16(data + 30), 0);
    goto done;
zero_velocity:
    w_u32(object_offset + 100, 0);
    w_u32(object_offset + 96, 0);
    w_u32(object_offset + 92, 0);
done:
    p2_guest_stack_pointer = saved_stack;
    return result;
}

uint32 p2_func_800534ec(void)
{
    return r_u32(0x800ABD70);
}

uint32 p2_func_80030a34(uint32 object_offset)
{
    uint32 parent, data, resource, animation;
    parent = r_u32(object_offset + 132);
    if (parent == 0)
        return 0;
    data = r_u32(parent + 44);
    if ((r_u8(data + 38) & 2) == 0)
        return 0;
    resource = r_u32(data + 72);
    animation = r_u32(object_offset + 104);
    return animation == r_u32(resource + 88) ? 5324 : 5734;
}

uint32 p2_func_80034eac(uint32 object_offset)
{
    uint32 data, count, kind, table, node, result;
    data = r_u32(r_u32(object_offset + 132) + 44);
    count = (uint32)(sint32)r_s8(0x8008A330);
    kind = r_u8(data + 37);
    result = 0;
    table = 0x8008B450;
    while (count != 0)
    {
        node = r_u32(table);
        if ((sint32)r_s8(node + 61) == (sint32)kind)
        {
            result = node;
            break;
        }
        --count;
        table += 4;
    }
    return r_u8(result + 622);
}

void p2_func_8005927c(void)
{
}

uint32 p2_func_80018144(uint32 object_offset, uint32 vector_offset, uint32 axes_offset)
{
    uint32 saved_stack, transformed, quaternion, matrix, parent_quaternion, composed, parent, track, source;
    uint32 index, address, shift, mask, upper, lower, first_word, second_word, value, destination, old, result;
    uint32 xx, yy, zz, xy, xz, yz, wx, wy, wz, x, y, z;
    sint32 frame, q0, q1, q2, q3, p0, p1, p2, p3;
    saved_stack = p2_guest_stack_pointer;
    p2_guest_stack_pointer -= 64;
    transformed = p2_guest_stack_pointer;
    quaternion = axes_offset != 0 ? axes_offset : transformed + 16;
    matrix = transformed + 24;
    parent_quaternion = transformed + 48;
    composed = transformed + 56;
    parent = r_u32(object_offset + 132);
    if (parent != 0 && (r_u32(parent + 16) & 8) != 0)
    {
        track = r_u32(r_u32(parent + 44) + 56);
        if (track != 0)
        {
            if ((r_u32(object_offset + 28) & 0x10) != 0)
                frame = r_s16(object_offset + 136);
            else
            {
                frame = r_s16(parent + 26);
                frame = (sint32)((uint32)frame - (uint32)(sint32)r_s16(object_offset + 136) - 1);
            }
            p2_func_80017938(frame, track, quaternion);
            parent = r_u32(r_u32(object_offset + 132) + 40);
            if (parent != 0)
            {
                p2_func_80017e4c(parent, parent_quaternion);
                q0 = r_s16(quaternion);
                source = 0;
                if (q0 == 0x4000)
                    source = parent_quaternion;
                else
                {
                    p0 = r_s16(parent_quaternion);
                    if (p0 == 0x4000)
                        source = quaternion;
                    else
                    {
                        q1 = r_s16(quaternion + 2);
                        p1 = r_s16(parent_quaternion + 2);
                        q2 = r_s16(quaternion + 4);
                        p2 = r_s16(parent_quaternion + 4);
                        q3 = r_s16(quaternion + 6);
                        p3 = r_s16(parent_quaternion + 6);
                        x = (uint32)(q0 * p0) - ((uint32)(q1 * p1) + (uint32)(q2 * p2) + (uint32)(q3 * p3));
                        y = (uint32)(q0 * p1) + (uint32)(p0 * q1) + (uint32)(q2 * p3) - (uint32)(q3 * p2);
                        z = (uint32)(q0 * p2) + (uint32)(p0 * q2) + (uint32)(q3 * p1) - (uint32)(q1 * p3);
                        value = (uint32)(q0 * p3) + (uint32)(p0 * q3) + (uint32)(q1 * p2) - (uint32)(q2 * p1);
                        w_u16(composed, x >> 14);
                        w_u16(composed + 2, y >> 14);
                        w_u16(composed + 4, z >> 14);
                        w_u16(composed + 6, (uint32)((sint32)value >> 14));
                    }
                }
                if (source != 0)
                {
                    /* Preserve both aligned reads of every LWL/LWR pair */
                    for (index = 0; index < 2; ++index)
                    {
                        address = source + index * 4;
                        upper = r_u32((address + 3) & 0xFFFFFFFC);
                        lower = r_u32(address & 0xFFFFFFFC);
                        shift = (address & 3) * 8;
                        value = shift == 0 ? lower : (upper << (32 - shift)) | (lower >> shift);
                        if (index == 0)
                            first_word = value;
                        else
                            second_word = value;
                    }
                    for (index = 0; index < 2; ++index)
                    {
                        value = index == 0 ? first_word : second_word;
                        destination = composed + index * 4;
                        shift = ((destination + 3) & 3) * 8;
                        mask = 0xFFFFFFFF >> (24 - shift);
                        old = r_u32((destination + 3) & 0xFFFFFFFC);
                        w_u32((destination + 3) & 0xFFFFFFFC, (old & ~mask) | (value >> (24 - shift)));
                        shift = (destination & 3) * 8;
                        mask = 0xFFFFFFFF << shift;
                        old = r_u32(destination & 0xFFFFFFFC);
                        w_u32(destination & 0xFFFFFFFC, (old & ~mask) | (value << shift));
                    }
                }
                first_word = r_u32(composed);
                first_word = r_u32(composed);
                second_word = r_u32(composed + 4);
                second_word = r_u32(composed + 4);
                for (index = 0; index < 2; ++index)
                {
                    /* Merge SWL then SWR without changing neighboring bytes */
                    value = index == 0 ? first_word : second_word;
                    destination = quaternion + index * 4;
                    shift = ((destination + 3) & 3) * 8;
                    mask = 0xFFFFFFFF >> (24 - shift);
                    old = r_u32((destination + 3) & 0xFFFFFFFC);
                    w_u32((destination + 3) & 0xFFFFFFFC, (old & ~mask) | (value >> (24 - shift)));
                    shift = (destination & 3) * 8;
                    mask = 0xFFFFFFFF << shift;
                    old = r_u32(destination & 0xFFFFFFFC);
                    w_u32(destination & 0xFFFFFFFC, (old & ~mask) | (value << shift));
                }
            }
            q1 = r_s16(quaternion + 2);
            q2 = r_s16(quaternion + 4);
            q3 = r_s16(quaternion + 6);
            q0 = r_s16(quaternion);
            xx = (uint32)(q1 * q1);
            yy = (uint32)(q2 * q2);
            zz = (uint32)(q3 * q3);
            xy = (uint32)(q1 * q2);
            xz = (uint32)(q1 * q3);
            yz = (uint32)(q2 * q3);
            wz = (uint32)(q0 * q3);
            wy = (uint32)(q0 * q2);
            wx = (uint32)(q0 * q1);
            w_u16(matrix, (uint32)((sint32)((uint32)0x08000000 - yy - zz) >> 15));
            w_u16(matrix + 8, (uint32)((sint32)((uint32)0x08000000 - xx - zz) >> 15));
            w_u16(matrix + 16, (uint32)((sint32)((uint32)0x08000000 - xx - yy) >> 15));
            w_u16(matrix + 2, (uint32)((sint32)(xy - wz) >> 15));
            w_u16(matrix + 6, (uint32)((sint32)(xy + wz) >> 15));
            w_u16(matrix + 4, (uint32)((sint32)(xz + wy) >> 15));
            w_u16(matrix + 12, (uint32)((sint32)(xz - wy) >> 15));
            w_u16(matrix + 10, (uint32)((sint32)(yz - wx) >> 15));
            w_u16(matrix + 14, (uint32)((sint32)(yz + wx) >> 15));
            PushMatrix();
            x = r_u32(matrix);
            y = r_u32(matrix + 4);
            xport_gte_write_control(0, x);
            xport_gte_write_control(1, y);
            x = r_u32(matrix + 8);
            y = r_u32(matrix + 12);
            z = r_u16(matrix + 16);
            xport_gte_write_control(2, x);
            xport_gte_write_control(3, y);
            xport_gte_write_control(4, z);
            p2_sdk_8007ec64(vector_offset, transformed);
            PopMatrix();
            vector_offset = transformed;
        }
    }
    if (r_u32(vector_offset) + 32 >= 65 || r_u32(vector_offset + 8) + 32 >= 65)
    {
        z = r_u32(vector_offset + 8);
        x = r_u32(vector_offset);
        result = (uint32)(sint32)(sint16)p2_func_8005dc00((sint32)(0 - z), (sint32)(0 - x));
    }
    else
        result = 4096;
    p2_guest_stack_pointer = saved_stack;
    return result;
}
