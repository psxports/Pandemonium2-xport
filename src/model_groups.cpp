#include "game.h"
#include "original_defaults.h"

static const uint32 routed_spawn_offset = 0xFFFFA000;

void ModelGroupDefinition::InitializeSpawn(AnimatedObject *spawned, AnimatedObject *object, uint32 phase) const
{
    const PathTransform *captured_path_transform;
    FUNCTION_MARKER(0x8002B278, "SLUS_005.78");

    union
    {
        VECTOR value;
        uint32 words[4];
    } vector, world, inverse, input;

    union
    {
        uint32 words[5];
        uint16 halves[10];
    } matrix_local;

    uint32 flags, value, x, y, z, a, b, c, factor, index, kind, result, reverse;
    const PathDefinition *parent;
    const PathTransform *path_transform;
    uint32 scaled_x, scaled_z;
    sint32 state;
    const AnimatedObject *owner;
    if (spawned == 0)
        return;
    spawned->spheres[1].radius = 0xFFFFFFFF;
    spawned->spheres[0].radius = 0xFFFFFFFF;
    spawned->behavior = (sint8)(6);
    spawned->event_word = 0;
    spawned->state = object->state;
    spawned->path_definition = (object->path_definition);
    spawned->path_frame = object->path_frame;
    x = object->position[0];
    y = object->position[1];
    z = object->position[2];
    spawned->position[0] = x;
    spawned->position[1] = y;
    spawned->position[2] = z;
    flags = spawned->flags;
    value = object->flags;
    spawned->flags = flags | 0x20440 | phase | (value & 0x2010);
    if (object->behavior == 6)
    {
        flags = object->flags;
        if ((flags & 0x40000000) != 0)
        {
            if ((flags & 0x2000) == 0)
            {
                parent = (spawned->path_definition);
                if (parent != 0 && (((const PathDefinition *)((const PathDefinition *)(parent)))->transform) != 0)
                {
                    x = spawned->position[0];
                    y = spawned->position[1];
                    z = spawned->position[2];
                    vector.words[0] = x;
                    vector.words[1] = y;
                    vector.words[2] = z;
                    PushMatrix();
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
                    ApplyRotMatrixLV(&vector.value, (VECTOR *)spawned->position);
                    PopMatrix();
                    path_transform = (const PathTransform *)((((const PathDefinition *)((const PathDefinition *)((spawned->path_definition))))->transform) != 0 ? (((const PathDefinition *)((const PathDefinition *)((spawned->path_definition))))->transform) : (PathTransform *)0);
                    x = spawned->position[0];
                    value = (uint32)path_transform->matrix.t[0];
                    parent = (spawned->path_definition);
                    spawned->position[0] = x + value;
                    path_transform = (const PathTransform *)((((const PathDefinition *)((const PathDefinition *)(parent)))->transform) != 0 ? (((const PathDefinition *)((const PathDefinition *)(parent)))->transform) : (PathTransform *)0);
                    y = spawned->position[1];
                    value = (uint32)path_transform->matrix.t[1];
                    parent = (spawned->path_definition);
                    spawned->position[1] = y + value;
                    path_transform = (const PathTransform *)((((const PathDefinition *)((const PathDefinition *)(parent)))->transform) != 0 ? (((const PathDefinition *)((const PathDefinition *)(parent)))->transform) : (PathTransform *)0);
                    z = spawned->position[2];
                    value = (uint32)path_transform->matrix.t[2];
                    spawned->position[2] = z + value;
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
                spawned->flags = spawned->flags | 0x2000;
            }
            if (phase == 0)
            {
                spawned->flags = spawned->flags | 0x80000000;
                if (this->kind != 19)
                {
                    for (index = 0; index < 3; ++index)
                    {
                        x = spawned->position[index];
                        y = object->velocity[index];
                        spawned->position[index] = x + y;
                    }
                }
            }
        }
    }
    if (phase == 0x40000000)
    {
        if (object->behavior == 6)
        {
            flags = object->flags;
            if ((flags & 0x20000000) != 0)
            {
                if ((flags & 0x400) == 0)
                    spawned->flags = spawned->flags & 0xFFFFFBFF;
                if (object->velocity[0] + 26754 > 0xD103 || object->velocity[1] + 26754 > 0xD103 || object->velocity[2] + 26754 > 0xD103)
                {
                    for (;;)
                    {
                        x = object->velocity[0];
                        z = object->velocity[2];
                        object->velocity[0] = (uint32)((sint32)x >> 1);
                        y = object->velocity[1];
                        x = object->velocity[0];
                        z = (uint32)((sint32)z >> 1);
                        object->velocity[2] = z;
                        object->velocity[1] = (uint32)((sint32)y >> 1);
                        if (x + 26754 > 0xD103)
                            continue;
                        if (object->velocity[1] + 26754 > 0xD103 || z + 26754 > 0xD103)
                            continue;
                        break;
                    }
                }
                VectorNormal((VECTOR *)object->velocity, (VECTOR *)object->velocity);
                state = object->animation_binding.tracks[0];
                if (state == -1)
                {
                    owner = FindPlayerObject(0);
                    x = object->velocity[0];
                    factor = launch_speed;
                    spawned->velocity[0] = (uint32)(((sint64)(sint32)x * (sint32)factor) >> 12);
                    factor = launch_speed;
                    y = object->velocity[1];
                    spawned->velocity[1] = (uint32)(((sint64)(sint32)y * (sint32)factor) >> 12);
                    factor = launch_speed;
                    z = object->velocity[2];
                    spawned->velocity[2] = (uint32)(((sint64)(sint32)z * (sint32)factor) >> 12);
                    value = (uint16)(character_controller.motion.form_flags >> 16) & 15;
                    if (value - 5 >= 2 && value != 7 && value - 10 >= 2)
                    {
                        x = spawned->velocity[0];
                        y = owner->velocity[0];
                        spawned->velocity[0] = x + y;
                        x = spawned->velocity[2];
                        y = owner->velocity[2];
                        spawned->velocity[2] = x + y;
                    }
                    if ((object->flags & 0x2000) == 0)
                        spawned->velocity[1] = 0;
                }
                else
                {
                    for (index = 0; index < 3; ++index)
                    {
                        factor = launch_speed;
                        value = object->velocity[index];
                        spawned->velocity[index] = (uint32)(((sint64)(sint32)value * (sint32)factor) >> 12);
                    }
                }
                if ((object->flags & 0x20000000) != 0)
                {
                    object->velocity[2] = 0;
                    object->velocity[1] = 0;
                    object->velocity[0] = 0;
                }
                goto orient;
            }
        }
        goto place_projectile;
    }
    if (spawned->animation_binding.tracks[0] != -3)
    {
        if (phase == 0x20000000)
            goto place_projectile;
        goto copy_velocity;
    }
    if (phase == 0x20000000)
    {
        x = object->world_position[0];
        y = object->world_position[1];
        z = object->world_position[2];
        spawned->position[0] = x;
        spawned->position[1] = y;
        spawned->position[2] = z;
    }
    if ((this->flags & 2) == 0)
        goto copy_velocity;
    if (phase != 0x20000000)
        goto orient;
    owner = FindPlayerObject(0);
    if (owner == 0)
        goto orient;
    if ((owner->path_flags & 0x100) != 0)
    {
        x = camera_motion.focus_position[0];
        y = camera_motion.focus_position[1];
        z = camera_motion.focus_position[2];
        spawned->velocity[0] = x;
        spawned->velocity[1] = y;
        spawned->velocity[2] = z;
        goto aim_first;
    }
    if ((owner->flags & 0x2000) == 0 && (parent = (owner->path_definition)) != 0 && (((const PathDefinition *)((const PathDefinition *)(parent)))->transform) != 0)
    {
        x = owner->position[0];
        y = owner->position[1];
        z = owner->position[2];
        vector.words[0] = x;
        vector.words[1] = y;
        vector.words[2] = z;
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
        ApplyRotMatrixLV(&vector.value, (VECTOR *)spawned->velocity);
        PopMatrix();
        for (index = 0; index < 3; ++index)
        {
            path_transform = (const PathTransform *)((((const PathDefinition *)((const PathDefinition *)((owner->path_definition))))->transform) != 0 ? (((const PathDefinition *)((const PathDefinition *)((owner->path_definition))))->transform) : (PathTransform *)0);
            value = spawned->velocity[index];
            x = (uint32)path_transform->matrix.t[index];
            spawned->velocity[index] = value + x;
        }
    }
    else
    {
        x = owner->position[0];
        y = owner->position[1];
        z = owner->position[2];
        spawned->velocity[0] = x;
        spawned->velocity[1] = y;
        spawned->velocity[2] = z;
    }
aim_first:
    spawned->velocity[1] = spawned->velocity[1] + 12288;
    x = spawned->velocity[0];
    a = spawned->position[0];
    y = spawned->velocity[1];
    b = spawned->position[1];
    z = spawned->velocity[2];
    c = spawned->position[2];
    spawned->velocity[0] = x - a;
    spawned->velocity[1] = y - b;
    spawned->velocity[2] = z - c;
    goto orient;
copy_velocity:
    x = object->velocity[0];
    y = object->velocity[1];
    z = object->velocity[2];
    spawned->velocity[0] = x;
    spawned->velocity[1] = y;
    spawned->velocity[2] = z;
    goto orient;
place_projectile:
    if ((spawned->flags & 0x2000) == 0)
    {
        parent = (spawned->path_definition);
        if (parent != 0 && (((const PathDefinition *)((const PathDefinition *)(parent)))->transform) != 0)
        {
            x = spawned->position[0];
            y = spawned->position[1];
            z = spawned->position[2];
            world.words[0] = x;
            world.words[1] = y;
            world.words[2] = z;
            PushMatrix();
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
            ApplyRotMatrixLV(&world.value, (VECTOR *)spawned->position);
            PopMatrix();
            path_transform = (const PathTransform *)((((const PathDefinition *)((const PathDefinition *)((spawned->path_definition))))->transform) != 0 ? (((const PathDefinition *)((const PathDefinition *)((spawned->path_definition))))->transform) : (PathTransform *)0);
            x = spawned->position[0];
            value = (uint32)path_transform->matrix.t[0];
            parent = (spawned->path_definition);
            spawned->position[0] = x + value;
            path_transform = (const PathTransform *)((((const PathDefinition *)((const PathDefinition *)(parent)))->transform) != 0 ? (((const PathDefinition *)((const PathDefinition *)(parent)))->transform) : (PathTransform *)0);
            y = spawned->position[1];
            value = (uint32)path_transform->matrix.t[1];
            parent = (spawned->path_definition);
            spawned->position[1] = y + value;
            path_transform = (const PathTransform *)((((const PathDefinition *)((const PathDefinition *)(parent)))->transform) != 0 ? (((const PathDefinition *)((const PathDefinition *)(parent)))->transform) : (PathTransform *)0);
            z = spawned->position[2];
            value = (uint32)path_transform->matrix.t[2];
            spawned->position[2] = z + value;
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
        if (this->kind == 19 && (((const PathDefinition *)((const PathDefinition *)((spawned->path_definition))))->transform) != 0)
            spawned->flags = spawned->flags | 0x2000;
    }
    if ((this->flags & 2) != 0)
    {
        owner = FindPlayerObject(0);
        if (owner != object)
        {
            if ((owner->flags & 0x2000) == 0 && (parent = (owner->path_definition)) != 0 && (((const PathDefinition *)((const PathDefinition *)(parent)))->transform) != 0)
            {
                x = owner->position[0];
                y = owner->position[1];
                z = owner->position[2];
                world.words[0] = x;
                world.words[1] = y;
                world.words[2] = z;
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
                ApplyRotMatrixLV(&world.value, (VECTOR *)spawned->velocity);
                PopMatrix();
                for (index = 0; index < 3; ++index)
                {
                    path_transform = (const PathTransform *)((((const PathDefinition *)((const PathDefinition *)((owner->path_definition))))->transform) != 0 ? (((const PathDefinition *)((const PathDefinition *)((owner->path_definition))))->transform) : (PathTransform *)0);
                    value = spawned->velocity[index];
                    x = (uint32)path_transform->matrix.t[index];
                    spawned->velocity[index] = value + x;
                }
            }
            else
            {
                x = owner->position[0];
                y = owner->position[1];
                z = owner->position[2];
                spawned->velocity[0] = x;
                spawned->velocity[1] = y;
                spawned->velocity[2] = z;
            }
            spawned->velocity[1] = spawned->velocity[1] + 12288;
            x = spawned->velocity[0];
            a = spawned->position[0];
            y = spawned->velocity[1];
            b = spawned->position[1];
            z = spawned->velocity[2];
            c = spawned->position[2];
            spawned->velocity[0] = x - a;
            spawned->velocity[1] = y - b;
            spawned->velocity[2] = z - c;
        }
    }
    x = spawned->velocity[0];
    y = spawned->velocity[1];
    z = spawned->velocity[2];
    vector.words[0] = x;
    vector.words[1] = y;
    vector.words[2] = z;
    x = vector.words[0];
    vector.words[1] = 0;
    if (x + 26754 > 0xD103 || vector.words[2] + 26754 > 0xD103)
    {
        do
        {
            x = (uint32)((sint32)vector.words[0] >> 1);
            y = (uint32)((sint32)vector.words[1] >> 1);
            z = (uint32)((sint32)vector.words[2] >> 1);
            vector.words[0] = x;
            vector.words[1] = y;
            vector.words[2] = z;
        } while (x + 26754 > 0xD103 || y + 26754 > 0xD103 || z + 26754 > 0xD103);
    }
    VectorNormal(&vector.value, &vector.value);
    spawned->position[1] = spawned->position[1] + 20480;
    kind = this->kind;
    if (kind == 19)
    {
        if ((((const PathDefinition *)((object->path_definition)))->flags & 0x10000) != 0)
        {
            parent = (spawned->path_definition);
            if ((((const PathDefinition *)(parent))->flags & 0x10000) != 0)
            {
                if (parent != 0 && (((const PathDefinition *)((const PathDefinition *)(parent)))->transform) != 0)
                {
                    x = spawned->position[0];
                    y = spawned->position[1];
                    z = spawned->position[2];
                    input.words[0] = x;
                    input.words[1] = y;
                    input.words[2] = z;
                    PushMatrix();
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
                    ApplyRotMatrixLV(&input.value, &world.value);
                    PopMatrix();
                    for (index = 0; index < 3; ++index)
                    {
                        path_transform = (const PathTransform *)((((const PathDefinition *)((const PathDefinition *)((spawned->path_definition))))->transform) != 0 ? (((const PathDefinition *)((const PathDefinition *)((spawned->path_definition))))->transform) : (PathTransform *)0);
                        value = world.words[index];
                        x = (uint32)path_transform->matrix.t[index];
                        world.words[index] = value + x;
                    }
                }
                else
                {
                    x = spawned->position[0];
                    y = spawned->position[1];
                    z = spawned->position[2];
                    world.words[0] = x;
                    world.words[1] = y;
                    world.words[2] = z;
                }
                x = world.words[0];
                y = world.words[1];
                z = world.words[2];
                inverse.words[0] = x;
                inverse.words[1] = y;
                inverse.words[2] = z;
                spawned->ChangeActorPath(camera_motion.path, world.words, inverse.words);
            }
            kind = this->kind;
        }
    }
    if (kind == 19)
    {
        reverse = (((const PathDefinition *)((object->path_definition)))->flags & 0x10000) != 0;
        x = vector.words[0];
        scaled_x = (uint32)(((sint64)(sint32)x * 20480) >> 12);
        value = spawned->position[0];
        spawned->position[0] = reverse != 0 ? value - scaled_x : value + scaled_x;
        z = vector.words[2];
        scaled_z = (uint32)(((sint64)(sint32)z * 20480) >> 12);
        value = spawned->position[2];
        spawned->position[2] = reverse != 0 ? value - scaled_z : value + scaled_z;
    }
    else if (this->kind == 0 && path_routing.flags != 0 && (((const PathDefinition *)((object->path_definition)))->flags & 0x10000) != 0)
    {
        x = vector.words[0];
        factor = routed_spawn_offset;
        scaled_x = (uint32)(((sint64)(sint32)x * (sint32)factor) >> 12);
        value = spawned->position[0];
        factor = routed_spawn_offset;
        spawned->position[0] = value + scaled_x;
        z = vector.words[2];
        scaled_z = (uint32)(((sint64)(sint32)z * (sint32)factor) >> 12);
        spawned->position[2] = spawned->position[2] + scaled_z;
    }
    else
    {
        x = vector.words[0];
        value = spawned->position[0];
        spawned->position[0] = value;
        z = vector.words[2];
        spawned->position[2] = spawned->position[2];
    }
    flags = spawned->flags;
    if ((flags & 0x2000) == 0)
    {
        parent = (spawned->path_definition);
        result = ((const PathDefinition *)((const PathDefinition *)(parent)))->FindHeightFrame((const uint32 *)spawned->position, flags & 0x10, -40960);
        parent = (spawned->path_definition);
        spawned->path_frame = (sint16)(result);
        if (((const PathDefinition *)((const PathDefinition *)(parent)))->positions.keys == 0 || (sint16)result < 0)
            goto no_surface;
        flags = spawned->flags;
        parent = (spawned->path_definition);
        result = (uint32)((const PathDefinition *)((const PathDefinition *)(parent)))->ProjectFrame((sint16)result, (const uint32 *)spawned->position, vector.words, flags & 0x10, 4096);
        spawned->path_frame = (sint16)(result);
        if ((sint16)result < 0)
            goto no_surface;
        y = spawned->position[1];
        vector.words[1] = y;
        parent = (spawned->path_definition);
        if (parent != 0 && (path_transform = ((captured_path_transform = (((const PathDefinition *)((const PathDefinition *)(parent)))->transform)) != 0 ? (const PathTransform *)((PathTransform *)(captured_path_transform)) : 0)) != 0)
        {
            x = vector.words[0];
            value = (uint32)path_transform->matrix.t[0];
            inverse.words[0] = x - value;
            path_transform = (const PathTransform *)((((const PathDefinition *)((const PathDefinition *)((spawned->path_definition))))->transform) != 0 ? (((const PathDefinition *)((const PathDefinition *)((spawned->path_definition))))->transform) : (PathTransform *)0);
            value = (uint32)path_transform->matrix.t[1];
            inverse.words[1] = y - value;
            path_transform = (const PathTransform *)((((const PathDefinition *)((const PathDefinition *)((spawned->path_definition))))->transform) != 0 ? (((const PathDefinition *)((const PathDefinition *)((spawned->path_definition))))->transform) : (PathTransform *)0);
            z = vector.words[2];
            value = (uint32)path_transform->matrix.t[2];
            inverse.words[2] = z - value;
            for (index = 0; index < 9; ++index)
            {
                path_transform = (const PathTransform *)((((const PathDefinition *)((const PathDefinition *)((spawned->path_definition))))->transform) != 0 ? (((const PathDefinition *)((const PathDefinition *)((spawned->path_definition))))->transform) : (PathTransform *)0);
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
            ApplyRotMatrixLV(&inverse.value, (VECTOR *)spawned->position);
            PopMatrix();
        }
        else
        {
            x = vector.words[0];
            y = vector.words[1];
            z = vector.words[2];
            spawned->position[0] = x;
            spawned->position[1] = y;
            spawned->position[2] = z;
        }
    }
    goto orient;
no_surface:
    spawned->flags = spawned->flags | 0x2000;
orient:
    if (phase != 0)
    {
        value = spawned->velocity[0];
        spawned->orientation[0] = (sint16)(0x7FFF);
        if (value != 0 || spawned->velocity[1] != 0 || spawned->velocity[2] != 0)
        {
            value = (uint32)spawned->ComputeObjectPitch((const uint32 *)spawned->velocity, 0);
            spawned->orientation[1] = (sint16)((value - 1024) & 0xFFF);
            value = spawned->ComputeObjectFacing((const uint32 *)spawned->velocity, 0);
            spawned->orientation[2] = (sint16)((value + 2048) & 0xFFF);
        }
        else if (object->behavior == 6)
        {
            spawned->target_yaw = object->target_yaw;
            spawned->pitch_scale = object->pitch_scale;
            spawned->orientation[2] = (sint16)(((uint16)object->target_yaw + 2048) & 0xFFF);
            spawned->orientation[1] = (sint16)((uint16)object->pitch_scale & 0xFFF);
        }
        else
        {
            spawned->target_yaw = object->target_yaw;
            if (object->velocity[0] != 0 || object->velocity[1] != 0 || object->velocity[2] != 0)
                spawned->pitch_scale = (sint16)(object->ComputeObjectPitch((const uint32 *)object->velocity, 0));
            else
                spawned->pitch_scale = (sint16)(0);
            value = (uint16)spawned->pitch_scale;
            flags = (uint16)spawned->target_yaw;
            spawned->pitch_scale = (sint16)((value - 1024) & 0xFFF);
            value = (uint16)spawned->pitch_scale;
            spawned->orientation[2] = (sint16)((flags + 2048) & 0xFFF);
            spawned->orientation[1] = (sint16)(value & 0xFFF);
        }
    }
    if (phase == 0x40000000 || (phase == 0x80000000 && object->behavior != 6))
    {
        x = spawned->position[0];
        y = spawned->position[1];
        z = spawned->position[2];
        spawned->spheres[0].position[0] = x;
        spawned->spheres[0].position[1] = y;
        spawned->spheres[0].position[2] = z;
        x = spawned->spheres[0].position[0];
        y = spawned->spheres[0].position[1];
        z = spawned->spheres[0].position[2];
        spawned->world_position[0] = x;
        spawned->world_position[1] = y;
        spawned->world_position[2] = z;
        spawned->spheres[0].radius = 12288;
        spawned->metadata_words72[2] = 12288;
        spawned->metadata_words72[1] = 12288;
        spawned->metadata_words72[0] = 12288;
    }
    if (phase == 0x40000000 && (object->behavior != 6 || (object->flags & 0x20) != 0))
    {
        spawned->node.links.registry_next = object->node.links.registry_next;
        object->node.links.registry_next = &spawned->node;
        spawned->path_motion_scale = 0;
        spawned->spheres[2].position[2] = 0;
        spawned->spheres[2].position[0] = 0;
        spawned->spheres[1].position[2] = 0;
        spawned->spheres[1].position[0] = 0;
        spawned->sound_voice = 0xFFFF;
    }
    else
    {
        SceneSection *section = scene_stream.active_section;
        spawned->node.links.sibling_next = section->transient_links.first_child;
        section->transient_links.first_child = &spawned->node;
    }
}

AnimatedObject *UpdateModelGroup(AnimatedObject *animated_object)
{
    AnimationHeader *active_animation;
    const VectorAnimationTrack *vector_track;
    const VectorAnimationKey *vector_keys;
    FUNCTION_MARKER(0x8002C608, "SLUS_005.78");
    uint32 vector[3];
    uint32 attenuation, pan, ending_attenuation, ending_pan;
    ModelGroupDefinition *const *table;
    const ModelGroupDefinition *definition, *entry;
    uint32 count, kind, phase, spawn_kind, frame_action;
    AnimatedObject *candidate, *spawned;
    const ScenePath *spawn_path;
    uint32 flags, value, scene, x, y, z, a, b, c, squared, magnitude, factor, heading, pitch, radius;
    const SoundBankStorage *sound_table;
    const SoundSample *sound;
    sint32 next_frame, frame, end_frame, divisor, quotient, angle;
    count = model_groups.count;
    spawned = 0;
    next_frame = -1;
    table = model_groups.definitions;
    kind = animated_object->state;
    definition = 0;
    while (count != 0)
    {
        --count;
        entry = *table;
        if (entry != 0 && entry->kind == kind)
        {
            definition = entry;
            break;
        }
        ++table;
    }
    flags = animated_object->flags;
    if ((flags & 0x100) != 0)
    {
        if (animated_object->behavior == 6)
        {
            FindPlayerObject(0);
            scene = OriginalDefaults::SceneContextIdentity;
            for (kind = 0; kind < 3; ++kind)
            {
                x = camera_motion.focus_position[kind];
                y = animated_object->world_position[kind];
                value = x - y;
                vector[kind] = value;
                if (kind == 0)
                    a = value;
                else if (kind == 1)
                    b = value;
                else
                    c = value;
            }
            if ((definition->flags & 4) == 0)
            {
                x = (uint32)((sint32)a >> 6);
                y = (uint32)((sint32)b >> 6);
                z = (uint32)((sint32)c >> 6);
                squared = x * x + y * y + z * z;
                if ((sint32)squared > 163839999)
                    goto end_phase;
            }
        }
        candidate = definition->models[0];
        spawn_kind = 0;
        goto spawn_template;
    }
    if (animated_object->behavior != 6)
        goto apply_phase;
    if ((sint32)flags < 0)
    {
        frame_action = 0;
        goto calculate_frame;
    }
    if ((flags & 0x20000000) != 0)
    {
        frame_action = 1;
        goto calculate_frame;
    }
    if ((flags & 0x40000000) == 0)
        goto apply_phase;
    animated_object->ApplyModelGroupContactMotion();
    frame = animated_object->ticks;
    if (frame == 0)
        goto select_ending;
    if (frame > 0)
    {
        value = (uint32)frame - 1;
        animated_object->ticks = value;
        if ((uint16)value == 0)
        {
            if (definition->kind != 19)
                goto end_phase;
            if ((player_interaction.flags & 3) == 2)
            {
                player_interaction.flags = 0;
                player_interaction.target = 0;
                goto end_phase;
            }
            animated_object->ReleaseActorInteraction();
            goto apply_phase;
        }
    }
    // The looping sound is optional even when a Doppler radius is set
    if (definition->doppler_radius != 0 && definition->sound_table != 0 && (sint32)definition->sound_table->header.count >= 3)
    {
        sound_table = definition->sound_table;
        sound = 0;
        if (sound_table != 0 && (sint32)sound_table->header.count >= 3)
            sound = sound_table->samples + 2;
        pitch = ((const SoundSample *)(sound))->pitch;
        radius = definition->doppler_radius;
        frame = (sint16)animated_object->sound_voice;
        value = PlaySpatialSound(sound, 8192, 0, pitch, (const uint32 *)animated_object->world_position, radius, (uint32)frame, 1, 2);
        animated_object->sound_voice = value;
        if ((sint16)value >= 0)
        {
            scene = OriginalDefaults::SceneContextIdentity;
            x = animated_object->world_position[0];
            a = camera_motion.focus_position[0] - x;
            vector[0] = a;
            x = camera_motion.focus_position[1];
            y = animated_object->world_position[1];
            vector[1] = x - y;
            z = camera_motion.focus_position[2];
            c = z - animated_object->world_position[2];
            vector[2] = c;
            x = (uint32)((sint32)a >> 6);
            z = (uint32)((sint32)c >> 6);
            squared = x * x + z * z;
            vector[1] = 0;
            magnitude = (sint32)squared >= 0 ? SquareRoot12((sint32)squared) : SquareRoot12((sint32)(squared >> 2)) << 1;
            vector[1] = magnitude;
            if (magnitude != 0)
            {
                divisor = (sint32)definition->doppler_radius;
                if (divisor == 0)
                    xport_mips_break(7);
                quotient = 0x40000000 / divisor;
                value = (uint32)(((sint64)(sint32)magnitude * quotient) >> 18);
                z = animated_object->velocity[2];
                x = animated_object->velocity[0];
                fixed_math_result = value;
                heading = (uint32)Atan2Angle((sint32)z, (sint32)x);
                z = vector[2];
                x = vector[0];
                value = (uint32)Atan2Angle((sint32)z, (sint32)x);
                value = (heading - value) & 0xFFF;
                if (value >= 2049)
                    value = 4096 - value;
                angle = (sint16)(value - 1024);
                factor = fixed_math_result;
                factor = (uint32)(((sint64)angle * (sint32)factor) >> 12);
                factor = 4096 - (uint32)(sint32)(sint16)factor;
                value = (uint32)(((sint64)(sint32)(pitch << 12) * (sint32)factor) >> 12);
                SetSoundVoicePitch(animated_object->sound_voice, (uint32)((sint32)value >> 12));
            }
        }
    }
    if (definition->launch_speed == 0 && (definition->flags & 1) == 0)
    {
        frame_action = 2;
        goto calculate_frame;
    }
    value = (uint32)animated_object->ComputeObjectPitch((const uint32 *)animated_object->velocity, 0);
    animated_object->orientation[1] = (value - 1024) & 0xFFF;
    if ((definition->flags & 1) != 0)
    {
        value = animated_object->velocity[1];
        factor = definition->gravity;
        animated_object->velocity[1] = value - factor;
    }
    frame_action = 3;
calculate_frame:
    active_animation = animated_object->animation;
    next_frame = 0;
    if (active_animation != 0)
    {
        end_frame = active_animation->frame_count - 1;
        frame = animated_object->frame;
        if (frame < end_frame)
            next_frame = frame + 1;
    }
    if (frame_action == 0)
    {
        if (animated_object->frame >= next_frame)
        {
            animated_object->node.flags = animated_object->node.flags | 2;
            return 0;
        }
        goto apply_phase;
    }
    if (frame_action == 1)
    {
        animated_object->flags = animated_object->flags | 0x80000000;
        goto select_second;
    }
    if (frame_action == 2)
        goto apply_phase;
    divisor = definition->flags >> 2;
    if (divisor != 0)
    {
        frame = animated_object->ticks;
        end_frame = (sint16)definition->lifetime;
        if ((end_frame - (frame + 1)) % divisor != 0)
            goto apply_phase;
    }
    if (definition->kind == 19 && (player_interaction.flags & 4) != 0)
        goto apply_phase;
    candidate = definition->models[3];
    spawn_kind = 3;
    goto spawn_template;
select_second:
    candidate = definition->models[1];
    spawn_kind = 1;
    goto spawn_template;
select_ending:
    candidate = definition->models[2];
    spawn_kind = 2;
spawn_template:
    if (candidate != 0)
    {
        if ((candidate->node.flags & 0x80) != 0)
        {
            value = candidate->node.flags;
            candidate->node.links.sibling_next = 0;
            candidate->node.flags = value & 0x7F;
        }
        else
        {
            candidate = CloneObjectResource(candidate);
            if (candidate == 0)
                goto after_spawn;
            candidate->flags &= 0xFFFFFFFB;
        }
        flags = candidate->flags;
        candidate->spheres[0].radius = 0xFFFFFFFF;
        candidate->frame = (sint16)(0);
        candidate->movement_mode = (sint8)(0);
        candidate->velocity[2] = 0;
        candidate->velocity[1] = 0;
        candidate->velocity[0] = 0;
        candidate->flags = (flags & 0x1FFD97EF) | 0x20000;
    }
after_spawn:
    if (spawn_kind == 3)
    {
        definition->InitializeSpawn(candidate, animated_object, 0);
        goto apply_phase;
    }
    spawned = candidate;
    if (spawn_kind == 0)
    {
        phase = 0x20000000;
        if (spawned == 0)
            goto select_second;
        goto initialize_motion;
    }
    if (spawn_kind == 1)
    {
        if (spawned == 0)
        {
            if ((animated_object->flags & 0x20000000) != 0)
                goto end_phase;
            goto select_ending;
        }
        phase = 0x40000000;
        if ((animated_object->flags & 0x2000) == 0)
            spawned->movement_mode = (sint8)(2);
        scene = OriginalDefaults::SceneContextIdentity;
        spawn_path = camera_motion.path;
        spawned->metadata126[0] = 3;
        spawned->scene_path = spawn_path;
        value = (uint8)animated_object->animation_binding.tracks[0];
        spawned->animation_binding.tracks[1] = (sint8)((uint8)-3);
        spawned->animation_binding.tracks[0] = (sint8)(value);
        value = definition->lifetime;
        spawned->spheres[2].position[2] = 0;
        spawned->spheres[2].position[0] = 0;
        spawned->spheres[1].position[2] = 0;
        spawned->spheres[1].position[0] = 0;
        spawned->ticks = (sint16)(value - 1);
        if ((animated_object->flags & 0x20000000) != 0)
            goto apply_phase;
        goto initialize_motion;
    }
    if (spawned == 0)
        goto end_phase;
    phase = 0x80000000;
    if (definition->sound_table != 0)
    {
        if ((ComputeSpatialSoundControls((const uint32 *)animated_object->world_position, definition->sound_radius, &ending_attenuation, &ending_pan) & 0xFF) == 0)
            goto apply_phase;
        factor = (uint32)(sint32)(sint16)(sound_settings.effects_volume);
        value = ending_attenuation;
        value = (uint32)((sint32)(value * factor) >> 12);
        ending_attenuation = value;
        if (value == 0)
            goto apply_phase;
        sound_table = definition->sound_table;
        sound = 0;
        if (sound_table != 0 && (sint32)sound_table->header.count >= 2)
            sound = sound_table->samples + 1;
        value = (uint16)ending_attenuation;
        factor = (uint16)ending_pan;
        sound_effects.Queue(sound, 0, 0, value, factor, 2);
    }
    goto apply_phase;
initialize_motion:
    flags = animated_object->behavior == 6;
    if (flags == 0)
    {
        frame = (sint8)animated_object->metadata126[0];
        flags = frame == 3;
    }
    if (flags != 0)
    {
        flags = spawned->flags;
        spawned->animation_binding.tracks[0] = (sint8)((uint8)-3);
        spawned->flags = flags | 0x2000;
    }
    else
    {
        spawned->animation_binding.tracks[0] = (sint8)(0 - (uint32)frame);
        if (definition->launch_speed != 0 || (definition->flags & 1) != 0)
        {
            if ((sint8)animated_object->metadata126[0] == 1)
            {
                value = (uint16)(character_controller.motion.form_flags >> 16) & 15;
                if (value - 5 < 2 || value == 7 || value - 10 < 2)
                    spawned->flags = spawned->flags | 0x2000;
            }
            heading = ((uint16)animated_object->target_yaw + 2048) & 0xFFF;
            factor = definition->launch_speed;
            value = (uint32)SinQ14((sint16)heading);
            value = (uint32)(((sint64)(sint32)factor * ((sint32)value >> 2)) >> 12);
            spawned->velocity[0] = value;
            spawned->velocity[1] = 0;
            factor = definition->launch_speed;
            value = (uint32)SinQ14((sint16)(1024 - heading));
            spawned->velocity[2] = (uint32)(((sint64)(sint32)factor * ((sint32)value >> 2)) >> 12);
        }
        else
        {
            spawned->velocity[2] = 0;
            spawned->velocity[1] = 0;
            spawned->velocity[0] = 0;
        }
    }
    if (definition->sound_table != 0 && (ComputeSpatialSoundControls((const uint32 *)animated_object->world_position, definition->sound_radius, &attenuation, &pan) & 0xFF) != 0)
    {
        factor = (uint32)(sint32)(sint16)(sound_settings.effects_volume);
        value = attenuation;
        value = (uint32)((sint32)(value * factor) >> 12);
        attenuation = value;
        if (value != 0)
        {
            sound_table = definition->sound_table;
            sound = 0;
            if (sound_table != 0 && (sint32)sound_table->header.count > 0)
                sound = sound_table->samples;
            value = (uint16)attenuation;
            factor = (uint16)pan;
            sound_effects.Queue(sound, 0, 0, value, factor, 2);
        }
    }
apply_phase:
    if (spawned == 0)
    {
        if (next_frame != -1)
        {
            active_animation = animated_object->animation;
            animated_object->frame = next_frame;
            if (active_animation != 0)
                animated_object->ClampAnimationFrame();
        }
        goto animate_velocity;
    }
    definition->InitializeSpawn(spawned, animated_object, phase);
end_phase:
    if (animated_object->behavior != 6)
        goto finish;
    flags = animated_object->flags;
    if ((flags & 0x20000100) != 0)
    {
        animated_object->flags = flags & 0xDFFFFEFF;
        goto animate_velocity;
    }
    if (definition->kind == 19)
    {
        if (player_interaction.flags != 0)
        {
            animated_object->ReleaseActorInteraction();
            goto animate_velocity;
        }
    }
    else if ((flags & 0x40000020) == 0x40000020)
    {
        count = model_groups.count;
        table = model_groups.definitions;
        kind = animated_object->state;
        const ModelGroupDefinition *sound_definition = 0;
        while (count != 0)
        {
            --count;
            entry = *table;
            if (entry != 0 && entry->kind == kind)
            {
                sound_definition = entry;
                break;
            }
            ++table;
        }
        if (sound_definition->doppler_radius != 0)
        {
            sound_table = sound_definition->sound_table;
            sound = 0;
            if (sound_table != 0 && (sint32)sound_table->header.count >= 3)
                sound = sound_table->samples + 2;
            frame = (sint16)animated_object->sound_voice;
            if ((sound_effects.IsPlaying(sound, (uint32)frame) & 0xFF) != 0)
                StopSoundVoice(animated_object->sound_voice);
            animated_object->sound_voice = 0xFFFF;
        }
    }
    animated_object->node.flags = animated_object->node.flags | 2;
animate_velocity:
    if (animated_object->behavior == 6 && (animated_object->flags & 0x20) == 0)
    {
        uint32 *event_slots[1] = {&animated_object->event_word};
        if ((ApplySceneEvents(event_slots, 1) & 0xFF) != 0)
            animated_object->flags = animated_object->flags | 0x100;
        active_animation = animated_object->animation;
        frame = animated_object->animation_binding.tracks[0];
        vector_track = active_animation->tracks[0].vectors;
        vector_track = ((const VectorAnimationTrack *)(vector_track)) + frame;
        vector_keys = (const VectorAnimationKey *)vector_track->track.keys;
        for (kind = 0; kind < 3; ++kind)
        {
            x = vector_keys[1].position[kind];
            y = vector_keys[0].position[kind];
            vector[kind] = x - y;
        }
        PushMatrix();
        const MATRIX *parent_matrix = (render_view.matrix_cursor->prev != 0 ? &render_view.matrix_cursor->prev->matrix : 0);
        x = xport_load_le32((const uint8 *)parent_matrix);
        y = xport_load_le32((const uint8 *)parent_matrix + 4);
        xport_gte_write_control(0, x);
        xport_gte_write_control(1, y);
        x = xport_load_le32((const uint8 *)parent_matrix + 8);
        y = xport_load_le32((const uint8 *)parent_matrix + 12);
        z = xport_load_le32((const uint8 *)parent_matrix + 16);
        xport_gte_write_control(2, x);
        xport_gte_write_control(3, y);
        xport_gte_write_control(4, z);
        ApplyRotMatrixLV((const VECTOR *)vector, (VECTOR *)animated_object->velocity);
        PopMatrix();
    }
finish:
    return spawned;
}

uint32 AnimatedObject::StopModelGroupSound()
{
    FUNCTION_MARKER(0x8002C510, "SLUS_005.78");
    uint32 result = 6;
    if (this->behavior == 6)
    {
        result = this->flags & 0x40000020;
        if (result == 0x40000020)
        {
            uint32 remaining = model_groups.count;
            ModelGroupDefinition *const *slot = model_groups.definitions;
            uint32 kind = this->state;
            const ModelGroupDefinition *entry = 0;
            while (remaining != 0)
            {
                const ModelGroupDefinition *candidate = *slot;
                --remaining;
                if (candidate != 0 && candidate->kind == kind)
                {
                    entry = candidate;
                    break;
                }
                ++slot;
            }
            result = entry->doppler_radius;
            if (result != 0)
            {
                const SoundSample *sound = 0;
                const SoundBankStorage *sounds = entry->sound_table;
                if (sounds != 0 && (sint32)sounds->header.count >= 3)
                    sound = sounds->samples + 2;
                if ((uint8)sound_effects.IsPlaying(sound, (uint32)(sint32)(sint16)this->sound_voice) != 0)
                    StopSoundVoice(this->sound_voice);
                result = (uint32)-1;
                this->sound_voice = result;
            }
        }
    }
    return result;
}

uint32 GetModelGroupLifetime(uint32 kind)
{
    FUNCTION_MARKER(0x8002D7A4, "SLUS_005.78");
    ModelGroupDefinition *const *table = model_groups.definitions;
    const ModelGroupDefinition *found = 0;
    uint32 count = model_groups.count;
    while (count != 0)
    {
        const ModelGroupDefinition *candidate = *table;
        --count;
        if (candidate != 0 && candidate->kind == kind)
        {
            found = candidate;
            break;
        }
        ++table;
    }
    return (uint32)(sint32)(sint16)found->lifetime;
}

uint32 GetModelGroupContactType(uint32 kind)
{
    FUNCTION_MARKER(0x8002D750, "SLUS_005.78");
    uint32 remaining = model_groups.count;
    ModelGroupDefinition *const *slot = model_groups.definitions;
    const ModelGroupDefinition *found = 0;
    while (remaining != 0)
    {
        const ModelGroupDefinition *definition = *slot;
        --remaining;
        if (definition != 0 && definition->kind == kind)
        {
            found = definition;
            break;
        }
        ++slot;
    }
    return found->contact_type;
}

uint32 AnimatedObject::GetModelGroupLaunchSpeed() const
{
    FUNCTION_MARKER(0x8002B220, "SLUS_005.78");
    uint32 count = model_groups.count;
    uint32 type = this->state;
    ModelGroupDefinition *const *slot = model_groups.definitions;
    while (count != 0)
    {
        const ModelGroupDefinition *object = *slot;
        --count;
        if (object != 0 && object->kind == type)
            return object->launch_speed;
        ++slot;
    }
    /* Return no match instead of the original read through address zero */
    return 0;
}

ModelGroupDefinition *FindRelatedModelGroup(uint32 kind)
{
    FUNCTION_MARKER(0x8002D51C, "SLUS_005.78");
    ModelGroupDefinition *const *slot;
    uint32 type, count;
    switch (kind)
    {
        case 15:
            type = 23;
            break;
        case 8:
            type = 28;
            break;
        case 12:
            type = 26;
            break;
        case 5:
            type = 30;
            break;
        case 31:
            type = 42;
            break;
        case 44:
            type = 4;
            break;
        default:
            return 0;
    }
    count = model_groups.count;
    slot = model_groups.definitions;
    while (count != 0)
    {
        ModelGroupDefinition *object = *slot;
        --count;
        if (object != 0 && object->kind == type)
            return object;
        ++slot;
    }
    return 0;
}

ModelGroupDefinition *SetRelatedModelGroupContactType(uint32 kind, uint8 value)
{
    FUNCTION_MARKER(0x8002D71C, "SLUS_005.78");
    ModelGroupDefinition *result = FindRelatedModelGroup(kind);
    if (result != 0)
        result->contact_type = value;
    return result;
}
