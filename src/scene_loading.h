#ifndef SCENE_LOADING_H
#define SCENE_LOADING_H

#include "psx.h"
#include "scene_section.h"
#include "screen_render.h"

// Own the runtime header independently of its encoded resource buffer
struct SceneSectionBuffer
{
    SceneSection section;
    uint8 *resource_data;
    uint32 resource_bytes;
};

// Own one of the two original scene sequence staging buffers
struct SceneSequenceBundle
{
    uint8 *data;
    uint32 bytes;
};

struct SceneStream
{
    enum
    {
        SequenceBufferCount = 2
    };

    SceneSection *active_section;
    SceneSectionBuffer buffers[2];
    SceneSectionBuffer *current_buffer;
    SceneSectionBuffer *pending_buffer;
    SceneSection *previous_section;
    SceneSection *target_section;
    uint16 checkpoint;
    uint16 transition_ticks;
    SceneSequenceBundle *sequence_slots[SequenceBufferCount];
    SceneSequenceBundle sequence_bundles[SequenceBufferCount];
    BackgroundResource backgrounds[2];
    BackgroundResource *background_slots[2];
};

extern SceneStream scene_stream;

struct SceneLoader
{
    uint16 asset;
    SceneSectionBuffer *destination_buffer;
    uint8 *image_data;
    uint32 image_bytes;
    uint8 *geometry_data;
    uint32 geometry_bytes;
    BackgroundResource *background;
    SceneSequenceBundle *sequence_bundle;
    uint8 *materials_data;
    uint32 materials_bytes;
    sint8 pending_section;
    sint8 phase;
    sint8 texture_slot_base;
    uint8 loading_section;
    uint32 load_sequence;
};

extern SceneLoader scene_loader;

extern "C"
{
    void BeginSceneResourceLoad(SceneSectionBuffer *destination, uint32 asset);
    void StartSceneResourceLoad(void);
    uint32 PollSceneResourceLoad(void);
    uint32 QueueSceneSection(uint32 section);
    uint8 *GetSceneLoadDestination(void);
    uint32 RequestSceneSection(uint32 scene);
    void PrepareSceneSectionLoad(void);
    uint32 CountSceneTreasure(void);
    uint32 ResetSceneStreams(void);
    uint32 ActivateSceneSection(void);
    uint32 ApplySceneSectionTransition(void);
    void ExchangeSceneResourceBuffers(SceneSection *scene, const SceneSection *previous);
    uint32 HasPendingSceneSection(void);
    SceneSection *SwapSceneSections(void);
    uint32 SetSceneTransition(SceneSection *source, SceneSection *scene, sint32 index, uint32 flags);
}

void ReleaseSceneObjects();

#endif
