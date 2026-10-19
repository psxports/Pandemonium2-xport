#include <stdlib.h>

#include "game.h"

VolleyEnemyState volley_enemy = {0};

uint32 LoadVolleyScripts(const uint8 *bundle)
{
    FUNCTION_MARKER(0x80020AE8, "SLUS_005.78");
    uint32 offset = resource_archive.Offset(resource_load_cursor.asset);
    const uint8 *bytes = bundle + offset;
    volley_enemy.script_count = xport_load_le32(bytes);
    offset = resource_archive.Offset(resource_load_cursor.asset);
    bytes = bundle + offset + 4;
    uint32 index, frame;
    for (index = 0; index < 20; ++index)
    {
        VolleyScript *script = &volley_enemy.scripts[index];
        script->move_speed = xport_load_le32(bytes);
        script->distance_squared = (sint32)xport_load_le32(bytes + 4);
        script->mode = xport_load_le32(bytes + 8);
        script->projectile_kind = xport_load_le32(bytes + 12);
        script->frame_count = (sint32)xport_load_le32(bytes + 16);
        for (frame = 0; frame < 10; ++frame)
        {
            script->frames[frame] = (sint32)xport_load_le32(bytes + 20 + frame * 4);
            script->angles[frame] = (sint32)xport_load_le32(bytes + 60 + frame * 4);
        }
        script->action_limit = xport_load_le32(bytes + 100);
        script->shot_delay = xport_load_le32(bytes + 104);
        script->turn_ticks = (sint32)xport_load_le32(bytes + 108);
        script->health_threshold = (sint32)xport_load_le32(bytes + 112);
        bytes += 116;
    }
    uint32 result = resource_load_cursor.asset + 1;
    resource_load_cursor.asset = result;
    return result;
}

static uint32 ResetVolleyScript(void)
{
    FUNCTION_MARKER(0x8001FA08, "SLUS_005.78");
    volley_enemy.motion_ticks = 0;
    volley_enemy.approach_ticks = 0;
    volley_enemy.frame_index = 0;
    volley_enemy.reset_pending = 1;
    return 1;
}

uint32 AnimatedObject::SpawnVolleyProjectile(const VolleyScript *script, const uint32 *target, sint32 index)
{
    FUNCTION_MARKER(0x8001FA24, "SLUS_005.78");

    union
    {
        VECTOR vector;
        uint32 words[4];
    } input;

    AnimatedObject *spawned;
    uint32 type, magnitude, length;
    const PathDefinition *definition;
    const PathTransform *path_transform;
    sint32 delta, degrees, angle, factor;
    uint32 axis;
    if (scene_events.script_values[15] >= 5)
        return 0;
    this->state = script->projectile_kind;
    this->flags = this->flags | 0x100;
    spawned = object_operations.behaviors[6].factory_handler(this);
    this->flags = this->flags & 0xFFFFFEFF;
    /* A failed factory must not write a projectile through address zero */
    if (spawned == 0)
        return 0;
    spawned->animation_binding.tracks[0] = -3;
    definition = (spawned->path_definition);
    if (definition != 0 && (((const PathDefinition *)((const PathDefinition *)(definition)))->transform) != 0)
    {
        input.words[0] = spawned->position[0];
        input.words[1] = spawned->position[1];
        input.words[2] = spawned->position[2];
        PushMatrix();
        path_transform = (const PathTransform *)((((const PathDefinition *)((const PathDefinition *)((spawned->path_definition))))->transform) != 0 ? (((const PathDefinition *)((const PathDefinition *)((spawned->path_definition))))->transform) : (PathTransform *)0);
        xport_gte_write_control(0, xport_load_le32((const uint8 *)&path_transform->matrix));
        xport_gte_write_control(1, xport_load_le32((const uint8 *)&path_transform->matrix + 4));
        xport_gte_write_control(2, xport_load_le32((const uint8 *)&path_transform->matrix + 8));
        xport_gte_write_control(3, xport_load_le32((const uint8 *)&path_transform->matrix + 12));
        xport_gte_write_control(4, xport_load_le32((const uint8 *)&path_transform->matrix + 16));
        ApplyRotMatrixLV(&input.vector, (VECTOR *)spawned->position);
        PopMatrix();
        for (axis = 0; axis < 3; ++axis)
        {
            path_transform = (const PathTransform *)((((const PathDefinition *)((const PathDefinition *)((spawned->path_definition))))->transform) != 0 ? (((const PathDefinition *)((const PathDefinition *)((spawned->path_definition))))->transform) : (PathTransform *)0);
            spawned->position[axis] = spawned->position[axis] + (uint32)path_transform->matrix.t[axis];
        }
    }
    else
    {
        uint32 x = spawned->position[0], y = spawned->position[1], z = spawned->position[2];
        spawned->position[0] = x;
        spawned->position[1] = y;
        spawned->position[2] = z;
    }
    type = script->mode;
    if (type == 0 || type == 1 || type == 3)
    {
        uint32 y = spawned->position[1], x = spawned->position[0];
        spawned->position[1] = y + 172032;
        spawned->position[0] = x + 61440;
    }
    spawned->movement_mode = 0;
    spawned->flags = spawned->flags | 0x2000;
    type = script->mode;
    if (type < 2)
    {
        spawned->velocity[0] = 0;
        spawned->velocity[1] = 24576;
        spawned->velocity[2] = target[2] - spawned->position[2];
    }
    else if (type == 3)
    {
        delta = (sint32)(target[2] - spawned->position[2]);
        degrees = script->angles[index];
        angle = (sint32)((degrees < 0 ? 0 - (uint32)degrees : (uint32)degrees) << 12) / 360;
        factor = SinQ14((sint16)angle) >> 2;
        magnitude = (uint32)(((sint64)factor * delta) >> 12);
        spawned->velocity[0] = degrees < 0 ? 0 - magnitude : magnitude;
        spawned->velocity[1] = 24576;
        factor = SinQ14((sint16)(1024 - angle)) >> 2;
        spawned->velocity[2] = (uint32)(((sint64)factor * delta) >> 12);
    }
    magnitude = 0;
    for (axis = 0; axis < 3; ++axis)
    {
        sint32 component = (sint32)spawned->velocity[axis];
        magnitude += (uint32)(((sint64)component * component) >> 12);
    }
    length = (sint32)magnitude >= 0 ? SquareRoot12((sint32)magnitude) : SquareRoot12((sint32)(magnitude >> 2)) << 1;
    length = (uint32)((sint32)length >> 1);
    if (length == 0)
    {
        fprintf(stderr, "P2: Zero projectile normalization length\n");
        abort();
    }
    factor = 0x40000000 / (sint32)length;
    for (axis = 0; axis < 3; ++axis)
    {
        sint32 component = (sint32)spawned->velocity[axis];
        sint32 normalized = (sint32)(((sint64)component * factor) >> 18);
        fixed_math_result = normalized;
        delta = (sint32)(((sint64)normalized * 122880) >> 12);
        spawned->position[axis] = spawned->position[axis] + (uint32)delta;
    }
    magnitude = script->shot_delay;
    this->behavior_word = magnitude;
    return magnitude;
}

uint32 AnimatedObject::UpdateVolleyPath(const uint32 *origin, const uint32 *target, sint32 distance, ActorModelDefinition *resource_model, const VolleyScript *script)
{
    FUNCTION_MARKER(0x80020094, "SLUS_005.78");
    AnimationHeader *animation, *attack, *next_animation;
    uint32 flags;
    sint32 frame, current, next;
    if (volley_enemy.reset_pending != 0)
    {
        this->flags = this->flags & 0xFFFFFFBF;
        {
            AnimationHeader *animation_value = (resource_model->animations[0]);
            uint32 animation_flags = this->flags;
            this->animation = animation_value;
            this->frame = -1;
            this->flags = animation_flags | 0x10000;
        }
        volley_enemy.reset_pending = 0;
        volley_enemy.path_active = 0;
        volley_enemy.event_started = 0;
        return 0;
    }
    animation = this->animation;
    if (animation == (resource_model->animations[0]))
    {
        frame = this->frame;
        if (animation != 0 && frame < ((AnimationHeader *)(animation))->frame_count - 1 && frame + 1 >= 40 && volley_enemy.event_started == 0)
        {
            volley_enemy.event_started = 1;
            scene_events.pending = volley_enemy.event_id | 0x1000;
        }
        if (this->frame >= ((AnimationHeader *)(this->animation))->frame_count - 1)
        {
            next = script->frames[volley_enemy.frame_index];
            if ((this->flags & 0x10) != 0)
                next = (sint32)((uint32)(sint32)(sint16)((const PathDefinition *)((this->path_definition)))->positions.metadata6 - (uint32)next);
            volley_enemy.target_path_frame = next;
            volley_enemy.path_active = 1;
            volley_enemy.prev_path_frame = (uint32)this->path_frame;
        }
    }
    if (volley_enemy.path_active != 0)
    {
        animation = this->animation;
        attack = (resource_model->animations[1]);
        if (animation == attack)
        {
            if (this->frame >= ((AnimationHeader *)(attack))->frame_count - 1)
            {
                volley_enemy.frame_index = volley_enemy.frame_index + 1;
                next_animation = (resource_model->animations[((this->flags & 0x10) != 0 ? 4 : 3)]);
                {
                    AnimationHeader *animation_value = next_animation;
                    uint32 animation_flags = this->flags;
                    this->animation = animation_value;
                    this->frame = -1;
                    this->flags = animation_flags | 0x10000;
                }
            }
            next = script->frames[volley_enemy.frame_index];
            if ((this->flags & 0x10) != 0)
                next = (sint32)((uint32)(sint32)(sint16)((const PathDefinition *)((this->path_definition)))->positions.metadata6 - (uint32)next);
            volley_enemy.target_path_frame = next;
        }
        else
        {
            if (distance < script->distance_squared || volley_enemy.approach_ticks == script->action_limit)
            {
                this->frame = -1;
                this->animation = attack;
                current = this->path_frame;
                this->flags = (this->flags & 0xFFFEFFBF) | 0x10000;
                volley_enemy.approach_ticks = 0;
                volley_enemy.prev_path_frame = current;
                return 0;
            }
            volley_enemy.approach_ticks = volley_enemy.approach_ticks + 1;
            flags = this->flags;
            current = this->path_frame;
            if (((flags & 0x10) != 0 && (sint32)target[0] < (sint32)origin[0]) || ((flags & 0x10) == 0 && (sint32)origin[0] < (sint32)target[0]))
                next = current + 1;
            else
                next = current - 1;
            volley_enemy.target_path_frame = next;
            current = this->path_frame;
            if (next < current)
            {
                if (current > 0)
                {
                    this->flags = ((this->flags & 0xFFFFF7FF) ^ 0x10) | 0x40;
                    this->path_frame = ((const PathDefinition *)((this->path_definition)))->positions.metadata6 - current;
                }
                else
                    this->flags = this->flags & 0xFFFFFFBF;
            }
            else if (current < (sint16)((const PathDefinition *)((this->path_definition)))->positions.metadata6)
                this->flags = this->flags | 0x40;
            else
                this->flags = this->flags & 0xFFFFFFBF;
            next_animation = (resource_model->animations[((this->flags & 0x10) != 0 ? 4 : 3)]);
            if (this->animation != next_animation)
            {
                AnimationHeader *animation_value = next_animation;
                uint32 animation_flags = this->flags;
                this->animation = animation_value;
                this->frame = -1;
                this->flags = animation_flags | 0x10000;
            }
        }
        volley_enemy.prev_path_frame = (uint32)this->path_frame;
    }
    if ((sint32)volley_enemy.frame_index < script->frame_count)
        return 0;
    volley_enemy.frame_index = 0;
    volley_enemy.path_active = 0;
    volley_enemy.event_started = 0;
    volley_enemy.reset_pending = 1;
    scene_events.pending = volley_enemy.event_id | 0x2000;
    return 1;
}

uint32 AnimatedObject::UpdateVolleyFiring(const uint32 *origin, const uint32 *target, sint32 distance, ActorModelDefinition *resource_model, const VolleyScript *script)
{
    FUNCTION_MARKER(0x800204CC, "SLUS_005.78");
    AnimationHeader *animation, *next_animation;
    uint32 kind, flags;
    sint32 frame, next_frame, count, index;
    if (this->behavior_word != 0 || volley_enemy.delay_ticks != 0)
        return 1;
    resource_model->motion_speed[0] = (sint32)(script->move_speed << 12) / 30;
    kind = script->mode;
    animation = this->animation;
    if (kind == 1 && (animation == (resource_model->animations[5]) || animation == (resource_model->animations[6])))
    {
        next_frame = animation != 0 && this->frame < ((AnimationHeader *)(animation))->frame_count - 1 ? this->frame + 1 : 0;
        count = script->frame_count;
        for (index = 0; index < count; ++index)
        {
            if (script->frames[index] == next_frame)
            {
                this->SpawnVolleyProjectile(script, target, index);
                return 1;
            }
        }
    }
    else if (kind == 0 || kind == 3)
    {
        if (animation == (resource_model->animations[2]) || script->distance_squared < distance || volley_enemy.motion_ticks == script->action_limit)
        {
            if (animation != (resource_model->animations[2]))
            {
                this->flags = this->flags & 0xFFFFFFBF;
                {
                    AnimationHeader *animation_value = (resource_model->animations[2]);
                    uint32 animation_flags = this->flags;
                    this->animation = animation_value;
                    this->frame = -1;
                    this->flags = animation_flags | 0x10000;
                }
            }
            animation = this->animation;
            next_frame = animation != 0 && this->frame < ((AnimationHeader *)(animation))->frame_count - 1 ? this->frame + 1 : 0;
            count = script->frame_count;
            for (index = 0; index < count; ++index)
            {
                if (script->frames[index] == next_frame)
                {
                    this->SpawnVolleyProjectile(script, target, index);
                    volley_enemy.turn_pending = 1;
                    volley_enemy.motion_ticks = 0;
                    return 1;
                }
            }
            animation = this->animation;
            frame = this->frame;
            if ((animation == 0 || frame >= ((AnimationHeader *)(animation))->frame_count - 1) && frame > 0)
            {
                {
                    AnimationHeader *animation_value = (resource_model->animations[0]);
                    uint32 animation_flags = this->flags;
                    this->animation = animation_value;
                    this->frame = -1;
                    this->flags = animation_flags | 0x10000;
                }
                if ((uint16)this->path_frame == 0 && ((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->positions.keys != 0)
                    this->path_frame = 1;
            }
            return 1;
        }
    }
    if (this->behavior_word != 0)
        return 1;
    kind = script->mode;
    if (kind == 1)
    {
        this->flags = this->flags | 0x40;
        count = (sint32)(volley_enemy.motion_ticks + 1);
        volley_enemy.motion_ticks = count;
        if (count > script->turn_ticks)
        {
            uint32 random = (uint32)randPSX() & 1;
            volley_enemy.turn_pending = 0;
            flags = this->flags;
            if ((random != 0 && (flags & 0x10) != 0) || (random == 0 && (flags & 0x10) == 0))
            {
                uint32 path_frame = (uint16)this->path_frame;
                uint32 path_count = ((const PathDefinition *)((this->path_definition)))->positions.metadata6;
                this->flags = (this->flags & 0xFFFFF7FF) ^ 0x10;
                this->path_frame = path_count - path_frame;
            }
            volley_enemy.motion_ticks = 0;
        }
        frame = this->path_frame;
        count = (sint16)((const PathDefinition *)((this->path_definition)))->positions.metadata6;
        if (frame >= count - 1)
        {
            this->path_frame = count - 1;
            {
                uint32 path_frame = (uint16)this->path_frame;
                uint32 path_count = ((const PathDefinition *)((this->path_definition)))->positions.metadata6;
                this->flags = (this->flags & 0xFFFFF7FF) ^ 0x10;
                this->path_frame = path_count - path_frame;
            }
        }
        next_animation = (resource_model->animations[((this->flags & 0x10) != 0 ? 6 : 5)]);
        if (this->animation != next_animation)
        {
            AnimationHeader *animation_value = next_animation;
            uint32 animation_flags = this->flags;
            this->animation = animation_value;
            this->frame = -1;
            this->flags = animation_flags | 0x10000;
        }
    }
    else if ((kind == 0 || kind == 3) && script->distance_squared >= distance)
    {
        count = (sint32)(volley_enemy.motion_ticks + 1);
        volley_enemy.motion_ticks = count;
        if (count >= (sint32)script->action_limit)
            return 1;
        this->flags = this->flags | 0x40;
        if (volley_enemy.turn_pending != 0)
        {
            uint32 random = (uint32)randPSX() & 1;
            volley_enemy.turn_pending = 0;
            flags = this->flags;
            if ((random != 0 && (flags & 0x10) != 0) || (random == 0 && (flags & 0x10) == 0))
            {
                uint32 path_frame = (uint16)this->path_frame;
                uint32 path_count = ((const PathDefinition *)((this->path_definition)))->positions.metadata6;
                this->flags = (this->flags & 0xFFFFF7FF) ^ 0x10;
                this->path_frame = path_count - path_frame;
            }
        }
        count = (sint16)((const PathDefinition *)((this->path_definition)))->positions.metadata6;
        if (this->path_frame >= count - 1)
        {
            this->path_frame = count - 1;
            {
                uint32 path_frame = (uint16)this->path_frame;
                uint32 path_count = ((const PathDefinition *)((this->path_definition)))->positions.metadata6;
                this->flags = (this->flags & 0xFFFFF7FF) ^ 0x10;
                this->path_frame = path_count - path_frame;
            }
        }
        if (this->path_frame == 0)
        {
            uint32 path_frame = (uint16)this->path_frame;
            uint32 path_count = ((const PathDefinition *)((this->path_definition)))->positions.metadata6;
            this->flags = (this->flags & 0xFFFFF7FF) ^ 0x10;
            this->path_frame = path_count - path_frame;
        }
        next_animation = (resource_model->animations[((this->flags & 0x10) != 0 ? 4 : 3)]);
        if (this->animation != next_animation)
        {
            AnimationHeader *animation_value = next_animation;
            uint32 animation_flags = this->flags;
            this->animation = animation_value;
            this->frame = -1;
            this->flags = animation_flags | 0x10000;
        }
    }
    return 1;
}

uint32 AnimatedObject::UpdateVolleyEnemy()
{
    typedef uint32 (AnimatedObject::*VolleyHandler)(const uint32 *, const uint32 *, sint32, ActorModelDefinition *, const VolleyScript *);
    static const VolleyHandler volley_handlers[4] = {&AnimatedObject::UpdateVolleyFiring, &AnimatedObject::UpdateVolleyFiring, &AnimatedObject::UpdateVolleyPath, &AnimatedObject::UpdateVolleyFiring};
    FUNCTION_MARKER(0x8001F6DC, "SLUS_005.78");
    ActorModelDefinition *bound_model;
    bound_model = ((const ActorBinding *)(((const ActorPlacement *)((const PathDefinition *)((this->path_definition))))->state != 0 ? ((const ActorPlacement *)((const PathDefinition *)((this->path_definition))))->state : 0))->model;
    uint32 distance, result;
    AnimationHeader *animation;
    uint32 origin[3], target[3], distance_output;
    const VolleyScript *script;
    sint32 delta, kind, step;
    this->UpdateActorStateAndTarget(origin, target, &distance_output);
    delta = (sint32)(origin[0] - target[0]) >> 12;
    distance = (uint32)delta * (uint32)delta;
    distance_output = distance;
    if ((((const PathDefinition *)((this->path_definition)))->flags & 0x40) != 0 && volley_enemy.skip_update == 0)
    {
        if (this->behavior_word != 0)
            this->behavior_word = this->behavior_word - 1;
        if (volley_enemy.delay_ticks != 0)
            volley_enemy.delay_ticks = volley_enemy.delay_ticks - 1;
        script = &volley_enemy.scripts[volley_enemy.script_index];
        result = (this->*volley_handlers[script->mode])(origin, target, (sint32)distance, bound_model, script);
        if (result == 1)
        {
            step = (sint32)volley_enemy.script_index;
            while (this->ticks < volley_enemy.scripts[step].health_threshold)
            {
                if (step >= (sint32)volley_enemy.script_count - 1)
                    break;
                volley_enemy.script_index = (uint32)step + 1;
                ResetVolleyScript();
                step = (sint32)volley_enemy.script_index;
                kind = (sint32)volley_enemy.scripts[step].mode;
                volley_enemy.delay_ticks = 30;
                if (kind == 2)
                    break;
            }
        }
    }
    else
    {
        this->flags = this->flags & 0xFFFFFFBF;
        if (this->animation != (bound_model->animations[3]))
        {
            AnimationHeader *animation_value = (bound_model->animations[3]);
            uint32 animation_flags = this->flags;
            this->animation = animation_value;
            this->frame = -1;
            this->flags = animation_flags | 0x10000;
        }
        volley_enemy.skip_update = 0;
    }
    script = &volley_enemy.scripts[volley_enemy.script_index];
    kind = (sint32)script->mode;
    if (kind >= 0 && kind < 2)
        target[0] = origin[0];
    else if (kind == 2 && this->animation == (bound_model->animations[1]))
    {
        sint32 marker = script->frames[volley_enemy.frame_index];
        if (this->frame == marker)
            volley_enemy.target_x = target[0];
        else if (marker < this->frame)
            target[0] = volley_enemy.target_x;
    }
    animation = this->animation;
    if (animation == (bound_model->animations[0]))
        target[0] = origin[0];
    result = this->UpdateActorFacingAndAnimation(target, origin, bound_model);
    return result;
}

uint32 AnimatedObject::InitializeVolleyEnemy()
{
    FUNCTION_MARKER(0x80020C08, "SLUS_005.78");
    {
        ObjectCallback operation = {0, &AnimatedObject::UpdateVolleyEnemy};
        object_operations.behaviors[30] = operation;
    }
    this->behavior = 30;
    volley_enemy.event_id = EncodeObjectName(74, 80);
    volley_enemy.delay_ticks = 30;
    volley_enemy.script_index = 0;
    volley_enemy.motion_ticks = 0;
    volley_enemy.approach_ticks = 0;
    volley_enemy.frame_index = 0;
    volley_enemy.reset_pending = 1;
    volley_enemy.skip_update = 1;
    return 1;
}
