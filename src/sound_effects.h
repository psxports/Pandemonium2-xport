#ifndef SOUND_EFFECTS_H
#define SOUND_EFFECTS_H
#include <stddef.h>

#include "psx.h"
#include "psx_spu.h"

struct SoundSample;

struct SoundSample;

struct SoundVoice;
struct SoundCommand;
struct SoundChannel;
class SoundEffects;

struct SoundVoice
{
    const SoundSample *sound;
    uint32 started_tick, priority;
};

struct SoundCommand
{
    const SoundSample *sound;
    const SoundSample *prev_sound;
    uint32 pitch;
    uint16 volume, pan;
    uint32 priority;
};

struct SoundChannel
{
    sint32 left_gain;
    sint32 right_gain;
    sint32 volume_scale;
};

class SoundEffects
{
public:
    SoundVoice voices[8];

    SoundCommand commands[8];
    uint8 pending_count;
    uint8 voice_order[8];
    SoundChannel channels[8];
    SpuVoiceAttr attributes;

    static void StopSceneSounds(void);
    static void PlaySoundEffect(const SoundSample *sound, uint16 voice, uint32, uint32, uint32 override_pitch, sint32 volume, uint16 program);
    void Queue(const SoundSample *sound, const SoundSample *prev_sound, uint32 pitch, uint32 volume, uint32 pan, uint32 priority);
    sint32 SelectVoice(uint32 requested, const SoundSample *sound, uint32 priority);
    uint32 IsPlaying(const SoundSample *sound, uint32 voice) const;
    static uint32 ComputeSpatialSoundControls(const sint32 *pos, uint32 radius, uint32 *attenuation, uint32 *pan_output);
    static uint32 PlaySpatialSound(const SoundSample *sound, sint32 volume, uint32 pan, uint32 pitch, const sint32 *pos, uint32 radius, uint32 voice, uint32 mode, uint32 option);
    void Stop(uint32 all_voices);
    void Flush();
    static void SetSoundVoicePan(uint16 voice, uint16 pan);
    static void SetSoundVoiceVolume(uint16 voice, uint16 volume);
    static void SetSoundVoicePitch(uint16 voice, uint32 pitch);
    static void StartSoundVoice(uint16 voice, const SoundSample *sample);
    static uint32 IsSoundVoicePlaying(uint16 voice);
    static void StopSoundVoice(uint16 voice);
};

extern SoundEffects sound_effects;

#endif
