#ifndef COLLISION_H
#define COLLISION_H
#include <stddef.h>

#include "psx.h"
#include "animation_track.h"
#include "scene_node.h"
#include "vector3.h"

struct MeshFace;

struct CollisionContact;
struct CollisionSphere;
union ObjectBounds;
struct EncodedSphereNode;
struct SphereNode;
struct EncodedCollisionTransform;
struct CollisionTransform;
struct CharacterGeometry;
struct TriggerContactMotion;

typedef struct CollisionContact
{
    uint32 selection;
    uint32 fraction;
    uint32 other_selection;
    uint32 flags;
} CollisionContact;

typedef char CollisionContactSize[(sizeof(CollisionContact) == 16) ? 1 : -1];

typedef char CollisionContactFractionOffset[(offsetof(CollisionContact, fraction) == 4) ? 1 : -1];

typedef char CollisionContactFlagsOffset[(offsetof(CollisionContact, flags) == 12) ? 1 : -1];

struct CollisionSphere
{
    Vec3 pos;
    uint32 radius;
};

// Other object spheres still use this binary layout in their resource buffers
typedef char CollisionSphereSize[(sizeof(CollisionSphere) == 16) ? 1 : -1];

// Bounds collection stores a maximum before deriving radius and vertical/depth extents
union ObjectBounds
{
    Vec3 maximum;

    struct
    {
        uint32 radius;
        uint32 height;
        uint32 depth;
    };
};

struct EncodedSphereNode
{
    EncodedSceneNode node;
    AnimationTrackBinding animation_binding;
    Vec3 pos;
    uint32 radius;
};

typedef char EncodedSphereNodeSize[(sizeof(EncodedSphereNode) == 36) ? 1 : -1];

struct SphereNode
{
    SceneNode node;
    AnimationTrackBinding animation_binding;
    Vec3 pos;
    uint32 radius;
};


struct EncodedCollisionTransform
{
    sint16 first_face, face_count;
    sint16 rotation[9];
    uint16 reserved;
    Vec3 pos;
    Vec3 motion;
};

struct CollisionTransform
{
    sint16 first_face, face_count;
    sint16 rotation[9];
    Vec3 pos;
    Vec3 motion;
};

typedef char EncodedCollisionTransformSize[(sizeof(EncodedCollisionTransform) == 48) ? 1 : -1];

struct CharacterGeometry
{
    CollisionSphere world[4];
    CollisionSphere local[4];
};

struct TriggerContactMotion
{
    uint32 blocked;
    uint32 dir;
};

#endif
