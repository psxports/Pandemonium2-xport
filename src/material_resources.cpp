#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "game.h"

uint32 TextureDescriptor::WritePolygonUV(uint32 destination_offset, uint32 stride) const
{
    FUNCTION_MARKER(0x800612C0, "SLUS_005.78");
    uint32 flags = this->flags;
    const TexturePayload *texture_fields;
    uint32 uv0, uv1, uv2, uv3;
    uint32 packed;
    if ((flags & 0x1400) == 0x1000)
    {
        const TextureDescriptor *texture;
        uv0 = payload.words[1];
        texture = (const TextureDescriptor *)(source_texture);
        packed = payload.words[2];
        texture_fields = &texture->payload;
        uv1 = uv0 >> 16;
        uv2 = packed >> 16;
        uv3 = packed & 0xFFFF;
        if ((flags & 0x1E000) != 0)
        {
            uint32 right = texture->payload.bytes[2];
            uint32 left = texture->payload.bytes[0];
            uint32 bottom = texture->payload.bytes[3];
            uint32 top = texture->payload.bytes[1];
            uint32 width = (uint32)((sint32)(right - left + 1) >> 1);
            uint32 height = (uint32)((sint32)(bottom - top + 1) >> 1);
            uint32 counter = texture->payload.words[2];
            uint32 u = 0, v = 0;
            if ((flags & 0x6000) != 0)
            {
                uint32 remainder;
                if (width == 0)
                    xport_mips_break(7);
                remainder = width == 0 ? counter : counter % width;
                u = (flags & 0x2000) != 0 ? remainder : width - remainder;
            }
            if ((flags & 0x18000) != 0)
            {
                uint32 remainder;
                if (height == 0)
                    xport_mips_break(7);
                remainder = height == 0 ? counter : counter % height;
                v = (flags & 0x8000) != 0 ? remainder : height - remainder;
            }
            packed = u | (v << 8);
            uv0 += packed;
            uv1 += packed;
            uv3 += packed;
            uv2 += packed;
        }
    }
    else
    {
        texture_fields = &payload;
        if ((flags & 0x1E000) != 0)
        {
            uint32 left = payload.bytes[0];
            uint32 right = payload.bytes[2];
            uint32 top = payload.bytes[1];
            uint32 bottom = payload.bytes[3];
            uint32 counter = payload.words[2];
            uint32 width = right - left + 1;
            uint32 height = bottom - top + 1;
            if ((flags & 0x6000) != 0)
            {
                uint32 remainder;
                width >>= 1;
                if (width == 0)
                    xport_mips_break(7);
                remainder = width == 0 ? counter : counter % width;
                left += (flags & 0x2000) != 0 ? remainder : width - remainder;
            }
            right = left + width - 1;
            if ((flags & 0x18000) != 0)
            {
                uint32 remainder;
                height >>= 1;
                if (height == 0)
                    xport_mips_break(7);
                remainder = height == 0 ? counter : counter % height;
                top += (flags & 0x8000) != 0 ? remainder : height - remainder;
            }
            bottom = top + height - 1;
            uv0 = left | (top << 8) | (right << 16) | (bottom << 24);
        }
        else
            uv0 = payload.words[0];
        uv3 = uv0 >> 16;
        if ((flags & 8) != 0)
        {
            uint32 right = uv3 & 0xFF;
            uv3 = (uv3 & 0xFFFFFF00) | (uv0 & 0xFF);
            uv0 = (uv0 & 0xFFFFFF00) | right;
        }
        if ((flags & 0x10) != 0)
        {
            uint32 bottom = uv3 & 0xFF00;
            uv3 = (uv3 & 0xFFFF00FF) | (uv0 & 0xFF00);
            uv0 = (uv0 & 0xFFFF00FF) | bottom;
        }
        uv1 = (uv0 & 0xFF00) | (uv3 & 0xFF);
        uv2 = (uv3 & 0xFF00) | (uv0 & 0xFF);
    }
    packed = texture_fields->words[1];
    uv1 |= ((flags & 0x60) << 16) | (packed << 16);
    uv0 = (uv0 & 0xFFFF) | (packed & 0xFFFF0000);
    w_u32(destination_offset, uv0);
    w_u32(destination_offset + stride * 4, uv1);
    w_u32(destination_offset + stride * 8, uv2);
    destination_offset += stride * 12;
    w_u32(destination_offset, uv3);
    return destination_offset;
}

void MaterialResourceHeader::Relocate(const uint8 *input, uint32 bytes, sint32 mode)
{
    FUNCTION_MARKER(0x80068790, "SLUS_005.78");
    if (input == 0 || bytes < sizeof(EncodedMaterialResourceHeader))
    {
        fprintf(stderr, "P2: Truncated material resource header\n");
        abort();
    }
    uint32 new_material_count = xport_load_le32(input);
    uint32 material_offset = xport_load_le32(input + 4);
    uint32 new_texture_count = xport_load_le32(input + 24);
    uint32 texture_offset = xport_load_le32(input + 28);
    uint32 payload_offsets[2] = {xport_load_le32(input + 12), xport_load_le32(input + 20)};
    if (new_texture_count > 0xFFFFFFFFu / sizeof(TextureDescriptor) || new_material_count > 0xFFFFFFFFu / sizeof(MaterialRecord) || texture_offset > bytes || material_offset > bytes || (new_texture_count != 0 && (texture_offset < sizeof(EncodedMaterialResourceHeader) || (texture_offset & 3) != 0 || new_texture_count > (bytes - texture_offset) / sizeof(EncodedTextureDescriptor))) || (new_material_count == 0 && material_offset != 0) || (new_material_count != 0 && (material_offset < sizeof(EncodedMaterialResourceHeader) || (material_offset & 3) != 0 || new_material_count > (bytes - material_offset) / sizeof(EncodedMaterialRecord))) || payload_offsets[0] > bytes || payload_offsets[1] > bytes)
    {
        fprintf(stderr, "P2: Material resource tables exceed their file extent\n");
        abort();
    }
    uint8 *new_data = (uint8 *)malloc(bytes);
    TextureDescriptor *new_textures = new_texture_count != 0 ? (TextureDescriptor *)calloc(new_texture_count, sizeof(TextureDescriptor)) : 0;
    MaterialRecord *new_materials = new_material_count != 0 ? (MaterialRecord *)calloc(new_material_count, sizeof(MaterialRecord)) : 0;
    if (new_data == 0 || (new_texture_count != 0 && new_textures == 0) || (new_material_count != 0 && new_materials == 0))
    {
        fprintf(stderr, "P2: Cannot allocate native material catalogs\n");
        abort();
    }
    memcpy(new_data, input, bytes);
    free(textures);
    free(materials);
    free(data);
    textures = new_textures;
    materials = new_materials;
    data = new_data;
    data_bytes = bytes;
    texture_count = new_texture_count;
    material_count = new_material_count;
    metadata_words[0] = xport_load_le32(data + 8);
    metadata_words[1] = xport_load_le32(data + 16);
    metadata_payloads[1] = payload_offsets[1] != 0 ? data + payload_offsets[1] : 0;
    for (uint32 texture_index = 0; texture_index < texture_count; ++texture_index)
    {
        const uint8 *encoded = data + texture_offset + texture_index * sizeof(EncodedTextureDescriptor);
        textures[texture_index].flags = xport_load_le32(encoded);
        memcpy(&textures[texture_index].payload, encoded + 4, sizeof(TexturePayload));
    }
    uint32 remaining, flags, count, index;
    TextureDescriptor *texture;
    const TextureDescriptor *referenced_texture;
    uint32 words[3];
    uint16 preserved, first, second;
    uint8 coordinates[8], x, y;
    if (texture_count != 0)
    {
        remaining = texture_count;
        texture = textures;
        do
        {
            flags = texture->flags;
            if ((flags & 0x1400) == 0x1000)
            {
                if (texture->payload.words[0] >= 2001)
                    texture->payload.words[0] = 0;
                coordinates[3] = texture->payload.bytes[7];
                coordinates[4] = texture->payload.bytes[8];
                coordinates[5] = texture->payload.bytes[9];
                coordinates[6] = texture->payload.bytes[10];
                if (texture->payload.words[0] >= texture_count)
                {
                    fprintf(stderr, "P2: Atlas texture index exceeds its catalog\n");
                    abort();
                }
                texture->source_texture = textures + texture->payload.words[0];
                referenced_texture = texture->source_texture;
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
                    if (texture->payload.words[0] >= texture_count)
                    {
                        fprintf(stderr, "P2: Copied texture index exceeds its catalog\n");
                        abort();
                    }
                    referenced_texture = textures + texture->payload.words[0];
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
            ++texture;
            --remaining;
        } while (remaining != 0);
    }
    metadata_payloads[0] = payload_offsets[0] != 0 ? data + payload_offsets[0] : 0;
    for (uint32 material_index = 0; material_index < material_count; ++material_index)
    {
        const uint8 *encoded = data + material_offset + material_index * sizeof(EncodedMaterialRecord);
        MaterialRecord *material_rec = materials + material_index;
        uint32 texture_value = xport_load_le32(encoded);
        uint32 animation_value = xport_load_le32(encoded + 8);
        uint32 data_value = xport_load_le32(encoded + 12);
        material_rec->kind = encoded[4];
        material_rec->flags = encoded[5];
        material_rec->cycle_count = xport_load_le16(encoded + 6);
        if (texture_value != 0)
        {
            if (texture_value < texture_offset || (texture_value - texture_offset) % sizeof(EncodedTextureDescriptor) != 0 || (texture_value - texture_offset) / sizeof(EncodedTextureDescriptor) >= texture_count)
            {
                fprintf(stderr, "P2: Material texture reference exceeds its catalog\n");
                abort();
            }
            material_rec->texture = textures + (texture_value - texture_offset) / sizeof(EncodedTextureDescriptor);
        }
        if (animation_value != 0)
        {
            uint32 animation_bytes = material_rec->kind == 0 && material_rec->cycle_count != 0 ? (uint32)material_rec->cycle_count * 8 : 8;
            if (animation_value > bytes || animation_bytes > bytes - animation_value)
            {
                fprintf(stderr, "P2: Material animation exceeds resource payload\n");
                abort();
            }
            material_rec->animation = data + animation_value;
        }
        if (material_rec->kind == 0 || material_rec->kind == 2)
        {
            uint32 minimum = material_rec->kind == 0 ? 2 : 8;
            if (data_value > bytes || minimum > bytes - data_value)
            {
                fprintf(stderr, "P2: Material data exceeds resource payload\n");
                abort();
            }
            material_rec->data = data + data_value;
        }
        if (material_rec->kind == 2 && (material_rec->flags & 0x80) == 0 && mode == 5)
        {
            count = material_rec->animation[5];
            if (count != 0)
            {
                ++count;
                if (count > (bytes - data_value) / 8)
                {
                    fprintf(stderr, "P2: Material UV frames exceed resource payload\n");
                    abort();
                }
                uint8 *coordinates_pointer = material_rec->data + 6;
                do
                {
                    first = xport_load_le16(coordinates_pointer - 2);
                    second = xport_load_le16(coordinates_pointer);
                    --count;
                    xport_store_le16(coordinates_pointer - 2, first | 0x10);
                    xport_store_le16(coordinates_pointer, second | 0x4000);
                    coordinates_pointer += 8;
                } while (count != 0);
            }
        }
    }
}

void RelocateMaterialResource(MaterialResourceHeader *resource, const uint8 *input, uint32 bytes, sint32 mode)
{
    FUNCTION_MARKER(0x80069F40, "SLUS_005.78");
    resource->Relocate(input, bytes, mode);
}

void BindModelMaterials(AnimatedObject **object_slot, uint8 **model_owner, const MaterialResourceHeader *resource, uint32 bytes, MeshFace **polygon_owner, MeshVertex **vertex_owner)
{
    FUNCTION_MARKER(0x80069F64, "SLUS_005.78");
    uint8 *input = *model_owner;
    if (input == 0 || bytes < 232)
    {
        fprintf(stderr, "P2: Invalid model codec owner\n");
        abort();
    }
    uint32 clone_bytes = xport_load_le32(input + 224);
    if (clone_bytes < 232 || clone_bytes > bytes)
    {
        fprintf(stderr, "P2: Model clone prefix exceeds input\n");
        abort();
    }
    uint32 capacity = clone_bytes / 4 + 1;
    if (capacity > 0xFFFFFFFFu / sizeof(ModelStorageRecord))
    {
        fprintf(stderr, "P2: Model layout capacity exceeds native range\n");
        abort();
    }
    ModelStorageRecord *records = (ModelStorageRecord *)malloc(capacity * sizeof(ModelStorageRecord));
    uint32 *pending = (uint32 *)malloc(capacity * sizeof(uint32));
    if (records == 0 || pending == 0)
    {
        fprintf(stderr, "P2: Cannot allocate model layout\n");
        abort();
    }
    records[0].source_offset = 0;
    records[0].source_bytes = 232;
    records[0].storage_offset = 0;
    records[0].storage_bytes = sizeof(AnimatedObject);
    records[0].kind = ModelStorageRoot;
    uint32 record_count = 1;
    uint32 pending_count = 1;
    uint32 pending_index = 0;
    pending[0] = xport_load_le32(input + 8);
    while (pending_index < pending_count)
    {
        uint32 offset = pending[pending_index++];
        if (offset == 0)
            continue;
        uint32 found = 0;
        for (uint32 index = 0; index < record_count; ++index)
        {
            if (records[index].source_offset == offset)
            {
                found = 1;
                break;
            }
        }
        if (found != 0)
            continue;
        if ((offset & 3) != 0 || offset < 232 || offset > clone_bytes || 16 > clone_bytes - offset)
        {
            fprintf(stderr, "P2: Model node lies outside clone prefix\n");
            abort();
        }
        uint32 kind = input[offset + 12];
        uint32 source_bytes = kind == 5 || kind == 7 ? 48 : (kind == 6 ? 36 : 16);
        uint32 storage_bytes = kind == 5 ? sizeof(MeshPartNode) : (kind == 6 ? sizeof(SphereNode) : (kind == 7 ? sizeof(SpriteRecord) : sizeof(SceneNode)));
        if (source_bytes > clone_bytes - offset || record_count >= capacity || pending_count > capacity - 2)
        {
            fprintf(stderr, "P2: Model node layout exceeds capacity\n");
            abort();
        }
        records[record_count].source_offset = offset;
        records[record_count].source_bytes = source_bytes;
        records[record_count].storage_offset = 0;
        records[record_count].storage_bytes = storage_bytes;
        records[record_count].kind = kind;
        ++record_count;
        pending[pending_count++] = xport_load_le32(input + offset + 8);
        pending[pending_count++] = xport_load_le32(input + offset + 4);
    }
    uint32 node_count = record_count;
    for (uint32 node_index = 0; node_index < node_count; ++node_index)
    {
        const ModelStorageRecord *node = records + node_index;
        uint32 offset = node->kind == ModelStorageRoot ? xport_load_le32(input + 44) : (node->kind == 7 ? xport_load_le32(input + node->source_offset + 20) : 0);
        if (offset == 0)
            continue;
        if ((offset & 3) != 0 || offset < 4 || offset > clone_bytes)
        {
            fprintf(stderr, "P2: Model list header exceeds clone prefix\n");
            abort();
        }
        uint32 count = xport_load_le32(input + offset - 4) & 255;
        if (count > (clone_bytes - offset) / sizeof(uint32))
        {
            fprintf(stderr, "P2: Model list exceeds clone prefix\n");
            abort();
        }
        uint32 list_bytes = (count + 1) * sizeof(uint32);
        uint32 found = 0;
        for (uint32 index = node_count; index < record_count; ++index)
        {
            if (records[index].source_offset == offset - 4)
            {
                if (records[index].source_bytes != list_bytes)
                {
                    fprintf(stderr, "P2: Conflicting model list alias\n");
                    abort();
                }
                found = 1;
                break;
            }
        }
        if (found != 0)
            continue;
        if (record_count >= capacity)
        {
            fprintf(stderr, "P2: Model list layout exceeds capacity\n");
            abort();
        }
        records[record_count].source_offset = offset - 4;
        records[record_count].source_bytes = list_bytes;
        records[record_count].storage_offset = 0;
        records[record_count].storage_bytes = list_bytes;
        records[record_count].kind = ModelStorageWordList;
        ++record_count;
    }
    for (uint32 index = 1; index < record_count; ++index)
    {
        ModelStorageRecord record = records[index];
        uint32 insert = index;
        while (insert != 0 && records[insert - 1].source_offset > record.source_offset)
        {
            records[insert] = records[insert - 1];
            --insert;
        }
        records[insert] = record;
    }
    uint32 source_cursor = 0;
    uint32 record_index = 0;
    while (record_index < record_count)
    {
        if (records[record_index].source_offset < source_cursor)
        {
            fprintf(stderr, "P2: Model records overlap\n");
            abort();
        }
        if (records[record_index].source_offset > source_cursor)
        {
            if (record_count >= capacity)
            {
                fprintf(stderr, "P2: Model byte layout exceeds capacity\n");
                abort();
            }
            uint32 gap = records[record_index].source_offset - source_cursor;
            memmove(records + record_index + 1, records + record_index, (record_count - record_index) * sizeof(ModelStorageRecord));
            records[record_index].source_offset = source_cursor;
            records[record_index].source_bytes = gap;
            records[record_index].storage_offset = 0;
            records[record_index].storage_bytes = gap;
            records[record_index].kind = ModelStorageBytes;
            ++record_count;
        }
        source_cursor = records[record_index].source_offset + records[record_index].source_bytes;
        ++record_index;
    }
    if (source_cursor < clone_bytes)
    {
        if (record_count >= capacity)
        {
            fprintf(stderr, "P2: Model trailing byte layout exceeds capacity\n");
            abort();
        }
        records[record_count].source_offset = source_cursor;
        records[record_count].source_bytes = clone_bytes - source_cursor;
        records[record_count].storage_offset = 0;
        records[record_count].storage_bytes = clone_bytes - source_cursor;
        records[record_count].kind = ModelStorageBytes;
        ++record_count;
    }
    uint32 storage_bytes = 0;
    for (uint32 index = 0; index < record_count; ++index)
    {
        if (storage_bytes > 0xFFFFFFFCu)
        {
            fprintf(stderr, "P2: Model storage alignment exceeds native range\n");
            abort();
        }
        storage_bytes = (storage_bytes + 3u) & ~3u;
        records[index].storage_offset = storage_bytes;
        if (records[index].storage_bytes > 0xFFFFFFFFu - storage_bytes)
        {
            fprintf(stderr, "P2: Model storage exceeds native range\n");
            abort();
        }
        storage_bytes += records[index].storage_bytes;
    }
    if (storage_bytes > 0xFFFFFFFCu)
    {
        fprintf(stderr, "P2: Model storage alignment exceeds native range\n");
        abort();
    }
    storage_bytes = (storage_bytes + 3u) & ~3u;
    uint32 root_flags = xport_load_le32(input + 28);
    uint32 packed_counts = xport_load_le32(input + 32);
    uint32 face_offset = xport_load_le32(input + 36);
    uint32 vertex_offset = xport_load_le32(input + 40);
    uint32 legacy_bytes = (((root_flags & 12) == 8 && face_offset != 0) || ((sint16)(packed_counts >> 16) <= 0 && vertex_offset != 0)) ? bytes : 0;
    if (legacy_bytes > 0xFFFFFFFFu - storage_bytes)
    {
        fprintf(stderr, "P2: Legacy model payload exceeds native range\n");
        abort();
    }
    uint8 *storage = (uint8 *)calloc(storage_bytes + legacy_bytes, 1);
    if (storage == 0)
    {
        fprintf(stderr, "P2: Cannot allocate native model storage\n");
        abort();
    }
    for (uint32 index = 0; index < record_count; ++index)
    {
        const ModelStorageRecord *record = records + index;
        if (record->kind == ModelStorageWordList)
        {
            uint32 *words = (uint32 *)(storage + record->storage_offset);
            for (uint32 word = 0; word < record->source_bytes / sizeof(uint32); ++word)
                words[word] = xport_load_le32(input + record->source_offset + word * sizeof(uint32));
        }
        else if (record->kind == ModelStorageBytes)
            memcpy(storage + record->storage_offset, input + record->source_offset, record->source_bytes);
    }
    if (legacy_bytes != 0)
        memcpy(storage + storage_bytes, input, bytes);
    ModelDecodeLayout layout = {0};
    layout.input = input;
    layout.input_bytes = bytes;
    layout.source_clone_bytes = clone_bytes;
    layout.storage = storage;
    layout.storage_bytes = storage_bytes;
    layout.legacy_data = legacy_bytes != 0 ? storage + storage_bytes : 0;
    layout.records = records;
    layout.record_count = record_count;
    AnimatedObject *object = (AnimatedObject *)storage;
    object->RelocateResourceMaterials(resource, &layout, polygon_owner, vertex_owner);
    *object_slot = object;
    *model_owner = storage;
    free(input);
    free(pending);
    free(records);
    uint32 count = resource->material_count;
    if ((sint32)count > 0)
    {
        MaterialRecord *destination = material_registry.records + material_registry.count;
        for (uint32 index = 0; index < count; ++index)
            destination[index] = resource->materials[index];
        material_registry.count = material_registry.count + resource->material_count;
    }
}

uint32 DrawTexturedQuad(uint32 position, uint32 extent, const TextureDescriptor *descriptor, uint32 blend)
{
    uint32 packet;
    POLY_FT4 *poly;
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
    poly = (POLY_FT4 *)psx_addr(packet, sizeof(POLY_FT4));
    setPolyFT4(poly);
    poly->tag = ((packet + 40) & 0xFFFFFF) | 0x09000000;
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
    poly->rgbc.rgbc = color;
    xport_store_le32(&poly->p[0].uv, corner0);
    right = (position & 0xFFFF) + (extent & 0xFFFF);
    bottom = (position & 0xFFFF0000) + (extent & 0xFFFF0000);
    poly->p[1].xy.xy = right | (position & 0xFFFF0000);
    result = packet + 40;
    poly->p[0].xy.xy = position;
    xport_store_le32(&poly->p[1].uv, corner1);
    poly->p[2].xy.xy = (position & 0xFFFF) | bottom;
    xport_store_le32(&poly->p[2].uv, corner3);
    poly->p[3].xy.xy = right | bottom;
    xport_store_le32(&poly->p[3].uv, corner2);
    render_commands.overlay_tail = (packet);
    render_commands.packet_offset = (result);
    return result;
}

uint32 UploadTexturePalette(uint16 clut, uint32 page, const uint8 *palette)
{
    PSX_RECT rectangle;
    FUNCTION_MARKER(0x80066B78, "SLUS_005.78");
    rectangle.x = (sint16)((clut & 0x3F) << 4);
    rectangle.y = (sint16)(clut >> 6);
    rectangle.w = ((page >> 7) & 3) == 1 ? 256 : 16;
    rectangle.h = 1;
    return (uint32)LoadImagePSX(&rectangle, (uint32 *)palette);
}
