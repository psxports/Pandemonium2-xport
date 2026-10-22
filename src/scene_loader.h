#ifndef SCENE_LOADER_H
#define SCENE_LOADER_H
#include <stddef.h>

#include "psx.h"

struct BackgroundResource;
struct SceneSectionBuffer;
struct SceneSequenceBundle;

struct SceneLoader;

struct SceneLoader
{
    uint16 asset;
    SceneSectionBuffer *dst_buffer;
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
    sint8 tex_slot_base;
    uint8 loading_section;
    uint32 load_sequence;

    static uint32 CountSceneTreasure(void);
    static void PrepareSceneSectionLoad(void);
    static uint8 *GetSceneLoadDst(void);
    static uint32 RequestSceneSection(uint32 scene);
    static void WaitSceneResourceLoad(void);
    static void StartSceneResourceLoad(void);
    static void QueueSceneSection(uint32 scene);
    static uint32 PollSceneResourceLoad(void);
    static void BeginSceneResourceLoad(SceneSectionBuffer *dst, uint32 asset);
};

extern SceneLoader scene_loader;

#endif
