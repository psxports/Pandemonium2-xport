#ifndef CHARACTER_MOTION_H
#define CHARACTER_MOTION_H

#include "psx.h"

struct CharacterMotion
{
    unsigned int form_flags;
    unsigned char state_code;
    unsigned char prev_state_code;
    sint32 horizontal_speed;
    sint32 vertical_speed;
    sint32 jump_phase;
    uint32 slope_scale;
    uint32 environment_acceleration;
    uint32 buffered_horizontal_speed;
    uint32 buffered_vertical_speed;
    uint32 buffered_rise_limit;
    uint16 position_changed;
    sint8 trail_effect;
};

#endif
