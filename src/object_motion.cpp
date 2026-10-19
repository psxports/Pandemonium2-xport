#include "game.h"
#include "original_defaults.h"

static const uint32 actor_target_height_tolerance = 8192;
static const uint32 actor_target_height_limit = 61440;
static const uint32 actor_path_height_tolerance = 8192;
static const sint8 actor_path_height_enabled = 1;
static const uint8 actor_target_avoidance_mode = 0;
static const uint32 actor_action_end_margin = 2;
static const sint8 actor_turnaround_enabled = 1;
static const sint8 actor_launch_angle_selector = -1;

uint32 AnimatedObject::CalculateActorTargetMotion(uint32 distance, uint32 *distance_ret, uint32 *height_ret, uint16 *angle_ret) const
{
    FUNCTION_MARKER(0x800378C8, "SLUS_005.78");
    void *data = ((const ActorPlacement *)((const PathDefinition *)((this->path_definition))))->state;
    ActorBinding *binding = (ActorBinding *)(data);
    const AnimatedObject *owner = GetCharacterModelObject();

    union
    {
        VECTOR vector;
        uint32 words[4];
    } input, transformed;

    uint32 target_position[3], position[3], difference[3];
    uint32 index, result, angle = 0;
    const PathDefinition *definition;
    const PathTransform *path_transform;
    uint32 horizontal = distance, vertical = 0;
    for (index = 0; index < 3; ++index)
        target_position[index] = camera_motion.focus_position[index];
    definition = (this->path_definition);
    if (definition != 0 && (((const PathDefinition *)((const PathDefinition *)(definition)))->transform) != 0)
    {
        for (index = 0; index < 3; ++index)
            position[index] = this->position[index];
        for (index = 0; index < 3; ++index)
            input.words[index] = position[index];
        PushMatrix();
        path_transform = (const PathTransform *)((((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) != 0 ? (((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) : (PathTransform *)0);
        for (index = 0; index < 5; ++index)
            xport_gte_write_control(index, xport_load_le32((const uint8 *)&path_transform->matrix + index * 4));
        ApplyRotMatrixLV(&input.vector, &transformed.vector);
        PopMatrix();
        for (index = 0; index < 3; ++index)
        {
            path_transform = (const PathTransform *)((((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) != 0 ? (((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) : (PathTransform *)0);
            position[index] = transformed.words[index] + (uint32)path_transform->matrix.t[index];
            transformed.words[index] = position[index];
        }
    }
    else
    {
        for (index = 0; index < 3; ++index)
            position[index] = this->position[index];
    }
    for (index = 0; index < 3; ++index)
        difference[index] = position[index] - target_position[index];
    if ((binding->action_effect_flags & 0x400) == 0)
    {
        uint32 threshold = actor_target_height_tolerance;
        if ((sint32)difference[1] < (sint32)(0u - threshold))
            vertical = distance;
        result = (sint32)threshold < (sint32)difference[1];
        if (result != 0)
            vertical = 0u - distance;
    }
    else if ((sint32)this->position[1] < (sint32)target_position[1] && (sint32)difference[1] < (sint32)actor_target_height_limit)
    {
        result = 0xFFFF0000;
        if ((sint32)owner->velocity[1] < 0)
            vertical = 0u - distance;
        else
        {
            result = (sint32)difference[1] < -40960;
            if (result != 0)
                vertical = 0u - distance;
        }
    }
    else
    {
        result = (sint32)(0u - actor_target_height_limit) < (sint32)difference[1];
        if (result != 0)
        {
            sint32 velocity = (sint32)owner->velocity[1];
            result = 40960;
            if (velocity >= 0)
                vertical = distance;
            else
            {
                result = (sint32)difference[1] > 40960;
                if (result != 0)
                    vertical = distance;
            }
        }
    }
    if (vertical != 0)
    {
        uint32 height = difference[1];
        uint32 reach;
        sint32 wave;
        sint64 product;
        uint32 flags = binding->action_flags;
        binding->desired_pitch = 0;
        binding->action_flags = flags | 0x800;
        if ((sint32)height < 0)
            height = 0u - height;
        if ((sint32)difference[2] < (sint32)difference[0])
            reach = difference[0] + (uint32)((sint32)difference[2] / 2);
        else
            reach = (uint32)((sint32)difference[0] / 2) + difference[2];
        angle = (uint32)Atan2Angle((sint32)reach, (sint32)height);
        if ((sint32)vertical < 0)
            angle = 0u - angle;
        wave = SinQ14((sint16)angle) >> 2;
        vertical = (uint32)(((sint64)(sint32)distance * wave) >> 12);
        wave = SinQ14((sint16)(1024u - angle)) >> 2;
        product = (sint64)(sint32)distance * wave;
        horizontal = (uint32)(product >> 12);
        result = (uint32)((uint64)product >> 32) << 20;
    }
    *height_ret = vertical;
    *distance_ret = horizontal;
    *angle_ret = (uint16)angle;
    return result;
}

uint32 AnimatedObject::CalculateActorPathHeightMotion(uint32 distance, uint32 *distance_ret, uint32 *height_ret, uint16 *angle_ret) const
{
    FUNCTION_MARKER(0x80037C50, "SLUS_005.78");
    const PathDefinition *definition = (this->path_definition);
    void *data = ((const ActorPlacement *)((const PathDefinition *)(definition)))->state;
    ActorBinding *binding = (ActorBinding *)(data);
    uint32 vertical = 0, result;
    if (actor_path_height_enabled == 0)
    {
        result = binding->action_flags & 0xF7FF;
        binding->action_flags = result;
    }
    else
    {
        union
        {
            VECTOR vector;
            uint32 words[4];
        } position, input;

        uint32 floor[3], index;
        const PathTransform *path_transform;
        if (definition != 0 && (((const PathDefinition *)((const PathDefinition *)(definition)))->transform) != 0)
        {
            uint32 local_position[3];
            for (index = 0; index < 3; ++index)
                local_position[index] = this->position[index];
            for (index = 0; index < 3; ++index)
                input.words[index] = local_position[index];
            PushMatrix();
            path_transform = (const PathTransform *)((((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) != 0 ? (((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) : (PathTransform *)0);
            for (index = 0; index < 5; ++index)
                xport_gte_write_control(index, xport_load_le32((const uint8 *)&path_transform->matrix + index * 4));
            ApplyRotMatrixLV(&input.vector, &position.vector);
            PopMatrix();
            for (index = 0; index < 3; ++index)
            {
                path_transform = (const PathTransform *)((((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) != 0 ? (((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) : (PathTransform *)0);
                position.words[index] = position.words[index] + (uint32)path_transform->matrix.t[index];
            }
        }
        else
        {
            uint32 local_position[3];
            for (index = 0; index < 3; ++index)
                local_position[index] = this->position[index];
            for (index = 0; index < 3; ++index)
                position.words[index] = local_position[index];
        }
        result = ((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->FindHeightFrame(position.words, this->flags & 16, 4096000);
        if (result != (uint32)-1)
        {
            uint32 direction = this->flags & 16;
            uint32 position_y, floor_y, difference;
            result = (uint32)((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->ProjectFrame((sint32)result, position.words, floor, direction, 4096);
            if (result != (uint32)-1)
            {
                uint32 tolerance = actor_path_height_tolerance;
                floor_y = floor[1];
                position_y = position.words[1];
                difference = floor_y - position_y;
                if (((sint32)difference < (sint32)tolerance && (sint32)(0u - tolerance) < (sint32)difference) || (sint16)binding->height_direction_changes >= 5)
                {
                    result = binding->action_flags & 0xF7FF;
                    binding->action_flags = result;
                }
                else
                {
                    uint32 flags = binding->action_flags;
                    result = flags & 32;
                    if ((sint32)position_y < (sint32)floor_y)
                    {
                        vertical = distance;
                        if (result == 0)
                        {
                            result = binding->height_direction_changes;
                            binding->action_flags = flags | 32;
                            ++result;
                            binding->height_direction_changes = result;
                        }
                        else
                            binding->height_direction_changes = 0;
                    }
                    else
                    {
                        vertical = 0u - distance;
                        if (result != 0)
                        {
                            result = binding->height_direction_changes;
                            binding->action_flags = flags & 0xFFDF;
                            ++result;
                            binding->height_direction_changes = result;
                        }
                        else
                            binding->height_direction_changes = 0;
                    }
                }
            }
        }
    }
    *height_ret = vertical;
    *distance_ret = distance;
    *angle_ret = 0;
    return result;
}

uint32 AnimatedObject::UpdateActorTurnaround(ActorBinding *binding, const ActorModelDefinition *resource_model)
{
    FUNCTION_MARKER(0x80034BA8, "SLUS_005.78");

    union
    {
        VECTOR vector;
        uint32 words[4];
    } input, output;

    uint32 target_position[3];
    uint32 world_position[3];
    uint32 difference[3];
    const PathDefinition *definition;
    uint32 index;
    uint32 angle;
    sint32 state;
    target_position[0] = camera_motion.focus_position[0];
    target_position[1] = camera_motion.focus_position[1];
    target_position[2] = camera_motion.focus_position[2];
    definition = (this->path_definition);
    if (definition != 0 && (((const PathDefinition *)((const PathDefinition *)(definition)))->transform) != 0)
    {
        const PathTransform *path_transform;
        for (index = 0; index < 3; ++index)
            world_position[index] = this->position[index];
        for (index = 0; index < 3; ++index)
            input.words[index] = world_position[index];
        PushMatrix();
        path_transform = (const PathTransform *)((((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) != 0 ? (((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) : (PathTransform *)0);
        for (index = 0; index < 5; ++index)
            xport_gte_write_control(index, xport_load_le32((const uint8 *)&path_transform->matrix + index * 4));
        ApplyRotMatrixLV(&input.vector, &output.vector);
        PopMatrix();
        for (index = 0; index < 3; ++index)
        {
            path_transform = (const PathTransform *)((((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) != 0 ? (((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) : (PathTransform *)0);
            world_position[index] = output.words[index] + (uint32)path_transform->matrix.t[index];
            output.words[index] = world_position[index];
        }
    }
    else
    {
        for (index = 0; index < 3; ++index)
            world_position[index] = this->position[index];
    }
    for (index = 0; index < 3; ++index)
        difference[index] = target_position[index] - world_position[index];
    angle = Atan2Angle((sint32)(0u - difference[2]), (sint32)(0u - difference[0]));
    angle = (binding->route_heading - angle) & 4095;
    if (angle > 2048)
        angle = (4096 - angle) & 4095;
    state = (sint8)binding->route_direction_mode;
    if (state == 2)
    {
        uint32 countdown = binding->turn_ticks - 1u;
        binding->turn_ticks = countdown;
        if ((sint8)countdown <= 0)
        {
            binding->route_direction_mode = 3;
            binding->turn_ticks = 4;
        }
    }
    else if (state == 1 && angle < 512)
    {
        definition = (this->path_definition);
        uint32 flags = this->flags;
        uint16 frame = (uint16)this->path_frame;
        uint16 count = ((const PathDefinition *)(definition))->positions.metadata6;
        sint32 divisor;
        this->flags = (flags & ~0x800u) ^ 16;
        this->path_frame = (sint16)((uint32)count - frame);
        binding->route_direction_mode = 2;
        divisor = resource_model->turn_step;
        if (divisor == 0)
            trap(7);
        binding->turn_ticks = (uint32)(2048 / divisor) + 5;
    }
    else if (state == 3 && angle < 512)
    {
        uint32 countdown;
        if ((binding->reaction_flags & 16) != 0)
            return 4096;
        countdown = binding->turn_ticks - 1u;
        binding->turn_ticks = countdown;
        if ((sint8)countdown > 0)
            return 4096;
        binding->route_direction_mode = 0;
    }
    return 0;
}

uint32 AnimatedObject::UpdateActorPathMotion()
{
    const PathDefinition *parent;
    AnimationHeader *selected_animation;
    FUNCTION_MARKER(0x80033564, "SLUS_005.78");
    void *data;
    const ActorModelDefinition *speed_model;
    const ActorModelDefinition *resource_model;
    const SoundSample *previous_sound;

    union
    {
        VECTOR vector;
        uint32 words[4];
    } position, local;

    uint32 output_distance, output_height;
    uint16 output_angle;
    uint32 flags, value, result;
    const PathTransform *path_transform;
    uint32 x, y, z, initial, scene, counter, eligible, distance, height, target, acceleration;
    sint32 kind;
    parent = (this->path_definition);
    data = ((const ActorPlacement *)((const PathDefinition *)(parent)))->state;
    ActorBinding *binding = (ActorBinding *)(data);
    value = (uint16)this->ticks;
    resource_model = binding->model;
    initial = value == 0;
    if (initial != 0)
    {
        this->path_motion_scale = 15;
        kind = (resource_model->kind_override);
        if (kind == -1)
            kind = (sint8)binding->model_kind;
        if (kind < 44)
        {
            previous_sound = binding->sound;
            x = (uint32)(sint32)(sint16)binding->sound_voice;
            result = sound_effects.IsPlaying(previous_sound, x);
            if ((result << 24) != 0)
                StopSoundVoice(binding->sound_voice);
            binding->sound = 0;
            binding->sound_voice = 0xFFFF;
        }
        else
        {
            selected_animation = (binding->model->animations[1]);
            this->animation = selected_animation;
            x = (uint32)(sint32)((AnimationHeader *)(selected_animation))->frame_count;
            value = this->path_motion_scale;
            this->frame = (sint16)(0xFFFF);
            this->path_motion_scale = value + x;
        }
        flags = binding->flags;
        if ((flags & 2) != 0)
        {
            const ActorModelDefinition *reaction_model = binding->model;
            binding->flags = flags ^ 2;
            selected_animation = (reaction_model->animations[6]);
            flags = player_interaction.flags;
            this->frame = (sint16)(0xFFFF);
            player_interaction.flags = flags & 0xFFFFFFF3;
            this->velocity[0] = 4096;
            this->animation = selected_animation;
            SpawnActionEffect(this, 0, 5);
            this->path_motion_scale = (uint32)(sint32)((AnimationHeader *)(this->animation))->frame_count + 15;
        }
        kind = this->behavior;
        this->movement_mode = (sint8)(0);
        this->metadata126[0] = 0;
        this->metadata126[1] = (object_operations.behaviors[kind].member_handler == &AnimatedObject::UpdateAttackEnemy) ? 0 : 2;
        value = this->node.flags;
        this->velocity[0] = 4096;
        this->node.flags = value & 0xFE;
        goto world_position;
    }
    value = binding->reaction_flags;
    flags = binding->flags;
    binding->reaction_flags = value & 0x80;
    binding->motion_frame = (uint16)this->path_frame;
    result = flags & 8;
    if ((flags & 2) != 0)
        goto done;
    if (result != 0)
    {
        this->velocity[2] = 0;
        this->velocity[1] = 0;
        this->velocity[0] = 0;
    }
    if ((binding->flags & 0x10) == 0)
        goto movement;
world_position:
    parent = (this->path_definition);
    if (parent != 0 && ((((const PathDefinition *)(parent)))->transform) != 0)
    {
        x = this->position[0];
        y = this->position[1];
        z = this->position[2];
        local.words[0] = x;
        local.words[1] = y;
        local.words[2] = z;
        PushMatrix();
        path_transform = (const PathTransform *)((((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) != 0 ? (((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) : (PathTransform *)0);
        x = xport_load_le32((const uint8 *)&path_transform->matrix);
        y = xport_load_le32((const uint8 *)&path_transform->matrix + 4);
        xport_gte_write_control(0, x);
        xport_gte_write_control(1, y);
        x = xport_load_le32((const uint8 *)&path_transform->matrix + 8);
        y = xport_load_le32((const uint8 *)&path_transform->matrix + 12);
        z = xport_load_le32((const uint8 *)&path_transform->matrix + 16);
        xport_gte_write_control(2, x);
        xport_gte_write_control(3, y);
        xport_gte_write_control(4, z);
        ApplyRotMatrixLV(&local.vector, &position.vector);
        PopMatrix();
        path_transform = (const PathTransform *)((((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) != 0 ? (((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) : (PathTransform *)0);
        x = position.words[0];
        x += (uint32)path_transform->matrix.t[0];
        position.words[0] = x;
        path_transform = (const PathTransform *)((((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) != 0 ? (((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) : (PathTransform *)0);
        y = position.words[1];
        y += (uint32)path_transform->matrix.t[1];
        position.words[1] = y;
        path_transform = (const PathTransform *)((((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) != 0 ? (((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) : (PathTransform *)0);
        z = position.words[2];
        z += (uint32)path_transform->matrix.t[2];
        position.words[2] = z;
    }
    else
    {
        x = this->position[0];
        y = this->position[1];
        z = this->position[2];
        position.words[0] = x;
        position.words[1] = y;
        position.words[2] = z;
    }
    if (initial != 0)
    {
        x = this->world_position[0];
        x -= position.words[0];
        this->spheres[0].position[0] = x;
        y = this->world_position[1];
        y -= position.words[1];
        this->spheres[0].position[1] = y;
        z = this->world_position[2];
        z -= position.words[2];
        this->spheres[0].position[2] = z;
        result = binding->event_command;
        if (result != 0)
        {
            scene = OriginalDefaults::SceneContextIdentity;
            counter = scene_events.deferred_count;
            scene_events.deferred_count = counter + 1;
            value = binding->event_command;
            result = scene + ((uint32)(sint32)(sint8)counter << 2);
            scene_events.deferred_entries[(sint8)counter] = (value & 0xFFF) | 0x4000;
        }
        goto done;
    }
    x = position.words[1];
    value = binding->interaction_height;
    value = x - value;
    if (value == 0)
    {
        binding->flags = binding->flags & 0xEF;
        this->velocity[1] = 0;
    }
    else
    {
        value = (sint32)value >= 4097 ? 0xFFFFF000 : 0 - value;
        this->velocity[1] = value;
        flags = this->flags;
        height = this->velocity[1];
        this->flags = flags | 0x400;
        this->ComputePathMotion(0, (sint32)height, NULL);
        this->flags = this->flags & 0xFFFFFBFF;
    }
movement:
    result = binding->flags & 8;
    if (result != 0)
        goto done;
    result = this->flags & 0x02000040;
    eligible = result == 0x40;
    if (result == 0x02000040)
    {
        value = (uint32)(sint32)(resource_model->reaction_motion_mode);
        result = 1;
        if (value == 1)
        {
            result = (uint32)(sint32)this->ticks;
            if (result != 0)
                eligible = 1;
        }
    }
    if (eligible == 0)
        goto zero_velocity;
    result = (uint32)((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->positions.keys;
    if (result != 0 && (sint8)binding->route_direction_mode != 0)
    {
        this->flags = this->flags & 0xFFFFF7FF;
        result = this->UpdateActorTurnaround(binding, resource_model);
        if (result == 0)
            goto zero_velocity;
        result = this->ComputePathMotion((sint32)result, (sint16)binding->path_pitch_speed, NULL);
        goto done;
    }
    parent = (this->path_definition);
    result = (uint32)(((const PathDefinition *)(parent)))->positions.keys;
    if (result == 0)
        goto zero_velocity;
    if ((((const PathDefinition *)(parent))->flags & 0x40) == 0)
    {
        kind = (resource_model->kind_override);
        result = 19;
        if (kind == -1)
        {
            if ((sint8)binding->model_kind != 19)
                goto zero_velocity;
        }
        else if (kind != 19)
            goto zero_velocity;
    }
    this->flags = this->flags & 0xFFFFF7FF;
    kind = (resource_model->kind_override);
    if (kind == -1)
        kind = (sint8)binding->model_kind;
    if (kind == 10)
        distance = binding->motion_speed;
    else
    {
        speed_model = binding->model;
        flags = this->flags;
        value = binding->action_flags;
        if ((flags & 0x08000000) != 0)
            target = (uint32)speed_model->motion_speed[1];
        else
            target = (uint32)speed_model->motion_speed[0];
        if ((value & 0x100) != 0)
            distance = binding->action_speed;
        else
        {
            acceleration = ((uint32)speed_model->motion_accel);
            distance = target;
            if (acceleration != 0)
            {
                distance = binding->motion_speed;
                if ((sint32)distance < (sint32)target)
                {
                    distance += acceleration;
                    if ((sint32)target < (sint32)distance)
                        distance = target;
                }
            }
        }
        binding->motion_speed = distance;
    }
    result = binding->action_effect_flags & 0x200;
    binding->desired_pitch = 0;
    if (result != 0)
        this->CalculateActorTargetMotion(distance, &output_distance, &output_height, &output_angle);
    else
    {
        result = binding->action_flags & 0x800;
        if (result == 0)
            goto direct_movement;
        result = binding->flags & 8;
        if (result != 0)
            goto direct_movement;
        this->CalculateActorPathHeightMotion(distance, &output_distance, &output_height, &output_angle);
    }
    value = output_height;
    binding->path_pitch_speed = value;
    result = (uint32)(sint32)(resource_model->pitch_limit);
    distance = output_distance;
    if (result != 0)
    {
        result = output_angle;
        binding->desired_pitch = result;
    }
    else
        binding->desired_pitch = 0;
    if (distance == 0)
        goto zero_velocity;
    this->flags = this->flags | 0x400;
    this->ComputePathMotion((sint32)distance, (sint16)binding->path_pitch_speed, NULL);
    result = this->flags & 0xFFFFFBFF;
    this->flags = result;
    goto done;
direct_movement:
    if (distance == 0)
        goto zero_velocity;
    result = this->ComputePathMotion((sint32)distance, (sint16)binding->path_pitch_speed, NULL);
    goto done;
zero_velocity:
    this->velocity[2] = 0;
    this->velocity[1] = 0;
    this->velocity[0] = 0;
done:
    return result;
}

uint32 AnimatedObject::ReleaseActorInteraction()
{
    const PathDefinition *parent;
    FUNCTION_MARKER(0x8004483C, "SLUS_005.78");

    union
    {
        VECTOR vector;
        uint32 words[4];
    } position;

    uint32 global_flags, flags, x, y, z, value, result;
    const PathTransform *path_transform;
    global_flags = player_interaction.flags;
    flags = this->flags;
    this->ticks = (sint16)(2);
    result = 0xFFFFFFFE;
    player_interaction.flags = global_flags & result;
    this->metadata126[0] = 0;
    if ((flags & 0x2000) != 0)
        return result;
    parent = (this->path_definition);
    if (parent != 0 && ((((const PathDefinition *)(parent)))->transform) != 0)
    {
        x = this->position[0];
        y = this->position[1];
        z = this->position[2];
        position.words[0] = x;
        position.words[1] = y;
        position.words[2] = z;
        PushMatrix();
        path_transform = (const PathTransform *)((((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) != 0 ? (((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) : (PathTransform *)0);
        x = xport_load_le32((const uint8 *)&path_transform->matrix);
        y = xport_load_le32((const uint8 *)&path_transform->matrix + 4);
        xport_gte_write_control(0, x);
        xport_gte_write_control(1, y);
        x = xport_load_le32((const uint8 *)&path_transform->matrix + 8);
        y = xport_load_le32((const uint8 *)&path_transform->matrix + 12);
        z = xport_load_le32((const uint8 *)&path_transform->matrix + 16);
        xport_gte_write_control(2, x);
        xport_gte_write_control(3, y);
        xport_gte_write_control(4, z);
        ApplyRotMatrixLV(&position.vector, (VECTOR *)this->position);
        PopMatrix();
        path_transform = (const PathTransform *)((((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) != 0 ? (((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) : (PathTransform *)0);
        x = this->position[0];
        value = (uint32)path_transform->matrix.t[0];
        parent = (this->path_definition);
        this->position[0] = x + value;
        path_transform = (const PathTransform *)(((((const PathDefinition *)(parent)))->transform) != 0 ? ((((const PathDefinition *)(parent)))->transform) : (PathTransform *)0);
        y = this->position[1];
        value = (uint32)path_transform->matrix.t[1];
        parent = (this->path_definition);
        this->position[1] = y + value;
        path_transform = (const PathTransform *)(((((const PathDefinition *)(parent)))->transform) != 0 ? ((((const PathDefinition *)(parent)))->transform) : (PathTransform *)0);
        z = this->position[2];
        value = (uint32)path_transform->matrix.t[2];
        this->position[2] = z + value;
    }
    else
    {
        x = this->position[0];
        y = this->position[1];
        z = this->position[2];
        this->position[0] = x;
        this->position[1] = y;
        this->position[2] = z;
    }
    result = this->flags | 0x2000;
    this->flags = result;
    return result;
}

uint32 AnimatedObject::ApplyInteractionDirectionalInput(uint32 *speed)
{
    const PathDefinition *parent;
    FUNCTION_MARKER(0x80044990, "SLUS_005.78");

    union
    {
        VECTOR vector;
        uint32 words[4];
    } tangent;

    uint32 kind, flags, buttons, mask, result;
    uint32 value, frame, x, y, z, multiplier, scaled_x;
    sint32 timer, limit, delta;
    FindPlayerObject(0);
    kind = this->state;
    timer = this->ticks;
    limit = (sint16)GetModelGroupLifetime(kind);
    if (timer < limit - 15 && (player_interaction.flags & 0x10) == 0)
    {
        result = ReleaseActorInteraction();
        return result;
    }
    flags = this->flags;
    if ((flags & 0x20200) == 0x200 || (sint8)character_controller.motion.state_code == 4 || (character_controller.secondary_flags & 0x240) != 0 || (character_controller.flags & 0x04000000) != 0)
    {
        result = ReleaseActorInteraction();
        return result;
    }
    buttons = gameplay_input.buttons;
    mask = (*control_bindings.active_scheme)[0];
    if ((buttons & mask) == 0)
    {
        result = ReleaseActorInteraction();
        return result;
    }
    value = player_interaction.flags;
    result = buttons & 0x8000;
    if ((value & 0x10) == 0)
    {
        return result;
    }
    delta = 0;
    if ((buttons & 0x8000) != 0)
        delta = (flags & 0x10) != 0 ? -819 : 819;
    else if ((buttons & 0x2000) != 0)
        delta = (flags & 0x10) != 0 ? 819 : -819;
    value = *speed + (uint32)delta;
    *speed = value;
    if ((sint32)value >= 6145)
        *speed = 6144;
    if (delta != 0)
    {
        value = *speed;
        if ((sint32)value < 0)
        {
            *speed = 0 - value;
            flags = this->flags;
            parent = (this->path_definition);
            this->flags = flags ^ 0x10;
            timer = (sint16)((const PathDefinition *)(parent))->positions.metadata6;
            frame = (uint32)timer - (uint32)(sint32)this->path_frame;
            if ((sint32)frame >= timer)
                frame = (uint32)timer - 1;
            else if ((sint32)frame < 0)
                frame = 0;
            this->path_frame = (sint16)(frame);
        }
        flags = this->flags;
        if ((flags & 0x2000) != 0)
        {
            parent = (this->path_definition);
            (((const PathDefinition *)(parent)))->SampleTangent(&tangent.vector, (sint32)(flags & 0x10));
            x = tangent.words[0];
            tangent.words[1] = 0;
            if (x + 26754 > 0xD103 || tangent.words[2] + 26754 > 0xD103)
            {
                do
                {
                    x = (uint32)((sint32)tangent.words[0] >> 1);
                    y = (uint32)((sint32)tangent.words[1] >> 1);
                    z = (uint32)((sint32)tangent.words[2] >> 1);
                    tangent.words[0] = x;
                    tangent.words[1] = y;
                    tangent.words[2] = z;
                } while (x + 26754 > 0xD103 || y + 26754 > 0xD103 || z + 26754 > 0xD103);
            }
            VectorNormal(&tangent.vector, &tangent.vector);
            x = tangent.words[0];
            multiplier = *speed;
            scaled_x = (uint32)(((sint64)(sint32)x * (sint32)multiplier) >> 12);
            tangent.words[0] = scaled_x;
            multiplier = *speed;
            y = tangent.words[1];
            tangent.words[1] = (uint32)(((sint64)(sint32)y * (sint32)multiplier) >> 12);
            multiplier = *speed;
            z = tangent.words[2];
            tangent.words[2] = (uint32)(((sint64)(sint32)z * (sint32)multiplier) >> 12);
            this->velocity[0] = scaled_x;
            z = tangent.words[2];
            this->velocity[2] = z;
        }
    }
    buttons = gameplay_input.buttons;
    delta = 0;
    if ((buttons & 0x1000) != 0)
        delta = 819;
    else if ((buttons & 0x4000) != 0)
        delta = -819;
    value = this->velocity[1] + (uint32)delta;
    this->velocity[1] = value;
    result = (sint32)value >= 6145 ? 6144 : (uint32)-6144;
    if ((sint32)value >= 6145 || (sint32)value < -6144)
        this->velocity[1] = result;
    return result;
}

uint32 AnimatedObject::ApplyActorDirectionalInput(uint32 *speed)
{
    FUNCTION_MARKER(0x80044CFC, "SLUS_005.78");
    uint32 flags, input, value;
    sint32 delta = 0;
    FindPlayerObject(0);
    if (((uint16)(character_controller.motion.form_flags >> 16) & 15) != 0)
        return 0x20000;
    flags = this->flags;
    if ((flags & 0x20200) == 512 || (sint8)character_controller.motion.state_code == 4)
        return 4;
    if ((path_routing.flags & 16) == 0)
        return 0;
    input = gameplay_input.buttons;
    if ((input & 0x8000) != 0)
    {
        delta = (flags & 16) != 0 ? -819 : 819;
        gameplay_input.buttons = input & ~0x8000u;
    }
    else if ((input & 0x2000) != 0)
    {
        delta = (flags & 16) != 0 ? 819 : -819;
        gameplay_input.buttons = input & ~0x2000u;
    }
    value = *speed + (uint32)delta;
    *speed = value;
    if ((sint32)value >= 12289)
        *speed = 12288;
    if (delta != 0)
    {
        if ((sint32)*speed < 0)
        {
            const PathDefinition *definition;
            sint32 count, frame;
            *speed = 0u - *speed;
            flags = this->flags;
            definition = (this->path_definition);
            this->flags = flags ^ 16;
            count = (sint16)((const PathDefinition *)(definition))->positions.metadata6;
            frame = count - this->path_frame;
            if (frame >= count)
                frame = count - 1;
            else if (frame < 0)
                frame = 0;
            this->path_frame = (sint16)(frame);
        }
        flags = this->flags;
        if ((flags & 0x2000) != 0)
        {
            union
            {
                VECTOR vector;
                uint32 words[4];
            } tangent;

            uint32 index, x;
            ((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->SampleTangent(&tangent.vector, flags & 16);
            tangent.words[1] = 0;
            if (tangent.words[0] + 26754u > 0xD103 || tangent.words[2] + 26754u > 0xD103)
            {
                do
                {
                    for (index = 0; index < 3; ++index)
                        tangent.words[index] = (uint32)((sint32)tangent.words[index] >> 1);
                } while (tangent.words[0] + 26754u > 0xD103 || tangent.words[1] + 26754u > 0xD103 || tangent.words[2] + 26754u > 0xD103);
            }
            VectorNormal(&tangent.vector, &tangent.vector);
            for (index = 0; index < 3; ++index)
                tangent.words[index] = (uint32)(((sint64)(sint32)tangent.words[index] * (sint32)*speed) >> 12);
            x = tangent.words[0];
            this->velocity[0] = x;
            this->velocity[2] = tangent.words[2];
        }
    }
    input = gameplay_input.buttons;
    delta = (input & 0x1000) != 0 ? 819 : ((input & 0x4000) != 0 ? -819 : 0);
    value = this->velocity[1] + (uint32)delta;
    this->velocity[1] = value;
    if ((sint32)value >= 12289)
    {
        this->velocity[1] = 12288;
        return 12288;
    }
    if ((sint32)value < -12288)
        this->velocity[1] = (uint32)-12288;
    return (uint32)-12288;
}

uint32 AnimatedObject::UpdateActorVelocity()
{
    FUNCTION_MARKER(0x8002FB94, "SLUS_005.78");
    uint32 result, speed, state, magnitude, x, y, z, squared;
    if (this->state == 19 && (player_interaction.flags & 3) != 3)
    {
        result = (uint16)this->ticks + 1;
        this->ticks = result;
        return result;
    }
    result = this->ComputeObjectFacing(this->velocity, 0);
    this->target_yaw = result;
    this->orientation[2] = (result + 2048) & 0xFFF;
    x = this->velocity[0];
    y = this->velocity[1];
    z = this->velocity[2];
    if (this->flags & 0x400)
        y = 0;
    if (x || z)
    {
        x = (uint32)((sint32)x >> 6);
        y = (uint32)((sint32)y >> 6);
        z = (uint32)((sint32)z >> 6);
        squared = x * x + y * y + z * z;
        if ((sint32)squared >= 0)
            result = SquareRoot12((sint32)squared);
        else
            result = SquareRoot12((sint32)(squared >> 2)) << 1;
        speed = result;
    }
    else
        speed = 0;
    state = this->state;
    if (state == 19)
        this->ApplyInteractionDirectionalInput(&speed);
    else if (!state)
        this->ApplyActorDirectionalInput(&speed);
    result = this->flags & 0x2000;
    if (!result)
    {
        magnitude = speed;
        if (magnitude || this->velocity[1])
            result = this->ComputePathMotion((sint32)magnitude, (sint32)this->velocity[1], NULL);
        else
        {
            this->velocity[2] = 0;
            this->velocity[1] = 0;
            this->velocity[0] = 0;
        }
    }
    return result;
}

ActorBinding *AnimatedObject::ResetActorAction()
{
    FUNCTION_MARKER(0x800378A0, "SLUS_005.78");
    void *data = ((const ActorPlacement *)((const PathDefinition *)((this->path_definition))))->state;
    ActorBinding *binding = (ActorBinding *)(data);
    binding->action_flags = 0;
    binding->action_effect_flags = 0;
    binding->action_index = 0xFF;
    binding->action_reset_half = 0;
    binding->route_mode = 0;
    return data != 0 ? binding : 0;
}

uint32 AnimatedObject::InitializeActorAction(ActorBinding *binding)
{
    const PathDefinition *parent;
    FUNCTION_MARKER(0x800352F4, "SLUS_005.78");
    const ActorModelDefinition *bound_model;
    uint32 result, x, y, z;
    uint32 pos0[3], pos1[3], delta[3];
    bound_model = ((const ActorBinding *)(((const ActorPlacement *)((const PathDefinition *)((this->path_definition))))->state != 0 ? ((const ActorPlacement *)((const PathDefinition *)((this->path_definition))))->state : 0))->model;
    yaw = 0;
    if ((bound_model->actions[0].distance_threshold) == 0)
    {
        binding->action_index = 0xFE;
        return 0xFFFFFFFE;
    }
    ResetActorAction();
    result = (uint32)(sint32)(bound_model->turn_motion_mode);
    if (result != 0)
        return result;
    parent = (this->path_definition);
    result = (uint32)(((const PathDefinition *)(parent)))->positions.keys;
    if (result == 0)
        return result;
    (((const PathDefinition *)(parent)))->SampleActorPosition(0, pos0, 64);
    parent = (this->path_definition);
    (((const PathDefinition *)(parent)))->SampleActorPosition(1, pos1, 64);
    x = pos1[0];
    x -= pos0[0];
    y = pos1[1];
    y -= pos0[1];
    z = pos1[2];
    z -= pos0[2];
    delta[0] = x;
    delta[1] = y;
    delta[2] = z;
    result = TurnObjectFacing(delta, bound_model->turn_step);
    return result;
}

sint32 AnimatedObject::ClassifyActorTargetDirection(const uint32 *pos, const uint32 *target) const
{
    FUNCTION_MARKER(0x80037F78, "SLUS_005.78");

    union
    {
        VECTOR vector;
        uint32 words[4];
    } sample;

    const PathDefinition *parent;
    uint32 point_x, flags, x, z, first_angle, second_angle;
    sint32 count, frame, next_frame, result;
    parent = (this->path_definition);
    if ((((const PathDefinition *)(parent)))->positions.keys == 0)
        goto no_direction;
    x = pos[0] - target[0] + 4096;
    if (x < 0x2001)
    {
        z = pos[2] - target[2] + 4096;
        if (z < 0x2001)
            goto no_direction;
    }
    parent = (this->path_definition);
    count = (sint16)((const PathDefinition *)(parent))->positions.metadata6;
    frame = this->path_frame;
    next_frame = frame + count / 8;
    if (next_frame == frame || next_frame > count - 1)
        next_frame = count - 1;
    flags = this->flags;
    parent = (this->path_definition);
    (((const PathDefinition *)(parent)))->SamplePosition(next_frame, sample.words, flags & 0x10);
    z = sample.words[2];
    x = pos[2];
    point_x = sample.words[0];
    flags = pos[0];
    first_angle = Atan2Angle((sint32)(z - x), (sint32)(point_x - flags));
    z = target[2];
    x = pos[2];
    point_x = target[0];
    flags = pos[0];
    second_angle = Atan2Angle((sint32)(z - x), (sint32)(point_x - flags));
    result = ((first_angle - second_angle + 1024) & 0xFFF) < 0x801 ? 1 : -1;
    return result;
no_direction:
    return 0;
}

uint32 AnimatedObject::CanActorTurnTowardTarget(const uint32 *position, const uint32 *target, const uint32 *distance, const ActorBinding *binding, const ActorModelDefinition *model, uint8 allowed) const
{
    FUNCTION_MARKER(0x80035494, "SLUS_005.78");
    sint32 range;
    uint32 angle;
    if (actor_turnaround_enabled == 0 || (model->route_action) == 0)
        return 0;
    if ((binding->route_mode) != 0 && allowed == 0)
        return 0;
    range = (sint32)((uint32)(sint32)(model->turnaround_range) << 5);
    if (range < (sint32)distance[0])
        return 0;
    angle = Atan2Angle((sint32)(target[2] - position[2]), (sint32)(target[0] - position[0]));
    return (((uint16)this->yaw - angle + 1024) & 4095) < 2049;
}

uint32 AnimatedObject::UpdateActorAction(const uint32 *pos, uint32 *target, uint32 *distance)
{
    const PathDefinition *parent;
    FUNCTION_MARKER(0x8003557C, "SLUS_005.78");
    void *definition;
    const ActorModelDefinition *actions_model;
    const ActorActionDefinition *action, *candidate;
    const AnimatedObject *owner;
    uint8 candidate_flags[6];
    uint32 launch_angles[3];
    uint32 value, flags, mode, state, x, y, z;
    AnimationHeader *animation;
    const PathTransform *path_transform;
    uint32 *counter;
    uint32 scan, trigger, threshold, blocked, chosen;

    union
    {
        VECTOR vector;
        uint32 words[4];
    } target_saved, spawn_pos, path_delta, input1, cached0, cached1;

    uint32 *input, *cached_input;
    AnimatedObject *spawned, *saved_spawn;
    uint16 motion_kind, pitch_angle;
    uint8 in_range, event_trigger_seen, projectile_count;
    sint32 target_direction;
    uint32 kind, speed, turn, delay, attack, count, iteration;
    uint32 length, magnitude, angle, sine, cosine, result;
    sint32 index, limit, direction, frame, next_frame, parameter, signed_value, sign_x, sign_z;
    definition = ((const ActorPlacement *)((const PathDefinition *)((this->path_definition))))->state;
    ActorBinding *binding = (ActorBinding *)(definition);
    result = 0xFFFFFFFE;
    if ((sint8)binding->action_index == -2)
        return result;
    if ((binding->flags & 2) != 0)
        return (uint32)this->ResetActorAction();
    actions_model = binding->model;
    owner = GetCharacterModelObject();
    if ((sint8)binding->turn_ticks != 0)
        return (uint32)this->ResetActorAction();
    chosen = 0;
restart_action:
    motion_kind = 0;
restart_mode:
    for (scan = 0; scan < 6; ++scan)
        candidate_flags[scan] = 0;
    mode = binding->route_mode;
    index = -1;
    limit = 6;
    blocked = 0;
    in_range = 0;
    event_trigger_seen = 0;
    if (mode == 0)
    {
        signed_value = (sint8)binding->action_index;
        if (signed_value != -1)
        {
            action = actions_model->actions + signed_value;
            value = action->priority_mode;
            if (value == 1)
                limit = (uint16)signed_value;
            else
            {
                if (value == 2)
                {
                    animation = (actions_model->animations[(uint32)action->animation]);
                    if (this->animation == animation && action->priority_frame >= this->frame)
                        goto retain_action_priority;
                    value = action->priority_mode;
                }
                if (value == 3 && this->animation == (actions_model->animations[(uint32)action->animation]) && this->frame >= action->priority_frame)
                {
                retain_action_priority:
                    limit = (uint16)(sint8)binding->action_index;
                }
                else
                    blocked = 1;
            }
        }
        scan = 0;
        candidate = actions_model->actions;
        counter = binding->trigger_ticks;
        while (candidate->distance_threshold != 0)
        {
            if ((sint8)index != -1)
                break;
            threshold = candidate->distance_threshold;
            if ((sint32)distance[0] < (sint32)threshold || (sint32)threshold < 0)
            {
                trigger = candidate->trigger;
                value = 0;
                in_range = 1;
                if (trigger == 7)
                    candidate_flags[scan] = 1;
                if ((sint32)scan < limit && blocked == 0)
                {
                    switch (candidate->trigger)
                    {
                        case 0:
                            value = 1;
                            break;
                        case 1:
                            if ((owner->flags & 0x400) != 0)
                                value = 1;
                            break;
                        case 2:
                            flags = gameplay_input.buttons;
                            if ((flags & (*control_bindings.active_scheme)[0]) != 0)
                                value = 1;
                            break;
                        case 3:
                            if ((binding->reaction_flags & 8) != 0)
                            {
                                flags = binding->action_flags;
                                if ((flags & 0x10) == 0)
                                {
                                    value = 1;
                                    binding->action_flags = flags | 0x10;
                                }
                            }
                            break;
                        case 4:
                            flags = binding->pending_event;
                            event_trigger_seen = 1;
                            if (flags != 0xFFFFFFFF)
                            {
                                value = 1;
                                binding->pending_event = 0xFFFFFFFF;
                            }
                            break;
                        case 5:
                            if (candidate->trigger_ticks >= this->ticks)
                                value = 1;
                            break;
                        case 6:
                            if (binding->range_ticks >= (uint32)candidate->trigger_ticks)
                            {
                                value = 1;
                                binding->range_ticks = 0;
                            }
                            break;
                        case 7:
                            if (*counter >= (uint32)candidate->trigger_ticks)
                            {
                                value = 1;
                                *counter = 0;
                            }
                            break;
                    }
                    if (value != 0)
                    {
                        index = (sint32)scan;
                        action = candidate;
                        binding->action_index = scan;
                        binding->action_phase = 0;
                    }
                }
            }
            ++candidate;
            ++counter;
            scan += 1;
        }
        goto selected_action;
    }
    direction = this->ClassifyActorTargetDirection(pos, target);
    target_direction = (uint32)direction;
    if (binding->route_mode == 2 && (sint8)binding->action_index < 0)
        this->ResetActorAction();
    mode = binding->route_mode;
    if (mode == 1)
    {
        this->ResetActorAction();
        if ((actions_model->route_action) != 0)
        {
            binding->route_mode = 2;
            binding->action_phase = 0;
            binding->action_index = ((uint8)actions_model->route_action) - 1;
        }
        else
        {
            animation = (binding->model->animations[0]);
            this->frame = (sint16)(0xFFFF);
            this->animation = animation;
            if (target_direction != -1)
                binding->route_mode = 4;
            else
            {
                binding->route_mode = 3;
                parent = (this->path_definition);
                flags = this->flags;
                x = (uint16)this->path_frame;
                y = ((const PathDefinition *)(parent))->positions.metadata6;
                this->flags = (flags & 0xFFFFF7FF) ^ 0x10;
                this->path_frame = (sint16)(y - x);
                parameter = (actions_model->turn_motion_mode);
                motion_kind = 1;
                if (parameter != 0)
                    binding->turn_delay = 0;
                else
                {
                    parameter = actions_model->turn_step;
                    if (parameter == 0)
                        xport_mips_break(7);
                    delay = (uint32)(2048 / parameter) - 2;
                    if ((delay & 0x80) != 0)
                        delay = 0;
                    binding->turn_delay = delay;
                }
            }
        }
    }
    else if (mode == 3)
    {
        value = (binding->turn_delay - 1) & 0xFF;
        binding->turn_delay = value;
        if (value == 0)
            binding->route_mode = 4;
    }
    mode = binding->route_mode;
    if (mode == 4)
    {
        threshold = (uint32)(actions_model->turnaround_range) << 5;
        if (target_direction != -1 && (sint32)distance[0] < (sint32)threshold)
        {
            binding->action_flags = binding->action_flags | 0x1100;
            this->flags = (this->flags & 0xFFFFF7BF) | 0x40;
            binding->action_speed = 0;
            goto selected_action;
        }
        this->ResetActorAction();
        motion_kind = 0;
        goto restart_mode;
    }
    if (mode == 2)
        action = actions_model->actions + (sint8)binding->action_index;
selected_action:
    index = (sint8)binding->action_index;
    binding->action_effect_flags = 0;
    if (index == -1)
        goto update_counters;
    action = actions_model->actions + index;
    direction = this->ClassifyActorTargetDirection(pos, target);
    target_direction = (uint32)direction;
    if (binding->action_phase == 0)
    {
        flags = binding->action_flags;
        binding->action_phase = 1;
        binding->action_flags = flags & 0xFFFE;
        parameter = action->motion;
        turn = ((parameter == 3 || parameter == 6 || parameter == 8) && target_direction == -1);
        if (turn == 0)
        {
            value = (uint16)action->motion;
            turn = ((value - 4 < 2 || (sint16)value == 7) && target_direction == 1);
        }
        if (turn != 0)
        {
            parent = (this->path_definition);
            flags = this->flags;
            x = (uint16)this->path_frame;
            y = ((const PathDefinition *)(parent))->positions.metadata6;
            this->flags = (flags & 0xFFFFF7FF) ^ 0x10;
            this->path_frame = (sint16)(y - x);
            parameter = (actions_model->turn_motion_mode);
            motion_kind = 1;
            if (parameter == 0)
            {
                parameter = actions_model->turn_step;
                if (parameter == 0)
                    xport_mips_break(7);
                delay = (uint32)(2048 / parameter) - 2;
                if ((delay & 0x80) != 0)
                    delay = 0;
                binding->turn_delay = delay;
            }
            else
                binding->turn_delay = 0;
        }
        else
            binding->turn_delay = 0;
        if (action->projectile_kind == -1)
            binding->projectile_phase = 0;
        else
        {
            flags = binding->action_flags;
            binding->projectile_phase = 1;
            binding->next_projectile_frame = 0;
            binding->action_flags = flags & 0xFFF3;
        }
    }
    state = binding->action_phase;
    if (state == 1)
    {
        value = binding->turn_delay;
        if (value != 0)
        {
            binding->turn_delay = value - 1;
            motion_kind = 1;
            value = binding->turn_delay;
        }
        if (value == 0)
        {
            animation = (actions_model->animations[(uint32)action->animation]);
            flags = this->flags;
            this->frame = (sint16)(0xFFFF);
            this->flags = flags | 0x10000;
            this->animation = animation;
            binding->action_phase = 4;
        }
        state = binding->action_phase;
    }
    if (state == 2)
    {
        animation = this->animation;
        if (animation != (actions_model->animations[(uint32)action->animation]))
            binding->action_phase = 7;
        else if (this->frame != ((AnimationHeader *)(animation))->frame_count - 1)
            binding->action_phase = 4;
        motion_kind = 8;
    }
    else if (state == 3)
    {
        if (this->animation != (actions_model->animations[(uint32)action->animation]))
            binding->action_phase = 7;
        if (this->frame == ((AnimationHeader *)(this->animation))->frame_count - 1)
            binding->action_phase = action->hold_ticks != 0 ? 5 : 7;
        else
        {
            value = (binding->turn_delay - 1) & 0xFF;
            binding->turn_delay = value;
            if (value == 0)
                motion_kind = 1;
            else
                binding->action_phase = 4;
        }
    }
    if (binding->action_phase == 4)
    {
        flags = binding->action_flags;
        motion_kind = ((flags & 1) != 0 ? (uint16)action->next_motion : (uint16)action->motion);
        if (this->animation != (actions_model->animations[(uint32)action->animation]))
            binding->action_phase = 7;
        else
        {
            flags = binding->action_flags;
            if ((flags & 1) == 0 && action->switch_frame != 0 && this->frame >= action->switch_frame)
            {
                binding->action_flags = flags | 1;
                parameter = action->next_motion;
                turn = ((parameter == 3 || parameter == 8) && target_direction != 1);
                if (turn == 0)
                {
                    if (parameter == 3 || parameter == 8)
                        parameter = action->next_motion;
                    turn = ((parameter == 4 || parameter == 5) && target_direction == 1) || (parameter == 7 && action->motion == 6);
                }
                if (turn == 0)
                    motion_kind = (uint16)action->next_motion;
                else
                    goto reverse_action;
            }
            else if (this->frame == ((AnimationHeader *)(this->animation))->frame_count - 1)
                binding->action_phase = action->hold_ticks != 0 ? 5 : 7;
            else if (motion_kind == 8 && target_direction != 1)
            {
            reverse_action:
                binding->action_phase = 3;
                parent = (this->path_definition);
                flags = this->flags;
                x = (uint16)this->path_frame;
                y = ((const PathDefinition *)(parent))->positions.metadata6;
                this->flags = (flags & 0xFFFFF7FF) ^ 0x10;
                this->path_frame = (sint16)(y - x);
                parameter = (actions_model->turn_motion_mode);
                motion_kind = 1;
                if (parameter != 0)
                    binding->turn_delay = 0;
                else
                {
                    parameter = actions_model->turn_step;
                    if (parameter == 0)
                        xport_mips_break(7);
                    delay = (uint32)(2048 / parameter) - 2;
                    if ((delay & 0x80) != 0)
                        delay = 0;
                    binding->turn_delay = delay;
                }
            }
        }
    }
    if (binding->action_phase == 5)
    {
        binding->action_phase = 6;
        animation = (actions_model->animations[(uint32)action->hold_animation]);
        flags = this->flags;
        this->frame = (sint16)(0xFFFF);
        this->animation = animation;
        this->flags = flags | 0x10000;
        this->behavior_word = (uint32)action->hold_ticks;
        parameter = action->hold_motion;
        turn = ((parameter == 3 || parameter == 8) && target_direction != 1);
        if (turn == 0)
        {
            if (parameter == 3 || parameter == 8)
                parameter = action->hold_motion;
            turn = ((parameter == 4 || parameter == 5) && target_direction == 1);
        }
        if (turn != 0)
        {
            parent = (this->path_definition);
            flags = this->flags;
            x = (uint16)this->path_frame;
            y = ((const PathDefinition *)(parent))->positions.metadata6;
            this->flags = (flags & 0xFFFFF7FF) ^ 0x10;
            this->path_frame = (sint16)(y - x);
            parameter = (actions_model->turn_motion_mode);
            motion_kind = 1;
            if (parameter != 0)
                binding->turn_delay = 0;
            else
            {
                parameter = actions_model->turn_step;
                if (parameter == 0)
                    xport_mips_break(7);
                delay = (uint32)(2048 / parameter) - 2;
                if ((delay & 0x80) != 0)
                    delay = 0;
                binding->turn_delay = delay;
            }
        }
    }
    if (binding->action_phase == 6)
    {
        value = this->behavior_word - 1;
        this->behavior_word = value;
        if (value == 0)
            binding->action_phase = 7;
        else
        {
            value = binding->turn_delay;
            if (value != 0)
            {
                binding->turn_delay = value - 1;
                motion_kind = 1;
                value = binding->turn_delay;
            }
            if (value == 0)
            {
                if (action->hold_speed == 0)
                    motion_kind = 1;
                else if (action->hold_motion == 4 && target_direction == 1)
                {
                    parent = (this->path_definition);
                    flags = this->flags;
                    x = (uint16)this->path_frame;
                    y = ((const PathDefinition *)(parent))->positions.metadata6;
                    this->flags = (flags & 0xFFFFF7FF) ^ 0x10;
                    this->path_frame = (sint16)(y - x);
                    parameter = (actions_model->turn_motion_mode);
                    motion_kind = 1;
                    if (parameter != 0)
                        binding->turn_delay = 0;
                    else
                    {
                        parameter = actions_model->turn_step;
                        if (parameter == 0)
                            xport_mips_break(7);
                        delay = (uint32)(2048 / parameter) - 2;
                        if ((delay & 0x80) != 0)
                            delay = 0;
                        binding->turn_delay = delay;
                    }
                }
                else
                    motion_kind = (uint16)action->hold_motion;
            }
        }
    }
    if (binding->action_phase == 7)
    {
        animation = (actions_model->animations[0]);
        flags = this->flags;
        this->frame = (sint16)(0xFFFF);
        this->animation = animation;
        this->flags = flags | 0x10000;
        binding->action_index = 0xFF;
        motion_kind = 9;
        if (binding->route_mode != 0)
            binding->route_mode = 0;
    }
    if (binding->action_phase == 6)
        speed = action->hold_speed;
    else if ((binding->action_flags & 1) != 0)
        speed = action->next_speed;
    else
        speed = action->speed;
    kind = motion_kind;
    if (kind == 0)
        goto free_motion;
    if (kind == 1)
    {
        binding->action_flags = binding->action_flags & 0xFEFF;
        this->flags = this->flags & 0xFFFFFFBF;
    }
    else if (kind >= 2 && kind <= 8)
    {
        parent = (this->path_definition);
        threshold = (uint32)(sint16)((const PathDefinition *)(parent))->positions.metadata6 - actor_action_end_margin;
        if (this->path_frame >= (sint32)threshold)
        {
            value = 0;
            if (actor_turnaround_enabled != 0 && (actions_model->route_action) != 0 && binding->route_mode == 0 && (sint32)((uint32)(actions_model->turnaround_range) << 5) >= (sint32)distance[0])
            {
                x = target[0] - pos[0];
                target_saved.words[0] = x;
                y = target[1] - pos[1];
                target_saved.words[1] = y;
                z = target[2] - pos[2];
                target_saved.words[2] = z;
                angle = Atan2Angle((sint32)z, (sint32)x);
                value = (((uint16)this->yaw - angle + 1024) & 0xFFF) < 0x801;
            }
            if (value != 0)
                binding->route_mode = 1;
            else
            {
                binding->action_flags = binding->action_flags | 0x100;
                this->flags = (this->flags & 0xFFFFF7BF) | 0x40;
                binding->action_speed = kind <= 4 ? speed : 0;
            }
        }
        else
        {
            binding->action_flags = binding->action_flags | 0x100;
            this->flags = (this->flags & 0xFFFFF7BF) | 0x40;
            binding->action_speed = speed;
        }
    }
    else
    {
        parent = (this->path_definition);
        if (this->path_frame < (sint16)((const PathDefinition *)(parent))->positions.metadata6 - 1)
        {
        free_motion:
            binding->action_flags = binding->action_flags & 0xFEFF;
            this->flags = (this->flags & 0xFFFFF7BF) | 0x40;
        }
        else
        {
            binding->action_flags = binding->action_flags & 0xFEFF;
            this->flags = this->flags & 0xFFFFFFBF;
        }
    }
    if (binding->action_phase == 7)
    {
        value = chosen;
        chosen = 1;
        if (value == 0)
            goto restart_action;
    }
    if (this->animation == (actions_model->animations[(uint32)action->animation]))
    {
        state = binding->projectile_phase;
        if (state == 2)
        {
            if (action->projectile_frame - 1 >= this->frame)
            {
                flags = binding->action_flags;
                binding->projectile_phase = 1;
                binding->next_projectile_frame = 0;
                binding->action_flags = flags & 0xFFF3;
            }
            state = binding->projectile_phase;
        }
        if (state == 1)
        {
            flags = binding->action_flags;
            trigger = 0;
            if ((flags & 4) == 0)
            {
                if (this->frame >= action->projectile_frame - 1)
                {
                    binding->action_flags = flags | 4;
                    x = (uint8)action->projectile_frame;
                    y = (uint8)action->projectile_interval;
                    binding->next_projectile_frame = x + y;
                    trigger = 1;
                }
            }
            else
            {
                value = action->projectile_end_frame;
                if (value != 0 && (sint32)(value - 1) < this->frame)
                    binding->projectile_phase = 2;
                else
                {
                    value = binding->next_projectile_frame;
                    if (this->frame >= (sint32)value - 1)
                    {
                        binding->next_projectile_frame = value + (uint8)action->projectile_interval;
                        trigger = 1;
                    }
                }
            }
            if (trigger == 1)
            {
                if (target_direction != 1)
                {
                    x = target[0];
                    y = target[1];
                    z = target[2];
                    target_saved.words[0] = x;
                    target_saved.words[1] = y;
                    target_saved.words[2] = z;
                    x = 0 - target[0];
                    z = 0 - target[2];
                    target[0] = x;
                    target[2] = z;
                }
                if (action->projectile_interval == 0)
                    binding->projectile_phase = 2;
                this->state = (uint32)action->projectile_kind;
                this->flags = this->flags | 0x100;
                spawned = object_operations.behaviors[6].factory_handler(this);
                flags = this->flags;
                saved_spawn = spawned;
                this->flags = flags & 0xFFFFFEFF;
                x = spawned->position[0];
                y = spawned->position[1];
                z = spawned->position[2];
                spawn_pos.words[0] = x;
                spawn_pos.words[1] = y;
                spawn_pos.words[2] = z;
                spawned = saved_spawn;
                value = action->projectile_height;
                value = spawned->position[1] + value;
                spawned->position[1] = value;
                spawn_pos.words[1] = value;
                parameter = this->yaw;
                speed = action->projectile_offset;
                value = 0 - (uint32)(SinQ14((sint16)parameter) >> 2);
                value = (uint32)(((sint64)(sint32)speed * (sint32)value) >> 12);
                spawned->position[0] = spawned->position[0] + value;
                parameter = (sint16)(1024 - (uint16)this->yaw);
                speed = action->projectile_offset;
                spawned = saved_spawn;
                value = 0 - (uint32)(SinQ14((sint16)parameter) >> 2);
                value = (uint32)(((sint64)(sint32)speed * (sint32)value) >> 12);
                spawned->position[2] = spawned->position[2] + value;
                parameter = action->aim_mode;
                if (parameter < 0 || parameter > 3)
                    goto attack_finished;
                if (parameter > 0 && parameter < 3)
                {
                    FindPlayerObject(0);
                    launch_angles[0] = action->spread_angle;
                    parameter = action->aim_mode;
                    pitch_angle = 0;
                    if (parameter == 1)
                        projectile_count = 1;
                    else
                    {
                        launch_angles[1] = 0;
                        launch_angles[2] = 4096 - action->spread_angle;
                        x = (uint32)((sint32)binding->sampled_velocity[0] >> 6);
                        y = (uint32)((sint32)binding->sampled_velocity[1] >> 6);
                        z = (uint32)((sint32)binding->sampled_velocity[2] >> 6);
                        value = x * x + y * y + z * z;
                        projectile_count = 3;
                        if ((sint32)value < 0)
                            length = SquareRoot12((sint32)(value >> 2)) << 1;
                        else
                            length = SquareRoot12((sint32)value);
                        if ((this->flags & 0x10) == 0)
                            length = 0 - length;
                        angle = (uint32)AsinAngle((sint32)binding->sampled_velocity[1], (sint32)length);
                        pitch_angle = angle;
                    }
                    iteration = 0;
                    if (projectile_count == 0)
                        goto attack_finished;
                    attack = 1;
                    input = input1.words;
                    cached_input = cached1.words;
                }
                else
                {
                    attack = parameter == 3 ? 3 : 0;
                    input = attack == 0 ? path_delta.words : input1.words;
                    cached_input = cached0.words;
                    iteration = 0;
                }
            next_projectile:
                if (attack == 1 && (sint32)iteration > 0)
                {
                    this->flags = this->flags | 0x100;
                    spawned = object_operations.behaviors[6].factory_handler(this);
                    flags = this->flags;
                    saved_spawn = spawned;
                    this->flags = flags & 0xFFFFFEFF;
                    value = spawned->position[1];
                    spawned->position[1] = value + action->projectile_height;
                }
                spawned = saved_spawn;
                parent = (spawned->path_definition);
                if (parent != 0 && ((((const PathDefinition *)(parent)))->transform) != 0)
                {
                    x = spawned->position[0];
                    y = spawned->position[1];
                    z = spawned->position[2];
                    input[0] = x;
                    input[1] = y;
                    input[2] = z;
                    PushMatrix();
                    spawned = saved_spawn;
                    path_transform = (const PathTransform *)((((const PathDefinition *)((const PathDefinition *)((spawned->path_definition))))->transform) != 0 ? (((const PathDefinition *)((const PathDefinition *)((spawned->path_definition))))->transform) : (PathTransform *)0);
                    x = xport_load_le32((const uint8 *)&path_transform->matrix);
                    y = xport_load_le32((const uint8 *)&path_transform->matrix + 4);
                    xport_gte_write_control(0, x);
                    xport_gte_write_control(1, y);
                    x = xport_load_le32((const uint8 *)&path_transform->matrix + 8);
                    y = xport_load_le32((const uint8 *)&path_transform->matrix + 12);
                    z = xport_load_le32((const uint8 *)&path_transform->matrix + 16);
                    xport_gte_write_control(2, x);
                    xport_gte_write_control(3, y);
                    xport_gte_write_control(4, z);
                    ApplyRotMatrixLV((const VECTOR *)input, (VECTOR *)spawned->position);
                    PopMatrix();
                    spawned = saved_spawn;
                    parent = (spawned->path_definition);
                    path_transform = (const PathTransform *)(((((const PathDefinition *)(parent)))->transform) != 0 ? ((((const PathDefinition *)(parent)))->transform) : (PathTransform *)0);
                    value = spawned->position[0] + (uint32)path_transform->matrix.t[0];
                    parent = (spawned->path_definition);
                    spawned->position[0] = value;
                    path_transform = (const PathTransform *)(((((const PathDefinition *)(parent)))->transform) != 0 ? ((((const PathDefinition *)(parent)))->transform) : (PathTransform *)0);
                    value = spawned->position[1] + (uint32)path_transform->matrix.t[1];
                    parent = (spawned->path_definition);
                    spawned->position[1] = value;
                    path_transform = (const PathTransform *)(((((const PathDefinition *)(parent)))->transform) != 0 ? ((((const PathDefinition *)(parent)))->transform) : (PathTransform *)0);
                    value = (uint32)path_transform->matrix.t[2];
                    spawned->position[2] = spawned->position[2] + value;
                }
                else
                {
                    x = spawned->position[0];
                    y = spawned->position[1];
                    z = spawned->position[2];
                    spawned->position[0] = x;
                    spawned->position[1] = y;
                    spawned->position[2] = z;
                }
                if (attack == 3)
                {
                    spawned = saved_spawn;
                    flags = spawned->flags;
                    spawned->movement_mode = 0;
                    spawned = saved_spawn;
                    spawned->flags = flags | 0x2000;
                    spawned->velocity[1] = 0;
                    goto attack_finished;
                }
                spawned = saved_spawn;
                parent = (spawned->path_definition);
                if (parent != 0 && ((((const PathDefinition *)(parent)))->transform) != 0)
                {
                    x = spawn_pos.words[0];
                    y = spawn_pos.words[1];
                    z = spawn_pos.words[2];
                    cached_input[0] = x;
                    cached_input[1] = y;
                    cached_input[2] = z;
                    PushMatrix();
                    spawned = saved_spawn;
                    path_transform = (const PathTransform *)((((const PathDefinition *)((const PathDefinition *)((spawned->path_definition))))->transform) != 0 ? (((const PathDefinition *)((const PathDefinition *)((spawned->path_definition))))->transform) : (PathTransform *)0);
                    x = xport_load_le32((const uint8 *)&path_transform->matrix);
                    y = xport_load_le32((const uint8 *)&path_transform->matrix + 4);
                    xport_gte_write_control(0, x);
                    xport_gte_write_control(1, y);
                    x = xport_load_le32((const uint8 *)&path_transform->matrix + 8);
                    y = xport_load_le32((const uint8 *)&path_transform->matrix + 12);
                    z = xport_load_le32((const uint8 *)&path_transform->matrix + 16);
                    xport_gte_write_control(2, x);
                    xport_gte_write_control(3, y);
                    xport_gte_write_control(4, z);
                    ApplyRotMatrixLV((const VECTOR *)cached_input, &spawn_pos.vector);
                    PopMatrix();
                    spawned = saved_spawn;
                    path_transform = (const PathTransform *)((((const PathDefinition *)((const PathDefinition *)((spawned->path_definition))))->transform) != 0 ? (((const PathDefinition *)((const PathDefinition *)((spawned->path_definition))))->transform) : (PathTransform *)0);
                    spawn_pos.words[0] = spawn_pos.words[0] + (uint32)path_transform->matrix.t[0];
                    path_transform = (const PathTransform *)((((const PathDefinition *)((const PathDefinition *)((spawned->path_definition))))->transform) != 0 ? (((const PathDefinition *)((const PathDefinition *)((spawned->path_definition))))->transform) : (PathTransform *)0);
                    spawn_pos.words[1] = spawn_pos.words[1] + (uint32)path_transform->matrix.t[1];
                    path_transform = (const PathTransform *)((((const PathDefinition *)((const PathDefinition *)((spawned->path_definition))))->transform) != 0 ? (((const PathDefinition *)((const PathDefinition *)((spawned->path_definition))))->transform) : (PathTransform *)0);
                    spawn_pos.words[2] = spawn_pos.words[2] + (uint32)path_transform->matrix.t[2];
                }
                spawned = saved_spawn;
                flags = spawned->flags;
                spawned->movement_mode = 0;
                spawned->flags = flags | 0x2000;
                if (attack == 0)
                    FindPlayerObject(0);
                spawned = saved_spawn;
                spawned->velocity[0] = target[0] - spawn_pos.words[0];
                spawned->velocity[1] = target[1] - spawn_pos.words[1];
                z = target[2] - spawn_pos.words[2];
                spawned->velocity[2] = z;
                if (attack == 0)
                {
                    magnitude = spawned->velocity[1];
                    if ((sint32)magnitude < 0)
                        magnitude = 0 - magnitude;
                }
                x = spawned->velocity[0];
                value = (uint32)(((sint64)(sint32)x * (sint32)x) >> 12) + (uint32)(((sint64)(sint32)z * (sint32)z) >> 12);
                if ((sint32)value < 0)
                {
                    x = spawned->velocity[0];
                    value = (uint32)(((sint64)(sint32)x * (sint32)x) >> 12) + (uint32)(((sint64)(sint32)z * (sint32)z) >> 12);
                    length = SquareRoot12((sint32)(value >> 2)) << 1;
                }
                else
                {
                    x = spawned->velocity[0];
                    value = (uint32)(((sint64)(sint32)x * (sint32)x) >> 12) + (uint32)(((sint64)(sint32)z * (sint32)z) >> 12);
                    length = SquareRoot12((sint32)value);
                }
                if (attack == 0)
                {
                    value = (uint32)((sint32)length >> 1);
                    distance[0] = value;
                    if ((sint32)magnitude >= (sint32)value)
                    {
                        if (magnitude == 0)
                            xport_mips_break(7);
                        signed_value = 0x40000000 / (sint32)magnitude;
                        value = (uint32)(((sint64)(sint32)value * signed_value) >> 18);
                        spawned = saved_spawn;
                        x = spawned->velocity[1];
                        fixed_math_result = value;
                        spawned->velocity[1] = (uint32)(((sint64)(sint32)x * (sint32)value) >> 12);
                    }
                    goto attack_finished;
                }
                sine = (uint32)SinQ14((sint16)launch_angles[0]);
                spawned = saved_spawn;
                signed_value = (sint16)((sint32)sine >> 2);
                spawned->velocity[1] = (uint32)(((sint64)(sint32)length * signed_value) >> 12);
                parent = (this->path_definition);
                const AnimationTrack *rotation_track = 0;
                if (parent != 0 && (((const PathDefinition *)(parent))->flags & 8) != 0)
                {
                    rotation_track = ((const ActorBinding *)(((const ActorPlacement *)((const PathDefinition *)(parent)))->state != 0 ? ((const ActorPlacement *)((const PathDefinition *)(parent)))->state : 0))->rotation_track;
                    sign_x = 1;
                }
                if (rotation_track != 0)
                {
                    sign_z = 1;
                    x = spawned->position[1];
                    y = action->projectile_height;
                    z = spawned->velocity[0];
                    spawned->position[1] = x - y;
                    if ((sint32)z < 0)
                    {
                        sign_x = -1;
                        spawned->velocity[0] = 0 - z;
                    }
                    spawned = saved_spawn;
                    z = spawned->velocity[2];
                    if ((sint32)z < 0)
                    {
                        sign_z = -1;
                        spawned->velocity[2] = 0 - z;
                    }
                    signed_value = actor_launch_angle_selector;
                    if (signed_value == -1)
                        signed_value = (sint32)iteration;
                    angle = (launch_angles[signed_value] + pitch_angle) & 0xFFF;
                    sine = (uint32)SinQ14((sint16)angle);
                    cosine = (uint32)SinQ14((sint16)(1024 - angle));
                    sine = (uint32)((sint32)sine >> 2);
                    cosine = (uint32)((sint32)cosine >> 2);
                    spawned = saved_spawn;
                    x = spawned->velocity[0];
                    z = spawned->velocity[2];
                    value = (uint32)(((sint64)(sint32)(x + z) * (sint32)cosine) >> 12);
                    x = spawned->velocity[0];
                    spawned->velocity[1] = value;
                    value = (uint32)(((sint64)(sint32)x * (sint32)sine) >> 12);
                    z = spawned->velocity[2];
                    spawned->velocity[0] = value;
                    spawned->velocity[2] = (uint32)(((sint64)(sint32)z * (sint32)sine) >> 12);
                    if (sign_x == -1)
                        spawned->velocity[0] = 0 - spawned->velocity[0];
                    if (sign_z == -1)
                    {
                        spawned = saved_spawn;
                        spawned->velocity[2] = 0 - spawned->velocity[2];
                    }
                }
                else if (iteration == 1)
                {
                    spawned = saved_spawn;
                    spawned->velocity[0] = 0;
                    spawned->velocity[2] = 0;
                }
                else if (iteration == 2)
                {
                    spawned = saved_spawn;
                    x = spawned->velocity[0];
                    z = spawned->velocity[2];
                    spawned->velocity[0] = 0 - x;
                    spawned->velocity[2] = 0 - z;
                }
                count = projectile_count;
                iteration += 1;
                if ((sint32)iteration < (sint32)count)
                    goto next_projectile;
            attack_finished:
                if (target_direction != 1)
                {
                    x = target_saved.words[0];
                    y = target_saved.words[1];
                    z = target_saved.words[2];
                    target[0] = x;
                    target[1] = y;
                    target[2] = z;
                }
                spawned = saved_spawn;
                x = spawned->velocity[0];
                y = spawned->velocity[1];
                z = spawned->velocity[2];
                binding->projectile_velocity[0] = x;
                binding->projectile_velocity[1] = y;
                binding->projectile_velocity[2] = z;
            }
        }
    }
    if (action->path_motion_mode == 1 && ((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->positions.keys != 0)
    {
        binding->action_effect_flags = binding->action_effect_flags | 2;
        frame = this->path_frame;
        if ((this->flags & 0x10) != 0)
        {
            parameter = (sint16)((const PathDefinition *)((this->path_definition)))->positions.metadata6;
            next_frame = frame + 1;
            if (frame == parameter - 1)
            {
                frame = parameter - 2;
                next_frame = frame + 1;
            }
        }
        else
        {
            next_frame = frame - 1;
            if (frame == 0)
            {
                frame = 1;
                next_frame = 0;
            }
        }
        ((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->SampleActorPosition(frame, target_saved.words, 64);
        ((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->SampleActorPosition(next_frame, spawn_pos.words, 64);
        x = spawn_pos.words[0];
        value = target_saved.words[0];
        y = spawn_pos.words[1];
        flags = target_saved.words[1];
        z = spawn_pos.words[2];
        threshold = target_saved.words[2];
        x -= value;
        y -= flags;
        z -= threshold;
        path_delta.words[0] = x;
        path_delta.words[1] = y;
        path_delta.words[2] = z;
        x = (uint32)((sint32)x >> 6);
        y = (uint32)((sint32)y >> 6);
        z = (uint32)((sint32)z >> 6);
        value = x * x + y * y + z * z;
        if ((sint32)value < 0)
            length = SquareRoot12((sint32)(value >> 2)) << 1;
        else
            length = SquareRoot12((sint32)value);
        angle = (uint32)AsinAngle((sint32)path_delta.words[1], (sint32)length);
        this->orientation[1] = (sint16)((0 - angle) & 0xFFF);
    }
    if ((sint8)binding->action_index != -1)
    {
        if (binding->action_phase >= 5)
            value = (uint8)action->flags & 4;
        else if ((binding->action_flags & 1) != 0)
            value = (uint8)action->flags & 2;
        else
            value = (uint8)action->flags & 1;
        if (value != 0)
        {
            flags = binding->action_effect_flags;
            kind = motion_kind;
            binding->action_effect_flags = flags | 0x200;
            if (kind == 4 || kind == 7 || kind == 5)
            {
                binding->action_effect_flags = binding->action_effect_flags | 0x400;
                goto check_motion_flag;
            }
        }
        else
        {
        check_motion_flag:
            if (motion_kind == 5)
                binding->action_effect_flags = binding->action_effect_flags | 0x800;
        }
        if (event_trigger_seen == 1)
            binding->action_effect_flags = binding->action_effect_flags | 0x100;
        if ((action->flags & 0x10) != 0)
            binding->action_effect_flags = binding->action_effect_flags | 0x1000;
    }
update_counters:
    if (((actions_model->action_counter_flags) & 1) != 0)
    {
        if (in_range == 1)
        {
            value = binding->range_ticks;
            flags = binding->action_flags & 0xFFFD;
            binding->range_ticks = value + 1;
            binding->action_flags = flags;
        }
        else if (binding->range_ticks != 0)
        {
            flags = binding->action_flags;
            if ((flags & 2) != 0)
                binding->range_ticks = 0;
            else
                binding->action_flags = flags | 2;
        }
        for (scan = 0; scan < 6; ++scan)
        {
            if ((sint8)candidate_flags[scan] != 0)
                binding->trigger_ticks[scan] = binding->trigger_ticks[scan] + 1;
            else
                binding->trigger_ticks[scan] = 0;
        }
    }
    flags = binding->action_flags;
    result = 16;
    if ((flags & 0x10) != 0)
    {
        result = flags & 0xFFEF;
        if (this->behavior != 16)
            binding->action_flags = result;
    }
    return result;
}

sint32 AnimatedObject::ClassifyActorPathDirection(const uint32 *pos, const uint32 *target) const
{
    FUNCTION_MARKER(0x80034958, "SLUS_005.78");
    const PathDefinition *definition = (this->path_definition);

    union
    {
        VECTOR vector;
        uint32 words[4];
    } sample;

    sint32 count, frame, lookahead;
    sint16 path_angle, target_angle;
    if (((const PathDefinition *)((const PathDefinition *)(definition)))->positions.keys == 0)
        return 0;
    if (pos[0] - target[0] + 4096 < 8193 && pos[2] - target[2] + 4096 < 8193)
        return 0;
    count = (sint16)((const PathDefinition *)(definition))->positions.metadata6;
    frame = this->path_frame;
    lookahead = frame + count / 8;
    if (lookahead == frame || lookahead > count - 1)
        lookahead = count - 1;
    ((const PathDefinition *)((const PathDefinition *)(definition)))->SamplePosition(lookahead, sample.words, this->flags & 0x10);
    path_angle = (sint16)Atan2Angle((sint32)(sample.words[2] - pos[2]), (sint32)(sample.words[0] - pos[0]));
    target_angle = (sint16)Atan2Angle((sint32)(target[2] - pos[2]), (sint32)(target[0] - pos[0]));
    return (((uint32)((sint32)path_angle - target_angle + 1024) & 4095) < 2049) ? 1 : -1;
}

static uint32 CaptureActorRouteDirection(uint16 *angle, uint32 *direction, const AnimatedObject *object)
{
    FUNCTION_MARKER(0x80035154, "SLUS_005.78");
    uint32 x = object->velocity[0];
    uint32 y = object->velocity[1];
    uint32 z = object->velocity[2];
    uint32 result;
    direction[0] = x;
    direction[1] = y;
    direction[2] = z;
    result = Atan2Angle((sint32)object->velocity[2], (sint32)object->velocity[0]);
    *angle = result;
    return result;
}

uint32 AnimatedObject::UpdateActorShrinkEffect()
{
    FUNCTION_MARKER(0x80030C2C, "SLUS_005.78");
    const ActorModelDefinition *bound_model;
    uint32 state = this->state;
    sint32 countdown = (sint32)state >> 16;
    sint32 scale = state & 0xFFFF;
    SpriteRecord *target;
    if (countdown != 0)
    {
        --countdown;
        if (countdown == 0)
        {
            uint32 flags = this->flags;
            const SoundBankStorage *sounds = sound_resources.common_table;
            const SoundSample *sound = 0;
            this->flags = flags | 0x20000000;
            if (sounds != 0 && (sint32)sounds->header.count >= 14)
                sound = sounds->samples + 13;
            sound_effects.Queue(sound, 0, 0, sound_settings.effects_volume, 0, 3);
            this->flags = this->flags & ~0x10000000u;
        }
    }
    else
    {
        uint32 flags = this->flags;
        if ((flags & 0x10000000) != 0)
        {
            if (scale >= 1366)
            {
                scale -= 182;
                if (scale < 1366)
                {
                    scale = 1365;
                    countdown = 150;
                }
            }
        }
        else if (scale < 4096)
        {
            scale += 182;
            if (scale >= 4096)
            {
                scale = 4096;
                this->flags = flags & ~0x20000000u;
            }
        }
    }
    target = (this->sprite);
    if (target != 0)
    {
        bound_model = ((const ActorBinding *)(((const ActorPlacement *)((const PathDefinition *)((this->path_definition))))->state != 0 ? ((const ActorPlacement *)((const PathDefinition *)((this->path_definition))))->state : 0))->model;
        uint32 product = (uint32)(((sint64)(bound_model->sprite_extent[0]) * scale) >> 12);
        target->extent[0] = (sint16)((sint32)product >> 5);
        product = (uint32)(((sint64)(bound_model->sprite_extent[1]) * scale) >> 12);
        target = (SpriteRecord *)(((SpriteRecord *)(this->sprite)));
        target->extent[1] = (sint16)((sint32)product >> 5);
    }
    state = (uint32)scale | ((uint32)countdown << 16);
    this->state = state;
    return state;
}

uint32 AnimatedObject::UpdateActorStateAndTarget(uint32 *pos, uint32 *target, uint32 *distance)
{
    const PathDefinition *parent;
    const PathTransform *captured_path_transform;
    FUNCTION_MARKER(0x80030DCC, "SLUS_005.78");
    void *definition;
    const ActorModelDefinition *motion_model;
    const ActorModelDefinition *animation_model;
    const ActorModelDefinition *resource_model;

    union
    {
        VECTOR vector;
        uint32 words[4];
    } delta, input;

    union
    {
        uint32 words[5];
        uint16 halves[10];
    } matrix_local;

    ActorBinding *interaction_target;
    const AnimatedObject *player;
    uint32 flags, value, x, y, z, amount, repetitions;
    const PathTransform *path_transform;
    uint32 heading, candidate, angle, pitch;
    AnimationHeader *animation, *alternate;
    const SoundBankStorage *sound_table;
    const SoundSample *sound;
    uint32 scene, squared, result, index, next;
    uint32 finish_early = 0;
    sint32 kind, frame, limit, timer;
    definition = ((const ActorPlacement *)((const PathDefinition *)((this->path_definition))))->state;
    ActorBinding *binding = (ActorBinding *)(definition);
    flags = binding->reaction_flags;
    resource_model = binding->model;
    if ((flags & 0x20000) != 0)
    {
        SpawnActionEffect(this, 0, 8);
        this->ticks = (sint16)(0);
    }
    if ((binding->reaction_flags & 4) != 0)
    {
        limit = binding->respawn_ticks;
        if ((sint32)this->path_motion_scale >= limit)
        {
            SceneNode *event_node = (SceneNode *)(binding->event_node);
            event_node->state_flags |= 8;
        }
    }
    if ((binding->reaction_flags & 0x28) != 0)
    {
        kind = (resource_model->kind_override);
        if (kind == -1)
            kind = (sint8)binding->model_kind;
        if (kind < 44)
        {
            sound = binding->sound;
            value = (uint32)(sint16)binding->sound_voice;
            if ((sound_effects.IsPlaying(sound, value) & 0xFF) != 0)
                StopSoundVoice(binding->sound_voice);
            binding->sound = 0;
            binding->sound_voice = 0xFFFF;
        }
        character_controller.support.release_pending = 0;
        kind = (resource_model->kind_override);
        if (kind == -1)
            kind = (sint8)binding->model_kind;
        if (kind != 30)
        {
            kind = (resource_model->kind_override);
            if (kind == -1)
                kind = (sint8)binding->model_kind;
            if (kind != 31)
                goto consume_timer;
        }
        flags = binding->reaction_flags & 0xFFFFFFDF;
        binding->reaction_flags = flags;
        frame = this->frame;
        if (frame == 0 || frame == ((AnimationHeader *)(this->animation))->frame_count - 1)
        {
            binding->reaction_flags = flags | 0x20;
            character_controller.support.release_pending = 1;
        }
    consume_timer:
        flags = binding->reaction_flags;
        amount = flags >> 9;
        if ((flags & 0x20) != 0 || this->ticks < (sint32)amount)
            amount = (uint32)this->ticks;
        if (amount != 0)
        {
            this->ticks = (sint16)((uint16)this->ticks - amount);
            repetitions = (uint32)(resource_model->health_restore_count);
            if (repetitions != 0)
            {
                --repetitions;
                do
                {
                    if (this->ticks == 0)
                    {
                        SpawnActionEffect(this, binding, 2);
                        RestoreCharacterHealthPoint();
                    }
                    --repetitions;
                } while (repetitions != 0xFFFFFFFF);
            }
            if (this->ticks == 0)
            {
                finish_early = 1;
                goto sample_target;
            }
        }
        kind = (resource_model->kind_override);
        if (kind == -1)
            kind = (sint8)binding->model_kind;
        if (kind != 19)
        {
            parent = (this->path_definition);
            ((PathDefinition *)((const PathDefinition *)(parent)))->flags |= 0x40;
        }
    }
    if (this->path_motion_scale != 0 && (this->velocity[0] != 0 || this->velocity[1] != 0 || this->velocity[2] != 0))
    {
        heading = this->ComputeObjectFacing((const uint32 *)this->velocity, 0);
        if ((sint16)heading == 4096)
            candidate = 0;
        else
        {
            candidate = (uint16)this->yaw - heading;
            if ((sint16)candidate < 0)
                candidate = 0 - candidate;
            if ((((uint32)(sint32)(sint16)candidate - 1024) & 0xFFF) < 2048 && (this->path_definition) != 0 && binding->rotation_track != 0)
            {
                z = this->velocity[2];
                x = this->velocity[0];
                angle = Atan2Angle((sint32)(0 - z), (sint32)(0 - x));
                if (((((uint32)(sint32)(sint16)heading - (uint32)(sint32)(sint16)angle) + 128) & 0xFFF) >= 257)
                    candidate = 0;
            }
        }
        motion_model = binding->model;
        if (motion_model->turn_step < (sint16)candidate && (motion_model->turn_motion_mode) == 0)
        {
            value = binding->motion_frame;
            binding->motion_speed = 0;
            this->path_frame = (sint16)(value);
            goto after_motion;
        }
        parent = (this->path_definition);
        if (parent != 0 && (path_transform = ((captured_path_transform = ((((const PathDefinition *)(parent)))->transform)) != 0 ? (const PathTransform *)((PathTransform *)(captured_path_transform)) : 0)) != 0)
        {
            matrix_local.halves[0] = path_transform->matrix.m[0][0];
            for (index = 1; index < 9; ++index)
            {
                path_transform = (const PathTransform *)((((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) != 0 ? (((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) : (PathTransform *)0);
                matrix_local.halves[index] = path_transform->matrix.m[index % 3][index / 3];
            }
            x = this->velocity[0];
            y = this->velocity[1];
            z = this->velocity[2];
            input.words[0] = x;
            input.words[1] = y;
            input.words[2] = z;
            PushMatrix();
            x = matrix_local.words[0];
            y = matrix_local.words[1];
            xport_gte_write_control(0, x);
            xport_gte_write_control(1, y);
            x = matrix_local.words[2];
            y = matrix_local.words[3];
            z = matrix_local.halves[8];
            xport_gte_write_control(2, x);
            xport_gte_write_control(3, y);
            xport_gte_write_control(4, z);
            ApplyRotMatrixLV(&input.vector, &delta.vector);
            PopMatrix();
        }
        else
        {
            x = this->velocity[0];
            y = this->velocity[1];
            z = this->velocity[2];
            delta.words[0] = x;
            delta.words[1] = y;
            delta.words[2] = z;
        }
        x = (uint32)(((sint64)(sint32)delta.words[0] * (sint32)this->path_motion_scale) >> 12);
        value = delta.words[1];
        delta.words[0] = x;
        y = (uint32)(((sint64)(sint32)value * (sint32)this->path_motion_scale) >> 12);
        value = delta.words[2];
        delta.words[1] = y;
        z = (uint32)(((sint64)(sint32)value * (sint32)this->path_motion_scale) >> 12);
        delta.words[2] = z;
        kind = this->behavior;
        if ((object_operations.behaviors[kind].member_handler == &AnimatedObject::UpdateAttackEnemy) && (this->flags & 0x10) == 0)
        {
            delta.words[0] = 0 - x;
            delta.words[1] = 0 - y;
            delta.words[2] = 0 - z;
        }
        x = this->position[0];
        this->position[0] = x + delta.words[0];
        y = this->position[1];
        this->position[1] = y + delta.words[1];
        z = this->position[2];
        this->position[2] = z + delta.words[2];
    }
    else
        this->path_frame = (sint16)(binding->motion_frame);
after_motion:
    flags = binding->reaction_flags;
    if ((flags & 1) != 0)
    {
        kind = this->behavior;
        this->behavior = (sint8)(17);
        this->state = ((uint32)kind << 8) | 0x96;
        goto after_reaction;
    }
    if ((flags & 8) != 0)
    {
        kind = (resource_model->kind_override);
        if (kind == -1)
        {
            if ((sint8)binding->model_kind >= 44)
                goto large_kind_reaction;
            kind = (resource_model->kind_override);
        }
        else if (kind >= 44)
            goto large_kind_reaction;
        if (kind == -1)
            kind = (sint8)binding->model_kind;
        if (kind == 10)
            goto blink;
        if ((resource_model->reaction_motion_mode) != 1 || this->ticks == 0)
        {
            flags = this->flags;
            angle = (uint32)this->yaw;
            kind = this->behavior;
            this->flags = flags | 0x2000000;
            binding->reaction_state = (angle << 16) | ((uint32)kind << 8) | 10;
            angle = (uint16)this->yaw;
            this->metadata126[0] = 0;
            this->behavior = (sint8)(16);
            this->target_yaw = (sint16)(angle << 3);
        }
        goto blink;
    large_kind_reaction:
        if ((binding->reaction_flags & 8) == 0)
            goto after_reaction;
        kind = (resource_model->kind_override);
        if (kind == -1)
            kind = (sint8)binding->model_kind;
        if (kind >= 47)
            goto after_reaction;
    blink:
        binding->blink_active = 1;
        binding->blink_ticks = 0;
        this->flags = this->flags | 0x80000;
    }
after_reaction:
    if ((this->flags & 0x30000000) != 0)
        this->UpdateActorShrinkEffect();
    flags = binding->reaction_flags;
    if ((flags & 0x10) != 0)
    {
        if ((flags & 0x80) != 0)
            goto active_route;
        if (((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->positions.keys != 0 && (this->velocity[0] != 0 || this->velocity[1] != 0 || this->velocity[2] != 0))
        {
            if ((sint8)binding->route_direction_mode == 0)
            {
                binding->reaction_flags = binding->reaction_flags | 0x80;
                CaptureActorRouteDirection(&binding->route_heading, binding->route_direction, this);
                binding->route_direction_mode = 1;
                if ((this->flags & 0x10) != 0)
                    value = binding->flags | 0x20;
                else
                    value = binding->flags & 0xDF;
                binding->flags = value;
            }
            goto active_route;
        }
        parent = (this->path_definition);
        flags = this->flags;
        frame = (uint16)this->path_frame;
        value = ((const PathDefinition *)(parent))->positions.metadata6;
        this->flags = (flags & 0xFFFFF7FF) ^ 0x10;
        this->path_frame = (sint16)(value - frame);
        value = binding->reaction_flags | 0x80;
    }
    else
        value = flags & 0xFFFFFF7F;
    binding->reaction_flags = value;
active_route:
    if ((this->flags & 0x800) != 0)
    {
        flags = this->flags;
        parent = (this->path_definition);
        this->path_frame = (sint16)(0);
        flags &= 0xFFFFF7FF;
        this->flags = flags;
        if ((((const PathDefinition *)(parent))->positions.flags & 2) == 0)
            this->flags = flags ^ 0x10;
    }
    if ((binding->flags & 2) == 0)
        goto animation_done;
    if ((player_interaction.flags & 4) == 0 && player_interaction.target == (definition != 0 ? binding : 0))
    {
        SpawnActionEffect(this, 0, 9);
        player_interaction.flags = player_interaction.flags | 12;
    }
    timer = (sint32)binding->interaction_timer;
    if (timer == 0)
    {
        sound = binding->sound;
        value = (uint32)(sint16)binding->sound_voice;
        if ((sound_effects.IsPlaying(sound, value) & 0xFF) != 0)
            StopSoundVoice(binding->sound_voice);
        sound_table = sound_resources.common_table;
        binding->sound = 0;
        binding->sound_voice = 0xFFFF;
        sound = 0;
        if (sound_table != 0 && (sint32)sound_table->header.count >= 13)
            sound = sound_table->samples + 12;
        pitch = (((const SoundSample *)(sound)))->pitch;
        PlaySpatialSound(sound, 8192, 0, pitch, (const uint32 *)this->world_position, 0x4B000, 0xFFFFFFFF, 0, 0);
        animation = (binding->model->animations[4]);
        this->animation = animation;
        animation = this->animation;
        this->velocity[2] = 0;
        this->velocity[1] = 0;
        this->velocity[0] = 0;
        this->frame = (sint16)(0xFFFF);
        value = (uint32)(uint16)((AnimationHeader *)(animation))->frame_count;
        binding->interaction_timer = (uint32)(((sint32)(sint16)value >> 1) + 45);
        goto animation_done;
    }
    if (timer < 0)
    {
        next = (uint32)timer + 1;
        binding->interaction_timer = next;
        if (next == 0)
        {
            animation_model = binding->model;
            animation = this->animation;
            alternate = (animation_model->animations[5]);
            if (animation == alternate)
            {
                animation = (animation_model->animations[4]);
                this->animation = animation;
                value = (uint32)(uint16)((AnimationHeader *)(animation))->frame_count;
                this->frame = (sint16)(value);
                binding->interaction_timer = 0 - (uint32)(sint32)(sint16)value;
                goto animation_done;
            }
            animation = (animation_model->animations[0]);
            this->frame = (sint16)(0xFFFF);
            this->animation = animation;
            kind = (resource_model->kind_override);
            if (kind == -1)
                kind = (sint8)binding->model_kind;
            if (kind == 10)
                this->flags = this->flags & 0xFFFFFFBF;
            binding->flags = binding->flags ^ 2;
            interaction_target = player_interaction.target;
            binding->reaction_flags = 0;
            if (interaction_target == (definition != 0 ? binding : 0))
                player_interaction.flags = player_interaction.flags & 0xFFFFFFDB;
            this->flags = this->flags | 0x20000;
            goto animation_done;
        }
        if (next == (uint32)-45)
        {
            animation = (binding->model->animations[4]);
            this->animation = animation;
            this->frame = (sint16)((uint32)(uint16)((AnimationHeader *)(animation))->frame_count - 1);
            binding->interaction_timer = 45;
            goto animation_done;
        }
        if ((player_interaction.flags & 8) == 0 || player_interaction.target != (definition != 0 ? binding : 0))
            goto animation_done;
        animation = this->animation;
        frame = this->frame;
        limit = ((AnimationHeader *)(animation))->frame_count;
        sound_table = sound_resources.common_table;
        binding->interaction_timer = (uint32)(((limit - frame) >> 1) + 45);
        sound = 0;
        if (sound_table != 0 && (sint32)sound_table->header.count >= 13)
            sound = sound_table->samples + 12;
        animation_model = binding->model;
        animation = this->animation;
        alternate = (animation_model->animations[5]);
        pitch = (((const SoundSample *)(sound)))->pitch;
        if (animation == alternate)
            pitch += pitch >> 3;
        goto play_state_sound;
    }
    next = (uint32)timer - 1;
    binding->interaction_timer = next;
    if ((sint32)next < 45 && (player_interaction.flags & 8) != 0 && player_interaction.target == (definition != 0 ? binding : 0))
    {
        animation_model = binding->model;
        animation = this->animation;
        alternate = (animation_model->animations[5]);
        if (animation == alternate)
        {
            sound_table = sound_resources.common_table;
            sound = 0;
            if (sound_table != 0 && (sint32)sound_table->header.count >= 15)
                sound = sound_table->samples + 14;
            pitch = (((const SoundSample *)(sound)))->pitch;
            PlaySpatialSound(sound, 8192, 0, pitch, (const uint32 *)this->world_position, 0x4B000, 0xFFFFFFFF, 0, 0);
            this->ticks = (sint16)(0);
            goto animation_done;
        }
        if (animation != (animation_model->animations[4]))
            goto animation_done;
        limit = ((AnimationHeader *)(animation))->frame_count;
        frame = this->frame;
        if (frame != limit - 1)
            goto animation_done;
        this->animation = alternate;
        animation = this->animation;
        this->frame = (sint16)(0xFFFF);
        value = (uint32)(uint16)((AnimationHeader *)(animation))->frame_count;
        sound_table = sound_resources.common_table;
        binding->interaction_timer = (uint32)(((sint32)(sint16)value >> 1) + 45);
        sound = 0;
        if (sound_table != 0 && (sint32)sound_table->header.count >= 13)
            sound = sound_table->samples + 12;
        pitch = (((const SoundSample *)(sound)))->pitch;
        pitch += pitch >> 3;
        goto play_state_sound;
    }
    if (binding->interaction_timer != 0)
        goto animation_done;
    animation = this->animation;
    limit = ((AnimationHeader *)(animation))->frame_count;
    sound_table = sound_resources.common_table;
    binding->interaction_timer = 0 - (uint32)limit;
    sound = 0;
    if (sound_table != 0 && (sint32)sound_table->header.count >= 14)
        sound = sound_table->samples + 13;
    animation_model = binding->model;
    animation = this->animation;
    alternate = (animation_model->animations[5]);
    pitch = (((const SoundSample *)(sound)))->pitch;
    if (animation == alternate)
    {
        value = binding->interaction_timer;
        pitch += pitch >> 3;
        binding->interaction_timer = value - 45;
    }
play_state_sound:
    PlaySpatialSound(sound, 8192, 0, pitch, (const uint32 *)this->world_position, 0x4B000, 0xFFFFFFFF, 0, 0);
animation_done:
    if ((sint16)binding->blink_active != 0)
    {
        value = binding->blink_ticks;
        binding->blink_ticks = value + 1;
        if ((sint16)value >= 5)
        {
            binding->blink_ticks = 0;
            this->flags = this->flags & 0xFFF7FFFF;
            binding->blink_active = 0;
        }
    }
sample_target:
    player = finish_early == 0 ? FindPlayerObject(0) : 0;
    distance[0] = 0x7FFFF000;
    parent = (this->path_definition);
    if (parent != 0 && ((((const PathDefinition *)(parent)))->transform) != 0)
    {
        x = this->position[0];
        y = this->position[1];
        z = this->position[2];
        input.words[0] = x;
        input.words[1] = y;
        input.words[2] = z;
        PushMatrix();
        path_transform = (const PathTransform *)((((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) != 0 ? (((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) : (PathTransform *)0);
        x = xport_load_le32((const uint8 *)&path_transform->matrix);
        y = xport_load_le32((const uint8 *)&path_transform->matrix + 4);
        xport_gte_write_control(0, x);
        xport_gte_write_control(1, y);
        x = xport_load_le32((const uint8 *)&path_transform->matrix + 8);
        y = xport_load_le32((const uint8 *)&path_transform->matrix + 12);
        z = xport_load_le32((const uint8 *)&path_transform->matrix + 16);
        xport_gte_write_control(2, x);
        xport_gte_write_control(3, y);
        xport_gte_write_control(4, z);
        ApplyRotMatrixLV(&input.vector, (VECTOR *)pos);
        PopMatrix();
        for (index = 0; index < 3; ++index)
        {
            path_transform = (const PathTransform *)((((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) != 0 ? (((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) : (PathTransform *)0);
            value = pos[index];
            pos[index] = value + (uint32)path_transform->matrix.t[index];
        }
    }
    else
    {
        x = this->position[0];
        y = this->position[1];
        z = this->position[2];
        pos[0] = x;
        pos[1] = y;
        pos[2] = z;
    }
    scene = OriginalDefaults::SceneContextIdentity;
    x = pos[0];
    x -= camera_motion.focus_position[0];
    y = pos[1];
    y -= camera_motion.focus_position[1];
    z = pos[2];
    z -= camera_motion.focus_position[2];
    x = (uint32)((sint32)x >> 12);
    y = (uint32)((sint32)y >> 12);
    z = (uint32)((sint32)z >> 12);
    squared = x * x + y * y + z * z;
    x = camera_motion.focus_position[0];
    y = camera_motion.focus_position[1];
    z = camera_motion.focus_position[2];
    target[0] = x;
    target[1] = y;
    target[2] = z;
    distance[0] = squared;
    if (finish_early != 0)
        return 0;
    binding->distance_squared = squared;
    if ((((const AnimatedObject *)(player))->path_flags & 0x100) != 0 && (resource_model->target_avoidance_mode) == 0 && (sint32)distance[0] <= 409599)
    {
        scene = OriginalDefaults::SceneContextIdentity;
        y = camera_motion.focus_position[1];
        z = pos[1];
        if ((sint32)z >= (sint32)(y - 8192))
        {
            kind = (sint8)actor_target_avoidance_mode;
            if (kind == 0 || kind == 2)
                distance[0] = 409600;
            if (actor_target_avoidance_mode < 2)
            {
                x = target[0];
                value = pos[0];
                target[0] = x + ((sint32)value < (sint32)x ? 409600 : (uint32)-409600);
                z = target[2];
                value = pos[2];
                target[2] = z + ((sint32)value < (sint32)z ? 409600 : (uint32)-409600);
            }
        }
    }
    if ((binding->flags & 10) == 0)
        UpdateActorAction(pos, target, distance);
    angle = (uint32)(sint16)binding->desired_pitch;
    if (angle != 0 && (limit = (resource_model->pitch_limit)) != 0)
    {
        value = angle << 2;
        frame = (sint16)value;
        if (frame > 0)
        {
            if (limit < frame)
                value = (uint32)limit;
        }
        else if (frame < -limit)
            value = 0 - (uint32)limit;
        this->orientation[1] = (sint16)(value & 0xFFF);
    }
    else
    {
        value = (uint32)(resource_model->fixed_pitch);
        if (value != 0)
            this->orientation[1] = (sint16)(value & 0xFFF);
    }
    result = ReadCharacterMovementFlags();
    return result;
}

uint32 AnimatedObject::UpdateActorFacingAndAnimation(const uint32 *target, const uint32 *origin, ActorModelDefinition *resource_model)
{
    const PathDefinition *parent;
    FUNCTION_MARKER(0x80032248, "SLUS_005.78");
    void *definition;
    const SoundBankStorage *actor_sounds;
    const ActorModelDefinition *sound_model;
    const SoundSample *selection;
    const SoundSample *previous_sound;

    union
    {
        VECTOR vector;
        uint32 words[4];
    } world, delta_vector, first_vector, second_vector, direction_vector, target_vector;

    const uint32 *direction;
    const AnimatedObject *owner;
    uint32 x, y, z, a, b, c, value, flags, index;
    const PathTransform *path_transform;
    uint32 slot, minimum, frequency_zero, random, divisor, pitch, scale, ratio, radius, option, result;
    AnimationHeader *animation, *selected_animation;
    sint32 angle, step, frame, next_frame, end_frame, mode, limit, voice;
    parent = (this->path_definition);
    definition = ((const ActorPlacement *)((const PathDefinition *)(parent)))->state;
    ActorBinding *binding = (ActorBinding *)(definition);
    if (parent != 0 && (((const PathDefinition *)(parent))->flags & 8) != 0 && binding->rotation_track != 0)
    {
        if (this->velocity[0] != 0 || this->velocity[1] != 0 || this->velocity[2] != 0)
        {
            x = this->velocity[0];
            y = this->velocity[1];
            z = this->velocity[2];
            binding->sampled_velocity[0] = x;
            binding->sampled_velocity[1] = y;
            binding->sampled_velocity[2] = z;
        }
        result = this->SampleObjectPitch(binding->sampled_velocity, 0);
        this->pitch_scale = (sint16)(result);
        angle = (sint16)result;
        binding->sampled_pitch = angle != 0 && angle != 4096 ? (uint32)angle : 0;
    }
    else
        binding->sampled_pitch = 0;
    if ((resource_model->turn_motion_mode) != 0 && (this->flags & 0x02000000) == 0)
    {
        direction = binding->path_direction;
        step = 4096;
        goto apply_direction;
    }
    if ((binding->action_flags & 0x1000) != 0)
    {
        owner = GetCharacterModelObject();
        parent = (owner->path_definition);
        if (parent != 0 && ((((const PathDefinition *)(parent)))->transform) != 0)
        {
            x = owner->position[0];
            y = owner->position[1];
            z = owner->position[2];
            first_vector.words[0] = x;
            first_vector.words[1] = y;
            first_vector.words[2] = z;
            PushMatrix();
            path_transform = (const PathTransform *)((((const PathDefinition *)((const PathDefinition *)((owner->path_definition))))->transform) != 0 ? (((const PathDefinition *)((const PathDefinition *)((owner->path_definition))))->transform) : (PathTransform *)0);
            x = xport_load_le32((const uint8 *)&path_transform->matrix);
            y = xport_load_le32((const uint8 *)&path_transform->matrix + 4);
            xport_gte_write_control(0, x);
            xport_gte_write_control(1, y);
            x = xport_load_le32((const uint8 *)&path_transform->matrix + 8);
            y = xport_load_le32((const uint8 *)&path_transform->matrix + 12);
            z = xport_load_le32((const uint8 *)&path_transform->matrix + 16);
            xport_gte_write_control(2, x);
            xport_gte_write_control(3, y);
            xport_gte_write_control(4, z);
            ApplyRotMatrixLV(&first_vector.vector, &world.vector);
            PopMatrix();
            for (index = 0; index < 3; ++index)
            {
                path_transform = (const PathTransform *)((((const PathDefinition *)((const PathDefinition *)((owner->path_definition))))->transform) != 0 ? (((const PathDefinition *)((const PathDefinition *)((owner->path_definition))))->transform) : (PathTransform *)0);
                value = world.words[index];
                x = (uint32)path_transform->matrix.t[index];
                world.words[index] = value + x;
            }
        }
        else
        {
            x = owner->position[0];
            y = owner->position[1];
            z = owner->position[2];
            world.words[0] = x;
            world.words[1] = y;
            world.words[2] = z;
        }
        for (index = 0; index < 3; ++index)
        {
            value = world.words[index];
            x = origin[index];
            delta_vector.words[index] = value - x;
        }
        direction = delta_vector.words;
        step = 4096;
        goto apply_direction;
    }
    if ((binding->action_effect_flags & 0x800) != 0 && (this->velocity[0] != 0 || this->velocity[1] != 0 || this->velocity[2] != 0))
    {
        this->ReverseObjectFacingFromVelocity();
        goto after_direction;
    }
    mode = this->behavior;
    if ((object_operations.behaviors[mode].member_handler == &AnimatedObject::UpdateAttackEnemy))
    {
        resource_model->turn_step = 4096;
        direction = (const uint32 *)this->velocity;
        step = 4096;
        goto apply_direction;
    }
    mode = (sint8)binding->route_direction_mode;
    if (mode != 0)
    {
        if (mode == 1)
        {
            step = resource_model->turn_step;
            direction = binding->route_direction;
        }
        else
        {
            x = binding->route_direction[0];
            y = binding->route_direction[1];
            z = binding->route_direction[2];
            first_vector.words[0] = x;
            first_vector.words[1] = y;
            first_vector.words[2] = z;
            x = first_vector.words[0];
            z = first_vector.words[2];
            first_vector.words[0] = 0 - x;
            first_vector.words[2] = 0 - z;
            step = resource_model->turn_step;
            direction = first_vector.words;
        }
        goto apply_direction;
    }
    if ((binding->action_flags & 0x100) != 0 && binding->action_speed == 0)
    {
        parent = (this->path_definition);
        if ((((const PathDefinition *)(parent)))->positions.keys != 0)
        {
            flags = this->flags;
            frame = this->path_frame;
            if ((flags & 0x10) != 0)
            {
                next_frame = frame + 1;
                if (frame >= (sint16)((const PathDefinition *)(parent))->positions.metadata6)
                {
                    next_frame = frame;
                    --frame;
                }
            }
            else
            {
                next_frame = frame - 1;
                if (frame <= 0)
                {
                    next_frame = frame;
                    ++frame;
                }
            }
            parent = (this->path_definition);
            result = (uint32)SampleDefinitionHeading(frame, next_frame, ((const PathDefinition *)(parent)));
            parent = (this->path_definition);
            this->target_yaw = (sint16)(result);
            (((const PathDefinition *)(parent)))->SampleActorPosition(frame, first_vector.words, 64);
            ((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->SampleActorPosition(next_frame, second_vector.words, 64);
            x = second_vector.words[0];
            a = first_vector.words[0];
            y = second_vector.words[1];
            b = first_vector.words[1];
            z = second_vector.words[2];
            c = first_vector.words[2];
            direction_vector.words[0] = x - a;
            direction_vector.words[1] = y - b;
            direction_vector.words[2] = z - c;
            step = resource_model->turn_step;
            direction = direction_vector.words;
            goto apply_direction;
        }
    }
    if ((this->flags & 0x40) != 0 && (this->velocity[0] != 0 || this->velocity[1] != 0 || this->velocity[2] != 0))
    {
        step = resource_model->turn_step;
        direction = (const uint32 *)this->velocity;
        goto apply_direction;
    }
    binding->motion_speed = 0;
    if (target == 0)
        goto after_direction;
    parent = (this->path_definition);
    if ((((const PathDefinition *)(parent))->flags & 0x40) == 0)
    {
        mode = (resource_model->kind_override);
        if (mode == -1)
        {
            if ((sint8)binding->model_kind != 19)
                goto after_direction;
        }
        else if (mode != 19)
            goto after_direction;
    }
    for (index = 0; index < 3; ++index)
    {
        value = target[index];
        x = origin[index];
        target_vector.words[index] = value - x;
    }
    step = resource_model->turn_step;
    direction = target_vector.words;
apply_direction:
    this->TurnObjectFacing(direction, (sint16)step);
after_direction:
    flags = binding->flags;
    if ((flags & 2) != 0)
    {
        if ((sint32)binding->interaction_timer > 0)
        {
            frame = this->frame;
            if (frame != -1)
            {
                animation = this->animation;
                value = (uint32)frame + 2;
                this->frame = value;
                end_frame = ((AnimationHeader *)(animation))->frame_count;
                if ((sint16)value >= end_frame)
                    this->frame = (uint32)end_frame - 1;
            }
        }
        else
        {
            frame = this->frame;
            if (frame != 0)
                this->frame = (uint32)frame - 1;
        }
        goto finish;
    }
    if ((flags & 8) != 0)
    {
        if ((resource_model->interaction_animation_mode) == -1)
        {
            frame = this->frame;
            if (frame != -1)
            {
                animation = this->animation;
                value = (uint32)frame + 2;
                this->frame = value;
                end_frame = ((AnimationHeader *)(animation))->frame_count;
                if ((sint16)value >= end_frame)
                    this->frame = (uint32)end_frame - 1;
            }
        }
        else
            this->frame = (resource_model->interaction_frame);
        mode = character_controller.action_disp_phase;
        if (mode < 0)
            mode = -mode;
        index = (uint32)(mode % 16);
        frame = (resource_model->interaction_limit_step);
        value = character_controller.action_heading;
        limit = 1;
        if (frame != 0 && (character_action_flags & 0x1000) == 0)
            limit = 10;
        x = binding->interaction_heading_ticks + 1;
        binding->interaction_heading_ticks = x;
        if ((sint32)x >= limit)
        {
            binding->interaction_heading_ticks = 0;
            x = (uint32)OriginalDefaults::InteractionWave[index];
            value = (value + (uint32)((sint32)(x << 16) >> 19)) & 0xFFF;
        }
        first_vector.words[1] = 0;
        x = (uint32)SinQ14((sint16)value);
        first_vector.words[0] = 0 - (uint32)((sint32)x >> 2);
        z = (uint32)SinQ14((sint16)(1024 - value));
        first_vector.words[2] = 0 - (uint32)((sint32)z >> 2);
        this->TurnObjectFacing(first_vector.words, 4096);
        goto finish;
    }
    if (resource_model->kind >= 47)
        goto finish;
    if (resource_model->sound_table == 0)
        goto advance_frame;
    frequency_zero = (resource_model->sound_interval) == 0;
    frame = this->frame;
    selection = 0;
    if (frame == -1 && frequency_zero != 0)
    {
        previous_sound = binding->sound;
        voice = (sint16)binding->sound_voice;
        if ((sound_effects.IsPlaying(previous_sound, (uint32)voice) & 0xFF) != 0)
            StopSoundVoice(binding->sound_voice);
        binding->sound = 0;
        binding->sound_voice = 0xFFFF;
    }
    mode = resource_model->kind;
    if (mode == 17)
    {
        animation = this->animation;
        selected_animation = (resource_model->animations[1]);
        if (animation == selected_animation)
        {
            if (this->frame == 19)
            {
                actor_sounds = resource_model->sound_table;
                if (actor_sounds != 0 && (sint32)actor_sounds->header.count >= 3)
                    selection = actor_sounds->samples + 2;
                binding->sound_voice = 0xFFFF;
                goto sound_selected;
            }
            animation = this->animation;
        }
    }
    else
        animation = this->animation;
    if (animation == (resource_model->animations[0]))
    {
        slot = 4;
        minimum = 1;
        goto random_sound_gate;
    }
    if (animation == (resource_model->animations[2]) && resource_model->kind != 17)
    {
        slot = 20;
        minimum = 3;
        goto random_sound_gate;
    }
    if (this->frame > 0)
        goto sound_selected;
    animation = this->animation;
    selected_animation = (resource_model->animations[3]);
    if (animation != selected_animation)
        goto sound_selected;
    slot = 12;
    minimum = 2;
    goto lookup_sound;
random_sound_gate:
    voice = (sint16)binding->sound_voice;
    if (voice < 0 && frequency_zero == 0)
    {
        random = random_generator.Next();
        divisor = (resource_model->sound_interval);
        if (divisor == 0)
            xport_mips_break(7);
        if (random % divisor != 0)
            goto sound_selected;
    }
lookup_sound:
    actor_sounds = resource_model->sound_table;
    if (actor_sounds != 0 && (sint32)actor_sounds->header.count >= (sint32)minimum)
        selection = actor_sounds->samples + ((slot - 4) / 8);
sound_selected:
    if (selection == 0)
        goto advance_frame;
    previous_sound = binding->sound;
    option = 0;
    if (selection != previous_sound && previous_sound != 0)
    {
        voice = (sint16)binding->sound_voice;
        if ((sound_effects.IsPlaying(previous_sound, (uint32)voice) & 0xFF) != 0)
            StopSoundVoice(binding->sound_voice);
        binding->sound = 0;
        binding->sound_voice = 0xFFFF;
    }
    voice = (sint16)binding->sound_voice;
    scale = 4096;
    if (voice >= 0)
        pitch = binding->sound_pitch;
    else
    {
        pitch = selection->pitch;
        if (pitch == 0)
        {
            mode = (sint8)binding->model_kind;
            p2_sdk_80082a18("Monster type %d bad SFX rate.\n", mode);
            goto advance_frame;
        }
        random = random_generator.Next();
        divisor = pitch >> 2;
        if (divisor == 0)
            xport_mips_break(7);
        pitch = pitch + random % divisor - (pitch >> 3);
        binding->sound_pitch = pitch;
    }
    if ((this->flags & 0x30000000) != 0)
    {
        value = (uint16)this->state;
        if ((sint32)value < 2048)
            value = 2048;
        scale = (uint32)(((sint64)(sint32)scale * (sint32)value) >> 12);
        if (value == 0)
            xport_mips_break(7);
        ratio = (uint32)(0x01000000 / (sint32)value);
        pitch = (uint32)(((sint64)(sint32)pitch * (sint32)ratio) >> 12);
    }
    if (frequency_zero != 0)
        option = 1;
    binding->sound = selection;
    sound_model = binding->model;
    radius = (uint32)(sint32)sound_model->sound_radius << 5;
    voice = (sint16)binding->sound_voice;
    result = PlaySpatialSound(selection, (sint32)(scale + 4096), 0, pitch, (const uint32 *)this->world_position, radius, (uint32)voice, frequency_zero, option);
    binding->sound_voice = result;
advance_frame:
    animation = this->animation;
    value = 0;
    if (animation != 0)
    {
        frame = this->frame;
        end_frame = ((AnimationHeader *)(animation))->frame_count - 1;
        if (frame < end_frame)
            value = (uint32)frame + 1;
    }
    this->frame = value;
finish:
    result = this->ClampAnimationFrame();
    return result;
}

sint32 AnimatedObject::FindActorPathSegment() const
{
    FUNCTION_MARKER(0x80022388, "SLUS_005.78");
    const PathDefinition *definition = (this->path_definition);
    const VectorAnimationKey *path, *segment;
    sint32 frame;
    if (definition == 0)
        return 0;
    path = (const VectorAnimationKey *)definition->positions.keys;
    if (path == 0)
        return 0;
    frame = (uint16)this->path_frame;
    if ((this->flags & 0x10) == 0)
        frame = (sint16)(definition->positions.metadata6 - (frame + 1) - 1);
    else
        frame = (sint16)frame;
    segment = path + definition->positions.key_count - 1;
    if (frame >= definition->positions.frame_limit)
    {
        if ((definition->positions.flags & 3) == 3)
            return 0;
    }
    else if ((uint32)frame < (segment->time >> 20))
    {
        segment = path;
        if ((uint32)frame >= (path[1].time >> 20))
        {
            ++segment;
            do
            {
                uint32 next_start = segment[1].time >> 20;
                ++segment;
                if ((uint32)frame < next_start)
                    break;
            } while (1);
            --segment;
        }
    }
    uint32 byte_delta = (uint32)((const uint8 *)segment - definition->positions.keys);
    return (sint32)(byte_delta * 0xCCCCCCCDu) >> 3;
}

void AnimatedObject::ApplyModelGroupContactMotion()
{
    const PathDefinition *parent;
    const PathDefinition *definition;
    FUNCTION_MARKER(0x8002FD54, "SLUS_005.78");

    union
    {
        VECTOR vector;
        uint32 words[4];
    } direction, velocity, translation;

    union
    {
        uint32 words[5];
        uint16 halves[10];
    } matrix_local;

    const AnimatedObject *owner;
    uint32 index;
    const PathTransform *path_transform;
    uint32 flags, value, x, y, z, a, b, c, factor, scaled_x, scaled_y, scaled_z, squared, magnitude;
    const ScenePath *support;
    SceneTrigger *contact;
    uint32 result;
    sint32 speed, state, timer;
    if (this->state == 19 && (player_interaction.flags & 3) != 3)
    {
        owner = FindPlayerObject(0);
        flags = player_interaction.flags;
        speed = 40960;
        if ((flags & 0x24) == 0x24)
        {
            result = this->node.flags | 1;
            this->node.flags = result;
            goto finish;
        }
        this->node.flags = this->node.flags & 0xFE;
        flags = owner->flags;
        if ((flags & 0x2000) == 0 && (parent = owner->path_definition) != 0 && (parent->transform) != 0)
        {
            x = owner->position[0];
            y = owner->position[1];
            z = owner->position[2];
            velocity.words[0] = x;
            velocity.words[1] = y;
            velocity.words[2] = z;
            PushMatrix();
            definition = owner->path_definition;
            path_transform = definition->transform;
            x = xport_load_le32((const uint8 *)&path_transform->matrix);
            y = xport_load_le32((const uint8 *)&path_transform->matrix + 4);
            xport_gte_write_control(0, x);
            xport_gte_write_control(1, y);
            x = xport_load_le32((const uint8 *)&path_transform->matrix + 8);
            y = xport_load_le32((const uint8 *)&path_transform->matrix + 12);
            z = xport_load_le32((const uint8 *)&path_transform->matrix + 16);
            xport_gte_write_control(2, x);
            xport_gte_write_control(3, y);
            xport_gte_write_control(4, z);
            ApplyRotMatrixLV(&velocity.vector, (VECTOR *)this->velocity);
            PopMatrix();
            for (index = 0; index < 3; ++index)
            {
                definition = owner->path_definition;
                path_transform = definition->transform;
                value = this->velocity[index];
                x = (uint32)path_transform->matrix.t[index];
                this->velocity[index] = value + x;
            }
        }
        else
        {
            x = owner->position[0];
            y = owner->position[1];
            z = owner->position[2];
            this->velocity[0] = x;
            this->velocity[1] = y;
            this->velocity[2] = z;
        }
        this->velocity[1] = this->velocity[1] + 12288;
        x = this->velocity[0];
        a = this->position[0];
        y = this->velocity[1];
        b = this->position[1];
        z = this->velocity[2];
        c = this->position[2];
        x -= a;
        y -= b;
        z -= c;
        this->velocity[0] = x;
        this->velocity[1] = y;
        this->velocity[2] = z;
        x = (uint32)((sint32)this->velocity[0] >> 6);
        y = (uint32)((sint32)this->velocity[1] >> 6);
        z = (uint32)((sint32)z >> 6);
        squared = x * x + y * y + z * z;
        magnitude = (sint32)squared >= 0 ? SquareRoot12((sint32)squared) : SquareRoot12((sint32)(squared >> 2)) << 1;
        if ((player_interaction.flags & 0x40) != 0)
            speed *= 16;
        if ((sint32)magnitude < speed + speed / 4)
            this->ticks = 1;
        if (this->velocity[0] + 26754 > 0xD103 || this->velocity[1] + 26754 > 0xD103 || this->velocity[2] + 26754 > 0xD103)
        {
            for (;;)
            {
                x = this->velocity[0];
                z = this->velocity[2];
                this->velocity[0] = (uint32)((sint32)x >> 1);
                y = this->velocity[1];
                x = this->velocity[0];
                z = (uint32)((sint32)z >> 1);
                this->velocity[2] = z;
                this->velocity[1] = (uint32)((sint32)y >> 1);
                if (x + 26754 > 0xD103)
                    continue;
                if (this->velocity[1] + 26754 > 0xD103 || z + 26754 > 0xD103)
                    continue;
                break;
            }
        }
        VectorNormal((VECTOR *)this->velocity, (VECTOR *)this->velocity);
        x = this->velocity[0];
        scaled_x = (uint32)(((sint64)(sint32)x * speed) >> 12);
        y = this->velocity[1];
        this->velocity[0] = scaled_x;
        scaled_y = (uint32)(((sint64)(sint32)y * speed) >> 12);
        z = this->velocity[2];
        this->velocity[1] = scaled_y;
        this->velocity[2] = (uint32)(((sint64)(sint32)z * speed) >> 12);
        result = this->ComputeObjectPitch(this->velocity, 0);
        this->pitch_scale = result;
        this->orientation[1] = result & 0xFFF;
        result = this->ComputeObjectFacing(this->velocity, 0);
        this->target_yaw = result;
        this->orientation[2] = (result + 2048) & 0xFFF;
        x = this->velocity[0];
        y = this->velocity[1];
        z = this->velocity[2];
        direction.words[0] = x;
        direction.words[1] = y;
        direction.words[2] = z;
        x = direction.words[0];
        if (x == 0 && direction.words[2] == 0)
            magnitude = 0;
        else
        {
            x = (uint32)((sint32)x >> 6);
            y = (uint32)((sint32)direction.words[1] >> 6);
            z = (uint32)((sint32)direction.words[2] >> 6);
            squared = x * x + y * y + z * z;
            magnitude = (sint32)squared >= 0 ? SquareRoot12((sint32)squared) : SquareRoot12((sint32)(squared >> 2)) << 1;
        }
        this->ComputePathMotion((sint32)magnitude, (sint32)this->velocity[1], &direction.vector);
        for (index = 0; index < 3; ++index)
        {
            value = this->position[index];
            x = direction.words[index];
            result = value + x;
            this->position[index] = result;
        }
        goto finish;
    }
    if (this->path_motion_scale != 0 && (this->velocity[0] != 0 || this->velocity[1] != 0 || this->velocity[2] != 0))
    {
        x = this->velocity[0];
        y = this->velocity[1];
        z = this->velocity[2];
        velocity.words[0] = x;
        velocity.words[1] = y;
        velocity.words[2] = z;
        if ((sint32)this->path_motion_scale < 4096)
        {
            translation.words[2] = 0;
            translation.words[1] = 0;
            translation.words[0] = 0;
            flags = this->flags;
            this->flags = flags & 0xFFFFF7FF;
            if ((flags & 0x6000) == 0)
            {
                parent = (this->path_definition);
                if (parent != 0 && (path_transform = parent->transform) != 0)
                {
                    x = path_transform->displacement[0];
                    y = path_transform->displacement[1];
                    z = path_transform->displacement[2];
                    translation.words[0] = x;
                    translation.words[1] = y;
                    translation.words[2] = z;
                }
            }
            x = velocity.words[0];
            a = translation.words[0];
            y = velocity.words[1];
            b = translation.words[1];
            z = velocity.words[2];
            c = translation.words[2];
            x += a;
            y += b;
            z += c;
            velocity.words[0] = x;
            velocity.words[1] = y;
            velocity.words[2] = z;
            factor = this->path_motion_scale;
            scaled_x = (uint32)(((sint64)(sint32)x * (sint32)factor) >> 12);
            velocity.words[0] = scaled_x;
            scaled_y = (uint32)(((sint64)(sint32)y * (sint32)factor) >> 12);
            velocity.words[1] = scaled_y;
            scaled_z = (uint32)(((sint64)(sint32)z * (sint32)factor) >> 12);
            velocity.words[2] = scaled_z;
            velocity.words[0] = scaled_x - a;
            velocity.words[1] = scaled_y - b;
            velocity.words[2] = scaled_z - c;
            if (this->state == 19)
                this->ticks = 0;
        }
        if ((this->flags & 0x2000) == 0)
        {
            parent = (this->path_definition);
            if (parent != 0 && (path_transform = parent->transform) != 0)
            {
                for (index = 0; index < 9; ++index)
                {
                    if (index != 0)
                    {
                        definition = this->path_definition;
                        path_transform = definition->transform;
                    }
                    value = (uint16)path_transform->matrix.m[index % 3][index / 3];
                    matrix_local.halves[index] = value;
                }
                x = velocity.words[0];
                y = velocity.words[1];
                z = velocity.words[2];
                translation.words[0] = x;
                translation.words[1] = y;
                translation.words[2] = z;
                PushMatrix();
                x = matrix_local.words[0];
                y = matrix_local.words[1];
                xport_gte_write_control(0, x);
                xport_gte_write_control(1, y);
                x = matrix_local.words[2];
                y = matrix_local.words[3];
                z = matrix_local.halves[8];
                xport_gte_write_control(2, x);
                xport_gte_write_control(3, y);
                xport_gte_write_control(4, z);
                ApplyRotMatrixLV(&translation.vector, &velocity.vector);
                PopMatrix();
            }
        }
        for (index = 0; index < 3; ++index)
        {
            value = this->position[index];
            x = velocity.words[index];
            this->position[index] = value + x;
        }
    }
    contact = this->model_contacts.surface0;
    if (contact != 0)
    {
        value = this->model_contacts.fraction0;
        factor = this->path_motion_scale;
        if ((sint32)factor >= (sint32)value)
        {
            if ((sint8)(contact->node.state_flags >> 8) == 1)
            {
                support = this->FindCharacterSupportPath(contact);
                if (support != 0)
                {
                    const PathDefinition *support_definition = support->definition;
                    this->ApplyPathVelocityChange(support_definition);
                    this->scene_path = support;
                }
            }
            else
                contact->node.state_flags = contact->node.state_flags | 8;
        }
    }
    contact = this->model_contacts.surface1;
    if (contact != 0)
    {
        value = this->model_contacts.fraction1;
        factor = this->path_motion_scale;
        if ((sint32)factor >= (sint32)value)
            contact->node.state_flags = contact->node.state_flags | 8;
    }
    contact = this->model_contacts.surface2;
    if (contact != 0)
    {
        value = this->model_contacts.fraction2;
        factor = this->path_motion_scale;
        if ((sint32)factor >= (sint32)value)
            contact->node.state_flags = contact->node.state_flags | 8;
    }
    if (this->model_contacts.blocker != 0)
    {
        this->ticks = 0;
        goto clear_contacts;
    }
    flags = this->flags;
    result = flags & 0x800;
    if ((flags & 0x4000) != 0)
    {
        this->ticks = 0;
        goto clear_contacts;
    }
    if (result == 0)
        goto clear_contacts;
    state = this->animation_binding.tracks[0];
    this->flags = flags & 0xFFFFF7FF;
    if (state == -2)
    {
        timer = this->ticks;
        result = (uint32)timer;
        if (timer <= 0)
        {
            this->ticks = 0;
            goto clear_contacts;
        }
    }
    parent = (this->path_definition);
    if (parent != 0 && (parent->transform) != 0)
    {
        x = this->position[0];
        y = this->position[1];
        z = this->position[2];
        translation.words[0] = x;
        translation.words[1] = y;
        translation.words[2] = z;
        PushMatrix();
        definition = this->path_definition;
        path_transform = definition->transform;
        x = xport_load_le32((const uint8 *)&path_transform->matrix);
        y = xport_load_le32((const uint8 *)&path_transform->matrix + 4);
        xport_gte_write_control(0, x);
        xport_gte_write_control(1, y);
        x = xport_load_le32((const uint8 *)&path_transform->matrix + 8);
        y = xport_load_le32((const uint8 *)&path_transform->matrix + 12);
        z = xport_load_le32((const uint8 *)&path_transform->matrix + 16);
        xport_gte_write_control(2, x);
        xport_gte_write_control(3, y);
        xport_gte_write_control(4, z);
        ApplyRotMatrixLV(&translation.vector, (VECTOR *)this->position);
        PopMatrix();
        definition = this->path_definition;
        path_transform = definition->transform;
        x = this->position[0];
        value = (uint32)path_transform->matrix.t[0];
        parent = (this->path_definition);
        this->position[0] = x + value;
        path_transform = parent->transform;
        y = this->position[1];
        value = (uint32)path_transform->matrix.t[1];
        parent = (this->path_definition);
        this->position[1] = y + value;
        path_transform = parent->transform;
        z = this->position[2];
        value = (uint32)path_transform->matrix.t[2];
        this->position[2] = z + value;
    }
    else
    {
        x = this->position[0];
        y = this->position[1];
        z = this->position[2];
        this->position[0] = x;
        this->position[1] = y;
        this->position[2] = z;
    }
    parent = (this->path_definition);
    if (parent != 0 && (path_transform = parent->transform) != 0)
    {
        x = this->velocity[0];
        value = path_transform->displacement[0];
        parent = (this->path_definition);
        this->velocity[0] = x + value;
        path_transform = parent->transform;
        y = this->velocity[1];
        value = path_transform->displacement[1];
        parent = (this->path_definition);
        this->velocity[1] = y + value;
        path_transform = parent->transform;
        z = this->velocity[2];
        value = path_transform->displacement[2];
        this->velocity[2] = z + value;
    }
    flags = this->flags;
    value = this->state;
    this->flags = flags | 0x2000;
    if (value != 19)
        this->movement_mode = 0;
    result = this->flags & 0x400;
    if (result != 0)
        goto clear_contacts;
    definition = this->path_definition;
    result = ((const PathDefinition *)(definition))->flags & 8;
    if (result == 0)
        goto clear_contacts;
    if (this->velocity[0] + 26754 > 0xD103 || this->velocity[1] + 26754 > 0xD103 || this->velocity[2] + 26754 > 0xD103)
    {
        for (;;)
        {
            x = this->velocity[0];
            z = this->velocity[2];
            this->velocity[0] = (uint32)((sint32)x >> 1);
            y = this->velocity[1];
            x = this->velocity[0];
            z = (uint32)((sint32)z >> 1);
            this->velocity[2] = z;
            this->velocity[1] = (uint32)((sint32)y >> 1);
            if (x + 26754 > 0xD103)
                continue;
            if (this->velocity[1] + 26754 > 0xD103 || z + 26754 > 0xD103)
                continue;
            break;
        }
    }
    VectorNormal((VECTOR *)this->velocity, (VECTOR *)this->velocity);
    x = this->velocity[0];
    factor = this->state;
    scaled_x = (uint32)(((sint64)(sint32)x * (sint32)factor) >> 12);
    factor = this->state;
    y = this->velocity[1];
    this->velocity[0] = scaled_x;
    scaled_y = (uint32)(((sint64)(sint32)y * (sint32)factor) >> 12);
    factor = this->state;
    result = this->velocity[2];
    this->velocity[1] = scaled_y;
    this->velocity[2] = (uint32)(((sint64)(sint32)result * (sint32)factor) >> 12);
clear_contacts:
    this->model_contacts.surface2 = 0;
    this->model_contacts.surface1 = 0;
    this->model_contacts.surface0 = 0;
    this->model_contacts.blocker = 0;
finish:
    return;
}

uint32 AnimatedObject::UpdateActorReactionSpin()
{
    const PathDefinition *parent;
    FUNCTION_MARKER(0x80030AC8, "SLUS_005.78");
    const ActorModelDefinition *bound_model;
    ActorBinding *binding = (ActorBinding *)(((const ActorPlacement *)((const PathDefinition *)((this->path_definition))))->state != 0 ? ((const ActorPlacement *)((const PathDefinition *)((this->path_definition))))->state : 0);
    uint32 value = binding->reaction_state - 1;
    uint32 flags, angle, result;
    binding->reaction_state = value;
    if ((value & 0xFF) == 0)
    {
        this->flags &= 0xFDFFFFFF;
        value = binding->reaction_state;
        this->metadata126[0] = 2;
        this->behavior = (sint32)value >> 8;
        angle = (binding->reaction_state >> 16) & 0xFFF;
        this->yaw = angle;
        this->target_yaw = angle;
    }
    else
    {
        parent = (this->path_definition);
        flags = parent == 0 ? 0 : ((const ActorBinding *)(((const ActorPlacement *)((const PathDefinition *)(parent)))->state != 0 ? ((const ActorPlacement *)((const PathDefinition *)(parent)))->state : 0))->flags & 8;
        if (flags != 0)
            angle = (uint16)this->target_yaw - 6553;
        else
        {
            bound_model = ((const ActorBinding *)(((const ActorPlacement *)((const PathDefinition *)((this->path_definition))))->state != 0 ? ((const ActorPlacement *)((const PathDefinition *)((this->path_definition))))->state : 0))->model;
            if ((bound_model->reaction_motion_mode) == 1)
                goto finish;
            angle = (uint16)this->target_yaw - 3276;
        }
        this->target_yaw = angle;
        this->yaw = (angle << 17) >> 20;
    }
finish:
    result = (uint16)this->yaw;
    this->orientation[0] = 0x7FFF;
    this->orientation[1] = 0;
    result = (result + 2048) & 0xFFF;
    this->orientation[2] = result;
    return result;
}

void AnimatedObject::UpdateActorReactionRecovery()
{
    FUNCTION_MARKER(0x800351A4, "SLUS_005.78");
    ActorBinding *binding = (ActorBinding *)(((const ActorPlacement *)((const PathDefinition *)((this->path_definition))))->state != 0 ? ((const ActorPlacement *)((const PathDefinition *)((this->path_definition))))->state : 0);
    uint32 flags, value;
    const SoundBankStorage *sound_table;
    const SoundSample *sound;
    if ((sound_effects.IsPlaying(binding->sound, (uint32)(sint32)(sint16)binding->sound_voice) & 0xFF) != 0)
        StopSoundVoice(binding->sound_voice);
    flags = binding->reaction_flags;
    binding->sound = 0;
    binding->sound_voice = 0xFFFF;
    if ((flags & 0x28) != 0)
    {
        sound_table = sound_resources.common_table;
        sound = sound_table != 0 && (sint32)sound_table->header.count >= 23 ? sound_table->samples + 22 : 0;
        sound_effects.Queue(sound, 0, 0, sound_settings.effects_volume, 0, 3);
        this->ticks = 0;
    }
    else if ((flags & 1) != 0)
        this->state = (this->state & 0xFFFFFF00) | 0x96;
    else
    {
        value = this->state - 1;
        this->state = value;
        if ((value & 0xFF) == 0)
        {
            this->behavior = (sint32)value >> 8;
            return;
        }
    }
    render_view.material_override = resource_groups[25].texture;
}
