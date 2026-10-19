#ifndef OBJECT_OPERATIONS_H
#define OBJECT_OPERATIONS_H
#include "collision_contact.h"
#include "animated_object.h"

struct ObjectCallback
{
    typedef void (*PlainHandler)();
    typedef unsigned int (AnimatedObject::*MemberHandler)();
    typedef AnimatedObject *(*FactoryHandler)(AnimatedObject *);
    typedef void (AnimatedObject::*VoidMemberHandler)();
    PlainHandler plain_handler;
    MemberHandler member_handler;
    FactoryHandler factory_handler;
    VoidMemberHandler void_member_handler;
};

typedef unsigned int (AnimatedObject::*CollisionHandler)(SceneNode *, CollisionContact *);

struct ObjectOperations
{
    ObjectCallback behaviors[34];
    ObjectCallback motions[7];
    ObjectCallback variants[5];
    CollisionHandler collisions[5];
};

extern ObjectOperations object_operations;
#endif
