#include "game.h"
#include <string.h>

// Packed material references contain a nine-bit record index
static MaterialRecord material_records[512];

MaterialRegistry material_registry = {0, 0, {0}};
MaterialRegistry saved_material_registry = {0, 0, {0}};

uint32 ResetMaterialRegistry(void)
{
    uint32 result;
    FUNCTION_MARKER(0x8002F144, "SLUS_005.78");
    memset(&material_registry, 0, sizeof(material_registry));
    result = r_u32(0x8008A284);
    material_registry.records = material_records;
    return result;
}
