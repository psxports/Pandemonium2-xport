#ifndef CHARACTER_CONTACTS_H
#define CHARACTER_CONTACTS_H

#include "psx.h"

struct SceneNode;

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
};

extern CharacterContactPool character_contact_pool;

CharacterContactNode *InitializeContactPool(void);
uint32 ContainsContact(const CharacterContactNode *head, const SceneNode *object);
CharacterContactNode *QueueContact(CharacterContactNode **head, SceneNode *object);
void ReleaseContacts(CharacterContactNode **head);
CharacterContactNode *FinishCharacterContacts(void);

#endif
