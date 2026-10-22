#ifndef ATTACK_ENEMY_STATE_H
#define ATTACK_ENEMY_STATE_H
#include <stddef.h>

#include "psx.h"

struct AttackScript;
struct AttackEnemyState;

struct AttackScript
{
    sint32 state;
    uint32 metadata_word;
    uint32 animation_idx;
    sint32 marker_frame;
    sint32 distance_sq;
    uint32 approach_ticks;
    sint32 repetitions;
};

struct AttackEnemyState
{
    AttackScript scripts[30];
    uint32 script_count;
    uint32 script_idx;
    uint32 approach_ticks;
    uint32 repeat_count;
    uint32 special_pending;
    uint32 retreat_ticks;
    uint32 event_pending;
    uint32 target_x;
    uint32 next_path_frame;
    uint32 event_id;
    uint32 special_event_id;

    static void LoadAttackScripts(const uint8 *bundle);
};

extern AttackEnemyState attack_enemy;

#endif
