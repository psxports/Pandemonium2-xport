#include <string.h>

#include "game.h"
#include "original_defaults.h"

CutsceneCamera cutscene_cameras[32] = {{0}};
CutscenePlayback cutscene = {0};

uint32 ResetCutsceneCameras(void)
{
    sint32 index;
    FUNCTION_MARKER(0x80026F88, "SLUS_005.78");
    for (index = 31; index >= 0; --index)
        cutscene_cameras[index].camera_id = 0xFFFFFFFF;
    return (uint32)-16;
}

uint32 RegisterCutsceneCamera(const SceneTrigger *object)
{
    FUNCTION_MARKER(0x80026B6C, "SLUS_005.78");
    uint32 result;
    sint32 index;
    index = object->cutscene_control.index;
    if (index < 0 || index >= 32)
        return p2_sdk_80082a18("Invalid cut scene idx %d. camera %08x\n", index, object->parameters.cutscene.camera_id);
    cutscene_cameras[index].camera_id = object->parameters.cutscene.camera_id;
    cutscene_cameras[index].next_camera = object->parameters.cutscene.next_camera;
    cutscene_cameras[index].duration = object->parameters.cutscene.duration;
    result = object->cutscene_control.flags;
    cutscene_cameras[index].flags = result;
    return result;
}

uint32 QueueCutsceneTrigger(SceneTrigger *object, uint32 contact)
{
    FUNCTION_MARKER(0x80026C54, "SLUS_005.78");
    cutscene.active = 1;
    cutscene.trigger = object;
    cutscene.trigger_contact = contact;
    return 1;
}

void StopCutscene(void)
{
    FUNCTION_MARKER(0x80026BF4, "SLUS_005.78");
    cutscene.remaining_ticks = 0;
    cutscene.active = 0;
}

uint32 GetCutsceneMode(void)
{
    FUNCTION_MARKER(0x80026C14, "SLUS_005.78");
    if ((sint32)cutscene.remaining_ticks <= 0)
        return 0;
    if ((cutscene.flags & 1) != 0)
        return 1;
    return 0xFFFFFFFF;
}

void StartPendingCutscene(const AnimatedObject *object)
{
    const SceneSection *section;
    FUNCTION_MARKER(0x80026C78, "SLUS_005.78");
    uint32 result, value;
    SceneTrigger *event;
    const ScenePath *selected_path;
    AnimatedObject *player;
    sint32 threshold;
    if (object != 0)
    {
        threshold = (sint32)cutscene.trigger_contact;
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
    value = event->parameters.cutscene.duration;
    cutscene.duration = value;
    cutscene.remaining_ticks = value;
    cutscene.next_camera = event->parameters.cutscene.next_camera;
    value = event->cutscene_control.flags;
    cutscene.frame = 0;
    cutscene.flags = value;
    selected_path = section->FindPath((uint16)event->parameters.cutscene.camera_id);
    event = cutscene.trigger;
    cutscene.path = selected_path;
    event->node.flags = event->node.flags | 0x80;
    result = cutscene.flags & 2;
    if (result != 0)
    {
        player = FindPlayerObject(0);
        player->node.flags = player->node.flags | 1;
    }
    return;
}

uint32 UpdateCutscene(void)
{
    const SceneSection *section;
    FUNCTION_MARKER(0x80026D80, "SLUS_005.78");
    uint32 remaining, destination, frame, duration, target, index, value;
    const ScenePath *path;
    AnimatedObject *player;
    sint32 numerator;
    remaining = cutscene.remaining_ticks - 1;
    cutscene.remaining_ticks = remaining;
    if (remaining != 0)
    {
        path = cutscene.path;
        destination = OriginalDefaults::SceneContextIdentity;
        frame = cutscene.frame;
        path->SampleOffset((sint32)frame, (uint32 *)camera_motion.position, 16);
        path = cutscene.path;
        destination = OriginalDefaults::SceneContextIdentity;
        frame = cutscene.frame;
        path->SamplePosition((sint32)frame, (uint32 *)camera_motion.focus_position, 16);
        path = cutscene.path;
        duration = cutscene.duration;
        remaining = cutscene.remaining_ticks;
        value = (uint32)(sint32)(sint16)path->offset_track.metadata6;
        numerator = (sint32)(value * (duration - remaining));
        if (duration == 0)
            xport_mips_break(7);
        if (duration == 0xFFFFFFFF && (uint32)numerator == 0x80000000)
            xport_mips_break(6);
        cutscene.frame = numerator / (sint32)duration;
        return 0;
    }
    target = cutscene.next_camera;
    if (target == 0xFFFFFFFF)
    {
        cutscene.active = 0;
        player = FindPlayerObject(0);
        player->node.flags = player->node.flags & 0xFE;
        return 1;
    }
    target = cutscene.next_camera;
    for (index = 0; index < 32; ++index)
    {
        if (cutscene_cameras[index].camera_id == target)
            break;
    }
    if (index >= 32)
    {
        p2_sdk_80082a18("ERROR: Couldn't find camera %08x for next cut scene.\n", target);
        cutscene.active = 0;
        return 1;
    }
    value = cutscene_cameras[index].duration;
    cutscene.duration = value;
    cutscene.remaining_ticks = value;
    value = cutscene_cameras[index].flags;
    cutscene.flags = value;
    if ((value & 2) != 0)
    {
        player = FindPlayerObject(0);
        player->node.flags = player->node.flags | 1;
    }
    section = scene_stream.active_section;
    target = (uint16)cutscene.next_camera;
    cutscene.frame = 0;
    cutscene.path = section->FindPath((sint32)target);
    cutscene.next_camera = cutscene_cameras[index].next_camera;
    return 0;
}

uint32 HasCutsceneCameraName(const uint8 *name)
{
    FUNCTION_MARKER(0x80026FB4, "SLUS_005.78");
    uint32 index;
    uint8 camera_name[4];
    for (index = 0; index < 32; ++index)
    {
        xport_store_le32(camera_name, cutscene_cameras[index].camera_id);
        if (memcmp(camera_name, name, 3) == 0)
            return 1;
    }
    return 0;
}
