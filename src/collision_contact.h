#ifndef COLLISION_CONTACT_H
#define COLLISION_CONTACT_H

#include <stddef.h>

typedef struct CollisionContact
{
    unsigned int selection;
    unsigned int fraction;
    unsigned int other_selection;
    unsigned int flags;
} CollisionContact;

typedef char CollisionContactSize[(sizeof(CollisionContact) == 16) ? 1 : -1];
typedef char CollisionContactFractionOffset[(offsetof(CollisionContact, fraction) == 4) ? 1 : -1];
typedef char CollisionContactFlagsOffset[(offsetof(CollisionContact, flags) == 12) ? 1 : -1];

extern CollisionContact collision_contact;

#endif
