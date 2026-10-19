#ifndef MODEL_REGISTRY_H
#define MODEL_REGISTRY_H

#include <stddef.h>

#include "psx.h"
#include "animated_object.h"
#include "sound_resources.h"
#include "material_resources.h"

struct AnimatedObject;
struct AnimationHeader;
struct MorphRestorationHeader;
struct MeshFace;
struct MeshVertex;

// Native definition decoded from the 48-byte resource record
struct ModelGroupDefinition
{
    uint8 flags;
    uint8 kind;
    uint16 lifetime;
    const SoundBankStorage *sound_table;
    SoundBankStorage sound_bank;
    AnimatedObject *models[4];
    uint32 launch_speed;
    uint32 metadata_word;
    uint32 gravity;
    uint32 sound_radius;
    uint32 doppler_radius;
    uint8 contact_type;
    uint8 metadata_bytes[3];

    // Own complete model payloads for the four prototype slots
    uint8 *model_data[4];
    uint32 model_bytes[4];
    MeshFace *model_polygons[4];
    MeshVertex *model_vertices[4];
    MaterialResourceHeader material_resources[4];
    // Own animation banks for the four prototype slots
    uint8 *animation_data[4];
    uint32 animation_bytes[4];
    AnimationHeader *animation_headers[4];

    void InitializeSpawn(AnimatedObject *spawned, AnimatedObject *object, uint32 phase) const;
};

struct ModelGroupRegistry
{
    enum
    {
        CommonCount = 3,
        Capacity = 12
    };

    uint32 count;
    ModelGroupDefinition records[Capacity];
    ModelGroupDefinition *definitions[Capacity];
    // File identities preserve aliases within each package
    uint32 source_packages[Capacity];
    uint32 source_offsets[Capacity];
};

struct ActorActionDefinition
{
    uint16 flags;
    uint16 metadata2;
    uint32 distance_threshold;
    uint8 priority_mode;
    uint8 trigger;
    uint8 priority_frame;
    uint8 metadata11;
    sint16 trigger_ticks;
    uint16 metadata14;
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
    uint16 metadata58;
    uint32 spread_angle;
    uint32 projectile_offset;
    uint32 metadata68[3];
};

struct ActorModelDefinition
{
    typedef uint32 (AnimatedObject::*Constructor)();
    Constructor constructor;
    AnimatedObject *prototype;
    uint8 *model_data;
    uint32 model_bytes;
    MeshFace *model_polygons;
    MeshVertex *model_vertices;
    MaterialResourceHeader material_resource;
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
    sint32 sprite_extent[2];
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
    enum
    {
        LoadSlotCount = 8
    };

    uint8 count;
    ActorModelDefinition definitions[6];
    uint8 load_reset_byte;
    uint32 load_words[LoadSlotCount][3];
};

// Native animated-model fields decoded from the resource record
struct AnimatedModelDefinition
{
    uint16 flags;
    sint8 kind_id;
    uint8 metadata3;
    uint32 initial_speed;
    uint32 speed_limit;
    uint32 gravity;
    uint32 damping;
    uint32 collision_scale;
    uint32 impulse;
    uint32 metadata28[2];
    AnimationHeader *animations[2];
    uint16 sound_radius;
    uint16 metadata46;
    const SoundBankStorage *sound_table;
    SoundBankStorage sound_bank;
    // Own the decoded animation bank independently of guest resource buffers
    uint8 *animation_data;
    uint32 animation_bytes;
    AnimationHeader *animation_headers;
    MorphRestorationHeader *morph_restoration;
};

struct AnimatedModelRegistry
{
    uint32 count;
    AnimatedModelDefinition records[4];
    AnimatedModelDefinition *definitions[4];
};

extern ModelGroupRegistry model_groups;
extern ActorModelRegistry actor_models;
extern AnimatedModelRegistry animated_models;

extern "C"
{
    void RegisterAnimatedModelOperations(void);
    void LoadCommonModelGroups(void);
    void InitializeActorModels(void);
    void RegisterActorVelocityOperations(void);
    uint32 ResetActorLoadWorkspace(void);
    void ResetActorModels(void);
    void ResetAnimatedModels(void);
    uint32 RetainCommonModelGroups(void);
    void ClearModelGroups(void);
    uint32 LoadModelGroup(uint8 kind);
    uint32 LoadActorModel(uint32 index);
    AnimatedObject *UpdateModelGroup(AnimatedObject *object);
    uint32 GetModelGroupLifetime(uint32 kind);
    uint32 GetModelGroupContactType(uint32 kind);
    ModelGroupDefinition *FindRelatedModelGroup(uint32 kind);
    ModelGroupDefinition *SetRelatedModelGroupContactType(uint32 kind, uint8 value);
}

#endif
