#ifndef ACTOR_BINDING_H
#define ACTOR_BINDING_H

#include <stddef.h>

#include "psx.h"

struct AnimatedObject;
struct SoundSample;
struct ActorModelDefinition;
struct ActorPlacement;
struct AnimationTrack;
struct SceneNode;

// Runtime actor state decoded from the section resource
struct ActorBinding
{
    ActorBinding *next;
    AnimatedObject *instance;
    uint32 event_command;
    uint32 distance_squared;
    uint32 motion_speed;
    uint16 path_parameters[2];
    uint16 path_frame;
    uint16 initial_path_frame;
    uint16 motion_frame;
    uint16 path_pitch_speed;
    uint8 resource_bytes32[5];
    uint8 model_kind;
    uint8 flags;
    uint8 resource_bytes39[1];
    uint16 respawn_ticks;
    uint8 resource_bytes42[2];
    uint32 reaction_flags;
    uint16 respawn_delay;
    uint8 resource_bytes50[2];
    SceneNode *event_node;
    // Own the optional header and its packed key payload
    AnimationTrack *rotation_track;
    ActorPlacement *definition;
    uint16 sound_pitch;
    uint16 sound_voice;
    const SoundSample *sound;
    // Borrow the selected registry-owned model definition
    ActorModelDefinition *model;
    uint32 interaction_timer;
    uint32 interaction_heading_ticks;
    uint32 interaction_height;
    uint16 path_heading;
    uint8 resource_bytes90[2];
    uint32 path_direction[3];
    uint16 route_heading;
    uint8 resource_bytes106[2];
    uint32 route_direction[3];
    uint16 action_flags;
    uint16 action_effect_flags;
    uint8 action_index;
    uint8 action_phase;
    uint8 resource_bytes126[2];
    uint32 pending_event;
    uint8 turn_delay;
    uint8 projectile_phase;
    uint8 resource_bytes134[1];
    uint8 next_projectile_frame;
    uint32 range_ticks;
    uint32 trigger_ticks[6];
    uint16 action_reset_half;
    uint8 route_mode;
    uint8 resource_bytes167[1];
    uint32 action_speed;
    uint32 sampled_pitch;
    uint16 desired_pitch;
    uint16 height_direction_changes;
    uint8 route_direction_mode;
    uint8 turn_ticks;
    uint8 resource_bytes182[2];
    uint32 projectile_velocity[3];
    uint32 sampled_velocity[3];
    uint8 resource_bytes208[12];
    uint32 reaction_state;
    uint16 blink_active;
    uint16 blink_ticks;

    uint32 ComputeInteractionDistance();
    uint32 StopSound();
    uint32 UpdateSound(AnimatedObject *object, const SoundSample *sound, uint32 mode);
};

typedef char ActorBindingSize[(sizeof(ActorBinding) == 228) ? 1 : -1];

// The current native record layout is restricted to the 32-bit target
typedef char ActorBindingInstanceOffset[(offsetof(ActorBinding, instance) == 4) ? 1 : -1];

typedef char ActorBindingDefinitionOffset[(offsetof(ActorBinding, definition) == 60) ? 1 : -1];

typedef char ActorBindingSoundOffset[(offsetof(ActorBinding, sound) == 68) ? 1 : -1];
typedef char ActorBindingModelOffset[(offsetof(ActorBinding, model) == 72) ? 1 : -1];
typedef char ActorBindingRotationTrackOffset[(offsetof(ActorBinding, rotation_track) == 56) ? 1 : -1];

typedef char ActorBindingActionOffset[(offsetof(ActorBinding, action_flags) == 120) ? 1 : -1];
typedef char ActorBindingVelocityOffset[(offsetof(ActorBinding, sampled_velocity) == 196) ? 1 : -1];

#endif
