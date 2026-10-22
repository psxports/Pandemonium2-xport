#ifndef SCENE_STREAM_H
#define SCENE_STREAM_H
#include <stddef.h>

#include "psx.h"
#include "background_resource.h"
#include "scene_section.h"

struct SceneSectionBuffer;
struct SceneSequenceBundle;
struct SceneStream;

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
    SceneSectionBuffer *cur_buffer;
    SceneSectionBuffer *pending_buffer;
    SceneSection *prev_section;
    SceneSection *target_section;
    uint16 checkpoint;
    uint16 transition_ticks;
    SceneSequenceBundle *sequence_slots[SequenceBufferCount];
    SceneSequenceBundle sequence_bundles[SequenceBufferCount];
    BackgroundResource backgrounds[2];
    BackgroundResource *background_slots[2];

    static void SetSceneTransition(SceneSection *src, SceneSection *scene, sint32 idx, uint32 flags);
    static uint32 HasPendingSceneSection(void);
    static SceneSection *SwapSceneSections(void);
    static uint32 ActivateSceneSection(void);
    static void ReleaseSceneObjects(void);
    static void ExchangeSceneResourceBuffers(SceneSection *scene, const SceneSection *prev);
    static void ApplySceneSectionTransition(void);
    static void ResetSceneStreams(void);
    static void EmptyStreamingOperation(void);
};

extern SceneStream scene_stream;

#endif
