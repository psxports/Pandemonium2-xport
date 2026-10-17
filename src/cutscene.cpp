#include <string.h>

#include "game.h"

CutsceneCamera cutscene_cameras[32] = {{0}};
CutscenePlayback cutscene = {0};

uint32 ResetCutsceneCameras(void)
{
    sint32 index;
    FUNCTION_MARKER(0x80026F88, "SLUS_005.78");
    w_u32(0x8008A11C, 0);
    for (index = 31; index >= 0; --index)
        cutscene_cameras[index].camera_id = 0xFFFFFFFF;
    return (uint32)-16;
}

uint32 RegisterCutsceneCamera(uint32 object_offset)
{
    FUNCTION_MARKER(0x80026B6C, "SLUS_005.78");
    uint32 result;
    sint32 index;
    index = r_s16(object_offset + 82);
    if (index >= 32)
        return p2_sdk_80082a18("Invalid cut scene idx %d. camera %08x\n", index, r_u32(object_offset + 68));
    cutscene_cameras[index].camera_id = r_u32(object_offset + 68);
    cutscene_cameras[index].next_camera = r_u32(object_offset + 72);
    cutscene_cameras[index].duration = r_u32(object_offset + 76);
    result = r_u16(object_offset + 80);
    cutscene_cameras[index].flags = result;
    return result;
}

uint32 QueueCutsceneTrigger(uint32 object_offset, uint32 contact)
{
    FUNCTION_MARKER(0x80026C54, "SLUS_005.78");
    cutscene.active = 1;
    cutscene.trigger_offset = object_offset;
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

uint32 StartPendingCutscene(uint32 object_offset)
{
    FUNCTION_MARKER(0x80026C78, "SLUS_005.78");
    uint32 result, event, scene, value;
    sint32 threshold;
    if (object_offset != 0)
    {
        threshold = (sint32)cutscene.trigger_contact;
        if (threshold >= r_s32(object_offset + 144))
            return 0;
    }
    result = cutscene.active;
    if (result == 0)
        return result;
    result = cutscene.remaining_ticks;
    if (result != 0)
        return result;
    event = cutscene.trigger_offset;
    scene = r_u32(0x8008A4E8);
    value = r_u32(event + 76);
    cutscene.duration = value;
    cutscene.remaining_ticks = value;
    cutscene.next_camera = r_u32(event + 72);
    value = r_u16(event + 80);
    cutscene.frame = 0;
    cutscene.flags = value;
    result = ((const SceneSection *)psx_addr(scene, sizeof(SceneSection)))->FindObjectOffset(r_u16(event + 68));
    event = cutscene.trigger_offset;
    cutscene.path_offset = result;
    w_u8(event + 13, r_u8(event + 13) | 0x80);
    result = cutscene.flags & 2;
    if (result != 0)
    {
        result = p2_func_800536f0(0);
        w_u8(result + 13, r_u8(result + 13) | 1);
    }
    return result;
}

uint32 UpdateCutscene(void)
{
    FUNCTION_MARKER(0x80026D80, "SLUS_005.78");
    uint32 remaining, path, destination, frame, duration, target, index, value, object;
    sint32 numerator;
    remaining = cutscene.remaining_ticks - 1;
    cutscene.remaining_ticks = remaining;
    if (remaining != 0)
    {
        path = cutscene.path_offset;
        destination = r_u32(0x8008A114);
        frame = cutscene.frame;
        SamplePathOffset(path, (sint32)frame, destination + 4, 16);
        path = cutscene.path_offset;
        destination = r_u32(0x8008A114);
        frame = cutscene.frame;
        SamplePathPosition(path, (sint32)frame, destination + 16, 16);
        path = cutscene.path_offset;
        duration = cutscene.duration;
        remaining = cutscene.remaining_ticks;
        value = (uint32)(sint32)r_s16(path + 46);
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
        object = p2_func_800536f0(0);
        w_u8(object + 13, r_u8(object + 13) & 0xFE);
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
        object = p2_func_800536f0(0);
        w_u8(object + 13, r_u8(object + 13) | 1);
    }
    value = r_u32(0x8008A4E8);
    target = (uint16)cutscene.next_camera;
    cutscene.frame = 0;
    value = ((const SceneSection *)psx_addr(value, sizeof(SceneSection)))->FindObjectOffset((sint32)target);
    cutscene.path_offset = value;
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
