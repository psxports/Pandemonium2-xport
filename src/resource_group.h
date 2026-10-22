#ifndef RESOURCE_GROUP_H
#define RESOURCE_GROUP_H
#include <stddef.h>

#include "psx.h"
#include "material_animation_list.h"

struct TextureDescriptor;

struct ResourceGroup;

struct ResourceGroup
{
    TextureDescriptor *tex;
    uint32 *animation_entries;
    MaterialAnimationList animation;

    static void LoadResourceGrps(void);
    static uint32 ClearResourceGrpLists(void);
    static void ReleaseResourceGrps(void);
    static void LoadCommonGameResources(void);
    static void UnloadResourceGrps(void);
};

extern ResourceGroup resource_grps[35];

#endif
