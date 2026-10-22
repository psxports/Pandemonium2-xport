#ifndef CAMERA_MOTION_H
#define CAMERA_MOTION_H
#include <stddef.h>

#include "psx.h"
#include "vector3.h"

struct ScenePath;

struct CameraMotion;

struct CameraMotion
{
    const ScenePath *path;
    uint8 flags;
    sint16 frame;
    Vec3 pos;
    Vec3 focus_pos;
    Vec3 desired_pos;
    uint8 smooth_ticks[3];
    Vec3 target;
    uint32 target_ticks;

    static void UpdateCameraAim(void);
    static void SelectScenePath(const ScenePath *path);
    static void UpdateCameraPos(void);
    static void SetCameraTransition(const Vec3 &delta);
    static void UpdateCameraTarget(void);
    static void EmptyCameraOperation();
};

extern CameraMotion camera_motion;

#endif
