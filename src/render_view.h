#ifndef RENDER_VIEW_H
#define RENDER_VIEW_H

#include "psx.h"
#include <stddef.h>

struct FrustumPlane
{
    unsigned int bias;
    SVECTOR normal;
};

struct RenderViewState
{
    unsigned int fog_color;
    unsigned int far_distance;
    unsigned int fog_scale;
    unsigned int depth_scale;
    unsigned int object_scale;
    unsigned int scale_reciprocal;
    unsigned int projection_offset;
    unsigned short projection_count;
    unsigned short projection_count_aux;
    FrustumPlane planes[5];
};

typedef char FrustumPlaneSize[(sizeof(FrustumPlane) == 12) ? 1 : -1];
typedef char FrustumPlaneNormalOffset[(offsetof(FrustumPlane, normal) == 4) ? 1 : -1];
typedef char RenderViewPlanesOffset[(offsetof(RenderViewState, planes) == 32) ? 1 : -1];
typedef char RenderViewSize[(sizeof(RenderViewState) == 92) ? 1 : -1];

extern RenderViewState render_view;

#endif
