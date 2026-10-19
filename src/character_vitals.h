#ifndef CHARACTER_VITALS_H
#define CHARACTER_VITALS_H

#include "psx.h"

struct CharacterSettings
{
    enum Counter
    {
        Score,
        Health,
        Lives
    };

    uint8 flags;
    uint8 health_limit;
    uint32 counters[3];
    uint8 action_code;
    uint8 health_pickups;
};

struct CharacterVitals
{
    unsigned char lives;
    unsigned char health_limit;
    unsigned char health;
    unsigned int score;
    unsigned short damage_ticks;
    unsigned char health_pickups;
    uint8 damage_amount;
};

#endif
