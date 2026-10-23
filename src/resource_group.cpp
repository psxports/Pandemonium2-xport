#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "resource_group.h"
#include "game.h"
#include "hd.h"

ResourceGroup resource_grps[35] = {{0}};

static MaterialCatalog interface_material_header = {0};

void ResourceGroup::LoadResourceGrps(void)
{
    MaterialCatalog *resource;
    MaterialRecord *entry_rec;
    uint32 count;
    uint32 grp;
    uint32 kind;
    MaterialRecord *entry;
    uint32 remaining;
    uint32 found;
    uint32 sel;
    uint32 vals[4];
    FUNCTION_MARKER(0x8002A470, "SLUS_005.78");
    uint32 resource_bytes = resource_archive.Size(resource_load_cursor.asset);
    uint8 *encoded_header = (uint8 *)malloc(resource_bytes);
    resource_archive.ReadAsset(resource_load_cursor.asset, encoded_header, 1, resource_load_cursor.container_asset);
    resource = &interface_material_header;
    interface_resources = resource;
    MaterialCatalog::RelocateMaterialResource(resource, encoded_header, resource_bytes, -1);
    free(encoded_header);
    resource = interface_resources;
    count = resource->material_count;
    resource_load_cursor.asset = resource_load_cursor.asset + 1;
    if (count != 0)
    {
        for (uint32 rec_idx = 0; rec_idx < count; ++rec_idx)
        {
            material_registry.recs[material_registry.count + rec_idx] = resource->materials[rec_idx];
        }
        material_registry.count = material_registry.count + interface_resources->material_count;
    }
    for (grp = 0; grp < 35; ++grp)
    {
        sel = 0;
        resource = interface_resources;
        resource_grps[grp].tex = resource->texs + grp;
        for (kind = 0; kind < 3; ++kind)
        {
            entry = resource->materials;
            remaining = resource->material_count;
            found = 0xFFFFFFFF;
            while (remaining != 0)
            {
                entry_rec = entry;
                if (entry_rec->tex == resource->texs + grp && entry_rec->kind == kind)
                {
                    found = (entry - resource->materials) << 1;
                    if (kind != 0)
                        found |= (entry_rec->flags & 15) << 26;
                    break;
                }
                --remaining;
                ++entry;
            }
            vals[sel] = found;
            if (found != 0xFFFFFFFF)
                ++sel;
        }
        if (sel != 0)
        {
            count = sel + 1;
            memset(&resource_grps[grp].animation, 255, 4 * count);
            resource_grps[grp].animation.header.count = count - 1;
            resource_grps[grp].animation.header.start_tick = 0;
            resource_grps[grp].animation_entries = resource_grps[grp].animation.entries;
            CopyBytes((uint8 *)resource_grps[grp].animation_entries, (const uint8 *)vals, 4 * sel);
        }
    }
}

uint32 ResourceGroup::ClearResourceGrpLists(void)
{
    FUNCTION_MARKER(0x8002A6D8, "SLUS_005.78");
    uint32 idx;
    for (idx = 0; idx < 35; ++idx)
    {
        uint32 *data = resource_grps[idx].animation_entries;
        if (data != 0)
        {
            resource_grps[idx].animation_entries = 0;
        }
    }
    return 0;
}

void ResourceGroup::ReleaseResourceGrps(void)
{
    FUNCTION_MARKER(0x8002A794, "SLUS_005.78");
    hd_invalidate_geometry();
    uint32 idx;
    for (idx = 0; idx < 35; ++idx)
    {
        uint32 *data = resource_grps[idx].animation_entries;
        if (data != 0)
        {
            resource_grps[idx].animation_entries = 0;
        }
    }
    free(interface_material_header.texs);
    free(interface_material_header.materials);
    free(interface_material_header.data);
    memset(&interface_material_header, 0, sizeof(interface_material_header));
    interface_resources = 0;
}

void ResourceGroup::LoadCommonGameResources(void)
{
    FUNCTION_MARKER(0x80060DF4, "SLUS_005.78");
    resource_load_cursor.asset = (game_flow.mode_flags & 0x200) ? 48 : 151;
    ResourceGroup::LoadResourceGrps();
    AnimatedModelRegistry::RegisterAnimatedModelOperations();
    ModelGroupRegistry::LoadCommonModelGrps();
    ActorModelRegistry::InitializeActorModels();
    sound_resources.common_table = 0;
    if ((sint32)resource_archive.Offset(resource_load_cursor.asset) > 0)
    {
        resource_archive.Offset(resource_load_cursor.asset);
        uint32 bytes = resource_archive.Size(resource_load_cursor.asset);
        uint8 *sound_input = (uint8 *)malloc(bytes);
        resource_archive.ReadAsset(resource_load_cursor.asset, sound_input, 1, resource_load_cursor.container_asset);
        const uint8 *src = sound_input;
        uint32 count = ((const EncodedSoundHeader *)src)->count;
        sound_resources.common_bank.header.count = count;
        if ((sint32)count > 0)
        {
            for (uint32 idx = 0; idx < count; ++idx)
            {
                const uint8 *rec = src + sizeof(SoundTableHeader) + idx * sizeof(SoundSample);
                SoundSample *sample = sound_resources.common_bank.samples + idx;
                const EncodedSoundSample *input_sample = (const EncodedSoundSample *)rec;
                sample->spu_address = input_sample->spu_address;
                sample->pitch = input_sample->pitch;
                sample->metadata = input_sample->metadata;
            }
        }
        sound_resources.common_table = sound_resources.common_bank.header.Relocate(bytes, sound_resources.common_bank.samples) != 0 ? &sound_resources.common_bank : 0;
        free(sound_input);
    }
    resource_load_cursor.asset = resource_load_cursor.asset + 1;
    CharacterModelRecord::InitializeCharacterModels();
    AnimatedObject::RegisterActorVelocityOperations();
}

void ResourceGroup::UnloadResourceGrps(void)
{
    FUNCTION_MARKER(0x80060F6C, "SLUS_005.78");
    /* 80026B64 and 8002F180 return without effects */
    ResourceGroup::ReleaseResourceGrps();
}
