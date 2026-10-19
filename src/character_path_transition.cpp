#include "game.h"
#include "original_defaults.h"

const PathTransitionRules path_transition_defaults = {
    1, 8192, 2560, 12, 24, 6, 3, 4, 10, 10, 1, 0, 0xFFFFD800, 4, 2, 512, 8192, 40960, 53248, 53248, 6, 1, 0, 2, ONE, {2048, 3072}, {2048, 2048}, 2, 12 * ONE,
};

PathTransitionRules path_transition_rules = {0};

union TransitionVector
{
    VECTOR vector;
    uint32 words[4];
};

uint32 AnimatedObject::BeginCharacterPathTransfer()
{
    const PathDefinition *owner;
    FUNCTION_MARKER(0x80057650, "SLUS_005.78");
    TransitionVector world, anchor, scene_cache, floor, sample, local, target;
    TransitionVector input, anchor_input, inverse_input, forward_input, first_floor, backup_floor;
    MATRIX transpose = {0}, final_transpose = {0};
    uint32 first_valid;
    const SceneTrigger *anchor_trigger;
    const ScenePath *path, *path_view, *current;
    uint32 result, flags, value, direction, frame, first_frame;
    const MATRIX *matrix;
    uint32 x, y, z, index;
    sint32 old_frame;
    result = this->path_flags & 0x40000;
    if (result != 0)
        goto finish_transfer;
    result = 0xFFFFFBFF;
    if ((sint8)character_controller.path_transition.phase >= 4)
        goto finish_transfer;
    flags = this->flags;
    this->velocity[2] = 0;
    this->velocity[1] = 0;
    this->velocity[0] = 0;
    this->flags = flags & 0xFFFFFBFF;
    anchor_trigger = (const SceneTrigger *)(character_controller.path_transition.anchor);
    path = (anchor_trigger->parameters.path.target);
    path_view = (const ScenePath *)(path);
    current = camera_motion.path;
    if (path == current)
    {
        result = this->flags & 0x2000;
        if (result == 0)
            goto finish_transfer;
    }
    if ((this->flags & 0x2000) == 0 && (owner = (this->path_definition)) != 0 && ((((const PathDefinition *)(owner)))->transform) != 0)
    {
        x = this->position[0];
        y = this->position[1];
        z = this->position[2];
        input.words[0] = x;
        input.words[1] = y;
        input.words[2] = z;
        PushMatrix();
        matrix = (const MATRIX *)((((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) != 0 ? &(((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform)->matrix : (MATRIX *)0);
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
        ApplyRotMatrixLV(&input.vector, &world.vector);
        PopMatrix();
        for (index = 0; index < 3; ++index)
        {
            matrix = (const MATRIX *)((((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) != 0 ? &(((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform)->matrix : (MATRIX *)0);
            value = world.words[index];
            world.words[index] = value + (uint32)matrix->t[index];
        }
    }
    else
    {
        x = this->position[0];
        y = this->position[1];
        z = this->position[2];
        world.words[0] = x;
        world.words[1] = y;
        world.words[2] = z;
    }
    owner = (path_view->definition);
    if (owner != 0 && ((((const PathDefinition *)(owner)))->transform) != 0)
    {
        anchor_trigger = (const SceneTrigger *)(character_controller.path_transition.anchor);
        x = anchor_trigger->position[0];
        y = anchor_trigger->position[1];
        z = anchor_trigger->position[2];
        anchor_input.words[0] = x;
        anchor_input.words[1] = y;
        anchor_input.words[2] = z;
        PushMatrix();
        matrix = (const MATRIX *)((((const PathDefinition *)((const PathDefinition *)((path_view->definition))))->transform) != 0 ? &(((const PathDefinition *)((const PathDefinition *)((path_view->definition))))->transform)->matrix : (MATRIX *)0);
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
        ApplyRotMatrixLV(&anchor_input.vector, &anchor.vector);
        PopMatrix();
        for (index = 0; index < 3; ++index)
        {
            matrix = (const MATRIX *)((((const PathDefinition *)((const PathDefinition *)((path_view->definition))))->transform) != 0 ? &(((const PathDefinition *)((const PathDefinition *)((path_view->definition))))->transform)->matrix : (MATRIX *)0);
            value = anchor.words[index];
            anchor.words[index] = value + (uint32)matrix->t[index];
        }
    }
    else
    {
        anchor_trigger = (const SceneTrigger *)(character_controller.path_transition.anchor);
        x = anchor_trigger->position[0];
        y = anchor_trigger->position[1];
        z = anchor_trigger->position[2];
        anchor.words[0] = x;
        anchor.words[1] = y;
        anchor.words[2] = z;
    }
    x = camera_motion.focus_position[0];
    y = camera_motion.focus_position[1];
    z = camera_motion.focus_position[2];
    character_controller.path_transition.camera_samples[0][0] = x;
    character_controller.path_transition.camera_samples[0][1] = y;
    character_controller.path_transition.camera_samples[0][2] = z;
    x = character_controller.path_transition.camera_samples[0][0];
    y = character_controller.path_transition.camera_samples[0][1];
    z = character_controller.path_transition.camera_samples[0][2];
    scene_cache.words[0] = x;
    scene_cache.words[1] = y;
    scene_cache.words[2] = z;
    x = world.words[0];
    y = world.words[1];
    z = world.words[2];
    character_controller.path_transition.start[0] = x;
    character_controller.path_transition.start[1] = y;
    character_controller.path_transition.start[2] = z;
    direction = (this->path_flags & 0x400) == 0 ? 64 : 0;
    path_view->SamplePosition(0, sample.words, direction);
    owner = (path_view->definition);
    if (owner != 0 && ((((const PathDefinition *)(owner)))->transform) != 0)
    {
        matrix = (const MATRIX *)(((((const PathDefinition *)(owner)))->transform) != 0 ? &((((const PathDefinition *)(owner)))->transform)->matrix : (MATRIX *)0);
        value = world.words[0];
        inverse_input.words[0] = value - (uint32)matrix->t[0];
        for (index = 1; index < 3; ++index)
        {
            matrix = (const MATRIX *)((((const PathDefinition *)((const PathDefinition *)((path_view->definition))))->transform) != 0 ? &(((const PathDefinition *)((const PathDefinition *)((path_view->definition))))->transform)->matrix : (MATRIX *)0);
            value = world.words[index];
            inverse_input.words[index] = value - (uint32)matrix->t[index];
        }
        for (index = 0; index < 9; ++index)
        {
            matrix = (const MATRIX *)((((const PathDefinition *)((const PathDefinition *)((path_view->definition))))->transform) != 0 ? &(((const PathDefinition *)((const PathDefinition *)((path_view->definition))))->transform)->matrix : (MATRIX *)0);
            transpose.m[index / 3][index % 3] = matrix->m[index % 3][index / 3];
        }
        PushMatrix();
        x = xport_load_le32((const uint8 *)&transpose);
        y = xport_load_le32((const uint8 *)&transpose + 4);
        xport_gte_write_control(0, x);
        xport_gte_write_control(1, y);
        x = xport_load_le32((const uint8 *)&transpose + 8);
        y = xport_load_le32((const uint8 *)&transpose + 12);
        z = (uint16)transpose.m[2][2];
        xport_gte_write_control(2, x);
        xport_gte_write_control(3, y);
        xport_gte_write_control(4, z);
        ApplyRotMatrixLV(&inverse_input.vector, &local.vector);
        PopMatrix();
    }
    else
    {
        x = world.words[0];
        y = world.words[1];
        z = world.words[2];
        local.words[0] = x;
        local.words[1] = y;
        local.words[2] = z;
    }
    flags = this->path_flags;
    value = local.words[0];
    if ((flags & 0x400) == 0)
        value += path_transition_rules.side_step;
    else
        value -= path_transition_rules.side_step;
    local.words[0] = value;
    owner = (path_view->definition);
    if (owner != 0 && ((((const PathDefinition *)(owner)))->transform) != 0)
    {
        x = local.words[0];
        y = local.words[1];
        z = local.words[2];
        forward_input.words[0] = x;
        forward_input.words[1] = y;
        forward_input.words[2] = z;
        PushMatrix();
        matrix = (const MATRIX *)((((const PathDefinition *)((const PathDefinition *)((path_view->definition))))->transform) != 0 ? &(((const PathDefinition *)((const PathDefinition *)((path_view->definition))))->transform)->matrix : (MATRIX *)0);
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
        ApplyRotMatrixLV(&forward_input.vector, &target.vector);
        PopMatrix();
        for (index = 0; index < 3; ++index)
        {
            matrix = (const MATRIX *)((((const PathDefinition *)((const PathDefinition *)((path_view->definition))))->transform) != 0 ? &(((const PathDefinition *)((const PathDefinition *)((path_view->definition))))->transform)->matrix : (MATRIX *)0);
            value = target.words[index];
            target.words[index] = value + (uint32)matrix->t[index];
        }
    }
    else
    {
        x = local.words[0];
        y = local.words[1];
        z = local.words[2];
        target.words[0] = x;
        target.words[1] = y;
        target.words[2] = z;
    }
    value = anchor.words[1];
    target.words[1] = value + 40960;
    x = camera_motion.focus_position[0];
    y = camera_motion.focus_position[1];
    z = camera_motion.focus_position[2];
    character_controller.path_transition.camera_samples[1][0] = x;
    character_controller.path_transition.camera_samples[1][1] = y;
    character_controller.path_transition.camera_samples[1][2] = z;
    x = character_controller.path_transition.camera_samples[1][0];
    y = character_controller.path_transition.camera_samples[1][1];
    z = character_controller.path_transition.camera_samples[1][2];
    character_controller.path_transition.camera_samples[3][0] = x;
    character_controller.path_transition.camera_samples[3][1] = y;
    character_controller.path_transition.camera_samples[3][2] = z;
    x = character_controller.path_transition.camera_samples[3][0];
    y = character_controller.path_transition.camera_samples[3][1];
    z = character_controller.path_transition.camera_samples[3][2];
    character_controller.path_transition.camera_samples[2][0] = x;
    character_controller.path_transition.camera_samples[2][1] = y;
    character_controller.path_transition.camera_samples[2][2] = z;
    value = character_controller.path_transition.camera_samples[1][1];
    if ((sint32)character_controller.flags < 0)
        value -= 32768;
    else
        value -= 14336;
    character_controller.path_transition.camera_samples[1][1] = value;
    x = character_controller.path_transition.camera_samples[1][0];
    y = character_controller.path_transition.camera_samples[1][1];
    z = character_controller.path_transition.camera_samples[1][2];
    character_controller.path_transition.camera_samples[0][0] = x;
    character_controller.path_transition.camera_samples[0][1] = y;
    character_controller.path_transition.camera_samples[0][2] = z;
    frame = ((const PathDefinition *)((const PathDefinition *)((path_view->definition))))->FindHeightFrame(target.words, direction, -40960);
    if ((sint32)frame >= 0)
        frame = (uint32)((const PathDefinition *)((const PathDefinition *)((path_view->definition))))->ProjectFrame((sint32)frame, target.words, floor.words, direction, 8192);
    first_valid = (sint32)frame >= 0;
    if (first_valid)
    {
        x = floor.words[0];
        y = floor.words[1];
        z = floor.words[2];
        first_floor.words[0] = x;
        first_floor.words[1] = y;
        first_floor.words[2] = z;
        first_frame = frame;
    }
    path_view->SamplePosition(0, floor.words, direction);
    x = floor.words[0];
    y = floor.words[1];
    z = floor.words[2];
    backup_floor.words[0] = x;
    backup_floor.words[1] = y;
    backup_floor.words[2] = z;
    x = floor.words[0];
    y = floor.words[1];
    z = floor.words[2];
    target.words[0] = x;
    target.words[1] = y;
    target.words[2] = z;
    frame = (uint32)((const PathDefinition *)((const PathDefinition *)((path_view->definition))))->ProjectFrame(0, target.words, floor.words, direction, 8192);
    if ((sint32)frame < 0)
    {
        frame = 0;
        x = backup_floor.words[0];
        y = backup_floor.words[1];
        z = backup_floor.words[2];
        floor.words[0] = x;
        floor.words[1] = y;
        floor.words[2] = z;
    }
    // Use the existing endpoint candidate when the height probe fails
    if (first_valid)
    {
        value = floor.words[0];
        value = value - first_floor.words[0] + 24576;
        if (value > 49152)
            goto restore_first;
        value = floor.words[2];
        value = value - first_floor.words[2] + 24576;
        if (value > 49152)
        {
        restore_first:
            x = first_floor.words[0];
            y = first_floor.words[1];
            z = first_floor.words[2];
            floor.words[0] = x;
            floor.words[1] = y;
            floor.words[2] = z;
            frame = first_frame;
        }
    }
    owner = (path_view->definition);
    this->path_frame = frame;
    this->path_definition = owner;
    x = floor.words[0];
    y = floor.words[1];
    z = floor.words[2];
    character_controller.path_transition.end[0] = x;
    character_controller.path_transition.end[1] = y;
    character_controller.path_transition.end[2] = z;
    owner = (this->path_definition);
    if (owner != 0 && ((((const PathDefinition *)(owner)))->transform) != 0)
    {
        matrix = (const MATRIX *)((((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) != 0 ? &(((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform)->matrix : (MATRIX *)0);
        value = floor.words[0];
        first_floor.words[0] = value - (uint32)matrix->t[0];
        for (index = 1; index < 3; ++index)
        {
            matrix = (const MATRIX *)((((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) != 0 ? &(((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform)->matrix : (MATRIX *)0);
            value = floor.words[index];
            first_floor.words[index] = value - (uint32)matrix->t[index];
        }
        for (index = 0; index < 9; ++index)
        {
            matrix = (const MATRIX *)((((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) != 0 ? &(((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform)->matrix : (MATRIX *)0);
            final_transpose.m[index / 3][index % 3] = matrix->m[index % 3][index / 3];
        }
        PushMatrix();
        x = xport_load_le32((const uint8 *)&final_transpose);
        y = xport_load_le32((const uint8 *)&final_transpose + 4);
        xport_gte_write_control(0, x);
        xport_gte_write_control(1, y);
        x = xport_load_le32((const uint8 *)&final_transpose + 8);
        y = xport_load_le32((const uint8 *)&final_transpose + 12);
        z = (uint16)final_transpose.m[2][2];
        xport_gte_write_control(2, x);
        xport_gte_write_control(3, y);
        xport_gte_write_control(4, z);
        ApplyRotMatrixLV(&first_floor.vector, (VECTOR *)this->position);
        PopMatrix();
    }
    else
    {
        x = floor.words[0];
        y = floor.words[1];
        z = floor.words[2];
        this->position[0] = x;
        this->position[1] = y;
        this->position[2] = z;
    }
    x = world.words[0];
    y = world.words[1];
    z = world.words[2];
    camera_motion.focus_position[0] = x;
    camera_motion.focus_position[1] = y;
    camera_motion.focus_position[2] = z;
    flags = character_controller.flags;
    value = camera_motion.focus_position[1];
    if ((sint32)flags < 0)
        value += 32768;
    else
        value += 14336;
    camera_motion.focus_position[1] = value;
    old_frame = this->path_frame;
    current = camera_motion.path;
    flags = this->flags;
    owner = (((const ScenePath *)(current))->definition);
    value = (((const PathDefinition *)(owner)))->FindHeightFrame((const uint32 *)camera_motion.focus_position, flags & 0x10, -40960);
    this->path_frame = value;
    this->UpdateObjectScenePath(path, old_frame);
    character_controller.path_transition.path = path;
    result = this->flags & 0xFFFFDFFF;
    this->flags = result;
finish_transfer:
    return result;
}

uint32 *UpdateCharacterCameraTarget(uint32 *destination, const AnimatedObject *object)
{
    const PathDefinition *parent;
    const PathDefinition *object_definition;
    const PathTransform *object_transform;
    const AnimationHeader *active_animation;
    FUNCTION_MARKER(0x80059AEC, "SLUS_005.78");
    const ScenePath *camera_path;
    const PathDefinition *camera_definition;
    TransitionVector local, world;
    uint32 operation, value, counter;
    const MATRIX *matrix;
    uint32 x, y, z, previous_x, previous_y, previous_z, dx, dy, dz, enabled;
    sint32 duration, threshold;
    operation = 0;
    if ((object->path_flags & 0x40000) != 0)
    {
        if ((sint8)character_controller.motion.state_code == 20)
        {
            duration = object->frame + 1;
            operation = 3;
        }
        else if ((object->flags & 0x2000) != 0)
        {
            x = object->position[0];
            y = object->position[1];
            z = object->position[2];
            goto publish;
        }
    }
    else
    {
        counter = character_controller.path_transition.camera_ticks;
        threshold = (sint8)path_transition_rules.camera_duration;
        if (counter < (uint32)threshold)
        {
            enabled = 0;
            if ((sint8)path_transition_rules.wait_animation_start == 0)
                enabled = 1;
            else if ((sint8)character_controller.motion.state_code == 20 && object->frame != 0)
            {
                active_animation = object->animation;
                counter = path_transition_rules.animation_tail_frames;
                value = (uint8)((const AnimationHeader *)(active_animation))->frame_count;
                path_transition_rules.camera_duration = value - counter;
                enabled = 1;
            }
            if (enabled != 0)
            {
                threshold = (sint8)path_transition_rules.camera_duration;
                counter = character_controller.path_transition.camera_ticks;
                previous_x = character_controller.path_transition.camera_samples[0][0];
                previous_y = character_controller.path_transition.camera_samples[0][1];
                previous_z = character_controller.path_transition.camera_samples[0][2];
                duration = threshold - (sint32)counter + 1;
                if (duration <= 0)
                    duration = 1;
                ++counter;
                character_controller.path_transition.camera_ticks = counter;
                if ((counter & 0xFF) != 0)
                {
                    previous_x = camera_motion.focus_position[0];
                    previous_y = camera_motion.focus_position[1];
                    previous_z = camera_motion.focus_position[2];
                }
                operation = 1;
            }
            else
            {
                camera_path = camera_motion.path;
                camera_definition = camera_path->definition;
                if ((camera_definition->transform) == 0)
                {
                    x = character_controller.path_transition.camera_samples[1][0];
                    y = character_controller.path_transition.camera_samples[1][1];
                    z = character_controller.path_transition.camera_samples[1][2];
                    goto publish;
                }
                operation = 2;
            }
        }
    }
    parent = (object->path_definition);
    if (parent != 0 && (parent->transform) != 0)
    {
        x = object->position[0];
        y = object->position[1];
        z = object->position[2];
        local.words[0] = x;
        local.words[1] = y;
        local.words[2] = z;
        PushMatrix();
        object_definition = object->path_definition;
        object_transform = object_definition->transform;
        matrix = object_transform != 0 ? &object_transform->matrix : (const MATRIX *)0;
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
        ApplyRotMatrixLV(&local.vector, &world.vector);
        PopMatrix();
        object_definition = object->path_definition;
        object_transform = object_definition->transform;
        matrix = object_transform != 0 ? &object_transform->matrix : (const MATRIX *)0;
        x = world.words[0];
        x += (uint32)matrix->t[0];
        world.words[0] = x;
        object_definition = object->path_definition;
        object_transform = object_definition->transform;
        matrix = object_transform != 0 ? &object_transform->matrix : (const MATRIX *)0;
        y = world.words[1];
        y += (uint32)matrix->t[1];
        world.words[1] = y;
        object_definition = object->path_definition;
        object_transform = object_definition->transform;
        matrix = object_transform != 0 ? &object_transform->matrix : (const MATRIX *)0;
        z = world.words[2];
        z += (uint32)matrix->t[2];
        world.words[2] = z;
    }
    else
    {
        x = object->position[0];
        y = object->position[1];
        z = object->position[2];
    }
    if (operation == 1)
    {
        dx = x - previous_x;
        dy = y - previous_y;
        dz = z - previous_z;
        x = previous_x + (uint32)((sint32)dx / duration);
        y = previous_y + (uint32)((sint32)dy / duration);
        z = previous_z + (uint32)((sint32)dz / duration);
    }
    else if (operation == 2)
    {
        dx = x - character_controller.path_transition.end[0];
        dy = y - character_controller.path_transition.end[1];
        dz = z - character_controller.path_transition.end[2];
        x = character_controller.path_transition.camera_samples[2][0];
        y = character_controller.path_transition.camera_samples[2][1];
        z = character_controller.path_transition.camera_samples[2][2];
        x += dx;
        y += dy;
        z += dz;
    }
    else if (operation == 3)
    {
        if (duration == 0)
            xport_mips_break(7);
        y += (uint32)(-32768 / duration);
    }
publish:
    character_controller.path_transition.camera_samples[1][0] = x;
    character_controller.path_transition.camera_samples[1][1] = y;
    character_controller.path_transition.camera_samples[1][2] = z;
    destination[0] = x;
    destination[1] = y;
    destination[2] = z;
    return destination;
}

void AnimatedObject::CompleteCharacterPathTransfer(CharacterController *controller)
{
    const PathDefinition *definition;
    FUNCTION_MARKER(0x80056884, "SLUS_005.78");
    uint32 saved_path_matches;
    TransitionVector world, input, position_vector, floor_vector;
    MATRIX inverse = {0};
    uint32 *start, *phase_ticks_slot;
    uint8 *phase_slot;
    const uint32 *end;
    uint32 flags = this->path_flags;
    uint32 special = flags & 0x400000;
    const ScenePath *path, *path_view;
    uint32 index, result;
    const MATRIX *matrix;
    uint32 position[3], delta[3];
    if (controller == 0)
        return;
    phase_slot = &controller->path_transition.phase;
    if (special == 0)
    {
        if ((sint8)*phase_slot != 7)
            return;
        result = this->flags;
        this->path_flags = flags & 0xFFFFFC37;
        this->flags = result | 0x400;
        controller->flags = controller->flags | 0x2000;
    }
    else if ((sint8)*phase_slot != 10)
        return;
    start = controller->path_transition.start;
    if ((((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) != 0)
    {
        path = camera_motion.path;
        path_view = (const ScenePath *)(path);
        definition = (path_view->definition);
        if (definition != 0 && ((((const PathDefinition *)(definition)))->transform) != 0)
        {
            for (index = 0; index < 3; ++index)
                position[index] = this->position[index];
            for (index = 0; index < 3; ++index)
                input.words[index] = position[index];
            PushMatrix();
            path = camera_motion.path;
            path_view = (const ScenePath *)(path);
            matrix = (const MATRIX *)((((const PathDefinition *)((const PathDefinition *)((path_view->definition))))->transform) != 0 ? &(((const PathDefinition *)((const PathDefinition *)((path_view->definition))))->transform)->matrix : (MATRIX *)0);
            for (index = 0; index < 5; ++index)
                xport_gte_write_control(index, xport_load_le32((const uint8 *)matrix + index * 4));
            ApplyRotMatrixLV(&input.vector, &world.vector);
            PopMatrix();
            for (index = 0; index < 3; ++index)
            {
                path = camera_motion.path;
                path_view = (const ScenePath *)(path);
                matrix = (const MATRIX *)((((const PathDefinition *)((const PathDefinition *)((path_view->definition))))->transform) != 0 ? &(((const PathDefinition *)((const PathDefinition *)((path_view->definition))))->transform)->matrix : (MATRIX *)0);
                world.words[index] = world.words[index] + (uint32)matrix->t[index];
            }
        }
        else
        {
            for (index = 0; index < 3; ++index)
                position[index] = this->position[index];
            for (index = 0; index < 3; ++index)
                world.words[index] = position[index];
        }
        end = controller->path_transition.end;
        for (index = 0; index < 3; ++index)
            delta[index] = end[index] - start[index];
        for (index = 0; index < 3; ++index)
            position[index] = world.words[index] - delta[index];
        for (index = 0; index < 3; ++index)
            start[index] = position[index];
    }
    definition = (this->path_definition);
    if (definition != 0 && ((((const PathDefinition *)(definition)))->transform) != 0)
    {
        for (index = 0; index < 3; ++index)
        {
            matrix = (const MATRIX *)((((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) != 0 ? &(((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform)->matrix : (MATRIX *)0);
            input.words[index] = start[index] - (uint32)matrix->t[index];
        }
        for (index = 0; index < 9; ++index)
        {
            matrix = (const MATRIX *)((((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) != 0 ? &(((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform)->matrix : (MATRIX *)0);
            inverse.m[index / 3][index % 3] = matrix->m[index % 3][index / 3];
        }
        PushMatrix();
        for (index = 0; index < 5; ++index)
            xport_gte_write_control(index, xport_load_le32((const uint8 *)&inverse + index * 4));
        ApplyRotMatrixLV(&input.vector, (VECTOR *)this->position);
        PopMatrix();
    }
    else
    {
        for (index = 0; index < 3; ++index)
            position[index] = start[index];
        for (index = 0; index < 3; ++index)
            this->position[index] = position[index];
    }
    for (index = 0; index < 3; ++index)
        position[index] = start[index];
    for (index = 0; index < 3; ++index)
        position_vector.words[index] = position[index];
    for (index = 0; index < 3; ++index)
        floor_vector.words[index] = position[index];
    if (special == 0)
    {
        ReverseScenePath();
        if (OriginalDefaults::MarkTransferredPath != 0)
            this->path_flags = this->path_flags | 0x2000000;
    }
    else
        this->path_flags = this->path_flags | 0x2200000;
    path = camera_motion.path;
    path_view = (const ScenePath *)(path);
    ChangeActorPath(path, position_vector.words, floor_vector.words);
    if (special == 0)
    {
        ReverseScenePath();
        /* The original normal branch keeps its transition flag zero */
        this->path_flags = this->path_flags & ~0x40000u;
    }
    path = camera_motion.path;
    path_view = (const ScenePath *)(path);
    const ScenePath *saved_path = controller->path_transition.path;
    saved_path_matches = path == saved_path;
    if (saved_path_matches != 0)
    {
        result = this->flags | 0x2000;
        this->flags = result;
    }
    if (special != 0)
    {
        result = 11;
        *phase_slot = result;
        phase_ticks_slot = &controller->path_transition.phase_ticks;
        *phase_ticks_slot = 0;
    }
}

uint32 AnimatedObject::ExitCharacterPathTransition()
{
    const PathDefinition *definition;
    FUNCTION_MARKER(0x80057208, "SLUS_005.78");
    const ScenePath *camera_path;
    TransitionVector position, floor, local;
    uint32 flags, state, x, y, z, value;
    const MATRIX *matrix;
    flags = this->path_flags;
    state = this->flags;
    this->path_flags = flags & 0xFFFFFC37;
    this->flags = state | 0x400;
    if ((sint8)character_controller.motion.state_code == 20 && this->frame == 0)
    {
        this->path_flags = this->path_flags | 0x400000;
        character_controller.path_transition.phase = 10;
    }
    else
    {
        this->ReverseScenePath();
        definition = (((const ScenePath *)(camera_motion.path))->definition);
        if (definition != 0 && ((((const PathDefinition *)(definition)))->transform) != 0)
        {
            x = this->position[0];
            y = this->position[1];
            z = this->position[2];
            local.words[0] = x;
            local.words[1] = y;
            local.words[2] = z;
            PushMatrix();
            matrix = &camera_motion.path->definition->transform->matrix;
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
            ApplyRotMatrixLV(&local.vector, &position.vector);
            PopMatrix();
            matrix = &camera_motion.path->definition->transform->matrix;
            x = position.words[0];
            x += (uint32)matrix->t[0];
            position.words[0] = x;
            matrix = &camera_motion.path->definition->transform->matrix;
            y = position.words[1];
            y += (uint32)matrix->t[1];
            position.words[1] = y;
            matrix = &camera_motion.path->definition->transform->matrix;
            z = position.words[2];
            z += (uint32)matrix->t[2];
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
        x = position.words[0];
        y = position.words[1];
        z = position.words[2];
        floor.words[0] = x;
        floor.words[1] = y;
        floor.words[2] = z;
        position.words[1] = position.words[1] - 0x4000;
        flags = this->path_flags;
        this->path_flags = flags | 0x02200000;
        this->ChangeActorPath(camera_motion.path, position.words, floor.words);
        camera_path = camera_motion.path;
        if (camera_path == character_controller.path_transition.path)
            this->flags = this->flags | 0x2000;
    }
    value = path_transition_rules.reentry_ticks;
    character_controller.path_transition.reentry_ticks = value;
    return value;
}

uint32 AnimatedObject::BeginCharacterPathTransition()
{
    FUNCTION_MARKER(0x80056700, "SLUS_005.78");
    uint32 flags, state;
    flags = this->path_flags;
    state = this->flags;
    this->path_flags = flags | 0x100;
    this->flags = state & 0xFFFFFBFF;
    flags = character_controller.flags;
    character_controller.path_transition.phase = 1;
    character_controller.path_transition.approach_distance = 0xFFFFFFFF;
    character_controller.path_transition.camera_ticks = 0;
    character_controller.path_transition.hold_ticks = 0;
    character_controller.path_transition.release_pending = 0;
    character_controller.motion.jump_phase = -1;
    character_controller.flags = flags & 0xE7FFFFBF;
    return 0xFFFFFFFF;
}

uint32 AnimatedObject::RequestCharacterPathExit()
{
    FUNCTION_MARKER(0x80056810, "SLUS_005.78");
    this->path_flags = this->path_flags | 0x40000;
    character_controller.path_transition.phase = 6;
    return 6;
}

uint32 AnimatedObject::UpdateCharacterPathTimeout()
{
    FUNCTION_MARKER(0x8005B588, "SLUS_005.78");
    uint32 result, limit, count;
    if (!character_controller.path_transition.movement_ticks)
        character_controller.timers.jump_input_ticks = 0xFFFF;
    result = character_controller.timers.jump_input_ticks;
    limit = path_transition_rules.early_movement_ticks;
    count = character_controller.path_transition.movement_ticks;
    ++result;
    character_controller.timers.jump_input_ticks = result;
    if (count >= limit)
    {
        this->path_flags = this->path_flags & 0xFFFFFFB1;
        character_controller.secondary_flags = character_controller.secondary_flags & 0xFFFFFFFB;
        this->flags = this->flags | 0x400;
        result = character_controller.flags | 0x2000;
        character_controller.flags = result;
    }
    return result;
}

uint32 CanContinuePathStep(void)
{
    FUNCTION_MARKER(0x80056328, "SLUS_005.78");
    uint32 limit, count;
    limit = path_transition_rules.early_movement_ticks;
    count = character_controller.path_transition.movement_ticks;
    return count < limit;
}

void ApplyCameraPositionCorrection(const AnimatedObject *owner, AnimatedObject *object)
{
    FUNCTION_MARKER(0x80059284, "SLUS_005.78");
    TransitionVector world, input;
    MATRIX inverse = {0};
    uint32 position[3], movement[3];
    const PathDefinition *definition;
    const MATRIX *matrix;
    uint32 index;
    if ((sint8)path_transition_rules.follow_enabled == 0)
        return;
    if ((sint8)path_transition_rules.follow_limit < (sint32)character_controller.path_transition.camera_ticks)
        return;
    character_controller.path_transition.correction_mode = 1;
    definition = (object->path_definition);
    if (definition != 0 && (definition->transform) != 0)
    {
        for (index = 0; index < 3; ++index)
            position[index] = object->position[index];
        for (index = 0; index < 3; ++index)
            input.words[index] = position[index];
        PushMatrix();
        matrix = (const MATRIX *)((((const PathDefinition *)((const PathDefinition *)((object->path_definition))))->transform) != 0 ? &(((const PathDefinition *)((const PathDefinition *)((object->path_definition))))->transform)->matrix : (MATRIX *)0);
        for (index = 0; index < 5; ++index)
            xport_gte_write_control(index, xport_load_le32((const uint8 *)matrix + index * 4));
        ApplyRotMatrixLV(&input.vector, &world.vector);
        PopMatrix();
        for (index = 0; index < 3; ++index)
        {
            matrix = (const MATRIX *)((((const PathDefinition *)((const PathDefinition *)((object->path_definition))))->transform) != 0 ? &(((const PathDefinition *)((const PathDefinition *)((object->path_definition))))->transform)->matrix : (MATRIX *)0);
            world.words[index] = world.words[index] + (uint32)matrix->t[index];
        }
    }
    else
    {
        for (index = 0; index < 3; ++index)
            position[index] = object->position[index];
        for (index = 0; index < 3; ++index)
            world.words[index] = position[index];
    }
    for (index = 0; index < 3; ++index)
        movement[index] = character_controller.path_transition.camera_samples[1][index] - world.words[index];
    movement[1] += path_transition_rules.follow_height;
    if ((sint8)path_transition_rules.follow_side_ticks >= (sint32)character_controller.path_transition.camera_ticks)
    {
        uint32 heading = character_controller.path_transition.heading;
        if (heading - 512u >= 0xC01)
            movement[2] -= path_transition_rules.follow_side_offset;
        else if ((sint16)heading < 1536)
            movement[0] += path_transition_rules.follow_side_offset;
        else if ((sint16)heading < 2560)
            movement[2] += path_transition_rules.follow_side_offset;
        else
            movement[0] -= path_transition_rules.follow_side_offset;
    }
    for (index = 0; index < 3; ++index)
        world.words[index] = world.words[index] + movement[index];
    for (index = 0; index < 3; ++index)
        character_controller.path_transition.correction[index] = movement[index];
    definition = (object->path_definition);
    if (definition != 0 && (definition->transform) != 0)
    {
        for (index = 0; index < 3; ++index)
        {
            matrix = (const MATRIX *)((((const PathDefinition *)((const PathDefinition *)((object->path_definition))))->transform) != 0 ? &(((const PathDefinition *)((const PathDefinition *)((object->path_definition))))->transform)->matrix : (MATRIX *)0);
            input.words[index] = world.words[index] - (uint32)matrix->t[index];
        }
        for (index = 0; index < 9; ++index)
        {
            matrix = (const MATRIX *)((((const PathDefinition *)((const PathDefinition *)((object->path_definition))))->transform) != 0 ? &(((const PathDefinition *)((const PathDefinition *)((object->path_definition))))->transform)->matrix : (MATRIX *)0);
            inverse.m[index / 3][index % 3] = matrix->m[index % 3][index / 3];
        }
        PushMatrix();
        for (index = 0; index < 5; ++index)
            xport_gte_write_control(index, xport_load_le32((const uint8 *)&inverse + index * 4));
        ApplyRotMatrixLV(&input.vector, (VECTOR *)object->position);
        PopMatrix();
    }
    else
    {
        for (index = 0; index < 3; ++index)
            position[index] = world.words[index];
        for (index = 0; index < 3; ++index)
            object->position[index] = position[index];
    }
    return;
}

void ApplyHeadingPositionCorrection(const AnimatedObject *owner, AnimatedObject *object)
{
    FUNCTION_MARKER(0x800596CC, "SLUS_005.78");
    TransitionVector world, input;
    MATRIX inverse = {0};
    uint32 position[3];
    const PathDefinition *definition;
    const MATRIX *matrix;
    uint32 index, amplitude, step_x, step_z;
    sint16 heading;
    sint32 state;
    if ((sint8)path_transition_rules.follow_enabled == 0)
        return;
    character_controller.path_transition.correction_mode = 2;
    definition = (object->path_definition);
    if (definition != 0 && (definition->transform) != 0)
    {
        for (index = 0; index < 3; ++index)
            position[index] = object->position[index];
        for (index = 0; index < 3; ++index)
            input.words[index] = position[index];
        PushMatrix();
        matrix = (const MATRIX *)((((const PathDefinition *)((const PathDefinition *)((object->path_definition))))->transform) != 0 ? &(((const PathDefinition *)((const PathDefinition *)((object->path_definition))))->transform)->matrix : (MATRIX *)0);
        for (index = 0; index < 5; ++index)
            xport_gte_write_control(index, xport_load_le32((const uint8 *)matrix + index * 4));
        ApplyRotMatrixLV(&input.vector, &world.vector);
        PopMatrix();
        for (index = 0; index < 3; ++index)
        {
            matrix = (const MATRIX *)((((const PathDefinition *)((const PathDefinition *)((object->path_definition))))->transform) != 0 ? &(((const PathDefinition *)((const PathDefinition *)((object->path_definition))))->transform)->matrix : (MATRIX *)0);
            world.words[index] = world.words[index] + (uint32)matrix->t[index];
        }
    }
    else
    {
        for (index = 0; index < 3; ++index)
            position[index] = object->position[index];
        for (index = 0; index < 3; ++index)
            world.words[index] = position[index];
    }
    state = (sint8)character_controller.motion.state_code;
    amplitude = state == 11 ? 4096 : (state == 10 ? 2048 : 3072);
    heading = (owner->yaw);
    step_x = (uint32)(((sint64)(SinQ14(heading) >> 2) * amplitude) >> 12);
    step_z = (uint32)(((sint64)(SinQ14((sint16)(1024 - heading)) >> 2) * amplitude) >> 12);
    position[0] = world.words[0] + step_x;
    position[1] = world.words[1];
    position[2] = world.words[2] + step_z;
    for (index = 0; index < 3; ++index)
        world.words[index] = position[index];
    definition = (object->path_definition);
    if (definition != 0 && (definition->transform) != 0)
    {
        for (index = 0; index < 3; ++index)
        {
            matrix = (const MATRIX *)((((const PathDefinition *)((const PathDefinition *)((object->path_definition))))->transform) != 0 ? &(((const PathDefinition *)((const PathDefinition *)((object->path_definition))))->transform)->matrix : (MATRIX *)0);
            input.words[index] = position[index] - (uint32)matrix->t[index];
        }
        for (index = 0; index < 9; ++index)
        {
            matrix = (const MATRIX *)((((const PathDefinition *)((const PathDefinition *)((object->path_definition))))->transform) != 0 ? &(((const PathDefinition *)((const PathDefinition *)((object->path_definition))))->transform)->matrix : (MATRIX *)0);
            inverse.m[index / 3][index % 3] = matrix->m[index % 3][index / 3];
        }
        PushMatrix();
        for (index = 0; index < 5; ++index)
            xport_gte_write_control(index, xport_load_le32((const uint8 *)&inverse + index * 4));
        ApplyRotMatrixLV(&input.vector, (VECTOR *)object->position);
        PopMatrix();
    }
    else
    {
        for (index = 0; index < 3; ++index)
            object->position[index] = position[index];
    }
    return;
}

const ScenePath *AnimatedObject::BeginCharacterSurfaceMotion()
{
    FUNCTION_MARKER(0x80058CA0, "SLUS_005.78");
    uint32 flags, object_flags, character_flags, scene;
    const ScenePath *result;
    flags = this->path_flags;
    object_flags = this->flags;
    this->path_flags = (flags & 0xFFFFFFB0) | 0x42;
    this->flags = object_flags & 0xFFFFFBFF;
    SceneTrigger *surface = character_controller.surface_interaction.surface;
    character_flags = character_controller.secondary_flags;
    character_controller.surface_interaction.surface_phase = 0;
    character_controller.surface_interaction.prev_surface_step = 0;
    character_controller.surface_interaction.surface_step = 0;
    character_controller.surface_interaction.prev_surface = surface;
    character_controller.secondary_flags = character_flags | 4;
    flags = this->path_flags;
    scene = OriginalDefaults::SceneContextIdentity;
    this->path_flags = flags | 12;
    character_controller.path_transition.movement_ticks = 0;
    result = camera_motion.path;
    character_controller.path_transition.anchor_path = result;
    return result;
}

uint32 AnimatedObject::BeginCharacterConstrainedPath()
{
    FUNCTION_MARKER(0x8005873C, "SLUS_005.78");
    TransitionVector world, first, second, delta, sample, destination, temporary, relative;
    MATRIX inverse;
    const PathDefinition *definition;
    const MATRIX *matrix;
    const ScenePath *selected, *route;
    uint32 flags, value, x, y, z, a, b, c, result, index;
    sint32 frame, previous, count;
    character_controller.flags = character_controller.flags & 0xF7FFFFFF;
    this->path_flags = this->path_flags | 1;
    flags = this->flags;
    value = this->path_flags;
    this->flags = flags & 0xFFFFFBFF;
    this->path_flags = value & 0xFFFFFFB1;
    SceneTrigger *surface = character_controller.surface_interaction.surface;
    character_controller.surface_interaction.surface_phase = 0;
    character_controller.surface_interaction.constrained_reset = 0;
    character_controller.path_transition.input_ready = 0;
    character_controller.surface_interaction.prev_surface = surface;
    character_controller.path_transition.entry_flags = this->flags;
    selected = camera_motion.path;
    route = (selected->adjacent[1]);
    character_controller.path_input.dir = 0;
    character_controller.path_input.settle_ticks = 0;
    character_controller.path_input.latch_phase = 0;
    character_controller.path_input.release_ticks = 0;
    character_controller.path_input.prev_motion = 0;
    character_controller.path_input.latched_motion = 0;
    definition = (this->path_definition);
    matrix = definition != 0 && (definition->transform) != 0 ? (const MATRIX *)((definition->transform) != 0 ? &(definition->transform)->matrix : (MATRIX *)0) : 0;
    if (matrix)
    {
        x = this->position[0];
        y = this->position[1];
        z = this->position[2];
        temporary.words[0] = x;
        temporary.words[1] = y;
        temporary.words[2] = z;
        PushMatrix();
        matrix = (const MATRIX *)((((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) != 0 ? &(((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform)->matrix : (MATRIX *)0);
        x = xport_load_le32(matrix);
        y = xport_load_le32((const uint8 *)matrix + 4);
        xport_gte_write_control(0, x);
        xport_gte_write_control(1, y);
        x = xport_load_le32((const uint8 *)matrix + 8);
        y = xport_load_le32((const uint8 *)matrix + 12);
        z = xport_load_le32((const uint8 *)matrix + 16);
        xport_gte_write_control(2, x);
        xport_gte_write_control(3, y);
        xport_gte_write_control(4, z);
        ApplyRotMatrixLV(&temporary.vector, &world.vector);
        PopMatrix();
        matrix = (const MATRIX *)((((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) != 0 ? &(((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform)->matrix : (MATRIX *)0);
        x = world.words[0];
        y = (uint32)matrix->t[0];
        world.words[0] = x + y;
        matrix = (const MATRIX *)((((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) != 0 ? &(((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform)->matrix : (MATRIX *)0);
        x = world.words[1];
        y = (uint32)matrix->t[1];
        world.words[1] = x + y;
        matrix = (const MATRIX *)((((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) != 0 ? &(((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform)->matrix : (MATRIX *)0);
        x = world.words[2];
        y = (uint32)matrix->t[2];
        world.words[2] = x + y;
    }
    else
    {
        x = this->position[0];
        y = this->position[1];
        z = this->position[2];
        world.words[0] = x;
        world.words[1] = y;
        world.words[2] = z;
    }
    frame = (sint32)((const PathDefinition *)((((const ScenePath *)(route))->definition) != 0 ? (((const ScenePath *)(route))->definition) : (const PathDefinition *)0))->FindHeightFrame(world.words, 64, -40960);
    if (frame != 0)
    {
        count = (sint16)((const PathDefinition *)((((const ScenePath *)(route))->definition) != 0 ? (((const ScenePath *)(route))->definition) : (const PathDefinition *)0))->positions.metadata6;
        value = (uint32)count - (uint32)frame - 1;
        if ((uint32)frame != value)
            frame = (sint32)((uint32)frame + 1);
        previous = (sint32)((uint32)frame - 1);
    }
    else
    {
        count = (sint16)((const PathDefinition *)((((const ScenePath *)(route))->definition) != 0 ? (((const ScenePath *)(route))->definition) : (const PathDefinition *)0))->positions.metadata6;
        frame = count - 1;
        previous = count - 2;
    }
    ((const ScenePath *)(route))->SamplePosition(previous, first.words, 64);
    ((const ScenePath *)(route))->SamplePosition(frame, second.words, 64);
    x = second.words[0];
    a = first.words[0];
    y = second.words[1];
    b = first.words[1];
    z = second.words[2];
    c = first.words[2];
    x -= a;
    y -= b;
    z -= c;
    delta.words[0] = x;
    delta.words[1] = y;
    delta.words[2] = z;
    value = Atan2Angle((sint32)(0 - z), (sint32)(0 - x));
    selected = camera_motion.path;
    character_controller.path_transition.tangent_heading = value;
    character_controller.path_transition.motion_heading = value;
    character_controller.path_transition.constrained_path = selected;
    selected = camera_motion.path;
    ((const ScenePath *)(selected))->SamplePosition(0, sample.words, 64);
    definition = (((const ScenePath *)(camera_motion.path))->definition);
    matrix = definition != 0 && (definition->transform) != 0 ? (const MATRIX *)((definition->transform) != 0 ? &(definition->transform)->matrix : (MATRIX *)0) : 0;
    if (matrix)
    {
        x = sample.words[0];
        y = (uint32)matrix->t[0];
        relative.words[0] = x - y;
        matrix = &camera_motion.path->definition->transform->matrix;
        x = sample.words[1];
        y = (uint32)matrix->t[1];
        relative.words[1] = x - y;
        matrix = &camera_motion.path->definition->transform->matrix;
        x = sample.words[2];
        y = (uint32)matrix->t[2];
        relative.words[2] = x - y;
        for (index = 0; index < 9; ++index)
        {
            matrix = &camera_motion.path->definition->transform->matrix;
            inverse.m[index / 3][index % 3] = matrix->m[index % 3][index / 3];
        }
        PushMatrix();
        x = xport_load_le32((const uint8 *)&inverse + 0);
        y = xport_load_le32((const uint8 *)&inverse + 4);
        xport_gte_write_control(0, x);
        xport_gte_write_control(1, y);
        x = xport_load_le32((const uint8 *)&inverse + 8);
        y = xport_load_le32((const uint8 *)&inverse + 12);
        z = (uint16)inverse.m[2][2];
        xport_gte_write_control(2, x);
        xport_gte_write_control(3, y);
        xport_gte_write_control(4, z);
        ApplyRotMatrixLV(&relative.vector, &destination.vector);
        PopMatrix();
    }
    else
    {
        x = sample.words[0];
        y = sample.words[1];
        z = sample.words[2];
        destination.words[0] = x;
        destination.words[1] = y;
        destination.words[2] = z;
    }
    this->position[0] = destination.words[0];
    result = destination.words[2];
    this->position[2] = result;
    return result;
}

const ScenePath *AnimatedObject::ArmCharacterPathHold()
{
    FUNCTION_MARKER(0x800585E0, "SLUS_005.78");
    uint32 flags, scene;
    const ScenePath *result;
    flags = this->path_flags;
    scene = OriginalDefaults::SceneContextIdentity;
    this->path_flags = (flags | 0x10) & 0xFFFFFF91;
    character_controller.path_transition.hold_motion_ticks = 0;
    result = camera_motion.path;
    character_controller.path_transition.anchor_path = result;
    return result;
}

uint32 AnimatedObject::EndCharacterPathHold()
{
    FUNCTION_MARKER(0x80058618, "SLUS_005.78");
    uint32 result;
    result = this->path_flags & 0xFFFFFFEF;
    this->path_flags = result;
    character_controller.path_transition.hold_motion_ticks = 0;
    character_controller.path_transition.movement_ticks = 0;
    return result;
}

uint32 AnimatedObject::BeginCharacterPathRelease()
{
    FUNCTION_MARKER(0x80058640, "SLUS_005.78");
    const ScenePath *path;
    uint32 flags, scene;
    sint32 mode, first, second;
    this->path_flags = this->path_flags & 0xFFFFFFEF;
    character_controller.path_transition.hold_motion_ticks = 0;
    character_controller.path_transition.movement_ticks = 0;
    flags = this->path_flags;
    scene = OriginalDefaults::SceneContextIdentity;
    this->path_flags = (flags | 0x68) & 0xFFFF7FFF;
    path = camera_motion.path;
    mode = path_transition_rules.release_mode;
    character_controller.path_transition.hold_motion_ticks = 0;
    character_controller.path_transition.anchor_path = path;
    if (mode == 1)
    {
        first = character_controller.movement.speed_limits[1];
        second = path_transition_rules.release_speed_scale;
        character_controller.motion.vertical_speed = (uint32)(((sint64)second * first) >> 12);
    }
    character_controller.motion.jump_phase = -1;
    return 0xFFFFFFFF;
}

uint32 AnimatedObject::ClearCharacterPathRelease()
{
    FUNCTION_MARKER(0x800586E8, "SLUS_005.78");
    uint32 result;
    result = this->path_flags & 0xFFFF7F97;
    this->path_flags = result;
    character_controller.path_transition.anchor_path = 0;
    character_controller.path_transition.hold_motion_ticks = 0;
    return result;
}

const ScenePath *ReadCharacterConstrainedPath(void)
{
    FUNCTION_MARKER(0x8005A430, "SLUS_005.78");
    return character_controller.path_transition.constrained_path;
}

uint32 IsCharacterAnchorPathActive(void)
{
    FUNCTION_MARKER(0x8005A3FC, "SLUS_005.78");
    const ScenePath *selected = character_controller.path_transition.anchor_path;
    const ScenePath *path = camera_motion.path;
    return selected == path;
}

uint32 AnimatedObject::ClearCharacterPathStep()
{
    FUNCTION_MARKER(0x80058728, "SLUS_005.78");
    uint32 result = this->path_flags & 0xFFFFF7F7;
    this->path_flags = result;
    return result;
}

uint32 AnimatedObject::EndCharacterConstrainedPath()
{
    FUNCTION_MARKER(0x80058C78, "SLUS_005.78");
    uint32 result = this->path_flags & 0xFFFFFFB0;
    this->path_flags = result;
    return result;
}

uint32 AnimatedObject::CanObjectChangePath(const SceneTrigger *trigger, const ScenePath *context) const
{
    FUNCTION_MARKER(0x80047058, "SLUS_005.78");
    uint32 flags, mode;
    const PathDefinition *target, *other_target;
    const ScenePath *controller_path;
    const ScenePath *node;
    if (this->movement_mode == 2)
    {
        mode = this->state;
        if ((mode == 19 || (mode == 0 && path_routing.flags != 0)) && trigger != 0)
        {
            if ((this->flags & 0x10) != 0)
                node = ((const ScenePath *)((trigger->parameters.path_pair.targets[0])));
            else
                node = ((const ScenePath *)((trigger->parameters.path_pair.targets[1])));
            if ((((const PathDefinition *)((const PathDefinition *)((node->definition))))->flags & 0x90000) != 0)
                return 0;
        }
    }
    flags = this->flags;
    if ((flags & 0xC00) == 0 && (this->path_flags & 0xA0) == 0)
        return 0;
    if (context == 0)
        return 0;
    if (trigger == 0)
        return 1;
    if ((flags & 0x10) != 0)
        node = ((const ScenePath *)((trigger->parameters.path_pair.targets[0])));
    else
        node = ((const ScenePath *)((trigger->parameters.path_pair.targets[1])));
    mode = this->path_flags;
    if ((mode & 0x20) != 0)
    {
        const ScenePath *anchor = (const ScenePath *)(character_controller.path_transition.anchor_path);
        target = (node->definition);
        other_target = (anchor->definition);
        if (target != other_target || (mode & 0x8000) != 0)
            return 1;
        return (this->flags & 0x2000) != 0;
    }
    target = (node->definition);
    other_target = (context->definition);
    if (target != other_target)
    {
        if ((sint8)this->metadata126[0] != 1)
            return 1;
        controller_path = (const ScenePath *)(character_controller.path);
        other_target = (controller_path->definition);
        if (target != other_target)
            return 1;
    }
    return (this->flags & 0x2000) != 0;
}

uint32 AnimatedObject::CancelCharacterPathTransition()
{
    FUNCTION_MARKER(0x80056774, "SLUS_005.78");
    uint32 result, trigger;
    this->path_flags = this->path_flags & 0xFFFFFC7F;
    result = character_controller.flags;
    trigger = character_controller.motion.slope_scale;
    result &= 0xF7FFFFFF;
    character_controller.flags = result;
    if (trigger != 0)
        result = this->SetCharacterPathExitLatch();
    return result;
}

uint32 AnimatedObject::ClearCharacterPathTransitionFlags()
{
    FUNCTION_MARKER(0x800567FC, "SLUS_005.78");
    uint32 result;
    result = this->path_flags & 0xFFFFFC37;
    this->path_flags = result;
    return result;
}

uint32 AnimatedObject::SetCharacterPathExitLatch()
{
    FUNCTION_MARKER(0x800567D0, "SLUS_005.78");
    uint32 result = this->path_flags | 0x1000000;
    this->path_flags = result;
    return result;
}

uint32 AnimatedObject::ClearCharacterPathExitLatch()
{
    FUNCTION_MARKER(0x800567E4, "SLUS_005.78");
    uint32 result = this->path_flags & 0xFEFFFFFF;
    this->path_flags = result;
    return result;
}

uint32 AnimatedObject::ClearCharacterSurfaceMotion()
{
    FUNCTION_MARKER(0x80058C8C, "SLUS_005.78");
    uint32 result = this->path_flags & 0xFFFFFFB1;
    this->path_flags = result;
    return result;
}

uint32 AnimatedObject::ClearCharacterPathAuxiliaryMode()
{
    FUNCTION_MARKER(0x80059268, "SLUS_005.78");
    uint32 result = this->path_flags & 0xFFFFEFBF;
    this->path_flags = result;
    return result;
}
