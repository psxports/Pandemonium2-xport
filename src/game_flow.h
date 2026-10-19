#ifndef GAME_FLOW_H
#define GAME_FLOW_H

struct GameProgress
{
    unsigned char selected_stage;
    unsigned char unlocked_stage;
    unsigned int hub_spawn_consumed;
};

struct GameFlowState
{
    unsigned int transition_flags;
    unsigned int mode_flags;
    unsigned int attract_phase;
    unsigned char requested_level;
    GameProgress progress;
};

extern GameFlowState game_flow;

int MapLevelProgression(unsigned int level, unsigned int mode);
void RunGame();
unsigned int CanOpenPauseMenu();

#endif
