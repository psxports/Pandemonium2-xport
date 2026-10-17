#include "game.h"



uint32 p2_func_80062e3c(uint32 vertices_offset, uint32 polygons_offset, sint32 count)
{
    FUNCTION_MARKER(0x80062E3C, "SLUS_005.78");
    uint32 result, saved_stack, local, color, packet, polygon, remaining, value, sum, limit;
    uint32 base, first, second, third, fourth, first_index, second_index, third_index, fourth_index;
    uint32 camera, far_distance, near_distance, distance, slot, entry, x, y, z;
    sint32 coefficient, normal;
    sint64 product;
    if ((game_flow.transition_flags & 4) && (r_u32(r_u32(polygons_offset)) & 0x40000))
    {
        result = player_interaction.flags;
        if (result)
            return result;
    }
    saved_stack = p2_guest_stack_pointer;
    p2_guest_stack_pointer -= 8;
    local = p2_guest_stack_pointer;
    color = render_view.fog_color;
    packet = render_commands.packet_offset;
    w_u32(local + 4, color | 0x28000000);
    xport_gte_write_data(8, 0);
    polygon = polygons_offset + 22;
    remaining = (uint32)count;
    do
    {
        value = render_commands.packet_offset;
        limit = render_commands.packet_limit;
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
        base = render_view.projection_offset;
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
        limit = render_view.far_distance;
        sum = x + y + z + value;
        result = (sint32)(sum << 1) < (sint32)limit;
        w_u32(local, sum);
        if (!result)
            goto next_polygon_80062e3c;
        value = r_u16(polygon);
        x = (uint32)(sint32)r_s16(0x1F8002A6);
        y = render_view.fog_scale;
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
            limit = render_view.far_distance;
            product = (sint64)(sint32)limit * (sint32)(slot << 2);
            distance = (uint32)(product >> 12);
            near_distance = r_u32(camera + 148);
            result = (sint32)near_distance < (sint32)distance;
            if (result)
            {
                result = distance - near_distance;
                if ((sint32)distance >= (sint32)far_distance)
                    goto next_polygon_80062e3c;
                value = render_view.depth_scale;
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
        entry = render_commands.ordering_table_offset + (slot << 2);
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
    render_commands.packet_offset = packet;
    p2_guest_stack_pointer = saved_stack;
    return result;
}

uint32 p2_func_80034044(uint32 object_offset)
{
    FUNCTION_MARKER(0x80034044, "SLUS_005.78");
    uint32 data, timer, value, resource, animation, count, frame, result;
    data = r_u32(r_u32(object_offset + 132) + 44);
    if (object_offset == character_action_target_offset)
        character_action_flags = character_action_flags | 0x800;
    timer = r_u32(object_offset + 144) - 1;
    w_u32(object_offset + 144, timer);
    if (timer == 0)
    {
        if ((game_flow.mode_flags & 0x100) && r_s8(data + 37) < 44)
            return p2_func_80033d7c(object_offset);
        value = r_u16(data + 48);
        resource = character_action_target_offset;
        w_u16(data + 40, value);
        if (object_offset == resource)
            character_action_target_offset = 0;
        resource = r_u32(object_offset + 132);
        w_u8(resource + 13, r_u8(resource + 13) | 0x80);
        result = r_u8(object_offset + 13) | 2;
        w_u8(object_offset + 13, result);
        return result;
    }
    if (r_s8(object_offset + 125) == 17)
    {
        result = r_u16(object_offset + 130) + 1;
        value = resource_groups[25].material_offset;
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
    FUNCTION_MARKER(0x80018FEC, "SLUS_005.78");
    uint32 saved_stack, quaternion, source, result, index, address, upper, lower, shift, value, mask, old;
    uint32 first_word, second_word, x, y, z;
    sint32 half_angle, sine, q0, q1, q2, q3, p0, p1, p2, p3;
    half_angle = (sint32)(((uint32)2048 - (uint32)(sint32)angle) << 16) >> 17;
    if (!quaternion_offset)
    {
        result = (uint32)SinQ14((sint16)(1024 - half_angle));
        w_u16(destination_offset, result);
        w_u16(destination_offset + 6, 0);
        w_u16(destination_offset + 2, 0);
        result = (uint32)SinQ14((sint16)half_angle);
        w_u16(destination_offset + 4, result);
        return result;
    }
    saved_stack = p2_guest_stack_pointer;
    p2_guest_stack_pointer -= 8;
    quaternion = p2_guest_stack_pointer;
    result = (uint32)SinQ14((sint16)(1024 - half_angle));
    w_u16(quaternion, result);
    w_u16(quaternion + 6, 0);
    w_u16(quaternion + 2, 0);
    sine = SinQ14((sint16)half_angle);
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
    FUNCTION_MARKER(0x8004002C, "SLUS_005.78");
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
        ResetMaterialAnimationList((value) != 0 ? (uint32 *)psx_addr((value) - 4, 4) + 1 : 0);
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
                ResetMaterialAnimationList((value) != 0 ? (uint32 *)psx_addr((value) - 4, 4) + 1 : 0);
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
    FUNCTION_MARKER(0x80034848, "SLUS_005.78");
    uint32 data, object, sound;
    sint32 frame, count;
    data = r_u32(allocation_offset + 44);
    object = r_u32(data + 4);
    if (!object)
        return;
    sound = r_u32(data + 68);
    if ((sound_effects.IsPlaying(sound, (uint32)r_s16(data + 66)) & 0xFF) != 0)
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
    FUNCTION_MARKER(0x80034658, "SLUS_005.78");
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
    FUNCTION_MARKER(0x8005BCFC, "SLUS_005.78");
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
    FUNCTION_MARKER(0x800346C8, "SLUS_005.78");
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
    FUNCTION_MARKER(0x8006117C, "SLUS_005.78");
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
    FUNCTION_MARKER(0x80049730, "SLUS_005.78");
    uint32 value, result, controller, acceleration, input, sum, flags, upper, lower;
    if ((character_controller.flags & 0x80000) != 0)
    {
        value = r_u32(vertical_offset) + 101;
        w_u32(vertical_offset, value);
        if ((sint32)value > 0)
            w_u32(object_offset + 28, r_u32(object_offset + 28) | 0x400);
        value = character_controller.movement.jump_ticks;
        character_controller.motion.jump_phase = 1;
        character_controller.timers.state_ticks = value;
        w_u32(horizontal_offset, 0);
        character_controller.motion.horizontal_speed = 0;
        result = r_u32(vertical_offset);
        character_controller.motion.vertical_speed = result;
        return result;
    }
    controller = r_u32(character_offset + 120);
    acceleration = r_u32(controller + 72);
    if (acceleration != 0)
    {
        input = gameplay_input.buttons;
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
                    if ((character_controller.secondary_flags & 0x80) == 0)
                        character_controller.motion.state_code = 3;
                }
            }
            value = character_controller.movement.jump_ticks;
            character_controller.motion.jump_phase = 1;
            character_controller.timers.state_ticks = value;
        }
        else if ((r_u32(object_offset + 28) & 0x400) != 0 && (sint32)sum < 0 && character_controller.motion.jump_phase >= 0)
            character_controller.motion.jump_phase = -1;
        w_u32(character_offset + 552, 0);
        character_controller.motion.vertical_speed = r_u32(vertical_offset);
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
        value = (uint32)character_controller.movement.speed_limits[3];
    else
        value = (uint32)character_controller.movement.speed_limits[1];
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
    FUNCTION_MARKER(0x80056340, "SLUS_005.78");
    uint32 matrix, saved_stack, vector, x, y, z, result;
    if (direction == 0)
        frame = (sint32)((uint32)(sint32)r_s16(definition_offset + 26) - (uint32)frame - 1);
    matrix = r_u32(definition_offset + 40);
    if (matrix == 0)
        return SampleVectorTrack(frame, definition_offset + 20, destination_offset);
    saved_stack = p2_guest_stack_pointer;
    p2_guest_stack_pointer -= 16;
    vector = p2_guest_stack_pointer;
    SampleVectorTrack(frame, definition_offset + 20, vector);
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
    FUNCTION_MARKER(0x8005642C, "SLUS_005.78");
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
            SampleVectorTrack(frame, definition_offset + 20, local);
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
            SampleVectorTrack(frame, definition_offset + 20, destination);
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
        result = Atan2Angle((sint32)(0 - dz), (sint32)(0 - dx));
    p2_guest_stack_pointer = saved_stack;
    return (sint16)result;
}

uint32 p2_func_80033d7c(uint32 object_offset)
{
    FUNCTION_MARKER(0x80033D7C, "SLUS_005.78");
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
        w_u16(object_offset + 148, random_generator.Next() & 0xFFF);
    parent = r_u32(object_offset + 132);
    if (r_u32(parent + 32) != 0)
    {
        value = r_u32(parent + 44);
        node = r_u32(object_offset + 132);
        value = r_u16(value + 24);
        frame = (sint16)value;
        w_u16(object_offset + 136, value);
        SampleVectorTrack(frame, node + 20, object_offset + 16);
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
    result = DecodeObjectFactory(target)(object_offset);
    return result;
}

uint32 p2_func_800352f4(uint32 object_offset, uint32 data_offset)
{
    FUNCTION_MARKER(0x800352F4, "SLUS_005.78");
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
    FUNCTION_MARKER(0x8002AFA8, "SLUS_005.78");
    uint32 parent, result;
    if ((!object_operations.behaviors[7].value_handler && !object_operations.behaviors[7].void_handler && !object_operations.behaviors[7].plain_handler))
        {
            ObjectCallback operation = {p2_func_8002b000, 0, 0};
            object_operations.behaviors[7] = operation;
        }
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
    FUNCTION_MARKER(0x800343F0, "SLUS_005.78");
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
            SampleWorldPath(definition, r_s16(object_offset + 24), position, 1);
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
    FUNCTION_MARKER(0x8002B000, "SLUS_005.78");
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
    FUNCTION_MARKER(0x80033564, "SLUS_005.78");
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
            result = sound_effects.IsPlaying(value, x);
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
            flags = player_interaction.flags;
            w_u16(object_offset + 130, 0xFFFF);
            player_interaction.flags = flags & 0xFFFFFFF3;
            w_u32(object_offset + 92, 4096);
            w_u32(object_offset + 104, value);
            SpawnActionEffect(object_offset, 0, 5);
            w_u32(object_offset + 144, (uint32)(sint32)r_s16(r_u32(object_offset + 104) + 6) + 15);
        }
        kind = r_s8(object_offset + 125);
        w_u8(object_offset + 124, 0);
        w_u8(object_offset + 126, 0);
        w_u8(object_offset + 127, (object_operations.behaviors[kind].value_handler == p2_func_80020c6c) ? 0 : 2);
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
        p2_func_8003db7c(object_offset, 0, (sint32)height, (const VECTOR *)psx_addr(0, 12));
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
        result = p2_func_8003db7c(object_offset, (sint32)result, r_s16(data + 30), (const VECTOR *)psx_addr(0, 12));
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
    p2_func_8003db7c(object_offset, (sint32)distance, r_s16(data + 30), (const VECTOR *)psx_addr(0, 12));
    result = r_u32(object_offset + 28) & 0xFFFFFBFF;
    w_u32(object_offset + 28, result);
    goto done;
direct_movement:
    if (distance == 0)
        goto zero_velocity;
    result = p2_func_8003db7c(object_offset, (sint32)distance, r_s16(data + 30), (const VECTOR *)psx_addr(0, 12));
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
    FUNCTION_MARKER(0x800534EC, "SLUS_005.78");
    return r_u32(0x800ABD70);
}

uint32 p2_func_80030a34(uint32 object_offset)
{
    FUNCTION_MARKER(0x80030A34, "SLUS_005.78");
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
    FUNCTION_MARKER(0x80034EAC, "SLUS_005.78");
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
    FUNCTION_MARKER(0x8005927C, "SLUS_005.78");
}

uint32 p2_func_80018144(uint32 object_offset, uint32 vector_offset, uint32 axes_offset)
{
    FUNCTION_MARKER(0x80018144, "SLUS_005.78");
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
        result = (uint32)(sint32)(sint16)Atan2Angle((sint32)(0 - z), (sint32)(0 - x));
    }
    else
        result = 4096;
    p2_guest_stack_pointer = saved_stack;
    return result;
}
