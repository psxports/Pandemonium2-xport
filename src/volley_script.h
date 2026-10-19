#ifndef VOLLEY_SCRIPT_H
#define VOLLEY_SCRIPT_H

#include "psx.h"

struct VolleyScript
{
    uint32 move_speed;
    sint32 distance_squared;
    uint32 mode;
    uint32 projectile_kind;
    sint32 frame_count;
    sint32 frames[10];
    sint32 angles[10];
    uint32 action_limit;
    uint32 shot_delay;
    sint32 turn_ticks;
    sint32 health_threshold;
};

struct VolleyEnemyState
{
    VolleyScript scripts[20];
    uint32 script_count;
    uint32 script_index;
    uint32 motion_ticks;
    uint32 approach_ticks;
    uint32 frame_index;
    uint32 delay_ticks;
    uint32 skip_update;
    uint32 reset_pending;
    uint32 path_active;
    uint32 event_started;
    uint32 turn_pending;
    uint32 target_x;
    uint32 target_path_frame;
    uint32 prev_path_frame;
    uint32 event_id;
};

extern VolleyEnemyState volley_enemy;

extern "C"
{
    uint32 LoadVolleyScripts(const uint8 *bundle);
}

#endif
