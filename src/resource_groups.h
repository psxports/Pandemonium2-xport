#ifndef RESOURCE_GROUPS_H
#define RESOURCE_GROUPS_H

#include "psx.h"

struct MaterialResourceHeader;

struct ResourceGroup
{
    unsigned int material_offset;
    unsigned int *animation_entries;
    unsigned int animation_words[4];
};

extern ResourceGroup resource_groups[35];
extern MaterialResourceHeader *interface_resources;

extern "C"
{
uint32 LoadResourceGroups(void);
uint32 ClearResourceGroupLists(void);
void UnloadResourceGroups(void);
}

#endif
