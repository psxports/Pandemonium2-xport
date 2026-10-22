#ifndef MESH_GEOMETRY_NODE_H
#define MESH_GEOMETRY_NODE_H
#include <stddef.h>

#include "psx.h"
#include "animation_track.h"
#include "quaternion16.h"
#include "scene_node.h"
#include "vector3.h"

struct MeshFace;
struct MeshVertex;

struct TextureDescriptor;

struct EncodedMeshGeometryNode;
struct MeshGeometryNode;
struct ProjectedCoordinates;
struct ProjectedVertex;
struct EncodedMeshLighting;
struct MeshLighting;
struct EncodedMeshVertex;
struct MeshVertex;
struct MeshFaceNormal;
struct EncodedMeshFace;
struct MeshFace;
struct MorphSpan;
struct EncodedMeshPartNode;
struct MeshPartNode;

// Shared binary geometry prefix without an inferred inheritance relationship
struct EncodedMeshGeometryNode
{
    EncodedSceneNode node;
    Vec3 pos;
    uint32 flags;
    sint16 polygon_count;
    sint16 vertex_count;
    uint32 polygons_offset;
    uint32 geometry_offset;
    uint32 materials_offset;
    uint16 lighting_idx;
    sint16 first_vertex;
};

typedef char EncodedMeshGeometryNodeSize[(sizeof(EncodedMeshGeometryNode) == 52) ? 1 : -1];

struct MeshGeometryNode
{
    SceneNode node;
    Vec3 pos;
    uint32 flags;
    sint16 polygon_count;
    sint16 vertex_count;
    MeshFace *polygons;

    // Select the role through the signed vertex count
    union
    {
        MeshVertex *vertices;
        MeshGeometryNode *src_mesh;
    };

    uint32 *material_animation_entries;
    uint16 lighting_idx;
    sint16 first_vertex;

    static void DrawSceneGeometry(void);
    static void DrawFacesWithFreshLighting(const MeshVertex *vertices, const MeshFace *faces, sint32 count);
    static void DrawFacesWithCachedLighting(const MeshVertex *vertices, const MeshFace *faces, sint32 count);
    static void DrawUniformColorFaces(const MeshFace *faces, sint32 count);
    static void DrawFacesWithoutLightingCache(const MeshVertex *vertices, const MeshFace *faces, sint32 count);
};

typedef char MeshGeometryNodeSize[(sizeof(MeshGeometryNode) == 52) ? 1 : -1];

typedef char MeshGeometryNodeCountOffset[(offsetof(MeshGeometryNode, polygon_count) == 32) ? 1 : -1];

typedef char MeshGeometryNodeSourceOffset[(offsetof(MeshGeometryNode, src_mesh) == 40) ? 1 : -1];

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
        ProjectedCoordinates coords;
    };

    uint32 depth;
};

struct EncodedMeshLighting
{
    uint32 colors[3];
    uint32 control;
};

typedef char EncodedMeshLightingSize[(sizeof(EncodedMeshLighting) == 16) ? 1 : -1];

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
    uint16 pos[4];
    uint16 n[4];
};

typedef char EncodedMeshVertexSize[(sizeof(EncodedMeshVertex) == 16) ? 1 : -1];

struct MeshVertex
{
    SVECTOR pos;
    SVECTOR n;
};

struct MeshFaceNormal
{
    sint16 x, y, z;
    // Low bit enables collision; rendering shifts the signed value right once
    sint16 depth_bias;
};

// Immutable polygon input with a numeric texture word
struct EncodedMeshFace
{
    uint32 tex_word;
    uint16 vertex_idxs[4];
    uint32 plane;
    MeshFaceNormal n;
};

typedef char EncodedMeshFaceSize[(sizeof(EncodedMeshFace) == 24) ? 1 : -1];

struct MeshFace
{
    const TextureDescriptor *tex;
    uint16 vertex_idxs[4];
    uint32 plane;
    MeshFaceNormal n;
};

typedef char MeshVertexSize[(sizeof(MeshVertex) == 16) ? 1 : -1];

typedef char MeshVertexNormalOffset[(offsetof(MeshVertex, n) == 8) ? 1 : -1];

typedef char MeshFaceSize[(sizeof(MeshFace) == 24) ? 1 : -1];

typedef char MeshFacePlaneOffset[(offsetof(MeshFace, plane) == 12) ? 1 : -1];

typedef char MeshFaceNormalOffset[(offsetof(MeshFace, n) == 16) ? 1 : -1];

struct MorphSpan
{
    sint16 first_vertex;
    sint16 vertex_count;
    sint16 first_polygon;
    sint16 polygon_count;
};

typedef char MorphSpanSize[(sizeof(MorphSpan) == 8) ? 1 : -1];

struct EncodedMeshPartNode
{
    EncodedSceneNode node;
    Vec3 pos;
    AnimationTrackBinding animation_binding;
    MorphSpan span;
    Quat rotation;
};

typedef char EncodedMeshPartNodeSize[(sizeof(EncodedMeshPartNode) == 48) ? 1 : -1];

// Rotation storage also carries debris velocity and packed spin
struct MeshPartNode
{
    SceneNode node;
    Vec3 pos;
    AnimationTrackBinding animation_binding;
    MorphSpan span;

    union
    {
        sint16 rotation[4];
        SVECTOR velocity;
        Quat quat;
    };
};

typedef char MeshPartNodeSize[(sizeof(MeshPartNode) == 48) ? 1 : -1];

#endif
