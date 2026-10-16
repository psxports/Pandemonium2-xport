#include "game.h"
#include <stdlib.h>

uint32 p2_func_80042320(uint32 definition_offset, uint32 object_offset)
{
    uint32 saved_stack = p2_guest_stack_pointer;
    uint32 first, delta, owner, direction, sample_frame, x, y, z, length, value, result;
    sint32 frame, heading, half_angle, complement;
    p2_guest_stack_pointer -= 32;
    first = p2_guest_stack_pointer;
    delta = first + 16;
    frame = r_s16(object_offset + 136);
    direction = r_u32(object_offset + 28) & 0x10;
    owner = r_u32(object_offset + 8);
    if (frame > 0)
        --frame;
    sample_frame = (uint32)frame;
    p2_func_800230f0(r_u32(object_offset + 132), (sint16)sample_frame, first, direction);
    do
    {
        ++sample_frame;
        frame = (sint16)sample_frame;
        p2_func_800230f0(r_u32(object_offset + 132), frame, delta, direction);
        x = r_u32(first) - r_u32(delta);
        y = r_u32(first + 4) - r_u32(delta + 4);
        z = r_u32(first + 8) - r_u32(delta + 8);
        w_u32(delta, x);
        w_u32(delta + 4, y);
        w_u32(delta + 8, z);
        heading = (sint16)p2_func_8005dc00((sint32)z, (sint32)x);
        w_u16(object_offset + 150, heading);
        if (heading != 4096)
            break;
    } while (frame < r_s16(r_u32(object_offset + 132) + 26) - 1);
    if (direction == 0)
    {
        if (heading == 4096)
            goto update_pitch;
        w_u16(object_offset + 150, ((uint32)heading + 2048) & 0xFFF);
    }
    heading = r_s16(object_offset + 150);
    if (heading != 4096 && r_s16(object_offset + 148) != heading)
    {
        half_angle = (sint32)((2048 - (uint32)heading) << 16) >> 17;
        w_u16(object_offset + 148, heading);
        result = (uint32)p2_func_8005dbb8((sint16)(1024 - (uint32)half_angle));
        w_u16(object_offset + 84, result);
        w_u16(object_offset + 90, 0);
        w_u16(object_offset + 86, 0);
        result = (uint32)p2_func_8005dbb8((sint16)half_angle);
        w_u16(object_offset + 88, result);
    }
update_pitch:
    if ((r_u32(object_offset + 28) & 0x400) == 0)
    {
        if (r_u32(delta + 4) + 32 < 65)
            w_u16(owner + 154, 0);
        else
        {
            x = (uint32)(r_s32(delta) >> 6);
            y = (uint32)(r_s32(delta + 4) >> 6);
            z = (uint32)(r_s32(delta + 8) >> 6);
            value = x * x + y * y + z * z;
            if ((sint32)value < 0)
                length = p2_sdk_8007e794(value >> 2) << 1;
            else
                length = p2_sdk_8007e794(value);
            result = 0 - (uint32)p2_func_8005dcd4(r_s32(delta + 4), (sint32)length);
            w_u16(owner + 154, result & 0xFFF);
        }
    }
    result = r_u16(definition_offset) & 2;
    if (result != 0)
    {
        value = r_u16(owner + 154);
        if ((r_u32(object_offset + 28) & 0x10) == 0)
            value = 0 - value;
        half_angle = (sint32)(value << 16) >> 17;
        complement = (sint32)(1024 - (uint32)half_angle);
        result = (uint32)p2_func_8005dbb8((sint16)complement);
        w_u16(owner + 84, result);
        result = (uint32)p2_func_8005dbb8((sint16)half_angle);
        w_u16(owner + 86, result);
        w_u16(owner + 90, 0);
        w_u16(owner + 88, 0);
        owner = r_u32(owner + 4);
        if (owner != 0)
        {
            result = (uint32)p2_func_8005dbb8((sint16)complement);
            w_u16(owner + 84, result);
            result = (uint32)p2_func_8005dbb8((sint16)half_angle);
            w_u16(owner + 86, result);
            w_u16(owner + 90, 0);
            w_u16(owner + 88, 0);
        }
    }
    p2_guest_stack_pointer = saved_stack;
    return result;
}

uint32 p2_func_80041edc(uint32 object_offset, uint32 distance)
{
    uint32 node = r_u32(object_offset + 8);
    uint32 changed = 0, factor, result;
    sint32 denominator, quotient, angle;

    while (node != 0)
    {
        if (r_u8(node + 12) == 6 && r_u8(object_offset + 12) == 5)
        {
            denominator = (sint32)(uint32)(((sint64)r_s32(node + 32) * 25735) >> 12);
            if (denominator == 0)
            {
                trap(7);
                abort();
            }
            quotient = 0x40000000 / denominator;
            factor = (uint32)(((sint64)(sint32)distance * quotient) >> 18);
            angle = (sint32)factor >> 1;
            w_u32(0x1F80030C, factor);
            w_u8(object_offset + 29, 0xFF);
            result = (uint32)p2_func_8005dbb8((sint16)(1024 - (uint32)angle));
            w_u16(object_offset + 40, result);
            result = (uint32)p2_func_8005dbb8((sint16)angle);
            changed = 1;
            w_u16(object_offset + 42, result);
            w_u16(object_offset + 46, 0);
            w_u16(object_offset + 44, 0);
        }
        result = p2_func_80041edc(node, distance);
        node = r_u32(node + 4);
        changed |= result;
    }
    return (uint32)(sint32)(sint8)changed;
}

static void p2_callback_transpose(uint32 object_offset, uint32 rotation)
{
    uint32 index, matrix;
    for (index = 0; index < 9; ++index)
    {
        matrix = r_u32(r_u32(object_offset + 132) + 40);
        w_u16(rotation + index * 2, r_u16(matrix + (index % 3) * 6 + (index / 3) * 2));
    }
}

static void p2_callback_rotate(uint32 rotation, uint32 input, uint32 output)
{
    uint32 index;
    PushMatrix();
    for (index = 0; index < 5; ++index)
        xport_gte_write_control(index, r_u32(rotation + index * 4));
    p2_sdk_8007ec64(input, output);
    PopMatrix();
}

static void p2_callback_world_position(uint32 object_offset, uint32 output, uint32 input)
{
    uint32 parent = r_u32(object_offset + 132);
    uint32 matrix, index, x, y, z;
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
        for (index = 0; index < 5; ++index)
            xport_gte_write_control(index, r_u32(matrix + index * 4));
        p2_sdk_8007ec64(input, output);
        PopMatrix();
        for (index = 0; index < 3; ++index)
        {
            matrix = r_u32(r_u32(object_offset + 132) + 40);
            w_u32(output + index * 4, r_u32(output + index * 4) + r_u32(matrix + 20 + index * 4));
        }
    }
    else
    {
        x = r_u32(object_offset + 16);
        y = r_u32(object_offset + 20);
        z = r_u32(object_offset + 24);
        w_u32(output, x);
        w_u32(output + 4, y);
        w_u32(output + 8, z);
    }
}

static void p2_callback_local_position(uint32 object_offset, uint32 source, uint32 input, uint32 rotation)
{
    uint32 parent = r_u32(object_offset + 132);
    uint32 index, matrix, x, y, z;
    if (parent != 0 && r_u32(parent + 40) != 0)
    {
        for (index = 0; index < 3; ++index)
        {
            matrix = r_u32(r_u32(object_offset + 132) + 40);
            w_u32(input + index * 4, r_u32(source + index * 4) - r_u32(matrix + 20 + index * 4));
        }
        p2_callback_transpose(object_offset, rotation);
        p2_callback_rotate(rotation, input, object_offset + 16);
    }
    else
    {
        x = r_u32(source);
        y = r_u32(source + 4);
        z = r_u32(source + 8);
        w_u32(object_offset + 16, x);
        w_u32(object_offset + 20, y);
        w_u32(object_offset + 24, z);
    }
}

/* Keep the wrapped squared sum used by the callback's repeated length queries */
static uint32 p2_callback_vector_length(uint32 vector)
{
    uint32 x = (uint32)(r_s32(vector) >> 6);
    uint32 y = (uint32)(r_s32(vector + 4) >> 6);
    uint32 z = (uint32)(r_s32(vector + 8) >> 6);
    uint32 sum = x * x + y * y + z * z;
    if ((sint32)sum < 0)
        return p2_sdk_8007e794(sum >> 2) << 1;
    return p2_sdk_8007e794(sum);
}

/* The three original reversal blocks reload the owning path before mirroring */
static void p2_reverse_callback_path(uint32 object_offset)
{
    uint32 data = r_u32(r_u32(object_offset + 132) + 44);
    uint32 flags, parent;
    sint32 count, frame;
    w_u32(data + 12, 0 - r_u32(data + 12));
    flags = r_u32(object_offset + 28);
    parent = r_u32(object_offset + 132);
    w_u32(object_offset + 28, flags ^ 0x10);
    count = r_s16(parent + 26);
    frame = count - r_s16(object_offset + 136);
    if (frame >= count)
        frame = count - 1;
    else if (frame < 0)
        frame = 0;
    w_u16(object_offset + 136, frame);
}

uint32 p2_func_800414d0(uint32 object_offset)
{
    uint32 saved_stack = p2_guest_stack_pointer;
    uint32 data = r_u32(r_u32(object_offset + 132) + 44);
    uint32 vector, x, y, z, sum, result;
    p2_guest_stack_pointer -= 16;
    vector = p2_guest_stack_pointer;
    x = r_u32(object_offset + 92);
    y = r_u32(object_offset + 96);
    z = r_u32(object_offset + 100);
    w_u32(vector, x);
    w_u32(vector + 4, y);
    w_u32(vector + 8, z);
    w_u32(data + 16, y);
    x = (uint32)(r_s32(vector) >> 6);
    z = (uint32)(r_s32(vector + 8) >> 6);
    sum = x * x + z * z;
    w_u32(vector + 4, 0);
    if ((sint32)sum < 0)
        result = p2_sdk_8007e794(sum >> 2) << 1;
    else
        result = p2_sdk_8007e794(sum);
    w_u32(data + 12, result);
    if ((r_u32(object_offset + 28) & 0x2000) != 0)
    {
        x = r_u32(vector);
        y = r_u32(vector + 4);
        z = r_u32(vector + 8);
        while (x + 26754 > 0xD103 || y + 26754 > 0xD103 || z + 26754 > 0xD103)
        {
            x = (uint32)((sint32)x >> 1);
            y = (uint32)((sint32)y >> 1);
            z = (uint32)((sint32)z >> 1);
            w_u32(vector, x);
            w_u32(vector + 4, y);
            w_u32(vector + 8, z);
        }
        VectorNormal((VECTOR *)psx_addr(vector, 12), (VECTOR *)psx_addr(vector, 12));
        x = r_u32(vector);
        y = r_u32(vector + 4);
        z = r_u32(vector + 8);
        w_u32(data + 24, x);
        w_u32(data + 28, y);
        w_u32(data + 32, z);
    }
    result = r_u32(object_offset + 28) | 0x400;
    w_u32(object_offset + 28, result);
    p2_guest_stack_pointer = saved_stack;
    return result;
}

uint32 p2_func_8004165c(uint32 object_offset)
{
    uint32 speed = r_u32(r_u32(object_offset + 132) + 44) + 12;
    uint32 count = r_u32(0x8008A338), cursor = 0x8008B4A8, definition = 0;
    uint32 kind = r_u32(object_offset + 116), owner, flags, angle, reverse, value;
    uint32 index, x, y, z, sum, length, scaled_length, relative_y, gravity, factor, result;
    sint32 heading, quotient;
    while (count != 0)
    {
        definition = r_u32(cursor);
        cursor += 4;
        if ((uint32)(sint32)r_s8(definition + 2) == kind)
            break;
        --count;
    }
    if (count == 0)
        definition = 0;
    flags = r_u32(object_offset + 28);
    owner = r_u32(object_offset + 8);
    if ((flags & 0x400000) != 0)
    {
        angle = r_u16(object_offset + 150);
        if ((flags & 0x10) != 0)
            angle += 2048;
        reverse = 0;
        heading = (sint16)p2_func_8005dc00(r_s32(speed + 20), r_s32(speed + 12));
        if (heading != 4096)
            reverse = (((uint32)(sint32)(sint16)angle - (uint32)heading + 512) & 0xFFF) >= 1025;
        for (index = 0; index < 3; ++index)
        {
            value = (uint32)(((sint64)r_s32(speed + 12 + index * 4) * r_s32(definition + 20)) >> 12);
            w_u32(speed + 12 + index * 4, value);
        }
        x = r_u32(speed + 12);
        y = r_u32(speed + 16);
        z = r_u32(speed + 20);
        w_u32(object_offset + 92, x);
        w_u32(object_offset + 96, y);
        w_u32(object_offset + 100, z);
        if ((r_u32(object_offset + 28) & 0x400) != 0)
        {
            p2_func_800414d0(object_offset);
            w_u32(speed + 4, r_u32(speed + 4) + r_u32(definition + 12));
        }
        else
        {
            w_u32(speed, p2_callback_vector_length(object_offset + 92));
            w_u32(speed + 4, 0);
        }
        if (reverse != 0)
        {
            w_u32(speed, 0 - r_u32(speed));
            x = r_u32(speed + 12);
            y = r_u32(speed + 16);
            z = r_u32(speed + 20);
            w_u32(speed + 12, 0 - x);
            w_u32(speed + 16, 0 - y);
            w_u32(speed + 20, 0 - z);
            p2_reverse_callback_path(object_offset);
            w_u16(owner + 154, 0 - r_u16(owner + 154));
        }
        if (r_s32(speed) < 256)
            w_u32(speed, 0);
        w_u32(object_offset + 28, (r_u32(object_offset + 28) & 0xFFBFFFFF) | 0x1000000);
    }
    flags = r_u32(object_offset + 28);
    if ((flags & 0x400) != 0)
        w_u32(speed + 4, r_u32(speed + 4) - r_u32(definition + 12));
    else
    {
        angle = r_u16(owner + 154);
        if ((flags & 0x4000) != 0)
        {
            w_u32(object_offset + 28, flags & 0xFFFFBFFF);
            length = p2_callback_vector_length(speed);
            scaled_length = (uint32)(((sint64)(sint32)length * r_s32(definition + 20)) >> 12);
            if (scaled_length != 0)
            {
                heading = p2_func_8005dcd4(r_s32(speed + 4), (sint32)length);
                value = (angle - (uint32)heading) & 0xFFF;
                x = (uint32)(((sint64)(p2_func_8005dbb8((sint16)(1024 - value)) >> 2) * (sint32)scaled_length) >> 12);
                w_u32(object_offset + 92, x);
                y = (uint32)(((sint64)(p2_func_8005dbb8((sint16)value) >> 2) * (sint32)scaled_length) >> 12);
                x = r_u32(object_offset + 92);
                w_u32(object_offset + 96, y);
                w_u32(object_offset + 100, 0);
                w_u32(speed, x);
            }
            w_u32(speed + 4, 0);
        }
        if (r_s32(speed) < 0)
        {
            p2_reverse_callback_path(object_offset);
            angle = 0 - angle;
        }
        x = r_u32(object_offset + 92);
        y = r_u32(object_offset + 96);
        z = r_u32(object_offset + 100);
        relative_y = y - r_u32(definition + 12);
        reverse = 0;
        if ((r_u16(definition) & 4) != 0)
        {
            x = (uint32)(r_s32(object_offset + 92) >> 6);
            y = (uint32)(r_s32(object_offset + 96) >> 6);
            z = (uint32)(r_s32(object_offset + 100) >> 6);
            sum = x * x + y * y + z * z;
            if ((sint32)sum >= 89)
            {
                x = (uint32)((sint32)r_u32(object_offset + 92) >> 6);
                y = (uint32)((sint32)relative_y >> 6);
                z = (uint32)((sint32)r_u32(object_offset + 100) >> 6);
                sum = x * x + y * y + z * z;
                if ((sint32)sum < 0)
                    length = p2_sdk_8007e794(sum >> 2) << 1;
                else
                    length = p2_sdk_8007e794(sum);
                heading = (sint16)p2_func_8005dcd4((sint32)relative_y, (sint32)length);
                reverse = (((uint32)heading - (uint32)(sint32)(sint16)angle) & 0xFFF) < 2048;
            }
        }
        if (reverse != 0)
            p2_func_800414d0(object_offset);
        else if (r_s16(object_offset + 136) > 0 || (r_u32(object_offset + 28) & 0x1000000) == 0)
        {
            gravity = r_u32(definition + 12);
            value = (uint32)(((sint64)(sint32)gravity * (p2_func_8005dbb8((sint16)angle) >> 2)) >> 12);
            value = r_u32(speed) - value;
            w_u32(speed, value);
            w_u32(speed, (uint32)(((sint64)r_s32(definition + 16) * (sint32)value) >> 12));
        }
    }
    if (r_s32(speed) < 0)
        p2_reverse_callback_path(object_offset);
    length = p2_callback_vector_length(speed);
    value = r_u32(definition + 8);
    if ((sint32)value < (sint32)length)
    {
        if (length == 0)
        {
            trap(7);
            abort();
        }
        quotient = 0x40000000 / (sint32)length;
        factor = (uint32)(((sint64)(sint32)value * quotient) >> 18);
        w_u32(0x1F80030C, factor);
        for (index = 0; index < 3; ++index)
            w_u32(speed + index * 4, (uint32)(((sint64)r_s32(speed + index * 4) * (sint32)factor) >> 12));
    }
    w_u32(object_offset + 28, r_u32(object_offset + 28) | 0x40);
    p2_func_8003db7c(object_offset, r_s32(speed), r_s32(speed + 4), speed + 12);
    result = r_u32(object_offset + 28) & 0xFFFFFFBF;
    w_u32(object_offset + 28, result);
    return result;
}

uint32 p2_func_80040a1c(uint32 object_offset, uint32 other_offset, uint32 mode, uint32 value)
{
    uint32 saved_stack = p2_guest_stack_pointer;
    uint32 local, delta, normal, other_velocity, rotation, descriptor, surface, data;
    uint32 index, x, y, z, first, second, sum, magnitude, length, factor, result;
    sint32 quotient;
    p2_guest_stack_pointer -= 128;
    local = p2_guest_stack_pointer;
    delta = local;
    normal = local + 16;
    other_velocity = local + 32;
    rotation = local + 48;
    descriptor = local + 72;
    if (value == 0xFFFFFFFF)
    {
        x = r_u32(other_offset + 92);
        y = r_u32(other_offset + 96);
        z = r_u32(other_offset + 100);
        w_u32(other_velocity, x);
        w_u32(other_velocity + 4, y);
        w_u32(other_velocity + 8, z);
        for (index = 0; index < 3; ++index)
        {
            first = (uint32)(((sint64)r_s32(object_offset + 92 + index * 4) * (sint32)mode) >> 12);
            w_u32(delta + index * 4, first);
        }
        for (index = 0; index < 3; ++index)
            w_u32(delta + index * 4, r_u32(delta + index * 4) + r_u32(object_offset + 160 + index * 4));
        for (index = 0; index < 3; ++index)
        {
            first = (uint32)(((sint64)r_s32(other_velocity + index * 4) * (sint32)mode) >> 12);
            w_u32(normal + index * 4, first);
        }
        for (index = 0; index < 3; ++index)
            w_u32(normal + index * 4, r_u32(normal + index * 4) + r_u32(other_offset + 160 + index * 4));
        x = r_u32(delta) - r_u32(normal);
        y = r_u32(delta + 4) - r_u32(normal + 4);
        z = r_u32(delta + 8) - r_u32(normal + 8);
        w_u32(normal, x);
        w_u32(normal + 4, y);
        w_u32(normal + 8, z);
        while (x + 26754 > 0xD103 || y + 26754 > 0xD103 || z + 26754 > 0xD103)
        {
            x = (uint32)((sint32)x >> 1);
            y = (uint32)((sint32)y >> 1);
            z = (uint32)((sint32)z >> 1);
            w_u32(normal, x);
            w_u32(normal + 4, y);
            w_u32(normal + 8, z);
        }
        VectorNormal((VECTOR *)psx_addr(normal, 12), (VECTOR *)psx_addr(normal, 12));
    }
    else
    {
        surface = p2_func_8001c544(other_offset, value & 0xFFFF8FFF, descriptor);
        x = r_u32(descriptor + 36);
        y = r_u32(descriptor + 40);
        z = r_u32(descriptor + 44);
        w_u32(other_velocity, x);
        w_u32(other_velocity + 4, y);
        w_u32(other_velocity + 8, z);
        for (index = 0; index < 9; ++index)
            w_u16(rotation + index * 2, r_u16(descriptor + 4 + (index % 3) * 6 + (index / 3) * 2));
        PushMatrix();
        for (index = 0; index < 5; ++index)
            xport_gte_write_control(index, r_u32(rotation + index * 4));
        xport_gte_write_data(0, r_u32(surface + 16));
        xport_gte_write_data(1, r_u32(surface + 20));
        xport_gte_mvmva(0x486012);
        w_u32(normal, xport_gte_read_data(25));
        w_u32(normal + 4, xport_gte_read_data(26));
        w_u32(normal + 8, xport_gte_read_data(27));
        PopMatrix();
    }
    for (index = 0; index < 3; ++index)
        w_u32(delta + index * 4, r_u32(object_offset + 92 + index * 4) - r_u32(other_velocity + index * 4));
    if (value == 0xFFFFFFFF)
    {
        sum = 0;
        for (index = 0; index < 3; ++index)
            sum += (uint32)(((sint64)r_s32(delta + index * 4) * r_s32(normal + index * 4)) >> 12);
        magnitude = 0 - sum;
        if (magnitude != 0)
        {
            x = (uint32)(r_s32(delta) >> 6);
            y = (uint32)(r_s32(delta + 4) >> 6);
            z = (uint32)(r_s32(delta + 8) >> 6);
            sum = x * x + y * y + z * z;
            if ((sint32)sum < 0)
                length = p2_sdk_8007e794(sum >> 2) << 1;
            else
                length = p2_sdk_8007e794(sum);
            if (length != 0)
            {
                quotient = 0x40000000 / (sint32)length;
                factor = (uint32)(((sint64)(sint32)magnitude * quotient) >> 18);
                w_u32(0x1F80030C, factor);
                factor = 4096 - factor;
                for (index = 0; index < 3; ++index)
                {
                    first = (uint32)(((sint64)r_s32(delta + index * 4) * (sint32)factor) >> 12);
                    w_u32(delta + index * 4, first);
                }
            }
        }
    }
    sum = 0;
    for (index = 0; index < 3; ++index)
        sum += (uint32)(((sint64)r_s32(delta + index * 4) * r_s32(normal + index * 4)) >> 12);
    data = r_u32(r_u32(object_offset + 132) + 44);
    factor = 0 - (sum << 1);
    for (index = 0; index < 3; ++index)
    {
        first = (uint32)(((sint64)r_s32(normal + index * 4) * (sint32)factor) >> 12);
        w_u32(data + 24 + index * 4, first);
    }
    for (index = 0; index < 3; ++index)
    {
        first = r_u32(data + 24 + index * 4);
        second = r_u32(delta + index * 4);
        w_u32(data + 24 + index * 4, first + second);
    }
    for (index = 0; index < 3; ++index)
    {
        first = r_u32(data + 24 + index * 4);
        second = r_u32(other_velocity + index * 4);
        result = first + second;
        w_u32(data + 24 + index * 4, result);
    }
    p2_guest_stack_pointer = saved_stack;
    return result;
}

uint32 p2_func_80041060(uint32 object_offset, uint32 other_offset, uint32 contact_offset)
{
    uint32 count = r_u32(0x8008A338), cursor = 0x8008B4A8, entry = 0;
    uint32 kind_id = r_u32(object_offset + 116), flags, mask, mode, value, data;
    uint32 x, y, z, distance, speed, magnitude, scale, index;
    sint32 kind, quotient;
    while (count != 0)
    {
        entry = r_u32(cursor);
        cursor += 4;
        if ((uint32)(sint32)r_s8(entry + 2) == kind_id)
            break;
        --count;
    }
    if (count == 0)
        entry = 0;
    if (contact_offset == 0)
    {
        if (other_offset == 0)
            return (r_u16(entry) & 0x180) != 0;
        if (r_u8(other_offset + 12) != 0)
            return 0;
        kind = r_s8(other_offset + 126);
        switch (kind)
        {
            case 1:
                mask = 0x18;
                break;
            case 2:
                mask = 0x60;
                break;
            case 3:
                mask = 0x600;
                break;
            case 4:
                mask = 0x1800;
                break;
            default:
                return 0;
        }
        return (r_u16(entry) & mask) != 0;
    }
    flags = r_u32(object_offset + 28) & 0xFF3FFFFF;
    w_u32(object_offset + 28, flags);
    value = r_u32(contact_offset + 12);
    mode = r_u32(contact_offset + 4);
    if (value == 0x7FFFF000)
    {
        mask = (r_u16(entry) & 0x80) != 0 ? 0x400000 : 0x800000;
        w_u32(object_offset + 28, flags | mask);
        value = r_u32(contact_offset);
    }
    else
    {
        kind = r_s8(other_offset + 126);
        switch (kind)
        {
            case 1:
                mask = 8;
                break;
            case 2:
                mask = 0x20;
                break;
            case 3:
                mask = 0x200;
                break;
            case 4:
                mask = 0x800;
                break;
            default:
                return 0;
        }
        flags |= (r_u16(entry) & mask) != 0 ? 0x400000 : 0x800000;
        w_u32(object_offset + 28, flags);
        value = 0xFFFFFFFF;
    }
    flags = r_u32(object_offset + 28);
    if ((flags & 0x400000) == 0)
        return 1;
    if (mode != 0 || value != 0xFFFFFFFF)
    {
        p2_func_80040a1c(object_offset, other_offset, mode, value);
        if (mode != 0 || (r_u32(object_offset + 28) & 0x1000000) == 0)
            return 1;
        return 0;
    }
    if ((flags & 0x1000000) == 0)
        return 0;
    data = r_u32(r_u32(object_offset + 132) + 44);
    x = r_u32(object_offset + 160) - r_u32(other_offset + 160);
    w_u32(data + 24, x);
    y = r_u32(object_offset + 164) - r_u32(other_offset + 164);
    w_u32(data + 28, y);
    z = r_u32(object_offset + 168) - r_u32(other_offset + 168);
    w_u32(data + 32, z);
    x = (uint32)(r_s32(data + 24) >> 6);
    y = (uint32)(r_s32(data + 28) >> 6);
    distance = x * x + y * y;
    x = (uint32)(r_s32(other_offset + 92) >> 6);
    y = (uint32)(r_s32(other_offset + 96) >> 6);
    speed = x * x + y * y;
    x = (uint32)(r_s32(other_offset + 100) >> 6);
    z = (uint32)((sint32)z >> 6);
    speed += x * x;
    distance += z * z;
    if (distance == 0)
    {
        trap(7);
        abort();
    }
    quotient = 0x40000000 / (sint32)distance;
    magnitude = (uint32)(((sint64)(sint32)speed * quotient) >> 18);
    w_u32(0x1F80030C, magnitude);
    if ((sint32)magnitude < 0)
        scale = p2_sdk_8007e794(magnitude >> 2) << 1;
    else
        scale = p2_sdk_8007e794(magnitude);
    if ((sint32)scale < 64)
        scale = 64;
    for (index = 0; index < 3; ++index)
    {
        value = (uint32)(((sint64)r_s32(data + 24 + index * 4) * (sint32)scale) >> 12);
        w_u32(data + 24 + index * 4, value);
    }
    return 0;
}

uint32 p2_func_80045ed8(uint32 object_offset)
{
    uint32 saved_stack = p2_guest_stack_pointer;
    uint32 scene = r_u32(0x8008A114), parent, matrix, vector, value, index, result;
    uint32 x, y, z;
    sint32 divisor, coefficient;
    p2_guest_stack_pointer -= 16;
    vector = p2_guest_stack_pointer;
    p2_func_80022c24(r_u32(scene + 136), r_s16(scene + 2), scene + 28, r_u32(object_offset + 28) & 0x10);
    value = r_u16(0x800ABDA0) - 1;
    w_u16(0x800ABDA0, value);
    if ((value & 0xFFFF) == 0)
        p2_func_80045d0c(object_offset, 0x800ABD68);
    else
    {
        parent = r_u32(object_offset + 132);
        if (parent != 0 && r_u32(parent + 40) != 0)
        {
            x = r_u32(object_offset + 16);
            y = r_u32(object_offset + 20);
            z = r_u32(object_offset + 24);
            w_u32(vector, x);
            w_u32(vector + 4, y);
            w_u32(vector + 8, z);
            PushMatrix();
            matrix = r_u32(r_u32(object_offset + 132) + 40);
            for (index = 0; index < 5; ++index)
                xport_gte_write_control(index, r_u32(matrix + index * 4));
            p2_sdk_8007ec64(vector, object_offset + 92);
            PopMatrix();
            for (index = 0; index < 3; ++index)
            {
                matrix = r_u32(r_u32(object_offset + 132) + 40);
                value = r_u32(object_offset + 92 + index * 4) + r_u32(matrix + 20 + index * 4);
                w_u32(object_offset + 92 + index * 4, value);
            }
        }
        else
        {
            x = r_u32(object_offset + 16);
            y = r_u32(object_offset + 20);
            z = r_u32(object_offset + 24);
            w_u32(object_offset + 92, x);
            w_u32(object_offset + 96, y);
            w_u32(object_offset + 100, z);
        }
        value = r_u32(object_offset + 96);
        value += r_s32(0x800ABD68) < 0 ? 0x8000 : 14336;
        w_u32(object_offset + 96, value);
        scene = r_u32(0x8008A114);
        for (index = 0; index < 3; ++index)
        {
            value = r_u32(object_offset + 92 + index * 4) - r_u32(scene + 16 + index * 4);
            w_u32(object_offset + 92 + index * 4, value);
        }
        for (index = 0; index < 3; ++index)
        {
            divisor = r_s16(0x800ABDA0) * 4096;
            if (divisor == 0)
            {
                trap(7);
                abort();
            }
            coefficient = 0x1000000 / divisor;
            value = (uint32)(((sint64)r_s32(object_offset + 92 + index * 4) * coefficient) >> 12);
            w_u32(object_offset + 92 + index * 4, value);
        }
        for (index = 0; index < 3; ++index)
        {
            value = r_u32(scene + 16 + index * 4) + r_u32(object_offset + 92 + index * 4);
            w_u32(scene + 16 + index * 4, value);
        }
    }
    scene = r_u32(0x8008A114);
    result = r_u8(scene) & 0xFE;
    w_u8(scene, result);
    p2_guest_stack_pointer = saved_stack;
    return result;
}

uint32 p2_func_800611e8(uint32 object_offset)
{
    uint32 parent = r_u32(object_offset + 108);
    uint32 result = (uint32)(sint32)r_s16(parent + 136);
    uint32 flags, parent_flags, frame;
    if (result == 0)
        return result;
    result = 0x40000;
    if ((r_u8(object_offset + 13) & 1) != 0)
        return result;
    flags = r_u32(object_offset + 28) | 0x40000;
    w_u32(object_offset + 28, flags);
    parent_flags = r_u32(parent + 28);
    if ((parent_flags & 0x1000000) != 0)
        w_u32(object_offset + 28, flags | 0x20000);
    else if ((parent_flags & 0x2000000) != 0)
        w_u32(object_offset + 28, flags | 0x10000);
    frame = r_u16(parent + 136);
    w_u16(object_offset + 136, frame);
    p2_func_8001b334(object_offset, (sint16)frame);
    parent_flags = r_u32(parent + 28);
    result = parent_flags & 0x840000;
    if (result == 0)
    {
        flags = r_u32(object_offset + 28);
        result = parent_flags | 0x40000;
        if ((flags & 0x40000) != 0)
            w_u32(parent + 28, result);
    }
    return result;
}

uint32 p2_func_80044230(uint32 object_offset)
{
    uint32 parent = r_u32(object_offset + 132);
    uint32 result;
    w_u8(parent + 13, r_u8(parent + 13) | 0x80);
    result = r_u8(object_offset + 13) | 0x80;
    w_u8(object_offset + 13, result);
    return result;
}

uint32 p2_func_80030ac8(uint32 object_offset)
{
    uint32 data = r_u32(r_u32(object_offset + 132) + 44);
    uint32 value = r_u32(data + 220) - 1;
    uint32 parent, flags, angle, result;
    w_u32(data + 220, value);
    if ((value & 0xFF) == 0)
    {
        w_u32(object_offset + 28, r_u32(object_offset + 28) & 0xFDFFFFFF);
        value = r_u32(data + 220);
        w_u8(object_offset + 126, 2);
        w_u8(object_offset + 125, (uint32)((sint32)value >> 8));
        angle = (uint32)(sint32)r_s16(data + 222) & 0xFFF;
        w_u16(object_offset + 148, angle);
        w_u16(object_offset + 150, angle);
    }
    else
    {
        parent = r_u32(object_offset + 132);
        flags = parent == 0 ? 0 : r_u8(r_u32(parent + 44) + 38) & 8;
        if (flags != 0)
            angle = r_u16(object_offset + 150) - 6553;
        else
        {
            parent = r_u32(r_u32(r_u32(object_offset + 132) + 44) + 72);
            if (r_s16(parent + 636) == 1)
                goto finish;
            angle = r_u16(object_offset + 150) - 3276;
        }
        w_u16(object_offset + 150, angle);
        w_u16(object_offset + 148, (angle << 17) >> 20);
    }
finish:
    result = r_u16(object_offset + 148);
    w_u16(object_offset + 84, 0x7FFF);
    w_u16(object_offset + 86, 0);
    result = (result + 2048) & 0xFFF;
    w_u16(object_offset + 88, result);
    return result;
}

uint32 p2_func_800351a4(uint32 object_offset)
{
    uint32 data = r_u32(r_u32(object_offset + 132) + 44);
    uint32 flags, sound, value, result;
    if ((p2_func_8005e868(r_u32(data + 68), (uint32)(sint32)r_s16(data + 66)) & 0xFF) != 0)
        p2_func_8006a970(r_u16(data + 66));
    flags = r_u32(data + 44);
    w_u32(data + 68, 0);
    w_u16(data + 66, 0xFFFF);
    if ((flags & 0x28) != 0)
    {
        value = r_u32(0x8008A408);
        sound = value != 0 && r_s32(value) >= 23 ? value + 180 : 0;
        p2_func_8005e538(sound, 0, 0, r_u16(0x8008A24C), 0, 3);
        w_u16(object_offset + 128, 0);
    }
    else if ((flags & 1) != 0)
        w_u32(object_offset + 116, (r_u32(object_offset + 116) & 0xFFFFFF00) | 0x96);
    else
    {
        value = r_u32(object_offset + 116) - 1;
        w_u32(object_offset + 116, value);
        if ((value & 0xFF) == 0)
        {
            result = (uint32)((sint32)value >> 8);
            w_u8(object_offset + 125, result);
            return result;
        }
    }
    result = r_u32(0x8008A520);
    w_u32(0x1F8002A8, result);
    return result;
}

sint32 p2_sdk_80079a18(uint16 bank, sint16 program)
{
    uint32 index, tone_table, program_table, sample_table, tone;
    if (bank >= 16)
        return -1;
    if (r_u8(0x800B7F90 + bank) != 1)
        return -1;
    if ((sint32)program >= r_s16(0x800B1704))
        return -1;
    index = 4 * (uint32)bank;
    sample_table = r_u32(0x800B15B0 + index);
    program_table = r_u32(0x800B1570 + index);
    tone_table = r_u32(0x800B1698 + index);
    w_u8(0x800B7F71, bank);
    w_u8(0x800B7F76, program);
    w_u32(0x800B182C, tone_table);
    w_u32(0x800B1828, sample_table);
    w_u32(0x800B1820, program_table);
    tone = r_u8(program_table + 16 * (uint32)(sint32)program + 8);
    w_u8(0x800B7F77, tone);
    return 0;
}

sint32 p2_sdk_80075fd0(sint16 bank, sint16 program, uint32 destination_offset)
{
    uint32 offset, index, value;
    if (r_u8(0x800B7F90 + (uint32)(sint32)bank) != 1)
        return -1;
    p2_sdk_80079a18((uint16)bank, program);
    offset = 16 * (uint32)(sint32)program;
    for (index = 0; index < 5; ++index)
    {
        value = r_u8(r_u32(0x800B1820) + offset + index);
        w_u8(destination_offset + index, value);
    }
    value = r_u16(r_u32(0x800B1820) + offset + 6);
    w_u16(destination_offset + 6, value);
    return 0;
}

sint32 p2_sdk_800760d8(sint16 bank, sint16 program, uint32 tone, uint32 destination_offset)
{
    static const uint8 byte_order[14] = {0, 1, 2, 3, 4, 5, 7, 6, 8, 9, 10, 11, 12, 13};
    uint32 offset, index, byte, value, source;
    if (r_u8(0x800B7F90 + (uint32)(sint32)bank) != 1)
        return -1;
    p2_sdk_80079a18((uint16)bank, program);
    offset = (uint32)((sint32)(sint16)(tone + 16 * r_u8(0x800B7F77)) * 32);
    for (index = 0; index < 14; ++index)
    {
        byte = byte_order[index];
        value = r_u8(r_u32(0x800B182C) + offset + byte);
        w_u8(destination_offset + byte, value);
    }
    source = r_u32(0x800B182C) + offset;
    for (index = 16; index <= 22; index += 2)
    {
        value = r_u16(source + index);
        w_u16(destination_offset + index, value);
    }
    return 0;
}

sint32 p2_sdk_800765f0(sint16 bank, sint16 program, uint32 tone, uint32 source_offset)
{
    static const uint8 byte_order[14] = {0, 1, 2, 3, 4, 5, 7, 6, 8, 9, 10, 11, 12, 13};
    uint32 offset, index, byte, value, destination;
    if (r_u8(0x800B7F90 + (uint32)(sint32)bank) != 1)
        return -1;
    p2_sdk_80079a18((uint16)bank, program);
    offset = (uint32)((sint32)(sint16)(tone + 16 * r_u8(0x800B7F77)) * 32);
    for (index = 0; index < 14; ++index)
    {
        byte = byte_order[index];
        destination = r_u32(0x800B182C) + offset;
        value = r_u8(source_offset + byte);
        w_u8(destination + byte, value);
    }
    destination = r_u32(0x800B182C) + offset;
    for (index = 16; index <= 22; index += 2)
    {
        value = r_u16(source_offset + index);
        w_u16(destination + index, value);
    }
    return 0;
}

static uint32 p2_update_tone_byte(sint16 bank, sint16 program, sint16 tone, uint32 attributes_offset, uint8 value, uint32 field)
{
    uint32 saved_stack = p2_guest_stack_pointer;
    uint32 attributes, index, result;
    p2_guest_stack_pointer -= 32;
    attributes = p2_guest_stack_pointer;
    /* The caller owns the disjoint input structure above this temporary */
    for (index = 0; index < 32; index += 4)
        w_u32(attributes + index, r_u32(attributes_offset + index));
    p2_sdk_800760d8(bank, program, (uint32)(sint32)tone, attributes);
    w_u8(attributes + field, value);
    result = (uint32)p2_sdk_800765f0(bank, program, (uint32)(sint32)tone, attributes);
    p2_guest_stack_pointer = saved_stack;
    return result;
}

uint32 p2_sdk_80072b5c(sint16 bank, sint16 program, sint16 tone, uint32 attributes_offset, uint8 value)
{
    return p2_update_tone_byte(bank, program, tone, attributes_offset, value, 0);
}

uint32 p2_sdk_80072bec(sint16 bank, sint16 program, sint16 tone, uint32 attributes_offset, uint8 value)
{
    p2_update_tone_byte(bank, program, tone, attributes_offset, value, 1);
    if (value == 0)
        return p2_sdk_800764a0(0);
    if (value == 4)
        return p2_sdk_800764a0(1);
    return 4;
}

uint32 p2_sdk_80072ca8(sint16 bank, sint16 program, sint16 tone, uint32 attributes_offset, uint8 value)
{
    return p2_update_tone_byte(bank, program, tone, attributes_offset, value, 6);
}

uint32 p2_sdk_80072d38(sint16 bank, sint16 program, sint16 tone, uint32 attributes_offset, uint8 value)
{
    return p2_update_tone_byte(bank, program, tone, attributes_offset, value, 7);
}

uint32 p2_sdk_800756a8(sint16 sequence, sint16 separation)
{
    uint32 slot, offset, record, flags, cursor, result;
    slot = 0x800B9B00 + 4 * (uint32)(sint32)sequence;
    offset = 172 * (uint32)(sint32)separation;
    record = r_u32(slot) + offset;
    w_u16(record + 70, 1);
    w_u16(record + 72, 0);
    flags = r_u32(slot) + offset + 144;
    w_u32(flags, r_u32(flags) & 0xFFFFFEFF);
    flags = r_u32(slot) + offset + 144;
    w_u32(flags, r_u32(flags) & 0xFFFFFFF7);
    flags = r_u32(slot) + offset + 144;
    w_u32(flags, r_u32(flags) & 0xFFFFFFFD);
    flags = r_u32(slot) + offset + 144;
    w_u32(flags, r_u32(flags) & 0xFFFFFFFB);
    flags = r_u32(slot) + offset + 144;
    w_u32(flags, r_u32(flags) & 0xFFFFFDFF);
    cursor = r_u32(record + 8);
    w_u8(record + 43, 1);
    w_u32(record + 4, cursor);
    flags = r_u32(slot) + offset + 144;
    result = r_u32(flags) | 1;
    w_u32(flags, result);
    return result;
}

uint32 p2_sdk_80073a10(sint16 sequence, sint16 separation)
{
    uint32 record, cursor, high, middle, microseconds, tempo, product;
    uint32 rate, quarter_rate, denominator, numerator, quotient, remainder, result;
    record = r_u32(0x800B9B00 + 4 * (uint32)(sint32)sequence) + 172 * (uint32)(sint32)separation;
    cursor = r_u32(record + 4);
    w_u32(record + 4, cursor + 1);
    high = r_u8(cursor);
    w_u32(record + 4, cursor + 2);
    middle = r_u8(cursor + 1);
    w_u32(record + 4, cursor + 3);
    microseconds = (high << 16) | (middle << 8) | r_u8(cursor + 2);
    if (microseconds == 0)
    {
        fprintf(stderr, "P2: Sequence tempo division by zero at 80073AA4\n");
        abort();
    }
    tempo = 60000000 / microseconds;
    product = (uint32)(sint32)r_s16(record + 74) * tempo;
    rate = r_u32(0x800B9930);
    quarter_rate = 15 * rate;
    denominator = 4 * quarter_rate;
    w_u32(record + 140, tempo);
    if (10 * product < denominator)
    {
        if (product == 0)
        {
            fprintf(stderr, "P2: Sequence tempo division by zero at 80073B24\n");
            abort();
        }
        quotient = (600 * rate) / product;
        w_u16(record + 110, quotient);
        w_u16(record + 112, quotient);
    }
    else
    {
        product = (uint32)(sint32)r_s16(record + 74) * r_u32(record + 140);
        numerator = 10 * product;
        if (denominator == 0)
        {
            fprintf(stderr, "P2: Sequence tempo division by zero at 80073B68\n");
            abort();
        }
        quotient = numerator / denominator;
        product = (uint32)(sint32)r_s16(record + 74) * r_u32(record + 140);
        remainder = (10 * product) % denominator;
        w_u16(record + 110, 0xFFFF);
        w_u16(record + 112, quotient);
        if (2 * quarter_rate < remainder)
            w_u16(record + 112, quotient + 1);
    }
    result = p2_sdk_80073d58(sequence, separation);
    w_u32(record + 136, result);
    return result;
}

uint32 p2_sdk_80072444(sint16 sequence, sint16 separation, uint8 value)
{
    uint32 record, channel, program, pan, result;
    sint16 bank, packed;
    record = r_u32(0x800B9B00 + 4 * (uint32)(sint32)sequence) + 172 * (uint32)(sint32)separation;
    channel = r_u8(record + 18);
    bank = r_s16(record + 76);
    program = r_u8(record + channel + 44);
    pan = r_u8(record + channel + 23);
    packed = (sint16)((uint32)(sint32)sequence | ((uint32)(sint32)separation << 8));
    p2_sdk_800794a0(packed, bank, (sint16)program, (uint16)value, (uint16)pan);
    w_u16(record + 2 * channel + 78, value);
    result = p2_sdk_80073d58(sequence, separation);
    w_u32(record + 136, result);
    return result;
}

uint32 p2_sdk_80072518(sint16 sequence, sint16 separation, uint8 value)
{
    uint32 record, channel, channel_record, program, volume, result;
    sint16 bank, packed;
    record = r_u32(0x800B9B00 + 4 * (uint32)(sint32)sequence) + 172 * (uint32)(sint32)separation;
    channel = r_u8(record + 18);
    bank = r_s16(record + 76);
    channel_record = record + channel;
    program = r_u8(channel_record + 44);
    volume = r_u16(record + 2 * channel + 78);
    packed = (sint16)((uint32)(sint32)sequence | ((uint32)(sint32)separation << 8));
    p2_sdk_800794a0(packed, bank, (sint16)program, (uint16)volume, (uint16)value);
    w_u8(channel_record + 23, value);
    result = p2_sdk_80073d58(sequence, separation);
    w_u32(record + 136, result);
    return result;
}

uint32 p2_sdk_80072a94(sint16 sequence, sint16 separation)
{
    uint32 record, channel, program, result;
    record = r_u32(0x800B9B00 + 4 * (uint32)(sint32)sequence) + 172 * (uint32)(sint32)separation;
    /* Inline the original reverb-off and damper-off helpers */
    p2_sdk_800764a0(0);
    w_u16(0x800B1690, 0);
    channel = r_u8(record + 18);
    program = r_u8(record + 18);
    w_u8(record + channel + 44, program);
    channel = r_u8(record + 18);
    w_u8(record + 19, 0);
    w_u8(record + 20, 0);
    w_u16(record + 2 * channel + 78, 127);
    channel = r_u8(record + 18);
    w_u8(record + channel + 23, 64);
    result = p2_sdk_80073d58(sequence, separation);
    w_u32(record + 136, result);
    return result;
}

uint32 p2_sdk_80072728(sint16 sequence, sint16 separation, uint8 value)
{
    uint32 record, mode, count, target, result;
    record = r_u32(0x800B9B00 + 4 * (uint32)(sint32)sequence) + 172 * (uint32)(sint32)separation;
    if (r_u8(record + 39) == 1 && r_u8(record + 16) == 0)
    {
        w_u8(record + 40, value);
        w_u8(record + 39, 0);
        w_u8(record + 16, 1);
        mode = r_u8(record + 22);
    }
    else
    {
        mode = r_u8(record + 22);
        if (mode != 30 && mode != 20)
        {
            count = r_u8(record + 42);
            w_u8(record + 21, value);
            w_u8(record + 39, 0);
            w_u8(record + 42, count + 1);
            mode = r_u8(record + 22);
        }
    }
    if (mode == 40)
    {
        target = r_u32(0x800B9B80 + 64 * (uint32)(sint32)sequence + 4 * (uint32)(sint32)separation);
        if (target != 0)
        {
            /* TODO: Bind reviewed user-installed marker callbacks */
            fprintf(stderr, "P2: Unmapped sequence marker callback %08X\n", target);
            abort();
        }
    }
    result = p2_sdk_80073d58(sequence, separation);
    w_u32(record + 136, result);
    return result;
}

uint32 p2_sdk_800725e8(sint16 sequence, sint16 separation, uint8 value)
{
    uint32 record, result;
    record = r_u32(0x800B9B00 + 4 * (uint32)(sint32)sequence) + 172 * (uint32)(sint32)separation;
    /* Inline the original single-store damper helpers */
    w_u16(0x800B1690, value >= 0x40 ? 2 : 0);
    result = p2_sdk_80073d58(sequence, separation);
    w_u32(record + 136, result);
    return result;
}

uint32 p2_sdk_80072698(sint16 sequence, sint16 separation, uint8 value)
{
    uint32 record, result;
    record = r_u32(0x800B9B00 + 4 * (uint32)(sint32)sequence) + 172 * (uint32)(sint32)separation;
    p2_sdk_80076354((sint16)value, (sint16)value);
    result = p2_sdk_80073d58(sequence, separation);
    w_u32(record + 136, result);
    return result;
}

uint32 p2_sdk_80072860(sint16 sequence, sint16 separation, uint8 value)
{
    uint32 record, count, result, cursor;
    record = r_u32(0x800B9B00 + 4 * (uint32)(sint32)sequence) + 172 * (uint32)(sint32)separation;
    if (value == 20)
    {
        w_u8(record + 22, value);
        w_u8(record + 39, 1);
        result = p2_sdk_80073d58(sequence, separation);
        cursor = r_u32(record + 4);
        w_u32(record + 136, result);
        w_u32(record + 12, cursor);
        return result;
    }
    if (value == 30)
    {
        count = r_u8(record + 40);
        w_u8(record + 22, value);
        if (count != 0)
        {
            if (count >= 0x7F)
            {
                p2_sdk_80073d58(sequence, separation);
                result = r_u32(record + 12);
                w_u32(record + 136, 0);
                w_u32(record + 4, result);
                return result;
            }
            count = r_u8(record + 40);
            w_u8(record + 40, count - 1);
            result = p2_sdk_80073d58(sequence, separation);
            count = r_u8(record + 40);
            w_u32(record + 136, result);
            if (count != 0)
            {
                result = r_u32(record + 12);
                w_u32(record + 4, result);
            }
            else
                w_u8(record + 16, 0);
            return result;
        }
        w_u8(record + 16, 0);
    }
    else
    {
        count = r_u8(record + 42);
        w_u8(record + 22, value);
        w_u8(record + 42, count + 1);
    }
    result = p2_sdk_80073d58(sequence, separation);
    w_u32(record + 136, result);
    return result;
}

uint32 p2_sdk_800729a4(sint16 sequence, sint16 separation, uint8 value)
{
    uint32 record, count, result;
    record = r_u32(0x800B9B00 + 4 * (uint32)(sint32)sequence) + 172 * (uint32)(sint32)separation;
    count = r_u8(record + 41);
    w_u8(record + 19, value);
    w_u8(record + 41, count + 1);
    result = p2_sdk_80073d58(sequence, separation);
    w_u32(record + 136, result);
    return result;
}

uint32 p2_sdk_80072a1c(sint16 sequence, sint16 separation, uint8 value)
{
    uint32 record, count, result;
    record = r_u32(0x800B9B00 + 4 * (uint32)(sint32)sequence) + 172 * (uint32)(sint32)separation;
    count = r_u8(record + 41);
    w_u8(record + 20, value);
    w_u8(record + 41, count + 1);
    result = p2_sdk_80073d58(sequence, separation);
    w_u32(record + 136, result);
    return result;
}

static void p2_require_sequence_handler(uint32 slot, uint32 expected)
{
    uint32 target = r_u32(slot);
    if (target != expected)
    {
        /* TODO: Bind any additional reviewed sequence callbacks */
        fprintf(stderr, "P2: Unmapped sequence callback %08X at %08X\n", target, slot);
        abort();
    }
}

uint32 p2_sdk_800737d0(sint16 sequence, sint16 separation, uint8 controller)
{
    uint32 record, cursor, value, result;
    record = r_u32(0x800B9B00 + 4 * (uint32)(sint32)sequence) + 172 * (uint32)(sint32)separation;
    cursor = r_u32(record + 4);
    w_u32(record + 4, cursor + 1);
    value = r_u8(cursor);
    switch (controller)
    {
        case 0:
            w_u16(record + 76, value);
            break;
        case 6:
            p2_require_sequence_handler(0x800AC120, 0x8007201C);
            return p2_sdk_8007201c(sequence, separation, (uint8)value);
        case 7:
            p2_require_sequence_handler(0x800AC124, 0x80072444);
            return p2_sdk_80072444(sequence, separation, (uint8)value);
        case 10:
            p2_require_sequence_handler(0x800AC128, 0x80072518);
            return p2_sdk_80072518(sequence, separation, (uint8)value);
        case 11:
        case 64:
            p2_require_sequence_handler(0x800AC12C, 0x800725E8);
            return p2_sdk_800725e8(sequence, separation, (uint8)value);
        case 91:
            p2_require_sequence_handler(0x800AC140, 0x80072698);
            return p2_sdk_80072698(sequence, separation, (uint8)value);
        case 98:
            p2_require_sequence_handler(0x800AC130, 0x80072728);
            return p2_sdk_80072728(sequence, separation, (uint8)value);
        case 99:
            p2_require_sequence_handler(0x800AC134, 0x80072860);
            return p2_sdk_80072860(sequence, separation, (uint8)value);
        case 100:
            p2_require_sequence_handler(0x800AC138, 0x800729A4);
            return p2_sdk_800729a4(sequence, separation, (uint8)value);
        case 101:
            p2_require_sequence_handler(0x800AC13C, 0x80072A1C);
            return p2_sdk_80072a1c(sequence, separation, (uint8)value);
        case 121:
            p2_require_sequence_handler(0x800AC144, 0x80072A94);
            return p2_sdk_80072a94(sequence, separation);
    }
    result = p2_sdk_80073d58(sequence, separation);
    w_u32(record + 136, result);
    return result;
}

uint32 p2_sdk_800753c4(sint16 sequence, sint16 separation)
{
    uint32 record, cursor, event, status, velocity, delta;
    record = r_u32(0x800B9B00 + 4 * (uint32)(sint32)sequence) + 172 * (uint32)(sint32)separation;
    cursor = r_u32(record + 4);
    w_u32(record + 4, cursor + 1);
    event = r_u8(cursor);
    if ((event & 0x80) != 0)
    {
        status = event & 0xF0;
        w_u8(record + 18, event & 0x0F);
        switch (status)
        {
            case 0x90:
                cursor = r_u32(record + 4);
                w_u8(record + 17, status);
                w_u32(record + 4, cursor + 1);
                event = r_u8(cursor);
                w_u32(record + 4, cursor + 2);
                velocity = r_u8(cursor + 1);
                delta = p2_sdk_80073d58(sequence, separation);
                w_u32(record + 136, delta);
                p2_require_sequence_handler(0x800AC108, 0x80073BF0);
                return p2_sdk_80073bf0(sequence, separation, (uint8)event, (uint8)velocity);
            case 0xB0:
            case 0xC0:
                cursor = r_u32(record + 4);
                w_u8(record + 17, status);
                w_u32(record + 4, cursor + 1);
                event = r_u8(cursor);
                break;
            case 0xE0:
                cursor = r_u32(record + 4);
                w_u8(record + 17, status);
                w_u32(record + 4, cursor + 1);
                break;
            case 0xF0:
                cursor = r_u32(record + 4);
                w_u8(record + 17, 0xFF);
                w_u8(record + 18, event & 0x0F);
                w_u32(record + 4, cursor + 1);
                event = r_u8(cursor);
                status = 0xFF;
                break;
            default:
                return status < 0xC1 ? 0xB0 : 0xFF;
        }
    }
    else
    {
        status = r_u8(record + 17);
        if (status == 0x90)
        {
            w_u32(record + 4, cursor + 2);
            velocity = r_u8(cursor + 1);
            delta = p2_sdk_80073d58(sequence, separation);
            w_u32(record + 136, delta);
            p2_require_sequence_handler(0x800AC108, 0x80073BF0);
            return p2_sdk_80073bf0(sequence, separation, (uint8)event, (uint8)velocity);
        }
    }
    switch (status)
    {
        case 0xB0:
            p2_require_sequence_handler(0x800AC118, 0x800737D0);
            return p2_sdk_800737d0(sequence, separation, (uint8)event);
        case 0xC0:
            p2_require_sequence_handler(0x800AC10C, 0x80073CE0);
            return p2_sdk_80073ce0(sequence, separation, (uint8)event);
        case 0xE0:
            p2_require_sequence_handler(0x800AC110, 0x80073718);
            return p2_sdk_80073718(sequence, separation);
        case 0xFF:
            if (event == 0x2F)
                return p2_sdk_80075200(sequence, separation);
            p2_require_sequence_handler(0x800AC114, 0x80073A10);
            return p2_sdk_80073a10(sequence, separation);
        default:
            return status < 0xC1 ? 0xB0 : 0xFF;
    }
}

uint32 p2_sdk_80075200(sint16 sequence, sint16 separation)
{
    uint32 slot, offset, record, count, result, copy, flags, value, bank;
    sint32 repeats;
    sint16 packed;
    slot = 0x800B9B00 + 4 * (uint32)(sint32)sequence;
    offset = 172 * (uint32)(sint32)separation;
    record = r_u32(slot) + offset;
    count = r_u16(record + 72) + 1;
    repeats = r_s16(record + 70);
    w_u16(record + 72, count);
    if (repeats == 0)
    {
        result = r_u32(record + 8);
        w_u32(record + 128, 0);
        w_u8(record + 39, 0);
        w_u32(record + 136, 0);
        w_u32(record + 4, result);
        return result;
    }
    if ((sint16)count < repeats)
    {
        result = r_u32(record + 8);
        copy = r_u32(record + 8);
        w_u32(record + 128, 0);
        w_u8(record + 39, 0);
        w_u32(record + 136, 0);
        w_u32(record + 4, result);
        w_u32(record + 12, copy);
        return result;
    }
    flags = r_u32(slot) + offset + 144;
    w_u32(flags, r_u32(flags) & 0xFFFFFFFE);
    flags = r_u32(slot) + offset + 144;
    w_u32(flags, r_u32(flags) & 0xFFFFFFF7);
    flags = r_u32(slot) + offset + 144;
    w_u32(flags, r_u32(flags) & 0xFFFFFFFD);
    flags = r_u32(slot) + offset + 144;
    w_u32(flags, r_u32(flags) | 0x200);
    flags = r_u32(slot) + offset + 144;
    w_u32(flags, r_u32(flags) | 4);
    result = r_u32(record + 8);
    value = r_u8(record + 60);
    w_u8(record + 43, 0);
    w_u32(record + 12, result);
    packed = (sint16)((uint32)(sint32)sequence | ((uint32)(sint32)separation << 8));
    if (value != 0xFF)
    {
        value = r_u8(record + 60);
        bank = r_u8(record);
        w_u8(record + 43, 0);
        p2_sdk_800756a8((sint16)value, (sint16)bank);
        p2_sdk_80079350(packed);
    }
    p2_sdk_80079350(packed);
    result = (uint32)(sint32)r_s16(record + 112);
    w_u32(record + 136, result);
    return result;
}

uint32 p2_sdk_80073d58(sint16 sequence, sint16 separation)
{
    uint32 record, cursor, value, byte, result;
    record = r_u32(0x800B9B00 + 4 * (uint32)(sint32)sequence) + 172 * (uint32)(sint32)separation;
    cursor = r_u32(record + 4);
    w_u32(record + 4, cursor + 1);
    value = r_u8(cursor);
    if (value == 0)
        return 0;
    if ((value & 0x80) != 0)
    {
        value &= 0x7F;
        do
        {
            cursor = r_u32(record + 4);
            w_u32(record + 4, cursor + 1);
            byte = r_u8(cursor);
            value = (value << 7) + (byte & 0x7F);
        } while ((byte & 0x80) != 0);
    }
    result = value * 10;
    w_u32(record + 128, r_u32(record + 128) + result);
    return result;
}

uint32 p2_sdk_80073ce0(sint16 sequence, sint16 separation, uint8 program)
{
    uint32 record, result;
    record = r_u32(0x800B9B00 + 4 * (uint32)(sint32)sequence) + 172 * (uint32)(sint32)separation;
    w_u8(record + r_u8(record + 18) + 44, program);
    result = p2_sdk_80073d58(sequence, separation);
    w_u32(record + 136, result);
    return result;
}

void p2_sdk_80011274(void)
{
    /* The original __main consists only of a return */
}

uint32 p2_sdk_800829d8(uint32 destination, uint32 value, uint32 bytes)
{
    uint32 index;
    for (index = 0; index < bytes; ++index)
        w_u8(destination + index, value);
    return destination;
}

uint32 p2_sdk_800834ac(uint32 counter, uint32 target, uint32 mode)
{
    return (uint32)SetRCnt(counter, (uint16)target, (sint32)mode);
}

uint32 p2_sdk_80083584(uint32 counter)
{
    return (uint32)StartRCnt(counter);
}

uint32 p2_sdk_8007ec64(uint32 source_offset, uint32 destination_offset)
{
    return ApplyRotMatrixLVPSX(source_offset, destination_offset);
}

uint32 p2_sdk_8007e944(uint32 matrix_offset)
{
    return MulRotMatrixPSX(matrix_offset);
}

uint32 p2_sdk_8007ea34(uint32 matrix_offset, uint32 destination_offset)
{
    return MulRotMatrix0PSX(matrix_offset, destination_offset);
}

uint32 p2_sdk_8007e834(uint32 left_offset, uint32 right_offset, uint32 destination_offset)
{
    MulMatrix0((MATRIX *)psx_addr(left_offset, 20), (MATRIX *)psx_addr(right_offset, 20), (MATRIX *)psx_addr(destination_offset, 20));
    return destination_offset;
}

sint32 p2_sdk_8006ff38(uint32 attributes_offset)
{
    return SpuSetReverbModeParam((SpuReverbAttr *)psx_addr(attributes_offset, sizeof(SpuReverbAttr)));
}

uint32 p2_sdk_8007a740(uint32 mode)
{
    return (uint32)(sint32)(sint16)SpuIsTransferCompleted((sint16)mode);
}

sint32 p2_sdk_80070df8(uint32 mask)
{
    return SpuGetKeyStatus(mask);
}

uint32 p2_sdk_80070eec(uint32 address)
{
    uint32 aligned, units;
    if (address - 0x1010 > 0x7EFE8)
        return 0;
    aligned = SpuSetTransferStartAddr(address);
    if (aligned == 0)
        return 0;
    units = (aligned >> (r_u32(0x80087D84) & 31)) & 0xFFFF;
    w_u16(0x80087D74, units);
    return units;
}

uint32 p2_sdk_800764a0(uint32 enabled)
{
    return (uint32)SpuSetReverb((sint32)enabled);
}

uint32 p2_sdk_800750f0(sint16 sequence, sint16 separation)
{
    uint32 record, ticks, result, accumulated;
    sint32 duration, countdown, delta;
    /* P2 uses the 172-byte LIBSND record layout */
    record = r_u32(0x800B9B00 + 4 * (uint32)(sint32)sequence) + 172 * (uint32)(sint32)separation;
    duration = r_s16(record + 112);
    ticks = r_u32(record + 136);
    delta = (sint32)(ticks - (uint32)duration);
    if (delta > 0)
    {
        countdown = r_s16(record + 110);
        result = (uint32)countdown;
        if (countdown > 0)
        {
            --result;
            w_u16(record + 110, result);
            return result;
        }
        if (countdown != 0)
        {
            w_u32(record + 136, (uint32)delta);
            return result;
        }
        result = r_u32(record + 136) - 1;
        w_u16(record + 110, duration);
    }
    else
    {
        result = duration < (sint32)ticks;
        accumulated = ticks;
        if (result != 0)
            return result;
        do
        {
            do
            {
                p2_sdk_800753c4(sequence, separation);
                ticks = r_u32(record + 136);
            } while (ticks == 0);
            duration = r_s16(record + 112);
            accumulated += ticks;
            result = accumulated - (uint32)duration;
        } while ((sint32)accumulated < duration);
    }
    w_u32(record + 136, result);
    return result;
}

uint32 p2_func_8004264c(uint32 object_offset)
{
    uint32 saved_stack, local, vector, motion, floor, input, rotation, world_input;
    uint32 inverse_input, inverse_rotation, event_input, sample_input, final_input, final_rotation;
    uint32 data, speed, owner, definition = 0, cursor, count, kind_id, initial_kind;
    uint32 flags, parent, mode, value, index, x, y, z, result, sound, sound_table;
    uint32 length, option = 0, volume = 4096, base, factor, node, distance, changed, previous;
    sint32 variant, frame, pitch, denominator, quotient;
    variant = r_s8(object_offset + 127);
    if (variant != 0)
        return p2_dispatch_object_variant(r_u32(0x800ABD50 + (uint32)variant * 4), object_offset);
    saved_stack = p2_guest_stack_pointer;
    p2_guest_stack_pointer -= 224;
    local = p2_guest_stack_pointer;
    vector = local;
    motion = local + 16;
    floor = local + 32;
    input = local + 48;
    rotation = local + 64;
    world_input = local + 88;
    inverse_input = local + 104;
    inverse_rotation = local + 120;
    event_input = local + 144;
    sample_input = local + 160;
    final_input = local + 176;
    final_rotation = local + 192;
    initial_kind = r_u8(object_offset + 124);
    owner = r_u32(object_offset + 8);
    data = r_u32(r_u32(object_offset + 132) + 44);
    speed = data + 12;
    count = r_u32(0x8008A338);
    cursor = 0x8008B4A8;
    w_u32(motion + 8, 0);
    w_u32(motion + 4, 0);
    w_u32(motion, 0);
    kind_id = r_u32(object_offset + 116);
    while (count != 0)
    {
        definition = r_u32(cursor);
        cursor += 4;
        if ((uint32)(sint32)r_s8(definition + 2) == kind_id)
            break;
        --count;
    }
    if (count == 0)
        definition = 0;
    if (r_s8(object_offset + 124) == 3)
    {
        if (r_u32(object_offset + 144) != 0)
            w_u32(object_offset + 28, r_u32(object_offset + 28) & 0xFEFFFFFF);
        flags = r_u32(object_offset + 28);
        if ((flags & 0x4000) != 0)
            w_u32(object_offset + 28, flags & 0xFFFFFBFF);
        if (r_s32(object_offset + 144) < 4096)
        {
            if ((r_u32(object_offset + 28) & 0x800000) != 0)
            {
                mode = r_u16(definition) >> 13;
                if (mode - 4 < 2)
                {
                    if (mode == 5)
                        w_u16(object_offset + 128, 0xFFFE);
                    w_u32(object_offset + 144, 4096);
                }
                else
                    w_u16(object_offset + 128, 0);
                w_u32(object_offset + 28, r_u32(object_offset + 28) & 0xFF7FFFFF);
            }
            value = (uint32)((sint32)(3686 * r_u32(object_offset + 144)) >> 12);
            w_u32(object_offset + 144, value);
            for (index = 0; index < 3; ++index)
            {
                value = (uint32)(((sint64)r_s32(object_offset + 92 + index * 4) * r_s32(object_offset + 144)) >> 12);
                w_u32(vector + index * 4, value);
            }
        }
        else
        {
            x = r_u32(object_offset + 92);
            y = r_u32(object_offset + 96);
            z = r_u32(object_offset + 100);
            w_u32(vector, x);
            w_u32(vector + 4, y);
            w_u32(vector + 8, z);
        }
        x = r_u32(vector);
        y = r_u32(vector + 4);
        z = r_u32(vector + 8);
        w_u32(motion, x);
        w_u32(motion + 4, y);
        w_u32(motion + 8, z);
        if ((r_u32(object_offset + 28) & 0x2000) != 0)
        {
            frame = r_s16(object_offset + 128);
            if (frame != 0)
                w_u16(object_offset + 128, frame - 1);
        }
        else
        {
            parent = r_u32(object_offset + 132);
            if (parent != 0 && r_u32(parent + 40) != 0)
            {
                p2_callback_transpose(object_offset, rotation);
                x = r_u32(vector);
                y = r_u32(vector + 4);
                z = r_u32(vector + 8);
                w_u32(input, x);
                w_u32(input + 4, y);
                w_u32(input + 8, z);
                p2_callback_rotate(rotation, input, vector);
            }
        }
        for (index = 0; index < 3; ++index)
            w_u32(object_offset + 16 + index * 4, r_u32(object_offset + 16 + index * 4) + r_u32(vector + index * 4));
        if (r_s32(object_offset + 144) < 4096 && (r_u32(object_offset + 28) & 0x2000) == 0)
        {
            p2_callback_world_position(object_offset, vector, world_input);
            frame = (sint16)p2_func_800231dc(r_u32(object_offset + 132), vector, r_u32(object_offset + 28) & 0x10, -61440);
            w_u16(object_offset + 136, frame);
            if (frame >= 0)
                w_u16(object_offset + 136, p2_func_8003d3fc(r_u32(object_offset + 132), frame, vector, floor, r_u32(object_offset + 28) & 0x10, 4096));
            flags = r_u32(object_offset + 28);
            if ((flags & 0x400) != 0)
            {
                if (r_s32(floor + 4) >= (sint32)(r_u32(vector + 4) - 682))
                    w_u32(object_offset + 28, flags ^ 0x4400);
                else
                    w_u32(floor + 4, r_u32(vector + 4));
            }
            p2_callback_local_position(object_offset, floor, inverse_input, inverse_rotation);
            if (r_s32(speed) < 0)
                p2_reverse_callback_path(object_offset);
        }
        flags = r_u32(object_offset + 28);
        if ((flags & 0x800) != 0)
        {
            mode = r_u16(definition) >> 13;
            w_u32(object_offset + 28, flags & 0xFFFFF7FF);
            switch (mode)
            {
                case 0:
                case 1:
                    if ((r_u8(r_u32(object_offset + 132) + 22) & 2) != 0)
                        break;
                    w_u16(object_offset + 136, 0);
                    w_u32(object_offset + 28, r_u32(object_offset + 28) ^ 0x10);
                    w_u32(speed, 0);
                    if (mode == 1)
                    {
                        w_u32(object_offset + 100, 0);
                        w_u32(object_offset + 92, 0);
                    }
                    else
                    {
                        x = r_u32(object_offset + 92);
                        y = r_u32(object_offset + 96);
                        z = r_u32(object_offset + 100);
                        w_u32(object_offset + 92, 0 - x);
                        w_u32(object_offset + 96, 0 - y);
                        w_u32(object_offset + 100, 0 - z);
                        if ((r_u32(object_offset + 28) & 0x400) != 0)
                        {
                            w_u32(object_offset + 96, 0 - r_u32(object_offset + 96));
                            p2_callback_world_position(object_offset, vector, event_input);
                        }
                        if (p2_callback_vector_length(object_offset + 92) != 0)
                        {
                            x = r_u32(object_offset + 92);
                            y = r_u32(object_offset + 96);
                            z = r_u32(object_offset + 100);
                            w_u32(data + 24, x);
                            w_u32(data + 28, y);
                            w_u32(data + 32, z);
                            w_u32(object_offset + 28, r_u32(object_offset + 28) | 0x400000);
                        }
                    }
                    p2_callback_world_position(object_offset, floor, event_input);
                    x = r_u32(motion) - r_u32(floor);
                    y = r_u32(motion + 4) - r_u32(floor + 4);
                    z = r_u32(motion + 8) - r_u32(floor + 8);
                    w_u32(motion, x);
                    w_u32(motion + 4, y);
                    w_u32(motion + 8, z);
                    parent = r_u32(object_offset + 132);
                    frame = r_s16(object_offset + 136);
                    if ((r_u32(object_offset + 28) & 0x10) == 0)
                        frame = r_s16(parent + 26) - frame - 1;
                    p2_func_80017364(frame, parent + 20, object_offset + 16);
                    p2_callback_world_position(object_offset, floor, sample_input);
                    x = r_u32(motion) + r_u32(floor);
                    y = r_u32(motion + 4) + r_u32(floor + 4);
                    z = r_u32(motion + 8) + r_u32(floor + 8);
                    w_u32(motion, x);
                    w_u32(motion + 4, y);
                    w_u32(motion + 8, z);
                    if ((r_u32(object_offset + 28) & 0x400) != 0)
                    {
                        w_u32(floor + 4, r_u32(vector + 4));
                        p2_callback_local_position(object_offset, floor, final_input, final_rotation);
                    }
                    break;
                case 2:
                    w_u16(object_offset + 128, 0);
                    break;
                case 3:
                    p2_callback_world_position(object_offset, object_offset + 16, world_input);
                    w_u32(object_offset + 28, r_u32(object_offset + 28) | 0x2000);
                    p2_func_800414d0(object_offset);
                    w_u16(object_offset + 128, 150);
                    break;
                case 4:
                case 5:
                    if (r_s16(object_offset + 128) == 0)
                        goto expire;
                    flags = r_u32(object_offset + 28);
                    value = r_u8(object_offset + 13);
                    w_u16(object_offset + 128, 0xFFFF);
                    w_u8(object_offset + 124, 0);
                    result = flags | 0x4000;
                    w_u32(object_offset + 28, result);
                    w_u8(object_offset + 13, value | 1);
                    goto finish;
            }
        }
    }
    if (r_s16(object_offset + 128) > 0)
        p2_func_80042024(object_offset);
    else
    {
    expire:
        if ((p2_func_8005e868(r_u32(data + 8), (uint32)(sint32)r_s16(data + 6)) & 0xFF) != 0)
            p2_func_8006a970(r_u16(data + 6));
        w_u32(data + 8, 0);
        w_u16(data + 6, 0xFFFF);
        frame = r_s16(object_offset + 128);
        value = r_u8(object_offset + 13) | 1;
        w_u32(object_offset + 28, r_u32(object_offset + 28) | 0x4000);
        w_u8(object_offset + 13, value);
        if (frame != -2)
        {
            frame = r_s16(object_offset + 128);
            w_u8(object_offset + 124, 0);
            if (frame != 0)
            {
                parent = r_u32(object_offset + 132);
                w_u16(object_offset + 128, 1);
                w_u16(object_offset + 136, 0);
                p2_func_80017364(0, parent + 20, object_offset + 16);
                result = r_u32(object_offset + 28) | 0x8000;
                value = r_u8(object_offset + 13) & 0xFE;
                w_u32(object_offset + 28, result);
                w_u8(object_offset + 13, value);
            }
            else
            {
                result = 4;
                w_u8(object_offset + 126, 0);
                w_u8(object_offset + 127, 4);
            }
            goto finish;
        }
    }
    if ((r_u32(object_offset + 28) & 0x200000) == 0)
    {
        flags = r_u32(owner + 28);
        w_u8(owner + 125, 0);
        w_u32(owner + 28, flags | 0x20020);
        w_u32(motion + 8, 0);
        w_u32(motion + 4, 0);
        w_u32(motion, 0);
        w_u32(object_offset + 100, 0);
        w_u32(object_offset + 96, 0);
        w_u32(object_offset + 92, 0);
        w_u32(speed + 8, 0);
        w_u32(speed + 4, 0);
        w_u32(speed, 0);
        w_u32(owner + 144, 0);
        flags = r_u32(object_offset + 28);
        w_u16(object_offset + 148, 4096);
        if ((flags & 0x40000) != 0)
        {
            w_u32(speed, r_u32(definition + 4));
            w_u8(object_offset + 124, 3);
        }
        else
            w_u8(object_offset + 124, 0);
        initial_kind = 3;
        w_u32(object_offset + 28, r_u32(object_offset + 28) | 0x200010);
    }
    parent = definition + (r_s8(object_offset + 124) == 3 ? 4 : 0);
    if (r_u32(owner + 104) != r_u32(parent + 36))
    {
        if ((p2_func_8005e868(r_u32(data + 8), (uint32)(sint32)r_s16(data + 6)) & 0xFF) != 0)
            p2_func_8006a970(r_u16(data + 6));
        w_u32(data + 8, 0);
        w_u16(data + 6, 0xFFFF);
        flags = r_u32(owner + 28);
        w_u32(owner + 104, r_u32(parent + 36));
        w_u16(owner + 130, 0xFFFF);
        w_u32(owner + 28, flags | 0x10000);
    }
    if (initial_kind == 3 && (r_u32(object_offset + 28) & 0x2000) == 0)
        p2_func_80042320(definition, object_offset);
    value = r_u8(object_offset + 13);
    result = value | 0x40;
    if ((value & 1) != 0)
        goto finish;
    w_u8(object_offset + 13, result);
    value = r_u32(owner + 104);
    if (value != 0)
    {
        frame = r_s16(owner + 130);
        frame = frame < r_s16(value + 6) - 1 ? frame + 1 : 0;
        w_u16(owner + 130, frame);
        p2_func_8001a6ac(owner);
    }
    if ((r_u32(object_offset + 28) & 0x400) != 0)
        w_u32(motion + 4, 0);
    length = p2_callback_vector_length(motion);
    w_u32(vector, length);
    if (length == 0)
    {
        sound_table = r_u32(definition + 48);
        sound = sound_table != 0 && r_s32(sound_table) > 0 ? sound_table + 4 : 0;
        pitch = r_u16(sound + 4);
        if (r_u32(object_offset + 116) == 4)
            option = 1;
    }
    else
    {
        if ((r_u32(object_offset + 28) & 0x10) != 0)
            w_u32(vector, 0 - length);
        w_u32(owner + 144, r_u32(owner + 144) + r_u32(vector));
        w_u32(owner + 28, r_u32(owner + 28) | 1);
        w_u8(owner + 13, r_u8(owner + 13) | 0x40);
        sound_table = r_u32(definition + 48);
        sound = sound_table != 0 && r_s32(sound_table) >= 2 ? sound_table + 12 : 0;
        if (r_u32(object_offset + 116) - 30 < 2)
            option = 1;
        base = (uint32)r_u16(sound + 4) << 12;
        value = p2_func_8002aa1c(r_u32(motion), 0, r_u32(motion + 8));
        length = r_u32(definition + 8);
        if ((sint32)value < (sint32)length)
        {
            if (length == 0)
            {
                trap(7);
                abort();
            }
            quotient = 0x40000000 / (sint32)length;
            factor = (uint32)(((sint64)(sint32)value * quotient) >> 18);
            w_u32(0x1F80030C, factor);
            base = (uint32)(((sint64)(sint32)base * 3072) >> 12);
            pitch = (sint32)(base + (uint32)(((sint64)(sint32)factor * (sint32)base) >> 12)) >> 12;
            volume = (uint32)((sint32)(factor * 3072) >> 12) + 1024;
        }
        else
            pitch = (sint32)(uint32)(((sint64)(sint32)base * 6144) >> 12) >> 12;
    }
    result = r_u32(owner + 28) & 0x2000000;
    if (result != 0)
    {
        node = r_u32(owner + 8);
        distance = r_u32(owner + 144);
        changed = 0;
        while (node != 0)
        {
            if (r_u8(node + 12) == 6 && r_u8(owner + 12) == 5)
            {
                denominator = (sint32)(uint32)(((sint64)25735 * r_s32(node + 32)) >> 12);
                if (denominator == 0)
                {
                    trap(7);
                    abort();
                }
                quotient = 0x40000000 / denominator;
                factor = (uint32)(((sint64)(sint32)distance * quotient) >> 18);
                frame = (sint32)factor >> 1;
                w_u32(0x1F80030C, factor);
                w_u8(owner + 29, 0xFF);
                w_u16(owner + 40, p2_func_8005dbb8((sint16)(1024 - (uint32)frame)));
                w_u16(owner + 42, p2_func_8005dbb8((sint16)frame));
                changed = 1;
                w_u16(owner + 46, 0);
                w_u16(owner + 44, 0);
            }
            value = p2_func_80041edc(node, distance);
            node = r_u32(node + 4);
            changed |= value;
        }
        result = changed << 24;
        if (result == 0)
        {
            result = r_u32(owner + 28) & 0xFDFFFFFF;
            w_u32(owner + 28, result);
        }
    }
    if (sound != 0)
    {
        previous = r_u32(data + 8);
        if (sound != previous && previous != 0)
        {
            if ((p2_func_8005e868(previous, (uint32)(sint32)r_s16(data + 6)) & 0xFF) != 0)
                p2_func_8006a970(r_u16(data + 6));
            w_u32(data + 8, 0);
            w_u16(data + 6, 0xFFFF);
        }
        w_u32(data + 8, sound);
        result = p2_func_8005eb18(sound, (sint32)(volume + 4096), 0, (uint32)pitch, object_offset + 60, (uint32)r_u16(definition + 44) << 5, (uint32)(sint32)r_s16(data + 6), 1, option);
        w_u16(data + 6, result);
    }
finish:
    p2_guest_stack_pointer = saved_stack;
    return result;
}
