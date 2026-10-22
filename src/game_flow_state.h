#ifndef GAME_FLOW_STATE_H
#define GAME_FLOW_STATE_H
#include <stddef.h>

#include "psx.h"

struct GameProgress;
struct GameFlowState;

struct GameProgress
{
    uint8 sel_stage;
    uint8 unlocked_stage;
    uint32 hub_spawn_consumed;
};

struct GameFlowState
{
    uint32 transition_flags;
    uint32 mode_flags;
    uint32 attract_phase;
    uint8 requested_level;
    GameProgress progress;

    static sint32 MapLevelProgression(uint32 level, uint32 mode);
    static uint32 PlayMovie(const char *path);
    static uint32 CanOpenPauseMenu(void);
    static void RunGame(void);
};

extern GameFlowState game_flow;

#endif
