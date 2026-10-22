#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "volley_enemy_state.h"
#include "game.h"
#include "animated_object.h"
#include "psx_gte.h"

VolleyEnemyState volley_enemy = {0};

void VolleyEnemyState::ResetVolleyScript(void)
{
    FUNCTION_MARKER(0x8001FA08, "SLUS_005.78");
    volley_enemy.motion_ticks = 0;
    volley_enemy.approach_ticks = 0;
    volley_enemy.frame_idx = 0;
    volley_enemy.reset_pending = 1;
}

void VolleyEnemyState::LoadVolleyScripts(const uint8 *bundle)
{
    FUNCTION_MARKER(0x80020AE8, "SLUS_005.78");

    struct ScriptFile

    {
        uint32 count;

        VolleyScript scripts[20];
    };

    uint32 offset = resource_archive.Offset(resource_load_cursor.asset);
    const ScriptFile *input = (const ScriptFile *)(bundle + offset);
    volley_enemy.script_count = input->count;
    for (uint32 idx = 0; idx < 20; ++idx)
        volley_enemy.scripts[idx] = input->scripts[idx];
    uint32 result = resource_load_cursor.asset + 1;
    resource_load_cursor.asset = result;
}
