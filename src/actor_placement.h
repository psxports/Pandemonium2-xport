#ifndef ACTOR_PLACEMENT_H
#define ACTOR_PLACEMENT_H
#include <stddef.h>

#include "psx.h"
#include "path_definition.h"

struct ActorModelDefinition;

struct ActorPlacement;
struct EncodedActorPlacement;

// Section-owned placement with a behavior-selected state record
struct ActorPlacement
{
    PathDefinition path;
    // The owning operation selects ActorBinding or AnimatedPathState
    void *state;

    ActorModelDefinition *InitializeBinding();
    void CreateBoundActor();
    void ReleaseBoundActor();
    static void EmptyActorLifecycleOperation(void);
    static uint32 LoadMonsterScriptEntry(void);
    uint32 UpdateEvents();
};

typedef char ActorPlacementSize[(sizeof(ActorPlacement) == 48) ? 1 : -1];

typedef char ActorPlacementStateOffset[(offsetof(ActorPlacement, state) == 44) ? 1 : -1];

struct EncodedActorPlacement
{
    EncodedPathDefinition path;
    uint32 state_offset;
};

typedef char EncodedActorPlacementSize[(sizeof(EncodedActorPlacement) == 48) ? 1 : -1];

#endif
