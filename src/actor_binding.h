#ifndef ACTOR_BINDING_H
#define ACTOR_BINDING_H
#include <stddef.h>

#include "psx.h"
#include "vector3.h"

struct ActorModelDefinition;
struct ActorPlacement;
struct AnimatedObject;
struct AnimationTrack;
struct SceneNode;
struct SoundSample;

struct EncodedActorBinding;
struct ActorBinding;

// Runtime actor state decoded from the section resource
// Packed section record; reserved bytes never become runtime state
struct EncodedActorBinding
{
    uint32 next_offset;
    uint32 instance_offset;
    uint32 event_command;
    uint32 distance_sq;
    sint32 motion_speed;
    sint16 path_parameters[2];
    sint16 path_frame;
    sint16 initial_path_frame;
    sint16 motion_frame;
    sint16 path_pitch_speed;
    uint8 reserved32[5];
    uint8 model_kind;
    uint8 flags;
    uint8 reserved39[1];
    uint16 respawn_ticks;
    uint8 reserved42[2];
    uint32 reaction_flags;
    uint16 respawn_delay;
    uint8 reserved50[2];
    uint32 event_offset;
    uint32 rotation_offset;
    uint32 placement_offset;
    uint16 sound_pitch;
    sint16 sound_voice;
    uint32 sound_offset;
    uint32 model_offset;
    sint32 interaction_timer;
    uint32 interaction_heading_ticks;
    sint32 interaction_height;
    uint16 path_heading;
    uint8 reserved90[2];
    Vec3 path_dir;
    uint16 route_heading;
    uint8 reserved106[2];
    Vec3 route_dir;
    uint16 action_flags;
    uint16 action_effect_flags;
    uint8 action_idx;
    uint8 action_phase;
    uint8 reserved126[2];
    uint32 pending_event;
    uint8 turn_delay;
    uint8 projectile_phase;
    uint8 reserved134[1];
    uint8 next_projectile_frame;
    uint32 range_ticks;
    uint32 trigger_ticks[6];
    uint16 action_reset_half;
    uint8 route_mode;
    uint8 reserved167[1];
    sint32 action_speed;
    uint32 sampled_pitch;
    sint16 desired_pitch;
    uint16 height_dir_changes;
    uint8 route_dir_mode;
    uint8 turn_ticks;
    uint8 reserved182[2];
    Vec3 projectile_velocity;
    Vec3 sampled_velocity;
    uint8 reserved208[12];
    uint32 reaction_state;
    uint16 blink_active;
    uint16 blink_ticks;
};

typedef char EncodedActorBindingSize[(sizeof(EncodedActorBinding) == 228) ? 1 : -1];

struct ActorBinding
{
    ActorBinding *next;
    AnimatedObject *instance;
    uint32 event_command;
    uint32 distance_sq;
    sint32 motion_speed;
    sint16 path_parameters[2];
    sint16 path_frame;
    sint16 initial_path_frame;
    sint16 motion_frame;
    sint16 path_pitch_speed;
    sint8 model_kind;
    uint8 flags;
    uint16 respawn_ticks;
    uint32 reaction_flags;
    uint16 respawn_delay;
    SceneNode *event_node;
    // Own the optional header and its packed key payload
    AnimationTrack *rotation_track;
    ActorPlacement *definition;
    uint16 sound_pitch;
    sint16 sound_voice;
    const SoundSample *sound;
    // Borrow the selected registry-owned model definition
    ActorModelDefinition *model;
    sint32 interaction_timer;
    uint32 interaction_heading_ticks;
    sint32 interaction_height;
    uint16 path_heading;
    Vec3 path_dir;
    uint16 route_heading;
    Vec3 route_dir;
    uint16 action_flags;
    uint16 action_effect_flags;
    sint8 action_idx;
    uint8 action_phase;
    uint32 pending_event;
    uint8 turn_delay;
    uint8 projectile_phase;
    uint8 next_projectile_frame;
    uint32 range_ticks;
    uint32 trigger_ticks[6];
    uint16 action_reset_half;
    uint8 route_mode;
    sint32 action_speed;
    uint32 sampled_pitch;
    sint16 desired_pitch;
    uint16 height_dir_changes;
    sint8 route_dir_mode;
    uint8 turn_ticks;
    Vec3 projectile_velocity;
    Vec3 sampled_velocity;
    uint32 reaction_state;
    uint16 blink_active;
    uint16 blink_ticks;

    uint32 StopSound();
    uint32 UpdateSound(AnimatedObject *object, const SoundSample *sound, uint32 mode);
    uint32 ComputeInteractionDistance();
};

#endif
