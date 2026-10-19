#ifndef CHARACTER_CONTROLLER_H
#define CHARACTER_CONTROLLER_H

#include "character_vitals.h"
#include "character_timers.h"
#include "character_motion.h"
#include "material_resources.h"

struct CharacterModelRecord;
struct AnimatedObject;
struct SceneNode;
struct MeshFace;
struct MeshVertex;
struct SceneTrigger;
struct ScenePath;

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

struct CharacterSupport
{
    uint32 speed[2];
    // The low half selects the face; the high half selects its transform
    uint32 contact_word;
    SceneNode *owner;
    SceneTrigger *candidate;
    uint32 fraction;
    uint32 motion[3];
    SceneNode *contact;
    SceneTrigger *impulse_surface;
    uint16 contact_direction;
    uint32 impulse_scale;
    uint32 release_pending;
};

struct CharacterPathTransition
{
    const ScenePath *anchor_path;
    SceneTrigger *anchor;
    uint32 start[3];
    uint32 end[3];
    uint32 camera_samples[4][3];
    const ScenePath *path;
    uint8 camera_ticks;
    uint8 phase;
    uint32 movement_ticks;
    uint8 contact_delay;
    uint32 approach_distance;
    uint8 release_pending;
    uint8 hold_ticks;
    uint32 phase_ticks;
    uint8 reentry_ticks;
    uint16 heading;
    uint32 probe_position[3];
    uint32 correction[3];
    uint8 correction_mode;
    const ScenePath *constrained_path;
    uint16 tangent_heading;
    uint16 motion_heading;
    sint8 hold_motion_ticks;
    sint8 input_ready;
    uint32 entry_flags;
    uint32 anchor_fraction;
};

struct CharacterPathInput
{
    uint32 latch_phase;
    uint32 release_ticks;
    uint32 latched_motion;
    uint32 prev_motion;
    sint8 dir;
    sint8 settle_ticks;
    sint8 routing_phase;
};

struct CharacterSurfaceInteraction
{
    SceneNode *selected;
    SceneTrigger *environment;
    SceneTrigger *jump_scale;
    SceneTrigger *surface;
    SceneTrigger *prev_surface;
    uint8 surface_phase;
    uint8 surface_step;
    uint8 prev_surface_step;
    uint8 constrained_reset;
};

struct CharacterTriggerContacts
{
    SceneTrigger *section_trigger;
    SceneTrigger *hide_trigger;
    SceneTrigger *show_trigger;
    uint32 section_fraction;
    uint32 jump_scale_fraction;
    uint32 hide_fraction;
    uint32 show_fraction;
    uint32 damage_fraction;
};

struct CharacterAppearance
{
    uint32 tint;
    uint32 blink_ticks;
};

struct CharacterController
{
    enum
    {
        ModelCount = 15
    };

    unsigned int flags;
    unsigned int secondary_flags;
    uint32 movement_flags;
    uint32 section_height_offset;
    CharacterSurfaceInteraction surface_interaction;
    uint32 weapon_state;
    CharacterVitals vitals;
    CharacterTimers timers;
    CharacterMotion motion;
    CharacterSupport support;
    CharacterTriggerContacts contacts;
    CharacterAppearance appearance;
    CharacterPathTransition path_transition;
    CharacterPathInput path_input;
    VECTOR path_tangent;
    VECTOR path_velocity;
    MovementProfile movement;
    unsigned short turn_step;
    CharacterModelRecord *model_slots[ModelCount];
    // Element 1 also holds the special-move recovery countdown
    uint32 special_move_backup[4];
    CharacterModelRecord *model;
    uint32 model_rebind_pending;
    AnimatedObject *object;
    const ScenePath *path;
    uint32 loaded_model_mask;
    SceneNode *trigger_objects[12];
    signed char interaction_phase;
    uint16 action_heading;
    sint8 action_disp_phase;
    sint8 action_wave_phase;
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
    SceneNode *pickup_objects[12];
    unsigned int trigger_fractions[12];
    unsigned int pickup_fractions[12];
    signed char trigger_count;
    signed char pickup_count;
    AnimatedObject *action_effects[15];
    signed char sound_voice;
    uint32 model_load_reset;

    unsigned int ClearTransientFlags();
    unsigned int EndInteraction();
    void RemoveAction(unsigned int action);
    unsigned int CancelCountedActions(unsigned int incoming_flags);
    unsigned int ClearInteractionActions();
};

extern CharacterController character_controller;

uint32 ReadCharacterMovementFlags(void);

extern unsigned int character_action_flags;
struct AnimationHeader;

struct ActionEffectResources
{
    enum
    {
        Count = 15
    };

    AnimatedObject *templates[Count];
    uint8 *model_data[Count];
    uint32 model_bytes[Count];
    MeshFace *model_polygons[Count];
    MeshVertex *model_vertices[Count];
    MaterialResourceHeader material_resources[Count];
    uint8 *animation_data[Count];
    uint32 animation_bytes[Count];
    AnimationHeader *animation_headers[Count];
};

extern ActionEffectResources action_effect_resources;
extern AnimatedObject *character_action_owner;
extern AnimatedObject *character_action_target;
extern uint32 character_action_target_score;
extern unsigned int character_sound_resume_flags;

#endif
