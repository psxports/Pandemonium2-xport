#ifndef ANIMATED_MODEL_REGISTRY_H
#define ANIMATED_MODEL_REGISTRY_H
#include <stddef.h>

#include "psx.h"
#include "sound_table_header.h"

struct AnimationHeader;
struct MorphRestorationHeader;

struct EncodedAnimatedModelMetadata;
struct AnimatedModelDefinition;
struct AnimatedModelRegistry;

struct EncodedAnimatedModelMetadata
{
    uint16 flags;
    sint8 kind_id;
    uint8 resource_byte3;
    uint32 initial_speed;
    uint32 speed_limit;
    uint32 gravity;
    uint32 damping;
    uint32 col_scale;
    uint32 impulse;
    uint8 reserved28[8];
    uint32 animation_offsets[2];
    uint16 sound_radius;
    uint16 resource_halfword46;
};

typedef char EncodedAnimatedModelMetadataSize[(sizeof(EncodedAnimatedModelMetadata) == 48) ? 1 : -1];

// Native animated-model fields decoded from the resource record
struct AnimatedModelDefinition
{
    uint16 flags;
    sint8 kind_id;
    uint8 resource_byte3;
    uint32 initial_speed;
    uint32 speed_limit;
    uint32 gravity;
    uint32 damping;
    uint32 col_scale;
    uint32 impulse;
    AnimationHeader *animations[2];
    uint16 sound_radius;
    uint16 resource_halfword46;
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
    AnimatedModelDefinition recs[4];
    AnimatedModelDefinition *definitions[4];

    static void ResetAnimatedModels(void);
    static void RegisterAnimatedModelOperations(void);
};

extern AnimatedModelRegistry animated_models;

#endif
