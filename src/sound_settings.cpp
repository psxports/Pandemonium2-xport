#include <stdio.h>
#include <stdlib.h>

#include "sound_settings.h"
#include "game.h"

SoundSettings sound_settings = {OriginalDefaults::EffectsVolume, OriginalDefaults::MusicVolume};

sint32 SoundSettings::AdjustEffects(sint32 dir)
{
    sint32 val;
    sint32 next;
    FUNCTION_MARKER(0x8005EFF4, "SLUS_005.78");
    val = effects_volume;
    if (dir < 0)
    {
        if (val <= 0)
            return val;
        next = val - 64;
        effects_volume = next;
        if ((sint16)next < 0)
            effects_volume = 0;
    }
    else if (val < 4096)
    {
        next = val + 64;
        effects_volume = next;
        if ((sint16)next >= 4097)
            effects_volume = 4096;
    }
    return effects_volume;
}

sint32 SoundSettings::AdjustMusic(sint32 dir)
{
    FUNCTION_MARKER(0x8005F070, "SLUS_005.78");
    sint32 volume = music_volume;
    if (dir < 0)
    {
        if (volume > 0)
        {
            sint16 decreased = volume - 64;
            music_volume = decreased;
            if (decreased < 0)
                music_volume = 0;
        }
    }
    else if (volume < 4096)
    {
        sint16 increased = volume + 64;
        music_volume = increased;
        if (increased >= 4097)
            music_volume = 4096;
    }
    if (dir != 0)
        SequencePlayback::SetSequenceVolume((uint8)sequence_playback.cur_handle - 1u, (uint16)music_volume);
    return music_volume;
}

uint32 SoundSettings::SetMusic(uint32 handle, sint16 volume)
{
    FUNCTION_MARKER(0x8005F124, "SLUS_005.78");
    music_volume = volume;
    SequencePlayback::SetSequenceVolume(handle - 1, volume);
    return music_volume;
}
