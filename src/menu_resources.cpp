#include <string.h>

#include "game.h"

static uint32 FindMaterialEntry(uint32 group, uint32 kind, const MaterialResourceHeader *header, uint32 base)
{
    uint32 entry;
    const MaterialRecord *record;
    uint32 remaining;
    uint32 index;
    uint32 result;
    FUNCTION_MARKER(0x8002A34C, "SLUS_005.78");
    entry = header->materials_offset;
    remaining = header->material_count;
    for (;;)
    {
        record = (const MaterialRecord *)psx_addr(entry, sizeof(MaterialRecord));
        if (record->texture_offset == group && record->kind == kind)
            break;
        --remaining;
        entry += 16;
        if (remaining == 0)
            return 0xFFFFFFFF;
    }
    index = (uint32)((sint32)(entry - header->materials_offset) >> 4);
    result = (index + base) << 1;
    if (kind != 0)
        result |= (record->flags & 15) << 26;
    return result;
}

uint32 LoadMenuResources(void)
{
    uint32 offset;
    uint32 source;
    uint32 length;
    uint32 second_length;
    uint32 third_length;
    MaterialResourceHeader *header;
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
        source = r_u32(0x8008A4C8) + offset;
        length = resource_archive.Size(260);
        second_length = resource_archive.Size(261);
        third_length = resource_archive.Size(262);
        length += second_length + third_length;
        p2_func_8006637c((uint8 *)psx_addr(0x800AC680, length), (const uint8 *)psx_addr(source, length), length);
    }
    w_u16(0x8008A530, 0);
    if ((sint32)resource_archive.Offset(263) > 0)
    {
        do
        {
            offset = resource_archive.Offset(263);
            source = r_u32(0x8008A4C8);
            w_u16(0x8008A530, p2_func_8006ac44(257, 0, source, source + offset));
        } while (r_u16(0x8008A530) == 0);
        p2_func_80066404();
    }
    offset = resource_archive.Offset(264);
    source = r_u32(0x8008A4C8) + offset;
    header = (MaterialResourceHeader *)psx_addr(source, sizeof(MaterialResourceHeader));
    menu_state.resources = header;
    RelocateMaterialResource(source, -1);
    header = menu_state.resources;
    count = header->material_count;
    if (count != 0 && (sint32)(material_registry.count + count) < 51)
    {
        material_dst = material_registry.records + material_registry.count;
        source = header->materials_offset;
        p2_func_8006637c((uint8 *)material_dst, (const uint8 *)psx_addr(source, 16 * count), 16 * count);
        material_registry.count = material_registry.count + menu_state.resources->material_count;
        header = menu_state.resources;
    }
    index = 0;
    if ((sint32)header->texture_count > 0)
    {
        do
        {
            selected = 0;
            menu_state.material_groups[index].material_offset = header->textures_offset + 20 * index;
            for (kind = 0; kind < 3; ++kind)
            {
                header = menu_state.resources;
                values[selected] = FindMaterialEntry(menu_state.material_groups[index].material_offset, kind, header, material_registry.count - header->material_count);
                if (values[selected] != 0xFFFFFFFF)
                    ++selected;
            }
            if (selected != 0)
            {
                memset(menu_state.material_groups[index].animation_words, 255, 4 * (selected + 1));
                menu_state.material_groups[index].animation_words[0] = selected;
                menu_state.material_groups[index].animation_entries = menu_state.material_groups[index].animation_words + 1;
                p2_func_8006637c((uint8 *)menu_state.material_groups[index].animation_entries, (const uint8 *)values, 4 * selected);
            }
            header = menu_state.resources;
            ++index;
        } while ((sint32)index < (sint32)header->texture_count);
    }
    offset = resource_archive.Offset(265);
    return p2_func_8006624c(r_u32(0x8008A4C8) + offset, 9, 1);
}
