#ifndef MORPH_BLEND_CONTEXT_H
#define MORPH_BLEND_CONTEXT_H
#include <stddef.h>

#include "psx.h"

struct AnimatedObject;
struct MeshVertex;
struct MorphAnimationTrack;
struct MorphSpan;

struct MorphBlendContext;

struct MorphBlendContext
{
    const AnimatedObject *owner;
    const MorphSpan *span;
    MeshVertex *dst;
    const SVECTOR *source0;
    const SVECTOR *source1;
    uint32 weight;

    void Apply() const;
};

#endif
