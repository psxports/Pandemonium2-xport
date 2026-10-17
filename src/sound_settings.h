#ifndef SOUND_SETTINGS_H
#define SOUND_SETTINGS_H

#include "original_defaults.h"

class SoundSettings
{
public:
    unsigned short effects_volume;
    unsigned short music_volume;

    int AdjustEffects(int direction);
    int AdjustMusic(int direction);
    unsigned int SetMusic(unsigned int handle, short volume);
};

extern SoundSettings sound_settings;

#endif
