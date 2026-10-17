#include "game.h"
#include <string.h>

ResourceGroup resource_groups[35] = {{0}};
MaterialResourceHeader *interface_resources = 0;

uint32 LoadResourceGroups(void)
{
    MaterialResourceHeader *resource;
    MaterialRecord *entry_rec;
    uint32 offset;
    uint32 header;
    uint32 count;
    uint32 group;
    uint32 kind;
    uint32 entry;
    uint32 remaining;
    uint32 found;
    uint32 selected;
    uint32 values[4];
    uint32 result;
    FUNCTION_MARKER(0x8002A470, "SLUS_005.78");
    offset = resource_archive.Offset(r_u32(0x8008A46C));
    header = r_u32(0x8008A488) + offset;
    resource = (MaterialResourceHeader *)psx_addr(header, sizeof(MaterialResourceHeader));
    interface_resources = resource;
    RelocateMaterialResource(header, -1);
    resource = interface_resources;
    count = resource->material_count;
    w_u32(0x8008A46C, r_u32(0x8008A46C) + 1);
    if (count != 0)
    {
        p2_func_8006637c((uint8 *)(material_registry.records + material_registry.count), (const uint8 *)psx_addr(resource->materials_offset, 16 * count), 16 * count);
        material_registry.count = material_registry.count + interface_resources->material_count;
    }
    for (group = 0; group < 35; ++group)
    {
        selected = 0;
        resource = interface_resources;
        resource_groups[group].material_offset = resource->textures_offset + 20 * group;
        for (kind = 0; kind < 3; ++kind)
        {
            entry = resource->materials_offset;
            remaining = resource->material_count;
            for (;;)
            {
                entry_rec = (MaterialRecord *)psx_addr(entry, sizeof(MaterialRecord));
                if (entry_rec->texture_offset == resource_groups[group].material_offset && entry_rec->kind == kind)
                {
                    found = (uint32)((sint32)(entry - resource->materials_offset) >> 4) << 1;
                    if (kind != 0)
                        found |= (entry_rec->flags & 15) << 26;
                    break;
                }
                --remaining;
                entry += 16;
                if (remaining == 0)
                {
                    found = 0xFFFFFFFF;
                    break;
                }
            }
            values[selected] = found;
            if (found != 0xFFFFFFFF)
                ++selected;
        }
        if (selected != 0)
        {
            count = selected + 1;
            memset(resource_groups[group].animation_words, 255, 4 * count);
            resource_groups[group].animation_words[0] = count - 1;
            resource_groups[group].animation_entries = resource_groups[group].animation_words + 1;
            p2_func_8006637c((uint8 *)resource_groups[group].animation_entries, (const uint8 *)values, 4 * selected);
        }
    }
    result = resource_groups[25].material_offset;
    return result;
}

uint32 ClearResourceGroupLists(void)
{
    FUNCTION_MARKER(0x8002A6D8, "SLUS_005.78");
    uint32 index;
    for (index = 0; index < 35; ++index)
    {
        uint32 *data = resource_groups[index].animation_entries;
        if (data != 0)
        {
            resource_groups[index].animation_entries = 0;
        }
    }
    return 0;
}

static void ReleaseResourceGroups(void)
{
    FUNCTION_MARKER(0x8002A794, "SLUS_005.78");
    uint32 index;
    for (index = 0; index < 35; ++index)
    {
        uint32 *data = resource_groups[index].animation_entries;
        if (data != 0)
        {
            resource_groups[index].animation_entries = 0;
        }
    }
    interface_resources = 0;
}

void UnloadResourceGroups(void)
{
    FUNCTION_MARKER(0x80060F6C, "SLUS_005.78");
    /* 80026B64 and 8002F180 return without effects */
    ReleaseResourceGroups();
}
