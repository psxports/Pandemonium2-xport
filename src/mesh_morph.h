#ifndef MESH_MORPH_H
#define MESH_MORPH_H

#include "animation_data.h"
#include "scene_node.h"
#include "mesh_geometry.h"

struct AnimatedObject;

struct MorphSpan
{
    sint16 first_vertex;
    sint16 vertex_count;
    sint16 first_polygon;
    sint16 polygon_count;
};

typedef char MorphSpanSize[(sizeof(MorphSpan) == 8) ? 1 : -1];

// Rotation storage also carries debris velocity and packed spin
struct MeshPartNode
{
    SceneNode node;
    uint32 position[3];
    AnimationTrackBinding animation_binding;
    MorphSpan span;

    union
    {
        sint16 rotation[4];
        SVECTOR velocity;
    };
};

typedef char MeshPartNodeSize[(sizeof(MeshPartNode) == 48) ? 1 : -1];

void SampleMorphTrack(sint32 frame, const MorphAnimationTrack *track, const AnimatedObject *owner, const MorphSpan *span);

#endif
