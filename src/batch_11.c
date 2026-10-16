#include "game.h"
#include <string.h>

uint32 p2_func_80045d0c(uint32 object_offset, uint32 character_offset)
{
    uint32 state, x, y, z, parent, matrix, saved_stack, vector, result;
    state = r_u32(0x8008A114);
    x = r_u32(state + 28);
    y = r_u32(state + 32);
    z = r_u32(state + 36);
    w_u32(state + 4, x);
    w_u32(state + 8, y);
    w_u32(state + 12, z);
    parent = r_u32(object_offset + 132);
    if (parent != 0 && r_u32(parent + 40) != 0)
    {
        saved_stack = p2_guest_stack_pointer;
        p2_guest_stack_pointer -= 16;
        vector = p2_guest_stack_pointer;
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
        state = r_u32(0x8008A114);
        p2_sdk_8007ec64(vector, state + 16);
        PopMatrix();
        state = r_u32(0x8008A114);
        matrix = r_u32(r_u32(object_offset + 132) + 40);
        w_u32(state + 16, r_u32(state + 16) + r_u32(matrix + 20));
        matrix = r_u32(r_u32(object_offset + 132) + 40);
        w_u32(state + 20, r_u32(state + 20) + r_u32(matrix + 24));
        matrix = r_u32(r_u32(object_offset + 132) + 40);
        w_u32(state + 24, r_u32(state + 24) + r_u32(matrix + 28));
        p2_guest_stack_pointer = saved_stack;
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
    result = r_u32(character_offset);
    y = r_u32(state + 20);
    w_u32(state + 20, y + ((sint32)result < 0 ? 0x8000 : 0x3800));
    w_u32(character_offset + 32, 0);
    w_u32(character_offset + 28, 0);
    w_u32(character_offset + 24, 0);
    w_u32(object_offset + 100, 0);
    w_u32(object_offset + 96, 0);
    w_u32(object_offset + 92, 0);
    w_u8(object_offset + 124, 1);
    result = r_u32(character_offset) & 0xF7FFFFFF;
    w_u32(character_offset, result);
    return result;
}

uint32 p2_func_80046628(uint32 object_offset, uint32 first, uint32 second, uint32 third, uint32 kind)
{
    uint32 saved_stack, initial, target, state, parent, matrix, x, y, z, value, direction, path, frame;
    uint32 flags, definition, duration, result, denominator;
    sint32 coefficient, sample_frame, reciprocal;
    sint64 product;
    if (kind == 87)
    {
        w_u32(0x8008A47C, 30);
        w_u32(0x8008A508, 15);
    }
    else if (kind == 66)
    {
        w_u32(0x8008A470, 30);
        w_u32(0x8008A4D4, 15);
    }
    value = r_u32(0x8008A178);
    if (value != 0)
        w_u32(0x8008A178, value | 0x40);
    p2_func_8001c684(0x800ABD68);
    if (r_u32(0x8008A1A4) != 0)
    {
        w_u8(0x800ABF1C, 0);
        p2_func_8005a540(0x800ABD68);
    }
    saved_stack = p2_guest_stack_pointer;
    p2_guest_stack_pointer -= 32;
    initial = p2_guest_stack_pointer;
    target = initial + 16;
    if ((r_u32(object_offset + 28) & 0x2000) == 0 && (parent = r_u32(object_offset + 132)) != 0 && r_u32(parent + 40) != 0)
    {
        x = r_u32(object_offset + 16);
        y = r_u32(object_offset + 20);
        z = r_u32(object_offset + 24);
        w_u32(initial, x);
        w_u32(initial + 4, y);
        w_u32(initial + 8, z);
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
        p2_sdk_8007ec64(initial, state + 16);
        PopMatrix();
        state = r_u32(0x8008A114);
        matrix = r_u32(r_u32(object_offset + 132) + 40);
        w_u32(state + 16, r_u32(state + 16) + r_u32(matrix + 20));
        matrix = r_u32(r_u32(object_offset + 132) + 40);
        w_u32(state + 20, r_u32(state + 20) + r_u32(matrix + 24));
        matrix = r_u32(r_u32(object_offset + 132) + 40);
        w_u32(state + 24, r_u32(state + 24) + r_u32(matrix + 28));
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
    w_u32(object_offset + 28, r_u32(object_offset + 28) & 0xEFFF93EF);
    x = r_u32(0x800ABD84);
    y = r_u32(0x800AC044);
    if ((sint32)y >= (sint32)x)
        w_u16(0x800ABD98, 0);
    path = first;
    frame = second;
    duration = third;
    flags = r_u16(path + 14);
    direction = flags & 0x10;
    if ((sint32)frame < 0)
    {
        frame = ~frame;
        if ((flags & 8) != 0)
            path = r_u32(path + 16);
        direction ^= 0x10;
    }
    w_u32(object_offset + 28, r_u32(object_offset + 28) | 0x20000 | direction);
    if ((r_u32(r_u32(path + 36) + 16) & 0x11) != 0)
    {
        x = r_u32(0x800AC068);
        flags = r_u32(0x800ABD68);
        w_u32(0x800ABD88, 0xFFFFFFFF);
        w_u32(0x800ABD84, 0);
        w_u16(0x800ABD98, 0);
        w_u16(0x800AC014, x);
        w_u32(0x800ABD68, flags | 0x08000000);
        if ((r_u32(0x800ABD6C) & 0x80) == 0)
            w_u8(0x800AC030, 6);
        w_u32(object_offset + 28, (r_u32(object_offset + 28) | 0x400) & 0xF7FFFFFF);
    }
    w_u32(object_offset + 132, r_u32(path + 36));
    coefficient = r_s16(r_u32(path + 36) + 36);
    if (coefficient != 0)
        frame = (uint32)((sint32)((uint32)coefficient * frame) >> 12);
    state = r_u32(0x8008A114);
    w_u16(state + 2, frame);
    w_u16(object_offset + 136, frame);
    w_u16(object_offset + 148, 4096);
    w_u32(0x800ABD68, r_u32(0x800ABD68) & 0xFCC5F459);
    p2_func_80024844(path);
    state = r_u32(0x8008A114);
    p2_func_80022c24(path, (sint32)frame, state + 28, direction);
    state = r_u32(0x8008A114);
    definition = r_u32(r_u32(state + 136) + 36);
    if (direction != 0)
        sample_frame = r_s16(state + 2);
    else
        sample_frame = (sint32)((uint32)(sint32)r_s16(definition + 26) - (uint32)(sint32)r_s16(state + 2) - 1);
    p2_func_80017364(sample_frame, definition + 20, object_offset + 16);
    w_u16(object_offset + 136, r_u16(r_u32(0x8008A114) + 2));
    parent = r_u32(object_offset + 132);
    if (parent != 0 && r_u32(parent + 40) != 0)
    {
        x = r_u32(object_offset + 16);
        y = r_u32(object_offset + 20);
        z = r_u32(object_offset + 24);
        w_u32(target, x);
        w_u32(target + 4, y);
        w_u32(target + 8, z);
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
        p2_sdk_8007ec64(target, object_offset + 92);
        PopMatrix();
        parent = r_u32(object_offset + 132);
        matrix = r_u32(parent + 40);
        x = r_u32(object_offset + 92);
        x += r_u32(matrix + 20);
        parent = r_u32(object_offset + 132);
        w_u32(object_offset + 92, x);
        matrix = r_u32(parent + 40);
        y = r_u32(object_offset + 96);
        y += r_u32(matrix + 24);
        parent = r_u32(object_offset + 132);
        w_u32(object_offset + 96, y);
        matrix = r_u32(parent + 40);
        w_u32(object_offset + 100, r_u32(object_offset + 100) + r_u32(matrix + 28));
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
    state = r_u32(0x8008A114);
    w_u32(object_offset + 92, r_u32(object_offset + 92) - r_u32(state + 16));
    w_u32(object_offset + 96, r_u32(object_offset + 96) - r_u32(state + 20));
    w_u32(object_offset + 100, r_u32(object_offset + 100) - r_u32(state + 24));
    w_u8(state + 74, duration);
    w_u8(state + 73, duration);
    w_u8(state + 72, duration);
    if (duration == 0)
    {
        x = r_u32(object_offset + 92) << 1;
        y = r_u32(object_offset + 96) << 1;
        z = r_u32(object_offset + 100) << 1;
        w_u32(object_offset + 60, x);
        w_u32(object_offset + 64, y);
        w_u32(object_offset + 68, z);
        p2_func_80045d0c(object_offset, 0x800ABD68);
        state = r_u32(0x8008A114);
        w_u8(state, r_u8(state) & 0xFE);
    }
    else
    {
        w_u16(0x800ABDA0, duration);
        x = r_u32(object_offset + 92);
        denominator = duration << 12;
        if (denominator == 0)
            xport_mips_break(7);
        reciprocal = 0x01000000 / (sint32)denominator;
        product = (sint64)(sint32)x * reciprocal;
        x = (uint32)((uint64)product >> 12);
        y = r_u32(object_offset + 96);
        w_u32(object_offset + 92, x);
        product = (sint64)(sint32)y * reciprocal;
        y = (uint32)((uint64)product >> 12);
        z = r_u32(object_offset + 100);
        w_u32(object_offset + 96, y);
        product = (sint64)(sint32)z * reciprocal;
        w_u32(object_offset + 100, (uint32)((uint64)product >> 12));
        state = r_u32(0x8008A114);
        flags = r_u32(0x800ABD68);
        y = r_u32(state + 20);
        w_u32(state + 20, y + ((sint32)flags < 0 ? 0x8000 : 0x3800));
        w_u8(object_offset + 13, r_u8(object_offset + 13) | 1);
        w_u8(object_offset + 124, 0);
        w_u32(0x800ABD68, r_u32(0x800ABD68) | 0x04000000);
    }
    result = p2_func_80022ec4(r_u32(object_offset + 132), 0x800ABDB0, (sint32)direction);
    p2_guest_stack_pointer = saved_stack;
    return result;
}

uint32 p2_func_80044780(void)
{
    return r_u16(0x800AC02A) & 15;
}

uint32 p2_func_8003420c(uint32 index)
{
    uint32 offset, asset, counter, base, object, table, header, saved_flags, node, value;
    sint32 callback_index;
    offset = p2_func_8006a3c4(r_u32(0x8008A46C));
    asset = r_u32(0x8008A46C);
    counter = r_u8(0x8008A330);
    base = r_u32(0x8008A488);
    object = base + offset;
    w_u8(0x8008A330, counter + 1);
    w_u32(0x8008B450 + ((uint32)(sint32)(sint8)counter << 2), object);
    value = p2_func_8006a3ac(asset + 4);
    if ((sint32)value <= 0)
        w_u32(object + 128, 0);
    else
    {
        offset = p2_func_8006a3c4(r_u32(0x8008A46C) + 4);
        base = r_u32(0x8008A488);
        value = p2_func_8006aa48(base + offset);
        w_u32(object + 128, value);
    }
    callback_index = r_s8(object + 61);
    table = callback_index < 44 ? 0x80083D6C : 0x80083D70;
    w_u32(object, r_u32(table + ((uint32)callback_index << 2)));
    callback_index = r_s8(object + 633);
    if (callback_index != -1)
        w_u32(object, r_u32(0x80083D6C + ((uint32)callback_index << 2)));
    asset = r_u32(0x8008A46C);
    saved_flags = r_u32(object + 48);
    offset = p2_func_8006a3c4(asset + 3);
    header = r_u32(0x8008A488) + offset;
    p2_func_80069f40(header, -1);
    offset = p2_func_8006a3c4(r_u32(0x8008A46C) + 1);
    node = r_u32(0x8008A488) + offset;
    w_u32(object + 48, node);
    p2_func_80069f64(node, header);
    offset = p2_func_8006a3c4(r_u32(0x8008A46C) + 2);
    base = r_u32(0x8008A488);
    node = r_u32(object + 48);
    p2_func_80069fd8(node, base + offset, object + 72);
    node = r_u32(object + 48);
    w_u8(node + 13, r_u8(node + 13) | 0x80);
    node = r_u32(object + 48);
    w_u32(node + 28, r_u32(node + 28) | 0x20024 | saved_flags);
    node = r_u32(object + 48);
    asset = r_u32(0x8008A46C);
    value = r_u32(node + 224);
    w_u32(0x8008A46C, asset + 5);
    w_u16(object + 70, value);
    return 1;
}

uint32 p2_func_80026b6c(uint32 object_offset)
{
    uint32 offset, result;
    sint32 index;
    index = r_s16(object_offset + 82);
    offset = (uint32)index << 4;
    if (index >= 32)
        return p2_sdk_80082a18((const char *)psx_addr(0x800100B0, sizeof("Invalid cut scene idx %d. camera %08x\n")), index, r_u32(object_offset + 68));
    w_u32(0x8008B228 + offset, r_u32(object_offset + 68));
    w_u32(0x8008B22C + offset, r_u32(object_offset + 72));
    w_u32(0x8008B224 + offset, r_u32(object_offset + 76));
    result = r_u16(object_offset + 80);
    w_u32(0x8008B220 + offset, result);
    return result;
}

uint32 p2_func_80069aac(uint32 scene)
{
    w_u8(0x8008A394, scene);
    w_u8(0x8008A49C, 0xFC);
    return 0xFFFFFFFC;
}

uint32 p2_func_80026c54(uint32 object_offset, uint32 contact)
{
    w_u32(0x8008B1FC, 1);
    w_u32(0x8008B218, object_offset);
    w_u32(0x8008B214, contact);
    return 1;
}

uint32 p2_func_80025be8(void)
{
    w_u16(0x800B99CE, 15);
    return 15;
}

uint32 p2_func_80055280(uint32 mode, uint32 scene_offset)
{
    uint32 character, target, first, second, result;
    character = 0x800ABD68 + mode * 928;
    p2_func_8001c684(character);
    result = r_u32(character + 4) & 0x40;
    if (result != 0)
    {
        target = r_u32(character + 780);
        first = r_u32(target + 168);
        second = r_u32(target + 88);
        w_u32(target + 88, first);
        result = r_u32(character + 780);
        w_u32(result + 168, second);
    }
    return result;
}

uint32 p2_func_8005c4f4(uint32 object_offset)
{
    uint32 table, voice, index, result;
    table = r_u32(0x8008A460);
    voice = r_u8(object_offset + 76);
    if (table != 0)
    {
        index = r_u32(object_offset + 68);
        result = r_u32(table);
        index &= 0xFFF;
        if (index < result)
            table += index * 8 + 4;
        else
            table = 0;
    }
    result = p2_func_8005e868(table, voice);
    if ((result << 24) != 0)
        p2_func_8006a970((uint16)voice);
    result = r_u32(object_offset + 76) | 0xFF;
    w_u32(object_offset + 76, result);
    return result;
}

uint32 p2_func_80030bd4(uint32 object_offset)
{
    uint32 sound, voice, result;
    sound = r_u32(object_offset + 68);
    voice = (uint32)(sint32)r_s16(object_offset + 66);
    result = p2_func_8005e868(sound, voice);
    if ((result << 24) != 0)
        p2_func_8006a970(r_u16(object_offset + 66));
    w_u32(object_offset + 68, 0);
    w_u16(object_offset + 66, 0xFFFF);
    return 0xFFFFFFFF;
}

uint32 p2_func_80025f58(void)
{
    uint32 node, scene, kind, flags, result;
    node = r_u32(0x8008A4EC);
    while (node != 0)
    {
        if (r_s8(node + 125) == 6)
            p2_func_8002c510(node);
        node = r_u32(node);
    }
    scene = r_u32(0x8008A4E8);
    node = r_u32(scene + 44);
    if (node != 0)
    {
        do
        {
            kind = r_u8(node + 15);
            if (kind - 40 >= 16)
                break;
            if ((sint8)kind == 50)
            {
                flags = r_u32(node + 72);
                if (((flags >> 17) & 0xFF) == 0 && (flags >> 25) == 0 && r_u8(node + 76) < 8)
                    p2_func_8005c4f4(node);
            }
            node = r_u32(node);
        } while (node != 0);
        scene = r_u32(0x8008A4E8);
    }
    node = r_u32(scene + 32);
    while (node != 0)
    {
        p2_func_800425f4(r_u32(r_u32(node + 132) + 44));
        node = r_u32(node);
    }
    result = r_u32(0x8008A4E8);
    node = r_u32(result + 68);
    while (node != 0)
    {
        result = p2_func_80030bd4(node);
        node = r_u32(node);
    }
    return result;
}

uint32 p2_func_80063e78(uint32 node_offset)
{
    uint32 saved_stack, corners, packet, matrix, x, y, z, value, sum, limit, result, context, end, start, link, bucket_offset;
    uint32 material, texture, flags, uv0, uv1, uv2, uv3, orientation;
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
    w_u16(corners + 4, 0);
    w_u16(corners + 12, 0);
    w_u16(corners + 20, 0);
    w_u16(corners + 28, 0);
    x = 0 - (value & 0xFFFF);
    height += 10;
    y = 0 - (uint32)height;
    w_u16(corners, x);
    w_u16(corners + 2, y);
    w_u16(corners + 8, value);
    w_u16(corners + 10, y);
    w_u16(corners + 16, value);
    w_u16(corners + 18, height);
    w_u16(corners + 24, x);
    w_u16(corners + 26, height);
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
    x = r_u32(0x1F8002B8);
    value = (uint32)((sint32)sum >> 2);
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
    w_u8(packet + 7, (r_u8(material + 19) & 3) | 0x2C);
    w_u32(packet + 12, uv0);
    w_u32(packet + 20, uv1);
    w_u32(packet + 28, uv2);
    w_u32(packet + 36, uv3);
    orientation = (r_u16(node_offset + 14) & 0x180) >> 7;
    switch (orientation)
    {
        case 0:
            w_u32(packet + 8, r_u32(corners));
            w_u32(packet + 16, r_u32(corners + 8));
            w_u32(packet + 24, r_u32(corners + 24));
            w_u32(packet + 32, r_u32(corners + 16));
            break;
        case 1:
            w_u32(packet + 8, r_u32(corners + 24));
            w_u32(packet + 16, r_u32(corners));
            w_u32(packet + 24, r_u32(corners + 16));
            w_u32(packet + 32, r_u32(corners + 8));
            break;
        case 2:
            w_u32(packet + 8, r_u32(corners + 16));
            w_u32(packet + 16, r_u32(corners + 24));
            w_u32(packet + 24, r_u32(corners + 8));
            w_u32(packet + 32, r_u32(corners));
            break;
        case 3:
            w_u32(packet + 8, r_u32(corners + 8));
            w_u32(packet + 16, r_u32(corners + 16));
            w_u32(packet + 24, r_u32(corners));
            w_u32(packet + 32, r_u32(corners + 24));
            break;
    }
    result = packet + 40;
    w_u32(0x8008A4F8, result);
done:
    p2_guest_stack_pointer = saved_stack;
    return result;
}

uint32 p2_func_80026d80(void)
{
    uint32 remaining, path, destination, frame, duration, target, index, offset, value, object;
    sint32 numerator;
    remaining = r_u32(0x8008B200) - 1;
    w_u32(0x8008B200, remaining);
    if (remaining != 0)
    {
        path = r_u32(0x8008B20C);
        destination = r_u32(0x8008A114);
        frame = r_u32(0x8008B210);
        p2_func_80022c24(path, (sint32)frame, destination + 4, 16);
        path = r_u32(0x8008B20C);
        destination = r_u32(0x8008A114);
        frame = r_u32(0x8008B210);
        p2_func_80022b20(path, (sint32)frame, destination + 16, 16);
        path = r_u32(0x8008B20C);
        duration = r_u32(0x8008B208);
        remaining = r_u32(0x8008B200);
        value = (uint32)(sint32)r_s16(path + 46);
        numerator = (sint32)(value * (duration - remaining));
        if (duration == 0)
            xport_mips_break(7);
        if (duration == 0xFFFFFFFF && (uint32)numerator == 0x80000000)
            xport_mips_break(6);
        w_u32(0x8008B210, numerator / (sint32)duration);
        return 0;
    }
    target = r_u32(0x8008B21C);
    if (target == 0xFFFFFFFF)
    {
        w_u32(0x8008B1FC, 0);
        object = p2_func_800536f0(0);
        w_u8(object + 13, r_u8(object + 13) & 0xFE);
        return 1;
    }
    target = r_u32(0x8008B21C);
    for (index = 0; index < 32; ++index)
    {
        if (r_u32(0x8008B228 + index * 16) == target)
            break;
    }
    offset = index * 16;
    if (index >= 32)
    {
        p2_sdk_80082a18((const char *)psx_addr(0x800100D8, sizeof("ERROR: Couldn't find camera %08x for next cut scene.\n")), target);
        w_u32(0x8008B1FC, 0);
        return 1;
    }
    value = r_u32(0x8008B224 + offset);
    w_u32(0x8008B208, value);
    w_u32(0x8008B200, value);
    value = r_u32(0x8008B220 + offset);
    w_u32(0x8008B1F8, value);
    if ((value & 2) != 0)
    {
        object = p2_func_800536f0(0);
        w_u8(object + 13, r_u8(object + 13) | 1);
    }
    value = r_u32(0x8008A4E8);
    target = r_u16(0x8008B21C);
    w_u32(0x8008B210, 0);
    value = p2_func_800229fc(value, (sint32)target);
    w_u32(0x8008B20C, value);
    w_u32(0x8008B21C, r_u32(0x8008B22C + offset));
    return 0;
}

uint32 p2_func_8002ab38(uint32 value)
{
    uint32 remainder, result, trial, count;
    remainder = 0;
    result = 0;
    count = 22;
    do
    {
        remainder = (remainder << 2) | (value >> 30);
        result <<= 1;
        trial = (result << 1) + 1;
        value <<= 2;
        if (remainder >= trial)
        {
            remainder -= trial;
            ++result;
        }
        --count;
    } while (count != 0);
    return result;
}

uint32 p2_func_80044ffc(sint32 action, uint32 audio_value)
{
    uint32 mode, table, sound, index, minimum;
    if (action == 2)
    {
        mode = r_u16(0x800AC02A) & 15;
        if (mode == 1 || mode == 7 || mode == 9 || mode == 14 || mode == 11)
            action = 3;
    }
    table = r_u32(0x8008A408);
    if (table != 0 && action < r_s32(table))
        sound = table + ((uint32)action << 3) + 4;
    else
        sound = 0;
    mode = r_u16(0x800AC02A) & 15;
    minimum = 0;
    index = 0;
    if (mode == 4)
    {
        if (action == 0)
        {
            minimum = 12;
            index = 11;
        }
        else if (action == 1)
        {
            minimum = 13;
            index = 12;
        }
        else if (action == 2)
        {
            minimum = 14;
            index = 13;
        }
        else if (action == 7)
        {
            minimum = 4;
            index = 3;
        }
    }
    else if (mode == 5 && action == 2)
    {
        minimum = 14;
        index = 13;
    }
    if (minimum != 0)
    {
        table = r_u32(0x8008A460);
        sound = 0;
        if (table != 0 && r_s32(table) >= (sint32)minimum)
            sound = table + index * 8 + 4;
    }
    mode = r_u16(0x8008A24C);
    return p2_func_8005e538(sound, 0, 0, mode, 0, audio_value);
}

uint32 p2_func_80046510(uint32 object_offset, uint32 path_offset, sint32 frame)
{
    uint32 state, result, direction, previous, definition, value;
    sint32 previous_frame;
    state = r_u32(0x8008A114);
    result = r_u32(state + 136);
    if (path_offset != result)
    {
        direction = r_u32(object_offset + 28) & 0x10;
        p2_func_800451cc(path_offset, frame, state + 28, direction);
        state = r_u32(0x8008A114);
        previous = r_u32(state + 136);
        definition = r_u32(path_offset + 36);
        if (definition == r_u32(previous + 36))
            direction ^= 0x10;
        else
            p2_func_80046228(object_offset, definition);
        state = r_u32(0x8008A114);
        if ((r_u32(state + 72) & 0xFFFFFF) == 0)
        {
            previous = r_u32(state + 136);
            value = r_u32(0x800AC018);
            if (previous == value && (r_u32(object_offset + 28) & 0x2000) == 0)
            {
                previous_frame = r_s16(object_offset + 136);
                if (previous_frame >= 0)
                    p2_func_800451cc(previous, previous_frame, state + 4, direction);
            }
        }
        result = p2_func_80024844(path_offset);
    }
    w_u16(object_offset + 136, frame);
    return result;
}

P2PathProbeResult p2_switch_path_definition_with_selection(uint32 object_offset, uint32 definition_offset, uint32 fallback_frame)
{
    uint32 result, parent, state, matrix, x, y, z, value, speed, angle, flags, secondary;
    sint32 kind;
    P2PathProbeResult output, probe;
    output.fallback_frame = fallback_frame;
    result = r_u32(object_offset + 28) & 0x2000;
    if (result != 0)
    {
        output.value = (sint32)(result);
        return output;
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
        output.value = (sint32)(result);
        return output;
    }
    result = r_u32(parent + 36);
    matrix = r_u32(result + 40);
    if (matrix == 0)
    {
        output.value = (sint32)(result);
        return output;
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
        output.value = (sint32)(1);
        return output;
    }
    if (x == 0 && y == 0 && z == 0)
    {
        output.value = (sint32)(0);
        return output;
    }
    if (y != 0)
    {
        value = r_u32(0x800ABD84);
        value += y;
        y = 0;
        w_u32(0x800ABD84, value);
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
        probe = p2_sqrt_with_selection(value >> 2, output.fallback_frame);
        speed = (uint32)probe.value << 1;
    }
    else
    {
        probe = p2_sqrt_with_selection(value, output.fallback_frame);
        speed = (uint32)probe.value;
    }
    output.fallback_frame = probe.fallback_frame;
    if (speed != 0)
    {
        angle = p2_func_8005dc00((sint32)z, (sint32)x);
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
    value = r_u32(0x800ABD80);
    flags = r_u32(0x800ABD68);
    secondary = r_u32(0x800ABD6C);
    w_u32(0x800ABD80, value + speed);
    w_u32(0x800ABD68, flags & 0xFF7FFFFF);
    w_u32(0x800ABD6C, secondary & 0xFFFFFF7F);
    {
        output.value = (sint32)(0xFFFFFF7F);
        return output;
    }
}

uint32 p2_func_80046228(uint32 object_offset, uint32 definition_offset)
{
    return (uint32)p2_switch_path_definition_with_selection(object_offset, definition_offset, 0).value;
}

uint32 p2_func_80024b3c(uint32 vector_offset)
{
    uint32 axis, value, state;
    sint32 count;
    for (axis = 0; axis < 3; ++axis)
    {
        value = r_u32(vector_offset + axis * 4);
        if ((sint32)value < 0)
            value = 0 - value;
        count = (sint32)value >> 12;
        if (count != 0)
        {
            count = (count >> 1) + 1;
            if (count >= 46)
                count = 45;
        }
        else
        {
            state = r_u32(0x8008A114);
            if (r_u8(state + 72 + axis) != 0)
                count = 1;
        }
        state = r_u32(0x8008A114);
        w_u8(state + 72 + axis, count);
    }
    return state;
}

uint32 p2_func_800471f4(uint32 object_offset, uint32 other_offset, uint32 contact)
{
    uint32 x, y, z, parent, matrix, saved_stack, bounds, delta, extent, result, value;
    if (r_s8(other_offset + 15) == 24 && (r_u32(object_offset + 228) & 4) != 0 && r_u32(0x800ABE34) == other_offset)
    {
        value = r_u8(0x8008A230);
        if (r_u32(0x800ABE78) < value)
            return 0;
    }
    if (contact != 0)
        return 1;
    if ((r_u32(object_offset + 28) & 0x2800) != 0)
        return 1;
    x = 0 - r_u32(other_offset + 48);
    y = 0 - r_u32(other_offset + 52);
    z = 0 - r_u32(other_offset + 56);
    parent = r_u32(object_offset + 132);
    if (parent != 0)
    {
        matrix = r_u32(parent + 40);
        if (matrix != 0 && (r_u32(object_offset + 28) & 0x2000) == 0)
        {
            x += r_u32(matrix + 32);
            matrix = r_u32(r_u32(object_offset + 132) + 40);
            y += r_u32(matrix + 36);
            matrix = r_u32(r_u32(object_offset + 132) + 40);
            z += r_u32(matrix + 40);
        }
    }
    if (x != 0 || y != 0 || z != 0)
        return 1;
    saved_stack = p2_guest_stack_pointer;
    p2_guest_stack_pointer -= 48;
    bounds = p2_guest_stack_pointer;
    delta = bounds + 16;
    extent = bounds + 32;
    w_u32(extent, contact);
    x = r_u32(object_offset + 160);
    y = r_u32(object_offset + 164);
    z = r_u32(object_offset + 168);
    w_u32(bounds, x);
    w_u32(bounds + 4, y);
    w_u32(bounds + 8, z);
    w_u32(bounds + 12, 0);
    x = r_u32(object_offset + 92);
    x -= r_u32(other_offset + 48);
    w_u32(delta, x);
    y = r_u32(object_offset + 96);
    y -= r_u32(other_offset + 52);
    w_u32(delta + 4, y);
    z = r_u32(object_offset + 100);
    z -= r_u32(other_offset + 56);
    w_u32(delta + 8, z);
    result = p2_func_80038990(bounds, other_offset, delta, extent, 0) != 0;
    p2_guest_stack_pointer = saved_stack;
    return result;
}

uint32 p2_func_80049928(uint32 object_offset, uint32 character_offset, uint32 flags_offset, uint32 mode, uint32 vector_offset, sint32 strength, sint32 limit)
{
    uint32 value, x, y, z, parent, matrix, dot, rx, ry, rz, nx, ny, nz, speed, coefficient;
    uint32 flags, vertical, minimum, angle, saved_stack, vector, result;
    sint64 product;
    value = (uint32)p2_func_8005dcd4(r_s32(vector_offset + 4), 4096);
    flags = r_u32(character_offset);
    if ((value & 0x3FF) - 64 < 0x380)
        flags |= 0x20000000;
    else
        flags &= 0xDFFFFFFF;
    w_u32(character_offset, flags);
    x = r_u32(object_offset + 92);
    y = r_u32(object_offset + 96);
    z = r_u32(object_offset + 100);
    if ((r_u32(flags_offset) & 8) == 0)
    {
        x = (uint32)(((sint64)(sint32)x * 3276) >> 12);
        y = (uint32)(((sint64)(sint32)y * 3276) >> 12);
        z = (uint32)(((sint64)(sint32)z * 3276) >> 12);
    }
    parent = r_u32(object_offset + 132);
    if (parent != 0)
    {
        matrix = r_u32(parent + 40);
        if (matrix != 0 && (r_u32(object_offset + 28) & 0x2000) == 0)
        {
            x += r_u32(matrix + 32);
            matrix = r_u32(r_u32(object_offset + 132) + 40);
            y += r_u32(matrix + 36);
            matrix = r_u32(r_u32(object_offset + 132) + 40);
            z += r_u32(matrix + 40);
        }
    }
    x -= r_u32(character_offset + 36);
    y -= r_u32(character_offset + 40);
    z -= r_u32(character_offset + 44);
    dot = (uint32)(((sint64)(sint32)x * r_s32(vector_offset)) >> 12);
    dot += (uint32)(((sint64)(sint32)y * r_s32(vector_offset + 4)) >> 12);
    dot += (uint32)(((sint64)(sint32)z * r_s32(vector_offset + 8)) >> 12);
    dot = 0 - (dot << 1);
    rx = (uint32)(((sint64)r_s32(vector_offset) * (sint32)dot) >> 12);
    ry = (uint32)(((sint64)r_s32(vector_offset + 4) * (sint32)dot) >> 12);
    rz = (uint32)(((sint64)r_s32(vector_offset + 8) * (sint32)dot) >> 12);
    rx += x;
    ry += y;
    rz += z;
    nx = (uint32)(((sint64)r_s32(vector_offset) * strength) >> 12);
    ny = (uint32)(((sint64)r_s32(vector_offset + 4) * strength) >> 12);
    nz = (uint32)(((sint64)r_s32(vector_offset + 8) * strength) >> 12);
    rx += nx;
    ry += ny;
    rz += nz;
    rx += r_u32(character_offset + 36);
    ry += r_u32(character_offset + 40);
    rz += r_u32(character_offset + 44);
    x = (uint32)((sint32)rx >> 6);
    y = (uint32)((sint32)ry >> 6);
    z = (uint32)((sint32)rz >> 6);
    value = x * x + y * y + z * z;
    if ((sint32)value < 0)
        speed = p2_sdk_8007e794(value >> 2) << 1;
    else
        speed = p2_sdk_8007e794(value);
    if (limit < (sint32)speed)
    {
        if (mode == 0xFFFFFFFF && ry + 12288 >= 0x6001)
        {
            value = (sint32)ry < 0 ? 0 - ry : ry;
            if (value == 0)
                xport_mips_break(7);
            coefficient = (uint32)(0x40000000 / (sint32)value);
            product = (sint64)12288 * (sint32)coefficient;
            coefficient = (uint32)((uint64)product >> 18);
            w_u32(0x1F80030C, coefficient);
            rx = (uint32)(((sint64)(sint32)rx * (sint32)coefficient) >> 12);
            ry = (uint32)(((sint64)(sint32)ry * (sint32)coefficient) >> 12);
            rz = (uint32)(((sint64)(sint32)rz * (sint32)coefficient) >> 12);
        }
        if (speed == 0)
            xport_mips_break(7);
        coefficient = (uint32)(0x40000000 / (sint32)speed);
        product = (sint64)limit * (sint32)coefficient;
        coefficient = (uint32)((uint64)product >> 18);
        w_u32(0x1F80030C, coefficient);
        rx = (uint32)(((sint64)(sint32)rx * (sint32)coefficient) >> 12);
        ry = (uint32)(((sint64)(sint32)ry * (sint32)coefficient) >> 12);
        rz = (uint32)(((sint64)(sint32)rz * (sint32)coefficient) >> 12);
    }
    w_u32(flags_offset, r_u32(flags_offset) & 0xFFFFFFF7);
    value = r_u16(character_offset + 706) & 15;
    w_u16(character_offset + 48, 0);
    w_u32(character_offset + 32, 0);
    if (value != 5 && (r_u32(character_offset + 4) & 0x80) == 0)
        w_u8(character_offset + 712, 3);
    w_u32(character_offset + 28, ry);
    w_u32(character_offset + 32, (sint32)ry < 0 ? 0xFFFFFFFF : 1);
    ry = 0;
    if ((r_u16(character_offset + 706) & 15) != 4 && ((r_u32(object_offset + 28) & 0x400) != 0 || (r_u32(object_offset + 228) & 4) != 0))
    {
        vertical = r_u32(character_offset + 28);
        minimum = r_u32(0x8008A188);
        if ((sint32)vertical < (sint32)minimum && (sint32)vertical > 0)
        {
            value = r_u32(character_offset + 24);
            coefficient = 0x40000000 / (sint32)vertical;
            value = (uint32)(((sint64)(sint32)value * (sint32)minimum) >> 12);
            product = (sint64)(sint32)value * (sint32)coefficient;
            value = (uint32)((uint64)product >> 18);
            w_u32(0x1F80030C, value);
            w_u32(character_offset + 24, value);
            w_u32(character_offset + 28, minimum);
        }
    }
    if ((r_u16(character_offset + 706) & 15) == 5)
        rz = 0;
    x = (uint32)((sint32)rx >> 6);
    y = (uint32)((sint32)ry >> 6);
    z = (uint32)((sint32)rz >> 6);
    value = x * x + y * y + z * z;
    if ((sint32)value < 0)
        speed = p2_sdk_8007e794(value >> 2) << 1;
    else
        speed = p2_sdk_8007e794(value);
    w_u32(character_offset + 24, speed);
    if (speed != 0)
    {
        angle = p2_func_8005dc00((sint32)rz, (sint32)rx);
        value = (uint32)(sint32)r_s16(object_offset + 150);
        angle = (uint32)(sint32)(sint16)angle - value - 1024;
        if ((angle & 0xFFF) >= 2048)
        {
            w_u32(character_offset + 24, 0 - r_u32(character_offset + 24));
            rx = 0 - rx;
            ry = 0 - ry;
            rz = 0 - rz;
        }
        flags = r_u32(object_offset + 28);
        if ((flags & 0x2000) != 0)
        {
            while (rx + 26754 > 0xD103 || ry + 26754 > 0xD103 || rz + 26754 > 0xD103)
            {
                rx = (uint32)((sint32)rx >> 1);
                ry = (uint32)((sint32)ry >> 1);
                rz = (uint32)((sint32)rz >> 1);
            }
            saved_stack = p2_guest_stack_pointer;
            p2_guest_stack_pointer -= 16;
            vector = p2_guest_stack_pointer;
            w_u32(vector, rx);
            w_u32(vector + 4, ry);
            w_u32(vector + 8, rz);
            VectorNormal((VECTOR *)psx_addr(vector, 12), (VECTOR *)psx_addr(vector, 12));
            x = r_u32(vector);
            y = r_u32(vector + 4);
            z = r_u32(vector + 8);
            w_u32(character_offset + 72, x);
            w_u32(character_offset + 76, y);
            w_u32(character_offset + 80, z);
            p2_guest_stack_pointer = saved_stack;
        }
        else
        {
            vertical = r_u32(character_offset + 28);
            value = r_u32(character_offset + 716);
            if ((sint32)value >= (sint32)vertical && (flags & 0x400) == 0)
            {
                w_u32(character_offset + 32, 1);
                w_u32(character_offset + 28, value);
                w_u16(character_offset + 48, 0);
                w_u16(object_offset + 130, 0xFFFF);
            }
        }
    }
    result = r_u32(object_offset + 28) | 0x400;
    w_u32(object_offset + 28, result);
    return result;
}

P2PathProbeResult p2_reverse_scene_path_with_selection(uint32 object_offset, uint32 character_offset, uint32 fallback_frame)
{
    uint32 state, path, result, flags, x, y, z, parent, matrix, frame, old_frame, heading;
    uint32 saved_stack, local, delta, direction, previous, definition, value;
    sint32 signed_frame;
    P2PathProbeResult output;
    output.fallback_frame = fallback_frame;
    state = r_u32(0x8008A114);
    path = r_u32(state + 136);
    result = r_u16(path + 14) & 8;
    if (result != 0)
        path = r_u32(path + 16);
    if (path == 0)
    {
        output.value = (sint32)result;
        return output;
    }
    result = r_u32(character_offset + 4) & 0x80;
    if (result != 0)
    {
        output.value = (sint32)result;
        return output;
    }
    w_u32(object_offset + 28, r_u32(object_offset + 28) ^ 0x10);
    x = r_u32(character_offset + 84);
    y = r_u32(character_offset + 24);
    w_u32(character_offset + 84, 0 - x);
    w_u32(character_offset + 24, 0 - y);
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
        x = r_u32(character_offset + 72);
        y = r_u32(character_offset + 76);
        z = r_u32(character_offset + 80);
        w_u32(character_offset + 72, 0 - x);
        w_u32(character_offset + 76, 0 - y);
        w_u32(character_offset + 80, 0 - z);
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
        /* TODO: Bind the original matrix-stack diagnostic exits */
        output.fallback_frame = xport_gte_read_translation(0);
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
        output.fallback_frame = p2_project_path_position_with_selection(path, signed_frame, state + 28, direction).fallback_frame;
        state = r_u32(0x8008A114);
        previous = r_u32(state + 136);
        definition = r_u32(path + 36);
        if (definition == r_u32(previous + 36))
            direction ^= 0x10;
        else
            output.fallback_frame = p2_switch_path_definition_with_selection(object_offset, definition, output.fallback_frame).fallback_frame;
        state = r_u32(0x8008A114);
        if ((r_u32(state + 72) & 0xFFFFFF) == 0)
        {
            previous = r_u32(state + 136);
            value = r_u32(0x800AC018);
            if (previous == value && (r_u32(object_offset + 28) & 0x2000) == 0)
            {
                signed_frame = r_s16(object_offset + 136);
                if (signed_frame >= 0)
                    output.fallback_frame = p2_project_path_position_with_selection(previous, signed_frame, state + 4, direction).fallback_frame;
            }
        }
        output.fallback_frame = p2_select_scene_path_with_selection(path, output.fallback_frame).fallback_frame;
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
    {
        output.value = (sint32)result;
        return output;
    }
}

uint32 p2_func_80046cbc(uint32 object_offset, uint32 character_offset)
{
    return (uint32)p2_reverse_scene_path_with_selection(object_offset, character_offset, 0).value;
}

uint32 p2_func_80056700(uint32 object_offset)
{
    uint32 flags, state;
    flags = r_u32(object_offset + 228);
    state = r_u32(object_offset + 28);
    w_u32(object_offset + 228, flags | 0x100);
    w_u32(object_offset + 28, state & 0xFFFFFBFF);
    flags = r_u32(0x800ABD68);
    w_u8(0x800ABE64, 1);
    w_u32(0x800ABE80, 0xFFFFFFFF);
    w_u8(0x800ABE8C, 0);
    w_u8(0x800ABE85, 0);
    w_u8(0x800ABE84, 0);
    w_u32(0x800ABD88, 0xFFFFFFFF);
    w_u32(0x800ABD68, flags & 0xE7FFFFBF);
    return 0xFFFFFFFF;
}

sint32 p2_func_8005814c(uint32 object_offset, uint32 character_offset, uint32 input_offset)
{
    uint32 saved_stack, current, target, first, second, data, path, matrix, x, y, z, flags, value;
    uint32 height, distance, dx, dz, angle, difference, result;
    sint32 first_frame, second_frame;
    saved_stack = p2_guest_stack_pointer;
    p2_guest_stack_pointer -= 64;
    current = p2_guest_stack_pointer;
    target = current + 16;
    first = current + 32;
    second = current + 48;
    data = r_u32(character_offset + 256);
    path = r_u32(data + 68);
    result = 0;
    if (r_u32(r_u32(path + 36) + 40) != 0)
    {
        flags = r_u32(object_offset + 228);
        p2_func_80022b20(path, 0, target, (flags & 0x400) == 0 ? 64 : 0);
    }
    else
    {
        x = r_u32(data + 88);
        y = r_u32(data + 92);
        z = r_u32(data + 96);
        w_u32(target, x);
        w_u32(target + 4, y);
        w_u32(target + 8, z);
    }
    value = r_u32(object_offset + 132);
    if (value != 0 && r_u32(value + 40) != 0)
    {
        x = r_u32(object_offset + 16);
        y = r_u32(object_offset + 20);
        z = r_u32(object_offset + 24);
        w_u32(first, x);
        w_u32(first + 4, y);
        w_u32(first + 8, z);
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
        p2_sdk_8007ec64(first, current);
        PopMatrix();
        matrix = r_u32(r_u32(object_offset + 132) + 40);
        x = r_u32(current);
        x += r_u32(matrix + 20);
        w_u32(current, x);
        matrix = r_u32(r_u32(object_offset + 132) + 40);
        y = r_u32(current + 4);
        y += r_u32(matrix + 24);
        w_u32(current + 4, y);
        matrix = r_u32(r_u32(object_offset + 132) + 40);
        z = r_u32(current + 8);
        z += r_u32(matrix + 28);
        w_u32(current + 8, z);
    }
    else
    {
        x = r_u32(object_offset + 16);
        y = r_u32(object_offset + 20);
        z = r_u32(object_offset + 24);
        w_u32(current, x);
        w_u32(current + 4, y);
        w_u32(current + 8, z);
    }
    y = r_u32(target + 4);
    y -= r_u32(current + 4);
    value = r_u32(0x8008A1C8);
    x = r_u32(current);
    height = y - value;
    x -= r_u32(target);
    z = r_u32(current + 8);
    value = r_u32(target + 8);
    if ((sint32)x < 0)
        x = 0 - x;
    y = r_u32(0x8008A1E0);
    z -= value;
    if ((sint32)z < 0)
        z = 0 - z;
    distance = x + z;
    if ((sint32)y < (sint32)distance)
    {
        result = 0xFFFFFFFF;
        goto done;
    }
    if ((r_u32(object_offset + 228) & 1) == 0)
    {
        if (r_s32(character_offset + 28) < 0 && r_s32(0x8008A1E8) < (sint32)height)
        {
            result = 0xFFFFFFFF;
            goto done;
        }
        if ((r_u32(object_offset + 228) & 1) != 0 && r_s32(0x8008A1E8) < (sint32)height)
        {
            result = 0xFFFFFFFF;
            goto done;
        }
    }
    else if (r_s32(0x8008A1E8) < (sint32)height)
    {
        result = 0xFFFFFFFF;
        goto done;
    }
    second_frame = 1;
    if ((r_u32(object_offset + 228) & 0x400) != 0)
    {
        first_frame = r_s16(r_u32(path + 36) + 26);
        second_frame = first_frame - 2;
        --first_frame;
    }
    else
        first_frame = 0;
    p2_func_80022b20(path, first_frame, first, 64);
    p2_func_80022b20(path, second_frame, second, 64);
    x = r_u32(second);
    dx = x - r_u32(first);
    y = r_u32(second + 4);
    y -= r_u32(first + 4);
    z = r_u32(second + 8);
    dz = z - r_u32(first + 8);
    angle = p2_func_8005dc00((sint32)(0 - dz), (sint32)(0 - dx));
    w_u16(character_offset + 392, angle);
    x = r_u32(0x800B14A0);
    y = r_u32(0x800B14A4);
    z = r_u32(0x800B14A8);
    w_u32(character_offset + 396, x);
    w_u32(character_offset + 400, y);
    w_u32(character_offset + 404, z);
    value = (uint32)(sint32)r_s16(object_offset + 148);
    difference = value - (uint32)(sint32)(sint16)angle;
    if ((sint32)difference < 0)
        difference = 0 - difference;
    if ((sint16)difference >= 2049)
        difference = 4096 - difference;
    w_u32(character_offset + 280, distance);
    flags = r_u32(object_offset + 228);
    if ((flags & 1) != 0)
    {
        if (r_s32(0x8008A1EC) >= (sint32)height)
            result = 1;
        goto done;
    }
    if (r_s32(character_offset + 28) > 0)
        goto done;
    if ((flags & 0x400) != 0)
    {
        if ((r_u32(object_offset + 28) & 0x10) != 0)
            goto done;
    }
    else if ((r_u32(object_offset + 28) & 0x10) == 0)
        goto done;
    if ((sint16)difference >= r_s16(0x8008A1DA))
        goto done;
    if ((sint32)height < r_s32(0x8008A1E4) || r_s32(0x8008A1E8) < (sint32)height)
        goto done;
    if ((r_u32(object_offset + 228) & 0x400) != 0)
    {
        if ((r_u32(input_offset) & 0x2000) == 0)
            result = 1;
    }
    else if ((r_u32(input_offset) & 0x8000) == 0)
        result = 1;
done:
    p2_guest_stack_pointer = saved_stack;
    return (sint32)result;
}

uint32 p2_func_80058d08(uint32 object_offset, uint32 character_offset, uint32 input_flags, uint32 mode, uint32 horizontal_offset, uint32 vertical_offset)
{
    uint32 flags, input, result, mask, animation;
    sint32 state, count, threshold;
    flags = r_u32(object_offset + 228);
    input = r_u32(0x1F800384);
    if ((flags & 0x200000) != 0 && (r_u32(character_offset + 4) & 0x80) == 0)
        w_u8(character_offset + 712, 3);
    flags = r_u32(object_offset + 228);
    if ((flags & 0x40000) != 0)
    {
        if (r_s8(character_offset + 252) != 6)
            return 6;
        if (r_s16(object_offset + 130) != 0)
            return 7;
        w_u8(character_offset + 252, 7);
        if ((r_u32(character_offset + 4) & 0x80) == 0)
            w_u8(character_offset + 712, 3);
        result = r_u32(object_offset + 228) | 0x200000;
        w_u32(object_offset + 228, result);
        return result;
    }
    if ((flags & 0x200000) != 0)
        return 1;
    state = r_s8(character_offset + 252);
    if (state == 1)
    {
        w_u32(character_offset + 28, 0);
        w_u32(vertical_offset, 0);
        if ((r_u32(character_offset + 4) & 0x240) != 0)
            w_u8(character_offset + 284, 0);
        count = r_s8(character_offset + 285);
        threshold = r_s8(0x8008A1D8);
        if (count < threshold)
        {
            w_u8(character_offset + 285, count + 1);
            mask = (r_u32(object_offset + 228) & 0x400) != 0 ? 0x8000 : 0x2000;
            if ((input & mask) == 0)
                w_u8(character_offset + 284, 1);
        }
        else if (count == threshold)
        {
            if (r_s8(character_offset + 284) == 0)
                w_u8(character_offset + 252, 2);
            else
            {
                mask = (r_u32(object_offset + 228) & 0x400) != 0 ? 0x8000 : 0x2000;
                if ((input & mask) != 0)
                    w_u8(character_offset + 252, 2);
            }
        }
        state = r_s8(character_offset + 252);
    }
    if (state == 2)
    {
        w_u32(character_offset + 28, 0);
        w_u32(vertical_offset, 0);
        w_u8(character_offset + 252, 3);
    }
    else if (state >= 3)
    {
        if (state == 3)
        {
            if (r_s8(character_offset + 712) == 20)
            {
                animation = r_u32(object_offset + 104);
                count = r_s16(animation + 6);
                threshold = r_s16(object_offset + 130);
                if (threshold != count - 1)
                {
                    w_u32(vertical_offset, 0);
                    w_u32(character_offset + 28, 0);
                    goto animation;
                }
            }
            w_u32(horizontal_offset, 0);
            w_u32(character_offset + 24, 0);
            w_u32(vertical_offset, 0);
            w_u32(character_offset + 28, 0);
        }
        w_u8(character_offset + 252, 4);
    }
    else if (state < 0)
        w_u8(character_offset + 252, 4);
animation:
    if ((r_u32(character_offset + 4) & 0x80) == 0)
        w_u8(character_offset + 712, 20);
    return 20;
}

void p2_func_80059ae4(uint32 character_offset)
{
}

uint32 p2_func_80056774(uint32 object_offset)
{
    uint32 result, trigger;
    w_u32(object_offset + 228, r_u32(object_offset + 228) & 0xFFFFFC7F);
    result = r_u32(0x800ABD68);
    trigger = r_u32(0x800ABF5C);
    result &= 0xF7FFFFFF;
    w_u32(0x800ABD68, result);
    if (trigger != 0)
        result = p2_func_800567d0(object_offset);
    return result;
}

uint32 p2_func_800567fc(uint32 object_offset)
{
    uint32 result;
    result = r_u32(object_offset + 228) & 0xFFFFFC37;
    w_u32(object_offset + 228, result);
    return result;
}

uint32 p2_func_80059aec(uint32 destination_offset, uint32 object_offset)
{
    uint32 saved_stack, local, world, operation, value, counter, state, parent, matrix;
    uint32 x, y, z, previous_x, previous_y, previous_z, dx, dy, dz, enabled;
    sint32 duration, threshold;
    saved_stack = p2_guest_stack_pointer;
    p2_guest_stack_pointer -= 32;
    local = p2_guest_stack_pointer;
    world = local + 16;
    operation = 0;
    if ((r_u32(object_offset + 228) & 0x40000) != 0)
    {
        if (r_s8(0x800AC030) == 20)
        {
            duration = r_s16(object_offset + 130) + 1;
            operation = 3;
        }
        else if ((r_u32(object_offset + 28) & 0x2000) != 0)
        {
            x = r_u32(object_offset + 16);
            y = r_u32(object_offset + 20);
            z = r_u32(object_offset + 24);
            goto publish;
        }
    }
    else
    {
        counter = r_u8(0x800ABE8C);
        threshold = r_s8(0x8008A1BF);
        if (counter < (uint32)threshold)
        {
            enabled = 0;
            if (r_s8(0x8008A1C0) == 0)
                enabled = 1;
            else if (r_s8(0x800AC030) == 20 && r_s16(object_offset + 130) != 0)
            {
                value = r_u32(object_offset + 104);
                counter = r_u8(0x8008A1B8);
                value = r_u8(value + 6);
                w_u8(0x8008A1BF, value - counter);
                enabled = 1;
            }
            if (enabled != 0)
            {
                threshold = r_s8(0x8008A1BF);
                counter = r_u8(0x800ABE8C);
                previous_x = r_u32(0x800ABEC0);
                previous_y = r_u32(0x800ABEC4);
                previous_z = r_u32(0x800ABEC8);
                duration = threshold - (sint32)counter + 1;
                if (duration <= 0)
                    duration = 1;
                ++counter;
                w_u8(0x800ABE8C, counter);
                if ((counter & 0xFF) != 0)
                {
                    state = r_u32(0x8008A114);
                    previous_x = r_u32(state + 16);
                    previous_y = r_u32(state + 20);
                    previous_z = r_u32(state + 24);
                }
                operation = 1;
            }
            else
            {
                state = r_u32(0x8008A114);
                value = r_u32(state + 136);
                value = r_u32(value + 36);
                if (r_u32(value + 40) == 0)
                {
                    x = r_u32(0x800ABECC);
                    y = r_u32(0x800ABED0);
                    z = r_u32(0x800ABED4);
                    goto publish;
                }
                operation = 2;
            }
        }
    }
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
        p2_sdk_8007ec64(local, world);
        PopMatrix();
        matrix = r_u32(r_u32(object_offset + 132) + 40);
        x = r_u32(world);
        x += r_u32(matrix + 20);
        w_u32(world, x);
        matrix = r_u32(r_u32(object_offset + 132) + 40);
        y = r_u32(world + 4);
        y += r_u32(matrix + 24);
        w_u32(world + 4, y);
        matrix = r_u32(r_u32(object_offset + 132) + 40);
        z = r_u32(world + 8);
        z += r_u32(matrix + 28);
        w_u32(world + 8, z);
    }
    else
    {
        x = r_u32(object_offset + 16);
        y = r_u32(object_offset + 20);
        z = r_u32(object_offset + 24);
    }
    if (operation == 1)
    {
        dx = x - previous_x;
        dy = y - previous_y;
        dz = z - previous_z;
        x = previous_x + (uint32)((sint32)dx / duration);
        y = previous_y + (uint32)((sint32)dy / duration);
        z = previous_z + (uint32)((sint32)dz / duration);
    }
    else if (operation == 2)
    {
        dx = x - r_u32(0x800ABE9C);
        dy = y - r_u32(0x800ABEA0);
        dz = z - r_u32(0x800ABEA4);
        x = r_u32(0x800ABED8);
        y = r_u32(0x800ABEDC);
        z = r_u32(0x800ABEE0);
        x += dx;
        y += dy;
        z += dz;
    }
    else if (operation == 3)
    {
        if (duration == 0)
            xport_mips_break(7);
        y += (uint32)(-32768 / duration);
    }
publish:
    w_u32(0x800ABECC, x);
    w_u32(0x800ABED0, y);
    w_u32(0x800ABED4, z);
    w_u32(destination_offset, x);
    w_u32(destination_offset + 4, y);
    w_u32(destination_offset + 8, z);
    p2_guest_stack_pointer = saved_stack;
    return destination_offset;
}

uint32 p2_func_80057208(uint32 object_offset, uint32 character_offset)
{
    uint32 flags, state, definition, matrix, x, y, z, saved_stack, position, floor, local, value;
    flags = r_u32(object_offset + 228);
    state = r_u32(object_offset + 28);
    w_u32(object_offset + 228, flags & 0xFFFFFC37);
    w_u32(object_offset + 28, state | 0x400);
    if (r_s8(character_offset + 712) == 20 && r_s16(object_offset + 130) == 0)
    {
        w_u32(object_offset + 228, r_u32(object_offset + 228) | 0x400000);
        w_u8(character_offset + 252, 10);
    }
    else
    {
        p2_func_80046cbc(object_offset, character_offset);
        saved_stack = p2_guest_stack_pointer;
        p2_guest_stack_pointer -= 48;
        position = p2_guest_stack_pointer;
        floor = position + 16;
        local = position + 32;
        state = r_u32(0x8008A114);
        definition = r_u32(r_u32(state + 136) + 36);
        if (definition != 0 && r_u32(definition + 40) != 0)
        {
            x = r_u32(object_offset + 16);
            y = r_u32(object_offset + 20);
            z = r_u32(object_offset + 24);
            w_u32(local, x);
            w_u32(local + 4, y);
            w_u32(local + 8, z);
            PushMatrix();
            state = r_u32(0x8008A114);
            matrix = r_u32(r_u32(r_u32(state + 136) + 36) + 40);
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
            state = r_u32(0x8008A114);
            matrix = r_u32(r_u32(r_u32(state + 136) + 36) + 40);
            x = r_u32(position);
            x += r_u32(matrix + 20);
            w_u32(position, x);
            matrix = r_u32(r_u32(r_u32(state + 136) + 36) + 40);
            y = r_u32(position + 4);
            y += r_u32(matrix + 24);
            w_u32(position + 4, y);
            matrix = r_u32(r_u32(r_u32(state + 136) + 36) + 40);
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
        x = r_u32(position);
        y = r_u32(position + 4);
        z = r_u32(position + 8);
        w_u32(floor, x);
        w_u32(floor + 4, y);
        w_u32(floor + 8, z);
        w_u32(position + 4, r_u32(position + 4) - 0x4000);
        flags = r_u32(object_offset + 228);
        state = r_u32(0x8008A114);
        w_u32(object_offset + 228, flags | 0x02200000);
        p2_func_8003af18(object_offset, r_u32(state + 136), position, floor);
        state = r_u32(0x8008A114);
        value = r_u32(state + 136);
        if (value == r_u32(character_offset + 408))
            w_u32(object_offset + 28, r_u32(object_offset + 28) | 0x2000);
        p2_guest_stack_pointer = saved_stack;
    }
    value = r_u8(0x8008A1BB);
    w_u8(character_offset + 294, value);
    return value;
}

uint32 p2_func_80025b30(uint32 index)
{
    if (r_u32(0x8008B1D4 + index * 36) != 0)
        return 1;
    return r_u16(r_u32(0x8008A4E4) + 262) != 0;
}

uint32 p2_func_80025b7c(uint32 index)
{
    uint32 offset, first, result;
    offset = index * 36;
    first = r_u32(0x8008B1D0 + offset);
    result = r_u32(0x8008B1D4 + offset);
    w_u32(0x8008B1D4 + offset, first);
    w_u32(0x8008B1D0 + offset, result);
    return result;
}

sint32 p2_func_80022a38(uint32 scene_offset, uint32 name_offset)
{
    uint32 node, kind, flags;
    node = r_u32(scene_offset + 40);
    while (node != 0)
    {
        if (strncmp((const char *)psx_addr(node + 60, 2), (const char *)psx_addr(name_offset, 2), 2) == 0)
        {
            kind = r_u8(node + 62);
            if (kind != 60 && kind != 62)
            {
                flags = r_u16(node + 14);
                if ((flags & 8) == 0 || kind == r_u8(name_offset + 2))
                    return (sint32)(flags >> 6);
            }
        }
        node = r_u32(node);
    }
    return -1;
}

uint32 p2_func_80025674(uint32 slot, uint32 source_offset, uint32 scene_offset, sint32 index, uint32 flags)
{
    uint32 entry, first, second, value, replacement, record, coefficient, result;
    sint32 kind;
    entry = 0x8008B1D0 + slot * 36;
    w_u32(entry + 8, scene_offset);
    if (source_offset != 0)
        w_u32(entry + 12, source_offset);
    else
    {
        first = r_u32(entry);
        w_u16(first + 18, r_u16(first + 18) & 0xFFF7);
        first = r_u32(entry);
        if ((r_u16(first + 18) & 0x10) != 0)
        {
            second = r_u32(entry + 4);
            if (second != 0)
            {
                kind = r_s8(second + 14);
                if (kind < r_s8(first + 14))
                {
                    w_u16(second + 18, r_u16(second + 18) & 0xFFF7);
                    second = r_u32(entry + 4);
                    if ((r_u16(second + 18) & 0x10) == 0)
                    {
                        first = r_u32(entry);
                        w_u32(entry + 12, second);
                        scene_offset = r_u32(entry + 12);
                        replacement = r_u32(entry + 12);
                        w_u32(entry + 8, first);
                        w_u32(entry + 4, first);
                        w_u32(entry, replacement);
                    }
                }
            }
        }
        else
        {
            second = r_u32(entry + 4);
            if (second != 0)
                w_u16(second + 18, r_u16(second + 18) & 0xFFF7);
        }
    }
    w_u16(entry + 16, index);
    if ((sint16)index == -1)
        w_u16(entry + 18, 10);
    else if ((sint16)index == -2)
    {
        kind = r_s8(scene_offset + 14);
        record = r_u32(0x8008A4E4) + (uint32)kind * 24;
        value = r_u32(record + 4);
        w_u16(entry + 16, value);
        coefficient = r_u16(record + 2);
        kind = r_s16(scene_offset + 16);
        value = (uint32)(((sint64)coefficient * kind) >> 12);
        w_u16(entry + 18, value);
    }
    else
        w_u16(entry + 18, flags);
    result = r_u32(0x1F800320);
    result |= (uint32)1 << ((slot + 6) & 31);
    w_u32(0x1F800320, result);
    return result;
}

sint32 p2_func_80083310(uint32 first_offset, uint32 second_offset, sint32 count)
{
    uint32 first, second;
    for (;;)
    {
        first = r_u8(first_offset);
        second = r_u8(second_offset);
        ++first_offset;
        if (first != second)
        {
            first = r_u8(first_offset - 1);
            second = r_u8(second_offset);
            return (sint32)(first - second);
        }
        count = (sint32)((uint32)count - 1);
        ++second_offset;
        if (count <= 0)
            return 0;
    }
}
