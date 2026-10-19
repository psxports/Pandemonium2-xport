#ifndef SCENE_FRAME_H
#define SCENE_FRAME_H

#include "psx.h"

struct SceneSection;

void UpdateAndDrawRegisteredObjects(void);
uint32 DrawSceneFrame(SceneSection *scene);
void TraverseSceneObjects(SceneSection *scene, uint32 mode);
extern "C" uint32 UpdatePlayerFrame(void);

#endif
