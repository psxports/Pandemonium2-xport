#ifndef COLLISION_GEOMETRY_H
#define COLLISION_GEOMETRY_H

#include "psx.h"
#include "scene_node.h"
#include "animation_data.h"

struct CollisionSphere
{
    uint32 position[3];
    uint32 radius;
};

// Other object spheres still use this binary layout in their resource buffers
typedef char CollisionSphereSize[(sizeof(CollisionSphere) == 16) ? 1 : -1];

struct SphereNode
{
    SceneNode node;
    AnimationTrackBinding animation_binding;
    uint32 position[3];
    uint32 radius;
};

typedef char SphereNodeSize[(sizeof(SphereNode) == 36) ? 1 : -1];

struct CollisionTransformFields
{
    sint16 first_face, face_count;
    sint16 rotation[9];
    uint16 metadata22;
    uint32 position[3];
    uint32 motion[3];
};

union CollisionTransform
{
    uint32 words[12];
    CollisionTransformFields fields;
};

typedef char CollisionTransformSize[(sizeof(CollisionTransform) == 48) ? 1 : -1];

extern CollisionTransform collision_transform;

struct CharacterGeometry
{
    CollisionSphere world[4];
    CollisionSphere local[4];
};

extern CharacterGeometry character_geometry;

struct TriggerContactMotion
{
    uint32 blocked;
    uint32 dir;
};

extern TriggerContactMotion trigger_contact_motion;

struct MeshFace;

extern "C"
{
    uint32 DisableCharacterSupportFace(void);
    uint32 RestoreCharacterSupportFace(void);
    uint32 SweepSphereAgainstObject(const CollisionSphere *bounds, const SceneNode *object, const uint32 *delta, uint32 *fraction, const CollisionSphere *geometry);
    const MeshFace *SelectCollisionFace(const SceneNode *object, uint32 selection, CollisionTransform *descriptor);
}

#endif
