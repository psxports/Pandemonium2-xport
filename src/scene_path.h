#ifndef SCENE_PATH_H
#define SCENE_PATH_H

#include "psx.h"
#include "animation_data.h"
#include "scene_node.h"

struct SoundSample;

struct PathRouting
{
    uint32 flags;
    uint8 fallback_phase;
    sint8 reject_repeated;
};

extern PathRouting path_routing;

// Runtime animated path state decoded from the section resource
struct AnimatedPathState
{
    AnimatedPathState *next;
    uint16 start_frame;
    sint16 sound_voice;
    const SoundSample *sound;
    uint32 speed[3];
    uint32 direction[3];

    uint32 StopSound();
};

typedef char AnimatedPathStateSize[(sizeof(AnimatedPathState) == 36) ? 1 : -1];
typedef char AnimatedPathStateVoiceOffset[(offsetof(AnimatedPathState, sound_voice) == 6) ? 1 : -1];
typedef char AnimatedPathStateSoundOffset[(offsetof(AnimatedPathState, sound) == 8) ? 1 : -1];

typedef char AnimatedPathStateSpeedOffset[(offsetof(AnimatedPathState, speed) == 12) ? 1 : -1];
typedef char AnimatedPathStateDirectionOffset[(offsetof(AnimatedPathState, direction) == 24) ? 1 : -1];

// Borrow the path matrix cache until resource ownership is migrated
struct PathTransform
{
    MATRIX matrix;
    uint32 displacement[3];
};

typedef char PathTransformSize[(sizeof(PathTransform) == 44) ? 1 : -1];
typedef char PathTransformDisplacementOffset[(offsetof(PathTransform, displacement) == 32) ? 1 : -1];

struct PathDefinition
{
    SceneNode node;
    uint32 flags;

    // Without keys the track state stores a static position
    union
    {
        AnimationTrack positions;
        uint32 static_position[3];
    };

    uint16 metadata36;
    sint16 search_radius;
    PathTransform *transform;

    void SampleTangent(VECTOR *destination, sint32 direction) const;
    uint32 SamplePosition(sint32 frame, uint32 *destination, uint32 direction) const;
    uint32 SampleActorPosition(sint32 frame, uint32 *destination, uint32 direction) const;
    sint32 ProjectFrame(sint32 frame, const uint32 *position, uint32 *floor, uint32 direction, sint32 scale) const;
    uint32 FindHeightFrame(const uint32 *position, uint32 direction, sint32 height_offset) const;

private:
    uint32 FindLinkedHeightFrame(const uint32 *position, uint32 direction, sint32 height_offset) const;
};

// The file stores one tagged word for a path reference or slope coefficient
struct EncodedScenePath
{
    uint32 links[3];
    uint8 kind;
    uint8 flags;
    uint16 state_flags;
    uint32 adjacent_offsets[2];
    sint16 motion_limits[2];
    uint32 fallback_word;
    uint32 matrix_offset;
    uint32 definition_offset;
    EncodedAnimationTrack offset_track;
};

struct ScenePath
{
    SceneNode node;
    const ScenePath *adjacent[2];
    sint16 motion_limits[2];
    const ScenePath *fallback;
    uint32 slope_scale;
    MATRIX *offset_matrix;
    const PathDefinition *definition;
    AnimationTrack offset_track;

    const ScenePath *SelectAdjacent(uint32 *frame_word, uint32 direction, uint32 flags) const;
    const ScenePath *Step(uint32 *frame_word, uint32 direction) const;
    uint32 ProjectOffset(sint32 frame, uint32 *destination, uint32 direction) const;
    uint32 SampleOffset(sint32 frame, uint32 *destination, uint32 direction) const;
    uint32 SamplePosition(sint32 frame, uint32 *destination, uint32 direction) const;
    uint32 IsProjectionWithinHeight(const uint32 *position, uint32 direction) const;
};

typedef char PathDefinitionSize[(sizeof(PathDefinition) == 44) ? 1 : -1];
typedef char PathDefinitionFlagsOffset[(offsetof(PathDefinition, flags) == 16) ? 1 : -1];
typedef char PathDefinitionPositionsOffset[(offsetof(PathDefinition, positions) == 20) ? 1 : -1];
typedef char PathDefinitionTransformOffset[(offsetof(PathDefinition, transform) == 40) ? 1 : -1];
typedef char EncodedScenePathSize[(sizeof(EncodedScenePath) == 56) ? 1 : -1];
typedef char EncodedScenePathFallbackOffset[(offsetof(EncodedScenePath, fallback_word) == 28) ? 1 : -1];
typedef char EncodedScenePathDefinitionOffset[(offsetof(EncodedScenePath, definition_offset) == 36) ? 1 : -1];

#ifdef __cplusplus
extern "C"
{
#endif

    uint32 InterpolateKeyTime(uint32 frame, uint32 end, uint32 start_key, uint32 end_key, const uint8 *key_data);
    uint32 SelectScenePath(const ScenePath *path);
    sint32 ProjectPathStart(const ScenePath *path, uint32 *position, uint32 *floor, uint32 direction);

#ifdef __cplusplus
}
#endif

#endif
