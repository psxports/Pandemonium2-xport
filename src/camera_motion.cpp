#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "camera_motion.h"
#include "game.h"
#include "animated_object.h"
#include "psx_gte.h"

CameraMotion camera_motion = {0};

static const uint16 camera_transition_factors[45] = {2048, 1365, 1024, 819, 682, 585, 512, 455, 409, 372, 341, 315, 292, 273, 256, 240, 227, 215, 204, 195, 186, 178, 170, 163, 157, 151, 146, 141, 136, 132, 128, 124, 120, 117, 113, 110, 107, 105, 102, 99, 97, 95, 93, 91, 89};

void CameraMotion::UpdateCameraAim(void)
{
    uint32 x;
    uint32 y;
    uint32 z;
    uint32 base;
    uint32 sum;
    uint32 len;
    uint32 mode;
    uint32 angle;
    uint32 result;
    sint32 component;
    FUNCTION_MARKER(0x80022D38, "SLUS_005.78");
    x = camera_motion.focus_pos.x;
    base = camera_motion.pos.x;
    x -= base;
    y = camera_motion.focus_pos.y;
    base = camera_motion.pos.y;
    y -= base;
    z = camera_motion.focus_pos.z;
    base = camera_motion.pos.z;
    z -= base;
    mode = CutscenePlayback::GetCutsceneMode();
    component = (sint32)x >> 6;
    sum = (sint64)component * component;
    component = (sint32)y >> 6;
    sum += (uint32)((sint64)component * component);
    component = (sint32)z >> 6;
    sum += (uint32)((sint64)component * component);
    if (mode != 0)
        len = SqRootQ12Exact(sum);
    else if ((sint32)sum >= 0)
        len = SquareRoot12(sum);
    else
        len = SquareRoot12((sum >> 2)) << 1;
    angle = Atan2Angle(z, x);
    if ((sint16)angle != 4096)
        camera_proj.rotation[1] = angle;
    angle = AsinAngle(y, len);
    result = -angle;
    if ((sint16)angle != 4096)
    {
        result &= 0xFFF;
        camera_proj.rotation[0] = result;
    }
}

void CameraMotion::SelectScenePath(const ScenePath *path)
{
    FUNCTION_MARKER(0x80024844, "SLUS_005.78");

    if (camera_proj.view_normals[0].vz != 4096)
        CameraProjection::SetCameraProj(0xE9555);

    camera_motion.path = path;
}

void CameraMotion::UpdateCameraPos(void)
{
    FUNCTION_MARKER(0x80024890, "SLUS_005.78");
    Vec3 difference;
    uint32 count, factor, divisor;
    difference.x = camera_motion.desired_pos.x - (uint32)camera_motion.pos.x;
    difference.y = camera_motion.desired_pos.y - (uint32)camera_motion.pos.y;
    difference.z = camera_motion.desired_pos.z - (uint32)camera_motion.pos.z;
    count = camera_motion.smooth_ticks[0];
    if (count != 0)
    {
        count = (count - 1) & 0xFF;
        camera_motion.smooth_ticks[0] = count;
        if (count < 45)
        {
            count = camera_motion.smooth_ticks[0];
            factor = camera_transition_factors[count];
        }
        else
        {
            count = camera_motion.smooth_ticks[0];
            divisor = (count + 2) << 12;
            factor = 0x1000000 / divisor;
        }
        difference.x = mul12(difference.x, factor);
    }
    count = camera_motion.smooth_ticks[1];
    if (count != 0)
    {
        count = (count - 1) & 0xFF;
        camera_motion.smooth_ticks[1] = count;
        if (count < 45)
        {
            count = camera_motion.smooth_ticks[1];
            factor = camera_transition_factors[count];
        }
        else
        {
            /* Each axis derives its own long transition coefficient */
            count = camera_motion.smooth_ticks[1];
            divisor = (count + 2) << 12;
            factor = 0x1000000 / divisor;
        }
        difference.y = mul12(difference.y, factor);
    }
    count = camera_motion.smooth_ticks[2];
    if (count != 0)
    {
        count = (count - 1) & 0xFF;
        camera_motion.smooth_ticks[2] = count;
        if (count < 45)
        {
            count = camera_motion.smooth_ticks[2];
            factor = camera_transition_factors[count];
        }
        else
        {
            count = camera_motion.smooth_ticks[2];
            divisor = (count + 2) << 12;
            factor = 0x1000000 / divisor;
        }
        difference.z = mul12(difference.z, factor);
    }
    camera_motion.pos.x = camera_motion.pos.x + ((uint32)difference.x);
    camera_motion.pos.y = camera_motion.pos.y + ((uint32)difference.y);
    camera_motion.pos.z = camera_motion.pos.z + ((uint32)difference.z);
    camera_motion.flags = camera_motion.flags & 0xFE;
}

void CameraMotion::SetCameraTransition(const Vec3 &delta)
{
    FUNCTION_MARKER(0x80024B3C, "SLUS_005.78");
    uint32 val;
    sint32 count;
    val = delta.x;
    if ((sint32)val < 0)
        val = -val;
    count = (sint32)val >> 12;
    if (count != 0)
    {
        count = (count >> 1) + 1;
        if (count >= 46)
            count = 45;
    }
    else
    {
        if (camera_motion.smooth_ticks[0] != 0)
            count = 1;
    }

    camera_motion.smooth_ticks[0] = count;
    val = delta.y;
    if ((sint32)val < 0)
        val = -val;
    count = (sint32)val >> 12;
    if (count != 0)
    {
        count = (count >> 1) + 1;
        if (count >= 46)
            count = 45;
    }
    else
    {
        if (camera_motion.smooth_ticks[1] != 0)
            count = 1;
    }

    camera_motion.smooth_ticks[1] = count;
    val = delta.z;
    if ((sint32)val < 0)
        val = -val;
    count = (sint32)val >> 12;
    if (count != 0)
    {
        count = (count >> 1) + 1;
        if (count >= 46)
            count = 45;
    }
    else
    {
        if (camera_motion.smooth_ticks[2] != 0)
            count = 1;
    }

    camera_motion.smooth_ticks[2] = count;
}

void CameraMotion::UpdateCameraTarget(void)
{
    FUNCTION_MARKER(0x80024C44, "SLUS_005.78");
    Vec3 difference;
    uint32 count, factor;
    difference.x = camera_motion.target.x - (uint32)camera_motion.focus_pos.x;
    difference.y = camera_motion.target.y - (uint32)camera_motion.focus_pos.y;
    difference.z = camera_motion.target.z - (uint32)camera_motion.focus_pos.z;
    count = camera_motion.target_ticks;
    if (count)
    {
        --count;
        factor = camera_transition_factors[count];
        camera_motion.target_ticks = count;
        difference.x = mul12(difference.x, factor);
        difference.y = mul12(difference.y, factor);
        difference.z = mul12(difference.z, factor);
    }
    camera_motion.focus_pos.x = camera_motion.focus_pos.x + ((uint32)difference.x);
    camera_motion.focus_pos.y = camera_motion.focus_pos.y + ((uint32)difference.y);
    camera_motion.focus_pos.z = camera_motion.focus_pos.z + ((uint32)difference.z);
    if (!camera_motion.target_ticks)
    {
        CharacterModelRecord::HideAlternateCharacterModel();
        game_flow.transition_flags = game_flow.transition_flags & 0xFFFFFEFF;
    }
    camera_motion.flags = camera_motion.flags & 0xFE;
}

void CameraMotion::EmptyCameraOperation()
{
    FUNCTION_MARKER(0x80059AE4, "SLUS_005.78");
}
