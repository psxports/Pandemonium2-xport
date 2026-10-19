#ifndef SOUND_EFFECTS_H
#define SOUND_EFFECTS_H
#include "original_defaults.h"
#include "psx.h"
#include "sound_resources.h"

struct SoundVoice
{
    const SoundSample *sound;
    unsigned int started_tick, priority;
};

struct SoundCommand
{
    const SoundSample *sound;
    const SoundSample *previous_sound;
    unsigned int pitch;
    unsigned short volume, pan;
    unsigned int priority;
};

struct SoundChannel
{
    unsigned int left_gain;
    unsigned int right_gain;
    unsigned int volume_scale;
};

class SoundEffects
{
public:
    SoundVoice voices[8];
    unsigned int Queue(const SoundSample *sound, const SoundSample *previous_sound, unsigned int pitch, unsigned int volume, unsigned int pan, unsigned int priority);
    int SelectVoice(unsigned int requested, const SoundSample *sound, unsigned int priority);
    unsigned int IsPlaying(const SoundSample *sound, unsigned int voice) const;
    unsigned int Stop(unsigned int all_voices);
    void Flush();
    SoundCommand commands[8];
    unsigned char pending_count;
    unsigned char voice_order[8];
    SoundChannel channels[8];
    SpuVoiceAttr attributes;
};

extern SoundEffects sound_effects;

extern "C"
{
    unsigned int IsSoundVoicePlaying(unsigned short voice);
    unsigned int SetSoundVoiceVolume(unsigned short voice, unsigned short volume);
    unsigned int SetSoundVoicePan(unsigned short voice, unsigned short pan);
    unsigned int SetSoundVoicePitch(unsigned short voice, unsigned int pitch);
    void StartSoundVoice(unsigned short voice, const SoundSample *sample);
    unsigned int StopSoundVoice(unsigned short voice);
    void PlaySoundEffect(const SoundSample *sound, unsigned short voice, unsigned int unused0, unsigned int unused1, unsigned int override_pitch, int volume, unsigned short program);
}

uint32 ComputeSpatialSoundControls(const uint32 *position, uint32 radius, uint32 *attenuation, uint32 *pan_output);
uint32 PlaySpatialSound(const SoundSample *sound, sint32 volume, uint32 pan, uint32 pitch, const uint32 *position, uint32 radius, uint32 voice, uint32 mode, uint32 option);

void StopSceneSounds();

#endif
