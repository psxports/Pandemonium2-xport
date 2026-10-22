#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "attack_enemy_state.h"
#include "game.h"

AttackEnemyState attack_enemy = {0};

void AttackEnemyState::LoadAttackScripts(const uint8 *bundle)
{
    FUNCTION_MARKER(0x80021538, "SLUS_005.78");

    struct ScriptFile

    {
        uint32 count;

        AttackScript scripts[30];
    };

    uint32 offset = resource_archive.Offset(resource_load_cursor.asset);
    const ScriptFile *input = (const ScriptFile *)(bundle + offset);
    attack_enemy.script_count = input->count;
    for (uint32 idx = 0; idx < 30; ++idx)
        attack_enemy.scripts[idx] = input->scripts[idx];
    uint32 result = resource_load_cursor.asset + 1;
    resource_load_cursor.asset = result;
}
