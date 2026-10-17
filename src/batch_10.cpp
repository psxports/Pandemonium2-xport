#include "game.h"



sint32 p2_func_8002dfc8(uint32 level, uint32 mode)
{
    FUNCTION_MARKER(0x8002DFC8, "SLUS_005.78");
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
    FUNCTION_MARKER(0x80034B38, "SLUS_005.78");
    w_u8(0x8008A330, 0);
}

void p2_func_800441dc(void)
{
    FUNCTION_MARKER(0x800441DC, "SLUS_005.78");
    w_u32(0x8008A338, 0);
}

uint32 p2_func_8002d4c0(void)
{
    FUNCTION_MARKER(0x8002D4C0, "SLUS_005.78");
    w_u32(0x8008A320, 4);
    return 4;
}



uint32 p2_func_8002dedc(void)
{
    FUNCTION_MARKER(0x8002DEDC, "SLUS_005.78");
    w_u8(0x800B999B, 0);
    p2_func_800260b0();
    w_u32(0x8008A4E8, 0);
    sound_effects.Stop(1);
    if (r_u16(0x8008A530) != 0)
    {
        p2_func_80066430();
        p2_func_8006ae90(r_u16(0x8008A530));
    }
    w_u16(0x8008A530, 0);
    ResetCharacterModels();
    p2_func_80034b38();
    p2_func_800441dc();
    p2_func_8002d4c0();
    material_registry = saved_material_registry;
    return material_registry.reserved[4];
}

uint32 p2_func_8001c544(uint32 object_offset, uint32 selection, uint32 descriptor_offset)
{
    FUNCTION_MARKER(0x8001C544, "SLUS_005.78");
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
    FUNCTION_MARKER(0x8001C710, "SLUS_005.78");
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

uint32 p2_func_8002a2f4(uint32 owner_offset, uint32 object_offset)
{
    FUNCTION_MARKER(0x8002A2F4, "SLUS_005.78");
    uint32 list;
    list = r_u32(object_offset + 20);
    if (list != 0)
    {
        p2_func_8002a448(0, list);
        w_u32(object_offset + 20, 0);
    }
    return p2_func_8002a144(object_offset, &sprite_pool.active_head, &sprite_pool.free_head);
}

uint32 p2_func_800455dc(uint32 object_offset, uint32 character_offset, sint32 horizontal_speed, sint32 vertical_speed)
{
    FUNCTION_MARKER(0x800455DC, "SLUS_005.78");
    uint32 distance, parent, matrix, saved_stack, position, projected, source, x, y, z, result;
    distance = (uint32)horizontal_speed;
    if (horizontal_speed >= 0)
        w_u32(object_offset + 28, r_u32(object_offset + 28) | 0x40);
    else
        distance = 0u - distance;
    p2_func_8003db7c(object_offset, (sint32)distance, vertical_speed, &character_controller.path_tangent);
    if (((uint16)(character_controller.motion.form_flags >> 16) & 15) == 5)
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
            p2_func_8003db7c(object_offset, 0, 0, &character_controller.path_tangent);
        p2_guest_stack_pointer = saved_stack;
    }
    result = r_u32(object_offset + 28) & 0xFFFFFFBF;
    w_u32(object_offset + 28, result);
    return result;
}

uint32 p2_func_8002a80c(uint32 position_offset, uint32 scale, uint32 effect)
{
    uint32 material_list_offset;
    uint32 *list;
    FUNCTION_MARKER(0x8002A80C, "SLUS_005.78");
    uint32 object, entry, next, previous, material, count, bytes, allocation, target, x, y, z, scene;
    if (effect >= 35)
        return 0;
    entry = effect;
    if (resource_groups[entry].animation_entries == 0)
        return 0;
    object = sprite_pool.free_head;
    if (object != 0)
    {
        next = r_u32(object);
        previous = sprite_pool.active_head;
        sprite_pool.active_head = object;
        sprite_pool.free_head = next;
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
    material = resource_groups[entry].material_offset;
    list = resource_groups[entry].animation_entries;
    w_u32(object + 16, material);
    if (list != 0)
    {
        count = (uint8)list[-1];
        bytes = (count + 1) * 4;
        allocation = p2_func_80064d28(bytes);
        p2_func_80066358(allocation, 0xFF, bytes);
        w_u32(allocation, count);
        target = allocation + 4;
        w_u32(object + 20, target);
        if (target != 0)
        {
            list = resource_groups[entry].animation_entries;
            bytes = count * 4;
            p2_func_8006637c((uint8 *)psx_addr(target, bytes), (const uint8 *)list, bytes);
            target = r_u32(object + 20);
            SetMaterialAnimationFlags((uint32 *)psx_addr(target + bytes - 4, 4), 8);
            ResetMaterialAnimationList((material_list_offset = r_u32(object + 20), material_list_offset != 0 ? (uint32 *)psx_addr(material_list_offset - 4, 4) + 1 : 0));
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
    FUNCTION_MARKER(0x80061F50, "SLUS_005.78");
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
        packet = render_commands.packet_offset;
        limit = render_commands.packet_limit;
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
        screen_base = render_view.projection_offset;
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
        limit = render_view.far_distance;
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
        value = render_view.fog_scale;
        sum = (uint32)((sint32)depth >> 2) + (uint32)(coefficient >> 1);
        bucket = (sint32)(sum * value) >> 19;
        if ((uint32)bucket >= 1024)
            bucket = bucket < 0 ? 0 : 1023;
        context = r_u32(0x8008A4E8);
        end = r_u32(context + 152);
        if (end != 0)
        {
            value = render_view.far_distance;
            product = (sint64)(sint32)value * (sint32)((uint32)bucket << 2);
            distance = (sint32)((uint64)product >> 12);
            start = r_u32(context + 148);
            if ((sint32)start < distance)
            {
                result = (uint32)distance - start;
                if (distance >= (sint32)end)
                    goto next_polygon;
                coefficient = ((sint32)render_view.depth_scale);
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
            if ((game_flow.transition_flags & 1) == 0)
            {
                p2_func_8002e84c(0);
                game_flow.transition_flags = game_flow.transition_flags | 1;
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
            bucket_offset = render_commands.ordering_table_offset + (uint32)bucket * 4;
            link = r_u32(bucket_offset);
            packet = render_commands.packet_offset;
            w_u32(packet, (link & 0xFFFFFF) | tag);
            link = r_u32(bucket_offset);
            w_u32(bucket_offset, (link & 0xFF000000) | (packet & 0xFFFFFF));
            if (kind == 6)
            {
                p2_func_800612c0(material, render_commands.packet_offset + 12, 3);
                w_u32(colors, xport_gte_read_data(20));
                w_u32(colors + 4, xport_gte_read_data(21));
                w_u32(colors + 8, xport_gte_read_data(22));
                xport_gte_write_data(0, r_u32(vertex2 + 8));
                xport_gte_write_data(1, r_u32(vertex2 + 12));
                xport_gte_execute(0x108041B);
                packet = render_commands.packet_offset;
                w_u32(packet + 8, r_u32(screen0));
                fourth_color = xport_gte_read_data(22);
                xport_gte_write_data(20, r_u32(colors));
                xport_gte_write_data(21, r_u32(colors + 4));
                xport_gte_write_data(22, r_u32(colors + 8));
                xport_gte_write_data(6, r_u32(colors + 8));
                xport_gte_execute(0xF8002A);
                packet = render_commands.packet_offset;
                w_u32(packet + 20, r_u32(screen1));
                w_u32(packet + 32, r_u32(screen3));
                w_u32(packet + 44, r_u32(screen2));
                packet = render_commands.packet_offset;
                w_u32(packet + 4, xport_gte_read_data(20));
                w_u32(packet + 16, xport_gte_read_data(21));
                w_u32(packet + 28, xport_gte_read_data(22));
                xport_gte_write_data(6, fourth_color);
                xport_gte_execute(0x780010);
                w_u32(metadata, r_u32(metadata) | 0x40000000 | (fourth_color & 0xFFFFFF));
                packet = render_commands.packet_offset;
                w_u32(packet + 40, xport_gte_read_data(22));
                result = render_commands.packet_offset + 52;
                render_commands.packet_offset = result;
            }
            else if (kind == 2)
            {
                packet = render_commands.packet_offset;
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
                render_commands.packet_offset = result;
            }
            else if (kind == 4)
            {
                packet = render_commands.packet_offset;
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
                render_commands.packet_offset = packet + 40;
            }
            else
            {
                packet = render_commands.packet_offset;
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
                render_commands.packet_offset = result;
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
            bucket_offset = render_commands.ordering_table_offset + (uint32)bucket * 4;
            link = r_u32(bucket_offset);
            packet = render_commands.packet_offset;
            w_u32(packet, (link & 0xFFFFFF) | tag);
            link = r_u32(bucket_offset);
            w_u32(bucket_offset, (link & 0xFF000000) | (packet & 0xFFFFFF));
            if (kind == 6)
            {
                p2_func_800612c0(material, render_commands.packet_offset + 12, 3);
                packet = render_commands.packet_offset;
                w_u32(packet + 4, xport_gte_read_data(20));
                w_u32(packet + 16, xport_gte_read_data(21));
                w_u32(packet + 28, xport_gte_read_data(22));
                xport_gte_write_data(6, r_u32(metadata));
                xport_gte_execute(0x780010);
                packet = render_commands.packet_offset;
                w_u32(packet + 8, r_u32(screen0));
                w_u32(packet + 20, r_u32(screen1));
                w_u32(packet + 32, r_u32(screen3));
                w_u32(packet + 44, r_u32(screen2));
                w_u32(packet + 40, xport_gte_read_data(22));
                result = render_commands.packet_offset + 52;
                render_commands.packet_offset = result;
            }
            else if (kind == 2)
            {
                packet = render_commands.packet_offset;
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
                render_commands.packet_offset = result;
            }
            else if (kind == 4)
            {
                packet = render_commands.packet_offset;
                p2_func_800612c0(material, packet + 12, 2);
                w_u32(packet + 8, r_u32(screen0));
                w_u32(packet + 16, r_u32(screen1));
                w_u32(packet + 24, r_u32(screen3));
                w_u32(packet + 32, r_u32(screen2));
                result = packet + 4;
                w_u32(result, xport_gte_read_data(22));
                render_commands.packet_offset = packet + 40;
            }
            else
            {
                packet = render_commands.packet_offset;
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
                render_commands.packet_offset = result;
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
    FUNCTION_MARKER(0x8005E8BC, "SLUS_005.78");
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
        angle = Atan2Angle((sint32)(0u - dz), (sint32)(0u - dx));
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
    FUNCTION_MARKER(0x8005EB18, "SLUS_005.78");
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
    scaled = (uint32)level * (uint32)(sint32)(sint16)(sound_settings.effects_volume);
    level = (sint32)scaled >> 12;
    w_u32(locals, (uint32)level);
    if (level == 0)
        goto silent;
    if (volume >= 8193)
        w_u32(locals + 4, 5);
    if ((sint32)voice >= 0 && voice < 8 && sound_effects.voices[voice].sound == sound_offset)
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
    selected = sound_effects.SelectVoice(0xFFFFFFFF, sound_offset, option);
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
            p2_func_8006a878((uint16)channel, sound_offset);
        }
    }
    p2_guest_stack_pointer = saved_stack;
    return (uint32)selected;

silent:
    if ((sint32)voice >= 0 && voice < 8 && sound_effects.voices[voice].sound == sound_offset)
        p2_func_8006a970((uint16)voice);
    p2_guest_stack_pointer = saved_stack;
    return 0xFFFFFFFF;
}

void p2_func_800606f4(void)
{
    FUNCTION_MARKER(0x800606F4, "SLUS_005.78");
    /* Original JR RA/NOP has no side effects */
}

uint32 p2_func_80065f78(sint32 intensity)
{
    FUNCTION_MARKER(0x80065F78, "SLUS_005.78");
    uint32 color, packet, next_packet;
    if (intensity >= 4096)
        color = 255;
    else if (intensity > 0)
        color = (uint32)(255 * intensity + 2048) >> 12;
    else
        color = 0;
    color |= (color << 8) | (color << 16);
    if (render_commands.overlay_head == 0)
        render_commands.overlay_head = render_commands.packet_offset;
    packet = render_commands.packet_offset;
    next_packet = packet + 28;
    w_u32(packet, ((packet + 12) & 0xFFFFFF) | 0x02000000);
    w_u32(packet + 4, 0xE1000020);
    w_u32(packet + 8, 0);
    w_u32(packet + 16, color | 0x62000000);
    w_u32(packet + 12, (next_packet & 0xFFFFFF) | 0x03000000);
    w_u32(packet + 20, 0);
    w_u32(packet + 24, 0x00F00140);
    render_commands.overlay_tail = packet + 12;
    render_commands.packet_offset = next_packet;
    return 0x03000000;
}

uint32 p2_func_80066070(sint32 intensity)
{
    FUNCTION_MARKER(0x80066070, "SLUS_005.78");
    uint32 color, packet, next_packet;
    if (intensity >= 4096)
        color = 255;
    else if (intensity > 0)
        color = (uint32)(255 * intensity + 2048) >> 12;
    else
        color = 0;
    color |= (color << 8) | (color << 16);
    if (render_commands.overlay_head == 0)
        render_commands.overlay_head = render_commands.packet_offset;
    packet = render_commands.packet_offset;
    next_packet = packet + 28;
    w_u32(packet, ((packet + 12) & 0xFFFFFF) | 0x02000000);
    w_u32(packet + 4, 0xE1000040);
    w_u32(packet + 8, 0);
    w_u32(packet + 16, color | 0x62000000);
    w_u32(packet + 12, (next_packet & 0xFFFFFF) | 0x03000000);
    w_u32(packet + 20, 0);
    w_u32(packet + 24, 0x00F00140);
    render_commands.overlay_tail = packet + 12;
    render_commands.packet_offset = next_packet;
    return 0x03000000;
}

uint32 p2_func_8005cf3c(void)
{
    FUNCTION_MARKER(0x8005CF3C, "SLUS_005.78");
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
    FUNCTION_MARKER(0x80066D60, "SLUS_005.78");
    return r_u8(0x800A5C28) == 0xFF;
}

uint32 p2_func_800678a8(void)
{
    FUNCTION_MARKER(0x800678A8, "SLUS_005.78");
    if ((gameplay_input.buttons & 0x800) == 0 || (r_u32(0x1F800388) & 0x800) != 0)
    {
        if (p2_func_80066d60() == 0)
            return 0;
    }
    if (r_u16(0x8008A530) == 0)
        return 0;
    return r_s16(r_u32(0x8008A114) + 46) == 0;
}



uint32 p2_func_80064dbc(sint32 top, uint32 height)
{
    FUNCTION_MARKER(0x80064DBC, "SLUS_005.78");
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
    FUNCTION_MARKER(0x8005FB40, "SLUS_005.78");
    uint32 scene, background, saved_color, width, height, table, map, cursor, command, kind, count;
    uint32 index, elapsed, tick, destination, angle, palette, tile, definition, ordering, packet;
    uint32 uv0, uv1, attributes, temporary, xy, next_xy, next_row, row_start, result, remaining;
    sint32 vertical, horizontal, row, column, x, y, rows, columns, current_column;

    result = game_flow.mode_flags & 0x800;
    if (result != 0)
        return result;
    ClearSceneOrderingTable();
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
        result = (uint32)AsinAngle(30720, (sint32)result);
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
        elapsed = frame_clock.milliseconds - r_u32(background + 20);
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
            ordering = render_commands.scene_ordering_table_offset + (index >> 1);
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
                packet = render_commands.aux_packet_offset;
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
                packet = render_commands.aux_packet_offset;
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
            render_commands.aux_packet_offset = destination;
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
    FUNCTION_MARKER(0x8005D170, "SLUS_005.78");
    uint32 object, result;
    p2_func_8005fb40();
    object = p2_func_800536f0(0);
    DrawHud(object);
    result = p2_func_8005cf3c();
    p2_func_800606f4();
    return result;
}

uint32 p2_func_80063a80(uint32 node_offset)
{
    FUNCTION_MARKER(0x80063A80, "SLUS_005.78");
    uint32 saved_stack, corners, packet, matrix, x, y, z, value, sum, limit, result, context, end, start, link, bucket_offset;
    uint32 material, texture, flags, uv0, uv1, uv2, uv3;
    sint32 height, bucket, distance, fade;
    sint64 product;
    packet = render_commands.packet_offset;
    limit = render_commands.packet_limit;
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
    limit = render_view.far_distance;
    sum = x + y + z + value;
    result = (sint32)(sum << 1) < (sint32)limit;
    if (result == 0)
        goto done;
    value = (uint32)(sint32)r_s16(0x1F8002A6);
    x = render_view.fog_scale;
    value += (uint32)((sint32)sum >> 2) + 128;
    bucket = (sint32)(value * x) >> 19;
    if ((uint32)bucket >= 1024)
        bucket = bucket < 0 ? 0 : 1023;
    xport_gte_write_data(8, 0);
    context = r_u32(0x8008A4E8);
    end = r_u32(context + 152);
    if (end != 0)
    {
        value = render_view.far_distance;
        product = (sint64)(sint32)value * (sint32)((uint32)bucket << 2);
        distance = (sint32)((uint64)product >> 12);
        start = r_u32(context + 148);
        if ((sint32)start < distance)
        {
            result = (uint32)distance - start;
            if (distance >= (sint32)end)
                goto done;
            product = (sint64)((sint32)render_view.depth_scale) * (sint32)result;
            fade = (sint32)((uint64)product >> 12) >> 8;
            xport_gte_write_data(8, (uint32)fade);
        }
        else
            xport_gte_write_data(8, 0);
    }
    else
        xport_gte_write_data(8, 0);
    bucket_offset = render_commands.ordering_table_offset + (uint32)bucket * 4;
    link = r_u32(bucket_offset);
    packet = render_commands.packet_offset;
    w_u32(packet, (link & 0xFFFFFF) | 0x09000000);
    link = r_u32(bucket_offset);
    w_u32(bucket_offset, (link & 0xFF000000) | (packet & 0xFFFFFF));
    material = r_u32(node_offset + 16);
    packet = render_commands.packet_offset;
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
    render_commands.packet_offset = result;
done:
    p2_guest_stack_pointer = saved_stack;
    return result;
}

uint32 p2_func_80011e04(void)
{
    uint32 material_list_offset;
    FUNCTION_MARKER(0x80011E04, "SLUS_005.78");
    uint32 matrix, flags, x, y, z, scene, node, geometry, vertices, output, remaining, first_index, value, result;
    matrix = r_u32(0x8008A0C8);
    flags = game_flow.transition_flags;
    w_u32(0x8008A0C8, matrix + 32);
    game_flow.transition_flags = flags | 0x200;
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
        UpdateMaterialAnimationList((material_list_offset = r_u32(geometry + 44), material_list_offset != 0 ? (uint32 *)psx_addr(material_list_offset - 4, 4 + 4 * r_u8(material_list_offset - 4)) + 1 : 0));
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
        value = render_view.projection_count;
        output = render_view.projection_offset + (value << 3);
        render_view.projection_offset = output;
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
        output = render_view.projection_offset;
        render_view.projection_count_aux = (uint16)(first_index);
        render_view.projection_count = (uint16)(first_index);
        render_view.projection_offset = output - (first_index << 3);
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
    flags = game_flow.transition_flags;
    matrix = r_u32(0x8008A0C8);
    result = flags & 0xFFFFFDFF;
    game_flow.transition_flags = result;
    w_u32(0x8008A0C8, matrix - 32);
    return result;
}

uint32 p2_func_80060c0c(uint32 object_offset)
{
    FUNCTION_MARKER(0x80060C0C, "SLUS_005.78");
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
