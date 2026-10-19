#ifndef RESOURCE_GROUPS_H
#define RESOURCE_GROUPS_H

#include "psx.h"

struct MaterialResourceHeader;
struct TextureDescriptor;

struct ResourceGroup
{
    TextureDescriptor *texture;
    unsigned int *animation_entries;
    unsigned int animation_words[4];
};

extern ResourceGroup resource_groups[35];
extern MaterialResourceHeader *interface_resources;

extern "C"
{
    void LoadResourceGroups(void);
    void LoadCommonGameResources(void);
    uint32 ClearResourceGroupLists(void);
    void UnloadResourceGroups(void);
}

#endif
