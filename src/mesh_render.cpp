#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "game.h"
#include "original_defaults.h"

uint32 DrawFacesWithFreshLighting(const MeshVertex *vertices, const MeshFace *faces, sint32 count)
{
    const SceneSection *section;
    const MeshFace *face;
    const TextureDescriptor *descriptor;
    FUNCTION_MARKER(0x800615C4, "SLUS_005.78");
    MeshLighting *lighting_cache;
    MeshLighting *colors;
    uint32 remaining = (uint32)count;
    uint32 packet, limit, sum, depth, value, index0, index1, index2, index3, first_projected_vertex;
    const ProjectedVertex *screen0, *screen1, *screen2, *screen3;
    const MeshVertex *vertex0, *vertex1, *vertex2, *vertex3;
    uint32 flags, start, end, kind, bucket_offset, link, tag, fourth_color, result;
    sint32 coordinate, coefficient, surface, bucket, distance, fade;
    sint64 product;
    TransformObjectLights(0);
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
        coordinate = render_view.inverse_transform.t[0] >> 6;
        coefficient = face->normal.vector.vx;
        sum = (uint32)((sint64)coordinate * coefficient);
        coordinate = render_view.inverse_transform.t[1] >> 6;
        coefficient = face->normal.vector.vy;
        sum += (uint32)((sint64)coordinate * coefficient);
        coordinate = render_view.inverse_transform.t[2] >> 6;
        coefficient = face->normal.vector.vz;
        value = face->plane;
        sum += (uint32)((sint64)coordinate * coefficient);
        result = (uint32)((sint32)sum >> 6) + value;
        surface = (sint32)result;
        if (surface <= 0)
            goto next_polygon;
        index0 = face->vertex_indices[0];
        first_projected_vertex = render_view.first_projected_vertex;
        index1 = face->vertex_indices[1];
        index2 = face->vertex_indices[2];
        index3 = face->vertex_indices[3];
        if (index0 < first_projected_vertex || index1 < first_projected_vertex || index2 < first_projected_vertex || index3 < first_projected_vertex)
        {
            fprintf(stderr, "P2: Face index precedes projected vertex range\n");
            abort();
        }
        screen0 = render_view.projected_vertices + (index0 - first_projected_vertex);
        screen1 = render_view.projected_vertices + (index1 - first_projected_vertex);
        screen2 = render_view.projected_vertices + (index2 - first_projected_vertex);
        screen3 = render_view.projected_vertices + (index3 - first_projected_vertex);
        vertex0 = vertices + (index0 - first_projected_vertex);
        vertex1 = vertices + (index1 - first_projected_vertex);
        vertex2 = vertices + (index2 - first_projected_vertex);
        vertex3 = vertices + (index3 - first_projected_vertex);
        if (screen0->coordinates.x >= 320 && screen1->coordinates.x >= 320 && screen2->coordinates.x >= 320 && screen3->coordinates.x >= 320)
        {
            result = 0;
            goto next_polygon;
        }
        if (screen0->coordinates.y >= 240 && screen1->coordinates.y >= 240 && screen2->coordinates.y >= 240 && screen3->coordinates.y >= 240)
        {
            result = 0;
            goto next_polygon;
        }
        depth = screen0->depth;
        sum = screen1->depth;
        value = screen2->depth;
        link = screen3->depth;
        limit = render_view.far_distance;
        depth += sum + value + link;
        result = (sint32)(depth << 1) < (sint32)limit;
        if (result == 0)
            goto next_polygon;
        descriptor = ((const TextureDescriptor *)(face->texture));
        flags = descriptor->flags;
        if ((flags & 1) != 0 && surface <= 0x3FFFF800)
            lighting_cache->control = 0x80000000;
        coefficient = face->normal.components.depth_bias;
        value = render_view.fog_scale;
        sum = (uint32)((sint32)depth >> 2) + (uint32)(coefficient >> 1);
        bucket = (sint32)(sum * value) >> 19;
        if ((uint32)bucket >= 1024)
            bucket = bucket < 0 ? 0 : 1023;
        value = lighting_cache->control;
        ApplyVertexLights(&vertex0->position, (flags >> 24) | (value & 0x80000000));
        xport_gte_write_data(6, descriptor->payload.words[3]);
        section = scene_stream.active_section;
        end = section->origin[2];
        if (end != 0)
        {
            value = render_view.far_distance;
            product = (sint64)(sint32)value * (sint32)((uint32)bucket << 2);
            distance = (sint32)((uint64)product >> 12);
            start = section->origin[1];
            if ((sint32)start < distance)
            {
                result = (uint32)distance - start;
                if (distance >= (sint32)end)
                    goto next_polygon;
                coefficient = ((sint32)render_view.depth_scale);
                product = (sint64)coefficient * (sint32)result;
                fade = (sint32)((uint64)product >> 12) >> 8;
                xport_gte_write_data(8, (uint32)fade);
            }
            else
                xport_gte_write_data(8, 0);
        }
        else
            xport_gte_write_data(8, 0);
        kind = flags & 6;
        if (kind == 6 || kind == 2)
        {
            xport_gte_write_data(0, xport_load_le32(&vertex0->normal));
            xport_gte_write_data(1, xport_load_le32((const uint8 *)&vertex0->normal + 4));
            xport_gte_write_data(2, xport_load_le32(&vertex1->normal));
            xport_gte_write_data(3, xport_load_le32((const uint8 *)&vertex1->normal + 4));
            xport_gte_write_data(4, xport_load_le32(&vertex3->normal));
            xport_gte_write_data(5, xport_load_le32((const uint8 *)&vertex3->normal + 4));
            xport_gte_execute(0x118043F);
            tag = kind == 6 ? 0x0C000000 : 0x08000000;
        }
        else
        {
            xport_gte_write_data(0, xport_load_le32(&face->normal.vector));
            xport_gte_write_data(1, xport_load_le32((const uint8 *)&face->normal.vector + 4));
            xport_gte_execute(0x108041B);
            tag = kind == 4 ? 0x09000000 : 0x05000000;
        }
        bucket_offset = render_commands.ordering_table_offset + (uint32)bucket * 4;
        link = r_u32(bucket_offset);
        packet = render_commands.packet_offset;
        w_u32(packet, (link & 0xFFFFFF) | tag);
        link = r_u32(bucket_offset);
        w_u32(bucket_offset, (link & 0xFF000000) | (packet & 0xFFFFFF));
        if (kind == 6)
        {
            descriptor->WritePolygonUV(render_commands.packet_offset + 12, 3);
            lighting_cache->colors[0] = xport_gte_read_data(20);
            lighting_cache->colors[1] = xport_gte_read_data(21);
            lighting_cache->colors[2] = xport_gte_read_data(22);
            xport_gte_write_data(0, xport_load_le32(&vertex2->normal));
            xport_gte_write_data(1, xport_load_le32((const uint8 *)&vertex2->normal + 4));
            xport_gte_execute(0x108041B);
            packet = render_commands.packet_offset;
            w_u32(packet + 8, screen0->xy);
            fourth_color = xport_gte_read_data(22);
            xport_gte_write_data(20, lighting_cache->colors[0]);
            xport_gte_write_data(21, lighting_cache->colors[1]);
            xport_gte_write_data(22, lighting_cache->colors[2]);
            xport_gte_write_data(6, lighting_cache->colors[2]);
            xport_gte_execute(0xF8002A);
            packet = render_commands.packet_offset;
            w_u32(packet + 20, screen1->xy);
            w_u32(packet + 32, screen3->xy);
            w_u32(packet + 44, screen2->xy);
            packet = render_commands.packet_offset;
            w_u32(packet + 4, xport_gte_read_data(20));
            w_u32(packet + 16, xport_gte_read_data(21));
            w_u32(packet + 28, xport_gte_read_data(22));
            xport_gte_write_data(6, fourth_color);
            xport_gte_execute(0x780010);
            lighting_cache->control = lighting_cache->control | 0x40000000 | (fourth_color & 0xFFFFFF);
            packet = render_commands.packet_offset;
            w_u32(packet + 40, xport_gte_read_data(22));
            result = render_commands.packet_offset + 52;
            render_commands.packet_offset = (result);
        }
        else if (kind == 2)
        {
            packet = render_commands.packet_offset;
            if ((flags & 0x80) != 0)
            {
                link = r_u32(packet);
                w_u32(packet + 12, (link & 0xFFFFFF) | 0x08000000);
                w_u32(packet, ((packet + 12) & 0xFFFFFF) | 0x02000000);
                w_u32(packet + 4, (flags & 0x60) | 0xE1000000);
                w_u32(packet + 8, 0);
                packet += 12;
            }
            lighting_cache->colors[0] = xport_gte_read_data(20);
            lighting_cache->colors[1] = xport_gte_read_data(21);
            lighting_cache->colors[2] = xport_gte_read_data(22);
            xport_gte_write_data(0, xport_load_le32(&vertex2->normal));
            xport_gte_write_data(1, xport_load_le32((const uint8 *)&vertex2->normal + 4));
            xport_gte_execute(0x108041B);
            w_u32(packet + 8, screen0->xy);
            fourth_color = xport_gte_read_data(22);
            xport_gte_write_data(20, lighting_cache->colors[0]);
            xport_gte_write_data(21, lighting_cache->colors[1]);
            xport_gte_write_data(22, lighting_cache->colors[2]);
            xport_gte_write_data(6, lighting_cache->colors[2]);
            xport_gte_execute(0xF8002A);
            w_u32(packet + 16, screen1->xy);
            w_u32(packet + 24, screen3->xy);
            w_u32(packet + 32, screen2->xy);
            w_u32(packet + 4, xport_gte_read_data(20));
            w_u32(packet + 12, xport_gte_read_data(21));
            w_u32(packet + 20, xport_gte_read_data(22));
            xport_gte_write_data(6, fourth_color);
            xport_gte_execute(0x780010);
            lighting_cache->control = lighting_cache->control | 0x40000000 | (fourth_color & 0xFFFFFF);
            w_u32(packet + 28, xport_gte_read_data(22));
            result = packet + 36;
            render_commands.packet_offset = (result);
        }
        else if (kind == 4)
        {
            packet = render_commands.packet_offset;
            descriptor->WritePolygonUV(packet + 12, 2);
            lighting_cache->colors[0] = xport_gte_read_data(22);
            xport_gte_write_data(6, lighting_cache->colors[0]);
            xport_gte_execute(0x780010);
            w_u32(packet + 8, screen0->xy);
            w_u32(packet + 16, screen1->xy);
            w_u32(packet + 24, screen3->xy);
            w_u32(packet + 32, screen2->xy);
            w_u32(packet + 4, xport_gte_read_data(22));
            result = lighting_cache->control | 0x40000000;
            lighting_cache->control = result;
            render_commands.packet_offset = (packet + 40);
        }
        else
        {
            packet = render_commands.packet_offset;
            if ((flags & 0x80) != 0)
            {
                link = r_u32(packet);
                w_u32(packet + 12, (link & 0xFFFFFF) | 0x05000000);
                w_u32(packet, ((packet + 12) & 0xFFFFFF) | 0x02000000);
                w_u32(packet + 4, (flags & 0x60) | 0xE1000000);
                w_u32(packet + 8, 0);
                packet += 12;
            }
            lighting_cache->colors[0] = xport_gte_read_data(22);
            xport_gte_write_data(6, lighting_cache->colors[0]);
            xport_gte_execute(0x780010);
            w_u32(packet + 8, screen0->xy);
            w_u32(packet + 12, screen1->xy);
            w_u32(packet + 16, screen3->xy);
            w_u32(packet + 20, screen2->xy);
            w_u32(packet + 4, xport_gte_read_data(22));
            lighting_cache->control = lighting_cache->control | 0x40000000;
            result = packet + 24;
            render_commands.packet_offset = (result);
        }
    next_polygon:
        ++faces;
        ++colors;
        --remaining;
    } while (remaining != 0);
    return result;
}

uint32 DrawFacesWithCachedLighting(const MeshVertex *vertices, const MeshFace *faces, sint32 count)
{
    const SceneSection *section;
    const MeshFace *face;
    const TextureDescriptor *descriptor;
    FUNCTION_MARKER(0x80061F50, "SLUS_005.78");
    MeshLighting *lighting_cache;
    MeshLighting *colors;
    uint32 remaining = (uint32)count;
    uint32 packet, limit, sum, depth, value, index0, index1, index2, index3, first_projected_vertex;
    const ProjectedVertex *screen0, *screen1, *screen2, *screen3;
    const MeshVertex *vertex0, *vertex1, *vertex2, *vertex3;
    uint32 flags, start, end, kind, bucket_offset, link, tag, fourth_color, result;
    sint32 coordinate, coefficient, surface, bucket, distance, fade;
    sint64 product;
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
        coordinate = render_view.inverse_transform.t[0] >> 6;
        coefficient = face->normal.vector.vx;
        sum = (uint32)((sint64)coordinate * coefficient);
        coordinate = render_view.inverse_transform.t[1] >> 6;
        coefficient = face->normal.vector.vy;
        sum += (uint32)((sint64)coordinate * coefficient);
        coordinate = render_view.inverse_transform.t[2] >> 6;
        coefficient = face->normal.vector.vz;
        value = face->plane;
        sum += (uint32)((sint64)coordinate * coefficient);
        result = (uint32)((sint32)sum >> 6) + value;
        surface = (sint32)result;
        if (surface <= 0)
            goto next_polygon;
        index0 = face->vertex_indices[0];
        first_projected_vertex = render_view.first_projected_vertex;
        index1 = face->vertex_indices[1];
        index3 = face->vertex_indices[3];
        index2 = face->vertex_indices[2];
        if (index0 < first_projected_vertex || index1 < first_projected_vertex || index2 < first_projected_vertex || index3 < first_projected_vertex)
        {
            fprintf(stderr, "P2: Face index precedes projected vertex range\n");
            abort();
        }
        screen0 = render_view.projected_vertices + (index0 - first_projected_vertex);
        screen1 = render_view.projected_vertices + (index1 - first_projected_vertex);
        screen2 = render_view.projected_vertices + (index2 - first_projected_vertex);
        screen3 = render_view.projected_vertices + (index3 - first_projected_vertex);
        if (screen0->coordinates.x >= 320 && screen1->coordinates.x >= 320 && screen2->coordinates.x >= 320 && screen3->coordinates.x >= 320)
        {
            result = 0;
            goto next_polygon;
        }
        if (screen0->coordinates.y >= 240 && screen1->coordinates.y >= 240 && screen2->coordinates.y >= 240 && screen3->coordinates.y >= 240)
        {
            result = 0;
            goto next_polygon;
        }
        depth = screen0->depth;
        sum = screen1->depth;
        value = screen2->depth;
        link = screen3->depth;
        limit = render_view.far_distance;
        depth += sum + value + link;
        result = (sint32)(depth << 1) < (sint32)limit;
        if (result == 0)
            goto next_polygon;
        descriptor = ((const TextureDescriptor *)(face->texture));
        flags = descriptor->flags;
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
        coefficient = face->normal.components.depth_bias;
        value = render_view.fog_scale;
        sum = (uint32)((sint32)depth >> 2) + (uint32)(coefficient >> 1);
        bucket = (sint32)(sum * value) >> 19;
        if ((uint32)bucket >= 1024)
            bucket = bucket < 0 ? 0 : 1023;
        section = scene_stream.active_section;
        end = section->origin[2];
        if (end != 0)
        {
            value = render_view.far_distance;
            product = (sint64)(sint32)value * (sint32)((uint32)bucket << 2);
            distance = (sint32)((uint64)product >> 12);
            start = section->origin[1];
            if ((sint32)start < distance)
            {
                result = (uint32)distance - start;
                if (distance >= (sint32)end)
                    goto next_polygon;
                coefficient = ((sint32)render_view.depth_scale);
                product = (sint64)coefficient * (sint32)result;
                fade = (sint32)((uint64)product >> 12) >> 8;
                xport_gte_write_data(8, (uint32)fade);
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
                TransformObjectLights(0);
                game_flow.transition_flags = game_flow.transition_flags | 1;
            }
            vertex0 = vertices + (index0 - first_projected_vertex);
            vertex1 = vertices + (index1 - first_projected_vertex);
            vertex3 = vertices + (index3 - first_projected_vertex);
            vertex2 = vertices + (index2 - first_projected_vertex);
            value = lighting_cache->control;
            ApplyVertexLights(&vertex0->position, (flags >> 24) | (value & 0x80000000));
            xport_gte_write_data(6, descriptor->payload.words[3]);
            if (kind == 6 || kind == 2)
            {
                xport_gte_write_data(0, xport_load_le32(&vertex0->normal));
                xport_gte_write_data(1, xport_load_le32((const uint8 *)&vertex0->normal + 4));
                xport_gte_write_data(2, xport_load_le32(&vertex1->normal));
                xport_gte_write_data(3, xport_load_le32((const uint8 *)&vertex1->normal + 4));
                xport_gte_write_data(4, xport_load_le32(&vertex3->normal));
                xport_gte_write_data(5, xport_load_le32((const uint8 *)&vertex3->normal + 4));
                xport_gte_execute(0x118043F);
                tag = kind == 6 ? 0x0C000000 : 0x08000000;
            }
            else
            {
                xport_gte_write_data(0, xport_load_le32(&face->normal.vector));
                xport_gte_write_data(1, xport_load_le32((const uint8 *)&face->normal.vector + 4));
                xport_gte_execute(0x108041B);
                tag = kind == 4 ? 0x09000000 : 0x05000000;
            }
            bucket_offset = render_commands.ordering_table_offset + (uint32)bucket * 4;
            link = r_u32(bucket_offset);
            packet = render_commands.packet_offset;
            w_u32(packet, (link & 0xFFFFFF) | tag);
            link = r_u32(bucket_offset);
            w_u32(bucket_offset, (link & 0xFF000000) | (packet & 0xFFFFFF));
            if (kind == 6)
            {
                descriptor->WritePolygonUV(render_commands.packet_offset + 12, 3);
                lighting_cache->colors[0] = xport_gte_read_data(20);
                lighting_cache->colors[1] = xport_gte_read_data(21);
                lighting_cache->colors[2] = xport_gte_read_data(22);
                xport_gte_write_data(0, xport_load_le32(&vertex2->normal));
                xport_gte_write_data(1, xport_load_le32((const uint8 *)&vertex2->normal + 4));
                xport_gte_execute(0x108041B);
                packet = render_commands.packet_offset;
                w_u32(packet + 8, screen0->xy);
                fourth_color = xport_gte_read_data(22);
                xport_gte_write_data(20, lighting_cache->colors[0]);
                xport_gte_write_data(21, lighting_cache->colors[1]);
                xport_gte_write_data(22, lighting_cache->colors[2]);
                xport_gte_write_data(6, lighting_cache->colors[2]);
                xport_gte_execute(0xF8002A);
                packet = render_commands.packet_offset;
                w_u32(packet + 20, screen1->xy);
                w_u32(packet + 32, screen3->xy);
                w_u32(packet + 44, screen2->xy);
                packet = render_commands.packet_offset;
                w_u32(packet + 4, xport_gte_read_data(20));
                w_u32(packet + 16, xport_gte_read_data(21));
                w_u32(packet + 28, xport_gte_read_data(22));
                xport_gte_write_data(6, fourth_color);
                xport_gte_execute(0x780010);
                lighting_cache->control = lighting_cache->control | 0x40000000 | (fourth_color & 0xFFFFFF);
                packet = render_commands.packet_offset;
                w_u32(packet + 40, xport_gte_read_data(22));
                result = render_commands.packet_offset + 52;
                render_commands.packet_offset = result;
            }
            else if (kind == 2)
            {
                packet = render_commands.packet_offset;
                if ((flags & 0x80) != 0)
                {
                    link = r_u32(packet);
                    w_u32(packet + 12, (link & 0xFFFFFF) | 0x08000000);
                    w_u32(packet, ((packet + 12) & 0xFFFFFF) | 0x02000000);
                    w_u32(packet + 4, (flags & 0x60) | 0xE1000000);
                    w_u32(packet + 8, 0);
                    packet += 12;
                }
                lighting_cache->colors[0] = xport_gte_read_data(20);
                lighting_cache->colors[1] = xport_gte_read_data(21);
                lighting_cache->colors[2] = xport_gte_read_data(22);
                xport_gte_write_data(0, xport_load_le32(&vertex2->normal));
                xport_gte_write_data(1, xport_load_le32((const uint8 *)&vertex2->normal + 4));
                xport_gte_execute(0x108041B);
                w_u32(packet + 8, screen0->xy);
                fourth_color = xport_gte_read_data(22);
                xport_gte_write_data(20, lighting_cache->colors[0]);
                xport_gte_write_data(21, lighting_cache->colors[1]);
                xport_gte_write_data(22, lighting_cache->colors[2]);
                xport_gte_write_data(6, lighting_cache->colors[2]);
                xport_gte_execute(0xF8002A);
                w_u32(packet + 16, screen1->xy);
                w_u32(packet + 24, screen3->xy);
                w_u32(packet + 32, screen2->xy);
                w_u32(packet + 4, xport_gte_read_data(20));
                w_u32(packet + 12, xport_gte_read_data(21));
                w_u32(packet + 20, xport_gte_read_data(22));
                xport_gte_write_data(6, fourth_color);
                xport_gte_execute(0x780010);
                lighting_cache->control = lighting_cache->control | 0x40000000 | (fourth_color & 0xFFFFFF);
                w_u32(packet + 28, xport_gte_read_data(22));
                result = packet + 36;
                render_commands.packet_offset = result;
            }
            else if (kind == 4)
            {
                packet = render_commands.packet_offset;
                descriptor->WritePolygonUV(packet + 12, 2);
                lighting_cache->colors[0] = xport_gte_read_data(22);
                xport_gte_write_data(6, lighting_cache->colors[0]);
                xport_gte_execute(0x780010);
                w_u32(packet + 8, screen0->xy);
                w_u32(packet + 16, screen1->xy);
                w_u32(packet + 24, screen3->xy);
                w_u32(packet + 32, screen2->xy);
                w_u32(packet + 4, xport_gte_read_data(22));
                result = lighting_cache->control | 0x40000000;
                lighting_cache->control = result;
                render_commands.packet_offset = packet + 40;
            }
            else
            {
                packet = render_commands.packet_offset;
                if ((flags & 0x80) != 0)
                {
                    link = r_u32(packet);
                    w_u32(packet + 12, (link & 0xFFFFFF) | 0x05000000);
                    w_u32(packet, ((packet + 12) & 0xFFFFFF) | 0x02000000);
                    w_u32(packet + 4, (flags & 0x60) | 0xE1000000);
                    w_u32(packet + 8, 0);
                    packet += 12;
                }
                lighting_cache->colors[0] = xport_gte_read_data(22);
                xport_gte_write_data(6, lighting_cache->colors[0]);
                xport_gte_execute(0x780010);
                w_u32(packet + 8, screen0->xy);
                w_u32(packet + 12, screen1->xy);
                w_u32(packet + 16, screen3->xy);
                w_u32(packet + 20, screen2->xy);
                w_u32(packet + 4, xport_gte_read_data(22));
                lighting_cache->control = lighting_cache->control | 0x40000000;
                result = packet + 24;
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
            link = r_u32(bucket_offset);
            packet = render_commands.packet_offset;
            w_u32(packet, (link & 0xFFFFFF) | tag);
            link = r_u32(bucket_offset);
            w_u32(bucket_offset, (link & 0xFF000000) | (packet & 0xFFFFFF));
            if (kind == 6)
            {
                descriptor->WritePolygonUV(render_commands.packet_offset + 12, 3);
                packet = render_commands.packet_offset;
                w_u32(packet + 4, xport_gte_read_data(20));
                w_u32(packet + 16, xport_gte_read_data(21));
                w_u32(packet + 28, xport_gte_read_data(22));
                xport_gte_write_data(6, lighting_cache->control);
                xport_gte_execute(0x780010);
                packet = render_commands.packet_offset;
                w_u32(packet + 8, screen0->xy);
                w_u32(packet + 20, screen1->xy);
                w_u32(packet + 32, screen3->xy);
                w_u32(packet + 44, screen2->xy);
                w_u32(packet + 40, xport_gte_read_data(22));
                result = render_commands.packet_offset + 52;
                render_commands.packet_offset = result;
            }
            else if (kind == 2)
            {
                packet = render_commands.packet_offset;
                if ((flags & 0x80) != 0)
                {
                    link = r_u32(packet);
                    w_u32(packet + 12, (link & 0xFFFFFF) | 0x08000000);
                    w_u32(packet, ((packet + 12) & 0xFFFFFF) | 0x02000000);
                    w_u32(packet + 4, (flags & 0x60) | 0xE1000000);
                    w_u32(packet + 8, 0);
                    packet += 12;
                }
                w_u32(packet + 4, xport_gte_read_data(20));
                w_u32(packet + 12, xport_gte_read_data(21));
                w_u32(packet + 20, xport_gte_read_data(22));
                xport_gte_write_data(6, lighting_cache->control);
                xport_gte_execute(0x780010);
                w_u32(packet + 8, screen0->xy);
                w_u32(packet + 16, screen1->xy);
                w_u32(packet + 24, screen3->xy);
                w_u32(packet + 32, screen2->xy);
                w_u32(packet + 28, xport_gte_read_data(22));
                result = packet + 36;
                render_commands.packet_offset = result;
            }
            else if (kind == 4)
            {
                packet = render_commands.packet_offset;
                descriptor->WritePolygonUV(packet + 12, 2);
                w_u32(packet + 8, screen0->xy);
                w_u32(packet + 16, screen1->xy);
                w_u32(packet + 24, screen3->xy);
                w_u32(packet + 32, screen2->xy);
                result = packet + 4;
                w_u32(result, xport_gte_read_data(22));
                render_commands.packet_offset = packet + 40;
            }
            else
            {
                packet = render_commands.packet_offset;
                if ((flags & 0x80) != 0)
                {
                    link = r_u32(packet);
                    w_u32(packet + 12, (link & 0xFFFFFF) | 0x05000000);
                    w_u32(packet, ((packet + 12) & 0xFFFFFF) | 0x02000000);
                    w_u32(packet + 4, (flags & 0x60) | 0xE1000000);
                    w_u32(packet + 8, 0);
                    packet += 12;
                }
                w_u32(packet + 8, screen0->xy);
                w_u32(packet + 12, screen1->xy);
                w_u32(packet + 16, screen3->xy);
                w_u32(packet + 20, screen2->xy);
                w_u32(packet + 4, xport_gte_read_data(22));
                result = packet + 24;
                render_commands.packet_offset = result;
            }
        }
    next_polygon:
        ++faces;
        ++colors;
        --remaining;
    } while (remaining != 0);
    return result;
}

uint32 DrawUniformColorFaces(const MeshFace *faces, sint32 count)
{
    const SceneSection *section;
    const MeshFace *face;
    FUNCTION_MARKER(0x80062E3C, "SLUS_005.78");
    uint32 result, color, packet, remaining, value, sum, limit;
    uint32 first_projected_vertex, first_index, second_index, third_index, fourth_index;
    const ProjectedVertex *first, *second, *third, *fourth;
    uint32 far_distance, near_distance, distance, slot, entry, x, y, z;
    sint32 coefficient, normal;
    sint64 product;
    if ((game_flow.transition_flags & 4) && (((const TextureDescriptor *)(faces->texture))->flags & 0x40000))
    {
        result = player_interaction.flags;
        if (result)
            return result;
    }
    color = render_view.fog_color | 0x28000000;
    packet = render_commands.packet_offset;
    xport_gte_write_data(8, 0);
    remaining = (uint32)count;
    do
    {
        face = faces;
        value = render_commands.packet_offset;
        limit = render_commands.packet_limit;
        result = value < limit;
        if (!result)
        {
            return result;
        }
        coefficient = render_view.inverse_transform.t[0] >> 6;
        normal = face->normal.vector.vx;
        x = (uint32)((sint64)coefficient * normal);
        coefficient = render_view.inverse_transform.t[1] >> 6;
        normal = face->normal.vector.vy;
        y = (uint32)((sint64)coefficient * normal);
        coefficient = render_view.inverse_transform.t[2] >> 6;
        normal = face->normal.vector.vz;
        z = (uint32)((sint64)coefficient * normal);
        value = face->plane;
        result = (uint32)((sint32)(x + y + z) >> 6) + value;
        if ((sint32)result <= 0)
            goto next_polygon_80062e3c;
        first_index = face->vertex_indices[0];
        first_projected_vertex = render_view.first_projected_vertex;
        second_index = face->vertex_indices[1];
        fourth_index = face->vertex_indices[3];
        third_index = face->vertex_indices[2];
        if (first_index < first_projected_vertex || second_index < first_projected_vertex || third_index < first_projected_vertex || fourth_index < first_projected_vertex)
        {
            fprintf(stderr, "P2: Face index precedes projected vertex range\n");
            abort();
        }
        first = render_view.projected_vertices + (first_index - first_projected_vertex);
        second = render_view.projected_vertices + (second_index - first_projected_vertex);
        third = render_view.projected_vertices + (third_index - first_projected_vertex);
        fourth = render_view.projected_vertices + (fourth_index - first_projected_vertex);
        result = first->coordinates.x < 320;
        if (!result)
            result = second->coordinates.x < 320;
        if (!result)
            result = third->coordinates.x < 320;
        if (!result)
            result = fourth->coordinates.x < 320;
        if (!result)
            goto next_polygon_80062e3c;
        result = first->coordinates.y < 240;
        if (!result)
            result = second->coordinates.y < 240;
        if (!result)
            result = third->coordinates.y < 240;
        if (!result)
            result = fourth->coordinates.y < 240;
        if (!result)
            goto next_polygon_80062e3c;
        x = first->depth;
        y = second->depth;
        z = third->depth;
        value = fourth->depth;
        limit = render_view.far_distance;
        sum = x + y + z + value;
        result = (sint32)(sum << 1) < (sint32)limit;
        if (!result)
            goto next_polygon_80062e3c;
        value = (uint16)face->normal.components.depth_bias;
        x = (uint32)(sint32)(sint16)render_view.object_depth_bias;
        y = render_view.fog_scale;
        value = (uint32)((sint32)(value << 16) >> 17) + x;
        sum = (uint32)((sint32)sum >> 2) + value;
        value = (uint32)((sint64)(sint32)sum * (sint32)y);
        slot = (uint32)((sint32)value >> 19);
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
        far_distance = section->origin[2];
        if (far_distance)
        {
            limit = render_view.far_distance;
            product = (sint64)(sint32)limit * (sint32)(slot << 2);
            distance = (uint32)(product >> 12);
            near_distance = section->origin[1];
            result = (sint32)near_distance < (sint32)distance;
            if (result)
            {
                result = distance - near_distance;
                if ((sint32)distance >= (sint32)far_distance)
                    goto next_polygon_80062e3c;
                value = render_view.depth_scale;
                product = (sint64)(sint32)value * (sint32)result;
                value = (uint32)((sint32)(uint32)(product >> 12) >> 8);
                xport_gte_write_data(8, value);
            }
            else
                xport_gte_write_data(8, 0);
        }
        else
            xport_gte_write_data(8, 0);
        entry = render_commands.ordering_table_offset + (slot << 2);
        value = r_u32(entry);
        w_u32(packet, (value & 0xFFFFFF) | 0x05000000);
        value = r_u32(entry);
        w_u32(entry, (value & 0xFF000000) | (packet & 0xFFFFFF));
        xport_gte_write_data(6, color);
        xport_gte_execute(0x780010);
        w_u32(packet + 4, xport_gte_read_data(22));
        w_u32(packet + 8, first->xy);
        w_u32(packet + 12, second->xy);
        w_u32(packet + 16, fourth->xy);
        result = third->xy;
        w_u32(packet + 20, result);
        packet += 24;
    next_polygon_80062e3c:
        --remaining;
        ++faces;
    } while (remaining);
    render_commands.packet_offset = packet;
    return result;
}

void DrawFacesWithoutLightingCache(const MeshVertex *vertices, const MeshFace *faces, sint32 count)
{
    const SceneSection *section;
    const MeshFace *face;
    const TextureDescriptor *descriptor;
    const TextureDescriptor *saved_material;
    const TextureDescriptor *material;
    uint32 material_flags;
    uint32 flags;
    uint32 first_index;
    uint32 second_index;
    uint32 third_index;
    uint32 fourth_index;
    uint32 first_projected_vertex;
    const ProjectedVertex *screen0;
    const ProjectedVertex *screen1;
    const ProjectedVertex *screen2;
    const ProjectedVertex *screen3;
    const MeshVertex *vertex0;
    const MeshVertex *vertex1;
    const MeshVertex *vertex2;
    const MeshVertex *vertex3;
    uint32 depth;
    uint32 far_distance;
    uint32 sum;
    uint32 product_low;
    uint32 extra;
    uint32 packet;
    uint32 bucket_offset;
    uint32 link;
    uint32 kind;
    uint32 tag;
    uint32 end;
    uint32 start;
    uint32 remaining = (uint32)count;
    sint32 coordinate;
    sint32 coefficient;
    sint32 surface;
    sint32 bucket;
    sint32 distance;
    sint32 fade;
    sint64 product;
    FUNCTION_MARKER(0x800631F0, "SLUS_005.78");
    flags = game_flow.transition_flags;
    saved_material = render_view.material_override;
    if ((flags & 4) != 0 && (((const TextureDescriptor *)(faces->texture))->flags & 0x40000) != 0)
    {
        if (player_interaction.flags != 0)
            return;
        render_view.material_override = resource_groups[27].texture;
    }
    TransformObjectLights(0);
    xport_gte_write_data(8, 0);
    do
    {
        face = faces;
        packet = render_commands.packet_offset;
        end = render_commands.packet_limit;
        if (packet >= end)
            return;
        coordinate = render_view.inverse_transform.t[0];
        coefficient = face->normal.vector.vx;
        sum = (uint32)((sint64)(coordinate >> 6) * coefficient);
        coordinate = render_view.inverse_transform.t[1];
        coefficient = face->normal.vector.vy;
        sum += (uint32)((sint64)(coordinate >> 6) * coefficient);
        coordinate = render_view.inverse_transform.t[2];
        coefficient = face->normal.vector.vz;
        sum += (uint32)((sint64)(coordinate >> 6) * coefficient);
        depth = face->plane;
        surface = (sint32)((uint32)((sint32)sum >> 6) + depth);
        if (surface <= 0)
            goto next_polygon;
        first_index = face->vertex_indices[0];
        first_projected_vertex = render_view.first_projected_vertex;
        second_index = face->vertex_indices[1];
        third_index = face->vertex_indices[2];
        fourth_index = face->vertex_indices[3];
        if (first_index < first_projected_vertex || second_index < first_projected_vertex || third_index < first_projected_vertex || fourth_index < first_projected_vertex)
        {
            fprintf(stderr, "P2: Face index precedes projected vertex range\n");
            abort();
        }
        screen0 = render_view.projected_vertices + (first_index - first_projected_vertex);
        screen1 = render_view.projected_vertices + (second_index - first_projected_vertex);
        screen2 = render_view.projected_vertices + (third_index - first_projected_vertex);
        screen3 = render_view.projected_vertices + (fourth_index - first_projected_vertex);
        vertex0 = vertices + (first_index - first_projected_vertex);
        vertex1 = vertices + (second_index - first_projected_vertex);
        vertex2 = vertices + (third_index - first_projected_vertex);
        vertex3 = vertices + (fourth_index - first_projected_vertex);
        if (!(screen0->coordinates.x < 320 || screen1->coordinates.x < 320 || screen2->coordinates.x < 320 || screen3->coordinates.x < 320))
            goto next_polygon;
        if (!(screen0->coordinates.y < 240 || screen1->coordinates.y < 240 || screen2->coordinates.y < 240 || screen3->coordinates.y < 240))
            goto next_polygon;
        depth = screen0->depth;
        sum = screen1->depth;
        depth += sum;
        sum = screen2->depth;
        depth += sum;
        sum = screen3->depth;
        depth += sum;
        far_distance = render_view.far_distance;
        if ((sint32)(depth << 1) >= (sint32)far_distance)
            goto next_polygon;
        extra = 0;
        material = render_view.material_override;
        if (material == NULL)
            material = ((const TextureDescriptor *)(face->texture));
        descriptor = material;
        material_flags = descriptor->flags;
        if ((material_flags & 1) != 0 && surface <= 0x3FFFF800)
            extra = 0x80000000;
        coefficient = face->normal.components.depth_bias;
        coordinate = (sint16)render_view.object_depth_bias;
        sum = render_view.fog_scale;
        product_low = (uint32)((sint64)(sint32)((uint32)((sint32)depth >> 2) + (uint32)(coefficient >> 1) + (uint32)coordinate) * (sint32)sum);
        bucket = (sint32)product_low >> 19;
        if ((uint32)bucket >= 1024)
            bucket = bucket < 0 ? 0 : 1023;
        ApplyVertexLights(&vertex0->position, (material_flags >> 24) | extra);
        xport_gte_write_data(6, descriptor->payload.words[3]);
        section = scene_stream.active_section;
        end = section->origin[2];
        if (end != 0)
        {
            far_distance = render_view.far_distance;
            product = (sint64)(sint32)far_distance * (sint32)((uint32)bucket << 2);
            distance = (sint32)((uint64)product >> 12);
            start = section->origin[1];
            if ((sint32)start < distance)
            {
                if (distance >= (sint32)end)
                    goto next_polygon;
                sum = (uint32)distance - start;
                coefficient = ((sint32)render_view.depth_scale);
                product = (sint64)coefficient * (sint32)sum;
                fade = (sint32)((uint64)product >> 12) >> 8;
                xport_gte_write_data(8, (uint32)fade);
            }
            else
                xport_gte_write_data(8, 0);
        }
        else
            xport_gte_write_data(8, 0);
        kind = material_flags & 6;
        if (kind == 6 || kind == 2)
        {
            xport_gte_write_data(0, xport_load_le32(&vertex0->normal));
            xport_gte_write_data(1, xport_load_le32((const uint8 *)&vertex0->normal + 4));
            xport_gte_write_data(2, xport_load_le32(&vertex1->normal));
            xport_gte_write_data(3, xport_load_le32((const uint8 *)&vertex1->normal + 4));
            xport_gte_write_data(4, xport_load_le32(&vertex3->normal));
            xport_gte_write_data(5, xport_load_le32((const uint8 *)&vertex3->normal + 4));
            xport_gte_execute(0xF80416);
            tag = kind == 6 ? 0x0C000000 : 0x08000000;
        }
        else
        {
            xport_gte_write_data(0, xport_load_le32(&face->normal.vector));
            xport_gte_write_data(1, xport_load_le32((const uint8 *)&face->normal.vector + 4));
            xport_gte_execute(0xE80413);
            tag = kind == 4 ? 0x09000000 : 0x05000000;
        }
        bucket_offset = render_commands.ordering_table_offset + 4 * (uint32)bucket;
        link = r_u32(bucket_offset);
        packet = render_commands.packet_offset;
        w_u32(packet, (link & 0xFFFFFF) | tag);
        link = r_u32(bucket_offset);
        w_u32(bucket_offset, (link & 0xFF000000) | (packet & 0xFFFFFF));
        if (kind == 6)
        {
            descriptor->WritePolygonUV(render_commands.packet_offset + 12, 3);
            packet = render_commands.packet_offset;
            w_u32(packet + 4, xport_gte_read_data(20));
            w_u32(packet + 16, xport_gte_read_data(21));
            w_u32(packet + 28, xport_gte_read_data(22));
            xport_gte_write_data(0, xport_load_le32(&vertex2->normal));
            xport_gte_write_data(1, xport_load_le32((const uint8 *)&vertex2->normal + 4));
            xport_gte_execute(0xE80413);
            packet = render_commands.packet_offset;
            w_u32(packet + 8, screen0->xy);
            w_u32(packet + 20, screen1->xy);
            w_u32(packet + 32, screen3->xy);
            w_u32(packet + 44, screen2->xy);
            w_u32(packet + 40, xport_gte_read_data(22));
            render_commands.packet_offset = (render_commands.packet_offset + 52);
        }
        else if (kind == 4)
        {
            packet = render_commands.packet_offset;
            descriptor->WritePolygonUV(packet + 12, 2);
            w_u32(packet + 8, screen0->xy);
            w_u32(packet + 16, screen1->xy);
            w_u32(packet + 24, screen3->xy);
            w_u32(packet + 32, screen2->xy);
            w_u32(packet + 4, xport_gte_read_data(22));
            render_commands.packet_offset = (packet + 40);
        }
        else
        {
            packet = render_commands.packet_offset;
            if ((material_flags & 0x80) != 0)
            {
                link = r_u32(packet);
                w_u32(packet + 12, (link & 0xFFFFFF) | tag);
                w_u32(packet, ((packet + 12) & 0xFFFFFF) | 0x02000000);
                w_u32(packet + 4, (material_flags & 0x60) | 0xE1000000);
                w_u32(packet + 8, 0);
                packet += 12;
            }
            if (kind == 2)
            {
                w_u32(packet + 4, xport_gte_read_data(20));
                w_u32(packet + 12, xport_gte_read_data(21));
                w_u32(packet + 20, xport_gte_read_data(22));
                xport_gte_write_data(0, xport_load_le32(&vertex2->normal));
                xport_gte_write_data(1, xport_load_le32((const uint8 *)&vertex2->normal + 4));
                xport_gte_execute(0xE80413);
                w_u32(packet + 8, screen0->xy);
                w_u32(packet + 16, screen1->xy);
                w_u32(packet + 24, screen3->xy);
                w_u32(packet + 32, screen2->xy);
                w_u32(packet + 28, xport_gte_read_data(22));
                render_commands.packet_offset = (packet + 36);
            }
            else
            {
                w_u32(packet + 4, xport_gte_read_data(22));
                w_u32(packet + 8, screen0->xy);
                w_u32(packet + 12, screen1->xy);
                w_u32(packet + 16, screen3->xy);
                w_u32(packet + 20, screen2->xy);
                render_commands.packet_offset = (packet + 24);
            }
        }
    next_polygon:
        ++faces;
        --remaining;
    } while (remaining != 0);
    render_view.material_override = saved_material;
}

void AnimatedObject::DrawGeometry(const MorphSpan *span) const
{
    const SceneSection *section;
    const MeshVertex *vertices, *vertex;
    ProjectedVertex *projected;
    sint32 polygon_count;
    RenderMatrixSlot *matrix;
    MATRIX *current_matrix, *output_matrix;
    const MATRIX *parent_matrix;
    uint32 point;
    uint32 remaining;
    const MeshFace *polygon;
    MeshLighting *lighting;
    uint32 index;
    uint32 value;
    uint32 x;
    uint32 y;
    uint32 z;
    uint32 flags;
    FUNCTION_MARKER(0x80011980, "SLUS_005.78");
    polygon_count = span->polygon_count;
    if (polygon_count == 0)
        return;
    matrix = render_view.matrix_cursor;
    current_matrix = &matrix->matrix;
    {
        RenderMatrixSlot *reserve_slot = matrix;
        uint32 reserve_index;
        for (reserve_index = 0; reserve_index < 3; ++reserve_index)
        {
            if (reserve_slot->next == 0)
            {
                RenderMatrixSlot *next_slot = (RenderMatrixSlot *)malloc(sizeof(RenderMatrixSlot));
                uint32 default_offset, default_bytes;
                if (next_slot == 0)
                {
                    fprintf(stderr, "P2: Cannot allocate native render matrix slot\n");
                    abort();
                }
                memset(next_slot, 0, sizeof(RenderMatrixSlot));
                next_slot->prev = reserve_slot;
                next_slot->depth = reserve_slot->depth + 1;
                default_offset = OriginalDefaults::RenderMatrixCursorOffset - 0x1F800000u + next_slot->depth * 32;
                if (default_offset < sizeof(OriginalDefaults::ScratchpadStartup))
                {
                    default_bytes = sizeof(OriginalDefaults::ScratchpadStartup) - default_offset;
                    if (default_bytes > sizeof(MATRIX))
                        default_bytes = sizeof(MATRIX);
                    CopyBytes((uint8 *)&next_slot->matrix, OriginalDefaults::ScratchpadStartup + default_offset, default_bytes);
                }
                reserve_slot->next = next_slot;
            }
            reserve_slot = reserve_slot->next;
        }
    }
    render_view.matrix_cursor = matrix->next;
    x = xport_load_le32((const uint8 *)&render_view.camera_transform);
    y = xport_load_le32((const uint8 *)&render_view.camera_transform + 4);
    xport_gte_write_control(0, x);
    xport_gte_write_control(1, y);
    x = xport_load_le32((const uint8 *)&render_view.camera_transform + 8);
    y = xport_load_le32((const uint8 *)&render_view.camera_transform + 12);
    z = xport_load_le32((const uint8 *)&render_view.camera_transform + 16);
    xport_gte_write_control(2, x);
    xport_gte_write_control(3, y);
    xport_gte_write_control(4, z);
    output_matrix = &render_view.matrix_cursor->matrix;
    ApplyRotMatrixLV((const VECTOR *)current_matrix->t, (VECTOR *)output_matrix->t);
    matrix = render_view.matrix_cursor;
    current_matrix = &matrix->matrix;
    value = (uint32)render_view.camera_transform.t[0];
    x = (uint32)current_matrix->t[0];
    current_matrix->t[0] = (sint32)(x + value);
    y = (uint32)current_matrix->t[1];
    value = (uint32)render_view.camera_transform.t[1];
    current_matrix->t[1] = (sint32)(y + value);
    z = (uint32)current_matrix->t[2];
    value = (uint32)render_view.camera_transform.t[2];
    current_matrix->t[2] = (sint32)(z + value);
    parent_matrix = (matrix->prev != 0 ? &matrix->prev->matrix : 0);
    MulRotMatrix0(parent_matrix, current_matrix);
    x = xport_load_le32((const uint8 *)current_matrix);
    y = xport_load_le32((const uint8 *)current_matrix + 4);
    xport_gte_write_control(0, x);
    xport_gte_write_control(1, y);
    x = xport_load_le32((const uint8 *)current_matrix + 8);
    y = xport_load_le32((const uint8 *)current_matrix + 12);
    z = xport_load_le32((const uint8 *)current_matrix + 16);
    xport_gte_write_control(2, x);
    xport_gte_write_control(3, y);
    xport_gte_write_control(4, z);
    matrix = render_view.matrix_cursor;
    current_matrix = &matrix->matrix;
    x = (uint32)current_matrix->t[0];
    y = (uint32)current_matrix->t[1];
    z = (uint32)current_matrix->t[2];
    xport_gte_write_control(5, (uint32)((sint32)x >> 5));
    xport_gte_write_control(6, (uint32)((sint32)y >> 5));
    xport_gte_write_control(7, (uint32)((sint32)z >> 5));
    remaining = (uint32)(sint32)span->vertex_count;
    if (remaining > RenderViewState::ProjectionCapacity)
    {
        fprintf(stderr, "P2: Vertex span exceeds native projection capacity\n");
        abort();
    }
    point = (uint32)(sint32)span->first_vertex;
    vertices = (((MeshVertex *)(this->vertices))) + point;
    vertex = vertices;
    projected = render_view.projected_vertices;
    while ((sint32)(remaining - 3) >= 0)
    {
        xport_gte_write_data(0, xport_load_le32(&vertex[0].position));
        xport_gte_write_data(1, xport_load_le32((const uint8 *)&vertex[0].position + 4));
        xport_gte_write_data(2, xport_load_le32(&vertex[1].position));
        xport_gte_write_data(3, xport_load_le32((const uint8 *)&vertex[1].position + 4));
        xport_gte_write_data(4, xport_load_le32(&vertex[2].position));
        xport_gte_write_data(5, xport_load_le32((const uint8 *)&vertex[2].position + 4));
        xport_gte_execute(0x280030);
        projected[0].xy = xport_gte_read_data(12);
        projected[1].xy = xport_gte_read_data(13);
        projected[2].xy = xport_gte_read_data(14);
        projected[0].depth = xport_gte_read_data(17);
        projected[1].depth = xport_gte_read_data(18);
        projected[2].depth = xport_gte_read_data(19);
        vertex += 3;
        remaining -= 3;
        projected += 3;
    }
    while (remaining != 0)
    {
        xport_gte_write_data(0, xport_load_le32(&vertex[0].position));
        xport_gte_write_data(1, xport_load_le32((const uint8 *)&vertex[0].position + 4));
        xport_gte_execute(0x180001);
        projected[0].xy = xport_gte_read_data(14);
        projected[0].depth = xport_gte_read_data(19);
        ++vertex;
        ++projected;
        --remaining;
    }
    point = (uint16)span->first_vertex;
    render_view.first_projected_vertex = (uint16)point;
    flags = game_flow.transition_flags;
    matrix = render_view.matrix_cursor;
    current_matrix = &matrix->matrix;
    for (index = 0; index < 8; ++index)
        render_view.inverse_transform.m[index / 3][index % 3] = current_matrix->m[index % 3][index / 3];
    value = (uint16)current_matrix->m[2][2];
    game_flow.transition_flags = flags & 0xFFFFFFFE;
    render_view.inverse_transform.m[2][2] = (sint16)value;
    x = xport_load_le32((const uint8 *)&render_view.inverse_transform);
    y = xport_load_le32((const uint8 *)&render_view.inverse_transform + 4);
    xport_gte_write_control(0, x);
    xport_gte_write_control(1, y);
    x = xport_load_le32((const uint8 *)&render_view.inverse_transform + 8);
    y = xport_load_le32((const uint8 *)&render_view.inverse_transform + 12);
    z = xport_load_le32((const uint8 *)&render_view.inverse_transform + 16);
    xport_gte_write_control(2, x);
    xport_gte_write_control(3, y);
    xport_gte_write_control(4, z);
    ApplyRotMatrixLV((const VECTOR *)current_matrix->t, (VECTOR *)render_view.inverse_transform.t);
    x = (uint32)render_view.inverse_transform.t[0];
    y = (uint32)render_view.inverse_transform.t[1];
    z = (uint32)render_view.inverse_transform.t[2];
    render_view.inverse_transform.t[0] = (sint32)(0u - x);
    render_view.inverse_transform.t[1] = (sint32)(0u - y);
    render_view.inverse_transform.t[2] = (sint32)(0u - z);
    if (render_view.lighting_enabled)
    {
        section = scene_stream.active_section;
        value = lighting_index;
        point = (uint32)(sint32)span->first_polygon;
        if ((value & 3) != 0 || polygon_count < 0 || point > section->lighting_count || value / 4 > section->lighting_count - point || (uint32)polygon_count > section->lighting_count - point - value / 4 || section->lighting_records == NULL)
        {
            fprintf(stderr, "P2: Animated mesh is outside scene lighting cache\n");
            abort();
        }
        lighting = section->lighting_records + value / 4 + point;
        render_view.lighting_cache = lighting;
        if ((this->flags & 1) != 0)
        {
            point = (uint32)(sint32)span->first_polygon;
            polygon = ((MeshFace *)(polygons)) + point;
            DrawFacesWithFreshLighting(vertices, polygon, polygon_count);
        }
        else
        {
            point = (uint32)(sint32)span->first_polygon;
            polygon = ((MeshFace *)(polygons)) + point;
            DrawFacesWithCachedLighting(vertices, polygon, polygon_count);
        }
    }
    else
    {
        flags = game_flow.transition_flags;
        point = (uint32)(sint32)span->first_polygon;
        polygon = ((MeshFace *)(polygons)) + point;
        if ((flags & 0x20) != 0)
            DrawUniformColorFaces(polygon, polygon_count);
        else
            DrawFacesWithoutLightingCache(vertices, polygon, polygon_count);
    }
    matrix = render_view.matrix_cursor->prev;
    current_matrix = &matrix->matrix;
    render_view.matrix_cursor = matrix;
    x = xport_load_le32((const uint8 *)current_matrix);
    y = xport_load_le32((const uint8 *)current_matrix + 4);
    xport_gte_write_control(0, x);
    xport_gte_write_control(1, y);
    x = xport_load_le32((const uint8 *)current_matrix + 8);
    y = xport_load_le32((const uint8 *)current_matrix + 12);
    z = xport_load_le32((const uint8 *)current_matrix + 16);
    xport_gte_write_control(2, x);
    xport_gte_write_control(3, y);
    xport_gte_write_control(4, z);
}

uint32 DrawSceneGeometry(void)
{
    const SceneSection *section;
    const MeshVertex *vertices, *vertex;
    MeshGeometryNode *mesh, *geometry_view;
    ProjectedVertex *projected;
    FUNCTION_MARKER(0x80011E04, "SLUS_005.78");
    RenderMatrixSlot *matrix;
    SceneNode *node;
    uint32 flags, x, y, z, remaining, first_index, value, result;
    MATRIX *current_matrix, *work_matrix, *camera_matrix, *parent_matrix;
    matrix = render_view.matrix_cursor;
    flags = game_flow.transition_flags;
    {
        RenderMatrixSlot *reserve_slot = matrix;
        uint32 reserve_index;
        for (reserve_index = 0; reserve_index < 3; ++reserve_index)
        {
            if (reserve_slot->next == 0)
            {
                RenderMatrixSlot *next_slot = (RenderMatrixSlot *)malloc(sizeof(RenderMatrixSlot));
                uint32 default_offset, default_bytes;
                if (next_slot == 0)
                {
                    fprintf(stderr, "P2: Cannot allocate native render matrix slot\n");
                    abort();
                }
                memset(next_slot, 0, sizeof(RenderMatrixSlot));
                next_slot->prev = reserve_slot;
                next_slot->depth = reserve_slot->depth + 1;
                default_offset = OriginalDefaults::RenderMatrixCursorOffset - 0x1F800000u + next_slot->depth * 32;
                if (default_offset < sizeof(OriginalDefaults::ScratchpadStartup))
                {
                    default_bytes = sizeof(OriginalDefaults::ScratchpadStartup) - default_offset;
                    if (default_bytes > sizeof(MATRIX))
                        default_bytes = sizeof(MATRIX);
                    CopyBytes((uint8 *)&next_slot->matrix, OriginalDefaults::ScratchpadStartup + default_offset, default_bytes);
                }
                reserve_slot->next = next_slot;
            }
            reserve_slot = reserve_slot->next;
        }
    }
    current_matrix = &matrix->matrix;
    work_matrix = &matrix->next->matrix;
    camera_matrix = &matrix->next->next->matrix;
    parent_matrix = (matrix->prev != 0 ? &matrix->prev->matrix : 0);
    render_view.matrix_cursor = matrix->next;
    game_flow.transition_flags = flags | 0x200;
    x = xport_load_le32((const uint8 *)&render_view.camera_transform);
    y = xport_load_le32((const uint8 *)&render_view.camera_transform + 4);
    xport_gte_write_control(0, x);
    xport_gte_write_control(1, y);
    x = xport_load_le32((const uint8 *)&render_view.camera_transform + 8);
    y = xport_load_le32((const uint8 *)&render_view.camera_transform + 12);
    z = xport_load_le32((const uint8 *)&render_view.camera_transform + 16);
    xport_gte_write_control(2, x);
    xport_gte_write_control(3, y);
    xport_gte_write_control(4, z);
    x = xport_load_le32((const uint8 *)&render_view.camera_transform);
    y = xport_load_le32((const uint8 *)&render_view.camera_transform + 4);
    z = xport_load_le32((const uint8 *)&render_view.camera_transform + 8);
    xport_store_le32((uint8 *)work_matrix, x);
    xport_store_le32((uint8 *)work_matrix + 4, y);
    xport_store_le32((uint8 *)work_matrix + 8, z);
    x = xport_load_le32((const uint8 *)&render_view.camera_transform + 12);
    y = xport_load_le32((const uint8 *)&render_view.camera_transform + 16);
    z = (uint32)render_view.camera_transform.t[0];
    xport_store_le32((uint8 *)work_matrix + 12, x);
    xport_store_le32((uint8 *)work_matrix + 16, y);
    work_matrix->t[0] = (sint32)(z);
    x = (uint32)render_view.camera_transform.t[1];
    y = (uint32)render_view.camera_transform.t[2];
    work_matrix->t[1] = (sint32)(x);
    work_matrix->t[2] = (sint32)(y);
    matrix = render_view.matrix_cursor;
    current_matrix = &matrix->matrix;
    work_matrix = &matrix->next->matrix;
    camera_matrix = &matrix->next->next->matrix;
    parent_matrix = (matrix->prev != 0 ? &matrix->prev->matrix : 0);
    x = camera_motion.position[0];
    y = camera_motion.position[1];
    z = camera_motion.position[2];
    camera_matrix->t[0] = (sint32)(x);
    camera_matrix->t[1] = (sint32)(y);
    camera_matrix->t[2] = (sint32)(z);
    section = scene_stream.active_section;
    node = section->object_heads[2];
    while (node != 0)
    {
        mesh = (MeshGeometryNode *)node;
        flags = mesh->flags;
        value = mesh->node.flags;
        flags |= 0x200;
        mesh->flags = flags;
        if ((value & 1) != 0)
            goto next_node;
        geometry_view = mesh;
        if (mesh->vertex_count <= 0)
        {
            geometry_view = (((MeshGeometryNode *)(mesh->source_mesh)));
            if ((geometry_view->node.flags & 1) != 0)
                goto next_node;
        }
        if (geometry_view == mesh && (flags & 0x2000) != 0)
        {
            mesh->flags = flags & 0xFFFFBFFF;
            goto next_node;
        }
        value = mesh->node.state_flags;
        if ((IsSphereVisible(mesh->position, value << 5) << 24) == 0)
            goto next_node;
        flags = mesh->flags;
        mesh->flags = flags & 0xFFFFFDFF;
        if ((flags & 0x80) != 0)
            goto next_node;
        remaining = (uint32)(sint32)geometry_view->vertex_count;
        if (remaining > RenderViewState::ProjectionCapacity)
        {
            fprintf(stderr, "P2: Mesh exceeds native projection capacity\n");
            abort();
        }
        first_index = (uint16)geometry_view->first_vertex;
        vertices = (((MeshVertex *)(geometry_view->vertices))) + first_index;
        if (geometry_view != mesh && (geometry_view->flags & 0x4000) != 0)
            goto render_node;
        matrix = render_view.matrix_cursor;
        current_matrix = &matrix->matrix;
        work_matrix = &matrix->next->matrix;
        camera_matrix = &matrix->next->next->matrix;
        parent_matrix = (matrix->prev != 0 ? &matrix->prev->matrix : 0);
        x = geometry_view->position[0];
        y = geometry_view->position[1];
        z = geometry_view->position[2];
        work_matrix->t[0] = (sint32)(x);
        work_matrix->t[1] = (sint32)(y);
        work_matrix->t[2] = (sint32)(z);
        UpdateMaterialAnimationList(geometry_view->material_animation_entries);
        matrix = render_view.matrix_cursor;
        current_matrix = &matrix->matrix;
        work_matrix = &matrix->next->matrix;
        camera_matrix = &matrix->next->next->matrix;
        parent_matrix = (matrix->prev != 0 ? &matrix->prev->matrix : 0);
        ApplyRotMatrixLV((const VECTOR *)work_matrix->t, (VECTOR *)current_matrix->t);
        matrix = render_view.matrix_cursor;
        current_matrix = &matrix->matrix;
        work_matrix = &matrix->next->matrix;
        camera_matrix = &matrix->next->next->matrix;
        parent_matrix = (matrix->prev != 0 ? &matrix->prev->matrix : 0);
        x = (uint32)render_view.camera_transform.t[0];
        current_matrix->t[0] = (sint32)((uint32)current_matrix->t[0] + x);
        current_matrix->t[1] = (sint32)((uint32)current_matrix->t[1] + (uint32)render_view.camera_transform.t[1]);
        current_matrix->t[2] = (sint32)((uint32)current_matrix->t[2] + (uint32)render_view.camera_transform.t[2]);
        render_view.inverse_transform.t[0] = (sint32)((uint32)camera_matrix->t[0] - (uint32)work_matrix->t[0]);
        render_view.inverse_transform.t[1] = (sint32)((uint32)camera_matrix->t[1] - (uint32)work_matrix->t[1]);
        render_view.inverse_transform.t[2] = (sint32)((uint32)camera_matrix->t[2] - (uint32)work_matrix->t[2]);
        x = (uint32)current_matrix->t[0];
        y = (uint32)current_matrix->t[1];
        z = (uint32)current_matrix->t[2];
        xport_gte_write_control(5, (uint32)((sint32)x >> 5));
        xport_gte_write_control(6, (uint32)((sint32)y >> 5));
        xport_gte_write_control(7, (uint32)((sint32)z >> 5));
        projected = render_view.projected_vertices;
        remaining -= 3;
        vertex = vertices;
        if ((sint32)remaining >= 0)
        {
            do
            {
                xport_gte_write_data(0, xport_load_le32(&vertex[0].position));
                xport_gte_write_data(1, xport_load_le32((const uint8 *)&vertex[0].position + 4));
                xport_gte_write_data(2, xport_load_le32(&vertex[1].position));
                xport_gte_write_data(3, xport_load_le32((const uint8 *)&vertex[1].position + 4));
                xport_gte_write_data(4, xport_load_le32(&vertex[2].position));
                xport_gte_write_data(5, xport_load_le32((const uint8 *)&vertex[2].position + 4));
                xport_gte_execute(0x280030);
                projected[0].xy = xport_gte_read_data(12);
                projected[1].xy = xport_gte_read_data(13);
                projected[2].xy = xport_gte_read_data(14);
                projected[0].depth = xport_gte_read_data(17);
                projected[1].depth = xport_gte_read_data(18);
                projected[2].depth = xport_gte_read_data(19);
                vertex += 3;
                remaining -= 3;
                projected += 3;
            } while ((sint32)remaining >= 0);
        }
        remaining += 3;
        while (remaining != 0)
        {
            xport_gte_write_data(0, xport_load_le32(&vertex->position));
            xport_gte_write_data(1, xport_load_le32((const uint8 *)&vertex->position + 4));
            xport_gte_execute(0x180001);
            projected[0].xy = xport_gte_read_data(14);
            projected[0].depth = xport_gte_read_data(19);
            ++vertex;
            ++projected;
            --remaining;
        }
        first_index = (uint16)geometry_view->first_vertex;
        render_view.first_projected_vertex = (uint16)first_index;
        if (geometry_view != mesh)
            geometry_view->flags = geometry_view->flags | 0x4000;
    render_node:
        section = scene_stream.active_section;
        value = mesh->lighting_index;
        matrix = render_view.matrix_cursor;
        current_matrix = &matrix->matrix;
        work_matrix = &matrix->next->matrix;
        camera_matrix = &matrix->next->next->matrix;
        parent_matrix = (matrix->prev != 0 ? &matrix->prev->matrix : 0);
        if ((value & 3) != 0 || mesh->polygon_count < 0 || value / 4 > section->lighting_count || (uint32)mesh->polygon_count > section->lighting_count - value / 4 || section->lighting_records == NULL)
        {
            fprintf(stderr, "P2: Scene mesh is outside lighting cache\n");
            abort();
        }
        render_view.lighting_cache = section->lighting_records + value / 4;
        x = mesh->position[0];
        y = mesh->position[1];
        z = mesh->position[2];
        parent_matrix->t[0] = (sint32)(x);
        parent_matrix->t[1] = (sint32)(y);
        parent_matrix->t[2] = (sint32)(z);
        SetLightingBounds(mesh->position, mesh->node.state_flags);
        if ((mesh->flags & 1) == 0)
            DrawFacesWithCachedLighting(vertices, ((MeshFace *)(mesh->polygons)), mesh->polygon_count);
        else
        {
            DrawFacesWithFreshLighting(vertices, ((MeshFace *)(mesh->polygons)), mesh->polygon_count);
            mesh->flags = mesh->flags & 0xFFFFFFFE;
        }
    next_node:
        node = mesh->node.links.registry_next;
    }
    flags = game_flow.transition_flags;
    matrix = render_view.matrix_cursor;
    current_matrix = &matrix->matrix;
    work_matrix = &matrix->next->matrix;
    camera_matrix = &matrix->next->next->matrix;
    parent_matrix = (matrix->prev != 0 ? &matrix->prev->matrix : 0);
    result = flags & 0xFFFFFDFF;
    game_flow.transition_flags = result;
    render_view.matrix_cursor = matrix->prev;
    return result;
}
