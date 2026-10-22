#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "object_speed_definitions.h"
#include "game.h"

ObjectSpeedDefinitions object_speed_definitions = {0};

const ObjectSpeedDefinition *ObjectSpeedDefinitions::FindObjectSpeedDefinition(uint32 desc)
{
    FUNCTION_MARKER(0x8003FB58, "SLUS_005.78");
    sint32 count = object_speed_definitions.count;
    sint32 idx;
    for (idx = 0; idx < count; ++idx)
    {
        const ObjectSpeedDefinition *entry = &object_speed_definitions.recs[idx];
        if (entry->kind == desc)
            return entry;
    }
    return 0;
}

void ObjectSpeedDefinitions::LoadObjectSpeedDefinitions(const uint8 *bundle)
{
    uint32 asset = 4;
    uint32 result;
    uint32 offset;
    uint32 idx;
    uint32 next;
    const uint8 *bytes;
    ObjectSpeedDefinition *definition;
    FUNCTION_MARKER(0x8003FD28, "SLUS_005.78");
    object_speed_definitions.count = 0;
    do
    {
        result = resource_archive.Size(asset);
        if (result != 72)
            break;
        ++asset;
        offset = resource_archive.Offset(resource_load_cursor.asset);
        idx = object_speed_definitions.count;
        next = resource_load_cursor.asset + 1;
        object_speed_definitions.count = idx + 1;
        bytes = bundle + offset;
        definition = &object_speed_definitions.recs[idx];
        memcpy(definition, bytes, sizeof(*definition));
        resource_load_cursor.asset = next;

    } while (asset < 150);
}
