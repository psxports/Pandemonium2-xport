#ifndef CHARACTER_CONTROLLER_H
#define CHARACTER_CONTROLLER_H

#include "character_vitals.h"
#include "character_timers.h"
#include "character_motion.h"

struct MovementProfile
{
    sint32 jump_speed;
    sint32 rise_acceleration;
    sint32 fall_acceleration;
    sint32 rise_speed_limit;
    sint32 fall_speed_limit;
    sint32 speed_limits[4];
    sint32 braking0;
    sint32 acceleration0;
    sint32 braking1;
    sint32 acceleration1;
    uint32 jump_ticks;
    sint32 turn_frames;
};

// The original profile copy consumes fifteen contiguous words
typedef char MovementProfileWordLayout[(sizeof(MovementProfile) == 60) ? 1 : -1];

struct CharacterController
{
    unsigned int flags;
    unsigned int secondary_flags;
    uint32 weapon_state;
    CharacterVitals vitals;
    CharacterTimers timers;
    CharacterMotion motion;
    VECTOR path_tangent;
    VECTOR path_velocity;
    MovementProfile movement;
    unsigned short turn_step;
    // The last four slots also hold the special-move speed backup
    uint32 model_slots[19];
    uint32 model_offset;
    unsigned int trigger_objects[12];
    signed char interaction_phase;
    signed char action_delay;
    signed char phase_ticks;
    signed char action_ticks;
    signed char action_mode;
    signed char action_latched;
    signed char action_limit_step;
    signed char action_delay_bias;
    signed char action_pending;
    signed char action_completion;
    signed char action13_count;
    signed char action14_count;
    unsigned char action13_limit_reached;
    unsigned char action14_limit_reached;
    unsigned int action13_endpoint;
    unsigned int action14_endpoint;
    unsigned int action_shared_limit;
    unsigned int action13_scale;
    unsigned int action14_scale;
    unsigned int pickup_objects[12];
    unsigned int trigger_fractions[12];
    unsigned int pickup_fractions[12];
    signed char trigger_count;
    signed char pickup_count;
    unsigned int action_effects[15];
    signed char sound_voice;

    unsigned int ClearTransientFlags();
    unsigned int EndInteraction();
    void RemoveAction(unsigned int action);
    unsigned int CancelCountedActions(unsigned int incoming_flags);
    unsigned int ClearInteractionActions();
};

extern CharacterController character_controller;

extern unsigned int character_action_flags;
extern unsigned int action_effect_templates[15];
extern unsigned int character_action_owner_offset;
extern unsigned int character_action_target_offset;
extern unsigned int character_sound_resume_flags;
extern unsigned int character_prev_contacts;
extern unsigned int character_cur_contacts;
extern unsigned int contact_free_head;



#endif
