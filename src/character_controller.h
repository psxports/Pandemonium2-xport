#ifndef CHARACTER_CONTROLLER_H
#define CHARACTER_CONTROLLER_H
#include <stddef.h>

#include "psx.h"
#include "vector3.h"

struct AnimatedObject;

struct CharacterSettings
{
    enum Counter
    {
        CNT_SCORE,
        CNT_HEALTH,
        CNT_LIVES,
        CNT_MAX
    };

    uint8 flags;
    uint8 health_limit;
    uint32 counters[CNT_MAX];
    uint8 action_code;
    uint8 health_pickups;
};
struct ScenePath;
struct SceneSection;

struct ActorBinding;
struct AnimatedObject;
struct CharacterModelRecord;
struct SceneNode;
struct ScenePath;
struct SceneTrigger;

struct CharacterVitals;
struct PlayerInteractionState;
struct CharacterTimers;
struct CharacterMotion;
struct MovementProfile;
struct SupportContact;
struct CharacterSupport;
struct CharacterPathTransition;
struct CharacterPathInput;
struct CharacterSurfaceInteraction;
struct CharacterTriggerContacts;
struct CharacterAppearance;
struct CharacterController;

struct CharacterVitals
{
    uint8 lives;
    uint8 health_limit;
    uint8 health;
    uint32 score;
    uint16 damage_ticks;
    uint8 health_pickups;
    uint8 damage_amount;
};

struct PlayerInteractionState
{
    uint32 flags;
    ActorBinding *target;
    VECTOR pos;
};

extern PlayerInteractionState player_interaction;

struct CharacterTimers
{
    uint16 state_ticks;
    uint16 action_cooldown_ticks;
    uint16 score_bonus_ticks;
    uint16 transition_ticks;
    uint16 special_move_ticks;
    uint16 idle_ticks;
    uint16 model_swap_ticks;
    uint16 interaction_ticks;
    uint16 color_effect_ticks;
    sint16 jump_input_ticks;
    uint16 contact_impulse_ticks;
    sint16 jump_duration_ticks;
    sint32 action_launch_ticks;
};

struct CharacterMotion
{
    uint32 form_flags;
    sint8 state_code;
    sint8 prev_state_code;
    sint32 horizontal_speed;
    sint32 vertical_speed;
    sint32 jump_phase;
    uint32 slope_scale;
    uint32 environment_acceleration;
    sint32 buffered_horizontal_speed;
    sint32 buffered_vertical_speed;
    sint32 buffered_rise_limit;
    uint16 pos_changed;
    sint8 trail_effect;
};

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

struct SupportContact
{
    uint16 face_idx;
    uint16 transform_idx;
};

struct CharacterSupport
{
    uint32 speed[2];
    SupportContact face_ref;
    SceneNode *owner;
    SceneTrigger *candidate;
    uint32 fraction;
    Vec3 motion;
    SceneNode *contact;
    SceneTrigger *impulse_surface;
    uint16 contact_dir;
    uint32 impulse_scale;
    uint32 release_pending;
};

struct CharacterPathTransition
{
    const ScenePath *anchor_path;
    SceneTrigger *anchor;
    Vec3 start;
    Vec3 end;
    Vec3 camera_samples[4];
    const ScenePath *path;
    uint8 camera_ticks;
    sint8 phase;
    uint32 movement_ticks;
    sint8 contact_delay;
    uint32 approach_distance;
    uint8 release_pending;
    sint8 hold_ticks;
    uint32 phase_ticks;
    uint8 reentry_ticks;
    uint16 heading;
    Vec3 probe_pos;
    Vec3 correction;
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
    SceneNode *sel;
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

    uint32 flags;
    uint32 secondary_flags;
    uint32 movement_flags;
    sint32 section_height_offset;
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

    union
    {
        VECTOR path_tangent;
        Vec3 path_tangent_coord;
    };

    VECTOR path_velocity;
    MovementProfile movement;
    uint16 turn_step;
    CharacterModelRecord *model_slots[ModelCount];
    // Element 1 also holds the special-move recovery countdown
    uint32 special_move_backup[4];
    CharacterModelRecord *model;
    uint32 model_rebind_pending;
    AnimatedObject *object;
    const ScenePath *path;
    uint32 loaded_model_mask;
    SceneNode *trigger_objects[12];
    sint8 interaction_phase;
    uint16 action_heading;
    sint8 action_disp_phase;
    sint8 action_wave_phase;
    sint8 action_delay;
    sint8 phase_ticks;
    sint8 action_ticks;
    sint8 action_mode;
    sint8 action_latched;
    sint8 action_limit_step;
    sint8 action_delay_bias;
    sint8 action_pending;
    sint8 action_completion;
    sint8 action13_count;
    sint8 action14_count;
    uint8 action13_limit_reached;
    uint8 action14_limit_reached;
    uint32 action13_endpoint;
    uint32 action14_endpoint;
    uint32 action_shared_limit;
    uint32 action13_scale;
    uint32 action14_scale;
    SceneNode *pickup_objects[12];
    uint32 trigger_fractions[12];
    uint32 pickup_fractions[12];
    sint8 trigger_count;
    sint8 pickup_count;
    AnimatedObject *action_effects[15];
    sint8 sound_voice;
    uint32 model_load_reset;

    void ClearTransientFlags();
    static void DisableCharacterSupportFace(void);
    static void RestoreCharacterSupportFace(void);
    void RemoveAction(uint32 action);
    static void RestoreCharacterHealthPoint(void);
    static uint32 GetCurCharacterForm(void);
    static uint32 ReadCharacterMovementFlags(void);
    static void ApplyCharacterSettings(const AnimatedObject *object, const CharacterSettings *settings, uint8 score);
    static void CaptureCharacterSettings(CharacterSettings *settings);
    static AnimatedObject *FindPlayerObject(uint32 idx);
    static void PlaceCharacterOnPath(const SceneSection *scene, sint16 path_id, sint16 frame_word);
    static uint32 GetSceneHealthPickupFlag(uint32 scene);
    static uint32 SelectCharacterForm(uint32 mode);
    static void ResetNewGameCharacter(void);
    static uint32 CanContinuePathStep(void);
    static uint32 IsCharacterAnchorPathActive(void);
    static const ScenePath *ReadCharacterConstrainedPath(void);
    uint32 CancelCountedActions(uint32 incoming_flags);
    void ClearInteractionActions();
    void EndInteraction();
};

extern CharacterController character_pad;

extern uint32 character_action_flags;

extern uint32 character_action_target_score;

extern uint32 character_sound_resume_flags;

#endif
