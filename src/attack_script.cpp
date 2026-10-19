#include "game.h"

AttackEnemyState attack_enemy = {0};

uint32 LoadAttackScripts(const uint8 *bundle)
{
    FUNCTION_MARKER(0x80021538, "SLUS_005.78");
    uint32 offset = resource_archive.Offset(resource_load_cursor.asset);
    const uint8 *bytes = bundle + offset;
    attack_enemy.script_count = xport_load_le32(bytes);
    offset = resource_archive.Offset(resource_load_cursor.asset);
    bytes = bundle + offset + 4;
    uint32 index;
    for (index = 0; index < 30; ++index)
    {
        AttackScript *script = &attack_enemy.scripts[index];
        script->state = (sint32)xport_load_le32(bytes);
        script->metadata_word = xport_load_le32(bytes + 4);
        script->animation_index = xport_load_le32(bytes + 8);
        script->marker_frame = (sint32)xport_load_le32(bytes + 12);
        script->distance_squared = (sint32)xport_load_le32(bytes + 16);
        script->approach_ticks = xport_load_le32(bytes + 20);
        script->repetitions = (sint32)xport_load_le32(bytes + 24);
        bytes += 28;
    }
    uint32 result = resource_load_cursor.asset + 1;
    resource_load_cursor.asset = result;
    return result;
}

uint32 AnimatedObject::UpdateAttackApproach(const uint32 *origin, const uint32 *target, sint32 distance, ActorModelDefinition *resource_model, const AttackScript *script)
{
    FUNCTION_MARKER(0x80021254, "SLUS_005.78");
    sint32 timer = (sint32)attack_enemy.retreat_ticks, frame, next;
    AnimationHeader *animation;
    uint32 flags;
    attack_enemy.retreat_ticks = (uint32)timer + 1;
    if (timer >= (sint32)script->approach_ticks)
    {
        attack_enemy.retreat_ticks = 0;
        return 1;
    }
    flags = this->flags | 0x40;
    this->flags = flags;
    if (distance < script->distance_squared)
    {
        this->flags = flags & 0xFFFFFFBF;
        {
            AnimationHeader *animation_value = (resource_model->animations[0]);
            uint32 animation_flags = this->flags;
            this->animation = animation_value;
            this->frame = -1;
            this->flags = animation_flags | 0x10000;
        }
        return 0;
    }
    frame = this->path_frame;
    next = (((flags & 0x10) != 0 && (sint32)target[0] < (sint32)origin[0]) || ((flags & 0x10) == 0 && (sint32)origin[0] < (sint32)target[0])) ? frame + 1 : frame - 1;
    if (frame > 0 && next < frame)
    {
        uint32 path_frame = (uint16)this->path_frame;
        uint32 path_count = ((const PathDefinition *)((this->path_definition)))->positions.metadata6;
        this->flags = (this->flags & 0xFFFFF7FF) ^ 0x10;
        this->path_frame = path_count - path_frame;
    }
    animation = (resource_model->animations[((this->flags & 0x10) != 0 ? 8 : 7)]);
    if (this->animation != animation)
    {
        AnimationHeader *animation_value = animation;
        uint32 animation_flags = this->flags;
        this->animation = animation_value;
        this->frame = -1;
        this->flags = animation_flags | 0x10000;
    }
    return 0;
}

uint32 AnimatedObject::UpdateAttackAnimation(ActorModelDefinition *resource_model, const AttackScript *script)
{
    FUNCTION_MARKER(0x800213D8, "SLUS_005.78");
    AnimationHeader *animation, *selected;
    if (attack_enemy.event_pending != 0)
    {
        this->flags = this->flags & 0xFFFFFFBF;
        {
            AnimationHeader *animation_value = (resource_model->animations[11]);
            uint32 animation_flags = this->flags;
            this->animation = animation_value;
            this->frame = -1;
            this->flags = animation_flags | 0x10000;
        }
        attack_enemy.event_pending = 0;
        scene_events.pending = attack_enemy.event_id | 0x1000;
        return 0;
    }
    animation = this->animation;
    selected = (resource_model->animations[attack_enemy.scripts[attack_enemy.script_index].animation_index]);
    if (animation == selected)
    {
        if (this->frame >= ((AnimationHeader *)(animation))->frame_count - 1)
            attack_enemy.repeat_count = attack_enemy.repeat_count + 1;
    }
    else if (this->frame == ((AnimationHeader *)(animation))->frame_count - 1)
    {
        AnimationHeader *animation_value = selected;
        uint32 animation_flags = this->flags;
        this->animation = animation_value;
        this->frame = -1;
        this->flags = animation_flags | 0x10000;
    }
    if ((sint32)attack_enemy.repeat_count < script->repetitions)
        return 0;
    this->flags = this->flags & 0xFFFFFFBF;
    selected = (resource_model->animations[10]);
    attack_enemy.repeat_count = 0;
    attack_enemy.event_pending = 1;
    this->frame = -1;
    scene_events.pending = attack_enemy.event_id | 0x2000;
    this->animation = selected;
    this->flags = this->flags | 0x10000;
    return 1;
}

uint32 AnimatedObject::UpdateAttackRepeat(const uint32 *origin, const uint32 *target, sint32 distance, ActorModelDefinition *resource_model, const AttackScript *script)
{
    FUNCTION_MARKER(0x80020F44, "SLUS_005.78");
    AnimationHeader *animation, *attack, *next_animation;
    uint32 flags;
    sint32 frame, next;
    if (script->animation_index == 6 && attack_enemy.special_pending != 0)
    {
        this->flags = this->flags & 0xFFFFFFBF;
        {
            AnimationHeader *animation_value = (resource_model->animations[5]);
            uint32 animation_flags = this->flags;
            this->animation = animation_value;
            this->frame = -1;
            this->flags = animation_flags | 0x10000;
        }
        attack_enemy.special_pending = 0;
        scene_events.pending = attack_enemy.special_event_id | 0x1000;
        return 0;
    }
    animation = this->animation;
    attack = (resource_model->animations[script->animation_index]);
    if (animation == attack)
    {
        if (this->frame >= ((AnimationHeader *)(animation))->frame_count - 1)
        {
            attack_enemy.repeat_count = attack_enemy.repeat_count + 1;
            next_animation = (resource_model->animations[((this->flags & 0x10) != 0 ? 8 : 7)]);
            if (this->animation != next_animation)
            {
                AnimationHeader *animation_value = next_animation;
                uint32 animation_flags = this->flags;
                this->animation = animation_value;
                this->frame = -1;
                this->flags = animation_flags | 0x10000;
            }
        }
    }
    else
    {
        if ((animation == (resource_model->animations[5]) || animation == (resource_model->animations[10])) && this->frame < ((AnimationHeader *)(animation))->frame_count - 1)
            return 0;
        if (distance < script->distance_squared || attack_enemy.approach_ticks == script->approach_ticks)
        {
            this->frame = -1;
            this->animation = attack;
            this->flags = (this->flags & 0xFFFEFFBF) | 0x10000;
            attack_enemy.approach_ticks = 0;
            return 0;
        }
        attack_enemy.approach_ticks = attack_enemy.approach_ticks + 1;
        flags = this->flags | 0x40;
        this->flags = flags;
        frame = this->path_frame;
        next = (((flags & 0x10) != 0 && (sint32)target[0] < (sint32)origin[0]) || ((flags & 0x10) == 0 && (sint32)origin[0] < (sint32)target[0])) ? frame + 1 : frame - 1;
        attack_enemy.next_path_frame = next;
        if (this->path_frame > 0 && next < this->path_frame)
        {
            uint32 path_frame = (uint16)this->path_frame;
            uint32 path_count = ((const PathDefinition *)((this->path_definition)))->positions.metadata6;
            this->flags = (this->flags & 0xFFFFF7FF) ^ 0x10;
            this->path_frame = path_count - path_frame;
        }
        next_animation = (resource_model->animations[((this->flags & 0x10) != 0 ? 8 : 7)]);
        if (this->animation != next_animation)
        {
            AnimationHeader *animation_value = next_animation;
            uint32 animation_flags = this->flags;
            this->animation = animation_value;
            this->frame = -1;
            this->flags = animation_flags | 0x10000;
        }
    }
    if ((sint32)attack_enemy.repeat_count < script->repetitions)
        return 0;
    attack_enemy.repeat_count = 0;
    if (script->animation_index == 6)
    {
        attack_enemy.special_pending = 1;
        this->flags = this->flags & 0xFFFFFFBF;
        {
            AnimationHeader *animation_value = (resource_model->animations[5]);
            uint32 animation_flags = this->flags;
            this->animation = animation_value;
            this->frame = -1;
            this->flags = animation_flags | 0x10000;
        }
        scene_events.pending = attack_enemy.special_event_id | 0x2000;
    }
    return 1;
}

uint32 AnimatedObject::UpdateAttackEnemy()
{
    FUNCTION_MARKER(0x80020C6C, "SLUS_005.78");
    ActorModelDefinition *bound_model;
    bound_model = ((const ActorBinding *)(((const ActorPlacement *)((const PathDefinition *)((this->path_definition))))->state != 0 ? ((const ActorPlacement *)((const PathDefinition *)((this->path_definition))))->state : 0))->model;
    uint32 result;
    uint32 origin[3], target[3], distance_output;
    const AttackScript *script;
    sint32 delta, state;
    if ((this->flags & 0x10) == 0 && this->ticks > 0)
    {
        uint32 x = this->velocity[0], y = this->velocity[1], z = this->velocity[2];
        this->velocity[0] = 0 - x;
        this->velocity[1] = 0 - y;
        this->velocity[2] = 0 - z;
    }
    this->UpdateActorStateAndTarget(origin, target, &distance_output);
    if (this->ticks <= 0)
    {
        this->frame = 40;
        this->velocity[2] = 0;
        this->velocity[0] = -4096;
        this->animation = (bound_model->animations[9]);
        this->flags = (this->flags | 0x10) & 0xFFF7FFFF;
    }
    else
    {
        delta = (sint32)(origin[0] - target[0]) >> 12;
        distance_output = (uint32)delta * (uint32)delta;
        if (this->behavior_word != 0)
            this->behavior_word = this->behavior_word - 1;
        script = &attack_enemy.scripts[attack_enemy.script_index];
        state = script->state;
        if (state == 0)
            result = this->UpdateAttackRepeat(origin, target, (sint32)distance_output, bound_model, script);
        else if (state == 1)
            result = this->UpdateAttackAnimation(bound_model, script);
        else
            result = this->UpdateAttackApproach(origin, target, (sint32)distance_output, bound_model, script);
        if (result == 1)
        {
            state = (sint32)attack_enemy.script_index;
            attack_enemy.script_index = state < (sint32)attack_enemy.script_count - 1 ? (uint32)state + 1 : 0;
        }
        script = &attack_enemy.scripts[attack_enemy.script_index];
        if (script->state != 0)
            target[0] = origin[0];
        else if (this->animation == (bound_model->animations[script->animation_index]))
        {
            if (this->frame == script->marker_frame)
                attack_enemy.target_x = target[0];
            else if (script->marker_frame < this->frame)
                target[0] = attack_enemy.target_x;
        }
    }
    target[0] = origin[0];
    result = this->UpdateActorFacingAndAnimation(target, origin, bound_model);
    return result;
}

uint32 AnimatedObject::InitializeAttackEnemy()
{
    FUNCTION_MARKER(0x80021678, "SLUS_005.78");
    uint32 result;
    {
        ObjectCallback operation = {0, &AnimatedObject::UpdateAttackEnemy};
        object_operations.behaviors[30] = operation;
    }
    this->behavior = 30;
    attack_enemy.event_id = EncodeObjectName(70, 71);
    result = EncodeObjectName(117, 112);
    attack_enemy.special_event_id = result;
    attack_enemy.script_index = 0;
    return result;
}
