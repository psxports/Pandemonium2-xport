#ifndef ACTOR_LIFECYCLE_H
#define ACTOR_LIFECYCLE_H

#include <stddef.h>

#include "scene_path.h"

struct ActorModelDefinition;

// Section-owned placement with a behavior-selected state record
struct ActorPlacement
{
    PathDefinition path;
    // The owning operation selects ActorBinding or AnimatedPathState
    void *state;

    ActorModelDefinition *InitializeBinding();
    void CreateBoundActor();
    void ReleaseBoundActor();
    uint32 UpdateEvents();
};

typedef char ActorPlacementSize[(sizeof(ActorPlacement) == 48) ? 1 : -1];
typedef char ActorPlacementStateOffset[(offsetof(ActorPlacement, state) == 44) ? 1 : -1];

#endif
