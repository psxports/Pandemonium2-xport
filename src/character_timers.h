#ifndef CHARACTER_TIMERS_H
#define CHARACTER_TIMERS_H

struct CharacterTimers
{
    unsigned short state_ticks;
    unsigned short action_cooldown_ticks;
    unsigned short score_bonus_ticks;
    unsigned short transition_ticks;
    unsigned short special_move_ticks;
    unsigned short idle_ticks;
    unsigned short model_swap_ticks;
    unsigned short interaction_ticks;
    unsigned short color_effect_ticks;
    unsigned short jump_input_ticks;
    unsigned short contact_impulse_ticks;
    unsigned short jump_duration_ticks;
};


#endif
