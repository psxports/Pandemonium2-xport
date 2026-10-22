#include "scene_lighting.h"
#include "game.h"
#include "psx_gte.h"

SceneLighting scene_lighting = {0};

void SceneLighting::MarkSceneLightingDirty(const SceneLight *light)
{
    FUNCTION_MARKER(0x8002E00C, "SLUS_005.78");
    SceneNode *node;
    AnimatedObject *actor;
    MeshGeometryNode *mesh;
    uint32 mark_all, result;
    const SceneSection *section;
    uint32 flags, radius, threshold, x, y, z, a, b, c;
    mark_all = 0;
    if (light == 0 || (light->event_flags & 2) == 0)
        mark_all = 1;
    section = scene_stream.active_section;
    node = section->object_heads[0];
    if (node != 0)
    {
        do
        {
            actor = (AnimatedObject *)node;
            if (actor->face_count != 0)
            {
                flags = actor->flags;
                if ((flags & 1) == 0)
                {
                    result = flags | 1;
                    if (mark_all == 0)
                    {
                        x = actor->world_pos.x;
                        y = actor->world_pos.y;
                        z = actor->world_pos.z;
                        a = x;
                        b = y;
                        c = z;
                        radius = actor->bounds.radius;
                        x = light->src.pos.x;
                        x -= a;
                        y = light->src.pos.y;
                        y -= b;
                        z = light->src.pos.z - c;
                        radius += light->radius << 5;
                        threshold = mul12((sint32)radius, (sint32)radius);
                        x = (sint32)x >> 6;
                        y = (sint32)y >> 6;
                        z = (sint32)z >> 6;
                        if (x * x + y * y + z * z >= threshold)
                        {
                            x = light->prev_pos.x - a;
                            y = light->prev_pos.y - b;
                            z = light->prev_pos.z - c;
                            x = (sint32)x >> 6;
                            y = (sint32)y >> 6;
                            z = (sint32)z >> 6;
                            if (x * x + y * y + z * z >= threshold)
                                goto next_first;
                        }
                        result = actor->flags | 1;
                    }
                    actor->flags = result;
                }
            }
        next_first:
            node = node->links.registry_next;
        } while (node != 0);
        section = scene_stream.active_section;
    }
    node = section->object_heads[2];
    while (node != 0)
    {
        mesh = (MeshGeometryNode *)node;
        if (mark_all == 0)
        {
            result = mesh->flags & 1;
            if (result != 0)
                goto next_second;
            x = light->src.pos.x;
            x -= (uint32)mesh->pos.x;
            y = light->src.pos.y;
            y -= (uint32)mesh->pos.y;
            z = light->src.pos.z;
            z -= (uint32)mesh->pos.z;
            radius = node->state_flags;
            radius += light->radius;
            threshold = (radius * radius) >> 2;
            x = (sint32)x >> 6;
            y = (sint32)y >> 6;
            z = (sint32)z >> 6;
            result = x * x + y * y + z * z < threshold;
            if (result == 0)
            {
                x = light->prev_pos.x;
                x -= (uint32)mesh->pos.x;
                y = light->prev_pos.y;
                y -= (uint32)mesh->pos.y;
                z = light->prev_pos.z;
                z -= (uint32)mesh->pos.z;
                x = (sint32)x >> 6;
                y = (sint32)y >> 6;
                z = (sint32)z >> 6;
                result = x * x + y * y + z * z < threshold;
                if (result == 0)
                    goto next_second;
            }
        }
        result = mesh->flags | 1;
        mesh->flags = result;
    next_second:
        node = node->links.registry_next;
    }
}

void SceneLighting::UpdateSceneLights()
{
    uint32 result;
    SceneLight *light;
    uint32 updated = 0;
    uint32 flags;
    uint32 kind;
    uint32 x;
    uint32 y;
    uint32 z;
    uint32 count;
    uint32 slot;
    uint8 byte_flags;
    FUNCTION_MARKER(0x8002E344, "SLUS_005.78");
    light = scene_lighting.lights;
    scene_lighting.directional_count = 0;
    while (light != 0)
    {
        flags = light->event_flags;
        if ((flags & 4) != 0)
        {
            kind = light->event_id;
            light->event_flags = flags & 0xFB;
            kind = ((kind >> 12) & 31) + 39;
            if (kind == 47)
                kind = (flags & 8) != 0 ? 42 : 43;
            switch (kind)
            {
                case 42:
                case 53:
                    byte_flags = light->flags;
                    if ((byte_flags & 1) == 0)
                    {
                        flags = light->event_flags;
                        light->flags = byte_flags | 1;
                        light->event_flags = flags | 1;
                    }
                    break;
                case 43:
                case 54:
                    byte_flags = light->flags;
                    if ((byte_flags & 1) != 0)
                    {
                        flags = light->event_flags;
                        light->flags = byte_flags & 0xFE;
                        light->event_flags = flags | 1;
                    }
                    break;
                case 51:
                    byte_flags = light->flags;
                    flags = light->event_flags;
                    light->flags = byte_flags ^ 1;
                    light->event_flags = flags | 1;
                    break;
                default:
                    break;
            }
        }
        uint32 *event_slots[1] = {&light->event_id};
        if ((SceneEvents::ApplySceneEvents(event_slots, 1) << 24) != 0)
            light->event_flags = light->event_flags | 4;
        flags = light->event_flags;
        if ((flags & 1) != 0)
        {
            light->event_flags = flags & 0xFE;
            if (updated == 0)
                SceneLighting::MarkSceneLightingDirty(light);
            if ((light->event_flags & 2) != 0)
            {
                light->src.pos = light->prev_pos;
            }
            else
            {
                updated = 1;
                if ((uint32)light->prev_pos.x + 26754 > 0xD103 || (uint32)light->prev_pos.y + 26754 > 0xD103 || (uint32)light->prev_pos.z + 26754 > 0xD103)
                {
                    for (;;)
                    {
                        x = light->prev_pos.x;
                        z = light->prev_pos.z;
                        light->prev_pos.x = (sint32)x >> 1;
                        y = light->prev_pos.y;
                        x = light->prev_pos.x;
                        z = (sint32)z >> 1;
                        light->prev_pos.z = z;
                        light->prev_pos.y = (sint32)y >> 1;
                        if (x + 26754 > 0xD103)
                            continue;
                        if ((uint32)light->prev_pos.y + 26754 > 0xD103)
                            continue;
                        if (z + 26754 <= 0xD103)
                            break;
                    }
                }
                {
                    VECTOR n_input = {light->prev_pos.x, light->prev_pos.y, light->prev_pos.z, 0};
                    VectorNormalS(&n_input, &light->src.dir);
                }
            }
        }
        result = ((light->flags & 1) << 8) | ((light->event_flags & 2) << 16);
        if (result == 0)
        {
            count = scene_lighting.directional_count;
            result = count < 3;
            if (result != 0)
            {
                x = light->color[0];
                slot = count;
                scene_lighting.color_matrix.m[0][slot] = x;
                scene_lighting.color_matrix.m[1][slot] = light->color[1];
                count = scene_lighting.directional_count;
                z = light->color[2];
                result = count + 1;
                scene_lighting.directional_count = result;
                scene_lighting.color_matrix.m[2][slot] = z;
            }
        }
        light = (SceneLight *)light->links.registry_next;
    }
    scene_lighting.flags = 0;
    x = scene_lighting.ambient[0];
    y = scene_lighting.ambient[1];
    z = scene_lighting.ambient[2];
    xport_gte_write_control(13, x << 4);
    xport_gte_write_control(14, y << 4);
    xport_gte_write_control(15, z << 4);
    SetColorMatrix(&scene_lighting.color_matrix);
}

void SceneLighting::SelectObjectLights()
{
    FUNCTION_MARKER(0x8002E684, "SLUS_005.78");
    uint32 table = 0;
    uint32 row = 0;
    SceneLight *light;
    if (scene_lighting.skip_transform == 0)
    {
        light = scene_lighting.lights;
        while (light != 0)
        {
            if ((light->flags & 1) == 0)
            {
                if ((light->event_flags & 2) != 0)
                {
                    uint32 radius = scene_lighting.bounds_radius + light->radius;
                    uint32 extent = radius << 5;
                    const sint32 *origin = scene_lighting.bounds_origin;
                    uint32 dx = (uint32)light->src.pos.x - origin[0];
                    uint32 dy = (uint32)light->src.pos.y - origin[1];
                    uint32 dz = (uint32)light->src.pos.z - origin[2];
                    sint32 sx, sy, sz;
                    sint32 distance;
                    sint32 limit;
                    if (dx + extent > (radius << 6) || dy + extent > (radius << 6) || dz + extent > (radius << 6))
                        goto next_light;
                    sx = (sint32)dx >> 6;
                    sy = (sint32)dy >> 6;
                    sz = (sint32)dz >> 6;
                    distance = (uint32)((sint64)sx * sx) + (uint32)((sint64)sy * sy) + (uint32)((sint64)sz * sz);
                    limit = (uint64)mul12((sint32)extent, (sint32)extent);
                    if (limit < distance)
                        goto next_light;
                    scene_lighting.point_lights[table] = light;
                    ++table;
                }
                ++row;
                if (row == 3)
                    break;
            }
        next_light:
            light = (SceneLight *)light->links.registry_next;
        }
    }
    while (row != 3)
    {
        for (uint32 axis = 0; axis < 3; ++axis)
            scene_lighting.light_matrix.m[row][axis] = 0;
        ++row;
    }
    scene_lighting.point_lights[table] = 0;
}

void SceneLighting::SetLightingBounds(const sint32 *origin, sint32 radius)
{
    uint32 flags;
    FUNCTION_MARKER(0x8002E824, "SLUS_005.78");
    flags = game_flow.transition_flags;
    scene_lighting.bounds_origin = origin;
    scene_lighting.bounds_radius = radius;
    game_flow.transition_flags = flags & 0xFFFFFFFE;
}

void SceneLighting::TransformObjectLights(uint32 mode)
{
    FUNCTION_MARKER(0x8002E84C, "SLUS_005.78");
    Vec3 vector;
    MATRIX *matrix, *parent_matrix, *next_matrix;
    SceneLight *light;
    uint32 row = 0;
    uint32 table;

    uint32 i;
    uint32 x, y, z;
    if (scene_lighting.bounds_origin != 0)
    {
        SceneLighting::SelectObjectLights();
        scene_lighting.bounds_origin = 0;
    }
    if (scene_lighting.skip_transform == 0)
    {
        matrix = &render_view.matrix_cursor->matrix;
        parent_matrix = render_view.matrix_cursor->prev != 0 ? &render_view.matrix_cursor->prev->matrix : 0;
        for (i = 0; i < 9; ++i)
            scene_lighting.inverse_matrix.m[i / 3][i % 3] = parent_matrix->m[i % 3][i / 3];
        {
            SetRotMatrix(&scene_lighting.inverse_matrix);
        }
        light = scene_lighting.lights;
        while (light != 0 && (light->event_flags & 2) == 0)
        {
            if ((light->flags & 1) == 0)
            {
                gte_ldv0(&light->src.dir);
                xport_gte_execute(0x486012);
                render_view.transform_result.vx = xport_gte_read_data(25);
                render_view.transform_result.vy = xport_gte_read_data(26);
                render_view.transform_result.vz = xport_gte_read_data(27);
                scene_lighting.light_matrix.m[row][0] = render_view.transform_result.vx;
                scene_lighting.light_matrix.m[row][1] = render_view.transform_result.vy;
                scene_lighting.light_matrix.m[row][2] = render_view.transform_result.vz;
                ++row;
            }
            light = (SceneLight *)light->links.registry_next;
        }
        table = 0;
        light = scene_lighting.point_lights[table];
        while (light != 0)
        {
            parent_matrix = render_view.matrix_cursor->prev != 0 ? &render_view.matrix_cursor->prev->matrix : 0;
            vector.x = (uint32)light->src.pos.x - parent_matrix->t[0];
            vector.y = (uint32)light->src.pos.y - parent_matrix->t[1];
            vector.z = (uint32)light->src.pos.z - parent_matrix->t[2];
            if ((game_flow.transition_flags & 0x200) != 0)
            {
                next_matrix = &render_view.matrix_cursor->next->matrix;
                light->relative_pos.x = (uint32)light->src.pos.x - next_matrix->t[0];
                light->relative_pos.y = (uint32)light->src.pos.y - next_matrix->t[1];
                light->relative_pos.z = (uint32)light->src.pos.z - next_matrix->t[2];
                x = light->relative_pos.x;
                y = light->relative_pos.y;
                z = light->relative_pos.z;
                light->relative_pos.x = (sint32)x >> 5;
                light->relative_pos.y = (sint32)y >> 5;
                light->relative_pos.z = (sint32)z >> 5;
            }
            else
            {
                if (table == 0)
                {
                    {
                        VECTOR rotation_input = {parent_matrix->t[0], parent_matrix->t[1], parent_matrix->t[2], 0};
                        VECTOR rotation_output;
                        ApplyRotMatrixLV(&rotation_input, &rotation_output);
                        scene_lighting.origin.x = rotation_output.vx;
                        scene_lighting.origin.y = rotation_output.vy;
                        scene_lighting.origin.z = rotation_output.vz;
                    }
                    x = scene_lighting.origin.x;
                    y = scene_lighting.origin.y;
                    z = scene_lighting.origin.z;
                    scene_lighting.origin.x = -x;
                    scene_lighting.origin.y = -y;
                    scene_lighting.origin.z = -z;
                }
                {
                    VECTOR rotation_input = {light->src.pos.x, light->src.pos.y, light->src.pos.z, 0};
                    VECTOR rotation_output;
                    ApplyRotMatrixLV(&rotation_input, &rotation_output);
                    vector.x = rotation_output.vx;
                    vector.y = rotation_output.vy;
                    vector.z = rotation_output.vz;
                }
                x = vector.x;
                x += (uint32)scene_lighting.origin.x;
                y = scene_lighting.origin.y;
                z = scene_lighting.origin.z;
                y += (uint32)vector.y;
                z += (uint32)vector.z;
                vector.x = x;
                vector.y = y;
                vector.z = z;
                light->relative_pos.x = (sint32)x >> 5;
                light->relative_pos.y = vector.y >> 5;
                light->relative_pos.z = vector.z >> 5;
            }
            while (vector.x + 26754u > 0xD103u || vector.y + 26754u > 0xD103u || vector.z + 26754u > 0xD103u)
            {
                x = vector.x >> 1;
                y = vector.y >> 1;
                z = vector.z >> 1;
                vector.x = x;
                vector.y = y;
                vector.z = z;
            }
            {
                VECTOR n_input = {vector.x, vector.y, vector.z, 0};
                SVECTOR n_output;
                VectorNormalS(&n_input, &n_output);
                scene_lighting.light_matrix.m[row][0] = n_output.vx;
                scene_lighting.light_matrix.m[row][1] = n_output.vy;
                scene_lighting.light_matrix.m[row][2] = n_output.vz;
            }
            ++table;
            light = scene_lighting.point_lights[table];
            ++row;
        }
        matrix = &render_view.matrix_cursor->matrix;
        {
            SetRotMatrix(matrix);
        }
    }

    if (mode != 0)
    {
        uint32 flags = scene_lighting.flags;
        scene_lighting.flags = flags | 0x80000000;
        for (i = 0; i < 9; ++i)
            scene_lighting.light_matrix.m[i / 3][i % 3] = -(uint16)scene_lighting.light_matrix.m[i / 3][i % 3];
    }
    else
    {
        scene_lighting.flags = scene_lighting.flags & 0x7FFFFFFF;
    }
    SetLightMatrix(&scene_lighting.light_matrix);
}

void SceneLighting::ApplyVertexLights(const SVECTOR *vertex, uint32 flags)
{
    FUNCTION_MARKER(0x8002EDE4, "SLUS_005.78");
    uint32 prev_flags = scene_lighting.flags;
    uint32 table = 0;
    SceneLight *light = scene_lighting.point_lights[table];
    uint32 result = flags ^ prev_flags;
    uint32 i;
    if (result != 0)
    {
        scene_lighting.flags = flags;
        if ((sint32)result < 0)
        {
            for (i = 0; i < 9; ++i)
                scene_lighting.light_matrix.m[i / 3][i % 3] = -(uint16)scene_lighting.light_matrix.m[i / 3][i % 3];
            SetLightMatrix(&scene_lighting.light_matrix);
        }
        {
            uint32 ambient = flags & 0x7FFFFFFF;
            uint32 x = (sint16)scene_lighting.ambient[0];
            uint32 y = (sint16)scene_lighting.ambient[1];
            uint32 z = (sint16)scene_lighting.ambient[2];
            result = z + ambient;
            xport_gte_write_control(13, (x + ambient) << 4);
            xport_gte_write_control(14, (y + ambient) << 4);
            xport_gte_write_control(15, result << 4);
        }
    }
    if (light != 0)
    {
        uint32 column = scene_lighting.directional_count;
        do
        {
            uint32 vx = vertex->vx;
            uint32 x = light->relative_pos.x;
            uint32 vy = vertex->vy;
            uint32 y = light->relative_pos.y;
            uint32 vz = vertex->vz;
            uint32 z = light->relative_pos.z;
            uint32 tmp;
            uint32 distance;
            sint32 attenuation;
            x -= vx;
            y -= vy;
            z -= vz;
            if ((sint32)x < 0)
                x = -x;
            if ((sint32)y < 0)
                y = -y;
            if ((sint32)z < 0)
                z = -z;
            if ((sint32)y < (sint32)x)
            {
                tmp = x;
                x = y;
                y = tmp;
            }
            if ((sint32)y < (sint32)z)
            {
                tmp = z;
                z = y;
                y = tmp;
            }
            else if ((sint32)z < (sint32)x)
            {
                tmp = z;
                z = x;
                x = tmp;
            }
            distance = y;
            if (x != 0)
                distance = (sint32)(36 * x + 48 * z + 120 * y) >> 7;
            else if (z != 0)
                distance = (sint32)(51 * z + 123 * y) >> 7;
            result = light->radius;
            attenuation = result - distance;
            if (attenuation <= 0)
            {
                scene_lighting.color_matrix.m[0][column] = 0;
                scene_lighting.color_matrix.m[1][column] = 0;
                scene_lighting.color_matrix.m[2][column] = 0;
            }
            else if (attenuation < (sint32)light->attenuation_range)
            {
                result = (sint32)(attenuation * light->color_scale[0]) >> 2;
                scene_lighting.color_matrix.m[0][column] = result;
                result = (sint32)(attenuation * light->color_scale[1]) >> 2;
                scene_lighting.color_matrix.m[1][column] = result;
                result = (sint32)(attenuation * light->color_scale[2]) >> 2;
                scene_lighting.color_matrix.m[2][column] = result;
            }
            else
            {
                result = light->color[0];
                scene_lighting.color_matrix.m[0][column] = result;
                result = light->color[1];
                scene_lighting.color_matrix.m[1][column] = result;
                result = light->color[2];
                scene_lighting.color_matrix.m[2][column] = result;
            }
            ++table;
            light = scene_lighting.point_lights[table];
            ++column;
        } while (light != 0);
        SetColorMatrix(&scene_lighting.color_matrix);
    }
}
