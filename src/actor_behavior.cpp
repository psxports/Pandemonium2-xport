#include <stdio.h>
#include <stdlib.h>

#include "game.h"

uint32 AnimatedObject::UpdateBasicActor()
{
    FUNCTION_MARKER(0x800229A4, "SLUS_005.78");
    uint32 origin[3], target[3], distance, result;
    this->UpdateActorStateAndTarget(origin, target, &distance);
    ActorBinding *binding = (ActorBinding *)(((const ActorPlacement *)((const PathDefinition *)((this->path_definition))))->state != 0 ? ((const ActorPlacement *)((const PathDefinition *)((this->path_definition))))->state : 0);
    result = this->UpdateActorFacingAndAnimation(0, origin, binding->model);
    return result;
}

uint32 AnimatedObject::UpdateActorPatrolPause()
{
    FUNCTION_MARKER(0x800278E8, "SLUS_005.78");
    ActorModelDefinition *resource_model;
    const PathDefinition *definition = (this->path_definition);
    ActorBinding *state = (ActorBinding *)(((const ActorPlacement *)((const PathDefinition *)(definition)))->state != 0 ? ((const ActorPlacement *)((const PathDefinition *)(definition)))->state : 0);
    ActorBinding *binding = state;
    resource_model = binding->model;
    uint32 result;
    uint32 origin[3], target[3], distance;
    this->UpdateActorStateAndTarget(origin, target, &distance);
    if ((binding->flags & 10) == 0)
    {
        if ((((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->flags & 0x40) != 0)
        {
            uint32 timer = this->behavior_word;
            if (timer != 0)
                this->behavior_word = --timer;
            if (timer == 0)
            {
                AnimationHeader *animation;
                uint32 delay;
                if ((this->flags & 0x40) != 0)
                {
                    animation = (resource_model->animations[0]);
                    delay = 60;
                }
                else
                {
                    ((ActorBinding *)(((const ActorPlacement *)((const PathDefinition *)((this->path_definition))))->state != 0 ? ((const ActorPlacement *)((const PathDefinition *)((this->path_definition))))->state : 0))->motion_speed = 0;
                    animation = (resource_model->animations[2]);
                    delay = 0;
                }
                this->animation = animation;
                this->frame = (sint16)(-1);
                this->flags = this->flags | 0x10000;
                if (delay == 0)
                    delay = random_generator.Next() % 60 + 90;
                this->behavior_word = delay;
                this->flags = this->flags ^ 0x40;
            }
        }
        else if (this->animation != (resource_model->animations[0]))
        {
            this->behavior_word = 0;
            this->flags = this->flags & 0xFFFFFFBF;
            this->animation = (resource_model->animations[0]);
            this->frame = (sint16)(-1);
            this->flags = this->flags | 0x10000;
        }
    }
    result = this->UpdateActorFacingAndAnimation(0, origin, resource_model);
    return result;
}

uint32 AnimatedObject::UpdateActorTargetPursuit()
{
    FUNCTION_MARKER(0x8003FDC8, "SLUS_005.78");
    ActorBinding *state;
    ActorModelDefinition *resource_model;
    uint32 origin[3], target[3], distance, result;
    const PathDefinition *definition;
    this->UpdateActorStateAndTarget(origin, target, &distance);
    definition = (this->path_definition);
    state = (ActorBinding *)(((const ActorPlacement *)((const PathDefinition *)(definition)))->state != 0 ? ((const ActorPlacement *)((const PathDefinition *)(definition)))->state : 0);
    ActorBinding *binding = state;
    resource_model = binding->model;
    if ((binding->flags & 2) == 0)
    {
        if ((resource_model->target_range[0]) < (sint32)distance || (((const PathDefinition *)((const PathDefinition *)(definition)))->flags & 0x40) == 0)
        {
            if ((this->flags & 0x40) != 0)
            {
                this->flags = this->flags & 0xFFFFFFBF;
                this->animation = (resource_model->animations[0]);
                this->frame = (sint16)(-1);
                this->flags = this->flags | 0x10000;
            }
        }
        else if ((this->flags & 0x40) == 0)
        {
            sint32 frame, previous;
            this->flags = this->flags | 0x40;
            this->animation = (resource_model->animations[2]);
            this->frame = (sint16)(-1);
            this->flags = this->flags | 0x10000;
            frame = (sint32)((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->FindHeightFrame(target, this->flags & 0x10, (resource_model->height_offset));
            previous = this->path_frame;
            if (frame < previous)
            {
                uint32 count = ((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->positions.metadata6;
                this->flags = (this->flags & 0xFFFFF7FF) ^ 0x10;
                this->path_frame = (sint16)(count - previous);
            }
        }
    }
    result = this->UpdateActorFacingAndAnimation(0, origin, resource_model);
    return result;
}

uint32 AnimatedObject::UpdateActorFacingAttack()
{
    FUNCTION_MARKER(0x8004425C, "SLUS_005.78");
    ActorModelDefinition *resource_model;
    ActorBinding *state = (ActorBinding *)(((const ActorPlacement *)((const PathDefinition *)((this->path_definition))))->state != 0 ? ((const ActorPlacement *)((const PathDefinition *)((this->path_definition))))->state : 0);
    ActorBinding *binding = state;
    resource_model = binding->model;
    uint32 origin[3], target_pos[3], distance, result;
    AnimationHeader *animation;
    const uint32 *target;
    sint32 frame;
    this->UpdateActorStateAndTarget(origin, target_pos, &distance);
    target = target_pos;
    if ((binding->flags & 2) != 0)
        target = 0;
    else
    {
        if ((((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->flags & 0x40) == 0 && this->animation != (resource_model->animations[0]))
        {
            this->animation = (resource_model->animations[0]);
            this->frame = (sint16)(-1);
            this->flags = this->flags | 0x10000;
        }
        animation = this->animation;
        frame = this->frame;
        if (animation == (resource_model->animations[2]))
        {
            if ((animation == 0 || frame >= ((AnimationHeader *)(animation))->frame_count - 1) && frame > 0)
            {
                this->frame = (sint16)(-1);
                this->animation = (resource_model->animations[0]);
                this->flags = this->flags | 0x10000;
            }
        }
        else if ((((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->flags & 0x40) != 0 && (resource_model->target_range[0]) >= (sint32)distance && (resource_model->target_range[1]) >= (sint32)distance)
        {
            sint16 angle = (sint16)((uint16)this->yaw - Atan2Angle((sint32)(origin[2] - target[2]), (sint32)(origin[0] - target[0])));
            if (angle >= -63 && angle < 64)
            {
                this->frame = (sint16)(-1);
                this->animation = (resource_model->animations[2]);
                this->flags = this->flags | 0x10000;
            }
        }
    }
    result = this->UpdateActorFacingAndAnimation(target, origin, resource_model);
    return result;
}

void AnimatedObject::UpdateActorPathSegmentMotion()
{
    FUNCTION_MARKER(0x800224AC, "SLUS_005.78");
    ActorBinding *state;
    const ActorModelDefinition *resource_model;
    uint32 timer = this->behavior_word - 1, result;
    AnimationHeader *animation;
    const PathDefinition *definition;

    union
    {
        VECTOR vector;
        uint32 words[4];
    } world_pos;

    uint32 axis;
    sint32 previous, current;
    this->behavior_word = timer;
    if (timer != 0xFFFFFFFF || this->ticks == 0)
    {
        (this->*object_operations.motions[4].member_handler)();
        return;
    }
    definition = (this->path_definition);
    state = (ActorBinding *)(((const ActorPlacement *)((const PathDefinition *)(definition)))->state != 0 ? ((const ActorPlacement *)((const PathDefinition *)(definition)))->state : 0);
    ActorBinding *binding = state;
    resource_model = binding->model;
    this->behavior_word = 0;
    ((PathDefinition *)((const PathDefinition *)(definition)))->flags = ((const PathDefinition *)((const PathDefinition *)(definition)))->flags | 0x40;
    previous = this->FindActorPathSegment();
    (this->*object_operations.motions[4].member_handler)();
    result = binding->flags & 2;
    if (result != 0)
        return;
    current = this->FindActorPathSegment();
    if (current == previous && this->path_frame < (sint16)((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->positions.metadata6 - 1)
        animation = (resource_model->animations[2]);
    else
    {
        if (current != previous)
        {
            definition = (this->path_definition);
            if ((this->flags & 0x10) != 0)
            {
                ++previous;
                if (previous == ((const PathDefinition *)((const PathDefinition *)(definition)))->positions.key_count)
                    previous = 0;
            }
            else
            {
                if (previous < 0)
                    previous = ((const PathDefinition *)((const PathDefinition *)(definition)))->positions.key_count - 1;
                this->path_frame = (sint16)(definition->positions.metadata6 - (((const VectorAnimationKey *)definition->positions.keys)[previous].time >> 20) - 1);
            }
            {
                const uint32 *point_src = (const uint32 *)this->position;
                uint32 *point_dst = world_pos.words;
                const PathDefinition *point_definition = (this->path_definition);
                const MATRIX *point_matrix;
                uint32 point_axis;
                uint32 point_x = point_src[0], point_y = point_src[1], point_z = point_src[2];
                if (point_definition != 0 && (point_definition->transform) != 0)
                {
                    union
                    {
                        VECTOR vector;
                        uint32 words[4];
                    } point_input;

                    point_input.words[0] = point_x;
                    point_input.words[1] = point_y;
                    point_input.words[2] = point_z;
                    PushMatrix();
                    point_definition = (const PathDefinition *)((const PathDefinition *)((this->path_definition)));
                    point_matrix = (const MATRIX *)((point_definition->transform) != 0 ? &(point_definition->transform)->matrix : (MATRIX *)0);
                    for (point_axis = 0; point_axis < 5; ++point_axis)
                        xport_gte_write_control(point_axis, xport_load_le32((const uint8 *)point_matrix + point_axis * 4));
                    ApplyRotMatrixLV(&point_input.vector, (VECTOR *)point_dst);
                    PopMatrix();
                    for (point_axis = 0; point_axis < 3; ++point_axis)
                    {
                        point_definition = (const PathDefinition *)((const PathDefinition *)((this->path_definition)));
                        point_matrix = (const MATRIX *)((point_definition->transform) != 0 ? &(point_definition->transform)->matrix : (MATRIX *)0);
                        point_dst[point_axis] = point_dst[point_axis] + (uint32)point_matrix->t[point_axis];
                    }
                }
                else
                {
                    point_dst[0] = point_x;
                    point_dst[1] = point_y;
                    point_dst[2] = point_z;
                }
            }
            definition = (this->path_definition);
            {
                const uint32 *point_src = ((const VectorAnimationKey *)definition->positions.keys)[previous].position;
                uint32 *point_dst = (uint32 *)this->velocity;
                const PathDefinition *point_definition = (this->path_definition);
                const MATRIX *point_matrix;
                uint32 point_axis;
                uint32 point_x = point_src[0], point_y = point_src[1], point_z = point_src[2];
                if (point_definition != 0 && (point_definition->transform) != 0)
                {
                    union
                    {
                        VECTOR vector;
                        uint32 words[4];
                    } point_input;

                    point_input.words[0] = point_x;
                    point_input.words[1] = point_y;
                    point_input.words[2] = point_z;
                    PushMatrix();
                    point_definition = (const PathDefinition *)((const PathDefinition *)((this->path_definition)));
                    point_matrix = (const MATRIX *)((point_definition->transform) != 0 ? &(point_definition->transform)->matrix : (MATRIX *)0);
                    for (point_axis = 0; point_axis < 5; ++point_axis)
                        xport_gte_write_control(point_axis, xport_load_le32((const uint8 *)point_matrix + point_axis * 4));
                    ApplyRotMatrixLV(&point_input.vector, (VECTOR *)point_dst);
                    PopMatrix();
                    for (point_axis = 0; point_axis < 3; ++point_axis)
                    {
                        point_definition = (const PathDefinition *)((const PathDefinition *)((this->path_definition)));
                        point_matrix = (const MATRIX *)((point_definition->transform) != 0 ? &(point_definition->transform)->matrix : (MATRIX *)0);
                        point_dst[point_axis] = point_dst[point_axis] + (uint32)point_matrix->t[point_axis];
                    }
                }
                else
                {
                    point_dst[0] = point_x;
                    point_dst[1] = point_y;
                    point_dst[2] = point_z;
                }
            }
            for (axis = 0; axis < 3; ++axis)
                this->velocity[axis] = this->velocity[axis] - world_pos.words[axis];
        }
        if (this->path_frame < (sint16)((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->positions.metadata6 - 1)
            this->flags = this->flags & 0xFFFFF7FF;
        else
            this->flags = this->flags | 0x800;
        this->behavior_word = 15;
        definition = (this->path_definition);
        ((PathDefinition *)((const PathDefinition *)(definition)))->flags = ((const PathDefinition *)((const PathDefinition *)(definition)))->flags & 0xFFFFFFBF;
        animation = (resource_model->animations[0]);
    }
    if (this->animation != animation)
    {
        {
            AnimationHeader *animation_value = animation;
            AnimatedObject *animation_object = this;
            uint32 animation_flags = animation_object->flags;
            animation_object->animation = animation_value;
            animation_object->frame = -1;
            animation_object->flags = animation_flags | 0x10000;
        }
    }
    return;
}

uint32 AnimatedObject::UpdateActorVerticalShot()
{
    FUNCTION_MARKER(0x80021E70, "SLUS_005.78");
    ActorModelDefinition *resource_model;
    ActorBinding *state = (ActorBinding *)(((const ActorPlacement *)((const PathDefinition *)((this->path_definition))))->state != 0 ? ((const ActorPlacement *)((const PathDefinition *)((this->path_definition))))->state : 0);
    ActorBinding *binding = state;
    resource_model = binding->model;
    uint32 result, timer;
    AnimationHeader *animation;
    AnimatedObject *spawned;
    uint32 origin[3], target_pos[3], distance_output;
    uint32 *target = target_pos;
    sint32 next_frame, frame, next, forward_distance, backward_distance;
    this->UpdateActorStateAndTarget(origin, target, &distance_output);
    if ((binding->flags & 2) != 0)
    {
        target = 0;
        goto finish;
    }
    timer = this->behavior_word;
    if (timer != 0)
    {
        this->behavior_word = --timer;
        if (timer != 0)
            goto finish;
    }
    animation = this->animation;
    next_frame = animation != 0 && this->frame < ((AnimationHeader *)(animation))->frame_count - 1 ? this->frame + 1 : 0;
    if (next_frame != (resource_model->shot_frame) || (((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->flags & 0x40) == 0 || (this->flags & 0x30000000) != 0 || this->behavior == 17 || (sint32)distance_output >= 10001)
        goto finish;
    next = this->ClassifyActorPathDirection(origin, target);
    frame = this->path_frame;
    next += frame;
    if (next < frame)
    {
        uint32 path_frame = (uint16)this->path_frame;
        uint32 path_count = ((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->positions.metadata6;
        AnimatedObject *path_object = this;
        path_object->flags = (path_object->flags & 0xFFFFF7FF) ^ 0x10;
        this->path_frame = (sint16)(path_count - path_frame);
    }
    else if (next == frame)
    {
        if (next >= (sint16)((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->positions.metadata6 - 1)
            forward_distance = (sint32)0x80001000;
        else
        {
            uint32 probe[3];
            uint32 probe_axis, probe_sum = 0;
            sint32 probe_frame = next + 1;
            ((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->SamplePosition(probe_frame, probe, this->flags & 0x10);
            for (probe_axis = 0; probe_axis < 3; ++probe_axis)
                probe[probe_axis] = probe[probe_axis] - target[probe_axis];
            for (probe_axis = 0; probe_axis < 3; ++probe_axis)
            {
                sint32 component = (sint32)probe[probe_axis] >> 12;
                probe_sum += (uint32)component * (uint32)component;
                probe[probe_axis] = component;
            }
            forward_distance = (sint32)probe_sum;
        }
        if (next <= 0)
            backward_distance = 0x7FFFF000;
        else
        {
            uint32 probe[3];
            uint32 probe_axis, probe_sum = 0;
            sint32 probe_frame = next - 1;
            ((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->SamplePosition(probe_frame, probe, this->flags & 0x10);
            for (probe_axis = 0; probe_axis < 3; ++probe_axis)
                probe[probe_axis] = probe[probe_axis] - target[probe_axis];
            for (probe_axis = 0; probe_axis < 3; ++probe_axis)
            {
                sint32 component = (sint32)probe[probe_axis] >> 12;
                probe_sum += (uint32)component * (uint32)component;
                probe[probe_axis] = component;
            }
            backward_distance = (sint32)probe_sum;
        }
        if (backward_distance < (sint32)distance_output || (sint32)distance_output < forward_distance)
        {
            uint32 path_frame = (uint16)this->path_frame;
            uint32 path_count = ((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->positions.metadata6;
            AnimatedObject *path_object = this;
            path_object->flags = (path_object->flags & 0xFFFFF7FF) ^ 0x10;
            this->path_frame = (sint16)(path_count - path_frame);
        }
    }
    this->state = (sint8)((const ActorBinding *)(((const ActorPlacement *)((const PathDefinition *)((this->path_definition))))->state != 0 ? ((const ActorPlacement *)((const PathDefinition *)((this->path_definition))))->state : 0))->model_kind == 8 ? 28 : 26;
    this->flags = this->flags | 0x100;
    spawned = object_operations.behaviors[6].factory_handler(this);
    this->flags = this->flags & 0xFFFFFEFF;
    if (spawned != 0)
    {
        if ((sint8)((const ActorBinding *)(((const ActorPlacement *)((const PathDefinition *)((this->path_definition))))->state != 0 ? ((const ActorPlacement *)((const PathDefinition *)((this->path_definition))))->state : 0))->model_kind != 8)
        {
            {
                const uint32 *point_src = spawned->position;
                uint32 *point_dst = spawned->position;
                const PathDefinition *point_definition = (spawned->path_definition);
                const MATRIX *point_matrix;
                uint32 point_axis;
                uint32 point_x = point_src[0], point_y = point_src[1], point_z = point_src[2];
                if (point_definition != 0 && (point_definition->transform) != 0)
                {
                    union
                    {
                        VECTOR vector;
                        uint32 words[4];
                    } point_input;

                    point_input.words[0] = point_x;
                    point_input.words[1] = point_y;
                    point_input.words[2] = point_z;
                    PushMatrix();
                    point_definition = (const PathDefinition *)((const PathDefinition *)((spawned->path_definition)));
                    point_matrix = (const MATRIX *)((point_definition->transform) != 0 ? &(point_definition->transform)->matrix : (MATRIX *)0);
                    for (point_axis = 0; point_axis < 5; ++point_axis)
                        xport_gte_write_control(point_axis, xport_load_le32((const uint8 *)point_matrix + point_axis * 4));
                    ApplyRotMatrixLV(&point_input.vector, (VECTOR *)point_dst);
                    PopMatrix();
                    for (point_axis = 0; point_axis < 3; ++point_axis)
                    {
                        point_definition = (const PathDefinition *)((const PathDefinition *)((spawned->path_definition)));
                        point_matrix = (const MATRIX *)((point_definition->transform) != 0 ? &(point_definition->transform)->matrix : (MATRIX *)0);
                        point_dst[point_axis] = point_dst[point_axis] + (uint32)point_matrix->t[point_axis];
                    }
                }
                else
                {
                    point_dst[0] = point_x;
                    point_dst[1] = point_y;
                    point_dst[2] = point_z;
                }
            }
            spawned->movement_mode = 0;
            spawned->flags = spawned->flags | 0x2000;
            spawned->velocity[2] = 0;
            spawned->velocity[0] = 0;
            spawned->velocity[1] = spawned->GetModelGroupLaunchSpeed();
        }
        spawned->animation_binding.tracks[0] = -2;
        spawned->animation_binding.tracks[1] = -3;
    }
    this->behavior_word = (uint32)(resource_model->shot_interval);
finish:
    result = this->UpdateActorFacingAndAnimation(target, origin, resource_model);

    return result;
}

uint32 AnimatedObject::UpdateActorAimedShot()
{
    FUNCTION_MARKER(0x8001EDAC, "SLUS_005.78");
    ActorBinding *state;
    ActorModelDefinition *resource_model;
    uint32 origin[3], target_pos[3], distance_output;
    uint32 *target = target_pos;
    uint32 flags = this->flags, was_end = flags & 0x800;
    uint32 magnitude, result;
    AnimationHeader *animation;
    AnimatedObject *spawned;
    sint32 distance, frame, next, count, velocity_y, absolute_y, root, factor;
    uint32 radius, half;
    if (was_end != 0)
    {
        this->flags = flags & 0xFFFFF7FF;
        this->path_frame = (sint16)(((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->positions.metadata6);
    }
    state = (ActorBinding *)(((const ActorPlacement *)((const PathDefinition *)((this->path_definition))))->state != 0 ? ((const ActorPlacement *)((const PathDefinition *)((this->path_definition))))->state : 0);
    ActorBinding *binding = state;
    resource_model = binding->model;
    this->UpdateActorStateAndTarget(origin, target, &distance_output);
    if ((binding->flags & 2) != 0)
    {
        target = 0;
        goto finish;
    }
    if ((((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->flags & 0x40) == 0)
    {
        this->flags = this->flags & ~0x40u;
        goto choose_animation;
    }
    if (this->behavior_word != 0)
        this->behavior_word = this->behavior_word - 1;
    distance = (sint32)distance_output;
    if (this->animation != (resource_model->animations[2]))
    {
        if (((resource_model->target_range[0]) >= distance && this->path_frame != 0) || (resource_model->target_range[1]) < distance)
            goto motion;
        if (this->behavior_word != 0)
            goto finish;
    }
    if ((this->flags & 0x30000000) != 0 || this->behavior == 17)
        goto motion;
    if (this->animation != (resource_model->animations[2]))
    {
        this->flags = this->flags & ~0x40u;
        {
            AnimationHeader *animation_value = (resource_model->animations[2]);
            AnimatedObject *animation_object = this;
            uint32 animation_flags = animation_object->flags;
            animation_object->animation = animation_value;
            animation_object->frame = -1;
            animation_object->flags = animation_flags | 0x10000;
        }
    }
    animation = this->animation;
    next = animation != 0 && this->frame < ((AnimationHeader *)(animation))->frame_count - 1 ? this->frame + 1 : 0;
    if (next == (resource_model->shot_frame))
    {
        this->state = 23;
        this->flags = this->flags | 0x100;
        spawned = object_operations.behaviors[6].factory_handler(this);
        this->flags = this->flags & ~0x100u;
        if (spawned != 0)
        {
            {
                const uint32 *point_src = spawned->position;
                uint32 *point_dst = spawned->position;
                const PathDefinition *point_definition = (spawned->path_definition);
                const MATRIX *point_matrix;
                uint32 point_axis;
                uint32 point_x = point_src[0], point_y = point_src[1], point_z = point_src[2];
                if (point_definition != 0 && (point_definition->transform) != 0)
                {
                    union
                    {
                        VECTOR vector;
                        uint32 words[4];
                    } point_input;

                    point_input.words[0] = point_x;
                    point_input.words[1] = point_y;
                    point_input.words[2] = point_z;
                    PushMatrix();
                    point_definition = (const PathDefinition *)((const PathDefinition *)((spawned->path_definition)));
                    point_matrix = (const MATRIX *)((point_definition->transform) != 0 ? &(point_definition->transform)->matrix : (MATRIX *)0);
                    for (point_axis = 0; point_axis < 5; ++point_axis)
                        xport_gte_write_control(point_axis, xport_load_le32((const uint8 *)point_matrix + point_axis * 4));
                    ApplyRotMatrixLV(&point_input.vector, (VECTOR *)point_dst);
                    PopMatrix();
                    for (point_axis = 0; point_axis < 3; ++point_axis)
                    {
                        point_definition = (const PathDefinition *)((const PathDefinition *)((spawned->path_definition)));
                        point_matrix = (const MATRIX *)((point_definition->transform) != 0 ? &(point_definition->transform)->matrix : (MATRIX *)0);
                        point_dst[point_axis] = point_dst[point_axis] + (uint32)point_matrix->t[point_axis];
                    }
                }
                else
                {
                    point_dst[0] = point_x;
                    point_dst[1] = point_y;
                    point_dst[2] = point_z;
                }
            }
            spawned->movement_mode = 0;
            spawned->flags = spawned->flags | 0x2000;
            const AnimatedObject *projectile_player = FindPlayerObject(0);
            spawned->velocity[0] = target[0] - spawned->position[0];
            spawned->velocity[1] = target[1] - spawned->position[1];
            spawned->velocity[2] = target[2] - spawned->position[2];
            velocity_y = (sint32)(spawned->velocity[1] - 12288 + (projectile_player->metadata_words72[1]));
            spawned->velocity[1] = (uint32)velocity_y;
            absolute_y = velocity_y < 0 ? (sint32)(0u - (uint32)velocity_y) : velocity_y;
            magnitude = (uint32)(((sint64)(sint32)spawned->velocity[0] * (sint32)spawned->velocity[0]) >> 12) + (uint32)(((sint64)(sint32)spawned->velocity[2] * (sint32)spawned->velocity[2]) >> 12);
            root = (sint32)magnitude < 0 ? (sint32)(2u * SquareRoot12((sint32)(magnitude >> 2))) : SquareRoot12((sint32)magnitude);
            distance = root >> 1;
            distance_output = distance;
            if (absolute_y >= distance)
            {
                if (absolute_y == 0)
                {
                    fprintf(stderr, "P2: Zero projectile normalization length at 8001F254\n");
                    abort();
                }
                factor = (sint32)(((sint64)distance * (0x40000000 / absolute_y)) >> 18);
                fixed_math_result = factor;
                spawned->velocity[1] = (uint32)(((sint64)velocity_y * factor) >> 12);
            }
        }
        this->behavior_word = (uint32)(resource_model->shot_interval);
        goto finish;
    }
    frame = this->frame;
    if (frame <= 0 || (animation != 0 && frame < ((AnimationHeader *)(animation))->frame_count - 1))
        goto finish;
    {
        AnimationHeader *animation_value = (resource_model->animations[0]);
        AnimatedObject *animation_object = this;
        uint32 animation_flags = animation_object->flags;
        animation_object->animation = animation_value;
        animation_object->frame = -1;
        animation_object->flags = animation_flags | 0x10000;
    }
    if ((uint16)this->path_frame == 0 && ((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->positions.keys != 0)
        this->path_frame = (sint16)(1);
    goto finish;
motion:
    if (this->behavior_word != 0)
        goto finish;
    if ((resource_model->target_range[0]) < distance && (resource_model->target_range[1]) >= distance)
        goto finish;
    if (distance < 10001)
        next = this->path_frame + this->ClassifyActorPathDirection(origin, target);
    else
    {
        if (was_end != 0)
        {
            uint32 path_frame = (uint16)this->path_frame;
            uint32 path_count = ((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->positions.metadata6;
            AnimatedObject *path_object = this;
            path_object->flags = (path_object->flags & 0xFFFFF7FF) ^ 0x10;
            this->path_frame = (sint16)(path_count - path_frame);
        }
        next = this->path_frame + 1;
    }
    frame = this->path_frame;
    count = (sint16)((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->positions.metadata6;
    if (next < frame)
    {
        if ((resource_model->target_range[1]) < distance)
        {
            radius = ((uint32)resource_model->patrol_radius);
            half = (uint32)((resource_model->patrol_radius) >> 1);
            this->flags = this->flags | 0x40;
            if (frame < count)
            {
                if (radius * 2 >= target[1] - this->world_position[1] + radius && (half * 2 < origin[0] - target[0] + half || half * 2 < origin[2] - target[2] + half))
                {
                    uint32 path_frame = (uint16)this->path_frame;
                    uint32 path_count = ((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->positions.metadata6;
                    AnimatedObject *path_object = this;
                    path_object->flags = (path_object->flags & 0xFFFFF7FF) ^ 0x10;
                    this->path_frame = (sint16)(path_count - path_frame);
                }
            }
            else if (radius * 2 < origin[0] - target[0] + radius || radius * 2 < origin[2] - target[2] + radius)
            {
                uint32 path_frame = (uint16)this->path_frame;
                uint32 path_count = ((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->positions.metadata6;
                AnimatedObject *path_object = this;
                path_object->flags = (path_object->flags & 0xFFFFF7FF) ^ 0x10;
                this->path_frame = (sint16)(path_count - path_frame);
            }
            else
                this->flags = this->flags & ~0x40u;
        }
        else if (frame >= count - 1)
        {
            this->path_frame = (sint16)(0);
            this->flags = (this->flags ^ 0x10) & ~0x40u;
        }
        else
            this->flags = this->flags | 0x40;
    }
    else if ((resource_model->target_range[1]) >= distance)
    {
        if (frame >= 2)
        {
            {
                uint32 path_frame = (uint16)this->path_frame;
                uint32 path_count = ((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->positions.metadata6;
                AnimatedObject *path_object = this;
                path_object->flags = (path_object->flags & 0xFFFFF7FF) ^ 0x10;
                this->path_frame = (sint16)(path_count - path_frame);
            }
            this->flags = this->flags | 0x40;
        }
        else
        {
            this->path_frame = (sint16)(0);
            this->flags = this->flags & ~0x40u;
        }
    }
    else
        this->flags = frame >= count ? this->flags & ~0x40u : this->flags | 0x40;
choose_animation:
    animation = (resource_model->animations[((this->flags & 0x40) != 0 ? 3 : 0)]);
    if (this->animation != animation)
    {
        AnimationHeader *animation_value = animation;
        AnimatedObject *animation_object = this;
        uint32 animation_flags = animation_object->flags;
        animation_object->animation = animation_value;
        animation_object->frame = -1;
        animation_object->flags = animation_flags | 0x10000;
    }
finish:
    result = this->UpdateActorFacingAndAnimation(target, origin, resource_model);

    return result;
}

uint32 AnimatedObject::UpdateActorChaseSpeed()
{
    FUNCTION_MARKER(0x800216CC, "SLUS_005.78");
    const ActorModelDefinition *speed_model;
    ActorBinding *binding;
    ActorBinding *state = (ActorBinding *)(((const ActorPlacement *)((const PathDefinition *)((this->path_definition))))->state != 0 ? ((const ActorPlacement *)((const PathDefinition *)((this->path_definition))))->state : 0);
    binding = state;
    uint32 flags, direction, result;
    sint32 maximum, step, speed;
    if ((binding->flags & 10) != 0 || (this->flags & 0x40) == 0)
        return (this->*object_operations.motions[4].member_handler)();
    flags = this->flags;
    direction = (flags >> 4) & 1;
    speed_model = binding->model;
    if (this->animation == (speed_model->animations[7]))
    {
        maximum = speed_model->motion_speed[0];
        step = (speed_model->motion_accel) >> 2;
    }
    else
    {
        maximum = speed_model->motion_speed[1];
        step = (speed_model->motion_accel);
    }
    speed = (sint32)binding->motion_speed;
    if ((this->flags & 0x40000000) != 0)
    {
        speed = (sint32)((uint32)speed - (uint32)step);
        binding->motion_speed = speed;
        if (speed <= 0)
        {
            binding->motion_speed = 0;
            flags = this->flags & 0xBFFFFFBF;
            this->flags = flags;
            this->path_frame = (sint16)(((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->positions.metadata6 - (uint16)this->path_frame);
            this->flags = (this->flags & 0xFFFFF7FF) ^ 0x10;
        }
    }
    else if (speed < maximum)
    {
        speed = (sint32)((uint32)speed + (uint32)step);
        binding->motion_speed = speed;
        if (speed > maximum)
            binding->motion_speed = maximum;
    }
    (this->*object_operations.motions[4].member_handler)();
    flags = this->flags;
    result = flags & 0x40;
    if (result != 0)
    {
        state = (ActorBinding *)(((const ActorPlacement *)((const PathDefinition *)((this->path_definition))))->state != 0 ? ((const ActorPlacement *)((const PathDefinition *)((this->path_definition))))->state : 0);
        binding = state;
        result = 0x40000000;
        if (this->path_frame >= (sint16)binding->path_parameters[direction != 0 ? 0 : 1])
        {
            result = flags | 0x40000000;
            this->flags = result;
        }
    }
    return result;
}

uint32 AnimatedObject::UpdateActorStationaryShots()
{
    FUNCTION_MARKER(0x8005DE30, "SLUS_005.78");
    ActorModelDefinition *resource_model;
    ActorBinding *binding;
    ActorBinding *state = (ActorBinding *)(((const ActorPlacement *)((const PathDefinition *)((this->path_definition))))->state != 0 ? ((const ActorPlacement *)((const PathDefinition *)((this->path_definition))))->state : 0);
    binding = state;
    resource_model = binding->model;
    uint32 origin[3], target_pos[3], distance_output, result;
    this->UpdateActorStateAndTarget(origin, target_pos, &distance_output);
    if ((binding->flags & 2) == 0 && (((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->flags & 0x40) != 0)
    {
        uint32 facing = this->TurnObjectFacing((const uint32 *)this->velocity, resource_model->turn_step);
        uint32 timer = this->behavior_word;
        if (timer != 0)
            this->behavior_word = --timer;
        if (timer == 0 && (facing & 255) != 0 && (this->flags & 0x30000000) == 0 && this->behavior != 17)
        {
            AnimatedObject *spawned;
            uint32 first, second;
            this->state = 30;
            this->flags = this->flags | 0x100;
            spawned = object_operations.behaviors[6].factory_handler(this);
            this->flags = this->flags & 0xFFFFFEFF;
            if (spawned != 0)
            {
                uint32 flags = spawned->flags;
                spawned->velocity[2] = 0;
                spawned->velocity[1] = 0;
                spawned->velocity[0] = 0;
                spawned->flags = flags & 0xFFFFFBFF;
                first = this->orientation_words[0];
                second = this->orientation_words[1];
                spawned->orientation_words[0] = first;
                spawned->orientation_words[1] = second;
                spawned->animation_binding.tracks[0] = -2;
            }
            this->behavior_word = (uint32)(resource_model->shot_interval);
        }
    }
    result = this->UpdateActorFacingAndAnimation(0, origin, resource_model);

    return result;
}

uint32 AnimatedObject::UpdateActorVerticalPursuit()
{
    FUNCTION_MARKER(0x80060360, "SLUS_005.78");
    ActorModelDefinition *resource_model;
    ActorBinding *binding;
    ActorBinding *state = (ActorBinding *)(((const ActorPlacement *)((const PathDefinition *)((this->path_definition))))->state != 0 ? ((const ActorPlacement *)((const PathDefinition *)((this->path_definition))))->state : 0);
    binding = state;
    resource_model = binding->model;
    uint32 result;
    uint32 origin[3], target_pos[3], distance_output;
    uint32 *target = target_pos;
    sint32 frame, previous, difference, velocity, angle;
    this->UpdateActorStateAndTarget(origin, target, &distance_output);
    if ((binding->flags & 2) != 0)
    {
        result = this->UpdateActorFacingAndAnimation(0, origin, resource_model);

        return result;
    }
    target[1] = target[1] + 14336;
    if ((((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->flags & 0x40) != 0)
    {
        this->flags = this->flags | 0x40;
        if ((sint32)distance_output < 10001)
            frame = this->path_frame + this->ClassifyActorPathDirection(origin, target);
        else
            frame = (sint16)((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->positions.metadata6;
        previous = this->path_frame;
        if (frame < previous)
        {
            uint32 count = ((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->positions.metadata6;
            this->flags = (this->flags & 0xFFFFF7FF) ^ 0x10;
            this->path_frame = (sint16)(count - previous);
        }
        difference = (sint32)(target[1] - origin[1]);
        if ((uint32)difference + 2048 < 4096)
            difference = 0;
        state = (ActorBinding *)(((const ActorPlacement *)((const PathDefinition *)((this->path_definition))))->state != 0 ? ((const ActorPlacement *)((const PathDefinition *)((this->path_definition))))->state : 0);
        binding = state;
        velocity = (sint16)binding->path_pitch_speed;
        angle = this->pitch;
        if (difference != 0)
        {
            if ((angle == 0 && difference >= 0) || (angle > 1024 && difference >= 0))
            {
                velocity += 409;
                if (velocity > 2048)
                    velocity = 2048;
            }
            else if (angle > 1024 && difference < 0)
            {
                velocity -= 409;
                if (velocity < 0)
                    velocity = 0;
            }
            else if (difference <= 0)
            {
                velocity -= 409;
                if (velocity < -2048)
                    velocity = -2048;
            }
            else
            {
                velocity += 409;
                if (velocity > 0)
                    velocity = 0;
            }
        }
        else if (velocity > 0)
        {
            velocity -= 409;
            if (velocity < 0)
                velocity = 0;
        }
        else if (velocity < 0)
        {
            velocity += 409;
            if (velocity > 0)
                velocity = 0;
        }
        if ((this->flags & 0x4000) != 0 && velocity < 0)
            velocity = 0;
        ((ActorBinding *)(((const ActorPlacement *)((const PathDefinition *)((this->path_definition))))->state != 0 ? ((const ActorPlacement *)((const PathDefinition *)((this->path_definition))))->state : 0))->path_pitch_speed = velocity;
    }
    else if ((this->flags & 0x40) != 0)
    {
        this->flags = this->flags & 0xFFFFFFBF;
        ((ActorBinding *)(((const ActorPlacement *)((const PathDefinition *)((this->path_definition))))->state != 0 ? ((const ActorPlacement *)((const PathDefinition *)((this->path_definition))))->state : 0))->path_pitch_speed = 0;
    }
    this->UpdateActorFacingAndAnimation(target, origin, resource_model);
    result = this->flags & 0xFFFFBFFF;
    this->flags = result;

    return result;
}

uint32 AnimatedObject::UpdateActorRandomPatrol()
{
    FUNCTION_MARKER(0x8005F154, "SLUS_005.78");
    ActorBinding *state;
    ActorModelDefinition *resource_model;
    ActorBinding *binding;
    uint32 event, result;
    AnimationHeader *animation;
    const PathDefinition *definition;
    uint32 origin[3], target_pos[3], distance_output;
    uint32 *target = target_pos;
    sint32 frame, direction;
    event = this->UpdateActorStateAndTarget(origin, target, &distance_output);
    definition = (this->path_definition);
    state = (ActorBinding *)(((const ActorPlacement *)((const PathDefinition *)(definition)))->state != 0 ? ((const ActorPlacement *)((const PathDefinition *)(definition)))->state : 0);
    binding = state;
    resource_model = binding->model;
    if ((binding->flags & 2) != 0)
        target = 0;
    else
    {
        if ((((const PathDefinition *)((const PathDefinition *)(definition)))->flags & 0x40) == 0)
        {
            this->flags = this->flags & 0xFFFFFFBF;
            if (this->animation != (resource_model->animations[0]))
            {
                AnimationHeader *animation_value = (resource_model->animations[0]);
                AnimatedObject *animation_object = this;
                uint32 animation_flags = animation_object->flags;
                animation_object->animation = animation_value;
                animation_object->frame = -1;
                animation_object->flags = animation_flags | 0x10000;
            }
        }
        else
        {
            if ((event & 0x400000) != 0 && this->animation != (resource_model->animations[2]))
            {
                this->frame = -1;
                this->animation = (resource_model->animations[2]);
                this->flags = (this->flags | 0x10000) & 0xFFFFFFBF;
            }
            else if (this->animation == (resource_model->animations[2]))
            {
                animation = this->animation;
                frame = this->frame;
                if ((animation == 0 || frame >= ((AnimationHeader *)(animation))->frame_count - 1) && frame > 0)
                {
                    animation = resource_model->animations[((random_generator.Next() & 1) != 0 ? 0 : 3)];
                    {
                        AnimationHeader *animation_value = animation;
                        AnimatedObject *animation_object = this;
                        uint32 animation_flags = animation_object->flags;
                        animation_object->animation = animation_value;
                        animation_object->frame = -1;
                        animation_object->flags = animation_flags | 0x10000;
                    }
                    this->behavior_word = 1;
                }
            }
            else
            {
                uint32 timer = this->behavior_word - 1;
                this->behavior_word = timer;
                if (timer == 0)
                {
                    this->behavior_word = random_generator.Next() % 75 + 30;
                    if (this->animation == (resource_model->animations[3]))
                    {
                        {
                            AnimationHeader *animation_value = (resource_model->animations[0]);
                            AnimatedObject *animation_object = this;
                            uint32 animation_flags = animation_object->flags;
                            animation_object->animation = animation_value;
                            animation_object->frame = -1;
                            animation_object->flags = animation_flags | 0x10000;
                        }
                        this->flags = this->flags & 0xFFFFFFBF;
                    }
                    else
                    {
                        {
                            AnimationHeader *animation_value = (resource_model->animations[3]);
                            AnimatedObject *animation_object = this;
                            uint32 animation_flags = animation_object->flags;
                            animation_object->animation = animation_value;
                            animation_object->frame = -1;
                            animation_object->flags = animation_flags | 0x10000;
                        }
                        this->flags = this->flags | 0x40;
                    }
                }
                if ((resource_model->target_range[0]) >= (sint32)distance_output)
                {
                    direction = this->ClassifyActorPathDirection(origin, target);
                    if (direction < 0)
                    {
                        uint32 path_frame = (uint16)this->path_frame;
                        uint32 path_count = ((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->positions.metadata6;
                        AnimatedObject *path_object = this;
                        path_object->flags = (path_object->flags & 0xFFFFF7FF) ^ 0x10;
                        this->path_frame = (sint16)(path_count - path_frame);
                    }
                    else if (direction == 0 && this->animation == (resource_model->animations[3]))
                    {
                        this->flags = this->flags & 0xFFFFFFBF;
                        {
                            AnimationHeader *animation_value = (resource_model->animations[0]);
                            AnimatedObject *animation_object = this;
                            uint32 animation_flags = animation_object->flags;
                            animation_object->animation = animation_value;
                            animation_object->frame = -1;
                            animation_object->flags = animation_flags | 0x10000;
                        }
                    }
                }
            }
        }
        if ((resource_model->target_range[0]) < (sint32)distance_output)
            target = 0;
    }
    result = this->UpdateActorFacingAndAnimation(target, origin, resource_model);

    return result;
}

uint32 AnimatedObject::UpdateActorPathDetour()
{
    FUNCTION_MARKER(0x8005F510, "SLUS_005.78");
    ActorModelDefinition *resource_model;
    ActorBinding *binding;
    ActorBinding *state = (ActorBinding *)(((const ActorPlacement *)((const PathDefinition *)((this->path_definition))))->state != 0 ? ((const ActorPlacement *)((const PathDefinition *)((this->path_definition))))->state : 0);
    binding = state;
    resource_model = binding->model;
    uint32 result, sum, axis;
    uint32 origin[3], target_pos[3], distance_output;
    uint32 *target = target_pos;
    uint32 selected[3];
    sint32 frame, current, next, difference, velocity;
    this->UpdateActorStateAndTarget(origin, target, &distance_output);
    if ((sint8)binding->action_index >= 0 || (sint8)binding->route_direction_mode != 0)
    {
        result = this->UpdateActorFacingAndAnimation(target, origin, resource_model);

        return result;
    }
    if ((((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->flags & 0x40) == 0)
    {
        uint32 packed = this->behavior_word;
        if ((packed & 0x8000) != 0)
        {
            selected[0] = (uint32)((sint32)packed >> 16) << 12;
            selected[1] = (uint32)(sint32)(sint16)binding->path_parameters[0] << 12;
            selected[2] = (uint32)(sint32)(sint16)binding->path_parameters[1] << 12;
            frame = packed & 0x7FFF;
        }
        else
        {
            for (axis = 0; axis < 3; ++axis)
                selected[axis] = target[axis];
            frame = (sint32)((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->FindHeightFrame(selected, 1, 4096000);
            packed = ((uint32)(((sint32)selected[0] >> 12) - 3 + (sint32)(random_generator.Next() & 7)) << 16) | (uint32)frame | 0x8000;
            this->behavior_word = packed;
            binding->path_parameters[0] = ((sint32)selected[1] >> 12) + (random_generator.Next() & 7);
            binding->path_parameters[1] = ((sint32)selected[2] >> 12) + (random_generator.Next() & 7) - 3;
        }
        if ((this->flags & 0x10) == 0)
            frame = (sint16)((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->positions.metadata6 - frame - 1;
        sum = 0;
        for (axis = 0; axis < 3; ++axis)
        {
            difference = (sint32)(origin[axis] - selected[axis]) >> 12;
            sum += (uint32)difference * (uint32)difference;
        }
        if ((resource_model->target_range[0]) < (sint32)sum)
            goto stop_motion;
        current = this->path_frame;
        if (frame < current)
        {
            if (current < (sint16)((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->positions.metadata6 - 1)
                this->flags = this->flags | 0x40;
            else
            {
                this->path_frame = (sint16)(0);
                this->flags = (this->flags ^ 0x10) & 0xFFFFFFBF;
            }
        }
        else if (current == 0)
            this->flags = this->flags & 0xFFFFFFBF;
        else
        {
            this->flags = ((this->flags & 0xFFFFF7FF) ^ 0x10) | 0x40;
            this->path_frame = (sint16)(((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->positions.metadata6 - current);
        }
        this->flags = this->flags | 0x8000000;
        if (this->animation != (resource_model->animations[3]))
        {
            AnimationHeader *animation_value = (resource_model->animations[3]);
            AnimatedObject *animation_object = this;
            uint32 animation_flags = animation_object->flags;
            animation_object->animation = animation_value;
            animation_object->frame = -1;
            animation_object->flags = animation_flags | 0x10000;
        }
        goto finish_motion;
    }
    if ((this->behavior_word & 0x8000) != 0)
    {
        this->behavior_word = ((random_generator.Next() & 7) - 3) << 16;
        binding->path_parameters[0] = random_generator.Next() & 3;
        random_generator.Next();
        binding->path_parameters[1] = 0;
        selected[0] = (uint32)(sint32)(sint16)(this->behavior_word >> 16) << 12;
        selected[1] = (uint32)(sint32)(sint16)binding->path_parameters[0] << 12;
        selected[2] = (uint32)(sint32)(sint16)binding->path_parameters[1] << 12;
        for (axis = 0; axis < 3; ++axis)
            target[axis] = target[axis] + selected[axis];
    }
    if ((sint32)distance_output >= 10000)
        goto stop_motion;
    next = this->ClassifyActorPathDirection(origin, target);
    this->flags = (this->flags & 0xF7FFFFFF) | 0x40;
    current = this->path_frame;
    next += current;
    if (this->animation != (resource_model->animations[2]))
    {
        AnimationHeader *animation_value = (resource_model->animations[2]);
        AnimatedObject *animation_object = this;
        uint32 animation_flags = animation_object->flags;
        animation_object->animation = animation_value;
        animation_object->frame = -1;
        animation_object->flags = animation_flags | 0x10000;
    }
    current = this->path_frame;
    if (next < current)
    {
        if (current == 0)
            this->flags = this->flags & 0xFFFFFFBF;
        else if ((binding->reaction_flags & 0x80) == 0)
        {
            this->flags = ((this->flags & 0xFFFFF7FF) ^ 0x10) | 0x40;
            this->path_frame = (sint16)(((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->positions.metadata6 - current);
        }
    }
    else if (current < (sint16)((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->positions.metadata6 - 1)
        this->flags = this->flags | 0x40;
    else
    {
        this->path_frame = (sint16)(0);
        this->flags = (this->flags ^ 0x10) & 0xFFFFFFBF;
    }
    difference = (sint32)(target[1] - origin[1]);
    velocity = difference;
    if (difference != 0)
    {
        velocity = (sint16)((ActorBinding *)(((const ActorPlacement *)((const PathDefinition *)((this->path_definition))))->state != 0 ? ((const ActorPlacement *)((const PathDefinition *)((this->path_definition))))->state : 0))->path_pitch_speed + (difference < 0 ? -204 : 204);
        if (velocity < -4096)
            velocity = -4096;
        if (velocity > 4096)
            velocity = 4096;
    }
    if ((this->flags & 0x4000) != 0 && velocity < 0)
        velocity = 0;
    ((ActorBinding *)(((const ActorPlacement *)((const PathDefinition *)((this->path_definition))))->state != 0 ? ((const ActorPlacement *)((const PathDefinition *)((this->path_definition))))->state : 0))->path_pitch_speed = velocity;
    goto finish_motion;
stop_motion:
    this->flags = this->flags & 0xFFFFFFBF;
finish_motion:
    this->UpdateActorFacingAndAnimation(target, origin, resource_model);
    this->flags = this->flags & 0xFFFFBFFF;
    result = (camera_projection.rotation[1] + 2048) & 4095;
    this->orientation[2] = (sint16)(result);

    return result;
}

uint32 AnimatedObject::UpdateActorProximityAttack()
{
    FUNCTION_MARKER(0x8005E010, "SLUS_005.78");
    ActorBinding *state;
    ActorModelDefinition *resource_model;
    ActorBinding *binding;
    uint32 timer, result;
    AnimationHeader *animation;
    uint32 origin[3], target_pos[3], distance_output;
    uint32 *target = target_pos;
    sint32 frame;
    this->UpdateActorStateAndTarget(origin, target, &distance_output);
    state = (ActorBinding *)(((const ActorPlacement *)((const PathDefinition *)((this->path_definition))))->state != 0 ? ((const ActorPlacement *)((const PathDefinition *)((this->path_definition))))->state : 0);
    binding = state;
    resource_model = binding->model;
    if ((binding->flags & 2) != 0)
    {
        target = 0;
        goto finish;
    }
    if ((binding->flags & 8) != 0)
        goto finish;
    if ((binding->reaction_flags & 8) != 0)
        this->behavior_word = 30;
    if ((((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->flags & 0x40) == 0)
    {
        if (this->animation != (resource_model->animations[0]))
        {
            this->flags = this->flags & 0xFBFFFFFF;
            {
                AnimationHeader *animation_value = (resource_model->animations[0]);
                AnimatedObject *animation_object = this;
                uint32 animation_flags = animation_object->flags;
                animation_object->animation = animation_value;
                animation_object->frame = -1;
                animation_object->flags = animation_flags | 0x10000;
            }
        }
        goto finish;
    }
    timer = this->behavior_word;
    if (timer != 0)
    {
        this->behavior_word = --timer;
        if (timer != 0)
        {
            distance_output = 0x7FFFF000;
            if (this->animation != (resource_model->animations[2]))
            {
                uint32 count;
                this->flags = (this->flags & 0xF7FFFFFF) | 0x40;
                {
                    AnimationHeader *animation_value = (resource_model->animations[2]);
                    AnimatedObject *animation_object = this;
                    uint32 animation_flags = animation_object->flags;
                    animation_object->animation = animation_value;
                    animation_object->frame = -1;
                    animation_object->flags = animation_flags | 0x10000;
                }
                count = ((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->positions.metadata6;
                if (((sint32)(count << 16) >> 17) < this->path_frame)
                {
                    uint32 path_frame = (uint16)this->path_frame;
                    uint32 path_count = ((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->positions.metadata6;
                    AnimatedObject *path_object = this;
                    path_object->flags = (path_object->flags & 0xFFFFF7FF) ^ 0x10;
                    this->path_frame = (sint16)(path_count - path_frame);
                }
            }
        }
    }
    animation = this->animation;
    frame = this->frame;
    if (animation == (resource_model->animations[3]) && ((animation != 0 && frame < ((AnimationHeader *)(animation))->frame_count - 1) || frame < 0))
    {
        target = 0;
        goto finish;
    }
    this->flags = this->flags & 0xFBFFFFFF;
    if ((resource_model->target_range[0]) >= (sint32)distance_output && target[1] - this->world_position[1] + 12288 < 24577)
    {
        sint16 angle = 64;
        if ((resource_model->target_range[1]) >= (sint32)distance_output)
            angle = (sint16)((uint16)this->yaw - Atan2Angle((sint32)(origin[2] - target[2]), (sint32)(origin[0] - target[0])));
        if (angle >= -63 && angle < 64)
        {
            this->flags = this->flags | 0x4000000;
            if (this->animation != (resource_model->animations[3]))
            {
                this->flags = this->flags & 0xFFFFFFBF;
                {
                    AnimationHeader *animation_value = (resource_model->animations[3]);
                    AnimatedObject *animation_object = this;
                    uint32 animation_flags = animation_object->flags;
                    animation_object->animation = animation_value;
                    animation_object->frame = -1;
                    animation_object->flags = animation_flags | 0x10000;
                }
            }
        }
        else if (this->animation != (resource_model->animations[2]))
        {
            this->flags = this->flags | 0x8000040;
            {
                AnimationHeader *animation_value = (resource_model->animations[2]);
                AnimatedObject *animation_object = this;
                uint32 animation_flags = animation_object->flags;
                animation_object->animation = animation_value;
                animation_object->frame = -1;
                animation_object->flags = animation_flags | 0x10000;
            }
        }
        if (this->ClassifyActorPathDirection(origin, target) < 0)
        {
            uint32 path_frame = (uint16)this->path_frame;
            uint32 path_count = ((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->positions.metadata6;
            AnimatedObject *path_object = this;
            path_object->flags = (path_object->flags & 0xFFFFF7FF) ^ 0x10;
            this->path_frame = (sint16)(path_count - path_frame);
        }
    }
    else
    {
        const PathDefinition *definition = (this->path_definition);
        const uint16 *limit = ((ActorBinding *)(((const ActorPlacement *)((const PathDefinition *)(definition)))->state != 0 ? ((const ActorPlacement *)((const PathDefinition *)(definition)))->state : 0))->path_parameters + ((this->flags & 0x10) != 0 ? 0 : 1);
        if (this->path_frame >= (sint16)*limit)
        {
            uint32 flags = this->flags;
            this->path_frame = (sint16)(((const PathDefinition *)((const PathDefinition *)(definition)))->positions.metadata6 - this->path_frame);
            this->flags = (flags & 0xFFFFF7FF) ^ 0x10;
        }
        if (this->behavior_word == 0 && this->animation != (resource_model->animations[0]) && (binding->flags & 8) == 0)
        {
            this->flags = this->flags | 0x40;
            {
                AnimationHeader *animation_value = (resource_model->animations[0]);
                AnimatedObject *animation_object = this;
                uint32 animation_flags = animation_object->flags;
                animation_object->animation = animation_value;
                animation_object->frame = -1;
                animation_object->flags = animation_flags | 0x10000;
            }
        }
    }
finish:
    result = this->UpdateActorFacingAndAnimation(target, origin, resource_model);

    return result;
}

uint32 AnimatedObject::UpdateActorChaseAttack()
{
    FUNCTION_MARKER(0x80021888, "SLUS_005.78");
    ActorModelDefinition *resource_model;
    ActorBinding *binding;
    ActorBinding *state = (ActorBinding *)(((const ActorPlacement *)((const PathDefinition *)((this->path_definition))))->state != 0 ? ((const ActorPlacement *)((const PathDefinition *)((this->path_definition))))->state : 0);
    binding = state;
    resource_model = binding->model;
    uint32 result, event, flags;
    AnimationHeader *animation;
    uint32 origin[3], target_pos[3], distance_output;
    uint32 *target = target_pos;
    sint32 close, frame, projected;
    event = this->UpdateActorStateAndTarget(origin, target, &distance_output);
    if ((binding->flags & 2) != 0)
    {
        target = 0;
        goto finish;
    }
    if ((binding->flags & 8) != 0)
        goto finish;
    close = (resource_model->target_range[0]) >= (sint32)distance_output && ((event & 0x1800000) != 0 || (binding->reaction_flags & 2) != 0);
    if ((((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->flags & 0x40) == 0)
    {
        if ((this->flags & 0x40) != 0)
            this->flags = this->flags & 0xFFFFFFBF;
        if (this->animation != (resource_model->animations[0]))
        {
            AnimationHeader *animation_value = (resource_model->animations[0]);
            AnimatedObject *animation_object = this;
            uint32 animation_flags = animation_object->flags;
            animation_object->animation = animation_value;
            animation_object->frame = -1;
            animation_object->flags = animation_flags | 0x10000;
        }
        goto finish;
    }
    animation = this->animation;
    if (close && animation != (resource_model->animations[3]) && animation != (resource_model->animations[0]) && animation != (resource_model->animations[8]))
        goto start_chase;
    if (animation == (resource_model->animations[0]))
    {
        if (close)
        {
            AnimationHeader *animation_value = (resource_model->animations[3]);
            AnimatedObject *animation_object = this;
            uint32 animation_flags = animation_object->flags;
            animation_object->animation = animation_value;
            animation_object->frame = -1;
            animation_object->flags = animation_flags | 0x10000;
        }
        else
        {
            frame = animation != 0 && this->frame < ((AnimationHeader *)(animation))->frame_count - 1 ? this->frame + 1 : 0;
            this->frame = frame;
        }
        target = origin;
        goto finish;
    }
    if (animation == (resource_model->animations[2]))
    {
        uint32 timer = this->behavior_word - 1;
        this->behavior_word = timer;
        if (timer == 0)
        {
            if ((this->flags & 0x40) == 0)
                this->flags = this->flags | 0x40;
            this->flags = this->flags | 0x40000000;
            {
                AnimationHeader *animation_value = (resource_model->animations[7]);
                AnimatedObject *animation_object = this;
                uint32 animation_flags = animation_object->flags;
                animation_object->animation = animation_value;
                animation_object->frame = -1;
                animation_object->flags = animation_flags | 0x10000;
            }
            goto finish;
        }
        goto chase;
    }
    if (animation == (resource_model->animations[7]))
    {
        if (binding->motion_speed == 0)
            goto stop_chase;
        if ((this->flags & 0x40) == 0)
            this->flags = this->flags | 0x40;
        goto finish;
    }
    if (animation == (resource_model->animations[8]))
    {
        frame = this->frame;
        if ((animation == 0 || frame >= ((AnimationHeader *)(animation))->frame_count - 1) && frame > 0)
        {
            AnimationHeader *animation_value = (resource_model->animations[0]);
            AnimatedObject *animation_object = this;
            uint32 animation_flags = animation_object->flags;
            animation_object->animation = animation_value;
            animation_object->frame = -1;
            animation_object->flags = animation_flags | 0x10000;
        }
        else
        {
            animation = this->animation;
            frame = animation != 0 && this->frame < ((AnimationHeader *)(animation))->frame_count - 1 ? this->frame + 1 : 0;
            this->frame = frame;
        }
        target = origin;
        goto finish;
    }
    if (animation == (resource_model->animations[3]))
    {
        frame = this->frame;
        if ((animation == 0 || frame >= ((AnimationHeader *)(animation))->frame_count - 1) && frame > 0)
            goto start_chase;
    }
    goto finish;
start_chase:
    this->behavior_word = 240;
    if (this->animation != (resource_model->animations[2]))
    {
        AnimationHeader *animation_value = (resource_model->animations[2]);
        AnimatedObject *animation_object = this;
        uint32 animation_flags = animation_object->flags;
        animation_object->animation = animation_value;
        animation_object->frame = -1;
        animation_object->flags = animation_flags | 0x10000;
    }
chase:
    if ((sint32)distance_output < 10001)
        projected = (sint32)((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->FindHeightFrame(target, this->flags & 0x10, 409600);
    else
        projected = (sint16)((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->positions.metadata6;
    frame = this->path_frame;
    flags = this->flags;
    if (projected >= frame)
        this->flags = flags & 0xBFFFFFFF;
    else if ((flags & 0x40) != 0)
        this->flags = flags | 0x40000000;
    else
    {
        this->flags = ((flags & 0xFFFFF7FF) ^ 0x10) & 0xBFFFFFFF;
        this->path_frame = (sint16)(((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->positions.metadata6 - frame);
    }
    flags = this->flags;
    frame = this->path_frame;
    if ((flags & 0x40000000) == 0)
    {
        const uint16 *limit;
        if (frame == 0 && projected == 0)
            goto stop_chase;
        limit = ((ActorBinding *)(((const ActorPlacement *)((const PathDefinition *)((this->path_definition))))->state != 0 ? ((const ActorPlacement *)((const PathDefinition *)((this->path_definition))))->state : 0))->path_parameters + ((flags & 0x10) != 0 ? 0 : 1);
        if (frame >= (sint16)*limit && projected >= frame)
            goto stop_chase;
    }
    this->flags = this->flags | 0x40;
    goto finish;
stop_chase:
    this->flags = this->flags & 0xFFFFFFBF;
    {
        AnimationHeader *animation_value = (resource_model->animations[8]);
        AnimatedObject *animation_object = this;
        uint32 animation_flags = animation_object->flags;
        animation_object->animation = animation_value;
        animation_object->frame = -1;
        animation_object->flags = animation_flags | 0x10000;
    }
    this->target_yaw = (sint16)(((uint16)this->target_yaw + 1024) & 4095);
    target = origin;
finish:
    result = this->UpdateActorFacingAndAnimation(target, origin, resource_model);

    return result;
}

uint32 AnimatedObject::InitializeCommonActor()
{
    FUNCTION_MARKER(0x8002AFA8, "SLUS_005.78");
    uint32 result;
    const PathDefinition *parent;
    if ((!object_operations.behaviors[7].plain_handler && !object_operations.behaviors[7].member_handler && !object_operations.behaviors[7].void_member_handler && !object_operations.behaviors[7].factory_handler))
    {
        ObjectCallback operation = {0, &AnimatedObject::UpdateCommonActor};
        object_operations.behaviors[7] = operation;
    }
    parent = (this->path_definition);
    this->behavior = 7;
    result = ((const PathDefinition *)(parent))->positions.keys != 0;
    if (result != 0)
    {
        result = this->flags | 0x40;
        this->flags = result;
    }
    return result;
}

uint32 AnimatedObject::UpdateCommonActor()
{
    FUNCTION_MARKER(0x8002B000, "SLUS_005.78");
    ActorModelDefinition *resource_model;
    uint32 origin[3], target[3], distance, result;
    this->UpdateActorStateAndTarget(origin, target, &distance);
    ActorBinding *binding = (ActorBinding *)(((const ActorPlacement *)((const PathDefinition *)((this->path_definition))))->state != 0 ? ((const ActorPlacement *)((const PathDefinition *)((this->path_definition))))->state : 0);
    resource_model = binding->model;
    result = this->UpdateActorFacingAndAnimation(0, origin, resource_model);
    return result;
}

uint32 AnimatedObject::InitializeActorAimedShot()
{
    FUNCTION_MARKER(0x8001F6AC, "SLUS_005.78");
    if (!object_operations.behaviors[18].plain_handler && !object_operations.behaviors[18].member_handler && !object_operations.behaviors[18].void_member_handler && !object_operations.behaviors[18].factory_handler)
    {
        ObjectCallback operation = {0, &AnimatedObject::UpdateActorAimedShot};
        object_operations.behaviors[18] = operation;
    }
    this->behavior = 18;
    return 18;
}

uint32 AnimatedObject::InitializeActorVerticalShot()
{
    FUNCTION_MARKER(0x80022358, "SLUS_005.78");
    if (!object_operations.behaviors[14].plain_handler && !object_operations.behaviors[14].member_handler && !object_operations.behaviors[14].void_member_handler && !object_operations.behaviors[14].factory_handler)
    {
        ObjectCallback operation = {0, &AnimatedObject::UpdateActorVerticalShot};
        object_operations.behaviors[14] = operation;
    }
    this->behavior = 14;
    return 14;
}

uint32 AnimatedObject::InitializeActorTargetPursuit()
{
    FUNCTION_MARKER(0x8003FF3C, "SLUS_005.78");
    if (!object_operations.behaviors[13].plain_handler && !object_operations.behaviors[13].member_handler && !object_operations.behaviors[13].void_member_handler && !object_operations.behaviors[13].factory_handler)
    {
        ObjectCallback operation = {0, &AnimatedObject::UpdateActorTargetPursuit};
        object_operations.behaviors[13] = operation;
    }
    this->behavior = 13;
    return 13;
}

uint32 AnimatedObject::InitializeActorFacingAttack()
{
    FUNCTION_MARKER(0x80044444, "SLUS_005.78");
    if (!object_operations.behaviors[8].plain_handler && !object_operations.behaviors[8].member_handler && !object_operations.behaviors[8].void_member_handler && !object_operations.behaviors[8].factory_handler)
    {
        ObjectCallback operation = {0, &AnimatedObject::UpdateActorFacingAttack};
        object_operations.behaviors[8] = operation;
    }
    this->behavior = 8;
    return 8;
}

uint32 AnimatedObject::InitializeActorChaseAttack()
{
    FUNCTION_MARKER(0x80021E28, "SLUS_005.78");
    if (!object_operations.behaviors[19].plain_handler && !object_operations.behaviors[19].member_handler && !object_operations.behaviors[19].void_member_handler && !object_operations.behaviors[19].factory_handler)
    {
        {
            ObjectCallback operation = {0, &AnimatedObject::UpdateActorChaseSpeed};
            object_operations.motions[6] = operation;
        }
        {
            ObjectCallback operation = {0, &AnimatedObject::UpdateActorChaseAttack};
            object_operations.behaviors[19] = operation;
        }
    }
    this->behavior = 19;
    this->movement_mode = 6;
    return 6;
}

uint32 AnimatedObject::InitializeActorPathSegmentMotion()
{
    FUNCTION_MARKER(0x8002293C, "SLUS_005.78");
    uint32 result;
    const PathDefinition *path_definition;
    if (!object_operations.behaviors[21].plain_handler && !object_operations.behaviors[21].member_handler && !object_operations.behaviors[21].void_member_handler && !object_operations.behaviors[21].factory_handler)
    {
        {
            ObjectCallback operation = {0, 0, 0, &AnimatedObject::UpdateActorPathSegmentMotion};
            object_operations.motions[5] = operation;
        }
        {
            ObjectCallback operation = {0, &AnimatedObject::UpdateBasicActor};
            object_operations.behaviors[21] = operation;
        }
    }
    path_definition = (this->path_definition);
    this->behavior = 21;
    result = (uint32)((const PathDefinition *)(path_definition))->positions.keys;
    if (result != 0)
    {
        result = this->flags;
        this->movement_mode = 5;
        result |= 0x40;
        this->flags = result;
    }
    this->behavior_word = 0;
    return result;
}

uint32 AnimatedObject::InitializeActorPatrolPause()
{
    FUNCTION_MARKER(0x80027A8C, "SLUS_005.78");
    if (!object_operations.behaviors[20].plain_handler && !object_operations.behaviors[20].member_handler && !object_operations.behaviors[20].void_member_handler && !object_operations.behaviors[20].factory_handler)
    {
        ObjectCallback operation = {0, &AnimatedObject::UpdateActorPatrolPause};
        object_operations.behaviors[20] = operation;
    }
    this->behavior = 20;
    this->behavior_word = 0;
    return 20;
}

uint32 AnimatedObject::InitializeActorStationaryShots()
{
    FUNCTION_MARKER(0x8005DFB8, "SLUS_005.78");
    uint32 result;
    const PathDefinition *path_definition;
    if (!object_operations.behaviors[15].plain_handler && !object_operations.behaviors[15].member_handler && !object_operations.behaviors[15].void_member_handler && !object_operations.behaviors[15].factory_handler)
    {
        ObjectCallback operation = {0, &AnimatedObject::UpdateActorStationaryShots};
        object_operations.behaviors[15] = operation;
    }
    path_definition = (this->path_definition);
    this->behavior = 15;
    result = (uint32)((const PathDefinition *)(path_definition))->positions.keys;
    if (result != 0)
    {
        result = this->flags | 0x40;
        this->flags = result;
    }
    this->behavior_word = 0;
    return result;
}

uint32 AnimatedObject::InitializeActorProximityAttack()
{
    FUNCTION_MARKER(0x8005E418, "SLUS_005.78");
    uint32 result;
    const PathDefinition *path_definition;
    if (!object_operations.behaviors[10].plain_handler && !object_operations.behaviors[10].member_handler && !object_operations.behaviors[10].void_member_handler && !object_operations.behaviors[10].factory_handler)
    {
        ObjectCallback operation = {0, &AnimatedObject::UpdateActorProximityAttack};
        object_operations.behaviors[10] = operation;
    }
    path_definition = (this->path_definition);
    this->behavior = 10;
    result = (uint32)((const PathDefinition *)(path_definition))->positions.keys;
    if (result != 0)
    {
        result = this->flags | 0x40;
        this->flags = result;
    }
    return result;
}

uint32 AnimatedObject::InitializeActorPathDetour()
{
    FUNCTION_MARKER(0x8005FADC, "SLUS_005.78");
    uint32 result;
    const PathDefinition *path_definition;
    if (!object_operations.behaviors[24].plain_handler && !object_operations.behaviors[24].member_handler && !object_operations.behaviors[24].void_member_handler && !object_operations.behaviors[24].factory_handler)
    {
        ObjectCallback operation = {0, &AnimatedObject::UpdateActorPathDetour};
        object_operations.behaviors[24] = operation;
    }
    path_definition = (this->path_definition);
    this->behavior = 24;
    result = ((const PathDefinition *)(path_definition))->positions.keys != 0;
    if (result != 0)
    {
        result = this->flags | 0x400;
        this->flags = result;
    }
    void *state = ((const ActorPlacement *)((const PathDefinition *)((this->path_definition))))->state;
    ActorBinding *binding = (ActorBinding *)(state);
    binding->path_pitch_speed = 0;
    return (uint32)state;
}

uint32 AnimatedObject::InitializeActorVerticalPursuit()
{
    FUNCTION_MARKER(0x80060688, "SLUS_005.78");
    uint32 result;
    const PathDefinition *path_definition;
    if (!object_operations.behaviors[23].plain_handler && !object_operations.behaviors[23].member_handler && !object_operations.behaviors[23].void_member_handler && !object_operations.behaviors[23].factory_handler)
    {
        ObjectCallback operation = {0, &AnimatedObject::UpdateActorVerticalPursuit};
        object_operations.behaviors[23] = operation;
    }
    path_definition = (this->path_definition);
    this->behavior = 23;
    result = ((const PathDefinition *)(path_definition))->positions.keys != 0;
    if (result != 0)
    {
        result = this->flags | 0x440;
        this->flags = result;
    }
    void *state = ((const ActorPlacement *)((const PathDefinition *)((this->path_definition))))->state;
    ActorBinding *binding = (ActorBinding *)(state);
    binding->path_pitch_speed = 0;
    pitch_scale = (sint16)(0);
    return (uint32)state;
}

uint32 AnimatedObject::InitializeActorRandomPatrol()
{
    FUNCTION_MARKER(0x8005F490, "SLUS_005.78");
    uint32 result;
    if (!object_operations.behaviors[12].plain_handler && !object_operations.behaviors[12].member_handler && !object_operations.behaviors[12].void_member_handler && !object_operations.behaviors[12].factory_handler)
    {
        ObjectCallback operation = {0, &AnimatedObject::UpdateActorRandomPatrol};
        object_operations.behaviors[12] = operation;
    }
    this->behavior = 12;
    result = random_generator.Next() % 75 + 15;
    this->behavior_word = result;
    return result;
}
