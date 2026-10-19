#ifndef PLAYER_INTERACTION_H
#define PLAYER_INTERACTION_H

#include "psx.h"

struct ActorBinding;

struct PlayerInteractionState
{
    unsigned int flags;
    ActorBinding *target;
    VECTOR position;
};

extern PlayerInteractionState player_interaction;

#endif
