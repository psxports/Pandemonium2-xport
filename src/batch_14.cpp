#include "game.h"

uint32 p2_func_8004483c(uint32 object_offset)
{
    FUNCTION_MARKER(0x8004483C, "SLUS_005.78");
    uint32 saved_stack, vector, global_flags, flags, parent, matrix, x, y, z, value, result;
    global_flags = player_interaction.flags;
    flags = r_u32(object_offset + 28);
    w_u16(object_offset + 128, 2);
    result = 0xFFFFFFFE;
    player_interaction.flags = global_flags & result;
    w_u8(object_offset + 126, 0);
    if ((flags & 0x2000) != 0)
        return result;
    saved_stack = p2_guest_stack_pointer;
    p2_guest_stack_pointer -= 16;
    vector = p2_guest_stack_pointer;
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
        p2_sdk_8007ec64(vector, object_offset + 16);
        PopMatrix();
        matrix = r_u32(r_u32(object_offset + 132) + 40);
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
    result = r_u32(object_offset + 28) | 0x2000;
    w_u32(object_offset + 28, result);
    p2_guest_stack_pointer = saved_stack;
    return result;
}

uint32 p2_func_80044990(uint32 object_offset, uint32 speed_offset)
{
    FUNCTION_MARKER(0x80044990, "SLUS_005.78");
    uint32 saved_stack, vector, kind, flags, buttons, mask, result;
    uint32 value, parent, frame, x, y, z, multiplier, scaled_x;
    sint32 timer, limit, delta;
    saved_stack = p2_guest_stack_pointer;
    p2_guest_stack_pointer -= 16;
    vector = p2_guest_stack_pointer;
    p2_func_800536f0(0);
    kind = r_u32(object_offset + 116);
    timer = r_s16(object_offset + 128);
    limit = (sint16)p2_func_8002d7a4(kind);
    if (timer < limit - 15 && (player_interaction.flags & 0x10) == 0)
    {
        result = p2_func_8004483c(object_offset);
        p2_guest_stack_pointer = saved_stack;
        return result;
    }
    flags = r_u32(object_offset + 28);
    if ((flags & 0x20200) == 0x200 || (sint8)character_controller.motion.state_code == 4 || (character_controller.secondary_flags & 0x240) != 0 || (character_controller.flags & 0x04000000) != 0)
    {
        result = p2_func_8004483c(object_offset);
        p2_guest_stack_pointer = saved_stack;
        return result;
    }
    buttons = gameplay_input.buttons;
    mask = (*control_bindings.active_scheme)[0];
    if ((buttons & mask) == 0)
    {
        result = p2_func_8004483c(object_offset);
        p2_guest_stack_pointer = saved_stack;
        return result;
    }
    value = player_interaction.flags;
    result = buttons & 0x8000;
    if ((value & 0x10) == 0)
    {
        p2_guest_stack_pointer = saved_stack;
        return result;
    }
    delta = 0;
    if ((buttons & 0x8000) != 0)
        delta = (flags & 0x10) != 0 ? -819 : 819;
    else if ((buttons & 0x2000) != 0)
        delta = (flags & 0x10) != 0 ? 819 : -819;
    value = r_u32(speed_offset) + (uint32)delta;
    w_u32(speed_offset, value);
    if ((sint32)value >= 6145)
        w_u32(speed_offset, 6144);
    if (delta != 0)
    {
        value = r_u32(speed_offset);
        if ((sint32)value < 0)
        {
            w_u32(speed_offset, 0 - value);
            flags = r_u32(object_offset + 28);
            parent = r_u32(object_offset + 132);
            w_u32(object_offset + 28, flags ^ 0x10);
            timer = r_s16(parent + 26);
            frame = (uint32)timer - (uint32)(sint32)r_s16(object_offset + 136);
            if ((sint32)frame >= timer)
                frame = (uint32)timer - 1;
            else if ((sint32)frame < 0)
                frame = 0;
            w_u16(object_offset + 136, frame);
        }
        flags = r_u32(object_offset + 28);
        if ((flags & 0x2000) != 0)
        {
            parent = r_u32(object_offset + 132);
            SamplePathTangent(parent, (VECTOR *)psx_addr(vector, 12), (sint32)(flags & 0x10));
            x = r_u32(vector);
            w_u32(vector + 4, 0);
            if (x + 26754 > 0xD103 || r_u32(vector + 8) + 26754 > 0xD103)
            {
                do
                {
                    x = (uint32)(r_s32(vector) >> 1);
                    y = (uint32)(r_s32(vector + 4) >> 1);
                    z = (uint32)(r_s32(vector + 8) >> 1);
                    w_u32(vector, x);
                    w_u32(vector + 4, y);
                    w_u32(vector + 8, z);
                } while (x + 26754 > 0xD103 || y + 26754 > 0xD103 || z + 26754 > 0xD103);
            }
            VectorNormal((VECTOR *)psx_addr(vector, 12), (VECTOR *)psx_addr(vector, 12));
            x = r_u32(vector);
            multiplier = r_u32(speed_offset);
            scaled_x = (uint32)(((sint64)(sint32)x * (sint32)multiplier) >> 12);
            w_u32(vector, scaled_x);
            multiplier = r_u32(speed_offset);
            y = r_u32(vector + 4);
            w_u32(vector + 4, (uint32)(((sint64)(sint32)y * (sint32)multiplier) >> 12));
            multiplier = r_u32(speed_offset);
            z = r_u32(vector + 8);
            w_u32(vector + 8, (uint32)(((sint64)(sint32)z * (sint32)multiplier) >> 12));
            w_u32(object_offset + 92, scaled_x);
            z = r_u32(vector + 8);
            w_u32(object_offset + 100, z);
        }
    }
    buttons = gameplay_input.buttons;
    delta = 0;
    if ((buttons & 0x1000) != 0)
        delta = 819;
    else if ((buttons & 0x4000) != 0)
        delta = -819;
    value = r_u32(object_offset + 96) + (uint32)delta;
    w_u32(object_offset + 96, value);
    result = (sint32)value >= 6145 ? 6144 : (uint32)-6144;
    if ((sint32)value >= 6145 || (sint32)value < -6144)
        w_u32(object_offset + 96, result);
    p2_guest_stack_pointer = saved_stack;
    return result;
}

uint32 p2_func_80074138(uint32 sequence, uint32 track)
{
    FUNCTION_MARKER(0x80074138, "SLUS_005.78");
    uint32 table, stride, entry, result;
    table = 0x800B9B00 + (uint32)(sint32)(sint16)sequence * 4;
    stride = (uint32)(172 * (sint32)(sint16)track);
    entry = r_u32(table) + stride;
    p2_sdk_80079244((sint16)(sequence | (track << 8)), entry + 120, entry + 122);
    entry = r_u32(table) + stride;
    result = r_u32(entry + 144) & 0xFFFFFFFE;
    w_u32(entry + 144, result);
    entry = r_u32(table) + stride;
    result = r_u32(entry + 144) & 0xFFFFFFF7;
    w_u32(entry + 144, result);
    entry = r_u32(table) + stride;
    result = r_u32(entry + 144) | 2;
    w_u32(entry + 144, result);
    return result;
}

uint32 p2_func_80048db4(uint32 object_offset)
{
    FUNCTION_MARKER(0x80048DB4, "SLUS_005.78");
    uint32 value, pointer, scene, x, y, z, fade, animation, frame, buttons, result;
    sint32 step, position, limit;
    if (player_interaction.flags != 0)
    {
        player_interaction.flags = 0;
        player_interaction.target_offset = 0;
    }
    if (character_action_flags != 0)
    {
        pointer = character_action_target_offset;
        character_action_flags = 0;
        if (pointer != 0)
        {
            pointer = r_u32(r_u32(pointer + 132) + 44);
            value = r_u8(pointer + 38);
            w_u8(pointer + 38, value & 0xF7);
        }
    }
    if (((uint16)(character_controller.motion.form_flags >> 16) & 15) == 1)
        character_controller.secondary_flags = character_controller.secondary_flags & 0xFFFFFFDF;
    else if ((game_flow.mode_flags & 0x200) == 0)
        character_controller.secondary_flags = character_controller.secondary_flags & 0xFFFFFEFF;
    scene = r_u32(0x8008A114);
    w_u8(scene + 74, 0);
    w_u8(scene + 73, 0);
    w_u8(scene + 72, 0);
    scene = r_u32(0x8008A114);
    x = r_u32(scene + 4);
    y = r_u32(scene + 8);
    z = r_u32(scene + 12);
    w_u32(scene + 28, x);
    w_u32(scene + 32, y);
    w_u32(scene + 36, z);
    if (r_u32(object_offset + 104) == 0)
    {
        value = character_controller.timers.state_ticks - 1;
        character_controller.timers.state_ticks = value;
        if ((uint16)value != 0)
            return 0xE0000;
        w_u32(0x1F80038C, 0xE9555);
        p2_func_8002417c(0xE9555);
        return p2_func_80048af0(object_offset, 0x800ABD68, 1);
    }
    fade = r_u16(0x8008A532);
    if (fade < 4096)
    {
        fade += 409;
        w_u16(0x8008A532, fade);
        if ((uint16)fade >= 4096)
        {
            w_u16(0x8008A532, 4096);
            p2_func_800260b0();
            p2_func_800401a8(object_offset);
        }
    }
    if ((character_controller.flags & 0x80) != 0)
    {
        if (r_u16(0x8008A532) < 4096)
        {
            value = r_u32(object_offset + 20);
            scene = r_u32(0x8008A114);
            w_u32(object_offset + 20, value - 10240);
            x = r_u32(object_offset + 16);
            y = r_u32(object_offset + 20);
            z = r_u32(object_offset + 24);
            w_u32(scene + 16, x);
            w_u32(scene + 20, y);
            w_u32(scene + 24, z);
            p2_func_80022d38();
            scene = r_u32(0x8008A114);
            w_u8(scene, r_u8(scene) | 1);
        }
        if ((sint16)character_controller.timers.state_ticks != 0)
        {
            step = character_controller.path_velocity.vz;
            if (step > 0)
            {
                position = r_s32(0x1F80038C);
                if (position <= 2457599)
                {
                    value = (uint32)position + (uint32)step;
                    w_u32(0x1F80038C, value);
                    if ((sint32)value > 2457600)
                        w_u32(0x1F80038C, 2457600);
                    p2_func_8002417c(r_s32(0x1F80038C));
                }
                if (r_u32(0x1F80038C) == 2457600)
                    character_controller.path_velocity.vz =  0 - (uint32)character_controller.path_velocity.vz;
            }
            else
            {
                position = r_s32(0x1F80038C);
                if (position > 0)
                {
                    value = (uint32)position + (uint32)step;
                    w_u32(0x1F80038C, value);
                    if ((sint32)value < 0)
                        w_u32(0x1F80038C, 0);
                    p2_func_8002417c(r_s32(0x1F80038C));
                }
            }
            character_controller.timers.state_ticks = character_controller.timers.state_ticks - 1;
            animation = r_u32(object_offset + 104);
            frame = 0;
            if (animation != 0)
            {
                position = r_s16(object_offset + 130);
                limit = r_s16(animation + 6) - 1;
                if (position < limit)
                    frame = (uint32)position + 1;
            }
            w_u16(object_offset + 130, frame);
            return p2_func_8001a6ac(object_offset);
        }
        character_controller.flags = character_controller.flags & 0xFFFFFF7F;
    }
    else
    {
        position = r_s32(0x1F80038C);
        if (position <= 2457599)
        {
            value = (uint32)position + 40960;
            w_u32(0x1F80038C, value);
            if ((sint32)value > 2457600)
                w_u32(0x1F80038C, 2457600);
            p2_func_8002417c(r_s32(0x1F80038C));
        }
        frame = r_u16(object_offset + 130);
        animation = r_u32(object_offset + 104);
        ++frame;
        w_u16(object_offset + 130, frame);
        limit = r_s16(animation + 6);
        if ((sint16)frame < limit)
            return p2_func_8001a6ac(object_offset);
    }
    w_u32(object_offset + 104, 0);
    p2_func_80066430();
    result = p2_func_8006aba0();
    buttons = game_flow.transition_flags;
    w_u16(0x8008A530, result);
    if ((buttons & 0x10000) != 0)
    {
        pointer = r_u32(0x8008A4E4);
        value = r_u16(pointer + 262);
        if (value - 96 >= 2)
        {
            value = character_controller.vitals.lives - 1;
            character_controller.vitals.lives = value;
            if ((uint8)value == 0)
                return p2_func_80048af0(object_offset, 0x800ABD68, 2);
        }
    }
    p2_func_80066404();
    scene = r_u32(0x8008A114);
    character_controller.timers.state_ticks = 15;
    w_u16(scene + 46, (uint16)-15);
    return scene;
}

void p2_func_80025eac(uint32 object_offset)
{
    FUNCTION_MARKER(0x80025EAC, "SLUS_005.78");
    uint32 flags;
    if (r_u8(object_offset + 12) == 7)
    {
        p2_func_8002a2f4(0, object_offset);
        return;
    }
    if (r_s8(object_offset + 127) == 2 || r_s8(object_offset + 124) >= 4)
        p2_func_80034848(r_u32(object_offset + 132));
    if ((r_u32(object_offset + 28) & 4) == 0)
        p2_func_80064d54(object_offset);
    else
    {
        flags = (r_u8(object_offset + 13) | 0x80) & 0xFC;
        w_u8(object_offset + 13, flags);
    }
}

void p2_func_8003ff6c(uint32 node_offset, uint32 destination_offset, uint32 source_offset)
{
    FUNCTION_MARKER(0x8003FF6C, "SLUS_005.78");
    uint32 pointer, child;
    pointer = r_u32(node_offset + 8);
    if (pointer != 0)
        w_u32(node_offset + 8, destination_offset + pointer - source_offset);
    pointer = r_u32(node_offset + 4);
    if (pointer != 0)
        w_u32(node_offset + 4, destination_offset + pointer - source_offset);
    if (r_u8(node_offset + 12) == 7)
    {
        pointer = r_u32(node_offset + 20);
        if (pointer != 0)
        {
            pointer = destination_offset + pointer - source_offset;
            w_u32(node_offset + 20, pointer);
            ResetMaterialAnimationList((pointer) != 0 ? (uint32 *)psx_addr((pointer) - 4, 4) + 1 : 0);
        }
    }
    child = r_u32(node_offset + 8);
    while (child != 0)
    {
        p2_func_8003ff6c(child, destination_offset, source_offset);
        child = r_u32(child + 4);
    }
}

uint32 p2_func_80032248(uint32 object_offset, uint32 target_offset, uint32 origin_offset, uint32 resource_offset)
{
    FUNCTION_MARKER(0x80032248, "SLUS_005.78");
    uint32 saved_stack, world, delta_vector, first_vector, second_vector, direction_vector, target_vector;
    uint32 parent, definition, owner, matrix, x, y, z, a, b, c, value, flags, index, pointer;
    uint32 direction, animation, selection, slot, minimum, frequency_zero, random, divisor, pitch, scale, ratio, radius, option, result;
    sint32 angle, step, frame, next_frame, end_frame, mode, limit, voice;
    saved_stack = p2_guest_stack_pointer;
    p2_guest_stack_pointer -= 96;
    world = p2_guest_stack_pointer;
    delta_vector = world + 16;
    first_vector = world + 32;
    second_vector = world + 48;
    direction_vector = world + 64;
    target_vector = world + 80;
    parent = r_u32(object_offset + 132);
    definition = r_u32(parent + 44);
    if (parent != 0 && (r_u32(parent + 16) & 8) != 0 && r_u32(definition + 56) != 0)
    {
        if (r_u32(object_offset + 92) != 0 || r_u32(object_offset + 96) != 0 || r_u32(object_offset + 100) != 0)
        {
            x = r_u32(object_offset + 92);
            y = r_u32(object_offset + 96);
            z = r_u32(object_offset + 100);
            w_u32(definition + 196, x);
            w_u32(definition + 200, y);
            w_u32(definition + 204, z);
        }
        result = p2_func_80018b40(object_offset, definition + 196, 0);
        w_u16(object_offset + 154, result);
        angle = (sint16)result;
        w_u32(definition + 172, angle != 0 && angle != 4096 ? (uint32)angle : 0);
    }
    else
        w_u32(definition + 172, 0);
    if (r_s16(resource_offset + 652) != 0 && (r_u32(object_offset + 28) & 0x02000000) == 0)
    {
        direction = definition + 92;
        step = 4096;
        goto apply_direction;
    }
    if ((r_u16(definition + 120) & 0x1000) != 0)
    {
        owner = GetCharacterModelObject();
        parent = r_u32(owner + 132);
        if (parent != 0 && r_u32(parent + 40) != 0)
        {
            x = r_u32(owner + 16);
            y = r_u32(owner + 20);
            z = r_u32(owner + 24);
            w_u32(first_vector, x);
            w_u32(first_vector + 4, y);
            w_u32(first_vector + 8, z);
            PushMatrix();
            matrix = r_u32(r_u32(owner + 132) + 40);
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
            p2_sdk_8007ec64(first_vector, world);
            PopMatrix();
            for (index = 0; index < 3; ++index)
            {
                matrix = r_u32(r_u32(owner + 132) + 40);
                value = r_u32(world + index * 4);
                x = r_u32(matrix + 20 + index * 4);
                w_u32(world + index * 4, value + x);
            }
        }
        else
        {
            x = r_u32(owner + 16);
            y = r_u32(owner + 20);
            z = r_u32(owner + 24);
            w_u32(world, x);
            w_u32(world + 4, y);
            w_u32(world + 8, z);
        }
        for (index = 0; index < 3; ++index)
        {
            value = r_u32(world + index * 4);
            x = r_u32(origin_offset + index * 4);
            w_u32(delta_vector + index * 4, value - x);
        }
        direction = delta_vector;
        step = 4096;
        goto apply_direction;
    }
    if ((r_u16(definition + 122) & 0x800) != 0 && (r_u32(object_offset + 92) != 0 || r_u32(object_offset + 96) != 0 || r_u32(object_offset + 100) != 0))
    {
        p2_func_80037f08(object_offset);
        goto after_direction;
    }
    mode = r_s8(object_offset + 125);
    if ((object_operations.behaviors[mode].value_handler == p2_func_80020c6c))
    {
        w_u16(resource_offset + 68, 4096);
        direction = object_offset + 92;
        step = 4096;
        goto apply_direction;
    }
    mode = r_s8(definition + 180);
    if (mode != 0)
    {
        if (mode == 1)
        {
            step = r_s16(resource_offset + 68);
            direction = definition + 108;
        }
        else
        {
            x = r_u32(definition + 108);
            y = r_u32(definition + 112);
            z = r_u32(definition + 116);
            w_u32(first_vector, x);
            w_u32(first_vector + 4, y);
            w_u32(first_vector + 8, z);
            x = r_u32(first_vector);
            z = r_u32(first_vector + 8);
            w_u32(first_vector, 0 - x);
            w_u32(first_vector + 8, 0 - z);
            step = r_s16(resource_offset + 68);
            direction = first_vector;
        }
        goto apply_direction;
    }
    if ((r_u16(definition + 120) & 0x100) != 0 && r_u32(definition + 168) == 0)
    {
        parent = r_u32(object_offset + 132);
        if (r_u32(parent + 32) != 0)
        {
            flags = r_u32(object_offset + 28);
            frame = r_s16(object_offset + 136);
            if ((flags & 0x10) != 0)
            {
                next_frame = frame + 1;
                if (frame >= r_s16(parent + 26))
                {
                    next_frame = frame;
                    --frame;
                }
            }
            else
            {
                next_frame = frame - 1;
                if (frame <= 0)
                {
                    next_frame = frame;
                    ++frame;
                }
            }
            parent = r_u32(object_offset + 132);
            result = (uint32)p2_func_8005642c(frame, next_frame, parent);
            parent = r_u32(object_offset + 132);
            w_u16(object_offset + 150, result);
            p2_func_80056340(parent, frame, first_vector, 64);
            p2_func_80056340(r_u32(object_offset + 132), next_frame, second_vector, 64);
            x = r_u32(second_vector);
            a = r_u32(first_vector);
            y = r_u32(second_vector + 4);
            b = r_u32(first_vector + 4);
            z = r_u32(second_vector + 8);
            c = r_u32(first_vector + 8);
            w_u32(direction_vector, x - a);
            w_u32(direction_vector + 4, y - b);
            w_u32(direction_vector + 8, z - c);
            step = r_s16(resource_offset + 68);
            direction = direction_vector;
            goto apply_direction;
        }
    }
    if ((r_u32(object_offset + 28) & 0x40) != 0 && (r_u32(object_offset + 92) != 0 || r_u32(object_offset + 96) != 0 || r_u32(object_offset + 100) != 0))
    {
        step = r_s16(resource_offset + 68);
        direction = object_offset + 92;
        goto apply_direction;
    }
    w_u32(definition + 16, 0);
    if (target_offset == 0)
        goto after_direction;
    parent = r_u32(object_offset + 132);
    if ((r_u32(parent + 16) & 0x40) == 0)
    {
        mode = r_s8(resource_offset + 633);
        if (mode == -1)
        {
            if (r_s8(definition + 37) != 19)
                goto after_direction;
        }
        else if (mode != 19)
            goto after_direction;
    }
    for (index = 0; index < 3; ++index)
    {
        value = r_u32(target_offset + index * 4);
        x = r_u32(origin_offset + index * 4);
        w_u32(target_vector + index * 4, value - x);
    }
    step = r_s16(resource_offset + 68);
    direction = target_vector;
apply_direction:
    p2_func_80040384(object_offset, direction, (sint16)step);
after_direction:
    flags = r_u8(definition + 38);
    if ((flags & 2) != 0)
    {
        if (r_s32(definition + 76) > 0)
        {
            frame = r_s16(object_offset + 130);
            if (frame != -1)
            {
                animation = r_u32(object_offset + 104);
                value = (uint32)frame + 2;
                w_u16(object_offset + 130, value);
                end_frame = r_s16(animation + 6);
                if ((sint16)value >= end_frame)
                    w_u16(object_offset + 130, (uint32)end_frame - 1);
            }
        }
        else
        {
            frame = r_s16(object_offset + 130);
            if (frame != 0)
                w_u16(object_offset + 130, (uint32)frame - 1);
        }
        goto finish;
    }
    if ((flags & 8) != 0)
    {
        if (r_s16(resource_offset + 654) == -1)
        {
            frame = r_s16(object_offset + 130);
            if (frame != -1)
            {
                animation = r_u32(object_offset + 104);
                value = (uint32)frame + 2;
                w_u16(object_offset + 130, value);
                end_frame = r_s16(animation + 6);
                if ((sint16)value >= end_frame)
                    w_u16(object_offset + 130, (uint32)end_frame - 1);
            }
        }
        else
            w_u16(object_offset + 130, r_u16(resource_offset + 656));
        mode = r_s8(0x8008A50D);
        if (mode < 0)
            mode = -mode;
        index = (uint32)(mode % 16);
        frame = r_s16(resource_offset + 646);
        value = r_u16(0x8008A484);
        limit = 1;
        if (frame != 0 && (character_action_flags & 0x1000) == 0)
            limit = 10;
        x = r_u32(definition + 80) + 1;
        w_u32(definition + 80, x);
        if ((sint32)x >= limit)
        {
            w_u32(definition + 80, 0);
            x = r_u32(0x80083BAC + index * 4);
            value = (value + (uint32)((sint32)(x << 16) >> 19)) & 0xFFF;
        }
        w_u32(first_vector + 4, 0);
        x = (uint32)SinQ14((sint16)value);
        w_u32(first_vector, 0 - (uint32)((sint32)x >> 2));
        z = (uint32)SinQ14((sint16)(1024 - value));
        w_u32(first_vector + 8, 0 - (uint32)((sint32)z >> 2));
        p2_func_80040384(object_offset, first_vector, 4096);
        goto finish;
    }
    if (r_s8(resource_offset + 61) >= 47)
        goto finish;
    if (r_u32(resource_offset + 128) == 0)
        goto advance_frame;
    frequency_zero = r_u16(resource_offset + 134) == 0;
    frame = r_s16(object_offset + 130);
    selection = 0;
    if (frame == -1 && frequency_zero != 0)
    {
        pointer = r_u32(definition + 68);
        voice = r_s16(definition + 66);
        if ((sound_effects.IsPlaying(pointer, (uint32)voice) & 0xFF) != 0)
            p2_func_8006a970(r_u16(definition + 66));
        w_u32(definition + 68, 0);
        w_u16(definition + 66, 0xFFFF);
    }
    mode = r_s8(resource_offset + 61);
    if (mode == 17)
    {
        animation = r_u32(object_offset + 104);
        pointer = r_u32(resource_offset + 76);
        if (animation == pointer)
        {
            if (r_s16(object_offset + 130) == 19)
            {
                pointer = r_u32(resource_offset + 128);
                if (pointer != 0 && r_s32(pointer) >= 3)
                    selection = pointer + 20;
                w_u16(definition + 66, 0xFFFF);
                goto sound_selected;
            }
            animation = r_u32(object_offset + 104);
        }
    }
    else
        animation = r_u32(object_offset + 104);
    if (animation == r_u32(resource_offset + 72))
    {
        slot = 4;
        minimum = 1;
        goto random_sound_gate;
    }
    if (animation == r_u32(resource_offset + 80) && r_s8(resource_offset + 61) != 17)
    {
        slot = 20;
        minimum = 3;
        goto random_sound_gate;
    }
    if (r_s16(object_offset + 130) > 0)
        goto sound_selected;
    animation = r_u32(object_offset + 104);
    pointer = r_u32(resource_offset + 84);
    if (animation != pointer)
        goto sound_selected;
    slot = 12;
    minimum = 2;
    goto lookup_sound;
random_sound_gate:
    voice = r_s16(definition + 66);
    if (voice < 0 && frequency_zero == 0)
    {
        random = random_generator.Next();
        divisor = r_u16(resource_offset + 134);
        if (divisor == 0)
            xport_mips_break(7);
        if (random % divisor != 0)
            goto sound_selected;
    }
lookup_sound:
    pointer = r_u32(resource_offset + 128);
    if (pointer != 0 && r_s32(pointer) >= (sint32)minimum)
        selection = pointer + slot;
sound_selected:
    if (selection == 0)
        goto advance_frame;
    pointer = r_u32(definition + 68);
    option = 0;
    if (selection != pointer && pointer != 0)
    {
        voice = r_s16(definition + 66);
        if ((sound_effects.IsPlaying(pointer, (uint32)voice) & 0xFF) != 0)
            p2_func_8006a970(r_u16(definition + 66));
        w_u32(definition + 68, 0);
        w_u16(definition + 66, 0xFFFF);
    }
    voice = r_s16(definition + 66);
    scale = 4096;
    if (voice >= 0)
        pitch = r_u16(definition + 64);
    else
    {
        pitch = r_u16(selection + 4);
        if (pitch == 0)
        {
            mode = r_s8(definition + 37);
            p2_sdk_80082a18((const char *)psx_addr(0x80010194, sizeof("Monster type %d bad SFX rate.\n")), mode);
            goto advance_frame;
        }
        random = random_generator.Next();
        divisor = pitch >> 2;
        if (divisor == 0)
            xport_mips_break(7);
        pitch = pitch + random % divisor - (pitch >> 3);
        w_u16(definition + 64, pitch);
    }
    if ((r_u32(object_offset + 28) & 0x30000000) != 0)
    {
        value = r_u16(object_offset + 116);
        if ((sint32)value < 2048)
            value = 2048;
        scale = (uint32)(((sint64)(sint32)scale * (sint32)value) >> 12);
        if (value == 0)
            xport_mips_break(7);
        ratio = (uint32)(0x01000000 / (sint32)value);
        pitch = (uint32)(((sint64)(sint32)pitch * (sint32)ratio) >> 12);
    }
    if (frequency_zero != 0)
        option = 1;
    w_u32(definition + 68, selection);
    pointer = r_u32(definition + 72);
    radius = (uint32)(sint32)r_s16(pointer + 132) << 5;
    voice = r_s16(definition + 66);
    result = p2_func_8005eb18(selection, (sint32)(scale + 4096), 0, pitch, object_offset + 60, radius, (uint32)voice, frequency_zero, option);
    w_u16(definition + 66, result);
advance_frame:
    animation = r_u32(object_offset + 104);
    value = 0;
    if (animation != 0)
    {
        frame = r_s16(object_offset + 130);
        end_frame = r_s16(animation + 6) - 1;
        if (frame < end_frame)
            value = (uint32)frame + 1;
    }
    w_u16(object_offset + 130, value);
finish:
    result = p2_func_8001a6ac(object_offset);
    p2_guest_stack_pointer = saved_stack;
    return result;
}

uint32 p2_func_8002fd54(uint32 object_offset)
{
    FUNCTION_MARKER(0x8002FD54, "SLUS_005.78");
    uint32 saved_stack, direction, velocity, translation, matrix_local, owner, parent, matrix, index;
    uint32 flags, value, x, y, z, a, b, c, factor, scaled_x, scaled_y, scaled_z, squared, magnitude;
    uint32 contact, support, result;
    sint32 speed, state, timer;
    saved_stack = p2_guest_stack_pointer;
    p2_guest_stack_pointer -= 80;
    direction = p2_guest_stack_pointer;
    velocity = direction + 16;
    translation = direction + 32;
    matrix_local = direction + 48;
    if (r_u32(object_offset + 116) == 19 && (player_interaction.flags & 3) != 3)
    {
        owner = p2_func_800536f0(0);
        flags = player_interaction.flags;
        speed = 40960;
        if ((flags & 0x24) == 0x24)
        {
            result = r_u8(object_offset + 13) | 1;
            w_u8(object_offset + 13, result);
            goto finish;
        }
        w_u8(object_offset + 13, r_u8(object_offset + 13) & 0xFE);
        flags = r_u32(owner + 28);
        if ((flags & 0x2000) == 0 && (parent = r_u32(owner + 132)) != 0 && r_u32(parent + 40) != 0)
        {
            x = r_u32(owner + 16);
            y = r_u32(owner + 20);
            z = r_u32(owner + 24);
            w_u32(velocity, x);
            w_u32(velocity + 4, y);
            w_u32(velocity + 8, z);
            PushMatrix();
            matrix = r_u32(r_u32(owner + 132) + 40);
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
            p2_sdk_8007ec64(velocity, object_offset + 92);
            PopMatrix();
            for (index = 0; index < 3; ++index)
            {
                matrix = r_u32(r_u32(owner + 132) + 40);
                value = r_u32(object_offset + 92 + index * 4);
                x = r_u32(matrix + 20 + index * 4);
                w_u32(object_offset + 92 + index * 4, value + x);
            }
        }
        else
        {
            x = r_u32(owner + 16);
            y = r_u32(owner + 20);
            z = r_u32(owner + 24);
            w_u32(object_offset + 92, x);
            w_u32(object_offset + 96, y);
            w_u32(object_offset + 100, z);
        }
        w_u32(object_offset + 96, r_u32(object_offset + 96) + 12288);
        x = r_u32(object_offset + 92);
        a = r_u32(object_offset + 16);
        y = r_u32(object_offset + 96);
        b = r_u32(object_offset + 20);
        z = r_u32(object_offset + 100);
        c = r_u32(object_offset + 24);
        x -= a;
        y -= b;
        z -= c;
        w_u32(object_offset + 92, x);
        w_u32(object_offset + 96, y);
        w_u32(object_offset + 100, z);
        x = (uint32)(r_s32(object_offset + 92) >> 6);
        y = (uint32)(r_s32(object_offset + 96) >> 6);
        z = (uint32)((sint32)z >> 6);
        squared = x * x + y * y + z * z;
        magnitude = (sint32)squared >= 0 ? p2_sdk_8007e794(squared) : p2_sdk_8007e794(squared >> 2) << 1;
        if ((player_interaction.flags & 0x40) != 0)
            speed *= 16;
        if ((sint32)magnitude < speed + speed / 4)
            w_u16(object_offset + 128, 1);
        if (r_u32(object_offset + 92) + 26754 > 0xD103 || r_u32(object_offset + 96) + 26754 > 0xD103 || r_u32(object_offset + 100) + 26754 > 0xD103)
        {
            for (;;)
            {
                x = r_u32(object_offset + 92);
                z = r_u32(object_offset + 100);
                w_u32(object_offset + 92, (uint32)((sint32)x >> 1));
                y = r_u32(object_offset + 96);
                x = r_u32(object_offset + 92);
                z = (uint32)((sint32)z >> 1);
                w_u32(object_offset + 100, z);
                w_u32(object_offset + 96, (uint32)((sint32)y >> 1));
                if (x + 26754 > 0xD103)
                    continue;
                if (r_u32(object_offset + 96) + 26754 > 0xD103 || z + 26754 > 0xD103)
                    continue;
                break;
            }
        }
        VectorNormal((VECTOR *)psx_addr(object_offset + 92, 12), (VECTOR *)psx_addr(object_offset + 92, 12));
        x = r_u32(object_offset + 92);
        scaled_x = (uint32)(((sint64)(sint32)x * speed) >> 12);
        y = r_u32(object_offset + 96);
        w_u32(object_offset + 92, scaled_x);
        scaled_y = (uint32)(((sint64)(sint32)y * speed) >> 12);
        z = r_u32(object_offset + 100);
        w_u32(object_offset + 96, scaled_y);
        w_u32(object_offset + 100, (uint32)(((sint64)(sint32)z * speed) >> 12));
        result = (uint32)p2_func_800185c8(object_offset, object_offset + 92, 0);
        w_u16(object_offset + 154, result);
        w_u16(object_offset + 86, result & 0xFFF);
        result = p2_func_80018144(object_offset, object_offset + 92, 0);
        w_u16(object_offset + 150, result);
        w_u16(object_offset + 88, (result + 2048) & 0xFFF);
        x = r_u32(object_offset + 92);
        y = r_u32(object_offset + 96);
        z = r_u32(object_offset + 100);
        w_u32(direction, x);
        w_u32(direction + 4, y);
        w_u32(direction + 8, z);
        x = r_u32(direction);
        if (x == 0 && r_u32(direction + 8) == 0)
            magnitude = 0;
        else
        {
            x = (uint32)((sint32)x >> 6);
            y = (uint32)(r_s32(direction + 4) >> 6);
            z = (uint32)(r_s32(direction + 8) >> 6);
            squared = x * x + y * y + z * z;
            magnitude = (sint32)squared >= 0 ? p2_sdk_8007e794(squared) : p2_sdk_8007e794(squared >> 2) << 1;
        }
        p2_func_8003db7c(object_offset, (sint32)magnitude, r_s32(object_offset + 96), (const VECTOR *)psx_addr(direction, 12));
        for (index = 0; index < 3; ++index)
        {
            value = r_u32(object_offset + 16 + index * 4);
            x = r_u32(direction + index * 4);
            result = value + x;
            w_u32(object_offset + 16 + index * 4, result);
        }
        goto finish;
    }
    if (r_u32(object_offset + 144) != 0 && (r_u32(object_offset + 92) != 0 || r_u32(object_offset + 96) != 0 || r_u32(object_offset + 100) != 0))
    {
        x = r_u32(object_offset + 92);
        y = r_u32(object_offset + 96);
        z = r_u32(object_offset + 100);
        w_u32(velocity, x);
        w_u32(velocity + 4, y);
        w_u32(velocity + 8, z);
        if (r_s32(object_offset + 144) < 4096)
        {
            w_u32(translation + 8, 0);
            w_u32(translation + 4, 0);
            w_u32(translation, 0);
            flags = r_u32(object_offset + 28);
            w_u32(object_offset + 28, flags & 0xFFFFF7FF);
            if ((flags & 0x6000) == 0)
            {
                parent = r_u32(object_offset + 132);
                if (parent != 0 && (matrix = r_u32(parent + 40)) != 0)
                {
                    x = r_u32(matrix + 32);
                    y = r_u32(matrix + 36);
                    z = r_u32(matrix + 40);
                    w_u32(translation, x);
                    w_u32(translation + 4, y);
                    w_u32(translation + 8, z);
                }
            }
            x = r_u32(velocity);
            a = r_u32(translation);
            y = r_u32(velocity + 4);
            b = r_u32(translation + 4);
            z = r_u32(velocity + 8);
            c = r_u32(translation + 8);
            x += a;
            y += b;
            z += c;
            w_u32(velocity, x);
            w_u32(velocity + 4, y);
            w_u32(velocity + 8, z);
            factor = r_u32(object_offset + 144);
            scaled_x = (uint32)(((sint64)(sint32)x * (sint32)factor) >> 12);
            w_u32(velocity, scaled_x);
            scaled_y = (uint32)(((sint64)(sint32)y * (sint32)factor) >> 12);
            w_u32(velocity + 4, scaled_y);
            scaled_z = (uint32)(((sint64)(sint32)z * (sint32)factor) >> 12);
            w_u32(velocity + 8, scaled_z);
            w_u32(velocity, scaled_x - a);
            w_u32(velocity + 4, scaled_y - b);
            w_u32(velocity + 8, scaled_z - c);
            if (r_u32(object_offset + 116) == 19)
                w_u16(object_offset + 128, 0);
        }
        if ((r_u32(object_offset + 28) & 0x2000) == 0)
        {
            parent = r_u32(object_offset + 132);
            if (parent != 0 && (matrix = r_u32(parent + 40)) != 0)
            {
                for (index = 0; index < 9; ++index)
                {
                    if (index != 0)
                        matrix = r_u32(r_u32(object_offset + 132) + 40);
                    value = r_u16(matrix + (index % 3) * 6 + (index / 3) * 2);
                    w_u16(matrix_local + index * 2, value);
                }
                x = r_u32(velocity);
                y = r_u32(velocity + 4);
                z = r_u32(velocity + 8);
                w_u32(translation, x);
                w_u32(translation + 4, y);
                w_u32(translation + 8, z);
                PushMatrix();
                x = r_u32(matrix_local);
                y = r_u32(matrix_local + 4);
                xport_gte_write_control(0, x);
                xport_gte_write_control(1, y);
                x = r_u32(matrix_local + 8);
                y = r_u32(matrix_local + 12);
                z = r_u16(matrix_local + 16);
                xport_gte_write_control(2, x);
                xport_gte_write_control(3, y);
                xport_gte_write_control(4, z);
                p2_sdk_8007ec64(translation, velocity);
                PopMatrix();
            }
        }
        for (index = 0; index < 3; ++index)
        {
            value = r_u32(object_offset + 16 + index * 4);
            x = r_u32(velocity + index * 4);
            w_u32(object_offset + 16 + index * 4, value + x);
        }
    }
    contact = r_u32(object_offset + 184);
    if (contact != 0)
    {
        value = r_u32(object_offset + 204);
        factor = r_u32(object_offset + 144);
        if ((sint32)factor >= (sint32)value)
        {
            if (r_s8(contact + 15) == 1)
            {
                support = p2_func_8004e118(object_offset, contact);
                if (support != 0)
                {
                    ApplyPathVelocityChange(object_offset, r_u32(support + 36));
                    w_u32(object_offset + 112, support);
                }
            }
            else
                w_u8(contact + 14, r_u8(contact + 14) | 8);
        }
    }
    contact = r_u32(object_offset + 192);
    if (contact != 0)
    {
        value = r_u32(object_offset + 196);
        factor = r_u32(object_offset + 144);
        if ((sint32)factor >= (sint32)value)
            w_u8(contact + 14, r_u8(contact + 14) | 8);
    }
    contact = r_u32(object_offset + 200);
    if (contact != 0)
    {
        value = r_u32(object_offset + 220);
        factor = r_u32(object_offset + 144);
        if ((sint32)factor >= (sint32)value)
            w_u8(contact + 14, r_u8(contact + 14) | 8);
    }
    result = r_u32(object_offset + 176);
    if (result != 0)
    {
        w_u16(object_offset + 128, 0);
        goto clear_contacts;
    }
    flags = r_u32(object_offset + 28);
    result = flags & 0x800;
    if ((flags & 0x4000) != 0)
    {
        w_u16(object_offset + 128, 0);
        goto clear_contacts;
    }
    if (result == 0)
        goto clear_contacts;
    state = r_s8(object_offset + 120);
    w_u32(object_offset + 28, flags & 0xFFFFF7FF);
    if (state == -2)
    {
        timer = r_s16(object_offset + 128);
        result = (uint32)timer;
        if (timer <= 0)
        {
            w_u16(object_offset + 128, 0);
            goto clear_contacts;
        }
    }
    parent = r_u32(object_offset + 132);
    if (parent != 0 && r_u32(parent + 40) != 0)
    {
        x = r_u32(object_offset + 16);
        y = r_u32(object_offset + 20);
        z = r_u32(object_offset + 24);
        w_u32(translation, x);
        w_u32(translation + 4, y);
        w_u32(translation + 8, z);
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
        p2_sdk_8007ec64(translation, object_offset + 16);
        PopMatrix();
        matrix = r_u32(r_u32(object_offset + 132) + 40);
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
    parent = r_u32(object_offset + 132);
    if (parent != 0 && (matrix = r_u32(parent + 40)) != 0)
    {
        x = r_u32(object_offset + 92);
        value = r_u32(matrix + 32);
        parent = r_u32(object_offset + 132);
        w_u32(object_offset + 92, x + value);
        matrix = r_u32(parent + 40);
        y = r_u32(object_offset + 96);
        value = r_u32(matrix + 36);
        parent = r_u32(object_offset + 132);
        w_u32(object_offset + 96, y + value);
        matrix = r_u32(parent + 40);
        z = r_u32(object_offset + 100);
        value = r_u32(matrix + 40);
        w_u32(object_offset + 100, z + value);
    }
    flags = r_u32(object_offset + 28);
    value = r_u32(object_offset + 116);
    w_u32(object_offset + 28, flags | 0x2000);
    if (value != 19)
        w_u8(object_offset + 124, 0);
    result = r_u32(object_offset + 28) & 0x400;
    if (result != 0)
        goto clear_contacts;
    result = r_u32(r_u32(object_offset + 132) + 16) & 8;
    if (result == 0)
        goto clear_contacts;
    if (r_u32(object_offset + 92) + 26754 > 0xD103 || r_u32(object_offset + 96) + 26754 > 0xD103 || r_u32(object_offset + 100) + 26754 > 0xD103)
    {
        for (;;)
        {
            x = r_u32(object_offset + 92);
            z = r_u32(object_offset + 100);
            w_u32(object_offset + 92, (uint32)((sint32)x >> 1));
            y = r_u32(object_offset + 96);
            x = r_u32(object_offset + 92);
            z = (uint32)((sint32)z >> 1);
            w_u32(object_offset + 100, z);
            w_u32(object_offset + 96, (uint32)((sint32)y >> 1));
            if (x + 26754 > 0xD103)
                continue;
            if (r_u32(object_offset + 96) + 26754 > 0xD103 || z + 26754 > 0xD103)
                continue;
            break;
        }
    }
    VectorNormal((VECTOR *)psx_addr(object_offset + 92, 12), (VECTOR *)psx_addr(object_offset + 92, 12));
    x = r_u32(object_offset + 92);
    factor = r_u32(object_offset + 116);
    scaled_x = (uint32)(((sint64)(sint32)x * (sint32)factor) >> 12);
    factor = r_u32(object_offset + 116);
    y = r_u32(object_offset + 96);
    w_u32(object_offset + 92, scaled_x);
    scaled_y = (uint32)(((sint64)(sint32)y * (sint32)factor) >> 12);
    factor = r_u32(object_offset + 116);
    result = r_u32(object_offset + 100);
    w_u32(object_offset + 96, scaled_y);
    w_u32(object_offset + 100, (uint32)(((sint64)(sint32)result * (sint32)factor) >> 12));
clear_contacts:
    w_u32(object_offset + 200, 0);
    w_u32(object_offset + 192, 0);
    w_u32(object_offset + 184, 0);
    w_u32(object_offset + 176, 0);
finish:
    p2_guest_stack_pointer = saved_stack;
    return result;
}

uint32 p2_func_8002c608(uint32 object_offset)
{
    FUNCTION_MARKER(0x8002C608, "SLUS_005.78");
    uint32 saved_stack, vector, attenuation, pan, ending_attenuation, ending_pan;
    uint32 count, table, kind, definition, candidate, spawned, phase, spawn_kind, frame_action;
    uint32 flags, value, pointer, scene, x, y, z, a, b, c, squared, magnitude, factor, heading, pitch, sound, radius, matrix;
    sint32 next_frame, frame, end_frame, divisor, quotient, angle;
    count = r_u32(0x8008A320);
    spawned = 0;
    next_frame = -1;
    table = 0x8008B420;
    kind = r_u32(object_offset + 116);
    definition = 0;
    while (count != 0)
    {
        --count;
        candidate = r_u32(table);
        if (candidate != 0 && r_u8(candidate + 1) == kind)
        {
            definition = candidate;
            break;
        }
        table += 4;
    }
    saved_stack = p2_guest_stack_pointer;
    p2_guest_stack_pointer -= 32;
    vector = p2_guest_stack_pointer;
    attenuation = vector + 16;
    pan = vector + 20;
    ending_attenuation = vector + 24;
    ending_pan = vector + 28;
    flags = r_u32(object_offset + 28);
    if ((flags & 0x100) != 0)
    {
        if (r_s8(object_offset + 125) == 6)
        {
            p2_func_800536f0(0);
            scene = r_u32(0x8008A114);
            for (kind = 0; kind < 3; ++kind)
            {
                x = r_u32(scene + 16 + kind * 4);
                y = r_u32(object_offset + 60 + kind * 4);
                value = x - y;
                w_u32(vector + kind * 4, value);
                if (kind == 0)
                    a = value;
                else if (kind == 1)
                    b = value;
                else
                    c = value;
            }
            if ((r_u8(definition) & 4) == 0)
            {
                x = (uint32)((sint32)a >> 6);
                y = (uint32)((sint32)b >> 6);
                z = (uint32)((sint32)c >> 6);
                squared = x * x + y * y + z * z;
                if ((sint32)squared > 163839999)
                    goto end_phase;
            }
        }
        candidate = r_u32(definition + 8);
        spawn_kind = 0;
        goto spawn_template;
    }
    if (r_s8(object_offset + 125) != 6)
        goto apply_phase;
    if ((sint32)flags < 0)
    {
        frame_action = 0;
        goto calculate_frame;
    }
    if ((flags & 0x20000000) != 0)
    {
        frame_action = 1;
        goto calculate_frame;
    }
    if ((flags & 0x40000000) == 0)
        goto apply_phase;
    p2_func_8002fd54(object_offset);
    frame = r_s16(object_offset + 128);
    if (frame == 0)
        goto select_ending;
    if (frame > 0)
    {
        value = (uint32)frame - 1;
        w_u16(object_offset + 128, value);
        if ((uint16)value == 0)
        {
            if (r_u8(definition + 1) != 19)
                goto end_phase;
            if ((player_interaction.flags & 3) == 2)
            {
                player_interaction.flags = 0;
                player_interaction.target_offset = 0;
                goto end_phase;
            }
            p2_func_8004483c(object_offset);
            goto apply_phase;
        }
    }
    if (r_u32(definition + 40) != 0)
    {
        pointer = r_u32(definition + 4);
        sound = 0;
        if (pointer != 0 && r_s32(pointer) >= 3)
            sound = pointer + 20;
        pitch = r_u16(sound + 4);
        radius = r_u32(definition + 40);
        frame = r_s16(object_offset + 138);
        value = p2_func_8005eb18(sound, 8192, 0, pitch, object_offset + 60, radius, (uint32)frame, 1, 2);
        w_u16(object_offset + 138, value);
        if ((sint16)value >= 0)
        {
            scene = r_u32(0x8008A114);
            x = r_u32(object_offset + 60);
            a = r_u32(scene + 16) - x;
            w_u32(vector, a);
            x = r_u32(scene + 20);
            y = r_u32(object_offset + 64);
            w_u32(vector + 4, x - y);
            z = r_u32(scene + 24);
            c = z - r_u32(object_offset + 68);
            w_u32(vector + 8, c);
            x = (uint32)((sint32)a >> 6);
            z = (uint32)((sint32)c >> 6);
            squared = x * x + z * z;
            w_u32(vector + 4, 0);
            magnitude = (sint32)squared >= 0 ? p2_sdk_8007e794(squared) : p2_sdk_8007e794(squared >> 2) << 1;
            w_u32(vector + 4, magnitude);
            if (magnitude != 0)
            {
                divisor = r_s32(definition + 40);
                if (divisor == 0)
                    xport_mips_break(7);
                quotient = 0x40000000 / divisor;
                value = (uint32)(((sint64)(sint32)magnitude * quotient) >> 18);
                z = r_u32(object_offset + 100);
                x = r_u32(object_offset + 92);
                w_u32(0x1F80030C, value);
                heading = (uint32)Atan2Angle((sint32)z, (sint32)x);
                z = r_u32(vector + 8);
                x = r_u32(vector);
                value = (uint32)Atan2Angle((sint32)z, (sint32)x);
                value = (heading - value) & 0xFFF;
                if (value >= 2049)
                    value = 4096 - value;
                angle = (sint16)(value - 1024);
                factor = r_u32(0x1F80030C);
                factor = (uint32)(((sint64)angle * (sint32)factor) >> 12);
                factor = 4096 - (uint32)(sint32)(sint16)factor;
                value = (uint32)(((sint64)(sint32)(pitch << 12) * (sint32)factor) >> 12);
                p2_func_8006a814(r_u16(object_offset + 138), (uint32)((sint32)value >> 12));
            }
        }
    }
    if (r_u32(definition + 24) == 0 && (r_u8(definition) & 1) == 0)
    {
        frame_action = 2;
        goto calculate_frame;
    }
    value = (uint32)p2_func_800185c8(object_offset, object_offset + 92, 0);
    w_u16(object_offset + 86, (value - 1024) & 0xFFF);
    if ((r_u8(definition) & 1) != 0)
    {
        value = r_u32(object_offset + 96);
        factor = r_u32(definition + 32);
        w_u32(object_offset + 96, value - factor);
    }
    frame_action = 3;
calculate_frame:
    pointer = r_u32(object_offset + 104);
    next_frame = 0;
    if (pointer != 0)
    {
        end_frame = r_s16(pointer + 6) - 1;
        frame = r_s16(object_offset + 130);
        if (frame < end_frame)
            next_frame = frame + 1;
    }
    if (frame_action == 0)
    {
        if (r_s16(object_offset + 130) >= next_frame)
        {
            w_u8(object_offset + 13, r_u8(object_offset + 13) | 2);
            p2_guest_stack_pointer = saved_stack;
            return 0;
        }
        goto apply_phase;
    }
    if (frame_action == 1)
    {
        w_u32(object_offset + 28, r_u32(object_offset + 28) | 0x80000000);
        goto select_second;
    }
    if (frame_action == 2)
        goto apply_phase;
    divisor = r_u8(definition) >> 2;
    if (divisor != 0)
    {
        frame = r_s16(object_offset + 128);
        end_frame = r_s16(definition + 2);
        if ((end_frame - (frame + 1)) % divisor != 0)
            goto apply_phase;
    }
    if (r_u8(definition + 1) == 19 && (player_interaction.flags & 4) != 0)
        goto apply_phase;
    candidate = r_u32(definition + 20);
    spawn_kind = 3;
    goto spawn_template;
select_second:
    candidate = r_u32(definition + 12);
    spawn_kind = 1;
    goto spawn_template;
select_ending:
    candidate = r_u32(definition + 16);
    spawn_kind = 2;
spawn_template:
    if (candidate != 0)
    {
        if ((r_u8(candidate + 13) & 0x80) != 0)
        {
            value = r_u8(candidate + 13);
            w_u32(candidate + 4, 0);
            w_u8(candidate + 13, value & 0x7F);
        }
        else
        {
            candidate = p2_func_8004002c(candidate);
            if (candidate == 0)
                goto after_spawn;
            w_u32(candidate + 28, r_u32(candidate + 28) & 0xFFFFFFFB);
        }
        flags = r_u32(candidate + 28);
        w_u32(candidate + 172, 0xFFFFFFFF);
        w_u16(candidate + 130, 0);
        w_u8(candidate + 124, 0);
        w_u32(candidate + 100, 0);
        w_u32(candidate + 96, 0);
        w_u32(candidate + 92, 0);
        w_u32(candidate + 28, (flags & 0x1FFD97EF) | 0x20000);
    }
after_spawn:
    if (spawn_kind == 3)
    {
        p2_func_8002b278(definition, candidate, object_offset, 0);
        goto apply_phase;
    }
    spawned = candidate;
    if (spawn_kind == 0)
    {
        phase = 0x20000000;
        if (spawned == 0)
            goto select_second;
        goto initialize_motion;
    }
    if (spawn_kind == 1)
    {
        if (spawned == 0)
        {
            if ((r_u32(object_offset + 28) & 0x20000000) != 0)
                goto end_phase;
            goto select_ending;
        }
        phase = 0x40000000;
        if ((r_u32(object_offset + 28) & 0x2000) == 0)
            w_u8(spawned + 124, 2);
        scene = r_u32(0x8008A114);
        value = r_u32(scene + 136);
        w_u8(spawned + 126, 3);
        w_u32(spawned + 112, value);
        value = r_u8(object_offset + 120);
        w_u8(spawned + 121, (uint8)-3);
        w_u8(spawned + 120, value);
        value = r_u16(definition + 2);
        w_u32(spawned + 200, 0);
        w_u32(spawned + 192, 0);
        w_u32(spawned + 184, 0);
        w_u32(spawned + 176, 0);
        w_u16(spawned + 128, value - 1);
        if ((r_u32(object_offset + 28) & 0x20000000) != 0)
            goto apply_phase;
        goto initialize_motion;
    }
    if (spawned == 0)
        goto end_phase;
    phase = 0x80000000;
    if (r_u32(definition + 4) != 0)
    {
        if ((p2_func_8005e8bc(object_offset + 60, r_u32(definition + 36), ending_attenuation, ending_pan) & 0xFF) == 0)
            goto apply_phase;
        factor = (uint32)(sint32)(sint16)(sound_settings.effects_volume);
        value = r_u32(ending_attenuation);
        value = (uint32)((sint32)(value * factor) >> 12);
        w_u32(ending_attenuation, value);
        if (value == 0)
            goto apply_phase;
        pointer = r_u32(definition + 4);
        sound = 0;
        if (pointer != 0 && r_s32(pointer) >= 2)
            sound = pointer + 12;
        value = r_u16(ending_attenuation);
        factor = r_u16(ending_pan);
        sound_effects.Queue(sound, 0, 0, value, factor, 2);
    }
    goto apply_phase;
initialize_motion:
    flags = r_s8(object_offset + 125) == 6;
    if (flags == 0)
    {
        frame = r_s8(object_offset + 126);
        flags = frame == 3;
    }
    if (flags != 0)
    {
        flags = r_u32(spawned + 28);
        w_u8(spawned + 120, (uint8)-3);
        w_u32(spawned + 28, flags | 0x2000);
    }
    else
    {
        w_u8(spawned + 120, 0 - (uint32)frame);
        if (r_u32(definition + 24) != 0 || (r_u8(definition) & 1) != 0)
        {
            if (r_s8(object_offset + 126) == 1)
            {
                value = (uint16)(character_controller.motion.form_flags >> 16) & 15;
                if (value - 5 < 2 || value == 7 || value - 10 < 2)
                    w_u32(spawned + 28, r_u32(spawned + 28) | 0x2000);
            }
            heading = (r_u16(object_offset + 150) + 2048) & 0xFFF;
            factor = r_u32(definition + 24);
            value = (uint32)SinQ14((sint16)heading);
            value = (uint32)(((sint64)(sint32)factor * ((sint32)value >> 2)) >> 12);
            w_u32(spawned + 92, value);
            w_u32(spawned + 96, 0);
            factor = r_u32(definition + 24);
            value = (uint32)SinQ14((sint16)(1024 - heading));
            w_u32(spawned + 100, (uint32)(((sint64)(sint32)factor * ((sint32)value >> 2)) >> 12));
        }
        else
        {
            w_u32(spawned + 100, 0);
            w_u32(spawned + 96, 0);
            w_u32(spawned + 92, 0);
        }
    }
    if (r_u32(definition + 4) != 0 && (p2_func_8005e8bc(object_offset + 60, r_u32(definition + 36), attenuation, pan) & 0xFF) != 0)
    {
        factor = (uint32)(sint32)(sint16)(sound_settings.effects_volume);
        value = r_u32(attenuation);
        value = (uint32)((sint32)(value * factor) >> 12);
        w_u32(attenuation, value);
        if (value != 0)
        {
            pointer = r_u32(definition + 4);
            sound = 0;
            if (pointer != 0 && r_s32(pointer) > 0)
                sound = pointer + 4;
            value = r_u16(attenuation);
            factor = r_u16(pan);
            sound_effects.Queue(sound, 0, 0, value, factor, 2);
        }
    }
apply_phase:
    if (spawned == 0)
    {
        if (next_frame != -1)
        {
            pointer = r_u32(object_offset + 104);
            w_u16(object_offset + 130, next_frame);
            if (pointer != 0)
                p2_func_8001a6ac(object_offset);
        }
        goto animate_velocity;
    }
    p2_func_8002b278(definition, spawned, object_offset, phase);
end_phase:
    if (r_s8(object_offset + 125) != 6)
        goto finish;
    flags = r_u32(object_offset + 28);
    if ((flags & 0x20000100) != 0)
    {
        w_u32(object_offset + 28, flags & 0xDFFFFEFF);
        goto animate_velocity;
    }
    if (r_u8(definition + 1) == 19)
    {
        if (player_interaction.flags != 0)
        {
            p2_func_8004483c(object_offset);
            goto animate_velocity;
        }
    }
    else if ((flags & 0x40000020) == 0x40000020)
    {
        count = r_u32(0x8008A320);
        table = 0x8008B420;
        kind = r_u32(object_offset + 116);
        candidate = 0;
        while (count != 0)
        {
            --count;
            pointer = r_u32(table);
            if (pointer != 0 && r_u8(pointer + 1) == kind)
            {
                candidate = pointer;
                break;
            }
            table += 4;
        }
        if (r_u32(candidate + 40) != 0)
        {
            pointer = r_u32(candidate + 4);
            sound = 0;
            if (pointer != 0 && r_s32(pointer) >= 3)
                sound = pointer + 20;
            frame = r_s16(object_offset + 138);
            if ((sound_effects.IsPlaying(sound, (uint32)frame) & 0xFF) != 0)
                p2_func_8006a970(r_u16(object_offset + 138));
            w_u16(object_offset + 138, 0xFFFF);
        }
    }
    w_u8(object_offset + 13, r_u8(object_offset + 13) | 2);
animate_velocity:
    if (r_s8(object_offset + 125) == 6 && (r_u32(object_offset + 28) & 0x20) == 0)
    {
        if ((p2_func_8005bc58(object_offset + 108, 1) & 0xFF) != 0)
            w_u32(object_offset + 28, r_u32(object_offset + 28) | 0x100);
        pointer = r_u32(object_offset + 104);
        frame = r_s8(object_offset + 120);
        pointer = r_u32(pointer + 8);
        pointer = r_u32(pointer + (uint32)frame * 20 + 12);
        for (kind = 0; kind < 3; ++kind)
        {
            x = r_u32(pointer + 44 + kind * 4);
            y = r_u32(pointer + 4 + kind * 4);
            w_u32(vector + kind * 4, x - y);
        }
        PushMatrix();
        matrix = r_u32(0x8008A0C8) - 32;
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
        p2_sdk_8007ec64(vector, object_offset + 92);
        PopMatrix();
    }
finish:
    p2_guest_stack_pointer = saved_stack;
    return spawned;
}
