#include "game.h"

SceneLighting scene_lighting = {0};

void MarkSceneLightingDirty(const SceneLight *light)
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
            if ((sint16)actor->metadata32 != 0)
            {
                flags = actor->flags;
                if ((flags & 1) == 0)
                {
                    result = flags | 1;
                    if (mark_all == 0)
                    {
                        x = actor->world_position[0];
                        y = actor->world_position[1];
                        z = actor->world_position[2];
                        a = x;
                        b = y;
                        c = z;
                        radius = actor->metadata_words72[0];
                        x = light->source.position[0];
                        x -= a;
                        y = light->source.position[1];
                        y -= b;
                        z = light->source.position[2] - c;
                        radius += light->radius << 5;
                        threshold = (uint32)(((sint64)(sint32)radius * (sint32)radius) >> 12);
                        x = (uint32)((sint32)x >> 6);
                        y = (uint32)((sint32)y >> 6);
                        z = (uint32)((sint32)z >> 6);
                        if (x * x + y * y + z * z >= threshold)
                        {
                            x = light->prev_position[0] - a;
                            y = light->prev_position[1] - b;
                            z = light->prev_position[2] - c;
                            x = (uint32)((sint32)x >> 6);
                            y = (uint32)((sint32)y >> 6);
                            z = (uint32)((sint32)z >> 6);
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
            x = light->source.position[0];
            x -= mesh->position[0];
            y = light->source.position[1];
            y -= mesh->position[1];
            z = light->source.position[2];
            z -= mesh->position[2];
            radius = node->state_flags;
            radius += light->radius;
            threshold = (radius * radius) >> 2;
            x = (uint32)((sint32)x >> 6);
            y = (uint32)((sint32)y >> 6);
            z = (uint32)((sint32)z >> 6);
            result = x * x + y * y + z * z < threshold;
            if (result == 0)
            {
                x = light->prev_position[0];
                x -= mesh->position[0];
                y = light->prev_position[1];
                y -= mesh->position[1];
                z = light->prev_position[2];
                z -= mesh->position[2];
                x = (uint32)((sint32)x >> 6);
                y = (uint32)((sint32)y >> 6);
                z = (uint32)((sint32)z >> 6);
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

void UpdateSceneLights()
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
    scene_lighting.directional_count = (uint8)(0);
    while (light != 0)
    {
        flags = light->event_flags;
        if ((flags & 4) != 0)
        {
            kind = light->event_id;
            light->event_flags = (uint8)(flags & 0xFB);
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
                        light->flags = (uint8)(byte_flags | 1);
                        light->event_flags = (uint8)(flags | 1);
                    }
                    break;
                case 43:
                case 54:
                    byte_flags = light->flags;
                    if ((byte_flags & 1) != 0)
                    {
                        flags = light->event_flags;
                        light->flags = (uint8)(byte_flags & 0xFE);
                        light->event_flags = (uint8)(flags | 1);
                    }
                    break;
                case 51:
                    byte_flags = light->flags;
                    flags = light->event_flags;
                    light->flags = (uint8)(byte_flags ^ 1);
                    light->event_flags = (uint8)(flags | 1);
                    break;
                default:
                    break;
            }
        }
        uint32 *event_slots[1] = {&light->event_id};
        if ((ApplySceneEvents(event_slots, 1) << 24) != 0)
            light->event_flags = (uint8)(light->event_flags | 4);
        flags = light->event_flags;
        if ((flags & 1) != 0)
        {
            light->event_flags = (uint8)(flags & 0xFE);
            if (updated == 0)
                MarkSceneLightingDirty(light);
            if ((light->event_flags & 2) != 0)
            {
                x = light->prev_position[0];
                y = light->prev_position[1];
                z = light->prev_position[2];
                light->source.position[0] = x;
                light->source.position[1] = y;
                light->source.position[2] = z;
            }
            else
            {
                updated = 1;
                if (light->prev_position[0] + 26754 > 0xD103 || light->prev_position[1] + 26754 > 0xD103 || light->prev_position[2] + 26754 > 0xD103)
                {
                    for (;;)
                    {
                        x = light->prev_position[0];
                        z = light->prev_position[2];
                        light->prev_position[0] = (uint32)((sint32)x >> 1);
                        y = light->prev_position[1];
                        x = light->prev_position[0];
                        z = (uint32)((sint32)z >> 1);
                        light->prev_position[2] = z;
                        light->prev_position[1] = (uint32)((sint32)y >> 1);
                        if (x + 26754 > 0xD103)
                            continue;
                        if (light->prev_position[1] + 26754 > 0xD103)
                            continue;
                        if (z + 26754 <= 0xD103)
                            break;
                    }
                }
                VectorNormalS((VECTOR *)light->prev_position, &light->source.direction);
            }
        }
        result = ((uint32)(light->flags & 1) << 8) | ((uint32)(light->event_flags & 2) << 16);
        if (result == 0)
        {
            count = scene_lighting.directional_count;
            result = count < 3;
            if (result != 0)
            {
                x = light->color[0];
                slot = count;
                scene_lighting.colors[slot] = (uint16)x;
                scene_lighting.colors[slot + 3] = (uint16)light->color[1];
                count = scene_lighting.directional_count;
                z = light->color[2];
                result = count + 1;
                scene_lighting.directional_count = (uint8)(result);
                scene_lighting.colors[slot + 6] = (uint16)z;
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
    x = xport_load_le32((const uint8 *)scene_lighting.colors + 0);
    y = xport_load_le32((const uint8 *)scene_lighting.colors + 4);
    xport_gte_write_control(16, x);
    xport_gte_write_control(17, y);
    x = xport_load_le32((const uint8 *)scene_lighting.colors + 8);
    y = xport_load_le32((const uint8 *)scene_lighting.colors + 12);
    z = xport_load_le32((const uint8 *)scene_lighting.colors + 16);
    xport_gte_write_control(18, x);
    xport_gte_write_control(19, y);
    xport_gte_write_control(20, z);
}

uint32 SetLightingBounds(const uint32 *origin, sint32 radius)
{
    uint32 flags;
    FUNCTION_MARKER(0x8002E824, "SLUS_005.78");
    flags = game_flow.transition_flags;
    scene_lighting.bounds_origin = origin;
    scene_lighting.bounds_radius = (sint16)radius;
    game_flow.transition_flags = flags & 0xFFFFFFFE;
    return 0xFFFFFFFE;
}

uint32 SelectObjectLights()
{
    FUNCTION_MARKER(0x8002E684, "SLUS_005.78");
    uint32 table = 0;
    uint16 *output = scene_lighting.transforms.direction_output;
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
                    uint32 radius = (uint32)scene_lighting.bounds_radius + light->radius;
                    uint32 extent = radius << 5;
                    const uint32 *origin = scene_lighting.bounds_origin;
                    uint32 dx = light->source.position[0] - origin[0];
                    uint32 dy = light->source.position[1] - origin[1];
                    uint32 dz = light->source.position[2] - origin[2];
                    sint32 sx, sy, sz;
                    uint32 distance;
                    sint32 limit;
                    if (dx + extent > (radius << 6) || dy + extent > (radius << 6) || dz + extent > (radius << 6))
                        goto next_light;
                    sx = (sint32)dx >> 6;
                    sy = (sint32)dy >> 6;
                    sz = (sint32)dz >> 6;
                    distance = (uint32)((sint64)sx * sx) + (uint32)((sint64)sy * sy) + (uint32)((sint64)sz * sz);
                    limit = (sint32)((uint64)((sint64)(sint32)extent * (sint32)extent) >> 12);
                    if (limit < (sint32)distance)
                        goto next_light;
                    scene_lighting.point_lights[table] = light;
                    ++table;
                }
                output += 3;
                if (output == scene_lighting.transforms.direction_output + 9)
                    break;
            }
        next_light:
            light = (SceneLight *)light->links.registry_next;
        }
    }
    while (output != scene_lighting.transforms.direction_output + 9)
    {
        *output = 0;
        ++output;
    }
    scene_lighting.point_lights[table] = 0;
    return 0;
}

uint32 TransformObjectLights(uint32 mode)
{
    FUNCTION_MARKER(0x8002E84C, "SLUS_005.78");
    uint32 vector[3];
    const MATRIX *matrix, *parent_matrix, *next_matrix;
    SceneLight *light;
    uint16 *output = scene_lighting.transforms.direction_output;
    uint32 table;
    uint32 result;
    uint32 i;
    uint32 x, y, z;
    if (scene_lighting.bounds_origin != 0)
    {
        SelectObjectLights();
        scene_lighting.bounds_origin = 0;
    }
    if (scene_lighting.skip_transform == 0)
    {
        matrix = &render_view.matrix_cursor->matrix;
        parent_matrix = (render_view.matrix_cursor->prev != 0 ? &render_view.matrix_cursor->prev->matrix : 0);
        for (i = 0; i < 9; ++i)
            scene_lighting.transforms.fields.inverse_rotation[i] = parent_matrix->m[i % 3][i / 3];
        for (i = 0; i < 5; ++i)
            xport_gte_write_control(i, xport_load_le32((const uint8 *)&scene_lighting.transforms.fields.inverse_rotation + i * 4));
        light = scene_lighting.lights;
        while (light != 0 && (light->event_flags & 2) == 0)
        {
            if ((light->flags & 1) == 0)
            {
                xport_gte_write_data(0, light->source.position[0]);
                xport_gte_write_data(1, light->source.position[1]);
                xport_gte_execute(0x486012);
                render_view.transform_result.vx = xport_gte_read_data(25);
                render_view.transform_result.vy = xport_gte_read_data(26);
                render_view.transform_result.vz = xport_gte_read_data(27);
                output[0] = (uint16)(render_view.transform_result.vx);
                output[1] = (uint16)(render_view.transform_result.vy);
                output[2] = (uint16)(render_view.transform_result.vz);
                output += 3;
            }
            light = (SceneLight *)light->links.registry_next;
        }
        table = 0;
        light = scene_lighting.point_lights[table];
        while (light != 0)
        {
            parent_matrix = (render_view.matrix_cursor->prev != 0 ? &render_view.matrix_cursor->prev->matrix : 0);
            vector[0] = light->source.position[0] - (uint32)parent_matrix->t[0];
            vector[1] = light->source.position[1] - (uint32)parent_matrix->t[1];
            vector[2] = light->source.position[2] - (uint32)parent_matrix->t[2];
            if ((game_flow.transition_flags & 0x200) != 0)
            {
                next_matrix = &render_view.matrix_cursor->next->matrix;
                light->relative_position[0] = light->source.position[0] - (uint32)next_matrix->t[0];
                light->relative_position[1] = light->source.position[1] - (uint32)next_matrix->t[1];
                light->relative_position[2] = light->source.position[2] - (uint32)next_matrix->t[2];
                x = light->relative_position[0];
                y = light->relative_position[1];
                z = light->relative_position[2];
                light->relative_position[0] = (sint32)x >> 5;
                light->relative_position[1] = (sint32)y >> 5;
                light->relative_position[2] = (sint32)z >> 5;
            }
            else
            {
                if (table == 0)
                {
                    ApplyRotMatrixLV((const VECTOR *)parent_matrix->t, (VECTOR *)scene_lighting.transforms.fields.transformed_origin);
                    x = scene_lighting.transforms.fields.transformed_origin[0];
                    y = scene_lighting.transforms.fields.transformed_origin[1];
                    z = scene_lighting.transforms.fields.transformed_origin[2];
                    scene_lighting.transforms.fields.transformed_origin[0] = 0u - x;
                    scene_lighting.transforms.fields.transformed_origin[1] = 0u - y;
                    scene_lighting.transforms.fields.transformed_origin[2] = 0u - z;
                }
                ApplyRotMatrixLV((const VECTOR *)light->source.position, (VECTOR *)vector);
                x = vector[0];
                x += scene_lighting.transforms.fields.transformed_origin[0];
                y = scene_lighting.transforms.fields.transformed_origin[1];
                z = scene_lighting.transforms.fields.transformed_origin[2];
                y += vector[1];
                z += vector[2];
                vector[0] = x;
                vector[1] = y;
                vector[2] = z;
                light->relative_position[0] = (sint32)x >> 5;
                light->relative_position[1] = (sint32)vector[1] >> 5;
                light->relative_position[2] = (sint32)vector[2] >> 5;
            }
            while (vector[0] + 26754u > 0xD103u || vector[1] + 26754u > 0xD103u || vector[2] + 26754u > 0xD103u)
            {
                x = (uint32)((sint32)vector[0] >> 1);
                y = (uint32)((sint32)vector[1] >> 1);
                z = (uint32)((sint32)vector[2] >> 1);
                vector[0] = x;
                vector[1] = y;
                vector[2] = z;
            }
            VectorNormalS((VECTOR *)vector, (SVECTOR *)output);
            ++table;
            light = scene_lighting.point_lights[table];
            output += 3;
        }
        matrix = &render_view.matrix_cursor->matrix;
        for (i = 0; i < 5; ++i)
            xport_gte_write_control(i, xport_load_le32((const uint8 *)matrix + i * 4));
    }
    result = 0x80000000;
    if (mode != 0)
    {
        uint32 flags = scene_lighting.flags;
        uint16 values[9];
        for (i = 0; i < 9; ++i)
            values[i] = scene_lighting.transforms.fields.directions[i];
        scene_lighting.flags = flags | 0x80000000;
        for (i = 0; i < 9; ++i)
            scene_lighting.transforms.fields.directions[i] = (uint16)(0u - values[i]);
    }
    else
    {
        result = scene_lighting.flags & 0x7FFFFFFF;
        scene_lighting.flags = result;
    }
    for (i = 0; i < 5; ++i)
        xport_gte_write_control(8 + i, xport_load_le32((const uint8 *)scene_lighting.transforms.fields.directions + i * 4));
    return result;
}

uint32 ApplyVertexLights(const SVECTOR *vertex, uint32 flags)
{
    FUNCTION_MARKER(0x8002EDE4, "SLUS_005.78");
    uint32 previous_flags = scene_lighting.flags;
    uint32 table = 0;
    SceneLight *light = scene_lighting.point_lights[table];
    uint32 result = flags ^ previous_flags;
    uint32 i;
    if (result != 0)
    {
        scene_lighting.flags = flags;
        if ((sint32)result < 0)
        {
            uint16 values[9];
            for (i = 0; i < 9; ++i)
                values[i] = scene_lighting.transforms.fields.directions[i];
            for (i = 0; i < 9; ++i)
                scene_lighting.transforms.fields.directions[i] = (uint16)(0u - values[i]);
            for (i = 0; i < 5; ++i)
                xport_gte_write_control(8 + i, xport_load_le32((const uint8 *)scene_lighting.transforms.fields.directions + i * 4));
        }
        {
            uint32 ambient = flags & 0x7FFFFFFF;
            uint32 x = (uint32)(sint16)scene_lighting.ambient[0];
            uint32 y = (uint32)(sint16)scene_lighting.ambient[1];
            uint32 z = (uint32)(sint16)scene_lighting.ambient[2];
            result = z + ambient;
            xport_gte_write_control(13, (x + ambient) << 4);
            xport_gte_write_control(14, (y + ambient) << 4);
            xport_gte_write_control(15, result << 4);
        }
    }
    if (light != 0)
    {
        uint16 *output = scene_lighting.colors + scene_lighting.directional_count;
        do
        {
            uint32 vx = (uint32)vertex->vx;
            uint32 x = light->relative_position[0];
            uint32 vy = (uint32)vertex->vy;
            uint32 y = light->relative_position[1];
            uint32 vz = (uint32)vertex->vz;
            uint32 z = light->relative_position[2];
            uint32 temporary, distance, attenuation;
            x -= vx;
            y -= vy;
            z -= vz;
            if ((sint32)x < 0)
                x = 0u - x;
            if ((sint32)y < 0)
                y = 0u - y;
            if ((sint32)z < 0)
                z = 0u - z;
            if ((sint32)y < (sint32)x)
            {
                temporary = x;
                x = y;
                y = temporary;
            }
            if ((sint32)y < (sint32)z)
            {
                temporary = z;
                z = y;
                y = temporary;
            }
            else if ((sint32)z < (sint32)x)
            {
                temporary = z;
                z = x;
                x = temporary;
            }
            distance = y;
            if (x != 0)
                distance = (uint32)((sint32)(36 * x + 48 * z + 120 * y) >> 7);
            else if (z != 0)
                distance = (uint32)((sint32)(51 * z + 123 * y) >> 7);
            result = light->radius;
            attenuation = result - distance;
            if ((sint32)attenuation <= 0)
            {
                output[0] = (uint16)(0);
                output[3] = (uint16)(0);
                output[6] = (uint16)(0);
            }
            else if ((sint32)attenuation < (sint32)light->attenuation_range)
            {
                result = (uint32)((sint32)(attenuation * light->color_scale[0]) >> 2);
                output[0] = (uint16)(result);
                result = (uint32)((sint32)(attenuation * light->color_scale[1]) >> 2);
                output[3] = (uint16)(result);
                result = (uint32)((sint32)(attenuation * light->color_scale[2]) >> 2);
                output[6] = (uint16)(result);
            }
            else
            {
                result = light->color[0];
                output[0] = (uint16)(result);
                result = light->color[1];
                output[3] = (uint16)(result);
                result = light->color[2];
                output[6] = (uint16)(result);
            }
            ++table;
            light = scene_lighting.point_lights[table];
            ++output;
        } while (light != 0);
        for (i = 0; i < 5; ++i)
            xport_gte_write_control(16 + i, xport_load_le32((const uint8 *)scene_lighting.colors + i * 4));
    }
    return result;
}
