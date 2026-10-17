#include "game.h"

SpritePoolState sprite_pool = {0, 0, 0};
unsigned int registered_object_head = 0;

uint32 p2_func_8002a144(uint32 object_offset, uint32 *source_head, uint32 *destination_head)
{
    FUNCTION_MARKER(0x8002A144, "SLUS_005.78");
    uint32 current = *source_head;
    uint32 previous = 0;
    uint32 result;
    if (object_offset != current)
    {
        do
        {
            previous = current;
            current = ((ObjectLinks *)psx_addr(previous, 4))->registry_next;
        } while (current != 0 && object_offset != current);
    }
    if (previous != 0)
    {
        result = ((ObjectLinks *)psx_addr(object_offset, 4))->registry_next;
        ((ObjectLinks *)psx_addr(previous, 4))->registry_next = result;
    }
    else
    {
        result = *source_head;
        if (object_offset == result)
        {
            result = ((ObjectLinks *)psx_addr(object_offset, 4))->registry_next;
            *source_head = result;
        }
    }
    if (destination_head != 0)
    {
        result = *destination_head;
        ((ObjectLinks *)psx_addr(object_offset, 4))->registry_next = result;
        *destination_head = object_offset;
    }
    return result;
}

unsigned int ObjectLinks::DetachChild(unsigned int object_offset)
{
    FUNCTION_MARKER(0x8002A210, "SLUS_005.78");
    uint32 result = first_child;
    uint32 node;
    if (result == 0)
        return result;
    node = result;
    if (object_offset == result)
    {
        result = ((ObjectLinks *)psx_addr(object_offset, 8))->sibling_next;
        first_child = result;
    }
    else
    {
        for (;;)
        {
            result = ((ObjectLinks *)psx_addr(node, 8))->sibling_next;
            if (result == object_offset)
            {
                result = ((ObjectLinks *)psx_addr(object_offset, 8))->sibling_next;
                ((ObjectLinks *)psx_addr(node, 8))->sibling_next = result;
                break;
            }
            node = ((ObjectLinks *)psx_addr(node, 8))->sibling_next;
            if (node == 0)
                break;
        }
    }
    ((ObjectLinks *)psx_addr(object_offset, 8))->sibling_next = 0;
    return result;
}
