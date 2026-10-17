#include <string.h>

#include "game.h"

void SceneSection::Relocate(const MaterialResourceHeader *resource)
{
    FUNCTION_MARKER(0x80068A24, "SLUS_005.78");
    uint32 *root_fields[] = {&links.first_child, &object_heads[0], &object_heads[1], &object_heads[2], &object_heads[3], &object_heads[4], &object_heads[7], &object_heads[5], &object_heads[6], &object_heads[8], &object_heads[9], &object_heads[10], &object_heads[11], &object_heads[12], &object_heads[13], &object_heads[14], &object_heads[15], &object_heads[16]};
    const uint32 mesh_fields[] = {0, 4, 8, 132, 140, 40};
    const uint32 basic_fields[] = {0, 4, 8};
    const uint32 object_fields[] = {0, 4, 8, 16, 20, 36};
    const uint32 effect_fields[] = {0, 4, 8, 44, 40, 32};
    const uint32 final_heads[] = {13, 15, 14};
    uint32 node, value, field, i, group;
    for (i = 0; i < 18; ++i)
    {
        value = *root_fields[i];
        if (value != 0)
            *root_fields[i] = data_offset + value;
    }
    node = object_heads[0];
    transient_links.first_child = 0;
    for (group = 0; group < 2; ++group)
    {
        if (group != 0)
            node = object_heads[1];
        while (node != 0)
        {
            for (i = 0; i < 6; ++i)
            {
                field = node + mesh_fields[i];
                value = r_u32(field);
                if (value != 0)
                    w_u32(field, data_offset + value);
            }
            value = r_u32(node + 36);
            if (value != 0)
            {
                uint32 polygon = data_offset + value;
                uint32 flags = r_u32(node + 28) & 12;
                w_u32(node + 36, polygon);
                if (flags != 8)
                {
                    uint32 count = (uint32)r_s16(node + 32);
                    do
                    {
                        uint32 index = r_u32(polygon);
                        --count;
                        w_u32(polygon, resource->textures_offset + index * 20);
                        polygon += 24;
                    } while (count != 0);
                }
            }
            value = r_u32(node + 104);
            if (value != 0)
            {
                uint32 animation = data_offset + value;
                w_u32(node + 104, animation);
                RelocateAnimationData((AnimationHeader *)psx_addr(animation, 24), data_offset);
            }
            value = r_u32(node + 44);
            if (value != 0)
                w_u32(node + 44, data_offset + value);
            if (r_s8(node + 125) == 31)
                w_u32(node + 108, data_offset + r_u32(node + 108));
            node = r_u32(node);
        }
    }
    node = object_heads[2];
    while (node != 0)
    {
        const uint32 fields[] = {0, 4, 8, 40};
        for (i = 0; i < 4; ++i)
        {
            field = node + fields[i];
            value = r_u32(field);
            if (value != 0)
                w_u32(field, data_offset + value);
        }
        value = r_u32(node + 36);
        if (value != 0)
        {
            uint32 polygon = data_offset + value;
            sint32 active = r_s16(node + 34);
            w_u32(node + 36, polygon);
            if (active > 0 && (r_u32(node + 28) & 0x400) == 0)
            {
                uint32 count = (uint32)r_s16(node + 32);
                do
                {
                    uint32 index = r_u32(polygon);
                    --count;
                    w_u32(polygon, resource->textures_offset + index * 20);
                    polygon += 24;
                } while (count != 0);
            }
        }
        value = r_u32(node + 44);
        if (value != 0)
            w_u32(node + 44, data_offset + value);
        node = r_u32(node);
    }
    node = object_heads[3];
    while (node != 0)
    {
        for (i = 0; i < 6; ++i)
        {
            field = node + object_fields[i];
            value = r_u32(field);
            if (value != 0)
                w_u32(field, data_offset + value);
        }
        value = r_u32(node + 28);
        if (value != 0 && (r_u32(r_u32(node + 36) + 16) & 0x100010) != 0)
        {
            uint32 flags = r_u16(node + 14);
            uint32 base = data_offset;
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
            w_u32(node + 32, data_offset + value);
        value = r_u32(node + 52);
        if (value != 0)
            w_u32(node + 52, data_offset + value);
        node = r_u32(node);
    }
    node = object_heads[4];
    while (node != 0)
    {
        sint32 kind;
        for (i = 0; i < 3; ++i)
        {
            field = node + basic_fields[i];
            value = r_u32(field);
            if (value != 0)
                w_u32(field, data_offset + value);
        }
        kind = r_s8(node + 15);
        if (kind == 8)
            RegisterCutsceneCamera(node);
        else
        {
            value = r_u32(node + 68);
            if (value != 0 && (kind == 7 || ((uint32)(kind - 26) & 0xFF) < 2))
                w_u32(node + 68, data_offset + value);
        }
        node = r_u32(node);
    }
    for (group = 0; group < 2; ++group)
    {
        const uint32 fields[] = {0, 4, 8, 68, 72};
        uint32 count = group == 0 ? 3 : 5;
        node = object_heads[5 + group];
        while (node != 0)
        {
            for (i = 0; i < count; ++i)
            {
                field = node + fields[i];
                value = r_u32(field);
                if (value != 0)
                    w_u32(field, data_offset + value);
            }
            node = r_u32(node);
        }
    }
    node = object_heads[8];
    while (node != 0)
    {
        uint32 type = r_u16(node + 18) & 0x7F;
        uint32 next = r_u32(node);
        w_u32(node + 20, resource_groups[type + 13].material_offset);
        if (next != 0)
            w_u32(node, data_offset + next);
        for (i = 1; i < 3; ++i)
        {
            field = node + basic_fields[i];
            value = r_u32(field);
            if (value != 0)
                w_u32(field, data_offset + value);
        }
        node = r_u32(node);
    }
    node = object_heads[9];
    while (node != 0)
    {
        for (i = 0; i < 6; ++i)
        {
            field = node + effect_fields[i];
            value = r_u32(field);
            if (value != 0)
                w_u32(field, data_offset + value);
        }
        node = r_u32(node);
    }
    node = object_heads[10];
    while (node != 0)
    {
        uint32 base, payload;
        value = r_u32(node);
        if (value != 0)
            w_u32(node, data_offset + value);
        base = data_offset;
        value = r_u32(node + 60);
        payload = r_u32(node + 56);
        w_u32(node + 60, base + value);
        if (payload != 0)
        {
            payload += data_offset;
            w_u32(node + 56, payload);
            value = r_u32(payload + 12);
            if (value != 0)
                w_u32(payload + 12, data_offset + value);
        }
        node = r_u32(node);
    }
    node = object_heads[11];
    while (node != 0)
    {
        value = r_u32(node);
        if (value != 0)
            w_u32(node, data_offset + value);
        node = r_u32(node);
    }
    node = object_heads[12];
    while (node != 0)
    {
        uint32 index = r_u32(node + 16);
        uint32 materials = resource->textures_offset;
        uint32 payload = r_u32(node + 20);
        w_u32(node + 16, materials + index * 20);
        if (payload != 0)
            w_u32(node + 20, data_offset + payload);
        for (i = 0; i < 3; ++i)
        {
            field = node + basic_fields[i];
            value = r_u32(field);
            if (value != 0)
                w_u32(field, data_offset + value);
        }
        if (r_s8(node + 27) != -1)
        {
            uint32 base = data_offset;
            payload = r_u32(node + 24);
            payload += base;
            w_u32(node + 24, payload);
            value = r_u32(payload + 12);
            if (value != 0)
                w_u32(payload + 12, data_offset + value);
        }
        node = r_u32(node);
    }
    for (group = 0; group < 3; ++group)
    {
        node = object_heads[final_heads[group]];
        while (node != 0)
        {
            for (i = 0; i < 3; ++i)
            {
                field = node + basic_fields[i];
                value = r_u32(field);
                if (value != 0)
                    w_u32(field, data_offset + value);
            }
            node = r_u32(node);
        }
    }
}

uint32 SceneSection::Activate()
{
    FUNCTION_MARKER(0x80025828, "SLUS_005.78");
    uint32 scene, value, flags, result;
    section_index = r_u8(0x8008A30C);
    palette_bank = r_u8(0x8008A514);
    scene = (uint32)r_s8(0x8008A30C);
    scene = r_u32(0x8008A4E4) + scene * 24;
    value = r_u32(scene + 8);
    flags = state_flags;
    origin[0] = value;
    origin[1] = r_u32(scene + 12);
    value = r_u32(scene + 16);
    flags &= 0xFFE7;
    state_flags = flags;
    origin[2] = value;
    result = (r_u8(scene) & 0x10) | 0x40;
    state_flags = flags | result;
    return result;
}


sint32 SceneSection::FindObjectId(const SceneObjectName *name) const
{
    FUNCTION_MARKER(0x80022A38, "SLUS_005.78");
    uint32 node, kind, flags;
    const SceneObjectName *object_name;
    node = object_heads[3];
    while (node != 0)
    {
        object_name = (const SceneObjectName *)psx_addr(node + 60, sizeof(SceneObjectName));
        if (strncmp(object_name->prefix, name->prefix, 2) == 0)
        {
            kind = object_name->kind;
            if (kind != 60 && kind != 62)
            {
                flags = r_u16(node + 14);
                if ((flags & 8) == 0 || kind == name->kind)
                    return (sint32)(flags >> 6);
            }
        }
        node = r_u32(node);
    }
    return -1;
}


uint32 SceneSection::FindObjectOffset(sint32 index) const
{
    FUNCTION_MARKER(0x800229FC, "SLUS_005.78");
    uint32 node = object_heads[3];
    while (node != 0)
    {
        if ((r_u16(node + 14) >> 6) == (uint32)index)
            return node;
        node = r_u32(node);
    }
    return 0;
}
