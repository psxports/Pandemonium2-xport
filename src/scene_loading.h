#ifndef SCENE_LOADING_H
#define SCENE_LOADING_H

#include "psx.h"

struct SceneStream
{
    uint32 current_offset;
    uint32 pending_offset;
    uint32 previous_offset;
    uint32 target_offset;
    uint16 checkpoint;
    uint16 transition_ticks;
    uint32 sample_offsets[2];
    uint32 header_offsets[2];
};

extern SceneStream scene_streams[1];

struct SceneLoader
{
    uint16 asset;
    uint32 destination_offset;
    uint32 image_offset;
    uint32 sound_header_offset;
    uint32 sound_samples_offset;
    uint32 materials_offset;
    sint8 pending_section;
    sint8 phase;
};

struct WorldLevelName
{
    char text[40];
};

extern SceneLoader scene_loader;
extern const WorldLevelName *world_level_names;

extern "C"
{
uint32 BeginSceneResourceLoad(uint32 destination_offset, uint32 asset);
uint32 StartSceneResourceLoad(void);
uint32 PollSceneResourceLoad(void);
uint32 QueueSceneSection(uint32 section);
uint32 GetSceneLoadDestination(uint32 mode);
uint32 RequestSceneSection(uint32 scene, uint32 mode);
uint32 PrepareSceneSectionLoad(void);
uint32 CountSceneTreasure(uint32 player);
uint32 ResetSceneStreams(void);
uint32 ActivateSceneSection(uint32 mode);
uint32 ApplySceneSectionTransition(uint32 mode);
void ExchangeSceneAudioBuffers(uint32 scene_offset, uint32 previous_offset, uint32 mode);
uint32 HasPendingSceneSection(uint32 index);
uint32 SwapSceneSections(uint32 index);
uint32 SetSceneTransition(uint32 slot, uint32 source_offset, uint32 scene_offset, sint32 index, uint32 flags);
}

#endif
