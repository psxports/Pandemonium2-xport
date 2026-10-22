#ifndef PATH_DEFINITION_H
#define PATH_DEFINITION_H
#include <stddef.h>

#include "psx.h"
#include "animation_track.h"
#include "scene_node.h"
#include "vector3.h"

extern Vec3 path_heading_delta;

struct PathTransform;
union EncodedPathPos;
struct EncodedPathDefinition;
struct PathDefinition;

// Native path matrix and its fixed-point displacement
struct PathTransform
{
    MATRIX matrix;
    Vec3 displacement;
};

typedef char PathTransformSize[(sizeof(PathTransform) == 44) ? 1 : -1];

typedef char PathTransformDisplacementOffset[(offsetof(PathTransform, displacement) == 32) ? 1 : -1];

union EncodedPathPos
{
    EncodedAnimationTrack track;
    Vec3 static_pos;
};

struct EncodedPathDefinition
{
    EncodedSceneNode node;
    uint32 flags;
    EncodedPathPos pos;
    uint16 frame_scale;
    sint16 search_radius;
    uint32 transform_offset;
};

typedef char EncodedPathDefinitionSize[(sizeof(EncodedPathDefinition) == 44) ? 1 : -1];

struct PathDefinition
{
    SceneNode node;
    uint32 flags;

    // Without keys the track state stores a static position
    union
    {
        AnimationTrack pos_track;
        Vec3 static_pos;
    };

    sint16 frame_scale;
    sint16 search_radius;
    PathTransform *transform;

    void SampleTangent(VECTOR *dst, sint32 dir) const;
    void SamplePos(sint32 frame, Vec3 &dst, uint32 dir) const;
    uint32 FindHeightFrame(const Vec3 &pos, uint32 dir, sint32 height_offset) const;

private:
    uint32 FindLinkedHeightFrame(const Vec3 &pos, uint32 dir, sint32 height_offset) const;

public:
    sint32 ProjectFrame(sint32 frame, const Vec3 &pos, Vec3 &floor, uint32 dir, sint32 scale) const;
    void SampleActorPos(sint32 frame, Vec3 &dst, uint32 dir) const;
    static sint16 SampleDefinitionHeading(sint32 frame0, sint32 frame1, const PathDefinition *definition);
};

typedef char PathDefinitionSize[(sizeof(PathDefinition) == 44) ? 1 : -1];

typedef char PathDefinitionFlagsOffset[(offsetof(PathDefinition, flags) == 16) ? 1 : -1];

typedef char PathDefinitionPositionsOffset[(offsetof(PathDefinition, pos_track) == 20) ? 1 : -1];

typedef char PathDefinitionTransformOffset[(offsetof(PathDefinition, transform) == 40) ? 1 : -1];

#endif
