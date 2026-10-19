#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "game.h"

MenuArchive menu_archive = {0};

static uint32 FindMaterialEntry(const TextureDescriptor *texture, uint32 kind, const MaterialResourceHeader *header, uint32 base)
{
    const MaterialRecord *entry;
    const MaterialRecord *record;
    uint32 remaining;
    uint32 index;
    uint32 result;
    FUNCTION_MARKER(0x8002A34C, "SLUS_005.78");
    entry = header->materials;
    remaining = header->material_count;
    for (;;)
    {
        record = entry;
        if (record->texture == texture && record->kind == kind)
            break;
        --remaining;
        ++entry;
        if (remaining == 0)
            return 0xFFFFFFFF;
    }
    index = (uint32)(entry - header->materials);
    result = (index + base) << 1;
    if (kind != 0)
        result |= (record->flags & 15) << 26;
    return result;
}

uint32 LoadMenuResources(void)
{
    uint32 offset;
    uint32 length;
    uint32 second_length;
    uint32 third_length;
    MaterialResourceHeader *header;
    const uint8 *encoded_header;
    uint32 material_bytes;
    uint32 count;
    uint32 index;
    uint32 kind;
    uint32 selected;
    MaterialRecord *material_dst;
    uint32 values[6];
    FUNCTION_MARKER(0x80066E98, "SLUS_005.78");
    if ((game_flow.mode_flags & 0x200) == 0 && (sint32)resource_archive.Size(260) > 0 && (sint32)resource_archive.Size(261) > 0 && (sint32)resource_archive.Size(262) > 0)
    {
        offset = resource_archive.Offset(260);
        length = resource_archive.Size(260);
        second_length = resource_archive.Size(261);
        third_length = resource_archive.Size(262);
        if (second_length > 0xFFFFFFFFu - length || third_length > 0xFFFFFFFFu - length - second_length)
        {
            fprintf(stderr, "P2: Invalid menu demo extent\n");
            abort();
        }
        length += second_length + third_length;
        if (menu_archive.data == NULL || offset > menu_archive.bytes || length > menu_archive.bytes - offset || length > sizeof(demo_background_workspace))
        {
            fprintf(stderr, "P2: Menu demo is outside archive\n");
            abort();
        }
        CopyBytes(demo_background_workspace, menu_archive.data + offset, length);
    }
    sequence_playback.current_handle = 0;
    if ((sint32)resource_archive.Offset(263) > 0)
    {
        do
        {
            uint32 bundle_bytes = menu_archive.bytes;
            offset = resource_archive.Offset(263);
            length = resource_archive.Size(263);
            if (menu_archive.data == NULL || offset > bundle_bytes || length > bundle_bytes - offset)
            {
                fprintf(stderr, "P2: Menu samples are outside archive\n");
                abort();
            }
            sequence_playback.current_handle = LoadSequence(257, 0, menu_archive.data, bundle_bytes, menu_archive.data + offset, length);
        } while (sequence_playback.current_handle == 0);
        StartCurrentSequence();
    }
    offset = resource_archive.Offset(264);
    material_bytes = resource_archive.Size(264);
    if (menu_archive.data == NULL || material_bytes < 32 || offset > menu_archive.bytes || material_bytes > menu_archive.bytes - offset)
    {
        fprintf(stderr, "P2: Menu material header is outside archive\n");
        abort();
    }
    encoded_header = menu_archive.data + offset;
    header = &menu_state.resource_view.header;
    if (xport_load_le32(encoded_header + 24) > 9)
    {
        fprintf(stderr, "P2: Menu texture catalog exceeds its capacity\n");
        abort();
    }
    RelocateMaterialResource(header, encoded_header, material_bytes, -1);
    menu_state.resource_view.textures = header->texture_count != 0 ? header->textures : NULL;
    menu_state.resources = &menu_state.resource_view;
    count = header->material_count;
    if (count != 0 && (sint32)(material_registry.count + count) < 51)
    {
        material_dst = material_registry.records + material_registry.count;
        for (uint32 record_index = 0; record_index < count; ++record_index)
        {
            material_dst[record_index] = header->materials[record_index];
        }
        material_registry.count = material_registry.count + header->material_count;
    }
    index = 0;
    if ((sint32)header->texture_count > 0)
    {
        do
        {
            selected = 0;
            menu_state.material_groups[index].texture = menu_state.resource_view.textures + index;
            for (kind = 0; kind < 3; ++kind)
            {
                values[selected] = FindMaterialEntry(menu_state.material_groups[index].texture, kind, header, material_registry.count - header->material_count);
                if (values[selected] != 0xFFFFFFFF)
                    ++selected;
            }
            if (selected != 0)
            {
                memset(menu_state.material_groups[index].animation_words, 255, 4 * (selected + 1));
                menu_state.material_groups[index].animation_words[0] = selected;
                menu_state.material_groups[index].animation_entries = menu_state.material_groups[index].animation_words + 1;
                CopyBytes((uint8 *)menu_state.material_groups[index].animation_entries, (const uint8 *)values, 4 * selected);
            }
            ++index;
        } while ((sint32)index < (sint32)header->texture_count);
    }
    offset = resource_archive.Offset(265);
    {
        uint32 image_index = 9;
        uint32 image_bytes = (uint32)OriginalDefaults::TextureImageRects[image_index].w * (uint32)OriginalDefaults::TextureImageRects[image_index].h * 2;
        if (image_index == 0)
        {
            uint32 second_image_bytes = resource_archive.Offset(252) + (uint32)OriginalDefaults::TextureImageRects[8].w * (uint32)OriginalDefaults::TextureImageRects[8].h * 2;
            if (image_bytes < second_image_bytes)
                image_bytes = second_image_bytes;
        }
        if (menu_archive.data == NULL || offset > menu_archive.bytes || image_bytes > menu_archive.bytes - offset)
        {
            fprintf(stderr, "P2: Menu atlas is outside archive\n");
            abort();
        }
        return UploadTextureImage(menu_archive.data + offset, image_index, 1);
    }
}

uint32 LoadMenuImage(uint32 asset)
{
    uint32 result;
    uint32 offset;
    uint32 image_bytes;
    PSX_RECT image_rect = OriginalDefaults::MenuImageRect;
    FUNCTION_MARKER(0x80066DE4, "SLUS_005.78");
    result = menu_state.image_asset;
    if (result != asset)
    {
        menu_state.image_asset = asset;
        DrawSync(0);
        VSync(0);
        offset = resource_archive.Offset(asset);
        image_bytes = (uint32)image_rect.w * (uint32)image_rect.h * 2;
        if (menu_archive.data == NULL || offset > menu_archive.bytes || image_bytes > menu_archive.bytes - offset)
        {
            fprintf(stderr, "P2: Menu image is outside archive\n");
            abort();
        }
        LoadImagePSX(&image_rect, (uint32 *)(menu_archive.data + offset));
        return (uint32)DrawSync(0);
    }
    return result;
}

void ReloadMenuArchive(void)
{
    FUNCTION_MARKER(0x80066E48, "SLUS_005.78");
    uint32 flags = game_flow.mode_flags;
    uint32 bytes = resource_archive.Size(254);
    uint32 capacity, offset, material_bytes;
    menu_state.image_asset = (uint32)-1;
    if (bytes == 0 || bytes > 0xFFFFF800u)
    {
        fprintf(stderr, "P2: Invalid menu archive extent\n");
        abort();
    }
    capacity = (bytes + 2047) & ~2047u;
    if (capacity > menu_archive.capacity)
    {
        uint8 *data = (uint8 *)realloc(menu_archive.data, capacity);
        if (data == NULL)
        {
            fprintf(stderr, "P2: Cannot allocate menu archive\n");
            abort();
        }
        menu_archive.data = data;
        menu_archive.capacity = capacity;
    }
    menu_archive.bytes = bytes;
    resource_archive.ReadAsset(254, menu_archive.data, (flags & 0x200) != 0);
    offset = resource_archive.Offset(264);
    material_bytes = resource_archive.Size(264);
    if (material_bytes < sizeof(EncodedMaterialResourceHeader) || offset > bytes || material_bytes > bytes - offset)
    {
        fprintf(stderr, "P2: Menu material child is outside archive\n");
        abort();
    }
}
