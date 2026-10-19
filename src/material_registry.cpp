#include <string.h>

#include "game.h"

// Packed material references contain a nine-bit record index
static MaterialRecord material_records[512];

MaterialRegistry material_registry = {0, 0, {0}};
MaterialRegistry saved_material_registry = {0, 0, {0}};

uint8 *ResetMaterialRegistry(void)
{
    uint8 *result;
    FUNCTION_MARKER(0x8002F144, "SLUS_005.78");
    memset(&material_registry, 0, sizeof(material_registry));
    result = resource_memory.arena;
    material_registry.records = material_records;
    return result;
}

void NoOp8002F180(void)
{
    FUNCTION_MARKER(0x8002F180, "SLUS_005.78");
    /* Original function returns without side effects */
}
