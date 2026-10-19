#ifndef LEVEL_RESOURCE_H
#define LEVEL_RESOURCE_H

#include "psx.h"

struct LevelDefinition;

struct WorldLevelName
{
    char text[40];
};

struct SectionDefinition
{
    uint8 flags;
    uint8 metadata;
    uint16 transition_ticks;
    uint32 checkpoint;
    uint32 origin[3];
    uint32 metadata_word;
};

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
    uint32 model_group_mask;
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

    void LoadAnimatedModels() const;
    void LoadModelGroups() const;
    uint32 LoadActorModels() const;
    uint32 LoadCharacterModels() const;
};

struct LevelLoader
{
    const LevelDefinition *definition;
    sint8 phase;
    uint8 *transfer_data;
    uint32 transfer_bytes;
};

extern LevelLoader level_loader;
extern LevelResource level_resource;

extern "C"
{
    uint32 LoadLevel(sint32 level);
    uint32 PollLevelLoad(sint32 level);
    sint32 PollLevelLoadResult(sint32 level);
}

uint32 UnloadLevelResources();

#endif
