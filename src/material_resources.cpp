#include "game.h"

uint32 MaterialResourceHeader::Relocate(uint32 base, sint32 mode)
{
    uint32 pointer;
    uint32 remaining;
    uint32 record;
    TextureDescriptor *texture;
    const TextureDescriptor *referenced_texture;
    MaterialRecord *material_rec;
    uint32 source;
    uint32 flags;
    uint32 count;
    uint32 index;
    uint32 result;
    uint32 words[3];
    uint16 preserved;
    uint16 first;
    uint16 second;
    uint8 coordinates[8];
    uint8 x;
    uint8 y;
    FUNCTION_MARKER(0x80068790, "SLUS_005.78");
    pointer = metadata[3];
    if (pointer != 0)
        metadata[3] = base + pointer;
    if (texture_count != 0)
    {
        remaining = texture_count;
        record = base + textures_offset;
        textures_offset = record;
        do
        {
            texture = (TextureDescriptor *)psx_addr(record, sizeof(TextureDescriptor));
            flags = texture->flags;
            if ((flags & 0x1400) == 0x1000)
            {
                if (texture->payload.words[0] >= 2001)
                    texture->payload.words[0] = 0;
                source = textures_offset + 20 * texture->payload.words[0];
                coordinates[3] = texture->payload.bytes[7];
                coordinates[4] = texture->payload.bytes[8];
                coordinates[5] = texture->payload.bytes[9];
                coordinates[6] = texture->payload.bytes[10];
                texture->payload.words[0] = source;
                referenced_texture = (const TextureDescriptor *)psx_addr(source, sizeof(TextureDescriptor));
                x = referenced_texture->payload.bytes[0];
                y = referenced_texture->payload.bytes[1];
                coordinates[0] = texture->payload.bytes[4];
                coordinates[1] = texture->payload.bytes[5];
                coordinates[2] = texture->payload.bytes[6];
                coordinates[7] = texture->payload.bytes[11];
                for (index = 0; index < 8; ++index)
                    texture->payload.bytes[4 + index] = coordinates[index] + ((index & 1) ? y : x);
            }
            else if (flags & 0x400)
            {
                if (flags & 0x800)
                {
                    preserved = texture->payload.halfwords[3];
                    source = textures_offset + 20 * texture->payload.words[0];
                    referenced_texture = (const TextureDescriptor *)psx_addr(source, sizeof(TextureDescriptor));
                    words[0] = referenced_texture->payload.words[0];
                    words[1] = referenced_texture->payload.words[1];
                    words[2] = referenced_texture->payload.words[2];
                    texture->payload.words[0] = words[0];
                    texture->payload.words[1] = words[1];
                    texture->payload.words[2] = words[2];
                    if (texture->flags & 0x80000)
                        texture->payload.halfwords[3] = preserved;
                }
                else if (mode == 5)
                {
                    first = texture->payload.halfwords[2];
                    second = texture->payload.halfwords[3];
                    texture->payload.halfwords[2] = first | 0x10;
                    texture->payload.halfwords[3] = second | 0x4000;
                }
            }
            record += 20;
            --remaining;
        } while (remaining != 0);
    }
    pointer = metadata[1];
    if (pointer != 0)
        metadata[1] = base + pointer;
    pointer = materials_offset;
    result = base + pointer;
    if (pointer != 0)
    {
        record = result;
        remaining = material_count;
        materials_offset = record;
        do
        {
            material_rec = (MaterialRecord *)psx_addr(record, sizeof(MaterialRecord));
            pointer = material_rec->texture_offset;
            if (pointer != 0)
                material_rec->texture_offset = base + pointer;
            pointer = material_rec->animation_offset;
            if (pointer != 0)
                material_rec->animation_offset = base + pointer;
            result = material_rec->kind;
            if (result == 0)
            {
                result = base + material_rec->data_offset;
                material_rec->data_offset = result;
            }
            else if (result == 2)
            {
                source = base + material_rec->data_offset;
                result = material_rec->flags & 0x80;
                material_rec->data_offset = source;
                if (result == 0 && mode == 5)
                {
                    result = material_rec->animation_offset;
                    count = r_u8(result + 5);
                    if (count != 0)
                    {
                        ++count;
                        pointer = source + 6;
                        do
                        {
                            first = r_u16(pointer - 2);
                            second = r_u16(pointer);
                            --count;
                            result = first | 0x10;
                            w_u16(pointer - 2, result);
                            w_u16(pointer, second | 0x4000);
                            pointer += 8;
                        } while (count != 0);
                    }
                }
            }
            record += 16;
            --remaining;
        } while (remaining != 0);
    }
    return result;
}

uint32 RelocateMaterialResource(uint32 object, sint32 mode)
{
    FUNCTION_MARKER(0x80069F40, "SLUS_005.78");
    return ((MaterialResourceHeader *)psx_addr(object, sizeof(MaterialResourceHeader)))->Relocate(object, mode);
}

uint32 BindModelMaterials(uint32 object, uint32 header)
{
    MaterialResourceHeader *resource;
    uint32 result;
    uint32 count;
    MaterialRecord *destination;
    uint32 source;
    FUNCTION_MARKER(0x80069F64, "SLUS_005.78");
    result = p2_func_80068570(object, header);
    resource = (MaterialResourceHeader *)psx_addr(header, sizeof(MaterialResourceHeader));
    count = resource->material_count;
    if ((sint32)count > 0)
    {
        destination = material_registry.records + material_registry.count;
        source = resource->materials_offset;
        p2_func_8006637c((uint8 *)destination, (const uint8 *)psx_addr(source, 16 * count), 16 * count);
        result = material_registry.count + resource->material_count;
        material_registry.count = result;
    }
    return result;
}

uint32 DrawTexturedQuad(uint32 position, uint32 extent, uint32 group, uint32 blend)
{
    const TextureDescriptor *descriptor;
    uint32 packet;
    uint32 texture;
    uint32 corner0;
    uint32 corner1;
    uint32 corner2;
    uint32 corner3;
    uint32 page;
    uint32 color;
    uint32 right;
    uint32 bottom;
    uint32 result;
    FUNCTION_MARKER(0x80064E74, "SLUS_005.78");
    packet = render_commands.packet_offset;
    result = packet < render_commands.packet_limit;
    if (result == 0)
        return result;
    descriptor = (const TextureDescriptor *)psx_addr(group, sizeof(TextureDescriptor));
    texture = descriptor->payload.words[0];
    corner2 = texture >> 16;
    corner1 = (texture & 0xFF00) | (corner2 & 255);
    corner3 = (corner2 & 0xFF00) | (texture & 255);
    if ((extent & 0xFFFF) == 0)
    {
        extent |= (corner2 & 255) - (texture & 255) + 1;
        ++corner1;
        ++corner2;
    }
    if ((extent >> 16) == 0)
    {
        extent |= ((((corner2 >> 8) & 255) - ((texture >> 8) & 255)) + 1) << 16;
        corner2 += 256;
        corner3 += 256;
    }
    page = descriptor->payload.words[1];
    corner0 = (texture & 0xFFFF) | (page & 0xFFFF0000);
    if (render_commands.overlay_head == 0)
        render_commands.overlay_head = (packet);
    w_u32(packet, ((packet + 40) & 0xFFFFFF) | 0x09000000);
    if (blend != 0)
    {
        corner1 |= (page | (((blend - 1) & 3) << 5)) << 16;
        color = 0x2F000000;
    }
    else
    {
        corner1 |= page << 16;
        color = 0x2D000000;
    }
    w_u32(packet + 4, color);
    w_u32(packet + 12, corner0);
    right = (position & 0xFFFF) + (extent & 0xFFFF);
    bottom = (position & 0xFFFF0000) + (extent & 0xFFFF0000);
    w_u32(packet + 16, right | (position & 0xFFFF0000));
    result = packet + 40;
    w_u32(packet + 8, position);
    w_u32(packet + 20, corner1);
    w_u32(packet + 24, (position & 0xFFFF) | bottom);
    w_u32(packet + 28, corner3);
    w_u32(packet + 32, right | bottom);
    w_u32(packet + 36, corner2);
    render_commands.overlay_tail = (packet);
    render_commands.packet_offset = (result);
    return result;
}
