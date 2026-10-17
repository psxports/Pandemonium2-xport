#ifndef PLAYER_INTERACTION_H
#define PLAYER_INTERACTION_H

struct PlayerInteractionState
{
    unsigned int flags;
    unsigned int target_offset;
};

typedef char PlayerInteractionStateSize[(sizeof(PlayerInteractionState) == 8) ? 1 : -1];

extern PlayerInteractionState player_interaction;

#endif
