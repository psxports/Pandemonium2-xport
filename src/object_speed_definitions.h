#ifndef OBJECT_SPEED_DEFINITIONS_H
#define OBJECT_SPEED_DEFINITIONS_H
#include <stddef.h>

#include "psx.h"

struct ObjectSpeedDefinition;

struct ObjectSpeedAction;
struct ObjectSpeedDefinition;
struct ObjectSpeedDefinitions;

struct ObjectSpeedAction
{
    uint32 action;
    sint32 speed_delta;
};

struct ObjectSpeedDefinition
{
    uint32 kind;
    sint32 min_speed;
    sint32 max_speed;
    sint32 default_speed;
    sint32 wrap_center;
    sint32 wrap_step;
    uint16 action_count;
    uint16 metadata_halfword;
    uint32 metadata_word;
    ObjectSpeedAction actions[5];
};

struct ObjectSpeedDefinitions
{
    uint32 count;
    ObjectSpeedDefinition recs[16];

    static const ObjectSpeedDefinition *FindObjectSpeedDefinition(uint32 desc);
    static void LoadObjectSpeedDefinitions(const uint8 *bundle);
};

extern ObjectSpeedDefinitions object_speed_definitions;

#endif
