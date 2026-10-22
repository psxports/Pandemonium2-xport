#include "model_group_definition.h"
#include "game.h"

static const uint32 routed_spawn_offset = 0xFFFFA000;

void ModelGroupDefinition::InitializeSpawn(AnimatedObject *spawned, AnimatedObject *object, uint32 phase) const
{
    FUNCTION_MARKER(0x8002B278, "SLUS_005.78");

    Vec3 vector, world, inverse, input;

    MATRIX matrix_local = {0};

    uint32 cur_flags;
    uint32 val;
    uint32 x;
    uint32 y;
    uint32 z;
    uint32 a;
    uint32 b;
    uint32 c;
    sint32 factor;
    uint32 idx;
    uint32 kind_val;
    uint32 result;
    uint32 reverse;
    const PathDefinition *parent;
    PathTransform *path_transform;
    uint32 scaled_x, scaled_z;
    sint32 state;
    const AnimatedObject *owner;
    if (spawned == 0)
        return;
    spawned->spheres[1].radius = 0xFFFFFFFF;
    spawned->spheres[0].radius = 0xFFFFFFFF;
    spawned->behavior = 6;
    spawned->event_word = 0;
    spawned->state = object->state;
    spawned->path_definition = object->path_definition;
    spawned->path_frame = object->path_frame;

    spawned->pos = object->pos;
    cur_flags = spawned->flags;
    val = object->flags;
    spawned->flags = cur_flags | 0x20440 | phase | (val & 0x2010);
    if (object->behavior == 6)
    {
        cur_flags = object->flags;
        if ((cur_flags & 0x40000000) != 0)
        {
            if ((cur_flags & 0x2000) == 0)
            {
                parent = spawned->path_definition;
                if (parent != 0 && parent->transform != 0)
                {
                    vector = spawned->pos;
                    PushMatrix();
                    path_transform = spawned->path_definition->transform;
                    SetRotMatrix(&path_transform->matrix);
                    {
                        VECTOR rotation_output;
                        {
                            VECTOR sdk_input = {vector.x, vector.y, vector.z, 0};
                            ApplyRotMatrixLV(&sdk_input, &rotation_output);
                        }
                        spawned->pos.x = rotation_output.vx;
                        spawned->pos.y = rotation_output.vy;
                        spawned->pos.z = rotation_output.vz;
                    }
                    PopMatrix();
                    path_transform = spawned->path_definition->transform;
                    x = spawned->pos.x;
                    val = path_transform->matrix.t[0];
                    parent = spawned->path_definition;
                    spawned->pos.x = x + val;
                    path_transform = parent->transform;
                    y = spawned->pos.y;
                    val = path_transform->matrix.t[1];
                    parent = spawned->path_definition;
                    spawned->pos.y = y + val;
                    path_transform = parent->transform;
                    z = spawned->pos.z;
                    val = path_transform->matrix.t[2];
                    spawned->pos.z = z + val;
                }
                spawned->flags = spawned->flags | 0x2000;
            }
            if (phase == 0)
            {
                spawned->flags = spawned->flags | 0x80000000;
                if (kind != 19)
                {
                    x = spawned->pos.x;
                    y = object->velocity.x;
                    spawned->pos.x = x + y;
                    x = spawned->pos.y;
                    y = object->velocity.y;
                    spawned->pos.y = x + y;
                    x = spawned->pos.z;
                    y = object->velocity.z;
                    spawned->pos.z = x + y;
                }
            }
        }
    }
    if (phase == 0x40000000)
    {
        if (object->behavior == 6)
        {
            cur_flags = object->flags;
            if ((cur_flags & 0x20000000) != 0)
            {
                if ((cur_flags & 0x400) == 0)
                    spawned->flags = spawned->flags & 0xFFFFFBFF;
                if ((uint32)object->velocity.x + 26754 > 0xD103 || (uint32)object->velocity.y + 26754 > 0xD103 || (uint32)object->velocity.z + 26754 > 0xD103)
                {
                    for (;;)
                    {
                        x = object->velocity.x;
                        z = object->velocity.z;
                        object->velocity.x = (sint32)x >> 1;
                        y = object->velocity.y;
                        x = object->velocity.x;
                        z = (sint32)z >> 1;
                        object->velocity.z = z;
                        object->velocity.y = (sint32)y >> 1;
                        if (x + 26754 > 0xD103)
                            continue;
                        if ((uint32)object->velocity.y + 26754 > 0xD103 || z + 26754 > 0xD103)
                            continue;
                        break;
                    }
                }
                {
                    VECTOR n_input = {object->velocity.x, object->velocity.y, object->velocity.z, 0};
                    VECTOR n_output;
                    VectorNormal(&n_input, &n_output);
                    object->velocity.x = n_output.vx;
                    object->velocity.y = n_output.vy;
                    object->velocity.z = n_output.vz;
                }
                state = object->animation_binding.tracks[0];
                if (state == -1)
                {
                    owner = CharacterController::FindPlayerObject(0);
                    x = object->velocity.x;
                    factor = launch_speed;
                    spawned->velocity.x = mul12((sint32)x, factor);
                    factor = launch_speed;
                    y = object->velocity.y;
                    spawned->velocity.y = mul12((sint32)y, factor);
                    factor = launch_speed;
                    z = object->velocity.z;
                    spawned->velocity.z = mul12((sint32)z, factor);
                    val = (uint16)(character_pad.motion.form_flags >> 16) & 15;
                    if (val - 5 >= 2 && val != 7 && val - 10 >= 2)
                    {
                        x = spawned->velocity.x;
                        y = owner->velocity.x;
                        spawned->velocity.x = x + y;
                        x = spawned->velocity.z;
                        y = owner->velocity.z;
                        spawned->velocity.z = x + y;
                    }
                    if ((object->flags & 0x2000) == 0)
                        spawned->velocity.y = 0;
                }
                else
                {
                    factor = launch_speed;
                    spawned->velocity.x = mul12(object->velocity.x, factor);
                    factor = launch_speed;
                    spawned->velocity.y = mul12(object->velocity.y, factor);
                    factor = launch_speed;
                    spawned->velocity.z = mul12(object->velocity.z, factor);
                }
                if ((object->flags & 0x20000000) != 0)
                {
                    object->velocity.z = 0;
                    object->velocity.y = 0;
                    object->velocity.x = 0;
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
        spawned->pos = object->world_pos;
    }
    if ((flags & 2) == 0)
        goto copy_velocity;
    if (phase != 0x20000000)
        goto orient;
    owner = CharacterController::FindPlayerObject(0);
    if (owner == 0)
        goto orient;
    if ((owner->path_flags & 0x100) != 0)
    {
        spawned->velocity = camera_motion.focus_pos;
        goto aim_first;
    }
    if ((owner->flags & 0x2000) == 0 && (parent = owner->path_definition) != 0 && parent->transform != 0)
    {
        vector = owner->pos;
        PushMatrix();
        path_transform = owner->path_definition->transform;
        SetRotMatrix(&path_transform->matrix);
        {
            VECTOR rotation_output;
            {
                VECTOR sdk_input = {vector.x, vector.y, vector.z, 0};
                ApplyRotMatrixLV(&sdk_input, &rotation_output);
            }
            spawned->velocity.x = rotation_output.vx;
            spawned->velocity.y = rotation_output.vy;
            spawned->velocity.z = rotation_output.vz;
        }
        PopMatrix();
        path_transform = owner->path_definition->transform;
        val = spawned->velocity.x;
        x = path_transform->matrix.t[0];
        spawned->velocity.x = val + x;
        path_transform = owner->path_definition->transform;
        val = spawned->velocity.y;
        x = path_transform->matrix.t[1];
        spawned->velocity.y = val + x;
        path_transform = owner->path_definition->transform;
        val = spawned->velocity.z;
        x = path_transform->matrix.t[2];
        spawned->velocity.z = val + x;
    }
    else
    {
        spawned->velocity = owner->pos;
    }
aim_first:
    spawned->velocity.y = (uint32)spawned->velocity.y + 12288;
    x = spawned->velocity.x;
    a = spawned->pos.x;
    y = spawned->velocity.y;
    b = spawned->pos.y;
    z = spawned->velocity.z;
    c = spawned->pos.z;
    spawned->velocity.x = x - a;
    spawned->velocity.y = y - b;
    spawned->velocity.z = z - c;
    goto orient;
copy_velocity:
    x = (uint32)object->velocity.x;
    y = object->velocity.y;
    z = object->velocity.z;
    spawned->velocity.x = x;
    spawned->velocity.y = y;
    spawned->velocity.z = z;
    goto orient;
place_projectile:
    if ((spawned->flags & 0x2000) == 0)
    {
        parent = spawned->path_definition;
        if (parent != 0 && parent->transform != 0)
        {
            world = spawned->pos;
            PushMatrix();
            path_transform = spawned->path_definition->transform;
            SetRotMatrix(&path_transform->matrix);
            {
                VECTOR rotation_output;
                {
                    VECTOR sdk_input = {world.x, world.y, world.z, 0};
                    ApplyRotMatrixLV(&sdk_input, &rotation_output);
                }
                spawned->pos.x = rotation_output.vx;
                spawned->pos.y = rotation_output.vy;
                spawned->pos.z = rotation_output.vz;
            }
            PopMatrix();
            path_transform = spawned->path_definition->transform;
            x = spawned->pos.x;
            val = path_transform->matrix.t[0];
            parent = spawned->path_definition;
            spawned->pos.x = x + val;
            path_transform = parent->transform;
            y = spawned->pos.y;
            val = path_transform->matrix.t[1];
            parent = spawned->path_definition;
            spawned->pos.y = y + val;
            path_transform = parent->transform;
            z = spawned->pos.z;
            val = path_transform->matrix.t[2];
            spawned->pos.z = z + val;
        }
        if (kind == 19 && spawned->path_definition->transform != 0)
            spawned->flags = spawned->flags | 0x2000;
    }
    if ((flags & 2) != 0)
    {
        owner = CharacterController::FindPlayerObject(0);
        if (owner != object)
        {
            if ((owner->flags & 0x2000) == 0 && (parent = owner->path_definition) != 0 && parent->transform != 0)
            {
                world = owner->pos;
                PushMatrix();
                path_transform = owner->path_definition->transform;
                SetRotMatrix(&path_transform->matrix);
                {
                    VECTOR rotation_output;
                    {
                        VECTOR sdk_input = {world.x, world.y, world.z, 0};
                        ApplyRotMatrixLV(&sdk_input, &rotation_output);
                    }
                    spawned->velocity.x = rotation_output.vx;
                    spawned->velocity.y = rotation_output.vy;
                    spawned->velocity.z = rotation_output.vz;
                }
                PopMatrix();
                path_transform = owner->path_definition->transform;
                val = spawned->velocity.x;
                x = path_transform->matrix.t[0];
                spawned->velocity.x = val + x;
                path_transform = owner->path_definition->transform;
                val = spawned->velocity.y;
                x = path_transform->matrix.t[1];
                spawned->velocity.y = val + x;
                path_transform = owner->path_definition->transform;
                val = spawned->velocity.z;
                x = path_transform->matrix.t[2];
                spawned->velocity.z = val + x;
            }
            else
            {
                spawned->velocity = owner->pos;
            }
            spawned->velocity.y = (uint32)spawned->velocity.y + 12288;
            x = spawned->velocity.x;
            a = spawned->pos.x;
            y = spawned->velocity.y;
            b = spawned->pos.y;
            z = spawned->velocity.z;
            c = spawned->pos.z;
            spawned->velocity.x = x - a;
            spawned->velocity.y = y - b;
            spawned->velocity.z = z - c;
        }
    }

    vector = spawned->velocity;
    x = vector.x;
    vector.y = 0;
    if (x + 26754 > 0xD103 || (uint32)vector.z + 26754 > 0xD103)
    {
        do
        {
            x = vector.x >> 1;
            y = vector.y >> 1;
            z = vector.z >> 1;
            vector.x = x;
            vector.y = y;
            vector.z = z;
        } while (x + 26754 > 0xD103 || y + 26754 > 0xD103 || z + 26754 > 0xD103);
    }
    {
        VECTOR sdk_input = {vector.x, vector.y, vector.z, 0};
        VECTOR sdk_output;
        VectorNormal(&sdk_input, &sdk_output);
        vector.x = sdk_output.vx;
        vector.y = sdk_output.vy;
        vector.z = sdk_output.vz;
    }
    spawned->pos.y = (uint32)spawned->pos.y + 20480;
    kind_val = kind;
    if (kind_val == 19)
    {
        if ((object->path_definition->flags & 0x10000) != 0)
        {
            parent = spawned->path_definition;
            if ((parent->flags & 0x10000) != 0)
            {
                if (parent != 0 && parent->transform != 0)
                {
                    input = spawned->pos;
                    PushMatrix();
                    path_transform = spawned->path_definition->transform;
                    SetRotMatrix(&path_transform->matrix);
                    {
                        VECTOR sdk_input = {input.x, input.y, input.z, 0};
                        VECTOR sdk_output;
                        ApplyRotMatrixLV(&sdk_input, &sdk_output);
                        world.x = sdk_output.vx;
                        world.y = sdk_output.vy;
                        world.z = sdk_output.vz;
                    }
                    PopMatrix();
                    path_transform = spawned->path_definition->transform;
                    val = world.x;
                    x = path_transform->matrix.t[0];
                    world.x = val + x;
                    path_transform = spawned->path_definition->transform;
                    val = world.y;
                    x = path_transform->matrix.t[1];
                    world.y = val + x;
                    path_transform = spawned->path_definition->transform;
                    val = world.z;
                    x = path_transform->matrix.t[2];
                    world.z = val + x;
                }
                else
                {
                    world = spawned->pos;
                }

                inverse = world;
                spawned->ChangeActorPath(camera_motion.path, world, inverse);
            }
            kind_val = kind;
        }
    }
    if (kind_val == 19)
    {
        reverse = (object->path_definition->flags & 0x10000) != 0;
        x = vector.x;
        scaled_x = mul12((sint32)x, 20480);
        val = spawned->pos.x;
        spawned->pos.x = reverse != 0 ? val - scaled_x : val + scaled_x;
        z = vector.z;
        scaled_z = mul12((sint32)z, 20480);
        val = spawned->pos.z;
        spawned->pos.z = reverse != 0 ? val - scaled_z : val + scaled_z;
    }
    else if (kind == 0 && path_routing.flags != 0 && (object->path_definition->flags & 0x10000) != 0)
    {
        x = vector.x;
        factor = routed_spawn_offset;
        scaled_x = mul12((sint32)x, factor);
        val = spawned->pos.x;
        factor = routed_spawn_offset;
        spawned->pos.x = val + scaled_x;
        z = vector.z;
        scaled_z = mul12((sint32)z, factor);
        spawned->pos.z = spawned->pos.z + scaled_z;
    }
    else
    {
        val = spawned->pos.x;
        spawned->pos.x = val;

        spawned->pos.z = spawned->pos.z;
    }
    cur_flags = spawned->flags;
    if ((cur_flags & 0x2000) == 0)
    {
        parent = spawned->path_definition;
        result = parent->FindHeightFrame(spawned->pos, cur_flags & 0x10, -40960);
        parent = spawned->path_definition;
        spawned->path_frame = result;
        if (parent->pos_track.keys == 0 || (sint16)result < 0)
            goto no_surface;
        cur_flags = spawned->flags;
        parent = spawned->path_definition;
        result = parent->ProjectFrame((sint16)result, spawned->pos, vector, cur_flags & 0x10, 4096);
        spawned->path_frame = result;
        if ((sint16)result < 0)
            goto no_surface;
        y = spawned->pos.y;
        vector.y = y;
        parent = spawned->path_definition;
        if (parent != 0 && (path_transform = parent->transform) != 0)
        {
            x = vector.x;
            val = path_transform->matrix.t[0];
            inverse.x = x - val;
            path_transform = spawned->path_definition->transform;
            val = path_transform->matrix.t[1];
            inverse.y = y - val;
            path_transform = spawned->path_definition->transform;
            z = vector.z;
            val = path_transform->matrix.t[2];
            inverse.z = z - val;
            for (idx = 0; idx < 9; ++idx)
            {
                path_transform = spawned->path_definition->transform;
                val = (uint16)path_transform->matrix.m[idx % 3][idx / 3];
                matrix_local.m[idx / 3][idx % 3] = val;
            }
            PushMatrix();
            SetRotMatrix(&matrix_local);
            {
                VECTOR rotation_output;
                {
                    VECTOR sdk_input = {inverse.x, inverse.y, inverse.z, 0};
                    ApplyRotMatrixLV(&sdk_input, &rotation_output);
                }
                spawned->pos.x = rotation_output.vx;
                spawned->pos.y = rotation_output.vy;
                spawned->pos.z = rotation_output.vz;
            }
            PopMatrix();
        }
        else
        {
            spawned->pos = vector;
        }
    }
    goto orient;
no_surface:
    spawned->flags = spawned->flags | 0x2000;
orient:
    if (phase != 0)
    {
        val = spawned->velocity.x;
        spawned->orientation[0] = 0x7FFF;
        if (val != 0 || spawned->velocity.y != 0 || spawned->velocity.z != 0)
        {
            val = spawned->ComputeObjectPitch(spawned->velocity, 0);
            spawned->orientation[1] = (val - 1024) & 0xFFF;
            val = spawned->ComputeObjectFacing(spawned->velocity, 0);
            spawned->orientation[2] = (val + 2048) & 0xFFF;
        }
        else if (object->behavior == 6)
        {
            spawned->target_yaw = object->target_yaw;
            spawned->pitch_scale = object->pitch_scale;
            spawned->orientation[2] = ((uint16)object->target_yaw + 2048) & 0xFFF;
            spawned->orientation[1] = (uint16)object->pitch_scale & 0xFFF;
        }
        else
        {
            spawned->target_yaw = object->target_yaw;
            if (object->velocity.x != 0 || object->velocity.y != 0 || object->velocity.z != 0)
                spawned->pitch_scale = object->ComputeObjectPitch(object->velocity, 0);
            else
                spawned->pitch_scale = 0;
            val = (uint16)spawned->pitch_scale;
            cur_flags = (uint16)spawned->target_yaw;
            spawned->pitch_scale = (val - 1024) & 0xFFF;
            val = (uint16)spawned->pitch_scale;
            spawned->orientation[2] = (cur_flags + 2048) & 0xFFF;
            spawned->orientation[1] = val & 0xFFF;
        }
    }
    if (phase == 0x40000000 || (phase == 0x80000000 && object->behavior != 6))
    {
        spawned->spheres[0].pos = spawned->pos;

        spawned->world_pos = spawned->spheres[0].pos;
        spawned->spheres[0].radius = 12288;
        spawned->bounds.depth = 12288;
        spawned->bounds.height = 12288;
        spawned->bounds.radius = 12288;
    }
    if (phase == 0x40000000 && (object->behavior != 6 || (object->flags & 0x20) != 0))
    {
        spawned->node.links.registry_next = object->node.links.registry_next;
        object->node.links.registry_next = &spawned->node;
        spawned->path_motion_scale = 0;
        spawned->spheres[2].pos.z = 0;
        spawned->spheres[2].pos.x = 0;
        spawned->spheres[1].pos.z = 0;
        spawned->spheres[1].pos.x = 0;
        spawned->sound_voice = 0xFFFF;
    }
    else
    {
        SceneSection *section = scene_stream.active_section;
        spawned->node.links.sibling_next = section->transient_links.first_child;
        section->transient_links.first_child = &spawned->node;
    }
}
