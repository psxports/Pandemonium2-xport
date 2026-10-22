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

typedef char AnimatedPathStateSize[(sizeof(AnimatedPathState) == 36) ? 1 : -1];

typedef char AnimatedPathStateVoiceOffset[(offsetof(AnimatedPathState, sound_voice) == 6) ? 1 : -1];

typedef char AnimatedPathStateSoundOffset[(offsetof(AnimatedPathState, sound) == 8) ? 1 : -1];

typedef char AnimatedPathStateSpeedOffset[(offsetof(AnimatedPathState, speed) == 12) ? 1 : -1];

typedef char AnimatedPathStateDirectionOffset[(offsetof(AnimatedPathState, dir) == 24) ? 1 : -1];

#endif
