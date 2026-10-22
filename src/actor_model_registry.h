#ifndef ACTOR_MODEL_REGISTRY_H
#define ACTOR_MODEL_REGISTRY_H
#include <stddef.h>

#include "psx.h"
#include "animated_object.h"
#include "material_catalog.h"
#include "sound_table_header.h"

struct AnimationHeader;
struct MeshFace;
struct MeshVertex;
struct MorphRestorationHeader;

struct EncodedActorAction;
struct ActorActionDefinition;
struct EncodedActorMetadata;
struct ActorModelDefinition;
struct ActorModelRegistry;

struct EncodedActorAction
{
    uint16 flags;
    uint16 reserved2;
    uint32 distance_threshold;
    uint8 priority_mode;
    uint8 trigger;
    uint8 priority_frame;
    uint8 reserved11;
    sint16 trigger_ticks;
    uint16 reserved14;
    sint16 animation;
    sint16 path_motion_mode;
    sint16 hold_ticks;
    sint16 hold_animation;
    uint32 hold_speed;
    sint16 hold_motion;
    sint16 motion;
    uint32 speed;
    sint16 switch_frame;
    sint16 next_motion;
    uint32 next_speed;
    sint16 projectile_kind;
    sint16 projectile_frame;
    uint32 projectile_height;
    uint16 projectile_interval;
    uint16 projectile_end_frame;
    sint16 aim_mode;
    uint16 reserved58;
    uint32 spread_angle;
    uint32 projectile_offset;
    uint8 reserved68[12];
};

typedef char EncodedActorActionSize[(sizeof(EncodedActorAction) == 80) ? 1 : -1];

struct ActorActionDefinition
{
    uint16 flags;
    uint32 distance_threshold;
    uint8 priority_mode;
    uint8 trigger;
    uint8 priority_frame;
    sint16 trigger_ticks;
    sint16 animation;
    sint16 path_motion_mode;
    sint16 hold_ticks;
    sint16 hold_animation;
    uint32 hold_speed;
    sint16 hold_motion;
    sint16 motion;
    uint32 speed;
    sint16 switch_frame;
    sint16 next_motion;
    uint32 next_speed;
    sint16 projectile_kind;
    sint16 projectile_frame;
    uint32 projectile_height;
    uint16 projectile_interval;
    uint16 projectile_end_frame;
    sint16 aim_mode;
    uint32 spread_angle;
    uint32 projectile_offset;
};

#pragma pack(push, 1)

// File metadata contains an unaligned halfword at byte 631
struct EncodedActorMetadata
{
    uint32 reserved0;
    sint32 target_range[2];
    uint32 reserved12;
    sint32 height_offset;
    sint32 patrol_radius;
    uint32 reserved24;
    sint32 motion_speed[2];
    sint32 motion_accel;
    sint32 spr_extent[2];
    uint32 flags;
    uint8 reserved52[8];
    sint8 initial_health;
    sint8 kind;
    uint16 reserved62;
    sint16 shot_interval;
    sint16 shot_frame;
    sint16 turn_step;
    uint16 instance_cost;
    uint32 animation_offsets[14];
    uint32 reserved128;
    sint16 sound_radius;
    uint16 sound_interval;
    uint8 action_counter_flags;
    uint8 reserved137[3];
    EncodedActorAction actions[6];
    uint16 reserved620;
    uint8 contact_damage;
    uint8 reserved623;
    uint16 contact_mode;
    uint8 reserved626;
    sint8 path_flag;
    uint8 fog_color[3];
    uint16 reserved631;
    sint8 kind_override;
    uint16 reserved634;
    sint16 reaction_motion_mode;
    uint16 reserved638;
    sint16 fixed_pitch;
    sint16 pitch_limit;
    sint16 target_avoidance_mode;
    sint16 interaction_limit_step;
    uint16 interaction_delay_bias;
    uint16 action_damage;
    sint16 turn_motion_mode;
    sint16 interaction_animation_mode;
    uint16 interaction_frame;
    sint16 route_action;
    sint16 turnaround_range;
    sint16 health_restore_count;
    uint8 reserved664[8];
};

#pragma pack(pop)

typedef char EncodedActorMetadataSize[(sizeof(EncodedActorMetadata) == 672) ? 1 : -1];

struct ActorModelDefinition
{
    typedef uint32 (AnimatedObject::*Constructor)();
    Constructor constructor;
    AnimatedObject *prototype;
    uint8 *model_data;
    uint32 model_bytes;
    MeshFace *model_polygons;
    MeshVertex *model_vertices;
    MaterialCatalog material_resource;
    AnimationHeader *animations[14];
    ActorActionDefinition actions[6];
    sint8 kind;
    sint8 kind_override;
    sint8 initial_health;
    sint32 target_range[2];
    sint32 height_offset;
    sint32 patrol_radius;
    sint16 shot_interval;
    sint16 shot_frame;
    sint8 path_flag;
    sint16 fixed_pitch;
    sint16 pitch_limit;
    sint16 target_avoidance_mode;
    sint16 health_restore_count;
    uint8 contact_damage;
    sint32 motion_speed[2];
    sint32 motion_accel;
    sint32 spr_extent[2];
    sint16 turn_step;
    sint16 reaction_motion_mode;
    sint16 turn_motion_mode;
    uint16 instance_cost;
    sint16 sound_radius;
    uint16 contact_mode;
    uint8 fog_color[3];
    uint16 action_damage;
    uint16 sound_interval;
    sint16 interaction_limit_step;
    uint16 interaction_delay_bias;
    sint16 interaction_animation_mode;
    uint16 interaction_frame;
    sint16 route_action;
    sint16 turnaround_range;
    uint8 action_counter_flags;
    const SoundBankStorage *sound_table;
    SoundBankStorage sound_bank;
    // Own the decoded animation bank independently of guest resource buffers
    uint8 *animation_data;
    uint32 animation_bytes;
    AnimationHeader *animation_headers;
    MorphRestorationHeader *morph_restoration;
};

struct ActorModelRegistry
{
    sint8 count;
    ActorModelDefinition definitions[6];
    uint8 load_reset_byte;

    static uint32 LoadActorModel(uint32 idx);
    static void ResetActorModels(void);
    static void InitializeActorModels(void);
    static uint32 ResetActorLoadState(void);
};

extern ActorModelRegistry actor_models;

#endif
