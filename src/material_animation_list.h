#ifndef MATERIAL_ANIMATION_LIST_H
#define MATERIAL_ANIMATION_LIST_H
#include <stddef.h>

#include "psx.h"

struct MaterialAnimationListHeader;
struct MaterialAnimationList;

struct MaterialAnimationListHeader
{
    uint32 count : 8;
    uint32 start_tick : 24;
};

struct MaterialAnimationList
{
    MaterialAnimationListHeader header;
    uint32 entries[3];

    static void ResetMaterialAnimationList(uint32 *entries);
    static sint32 UpdateMaterialAnimationList(uint32 *entries);
};

#endif
