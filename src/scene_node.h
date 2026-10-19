#ifndef SCENE_NODE_H
#define SCENE_NODE_H

#include "psx.h"
#include "object_links.h"

struct AnimatedObject;
struct SpriteRecord;
struct MaterialResourceHeader;
struct ModelDecodeLayout;

struct SceneNode
{
    ObjectLinks links;
    uint8 kind;
    uint8 flags;
    uint16 state_flags;

    void MoveBetweenLists(SceneNode **source_head, SceneNode **destination_head);
    void Register();
    void Unregister();

    uint32 DrawBillboard() const;
    void DrawChildren(AnimatedObject *owner);
    uint32 ResetPlayback(const SceneNode *parent);
    void RelocateResourceLinks(const ModelDecodeLayout *layout, const MaterialResourceHeader *resource, const SpriteRecord *default_sprite);
    void RelocateClone(uint8 *destination, const uint8 *source);
};

typedef char SceneNodeSize[(sizeof(SceneNode) == 16) ? 1 : -1];
typedef char SceneNodeKindOffset[(offsetof(SceneNode, kind) == 12) ? 1 : -1];

AnimatedObject *CloneObjectResource(const AnimatedObject *source);

void ReleaseSceneObject(SceneNode *object);

void ApplyNodeAnimation(AnimatedObject *parent, SceneNode *node);

#endif
