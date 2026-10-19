#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "game.h"

ResourceGroup resource_groups[35] = {{0}};
static MaterialResourceHeader interface_material_header = {0};
MaterialResourceHeader *interface_resources = 0;

void LoadResourceGroups(void)
{
    MaterialResourceHeader *resource;
    MaterialRecord *entry_rec;
    uint32 count;
    uint32 group;
    uint32 kind;
    MaterialRecord *entry;
    uint32 remaining;
    uint32 found;
    uint32 selected;
    uint32 values[4];
    FUNCTION_MARKER(0x8002A470, "SLUS_005.78");
    uint32 resource_bytes = resource_archive.Size(resource_load_cursor.asset);
    if (resource_bytes < sizeof(EncodedMaterialResourceHeader))
    {
        fprintf(stderr, "P2: Truncated interface material header\n");
        abort();
    }
    uint8 *encoded_header = (uint8 *)malloc(resource_bytes);
    if (encoded_header == 0)
    {
        fprintf(stderr, "P2: Cannot allocate interface material input\n");
        abort();
    }
    resource_archive.ReadAsset(resource_load_cursor.asset, encoded_header, 1, resource_load_cursor.container_asset);
    resource = &interface_material_header;
    if (xport_load_le32(encoded_header + 24) < 35)
    {
        fprintf(stderr, "P2: Interface texture catalog has fewer than35 entries\n");
        abort();
    }
    interface_resources = resource;
    RelocateMaterialResource(resource, encoded_header, resource_bytes, -1);
    free(encoded_header);
    resource = interface_resources;
    count = resource->material_count;
    resource_load_cursor.asset = resource_load_cursor.asset + 1;
    if (count != 0)
    {
        for (uint32 record_index = 0; record_index < count; ++record_index)
        {
            material_registry.records[material_registry.count + record_index] = resource->materials[record_index];
        }
        material_registry.count = material_registry.count + interface_resources->material_count;
    }
    for (group = 0; group < 35; ++group)
    {
        selected = 0;
        resource = interface_resources;
        resource_groups[group].texture = resource->textures + group;
        for (kind = 0; kind < 3; ++kind)
        {
            entry = resource->materials;
            remaining = resource->material_count;
            found = 0xFFFFFFFF;
            while (remaining != 0)
            {
                entry_rec = entry;
                if (entry_rec->texture == resource->textures + group && entry_rec->kind == kind)
                {
                    found = (uint32)(entry - resource->materials) << 1;
                    if (kind != 0)
                        found |= (entry_rec->flags & 15) << 26;
                    break;
                }
                --remaining;
                ++entry;
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
            CopyBytes((uint8 *)resource_groups[group].animation_entries, (const uint8 *)values, 4 * selected);
        }
    }
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
    free(interface_material_header.textures);
    free(interface_material_header.materials);
    free(interface_material_header.data);
    memset(&interface_material_header, 0, sizeof(interface_material_header));
    interface_resources = 0;
}

void UnloadResourceGroups(void)
{
    FUNCTION_MARKER(0x80060F6C, "SLUS_005.78");
    /* 80026B64 and 8002F180 return without effects */
    ReleaseResourceGroups();
}

void LoadCommonGameResources(void)
{
    uint32 offset;
    FUNCTION_MARKER(0x80060DF4, "SLUS_005.78");
    resource_load_cursor.asset = (game_flow.mode_flags & 0x200) ? 48 : 151;
    LoadResourceGroups();
    RegisterAnimatedModelOperations();
    LoadCommonModelGroups();
    InitializeActorModels();
    sound_resources.common_table = 0;
    if ((sint32)resource_archive.Offset(resource_load_cursor.asset) > 0)
    {
        offset = resource_archive.Offset(resource_load_cursor.asset);
        uint32 bytes = resource_archive.Size(resource_load_cursor.asset);
        if (bytes < sizeof(SoundTableHeader))
        {
            fprintf(stderr, "P2: Truncated common sound table\n");
            abort();
        }
        uint8 *sound_input = (uint8 *)malloc(bytes);
        if (sound_input == 0)
        {
            fprintf(stderr, "P2: Cannot allocate native sound table input\n");
            abort();
        }
        resource_archive.ReadAsset(resource_load_cursor.asset, sound_input, 1, resource_load_cursor.container_asset);
        const uint8 *source = sound_input;
        uint32 count = xport_load_le32(source);
        if ((sint32)count > 0 && (count > SoundBankStorage::SampleCapacity || count > (bytes - sizeof(SoundTableHeader)) / sizeof(SoundSample)))
        {
            fprintf(stderr, "P2: Common sound samples exceed table capacity\n");
            abort();
        }
        sound_resources.common_bank.header.count = count;
        if ((sint32)count > 0)
        {
            for (uint32 index = 0; index < count; ++index)
            {
                const uint8 *record = source + sizeof(SoundTableHeader) + index * sizeof(SoundSample);
                SoundSample *sample = sound_resources.common_bank.samples + index;
                sample->spu_address = xport_load_le32(record);
                sample->pitch = xport_load_le16(record + 4);
                sample->metadata = xport_load_le16(record + 6);
            }
        }
        sound_resources.common_table = sound_resources.common_bank.header.Relocate(bytes, sound_resources.common_bank.samples) != 0 ? &sound_resources.common_bank : 0;
        free(sound_input);
    }
    resource_load_cursor.asset = resource_load_cursor.asset + (1);
    InitializeCharacterModels();
    RegisterActorVelocityOperations();
}
