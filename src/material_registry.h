#ifndef MATERIAL_REGISTRY_H
#define MATERIAL_REGISTRY_H

#include "psx.h"

struct MaterialRecord;

struct MaterialRegistry
{
    unsigned int count;
    MaterialRecord *records;
    unsigned int reserved[6];
};

extern MaterialRegistry material_registry;
extern MaterialRegistry saved_material_registry;

extern "C" uint8 *ResetMaterialRegistry(void);

#endif
