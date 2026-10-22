#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "material_registry.h"
#include "game.h"

// Packed material references contain a nine-bit record index
static MaterialRecord material_recs[512];

MaterialRegistry material_registry = {0, 0};

MaterialRegistry saved_material_registry = {0, 0};

void MaterialRegistry::UpdateResourceMaterialAnimations(void)
{
    uint32 idx;
    uint32 saved_frame;

    FUNCTION_MARKER(0x80027228, "SLUS_005.78");
    for (idx = 0; idx < 28; ++idx)
    {
        if (idx == 1 || idx == 5 || ((game_flow.transition_flags & 0x10) == 0 && idx >= 13))
        {
            saved_frame = frame_clock.render_tick;
            if (idx == 1 || idx == 5)
                frame_clock.render_tick = frame_clock.menu_tick;
            MaterialAnimationList::UpdateMaterialAnimationList(resource_grps[idx].animation_entries);
            frame_clock.render_tick = saved_frame;
        }
    }
    frame_clock.menu_tick = frame_clock.menu_tick + 1;
}

uint8 *MaterialRegistry::ResetMaterialRegistry(void)
{
    uint8 *result;
    FUNCTION_MARKER(0x8002F144, "SLUS_005.78");
    memset(&material_registry, 0, sizeof(material_registry));
    result = resource_memory.arena;
    material_registry.recs = material_recs;
    return result;
}
