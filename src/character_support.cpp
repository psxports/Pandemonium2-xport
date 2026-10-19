#include "game.h"
#include "original_defaults.h"

static const uint32 support_search_limit = 20;

union SupportVector
{
    VECTOR vector;
    uint32 words[4];
};

uint32 AnimatedObject::UpdateCharacterSupportMotion()
{
    FUNCTION_MARKER(0x8001D698, "SLUS_005.78");
    uint32 velocity[3];
    const SceneNode *other;
    const MeshGeometryNode *other_mesh;
    uint32 packed;
    CollisionTransform collision = {0};
    uint32 fraction_limit;
    uint32 result;
    uint32 index;
    this->ApplyCharacterPathMotion((sint32)character_controller.support.speed[0], (sint32)character_controller.support.speed[1]);
    velocity[0] = this->velocity[0];
    velocity[1] = this->velocity[1];
    velocity[2] = this->velocity[2];
    this->velocity[0] = 0;
    this->velocity[1] = (uint32)-40960;
    this->velocity[2] = 0;
    other = character_controller.support.owner;
    other_mesh = (const MeshGeometryNode *)other;
    fraction_limit = 4096;
    {
        uint32 kind = other_mesh->node.kind;
        packed = character_controller.support.contact_word;
        if (kind == 1)
        {
            uint32 first = psx_identity_rotation_words[0];
            uint32 second = psx_identity_rotation_words[1];
            uint32 third = psx_identity_rotation_words[2];

            collision.words[1] = first;
            collision.words[2] = second;
            collision.words[3] = third;
            collision.words[4] = psx_identity_rotation_words[3];
            collision.fields.rotation[8] = psx_identity_matrix.m[2][2];
            const MeshGeometryNode *position_source = other_mesh->vertex_count > 0 ? other_mesh : (((MeshGeometryNode *)(other_mesh->source_mesh)));
            first = position_source->position[0];
            second = position_source->position[1];
            third = position_source->position[2];
            collision.words[6] = first;
            collision.words[7] = second;
            collision.words[8] = third;
            collision.words[11] = 0;
            collision.words[10] = 0;
            collision.words[9] = 0;
        }
        else
        {
            const CollisionTransform *cached_transform = ((const AnimatedObject *)other_mesh)->collision_transforms;
            cached_transform = ((const CollisionTransform *)(cached_transform)) + (packed >> 16);
            uint32 offset;
            for (offset = 0; offset < 48; offset += 16)
            {
                uint32 first = cached_transform->words[(offset >> 2)];
                uint32 second = cached_transform->words[(offset >> 2) + 1];
                uint32 third = cached_transform->words[(offset >> 2) + 2];
                uint32 fourth = cached_transform->words[(offset >> 2) + 3];
                collision.words[(offset >> 2)] = first;
                collision.words[(offset >> 2) + 1] = second;
                collision.words[(offset >> 2) + 2] = third;
                collision.words[(offset >> 2) + 3] = fourth;
            }
        }
    }
    collision.fields.face_count = 1;
    collision.fields.first_face = packed;
    result = (uint32)this->FindSphereFaceCollision(character_geometry.world, character_controller.support.owner, &collision, &fraction_limit);
    this->velocity[0] = velocity[0];
    this->velocity[1] = velocity[1];
    this->velocity[2] = velocity[2];
    if ((sint32)result < 0)
    {
        uint32 horizontal = character_controller.support.speed[0];
        uint32 vertical = character_controller.support.speed[1];
        character_controller.motion.horizontal_speed = horizontal;
        character_controller.motion.vertical_speed = vertical;
        if ((sint32)vertical > 0)
        {
            vertical = character_controller.motion.vertical_speed;
            character_controller.motion.horizontal_speed = 0;
            character_controller.motion.vertical_speed = 0u - vertical;
        }
        result = character_controller.flags & ~0x10000u;
        character_controller.flags = result;
    }
    else
    {
        fraction_limit = 4096;
        result = (uint32)this->FindSphereFaceCollision(character_geometry.world, character_controller.support.owner, &collision, &fraction_limit);
        if ((sint32)result >= 0)
        {
            const PathDefinition *definition;
            if ((this->flags & 0x2000) == 0 && (definition = (this->path_definition)) != 0 && (((const PathDefinition *)(definition))->transform) != 0)
            {
                for (index = 0; index < 3; ++index)
                {
                    const PathTransform *path_transform = (const PathTransform *)((((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) != 0 ? (((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) : (PathTransform *)0);
                    uint32 value = collision.fields.motion[index];
                    value -= path_transform->displacement[index];
                    collision.fields.motion[index] = value;
                }
            }
            if (collision.fields.motion[0] != 0 || collision.fields.motion[1] != 0 || collision.fields.motion[2] != 0)
            {
                uint32 horizontal = this->velocity[0];
                uint32 brake = 0;
                sint32 factor;
                if (horizontal != 0 || this->velocity[2] != 0)
                {
                    sint16 motion_angle = (sint16)Atan2Angle((sint32)this->velocity[2], (sint32)horizontal);
                    sint16 contact_angle = (sint16)Atan2Angle((sint32)collision.fields.motion[2], (sint32)collision.fields.motion[0]);
                    brake = (((uint32)(sint32)motion_angle - (uint32)(sint32)contact_angle + 1024) & 4095) < 2048;
                }
                factor = brake != 0 ? 3993 : 4198;
                for (index = 0; index < 3; ++index)
                {
                    uint32 value = (uint32)(((sint64)(sint32)collision.fields.motion[index] * factor) >> 12);
                    collision.fields.motion[index] = value;
                }
                if (brake != 0)
                {
                    for (index = 0; index < 3; ++index)
                        this->velocity[index] = this->velocity[index] + collision.fields.motion[index];
                }
                else
                    this->TranslateCharacterWithBounds((sint32)collision.fields.motion[0], (sint32)collision.fields.motion[1], (sint32)collision.fields.motion[2]);
            }
            result = this->velocity[1] + 682;
            this->velocity[1] = result;
        }
    }
    return result;
}

uint32 AnimatedObject::TranslateCharacterWithBounds(sint32 x, sint32 y, sint32 z)
{
    FUNCTION_MARKER(0x8001D3F0, "SLUS_005.78");
    CollisionSphere *spheres = this->spheres;
    uint32 index;
    uint32 result;
    const PathDefinition *definition;
    for (index = 0; index < 4; ++index)
    {
        spheres[index].position[0] += (uint32)x;
        spheres[index].position[1] += (uint32)y;
        spheres[index].position[2] += (uint32)z;
    }
    this->world_position[0] = this->world_position[0] + (uint32)x;
    this->world_position[1] = this->world_position[1] + (uint32)y;
    this->world_position[2] = this->world_position[2] + (uint32)z;
    if ((this->flags & 0x2000) == 0)
    {
        definition = (this->path_definition);
        if (definition != 0 && (((const PathDefinition *)(definition))->transform) != 0)
        {
            VECTOR vector, transformed;
            MATRIX transpose = {0};
            xport_store_le16((uint8 *)&transpose + 18, 0);
            for (index = 0; index < 9; ++index)
            {
                const PathTransform *path_transform = (const PathTransform *)((((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) != 0 ? (((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) : (PathTransform *)0);
                transpose.m[index / 3][index % 3] = path_transform->matrix.m[index % 3][index / 3];
            }
            vector.vx = x;
            vector.vy = y;
            vector.vz = z;
            PushMatrix();
            for (index = 0; index < 5; ++index)
                xport_gte_write_control(index, xport_load_le32((const uint8 *)&transpose + index * 4));
            ApplyRotMatrixLV(&vector, &transformed);
            PopMatrix();
            x = transformed.vx;
            y = transformed.vy;
            z = transformed.vz;
        }
    }
    this->position[0] = this->position[0] + (uint32)x;
    this->position[1] = this->position[1] + (uint32)y;
    this->position[2] = this->position[2] + (uint32)z;
    character_controller.flags = character_controller.flags | 1;
    character_controller.support.candidate = 0;
    character_controller.support.fraction = 0;
    result = this->flags & ~0x4000u;
    this->flags = result;
    return result;
}

uint32 AnimatedObject::ProbeCharacterSupport()
{
    FUNCTION_MARKER(0x8001EB44, "SLUS_005.78");
    const SceneNode *other = character_controller.support.owner;
    const MeshGeometryNode *other_mesh = (const MeshGeometryNode *)other;
    uint32 kind = other_mesh->node.kind;
    uint32 packed = character_controller.support.contact_word;
    CollisionTransform collision = {0};
    uint32 fraction_limit;
    uint32 result;
    if (kind == 1)
    {
        uint32 first = psx_identity_rotation_words[0];
        uint32 second = psx_identity_rotation_words[1];
        uint32 third = psx_identity_rotation_words[2];

        collision.words[1] = first;
        collision.words[2] = second;
        collision.words[3] = third;
        collision.words[4] = psx_identity_rotation_words[3];
        collision.fields.rotation[8] = psx_identity_matrix.m[2][2];
        const MeshGeometryNode *position_source = other_mesh->vertex_count > 0 ? other_mesh : (((MeshGeometryNode *)(other_mesh->source_mesh)));
        first = position_source->position[0];
        second = position_source->position[1];
        third = position_source->position[2];
        collision.words[6] = first;
        collision.words[7] = second;
        collision.words[8] = third;
        collision.words[11] = 0;
        collision.words[10] = 0;
        collision.words[9] = 0;
    }
    else
    {
        const CollisionTransform *cached_transform = ((const AnimatedObject *)other_mesh)->collision_transforms;
        cached_transform = ((const CollisionTransform *)(cached_transform)) + (packed >> 16);
        uint32 offset;
        for (offset = 0; offset < 48; offset += 16)
        {
            uint32 first = cached_transform->words[(offset >> 2)];
            uint32 second = cached_transform->words[(offset >> 2) + 1];
            uint32 third = cached_transform->words[(offset >> 2) + 2];
            uint32 fourth = cached_transform->words[(offset >> 2) + 3];
            collision.words[(offset >> 2)] = first;
            collision.words[(offset >> 2) + 1] = second;
            collision.words[(offset >> 2) + 2] = third;
            collision.words[(offset >> 2) + 3] = fourth;
        }
    }
    collision.fields.face_count = 1;
    collision.fields.first_face = packed;
    fraction_limit = 4096;
    result = (uint32)this->FindSphereFaceCollision(character_geometry.world, character_controller.support.owner, &collision, &fraction_limit);
    if ((sint32)result >= 0)
        this->velocity[1] = 0;
    else
    {
        result = this->velocity[1];
        if (result != 0)
        {
            result = character_controller.flags & 0xFFFF7FFF;
            character_controller.flags = result;
        }
    }
    return result;
}

uint32 AnimatedObject::ResolveCharacterSupport(uint32 grounded)
{
    const PathDefinition *parent, *controller_definition;
    const PathTransform *captured_path_transform;
    FUNCTION_MARKER(0x8001DB60, "SLUS_005.78");
    CollisionTransform record = {0};
    SupportVector old_velocity, inverse, input;
    MATRIX matrix_local = {0};
    uint32 fraction;
    SupportVector world, floor;
    uint32 contact, type, pointer, index, row, axis;
    const MeshFace *triangle;
    const PathTransform *path_transform;
    const ScenePath *controller_path;
    const SceneNode *owner;
    const MeshGeometryNode *owner_mesh;
    uint32 x, y, z, a, b, c, d, e, f, g, h, i, flags, value, aligned, factor, result;
    sint32 query, correction, heading, angle;
    xport_store_le16((uint8 *)&matrix_local + 18, 0);
    owner = character_controller.support.owner;
    owner_mesh = (const MeshGeometryNode *)owner;
    type = owner_mesh->node.kind;
    contact = character_controller.support.contact_word;
    if (type == 1)
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
        const MeshGeometryNode *position_source = owner_mesh->vertex_count > 0 ? owner_mesh : (((MeshGeometryNode *)(owner_mesh->source_mesh)));
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
    fraction = 4096;
    triangle = ((MeshFace *)(owner_mesh->polygons)) + (uint16)contact;
    if ((uint8)grounded != 0)
    {
        x = this->velocity[0];
        y = this->velocity[1];
        z = this->velocity[2];
        this->velocity[0] = x << 2;
        this->velocity[1] = y << 2;
        this->velocity[2] = z << 2;
    }
    owner = character_controller.support.owner;
    owner_mesh = (const MeshGeometryNode *)owner;
    query = this->FindSphereFaceCollision(character_geometry.world, owner, &record, &fraction);
    if ((uint8)grounded != 0)
    {
        x = (uint32)((sint32)this->velocity[0] >> 2);
        y = (uint32)((sint32)this->velocity[1] >> 2);
        z = (uint32)((sint32)this->velocity[2] >> 2);
        this->velocity[0] = x;
        this->velocity[1] = y;
        this->velocity[2] = z;
        if (query < 0)
            goto no_contact;
        if ((this->AcceptVerticalFaceContact(&record, (uint32)query) & 0xFF) != 0)
            query = -1;
    }
    if (query < 0)
        goto no_contact;
    flags = character_controller.flags;
    controller_path = character_controller.path;
    character_controller.support.candidate = 0;
    character_controller.support.fraction = 0;
    character_controller.flags = flags | 1;
    controller_definition = (controller_path->definition);
    parent = (this->path_definition);
    if (controller_definition != parent)
    {
        if (parent != 0 && ((((const PathDefinition *)(parent)))->transform) != 0)
        {
            x = this->position[0];
            y = this->position[1];
            z = this->position[2];
            world.words[0] = x;
            world.words[1] = y;
            world.words[2] = z;
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
            ApplyRotMatrixLV(&world.vector, (VECTOR *)this->position);
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
        SelectScenePath(character_controller.path);
        controller_path = character_controller.path;
        parent = (controller_path->definition);
        this->path_definition = parent;
        if (parent != 0 && (path_transform = ((captured_path_transform = ((((const PathDefinition *)(parent)))->transform)) != 0 ? (const PathTransform *)((PathTransform *)(captured_path_transform)) : 0)) != 0)
        {
            for (index = 0; index < 3; ++index)
            {
                if (index != 0)
                    path_transform = (const PathTransform *)((((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) != 0 ? (((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) : (PathTransform *)0);
                x = this->position[index];
                value = (uint32)path_transform->matrix.t[index];
                inverse.words[index] = x - value;
            }
            for (index = 0; index < 9; ++index)
            {
                path_transform = (const PathTransform *)((((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) != 0 ? (((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) : (PathTransform *)0);
                value = (uint16)path_transform->matrix.m[index % 3][index / 3];
                matrix_local.m[index / 3][index % 3] = value;
            }
            PushMatrix();
            x = xport_load_le32((const uint8 *)&matrix_local + 0);
            y = xport_load_le32((const uint8 *)&matrix_local + 4);
            xport_gte_write_control(0, x);
            xport_gte_write_control(1, y);
            x = xport_load_le32((const uint8 *)&matrix_local + 8);
            y = xport_load_le32((const uint8 *)&matrix_local + 12);
            z = (uint16)matrix_local.m[2][2];
            xport_gte_write_control(2, x);
            xport_gte_write_control(3, y);
            xport_gte_write_control(4, z);
            ApplyRotMatrixLV(&inverse.vector, (VECTOR *)this->position);
            PopMatrix();
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
        this->flags = this->flags & 0xFFFFBFFF;
    }
    x = this->velocity[0];
    y = this->velocity[1];
    z = this->velocity[2];
    old_velocity.words[0] = x;
    old_velocity.words[1] = y;
    old_velocity.words[2] = z;
    this->velocity[2] = 0;
    this->velocity[0] = 0;
    if ((uint8)grounded == 0)
    {
        if (((uint16)(character_controller.motion.form_flags >> 16) & 15) == 1 && (this->flags & 0x04000000) != 0)
            this->EndCharacterSpecialMove();
        flags = this->flags;
        this->velocity[1] = 0;
        this->flags = flags & 0xFFFFFBFF;
        if ((flags & 0x2000) == 0)
        {
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
                ApplyRotMatrixLV(&input.vector, &world.vector);
                PopMatrix();
                for (index = 0; index < 3; ++index)
                {
                    path_transform = (const PathTransform *)((((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) != 0 ? (((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) : (PathTransform *)0);
                    x = world.words[index];
                    value = (uint32)path_transform->matrix.t[index];
                    world.words[index] = x + value;
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
            parent = (this->path_definition);
            flags = this->flags;
            correction = (sint32)(((const PathDefinition *)(parent)))->FindHeightFrame(world.words, flags & 0x10, -40960);
            if (correction >= 0)
            {
                flags = this->flags;
                parent = (this->path_definition);
                correction = (((const PathDefinition *)(parent)))->ProjectFrame(correction, world.words, floor.words, flags & 0x10, 4096);
                if (correction >= 0)
                {
                    pointer = OriginalDefaults::SceneContextIdentity;
                    camera_motion.frame = correction;
                    y = floor.words[1];
                    value = world.words[1];
                    this->velocity[1] = y - value;
                }
            }
        }
    }
    else if ((sint32)this->velocity[1] > 0)
    {
        value = ((uint32)query & 0x7000) >> 12;
        if (character_geometry.local[0].position[2] != 0 || value >= 4 || character_geometry.world[value].radius == 0xFFFFFFFF)
        {
            a = (uint16)record.fields.rotation[0];
            b = (uint16)record.fields.rotation[3];
            c = (uint16)record.fields.rotation[6];
            d = (uint16)record.fields.rotation[1];
            e = (uint16)record.fields.rotation[4];
            f = (uint16)record.fields.rotation[7];
            g = (uint16)record.fields.rotation[2];
            h = (uint16)record.fields.rotation[5];
            i = (uint16)record.fields.rotation[8];
            matrix_local.m[0][0] = a;
            matrix_local.m[0][1] = b;
            matrix_local.m[0][2] = c;
            matrix_local.m[1][0] = d;
            matrix_local.m[1][1] = e;
            matrix_local.m[1][2] = f;
            matrix_local.m[2][0] = g;
            matrix_local.m[2][1] = h;
            matrix_local.m[2][2] = i;
            PushMatrix();
            x = xport_load_le32((const uint8 *)&matrix_local + 0);
            y = xport_load_le32((const uint8 *)&matrix_local + 4);
            xport_gte_write_control(0, x);
            xport_gte_write_control(1, y);
            x = xport_load_le32((const uint8 *)&matrix_local + 8);
            y = xport_load_le32((const uint8 *)&matrix_local + 12);
            z = (uint16)matrix_local.m[2][2];
            xport_gte_write_control(2, x);
            xport_gte_write_control(3, y);
            xport_gte_write_control(4, z);
            xport_gte_write_data(0, triangle->normal.words[0]);
            xport_gte_write_data(1, triangle->normal.words[1]);
            xport_gte_execute(0x486012);
            floor.words[0] = xport_gte_read_data(25);
            floor.words[1] = xport_gte_read_data(26);
            floor.words[2] = xport_gte_read_data(27);
            PopMatrix();
            if ((sint32)floor.words[1] < -256)
            {
                value = character_controller.movement.jump_ticks;
                flags = character_controller.flags;
                character_controller.motion.jump_phase = -1;
                character_controller.motion.vertical_speed = 0;
                character_controller.timers.state_ticks = 0;
                character_controller.timers.jump_duration_ticks = value;
                character_controller.flags = flags | 0x08000000;
                if ((character_controller.secondary_flags & 0x80) == 0)
                    character_controller.motion.state_code = 6;
                this->velocity[1] = 0;
                result = 6;
                goto finish;
            }
        }
    }
    parent = (this->path_definition);
    if (parent != 0 && (path_transform = ((captured_path_transform = ((((const PathDefinition *)(parent)))->transform)) != 0 ? (const PathTransform *)((PathTransform *)(captured_path_transform)) : 0)) != 0 && (this->flags & 0x2000) == 0)
    {
        for (index = 0; index < 3; ++index)
        {
            if (index != 0)
                path_transform = (const PathTransform *)((((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) != 0 ? (((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) : (PathTransform *)0);
            x = record.fields.motion[index];
            value = path_transform->displacement[index];
            record.fields.motion[index] = x - value;
        }
    }
    if (record.fields.motion[0] == 0 && record.fields.motion[2] == 0)
    {
        if (character_controller.timers.contact_impulse_ticks == 0)
            character_controller.motion.horizontal_speed = 0;
        goto save_frame;
    }
    x = old_velocity.words[0];
    aligned = 0;
    if (x != 0 || old_velocity.words[2] != 0)
    {
        z = old_velocity.words[2];
        heading = (sint16)Atan2Angle((sint32)z, (sint32)x);
        z = record.fields.motion[2];
        x = record.fields.motion[0];
        angle = (sint16)Atan2Angle((sint32)z, (sint32)x);
        aligned = (((uint32)heading - (uint32)angle + 1024) & 0xFFF) < 2048;
    }
    if (fraction != 0)
    {
        old_velocity.words[2] = 0;
        old_velocity.words[1] = 0;
        old_velocity.words[0] = 0;
        goto damp_velocity;
    }
    x = record.fields.motion[0];
    y = record.fields.motion[1];
    z = record.fields.motion[2];
    old_velocity.words[0] = x;
    old_velocity.words[1] = y;
    old_velocity.words[2] = z;
    if (aligned != 0)
    {
        x = old_velocity.words[0];
        y = old_velocity.words[1];
        z = old_velocity.words[2];
        old_velocity.words[0] = 0 - x;
        old_velocity.words[1] = 0 - y;
        old_velocity.words[2] = 0 - z;
    }
    x = old_velocity.words[0];
    a = (uint32)(((sint64)(sint32)x * 40960) >> 12);
    y = old_velocity.words[1];
    old_velocity.words[0] = a;
    b = (uint32)(((sint64)(sint32)y * 40960) >> 12);
    z = old_velocity.words[2];
    old_velocity.words[1] = b;
    old_velocity.words[2] = (uint32)(((sint64)(sint32)z * 40960) >> 12);
    for (row = 0; row < 4; ++row)
    {
        for (axis = 0; axis < 3; ++axis)
        {
            x = this->spheres[row].position[axis];
            y = old_velocity.words[axis];
            this->spheres[row].position[axis] = x + y;
        }
    }
    for (axis = 0; axis < 3; ++axis)
    {
        x = this->velocity[axis];
        y = old_velocity.words[axis];
        this->velocity[axis] = x - y;
    }
    fraction = 4096;
    owner = character_controller.support.owner;
    owner_mesh = (const MeshGeometryNode *)owner;
    type = owner_mesh->node.kind;
    contact = character_controller.support.contact_word;
    if (type == 1)
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
        const MeshGeometryNode *position_source = owner_mesh->vertex_count > 0 ? owner_mesh : (((MeshGeometryNode *)(owner_mesh->source_mesh)));
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
    owner = character_controller.support.owner;
    owner_mesh = (const MeshGeometryNode *)owner;
    query = this->FindSphereFaceCollision(character_geometry.world, owner, &record, &fraction);
    for (axis = 0; axis < 3; ++axis)
    {
        x = this->velocity[axis];
        y = old_velocity.words[axis];
        this->velocity[axis] = x + y;
    }
    for (row = 0; row < 4; ++row)
    {
        for (axis = 0; axis < 3; ++axis)
        {
            x = this->spheres[row].position[axis];
            y = old_velocity.words[axis];
            this->spheres[row].position[axis] = x - y;
        }
    }
    if (query >= 0 && (factor = fraction) != 0)
    {
        factor = 4096 - factor;
        fraction = factor;
        x = old_velocity.words[0];
        a = (uint32)(((sint64)(sint32)x * (sint32)factor) >> 12);
        y = old_velocity.words[1];
        old_velocity.words[0] = a;
        b = (uint32)(((sint64)(sint32)y * (sint32)factor) >> 12);
        z = old_velocity.words[2];
        old_velocity.words[1] = b;
        old_velocity.words[2] = (uint32)(((sint64)(sint32)z * (sint32)factor) >> 12);
    }
    else
    {
        old_velocity.words[2] = 0;
        old_velocity.words[1] = 0;
        old_velocity.words[0] = 0;
    }
    if ((this->flags & 0x2000) == 0)
    {
        parent = (this->path_definition);
        if (parent != 0 && (path_transform = ((captured_path_transform = ((((const PathDefinition *)(parent)))->transform)) != 0 ? (const PathTransform *)((PathTransform *)(captured_path_transform)) : 0)) != 0)
        {
            for (index = 0; index < 3; ++index)
            {
                if (index != 0)
                    path_transform = (const PathTransform *)((((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) != 0 ? (((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) : (PathTransform *)0);
                x = record.fields.motion[index];
                value = path_transform->displacement[index];
                record.fields.motion[index] = x - value;
            }
        }
    }
damp_velocity:
    factor = aligned != 0 ? 3993 : 4198;
    x = record.fields.motion[0];
    a = (uint32)(((sint64)(sint32)x * (sint32)factor) >> 12);
    y = record.fields.motion[1];
    record.words[9] = a;
    b = (uint32)(((sint64)(sint32)y * (sint32)factor) >> 12);
    z = record.fields.motion[2];
    record.words[10] = b;
    c = (uint32)(((sint64)(sint32)z * (sint32)factor) >> 12);
    record.words[11] = c;
    if (aligned != 0)
    {
        this->velocity[0] = this->velocity[0] + a;
        y = this->velocity[1];
        value = record.fields.motion[1];
        this->velocity[1] = y + value;
        z = this->velocity[2];
        value = record.fields.motion[2];
        this->velocity[2] = z + value;
    }
    else
    {
        x = old_velocity.words[0];
        y = old_velocity.words[1];
        z = old_velocity.words[2];
        old_velocity.words[0] = a + x;
        old_velocity.words[1] = b + y;
        old_velocity.words[2] = c + z;
        if (character_controller.timers.contact_impulse_ticks == 0)
            character_controller.motion.horizontal_speed = 0;
    }
    z = old_velocity.words[2];
    x = old_velocity.words[0];
    y = old_velocity.words[1];
    this->TranslateCharacterWithBounds((sint32)x, (sint32)y, (sint32)z);
save_frame:
    pointer = OriginalDefaults::SceneContextIdentity;
    flags = this->flags;
    value = (uint16)camera_motion.frame;
    result = flags & 0xFFFFF7FF;
    this->flags = result;
    this->path_frame = value;
    goto finish;
no_contact:
    result = this->velocity[0];
    if (result == 0)
        result = this->velocity[2];
    if (result != 0)
    {
        result = character_controller.flags & 0xFFFFFFBF;
        character_controller.flags = result;
    }
finish:
    return result;
}

const ScenePath *AnimatedObject::FindCharacterSupportPath(const SceneTrigger *support)
{
    const ScenePath *support_first, *support_second, *anchor_target;
    const PathTransform *captured_path_transform;

    FUNCTION_MARKER(0x8004E118, "SLUS_005.78");
    const ScenePath *camera_path;
    const ScenePath *start, *current, *backup, *previous, *next, *result;
    uint32 floor[3], query_frame, backup_frame;
    SupportVector position, input, transform;
    const ScenePath *visited[7];

    union
    {
        uint32 words[5];
        uint16 halves[10];
    } transpose;

    const PathDefinition *owner, *parent;
    uint32 flags, direction;
    const PathTransform *path_transform;
    uint32 x, y, z, a, b, value, frame, steps, index, height_delta, need_transition;
    sint32 travel_direction;
    flags = this->path_flags;
    steps = 0;
    if ((flags & 0x400000) != 0)
    {
        if (support != 0)
        {
            const SceneTrigger *anchor = (const SceneTrigger *)(character_controller.path_transition.anchor);
            support_first = (support->parameters.path_pair.targets[0]);
            anchor_target = (anchor->parameters.path.target);
            support_second = (support->parameters.path_pair.targets[1]);
            if (support_first == anchor_target || support_second == anchor_target)
                goto failed;
        }
    }
    else if ((flags & 0x20) != 0 && support != 0 && path_transition_rules.release_mode == 1)
    {
        support_first = (support->parameters.path_pair.targets[0]);
        const ScenePath *anchor = character_controller.path_transition.anchor_path;
        support_second = (support->parameters.path_pair.targets[1]);
        if (support_first == anchor || support_second == anchor)
            goto failed;
    }
    flags = this->flags;
    direction = flags & 0x10;
    if (support != 0)
    {
        flags &= 0x2000;
        if (direction != 0)
        {
            start = (support->parameters.path_pair.targets[0]);
        }
        else
        {
            start = (support->parameters.path_pair.targets[1]);
            flags = this->flags & 0x2000;
        }
    }
    else
    {
        z = this->velocity[2];
        x = this->velocity[0];
        start = character_controller.path;
        value = (uint32)(sint32)(sint16)Atan2Angle((sint32)z, (sint32)x);
        query_frame = value;
        if ((sint32)value == 4096)
            travel_direction = 0;
        else
        {
            a = (uint32)this->target_yaw;
            travel_direction = -1;
            if (((value - a - 1024) & 0xFFF) < 2048)
                travel_direction = 1;
        }
        flags = this->flags & 0x2000;
    }
    if (flags == 0 && (parent = (this->path_definition)) != 0 && ((((const PathDefinition *)(parent)))->transform) != 0)
    {
        x = this->position[0];
        y = this->position[1];
        z = this->position[2];
        input.words[0] = x;
        input.words[1] = y;
        input.words[2] = z;
        current = start;
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
        ApplyRotMatrixLV(&input.vector, &position.vector);
        backup = 0;
        PopMatrix();
        for (index = 0; index < 3; ++index)
        {
            path_transform = (const PathTransform *)((((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) != 0 ? (((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) : (PathTransform *)0);
            value = position.words[index];
            position.words[index] = value + (uint32)path_transform->matrix.t[index];
        }
    }
    else
    {
        x = this->position[0];
        y = this->position[1];
        z = this->position[2];
        position.words[0] = x;
        position.words[1] = y;
        position.words[2] = z;
        current = start;
        backup = 0;
    }
    previous = 0;
    visited[0] = 0;
    for (index = 6; index != 0; --index)
        visited[index] = 0;
walk:
    value = support_search_limit;
    ++steps;
    if ((sint32)steps >= (sint32)value)
        goto failed;
    for (index = 0; index < 7; ++index)
    {
        if (visited[index] == current)
            goto failed;
    }
    for (index = 0; index < 7; ++index)
    {
        if (visited[index] == 0 && current != start)
        {
            visited[index] = current;
            break;
        }
    }
    frame = ((const PathDefinition *)((const PathDefinition *)((current->definition))))->FindHeightFrame(position.words, direction, -40960);
    query_frame = frame;
    if ((sint32)frame < 0)
        goto failed;
    frame = (uint32)((const PathDefinition *)((const PathDefinition *)((current->definition))))->ProjectFrame((sint32)frame, position.words, floor, direction, 4096);
    owner = (current->definition);
    a = floor[1];
    b = position.words[1];
    value = (uint32)((sint16)((const PathDefinition *)(owner))->positions.metadata6 - 1);
    height_delta = a - b;
    need_transition = (sint32)(frame - 1) >= (sint32)value;
    if (need_transition == 0 && support == 0)
    {
        flags = ((const PathDefinition *)(owner))->flags;
        if ((flags & 0x11) != 0 || (this->flags & 0x2000) != 0)
        {
            if ((sint32)b < (sint32)a)
                need_transition = 1;
        }
        if (need_transition == 0 && current != start && (((const PathDefinition *)((current->definition)))->flags & 0x11) == 0 && (((const PathDefinition *)((previous->definition)))->flags & 0x11) != 0 && (sint32)height_delta >= 1366)
            need_transition = 1;
    }
    if (need_transition != 0)
    {
        if (support != 0)
            goto failed;
        if ((this->flags & 0x2000) != 0)
        {
            if ((sint32)frame < 0)
                goto failed;
            if ((((const PathDefinition *)((current->definition)))->flags & 0x11) == 0)
            {
                a = floor[1];
                b = position.words[1];
                if ((sint32)b < (sint32)a)
                    goto failed;
            }
        }
        if ((sint32)frame > 0)
        {
            owner = (current->definition);
            value = (uint32)(sint16)((const PathDefinition *)(owner))->positions.metadata6;
            if ((sint32)frame < (sint32)value && (((const PathDefinition *)(owner))->flags & 0x10) != 0)
            {
                next = current->fallback;
                if (next == 0)
                    goto accept;
                if ((current->node.state_flags & (direction | 8)) == 0 && (next->node.state_flags & 8) != 0)
                {
                    next = (next->adjacent[0]);
                }
                if (next == 0 || next == previous || next == start)
                    goto accept;
                goto advance;
            }
        }
        if ((sint32)frame >= 0)
        {
            value = ((const PathDefinition *)((current->definition)))->positions.metadata6;
            travel_direction = -1;
            if (((sint32)(sint16)value >> 1) < (sint32)frame)
                travel_direction = 1;
        }
        query_frame = (uint32)travel_direction;
        next = current->Step(&query_frame, direction);
        if (next == 0 || next == previous || next == start)
        {
            current = backup;
            if (backup == 0)
                goto failed;
            frame = backup_frame;
            ((const PathDefinition *)((const PathDefinition *)((current->definition))))->ProjectFrame((sint32)frame, position.words, floor, direction, 4096);
            goto accept;
        }
    advance:
        if ((sint32)frame >= 0 && backup == 0)
        {
            backup = current;
            backup_frame = frame;
        }
        frame = 0xFFFFFFFF;
        previous = current;
        current = next;
    }
    if ((sint32)frame < 0)
        goto walk;
accept:
    owner = (current->definition);
    this->path_frame = frame;
    this->path_definition = owner;
    owner = (current->definition);
    a = floor[1];
    b = position.words[1];
    flags = ((const PathDefinition *)(owner))->flags;
    height_delta = a - b;
    if ((flags & 0x11) != 0 || (((this->flags & 0x400) != 0 || current != start) && (sint32)height_delta < -681))
    {
        owner = (current->definition);
        if ((((const PathDefinition *)(owner))->flags & 0x10000) == 0)
            goto leave_ground;
        camera_path = camera_motion.path;
        parent = (camera_path->definition);
        if ((((const PathDefinition *)(parent))->flags & 0x10000) != 0)
            goto leave_ground;
    }
    this->flags = this->flags | 0x4000;
    goto after_ground;
leave_ground:
    this->flags = (this->flags & 0xFFFFBFFF) | 0x400;
    floor[1] = position.words[1];
after_ground:
    flags = this->path_flags;
    if ((flags & 2) != 0)
    {
        value = this->flags;
        if ((value & 0x4000) != 0 && (flags & 4) == 0)
            this->flags = value & 0xFFFFBFFF;
    }
    if ((this->path_flags & 0x20) != 0)
    {
        this->ClearCharacterPathRelease();
        this->ClearCharacterPathStep();
    }
    this->flags = this->flags & 0xFFFFD7FF;
    if ((((const PathDefinition *)((current->definition)))->flags & 0x10000) != 0)
    {
        x = floor[0];
        z = floor[2];
        position.words[0] = x;
        position.words[2] = z;
    }
    flags = this->flags;
    if ((flags & 0x4400) == 0x400 || ((sint8)this->metadata126[0] == 1 && support == 0 && (flags & 0x4000) == 0))
    {
        x = position.words[0];
        z = position.words[2];
        floor[0] = x;
        floor[2] = z;
    }
    parent = (this->path_definition);
    if (parent != 0 && (path_transform = ((captured_path_transform = ((((const PathDefinition *)(parent)))->transform)) != 0 ? (const PathTransform *)((PathTransform *)(captured_path_transform)) : 0)) != 0)
    {
        value = floor[0];
        transform.words[0] = value - (uint32)path_transform->matrix.t[0];
        for (index = 1; index < 3; ++index)
        {
            path_transform = (const PathTransform *)((((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) != 0 ? (((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) : (PathTransform *)0);
            value = floor[index];
            transform.words[index] = value - (uint32)path_transform->matrix.t[index];
        }
        for (index = 0; index < 9; ++index)
        {
            path_transform = (const PathTransform *)((((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) != 0 ? (((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) : (PathTransform *)0);
            transpose.halves[index] = path_transform->matrix.m[index % 3][index / 3];
        }
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
        ApplyRotMatrixLV(&transform.vector, (VECTOR *)this->position);
        PopMatrix();
    }
    else
    {
        x = floor[0];
        y = floor[1];
        z = floor[2];
        this->position[0] = x;
        this->position[1] = y;
        this->position[2] = z;
    }
    result = current;
    goto finish_support;
failed:
    result = 0;
finish_support:
    return result;
}
