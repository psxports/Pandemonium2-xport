#include "game.h"

uint32 IsSoundVoicePlaying(uint16 voice)
{
    uint32 mask;
    FUNCTION_MARKER(0x8006A940, "SLUS_005.78");
    mask = (uint32)1 << (((uint32)voice + 16) & 31);
    return p2_sdk_80070df8(mask) == 1;
}

uint32 SetSoundVoiceVolume(uint16 voice, uint16 volume)
{
    SoundChannel *channel = &sound_effects.channels[voice];
    uint32 gain;
    sint64 product;
    FUNCTION_MARKER(0x8006A740, "SLUS_005.78");
    gain = (16383 * (uint32)volume) / 256;
    channel->volume_scale = gain;
    sound_effects.attributes.voice = (uint32)1 << (((uint32)voice + 16) & 31);
    sound_effects.attributes.mask = 3;
    product = (sint64)(sint32)channel->volume_scale * (sint32)channel->left_gain;
    sound_effects.attributes.volume.left = (uint32)(product >> 12);
    product = (sint64)(sint32)channel->volume_scale * (sint32)channel->right_gain;
    sound_effects.attributes.volume.right = (uint32)(product >> 12);
    return p2_sdk_8007101c(&sound_effects.attributes);
}

uint32 SetSoundVoicePan(uint16 voice, uint16 pan)
{
    SoundChannel *channel;
    sint32 delta;
    sint64 product;
    FUNCTION_MARKER(0x8006A638, "SLUS_005.78");
    if (pan >= 9)
        pan = 8;
    delta = ((sint32)pan - 4) * 512;
    channel = &sound_effects.channels[voice];
    channel->left_gain = (uint32)(2048 - delta);
    channel->right_gain = 4096 - channel->left_gain;
    sound_effects.attributes.voice = (uint32)1 << (((uint32)voice + 16) & 31);
    sound_effects.attributes.mask = 3;
    product = (sint64)(sint32)channel->volume_scale * (sint32)channel->left_gain;
    sound_effects.attributes.volume.left = (uint32)(product >> 12);
    product = (sint64)(sint32)channel->volume_scale * (sint32)channel->right_gain;
    sound_effects.attributes.volume.right = (uint32)(product >> 12);
    return p2_sdk_8007101c(&sound_effects.attributes);
}

uint32 SetSoundVoicePitch(uint16 voice, uint32 pitch)
{
    FUNCTION_MARKER(0x8006A814, "SLUS_005.78");
    sound_effects.attributes.voice = (uint32)1 << (((uint32)voice + 16) & 31);
    sound_effects.attributes.mask = 16;
    sound_effects.attributes.pitch = (95 * pitch) >> 10;
    return p2_sdk_8007101c(&sound_effects.attributes);
}

void StartSoundVoice(uint16 voice, const SoundSample *sample)
{
    uint32 mask;
    uint32 address;
    FUNCTION_MARKER(0x8006A878, "SLUS_005.78");
    if (sample != 0)
    {
        mask = (uint32)1 << (((uint32)voice + 16) & 31);
        p2_sdk_80070bc0(0, mask);
        sound_effects.attributes.mask = 65408;
        sound_effects.attributes.voice = mask;
        address = sample->spu_address;
        sound_effects.attributes.r_mode = 3;
        sound_effects.attributes.a_mode = 1;
        sound_effects.attributes.s_mode = 1;
        sound_effects.attributes.ar = 0;
        sound_effects.attributes.dr = 0;
        sound_effects.attributes.sr = 0;
        sound_effects.attributes.rr = 0;
        sound_effects.attributes.sl = 15;
        sound_effects.attributes.addr = address;
        p2_sdk_80070dc4(&sound_effects.attributes);
    }
}

uint32 StopSoundVoice(uint16 voice)
{
    uint32 mask = (uint32)1 << (((uint32)voice + 16) & 31);
    SoundChannel *channel;
    sint32 scale;
    sint32 gain;
    sint64 product;
    FUNCTION_MARKER(0x8006A970, "SLUS_005.78");
    p2_sdk_80070bc0(0, mask);
    channel = &sound_effects.channels[voice];
    channel->volume_scale = 0;
    sound_effects.attributes.voice = mask;
    sound_effects.attributes.mask = 3;
    scale = (sint32)channel->volume_scale;
    gain = (sint32)channel->left_gain;
    product = (sint64)scale * gain;
    sound_effects.attributes.volume.left = (uint16)((uint64)product >> 12);
    scale = (sint32)channel->volume_scale;
    gain = (sint32)channel->right_gain;
    product = (sint64)scale * gain;
    sound_effects.attributes.volume.right = (uint16)((uint64)product >> 12);
    return p2_sdk_8007101c(&sound_effects.attributes);
}

void PlaySoundEffect(const SoundSample *sound, uint16 voice, uint32 unused0, uint32 unused1, uint32 override_pitch, sint32 volume, uint16 program)
{
    uint32 pitch = override_pitch;
    uint16 pan;
    FUNCTION_MARKER(0x8005E470, "SLUS_005.78");
    if ((volume >> 4) != 0)
    {
        if (pitch == 0)
        {
            const SoundSample *sample = (const SoundSample *)(sound);
            pitch = sample->pitch;
        }
        pan = program != 0 ? (uint16)(program - 1) : 4;
        SetSoundVoiceVolume(voice, (uint16)(volume >> 4));
        SetSoundVoicePan(voice, pan);
        SetSoundVoicePitch(voice, pitch);
        StartSoundVoice(voice, sound);
    }
}
