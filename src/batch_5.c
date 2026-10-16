#include "game.h"

uint32 p2_func_80069f18(uint32 destination_offset, uint32 asset)
{
    w_u32(0x8008A380, destination_offset);
    w_u16(0x8008A37C, asset);
    return p2_func_8006997c();
}

uint32 p2_func_8006997c(void)
{
    uint32 result;
    uint32 destination, asset;
    w_u32(0x8008A38C, 0);
    w_u32(0x8008A388, 0);
    if ((sint32)p2_func_8006a3ac(r_u16(0x8008A37C)) > 0)
    {
        uint32 object = r_u32(0x8008A380);
        asset = r_u16(0x8008A37C);
        destination = r_u32(object + 124);
        w_u8(0x8008A49C, 1);
        result = p2_func_8006a1ac(asset, destination, 0);
        w_u32(0x8008A388, result);
    }
    else if ((sint32)p2_func_8006a3ac(r_u16(0x8008A37C) + 2) > 0 && r_u32(0x8008A48C) != 0)
    {
        uint32 object = r_u32(0x8008A380);
        asset = r_u16(0x8008A37C);
        destination = r_u32(object + 128);
        w_u8(0x8008A49C, 3);
        result = p2_func_8006a1ac(asset + 2, destination, 0);
        w_u32(0x8008A38C, result);
    }
    else
    {
        uint32 bytes, rounded, count;
        asset = r_u16(0x8008A37C);
        w_u8(0x8008A49C, 5);
        bytes = p2_func_8006a3ac(asset + 7);
        rounded = bytes + 2047;
        if ((sint32)rounded < 0)
            rounded = bytes + 4094;
        count = (uint32)((sint32)rounded >> 11);
        if ((sint32)count < 87)
            destination = r_u32(0x8008A380) + 0x2B800;
        else
        {
            bytes = p2_func_8006a3ac(r_u16(0x8008A37C) + 7);
            rounded = bytes + 2047;
            if ((sint32)rounded < 0)
                rounded = bytes + 4094;
            destination = r_u32(0x8008A380) + ((uint32)((sint32)rounded >> 11) << 11);
        }
        result = p2_func_8006a1ac(r_u16(0x8008A37C) + 5, destination, 0);
        w_u32(0x8008A390, result);
    }
    return result;
}

uint32 p2_func_800602f4(uint32 object)
{
    uint32 kind = r_u16(object + 4);
    uint32 payload = object + 40;
    uint32 size;
    w_u32(object + 8, payload);
    if (kind == 0)
    {
        uint32 width = r_u16(object + 24);
        uint32 height = r_u16(object + 26);
        uint32 pixels = width * height;
        payload = object + 44;
        w_u32(object + 28, payload);
        if ((pixels & 1) != 0)
            ++pixels;
        size = pixels << 1;
    }
    else
    {
        size = r_u16(object + 2);
        w_u32(object + 28, 0x800AC680);
    }
    w_u32(object + 36, payload + size);
    w_u16(object + 2, r_u16(object + 4));
    return object;
}

void p2_func_80068a24(uint32 object, uint32 header)
{
    const uint32 root_fields[] = {8, 28, 32, 36, 40, 44, 56, 48, 52, 60, 64, 68, 72, 76, 80, 84, 88, 92};
    const uint32 mesh_fields[] = {0, 4, 8, 132, 140, 40};
    const uint32 basic_fields[] = {0, 4, 8};
    const uint32 object_fields[] = {0, 4, 8, 16, 20, 36};
    const uint32 effect_fields[] = {0, 4, 8, 44, 40, 32};
    const uint32 final_heads[] = {80, 88, 84};
    uint32 node, value, field, i, group;
    for (i = 0; i < 18; ++i)
    {
        field = object + root_fields[i];
        value = r_u32(field);
        if (value != 0)
            w_u32(field, r_u32(object + 20) + value);
    }
    node = r_u32(object + 28);
    w_u32(object + 108, 0);
    for (group = 0; group < 2; ++group)
    {
        if (group != 0)
            node = r_u32(object + 32);
        while (node != 0)
        {
            for (i = 0; i < 6; ++i)
            {
                field = node + mesh_fields[i];
                value = r_u32(field);
                if (value != 0)
                    w_u32(field, r_u32(object + 20) + value);
            }
            value = r_u32(node + 36);
            if (value != 0)
            {
                uint32 polygon = r_u32(object + 20) + value;
                uint32 flags = r_u32(node + 28) & 12;
                w_u32(node + 36, polygon);
                if (flags != 8)
                {
                    uint32 count = (uint32)r_s16(node + 32);
                    do
                    {
                        uint32 index = r_u32(polygon);
                        --count;
                        w_u32(polygon, r_u32(header + 28) + index * 20);
                        polygon += 24;
                    } while (count != 0);
                }
            }
            value = r_u32(node + 104);
            if (value != 0)
            {
                uint32 animation = r_u32(object + 20) + value;
                w_u32(node + 104, animation);
                p2_func_80068294(animation, r_u32(object + 20));
            }
            value = r_u32(node + 44);
            if (value != 0)
                w_u32(node + 44, r_u32(object + 20) + value);
            if (r_s8(node + 125) == 31)
                w_u32(node + 108, r_u32(object + 20) + r_u32(node + 108));
            node = r_u32(node);
        }
    }
    node = r_u32(object + 36);
    while (node != 0)
    {
        const uint32 fields[] = {0, 4, 8, 40};
        for (i = 0; i < 4; ++i)
        {
            field = node + fields[i];
            value = r_u32(field);
            if (value != 0)
                w_u32(field, r_u32(object + 20) + value);
        }
        value = r_u32(node + 36);
        if (value != 0)
        {
            uint32 polygon = r_u32(object + 20) + value;
            sint32 active = r_s16(node + 34);
            w_u32(node + 36, polygon);
            if (active > 0 && (r_u32(node + 28) & 0x400) == 0)
            {
                uint32 count = (uint32)r_s16(node + 32);
                do
                {
                    uint32 index = r_u32(polygon);
                    --count;
                    w_u32(polygon, r_u32(header + 28) + index * 20);
                    polygon += 24;
                } while (count != 0);
            }
        }
        value = r_u32(node + 44);
        if (value != 0)
            w_u32(node + 44, r_u32(object + 20) + value);
        node = r_u32(node);
    }
    node = r_u32(object + 40);
    while (node != 0)
    {
        for (i = 0; i < 6; ++i)
        {
            field = node + object_fields[i];
            value = r_u32(field);
            if (value != 0)
                w_u32(field, r_u32(object + 20) + value);
        }
        value = r_u32(node + 28);
        if (value != 0 && (r_u32(r_u32(node + 36) + 16) & 0x100010) != 0)
        {
            uint32 flags = r_u16(node + 14);
            uint32 base = r_u32(object + 20);
            uint32 relocated = base + value;
            if ((flags & ((flags & 0x10) | 8)) == 0)
            {
                // Preserve the original unbased reference access
                if ((r_u16(value + 14) & 8) != 0)
                    relocated = base + r_u32(value + 16);
                else
                    relocated = base + value;
            }
            w_u32(node + 28, relocated);
        }
        value = r_u32(node + 32);
        if (value != 0)
            w_u32(node + 32, r_u32(object + 20) + value);
        value = r_u32(node + 52);
        if (value != 0)
            w_u32(node + 52, r_u32(object + 20) + value);
        node = r_u32(node);
    }
    node = r_u32(object + 44);
    while (node != 0)
    {
        sint32 kind;
        for (i = 0; i < 3; ++i)
        {
            field = node + basic_fields[i];
            value = r_u32(field);
            if (value != 0)
                w_u32(field, r_u32(object + 20) + value);
        }
        kind = r_s8(node + 15);
        if (kind == 8)
            p2_func_80026b6c(node);
        else
        {
            value = r_u32(node + 68);
            if (value != 0 && (kind == 7 || ((uint32)(kind - 26) & 0xFF) < 2))
                w_u32(node + 68, r_u32(object + 20) + value);
        }
        node = r_u32(node);
    }
    for (group = 0; group < 2; ++group)
    {
        const uint32 fields[] = {0, 4, 8, 68, 72};
        uint32 count = group == 0 ? 3 : 5;
        node = r_u32(object + 48 + group * 4);
        while (node != 0)
        {
            for (i = 0; i < count; ++i)
            {
                field = node + fields[i];
                value = r_u32(field);
                if (value != 0)
                    w_u32(field, r_u32(object + 20) + value);
            }
            node = r_u32(node);
        }
    }
    node = r_u32(object + 60);
    while (node != 0)
    {
        uint32 type = r_u16(node + 18) & 0x7F;
        uint32 next = r_u32(node);
        w_u32(node + 20, r_u32(0x800B1708 + (type + 13) * 8));
        if (next != 0)
            w_u32(node, r_u32(object + 20) + next);
        for (i = 1; i < 3; ++i)
        {
            field = node + basic_fields[i];
            value = r_u32(field);
            if (value != 0)
                w_u32(field, r_u32(object + 20) + value);
        }
        node = r_u32(node);
    }
    node = r_u32(object + 64);
    while (node != 0)
    {
        for (i = 0; i < 6; ++i)
        {
            field = node + effect_fields[i];
            value = r_u32(field);
            if (value != 0)
                w_u32(field, r_u32(object + 20) + value);
        }
        node = r_u32(node);
    }
    node = r_u32(object + 68);
    while (node != 0)
    {
        uint32 base, payload;
        value = r_u32(node);
        if (value != 0)
            w_u32(node, r_u32(object + 20) + value);
        base = r_u32(object + 20);
        value = r_u32(node + 60);
        payload = r_u32(node + 56);
        w_u32(node + 60, base + value);
        if (payload != 0)
        {
            payload += r_u32(object + 20);
            w_u32(node + 56, payload);
            value = r_u32(payload + 12);
            if (value != 0)
                w_u32(payload + 12, r_u32(object + 20) + value);
        }
        node = r_u32(node);
    }
    node = r_u32(object + 72);
    while (node != 0)
    {
        value = r_u32(node);
        if (value != 0)
            w_u32(node, r_u32(object + 20) + value);
        node = r_u32(node);
    }
    node = r_u32(object + 76);
    while (node != 0)
    {
        uint32 index = r_u32(node + 16);
        uint32 materials = r_u32(header + 28);
        uint32 payload = r_u32(node + 20);
        w_u32(node + 16, materials + index * 20);
        if (payload != 0)
            w_u32(node + 20, r_u32(object + 20) + payload);
        for (i = 0; i < 3; ++i)
        {
            field = node + basic_fields[i];
            value = r_u32(field);
            if (value != 0)
                w_u32(field, r_u32(object + 20) + value);
        }
        if (r_s8(node + 27) != -1)
        {
            uint32 base = r_u32(object + 20);
            payload = r_u32(node + 24);
            payload += base;
            w_u32(node + 24, payload);
            value = r_u32(payload + 12);
            if (value != 0)
                w_u32(payload + 12, r_u32(object + 20) + value);
        }
        node = r_u32(node);
    }
    for (group = 0; group < 3; ++group)
    {
        node = r_u32(object + final_heads[group]);
        while (node != 0)
        {
            for (i = 0; i < 3; ++i)
            {
                field = node + basic_fields[i];
                value = r_u32(field);
                if (value != 0)
                    w_u32(field, r_u32(object + 20) + value);
            }
            node = r_u32(node);
        }
    }
}

uint32 p2_func_80025828(uint32 object)
{
    uint32 scene, value, flags, result;
    w_u8(object + 14, r_u8(0x8008A30C));
    w_u8(object + 15, r_u8(0x8008A514));
    scene = (uint32)r_s8(0x8008A30C);
    scene = r_u32(0x8008A4E4) + scene * 24;
    value = r_u32(scene + 8);
    flags = r_u16(object + 18);
    w_u32(object + 144, value);
    w_u32(object + 148, r_u32(scene + 12));
    value = r_u32(scene + 16);
    flags &= 0xFFE7;
    w_u16(object + 18, flags);
    w_u32(object + 152, value);
    result = (r_u8(scene) & 0x10) | 0x40;
    w_u16(object + 18, flags | result);
    return result;
}

uint32 p2_func_80025bfc(uint32 mode)
{
    uint32 slot = 0x8008B1D0 + mode * 36;
    uint32 pending = r_u32(slot + 4);
    uint32 current = r_u32(slot);
    uint32 spare = r_u32(0x8008A4E0);
    uint32 index = (uint32)r_s8(pending + 14);
    uint32 next = r_u32(slot + 4);
    uint32 parameters, value, flags, scale;
    sint32 factor;
    w_u32(slot + 4, current);
    w_u32(slot, next);
    parameters = r_u32(0x8008A4E4);
    current = r_u32(slot);
    parameters += index * 24;
    if (current == spare)
        w_u32(slot + 4, r_u32(0x8008A284) + 0xE8C00);
    else
        w_u32(slot + 4, spare);
    pending = r_u32(slot + 4);
    w_u32(pending + 20, pending + 188);
    w_u8(r_u32(slot + 4) + 14, 0xFF);
    value = r_u32(parameters + 4);
    current = r_u32(slot);
    flags = r_u32(0x1F800320);
    w_u16(slot + 16, value);
    scale = r_u16(parameters + 2);
    factor = r_s16(current + 16);
    w_u32(0x1F800320, flags | (1u << ((mode + 6) & 31)));
    scale = (uint32)(((sint64)(sint32)scale * factor) >> 12);
    next = r_u32(slot);
    current = r_u32(slot);
    w_u16(slot + 18, scale);
    w_u32(slot + 8, 0);
    w_u32(slot + 12, next);
    flags = r_u16(current + 18);
    w_u16(current + 18, flags & ~((1u << (mode & 31)) | 8));
    w_u16(0x800B99CE, 15);
    return 15;
}

uint32 p2_func_8005cd24(void)
{
    uint32 flags = r_u32(0x1F800320);
    uint32 pending, scene, node;
    w_u32(0x8008A114, 0x800B99A0);
    w_u8(0x800B99A1, 0);
    if ((flags & 0x40) != 0)
    {
        w_u32(0x1F800320, flags & 0xFFFFFFBF);
        p2_func_800264c8(0);
    }
    if (r_u16(0x8008A532) < 0x1000)
    {
        pending = r_u32(0x8008A240);
        if (pending != 0xFFFFFFFF)
        {
            uint32 queue = r_u32(0x8008A114);
            sint32 count = r_s8(queue + 1);
            if (count < 40)
            {
                w_u8(queue + 1, (uint32)(count + 1));
                w_u32(queue + (uint32)count * 4 + 140, pending | 0x80000);
            }
            w_u32(0x8008A240, 0xFFFFFFFF);
        }
        scene = r_u32(0x8008A4E8);
        node = r_u32(scene + 44);
        p2_func_8005c588(node, r_u32(scene + 56));
        scene = r_u32(0x8008A4E8);
        p2_func_8005c588(r_u32(scene + 48), 0);
        p2_func_80047c1c();
        node = r_u32(0x8008A4EC);
        while (node != 0)
        {
            if (((r_u8(node + 13) & 0x81) == 0 || r_s8(node + 126) == 1) && r_s8(node + 126) != 0 && (r_s8(node + 126) != 3 || r_u32(node + 116) != 30))
                p2_func_8003a930(node);
            node = r_u32(node);
        }
        node = r_u32(r_u32(0x8008A4E8) + 32);
        while (node != 0)
        {
            if ((r_u8(node + 13) & 0x81) == 0 && r_s8(node + 126) != 0)
                p2_func_8003a930(node);
            node = r_u32(node);
        }
        p2_func_8005bff4();
    }
    p2_func_80016ef4(r_u32(0x8008A4E8));
    return p2_func_8005ee20();
}

uint32 p2_func_800264c8(uint32 mode)
{
    uint32 slot, scene, node, flags, process_new, x, y;
    p2_func_800260b0();
    p2_func_8005ba34();
    slot = 0x8008B1D0 + mode * 36;
    p2_func_80026370(r_u32(slot + 12), r_u32(slot + 8), mode);
    scene = r_u32(slot + 12);
    w_u32(slot + 8, 0);
    w_u32(0x8008A4E8, scene);
    x = r_u32(scene + 116);
    y = r_u32(scene + 120);
    w_u32(0x1F80003C, x);
    w_u32(0x1F800040, y);
    if (p2_func_80026c14() != 0)
        p2_func_80026bf4();
    flags = r_u16(r_u32(slot + 12) + 18);
    process_new = ((flags >> 3) ^ 1) & 1;
    if (process_new != 0)
    {
        if ((flags & 0x10) == 0)
            p2_func_80066358(0x800B9938, 0, 50);
        scene = r_u32(slot + 12);
        node = r_u32(scene + 84);
        while (node != 0)
        {
            flags = r_u8(node + 14);
            w_u8(node + 14, flags & 0xFB);
            if ((flags & 8) != 0)
                w_u8(node + 13, r_u8(node + 13) | 1);
            else
                w_u8(node + 13, r_u8(node + 13) & 0xFE);
            node = r_u32(node);
            if (node == 0)
                scene = r_u32(slot + 12);
        }
        node = r_u32(scene + 44);
        while (node != 0)
        {
            uint32 kind;
            flags = r_u8(node + 14);
            kind = r_u8(node + 15);
            w_u8(node + 14, flags & 0xB7);
            if (kind - 40 < 16)
            {
                uint32 first = r_u32(node + 68);
                uint32 second = r_u32(node + 72);
                w_u32(node + 68, (first & 0xFFF) | ((kind - 39) << 12) | (second & 0x1FE0000));
            }
            else if ((sint8)kind == 2)
                process_new = 0;
            else if ((sint8)kind == 5)
            {
                w_u32(node + 72, 0);
                w_u32(node + 68, 0);
            }
            if (r_s8(node + 15) != 50 || (r_u16(node + 78) & 0x3FFF) != 0)
                w_u8(node + 13, r_u8(node + 13) & 0x7F);
            node = r_u32(node);
        }
        node = r_u32(r_u32(slot + 12) + 48);
        while (node != 0)
        {
            uint32 kind;
            flags = r_u8(node + 13);
            kind = r_u8(node + 15);
            w_u8(node + 13, flags & 0x7F);
            if (kind - 40 < 16)
            {
                uint32 first = r_u32(node + 68);
                uint32 second = r_u32(node + 72);
                sint32 current_kind = r_s8(node + 15);
                w_u32(node + 68, (first & 0xFFF) | ((kind - 39) << 12) | (second & 0x1FE0000));
                if (current_kind == 50)
                    w_u32(node + 76, r_u32(node + 76) | 0xFF);
            }
            node = r_u32(node);
        }
        node = r_u32(r_u32(0x8008A4E8) + 68);
        while (node != 0)
        {
            uint32 object;
            p2_func_80034658(r_u32(node + 60));
            object = r_u32(node + 60);
            w_u32(object + 16, r_u32(object + 16) & 0xFFFFFFFA);
            object = r_u32(node + 60);
            if ((r_u32(object + 16) & 0x20) != 0)
            {
                w_u8(object + 13, r_u8(object + 13) | 1);
                object = r_u32(node + 60);
                w_u32(object + 16, r_u32(object + 16) | 1);
            }
            else
                w_u8(object + 13, r_u8(object + 13) & 0xFE);
            node = r_u32(node);
        }
        p2_func_80016c54(r_u32(slot + 12), 4);
        scene = r_u32(slot + 12);
        w_u16(scene + 18, r_u16(scene + 18) | 8);
        w_u16(r_u32(slot + 12) + 96, 0);
        scene = r_u32(slot + 12);
        node = r_u32(0x8008A114);
        x = r_u32(scene + 144);
        w_u32(node + 344, x);
        w_u32(node + 340, x);
    }
    scene = r_u32(slot + 12);
    x = (uint32)r_s16(slot + 16);
    y = (uint32)r_s16(slot + 18);
    p2_func_80053e7c((sint16)mode, scene, (sint32)x, (sint16)y);
    scene = r_u32(slot + 12);
    flags = r_u16(scene + 18);
    x = r_u16(0x8008A530);
    w_u16(scene + 18, flags | (1u << (mode & 31)));
    if (x == 0)
    {
        scene = r_u32(slot + 12);
        if (r_u32(scene + 128) != 0)
        {
            uint32 level = r_u32(0x8008A4E4);
            uint32 index = (uint32)r_s8(scene + 14);
            uint32 asset = r_u32(level + 240) + index * 9;
            if ((sint32)p2_func_8006a3c4(asset + 8) >= 0)
            {
                uint32 samples, header;
                scene = r_u32(slot + 12);
                level = r_u32(0x8008A4E4);
                header = r_u32(0x8008B1D4);
                index = (uint32)r_s8(scene + 14);
                samples = r_u32(scene + 128);
                asset = r_u32(level + 240);
                header = r_u32(header + 20);
                asset += index * 9;
                x = p2_func_8006ac44(asset + 2, asset + 8, samples, header);
                w_u16(0x8008A530, x);
                if ((x & 0xFFFF) != 0)
                    p2_func_80066404();
                else
                    w_u8(0x8008A49C, 4);
            }
        }
    }
    if (r_u16(r_u32(0x8008A4E4) + 262) != 0)
    {
        if (process_new == 0)
            return 0;
        flags = r_u16(r_u32(slot + 12) + 18) & 0x10;
        if (flags != 0)
            return flags;
        return p2_func_80025d3c(0xFFFFFFFF, mode);
    }
    if (r_s8(0x8008A43C) > 0 && r_u32(0x8008A130) == 0)
    {
        uint32 index = (uint32)r_s8(0x8008A43C);
        uint32 next = r_u32(0x80083CF4 + index * 4);
        next = (sint32)next < 3 ? next + 1 : next - 1;
        p2_func_80025d3c(next, mode);
    }
    {
        sint32 number = r_s8(0x8008A43C);
        sint32 high = (sint32)(((sint64)number * 0x66666667) >> 32);
        sint32 quotient = (high >> 2) - (number >> 31);
        uint32 level = r_u32(0x8008A4E4);
        uint32 event;
        w_u16(level + 266, r_u16(slot + 16));
        w_u16(level + 268, r_u16(slot + 18));
        event = p2_func_8005bb90((sint8)quotient + 48, (sint8)(number - quotient * 10) + 48);
        return p2_func_8005bb48(event | 0x4000, 4);
    }
}

void p2_func_800260b0(void)
{
    uint32 node = r_u32(0x8008A4EC);
    while (node != 0)
    {
        uint32 next = r_u32(node);
        if (r_s8(node + 125) == 6)
            p2_func_8002c510(node);
        p2_func_800401c8(node);
        if (r_u8(node + 12) == 7)
            p2_func_8002a2f4(0, node);
        else
        {
            if (r_s8(node + 127) == 2 || r_s8(node + 124) >= 4)
                p2_func_80034848(r_u32(node + 132));
            if ((r_u32(node + 28) & 4) != 0)
                w_u8(node + 13, (r_u8(node + 13) | 0x80) & 0xFC);
            else
                p2_func_80064d54(node);
        }
        node = next;
    }
    if (r_u32(0x8008A4E8) != 0)
    {
        uint32 group;
        for (group = 0; group < 2; ++group)
        {
            uint32 scene = r_u32(0x8008A4E8);
            node = r_u32(scene + (group == 0 ? 8 : 108));
            while (node != 0)
            {
                uint32 next = r_u32(node + 4);
                if (group != 0 || (r_u32(node + 12) & 0x200FF) == 0x20007)
                {
                    scene = r_u32(0x8008A4E8);
                    p2_func_8002a210(scene + (group == 0 ? 0 : 100), node);
                    if (r_u8(node + 12) == 7)
                        p2_func_8002a2f4(0, node);
                    else
                    {
                        if (r_s8(node + 127) == 2 || r_s8(node + 124) >= 4)
                            p2_func_80034848(r_u32(node + 132));
                        if ((r_u32(node + 28) & 4) != 0)
                            w_u8(node + 13, (r_u8(node + 13) | 0x80) & 0xFC);
                        else
                            p2_func_80064d54(node);
                    }
                }
                node = next;
            }
        }
        p2_func_80055280(0, r_u32(0x8008A4E8));
        {
            uint32 scene = r_u32(0x8008A4E8);
            w_u16(scene + 18, r_u16(scene + 18) & 0xFFFE);
        }
        p2_func_80025f58();
    }
}

void p2_func_80026370(uint32 scene_offset, uint32 previous_offset, uint32 mode)
{
    uint32 changed = 0;
    if (scene_offset == 0)
        return;
    if (previous_offset == 0 || p2_func_80083310(scene_offset + 132, previous_offset + 132, 4) != 0)
        changed = 1;
    if (scene_offset == previous_offset)
        return;
    if (r_u32(scene_offset + 128) != 0 && changed != 0)
    {
        uint32 first, second;
        if (r_u16(0x8008A530) != 0)
        {
            p2_func_80066430();
            p2_func_8006ae90(r_u16(0x8008A530));
            w_u16(0x8008A530, 0);
        }
        first = r_u32(0x8008B1E4 + mode * 36);
        second = r_u32(0x8008B1E8 + mode * 36);
        w_u32(0x8008B1E8 + mode * 36, first);
        w_u32(0x8008B1E4 + mode * 36, second);
    }
    if (r_u32(scene_offset + 124) != 0)
    {
        uint32 first = r_u32(0x8008B1EC + mode * 36);
        uint32 second = r_u32(0x8008B1F0 + mode * 36);
        w_u32(0x8008B1F0 + mode * 36, first);
        w_u32(0x8008B1EC + mode * 36, second);
    }
    else if (previous_offset != 0)
        w_u32(scene_offset + 124, 0);
}

uint32 p2_func_80016c54(uint32 scene_offset, uint32 mode)
{
    uint32 i;
    uint32 matrix;
    uint32 first, second, third;
    for (i = 0; i < 5; ++i)
        xport_gte_write_control(i, r_u32(0x80010180 + i * 4));
    matrix = r_u32(0x8008A0C8);
    first = r_u32(0x80010180);
    second = r_u32(0x80010184);
    third = r_u32(0x80010188);
    w_u32(matrix, first);
    w_u32(matrix + 4, second);
    w_u32(matrix + 8, third);
    first = r_u32(0x8001018C);
    second = (uint32)r_s16(0x80010190);
    w_u32(matrix + 12, first);
    w_u16(matrix + 16, second);
    matrix = r_u32(0x8008A0C8);
    w_u32(matrix + 28, 0);
    w_u32(matrix + 24, 0);
    w_u32(matrix + 20, 0);
    return p2_func_800140f8(scene_offset, 0, mode);
}

uint32 p2_func_800140f8(uint32 scene_offset, uint32 parent_offset, uint32 mode)
{
    const uint32 transpose_offsets[9] = {0, 6, 12, 2, 8, 14, 4, 10, 16};
    uint32 saved_stack = p2_guest_stack_pointer;
    uint32 delta, input, temp_matrix, before, relative, transposed;
    uint32 node, next_node, base_mode, work, category, old_active, flags;
    uint32 matrix, source, target, attachment, player, follow;
    uint32 x, y, z, w, i;
    sint32 index, angle, cosine, sine;
    p2_guest_stack_pointer -= 0xC0;
    delta = p2_guest_stack_pointer + 0x10;
    input = p2_guest_stack_pointer + 0x20;
    temp_matrix = p2_guest_stack_pointer + 0x30;
    before = p2_guest_stack_pointer + 0x48;
    relative = p2_guest_stack_pointer + 0x58;
    transposed = p2_guest_stack_pointer + 0x68;
    matrix = r_u32(0x8008A0C8);
    node = r_u32(scene_offset + 8);
    w_u32(0x8008A0C8, matrix + 32);
    base_mode = mode & 0xFFFFFFCD;
    while (node != 0)
    {
        flags = r_u8(node + 13);
        work = base_mode;
        next_node = r_u32(node + 4);
        old_active = flags & 1;
        if ((work & 1) != 0)
            w_u8(node + 13, flags | 1);
        else if ((work & 8) != 0)
            w_u8(node + 13, flags & 0xFE);
        category = r_u8(node + 12);
        if (category == 0 && (work & 0x85) == 0)
        {
            p2_dispatch_object_callback(r_u32(0x800BA380 + (uint32)r_s8(node + 125) * 4), node);
            flags = r_u32(node + 28);
            if ((flags & 0x8C000) != 0)
            {
                work &= 0xFFFFFFF7;
                if ((flags & 0x8000) != 0)
                    work |= 0x80;
                else
                {
                    if ((r_u8(node + 13) & 1) != old_active)
                        work |= old_active != 0 ? 0x88 : 1;
                    else if ((flags & 0x80002) == 0x80002 && old_active == 0)
                        work |= 8;
                    else if (r_s8(node + 125) == 22)
                        old_active = 0;
                    w_u32(node + 28, r_u32(node + 28) & 0xFFF7BFFF);
                }
            }
            if (old_active != 0 && work == 0)
                goto next_object;
        }
        if ((work & 0x84) != 0 && (p2_func_80013cec(node, parent_offset) & 0xFF) != 0)
        {
            if ((work & 8) != 0 && (r_u8(node + 13) & 1) != 0 && (base_mode & 8) == 0)
                w_u8(node + 13, r_u8(node + 13) & 0xFE);
            flags = r_u8(node + 13);
            work &= 0xFFFFFFF6;
            work |= (flags & 1) != 0 ? 0xC1 : 0xC8;
        }
        if ((r_u8(node + 13) & 0x91) == 0x10 && (parent_offset == 0 || category == 0 || (r_u32(parent_offset + 28) & 0x200) == 0))
            work |= 0x10;
        flags = r_u8(node + 13) & 0xE2;
        if (flags >= 0x80)
            goto next_object;
        work |= flags;
        if (work == 0)
            goto next_object;
        if ((work & 0x62) != 0)
            p2_func_8001aecc(parent_offset, node);
        matrix = r_u32(0x8008A0C8);
        source = matrix - 32;
        for (i = 0; i < 5; ++i)
            xport_gte_write_control(i, r_u32(source + i * 4));

        if (category == 5)
        {
            p2_sdk_8007ec64(node + 16, matrix + 20);
            matrix = r_u32(0x8008A0C8);
            x = r_u32(matrix + 20) + r_u32(matrix - 12);
            y = r_u32(matrix + 24) + r_u32(matrix - 8);
            z = r_u32(matrix + 28) + r_u32(matrix - 4);
            w_u32(matrix + 20, x);
            w_u32(matrix + 24, y);
            w_u32(matrix + 28, z);

            if (r_s16(node + 40) == 0x4000)
            {
                source = matrix - 32;
                {
                    uint32 c0 = r_u32(source), c1 = r_u32(source + 4), c2 = r_u32(source + 8), c3 = r_u32(source + 12);
                    w_u32(matrix, c0);
                    w_u32(matrix + 4, c1);
                    w_u32(matrix + 8, c2);
                    w_u32(matrix + 12, c3);
                    w_u16(matrix + 16, r_u16(source + 16));
                }
            }
            else
            {
                {
                    uint32 qw = (uint32)r_s16(node + 40), qx = (uint32)r_s16(node + 40 + 2), qy = (uint32)r_s16(node + 40 + 4), qz = (uint32)r_s16(node + 40 + 6);
                    uint32 xx = qx * qx, yy = qy * qy, zz = qz * qz, xy = qx * qy, xz = qx * qz, yz = qy * qz, wz = qw * qz, wy = qw * qy, wx = qw * qx;
                    w_u16(matrix, (sint32)(0x08000000u - (yy + zz)) >> 15);
                    w_u16(matrix + 8, (sint32)(0x08000000u - (xx + zz)) >> 15);
                    w_u16(matrix + 16, (sint32)(0x08000000u - (xx + yy)) >> 15);
                    w_u16(matrix + 6, (sint32)(xy - wz) >> 15);
                    w_u16(matrix + 2, (sint32)(xy + wz) >> 15);
                    w_u16(matrix + 12, (sint32)(xz + wy) >> 15);
                    w_u16(matrix + 4, (sint32)(xz - wy) >> 15);
                    w_u16(matrix + 14, (sint32)(yz - wx) >> 15);
                    w_u16(matrix + 10, (sint32)(yz + wx) >> 15);
                }
                source = p2_sdk_8007e944(matrix);
                for (i = 0; i < 5; ++i)
                    xport_gte_write_control(i, r_u32(source + i * 4));
            }
            index = r_s8(node + 30);
            if (index != -1)
            {
                if (index < 0)
                {
                    index = -(index + 2);
                    work &= 0xFFFFFFEF;
                }
                if ((work & 0xCC) != 0)
                {
                    target = r_u32(parent_offset + 140) + (uint32)index * 48 + 4;

                    if ((work & 0x8C) != 0)
                    {
                        w_u32(target + 40, 0);
                        w_u32(target + 36, 0);
                        w_u32(target + 32, 0);
                    }
                    else
                    {
                        matrix = r_u32(0x8008A0C8);
                        w_u32(target + 32, r_u32(matrix + 20) - r_u32(target + 20));
                        w_u32(target + 36, r_u32(matrix + 24) - r_u32(target + 24));
                        w_u32(target + 40, r_u32(matrix + 28) - r_u32(target + 28));
                    }
                    matrix = r_u32(0x8008A0C8);
                    {
                        uint32 c0 = r_u32(matrix + 20), c1 = r_u32(matrix + 20 + 4), c2 = r_u32(matrix + 20 + 8);
                        w_u32(target + 20, c0);
                        w_u32(target + 20 + 4, c1);
                        w_u32(target + 20 + 8, c2);
                    }
                    matrix = r_u32(0x8008A0C8);
                    for (i = 0; i < 9; ++i)
                        w_u16(target + i * 2, r_u16(matrix + transpose_offsets[i]));

                    if (parent_offset != 0)
                    {
                        if ((r_u32(parent_offset + 28) & 0x200) != 0)
                            w_u8(node + 13, r_u8(node + 13) | 4);
                        else if ((r_u8(node + 13) & 4) != 0)
                        {
                            if ((work & 2) == 0)
                            {
                                w_u32(target + 40, 0);
                                w_u32(target + 36, 0);
                                w_u32(target + 32, 0);
                            }
                            w_u8(node + 13, r_u8(node + 13) & 0xFB);
                        }
                    }
                }
            }
            if ((work & 0x14) == 0x10)
                p2_func_80011980(parent_offset, node + 32);
            goto visit_children;
        }
        if (category == 0)
        {
            if (r_s8(node + 125) == 22)
            {
                if ((r_u32(node + 28) & 0x2000) != 0)
                {
                    {
                        uint32 c0 = r_u32(node + 16), c1 = r_u32(node + 16 + 4), c2 = r_u32(node + 16 + 8);
                        w_u32(matrix + 20, c0);
                        w_u32(matrix + 20 + 4, c1);
                        w_u32(matrix + 20 + 8, c2);
                    }
                }
                else
                {
                    attachment = r_u32(node + 132);
                    if (attachment != 0 && r_u32(attachment + 40) != 0)
                    {
                        {
                            uint32 c0 = r_u32(node + 16), c1 = r_u32(node + 16 + 4), c2 = r_u32(node + 16 + 8);
                            w_u32(input, c0);
                            w_u32(input + 4, c1);
                            w_u32(input + 8, c2);
                        }
                        PushMatrix();
                        source = r_u32(r_u32(node + 132) + 40);
                        for (i = 0; i < 5; ++i)
                            xport_gte_write_control(i, r_u32(source + i * 4));
                        p2_sdk_8007ec64(input, r_u32(0x8008A0C8) + 20);
                        PopMatrix();
                        matrix = r_u32(0x8008A0C8);
                        for (i = 0; i < 3; ++i)
                        {
                            source = r_u32(r_u32(node + 132) + 40);
                            w_u32(matrix + 20 + i * 4, r_u32(matrix + 20 + i * 4) + r_u32(source + 20 + i * 4));
                        }
                    }
                    else
                    {
                        matrix = r_u32(0x8008A0C8);
                        {
                            uint32 c0 = r_u32(node + 16), c1 = r_u32(node + 16 + 4), c2 = r_u32(node + 16 + 8);
                            w_u32(matrix + 20, c0);
                            w_u32(matrix + 20 + 4, c1);
                            w_u32(matrix + 20 + 8, c2);
                        }
                    }
                }
                matrix = r_u32(0x8008A0C8);
                {
                    uint32 qw = (uint32)r_s16(node + 84), qx = (uint32)r_s16(node + 84 + 2), qy = (uint32)r_s16(node + 84 + 4), qz = (uint32)r_s16(node + 84 + 6);
                    uint32 xx = qx * qx, yy = qy * qy, zz = qz * qz, xy = qx * qy, xz = qx * qz, yz = qy * qz, wz = qw * qz, wy = qw * qy, wx = qw * qx;
                    w_u16(matrix, (sint32)(0x08000000u - (yy + zz)) >> 15);
                    w_u16(matrix + 8, (sint32)(0x08000000u - (xx + zz)) >> 15);
                    w_u16(matrix + 16, (sint32)(0x08000000u - (xx + yy)) >> 15);
                    w_u16(matrix + 6, (sint32)(xy - wz) >> 15);
                    w_u16(matrix + 2, (sint32)(xy + wz) >> 15);
                    w_u16(matrix + 12, (sint32)(xz + wy) >> 15);
                    w_u16(matrix + 4, (sint32)(xz - wy) >> 15);
                    w_u16(matrix + 14, (sint32)(yz - wx) >> 15);
                    w_u16(matrix + 10, (sint32)(yz + wx) >> 15);
                }
                for (i = 0; i < 5; ++i)
                    xport_gte_write_control(i, r_u32(matrix + i * 4));

                for (i = 0; i < 3; ++i)
                    w_u32(delta + i * 4, r_u32(matrix + 20 + i * 4) - r_u32(node + 176 + i * 4));
                for (i = 0; i < 3; ++i)
                    w_u32(node + 160 + i * 4, r_u32(node + 160 + i * 4) + r_u32(delta + i * 4));
                for (i = 0; i < 3; ++i)
                    w_u32(node + 60 + i * 4, r_u32(node + 60 + i * 4) + r_u32(delta + i * 4));
                target = r_u32(node + 8);
                {
                    uint32 c0 = r_u32(node + 60), c1 = r_u32(node + 60 + 4), c2 = r_u32(node + 60 + 8);
                    w_u32(target + 60, c0);
                    w_u32(target + 60 + 4, c1);
                    w_u32(target + 60 + 8, c2);
                }
                matrix = r_u32(0x8008A0C8);
                for (i = 0; i < 3; ++i)
                    w_u32(target + 160 + i * 4, r_u32(target + 60 + i * 4) - r_u32(matrix + 20 + i * 4));
                goto cache_actor;
            }
            p2_sdk_8007ec64(node + 16, matrix + 20);
            matrix = r_u32(0x8008A0C8);
            x = r_u32(matrix + 20) + r_u32(matrix - 12);
            y = r_u32(matrix + 24) + r_u32(matrix - 8);
            z = r_u32(matrix + 28) + r_u32(matrix - 4);
            w_u32(matrix + 20, x);
            w_u32(matrix + 24, y);
            w_u32(matrix + 28, z);

            if (r_s16(node + 84) == 0x4000)
            {
                source = matrix - 32;
                {
                    uint32 c0 = r_u32(source), c1 = r_u32(source + 4), c2 = r_u32(source + 8), c3 = r_u32(source + 12);
                    w_u32(matrix, c0);
                    w_u32(matrix + 4, c1);
                    w_u32(matrix + 8, c2);
                    w_u32(matrix + 12, c3);
                    w_u16(matrix + 16, r_u16(source + 16));
                }
            }
            else
            {
                {
                    uint32 qw = (uint32)r_s16(node + 84), qx = (uint32)r_s16(node + 84 + 2), qy = (uint32)r_s16(node + 84 + 4), qz = (uint32)r_s16(node + 84 + 6);
                    uint32 xx = qx * qx, yy = qy * qy, zz = qz * qz, xy = qx * qy, xz = qx * qz, yz = qy * qz, wz = qw * qz, wy = qw * qy, wx = qw * qx;
                    w_u16(matrix, (sint32)(0x08000000u - (yy + zz)) >> 15);
                    w_u16(matrix + 8, (sint32)(0x08000000u - (xx + zz)) >> 15);
                    w_u16(matrix + 16, (sint32)(0x08000000u - (xx + yy)) >> 15);
                    w_u16(matrix + 6, (sint32)(xy - wz) >> 15);
                    w_u16(matrix + 2, (sint32)(xy + wz) >> 15);
                    w_u16(matrix + 12, (sint32)(xz + wy) >> 15);
                    w_u16(matrix + 4, (sint32)(xz - wy) >> 15);
                    w_u16(matrix + 14, (sint32)(yz - wx) >> 15);
                    w_u16(matrix + 10, (sint32)(yz + wx) >> 15);
                }
                source = p2_sdk_8007e944(matrix);
                for (i = 0; i < 5; ++i)
                    xport_gte_write_control(i, r_u32(source + i * 4));
            }
            if ((work & 0xCC) != 0)
            {
                matrix = r_u32(0x8008A0C8);
                for (i = 0; i < 3; ++i)
                    w_u32(node + 60 + i * 4, r_u32(node + 160 + i * 4) + r_u32(matrix + 20 + i * 4));
            cache_actor:
                if ((r_u32(node + 28) & 0x100000) != 0 && r_s16(node + 56) != 0)
                {
                    target = r_u32(node + 140);
                    if (r_s16(node + 54) == r_s16(target))
                    {
                        target += 4;

                        if ((work & 0x8C) != 0)
                        {
                            w_u32(target + 40, 0);
                            w_u32(target + 36, 0);
                            w_u32(target + 32, 0);
                        }
                        else
                        {
                            matrix = r_u32(0x8008A0C8);
                            w_u32(target + 32, r_u32(matrix + 20) - r_u32(target + 20));
                            w_u32(target + 36, r_u32(matrix + 24) - r_u32(target + 24));
                            w_u32(target + 40, r_u32(matrix + 28) - r_u32(target + 28));
                        }
                        matrix = r_u32(0x8008A0C8);
                        {
                            uint32 c0 = r_u32(matrix + 20), c1 = r_u32(matrix + 20 + 4), c2 = r_u32(matrix + 20 + 8);
                            w_u32(target + 20, c0);
                            w_u32(target + 20 + 4, c1);
                            w_u32(target + 20 + 8, c2);
                        }
                        matrix = r_u32(0x8008A0C8);
                        for (i = 0; i < 9; ++i)
                            w_u16(target + i * 2, r_u16(matrix + transpose_offsets[i]));
                    }
                }
                w_u8(node + 13, r_u8(node + 13) & 0xBF);
            }
            if ((work & 0x14) == 0x10)
            {
                if ((p2_func_80011338(node + 60, r_u32(node + 72)) & 0xFF) != 0)
                {
                    flags = r_u32(node + 28);
                    if ((flags & 0x200) != 0)
                    {
                        work |= 0x40;
                        w_u32(node + 28, flags & 0xFFFFFDFF);
                    }
                    if ((r_u32(node + 28) & 0x80) != 0)
                        work &= 0xFFFFFFEF;
                    else
                    {
                        p2_func_800275bc(r_u32(node + 44));
                        matrix = r_u32(0x8008A0C8);
                        p2_func_8002e824(matrix + 20, r_s32(node + 72) >> 5);
                        if (r_s16(node + 56) != 0)
                            p2_func_80011980(node, node + 50);
                    }
                }
                else
                {
                    work &= 0xFFFFFFEF;
                    goto mark_actor_hidden;
                }
            }
            else if (parent_offset != 0 && (r_u32(parent_offset + 28) & 0x200) != 0)
            {
            mark_actor_hidden:
                if ((work & 0x40) != 0 && (r_u32(node + 28) & 0x30200) == 0x200)
                {
                    work &= 0xFFFFFFBF;
                    w_u8(node + 13, r_u8(node + 13) | 0x40);
                }
                else
                {
                    work &= 0xFFFFFFFD;
                    w_u32(node + 28, r_u32(node + 28) | 0x200);
                }
            }
            else
            {
                flags = r_u32(node + 28);
                if ((flags & 0x200) != 0)
                {
                    work |= 0x40;
                    w_u32(node + 28, flags & 0xFFFFFDFF);
                }
            }
            if (work != 0)
            {
                p2_func_800140f8(node, node, work);
                if ((work & 0x14) == 0x10)
                    w_u32(node + 28, r_u32(node + 28) & 0xFFFFFFFE);
            }
            if (r_s8(node + 125) == 22)
            {
                target = r_u32(node + 8);
                flags = r_u32(target + 28);
                if ((flags & 0x20200) != 0x200)
                {
                    w_u32(target + 28, flags & 0xFFFDFFFF);
                    {
                        uint32 c0 = r_u32(target + 160), c1 = r_u32(target + 160 + 4), c2 = r_u32(target + 160 + 8);
                        w_u32(target + 60, c0);
                        w_u32(target + 60 + 4, c1);
                        w_u32(target + 60 + 8, c2);
                    }
                    x = r_u32(target + 172);
                    w_u32(target + 80, x);
                    w_u32(target + 76, x);
                    w_u32(target + 72, x);
                    x = r_u32(target + 160);
                    y = r_u32(target + 164);
                    z = r_u32(target + 168);
                    w = r_u32(target + 172);
                    w_u32(node + 160, x);
                    w_u32(node + 164, y);
                    w_u32(node + 168, z);
                    w_u32(node + 172, w);
                    {
                        uint32 c0 = r_u32(target + 60), c1 = r_u32(target + 60 + 4), c2 = r_u32(target + 60 + 8);
                        w_u32(node + 60, c0);
                        w_u32(node + 60 + 4, c1);
                        w_u32(node + 60 + 8, c2);
                    }
                    {
                        uint32 c0 = r_u32(target + 72), c1 = r_u32(target + 72 + 4), c2 = r_u32(target + 72 + 8);
                        w_u32(node + 72, c0);
                        w_u32(node + 72 + 4, c1);
                        w_u32(node + 72 + 8, c2);
                    }
                }
                matrix = r_u32(0x8008A0C8);
                {
                    uint32 c0 = r_u32(matrix + 20), c1 = r_u32(matrix + 20 + 4), c2 = r_u32(matrix + 20 + 8);
                    w_u32(node + 176, c0);
                    w_u32(node + 176 + 4, c1);
                    w_u32(node + 176 + 8, c2);
                }
                if (r_s8(node + 126) != 0)
                    w_u32(node + 144, 4096);
                index = r_s8(node + 124);
                if (index != 0)
                    p2_dispatch_object_callback(r_u32(0x800B7A60 + (uint32)index * 4), node);
            }
            goto next_object;
        }
        if (category == 7)
        {
            p2_sdk_8007ec64(node + 28, matrix + 20);
            matrix = r_u32(0x8008A0C8);
            x = r_u32(matrix + 20) + r_u32(matrix - 12);
            y = r_u32(matrix + 24) + r_u32(matrix - 8);
            z = r_u32(matrix + 28) + r_u32(matrix - 4);
            w_u32(matrix + 20, x);
            w_u32(matrix + 24, y);
            w_u32(matrix + 28, z);

            if ((work & 0x14) == 0x10)
            {
                if (parent_offset == 0)
                {
                    p2_func_8001baa8(node);
                    if ((r_u16(node + 14) & 0x10) != 0)
                        w_u32(0x1F8002B4, 0x1FFFFC00);
                    matrix = r_u32(0x8008A0C8);
                    if ((p2_func_80011338(matrix + 20, (uint32)(r_s16(node + 40) + r_s16(node + 42)) << 5) & 0xFF) != 0)
                    {
                        if ((p2_func_800275bc(r_u32(node + 20)) & 0xFF) == 0)
                        {
                            p2_func_8002a210(scene_offset, node);
                            p2_func_80025eac(node);
                        }
                        else
                        {
                            w_u32(0x1F800320, r_u32(0x1F800320) | 0x200);
                            p2_func_8001156c(node);
                            w_u32(0x1F800320, r_u32(0x1F800320) & 0xFFFFFDFF);
                        }
                    }
                    w_u32(0x1F8002B4, r_u32(r_u32(0x8008A114) + 340));
                    goto next_object;
                }
                source = matrix - 32;
                {
                    uint32 c0 = r_u32(source), c1 = r_u32(source + 4), c2 = r_u32(source + 8), c3 = r_u32(source + 12);
                    w_u32(matrix, c0);
                    w_u32(matrix + 4, c1);
                    w_u32(matrix + 8, c2);
                    w_u32(matrix + 12, c3);
                    w_u16(matrix + 16, r_u16(source + 16));
                }

                if ((p2_func_800275bc(r_u32(node + 20)) & 0xFF) != 0)
                    p2_func_8001156c(node);
                else
                {
                    p2_func_80013cec(node, parent_offset);
                    work = (work & 0xFFFFFF36) | 0x81;
                    w_u8(node + 13, r_u8(node + 13) | 1);
                }
            }
            else if ((r_u16(node + 14) & 2) != 0)
            {
                p2_func_8002a210(scene_offset, node);
                p2_func_80025eac(node);
                goto next_object;
            }
            if (r_u32(node + 8) != 0)
            {
                matrix = r_u32(0x8008A0C8);
                source = matrix - 32;
                {
                    uint32 c0 = r_u32(source), c1 = r_u32(source + 4), c2 = r_u32(source + 8), c3 = r_u32(source + 12);
                    w_u32(matrix, c0);
                    w_u32(matrix + 4, c1);
                    w_u32(matrix + 8, c2);
                    w_u32(matrix + 12, c3);
                    w_u16(matrix + 16, r_u16(source + 16));
                }
                goto visit_children;
            }
            goto next_object;
        }
        if (category == 8)
        {
            p2_sdk_8007ec64(node + 28, node + 40);
            matrix = r_u32(0x8008A0C8);
            for (i = 0; i < 3; ++i)
                w_u32(node + 40 + i * 4, r_u32(node + 40 + i * 4) + r_u32(matrix - 12 + i * 4));
            if ((work & 0x14) != 0x10 || (parent_offset == 0 && (p2_func_80011338(node + 40, r_u32(node + 24)) & 0xFF) == 0))
            {
                w_u8(node + 13, r_u8(node + 13) | 4);
                goto next_object;
            }
            for (i = 0; i < 5; ++i)
                xport_gte_write_control(i, r_u32(0x1F800264 + i * 4));
            p2_sdk_8007ec64(node + 40, r_u32(0x8008A0C8) + 20);
            matrix = r_u32(0x8008A0C8);
            for (i = 0; i < 3; ++i)
                w_u32(matrix + 20 + i * 4, r_u32(matrix + 20 + i * 4) + r_u32(0x1F800278 + i * 4));
            p2_func_80064358(node);
            w_u8(node + 13, r_u8(node + 13) & 0xFB);
            goto next_object;
        }
        if (category == 2)
        {
            p2_sdk_8007ec64(node + 88, matrix + 20);
            matrix = r_u32(0x8008A0C8);
            x = r_u32(matrix + 20) + r_u32(matrix - 12);
            y = r_u32(matrix + 24) + r_u32(matrix - 8);
            z = r_u32(matrix + 28) + r_u32(matrix - 4);
            w_u32(matrix + 20, x);
            w_u32(matrix + 24, y);
            w_u32(matrix + 28, z);

            if ((r_u32(node + 12) & 0xFF200000) != 0x01200000)
            {
                if (r_s16(node + 100) == 0x4000)
                {
                    source = matrix - 32;
                    {
                        uint32 c0 = r_u32(source), c1 = r_u32(source + 4), c2 = r_u32(source + 8), c3 = r_u32(source + 12);
                        w_u32(matrix, c0);
                        w_u32(matrix + 4, c1);
                        w_u32(matrix + 8, c2);
                        w_u32(matrix + 12, c3);
                        w_u16(matrix + 16, r_u16(source + 16));
                    }
                }
                else
                {
                    {
                        uint32 qw = (uint32)r_s16(node + 100), qx = (uint32)r_s16(node + 100 + 2), qy = (uint32)r_s16(node + 100 + 4), qz = (uint32)r_s16(node + 100 + 6);
                        uint32 xx = qx * qx, yy = qy * qy, zz = qz * qz, xy = qx * qy, xz = qx * qz, yz = qy * qz, wz = qw * qz, wy = qw * qy, wx = qw * qx;
                        w_u16(matrix, (sint32)(0x08000000u - (yy + zz)) >> 15);
                        w_u16(matrix + 8, (sint32)(0x08000000u - (xx + zz)) >> 15);
                        w_u16(matrix + 16, (sint32)(0x08000000u - (xx + yy)) >> 15);
                        w_u16(matrix + 6, (sint32)(xy - wz) >> 15);
                        w_u16(matrix + 2, (sint32)(xy + wz) >> 15);
                        w_u16(matrix + 12, (sint32)(xz + wy) >> 15);
                        w_u16(matrix + 4, (sint32)(xz - wy) >> 15);
                        w_u16(matrix + 14, (sint32)(yz - wx) >> 15);
                        w_u16(matrix + 10, (sint32)(yz + wx) >> 15);
                    }
                    source = p2_sdk_8007e944(matrix);
                    for (i = 0; i < 5; ++i)
                        xport_gte_write_control(i, r_u32(source + i * 4));
                }
            }
            else
            {
                source = r_u32(r_u32(node + 68) + 36);
                x = r_u32(node + 76);
                p2_func_80022ec4(source, input, (sint32)x < 0 ? 0 : (sint32)x);
                if ((sint32)x >= 0)
                {
                    angle = (sint16)p2_func_8005dc00(r_s32(input + 8), r_s32(input));
                    p2_func_80018fec((sint16)angle, 0, node + 100);
                    matrix = r_u32(0x8008A0C8);
                    if (r_s16(node + 100) == 0x4000)
                    {
                        source = 0x1F800044;
                        {
                            uint32 c0 = r_u32(source), c1 = r_u32(source + 4), c2 = r_u32(source + 8);
                            w_u32(matrix, c0);
                            w_u32(matrix + 4, c1);
                            w_u32(matrix + 8, c2);
                            c0 = r_u32(source + 12);
                            c1 = (uint32)r_s16(source + 16);
                            w_u32(matrix + 12, c0);
                            w_u16(matrix + 16, c1);
                        }
                    }
                    else
                    {
                        {
                            uint32 qw = (uint32)r_s16(node + 100), qx = (uint32)r_s16(node + 100 + 2), qy = (uint32)r_s16(node + 100 + 4), qz = (uint32)r_s16(node + 100 + 6);
                            uint32 xx = qx * qx, yy = qy * qy, zz = qz * qz, xy = qx * qy, xz = qx * qz, yz = qy * qz, wz = qw * qz, wy = qw * qy, wx = qw * qx;
                            w_u16(temp_matrix, (sint32)(0x08000000u - (yy + zz)) >> 15);
                            w_u16(temp_matrix + 8, (sint32)(0x08000000u - (xx + zz)) >> 15);
                            w_u16(temp_matrix + 16, (sint32)(0x08000000u - (xx + yy)) >> 15);
                            w_u16(temp_matrix + 6, (sint32)(xy - wz) >> 15);
                            w_u16(temp_matrix + 2, (sint32)(xy + wz) >> 15);
                            w_u16(temp_matrix + 12, (sint32)(xz + wy) >> 15);
                            w_u16(temp_matrix + 4, (sint32)(xz - wy) >> 15);
                            w_u16(temp_matrix + 14, (sint32)(yz - wx) >> 15);
                            w_u16(temp_matrix + 10, (sint32)(yz + wx) >> 15);
                        }
                        MulMatrix0((MATRIX *)psx_addr(0x1F800044, 18), (MATRIX *)psx_addr(temp_matrix, 18), (MATRIX *)psx_addr(r_u32(0x8008A0C8), 18));
                    }
                }
                else
                {
                    angle = p2_func_8005dcd4((sint32)(0u - r_u32(input + 4)), 4096);
                    cosine = p2_func_8005dbb8((sint16)(1024 - (angle & 0xFFF))) >> 2;
                    angle = p2_func_8005dcd4((sint32)(0u - r_u32(input + 4)), 4096);
                    sine = p2_func_8005dbb8((sint16)(angle & 0xFFF)) >> 2;
                    w_u16(0x1F800008, cosine);
                    w_u16(0x1F80000E, sine);
                    w_u16(0x1F80000A, 0u - (uint32)sine);
                    w_u16(0x1F800010, cosine);
                    angle = (sint16)p2_func_8005dc00(r_s32(input + 8), r_s32(input));
                    cosine = p2_func_8005dbb8((sint16)(1024 - angle)) >> 2;
                    angle = (sint16)p2_func_8005dc00(r_s32(input + 8), r_s32(input));
                    sine = p2_func_8005dbb8((sint16)angle) >> 2;
                    w_u16(0x1F800014, cosine);
                    w_u16(0x1F800020, 0u - (uint32)sine);
                    w_u16(0x1F800018, sine);
                    w_u16(0x1F800024, cosine);
                    MulMatrix0((MATRIX *)psx_addr(0x1F800014, 18), (MATRIX *)psx_addr(0x1F800000, 18), (MATRIX *)psx_addr(temp_matrix, 18));
                    MulMatrix0((MATRIX *)psx_addr(0x1F800044, 18), (MATRIX *)psx_addr(temp_matrix, 18), (MATRIX *)psx_addr(r_u32(0x8008A0C8), 18));
                    source = r_u32(0x8008A0C8);
                    for (i = 0; i < 5; ++i)
                        xport_gte_write_control(i, r_u32(source + i * 4));
                }
            }
            w_u8(node + 14, r_u8(node + 14) | 0x40);
            target = node + 16;

            if ((work & 0x8C) != 0)
            {
                w_u32(target + 40, 0);
                w_u32(target + 36, 0);
                w_u32(target + 32, 0);
            }
            else
            {
                matrix = r_u32(0x8008A0C8);
                w_u32(target + 32, r_u32(matrix + 20) - r_u32(target + 20));
                w_u32(target + 36, r_u32(matrix + 24) - r_u32(target + 24));
                w_u32(target + 40, r_u32(matrix + 28) - r_u32(target + 28));
            }
            matrix = r_u32(0x8008A0C8);
            {
                uint32 c0 = r_u32(matrix + 20), c1 = r_u32(matrix + 20 + 4), c2 = r_u32(matrix + 20 + 8);
                w_u32(target + 20, c0);
                w_u32(target + 20 + 4, c1);
                w_u32(target + 20 + 8, c2);
            }
            matrix = r_u32(0x8008A0C8);
            for (i = 0; i < 9; ++i)
                w_u16(target + i * 2, r_u16(matrix + transpose_offsets[i]));

            if (parent_offset != 0)
            {
                if ((r_u32(parent_offset + 28) & 0x200) != 0)
                    w_u8(node + 13, r_u8(node + 13) | 4);
                else if ((r_u8(node + 13) & 4) != 0)
                {
                    if ((work & 2) == 0)
                    {
                        w_u32(target + 40, 0);
                        w_u32(target + 36, 0);
                        w_u32(target + 32, 0);
                    }
                    w_u8(node + 13, r_u8(node + 13) & 0xFB);
                }
            }
            goto visit_children;
        }
        if (category == 6)
        {
            p2_sdk_8007ec64(node + 20, matrix + 20);
            matrix = r_u32(0x8008A0C8);
            x = r_u32(matrix + 20) + r_u32(matrix - 12);
            y = r_u32(matrix + 24) + r_u32(matrix - 8);
            z = r_u32(matrix + 28) + r_u32(matrix - 4);
            w_u32(matrix + 20, x);
            w_u32(matrix + 24, y);
            w_u32(matrix + 28, z);

            if ((work & 0x40) != 0 && scene_offset == parent_offset)
            {
                {
                    uint32 c0 = r_u32(matrix + 20), c1 = r_u32(matrix + 20 + 4), c2 = r_u32(matrix + 20 + 8);
                    w_u32(parent_offset + 160, c0);
                    w_u32(parent_offset + 160 + 4, c1);
                    w_u32(parent_offset + 160 + 8, c2);
                }
                flags = r_u32(node + 32) & 0xFFFFFFF8;
                w_u32(parent_offset + 172, flags);
                w_u32(parent_offset + 172, flags | (r_u8(node + 15) & 7));
            }
            goto visit_children;
        }
        if (category == 3)
        {
            follow = (work & 4) == 0 && node == r_u32(r_u32(0x8008A114) + 136);
            if (follow != 0)
            {
                source = r_u32(node + 32);
                {
                    uint32 c0 = r_u32(source + 20), c1 = r_u32(source + 20 + 4), c2 = r_u32(source + 20 + 8);
                    w_u32(delta, c0);
                    w_u32(delta + 4, c1);
                    w_u32(delta + 8, c2);
                }
            }
            target = r_u32(node + 32);
            matrix = r_u32(0x8008A0C8);
            x = r_u32(matrix - 32);
            y = r_u32(matrix - 28);
            z = r_u32(matrix - 24);
            w = r_u32(matrix - 20);
            w_u32(target, x);
            w_u32(target + 4, y);
            w_u32(target + 8, z);
            w_u32(target + 12, w);
            x = r_u32(matrix - 16);
            y = r_u32(matrix - 12);
            z = r_u32(matrix - 8);
            w = r_u32(matrix - 4);
            w_u32(target + 16, x);
            w_u32(target + 20, y);
            w_u32(target + 24, z);
            w_u32(target + 28, w);
            if (follow != 0)
            {
                player = p2_func_800536f0(0);
                if (player != 0 && (r_u32(player + 28) & 0x2000) == 0)
                {
                    target = r_u32(0x8008A114);
                    for (i = 0; i < 3; ++i)
                        w_u32(target + 28 + i * 4, r_u32(target + 28 + i * 4) + r_u32(delta + i * 4));
                    for (i = 0; i < 3; ++i)
                        w_u32(delta + i * 4, r_u32(r_u32(node + 32) + 20 + i * 4) - r_u32(delta + i * 4));
                    if ((r_u32(parent_offset + 28) & 0x20) != 0)
                    {
                        for (i = 0; i < 3; ++i)
                            w_u32(temp_matrix + i * 4, r_u32(target + 28 + i * 4) - r_u32(target + 4 + i * 4));
                        p2_func_80024b3c(temp_matrix);
                        target = r_u32(0x8008A114);
                        if (r_u8(target + 72) < 4)
                        {
                            w_u8(target + 72, 4);
                            target = r_u32(0x8008A114);
                        }
                        if (r_u8(target + 73) < 4)
                            w_u8(target + 73, 4);
                        target = r_u32(0x8008A114);
                        if (r_u8(target + 74) < 4)
                            w_u8(target + 74, 4);
                    }
                    target = r_u32(0x8008A114);
                    for (i = 0; i < 3; ++i)
                        w_u32(target + 4 + i * 4, r_u32(target + 4 + i * 4) + r_u32(delta + i * 4));
                }
            }
            goto visit_children;
        }
        if (category == 9)
        {
            if (r_u32(node + 40) != 0)
            {
                follow = 0;
                player = 0;
                if ((work & 4) == 0 && node == r_u32(r_u32(r_u32(0x8008A114) + 136) + 36))
                {
                    player = p2_func_800536f0(0);
                    if (player != 0 && node == r_u32(player + 132))
                        follow = (r_u32(player + 28) & 0x2400) == 0x400;
                }
                if (follow != 0)
                {
                    attachment = r_u32(player + 132);
                    if (attachment != 0 && r_u32(attachment + 40) != 0)
                    {
                        {
                            uint32 c0 = r_u32(player + 16), c1 = r_u32(player + 16 + 4), c2 = r_u32(player + 16 + 8);
                            w_u32(temp_matrix, c0);
                            w_u32(temp_matrix + 4, c1);
                            w_u32(temp_matrix + 8, c2);
                        }
                        PushMatrix();
                        source = r_u32(r_u32(player + 132) + 40);
                        for (i = 0; i < 5; ++i)
                            xport_gte_write_control(i, r_u32(source + i * 4));
                        p2_sdk_8007ec64(temp_matrix, delta);
                        PopMatrix();
                        for (i = 0; i < 3; ++i)
                        {
                            source = r_u32(r_u32(player + 132) + 40);
                            w_u32(delta + i * 4, r_u32(delta + i * 4) + r_u32(source + 20 + i * 4));
                        }
                    }
                    else
                    {
                        {
                            uint32 c0 = r_u32(player + 16), c1 = r_u32(player + 16 + 4), c2 = r_u32(player + 16 + 8);
                            w_u32(delta, c0);
                            w_u32(delta + 4, c1);
                            w_u32(delta + 8, c2);
                        }
                    }
                }
                matrix = r_u32(0x8008A0C8);
                target = r_u32(node + 40);
                w_u32(0x8008A0C8, matrix - 32);

                if ((work & 0x8C) != 0)
                {
                    w_u32(target + 40, 0);
                    w_u32(target + 36, 0);
                    w_u32(target + 32, 0);
                }
                else
                {
                    matrix = r_u32(0x8008A0C8);
                    w_u32(target + 32, r_u32(matrix + 20) - r_u32(target + 20));
                    w_u32(target + 36, r_u32(matrix + 24) - r_u32(target + 24));
                    w_u32(target + 40, r_u32(matrix + 28) - r_u32(target + 28));
                }
                matrix = r_u32(0x8008A0C8);
                {
                    uint32 c0 = r_u32(matrix + 20), c1 = r_u32(matrix + 20 + 4), c2 = r_u32(matrix + 20 + 8);
                    w_u32(target + 20, c0);
                    w_u32(target + 20 + 4, c1);
                    w_u32(target + 20 + 8, c2);
                }
                matrix = r_u32(0x8008A0C8);
                for (i = 0; i < 9; ++i)
                    w_u16(target + i * 2, r_u16(matrix + transpose_offsets[i]));

                if (parent_offset != 0)
                {
                    if ((r_u32(parent_offset + 28) & 0x200) != 0)
                        w_u8(node + 13, r_u8(node + 13) | 4);
                    else if ((r_u8(node + 13) & 4) != 0)
                    {
                        if ((work & 2) == 0)
                        {
                            w_u32(target + 40, 0);
                            w_u32(target + 36, 0);
                            w_u32(target + 32, 0);
                        }
                        w_u8(node + 13, r_u8(node + 13) & 0xFB);
                    }
                }
                target = r_u32(node + 40);
                source = r_u32(0x8008A0C8);
                {
                    uint32 c0 = r_u32(source), c1 = r_u32(source + 4), c2 = r_u32(source + 8), c3 = r_u32(source + 12);
                    w_u32(target, c0);
                    w_u32(target + 4, c1);
                    w_u32(target + 8, c2);
                    w_u32(target + 12, c3);
                    w_u16(target + 16, r_u16(source + 16));
                }
                w_u32(0x8008A0C8, r_u32(0x8008A0C8) + 32);
                if (follow != 0)
                {
                    w_u32(parent_offset + 28, r_u32(parent_offset + 28) & 0xFFFFFDFF);
                    target = r_u32(node + 40);
                    w_u8(node + 13, r_u8(node + 13) & 0xFB);
                    if (r_s32(target + 36) < 0 || (work & 8) != 0)
                    {
                        attachment = r_u32(player + 132);
                        if (attachment != 0 && r_u32(attachment + 40) != 0)
                        {
                            {
                                uint32 c0 = r_u32(player + 16), c1 = r_u32(player + 16 + 4), c2 = r_u32(player + 16 + 8);
                                w_u32(before, c0);
                                w_u32(before + 4, c1);
                                w_u32(before + 8, c2);
                            }
                            PushMatrix();
                            source = r_u32(r_u32(player + 132) + 40);
                            for (i = 0; i < 5; ++i)
                                xport_gte_write_control(i, r_u32(source + i * 4));
                            p2_sdk_8007ec64(before, player + 16);
                            PopMatrix();
                            attachment = r_u32(player + 132);
                            source = r_u32(attachment + 40);
                            w_u32(player + 16, r_u32(player + 16) + r_u32(source + 20));
                            source = r_u32(attachment + 40);
                            x = r_u32(source + 24);
                            attachment = r_u32(player + 132);
                            w_u32(player + 20, r_u32(player + 20) + x);
                            source = r_u32(attachment + 40);
                            w_u32(player + 24, r_u32(player + 24) + r_u32(source + 28));
                        }
                        else
                        {
                            {
                                uint32 c0 = r_u32(player + 16), c1 = r_u32(player + 16 + 4), c2 = r_u32(player + 16 + 8);
                                w_u32(player + 16, c0);
                                w_u32(player + 16 + 4, c1);
                                w_u32(player + 16 + 8, c2);
                            }
                        }
                        w_u32(player + 20, r_u32(delta + 4));
                        attachment = r_u32(player + 132);
                        if (attachment != 0 && r_u32(attachment + 40) != 0)
                        {
                            for (i = 0; i < 3; ++i)
                            {
                                source = r_u32(r_u32(player + 132) + 40);
                                w_u32(relative + i * 4, r_u32(player + 16 + i * 4) - r_u32(source + 20 + i * 4));
                            }
                            for (i = 0; i < 9; ++i)
                            {
                                source = r_u32(r_u32(player + 132) + 40);
                                w_u16(transposed + i * 2, r_u16(source + transpose_offsets[i]));
                            }
                            PushMatrix();
                            for (i = 0; i < 5; ++i)
                                xport_gte_write_control(i, r_u32(transposed + i * 4));
                            p2_sdk_8007ec64(relative, player + 16);
                            PopMatrix();
                        }
                        else
                        {
                            {
                                uint32 c0 = r_u32(player + 16), c1 = r_u32(player + 16 + 4), c2 = r_u32(player + 16 + 8);
                                w_u32(player + 16, c0);
                                w_u32(player + 16 + 4, c1);
                                w_u32(player + 16 + 8, c2);
                            }
                        }
                    }
                }
            }
            if ((r_u8(node + 13) & 1) != 0)
                w_u32(node + 16, r_u32(node + 16) | 1);
            else
                w_u32(node + 16, r_u32(node + 16) & 0xFFFFFFFE);
            goto visit_children;
        }
        if (category == 4)
        {
            p2_sdk_8007ec64(node + 20, temp_matrix);
            matrix = r_u32(0x8008A0C8);
            for (i = 0; i < 3; ++i)
                w_u32(temp_matrix + i * 4, r_u32(temp_matrix + i * 4) + r_u32(matrix - 12 + i * 4));
            if ((work & 0x8D) != 0 || r_u32(node + 44) != r_u32(temp_matrix) || r_u32(node + 48) != r_u32(temp_matrix + 4) || r_u32(node + 52) != r_u32(temp_matrix + 8))
            {
                {
                    uint32 c0 = r_u32(temp_matrix), c1 = r_u32(temp_matrix + 4), c2 = r_u32(temp_matrix + 8);
                    w_u32(node + 44, c0);
                    w_u32(node + 44 + 4, c1);
                    w_u32(node + 44 + 8, c2);
                }
                w_u8(node + 14, r_u8(node + 14) | 1);
            }
        }
    visit_children:
        if (r_u32(node + 8) != 0)
            p2_func_800140f8(node, parent_offset, work);
    next_object:
        node = next_node;
    }
    matrix = r_u32(0x8008A0C8) - 32;
    w_u32(0x8008A0C8, matrix);
    p2_guest_stack_pointer = saved_stack;
    return matrix;
}

uint32 p2_func_80013cec(uint32 object_offset, uint32 parent_offset)
{
    uint32 category = r_u8(object_offset + 12);
    uint32 flags, value;
    sint32 kind;
    if (category == 1)
    {
        p2_func_80027594(r_u32(object_offset + 44));
        w_u8(object_offset + 13, r_u8(object_offset + 13) | 0x80);
        return 0;
    }
    if (category == 0)
    {
        p2_func_80027594(r_u32(object_offset + 44));
        kind = r_s8(object_offset + 125);
        if (kind == 22)
        {
            p2_func_80043de4(object_offset);
            w_u8(object_offset + 13, r_u8(object_offset + 13) & 0x7F);
            return 1;
        }
        if ((((uint32)kind - 3) & 0xFF) < 2 || (((uint32)kind - 31) & 0xFF) < 2)
        {
            w_u32(object_offset + 28, r_u32(object_offset + 28) & 0xFFF03EFF);
            if (parent_offset != 0 && (r_u8(parent_offset + 13) & 1) != 0)
                w_u8(object_offset + 13, r_u8(object_offset + 13) | 1);
            else
            {
                flags = r_u8(object_offset + 13);
                value = r_u32(object_offset + 192) & 1;
                flags &= 0xFE;
                w_u8(object_offset + 13, flags);
                w_u8(object_offset + 13, flags | value);
            }
            value = r_u32(object_offset + 192);
            flags = r_u32(object_offset + 28);
            kind = r_s8(object_offset + 125);
            w_u32(object_offset + 28, flags | (value & 0x40000));
            if (kind == 4)
                w_u16(object_offset + 130, r_u32(object_offset + 196));
            else if (kind == 31)
                w_u16(object_offset + 130, r_u16(r_u32(object_offset + 108) + 130));
            else
                w_u16(object_offset + 130, 0);
            p2_func_8001b334(object_offset, 0);
            w_u8(object_offset + 13, r_u8(object_offset + 13) & 0x7F);
            return 1;
        }
        goto clear_hidden;
    }
    if (category == 7)
    {
        p2_func_80027594(r_u32(object_offset + 20));
        if (parent_offset == 0)
        {
            if ((r_u16(object_offset + 14) & 2) != 0)
            {
                w_u8(object_offset + 13, r_u8(object_offset + 13) | 1);
                return 1;
            }
            value = r_u32(object_offset + 24);
            if (value != 0xFFFFFFFF)
                w_u16(value + 8, 0xFFFF);
        }
        goto clear_hidden;
    }
    if (category == 8)
    {
        flags = r_u16(object_offset + 18);
        w_u16(object_offset + 18, flags & 0x7FFF);
        if ((flags & 0x7F) == 2 && p2_func_80055848(r_u16(r_u32(0x8008A4E4) + 262)) != 0)
        {
            w_u8(object_offset + 13, r_u8(object_offset + 13) | 0x80);
            return 0;
        }
        if ((r_u16(object_offset + 18) & 0x200) != 0)
            return 0;
        goto clear_hidden;
    }
    if (category == 9)
    {
        flags = r_u32(object_offset + 16);
        if ((flags & 8) == 0)
            goto clear_hidden;
        if ((r_u8(object_offset + 13) & 0x80) != 0)
            return 0;
        w_u32(object_offset + 16, flags & 0xFFFFFFFA);
        if ((flags & 0x20) != 0 || (parent_offset != 0 && (r_u8(parent_offset + 13) & 1) != 0))
        {
            flags = r_u8(object_offset + 13) | 1;
            value = r_u32(object_offset + 16) | 1;
            w_u8(object_offset + 13, flags);
            w_u32(object_offset + 16, value);
        }
        else
            w_u8(object_offset + 13, r_u8(object_offset + 13) & 0xFE);
        goto clear_hidden;
    }
    if (category == 2)
    {
        uint32 subtype = r_u8(object_offset + 15);
        if (subtype - 40 < 16)
        {
            uint32 first = r_u32(object_offset + 68);
            uint32 second = r_u32(object_offset + 72);
            uint32 packed = r_u32(object_offset + 12) & 0xFF020000;
            w_u32(object_offset + 68, (first & 0xFFF) | ((subtype - 39) << 12) | (second & 0x1FE0000));
            if (packed == 0x32000000 && r_s8(object_offset + 86) >= 0)
            {
                w_u32(object_offset + 80, parent_offset);
                p2_func_8001bb58(object_offset);
            }
        }
        else if ((sint8)subtype == 5)
        {
            w_u32(object_offset + 72, 0);
            w_u32(object_offset + 68, 0);
        }
        if (r_s8(object_offset + 15) == 50 && (r_u16(object_offset + 78) & 0x3FFF) == 0)
            return 0;
        goto clear_hidden;
    }
    if (category == 4)
    {
        flags = r_u8(object_offset + 14);
        w_u8(object_offset + 14, flags & 0xFB);
        if ((flags & 8) != 0 || (parent_offset != 0 && (r_u8(parent_offset + 13) & 1) != 0))
            w_u8(object_offset + 13, r_u8(object_offset + 13) | 1);
        else
            w_u8(object_offset + 13, r_u8(object_offset + 13) & 0xFE);
    }
clear_hidden:
    w_u8(object_offset + 13, r_u8(object_offset + 13) & 0x7F);
    return 0;
}

void p2_func_8001aecc(uint32 parent_offset, uint32 object_offset)
{
    uint32 saved_stack = p2_guest_stack_pointer;
    uint32 owner = parent_offset;
    uint32 category = r_u8(object_offset + 12);
    uint32 animation, flags, track, destination;
    sint32 index, frame;
    sint32 position_field, position_target, rotation_field = -1, rotation_target = 0, morph_field = -1, morph_target = 0;
    p2_guest_stack_pointer -= 0x30;
    if (category == 0)
        owner = object_offset;
    if (owner == 0)
        goto done;
    animation = r_u32(owner + 104);
    if (animation == 0)
        goto done;
    flags = r_u32(owner + 28);
    if ((flags & 0x20) != 0)
    {
        w_u32(owner + 28, flags & 0xFFFFFFDF);
        p2_func_80019cf4(owner, object_offset);
        w_u32(owner + 28, r_u32(owner + 28) | 0x20);
        goto done;
    }
    switch (category)
    {
        case 0:
            if (r_u32(owner + 120) == 0xFFFFFFFF)
                goto done;
            position_field = 120;
            position_target = 16;
            rotation_field = 121;
            rotation_target = 84;
            morph_field = 123;
            morph_target = 50;
            break;
        case 5:
            position_field = 28;
            position_target = 16;
            rotation_field = 29;
            rotation_target = 40;
            morph_field = 31;
            morph_target = 32;
            break;
        case 7:
            position_field = 24;
            position_target = 28;
            break;
        case 8:
            position_field = 14;
            position_target = 28;
            break;
        case 2:
            position_field = 84;
            position_target = 88;
            rotation_field = 85;
            rotation_target = 100;
            break;
        case 4:
            position_field = 15;
            position_target = 20;
            break;
        default:
            goto done;
    }
    index = r_s8(object_offset + (uint32)position_field);
    if (index >= 0)
    {
        track = r_u32(animation + 8) + (uint32)index * 20;
        flags = r_u8(track + 2);
        if ((flags & 8) != 0)
        {
            frame = r_s16(track + 8);
            w_u8(track + 2, flags & 0xF7);
            p2_func_80017364(frame, track, object_offset + (uint32)position_target);
        }
    }
    if (category == 7)
    {
        index = r_s8(object_offset + 26);
        if (index >= 0)
        {
            track = r_u32(animation + 16) + (uint32)index * 16;
            flags = r_u8(track + 2);
            if ((flags & 8) != 0)
            {
                w_u8(track + 2, flags & 0xF7);
                destination = p2_guest_stack_pointer + 0x10;
                w_u32(destination, (uint32)r_s16(object_offset + 40) << 5);
                w_u32(destination + 4, (uint32)r_s16(object_offset + 42) << 5);
                p2_func_80017364(r_s16(track + 8), track, destination);
                w_u16(object_offset + 40, r_s32(destination) >> 5);
                w_u16(object_offset + 42, r_s32(destination + 4) >> 5);
            }
        }
        goto done;
    }
    if (rotation_field >= 0)
    {
        index = r_s8(object_offset + (uint32)rotation_field);
        if (index >= 0)
        {
            track = r_u32(animation + 12) + (uint32)index * 16;
            flags = r_u8(track + 2);
            if ((flags & 8) != 0)
            {
                frame = r_s16(track + 8);
                w_u8(track + 2, flags & 0xF7);
                p2_func_80017938(frame, track, object_offset + (uint32)rotation_target);
            }
        }
    }
    if (morph_field >= 0)
    {
        index = r_s8(object_offset + (uint32)morph_field);
        if (index >= 0)
        {
            track = r_u32(animation + 20) + (uint32)index * 24;
            flags = r_u8(track + 2);
            if ((flags & 8) != 0)
            {
                frame = r_s16(track + 8);
                w_u8(track + 2, flags & 0xF7);
                p2_func_80019684(frame, track, owner, object_offset + (uint32)morph_target);
            }
        }
    }
done:
    p2_guest_stack_pointer = saved_stack;
}
