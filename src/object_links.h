#ifndef OBJECT_LINKS_H
#define OBJECT_LINKS_H

#include <stddef.h>

#include "psx.h"

struct SceneNode;
struct SpriteRecord;
struct AnimatedObject;

struct ObjectLinks
{
    union
    {
        SceneNode *registry_next;
        uint32 source_registry_word;
    };

    SceneNode *sibling_next;
    SceneNode *first_child;

    void DetachChild(SceneNode *object);
    void UpdateGraph(AnimatedObject *parent, uint32 mode);
};

typedef char ObjectLinksSize[(sizeof(ObjectLinks) == 12) ? 1 : -1];
typedef char ObjectLinksSiblingOffset[(offsetof(ObjectLinks, sibling_next) == 4) ? 1 : -1];
typedef char ObjectLinksChildOffset[(offsetof(ObjectLinks, first_child) == 8) ? 1 : -1];

// Own pooled material playback separately from borrowed graph records
struct SpritePoolEntry
{
    SceneNode *node;
    uint32 animation_words[4];
};

struct SpritePoolState
{
    enum
    {
        EntryCount = 100,
        AnimationCapacity = 3
    };

    SceneNode *first;
    SceneNode *free_head;
    SceneNode *active_head;
    SpriteRecord *records;
    SpritePoolEntry entries[EntryCount];
};

extern SpritePoolState sprite_pool;
extern SceneNode *registered_object_head;

struct SceneNode;

#endif
