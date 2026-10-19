#ifndef OBJECT_SPEED_H
#define OBJECT_SPEED_H

#include "psx.h"

struct ObjectSpeedAction
{
    uint32 action;
    uint32 speed_delta;
};

struct ObjectSpeedDefinition
{
    uint32 kind;
    uint32 min_speed;
    uint32 max_speed;
    uint32 default_speed;
    uint32 wrap_center;
    uint32 wrap_step;
    uint16 action_count;
    uint16 metadata_halfword;
    uint32 metadata_word;
    ObjectSpeedAction actions[5];
};

struct ObjectSpeedDefinitions
{
    uint32 count;
    ObjectSpeedDefinition records[16];
};

extern ObjectSpeedDefinitions object_speed_definitions;

extern "C"
{
    uint32 LoadObjectSpeedDefinitions(const uint8 *bundle);
    const ObjectSpeedDefinition *FindObjectSpeedDefinition(uint32 descriptor);
}

#endif
