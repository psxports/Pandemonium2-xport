#ifndef LEVEL_RESOURCE_H
#define LEVEL_RESOURCE_H
#include <stddef.h>

#include "psx.h"

struct WorldLevelName;
struct SectionDefinition;
struct EncodedLevelHeader;
struct LevelResource;

struct WorldLevelName
{
    char text[40];
};

struct SectionDefinition
{
    uint8 flags;
    uint8 respawn_form;
    uint16 transition_ticks;
    uint32 checkpoint;
    // Camera range and fog depth interval
    uint32 far_distance;
    uint32 fog_near;
    uint32 fog_far;
    uint32 metadata_word;
};

struct EncodedLevelHeader
{
    SectionDefinition sections[10];
    uint32 asset_base;
    uint32 actor_masks[2];
    uint32 animated_model_mask;
    uint32 model_grp_mask;
    uint16 character_mask;
    uint16 level_id;
    uint8 active_sections;
    uint8 section_count;
    uint16 checkpoint;
    uint16 transition_ticks;
    uint16 treasure_total;
    uint16 collected_total;
    uint16 metadata_halfword;
    uint32 audio_mode;
};

typedef char EncodedLevelHeaderSize[(sizeof(EncodedLevelHeader) == 280) ? 1 : -1];

// Runtime level state is decoded once when the level resource is loaded
struct LevelResource
{
    enum
    {
        WorldNameCount = 30
    };

    SectionDefinition sections[10];
    uint32 asset_base;
    uint32 actor_masks[2];
    uint32 animated_model_mask;
    uint32 model_grp_mask;
    uint16 character_mask;
    uint16 level_id;
    uint8 active_sections;
    uint8 section_count;
    uint16 checkpoint;
    uint16 transition_ticks;
    uint16 treasure_total;
    uint16 collected_total;
    uint16 metadata_halfword;
    uint32 audio_mode;
    WorldLevelName world_names[WorldNameCount];

    void LoadModelGrps() const;
    void LoadActorModels() const;
    void LoadAnimatedModels() const;
    void LoadCharacterModels() const;
};

extern LevelResource level_resource;

#endif
