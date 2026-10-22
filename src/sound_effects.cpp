#include <stdio.h>
#include <stdlib.h>

#include "sound_effects.h"
#include "game.h"
#include "global.h"

SoundEffects sound_effects = {{{0}}, {{0}}, 0, {0, 1, 2, 3, 4, 5, 6, 7}};

void SoundEffects::StopSceneSounds(void)
{
    FUNCTION_MARKER(0x80025F58, "SLUS_005.78");
    SceneNode *registered, *node;
    uint32 kind, flags;
    const SceneSection *section;
    registered = registered_object_head;
    while (registered != 0)
    {
        if (((const AnimatedObject *)registered)->behavior == 6)
            ((AnimatedObject *)registered)->StopModelGrpSound();
        registered = registered->links.registry_next;
    }
    section = scene_stream.active_section;
    node = section->object_heads[4];
    if (node != 0)
    {
        do
        {
            kind = node->state_flags >> 8;
            if (kind - 40 >= 16)
                break;
            if ((sint8)kind == 50)
            {
                SceneTrigger *trigger = (SceneTrigger *)node;
                const SceneSoundControl *control = &trigger->parameters.sound;
                flags = control->flags;
                if (((flags >> 17) & 0xFF) == 0 && (flags >> 25) == 0 && control->voice < 8)
                    trigger->StopSound();
            }
            node = node->links.registry_next;
        } while (node != 0);
        section = scene_stream.active_section;
    }
    node = section->object_heads[1];
    while (node != 0)
    {
        ((AnimatedPathState *)((const ActorPlacement *)((const AnimatedObject *)node)->path_definition)->state)->StopSound();
        node = node->links.registry_next;
    }
    section = scene_stream.active_section;
    ActorBinding *binding = section->actor_bindings;
    while (binding != 0)
    {
        binding->StopSound();
        binding = binding->next;
    }
}

void SoundEffects::PlaySoundEffect(const SoundSample *sound, uint16 voice, uint32, uint32, uint32 override_pitch, sint32 volume, uint16 program)
{
    uint32 pitch = override_pitch;
    uint16 pan;
    FUNCTION_MARKER(0x8005E470, "SLUS_005.78");
    if ((volume >> 4) != 0)
    {
        if (pitch == 0)
        {
            const SoundSample *sample = sound;
            pitch = sample->pitch;
        }
        pan = program != 0 ? program - 1 : 4;
        SoundEffects::SetSoundVoiceVolume(voice, volume >> 4);
        SoundEffects::SetSoundVoicePan(voice, pan);
        SoundEffects::SetSoundVoicePitch(voice, pitch);
        SoundEffects::StartSoundVoice(voice, sound);
    }
}

void SoundEffects::Queue(const SoundSample *sound, const SoundSample *prev, uint32 pitch, uint32 volume, uint32 pan, uint32 val)
{
    FUNCTION_MARKER(0x8005E538, "SLUS_005.78");
    uint32 idx, count, result;
    if (prev == 0)
        idx = pending_count;
    else
    {
        count = pending_count;
        idx = 0;
        while (idx < count)
        {
            if (commands[idx].sound == prev)
            {
                if (sound == prev)
                    return;
                break;
            }
            ++idx;
        }
    }
    result = idx < 8;
    if (result == 0)
        return;
    count = pending_count;
    if (idx == count)
        pending_count = count + 1;

    commands[idx].sound = sound;
    commands[idx].prev_sound = prev;
    commands[idx].pitch = pitch;
    commands[idx].volume = volume;
    commands[idx].pan = pan;
    commands[idx].priority = val;
}

sint32 SoundEffects::SelectVoice(uint32 requested, const SoundSample *sound, uint32 priority)
{
    FUNCTION_MARKER(0x8005E628, "SLUS_005.78");
    uint32 voice = requested;
    uint32 min_voice;
    uint32 idx;
    uint32 prev;
    uint32 frame;
    if (requested >= 8)
    {
        min_voice = priority != 1 ? 3 : 0;
        voice = min_voice;
        while (voice < 8)
        {
            if (voices[voice].started_tick < frame_clock.render_tick)
            {
                if (SoundEffects::IsSoundVoicePlaying(voice) == 0)
                    goto assign;
                prev = voices[voice].started_tick;
                frame = frame_clock.render_tick;
                if (prev < frame && min_voice == 3 && frame - prev >= 301)
                    goto assign_with_frame;
            }
            ++voice;
        }
        voice = voice_order[0];
        idx = 0;
        if (priority == 1)
        {
            if ((sint32)voice < (sint32)min_voice)
                goto next_candidate;
            if (voice < 3)
                goto assign;
            voice = voice_order[1];
            idx = 1;
            goto reserved_candidate;
        }
    priority_candidate:
        if (voices[voice].priority < priority && (sint32)voice >= (sint32)min_voice)
            goto assign;
    next_candidate:
        ++idx;
        if (idx >= 8)
            return -1;
        voice = voice_order[idx];
        if (priority != 1)
            goto priority_candidate;
    reserved_candidate:
        if ((sint32)voice < (sint32)min_voice || voice >= 3)
            goto next_candidate;
    }
assign:
    frame = frame_clock.render_tick;
assign_with_frame:
    voices[voice].sound = sound;
    voices[voice].priority = priority;
    voices[voice].started_tick = frame;
    idx = 0;
    while (voice_order[idx] != voice)
    {
        ++idx;
        if (idx >= 7)
            goto finish;
    }
    for (uint32 i = idx; i + 1 < 8; ++i)
        voice_order[i] = voice_order[i + 1];
finish:
    voice_order[7] = voice;
    return voice;
}

uint32 SoundEffects::IsPlaying(const SoundSample *sound, uint32 voice) const
{
    FUNCTION_MARKER(0x8005E868, "SLUS_005.78");
    if (voice < 8 && voices[voice].sound == sound)
        return SoundEffects::IsSoundVoicePlaying(voice) != 0;
    return 0;
}

uint32 SoundEffects::ComputeSpatialSoundControls(const sint32 *pos, uint32 radius, uint32 *attenuation, uint32 *pan_output)
{
    FUNCTION_MARKER(0x8005E8BC, "SLUS_005.78");
    uint32 dx;
    uint32 dy;
    uint32 dz;
    sint16 angle;
    uint32 pan;
    uint32 factor;
    uint32 denominator;
    sint32 distance, quarter, rcp, delta;
    sint64 product;
    dx = (uint32)camera_motion.focus_pos.x - pos[0];
    dy = (uint32)camera_motion.focus_pos.y - pos[1];
    dz = (uint32)camera_motion.focus_pos.z - pos[2];
    distance = ApproxVectorLen(dx, dy, dz);
    if (distance >= (sint32)radius)
    {
        if (attenuation != 0)
        {
            *attenuation = 0;
            return 0;
        }
    }
    else if (attenuation != 0)
    {
        quarter = (sint32)radius >> 2;
        if (quarter < distance)
        {
            delta = radius - distance;
            denominator = radius - quarter;
            if (denominator == 0)
                xport_mips_break(7);
            rcp = 0x40000000 / (sint32)denominator;
            product = (sint64)delta * rcp;
            factor = product >> 18;
            fixed_math_result = factor;
        }
        else
            factor = 4096;
        *attenuation = factor;
        if (factor == 0)
            return 0;
    }
    pan = 1024;
    if (dx + 32 >= 65 || dz + 32 >= 65)
    {
        angle = Atan2Angle((-dz), (-dx));
        pan = ((uint32)(sint16)camera_proj.rotation[1] - angle + 1024) & 0xFFF;
        if (pan >= 0x801)
            pan = 4096 - pan;
        if (distance < (sint32)radius)
        {
            if (radius == 0)
                xport_mips_break(7);
            rcp = 0x40000000 / (sint32)radius;
            product = (sint64)distance * rcp;
            factor = product >> 18;
            delta = (pan - 1024) * factor;
            fixed_math_result = factor;
            pan = (uint32)(delta >> 12) + 1024;
        }
        if (pan == 2048)
            pan = 2047;
    }
    *pan_output = (((pan << 3) + pan) >> 11) + 1;
    return 1;
}

uint32 SoundEffects::PlaySpatialSound(const SoundSample *sound, sint32 volume, uint32 pan, uint32 pitch, const sint32 *pos, uint32 radius, uint32 voice, uint32 mode, uint32 option)
{
    FUNCTION_MARKER(0x8005EB18, "SLUS_005.78");
    uint32 level_word, pan_word;
    uint32 *attenuation;
    uint32 channel;
    uint32 active;
    uint32 val;
    uint32 pan_val;
    sint32 scaled;
    sint32 level, sel;
    pan_word = pan;
    level = volume;
    if (pan == 0)
    {
        attenuation = volume < 4097 ? 0 : &level_word;
        val = SoundEffects::ComputeSpatialSoundControls(pos, radius, attenuation, &pan_word);
        if ((val << 24) == 0)
            goto silent;
        if (volume >= 4097)
        {
            val = (((uint32)volume - 1) & 0xFFF) + 1;
            level = (sint32)(level_word * val) >> 12;
            if (level == 0)
                goto silent;
        }
    }
    scaled = (uint32)level * sound_settings.effects_volume;
    level = scaled >> 12;
    level_word = level;
    if (level == 0)
        goto silent;
    if (volume >= 8193)
        pan_word = 5;
    if ((sint32)voice >= 0 && voice < 8 && sound_effects.voices[voice].sound == sound)
    {
        channel = voice & 0xFFFF;
        active = SoundEffects::IsSoundVoicePlaying(channel);
        if (active != 0)
        {
            level = (sint32)level_word >> 4;
            SoundEffects::SetSoundVoiceVolume(channel, level);
            SoundEffects::SetSoundVoicePan(channel, pan_word);
            SoundEffects::SetSoundVoicePitch(channel, pitch);
            return voice;
        }
        if (mode == 0)
        {
            return 0xFFFFFFFF;
        }
    }
    sel = sound_effects.SelectVoice(0xFFFFFFFF, sound, option);
    if (sel >= 0)
    {
        level = (sint32)level_word >> 4;
        pan_val = pan_word;
        if (level != 0)
        {
            val = pitch;
            if (val == 0)
                val = sound->pitch;
            pan_val = (pan_val & 0xFFFF) != 0 ? pan_val - 1 : 4;
            channel = (uint32)sel & 0xFFFF;
            SoundEffects::SetSoundVoiceVolume(channel, level);
            SoundEffects::SetSoundVoicePan(channel, pan_val);
            SoundEffects::SetSoundVoicePitch(channel, val);
            SoundEffects::StartSoundVoice(channel, sound);
        }
    }
    return sel;

silent:
    if ((sint32)voice >= 0 && voice < 8 && sound_effects.voices[voice].sound == sound)
        SoundEffects::StopSoundVoice(voice);
    return 0xFFFFFFFF;
}

void SoundEffects::Stop(uint32 all_voices)
{
    FUNCTION_MARKER(0x8005ED74, "SLUS_005.78");
    uint32 voice;
    if (all_voices != 0)
    {
        pending_count = 0;
        for (voice = 0; voice < 8; ++voice)
            voices[voice] = SoundVoice();
        for (voice = 0; voice < 8; ++voice)
        {
            SoundEffects::StopSoundVoice(voice);
            voice_order[voice] = voice;
        }
    }
    else
    {
        for (voice = 3; voice < 8; ++voice)
        {
            voices[voice] = SoundVoice();
        }
    }
}

void SoundEffects::Flush()
{
    FUNCTION_MARKER(0x8005EE20, "SLUS_005.78");
    uint32 count, requested, skip, pitch, volume, pan, channel;
    const SoundSample *prev, *cached, *sound;
    sint32 sel;
    count = pending_count;
    pending_count = 0;
    while (count != 0)
    {
        SoundCommand &command = commands[--count];
        prev = command.prev_sound;
        requested = 8;
        skip = 0;
        if (prev != 0)
        {
            for (requested = 0; requested < 8; ++requested)
            {
                cached = voices[requested].sound;
                if (cached == prev)
                {
                    if (cached == command.sound && frame_clock.render_tick - voices[requested].started_tick < 60 && SoundEffects::IsSoundVoicePlaying(requested) != 0)
                        skip = 1;
                    break;
                }
            }
        }
        if (skip != 0)
            continue;
        sound = command.sound;
        sel = SelectVoice(requested, sound, command.priority);
        if (sel >= 0)
        {
            pan = command.pan;
            volume = command.volume >> 4;
            pitch = command.pitch;
            sound = command.sound;
            if (volume != 0)
            {
                if (pitch == 0)
                    pitch = sound->pitch;
                pan = pan != 0 ? pan - 1 : 4;
                channel = (uint32)sel & 0xFFFF;
                SoundEffects::SetSoundVoiceVolume(channel, volume);
                SoundEffects::SetSoundVoicePan(channel, pan);
                SoundEffects::SetSoundVoicePitch(channel, pitch);
                SoundEffects::StartSoundVoice(channel, sound);
            }
        }
    }
}

void SoundEffects::SetSoundVoicePan(uint16 voice, uint16 pan)
{
    SoundChannel *channel;
    sint32 delta;
    FUNCTION_MARKER(0x8006A638, "SLUS_005.78");
    if (pan >= 9)
        pan = 8;
    delta = (pan - 4) * 512;
    channel = &sound_effects.channels[voice];
    channel->left_gain = 2048 - delta;
    channel->right_gain = 4096 - channel->left_gain;
    sound_effects.attributes.voice = 1u << ((voice + 16) & 31);
    sound_effects.attributes.mask = 3;
    sound_effects.attributes.volume.left = mul12(channel->volume_scale, channel->left_gain);
    sound_effects.attributes.volume.right = mul12(channel->volume_scale, channel->right_gain);
    SetSpuVoiceAttributes(&sound_effects.attributes);
}

void SoundEffects::SetSoundVoiceVolume(uint16 voice, uint16 volume)
{
    SoundChannel *channel = &sound_effects.channels[voice];

    FUNCTION_MARKER(0x8006A740, "SLUS_005.78");
    channel->volume_scale = (16383 * volume) / 256;
    sound_effects.attributes.voice = 1u << ((voice + 16) & 31);
    sound_effects.attributes.mask = 3;
    sound_effects.attributes.volume.left = mul12(channel->volume_scale, channel->left_gain);
    sound_effects.attributes.volume.right = mul12(channel->volume_scale, channel->right_gain);
    SetSpuVoiceAttributes(&sound_effects.attributes);
}

void SoundEffects::SetSoundVoicePitch(uint16 voice, uint32 pitch)
{
    FUNCTION_MARKER(0x8006A814, "SLUS_005.78");
    sound_effects.attributes.voice = 1u << ((voice + 16) & 31);
    sound_effects.attributes.mask = 16;
    sound_effects.attributes.pitch = (95 * pitch) >> 10;
    SetSpuVoiceAttributes(&sound_effects.attributes);
}

void SoundEffects::StartSoundVoice(uint16 voice, const SoundSample *sample)
{
    uint32 mask;
    uint32 address;
    FUNCTION_MARKER(0x8006A878, "SLUS_005.78");
    if (sample != 0)
    {
        mask = 1u << ((voice + 16) & 31);
        SetSpuKeys(0, mask);
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
        SetSpuVoiceAttributesAndKeyOn(&sound_effects.attributes);
    }
}

uint32 SoundEffects::IsSoundVoicePlaying(uint16 voice)
{
    uint32 mask;
    FUNCTION_MARKER(0x8006A940, "SLUS_005.78");
    mask = 1u << ((voice + 16) & 31);
    return GetSpuKeyStatus(mask) == 1;
}

void SoundEffects::StopSoundVoice(uint16 voice)
{
    uint32 mask = 1u << ((voice + 16) & 31);
    SoundChannel *channel;
    sint32 scale;
    sint32 gain;
    FUNCTION_MARKER(0x8006A970, "SLUS_005.78");
    SetSpuKeys(0, mask);
    channel = &sound_effects.channels[voice];
    channel->volume_scale = 0;
    sound_effects.attributes.voice = mask;
    sound_effects.attributes.mask = 3;
    scale = channel->volume_scale;
    gain = channel->left_gain;
    sound_effects.attributes.volume.left = mul12(scale, gain);
    scale = channel->volume_scale;
    gain = channel->right_gain;
    sound_effects.attributes.volume.right = mul12(scale, gain);
    SetSpuVoiceAttributes(&sound_effects.attributes);
}
