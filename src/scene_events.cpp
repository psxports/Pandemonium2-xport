#include "scene_events.h"
#include "game.h"

SceneEvents scene_events = {0};

// Retain the original switch values carried by unhandled event actions
static const uint32 animated_event_dispatch_vals[15] = {0x80042138, 0x800422E8, 0x80042154, 0x8004229C, 0x8004216C, 0x800422E8, 0x8004219C, 0x800422E8, 0x80042210, 0x800422E8, 0x800422E8, 0x80042288, 0x800422E8, 0x80042154, 0x8004229C};

struct SceneEventCandidate
{
    SceneEventCandidate *next;
    ActorBinding *binding;
    uint32 score;
};

void SceneEvents::QueueSceneEvent(uint32 event, uint32 kind)
{
    FUNCTION_MARKER(0x8005BB48, "SLUS_005.78");
    sint32 count = scene_events.count;
    uint32 result = count < 40;
    if (result != 0)
    {
        scene_events.count = count + 1;
        result = event | (kind << 17);
        scene_events.entries[count] = result;
    }
}

uint32 SceneEvents::EncodeObjectName(sint32 char0, sint32 char1)
{
    FUNCTION_MARKER(0x8005BB90, "SLUS_005.78");
    uint32 high, low;
    if (char0 == 95 || char1 == 95)
        return 0;
    if ((uint32)char0 - 48 < 10)
        high = (uint32)char0 - 48;
    else if ((uint32)char0 - 97 < 26)
        high = (uint32)char0 - 87;
    else if ((uint32)char0 - 65 < 26)
        high = (uint32)char0 - 29;
    else
        return 0xFFFFFFFF;
    if ((uint32)char1 - 48 < 10)
        low = (uint32)char1 - 48;
    else if ((uint32)char1 - 97 < 26)
        low = (uint32)char1 - 87;
    else if ((uint32)char1 - 65 < 26)
        low = (uint32)char1 - 29;
    else
        return 0xFFFFFFFF;
    return high * 62 + low + 1;
}

uint32 SceneEvents::ApplySceneEvents(uint32 *const *destinations, sint32 count)
{
    uint32 result = 0;
    uint32 idx = 0;
    uint32 val;
    uint32 key;
    const uint32 *cursor;
    uint32 remaining;
    FUNCTION_MARKER(0x8005BC58, "SLUS_005.78");
    if (scene_events.count == 0)
        return 0;
    if (count > 0)
    {
        do
        {
            uint32 *dst = destinations[idx];
            key = *dst & 0xFFF;
            if (key != 0)
            {
                *dst = key;
                val = scene_events.entries[0];
                remaining = scene_events.count;
                cursor = scene_events.entries + 1;
                for (;;)
                {
                    --remaining;
                    if (key == (val & 0xFFF))
                    {
                        *dst = val;
                        result = 1;
                        break;
                    }
                    if (remaining == 0)
                        break;
                    val = *cursor;
                    ++cursor;
                }
            }
            ++idx;
        } while ((sint32)idx < count);
    }
    return result;
}

void SceneEvents::UpdateSceneEventObjects(void)
{
    const SceneSection *section;
    FUNCTION_MARKER(0x8005BFF4, "SLUS_005.78");
    const ActorModelDefinition *cost_model;
    SceneEventCandidate candidates[50];
    SceneEventCandidate *rec = candidates;
    SceneEventCandidate *limit = candidates + 50;
    SceneEventCandidate *head = 0, *cursor, *prev;
    Vec3 target_delta;
    ActorBinding *binding;
    AnimatedObject *instance;
    ActorPlacement *placement;
    const uint32 *event_src;
    uint32 count;
    uint32 val;
    sint32 score;
    uint32 x, z, flags, result, slots, budget, cost, rank;
    if (scene_events.deferred_count != 0)
    {
        event_src = scene_events.deferred_entries;
        do
        {
            count = scene_events.count;
            val = *event_src;
            ++event_src;
            if ((sint32)count < 40)
            {
                scene_events.count = count + 1;
                scene_events.entries[count] = val | 0x20000;
            }
            count = scene_events.deferred_count - 1;
            scene_events.deferred_count = count;
        } while ((count & 0xFF) != 0);
    }
    section = scene_stream.active_section;
    for (binding = section->actor_bindings; binding != 0; binding = binding->next)
    {
        placement = binding->definition;
        if ((placement->path.node.flags & 0x80) != 0)
        {
            result = binding->respawn_delay;
            if (result == 0)
                continue;
            val = binding->respawn_ticks - 1;
            binding->respawn_ticks = val;
            result = val & 0xFFFF;
            if (result != 0)
                continue;
            val = (uint16)binding->initial_path_frame;
            placement = binding->definition;
            binding->path_frame = val;
            placement->path.flags &= 0xFFFFFFFB;
            placement = binding->definition;
            placement->path.node.flags &= 0x7F;
        }
        placement = binding->definition;
        if ((placement->path.node.flags & 1) != 0)
        {
            placement->UpdateEvents();
            continue;
        }
        val = binding->distance_sq;
        if (val == 0x7FFFF000)
            val = binding->ComputeInteractionDistance();
        rec->score = val;
        if ((character_action_flags & 0x200) != 0)
        {
            instance = binding->instance;
            if (instance != 0 && (binding->model_kind != 12 || binding->action_idx != 0) && (character_action_target == 0 || (sint32)rec->score < (sint32)character_action_target_score))
            {
                x = player_interaction.pos.vx - (uint32)instance->pos.x;
                target_delta.x = x;
                instance = binding->instance;
                target_delta.y = player_interaction.pos.vy - (uint32)instance->pos.y;
                instance = binding->instance;
                z = player_interaction.pos.vz - (uint32)instance->pos.z;
                target_delta.z = z;
                val = Atan2Angle(z, x);
                val = (uint32)(sint16)val - character_action_owner->yaw;
                if ((sint32)val < 0)
                    val = -val;
                if ((sint16)val < 512)
                {
                    instance = binding->instance;
                    val = rec->score;
                    character_action_target = instance;
                    character_action_target_score = val;
                }
            }
        }
        cursor = head;
        prev = cursor;
        rec->binding = binding;
        if (cursor == 0)
        {
            head = rec;
            rec->next = 0;
        }
        else
        {
            score = rec->score;
            if ((sint32)cursor->score < score)
            {
                do
                {
                    prev = cursor;
                    cursor = cursor->next;
                } while (cursor != 0 && (sint32)cursor->score < score);
            }
            if (cursor == head)
                head = rec;
            else
                prev->next = rec;
            rec->next = cursor;
        }
        ++rec;
        if (rec == limit)
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
                    binding->distance_sq = rank;
                    --slots;
                }
            }
        }
        binding->definition->UpdateEvents();
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
