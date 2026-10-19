#ifndef RENDER_VIEW_H
#define RENDER_VIEW_H

#include <stddef.h>

#include "psx.h"
#include "mesh_geometry.h"

struct CollisionSphere;
struct TextureDescriptor;

struct FrustumPlane
{
    unsigned int bias;
    SVECTOR normal;
};

struct RenderMatrixSlot
{
    MATRIX matrix;
    RenderMatrixSlot *prev;
    RenderMatrixSlot *next;
    uint32 depth;
};

struct RenderViewState
{
    enum
    {
        ProjectionCapacity = 65536
    };

    RenderMatrixSlot matrix_root;
    RenderMatrixSlot *matrix_cursor;
    unsigned int fog_color;
    unsigned int far_distance;
    unsigned int fog_scale;
    unsigned int depth_scale;
    unsigned int object_scale;
    unsigned int scale_reciprocal;
    ProjectedVertex projected_vertices[ProjectionCapacity];
    uint16 first_projected_vertex;
    FrustumPlane planes[5];
    MeshLighting *lighting_cache;
    bool lighting_enabled;
    const TextureDescriptor *material_override;
    uint16 object_depth_bias;
    CollisionSphere *sphere_output;
    uint8 special_transform;
    VECTOR transform_result;
    MATRIX camera_transform;
    MATRIX inverse_transform;
    MATRIX billboard_transform;
};

typedef char FrustumPlaneSize[(sizeof(FrustumPlane) == 12) ? 1 : -1];
typedef char FrustumPlaneNormalOffset[(offsetof(FrustumPlane, normal) == 4) ? 1 : -1];

extern RenderViewState render_view;

#endif
