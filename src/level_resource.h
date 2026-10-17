#ifndef LEVEL_RESOURCE_H
#define LEVEL_RESOURCE_H

#include <stddef.h>

#include "psx.h"

struct LevelDefinition;

struct SectionDefinition
{
    uint8 flags;
    uint8 metadata;
    uint16 transition_ticks;
    uint32 checkpoint;
    uint32 origin[3];
    uint32 metadata_word;
};

// This prefix views the existing mutable level resource buffer
struct LevelResource
{
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

    void LoadAnimatedModels() const;
    void LoadModelGroups() const;
    uint32 LoadActorModels() const;
    uint32 LoadCharacterModels() const;
};

typedef char SectionDefinitionBinaryLayout[(sizeof(SectionDefinition) == 24) ? 1 : -1];
typedef char LevelResourcePrefixLayout[(sizeof(LevelResource) == 280) ? 1 : -1];
typedef char LevelResourceAssetOffset[(offsetof(LevelResource, asset_base) == 240) ? 1 : -1];
typedef char LevelResourceIdOffset[(offsetof(LevelResource, level_id) == 262) ? 1 : -1];

struct LevelLoader
{
    const LevelDefinition *definition;
    sint8 phase;
};

extern LevelLoader level_loader;

extern "C"
{
uint32 LoadLevel(sint32 level, uint32 mode);
uint32 PollLevelLoad(sint32 level, uint32 mode);
sint32 PollLevelLoadResult(sint32 level, uint32 mode);
}

#endif
