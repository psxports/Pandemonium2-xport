#ifndef MESH_GEOMETRY_H
#define MESH_GEOMETRY_H

#include <stddef.h>

#include "psx.h"
#include "scene_node.h"

struct TextureDescriptor;
struct MeshFace;
struct MeshVertex;

// Shared binary geometry prefix without an inferred inheritance relationship
struct MeshGeometryNode
{
    SceneNode node;
    uint32 position[3];
    uint32 flags;
    sint16 polygon_count;
    sint16 vertex_count;
    MeshFace *polygons;

    // Select the role through the signed vertex count
    union
    {
        MeshVertex *vertices;
        MeshGeometryNode *source_mesh;
    };

    uint32 *material_animation_entries;
    uint16 lighting_index;
    sint16 first_vertex;
};

typedef char MeshGeometryNodeSize[(sizeof(MeshGeometryNode) == 52) ? 1 : -1];
typedef char MeshGeometryNodeCountOffset[(offsetof(MeshGeometryNode, polygon_count) == 32) ? 1 : -1];
typedef char MeshGeometryNodeSourceOffset[(offsetof(MeshGeometryNode, source_mesh) == 40) ? 1 : -1];
typedef char MeshGeometryNodeFirstVertexOffset[(offsetof(MeshGeometryNode, first_vertex) == 50) ? 1 : -1];

struct ProjectedCoordinates
{
    uint16 x, y;
};

struct ProjectedVertex
{
    union
    {
        uint32 xy;
        ProjectedCoordinates coordinates;
    };

    uint32 depth;
};

struct MeshLighting
{
    uint32 colors[3];
    uint32 control;
};

typedef char ProjectedVertexSize[(sizeof(ProjectedVertex) == 8) ? 1 : -1];
typedef char MeshLightingSize[(sizeof(MeshLighting) == 16) ? 1 : -1];

// Immutable vector words in the model file
struct EncodedMeshVertex
{
    uint16 position[4];
    uint16 normal[4];
};

typedef char EncodedMeshVertexSize[(sizeof(EncodedMeshVertex) == 16) ? 1 : -1];

struct MeshVertex
{
    SVECTOR position;
    SVECTOR normal;
};

struct MeshFaceNormalFields
{
    sint16 x, y, z;
    // Low bit enables collision; rendering shifts the signed value right once
    sint16 depth_bias;
};

union MeshFaceNormal
{
    uint32 words[2];
    SVECTOR vector;
    MeshFaceNormalFields components;
};

// Immutable polygon input with a numeric texture word
struct EncodedMeshFace
{
    uint32 texture_word;
    uint16 vertex_indices[4];
    uint32 plane;
    MeshFaceNormal normal;
};

typedef char EncodedMeshFaceSize[(sizeof(EncodedMeshFace) == 24) ? 1 : -1];

struct MeshFace
{
    const TextureDescriptor *texture;
    uint16 vertex_indices[4];
    uint32 plane;
    MeshFaceNormal normal;
};

typedef char MeshVertexSize[(sizeof(MeshVertex) == 16) ? 1 : -1];
typedef char MeshVertexNormalOffset[(offsetof(MeshVertex, normal) == 8) ? 1 : -1];
typedef char MeshFaceSize[(sizeof(MeshFace) == 24) ? 1 : -1];
typedef char MeshFacePlaneOffset[(offsetof(MeshFace, plane) == 12) ? 1 : -1];
typedef char MeshFaceNormalOffset[(offsetof(MeshFace, normal) == 16) ? 1 : -1];

uint32 DrawFacesWithFreshLighting(const MeshVertex *vertices, const MeshFace *faces, sint32 count);
uint32 DrawFacesWithCachedLighting(const MeshVertex *vertices, const MeshFace *faces, sint32 count);
uint32 DrawUniformColorFaces(const MeshFace *faces, sint32 count);
void DrawFacesWithoutLightingCache(const MeshVertex *vertices, const MeshFace *faces, sint32 count);

uint32 DrawSceneGeometry(void);

#endif
