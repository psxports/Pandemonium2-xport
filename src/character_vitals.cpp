#include "game.h"
#include "original_defaults.h"

uint32 ApplyCharacterSettings(const AnimatedObject *object, const CharacterSettings *settings, uint8 score)
{
    FUNCTION_MARKER(0x80053530, "SLUS_005.78");
    uint32 value;
    uint32 flags;
    if ((game_flow.mode_flags & 1) == 0)
        game_flow.progress.unlocked_stage = score;
    value = 32;
    if ((game_flow.mode_flags & 8) == 0)
        value = settings->counters[CharacterSettings::Lives];
    character_controller.vitals.lives = value;
    value = 24;
    if ((game_flow.mode_flags & 4) == 0)
        value = settings->health_limit;
    character_controller.vitals.health_limit = value;
    character_controller.vitals.health = value;
    flags = character_controller.motion.form_flags & 0xFF0FFFFF;
    character_controller.vitals.score = 0;
    character_controller.motion.form_flags = flags;
    flags = (flags | ((uint32)(settings->flags & 1) << 20)) & 0xFFF0FFFF;
    flags |= ((flags >> 20) & 15) << 16;
    character_controller.motion.form_flags = flags;
    value = settings->action_code;
    if (value == 0)
        character_controller.weapon_state = 0;
    else
        character_controller.weapon_state = value == 1 ? 1 : ((value + 9) << 16) | 1;
    character_controller.timers.idle_ticks = 900;
    value = settings->health_pickups;
    character_controller.vitals.health_pickups = value;
    return value;
}

uint32 CaptureCharacterSettings(CharacterSettings *settings)
{
    FUNCTION_MARKER(0x80053638, "SLUS_005.78");
    uint32 flags, value, result;
    flags = (character_controller.flags >> 21) & 2;
    settings->flags = flags;
    value = (character_controller.motion.form_flags >> 20) & 15;
    flags |= value;
    settings->flags = flags;
    settings->counters[CharacterSettings::Health] = (uint32)(sint8)character_controller.vitals.health;
    settings->health_limit = character_controller.vitals.health_limit;
    settings->counters[CharacterSettings::Lives] = character_controller.vitals.lives;
    settings->counters[CharacterSettings::Score] = (uint16)character_controller.vitals.score;
    value = character_controller.weapon_state;
    if ((value & 0xFFFF) != 0)
    {
        value >>= 16;
        if (value == 0)
            value = 1;
        else
            value -= 9;
    }
    else
        value = 0;
    settings->action_code = value;
    result = character_controller.vitals.health_pickups;
    settings->health_pickups = result;
    return result;
}

uint32 RestoreCharacterHealthPoint(void)
{
    FUNCTION_MARKER(0x800444D0, "SLUS_005.78");
    sint32 current = (sint8)character_controller.vitals.health;
    sint32 limit = (sint8)character_controller.vitals.health_limit;
    uint32 result = current < limit;
    if (result != 0)
    {
        result = (uint32)(current + 1);
        character_controller.vitals.health = result;
    }
    return result;
}

uint32 AnimatedObject::UpdateCharacterDeathState()
{
    FUNCTION_MARKER(0x800526F8, "SLUS_005.78");
    const PathDefinition *parent;
    uint32 flags, other_flags, value, result, x, y, z;
    const PathTransform *path_transform;
    const SoundBankStorage *sound_table;
    const SoundSample *sound;

    union
    {
        VECTOR value;
        uint32 words[4];
    } position, input, death_input;

    if ((game_flow.transition_flags & 0x100) == 0)
        this->UpdateCharacterCameraPath();
    if ((sint8)character_controller.vitals.health > 0)
        return 1;
    flags = character_controller.flags;
    other_flags = character_controller.secondary_flags;
    character_controller.trigger_count = 0;
    character_controller.vitals.health = 0;
    character_controller.flags = flags & 0xFEFFBFDD;
    character_controller.secondary_flags = other_flags & 0xFFFFFF7F;
    value = this->node.flags;
    flags = this->flags;
    this->node.flags = value & 0xFE;
    this->flags = flags & 0xFDF7FFFF;
    character_controller.timers.state_ticks = 30;
    this->movement_mode = 0;
    this->metadata126[0] = 0;
    this->metadata126[1] = 1;
    StopCurrentSequence();

    sound_table = sound_resources.common_table;
    sound = sound_table != 0 && (sint32)sound_table->header.count >= 7 ? sound_table->samples + 6 : 0;
    result = sound_effects.IsPlaying(sound, (uint32)(sint32)character_controller.sound_voice);
    if ((result << 24) == 0)
    {
        sound_table = sound_resources.common_table;
        sound = sound_table != 0 && (sint32)sound_table->header.count >= 11 ? sound_table->samples + 10 : 0;
        result = sound_effects.IsPlaying(sound, (uint32)(sint32)character_controller.sound_voice);
    }
    if ((result << 24) != 0)
    {
        StopSoundVoice((uint16)(sint16)character_controller.sound_voice);
        character_controller.sound_voice = (sint8)(0xFF);
    }
    PlayLoopSequence();
    value = game_flow.transition_flags;
    flags = this->flags;
    screen_render.fade_level = 0;
    game_flow.transition_flags = value | 0x100;
    if ((flags & 0x2000) != 0)
        goto local_position;
    parent = (this->path_definition);
    if (parent == 0 || ((((const PathDefinition *)(parent)))->transform) == 0)
        goto local_position;
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
    ApplyRotMatrixLV(&input.value, &position.value);
    PopMatrix();
    path_transform = (const PathTransform *)((((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) != 0 ? (((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) : (PathTransform *)0);
    x = position.words[0];
    position.words[0] = x + (uint32)path_transform->matrix.t[0];
    path_transform = (const PathTransform *)((((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) != 0 ? (((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) : (PathTransform *)0);
    y = position.words[1];
    position.words[1] = y + (uint32)path_transform->matrix.t[1];
    path_transform = (const PathTransform *)((((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) != 0 ? (((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) : (PathTransform *)0);
    z = position.words[2];
    position.words[2] = z + (uint32)path_transform->matrix.t[2];
    goto position_ready;
local_position:
    x = this->position[0];
    y = this->position[1];
    z = this->position[2];
    position.words[0] = x;
    position.words[1] = y;
    position.words[2] = z;
position_ready:
    this->spheres[0].position[0] = this->world_position[0] - position.words[0];
    this->spheres[0].position[1] = this->world_position[1] - position.words[1];
    this->spheres[0].position[2] = this->world_position[2] - position.words[2];
    if ((character_controller.flags & 0x80) != 0)
    {
        character_controller.timers.state_ticks = 70;
        character_controller.path_velocity.vz = 40960;
        if ((character_controller.secondary_flags & 0x80) == 0)
            character_controller.motion.state_code = 6;
        if ((this->flags & 0x2000) == 0)
        {
            parent = (this->path_definition);
            if (parent != 0 && ((((const PathDefinition *)(parent)))->transform) != 0)
            {
                x = this->position[0];
                y = this->position[1];
                z = this->position[2];
                death_input.words[0] = x;
                death_input.words[1] = y;
                death_input.words[2] = z;
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
                ApplyRotMatrixLV(&death_input.value, (VECTOR *)this->position);
                PopMatrix();
                parent = (this->path_definition);
                path_transform = (const PathTransform *)(((((const PathDefinition *)(parent)))->transform) != 0 ? ((((const PathDefinition *)(parent)))->transform) : (PathTransform *)0);
                x = this->position[0];
                y = (uint32)path_transform->matrix.t[0];
                parent = (this->path_definition);
                this->position[0] = x + y;
                path_transform = (const PathTransform *)(((((const PathDefinition *)(parent)))->transform) != 0 ? ((((const PathDefinition *)(parent)))->transform) : (PathTransform *)0);
                x = this->position[1];
                y = (uint32)path_transform->matrix.t[1];
                parent = (this->path_definition);
                this->position[1] = x + y;
                path_transform = (const PathTransform *)(((((const PathDefinition *)(parent)))->transform) != 0 ? ((((const PathDefinition *)(parent)))->transform) : (PathTransform *)0);
                z = this->position[2];
                this->position[2] = z + (uint32)path_transform->matrix.t[2];
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
            this->flags = this->flags | 0x2000;
        }
    }
    else if ((character_controller.secondary_flags & 0x80) == 0)
        character_controller.motion.state_code = 1;
    camera_motion.smooth_ticks[2] = (uint8)(0);
    camera_motion.smooth_ticks[1] = (uint8)(0);
    camera_motion.smooth_ticks[0] = (uint8)(0);
    x = camera_motion.position[0];
    y = camera_motion.position[1];
    z = camera_motion.position[2];
    camera_motion.desired_position[0] = x;
    camera_motion.desired_position[1] = y;
    camera_motion.desired_position[2] = z;
    return 0;
}

void AnimatedObject::UpdateCharacterFallTimeout()
{
    FUNCTION_MARKER(0x800524AC, "SLUS_005.78");
    const PathDefinition *parent;
    uint32 result, value, flags, mask, x, y, z;
    const PathTransform *path_transform;

    union
    {
        VECTOR value;
        uint32 words[4];
    } position, floor, input;

    sint32 elapsed;
    result = (uint32)(sint32)(sint8)character_controller.vitals.health;
    if (result == 0)
        return;
    value = character_controller.motion.vertical_speed;
    if (character_controller.movement.fall_speed_limit < (sint32)value)
        return;
    result = (uint16)(character_controller.motion.form_flags >> 16) & 15;
    if (result == 5)
        return;
    if ((this->flags & 0x2000) != 0)
        goto increment;
    parent = (this->path_definition);
    flags = ((const PathDefinition *)(parent))->flags;
    mask = flags & 0x11;
    result = flags & 0x10;
    if (mask != 1)
    {
        if (result == 0)
            goto done;
        const ScenePath *camera_path = camera_motion.path;
        const PathDefinition *camera_definition = camera_path->definition;
        if ((camera_definition->flags & 0x100010) != 0)
        {
            const ScenePath *fallback = camera_path->fallback;
            result = fallback != 0;
        }
        else
            result = camera_path->slope_scale;
        if (result != 0)
            goto done;
    }
    if (mask == 0)
        goto increment;
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
        ApplyRotMatrixLV(&input.value, &position.value);
        PopMatrix();
        path_transform = (const PathTransform *)((((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) != 0 ? (((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) : (PathTransform *)0);
        x = position.words[0];
        position.words[0] = x + (uint32)path_transform->matrix.t[0];
        path_transform = (const PathTransform *)((((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) != 0 ? (((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) : (PathTransform *)0);
        y = position.words[1];
        position.words[1] = y + (uint32)path_transform->matrix.t[1];
        path_transform = (const PathTransform *)((((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) != 0 ? (((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) : (PathTransform *)0);
        z = position.words[2];
        position.words[2] = z + (uint32)path_transform->matrix.t[2];
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
    elapsed = this->path_frame;
    flags = this->flags;
    parent = (this->path_definition);
    if ((((const PathDefinition *)(parent)))->ProjectFrame(elapsed, position.words, floor.words, flags & 0x10, 4096) >= 0 && (sint32)position.words[1] >= (sint32)floor.words[1])
        character_controller.timers.state_ticks = (uint32)character_controller.timers.state_ticks - 1;
increment:
    value = character_controller.timers.state_ticks + 1;
    character_controller.timers.state_ticks = value;
    result = (sint16)value < 16;
    if (result == 0)
        character_controller.vitals.health = 0;
done:
    return;
}

uint32 AnimatedObject::ResetCharacterForScene(sint32 mode)
{
    CharacterModelRecord *model_record;
    AnimatedObject *model_object;
    SpriteRecord *sprite;
    FUNCTION_MARKER(0x80048AF0, "SLUS_005.78");
    uint32 flags, value, packed, category, result = 1;
    sint32 selected;
    ResetHud();
    category = (character_controller.motion.form_flags >> 16) & 0xF;
    character_controller.motion.slope_scale = 0;
    {
        ObjectCallback operation = {0, category == 5 ? &AnimatedObject::UpdateForm5Movement : &AnimatedObject::UpdateCharacterMovement};
        object_operations.motions[1] = operation;
    }
    {
        ObjectCallback operation = {0, &AnimatedObject::UpdateCharacterBehavior};
        object_operations.behaviors[1] = operation;
    }
    model_record = character_controller.model;
    character_controller.motion.trail_effect = 0;
    model_object = ((AnimatedObject *)(model_record->object));
    sprite = (SpriteRecord *)(((SpriteRecord *)(model_object->sprite)));
    sprite->extent[0] = model_record->extent[0] >> 5;
    model_record = character_controller.model;
    model_object = ((AnimatedObject *)(model_record->object));
    sprite = (SpriteRecord *)(((SpriteRecord *)(model_object->sprite)));
    sprite->extent[1] = model_record->extent[1] >> 5;
    value = game_flow.transition_flags;
    flags = character_controller.flags;
    game_flow.transition_flags = value & 0xFFFFFEFF;
    character_controller.flags = flags & 0xBFFFFFFF;
    character_controller.ClearTransientFlags();
    character_controller.flags = character_controller.flags & 0xC0000000;
    flags = this->flags;
    value = this->path_flags;
    this->flags = flags & 0xC0F793FF;
    this->path_flags = value & 0xFC000000;
    flags = character_controller.secondary_flags;
    value = game_flow.mode_flags;
    character_controller.motion.jump_phase = 0;
    character_controller.motion.vertical_speed = 0;
    character_controller.motion.horizontal_speed = 0;
    character_controller.secondary_flags = flags & 0xFFFFF932;
    this->velocity[2] = 0;
    this->velocity[1] = 0;
    this->velocity[0] = 0;
    character_controller.timers.special_move_ticks = 0;
    character_controller.timers.transition_ticks = 0;
    character_controller.timers.score_bonus_ticks = 0;
    character_controller.timers.action_cooldown_ticks = 0;
    character_controller.vitals.damage_ticks = 0;
    character_controller.timers.state_ticks = 0;
    character_controller.section_height_offset = 0;
    character_controller.timers.contact_impulse_ticks = 0;
    character_controller.motion.environment_acceleration = 0;
    if ((value & 0x10) == 0 && mode != 0)
    {
        character_controller.weapon_state = 0;
    }
    if (mode == 2)
    {
        character_controller.vitals.lives = 3;
        character_controller.vitals.health_limit = 4;
        character_controller.vitals.health = 4;
        character_controller.vitals.score = 0;
        if ((character_controller.secondary_flags & 0x80) == 0)
            character_controller.motion.state_code = 0;
        result = game_flow.transition_flags | 0x20000;
        game_flow.transition_flags = result;
    }
    else if (mode == 1)
    {
        value = GetSectionRespawnForm(scene_stream.active_section);
        character_controller.vitals.health = value;
        selected = (sint8)value;
        if (selected == -1)
        {
            packed = character_controller.motion.form_flags & 0xFFF0FFFF;
            category = ((packed >> 20) & 0xF) << 16;
        }
        else
        {
            value = ResolveCharacterFormVariant(selected);
            character_controller.vitals.health = value;
            packed = character_controller.motion.form_flags;
            selected = (sint8)character_controller.vitals.health;
            packed &= 0xFFF0FFFF;
            category = (uint32)selected << 16;
            character_controller.motion.form_flags = packed;
        }
        character_controller.motion.form_flags = packed | category;
        this->flags = this->flags | 0x10;
        character_controller.vitals.health = character_controller.vitals.health_limit;
        if ((character_controller.secondary_flags & 0x80) == 0)
            character_controller.motion.state_code = 0;
        SetSceneTransition(0, scene_stream.active_section, -2, 0);
        result = 15;
        screen_render.transition_ticks = result;
    }
    return result;
}

uint32 AnimatedObject::UpdateCharacterDeathTransition()
{
    FUNCTION_MARKER(0x80048DB4, "SLUS_005.78");
    ActorBinding *binding;
    const AnimationHeader *animation;
    uint32 value, scene, x, y, z, fade, frame, buttons, result;
    sint32 step, position, limit;
    if (player_interaction.flags != 0)
    {
        player_interaction.flags = 0;
        player_interaction.target = 0;
    }
    if (character_action_flags != 0)
    {
        AnimatedObject *selected = character_action_target;
        character_action_flags = 0;
        if (selected != 0)
        {
            binding = (ActorBinding *)(((const ActorPlacement *)((const PathDefinition *)((selected->path_definition))))->state != 0 ? ((const ActorPlacement *)((const PathDefinition *)((selected->path_definition))))->state : 0);
            value = binding->flags;
            binding->flags = value & 0xF7;
        }
    }
    if (((character_controller.motion.form_flags >> 16) & 15) == 1)
        character_controller.secondary_flags = character_controller.secondary_flags & 0xFFFFFFDF;
    else if ((game_flow.mode_flags & 0x200) == 0)
        character_controller.secondary_flags = character_controller.secondary_flags & 0xFFFFFEFF;
    scene = OriginalDefaults::SceneContextIdentity;
    camera_motion.smooth_ticks[2] = 0;
    camera_motion.smooth_ticks[1] = 0;
    camera_motion.smooth_ticks[0] = 0;
    scene = OriginalDefaults::SceneContextIdentity;
    x = camera_motion.position[0];
    y = camera_motion.position[1];
    z = camera_motion.position[2];
    camera_motion.desired_position[0] = x;
    camera_motion.desired_position[1] = y;
    camera_motion.desired_position[2] = z;
    if (this->animation == 0)
    {
        value = character_controller.timers.state_ticks - 1;
        character_controller.timers.state_ticks = value;
        if ((uint16)value != 0)
            return 0xE0000;
        camera_projection.scale = 0xE9555;
        SetCameraProjection(0xE9555);
        return this->ResetCharacterForScene(1);
    }
    fade = screen_render.fade_level;
    if (fade < 4096)
    {
        fade += 409;
        screen_render.fade_level = fade;
        if ((uint16)fade >= 4096)
        {
            screen_render.fade_level = 4096;
            ReleaseSceneObjects();
            this->node.Register();
        }
    }
    if ((character_controller.flags & 0x80) != 0)
    {
        if (screen_render.fade_level < 4096)
        {
            value = this->position[1];
            scene = OriginalDefaults::SceneContextIdentity;
            this->position[1] = value - 10240;
            x = this->position[0];
            y = this->position[1];
            z = this->position[2];
            camera_motion.focus_position[0] = x;
            camera_motion.focus_position[1] = y;
            camera_motion.focus_position[2] = z;
            UpdateCameraAim();
            scene = OriginalDefaults::SceneContextIdentity;
            camera_motion.flags = camera_motion.flags | 1;
        }
        if ((sint16)character_controller.timers.state_ticks != 0)
        {
            step = character_controller.path_velocity.vz;
            if (step > 0)
            {
                position = (sint32)camera_projection.scale;
                if (position <= 2457599)
                {
                    value = (uint32)position + (uint32)step;
                    camera_projection.scale = value;
                    if ((sint32)value > 2457600)
                        camera_projection.scale = 2457600;
                    SetCameraProjection((sint32)camera_projection.scale);
                }
                if (camera_projection.scale == 2457600)
                    character_controller.path_velocity.vz = 0 - (uint32)character_controller.path_velocity.vz;
            }
            else
            {
                position = (sint32)camera_projection.scale;
                if (position > 0)
                {
                    value = (uint32)position + (uint32)step;
                    camera_projection.scale = value;
                    if ((sint32)value < 0)
                        camera_projection.scale = 0;
                    SetCameraProjection((sint32)camera_projection.scale);
                }
            }
            character_controller.timers.state_ticks = character_controller.timers.state_ticks - 1;
            animation = this->animation;
            frame = 0;
            if (animation != 0)
            {
                position = this->frame;
                limit = animation->frame_count - 1;
                if (position < limit)
                    frame = (uint32)position + 1;
            }
            this->frame = frame;
            return this->ClampAnimationFrame();
        }
        character_controller.flags = character_controller.flags & 0xFFFFFF7F;
    }
    else
    {
        position = (sint32)camera_projection.scale;
        if (position <= 2457599)
        {
            value = (uint32)position + 40960;
            camera_projection.scale = value;
            if ((sint32)value > 2457600)
                camera_projection.scale = 2457600;
            SetCameraProjection((sint32)camera_projection.scale);
        }
        frame = (uint16)this->frame;
        animation = (const AnimationHeader *)(this->animation);
        ++frame;
        this->frame = frame;
        limit = animation->frame_count;
        if ((sint16)frame < limit)
            return this->ClampAnimationFrame();
    }
    this->animation = 0;
    StopCurrentSequence();
    result = PrepareSavedSequence();
    buttons = game_flow.transition_flags;
    sequence_playback.current_handle = result;
    if ((buttons & 0x10000) != 0)
    {
        value = level_resource.level_id;
        if (value - 96 >= 2)
        {
            value = character_controller.vitals.lives - 1;
            character_controller.vitals.lives = value;
            if ((uint8)value == 0)
                return this->ResetCharacterForScene(2);
        }
    }
    StartCurrentSequence();
    scene = OriginalDefaults::SceneContextIdentity;
    character_controller.timers.state_ticks = 15;
    screen_render.transition_ticks = -15;
    return scene;
}

uint32 GetCurrentCharacterForm(void)
{
    FUNCTION_MARKER(0x80044780, "SLUS_005.78");
    return (uint16)(character_controller.motion.form_flags >> 16) & 15;
}

uint32 GetSceneHealthPickupFlag(uint32 scene)
{
    FUNCTION_MARKER(0x80055848, "SLUS_005.78");
    if (scene == 20)
        return character_controller.vitals.health_pickups & 4;
    if (scene == 75)
        return character_controller.vitals.health_pickups & 1;
    if (scene == 10)
        return character_controller.vitals.health_pickups & 2;
    return 1;
}
