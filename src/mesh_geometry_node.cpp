#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mesh_geometry_node.h"
#include "game.h"
#include "animated_object.h"
#include "psx_gte.h"
#include "hd.h"

void MeshGeometryNode::DrawSceneGeometry(bool advance)
{
    const SceneSection *section;
    const MeshVertex *vertices, *vertex;
    MeshGeometryNode *mesh, *geometry_view;
    ProjectedVertex *proj;
    FUNCTION_MARKER(0x80011E04, "SLUS_005.78");
    RenderMatrixSlot *matrix;
    SceneNode *node;
    uint32 flags, x, y, z, remaining, first_idx, val;
    uint32 saved_flags = game_flow.transition_flags;
    MATRIX *cur_matrix, *work_matrix, *camera_matrix, *parent_matrix;
    if (hd_frame.active)
    {
        if (advance)
        {
            game_flow.transition_flags |= 0x200;
            section = scene_stream.active_section;
            node = section->object_heads[2];
            while (node != 0)
            {
                mesh = (MeshGeometryNode *)node;
                mesh->flags |= 0x200;
                geometry_view = mesh->vertex_count > 0 ? mesh : mesh->src_mesh;
                if ((mesh->node.flags & 1) == 0 && (geometry_view->node.flags & 1) == 0)
                {
                    if (geometry_view == mesh && (mesh->flags & 0x2000) != 0)
                        mesh->flags &= ~0x4000u;
                    else if (CameraProjection::IsSphereVisible(mesh->pos.comp, mesh->node.state_flags << 5))
                    {
                        mesh->flags &= ~0x200u;
                        if ((mesh->flags & 0x80) == 0)
                        {
                            if (geometry_view == mesh || (geometry_view->flags & 0x4000) == 0)
                            {
                                MaterialAnimationList::UpdateMaterialAnimationList(geometry_view->material_animation_entries);
                                if (geometry_view != mesh)
                                    geometry_view->flags |= 0x4000;
                            }
                            SceneLighting::SetLightingBounds(mesh->pos.comp, mesh->node.state_flags);
                            mesh->flags &= ~1u;
                        }
                    }
                }
                node = mesh->node.links.registry_next;
            }
            game_flow.transition_flags &= ~0x200u;
        }
        if (hd_frame.collecting && !hd_frame.failed && (!hd_frame.camera_valid || !hd_scene(scene_stream.active_section, &hd_frame.camera, &scene_lighting, NULL)))
            hd_frame.failed = true;
        return;
    }
    matrix = render_view.matrix_cursor;
    flags = game_flow.transition_flags;
    {
        RenderMatrixSlot *reserve_slot = matrix;
        uint32 reserve_idx;
        for (reserve_idx = 0; reserve_idx < 3; ++reserve_idx)
        {
            if (reserve_slot->next == 0)
            {
                RenderMatrixSlot *next_slot = (RenderMatrixSlot *)malloc(sizeof(RenderMatrixSlot));
                uint32 default_offset, default_bytes;
                memset(next_slot, 0, sizeof(RenderMatrixSlot));
                next_slot->prev = reserve_slot;
                next_slot->depth = reserve_slot->depth + 1;
                default_offset = OriginalDefaults::RenderMatrixCursorOffset - 0x1F800000u + next_slot->depth * 32;
                if (default_offset < sizeof(OriginalDefaults::scratchpad_startup))
                {
                    default_bytes = sizeof(OriginalDefaults::scratchpad_startup) - default_offset;
                    if (default_bytes > sizeof(MATRIX))
                        default_bytes = sizeof(MATRIX);
                    CopyBytes((uint8 *)&next_slot->matrix, OriginalDefaults::scratchpad_startup + default_offset, default_bytes);
                }
                reserve_slot->next = next_slot;
            }
            reserve_slot = reserve_slot->next;
        }
    }
    cur_matrix = &matrix->matrix;
    work_matrix = &matrix->next->matrix;
    camera_matrix = &matrix->next->next->matrix;
    parent_matrix = matrix->prev != 0 ? &matrix->prev->matrix : 0;
    render_view.matrix_cursor = matrix->next;
    game_flow.transition_flags = flags | 0x200;
    SetRotMatrix(&render_view.camera_transform);
    *work_matrix = render_view.camera_transform;
    matrix = render_view.matrix_cursor;
    cur_matrix = &matrix->matrix;
    work_matrix = &matrix->next->matrix;
    camera_matrix = &matrix->next->next->matrix;
    parent_matrix = matrix->prev != 0 ? &matrix->prev->matrix : 0;
    x = camera_motion.pos.x;
    y = camera_motion.pos.y;
    z = camera_motion.pos.z;
    camera_matrix->t[0] = x;
    camera_matrix->t[1] = y;
    camera_matrix->t[2] = z;
    section = scene_stream.active_section;
    node = section->object_heads[2];
    while (node != 0)
    {
        mesh = (MeshGeometryNode *)node;
        flags = mesh->flags;
        val = mesh->node.flags;
        flags |= 0x200;
        if (advance)
            mesh->flags = flags;
        if ((val & 1) != 0)
            goto next_node;
        geometry_view = mesh;
        if (mesh->vertex_count <= 0)
        {
            geometry_view = mesh->src_mesh;
            if ((geometry_view->node.flags & 1) != 0)
                goto next_node;
        }
        if (geometry_view == mesh && (flags & 0x2000) != 0)
        {
            if (advance)
                mesh->flags = flags & 0xFFFFBFFF;
            goto next_node;
        }
        val = mesh->node.state_flags;
        if ((CameraProjection::IsSphereVisible(mesh->pos.comp, val << 5) << 24) == 0)
            goto next_node;
        flags = mesh->flags;
        if (advance)
            mesh->flags = flags & 0xFFFFFDFF;
        if ((flags & 0x80) != 0)
            goto next_node;
        remaining = geometry_view->vertex_count;
        first_idx = (uint16)geometry_view->first_vertex;
        vertices = geometry_view->vertices + first_idx;
        if (advance && geometry_view != mesh && (geometry_view->flags & 0x4000) != 0)
            goto render_node;
        matrix = render_view.matrix_cursor;
        cur_matrix = &matrix->matrix;
        work_matrix = &matrix->next->matrix;
        camera_matrix = &matrix->next->next->matrix;
        parent_matrix = matrix->prev != 0 ? &matrix->prev->matrix : 0;
        x = geometry_view->pos.x;
        y = geometry_view->pos.y;
        z = geometry_view->pos.z;
        work_matrix->t[0] = x;
        work_matrix->t[1] = y;
        work_matrix->t[2] = z;
        if (advance)
            MaterialAnimationList::UpdateMaterialAnimationList(geometry_view->material_animation_entries);
        matrix = render_view.matrix_cursor;
        cur_matrix = &matrix->matrix;
        work_matrix = &matrix->next->matrix;
        camera_matrix = &matrix->next->next->matrix;
        parent_matrix = matrix->prev != 0 ? &matrix->prev->matrix : 0;
        {
            VECTOR rotation_input = {work_matrix->t[0], work_matrix->t[1], work_matrix->t[2], 0};
            VECTOR rotation_output;
            ApplyRotMatrixLV(&rotation_input, &rotation_output);
            cur_matrix->t[0] = rotation_output.vx;
            cur_matrix->t[1] = rotation_output.vy;
            cur_matrix->t[2] = rotation_output.vz;
        }
        matrix = render_view.matrix_cursor;
        cur_matrix = &matrix->matrix;
        work_matrix = &matrix->next->matrix;
        camera_matrix = &matrix->next->next->matrix;
        parent_matrix = matrix->prev != 0 ? &matrix->prev->matrix : 0;
        x = render_view.camera_transform.t[0];
        cur_matrix->t[0] = cur_matrix->t[0] + x;
        cur_matrix->t[1] = cur_matrix->t[1] + (uint32)render_view.camera_transform.t[1];
        cur_matrix->t[2] = cur_matrix->t[2] + (uint32)render_view.camera_transform.t[2];
        render_view.inverse_transform.t[0] = camera_matrix->t[0] - (uint32)work_matrix->t[0];
        render_view.inverse_transform.t[1] = camera_matrix->t[1] - (uint32)work_matrix->t[1];
        render_view.inverse_transform.t[2] = camera_matrix->t[2] - (uint32)work_matrix->t[2];
        x = cur_matrix->t[0];
        y = cur_matrix->t[1];
        z = cur_matrix->t[2];
        xport_gte_write_control(5, ((sint32)x >> 5));
        xport_gte_write_control(6, ((sint32)y >> 5));
        xport_gte_write_control(7, ((sint32)z >> 5));
        proj = render_view.proj_vertices;
        remaining -= 3;
        vertex = vertices;
        if ((sint32)remaining >= 0)
        {
            do
            {
                gte_ldv0(&vertex[0].pos);
                gte_ldv1(&vertex[1].pos);
                gte_ldv2(&vertex[2].pos);
                xport_gte_execute(0x280030);
                proj[0].xy = xport_gte_read_data(12);
                proj[1].xy = xport_gte_read_data(13);
                proj[2].xy = xport_gte_read_data(14);
                proj[0].depth = xport_gte_read_data(17);
                proj[1].depth = xport_gte_read_data(18);
                proj[2].depth = xport_gte_read_data(19);
                vertex += 3;
                remaining -= 3;
                proj += 3;
            } while ((sint32)remaining >= 0);
        }
        remaining += 3;
        while (remaining != 0)
        {
            gte_ldv0(&vertex->pos);
            xport_gte_execute(0x180001);
            proj[0].xy = xport_gte_read_data(14);
            proj[0].depth = xport_gte_read_data(19);
            ++vertex;
            ++proj;
            --remaining;
        }
        first_idx = (uint16)geometry_view->first_vertex;
        render_view.first_proj_vertex = first_idx;
        if (advance && geometry_view != mesh)
            geometry_view->flags = geometry_view->flags | 0x4000;
    render_node:
        section = scene_stream.active_section;
        val = mesh->lighting_idx;
        matrix = render_view.matrix_cursor;
        cur_matrix = &matrix->matrix;
        work_matrix = &matrix->next->matrix;
        camera_matrix = &matrix->next->next->matrix;
        parent_matrix = matrix->prev != 0 ? &matrix->prev->matrix : 0;
        render_view.lighting_cache = section->lighting_recs + val / 4;
        x = mesh->pos.x;
        y = mesh->pos.y;
        z = mesh->pos.z;
        parent_matrix->t[0] = x;
        parent_matrix->t[1] = y;
        parent_matrix->t[2] = z;
        SceneLighting::SetLightingBounds(mesh->pos.comp, mesh->node.state_flags);
        if ((mesh->flags & 1) == 0)
            MeshGeometryNode::DrawFacesWithCachedLighting(vertices, mesh->polygons, mesh->polygon_count);
        else
        {
            MeshGeometryNode::DrawFacesWithFreshLighting(vertices, mesh->polygons, mesh->polygon_count);
            if (advance)
                mesh->flags = mesh->flags & 0xFFFFFFFE;
        }
    next_node:
        node = mesh->node.links.registry_next;
    }
    flags = game_flow.transition_flags;
    matrix = render_view.matrix_cursor;
    cur_matrix = &matrix->matrix;
    work_matrix = &matrix->next->matrix;
    camera_matrix = &matrix->next->next->matrix;
    parent_matrix = matrix->prev != 0 ? &matrix->prev->matrix : 0;
    game_flow.transition_flags = advance ? flags & 0xFFFFFDFF : saved_flags;
    render_view.matrix_cursor = matrix->prev;
    if (hd_frame.collecting && !hd_frame.failed && (!hd_frame.camera_valid || !hd_scene(scene_stream.active_section, &hd_frame.camera, &scene_lighting, NULL)))
        hd_frame.failed = true;
}

void MeshGeometryNode::DrawFacesWithFreshLighting(const MeshVertex *vertices, const MeshFace *faces, sint32 count)
{
    uint32 *bucket_entry;
    uint32 *packet_tag;

    const SceneSection *section;
    const MeshFace *face;
    const TextureDescriptor *desc;
    FUNCTION_MARKER(0x800615C4, "SLUS_005.78");
    MeshLighting *lighting_cache;
    MeshLighting *colors;
    uint32 remaining = count;
    uint32 packet, limit, sum, depth, val, index0, index1, index2, index3, first_proj_vertex;
    const ProjectedVertex *screen0, *screen1, *screen2, *screen3;
    const MeshVertex *vertex0, *vertex1, *vertex2, *vertex3;
    uint32 flags, start, end, kind, bucket_offset, link, tag, color3, result;
    sint32 coord, coefficient, surface, bucket, distance, fade;
    SceneLighting::TransformObjectLights(0);
    colors = render_view.lighting_cache;
    xport_gte_write_data(8, 0);
    do
    {
        face = faces;
        lighting_cache = colors;
        lighting_cache->control = 0;
        packet = render_commands.packet_offset;
        limit = render_commands.packet_limit;
        result = packet < limit;
        if (result == 0)
            break;
        coord = render_view.inverse_transform.t[0] >> 6;
        coefficient = face->n.x;
        sum = (sint64)coord * coefficient;
        coord = render_view.inverse_transform.t[1] >> 6;
        coefficient = face->n.y;
        sum += (uint32)((sint64)coord * coefficient);
        coord = render_view.inverse_transform.t[2] >> 6;
        coefficient = face->n.z;
        val = face->plane;
        sum += (uint32)((sint64)coord * coefficient);
        result = ((sint32)sum >> 6) + val;
        surface = result;
        if (surface <= 0)
            goto next_polygon;
        index0 = face->vertex_idxs[0];
        first_proj_vertex = render_view.first_proj_vertex;
        index1 = face->vertex_idxs[1];
        index2 = face->vertex_idxs[2];
        index3 = face->vertex_idxs[3];
        screen0 = render_view.proj_vertices + (index0 - first_proj_vertex);
        screen1 = render_view.proj_vertices + (index1 - first_proj_vertex);
        screen2 = render_view.proj_vertices + (index2 - first_proj_vertex);
        screen3 = render_view.proj_vertices + (index3 - first_proj_vertex);
        vertex0 = vertices + (index0 - first_proj_vertex);
        vertex1 = vertices + (index1 - first_proj_vertex);
        vertex2 = vertices + (index2 - first_proj_vertex);
        vertex3 = vertices + (index3 - first_proj_vertex);
        if (screen0->coords.x >= 320 && screen1->coords.x >= 320 && screen2->coords.x >= 320 && screen3->coords.x >= 320)
        {
            goto next_polygon;
        }
        if (screen0->coords.y >= 240 && screen1->coords.y >= 240 && screen2->coords.y >= 240 && screen3->coords.y >= 240)
        {
            goto next_polygon;
        }
        depth = screen0->depth;
        sum = screen1->depth;
        val = screen2->depth;
        link = screen3->depth;
        limit = render_view.far_distance;
        depth += sum + val + link;
        result = (sint32)(depth << 1) < (sint32)limit;
        if (result == 0)
            goto next_polygon;
        desc = face->tex;
        flags = desc->flags;
        if ((flags & 1) != 0 && surface <= 0x3FFFF800)
            lighting_cache->control = 0x80000000;
        coefficient = face->n.depth_bias;
        val = render_view.fog_scale;
        sum = ((sint32)depth >> 2) + (uint32)(coefficient >> 1);
        bucket = (sint32)(sum * val) >> 19;
        if ((uint32)bucket >= 1024)
            bucket = bucket < 0 ? 0 : 1023;
        val = lighting_cache->control;
        SceneLighting::ApplyVertexLights(&vertex0->pos, (flags >> 24) | (val & 0x80000000));
        xport_gte_write_data(6, desc->mapping.color.rgbc);
        section = scene_stream.active_section;
        end = section->fog_far;
        if (end != 0)
        {
            val = render_view.far_distance;
            distance = mul12((sint32)val, (sint32)((uint32)bucket << 2));
            start = section->fog_near;
            if ((sint32)start < distance)
            {
                result = distance - start;
                if (distance >= (sint32)end)
                    goto next_polygon;
                coefficient = render_view.depth_scale;
                fade = (sint32)mul12(coefficient, (sint32)result) >> 8;
                xport_gte_write_data(8, fade);
            }
            else
                xport_gte_write_data(8, 0);
        }
        else
            xport_gte_write_data(8, 0);
        kind = flags & 6;
        if (kind == 6 || kind == 2)
        {
            gte_ldv0(&vertex0->n);
            gte_ldv1(&vertex1->n);
            gte_ldv2(&vertex3->n);
            xport_gte_execute(0x118043F);
            tag = kind == 6 ? 0x0C000000 : 0x08000000;
        }
        else
        {
            xport_gte_write_data(0, (uint16)face->n.x | ((uint32)face->n.y << 16));
            xport_gte_write_data(1, (uint16)face->n.z | ((uint32)face->n.depth_bias << 16));
            xport_gte_execute(0x108041B);
            tag = kind == 4 ? 0x09000000 : 0x05000000;
        }
        bucket_offset = render_commands.ordering_table_offset + (uint32)bucket * 4;
        bucket_entry = (uint32 *)psx_addr(bucket_offset, sizeof(uint32));
        link = *bucket_entry;
        packet = render_commands.packet_offset;
        packet_tag = (uint32 *)psx_addr(packet, sizeof(uint32));
        *packet_tag = (link & 0xFFFFFF) | tag;
        link = *bucket_entry;
        *bucket_entry = (link & 0xFF000000) | (packet & 0xFFFFFF);
        if (kind == 6)
        {
            desc->WritePolygonUV(render_commands.packet_offset + 12, 3);
            lighting_cache->colors[0] = xport_gte_read_data(20);
            lighting_cache->colors[1] = xport_gte_read_data(21);
            lighting_cache->colors[2] = xport_gte_read_data(22);
            gte_ldv0(&vertex2->n);
            xport_gte_execute(0x108041B);
            packet = render_commands.packet_offset;
            POLY_GT4 *poly = (POLY_GT4 *)psx_addr(packet, sizeof(*poly));
            poly->p[0].xy.xy = screen0->xy;
            color3 = xport_gte_read_data(22);
            xport_gte_write_data(20, lighting_cache->colors[0]);
            xport_gte_write_data(21, lighting_cache->colors[1]);
            xport_gte_write_data(22, lighting_cache->colors[2]);
            xport_gte_write_data(6, lighting_cache->colors[2]);
            xport_gte_execute(0xF8002A);

            poly->p[1].xy.xy = screen1->xy;
            poly->p[2].xy.xy = screen3->xy;
            poly->p[3].xy.xy = screen2->xy;

            poly->p[0].rgbc.rgbc = xport_gte_read_data(20);
            poly->p[1].rgbc.rgbc = xport_gte_read_data(21);
            poly->p[2].rgbc.rgbc = xport_gte_read_data(22);
            xport_gte_write_data(6, color3);
            xport_gte_execute(0x780010);
            lighting_cache->control = lighting_cache->control | 0x40000000 | (color3 & 0xFFFFFF);

            poly->p[3].rgbc.rgbc = xport_gte_read_data(22);
            result = render_commands.packet_offset + sizeof(*poly);
            render_commands.packet_offset = result;
        }
        else if (kind == 2)
        {
            packet = render_commands.packet_offset;
            if ((flags & 0x80) != 0)
            {
                DR_MODE *mode = (DR_MODE *)psx_addr(packet, sizeof(*mode));
                uint32 *next_tag = (uint32 *)psx_addr(packet + sizeof(*mode), sizeof(*next_tag));
                link = mode->tag;
                *next_tag = (link & 0xFFFFFF) | 0x08000000;
                mode->tag = ((packet + sizeof(*mode)) & 0xFFFFFF) | 0x02000000;
                mode->code[0] = (flags & 0x60) | 0xE1000000;
                mode->code[1] = 0;
                packet += sizeof(*mode);
            }
            lighting_cache->colors[0] = xport_gte_read_data(20);
            lighting_cache->colors[1] = xport_gte_read_data(21);
            lighting_cache->colors[2] = xport_gte_read_data(22);
            gte_ldv0(&vertex2->n);
            xport_gte_execute(0x108041B);
            POLY_G4 *poly = (POLY_G4 *)psx_addr(packet, sizeof(*poly));
            poly->p[0].xy.xy = screen0->xy;
            color3 = xport_gte_read_data(22);
            xport_gte_write_data(20, lighting_cache->colors[0]);
            xport_gte_write_data(21, lighting_cache->colors[1]);
            xport_gte_write_data(22, lighting_cache->colors[2]);
            xport_gte_write_data(6, lighting_cache->colors[2]);
            xport_gte_execute(0xF8002A);
            poly->p[1].xy.xy = screen1->xy;
            poly->p[2].xy.xy = screen3->xy;
            poly->p[3].xy.xy = screen2->xy;
            poly->p[0].rgbc.rgbc = xport_gte_read_data(20);
            poly->p[1].rgbc.rgbc = xport_gte_read_data(21);
            poly->p[2].rgbc.rgbc = xport_gte_read_data(22);
            xport_gte_write_data(6, color3);
            xport_gte_execute(0x780010);
            lighting_cache->control = lighting_cache->control | 0x40000000 | (color3 & 0xFFFFFF);
            poly->p[3].rgbc.rgbc = xport_gte_read_data(22);
            result = packet + sizeof(*poly);
            render_commands.packet_offset = result;
        }
        else if (kind == 4)
        {
            packet = render_commands.packet_offset;
            desc->WritePolygonUV(packet + 12, 2);
            lighting_cache->colors[0] = xport_gte_read_data(22);
            xport_gte_write_data(6, lighting_cache->colors[0]);
            xport_gte_execute(0x780010);
            POLY_FT4 *poly = (POLY_FT4 *)psx_addr(packet, sizeof(*poly));
            poly->p[0].xy.xy = screen0->xy;
            poly->p[1].xy.xy = screen1->xy;
            poly->p[2].xy.xy = screen3->xy;
            poly->p[3].xy.xy = screen2->xy;
            poly->rgbc.rgbc = xport_gte_read_data(22);
            result = lighting_cache->control | 0x40000000;
            lighting_cache->control = result;
            render_commands.packet_offset = packet + sizeof(*poly);
        }
        else
        {
            packet = render_commands.packet_offset;
            if ((flags & 0x80) != 0)
            {
                DR_MODE *mode = (DR_MODE *)psx_addr(packet, sizeof(*mode));
                uint32 *next_tag = (uint32 *)psx_addr(packet + sizeof(*mode), sizeof(*next_tag));
                link = mode->tag;
                *next_tag = (link & 0xFFFFFF) | 0x05000000;
                mode->tag = ((packet + sizeof(*mode)) & 0xFFFFFF) | 0x02000000;
                mode->code[0] = (flags & 0x60) | 0xE1000000;
                mode->code[1] = 0;
                packet += sizeof(*mode);
            }
            lighting_cache->colors[0] = xport_gte_read_data(22);
            xport_gte_write_data(6, lighting_cache->colors[0]);
            xport_gte_execute(0x780010);
            POLY_F4 *poly = (POLY_F4 *)psx_addr(packet, sizeof(*poly));
            poly->p[0].xy = screen0->xy;
            poly->p[1].xy = screen1->xy;
            poly->p[2].xy = screen3->xy;
            poly->p[3].xy = screen2->xy;
            poly->rgbc.rgbc = xport_gte_read_data(22);
            lighting_cache->control = lighting_cache->control | 0x40000000;
            result = packet + sizeof(*poly);
            render_commands.packet_offset = result;
        }
    next_polygon:
        ++faces;
        ++colors;
        --remaining;
    } while (remaining != 0);
}

void MeshGeometryNode::DrawFacesWithCachedLighting(const MeshVertex *vertices, const MeshFace *faces, sint32 count)
{
    uint32 *bucket_entry;
    uint32 *packet_tag;

    const SceneSection *section;
    const MeshFace *face;
    const TextureDescriptor *desc;
    FUNCTION_MARKER(0x80061F50, "SLUS_005.78");
    MeshLighting *lighting_cache;
    MeshLighting *colors;
    uint32 remaining = count;
    uint32 packet, limit, sum, depth, val, index0, index1, index2, index3, first_proj_vertex;
    const ProjectedVertex *screen0, *screen1, *screen2, *screen3;
    const MeshVertex *vertex0, *vertex1, *vertex2, *vertex3;
    uint32 flags, start, end, kind, bucket_offset, link, tag, color3, result;
    sint32 coord, coefficient, surface, bucket, distance, fade;
    colors = render_view.lighting_cache;
    xport_gte_write_data(8, 0);
    do
    {
        face = faces;
        lighting_cache = colors;
        packet = render_commands.packet_offset;
        limit = render_commands.packet_limit;
        result = packet < limit;
        if (result == 0)
            break;
        coord = render_view.inverse_transform.t[0] >> 6;
        coefficient = face->n.x;
        sum = (sint64)coord * coefficient;
        coord = render_view.inverse_transform.t[1] >> 6;
        coefficient = face->n.y;
        sum += (uint32)((sint64)coord * coefficient);
        coord = render_view.inverse_transform.t[2] >> 6;
        coefficient = face->n.z;
        val = face->plane;
        sum += (uint32)((sint64)coord * coefficient);
        result = ((sint32)sum >> 6) + val;
        surface = result;
        if (surface <= 0)
            goto next_polygon;
        index0 = face->vertex_idxs[0];
        first_proj_vertex = render_view.first_proj_vertex;
        index1 = face->vertex_idxs[1];
        index3 = face->vertex_idxs[3];
        index2 = face->vertex_idxs[2];
        screen0 = render_view.proj_vertices + (index0 - first_proj_vertex);
        screen1 = render_view.proj_vertices + (index1 - first_proj_vertex);
        screen2 = render_view.proj_vertices + (index2 - first_proj_vertex);
        screen3 = render_view.proj_vertices + (index3 - first_proj_vertex);
        if (screen0->coords.x >= 320 && screen1->coords.x >= 320 && screen2->coords.x >= 320 && screen3->coords.x >= 320)
        {
            goto next_polygon;
        }
        if (screen0->coords.y >= 240 && screen1->coords.y >= 240 && screen2->coords.y >= 240 && screen3->coords.y >= 240)
        {
            goto next_polygon;
        }
        depth = screen0->depth;
        sum = screen1->depth;
        val = screen2->depth;
        link = screen3->depth;
        limit = render_view.far_distance;
        depth += sum + val + link;
        result = (sint32)(depth << 1) < (sint32)limit;
        if (result == 0)
            goto next_polygon;
        desc = face->tex;
        flags = desc->flags;
        if ((flags & 1) != 0)
        {
            if (surface > 0x3FFFF800)
            {
                if ((sint32)lighting_cache->control < 0)
                    lighting_cache->control = 0;
            }
            else if ((sint32)lighting_cache->control >= 0)
                lighting_cache->control = 0x80000000;
        }
        coefficient = face->n.depth_bias;
        val = render_view.fog_scale;
        sum = ((sint32)depth >> 2) + (uint32)(coefficient >> 1);
        bucket = (sint32)(sum * val) >> 19;
        if ((uint32)bucket >= 1024)
            bucket = bucket < 0 ? 0 : 1023;
        section = scene_stream.active_section;
        end = section->fog_far;
        if (end != 0)
        {
            val = render_view.far_distance;
            distance = mul12((sint32)val, (sint32)((uint32)bucket << 2));
            start = section->fog_near;
            if ((sint32)start < distance)
            {
                result = distance - start;
                if (distance >= (sint32)end)
                    goto next_polygon;
                coefficient = render_view.depth_scale;
                fade = (sint32)mul12(coefficient, (sint32)result) >> 8;
                xport_gte_write_data(8, fade);
            }
            else
                xport_gte_write_data(8, 0);
        }
        else
            xport_gte_write_data(8, 0);
        kind = flags & 6;
        if ((lighting_cache->control & 0x40000000) == 0)
        {
            if ((game_flow.transition_flags & 1) == 0)
            {
                SceneLighting::TransformObjectLights(0);
                game_flow.transition_flags = game_flow.transition_flags | 1;
            }
            vertex0 = vertices + (index0 - first_proj_vertex);
            vertex1 = vertices + (index1 - first_proj_vertex);
            vertex3 = vertices + (index3 - first_proj_vertex);
            vertex2 = vertices + (index2 - first_proj_vertex);
            val = lighting_cache->control;
            SceneLighting::ApplyVertexLights(&vertex0->pos, (flags >> 24) | (val & 0x80000000));
            xport_gte_write_data(6, desc->mapping.color.rgbc);
            if (kind == 6 || kind == 2)
            {
                gte_ldv0(&vertex0->n);
                gte_ldv1(&vertex1->n);
                gte_ldv2(&vertex3->n);
                xport_gte_execute(0x118043F);
                tag = kind == 6 ? 0x0C000000 : 0x08000000;
            }
            else
            {
                xport_gte_write_data(0, (uint16)face->n.x | ((uint32)face->n.y << 16));
                xport_gte_write_data(1, (uint16)face->n.z | ((uint32)face->n.depth_bias << 16));
                xport_gte_execute(0x108041B);
                tag = kind == 4 ? 0x09000000 : 0x05000000;
            }
            bucket_offset = render_commands.ordering_table_offset + (uint32)bucket * 4;
            bucket_entry = (uint32 *)psx_addr(bucket_offset, sizeof(uint32));
            link = *bucket_entry;
            packet = render_commands.packet_offset;
            packet_tag = (uint32 *)psx_addr(packet, sizeof(uint32));
            *packet_tag = (link & 0xFFFFFF) | tag;
            link = *bucket_entry;
            *bucket_entry = (link & 0xFF000000) | (packet & 0xFFFFFF);
            if (kind == 6)
            {
                desc->WritePolygonUV(render_commands.packet_offset + 12, 3);
                lighting_cache->colors[0] = xport_gte_read_data(20);
                lighting_cache->colors[1] = xport_gte_read_data(21);
                lighting_cache->colors[2] = xport_gte_read_data(22);
                gte_ldv0(&vertex2->n);
                xport_gte_execute(0x108041B);
                packet = render_commands.packet_offset;
                POLY_GT4 *poly = (POLY_GT4 *)psx_addr(packet, sizeof(*poly));
                poly->p[0].xy.xy = screen0->xy;
                color3 = xport_gte_read_data(22);
                xport_gte_write_data(20, lighting_cache->colors[0]);
                xport_gte_write_data(21, lighting_cache->colors[1]);
                xport_gte_write_data(22, lighting_cache->colors[2]);
                xport_gte_write_data(6, lighting_cache->colors[2]);
                xport_gte_execute(0xF8002A);

                poly->p[1].xy.xy = screen1->xy;
                poly->p[2].xy.xy = screen3->xy;
                poly->p[3].xy.xy = screen2->xy;

                poly->p[0].rgbc.rgbc = xport_gte_read_data(20);
                poly->p[1].rgbc.rgbc = xport_gte_read_data(21);
                poly->p[2].rgbc.rgbc = xport_gte_read_data(22);
                xport_gte_write_data(6, color3);
                xport_gte_execute(0x780010);
                lighting_cache->control = lighting_cache->control | 0x40000000 | (color3 & 0xFFFFFF);

                poly->p[3].rgbc.rgbc = xport_gte_read_data(22);
                result = render_commands.packet_offset + sizeof(*poly);
                render_commands.packet_offset = result;
            }
            else if (kind == 2)
            {
                packet = render_commands.packet_offset;
                if ((flags & 0x80) != 0)
                {
                    DR_MODE *mode = (DR_MODE *)psx_addr(packet, sizeof(*mode));
                    uint32 *next_tag = (uint32 *)psx_addr(packet + sizeof(*mode), sizeof(*next_tag));
                    link = mode->tag;
                    *next_tag = (link & 0xFFFFFF) | 0x08000000;
                    mode->tag = ((packet + sizeof(*mode)) & 0xFFFFFF) | 0x02000000;
                    mode->code[0] = (flags & 0x60) | 0xE1000000;
                    mode->code[1] = 0;
                    packet += sizeof(*mode);
                }
                lighting_cache->colors[0] = xport_gte_read_data(20);
                lighting_cache->colors[1] = xport_gte_read_data(21);
                lighting_cache->colors[2] = xport_gte_read_data(22);
                gte_ldv0(&vertex2->n);
                xport_gte_execute(0x108041B);
                POLY_G4 *poly = (POLY_G4 *)psx_addr(packet, sizeof(*poly));
                poly->p[0].xy.xy = screen0->xy;
                color3 = xport_gte_read_data(22);
                xport_gte_write_data(20, lighting_cache->colors[0]);
                xport_gte_write_data(21, lighting_cache->colors[1]);
                xport_gte_write_data(22, lighting_cache->colors[2]);
                xport_gte_write_data(6, lighting_cache->colors[2]);
                xport_gte_execute(0xF8002A);
                poly->p[1].xy.xy = screen1->xy;
                poly->p[2].xy.xy = screen3->xy;
                poly->p[3].xy.xy = screen2->xy;
                poly->p[0].rgbc.rgbc = xport_gte_read_data(20);
                poly->p[1].rgbc.rgbc = xport_gte_read_data(21);
                poly->p[2].rgbc.rgbc = xport_gte_read_data(22);
                xport_gte_write_data(6, color3);
                xport_gte_execute(0x780010);
                lighting_cache->control = lighting_cache->control | 0x40000000 | (color3 & 0xFFFFFF);
                poly->p[3].rgbc.rgbc = xport_gte_read_data(22);
                result = packet + sizeof(*poly);
                render_commands.packet_offset = result;
            }
            else if (kind == 4)
            {
                packet = render_commands.packet_offset;
                desc->WritePolygonUV(packet + 12, 2);
                lighting_cache->colors[0] = xport_gte_read_data(22);
                xport_gte_write_data(6, lighting_cache->colors[0]);
                xport_gte_execute(0x780010);
                POLY_FT4 *poly = (POLY_FT4 *)psx_addr(packet, sizeof(*poly));
                poly->p[0].xy.xy = screen0->xy;
                poly->p[1].xy.xy = screen1->xy;
                poly->p[2].xy.xy = screen3->xy;
                poly->p[3].xy.xy = screen2->xy;
                poly->rgbc.rgbc = xport_gte_read_data(22);
                result = lighting_cache->control | 0x40000000;
                lighting_cache->control = result;
                render_commands.packet_offset = packet + sizeof(*poly);
            }
            else
            {
                packet = render_commands.packet_offset;
                if ((flags & 0x80) != 0)
                {
                    DR_MODE *mode = (DR_MODE *)psx_addr(packet, sizeof(*mode));
                    uint32 *next_tag = (uint32 *)psx_addr(packet + sizeof(*mode), sizeof(*next_tag));
                    link = mode->tag;
                    *next_tag = (link & 0xFFFFFF) | 0x05000000;
                    mode->tag = ((packet + sizeof(*mode)) & 0xFFFFFF) | 0x02000000;
                    mode->code[0] = (flags & 0x60) | 0xE1000000;
                    mode->code[1] = 0;
                    packet += sizeof(*mode);
                }
                lighting_cache->colors[0] = xport_gte_read_data(22);
                xport_gte_write_data(6, lighting_cache->colors[0]);
                xport_gte_execute(0x780010);
                POLY_F4 *poly = (POLY_F4 *)psx_addr(packet, sizeof(*poly));
                poly->p[0].xy = screen0->xy;
                poly->p[1].xy = screen1->xy;
                poly->p[2].xy = screen3->xy;
                poly->p[3].xy = screen2->xy;
                poly->rgbc.rgbc = xport_gte_read_data(22);
                lighting_cache->control = lighting_cache->control | 0x40000000;
                result = packet + sizeof(*poly);
                render_commands.packet_offset = result;
            }
        }
        else
        {
            if (kind == 6 || kind == 2)
            {
                xport_gte_write_data(20, lighting_cache->colors[0]);
                xport_gte_write_data(21, lighting_cache->colors[1]);
                xport_gte_write_data(22, lighting_cache->colors[2]);
                xport_gte_write_data(6, lighting_cache->colors[2]);
                xport_gte_execute(0xF8002A);
                tag = kind == 6 ? 0x0C000000 : 0x08000000;
            }
            else
            {
                xport_gte_write_data(6, lighting_cache->colors[0]);
                xport_gte_execute(0x780010);
                tag = kind == 4 ? 0x09000000 : 0x05000000;
            }
            bucket_offset = render_commands.ordering_table_offset + (uint32)bucket * 4;
            bucket_entry = (uint32 *)psx_addr(bucket_offset, sizeof(uint32));
            link = *bucket_entry;
            packet = render_commands.packet_offset;
            packet_tag = (uint32 *)psx_addr(packet, sizeof(uint32));
            *packet_tag = (link & 0xFFFFFF) | tag;
            link = *bucket_entry;
            *bucket_entry = (link & 0xFF000000) | (packet & 0xFFFFFF);
            if (kind == 6)
            {
                desc->WritePolygonUV(render_commands.packet_offset + 12, 3);
                packet = render_commands.packet_offset;
                POLY_GT4 *poly = (POLY_GT4 *)psx_addr(packet, sizeof(*poly));
                poly->p[0].rgbc.rgbc = xport_gte_read_data(20);
                poly->p[1].rgbc.rgbc = xport_gte_read_data(21);
                poly->p[2].rgbc.rgbc = xport_gte_read_data(22);
                xport_gte_write_data(6, lighting_cache->control);
                xport_gte_execute(0x780010);

                poly->p[0].xy.xy = screen0->xy;
                poly->p[1].xy.xy = screen1->xy;
                poly->p[2].xy.xy = screen3->xy;
                poly->p[3].xy.xy = screen2->xy;
                poly->p[3].rgbc.rgbc = xport_gte_read_data(22);
                result = render_commands.packet_offset + sizeof(*poly);
                render_commands.packet_offset = result;
            }
            else if (kind == 2)
            {
                packet = render_commands.packet_offset;
                if ((flags & 0x80) != 0)
                {
                    DR_MODE *mode = (DR_MODE *)psx_addr(packet, sizeof(*mode));
                    uint32 *next_tag = (uint32 *)psx_addr(packet + sizeof(*mode), sizeof(*next_tag));
                    link = mode->tag;
                    *next_tag = (link & 0xFFFFFF) | 0x08000000;
                    mode->tag = ((packet + sizeof(*mode)) & 0xFFFFFF) | 0x02000000;
                    mode->code[0] = (flags & 0x60) | 0xE1000000;
                    mode->code[1] = 0;
                    packet += sizeof(*mode);
                }
                POLY_G4 *poly = (POLY_G4 *)psx_addr(packet, sizeof(*poly));
                poly->p[0].rgbc.rgbc = xport_gte_read_data(20);
                poly->p[1].rgbc.rgbc = xport_gte_read_data(21);
                poly->p[2].rgbc.rgbc = xport_gte_read_data(22);
                xport_gte_write_data(6, lighting_cache->control);
                xport_gte_execute(0x780010);
                poly->p[0].xy.xy = screen0->xy;
                poly->p[1].xy.xy = screen1->xy;
                poly->p[2].xy.xy = screen3->xy;
                poly->p[3].xy.xy = screen2->xy;
                poly->p[3].rgbc.rgbc = xport_gte_read_data(22);
                result = packet + sizeof(*poly);
                render_commands.packet_offset = result;
            }
            else if (kind == 4)
            {
                packet = render_commands.packet_offset;
                desc->WritePolygonUV(packet + 12, 2);
                POLY_FT4 *poly = (POLY_FT4 *)psx_addr(packet, sizeof(*poly));
                poly->p[0].xy.xy = screen0->xy;
                poly->p[1].xy.xy = screen1->xy;
                poly->p[2].xy.xy = screen3->xy;
                poly->p[3].xy.xy = screen2->xy;

                poly->rgbc.rgbc = xport_gte_read_data(22);
                render_commands.packet_offset = packet + sizeof(*poly);
            }
            else
            {
                packet = render_commands.packet_offset;
                if ((flags & 0x80) != 0)
                {
                    DR_MODE *mode = (DR_MODE *)psx_addr(packet, sizeof(*mode));
                    uint32 *next_tag = (uint32 *)psx_addr(packet + sizeof(*mode), sizeof(*next_tag));
                    link = mode->tag;
                    *next_tag = (link & 0xFFFFFF) | 0x05000000;
                    mode->tag = ((packet + sizeof(*mode)) & 0xFFFFFF) | 0x02000000;
                    mode->code[0] = (flags & 0x60) | 0xE1000000;
                    mode->code[1] = 0;
                    packet += sizeof(*mode);
                }
                POLY_F4 *poly = (POLY_F4 *)psx_addr(packet, sizeof(*poly));
                poly->p[0].xy = screen0->xy;
                poly->p[1].xy = screen1->xy;
                poly->p[2].xy = screen3->xy;
                poly->p[3].xy = screen2->xy;
                poly->rgbc.rgbc = xport_gte_read_data(22);
                result = packet + sizeof(*poly);
                render_commands.packet_offset = result;
            }
        }
    next_polygon:
        ++faces;
        ++colors;
        --remaining;
    } while (remaining != 0);
}

void MeshGeometryNode::DrawUniformColorFaces(const MeshFace *faces, sint32 count)
{
    uint32 *ordering_entry;
    uint32 *packet_tag;

    POLY_F4 *poly;
    const SceneSection *section;
    const MeshFace *face;
    FUNCTION_MARKER(0x80062E3C, "SLUS_005.78");
    uint32 result, color, packet, remaining, val, sum, limit;
    uint32 first_proj_vertex, idx0, idx1, idx2, idx3;
    const ProjectedVertex *v0, *v1, *v2, *v3;
    sint32 far_distance;
    sint32 near_distance;
    uint32 distance;
    uint32 slot;
    uint32 entry;
    uint32 x;
    sint32 y;
    uint32 z;
    sint32 coefficient, n;
    if ((game_flow.transition_flags & 4) && (faces->tex->flags & 0x40000))
    {
        result = player_interaction.flags;
        if (result)
            return;
    }
    color = render_view.fog_color | 0x28000000;
    packet = render_commands.packet_offset;
    xport_gte_write_data(8, 0);
    remaining = count;
    do
    {
        face = faces;
        val = render_commands.packet_offset;
        limit = render_commands.packet_limit;
        result = val < limit;
        if (!result)
        {
            return;
        }
        coefficient = render_view.inverse_transform.t[0] >> 6;
        n = face->n.x;
        x = (sint64)coefficient * n;
        coefficient = render_view.inverse_transform.t[1] >> 6;
        n = face->n.y;
        y = (sint64)coefficient * n;
        coefficient = render_view.inverse_transform.t[2] >> 6;
        n = face->n.z;
        z = (sint64)coefficient * n;
        val = face->plane;
        result = ((sint32)(x + y + z) >> 6) + val;
        if ((sint32)result <= 0)
            goto next_polygon;
        idx0 = face->vertex_idxs[0];
        first_proj_vertex = render_view.first_proj_vertex;
        idx1 = face->vertex_idxs[1];
        idx3 = face->vertex_idxs[3];
        idx2 = face->vertex_idxs[2];
        v0 = render_view.proj_vertices + (idx0 - first_proj_vertex);
        v1 = render_view.proj_vertices + (idx1 - first_proj_vertex);
        v2 = render_view.proj_vertices + (idx2 - first_proj_vertex);
        v3 = render_view.proj_vertices + (idx3 - first_proj_vertex);
        result = v0->coords.x < 320;
        if (!result)
            result = v1->coords.x < 320;
        if (!result)
            result = v2->coords.x < 320;
        if (!result)
            result = v3->coords.x < 320;
        if (!result)
            goto next_polygon;
        result = v0->coords.y < 240;
        if (!result)
            result = v1->coords.y < 240;
        if (!result)
            result = v2->coords.y < 240;
        if (!result)
            result = v3->coords.y < 240;
        if (!result)
            goto next_polygon;
        x = v0->depth;
        y = v1->depth;
        z = v2->depth;
        val = v3->depth;
        limit = render_view.far_distance;
        sum = x + y + z + val;
        result = (sint32)(sum << 1) < (sint32)limit;
        if (!result)
            goto next_polygon;
        val = (uint16)face->n.depth_bias;
        x = render_view.object_depth_bias;
        y = render_view.fog_scale;
        val = ((sint32)(val << 16) >> 17) + x;
        sum = ((sint32)sum >> 2) + val;
        val = (sint64)(sint32)sum * y;
        slot = (sint32)val >> 19;
        result = slot < 1024;
        if (!result)
        {
            result = 1023;
            if ((sint32)slot < 0)
                slot = 0;
            else
                slot = result;
        }
        section = scene_stream.active_section;
        far_distance = section->fog_far;
        if (far_distance)
        {
            limit = render_view.far_distance;
            distance = mul12((sint32)limit, (sint32)(slot << 2));
            near_distance = section->fog_near;
            result = near_distance < (sint32)distance;
            if (result)
            {
                result = distance - near_distance;
                if ((sint32)distance >= far_distance)
                    goto next_polygon;
                val = render_view.depth_scale;
                val = (sint32)mul12((sint32)val, (sint32)result) >> 8;
                xport_gte_write_data(8, val);
            }
            else
                xport_gte_write_data(8, 0);
        }
        else
            xport_gte_write_data(8, 0);
        entry = render_commands.ordering_table_offset + (slot << 2);
        ordering_entry = (uint32 *)psx_addr(entry, sizeof(uint32));
        val = *ordering_entry;
        packet_tag = (uint32 *)psx_addr(packet, sizeof(uint32));
        *packet_tag = (val & 0xFFFFFF) | 0x05000000;
        val = *ordering_entry;
        *ordering_entry = (val & 0xFF000000) | (packet & 0xFFFFFF);
        xport_gte_write_data(6, color);
        xport_gte_execute(0x780010);
        poly = (POLY_F4 *)psx_addr(packet, sizeof(*poly));
        poly->rgbc.rgbc = xport_gte_read_data(22);
        poly->p[0].xy = v0->xy;
        poly->p[1].xy = v1->xy;
        poly->p[2].xy = v3->xy;
        result = v2->xy;
        poly->p[3].xy = result;
        packet += sizeof(*poly);
    next_polygon:
        --remaining;
        ++faces;
    } while (remaining);
    render_commands.packet_offset = packet;
}

void MeshGeometryNode::DrawFacesWithoutLightingCache(const MeshVertex *vertices, const MeshFace *faces, sint32 count)
{
    uint32 *bucket_entry;
    uint32 *packet_tag;

    const SceneSection *section;
    const MeshFace *face;
    const TextureDescriptor *desc;
    const TextureDescriptor *saved_material;
    const TextureDescriptor *material;
    uint32 material_flags;
    uint32 flags;
    uint32 idx0;
    uint32 idx1;
    uint32 idx2;
    uint32 idx3;
    uint32 first_proj_vertex;
    const ProjectedVertex *screen0;
    const ProjectedVertex *screen1;
    const ProjectedVertex *screen2;
    const ProjectedVertex *screen3;
    const MeshVertex *vertex0;
    const MeshVertex *vertex1;
    const MeshVertex *vertex2;
    const MeshVertex *vertex3;
    uint32 depth;
    sint32 far_distance;
    uint32 sum;
    sint32 product_low;
    uint32 extra;
    uint32 packet;
    uint32 bucket_offset;
    uint32 link;
    uint32 kind;
    uint32 tag;
    uint32 end;
    uint32 start;
    uint32 remaining = count;
    sint32 coord;
    sint32 coefficient;
    sint32 surface;
    sint32 bucket;
    sint32 distance;
    sint32 fade;
    FUNCTION_MARKER(0x800631F0, "SLUS_005.78");
    flags = game_flow.transition_flags;
    saved_material = render_view.material_override;
    if ((flags & 4) != 0 && (faces->tex->flags & 0x40000) != 0)
    {
        if (player_interaction.flags != 0)
            return;
        render_view.material_override = resource_grps[27].tex;
    }
    SceneLighting::TransformObjectLights(0);
    xport_gte_write_data(8, 0);
    do
    {
        face = faces;
        packet = render_commands.packet_offset;
        end = render_commands.packet_limit;
        if (packet >= end)
            return;
        coord = render_view.inverse_transform.t[0];
        coefficient = face->n.x;
        sum = (sint64)(coord >> 6) * coefficient;
        coord = render_view.inverse_transform.t[1];
        coefficient = face->n.y;
        sum += (uint32)((sint64)(coord >> 6) * coefficient);
        coord = render_view.inverse_transform.t[2];
        coefficient = face->n.z;
        sum += (uint32)((sint64)(coord >> 6) * coefficient);
        depth = face->plane;
        surface = ((sint32)sum >> 6) + depth;
        if (surface <= 0)
            goto next_polygon;
        idx0 = face->vertex_idxs[0];
        first_proj_vertex = render_view.first_proj_vertex;
        idx1 = face->vertex_idxs[1];
        idx2 = face->vertex_idxs[2];
        idx3 = face->vertex_idxs[3];
        screen0 = render_view.proj_vertices + (idx0 - first_proj_vertex);
        screen1 = render_view.proj_vertices + (idx1 - first_proj_vertex);
        screen2 = render_view.proj_vertices + (idx2 - first_proj_vertex);
        screen3 = render_view.proj_vertices + (idx3 - first_proj_vertex);
        vertex0 = vertices + (idx0 - first_proj_vertex);
        vertex1 = vertices + (idx1 - first_proj_vertex);
        vertex2 = vertices + (idx2 - first_proj_vertex);
        vertex3 = vertices + (idx3 - first_proj_vertex);
        if (!(screen0->coords.x < 320 || screen1->coords.x < 320 || screen2->coords.x < 320 || screen3->coords.x < 320))
            goto next_polygon;
        if (!(screen0->coords.y < 240 || screen1->coords.y < 240 || screen2->coords.y < 240 || screen3->coords.y < 240))
            goto next_polygon;
        depth = screen0->depth;
        sum = screen1->depth;
        depth += sum;
        sum = screen2->depth;
        depth += sum;
        sum = screen3->depth;
        depth += sum;
        far_distance = render_view.far_distance;
        if ((sint32)(depth << 1) >= far_distance)
            goto next_polygon;
        extra = 0;
        material = render_view.material_override;
        if (material == NULL)
            material = face->tex;
        desc = material;
        material_flags = desc->flags;
        if ((material_flags & 1) != 0 && surface <= 0x3FFFF800)
            extra = 0x80000000;
        coefficient = face->n.depth_bias;
        coord = render_view.object_depth_bias;
        sum = render_view.fog_scale;
        product_low = (sint64)(sint32)(((sint32)depth >> 2) + (uint32)(coefficient >> 1) + coord) * (sint32)sum;
        bucket = product_low >> 19;
        if ((uint32)bucket >= 1024)
            bucket = bucket < 0 ? 0 : 1023;
        SceneLighting::ApplyVertexLights(&vertex0->pos, (material_flags >> 24) | extra);
        xport_gte_write_data(6, desc->mapping.color.rgbc);
        section = scene_stream.active_section;
        end = section->fog_far;
        if (end != 0)
        {
            far_distance = render_view.far_distance;
            distance = mul12(far_distance, (sint32)((uint32)bucket << 2));
            start = section->fog_near;
            if ((sint32)start < distance)
            {
                if (distance >= (sint32)end)
                    goto next_polygon;
                sum = distance - start;
                coefficient = render_view.depth_scale;
                fade = (sint32)mul12(coefficient, (sint32)sum) >> 8;
                xport_gte_write_data(8, fade);
            }
            else
                xport_gte_write_data(8, 0);
        }
        else
            xport_gte_write_data(8, 0);
        kind = material_flags & 6;
        if (kind == 6 || kind == 2)
        {
            gte_ldv0(&vertex0->n);
            gte_ldv1(&vertex1->n);
            gte_ldv2(&vertex3->n);
            xport_gte_execute(0xF80416);
            tag = kind == 6 ? 0x0C000000 : 0x08000000;
        }
        else
        {
            xport_gte_write_data(0, (uint16)face->n.x | ((uint32)face->n.y << 16));
            xport_gte_write_data(1, (uint16)face->n.z | ((uint32)face->n.depth_bias << 16));
            xport_gte_execute(0xE80413);
            tag = kind == 4 ? 0x09000000 : 0x05000000;
        }
        bucket_offset = render_commands.ordering_table_offset + 4 * (uint32)bucket;
        bucket_entry = (uint32 *)psx_addr(bucket_offset, sizeof(uint32));
        link = *bucket_entry;
        packet = render_commands.packet_offset;
        packet_tag = (uint32 *)psx_addr(packet, sizeof(uint32));
        *packet_tag = (link & 0xFFFFFF) | tag;
        link = *bucket_entry;
        *bucket_entry = (link & 0xFF000000) | (packet & 0xFFFFFF);
        if (kind == 6)
        {
            desc->WritePolygonUV(render_commands.packet_offset + 12, 3);
            packet = render_commands.packet_offset;
            POLY_GT4 *poly = (POLY_GT4 *)psx_addr(packet, sizeof(*poly));
            poly->p[0].rgbc.rgbc = xport_gte_read_data(20);
            poly->p[1].rgbc.rgbc = xport_gte_read_data(21);
            poly->p[2].rgbc.rgbc = xport_gte_read_data(22);
            gte_ldv0(&vertex2->n);
            xport_gte_execute(0xE80413);

            poly->p[0].xy.xy = screen0->xy;
            poly->p[1].xy.xy = screen1->xy;
            poly->p[2].xy.xy = screen3->xy;
            poly->p[3].xy.xy = screen2->xy;
            poly->p[3].rgbc.rgbc = xport_gte_read_data(22);
            render_commands.packet_offset = render_commands.packet_offset + sizeof(*poly);
        }
        else if (kind == 4)
        {
            packet = render_commands.packet_offset;
            desc->WritePolygonUV(packet + 12, 2);
            POLY_FT4 *poly = (POLY_FT4 *)psx_addr(packet, sizeof(*poly));
            poly->p[0].xy.xy = screen0->xy;
            poly->p[1].xy.xy = screen1->xy;
            poly->p[2].xy.xy = screen3->xy;
            poly->p[3].xy.xy = screen2->xy;
            poly->rgbc.rgbc = xport_gte_read_data(22);
            render_commands.packet_offset = packet + sizeof(*poly);
        }
        else
        {
            packet = render_commands.packet_offset;
            if ((material_flags & 0x80) != 0)
            {
                DR_MODE *mode = (DR_MODE *)psx_addr(packet, sizeof(*mode));
                uint32 *next_tag = (uint32 *)psx_addr(packet + sizeof(*mode), sizeof(*next_tag));
                link = mode->tag;
                *next_tag = (link & 0xFFFFFF) | tag;
                mode->tag = ((packet + sizeof(*mode)) & 0xFFFFFF) | 0x02000000;
                mode->code[0] = (material_flags & 0x60) | 0xE1000000;
                mode->code[1] = 0;
                packet += sizeof(*mode);
            }
            if (kind == 2)
            {
                POLY_G4 *poly = (POLY_G4 *)psx_addr(packet, sizeof(*poly));
                poly->p[0].rgbc.rgbc = xport_gte_read_data(20);
                poly->p[1].rgbc.rgbc = xport_gte_read_data(21);
                poly->p[2].rgbc.rgbc = xport_gte_read_data(22);
                gte_ldv0(&vertex2->n);
                xport_gte_execute(0xE80413);
                poly->p[0].xy.xy = screen0->xy;
                poly->p[1].xy.xy = screen1->xy;
                poly->p[2].xy.xy = screen3->xy;
                poly->p[3].xy.xy = screen2->xy;
                poly->p[3].rgbc.rgbc = xport_gte_read_data(22);
                render_commands.packet_offset = packet + sizeof(*poly);
            }
            else
            {
                POLY_F4 *poly = (POLY_F4 *)psx_addr(packet, sizeof(*poly));
                poly->rgbc.rgbc = xport_gte_read_data(22);
                poly->p[0].xy = screen0->xy;
                poly->p[1].xy = screen1->xy;
                poly->p[2].xy = screen3->xy;
                poly->p[3].xy = screen2->xy;
                render_commands.packet_offset = packet + sizeof(*poly);
            }
        }
    next_polygon:
        ++faces;
        --remaining;
    } while (remaining != 0);
    render_view.material_override = saved_material;
}
