#ifndef SOUND_SETTINGS_H
#define SOUND_SETTINGS_H
#include <stddef.h>

#include "psx.h"

class SoundSettings;

class SoundSettings
{
public:
    sint16 effects_volume;
    sint16 music_volume;

    sint32 AdjustEffects(sint32 dir);
    sint32 AdjustMusic(sint32 dir);
    uint32 SetMusic(uint32 handle, sint16 volume);
};

extern SoundSettings sound_settings;

#endif
