#ifndef SCENE_NODE_H
#define SCENE_NODE_H
#include <stddef.h>

#include "psx.h"
#include "object_links.h"

struct CollisionTransform;
struct MeshFace;

struct AnimatedObject;
struct MaterialCatalog;
struct ModelDecodeLayout;
struct SpriteRecord;

struct EncodedSceneNode;
struct SceneObjectName;
struct SceneNode;

struct EncodedSceneNode
{
    uint32 registry_offset;
    uint32 sibling_offset;
    uint32 child_offset;
    uint8 kind;
    uint8 flags;
    uint16 state_flags;
};

typedef char EncodedSceneNodeSize[(sizeof(EncodedSceneNode) == 16) ? 1 : -1];

struct SceneObjectName
{
    char prefix[2];
    uint8 kind;
};

typedef char SceneObjectNameBinaryLayout[(sizeof(SceneObjectName) == 3) ? 1 : -1];

extern SceneNode *registered_object_head;

struct SceneNode
{
    ObjectLinks links;
    uint8 kind;
    uint8 flags;
    uint16 state_flags;

    void DrawChildren(AnimatedObject *owner);
    uint32 ResetPlayback(const SceneNode *parent);
    static const MeshFace *SelectColFace(const SceneNode *object, uint32 selection, CollisionTransform *desc);
    static void ReleaseSceneObject(SceneNode *object);
    void MoveBetweenLists(SceneNode **src_head, SceneNode **dst_head);
    void RelocateClone(uint8 *dst, const uint8 *src);
    void Register();
    void Unregister();
    void DrawBillboard() const;
    void RelocateResourceLinks(const ModelDecodeLayout *layout, const MaterialCatalog *resource, const SpriteRecord *default_spr);
};

typedef char SceneNodeSize[(sizeof(SceneNode) == 16) ? 1 : -1];

typedef char SceneNodeKindOffset[(offsetof(SceneNode, kind) == 12) ? 1 : -1];

#endif
