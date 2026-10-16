#include "game.h"

uint32 p2_func_800231dc(uint32 definition_offset, uint32 position_offset, uint32 direction, sint32 height_offset)
{
    return (uint32)p2_query_path_height_with_selection(definition_offset, position_offset, direction, height_offset, 0).value;
}

P2PathProbeResult p2_query_path_height_with_selection(uint32 definition_offset, uint32 position_offset, uint32 direction, sint32 height_offset, uint32 fallback_frame)
{
    P2PathProbeResult output, probe;
    uint32 saved_stack, point, left, right, middle, raw, query, flags, matrix, destination, index;
    uint32 x, y, z, a, b, c, d, dx, dy, dz, distance, left_distance, right_distance;
    uint32 first_distance, best_distance, radius, span, forward, outside, unresolved, move_left;
    sint32 best, remaining, frame, low, high, candidate, ca, cb, c0, c1, c2, c3, c4, c5, c6, c7, c8;
    sint64 product;
    if (r_u32(definition_offset + 32) == 0)
    {
        output.value = 0;
        output.fallback_frame = fallback_frame;
        return output;
    }
    flags = r_u32(definition_offset + 16);
    if ((flags & 0x10000) != 0)
        return p2_query_link_height_with_selection(definition_offset, position_offset, direction, height_offset, fallback_frame);
    saved_stack = p2_guest_stack_pointer;
    p2_guest_stack_pointer -= 0x60;
    point = p2_guest_stack_pointer;
    left = point + 16;
    right = point + 32;
    middle = point + 48;
    raw = point + 64;
    query = point + 80;
    best = -1;
    best_distance = 0x7FFFF000;
    remaining = r_s16(definition_offset + 26);
    frame = 0;
    if (height_offset >= 0)
    {
        radius = (uint32)height_offset;
        span = radius << 1;
        matrix = r_u32(definition_offset + 40);
        if (matrix != 0)
        {
            c0 = r_s16(matrix);
            c1 = r_s16(r_u32(definition_offset + 40) + 6);
            c2 = r_s16(r_u32(definition_offset + 40) + 12);
            c3 = r_s16(r_u32(definition_offset + 40) + 2);
            c4 = r_s16(r_u32(definition_offset + 40) + 8);
            c5 = r_s16(r_u32(definition_offset + 40) + 14);
            c6 = r_s16(r_u32(definition_offset + 40) + 4);
            c7 = r_s16(r_u32(definition_offset + 40) + 10);
            c8 = r_s16(r_u32(definition_offset + 40) + 16);
            x = 0u - r_u32(r_u32(definition_offset + 40) + 20);
            y = 0u - r_u32(r_u32(definition_offset + 40) + 24);
            z = 0u - r_u32(r_u32(definition_offset + 40) + 28);
            x += r_u32(position_offset);
            y += r_u32(position_offset + 4);
            z += r_u32(position_offset + 8);
            product = (sint64)(sint32)x * c0;
            a = (uint32)((uint64)product >> 12);
            product = (sint64)(sint32)y * c1;
            fallback_frame = (uint32)((uint64)product >> 32) << 20;
            a += (uint32)((uint64)product >> 12);
            product = (sint64)(sint32)z * c2;
            a += (uint32)((uint64)product >> 12);
            w_u32(query, a);
            product = (sint64)(sint32)x * c3;
            a = (uint32)((uint64)product >> 12);
            product = (sint64)(sint32)y * c4;
            a += (uint32)((uint64)product >> 12);
            product = (sint64)(sint32)z * c5;
            a += (uint32)((uint64)product >> 12);
            w_u32(query + 4, a);
            product = (sint64)(sint32)x * c6;
            a = (uint32)((uint64)product >> 12);
            product = (sint64)(sint32)y * c7;
            a += (uint32)((uint64)product >> 12);
            product = (sint64)(sint32)z * c8;
            a += (uint32)((uint64)product >> 12);
            w_u32(query + 8, a);
            position_offset = query;
        }
        do
        {
            probe = p2_sample_track_with_selection(frame, definition_offset + 20, point, fallback_frame);
            fallback_frame = probe.fallback_frame;
            dx = r_u32(point) - r_u32(position_offset);
            w_u32(point, dx);
            if (dx + radius < span)
            {
                dy = r_u32(point + 4) - r_u32(position_offset + 4);
                w_u32(point + 4, dy);
                if (dy + radius < span)
                {
                    dz = r_u32(point + 8) - r_u32(position_offset + 8);
                    w_u32(point + 8, dz);
                    if (dz + radius < span)
                    {
                        a = (uint32)((sint32)dx >> 6);
                        b = (uint32)((sint32)dy >> 6);
                        c = (uint32)((sint32)dz >> 6);
                        distance = a * a + b * b + c * c;
                        if ((sint32)distance < (sint32)best_distance)
                        {
                            best_distance = distance;
                            best = frame;
                        }
                    }
                }
            }
            remaining = (sint32)((uint32)remaining - 1);
            frame = (sint32)((uint32)frame + 1);
        } while (remaining > 0);
        if (best >= 0 && direction == 0)
            best = (sint32)((uint32)(sint32)r_s16(definition_offset + 26) - (uint32)best - 1);
        goto finish;
    }
    radius = (uint32)(sint32)r_s16(definition_offset + 38) << 5;
    span = radius << 1;
    if ((flags & 2) != 0)
    {
        first_distance = 0x7FFFF000;
        matrix = r_u32(definition_offset + 40);
        if (matrix != 0)
        {
            probe = p2_sample_track_with_selection(0, definition_offset + 20, raw, fallback_frame);
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
            p2_sdk_8007ec64(raw, left);
            PopMatrix();
            /* TODO: Audit PopMatrix diagnostic exits for the secondary output */
            fallback_frame = xport_gte_read_translation(0);
            w_u32(left, r_u32(left) + r_u32(matrix + 20));
            w_u32(left + 4, r_u32(left + 4) + r_u32(matrix + 24));
            w_u32(left + 8, r_u32(left + 8) + r_u32(matrix + 28));
        }
        else
        {
            probe = p2_sample_track_with_selection(0, definition_offset + 20, left, fallback_frame);
            fallback_frame = probe.fallback_frame;
        }
        for (;;)
        {
            matrix = r_u32(definition_offset + 40);
            if (matrix != 0)
            {
                probe = p2_sample_track_with_selection(frame, definition_offset + 20, raw, fallback_frame);
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
                p2_sdk_8007ec64(raw, point);
                PopMatrix();
                /* TODO: Audit PopMatrix diagnostic exits for the secondary output */
                fallback_frame = xport_gte_read_translation(0);
                w_u32(point, r_u32(point) + r_u32(matrix + 20));
                w_u32(point + 4, r_u32(point + 4) + r_u32(matrix + 24));
                w_u32(point + 8, r_u32(point + 8) + r_u32(matrix + 28));
            }
            else
            {
                probe = p2_sample_track_with_selection(frame, definition_offset + 20, point, fallback_frame);
                fallback_frame = probe.fallback_frame;
            }
            dx = r_u32(point) - r_u32(position_offset);
            dy = r_u32(point + 4) - r_u32(position_offset + 4);
            dz = r_u32(point + 8) - r_u32(position_offset + 8);
            if (frame == 0)
            {
            repeat_last_point:
                if ((sint32)(dy - 40960) <= 0 || (r_u32(definition_offset + 16) & 0x11) != 0)
                    dy = 0;
            }
            if (dx + radius < span && dz + radius < span && ((sint32)(dy - 40960) <= 0 || (r_u32(definition_offset + 16) & 0x11) != 0))
            {
                x = r_u32(left);
                z = r_u32(left + 8);
                a = r_u32(point);
                b = r_u32(point + 8);
                c = r_u32(position_offset);
                fallback_frame = c;
                d = r_u32(position_offset + 8);
                product = (sint64)(sint32)(x - c) * (sint32)(x - a);
                x = (uint32)((uint64)product >> 12);
                product = (sint64)(sint32)(z - d) * (sint32)(b - z);
                ca = (sint32)(x - (uint32)((uint64)product >> 12));
                product = (sint64)(sint32)(a - c) * (sint32)(r_u32(left) - a);
                x = (uint32)((uint64)product >> 12);
                product = (sint64)(sint32)(b - d) * (sint32)(b - z);
                cb = (sint32)(x - (uint32)((uint64)product >> 12));
                if ((ca ^ cb) < 0 || (uint32)ca + 16 < 33 || (uint32)cb + 16 < 33)
                {
                    a = (uint32)((sint32)dx >> 6);
                    b = (uint32)((sint32)dy >> 6);
                    c = (uint32)((sint32)dz >> 6);
                    distance = a * a + b * b + c * c;
                    fallback_frame = c * c;
                    if ((sint32)distance < (sint32)best_distance)
                    {
                        best = frame;
                        if (frame != 0)
                            best_distance = distance;
                        else
                            first_distance = distance;
                    }
                }
            }
            x = r_u32(point);
            y = r_u32(point + 4);
            z = r_u32(point + 8);
            w_u32(left, x);
            w_u32(left + 4, y);
            w_u32(left + 8, z);
            remaining = (sint32)((uint32)remaining - 1);
            frame = (sint32)((uint32)frame + 1);
            if (remaining > 0)
                continue;
            if (best > 0)
                break;
            if (remaining != 0)
                break;
            best_distance = first_distance;
            goto repeat_last_point;
        }
        if (best >= 0 && direction == 0)
            best = (sint32)((uint32)(sint32)r_s16(definition_offset + 26) - (uint32)best);
        goto finish;
    }
    low = 0;
    high = (sint32)((uint32)remaining - 1);
    for (index = 0; index < 2; ++index)
    {
        candidate = index == 0 ? 0 : high;
        destination = index == 0 ? left : right;
        matrix = r_u32(definition_offset + 40);
        if (matrix != 0)
        {
            probe = p2_sample_track_with_selection(candidate, definition_offset + 20, raw, fallback_frame);
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
            p2_sdk_8007ec64(raw, destination);
            PopMatrix();
            /* TODO: Audit PopMatrix diagnostic exits for the secondary output */
            fallback_frame = xport_gte_read_translation(0);
            w_u32(destination, r_u32(destination) + r_u32(matrix + 20));
            w_u32(destination + 4, r_u32(destination + 4) + r_u32(matrix + 24));
            w_u32(destination + 8, r_u32(destination + 8) + r_u32(matrix + 28));
        }
        else
        {
            probe = p2_sample_track_with_selection(candidate, definition_offset + 20, destination, fallback_frame);
            fallback_frame = probe.fallback_frame;
        }
        dx = r_u32(destination) - r_u32(position_offset);
        dy = r_u32(destination + 4) - r_u32(position_offset + 4);
        dz = r_u32(destination + 8) - r_u32(position_offset + 8);
        a = (uint32)((sint32)dx >> 6);
        b = (uint32)((sint32)dz >> 6);
        distance = a * a + b * b;
        if (index == 0)
        {
            left_distance = distance;
            if (distance < 0x4000)
                best = 0;
        }
        else
        {
            right_distance = distance;
            if (distance < 0x4000 && (sint32)distance < (sint32)left_distance)
                best = r_s16(definition_offset + 26);
        }
    }
    forward = left_distance < right_distance;
    if (forward != 0)
    {
        if ((sint32)(left_distance << 2) < (sint32)right_distance)
            candidate = high >> 1;
        else
            candidate = (sint32)((uint32)(high >> 2) + (uint32)(high >> 1));
    }
    else if ((sint32)(right_distance << 2) < (sint32)left_distance)
        candidate = high >> 1;
    else
        candidate = high >> 2;
    unresolved = 1;
    do
    {
        matrix = r_u32(definition_offset + 40);
        if (matrix != 0)
        {
            probe = p2_sample_track_with_selection(candidate, definition_offset + 20, raw, fallback_frame);
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
            p2_sdk_8007ec64(raw, middle);
            PopMatrix();
            /* TODO: Audit PopMatrix diagnostic exits for the secondary output */
            fallback_frame = xport_gte_read_translation(0);
            w_u32(middle, r_u32(middle) + r_u32(matrix + 20));
            w_u32(middle + 4, r_u32(middle + 4) + r_u32(matrix + 24));
            w_u32(middle + 8, r_u32(middle + 8) + r_u32(matrix + 28));
        }
        else
        {
            probe = p2_sample_track_with_selection(candidate, definition_offset + 20, middle, fallback_frame);
            fallback_frame = probe.fallback_frame;
        }
        destination = forward != 0 ? left : right;
        x = r_u32(destination);
        z = r_u32(destination + 8);
        a = r_u32(middle);
        b = r_u32(middle + 8);
        c = r_u32(position_offset);
        fallback_frame = c;
        d = r_u32(position_offset + 8);
        product = (sint64)(sint32)(x - c) * (sint32)(x - a);
        y = (uint32)((uint64)product >> 12);
        product = (sint64)(sint32)(z - d) * (sint32)(b - z);
        ca = (sint32)(y - (uint32)((uint64)product >> 12));
        product = (sint64)(sint32)(a - c) * (sint32)(x - a);
        y = (uint32)((uint64)product >> 12);
        product = (sint64)(sint32)(b - d) * (sint32)(b - z);
        cb = (sint32)(y - (uint32)((uint64)product >> 12));
        outside = (ca ^ cb) < 0 || (ca > 0 ? ca < cb : cb < ca || (ca == 0 && ca < cb));
        if (outside != 0 && unresolved != 0)
            unresolved = (ca ^ cb) > 0 && ca != 0 && cb != 0;
        move_left = forward != 0 ? outside == 0 : outside != 0;
        x = r_u32(middle);
        y = r_u32(middle + 4);
        z = r_u32(middle + 8);
        if (move_left != 0)
        {
            w_u32(left, x);
            w_u32(left + 4, y);
            w_u32(left + 8, z);
            low = candidate;
        }
        else
        {
            w_u32(right, x);
            w_u32(right + 4, y);
            w_u32(right + 8, z);
            high = candidate;
        }
        candidate = (sint32)((uint32)low + (uint32)high) >> 1;
    } while (low != candidate);
    if (unresolved == 0 && (forward != 0 ? high < r_s16(definition_offset + 26) - 1 : low > 0))
    {
        dx = r_u32(right) - r_u32(position_offset);
        dy = r_u32(right + 4) - r_u32(position_offset + 4);
        dz = r_u32(right + 8) - r_u32(position_offset + 8);
        if (dx + radius < span && dz + radius < span && ((sint32)(dy - 40960) <= 0 || (r_u32(definition_offset + 16) & 0x11) != 0))
        {
            if (direction != 0)
                high = (sint32)((uint32)high - 1);
            if (high <= 0)
                best = 1;
            else
            {
                best = high;
                remaining = r_s16(definition_offset + 26);
                if (high >= remaining)
                    best = remaining - 1;
            }
        }
        else
        {
            dx = r_u32(left) - r_u32(position_offset);
            dy = r_u32(left + 4) - r_u32(position_offset + 4);
            dz = r_u32(left + 8) - r_u32(position_offset + 8);
            if (dx + radius < span && dz + radius < span && ((sint32)(dy - 40960) <= 0 || (r_u32(definition_offset + 16) & 0x11) != 0))
            {
                if (low <= 0)
                    low = 1;
                else
                {
                    remaining = r_s16(definition_offset + 26);
                    if (low >= remaining)
                        low = remaining - 1;
                }
                best = low;
            }
        }
    }
    if (best >= 0)
    {
        if (direction == 0)
            best = (sint32)((uint32)(sint32)r_s16(definition_offset + 26) - (uint32)best);
        remaining = r_s16(definition_offset + 26);
        if (best == remaining - 1)
            best = remaining - 2;
    }
finish:
    p2_guest_stack_pointer = saved_stack;
    output.value = best;
    output.fallback_frame = fallback_frame;
    return output;
}

sint32 p2_func_8002dfc8(uint32 level, uint32 mode)
{
    uint32 entry;
    sint32 value;
    entry = mode != 0 ? 0x80083D44 : 0x80083D45;
    do
    {
        value = r_s8(entry);
        entry += 2;
    } while ((uint32)value != level);
    entry -= 2;
    return mode != 0 ? r_s8(entry + 1) : r_s8(entry - 1);
}

void p2_func_80034b38(void)
{
    w_u8(0x8008A330, 0);
}

void p2_func_800441dc(void)
{
    w_u32(0x8008A338, 0);
}

uint32 p2_func_8002d4c0(void)
{
    w_u32(0x8008A320, 4);
    return 4;
}

uint32 p2_func_800556e0(void)
{
    uint32 object, flags, value, entry, index;
    object = r_u32(0x8008A340);
    if (object != 0)
    {
        p2_func_8001c684(0x800ABD68);
        w_u32(0x800ABD68, r_u32(0x800ABD68) & 0xC0000000);
        flags = r_u32(object + 28);
        value = r_u32(object + 228);
        w_u32(object + 28, flags & 0xC0F793FF);
        w_u32(object + 228, value & 0xFC000000);
        flags = r_u32(0x800ABD6C);
        w_u32(0x800ABD88, 0);
        w_u32(0x800ABD84, 0);
        w_u32(0x800ABD80, 0);
        w_u32(0x800ABD6C, flags & 0xFFFFF932);
        w_u32(object + 100, 0);
        w_u32(object + 96, 0);
        w_u32(object + 92, 0);
        w_u16(0x800ABDA4, 0);
        w_u16(0x800ABDA0, 0);
        w_u16(0x800ABD9E, 0);
        w_u16(0x800ABD9C, 0);
        w_u16(0x800ABD9A, 0);
        w_u16(0x800ABD98, 0);
        w_u32(0x800AC078, 0);
        w_u16(0x800AC00C, 0);
        w_u32(0x800ABF90, 0);
    }
    entry = 0x8008B4C0;
    index = 2;
    do
    {
        if (r_u32(entry) != 0)
            w_u32(entry, 0);
        value = r_u32(0x800AC028);
        if (index == ((value >> 16) & 15))
            p2_func_80055dc0(0x8008A340, 0x800ABD68, (value >> 20) & 15);
        ++index;
        entry += 4;
    } while (index < 15);
    return 0;
}

uint32 p2_func_8002dedc(void)
{
    uint32 offset, first, second, result;
    w_u8(0x800B999B, 0);
    p2_func_800260b0();
    w_u32(0x8008A4E8, 0);
    p2_func_8005ed74(1);
    if (r_u16(0x8008A530) != 0)
    {
        p2_func_80066430();
        p2_func_8006ae90(r_u16(0x8008A530));
    }
    (void)r_u32(0x8008A4E4);
    w_u16(0x8008A530, 0);
    p2_func_800556e0();
    (void)r_u32(0x8008A4E4);
    p2_func_80034b38();
    (void)r_u32(0x8008A4E4);
    p2_func_800441dc();
    (void)r_u32(0x8008A4E4);
    p2_func_8002d4c0();
    offset = 0;
    do
    {
        first = r_u32(0x800B14D8 + offset);
        second = r_u32(0x800B14DC + offset);
        w_u32(0x800B7F50 + offset, first);
        w_u32(0x800B7F54 + offset, second);
        result = first;
        offset += 8;
    } while (offset != 32);
    return result;
}

uint32 p2_func_8001c544(uint32 object_offset, uint32 selection, uint32 descriptor_offset)
{
    uint32 source, destination, end, x, y, z, fourth, value, half, shift, mask, index;
    if (descriptor_offset != 0)
    {
        if (r_u8(object_offset + 12) == 1)
        {
            /* Preserve both reads of each aligned LWL/LWR pair */
            x = r_u32(0x80010180);
            x = r_u32(0x80010180);
            y = r_u32(0x80010184);
            y = r_u32(0x80010184);
            z = r_u32(0x80010188);
            z = r_u32(0x80010188);
            for (index = 0; index < 3; ++index)
            {
                /* Merge SWL then SWR while retaining neighboring bytes */
                value = index == 0 ? x : index == 1 ? y : z;
                destination = descriptor_offset + 4 + index * 4;
                shift = ((destination + 3) & 3) * 8;
                mask = 0xFFFFFFFF >> (24 - shift);
                fourth = r_u32((destination + 3) & 0xFFFFFFFC);
                w_u32((destination + 3) & 0xFFFFFFFC, (fourth & ~mask) | (value >> (24 - shift)));
                shift = (destination & 3) * 8;
                mask = 0xFFFFFFFF << shift;
                fourth = r_u32(destination & 0xFFFFFFFC);
                w_u32(destination & 0xFFFFFFFC, (fourth & ~mask) | (value << shift));
            }
            value = r_u32(0x8001018C);
            value = r_u32(0x8001018C);
            half = r_u16(0x80010190);
            destination = descriptor_offset + 16;
            shift = ((destination + 3) & 3) * 8;
            mask = 0xFFFFFFFF >> (24 - shift);
            fourth = r_u32((destination + 3) & 0xFFFFFFFC);
            w_u32((destination + 3) & 0xFFFFFFFC, (fourth & ~mask) | (value >> (24 - shift)));
            shift = (destination & 3) * 8;
            mask = 0xFFFFFFFF << shift;
            fourth = r_u32(destination & 0xFFFFFFFC);
            w_u32(destination & 0xFFFFFFFC, (fourth & ~mask) | (value << shift));
            w_u16(descriptor_offset + 20, half);
            if (r_s16(object_offset + 34) > 0)
            {
                x = r_u32(object_offset + 16);
                y = r_u32(object_offset + 20);
                z = r_u32(object_offset + 24);
            }
            else
            {
                source = r_u32(object_offset + 40);
                x = r_u32(source + 16);
                y = r_u32(source + 20);
                z = r_u32(source + 24);
            }
            w_u32(descriptor_offset + 24, x);
            w_u32(descriptor_offset + 28, y);
            w_u32(descriptor_offset + 32, z);
            w_u32(descriptor_offset + 44, 0);
            w_u32(descriptor_offset + 40, 0);
            w_u32(descriptor_offset + 36, 0);
        }
        else
        {
            source = r_u32(object_offset + 140) + (selection >> 16) * 48;
            end = source + 48;
            destination = descriptor_offset;
            do
            {
                x = r_u32(source);
                y = r_u32(source + 4);
                z = r_u32(source + 8);
                fourth = r_u32(source + 12);
                w_u32(destination, x);
                w_u32(destination + 4, y);
                w_u32(destination + 8, z);
                w_u32(destination + 12, fourth);
                source += 16;
                destination += 16;
            } while (source != end);
        }
        w_u16(descriptor_offset + 2, 1);
        w_u16(descriptor_offset, selection);
    }
    return r_u32(object_offset + 36) + (selection & 0xFFFF) * 24;
}

uint32 p2_func_8001c710(uint32 object_offset, uint32 descriptor_offset, uint32 selection)
{
    uint32 dx, dy, dz, parent, matrix, mode, upper, lower;
    dx = r_u32(object_offset + 92) - r_u32(descriptor_offset + 36);
    dy = r_u32(object_offset + 96) - r_u32(descriptor_offset + 40);
    dz = r_u32(object_offset + 100) - r_u32(descriptor_offset + 44);
    parent = r_u32(object_offset + 132);
    if (parent != 0)
    {
        matrix = r_u32(parent + 40);
        if (matrix != 0 && (r_u32(object_offset + 28) & 0x2000) == 0)
        {
            dx += r_u32(matrix + 32);
            matrix = r_u32(r_u32(object_offset + 132) + 40);
            dy += r_u32(matrix + 36);
            matrix = r_u32(r_u32(object_offset + 132) + 40);
            dz += r_u32(matrix + 40);
        }
    }
    if (dx != 0 || dz != 0)
        return 0;
    mode = (selection & 0x7000) >> 12;
    if (r_s8(object_offset + 126) == 1 && r_u32(0x800AC084) == 0)
    {
        lower = mode == 1;
        upper = mode == 4 || r_u32(0x8008A544 + mode * 16) == 0xFFFFFFFF;
    }
    else
    {
        lower = 1;
        upper = 1;
    }
    if ((lower == 0 || (sint32)dy > 0) && upper == 0)
        return 1;
    if ((sint32)dy < 0 && lower == 0)
        return 1;
    return 0;
}

uint32 p2_func_80027648(uint32 entry_offset, uint32 flags)
{
    uint32 value, index, table, result;
    value = r_u32(entry_offset);
    result = value >> 1;
    if (value == 0xFFFFFFFF)
        return result;
    index = result & 0x1FF;
    if ((value & 1) != 0)
        table = r_u32(r_u32(0x8008A4E8) + 160);
    else
        table = r_u32(0x800B7F54);
    result = index * 2;
    if (r_u8(table + index * 16 + 4) == 2)
    {
        result |= (value & 1) | (flags << 26);
        w_u32(entry_offset, result);
    }
    return result;
}

uint32 p2_func_8002a210(uint32 list_offset, uint32 object_offset)
{
    uint32 result, node;
    result = r_u32(list_offset + 8);
    if (result == 0)
        return result;
    node = result;
    if (object_offset == result)
    {
        result = r_u32(object_offset + 4);
        w_u32(list_offset + 8, result);
    }
    else
    {
        for (;;)
        {
            result = r_u32(node + 4);
            if (result == object_offset)
            {
                result = r_u32(object_offset + 4);
                w_u32(node + 4, result);
                break;
            }
            node = r_u32(node + 4);
            if (node == 0)
                break;
        }
    }
    w_u32(object_offset + 4, 0);
    return result;
}

uint32 p2_func_8002a2f4(uint32 owner_offset, uint32 object_offset)
{
    uint32 list;
    list = r_u32(object_offset + 20);
    if (list != 0)
    {
        p2_func_8002a448(0, list);
        w_u32(object_offset + 20, 0);
    }
    return p2_func_8002a144(object_offset, 0x8008A31C, 0x8008A318);
}

uint32 p2_func_8005e538(uint32 sound_offset, uint32 handle, uint32 pitch, uint32 volume, uint32 pan, uint32 value)
{
    uint32 index, count, result, offset;
    if (handle == 0)
        index = r_u8(0x8008A350);
    else
    {
        count = r_u8(0x8008A350);
        index = 0;
        while (index < count)
        {
            if (r_u32(0x8008B608 + index * 20) == handle)
            {
                result = index < 8;
                if (sound_offset == handle)
                    return result;
                break;
            }
            ++index;
        }
    }
    result = index < 8;
    if (result == 0)
        return result;
    count = r_u8(0x8008A350);
    if (index == count)
        w_u8(0x8008A350, count + 1);
    offset = index * 20;
    w_u32(0x8008B608 + offset, sound_offset);
    w_u32(0x8008B60C + offset, handle);
    w_u32(0x8008B610 + offset, pitch);
    w_u16(0x8008B614 + offset, volume);
    w_u16(0x8008B616 + offset, pan);
    w_u32(0x8008B618 + offset, value);
    return offset;
}

uint32 p2_func_800455dc(uint32 object_offset, uint32 character_offset, sint32 horizontal_speed, sint32 vertical_speed)
{
    uint32 distance, parent, matrix, saved_stack, position, projected, source, x, y, z, result;
    distance = (uint32)horizontal_speed;
    if (horizontal_speed >= 0)
        w_u32(object_offset + 28, r_u32(object_offset + 28) | 0x40);
    else
        distance = 0u - distance;
    p2_func_8003db7c(object_offset, (sint32)distance, vertical_speed, character_offset + 72);
    if ((r_u16(character_offset + 706) & 15) == 5)
    {
        saved_stack = p2_guest_stack_pointer;
        p2_guest_stack_pointer -= 0x30;
        position = p2_guest_stack_pointer;
        projected = position + 16;
        source = position + 32;
        parent = r_u32(object_offset + 132);
        if (parent != 0 && r_u32(parent + 40) != 0)
        {
            x = r_u32(object_offset + 16);
            y = r_u32(object_offset + 20);
            z = r_u32(object_offset + 24);
            w_u32(source, x);
            w_u32(source + 4, y);
            w_u32(source + 8, z);
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
            p2_sdk_8007ec64(source, position);
            PopMatrix();
            matrix = r_u32(r_u32(object_offset + 132) + 40);
            w_u32(position, r_u32(position) + r_u32(matrix + 20));
            matrix = r_u32(r_u32(object_offset + 132) + 40);
            w_u32(position + 4, r_u32(position + 4) + r_u32(matrix + 24));
            matrix = r_u32(r_u32(object_offset + 132) + 40);
            w_u32(position + 8, r_u32(position + 8) + r_u32(matrix + 28));
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
        w_u32(projected, r_u32(position) + r_u32(object_offset + 92));
        w_u32(projected + 4, r_u32(position + 4) + r_u32(object_offset + 96));
        w_u32(projected + 8, r_u32(position + 8) + r_u32(object_offset + 100));
        result = p2_func_8002ab84(object_offset, projected);
        if ((sint32)result > 0)
            p2_func_8003db7c(object_offset, 0, 0, character_offset + 72);
        p2_guest_stack_pointer = saved_stack;
    }
    result = r_u32(object_offset + 28) & 0xFFFFFFBF;
    w_u32(object_offset + 28, result);
    return result;
}

uint32 p2_func_8002a80c(uint32 position_offset, uint32 scale, uint32 effect)
{
    uint32 object, entry, next, previous, material, list, count, bytes, allocation, target, x, y, z, scene;
    if (effect >= 35)
        return 0;
    entry = effect * 8;
    if (r_u32(0x800B170C + entry) == 0)
        return 0;
    object = r_u32(0x8008A318);
    if (object != 0)
    {
        next = r_u32(object);
        previous = r_u32(0x8008A31C);
        w_u32(0x8008A31C, object);
        w_u32(0x8008A318, next);
        w_u32(object, previous);
    }
    w_u32(object + 4, 0);
    w_u32(object + 8, 0);
    w_u8(object + 12, 7);
    w_u8(object + 13, 16);
    w_u16(object + 14, 1);
    w_u8(object + 27, 0xFF);
    w_u8(object + 26, 0xFF);
    w_u8(object + 25, 0xFF);
    w_u8(object + 24, 0xFF);
    w_u32(object + 16, 0);
    w_u32(object + 20, 0);
    w_u32(object + 36, 0);
    w_u32(object + 32, 0);
    w_u32(object + 28, 0);
    w_u16(object + 42, 128);
    w_u16(object + 40, 128);
    w_u16(object + 46, 0);
    w_u16(object + 44, 0);
    if (object == 0)
        return object;
    material = r_u32(0x800B1708 + entry);
    list = r_u32(0x800B170C + entry);
    w_u32(object + 16, material);
    w_u32(object + 20, list);
    list = r_u32(object + 20);
    if (list != 0)
    {
        count = r_u8(list - 4);
        bytes = (count + 1) * 4;
        allocation = p2_func_80064d28(bytes);
        p2_func_80066358(allocation, 0xFF, bytes);
        w_u32(allocation, count);
        target = allocation + 4;
        w_u32(object + 20, target);
        if (target != 0)
        {
            list = r_u32(0x800B170C + entry);
            bytes = count * 4;
            p2_func_8006637c((uint8 *)psx_addr(target, bytes), (const uint8 *)psx_addr(list, bytes), bytes);
            target = r_u32(object + 20);
            p2_func_80027648(target + bytes - 4, 8);
            p2_func_80027594(r_u32(object + 20));
        }
    }
    w_u16(object + 14, r_u16(object + 14) | 2);
    x = r_u32(position_offset);
    y = r_u32(position_offset + 4);
    z = r_u32(position_offset + 8);
    w_u32(object + 28, x);
    w_u32(object + 32, y);
    w_u32(object + 36, z);
    w_u16(object + 42, scale >> 5);
    w_u16(object + 40, scale >> 5);
    scene = r_u32(0x8008A4E8);
    w_u32(object + 4, r_u32(scene + 108));
    w_u32(scene + 108, object);
    return object;
}

uint32 p2_func_80061f50(uint32 vertices_offset, uint32 polygons_offset, sint32 count)
{
    uint32 colors, metadata, normal, remaining = (uint32)count;
    uint32 packet, limit, sum, depth, value, index0, index1, index2, index3, screen_base;
    uint32 screen0, screen1, screen2, screen3, vertex0, vertex1, vertex2, vertex3;
    uint32 material, flags, context, start, end, kind, bucket_offset, link, tag, fourth_color, result;
    sint32 coordinate, coefficient, surface, bucket, distance, fade;
    sint64 product;
    colors = r_u32(0x1F8002AC);
    xport_gte_write_data(8, 0);
    normal = polygons_offset + 16;
    metadata = colors + 12;
    do
    {
        packet = r_u32(0x8008A4F8);
        limit = r_u32(0x8008A4A0);
        result = packet < limit;
        if (result == 0)
            break;
        coordinate = r_s32(0x1F800258) >> 6;
        coefficient = r_s16(normal);
        sum = (uint32)((sint64)coordinate * coefficient);
        coordinate = r_s32(0x1F80025C) >> 6;
        coefficient = r_s16(normal + 2);
        sum += (uint32)((sint64)coordinate * coefficient);
        coordinate = r_s32(0x1F800260) >> 6;
        coefficient = r_s16(normal + 4);
        value = r_u32(normal - 4);
        sum += (uint32)((sint64)coordinate * coefficient);
        result = (uint32)((sint32)sum >> 6) + value;
        surface = (sint32)result;
        if (surface <= 0)
            goto next_polygon;
        index0 = r_u16(normal - 12);
        screen_base = r_u32(0x1F8002C8);
        index1 = r_u16(normal - 10);
        index3 = r_u16(normal - 6);
        index2 = r_u16(normal - 8);
        screen0 = screen_base + index0 * 8;
        screen1 = screen_base + index1 * 8;
        screen2 = screen_base + index2 * 8;
        screen3 = screen_base + index3 * 8;
        if (r_u16(screen0) >= 320 && r_u16(screen1) >= 320 && r_u16(screen2) >= 320 && r_u16(screen3) >= 320)
        {
            result = 0;
            goto next_polygon;
        }
        if (r_u16(screen0 + 2) >= 240 && r_u16(screen1 + 2) >= 240 && r_u16(screen2 + 2) >= 240 && r_u16(screen3 + 2) >= 240)
        {
            result = 0;
            goto next_polygon;
        }
        depth = r_u32(screen0 + 4);
        sum = r_u32(screen1 + 4);
        value = r_u32(screen2 + 4);
        link = r_u32(screen3 + 4);
        limit = r_u32(0x1F8002B4);
        depth += sum + value + link;
        result = (sint32)(depth << 1) < (sint32)limit;
        if (result == 0)
            goto next_polygon;
        material = r_u32(polygons_offset);
        flags = r_u32(material);
        if ((flags & 1) != 0)
        {
            if (surface > 0x3FFFF800)
            {
                if (r_s32(metadata) < 0)
                    w_u32(metadata, 0);
            }
            else if (r_s32(metadata) >= 0)
                w_u32(metadata, 0x80000000);
        }
        coefficient = r_s16(normal + 6);
        value = r_u32(0x1F8002B8);
        sum = (uint32)((sint32)depth >> 2) + (uint32)(coefficient >> 1);
        bucket = (sint32)(sum * value) >> 19;
        if ((uint32)bucket >= 1024)
            bucket = bucket < 0 ? 0 : 1023;
        context = r_u32(0x8008A4E8);
        end = r_u32(context + 152);
        if (end != 0)
        {
            value = r_u32(0x1F8002B4);
            product = (sint64)(sint32)value * (sint32)((uint32)bucket << 2);
            distance = (sint32)((uint64)product >> 12);
            start = r_u32(context + 148);
            if ((sint32)start < distance)
            {
                result = (uint32)distance - start;
                if (distance >= (sint32)end)
                    goto next_polygon;
                coefficient = r_s32(0x1F8002BC);
                product = (sint64)coefficient * (sint32)result;
                fade = (sint32)((uint64)product >> 12) >> 8;
                xport_gte_write_data(8, (uint32)fade);
            }
            else
                xport_gte_write_data(8, 0);
        }
        else
            xport_gte_write_data(8, 0);
        kind = flags & 6;
        if ((r_u32(metadata) & 0x40000000) == 0)
        {
            if ((r_u32(0x1F800320) & 1) == 0)
            {
                p2_func_8002e84c(0);
                w_u32(0x1F800320, r_u32(0x1F800320) | 1);
            }
            vertex0 = vertices_offset + (uint32)r_u16(normal - 12) * 16;
            vertex1 = vertices_offset + (uint32)r_u16(normal - 10) * 16;
            vertex3 = vertices_offset + (uint32)r_u16(normal - 6) * 16;
            vertex2 = vertices_offset + (uint32)r_u16(normal - 8) * 16;
            value = r_u32(metadata);
            p2_func_8002ede4(vertex0, (flags >> 24) | (value & 0x80000000));
            xport_gte_write_data(6, r_u32(material + 16));
            if (kind == 6 || kind == 2)
            {
                xport_gte_write_data(0, r_u32(vertex0 + 8));
                xport_gte_write_data(1, r_u32(vertex0 + 12));
                xport_gte_write_data(2, r_u32(vertex1 + 8));
                xport_gte_write_data(3, r_u32(vertex1 + 12));
                xport_gte_write_data(4, r_u32(vertex3 + 8));
                xport_gte_write_data(5, r_u32(vertex3 + 12));
                xport_gte_execute(0x118043F);
                tag = kind == 6 ? 0x0C000000 : 0x08000000;
            }
            else
            {
                xport_gte_write_data(0, r_u32(normal));
                xport_gte_write_data(1, r_u32(normal + 4));
                xport_gte_execute(0x108041B);
                tag = kind == 4 ? 0x09000000 : 0x05000000;
            }
            bucket_offset = r_u32(0x8008A44C) + (uint32)bucket * 4;
            link = r_u32(bucket_offset);
            packet = r_u32(0x8008A4F8);
            w_u32(packet, (link & 0xFFFFFF) | tag);
            link = r_u32(bucket_offset);
            w_u32(bucket_offset, (link & 0xFF000000) | (packet & 0xFFFFFF));
            if (kind == 6)
            {
                p2_func_800612c0(material, r_u32(0x8008A4F8) + 12, 3);
                w_u32(colors, xport_gte_read_data(20));
                w_u32(colors + 4, xport_gte_read_data(21));
                w_u32(colors + 8, xport_gte_read_data(22));
                xport_gte_write_data(0, r_u32(vertex2 + 8));
                xport_gte_write_data(1, r_u32(vertex2 + 12));
                xport_gte_execute(0x108041B);
                packet = r_u32(0x8008A4F8);
                w_u32(packet + 8, r_u32(screen0));
                fourth_color = xport_gte_read_data(22);
                xport_gte_write_data(20, r_u32(colors));
                xport_gte_write_data(21, r_u32(colors + 4));
                xport_gte_write_data(22, r_u32(colors + 8));
                xport_gte_write_data(6, r_u32(colors + 8));
                xport_gte_execute(0xF8002A);
                packet = r_u32(0x8008A4F8);
                w_u32(packet + 20, r_u32(screen1));
                w_u32(packet + 32, r_u32(screen3));
                w_u32(packet + 44, r_u32(screen2));
                packet = r_u32(0x8008A4F8);
                w_u32(packet + 4, xport_gte_read_data(20));
                w_u32(packet + 16, xport_gte_read_data(21));
                w_u32(packet + 28, xport_gte_read_data(22));
                xport_gte_write_data(6, fourth_color);
                xport_gte_execute(0x780010);
                w_u32(metadata, r_u32(metadata) | 0x40000000 | (fourth_color & 0xFFFFFF));
                packet = r_u32(0x8008A4F8);
                w_u32(packet + 40, xport_gte_read_data(22));
                result = r_u32(0x8008A4F8) + 52;
                w_u32(0x8008A4F8, result);
            }
            else if (kind == 2)
            {
                packet = r_u32(0x8008A4F8);
                if ((flags & 0x80) != 0)
                {
                    link = r_u32(packet);
                    w_u32(packet + 12, (link & 0xFFFFFF) | 0x08000000);
                    w_u32(packet, ((packet + 12) & 0xFFFFFF) | 0x02000000);
                    w_u32(packet + 4, (flags & 0x60) | 0xE1000000);
                    w_u32(packet + 8, 0);
                    packet += 12;
                }
                w_u32(colors, xport_gte_read_data(20));
                w_u32(colors + 4, xport_gte_read_data(21));
                w_u32(colors + 8, xport_gte_read_data(22));
                xport_gte_write_data(0, r_u32(vertex2 + 8));
                xport_gte_write_data(1, r_u32(vertex2 + 12));
                xport_gte_execute(0x108041B);
                w_u32(packet + 8, r_u32(screen0));
                fourth_color = xport_gte_read_data(22);
                xport_gte_write_data(20, r_u32(colors));
                xport_gte_write_data(21, r_u32(colors + 4));
                xport_gte_write_data(22, r_u32(colors + 8));
                xport_gte_write_data(6, r_u32(colors + 8));
                xport_gte_execute(0xF8002A);
                w_u32(packet + 16, r_u32(screen1));
                w_u32(packet + 24, r_u32(screen3));
                w_u32(packet + 32, r_u32(screen2));
                w_u32(packet + 4, xport_gte_read_data(20));
                w_u32(packet + 12, xport_gte_read_data(21));
                w_u32(packet + 20, xport_gte_read_data(22));
                xport_gte_write_data(6, fourth_color);
                xport_gte_execute(0x780010);
                w_u32(metadata, r_u32(metadata) | 0x40000000 | (fourth_color & 0xFFFFFF));
                w_u32(packet + 28, xport_gte_read_data(22));
                result = packet + 36;
                w_u32(0x8008A4F8, result);
            }
            else if (kind == 4)
            {
                packet = r_u32(0x8008A4F8);
                p2_func_800612c0(material, packet + 12, 2);
                w_u32(colors, xport_gte_read_data(22));
                xport_gte_write_data(6, r_u32(colors));
                xport_gte_execute(0x780010);
                w_u32(packet + 8, r_u32(screen0));
                w_u32(packet + 16, r_u32(screen1));
                w_u32(packet + 24, r_u32(screen3));
                w_u32(packet + 32, r_u32(screen2));
                w_u32(packet + 4, xport_gte_read_data(22));
                result = r_u32(metadata) | 0x40000000;
                w_u32(metadata, result);
                w_u32(0x8008A4F8, packet + 40);
            }
            else
            {
                packet = r_u32(0x8008A4F8);
                if ((flags & 0x80) != 0)
                {
                    link = r_u32(packet);
                    w_u32(packet + 12, (link & 0xFFFFFF) | 0x05000000);
                    w_u32(packet, ((packet + 12) & 0xFFFFFF) | 0x02000000);
                    w_u32(packet + 4, (flags & 0x60) | 0xE1000000);
                    w_u32(packet + 8, 0);
                    packet += 12;
                }
                w_u32(colors, xport_gte_read_data(22));
                xport_gte_write_data(6, r_u32(colors));
                xport_gte_execute(0x780010);
                w_u32(packet + 8, r_u32(screen0));
                w_u32(packet + 12, r_u32(screen1));
                w_u32(packet + 16, r_u32(screen3));
                w_u32(packet + 20, r_u32(screen2));
                w_u32(packet + 4, xport_gte_read_data(22));
                w_u32(metadata, r_u32(metadata) | 0x40000000);
                result = packet + 24;
                w_u32(0x8008A4F8, result);
            }
        }
        else
        {
            if (kind == 6 || kind == 2)
            {
                xport_gte_write_data(20, r_u32(colors));
                xport_gte_write_data(21, r_u32(colors + 4));
                xport_gte_write_data(22, r_u32(colors + 8));
                xport_gte_write_data(6, r_u32(colors + 8));
                xport_gte_execute(0xF8002A);
                tag = kind == 6 ? 0x0C000000 : 0x08000000;
            }
            else
            {
                xport_gte_write_data(6, r_u32(colors));
                xport_gte_execute(0x780010);
                tag = kind == 4 ? 0x09000000 : 0x05000000;
            }
            bucket_offset = r_u32(0x8008A44C) + (uint32)bucket * 4;
            link = r_u32(bucket_offset);
            packet = r_u32(0x8008A4F8);
            w_u32(packet, (link & 0xFFFFFF) | tag);
            link = r_u32(bucket_offset);
            w_u32(bucket_offset, (link & 0xFF000000) | (packet & 0xFFFFFF));
            if (kind == 6)
            {
                p2_func_800612c0(material, r_u32(0x8008A4F8) + 12, 3);
                packet = r_u32(0x8008A4F8);
                w_u32(packet + 4, xport_gte_read_data(20));
                w_u32(packet + 16, xport_gte_read_data(21));
                w_u32(packet + 28, xport_gte_read_data(22));
                xport_gte_write_data(6, r_u32(metadata));
                xport_gte_execute(0x780010);
                packet = r_u32(0x8008A4F8);
                w_u32(packet + 8, r_u32(screen0));
                w_u32(packet + 20, r_u32(screen1));
                w_u32(packet + 32, r_u32(screen3));
                w_u32(packet + 44, r_u32(screen2));
                w_u32(packet + 40, xport_gte_read_data(22));
                result = r_u32(0x8008A4F8) + 52;
                w_u32(0x8008A4F8, result);
            }
            else if (kind == 2)
            {
                packet = r_u32(0x8008A4F8);
                if ((flags & 0x80) != 0)
                {
                    link = r_u32(packet);
                    w_u32(packet + 12, (link & 0xFFFFFF) | 0x08000000);
                    w_u32(packet, ((packet + 12) & 0xFFFFFF) | 0x02000000);
                    w_u32(packet + 4, (flags & 0x60) | 0xE1000000);
                    w_u32(packet + 8, 0);
                    packet += 12;
                }
                w_u32(packet + 4, xport_gte_read_data(20));
                w_u32(packet + 12, xport_gte_read_data(21));
                w_u32(packet + 20, xport_gte_read_data(22));
                xport_gte_write_data(6, r_u32(metadata));
                xport_gte_execute(0x780010);
                w_u32(packet + 8, r_u32(screen0));
                w_u32(packet + 16, r_u32(screen1));
                w_u32(packet + 24, r_u32(screen3));
                w_u32(packet + 32, r_u32(screen2));
                w_u32(packet + 28, xport_gte_read_data(22));
                result = packet + 36;
                w_u32(0x8008A4F8, result);
            }
            else if (kind == 4)
            {
                packet = r_u32(0x8008A4F8);
                p2_func_800612c0(material, packet + 12, 2);
                w_u32(packet + 8, r_u32(screen0));
                w_u32(packet + 16, r_u32(screen1));
                w_u32(packet + 24, r_u32(screen3));
                w_u32(packet + 32, r_u32(screen2));
                result = packet + 4;
                w_u32(result, xport_gte_read_data(22));
                w_u32(0x8008A4F8, packet + 40);
            }
            else
            {
                packet = r_u32(0x8008A4F8);
                if ((flags & 0x80) != 0)
                {
                    link = r_u32(packet);
                    w_u32(packet + 12, (link & 0xFFFFFF) | 0x05000000);
                    w_u32(packet, ((packet + 12) & 0xFFFFFF) | 0x02000000);
                    w_u32(packet + 4, (flags & 0x60) | 0xE1000000);
                    w_u32(packet + 8, 0);
                    packet += 12;
                }
                w_u32(packet + 8, r_u32(screen0));
                w_u32(packet + 12, r_u32(screen1));
                w_u32(packet + 16, r_u32(screen3));
                w_u32(packet + 20, r_u32(screen2));
                w_u32(packet + 4, xport_gte_read_data(22));
                result = packet + 24;
                w_u32(0x8008A4F8, result);
            }
        }
    next_polygon:
        metadata += 16;
        polygons_offset += 24;
        colors += 16;
        normal += 24;
        --remaining;
    } while (remaining != 0);
    return result;
}

uint32 p2_func_8005e8bc(uint32 position_offset, uint32 radius, uint32 attenuation_offset, uint32 pan_offset)
{
    uint32 scene, dx, dy, dz, angle, pan, factor, denominator;
    sint32 distance, quarter, reciprocal, delta;
    sint64 product;
    scene = r_u32(0x8008A114);
    dx = r_u32(scene + 16) - r_u32(position_offset);
    dy = r_u32(scene + 20) - r_u32(position_offset + 4);
    dz = r_u32(scene + 24) - r_u32(position_offset + 8);
    distance = (sint32)p2_func_8002aa1c(dx, dy, dz);
    if (distance >= (sint32)radius)
    {
        if (attenuation_offset != 0)
        {
            w_u32(attenuation_offset, 0);
            return 0;
        }
    }
    else if (attenuation_offset != 0)
    {
        quarter = (sint32)radius >> 2;
        if (quarter < distance)
        {
            delta = (sint32)(radius - (uint32)distance);
            denominator = radius - (uint32)quarter;
            if (denominator == 0)
                xport_mips_break(7);
            reciprocal = 0x40000000 / (sint32)denominator;
            product = (sint64)delta * reciprocal;
            factor = (uint32)(product >> 18);
            w_u32(0x1F80030C, factor);
        }
        else
            factor = 4096;
        w_u32(attenuation_offset, factor);
        if (factor == 0)
            return 0;
    }
    pan = 1024;
    if (dx + 32 >= 65 || dz + 32 >= 65)
    {
        angle = p2_func_8005dc00((sint32)(0u - dz), (sint32)(0u - dx));
        scene = r_u32(0x8008A114);
        pan = ((uint32)(sint32)r_s16(scene + 42) - (uint32)(sint32)(sint16)angle + 1024) & 0xFFF;
        if (pan >= 0x801)
            pan = 4096 - pan;
        if (distance < (sint32)radius)
        {
            if (radius == 0)
                xport_mips_break(7);
            reciprocal = 0x40000000 / (sint32)radius;
            product = (sint64)distance * reciprocal;
            factor = (uint32)(product >> 18);
            delta = (sint32)((pan - 1024) * factor);
            w_u32(0x1F80030C, factor);
            pan = (uint32)(delta >> 12) + 1024;
        }
        if (pan == 2048)
            pan = 2047;
    }
    w_u32(pan_offset, (((pan << 3) + pan) >> 11) + 1);
    return 1;
}

uint32 p2_func_8005eb18(uint32 sound_offset, sint32 volume, uint32 pan, uint32 pitch, uint32 position_offset, uint32 radius, uint32 voice, uint32 mode, uint32 option)
{
    uint32 saved_stack, locals, attenuation, channel, active, value, pan_value, scaled;
    sint32 level, selected;
    saved_stack = p2_guest_stack_pointer;
    p2_guest_stack_pointer -= 8;
    locals = p2_guest_stack_pointer;
    w_u32(locals + 4, pan);
    level = volume;
    if (pan == 0)
    {
        attenuation = volume < 4097 ? 0 : locals;
        value = p2_func_8005e8bc(position_offset, radius, attenuation, locals + 4);
        if ((value << 24) == 0)
            goto silent;
        if (volume >= 4097)
        {
            value = (((uint32)volume - 1) & 0xFFF) + 1;
            level = (sint32)(r_u32(locals) * value) >> 12;
            if (level == 0)
                goto silent;
        }
    }
    scaled = (uint32)level * (uint32)(sint32)r_s16(0x8008A24C);
    level = (sint32)scaled >> 12;
    w_u32(locals, (uint32)level);
    if (level == 0)
        goto silent;
    if (volume >= 8193)
        w_u32(locals + 4, 5);
    if ((sint32)voice >= 0 && voice < 8 && r_u32(0x8008B6A8 + voice * 12) == sound_offset)
    {
        channel = voice & 0xFFFF;
        active = p2_func_8006a940((uint16)channel);
        if (active != 0)
        {
            level = r_s32(locals) >> 4;
            p2_func_8006a740((uint16)channel, (uint16)level);
            p2_func_8006a638((uint16)channel, r_u16(locals + 4));
            p2_func_8006a814((uint16)channel, pitch);
            p2_guest_stack_pointer = saved_stack;
            return voice;
        }
        if (mode == 0)
        {
            p2_guest_stack_pointer = saved_stack;
            return 0xFFFFFFFF;
        }
    }
    if (mode != 0)
        (void)r_u16(sound_offset + 6);
    selected = p2_func_8005e628(0xFFFFFFFF, sound_offset, option);
    if (selected >= 0)
    {
        level = r_s32(locals) >> 4;
        pan_value = r_u32(locals + 4);
        if (level != 0)
        {
            value = pitch;
            if (value == 0)
                value = r_u16(sound_offset + 4);
            pan_value = (pan_value & 0xFFFF) != 0 ? pan_value - 1 : 4;
            channel = (uint32)selected & 0xFFFF;
            p2_func_8006a740((uint16)channel, (uint16)level);
            p2_func_8006a638((uint16)channel, (uint16)pan_value);
            p2_func_8006a814((uint16)channel, value);
            (void)r_u16(sound_offset + 6);
            p2_func_8006a878((uint16)channel, sound_offset);
        }
    }
    p2_guest_stack_pointer = saved_stack;
    return (uint32)selected;

silent:
    if ((sint32)voice >= 0 && voice < 8 && r_u32(0x8008B6A8 + voice * 12) == sound_offset)
        p2_func_8006a970((uint16)voice);
    p2_guest_stack_pointer = saved_stack;
    return 0xFFFFFFFF;
}

uint32 p2_func_8005ee20(void)
{
    uint32 count, entry, requested, previous, cached, skip, sound, pitch, volume, pan, channel;
    sint32 selected;
    count = r_u8(0x8008A350);
    w_u8(0x8008A350, 0);
    entry = 0x8008B608 + count * 20;
    for (;;)
    {
        entry -= 20;
        if (entry == 0x8008B5F4)
            return 0x8008B5F4;
        previous = r_u32(entry + 4);
        requested = 8;
        skip = 0;
        if (previous == 0xFFFFFFFF)
            (void)r_u16(r_u32(entry) + 6);
        else if (previous != 0)
        {
            for (requested = 0; requested < 8; ++requested)
            {
                cached = r_u32(0x8008B6A8 + requested * 12);
                if (cached == r_u32(entry + 4))
                {
                    if (cached == r_u32(entry) && r_u32(0x8008A518) - r_u32(0x8008B6AC + requested * 12) < 60 && p2_func_8006a940((uint16)requested) != 0)
                        skip = 1;
                    break;
                }
            }
        }
        if (skip != 0)
            continue;
        sound = r_u32(entry);
        selected = p2_func_8005e628(requested, sound, r_u32(entry + 16));
        if (selected >= 0)
        {
            pan = r_u16(entry + 14);
            volume = r_u16(entry + 12) >> 4;
            pitch = r_u32(entry + 8);
            sound = r_u32(entry);
            if (volume != 0)
            {
                if (pitch == 0)
                    pitch = r_u16(sound + 4);
                pan = pan != 0 ? pan - 1 : 4;
                channel = (uint32)selected & 0xFFFF;
                p2_func_8006a740((uint16)channel, (uint16)volume);
                p2_func_8006a638((uint16)channel, (uint16)pan);
                p2_func_8006a814((uint16)channel, pitch);
                (void)r_u16(sound + 6);
                p2_func_8006a878((uint16)channel, sound);
            }
        }
    }
}

void p2_func_800606f4(void)
{
    /* Original JR RA/NOP has no side effects */
}

uint32 p2_func_80065f78(sint32 intensity)
{
    uint32 color, packet, next_packet;
    if (intensity >= 4096)
        color = 255;
    else if (intensity > 0)
        color = (uint32)(255 * intensity + 2048) >> 12;
    else
        color = 0;
    color |= (color << 8) | (color << 16);
    if (r_u32(0x8008A4B8) == 0)
        w_u32(0x8008A4B8, r_u32(0x8008A4F8));
    packet = r_u32(0x8008A4F8);
    next_packet = packet + 28;
    w_u32(packet, ((packet + 12) & 0xFFFFFF) | 0x02000000);
    w_u32(packet + 4, 0xE1000020);
    w_u32(packet + 8, 0);
    w_u32(packet + 16, color | 0x62000000);
    w_u32(packet + 12, (next_packet & 0xFFFFFF) | 0x03000000);
    w_u32(packet + 20, 0);
    w_u32(packet + 24, 0x00F00140);
    w_u32(0x8008A478, packet + 12);
    w_u32(0x8008A4F8, next_packet);
    return 0x03000000;
}

uint32 p2_func_80066070(sint32 intensity)
{
    uint32 color, packet, next_packet;
    if (intensity >= 4096)
        color = 255;
    else if (intensity > 0)
        color = (uint32)(255 * intensity + 2048) >> 12;
    else
        color = 0;
    color |= (color << 8) | (color << 16);
    if (r_u32(0x8008A4B8) == 0)
        w_u32(0x8008A4B8, r_u32(0x8008A4F8));
    packet = r_u32(0x8008A4F8);
    next_packet = packet + 28;
    w_u32(packet, ((packet + 12) & 0xFFFFFF) | 0x02000000);
    w_u32(packet + 4, 0xE1000040);
    w_u32(packet + 8, 0);
    w_u32(packet + 16, color | 0x62000000);
    w_u32(packet + 12, (next_packet & 0xFFFFFF) | 0x03000000);
    w_u32(packet + 20, 0);
    w_u32(packet + 24, 0x00F00140);
    w_u32(0x8008A478, packet + 12);
    w_u32(0x8008A4F8, next_packet);
    return 0x03000000;
}

uint32 p2_func_8005cf3c(void)
{
    uint32 scene, result, value;
    sint32 fade, numerator, remaining, duration, half, intensity;
    scene = r_u32(0x8008A114);
    fade = r_s16(scene + 46);
    if (fade != 0)
    {
        if (fade > 0)
        {
            value = (uint32)fade - 1u;
            numerator = (sint32)(value << 16) >> 4;
            w_u16(scene + 46, value);
            intensity = numerator / 15;
        }
        else
        {
            value = (uint32)fade + 1u;
            fade = (sint16)value;
            numerator = (sint32)(((uint32)fade + 15u) << 12);
            w_u16(scene + 46, value);
            intensity = numerator / 15;
            if (fade == 0)
                w_u16(scene + 46, 0xFFFF);
        }
        return p2_func_80066070(intensity);
    }
    result = 0;
    remaining = r_s32(0x8008A508);
    if (remaining > 0)
    {
        duration = r_s32(0x8008A47C);
        result = duration < 2;
        if (duration >= 2)
        {
            --remaining;
            w_u32(0x8008A508, (uint32)remaining);
            half = duration / 2;
            if (remaining > half)
            {
                numerator = (sint32)(((uint32)remaining - (uint32)half) << 12);
                intensity = (sint32)(4096u - (uint32)(numerator / half));
            }
            else
            {
                numerator = (sint32)((uint32)remaining << 12);
                intensity = numerator / half;
            }
            return p2_func_80065f78(intensity);
        }
    }
    remaining = r_s32(0x8008A4D4);
    if (remaining <= 0)
        return result;
    duration = r_s32(0x8008A470);
    result = duration < 2;
    if (duration < 2)
        return result;
    --remaining;
    w_u32(0x8008A4D4, (uint32)remaining);
    half = duration / 2;
    if (remaining > half)
    {
        numerator = (sint32)(((uint32)remaining - (uint32)half) << 12);
        intensity = (sint32)(4096u - (uint32)(numerator / half));
    }
    else
    {
        numerator = (sint32)((uint32)remaining << 12);
        intensity = numerator / half;
    }
    return p2_func_80066070(intensity);
}

uint32 p2_func_80066d60(void)
{
    return r_u8(0x800A5C28) == 0xFF;
}

uint32 p2_func_800678a8(void)
{
    if ((r_u32(0x1F800384) & 0x800) == 0 || (r_u32(0x1F800388) & 0x800) != 0)
    {
        if (p2_func_80066d60() == 0)
            return 0;
    }
    if (r_u16(0x8008A530) == 0)
        return 0;
    return r_s16(r_u32(0x8008A114) + 46) == 0;
}

uint32 p2_func_80065bc4(uint32 object_offset)
{
    uint32 result, value, saved_stack, state, text, index, unchanged, mode, first, second, third;
    uint32 prompt, target;
    result = r_u32(0x1F800320) & 0x10;
    if (result == 0)
    {
        w_u32(0x8008A264, 0);
        result = p2_func_800678a8() << 24;
        if (result != 0)
        {
            value = r_u32(0x1F800320);
            mode = r_u8(0x8008B708);
            w_u32(0x1F800320, value | 0x10);
            result = 3;
            if (mode == 1 || mode == 3)
                w_u32(0x8008A264, 1);
        }
    }
    if (object_offset == 0)
        return result;
    saved_stack = p2_guest_stack_pointer;
    p2_guest_stack_pointer -= 0x30;
    state = p2_guest_stack_pointer;
    text = state + 0x20;
    p2_func_80053638(state);
    if ((r_u32(0x1F800320) & 0x20000) != 0)
    {
        w_u32(state + 8, 0);
        w_u32(state + 12, 0);
    }
    unchanged = 1;
    if (r_u32(0x8008A264) == 0)
    {
        for (index = 0; index < 3; ++index)
        {
            if (r_u8(0x8008B708 + index * 8) != 0 || r_u32(0x8008B70C + index * 8) != r_u32(state + 4 + index * 4) || (r_u32(0x1F800320) & 0x10) != 0 || r_u16(r_u32(0x8008A4E4) + 262) == 0)
            {
                // Indices0..2 overwrite all three incoming layout values
                p2_func_80065478(index, r_u32(state + 4 + index * 4), 0, 0, 0);
                unchanged = 0;
            }
        }
    }
    if (unchanged != 0)
    {
        target = r_u32(0x800B1714);
        w_u32(0x8008A4F0, 0);
        p2_func_80027594(target);
        p2_func_80027594(r_u32(0x800B1734));
    }
    if ((r_u32(0x1F800320) & 2) != 0)
    {
        mode = (r_u32(0x8008A518) & 6) >> 1;
        if (mode == 0)
        {
            first = r_u32(0x8008A268);
            second = r_u8(0x8008A26C);
            w_u32(text, first);
            w_u8(text + 4, second);
        }
        else if (mode == 2)
        {
            first = r_u32(0x8001062C);
            second = r_u32(0x80010630);
            third = r_u16(0x80010634);
            w_u32(text, first);
            w_u32(text + 4, second);
            w_u16(text + 8, third);
            w_u8(text + 10, r_u8(0x80010636));
        }
        else
        {
            first = r_u32(0x8008A270);
            second = r_u32(0x8008A274);
            w_u32(text, first);
            w_u32(text + 4, second);
        }
        p2_func_80064ff4(0x00CA00A0, text, 1, 0);
    }
    result = r_u16(r_u32(0x8008A4E4) + 262);
    if (result != 0)
    {
        p2_guest_stack_pointer = saved_stack;
        return result;
    }
    index = r_u8(0x800B999B);
    if (index != 0)
    {
        target = r_u32(0x8008A428) + index * 40u - 40u;
        p2_func_80064ff4(0x005500A0, target, 1, 0);
        index = r_u8(0x800B999B);
    }
    if (index == 0)
        w_u8(0x8008A368, 0);
    else
    {
        value = r_u32(0x8008A518);
        if (r_u32(0x8008A364) < value)
        {
            mode = r_u8(0x8008A368);
            w_u32(0x8008A364, value + 60);
            w_u8(0x8008A368, 1u - mode);
        }
    }
    value = r_u32(0x8008A478);
    mode = r_u8(0x8008A368);
    w_u32(0x8008A438, value);
    if (mode == 0)
    {
        target = r_u32(r_u32(0x8008A4A4) + 28) + 220u;
        if (target != 0)
            p2_func_80064e74(0x00C8001E, 0, target, 0);
        prompt = 0x80010638;
    }
    else
    {
        target = r_u32(r_u32(0x8008A4A4) + 28) + 160u;
        if (target != 0)
            p2_func_80064e74(0x00C8001E, 0, target, 0);
        prompt = 0x8001064C;
    }
    result = p2_func_80064ff4(0x00CA003C, prompt, 0, 0);
    p2_guest_stack_pointer = saved_stack;
    return result;
}

uint32 p2_func_80064dbc(sint32 top, uint32 height)
{
    uint32 extent, scene;
    sint32 bottom, remaining;
    if (top >= 0)
    {
        extent = 239;
        if (top < 239)
            extent = (uint32)top;
        scene = r_u32(0x8008A4E8);
        p2_func_80066bdc(0, ((extent + 1u) << 16) | 320u, r_u32(scene + 136));
    }
    bottom = (sint32)((uint32)top + height);
    if (bottom >= 240)
        return 0;
    extent = 239;
    remaining = (sint32)(239u - (uint32)bottom);
    if (remaining < 239)
        extent = (uint32)remaining;
    else if (bottom < 0)
        bottom = 0;
    scene = r_u32(0x8008A4E8);
    return p2_func_80066bdc((uint32)bottom << 16, ((extent + 1u) << 16) | 320u, r_u32(scene + 140));
}

uint32 p2_func_8005fb40(void)
{
    uint32 scene, background, saved_color, width, height, table, map, cursor, command, kind, count;
    uint32 index, elapsed, tick, destination, angle, palette, tile, definition, ordering, packet;
    uint32 uv0, uv1, attributes, temporary, xy, next_xy, next_row, row_start, result, remaining;
    sint32 vertical, horizontal, row, column, x, y, rows, columns, current_column;

    result = r_u32(0x8008A52C) & 0x800;
    if (result != 0)
        return result;
    p2_func_800665b0();
    if (r_u16(0x8008A532) == 4096)
    {
        scene = r_u32(0x8008A4E8);
        saved_color = r_u32(scene + 136);
        w_u32(scene + 136, 0);
        p2_func_80064dbc(240, 1);
        result = r_u32(0x8008A4E8);
        w_u32(result + 136, saved_color);
        return result;
    }
    scene = r_u32(0x8008A4E8);
    background = r_u32(scene + 124);
    if (background == 0)
        return p2_func_80066bdc(0, 0x00F00140, r_u32(scene + 136));
    if ((r_u16(scene + 18) & 0x40) != 0)
    {
        w_u16(scene + 18, r_u16(scene + 18) & 0xFFBF);
        temporary = (uint32)(sint32)r_s8(scene + 15);
        table = r_u32(background + 36);
        remaining = r_u16(background + 34);
        if (temporary == 2)
        {
            do
            {
                table += 4;
                --remaining;
                w_u32(table, r_u32(table) & 0xBFFFFFEF);
                table += 4;
            } while (remaining != 0);
        }
        else
        {
            do
            {
                table += 4;
                --remaining;
                w_u32(table, r_u32(table) | 0x40000010);
                table += 4;
            } while (remaining != 0);
        }
    }
    width = r_u16(background + 24);
    height = r_u16(background + 26);
    if (r_u32(0x8008A358) != 0x000E9555)
    {
        w_u32(0x8008A358, 0x000E9555);
        result = p2_sdk_8007e794(0x00170C59);
        result = (uint32)p2_func_8005dcd4(30720, (sint32)result);
        w_u16(0x8008A354, (0u - result) & 0xFFF);
    }
    angle = (1024u + r_u16(r_u32(0x8008A114) + 40) + r_u16(0x8008A354)) & 0xFFF;
    if (angle >= 0x801)
        angle |= 0xF000;
    vertical = (640 * (sint32)(sint16)angle) / 2048;
    row = vertical / 16 - ((sint32)(40u - height) >> 1);
    y = -(vertical & 15);
    p2_func_80064dbc((sint32)((uint32)y - ((uint32)row << 4) - 1u), height << 4);
    angle = (0u - (uint32)(sint32)r_s16(r_u32(0x8008A114) + 42)) & 0xFFF;
    horizontal = (sint32)(angle * width) / 256;
    column = horizontal / 16;
    x = -(horizontal & 15);
    map = r_u32(background + 28);
    if (r_u16(background + 4) != 0)
    {
        elapsed = r_u32(0x8008A434) - r_u32(background + 20);
        tick = (elapsed * (uint32)r_u16(background + 6)) / 1000u;
        if (tick != r_u32(background + 16))
        {
            index = (r_u16(background + 2) + 1u) & 0xFFFF;
            count = r_u16(background + 4);
            w_u32(background + 16, tick);
            w_u16(background + 2, index);
            if (count < index)
            {
                cursor = r_u32(background + 8);
                w_u16(background + 2, 0);
            }
            else
                cursor = r_u32(background + 12);
            command = ((uint32)r_u16(cursor + 2) << 16) | r_u16(cursor);
            cursor += 4;
            while (command != 0)
            {
                kind = command >> 30;
                if (kind == 0)
                {
                    count = (((command >> 20) & 0x3FF) + 1u) * (((command >> 10) & 0x3FF) + 1u) * 2u;
                    p2_func_8006637c((uint8 *)psx_addr(map, count), (const uint8 *)psx_addr(cursor, count), count);
                    cursor += count;
                    break;
                }
                destination = map + 2u * (((command >> 20) & 0x3FF) * height + ((command >> 10) & 0x3FF));
                count = (command & 0x3FF) + 1u;
                if (kind == 1)
                {
                    do
                    {
                        temporary = r_u16(cursor);
                        cursor += 2;
                        --count;
                        w_u16(destination, temporary);
                        destination += height * 2;
                    } while (count != 0);
                }
                else if (kind == 2)
                {
                    count *= 2;
                    p2_func_8006637c((uint8 *)psx_addr(destination, count), (const uint8 *)psx_addr(cursor, count), count);
                    cursor += count;
                }
                command = ((uint32)r_u16(cursor + 2) << 16) | r_u16(cursor);
                cursor += 4;
            }
            w_u32(background + 12, cursor);
        }
    }
    if (column < 0)
        column = (sint32)((uint32)column + width);
    if (row < 0)
    {
        y = (sint32)((uint32)y - ((uint32)row << 4));
        row = 0;
    }
    index = r_u16(r_u32(0x8008A4E4) + 262);
    if (index == 78 || (index == 14 && r_s8(r_u32(0x8008A4E8) + 14) == 0))
    {
        index = r_u8(0x8008A35C);
        column = (sint32)(index + (uint32)column);
        w_u8(0x8008A35C, index + 1);
        if (((index + 1) & 0xFF) == width)
            w_u8(0x8008A35C, 0);
        if (column >= (sint32)width)
            column = (sint32)((uint32)column - width);
    }
    rows = (sint32)(255u - (uint32)y) / 16;
    vertical = (sint32)(height - (uint32)row);
    if (vertical < rows)
        rows = vertical;
    result = 335;
    if (rows <= 0)
        return result;
    result = 335u - (uint32)x;
    columns = (sint32)result / 16;
    if ((sint32)result < 0)
        result += 15;
    if (columns <= 0)
        return result;
    next_row = ((uint32)y << 16) + 0x100000;
    table = r_u32(background + 36);
    row_start = map + 2u * (uint32)column * height + 2u * (uint32)row;
    do
    {
        current_column = column;
        cursor = row_start;
        horizontal = x;
        remaining = (uint32)columns;
        do
        {
            if (current_column == (sint32)width)
            {
                current_column = 0;
                cursor = map + 2u * (uint32)row;
            }
            tile = r_u16(cursor);
            xy = ((uint32)horizontal & 0xFFFF) | ((uint32)y << 16);
            index = (tile & 0x1FFF) * 8;
            definition = table + index;
            ordering = r_u32(0x8008A450) + (index >> 1);
            if ((tile & 0xC000) != 0)
            {
                uv0 = r_u32(definition);
                attributes = r_u32(definition + 4);
                uv1 = uv0 >> 16;
                if ((tile & 0x4000) != 0)
                {
                    temporary = uv1 & 0xFF;
                    uv1 = (uv1 & 0xFFFFFF00) | (uv0 & 0xFF);
                    uv0 = (uv0 & 0xFFFFFF00) | temporary;
                }
                if ((tile & 0x8000) != 0)
                {
                    temporary = uv1 & 0xFF00;
                    uv1 = (uv1 & 0xFFFF00FF) | (uv0 & 0xFF00);
                    uv0 = (uv0 & 0xFFFF00FF) | temporary;
                }
                uv0 = (uv0 & 0xFFFF) | (attributes & 0xFFFF0000);
                palette = (uv1 & 0xFF00) | (uv0 & 0xFF);
                next_xy = (((uint32)horizontal + 16u) & 0xFFFF) | next_row;
                temporary = r_u32(ordering + 4);
                packet = r_u32(0x8008A500);
                w_u32(packet + 4, 0x2D000000);
                w_u32(packet, (temporary & 0xFFFFFF) | 0x09000000);
                w_u32(packet + 16, (next_xy & 0xFFFF) | (xy & 0xFFFF0000));
                w_u32(packet + 20, (uv0 & 0xFF00) | (uv1 & 0xFF) | (attributes << 16));
                w_u32(packet + 8, xy);
                w_u32(packet + 12, uv0);
                w_u32(packet + 24, (xy & 0xFFFF) | (next_xy & 0xFFFF0000));
                w_u32(packet + 28, palette);
                w_u32(packet + 32, next_xy);
                w_u32(packet + 36, uv1);
                temporary = r_u32(ordering + 4);
                destination = packet + 40;
            }
            else
            {
                uv0 = r_u32(definition);
                attributes = r_u32(definition + 4);
                packet = r_u32(0x8008A500);
                w_u32(packet, ((packet + 12) & 0xFFFFFF) | 0x02000000);
                w_u32(packet + 4, (attributes & 0xFFFF) | 0xE1000000);
                w_u32(packet + 8, 0);
                w_u32(packet + 12, (r_u32(ordering + 4) & 0xFFFFFF) | 0x03000000);
                w_u32(packet + 16, 0x7D000000);
                w_u32(packet + 20, xy);
                w_u32(packet + 24, (uv0 & 0xFFFF) | (attributes & 0xFFFF0000));
                temporary = r_u32(ordering + 4);
                destination = packet + 28;
            }
            w_u32(ordering + 4, (temporary & 0xFF000000) | (packet & 0xFFFFFF));
            w_u32(0x8008A500, destination);
            current_column = (sint32)((uint32)current_column + 1u);
            horizontal = (sint32)((uint32)horizontal + 16u);
            --remaining;
            cursor += height * 2;
        } while (remaining != 0);
        row = (sint32)((uint32)row + 1u);
        result = 0x100000;
        next_row += 0x100000;
        y = (sint32)((uint32)y + 16u);
        --rows;
        row_start += 2;
    } while (rows != 0);
    return result;
}

uint32 p2_func_8005d170(void)
{
    uint32 object, result;
    p2_func_8005fb40();
    object = p2_func_800536f0(0);
    p2_func_80065bc4(object);
    result = p2_func_8005cf3c();
    p2_func_800606f4();
    return result;
}

uint32 p2_func_80063a80(uint32 node_offset)
{
    uint32 saved_stack, corners, packet, matrix, x, y, z, value, sum, limit, result, context, end, start, link, bucket_offset;
    uint32 material, texture, flags, uv0, uv1, uv2, uv3;
    sint32 height, bucket, distance, fade;
    sint64 product;
    packet = r_u32(0x8008A4F8);
    limit = r_u32(0x8008A4A0);
    if (packet >= limit)
        return 0;
    saved_stack = p2_guest_stack_pointer;
    p2_guest_stack_pointer -= 0x20;
    corners = p2_guest_stack_pointer;
    matrix = r_u32(0x8008A0C8);
    x = r_u32(matrix + 20);
    y = r_u32(matrix + 24);
    z = r_u32(matrix + 28);
    xport_gte_write_control(5, (uint32)((sint32)x >> 5));
    xport_gte_write_control(6, (uint32)((sint32)y >> 5));
    xport_gte_write_control(7, (uint32)((sint32)z >> 5));
    value = (uint32)(sint32)r_s16(node_offset + 40);
    height = r_s16(node_offset + 42);
    w_u16(corners + 2, 0);
    w_u16(corners + 10, 0);
    w_u16(corners + 18, 0);
    w_u16(corners + 26, 0);
    x = 0 - (value & 0xFFFF);
    z = 0 - (uint32)height;
    w_u16(corners, x);
    w_u16(corners + 4, z);
    w_u16(corners + 8, x);
    w_u16(corners + 12, height);
    w_u16(corners + 16, value);
    w_u16(corners + 20, height);
    w_u16(corners + 24, value);
    w_u16(corners + 28, z);
    xport_gte_write_data(0, r_u32(corners));
    xport_gte_write_data(1, r_u16(corners + 4));
    xport_gte_write_data(2, r_u32(corners + 8));
    xport_gte_write_data(3, r_u16(corners + 12));
    xport_gte_write_data(4, r_u32(corners + 16));
    xport_gte_write_data(5, r_u16(corners + 20));
    xport_gte_execute(0x280030);
    w_u32(corners, xport_gte_read_data(12));
    w_u32(corners + 8, xport_gte_read_data(13));
    w_u32(corners + 16, xport_gte_read_data(14));
    w_u32(corners + 4, xport_gte_read_data(17));
    w_u32(corners + 12, xport_gte_read_data(18));
    w_u32(corners + 20, xport_gte_read_data(19));
    xport_gte_write_data(0, r_u32(corners + 24));
    xport_gte_write_data(1, r_u16(corners + 28));
    xport_gte_execute(0x180001);
    w_u32(corners + 24, xport_gte_read_data(14));
    w_u32(corners + 28, xport_gte_read_data(19));
    if (r_u16(corners) >= 320 && r_u16(corners + 8) >= 320 && r_u16(corners + 16) >= 320 && r_u16(corners + 24) >= 320)
    {
        result = 0;
        goto done;
    }
    if (r_u16(corners + 2) >= 240 && r_u16(corners + 10) >= 240 && r_u16(corners + 18) >= 240 && r_u16(corners + 26) >= 240)
    {
        result = 0;
        goto done;
    }
    x = r_u32(corners + 4);
    y = r_u32(corners + 12);
    z = r_u32(corners + 20);
    value = r_u32(corners + 28);
    limit = r_u32(0x1F8002B4);
    sum = x + y + z + value;
    result = (sint32)(sum << 1) < (sint32)limit;
    if (result == 0)
        goto done;
    value = (uint32)(sint32)r_s16(0x1F8002A6);
    x = r_u32(0x1F8002B8);
    value += (uint32)((sint32)sum >> 2) + 128;
    bucket = (sint32)(value * x) >> 19;
    if ((uint32)bucket >= 1024)
        bucket = bucket < 0 ? 0 : 1023;
    xport_gte_write_data(8, 0);
    context = r_u32(0x8008A4E8);
    end = r_u32(context + 152);
    if (end != 0)
    {
        value = r_u32(0x1F8002B4);
        product = (sint64)(sint32)value * (sint32)((uint32)bucket << 2);
        distance = (sint32)((uint64)product >> 12);
        start = r_u32(context + 148);
        if ((sint32)start < distance)
        {
            result = (uint32)distance - start;
            if (distance >= (sint32)end)
                goto done;
            product = (sint64)r_s32(0x1F8002BC) * (sint32)result;
            fade = (sint32)((uint64)product >> 12) >> 8;
            xport_gte_write_data(8, (uint32)fade);
        }
        else
            xport_gte_write_data(8, 0);
    }
    else
        xport_gte_write_data(8, 0);
    bucket_offset = r_u32(0x8008A44C) + (uint32)bucket * 4;
    link = r_u32(bucket_offset);
    packet = r_u32(0x8008A4F8);
    w_u32(packet, (link & 0xFFFFFF) | 0x09000000);
    link = r_u32(bucket_offset);
    w_u32(bucket_offset, (link & 0xFF000000) | (packet & 0xFFFFFF));
    material = r_u32(node_offset + 16);
    packet = r_u32(0x8008A4F8);
    texture = r_u32(material + 4);
    uv0 = texture & 0xFFFF;
    uv3 = texture >> 16;
    texture = r_u32(material + 8);
    flags = r_u32(material);
    uv0 |= texture & 0xFFFF0000;
    uv1 = (uv0 & 0xFF00) | (uv3 & 0xFF) | (texture << 16);
    if ((flags & 0x80) != 0)
        uv1 |= (flags & 0x60) << 16;
    uv2 = (uv3 & 0xFF00) | (uv0 & 0xFF);
    xport_gte_write_data(6, r_u32(material + 16));
    xport_gte_execute(0x780010);
    w_u32(packet + 4, xport_gte_read_data(22));
    w_u32(packet + 12, uv0);
    w_u32(packet + 20, uv1);
    w_u32(packet + 28, uv2);
    w_u32(packet + 36, uv3);
    w_u32(packet + 8, r_u32(corners));
    w_u32(packet + 16, r_u32(corners + 8));
    w_u32(packet + 24, r_u32(corners + 24));
    w_u32(packet + 32, r_u32(corners + 16));
    result = packet + 40;
    w_u32(0x8008A4F8, result);
done:
    p2_guest_stack_pointer = saved_stack;
    return result;
}

uint32 p2_func_80011e04(void)
{
    uint32 matrix, flags, x, y, z, scene, node, geometry, vertices, output, remaining, first_index, value, result;
    matrix = r_u32(0x8008A0C8);
    flags = r_u32(0x1F800320);
    w_u32(0x8008A0C8, matrix + 32);
    w_u32(0x1F800320, flags | 0x200);
    x = r_u32(0x1F800264);
    y = r_u32(0x1F800268);
    xport_gte_write_control(0, x);
    xport_gte_write_control(1, y);
    x = r_u32(0x1F80026C);
    y = r_u32(0x1F800270);
    z = r_u32(0x1F800274);
    xport_gte_write_control(2, x);
    xport_gte_write_control(3, y);
    xport_gte_write_control(4, z);
    x = r_u32(0x1F800264);
    y = r_u32(0x1F800268);
    z = r_u32(0x1F80026C);
    w_u32(matrix + 32, x);
    w_u32(matrix + 36, y);
    w_u32(matrix + 40, z);
    x = r_u32(0x1F800270);
    y = r_u32(0x1F800274);
    z = r_u32(0x1F800278);
    w_u32(matrix + 44, x);
    w_u32(matrix + 48, y);
    w_u32(matrix + 52, z);
    x = r_u32(0x1F80027C);
    y = r_u32(0x1F800280);
    w_u32(matrix + 56, x);
    w_u32(matrix + 60, y);
    matrix = r_u32(0x8008A0C8);
    scene = r_u32(0x8008A114);
    x = r_u32(scene + 4);
    y = r_u32(scene + 8);
    z = r_u32(scene + 12);
    w_u32(matrix + 84, x);
    w_u32(matrix + 88, y);
    w_u32(matrix + 92, z);
    node = r_u32(r_u32(0x8008A4E8) + 36);
    while (node != 0)
    {
        flags = r_u32(node + 28);
        value = r_u8(node + 13);
        flags |= 0x200;
        w_u32(node + 28, flags);
        if ((value & 1) != 0)
            goto next_node;
        geometry = node;
        if (r_s16(node + 34) <= 0)
        {
            geometry = r_u32(node + 40);
            if ((r_u8(geometry + 13) & 1) != 0)
                goto next_node;
        }
        if (geometry == node && (flags & 0x2000) != 0)
        {
            w_u32(node + 28, flags & 0xFFFFBFFF);
            goto next_node;
        }
        value = r_u16(node + 14);
        if ((p2_func_80011338(node + 16, value << 5) << 24) == 0)
            goto next_node;
        flags = r_u32(node + 28);
        w_u32(node + 28, flags & 0xFFFFFDFF);
        if ((flags & 0x80) != 0)
            goto next_node;
        if (geometry != node && (r_u32(geometry + 28) & 0x4000) != 0)
            goto render_node;
        matrix = r_u32(0x8008A0C8);
        x = r_u32(geometry + 16);
        y = r_u32(geometry + 20);
        z = r_u32(geometry + 24);
        w_u32(matrix + 52, x);
        w_u32(matrix + 56, y);
        w_u32(matrix + 60, z);
        p2_func_800275bc(r_u32(geometry + 44));
        matrix = r_u32(0x8008A0C8);
        p2_sdk_8007ec64(matrix + 52, matrix + 20);
        matrix = r_u32(0x8008A0C8);
        x = r_u32(0x1F800278);
        w_u32(matrix + 20, r_u32(matrix + 20) + x);
        w_u32(matrix + 24, r_u32(matrix + 24) + r_u32(0x1F80027C));
        w_u32(matrix + 28, r_u32(matrix + 28) + r_u32(0x1F800280));
        w_u32(0x1F800258, r_u32(matrix + 84) - r_u32(matrix + 52));
        w_u32(0x1F80025C, r_u32(matrix + 88) - r_u32(matrix + 56));
        w_u32(0x1F800260, r_u32(matrix + 92) - r_u32(matrix + 60));
        value = r_u16(0x1F8002CC);
        output = r_u32(0x1F8002C8) + (value << 3);
        w_u32(0x1F8002C8, output);
        x = r_u32(matrix + 20);
        y = r_u32(matrix + 24);
        z = r_u32(matrix + 28);
        xport_gte_write_control(5, (uint32)((sint32)x >> 5));
        xport_gte_write_control(6, (uint32)((sint32)y >> 5));
        xport_gte_write_control(7, (uint32)((sint32)z >> 5));
        remaining = (uint32)(sint32)r_s16(geometry + 34) - 3;
        first_index = r_u16(geometry + 50);
        vertices = r_u32(geometry + 40) + (first_index << 4);
        if ((sint32)remaining >= 0)
        {
            do
            {
                xport_gte_write_data(0, r_u32(vertices));
                xport_gte_write_data(1, r_u32(vertices + 4));
                xport_gte_write_data(2, r_u32(vertices + 16));
                xport_gte_write_data(3, r_u32(vertices + 20));
                xport_gte_write_data(4, r_u32(vertices + 32));
                xport_gte_write_data(5, r_u32(vertices + 36));
                xport_gte_execute(0x280030);
                w_u32(output, xport_gte_read_data(12));
                w_u32(output + 8, xport_gte_read_data(13));
                w_u32(output + 16, xport_gte_read_data(14));
                w_u32(output + 4, xport_gte_read_data(17));
                w_u32(output + 12, xport_gte_read_data(18));
                w_u32(output + 20, xport_gte_read_data(19));
                vertices += 48;
                remaining -= 3;
                output += 24;
            } while ((sint32)remaining >= 0);
        }
        remaining += 3;
        while (remaining != 0)
        {
            xport_gte_write_data(0, r_u32(vertices));
            xport_gte_write_data(1, r_u32(vertices + 4));
            xport_gte_execute(0x180001);
            w_u32(output, xport_gte_read_data(14));
            w_u32(output + 4, xport_gte_read_data(19));
            vertices += 16;
            output += 8;
            --remaining;
        }
        first_index = r_u16(geometry + 50);
        output = r_u32(0x1F8002C8);
        w_u16(0x1F8002CE, first_index);
        w_u16(0x1F8002CC, first_index);
        w_u32(0x1F8002C8, output - (first_index << 3));
        if (geometry != node)
            w_u32(geometry + 28, r_u32(geometry + 28) | 0x4000);
    render_node:
        scene = r_u32(0x8008A4E8);
        value = r_u16(node + 48);
        output = r_u32(scene + 92);
        matrix = r_u32(0x8008A0C8);
        w_u32(0x1F8002AC, output + (value << 2));
        x = r_u32(node + 16);
        y = r_u32(node + 20);
        z = r_u32(node + 24);
        w_u32(matrix - 12, x);
        w_u32(matrix - 8, y);
        w_u32(matrix - 4, z);
        p2_func_8002e824(node + 16, r_u16(node + 14));
        if ((r_u32(node + 28) & 1) == 0)
            p2_func_80061f50(r_u32(geometry + 40), r_u32(node + 36), r_s16(node + 32));
        else
        {
            p2_func_800615c4(r_u32(geometry + 40), r_u32(node + 36), r_s16(node + 32));
            w_u32(node + 28, r_u32(node + 28) & 0xFFFFFFFE);
        }
    next_node:
        node = r_u32(node);
    }
    flags = r_u32(0x1F800320);
    matrix = r_u32(0x8008A0C8);
    result = flags & 0xFFFFFDFF;
    w_u32(0x1F800320, result);
    w_u32(0x8008A0C8, matrix - 32);
    return result;
}

uint32 p2_func_80060c0c(uint32 object_offset)
{
    uint32 value, flags, index, byte, condition;
    sint32 mode, threshold, step;
    if (r_u32(object_offset + 108) != 0)
    {
        if ((p2_func_8005bc58(object_offset + 108, 1) << 24) != 0)
        {
            if (r_u32(object_offset + 112) != 0)
            {
                index = (uint32)(sint32)r_s16(object_offset + 128);
                value = r_u8(0x800B9938 + index);
                value += r_u32(object_offset + 116);
                if ((sint32)value < 0)
                    value = 0;
                else if ((sint32)value >= 100)
                    value = 99;
            }
            else
                value = r_u32(object_offset + 116);
            index = (uint32)(sint32)r_s16(object_offset + 128);
            w_u8(0x800B9938 + index, value);
        }
    }
    else
    {
        mode = r_s32(object_offset + 112);
        index = (uint32)(sint32)r_s16(object_offset + 128);
        value = r_u8(0x800B9938 + index);
        threshold = r_s32(object_offset + 116);
        if (mode == 0)
            condition = value == (uint32)threshold;
        else if (mode < 0)
            condition = (sint32)value < threshold;
        else
            condition = threshold < (sint32)value;
        if (condition != 0)
        {
            if ((r_u8(object_offset + 13) & 1) != 0)
            {
                flags = r_u32(object_offset + 28);
                byte = r_u8(object_offset + 13);
                w_u32(object_offset + 28, flags | 0x80000);
                w_u8(object_offset + 13, byte & 0xFE);
            }
        }
        else if ((r_u8(object_offset + 13) & 1) == 0)
        {
            flags = r_u32(object_offset + 28);
            byte = r_u8(object_offset + 13);
            w_u32(object_offset + 28, flags | 0x4000);
            w_u8(object_offset + 13, byte | 1);
        }
    }
    flags = r_u32(object_offset + 28);
    value = flags & 0x840000;
    if (value == 0)
        return value;
    if ((r_u8(object_offset + 13) & 1) != 0)
        return 0x200000;
    step = (flags & 0x200000) != 0 ? -1 : 1;
    w_u16(object_offset + 136, step);
    return p2_func_8001b334(object_offset, step);
}
