#include "scene_trigger.h"
#include "game.h"
#include "scene_events.h"

void SceneTrigger::UpdateExtentsFromAnimation()
{
    FUNCTION_MARKER(0x8001BB58, "SLUS_005.78");
    const AnimatedObject *owner = animation_owner;
    uint32 result = owner->flags & 32;
    if (result == 0)
    {
        const AnimationHeader *animation = owner->animation;
        if (animation != 0)
        {
            sint32 idx = animation_binding.tracks[2];
            if (idx >= 0)
            {
                const AnimationHeader *header = animation;
                AnimationTrack *track = header->tracks[2].recs + idx;
                uint8 flags = track->flags;
                result = flags & 0xF7;
                if ((flags & 8) != 0)
                {
                    sint32 frame = track->frame;
                    Vec3 output;
                    uint32 x;
                    uint32 y;
                    uint32 z;
                    track->flags = result;
                    AnimationTrack::SampleVectorTrack(frame, track, output);
                    extent[0] = output.x >> 5;
                    x = extent[0];
                    extent[1] = output.y >> 5;
                    z = output.z;
                    y = extent[1];
                    radius = x;
                    extent[2] = (sint32)z >> 5;
                    if (x < y)
                        radius = y;
                    z = extent[2];
                    result = radius < z;
                    if (result != 0)
                        radius = z;
                }
            }
        }
    }
}

uint32 SceneTrigger::PlaySound()
{
    FUNCTION_MARKER(0x8005C40C, "SLUS_005.78");
    SceneSoundControl *control = &parameters.sound;
    uint32 idx, result;
    const SoundBankStorage *table;
    const SoundSample *sound = 0;
    if ((node.state_flags & 2) == 0 && animation_binding.tracks[2] >= 0)
        UpdateExtentsFromAnimation();
    table = sound_resources.scene_table;
    idx = control->sample_id & 4095;
    if (table != 0 && idx < (uint32)table->header.count)
        sound = table->samples + idx;
    /* An invalid sound identifier must not read the null guest descriptor */
    if (sound == 0)
        return 0;
    result = SoundEffects::PlaySpatialSound(sound, control->volume_flags & 0x3FFF, control->pan, sound->pitch, transform.t, (uint32)radius << 5, control->voice, 1, 1) & 255;
    control->voice = result;
    return result;
}

uint32 SceneTrigger::StopSound()
{
    FUNCTION_MARKER(0x8005C4F4, "SLUS_005.78");
    SceneSoundControl *control = &parameters.sound;
    uint32 voice, idx, result;
    const SoundBankStorage *table;
    const SoundSample *sound = 0;
    table = sound_resources.scene_table;
    voice = control->voice;
    if (table != 0)
    {
        idx = control->sample_id;
        result = table->header.count;
        idx &= 0xFFF;
        if (idx < result)
            sound = table->samples + idx;
    }
    result = sound_effects.IsPlaying(sound, voice);
    if ((result << 24) != 0)
        SoundEffects::StopSoundVoice(voice);
    control->voice = 255;
    result = ((uint32)control->volume_flags << 16) | ((uint32)control->pan << 8) | control->voice;
    return result;
}

void SceneTrigger::ProcessSceneTriggers(SceneNode *objects, SceneNode *auxiliary)
{
    SceneNode *node, *candidate;

    const SceneSection *section;
    FUNCTION_MARKER(0x8005C588, "SLUS_005.78");
    CollisionSphere bounds;
    Vec3 delta;
    uint32 extent;
    uint32 attenuation, pan, flags, active, val, x, y, z, radius, limit, packed, delay, count, kind, idx, voice, divisor, pitch, result;
    SceneTrigger *trigger, *other;
    const SoundBankStorage *sound_table;
    const SoundSample *sound;
    sint32 volume, queue_count;
    for (node = auxiliary; node != 0; node = trigger->node.links.registry_next)
    {
        trigger = (SceneTrigger *)node;
        if ((trigger->node.flags & 0x81) != 0)
            continue;
        flags = trigger->node.state_flags & 0xFF;
        trigger->node.state_flags = (trigger->node.state_flags & 0xFF00) | (uint8)(flags & 0xBF);
        x = trigger->transform.t[0];
        y = trigger->transform.t[1];
        z = trigger->transform.t[2];
        bounds.pos.x = x;
        bounds.pos.y = y;
        bounds.pos.z = z;
        val = trigger->radius;
        section = scene_stream.active_section;
        bounds.radius = val << 5;
        candidate = section->object_heads[4];
        active = (flags >> 6) & 1;
        for (; candidate != 0; candidate = other->node.links.registry_next)
        {
            other = (SceneTrigger *)candidate;
            kind = (other->parameters.event.schedule >> 12) & 0x1F;
            if (kind - 3 >= 2)
                break;
            if ((other->node.flags & 0x81) != 0 || ((other->parameters.event.command >> 24) & 0xFE) != 0)
                continue;
            if (active == 0 && ((other->node.state_flags & 0xFF) & 0x40) == 0)
                continue;
            radius = other->radius;
            radius = bounds.radius + (radius << 5) + 20480;
            val = bounds.pos.x;
            val = val - other->transform.t[0] + radius;
            limit = radius << 1;
            extent = radius;
            if (val > limit)
                continue;
            val = bounds.pos.y;
            val = val - other->transform.t[1] + radius;
            if (val > limit)
                continue;
            val = bounds.pos.z;
            val = val - other->transform.t[2] + radius;
            if (val > limit)
                continue;
            val = trigger->delta.x;
            val -= (uint32)other->delta.x;
            delta.x = val;
            val = trigger->delta.y;
            val -= (uint32)other->delta.y;
            delta.y = val;
            val = trigger->delta.z;
            val -= (uint32)other->delta.z;
            delta.z = val;
            if (AnimatedObject::SweepSphereAgainstObject(&bounds, &other->node, delta, &extent, 0) != 0)
            {
                other->node.state_flags = (other->node.state_flags & 0xFF00) | (uint8)((other->node.state_flags & 0xFF) | 8);
                trigger->node.state_flags = (trigger->node.state_flags & 0xFF00) | (uint8)((trigger->node.state_flags & 0xFF) | 0x40);
            }
        }
    }
    for (node = objects; node != 0; node = trigger->node.links.registry_next)
    {
        trigger = (SceneTrigger *)node;
        flags = trigger->node.state_flags & 0xFF;
        kind = trigger->node.state_flags >> 8;
        flags &= 0xBF;
        trigger->node.state_flags = (trigger->node.state_flags & 0xFF00) | (uint8)flags;
        if (kind - 40 >= 16)
            break;
        packed = trigger->parameters.event.command;
        delay = (packed >> 25) * 2;
        count = (packed >> 17) & 0xFF;
        if (delay != 0)
        {
            --delay;
            if (delay == 0)
                trigger->node.state_flags = (trigger->node.state_flags & 0xFF00) | (uint8)(flags | 0x40);
            val = trigger->parameters.event.command & 0x1FFFF;
            trigger->parameters.event.command = val | (count << 17) | ((delay >> 1) << 25);
        }
        flags = trigger->node.state_flags & 0xFF;
        if ((flags & 8) != 0)
            goto activate;
        packed = trigger->parameters.event.schedule;
        if (((packed >> 12) & 0x1F) != 1 || (trigger->node.flags & 0x81) != 0 || delay != 0)
            goto stop_voice;
        if ((flags & 0x10) == 0)
            goto activate;
        count = (packed >> 17) & 0xFF;
        if (count == 0)
            goto activate;
        result = random_generator.Next();
        count = result % (count * 15);
        if (count != 0)
            continue;
    activate:
        packed = trigger->parameters.event.schedule;
        flags = (trigger->node.state_flags & 0xFF) | 0x48;
        delay = (packed >> 25) * 2;
        trigger->node.state_flags = (trigger->node.state_flags & 0xFF00) | (uint8)flags;
        if (count != 0)
        {
            --count;
            val = trigger->parameters.event.command & 0x1FFFF;
            trigger->parameters.event.command = val | (count << 17) | (delay << 24);
            if (delay == 254)
                trigger->node.flags = trigger->node.flags | 0x80;
            continue;
        }
        kind = (sint8)(trigger->node.state_flags >> 8);
        count = (packed >> 17) & 0xFF;
        trigger->node.state_flags = (trigger->node.state_flags & 0xFF00) | (uint8)(flags & 0xF7);
        if (kind == 55)
        {
            game_flow.transition_flags = game_flow.transition_flags | 0x8000;
            goto reset;
        }
        if (kind != 50)
        {
            packed = trigger->parameters.event.schedule;
            val = trigger->parameters.event.command;
            queue_count = scene_events.count;
            kind = ((packed >> 12) & 0x1F) - 1;
            if (queue_count < 40)
            {
                scene_events.count = queue_count + 1;
                scene_events.entries[queue_count] = val | (kind << 17);
            }
            goto reset;
        }
        if (delay == 0 && count == 0)
        {
            if ((flags & 2) == 0 && trigger->animation_binding.tracks[2] >= 0)
                trigger->UpdateExtentsFromAnimation();
            sound_table = sound_resources.scene_table;
            sound = 0;
            if (sound_table != 0)
            {
                idx = trigger->parameters.event.command & 0xFFF;
                if (idx < (uint32)sound_table->header.count)
                    sound = sound_table->samples + idx;
            }
            pitch = sound->pitch;
            radius = trigger->radius << 5;
            result = SoundEffects::PlaySpatialSound(sound, trigger->parameters.sound.volume_flags & 0x3FFF, trigger->parameters.sound.pan, pitch, trigger->transform.t, radius, trigger->parameters.sound.voice, 1, 1);
            trigger->parameters.sound.voice = result & 0xFF;
            goto reset;
        }
        volume = trigger->parameters.sound.volume_flags & 0x3FFF;
        if (volume == 0)
            goto reset;
        if ((trigger->parameters.sound.volume_flags & 0x4000) != 0)
        {
            divisor = (((uint32)volume - 1) & 0xFFF) >> 1;
            if (divisor != 0)
                volume = volume - random_generator.Next() % divisor;
        }
        val = trigger->parameters.sound.pan;
        pan = val;
        if (val == 0)
        {
            if (((trigger->node.state_flags & 0xFF) & 2) == 0 && trigger->animation_binding.tracks[2] >= 0)
                trigger->UpdateExtentsFromAnimation();
            radius = trigger->radius << 5;
            SoundEffects::ComputeSpatialSoundControls(trigger->transform.t, radius, volume >= 4097 ? &attenuation : 0, &pan);
            if (volume >= 4097)
            {
                if (volume >= 8193)
                {
                    volume -= 4096;
                    pan = 5;
                }
                val = attenuation * ((uint32)volume - 4096);
                volume = (sint32)val >> 12;
            }
        }
        volume = (sint32)((uint32)volume * sound_settings.effects_volume) >> 12;
        if (volume == 0)
            goto reset;
        sound_table = sound_resources.scene_table;
        sound = 0;
        if (sound_table != 0)
        {
            idx = trigger->parameters.event.command & 0xFFF;
            if (idx < (uint32)sound_table->header.count)
                sound = sound_table->samples + idx;
        }
        pitch = 0;
        if ((trigger->parameters.sound.volume_flags & 0x8000) != 0)
        {
            pitch = sound->pitch;
            divisor = pitch >> 2;
            if (divisor != 0)
            {
                result = random_generator.Next();
                pitch += result % divisor - (pitch >> 3);
            }
        }
        val = (uint16)pan;
        sound_effects.Queue(sound, 0, pitch, (uint32)volume & 0xFFFF, val, 0);
    reset:
        if (delay == 254)
            trigger->node.flags = trigger->node.flags | 0x80;
        else
            trigger->parameters.event.command = (trigger->parameters.event.command & 0x1FFFF) | (count << 17) | ((delay >> 1) << 25);
        continue;
    stop_voice:
        if ((sint8)(trigger->node.state_flags >> 8) != 50)
            continue;
        packed = trigger->parameters.event.schedule;
        if (((packed >> 17) & 0xFF) != 0 || (packed >> 25) != 0)
            continue;
        voice = trigger->parameters.sound.voice;
        if (voice >= 8)
            continue;
        sound_table = sound_resources.scene_table;
        sound = 0;
        if (sound_table != 0)
        {
            idx = trigger->parameters.event.command & 0xFFF;
            if (idx < (uint32)sound_table->header.count)
                sound = sound_table->samples + idx;
        }
        if ((sound_effects.IsPlaying(sound, voice) & 0xFF) != 0)
            SoundEffects::StopSoundVoice(voice);
        trigger->parameters.sound.voice = 255;
    }
}
