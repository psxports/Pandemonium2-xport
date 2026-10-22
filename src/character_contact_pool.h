#ifndef CHARACTER_CONTACT_POOL_H
#define CHARACTER_CONTACT_POOL_H
#include <stddef.h>

#include "psx.h"

struct CharacterContactNode;
struct SceneNode;

struct SceneNode;

struct CharacterContactNode;
struct CharacterContactPool;

struct CharacterContactNode
{
    SceneNode *object;
    CharacterContactNode *next;
};

struct CharacterContactPool
{
    enum
    {
        Capacity = 32
    };

    CharacterContactNode nodes[Capacity];
    CharacterContactNode *free_head;
    CharacterContactNode *prev;
    CharacterContactNode *cur;

    static uint32 ResolveCharacterFormVariant(sint32 kind);
    static CharacterContactNode *FinishCharacterContacts(void);
    static void QueueCharacterPickup(SceneNode *object, uint32 context_offset);
    static CharacterContactNode *InitializeContactPool(void);
    static uint32 ContainsContact(const CharacterContactNode *head, const SceneNode *object);
    static CharacterContactNode *QueueContact(CharacterContactNode **head, SceneNode *object);
    static void ReleaseContacts(CharacterContactNode **head);
};

extern CharacterContactPool character_contact_pool;

#endif
