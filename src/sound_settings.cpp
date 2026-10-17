#include "game.h"

SoundSettings sound_settings = {OriginalDefaults::EffectsVolume, OriginalDefaults::MusicVolume};

sint32 SoundSettings::AdjustEffects(sint32 direction)
{
    sint32 value;
    sint32 next;
    FUNCTION_MARKER(0x8005EFF4, "SLUS_005.78");
    value = (sint16)(effects_volume);
    if (direction < 0)
    {
        if (value <= 0)
            return value;
        next = value - 64;
        effects_volume = (uint16)((uint32)next);
        if ((sint16)next < 0)
            effects_volume = (uint16)(0);
    }
    else if (value < 4096)
    {
        next = value + 64;
        effects_volume = (uint16)((uint32)next);
        if ((sint16)next >= 4097)
            effects_volume = (uint16)(4096);
    }
    return (sint16)(effects_volume);
}

sint32 SoundSettings::AdjustMusic(sint32 direction)
{
    FUNCTION_MARKER(0x8005F070, "SLUS_005.78");
    sint32 volume = (sint16)(music_volume);
    if (direction < 0)
    {
        if (volume > 0)
        {
            sint16 decreased = (sint16)(volume - 64);
            music_volume = (uint16)(decreased);
            if (decreased < 0)
                music_volume = (uint16)(0);
        }
    }
    else if (volume < 4096)
    {
        sint16 increased = (sint16)(volume + 64);
        music_volume = (uint16)(increased);
        if (increased >= 4097)
            music_volume = (uint16)(4096);
    }
    if (direction != 0)
        p2_func_8006a5e0(r_u8(0x8008A530) - 1u, music_volume);
    return (sint16)(music_volume);
}

uint32 SoundSettings::SetMusic(uint32 handle, sint16 volume)
{
    FUNCTION_MARKER(0x8005F124, "SLUS_005.78");
    music_volume = (uint16)(volume);
    p2_func_8006a5e0(handle - 1, volume);
    return (uint32)(sint16)(music_volume);
}
