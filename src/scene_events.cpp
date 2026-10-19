#include "game.h"
#include "original_defaults.h"

SceneEvents scene_events = {0};

uint32 EncodeObjectName(sint32 first, sint32 second)
{
    FUNCTION_MARKER(0x8005BB90, "SLUS_005.78");
    uint32 high, low;
    if (first == 95 || second == 95)
        return 0;
    if ((uint32)first - 48 < 10)
        high = (uint32)first - 48;
    else if ((uint32)first - 97 < 26)
        high = (uint32)first - 87;
    else if ((uint32)first - 65 < 26)
        high = (uint32)first - 29;
    else
        return 0xFFFFFFFF;
    if ((uint32)second - 48 < 10)
        low = (uint32)second - 48;
    else if ((uint32)second - 97 < 26)
        low = (uint32)second - 87;
    else if ((uint32)second - 65 < 26)
        low = (uint32)second - 29;
    else
        return 0xFFFFFFFF;
    return high * 62 + low + 1;
}

uint32 QueueSceneEvent(uint32 event, uint32 kind)
{
    FUNCTION_MARKER(0x8005BB48, "SLUS_005.78");
    sint32 count = (sint8)scene_events.count;
    uint32 result = count < 40;
    if (result != 0)
    {
        scene_events.count = count + 1;
        result = event | (kind << 17);
        scene_events.entries[count] = result;
    }
    return result;
}

uint32 ApplySceneEvents(uint32 *const *destinations, sint32 count)
{
    uint32 result = 0;
    uint32 index = 0;
    uint32 value;
    uint32 key;
    const uint32 *cursor;
    uint32 remaining;
    FUNCTION_MARKER(0x8005BC58, "SLUS_005.78");
    if ((sint8)scene_events.count == 0)
        return 0;
    if (count > 0)
    {
        do
        {
            uint32 *destination = destinations[index];
            key = *destination & 0xFFF;
            if (key != 0)
            {
                *destination = key;
                value = scene_events.entries[0];
                remaining = (uint32)(sint32)(sint8)scene_events.count;
                cursor = scene_events.entries + 1;
                for (;;)
                {
                    --remaining;
                    if (key == (value & 0xFFF))
                    {
                        *destination = value;
                        result = 1;
                        break;
                    }
                    if (remaining == 0)
                        break;
                    value = *cursor;
                    ++cursor;
                }
            }
            ++index;
        } while ((sint32)index < count);
    }
    return result;
}

// Retain the original switch values carried by unhandled event actions
static const uint32 animated_event_dispatch_values[15] = {0x80042138, 0x800422E8, 0x80042154, 0x8004229C, 0x8004216C, 0x800422E8, 0x8004219C, 0x800422E8, 0x80042210, 0x800422E8, 0x800422E8, 0x80042288, 0x800422E8, 0x80042154, 0x8004229C};

uint32 AnimatedObject::ApplyEvents()
{
    FUNCTION_MARKER(0x80042024, "SLUS_005.78");
    uint32 scene = OriginalDefaults::SceneContextIdentity, result = scene;
    uint32 remaining = (uint32)(sint32)(sint8)scene_events.count;
    const uint32 *events;
    uint32 count, kind, object_id, entry, action, subtype, impulse, flags;
    uint32 *speed;
    const AnimatedModelDefinition *definition = (const AnimatedModelDefinition *)0;
    if (remaining == 0)
        return result;
    count = animated_models.count;
    AnimatedModelDefinition *const *definitions = animated_models.definitions;
    kind = this->state;
    while (count != 0)
    {
        definition = *definitions;
        if ((uint32)(sint32)definition->kind_id == kind)
            break;
        ++definitions;
        --count;
    }
    if (count == 0)
        definition = (const AnimatedModelDefinition *)0;
    object_id = this->event_word & 0xFFF;
    events = scene_events.entries;
    speed = ((AnimatedPathState *)(((const ActorPlacement *)((const PathDefinition *)((this->path_definition))))->state != 0 ? ((const ActorPlacement *)((const PathDefinition *)((this->path_definition))))->state : 0))->speed;
    do
    {
        entry = *events;
        ++events;
        result = entry & 0xFFF;
        if (object_id == result)
        {
            action = ((entry >> 12) & 0x1F) + 39;
            subtype = (entry >> 17) & 0xFF;
            if (((uint8)this->node.flags & 1) != 0 && action != 43)
                action = 42;
            result = action - 40 < 15;
            if (result != 0)
            {
                result = animated_event_dispatch_values[action - 40];
                switch (action)
                {
                    case 40:
                        result = (uint32)(sint32)(sint8)this->movement_mode;
                        if (result != 3)
                        {
                            result = definition->initial_speed;
                            speed[0] = result;
                            goto moving;
                        }
                        break;
                    case 42:
                    case 53:
                        result = this->flags | 0x4000;
                        flags = (uint8)this->node.flags | 1;
                        this->flags = result;
                        this->node.flags = flags;
                        goto stop_motion;
                    case 44:
                    stop_motion:
                        result = (uint32)(sint32)(sint8)this->movement_mode;
                        if (result != 0)
                        {
                            this->velocity[2] = 0;
                            this->velocity[1] = 0;
                            this->velocity[0] = 0;
                            speed[2] = 0;
                            speed[1] = 0;
                            speed[0] = 0;
                            this->movement_mode = 0;
                        }
                        break;
                    case 46:
                        impulse = definition->impulse;
                        result = 11;
                        if (impulse == 0)
                            break;
                        if (subtype == 11)
                        {
                            flags = this->flags;
                            result = flags & 0x400;
                            if ((flags & 0x4000) == 0)
                            {
                                if (result == 0)
                                    CapturePathVelocity();
                                result = speed[1] + definition->impulse;
                                speed[1] = result;
                            }
                        }
                        else
                        {
                            result = (this->flags & 0x10) != 0 ? speed[0] + impulse : speed[0] - impulse;
                            speed[0] = result;
                        }
                        goto moving;
                    case 48:
                        impulse = definition->impulse;
                        result = 10;
                        if (impulse == 0)
                            break;
                        if (subtype == 10)
                        {
                            result = this->flags & 0x400;
                            if (result != 0)
                            {
                                result = speed[1] - impulse;
                                speed[1] = result;
                            }
                        }
                        else
                        {
                            result = (this->flags & 0x10) != 0 ? speed[0] - impulse : speed[0] + impulse;
                            speed[0] = result;
                        }
                    moving:
                        this->movement_mode = 3;
                        break;
                    case 51:
                        if (((uint8)this->node.flags & 1) == 0)
                        {
                            result = this->flags | 0x4000;
                            flags = (uint8)this->node.flags | 1;
                            this->flags = result;
                            this->node.flags = flags;
                            break;
                        }
                        goto activate;
                    case 43:
                    case 54:
                    activate:
                        this->flags = this->flags | 0x80000;
                        result = (uint8)this->node.flags;
                        flags = this->flags;
                        result &= 0xFE;
                        flags &= 0xFFDFFFFF;
                        this->node.flags = result;
                        this->flags = flags;
                        break;
                }
            }
        }
        --remaining;
    } while (remaining != 0);
    return result;
}

void UpdateSceneObjects(void)
{
    SceneNode *node;
    SceneSection *section;
    FUNCTION_MARKER(0x8005CD24, "SLUS_005.78");
    uint32 flags = game_flow.transition_flags;
    SceneNode *registered;
    const sint8 *registered_bytes;
    uint32 pending;
    scene_events.count = 0;
    if ((flags & 0x40) != 0)
    {
        game_flow.transition_flags = flags & 0xFFFFFFBF;
        ApplySceneSectionTransition();
    }
    if (screen_render.fade_level < 0x1000)
    {
        pending = scene_events.pending;
        if (pending != 0xFFFFFFFF)
        {
            sint32 count = (sint8)scene_events.count;
            if (count < 40)
            {
                scene_events.count = count + 1;
                scene_events.entries[count] = pending | 0x80000;
            }
            scene_events.pending = 0xFFFFFFFF;
        }
        section = scene_stream.active_section;
        node = section->object_heads[4];
        ProcessSceneTriggers(node, section->object_heads[7]);
        section = scene_stream.active_section;
        ProcessSceneTriggers(section->object_heads[5], 0);
        FinishCharacterContacts();
        registered = registered_object_head;
        while (registered != 0)
        {
            registered_bytes = (const sint8 *)registered;
            if (((registered->flags & 0x81) == 0 || registered_bytes[126] == 1) && registered_bytes[126] != 0 && (registered_bytes[126] != 3 || xport_load_le32(registered_bytes + 116) != 30))
                ((AnimatedObject *)registered)->UpdateObjectCollisions();
            registered = registered->links.registry_next;
        }
        section = scene_stream.active_section;
        node = section->object_heads[1];
        while (node != 0)
        {
            if ((node->flags & 0x81) == 0 && (sint8)((const AnimatedObject *)node)->metadata126[0] != 0)
                ((AnimatedObject *)node)->UpdateObjectCollisions();
            node = node->links.registry_next;
        }
        UpdateSceneEventObjects();
    }
    section = scene_stream.active_section != 0 ? scene_stream.active_section : 0;
    DrawSceneFrame(section);
    sound_effects.Flush();
}

void ProcessSceneTriggers(SceneNode *objects, SceneNode *auxiliary)
{
    SceneNode *node, *candidate;

    const SceneSection *section;
    FUNCTION_MARKER(0x8005C588, "SLUS_005.78");
    CollisionSphere bounds;
    uint32 delta[3], extent;
    uint32 attenuation, pan, flags, active, value, x, y, z, radius, limit, packed, delay, count, kind, index, voice, divisor, pitch, result;
    SceneTrigger *trigger, *other;
    const SoundBankStorage *sound_table;
    const SoundSample *sound;
    sint32 volume, queue_count;
    for (node = auxiliary; node != 0; node = trigger->node.links.registry_next)
    {
        trigger = (SceneTrigger *)node;
        if ((trigger->node.flags & 0x81) != 0)
            continue;
        flags = (trigger->node.state_flags & 0xFF);
        trigger->node.state_flags = (trigger->node.state_flags & 0xFF00) | (uint8)(flags & 0xBF);
        x = trigger->transform.t[0];
        y = trigger->transform.t[1];
        z = trigger->transform.t[2];
        bounds.position[0] = x;
        bounds.position[1] = y;
        bounds.position[2] = z;
        value = trigger->radius;
        section = scene_stream.active_section;
        bounds.radius = value << 5;
        candidate = section->object_heads[4];
        active = (flags >> 6) & 1;
        for (; candidate != 0; candidate = other->node.links.registry_next)
        {
            other = (SceneTrigger *)candidate;
            kind = (other->parameters.words[1] >> 12) & 0x1F;
            if (kind - 3 >= 2)
                break;
            if ((other->node.flags & 0x81) != 0 || ((other->parameters.words[0] >> 24) & 0xFE) != 0)
                continue;
            if (active == 0 && ((other->node.state_flags & 0xFF) & 0x40) == 0)
                continue;
            radius = other->radius;
            radius = bounds.radius + (radius << 5) + 20480;
            value = bounds.position[0];
            value = value - other->transform.t[0] + radius;
            limit = radius << 1;
            extent = radius;
            if (value > limit)
                continue;
            value = bounds.position[1];
            value = value - other->transform.t[1] + radius;
            if (value > limit)
                continue;
            value = bounds.position[2];
            value = value - other->transform.t[2] + radius;
            if (value > limit)
                continue;
            for (index = 0; index < 3; ++index)
            {
                value = trigger->delta[index];
                value -= other->delta[index];
                delta[index] = value;
            }
            if (SweepSphereAgainstObject(&bounds, &other->node, delta, &extent, 0) != 0)
            {
                other->node.state_flags = (other->node.state_flags & 0xFF00) | (uint8)((other->node.state_flags & 0xFF) | 8);
                trigger->node.state_flags = (trigger->node.state_flags & 0xFF00) | (uint8)((trigger->node.state_flags & 0xFF) | 0x40);
            }
        }
    }
    for (node = objects; node != 0; node = trigger->node.links.registry_next)
    {
        trigger = (SceneTrigger *)node;
        flags = (trigger->node.state_flags & 0xFF);
        kind = (trigger->node.state_flags >> 8);
        flags &= 0xBF;
        trigger->node.state_flags = (trigger->node.state_flags & 0xFF00) | (uint8)(flags);
        if (kind - 40 >= 16)
            break;
        packed = trigger->parameters.words[0];
        delay = (packed >> 25) * 2;
        count = (packed >> 17) & 0xFF;
        if (delay != 0)
        {
            --delay;
            if (delay == 0)
                trigger->node.state_flags = (trigger->node.state_flags & 0xFF00) | (uint8)(flags | 0x40);
            value = trigger->parameters.words[0] & 0x1FFFF;
            trigger->parameters.words[0] = value | (count << 17) | ((delay >> 1) << 25);
        }
        flags = (trigger->node.state_flags & 0xFF);
        if ((flags & 8) != 0)
            goto activate;
        packed = trigger->parameters.words[1];
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
        packed = trigger->parameters.words[1];
        flags = (trigger->node.state_flags & 0xFF) | 0x48;
        delay = (packed >> 25) * 2;
        trigger->node.state_flags = (trigger->node.state_flags & 0xFF00) | (uint8)(flags);
        if (count != 0)
        {
            --count;
            value = trigger->parameters.words[0] & 0x1FFFF;
            trigger->parameters.words[0] = value | (count << 17) | (delay << 24);
            if (delay == 254)
                trigger->node.flags = trigger->node.flags | 0x80;
            continue;
        }
        kind = (uint32)(sint8)(trigger->node.state_flags >> 8);
        count = (packed >> 17) & 0xFF;
        trigger->node.state_flags = (trigger->node.state_flags & 0xFF00) | (uint8)(flags & 0xF7);
        if (kind == 55)
        {
            game_flow.transition_flags = game_flow.transition_flags | 0x8000;
            goto reset;
        }
        if (kind != 50)
        {
            packed = trigger->parameters.words[1];
            value = trigger->parameters.words[0];
            queue_count = (sint8)scene_events.count;
            kind = ((packed >> 12) & 0x1F) - 1;
            if (queue_count < 40)
            {
                scene_events.count = queue_count + 1;
                scene_events.entries[queue_count] = value | (kind << 17);
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
                index = trigger->parameters.words[0] & 0xFFF;
                if (index < sound_table->header.count)
                    sound = sound_table->samples + index;
            }
            packed = trigger->parameters.words[2];
            pitch = (((const SoundSample *)(sound)))->pitch;
            radius = trigger->radius << 5;
            result = PlaySpatialSound(sound, (sint32)((packed >> 16) & 0x3FFF), (packed >> 8) & 0xFF, pitch, (const uint32 *)trigger->transform.t, radius, packed & 0xFF, 1, 1);
            trigger->parameters.words[2] = (trigger->parameters.words[2] & 0xFFFFFF00) | (result & 0xFF);
            goto reset;
        }
        packed = trigger->parameters.words[2];
        volume = (sint32)((packed >> 16) & 0x3FFF);
        if (volume == 0)
            goto reset;
        if ((packed & 0x40000000) != 0)
        {
            divisor = (((uint32)volume - 1) & 0xFFF) >> 1;
            if (divisor != 0)
                volume = (sint32)((uint32)volume - random_generator.Next() % divisor);
        }
        value = trigger->parameters.sound.playback.bytes[1];
        pan = value;
        if (value == 0)
        {
            if (((trigger->node.state_flags & 0xFF) & 2) == 0 && trigger->animation_binding.tracks[2] >= 0)
                trigger->UpdateExtentsFromAnimation();
            radius = trigger->radius << 5;
            ComputeSpatialSoundControls((const uint32 *)trigger->transform.t, radius, volume >= 4097 ? &attenuation : 0, &pan);
            if (volume >= 4097)
            {
                if (volume >= 8193)
                {
                    volume -= 4096;
                    pan = 5;
                }
                value = attenuation * ((uint32)volume - 4096);
                volume = (sint32)value >> 12;
            }
        }
        volume = (sint32)((uint32)volume * (uint32)(sint32)(sint16)(sound_settings.effects_volume)) >> 12;
        if (volume == 0)
            goto reset;
        sound_table = sound_resources.scene_table;
        sound = 0;
        if (sound_table != 0)
        {
            index = trigger->parameters.words[0] & 0xFFF;
            if (index < sound_table->header.count)
                sound = sound_table->samples + index;
        }
        pitch = 0;
        if ((sint32)trigger->parameters.words[2] < 0)
        {
            pitch = (((const SoundSample *)(sound)))->pitch;
            divisor = pitch >> 2;
            if (divisor != 0)
            {
                result = random_generator.Next();
                pitch += result % divisor - (pitch >> 3);
            }
        }
        value = (uint16)pan;
        sound_effects.Queue(sound, 0, pitch, (uint32)volume & 0xFFFF, value, 0);
    reset:
        if (delay == 254)
            trigger->node.flags = trigger->node.flags | 0x80;
        else
            trigger->parameters.words[0] = (trigger->parameters.words[0] & 0x1FFFF) | (count << 17) | ((delay >> 1) << 25);
        continue;
    stop_voice:
        if ((sint8)(trigger->node.state_flags >> 8) != 50)
            continue;
        packed = trigger->parameters.words[1];
        if (((packed >> 17) & 0xFF) != 0 || (packed >> 25) != 0)
            continue;
        voice = trigger->parameters.sound.playback.bytes[0];
        if (voice >= 8)
            continue;
        sound_table = sound_resources.scene_table;
        sound = 0;
        if (sound_table != 0)
        {
            index = trigger->parameters.words[0] & 0xFFF;
            if (index < sound_table->header.count)
                sound = sound_table->samples + index;
        }
        if ((sound_effects.IsPlaying(sound, voice) & 0xFF) != 0)
            StopSoundVoice((uint16)voice);
        trigger->parameters.words[2] = trigger->parameters.words[2] | 0xFF;
    }
}

struct SceneEventCandidate
{
    SceneEventCandidate *next;
    ActorBinding *binding;
    uint32 score;
};

void UpdateSceneEventObjects(void)
{
    const SceneSection *section;
    FUNCTION_MARKER(0x8005BFF4, "SLUS_005.78");
    const ActorModelDefinition *cost_model;
    SceneEventCandidate candidates[50];
    SceneEventCandidate *record = candidates;
    SceneEventCandidate *limit = candidates + 50;
    SceneEventCandidate *head = 0, *cursor, *previous;
    uint32 target_delta[3];
    ActorBinding *binding;
    AnimatedObject *instance;
    ActorPlacement *placement;
    const uint32 *event_source;
    uint32 count, value, score;
    uint32 x, y, z, flags, result, slots, budget, cost, rank;
    if ((sint8)scene_events.deferred_count != 0)
    {
        event_source = scene_events.deferred_entries;
        do
        {
            count = (uint32)(sint32)(sint8)scene_events.count;
            value = *event_source;
            ++event_source;
            if ((sint32)count < 40)
            {
                scene_events.count = count + 1;
                scene_events.entries[count] = value | 0x20000;
            }
            count = scene_events.deferred_count - 1;
            scene_events.deferred_count = count;
        } while ((count & 0xFF) != 0);
    }
    section = scene_stream.active_section;
    for (binding = section->actor_bindings; binding != 0; binding = binding->next)
    {
        placement = ((ActorPlacement *)(binding->definition));
        if ((placement->path.node.flags & 0x80) != 0)
        {
            result = binding->respawn_delay;
            if (result == 0)
                continue;
            value = binding->respawn_ticks - 1;
            binding->respawn_ticks = value;
            result = value & 0xFFFF;
            if (result != 0)
                continue;
            value = binding->initial_path_frame;
            placement = ((ActorPlacement *)(binding->definition));
            binding->path_frame = value;
            placement->path.flags &= 0xFFFFFFFB;
            placement = ((ActorPlacement *)(binding->definition));
            placement->path.node.flags &= 0x7F;
        }
        placement = ((ActorPlacement *)(binding->definition));
        if ((placement->path.node.flags & 1) != 0)
        {
            result = placement->UpdateEvents();
            continue;
        }
        value = binding->distance_squared;
        if (value == 0x7FFFF000)
            value = binding->ComputeInteractionDistance();
        record->score = value;
        if ((character_action_flags & 0x200) != 0)
        {
            instance = binding->instance;
            if (instance != 0 && ((sint8)binding->model_kind != 12 || (sint8)binding->action_index != 0) && (character_action_target == 0 || (sint32)record->score < (sint32)character_action_target_score))
            {
                x = (uint32)player_interaction.position.vx - (instance->position[0]);
                target_delta[0] = x;
                instance = binding->instance;
                y = (uint32)player_interaction.position.vy - (instance->position[1]);
                target_delta[1] = y;
                instance = binding->instance;
                z = (uint32)player_interaction.position.vz - (instance->position[2]);
                target_delta[2] = z;
                value = Atan2Angle((sint32)z, (sint32)x);
                value = (uint32)(sint32)(sint16)value - (uint32)(sint32)(character_action_owner->yaw);
                if ((sint32)value < 0)
                    value = 0 - value;
                if ((sint16)value < 512)
                {
                    instance = binding->instance;
                    value = record->score;
                    character_action_target = instance;
                    character_action_target_score = value;
                }
            }
        }
        cursor = head;
        previous = cursor;
        record->binding = binding;
        if (cursor == 0)
        {
            head = record;
            record->next = 0;
        }
        else
        {
            score = record->score;
            if ((sint32)cursor->score < (sint32)score)
            {
                do
                {
                    previous = cursor;
                    cursor = cursor->next;
                } while (cursor != 0 && (sint32)cursor->score < (sint32)score);
            }
            if (cursor == head)
                head = record;
            else
                previous->next = record;
            record->next = cursor;
        }
        ++record;
        if (record == limit)
            break;
    }
    rank = 0xFFFFFFFF;
    slots = 12;
    budget = 102400;
    for (cursor = head; cursor != 0; cursor = cursor->next)
    {
        binding = cursor->binding;
        if (slots != 0)
        {
            cost_model = binding->model;
            if (cost_model != 0)
            {
                cost = cost_model->instance_cost;
                if ((sint32)budget >= (sint32)cost)
                {
                    budget -= cost;
                    --rank;
                    binding->distance_squared = rank;
                    --slots;
                }
            }
        }
        (((ActorPlacement *)(binding->definition)))->UpdateEvents();
    }
    if (head != 0)
    {
        flags = character_action_flags;
        result = flags ^ 0x200;
        if ((flags & 0x200) != 0)
        {
            result |= 0x400;
            character_action_flags = result;
        }
    }
}

uint32 AnimatedObject::UpdateSceneScriptVariable()
{
    FUNCTION_MARKER(0x80060C0C, "SLUS_005.78");
    uint32 *commands[1] = {&this->event_word};
    uint32 value, flags, index, byte, condition;
    sint32 mode, threshold, step;
    if (this->event_word != 0)
    {
        if ((ApplySceneEvents(commands, 1) << 24) != 0)
        {
            if (this->behavior_word != 0)
            {
                index = (uint32)(sint32)this->ticks;
                if (index == 98)
                    value = game_flow.progress.unlocked_stage;
                else if (index == 99)
                    value = scene_events.world_name_index;
                else
                    value = scene_events.script_values[index];
                value += this->state;
                if ((sint32)value < 0)
                    value = 0;
                else if ((sint32)value >= 100)
                    value = 99;
            }
            else
                value = this->state;
            index = (uint32)(sint32)this->ticks;
            if (index == 98)
                game_flow.progress.unlocked_stage = value;
            else if (index == 99)
                scene_events.world_name_index = value;
            else
                scene_events.script_values[index] = value;
        }
    }
    else
    {
        mode = (sint32)this->behavior_word;
        index = (uint32)(sint32)this->ticks;
        if (index == 98)
            value = game_flow.progress.unlocked_stage;
        else if (index == 99)
            value = scene_events.world_name_index;
        else
            value = scene_events.script_values[index];
        threshold = (sint32)this->state;
        if (mode == 0)
            condition = value == (uint32)threshold;
        else if (mode < 0)
            condition = (sint32)value < threshold;
        else
            condition = threshold < (sint32)value;
        if (condition != 0)
        {
            if ((this->node.flags & 1) != 0)
            {
                flags = this->flags;
                byte = this->node.flags;
                this->flags = flags | 0x80000;
                this->node.flags = byte & 0xFE;
            }
        }
        else if ((this->node.flags & 1) == 0)
        {
            flags = this->flags;
            byte = this->node.flags;
            this->flags = flags | 0x4000;
            this->node.flags = byte | 1;
        }
    }
    flags = this->flags;
    value = flags & 0x840000;
    if (value == 0)
        return value;
    if ((this->node.flags & 1) != 0)
        return 0x200000;
    step = (flags & 0x200000) != 0 ? -1 : 1;
    this->path_frame = step;
    return this->AdvanceTracks(step);
}

uint32 AnimatedObject::UpdateAnimatedObjectCommands()
{
    SceneNode *node;
    FUNCTION_MARKER(0x800607BC, "SLUS_005.78");
    const ObjectSpeedDefinition *definition;
    AnimatedObject *child;
    uint32 *commands[3] = {&this->event_word, &this->behavior_word, &this->state};
    uint32 flags, value, index, command, selected, result;
    sint32 step;
    flags = this->flags;
    value = this->path_flags;
    this->flags = flags & 0xFC7FFFFF;
    if ((value & 0x2000) != 0)
        this->WrapObjectSpeed();
    flags = this->flags;
    if ((flags & 0x100) != 0)
    {
        this->flags = flags & 0xFFFFFEFF;
        for (index = 0; index < 3; ++index)
        {
            value = *commands[index];
            if ((value & 0xFFF) == 0)
                continue;
            command = (value >> 12) & 31;
            switch (command)
            {
                case 1:
                    flags = this->flags;
                    if ((flags & 0x240000) != 0x40000)
                    {
                        if ((flags & 0x440000) != 0)
                            this->flags = flags & 0xFFBFFFFF;
                        else
                            this->flags = flags | 0x1020000;
                        this->flags = (this->flags & 0xFFDFFFFF) | 0x40000;
                    }
                    if ((this->path_flags & 0x2000) == 0)
                        break;
                    definition = FindObjectSpeedDefinition(this->state);
                    selected = 0;
                    node = this->node.links.first_child;
                    while (node != 0)
                    {
                        const SceneNode *child_node = node;
                        if (child_node->kind == 0)
                        {
                            child = (AnimatedObject *)node;
                            if ((child->path_flags & 0x2000) != 0)
                            {
                                value = child->velocity[0];
                                if (value != 0)
                                {
                                    selected = value;
                                    break;
                                }
                                value = child->ResolveObjectSpeed();
                                if (value != child->velocity[0])
                                {
                                    selected = value;
                                    break;
                                }
                            }
                        }
                        node = child_node->links.sibling_next;
                    }
                    if (selected == 0)
                        selected = definition->default_speed;
                    this->velocity[0] = selected;
                    break;
                case 2:
                    if ((this->path_flags & 0x2000) != 0)
                        this->ApplyObjectSpeedAction(41);
                    else
                    {
                        flags = this->flags;
                        if ((flags & 0x240000) != 0x240000)
                        {
                            value = (flags & 0x440000) != 0 ? flags & 0xFFBFFFFF : flags | 0x2010000;
                            this->flags = value;
                            this->flags = this->flags | 0x240000;
                        }
                    }
                    break;
                case 3:
                case 14:
                    goto hide_object;
                case 4:
                case 15:
                    goto show_object;
                case 5:
                    flags = this->flags;
                    if ((flags & 0x40000) != 0)
                        this->flags = (flags & 0xFFFBFFFF) | 0x400000;
                    break;
                case 6:
                    if ((this->path_flags & 0x2000) != 0)
                        this->ApplyObjectSpeedAction(45);
                    else
                        this->flags = (this->flags | 0x40000) & 0xFFBFFFFF;
                    break;
                case 7:
                    if ((this->path_flags & 0x2000) != 0)
                        this->ApplyObjectSpeedAction(46);
                    else
                        this->flags = (this->flags & 0xFFDFFFFF) | 0x800000;
                    break;
                case 8:
                    this->flags = (this->flags | 0x8000) & 0xFC1BFFFF;
                    goto process_mode;
                case 9:
                    if ((this->path_flags & 0x2000) != 0)
                        this->ApplyObjectSpeedAction(48);
                    else
                        this->flags = this->flags | 0xA00000;
                    break;
                case 10:
                process_mode:
                    if ((this->path_flags & 0x2000) == 0)
                        goto finish_commands;
                    this->ApplyObjectSpeedAction(49);
                    break;
                case 12:
                    if ((this->node.flags & 1) != 0)
                        goto show_object;
                hide_object:
                    flags = this->flags;
                    value = this->node.flags;
                    this->flags = flags | 0x4000;
                    this->node.flags = value | 1;
                    break;
                show_object:
                    flags = this->flags;
                    value = this->node.flags;
                    this->flags = flags | 0x80000;
                    this->node.flags = value & 0xFE;
                    break;
                case 13:
                    flags = this->flags;
                    if ((flags & 0x40000) != 0)
                        this->flags = (flags & 0xFFFBFFFF) | 0x400000;
                    else
                        this->flags = (flags | 0x40000) & 0xFFBFFFFF;
                    break;
            }
        }
        flags = this->flags;
    }
    this->path_frame = 0;
    if ((flags & 0x840000) != 0 && (this->node.flags & 1) == 0)
    {
        step = (flags & 0x200000) != 0 ? -1 : 1;
        this->path_frame = step;
        this->AdvanceTracks(step);
    }
finish_commands:
    result = ApplySceneEvents(commands, 3) << 24;
    if (result != 0)
    {
        result = this->flags | 0x100;
        this->flags = result;
    }
    return result;
}

void SceneTrigger::UpdateExtentsFromAnimation()
{
    FUNCTION_MARKER(0x8001BB58, "SLUS_005.78");
    const AnimatedObject *owner = (const AnimatedObject *)(animation_owner);
    uint32 result = owner->flags & 32;
    if (result == 0)
    {
        const AnimationHeader *animation = owner->animation;
        if (animation != 0)
        {
            sint32 index = animation_binding.tracks[2];
            if (index >= 0)
            {
                const AnimationHeader *header = animation;
                AnimationTrack *track = ((AnimationTrack *)(header->tracks[2].records)) + index;
                uint8 flags = track->flags;
                result = flags & 0xF7;
                if ((flags & 8) != 0)
                {
                    sint32 frame = track->frame;
                    uint32 output[3];
                    uint32 x;
                    uint32 y;
                    uint32 z;
                    track->flags = result;
                    SampleVectorTrack(frame, track, output);
                    extent[0] = (sint32)output[0] >> 5;
                    x = extent[0];
                    extent[1] = (sint32)output[1] >> 5;
                    z = (uint32)(sint32)output[2];
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
    return;
}

uint32 SceneTrigger::PlaySound()
{
    FUNCTION_MARKER(0x8005C40C, "SLUS_005.78");
    SceneSoundControl *control = &parameters.sound;
    uint32 index, packed, result;
    const SoundBankStorage *table;
    const SoundSample *sound = 0;
    if ((node.state_flags & 2) == 0 && animation_binding.tracks[2] >= 0)
        UpdateExtentsFromAnimation();
    table = sound_resources.scene_table;
    index = control->sample_id & 4095;
    if (table != 0 && index < table->header.count)
        sound = table->samples + index;
    /* An invalid sound identifier must not read the null guest descriptor */
    if (sound == 0)
        return 0;
    packed = control->playback.word;
    result = PlaySpatialSound(sound, (packed >> 16) & 0x3FFF, (packed >> 8) & 255, sound->pitch, (const uint32 *)transform.t, (uint32)radius << 5, packed & 255, 1, 1) & 255;
    control->playback.word = (control->playback.word & 0xFFFFFF00) | result;
    return result;
}

uint32 SceneTrigger::StopSound()
{
    FUNCTION_MARKER(0x8005C4F4, "SLUS_005.78");
    SceneSoundControl *control = &parameters.sound;
    uint32 voice, index, result;
    const SoundBankStorage *table;
    const SoundSample *sound = 0;
    table = sound_resources.scene_table;
    voice = control->playback.bytes[0];
    if (table != 0)
    {
        index = control->sample_id;
        result = table->header.count;
        index &= 0xFFF;
        if (index < result)
            sound = table->samples + index;
    }
    result = sound_effects.IsPlaying(sound, voice);
    if ((result << 24) != 0)
        StopSoundVoice((uint16)voice);
    result = control->playback.word | 0xFF;
    control->playback.word = result;
    return result;
}
