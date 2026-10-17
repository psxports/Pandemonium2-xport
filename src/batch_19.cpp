#include "game.h"
#include <stdlib.h>
#include <stdarg.h>

uint32 p2_func_8002ab84(uint32 object_offset, uint32 position_offset)
{
    FUNCTION_MARKER(0x8002AB84, "SLUS_005.78");
    uint32 definition = r_u32(object_offset + 132);
    uint32 saved_stack;
    uint32 first;
    uint32 second;
    uint32 square = 0;
    uint32 length;
    uint32 midpoint[3];
    uint32 index;
    uint32 radius;
    uint32 result;
    if (definition == 0)
        return 0;
    saved_stack = p2_guest_stack_pointer;
    p2_guest_stack_pointer -= 32;
    first = p2_guest_stack_pointer;
    second = first + 16;
    SampleWorldPath(definition, 0, first, 1);
    definition = r_u32(object_offset + 132);
    SampleWorldPath(definition, r_s16(definition + 26) - 1, second, 1);
    for (index = 0; index < 3; ++index)
    {
        uint32 difference = r_u32(first + index * 4) - r_u32(second + index * 4);
        uint32 scaled = (uint32)((sint32)difference >> 6);
        square += scaled * scaled;
    }
    if ((sint32)square >= 0)
        length = p2_sdk_8007e794(square);
    else
        length = p2_sdk_8007e794(square >> 2) * 2;
    radius = (uint32)((sint32)length >> 1);
    for (index = 0; index < 3; ++index)
    {
        uint32 sum = r_u32(first + index * 4) + r_u32(second + index * 4);
        midpoint[index] = (uint32)((sint32)sum >> 1);
    }
    square = 0;
    for (index = 0; index < 3; ++index)
    {
        uint32 difference = r_u32(position_offset + index * 4) - midpoint[index];
        uint32 scaled = (uint32)((sint32)difference >> 6);
        square += scaled * scaled;
    }
    radius -= 40960;
    if ((sint32)square >= 0)
        result = p2_sdk_8007e794(square) - radius;
    else
        result = p2_sdk_8007e794(square >> 2) * 2 - radius;
    p2_guest_stack_pointer = saved_stack;
    return result;
}



uint32 p2_func_8001d698(uint32 object_offset, uint32 character_offset)
{
    FUNCTION_MARKER(0x8001D698, "SLUS_005.78");
    uint32 saved_stack;
    uint32 velocity[3];
    uint32 other;
    uint32 packed;
    uint32 collision;
    uint32 state;
    uint32 result;
    uint32 index;
    p2_func_800455dc(object_offset, character_offset, r_s32(character_offset + 84), r_s32(character_offset + 88));
    velocity[0] = r_u32(object_offset + 92);
    velocity[1] = r_u32(object_offset + 96);
    velocity[2] = r_u32(object_offset + 100);
    w_u32(object_offset + 92, 0);
    w_u32(object_offset + 96, (uint32)-40960);
    w_u32(object_offset + 100, 0);
    other = r_u32(character_offset + 112);
    saved_stack = p2_guest_stack_pointer;
    p2_guest_stack_pointer -= 64;
    collision = p2_guest_stack_pointer;
    state = collision + 48;
    w_u32(state, 4096);
    {
        uint32 kind = r_u8(other + 12);
        packed = r_u32(character_offset + 108);
        if (kind == 1)
        {
            uint32 first = r_u32(0x80010180);
            uint32 second = r_u32(0x80010184);
            uint32 third = r_u32(0x80010188);
            uint32 source;
            w_u32(collision + 4, first);
            w_u32(collision + 8, second);
            w_u32(collision + 12, third);
            w_u32(collision + 16, r_u32(0x8001018C));
            w_u16(collision + 20, r_u16(0x80010190));
            source = r_s16(other + 34) > 0 ? other : r_u32(other + 40);
            first = r_u32(source + 16);
            second = r_u32(source + 20);
            third = r_u32(source + 24);
            w_u32(collision + 24, first);
            w_u32(collision + 28, second);
            w_u32(collision + 32, third);
            w_u32(collision + 44, 0);
            w_u32(collision + 40, 0);
            w_u32(collision + 36, 0);
        }
        else
        {
            uint32 source = r_u32(other + 140) + (packed >> 16) * 48;
            uint32 offset;
            for (offset = 0; offset < 48; offset += 16)
            {
                uint32 first = r_u32(source + offset);
                uint32 second = r_u32(source + offset + 4);
                uint32 third = r_u32(source + offset + 8);
                uint32 fourth = r_u32(source + offset + 12);
                w_u32(collision + offset, first);
                w_u32(collision + offset + 4, second);
                w_u32(collision + offset + 8, third);
                w_u32(collision + offset + 12, fourth);
            }
        }
    }
    w_u16(collision + 2, 1);
    w_u16(collision, packed);
    result = (uint32)p2_func_800380ac(object_offset, 0x8008A538, r_u32(character_offset + 112), collision, state);
    w_u32(object_offset + 92, velocity[0]);
    w_u32(object_offset + 96, velocity[1]);
    w_u32(object_offset + 100, velocity[2]);
    if ((sint32)result < 0)
    {
        uint32 horizontal = r_u32(character_offset + 84);
        uint32 vertical = r_u32(character_offset + 88);
        character_controller.motion.horizontal_speed = horizontal;
        character_controller.motion.vertical_speed = vertical;
        if ((sint32)vertical > 0)
        {
            vertical = character_controller.motion.vertical_speed;
            character_controller.motion.horizontal_speed = 0;
            character_controller.motion.vertical_speed = 0u - vertical;
        }
        result = character_controller.flags & ~0x10000u;
        character_controller.flags = result;
    }
    else
    {
        w_u32(state, 4096);
        result = (uint32)p2_func_800380ac(object_offset, 0x8008A538, r_u32(character_offset + 112), collision, state);
        if ((sint32)result >= 0)
        {
            uint32 definition;
            if ((r_u32(object_offset + 28) & 0x2000) == 0 && (definition = r_u32(object_offset + 132)) != 0 && r_u32(definition + 40) != 0)
            {
                for (index = 0; index < 3; ++index)
                {
                    uint32 matrix = r_u32(r_u32(object_offset + 132) + 40);
                    uint32 value = r_u32(collision + 36 + index * 4);
                    value -= r_u32(matrix + 32 + index * 4);
                    w_u32(collision + 36 + index * 4, value);
                }
            }
            if (r_u32(collision + 36) != 0 || r_u32(collision + 40) != 0 || r_u32(collision + 44) != 0)
            {
                uint32 horizontal = r_u32(object_offset + 92);
                uint32 brake = 0;
                sint32 factor;
                if (horizontal != 0 || r_u32(object_offset + 100) != 0)
                {
                    sint16 motion_angle = (sint16)Atan2Angle(r_s32(object_offset + 100), (sint32)horizontal);
                    sint16 contact_angle = (sint16)Atan2Angle(r_s32(collision + 44), r_s32(collision + 36));
                    brake = (((uint32)(sint32)motion_angle - (uint32)(sint32)contact_angle + 1024) & 4095) < 2048;
                }
                factor = brake != 0 ? 3993 : 4198;
                for (index = 0; index < 3; ++index)
                {
                    uint32 value = (uint32)(((sint64)r_s32(collision + 36 + index * 4) * factor) >> 12);
                    w_u32(collision + 36 + index * 4, value);
                }
                if (brake != 0)
                {
                    for (index = 0; index < 3; ++index)
                        w_u32(object_offset + 92 + index * 4, r_u32(object_offset + 92 + index * 4) + r_u32(collision + 36 + index * 4));
                }
                else
                    p2_func_8001d3f0(object_offset, character_offset, r_s32(collision + 36), r_s32(collision + 40), r_s32(collision + 44));
            }
            result = r_u32(object_offset + 96) + 682;
            w_u32(object_offset + 96, result);
        }
    }
    p2_guest_stack_pointer = saved_stack;
    return result;
}

uint32 p2_func_8001d3f0(uint32 object_offset, uint32 character_offset, sint32 x, sint32 y, sint32 z)
{
    FUNCTION_MARKER(0x8001D3F0, "SLUS_005.78");
    uint32 index;
    uint32 result;
    uint32 definition;
    for (index = 0; index < 4; ++index)
    {
        uint32 point = object_offset + index * 16;
        w_u32(point + 160, r_u32(point + 160) + (uint32)x);
        w_u32(point + 164, r_u32(point + 164) + (uint32)y);
        w_u32(point + 168, r_u32(point + 168) + (uint32)z);
    }
    w_u32(object_offset + 60, r_u32(object_offset + 60) + (uint32)x);
    w_u32(object_offset + 64, r_u32(object_offset + 64) + (uint32)y);
    w_u32(object_offset + 68, r_u32(object_offset + 68) + (uint32)z);
    if ((r_u32(object_offset + 28) & 0x2000) == 0)
    {
        definition = r_u32(object_offset + 132);
        if (definition != 0 && r_u32(definition + 40) != 0)
        {
            uint32 saved_stack = p2_guest_stack_pointer;
            uint32 vector;
            uint32 transpose;
            uint32 transformed;
            p2_guest_stack_pointer -= 64;
            vector = p2_guest_stack_pointer;
            transpose = vector + 16;
            transformed = vector + 48;
            for (index = 0; index < 9; ++index)
            {
                uint32 matrix = r_u32(r_u32(object_offset + 132) + 40);
                w_u16(transpose + index * 2, r_u16(matrix + (index % 3) * 6 + (index / 3) * 2));
            }
            w_u32(vector, (uint32)x);
            w_u32(vector + 4, (uint32)y);
            w_u32(vector + 8, (uint32)z);
            PushMatrix();
            for (index = 0; index < 5; ++index)
                xport_gte_write_control(index, r_u32(transpose + index * 4));
            p2_sdk_8007ec64(vector, transformed);
            PopMatrix();
            x = r_s32(transformed);
            y = r_s32(transformed + 4);
            z = r_s32(transformed + 8);
            p2_guest_stack_pointer = saved_stack;
        }
    }
    w_u32(object_offset + 16, r_u32(object_offset + 16) + (uint32)x);
    w_u32(object_offset + 20, r_u32(object_offset + 20) + (uint32)y);
    w_u32(object_offset + 24, r_u32(object_offset + 24) + (uint32)z);
    character_controller.flags = character_controller.flags | 1;
    w_u32(character_offset + 188, 0);
    w_u32(character_offset + 672, 0);
    result = r_u32(object_offset + 28) & ~0x4000u;
    w_u32(object_offset + 28, result);
    return result;
}

uint32 p2_func_8001eb44(uint32 object_offset, uint32 character_offset)
{
    FUNCTION_MARKER(0x8001EB44, "SLUS_005.78");
    uint32 other = r_u32(character_offset + 112);
    uint32 kind = r_u8(other + 12);
    uint32 packed = r_u32(character_offset + 108);
    uint32 saved_stack = p2_guest_stack_pointer;
    uint32 collision;
    uint32 state;
    uint32 result;
    p2_guest_stack_pointer -= 64;
    collision = p2_guest_stack_pointer;
    state = collision + 48;
    if (kind == 1)
    {
        uint32 first = r_u32(0x80010180);
        uint32 second = r_u32(0x80010184);
        uint32 third = r_u32(0x80010188);
        uint32 source;
        w_u32(collision + 4, first);
        w_u32(collision + 8, second);
        w_u32(collision + 12, third);
        w_u32(collision + 16, r_u32(0x8001018C));
        w_u16(collision + 20, r_u16(0x80010190));
        source = r_s16(other + 34) > 0 ? other : r_u32(other + 40);
        first = r_u32(source + 16);
        second = r_u32(source + 20);
        third = r_u32(source + 24);
        w_u32(collision + 24, first);
        w_u32(collision + 28, second);
        w_u32(collision + 32, third);
        w_u32(collision + 44, 0);
        w_u32(collision + 40, 0);
        w_u32(collision + 36, 0);
    }
    else
    {
        uint32 source = r_u32(other + 140) + (packed >> 16) * 48;
        uint32 offset;
        for (offset = 0; offset < 48; offset += 16)
        {
            uint32 first = r_u32(source + offset);
            uint32 second = r_u32(source + offset + 4);
            uint32 third = r_u32(source + offset + 8);
            uint32 fourth = r_u32(source + offset + 12);
            w_u32(collision + offset, first);
            w_u32(collision + offset + 4, second);
            w_u32(collision + offset + 8, third);
            w_u32(collision + offset + 12, fourth);
        }
    }
    w_u16(collision + 2, 1);
    w_u16(collision, packed);
    w_u32(state, 4096);
    result = (uint32)p2_func_800380ac(object_offset, 0x8008A538, r_u32(character_offset + 112), collision, state);
    if ((sint32)result >= 0)
        w_u32(object_offset + 96, 0);
    else
    {
        result = r_u32(object_offset + 96);
        if (result != 0)
        {
            result = character_controller.flags & 0xFFFF7FFF;
            character_controller.flags = result;
        }
    }
    p2_guest_stack_pointer = saved_stack;
    return result;
}

uint32 p2_func_80035058(uint32 angle_offset, uint32 vector_offset, uint32 object_offset, sint32 frame, uint32 reverse)
{
    FUNCTION_MARKER(0x80035058, "SLUS_005.78");
    sint32 first_frame = frame;
    sint32 second_frame = (sint32)((uint32)frame + 1);
    uint32 saved_stack = p2_guest_stack_pointer;
    uint32 first;
    uint32 second;
    uint32 x;
    uint32 y;
    uint32 z;
    uint32 result;
    /* Initialize the adjacent sample for the original undefined reverse-zero branch */
    if (reverse != 0 && r_s16(object_offset + 136) >= r_s16(r_u32(object_offset + 132) + 26))
    {
        second_frame = frame;
        first_frame = (sint32)((uint32)frame - 1);
    }
    p2_guest_stack_pointer -= 32;
    first = p2_guest_stack_pointer;
    second = first + 16;
    p2_func_80056340(r_u32(object_offset + 132), first_frame, first, reverse);
    p2_func_80056340(r_u32(object_offset + 132), second_frame, second, reverse);
    x = r_u32(second) - r_u32(first);
    y = r_u32(second + 4) - r_u32(first + 4);
    z = r_u32(second + 8) - r_u32(first + 8);
    w_u32(vector_offset, x);
    w_u32(vector_offset + 4, y);
    w_u32(vector_offset + 8, z);
    result = Atan2Angle((sint32)z, (sint32)x);
    w_u16(angle_offset, result);
    p2_guest_stack_pointer = saved_stack;
    return result;
}

uint32 p2_func_80037f08(uint32 object_offset)
{
    FUNCTION_MARKER(0x80037F08, "SLUS_005.78");
    uint32 saved_stack = p2_guest_stack_pointer;
    uint32 vector;
    uint32 result;
    p2_guest_stack_pointer -= 16;
    vector = p2_guest_stack_pointer;
    w_u32(vector, 0u - r_u32(object_offset + 92));
    w_u32(vector + 8, 0u - r_u32(object_offset + 100));
    w_u32(vector + 4, r_u32(object_offset + 96));
    p2_func_80040384(object_offset, vector, 4096);
    result = (r_u16(object_offset + 148) + 2048) & 4095;
    w_u16(object_offset + 150, result);
    w_u16(object_offset + 148, result);
    p2_guest_stack_pointer = saved_stack;
    return result;
}

uint32 p2_sdk_800742f4(sint16 sequence, sint16 track)
{
    FUNCTION_MARKER(0x800742F4, "SLUS_005.78");
    uint32 table = 0x800B9B00 + (uint32)(sint32)sequence * 4;
    uint32 stride = (uint32)((sint32)track * 172);
    uint32 entry = r_u32(table) + stride;
    uint32 flags = r_u32(entry + 144);
    uint32 current;
    if ((flags & 0x204) != 0)
        return flags & 0x100;
    if ((flags & 0x100) != 0)
        return (uint32)-3;
    w_u32(entry + 144, flags & ~2u);
    current = r_u32(table) + stride;
    w_u32(current + 144, r_u32(current + 144) | 8);
    current = r_u32(table) + stride;
    flags = r_u32(current + 144) | 1;
    w_u32(current + 144, flags);
    return flags;
}

uint32 p2_sdk_80074398(sint16 sequence)
{
    return p2_sdk_800742f4(sequence, 0);
}

uint32 p2_func_8006ada4(uint16 handle)
{
    FUNCTION_MARKER(0x8006ADA4, "SLUS_005.78");
    return p2_sdk_80074398((sint16)((sint32)(uint8)handle - 1));
}

void p2_func_8006b000(void)
{
    FUNCTION_MARKER(0x8006B000, "SLUS_005.78");
    p2_func_8008346c();
    StopRCnt(0xF2000002, 0x8008A0B0, 0x8008A0B8);
    DisableEventPSX(r_u32(0x8008A3B4));
    CloseEventPSX(r_u32(0x8008A3B4));
    p2_func_8008347c();
}

uint32 p2_func_800289d4(uint32 object_offset, uint32 owner_offset)
{
    FUNCTION_MARKER(0x800289D4, "SLUS_005.78");
    uint32 flags = character_action_flags;
    uint32 amplitude = 8192;
    uint32 step[3];
    uint32 position[3] = {0, 0, 0};
    uint32 child;
    uint32 index;
    sint16 countdown;
    if ((flags & 8) == 0 || (r_u32(object_offset + 28) & 0x20200) == 512)
    {
        uint32 active = character_action_target_offset;
        uint8 object_flags;
        character_action_flags = (flags | 0x800) & ~8u;
        if (active != 0)
        {
            uint32 track = r_u32(r_u32(active + 132) + 44);
            w_u8(track + 38, r_u8(track + 38) & 0xF7);
        }
        object_flags = r_u8(object_offset + 13);
        w_u8(object_offset + 125, 0);
        w_u8(object_offset + 13, object_flags | 2);
        return object_flags | 2;
    }
    countdown = r_s16(object_offset + 136);
    if (countdown != 0)
    {
        countdown = (sint16)(countdown - 1);
        w_u16(object_offset + 136, countdown);
        if (countdown >= 3)
            amplitude = (uint32)((4 - countdown) * 2048 + 8192);
        else if (countdown >= 0)
        {
            amplitude = 12288;
            if (countdown == 0)
                w_u16(object_offset + 136, (uint32)-1);
        }
        else if (countdown == -5)
            w_u16(object_offset + 136, 0);
        else
            amplitude = (uint32)(((countdown + 6) * 4096) / 5 + 8192);
    }
    for (index = 0; index < 3; ++index)
    {
        uint32 difference = r_u32(owner_offset + 60 + index * 4) - r_u32(object_offset + 16 + index * 4);
        step[index] = (uint32)(((sint64)(sint32)difference * 256) >> 12);
    }
    {
        uint32 phase = r_u8(0x800ABF23) + 1;
        w_u8(0x800ABF23, phase);
        if ((sint8)phase >= 16)
            w_u8(0x800ABF23, 0);
    }
    child = r_u32(object_offset + 8);
    for (index = 0; index < 16; ++index)
    {
        uint32 wave_scale = (index < 9 ? index : 15 - index) * 1280;
        sint32 phase;
        uint32 wave;
        uint32 height;
        w_u32(child + 28, position[0]);
        w_u32(child + 32, position[1]);
        w_u32(child + 36, position[2]);
        phase = (r_s8(0x800ABF23) + (sint32)index) % 16;
        wave = (uint32)(((sint64)r_s32(0x80083BAC + (uint32)phase * 4) * wave_scale) >> 12);
        height = r_u32(child + 32);
        w_u16(child + 42, amplitude >> 5);
        w_u16(child + 40, amplitude >> 5);
        w_u32(child + 32, height + wave);
        position[0] += step[0];
        position[1] += step[1];
        position[2] += step[2];
        child = r_u32(child + 4);
    }
    index = r_u16(object_offset + 130) + 1;
    w_u16(object_offset + 130, index);
    return index;
}

uint32 p2_func_8001bb58(uint32 object_offset)
{
    FUNCTION_MARKER(0x8001BB58, "SLUS_005.78");
    uint32 owner = r_u32(object_offset + 80);
    uint32 result = r_u32(owner + 28) & 32;
    if (result == 0)
    {
        result = r_u32(owner + 104);
        if (result != 0)
        {
            sint32 index = r_s8(object_offset + 86);
            if (index >= 0)
            {
                uint32 track = r_u32(result + 16) + (uint32)index * 16;
                uint8 flags = r_u8(track + 2);
                result = flags & 0xF7;
                if ((flags & 8) != 0)
                {
                    sint32 frame = r_s16(track + 8);
                    uint32 saved_stack = p2_guest_stack_pointer;
                    uint32 output;
                    uint32 first;
                    uint32 second;
                    uint32 third;
                    p2_guest_stack_pointer -= 16;
                    output = p2_guest_stack_pointer;
                    w_u8(track + 2, result);
                    SampleVectorTrack(frame, track, output);
                    w_u16(object_offset + 60, r_s32(output) >> 5);
                    first = r_u16(object_offset + 60);
                    w_u16(object_offset + 62, r_s32(output + 4) >> 5);
                    third = (uint32)r_s32(output + 8);
                    second = r_u16(object_offset + 62);
                    w_u16(object_offset + 66, first);
                    w_u16(object_offset + 64, (sint32)third >> 5);
                    if (first < second)
                        w_u16(object_offset + 66, second);
                    third = r_u16(object_offset + 64);
                    result = r_u16(object_offset + 66) < third;
                    if (result != 0)
                        w_u16(object_offset + 66, third);
                    p2_guest_stack_pointer = saved_stack;
                }
            }
        }
    }
    return result;
}

uint32 p2_func_8003fc44(uint32 object_offset, uint32 action)
{
    FUNCTION_MARKER(0x8003FC44, "SLUS_005.78");
    sint32 count = r_s32(0x8008A334);
    uint32 descriptor = r_u32(object_offset + 116);
    uint32 entry = 0;
    sint32 index;
    uint32 result;
    for (index = 0; index < count; ++index)
    {
        uint32 candidate = r_u32(0x8008B468 + (uint32)index * 4);
        if (r_u32(candidate) == descriptor)
        {
            entry = candidate;
            break;
        }
    }
    result = r_u16(entry + 24);
    if (result != 0)
    {
        uint32 slot = entry;
        uint32 position = 0;
        do
        {
            uint32 found = r_u32(slot + 32);
            ++position;
            if (action == found)
            {
                uint32 value = r_u32(object_offset + 92) + r_u32(slot + 36);
                uint32 limit;
                w_u32(object_offset + 92, value);
                limit = r_u32(entry + 8);
                if ((sint32)limit < (sint32)value)
                    w_u32(object_offset + 92, limit);
                value = r_u32(object_offset + 92);
                limit = r_u32(entry + 4);
                result = (sint32)value < (sint32)limit;
                if (result != 0)
                    w_u32(object_offset + 92, limit);
                return result;
            }
            result = position < r_u16(entry + 24);
            slot += 8;
        } while (result != 0);
    }
    return result;
}



uint32 p2_func_800606fc(uint32 object_offset)
{
    FUNCTION_MARKER(0x800606FC, "SLUS_005.78");
    uint32 definition = p2_func_8003fb58(r_u32(object_offset + 116));
    uint32 child = r_u32(object_offset + 8);
    uint32 result = 0;
    while (child != 0)
    {
        if (r_u8(child + 12) == 0 && (r_u32(child + 228) & 0x2000) != 0)
        {
            uint32 position = r_u32(child + 92);
            if (position != 0)
            {
                result = position;
                break;
            }
            position = p2_func_800606fc(child);
            if (position != r_u32(child + 92))
            {
                result = position;
                break;
            }
        }
        child = r_u32(child + 4);
    }
    return result != 0 ? result : r_u32(definition + 12);
}

uint32 p2_func_8002c510(uint32 object_offset)
{
    FUNCTION_MARKER(0x8002C510, "SLUS_005.78");
    uint32 result = 6;
    if (r_s8(object_offset + 125) == 6)
    {
        result = r_u32(object_offset + 28) & 0x40000020;
        if (result == 0x40000020)
        {
            uint32 remaining = r_u32(0x8008A320);
            uint32 slot = 0x8008B420;
            uint32 kind = r_u32(object_offset + 116);
            uint32 entry = 0;
            while (remaining != 0)
            {
                uint32 candidate = r_u32(slot);
                --remaining;
                if (candidate != 0 && r_u8(candidate + 1) == kind)
                {
                    entry = candidate;
                    break;
                }
                slot += 4;
            }
            result = r_u32(entry + 40);
            if (result != 0)
            {
                uint32 sounds = r_u32(entry + 4);
                uint32 sound = 0;
                if (sounds != 0 && r_s32(sounds) >= 3)
                    sound = sounds + 20;
                if ((uint8)sound_effects.IsPlaying(sound, (uint32)(sint32)r_s16(object_offset + 138)) != 0)
                    p2_func_8006a970(r_u16(object_offset + 138));
                result = (uint32)-1;
                w_u16(object_offset + 138, result);
            }
        }
    }
    return result;
}

uint32 p2_func_80030c2c(uint32 object_offset)
{
    FUNCTION_MARKER(0x80030C2C, "SLUS_005.78");
    uint32 state = r_u32(object_offset + 116);
    sint32 countdown = (sint32)state >> 16;
    sint32 scale = state & 0xFFFF;
    uint32 target;
    if (countdown != 0)
    {
        --countdown;
        if (countdown == 0)
        {
            uint32 flags = r_u32(object_offset + 28);
            uint32 sounds = r_u32(0x8008A408);
            uint32 sound = 0;
            w_u32(object_offset + 28, flags | 0x20000000);
            if (sounds != 0 && r_s32(sounds) >= 14)
                sound = sounds + 108;
            sound_effects.Queue(sound, 0, 0, sound_settings.effects_volume, 0, 3);
            w_u32(object_offset + 28, r_u32(object_offset + 28) & ~0x10000000u);
        }
    }
    else
    {
        uint32 flags = r_u32(object_offset + 28);
        if ((flags & 0x10000000) != 0)
        {
            if (scale >= 1366)
            {
                scale -= 182;
                if (scale < 1366)
                {
                    scale = 1365;
                    countdown = 150;
                }
            }
        }
        else if (scale < 4096)
        {
            scale += 182;
            if (scale >= 4096)
            {
                scale = 4096;
                w_u32(object_offset + 28, flags & ~0x20000000u);
            }
        }
    }
    target = r_u32(object_offset + 156);
    if (target != 0)
    {
        uint32 definition = r_u32(r_u32(r_u32(object_offset + 132) + 44) + 72);
        uint32 product = (uint32)(((sint64)r_s32(definition + 40) * scale) >> 12);
        w_u16(target + 40, (sint32)product >> 5);
        product = (uint32)(((sint64)r_s32(definition + 44) * scale) >> 12);
        target = r_u32(object_offset + 156);
        w_u16(target + 42, (sint32)product >> 5);
    }
    state = (uint32)scale | ((uint32)countdown << 16);
    w_u32(object_offset + 116, state);
    return state;
}

uint32 p2_func_8002d750(uint32 kind)
{
    FUNCTION_MARKER(0x8002D750, "SLUS_005.78");
    uint32 remaining = r_u32(0x8008A320);
    uint32 slot = 0x8008B420;
    uint32 found = 0;
    while (remaining != 0)
    {
        uint32 object_offset = r_u32(slot);
        --remaining;
        if (object_offset != 0 && r_u8(object_offset + 1) == kind)
        {
            found = object_offset;
            break;
        }
        slot += 4;
    }
    return r_u8(found + 44);
}

uint32 p2_func_80035154(uint32 angle_offset, uint32 vector_offset, uint32 object_offset)
{
    FUNCTION_MARKER(0x80035154, "SLUS_005.78");
    uint32 x = r_u32(object_offset + 92);
    uint32 y = r_u32(object_offset + 96);
    uint32 z = r_u32(object_offset + 100);
    uint32 result;
    w_u32(vector_offset, x);
    w_u32(vector_offset + 4, y);
    w_u32(vector_offset + 8, z);
    result = Atan2Angle(r_s32(object_offset + 100), r_s32(object_offset + 92));
    w_u16(angle_offset, result);
    return result;
}

uint32 p2_func_8003fb58(uint32 descriptor)
{
    FUNCTION_MARKER(0x8003FB58, "SLUS_005.78");
    sint32 count = r_s32(0x8008A334);
    sint32 index;
    for (index = 0; index < count; ++index)
    {
        uint32 entry = r_u32(0x8008B468 + (uint32)index * 4);
        if (r_u32(entry) == descriptor)
            return entry;
    }
    return 0;
}

uint32 p2_func_8003fbb0(uint32 object_offset)
{
    FUNCTION_MARKER(0x8003FBB0, "SLUS_005.78");
    sint32 count = r_s32(0x8008A334);
    uint32 descriptor = r_u32(object_offset + 116);
    uint32 entry = 0;
    uint32 center;
    uint32 extent;
    uint32 position;
    uint32 result;
    sint32 index;
    for (index = 0; index < count; ++index)
    {
        uint32 candidate = r_u32(0x8008B468 + (uint32)index * 4);
        if (r_u32(candidate) == descriptor)
        {
            entry = candidate;
            break;
        }
    }
    center = r_u32(entry + 16);
    extent = r_u32(entry + 20);
    position = r_u32(object_offset + 92);
    result = position - extent;
    if ((sint32)(center + extent) < (sint32)position)
        w_u32(object_offset + 92, result);
    else
    {
        result = position + extent;
        if ((sint32)position < (sint32)(center - extent))
            w_u32(object_offset + 92, result);
    }
    return result;
}


uint32 p2_func_8004a888(uint32 object_offset, uint32 character_offset)
{
    FUNCTION_MARKER(0x8004A888, "SLUS_005.78");
    w_u32(object_offset + 28, r_u32(object_offset + 28) & ~0x4000000u);
    if ((sint8)character_controller.motion.state_code == 9 && (character_controller.secondary_flags & 128) == 0)
        character_controller.motion.state_code = 2;
    character_controller.movement.speed_limits[0] =  character_controller.model_slots[15];
    character_controller.movement.speed_limits[1] =  character_controller.model_slots[16];
    character_controller.movement.speed_limits[2] =  character_controller.model_slots[17];
    character_controller.movement.speed_limits[3] =  character_controller.model_slots[18];
    character_controller.motion.horizontal_speed = character_controller.model_slots[16];
    character_controller.model_slots[16] =  (uint32)-15;
    return (uint32)-15;
}

void p2_func_80059ac4(void)
{
    FUNCTION_MARKER(0x80059AC4, "SLUS_005.78");
}

void p2_func_80059acc(void)
{
    FUNCTION_MARKER(0x80059ACC, "SLUS_005.78");
}

void p2_func_8006a528(void)
{
    FUNCTION_MARKER(0x8006A528, "SLUS_005.78");
}

uint32 p2_func_80066e48(void)
{
    FUNCTION_MARKER(0x80066E48, "SLUS_005.78");
    uint32 flags = game_flow.mode_flags;
    w_u32(0x8008A290, (uint32)-1);
    return ReadAssetToGuest(254, r_u32(0x8008A4C8), (flags & 0x200) != 0);
}

uint32 p2_func_8006ec24(uint32 left_low, uint32 left_high, uint32 right_low, uint32 right_high)
{
    FUNCTION_MARKER(0x8006EC24, "SLUS_005.78");
    if (left_high == right_high && left_low == right_low)
        return 0;
    return (left_high & 0x7FFFFFFF) != 0 || left_low != 0 || (right_high & 0x7FFFFFFF) != 0 || right_low != 0;
}

uint32 p2_func_8007e340(uint32 callback)
{
    uint32 previous = r_u32(0x800898C4);
    w_u32(0x800898C4, callback);
    return previous;
}

uint32 p2_func_8006a5e0(uint32 handle, uint16 volume)
{
    FUNCTION_MARKER(0x8006A5E0, "SLUS_005.78");
    sint16 scaled = (sint16)(((uint32)volume * 127) >> 12);
    return p2_sdk_80075d58((uint8)handle, scaled, scaled);
}



void p2_func_8006ab74(void)
{
    FUNCTION_MARKER(0x8006AB74, "SLUS_005.78");
    uint16 handle = r_u16(0x8008A3B0);
    if (handle != 0)
    {
        p2_func_8006ae90(handle);
        w_u16(0x8008A3B0, 0);
    }
}

uint32 p2_sdk_800794a0(sint16 sequence, sint16 bank, sint16 program, uint16 volume, uint16 pan)
{
    uint32 entry = r_u32(0x800B9B00 + ((uint32)sequence & 255) * 4) + (((uint32)sequence & 0xFF00) >> 8) * 172;
    uint32 updated = 0;
    sint32 voice = 0;
    p2_sdk_80079a18((uint16)bank, program);
    w_u16(0x800B7F86, sequence);
    if (pan == 0)
        pan = 1;
    if (volume == 0)
        volume = 1;
    if (r_u8(0x800B7A40) != 0)
    {
        do
        {
            uint32 attributes = 0x800AC1A8 + (uint32)(sint32)(sint16)voice * 52;
            if (r_s16(attributes + 6) == sequence && r_s16(attributes + 10) == program && r_s16(attributes + 14) == bank)
            {
                uint32 channel = entry + 78 + 2 * r_u8(entry + 18);
                sint32 channel_volume = r_s16(channel);
                sint32 scaled;
                uint32 bank_volume;
                uint32 program_offset;
                uint32 tone;
                uint32 level;
                uint32 left;
                uint32 right;
                uint32 balance;
                if (channel_volume != volume && channel_volume == 0)
                    w_u16(channel, 1);
                scaled = r_s16(attributes) * (sint32)volume;
                bank_volume = r_u8(r_u32(0x800B1828) + 24);
                scaled = (sint32)((uint32)(scaled / 127) * (bank_volume * 16383));
                program_offset = r_u32(0x800B1820) + (uint32)(sint32)program * 16;
                level = (uint32)(scaled / 16129) * r_u8(program_offset + 1);
                tone = r_u32(0x800B182C) + ((uint32)(sint32)r_s16(attributes + 8) * 16 + (uint32)(sint32)r_s16(attributes + 12)) * 32;
                level = (level * r_u8(tone + 2)) / 16129;
                left = (level * r_u16(entry + 116)) / 127;
                right = (level * r_u16(entry + 118)) / 127;
                balance = r_u8(tone + 3);
                if (balance < 64)
                    right = (right * balance) / 63;
                else
                    left = (left * (127 - balance)) / 63;
                program_offset = r_u32(0x800B1820) + (uint32)(sint32)r_s16(attributes + 10) * 16;
                balance = r_u8(program_offset + 4);
                if (balance < 64)
                    right = (right * balance) / 63;
                else
                    left = (left * (127 - balance)) / 63;
                balance = (uint8)pan;
                if (balance < 64)
                    right = (right * balance) / 63;
                else
                    left = (left * (127 - balance)) / 63;
                if (r_s16(0x800B16FC) == 1)
                {
                    if (left < right)
                        left = right;
                    else
                        right = left;
                }
                w_u16(0x800B7FA0 + (uint32)(sint32)(sint16)voice * 16, (left * left) / 16383);
                w_u16(0x800B7FA2 + (uint32)(sint32)(sint16)voice * 16, (right * right) / 16383);
                ++updated;
                w_u8(0x800B14F8 + (uint32)(sint32)(sint16)voice, r_u8(0x800B14F8 + (uint32)(sint32)(sint16)voice) | 3);
            }
            ++voice;
        } while ((sint16)voice < r_u8(0x800B7A40));
    }
    return updated;
}

uint32 p2_sdk_80070bc0(uint32 enabled, uint32 voices)
{
    uint32 upper;
    uint32 result;
    uint32 registers;
    voices &= 0xFFFFFF;
    upper = voices >> 16;
    if (enabled != 0 && enabled != 1)
        return 1;
    if ((r_u32(0x80087D54) & 1) != 0)
    {
        uint32 pending = enabled != 0 ? 0x800B16E8 : 0x800B16EC;
        uint32 opposite = enabled != 0 ? 0x800B16EC : 0x800B16E8;
        w_u16(pending, voices);
        w_u16(pending + 2, upper);
        w_u32(0x80087920, r_u32(0x80087920) | 1);
        result = r_u32(0x8008791C);
        w_u32(0x8008791C, enabled != 0 ? result | voices : result & ~voices);
        if ((r_u16(opposite) & voices) != 0)
            w_u16(opposite, r_u16(opposite) & ~voices);
        result = r_u16(opposite + 2) & upper;
        if (result != 0)
        {
            result = r_u16(opposite + 2) & ~upper;
            w_u16(opposite + 2, result);
        }
        return result;
    }
    if (enabled != 0)
    {
        result = r_u32(0x800878F4);
        registers = r_u32(0x80087D5C);
        result |= voices;
        w_u16(registers + 0x188, voices);
        w_u16(registers + 0x18A, upper);
    }
    else
    {
        registers = r_u32(0x80087D5C);
        result = r_u32(0x800878F4);
        w_u16(registers + 0x18C, voices);
        w_u16(registers + 0x18E, upper);
        result &= ~voices;
    }
    w_u32(0x800878F4, result);
    return result;
}

uint32 p2_sdk_80078474(void)
{
    uint32 voice = r_u16(0x800B7F8A);
    uint32 low = voice < 16 ? 1u << (voice & 31) : 0;
    uint32 high = voice < 16 ? 0 : 1u << ((voice - 16) & 31);
    uint32 low_pending;
    uint32 high_pending;
    uint32 result;
    w_u8(0x800AC1BB + voice * 52, 0);
    low_pending = r_u16(0x800BA518);
    high_pending = r_u16(0x800BA51A);
    w_u16(0x800AC1A4 + voice * 52, 0);
    w_u16(0x800AC1A0 + voice * 52, 0);
    result = r_u16(0x800ABD48);
    low_pending |= low;
    w_u16(0x800BA518, low_pending);
    w_u16(0x800ABD48, result & ~low_pending);
    result = r_u16(0x800ABD4A);
    high_pending |= high;
    w_u16(0x800BA51A, high_pending);
    result &= ~high_pending;
    w_u16(0x800ABD4A, result);
    return result;
}

uint32 p2_sdk_80079350(sint16 sequence)
{
    uint32 voice = 0;
    if (r_u8(0x800B7A40) != 0)
    {
        do
        {
            if (r_s16(0x800AC1AE + voice * 52) == sequence)
            {
                w_u16(0x800B7F8A, voice);
                p2_sdk_80078474();
            }
            voice = (uint8)(voice + 1);
        } while (voice < r_u8(0x800B7A40));
    }
    return voice;
}

uint32 p2_sdk_80075818(sint16 sequence, sint16 track)
{
    uint32 table = 0x800B9B00 + (uint32)(sint32)sequence * 4;
    uint32 stride = (uint32)((sint32)track * 172);
    uint32 entry = r_u32(table) + stride;
    uint32 current;
    uint32 saved_ticks;
    uint32 saved_tempo;
    uint16 saved_duration;
    uint32 saved_position;
    uint32 saved_loop;
    uint32 channel;
    w_u32(entry + 144, r_u32(entry + 144) & ~1u);
    current = r_u32(table) + stride;
    w_u32(current + 144, r_u32(current + 144) & ~2u);
    current = r_u32(table) + stride;
    w_u32(current + 144, r_u32(current + 144) & ~8u);
    current = r_u32(table) + stride;
    w_u32(current + 144, r_u32(current + 144) | 4);
    p2_sdk_80079350((sint16)((uint32)(sint32)sequence | ((uint32)(sint32)track << 8)));
    saved_ticks = r_u32(entry + 124);
    saved_tempo = r_u32(entry + 132);
    saved_duration = r_u16(entry + 114);
    saved_position = r_u32(entry + 8);
    saved_loop = r_u32(entry + 8);
    w_u8(entry + 43, 0);
    w_u32(entry + 128, 0);
    w_u8(entry + 39, 0);
    w_u8(entry + 19, 0);
    w_u8(entry + 20, 0);
    w_u8(entry + 41, 0);
    w_u8(entry + 21, 0);
    w_u8(entry + 22, 0);
    w_u8(entry + 42, 0);
    w_u8(entry + 18, 0);
    w_u16(entry + 72, 0);
    w_u8(entry + 39, 0);
    w_u8(entry + 40, 0);
    w_u8(entry + 16, 0);
    w_u8(entry + 17, 0);
    w_u32(entry + 136, saved_ticks);
    w_u32(entry + 140, saved_tempo);
    w_u16(entry + 112, saved_duration);
    w_u32(entry + 4, saved_position);
    w_u32(entry + 12, saved_loop);
    for (channel = 0; channel < 16; ++channel)
    {
        w_u8(entry + 44 + channel, channel);
        w_u8(entry + 23 + channel, 64);
        w_u16(entry + 78 + channel * 2, 127);
    }
    w_u16(entry + 120, 127);
    w_u16(entry + 122, 127);
    return 127;
}

uint32 p2_sdk_80075980(sint32 sequence)
{
    return p2_sdk_80075818((sint16)sequence, 0);
}

uint32 p2_sdk_80076354(sint16 left, sint16 right)
{
    w_u32(0x800B14B0, 6);
    w_u16(0x800B14B8, ((sint32)left * 32767) / 127);
    w_u16(0x800B14BA, ((sint32)right * 32767) / 127);
    return (uint32)p2_sdk_8006ff38(0x800B14B0);
}

uint32 p2_sdk_800763ec(uint32 mode)
{
    uint32 magnitude = (mode & 0x8000) != 0 ? 0u - mode : mode;
    sint16 type;
    if ((uint16)magnitude >= 10)
        return (uint32)-1;
    w_u32(0x800B14B0, 1);
    w_u32(0x800B14B4, (uint32)(sint32)(sint16)(magnitude | ((mode & 0x8000) != 0 ? 0x100 : 0)));
    type = (sint16)magnitude;
    if (type == 0)
        p2_sdk_800764a0(0);
    p2_sdk_8006ff38(0x800B14B0);
    return (uint32)(sint32)type;
}

uint32 p2_sdk_800759d4(uint32 table, sint16 sequences, sint16 separations)
{
    sint32 sequence;
    uint32 position = 0;
    w_u16(0x800BA408, sequences);
    w_u16(0x800BA40A, separations);
    for (sequence = 0; sequence < sequences; ++sequence)
    {
        w_u32(0x800B9B00 + 4 * (uint32)sequence, table + 172 * position);
        position += (uint32)(sint32)separations;
    }
    if (r_s16(0x800BA408) <= 0)
        return (uint32)(sint32)r_s16(0x800BA408);
    sequence = 0;
    do
    {
        sint32 track = 0;
        uint32 offset = 0;
        uint32 slot = 0x800B9B00 + 4 * (uint32)sequence;
        if (r_s16(0x800BA40A) > 0)
        {
            sint32 more;
            do
            {
                uint32 entry;
                w_u32(r_u32(slot) + offset + 144, 0);
                w_u8(r_u32(slot) + offset + 60, 255);
                w_u8(r_u32(slot) + offset, 0);
                w_u16(r_u32(slot) + offset + 62, 0);
                w_u16(r_u32(slot) + offset + 64, 0);
                w_u32(r_u32(slot) + offset + 148, 0);
                w_u32(r_u32(slot) + offset + 152, 0);
                w_u16(r_u32(slot) + offset + 66, 0);
                w_u32(r_u32(slot) + offset + 164, 0);
                w_u32(r_u32(slot) + offset + 160, 0);
                w_u32(r_u32(slot) + offset + 156, 0);
                w_u16(r_u32(slot) + offset + 68, 0);
                w_u16(r_u32(slot) + offset + 116, 127);
                w_u16(r_u32(slot) + offset + 118, 127);
                ++track;
                w_u16(r_u32(slot) + offset + 120, 127);
                entry = r_u32(slot) + offset;
                offset += 172;
                more = track < r_s16(0x800BA40A);
                w_u16(entry + 122, 127);
            } while (more);
        }
        ++sequence;
    } while (sequence < r_s16(0x800BA408));
    return 0;
}

uint32 p2_sdk_80078d1c(sint16 handle, sint16 left, sint16 right, sint16 mode)
{
    uint32 entry = r_u32(0x800B9B00 + ((uint32)handle & 255) * 4);
    sint32 voice = 0;
    w_u16(0x800B7F86, handle);
    entry += (((uint32)handle & 0xFF00) >> 8) * 172;
    if (left == 0)
        left = 1;
    w_u16(entry + 116, left);
    if (right == 0)
        right = 1;
    {
        uint32 valid_left = r_u16(entry + 116) < 128;
        w_u16(entry + 118, right);
        if (!valid_left)
            w_u16(entry + 116, 127);
    }
    if (r_u16(entry + 118) >= 128)
        w_u16(entry + 118, 127);
    if (mode == 1 && r_u8(0x800B7A40) != 0)
    {
        do
        {
            uint32 attributes = 0x800AC1A8 + (uint32)(sint32)(sint16)voice * 52;
            if (r_u16(attributes + 6) == (uint16)handle)
            {
                sint32 scaled = r_s16(attributes) * r_s16(entry + 78 + 2 * r_u8(entry + 18));
                uint32 bank_volume = r_u8(r_u32(0x800B1828) + 24);
                sint32 sequence_volume = scaled / 127;
                uint32 program = r_u32(0x800B1820) + (uint32)(sint32)r_s16(attributes + 10) * 16;
                uint32 tone;
                uint32 level;
                uint32 left_volume;
                uint32 right_volume;
                uint32 pan;
                uint32 square;
                sint32 high;
                scaled = (sint32)(bank_volume * ((uint32)sequence_volume * 16383));
                level = (uint32)(scaled / 16129) * r_u8(program + 1);
                tone = r_u32(0x800B182C) + ((uint32)(sint32)r_s16(attributes + 8) * 16 + (uint32)(sint32)r_s16(attributes + 12)) * 32;
                level = (level * r_u8(tone + 2)) / 16129;
                left_volume = (level * r_u16(entry + 116)) / 127;
                right_volume = (level * r_u16(entry + 118)) / 127;
                pan = r_u8(tone + 3);
                if (pan < 64)
                    right_volume = (right_volume * pan) / 63;
                else
                    left_volume = (left_volume * (127 - pan)) / 63;
                program = r_u32(0x800B1820) + (uint32)(sint32)r_s16(attributes + 10) * 16;
                pan = r_u8(program + 4);
                if (pan < 64)
                    right_volume = ((uint16)right_volume * pan) / 63;
                else
                    left_volume = (uint32)(((sint32)(uint16)left_volume * (127 - (sint32)pan)) / 63);
                pan = r_u8(attributes + 2);
                if (pan < 64)
                    right_volume = ((uint16)right_volume * pan) / 63;
                else
                    left_volume = (uint32)(((sint32)(uint16)left_volume * (127 - (sint32)pan)) / 63);
                if (r_s16(0x800B16FC) == 1)
                {
                    if ((uint16)left_volume < (uint16)right_volume)
                        left_volume = right_volume;
                    else
                        right_volume = left_volume;
                }
                square = (uint32)(uint16)left_volume * (uint16)left_volume;
                high = (sint32)(((sint64)(sint32)square * (sint32)0x80020009u) >> 32);
                left_volume = (uint32)(((sint32)((uint32)high + square) >> 13) - ((sint32)square >> 31));
                w_u16(0x800B7FA0 + (uint32)(sint32)(sint16)voice * 16, left_volume);
                square = (uint32)(uint16)right_volume * (uint16)right_volume;
                high = (sint32)(((sint64)(sint32)square * (sint32)0x80020009u) >> 32);
                right_volume = (uint32)(((sint32)((uint32)high + square) >> 13) - ((sint32)square >> 31));
                w_u16(0x800B7FA2 + (uint32)(sint32)(sint16)voice * 16, right_volume);
                w_u8(0x800B14F8 + (uint32)(sint32)(sint16)voice, r_u8(0x800B14F8 + (uint32)(sint32)(sint16)voice) | 3);
            }
            ++voice;
        } while ((sint16)voice < r_u8(0x800B7A40));
    }
    return (uint32)(sint32)r_s16(0x800B7F86);
}

uint32 p2_sdk_80082a18(const char *format, ...)
{
    va_list arguments;
    int result;
    va_start(arguments, format);
    result = vprintf(format, arguments);
    va_end(arguments);
    return (uint32)result;
}

uint32 p2_sdk_80079adc(uint8 voices)
{
    if (voices == 0 || voices >= 25)
        return 255;
    w_u8(0x800B7A40, voices);
    return r_u8(0x800B7A40);
}

uint32 p2_sdk_80079244(sint16 handle, uint32 left_offset, uint32 right_offset)
{
    uint32 entry = r_u32(0x800B9B00 + ((uint32)handle & 0xFF) * 4);
    w_u16(0x800B7F86, handle);
    entry += (((uint32)handle & 0xFF00) >> 8) * 172;
    w_u16(left_offset, r_u16(entry + 116));
    w_u16(right_offset, r_u16(entry + 118));
    return (uint32)(sint32)r_s16(0x800B7F86);
}

uint32 p2_sdk_80074210(sint16 sequence)
{
    return p2_func_80074138((uint32)(sint32)sequence, 0);
}

uint32 p2_sdk_80074264(sint32 sequence, sint32 mode, sint32 repeat)
{
    return p2_sdk_800743ec((sint16)sequence, 0, (uint8)mode, (sint16)repeat);
}

uint32 p2_sdk_80075d58(sint16 sequence, sint16 left, sint16 right)
{
    return p2_sdk_80078d1c(sequence, left, right, 1);
}

uint32 p2_sdk_800743ec(sint16 sequence, sint16 track, uint8 mode, sint16 repeat)
{
    uint32 table = 0x800B9B00 + (uint32)(sint32)sequence * 4;
    uint32 stride = (uint32)((sint32)track * 172);
    uint32 entry = r_u32(table) + stride;
    uint32 current;
    uint32 result = 1;
    w_u32(entry + 144, r_u32(entry + 144) & ~0x200u);
    current = r_u32(table) + stride;
    w_u32(current + 144, r_u32(current + 144) & ~4u);
    w_u16(entry + 70, repeat);
    if (mode == 1)
    {
        sint16 handle = (sint16)((uint32)(sint32)sequence | ((uint32)(sint32)track << 8));
        uint16 left;
        uint16 right;
        current = r_u32(table) + stride;
        w_u32(current + 144, r_u32(current + 144) | 1);
        left = r_u16(entry + 116);
        right = r_u16(entry + 118);
        w_u16(entry + 72, 0);
        w_u8(entry + 43, 1);
        return p2_sdk_80078d1c(handle, (sint16)left, (sint16)right, 0);
    }
    if (mode == 0)
    {
        current = r_u32(table) + stride;
        result = r_u32(current + 144) | 2;
        w_u32(current + 144, result);
    }
    return result;
}



uint32 p2_func_800378a0(uint32 object_offset)
{
    FUNCTION_MARKER(0x800378A0, "SLUS_005.78");
    uint32 data = r_u32(r_u32(object_offset + 132) + 44);
    w_u16(data + 120, 0);
    w_u16(data + 122, 0);
    w_u8(data + 124, 0xFF);
    w_u16(data + 164, 0);
    w_u8(data + 166, 0);
    return data;
}

uint32 p2_func_800425f4(uint32 allocation_offset)
{
    FUNCTION_MARKER(0x800425F4, "SLUS_005.78");
    if ((sound_effects.IsPlaying(r_u32(allocation_offset + 8), (uint32)(sint32)r_s16(allocation_offset + 6)) & 0xFF) != 0)
        p2_func_8006a970(r_u16(allocation_offset + 6));
    w_u32(allocation_offset + 8, 0);
    w_u16(allocation_offset + 6, 0xFFFF);
    return 0xFFFFFFFF;
}


uint32 p2_func_80055848(uint32 scene)
{
    FUNCTION_MARKER(0x80055848, "SLUS_005.78");
    if (scene == 20)
        return character_controller.vitals.health_pickups & 4;
    if (scene == 75)
        return character_controller.vitals.health_pickups & 1;
    if (scene == 10)
        return character_controller.vitals.health_pickups & 2;
    return 1;
}



uint32 p2_func_800567d0(uint32 object_offset)
{
    FUNCTION_MARKER(0x800567D0, "SLUS_005.78");
    uint32 result = r_u32(object_offset + 228) | 0x1000000;
    w_u32(object_offset + 228, result);
    return result;
}

uint32 p2_func_800567e4(uint32 object_offset)
{
    FUNCTION_MARKER(0x800567E4, "SLUS_005.78");
    uint32 result = r_u32(object_offset + 228) & 0xFEFFFFFF;
    w_u32(object_offset + 228, result);
    return result;
}

uint32 p2_func_80056810(uint32 object_offset, uint32 character_offset)
{
    FUNCTION_MARKER(0x80056810, "SLUS_005.78");
    w_u32(object_offset + 228, r_u32(object_offset + 228) | 0x40000);
    w_u8(character_offset + 252, 6);
    return 6;
}

uint32 p2_func_80058728(uint32 object_offset)
{
    FUNCTION_MARKER(0x80058728, "SLUS_005.78");
    uint32 result = r_u32(object_offset + 228) & 0xFFFFF7F7;
    w_u32(object_offset + 228, result);
    return result;
}

uint32 p2_func_80058c78(uint32 object_offset)
{
    FUNCTION_MARKER(0x80058C78, "SLUS_005.78");
    uint32 result = r_u32(object_offset + 228) & 0xFFFFFFB0;
    w_u32(object_offset + 228, result);
    return result;
}

uint32 p2_func_80058c8c(uint32 object_offset)
{
    FUNCTION_MARKER(0x80058C8C, "SLUS_005.78");
    uint32 result = r_u32(object_offset + 228) & 0xFFFFFFB1;
    w_u32(object_offset + 228, result);
    return result;
}

uint32 p2_func_80059268(uint32 object_offset)
{
    FUNCTION_MARKER(0x80059268, "SLUS_005.78");
    uint32 result = r_u32(object_offset + 228) & 0xFFFFEFBF;
    w_u32(object_offset + 228, result);
    return result;
}

uint32 p2_func_8006a614(void)
{
    FUNCTION_MARKER(0x8006A614, "SLUS_005.78");
    return p2_sdk_80070bc0(0, 0xFF0000);
}

uint32 p2_func_80042024(uint32 object_offset)
{
    FUNCTION_MARKER(0x80042024, "SLUS_005.78");
    uint32 scene = r_u32(0x8008A114), result = scene;
    uint32 remaining = (uint32)(sint32)r_s8(scene + 1);
    uint32 count, cursor, definition = 0, kind, speed, object_id, entry, action, subtype, impulse, flags;
    if (remaining == 0)
        return result;
    count = r_u32(0x8008A338);
    cursor = 0x8008B4A8;
    kind = r_u32(object_offset + 116);
    while (count != 0)
    {
        definition = r_u32(cursor);
        if ((uint32)(sint32)r_s8(definition + 2) == kind)
            break;
        cursor += 4;
        --count;
    }
    if (count == 0)
        definition = 0;
    object_id = r_u32(object_offset + 108) & 0xFFF;
    cursor = r_u32(0x8008A114) + 140;
    speed = r_u32(r_u32(object_offset + 132) + 44) + 12;
    do
    {
        entry = r_u32(cursor);
        cursor += 4;
        result = entry & 0xFFF;
        if (object_id == result)
        {
            action = ((entry >> 12) & 0x1F) + 39;
            subtype = (entry >> 17) & 0xFF;
            if ((r_u8(object_offset + 13) & 1) != 0 && action != 43)
                action = 42;
            result = action - 40 < 15;
            if (result != 0)
            {
                result = r_u32(0x800102CC + (action - 40) * 4);
                switch (action)
                {
                    case 40:
                        result = (uint32)(sint32)r_s8(object_offset + 124);
                        if (result != 3)
                        {
                            result = r_u32(definition + 4);
                            w_u32(speed, result);
                            goto moving;
                        }
                        break;
                    case 42:
                    case 53:
                        result = r_u32(object_offset + 28) | 0x4000;
                        flags = r_u8(object_offset + 13) | 1;
                        w_u32(object_offset + 28, result);
                        w_u8(object_offset + 13, flags);
                        goto stop_motion;
                    case 44:
                    stop_motion:
                        result = (uint32)(sint32)r_s8(object_offset + 124);
                        if (result != 0)
                        {
                            w_u32(object_offset + 100, 0);
                            w_u32(object_offset + 96, 0);
                            w_u32(object_offset + 92, 0);
                            w_u32(speed + 8, 0);
                            w_u32(speed + 4, 0);
                            w_u32(speed, 0);
                            w_u8(object_offset + 124, 0);
                        }
                        break;
                    case 46:
                        impulse = r_u32(definition + 24);
                        result = 11;
                        if (impulse == 0)
                            break;
                        if (subtype == 11)
                        {
                            flags = r_u32(object_offset + 28);
                            result = flags & 0x400;
                            if ((flags & 0x4000) == 0)
                            {
                                if (result == 0)
                                    p2_func_800414d0(object_offset);
                                result = r_u32(speed + 4) + r_u32(definition + 24);
                                w_u32(speed + 4, result);
                            }
                        }
                        else
                        {
                            result = (r_u32(object_offset + 28) & 0x10) != 0 ? r_u32(speed) + impulse : r_u32(speed) - impulse;
                            w_u32(speed, result);
                        }
                        goto moving;
                    case 48:
                        impulse = r_u32(definition + 24);
                        result = 10;
                        if (impulse == 0)
                            break;
                        if (subtype == 10)
                        {
                            result = r_u32(object_offset + 28) & 0x400;
                            if (result != 0)
                            {
                                result = r_u32(speed + 4) - impulse;
                                w_u32(speed + 4, result);
                            }
                        }
                        else
                        {
                            result = (r_u32(object_offset + 28) & 0x10) != 0 ? r_u32(speed) - impulse : r_u32(speed) + impulse;
                            w_u32(speed, result);
                        }
                    moving:
                        w_u8(object_offset + 124, 3);
                        break;
                    case 51:
                        if ((r_u8(object_offset + 13) & 1) == 0)
                        {
                            result = r_u32(object_offset + 28) | 0x4000;
                            flags = r_u8(object_offset + 13) | 1;
                            w_u32(object_offset + 28, result);
                            w_u8(object_offset + 13, flags);
                            break;
                        }
                        goto activate;
                    case 43:
                    case 54:
                    activate:
                        w_u32(object_offset + 28, r_u32(object_offset + 28) | 0x80000);
                        result = r_u8(object_offset + 13);
                        flags = r_u32(object_offset + 28);
                        result &= 0xFE;
                        flags &= 0xFFDFFFFF;
                        w_u8(object_offset + 13, result);
                        w_u32(object_offset + 28, flags);
                        break;
                }
            }
        }
        --remaining;
    } while (remaining != 0);
    return result;
}
