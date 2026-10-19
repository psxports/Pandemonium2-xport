#include "game.h"

CharacterGeometry character_geometry = {0};
CollisionTransform collision_transform = {0};
TriggerContactMotion trigger_contact_motion = {0};

union SweepVector
{
    VECTOR vector;
    uint32 words[4];
};

CollisionSphere *AnimatedObject::BuildCharacterGeometry()
{
    FUNCTION_MARKER(0x8001BD30, "SLUS_005.78");
    MATRIX rotation;
    SweepVector world;
    VECTOR input;
    uint32 minimum[3], maximum[3], low[3], high[3];
    const PathDefinition *parent;
    uint32 count, offset, index, value, x, y, z, a, b, c;
    uint32 radius, half_y, half_z, ratio, spacing, angle, cosine, sine;
    CollisionSphere *output, *destination;
    const CollisionSphere *source;
    const MATRIX *parent_matrix;
    const SceneNode *shape_node;
    sint32 quotient;
    if ((this->flags & 0x2000) == 0 && (parent = (this->path_definition)) != 0 && ((parent)->transform) != 0)
    {
        x = this->position[0];
        y = this->position[1];
        z = this->position[2];
        input.vx = x;
        input.vy = y;
        input.vz = z;
        PushMatrix();
        parent_matrix = (const MATRIX *)((((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) != 0 ? &(((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform)->matrix : (MATRIX *)0);
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
        ApplyRotMatrixLV(&input, &world.vector);
        PopMatrix();
        for (index = 0; index < 3; ++index)
        {
            parent_matrix = (const MATRIX *)((((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) != 0 ? &(((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform)->matrix : (MATRIX *)0);
            value = world.words[index];
            world.words[index] = value + (uint32)parent_matrix->t[index];
        }
    }
    else
    {
        x = this->position[0];
        y = this->position[1];
        z = this->position[2];
        world.vector.vx = x;
        world.vector.vy = y;
        world.vector.vz = z;
    }
    if ((character_controller.flags & 0x40000000) == 0)
    {
        character_geometry.world[3].radius = 0xFFFFFFFF;
        character_geometry.world[2].radius = 0xFFFFFFFF;
        character_geometry.world[1].radius = 0xFFFFFFFF;
        character_geometry.world[0].radius = 0xFFFFFFFF;
        shape_node = this->node.links.first_child;
        count = 0;
        offset = 0;
        output = character_geometry.world;
        while (shape_node != 0)
        {
            if (shape_node->kind == 6)
            {
                const SphereNode *sphere_node = (const SphereNode *)shape_node;
                x = sphere_node->position[0];
                y = sphere_node->position[1];
                z = sphere_node->position[2];
                output->position[0] = x;
                output->position[1] = y;
                output->position[2] = z;
                radius = sphere_node->radius & 0xFFFFFFF8;
                output->radius = radius;
                high[2] = radius;
                high[1] = radius;
                high[0] = radius;
                x = output->position[0] - radius;
                low[0] = x;
                y = output->position[1] - radius;
                low[1] = y;
                z = output->position[2] - radius;
                low[2] = z;
                a = output->position[0] + radius;
                high[0] = a;
                b = output->position[1] + radius;
                high[1] = b;
                c = output->position[2] + radius;
                high[2] = c;
                if (count == 0)
                {
                    x = low[0];
                    y = low[1];
                    z = low[2];
                    minimum[0] = x;
                    minimum[1] = y;
                    minimum[2] = z;
                    x = high[0];
                    y = high[1];
                    z = high[2];
                    maximum[0] = x;
                    maximum[1] = y;
                    maximum[2] = z;
                }
                else
                {
                    if ((sint32)x < (sint32)minimum[0])
                        minimum[0] = x;
                    if ((sint32)maximum[0] < (sint32)a)
                        maximum[0] = a;
                    if ((sint32)y < (sint32)minimum[1])
                        minimum[1] = y;
                    if ((sint32)maximum[1] < (sint32)b)
                        maximum[1] = b;
                    if ((sint32)z < (sint32)minimum[2])
                        minimum[2] = z;
                    if ((sint32)maximum[2] < (sint32)c)
                        maximum[2] = c;
                }
                ++output;
                offset += 16;
                ++count;
            }
            shape_node = shape_node->links.sibling_next;
        }
        if (count == 0)
            goto copy_geometry;
        x = minimum[0];
        a = maximum[0];
        value = 0 - x;
        if ((sint32)value < (sint32)a)
            minimum[0] = 0 - a;
        else
            maximum[0] = value;
        z = minimum[2];
        c = maximum[2];
        value = 0 - z;
        if ((sint32)value < (sint32)c)
            minimum[2] = 0 - c;
        else
            maximum[2] = value;
        b = maximum[1];
        y = minimum[1];
        c = maximum[2];
        z = minimum[2];
        half_y = (uint32)((sint32)(b - y) >> 1);
        half_z = (uint32)((sint32)(c - z) >> 1);
        if (half_z == 0)
            xport_mips_break(7);
        quotient = 0x40000000 / (sint32)half_z;
        world.vector.vy = half_y;
        world.vector.vz = half_z;
        ratio = (uint32)(((sint64)(sint32)half_y * quotient) >> 18);
        fixed_math_result = ratio;
        character_geometry.world[0].position[0] = 0;
        world.vector.vx = ratio;
        if (ratio - 3277 < 0x666)
        {
            radius = (sint32)ratio < 4096 ? half_z : half_y;
            character_geometry.world[0].position[1] = y + radius;
            character_geometry.world[0].radius = radius;
            character_geometry.world[0].position[2] = 0;
            character_geometry.world[3].radius = 0xFFFFFFFF;
            character_geometry.world[2].radius = 0xFFFFFFFF;
            character_geometry.world[1].radius = 0xFFFFFFFF;
        }
        else if ((sint32)half_z < (sint32)half_y)
        {
            spacing = (uint32)((sint32)half_y / 4);
            world.vector.vy = spacing;
            if ((sint32)spacing < (sint32)half_z)
            {
                value = (b - half_z) - (y + half_z);
                character_geometry.world[0].radius = half_z;
                spacing = (uint32)((sint32)value / 6);
                world.vector.vy = spacing;
            }
            else
                character_geometry.world[0].radius = spacing;
            spacing = (uint32)world.vector.vy;
            y = minimum[1];
            radius = character_geometry.world[0].radius;
            character_geometry.world[0].position[2] = 0;
            spacing <<= 1;
            world.vector.vy = spacing;
            character_geometry.world[0].position[1] = y + radius;
            for (offset = 16; offset < 64; offset += 16)
            {
                if (character_geometry.world[offset / 16].radius != 0xFFFFFFFF)
                {
                    source = &character_geometry.world[offset / 16 - 1];
                    destination = &character_geometry.world[offset / 16];
                    x = source->position[0];
                    y = source->position[1];
                    z = source->position[2];
                    radius = source->radius;
                    destination->position[0] = x;
                    destination->position[1] = y;
                    destination->position[2] = z;
                    destination->radius = radius;
                    value = destination->position[1];
                    destination->position[1] = value + (uint32)world.vector.vy;
                }
            }
        }
        else
        {
            spacing = (uint32)((sint32)half_z / 4);
            world.vector.vz = spacing;
            if ((sint32)spacing < (sint32)half_y)
            {
                value = (c - half_y) - (z + half_y);
                character_geometry.world[0].radius = half_y;
                spacing = (uint32)((sint32)value / 6);
                world.vector.vz = spacing;
            }
            else
                character_geometry.world[0].radius = spacing;
            spacing = (uint32)world.vector.vz;
            y = minimum[1];
            radius = character_geometry.world[0].radius;
            z = minimum[2];
            spacing <<= 1;
            world.vector.vz = spacing;
            character_geometry.world[0].position[1] = y + radius;
            character_geometry.world[0].position[2] = z + radius;
            for (offset = 16; offset < 64; offset += 16)
            {
                if (character_geometry.world[offset / 16].radius != 0xFFFFFFFF)
                {
                    source = &character_geometry.world[offset / 16 - 1];
                    destination = &character_geometry.world[offset / 16];
                    x = source->position[0];
                    y = source->position[1];
                    z = source->position[2];
                    radius = source->radius;
                    destination->position[0] = x;
                    destination->position[1] = y;
                    destination->position[2] = z;
                    destination->radius = radius;
                    value = destination->position[2];
                    destination->position[2] = value + (uint32)world.vector.vz;
                }
            }
        }
    copy_geometry:
        for (index = 0; index < 4; ++index)
            character_geometry.local[index] = character_geometry.world[index];
        if ((this->flags & 0x10000) == 0)
            character_controller.flags = character_controller.flags | 0x40000000;
    }
    value = this->flags;
    angle = (uint16)this->target_yaw;
    if ((value & 0x10) != 0)
        angle += 2048;
    x = psx_identity_rotation_words[0];
    y = psx_identity_rotation_words[1];
    xport_gte_write_control(0, x);
    xport_gte_write_control(1, y);
    x = psx_identity_rotation_words[2];
    y = psx_identity_rotation_words[3];
    z = psx_identity_rotation_words[4];
    xport_gte_write_control(2, x);
    xport_gte_write_control(3, y);
    xport_gte_write_control(4, z);
    cosine = (uint32)(SinQ14((sint16)(1024 - angle)) >> 2);
    sine = (uint32)(SinQ14((sint16)angle) >> 2);
    rotation_y.m[0][0] = (sint16)(cosine);
    rotation_y.m[2][0] = (sint16)(0 - sine);
    rotation_y.m[0][2] = (sint16)(sine);
    rotation_y.m[2][2] = (sint16)(cosine);
    MulRotMatrix0(&rotation_y, &rotation);
    SetRotMatrix(&rotation);
    for (offset = 0; offset < 64; offset += 16)
    {
        value = character_geometry.local[offset / 16].radius;
        output = &character_geometry.world[offset / 16];
        output->radius = value;
        if (value != 0xFFFFFFFF)
        {
            VECTOR input_vector, output_vector;
            input_vector.vx = character_geometry.local[offset / 16].position[0];
            input_vector.vy = character_geometry.local[offset / 16].position[1];
            input_vector.vz = character_geometry.local[offset / 16].position[2];
            ApplyRotMatrixLV(&input_vector, &output_vector);
            output->position[0] = output_vector.vx;
            output->position[1] = output_vector.vy;
            output->position[2] = output_vector.vz;
            value = output->position[0];
            output->position[0] = value + (uint32)world.vector.vx;
            value = output->position[1];
            output->position[1] = value + (uint32)world.vector.vy;
            value = output->position[2];
            output->position[2] = value + (uint32)world.vector.vz;
        }
    }
    return character_geometry.world;
}

sint32 AnimatedObject::FindSphereFaceCollision(const CollisionSphere *geometry, const SceneNode *other, const CollisionTransform *transform, uint32 *fraction_limit) const
{
    const PathTransform *path_transform;
    FUNCTION_MARKER(0x800380AC, "SLUS_005.78");
    VECTOR start, end, point;
    const MeshFace *polygon;
    const MeshVertex *vertex;
    const MeshVertex *vertex_base;
    const MeshFace *face_base, *face, *selected_face = 0;
    uint32 shape, face_index, edge, i, radius, radius_square, margin, value;
    uint32 motion_x, motion_y, motion_z, x, y, z, sx, sy, sz, ex, ey, ez;
    uint32 vx, vy, vz, nx, ny, nz, dx, dy, dz, cx, cy, cz, px, py, pz;
    uint32 dot, start_distance, end_distance, fraction, edge_radius, edge_square, length_square, bound;
    sint32 face_count = transform->fields.face_count;
    const CollisionSphere *sphere, *selected_sphere = 0;
    sint32 reciprocal, angle, result;
    if (face_count <= 0)
        return -1;
    const MeshGeometryNode *mesh = other != 0 ? (const MeshGeometryNode *)other : (const MeshGeometryNode *)0;
    const MeshGeometryNode *vertices_owner = mesh;
    face_base = ((MeshFace *)(mesh->polygons)) + transform->fields.first_face;
    if (mesh->vertex_count <= 0)
        vertices_owner = (((MeshGeometryNode *)(mesh->source_mesh)));
    vertex_base = vertices_owner->vertices;
    motion_x = this->velocity[0] - transform->fields.motion[0];
    motion_y = this->velocity[1] - transform->fields.motion[1];
    motion_z = this->velocity[2] - transform->fields.motion[2];
    const PathDefinition *definition = (this->path_definition);
    if (definition != 0)
    {
        const PathTransform *captured_path_transform = (definition->transform);
        if (captured_path_transform != 0 && (this->flags & 0x2000) == 0)
        {
            path_transform = (const PathTransform *)((PathTransform *)(captured_path_transform));
            motion_x += path_transform->displacement[0];
            motion_y += path_transform->displacement[1];
            motion_z += path_transform->displacement[2];
        }
    }
    for (shape = 0; shape < 4; ++shape)
    {
        sphere = &geometry[shape];
        radius = sphere->radius;
        if (radius == 0xFFFFFFFF)
            continue;
        value = (uint32)(((sint64)(sint32)radius * 4014) >> 12);
        radius_square = (uint32)(((sint64)(sint32)value * (sint32)value) >> 12);
        margin = (uint32)(((sint64)(sint32)radius * 3686) >> 12);
        x = sphere->position[0] - transform->fields.position[0];
        end.vx = x;
        y = sphere->position[1] - transform->fields.position[1];
        end.vy = y;
        z = sphere->position[2] - transform->fields.position[2];
        end.vz = z;
        PushMatrix();
        for (i = 0; i < 5; ++i)
            xport_gte_write_control(i, transform->words[1 + i]);
        ApplyRotMatrixLV(&end, &start);
        x = (uint32)(start.vx >> 5);
        y = (uint32)(start.vy >> 5);
        z = (uint32)(start.vz >> 5);
        start.vx = x;
        start.vy = y;
        start.vz = z;
        x = (uint32)end.vx + motion_x;
        y = (uint32)end.vy + motion_y;
        z = (uint32)end.vz + motion_z;
        point.vx = x;
        point.vy = y;
        point.vz = z;
        ApplyRotMatrixLV(&point, &end);
        PopMatrix();
        x = (uint32)(end.vx >> 5);
        y = (uint32)(end.vy >> 5);
        z = (uint32)(end.vz >> 5);
        end.vx = x;
        end.vy = y;
        end.vz = z;
        for (face_index = 0; face_index < (uint32)face_count; ++face_index)
        {
            face = face_base + face_index;
            polygon = face;
            if (((uint16)polygon->normal.components.depth_bias & 1) == 0)
                continue;
            vertex = vertex_base + polygon->vertex_indices[3];
            vx = vertex->position.vx;
            vy = vertex->position.vy;
            vz = vertex->position.vz;
            sx = (uint32)start.vx;
            sy = (uint32)start.vy;
            sz = (uint32)start.vz;
            dx = sx - vx;
            dy = sy - vy;
            dz = sz - vz;
            nx = (uint32)(sint32)polygon->normal.vector.vx;
            ny = (uint32)(sint32)polygon->normal.vector.vy;
            nz = (uint32)(sint32)polygon->normal.vector.vz;
            dot = (uint32)((sint32)(dx * nx + dy * ny + dz * nz) >> 7);
            if ((sint32)(dot + margin) <= 0)
                continue;
            ex = (uint32)end.vx;
            ey = (uint32)end.vy;
            ez = (uint32)end.vz;
            dx = ex - vx;
            dy = ey - vy;
            dz = ez - vz;
            nx = (uint32)(sint32)polygon->normal.vector.vx;
            ny = (uint32)(sint32)polygon->normal.vector.vy;
            nz = (uint32)(sint32)polygon->normal.vector.vz;
            end_distance = (uint32)((sint32)(dx * nx + dy * ny + dz * nz) >> 7) - radius;
            start_distance = dot - radius;
            if ((sint32)end_distance >= 0)
                continue;
            if ((sint32)start_distance > 0)
            {
                value = start_distance - end_distance;
                if (value == 0)
                    xport_mips_break(7);
                reciprocal = value == 0 ? -1 : 0x40000000 / (sint32)value;
                fraction = (uint32)(((sint64)(sint32)start_distance * reciprocal) >> 18);
                fixed_math_result = fraction;
                dx = ex - sx;
                dy = ey - sy;
                dz = ez - sz;
                point.vx = dx;
                point.vy = dy;
                point.vz = dz;
                if ((sint32)*fraction_limit < (sint32)fraction)
                    continue;
                px = (uint32)(((sint64)(sint32)dx * (sint32)fraction) >> 12);
                py = (uint32)(((sint64)(sint32)dy * (sint32)fraction) >> 12);
                pz = (uint32)(((sint64)(sint32)dz * (sint32)fraction) >> 12);
                point.vx = px;
                point.vy = py;
                point.vz = pz;
                px += sx;
                py += sy;
                pz += sz;
                point.vx = px;
                point.vy = py;
                point.vz = pz;
                edge_radius = radius_square;
            }
            else
            {
                if ((sint32)end_distance >= (sint32)start_distance)
                    continue;
                dot = start_distance + radius;
                if ((sint32)dot < 0)
                {
                    angle = AcosAngle((sint32)(0 - dot), (sint32)radius);
                    angle = SinQ14((sint16)(1024 - (uint32)angle)) >> 2;
                    value = (uint32)(((sint64)(sint32)radius * angle) >> 12);
                    edge_radius = (uint32)(((sint64)(sint32)value * (sint32)value) >> 12);
                }
                else
                    edge_radius = radius_square;
                px = (uint32)start.vx;
                py = (uint32)start.vy;
                pz = (uint32)start.vz;
                point.vx = px;
                point.vy = py;
                point.vz = pz;
                fraction = 0;
            }
            for (edge = 0; edge < 4; ++edge)
            {
                x = vx;
                y = vy;
                z = vz;
                vertex = vertex_base + polygon->vertex_indices[edge];
                vx = vertex->position.vx;
                vy = vertex->position.vy;
                vz = vertex->position.vz;
                dx = x - vx;
                dy = y - vy;
                dz = z - vz;
                nx = (uint32)(sint32)polygon->normal.vector.vx;
                ny = (uint32)(sint32)polygon->normal.vector.vy;
                nz = (uint32)(sint32)polygon->normal.vector.vz;
                cx = (uint32)((sint32)(ny * dz - dy * nz) >> 7);
                cy = (uint32)((sint32)(nz * dx - dz * nx) >> 7);
                cz = (uint32)((sint32)(nx * dy - dx * ny) >> 7);
                dx = ((uint32)point.vx - vx) << 5;
                dy = ((uint32)point.vy - vy) << 5;
                dz = ((uint32)point.vz - vz) << 5;
                dot = (uint32)(((sint64)(sint32)cx * (sint32)dx) >> 12);
                dot += (uint32)(((sint64)(sint32)cy * (sint32)dy) >> 12);
                dot += (uint32)(((sint64)(sint32)cz * (sint32)dz) >> 12);
                if ((sint32)dot >= 0)
                {
                    if ((sint32)dot > 0x200000)
                        break;
                    edge_square = (uint32)(((sint64)(sint32)dot * (sint32)dot) >> 12);
                    length_square = (uint32)(((sint64)(sint32)cx * (sint32)cx) >> 12);
                    length_square += (uint32)(((sint64)(sint32)cy * (sint32)cy) >> 12);
                    length_square += (uint32)(((sint64)(sint32)cz * (sint32)cz) >> 12);
                    bound = (uint32)(((sint64)(sint32)edge_radius * (sint32)length_square) >> 12);
                    if (bound < edge_square)
                        break;
                }
            }
            if (edge == 4)
            {
                *fraction_limit = fraction;
                selected_face = face;
                selected_sphere = sphere;
            }
        }
    }
    if (selected_face == 0)
        result = -1;
    else
    {
        value = (uint32)(selected_sphere - geometry) + 1;
        x = (uint32)((sint32)((uint32)((const uint8 *)selected_face - (const uint8 *)face_base) * 0xAAAAAAAB) >> 3);
        result = (sint32)(x | (value << 12));
    }
    return result;
}

uint32 AnimatedObject::FindObjectGeometryCollision()
{
    SceneNode *node, *selected = 0;

    const SceneSection *section;
    FUNCTION_MARKER(0x80039EA4, "SLUS_005.78");
    CollisionHandler collision_callback = 0;
    const CollisionSphere *geometry;
    const MeshGeometryNode *mesh, *position_source;
    const AnimatedObject *geometry_object;
    const CollisionTransform *cached_transform;
    uint32 fraction_limit;
    uint32 mask, index, destination, x, y, z, radius, limit, value, result;
    sint32 kind, hit, selection;
    kind = (sint8)this->metadata126[0];
    result = 4;
    if (kind == 1)
        goto search;
    if (kind == 3)
    {
        if (this->animation_binding.tracks[0] != -2)
            goto search;
        value = this->state;
        if (value == 23 || value == 42)
            goto search;
    }
    if (kind != 4)
        return result;
    collision_callback = object_operations.collisions[4];
    result = (this->*collision_callback)(0, 0) << 24;
    if (result == 0)
        return result;
search:
    geometry = this->spheres;
    if ((sint8)this->metadata126[0] == 1)
    {
        geometry = this->BuildCharacterGeometry();
        DisableCharacterSupportFace();
    }
    mask = (this->flags & 0x200) != 0 ? 0x100000 : 0x100200;
    section = scene_stream.active_section;
    x = psx_identity_rotation_words[0];
    y = psx_identity_rotation_words[1];
    z = psx_identity_rotation_words[2];
    collision_transform.words[1] = x;
    collision_transform.words[2] = y;
    collision_transform.words[3] = z;
    value = psx_identity_rotation_words[3];
    x = (uint32)psx_identity_matrix.m[2][2];
    collision_transform.words[4] = value;
    collision_transform.fields.rotation[8] = x;
    collision_transform.fields.motion[2] = 0;
    collision_transform.fields.motion[1] = 0;
    collision_transform.fields.motion[0] = 0;
    collision_transform.fields.first_face = 0;
    node = section->object_heads[2];
    value = this->path_motion_scale;
    selection = -1;
    mask |= 0x2000;
    fraction_limit = value;
    for (; node != 0; node = mesh->node.links.registry_next)
    {
        mesh = (const MeshGeometryNode *)node;
        if ((mesh->flags & mask) != 0x100000 || (mesh->node.flags & 1) != 0)
            continue;
        if (mesh->vertex_count > 0)
        {
            x = mesh->position[0];
            y = mesh->position[1];
            z = mesh->position[2];
        }
        else
        {
            position_source = (((MeshGeometryNode *)(mesh->source_mesh)));
            x = position_source->position[0];
            y = position_source->position[1];
            z = position_source->position[2];
        }
        collision_transform.fields.position[0] = x;
        collision_transform.fields.position[1] = y;
        collision_transform.fields.position[2] = z;
        radius = mesh->node.state_flags << 5;
        limit = (radius + 61440) << 1;
        value = this->world_position[0];
        value = value - mesh->position[0] + radius + 61440;
        if (value > limit)
            continue;
        value = this->world_position[1];
        value = value - mesh->position[1] + radius + 61440;
        if (value > limit)
            continue;
        value = this->world_position[2];
        value = value - mesh->position[2] + radius + 61440;
        if (value > limit)
            continue;
        if ((this->TestSweptObjectBounds(mesh->position, this->velocity, radius) & 0xFF) == 0)
            continue;
        collision_transform.fields.face_count = (uint16)mesh->polygon_count;
        hit = this->FindSphereFaceCollision(geometry, node, &collision_transform, &fraction_limit);
        if (hit >= 0)
        {
            selection = (sint32)((uint32)hit & 0xFFFF);
            selected = node;
        }
    }
    section = scene_stream.active_section;
    node = section->object_heads[0];
    mask &= 0xFFFFDFFF;
    for (; node != 0; node = geometry_object->node.links.registry_next)
    {
        geometry_object = (const AnimatedObject *)node;
        if ((geometry_object->node.flags & 0x81) != 0 || (geometry_object->flags & mask) != 0x100000)
            continue;
        if ((sint8)this->metadata126[0] == 4 && node == (this->node.links.first_child))
            continue;
        radius = geometry_object->metadata_words72[0] + 61440;
        limit = radius << 1;
        value = this->world_position[0];
        value = value - geometry_object->world_position[0] + radius;
        if (value > limit)
            continue;
        value = this->world_position[1];
        value = value - geometry_object->world_position[1] + radius;
        if (value > limit)
            continue;
        value = this->world_position[2];
        value = value - geometry_object->world_position[2] + radius;
        if (value > limit)
            continue;
        for (index = 0; (sint32)index < geometry_object->collision_transform_count; ++index)
        {
            cached_transform = geometry_object->collision_transforms;
            cached_transform = ((const CollisionTransform *)(cached_transform)) + index;
            for (destination = 0; destination != 12; destination += 4)
            {
                value = cached_transform->words[destination];
                x = cached_transform->words[destination + 1];
                y = cached_transform->words[destination + 2];
                z = cached_transform->words[destination + 3];
                collision_transform.words[destination] = value;
                collision_transform.words[destination + 1] = x;
                collision_transform.words[destination + 2] = y;
                collision_transform.words[destination + 3] = z;
            }
            hit = this->FindSphereFaceCollision(geometry, node, &collision_transform, &fraction_limit);
            if (hit >= 0)
            {
                selected = node;
                value = (uint32)hit + (uint32)(sint32)collision_transform.fields.first_face;
                selection = (sint32)((index << 16) | (value & 0xFFFF));
            }
        }
    }
    if ((sint8)this->metadata126[0] == 1)
        RestoreCharacterSupportFace();
    result = 0x7FFF0000;
    if (selection >= 0)
    {
        value = fraction_limit;
        collision_contact.selection = (uint32)selection;
        collision_contact.flags = 0x7FFFF000;
        collision_contact.fraction = value;
        collision_callback = object_operations.collisions[(sint8)this->metadata126[0]];
        result = (this->*collision_callback)(selected, &collision_contact) << 24;
        if (result != 0)
        {
            value = fraction_limit;
            mask = this->flags;
            this->path_motion_scale = value;
            result = 0xFFFFBFFF;
            this->flags = mask & result;
        }
    }
    return result;
}

uint32 AnimatedObject::GetCharacterSphereEnds(uint32 sphere, uint8 *first, uint8 *last_output) const
{
    FUNCTION_MARKER(0x8001C69C, "SLUS_005.78");
    uint32 result, last = 0;
    if ((sint8)this->metadata126[0] != 1 || character_geometry.local[0].position[2] != 0)
    {
        *last_output = 1;
        *first = 1;
        return 1;
    }
    *first = sphere == 1;
    result = 4;
    if (sphere == 4)
        last = 1;
    else
    {
        last = character_geometry.world[sphere].radius;
        result = 0xFFFFFFFF;
        last = last == result;
    }
    *last_output = last;
    return result;
}

uint32 DisableCharacterSupportFace(void)
{
    FUNCTION_MARKER(0x8001ECEC, "SLUS_005.78");
    uint32 result = character_controller.flags & 0x18040;
    uint32 index;
    MeshFace *face;
    if (result != 0)
    {
        const SceneNode *owner = character_controller.support.owner;
        MeshFace *polygons = ((const MeshGeometryNode *)owner)->polygons;
        index = character_controller.support.contact_word & 0xFFFF;
        face = ((MeshFace *)(polygons)) + index;
        result = (uint16)face->normal.components.depth_bias & 0xFFFE;
        face->normal.components.depth_bias = result;
    }
    return result;
}

uint32 RestoreCharacterSupportFace(void)
{
    FUNCTION_MARKER(0x8001ED4C, "SLUS_005.78");
    uint32 result = character_controller.flags & 0x18040;
    uint32 index;
    MeshFace *face;
    if (result != 0)
    {
        const SceneNode *owner = character_controller.support.owner;
        MeshFace *polygons = ((const MeshGeometryNode *)owner)->polygons;
        index = character_controller.support.contact_word & 0xFFFF;
        face = ((MeshFace *)(polygons)) + index;
        result = (uint16)face->normal.components.depth_bias | 1;
        face->normal.components.depth_bias = result;
    }
    return result;
}

uint32 AnimatedObject::TestSweptObjectBounds(const uint32 *position, const uint32 *velocity, uint32 radius) const
{
    FUNCTION_MARKER(0x80039404, "SLUS_005.78");
    uint32 combined, radius_square, start_square, velocity_square, end_square, x, y, z, vx, vy, vz, scaled, difference, left, right;
    combined = radius + this->metadata_words72[0];
    radius_square = (uint32)(((sint64)(sint32)combined * (sint32)combined) >> 12);
    x = position[0] - this->world_position[0];
    y = position[1] - this->world_position[1];
    z = position[2] - this->world_position[2];
    x = (uint32)((sint32)x >> 6);
    y = (uint32)((sint32)y >> 6);
    z = (uint32)((sint32)z >> 6);
    start_square = x * x + y * y + z * z;
    if ((sint32)radius_square >= (sint32)start_square)
        return 1;
    vx = velocity[0];
    x = (uint32)((sint32)vx >> 6);
    vy = velocity[1];
    y = (uint32)((sint32)vy >> 6);
    vz = velocity[2];
    z = (uint32)((sint32)vz >> 6);
    velocity_square = x * x + y * y + z * z;
    if (velocity_square == 0)
        return 0;
    x = this->world_position[0] + vx;
    y = this->world_position[1] + velocity[1];
    z = this->world_position[2] + velocity[2];
    x = position[0] - x;
    y = position[1] - y;
    z = position[2] - z;
    x = (uint32)((sint32)x >> 6);
    y = (uint32)((sint32)y >> 6);
    z = (uint32)((sint32)z >> 6);
    end_square = x * x + y * y + z * z;
    if ((sint32)radius_square >= (sint32)end_square)
        return 1;
    difference = end_square - velocity_square - start_square;
    scaled = velocity_square << 2;
    right = (uint32)(((sint64)(sint32)start_square * (sint32)scaled) >> 12);
    right -= (uint32)(((sint64)(sint32)difference * (sint32)difference) >> 12);
    left = (uint32)(((sint64)(sint32)radius_square * (sint32)scaled) >> 12);
    return (sint32)left >= (sint32)right;
}

uint32 AnimatedObject::AcceptVerticalFaceContact(const CollisionTransform *descriptor, uint32 selection) const
{
    const PathTransform *path_transform;
    FUNCTION_MARKER(0x8001C710, "SLUS_005.78");
    const PathDefinition *definition;
    uint32 dx, dy, dz, mode, upper, lower;
    dx = this->velocity[0] - descriptor->fields.motion[0];
    dy = this->velocity[1] - descriptor->fields.motion[1];
    dz = this->velocity[2] - descriptor->fields.motion[2];
    definition = (this->path_definition);
    if (definition != 0)
    {
        const PathTransform *captured_path_transform = (definition->transform);
        if (captured_path_transform != 0 && (this->flags & 0x2000) == 0)
        {
            path_transform = (const PathTransform *)((PathTransform *)(captured_path_transform));
            dx += path_transform->displacement[0];
            dy += path_transform->displacement[1];
            dz += path_transform->displacement[2];
        }
    }
    if (dx != 0 || dz != 0)
        return 0;
    mode = (selection & 0x7000) >> 12;
    if ((sint8)this->metadata126[0] == 1 && character_geometry.local[0].position[2] == 0)
    {
        lower = mode == 1;
        upper = mode == 4 || character_geometry.world[mode].radius == 0xFFFFFFFF;
    }
    else
    {
        lower = 1;
        upper = 1;
    }
    if ((lower == 0 || (sint32)dy > 0) && upper == 0)
        return 1;
    if ((sint32)dy < 0 && lower == 0)
        return 1;
    return 0;
}

const MeshFace *SelectCollisionFace(const SceneNode *object, uint32 selection, CollisionTransform *descriptor)
{
    FUNCTION_MARKER(0x8001C544, "SLUS_005.78");
    uint32 destination, x, y, z, fourth, value, index;
    const CollisionTransform *cached_transform;
    const MeshGeometryNode *mesh = object != 0 ? (const MeshGeometryNode *)object : (const MeshGeometryNode *)0;
    const MeshGeometryNode *position_source;
    if (descriptor != 0)
    {
        if (mesh->node.kind == 1)
        {
            x = psx_identity_rotation_words[0];
            x = psx_identity_rotation_words[0];
            y = psx_identity_rotation_words[1];
            y = psx_identity_rotation_words[1];
            z = psx_identity_rotation_words[2];
            z = psx_identity_rotation_words[2];
            for (index = 0; index < 3; ++index)
            {
                value = index == 0 ? x : index == 1 ? y : z;
                descriptor->words[1 + index] = value;
            }
            value = psx_identity_rotation_words[3];
            value = psx_identity_rotation_words[3];
            descriptor->words[4] = value;
            descriptor->fields.rotation[8] = psx_identity_matrix.m[2][2];
            if (mesh->vertex_count > 0)
            {
                x = mesh->position[0];
                y = mesh->position[1];
                z = mesh->position[2];
            }
            else
            {
                position_source = (((MeshGeometryNode *)(mesh->source_mesh)));
                x = position_source->position[0];
                y = position_source->position[1];
                z = position_source->position[2];
            }
            descriptor->fields.position[0] = x;
            descriptor->fields.position[1] = y;
            descriptor->fields.position[2] = z;
            descriptor->fields.motion[2] = 0;
            descriptor->fields.motion[1] = 0;
            descriptor->fields.motion[0] = 0;
        }
        else
        {
            const AnimatedObject *geometry_object = (const AnimatedObject *)object;
            cached_transform = geometry_object->collision_transforms;
            cached_transform = ((const CollisionTransform *)(cached_transform)) + (selection >> 16);
            destination = 0;
            do
            {
                x = cached_transform->words[destination];
                y = cached_transform->words[destination + 1];
                z = cached_transform->words[destination + 2];
                fourth = cached_transform->words[destination + 3];
                descriptor->words[destination] = x;
                descriptor->words[destination + 1] = y;
                descriptor->words[destination + 2] = z;
                descriptor->words[destination + 3] = fourth;
                destination += 4;
            } while (destination != 12);
        }
        descriptor->fields.face_count = 1;
        descriptor->fields.first_face = selection;
    }
    uint32 face_index = selection & 0xFFFF;
    if (mesh->polygons == 0 && face_index == 0)
        return 0;
    return ((MeshFace *)(mesh->polygons)) + face_index;
}

uint32 SweepSphereAgainstObject(const CollisionSphere *bounds, const SceneNode *object, const uint32 *delta, uint32 *fraction, const CollisionSphere *geometry)
{
    FUNCTION_MARKER(0x80038990, "SLUS_005.78");
    SweepVector start, end, relative;
    uint32 category, hx, hy, hz, radius, value, old, reciprocal;
    uint32 sx, sy, sz, ex, ey, ez, dx, dy, dz, lower_x, upper_x, lower_y, upper_y, lower_z, upper_z;
    uint32 enter_x, exit_x, enter_y, exit_y, enter_z, exit_z, time, leave, side, best, nx, ny, nz, dot, i, result = 0;
    const SceneTrigger *trigger = 0;
    const sint16 *normal;
    category = object->kind;
    if (category == 2)
    {
        trigger = (const SceneTrigger *)object;
        hx = trigger->extent[0] << 5;
        hy = trigger->extent[1] << 5;
        hz = trigger->extent[2] << 5;
        relative.words[0] = bounds->position[0] - (uint32)trigger->transform.t[0];
        relative.words[1] = bounds->position[1] - (uint32)trigger->transform.t[1];
        relative.words[2] = bounds->position[2] - (uint32)trigger->transform.t[2];
        PushMatrix();
        for (i = 0; i < 5; ++i)
            xport_gte_write_control(i, xport_load_le32((const uint8 *)&trigger->transform + i * 4));
        ApplyRotMatrixLV(&relative.vector, &start.vector);
        relative.words[0] = relative.words[0] + delta[0];
        relative.words[1] = relative.words[1] + delta[1];
        relative.words[2] = relative.words[2] + delta[2];
        ApplyRotMatrixLV(&relative.vector, &end.vector);
        PopMatrix();
    }
    else
    {
        if (category == 8)
        {
            const PositionedBillboardNode *billboard = (const PositionedBillboardNode *)object;
            sx = bounds->position[0] - billboard->world_position[0];
            start.words[0] = sx;
            sy = bounds->position[1] - billboard->world_position[1];
            start.words[1] = sy;
            sz = bounds->position[2] - billboard->world_position[2];
            start.words[2] = sz;
            hz = (uint32)billboard->billboard.extent;
        }
        else if (category == 0)
        {
            sx = bounds->position[0] - geometry->position[0];
            start.words[0] = sx;
            sy = bounds->position[1] - geometry->position[1];
            start.words[1] = sy;
            sz = bounds->position[2] - geometry->position[2];
            start.words[2] = sz;
            hz = geometry->radius;
        }
        else
            goto done;
        hy = hz;
        hx = hz;
        end.words[0] = sx + delta[0];
        end.words[1] = sy + delta[1];
        end.words[2] = sz + delta[2];
    }
    radius = bounds->radius;
    sx = start.words[0];
    ex = end.words[0];
    value = (0 - hx) - (sx - radius);
    lower_x = value - (radius << 1);
    upper_x = value + (hx << 1);
    dx = ex - sx;
    if (!((sint32)lower_x > 0 ? (sint32)dx >= (sint32)lower_x : (sint32)upper_x >= 0))
    {
        if ((sint32)upper_x >= 0 || (sint32)upper_x < (sint32)dx)
            goto done;
    }
    sy = start.words[1];
    ey = end.words[1];
    value = (0 - hy) - (sy - radius);
    lower_y = value - (radius << 1);
    upper_y = value + (hy << 1);
    dy = ey - sy;
    if (!((sint32)lower_y > 0 ? (sint32)dy >= (sint32)lower_y : (sint32)upper_y >= 0))
    {
        if ((sint32)upper_y >= 0 || (sint32)upper_y < (sint32)dy)
            goto done;
    }
    sz = start.words[2];
    ez = end.words[2];
    value = (0 - hz) - (sz - radius);
    lower_z = value - (radius << 1);
    upper_z = value + (hz << 1);
    dz = ez - sz;
    if (!((sint32)lower_z > 0 ? (sint32)dz >= (sint32)lower_z : (sint32)upper_z >= 0))
    {
        if ((sint32)upper_z >= 0 || (sint32)upper_z < (sint32)dz)
            goto done;
    }
    if (dx + 256 < 0x201)
    {
        enter_x = 0;
        exit_x = 4096;
    }
    else
    {
        old = lower_x;
        if ((sint32)upper_x < 0)
        {
            lower_x = 0 - upper_x;
            upper_x = 0 - old;
        }
        else if ((sint32)lower_x <= 0)
        {
            if ((sint32)dx < 0)
                upper_x = 0 - lower_x;
            lower_x = 0;
        }
        if ((sint32)dx < 0)
            dx = 0 - dx;
        if ((sint32)dx < (sint32)upper_x)
            upper_x = dx;
        if (dx == 0)
            xport_mips_break(7);
        reciprocal = dx == 0 ? 0xFFFFFFFF : (uint32)(0x1000000 / (sint32)dx);
        enter_x = (uint32)(((sint64)(sint32)lower_x * (sint32)reciprocal) >> 12);
        exit_x = (uint32)(((sint64)(sint32)upper_x * (sint32)reciprocal) >> 12);
    }
    time = enter_x;
    leave = exit_x;
    if (dy + 256 < 0x201)
    {
        enter_y = 0;
    }
    else
    {
        old = lower_y;
        if ((sint32)upper_y < 0)
        {
            lower_y = 0 - upper_y;
            upper_y = 0 - old;
        }
        else if ((sint32)lower_y <= 0)
        {
            if ((sint32)dy < 0)
                upper_y = 0 - lower_y;
            lower_y = 0;
        }
        if ((sint32)dy < 0)
            dy = 0 - dy;
        if ((sint32)dy < (sint32)upper_y)
            upper_y = dy;
        if (dy == 0)
            xport_mips_break(7);
        reciprocal = dy == 0 ? 0xFFFFFFFF : (uint32)(0x1000000 / (sint32)dy);
        enter_y = (uint32)(((sint64)(sint32)lower_y * (sint32)reciprocal) >> 12);
        exit_y = (uint32)(((sint64)(sint32)upper_y * (sint32)reciprocal) >> 12);
        if ((sint32)time < (sint32)enter_y)
            time = enter_y;
        if ((sint32)exit_y < (sint32)leave)
            leave = exit_y;
    }
    if (dz + 256 < 0x201)
    {
        enter_z = 0;
    }
    else
    {
        old = lower_z;
        if ((sint32)upper_z < 0)
        {
            lower_z = 0 - upper_z;
            upper_z = 0 - old;
        }
        else if ((sint32)lower_z <= 0)
        {
            if ((sint32)dz < 0)
                upper_z = 0 - lower_z;
            lower_z = 0;
        }
        if ((sint32)dz < 0)
            dz = 0 - dz;
        if ((sint32)dz < (sint32)upper_z)
            upper_z = dz;
        if (dz == 0)
            xport_mips_break(7);
        reciprocal = dz == 0 ? 0xFFFFFFFF : (uint32)(0x1000000 / (sint32)dz);
        enter_z = (uint32)(((sint64)(sint32)lower_z * (sint32)reciprocal) >> 12);
        exit_z = (uint32)(((sint64)(sint32)upper_z * (sint32)reciprocal) >> 12);
        if ((sint32)time < (sint32)enter_z)
            time = enter_z;
        if ((sint32)exit_z < (sint32)leave)
            leave = exit_z;
    }
    if ((sint32)leave < (sint32)time)
        goto done;
    result = 1;
    if (object->kind != 2)
        goto hit;
    value = (uint32)(sint8)(object->state_flags >> 8);
    if (value != 9 && value != 6)
        goto hit;
    if (time != 0)
    {
        if (time == enter_y)
            side = (sint32)sy < 0 ? 2 : 3;
        else if (time == enter_z)
            side = (sint32)sz < 0 ? 4 : 5;
        else
            side = (sint32)sx >= 0;
        goto side_hit;
    }
    value = (sint32)sx < 0 ? sx + hx : sx - hx;
    end.words[0] = value;
    if ((sint32)value < 0)
        end.words[0] = 0 - value;
    value = (sint32)sy < 0 ? sy + hy : sy - hy;
    end.words[1] = value;
    if ((sint32)value < 0)
        end.words[1] = 0 - value;
    value = (sint32)sz < 0 ? sz + hz : sz - hz;
    end.words[2] = value;
    if ((sint32)value < 0)
        end.words[2] = 0 - value;
    value = end.words[1];
    if ((sint32)end.words[0] >= (sint32)value && (sint32)end.words[2] >= (sint32)value)
        side = (sint32)sy < 0 ? 2 : 3;
    else
    {
        value = end.words[2];
        if ((sint32)end.words[0] >= (sint32)value && (sint32)end.words[1] >= (sint32)value)
            side = (sint32)sz < 0 ? 4 : 5;
        else
            side = (sint32)sx >= 0;
    }
    if (delta[0] == 0 && delta[1] == 0 && delta[2] == 0)
        goto side_hit;
    best = enter_z;
    for (i = 0; i < 6; ++i)
    {
        normal = trigger->transform.m[i >> 1];
        nx = (uint32)(sint32)normal[0];
        ny = (uint32)(sint32)normal[1];
        nz = (uint32)(sint32)normal[2];
        if ((i & 1) == 0)
        {
            nx = 0 - nx;
            ny = 0 - ny;
            nz = 0 - nz;
        }
        dot = (uint32)(((sint64)(sint32)delta[0] * (sint32)nx) >> 12);
        dot += (uint32)(((sint64)(sint32)delta[1] * (sint32)ny) >> 12);
        dot += (uint32)(((sint64)(sint32)delta[2] * (sint32)nz) >> 12);
        if ((sint32)dot < (sint32)time)
        {
            time = dot;
            best = i;
        }
    }
    if (best != side)
    {
        uint32 row = (uint32)((sint32)best >> 1);
        normal = row < 3 ? trigger->transform.m[row] : (const sint16 *)((const uint8 *)trigger + 16 + row * 6);
        nx = (uint32)(sint32)normal[0];
        ny = (uint32)(sint32)normal[1];
        nz = (uint32)(sint32)normal[2];
        if ((best & 1) == 0)
        {
            nx = 0 - nx;
            ny = 0 - ny;
            nz = 0 - nz;
        }
        if (ny + 128 >= 0x101)
        {
            normal = trigger->transform.m[side >> 1];
            nx = (uint32)(sint32)normal[0];
            ny = (uint32)(sint32)normal[1];
            nz = (uint32)(sint32)normal[2];
            if ((side & 1) == 0)
            {
                nx = 0 - nx;
                ny = 0 - ny;
                nz = 0 - nz;
            }
            if (ny + 128 < 0x101)
                best = side;
        }
    }
    side = best;
    time = 0;
side_hit:
    result = side + 1;
hit:
    *fraction = time;
done:
    return result;
}

uint32 AnimatedObject::CollideObjectWithTrigger(const SceneTrigger *trigger)
{
    FUNCTION_MARKER(0x80039B20, "SLUS_005.78");
    CollisionHandler collision_callback = 0;
    uint32 delta[3], fraction, selected_index;
    uint8 shape_flags[2];
    uint32 value, flags, radius, first_filter, last_filter, best, side, index, result;
    sint32 kind;
    const SceneTrigger *other = (const SceneTrigger *)(trigger);
    value = this->velocity[0];
    delta[0] = value - other->delta[0];
    value = this->velocity[1];
    delta[1] = value - other->delta[1];
    value = this->velocity[2];
    delta[2] = value - other->delta[2];
    radius = other->radius << 5;
    result = 1;
    if ((this->TestSweptObjectBounds((const uint32 *)other->transform.t, delta, radius) & 0xFF) == 0)
        goto finish_contact;
    kind = (sint8)(other->node.state_flags >> 8);
    first_filter = 0;
    if (kind != 1)
    {
        flags = this->path_flags;
        if ((flags & 2) != 0 || (flags & 0x180) == 0x80)
        {
            last_filter = 1;
            goto select_contact;
        }
        kind = (sint8)(other->node.state_flags >> 8);
    }
    if (kind == 1 || kind == 24 || kind == 6 || ((uint8)(kind - 40) < 16 && ((other->parameters.words[1] >> 12) & 31) == 15))
        first_filter = 1;
    last_filter = 0;
    if (first_filter == 0 && (uint32)(other->node.state_flags >> 8) - 40 < 16)
        last_filter = ((other->parameters.words[1] >> 12) & 31) == 16;
select_contact:
    side = 0xFFFFFFFF;
    best = this->path_motion_scale + 1;
    for (index = 0; index < 4; ++index)
    {
        if (this->spheres[index].radius == 0xFFFFFFFF)
            break;
        this->GetCharacterSphereEnds(index + 1, shape_flags, shape_flags + 1);
        if (first_filter != 0)
        {
            if ((sint8)shape_flags[0] != 0)
                goto test_contact;
        }
        else if (last_filter == 0)
            goto test_contact;
        if (last_filter == 0 || (sint8)shape_flags[1] == 0)
            continue;
    test_contact:
        value = SweepSphereAgainstObject(this->spheres + index, &other->node, delta, &fraction, 0);
        if (value != 0)
        {
            flags = fraction;
            if ((sint32)flags < (sint32)best)
            {
                best = flags;
                side = value;
                selected_index = index;
            }
        }
    }
    if ((sint32)this->path_motion_scale >= (sint32)best)
    {
        collision_contact.selection = side;
        if (side != 0xFFFFFFFF)
        {
            value = selected_index;
            collision_contact.selection |= (value + 1) << 12;
        }
        collision_contact.fraction = best;
        collision_contact.flags = 0;
        kind = (sint8)this->metadata126[0];
        collision_callback = object_operations.collisions[kind];
        if (((this->*collision_callback)(trigger != 0 ? (SceneNode *)&trigger->node : 0, &collision_contact) & 0xFF) != 0)
        {
            flags = this->flags;
            this->path_motion_scale = best;
            this->flags = flags & 0xFFFFBFFF;
        }
    }
    flags = this->path_flags;
    result = flags & 0x80;
    if ((flags & 2) != 0)
    {
        value = gameplay_input.buttons;
        trigger_contact_motion.dir = 0;
        if ((value & 0x1000) != 0)
        {
            result = 1;
            if ((sint32)this->path_motion_scale < (sint32)best)
                trigger_contact_motion.dir = 1;
        }
        else
        {
            result = value & 0x4000;
            if (result != 0)
            {
                result = 0xFFFFFFFF;
                if ((sint32)this->path_motion_scale < (sint32)best)
                    trigger_contact_motion.dir = 0xFFFFFFFF;
            }
        }
    }
    else if (result != 0)
    {
        value = this->path_motion_scale;
        trigger_contact_motion.blocked = 0;
        result = 1;
        if ((sint32)value < (sint32)best)
            trigger_contact_motion.blocked = 1;
    }
finish_contact:
    return result;
}

uint32 AnimatedObject::CollideObjectSpheres(AnimatedObject *other_object)
{
    const PathTransform *path_transform;
    FUNCTION_MARKER(0x80039614, "SLUS_005.78");
    AnimatedObject *other = (AnimatedObject *)(other_object);
    CollisionHandler collision_callback = 0;
    uint32 difference[3], extent;
    const PathDefinition *definition;
    uint32 x, y, z, value, flags, best, result;
    const CollisionSphere *own_shape, *selected_own = 0;
    const CollisionSphere *other_shape, *selected_other = 0;
    uint32 own_index, other_index, selected = 0;
    sint32 kind, other_kind;
    x = other->velocity[0];
    y = other->velocity[1];
    z = other->velocity[2];
    difference[0] = x;
    difference[1] = y;
    difference[2] = z;
    definition = (other->path_definition);
    if (definition != 0)
    {
        const PathTransform *captured_path_transform = (definition->transform);
        if (captured_path_transform != 0 && (other->flags & 0x2000) == 0)
        {
            path_transform = (const PathTransform *)((PathTransform *)(captured_path_transform));
            for (own_index = 0; own_index < 3; ++own_index)
                difference[own_index] += path_transform->displacement[own_index];
        }
    }
    x = this->velocity[0] - difference[0];
    difference[0] = x;
    y = this->velocity[1] - difference[1];
    difference[1] = y;
    z = this->velocity[2] - difference[2];
    difference[2] = z;
    result = this->TestSweptObjectBounds(other->world_position, difference, other->metadata_words72[0]) << 24;
    if (result == 0)
        goto finish;
    best = this->path_motion_scale;
    own_shape = this->spheres;
    for (own_index = 0; own_index < 4; ++own_index, ++own_shape)
    {
        if (own_shape->radius == 0xFFFFFFFF)
            break;
        other_shape = other->spheres;
        for (other_index = 0; other_index < 4; ++other_index, ++other_shape)
        {
            if (other_shape->radius == 0xFFFFFFFF)
                break;
            if (SweepSphereAgainstObject(own_shape, &other->node, difference, &extent, other_shape) != 0)
            {
                value = extent;
                if ((sint32)value <= (sint32)best && (selected == 0 || (sint32)value < (sint32)best))
                {
                    best = value;
                    selected_other = other_shape;
                    selected_own = own_shape;
                    selected = 1;
                }
            }
        }
    }
    result = 1;
    if (selected == 0 || (sint32)this->path_motion_scale < (sint32)best)
        goto finish;
    kind = (sint8)this->metadata126[0];
    if (kind == 1)
    {
        other_kind = (sint8)other->metadata126[0];
        if (other_kind == 2)
        {
            collision_contact.other_selection = 0;
            if (other->ReadObjectImpulseScale() != 0)
                best = 4096;
            if ((this->flags & 0x2400) != 0 || (this->path_flags & 2) != 0)
            {
                x = this->world_position[1];
                y = this->metadata_words72[1];
                value = this->velocity[1];
                x -= y;
                value = (uint32)(((sint64)(sint32)value * (sint32)best) >> 12);
                y = other->velocity[1];
                x += value + 6144;
                y = (uint32)(((sint64)(sint32)y * (sint32)best) >> 12);
                y += other->world_position[1];
                if ((sint32)x >= (sint32)y)
                    collision_contact.other_selection = 4096;
            }
            goto prepare_callback;
        }
    }
    else
        other_kind = (sint8)other->metadata126[0];
    if (other_kind == 1 && (sint8)this->metadata126[0] == 2)
    {
        collision_contact.selection = 0;
        if ((other->flags & 0x2400) != 0 || (other->path_flags & 2) != 0)
        {
            x = other->world_position[1];
            y = other->metadata_words72[1];
            value = other->velocity[1];
            x -= y;
            value = (uint32)(((sint64)(sint32)value * (sint32)best) >> 12);
            y = this->velocity[1];
            x += value + 6144;
            y = (uint32)(((sint64)(sint32)y * (sint32)best) >> 12);
            y += this->world_position[1];
            if ((sint32)x >= (sint32)y)
                collision_contact.selection = 4096;
        }
    }
prepare_callback:
    collision_contact.fraction = best;
    kind = (sint8)this->metadata126[0];
    if (kind == 1)
        value = selected_other->radius & 7;
    else if (kind == 2)
        value = selected_own->radius & 7;
    else
        value = 0;
    collision_contact.flags = value;
    kind = (sint8)this->metadata126[0];
    collision_callback = object_operations.collisions[kind];
    result = (this->*collision_callback)((SceneNode *)other_object, &collision_contact) << 24;
    if (result != 0)
    {
        flags = this->flags;
        this->path_motion_scale = best;
        result = flags & 0xFFFFBFFF;
        this->flags = result;
    }
finish:
    return result;
}

void AnimatedObject::UpdateObjectCollisions()
{
    const SceneSection *section;
    const PathTransform *path_transform;
    FUNCTION_MARKER(0x8003A930, "SLUS_005.78");
    SceneNode *other_node;
    AnimatedObject *other;
    CollisionHandler collision_callback = 0;
    const PathDefinition *parent, *restore_definition;
    uint32 value, result, radius, limit;
    const PathTransform *captured_path_transform;
    this->FindObjectGeometryCollision();
    parent = (this->path_definition);
    if (parent != 0)
    {
        captured_path_transform = ((((const PathDefinition *)(parent)))->transform);
        if (captured_path_transform != 0 && (this->flags & 0x2000) == 0)
        {
            path_transform = (const PathTransform *)((PathTransform *)(captured_path_transform));
            value = this->velocity[0] + path_transform->displacement[0];
            parent = (this->path_definition);
            this->velocity[0] = value;
            path_transform = (const PathTransform *)(((((const PathDefinition *)(parent)))->transform) != 0 ? ((((const PathDefinition *)(parent)))->transform) : (PathTransform *)0);
            value = this->velocity[1] + path_transform->displacement[1];
            parent = (this->path_definition);
            this->velocity[1] = value;
            path_transform = (const PathTransform *)(((((const PathDefinition *)(parent)))->transform) != 0 ? ((((const PathDefinition *)(parent)))->transform) : (PathTransform *)0);
            value = this->velocity[2] + path_transform->displacement[2];
            this->velocity[2] = value;
        }
    }
    this->ProcessObjectTriggerContacts();
    if ((this->node.flags & 1) != 0)
        goto restore;
    for (other_node = registered_object_head; other_node != 0; other_node = other_node->links.registry_next)
    {
        other = (AnimatedObject *)other_node;
        if (other_node == &this->node || (other_node->flags & 0x81) != 0 || (sint8)other->metadata126[0] == 0)
            continue;
        value = (uint32)this->behavior;
        radius = other->metadata_words72[0] + (value == 30 ? 184320 : 61440);
        limit = radius << 1;
        value = this->world_position[0];
        value = value - other->world_position[0] + radius;
        if (value > limit)
            continue;
        value = this->world_position[1];
        value = value - other->world_position[1] + radius;
        if (value > limit)
            continue;
        value = this->world_position[2];
        value = value - other->world_position[2] + radius;
        if (value > limit)
            continue;
        collision_callback = object_operations.collisions[(sint8)this->metadata126[0]];
        if (((this->*collision_callback)(other_node, 0) & 0xFF) != 0)
            this->CollideObjectSpheres(other);
    }
    if ((sint8)this->metadata126[0] == 4)
    {
        section = scene_stream.active_section;
        for (other_node = section->object_heads[1]; other_node != 0; other_node = other_node->links.registry_next)
        {
            other = (AnimatedObject *)other_node;
            if (other_node == &this->node || (other_node->flags & 0x81) != 0 || (sint8)other->metadata126[0] == 0)
                continue;
            value = this->world_position[0];
            value = value - other->world_position[0] + 81920;
            if (value > 163840)
                continue;
            value = this->world_position[1];
            value = value - other->world_position[1] + 81920;
            if (value > 163840)
                continue;
            value = this->world_position[2];
            value = value - other->world_position[2] + 81920;
            if (value > 163840)
                continue;
            collision_callback = object_operations.collisions[(sint8)this->metadata126[0]];
            if (((this->*collision_callback)(other_node, 0) & 0xFF) != 0)
                this->CollideObjectSpheres(other);
        }
    }
restore:
    restore_definition = (this->path_definition);
    if (restore_definition != 0)
    {
        captured_path_transform = ((restore_definition)->transform);
        if (captured_path_transform != 0)
        {
            result = this->flags & 0x2000;
            if (result == 0)
            {
                path_transform = (const PathTransform *)((PathTransform *)(captured_path_transform));
                result = this->velocity[0] - path_transform->displacement[0];
                parent = (this->path_definition);
                this->velocity[0] = result;
                path_transform = (const PathTransform *)(((((const PathDefinition *)(parent)))->transform) != 0 ? ((((const PathDefinition *)(parent)))->transform) : (PathTransform *)0);
                result = this->velocity[1] - path_transform->displacement[1];
                parent = (this->path_definition);
                this->velocity[1] = result;
                path_transform = (const PathTransform *)(((((const PathDefinition *)(parent)))->transform) != 0 ? ((((const PathDefinition *)(parent)))->transform) : (PathTransform *)0);
                result = this->velocity[2] - path_transform->displacement[2];
                this->velocity[2] = result;
            }
        }
    }
    return;
}

void AnimatedObject::ProcessObjectTriggerContacts()
{
    SceneNode *node;

    FUNCTION_MARKER(0x8003A384, "SLUS_005.78");
    CollisionHandler collision_callback = 0;
    const SceneTrigger *trigger;
    PositionedBillboardNode *billboard;
    const ScenePath *context;
    uint32 velocity[3], extent, kind, radius, limit, value, x, y, z, shape, nearest, result;
    const SceneSection *section;
    kind = (uint32)(sint8)this->metadata126[0];
    result = 4;
    if (kind == 2)
    {
        section = scene_stream.active_section;
        for (node = section->object_heads[5]; node != 0; node = trigger->node.links.registry_next)
        {
            trigger = (const SceneTrigger *)node;
            result = 61440;
            if ((trigger->node.flags & 0x81) != 0)
                continue;
            radius = (trigger->radius << 5) + 61440;
            limit = radius << 1;
            value = this->world_position[0];
            value = value - (uint32)trigger->transform.t[0] + radius;
            if (value > limit)
                continue;
            value = this->world_position[1];
            value = value - (uint32)trigger->transform.t[1] + radius;
            result = value > limit;
            if (result != 0)
                continue;
            value = this->world_position[2];
            value = value - (uint32)trigger->transform.t[2] + radius;
            result = value > limit;
            if (result != 0)
                continue;
            collision_callback = object_operations.collisions[(sint8)this->metadata126[0]];
            result = (this->*collision_callback)((SceneNode *)&trigger->node, 0) << 24;
            if (result != 0)
                result = this->CollideObjectWithTrigger(trigger);
        }
        goto done;
    }
    if (kind == 4)
        goto done;
    section = scene_stream.active_section;
    for (node = section->object_heads[4]; node != 0; node = trigger->node.links.registry_next)
    {
        trigger = (const SceneTrigger *)node;
        if ((trigger->node.flags & 0x85) != 0)
            continue;
        radius = (trigger->radius << 5) + 61440;
        limit = radius << 1;
        value = this->world_position[0];
        value = value - (uint32)trigger->transform.t[0] + radius;
        if (value > limit)
            continue;
        value = this->world_position[1];
        value = value - (uint32)trigger->transform.t[1] + radius;
        if (value > limit)
            continue;
        value = this->world_position[2];
        value = value - (uint32)trigger->transform.t[2] + radius;
        if (value > limit)
            continue;
        collision_callback = object_operations.collisions[(sint8)this->metadata126[0]];
        if (((this->*collision_callback)((SceneNode *)&trigger->node, 0) & 0xFF) != 0)
            this->CollideObjectWithTrigger(trigger);
    }
    kind = (uint32)(sint8)this->metadata126[0];
    context = 0;
    if (kind == 1)
    {
        if (this->behavior == 33)
            context = (this->scene_path);
        else
            context = camera_motion.path;
    }
    else if (kind == 3 && this->animation_binding.tracks[0] == -1)
        context = (this->scene_path);
    if ((this->CanObjectChangePath(0, context) & 0xFF) != 0)
    {
        section = scene_stream.active_section;
        for (node = section->object_heads[6]; node != 0; node = trigger->node.links.registry_next)
        {
            trigger = (const SceneTrigger *)node;
            if ((trigger->node.flags & 0x85) != 0)
                continue;
            radius = (trigger->radius << 5) + 61440;
            limit = radius << 1;
            value = this->spheres[0].position[0];
            value = value - (uint32)trigger->transform.t[0] + radius;
            if (value > limit)
                continue;
            value = this->spheres[0].position[1];
            value = value - (uint32)trigger->transform.t[1] + radius;
            if (value > limit)
                continue;
            value = this->spheres[0].position[2];
            value = value - (uint32)trigger->transform.t[2] + radius;
            if (value > limit)
                continue;
            if ((this->CanObjectChangePath(trigger, context) & 0xFF) != 0)
                this->CollideObjectWithTrigger(trigger);
        }
    }
    kind = (uint32)(sint8)this->metadata126[0];
    result = 19;
    if (kind != 1 && (kind != 3 || this->state != 19))
        goto done;
    section = scene_stream.active_section;
    for (node = section->object_heads[8]; node != 0; node = billboard->billboard.node.links.registry_next)
    {
        billboard = (PositionedBillboardNode *)node;
        if ((billboard->billboard.playback_flags & 0x8000) != 0)
        {
            QueueSceneEvent(billboard->billboard.metadata16 | 0x4000, 4);
            billboard->billboard.playback_flags = billboard->billboard.playback_flags & 0x7FFF;
        }
        result = billboard->billboard.node.flags & 0x85;
        if (result != 0)
            continue;
        value = this->world_position[0];
        value = value - billboard->world_position[0] + 61440;
        result = value > 0x1E000;
        if (result != 0)
            continue;
        value = this->world_position[1];
        value = value - billboard->world_position[1] + 61440;
        result = value > 0x1E000;
        if (result != 0)
            continue;
        value = this->world_position[2];
        value = value - billboard->world_position[2] + 61440;
        result = value > 0x1E000;
        if (result != 0)
            continue;
        collision_callback = object_operations.collisions[(sint8)this->metadata126[0]];
        result = (this->*collision_callback)(&billboard->billboard.node, 0) << 24;
        if (result == 0)
            continue;
        x = this->velocity[0];
        y = this->velocity[1];
        z = this->velocity[2];
        velocity[0] = x;
        velocity[1] = y;
        velocity[2] = z;
        nearest = this->path_motion_scale + 1;
        for (shape = 0; shape < 4; ++shape)
        {
            if (this->spheres[shape].radius == 0xFFFFFFFF)
                break;
            if (SweepSphereAgainstObject(this->spheres + shape, &billboard->billboard.node, velocity, &extent, 0) != 0)
            {
                value = extent;
                if ((sint32)value < (sint32)nearest)
                    nearest = value;
            }
        }
        result = (sint32)this->path_motion_scale < (sint32)nearest;
        if (result != 0)
            continue;
        collision_contact.fraction = nearest;
        collision_contact.flags = 0;
        collision_callback = object_operations.collisions[(sint8)this->metadata126[0]];
        value = (this->*collision_callback)(&billboard->billboard.node, &collision_contact) << 24;
        result = 0xFFFFBFFF;
        if (value != 0)
        {
            value = this->flags;
            this->path_motion_scale = nearest;
            this->flags = value & result;
        }
    }
done:
    return;
}
