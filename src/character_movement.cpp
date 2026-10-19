#include "game.h"
#include "original_defaults.h"

static const uint32 min_impulse_vertical_speed = 4096;

union MovementVector
{
    VECTOR vector;
    uint32 words[4];
};

uint32 AnimatedObject::UpdateCharacterHorizontalSpeed(uint32 decelerate)
{
    CharacterModelRecord *parent_record;
    FUNCTION_MARKER(0x8004922C, "SLUS_005.78");
    uint32 flags, object_flags, speed, cap, minimum;
    uint32 acceleration = 0, slope = 0, x, y, z, length, angle, folded, value, proposed, result;
    object_flags = this->flags;
    speed = character_controller.motion.horizontal_speed;
    decelerate &= 0xFF;
    if ((object_flags & 0x400) != 0)
        goto ground_speed;
    flags = this->path_flags;
    if ((flags & 0x1024) != 0)
    {
    ground_speed:
        /* Braking uses the same ground speed limit as acceleration */
        cap = (uint32)character_controller.movement.speed_limits[3];
        if (decelerate != 0)
        {
            if ((character_controller.flags & 0x20000000) == 0)
            {
                if ((sint32)speed < 0)
                    acceleration = (uint32)character_controller.movement.braking0;
                else
                    acceleration = 0 - (uint32)character_controller.movement.braking0;
            }
        }
        else
        {
            minimum = (uint32)character_controller.movement.speed_limits[2];
            acceleration = (uint32)character_controller.movement.acceleration0;
            if ((sint32)speed < 0)
                acceleration += (uint32)character_controller.movement.braking0;
        }
        goto apply_speed;
    }
    cap = (uint32)character_controller.movement.speed_limits[1];
    if ((flags & 0x10) != 0)
    {
        parent_record = character_controller.model;
        cap = (uint32)(((sint64)(sint32)cap * parent_record->motion_scale[2]) >> 12);
    }
    else if ((flags & 1) != 0)
    {
        parent_record = character_controller.model;
        cap = (uint32)(((sint64)(sint32)cap * parent_record->motion_scale[1]) >> 12);
    }
    else if ((flags & 4) != 0)
        cap = (uint32)(((sint64)(sint32)cap * path_transition_rules.axis_speed_limit_scale) >> 12);
    if (decelerate != 0)
    {
        if ((sint32)speed <= 0)
            acceleration = (uint32)character_controller.movement.braking1;
        else
            acceleration = 0 - (uint32)character_controller.movement.braking1;
    }
    else
    {
        minimum = (uint32)character_controller.movement.speed_limits[0];
        acceleration = (uint32)character_controller.movement.acceleration1;
        if ((sint32)speed < 0)
            acceleration += (uint32)character_controller.movement.braking1;
    }
    if (character_controller.motion.slope_scale != 0 && (this->path_flags & 0x1440100) == 0)
    {
        x = (uint32)(character_controller.path_velocity.vx >> 6);
        y = (uint32)(character_controller.path_velocity.vy >> 6);
        z = (uint32)(character_controller.path_velocity.vz >> 6);
        value = x * x + y * y + z * z;
        if ((sint32)value < 0)
            length = SquareRoot12((sint32)(value >> 2)) << 1;
        else
            length = SquareRoot12((sint32)value);
        angle = (0 - (uint32)(sint16)AsinAngle(character_controller.path_velocity.vy, (sint32)length)) & 0xFFF;
        if (angle != 0)
        {
            folded = angle;
            if (angle >= 1025)
                folded = 4096 - angle;
            if ((sint32)character_controller.motion.slope_scale >= (sint32)folded)
            {
                value = (uint32)SinQ14((sint16)angle);
                slope = 0 - (uint32)(((sint64)1024 * ((sint32)value >> 2)) >> 12);
            }
        }
        if (decelerate != 0)
        {
            value = 4096 - character_controller.motion.slope_scale;
            acceleration = (uint32)(((sint64)(sint32)acceleration * (sint32)value) >> 12);
        }
        else if ((sint32)slope < 0 && (sint32)cap < (sint32)speed)
            acceleration = 0;
        value = 15360 - cap;
        value = (uint32)(((sint64)(sint32)value * (sint32)character_controller.motion.slope_scale) >> 12);
        cap += value;
        if (decelerate == 0)
        {
            value = 4096 - character_controller.motion.slope_scale;
            minimum = (uint32)(((sint64)(sint32)minimum * (sint32)value) >> 12);
        }
        else if ((sint32)speed < 0)
            cap = 0 - cap;
        else if (speed == 0 && (sint32)(acceleration ^ slope) < 0)
            acceleration = 0 - acceleration;
    }
apply_speed:
    if (decelerate != 0)
    {
        if (speed != 0)
        {
            acceleration += slope;
            proposed = speed + acceleration;
            if ((sint32)(speed ^ proposed) < 0)
                speed = 0;
            else
            {
                speed = proposed;
                if ((this->flags & 0x400) == 0)
                {
                    if ((sint32)proposed < 0 && (sint32)cap < 0 && (sint32)proposed < (sint32)cap)
                        speed = cap;
                    else if ((sint32)proposed > 0 && (sint32)cap > 0 && (sint32)cap < (sint32)proposed)
                        speed = cap;
                }
            }
        }
        else
        {
            proposed = slope - acceleration;
            if (slope != 0 && (sint32)(slope ^ proposed) >= 0)
                speed = proposed;
        }
    }
    else
    {
        acceleration += slope;
        if ((sint32)speed < 0)
            speed += acceleration;
        else if ((sint32)speed < (sint32)minimum)
            speed = minimum;
        else if ((sint32)speed < (sint32)cap)
        {
            speed += acceleration;
            if ((sint32)cap < (sint32)speed)
                speed = cap;
        }
        else if ((this->flags & 0x400) == 0)
            speed = cap;
    }
    result = character_controller.flags & 0x800000;
    character_controller.motion.horizontal_speed = speed;
    if (result != 0)
    {
        const SceneTrigger *jump_scale = character_controller.surface_interaction.jump_scale;
        value = ((const SceneTrigger *)(jump_scale))->parameters.words[0];
        speed = (uint32)(((sint64)(sint32)speed * (sint32)value) >> 12);
        jump_scale = character_controller.surface_interaction.jump_scale;
        character_controller.motion.horizontal_speed = speed;
        value = ((const SceneTrigger *)(jump_scale))->parameters.words[0];
        result = character_controller.motion.vertical_speed;
        character_controller.motion.vertical_speed = (uint32)(((sint64)(sint32)result * (sint32)value) >> 12);
    }
    return result;
}

uint32 AnimatedObject::ApplyDirectionalImpulse(uint32 *move_flags, uint32 mode, const uint32 *direction_vector, sint32 strength, sint32 limit)
{
    const PathTransform *captured_path_transform;
    FUNCTION_MARKER(0x80049928, "SLUS_005.78");
    const PathDefinition *parent;
    uint32 value, x, y, z, dot, rx, ry, rz, nx, ny, nz, speed, coefficient;
    const PathTransform *path_transform;
    uint32 flags, vertical, minimum, angle, result;
    MovementVector vector;
    sint64 product;
    value = (uint32)AsinAngle((sint32)direction_vector[1], 4096);
    flags = character_controller.flags;
    if ((value & 0x3FF) - 64 < 0x380)
        flags |= 0x20000000;
    else
        flags &= 0xDFFFFFFF;
    character_controller.flags = flags;
    x = this->velocity[0];
    y = this->velocity[1];
    z = this->velocity[2];
    if ((*move_flags & 8) == 0)
    {
        x = (uint32)(((sint64)(sint32)x * 3276) >> 12);
        y = (uint32)(((sint64)(sint32)y * 3276) >> 12);
        z = (uint32)(((sint64)(sint32)z * 3276) >> 12);
    }
    parent = (this->path_definition);
    if (parent != 0)
    {
        path_transform = ((captured_path_transform = ((((const PathDefinition *)(parent)))->transform)) != 0 ? (const PathTransform *)((PathTransform *)(captured_path_transform)) : 0);
        if (path_transform != 0 && (this->flags & 0x2000) == 0)
        {
            x += path_transform->displacement[0];
            path_transform = (const PathTransform *)((((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) != 0 ? (((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) : (PathTransform *)0);
            y += path_transform->displacement[1];
            path_transform = (const PathTransform *)((((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) != 0 ? (((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) : (PathTransform *)0);
            z += path_transform->displacement[2];
        }
    }
    x -= character_controller.support.motion[0];
    y -= character_controller.support.motion[1];
    z -= character_controller.support.motion[2];
    dot = (uint32)(((sint64)(sint32)x * (sint32)direction_vector[0]) >> 12);
    dot += (uint32)(((sint64)(sint32)y * (sint32)direction_vector[1]) >> 12);
    dot += (uint32)(((sint64)(sint32)z * (sint32)direction_vector[2]) >> 12);
    dot = 0 - (dot << 1);
    rx = (uint32)(((sint64)(sint32)direction_vector[0] * (sint32)dot) >> 12);
    ry = (uint32)(((sint64)(sint32)direction_vector[1] * (sint32)dot) >> 12);
    rz = (uint32)(((sint64)(sint32)direction_vector[2] * (sint32)dot) >> 12);
    rx += x;
    ry += y;
    rz += z;
    nx = (uint32)(((sint64)(sint32)direction_vector[0] * strength) >> 12);
    ny = (uint32)(((sint64)(sint32)direction_vector[1] * strength) >> 12);
    nz = (uint32)(((sint64)(sint32)direction_vector[2] * strength) >> 12);
    rx += nx;
    ry += ny;
    rz += nz;
    rx += character_controller.support.motion[0];
    ry += character_controller.support.motion[1];
    rz += character_controller.support.motion[2];
    x = (uint32)((sint32)rx >> 6);
    y = (uint32)((sint32)ry >> 6);
    z = (uint32)((sint32)rz >> 6);
    value = x * x + y * y + z * z;
    if ((sint32)value < 0)
        speed = SquareRoot12((sint32)(value >> 2)) << 1;
    else
        speed = SquareRoot12((sint32)value);
    if (limit < (sint32)speed)
    {
        if (mode == 0xFFFFFFFF && ry + 12288 >= 0x6001)
        {
            value = (sint32)ry < 0 ? 0 - ry : ry;
            if (value == 0)
                xport_mips_break(7);
            coefficient = (uint32)(0x40000000 / (sint32)value);
            product = (sint64)12288 * (sint32)coefficient;
            coefficient = (uint32)((uint64)product >> 18);
            fixed_math_result = coefficient;
            rx = (uint32)(((sint64)(sint32)rx * (sint32)coefficient) >> 12);
            ry = (uint32)(((sint64)(sint32)ry * (sint32)coefficient) >> 12);
            rz = (uint32)(((sint64)(sint32)rz * (sint32)coefficient) >> 12);
        }
        if (speed == 0)
            xport_mips_break(7);
        coefficient = (uint32)(0x40000000 / (sint32)speed);
        product = (sint64)limit * (sint32)coefficient;
        coefficient = (uint32)((uint64)product >> 18);
        fixed_math_result = coefficient;
        rx = (uint32)(((sint64)(sint32)rx * (sint32)coefficient) >> 12);
        ry = (uint32)(((sint64)(sint32)ry * (sint32)coefficient) >> 12);
        rz = (uint32)(((sint64)(sint32)rz * (sint32)coefficient) >> 12);
    }
    *move_flags = *move_flags & 0xFFFFFFF7;
    value = (uint16)(character_controller.motion.form_flags >> 16) & 15;
    character_controller.timers.state_ticks = 0;
    character_controller.motion.jump_phase = 0;
    if (value != 5 && (character_controller.secondary_flags & 0x80) == 0)
        character_controller.motion.state_code = 3;
    character_controller.motion.vertical_speed = ry;
    character_controller.motion.jump_phase = (sint32)ry < 0 ? 0xFFFFFFFF : 1;
    ry = 0;
    if (((uint16)(character_controller.motion.form_flags >> 16) & 15) != 4 && ((this->flags & 0x400) != 0 || (this->path_flags & 4) != 0))
    {
        vertical = character_controller.motion.vertical_speed;
        minimum = min_impulse_vertical_speed;
        if ((sint32)vertical < (sint32)minimum && (sint32)vertical > 0)
        {
            value = character_controller.motion.horizontal_speed;
            coefficient = 0x40000000 / (sint32)vertical;
            value = (uint32)(((sint64)(sint32)value * (sint32)minimum) >> 12);
            product = (sint64)(sint32)value * (sint32)coefficient;
            value = (uint32)((uint64)product >> 18);
            fixed_math_result = value;
            character_controller.motion.horizontal_speed = value;
            character_controller.motion.vertical_speed = minimum;
        }
    }
    if (((uint16)(character_controller.motion.form_flags >> 16) & 15) == 5)
        rz = 0;
    x = (uint32)((sint32)rx >> 6);
    y = (uint32)((sint32)ry >> 6);
    z = (uint32)((sint32)rz >> 6);
    value = x * x + y * y + z * z;
    if ((sint32)value < 0)
        speed = SquareRoot12((sint32)(value >> 2)) << 1;
    else
        speed = SquareRoot12((sint32)value);
    character_controller.motion.horizontal_speed = speed;
    if (speed != 0)
    {
        angle = Atan2Angle((sint32)rz, (sint32)rx);
        value = (uint32)(sint32)this->target_yaw;
        angle = (uint32)(sint32)(sint16)angle - value - 1024;
        if ((angle & 0xFFF) >= 2048)
        {
            character_controller.motion.horizontal_speed = 0 - (uint32)character_controller.motion.horizontal_speed;
            rx = 0 - rx;
            ry = 0 - ry;
            rz = 0 - rz;
        }
        flags = this->flags;
        if ((flags & 0x2000) != 0)
        {
            while (rx + 26754 > 0xD103 || ry + 26754 > 0xD103 || rz + 26754 > 0xD103)
            {
                rx = (uint32)((sint32)rx >> 1);
                ry = (uint32)((sint32)ry >> 1);
                rz = (uint32)((sint32)rz >> 1);
            }
            vector.words[0] = rx;
            vector.words[1] = ry;
            vector.words[2] = rz;
            VectorNormal(&vector.vector, &vector.vector);
            x = vector.words[0];
            y = vector.words[1];
            z = vector.words[2];
            character_controller.path_tangent.vx = x;
            character_controller.path_tangent.vy = y;
            character_controller.path_tangent.vz = z;
        }
        else
        {
            vertical = character_controller.motion.vertical_speed;
            value = (uint32)character_controller.movement.jump_speed;
            if ((sint32)value >= (sint32)vertical && (flags & 0x400) == 0)
            {
                character_controller.motion.jump_phase = 1;
                character_controller.motion.vertical_speed = value;
                character_controller.timers.state_ticks = 0;
                this->frame = -1;
            }
        }
    }
    result = this->flags | 0x400;
    this->flags = result;
    return result;
}

uint32 AnimatedObject::ResolveSurfaceMotion(uint32 *move_flags)
{
    const PathTransform *captured_path_transform;
    FUNCTION_MARKER(0x8001C8CC, "SLUS_005.78");
    CollisionTransform record;
    MATRIX matrix_local;
    MovementVector normal, relative, tangent;
    uint32 contact, index;
    const MeshFace *face;
    const SceneNode *owner;
    const MeshGeometryNode *owner_mesh, *position_source;
    const PathDefinition *parent;
    uint32 x, y, z, a, b, c, d, e, f, g, h, i, value, flags, up, down;
    const PathTransform *path_transform;
    uint32 dot, squared, magnitude, scaled_x, scaled_y, scaled_z, result;
    sint32 heading, angle;
    owner = character_controller.surface_interaction.selected;
    owner_mesh = (const MeshGeometryNode *)owner;
    contact = this->behavior_word & 0xFFFF8FFF;
    if (owner_mesh->node.kind == 1)
    {
        x = psx_identity_rotation_words[0];
        x = psx_identity_rotation_words[0];
        y = psx_identity_rotation_words[1];
        y = psx_identity_rotation_words[1];
        z = psx_identity_rotation_words[2];
        z = psx_identity_rotation_words[2];
        record.words[1] = x;
        record.words[1] = x;
        record.words[2] = y;
        record.words[2] = y;
        record.words[3] = z;
        record.words[3] = z;
        x = psx_identity_rotation_words[3];
        x = psx_identity_rotation_words[3];
        y = (uint32)(sint32)psx_identity_matrix.m[2][2];
        record.words[4] = x;
        record.words[4] = x;
        record.fields.rotation[8] = y;
        if (owner_mesh->vertex_count > 0)
            position_source = owner_mesh;
        else
            position_source = (((MeshGeometryNode *)(owner_mesh->source_mesh)));
        x = position_source->position[0];
        y = position_source->position[1];
        z = position_source->position[2];
        record.words[6] = x;
        record.words[7] = y;
        record.words[8] = z;
        record.words[11] = 0;
        record.words[10] = 0;
        record.words[9] = 0;
    }
    else
    {
        const CollisionTransform *cached_transform = ((const AnimatedObject *)owner_mesh)->collision_transforms;
        cached_transform = ((const CollisionTransform *)(cached_transform)) + (contact >> 16);
        for (index = 0; index < 48; index += 16)
        {
            x = cached_transform->words[(index >> 2)];
            y = cached_transform->words[(index >> 2) + 1];
            z = cached_transform->words[(index >> 2) + 2];
            a = cached_transform->words[(index >> 2) + 3];
            record.words[(index >> 2)] = x;
            record.words[(index >> 2) + 1] = y;
            record.words[(index >> 2) + 2] = z;
            record.words[(index >> 2) + 3] = a;
        }
    }
    record.fields.face_count = 1;
    record.fields.first_face = contact;
    face = ((MeshFace *)(owner_mesh->polygons));
    a = (uint16)record.fields.rotation[6];
    b = (uint16)record.fields.rotation[0];
    c = (uint16)record.fields.rotation[3];
    d = (uint16)record.fields.rotation[1];
    e = (uint16)record.fields.rotation[4];
    f = (uint16)record.fields.rotation[7];
    g = (uint16)record.fields.rotation[2];
    h = (uint16)record.fields.rotation[5];
    i = (uint16)record.fields.rotation[8];
    matrix_local.m[0][2] = a;
    matrix_local.m[0][1] = c;
    face += (uint16)contact;
    matrix_local.m[0][0] = b;
    matrix_local.m[1][0] = d;
    matrix_local.m[1][1] = e;
    matrix_local.m[1][2] = f;
    matrix_local.m[2][0] = g;
    matrix_local.m[2][1] = h;
    matrix_local.m[2][2] = i;
    PushMatrix();
    x = xport_load_le32((const uint8 *)&matrix_local);
    y = xport_load_le32((const uint8 *)&matrix_local + 4);
    xport_gte_write_control(0, x);
    xport_gte_write_control(1, y);
    x = xport_load_le32((const uint8 *)&matrix_local + 8);
    y = xport_load_le32((const uint8 *)&matrix_local + 12);
    z = (uint16)matrix_local.m[2][2];
    xport_gte_write_control(2, x);
    xport_gte_write_control(3, y);
    xport_gte_write_control(4, z);
    xport_gte_write_data(0, face->normal.words[0]);
    xport_gte_write_data(1, face->normal.words[1]);
    xport_gte_execute(0x486012);
    normal.words[0] = xport_gte_read_data(25);
    normal.words[1] = xport_gte_read_data(26);
    normal.words[2] = xport_gte_read_data(27);
    PopMatrix();
    x = this->velocity[0];
    a = record.words[9];
    x -= a;
    relative.words[0] = x;
    y = this->velocity[1];
    b = record.words[10];
    y -= b;
    relative.words[1] = y;
    z = this->velocity[2];
    c = record.words[11];
    z -= c;
    relative.words[2] = z;
    parent = (this->path_definition);
    if (parent != 0 && (path_transform = ((captured_path_transform = ((((const PathDefinition *)(parent)))->transform)) != 0 ? (const PathTransform *)((PathTransform *)(captured_path_transform)) : 0)) != 0 && (this->flags & 0x2000) == 0)
    {
        relative.words[0] = x + path_transform->displacement[0];
        path_transform = (const PathTransform *)((((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) != 0 ? (((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) : (PathTransform *)0);
        relative.words[1] = y + path_transform->displacement[1];
        path_transform = (const PathTransform *)((((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) != 0 ? (((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) : (PathTransform *)0);
        relative.words[2] = z + path_transform->displacement[2];
    }
    value = this->behavior_word;
    heading = (sint8)this->metadata126[0];
    value = (value & 0x7000) >> 12;
    if (heading == 1 && character_geometry.local[0].position[2] == 0)
    {
        up = value == 1;
        down = value == 4 || character_geometry.world[value].radius == 0xFFFFFFFF;
    }
    else
    {
        up = 1;
        down = 1;
    }
    if (normal.words[1] + 128 < 257 || (this->flags & 0x400) == 0 || (this->velocity[0] == 0 && this->velocity[1] == 0 && this->velocity[2] == 0) || (up == 0 && down == 0))
        goto reject_contact;
    value = normal.words[1];
    if ((sint32)value < 0 && (down == 0 || (sint32)relative.words[1] < 0))
        goto reject_contact;
    if ((sint32)value > 0 && up == 0)
        goto reject_contact;
    if ((sint32)normal.words[1] >= 0 && up != 0)
        goto slide_contact;
    if ((character_controller.flags & 0x10000) != 0)
        goto reverse_speed;
    value = character_controller.movement.jump_ticks;
    flags = character_controller.flags;
    character_controller.motion.jump_phase = -1;
    character_controller.motion.vertical_speed = 0;
    character_controller.timers.state_ticks = 0;
    character_controller.timers.jump_duration_ticks = value;
    character_controller.flags = flags | 0x08000000;
    value = record.words[10];
    if ((sint32)value < 0)
        character_controller.motion.vertical_speed = value;
    if ((character_controller.secondary_flags & 0x80) == 0)
        character_controller.motion.state_code = 6;
    flags = character_controller.flags;
    if ((flags & 0x40) == 0 && ((uint16)(character_controller.motion.form_flags >> 16) & 15) == 5 && (sint32)(normal.words[1] - 128) < -4095)
    {
        character_controller.flags = (flags & 0xFFFE7FBF) | 0x8000;
        goto save_contact;
    }
    result = (uint16)(character_controller.motion.form_flags >> 16) & 15;
    character_controller.motion.horizontal_speed = 0;
    if (result != 5)
    {
        result = *move_flags & 0xFFFFFFFE;
        *move_flags = result;
    }
    goto finish;
reject_contact:
    flags = character_controller.flags;
    if ((flags & 0x10000) != 0)
        goto reverse_speed;
    if ((flags & 0x40) != 0)
    {
        result = 0xFFFE0000;
        goto finish;
    }
    character_controller.flags = (flags & 0xFFFE7FBF) | 0x40;
    goto save_contact;
reverse_speed:
    x = character_controller.support.speed[0];
    y = character_controller.support.speed[1];
    result = 0 - x;
    character_controller.support.speed[0] = result;
    character_controller.support.speed[1] = 0 - y;
    goto finish;
slide_contact:
    value = character_controller.movement.jump_ticks;
    flags = character_controller.flags;
    character_controller.motion.jump_phase = -1;
    character_controller.motion.vertical_speed = 0;
    character_controller.timers.state_ticks = 0;
    character_controller.motion.horizontal_speed = 0;
    character_controller.timers.jump_duration_ticks = value;
    character_controller.flags = flags | 0x08000000;
    dot = 0;
    for (index = 0; index < 3; ++index)
    {
        value = this->velocity[index];
        x = normal.words[index];
        dot += (uint32)(((sint64)(sint32)value * (sint32)x) >> 12);
    }
    x = normal.words[0];
    scaled_x = (uint32)(((sint64)(sint32)x * (sint32)dot) >> 12);
    y = normal.words[1];
    tangent.words[0] = scaled_x;
    scaled_y = (uint32)(((sint64)(sint32)y * (sint32)dot) >> 12);
    z = normal.words[2];
    tangent.words[1] = scaled_y;
    scaled_z = (uint32)(((sint64)(sint32)z * (sint32)dot) >> 12);
    tangent.words[2] = scaled_z;
    x = this->velocity[0] - scaled_x;
    tangent.words[0] = x;
    y = this->velocity[1] - scaled_y;
    tangent.words[1] = y;
    z = this->velocity[2] - scaled_z;
    tangent.words[2] = z;
    squared = (uint32)(((sint64)(sint32)x * (sint32)x) >> 12) + (uint32)(((sint64)(sint32)y * (sint32)y) >> 12) + (uint32)(((sint64)(sint32)z * (sint32)z) >> 12);
    if ((sint32)squared < 16)
    {
        a = normal.words[0];
        x = this->velocity[0];
        dot = (uint32)(((sint64)(sint32)a * (sint32)x) >> 12);
        b = normal.words[2];
        z = this->velocity[2];
        dot += (uint32)(((sint64)(sint32)b * (sint32)z) >> 12);
        x = normal.words[0];
        scaled_x = (uint32)(((sint64)(sint32)x * (sint32)dot) >> 12);
        y = normal.words[1];
        tangent.words[0] = scaled_x;
        scaled_y = (uint32)(((sint64)(sint32)y * (sint32)dot) >> 12);
        z = normal.words[2];
        tangent.words[1] = scaled_y;
        scaled_z = (uint32)(((sint64)(sint32)z * (sint32)dot) >> 12);
        tangent.words[2] = scaled_z;
        x = this->velocity[0] - scaled_x;
        y = 0 - scaled_y;
        tangent.words[0] = x;
        tangent.words[1] = y;
        z = this->velocity[2] - scaled_z;
        tangent.words[2] = z;
        squared = (uint32)(((sint64)(sint32)x * (sint32)x) >> 12) + (uint32)(((sint64)(sint32)y * (sint32)y) >> 12) + (uint32)(((sint64)(sint32)z * (sint32)z) >> 12);
    }
    if ((sint32)squared < 2)
    {
        character_controller.support.speed[1] = 0;
        tangent.words[0] = 6144;
        goto adjust_sign;
    }
    y = tangent.words[1];
    if ((sint32)y > 0)
    {
        x = tangent.words[0];
        z = tangent.words[2];
        tangent.words[1] = 0 - y;
        tangent.words[0] = 0 - x;
        tangent.words[2] = 0 - z;
    }
    if (tangent.words[0] + 26754 > 0xD103 || tangent.words[1] + 26754 > 0xD103 || tangent.words[2] + 26754 > 0xD103)
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
    scaled_x = (uint32)(((sint64)(sint32)x * 6144) >> 12);
    y = tangent.words[1];
    tangent.words[0] = scaled_x;
    scaled_y = (uint32)(((sint64)(sint32)y * 6144) >> 12);
    z = tangent.words[2];
    tangent.words[1] = scaled_y;
    scaled_z = (uint32)(((sint64)(sint32)z * 6144) >> 12);
    tangent.words[2] = scaled_z;
    character_controller.support.speed[1] = scaled_y;
    x = (uint32)((sint32)tangent.words[0] >> 6);
    z = (uint32)((sint32)tangent.words[2] >> 6);
    squared = x * x + z * z;
    tangent.words[1] = 0;
    magnitude = (sint32)squared >= 0 ? SquareRoot12((sint32)squared) : SquareRoot12((sint32)(squared >> 2)) << 1;
    z = this->velocity[2];
    x = this->velocity[0];
    heading = (sint16)Atan2Angle((sint32)z, (sint32)x);
    if (heading == 4096)
        heading = ((uint16)this->target_yaw + 2048) & 0xFFF;
    z = tangent.words[2];
    x = tangent.words[0];
    angle = (sint16)Atan2Angle((sint32)z, (sint32)x);
    value = ((uint32)angle - (uint32)(sint32)(sint16)heading + 1024) & 0xFFF;
    tangent.words[0] = value < 2048 ? magnitude : 0 - magnitude;
adjust_sign:
    if ((sint32)character_controller.support.speed[0] < 0)
        tangent.words[0] = 0 - tangent.words[0];
    if ((character_controller.flags & 0x10000) != 0)
    {
        x = character_controller.support.speed[0];
        y = tangent.words[0];
        if ((sint32)(x ^ y) < 0)
        {
            tangent.words[0] = 0 - y;
            character_controller.support.speed[1] = 0 - character_controller.support.speed[1];
        }
    }
    if ((character_controller.flags & 0x40) != 0)
    {
        x = character_controller.support.speed[0];
        y = tangent.words[0];
        if ((sint32)(x ^ y) >= 0)
        {
            tangent.words[0] = 0 - y;
            character_controller.support.speed[1] = 0 - character_controller.support.speed[1];
        }
    }
    flags = character_controller.flags;
    value = tangent.words[0];
    character_controller.support.speed[0] = value;
    character_controller.flags = (flags & 0xFFFE7FBF) | 0x10000;
    this->flags = this->flags | 0x400;
save_contact:
    result = this->behavior_word;
    owner = character_controller.surface_interaction.selected;
    result &= 0xFFFF8FFF;
    character_controller.support.contact_word = result;
    character_controller.support.owner = (SceneNode *)owner;
finish:
    return result;
}

uint32 AnimatedObject::ApplyContactImpulse(uint32 *move_flags)
{
    FUNCTION_MARKER(0x8004A218, "SLUS_005.78");
    const ActorModelDefinition *bound_model;
    uint32 vector[3];
    const PathDefinition *parent;
    uint32 flags, auxiliary_flags, parent_flag, special;
    const ActorBinding *contact_binding;
    const SceneNode *other;
    const SceneTrigger *impulse_trigger;
    const AnimatedObject *contact_object, *contact_root;
    uint32 volume, pointer, x, y, z, relative_x, relative_y, relative_z, strength, value, mode, index;
    sint32 kind, direction, angle;
    flags = character_controller.flags;
    auxiliary_flags = character_controller.secondary_flags;
    other = character_controller.support.contact;
    character_controller.flags = (flags | 0x40000) & 0xFF7FFFFF;
    character_controller.secondary_flags = auxiliary_flags & 0xFFFFFF7F;
    if (other == 0 || other->kind != 0)
    {
        const SceneTrigger *selected = character_controller.surface_interaction.selected != 0 ? (const SceneTrigger *)character_controller.surface_interaction.selected : (const SceneTrigger *)0;
        strength = selected->parameters.words[0];
        relative_x = selected->transform.t[0];
        relative_y = selected->transform.t[1];
        relative_z = selected->transform.t[2];
        direction = (sint16)character_controller.support.contact_direction;
        goto resolve_direction_8004a218;
    }
    contact_object = (const AnimatedObject *)other;
    parent = (contact_object->path_definition);
    parent_flag = 0;
    contact_binding = 0;
    if (parent)
    {
        contact_binding = (const ActorBinding *)(((const ActorPlacement *)parent)->state != 0 ? ((const ActorPlacement *)parent)->state : 0);
        parent_flag = (contact_binding->action_effect_flags) & 0x1000;
    }
    kind = (sint8)contact_object->behavior;
    special = 0;
    if (kind == 20 && (contact_object->flags & 0x40))
        special = 1;
    if (!special && parent_flag && (contact_object->flags & 0x40))
    {
        kind = (sint8)contact_object->behavior;
        special = 1;
    }
    if (special)
    {
        if (kind == 20 || ((contact_binding->action_effect_flags) & 0x1000))
        {
            bound_model = ((const ActorBinding *)(((const ActorPlacement *)((const PathDefinition *)((contact_object->path_definition))))->state != 0 ? ((const ActorPlacement *)((const PathDefinition *)((contact_object->path_definition))))->state : 0))->model;
            const SoundBankStorage *actor_sounds = bound_model->sound_table;
            const SoundSample *sample = actor_sounds != 0 && (sint32)actor_sounds->header.count >= 2 ? actor_sounds->samples + 1 : 0;
            volume = sound_settings.effects_volume;
            sound_effects.Queue(sample, sample, 0, volume, 0, 3);
            character_controller.flags = character_controller.flags | 0x08000000;
        }
        if ((this->flags & 0x400) && (sint8)contact_object->behavior != 30)
        {
            vector[2] = 0;
            vector[0] = 0;
            vector[1] = 4096;
            this->velocity[1] = 0;
        }
        else
        {
            x = this->world_position[0];
            y = this->world_position[1];
            z = this->world_position[2];
            vector[0] = x;
            vector[1] = y;
            vector[2] = z;
            contact_root = character_controller.support.contact != 0 ? (const AnimatedObject *)character_controller.support.contact : (const AnimatedObject *)0;
            relative_x = contact_root->world_position[0];
            relative_y = contact_root->world_position[1];
            relative_z = contact_root->world_position[2];
            relative_x = x - relative_x;
            relative_y = y - relative_y;
            relative_z = z - relative_z;
            if ((sint8)contact_object->behavior == 30)
            {
                vector[0] = (sint32)relative_x < 0 ? (uint32)-1672 : 1672;
                vector[2] = (sint32)relative_z < 0 ? (uint32)-1672 : 1672;
                vector[1] = 3344;
                SpawnActionEffect(this, &character_controller, 1);
            }
            else
            {
                vector[0] = (sint32)relative_x < 0 ? (uint32)-2364 : 2364;
                vector[2] = (sint32)relative_z < 0 ? (uint32)-2364 : 2364;
                vector[1] = 2364;
            }
        }
        if ((sint8)contact_object->behavior == 30)
            strength = 409;
        else
            strength = (this->flags & 0x400) ? 16384 : 40960;
        mode = 0xFFFFFFFF;
        character_controller.support.motion[2] = 0;
        character_controller.support.motion[1] = 0;
        character_controller.support.motion[0] = 0;
        goto apply_vector_8004a218;
    }
    contact_root = character_controller.support.contact != 0 ? (const AnimatedObject *)character_controller.support.contact : (const AnimatedObject *)0;
    relative_x = contact_root->world_position[0];
    relative_y = contact_root->world_position[1];
    relative_z = contact_root->world_position[2];
    kind = (sint8)contact_object->behavior;
    if (kind == 19 && (contact_object->flags & 0x40))
    {
        strength = 40960;
        direction = -1;
        goto resolve_direction_8004a218;
    }
    if (kind == 23)
    {
        bound_model = ((const ActorBinding *)(((const ActorPlacement *)((const PathDefinition *)((contact_object->path_definition))))->state != 0 ? ((const ActorPlacement *)((const PathDefinition *)((contact_object->path_definition))))->state : 0))->model;
        const SoundBankStorage *actor_sounds = bound_model->sound_table;
        const SoundSample *sample = actor_sounds != 0 && (sint32)actor_sounds->header.count >= 2 ? actor_sounds->samples + 1 : 0;
        volume = sound_settings.effects_volume;
        sound_effects.Queue(sample, sample, 0, volume, 0, 3);
    }
    strength = (sint8)contact_object->behavior == 19 ? 12288 : 8192;
    direction = -1;
    if ((sint8)contact_object->metadata126[0] == 2 && (contact_object->flags & 0x40))
    {
        x = this->world_position[0];
        y = this->world_position[1];
        z = this->world_position[2];
        vector[0] = x;
        vector[1] = y;
        vector[2] = z;
        value = Atan2Angle((sint32)(relative_z - z), (sint32)(relative_x - x));
        angle = (sint16)contact_object->yaw - (sint16)value;
        vector[0] = (uint32)angle;
        if (angle < 0)
            vector[0] = 0 - (uint32)angle;
        if ((sint32)vector[0] < 1024)
            strength += ((const ActorBinding *)(((const ActorPlacement *)((const PathDefinition *)((contact_object->path_definition))))->state != 0 ? ((const ActorPlacement *)((const PathDefinition *)((contact_object->path_definition))))->state : 0))->motion_speed;
        relative_x += contact_object->velocity[0];
        relative_y += contact_object->velocity[1];
        relative_z += contact_object->velocity[2];
    }
resolve_direction_8004a218:
    mode = (uint32)direction - 1;
    if (direction < 0)
    {
        pointer = OriginalDefaults::SceneContextIdentity;
        x = camera_motion.focus_position[0];
        relative_x = x - relative_x;
        y = camera_motion.focus_position[1];
        relative_y = y - relative_y;
        z = camera_motion.focus_position[2];
        relative_z = z - relative_z;
        value = Atan2Angle((sint32)relative_z, (sint32)relative_x);
        angle = this->target_yaw;
        value = ((uint32)(sint32)(sint16)value - (uint32)angle - 1024) & 0xFFF;
        character_controller.motion.horizontal_speed = value < 2048 ? strength : 0 - strength;
        goto finish_8004a218;
    }
    index = (uint32)((sint32)mode >> 1);
    impulse_trigger = character_controller.surface_interaction.selected != 0 ? (const SceneTrigger *)character_controller.surface_interaction.selected : (const SceneTrigger *)0;
    value = (uint32)(sint32)impulse_trigger->transform.m[index][0];
    vector[0] = (mode & 1) ? value : 0 - value;
    impulse_trigger = character_controller.surface_interaction.selected != 0 ? (const SceneTrigger *)character_controller.surface_interaction.selected : (const SceneTrigger *)0;
    value = (uint32)(sint32)impulse_trigger->transform.m[index][1];
    vector[1] = (mode & 1) ? value : 0 - value;
    impulse_trigger = character_controller.surface_interaction.selected != 0 ? (const SceneTrigger *)character_controller.surface_interaction.selected : (const SceneTrigger *)0;
    value = (uint32)(sint32)impulse_trigger->transform.m[index][2];
    vector[2] = (mode & 1) ? value : 0 - value;
apply_vector_8004a218:
    *move_flags = *move_flags | 8;
    this->ApplyDirectionalImpulse(move_flags, mode, vector, (sint32)strength, 36044);
finish_8004a218:
    value = (uint16)(character_controller.motion.form_flags >> 16);
    character_controller.timers.contact_impulse_ticks = 10;
    flags = *move_flags;
    value = flags & ((value & 15) == 3 ? 0xFFFFFFBE : 0xFFFFFF3E);
    *move_flags = value;
    return value;
}

uint32 AnimatedObject::UpdateMovementImpulse(uint32 *move_flags)
{
    FUNCTION_MARKER(0x8004D5C0, "SLUS_005.78");
    uint32 vector[3];
    uint32 flags, value, mask, mode, snapshot, i, result;
    sint32 index = 0, strength, applied;
    if ((character_controller.flags & 4) != 0)
    {
        this->ResolveSurfaceMotion(move_flags);
        character_controller.flags = character_controller.flags & 0xFFFFFFFB;
    }
    flags = character_controller.flags;
    if ((flags & 0x18040) != 0)
    {
        if ((((const SceneNode *)(character_controller.support.owner))->flags & 1) != 0)
            character_controller.ClearTransientFlags();
        else if ((flags & 0x10000) != 0)
        {
            if ((character_controller.secondary_flags & 0x80) == 0)
                character_controller.motion.state_code = 8;
            *move_flags = *move_flags & 0xFFFFFBFE;
        }
    }
    mode = character_controller.flags & 0x60000;
    if (mode == 0x40000)
    {
        value = character_controller.timers.contact_impulse_ticks;
        character_controller.timers.contact_impulse_ticks = value - 1;
        if (value == 0)
        {
            flags = character_controller.flags;
            character_controller.timers.contact_impulse_ticks = 0;
            character_controller.flags = flags & 0xFFFBFFFF;
            this->flags = this->flags & 0xEFFFFFFF;
            character_controller.flags = character_controller.flags & 0xFFEFFFFF;
        }
        else
            goto suppress;
    }
    else if (mode == 0x60000 && (this->flags & 0x400) != 0)
    {
    suppress:
        *move_flags = 0;
        mask = (*control_bindings.active_scheme)[1];
        gameplay_input.buttons = gameplay_input.buttons & ~mask;
        mask = (*control_bindings.active_scheme)[1];
        gameplay_input.prev_buttons &= ~mask;
    }
    if ((character_controller.flags & 0x80000) != 0)
        *move_flags = 0;
    if ((this->path_flags & 0x101) != 0 && (character_controller.flags & 0x20000) != 0)
        character_controller.flags = character_controller.flags & 0xFFFDFFFF;
    flags = character_controller.flags;
    if ((flags & 0x810) == 0 && (flags & 0x60000) != 0x20000)
    {
        result = this->flags & 0x400;
        if (result == 0)
        {
            result = this->path_flags & 0x1220;
            if (result == 0)
            {
                result = (uint32)character_controller.model->movement.rise_speed_limit;
                character_controller.motion.vertical_speed = 0;
                character_controller.movement.rise_speed_limit = result;
            }
        }
        goto done;
    }
    if ((this->flags & 0x400) == 0)
        *move_flags = *move_flags | 8;
    value = (uint16)(character_controller.motion.form_flags >> 16);
    character_controller.motion.jump_phase = 1;
    character_controller.timers.state_ticks = 0;
    if ((value & 0xF) != 4 || (this->flags & 0x8000000) == 0)
        this->frame = -1;
    if (((uint16)(character_controller.motion.form_flags >> 16) & 0xF) == 1)
    {
        flags = this->flags;
        if ((flags & 0x4000000) != 0)
        {
            this->flags = flags & 0xFBFFFFFF;
            if ((sint8)character_controller.motion.state_code == 9 && (character_controller.secondary_flags & 0x80) == 0)
                character_controller.motion.state_code = 2;
            for (i = 0; i < 4; ++i)
                character_controller.movement.speed_limits[i] = character_controller.special_move_backup[i];
            character_controller.motion.horizontal_speed = character_controller.special_move_backup[1];
            character_controller.special_move_backup[1] = (uint32)-15;
        }
    }
    value = character_controller.movement.jump_ticks;
    flags = character_controller.flags;
    character_controller.timers.jump_duration_ticks = value;
    if ((flags & 0x60000) == 0x20000)
    {
        *move_flags = *move_flags | 8;
        character_controller.flags = character_controller.flags | 0x2000;
        result = this->ApplyContactImpulse(move_flags);
        goto done;
    }
    value = (uint32)character_controller.model->movement.rise_speed_limit;
    flags = character_controller.flags;
    character_controller.timers.jump_duration_ticks = 7;
    character_controller.movement.rise_speed_limit = value;
    if ((flags & 0x10) != 0)
    {
        index = (sint16)character_controller.support.contact_direction - 1;
        for (i = 0; i < 3; ++i)
        {
            const SceneTrigger *surface = (const SceneTrigger *)(character_controller.support.impulse_surface);
            value = (uint32)(sint32)surface->transform.m[index >> 1][i];
            if (((uint32)index & 1) == 0)
                value = 0 - value;
            vector[i] = value;
        }
        strength = (sint32)((const SceneTrigger *)(character_controller.support.impulse_surface))->parameters.words[0];
    }
    else
    {
        vector[2] = 0;
        vector[0] = 0;
        vector[1] = 4096;
        this->velocity[1] = 0;
        if ((character_controller.flags & 0x200000) == 0 && character_controller.support.impulse_scale == 0 && (sint8)character_controller.path_transition.contact_delay == 4)
        {
            QueueCharacterActionSound(7, 3);
            SpawnActionEffect(this, &character_controller, 1);
        }
        strength = character_controller.movement.jump_speed;
        value = character_controller.support.impulse_scale;
        character_controller.support.motion[2] = 0;
        character_controller.support.motion[1] = 0;
        character_controller.support.motion[0] = 0;
        if (value != 0)
            strength = (sint32)(uint32)(((sint64)strength * (sint32)value) >> 12);
    }
    value = (sint16)character_controller.timers.jump_input_ticks < 3;
    character_controller.motion.buffered_rise_limit = 0;
    if (value != 0)
    {
        character_controller.timers.jump_input_ticks = 6;
        *move_flags = *move_flags | 8;
    }
    else if ((*move_flags & 8) == 0)
    {
        character_controller.timers.jump_input_ticks = 3;
        *move_flags = *move_flags | 8;
        this->ApplyDirectionalImpulse(move_flags, (uint32)index, vector, strength, (sint32)((uint32)strength << 1));
        if ((character_controller.flags & 0x10) != 0 && character_controller.motion.vertical_speed < character_controller.movement.jump_speed)
            character_controller.motion.vertical_speed = (uint32)character_controller.movement.jump_speed;
        flags = (uint32)character_controller.movement.rise_speed_limit;
        value = character_controller.motion.horizontal_speed;
        mode = character_controller.motion.vertical_speed;
        snapshot = character_controller.motion.vertical_speed;
        character_controller.motion.buffered_rise_limit = flags;
        character_controller.motion.buffered_horizontal_speed = value;
        character_controller.motion.buffered_vertical_speed = mode;
        if ((sint32)flags < (sint32)snapshot)
            character_controller.motion.buffered_rise_limit = snapshot;
    }
    snapshot = *move_flags;
    applied = strength;
    if ((*move_flags & 8) == 0)
        applied = strength >> 2;
    this->ApplyDirectionalImpulse(move_flags, snapshot, vector, applied, (sint32)((uint32)strength << 1));
    if ((character_controller.flags & 0x10) != 0 && character_controller.motion.vertical_speed < character_controller.movement.jump_speed)
        character_controller.motion.vertical_speed = (uint32)character_controller.movement.jump_speed;
    value = character_controller.motion.vertical_speed;
    if (character_controller.movement.rise_speed_limit < (sint32)value)
        character_controller.movement.rise_speed_limit = value;
    if ((snapshot & 8) != 0)
    {
        if (character_controller.motion.jump_phase == 1)
            character_controller.motion.jump_phase = 2;
        character_controller.flags = character_controller.flags | 0x10000000;
    }
    *move_flags = *move_flags | 8;
    result = (character_controller.flags | 0x2000) & 0xF7FFFFFF;
    character_controller.flags = result;
done:
    return result;
}

uint32 AnimatedObject::UpdateJumpMotion(uint32 *move_flags)
{
    FUNCTION_MARKER(0x8004B8F8, "SLUS_005.78");
    uint32 flags, value, mask, x, y, z, squared, angle, result;
    sint32 old_frame, speed, length, cosine, duration, timer;
    if (((*move_flags & 8) == 0 && character_controller.motion.jump_phase < 2 && (character_controller.flags & 0x10000000) == 0) || (character_controller.flags & 0x10000) != 0 || (this->path_flags & 0x200) != 0)
    {
        if ((this->flags & 0x400) != 0)
        {
            if (character_controller.motion.jump_phase > 0)
            {
                if ((character_controller.flags & 0x10000000) == 0)
                {
                    character_controller.timers.state_ticks = 0;
                    character_controller.motion.jump_phase = 0;
                }
            }
            else
                character_controller.flags = character_controller.flags & 0xEFFFFFFF;
        }
        goto done;
    }
    flags = character_controller.flags;
    if ((flags & 0x2000) != 0 && (this->flags & 0x400) == 0)
        goto done;
    if ((flags & 0x10000000) == 0)
        character_controller.timers.jump_duration_ticks = character_controller.movement.jump_ticks;
    *move_flags = *move_flags | 8;
    if (((uint16)(character_controller.motion.form_flags >> 16) & 0xF) == 5 && (character_controller.secondary_flags & 0x80) == 0)
        character_controller.motion.state_code = 3;
    if ((this->flags & 0x400) == 0)
    {
        character_controller.flags = character_controller.flags | 0x2000;
        if ((this->flags & 0x8000000) != 0)
        {
            *move_flags = *move_flags & 0xFFFFFBFE;
            this->flags = this->flags & 0xF7FFFFFF;
        }
        if (character_controller.motion.vertical_speed <= 0)
        {
            old_frame = this->frame;
            value = (uint32)character_controller.movement.jump_speed;
            character_controller.motion.jump_phase = 1;
            character_controller.timers.state_ticks = 0;
            character_controller.motion.vertical_speed = value;
            this->frame = -1;
            if ((character_controller.flags & 0x800000) != 0)
            {
                value = character_controller.motion.vertical_speed;
                speed = (sint32)((const SceneTrigger *)(character_controller.surface_interaction.jump_scale))->parameters.words[1];
                speed = (sint32)(uint32)(((sint64)(sint32)value * speed) >> 12);
                character_controller.motion.vertical_speed = (uint32)speed;
                if (speed <= 0)
                {
                    *move_flags = *move_flags & 0xFFFFFFF7;
                    value = character_controller.movement.jump_ticks;
                    flags = character_controller.flags;
                    character_controller.motion.vertical_speed = 0;
                    character_controller.motion.jump_phase = -1;
                    character_controller.timers.state_ticks = 0;
                    character_controller.timers.jump_duration_ticks = value;
                    character_controller.flags = flags | 0x8000000;
                    this->frame = (uint32)old_frame;
                }
            }
            if (character_controller.motion.vertical_speed == 0)
                goto done;
        }
        character_controller.movement_flags = character_controller.movement_flags | 0x400000;
        flags = this->flags | 0x400;
        this->flags = flags;
        if ((flags & 0x2000) == 0)
        {
            x = (uint32)(character_controller.path_velocity.vx >> 6);
            y = (uint32)(character_controller.path_velocity.vy >> 6);
            z = (uint32)(character_controller.path_velocity.vz >> 6);
            squared = x * x + y * y + z * z;
            if ((sint32)squared >= 0)
                length = (sint32)SquareRoot12((sint32)squared);
            else
                length = (sint32)(SquareRoot12((sint32)(squared >> 2)) << 1);
            angle = (0 - (uint32)AsinAngle(character_controller.path_velocity.vy, length)) & 0xFFF;
            if (character_controller.motion.horizontal_speed < 0)
                angle = (0 - angle) & 0xFFF;
            if ((sint16)angle >= 1025)
                angle += (uint32)((4096 - (sint16)angle) >> 1);
            else
            {
                angle <<= 1;
                if ((sint16)angle >= 1025)
                    angle = 1024;
            }
            if ((this->path_flags & 4) != 0)
            {
                angle = 3584;
                character_controller.motion.horizontal_speed = (uint32)character_controller.movement.speed_limits[1];
            }
            if ((sint16)angle >= 2048)
            {
                cosine = SinQ14((sint16)(1024 - angle)) >> 2;
                speed = character_controller.motion.horizontal_speed;
                speed = (sint32)(uint32)(((sint64)cosine * speed) >> 12);
                if (((uint32)speed ^ (uint32)character_controller.motion.horizontal_speed) >= 0x80000000)
                    speed = 0;
                character_controller.motion.horizontal_speed = (uint32)speed;
            }
            else if ((sint16)angle >= 512)
            {
                character_controller.motion.horizontal_speed = 0;
                *move_flags = *move_flags & 0xFFFFFBFE;
            }
        }
        flags = this->flags;
        if ((flags & 0x8000000) != 0 && ((uint16)(character_controller.motion.form_flags >> 16) & 0xF) == 4)
            goto done;
        value = (uint32)(uint16)(character_controller.motion.form_flags >> 16) & 0xF;
        if (value - 6 < 2)
        {
            if (character_controller.motion.horizontal_speed != 0)
                value = (flags & 0x10) != 0 ? 34 : 33;
            else
                value = 3;
        }
        else
            value = 3;
        if ((character_controller.secondary_flags & 0x80) == 0)
            character_controller.motion.state_code = value;
        QueueCharacterActionSound(0, 3);
        goto done;
    }
    duration = (sint16)character_controller.timers.jump_duration_ticks;
    if (character_controller.motion.jump_phase > 0)
    {
        value = character_controller.timers.state_ticks + 1;
        character_controller.timers.state_ticks = value;
        timer = (sint16)value;
        if (duration < timer || ((character_controller.flags & 0x8000000) != 0 && (duration >> 1) < timer))
        {
            flags = character_controller.flags;
            character_controller.timers.state_ticks = 0;
            character_controller.motion.jump_phase = 0;
            character_controller.flags = flags & 0xEFFFFFFF;
        }
    }
    if (((uint16)(character_controller.motion.form_flags >> 16) & 0xF) == 0)
    {
        mask = (*control_bindings.active_scheme)[1];
        if ((gameplay_input.prev_buttons & mask) == 0 && (gameplay_input.buttons & mask) != 0 && (character_controller.flags & 0x8000000) == 0 && (uint32)character_controller.motion.vertical_speed + 4504 < 0x4198)
            goto reactivate;
    }
    if ((game_flow.mode_flags & 0x400) == 0)
        goto done;
    mask = (*control_bindings.active_scheme)[1];
    if ((gameplay_input.prev_buttons & mask) != 0 || (gameplay_input.buttons & mask) == 0)
        goto done;
reactivate:
    value = (uint32)character_controller.model->movement.rise_speed_limit;
    flags = character_controller.movement.jump_ticks;
    x = (uint32)character_controller.movement.jump_speed;
    character_controller.timers.jump_duration_ticks = flags;
    character_controller.timers.state_ticks = 0;
    character_controller.motion.vertical_speed = x;
    character_controller.motion.jump_phase = 1;
    character_controller.movement.rise_speed_limit = value;
    this->frame = -1;
    if ((character_controller.secondary_flags & 0x80) == 0)
        character_controller.motion.state_code = 9;
    QueueCharacterActionSound(5, 3);
    character_controller.flags = (character_controller.flags | 0x8002000) & 0xEFFFFFFF;
done:
    result = *move_flags & 8;
    if (result == 0)
    {
        result = character_controller.flags & 0xFFFFDFFF;
        character_controller.flags = result;
    }
    return result;
}

uint32 AnimatedObject::UpdateSpecialMovement(uint32 *move_flags)
{
    FUNCTION_MARKER(0x8004BFD0, "SLUS_005.78");
    uint32 flags, value, kind, i, result;
    sint32 timer;
    if ((this->flags & 0x8000000) != 0 && (((uint32)(uint16)(character_controller.motion.form_flags >> 16) & 0xF) - 13 >= 2))
    {
        *move_flags = *move_flags & 0xFFFFFBFE;
        goto secondary;
    }
    if (((uint16)(character_controller.motion.form_flags >> 16) & 0xF) != 1)
    {
        this->flags = this->flags & 0xFBFFFFFF;
        goto secondary;
    }
    if ((this->flags & 0x4000000) != 0)
    {
        if (((*move_flags & 0x400) == 0 && (this->node.flags & 1) == 0) || (character_controller.flags & 0x18040) != 0 || (sint8)character_controller.motion.state_code != 9)
            goto restore;
        value = character_controller.timers.special_move_ticks - 1;
        character_controller.timers.special_move_ticks = value;
        if ((value & 0xFFFF) != 0)
            goto secondary;
    restore:
        this->flags = this->flags & 0xFBFFFFFF;
        if ((sint8)character_controller.motion.state_code == 9 && (character_controller.secondary_flags & 0x80) == 0)
            character_controller.motion.state_code = 2;
        for (i = 0; i < 4; ++i)
            character_controller.movement.speed_limits[i] = character_controller.special_move_backup[i];
        character_controller.motion.horizontal_speed = character_controller.special_move_backup[1];
        character_controller.special_move_backup[1] = (uint32)-15;
    }
    else
    {
        timer = (sint32)character_controller.special_move_backup[1];
        if (timer > 0)
        {
            character_controller.special_move_backup[1] = 0;
            timer = (sint32)character_controller.special_move_backup[1];
        }
        value = 0;
        if (timer == 0)
            value = 1;
        else
        {
            timer = (sint32)((uint32)timer + 1);
            character_controller.special_move_backup[1] = (uint32)timer;
            if (timer == 0)
                value = 1;
        }
        if (value == 0 || (*move_flags & 0x400) == 0 || (this->flags & 0x400) != 0)
        {
            *move_flags = *move_flags & 0xFFFFFBFF;
            goto secondary;
        }
        this->flags = this->flags | 0x4000000;
        if ((character_controller.secondary_flags & 0x80) == 0)
            character_controller.motion.state_code = 9;
        value = (uint32)character_controller.movement.speed_limits[0];
        character_controller.timers.special_move_ticks = 10;
        character_controller.special_move_backup[0] = value;
        value = (uint32)character_controller.movement.speed_limits[1];
        character_controller.special_move_backup[1] = value;
        value = (uint32)(((sint64)(sint32)value * 5120) >> 12);
        flags = (uint32)character_controller.movement.speed_limits[2];
        character_controller.movement.speed_limits[1] = value;
        character_controller.movement.speed_limits[0] = value;
        character_controller.special_move_backup[2] = flags;
        value = (uint32)character_controller.movement.speed_limits[3];
        character_controller.special_move_backup[3] = value;
        value = (uint32)(((sint64)(sint32)value * 5120) >> 12);
        character_controller.movement.speed_limits[3] = value;
        character_controller.movement.speed_limits[2] = value;
    }
secondary:
    if ((*move_flags & 0x80) != 0 && character_controller.model->animations[7] != 0)
    {
        flags = this->flags;
        if ((flags & 0x5000000) == 0 && (((uint16)(character_controller.motion.form_flags >> 16) & 0xF) == 3 || (flags & 0x400) == 0))
            this->flags = flags | 0x8000000;
        result = this->flags & 0x8000000;
        if (result != 0)
        {
            if ((character_controller.secondary_flags & 0x80) == 0)
                character_controller.motion.state_code = 7;
            result = character_controller.motion.slope_scale;
            if (result == 0)
                character_controller.motion.horizontal_speed = 0;
        }
    }
    else
    {
        result = this->flags & 0x8010000;
        if (result == 0x8000000)
        {
            kind = (uint16)(character_controller.motion.form_flags >> 16) & 0xF;
            if (kind == 4)
            {
                result = character_controller.flags & 0x60000;
                if (result != 0)
                    return result;
            }
            if ((sint8)character_controller.motion.state_code == 7 && (character_controller.secondary_flags & 0x80) == 0)
                character_controller.motion.state_code = 0;
            result = this->flags & 0xF7FFFFFF;
            this->flags = result;
        }
    }
    return result;
}

uint32 AnimatedObject::UpdateMovementState(uint32 *move_flags)
{
    FUNCTION_MARKER(0x8004C3B0, "SLUS_005.78");
    uint32 flags, value, kind, animation, i, result, decelerate;
    sint32 speed, state;
    if ((character_controller.flags & 0x10000) != 0)
        goto input_actions;
    if ((this->path_flags & 0x30000) != 0)
    {
        value = character_controller.path_transition.phase_ticks + 1;
        kind = path_transition_rules.special_ticks;
        character_controller.path_transition.phase_ticks = value;
        if (value >= kind)
            this->path_flags = this->path_flags & 0xFFFCFFFF;
    }
    decelerate = 0;
    if ((*move_flags & 0x401) == 0)
        decelerate = (this->path_flags & 0x30000) == 0;
    this->UpdateCharacterHorizontalSpeed(decelerate);
    if ((*move_flags & 0x10000) != 0)
    {
        if ((character_controller.secondary_flags & 0x80) == 0)
            character_controller.motion.state_code = 6;
    }
    else if ((*move_flags & 0x8000) != 0 && (this->flags & 0x400) == 0)
    {
        if ((*move_flags & 0x800) != 0)
            animation = 17;
        else if ((character_controller.motion.slope_scale != 0 && (sint32)this->velocity[1] < 0) || character_controller.motion.horizontal_speed <= 0)
            animation = 16;
        else
            animation = 15;
        if ((character_controller.secondary_flags & 0x80) == 0)
            character_controller.motion.state_code = animation;
    }
    else if ((this->path_flags & 1) != 0)
    {
        if (character_controller.motion.horizontal_speed != 0)
            animation = (gameplay_input.buttons & 0x1000) != 0 ? 10 : 12;
        else
            animation = 11;
        if ((character_controller.secondary_flags & 0x80) == 0)
            character_controller.motion.state_code = animation;
        if ((*move_flags & 0x800) != 0)
            character_controller.motion.horizontal_speed = (uint32)(((sint64)character_controller.movement.speed_limits[1] * character_controller.model->motion_scale[0]) >> 12);
        else if ((*move_flags & 0x1000) != 0)
            character_controller.motion.horizontal_speed = (uint32)(((sint64)character_controller.movement.speed_limits[1] * character_controller.model->motion_scale[1]) >> 12);
        else
            character_controller.motion.horizontal_speed = 0;
    }
    flags = this->flags;
    if ((flags & 0x9000400) != 0 || (this->path_flags & 0x1133) != 0)
        goto input_actions;
    speed = character_controller.motion.horizontal_speed;
    if (speed != 0)
    {
        if ((*move_flags & 0x401) != 0)
        {
            kind = (uint16)(character_controller.motion.form_flags >> 16) & 0xF;
            if (kind == 1 && (flags & 0x4000000) != 0)
                goto input_actions;
            goto moving_animation;
        }
        animation = character_controller.motion.state_code;
        if (animation - 4 < 2)
            goto input_actions;
        if (character_controller.motion.slope_scale != 0 || (character_controller.flags & 8) != 0)
        {
            animation = 8;
            goto set_animation;
        }
        if (speed > 0)
        {
            kind = (uint16)(character_controller.motion.form_flags >> 16) & 0xF;
            goto moving_animation;
        }
        goto idle_animation;
    }
    if (((uint16)(character_controller.motion.form_flags >> 16) & 0xF) == 1 && (flags & 0x4000000) != 0)
    {
        this->flags = flags & 0xFBFFFFFF;
        if ((sint8)character_controller.motion.state_code == 9 && (character_controller.secondary_flags & 0x80) == 0)
            character_controller.motion.state_code = 2;
        for (i = 0; i < 4; ++i)
            character_controller.movement.speed_limits[i] = character_controller.special_move_backup[i];
        character_controller.motion.horizontal_speed = character_controller.special_move_backup[1];
        character_controller.special_move_backup[1] = (uint32)-15;
        character_controller.motion.horizontal_speed = 0;
    }
    animation = character_controller.motion.state_code;
    if (animation - 4 < 2)
        goto input_actions;
    if ((character_controller.flags & 8) != 0)
    {
        animation = 8;
        goto set_animation;
    }
    if ((*move_flags & 1) != 0)
    {
        kind = (uint16)(character_controller.motion.form_flags >> 16) & 0xF;
        flags = this->flags;
        goto moving_animation;
    }
idle_animation:
    if ((uint32)(sint32)(sint8)animation - 1 < 34)
    {
        animation = 0;
        goto set_animation;
    }
    goto input_actions;
moving_animation:
    if (kind - 6 < 2)
        animation = (flags & 0x10) != 0 ? 32 : 31;
    else
        animation = 2;
set_animation:
    if ((character_controller.secondary_flags & 0x80) == 0)
        character_controller.motion.state_code = animation;
input_actions:
    if ((*move_flags & 0x100) != 0)
    {
        state = (sint8)character_controller.motion.state_code;
        character_controller.motion.jump_phase = 1;
        character_controller.timers.state_ticks = 0;
        if (state != 9 && state != 5)
        {
            if ((character_controller.secondary_flags & 0x80) == 0)
                character_controller.motion.state_code = 9;
            this->flags = this->flags | 0x400;
            if ((character_controller.flags & 0x10000) != 0)
            {
                value = character_controller.support.speed[1];
                character_controller.motion.horizontal_speed = 0;
                character_controller.motion.vertical_speed = value;
                if ((sint32)value < 0)
                    character_controller.motion.vertical_speed = 0 - value;
                character_controller.ClearTransientFlags();
            }
        }
    }
    result = 16;
    if ((this->path_flags & 1) != 0)
    {
        if (character_controller.path_input.dir == 16)
            result = character_controller.path_transition.motion_heading;
        else
            result = (character_controller.path_transition.motion_heading + 2048) & 0xFFF;
        this->yaw = result;
        this->target_yaw = result;
    }
    return result;
}

uint32 AnimatedObject::UpdateActionInput(uint32 *move_flags)
{
    FUNCTION_MARKER(0x8004CA00, "SLUS_005.78");
    uint32 kind, flags, result;
    if ((uint16)character_controller.weapon_state != 0)
        goto action;
    kind = (uint16)(character_controller.motion.form_flags >> 16) & 0xF;
    if (kind == 5 || kind - 10 < 2)
        goto action;
    if (kind == 1 && player_interaction.flags == 0 && (this->flags & 0x8000000) == 0 && (sint8)character_controller.motion.state_code != 4)
        goto action;
    result = (uint16)(character_controller.motion.form_flags >> 16) & 0xF;
    if (result != 4)
        return result;
action:
    result = *move_flags & 0x40;
    if (result == 0)
        return result;
    if ((sint16)character_controller.timers.action_cooldown_ticks != 0)
        return 0x5000000;
    flags = this->flags;
    result = flags & 0x5000000;
    if (result != 0)
        return result;
    result = this->node.flags & 1;
    if (result != 0)
        return result;
    if (character_controller.model->animations[5] == 0)
        return 0x1000000;
    flags |= 0x1000000;
    this->flags = flags;
    kind = (uint16)(character_controller.motion.form_flags >> 16) & 0xF;
    if (kind == 1)
    {
        if ((character_controller.secondary_flags & 0x80) == 0)
            character_controller.motion.state_code = 5;
        this->frame = -1;
        character_controller.timers.action_launch_ticks = 5;
        return 5;
    }
    if (kind != 4 && kind != 5 && kind - 10 >= 2)
    {
        if ((flags & 0x8000400) != 0 || (sint8)character_controller.motion.state_code == 2)
            return 2;
    }
    if ((character_controller.secondary_flags & 0x80) == 0)
        character_controller.motion.state_code = 5;
    this->frame = -1;
    return 0xFFFFFFFF;
}

uint32 AnimatedObject::ComputePathBoundaryExcess(const uint32 *position) const
{
    FUNCTION_MARKER(0x8002AB84, "SLUS_005.78");
    const PathDefinition *definition = (this->path_definition);
    MovementVector first, second;
    uint32 square = 0;
    uint32 length;
    uint32 midpoint[3];
    uint32 index;
    uint32 radius;
    uint32 result;
    if (definition == 0)
        return 0;
    (((const PathDefinition *)(definition)))->SamplePosition(0, first.words, 1);
    definition = (this->path_definition);
    (((const PathDefinition *)(definition)))->SamplePosition((sint16)((const PathDefinition *)(definition))->positions.metadata6 - 1, second.words, 1);
    for (index = 0; index < 3; ++index)
    {
        uint32 difference = first.words[index] - second.words[index];
        uint32 scaled = (uint32)((sint32)difference >> 6);
        square += scaled * scaled;
    }
    if ((sint32)square >= 0)
        length = SquareRoot12((sint32)square);
    else
        length = SquareRoot12((sint32)(square >> 2)) * 2;
    radius = (uint32)((sint32)length >> 1);
    for (index = 0; index < 3; ++index)
    {
        uint32 sum = first.words[index] + second.words[index];
        midpoint[index] = (uint32)((sint32)sum >> 1);
    }
    square = 0;
    for (index = 0; index < 3; ++index)
    {
        uint32 difference = position[index] - midpoint[index];
        uint32 scaled = (uint32)((sint32)difference >> 6);
        square += scaled * scaled;
    }
    radius -= 40960;
    if ((sint32)square >= 0)
        result = SquareRoot12((sint32)square) - radius;
    else
        result = SquareRoot12((sint32)(square >> 2)) * 2 - radius;
    return result;
}

uint32 AnimatedObject::ApplyCharacterPathMotion(sint32 horizontal_speed, sint32 vertical_speed)
{
    FUNCTION_MARKER(0x800455DC, "SLUS_005.78");
    const PathDefinition *parent;
    uint32 distance, x, y, z, result;
    const PathTransform *path_transform;
    MovementVector position, projected, source;
    distance = (uint32)horizontal_speed;
    if (horizontal_speed >= 0)
        this->flags = this->flags | 0x40;
    else
        distance = 0u - distance;
    this->ComputePathMotion((sint32)distance, vertical_speed, &character_controller.path_tangent);
    if (((uint16)(character_controller.motion.form_flags >> 16) & 15) == 5)
    {
        parent = (this->path_definition);
        if (parent != 0 && ((((const PathDefinition *)(parent)))->transform) != 0)
        {
            x = this->position[0];
            y = this->position[1];
            z = this->position[2];
            source.words[0] = x;
            source.words[1] = y;
            source.words[2] = z;
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
            ApplyRotMatrixLV(&source.vector, &position.vector);
            PopMatrix();
            path_transform = (const PathTransform *)((((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) != 0 ? (((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) : (PathTransform *)0);
            position.words[0] = position.words[0] + (uint32)path_transform->matrix.t[0];
            path_transform = (const PathTransform *)((((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) != 0 ? (((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) : (PathTransform *)0);
            position.words[1] = position.words[1] + (uint32)path_transform->matrix.t[1];
            path_transform = (const PathTransform *)((((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) != 0 ? (((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) : (PathTransform *)0);
            position.words[2] = position.words[2] + (uint32)path_transform->matrix.t[2];
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
        projected.words[0] = position.words[0] + this->velocity[0];
        projected.words[1] = position.words[1] + this->velocity[1];
        projected.words[2] = position.words[2] + this->velocity[2];
        result = this->ComputePathBoundaryExcess(projected.words);
        if ((sint32)result > 0)
            this->ComputePathMotion(0, 0, &character_controller.path_tangent);
    }
    result = this->flags & 0xFFFFFFBF;
    this->flags = result;
    return result;
}

uint32 AnimatedObject::UpdateTransitionMotion(uint32 input_flags, uint32 mode, uint32 *horizontal, uint32 *vertical)
{
    FUNCTION_MARKER(0x80058D08, "SLUS_005.78");
    uint32 flags, input, result, mask;
    AnimationHeader *animation;
    sint32 state, count, threshold;
    flags = this->path_flags;
    input = gameplay_input.buttons;
    if ((flags & 0x200000) != 0 && (character_controller.secondary_flags & 0x80) == 0)
        character_controller.motion.state_code = 3;
    flags = this->path_flags;
    if ((flags & 0x40000) != 0)
    {
        if ((sint8)character_controller.path_transition.phase != 6)
            return 6;
        if (this->frame != 0)
            return 7;
        character_controller.path_transition.phase = 7;
        if ((character_controller.secondary_flags & 0x80) == 0)
            character_controller.motion.state_code = 3;
        result = this->path_flags | 0x200000;
        this->path_flags = result;
        return result;
    }
    if ((flags & 0x200000) != 0)
        return 1;
    state = (sint8)character_controller.path_transition.phase;
    if (state == 1)
    {
        character_controller.motion.vertical_speed = 0;
        *vertical = 0;
        if ((character_controller.secondary_flags & 0x240) != 0)
            character_controller.path_transition.release_pending = 0;
        count = (sint8)character_controller.path_transition.hold_ticks;
        threshold = (sint8)path_transition_rules.release_ticks;
        if (count < threshold)
        {
            character_controller.path_transition.hold_ticks = count + 1;
            mask = (this->path_flags & 0x400) != 0 ? 0x8000 : 0x2000;
            if ((input & mask) == 0)
                character_controller.path_transition.release_pending = 1;
        }
        else if (count == threshold)
        {
            if ((sint8)character_controller.path_transition.release_pending == 0)
                character_controller.path_transition.phase = 2;
            else
            {
                mask = (this->path_flags & 0x400) != 0 ? 0x8000 : 0x2000;
                if ((input & mask) != 0)
                    character_controller.path_transition.phase = 2;
            }
        }
        state = (sint8)character_controller.path_transition.phase;
    }
    if (state == 2)
    {
        character_controller.motion.vertical_speed = 0;
        *vertical = 0;
        character_controller.path_transition.phase = 3;
    }
    else if (state >= 3)
    {
        if (state == 3)
        {
            if ((sint8)character_controller.motion.state_code == 20)
            {
                animation = this->animation;
                count = ((AnimationHeader *)(animation))->frame_count;
                threshold = this->frame;
                if (threshold != count - 1)
                {
                    *vertical = 0;
                    character_controller.motion.vertical_speed = 0;
                    goto animation;
                }
            }
            *horizontal = 0;
            character_controller.motion.horizontal_speed = 0;
            *vertical = 0;
            character_controller.motion.vertical_speed = 0;
        }
        character_controller.path_transition.phase = 4;
    }
    else if (state < 0)
        character_controller.path_transition.phase = 4;
animation:
    if ((character_controller.secondary_flags & 0x80) == 0)
        character_controller.motion.state_code = 20;
    return 20;
}

uint32 AnimatedObject::UpdateSpecialAxisMotion(uint32 input_flags, uint32 mode, uint32 *horizontal, uint32 *vertical)
{
    CharacterModelRecord *pointer_record;
    FUNCTION_MARKER(0x80058F50, "SLUS_005.78");
    uint32 flags, result, delta, value, limit, count;
    sint32 coefficient, scale;
    sint64 product;
    flags = this->path_flags;
    if (!(flags & 4))
    {
        result = mode & 8;
        if (result)
            return result;
        if (!(character_controller.flags & 0x40000))
        {
            delta = 0;
            if (input_flags & 0x800)
            {
                pointer_record = character_controller.model;
                coefficient = pointer_record->motion_scale[0];
                scale = character_controller.movement.speed_limits[1];
                delta = (uint32)(((sint64)scale * coefficient) >> 12);
            }
            else if (input_flags & 0x1000)
            {
                pointer_record = character_controller.model;
                scale = character_controller.movement.speed_limits[1];
                coefficient = pointer_record->motion_scale[1];
                delta = 0 - (uint32)(((sint64)scale * coefficient) >> 12);
            }
            result = (uint32)character_controller.motion.vertical_speed + delta;
            character_controller.motion.vertical_speed = result;
            *vertical = result;
        }
        result = (input_flags & 0x800) ? 10 : ((input_flags & 0x1000) ? 12 : 11);
        if (!(character_controller.secondary_flags & 0x80))
            character_controller.motion.state_code = result;
        return result;
    }
    result = character_controller.flags & 0x40000;
    if (result)
        return result;
    limit = path_transition_rules.axis_input_ticks;
    count = character_controller.path_transition.movement_ticks;
    result = input_flags & 0x30;
    if (count >= limit)
        return result;
    coefficient = path_transition_rules.axis_horizontal_acceleration[(input_flags & 0x30) != 0];
    scale = character_controller.movement.speed_limits[1];
    product = (sint64)coefficient * scale;
    delta = (uint32)(product >> 12);
    result = (uint32)character_controller.motion.horizontal_speed + delta;
    character_controller.motion.horizontal_speed = result;
    *horizontal = result;
    coefficient = path_transition_rules.axis_vertical_limits[0];
    scale = character_controller.movement.speed_limits[1];
    product = (sint64)coefficient * scale;
    value = (uint32)(product >> 12);
    result = character_controller.motion.vertical_speed;
    if ((sint32)value >= (sint32)result)
        return result;
    coefficient = path_transition_rules.axis_vertical_limits[(input_flags & 0x30) != 0];
    scale = character_controller.movement.speed_limits[1];
    product = (sint64)coefficient * scale;
    value = (uint32)(product >> 12);
    result = (uint32)((uint64)product >> 32) << 20;
    character_controller.motion.vertical_speed = value;
    *vertical = value;
    return result;
}

void AnimatedObject::ApplyEnvironmentMotion(uint32 *horizontal, uint32 *vertical)
{
    FUNCTION_MARKER(0x80049730, "SLUS_005.78");
    uint32 value, result, acceleration, input, sum, flags, upper, lower;
    if ((character_controller.flags & 0x80000) != 0)
    {
        value = *vertical + 101;
        *vertical = value;
        if ((sint32)value > 0)
            this->flags = this->flags | 0x400;
        value = character_controller.movement.jump_ticks;
        character_controller.motion.jump_phase = 1;
        character_controller.timers.state_ticks = value;
        *horizontal = 0;
        character_controller.motion.horizontal_speed = 0;
        result = *vertical;
        character_controller.motion.vertical_speed = result;
        return;
    }
    acceleration = character_controller.surface_interaction.environment->parameters.words[1];
    if (acceleration != 0)
    {
        input = gameplay_input.buttons;
        if ((input & 0x1000) != 0)
            acceleration += 40;
        else if ((input & 0x4000) != 0)
            acceleration -= 40;
        sum = *vertical + acceleration;
        *vertical = sum;
        if ((sint32)acceleration > 0)
        {
            if ((sint32)sum > 0)
            {
                flags = this->flags;
                if ((flags & 0x400) == 0)
                {
                    this->flags = flags | 0x400;
                    if ((character_controller.secondary_flags & 0x80) == 0)
                        character_controller.motion.state_code = 3;
                }
            }
            value = character_controller.movement.jump_ticks;
            character_controller.motion.jump_phase = 1;
            character_controller.timers.state_ticks = value;
        }
        else if ((this->flags & 0x400) != 0 && (sint32)sum < 0 && character_controller.motion.jump_phase >= 0)
            character_controller.motion.jump_phase = -1;
        character_controller.motion.environment_acceleration = 0;
        character_controller.motion.vertical_speed = *vertical;
    }
    acceleration = character_controller.surface_interaction.environment->parameters.words[0];
    if (acceleration == 0)
        return;
    if ((this->flags & 0x10) == 0)
        acceleration = 0 - acceleration;
    value = ((const SceneTrigger *)(character_controller.surface_interaction.environment))->parameters.words[0];
    character_controller.motion.environment_acceleration = value;
    *horizontal = *horizontal + acceleration;
    if ((this->flags & 0x400) != 0)
        value = (uint32)character_controller.movement.speed_limits[3];
    else
        value = (uint32)character_controller.movement.speed_limits[1];
    lower = 0 - value;
    sum = *horizontal;
    upper = value + acceleration;
    result = (sint32)upper < (sint32)sum;
    lower += acceleration;
    if (result != 0)
        *horizontal = upper;
    else
    {
        result = (sint32)sum < (sint32)lower;
        if (result != 0)
            *horizontal = lower;
    }
    return;
}

uint32 AnimatedObject::IntegrateCharacterMovement(uint32 input_flags)
{
    FUNCTION_MARKER(0x8004CBA4, "SLUS_005.78");
    const ScenePath *camera_path;
    uint32 horizontal, vertical, flags, value, kind, state, i, grounded, result;
    const PathDefinition *definition;
    const PathTransform *path_transform;
    MovementVector position, floor, original;
    sint32 lower_limit, acceleration, upper_limit, rise, speed, direction, frame;
    if ((input_flags & 2) != 0 && (this->flags & 0x9000000) == 0 && (((uint32)(uint16)(character_controller.motion.form_flags >> 16) & 0xF) - 13 >= 2))
        goto prepare;
    if ((input_flags & 0x20) != 0 && (this->path_flags & 2) != 0)
    {
    prepare:
        this->ReverseScenePath();
    }
    flags = character_controller.flags;
    if ((flags & 0x10000) != 0)
    {
        this->UpdateCharacterSupportMotion();
        goto finish_motion;
    }
    if ((this->flags & 0x400) == 0 && character_controller.motion.horizontal_speed == 0 && (flags & 0x99048) == 0 && character_controller.motion.environment_acceleration == 0 && (this->path_flags & 0x122) == 0)
    {
        this->velocity[2] = 0;
        this->velocity[1] = 0;
        this->velocity[0] = 0;
        goto finish_motion;
    }
    flags = this->flags;
    value = character_controller.motion.horizontal_speed;
    horizontal = value;
    if ((flags & 0x400) == 0 && (this->path_flags & 0x122) == 0)
    {
        vertical = 0;
        goto adjust_horizontal;
    }
    lower_limit = character_controller.movement.fall_speed_limit;
    acceleration = character_controller.movement.fall_acceleration;
    if ((input_flags & 0x20000) != 0 || (this->path_flags & 0x200000) != 0)
    {
        this->UpdateTransitionMotion(input_flags, 0, &horizontal, &vertical);
        goto integrate_vertical;
    }
    if ((input_flags & 0x10000) != 0 && path_transition_rules.release_mode == 0)
    {
        value = (uint32)(((sint64)path_transition_rules.release_motion_scale * character_controller.movement.speed_limits[1]) >> 12);
        character_controller.motion.vertical_speed = value;
        flags = character_controller.motion.vertical_speed;
        vertical = value;
        value = 0 - flags;
        character_controller.motion.vertical_speed = value;
        vertical = value;
        goto integrate_vertical;
    }
    if ((input_flags & 0x2000) != 0 || (this->path_flags & 4) != 0)
    {
        this->UpdateSpecialAxisMotion(input_flags, (input_flags & 0x2000) != 0 ? 8 : 0, &horizontal, &vertical);
        goto integrate_vertical;
    }
    if ((input_flags & 0x200) != 0)
    {
        state = (uint32)(sint8)character_controller.motion.state_code;
        character_controller.motion.jump_phase = -1;
        character_controller.timers.state_ticks = 0;
        if (state != 5 && (character_controller.secondary_flags & 0x80) == 0)
            character_controller.motion.state_code = 7;
        goto integrate_vertical;
    }
    kind = (uint16)(character_controller.motion.form_flags >> 16) & 0xF;
    if ((kind != 5 && kind - 10 >= 2) || (input_flags & 0x100) != 0)
        goto integrate_vertical;
    if (character_controller.motion.vertical_speed < lower_limit)
    {
        character_controller.motion.jump_phase = 1;
        input_flags |= 0x100;
    }
    state = (uint32)(sint8)character_controller.motion.state_code;
    if (state != 3 || (input_flags & 8) != 0)
        goto check_animation;
    if ((character_controller.flags & 0x60000) != 0)
    {
        state = (uint32)(sint8)character_controller.motion.state_code;
    check_animation:
        if (state == 5 || state == 3)
            goto integrate_vertical;
    }
    flags = character_controller.secondary_flags;
    if ((flags & 0x400) == 0 && (flags & 0x80) == 0)
        character_controller.motion.state_code = (input_flags & 3) != 0 ? 9 : 6;
integrate_vertical:
    if ((character_controller.flags & 0x800000) != 0)
    {
        const SceneTrigger *jump_scale = character_controller.surface_interaction.jump_scale;
        upper_limit = (sint32)(((sint64)character_controller.movement.rise_speed_limit * (sint32)jump_scale->parameters.words[1]) >> 12);
        rise = (sint32)(((sint64)character_controller.movement.rise_acceleration * (sint32)jump_scale->parameters.words[1]) >> 12);
    }
    else
    {
        upper_limit = character_controller.movement.rise_speed_limit;
        rise = character_controller.movement.rise_acceleration;
    }
    flags = this->path_flags;
    if ((flags & 2) != 0 && (flags & 4) == 0)
    {
        direction = character_controller.motion.jump_phase;
        if (direction > 0)
        {
            speed = character_controller.motion.vertical_speed;
            if (speed < upper_limit)
                character_controller.motion.vertical_speed = (uint32)speed;
            direction = character_controller.motion.jump_phase;
        }
        if (direction < 0)
        {
            speed = character_controller.motion.vertical_speed;
            if (lower_limit < speed)
            {
                character_controller.motion.vertical_speed = (uint32)speed;
                if (speed < lower_limit)
                    goto clamp_lower;
            }
        }
        goto save_vertical;
    }
    direction = character_controller.motion.jump_phase;
    if (direction > 0)
    {
        speed = character_controller.motion.vertical_speed;
        if (speed < upper_limit)
        {
            character_controller.motion.vertical_speed = (uint32)speed;
            if (((flags & 2) != 0 && (input_flags & 0x4000) != 0) || ((flags & 2) == 0 && (input_flags & 0x108) != 0))
                character_controller.motion.vertical_speed = (uint32)speed + (uint32)rise;
            speed = character_controller.motion.vertical_speed;
            character_controller.motion.vertical_speed = (uint32)speed;
            if (speed >= upper_limit)
            {
                character_controller.motion.vertical_speed = (uint32)upper_limit;
                vertical = (uint32)upper_limit;
                goto adjust_horizontal;
            }
        }
        direction = character_controller.motion.jump_phase;
    }
    if (direction < 0)
    {
        speed = character_controller.motion.vertical_speed;
        if (lower_limit < speed)
        {
            speed = (sint32)((uint32)speed + (uint32)acceleration);
            character_controller.motion.vertical_speed = (uint32)speed;
            if (speed < lower_limit)
            {
            clamp_lower:
                character_controller.motion.vertical_speed = (uint32)lower_limit;
                vertical = (uint32)lower_limit;
                goto adjust_horizontal;
            }
        }
    }
save_vertical:
    vertical = (uint32)character_controller.motion.vertical_speed;
adjust_horizontal:
    if ((character_controller.flags & 0x80008) != 0)
        this->ApplyEnvironmentMotion(&horizontal, &vertical);
    else
    {
        value = character_controller.motion.environment_acceleration;
        if (value != 0)
        {
            if ((this->flags & 0x10) != 0)
                value = (uint32)character_controller.motion.horizontal_speed + value;
            else
                value = (uint32)character_controller.motion.horizontal_speed - value;
            character_controller.motion.horizontal_speed = value;
            value = character_controller.motion.horizontal_speed;
            character_controller.motion.environment_acceleration = 0;
            horizontal = value;
        }
    }
    speed = (sint32)horizontal;
    if (speed > 15360)
        horizontal = 15360;
    else if (speed < -15360)
        horizontal = (uint32)-15360;
    value = horizontal;
    flags = vertical;
    character_controller.support.speed[0] = value;
    character_controller.support.speed[1] = flags;
    flags = this->flags;
    grounded = (flags >> 10) & 1;
    if (grounded != 0)
        character_controller.flags = character_controller.flags & 0xFFFFEFFF;
    else if ((character_controller.flags & 0x1000) != 0)
        this->flags = flags | 0x400;
    this->ApplyCharacterPathMotion((sint32)horizontal, (sint32)vertical);
    flags = character_controller.flags;
    if ((flags & 0x40) != 0)
        this->ResolveCharacterSupport(grounded);
    else if ((flags & 0x8000) != 0)
        this->ProbeCharacterSupport();
    if (grounded != 0)
        goto finish_motion;
    if ((((const PathDefinition *)((this->path_definition)))->flags & 0x11) != 0)
        this->flags = this->flags | 0x400;
    if ((this->flags & 0x400) == 0)
        goto finish;
    if ((sint32)horizontal <= 0 || (character_controller.flags & 0x1000) != 0)
        goto stop_vertical;
    definition = (character_controller.path->definition);
    if ((((const PathDefinition *)(definition))->flags & 0x11) != 0)
    {
    stop_vertical:
        value = character_controller.movement.jump_ticks;
        flags = character_controller.flags;
        kind = (uint16)(character_controller.motion.form_flags >> 16) & 0xF;
        character_controller.motion.jump_phase = -1;
        character_controller.motion.vertical_speed = 0;
        character_controller.timers.state_ticks = 0;
        character_controller.timers.jump_duration_ticks = value;
        character_controller.flags = flags | 0x8000000;
        if ((kind != 1 || (this->flags & 0x4000000) == 0) && (this->path_flags & 0x100) == 0 && (character_controller.secondary_flags & 0x80) == 0)
            character_controller.motion.state_code = 6;
        goto finish_motion;
    }
    horizontal = (uint32)(sint16)((const PathDefinition *)(definition))->positions.metadata6;
    definition = (this->path_definition);
    if (definition != 0 && ((((const PathDefinition *)(definition)))->transform) != 0)
    {
        value = this->position[0];
        flags = this->position[1];
        kind = this->position[2];
        original.words[0] = value;
        original.words[1] = flags;
        original.words[2] = kind;
        PushMatrix();
        path_transform = (const PathTransform *)((((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) != 0 ? (((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) : (PathTransform *)0);
        for (i = 0; i < 5; ++i)
            xport_gte_write_control(i, xport_load_le32((const uint8 *)&path_transform->matrix + i * 4));
        ApplyRotMatrixLV(&original.vector, &position.vector);
        PopMatrix();
        for (i = 0; i < 3; ++i)
        {
            path_transform = (const PathTransform *)((((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) != 0 ? (((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) : (PathTransform *)0);
            value = position.words[i];
            value += (uint32)path_transform->matrix.t[i];
            position.words[i] = value;
        }
    }
    else
    {
        value = this->position[0];
        flags = this->position[1];
        kind = this->position[2];
        position.words[0] = value;
        position.words[1] = flags;
        position.words[2] = kind;
    }
    for (i = 0; i < 3; ++i)
    {
        value = position.words[i];
        value += this->velocity[i];
        position.words[i] = value;
    }
    definition = (character_controller.path->definition);
    frame = (sint32)horizontal;
    direction = (((const PathDefinition *)(definition)))->ProjectFrame(frame, position.words, floor.words, this->flags & 0x10, 6144);
    if (direction == (sint32)horizontal)
    {
        this->flags = this->flags & 0xFFFFFBFF;
        character_controller.flags = character_controller.flags | 0x1000;
    }
finish_motion:
    if ((this->flags & 0x400) != 0 && character_controller.motion.jump_phase == 0)
    {
        flags = character_controller.flags;
        character_controller.motion.jump_phase = -1;
        character_controller.flags = flags & 0xEFFFFFFF;
    }
finish:

    character_controller.support.contact = 0;
    camera_path = camera_motion.path;
    if ((((((const ScenePath *)(camera_path))->definition) != 0 ? (((const ScenePath *)(camera_path))->definition) : (const PathDefinition *)0)->flags & 0x100011) != 0)
        character_controller.motion.slope_scale = 0;
    else
        character_controller.motion.slope_scale = ((const ScenePath *)(camera_path))->slope_scale;
    result = character_controller.flags & 0xF855B041;
    character_controller.flags = result;
    return result;
}

sint32 AnimatedObject::EvaluatePathTransition(const uint32 *input) const
{
    FUNCTION_MARKER(0x8005814C, "SLUS_005.78");
    MovementVector current, target, first, second;
    const ScenePath *path;
    const PathDefinition *definition;
    uint32 x, y, z, flags, value;
    const PathTransform *path_transform;
    const SceneTrigger *anchor;
    uint32 height, distance, dx, dz, angle, difference, result;
    sint32 first_frame, second_frame;
    anchor = (const SceneTrigger *)(character_controller.path_transition.anchor);
    path = (anchor->parameters.path.target);
    result = 0;
    definition = path->definition;
    path_transform = definition->transform;
    if (path_transform != 0)
    {
        flags = this->path_flags;
        (((const ScenePath *)(path)))->SamplePosition(0, target.words, (flags & 0x400) == 0 ? 64 : 0);
    }
    else
    {
        x = anchor->position[0];
        y = anchor->position[1];
        z = anchor->position[2];
        target.words[0] = x;
        target.words[1] = y;
        target.words[2] = z;
    }
    definition = this->path_definition;
    if (definition != 0 && (definition->transform) != 0)
    {
        x = this->position[0];
        y = this->position[1];
        z = this->position[2];
        first.words[0] = x;
        first.words[1] = y;
        first.words[2] = z;
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
        ApplyRotMatrixLV(&first.vector, &current.vector);
        PopMatrix();
        definition = this->path_definition;
        path_transform = definition->transform;
        x = current.words[0];
        x += (uint32)path_transform->matrix.t[0];
        current.words[0] = x;
        definition = this->path_definition;
        path_transform = definition->transform;
        y = current.words[1];
        y += (uint32)path_transform->matrix.t[1];
        current.words[1] = y;
        definition = this->path_definition;
        path_transform = definition->transform;
        z = current.words[2];
        z += (uint32)path_transform->matrix.t[2];
        current.words[2] = z;
    }
    else
    {
        x = this->position[0];
        y = this->position[1];
        z = this->position[2];
        current.words[0] = x;
        current.words[1] = y;
        current.words[2] = z;
    }
    y = target.words[1];
    y -= current.words[1];
    value = path_transition_rules.height_bias;
    x = current.words[0];
    height = y - value;
    x -= target.words[0];
    z = current.words[2];
    value = target.words[2];
    if ((sint32)x < 0)
        x = 0 - x;
    y = path_transition_rules.approach_limit;
    z -= value;
    if ((sint32)z < 0)
        z = 0 - z;
    distance = x + z;
    if ((sint32)y < (sint32)distance)
    {
        result = 0xFFFFFFFF;
        goto done;
    }
    if ((this->path_flags & 1) == 0)
    {
        if (character_controller.motion.vertical_speed < 0 && (sint32)path_transition_rules.max_height < (sint32)height)
        {
            result = 0xFFFFFFFF;
            goto done;
        }
        if ((this->path_flags & 1) != 0 && (sint32)path_transition_rules.max_height < (sint32)height)
        {
            result = 0xFFFFFFFF;
            goto done;
        }
    }
    else if ((sint32)path_transition_rules.max_height < (sint32)height)
    {
        result = 0xFFFFFFFF;
        goto done;
    }
    second_frame = 1;
    if ((this->path_flags & 0x400) != 0)
    {
        definition = path->definition;
        first_frame = (sint16)((const PathDefinition *)(definition))->positions.metadata6;
        second_frame = first_frame - 2;
        --first_frame;
    }
    else
        first_frame = 0;
    (((const ScenePath *)(path)))->SamplePosition(first_frame, first.words, 64);
    (((const ScenePath *)(path)))->SamplePosition(second_frame, second.words, 64);
    x = second.words[0];
    dx = x - first.words[0];
    y = second.words[1];
    y -= first.words[1];
    z = second.words[2];
    dz = z - first.words[2];
    angle = Atan2Angle((sint32)(0 - dz), (sint32)(0 - dx));
    character_controller.path_transition.heading = angle;
    x = path_heading_delta[0];
    y = path_heading_delta[1];
    z = path_heading_delta[2];
    character_controller.path_transition.probe_position[0] = x;
    character_controller.path_transition.probe_position[1] = y;
    character_controller.path_transition.probe_position[2] = z;
    value = (uint32)(sint32)this->yaw;
    difference = value - (uint32)(sint32)(sint16)angle;
    if ((sint32)difference < 0)
        difference = 0 - difference;
    if ((sint16)difference >= 2049)
        difference = 4096 - difference;
    character_controller.path_transition.approach_distance = distance;
    flags = this->path_flags;
    if ((flags & 1) != 0)
    {
        if ((sint32)path_transition_rules.air_height_limit >= (sint32)height)
            result = 1;
        goto done;
    }
    if (character_controller.motion.vertical_speed > 0)
        goto done;
    if ((flags & 0x400) != 0)
    {
        if ((this->flags & 0x10) != 0)
            goto done;
    }
    else if ((this->flags & 0x10) == 0)
        goto done;
    if ((sint16)difference >= (sint16)path_transition_rules.angle_limit)
        goto done;
    if ((sint32)height < (sint32)path_transition_rules.min_height || (sint32)path_transition_rules.max_height < (sint32)height)
        goto done;
    if ((this->path_flags & 0x400) != 0)
    {
        if ((*input & 0x2000) == 0)
            result = 1;
    }
    else if ((*input & 0x8000) == 0)
        result = 1;
done:
    return (sint32)result;
}

uint32 AnimatedObject::BuildMovementFlags()
{
    const PathTransform *captured_path_transform;
    FUNCTION_MARKER(0x8004AA68, "SLUS_005.78");
    uint32 input, result = 0, previous, flags, value, mask, filtered, x, y, z;
    const PathTransform *path_transform;
    sint32 timer, state, limit, response;
    if ((character_controller.flags & 0x4000000) != 0)
    {
        this->node.flags = this->node.flags & 0xFE;
        character_controller.flags = character_controller.flags & 0xFBFFFFFF;
        gameplay_input.buttons = 0;
        goto finish;
    }
    if ((sint16)character_controller.timers.transition_ticks != 0 || (this->node.flags & 1) != 0)
        goto finish;
    input = gameplay_input.buttons;
    previous = gameplay_input.prev_buttons;
    timer = (sint8)character_controller.path_transition.contact_delay;
    if (timer > 0)
        character_controller.path_transition.contact_delay = (uint32)timer - 1;
    state = character_controller.path_input.routing_phase;
    if (state == 0)
    {
        if ((player_interaction.flags & 0x10) != 0 || (path_routing.flags & 0x10) != 0)
            character_controller.path_input.routing_phase = 1;
    }
    else if (state == 1)
    {
        if ((player_interaction.flags & 0x10) == 0 && (path_routing.flags & 0x10) == 0)
            character_controller.path_input.routing_phase = 2;
    }
    else
    {
        character_controller.path_input.routing_phase = 2;
        mask = (*control_bindings.active_scheme)[0];
        if ((gameplay_input.buttons & mask) == 0)
            character_controller.path_input.routing_phase = 0;
    }
    if ((this->path_flags & 4) != 0)
        this->UpdateCharacterPathTimeout();
    if ((this->flags & 0x400) != 0)
        character_controller.path_transition.movement_ticks = character_controller.path_transition.movement_ticks + 1;
    else if ((this->path_flags & 4) == 0)
        character_controller.path_transition.movement_ticks = 0;
    value = character_controller.path_transition.reentry_ticks;
    if (value != 0)
        character_controller.path_transition.reentry_ticks = value - 1;
    if ((this->path_flags & 0x40000) != 0 && (sint8)character_controller.path_transition.phase == 8)
    {
        value = character_controller.path_transition.phase_ticks + 1;
        limit = (sint8)path_transition_rules.exit_ticks;
        character_controller.path_transition.phase_ticks = value;
        if ((uint32)limit < value)
            this->path_flags = this->path_flags & 0xFFFBFFFF;
    }
    if ((this->path_flags & 0x80) != 0 && (sint8)character_controller.path_transition.phase == 0)
    {
        response = this->EvaluatePathTransition(&input);
        if (response == -1)
            this->ClearCharacterPathTransitionFlags();
        else if (response == 1)
        {
            flags = this->path_flags;
            if ((flags & 1) != 0)
            {
                if ((flags & 0x400) != 0)
                {
                    value = character_controller.path_transition.motion_heading;
                    result = 2;
                    this->yaw = value;
                    this->target_yaw = value;
                }
                this->EndCharacterConstrainedPath();
            }
            if ((this->path_flags & 0x820) != 0)
            {
                this->ClearCharacterPathRelease();
                this->ClearCharacterPathStep();
            }
            this->BeginCharacterPathTransition();
        }
    }
    if ((this->path_flags & 0x1000000) != 0 && ((input & ((*control_bindings.active_scheme)[1] | 0xA000)) != 0 || (character_controller.flags & 0x60000) != 0))
        this->ClearCharacterPathExitLatch();
    if ((this->path_flags & 0x20) != 0)
    {
        timer = character_controller.path_transition.hold_motion_ticks;
        if (timer < path_transition_rules.hold_motion_limit)
        {
            value = (uint32)timer + 1;
            character_controller.path_transition.hold_motion_ticks = value;
            if ((sint8)value < path_transition_rules.hold_motion_limit)
                this->path_flags = this->path_flags | 0x8000;
        }
    }
    if ((character_controller.secondary_flags & 0x240) != 0 || ((uint32)(uint16)(character_controller.motion.form_flags >> 16) & 0xF) - 13 < 2)
        result |= 1;
    if ((this->path_flags & 0x3C8) == 0)
        character_controller.path_transition.path = 0;
    flags = this->path_flags;
    if ((flags & 0x100) != 0 || ((flags & 0x80) != 0 && (sint8)character_controller.path_transition.phase > 0))
    {
        if ((character_controller.secondary_flags & 0x240) != 0 && (sint8)character_controller.path_transition.phase < 4)
            result &= 0xFFFFFFFE;
        flags = this->path_flags;
        if ((flags & 0x640000) != 0 || camera_motion.path != character_controller.path_transition.path)
            goto post_transition;
        if ((sint8)character_controller.path_transition.phase >= 4)
        {
            this->CancelCharacterPathTransition();
            goto post_transition;
        }
        if ((sint8)character_controller.path_transition.hold_ticks >= (sint8)path_transition_rules.exit_hold_ticks)
        {
            if (((flags & 0x400) != 0 && (input & 0x2000) != 0) || ((flags & 0x400) == 0 && (input & 0x8000) != 0))
            {
                this->ExitCharacterPathTransition();
                if ((input & 0x2000) != 0)
                    this->path_flags = this->path_flags | 0x10000;
                else
                    this->path_flags = this->path_flags | 0x20000;
                goto post_transition;
            }
        }
        if ((sint8)character_controller.path_transition.phase < 4 && (character_controller.secondary_flags & 0x240) == 0 && (input & 0x4000) != 0 && (previous & 0x4000) == 0)
            this->RequestCharacterPathExit();
    post_transition:
        if ((this->path_flags & 0x40100) != 0)
            result |= 0x20000;
    }
    if ((((((const ScenePath *)(camera_motion.path))->definition) != 0 ? (((const ScenePath *)(camera_motion.path))->definition) : (const PathDefinition *)0)->flags & 0x80000) != 0 && (this->path_flags & 0x30) == 0)
        this->ArmCharacterPathHold();
    flags = this->path_flags;
    if ((flags & 0x10) != 0)
    {
        if ((input & 0x4000) != 0 && (previous & 0x4000) == 0 && character_controller.path_input.routing_phase == 0 && (input & 0xA000) == 0 && (this->flags & 0x400) == 0 && (character_controller.flags & 0x800000) == 0)
        {
            if ((flags & 0x20) == 0)
                this->BeginCharacterPathRelease();
        }
        else if ((this->path_flags & 0x20) == 0)
        {
            if ((((((const ScenePath *)(camera_motion.path))->definition) != 0 ? (((const ScenePath *)(camera_motion.path))->definition) : (const PathDefinition *)0)->flags & 0x80000) != 0)
                character_controller.path_transition.hold_motion_ticks = 0;
            else
            {
                timer = character_controller.path_transition.hold_motion_ticks;
                if (timer < 4)
                    character_controller.path_transition.hold_motion_ticks = (uint32)timer + 1;
                else
                    this->EndCharacterPathHold();
            }
        }
    }
    if ((this->path_flags & 0x20) != 0)
        result |= 0x10000;
    if ((this->path_flags & 0x1081) == 0 && (((((const ScenePath *)(camera_motion.path))->definition) != 0 ? (((const ScenePath *)(camera_motion.path))->definition) : (const PathDefinition *)0)->flags & 0x10000) != 0)
    {
        this->BeginCharacterConstrainedPath();
        if ((input & 0xA000) != 0)
            value = (input & 0x2000) != 0 ? 16 : 32;
        else
            value = (this->flags & 0x10) != 0 ? 16 : 32;
        character_controller.path_input.dir = value;
    }
    if ((((((const ScenePath *)(camera_motion.path))->definition) != 0 ? (((const ScenePath *)(camera_motion.path))->definition) : (const PathDefinition *)0)->flags & 0x10000) == 0)
    {
        flags = this->path_flags;
        if ((flags & 1) == 0)
            goto ungrounded;
        if ((flags & 0x80) == 0 || (result & 0x800) == 0)
        {
            timer = character_controller.path_input.settle_ticks;
            if (timer < 4)
                character_controller.path_input.settle_ticks = (uint32)timer + 1;
            else
                this->EndCharacterConstrainedPath();
        }
    }
    if ((this->path_flags & 1) != 0)
    {
        mask = (*control_bindings.active_scheme)[1];
        if ((input & mask) != 0 && (previous & mask) == 0)
        {
            this->BeginCharacterSurfaceMotion();
            state = character_controller.path_input.dir;
            character_controller.motion.jump_phase = 1;
            character_controller.motion.vertical_speed = 0;
            character_controller.timers.state_ticks = 0;
            this->frame = -1;
            result |= 0x2000;
            if ((state == 16 && (this->flags & 0x10) != 0) || (state == 32 && (this->flags & 0x10) == 0))
            {
                this->ReverseScenePath();
                value = (uint16)this->target_yaw;
                this->yaw = value;
                this->orientation[2] = (value + 2048) & 0xFFF;
            }
            goto movement;
        }
        if (character_controller.path_transition.input_ready == 0)
        {
            character_controller.path_transition.input_ready = 1;
            goto movement;
        }
        if (character_controller.path_input.routing_phase != 0)
            goto movement;
        path_transform = ((captured_path_transform = (((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform)) != 0 ? (const PathTransform *)((PathTransform *)(captured_path_transform)) : 0);
        filtered = input;
        value = path_transform != 0 && path_transform->displacement[1] != 0;
        if (value == 0 && ((character_controller.flags & 0x40) != 0 || character_controller.path_input.latch_phase != 0))
        {
            if (character_controller.path_input.latch_phase != 0)
            {
                character_controller.path_input.latch_phase = 2;
                filtered &= 0xFFFFEFFF;
            }
            else
            {
                value = character_controller.path_input.prev_motion;
                if ((value & 0x800) != 0)
                {
                    filtered &= 0xFFFFEFFF;
                    character_controller.path_input.latched_motion = value;
                    character_controller.path_input.latch_phase = 1;
                    character_controller.path_input.release_ticks = 0;
                }
            }
            if (character_controller.path_input.latch_phase != 0 && (input & 0x4000) != 0)
            {
                value = character_controller.path_input.release_ticks + 1;
                character_controller.path_input.release_ticks = value;
                if ((sint32)value >= 2)
                    character_controller.path_input.latch_phase = 0;
            }
        }
        if ((this->flags & 0x10) != 0)
        {
            if ((filtered & 0x1000) != 0)
                result |= 0x801;
            else if ((filtered & 0x4000) != 0)
                result |= 2;
        }
        else
        {
            if ((filtered & 0x4000) != 0)
                result |= 0x1001;
            else if ((filtered & 0x1000) != 0)
                result |= 2;
        }
        value = result & 0x1800;
        if (value != 0)
            character_controller.path_input.prev_motion = value;
        if ((input & 0x2000) != 0)
            character_controller.path_input.dir = 32;
        if ((input & 0x8000) != 0)
            character_controller.path_input.dir = 16;
        result |= (uint32)character_controller.path_input.dir;
        goto movement;
    }
ungrounded:
    if ((this->path_flags & 4) != 0 && (this->flags & 0x400) == 0)
        this->EndCharacterConstrainedPath();
movement:
    flags = this->path_flags;
    if ((flags & 6) != 2 && (flags & 0x101) == 0)
    {
        if ((this->flags & 0x10) != 0)
        {
            if ((input & 0x2000) != 0)
                result |= 1;
            else if ((input & 0x8000) != 0)
                result |= 2;
        }
        else
        {
            if ((input & 0x8000) != 0)
                result |= 1;
            else if ((input & 0x2000) != 0)
                result |= 2;
        }
    }
    if ((this->path_flags & 0x10) != 0)
    {
        result |= 0x8000;
        if ((input & 0x1000) != 0 && (this->flags & 0x400) == 0)
            result = (result | 0x800) & 0xFFFFFFFC;
    }
    if ((sint32)character_controller.flags < 0)
        goto finish;
    flags = this->path_flags;
    if ((flags & 0x1120) != 0)
        goto finish;
    if ((character_controller.secondary_flags & 0x240) != 0)
        goto button_actions;
    if ((input & 0x4000) != 0 && (flags & 0x15) == 0)
        result |= 0x80;
    mask = (*control_bindings.active_scheme)[0];
    if ((input & mask) != 0)
    {
        if ((previous & mask) == 0)
            result |= 0x40;
        else
        {
            value = (uint16)(character_controller.motion.form_flags >> 16) & 0xF;
            if (value == 4 || value == 5 || value - 10 < 2)
                result |= 0x40;
        }
    }
    if ((result & 0x40) != 0)
    {
        if ((character_controller.secondary_flags & 0x100) != 0)
            this->flags = this->flags | 0x1000000;
        flags = player_interaction.flags;
        if ((flags & 4) != 0)
        {
            player_interaction.flags = flags | 8;
            result ^= 0x40;
        }
    }
    else
    {
        flags = player_interaction.flags;
        value = flags & 0xFFFFFFF7;
        player_interaction.flags = value;
        mask = (*control_bindings.active_scheme)[0];
        if ((input & mask) != 0)
        {
            if ((flags & 0x10) != 0)
                result &= 0xFFFFFFFD;
        }
        else
        {
            x = path_routing.flags;
            player_interaction.flags = value & 0xFFFFFFEF;
            path_routing.flags = x & 0xFFFFFFEF;
        }
    }
    value = (uint16)(character_controller.motion.form_flags >> 16) & 0xF;
    if (value == 5 || value - 10 < 2)
    {
        if ((result & 0x80) != 0)
        {
            flags = this->flags;
            result &= 0xFFFFFF7F;
            if ((flags & 0x400) != 0)
                result |= 0x200;
        }
        else if ((input & 0x1000) != 0)
            result |= 0x100;
    }
    else if (value == 1)
    {
        if ((this->flags & 0x4000000) != 0)
            result = 1024;
        else
        {
            mask = (*control_bindings.active_scheme)[2];
            value = input & mask;
            if (value == mask && (previous & value) != value)
                result |= 0x400;
        }
    }
button_actions:
    mask = (*control_bindings.active_scheme)[1];
    if ((input & mask) != 0)
    {
        result |= 8;
        if ((previous & mask) == 0)
        {
            if (character_controller.motion.buffered_rise_limit != 0)
            {
                if ((sint16)character_controller.timers.jump_input_ticks < 6)
                {
                    x = character_controller.motion.buffered_horizontal_speed;
                    y = character_controller.motion.buffered_vertical_speed;
                    z = character_controller.motion.buffered_rise_limit;
                    flags = character_controller.flags;
                    character_controller.motion.jump_phase = 1;
                    character_controller.timers.state_ticks = 0;
                    character_controller.motion.horizontal_speed = x;
                    character_controller.motion.vertical_speed = y;
                    character_controller.movement.rise_speed_limit = z;
                    character_controller.flags = flags | 0x10000000;
                }
                character_controller.motion.buffered_rise_limit = 0;
            }
            character_controller.timers.jump_input_ticks = 0xFFFF;
        }
    }
    timer = (sint16)character_controller.timers.jump_input_ticks;
    if (timer < 6)
        character_controller.timers.jump_input_ticks = (uint32)timer + 1;
finish:
    if (character_controller.support.release_pending == 1)
    {
        flags = character_controller.flags;
        character_controller.support.release_pending = 0;
        character_controller.flags = flags & 0xFFFFF7EF;
    }
    return result;
}

uint32 AnimatedObject::UpdateCharacterMovement()
{
    FUNCTION_MARKER(0x8004DCD0, "SLUS_005.78");
    uint32 previous_kind = character_controller.motion.state_code;
    sint32 timer = (sint16)character_controller.timers.score_bonus_ticks;
    const ScenePath *path;
    uint32 move_flags, flags, value, input, mask, result;
    character_controller.motion.prev_state_code = previous_kind;
    flags = character_controller.movement_flags;
    path = camera_motion.path;
    character_controller.movement_flags = flags & 0xF83FFFFF;
    character_controller.path = path;
    if (timer != 0)
    {
        value = (uint32)timer - 1;
        character_controller.timers.score_bonus_ticks = value;
        if ((value & 0xFFFF) == 0)
            character_controller.flags = character_controller.flags & 0xFFBFFFFF;
    }
    flags = game_flow.transition_flags;
    input = gameplay_input.buttons;
    gameplay_input.prev_buttons = input;
    if ((flags & 0x102) != 0)
    {
        gameplay_input.buttons = 0;
        ReadGamepadButtons();
    }
    else
    {
        value = ReadGamepadButtons();
        gameplay_input.buttons = FilterDebugInput(value);
    }
    mask = gameplay_input.release_mask;
    if (mask != 0)
    {
        input = gameplay_input.buttons;
        gameplay_input.release_mask = mask & input;
        gameplay_input.buttons = input & ~mask;
    }
    if ((sint32)GetCutsceneMode() < 0)
        gameplay_input.buttons = gameplay_input.buttons & 0x900;
    gameplay_input.ApplyDemo();
    flags = character_controller.secondary_flags;
    if ((flags & 0x400) != 0 && this->frame == ((AnimationHeader *)(this->animation))->frame_count - 1)
        character_controller.secondary_flags = flags & 0xFFFFFBFF;
    value = this->BuildMovementFlags();
    flags = this->flags;
    move_flags = value;
    if ((flags & 0x8000000) != 0 && ((uint16)(character_controller.motion.form_flags >> 16) & 0xF) == 4)
        move_flags = value & 0xFFFFFFF7;
    this->UpdateMovementImpulse(&move_flags);
    this->UpdateJumpMotion(&move_flags);
    this->UpdateSpecialMovement(&move_flags);
    this->UpdateMovementState(&move_flags);
    this->UpdateActionInput(&move_flags);
    result = this->IntegrateCharacterMovement(move_flags);
    return result;
}

uint32 AnimatedObject::UpdateForm5Movement()
{
    FUNCTION_MARKER(0x8002AD4C, "SLUS_005.78");
    uint32 character = 0x800ABD68;
    uint32 input, flags, result, speed, acceleration, limit;
    if ((character_controller.secondary_flags & 0x400) != 0 && this->frame == ((AnimationHeader *)(this->animation))->frame_count - 1)
        character_controller.secondary_flags = character_controller.secondary_flags & 0xFFFFFBFF;
    this->flags = this->flags | 0x400;
    UpdateGameplayInput();
    input = gameplay_input.buttons;
    flags = 0;
    speed = character_controller.motion.horizontal_speed;
    acceleration = (uint32)character_controller.movement.acceleration0;
    if ((input & 0x2000) != 0)
        speed = (this->flags & 0x10) != 0 ? speed + acceleration : speed - acceleration;
    else if ((input & 0x8000) != 0)
        speed = (this->flags & 0x10) != 0 ? speed - acceleration : speed + acceleration;
    else
        speed = (uint32)((sint32)(speed * 7) / 8);
    character_controller.motion.horizontal_speed = speed;
    if ((sint32)speed < 0)
        flags |= 2;
    limit = (uint32)character_controller.movement.acceleration0 << 2;
    if ((sint32)limit < (sint32)speed)
        character_controller.motion.horizontal_speed = limit;
    speed = character_controller.motion.vertical_speed;
    if ((input & 0x1000) != 0)
    {
        speed += (uint32)character_controller.movement.acceleration0;
        character_controller.motion.vertical_speed = speed;
        limit = (uint32)character_controller.movement.acceleration0 << 2;
        if ((sint32)limit < (sint32)speed)
            character_controller.motion.vertical_speed = limit;
    }
    else if ((input & 0x4000) != 0)
    {
        speed -= (uint32)character_controller.movement.acceleration0;
        character_controller.motion.vertical_speed = speed;
        limit = (0 - (uint32)character_controller.movement.acceleration0) << 2;
        if ((sint32)speed < (sint32)limit)
            character_controller.motion.vertical_speed = limit;
    }
    else
        character_controller.motion.vertical_speed = (uint32)((sint32)(speed * 7) / 8);
    character_controller.motion.jump_phase = 0;
    if ((input & (*control_bindings.active_scheme)[0]) != 0)
        flags |= 0x40;
    this->UpdateMovementImpulse(&flags);
    this->UpdateActionInput(&flags);
    result = this->IntegrateCharacterMovement(flags);
    return result;
}

uint32 ReadCharacterMovementFlags(void)
{
    FUNCTION_MARKER(0x800534EC, "SLUS_005.78");
    return character_controller.movement_flags;
}

uint32 AnimatedObject::LeaveCharacterSupport()
{
    const PathTransform *captured_path_transform;
    FUNCTION_MARKER(0x8004DF14, "SLUS_005.78");
    MovementVector vector;
    const PathDefinition *parent;
    uint32 result, x, y, z;
    const PathTransform *path_transform;
    result = this->flags & 0x2000;
    if (result)
        return result;
    parent = (this->path_definition);
    path_transform = parent != 0 ? ((captured_path_transform = ((((const PathDefinition *)(parent)))->transform)) != 0 ? (const PathTransform *)((PathTransform *)(captured_path_transform)) : 0) : 0;
    if (path_transform)
    {
        x = this->position[0];
        y = this->position[1];
        z = this->position[2];
        vector.words[0] = x;
        vector.words[1] = y;
        vector.words[2] = z;
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
        ApplyRotMatrixLV(&vector.vector, (VECTOR *)this->position);
        PopMatrix();
        path_transform = (const PathTransform *)((((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) != 0 ? (((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) : (PathTransform *)0);
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
        x = this->position[2];
        y = (uint32)path_transform->matrix.t[2];
        this->position[2] = x + y;
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
    this->flags = this->flags | 0x2400;
    x = (uint32)character_controller.path_velocity.vx;
    y = (uint32)character_controller.path_velocity.vy;
    z = (uint32)character_controller.path_velocity.vz;
    character_controller.path_tangent.vx = 0 - x;
    character_controller.path_tangent.vy = 0 - y;
    character_controller.path_tangent.vz = 0 - z;
    character_controller.path_tangent.vy = 0;
    if ((uint32)character_controller.path_tangent.vx + 26754 > 0xD103 || (uint32)character_controller.path_tangent.vz + 26754 > 0xD103)
    {
        for (;;)
        {
            x = (uint32)character_controller.path_tangent.vx;
            z = (uint32)character_controller.path_tangent.vz;
            character_controller.path_tangent.vx = (uint32)((sint32)x >> 1);
            y = (uint32)character_controller.path_tangent.vy;
            x = (uint32)character_controller.path_tangent.vx;
            z = (uint32)((sint32)z >> 1);
            character_controller.path_tangent.vz = z;
            character_controller.path_tangent.vy = (uint32)((sint32)y >> 1);
            if (x + 26754 > 0xD103)
                continue;
            if ((uint32)character_controller.path_tangent.vy + 26754 > 0xD103)
                continue;
            if (z + 26754 <= 0xD103)
                break;
        }
    }
    result = (uint32)VectorNormal(&character_controller.path_tangent, &character_controller.path_tangent);
    return result;
}

uint32 AnimatedObject::ApplyCharacterContactMotion()
{
    const PathTransform *captured_path_transform;
    FUNCTION_MARKER(0x8004EB74, "SLUS_005.78");
    const ScenePath *support, *current, *saved_path;
    MovementVector velocity, parent_velocity, input, temporary;

    union
    {
        uint32 words[5];
        uint16 halves[10];
    } transpose;

    const PathDefinition *parent, *definition, *other_definition;
    uint32 x, y, z, flags, fraction, scene, direction, kind, moved, was_grounded, was_detached, special;
    uint32 same_definition;
    const PathTransform *path_transform;
    uint32 first, second, third;
    sint32 frame;
    if ((this->path_flags & 0x100) != 0)
        this->BeginCharacterPathTransfer();
    x = this->velocity[0];
    y = this->velocity[1];
    z = this->velocity[2];
    velocity.words[0] = x;
    velocity.words[1] = y;
    velocity.words[2] = z;
    if ((sint32)this->path_motion_scale < 4096)
    {
        character_controller.flags = character_controller.flags & 0xFFFFEFFF;
        parent_velocity.words[2] = 0;
        parent_velocity.words[1] = 0;
        parent_velocity.words[0] = 0;
        flags = this->flags;
        this->flags = flags & 0xFFFFF7FF;
        if ((flags & 0x4000) != 0)
        {
            if ((this->path_flags & 2) != 0 && (flags & 0x400) != 0)
            {
                scene = OriginalDefaults::SceneContextIdentity;
                current = camera_motion.path;
                definition = current->definition;
                if ((((const PathDefinition *)(definition))->flags & 0x10000) == 0)
                    this->ClearCharacterSurfaceMotion();
            }
            scene = OriginalDefaults::SceneContextIdentity;
            current = camera_motion.path;
            definition = current->definition;
            if ((((const PathDefinition *)(definition))->flags & 0x80000) == 0)
            {
                if ((this->path_flags & 0x20) != 0)
                    this->ClearCharacterPathRelease();
                if ((this->path_flags & 0x10) != 0)
                    this->EndCharacterPathHold();
            }
            if ((this->path_flags & 0x200) != 0)
                this->CancelCharacterPathTransition();
            if ((this->path_flags & 0x1000) != 0)
                this->ClearCharacterPathAuxiliaryMode();
            character_controller.support.motion[2] = 0;
            character_controller.support.motion[1] = 0;
            character_controller.support.motion[0] = 0;
            this->flags = this->flags & 0xFFFFFBFF;
        }
        else
        {
            parent = (this->path_definition);
            if (parent != 0)
            {
                path_transform = ((captured_path_transform = ((((const PathDefinition *)(parent)))->transform)) != 0 ? (const PathTransform *)((PathTransform *)(captured_path_transform)) : 0);
                if (path_transform != 0 && (flags & 0x2000) == 0)
                {
                    x = path_transform->displacement[0];
                    y = path_transform->displacement[1];
                    z = path_transform->displacement[2];
                    parent_velocity.words[0] = x;
                    parent_velocity.words[1] = y;
                    parent_velocity.words[2] = z;
                }
            }
        }
        x = velocity.words[0] + parent_velocity.words[0];
        y = velocity.words[1] + parent_velocity.words[1];
        z = velocity.words[2] + parent_velocity.words[2];
        velocity.words[0] = x;
        velocity.words[1] = y;
        velocity.words[2] = z;
        fraction = this->path_motion_scale;
        if (fraction != 0 || (this->path_flags & 6) != 2)
        {
            velocity.words[0] = (uint32)(((sint64)(sint32)x * (sint32)fraction) >> 12);
            velocity.words[1] = (uint32)(((sint64)(sint32)y * (sint32)fraction) >> 12);
            velocity.words[2] = (uint32)(((sint64)(sint32)z * (sint32)fraction) >> 12);
        }
        x = velocity.words[0] - parent_velocity.words[0];
        y = velocity.words[1] - parent_velocity.words[1];
        z = velocity.words[2] - parent_velocity.words[2];
        velocity.words[0] = x;
        velocity.words[1] = y;
        velocity.words[2] = z;
        fraction = 4096 - this->path_motion_scale;
        if ((character_controller.flags & 0x18040) != 0)
        {
            flags = character_controller.flags;
            character_controller.support.motion[2] = 0;
            character_controller.support.motion[0] = 0;
            if ((flags & 0x18000) != 0)
                character_controller.support.motion[1] = 0;
        }
        x = (uint32)(((sint64)(sint32)character_controller.support.motion[0] * (sint32)fraction) >> 12);
        y = character_controller.support.motion[1];
        character_controller.support.motion[0] = x;
        y = (uint32)(((sint64)(sint32)y * (sint32)fraction) >> 12);
        z = character_controller.support.motion[2];
        character_controller.support.motion[1] = y;
        character_controller.support.motion[2] = (uint32)(((sint64)(sint32)z * (sint32)fraction) >> 12);
        velocity.words[0] = velocity.words[0] + character_controller.support.motion[0];
        velocity.words[1] = velocity.words[1] + character_controller.support.motion[1];
        velocity.words[2] = velocity.words[2] + character_controller.support.motion[2];
    }
    moved = velocity.words[0] != 0 || velocity.words[2] != 0;
    if (moved != 0)
        character_controller.movement_flags = character_controller.movement_flags | 0x2000000;
    else if ((sint32)velocity.words[1] <= 0)
        goto rotate_velocity;
    if ((frame_clock.render_tick & 1) != 0 && character_controller.motion.trail_effect >= 0 && (this->path_flags & 0x400011) == 0)
    {
        scene = OriginalDefaults::SceneContextIdentity;
        current = camera_motion.path;
        definition = current->definition;
        if ((((const PathDefinition *)(definition))->flags & 0x80000) == 0)
        {
            if ((this->flags & 0x2000) != 0)
                goto effect_position;
            parent = (this->path_definition);
            if (parent == 0 || ((((const PathDefinition *)(parent)))->transform) == 0)
                goto effect_position;
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
            ApplyRotMatrixLV(&input.vector, &parent_velocity.vector);
            PopMatrix();
            path_transform = (const PathTransform *)((((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) != 0 ? (((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) : (PathTransform *)0);
            x = parent_velocity.words[0];
            parent_velocity.words[0] = x + (uint32)path_transform->matrix.t[0];
            path_transform = (const PathTransform *)((((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) != 0 ? (((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) : (PathTransform *)0);
            y = parent_velocity.words[1];
            parent_velocity.words[1] = y + (uint32)path_transform->matrix.t[1];
            path_transform = (const PathTransform *)((((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) != 0 ? (((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) : (PathTransform *)0);
            z = parent_velocity.words[2];
            parent_velocity.words[2] = z + (uint32)path_transform->matrix.t[2];
            goto emit_effect;
        effect_position:
            x = this->position[0];
            y = this->position[1];
            z = this->position[2];
            parent_velocity.words[0] = x;
            parent_velocity.words[1] = y;
            parent_velocity.words[2] = z;
        emit_effect:
            kind = (uint16)(character_controller.motion.form_flags >> 16) & 15;
            if (kind != 5 && kind - 10 >= 2)
                SpawnEffectSprite(parent_velocity.words, 4096, character_controller.motion.trail_effect + 7);
        }
    }
rotate_velocity:
    if ((this->flags & 0x2000) != 0)
    {
        if (this->path_motion_scale == 4096)
            this->path_motion_scale = 4095;
    }
    else
    {
        parent = (this->path_definition);
        if (parent != 0)
        {
            path_transform = ((captured_path_transform = ((((const PathDefinition *)(parent)))->transform)) != 0 ? (const PathTransform *)((PathTransform *)(captured_path_transform)) : 0);
            if (path_transform != 0)
            {
                transpose.halves[0] = path_transform->matrix.m[0][0];
                transpose.halves[1] = ((const MATRIX *)((((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) != 0 ? &(((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform)->matrix : (MATRIX *)0))->m[1][0];
                transpose.halves[2] = ((const MATRIX *)((((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) != 0 ? &(((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform)->matrix : (MATRIX *)0))->m[2][0];
                transpose.halves[3] = ((const MATRIX *)((((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) != 0 ? &(((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform)->matrix : (MATRIX *)0))->m[0][1];
                transpose.halves[4] = ((const MATRIX *)((((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) != 0 ? &(((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform)->matrix : (MATRIX *)0))->m[1][1];
                transpose.halves[5] = ((const MATRIX *)((((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) != 0 ? &(((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform)->matrix : (MATRIX *)0))->m[2][1];
                transpose.halves[6] = ((const MATRIX *)((((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) != 0 ? &(((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform)->matrix : (MATRIX *)0))->m[0][2];
                transpose.halves[7] = ((const MATRIX *)((((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) != 0 ? &(((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform)->matrix : (MATRIX *)0))->m[1][2];
                transpose.halves[8] = ((const MATRIX *)((((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) != 0 ? &(((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform)->matrix : (MATRIX *)0))->m[2][2];
                x = velocity.words[0];
                y = velocity.words[1];
                z = velocity.words[2];
                input.words[0] = x;
                input.words[1] = y;
                input.words[2] = z;
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
                ApplyRotMatrixLV(&input.vector, &velocity.vector);
                PopMatrix();
            }
        }
    }
    this->position[0] = this->position[0] + velocity.words[0];
    this->position[1] = this->position[1] + velocity.words[1];
    this->position[2] = this->position[2] + velocity.words[2];
    if ((this->flags & 0x4000) != 0 || (this->path_flags & 0x100) != 0)
        goto final_position;
    flags = character_controller.flags;
    if ((flags & 0x22) != 0)
        goto final_position;
    fraction = this->path_motion_scale;
    if ((sint32)fraction >= 4096 && ((flags & 1) == 0 || (sint32)fraction < (sint32)character_controller.support.fraction))
        goto final_position;
    scene = OriginalDefaults::SceneContextIdentity;
    camera_motion.frame = (uint16)this->path_frame;
    flags = this->flags;
    x = character_controller.flags;
    was_grounded = (flags >> 10) & 1;
    was_detached = (flags >> 13) & 1;
    if ((x & 1) == 0 || (sint32)this->path_motion_scale < (sint32)character_controller.support.fraction)
        character_controller.support.candidate = 0;
    moved = 1;
    support = this->FindCharacterSupportPath(character_controller.support.candidate);
    if (support == 0)
    {
        if ((this->flags & 0x2000) != 0 || character_controller.support.candidate != 0)
            goto grounded_transition;
        parent = (this->path_definition);
        if (parent != 0)
        {
            path_transform = ((captured_path_transform = ((((const PathDefinition *)(parent)))->transform)) != 0 ? (const PathTransform *)((PathTransform *)(captured_path_transform)) : 0);
            if (path_transform != 0)
            {
                x = this->velocity[0];
                y = path_transform->displacement[0];
                parent = (this->path_definition);
                this->velocity[0] = x + y;
                path_transform = (const PathTransform *)(((((const PathDefinition *)(parent)))->transform) != 0 ? ((((const PathDefinition *)(parent)))->transform) : (PathTransform *)0);
                x = this->velocity[1];
                y = path_transform->displacement[1];
                parent = (this->path_definition);
                this->velocity[1] = x + y;
                path_transform = (const PathTransform *)(((((const PathDefinition *)(parent)))->transform) != 0 ? ((((const PathDefinition *)(parent)))->transform) : (PathTransform *)0);
                x = this->velocity[2];
                this->velocity[2] = x + path_transform->displacement[2];
            }
        }
        this->LeaveCharacterSupport();
        goto grounded_transition;
    }
    flags = this->flags;
    if ((flags & 0x4000) != 0)
    {
        if (character_controller.support.candidate != 0)
            character_controller.flags = character_controller.flags & 0xFCC5F459;
        else
        {
            this->flags = flags & 0xFFFFBBFF;
            character_controller.flags = character_controller.flags & 0xF7FFFFFF;
        }
    }
    special = 0;
    if ((this->path_flags & 0x20) != 0 && support == character_controller.path_transition.anchor_path)
        special = 1;
    scene = OriginalDefaults::SceneContextIdentity;
    if (support == camera_motion.path || special != 0)
        goto grounded_transition;
    parent = (this->path_definition);
    if (parent != 0 && ((((const PathDefinition *)(parent)))->transform) != 0)
    {
        x = this->position[0];
        y = this->position[1];
        z = this->position[2];
        temporary.words[0] = x;
        temporary.words[1] = y;
        temporary.words[2] = z;
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
        scene = OriginalDefaults::SceneContextIdentity;
        ApplyRotMatrixLV(&temporary.vector, (VECTOR *)camera_motion.focus_position);
        PopMatrix();
        scene = OriginalDefaults::SceneContextIdentity;
        path_transform = (const PathTransform *)((((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) != 0 ? (((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) : (PathTransform *)0);
        x = camera_motion.focus_position[0];
        camera_motion.focus_position[0] = x + (uint32)path_transform->matrix.t[0];
        path_transform = (const PathTransform *)((((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) != 0 ? (((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) : (PathTransform *)0);
        y = camera_motion.focus_position[1];
        camera_motion.focus_position[1] = y + (uint32)path_transform->matrix.t[1];
        path_transform = (const PathTransform *)((((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) != 0 ? (((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) : (PathTransform *)0);
        z = camera_motion.focus_position[2];
        camera_motion.focus_position[2] = z + (uint32)path_transform->matrix.t[2];
    }
    else
    {
        scene = OriginalDefaults::SceneContextIdentity;
        x = this->position[0];
        y = this->position[1];
        z = this->position[2];
        camera_motion.focus_position[0] = x;
        camera_motion.focus_position[1] = y;
        camera_motion.focus_position[2] = z;
    }
    frame = this->path_frame;
    if (was_detached != 0)
        this->flags = this->flags | 0x2000;
    else
    {
        scene = OriginalDefaults::SceneContextIdentity;
        current = camera_motion.path;
        flags = this->flags;
        definition = current->definition;
        this->path_frame = (((const PathDefinition *)(definition)))->FindHeightFrame((const uint32 *)camera_motion.focus_position, flags & 0x10, -40960);
    }
    scene = OriginalDefaults::SceneContextIdentity;
    if (support != camera_motion.path)
    {
        direction = this->flags & 0x10;
        support->ProjectOffset(frame, (uint32 *)camera_motion.desired_position, direction);
        scene = OriginalDefaults::SceneContextIdentity;
        current = camera_motion.path;
        definition = support->definition;
        other_definition = current->definition;
        same_definition = definition == other_definition;
        if (same_definition != 0)
            direction ^= 0x10;
        else
            this->ApplyPathVelocityChange(definition);
        scene = OriginalDefaults::SceneContextIdentity;
        if ((camera_motion.smooth_ticks[0] | camera_motion.smooth_ticks[1] | camera_motion.smooth_ticks[2]) == 0)
        {
            current = camera_motion.path;
            if (current == character_controller.path && (this->flags & 0x2000) == 0)
            {
                kind = (uint32)(sint32)this->path_frame;
                if ((sint32)kind >= 0)
                    ((const ScenePath *)(current))->ProjectOffset((sint32)kind, (uint32 *)camera_motion.position, direction);
            }
        }
        SelectScenePath(support);
    }
    flags = this->flags;
    this->path_frame = frame;
    this->flags = flags & 0xFFFFDFFF;
    x = character_controller.motion.vertical_speed;
    if (character_controller.movement.fall_speed_limit >= (sint32)x)
        character_controller.timers.state_ticks = 0;
    if (character_controller.support.candidate != 0 && (character_controller.flags & 0x8040) != 0)
        character_controller.ClearTransientFlags();
grounded_transition:
    if ((this->flags & 0x400) != 0 && was_grounded == 0)
    {
        x = character_controller.movement.jump_ticks;
        flags = character_controller.flags;
        kind = (uint16)(character_controller.motion.form_flags >> 16);
        character_controller.motion.jump_phase = -1;
        character_controller.motion.vertical_speed = 0;
        character_controller.timers.state_ticks = 0;
        character_controller.timers.jump_duration_ticks = x;
        character_controller.flags = flags | 0x8000000;
        if (((kind & 15) != 1 || (this->flags & 0x4000000) == 0) && (character_controller.secondary_flags & 0x80) == 0)
            character_controller.motion.state_code = 6;
    }
final_position:
    scene = OriginalDefaults::SceneContextIdentity;
    current = character_controller.path;
    if (camera_motion.path != current)
    {
        temporary.words[0] = camera_motion.desired_position[0] - camera_motion.position[0];
        temporary.words[1] = camera_motion.desired_position[1] - camera_motion.position[1];
        temporary.words[2] = camera_motion.desired_position[2] - camera_motion.position[2];
        SetCameraTransition(temporary.words);
        scene = OriginalDefaults::SceneContextIdentity;
        current = camera_motion.path;
        saved_path = character_controller.path;
        definition = current->definition;
        other_definition = saved_path->definition;
        same_definition = definition == other_definition;
        if (same_definition == 0 && (camera_motion.smooth_ticks[0] | camera_motion.smooth_ticks[1] | camera_motion.smooth_ticks[2]) != 0)
        {
            first = camera_motion.smooth_ticks[0];
            second = camera_motion.smooth_ticks[1];
            third = camera_motion.smooth_ticks[2];
            x = this->velocity[0] * 2;
            y = this->velocity[1] * 2;
            z = this->velocity[2] * 2;
            temporary.words[0] = temporary.words[0] - x;
            temporary.words[1] = temporary.words[1] - y;
            temporary.words[2] = temporary.words[2] - z;
            SetCameraTransition(temporary.words);
            scene = OriginalDefaults::SceneContextIdentity;
            if (first < camera_motion.smooth_ticks[0])
            {
                camera_motion.smooth_ticks[0] = (uint8)(first);
                scene = OriginalDefaults::SceneContextIdentity;
            }
            if (second < camera_motion.smooth_ticks[1])
                camera_motion.smooth_ticks[1] = (uint8)(second);
            scene = OriginalDefaults::SceneContextIdentity;
            if (third < camera_motion.smooth_ticks[2])
            {
                camera_motion.smooth_ticks[2] = (uint8)(third);
                scene = OriginalDefaults::SceneContextIdentity;
            }
        }
        else
            scene = OriginalDefaults::SceneContextIdentity;
        moved = 1;
        camera_motion.flags = camera_motion.flags & 0xFE;
    }
    scene = OriginalDefaults::SceneContextIdentity;
    camera_motion.frame = (uint16)this->path_frame;
    character_controller.motion.position_changed = moved != 0 ? 1 : 0;
    flags = character_controller.flags;
    x = character_controller.secondary_flags;
    character_controller.flags = flags & 0xFFFFFFFE;
    character_controller.secondary_flags = x & 0xFFFFFFF6;
    return 0xFFFFFFF6;
}

uint32 AnimatedObject::EndCharacterSpecialMove()
{
    FUNCTION_MARKER(0x8004A888, "SLUS_005.78");
    this->flags = this->flags & ~0x4000000;
    if ((sint8)character_controller.motion.state_code == 9 && (character_controller.secondary_flags & 128) == 0)
        character_controller.motion.state_code = 2;
    character_controller.movement.speed_limits[0] = character_controller.special_move_backup[0];
    character_controller.movement.speed_limits[1] = character_controller.special_move_backup[1];
    character_controller.movement.speed_limits[2] = character_controller.special_move_backup[2];
    character_controller.movement.speed_limits[3] = character_controller.special_move_backup[3];
    character_controller.motion.horizontal_speed = character_controller.special_move_backup[1];
    character_controller.special_move_backup[1] = (uint32)-15;
    return (uint32)-15;
}

uint32 AnimatedObject::UpdateCharacterContactState()
{
    FUNCTION_MARKER(0x800521BC, "SLUS_005.78");
    uint32 flags, value, kind;
    sint32 animation, limit;
    if ((this->flags & 0x4000) != 0)
    {
        if (character_controller.motion.vertical_speed < 0)
            QueueCharacterActionSound(1, 3);
        flags = character_controller.movement_flags;
        character_controller.motion.vertical_speed = 0;
        character_controller.timers.contact_impulse_ticks = 0;
        character_controller.movement_flags = flags | 0x800000;
        this->flags = this->flags & 0xEFFFBBFF;
        flags = character_controller.flags;
        if ((flags & 0x18000) != 0)
            character_controller.flags = (flags | 0x40) & 0xFFFE7FFF;
        flags = character_controller.flags;
        value = character_controller.motion.horizontal_speed;
        limit = character_controller.movement.speed_limits[1];
        character_controller.flags = flags & 0xC7F9FFFF;
        if (limit < (sint32)value)
            character_controller.motion.horizontal_speed = limit;
    }
    if ((this->path_flags & 0x100) != 0)
        this->BeginCharacterPathTransfer();
    flags = this->flags;
    if ((flags & 0x800) != 0)
        return this->DetachCharacterFromPath(0);
    if ((this->path_flags & 0x800) != 0)
    {
        this->flags = flags | 0x2000;
        return this->DetachCharacterFromPath(0);
    }
    if ((flags & 0x400) == 0)
    {
        if ((((const PathDefinition *)((this->path_definition)))->flags & 0x11) != 0)
        {
            value = character_controller.movement.jump_ticks;
            flags = character_controller.flags;
            character_controller.motion.jump_phase = -1;
            character_controller.motion.vertical_speed = 0;
            character_controller.timers.state_ticks = 0;
            character_controller.timers.jump_duration_ticks = value;
            character_controller.flags = flags | 0x8000000;
            this->flags = this->flags | 0x400;
        }
        value = this->flags & 0x400;
        if (value == 0)
            return value;
    }
    animation = (sint8)character_controller.motion.state_code;
    if (animation == 6)
        return 6;
    if (animation == 3)
        return (uint32)animation - 33;
    if ((uint8)(animation - 33) < 2)
        return 15;
    if (animation == 15 || animation == 23)
        return 23;
    kind = (uint16)(character_controller.motion.form_flags >> 16) & 15;
    if ((kind == 0 || kind == 8) && animation == 9)
        return 9;
    flags = this->flags;
    if ((flags & 0x8000000) != 0 && ((uint16)(character_controller.motion.form_flags >> 16) & 15) == 4)
        return 4;
    if (((uint16)(character_controller.motion.form_flags >> 16) & 15) == 5 && (sint8)character_controller.motion.state_code == 7)
        return 7;
    if ((character_controller.secondary_flags & 0x400) != 0)
        return 0xF7FF0000;
    this->flags = flags & 0xF7FFFFFF;
    animation = (sint8)character_controller.motion.state_code;
    if (animation == 4)
        return 4;
    kind = (uint16)(character_controller.motion.form_flags >> 16) & 15;
    if ((kind == 5 || kind - 10 < 2) && (animation == 5 || animation == 9))
        return 9;
    if (((uint16)(character_controller.motion.form_flags >> 16) & 15) == 1)
    {
        value = this->flags & 0x4000000;
        if (value != 0)
            return value;
    }
    if ((character_controller.secondary_flags & 0x80) == 0)
        character_controller.motion.state_code = 6;
    return 6;
}
