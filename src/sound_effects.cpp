#include "game.h"

SoundEffects sound_effects = {{{0}}, {{0}}, 0, {0, 1, 2, 3, 4, 5, 6, 7}};

uint32 SoundEffects::Queue(uint32 sound_offset, uint32 handle, uint32 pitch, uint32 volume, uint32 pan, uint32 value)
{
    FUNCTION_MARKER(0x8005E538, "SLUS_005.78");
    uint32 index, count, result, offset;
    if (handle == 0)
        index = pending_count;
    else
    {
        count = pending_count;
        index = 0;
        while (index < count)
        {
            if (commands[index].sound == handle)
            {
                result = index < 8;
                if (sound_offset == handle)
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
    commands[index].sound = sound_offset;
    commands[index].previous_sound = handle;
    commands[index].pitch = pitch;
    commands[index].volume = (unsigned short)(volume);
    commands[index].pan = (unsigned short)(pan);
    commands[index].priority = value;
    return offset;
}

sint32 SoundEffects::SelectVoice(uint32 requested, uint32 sound, uint32 priority)
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
                if (p2_func_8006a940((uint16)voice) == 0)
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
    for (uint32 i = index; i + 1 < 8; ++i) voice_order[i] = voice_order[i + 1];
finish:
    voice_order[7] = (unsigned char)(voice);
    return (sint32)voice;
}

uint32 SoundEffects::IsPlaying(uint32 sound, uint32 voice) const
{
    FUNCTION_MARKER(0x8005E868, "SLUS_005.78");
    if (voice < 8 && voices[voice].sound == sound)
        return p2_func_8006a940((uint16)voice) != 0;
    return 0;
}

uint32 SoundEffects::Stop(uint32 all_voices)
{
    FUNCTION_MARKER(0x8005ED74, "SLUS_005.78");
    uint32 voice;
    if (all_voices != 0)
    {
        pending_count = 0;
        for (voice = 0; voice < 8; ++voice) voices[voice] = SoundVoice();
        for (voice = 0; voice < 8; ++voice)
        {
            p2_func_8006a970((uint16)voice);
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
    uint32 count, requested, previous, cached, skip, sound, pitch, volume, pan, channel;
    sint32 selected;
    count = pending_count;
    pending_count = 0;
    while (count != 0)
    {
        SoundCommand &command = commands[--count];
        previous = command.previous_sound;
        requested = 8;
        skip = 0;
        if (previous != 0 && previous != 0xFFFFFFFF)
        {
            for (requested = 0; requested < 8; ++requested)
            {
                cached = voices[requested].sound;
                if (cached == command.previous_sound)
                {
                    if (cached == command.sound && frame_clock.render_tick - voices[requested].started_tick < 60 && p2_func_8006a940((uint16)requested) != 0)
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
                    pitch = r_u16(sound + 4);
                pan = pan != 0 ? pan - 1 : 4;
                channel = (uint32)selected & 0xFFFF;
                p2_func_8006a740((uint16)channel, (uint16)volume);
                p2_func_8006a638((uint16)channel, (uint16)pan);
                p2_func_8006a814((uint16)channel, pitch);
                p2_func_8006a878((uint16)channel, sound);
            }
        }
    }
}
