#ifndef ATTACK_SCRIPT_H
#define ATTACK_SCRIPT_H

#include "psx.h"

struct AttackScript
{
    sint32 state;
    uint32 metadata_word;
    uint32 animation_index;
    sint32 marker_frame;
    sint32 distance_squared;
    uint32 approach_ticks;
    sint32 repetitions;
};

struct AttackEnemyState
{
    AttackScript scripts[30];
    uint32 script_count;
    uint32 script_index;
    uint32 approach_ticks;
    uint32 repeat_count;
    uint32 special_pending;
    uint32 retreat_ticks;
    uint32 event_pending;
    uint32 target_x;
    uint32 next_path_frame;
    uint32 event_id;
    uint32 special_event_id;
};

extern AttackEnemyState attack_enemy;

extern "C"
{
    uint32 LoadAttackScripts(const uint8 *bundle);
}

#endif
