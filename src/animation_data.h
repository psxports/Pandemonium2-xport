#ifndef ANIMATION_DATA_H
#define ANIMATION_DATA_H

#include "psx.h"

typedef struct AnimationHeader AnimationHeader;

struct AnimationHeader
{
    sint8 counts[4];
    uint32 frame_info;
    uint32 track_offsets[4];
};

typedef char AnimationHeaderBinaryLayout[(sizeof(AnimationHeader) == 24) ? 1 : -1];

#ifdef __cplusplus
extern "C"
{
#endif

uint32 RelocateAnimationData(AnimationHeader *animation, uint32 base);
uint32 AttachObjectAnimation(uint32 object, uint32 animation);
uint32 LoadAnimationTable(uint32 object, uint32 source, uint32 *destination);

#ifdef __cplusplus
}
#endif

#endif
