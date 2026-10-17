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
};


#endif
