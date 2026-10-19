#include "game.h"

SoundEffects sound_effects = {{{0}}, {{0}}, 0, {0, 1, 2, 3, 4, 5, 6, 7}};

uint32 SoundEffects::Queue(const SoundSample *sound, const SoundSample *previous, uint32 pitch, uint32 volume, uint32 pan, uint32 value)
{
    FUNCTION_MARKER(0x8005E538, "SLUS_005.78");
    uint32 index, count, result, offset;
    if (previous == 0)
        index = pending_count;
    else
    {
        count = pending_count;
        index = 0;
        while (index < count)
        {
            if (commands[index].sound == previous)
            {
                result = index < 8;
                if (sound == previous)
                    return result;
                break;
            }
            ++index;
        }
    }
    result = index < 8;
    if (result == 0)
        return result;
    count = pending_count;
    if (index == count)
        pending_count = (unsigned char)(count + 1);
    offset = index * 20;
    commands[index].sound = sound;
    commands[index].previous_sound = previous;
    commands[index].pitch = pitch;
    commands[index].volume = (unsigned short)(volume);
    commands[index].pan = (unsigned short)(pan);
    commands[index].priority = value;
    return offset;
}

sint32 SoundEffects::SelectVoice(uint32 requested, const SoundSample *sound, uint32 priority)
{
    FUNCTION_MARKER(0x8005E628, "SLUS_005.78");
    uint32 voice = requested;
    uint32 first;
    uint32 index;
    uint32 previous;
    uint32 frame;
    if (requested >= 8)
    {
        first = priority != 1 ? 3 : 0;
        voice = first;
        while (voice < 8)
        {
            if (voices[voice].started_tick < frame_clock.render_tick)
            {
                if (IsSoundVoicePlaying((uint16)voice) == 0)
                    goto assign;
                previous = voices[voice].started_tick;
                frame = frame_clock.render_tick;
                if (previous < frame && first == 3 && frame - previous >= 301)
                    goto assign_with_frame;
            }
            ++voice;
        }
        voice = voice_order[0];
        index = 0;
        if (priority == 1)
        {
            if ((sint32)voice < (sint32)first)
                goto next_candidate;
            if (voice < 3)
                goto assign;
            voice = voice_order[1];
            index = 1;
            goto reserved_candidate;
        }
    priority_candidate:
        if (voices[voice].priority < priority && (sint32)voice >= (sint32)first)
            goto assign;
    next_candidate:
        ++index;
        if (index >= 8)
            return -1;
        voice = voice_order[index];
        if (priority != 1)
            goto priority_candidate;
    reserved_candidate:
        if ((sint32)voice < (sint32)first || voice >= 3)
            goto next_candidate;
    }
assign:
    frame = frame_clock.render_tick;
assign_with_frame:
    voices[voice].sound = sound;
    voices[voice].priority = priority;
    voices[voice].started_tick = frame;
    index = 0;
    while (voice_order[index] != voice)
    {
        ++index;
        if (index >= 7)
            goto finish;
    }
    for (uint32 i = index; i + 1 < 8; ++i)
        voice_order[i] = voice_order[i + 1];
finish:
    voice_order[7] = (unsigned char)(voice);
    return (sint32)voice;
}

uint32 SoundEffects::IsPlaying(const SoundSample *sound, uint32 voice) const
{
    FUNCTION_MARKER(0x8005E868, "SLUS_005.78");
    if (voice < 8 && voices[voice].sound == sound)
        return IsSoundVoicePlaying((uint16)voice) != 0;
    return 0;
}

uint32 SoundEffects::Stop(uint32 all_voices)
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
            StopSoundVoice((uint16)voice);
            voice_order[voice] = (unsigned char)(voice);
        }
    }
    else
    {
        for (voice = 3; voice < 8; ++voice)
        {
            voices[voice] = SoundVoice();
        }
    }
    return 0;
}

void SoundEffects::Flush()
{
    FUNCTION_MARKER(0x8005EE20, "SLUS_005.78");
    uint32 count, requested, skip, pitch, volume, pan, channel;
    const SoundSample *previous, *cached, *sound;
    sint32 selected;
    count = pending_count;
    pending_count = 0;
    while (count != 0)
    {
        SoundCommand &command = commands[--count];
        previous = command.previous_sound;
        requested = 8;
        skip = 0;
        if (previous != 0)
        {
            for (requested = 0; requested < 8; ++requested)
            {
                cached = voices[requested].sound;
                if (cached == previous)
                {
                    if (cached == command.sound && frame_clock.render_tick - voices[requested].started_tick < 60 && IsSoundVoicePlaying((uint16)requested) != 0)
                        skip = 1;
                    break;
                }
            }
        }
        if (skip != 0)
            continue;
        sound = command.sound;
        selected = SelectVoice(requested, sound, command.priority);
        if (selected >= 0)
        {
            pan = command.pan;
            volume = command.volume >> 4;
            pitch = command.pitch;
            sound = command.sound;
            if (volume != 0)
            {
                if (pitch == 0)
                    pitch = ((const SoundSample *)(sound))->pitch;
                pan = pan != 0 ? pan - 1 : 4;
                channel = (uint32)selected & 0xFFFF;
                SetSoundVoiceVolume((uint16)channel, (uint16)volume);
                SetSoundVoicePan((uint16)channel, (uint16)pan);
                SetSoundVoicePitch((uint16)channel, pitch);
                StartSoundVoice((uint16)channel, sound);
            }
        }
    }
}

uint32 ComputeSpatialSoundControls(const uint32 *position, uint32 radius, uint32 *attenuation, uint32 *pan_output)
{
    FUNCTION_MARKER(0x8005E8BC, "SLUS_005.78");
    uint32 dx, dy, dz, angle, pan, factor, denominator;
    sint32 distance, quarter, reciprocal, delta;
    sint64 product;
    dx = camera_motion.focus_position[0] - position[0];
    dy = camera_motion.focus_position[1] - position[1];
    dz = camera_motion.focus_position[2] - position[2];
    distance = (sint32)ApproxVectorLength(dx, dy, dz);
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
            delta = (sint32)(radius - (uint32)distance);
            denominator = radius - (uint32)quarter;
            if (denominator == 0)
                xport_mips_break(7);
            reciprocal = 0x40000000 / (sint32)denominator;
            product = (sint64)delta * reciprocal;
            factor = (uint32)(product >> 18);
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
        angle = Atan2Angle((sint32)(0u - dz), (sint32)(0u - dx));
        pan = ((uint32)(sint32)(sint16)camera_projection.rotation[1] - (uint32)(sint32)(sint16)angle + 1024) & 0xFFF;
        if (pan >= 0x801)
            pan = 4096 - pan;
        if (distance < (sint32)radius)
        {
            if (radius == 0)
                xport_mips_break(7);
            reciprocal = 0x40000000 / (sint32)radius;
            product = (sint64)distance * reciprocal;
            factor = (uint32)(product >> 18);
            delta = (sint32)((pan - 1024) * factor);
            fixed_math_result = factor;
            pan = (uint32)(delta >> 12) + 1024;
        }
        if (pan == 2048)
            pan = 2047;
    }
    *pan_output = (((pan << 3) + pan) >> 11) + 1;
    return 1;
}

uint32 PlaySpatialSound(const SoundSample *sound, sint32 volume, uint32 pan, uint32 pitch, const uint32 *position, uint32 radius, uint32 voice, uint32 mode, uint32 option)
{
    FUNCTION_MARKER(0x8005EB18, "SLUS_005.78");
    uint32 level_word, pan_word;
    uint32 *attenuation;
    uint32 channel, active, value, pan_value, scaled;
    sint32 level, selected;
    pan_word = pan;
    level = volume;
    if (pan == 0)
    {
        attenuation = volume < 4097 ? 0 : &level_word;
        value = ComputeSpatialSoundControls(position, radius, attenuation, &pan_word);
        if ((value << 24) == 0)
            goto silent;
        if (volume >= 4097)
        {
            value = (((uint32)volume - 1) & 0xFFF) + 1;
            level = (sint32)(level_word * value) >> 12;
            if (level == 0)
                goto silent;
        }
    }
    scaled = (uint32)level * (uint32)(sint32)(sint16)(sound_settings.effects_volume);
    level = (sint32)scaled >> 12;
    level_word = (uint32)level;
    if (level == 0)
        goto silent;
    if (volume >= 8193)
        pan_word = 5;
    if ((sint32)voice >= 0 && voice < 8 && sound_effects.voices[voice].sound == sound)
    {
        channel = voice & 0xFFFF;
        active = IsSoundVoicePlaying((uint16)channel);
        if (active != 0)
        {
            level = (sint32)level_word >> 4;
            SetSoundVoiceVolume((uint16)channel, (uint16)level);
            SetSoundVoicePan((uint16)channel, (uint16)pan_word);
            SetSoundVoicePitch((uint16)channel, pitch);
            return voice;
        }
        if (mode == 0)
        {
            return 0xFFFFFFFF;
        }
    }
    selected = sound_effects.SelectVoice(0xFFFFFFFF, sound, option);
    if (selected >= 0)
    {
        level = (sint32)level_word >> 4;
        pan_value = pan_word;
        if (level != 0)
        {
            value = pitch;
            if (value == 0)
                value = ((const SoundSample *)(sound))->pitch;
            pan_value = (pan_value & 0xFFFF) != 0 ? pan_value - 1 : 4;
            channel = (uint32)selected & 0xFFFF;
            SetSoundVoiceVolume((uint16)channel, (uint16)level);
            SetSoundVoicePan((uint16)channel, (uint16)pan_value);
            SetSoundVoicePitch((uint16)channel, value);
            StartSoundVoice((uint16)channel, sound);
        }
    }
    return (uint32)selected;

silent:
    if ((sint32)voice >= 0 && voice < 8 && sound_effects.voices[voice].sound == sound)
        StopSoundVoice((uint16)voice);
    return 0xFFFFFFFF;
}

void StopSceneSounds(void)
{
    FUNCTION_MARKER(0x80025F58, "SLUS_005.78");
    SceneNode *registered, *node;
    uint32 kind, flags;
    const SceneSection *section;
    registered = registered_object_head;
    while (registered != 0)
    {
        if (((const sint8 *)registered)[125] == 6)
            ((AnimatedObject *)registered)->StopModelGroupSound();
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
                if (((flags >> 17) & 0xFF) == 0 && (flags >> 25) == 0 && control->playback.bytes[0] < 8)
                    trigger->StopSound();
            }
            node = node->links.registry_next;
        } while (node != 0);
        section = scene_stream.active_section;
    }
    node = section->object_heads[1];
    while (node != 0)
    {
        ((AnimatedPathState *)(((const ActorPlacement *)((((const AnimatedObject *)node)->path_definition) != 0 ? (((const AnimatedObject *)node)->path_definition) : (const PathDefinition *)0))->state != 0 ? ((const ActorPlacement *)((((const AnimatedObject *)node)->path_definition) != 0 ? (((const AnimatedObject *)node)->path_definition) : (const PathDefinition *)0))->state : 0))->StopSound();
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
