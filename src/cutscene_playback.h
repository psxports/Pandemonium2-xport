#ifndef CUTSCENE_PLAYBACK_H
#define CUTSCENE_PLAYBACK_H
#include <stddef.h>

#include "psx.h"

struct AnimatedObject;
struct ScenePath;
struct SceneTrigger;

struct CutsceneCamera;
struct CutscenePlayback;

struct CutsceneCamera
{
    uint32 flags;
    uint32 duration;
    uint32 camera_id;
    uint32 next_camera;
};

struct CutscenePlayback
{
    uint32 flags;
    uint32 active;
    uint32 remaining_ticks;
    uint32 duration;
    const ScenePath *path;
    uint32 frame;
    uint32 trigger_contact;
    SceneTrigger *trigger;
    uint32 next_camera;

    static void RegisterCutsceneCamera(const SceneTrigger *object);
    static void StopCutscene(void);
    static uint32 GetCutsceneMode(void);
    static void QueueCutsceneTrigger(SceneTrigger *object, uint32 contact);
    static void StartPendingCutscene(const AnimatedObject *object);
    static uint32 UpdateCutscene(void);
    static void ResetCutsceneCameras(void);
    static uint32 HasCutsceneCameraName(const uint8 *name);
};

extern CutsceneCamera cutscene_cameras[32];

extern CutscenePlayback cutscene;

#endif
