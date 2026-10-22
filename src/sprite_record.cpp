#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sprite_record.h"
#include "game.h"
#include "psx_gte.h"

void SpriteRecord::Draw() const
{
    FUNCTION_MARKER(0x8001156C, "SLUS_005.78");
    MATRIX tmp = {0};
    RenderMatrixSlot *matrix;
    uint32 x, y, sum;
    MATRIX *cur_matrix;
    const MATRIX *parent_matrix;
    MATRIX *rotation;
    sint32 c, s, angle;
    matrix = render_view.matrix_cursor;
    cur_matrix = &matrix->matrix;
    memset((uint8 *)&tmp + sizeof(tmp.m), 0, offsetof(MATRIX, t) - sizeof(tmp.m));
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
    render_view.matrix_cursor = matrix->next;
    SetRotMatrix(&render_view.camera_transform);
    parent_matrix = &matrix->matrix;
    cur_matrix = &render_view.matrix_cursor->matrix;
    {
        VECTOR rotation_input = {parent_matrix->t[0], parent_matrix->t[1], parent_matrix->t[2], 0};
        VECTOR rotation_output;
        ApplyRotMatrixLV(&rotation_input, &rotation_output);
        cur_matrix->t[0] = rotation_output.vx;
        cur_matrix->t[1] = rotation_output.vy;
        cur_matrix->t[2] = rotation_output.vz;
    }
    matrix = render_view.matrix_cursor;
    cur_matrix = &matrix->matrix;
    x = render_view.camera_transform.t[0];
    cur_matrix->t[0] = cur_matrix->t[0] + x;
    cur_matrix->t[1] = cur_matrix->t[1] + (uint32)render_view.camera_transform.t[1];
    cur_matrix->t[2] = cur_matrix->t[2] + (uint32)render_view.camera_transform.t[2];
    if ((node.state_flags & 0x200) != 0)
    {
        c = SinQ14((1024 - angles[0])) >> 2;
        s = SinQ14(angles[0]) >> 2;
        rotation_x.m[1][1] = c;
        rotation_x.m[2][1] = s;
        rotation_x.m[1][2] = -(uint32)s;
        rotation_x.m[2][2] = c;
        c = SinQ14((1024 - angles[1])) >> 2;
        s = SinQ14(angles[1]) >> 2;
        rotation_y.m[0][0] = c;
        rotation_y.m[2][0] = -(uint32)s;
        rotation_y.m[0][2] = s;
        rotation_y.m[2][2] = c;
        MulMatrix0(&rotation_y, &rotation_x, &tmp);
        rotation = &render_view.matrix_cursor->matrix;
        MulMatrix0(&render_view.camera_transform, &tmp, rotation);
        SetRotMatrix(rotation);
        matrix = render_view.matrix_cursor;
        cur_matrix = &matrix->matrix;
        angle = render_view.billboard_transform.m[1][0];
        sum = mul12(cur_matrix->t[0], angle);
        x = cur_matrix->t[1];
        angle = render_view.billboard_transform.m[1][1];
        sum += (uint32)mul12((sint32)x, angle);
        y = cur_matrix->t[2];
        angle = render_view.billboard_transform.m[1][2];
        sum += (uint32)mul12((sint32)y, angle);
        if ((sint32)sum < 0)
            DrawGroundQuad();
    }
    else
    {
        angle = (sint16)angles[0];
        s = extent[0];
        cur_matrix->t[0] = (uint32)cur_matrix->t[0] + ((angle * s) >> 2);
        angle = (sint16)angles[1];
        s = extent[1];
        cur_matrix->t[1] = (uint32)cur_matrix->t[1] + ((angle * s) >> 2);
        if ((node.state_flags & 4) != 0)
        {
            PushMatrix();
            SetRotMatrix(&render_view.billboard_transform);
            {
                VECTOR rotation_input = {cur_matrix->t[0], cur_matrix->t[1], cur_matrix->t[2], 0};
                VECTOR rotation_output;
                ApplyRotMatrixLV(&rotation_input, &rotation_output);
                render_view.inverse_transform.t[0] = rotation_output.vx;
                render_view.inverse_transform.t[1] = rotation_output.vy;
                render_view.inverse_transform.t[2] = rotation_output.vz;
            }
            PopMatrix();
            x = render_view.inverse_transform.t[2];
            y = render_view.inverse_transform.t[0];
            angle = Atan2Angle(x, y);
            c = SinQ14((1024 - (uint32)angle)) >> 2;
            x = render_view.inverse_transform.t[2];
            y = render_view.inverse_transform.t[0];
            angle = Atan2Angle(x, y);
            s = SinQ14(angle) >> 2;
            rotation_y.m[0][0] = c;
            rotation_y.m[2][0] = -(uint32)s;
            rotation_y.m[0][2] = s;
            rotation_y.m[2][2] = c;
            MulRotMatrix0(&rotation_y, cur_matrix);
            rotation = &render_view.matrix_cursor->matrix;
            SetRotMatrix(rotation);
            DrawFacingQuad();
        }
        else
            node.DrawBillboard();
    }
    matrix = render_view.matrix_cursor;
    cur_matrix = &matrix->matrix;
    render_view.matrix_cursor = matrix->prev;
    rotation = matrix->prev != 0 && matrix->prev->prev != 0 ? &matrix->prev->prev->matrix : 0;
    SetRotMatrix(rotation);
}

void SpriteRecord::UpdateSprExtent(SpriteRecord *spr)
{
    FUNCTION_MARKER(0x8001BAA8, "SLUS_005.78");
    uint32 val, result;
    Vec3 coords;
    sint32 limit;
    AnimationTrack *track = spr->extent_track;
    if (track == 0)
        return;
    val = (uint16)track->frame;
    limit = track->frame_limit;
    ++val;
    track->frame = val;
    if (limit < (sint16)val)
    {
        result = track->flags & 2;
        if (result == 0)
            return;
        track->frame = 0;
    }
    AnimationTrack::SampleVectorTrack(track->frame, track, coords);
    if (coords.y < 0)
        coords.y = 0;
    spr->extent[0] = coords.x >> 5;
    spr->extent[1] = coords.y >> 5;
}

void SpriteRecord::ReleaseSpr(SpriteRecord *object)
{
    FUNCTION_MARKER(0x8002A2F4, "SLUS_005.78");
    SpriteRecord *spr = object;
    uint32 *list;
    list = spr->animation_entries;
    if (list != 0)
    {
        spr->animation_entries = 0;
    }
    ((SceneNode *)object)->MoveBetweenLists(&spr_pool.active_head, &spr_pool.free_head);
}

void SpriteRecord::InitializeSprPool(void)
{
    uint32 remaining = SpritePoolState::EntryCount - 1;
    uint32 idx;
    SpriteRecord *cur, *next;
    FUNCTION_MARKER(0x8002A74C, "SLUS_005.78");
    if (spr_pool.recs == 0)
    {
        spr_pool.recs = (SpriteRecord *)malloc(SpritePoolState::EntryCount * sizeof(SpriteRecord));
    }
    cur = spr_pool.recs;
    for (idx = 0; idx < SpritePoolState::EntryCount; ++idx)
    {
        spr_pool.entries[idx].node = (SceneNode *)(cur + idx);
        cur[idx].animation_entries = 0;
    }
    spr_pool.first = (SceneNode *)cur;
    spr_pool.free_head = (SceneNode *)cur;
    do
    {
        cur = (SpriteRecord *)spr_pool.free_head;
        --remaining;
        next = cur + 1;
        cur->node.links.registry_next = &next->node;
        spr_pool.free_head = (SceneNode *)next;
    } while (remaining != 0);
    (cur + 1)->node.links.registry_next = 0;
    spr_pool.free_head = spr_pool.first;
}

void SpriteRecord::SpawnEffectSpr(const sint32 *pos, uint32 scale, uint32 effect)
{
    SpriteRecord *spr;
    SpritePoolEntry *pool_entry = 0;
    uint32 pool_idx;
    uint32 *list;
    FUNCTION_MARKER(0x8002A80C, "SLUS_005.78");
    SceneNode *next, *prev;
    uint32 entry, count, bytes, x, y, z;
    if (effect >= 35)
        return;
    entry = effect;
    if (resource_grps[entry].animation_entries == 0)
        return;
    spr = (SpriteRecord *)spr_pool.free_head;
    if (spr == 0)
        return;
    for (pool_idx = 0; pool_idx < SpritePoolState::EntryCount; ++pool_idx)
    {
        if (spr_pool.entries[pool_idx].node == &spr->node)
        {
            pool_entry = spr_pool.entries + pool_idx;
            break;
        }
    }
    next = spr->node.links.registry_next;
    prev = spr_pool.active_head;
    spr_pool.active_head = (SceneNode *)spr;
    spr_pool.free_head = next;
    spr->node.links.registry_next = prev;
    spr->node.links.sibling_next = 0;
    spr->node.links.first_child = 0;
    spr->node.kind = 7;
    spr->node.flags = 16;
    spr->node.state_flags = 1;
    spr->animation_binding.tracks[3] = -1;
    spr->animation_binding.tracks[2] = -1;
    spr->animation_binding.tracks[1] = -1;
    spr->animation_binding.tracks[0] = -1;
    spr->extent_track = 0;
    spr->tex = 0;
    spr->animation_entries = 0;
    spr->pos.z = 0;
    spr->pos.y = 0;
    spr->pos.x = 0;
    spr->extent[1] = 128;
    spr->extent[0] = 128;
    spr->angles[1] = 0;
    spr->angles[0] = 0;
    list = resource_grps[entry].animation_entries;
    spr->tex = resource_grps[entry].tex;
    if (list != 0)
    {
        count = ((const MaterialAnimationListHeader *)list - 1)->count;
        bytes = (count + 1) * 4;
        FillBytes((uint8 *)&pool_entry->animation, 0xFF, bytes);
        pool_entry->animation.header.count = count;
        pool_entry->animation.header.start_tick = 0;
        spr->animation_entries = pool_entry->animation.entries;
        list = resource_grps[entry].animation_entries;
        CopyBytes((uint8 *)spr->animation_entries, (const uint8 *)list, count * 4);
        MaterialRecord::SetMaterialAnimationFlags(spr->animation_entries + count - 1, 8);
        MaterialAnimationList::ResetMaterialAnimationList(spr->animation_entries);
    }
    spr->node.state_flags = spr->node.state_flags | 2;
    x = pos[0];
    y = pos[1];
    z = pos[2];
    spr->pos.x = x;
    spr->pos.y = y;
    spr->pos.z = z;
    spr->extent[1] = scale >> 5;
    spr->extent[0] = scale >> 5;
    SceneSection *section = scene_stream.active_section;
    spr->node.links.sibling_next = section->transient_links.first_child;
    section->transient_links.first_child = &spr->node;
}

void SpriteRecord::DrawGroundQuad() const
{
    uint32 *bucket_entry;

    POLY_FT4 *poly;
    const SceneSection *section;
    FUNCTION_MARKER(0x80063A80, "SLUS_005.78");
    SpriteCorner corner[4];
    const TextureDescriptor *desc;
    uint32 packet;
    uint32 x;
    sint32 y;
    uint32 z;
    uint32 val;
    uint32 sum;
    uint32 limit;
    uint32 result;
    uint32 end;
    uint32 start;
    uint32 link;
    uint32 bucket_offset;
    uint32 tex_val, flags, uv0, uv1, uv2, uv3;
    const MATRIX *matrix;
    sint32 height, bucket, distance, fade;
    packet = render_commands.packet_offset;
    limit = render_commands.packet_limit;
    if (packet >= limit)
        return;
    matrix = &render_view.matrix_cursor->matrix;
    x = matrix->t[0];
    y = matrix->t[1];
    z = matrix->t[2];
    xport_gte_write_control(5, ((sint32)x >> 5));
    xport_gte_write_control(6, (y >> 5));
    xport_gte_write_control(7, ((sint32)z >> 5));
    val = extent[0];
    height = extent[1];
    corner[0].vertex.vy = 0;
    corner[1].vertex.vy = 0;
    corner[2].vertex.vy = 0;
    corner[3].vertex.vy = 0;
    x = -(val & 0xFFFF);
    z = -(uint32)height;
    corner[0].vertex.vx = x;
    corner[0].vertex.vz = z;
    corner[1].vertex.vx = x;
    corner[1].vertex.vz = height;
    corner[2].vertex.vx = val;
    corner[2].vertex.vz = height;
    corner[3].vertex.vx = val;
    corner[3].vertex.vz = z;
    gte_ldv0(&corner[0].vertex);
    gte_ldv1(&corner[1].vertex);
    gte_ldv2(&corner[2].vertex);
    xport_gte_execute(0x280030);
    corner[0].proj.xy = xport_gte_read_data(12);
    corner[1].proj.xy = xport_gte_read_data(13);
    corner[2].proj.xy = xport_gte_read_data(14);
    corner[0].proj.depth = xport_gte_read_data(17);
    corner[1].proj.depth = xport_gte_read_data(18);
    corner[2].proj.depth = xport_gte_read_data(19);
    gte_ldv0(&corner[3].vertex);
    xport_gte_execute(0x180001);
    corner[3].proj.xy = xport_gte_read_data(14);
    corner[3].proj.depth = xport_gte_read_data(19);
    if (corner[0].proj.coords.x >= 320 && corner[1].proj.coords.x >= 320 && corner[2].proj.coords.x >= 320 && corner[3].proj.coords.x >= 320)
    {
        goto done;
    }
    if (corner[0].proj.coords.y >= 240 && corner[1].proj.coords.y >= 240 && corner[2].proj.coords.y >= 240 && corner[3].proj.coords.y >= 240)
    {
        goto done;
    }
    x = corner[0].proj.depth;
    y = corner[1].proj.depth;
    z = corner[2].proj.depth;
    val = corner[3].proj.depth;
    limit = render_view.far_distance;
    sum = x + y + z + val;
    result = (sint32)(sum << 1) < (sint32)limit;
    if (result == 0)
        goto done;
    val = render_view.object_depth_bias;
    x = render_view.fog_scale;
    val += (uint32)((sint32)sum >> 2) + 128;
    bucket = (sint32)(val * x) >> 19;
    if ((uint32)bucket >= 1024)
        bucket = bucket < 0 ? 0 : 1023;
    xport_gte_write_data(8, 0);
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
                goto done;
            fade = (sint32)mul12(((sint32)render_view.depth_scale), (sint32)result) >> 8;
            xport_gte_write_data(8, fade);
        }
        else
            xport_gte_write_data(8, 0);
    }
    else
        xport_gte_write_data(8, 0);
    bucket_offset = render_commands.ordering_table_offset + (uint32)bucket * 4;
    bucket_entry = (uint32 *)psx_addr(bucket_offset, sizeof(uint32));
    link = *bucket_entry;
    packet = render_commands.packet_offset;
    poly = (POLY_FT4 *)psx_addr(packet, sizeof(*poly));
    poly->tag = (link & 0xFFFFFF) | 0x09000000;
    link = *bucket_entry;
    *bucket_entry = (link & 0xFF000000) | (packet & 0xFFFFFF);
    desc = tex;
    packet = render_commands.packet_offset;
    tex_val = desc->mapping.uv_rect;
    uv0 = tex_val & 0xFFFF;
    uv3 = tex_val >> 16;
    tex_val = desc->mapping.page_clut;
    flags = desc->flags;
    uv0 |= tex_val & 0xFFFF0000;
    uv1 = (uv0 & 0xFF00) | (uv3 & 0xFF) | (tex_val << 16);
    if ((flags & 0x80) != 0)
        uv1 |= (flags & 0x60) << 16;
    uv2 = (uv3 & 0xFF00) | (uv0 & 0xFF);
    xport_gte_write_data(6, desc->mapping.color.rgbc);
    xport_gte_execute(0x780010);
    poly->rgbc.rgbc = xport_gte_read_data(22);
    poly->p[0].uv_attr = uv0;
    poly->p[1].uv_attr = uv1;
    poly->p[2].uv_attr = uv2;
    poly->p[3].uv_attr = uv3;
    poly->p[0].xy.xy = corner[0].proj.xy;
    poly->p[1].xy.xy = corner[1].proj.xy;
    poly->p[2].xy.xy = corner[3].proj.xy;
    poly->p[3].xy.xy = corner[2].proj.xy;
    result = packet + sizeof(*poly);
    render_commands.packet_offset = result;
done:
    return;
}

void SpriteRecord::DrawFacingQuad() const
{
    uint32 *bucket_entry;

    POLY_FT4 *poly;
    const SceneSection *section;
    FUNCTION_MARKER(0x80063E78, "SLUS_005.78");
    SpriteCorner corner[4];
    const TextureDescriptor *desc;
    uint32 packet;
    uint32 x;
    uint32 y;
    sint32 z;
    uint32 val;
    uint32 sum;
    uint32 limit;
    uint32 result;
    uint32 end;
    uint32 start;
    uint32 link;
    uint32 bucket_offset;
    uint32 tex_val, flags, uv0, uv1, uv2, uv3, orientation;
    const MATRIX *matrix;
    sint32 height, bucket, distance, fade;
    packet = render_commands.packet_offset;
    limit = render_commands.packet_limit;
    if (packet >= limit)
        return;
    matrix = &render_view.matrix_cursor->matrix;
    x = matrix->t[0];
    y = matrix->t[1];
    z = matrix->t[2];
    xport_gte_write_control(5, ((sint32)x >> 5));
    xport_gte_write_control(6, ((sint32)y >> 5));
    xport_gte_write_control(7, (z >> 5));
    val = extent[0];
    height = extent[1];
    corner[0].vertex.vz = 0;
    corner[1].vertex.vz = 0;
    corner[2].vertex.vz = 0;
    corner[3].vertex.vz = 0;
    x = -(val & 0xFFFF);
    height += 10;
    y = -(uint32)height;
    corner[0].vertex.vx = x;
    corner[0].vertex.vy = y;
    corner[1].vertex.vx = val;
    corner[1].vertex.vy = y;
    corner[2].vertex.vx = val;
    corner[2].vertex.vy = height;
    corner[3].vertex.vx = x;
    corner[3].vertex.vy = height;
    gte_ldv0(&corner[0].vertex);
    gte_ldv1(&corner[1].vertex);
    gte_ldv2(&corner[2].vertex);
    xport_gte_execute(0x280030);
    corner[0].proj.xy = xport_gte_read_data(12);
    corner[1].proj.xy = xport_gte_read_data(13);
    corner[2].proj.xy = xport_gte_read_data(14);
    corner[0].proj.depth = xport_gte_read_data(17);
    corner[1].proj.depth = xport_gte_read_data(18);
    corner[2].proj.depth = xport_gte_read_data(19);
    gte_ldv0(&corner[3].vertex);
    xport_gte_execute(0x180001);
    corner[3].proj.xy = xport_gte_read_data(14);
    corner[3].proj.depth = xport_gte_read_data(19);
    if (corner[0].proj.coords.x >= 320 && corner[1].proj.coords.x >= 320 && corner[2].proj.coords.x >= 320 && corner[3].proj.coords.x >= 320)
    {
        goto done;
    }
    if (corner[0].proj.coords.y >= 240 && corner[1].proj.coords.y >= 240 && corner[2].proj.coords.y >= 240 && corner[3].proj.coords.y >= 240)
    {
        goto done;
    }
    x = corner[0].proj.depth;
    y = corner[1].proj.depth;
    z = corner[2].proj.depth;
    val = corner[3].proj.depth;
    limit = render_view.far_distance;
    sum = x + y + z + val;
    result = (sint32)(sum << 1) < (sint32)limit;
    if (result == 0)
        goto done;
    x = render_view.fog_scale;
    val = (sint32)sum >> 2;
    bucket = (sint32)(val * x) >> 19;
    if ((uint32)bucket >= 1024)
        bucket = bucket < 0 ? 0 : 1023;
    xport_gte_write_data(8, 0);
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
                goto done;
            fade = (sint32)mul12(((sint32)render_view.depth_scale), (sint32)result) >> 8;
            xport_gte_write_data(8, fade);
        }
        else
            xport_gte_write_data(8, 0);
    }
    else
        xport_gte_write_data(8, 0);
    bucket_offset = render_commands.ordering_table_offset + (uint32)bucket * 4;
    bucket_entry = (uint32 *)psx_addr(bucket_offset, sizeof(uint32));
    link = *bucket_entry;
    packet = render_commands.packet_offset;
    poly = (POLY_FT4 *)psx_addr(packet, sizeof(*poly));
    poly->tag = (link & 0xFFFFFF) | 0x09000000;
    link = *bucket_entry;
    *bucket_entry = (link & 0xFF000000) | (packet & 0xFFFFFF);
    desc = tex;
    packet = render_commands.packet_offset;
    tex_val = desc->mapping.uv_rect;
    uv0 = tex_val & 0xFFFF;
    uv3 = tex_val >> 16;
    tex_val = desc->mapping.page_clut;
    flags = desc->flags;
    uv0 |= tex_val & 0xFFFF0000;
    uv1 = (uv0 & 0xFF00) | (uv3 & 0xFF) | (tex_val << 16);
    if ((flags & 0x80) != 0)
        uv1 |= (flags & 0x60) << 16;
    uv2 = (uv3 & 0xFF00) | (uv0 & 0xFF);
    xport_gte_write_data(6, desc->mapping.color.rgbc);
    xport_gte_execute(0x780010);
    poly->rgbc.rgbc = xport_gte_read_data(22);
    poly->rgbc.code = (desc->mapping.color.code & 3) | 0x2C;
    poly->p[0].uv_attr = uv0;
    poly->p[1].uv_attr = uv1;
    poly->p[2].uv_attr = uv2;
    poly->p[3].uv_attr = uv3;
    orientation = (node.state_flags & 0x180) >> 7;
    switch (orientation)
    {
        case 0:
            poly->p[0].xy.xy = corner[0].proj.xy;
            poly->p[1].xy.xy = corner[1].proj.xy;
            poly->p[2].xy.xy = corner[3].proj.xy;
            poly->p[3].xy.xy = corner[2].proj.xy;
            break;
        case 1:
            poly->p[0].xy.xy = corner[3].proj.xy;
            poly->p[1].xy.xy = corner[0].proj.xy;
            poly->p[2].xy.xy = corner[2].proj.xy;
            poly->p[3].xy.xy = corner[1].proj.xy;
            break;
        case 2:
            poly->p[0].xy.xy = corner[2].proj.xy;
            poly->p[1].xy.xy = corner[3].proj.xy;
            poly->p[2].xy.xy = corner[1].proj.xy;
            poly->p[3].xy.xy = corner[0].proj.xy;
            break;
        case 3:
            poly->p[0].xy.xy = corner[1].proj.xy;
            poly->p[1].xy.xy = corner[2].proj.xy;
            poly->p[2].xy.xy = corner[0].proj.xy;
            poly->p[3].xy.xy = corner[3].proj.xy;
            break;
    }
    result = packet + sizeof(*poly);
    render_commands.packet_offset = result;
done:
    return;
}
