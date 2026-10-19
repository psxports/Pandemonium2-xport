#include <stdio.h>
#include <stdlib.h>

#include "game.h"

CharacterController character_controller = {0};

uint32 AnimatedObject::UpdateCharacterBehavior()
{
    FUNCTION_MARKER(0x800530A0, "SLUS_005.78");
    uint32 state = 0x800ABD68;
    uint32 flags, value, second, third, fourth, animation, kind, result;
    AnimationHeader *source;
    sint32 frame, next = 0;
    if ((sint8)this->metadata126[1] == 1)
        return 1;
    if ((character_controller.flags & 0x4000000) != 0)
        return this->UpdateCharacterCameraTransition();
    this->ApplyCharacterContactMotion();
    if ((this->flags & 0x2000000) != 0)
    {
        value = character_controller.vitals.damage_ticks - 1;
        character_controller.vitals.damage_ticks = value;
        if ((sint16)value <= 0)
        {
            this->flags = this->flags & 0xEDF7FFFF;
            character_controller.vitals.damage_ticks = 0;
        }
        else
            this->flags = this->flags ^ 0x80000;
    }
    this->ProcessCharacterPickups();
    this->ProcessCharacterTriggers();
    this->UpdateCharacterContactState();
    this->UpdateCharacterFallTimeout();
    if ((this->UpdateCharacterDeathState() & 0xFF) == 0)
        goto finish;
    result = this->ProcessCharacterSceneEvent() << 24;
    if (result != 0)
        return result;
    if ((character_controller.flags & 0x20) != 0)
    {
        const SceneTrigger *camera_trigger = (const SceneTrigger *)character_controller.surface_interaction.selected;
        fourth = camera_trigger->control_word;
        const ScenePath *camera_path = camera_trigger->parameters.path.target;
        second = camera_trigger->parameters.path.values[0];
        third = camera_trigger->parameters.path.values[1];
        this->BeginCharacterCameraTransition(camera_path, second, third, fourth);
        character_controller.flags = character_controller.flags & 0xFFFFFFDD;
        goto finish;
    }
    this->UpdateCharacterActionEffects();
    if ((sint8)character_controller.motion.state_code == 23)
        goto finish;
    source = this->animation;
    if (source != 0)
    {
        value = (uint32)(sint32)((AnimationHeader *)(source))->frame_count;
        frame = this->frame;
        if (frame < (sint32)value - 1)
            next = frame + 1;
    }
    frame = this->frame;
    if (next < frame)
    {
        if ((this->path_flags & 0x40000) != 0)
            goto reverse;
        animation = (uint32)(sint32)(sint8)character_controller.motion.state_code;
        if (animation - 1 >= 34 && ((uint16)(character_controller.motion.form_flags >> 16) & 0xF) < 2)
        {
            this->frame = (uint32)next;
            value = 0;
            if ((sint8)character_controller.motion.prev_state_code == 0 && (sint16)character_controller.timers.idle_ticks <= 0)
            {
                value = 37;
                if ((sint16)character_controller.timers.idle_ticks < -4499)
                    character_controller.timers.idle_ticks = (uint16)-4500;
                else
                {
                    flags = random_generator.Next() & 0x7F;
                    value = flags < 50 ? 0 : (flags < 101 ? 35 : 36);
                }
            }
            goto set_animation;
        }
        animation = character_controller.motion.state_code;
        if (animation - 4 < 2)
        {
            if ((this->flags & 0x400) != 0)
                value = 6;
            else
                value = (sint8)character_controller.vitals.health <= 0 ? 1 : 0;
            goto set_animation;
        }
        if ((sint8)animation == 3)
        {
            value = 6;
            goto set_animation;
        }
        kind = (uint16)(character_controller.motion.form_flags >> 16) & 0xF;
        if ((kind == 0 || kind == 8) && (sint8)animation == 9)
        {
            value = 6;
            goto set_animation;
        }
        animation = (uint32)(sint32)(sint8)character_controller.motion.state_code;
        if (animation == 7)
        {
            if (((uint16)(character_controller.motion.form_flags >> 16) & 0xF) != 5)
                goto last_frame;
            animation = (uint32)(sint32)(sint8)character_controller.motion.state_code;
        }
        if (animation == 17 || animation == 20)
        {
        last_frame:
            source = this->animation;
            this->frame = (uint32)(uint16)((AnimationHeader *)(source))->frame_count - 1;
            goto finish;
        }
    }
    else
    {
        if ((this->path_flags & 0x40000) != 0)
            goto reverse;
        if ((sint8)character_controller.motion.state_code == 20 && (sint8)character_controller.path_transition.phase == 1)
        {
            this->frame = 0;
            goto finish;
        }
    }
    this->frame = (uint32)next;
    goto finish;
reverse:
    frame = this->frame;
    if (frame > 0)
        this->frame = (uint32)frame - 1;
    goto finish;
set_animation:
    if ((character_controller.secondary_flags & 0x80) == 0)
        character_controller.motion.state_code = value;
finish:
    this->UpdateCharacterAnimationState();
    this->UpdateCharacterForm();
    return this->ClampAnimationFrame();
}

uint32 AnimatedObject::UpdateAlternateCharacterFollow()
{
    CharacterModelRecord *model_record;
    FUNCTION_MARKER(0x80053704, "SLUS_005.78");
    AnimatedObject *parent = (AnimatedObject *)((this->follow_target));
    uint32 vector[3];
    const ScenePath *path;
    const PathDefinition *captured_definition;
    uint32 flags, x, y, z, squared, result;
    AnimationHeader *definition, *animation;
    sint32 distance, speed, cap, target_frame, frame;
    flags = this->flags;
    if ((flags & 0x10) != (parent->flags & 0x10))
    {
        path = (this->scene_path);
        this->flags = flags ^ 0x10;
        if ((((const ScenePath *)(path))->node.state_flags & 8) != 0)
        {
            path = path->adjacent[0];
        }
        captured_definition = (this->path_definition);
        this->scene_path = path;
        frame = (uint16)this->path_frame;
        this->path_frame = ((const PathDefinition *)(captured_definition))->positions.metadata6 - (uint32)frame;
    }
    if (this->movement_mode == 1)
    {
        this->behavior = 1;
        character_controller.object = this;
        parent->behavior = 33;
        parent->follow_target = this;
        {
            uint32 model_index = ((character_controller.motion.form_flags >> 18) & 0x3C) / 4;
            CharacterModelRecord *model_binding;
            if (model_index < CharacterController::ModelCount)
                model_binding = character_controller.model_slots[model_index];
            else
            {
                fprintf(stderr, "P2: Invalid character model index %u\n", model_index);
                abort();
            }
            if (model_binding == 0)
            {
                fprintf(stderr, "P2: Character model %u is not loaded\n", model_index);
                abort();
            }
            model_record = model_binding;
            character_controller.model = model_binding;
        }
        this->velocity[2] = 0;
        this->velocity[1] = 0;
        this->velocity[0] = 0;
        parent->velocity[2] = 0;
        parent->velocity[1] = 0;
        parent->velocity[0] = 0;
        character_controller.motion.state_code = 0;
        goto update_animation;
    }
    {
        uint32 model_index = 1 - ((character_controller.motion.form_flags >> 16) & 0xF);
        CharacterModelRecord *model_binding;
        if (model_index < CharacterController::ModelCount)
            model_binding = character_controller.model_slots[model_index];
        else
        {
            fprintf(stderr, "P2: Invalid character model index %u\n", model_index);
            abort();
        }
        if (model_binding == 0)
        {
            fprintf(stderr, "P2: Character model %u is not loaded\n", model_index);
            abort();
        }
        model_record = model_binding;
    }
    vector[0] = parent->position[0] - this->position[0];
    vector[1] = parent->position[1] - this->position[1];
    vector[2] = parent->position[2] - this->position[2];
    if (parent->velocity[0] == 0 && parent->velocity[1] == 0 && parent->velocity[2] == 0 && this->velocity[0] == 0 && this->velocity[1] == 0 && this->velocity[2] == 0)
        goto orient;
    x = (uint32)((sint32)vector[0] >> 6);
    y = (uint32)((sint32)vector[1] >> 6);
    z = (uint32)((sint32)vector[2] >> 6);
    squared = x * x + y * y + z * z;
    if ((sint32)squared >= 0)
        distance = (sint32)SquareRoot12((sint32)squared);
    else
        distance = (sint32)(SquareRoot12((sint32)(squared >> 2)) << 1);
    vector[1] = (uint32)distance;
    if (distance <= 49151 && (distance <= 36863 || (this->velocity[0] == 0 && this->velocity[1] == 0 && this->velocity[2] == 0)))
        goto stop;
    target_frame = parent->path_frame;
    if (((this->scene_path)) != camera_motion.path)
    {
        if (target_frame < ((sint16)((const PathDefinition *)((parent->path_definition)))->positions.metadata6 >> 1))
            target_frame = this->path_frame + 1;
        else
            target_frame = this->path_frame - 1;
    }
    if (this->path_frame >= target_frame)
        goto stop;
    x = (uint32)((sint32)parent->velocity[0] >> 6);
    y = (uint32)((sint32)parent->velocity[1] >> 6);
    z = (uint32)((sint32)parent->velocity[2] >> 6);
    squared = x * x + y * y + z * z;
    if ((sint32)squared >= 0)
        speed = (sint32)SquareRoot12((sint32)squared);
    else
        speed = (sint32)(SquareRoot12((sint32)(squared >> 2)) << 1);
    vector[2] = (uint32)speed;
    vector[0] = (uint32)speed;
    if ((sint32)vector[1] > 49152)
    {
        cap = model_record->movement.speed_limits[1];
        vector[0] = (uint32)cap;
        if (cap < speed)
            vector[0] = (uint32)speed + (uint32)(speed >> 3);
    }
    flags = this->flags;
    this->metadata126[0] = 1;
    this->flags = flags | 0x40;
    this->ComputePathMotion((sint32)vector[0], 0, NULL);
    flags = this->flags;
    this->metadata126[0] = 0;
    this->flags = flags & 0xFFFFFFBF;
    x = this->velocity[0];
    y = this->velocity[1];
    z = this->velocity[2];
    vector[0] = x;
    vector[1] = y;
    vector[2] = z;
    goto orient;
stop:
    this->velocity[2] = 0;
    this->velocity[1] = 0;
    this->velocity[0] = 0;
orient:
    this->TurnObjectFacing(vector, (sint16)character_controller.turn_step);
update_animation:
    if (this->velocity[0] != 0 || this->velocity[1] != 0 || this->velocity[2] != 0)
    {
        x = this->position[0] + this->velocity[0];
        y = this->position[1] + this->velocity[1];
        z = this->position[2] + this->velocity[2];
        this->position[0] = x;
        this->position[1] = y;
        this->position[2] = z;
        animation = this->animation;
        definition = model_record->animations[2];
    }
    else
    {
        animation = this->animation;
        definition = model_record->animations[0];
    }
    if (animation != definition)
    {
        flags = this->flags;
        this->animation = definition;
        this->frame = -1;
        this->flags = flags | 0x10000;
    }
    animation = this->animation;
    if (animation != 0)
    {
        frame = this->frame;
        if (frame < ((AnimationHeader *)(animation))->frame_count - 1)
            frame += 1;
        else
            frame = 0;
    }
    else
        frame = 0;
    this->frame = (uint32)frame;
    result = this->ClampAnimationFrame();
    return result;
}
