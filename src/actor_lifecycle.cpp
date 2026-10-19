#include "game.h"
#include "original_defaults.h"

ActorModelDefinition *ActorPlacement::InitializeBinding()
{
    FUNCTION_MARKER(0x80034658, "SLUS_005.78");
    void *data;
    ActorModelDefinition *table;
    ActorModelDefinition *selected = 0;
    uint32 value, count, frame;
    sint32 kind;
    value = this->path.node.flags;
    data = this->state;
    ActorBinding *binding = (ActorBinding *)(data);
    this->path.node.flags = value & 0x7F;
    count = (uint32)(sint32)(sint8)actor_models.count;
    frame = binding->initial_path_frame;
    kind = (sint8)binding->model_kind;
    binding->instance = 0;
    binding->distance_squared = 0x7FFFF000;
    binding->reaction_flags = 0;
    binding->path_frame = frame;
    table = actor_models.definitions;
    while (count != 0)
    {
        if (table->kind == kind)
        {
            selected = table;
            break;
        }
        --count;
        ++table;
    }
    binding->model = selected;
    return selected;
}

void ActorPlacement::CreateBoundActor()
{
    FUNCTION_MARKER(0x800346C8, "SLUS_005.78");
    void *data;
    const ActorModelDefinition *resource_model;
    uint32 value;
    AnimatedObject *prototype;
    SpriteRecord *sprite;
    SceneSection *section;
    AnimatedObject *actor;
    data = this->state;
    ActorBinding *binding = (ActorBinding *)(data);
    resource_model = binding->model;
    prototype = resource_model->prototype;
    if (prototype == 0 || binding->instance != 0)
        return;
    if ((prototype->node.flags & 0x80) != 0)
    {
        actor = prototype;
        prototype->node.links.sibling_next = 0;
    }
    else
    {
        AnimatedObject *created = CloneObjectResource(prototype);
        if (created == 0)
            return;
        actor = created;
        created->flags &= 0xFFFFFFFB;
    }
    if ((actor->flags & 0x8000) == 0)
    {
        if ((((const ActorBinding *)(this->state))->flags & 1) != 0)
        {
            sprite = (SpriteRecord *)(((SpriteRecord *)(actor->sprite)));
            sprite->extent[1] = 0;
            sprite->extent[0] = 0;
            sprite = (SpriteRecord *)(((SpriteRecord *)(actor->sprite)));
            value = sprite->node.flags | 1;
        }
        else
        {
            value = ((uint32)resource_model->sprite_extent[0]);
            sprite = (SpriteRecord *)(((SpriteRecord *)(actor->sprite)));
            sprite->extent[0] = (sint32)value >> 5;
            value = ((uint32)resource_model->sprite_extent[1]);
            sprite = (SpriteRecord *)(((SpriteRecord *)(actor->sprite)));
            sprite->extent[1] = (sint32)value >> 5;
            sprite = (SpriteRecord *)(((SpriteRecord *)(actor->sprite)));
            value = sprite->node.flags & 0xFE;
        }
        sprite->node.flags = value;
    }
    actor->path_definition = &this->path;
    actor->ResetActorInstance();
    binding->instance = actor;
    binding->sound_voice = 0xFFFF;
    section = scene_stream.active_section;
    actor->node.links.sibling_next = section->transient_links.first_child;
    section->transient_links.first_child = &actor->node;
}

void ActorPlacement::ReleaseBoundActor()
{
    FUNCTION_MARKER(0x80034848, "SLUS_005.78");
    void *data;
    const SoundSample *sound;
    sint32 frame, count;
    AnimatedObject *actor;
    data = this->state;
    ActorBinding *binding = (ActorBinding *)(data);
    actor = binding->instance;
    if (actor == 0)
        return;
    sound = binding->sound;
    if ((sound_effects.IsPlaying(sound, (uint32)(sint16)binding->sound_voice) & 0xFF) != 0)
        StopSoundVoice(binding->sound_voice);
    binding->sound = 0;
    binding->sound_voice = 0xFFFF;
    binding->instance = 0;
    if (actor->flags & 0x10)
        binding->path_frame = (uint16)actor->path_frame;
    else
    {
        count = this->path.positions.metadata6;
        frame = (uint16)actor->path_frame;
        binding->path_frame = (uint32)count - (uint32)frame - 1;
    }
    frame = (sint16)binding->path_frame;
    if (frame < 0)
        binding->path_frame = 0;
    else
    {
        count = (sint16)this->path.positions.metadata6;
        if (frame >= count)
            binding->path_frame = (uint32)count - 1;
    }
    binding->distance_squared = 0x7FFFF000;
    actor->node.flags |= 2;
}

uint32 ActorPlacement::UpdateEvents()
{
    FUNCTION_MARKER(0x8005BCFC, "SLUS_005.78");
    void *data;
    const uint32 *events;
    AnimatedObject *target;
    uint32 flags, command, kind, value, found, key, count, match;
    flags = this->path.flags;
    data = this->state;
    ActorBinding *binding = (ActorBinding *)(data);
    if ((flags & 4) != 0)
    {
        this->path.flags = flags & 0xFFFFFFFB;
        command = binding->event_command;
        if ((command & 0xFFF) != 0)
        {
            kind = (command >> 12) & 31;
            if ((binding->action_effect_flags & 0x100) != 0)
                binding->pending_event = kind + 39;
            switch (kind)
            {
                case 1:
                    this->path.flags = this->path.flags | 0x40;
                    break;
                case 3:
                    if ((this->path.node.flags & 1) == 0)
                    {
                        this->ReleaseBoundActor();
                        value = this->path.node.flags;
                        flags = this->path.flags;
                        this->path.node.flags = value | 1;
                        this->path.flags = flags | 1;
                    }
                    break;
                case 5:
                    this->path.flags = this->path.flags & 0xFFFFFFBF;
                    break;
                case 8:
                    binding->path_frame = binding->initial_path_frame;
                    this->ReleaseBoundActor();
                    value = this->path.node.flags;
                    flags = this->path.flags;
                    this->path.node.flags = value | 1;
                    if ((flags & 0x20) != 0)
                    {
                        this->path.flags = flags | 1;
                        break;
                    }
                case 4:
                    if ((this->path.node.flags & 1) != 0)
                    {
                        this->CreateBoundActor();
                        if (kind == 8)
                        {
                            target = binding->instance;
                            if (target != 0)
                            {
                                target->scale = 4096;
                                target = ((AnimatedObject *)(binding->instance));
                                target->orientation[0] = 0x4000;
                                target = ((AnimatedObject *)(binding->instance));
                                target->orientation[3] = 0;
                                target->orientation[2] = 0;
                                target->orientation[1] = 0;
                            }
                        }
                        value = this->path.node.flags;
                        flags = this->path.flags;
                        this->path.node.flags = value & 0xFE;
                        this->path.flags = flags & 0xFFFFFFFE;
                    }
                    break;
            }
        }
        return 4;
    }
    found = 0;
    if ((sint8)scene_events.count != 0)
    {
        key = binding->event_command & 0xFFF;
        if (key != 0)
        {
            binding->event_command = key;
            command = scene_events.entries[0];
            count = (uint32)(sint32)(sint8)scene_events.count;
            events = scene_events.entries + 1;
            for (;;)
            {
                match = command & 0xFFF;
                --count;
                if (key == match)
                {
                    binding->event_command = command;
                    found = 1;
                    break;
                }
                if (count == 0)
                    break;
                command = *events;
                ++events;
            }
        }
    }
    if (found != 0)
        this->path.flags = this->path.flags | 4;
    else if ((this->path.flags & 1) != 0 || (sint32)(0 - (binding->distance_squared + 2)) < 0)
    {
        if (binding->instance != 0)
            this->ReleaseBoundActor();
    }
    else if (binding->instance == 0)
        this->CreateBoundActor();
    binding->distance_squared = 0x7FFFF000;
    return 0x7FFFF000;
}

uint32 AnimatedObject::ResetActorInstance()
{
    AnimationHeader *initial_animation;
    FUNCTION_MARKER(0x80033D7C, "SLUS_005.78");
    void *data;
    ActorModelDefinition *table;
    uint32 flags, count, value, x, y, z;
    const PathDefinition *parent;
    const ActorBinding *kind_binding;
    uint32 pos0[3], pos1[3];
    uint32 result;
    ActorModelDefinition *definition = 0;
    sint32 kind, frame;
    parent = (this->path_definition);
    flags = this->flags;
    data = ((const ActorPlacement *)((const PathDefinition *)(parent)))->state;
    ActorBinding *binding = (ActorBinding *)(data);
    parent = (this->path_definition);
    this->flags = (flags & 0x01C08024) | 0x20010;
    kind_binding = (const ActorBinding *)(((const ActorPlacement *)((const PathDefinition *)(parent)))->state != 0 ? ((const ActorPlacement *)((const PathDefinition *)(parent)))->state : 0);
    count = (uint32)(sint32)(sint8)actor_models.count;
    kind = (sint8)kind_binding->model_kind;
    table = actor_models.definitions;
    while (count != 0)
    {
        value = (uint32)(sint32)table->kind;
        --count;
        if ((sint32)value == kind)
        {
            definition = table;
            break;
        }
        ++table;
    }
    initial_animation = (definition->animations[0]);
    this->frame = -1;
    this->movement_mode = 4;
    this->metadata126[0] = 2;
    this->metadata126[1] = 0;
    this->animation = initial_animation;
    this->unbound_animation_word = 0;
    value = (uint8)definition->initial_health;
    parent = (this->path_definition);
    this->ticks = (sint8)value;
    if (((const PathDefinition *)((const PathDefinition *)(parent)))->positions.keys != 0)
    {
        value = (uint32)SampleDefinitionHeading(0, 1, (const PathDefinition *)((const PathDefinition *)(parent)));
        this->target_yaw = (sint16)(value);
        this->yaw = (sint16)(value);
        binding->path_heading = value;
        parent = (this->path_definition);
        ((const PathDefinition *)((const PathDefinition *)(parent)))->SampleActorPosition(0, pos0, 64);
        parent = (this->path_definition);
        ((const PathDefinition *)((const PathDefinition *)(parent)))->SampleActorPosition(1, pos1, 64);
        x = pos1[0];
        x -= pos0[0];
        y = pos1[1];
        y -= pos0[1];
        z = pos1[2];
        z -= pos0[2];
        binding->path_direction[0] = x;
        binding->path_direction[1] = y;
        binding->path_direction[2] = z;
    }
    else
    {
        this->yaw = (sint16)(0);
        binding->path_heading = 0;
        binding->path_direction[2] = 0;
        binding->path_direction[1] = 0;
        binding->path_direction[0] = 0;
        binding->path_direction[2] = 4096;
    }
    if (definition->turn_motion_mode == 0)
        this->yaw = (sint16)(random_generator.Next() & 0xFFF);
    parent = (this->path_definition);
    if (((const PathDefinition *)((const PathDefinition *)(parent)))->positions.keys != 0)
    {
        const ActorBinding *frame_binding = (const ActorBinding *)(((const ActorPlacement *)((const PathDefinition *)(parent)))->state != 0 ? ((const ActorPlacement *)((const PathDefinition *)(parent)))->state : 0);
        const PathDefinition *path_definition = (const PathDefinition *)((const PathDefinition *)((this->path_definition)));
        value = frame_binding->initial_path_frame;
        frame = (sint16)value;
        this->path_frame = (sint16)(value);
        SampleVectorTrack(frame, &path_definition->positions, this->position);
    }
    else
    {
        const PathDefinition *path_definition = (const PathDefinition *)((const PathDefinition *)(parent));
        const PathTransform *captured_path_matrix = path_definition->transform;
        if (captured_path_matrix != 0)
        {
            x = (uint32)captured_path_matrix->matrix.t[0];
            y = (uint32)captured_path_matrix->matrix.t[1];
            z = (uint32)captured_path_matrix->matrix.t[2];
        }
        else
        {
            x = path_definition->static_position[0];
            y = path_definition->static_position[1];
            z = path_definition->static_position[2];
        }
        this->position[0] = x;
        this->position[1] = y;
        this->position[2] = z;
    }
    value = this->node.flags;
    this->spheres[3].radius = 0xFFFFFFFF;
    this->spheres[2].radius = 0xFFFFFFFF;
    this->spheres[1].radius = 0xFFFFFFFF;
    this->spheres[0].radius = 0xFFFFFFFF;
    this->node.flags = (value & 0x7D) | 0x40;
    if ((definition->path_flag) != 0)
        this->path_flags = this->path_flags | 0x4000;
    value = binding->flags;
    binding->interaction_timer = 0;
    binding->interaction_height = 0;
    binding->route_direction_mode = 0;
    binding->sampled_velocity[2] = 0;
    binding->sampled_velocity[1] = 0;
    binding->sampled_velocity[0] = 0;
    binding->flags = value & 0xE5;
    InitializeActorAction(binding);
    result = (this->*definition->constructor)();
    return result;
}

uint32 AnimatedObject::UpdateActorDeath()
{
    FUNCTION_MARKER(0x80034044, "SLUS_005.78");
    void *data;
    const ActorModelDefinition *animation_model;
    uint32 timer, value, count, frame, result;
    AnimationHeader *animation, *selected_animation;
    const PathDefinition *resource;
    data = ((const ActorPlacement *)((const PathDefinition *)((this->path_definition))))->state;
    ActorBinding *binding = (ActorBinding *)(data);
    if (this == character_action_target)
        character_action_flags = character_action_flags | 0x800;
    timer = this->death_ticks - 1;
    this->death_ticks = timer;
    if (timer == 0)
    {
        if ((game_flow.mode_flags & 0x100) && (sint8)binding->model_kind < 44)
            return this->ResetActorInstance();
        value = binding->respawn_delay;
        AnimatedObject *action_target = character_action_target;
        binding->respawn_ticks = value;
        if (this == action_target)
            character_action_target = 0;
        resource = (this->path_definition);
        ((PathDefinition *)((const PathDefinition *)(resource)))->node.flags |= 0x80;
        result = this->node.flags | 2;
        this->node.flags = result;
        return result;
    }
    if (this->behavior == 17)
    {
        result = (uint16)this->frame + 1;
        this->frame = result;
        render_view.material_override = resource_groups[25].texture;
        return result;
    }
    if ((sint32)timer < 15)
    {
        animation_model = binding->model;
        animation = this->animation;
        selected_animation = (animation_model->animations[6]);
        if (animation != selected_animation)
        {
            this->flags = this->flags ^ 0x80000;
            animation_model = binding->model;
            animation = this->animation;
            selected_animation = (animation_model->animations[6]);
            if (animation != selected_animation)
                this->velocity[0] = (uint32)(((sint64)(sint32)this->velocity[0] * 3513) >> 12);
            return this->ClampAnimationFrame();
        }
    }
    frame = (uint16)this->frame + 1;
    animation = this->animation;
    this->frame = frame;
    count = (uint32)((AnimationHeader *)(animation))->frame_count;
    if ((sint32)(sint16)frame >= (sint32)count)
        this->frame = count - 1;
    return this->ClampAnimationFrame();
}

uint32 ActorBinding::ComputeInteractionDistance()
{
    FUNCTION_MARKER(0x800343F0, "SLUS_005.78");

    union
    {
        VECTOR vector;
        uint32 words[4];
    } pos, input;

    uint32 scene, x, y, z;
    const ActorPlacement *definition;
    const PathDefinition *path;
    const MATRIX *matrix;
    const AnimatedObject *actor;
    this->distance_squared = 0x7FFFF000;
    if (FindPlayerObject(0) == 0)
        return this->distance_squared;
    actor = this->instance;
    if (actor != 0)
    {
        definition = this->definition;
        path = definition != 0 ? &definition->path : 0;
        if (path != 0 && (path->transform) != 0)
        {
            x = actor->position[0];
            y = actor->position[1];
            z = actor->position[2];
            input.words[0] = x;
            input.words[1] = y;
            input.words[2] = z;
            PushMatrix();
            path = this->definition != 0 ? &this->definition->path : (const PathDefinition *)0;
            matrix = (const MATRIX *)((path->transform) != 0 ? &(path->transform)->matrix : (MATRIX *)0);
            x = xport_load_le32((const uint8 *)matrix);
            y = xport_load_le32((const uint8 *)matrix + 4);
            xport_gte_write_control(0, x);
            xport_gte_write_control(1, y);
            x = xport_load_le32((const uint8 *)matrix + 8);
            y = xport_load_le32((const uint8 *)matrix + 12);
            z = xport_load_le32((const uint8 *)matrix + 16);
            xport_gte_write_control(2, x);
            xport_gte_write_control(3, y);
            xport_gte_write_control(4, z);
            ApplyRotMatrixLV(&input.vector, &pos.vector);
            PopMatrix();
            path = this->definition != 0 ? &this->definition->path : (const PathDefinition *)0;
            matrix = (const MATRIX *)((path->transform) != 0 ? &(path->transform)->matrix : (MATRIX *)0);
            x = pos.words[0];
            x += (uint32)matrix->t[0];
            pos.words[0] = x;
            path = this->definition != 0 ? &this->definition->path : (const PathDefinition *)0;
            matrix = (const MATRIX *)((path->transform) != 0 ? &(path->transform)->matrix : (MATRIX *)0);
            y = pos.words[1];
            y += (uint32)matrix->t[1];
            pos.words[1] = y;
            path = this->definition != 0 ? &this->definition->path : (const PathDefinition *)0;
            matrix = (const MATRIX *)((path->transform) != 0 ? &(path->transform)->matrix : (MATRIX *)0);
            z = pos.words[2];
            z += (uint32)matrix->t[2];
            pos.words[2] = z;
        }
        else
        {
            actor = (const AnimatedObject *)(this->instance);
            x = actor->position[0];
            y = actor->position[1];
            z = actor->position[2];
            pos.words[0] = x;
            pos.words[1] = y;
            pos.words[2] = z;
        }
    }
    else
    {
        definition = this->definition;
        path = definition != 0 ? &definition->path : (const PathDefinition *)0;
        if (path->positions.keys != 0)
            path->SamplePosition((sint16)this->path_frame, pos.words, 1);
        else
        {
            const PathTransform *captured_path_matrix = path->transform;
            if (captured_path_matrix != 0)
            {
                x = (uint32)captured_path_matrix->matrix.t[0];
                y = (uint32)captured_path_matrix->matrix.t[1];
                z = (uint32)captured_path_matrix->matrix.t[2];
            }
            else
            {
                x = path->static_position[0];
                y = path->static_position[1];
                z = path->static_position[2];
            }
            pos.words[0] = x;
            pos.words[1] = y;
            pos.words[2] = z;
        }
    }
    scene = OriginalDefaults::SceneContextIdentity;
    x = pos.words[0];
    x -= camera_motion.focus_position[0];
    y = pos.words[1];
    y -= camera_motion.focus_position[1];
    z = pos.words[2];
    z -= camera_motion.focus_position[2];
    x = (uint32)((sint32)x >> 12);
    y = (uint32)((sint32)y >> 12);
    z = (uint32)((sint32)z >> 12);
    this->distance_squared = x * x + y * y + z * z;
    return this->distance_squared;
}

uint32 AnimatedObject::GetActorContactDamage() const
{
    FUNCTION_MARKER(0x80034EAC, "SLUS_005.78");
    void *data;
    ActorModelDefinition *table;
    uint32 count, kind;
    ActorModelDefinition *definition = 0;
    data = ((const ActorPlacement *)((const PathDefinition *)((this->path_definition))))->state;
    ActorBinding *binding = (ActorBinding *)(data);
    count = (uint32)(sint32)(sint8)actor_models.count;
    kind = binding->model_kind;
    table = actor_models.definitions;
    while (count != 0)
    {
        if ((sint32)table->kind == (sint32)kind)
        {
            definition = table;
            break;
        }
        --count;
        ++table;
    }
    return definition->contact_damage;
}

uint32 ActorBinding::StopSound()
{
    FUNCTION_MARKER(0x80030BD4, "SLUS_005.78");
    const SoundSample *sound = this->sound;
    uint32 voice = (uint32)(sint32)(sint16)sound_voice;
    uint32 result = sound_effects.IsPlaying(sound, voice);
    if ((result << 24) != 0)
        StopSoundVoice(sound_voice);
    this->sound = 0;
    sound_voice = -1;
    return 0xFFFFFFFF;
}

uint32 ActorBinding::UpdateSound(AnimatedObject *object, const SoundSample *sound, uint32 mode)
{
    FUNCTION_MARKER(0x80032048, "SLUS_005.78");
    const SoundSample *previous = this->sound;
    uint32 pitch, rate, random, result;
    sint32 scale = 4096;
    if (sound != previous && previous != 0)
    {
        if ((sound_effects.IsPlaying(previous, (uint32)(sint32)(sint16)sound_voice) & 255) != 0)
            StopSoundVoice(sound_voice);
        this->sound = 0;
        sound_voice = -1;
    }
    if ((sint16)sound_voice < 0)
    {
        rate = ((const SoundSample *)(sound))->pitch;
        if (rate == 0)
            return p2_sdk_80082a18("Monster type %d bad SFX rate.\n", (sint8)model_kind);
        random = random_generator.Next();
        if ((rate >> 2) == 0)
            xport_mips_break(7);
        pitch = rate + random % (rate >> 2) - (rate >> 3);
        sound_pitch = pitch;
    }
    else
        pitch = sound_pitch;
    if ((object->flags & 0x30000000) != 0)
    {
        rate = (uint16)object->state;
        if (rate < 2048)
            rate = 2048;
        scale = (sint32)rate;
        pitch = (uint32)(((sint64)(sint32)pitch * (0x1000000 / (sint32)rate)) >> 12);
    }
    this->sound = sound;
    result = PlaySpatialSound(sound, scale + 4096, 0, pitch, (const uint32 *)object->world_position, (uint32)model->sound_radius << 5, (uint32)(sint32)(sint16)sound_voice, mode, mode != 0);
    sound_voice = result;
    return result;
}

void NoOp80034BA0(void)
{
    FUNCTION_MARKER(0x80034BA0, "SLUS_005.78");
    /* Original function returns without side effects */
}

uint32 LoadMonsterScriptEntry(void)
{
    FUNCTION_MARKER(0x800352CC, "SLUS_005.78");
    return p2_sdk_80082a18("LSC code should not be calling load_monster_script_entry\n");
}
