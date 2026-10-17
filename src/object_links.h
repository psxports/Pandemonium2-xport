#ifndef OBJECT_LINKS_H
#define OBJECT_LINKS_H

#include <stddef.h>

struct ObjectLinks
{
    unsigned int registry_next;
    unsigned int sibling_next;
    unsigned int first_child;

    unsigned int DetachChild(unsigned int object_offset);
};

typedef char ObjectLinksSize[(sizeof(ObjectLinks) == 12) ? 1 : -1];
typedef char ObjectLinksSiblingOffset[(offsetof(ObjectLinks, sibling_next) == 4) ? 1 : -1];
typedef char ObjectLinksChildOffset[(offsetof(ObjectLinks, first_child) == 8) ? 1 : -1];

struct SpritePoolState
{
    unsigned int first;
    unsigned int free_head;
    unsigned int active_head;
};

extern SpritePoolState sprite_pool;
extern unsigned int registered_object_head;

#endif
