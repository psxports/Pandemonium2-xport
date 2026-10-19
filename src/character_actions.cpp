#include <stdlib.h>

#include "game.h"
#include "original_defaults.h"

unsigned int character_action_flags = OriginalDefaults::CharacterActionFlags;
ActionEffectResources action_effect_resources = {0};
AnimatedObject *character_action_owner = 0;
AnimatedObject *character_action_target = 0;
uint32 character_action_target_score = OriginalDefaults::CharacterActionTargetScore;
unsigned int character_sound_resume_flags = OriginalDefaults::CharacterSoundResumeFlags;

static const sint8 action_start_mode = 1;
static const sint8 hold_interaction_timeout = 1;
static const sint8 raise_completed_target = 1;
static const sint8 pause_interaction_actions = 0;

union ActionVector
{
    VECTOR vector;
    uint32 words[4];
};

static uint32 ActionDistance(const uint32 *first, const uint32 *second)
{
    FUNCTION_MARKER(0x80028CC0, "SLUS_005.78");
    uint32 x = second[0] - first[0];
    uint32 y = second[1] - first[1];
    uint32 z = second[2] - first[2];
    if ((sint32)x < 0)
        x = 0u - x;
    if ((sint32)y < 0)
        y = 0u - y;
    if ((sint32)z < 0)
        z = 0u - z;
    return 2 * (x + z) + y;
}

uint32 CharacterController::ClearTransientFlags()
{
    FUNCTION_MARKER(0x8001C684, "SLUS_005.78");
    uint32 result = flags & 0xFFFE7FBF;
    flags = result;
    return result;
}

void CharacterController::RemoveAction(uint32 action)
{
    FUNCTION_MARKER(0x80027AC0, "SLUS_005.78");
    AnimatedObject *effect = action_effects[action];
    if (effect != 0)
    {
        uint8 flags = effect->node.flags;
        effect->behavior = 0;
        effect->node.flags = flags | 2;
        action_effects[action] = 0;
    }
}

uint32 CharacterController::CancelCountedActions(uint32 incoming_flags)
{
    FUNCTION_MARKER(0x8005A480, "SLUS_005.78");
    sint32 index;
    if ((incoming_flags & 0x10) != 0)
    {
        for (index = 0; index < action13_count; ++index)
            RemoveAction(13);
        action13_count = 0;
    }
    if ((character_action_flags & 0x20) != 0)
    {
        for (index = 0; index < action14_count; ++index)
            RemoveAction(14);
        action14_count = 0;
    }
    incoming_flags = character_action_flags & 0xFFFFFFCF;
    character_action_flags = incoming_flags;
    return incoming_flags;
}

uint32 SpawnActionEffect(AnimatedObject *object, const void *context, uint32 action)
{
    const PathTransform *captured_path_transform;
    FUNCTION_MARKER(0x80027B1C, "SLUS_005.78");
    uint32 output;
    uint16 heading;
    ActionVector direction;
    ActionVector world, other_world, input, inverse;
    ActionVector *destination;

    union
    {
        uint32 words[5];
        uint16 halves[10];
    } matrix_local;

    const PathDefinition *parent;
    uint32 flags, value, x, y, z, a, b, c;
    const PathTransform *path_transform;
    const AnimatedObject *owner;
    uint32 pass, passes, global_source, index, result;
    sint32 mode;
    result = action << 2;
    AnimatedObject *effect = action_effect_resources.templates[action];
    if (effect == 0)
        return result;
    if ((effect->node.flags & 0x80) != 0)
    {
        value = effect->node.flags;
        effect->node.links.sibling_next = 0;
        effect->node.flags = value & 0x7F;
    }
    else
    {
        AnimatedObject *created = CloneObjectResource(effect);
        if (created == 0)
            return 0;
        effect = created;
        created->flags &= 0xFFFFFFFB;
    }
    flags = effect->flags;
    effect->frame = 0;
    effect->action_context = 0;
    effect->state = action;
    effect->flags = flags | 0x20000;
    if (action == 10)
    {
        x = object->action_ox;
        y = object->action_oy;
        z = object->action_oz;
        effect->position[0] = x;
        effect->position[1] = y;
        effect->position[2] = z;
    }
    else
    {
        x = object->world_position[0];
        y = object->world_position[1];
        z = object->world_position[2];
        effect->position[0] = x;
        effect->position[1] = y;
        effect->position[2] = z;
        if (context != 0 && action != 2)
        {
            NoOp8005927C();
            mode = (sint8)character_controller.motion.state_code;
            if (mode >= 10 && mode < 13)
                NoOp80059ACC();
            else if (mode == 20)
                NoOp80059AC4();
        }
        switch (action)
        {
            case 0:
                x = effect->position[1];
                value = object->metadata_words72[1];
                effect->position[1] = x + value;
                break;
            case 1:
                x = effect->position[1];
                value = object->metadata_words72[1];
                effect->position[1] = x - value;
                break;
            case 2:
                effect->action_context = (const ActorBinding *)context;
                value = random_generator.Next() & 0x7FF;
                effect->velocity[0] = value - 1024;
                effect->velocity[0] = value >= 1024 ? value + 1024 : value - 3072;
                value = (random_generator.Next() & 0x3FF) + 2048;
                effect->velocity[1] = value;
                value = random_generator.Next() & 0x7FF;
                effect->velocity[2] = value - 1024;
                effect->velocity[2] = value >= 1024 ? value + 1024 : value - 3072;
                break;
            case 5:
            case 8:
                break;
            case 9:
            case 12:
                x = effect->position[0];
                y = effect->position[1];
                z = effect->position[2];
                effect->world_position[0] = x;
                effect->world_position[1] = y;
                effect->world_position[2] = z;
                effect->metadata_words72[0] = 12288;
                effect->frame = 0;
                effect->path_frame = 0;
                if (action == 12)
                {
                    character_controller.action_effects[action] = effect;
                    effect->flags = effect->flags | 0x8000;
                }
                break;
            case 11:
                mode = action_start_mode;
                effect->velocity[2] = 0;
                if (mode == 1)
                {
                    owner = FindPlayerObject(0);
                    flags = owner->flags;
                    mode = owner->path_frame;
                    SampleObjectPathDirection(&heading, direction.words, owner, mode, flags & 0x10);
                    effect->TurnObjectFacing(direction.words, 4096);
                }
                else if (mode == 2)
                {
                    MATRIX *current_matrix = &render_view.matrix_cursor->matrix;
                    ApplyRotMatrixLV((const VECTOR *)((const SpriteRecord *)effect)->position, (VECTOR *)current_matrix->t);
                    current_matrix = &render_view.matrix_cursor->matrix;
                    const MATRIX *parent_matrix = (render_view.matrix_cursor->prev != 0 ? &render_view.matrix_cursor->prev->matrix : 0);
                    x = (uint32)current_matrix->t[0];
                    a = (uint32)parent_matrix->t[0];
                    y = (uint32)current_matrix->t[1];
                    b = (uint32)parent_matrix->t[1];
                    z = (uint32)current_matrix->t[2];
                    c = (uint32)parent_matrix->t[2];
                    current_matrix->t[0] = (sint32)(x + a);
                    current_matrix->t[1] = (sint32)(y + b);
                    current_matrix->t[2] = (sint32)(z + c);
                    effect->node.DrawBillboard();
                }
                break;
            case 13:
            case 14:
                owner = FindPlayerObject(0);
                passes = action == 13 ? 2 : 1;
                for (pass = 0; pass < passes; ++pass)
                {
                    global_source = action == 14 || pass != 0;
                    const AnimatedObject *source_object = global_source != 0 ? character_action_target : owner;
                    destination = pass == 0 ? &world : &other_world;
                    parent = (source_object->path_definition);
                    if (parent != 0 && ((((const PathDefinition *)(parent)))->transform) != 0)
                    {
                        x = source_object->position[0];
                        y = source_object->position[1];
                        z = source_object->position[2];
                        input.words[0] = x;
                        input.words[1] = y;
                        input.words[2] = z;
                        PushMatrix();
                        if (global_source != 0)
                        {
                            source_object = character_action_target;
                        }
                        path_transform = (const PathTransform *)((((const PathDefinition *)((const PathDefinition *)((source_object->path_definition))))->transform) != 0 ? (((const PathDefinition *)((const PathDefinition *)((source_object->path_definition))))->transform) : (PathTransform *)0);
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
                        ApplyRotMatrixLV(&input.vector, &destination->vector);
                        PopMatrix();
                        if (global_source != 0)
                        {
                            source_object = character_action_target;
                        }
                        for (index = 0; index < 3; ++index)
                        {
                            path_transform = (const PathTransform *)((((const PathDefinition *)((const PathDefinition *)((source_object->path_definition))))->transform) != 0 ? (((const PathDefinition *)((const PathDefinition *)((source_object->path_definition))))->transform) : (PathTransform *)0);
                            value = destination->words[index];
                            x = (uint32)path_transform->matrix.t[index];
                            destination->words[index] = value + x;
                        }
                    }
                    else
                    {
                        if (global_source != 0)
                        {
                            source_object = character_action_target;
                        }
                        x = source_object->position[0];
                        y = source_object->position[1];
                        z = source_object->position[2];
                        destination->words[0] = x;
                        destination->words[1] = y;
                        destination->words[2] = z;
                    }
                    if (pass == 0)
                    {
                        parent = (effect->path_definition);
                        path_transform = parent != 0 ? ((captured_path_transform = ((((const PathDefinition *)(parent)))->transform)) != 0 ? (const PathTransform *)((PathTransform *)(captured_path_transform)) : 0) : 0;
                        if (path_transform != 0)
                        {
                            for (index = 0; index < 3; ++index)
                            {
                                if (index != 0)
                                    path_transform = (const PathTransform *)((((const PathDefinition *)((const PathDefinition *)((effect->path_definition))))->transform) != 0 ? (((const PathDefinition *)((const PathDefinition *)((effect->path_definition))))->transform) : (PathTransform *)0);
                                value = world.words[index];
                                x = (uint32)path_transform->matrix.t[index];
                                inverse.words[index] = value - x;
                            }
                            for (index = 0; index < 9; ++index)
                            {
                                path_transform = (const PathTransform *)((((const PathDefinition *)((const PathDefinition *)((effect->path_definition))))->transform) != 0 ? (((const PathDefinition *)((const PathDefinition *)((effect->path_definition))))->transform) : (PathTransform *)0);
                                value = (uint16)path_transform->matrix.m[index % 3][index / 3];
                                matrix_local.halves[index] = value;
                            }
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
                            ApplyRotMatrixLV(&inverse.vector, (VECTOR *)effect->position);
                            PopMatrix();
                        }
                        else
                        {
                            x = world.words[0];
                            y = world.words[1];
                            z = world.words[2];
                            effect->position[0] = x;
                            effect->position[1] = y;
                            effect->position[2] = z;
                        }
                        effect->position[1] = effect->position[1] + 0x4000;
                    }
                }
                value = ActionDistance(action == 13 ? other_world.words : world.words, effect->position);
                effect->velocity[0] = value;
                value = action == 13 ? character_controller.action13_scale : character_controller.action14_scale;
                effect->AdvanceActionEffect(owner, action, value << 1, &output);
                x = effect->position[0];
                y = effect->position[1];
                z = effect->position[2];
                effect->world_position[0] = x;
                effect->world_position[1] = y;
                effect->world_position[2] = z;
                effect->metadata_words72[0] = 12288;
                effect->frame = 0;
                effect->path_frame = 0;
                character_controller.action_effects[action] = effect;
                flags = effect->flags;
                if (action == 14)
                    effect->velocity[2] = 0;
                effect->flags = flags | 0x8000;
                if (action == 13)
                    character_controller.action13_count += 1;
                else
                    character_controller.action14_count += 1;
                if (action == 13)
                    effect->velocity[2] = 0;
                break;
            default:
                character_controller.action_effects[action] = effect;
                effect->flags = effect->flags | 0x8000;
                break;
        }
    }
    effect->behavior = 26;
    {
        SceneSection *section = scene_stream.active_section;
        SceneNode *previous = section->transient_links.first_child;
        result = (uint32)previous;
        effect->node.links.sibling_next = previous;
        section->transient_links.first_child = &effect->node;
    }
    return result;
}

uint32 CharacterController::ClearInteractionActions()
{
    FUNCTION_MARKER(0x8005A540, "SLUS_005.78");
    uint32 flags;
    sint32 index;
    if (character_action_flags != 0)
    {
        AnimatedObject *active = character_action_target;
        if (active != 0)
        {
            ActorBinding *binding = (ActorBinding *)(((const ActorPlacement *)((const PathDefinition *)((active->path_definition))))->state != 0 ? ((const ActorPlacement *)((const PathDefinition *)((active->path_definition))))->state : 0);
            binding->flags &= 0xF7;
        }
    }
    if ((character_action_flags & 8) != 0)
        RemoveAction(12);
    flags = character_action_flags;
    character_action_flags = flags & 0xFFFFEFF7;
    if ((flags & 16) != 0)
    {
        if (action13_count > 0)
        {
            index = 0;
            do
            {
                RemoveAction(13);
                ++index;
            } while (index < action13_count);
        }
        action13_count = 0;
    }
    if ((character_action_flags & 32) != 0)
    {
        if (action14_count > 0)
        {
            index = 0;
            do
            {
                RemoveAction(14);
                ++index;
            } while (index < action14_count);
        }
        action14_count = 0;
    }
    flags = character_action_flags & ~48u;
    character_action_flags = flags;
    return flags;
}

uint32 CharacterController::EndInteraction()
{
    FUNCTION_MARKER(0x8005A668, "SLUS_005.78");
    uint32 flags;
    sint32 index;
    if (character_action_flags != 0)
    {
        AnimatedObject *active = character_action_target;
        if (active != 0)
        {
            ActorBinding *binding = (ActorBinding *)(((const ActorPlacement *)((const PathDefinition *)((active->path_definition))))->state != 0 ? ((const ActorPlacement *)((const PathDefinition *)((active->path_definition))))->state : 0);

            binding->flags &= 0xF7;
        }
    }
    if ((character_action_flags & 8) != 0)
        RemoveAction(12);
    flags = character_action_flags;
    character_action_flags = flags & 0xFFFFEFF7;
    if ((flags & 16) != 0)
    {
        if (action13_count > 0)
        {
            index = 0;
            do
            {
                RemoveAction(13);
                ++index;
            } while (index < action13_count);
        }
        action13_count = 0;
    }
    if ((character_action_flags & 32) != 0)
    {
        if (action14_count > 0)
        {
            index = 0;
            do
            {
                RemoveAction(14);
                ++index;
            } while (index < action14_count);
        }
        action14_count = 0;
    }
    flags = secondary_flags;
    character_action_flags = 0;
    flags &= ~256u;
    secondary_flags = flags;
    return flags;
}

uint32 BeginCharacterInteraction(AnimatedObject *object)
{
    FUNCTION_MARKER(0x8005A440, "SLUS_005.78");
    CharacterController *character = &character_controller;
    uint32 flags = character->secondary_flags;
    character_action_flags = 4;
    character_controller.timers.interaction_ticks = 900;
    character_controller.action_ticks = 0;
    character_controller.phase_ticks = 0;
    character_controller.interaction_phase = 0;
    character_action_owner = object;
    character_action_target = 0;
    character_controller.action13_count = 0;
    character_controller.action14_count = 0;
    character->secondary_flags = flags | 0x100;
    return 900;
}

uint32 AnimatedObject::UpdateCharacterInteraction()
{
    FUNCTION_MARKER(0x8005A7F4, "SLUS_005.78");
    const ActorModelDefinition *bound_model;
    uint32 flags, result;
    uint32 cleanup_flags;
    ActorBinding *cleanup_binding;
    AnimatedObject *cleanup_target;
    sint32 action_index;
    CharacterController *character = &character_controller;
    if (hold_interaction_timeout == 0)
        character_controller.timers.interaction_ticks = character_controller.timers.interaction_ticks - 1u;
    character_controller.phase_ticks = (uint8)character_controller.phase_ticks + 1u;
    flags = character_action_flags;
    if ((flags & 0x800) != 0)
    {
        character_action_flags = flags & ~0x800u;
        character_controller.phase_ticks = 0;
        character_controller.interaction_phase = 0;
        cleanup_flags = character_action_flags;
        if (cleanup_flags != 0)
        {
            cleanup_target = character_action_target;
            if (cleanup_target != 0)
            {
                cleanup_binding = (ActorBinding *)(((const ActorPlacement *)((const PathDefinition *)((cleanup_target->path_definition))))->state != 0 ? ((const ActorPlacement *)((const PathDefinition *)((cleanup_target->path_definition))))->state : 0);
                cleanup_binding->flags &= 0xF7;
            }
            cleanup_flags = character_action_flags;
        }
        if ((cleanup_flags & 8) != 0)
            character_controller.RemoveAction(12);
        cleanup_flags = character_action_flags;
        character_action_flags = cleanup_flags & 0xFFFFEFF7;
        if ((cleanup_flags & 16) != 0)
        {
            action_index = 0;
            while (action_index < character_controller.action13_count)
            {
                character_controller.RemoveAction(13);
                ++action_index;
            }
            character_controller.action13_count = 0;
        }
        if ((character_action_flags & 32) != 0)
        {
            action_index = 0;
            while (action_index < character_controller.action14_count)
            {
                character_controller.RemoveAction(14);
                ++action_index;
            }
            character_controller.action14_count = 0;
        }
        character_action_flags = character_action_flags & ~48u;
    }
    switch (character_controller.interaction_phase)
    {
        case 0:
            if ((this->flags & 0x1000400) == 0x1000000)
            {
                character_controller.phase_ticks = 0;
                character_controller.interaction_phase = 1;
            }
            break;
        case 1:
        {
            sint32 tick = character_controller.phase_ticks;
            if (tick == 1)
            {
                QueueCharacterActionSound(16, 3);
                SpawnActionEffect(this, &character_controller, 11);
            }
            else if (tick == 4)
            {
                flags = character_action_flags;
                character_action_target = 0;
                character_action_flags = flags | 0x200;
            }
            else if (tick == 5 || tick == 6)
            {
                AnimatedObject *target = character_action_target;
                uint32 index;
                const PathDefinition *definition;
                const PathTransform *path_transform;
                ActorBinding *data;

                union
                {
                    VECTOR vector;
                    uint32 words[4];
                } world, input, other;

                uint32 position[3];
                if (tick == 5 && target == 0)
                {
                    character_action_flags = character_action_flags | 0x200;
                    break;
                }
                character_controller.phase_ticks = 0;
                target = character_action_target;
                if (target == 0)
                {
                    character_controller.interaction_phase = 0;
                    break;
                }
                data = (ActorBinding *)(((const ActorPlacement *)((const PathDefinition *)((target->path_definition))))->state != 0 ? ((const ActorPlacement *)((const PathDefinition *)((target->path_definition))))->state : 0);
                flags = character_action_flags;
                bound_model = data->model;
                character_action_flags = flags & ~0x200u;
                definition = (target->path_definition);
                if (definition != 0 && ((definition)->transform) != 0)
                {
                    for (index = 0; index < 3; ++index)
                        position[index] = target->position[index];
                    for (index = 0; index < 3; ++index)
                        input.words[index] = position[index];
                    PushMatrix();
                    target = character_action_target;
                    path_transform = (const PathTransform *)((((const PathDefinition *)((const PathDefinition *)((target->path_definition))))->transform) != 0 ? (((const PathDefinition *)((const PathDefinition *)((target->path_definition))))->transform) : (PathTransform *)0);
                    for (index = 0; index < 5; ++index)
                        xport_gte_write_control(index, xport_load_le32((const uint8 *)&path_transform->matrix + index * 4));
                    ApplyRotMatrixLV(&input.vector, &world.vector);
                    PopMatrix();
                    target = character_action_target;
                    for (index = 0; index < 3; ++index)
                    {
                        path_transform = (const PathTransform *)((((const PathDefinition *)((const PathDefinition *)((target->path_definition))))->transform) != 0 ? (((const PathDefinition *)((const PathDefinition *)((target->path_definition))))->transform) : (PathTransform *)0);
                        world.words[index] = world.words[index] + (uint32)path_transform->matrix.t[index];
                    }
                }
                else
                {
                    target = character_action_target;
                    for (index = 0; index < 3; ++index)
                        position[index] = target->position[index];
                    for (index = 0; index < 3; ++index)
                        world.words[index] = position[index];
                }
                data->interaction_height = world.words[1];
                if ((bound_model->interaction_animation_mode) != -1 || (bound_model->interaction_limit_step) != 0)
                {
                    definition = (this->path_definition);
                    if (definition != 0 && ((definition)->transform) != 0)
                    {
                        for (index = 0; index < 3; ++index)
                            position[index] = this->position[index];
                        for (index = 0; index < 3; ++index)
                            input.words[index] = position[index];
                        PushMatrix();
                        path_transform = (const PathTransform *)((((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) != 0 ? (((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) : (PathTransform *)0);
                        for (index = 0; index < 5; ++index)
                            xport_gte_write_control(index, xport_load_le32((const uint8 *)&path_transform->matrix + index * 4));
                        ApplyRotMatrixLV(&input.vector, &other.vector);
                        PopMatrix();
                        for (index = 0; index < 3; ++index)
                        {
                            path_transform = (const PathTransform *)((((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) != 0 ? (((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) : (PathTransform *)0);
                            other.words[index] = other.words[index] + (uint32)path_transform->matrix.t[index];
                        }
                    }
                    else
                    {
                        for (index = 0; index < 3; ++index)
                            position[index] = this->position[index];
                        for (index = 0; index < 3; ++index)
                            other.words[index] = position[index];
                    }
                    result = (uint32)Atan2Angle((sint32)(world.words[2] - other.words[2]), (sint32)(world.words[0] - other.words[0]));
                }
                else
                    result = ((uint16)character_action_target->yaw);
                character_controller.action_heading = result;
                character_action_flags = character_action_flags | 8;
                data->flags |= 8;
                SpawnActionEffect(character_action_target, &character_controller, 12);
                if ((character_action_flags & 0x1000) != 0 || (bound_model->interaction_limit_step) == 0)
                    character_controller.interaction_phase = 2;
                else
                {
                    character_controller.interaction_phase = 4;
                    character_controller.action13_scale = 4096;
                    character_controller.action_pending = 0;
                    character_controller.action_delay = 0;
                    character_controller.action_mode = 0;
                    character_controller.action_latched = 0;
                    character_controller.action13_count = 0;
                    character_controller.action14_count = 0;
                    character_controller.action13_limit_reached = 0;
                    character_controller.action14_limit_reached = 0;
                    character_controller.action_completion = 0;
                    character_controller.action13_endpoint = (uint32)-1;
                    character_controller.action14_endpoint = (uint32)-1;
                    character_controller.action_shared_limit = (uint32)-1;
                    character_controller.action_limit_step = (sint8)((uint16)bound_model->interaction_limit_step);
                    character_controller.action_delay_bias = (sint8)(bound_model->interaction_delay_bias);
                }
            }
            break;
        }
        case 2:
            if (character_action_target == 0 || character_action_target->ticks <= 0)
            {
                if ((character_action_flags & 8) != 0)
                    character_controller.RemoveAction(12);
                character_controller.interaction_phase = 0;
                cleanup_flags = character_action_flags;
                if (cleanup_flags != 0)
                {
                    cleanup_target = character_action_target;
                    if (cleanup_target != 0)
                    {
                        cleanup_binding = (ActorBinding *)(((const ActorPlacement *)((const PathDefinition *)((cleanup_target->path_definition))))->state != 0 ? ((const ActorPlacement *)((const PathDefinition *)((cleanup_target->path_definition))))->state : 0);
                        cleanup_binding->flags &= 0xF7;
                    }
                    cleanup_flags = character_action_flags;
                }
                if ((cleanup_flags & 8) != 0)
                    character_controller.RemoveAction(12);
                cleanup_flags = character_action_flags;
                character_action_flags = cleanup_flags & 0xFFFFEFF7;
                if ((cleanup_flags & 16) != 0)
                {
                    action_index = 0;
                    while (action_index < character_controller.action13_count)
                    {
                        character_controller.RemoveAction(13);
                        ++action_index;
                    }
                    character_controller.action13_count = 0;
                }
                if ((character_action_flags & 32) != 0)
                {
                    action_index = 0;
                    while (action_index < character_controller.action14_count)
                    {
                        character_controller.RemoveAction(14);
                        ++action_index;
                    }
                    character_controller.action14_count = 0;
                }
                character_action_flags = character_action_flags & ~48u;
            }
            else
            {
                if ((this->flags & 0x1000400) == 0x1000000)
                {
                    QueueCharacterActionSound(16, 3);
                    SpawnActionEffect(this, &character_controller, 11);
                    character_controller.phase_ticks = 0;
                    SpawnActionEffect(this, &character_controller, 13);
                    character_action_flags = character_action_flags | 16;
                }
                if (character_controller.action_completion == 1)
                {
                    AnimatedObject *target;
                    ActorBinding *data;
                    character_controller.action_completion = 0;
                    target = character_action_target;
                    if (target->ticks != 0)
                    {
                        data = (ActorBinding *)(((const ActorPlacement *)((const PathDefinition *)((target->path_definition))))->state != 0 ? ((const ActorPlacement *)((const PathDefinition *)((target->path_definition))))->state : 0);
                        flags = data->reaction_flags;
                        result = (uint32)(sint32)raise_completed_target;
                        data->reaction_flags = (flags & 0xFFFF01FF) | 0x208;
                        if (result != 0)
                        {
                            target->position[1] = target->position[1] + 3072u;
                            data->flags |= 16;
                        }
                    }
                }
            }
            break;
        case 4:
            if (character_action_target == 0 || character_action_target->ticks <= 0)
            {
                character_controller.phase_ticks = 0;
                character_controller.interaction_phase = 0;
                cleanup_flags = character_action_flags;
                if (cleanup_flags != 0)
                {
                    cleanup_target = character_action_target;
                    if (cleanup_target != 0)
                    {
                        cleanup_binding = (ActorBinding *)(((const ActorPlacement *)((const PathDefinition *)((cleanup_target->path_definition))))->state != 0 ? ((const ActorPlacement *)((const PathDefinition *)((cleanup_target->path_definition))))->state : 0);
                        cleanup_binding->flags &= 0xF7;
                    }
                    cleanup_flags = character_action_flags;
                }
                if ((cleanup_flags & 8) != 0)
                    character_controller.RemoveAction(12);
                cleanup_flags = character_action_flags;
                character_action_flags = cleanup_flags & 0xFFFFEFF7;
                if ((cleanup_flags & 16) != 0)
                {
                    action_index = 0;
                    while (action_index < character_controller.action13_count)
                    {
                        character_controller.RemoveAction(13);
                        ++action_index;
                    }
                    character_controller.action13_count = 0;
                }
                if ((character_action_flags & 32) != 0)
                {
                    action_index = 0;
                    while (action_index < character_controller.action14_count)
                    {
                        character_controller.RemoveAction(14);
                        ++action_index;
                    }
                    character_controller.action14_count = 0;
                }
                character_action_flags = character_action_flags & ~48u;
                break;
            }
            if ((this->flags & 0x1000400) == 0x1000000)
            {
                QueueCharacterActionSound(16, 3);
                SpawnActionEffect(this, &character_controller, 11);
                character_controller.phase_ticks = 0;
                SpawnActionEffect(this, &character_controller, 13);
                character_action_flags = character_action_flags | 16;
                character_controller.action_mode = 1;
            }
            if (character_controller.action_pending != 0)
            {
                sint32 table_index, delay;
                character_controller.action14_scale = character_controller.action_mode == 1 ? 4096 : 2048;
                QueueCharacterActionSound(16, 0);
                SpawnActionEffect(this, &character_controller, 14);
                character_action_flags = character_action_flags | 32;
                table_index = character_controller.action_ticks % 16;
                character_controller.action_pending = 0;
                delay = OriginalDefaults::InteractionWave[table_index] / 2048;
                character_controller.action_delay = delay;
                delay += (uint8)character_controller.action_delay_bias;
                character_controller.action_delay = (sint32)(sint8)delay * 4;
            }
            else
            {
                if (character_controller.action_latched != 0 && character_controller.action_mode != 1)
                    goto interaction_result;
                result = (uint8)character_controller.action_delay - 1u;
                character_controller.action_delay = result;
                if ((sint8)result > 0 || pause_interaction_actions != 0 || (character_controller.action_latched != 0 && character_controller.action_mode != 1))
                    goto interaction_result;
                {
                    uint32 pending = (uint8)character_controller.action_pending;
                    sint32 table_index = character_controller.action_ticks % 16;
                    sint32 delay;
                    character_controller.action_latched = 1;
                    character_controller.action_pending = pending + 1u;
                    delay = OriginalDefaults::InteractionWave[table_index] / 2048;
                    result = (uint8)character_controller.action_delay_bias;
                    character_controller.action_delay = delay;
                    character_controller.action_delay = (uint32)delay + result;
                }
            }
        interaction_result:
            if (character_controller.action_completion == 1)
            {
                character_controller.action_completion = 0;
                if ((character_action_flags & 16) != 0)
                {
                    action_index = 0;
                    while (action_index < character_controller.action13_count)
                    {
                        character_controller.RemoveAction(13);
                        ++action_index;
                    }
                    character_controller.action13_count = 0;
                }
                if ((character_action_flags & 32) != 0)
                {
                    action_index = 0;
                    while (action_index < character_controller.action14_count)
                    {
                        character_controller.RemoveAction(14);
                        ++action_index;
                    }
                    character_controller.action14_count = 0;
                }
                character_action_flags = (character_action_flags & ~48u) | 0x3000;
                character_controller.interaction_phase = 5;
                character_controller.action_ticks = 0;
            }
            else if (character_controller.action_completion == 2)
            {
                character_controller.action_completion = 0;
                flags = character_action_flags;
                character_controller.support.contact = 0;
                character_controller.contacts.damage_fraction = 0;
                character_action_flags = flags | 0x2100;
            }
            break;
        case 5:
            result = (uint8)character_controller.action_ticks + 1u;
            character_controller.action_ticks = result;
            if ((sint8)result >= 3)
            {
                character_action_flags = character_action_flags & ~0x2000u;
                character_controller.action_ticks = 0;
                character_controller.interaction_phase = 2;
            }
            break;
        default:
            character_controller.phase_ticks = 0;
            character_controller.action_ticks = 0;
            character_controller.interaction_phase = 0;
            break;
    }
    if (character_controller.timers.interaction_ticks == 0)
    {
        cleanup_flags = character_action_flags;
        if (cleanup_flags != 0)
        {
            cleanup_target = character_action_target;
            if (cleanup_target != 0)
            {
                cleanup_binding = (ActorBinding *)(((const ActorPlacement *)((const PathDefinition *)((cleanup_target->path_definition))))->state != 0 ? ((const ActorPlacement *)((const PathDefinition *)((cleanup_target->path_definition))))->state : 0);
                cleanup_binding->flags &= 0xF7;
            }
            cleanup_flags = character_action_flags;
        }
        if ((cleanup_flags & 8) != 0)
            character_controller.RemoveAction(12);
        cleanup_flags = character_action_flags;
        character_action_flags = cleanup_flags & 0xFFFFEFF7;
        if ((cleanup_flags & 16) != 0)
        {
            action_index = 0;
            while (action_index < character_controller.action13_count)
            {
                character_controller.RemoveAction(13);
                ++action_index;
            }
            character_controller.action13_count = 0;
        }
        if ((character_action_flags & 32) != 0)
        {
            action_index = 0;
            while (action_index < character_controller.action14_count)
            {
                character_controller.RemoveAction(14);
                ++action_index;
            }
            character_controller.action14_count = 0;
        }
        flags = character->secondary_flags;
        character_action_flags = 0;
        character->secondary_flags = flags & ~0x100u;
    }
    flags = this->flags;
    result = (uint8)character_controller.phase_ticks;
    character_controller.action_disp_phase = result;
    this->flags = flags & ~0x1000000u;
    return result;
}

uint32 AnimatedObject::AdvanceActionEffect(const AnimatedObject *owner, uint32 action, uint32 distance, uint32 *progress_output)
{
    FUNCTION_MARKER(0x80028D60, "SLUS_005.78");
    ActionVector world[3], input, relative;

    union
    {
        uint32 words[5];
        uint16 halves[10];
    } transpose;

    uint32 source_index, index;
    const PathDefinition *definition;
    const PathTransform *path_transform;
    uint32 difference[3], absolute[3], interpolated[3];
    uint32 length, progress, coefficient, initialized = 0, result;
    sint32 reciprocal;
    CharacterController *character;
    for (source_index = 0; source_index < 3; ++source_index)
    {
        const AnimatedObject *source = source_index == 0 ? owner : (source_index == 1 ? character_action_target : this);
        ActionVector *destination = &world[source_index];
        uint32 position[3];
        definition = (source->path_definition);
        if (definition != 0 && ((definition)->transform) != 0)
        {
            for (index = 0; index < 3; ++index)
                position[index] = source->position[index];
            for (index = 0; index < 3; ++index)
                input.words[index] = position[index];
            PushMatrix();
            if (source_index == 1)
            {
                source = character_action_target;
            }
            path_transform = (const PathTransform *)((((const PathDefinition *)((const PathDefinition *)((source->path_definition))))->transform) != 0 ? (((const PathDefinition *)((const PathDefinition *)((source->path_definition))))->transform) : (PathTransform *)0);
            for (index = 0; index < 5; ++index)
                xport_gte_write_control(index, xport_load_le32((const uint8 *)&path_transform->matrix + index * 4));
            ApplyRotMatrixLV(&input.vector, &destination->vector);
            PopMatrix();
            if (source_index == 1)
            {
                source = character_action_target;
            }
            for (index = 0; index < 3; ++index)
            {
                path_transform = (const PathTransform *)((((const PathDefinition *)((const PathDefinition *)((source->path_definition))))->transform) != 0 ? (((const PathDefinition *)((const PathDefinition *)((source->path_definition))))->transform) : (PathTransform *)0);
                destination->words[index] = destination->words[index] + (uint32)path_transform->matrix.t[index];
            }
        }
        else
        {
            if (source_index == 1)
            {
                source = character_action_target;
            }
            for (index = 0; index < 3; ++index)
                position[index] = source->position[index];
            for (index = 0; index < 3; ++index)
                destination->words[index] = position[index];
        }
    }
    world[2].words[1] = world[2].words[1] - 16384;
    for (index = 0; index < 3; ++index)
    {
        difference[index] = world[0].words[index] - world[1].words[index];
        relative.words[index] = difference[index];
        absolute[index] = (sint32)difference[index] < 0 ? 0u - difference[index] : difference[index];
    }
    length = 2u * (absolute[0] + absolute[2]) + absolute[1];
    progress = this->velocity[0];
    progress = action == 13 ? progress - distance : progress + distance;
    if (length == 0)
    {
        fprintf(stderr, "P2: Original division-by-zero trap in 80028D60\n");
        abort();
    }
    reciprocal = 0x40000000 / (sint32)length;
    this->velocity[0] = progress;
    coefficient = (uint32)(((sint64)(sint32)progress * reciprocal) >> 18);
    fixed_math_result = coefficient;
    character = &character_controller;
    if (character_controller.interaction_phase == 4 && character_controller.action_mode == 1)
    {
        uint32 limit = character_controller.action_shared_limit;
        if (limit == (uint32)-1)
        {
            if (action == 13)
            {
                uint32 endpoint = character_controller.action14_endpoint;
                if ((sint32)progress < (sint32)endpoint && endpoint != (uint32)-1)
                {
                    initialized = 1;
                    character_controller.action_shared_limit = endpoint;
                    character_controller.action13_limit_reached = 1;
                    this->velocity[2] = 1;
                    progress = character_controller.action_shared_limit;
                    character_controller.action13_endpoint = progress;
                }
            }
            else
            {
                uint32 endpoint = character_controller.action13_endpoint;
                if ((sint32)endpoint < (sint32)progress && character_controller.action14_endpoint != (uint32)-1)
                {
                    initialized = 1;
                    character_controller.action_shared_limit = endpoint;
                    character_controller.action14_limit_reached = 1;
                    this->velocity[2] = 1;
                    progress = character_controller.action_shared_limit;
                    character_controller.action14_endpoint = progress;
                }
            }
        }
        else if (this->velocity[2] == 1)
        {
            if (action == 13 ? (sint32)progress < (sint32)limit : (sint32)limit < (sint32)progress)
                progress = limit;
        }
        else if (action == 13)
        {
            if ((sint32)progress < (sint32)limit)
            {
                if (character_controller.action13_limit_reached != 0)
                {
                    result = (uint32)-2;
                    goto finished;
                }
                this->velocity[2] = 1;
                progress = character_controller.action_shared_limit;
                character_controller.action13_limit_reached = 1;
            }
        }
        else if ((sint32)limit < (sint32)progress)
        {
            if (character_controller.action14_limit_reached != 0)
            {
                result = (uint32)-2;
                goto finished;
            }
            this->velocity[2] = 1;
            progress = character_controller.action_shared_limit;
            character_controller.action14_limit_reached = 1;
        }
    }
    if ((sint32)progress < (sint32)length && (sint32)progress >= 0 && initialized == 0)
    {
        coefficient = (uint32)(((sint64)(sint32)progress * reciprocal) >> 18);
        fixed_math_result = coefficient;
        for (index = 0; index < 3; ++index)
            interpolated[index] = (uint32)(((sint64)(sint32)coefficient * (sint32)difference[index]) >> 12);
        interpolated[1] += 16384;
        for (index = 0; index < 3; ++index)
            interpolated[index] += world[1].words[index];
        definition = (this->path_definition);
        if (definition != 0 && ((definition)->transform) != 0)
        {
            for (index = 0; index < 3; ++index)
            {
                path_transform = (const PathTransform *)((((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) != 0 ? (((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) : (PathTransform *)0);
                relative.words[index] = interpolated[index] - (uint32)path_transform->matrix.t[index];
            }
            for (index = 0; index < 9; ++index)
            {
                path_transform = (const PathTransform *)((((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) != 0 ? (((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) : (PathTransform *)0);
                transpose.halves[index] = path_transform->matrix.m[index % 3][index / 3];
            }
            PushMatrix();
            for (index = 0; index < 4; ++index)
                xport_gte_write_control(index, transpose.words[index]);
            xport_gte_write_control(4, transpose.halves[8]);
            ApplyRotMatrixLV(&relative.vector, (VECTOR *)this->position);
            PopMatrix();
        }
        else
        {
            for (index = 0; index < 3; ++index)
                this->position[index] = interpolated[index];
        }
    }
    *progress_output = progress;
    if (action == 13)
        result = (sint32)progress < 0 ? (uint32)-1 : progress;
    else
        result = (sint32)progress < (sint32)length ? length - progress : (uint32)-1;
finished:
    return result;
}

uint32 AnimatedObject::UpdateActionEffect(const AnimatedObject *owner, uint32 action)
{
    FUNCTION_MARKER(0x800296A4, "SLUS_005.78");
    uint32 output, result, distance;
    if ((character_action_flags & 0x2008) != 8)
        goto remove_object;
    if (character_controller.interaction_phase != 4)
    {
        AdvanceActionEffect(owner, action, 20480, &output);
        result = 1;
        if ((sint32)output >= 2048)
            goto finished;
        character_controller.action_completion = 1;
        goto remove_primary;
    }
    distance = AdvanceActionEffect(owner, action, action == 13 ? 20480 : 4096, &output);
    if (action == 13)
    {
        uint32 prev = character_controller.action_shared_limit;
        uint32 check_status = prev != (uint32)-1;
        if (prev == (uint32)-1)
        {
            uint32 sample = output;
            uint32 cur = character_controller.action_shared_limit;
            character_controller.action13_endpoint = sample;
            check_status = cur != prev;
        }
        if (check_status != 0 && distance == (uint32)-2)
        {
            uint32 flags;
            character_controller.action_shared_limit = character_controller.action_shared_limit - 8192;
            flags = this->node.flags;
            this->behavior = 0;
            this->node.flags = flags | 2;
            character_controller.action13_count = (uint8)character_controller.action13_count - 1u;
        }
        if ((sint32)distance < 2048 && (this->node.flags & 2) == 0)
            character_controller.action_completion = 1;
    }
    else
    {
        if (character_controller.action_shared_limit == (uint32)-1)
            character_controller.action14_endpoint = output;
        if (character_controller.action_mode == 1 && distance == (uint32)-2 && this->velocity[2] != 1)
        {
            uint32 flags;
            sint32 displacement = (sint32)character_controller.action_limit_step * 4096;
            character_controller.action_shared_limit = character_controller.action_shared_limit + (uint32)displacement;
            flags = this->node.flags;
            this->behavior = 0;
            this->node.flags = flags | 2;
            character_controller.action14_count = (uint8)character_controller.action14_count - 1u;
        }
        if ((sint32)distance < 2048 && (this->node.flags & 2) == 0)
        {
            uint32 flags;
            character_controller.action_completion = 2;
            flags = this->node.flags;
            this->behavior = 0;
            this->node.flags = flags | 2;
            character_controller.action14_count = (uint8)character_controller.action14_count - 1u;
        }
    }
    result = distance + 2u < 2;
    if (result == 0)
        goto finished;
    result = this->node.flags & 2;
    if (result != 0)
        goto finished;
remove_object:
    if (action != 13)
    {
        uint32 flags = this->node.flags;
        this->behavior = 0;
        this->node.flags = flags | 2;
        result = (uint8)character_controller.action14_count - 1u;
        character_controller.action14_count = result;
        goto finished;
    }
remove_primary:
{
    uint32 flags = this->node.flags;
    this->behavior = 0;
    this->node.flags = flags | 2;
    result = (uint8)character_controller.action13_count - 1u;
    character_controller.action13_count = result;
}
finished:
    return result;
}

uint32 QueueCharacterActionSound(sint32 action, uint32 priority)
{
    FUNCTION_MARKER(0x80044FFC, "SLUS_005.78");
    uint32 mode, index, minimum;
    const SoundBankStorage *table;
    const SoundSample *sound;
    if (action == 2)
    {
        mode = (uint16)(character_controller.motion.form_flags >> 16) & 15;
        if (mode == 1 || mode == 7 || mode == 9 || mode == 14 || mode == 11)
            action = 3;
    }
    table = sound_resources.common_table;
    if (table != 0 && action < (sint32)table->header.count)
        sound = table->samples + action;
    else
        sound = 0;
    mode = (uint16)(character_controller.motion.form_flags >> 16) & 15;
    minimum = 0;
    index = 0;
    if (mode == 4)
    {
        if (action == 0)
        {
            minimum = 12;
            index = 11;
        }
        else if (action == 1)
        {
            minimum = 13;
            index = 12;
        }
        else if (action == 2)
        {
            minimum = 14;
            index = 13;
        }
        else if (action == 7)
        {
            minimum = 4;
            index = 3;
        }
    }
    else if (mode == 5 && action == 2)
    {
        minimum = 14;
        index = 13;
    }
    if (minimum != 0)
    {
        table = sound_resources.scene_table;
        sound = 0;
        if (table != 0 && (sint32)table->header.count >= (sint32)minimum)
            sound = table->samples + index;
    }
    mode = sound_settings.effects_volume;
    return sound_effects.Queue(sound, 0, 0, mode, 0, priority);
}

uint32 PauseCharacterSound(void)
{
    FUNCTION_MARKER(0x800504E4, "SLUS_005.78");
    uint32 result;
    const SoundBankStorage *table;
    const SoundSample *sound;
    table = sound_resources.common_table;
    character_sound_resume_flags = 0;
    sound = 0;
    if (table && (sint32)table->header.count >= 7)
        sound = table->samples + 6;
    if (sound_effects.IsPlaying(sound, character_controller.sound_voice) & 0xFF)
        character_sound_resume_flags = character_sound_resume_flags | 1;
    table = sound_resources.common_table;
    sound = 0;
    if (table && (sint32)table->header.count >= 11)
        sound = table->samples + 10;
    if (sound_effects.IsPlaying(sound, character_controller.sound_voice) & 0xFF)
        character_sound_resume_flags = character_sound_resume_flags | 2;
    result = character_sound_resume_flags;
    if (result)
    {
        StopSoundVoice(character_controller.sound_voice);
        result = 0xFFFFFFFF;
        character_controller.sound_voice = (sint8)result;
    }
    return result;
}

void ResumeCharacterSound(void)
{
    FUNCTION_MARKER(0x800505E4, "SLUS_005.78");
    uint32 flags = character_sound_resume_flags;
    const SoundSample *sound = 0;
    const SoundBankStorage *sounds;
    sint32 voice;
    if ((flags & 1) != 0)
    {
        sounds = sound_resources.common_table;
        if (sounds != 0 && (sint32)sounds->header.count >= 7)
            sound = sounds->samples + 6;
    }
    else
    {
        if ((flags & 2) == 0)
            return;
        sounds = sound_resources.common_table;
        if (sounds != 0 && (sint32)sounds->header.count >= 11)
            sound = sounds->samples + 10;
    }
    voice = sound_effects.SelectVoice((uint32)-1, sound, 1);
    character_controller.sound_voice = (sint8)voice;
    voice = (sint8)voice;
    if (voice >= 0 && sound != 0)
    {
        const SoundSample *sample = ((const SoundSample *)(sound));
        uint16 metadata = sample->metadata;
        uint16 pitch = sample->pitch;
        sint16 volume = (sint16)(sound_settings.effects_volume);
        PlaySoundEffect(sound, (uint16)voice, 0, metadata, pitch, volume, 0);
    }
}

void AnimatedObject::UpdateCharacterActionEffects()
{
    const PathTransform *captured_path_transform;
    CharacterModelRecord *parent_record;
    AnimatedObject *created_object;
    FUNCTION_MARKER(0x800515F0, "SLUS_005.78");
    VECTOR input, relative;

    union
    {
        uint32 words[5];
        uint16 halves[10];
    } transpose;

    const PathDefinition *parent;
    uint32 flags, original_flags, value, x, y, z, result, kind, eject, angle, pitch;
    const PathTransform *path_transform;
    const SoundBankStorage *sound_table;
    const SoundSample *sound;
    uint64 encoded;
    sint32 timer, voice, volume, wave;
    if ((character_controller.secondary_flags & 0x100) != 0)
    {
        if ((this->flags & 0x1000000) != 0)
        {
            parent = (this->path_definition);
            if (parent != 0 && ((((const PathDefinition *)(parent)))->transform) != 0)
            {
                x = this->position[0];
                y = this->position[1];
                z = this->position[2];
                input.vx = x;
                input.vy = y;
                input.vz = z;
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
                ApplyRotMatrixLV(&input, &player_interaction.position);
                PopMatrix();
                path_transform = (const PathTransform *)((((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) != 0 ? (((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) : (PathTransform *)0);
                x = (uint32)player_interaction.position.vx;
                player_interaction.position.vx = x + (uint32)path_transform->matrix.t[0];
                path_transform = (const PathTransform *)((((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) != 0 ? (((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) : (PathTransform *)0);
                y = (uint32)player_interaction.position.vy;
                player_interaction.position.vy = y + (uint32)path_transform->matrix.t[1];
                path_transform = (const PathTransform *)((((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) != 0 ? (((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) : (PathTransform *)0);
                z = (uint32)player_interaction.position.vz;
                player_interaction.position.vz = z + (uint32)path_transform->matrix.t[2];
            }
            else
            {
                x = this->position[0];
                y = this->position[1];
                z = this->position[2];
                player_interaction.position.vx = x;
                player_interaction.position.vy = y;
                player_interaction.position.vz = z;
            }
        }
        UpdateCharacterInteraction();
    }
    flags = character_controller.secondary_flags;
    if ((flags & 0x40) != 0)
    {
        value = (uint32)character_controller.timers.model_swap_ticks - 1;
        character_controller.timers.model_swap_ticks = value;
        encoded = psx_double_bits(__floatsidf((sint16)value));
        if (p2_func_8006ec24((uint32)encoded, (uint32)(encoded >> 32), 0, 0x407B3000) == 0)
        {
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
            QueueCharacterActionSound(9, 3);
            sound_table = sound_resources.common_table;
            sound = sound_table != 0 && (sint32)sound_table->header.count >= 11 ? sound_table->samples + 10 : 0;
            voice = sound_effects.SelectVoice(0xFFFFFFFF, sound, 1);
            character_controller.sound_voice = (sint8)(voice);
            voice = (sint8)voice;
            if (voice >= 0)
            {
                const SoundSample *sample = ((const SoundSample *)(sound));
                value = sample->metadata;
                pitch = sample->pitch;
                volume = (sint16)(sound_settings.effects_volume);
                PlaySoundEffect(sound, (uint16)voice, 0, value, pitch, volume, 0);
            }
        }
        if ((sint16)character_controller.timers.model_swap_ticks < 150)
        {
            value = character_controller.appearance.blink_ticks;
            character_controller.appearance.blink_ticks = value + 1;
            if ((sint32)value >= 15)
            {
                flags = this->flags;
                character_controller.appearance.blink_ticks = 0;
                this->flags = flags ^ 0x80000;
            }
        }
        if ((sint16)character_controller.timers.model_swap_ticks == 0)
        {
            parent_record = character_controller.model;
            AnimationHeader *animation22 = parent_record->animations[22];
            AnimationHeader *animation2 = parent_record->animations[2];
            parent_record->animations[2] = animation22;
            parent_record = character_controller.model;
            parent_record->animations[22] = animation2;
            character_controller.RemoveAction(7);
            this->flags = this->flags & 0xDFFFFFFF;
            character_controller.secondary_flags = character_controller.secondary_flags & 0xFFFFFFBF;
            this->flags = this->flags & 0xFFF7FFFF;
        }
    }
    else if ((flags & 0x200) != 0)
    {
        value = (uint32)character_controller.timers.color_effect_ticks - 1;
        character_controller.timers.color_effect_ticks = value;
        timer = (sint16)character_controller.timers.color_effect_ticks;
        character_controller.appearance.tint = (((16 * (value & 15) + 64) & 0xFF) << 16) | 0x2020;
        if (timer == 0)
        {
            this->flags = this->flags & 0xDFF7FFFF;
            character_controller.secondary_flags = character_controller.secondary_flags & 0xFFFFFDFF;
        }
    }
    flags = this->flags;
    if ((flags & 0x1000000) == 0)
    {
        timer = (sint16)character_controller.timers.action_cooldown_ticks;
        result = (uint32)timer;
        if (timer != 0)
        {
            result = (uint32)timer - 1;
            character_controller.timers.action_cooldown_ticks = result;
        }
        goto done;
    }
    value = character_controller.timers.action_launch_ticks;
    result = value - 1;
    if ((sint32)value > 0)
    {
        character_controller.timers.action_launch_ticks = result;
        goto done;
    }
    this->flags = flags & 0xFEFFFFFF;
    flags = character_controller.movement_flags;
    kind = (uint16)(character_controller.motion.form_flags >> 16);
    character_controller.timers.action_cooldown_ticks = 15;
    character_controller.movement_flags = flags | 0x1000000;
    switch (kind & 15)
    {
        case 1:
            this->state = 19;
            flags = character_controller.secondary_flags;
            player_interaction.flags = 19;
            if ((flags & 0x20) != 0)
                player_interaction.flags = 51;
            break;
        case 4:
            this->state = 21;
            break;
        case 5:
        case 10:
        case 11:
            this->state = 25;
            break;
        default:
            this->state = (uint16)(character_controller.weapon_state >> 16);
            if (((uint16)(character_controller.motion.form_flags >> 16) & 15) == 0)
                path_routing.flags = path_routing.flags | 0x10;
            break;
    }
    value = this->state;
    original_flags = this->flags;
    if ((value == 19 || path_routing.flags != 0) && (((const PathDefinition *)((this->path_definition)))->flags & 0x10000) != 0)
    {
        timer = this->yaw;
        wave = (sint16)character_controller.path_transition.motion_heading;
        this->flags = timer == wave ? original_flags & 0xFFFFFFEF : original_flags | 0x10;
    }
    this->flags = this->flags | 0x100;
    created_object = object_operations.behaviors[6].factory_handler(this);
    this->flags = original_flags;
    kind = (uint16)(character_controller.motion.form_flags >> 16) & 15;
    eject = kind - 5 < 2 || kind == 7 || kind - 10 < 2;
    if (eject == 0)
    {
        if (kind == 4)
        {
            result = created_object->position[1] + ((original_flags & 0x8000000) != 0 ? 4096 : 20480);
            created_object->position[1] = result;
            goto done;
        }
        result = original_flags & 0x8000000;
        if (result == 0)
            goto done;
    }
    parent = (created_object->path_definition);
    if (parent != 0 && ((((const PathDefinition *)(parent)))->transform) != 0)
    {
        x = created_object->position[0];
        y = created_object->position[1];
        z = created_object->position[2];
        input.vx = x;
        input.vy = y;
        input.vz = z;
        PushMatrix();
        path_transform = (const PathTransform *)((((const PathDefinition *)((const PathDefinition *)((created_object->path_definition))))->transform) != 0 ? (((const PathDefinition *)((const PathDefinition *)((created_object->path_definition))))->transform) : (PathTransform *)0);
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
        ApplyRotMatrixLV(&input, (VECTOR *)created_object->position);
        PopMatrix();
        parent = (created_object->path_definition);
        path_transform = (const PathTransform *)(((((const PathDefinition *)(parent)))->transform) != 0 ? ((((const PathDefinition *)(parent)))->transform) : (PathTransform *)0);
        x = created_object->position[0];
        y = (uint32)path_transform->matrix.t[0];
        parent = (created_object->path_definition);
        created_object->position[0] = x + y;
        path_transform = (const PathTransform *)(((((const PathDefinition *)(parent)))->transform) != 0 ? ((((const PathDefinition *)(parent)))->transform) : (PathTransform *)0);
        x = created_object->position[1];
        y = (uint32)path_transform->matrix.t[1];
        parent = (created_object->path_definition);
        created_object->position[1] = x + y;
        path_transform = (const PathTransform *)(((((const PathDefinition *)(parent)))->transform) != 0 ? ((((const PathDefinition *)(parent)))->transform) : (PathTransform *)0);
        z = created_object->position[2];
        created_object->position[2] = z + (uint32)path_transform->matrix.t[2];
    }
    else
    {
        x = created_object->position[0];
        y = created_object->position[1];
        z = created_object->position[2];
        created_object->position[0] = x;
        created_object->position[1] = y;
        created_object->position[2] = z;
    }
    if (eject != 0)
    {
        angle = ((uint16)this->target_yaw + 2048) & 0xFFF;
        wave = SinQ14((sint16)angle) >> 2;
        x = created_object->position[0];
        created_object->position[0] = x + (uint32)(((sint64)12288 * wave) >> 12);
        wave = SinQ14((sint16)(1024 - angle)) >> 2;
        z = created_object->position[2];
        created_object->position[2] = z + (uint32)(((sint64)12288 * wave) >> 12);
        created_object->position[0] = created_object->position[0] + this->velocity[0];
        created_object->position[1] = created_object->position[1] + this->velocity[1];
        created_object->position[2] = created_object->position[2] + this->velocity[2];
        result = created_object->flags | 0x2000;
        created_object->flags = result;
        created_object->movement_mode = 0;
        goto done;
    }
    created_object->position[1] = created_object->position[1] - 8192;
    parent = (created_object->path_definition);
    if (parent == 0)
    {
        goto done;
    }
    path_transform = ((captured_path_transform = ((((const PathDefinition *)(parent)))->transform)) != 0 ? (const PathTransform *)((PathTransform *)(captured_path_transform)) : 0);
    if (path_transform == 0)
    {
        goto done;
    }
    relative.vx = created_object->position[0] - (uint32)path_transform->matrix.t[0];
    path_transform = (const PathTransform *)((((const PathDefinition *)((const PathDefinition *)((created_object->path_definition))))->transform) != 0 ? (((const PathDefinition *)((const PathDefinition *)((created_object->path_definition))))->transform) : (PathTransform *)0);
    relative.vy = created_object->position[1] - (uint32)path_transform->matrix.t[1];
    path_transform = (const PathTransform *)((((const PathDefinition *)((const PathDefinition *)((created_object->path_definition))))->transform) != 0 ? (((const PathDefinition *)((const PathDefinition *)((created_object->path_definition))))->transform) : (PathTransform *)0);
    relative.vz = created_object->position[2] - (uint32)path_transform->matrix.t[2];
    transpose.halves[0] = ((const MATRIX *)((((const PathDefinition *)((const PathDefinition *)((created_object->path_definition))))->transform) != 0 ? &(((const PathDefinition *)((const PathDefinition *)((created_object->path_definition))))->transform)->matrix : (MATRIX *)0))->m[0][0];
    transpose.halves[1] = ((const MATRIX *)((((const PathDefinition *)((const PathDefinition *)((created_object->path_definition))))->transform) != 0 ? &(((const PathDefinition *)((const PathDefinition *)((created_object->path_definition))))->transform)->matrix : (MATRIX *)0))->m[1][0];
    transpose.halves[2] = ((const MATRIX *)((((const PathDefinition *)((const PathDefinition *)((created_object->path_definition))))->transform) != 0 ? &(((const PathDefinition *)((const PathDefinition *)((created_object->path_definition))))->transform)->matrix : (MATRIX *)0))->m[2][0];
    transpose.halves[3] = ((const MATRIX *)((((const PathDefinition *)((const PathDefinition *)((created_object->path_definition))))->transform) != 0 ? &(((const PathDefinition *)((const PathDefinition *)((created_object->path_definition))))->transform)->matrix : (MATRIX *)0))->m[0][1];
    transpose.halves[4] = ((const MATRIX *)((((const PathDefinition *)((const PathDefinition *)((created_object->path_definition))))->transform) != 0 ? &(((const PathDefinition *)((const PathDefinition *)((created_object->path_definition))))->transform)->matrix : (MATRIX *)0))->m[1][1];
    transpose.halves[5] = ((const MATRIX *)((((const PathDefinition *)((const PathDefinition *)((created_object->path_definition))))->transform) != 0 ? &(((const PathDefinition *)((const PathDefinition *)((created_object->path_definition))))->transform)->matrix : (MATRIX *)0))->m[2][1];
    transpose.halves[6] = ((const MATRIX *)((((const PathDefinition *)((const PathDefinition *)((created_object->path_definition))))->transform) != 0 ? &(((const PathDefinition *)((const PathDefinition *)((created_object->path_definition))))->transform)->matrix : (MATRIX *)0))->m[0][2];
    transpose.halves[7] = ((const MATRIX *)((((const PathDefinition *)((const PathDefinition *)((created_object->path_definition))))->transform) != 0 ? &(((const PathDefinition *)((const PathDefinition *)((created_object->path_definition))))->transform)->matrix : (MATRIX *)0))->m[1][2];
    transpose.halves[8] = ((const MATRIX *)((((const PathDefinition *)((const PathDefinition *)((created_object->path_definition))))->transform) != 0 ? &(((const PathDefinition *)((const PathDefinition *)((created_object->path_definition))))->transform)->matrix : (MATRIX *)0))->m[2][2];
    PushMatrix();
    x = transpose.words[0];
    y = transpose.words[1];
    xport_gte_write_control(0, x);
    xport_gte_write_control(1, y);
    x = transpose.words[2];
    y = transpose.words[3];
    z = transpose.halves[8];
    xport_gte_write_control(2, x);
    xport_gte_write_control(3, y);
    xport_gte_write_control(4, z);
    ApplyRotMatrixLV(&relative, (VECTOR *)created_object->position);
    PopMatrix();
done:
    return;
}

uint32 AnimatedObject::UpdateInteractionWave(const AnimatedObject *owner)
{
    FUNCTION_MARKER(0x800289D4, "SLUS_005.78");
    uint32 flags = character_action_flags;
    uint32 amplitude = 8192;
    uint32 step[3];
    uint32 position[3] = {0, 0, 0};
    SceneNode *child_node;
    SpriteRecord *child;
    uint32 index;
    sint16 countdown;
    if ((flags & 8) == 0 || (this->flags & 0x20200) == 512)
    {
        AnimatedObject *active = character_action_target;
        uint8 object_flags;
        character_action_flags = (flags | 0x800) & ~8u;
        if (active != 0)
        {
            ActorBinding *binding = (ActorBinding *)(((const ActorPlacement *)((const PathDefinition *)((active->path_definition))))->state != 0 ? ((const ActorPlacement *)((const PathDefinition *)((active->path_definition))))->state : 0);

            binding->flags &= 0xF7;
        }
        object_flags = this->node.flags;
        this->behavior = 0;
        this->node.flags = object_flags | 2;
        return object_flags | 2;
    }
    countdown = this->path_frame;
    if (countdown != 0)
    {
        countdown = (sint16)(countdown - 1);
        this->path_frame = countdown;
        if (countdown >= 3)
            amplitude = (uint32)((4 - countdown) * 2048 + 8192);
        else if (countdown >= 0)
        {
            amplitude = 12288;
            if (countdown == 0)
                this->path_frame = -1;
        }
        else if (countdown == -5)
            this->path_frame = 0;
        else
            amplitude = (uint32)(((countdown + 6) * 4096) / 5 + 8192);
    }
    for (index = 0; index < 3; ++index)
    {
        uint32 difference = owner->world_position[index] - this->position[index];
        step[index] = (uint32)(((sint64)(sint32)difference * 256) >> 12);
    }
    {
        uint32 phase = (uint8)character_controller.action_wave_phase + 1;
        character_controller.action_wave_phase = phase;
        if ((sint8)phase >= 16)
            character_controller.action_wave_phase = 0;
    }
    child_node = this->node.links.first_child;
    for (index = 0; index < 16; ++index)
    {
        child = child_node != 0 ? (SpriteRecord *)child_node : (SpriteRecord *)0;
        uint32 wave_scale = (index < 9 ? index : 15 - index) * 1280;
        sint32 phase;
        uint32 wave;
        uint32 height;
        child->position[0] = position[0];
        child->position[1] = position[1];
        child->position[2] = position[2];
        phase = (character_controller.action_wave_phase + (sint32)index) % 16;
        wave = (uint32)(((sint64)OriginalDefaults::InteractionWave[phase] * wave_scale) >> 12);
        height = child->position[1];
        child->extent[1] = amplitude >> 5;
        child->extent[0] = amplitude >> 5;
        child->position[1] = height + wave;
        position[0] += step[0];
        position[1] += step[1];
        position[2] += step[2];
        child_node = child->node.links.sibling_next;
    }
    index = (uint16)this->frame + 1;
    this->frame = index;
    return index;
}

uint32 AnimatedObject::UpdateInteractionChain(const AnimatedObject *owner)
{
    FUNCTION_MARKER(0x80028720, "SLUS_005.78");
    uint32 flags = player_interaction.flags;
    uint32 amplitude = 8192;
    uint32 step[3];
    uint32 position[3] = {0, 0, 0};
    SceneNode *child_node;
    SpriteRecord *child;
    uint32 index;
    if ((flags & 32) == 0 || (this->flags & 0x20200) == 512)
    {
        uint8 object_flags;
        player_interaction.flags = flags & ~36u;
        player_interaction.target = 0;
        object_flags = this->node.flags;
        this->behavior = 0;
        this->node.flags = object_flags | 2;
        return object_flags | 2;
    }
    if ((flags & 8) != 0)
        this->path_frame = 4;
    else
    {
        sint16 countdown = this->path_frame;
        if (countdown != 0)
        {
            countdown = (sint16)(countdown - 1);
            this->path_frame = countdown;
            if (countdown >= 3)
                amplitude = (uint32)((4 - countdown) * 2048 + 8192);
            else if (countdown >= 0)
            {
                amplitude = 12288;
                if (countdown == 0)
                    this->path_frame = -1;
            }
            else if (countdown == -5)
                this->path_frame = 0;
            else
                amplitude = (uint32)(((countdown + 6) * 4096) / 5 + 8192);
        }
    }
    for (index = 0; index < 3; ++index)
    {
        uint32 difference = owner->world_position[index] - this->position[index];
        step[index] = (uint32)(((sint64)(sint32)difference * 256) >> 12);
    }
    child_node = this->node.links.first_child;
    for (index = 0; index < 16; ++index)
    {
        child = child_node != 0 ? (SpriteRecord *)child_node : (SpriteRecord *)0;
        uint32 weight = (index < 9 ? index : 15 - index) * 1024;
        sint16 phase;
        sint32 wave;
        uint32 height;
        child->position[0] = position[0];
        child->position[1] = position[1];
        child->position[2] = position[2];
        phase = (sint16)((index + (uint32)(sint32)this->frame) << 9);
        wave = SinQ14(phase) >> 2;
        height = child->position[1];
        child->extent[1] = amplitude >> 5;
        child->extent[0] = amplitude >> 5;
        child->position[1] = height + (uint32)(((sint64)wave * weight) >> 12);
        position[0] += step[0];
        position[1] += step[1];
        position[2] += step[2];
        child_node = child->node.links.sibling_next;
    }
    index = (uint16)this->frame + 1;
    this->frame = index;
    return index;
}

uint32 AnimatedObject::UpdateActionObject()
{
    const PathTransform *captured_path_transform;
    FUNCTION_MARKER(0x8002995C, "SLUS_005.78");
    const AnimatedObject *owner;
    const ActorBinding *context;
    const PathDefinition *parent;
    uint32 pointer, state, value, flags, x, y, z, result;
    AnimationHeader *animation;
    const PathTransform *path_transform;
    ActionVector world, local, relative;

    union
    {
        uint32 words[5];
        uint16 halves[10];
    } inverse;

    sint32 frame, previous, length, timer;
    context = this->action_context;
    if (context != 0)
        owner = context->instance;
    else
        owner = FindPlayerObject(0);
    state = this->state;
    if (state == 9)
        return UpdateInteractionChain(owner);
    if (state == 12)
        return UpdateInteractionWave(owner);
    if (state == 13 || state == 14)
    {
        const AnimatedObject *player = FindPlayerObject(0);
        animation = this->animation;
        frame = 0;
        if (animation)
        {
            previous = this->frame;
            length = ((AnimationHeader *)(animation))->frame_count;
            if (previous < length - 1)
                frame = previous + 1;
        }
        const AnimatedObject *selected = character_action_target;
        this->frame = (uint32)frame;
        if (selected != 0)
            this->UpdateActionEffect(player, state);
        goto finish;
    }
    if (state == 11)
    {
        const AnimatedObject *source = (const AnimatedObject *)(owner);
        animation = this->animation;
        frame = 0;
        if (animation)
        {
            previous = this->frame;
            length = ((AnimationHeader *)(animation))->frame_count;
            if (previous < length - 1)
                frame = previous + 1;
        }
        value = this->velocity[2];
        this->frame = (uint32)frame;
        ++value;
        this->velocity[2] = value;
        if ((sint32)value >= 4)
            goto expire;
        parent = (source->path_definition);
        path_transform = parent != 0 ? ((captured_path_transform = ((((const PathDefinition *)(parent)))->transform)) != 0 ? (const PathTransform *)((PathTransform *)(captured_path_transform)) : 0) : 0;
        if (path_transform)
        {
            x = source->position[0];
            y = source->position[1];
            z = source->position[2];
            local.words[0] = x;
            local.words[1] = y;
            local.words[2] = z;
            PushMatrix();
            path_transform = (const PathTransform *)((((const PathDefinition *)((const PathDefinition *)((source->path_definition))))->transform) != 0 ? (((const PathDefinition *)((const PathDefinition *)((source->path_definition))))->transform) : (PathTransform *)0);
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
            ApplyRotMatrixLV(&local.vector, &world.vector);
            PopMatrix();
            path_transform = (const PathTransform *)((((const PathDefinition *)((const PathDefinition *)((source->path_definition))))->transform) != 0 ? (((const PathDefinition *)((const PathDefinition *)((source->path_definition))))->transform) : (PathTransform *)0);
            x = world.words[0];
            y = (uint32)path_transform->matrix.t[0];
            world.words[0] = x + y;
            path_transform = (const PathTransform *)((((const PathDefinition *)((const PathDefinition *)((source->path_definition))))->transform) != 0 ? (((const PathDefinition *)((const PathDefinition *)((source->path_definition))))->transform) : (PathTransform *)0);
            x = world.words[1];
            y = (uint32)path_transform->matrix.t[1];
            world.words[1] = x + y;
            path_transform = (const PathTransform *)((((const PathDefinition *)((const PathDefinition *)((source->path_definition))))->transform) != 0 ? (((const PathDefinition *)((const PathDefinition *)((source->path_definition))))->transform) : (PathTransform *)0);
            x = world.words[2];
            y = (uint32)path_transform->matrix.t[2];
            world.words[2] = x + y;
        }
        else
        {
            x = source->position[0];
            y = source->position[1];
            z = source->position[2];
            world.words[0] = x;
            world.words[1] = y;
            world.words[2] = z;
        }
        parent = (this->path_definition);
        path_transform = parent != 0 ? ((captured_path_transform = ((((const PathDefinition *)(parent)))->transform)) != 0 ? (const PathTransform *)((PathTransform *)(captured_path_transform)) : 0) : 0;
        if (path_transform)
        {
            x = world.words[0];
            y = (uint32)path_transform->matrix.t[0];
            relative.words[0] = x - y;
            path_transform = (const PathTransform *)((((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) != 0 ? (((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) : (PathTransform *)0);
            x = world.words[1];
            y = (uint32)path_transform->matrix.t[1];
            relative.words[1] = x - y;
            path_transform = (const PathTransform *)((((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) != 0 ? (((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) : (PathTransform *)0);
            x = world.words[2];
            y = (uint32)path_transform->matrix.t[2];
            relative.words[2] = x - y;
            path_transform = (const PathTransform *)((((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) != 0 ? (((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) : (PathTransform *)0);
            inverse.halves[0] = path_transform->matrix.m[0][0];
            path_transform = (const PathTransform *)((((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) != 0 ? (((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) : (PathTransform *)0);
            inverse.halves[1] = path_transform->matrix.m[1][0];
            path_transform = (const PathTransform *)((((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) != 0 ? (((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) : (PathTransform *)0);
            inverse.halves[2] = path_transform->matrix.m[2][0];
            path_transform = (const PathTransform *)((((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) != 0 ? (((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) : (PathTransform *)0);
            inverse.halves[3] = path_transform->matrix.m[0][1];
            path_transform = (const PathTransform *)((((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) != 0 ? (((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) : (PathTransform *)0);
            inverse.halves[4] = path_transform->matrix.m[1][1];
            path_transform = (const PathTransform *)((((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) != 0 ? (((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) : (PathTransform *)0);
            inverse.halves[5] = path_transform->matrix.m[2][1];
            path_transform = (const PathTransform *)((((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) != 0 ? (((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) : (PathTransform *)0);
            inverse.halves[6] = path_transform->matrix.m[0][2];
            path_transform = (const PathTransform *)((((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) != 0 ? (((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) : (PathTransform *)0);
            inverse.halves[7] = path_transform->matrix.m[1][2];
            path_transform = (const PathTransform *)((((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) != 0 ? (((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) : (PathTransform *)0);
            inverse.halves[8] = path_transform->matrix.m[2][2];
            PushMatrix();
            x = inverse.words[0];
            y = inverse.words[1];
            xport_gte_write_control(0, x);
            xport_gte_write_control(1, y);
            x = inverse.words[2];
            y = inverse.words[3];
            z = inverse.halves[8];
            xport_gte_write_control(2, x);
            xport_gte_write_control(3, y);
            xport_gte_write_control(4, z);
            ApplyRotMatrixLV(&relative.vector, (VECTOR *)this->position);
            PopMatrix();
        }
        else
        {
            x = world.words[0];
            y = world.words[1];
            z = world.words[2];
            this->position[0] = x;
            this->position[1] = y;
            this->position[2] = z;
        }
        goto finish;
    }
    animation = this->animation;
    frame = 0;
    if (animation)
    {
        length = ((AnimationHeader *)(animation))->frame_count;
        previous = this->frame;
        if (previous < length - 1)
            frame = previous + 1;
    }
    state = this->state;
    if (state == 2)
    {
        x = this->position[0];
        value = this->velocity[0];
        y = this->position[1];
        flags = this->velocity[1];
        z = this->position[2];
        pointer = this->velocity[2];
        this->position[0] = x + value;
        this->position[1] = y + flags;
        this->position[2] = z + pointer;
    }
    else if (state == 8 && frame == 15)
        QueueCharacterActionSound(11, 3);
    if ((this->flags & 0x8000) && owner)
    {
        this->node.state_flags = owner->node.state_flags + 64;
        path_transform = 0;
        if (!(owner->flags & 0x2000))
        {
            parent = (owner->path_definition);
            if (parent)
                path_transform = ((captured_path_transform = ((((const PathDefinition *)(parent)))->transform)) != 0 ? (const PathTransform *)((PathTransform *)(captured_path_transform)) : 0);
        }
        if (path_transform)
        {
            x = owner->position[0];
            y = owner->position[1];
            z = owner->position[2];
            local.words[0] = x;
            local.words[1] = y;
            local.words[2] = z;
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
            ApplyRotMatrixLV(&local.vector, (VECTOR *)this->position);
            PopMatrix();
            path_transform = (const PathTransform *)((((const PathDefinition *)((const PathDefinition *)((owner->path_definition))))->transform) != 0 ? (((const PathDefinition *)((const PathDefinition *)((owner->path_definition))))->transform) : (PathTransform *)0);
            x = this->position[0];
            y = (uint32)path_transform->matrix.t[0];
            this->position[0] = x + y;
            path_transform = (const PathTransform *)((((const PathDefinition *)((const PathDefinition *)((owner->path_definition))))->transform) != 0 ? (((const PathDefinition *)((const PathDefinition *)((owner->path_definition))))->transform) : (PathTransform *)0);
            x = this->position[1];
            y = (uint32)path_transform->matrix.t[1];
            this->position[1] = x + y;
            path_transform = (const PathTransform *)((((const PathDefinition *)((const PathDefinition *)((owner->path_definition))))->transform) != 0 ? (((const PathDefinition *)((const PathDefinition *)((owner->path_definition))))->transform) : (PathTransform *)0);
            x = this->position[2];
            y = (uint32)path_transform->matrix.t[2];
            this->position[2] = x + y;
        }
        else
        {
            x = owner->position[0];
            y = owner->position[1];
            z = owner->position[2];
            this->position[0] = x;
            this->position[1] = y;
            this->position[2] = z;
        }
        x = this->position[1];
        y = owner->metadata_words72[1];
        context = this->action_context;
        this->position[1] = x + y;
        if (context == 0)
        {
            flags = owner->path_flags;
            if (flags & 1)
                ApplyHeadingPositionCorrection(owner, this);
            else if (flags & 0x100)
                ApplyCameraPositionCorrection(owner, this);
        }
        if ((owner->node.flags & 1) || (this->action_context == 0 && ((uint16)(character_controller.motion.form_flags >> 16) & 15) >= 2 && this->state - 3 < 2))
            this->node.flags = this->node.flags | 1;
        else
            this->node.flags = this->node.flags & 0xFE;
    }
    else if (frame < this->frame)
        goto expire;
    state = this->state;
    this->frame = (uint32)frame;
    if (state == 7)
    {
        timer = (sint16)character_controller.timers.model_swap_ticks;
        if (timer < 64)
        {
            value = this->node.flags;
            this->node.flags = (timer & 4) ? value & 0xFE : value | 1;
        }
    }
finish:
    result = this->node.flags & 1;
    if (!result)
        result = this->ClampAnimationFrame();
    return result;
expire:
    value = this->node.flags;
    this->behavior = 0;
    result = value | 2;
    this->node.flags = result;
    return result;
}

void NoOp8005927C(void)
{
    FUNCTION_MARKER(0x8005927C, "SLUS_005.78");
}

void NoOp80059AC4(void)
{
    FUNCTION_MARKER(0x80059AC4, "SLUS_005.78");
}

void NoOp80059ACC(void)
{
    FUNCTION_MARKER(0x80059ACC, "SLUS_005.78");
}

void NoOp80059AD4(void)
{
    FUNCTION_MARKER(0x80059AD4, "SLUS_005.78");
    /* Original function returns without side effects */
}
