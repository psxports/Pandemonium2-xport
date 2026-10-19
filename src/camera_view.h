#ifndef CAMERA_VIEW_H
#define CAMERA_VIEW_H

#include <stddef.h>

#include "psx.h"

struct ScenePath;
struct SceneSection;

struct CameraProjection
{
    uint32 scale;
    uint16 rotation[3];
    uint32 far_distance, far_distance_copy;
    SVECTOR view_normals[5];
};

struct CameraMotion
{
    const ScenePath *path;
    uint8 flags;
    sint16 frame;
    uint32 position[3];
    uint32 focus_position[3];
    uint32 desired_position[3];
    uint8 smooth_ticks[3];
    uint32 target[3];
    uint32 target_ticks;
};

extern CameraMotion camera_motion;

uint32 UpdateCameraPosition();
uint32 SetCameraTransition(const uint32 *delta);
uint32 UpdateCameraTarget();

extern CameraProjection camera_projection;

uint32 SetCameraProjection(sint32 scale);
void BuildCameraView(SceneSection *scene);
uint32 UpdateCameraAim();
uint32 IsSphereVisible(const uint32 *position, uint32 radius);
uint32 ResetSceneRendering();

#endif
