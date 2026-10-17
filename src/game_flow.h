#ifndef GAME_FLOW_H
#define GAME_FLOW_H

struct GameFlowState
{
    unsigned int transition_flags;
    unsigned int mode_flags;
};

typedef char GameFlowStateSize[(sizeof(GameFlowState) == 8) ? 1 : -1];

extern GameFlowState game_flow;

#endif
