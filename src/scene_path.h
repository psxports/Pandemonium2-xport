#ifndef SCENE_PATH_H
#define SCENE_PATH_H

#include "psx.h"

#ifdef __cplusplus
extern "C"
{
#endif

void SamplePathTangent(uint32 source_offset, VECTOR *destination, sint32 position);
uint32 InterpolateKeyTime(uint32 frame, uint32 end, uint32 start_key, uint32 end_key, uint32 keys_offset);
uint32 SampleVectorTrack(sint32 frame, uint32 track_offset, uint32 destination_offset);
uint32 SampleWorldPath(uint32 definition_offset, sint32 frame, uint32 destination_offset, uint32 direction);
uint32 SamplePathOffset(uint32 path_offset, sint32 frame, uint32 destination_offset, uint32 direction);
uint32 SamplePathPosition(uint32 path_offset, sint32 frame, uint32 destination_offset, uint32 direction);
sint32 ProjectPathFrame(uint32 definition_offset, sint32 frame, uint32 position_offset, uint32 floor_offset, uint32 direction, sint32 scale);
uint32 FindPathHeightFrame(uint32 definition_offset, uint32 position_offset, uint32 direction, sint32 height_offset);
uint32 ChangeActorPath(uint32 object_offset, uint32 path_offset, uint32 position_offset, uint32 floor_offset);
uint32 SelectScenePath(uint32 path_offset);
uint32 ProjectPathOffset(uint32 path_offset, sint32 frame, uint32 destination_offset, uint32 direction);
uint32 ApplyPathVelocityChange(uint32 object_offset, uint32 definition_offset);
uint32 ReverseScenePath(uint32 object_offset, uint32 character_offset);

#ifdef __cplusplus
}
#endif

#endif
