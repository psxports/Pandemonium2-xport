#include "game.h"

static const uint32 key_reciprocal[257] =
{
    0, 1048576, 524288, 349525, 262144, 209715, 174762, 149796,
    131072, 116508, 104857, 95325, 87381, 80659, 74898, 69905,
    65536, 61680, 58254, 55188, 52428, 49932, 47662, 45590,
    43690, 41943, 40329, 38836, 37449, 36157, 34952, 33825,
    32768, 31775, 30840, 29959, 29127, 28339, 27594, 26886,
    26214, 25575, 24966, 24385, 23831, 23301, 22795, 22310,
    21845, 21399, 20971, 20560, 20164, 19784, 19418, 19065,
    18724, 18396, 18078, 17772, 17476, 17189, 16912, 16644,
    16384, 16131, 15887, 15650, 15420, 15196, 14979, 14768,
    14563, 14364, 14169, 13981, 13797, 13617, 13443, 13273,
    13107, 12945, 12787, 12633, 12483, 12336, 12192, 12052,
    11915, 11781, 11650, 11522, 11397, 11275, 11155, 11037,
    10922, 10810, 10699, 10591, 10485, 10381, 10280, 10180,
    10082, 9986, 9892, 9799, 9709, 9619, 9532, 9446,
    9362, 9279, 9198, 9118, 9039, 8962, 8886, 8811,
    8738, 8665, 8594, 8525, 8456, 8388, 8322, 8256,
    8192, 8128, 8065, 8004, 7943, 7884, 7825, 7767,
    7710, 7653, 7598, 7543, 7489, 7436, 7384, 7332,
    7281, 7231, 7182, 7133, 7084, 7037, 6990, 6944,
    6898, 6853, 6808, 6765, 6721, 6678, 6636, 6594,
    6553, 6512, 6472, 6432, 6393, 6355, 6316, 6278,
    6241, 6204, 6168, 6132, 6096, 6061, 6026, 5991,
    5957, 5924, 5890, 5857, 5825, 5793, 5761, 5729,
    5698, 5667, 5637, 5607, 5577, 5548, 5518, 5489,
    5461, 5433, 5405, 5377, 5349, 5322, 5295, 5269,
    5242, 5216, 5190, 5165, 5140, 5115, 5090, 5065,
    5041, 5017, 4993, 4969, 4946, 4922, 4899, 4877,
    4854, 4832, 4809, 4788, 4766, 4744, 4723, 4702,
    4681, 4660, 4639, 4619, 4599, 4578, 4559, 4539,
    4519, 4500, 4481, 4462, 4443, 4424, 4405, 4387,
    4369, 4350, 4332, 4315, 4297, 4279, 4262, 4245,
    4228, 4211, 4194, 4177, 4161, 4144, 4128, 4112,
    4096
};

uint32 InterpolateKeyTime(uint32 frame, uint32 end, uint32 start_key, uint32 end_key, uint32 keys_offset)
{
    FUNCTION_MARKER(0x80017030, "SLUS_005.78");
    uint32 elapsed = (frame >> 20) - (start_key >> 20);
    uint32 duration = (end >> 20) - (start_key >> 20);
    uint32 reciprocal = duration < 0x101 ? key_reciprocal[duration] : 0x100000u / duration;
    uint32 left = start_key & 0xFF;
    uint32 right = end_key & 0xFF;
    uint32 value = elapsed << 12;
    uint32 amplitude, threshold, left_curve, right_curve, coefficient;
    uint32 product, term;
    if ((left | right) == 0)
        goto finish;
    amplitude = ((start_key >> 8) & 0xFFF) + 2048;
    if (left != 0)
    {
        left_curve = keys_offset - left * 12;
        threshold = (uint32)r_s16(left_curve + 2) * duration;
        if ((sint32)value < (sint32)threshold)
        {
            coefficient = r_u32(left_curve + 8);
            goto ease_in;
        }
        if (right != 0)
        {
            right_curve = keys_offset - right * 12;
            if ((sint32)value >= (sint32)((uint32)r_s16(right_curve) * duration))
                goto ease_out;
        }
        else if ((sint32)value >= (sint32)(duration << 12))
        {
            value = duration << 12;
            goto finish;
        }
        term = (uint32)(((sint64)(sint32)amplitude * (sint32)threshold) >> 12);
        value = (amplitude << 1) * elapsed - term;
        goto finish;
    }
    right_curve = keys_offset - right * 12;
    if ((sint32)value < (sint32)((uint32)r_s16(right_curve) * duration))
    {
        value = (amplitude << 1) * elapsed;
        goto finish;
    }
ease_out:
    elapsed = duration - elapsed;
    coefficient = r_u32(right_curve + 4);
    product = amplitude * (elapsed * elapsed);
    term = (uint32)(((sint64)(sint32)product * (sint32)coefficient) >> 12);
    term = (uint32)(((sint64)(sint32)term * (sint32)reciprocal) >> 12);
    value = (duration << 12) - (uint32)((sint32)term >> 8);
    goto finish;
ease_in:
    product = amplitude * (elapsed * elapsed);
    term = (uint32)(((sint64)(sint32)product * (sint32)coefficient) >> 12);
    term = (uint32)(((sint64)(sint32)term * (sint32)reciprocal) >> 12);
    value = (uint32)((sint32)term >> 8);
finish:
    return (uint32)(((sint64)(sint32)value * (sint32)reciprocal) >> 12);
}

uint32 SampleVectorTrack(sint32 frame, uint32 track_offset, uint32 destination_offset)
{
    FUNCTION_MARKER(0x80017364, "SLUS_005.78");
    uint32 keys = r_u32(track_offset + 12);
    uint32 previous = keys + (uint32)r_u8(track_offset) * 40 - 40;
    uint32 next = keys + 40;
    uint32 frame_value = ((uint32)frame << 20) | 0xFFFFF;
    uint32 end_value = r_u32(previous);
    uint32 value, square, cube;
    uint32 previous_weight, next_weight, previous_tangent_weight, next_tangent_weight;
    uint32 flags, result = 0, i;
    if (frame_value < end_value)
    {
        do
        {
            end_value = r_u32(next);
            next += 40;
        } while (frame_value >= end_value);
        next -= 40;
        previous = next - 40;
    }
    else
    {
        next -= 40;
        if (frame_value == (end_value | 0xFFFFF))
            goto copy_key;
        end_value = ((uint32)r_s16(track_offset + 4) << 20) | 0xFFFFF;
        if (frame_value >= end_value)
        {
            if ((r_u8(track_offset + 2) & 3) == 3)
                previous = next;
            goto copy_key;
        }
    }
    value = InterpolateKeyTime(frame_value, end_value, r_u32(previous), r_u32(next), r_u32(track_offset + 12));
    square = (uint32)(((sint64)(sint32)value * (sint32)value) >> 12) >> 8;
    cube = (uint32)(((sint64)(sint32)square * (sint32)value) >> 12) >> 8;
    next_tangent_weight = (uint32)((sint32)(cube - square) >> 8);
    next_weight = (uint32)((sint32)(3 * square - 2 * cube) >> 8);
    previous_tangent_weight = (uint32)((sint32)(cube - square + value - square) >> 8);
    previous_weight = 4096u - next_weight;
    flags = r_u8(track_offset + 2);
    for (i = 0; i < 3; ++i)
    {
        uint32 first, second, tangent0, tangent1, sum;
        if ((flags & 0x20) != 0)
        {
            first = (uint32)(((sint64)r_s32(previous + 4 + i * 4) * (sint32)previous_weight) >> 12);
            second = (uint32)(((sint64)r_s32(next + 4 + i * 4) * (sint32)next_weight) >> 12);
            tangent0 = r_u32(previous + 16 + i * 4) * previous_tangent_weight;
            tangent1 = r_u32(next + 28 + i * 4) * next_tangent_weight;
            result = (uint32)((sint32)(tangent0 + tangent1) >> 12);
            sum = first + second + result;
        }
        else if ((flags & 0x10) != 0)
        {
            first = r_u32(previous + 4 + i * 4) * previous_weight;
            second = r_u32(next + 4 + i * 4) * next_weight;
            tangent0 = r_u32(previous + 16 + i * 4) * previous_tangent_weight;
            tangent1 = r_u32(next + 28 + i * 4) * next_tangent_weight;
            result = (uint32)((sint32)(first + second + tangent0 + tangent1) >> 12);
            sum = result;
        }
        else
        {
            first = (uint32)(((sint64)r_s32(previous + 4 + i * 4) * (sint32)previous_weight) >> 12);
            second = (uint32)(((sint64)r_s32(next + 4 + i * 4) * (sint32)next_weight) >> 12);
            tangent0 = (uint32)(((sint64)r_s32(previous + 16 + i * 4) * (sint32)previous_tangent_weight) >> 12);
            result = (uint32)(((sint64)r_s32(next + 28 + i * 4) * (sint32)next_tangent_weight) >> 12);
            sum = first + second + tangent0 + result;
        }
        w_u32(destination_offset + i * 4, sum);
    }
    return result;
copy_key:
{
    uint32 x = r_u32(previous + 4);
    uint32 y = r_u32(previous + 8);
    uint32 z = r_u32(previous + 12);
    w_u32(destination_offset, x);
    w_u32(destination_offset + 4, y);
    w_u32(destination_offset + 8, z);
    return x;
}
}

uint32 SampleWorldPath(uint32 definition_offset, sint32 frame, uint32 destination_offset, uint32 direction)
{
    FUNCTION_MARKER(0x800230F0, "SLUS_005.78");
    sint32 output;
    uint32 saved_stack = p2_guest_stack_pointer;
    uint32 matrix, vector, i, result;
    p2_guest_stack_pointer -= 0x20;
    vector = p2_guest_stack_pointer + 0x10;
    if (direction == 0)
        frame = (sint32)((uint32)r_s16(definition_offset + 26) - (uint32)frame - 1);
    matrix = r_u32(definition_offset + 40);
    if (matrix != 0)
    {
        output = SampleVectorTrack(frame, definition_offset + 20, vector);
        PushMatrix();
        for (i = 0; i < 5; ++i)
            xport_gte_write_control(i, r_u32(matrix + i * 4));
        p2_sdk_8007ec64(vector, destination_offset);
        PopMatrix();
        /* TODO: Bind the original matrix-stack diagnostic exits */
        for (i = 0; i < 3; ++i)
        {
            result = r_u32(destination_offset + i * 4) + r_u32(matrix + 20 + i * 4);
            w_u32(destination_offset + i * 4, result);
        }
    }
    else
    {
        result = SampleVectorTrack(frame, definition_offset + 20, destination_offset);
    }
    p2_guest_stack_pointer = saved_stack;
    return result;
}

uint32 SamplePathOffset(uint32 path_offset, sint32 frame, uint32 destination_offset, uint32 direction)
{
    FUNCTION_MARKER(0x80022C24, "SLUS_005.78");
    uint32 saved_stack = p2_guest_stack_pointer;
    uint32 matrix, vector, i, result;
    p2_guest_stack_pointer -= 0x20;
    vector = p2_guest_stack_pointer + 0x10;
    if ((r_u16(path_offset + 14) & (direction | 8)) == 0)
        frame = (sint32)((uint32)r_s16(r_u32(path_offset + 36) + 26) - (uint32)frame - 1);
    if (r_u32(path_offset + 32) != 0)
    {
        SampleVectorTrack(frame, path_offset + 40, vector);
        PushMatrix();
        matrix = r_u32(path_offset + 32);
        for (i = 0; i < 5; ++i)
            xport_gte_write_control(i, r_u32(matrix + i * 4));
        p2_sdk_8007ec64(vector, destination_offset);
        PopMatrix();
        for (i = 0; i < 3; ++i)
        {
            matrix = r_u32(path_offset + 32);
            result = r_u32(destination_offset + i * 4) + r_u32(matrix + 20 + i * 4);
            w_u32(destination_offset + i * 4, result);
        }
    }
    else
    {
        result = SampleVectorTrack(frame, path_offset + 40, destination_offset);
    }
    p2_guest_stack_pointer = saved_stack;
    return result;
}

uint32 SamplePathPosition(uint32 path_offset, sint32 frame, uint32 destination_offset, uint32 direction)
{
    FUNCTION_MARKER(0x80022B20, "SLUS_005.78");
    uint32 saved_stack = p2_guest_stack_pointer;
    uint32 definition, matrix, vector, i, result;
    p2_guest_stack_pointer -= 0x20;
    vector = p2_guest_stack_pointer + 0x10;
    if (direction == 0)
        frame = (sint32)((uint32)r_s16(r_u32(path_offset + 36) + 26) - (uint32)frame - 1);
    definition = r_u32(path_offset + 36);
    matrix = r_u32(definition + 40);
    if (matrix != 0)
    {
        SampleVectorTrack(frame, definition + 20, vector);
        PushMatrix();
        for (i = 0; i < 5; ++i)
            xport_gte_write_control(i, r_u32(matrix + i * 4));
        p2_sdk_8007ec64(vector, destination_offset);
        PopMatrix();
        for (i = 0; i < 3; ++i)
        {
            result = r_u32(destination_offset + i * 4) + r_u32(matrix + 20 + i * 4);
            w_u32(destination_offset + i * 4, result);
        }
    }
    else
    {
        result = SampleVectorTrack(frame, definition + 20, destination_offset);
    }
    p2_guest_stack_pointer = saved_stack;
    return result;
}

sint32 ProjectPathFrame(uint32 definition_offset, sint32 frame, uint32 position_offset, uint32 floor_offset, uint32 direction, sint32 scale)
{
    FUNCTION_MARKER(0x8003D3FC, "SLUS_005.78");
    uint32 saved_stack = p2_guest_stack_pointer;
    uint32 vector, side, i, x, y, z, floor_x, floor_z, point_x, point_z, input_x, input_z;
    uint32 dx, dz, point_dot, floor_dot, ratio, squared, limit, last_endpoint = 0;
    sint32 reciprocal, factor, result;
    p2_guest_stack_pointer -= 0x20;
    vector = p2_guest_stack_pointer + 0x10;
    if (frame > 0)
    {
        sint32 count = r_s16(definition_offset + 26);
        if (frame >= count)
            frame = count - 1;
    }
    else
        frame = 0;
    SampleWorldPath(definition_offset, frame, floor_offset, direction);
    for (side = 0; side < 2; ++side)
    {
        if (side == 0 && frame == 0)
            continue;
        SampleWorldPath(definition_offset, (sint32)((uint32)frame + (side == 0 ? 0xFFFFFFFF : 1)), vector, direction);
        floor_z = r_u32(floor_offset + 8);
        point_z = r_u32(vector + 8);
        point_x = r_u32(vector);
        floor_x = r_u32(floor_offset);
        input_x = r_u32(position_offset);
        dz = floor_z - point_z;
        dx = point_x - floor_x;
        x = point_x - input_x;
        point_dot = (uint32)(((sint64)(sint32)x * (sint32)dx) >> 12);
        input_z = r_u32(position_offset + 8);
        z = point_z - input_z;
        point_dot -= (uint32)(((sint64)(sint32)z * (sint32)dz) >> 12);
        x = floor_x - input_x;
        floor_dot = (uint32)(((sint64)(sint32)x * (sint32)dx) >> 12);
        z = floor_z - input_z;
        floor_dot -= (uint32)(((sint64)(sint32)z * (sint32)dz) >> 12);
        if (((point_dot ^ floor_dot) & 0x80000000) != 0 || point_dot + 16 < 33 || floor_dot + 16 < 33)
        {
            if (side != 0)
                frame = (sint32)((uint32)frame + 1);
            goto interpolate;
        }
        if (side == 0)
        {
            if (frame != r_s16(definition_offset + 26) - 1)
                continue;
            last_endpoint = 1;
        }
        else if (frame != 0)
        {
            result = (sint32)((uint32)frame + 1);
            goto done;
        }
        goto endpoint;
    }
    result = frame;
    goto done;
endpoint:
    if (((uint32)scale << 1) < r_u32(floor_offset) - r_u32(position_offset) + (uint32)scale && ((uint32)scale << 1) < r_u32(floor_offset + 8) - r_u32(position_offset + 8) + (uint32)scale)
    {
        result = -1;
        goto done;
    }
    if (point_dot == floor_dot)
    {
        for (i = 0; i < 3; ++i)
            w_u32(vector + i * 4, r_u32(floor_offset + i * 4) - r_u32(position_offset + i * 4));
    }
    else
    {
        reciprocal = 0x40000000 / (sint32)(point_dot - floor_dot);
        ratio = (uint32)(((sint64)(sint32)point_dot * reciprocal) >> 18);
        w_u32(0x1F80030C, ratio);
        x = r_u32(floor_offset) - r_u32(vector);
        w_u32(vector, x);
        y = r_u32(floor_offset + 4) - r_u32(vector + 4);
        w_u32(vector + 4, y);
        z = r_u32(floor_offset + 8) - r_u32(vector + 8);
        w_u32(vector + 8, z);
        factor = (sint32)(ratio - 4096);
        w_u32(vector, (uint32)(((sint64)(sint32)x * factor) >> 12));
        w_u32(vector + 4, (uint32)(((sint64)(sint32)y * factor) >> 12));
        w_u32(vector + 8, (uint32)(((sint64)(sint32)z * factor) >> 12));
    }
    x = (uint32)(r_s32(vector) >> 6);
    z = (uint32)(r_s32(vector + 8) >> 6);
    w_u32(vector + 4, 0);
    squared = x * x + z * z;
    limit = (uint32)(((sint64)scale * scale) >> 12);
    if (last_endpoint != 0)
        result = (sint32)limit < (sint32)squared ? -1 : r_s16(definition_offset + 26);
    else
        result = (sint32)limit < (sint32)squared ? -1 : 0;
    goto done;
interpolate:
    if (point_dot != floor_dot)
    {
        reciprocal = 0x40000000 / (sint32)(floor_dot - point_dot);
        ratio = (uint32)(((sint64)(sint32)floor_dot * reciprocal) >> 18);
        w_u32(0x1F80030C, ratio);
        x = r_u32(vector) - r_u32(floor_offset);
        w_u32(vector, x);
        y = r_u32(vector + 4) - r_u32(floor_offset + 4);
        w_u32(vector + 4, y);
        z = r_u32(vector + 8) - r_u32(floor_offset + 8);
        w_u32(vector + 8, z);
        w_u32(vector, (uint32)(((sint64)(sint32)x * (sint32)ratio) >> 12));
        w_u32(vector + 4, (uint32)(((sint64)(sint32)y * (sint32)ratio) >> 12));
        w_u32(vector + 8, (uint32)(((sint64)(sint32)z * (sint32)ratio) >> 12));
        for (i = 0; i < 3; ++i)
            w_u32(floor_offset + i * 4, r_u32(floor_offset + i * 4) + r_u32(vector + i * 4));
    }
    result = frame;
done:
    p2_guest_stack_pointer = saved_stack;
    return result;
}

static uint32 FindLinkedPathHeightFrame(uint32 definition_offset, uint32 position_offset, uint32 direction, sint32 height_offset)
{
    FUNCTION_MARKER(0x80024DE0, "SLUS_005.78");
    uint32 saved_stack, delta, first, second, sample, temporary, matrix, value, x, y, z;
    uint32 first_distance, second_distance, distance, best_distance, radius, extent, effective_direction;
    sint32 frames, low, high, frame, best_frame, nearer_first, choose_high;
    sint32 c0, c1, c2, c3, c4, c5, c6, c7, c8;
    if (!r_u32(definition_offset + 32))
    {
        return 0;
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
            SampleVectorTrack(0, definition_offset + 20, temporary);
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
            SampleVectorTrack(0, definition_offset + 20, first);
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
            SampleVectorTrack(high, definition_offset + 20, temporary);
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
            SampleVectorTrack(high, definition_offset + 20, second);
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
            return -1;
        }
        for (;;)
        {
            matrix = r_u32(definition_offset + 40);
            if (matrix)
            {
                SampleVectorTrack(frame, definition_offset + 20, temporary);
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
                SampleVectorTrack(frame, definition_offset + 20, sample);
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
                return frame;
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
        SampleVectorTrack(frame, definition_offset + 20, delta);
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
    return best_frame;
}

uint32 FindPathHeightFrame(uint32 definition_offset, uint32 position_offset, uint32 direction, sint32 height_offset)
{
    FUNCTION_MARKER(0x800231DC, "SLUS_005.78");
    uint32 saved_stack, point, left, right, middle, raw, query, flags, matrix, destination, index;
    uint32 x, y, z, a, b, c, d, dx, dy, dz, distance, left_distance, right_distance;
    uint32 first_distance, best_distance, radius, span, forward, outside, unresolved, move_left;
    sint32 best, remaining, frame, low, high, candidate, ca, cb, c0, c1, c2, c3, c4, c5, c6, c7, c8;
    sint64 product;
    if (r_u32(definition_offset + 32) == 0)
    {
        return 0;
    }
    flags = r_u32(definition_offset + 16);
    if ((flags & 0x10000) != 0)
        return FindLinkedPathHeightFrame(definition_offset, position_offset, direction, height_offset);
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
            SampleVectorTrack(frame, definition_offset + 20, point);
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
            SampleVectorTrack(0, definition_offset + 20, raw);
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
            w_u32(left, r_u32(left) + r_u32(matrix + 20));
            w_u32(left + 4, r_u32(left + 4) + r_u32(matrix + 24));
            w_u32(left + 8, r_u32(left + 8) + r_u32(matrix + 28));
        }
        else
        {
            SampleVectorTrack(0, definition_offset + 20, left);
        }
        for (;;)
        {
            matrix = r_u32(definition_offset + 40);
            if (matrix != 0)
            {
                SampleVectorTrack(frame, definition_offset + 20, raw);
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
                w_u32(point, r_u32(point) + r_u32(matrix + 20));
                w_u32(point + 4, r_u32(point + 4) + r_u32(matrix + 24));
                w_u32(point + 8, r_u32(point + 8) + r_u32(matrix + 28));
            }
            else
            {
                SampleVectorTrack(frame, definition_offset + 20, point);
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
            SampleVectorTrack(candidate, definition_offset + 20, raw);
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
            w_u32(destination, r_u32(destination) + r_u32(matrix + 20));
            w_u32(destination + 4, r_u32(destination + 4) + r_u32(matrix + 24));
            w_u32(destination + 8, r_u32(destination + 8) + r_u32(matrix + 28));
        }
        else
        {
            SampleVectorTrack(candidate, definition_offset + 20, destination);
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
            SampleVectorTrack(candidate, definition_offset + 20, raw);
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
            w_u32(middle, r_u32(middle) + r_u32(matrix + 20));
            w_u32(middle + 4, r_u32(middle + 4) + r_u32(matrix + 24));
            w_u32(middle + 8, r_u32(middle + 8) + r_u32(matrix + 28));
        }
        else
        {
            SampleVectorTrack(candidate, definition_offset + 20, middle);
        }
        destination = forward != 0 ? left : right;
        x = r_u32(destination);
        z = r_u32(destination + 8);
        a = r_u32(middle);
        b = r_u32(middle + 8);
        c = r_u32(position_offset);
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
    return best;
}

static uint32 CanChangePath(uint32 object_offset, uint32 position_offset, uint32 direction)
{
    FUNCTION_MARKER(0x8003D190, "SLUS_005.78");
    uint32 scene, flags, path, saved_stack, floor, x, y, z, result;
    sint32 frame;
    scene = r_u32(0x8008A114);
    flags = r_u32(object_offset + 228);
    path = r_u32(scene + 136);
    if ((flags & 6) == 2)
    {
        return 0;
    }
    saved_stack = p2_guest_stack_pointer;
    p2_guest_stack_pointer -= 16;
    floor = p2_guest_stack_pointer;
    if ((flags & 4) && p2_func_8005a430() != path)
    {
        frame = FindPathHeightFrame(r_u32(path + 36), position_offset, direction, -40960);
        if (frame != -1)
        {
            frame = ProjectPathFrame(r_u32(path + 36), frame, position_offset, floor, direction, 4096);
        }
        if (frame >= 0)
        {
            x = r_u32(floor);
            x -= r_u32(position_offset);
            if (x + 24576 > 0xC000)
                frame = -1;
            else
            {
                z = r_u32(floor + 8);
                z -= r_u32(position_offset + 8);
                if (z + 24576 > 0xC000)
                    frame = -1;
            }
        }
        if (frame != -1)
        {
            x = r_u32(floor);
            x -= r_u32(position_offset);
            y = r_u32(floor + 4);
            w_u32(floor, x);
            y -= r_u32(position_offset + 4);
            z = r_u32(floor + 8);
            w_u32(floor + 4, y);
            z -= r_u32(position_offset + 8);
            w_u32(floor + 8, z);
            if ((sint32)y <= 0)
            {
                p2_guest_stack_pointer = saved_stack;
                return 0;
            }
        }
    }
    result = 1;
    if ((r_u32(object_offset + 228) & 0x20) && (p2_func_8005a3fc() & 0xFF) == 0)
    {
        frame = FindPathHeightFrame(r_u32(path + 36), position_offset, direction, -40960);
        if (frame != -1)
        {
            frame = ProjectPathFrame(r_u32(path + 36), frame, position_offset, floor, direction, 4096);
        }
        if (frame >= 0)
        {
            x = r_u32(floor);
            x -= r_u32(position_offset);
            if (x + 24576 > 0xC000)
                frame = -1;
            else
            {
                z = r_u32(floor + 8);
                z -= r_u32(position_offset + 8);
                if (z + 24576 > 0xC000)
                    frame = -1;
            }
        }
        if (frame != -1)
        {
            x = r_u32(floor);
            x -= r_u32(position_offset);
            y = r_u32(floor + 4);
            w_u32(floor, x);
            y -= r_u32(position_offset + 4);
            z = r_u32(floor + 8);
            w_u32(floor + 4, y);
            z -= r_u32(position_offset + 8);
            w_u32(floor + 8, z);
            if ((sint32)y <= 0)
                result = 0;
        }
    }
    p2_guest_stack_pointer = saved_stack;
    return result;
}

static uint32 SelectAdjacentPath(uint32 segment_offset, uint32 frame_offset, uint32 direction, uint32 flags)
{
    FUNCTION_MARKER(0x8003ADC0, "SLUS_005.78");
    uint32 old_frame, current, selected, mask, definition, value, first_frame;
    old_frame = r_u32(frame_offset);
    /* Forced transitions use the same frame side as ordinary transitions */
    first_frame = (sint32)old_frame >= 0;
    if ((flags & 0x2000000) != 0)
    {
        mask = r_u16(segment_offset + 14);
        if ((mask & (((flags >> 6) & 0x10) | 8)) != 0)
            selected = r_u32(segment_offset + 20);
        else
            selected = r_u32(segment_offset + 16);
        if (selected != 0)
        {
            if ((first_frame & 0xFF) != 0)
                w_u32(frame_offset, 0);
            else
            {
                definition = r_u32(selected + 36);
                value = (uint32)r_s16(definition + 26) - 1;
                w_u32(frame_offset, value);
            }
        }
    }
    else
    {
        current = segment_offset;
        if (first_frame == 0)
        {
            if ((r_u16(segment_offset + 14) & 8) != 0)
                current = r_u32(segment_offset + 16);
            direction ^= 0x10;
        }
        mask = r_u16(current + 14);
        if ((mask & (direction | 8)) != 0)
            selected = r_u32(current + 20);
        else
            selected = r_u32(current + 16);
        if (selected != 0)
        {
            if (first_frame != 0)
                w_u32(frame_offset, 0);
            else
            {
                if ((r_u16(selected + 14) & 8) != 0)
                    selected = r_u32(selected + 16);
                definition = r_u32(selected + 36);
                value = (uint32)r_s16(definition + 26) - 1;
                w_u32(frame_offset, value);
            }
        }
    }
    if (selected != 0)
    {
        definition = r_u32(selected + 36);
        if ((r_u32(definition + 16) & 0x10000) != 0)
        {
            w_u32(frame_offset, old_frame);
            selected = 0;
        }
    }
    return selected;
}

uint32 ApplyPathVelocityChange(uint32 object_offset, uint32 definition_offset)
{
    FUNCTION_MARKER(0x80046228, "SLUS_005.78");
    uint32 result, parent, state, matrix, x, y, z, value, speed, angle, flags, secondary;
    sint32 kind;

    result = r_u32(object_offset + 28) & 0x2000;
    if (result != 0)
    {
        return result;
    }
    kind = r_s8(object_offset + 126);
    result = 3;
    parent = 0;
    if (kind == 1)
    {
        result = 33;
        if (r_s8(object_offset + 125) != 33)
        {
            state = r_u32(0x8008A114);
            result = state;
            parent = r_u32(state + 136);
        }
        else
            parent = r_u32(object_offset + 112);
    }
    else if (kind == 3)
    {
        result = 0xFFFFFFFF;
        if (r_s8(object_offset + 120) == -1)
            parent = r_u32(object_offset + 112);
    }
    if (parent == 0)
    {
        return result;
    }
    result = r_u32(parent + 36);
    matrix = r_u32(result + 40);
    if (matrix == 0)
    {
        return result;
    }
    x = r_u32(matrix + 32);
    y = r_u32(matrix + 36);
    z = r_u32(matrix + 40);
    if (definition_offset != 0)
    {
        matrix = r_u32(definition_offset + 40);
        if (matrix != 0)
        {
            x -= r_u32(matrix + 32);
            matrix = r_u32(definition_offset + 40);
            y -= r_u32(matrix + 36);
            matrix = r_u32(definition_offset + 40);
            z -= r_u32(matrix + 40);
        }
    }
    w_u32(object_offset + 92, r_u32(object_offset + 92) + x);
    w_u32(object_offset + 96, r_u32(object_offset + 96) + y);
    w_u32(object_offset + 100, r_u32(object_offset + 100) + z);
    if (r_s8(object_offset + 126) != 1)
    {
        return 1;
    }
    if (x == 0 && y == 0 && z == 0)
    {
        return 0;
    }
    if (y != 0)
    {
        value = character_controller.motion.vertical_speed;
        value += y;
        y = 0;
        character_controller.motion.vertical_speed = value;
        w_u32(object_offset + 28, r_u32(object_offset + 28) | 0x400);
    }
    value = (uint32)((sint32)x >> 6);
    value *= value;
    speed = (uint32)((sint32)y >> 6);
    value += speed * speed;
    speed = (uint32)((sint32)z >> 6);
    value += speed * speed;
    if ((sint32)value < 0)
    {
        speed = p2_sdk_8007e794(value >> 2) << 1;
    }
    else
    {
        speed = p2_sdk_8007e794(value);
    }

    if (speed != 0)
    {
        angle = Atan2Angle((sint32)z, (sint32)x);
        value = (uint32)(sint32)r_s16(object_offset + 150);
        angle = (uint32)(sint32)(sint16)angle - value - 1024;
        if ((angle & 0xFFF) < 2048)
        {
            if (speed != 0 && (r_u32(object_offset + 28) & 0x400) == 0)
                speed += 4096;
        }
        else
            speed = 0 - speed;
    }
    value = character_controller.motion.horizontal_speed;
    flags = character_controller.flags;
    secondary = character_controller.secondary_flags;
    character_controller.motion.horizontal_speed = value + speed;
    character_controller.flags = flags & 0xFF7FFFFF;
    character_controller.secondary_flags = secondary & 0xFFFFFF7F;
    return 0xFFFFFF7F;
}

uint32 ChangeActorPath(uint32 object_offset, uint32 path_offset, uint32 position_offset, uint32 floor_offset)
{
    FUNCTION_MARKER(0x8003AF18, "SLUS_005.78");
    uint32 saved_stack = p2_guest_stack_pointer;
    uint32 local, frame_offset, query_floor, input, inverse, transpose, visited;
    /* No candidate exists until this function selects a path */
    uint32 candidate = 0, visited_path;
    uint32 initial_path = path_offset, direction, flags, definition, parent, matrix;
    uint32 kind_one_probe = 0, restrict_paths = 0, allow_special = 0, allow_link = 0;
    uint32 fallback_used = 0, result = 0, scene, x, y, z, value, index, repeated;
    sint32 kind, frame, probe_frame, retries = 2, previous_retries, gap;
    sint32 probe;
    p2_guest_stack_pointer -= 112;
    local = p2_guest_stack_pointer;
    frame_offset = local;
    query_floor = local + 16;
    input = local + 32;
    inverse = local + 48;
    transpose = local + 64;
    visited = local + 88;
    kind = r_s8(object_offset + 126);
    if (kind == 1)
        kind_one_probe = (character_controller.flags & 0x60000) != 0;
    else if (r_s8(object_offset + 124) == 2 && (r_u32(object_offset + 116) == 19 || r_u32(0x8008A498) != 0))
    {
        parent = r_u32(p2_func_800536f0(0) + 132);
        value = r_u32(parent + 16) & 0x80000;
        restrict_paths = 1;
        if (value != 0)
            allow_special = 1;
        if ((r_u32(r_u32(object_offset + 132) + 16) & 0x10000) != 0)
        {
            allow_link = 1;
        }
    }
    direction = r_u32(object_offset + 28) & 0x10;
    if ((r_u32(object_offset + 228) & 0x26) != 0)
    {
        probe = CanChangePath(object_offset, position_offset, direction);
        if (((uint32)probe & 0xFF) == 0)
            goto done;
    }
    for (index = 0; index < 6; ++index)
        w_u32(visited + index * 4, 0);
select_path:
    frame = -1;
    if ((r_u32(object_offset + 28) & 0x40) != 0)
        frame = r_s16(object_offset + 136);
    w_u32(frame_offset, (uint32)frame);
    kind = r_s8(object_offset + 126);
    if (kind == 1 || (kind == 3 && r_s8(object_offset + 120) == -1))
    {
        if ((r_u32(r_u32(path_offset + 36) + 16) & 0x50) >= 0x41 && ((r_u32(object_offset + 28) & 0x4800) == 0 || retries < 2))
        {
            value = r_u32(path_offset + 28);
            if (value != 0)
            {
                candidate = value;
                if ((r_u16(path_offset + 14) & (direction | 8)) == 0 && (r_u16(value + 14) & 8) != 0)
                    candidate = r_u32(value + 16);
                if (candidate != 0)
                    goto check_candidate;
            }
        }
        candidate = SelectAdjacentPath(path_offset, frame_offset, direction, r_u32(object_offset + 228));
        if (candidate != 0)
            goto check_candidate;
    }
    if (candidate == 0)
        goto fallback;
    if (r_s8(object_offset + 125) != 6 || r_u32(object_offset + 116) != 0 || r_u32(0x8008A498) == 0 || fallback_used != 0)
        goto done;
    if ((r_u32(r_u32(object_offset + 132) + 16) & 0x10000) == 0)
        goto done;
    value = r_u32(r_u32(r_u32(0x8008A114) + 136) + 20);
    fallback_used = 1;
    if (value == 0)
        goto done;
    candidate = value;
    goto check_candidate;
fallback:
    if (fallback_used == 1)
        goto done;
    fallback_used = 1;
    candidate = p2_func_8003bc08(object_offset, position_offset, floor_offset, direction);
    if (candidate == 0)
        goto done;
check_candidate:
    flags = r_u32(object_offset + 228);
    w_u32(object_offset + 228, flags & 0xFDFFFFFF);
    if ((flags & 0x20) != 0 && r_u32(r_u32(0x8008A114) + 136) == candidate)
        goto fallback;
    if (candidate != initial_path && candidate != 0)
    {
        repeated = 0;
        for (index = 0; index < 6; ++index)
        {
            visited_path = r_u32(visited + index * 4);
            if (candidate == visited_path)
            {
                repeated = 1;
                break;
            }
        }
        if (repeated != 0 && r_s8(0x8008A170) != 0)
            goto fallback;
        for (index = 0; index < 6; ++index)
        {
            visited_path = r_u32(visited + index * 4);
            if (visited_path == 0)
            {
                w_u32(visited + index * 4, candidate);
                break;
            }
        }
    }
    flags = r_u32(r_u32(path_offset + 36) + 16);
    if ((flags & 0x50) == 0 && retries >= 2)
        goto accept_path;
    if ((r_u32(object_offset + 28) & 0x6000) == 0x4000)
    {
        probe = ProjectPathFrame(r_u32(candidate + 36), r_s32(frame_offset), position_offset, query_floor, direction, 4096);
        w_u32(frame_offset, (uint32)probe);
        if (probe < 0)
            goto done;
        goto check_floor;
    }
    if (((flags & 0x40) == 0 || candidate == path_offset) && retries == 2)
    {
        probe = SamplePathPosition(candidate, r_s32(frame_offset), query_floor, direction);
        goto floor_ready;
    }
    if ((r_u32(object_offset + 28) & 0x2000) == 0 || (r_u32(object_offset + 228) & 0x40000) != 0)
    {
        probe = FindPathHeightFrame(r_u32(candidate + 36), position_offset, direction, -40960);
        w_u32(frame_offset, (uint32)probe);
    }
    if ((r_u32(object_offset + 228) & 0x24) != 0 && r_s32(frame_offset) == -1)
    {
        probe = FindPathHeightFrame(r_u32(candidate + 36), position_offset, direction, -40960);
        if (probe != -1)
        {
            probe = ProjectPathFrame(r_u32(candidate + 36), probe, position_offset, query_floor, direction, 8192);
            w_u32(frame_offset, (uint32)probe);
        }
    }
    if (kind_one_probe != 0 && r_s32(frame_offset) == -1)
    {
        probe = FindPathHeightFrame(r_u32(candidate + 36), position_offset, direction, -40960);
        if (probe != -1)
        {
            probe = ProjectPathFrame(r_u32(candidate + 36), probe, position_offset, query_floor, direction, 20480);
            w_u32(frame_offset, (uint32)probe);
        }
    }
    frame = r_s32(frame_offset);
    if (frame < 0)
        probe_frame = -1;
    else
    {
        probe = ProjectPathFrame(r_u32(candidate + 36), frame, position_offset, query_floor, direction, 4096);
        probe_frame = probe;
        if ((r_u32(object_offset + 228) & 0x24) != 0 && probe_frame == -1)
        {
            probe = FindPathHeightFrame(r_u32(candidate + 36), position_offset, direction, -40960);
            if (probe != -1)
            {
                probe = ProjectPathFrame(r_u32(candidate + 36), probe, position_offset, query_floor, direction, 8192);
                probe_frame = probe;
            }
        }
        if (probe_frame >= 0 && (r_u32(query_floor) - r_u32(position_offset) + 24576 > 0xC000 || r_u32(query_floor + 8) - r_u32(position_offset + 8) + 24576 > 0xC000))
            probe_frame = -1;
    }
    w_u32(frame_offset, (uint32)probe_frame);
floor_ready:
    if (r_s32(frame_offset) < 0)
        goto retry_path;
check_floor:
    if ((r_u32(r_u32(candidate + 36) + 16) & 0x10000) != 0)
        goto retry_path;
    gap = (sint32)(r_u32(position_offset + 4) - r_u32(query_floor + 4));
    if (gap < -1365 && ((r_u32(r_u32(path_offset + 36) + 16) & 0x40) == 0 || gap < -4096))
        goto retry_path;
    x = r_u32(query_floor);
    y = r_u32(query_floor + 4);
    z = r_u32(query_floor + 8);
    w_u32(floor_offset, x);
    w_u32(floor_offset + 4, y);
    w_u32(floor_offset + 8, z);
    goto accept_path;
retry_path:
    if (retries != 0)
    {
        flags = r_u32(object_offset + 28);
        if ((flags & 0x4000) == 0)
        {
            previous_retries = retries--;
            if (previous_retries != 2 || (r_u32(r_u32(path_offset + 36) + 16) & 0x10) == 0 || (flags & 0x800) == 0 || r_u32(path_offset + 28) == 0)
                path_offset = candidate;
            goto select_path;
        }
    }
    if (fallback_used == 0)
        goto fallback;
    goto done;
accept_path:
    if (candidate == initial_path && ((r_u32(object_offset + 28) & 0x800) == 0 || retries != 2))
        goto done;
    flags = r_u32(r_u32(candidate + 36) + 16);
    if ((flags & 0x10000) != 0)
        goto done;
    if (restrict_paths != 0)
    {
        if ((flags & 0x80000) != 0 && allow_special == 0)
            goto done;
        if (allow_link == 0 && (r_u32(r_u32(candidate + 36) + 16) & 0x10) != 0)
            goto done;
    }
    flags = r_u32(object_offset + 28);
    kind = r_s8(object_offset + 126);
    w_u32(object_offset + 28, flags & 0xFFFF9FFF);
    if (kind == 1 && r_s8(object_offset + 125) != 33)
    {
        scene = r_u32(0x8008A114);
        x = r_u32(position_offset);
        y = r_u32(position_offset + 4);
        z = r_u32(position_offset + 8);
        w_u32(scene + 16, x);
        w_u32(scene + 20, y);
        w_u32(scene + 24, z);
        p2_func_80046510(object_offset, candidate, r_s32(frame_offset));
    }
    else
    {
        if (kind != 1)
            ApplyPathVelocityChange(object_offset, r_u32(candidate + 36));
        w_u32(object_offset + 112, candidate);
    }
    w_u16(object_offset + 136, r_u32(frame_offset));
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
        p2_sdk_8007ec64(input, object_offset + 16);
        PopMatrix();
        parent = r_u32(object_offset + 132);
        matrix = r_u32(parent + 40);
        x = r_u32(object_offset + 16);
        value = r_u32(matrix + 20);
        parent = r_u32(object_offset + 132);
        w_u32(object_offset + 16, x + value);
        matrix = r_u32(parent + 40);
        y = r_u32(object_offset + 20);
        value = r_u32(matrix + 24);
        parent = r_u32(object_offset + 132);
        w_u32(object_offset + 20, y + value);
        matrix = r_u32(parent + 40);
        z = r_u32(object_offset + 24);
        value = r_u32(matrix + 28);
        w_u32(object_offset + 24, z + value);
    }
    else
    {
        x = r_u32(object_offset + 16);
        y = r_u32(object_offset + 20);
        z = r_u32(object_offset + 24);
        w_u32(object_offset + 16, x);
        w_u32(object_offset + 20, y);
        w_u32(object_offset + 24, z);
    }
    definition = r_u32(candidate + 36);
    w_u32(object_offset + 132, definition);
    if (definition != 0 && (matrix = r_u32(definition + 40)) != 0)
    {
        x = r_u32(object_offset + 16);
        w_u32(inverse, x - r_u32(matrix + 20));
        matrix = r_u32(r_u32(object_offset + 132) + 40);
        y = r_u32(object_offset + 20);
        w_u32(inverse + 4, y - r_u32(matrix + 24));
        matrix = r_u32(r_u32(object_offset + 132) + 40);
        z = r_u32(object_offset + 24);
        w_u32(inverse + 8, z - r_u32(matrix + 28));
        for (index = 0; index < 9; ++index)
        {
            matrix = r_u32(r_u32(object_offset + 132) + 40);
            value = r_u16(matrix + (index % 3) * 6 + (index / 3) * 2);
            w_u16(transpose + index * 2, value);
        }
        PushMatrix();
        x = r_u32(transpose);
        y = r_u32(transpose + 4);
        xport_gte_write_control(0, x);
        xport_gte_write_control(1, y);
        x = r_u32(transpose + 8);
        y = r_u32(transpose + 12);
        z = r_u32(transpose + 16);
        xport_gte_write_control(2, x);
        xport_gte_write_control(3, y);
        xport_gte_write_control(4, z);
        p2_sdk_8007ec64(inverse, object_offset + 16);
        PopMatrix();
    }
    else
    {
        x = r_u32(object_offset + 16);
        y = r_u32(object_offset + 20);
        z = r_u32(object_offset + 24);
        w_u32(object_offset + 16, x);
        w_u32(object_offset + 20, y);
        w_u32(object_offset + 24, z);
    }
    result = 1;
done:
    p2_guest_stack_pointer = saved_stack;
    return result;
}

uint32 SelectScenePath(uint32 path_offset)
{
    FUNCTION_MARKER(0x80024844, "SLUS_005.78");
    uint32 context = r_u32(0x8008A114);

    if (r_s16(context + 304) != 4096)
        p2_func_8002417c(0xE9555);
    context = r_u32(0x8008A114);
    w_u32(context + 136, path_offset);
    return context;
}

uint32 ProjectPathOffset(uint32 path_offset, sint32 frame, uint32 destination_offset, uint32 direction)
{
    FUNCTION_MARKER(0x800451CC, "SLUS_005.78");
    uint32 saved_stack = p2_guest_stack_pointer;
    uint32 point, previous, following, x, y, z, numerator, denominator, ratio, scale, result, i;
    sint32 selected, count, gap, reciprocal, lift;
    p2_guest_stack_pointer -= 0x48;
    point = p2_guest_stack_pointer + 0x18;
    previous = p2_guest_stack_pointer + 0x28;
    following = p2_guest_stack_pointer + 0x38;
    selected = ProjectPathFrame(r_u32(path_offset + 36), frame, r_u32(0x8008A114) + 16, point, direction, 4096);
    if (selected < 0)
        selected = frame;
    gap = (sint32)(r_u32(r_u32(0x8008A114) + 20) - r_u32(point + 4));
    if (selected <= 0)
    {
        SamplePathPosition(path_offset, 0, point, direction);

        SamplePathOffset(path_offset, 0, destination_offset, direction);

    }
    else
    {
        count = r_s16(r_u32(path_offset + 36) + 26);
        if (selected < count)
        {
            SamplePathPosition(path_offset, (sint32)((uint32)selected - 1), previous, direction);

            x = r_u32(point) - r_u32(previous);
            y = r_u32(point + 4) - r_u32(previous + 4);
            z = r_u32(point + 8) - r_u32(previous + 8);
            w_u32(point, x);
            w_u32(point + 4, y);
            w_u32(point + 8, z);
            x = (uint32)((sint32)x >> 6);
            y = (uint32)((sint32)y >> 6);
            z = (uint32)((sint32)z >> 6);

            numerator = x * x + y * y + z * z;
            SamplePathOffset(path_offset, (sint32)((uint32)selected - 1), destination_offset, direction);

            SamplePathPosition(path_offset, selected, following, direction);

            x = r_u32(following) - r_u32(previous);
            y = r_u32(following + 4) - r_u32(previous + 4);
            z = r_u32(following + 8) - r_u32(previous + 8);
            w_u32(following, x);
            w_u32(following + 4, y);
            w_u32(following + 8, z);
            x = (uint32)((sint32)x >> 6);
            y = (uint32)((sint32)y >> 6);
            z = (uint32)((sint32)z >> 6);
            denominator = x * x + y * y + z * z;
            if (denominator == 0)
            {
                SamplePathOffset(path_offset, selected, destination_offset, direction);

            }
            else
            {
                reciprocal = 0x40000000 / (sint32)denominator;
                ratio = (uint32)(((sint64)(sint32)numerator * reciprocal) >> 18);
                w_u32(0x1F80030C, ratio);
                SamplePathOffset(path_offset, selected, point, direction);

                for (i = 0; i < 3; ++i)
                    w_u32(point + i * 4, r_u32(point + i * 4) - r_u32(destination_offset + i * 4));
                ratio = r_u32(0x1F80030C);
                if ((sint32)ratio >= 0)
                {
                    scale = p2_sdk_8007e794(ratio);
                }
                else
                {
                    scale = p2_sdk_8007e794(ratio >> 2) << 1;
                }

                for (i = 0; i < 3; ++i)
                    w_u32(point + i * 4, (uint32)(((sint64)r_s32(point + i * 4) * (sint32)scale) >> 12));
                for (i = 0; i < 3; ++i)
                    w_u32(destination_offset + i * 4, r_u32(destination_offset + i * 4) + r_u32(point + i * 4));
            }
        }
        else
        {
            SamplePathPosition(path_offset, count - 1, point, direction);

            SamplePathOffset(path_offset, r_s16(r_u32(path_offset + 36) + 26) - 1, destination_offset, direction);

        }
    }
    result = r_u16(path_offset + 14) & 1;
    if (result != 0)
    {
        result = 5120;
        if (gap > 0)
        {
            lift = (sint32)(uint32)(((sint64)gap * 5120) >> 12);
            if ((sint32)((uint32)lift - (uint32)gap) > 61440)
                lift = (sint32)((uint32)gap + 61440);
            result = r_u32(destination_offset + 4) + (uint32)lift;
            w_u32(destination_offset + 4, result);
        }
    }
    p2_guest_stack_pointer = saved_stack;
    return result;
}

uint32 ReverseScenePath(uint32 object_offset, uint32 character_offset)
{
    FUNCTION_MARKER(0x80046CBC, "SLUS_005.78");
    uint32 state, path, result, flags, x, y, z, parent, matrix, frame, old_frame, heading;
    uint32 saved_stack, local, delta, direction, previous, definition, value;
    sint32 signed_frame;

    state = r_u32(0x8008A114);
    path = r_u32(state + 136);
    result = r_u16(path + 14) & 8;
    if (result != 0)
        path = r_u32(path + 16);
    if (path == 0)
    {
        return result;
    }
    result = character_controller.secondary_flags & 0x80;
    if (result != 0)
    {
        return result;
    }
    w_u32(object_offset + 28, r_u32(object_offset + 28) ^ 0x10);
    x = r_u32(character_offset + 84);
    y = character_controller.motion.horizontal_speed;
    w_u32(character_offset + 84, 0 - x);
    character_controller.motion.horizontal_speed = 0 - y;
    heading = r_u16(object_offset + 150);
    parent = r_u32(object_offset + 132);
    old_frame = r_u16(object_offset + 136);
    w_u16(object_offset + 150, (heading + 2048) & 0xFFF);
    frame = r_u16(parent + 26);
    flags = r_u32(object_offset + 28);
    frame -= old_frame;
    saved_stack = p2_guest_stack_pointer;
    p2_guest_stack_pointer -= 32;
    local = p2_guest_stack_pointer;
    delta = local + 16;
    if ((flags & 0x2000) != 0)
    {
        x = (uint32)character_controller.path_tangent.vx;
        y = (uint32)character_controller.path_tangent.vy;
        z = (uint32)character_controller.path_tangent.vz;
        character_controller.path_tangent.vx =  0 - x;
        character_controller.path_tangent.vy =  0 - y;
        character_controller.path_tangent.vz =  0 - z;
        x = r_u32(object_offset + 16);
        y = r_u32(object_offset + 20);
        z = r_u32(object_offset + 24);
        w_u32(state + 16, x);
        w_u32(state + 20, y);
        w_u32(state + 24, z);
    }
    else if (parent != 0 && r_u32(parent + 40) != 0)
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
        state = r_u32(0x8008A114);
        p2_sdk_8007ec64(local, state + 16);
        PopMatrix();

        state = r_u32(0x8008A114);
        matrix = r_u32(r_u32(object_offset + 132) + 40);
        x = r_u32(state + 16);
        x += r_u32(matrix + 20);
        w_u32(state + 16, x);
        matrix = r_u32(r_u32(object_offset + 132) + 40);
        y = r_u32(state + 20);
        y += r_u32(matrix + 24);
        w_u32(state + 20, y);
        matrix = r_u32(r_u32(object_offset + 132) + 40);
        z = r_u32(state + 24);
        z += r_u32(matrix + 28);
        w_u32(state + 24, z);
    }
    else
    {
        state = r_u32(0x8008A114);
        x = r_u32(object_offset + 16);
        y = r_u32(object_offset + 20);
        z = r_u32(object_offset + 24);
        w_u32(state + 16, x);
        w_u32(state + 20, y);
        w_u32(state + 24, z);
    }
    state = r_u32(0x8008A114);
    previous = r_u32(state + 136);
    signed_frame = (sint16)frame;
    if (path != previous)
    {
        direction = r_u32(object_offset + 28) & 0x10;
        ProjectPathOffset(path, signed_frame, state + 28, direction);
        state = r_u32(0x8008A114);
        previous = r_u32(state + 136);
        definition = r_u32(path + 36);
        if (definition == r_u32(previous + 36))
            direction ^= 0x10;
        else
            ApplyPathVelocityChange(object_offset, definition);
        state = r_u32(0x8008A114);
        if ((r_u32(state + 72) & 0xFFFFFF) == 0)
        {
            previous = r_u32(state + 136);
            value = r_u32(0x800AC018);
            if (previous == value && (r_u32(object_offset + 28) & 0x2000) == 0)
            {
                signed_frame = r_s16(object_offset + 136);
                if (signed_frame >= 0)
                    ProjectPathOffset(previous, signed_frame, state + 4, direction);
            }
        }
        SelectScenePath(path);
    }
    w_u16(object_offset + 136, frame);
    if (path != r_u32(character_offset + 688))
    {
        w_u32(character_offset + 688, path);
        state = r_u32(0x8008A114);
        x = r_u32(state + 28);
        x -= r_u32(state + 4);
        w_u32(delta, x);
        y = r_u32(state + 32);
        y -= r_u32(state + 8);
        w_u32(delta + 4, y);
        z = r_u32(state + 36);
        z -= r_u32(state + 12);
        w_u32(delta + 8, z);
        p2_func_80024b3c(delta);
    }
    result = r_u32(0x8008A114);
    w_u16(result + 2, frame);
    p2_guest_stack_pointer = saved_stack;
    return result;
}

void SamplePathTangent(uint32 source_offset, VECTOR *destination, sint32 position)
{
    FUNCTION_MARKER(0x80022EC4, "SLUS_005.78");
    VECTOR vector;
    uint32 node, index, value, i;
    sint32 x, y, z;
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
        if (i == 0)
            vector.vx = value;
        else if (i == 1)
            vector.vy = value;
        else
            vector.vz = value;
    }
    while ((uint32)vector.vx + 26754 > 0xD103 || (uint32)vector.vy + 26754 > 0xD103 || (uint32)vector.vz + 26754 > 0xD103)
    {
        x = vector.vx >> 1;
        y = vector.vy >> 1;
        z = vector.vz >> 1;
        vector.vx = x;
        vector.vy = y;
        vector.vz = z;
    }
    VectorNormal(&vector, &vector);
    if (r_u32(source_offset + 40) != 0)
    {
        PushMatrix();
        node = r_u32(source_offset + 40);
        for (i = 0; i < 5; ++i)
            xport_gte_write_control(i, r_u32(node + i * 4));
        ApplyRotMatrixLV(&vector, destination);
        PopMatrix();
    }
    else
    {
        destination->vx = vector.vx;
        destination->vy = vector.vy;
        destination->vz = vector.vz;
    }
}
