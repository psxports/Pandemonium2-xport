#include "game.h"

SpritePoolState sprite_pool = {0, 0, 0};
SceneNode *registered_object_head = 0;

void SceneNode::MoveBetweenLists(SceneNode **source_head, SceneNode **destination_head)
{
    FUNCTION_MARKER(0x8002A144, "SLUS_005.78");
    SceneNode *current, *previous = 0, *next;
    current = *source_head;
    if (this != current)
    {
        do
        {
            previous = current;
            current = previous->links.registry_next;
        } while (current != 0 && this != current);
    }
    if (previous != 0)
    {
        next = links.registry_next;
        previous->links.registry_next = next;
    }
    else if (this == *source_head)
        *source_head = links.registry_next;
    if (destination_head != 0)
    {
        current = *destination_head;
        links.registry_next = current;
        *destination_head = this;
    }
}

void ObjectLinks::DetachChild(SceneNode *object)
{
    FUNCTION_MARKER(0x8002A210, "SLUS_005.78");
    SceneNode *next, *node;
    SceneNode *child = object;
    node = first_child;
    if (node == 0)
        return;
    if (object == node)
    {
        next = child->links.sibling_next;
        first_child = next;
    }
    else
    {
        for (;;)
        {
            next = node->links.sibling_next;
            if (next == object)
            {
                next = child->links.sibling_next;
                node->links.sibling_next = next;
                break;
            }
            node = next;
            if (node == 0)
                break;
        }
    }
    child->links.sibling_next = 0;
}

void SceneNode::Register()
{
    SceneNode *previous;
    FUNCTION_MARKER(0x800401A8, "SLUS_005.78");
    flags &= 0x7F;
    previous = registered_object_head;
    registered_object_head = this;
    links.registry_next = previous;
}

void SceneNode::Unregister()
{
    FUNCTION_MARKER(0x800401C8, "SLUS_005.78");
    MoveBetweenLists(&registered_object_head, 0);
}
