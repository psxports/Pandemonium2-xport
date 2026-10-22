#ifndef LEVEL_LOADER_H
#define LEVEL_LOADER_H
#include <stddef.h>

#include "psx.h"

struct LevelDefinition;

struct LevelLoader;

struct LevelLoader
{
    const LevelDefinition *definition;
    sint8 phase;
    uint8 *transfer_data;
    uint32 transfer_bytes;

    static uint32 PollLevelLoad(sint32 level);
    static void LoadLevel(sint32 level);
    static sint32 PollLevelLoadResult(sint32 level);
    static void UnloadLevelResources(void);
};

extern LevelLoader level_loader;

#endif
