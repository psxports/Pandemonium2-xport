#ifndef CUTSCENE_H
#define CUTSCENE_H

#include "psx.h"

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
    uint32 path_offset;
    uint32 frame;
    uint32 trigger_contact;
    uint32 trigger_offset;
    uint32 next_camera;
};

extern CutsceneCamera cutscene_cameras[32];
extern CutscenePlayback cutscene;

extern "C"
{
uint32 ResetCutsceneCameras(void);
uint32 RegisterCutsceneCamera(uint32 object_offset);
uint32 QueueCutsceneTrigger(uint32 object_offset, uint32 contact);
void StopCutscene(void);
uint32 GetCutsceneMode(void);
uint32 StartPendingCutscene(uint32 object_offset);
uint32 UpdateCutscene(void);
uint32 HasCutsceneCameraName(const uint8 *name);
}

#endif
