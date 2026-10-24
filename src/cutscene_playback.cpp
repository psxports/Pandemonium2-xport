#include <string.h>

#include "cutscene_playback.h"
#include "game.h"
#include "hd.h"

CutsceneCamera cutscene_cameras[32] = {{0}};

CutscenePlayback cutscene = {0};

void CutscenePlayback::RegisterCutsceneCamera(const SceneTrigger *object)
{
    FUNCTION_MARKER(0x80026B6C, "SLUS_005.78");

    sint32 idx;
    idx = object->cutscene_control.idx;
    if (idx < 0 || idx >= 32)
    {
        PrintSdkDiagnostic("Invalid cut scene idx %d. camera %08x\n", idx, object->parameters.cutscene.camera_id);
        return;
    }
    cutscene_cameras[idx].camera_id = object->parameters.cutscene.camera_id;
    cutscene_cameras[idx].next_camera = object->parameters.cutscene.next_camera;
    cutscene_cameras[idx].duration = object->parameters.cutscene.duration;
    cutscene_cameras[idx].flags = object->cutscene_control.flags;
}

void CutscenePlayback::StopCutscene(void)
{
    FUNCTION_MARKER(0x80026BF4, "SLUS_005.78");
    if (cutscene.remaining_ticks != 0 || cutscene.active != 0)
        hd_camera_cut();
    cutscene.remaining_ticks = 0;
    cutscene.active = 0;
}

uint32 CutscenePlayback::GetCutsceneMode(void)
{
    FUNCTION_MARKER(0x80026C14, "SLUS_005.78");
    if ((sint32)cutscene.remaining_ticks <= 0)
        return 0;
    if ((cutscene.flags & 1) != 0)
        return 1;
    return 0xFFFFFFFF;
}

void CutscenePlayback::QueueCutsceneTrigger(SceneTrigger *object, uint32 contact)
{
    FUNCTION_MARKER(0x80026C54, "SLUS_005.78");
    cutscene.active = 1;
    cutscene.trigger = object;
    cutscene.trigger_contact = contact;
}

void CutscenePlayback::StartPendingCutscene(const AnimatedObject *object)
{
    const SceneSection *section;
    FUNCTION_MARKER(0x80026C78, "SLUS_005.78");
    uint32 result, val;
    SceneTrigger *event;
    const ScenePath *sel_path;
    AnimatedObject *player;
    sint32 threshold;
    if (object != 0)
    {
        threshold = cutscene.trigger_contact;
        if (threshold >= (sint32)object->path_motion_scale)
            return;
    }
    result = cutscene.active;
    if (result == 0)
        return;
    result = cutscene.remaining_ticks;
    if (result != 0)
        return;
    event = cutscene.trigger;
    section = scene_stream.active_section;
    hd_camera_cut();
    val = event->parameters.cutscene.duration;
    cutscene.duration = val;
    cutscene.remaining_ticks = val;
    cutscene.next_camera = event->parameters.cutscene.next_camera;
    val = event->cutscene_control.flags;
    cutscene.frame = 0;
    cutscene.flags = val;
    sel_path = section->FindPath((uint16)event->parameters.cutscene.camera_id);
    event = cutscene.trigger;
    cutscene.path = sel_path;
    event->node.flags = event->node.flags | 0x80;
    result = cutscene.flags & 2;
    if (result != 0)
    {
        player = CharacterController::FindPlayerObject(0);
        player->node.flags = player->node.flags | 1;
    }
}

uint32 CutscenePlayback::UpdateCutscene(void)
{
    const SceneSection *section;
    FUNCTION_MARKER(0x80026D80, "SLUS_005.78");
    uint32 remaining, frame, duration, target, idx, val;
    const ScenePath *path;
    AnimatedObject *player;
    sint32 numerator;
    remaining = cutscene.remaining_ticks - 1;
    cutscene.remaining_ticks = remaining;
    if (remaining != 0)
    {
        path = cutscene.path;

        frame = cutscene.frame;
        path->SampleOffset(frame, camera_motion.pos, 16);
        path = cutscene.path;

        frame = cutscene.frame;
        path->SamplePos(frame, camera_motion.focus_pos, 16);
        path = cutscene.path;
        if (cutscene.duration != 0)
            hd_camera_path(path, path->offset_track.frame_count * ((float)(cutscene.duration - remaining - 1) / cutscene.duration), 16);
        duration = cutscene.duration;
        remaining = cutscene.remaining_ticks;
        val = path->offset_track.frame_count;
        numerator = val * (duration - remaining);
        if (duration == 0)
            xport_mips_break(7);
        if (duration == 0xFFFFFFFF && numerator == 0x80000000)
            xport_mips_break(6);
        cutscene.frame = numerator / (sint32)duration;
        return 0;
    }
    target = cutscene.next_camera;
    hd_camera_cut();
    if (target == 0xFFFFFFFF)
    {
        cutscene.active = 0;
        player = CharacterController::FindPlayerObject(0);
        player->node.flags = player->node.flags & 0xFE;
        return 1;
    }
    target = cutscene.next_camera;
    for (idx = 0; idx < 32; ++idx)
    {
        if (cutscene_cameras[idx].camera_id == target)
            break;
    }
    if (idx >= 32)
    {
        PrintSdkDiagnostic("ERROR: Couldn't find camera %08x for next cut scene.\n", target);
        cutscene.active = 0;
        return 1;
    }
    val = cutscene_cameras[idx].duration;
    cutscene.duration = val;
    cutscene.remaining_ticks = val;
    val = cutscene_cameras[idx].flags;
    cutscene.flags = val;
    if ((val & 2) != 0)
    {
        player = CharacterController::FindPlayerObject(0);
        player->node.flags = player->node.flags | 1;
    }
    section = scene_stream.active_section;
    target = (uint16)cutscene.next_camera;
    cutscene.frame = 0;
    cutscene.path = section->FindPath(target);
    cutscene.next_camera = cutscene_cameras[idx].next_camera;
    return 0;
}

void CutscenePlayback::ResetCutsceneCameras(void)
{
    sint32 idx;
    FUNCTION_MARKER(0x80026F88, "SLUS_005.78");
    for (idx = 31; idx >= 0; --idx)
        cutscene_cameras[idx].camera_id = 0xFFFFFFFF;
}

uint32 CutscenePlayback::HasCutsceneCameraName(const uint8 *name)
{
    FUNCTION_MARKER(0x80026FB4, "SLUS_005.78");
    uint32 idx;
    for (idx = 0; idx < 32; ++idx)
    {
        if (memcmp(&cutscene_cameras[idx].camera_id, name, 3) == 0)
            return 1;
    }
    return 0;
}
