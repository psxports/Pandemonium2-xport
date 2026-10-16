#include "game.h"

uint32 p2_func_80064358(uint32 object_offset)
{
    uint32 saved_stack, first, second, normal, packet, limit, result, matrix, x, y, z, scale, depth, value, context, end, start, bucket_offset, link;
    uint32 orientation, material, texture, uv0, uv1, uv2, uv3, flags;
    sint32 width, height, bucket, distance, fade, radius;
    sint64 product;
    packet = r_u32(0x8008A4F8);
    limit = r_u32(0x8008A4A0);
    if (packet >= limit)
        return 0;
    saved_stack = p2_guest_stack_pointer;
    p2_guest_stack_pointer -= 0x30;
    first = p2_guest_stack_pointer + 0x10;
    second = p2_guest_stack_pointer + 0x18;
    normal = p2_guest_stack_pointer + 0x28;
    if (r_u8(object_offset + 12) == 8)
    {
        width = r_s32(object_offset + 24) >> 5;
        height = width;
    }
    else
    {
        width = r_s16(object_offset + 40);
        height = r_s16(object_offset + 42);
        scale = r_u32(0x1F8002C0);
        if (scale != 0)
        {
            width = (sint32)((uint32)width * scale) >> 12;
            height = (sint32)((uint32)height * scale) >> 12;
        }
    }
    x = r_u32(0x80010180);
    y = r_u32(0x80010184);
    xport_gte_write_control(0, x);
    xport_gte_write_control(1, y);
    x = r_u32(0x80010188);
    y = r_u32(0x8001018C);
    z = r_u32(0x80010190);
    xport_gte_write_control(2, x);
    xport_gte_write_control(3, y);
    xport_gte_write_control(4, z);
    matrix = r_u32(0x8008A0C8);
    x = r_u32(matrix + 20);
    y = r_u32(matrix + 24);
    z = r_u32(matrix + 28);
    xport_gte_write_control(5, (uint32)((sint32)x >> 5));
    xport_gte_write_control(6, (uint32)((sint32)y >> 5));
    xport_gte_write_control(7, (uint32)((sint32)z >> 5));
    w_u16(first, 0 - (uint32)(uint16)width);
    w_u16(first + 2, height);
    w_u16(first + 4, 0);
    w_u16(second, width);
    w_u16(second + 2, 0 - (uint32)height);
    w_u16(second + 4, 0);
    xport_gte_write_data(0, r_u32(first));
    xport_gte_write_data(1, r_u16(first + 4));
    xport_gte_write_data(2, r_u32(second));
    xport_gte_write_data(3, r_u16(second + 4));
    xport_gte_execute(0x280030);
    w_u32(first, xport_gte_read_data(12));
    w_u32(second, xport_gte_read_data(13));
    w_u32(first + 4, xport_gte_read_data(17));
    w_u32(second + 4, xport_gte_read_data(18));
    w_u32(p2_guest_stack_pointer + 0x20, xport_gte_read_data(19));
    depth = r_u32(first + 4) + r_u32(second + 4);
    limit = r_u32(0x1F8002B4);
    result = (sint32)(depth << 2) < (sint32)limit;
    if (result == 0)
        goto done;
    value = (uint32)(sint32)r_s16(0x1F8002A6);
    scale = r_u32(0x1F8002B8);
    depth <<= 1;
    value += (uint32)((sint32)depth >> 2);
    bucket = (sint32)(value * scale) >> 19;
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
    orientation = 0;
    if (r_u8(object_offset + 12) == 8)
        material = r_u32(object_offset + 20);
    else
    {
        material = r_u32(0x1F8002A8);
        if (material == 0)
        {
            value = r_u16(object_offset + 14);
            material = r_u32(object_offset + 16);
            orientation = (value & 0x180) >> 7;
        }
    }
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
    if (r_u8(object_offset + 12) == 8 || (r_u16(object_offset + 14) & 1) != 0)
    {
        xport_gte_write_data(6, r_u32(material + 16));
        xport_gte_execute(0x780010);
        w_u32(packet + 4, xport_gte_read_data(22));
    }
    else
    {
        matrix = r_u32(0x8008A0C8);
        x = r_u32(object_offset + 28);
        y = r_u32(object_offset + 32);
        z = r_u32(object_offset + 36);
        w_u32(matrix + 52, x);
        w_u32(matrix + 56, y);
        w_u32(matrix + 60, z);
        if ((r_u32(0x1F800320) & 0x201) != 1)
        {
            width = r_s16(object_offset + 40);
            radius = r_s16(object_offset + 42);
            if (radius < width)
                radius = width;
            p2_func_8002e824(r_u32(0x8008A0C8) - 12, radius);
            p2_func_8002e84c(0x80000000);
            value = r_u32(0x1F800320);
            matrix = r_u32(0x8008A0C8);
            w_u32(0x1F800320, value | 1);
            w_u32(matrix + 60, 0);
            w_u32(matrix + 56, 0);
            w_u32(matrix + 52, 0);
        }
        matrix = r_u32(0x8008A0C8);
        w_u16(normal, (uint32)(r_s32(matrix + 52) >> 5));
        w_u16(normal + 2, (uint32)(r_s32(matrix + 56) >> 5));
        w_u16(normal + 4, (uint32)(r_s32(matrix + 60) >> 5));
        xport_gte_write_data(6, r_u32(material + 16));
        p2_func_8002ede4(normal, r_u8(material + 3) | 0x80000000);
        PushMatrix();
        x = r_u32(0x1F800364);
        y = r_u32(0x1F800368);
        xport_gte_write_control(0, x);
        xport_gte_write_control(1, y);
        x = r_u32(0x1F80036C);
        y = r_u32(0x1F800370);
        z = r_u32(0x1F800374);
        xport_gte_write_control(2, x);
        xport_gte_write_control(3, y);
        xport_gte_write_control(4, z);
        xport_gte_write_data(0, r_u32(0x1F8002D4));
        xport_gte_write_data(1, r_u32(0x1F8002D8));
        xport_gte_execute(0x486012);
        w_u32(0x1F800390, xport_gte_read_data(25));
        w_u32(0x1F800394, xport_gte_read_data(26));
        w_u32(0x1F800398, xport_gte_read_data(27));
        x = r_u32(0x1F800390);
        y = r_u32(0x1F800394);
        z = r_u32(0x1F800398);
        w_u16(normal, x);
        w_u16(normal + 2, y);
        w_u16(normal + 4, z);
        PopMatrix();
        xport_gte_write_data(0, r_u32(normal));
        xport_gte_write_data(1, r_u16(normal + 4));
        xport_gte_execute(0xE80413);
        w_u32(packet + 4, xport_gte_read_data(22));
    }
    value = r_u8(material + 19);
    w_u32(packet + 12, uv0);
    w_u32(packet + 20, uv1);
    w_u32(packet + 28, uv2);
    w_u32(packet + 36, uv3);
    w_u8(packet + 7, (value & 3) | 0x2C);
    switch (orientation)
    {
        case 0:
            w_u32(packet + 8, r_u32(first));
            w_u32(packet + 16, (r_u32(first) & 0xFFFF0000) | r_u16(second));
            w_u32(packet + 24, (r_u32(second) & 0xFFFF0000) | r_u16(first));
            w_u32(packet + 32, r_u32(second));
            break;
        case 1:
            w_u32(packet + 8, (r_u32(second) & 0xFFFF0000) | r_u16(first));
            w_u32(packet + 16, r_u32(first));
            w_u32(packet + 24, r_u32(second));
            w_u32(packet + 32, (r_u32(first) & 0xFFFF0000) | r_u16(second));
            break;
        case 2:
            w_u32(packet + 8, r_u32(second));
            w_u32(packet + 16, (r_u32(second) & 0xFFFF0000) | r_u16(first));
            w_u32(packet + 24, (r_u32(first) & 0xFFFF0000) | r_u16(second));
            w_u32(packet + 32, r_u32(first));
            break;
        case 3:
            w_u32(packet + 8, (r_u32(first) & 0xFFFF0000) | r_u16(second));
            w_u32(packet + 16, r_u32(second));
            w_u32(packet + 24, r_u32(first));
            w_u32(packet + 32, (r_u32(second) & 0xFFFF0000) | r_u16(first));
            break;
    }
    result = packet + 40;
    w_u32(0x8008A4F8, result);
done:
    p2_guest_stack_pointer = saved_stack;
    return result;
}

uint32 p2_func_8001156c(uint32 node_offset)
{
    uint32 saved_stack = p2_guest_stack_pointer;
    uint32 temporary, matrix, result, x, y, z, sum;
    sint32 cosine, sine, angle;
    matrix = r_u32(0x8008A0C8);
    p2_guest_stack_pointer -= 0x28;
    temporary = p2_guest_stack_pointer + 0x10;
    w_u32(0x8008A0C8, matrix + 32);
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
    p2_sdk_8007ec64(matrix + 20, matrix + 52);
    matrix = r_u32(0x8008A0C8);
    x = r_u32(0x1F800278);
    w_u32(matrix + 20, r_u32(matrix + 20) + x);
    w_u32(matrix + 24, r_u32(matrix + 24) + r_u32(0x1F80027C));
    w_u32(matrix + 28, r_u32(matrix + 28) + r_u32(0x1F800280));
    if ((r_u16(node_offset + 14) & 0x200) != 0)
    {
        cosine = p2_func_8005dbb8((sint16)(1024 - r_u16(node_offset + 44))) >> 2;
        sine = p2_func_8005dbb8(r_s16(node_offset + 44)) >> 2;
        w_u16(0x1F800008, cosine);
        w_u16(0x1F80000E, sine);
        w_u16(0x1F80000A, 0 - (uint32)sine);
        w_u16(0x1F800010, cosine);
        cosine = p2_func_8005dbb8((sint16)(1024 - r_u16(node_offset + 46))) >> 2;
        sine = p2_func_8005dbb8(r_s16(node_offset + 46)) >> 2;
        w_u16(0x1F800014, cosine);
        w_u16(0x1F800020, 0 - (uint32)sine);
        w_u16(0x1F800018, sine);
        w_u16(0x1F800024, cosine);
        p2_sdk_8007e834(0x1F800014, 0x1F800000, temporary);
        result = p2_sdk_8007e834(0x1F800264, temporary, r_u32(0x8008A0C8));
        x = r_u32(result);
        y = r_u32(result + 4);
        xport_gte_write_control(0, x);
        xport_gte_write_control(1, y);
        x = r_u32(result + 8);
        y = r_u32(result + 12);
        z = r_u32(result + 16);
        xport_gte_write_control(2, x);
        xport_gte_write_control(3, y);
        xport_gte_write_control(4, z);
        matrix = r_u32(0x8008A0C8);
        angle = r_s16(0x1F80028A);
        sum = (uint32)(((sint64)r_s32(matrix + 20) * angle) >> 12);
        x = r_u32(matrix + 24);
        angle = r_s16(0x1F80028C);
        sum += (uint32)(((sint64)(sint32)x * angle) >> 12);
        y = r_u32(matrix + 28);
        angle = r_s16(0x1F80028E);
        sum += (uint32)(((sint64)(sint32)y * angle) >> 12);
        if ((sint32)sum < 0)
            p2_func_80063a80(node_offset);
    }
    else
    {
        angle = r_s16(node_offset + 44);
        sine = r_s16(node_offset + 40);
        w_u32(matrix + 20, r_u32(matrix + 20) + (uint32)((angle * sine) >> 2));
        angle = r_s16(node_offset + 46);
        sine = r_s16(node_offset + 42);
        w_u32(matrix + 24, r_u32(matrix + 24) + (uint32)((angle * sine) >> 2));
        if ((r_u16(node_offset + 14) & 4) != 0)
        {
            PushMatrix();
            x = r_u32(0x1F800284);
            y = r_u32(0x1F800288);
            xport_gte_write_control(0, x);
            xport_gte_write_control(1, y);
            x = r_u32(0x1F80028C);
            y = r_u32(0x1F800290);
            z = r_u32(0x1F800294);
            xport_gte_write_control(2, x);
            xport_gte_write_control(3, y);
            xport_gte_write_control(4, z);
            p2_sdk_8007ec64(r_u32(0x8008A0C8) + 20, 0x1F800258);
            PopMatrix();
            x = r_u32(0x1F800260);
            y = r_u32(0x1F800258);
            angle = (sint32)p2_func_8005dc00((sint32)x, (sint32)y);
            cosine = p2_func_8005dbb8((sint16)(1024 - (uint32)angle)) >> 2;
            x = r_u32(0x1F800260);
            y = r_u32(0x1F800258);
            angle = (sint32)p2_func_8005dc00((sint32)x, (sint32)y);
            sine = p2_func_8005dbb8((sint16)angle) >> 2;
            w_u16(0x1F800014, cosine);
            w_u16(0x1F800020, 0 - (uint32)sine);
            w_u16(0x1F800018, sine);
            w_u16(0x1F800024, cosine);
            result = p2_sdk_8007ea34(0x1F800014, r_u32(0x8008A0C8));
            x = r_u32(result);
            y = r_u32(result + 4);
            xport_gte_write_control(0, x);
            xport_gte_write_control(1, y);
            x = r_u32(result + 8);
            y = r_u32(result + 12);
            z = r_u32(result + 16);
            xport_gte_write_control(2, x);
            xport_gte_write_control(3, y);
            xport_gte_write_control(4, z);
            p2_func_80063e78(node_offset);
        }
        else
            p2_func_80064358(node_offset);
    }
    matrix = r_u32(0x8008A0C8);
    w_u32(0x8008A0C8, matrix - 32);
    result = matrix - 64;
    x = r_u32(result);
    y = r_u32(result + 4);
    xport_gte_write_control(0, x);
    xport_gte_write_control(1, y);
    x = r_u32(result + 8);
    y = r_u32(result + 12);
    z = r_u32(result + 16);
    xport_gte_write_control(2, x);
    xport_gte_write_control(3, y);
    xport_gte_write_control(4, z);
    p2_guest_stack_pointer = saved_stack;
    return result;
}

uint32 p2_func_8001baa8(uint32 object_offset)
{
    uint32 saved_stack, animation, value, result, coordinates;
    sint32 limit;
    animation = r_u32(object_offset + 24);
    if (animation == 0xFFFFFFFF)
        return 0xFFFFFFFF;
    value = r_u16(animation + 8);
    limit = r_s16(animation + 4);
    ++value;
    w_u16(animation + 8, value);
    if (limit < (sint16)value)
    {
        result = r_u8(animation + 2) & 2;
        if (result == 0)
            return result;
        w_u16(animation + 8, 0);
    }
    saved_stack = p2_guest_stack_pointer;
    p2_guest_stack_pointer -= 0x18;
    coordinates = p2_guest_stack_pointer + 0x10;
    p2_func_80017364(r_s16(animation + 8), animation, coordinates);
    if (r_s32(coordinates + 4) < 0)
        w_u32(coordinates + 4, 0);
    w_u16(object_offset + 40, (uint32)(r_s32(coordinates) >> 5));
    result = (uint32)(r_s32(coordinates + 4) >> 5);
    w_u16(object_offset + 42, result);
    p2_guest_stack_pointer = saved_stack;
    return result;
}

uint32 p2_func_800615c4(uint32 vertices_offset, uint32 polygons_offset, sint32 count)
{
    uint32 colors, metadata, normal, remaining = (uint32)count;
    uint32 packet, limit, sum, depth, value, index0, index1, index2, index3, screen_base;
    uint32 screen0, screen1, screen2, screen3, vertex0, vertex1, vertex2, vertex3;
    uint32 material, flags, context, start, end, kind, bucket_offset, link, tag, fourth_color, result;
    sint32 coordinate, coefficient, surface, bucket, distance, fade;
    sint64 product;
    p2_func_8002e84c(0);
    colors = r_u32(0x1F8002AC);
    xport_gte_write_data(8, 0);
    normal = polygons_offset + 16;
    metadata = colors + 12;
    do
    {
        w_u32(metadata, 0);
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
        index2 = r_u16(normal - 8);
        index3 = r_u16(normal - 6);
        screen0 = screen_base + index0 * 8;
        screen1 = screen_base + index1 * 8;
        screen2 = screen_base + index2 * 8;
        screen3 = screen_base + index3 * 8;
        vertex0 = vertices_offset + index0 * 16;
        vertex1 = vertices_offset + index1 * 16;
        vertex2 = vertices_offset + index2 * 16;
        vertex3 = vertices_offset + index3 * 16;
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
        if ((flags & 1) != 0 && surface <= 0x3FFFF800)
            w_u32(metadata, 0x80000000);
        coefficient = r_s16(normal + 6);
        value = r_u32(0x1F8002B8);
        sum = (uint32)((sint32)depth >> 2) + (uint32)(coefficient >> 1);
        bucket = (sint32)(sum * value) >> 19;
        if ((uint32)bucket >= 1024)
            bucket = bucket < 0 ? 0 : 1023;
        value = r_u32(metadata);
        p2_func_8002ede4(vertex0, (flags >> 24) | (value & 0x80000000));
        xport_gte_write_data(6, r_u32(material + 16));
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
    next_polygon:
        metadata += 16;
        polygons_offset += 24;
        colors += 16;
        normal += 24;
        --remaining;
    } while (remaining != 0);
    return result;
}

uint32 p2_func_800607bc(uint32 object_offset)
{
    uint32 flags, value, index, command, node, definition, selected, result;
    sint32 step;
    flags = r_u32(object_offset + 28);
    value = r_u32(object_offset + 228);
    w_u32(object_offset + 28, flags & 0xFC7FFFFF);
    if ((value & 0x2000) != 0)
        p2_func_8003fbb0(object_offset);
    flags = r_u32(object_offset + 28);
    if ((flags & 0x100) != 0)
    {
        w_u32(object_offset + 28, flags & 0xFFFFFEFF);
        for (index = 0; index < 3; ++index)
        {
            value = r_u32(object_offset + 108 + index * 4);
            if ((value & 0xFFF) == 0)
                continue;
            command = (value >> 12) & 31;
            switch (command)
            {
                case 1:
                    flags = r_u32(object_offset + 28);
                    if ((flags & 0x240000) != 0x40000)
                    {
                        if ((flags & 0x440000) != 0)
                            w_u32(object_offset + 28, flags & 0xFFBFFFFF);
                        else
                            w_u32(object_offset + 28, flags | 0x1020000);
                        w_u32(object_offset + 28, (r_u32(object_offset + 28) & 0xFFDFFFFF) | 0x40000);
                    }
                    if ((r_u32(object_offset + 228) & 0x2000) == 0)
                        break;
                    definition = p2_func_8003fb58(r_u32(object_offset + 116));
                    selected = 0;
                    node = r_u32(object_offset + 8);
                    while (node != 0)
                    {
                        if (r_u8(node + 12) == 0 && (r_u32(node + 228) & 0x2000) != 0)
                        {
                            value = r_u32(node + 92);
                            if (value != 0)
                            {
                                selected = value;
                                break;
                            }
                            value = p2_func_800606fc(node);
                            if (value != r_u32(node + 92))
                            {
                                selected = value;
                                break;
                            }
                        }
                        node = r_u32(node + 4);
                    }
                    if (selected == 0)
                        selected = r_u32(definition + 12);
                    w_u32(object_offset + 92, selected);
                    break;
                case 2:
                    if ((r_u32(object_offset + 228) & 0x2000) != 0)
                        p2_func_8003fc44(object_offset, 41);
                    else
                    {
                        flags = r_u32(object_offset + 28);
                        if ((flags & 0x240000) != 0x240000)
                        {
                            value = (flags & 0x440000) != 0 ? flags & 0xFFBFFFFF : flags | 0x2010000;
                            w_u32(object_offset + 28, value);
                            w_u32(object_offset + 28, r_u32(object_offset + 28) | 0x240000);
                        }
                    }
                    break;
                case 3:
                case 14:
                    goto hide_object;
                case 4:
                case 15:
                    goto show_object;
                case 5:
                    flags = r_u32(object_offset + 28);
                    if ((flags & 0x40000) != 0)
                        w_u32(object_offset + 28, (flags & 0xFFFBFFFF) | 0x400000);
                    break;
                case 6:
                    if ((r_u32(object_offset + 228) & 0x2000) != 0)
                        p2_func_8003fc44(object_offset, 45);
                    else
                        w_u32(object_offset + 28, (r_u32(object_offset + 28) | 0x40000) & 0xFFBFFFFF);
                    break;
                case 7:
                    if ((r_u32(object_offset + 228) & 0x2000) != 0)
                        p2_func_8003fc44(object_offset, 46);
                    else
                        w_u32(object_offset + 28, (r_u32(object_offset + 28) & 0xFFDFFFFF) | 0x800000);
                    break;
                case 8:
                    w_u32(object_offset + 28, (r_u32(object_offset + 28) | 0x8000) & 0xFC1BFFFF);
                    goto process_mode;
                case 9:
                    if ((r_u32(object_offset + 228) & 0x2000) != 0)
                        p2_func_8003fc44(object_offset, 48);
                    else
                        w_u32(object_offset + 28, r_u32(object_offset + 28) | 0xA00000);
                    break;
                case 10:
                process_mode:
                    if ((r_u32(object_offset + 228) & 0x2000) == 0)
                        goto finish_commands;
                    p2_func_8003fc44(object_offset, 49);
                    break;
                case 12:
                    if ((r_u8(object_offset + 13) & 1) != 0)
                        goto show_object;
                hide_object:
                    flags = r_u32(object_offset + 28);
                    value = r_u8(object_offset + 13);
                    w_u32(object_offset + 28, flags | 0x4000);
                    w_u8(object_offset + 13, value | 1);
                    break;
                show_object:
                    flags = r_u32(object_offset + 28);
                    value = r_u8(object_offset + 13);
                    w_u32(object_offset + 28, flags | 0x80000);
                    w_u8(object_offset + 13, value & 0xFE);
                    break;
                case 13:
                    flags = r_u32(object_offset + 28);
                    if ((flags & 0x40000) != 0)
                        w_u32(object_offset + 28, (flags & 0xFFFBFFFF) | 0x400000);
                    else
                        w_u32(object_offset + 28, (flags | 0x40000) & 0xFFBFFFFF);
                    break;
            }
        }
        flags = r_u32(object_offset + 28);
    }
    w_u16(object_offset + 136, 0);
    if ((flags & 0x840000) != 0 && (r_u8(object_offset + 13) & 1) == 0)
    {
        step = (flags & 0x200000) != 0 ? -1 : 1;
        w_u16(object_offset + 136, step);
        p2_func_8001b334(object_offset, step);
    }
finish_commands:
    result = p2_func_8005bc58(object_offset + 108, 3) << 24;
    if (result != 0)
    {
        result = r_u32(object_offset + 28) | 0x100;
        w_u32(object_offset + 28, result);
    }
    return result;
}

uint32 p2_func_8004fd44(uint32 object_offset, uint32 character_offset)
{
    uint32 saved_stack = p2_guest_stack_pointer;
    uint32 position, input, object_slot, current, value, flags, parent, matrix, scene, x, y, z, result, kind;
    p2_guest_stack_pointer -= 0x38;
    position = p2_guest_stack_pointer + 0x10;
    input = p2_guest_stack_pointer + 0x20;
    object_slot = p2_guest_stack_pointer + 0x30;
    w_u32(object_slot, object_offset);
    if (r_s32(character_offset) >= 0 || (r_u32(0x1F800384) & 0x10) == 0 || (r_u32(0x1F800388) & 0x10) != 0)
        goto check_form;
    value = r_u32(character_offset + 704);
    if (((value >> 16) & 15) >= 2)
        goto check_form;
    if (((value >> 24) & 15) != 15)
        goto change_form;
    if ((r_u32(0x1F800320) & 0x48) != 0)
        goto check_form;
    value &= 0xFF0FFFFF;
    value |= (1 - ((value >> 16) & 15)) << 20;
    value &= 0xF0FFFFFF;
    flags = r_u32(character_offset);
    value |= (value >> 20) << 24;
    w_u32(character_offset + 704, value);
    if ((flags & 0x80000000) == 0)
        goto check_form;
    p2_func_8001c684(character_offset);
    value = r_u32(character_offset + 704);
    w_u32(character_offset + 32, 0);
    w_u32(character_offset + 28, 0);
    w_u32(character_offset + 24, 0);
    value = ((value & 0xF0FFFFFF) | 0xF000000) & 0xFFF0FFFF;
    current = r_u32(object_slot);
    value |= ((value >> 20) & 15) << 16;
    w_u32(character_offset + 704, value);
    w_u8(current + 124, 0);
    current = r_u32(object_slot);
    w_u8(current + 126, 0);
    scene = r_u32(0x8008A114);
    value = r_u32(scene + 136);
    current = r_u32(object_slot);
    w_u32(current + 112, value);
    value = (r_u32(character_offset + 704) >> 18) & 0x3C;
    current = r_u32(r_u32(0x8008B4B8 + value) + 232);
    w_u32(object_slot, current);
    w_u8(current + 124, 1);
    current = r_u32(object_slot);
    w_u8(current + 126, 1);
    current = r_u32(object_slot);
    w_u8(current + 13, r_u8(current + 13) & 0xFE);
    current = r_u32(object_slot);
    if ((r_u32(current + 28) & 0x2000) != 0)
        goto local_position;
    parent = r_u32(current + 132);
    if (parent == 0 || r_u32(parent + 40) == 0)
        goto local_position;
    x = r_u32(current + 16);
    y = r_u32(current + 20);
    z = r_u32(current + 24);
    w_u32(input, x);
    w_u32(input + 4, y);
    w_u32(input + 8, z);
    PushMatrix();
    current = r_u32(object_slot);
    matrix = r_u32(r_u32(current + 132) + 40);
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
    current = r_u32(object_slot);
    matrix = r_u32(r_u32(current + 132) + 40);
    x = r_u32(position);
    w_u32(position, x + r_u32(matrix + 20));
    matrix = r_u32(r_u32(current + 132) + 40);
    y = r_u32(position + 4);
    w_u32(position + 4, y + r_u32(matrix + 24));
    matrix = r_u32(r_u32(current + 132) + 40);
    z = r_u32(position + 8);
    w_u32(position + 8, z + r_u32(matrix + 28));
    goto position_ready;
local_position:
    current = r_u32(object_slot);
    x = r_u32(current + 16);
    y = r_u32(current + 20);
    z = r_u32(current + 24);
    w_u32(position, x);
    w_u32(position + 4, y);
    w_u32(position + 8, z);
position_ready:
    w_u32(position + 4, r_u32(position + 4) + 12288);
    p2_func_8002a80c(position, 10240, 7);
    p2_func_80044ffc(14, 3);
    current = r_u32(object_slot);
    flags = r_u32(current + 28);
    parent = r_u32(current + 112);
    w_u32(current + 28, flags & 0xFFFFF7FF);
    p2_func_80024844(parent);
    scene = r_u32(0x8008A114);
    x = r_u32(scene + 16);
    y = r_u32(scene + 20);
    z = r_u32(scene + 24);
    w_u32(0x800B16D8, x);
    w_u32(0x800B16DC, y);
    w_u32(0x800B16E0, z);
    scene = r_u32(0x8008A114);
    current = r_u32(object_slot);
    x = r_u32(current + 16);
    y = r_u32(current + 20);
    z = r_u32(current + 24);
    w_u32(scene + 16, x);
    w_u32(scene + 20, y);
    w_u32(scene + 24, z);
    scene = r_u32(0x8008A114);
    current = r_u32(object_slot);
    parent = r_u32(scene + 136);
    flags = r_u32(current + 28);
    p2_func_800451cc(parent, r_s16(current + 136), scene + 28, flags & 0x10);
    scene = r_u32(0x8008A114);
    x = r_u32(0x800B16D8);
    y = r_u32(0x800B16DC);
    z = r_u32(0x800B16E0);
    w_u32(scene + 16, x);
    w_u32(scene + 20, y);
    w_u32(scene + 24, z);
    scene = r_u32(0x8008A114);
    w_u8(scene + 74, 15);
    w_u8(scene + 73, 15);
    w_u8(scene + 72, 15);
    current = r_u32(object_slot);
    x = r_u32(current + 16);
    y = r_u32(current + 20);
    z = r_u32(current + 24);
    w_u32(0x800B16D8, x);
    w_u32(0x800B16DC, y);
    w_u32(0x800B16E0, z);
    flags = r_u32(character_offset);
    y = r_u32(0x800B16DC);
    w_u32(0x800B16DC, y + ((sint32)flags >= 0 ? 14336 : 32768));
    value = r_u32(0x1F800320);
    flags = r_u32(character_offset + 4);
    w_u32(0x8008A4F4, 15);
    w_u32(0x1F800320, value | 0x100);
    if ((flags & 0x20) != 0)
    {
        p2_func_8005a440(r_u32(object_slot), character_offset);
        flags = r_u32(character_offset + 4);
        value = r_u32(0x8008A52C);
        w_u32(character_offset + 4, flags & 0xFFFFFFDF);
        if ((value & 0x200) != 0)
            goto ensure_form;
        p2_func_80027b1c(r_u32(object_slot), character_offset, 6);
    }
    else if ((flags & 0x100) != 0)
    {
        w_u32(character_offset + 4, (flags | 0x20) & 0xFFFFFEFF);
        p2_func_80027ac0(character_offset, 6);
        p2_func_8005a668(character_offset);
    }
    result = r_u32(0x8008A52C) & 0x200;
    if (result == 0)
        goto done;
ensure_form:
    result = r_u16(character_offset + 706) & 15;
    if (result == 0)
    {
        result = r_u32(character_offset + 4) & 0x100;
        if (result == 0)
            result = p2_func_8005a440(r_u32(object_slot), character_offset);
    }
    goto done;
check_form:
    result = r_u8(character_offset + 707) & 15;
    if (result == 15)
        goto done;
change_form:
    value = r_u32(character_offset + 704);
    kind = (value >> 24) & 15;
    result = (value >> 16) & 15;
    if (kind == result)
        goto done;
    p2_func_800401c8(r_u32(object_slot));
    kind = r_u8(character_offset + 707) & 15;
    kind = p2_func_80055dc0(object_slot, character_offset, kind);
    p2_func_800401a8(r_u32(object_slot));
    current = r_u32(object_slot);
    flags = r_u32(current + 28);
    if ((flags & 0x8000000) != 0 && r_u32(r_u32(character_offset + 780) + 108) == 0)
    {
        w_u32(current + 28, flags & 0xF7FFFFFF);
        current = r_u32(object_slot);
        flags = r_u32(current + 28);
    }
    if ((flags & 0x1000000) != 0 && r_u32(r_u32(character_offset + 780) + 100) == 0)
        w_u32(current + 28, flags & 0xFEFFFFFF);
    current = r_u32(object_slot);
    if ((r_u32(current + 28) & 0x400) != 0 && r_u32(r_u32(character_offset + 780) + 92) == 0)
    {
        value = r_u32(character_offset + 768);
        flags = r_u32(character_offset);
        w_u32(character_offset + 32, 0xFFFFFFFF);
        w_u32(character_offset + 28, 0);
        w_u16(character_offset + 48, 0);
        w_u16(character_offset + 684, value);
        w_u32(character_offset, flags | 0x8000000);
    }
    w_u8(character_offset + 713, 38);
    current = r_u32(object_slot);
    result = r_u32(current + 104);
    if (result != 0)
        goto done;
    switch (kind)
    {
        case 3:
        case 10:
        case 11:
        case 12:
            if ((r_u32(character_offset + 4) & 0x80) == 0)
                w_u8(character_offset + 712, 6);
            break;
        case 5:
        case 7:
            current = r_u32(object_slot);
            if ((r_u32(current + 28) & 0x400) != 0 && (r_u32(character_offset + 4) & 0x80) == 0)
                w_u8(character_offset + 712, 3);
            break;
        case 9:
            if ((r_u32(character_offset + 4) & 0x80) == 0)
                w_u8(character_offset + 712, 2);
            break;
    }
    current = r_u32(object_slot);
    result = r_u32(current + 104);
    if (result == 0)
    {
        result = r_u32(character_offset + 4) & 0x80;
        if (result == 0)
            w_u8(character_offset + 712, 0);
    }
done:
    p2_guest_stack_pointer = saved_stack;
    return result;
}

uint32 p2_func_80052ba4(uint32 object_offset, uint32 character_offset)
{
    uint32 flags, value, result, voice_owner, sound, kind, animation_data, pitch;
    sint32 previous, animation, voice, volume;
    if (r_s8(character_offset + 14) != 0 && (r_u32(character_offset) & 0x180000) != 0 && (r_u32(character_offset + 4) & 0x80) == 0)
        w_u8(character_offset + 712, 8);
    previous = r_s8(character_offset + 713);
    animation = r_s8(character_offset + 712);
    if (previous == animation)
    {
        if ((uint32)previous - 1 >= 34)
            w_u16(character_offset + 62, (uint32)r_u16(character_offset + 62) - 1);
        if (r_s8(character_offset + 712) != 7)
            return 7;
        result = r_u32(character_offset + 24);
        if (result != 0)
            return result;
        result = r_u32(character_offset + 4) & 0x40;
        if (result != 0)
            return result;
        voice_owner = character_offset != 0 ? character_offset : 0x800ABD68;
        sound = r_u32(0x8008A408);
        value = sound != 0 && r_s32(sound) >= 7 ? sound + 52 : 0;
        result = p2_func_8005e868(value, (uint32)(sint32)r_s8(voice_owner + 912)) << 24;
        if (result == 0)
        {
            sound = r_u32(0x8008A408);
            value = sound != 0 && r_s32(sound) >= 11 ? sound + 84 : 0;
            result = p2_func_8005e868(value, (uint32)(sint32)r_s8(voice_owner + 912)) << 24;
            if (result == 0)
                return result;
        }
        p2_func_8006a970((uint16)(sint16)r_s8(voice_owner + 912));
        w_u8(voice_owner + 912, 0xFF);
        return 0xFFFFFFFF;
    }
    if (previous == 7 && (r_u32(object_offset + 28) & 0x8000000) != 0 && (r_u16(character_offset + 706) & 15) == 4)
    {
        w_u8(character_offset + 712, 7);
        goto finish;
    }
    animation = r_s8(character_offset + 712);
    if (animation == 7)
    {
        kind = r_u16(character_offset + 706) & 15;
        if (kind != 5 && kind - 10 >= 2)
        {
            animation_data = r_u32(character_offset + 780);
            flags = r_u32(object_offset + 28);
            value = r_u32(animation_data + 108);
            w_u16(object_offset + 130, 4);
            w_u32(object_offset + 28, (flags & 0xFFFEFFFF) | 0x20000);
            w_u32(object_offset + 104, value);
            goto finish;
        }
        animation = r_s8(character_offset + 712);
    }
    if (animation == 20)
    {
        animation_data = r_u32(character_offset + 780);
        flags = r_u32(object_offset + 28);
        value = r_u32(animation_data + 160);
        goto set_animation_data;
    }
    if (r_u8(character_offset + 294) != 0)
    {
        animation_data = r_u32(character_offset + 780);
        flags = r_u32(object_offset + 28);
        value = r_u32(animation_data + 92);
        goto set_animation_data;
    }
    if ((r_u32(character_offset + 4) & 0x40) == 0)
    {
        voice_owner = character_offset != 0 ? character_offset : 0x800ABD68;
        sound = r_u32(0x8008A408);
        value = sound != 0 && r_s32(sound) >= 7 ? sound + 52 : 0;
        result = p2_func_8005e868(value, (uint32)(sint32)r_s8(voice_owner + 912)) << 24;
        if (result == 0)
        {
            sound = r_u32(0x8008A408);
            value = sound != 0 && r_s32(sound) >= 11 ? sound + 84 : 0;
            result = p2_func_8005e868(value, (uint32)(sint32)r_s8(voice_owner + 912)) << 24;
        }
        if (result != 0)
        {
            p2_func_8006a970((uint16)(sint16)r_s8(voice_owner + 912));
            w_u8(voice_owner + 912, 0xFF);
        }
    }
    animation = r_s8(character_offset + 712);
    if (animation == 0)
    {
        previous = r_s8(character_offset + 713);
        if ((uint32)previous - 1 < 34)
            w_u16(character_offset + 62, 900);
        animation = r_s8(character_offset + 712);
    }
    animation_data = r_u32(character_offset + 780);
    value = r_u32(animation_data + 80 + (uint32)animation * 4);
    flags = r_u32(object_offset + 28);
    w_u16(object_offset + 130, 0xFFFF);
    w_u32(object_offset + 28, flags | 0x10000);
    w_u32(object_offset + 104, value);
    animation = r_s8(character_offset + 712);
    if (animation == 8)
    {
        if ((r_u32(character_offset + 4) & 0x40) == 0)
        {
            value = r_u32(0x8008A408);
            sound = value != 0 && r_s32(value) >= 7 ? value + 52 : 0;
            voice = p2_func_8005e628(0xFFFFFFFF, sound, 1);
            w_u8(character_offset + 912, voice);
            voice = (sint8)voice;
            if (voice >= 0)
            {
                value = r_u16(sound + 6);
                pitch = r_u16(sound + 4);
                volume = r_s16(0x8008A24C);
                p2_func_8005e470(sound, (uint16)voice, 0, value, pitch, volume, 0);
            }
        }
    }
    else if (animation == 9 && (r_u16(character_offset + 706) & 15) == 1)
        p2_func_80044ffc(4, 3);
    else if ((r_u32(object_offset + 228) & 0x40000) != 0 && r_s8(character_offset + 252) == 8)
        w_u32(object_offset + 28, r_u32(object_offset + 28) & 0xFFFEFFFF);
    goto finish;
set_animation_data:
    w_u32(object_offset + 28, flags & 0xFFFEFFFF);
    w_u32(object_offset + 104, value);
finish:
    flags = r_u32(object_offset + 228);
    result = 0xFFDF0000;
    if ((flags & 0x200000) != 0)
    {
        w_u32(object_offset + 228, flags & 0xFFDFFFFF);
        result = r_u32(object_offset + 28);
        flags = r_u32(object_offset + 228);
        result &= 0xFFFEFFFF;
        w_u32(object_offset + 28, result);
        w_u32(object_offset + 228, flags & 0xFFBFFFFF);
    }
    return result;
}

uint32 p2_func_800515f0(uint32 object_offset, uint32 character_offset)
{
    uint32 saved_stack = p2_guest_stack_pointer;
    uint32 input, relative, transpose, parent, matrix, flags, original_flags, value, x, y, z, sound, voice_owner, result, kind, spawned, eject, angle, pitch;
    uint64 encoded;
    sint32 timer, voice, volume, wave;
    p2_guest_stack_pointer -= 0x60;
    input = p2_guest_stack_pointer + 0x20;
    relative = p2_guest_stack_pointer + 0x38;
    transpose = p2_guest_stack_pointer + 0x48;
    if ((r_u32(character_offset + 4) & 0x100) != 0)
    {
        if ((r_u32(object_offset + 28) & 0x1000000) != 0)
        {
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
                p2_sdk_8007ec64(input, 0x800B1510);
                PopMatrix();
                matrix = r_u32(r_u32(object_offset + 132) + 40);
                x = r_u32(0x800B1510);
                w_u32(0x800B1510, x + r_u32(matrix + 20));
                matrix = r_u32(r_u32(object_offset + 132) + 40);
                y = r_u32(0x800B1514);
                w_u32(0x800B1514, y + r_u32(matrix + 24));
                matrix = r_u32(r_u32(object_offset + 132) + 40);
                z = r_u32(0x800B1518);
                w_u32(0x800B1518, z + r_u32(matrix + 28));
            }
            else
            {
                x = r_u32(object_offset + 16);
                y = r_u32(object_offset + 20);
                z = r_u32(object_offset + 24);
                w_u32(0x800B1510, x);
                w_u32(0x800B1514, y);
                w_u32(0x800B1518, z);
            }
        }
        p2_func_8005a7f4(object_offset, character_offset);
    }
    flags = r_u32(character_offset + 4);
    if ((flags & 0x40) != 0)
    {
        value = (uint32)r_u16(character_offset + 64) - 1;
        w_u16(character_offset + 64, value);
        encoded = psx_double_bits(__floatsidf((sint16)value));
        if (p2_func_8006ec24((uint32)encoded, (uint32)(encoded >> 32), 0, 0x407B3000) == 0)
        {
            voice_owner = character_offset != 0 ? character_offset : 0x800ABD68;
            sound = r_u32(0x8008A408);
            value = sound != 0 && r_s32(sound) >= 7 ? sound + 52 : 0;
            result = p2_func_8005e868(value, (uint32)(sint32)r_s8(voice_owner + 912));
            if ((result << 24) == 0)
            {
                sound = r_u32(0x8008A408);
                value = sound != 0 && r_s32(sound) >= 11 ? sound + 84 : 0;
                result = p2_func_8005e868(value, (uint32)(sint32)r_s8(voice_owner + 912));
            }
            if ((result << 24) != 0)
            {
                p2_func_8006a970((uint16)(sint16)r_s8(voice_owner + 912));
                w_u8(voice_owner + 912, 0xFF);
            }
            p2_func_80044ffc(9, 3);
            value = r_u32(0x8008A408);
            sound = value != 0 && r_s32(value) >= 11 ? value + 84 : 0;
            voice = p2_func_8005e628(0xFFFFFFFF, sound, 1);
            w_u8(character_offset + 912, voice);
            voice = (sint8)voice;
            if (voice >= 0)
            {
                value = r_u16(sound + 6);
                pitch = r_u16(sound + 4);
                volume = r_s16(0x8008A24C);
                p2_func_8005e470(sound, (uint16)voice, 0, value, pitch, volume, 0);
            }
        }
        if (r_s16(character_offset + 64) < 150)
        {
            value = r_u32(0x8008A198);
            w_u32(0x8008A198, value + 1);
            if ((sint32)value >= 15)
            {
                flags = r_u32(object_offset + 28);
                w_u32(0x8008A198, 0);
                w_u32(object_offset + 28, flags ^ 0x80000);
            }
        }
        if (r_s16(character_offset + 64) == 0)
        {
            parent = r_u32(character_offset + 780);
            value = r_u32(parent + 168);
            x = r_u32(parent + 88);
            w_u32(parent + 88, value);
            parent = r_u32(character_offset + 780);
            w_u32(parent + 168, x);
            p2_func_80027ac0(character_offset, 7);
            w_u32(object_offset + 28, r_u32(object_offset + 28) & 0xDFFFFFFF);
            w_u32(character_offset + 4, r_u32(character_offset + 4) & 0xFFFFFFBF);
            w_u32(object_offset + 28, r_u32(object_offset + 28) & 0xFFF7FFFF);
        }
    }
    else if ((flags & 0x200) != 0)
    {
        value = (uint32)r_u16(character_offset + 68) - 1;
        w_u16(character_offset + 68, value);
        timer = r_s16(character_offset + 68);
        w_u32(0x8008A474, (((16 * (value & 15) + 64) & 0xFF) << 16) | 0x2020);
        if (timer == 0)
        {
            w_u32(object_offset + 28, r_u32(object_offset + 28) & 0xDFF7FFFF);
            w_u32(character_offset + 4, r_u32(character_offset + 4) & 0xFFFFFDFF);
        }
    }
    flags = r_u32(object_offset + 28);
    if ((flags & 0x1000000) == 0)
    {
        timer = r_s16(character_offset + 52);
        result = (uint32)timer;
        if (timer != 0)
        {
            result = (uint32)timer - 1;
            w_u16(character_offset + 52, result);
        }
        goto done;
    }
    value = r_u32(0x8008A4CC);
    result = value - 1;
    if ((sint32)value > 0)
    {
        w_u32(0x8008A4CC, result);
        goto done;
    }
    w_u32(object_offset + 28, flags & 0xFEFFFFFF);
    flags = r_u32(character_offset + 8);
    kind = r_u16(character_offset + 706);
    w_u16(character_offset + 52, 15);
    w_u32(character_offset + 8, flags | 0x1000000);
    switch (kind & 15)
    {
        case 1:
            w_u32(object_offset + 116, 19);
            flags = r_u32(character_offset + 4);
            w_u32(0x8008A178, 19);
            if ((flags & 0x20) != 0)
                w_u32(0x8008A178, 51);
            break;
        case 4:
            w_u32(object_offset + 116, 21);
            break;
        case 5:
        case 10:
        case 11:
            w_u32(object_offset + 116, 25);
            break;
        default:
            w_u32(object_offset + 116, r_u16(character_offset + 22));
            if ((r_u16(character_offset + 706) & 15) == 0)
                w_u32(0x8008A498, r_u32(0x8008A498) | 0x10);
            break;
    }
    value = r_u32(object_offset + 116);
    original_flags = r_u32(object_offset + 28);
    if ((value == 19 || r_u32(0x8008A498) != 0) && (r_u32(r_u32(object_offset + 132) + 16) & 0x10000) != 0)
    {
        timer = r_s16(object_offset + 148);
        wave = r_s16(character_offset + 266);
        w_u32(object_offset + 28, timer == wave ? original_flags & 0xFFFFFFEF : original_flags | 0x10);
    }
    w_u32(object_offset + 28, r_u32(object_offset + 28) | 0x100);
    spawned = p2_dispatch_object_factory(r_u32(0x800BA398), object_offset);
    w_u32(object_offset + 28, original_flags);
    kind = r_u16(character_offset + 706) & 15;
    eject = kind - 5 < 2 || kind == 7 || kind - 10 < 2;
    if (eject == 0)
    {
        if (kind == 4)
        {
            result = r_u32(spawned + 20) + ((original_flags & 0x8000000) != 0 ? 4096 : 20480);
            w_u32(spawned + 20, result);
            goto done;
        }
        result = original_flags & 0x8000000;
        if (result == 0)
            goto done;
    }
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
        parent = r_u32(spawned + 132);
        matrix = r_u32(parent + 40);
        x = r_u32(spawned + 16);
        y = r_u32(matrix + 20);
        parent = r_u32(spawned + 132);
        w_u32(spawned + 16, x + y);
        matrix = r_u32(parent + 40);
        x = r_u32(spawned + 20);
        y = r_u32(matrix + 24);
        parent = r_u32(spawned + 132);
        w_u32(spawned + 20, x + y);
        matrix = r_u32(parent + 40);
        z = r_u32(spawned + 24);
        w_u32(spawned + 24, z + r_u32(matrix + 28));
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
    if (eject != 0)
    {
        angle = (r_u16(object_offset + 150) + 2048) & 0xFFF;
        wave = p2_func_8005dbb8((sint16)angle) >> 2;
        x = r_u32(spawned + 16);
        w_u32(spawned + 16, x + (uint32)(((sint64)12288 * wave) >> 12));
        wave = p2_func_8005dbb8((sint16)(1024 - angle)) >> 2;
        z = r_u32(spawned + 24);
        w_u32(spawned + 24, z + (uint32)(((sint64)12288 * wave) >> 12));
        w_u32(spawned + 16, r_u32(spawned + 16) + r_u32(object_offset + 92));
        w_u32(spawned + 20, r_u32(spawned + 20) + r_u32(object_offset + 96));
        w_u32(spawned + 24, r_u32(spawned + 24) + r_u32(object_offset + 100));
        result = r_u32(spawned + 28) | 0x2000;
        w_u32(spawned + 28, result);
        w_u8(spawned + 124, 0);
        goto done;
    }
    w_u32(spawned + 20, r_u32(spawned + 20) - 8192);
    parent = r_u32(spawned + 132);
    if (parent == 0)
    {
        result = spawned;
        goto done;
    }
    matrix = r_u32(parent + 40);
    if (matrix == 0)
    {
        result = spawned;
        goto done;
    }
    w_u32(relative, r_u32(spawned + 16) - r_u32(matrix + 20));
    matrix = r_u32(r_u32(spawned + 132) + 40);
    w_u32(relative + 4, r_u32(spawned + 20) - r_u32(matrix + 24));
    matrix = r_u32(r_u32(spawned + 132) + 40);
    w_u32(relative + 8, r_u32(spawned + 24) - r_u32(matrix + 28));
    w_u16(transpose, r_u16(r_u32(r_u32(spawned + 132) + 40)));
    w_u16(transpose + 2, r_u16(r_u32(r_u32(spawned + 132) + 40) + 6));
    w_u16(transpose + 4, r_u16(r_u32(r_u32(spawned + 132) + 40) + 12));
    w_u16(transpose + 6, r_u16(r_u32(r_u32(spawned + 132) + 40) + 2));
    w_u16(transpose + 8, r_u16(r_u32(r_u32(spawned + 132) + 40) + 8));
    w_u16(transpose + 10, r_u16(r_u32(r_u32(spawned + 132) + 40) + 14));
    w_u16(transpose + 12, r_u16(r_u32(r_u32(spawned + 132) + 40) + 4));
    w_u16(transpose + 14, r_u16(r_u32(r_u32(spawned + 132) + 40) + 10));
    w_u16(transpose + 16, r_u16(r_u32(r_u32(spawned + 132) + 40) + 16));
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
    result = p2_sdk_8007ec64(relative, spawned + 16);
    PopMatrix();
done:
    p2_guest_stack_pointer = saved_stack;
    return result;
}

uint32 p2_func_80050c94(uint32 object_offset, uint32 character_offset)
{
    uint32 saved_stack, position, floor, input, event, parent, matrix, scene, flags, value, x, y, z, source;
    sint32 frame, index;
    if ((r_u32(character_offset) & 2) == 0)
        return 0;
    saved_stack = p2_guest_stack_pointer;
    p2_guest_stack_pointer -= 0x48;
    position = p2_guest_stack_pointer + 0x18;
    floor = p2_guest_stack_pointer + 0x28;
    input = p2_guest_stack_pointer + 0x38;
    event = r_u32(character_offset + 116);
    if (r_s8(event + 15) == 17)
    {
        p2_func_80048af0(object_offset, character_offset, 0);
        event = r_u32(character_offset + 116);
        value = (uint32)p2_func_8002dfc8(r_u32(event + 68), 0);
        flags = r_u32(0x1F800320);
        w_u8(0x8008A134, value);
        w_u32(0x1F800320, flags | 8);
        goto finish;
    }
    w_u32(character_offset + 784, 0);
    if ((r_u32(object_offset + 28) & 0x2400) == 0x400)
    {
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
            matrix = r_u32(r_u32(object_offset + 132) + 40);
            x = r_u32(position);
            w_u32(position, x + r_u32(matrix + 20));
            matrix = r_u32(r_u32(object_offset + 132) + 40);
            y = r_u32(position + 4);
            w_u32(position + 4, y + r_u32(matrix + 24));
            matrix = r_u32(r_u32(object_offset + 132) + 40);
            z = r_u32(position + 8);
            w_u32(position + 8, z + r_u32(matrix + 28));
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
        frame = r_s16(object_offset + 136);
        flags = r_u32(object_offset + 28);
        parent = r_u32(object_offset + 132);
        if (p2_func_8003d3fc(parent, frame, position, floor, flags & 0x10, 4096) >= 0)
            w_u32(character_offset + 784, r_u32(position + 4) - r_u32(floor + 4));
    }
    event = r_u32(character_offset + 116);
    if (r_u32(event + 76) != 0)
    {
        while (p2_func_80069ac0() != 0)
        {
        }
        source = p2_func_80025b7c(0);
        event = r_u32(character_offset + 116);
        index = p2_func_80022a38(source, event + 68);
        event = r_u32(character_offset + 116);
        scene = r_u32(0x8008A4E8);
        value = r_u32(event + 72);
        p2_func_80025674(0, source, scene, index, value);
    }
    else
    {
        parent = r_u32(character_offset + 780);
        flags = r_u32(object_offset + 28);
        value = r_u32(parent + 88);
        w_u16(object_offset + 130, 0xFFFF);
        w_u32(object_offset + 28, flags | 0x10000);
        w_u32(object_offset + 104, value);
        w_u32(0x1F800320, r_u32(0x1F800320) | 0x1000);
    }
finish:
    p2_func_8001c684(character_offset);
    w_u32(character_offset, r_u32(character_offset) & 0xFFFFFFDD);
    p2_guest_stack_pointer = saved_stack;
    return 1;
}

uint32 p2_func_800457c8(uint32 object_offset)
{
    uint32 saved_stack = p2_guest_stack_pointer;
    uint32 delta, position, scene, direction, flags, parent, matrix, destination, current, x, y, z, result;
    sint32 frame;
    p2_guest_stack_pointer -= 0x30;
    delta = p2_guest_stack_pointer + 0x10;
    position = p2_guest_stack_pointer + 0x20;
    scene = r_u32(0x8008A114);
    flags = r_u32(object_offset + 28);
    direction = flags & 0x10;
    w_u8(scene, r_u8(scene) & 0xFE);
    if (p2_func_80026c14() != 0)
    {
        if (p2_func_80026d80() == 0)
        {
            w_u32(0x800AC024, 0);
            w_u32(0x800AC020, 0);
            w_u32(0x800AC01C, 0);
            p2_guest_stack_pointer = saved_stack;
            return 1;
        }
        w_u16(0x800AC012, 1);
    }
    flags = r_u32(object_offset + 228);
    if (((flags & 0x100) != 0 && r_s8(0x8008A1BF) != 0) || (flags & 0x40000) != 0)
    {
        scene = r_u32(0x8008A114);
        flags = r_u32(0x800ABD68);
        y = r_u32(scene + 20);
        w_u32(scene + 20, y - ((sint32)flags >= 0 ? 14336 : 32768));
        p2_func_80059aec(position, object_offset);
        scene = r_u32(0x8008A114);
        x = r_u32(position);
        y = r_u32(position + 4);
        z = r_u32(position + 8);
        w_u32(scene + 16, x);
        w_u32(scene + 20, y);
        w_u32(scene + 24, z);
        goto select_position;
    }
    if ((r_u32(object_offset + 28) & 0x2000) != 0)
        goto local_position;
    parent = r_u32(object_offset + 132);
    if (parent == 0 || r_u32(parent + 40) == 0)
        goto local_position;
    x = r_u32(object_offset + 16);
    y = r_u32(object_offset + 20);
    z = r_u32(object_offset + 24);
    w_u32(position, x);
    w_u32(position + 4, y);
    w_u32(position + 8, z);
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
    scene = r_u32(0x8008A114);
    p2_sdk_8007ec64(position, scene + 16);
    PopMatrix();
    scene = r_u32(0x8008A114);
    matrix = r_u32(r_u32(object_offset + 132) + 40);
    x = r_u32(scene + 16);
    w_u32(scene + 16, x + r_u32(matrix + 20));
    matrix = r_u32(r_u32(object_offset + 132) + 40);
    y = r_u32(scene + 20);
    w_u32(scene + 20, y + r_u32(matrix + 24));
    matrix = r_u32(r_u32(object_offset + 132) + 40);
    z = r_u32(scene + 24);
    w_u32(scene + 24, z + r_u32(matrix + 28));
    goto select_position;
local_position:
    scene = r_u32(0x8008A114);
    x = r_u32(object_offset + 16);
    y = r_u32(object_offset + 20);
    z = r_u32(object_offset + 24);
    w_u32(scene + 16, x);
    w_u32(scene + 20, y);
    w_u32(scene + 24, z);
select_position:
    scene = r_u32(0x8008A114);
    destination = (r_u32(scene + 72) & 0xFFFFFF) != 0 ? scene + 28 : scene + 4;
    if (r_s16(0x800AC012) != 0)
        goto update_path;
    scene = r_u32(0x8008A114);
    if (destination == scene + 28)
        goto update_path;
    current = r_u32(scene + 136);
    if (r_u32(current + 32) != 0)
        goto update_path;
    if ((r_u16(current + 14) & 1) != 0 && (r_u32(object_offset + 28) & 0x400) != 0)
        goto update_path;
    if ((r_u32(object_offset + 228) & 0x212) == 0)
        goto compute_delta;
update_path:
    scene = r_u32(0x8008A114);
    current = r_u32(scene + 136);
    frame = r_s16(scene + 2);
    p2_func_800451cc(current, frame, destination, direction);
compute_delta:
    if ((r_u32(object_offset + 28) & 0x2000) != 0)
    {
        flags = r_u32(0x800ABDBC);
        x = r_u32(object_offset + 92);
        y = r_u32(object_offset + 96);
        z = r_u32(object_offset + 100);
        w_u32(delta, x);
        w_u32(delta + 4, y);
        w_u32(delta + 8, z);
        if ((sint32)flags < 0)
        {
            x = r_u32(delta);
            y = r_u32(delta + 4);
            z = r_u32(delta + 8);
            w_u32(delta, 0 - x);
            w_u32(delta + 4, 0 - y);
            w_u32(delta + 8, 0 - z);
        }
    }
    else
    {
        scene = r_u32(0x8008A114);
        current = r_u32(scene + 136);
        frame = r_s16(scene + 2);
        p2_func_80022b20(current, frame, position, direction);
        scene = r_u32(0x8008A114);
        frame = r_s16(scene + 2);
        if (frame == 0)
        {
            x = r_u32(position);
            y = r_u32(position + 4);
            z = r_u32(position + 8);
            w_u32(delta, x);
            w_u32(delta + 4, y);
            w_u32(delta + 8, z);
            frame = r_s16(scene + 2);
            current = r_u32(scene + 136);
            p2_func_80022b20(current, frame + 1, position, direction);
        }
        else
        {
            current = r_u32(scene + 136);
            p2_func_80022b20(current, frame - 1, delta, direction);
        }
        x = r_u32(position) - r_u32(delta);
        y = r_u32(position + 4) - r_u32(delta + 4);
        z = r_u32(position + 8) - r_u32(delta + 8);
        w_u32(delta, x);
        w_u32(delta + 4, y);
        w_u32(delta + 8, z);
    }
    if ((r_u32(0x800ABD68) & 0x180000) != 0)
    {
        x = r_u16(object_offset + 148);
        flags = r_u32(object_offset + 28);
        x = (x + 1024) & 0xFFF;
        w_u16(object_offset + 148, x);
        w_u16(object_offset + 88, (x + 2048) & 0xFFF);
        w_u32(object_offset + 28, flags | 1);
    }
    else if ((r_u32(object_offset + 228) & 1) != 0)
        w_u16(object_offset + 88, (r_u16(object_offset + 148) + 2048) & 0xFFF);
    else
    {
        x = r_u32(delta);
        y = r_u32(delta + 4);
        z = r_u32(delta + 8);
        w_u32(0x800AC01C, 0 - x);
        w_u32(0x800AC020, 0 - y);
        w_u32(0x800AC024, 0 - z);
        p2_func_80040384(object_offset, delta, r_s16(0x800AC070));
    }
    if ((r_u32(object_offset + 228) & 0x100) != 0)
        p2_func_80059ae4(0x800ABD68);
    scene = r_u32(0x8008A114);
    flags = r_u32(0x800ABD68);
    y = r_u32(scene + 20);
    result = y + ((sint32)flags >= 0 ? 14336 : 32768);
    w_u32(scene + 20, result);
    p2_guest_stack_pointer = saved_stack;
    return result;
}

uint32 p2_func_800526f8(uint32 object_offset, uint32 character_offset)
{
    uint32 saved_stack, position, input, death_input, flags, other_flags, value, voice_owner, sound, result, parent, matrix, scene, x, y, z;
    if ((r_u32(0x1F800320) & 0x100) == 0)
        p2_func_800457c8(object_offset);
    if (r_s8(character_offset + 14) > 0)
        return 1;
    saved_stack = p2_guest_stack_pointer;
    p2_guest_stack_pointer -= 0x40;
    position = p2_guest_stack_pointer + 0x10;
    input = p2_guest_stack_pointer + 0x20;
    death_input = p2_guest_stack_pointer + 0x30;
    flags = r_u32(character_offset);
    other_flags = r_u32(character_offset + 4);
    w_u8(character_offset + 680, 0);
    w_u8(character_offset + 14, 0);
    w_u32(character_offset, flags & 0xFEFFBFDD);
    w_u32(character_offset + 4, other_flags & 0xFFFFFF7F);
    value = r_u8(object_offset + 13);
    flags = r_u32(object_offset + 28);
    w_u8(object_offset + 13, value & 0xFE);
    w_u32(object_offset + 28, flags & 0xFDF7FFFF);
    w_u16(character_offset + 48, 30);
    w_u8(object_offset + 124, 0);
    w_u8(object_offset + 126, 0);
    w_u8(object_offset + 127, 1);
    p2_func_80066430();
    voice_owner = character_offset != 0 ? character_offset : 0x800ABD68;
    sound = r_u32(0x8008A408);
    value = sound != 0 && r_s32(sound) >= 7 ? sound + 52 : 0;
    result = p2_func_8005e868(value, (uint32)(sint32)r_s8(voice_owner + 912));
    if ((result << 24) == 0)
    {
        sound = r_u32(0x8008A408);
        value = sound != 0 && r_s32(sound) >= 11 ? sound + 84 : 0;
        result = p2_func_8005e868(value, (uint32)(sint32)r_s8(voice_owner + 912));
    }
    if ((result << 24) != 0)
    {
        p2_func_8006a970((uint16)(sint16)r_s8(voice_owner + 912));
        w_u8(voice_owner + 912, 0xFF);
    }
    p2_func_8006abe4();
    value = r_u32(0x1F800320);
    flags = r_u32(object_offset + 28);
    w_u16(0x8008A532, 0);
    w_u32(0x1F800320, value | 0x100);
    if ((flags & 0x2000) != 0)
        goto local_position;
    parent = r_u32(object_offset + 132);
    if (parent == 0 || r_u32(parent + 40) == 0)
        goto local_position;
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
    matrix = r_u32(r_u32(object_offset + 132) + 40);
    x = r_u32(position);
    w_u32(position, x + r_u32(matrix + 20));
    matrix = r_u32(r_u32(object_offset + 132) + 40);
    y = r_u32(position + 4);
    w_u32(position + 4, y + r_u32(matrix + 24));
    matrix = r_u32(r_u32(object_offset + 132) + 40);
    z = r_u32(position + 8);
    w_u32(position + 8, z + r_u32(matrix + 28));
    goto position_ready;
local_position:
    x = r_u32(object_offset + 16);
    y = r_u32(object_offset + 20);
    z = r_u32(object_offset + 24);
    w_u32(position, x);
    w_u32(position + 4, y);
    w_u32(position + 8, z);
position_ready:
    w_u32(object_offset + 160, r_u32(object_offset + 60) - r_u32(position));
    w_u32(object_offset + 164, r_u32(object_offset + 64) - r_u32(position + 4));
    w_u32(object_offset + 168, r_u32(object_offset + 68) - r_u32(position + 8));
    if ((r_u32(character_offset) & 0x80) != 0)
    {
        w_u16(character_offset + 48, 70);
        w_u32(character_offset + 700, 40960);
        if ((r_u32(character_offset + 4) & 0x80) == 0)
            w_u8(character_offset + 712, 6);
        if ((r_u32(object_offset + 28) & 0x2000) == 0)
        {
            parent = r_u32(object_offset + 132);
            if (parent != 0 && r_u32(parent + 40) != 0)
            {
                x = r_u32(object_offset + 16);
                y = r_u32(object_offset + 20);
                z = r_u32(object_offset + 24);
                w_u32(death_input, x);
                w_u32(death_input + 4, y);
                w_u32(death_input + 8, z);
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
                p2_sdk_8007ec64(death_input, object_offset + 16);
                PopMatrix();
                parent = r_u32(object_offset + 132);
                matrix = r_u32(parent + 40);
                x = r_u32(object_offset + 16);
                y = r_u32(matrix + 20);
                parent = r_u32(object_offset + 132);
                w_u32(object_offset + 16, x + y);
                matrix = r_u32(parent + 40);
                x = r_u32(object_offset + 20);
                y = r_u32(matrix + 24);
                parent = r_u32(object_offset + 132);
                w_u32(object_offset + 20, x + y);
                matrix = r_u32(parent + 40);
                z = r_u32(object_offset + 24);
                w_u32(object_offset + 24, z + r_u32(matrix + 28));
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
            w_u32(object_offset + 28, r_u32(object_offset + 28) | 0x2000);
        }
    }
    else if ((r_u32(character_offset + 4) & 0x80) == 0)
        w_u8(character_offset + 712, 1);
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
    p2_guest_stack_pointer = saved_stack;
    return 0;
}

uint32 p2_func_800521bc(uint32 object_offset, uint32 character_offset)
{
    uint32 flags, value, kind;
    sint32 animation, limit;
    if ((r_u32(object_offset + 28) & 0x4000) != 0)
    {
        if (r_s32(character_offset + 28) < 0)
            p2_func_80044ffc(1, 3);
        flags = r_u32(character_offset + 8);
        w_u32(character_offset + 28, 0);
        w_u16(character_offset + 676, 0);
        w_u32(character_offset + 8, flags | 0x800000);
        w_u32(object_offset + 28, r_u32(object_offset + 28) & 0xEFFFBBFF);
        flags = r_u32(character_offset);
        if ((flags & 0x18000) != 0)
            w_u32(character_offset, (flags | 0x40) & 0xFFFE7FFF);
        flags = r_u32(character_offset);
        value = r_u32(character_offset + 24);
        limit = r_s32(character_offset + 740);
        w_u32(character_offset, flags & 0xC7F9FFFF);
        if (limit < (sint32)value)
            w_u32(character_offset + 24, limit);
    }
    if ((r_u32(object_offset + 228) & 0x100) != 0)
        p2_func_80057650(object_offset);
    flags = r_u32(object_offset + 28);
    if ((flags & 0x800) != 0)
        return p2_func_8004fa54(object_offset, 0);
    if ((r_u32(object_offset + 228) & 0x800) != 0)
    {
        w_u32(object_offset + 28, flags | 0x2000);
        return p2_func_8004fa54(object_offset, 0);
    }
    if ((flags & 0x400) == 0)
    {
        if ((r_u32(r_u32(object_offset + 132) + 16) & 0x11) != 0)
        {
            value = r_u32(character_offset + 768);
            flags = r_u32(character_offset);
            w_u32(character_offset + 32, 0xFFFFFFFF);
            w_u32(character_offset + 28, 0);
            w_u16(character_offset + 48, 0);
            w_u16(character_offset + 684, value);
            w_u32(character_offset, flags | 0x8000000);
            w_u32(object_offset + 28, r_u32(object_offset + 28) | 0x400);
        }
        value = r_u32(object_offset + 28) & 0x400;
        if (value == 0)
            return value;
    }
    animation = r_s8(character_offset + 712);
    if (animation == 6)
        return 6;
    if (animation == 3)
        return (uint32)animation - 33;
    if ((uint8)(animation - 33) < 2)
        return 15;
    if (animation == 15 || animation == 23)
        return 23;
    kind = r_u16(character_offset + 706) & 15;
    if ((kind == 0 || kind == 8) && animation == 9)
        return 9;
    flags = r_u32(object_offset + 28);
    if ((flags & 0x8000000) != 0 && (r_u16(character_offset + 706) & 15) == 4)
        return 4;
    if ((r_u16(character_offset + 706) & 15) == 5 && r_s8(character_offset + 712) == 7)
        return 7;
    if ((r_u32(character_offset + 4) & 0x400) != 0)
        return 0xF7FF0000;
    w_u32(object_offset + 28, flags & 0xF7FFFFFF);
    animation = r_s8(character_offset + 712);
    if (animation == 4)
        return 4;
    kind = r_u16(character_offset + 706) & 15;
    if ((kind == 5 || kind - 10 < 2) && (animation == 5 || animation == 9))
        return 9;
    if ((r_u16(character_offset + 706) & 15) == 1)
    {
        value = r_u32(object_offset + 28) & 0x4000000;
        if (value != 0)
            return value;
    }
    if ((r_u32(character_offset + 4) & 0x80) == 0)
        w_u8(character_offset + 712, 6);
    return 6;
}

uint32 p2_func_800524ac(uint32 object_offset, uint32 character_offset)
{
    uint32 saved_stack, position, floor, input, result, value, parent, flags, mask, matrix, x, y, z;
    sint32 elapsed;
    result = (uint32)(sint32)r_s8(character_offset + 14);
    if (result == 0)
        return result;
    value = r_u32(character_offset + 28);
    if (r_s32(character_offset + 732) < (sint32)value)
        return 1;
    result = r_u16(character_offset + 706) & 15;
    if (result == 5)
        return result;
    saved_stack = p2_guest_stack_pointer;
    p2_guest_stack_pointer -= 0x48;
    position = p2_guest_stack_pointer + 0x18;
    floor = p2_guest_stack_pointer + 0x28;
    input = p2_guest_stack_pointer + 0x38;
    if ((r_u32(object_offset + 28) & 0x2000) != 0)
        goto increment;
    parent = r_u32(object_offset + 132);
    flags = r_u32(parent + 16);
    mask = flags & 0x11;
    result = flags & 0x10;
    if (mask != 1)
    {
        if (result == 0)
            goto done;
        result = r_u32(r_u32(r_u32(0x8008A114) + 136) + 28);
        if (result != 0)
            goto done;
    }
    if (mask == 0)
        goto increment;
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
        matrix = r_u32(r_u32(object_offset + 132) + 40);
        x = r_u32(position);
        w_u32(position, x + r_u32(matrix + 20));
        matrix = r_u32(r_u32(object_offset + 132) + 40);
        y = r_u32(position + 4);
        w_u32(position + 4, y + r_u32(matrix + 24));
        matrix = r_u32(r_u32(object_offset + 132) + 40);
        z = r_u32(position + 8);
        w_u32(position + 8, z + r_u32(matrix + 28));
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
    elapsed = r_s16(object_offset + 136);
    flags = r_u32(object_offset + 28);
    parent = r_u32(object_offset + 132);
    if (p2_func_8003d3fc(parent, elapsed, position, floor, flags & 0x10, 4096) >= 0 && r_s32(position + 4) >= r_s32(floor + 4))
        w_u16(character_offset + 48, (uint32)r_u16(character_offset + 48) - 1);
increment:
    value = r_u16(character_offset + 48) + 1;
    w_u16(character_offset + 48, value);
    result = (sint16)value < 16;
    if (result == 0)
        w_u8(character_offset + 14, 0);
done:
    p2_guest_stack_pointer = saved_stack;
    return result;
}
