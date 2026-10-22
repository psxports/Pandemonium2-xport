#ifndef CAMERA_PROJECTION_H
#define CAMERA_PROJECTION_H
#include <stddef.h>

#include "psx.h"

struct SceneSection;

struct SceneSection;

struct CameraProjection;

struct CameraProjection
{
    uint32 scale;
    uint16 rotation[3];
    uint32 far_distance, far_distance_copy;
    SVECTOR view_normals[5];

    static uint32 IsSphereVisible(const sint32 *pos, uint32 radius);
    static void UpdateAndDrawRegisteredObjects(void);
    static void BuildCameraView(SceneSection *scene);
    static void TraverseSceneObjects(SceneSection *scene, uint32 mode);
    static uint32 UpdatePlayerFrame(void);
    static void DrawSceneFrame(SceneSection *scene);
    static void SetCameraProj(sint32 scale);
    static uint32 ResetSceneRendering(void);
};

extern CameraProjection camera_proj;

#endif
