#ifndef SCENE_PATH_H
#define SCENE_PATH_H
#include <stddef.h>

#include "psx.h"
#include "animation_track.h"
#include "scene_node.h"

struct PathDefinition;
union Vec3;

struct EncodedScenePath;
struct ScenePath;

// The file stores one tagged word for a path reference or slope coefficient
struct EncodedScenePath
{
    EncodedSceneNode node;
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

    void SamplePos(sint32 frame, Vec3 &dst, uint32 dir) const;
    void SampleOffset(sint32 frame, Vec3 &dst, uint32 dir) const;
    const ScenePath *Step(uint32 *frame_word, uint32 dir) const;
    const ScenePath *SelectAdjacent(uint32 *frame_word, uint32 dir, uint32 flags) const;
    uint32 IsProjWithinHeight(const Vec3 &pos, uint32 dir) const;
    uint32 ProjectOffset(sint32 frame, Vec3 &dst, uint32 dir) const;
    static sint16 SampleScenePathHeading(sint32 frame0, sint32 frame1, const ScenePath *path);
    static sint32 ProjectPathStart(const ScenePath *path, Vec3 &pos, Vec3 &floor, uint32 dir);
};

typedef char EncodedScenePathSize[(sizeof(EncodedScenePath) == 56) ? 1 : -1];

typedef char EncodedScenePathFallbackOffset[(offsetof(EncodedScenePath, fallback_word) == 28) ? 1 : -1];

typedef char EncodedScenePathDefinitionOffset[(offsetof(EncodedScenePath, definition_offset) == 36) ? 1 : -1];

#endif
