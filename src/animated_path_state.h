#ifndef ANIMATED_PATH_STATE_H
#define ANIMATED_PATH_STATE_H
#include <stddef.h>

#include "psx.h"
#include "vector3.h"

struct SoundSample;

struct EncodedAnimatedPathState;
struct AnimatedPathState;

struct EncodedAnimatedPathState
{
    uint32 next_offset;
    uint16 start_frame;
    sint16 sound_voice;
    uint32 sound_offset;
    uint32 speed[3];
    Vec3 dir;
};

typedef char EncodedAnimatedPathStateSize[(sizeof(EncodedAnimatedPathState) == 36) ? 1 : -1];

// Runtime animated path state decoded from the section resource
struct AnimatedPathState
{
    AnimatedPathState *next;
    uint16 start_frame;
    sint16 sound_voice;
    const SoundSample *sound;
    uint32 speed[3];
    Vec3 dir;

    uint32 StopSound();
};






#endif
