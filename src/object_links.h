#ifndef OBJECT_LINKS_H
#define OBJECT_LINKS_H
#include <stddef.h>

#include "psx.h"

struct AnimatedObject;
struct SceneNode;

struct ObjectLinks;

struct ObjectLinks
{
    union
    {
        SceneNode *registry_next;
        uint32 src_registry_word;
    };

    SceneNode *sibling_next;
    SceneNode *first_child;

    void UpdateGraph(AnimatedObject *parent, uint32 mode, bool advance = true);
    void DetachChild(SceneNode *object);
};

typedef char ObjectLinksSize[(sizeof(ObjectLinks) == 12) ? 1 : -1];

typedef char ObjectLinksSiblingOffset[(offsetof(ObjectLinks, sibling_next) == 4) ? 1 : -1];

typedef char ObjectLinksChildOffset[(offsetof(ObjectLinks, first_child) == 8) ? 1 : -1];

#endif
