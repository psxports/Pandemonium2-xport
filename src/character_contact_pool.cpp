#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "character_contact_pool.h"
#include "game.h"
#include "animated_object.h"
#include "psx_gte.h"

CharacterContactPool character_contact_pool = {0};

uint32 CharacterContactPool::ResolveCharacterFormVariant(sint32 kind)
{
    FUNCTION_MARKER(0x800473F0, "SLUS_005.78");
    uint32 cur;
    uint32 compatible;
    if (kind != 6 && kind != 7 && kind != 8 && kind != 13)
        return kind;
    cur = (character_pad.motion.form_flags >> 16) & 15;
    compatible = cur == 0 || cur == 6 || cur == 8 || cur == 13 || cur == 10;
    switch (kind)
    {
        case 6:
            return compatible ? 6 : 7;
        case 7:
            return compatible ? 8 : 9;
        case 8:
            return compatible ? 10 : 11;
        default:
            return compatible ? 13 : 14;
    }
}

CharacterContactNode *CharacterContactPool::FinishCharacterContacts(void)
{
    FUNCTION_MARKER(0x80047C1C, "SLUS_005.78");
    CharacterContactNode *node, *result, *val;
    SceneNode *object;
    sint32 kind;
    for (node = character_contact_pool.prev; node != 0; node = node->next)
    {
        val = character_contact_pool.cur;
        object = node->object;
        if (CharacterContactPool::ContainsContact(val, object) == 0)
        {
            object = node->object;
            kind = (sint8)(object->state_flags >> 8);
            if (kind == 53)
                SceneEvents::QueueSceneEvent((((const SceneTrigger *)object)->parameters.event.command & 0xFFF) | 0x4000, 4);
            else if (kind == 54)
                SceneEvents::QueueSceneEvent((((const SceneTrigger *)object)->parameters.event.command & 0xFFF) | 0x3000, 4);
        }
    }
    CharacterContactPool::ReleaseContacts(&character_contact_pool.prev);
    result = character_contact_pool.cur;
    character_contact_pool.cur = 0;
    character_contact_pool.prev = result;
    return result;
}

void CharacterContactPool::QueueCharacterPickup(SceneNode *object, uint32 context_offset)
{
    FUNCTION_MARKER(0x80047D18, "SLUS_005.78");
    sint32 idx = character_pad.pickup_count;
    if (idx < 12)
    {
        uint8 old_count;
        character_pad.pickup_objects[idx] = object;
        old_count = character_pad.pickup_count;
        character_pad.pickup_count = old_count + 1;
        character_pad.pickup_fractions[(sint8)old_count] = context_offset;
    }
}

CharacterContactNode *CharacterContactPool::InitializeContactPool(void)
{
    FUNCTION_MARKER(0x8005BA34, "SLUS_005.78");
    CharacterContactNode *pointer = character_contact_pool.nodes + CharacterContactPool::Capacity - 1;
    sint32 idx;
    for (idx = CharacterContactPool::Capacity - 2; idx >= 0; --idx)
    {
        character_contact_pool.nodes[idx].next = pointer;
        --pointer;
    }
    character_contact_pool.nodes[CharacterContactPool::Capacity - 1].next = 0;
    character_contact_pool.free_head = character_contact_pool.nodes;
    return character_contact_pool.nodes;
}

uint32 CharacterContactPool::ContainsContact(const CharacterContactNode *head, const SceneNode *object)
{
    FUNCTION_MARKER(0x8005BAAC, "SLUS_005.78");
    while (head != 0)
    {
        if (head->object == object)
            return 1;
        head = head->next;
    }
    return 0;
}

CharacterContactNode *CharacterContactPool::QueueContact(CharacterContactNode **head, SceneNode *object)
{
    FUNCTION_MARKER(0x8005BADC, "SLUS_005.78");
    CharacterContactNode *node = character_contact_pool.free_head;
    CharacterContactNode *result;
    if (node == 0)
        return *head;
    character_contact_pool.free_head = node->next;
    node->object = object;
    result = *head;
    node->next = result;
    *head = node;
    return result;
}

void CharacterContactPool::ReleaseContacts(CharacterContactNode **head)
{
    FUNCTION_MARKER(0x8005BB14, "SLUS_005.78");
    CharacterContactNode *node = *head;
    CharacterContactNode *next, *free_head;
    while (node != 0)
    {
        next = node->next;
        free_head = character_contact_pool.free_head;
        character_contact_pool.free_head = node;
        node->next = free_head;
        node = next;
    }
    *head = 0;
}
