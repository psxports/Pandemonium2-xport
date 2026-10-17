#ifndef SOUND_EFFECTS_H
#define SOUND_EFFECTS_H
#include "original_defaults.h"
struct SoundVoice
{
    unsigned int sound, started_tick, priority;
};
struct SoundCommand
{
    unsigned int sound, previous_sound, pitch;
    unsigned short volume, pan;
    unsigned int priority;
};
class SoundEffects
{
public:
    SoundVoice voices[8];
    unsigned int Queue(unsigned int sound, unsigned int previous_sound, unsigned int pitch, unsigned int volume, unsigned int pan, unsigned int priority);
    int SelectVoice(unsigned int requested, unsigned int sound, unsigned int priority);
    unsigned int IsPlaying(unsigned int sound, unsigned int voice) const;
    unsigned int Stop(unsigned int all_voices);
    void Flush();
    SoundCommand commands[8];
    unsigned char pending_count;
    unsigned char voice_order[8];
};
extern SoundEffects sound_effects;
#endif
