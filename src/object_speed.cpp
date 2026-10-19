#include "game.h"

ObjectSpeedDefinitions object_speed_definitions = {0};

uint32 LoadObjectSpeedDefinitions(const uint8 *bundle)
{
    uint32 asset = 4;
    uint32 result;
    uint32 offset;
    uint32 index;
    uint32 next;
    const uint8 *bytes;
    ObjectSpeedDefinition *definition;
    uint32 action;
    FUNCTION_MARKER(0x8003FD28, "SLUS_005.78");
    do
    {
        result = resource_archive.Size(asset);
        if (result != 72)
            break;
        ++asset;
        offset = resource_archive.Offset(resource_load_cursor.asset);
        index = object_speed_definitions.count;
        next = resource_load_cursor.asset + 1;
        object_speed_definitions.count = index + 1;
        bytes = bundle + offset;
        definition = &object_speed_definitions.records[index];
        definition->kind = xport_load_le32(bytes);
        definition->min_speed = xport_load_le32(bytes + 4);
        definition->max_speed = xport_load_le32(bytes + 8);
        definition->default_speed = xport_load_le32(bytes + 12);
        definition->wrap_center = xport_load_le32(bytes + 16);
        definition->wrap_step = xport_load_le32(bytes + 20);
        definition->action_count = xport_load_le16(bytes + 24);
        definition->metadata_halfword = xport_load_le16(bytes + 26);
        definition->metadata_word = xport_load_le32(bytes + 28);
        for (action = 0; action < 5; ++action)
        {
            definition->actions[action].action = xport_load_le32(bytes + 32 + action * 8);
            definition->actions[action].speed_delta = xport_load_le32(bytes + 36 + action * 8);
        }
        resource_load_cursor.asset = next;
        result = asset < 150;
    } while (asset < 150);
    return result;
}

const ObjectSpeedDefinition *FindObjectSpeedDefinition(uint32 descriptor)
{
    FUNCTION_MARKER(0x8003FB58, "SLUS_005.78");
    sint32 count = (sint32)object_speed_definitions.count;
    sint32 index;
    for (index = 0; index < count; ++index)
    {
        const ObjectSpeedDefinition *entry = &object_speed_definitions.records[index];
        if (entry->kind == descriptor)
            return entry;
    }
    return 0;
}

uint32 AnimatedObject::WrapObjectSpeed()
{
    FUNCTION_MARKER(0x8003FBB0, "SLUS_005.78");
    sint32 count = (sint32)object_speed_definitions.count;
    uint32 descriptor = this->state;
    const ObjectSpeedDefinition *entry = 0;
    uint32 center;
    uint32 extent;
    uint32 position;
    uint32 result;
    sint32 index;
    for (index = 0; index < count; ++index)
    {
        const ObjectSpeedDefinition *candidate = &object_speed_definitions.records[index];
        if (candidate->kind == descriptor)
        {
            entry = candidate;
            break;
        }
    }
    center = entry->wrap_center;
    extent = entry->wrap_step;
    position = this->velocity[0];
    result = position - extent;
    if ((sint32)(center + extent) < (sint32)position)
        this->velocity[0] = result;
    else
    {
        result = position + extent;
        if ((sint32)position < (sint32)(center - extent))
            this->velocity[0] = result;
    }
    return result;
}

uint32 AnimatedObject::ApplyObjectSpeedAction(uint32 action)
{
    FUNCTION_MARKER(0x8003FC44, "SLUS_005.78");
    sint32 count = (sint32)object_speed_definitions.count;
    uint32 descriptor = this->state;
    const ObjectSpeedDefinition *entry = 0;
    sint32 index;
    uint32 result;
    for (index = 0; index < count; ++index)
    {
        const ObjectSpeedDefinition *candidate = &object_speed_definitions.records[index];
        if (candidate->kind == descriptor)
        {
            entry = candidate;
            break;
        }
    }
    result = entry->action_count;
    if (result != 0)
    {
        const ObjectSpeedAction *slot = entry->actions;
        uint32 position = 0;
        do
        {
            uint32 found = slot->action;
            ++position;
            if (action == found)
            {
                uint32 value = this->velocity[0] + slot->speed_delta;
                uint32 limit;
                this->velocity[0] = value;
                limit = entry->max_speed;
                if ((sint32)limit < (sint32)value)
                    this->velocity[0] = limit;
                value = this->velocity[0];
                limit = entry->min_speed;
                result = (sint32)value < (sint32)limit;
                if (result != 0)
                    this->velocity[0] = limit;
                return result;
            }
            result = position < entry->action_count;
            ++slot;
        } while (result != 0);
    }
    return result;
}

uint32 AnimatedObject::ResolveObjectSpeed() const
{
    FUNCTION_MARKER(0x800606FC, "SLUS_005.78");
    const ObjectSpeedDefinition *definition = FindObjectSpeedDefinition(this->state);
    const SceneNode *child_node = this->node.links.first_child;
    uint32 result = 0;
    while (child_node != 0)
    {
        if (child_node->kind == 0)
        {
            const AnimatedObject *child_object = (const AnimatedObject *)child_node;
            if ((child_object->path_flags & 0x2000) != 0)
            {
                uint32 position = child_object->velocity[0];
                if (position != 0)
                {
                    result = position;
                    break;
                }
                position = child_object->ResolveObjectSpeed();
                if (position != child_object->velocity[0])
                {
                    result = position;
                    break;
                }
            }
        }
        child_node = child_node->links.sibling_next;
    }
    return result != 0 ? result : definition->default_speed;
}
