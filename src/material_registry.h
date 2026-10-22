#ifndef MATERIAL_REGISTRY_H
#define MATERIAL_REGISTRY_H
#include <stddef.h>

#include "psx.h"

struct MaterialRecord;

struct MaterialRegistry;

struct MaterialRegistry
{
    uint32 count;
    MaterialRecord *recs;
    uint32 unload_result;

    static void UpdateResourceMaterialAnimations(void);
    static uint8 *ResetMaterialRegistry(void);
};

extern MaterialRegistry material_registry;

extern MaterialRegistry saved_material_registry;

#endif
