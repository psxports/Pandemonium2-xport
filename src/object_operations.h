#ifndef OBJECT_OPERATIONS_H
#define OBJECT_OPERATIONS_H
#include "collision_contact.h"
struct ObjectCallback
{
    typedef unsigned int (*ValueHandler)(unsigned int);
    typedef void (*VoidHandler)(unsigned int);
    typedef void (*PlainHandler)();
    ValueHandler value_handler;
    VoidHandler void_handler;
    PlainHandler plain_handler;
};
typedef unsigned int (*CollisionHandler)(unsigned int, unsigned int, CollisionContact *);
struct ObjectOperations
{
    ObjectCallback behaviors[34];
    ObjectCallback motions[7];
    ObjectCallback variants[5];
    CollisionHandler collisions[5];
};
extern ObjectOperations object_operations;
ObjectCallback::ValueHandler DecodeObjectFactory(unsigned int original_address);
#endif
