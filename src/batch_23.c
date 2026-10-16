#include "game.h"
#include <stdlib.h>

static uint32 p2_dispatch_base_motion(uint32 object)
{
    uint32 target = r_u32(0x800B7A70);
    if (target == 0x80033564)
        return p2_func_80033564(object);
    fprintf(stderr, "P2: Unmapped base motion callback %08X\n", target);
    abort();
}

uint32 p2_func_800216cc(uint32 object)
{
    uint32 state = r_u32(r_u32(object + 132) + 44), flags, resource, direction, result;
    sint32 maximum, step, speed;
    if ((r_u8(state + 38) & 10) != 0 || (r_u32(object + 28) & 0x40) == 0)
        return p2_dispatch_base_motion(object);
    flags = r_u32(object + 28);
    direction = (flags >> 4) & 1;
    resource = r_u32(state + 72);
    if (r_u32(object + 104) == r_u32(resource + 100))
    {
        maximum = r_s32(resource + 28);
        step = r_s32(resource + 36) >> 2;
    }
    else
    {
        maximum = r_s32(resource + 32);
        step = r_s32(resource + 36);
    }
    speed = r_s32(state + 16);
    if ((r_u32(object + 28) & 0x40000000) != 0)
    {
        speed = (sint32)((uint32)speed - (uint32)step);
        w_u32(state + 16, speed);
        if (speed <= 0)
        {
            w_u32(state + 16, 0);
            flags = r_u32(object + 28) & 0xBFFFFFBF;
            w_u32(object + 28, flags);
            w_u16(object + 136, r_u16(r_u32(object + 132) + 26) - r_u16(object + 136));
            w_u32(object + 28, (r_u32(object + 28) & 0xFFFFF7FF) ^ 0x10);
        }
    }
    else if (speed < maximum)
    {
        speed = (sint32)((uint32)speed + (uint32)step);
        w_u32(state + 16, speed);
        if (speed > maximum)
            w_u32(state + 16, maximum);
    }
    p2_dispatch_base_motion(object);
    flags = r_u32(object + 28);
    result = flags & 0x40;
    if (result != 0)
    {
        state = r_u32(r_u32(object + 132) + 44);
        result = 0x40000000;
        if (r_s16(object + 136) >= r_s16(state + (direction != 0 ? 20 : 22)))
        {
            result = flags | 0x40000000;
            w_u32(object + 28, result);
        }
    }
    return result;
}

uint32 p2_func_8005de30(uint32 object)
{
    uint32 state = r_u32(r_u32(object + 132) + 44), resource = r_u32(state + 72);
    uint32 saved_stack = p2_guest_stack_pointer, result;
    p2_guest_stack_pointer -= 48;
    p2_func_80030dcc(object, p2_guest_stack_pointer, p2_guest_stack_pointer + 16, p2_guest_stack_pointer + 32);
    if ((r_u8(state + 38) & 2) == 0 && (r_u32(r_u32(object + 132) + 16) & 0x40) != 0)
    {
        uint32 facing = p2_func_80040384(object, object + 92, r_s16(resource + 68));
        uint32 timer = r_u32(object + 112);
        if (timer != 0)
            w_u32(object + 112, --timer);
        if (timer == 0 && (facing & 255) != 0 && (r_u32(object + 28) & 0x30000000) == 0 && r_s8(object + 125) != 17)
        {
            uint32 spawned, first, second;
            w_u32(object + 116, 30);
            w_u32(object + 28, r_u32(object + 28) | 0x100);
            spawned = p2_dispatch_object_factory(r_u32(0x800BA398), object);
            w_u32(object + 28, r_u32(object + 28) & 0xFFFFFEFF);
            if (spawned != 0)
            {
                uint32 flags = r_u32(spawned + 28);
                w_u32(spawned + 100, 0);
                w_u32(spawned + 96, 0);
                w_u32(spawned + 92, 0);
                w_u32(spawned + 28, flags & 0xFFFFFBFF);
                first = r_u32(object + 84);
                second = r_u32(object + 88);
                w_u32(spawned + 84, first);
                w_u32(spawned + 88, second);
                w_u8(spawned + 120, -2);
            }
            w_u32(object + 112, (uint32)r_s16(resource + 64));
        }
    }
    result = p2_func_80032248(object, 0, p2_guest_stack_pointer, resource);
    p2_guest_stack_pointer = saved_stack;
    return result;
}

uint32 p2_func_80060360(uint32 object)
{
    uint32 state = r_u32(r_u32(object + 132) + 44), resource = r_u32(state + 72);
    uint32 saved_stack = p2_guest_stack_pointer, origin, target, result;
    sint32 frame, previous, difference, velocity, angle;
    p2_guest_stack_pointer -= 48;
    origin = p2_guest_stack_pointer;
    target = origin + 16;
    p2_func_80030dcc(object, origin, target, origin + 32);
    if ((r_u8(state + 38) & 2) != 0)
    {
        result = p2_func_80032248(object, 0, origin, resource);
        p2_guest_stack_pointer = saved_stack;
        return result;
    }
    w_u32(target + 4, r_u32(target + 4) + 14336);
    if ((r_u32(r_u32(object + 132) + 16) & 0x40) != 0)
    {
        w_u32(object + 28, r_u32(object + 28) | 0x40);
        if (r_s32(origin + 32) < 10001)
            frame = r_s16(object + 136) + p2_func_80034958(object, origin, target);
        else
            frame = r_s16(r_u32(object + 132) + 26);
        previous = r_s16(object + 136);
        if (frame < previous)
        {
            uint32 count = r_u16(r_u32(object + 132) + 26);
            w_u32(object + 28, (r_u32(object + 28) & 0xFFFFF7FF) ^ 0x10);
            w_u16(object + 136, count - previous);
        }
        difference = (sint32)(r_u32(target + 4) - r_u32(origin + 4));
        if ((uint32)difference + 2048 < 4096)
            difference = 0;
        state = r_u32(r_u32(object + 132) + 44);
        velocity = r_s16(state + 30);
        angle = r_s16(object + 152);
        if (difference != 0)
        {
            if ((angle == 0 && difference >= 0) || (angle > 1024 && difference >= 0))
            {
                velocity += 409;
                if (velocity > 2048) velocity = 2048;
            }
            else if (angle > 1024 && difference < 0)
            {
                velocity -= 409;
                if (velocity < 0) velocity = 0;
            }
            else if (difference <= 0)
            {
                velocity -= 409;
                if (velocity < -2048) velocity = -2048;
            }
            else
            {
                velocity += 409;
                if (velocity > 0) velocity = 0;
            }
        }
        else if (velocity > 0)
        {
            velocity -= 409;
            if (velocity < 0) velocity = 0;
        }
        else if (velocity < 0)
        {
            velocity += 409;
            if (velocity > 0) velocity = 0;
        }
        if ((r_u32(object + 28) & 0x4000) != 0 && velocity < 0)
            velocity = 0;
        w_u16(r_u32(r_u32(object + 132) + 44) + 30, velocity);
    }
    else if ((r_u32(object + 28) & 0x40) != 0)
    {
        w_u32(object + 28, r_u32(object + 28) & 0xFFFFFFBF);
        w_u16(r_u32(r_u32(object + 132) + 44) + 30, 0);
    }
    p2_func_80032248(object, target, origin, resource);
    result = r_u32(object + 28) & 0xFFFFBFFF;
    w_u32(object + 28, result);
    p2_guest_stack_pointer = saved_stack;
    return result;
}

static void p2_set_object_animation(uint32 object, uint32 animation)
{
    uint32 flags = r_u32(object + 28);
    w_u32(object + 104, animation);
    w_u16(object + 130, -1);
    w_u32(object + 28, flags | 0x10000);
}

static void p2_reverse_object_path(uint32 object)
{
    uint32 frame = r_u16(object + 136);
    uint32 count = r_u16(r_u32(object + 132) + 26);
    w_u32(object + 28, (r_u32(object + 28) & 0xFFFFF7FF) ^ 0x10);
    w_u16(object + 136, count - frame);
}

uint32 p2_func_8001fa24(uint32 object, uint32 script, uint32 target, sint32 index)
{
    uint32 spawned, definition, matrix, saved_stack, vector, type, magnitude, length;
    sint32 delta, degrees, angle, factor;
    uint32 axis;
    if (r_u8(0x800B9947) >= 5)
        return 0;
    w_u32(object + 116, r_u32(script + 12));
    w_u32(object + 28, r_u32(object + 28) | 0x100);
    spawned = p2_dispatch_object_factory(r_u32(0x800BA398), object);
    w_u32(object + 28, r_u32(object + 28) & 0xFFFFFEFF);
    /* A failed factory must not write a projectile through address zero */
    if (spawned == 0)
        return 0;
    w_u8(spawned + 120, -3);
    definition = r_u32(spawned + 132);
    saved_stack = p2_guest_stack_pointer;
    p2_guest_stack_pointer -= 16;
    vector = p2_guest_stack_pointer;
    if (definition != 0 && r_u32(definition + 40) != 0)
    {
        w_u32(vector, r_u32(spawned + 16));
        w_u32(vector + 4, r_u32(spawned + 20));
        w_u32(vector + 8, r_u32(spawned + 24));
        PushMatrix();
        matrix = r_u32(r_u32(spawned + 132) + 40);
        xport_gte_write_control(0, r_u32(matrix));
        xport_gte_write_control(1, r_u32(matrix + 4));
        xport_gte_write_control(2, r_u32(matrix + 8));
        xport_gte_write_control(3, r_u32(matrix + 12));
        xport_gte_write_control(4, r_u32(matrix + 16));
        p2_sdk_8007ec64(vector, spawned + 16);
        PopMatrix();
        for (axis = 0; axis < 3; ++axis)
        {
            matrix = r_u32(r_u32(spawned + 132) + 40);
            w_u32(spawned + 16 + axis * 4, r_u32(spawned + 16 + axis * 4) + r_u32(matrix + 20 + axis * 4));
        }
    }
    else
    {
        uint32 x = r_u32(spawned + 16), y = r_u32(spawned + 20), z = r_u32(spawned + 24);
        w_u32(spawned + 16, x);
        w_u32(spawned + 20, y);
        w_u32(spawned + 24, z);
    }
    type = r_u32(script + 8);
    if (type == 0 || type == 1 || type == 3)
    {
        uint32 y = r_u32(spawned + 20), x = r_u32(spawned + 16);
        w_u32(spawned + 20, y + 172032);
        w_u32(spawned + 16, x + 61440);
    }
    w_u8(spawned + 124, 0);
    w_u32(spawned + 28, r_u32(spawned + 28) | 0x2000);
    type = r_u32(script + 8);
    if (type < 2)
    {
        w_u32(spawned + 92, 0);
        w_u32(spawned + 96, 24576);
        w_u32(spawned + 100, r_u32(target + 8) - r_u32(spawned + 24));
    }
    else if (type == 3)
    {
        delta = (sint32)(r_u32(target + 8) - r_u32(spawned + 24));
        degrees = r_s32(script + 60 + (uint32)index * 4);
        angle = (sint32)((degrees < 0 ? 0 - (uint32)degrees : (uint32)degrees) << 12) / 360;
        factor = p2_func_8005dbb8((sint16)angle) >> 2;
        magnitude = (uint32)(((sint64)factor * delta) >> 12);
        w_u32(spawned + 92, degrees < 0 ? 0 - magnitude : magnitude);
        w_u32(spawned + 96, 24576);
        factor = p2_func_8005dbb8((sint16)(1024 - angle)) >> 2;
        w_u32(spawned + 100, (uint32)(((sint64)factor * delta) >> 12));
    }
    magnitude = 0;
    for (axis = 0; axis < 3; ++axis)
    {
        sint32 component = r_s32(spawned + 92 + axis * 4);
        magnitude += (uint32)(((sint64)component * component) >> 12);
    }
    length = (sint32)magnitude >= 0 ? p2_sdk_8007e794(magnitude) : p2_sdk_8007e794(magnitude >> 2) << 1;
    length = (uint32)((sint32)length >> 1);
    if (length == 0)
    {
        fprintf(stderr, "P2: Zero projectile normalization length\n");
        abort();
    }
    factor = 0x40000000 / (sint32)length;
    for (axis = 0; axis < 3; ++axis)
    {
        sint32 component = r_s32(spawned + 92 + axis * 4);
        sint32 normalized = (sint32)(((sint64)component * factor) >> 18);
        w_u32(0x1F80030C, normalized);
        delta = (sint32)(((sint64)normalized * 122880) >> 12);
        w_u32(spawned + 16 + axis * 4, r_u32(spawned + 16 + axis * 4) + (uint32)delta);
    }
    p2_guest_stack_pointer = saved_stack;
    magnitude = r_u32(script + 104);
    w_u32(object + 112, magnitude);
    return magnitude;
}

uint32 p2_func_80020094(uint32 object, uint32 origin, uint32 target, sint32 distance, uint32 resource, uint32 script)
{
    uint32 animation, attack, next_animation, flags;
    sint32 frame, current, next;
    if (r_u32(0x8008A0E4) != 0)
    {
        w_u32(object + 28, r_u32(object + 28) & 0xFFFFFFBF);
        p2_set_object_animation(object, r_u32(resource + 72));
        w_u32(0x8008A0E4, 0);
        w_u32(0x8008A0E8, 0);
        w_u32(0x8008A0EC, 0);
        return 0;
    }
    animation = r_u32(object + 104);
    if (animation == r_u32(resource + 72))
    {
        frame = r_s16(object + 130);
        if (animation != 0 && frame < r_s16(animation + 6) - 1 && frame + 1 >= 40 && r_u32(0x8008A0EC) == 0)
        {
            w_u32(0x8008A0EC, 1);
            w_u32(0x8008A240, r_u32(0x8008A2EC) | 0x1000);
        }
        if (r_s16(object + 130) >= r_s16(r_u32(object + 104) + 6) - 1)
        {
            next = r_s32(script + 20 + r_u32(0x8008A0D8) * 4);
            if ((r_u32(object + 28) & 0x10) != 0)
                next = (sint32)((uint32)(sint32)r_s16(r_u32(object + 132) + 26) - (uint32)next);
            w_u32(0x8008A2E0, next);
            w_u32(0x8008A0E8, 1);
            w_u32(0x8008A2E4, (uint32)r_s16(object + 136));
        }
    }
    if (r_u32(0x8008A0E8) != 0)
    {
        animation = r_u32(object + 104);
        attack = r_u32(resource + 76);
        if (animation == attack)
        {
            if (r_s16(object + 130) >= r_s16(attack + 6) - 1)
            {
                w_u32(0x8008A0D8, r_u32(0x8008A0D8) + 1);
                next_animation = r_u32(resource + ((r_u32(object + 28) & 0x10) != 0 ? 88 : 84));
                p2_set_object_animation(object, next_animation);
            }
            next = r_s32(script + 20 + r_u32(0x8008A0D8) * 4);
            if ((r_u32(object + 28) & 0x10) != 0)
                next = (sint32)((uint32)(sint32)r_s16(r_u32(object + 132) + 26) - (uint32)next);
            w_u32(0x8008A2E0, next);
        }
        else
        {
            if (distance < r_s32(script + 4) || r_u32(0x8008A0D4) == r_u32(script + 100))
            {
                w_u16(object + 130, -1);
                w_u32(object + 104, attack);
                current = r_s16(object + 136);
                w_u32(object + 28, (r_u32(object + 28) & 0xFFFEFFBF) | 0x10000);
                w_u32(0x8008A0D4, 0);
                w_u32(0x8008A2E4, current);
                return 0;
            }
            w_u32(0x8008A0D4, r_u32(0x8008A0D4) + 1);
            flags = r_u32(object + 28);
            current = r_s16(object + 136);
            if (((flags & 0x10) != 0 && r_s32(target) < r_s32(origin)) ||
                ((flags & 0x10) == 0 && r_s32(origin) < r_s32(target)))
                next = current + 1;
            else
                next = current - 1;
            w_u32(0x8008A2E0, next);
            current = r_s16(object + 136);
            if (next < current)
            {
                if (current > 0)
                {
                    w_u32(object + 28, ((r_u32(object + 28) & 0xFFFFF7FF) ^ 0x10) | 0x40);
                    w_u16(object + 136, r_u16(r_u32(object + 132) + 26) - current);
                }
                else
                    w_u32(object + 28, r_u32(object + 28) & 0xFFFFFFBF);
            }
            else if (current < r_s16(r_u32(object + 132) + 26))
                w_u32(object + 28, r_u32(object + 28) | 0x40);
            else
                w_u32(object + 28, r_u32(object + 28) & 0xFFFFFFBF);
            next_animation = r_u32(resource + ((r_u32(object + 28) & 0x10) != 0 ? 88 : 84));
            if (r_u32(object + 104) != next_animation)
                p2_set_object_animation(object, next_animation);
        }
        w_u32(0x8008A2E4, (uint32)r_s16(object + 136));
    }
    if (r_s32(0x8008A0D8) < r_s32(script + 16))
        return 0;
    w_u32(0x8008A0D8, 0);
    w_u32(0x8008A0E8, 0);
    w_u32(0x8008A0EC, 0);
    w_u32(0x8008A0E4, 1);
    w_u32(0x8008A240, r_u32(0x8008A2EC) | 0x2000);
    return 1;
}

uint32 p2_func_800204cc(uint32 object, uint32 origin, uint32 target, sint32 distance, uint32 resource, uint32 script)
{
    uint32 animation, next_animation, kind, flags;
    sint32 frame, next_frame, count, index;
    (void)origin;
    if (r_u32(object + 112) != 0 || r_u32(0x8008A0DC) != 0)
        return 1;
    w_u32(resource + 28, (uint32)((sint32)(r_u32(script) << 12) / 30));
    kind = r_u32(script + 8);
    animation = r_u32(object + 104);
    if (kind == 1 && (animation == r_u32(resource + 92) || animation == r_u32(resource + 96)))
    {
        next_frame = animation != 0 && r_s16(object + 130) < r_s16(animation + 6) - 1 ? r_s16(object + 130) + 1 : 0;
        count = r_s32(script + 16);
        for (index = 0; index < count; ++index)
        {
            if (r_s32(script + 20 + (uint32)index * 4) == next_frame)
            {
                p2_func_8001fa24(object, script, target, index);
                return 1;
            }
        }
    }
    else if (kind == 0 || kind == 3)
    {
        if (animation == r_u32(resource + 80) || r_s32(script + 4) < distance || r_u32(0x8008A0D0) == r_u32(script + 100))
        {
            if (animation != r_u32(resource + 80))
            {
                w_u32(object + 28, r_u32(object + 28) & 0xFFFFFFBF);
                p2_set_object_animation(object, r_u32(resource + 80));
            }
            animation = r_u32(object + 104);
            next_frame = animation != 0 && r_s16(object + 130) < r_s16(animation + 6) - 1 ? r_s16(object + 130) + 1 : 0;
            count = r_s32(script + 16);
            for (index = 0; index < count; ++index)
            {
                if (r_s32(script + 20 + (uint32)index * 4) == next_frame)
                {
                    p2_func_8001fa24(object, script, target, index);
                    w_u32(0x8008A0F0, 1);
                    w_u32(0x8008A0D0, 0);
                    return 1;
                }
            }
            animation = r_u32(object + 104);
            frame = r_s16(object + 130);
            if ((animation == 0 || frame >= r_s16(animation + 6) - 1) && frame > 0)
            {
                p2_set_object_animation(object, r_u32(resource + 72));
                if (r_u16(object + 136) == 0 && r_u32(r_u32(object + 132) + 32) != 0)
                    w_u16(object + 136, 1);
            }
            return 1;
        }
    }
    if (r_u32(object + 112) != 0)
        return 1;
    kind = r_u32(script + 8);
    if (kind == 1)
    {
        w_u32(object + 28, r_u32(object + 28) | 0x40);
        count = (sint32)(r_u32(0x8008A0D0) + 1);
        w_u32(0x8008A0D0, count);
        if (count > r_s32(script + 108))
        {
            uint32 random = (uint32)randPSX() & 1;
            w_u32(0x8008A0F0, 0);
            flags = r_u32(object + 28);
            if ((random != 0 && (flags & 0x10) != 0) || (random == 0 && (flags & 0x10) == 0))
                p2_reverse_object_path(object);
            w_u32(0x8008A0D0, 0);
        }
        frame = r_s16(object + 136);
        count = r_s16(r_u32(object + 132) + 26);
        if (frame >= count - 1)
        {
            w_u16(object + 136, count - 1);
            p2_reverse_object_path(object);
        }
        next_animation = r_u32(resource + ((r_u32(object + 28) & 0x10) != 0 ? 96 : 92));
        if (r_u32(object + 104) != next_animation)
            p2_set_object_animation(object, next_animation);
    }
    else if ((kind == 0 || kind == 3) && r_s32(script + 4) >= distance)
    {
        count = (sint32)(r_u32(0x8008A0D0) + 1);
        w_u32(0x8008A0D0, count);
        if (count >= r_s32(script + 100))
            return 1;
        w_u32(object + 28, r_u32(object + 28) | 0x40);
        if (r_u32(0x8008A0F0) != 0)
        {
            uint32 random = (uint32)randPSX() & 1;
            w_u32(0x8008A0F0, 0);
            flags = r_u32(object + 28);
            if ((random != 0 && (flags & 0x10) != 0) || (random == 0 && (flags & 0x10) == 0))
                p2_reverse_object_path(object);
        }
        count = r_s16(r_u32(object + 132) + 26);
        if (r_s16(object + 136) >= count - 1)
        {
            w_u16(object + 136, count - 1);
            p2_reverse_object_path(object);
        }
        if (r_s16(object + 136) == 0)
            p2_reverse_object_path(object);
        next_animation = r_u32(resource + ((r_u32(object + 28) & 0x10) != 0 ? 88 : 84));
        if (r_u32(object + 104) != next_animation)
            p2_set_object_animation(object, next_animation);
    }
    return 1;
}

uint32 p2_func_8001f6dc(uint32 object)
{
    uint32 resource = r_u32(r_u32(r_u32(object + 132) + 44) + 72);
    uint32 saved_stack = p2_guest_stack_pointer, origin, target, distance, script, handler, result, animation;
    sint32 delta, kind, step;
    p2_guest_stack_pointer -= 48;
    origin = p2_guest_stack_pointer;
    target = origin + 16;
    p2_func_80030dcc(object, origin, target, origin + 32);
    delta = (sint32)(r_u32(origin) - r_u32(target)) >> 12;
    distance = (uint32)delta * (uint32)delta;
    w_u32(origin + 32, distance);
    if ((r_u32(r_u32(object + 132) + 16) & 0x40) != 0 && r_u32(0x8008A0E0) == 0)
    {
        if (r_u32(object + 112) != 0)
            w_u32(object + 112, r_u32(object + 112) - 1);
        if (r_u32(0x8008A0DC) != 0)
            w_u32(0x8008A0DC, r_u32(0x8008A0DC) - 1);
        script = 0x8008A578 + r_u32(0x8008A0CC) * 116;
        handler = r_u32(0x80083B9C + r_u32(script + 8) * 4);
        if (handler == 0x800204CC)
            result = p2_func_800204cc(object, origin, target, (sint32)distance, resource, script);
        else if (handler == 0x80020094)
            result = p2_func_80020094(object, origin, target, (sint32)distance, resource, script);
        else
        {
            fprintf(stderr, "P2: Unmapped scripted behavior callback %08X\n", handler);
            abort();
        }
        if (result == 1)
        {
            step = r_s32(0x8008A0CC);
            while (r_s16(object + 128) < r_s32(0x8008A5E8 + (uint32)step * 116))
            {
                if (step >= r_s32(0x8008A2E8) - 1)
                    break;
                w_u32(0x8008A0CC, (uint32)step + 1);
                p2_func_8001fa08();
                step = r_s32(0x8008A0CC);
                kind = r_s32(0x8008A580 + (uint32)step * 116);
                w_u32(0x8008A0DC, 30);
                if (kind == 2)
                    break;
            }
        }
    }
    else
    {
        w_u32(object + 28, r_u32(object + 28) & 0xFFFFFFBF);
        if (r_u32(object + 104) != r_u32(resource + 84))
            p2_set_object_animation(object, r_u32(resource + 84));
        w_u32(0x8008A0E0, 0);
    }
    script = r_u32(0x8008A0CC) * 116;
    kind = r_s32(0x8008A580 + script);
    if (kind >= 0 && kind < 2)
        w_u32(target, r_u32(origin));
    else if (kind == 2 && r_u32(object + 104) == r_u32(resource + 76))
    {
        sint32 marker = r_s32(0x8008A58C + script + r_u32(0x8008A0D8) * 4);
        if (r_s16(object + 130) == marker)
            w_u32(0x8008A2DC, r_u32(target));
        else if (marker < r_s16(object + 130))
            w_u32(target, r_u32(0x8008A2DC));
    }
    animation = r_u32(object + 104);
    if (animation == r_u32(resource + 72))
        w_u32(target, r_u32(origin));
    result = p2_func_80032248(object, target, origin, resource);
    p2_guest_stack_pointer = saved_stack;
    return result;
}

uint32 p2_func_80021254(uint32 object, uint32 origin, uint32 target, sint32 distance, uint32 resource, uint32 script)
{
    sint32 timer = r_s32(0x8008A10C), frame, next;
    uint32 animation, flags;
    w_u32(0x8008A10C, (uint32)timer + 1);
    if (timer >= r_s32(script + 20))
    {
        w_u32(0x8008A10C, 0);
        return 1;
    }
    flags = r_u32(object + 28) | 0x40;
    w_u32(object + 28, flags);
    if (distance < r_s32(script + 16))
    {
        w_u32(object + 28, flags & 0xFFFFFFBF);
        p2_set_object_animation(object, r_u32(resource + 72));
        return 0;
    }
    frame = r_s16(object + 136);
    next = (((flags & 0x10) != 0 && r_s32(target) < r_s32(origin)) ||
        ((flags & 0x10) == 0 && r_s32(origin) < r_s32(target))) ? frame + 1 : frame - 1;
    if (frame > 0 && next < frame)
        p2_reverse_object_path(object);
    animation = r_u32(resource + ((r_u32(object + 28) & 0x10) != 0 ? 104 : 100));
    if (r_u32(object + 104) != animation)
        p2_set_object_animation(object, animation);
    return 0;
}

uint32 p2_func_800213d8(uint32 object, uint32 resource, uint32 script)
{
    uint32 animation, selected;
    if (r_u32(0x8008A110) != 0)
    {
        w_u32(object + 28, r_u32(object + 28) & 0xFFFFFFBF);
        p2_set_object_animation(object, r_u32(resource + 116));
        w_u32(0x8008A110, 0);
        w_u32(0x8008A240, r_u32(0x8008A2FC) | 0x1000);
        return 0;
    }
    animation = r_u32(object + 104);
    selected = r_u32(resource + 72 + r_u32(0x8008AE90 + r_u32(0x8008A0F4) * 28) * 4);
    if (animation == selected)
    {
        if (r_s16(object + 130) >= r_s16(animation + 6) - 1)
            w_u32(0x8008A100, r_u32(0x8008A100) + 1);
    }
    else if (r_s16(object + 130) == r_s16(animation + 6) - 1)
        p2_set_object_animation(object, selected);
    if (r_s32(0x8008A100) < r_s32(script + 24))
        return 0;
    w_u32(object + 28, r_u32(object + 28) & 0xFFFFFFBF);
    selected = r_u32(resource + 112);
    w_u32(0x8008A100, 0);
    w_u32(0x8008A110, 1);
    w_u16(object + 130, -1);
    w_u32(0x8008A240, r_u32(0x8008A2FC) | 0x2000);
    w_u32(object + 104, selected);
    w_u32(object + 28, r_u32(object + 28) | 0x10000);
    return 1;
}

uint32 p2_func_80020f44(uint32 object, uint32 origin, uint32 target, sint32 distance, uint32 resource, uint32 script)
{
    uint32 animation, attack, flags, next_animation;
    sint32 frame, next;
    if (r_u32(script + 8) == 6 && r_u32(0x8008A108) != 0)
    {
        w_u32(object + 28, r_u32(object + 28) & 0xFFFFFFBF);
        p2_set_object_animation(object, r_u32(resource + 92));
        w_u32(0x8008A108, 0);
        w_u32(0x8008A240, r_u32(0x8008A300) | 0x1000);
        return 0;
    }
    animation = r_u32(object + 104);
    attack = r_u32(resource + 72 + r_u32(script + 8) * 4);
    if (animation == attack)
    {
        if (r_s16(object + 130) >= r_s16(animation + 6) - 1)
        {
            w_u32(0x8008A100, r_u32(0x8008A100) + 1);
            next_animation = r_u32(resource + ((r_u32(object + 28) & 0x10) != 0 ? 104 : 100));
            if (r_u32(object + 104) != next_animation)
                p2_set_object_animation(object, next_animation);
        }
    }
    else
    {
        if ((animation == r_u32(resource + 92) || animation == r_u32(resource + 112)) &&
            r_s16(object + 130) < r_s16(animation + 6) - 1)
            return 0;
        if (distance < r_s32(script + 16) || r_u32(0x8008A0FC) == r_u32(script + 20))
        {
            w_u16(object + 130, -1);
            w_u32(object + 104, attack);
            w_u32(object + 28, (r_u32(object + 28) & 0xFFFEFFBF) | 0x10000);
            w_u32(0x8008A0FC, 0);
            return 0;
        }
        w_u32(0x8008A0FC, r_u32(0x8008A0FC) + 1);
        flags = r_u32(object + 28) | 0x40;
        w_u32(object + 28, flags);
        frame = r_s16(object + 136);
        next = (((flags & 0x10) != 0 && r_s32(target) < r_s32(origin)) ||
            ((flags & 0x10) == 0 && r_s32(origin) < r_s32(target))) ? frame + 1 : frame - 1;
        w_u32(0x8008A2F4, next);
        if (r_s16(object + 136) > 0 && next < r_s16(object + 136))
            p2_reverse_object_path(object);
        next_animation = r_u32(resource + ((r_u32(object + 28) & 0x10) != 0 ? 104 : 100));
        if (r_u32(object + 104) != next_animation)
            p2_set_object_animation(object, next_animation);
    }
    if (r_s32(0x8008A100) < r_s32(script + 24))
        return 0;
    w_u32(0x8008A100, 0);
    if (r_u32(script + 8) == 6)
    {
        w_u32(0x8008A108, 1);
        w_u32(object + 28, r_u32(object + 28) & 0xFFFFFFBF);
        p2_set_object_animation(object, r_u32(resource + 92));
        w_u32(0x8008A240, r_u32(0x8008A300) | 0x2000);
    }
    return 1;
}

uint32 p2_func_80020c6c(uint32 object)
{
    uint32 resource = r_u32(r_u32(r_u32(object + 132) + 44) + 72);
    uint32 saved_stack = p2_guest_stack_pointer, origin, target, script, result;
    sint32 delta, state;
    if ((r_u32(object + 28) & 0x10) == 0 && r_s16(object + 128) > 0)
    {
        uint32 x = r_u32(object + 92), y = r_u32(object + 96), z = r_u32(object + 100);
        w_u32(object + 92, 0 - x);
        w_u32(object + 96, 0 - y);
        w_u32(object + 100, 0 - z);
    }
    p2_guest_stack_pointer -= 48;
    origin = p2_guest_stack_pointer;
    target = origin + 16;
    p2_func_80030dcc(object, origin, target, origin + 32);
    if (r_s16(object + 128) <= 0)
    {
        w_u16(object + 130, 40);
        w_u32(object + 100, 0);
        w_u32(object + 92, -4096);
        w_u32(object + 104, r_u32(resource + 108));
        w_u32(object + 28, (r_u32(object + 28) | 0x10) & 0xFFF7FFFF);
    }
    else
    {
        delta = (sint32)(r_u32(origin) - r_u32(target)) >> 12;
        w_u32(origin + 32, (uint32)delta * (uint32)delta);
        if (r_u32(object + 112) != 0)
            w_u32(object + 112, r_u32(object + 112) - 1);
        script = 0x8008AE88 + r_u32(0x8008A0F4) * 28;
        state = r_s32(script);
        if (state == 0)
            result = p2_func_80020f44(object, origin, target, r_s32(origin + 32), resource, script);
        else if (state == 1)
            result = p2_func_800213d8(object, resource, script);
        else
            result = p2_func_80021254(object, origin, target, r_s32(origin + 32), resource, script);
        if (result == 1)
        {
            state = r_s32(0x8008A0F4);
            w_u32(0x8008A0F4, state < r_s32(0x8008A2F8) - 1 ? (uint32)state + 1 : 0);
        }
        script = 0x8008AE88 + r_u32(0x8008A0F4) * 28;
        if (r_u32(script) != 0)
            w_u32(target, r_u32(origin));
        else if (r_u32(object + 104) == r_u32(resource + 72 + r_u32(script + 8) * 4))
        {
            if (r_s16(object + 130) == r_s32(script + 12))
                w_u32(0x8008A2F0, r_u32(target));
            else if (r_s32(script + 12) < r_s16(object + 130))
                w_u32(target, r_u32(0x8008A2F0));
        }
    }
    w_u32(target, r_u32(origin));
    result = p2_func_80032248(object, target, origin, resource);
    p2_guest_stack_pointer = saved_stack;
    return result;
}

uint32 p2_func_8005f154(uint32 object)
{
    uint32 saved_stack = p2_guest_stack_pointer, origin, target, resource, state, definition, event, animation, result;
    sint32 frame, direction;
    p2_guest_stack_pointer -= 48;
    origin = p2_guest_stack_pointer + 16;
    target = p2_guest_stack_pointer;
    event = p2_func_80030dcc(object, origin, target, p2_guest_stack_pointer + 32);
    definition = r_u32(object + 132);
    state = r_u32(definition + 44);
    resource = r_u32(state + 72);
    if ((r_u8(state + 38) & 2) != 0)
        target = 0;
    else
    {
        if ((r_u32(definition + 16) & 0x40) == 0)
        {
            w_u32(object + 28, r_u32(object + 28) & 0xFFFFFFBF);
            if (r_u32(object + 104) != r_u32(resource + 72))
                p2_set_object_animation(object, r_u32(resource + 72));
        }
        else
        {
            if ((event & 0x400000) != 0 && r_u32(object + 104) != r_u32(resource + 80))
            {
                w_u16(object + 130, -1);
                w_u32(object + 104, r_u32(resource + 80));
                w_u32(object + 28, (r_u32(object + 28) | 0x10000) & 0xFFFFFFBF);
            }
            else if (r_u32(object + 104) == r_u32(resource + 80))
            {
                animation = r_u32(object + 104);
                frame = r_s16(object + 130);
                if ((animation == 0 || frame >= r_s16(animation + 6) - 1) && frame > 0)
                {
                    animation = r_u32(resource + ((p2_func_80068198() & 1) != 0 ? 72 : 84));
                    p2_set_object_animation(object, animation);
                    w_u32(object + 112, 1);
                }
            }
            else
            {
                uint32 timer = r_u32(object + 112) - 1;
                w_u32(object + 112, timer);
                if (timer == 0)
                {
                    w_u32(object + 112, p2_func_80068198() % 75 + 30);
                    if (r_u32(object + 104) == r_u32(resource + 84))
                    {
                        p2_set_object_animation(object, r_u32(resource + 72));
                        w_u32(object + 28, r_u32(object + 28) & 0xFFFFFFBF);
                    }
                    else
                    {
                        p2_set_object_animation(object, r_u32(resource + 84));
                        w_u32(object + 28, r_u32(object + 28) | 0x40);
                    }
                }
                if (r_s32(resource + 4) >= r_s32(p2_guest_stack_pointer + 32))
                {
                    direction = p2_func_80034958(object, origin, target);
                    if (direction < 0)
                        p2_reverse_object_path(object);
                    else if (direction == 0 && r_u32(object + 104) == r_u32(resource + 84))
                    {
                        w_u32(object + 28, r_u32(object + 28) & 0xFFFFFFBF);
                        p2_set_object_animation(object, r_u32(resource + 72));
                    }
                }
            }
        }
        if (r_s32(resource + 4) < r_s32(p2_guest_stack_pointer + 32))
            target = 0;
    }
    result = p2_func_80032248(object, target, origin, resource);
    p2_guest_stack_pointer = saved_stack;
    return result;
}

uint32 p2_func_8005f510(uint32 object)
{
    uint32 state = r_u32(r_u32(object + 132) + 44), resource = r_u32(state + 72);
    uint32 saved_stack = p2_guest_stack_pointer, origin, target, selected, result, sum, axis;
    sint32 frame, current, next, difference, velocity;
    p2_guest_stack_pointer -= 80;
    origin = p2_guest_stack_pointer;
    target = origin + 16;
    selected = origin + 32;
    p2_func_80030dcc(object, origin, target, origin + 48);
    if (r_s8(state + 124) >= 0 || r_s8(state + 180) != 0)
    {
        result = p2_func_80032248(object, target, origin, resource);
        p2_guest_stack_pointer = saved_stack;
        return result;
    }
    if ((r_u32(r_u32(object + 132) + 16) & 0x40) == 0)
    {
        uint32 packed = r_u32(object + 112);
        if ((packed & 0x8000) != 0)
        {
            w_u32(selected, (uint32)((sint32)packed >> 16) << 12);
            w_u32(selected + 4, (uint32)(sint32)r_s16(state + 20) << 12);
            w_u32(selected + 8, (uint32)(sint32)r_s16(state + 22) << 12);
            frame = packed & 0x7FFF;
        }
        else
        {
            for (axis = 0; axis < 3; ++axis)
                w_u32(selected + axis * 4, r_u32(target + axis * 4));
            frame = (sint32)p2_func_800231dc(r_u32(object + 132), selected, 1, 4096000);
            packed = ((uint32)((r_s32(selected) >> 12) - 3 + (sint32)(p2_func_80068198() & 7)) << 16) | (uint32)frame | 0x8000;
            w_u32(object + 112, packed);
            w_u16(state + 20, (r_s32(selected + 4) >> 12) + (p2_func_80068198() & 7));
            w_u16(state + 22, (r_s32(selected + 8) >> 12) + (p2_func_80068198() & 7) - 3);
        }
        if ((r_u32(object + 28) & 0x10) == 0)
            frame = r_s16(r_u32(object + 132) + 26) - frame - 1;
        sum = 0;
        for (axis = 0; axis < 3; ++axis)
        {
            difference = (sint32)(r_u32(origin + axis * 4) - r_u32(selected + axis * 4)) >> 12;
            sum += (uint32)difference * (uint32)difference;
        }
        if (r_s32(resource + 4) < (sint32)sum)
            goto stop_motion;
        current = r_s16(object + 136);
        if (frame < current)
        {
            if (current < r_s16(r_u32(object + 132) + 26) - 1)
                w_u32(object + 28, r_u32(object + 28) | 0x40);
            else
            {
                w_u16(object + 136, 0);
                w_u32(object + 28, (r_u32(object + 28) ^ 0x10) & 0xFFFFFFBF);
            }
        }
        else if (current == 0)
            w_u32(object + 28, r_u32(object + 28) & 0xFFFFFFBF);
        else
        {
            w_u32(object + 28, ((r_u32(object + 28) & 0xFFFFF7FF) ^ 0x10) | 0x40);
            w_u16(object + 136, r_u16(r_u32(object + 132) + 26) - current);
        }
        w_u32(object + 28, r_u32(object + 28) | 0x8000000);
        if (r_u32(object + 104) != r_u32(resource + 84))
            p2_set_object_animation(object, r_u32(resource + 84));
        goto finish_motion;
    }
    if ((r_u32(object + 112) & 0x8000) != 0)
    {
        w_u32(object + 112, ((p2_func_80068198() & 7) - 3) << 16);
        w_u16(state + 20, p2_func_80068198() & 3);
        p2_func_80068198();
        w_u16(state + 22, 0);
        w_u32(selected, (uint32)(sint32)r_s16(object + 114) << 12);
        w_u32(selected + 4, (uint32)(sint32)r_s16(state + 20) << 12);
        w_u32(selected + 8, (uint32)(sint32)r_s16(state + 22) << 12);
        for (axis = 0; axis < 3; ++axis)
            w_u32(target + axis * 4, r_u32(target + axis * 4) + r_u32(selected + axis * 4));
    }
    if (r_s32(origin + 48) >= 10000)
        goto stop_motion;
    next = p2_func_80034958(object, origin, target);
    w_u32(object + 28, (r_u32(object + 28) & 0xF7FFFFFF) | 0x40);
    current = r_s16(object + 136);
    next += current;
    if (r_u32(object + 104) != r_u32(resource + 80))
        p2_set_object_animation(object, r_u32(resource + 80));
    current = r_s16(object + 136);
    if (next < current)
    {
        if (current == 0)
            w_u32(object + 28, r_u32(object + 28) & 0xFFFFFFBF);
        else if ((r_u32(state + 44) & 0x80) == 0)
        {
            w_u32(object + 28, ((r_u32(object + 28) & 0xFFFFF7FF) ^ 0x10) | 0x40);
            w_u16(object + 136, r_u16(r_u32(object + 132) + 26) - current);
        }
    }
    else if (current < r_s16(r_u32(object + 132) + 26) - 1)
        w_u32(object + 28, r_u32(object + 28) | 0x40);
    else
    {
        w_u16(object + 136, 0);
        w_u32(object + 28, (r_u32(object + 28) ^ 0x10) & 0xFFFFFFBF);
    }
    difference = (sint32)(r_u32(target + 4) - r_u32(origin + 4));
    velocity = difference;
    if (difference != 0)
    {
        velocity = r_s16(r_u32(r_u32(object + 132) + 44) + 30) + (difference < 0 ? -204 : 204);
        if (velocity < -4096) velocity = -4096;
        if (velocity > 4096) velocity = 4096;
    }
    if ((r_u32(object + 28) & 0x4000) != 0 && velocity < 0)
        velocity = 0;
    w_u16(r_u32(r_u32(object + 132) + 44) + 30, velocity);
    goto finish_motion;
stop_motion:
    w_u32(object + 28, r_u32(object + 28) & 0xFFFFFFBF);
finish_motion:
    p2_func_80032248(object, target, origin, resource);
    w_u32(object + 28, r_u32(object + 28) & 0xFFFFBFFF);
    result = (r_u16(r_u32(0x8008A114) + 42) + 2048) & 4095;
    w_u16(object + 88, result);
    p2_guest_stack_pointer = saved_stack;
    return result;
}

uint32 p2_func_8005e010(uint32 object)
{
    uint32 saved_stack = p2_guest_stack_pointer, origin, target, state, resource, animation, timer, result;
    sint32 frame;
    p2_guest_stack_pointer -= 64;
    origin = p2_guest_stack_pointer;
    target = origin + 16;
    p2_func_80030dcc(object, origin, target, origin + 32);
    state = r_u32(r_u32(object + 132) + 44);
    resource = r_u32(state + 72);
    if ((r_u8(state + 38) & 2) != 0)
    {
        target = 0;
        goto finish;
    }
    if ((r_u8(state + 38) & 8) != 0)
        goto finish;
    if ((r_u32(state + 44) & 8) != 0)
        w_u32(object + 112, 30);
    if ((r_u32(r_u32(object + 132) + 16) & 0x40) == 0)
    {
        if (r_u32(object + 104) != r_u32(resource + 72))
        {
            w_u32(object + 28, r_u32(object + 28) & 0xFBFFFFFF);
            p2_set_object_animation(object, r_u32(resource + 72));
        }
        goto finish;
    }
    timer = r_u32(object + 112);
    if (timer != 0)
    {
        w_u32(object + 112, --timer);
        if (timer != 0)
        {
            w_u32(origin + 32, 0x7FFFF000);
            if (r_u32(object + 104) != r_u32(resource + 80))
            {
                uint32 count;
                w_u32(object + 28, (r_u32(object + 28) & 0xF7FFFFFF) | 0x40);
                p2_set_object_animation(object, r_u32(resource + 80));
                count = r_u16(r_u32(object + 132) + 26);
                if (((sint32)(count << 16) >> 17) < r_s16(object + 136))
                    p2_reverse_object_path(object);
            }
        }
    }
    animation = r_u32(object + 104);
    frame = r_s16(object + 130);
    if (animation == r_u32(resource + 84) &&
        ((animation != 0 && frame < r_s16(animation + 6) - 1) || frame < 0))
    {
        target = 0;
        goto finish;
    }
    w_u32(object + 28, r_u32(object + 28) & 0xFBFFFFFF);
    if (r_s32(resource + 4) >= r_s32(origin + 32) && r_u32(target + 4) - r_u32(object + 64) + 12288 < 24577)
    {
        sint16 angle = 64;
        if (r_s32(resource + 8) >= r_s32(origin + 32))
            angle = (sint16)(r_u16(object + 148) - p2_func_8005dc00(
                (sint32)(r_u32(origin + 8) - r_u32(target + 8)), (sint32)(r_u32(origin) - r_u32(target))));
        if (angle >= -63 && angle < 64)
        {
            w_u32(object + 28, r_u32(object + 28) | 0x4000000);
            if (r_u32(object + 104) != r_u32(resource + 84))
            {
                w_u32(object + 28, r_u32(object + 28) & 0xFFFFFFBF);
                p2_set_object_animation(object, r_u32(resource + 84));
            }
        }
        else if (r_u32(object + 104) != r_u32(resource + 80))
        {
            w_u32(object + 28, r_u32(object + 28) | 0x8000040);
            p2_set_object_animation(object, r_u32(resource + 80));
        }
        if (p2_func_80034958(object, origin, target) < 0)
            p2_reverse_object_path(object);
    }
    else
    {
        uint32 definition = r_u32(object + 132);
        uint32 limit = r_u32(definition + 44) + ((r_u32(object + 28) & 0x10) != 0 ? 20 : 22);
        if (r_s16(object + 136) >= r_s16(limit))
        {
            uint32 flags = r_u32(object + 28);
            w_u16(object + 136, r_u16(definition + 26) - r_s16(object + 136));
            w_u32(object + 28, (flags & 0xFFFFF7FF) ^ 0x10);
        }
        if (r_u32(object + 112) == 0 && r_u32(object + 104) != r_u32(resource + 72) && (r_u8(state + 38) & 8) == 0)
        {
            w_u32(object + 28, r_u32(object + 28) | 0x40);
            p2_set_object_animation(object, r_u32(resource + 72));
        }
    }
finish:
    result = p2_func_80032248(object, target, origin, resource);
    p2_guest_stack_pointer = saved_stack;
    return result;
}

static void p2_transform_object_point(uint32 object, uint32 source, uint32 destination, uint32 temporary)
{
    uint32 definition = r_u32(object + 132), matrix, axis;
    uint32 first = r_u32(source), second = r_u32(source + 4), third = r_u32(source + 8);
    if (definition != 0 && r_u32(definition + 40) != 0)
    {
        w_u32(temporary, first);
        w_u32(temporary + 4, second);
        w_u32(temporary + 8, third);
        PushMatrix();
        matrix = r_u32(r_u32(object + 132) + 40);
        for (axis = 0; axis < 5; ++axis)
            xport_gte_write_control(axis, r_u32(matrix + axis * 4));
        p2_sdk_8007ec64(temporary, destination);
        PopMatrix();
        for (axis = 0; axis < 3; ++axis)
        {
            matrix = r_u32(r_u32(object + 132) + 40);
            w_u32(destination + axis * 4, r_u32(destination + axis * 4) + r_u32(matrix + 20 + axis * 4));
        }
    }
    else
    {
        w_u32(destination, first);
        w_u32(destination + 4, second);
        w_u32(destination + 8, third);
    }
}

uint32 p2_func_800224ac(uint32 object)
{
    uint32 timer = r_u32(object + 112) - 1, state, definition, resource, result, animation;
    uint32 saved_stack, position, temporary, axis;
    sint32 previous, current;
    w_u32(object + 112, timer);
    if (timer != 0xFFFFFFFF || r_s16(object + 128) == 0)
        return p2_dispatch_base_motion(object);
    definition = r_u32(object + 132);
    state = r_u32(definition + 44);
    resource = r_u32(state + 72);
    w_u32(object + 112, 0);
    w_u32(definition + 16, r_u32(definition + 16) | 0x40);
    previous = p2_func_80022388(object);
    p2_dispatch_base_motion(object);
    result = r_u8(state + 38) & 2;
    if (result != 0)
        return result;
    current = p2_func_80022388(object);
    if (current == previous && r_s16(object + 136) < r_s16(r_u32(object + 132) + 26) - 1)
        animation = r_u32(resource + 80);
    else
    {
        if (current != previous)
        {
            definition = r_u32(object + 132);
            if ((r_u32(object + 28) & 0x10) != 0)
            {
                ++previous;
                if (previous == r_u8(definition + 20)) previous = 0;
            }
            else
            {
                if (previous < 0) previous = r_u8(definition + 20) - 1;
                w_u16(object + 136, r_u16(definition + 26) - (r_u32(r_u32(definition + 32) + (uint32)previous * 40) >> 20) - 1);
            }
            saved_stack = p2_guest_stack_pointer;
            p2_guest_stack_pointer -= 48;
            position = p2_guest_stack_pointer;
            temporary = position + 16;
            p2_transform_object_point(object, object + 16, position, temporary);
            definition = r_u32(object + 132);
            p2_transform_object_point(object, r_u32(definition + 32) + (uint32)previous * 40 + 4, object + 92, temporary + 16);
            for (axis = 0; axis < 3; ++axis)
                w_u32(object + 92 + axis * 4, r_u32(object + 92 + axis * 4) - r_u32(position + axis * 4));
            p2_guest_stack_pointer = saved_stack;
        }
        if (r_s16(object + 136) < r_s16(r_u32(object + 132) + 26) - 1)
            w_u32(object + 28, r_u32(object + 28) & 0xFFFFF7FF);
        else
            w_u32(object + 28, r_u32(object + 28) | 0x800);
        w_u32(object + 112, 15);
        definition = r_u32(object + 132);
        w_u32(definition + 16, r_u32(definition + 16) & 0xFFFFFFBF);
        animation = r_u32(resource + 72);
    }
    result = r_u32(object + 104);
    if (result != animation)
    {
        p2_set_object_animation(object, animation);
        result = r_u32(object + 28);
    }
    return result;
}

uint32 p2_func_80021888(uint32 object)
{
    uint32 state = r_u32(r_u32(object + 132) + 44), resource = r_u32(state + 72);
    uint32 saved_stack = p2_guest_stack_pointer, origin, target, result, event, animation, flags;
    sint32 close, frame, projected;
    p2_guest_stack_pointer -= 64;
    origin = p2_guest_stack_pointer + 16;
    target = p2_guest_stack_pointer;
    event = p2_func_80030dcc(object, origin, target, p2_guest_stack_pointer + 32);
    if ((r_u8(state + 38) & 2) != 0)
    {
        target = 0;
        goto finish;
    }
    if ((r_u8(state + 38) & 8) != 0)
        goto finish;
    close = r_s32(resource + 4) >= r_s32(p2_guest_stack_pointer + 32) &&
        ((event & 0x1800000) != 0 || (r_u32(state + 44) & 2) != 0);
    if ((r_u32(r_u32(object + 132) + 16) & 0x40) == 0)
    {
        if ((r_u32(object + 28) & 0x40) != 0)
            w_u32(object + 28, r_u32(object + 28) & 0xFFFFFFBF);
        if (r_u32(object + 104) != r_u32(resource + 72))
            p2_set_object_animation(object, r_u32(resource + 72));
        goto finish;
    }
    animation = r_u32(object + 104);
    if (close && animation != r_u32(resource + 84) && animation != r_u32(resource + 72) && animation != r_u32(resource + 104))
        goto start_chase;
    if (animation == r_u32(resource + 72))
    {
        if (close)
            p2_set_object_animation(object, r_u32(resource + 84));
        else
        {
            frame = animation != 0 && r_s16(object + 130) < r_s16(animation + 6) - 1 ? r_s16(object + 130) + 1 : 0;
            w_u16(object + 130, frame);
        }
        target = origin;
        goto finish;
    }
    if (animation == r_u32(resource + 80))
    {
        uint32 timer = r_u32(object + 112) - 1;
        w_u32(object + 112, timer);
        if (timer == 0)
        {
            if ((r_u32(object + 28) & 0x40) == 0)
                w_u32(object + 28, r_u32(object + 28) | 0x40);
            w_u32(object + 28, r_u32(object + 28) | 0x40000000);
            p2_set_object_animation(object, r_u32(resource + 100));
            goto finish;
        }
        goto chase;
    }
    if (animation == r_u32(resource + 100))
    {
        if (r_u32(state + 16) == 0)
            goto stop_chase;
        if ((r_u32(object + 28) & 0x40) == 0)
            w_u32(object + 28, r_u32(object + 28) | 0x40);
        goto finish;
    }
    if (animation == r_u32(resource + 104))
    {
        frame = r_s16(object + 130);
        if ((animation == 0 || frame >= r_s16(animation + 6) - 1) && frame > 0)
            p2_set_object_animation(object, r_u32(resource + 72));
        else
        {
            animation = r_u32(object + 104);
            frame = animation != 0 && r_s16(object + 130) < r_s16(animation + 6) - 1 ? r_s16(object + 130) + 1 : 0;
            w_u16(object + 130, frame);
        }
        target = origin;
        goto finish;
    }
    if (animation == r_u32(resource + 84))
    {
        frame = r_s16(object + 130);
        if ((animation == 0 || frame >= r_s16(animation + 6) - 1) && frame > 0)
            goto start_chase;
    }
    goto finish;
start_chase:
    w_u32(object + 112, 240);
    if (r_u32(object + 104) != r_u32(resource + 80))
        p2_set_object_animation(object, r_u32(resource + 80));
chase:
    if (r_s32(p2_guest_stack_pointer + 32) < 10001)
        projected = (sint32)p2_func_800231dc(r_u32(object + 132), target, r_u32(object + 28) & 0x10, 409600);
    else
        projected = r_s16(r_u32(object + 132) + 26);
    frame = r_s16(object + 136);
    flags = r_u32(object + 28);
    if (projected >= frame)
        w_u32(object + 28, flags & 0xBFFFFFFF);
    else if ((flags & 0x40) != 0)
        w_u32(object + 28, flags | 0x40000000);
    else
    {
        w_u32(object + 28, ((flags & 0xFFFFF7FF) ^ 0x10) & 0xBFFFFFFF);
        w_u16(object + 136, r_u16(r_u32(object + 132) + 26) - frame);
    }
    flags = r_u32(object + 28);
    frame = r_s16(object + 136);
    if ((flags & 0x40000000) == 0)
    {
        uint32 limit;
        if (frame == 0 && projected == 0)
            goto stop_chase;
        limit = r_u32(r_u32(object + 132) + 44) + ((flags & 0x10) != 0 ? 20 : 22);
        if (frame >= r_s16(limit) && projected >= frame)
            goto stop_chase;
    }
    w_u32(object + 28, r_u32(object + 28) | 0x40);
    goto finish;
stop_chase:
    w_u32(object + 28, r_u32(object + 28) & 0xFFFFFFBF);
    p2_set_object_animation(object, r_u32(resource + 104));
    w_u16(object + 150, (r_u16(object + 150) + 1024) & 4095);
    target = origin;
finish:
    result = p2_func_80032248(object, target, origin, resource);
    p2_guest_stack_pointer = saved_stack;
    return result;
}

static sint32 p2_path_target_distance(uint32 object, sint32 frame, uint32 target, uint32 temporary)
{
    uint32 axis, sum = 0;
    p2_func_800230f0(r_u32(object + 132), frame, temporary, r_u32(object + 28) & 0x10);
    for (axis = 0; axis < 3; ++axis)
        w_u32(temporary + axis * 4, r_u32(temporary + axis * 4) - r_u32(target + axis * 4));
    for (axis = 0; axis < 3; ++axis)
    {
        sint32 component = r_s32(temporary + axis * 4) >> 12;
        sum += (uint32)component * (uint32)component;
        w_u32(temporary + axis * 4, component);
    }
    return (sint32)sum;
}

uint32 p2_func_80021e70(uint32 object)
{
    uint32 state = r_u32(r_u32(object + 132) + 44), resource = r_u32(state + 72);
    uint32 saved_stack = p2_guest_stack_pointer, origin, target, temporary, result, timer, animation, spawned;
    sint32 next_frame, frame, next, forward_distance, backward_distance;
    p2_guest_stack_pointer -= 80;
    origin = p2_guest_stack_pointer;
    target = origin + 16;
    temporary = origin + 40;
    p2_func_80030dcc(object, origin, target, origin + 32);
    if ((r_u8(state + 38) & 2) != 0)
    {
        target = 0;
        goto finish;
    }
    timer = r_u32(object + 112);
    if (timer != 0)
    {
        w_u32(object + 112, --timer);
        if (timer != 0) goto finish;
    }
    animation = r_u32(object + 104);
    next_frame = animation != 0 && r_s16(object + 130) < r_s16(animation + 6) - 1 ? r_s16(object + 130) + 1 : 0;
    if (next_frame != r_s16(resource + 66) || (r_u32(r_u32(object + 132) + 16) & 0x40) == 0 ||
        (r_u32(object + 28) & 0x30000000) != 0 || r_s8(object + 125) == 17 || r_s32(origin + 32) >= 10001)
        goto finish;
    next = p2_func_80034958(object, origin, target);
    frame = r_s16(object + 136);
    next += frame;
    if (next < frame)
        p2_reverse_object_path(object);
    else if (next == frame)
    {
        if (next >= r_s16(r_u32(object + 132) + 26) - 1)
            forward_distance = (sint32)0x80001000;
        else
            forward_distance = p2_path_target_distance(object, next + 1, target, temporary);
        if (next <= 0)
            backward_distance = 0x7FFFF000;
        else
            backward_distance = p2_path_target_distance(object, next - 1, target, temporary);
        if (backward_distance < r_s32(origin + 32) || r_s32(origin + 32) < forward_distance)
            p2_reverse_object_path(object);
    }
    w_u32(object + 116, r_s8(r_u32(r_u32(object + 132) + 44) + 37) == 8 ? 28 : 26);
    w_u32(object + 28, r_u32(object + 28) | 0x100);
    spawned = p2_dispatch_object_factory(r_u32(0x800BA398), object);
    w_u32(object + 28, r_u32(object + 28) & 0xFFFFFEFF);
    if (spawned != 0)
    {
        if (r_s8(r_u32(r_u32(object + 132) + 44) + 37) != 8)
        {
            p2_transform_object_point(spawned, spawned + 16, spawned + 16, origin + 56);
            w_u8(spawned + 124, 0);
            w_u32(spawned + 28, r_u32(spawned + 28) | 0x2000);
            w_u32(spawned + 100, 0);
            w_u32(spawned + 92, 0);
            w_u32(spawned + 96, p2_func_8002b220(spawned));
        }
        w_u8(spawned + 120, -2);
        w_u8(spawned + 121, -3);
    }
    w_u32(object + 112, (uint32)r_s16(resource + 64));
finish:
    result = p2_func_80032248(object, target, origin, resource);
    p2_guest_stack_pointer = saved_stack;
    return result;
}

uint32 p2_func_8001edac(uint32 object)
{
    uint32 saved_stack = p2_guest_stack_pointer;
    uint32 origin = p2_guest_stack_pointer - 64, target = origin + 16;
    uint32 flags = r_u32(object + 28), was_end = flags & 0x800;
    uint32 state, resource, animation, spawned, shape, magnitude, result;
    sint32 distance, frame, next, count, velocity_y, absolute_y, root, factor;
    uint32 radius, half;
    p2_guest_stack_pointer = origin;
    if (was_end != 0)
    {
        w_u32(object + 28, flags & 0xFFFFF7FF);
        w_u16(object + 136, r_u16(r_u32(object + 132) + 26));
    }
    state = r_u32(r_u32(object + 132) + 44);
    resource = r_u32(state + 72);
    p2_func_80030dcc(object, origin, target, origin + 32);
    if ((r_u8(state + 38) & 2) != 0)
    {
        target = 0;
        goto finish;
    }
    if ((r_u32(r_u32(object + 132) + 16) & 0x40) == 0)
    {
        w_u32(object + 28, r_u32(object + 28) & ~0x40u);
        goto choose_animation;
    }
    if (r_u32(object + 112) != 0)
        w_u32(object + 112, r_u32(object + 112) - 1);
    distance = r_s32(origin + 32);
    if (r_u32(object + 104) != r_u32(resource + 80))
    {
        if ((r_s32(resource + 4) >= distance && r_s16(object + 136) != 0) || r_s32(resource + 8) < distance)
            goto motion;
        if (r_u32(object + 112) != 0) goto finish;
    }
    if ((r_u32(object + 28) & 0x30000000) != 0 || r_s8(object + 125) == 17)
        goto motion;
    if (r_u32(object + 104) != r_u32(resource + 80))
    {
        w_u32(object + 28, r_u32(object + 28) & ~0x40u);
        p2_set_object_animation(object, r_u32(resource + 80));
    }
    animation = r_u32(object + 104);
    next = animation != 0 && r_s16(object + 130) < r_s16(animation + 6) - 1 ? r_s16(object + 130) + 1 : 0;
    if (next == r_s16(resource + 66))
    {
        w_u32(object + 116, 23);
        w_u32(object + 28, r_u32(object + 28) | 0x100);
        spawned = p2_dispatch_object_factory(r_u32(0x800BA398), object);
        w_u32(object + 28, r_u32(object + 28) & ~0x100u);
        if (spawned != 0)
        {
            p2_transform_object_point(spawned, spawned + 16, spawned + 16, origin + 40);
            w_u8(spawned + 124, 0);
            w_u32(spawned + 28, r_u32(spawned + 28) | 0x2000);
            shape = p2_func_800536f0(0);
            w_u32(spawned + 92, r_u32(target) - r_u32(spawned + 16));
            w_u32(spawned + 96, r_u32(target + 4) - r_u32(spawned + 20));
            w_u32(spawned + 100, r_u32(target + 8) - r_u32(spawned + 24));
            velocity_y = (sint32)(r_u32(spawned + 96) - 12288 + r_u32(shape + 76));
            w_u32(spawned + 96, (uint32)velocity_y);
            absolute_y = velocity_y < 0 ? (sint32)(0u - (uint32)velocity_y) : velocity_y;
            magnitude = (uint32)(((sint64)r_s32(spawned + 92) * r_s32(spawned + 92)) >> 12) +
                (uint32)(((sint64)r_s32(spawned + 100) * r_s32(spawned + 100)) >> 12);
            root = (sint32)magnitude < 0 ? (sint32)(2u * p2_sdk_8007e794(magnitude >> 2)) : p2_sdk_8007e794(magnitude);
            distance = root >> 1;
            w_u32(origin + 32, distance);
            if (absolute_y >= distance)
            {
                if (absolute_y == 0)
                {
                    fprintf(stderr, "P2: Zero projectile normalization length at 8001F254\n");
                    abort();
                }
                factor = (sint32)(((sint64)distance * (0x40000000 / absolute_y)) >> 18);
                w_u32(0x1F80030C, factor);
                w_u32(spawned + 96, (uint32)(((sint64)velocity_y * factor) >> 12));
            }
        }
        w_u32(object + 112, (uint32)r_s16(resource + 64));
        goto finish;
    }
    frame = r_s16(object + 130);
    if (frame <= 0 || (animation != 0 && frame < r_s16(animation + 6) - 1)) goto finish;
    p2_set_object_animation(object, r_u32(resource + 72));
    if (r_u16(object + 136) == 0 && r_u32(r_u32(object + 132) + 32) != 0)
        w_u16(object + 136, 1);
    goto finish;
motion:
    if (r_u32(object + 112) != 0) goto finish;
    if (r_s32(resource + 4) < distance && r_s32(resource + 8) >= distance) goto finish;
    if (distance < 10001)
        next = r_s16(object + 136) + p2_func_80034958(object, origin, target);
    else
    {
        if (was_end != 0) p2_reverse_object_path(object);
        next = r_s16(object + 136) + 1;
    }
    frame = r_s16(object + 136);
    count = r_s16(r_u32(object + 132) + 26);
    if (next < frame)
    {
        if (r_s32(resource + 8) < distance)
        {
            radius = r_u32(resource + 20);
            half = (uint32)(r_s32(resource + 20) >> 1);
            w_u32(object + 28, r_u32(object + 28) | 0x40);
            if (frame < count)
            {
                if (radius * 2 >= r_u32(target + 4) - r_u32(object + 64) + radius &&
                    (half * 2 < r_u32(origin) - r_u32(target) + half || half * 2 < r_u32(origin + 8) - r_u32(target + 8) + half))
                    p2_reverse_object_path(object);
            }
            else if (radius * 2 < r_u32(origin) - r_u32(target) + radius || radius * 2 < r_u32(origin + 8) - r_u32(target + 8) + radius)
                p2_reverse_object_path(object);
            else
                w_u32(object + 28, r_u32(object + 28) & ~0x40u);
        }
        else if (frame >= count - 1)
        {
            w_u16(object + 136, 0);
            w_u32(object + 28, (r_u32(object + 28) ^ 0x10) & ~0x40u);
        }
        else w_u32(object + 28, r_u32(object + 28) | 0x40);
    }
    else if (r_s32(resource + 8) >= distance)
    {
        if (frame >= 2)
        {
            p2_reverse_object_path(object);
            w_u32(object + 28, r_u32(object + 28) | 0x40);
        }
        else
        {
            w_u16(object + 136, 0);
            w_u32(object + 28, r_u32(object + 28) & ~0x40u);
        }
    }
    else w_u32(object + 28, frame >= count ? r_u32(object + 28) & ~0x40u : r_u32(object + 28) | 0x40);
choose_animation:
    animation = r_u32(resource + ((r_u32(object + 28) & 0x40) != 0 ? 84 : 72));
    if (r_u32(object + 104) != animation) p2_set_object_animation(object, animation);
finish:
    result = p2_func_80032248(object, target, origin, resource);
    p2_guest_stack_pointer = saved_stack;
    return result;
}

void p2_func_80073660(uint8 value)
{
    p2_sdk_800763ec(value);
}

void p2_func_800736ac(uint8 value)
{
    w_u32(0x800B14B0, 16);
    w_u32(0x800B14C0, value);
    p2_sdk_8006ff38(0x800B14B0);
}

void p2_func_800736d0(uint8 value)
{
    w_u32(0x800B14B0, 8);
    w_u32(0x800B14BC, value);
    p2_sdk_8006ff38(0x800B14B0);
}

void p2_func_800736f4(uint8 value)
{
    p2_func_800736d0(value);
}

uint32 p2_func_8006f824(uint32 source, uint16 address_units, uint32 blocks, uint32 port)
{
    uint32 control;
    w_u16(port + 422, address_units);
    port = r_u32(0x80087D5C);
    w_u16(port + 426, r_u16(port + 426) | 0x30);
    control = r_u32(0x80087D70);
    w_u32(control, (r_u32(control) & 0xD0FFFFFF) | 0x22000000);
    w_u32(r_u32(0x80087D60), source);
    w_u32(r_u32(0x80087D64), (blocks << 16) | 0x10);
    w_u32(0x80087DAC, 1);
    w_u32(r_u32(0x80087D68), 0x01000200);
    return 1;
}

uint32 p2_func_8008040c(uint32 ordering_table, sint32 count, uint32 debug_level)
{
    uint32 cursor = ordering_table;
    if (debug_level >= 2)
        p2_sdk_80082a18((const char *)psx_addr(0x800110BC, 1), ordering_table, (uint32)count);
    if (count <= 0)
    {
        fprintf(stderr, "P2: Invalid ClearOTag count %d\n", count);
        abort();
    }
    while (--count != 0)
    {
        w_u8(cursor + 3, 0);
        w_u32(cursor, (r_u32(cursor) & 0xFF000000) | ((cursor + 4) & 0xFFFFFF));
        cursor += 4;
    }
    w_u32(cursor, 0x0008A040);
    return cursor;
}

void p2_func_8008342c(void)
{
    PadStopCom();
}

uint32 p2_func_8007aa1c(uint32 driver, uint32 channel, uint32 callback)
{
    uint32 target = r_u32(driver + 20), slot, previous;
    if (target != 0x8007B130)
    {
        fprintf(stderr, "P2: Unmapped interrupt vector driver %08X\n", target);
        abort();
    }
    slot = 0x800894D4 + channel * 4;
    previous = r_u32(slot);
    if (callback != previous) w_u32(slot, callback);
    return previous;
}

uint32 p2_func_8007aa7c(uint32 driver)
{
    uint32 target = r_u32(driver + 24);
    if (target != 0x8007AFAC)
    {
        fprintf(stderr, "P2: Unmapped restart callback driver %08X\n", target);
        abort();
    }
    if (r_u16(0x80088438) != 0) return 0;
    w_u32(0x75D0, 0x80088470);
    w_u16(0x80088438, 1);
    w_u16(r_u32(0x800894C8), r_u16(0x8008846A));
    w_u32(r_u32(0x800894CC), r_u32(0x8008846C));
    ExitCriticalSection();
    return 0x80088438;
}

static void p2_graph_control(uint32 driver, uint32 command)
{
    uint32 target = r_u32(driver + 16);
    if (target != 0x80081D20)
    {
        fprintf(stderr, "P2: Unmapped GPU control driver %08X\n", target);
        abort();
    }
    w_u32(r_u32(0x8008A058), command);
    w_u8(0x800ABC44 + (command >> 24), command);
}

uint32 p2_func_8007fc78(uint32 reverse, uint32 debug_level)
{
    uint32 previous = r_u8(0x80089F87), driver, mode;
    if (debug_level >= 2)
        p2_sdk_80082a18((const char *)psx_addr(0x80010FCC, 1), reverse);
    w_u8(0x80089F87, reverse);
    driver = r_u32(0x80089F7C);
    if (r_u32(driver + 40) != 0x80081D48)
    {
        fprintf(stderr, "P2: Unmapped GPU cached command driver %08X\n", r_u32(driver + 40));
        abort();
    }
    mode = r_u8(0x800ABC4C);
    p2_graph_control(driver, mode | (r_u8(0x80089F87) != 0 ? 0x08000080 : 0x08000000));
    if (r_u8(0x80089F84) == 2)
        p2_graph_control(r_u32(0x80089F7C), r_u8(0x80089F87) != 0 ? 0x20000501 : 0x20000504);
    return previous;
}

uint32 p2_func_8007fdf8(uint32 enabled, uint32 debug_level)
{
    uint32 previous = r_u8(0x80089F85), driver;
    if (debug_level >= 2)
        p2_sdk_80082a18((const char *)psx_addr(0x80011010, 1), enabled);
    if (enabled == r_u8(0x80089F85)) return previous;
    driver = r_u32(0x80089F7C);
    if (r_u32(driver + 52) != 0x8008242C)
    {
        fprintf(stderr, "P2: Unmapped GPU reset queue driver %08X\n", r_u32(driver + 52));
        abort();
    }
    w_u32(0x8008A098, psx_irq_native_mask_exchange(0));
    w_u32(0x8008A08C, 0);
    w_u32(0x8008A088, 0);
    w_u32(r_u32(0x8008A064), 1025);
    w_u32(r_u32(0x8008A074), r_u32(r_u32(0x8008A074)) | 0x800);
    w_u32(r_u32(0x8008A058), 0x02000000);
    w_u32(r_u32(0x8008A058), 0x01000000);
    psx_irq_native_mask_exchange(r_u16(0x8008A098));
    w_u8(0x80089F85, enabled);
    driver = r_u32(0x800894C0);
    if (r_u32(driver + 8) != 0x8007ADAC)
    {
        fprintf(stderr, "P2: Unmapped interrupt callback driver %08X\n", r_u32(driver + 8));
        abort();
    }
    if (r_u16(0x80088438) != 0 && r_u32(0x80088444) != 0)
    {
        uint16 mask = psx_irq_native_mask_exchange(0);
        w_u32(0x80088444, 0);
        w_u16(0x80088468, r_u16(0x80088468) & ~4u);
        InterruptCallback(2, NULL);
        psx_irq_native_mask_exchange(mask & ~4u);
    }
    return previous;
}

uint32 p2_func_80080cf0(uint32 driver)
{
    uint32 target = r_u32(driver + 56);
    if (target != 0x800814C0)
    {
        fprintf(stderr, "P2: Unmapped GPU status driver %08X\n", target);
        abort();
    }
    return r_u32(r_u32(0x8008A058)) >> 31;
}

uint32 p2_func_8008373c(uint32 type, uint32 destination)
{
    return PAD_init2(type, destination);
}
