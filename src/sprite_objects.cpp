#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "game.h"
#include "original_defaults.h"

union SpriteCorner
{
    SVECTOR vertex;
    ProjectedVertex projected;
};

void InitializeSpritePool(void)
{
    uint32 remaining = SpritePoolState::EntryCount - 1;
    uint32 index;
    SpriteRecord *current, *next;
    FUNCTION_MARKER(0x8002A74C, "SLUS_005.78");
    if (sprite_pool.records == 0)
    {
        sprite_pool.records = (SpriteRecord *)malloc(SpritePoolState::EntryCount * sizeof(SpriteRecord));
        if (sprite_pool.records == 0)
        {
            fprintf(stderr, "P2: Cannot allocate sprite pool\n");
            abort();
        }
    }
    current = sprite_pool.records;
    for (index = 0; index < SpritePoolState::EntryCount; ++index)
    {
        sprite_pool.entries[index].node = (SceneNode *)(current + index);
        current[index].animation_entries = 0;
    }
    sprite_pool.first = (SceneNode *)current;
    sprite_pool.free_head = (SceneNode *)current;
    do
    {
        current = (SpriteRecord *)sprite_pool.free_head;
        --remaining;
        next = current + 1;
        current->node.links.registry_next = &next->node;
        sprite_pool.free_head = (SceneNode *)next;
    } while (remaining != 0);
    (current + 1)->node.links.registry_next = 0;
    sprite_pool.free_head = sprite_pool.first;
}

void SpawnEffectSprite(const uint32 *position, uint32 scale, uint32 effect)
{
    SpriteRecord *sprite;
    SpritePoolEntry *pool_entry = 0;
    uint32 pool_index;
    uint32 *list;
    FUNCTION_MARKER(0x8002A80C, "SLUS_005.78");
    SceneNode *next, *previous;
    uint32 entry, count, bytes, x, y, z;
    if (effect >= 35)
        return;
    entry = effect;
    if (resource_groups[entry].animation_entries == 0)
        return;
    sprite = (SpriteRecord *)sprite_pool.free_head;
    if (sprite == 0)
        return;
    for (pool_index = 0; pool_index < SpritePoolState::EntryCount; ++pool_index)
    {
        if (sprite_pool.entries[pool_index].node == &sprite->node)
        {
            pool_entry = sprite_pool.entries + pool_index;
            break;
        }
    }
    if (pool_entry == 0)
    {
        fprintf(stderr, "P2: Sprite free list refers outside its pool\n");
        abort();
    }
    next = sprite->node.links.registry_next;
    previous = sprite_pool.active_head;
    sprite_pool.active_head = (SceneNode *)sprite;
    sprite_pool.free_head = next;
    sprite->node.links.registry_next = previous;
    sprite->node.links.sibling_next = 0;
    sprite->node.links.first_child = 0;
    sprite->node.kind = 7;
    sprite->node.flags = 16;
    sprite->node.state_flags = 1;
    sprite->animation_binding.tracks[3] = -1;
    sprite->animation_binding.tracks[2] = -1;
    sprite->animation_binding.tracks[1] = -1;
    sprite->animation_binding.tracks[0] = -1;
    sprite->extent_track = 0;
    sprite->texture = 0;
    sprite->animation_entries = 0;
    sprite->position[2] = 0;
    sprite->position[1] = 0;
    sprite->position[0] = 0;
    sprite->extent[1] = 128;
    sprite->extent[0] = 128;
    sprite->metadata_halfwords[1] = 0;
    sprite->metadata_halfwords[0] = 0;
    list = resource_groups[entry].animation_entries;
    sprite->texture = resource_groups[entry].texture;
    if (list != 0)
    {
        count = (uint8)list[-1];
        if (count == 0 || count > SpritePoolState::AnimationCapacity)
        {
            fprintf(stderr, "P2: Invalid pooled material animation count\n");
            abort();
        }
        bytes = (count + 1) * 4;
        FillBytes((uint8 *)pool_entry->animation_words, 0xFF, bytes);
        pool_entry->animation_words[0] = count;
        sprite->animation_entries = pool_entry->animation_words + 1;
        list = resource_groups[entry].animation_entries;
        CopyBytes((uint8 *)sprite->animation_entries, (const uint8 *)list, count * 4);
        SetMaterialAnimationFlags(sprite->animation_entries + count - 1, 8);
        ResetMaterialAnimationList(sprite->animation_entries);
    }
    sprite->node.state_flags = sprite->node.state_flags | 2;
    x = position[0];
    y = position[1];
    z = position[2];
    sprite->position[0] = x;
    sprite->position[1] = y;
    sprite->position[2] = z;
    sprite->extent[1] = scale >> 5;
    sprite->extent[0] = scale >> 5;
    SceneSection *section = scene_stream.active_section;
    sprite->node.links.sibling_next = section->transient_links.first_child;
    section->transient_links.first_child = &sprite->node;
}

void ReleaseSprite(SpriteRecord *object)
{
    FUNCTION_MARKER(0x8002A2F4, "SLUS_005.78");
    SpriteRecord *sprite = (SpriteRecord *)(object);
    uint32 *list;
    list = sprite->animation_entries;
    if (list != 0)
    {
        sprite->animation_entries = 0;
    }
    ((SceneNode *)object)->MoveBetweenLists(&sprite_pool.active_head, &sprite_pool.free_head);
}

uint32 SceneNode::DrawBillboard() const
{
    const SceneSection *section;
    FUNCTION_MARKER(0x80064358, "SLUS_005.78");
    const SpriteRecord *sprite = 0;
    const BillboardNode *billboard;
    const TextureDescriptor *descriptor;
    SpriteCorner corner[3];
    SVECTOR normal;
    uint32 packet, limit, result, x, y, z, scale, depth, value, end, start, bucket_offset, link;
    uint32 orientation, texture, uv0, uv1, uv2, uv3, flags;
    const MATRIX *current_matrix;
    MATRIX *work_matrix;
    sint32 width, height, bucket, distance, fade, radius;
    sint64 product;
    packet = render_commands.packet_offset;
    limit = render_commands.packet_limit;
    if (packet >= limit)
        return 0;
    if (kind == 8)
    {
        billboard = (const BillboardNode *)this;
        width = billboard->extent >> 5;
        height = width;
    }
    else
    {
        sprite = (const SpriteRecord *)this;
        width = sprite->extent[0];
        height = sprite->extent[1];
        scale = render_view.object_scale;
        if (scale != 0)
        {
            width = (sint32)((uint32)width * scale) >> 12;
            height = (sint32)((uint32)height * scale) >> 12;
        }
    }
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
    current_matrix = &render_view.matrix_cursor->matrix;
    x = (uint32)current_matrix->t[0];
    y = (uint32)current_matrix->t[1];
    z = (uint32)current_matrix->t[2];
    xport_gte_write_control(5, (uint32)((sint32)x >> 5));
    xport_gte_write_control(6, (uint32)((sint32)y >> 5));
    xport_gte_write_control(7, (uint32)((sint32)z >> 5));
    corner[0].vertex.vx = 0 - (uint32)(uint16)width;
    corner[0].vertex.vy = height;
    corner[0].vertex.vz = 0;
    corner[1].vertex.vx = width;
    corner[1].vertex.vy = 0 - (uint32)height;
    corner[1].vertex.vz = 0;
    xport_gte_write_data(0, xport_load_le32(&corner[0].vertex));
    xport_gte_write_data(1, (uint16)corner[0].vertex.vz);
    xport_gte_write_data(2, xport_load_le32(&corner[1].vertex));
    xport_gte_write_data(3, (uint16)corner[1].vertex.vz);
    xport_gte_execute(0x280030);
    corner[0].projected.xy = xport_gte_read_data(12);
    corner[1].projected.xy = xport_gte_read_data(13);
    corner[0].projected.depth = xport_gte_read_data(17);
    corner[1].projected.depth = xport_gte_read_data(18);
    corner[2].projected.depth = xport_gte_read_data(19);
    depth = corner[0].projected.depth + corner[1].projected.depth;
    limit = render_view.far_distance;
    result = (sint32)(depth << 2) < (sint32)limit;
    if (result == 0)
        goto done;
    value = (uint32)(sint32)(sint16)render_view.object_depth_bias;
    scale = render_view.fog_scale;
    depth <<= 1;
    value += (uint32)((sint32)depth >> 2);
    bucket = (sint32)(value * scale) >> 19;
    if ((uint32)bucket >= 1024)
        bucket = bucket < 0 ? 0 : 1023;
    xport_gte_write_data(8, 0);
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
                goto done;
            product = (sint64)((sint32)render_view.depth_scale) * (sint32)result;
            fade = (sint32)((uint64)product >> 12) >> 8;
            xport_gte_write_data(8, (uint32)fade);
        }
        else
            xport_gte_write_data(8, 0);
    }
    else
        xport_gte_write_data(8, 0);
    bucket_offset = render_commands.ordering_table_offset + (uint32)bucket * 4;
    link = r_u32(bucket_offset);
    packet = render_commands.packet_offset;
    w_u32(packet, (link & 0xFFFFFF) | 0x09000000);
    link = r_u32(bucket_offset);
    w_u32(bucket_offset, (link & 0xFF000000) | (packet & 0xFFFFFF));
    orientation = 0;
    if (kind == 8)
        descriptor = (((const BillboardNode *)this)->texture != 0 ? ((const BillboardNode *)this)->texture : (const TextureDescriptor *)0);
    else
    {
        descriptor = render_view.material_override;
        if (descriptor == NULL)
        {
            value = sprite->node.state_flags;
            descriptor = ((const TextureDescriptor *)(sprite->texture));
            orientation = (value & 0x180) >> 7;
        }
    }
    packet = render_commands.packet_offset;
    texture = descriptor->payload.words[0];
    uv0 = texture & 0xFFFF;
    uv3 = texture >> 16;
    texture = descriptor->payload.words[1];
    flags = descriptor->flags;
    uv0 |= texture & 0xFFFF0000;
    uv1 = (uv0 & 0xFF00) | (uv3 & 0xFF) | (texture << 16);
    if ((flags & 0x80) != 0)
        uv1 |= (flags & 0x60) << 16;
    uv2 = (uv3 & 0xFF00) | (uv0 & 0xFF);
    if (kind == 8 || (sprite->node.state_flags & 1) != 0)
    {
        xport_gte_write_data(6, descriptor->payload.words[3]);
        xport_gte_execute(0x780010);
        w_u32(packet + 4, xport_gte_read_data(22));
    }
    else
    {
        work_matrix = &render_view.matrix_cursor->next->matrix;
        x = sprite->position[0];
        y = sprite->position[1];
        z = sprite->position[2];
        work_matrix->t[0] = (sint32)(x);
        work_matrix->t[1] = (sint32)(y);
        work_matrix->t[2] = (sint32)(z);
        if ((game_flow.transition_flags & 0x201) != 1)
        {
            width = sprite->extent[0];
            radius = sprite->extent[1];
            if (radius < width)
                radius = width;
            const MATRIX *parent_matrix = (render_view.matrix_cursor->prev != 0 ? &render_view.matrix_cursor->prev->matrix : 0);
            SetLightingBounds((const uint32 *)parent_matrix->t, radius);
            TransformObjectLights(0x80000000);
            value = game_flow.transition_flags;
            work_matrix = &render_view.matrix_cursor->next->matrix;
            game_flow.transition_flags = value | 1;
            work_matrix->t[2] = (sint32)(0);
            work_matrix->t[1] = (sint32)(0);
            work_matrix->t[0] = (sint32)(0);
        }
        work_matrix = &render_view.matrix_cursor->next->matrix;
        normal.vx = (uint32)(work_matrix->t[0] >> 5);
        normal.vy = (uint32)(work_matrix->t[1] >> 5);
        normal.vz = (uint32)(work_matrix->t[2] >> 5);
        xport_gte_write_data(6, descriptor->payload.words[3]);
        ApplyVertexLights(&normal, (descriptor->flags >> 24) | 0x80000000);
        PushMatrix();
        x = xport_load_le32((const uint8 *)&scene_lighting.transforms.fields.inverse_rotation + 0);
        y = xport_load_le32((const uint8 *)&scene_lighting.transforms.fields.inverse_rotation + 4);
        xport_gte_write_control(0, x);
        xport_gte_write_control(1, y);
        x = xport_load_le32((const uint8 *)&scene_lighting.transforms.fields.inverse_rotation + 8);
        y = xport_load_le32((const uint8 *)&scene_lighting.transforms.fields.inverse_rotation + 12);
        z = xport_load_le32((const uint8 *)&scene_lighting.transforms.fields.inverse_rotation + 16);
        xport_gte_write_control(2, x);
        xport_gte_write_control(3, y);
        xport_gte_write_control(4, z);
        xport_gte_write_data(0, ((uint32)(uint16)render_view.planes[0].normal.vx | ((uint32)(uint16)render_view.planes[0].normal.vy << 16)));
        xport_gte_write_data(1, ((uint32)(uint16)render_view.planes[0].normal.vz | ((uint32)(uint16)render_view.planes[0].normal.pad << 16)));
        xport_gte_execute(0x486012);
        render_view.transform_result.vx = xport_gte_read_data(25);
        render_view.transform_result.vy = xport_gte_read_data(26);
        render_view.transform_result.vz = xport_gte_read_data(27);
        x = render_view.transform_result.vx;
        y = render_view.transform_result.vy;
        z = render_view.transform_result.vz;
        normal.vx = x;
        normal.vy = y;
        normal.vz = z;
        PopMatrix();
        xport_gte_write_data(0, xport_load_le32(&normal));
        xport_gte_write_data(1, (uint16)normal.vz);
        xport_gte_execute(0xE80413);
        w_u32(packet + 4, xport_gte_read_data(22));
    }
    value = descriptor->payload.bytes[15];
    w_u32(packet + 12, uv0);
    w_u32(packet + 20, uv1);
    w_u32(packet + 28, uv2);
    w_u32(packet + 36, uv3);
    w_u8(packet + 7, (value & 3) | 0x2C);
    switch (orientation)
    {
        case 0:
            w_u32(packet + 8, corner[0].projected.xy);
            w_u32(packet + 16, (corner[0].projected.xy & 0xFFFF0000) | corner[1].projected.coordinates.x);
            w_u32(packet + 24, (corner[1].projected.xy & 0xFFFF0000) | corner[0].projected.coordinates.x);
            w_u32(packet + 32, corner[1].projected.xy);
            break;
        case 1:
            w_u32(packet + 8, (corner[1].projected.xy & 0xFFFF0000) | corner[0].projected.coordinates.x);
            w_u32(packet + 16, corner[0].projected.xy);
            w_u32(packet + 24, corner[1].projected.xy);
            w_u32(packet + 32, (corner[0].projected.xy & 0xFFFF0000) | corner[1].projected.coordinates.x);
            break;
        case 2:
            w_u32(packet + 8, corner[1].projected.xy);
            w_u32(packet + 16, (corner[1].projected.xy & 0xFFFF0000) | corner[0].projected.coordinates.x);
            w_u32(packet + 24, (corner[0].projected.xy & 0xFFFF0000) | corner[1].projected.coordinates.x);
            w_u32(packet + 32, corner[0].projected.xy);
            break;
        case 3:
            w_u32(packet + 8, (corner[0].projected.xy & 0xFFFF0000) | corner[1].projected.coordinates.x);
            w_u32(packet + 16, corner[1].projected.xy);
            w_u32(packet + 24, corner[0].projected.xy);
            w_u32(packet + 32, (corner[1].projected.xy & 0xFFFF0000) | corner[0].projected.coordinates.x);
            break;
    }
    result = packet + 40;
    render_commands.packet_offset = (result);
done:
    return result;
}

void SpriteRecord::Draw() const
{
    FUNCTION_MARKER(0x8001156C, "SLUS_005.78");
    MATRIX temporary = {0};
    RenderMatrixSlot *matrix;
    uint32 x, y, z, sum;
    MATRIX *current_matrix;
    const MATRIX *parent_matrix;
    MATRIX *rotation;
    sint32 cosine, sine, angle;
    matrix = render_view.matrix_cursor;
    current_matrix = &matrix->matrix;
    xport_store_le16((uint8 *)&temporary + 18, 0);
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
    parent_matrix = &matrix->matrix;
    current_matrix = &render_view.matrix_cursor->matrix;
    ApplyRotMatrixLV((const VECTOR *)parent_matrix->t, (VECTOR *)current_matrix->t);
    matrix = render_view.matrix_cursor;
    current_matrix = &matrix->matrix;
    x = (uint32)render_view.camera_transform.t[0];
    current_matrix->t[0] = (sint32)((uint32)current_matrix->t[0] + x);
    current_matrix->t[1] = (sint32)((uint32)current_matrix->t[1] + (uint32)render_view.camera_transform.t[1]);
    current_matrix->t[2] = (sint32)((uint32)current_matrix->t[2] + (uint32)render_view.camera_transform.t[2]);
    if ((node.state_flags & 0x200) != 0)
    {
        cosine = SinQ14((sint16)(1024 - metadata_halfwords[0])) >> 2;
        sine = SinQ14((sint16)metadata_halfwords[0]) >> 2;
        rotation_x.m[1][1] = (sint16)(cosine);
        rotation_x.m[2][1] = (sint16)(sine);
        rotation_x.m[1][2] = (sint16)(0 - (uint32)sine);
        rotation_x.m[2][2] = (sint16)(cosine);
        cosine = SinQ14((sint16)(1024 - metadata_halfwords[1])) >> 2;
        sine = SinQ14((sint16)metadata_halfwords[1]) >> 2;
        rotation_y.m[0][0] = (sint16)(cosine);
        rotation_y.m[2][0] = (sint16)(0 - (uint32)sine);
        rotation_y.m[0][2] = (sint16)(sine);
        rotation_y.m[2][2] = (sint16)(cosine);
        MulMatrix0(&rotation_y, &rotation_x, &temporary);
        rotation = &render_view.matrix_cursor->matrix;
        MulMatrix0(&render_view.camera_transform, &temporary, rotation);
        x = xport_load_le32((const uint8 *)rotation);
        y = xport_load_le32((const uint8 *)rotation + 4);
        xport_gte_write_control(0, x);
        xport_gte_write_control(1, y);
        x = xport_load_le32((const uint8 *)rotation + 8);
        y = xport_load_le32((const uint8 *)rotation + 12);
        z = xport_load_le32((const uint8 *)rotation + 16);
        xport_gte_write_control(2, x);
        xport_gte_write_control(3, y);
        xport_gte_write_control(4, z);
        matrix = render_view.matrix_cursor;
        current_matrix = &matrix->matrix;
        angle = render_view.billboard_transform.m[1][0];
        sum = (uint32)(((sint64)current_matrix->t[0] * angle) >> 12);
        x = (uint32)current_matrix->t[1];
        angle = render_view.billboard_transform.m[1][1];
        sum += (uint32)(((sint64)(sint32)x * angle) >> 12);
        y = (uint32)current_matrix->t[2];
        angle = render_view.billboard_transform.m[1][2];
        sum += (uint32)(((sint64)(sint32)y * angle) >> 12);
        if ((sint32)sum < 0)
            DrawGroundQuad();
    }
    else
    {
        angle = (sint16)metadata_halfwords[0];
        sine = extent[0];
        current_matrix->t[0] = (sint32)((uint32)current_matrix->t[0] + (uint32)((angle * sine) >> 2));
        angle = (sint16)metadata_halfwords[1];
        sine = extent[1];
        current_matrix->t[1] = (sint32)((uint32)current_matrix->t[1] + (uint32)((angle * sine) >> 2));
        if ((node.state_flags & 4) != 0)
        {
            PushMatrix();
            x = xport_load_le32((const uint8 *)&render_view.billboard_transform);
            y = xport_load_le32((const uint8 *)&render_view.billboard_transform + 4);
            xport_gte_write_control(0, x);
            xport_gte_write_control(1, y);
            x = xport_load_le32((const uint8 *)&render_view.billboard_transform + 8);
            y = xport_load_le32((const uint8 *)&render_view.billboard_transform + 12);
            z = xport_load_le32((const uint8 *)&render_view.billboard_transform + 16);
            xport_gte_write_control(2, x);
            xport_gte_write_control(3, y);
            xport_gte_write_control(4, z);
            ApplyRotMatrixLV((const VECTOR *)current_matrix->t, (VECTOR *)render_view.inverse_transform.t);
            PopMatrix();
            x = (uint32)render_view.inverse_transform.t[2];
            y = (uint32)render_view.inverse_transform.t[0];
            angle = (sint32)Atan2Angle((sint32)x, (sint32)y);
            cosine = SinQ14((sint16)(1024 - (uint32)angle)) >> 2;
            x = (uint32)render_view.inverse_transform.t[2];
            y = (uint32)render_view.inverse_transform.t[0];
            angle = (sint32)Atan2Angle((sint32)x, (sint32)y);
            sine = SinQ14((sint16)angle) >> 2;
            rotation_y.m[0][0] = (sint16)(cosine);
            rotation_y.m[2][0] = (sint16)(0 - (uint32)sine);
            rotation_y.m[0][2] = (sint16)(sine);
            rotation_y.m[2][2] = (sint16)(cosine);
            MulRotMatrix0(&rotation_y, current_matrix);
            rotation = &render_view.matrix_cursor->matrix;
            x = xport_load_le32((const uint8 *)rotation);
            y = xport_load_le32((const uint8 *)rotation + 4);
            xport_gte_write_control(0, x);
            xport_gte_write_control(1, y);
            x = xport_load_le32((const uint8 *)rotation + 8);
            y = xport_load_le32((const uint8 *)rotation + 12);
            z = xport_load_le32((const uint8 *)rotation + 16);
            xport_gte_write_control(2, x);
            xport_gte_write_control(3, y);
            xport_gte_write_control(4, z);
            DrawFacingQuad();
        }
        else
            node.DrawBillboard();
    }
    matrix = render_view.matrix_cursor;
    current_matrix = &matrix->matrix;
    render_view.matrix_cursor = matrix->prev;
    rotation = (matrix->prev != 0 && matrix->prev->prev != 0 ? &matrix->prev->prev->matrix : 0);
    x = xport_load_le32((const uint8 *)rotation);
    y = xport_load_le32((const uint8 *)rotation + 4);
    xport_gte_write_control(0, x);
    xport_gte_write_control(1, y);
    x = xport_load_le32((const uint8 *)rotation + 8);
    y = xport_load_le32((const uint8 *)rotation + 12);
    z = xport_load_le32((const uint8 *)rotation + 16);
    xport_gte_write_control(2, x);
    xport_gte_write_control(3, y);
    xport_gte_write_control(4, z);
}

uint32 SpriteRecord::DrawGroundQuad() const
{
    const SceneSection *section;
    FUNCTION_MARKER(0x80063A80, "SLUS_005.78");
    SpriteCorner corner[4];
    const TextureDescriptor *descriptor;
    uint32 packet, x, y, z, value, sum, limit, result, end, start, link, bucket_offset;
    uint32 texture, flags, uv0, uv1, uv2, uv3;
    const MATRIX *matrix;
    sint32 height, bucket, distance, fade;
    sint64 product;
    packet = render_commands.packet_offset;
    limit = render_commands.packet_limit;
    if (packet >= limit)
        return 0;
    matrix = &render_view.matrix_cursor->matrix;
    x = (uint32)matrix->t[0];
    y = (uint32)matrix->t[1];
    z = (uint32)matrix->t[2];
    xport_gte_write_control(5, (uint32)((sint32)x >> 5));
    xport_gte_write_control(6, (uint32)((sint32)y >> 5));
    xport_gte_write_control(7, (uint32)((sint32)z >> 5));
    value = (uint32)(sint32)extent[0];
    height = extent[1];
    corner[0].vertex.vy = 0;
    corner[1].vertex.vy = 0;
    corner[2].vertex.vy = 0;
    corner[3].vertex.vy = 0;
    x = 0 - (value & 0xFFFF);
    z = 0 - (uint32)height;
    corner[0].vertex.vx = x;
    corner[0].vertex.vz = z;
    corner[1].vertex.vx = x;
    corner[1].vertex.vz = height;
    corner[2].vertex.vx = value;
    corner[2].vertex.vz = height;
    corner[3].vertex.vx = value;
    corner[3].vertex.vz = z;
    xport_gte_write_data(0, xport_load_le32(&corner[0].vertex));
    xport_gte_write_data(1, (uint16)corner[0].vertex.vz);
    xport_gte_write_data(2, xport_load_le32(&corner[1].vertex));
    xport_gte_write_data(3, (uint16)corner[1].vertex.vz);
    xport_gte_write_data(4, xport_load_le32(&corner[2].vertex));
    xport_gte_write_data(5, (uint16)corner[2].vertex.vz);
    xport_gte_execute(0x280030);
    corner[0].projected.xy = xport_gte_read_data(12);
    corner[1].projected.xy = xport_gte_read_data(13);
    corner[2].projected.xy = xport_gte_read_data(14);
    corner[0].projected.depth = xport_gte_read_data(17);
    corner[1].projected.depth = xport_gte_read_data(18);
    corner[2].projected.depth = xport_gte_read_data(19);
    xport_gte_write_data(0, xport_load_le32(&corner[3].vertex));
    xport_gte_write_data(1, (uint16)corner[3].vertex.vz);
    xport_gte_execute(0x180001);
    corner[3].projected.xy = xport_gte_read_data(14);
    corner[3].projected.depth = xport_gte_read_data(19);
    if (corner[0].projected.coordinates.x >= 320 && corner[1].projected.coordinates.x >= 320 && corner[2].projected.coordinates.x >= 320 && corner[3].projected.coordinates.x >= 320)
    {
        result = 0;
        goto done;
    }
    if (corner[0].projected.coordinates.y >= 240 && corner[1].projected.coordinates.y >= 240 && corner[2].projected.coordinates.y >= 240 && corner[3].projected.coordinates.y >= 240)
    {
        result = 0;
        goto done;
    }
    x = corner[0].projected.depth;
    y = corner[1].projected.depth;
    z = corner[2].projected.depth;
    value = corner[3].projected.depth;
    limit = render_view.far_distance;
    sum = x + y + z + value;
    result = (sint32)(sum << 1) < (sint32)limit;
    if (result == 0)
        goto done;
    value = (uint32)(sint32)(sint16)render_view.object_depth_bias;
    x = render_view.fog_scale;
    value += (uint32)((sint32)sum >> 2) + 128;
    bucket = (sint32)(value * x) >> 19;
    if ((uint32)bucket >= 1024)
        bucket = bucket < 0 ? 0 : 1023;
    xport_gte_write_data(8, 0);
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
                goto done;
            product = (sint64)((sint32)render_view.depth_scale) * (sint32)result;
            fade = (sint32)((uint64)product >> 12) >> 8;
            xport_gte_write_data(8, (uint32)fade);
        }
        else
            xport_gte_write_data(8, 0);
    }
    else
        xport_gte_write_data(8, 0);
    bucket_offset = render_commands.ordering_table_offset + (uint32)bucket * 4;
    link = r_u32(bucket_offset);
    packet = render_commands.packet_offset;
    w_u32(packet, (link & 0xFFFFFF) | 0x09000000);
    link = r_u32(bucket_offset);
    w_u32(bucket_offset, (link & 0xFF000000) | (packet & 0xFFFFFF));
    descriptor = ((const TextureDescriptor *)(this->texture));
    packet = render_commands.packet_offset;
    texture = descriptor->payload.words[0];
    uv0 = texture & 0xFFFF;
    uv3 = texture >> 16;
    texture = descriptor->payload.words[1];
    flags = descriptor->flags;
    uv0 |= texture & 0xFFFF0000;
    uv1 = (uv0 & 0xFF00) | (uv3 & 0xFF) | (texture << 16);
    if ((flags & 0x80) != 0)
        uv1 |= (flags & 0x60) << 16;
    uv2 = (uv3 & 0xFF00) | (uv0 & 0xFF);
    xport_gte_write_data(6, descriptor->payload.words[3]);
    xport_gte_execute(0x780010);
    w_u32(packet + 4, xport_gte_read_data(22));
    w_u32(packet + 12, uv0);
    w_u32(packet + 20, uv1);
    w_u32(packet + 28, uv2);
    w_u32(packet + 36, uv3);
    w_u32(packet + 8, corner[0].projected.xy);
    w_u32(packet + 16, corner[1].projected.xy);
    w_u32(packet + 24, corner[3].projected.xy);
    w_u32(packet + 32, corner[2].projected.xy);
    result = packet + 40;
    render_commands.packet_offset = result;
done:
    return result;
}

uint32 SpriteRecord::DrawFacingQuad() const
{
    const SceneSection *section;
    FUNCTION_MARKER(0x80063E78, "SLUS_005.78");
    SpriteCorner corner[4];
    const TextureDescriptor *descriptor;
    uint32 packet, x, y, z, value, sum, limit, result, end, start, link, bucket_offset;
    uint32 texture, flags, uv0, uv1, uv2, uv3, orientation;
    const MATRIX *matrix;
    sint32 height, bucket, distance, fade;
    sint64 product;
    packet = render_commands.packet_offset;
    limit = render_commands.packet_limit;
    if (packet >= limit)
        return 0;
    matrix = &render_view.matrix_cursor->matrix;
    x = (uint32)matrix->t[0];
    y = (uint32)matrix->t[1];
    z = (uint32)matrix->t[2];
    xport_gte_write_control(5, (uint32)((sint32)x >> 5));
    xport_gte_write_control(6, (uint32)((sint32)y >> 5));
    xport_gte_write_control(7, (uint32)((sint32)z >> 5));
    value = (uint32)(sint32)extent[0];
    height = extent[1];
    corner[0].vertex.vz = 0;
    corner[1].vertex.vz = 0;
    corner[2].vertex.vz = 0;
    corner[3].vertex.vz = 0;
    x = 0 - (value & 0xFFFF);
    height += 10;
    y = 0 - (uint32)height;
    corner[0].vertex.vx = x;
    corner[0].vertex.vy = y;
    corner[1].vertex.vx = value;
    corner[1].vertex.vy = y;
    corner[2].vertex.vx = value;
    corner[2].vertex.vy = height;
    corner[3].vertex.vx = x;
    corner[3].vertex.vy = height;
    xport_gte_write_data(0, xport_load_le32(&corner[0].vertex));
    xport_gte_write_data(1, (uint16)corner[0].vertex.vz);
    xport_gte_write_data(2, xport_load_le32(&corner[1].vertex));
    xport_gte_write_data(3, (uint16)corner[1].vertex.vz);
    xport_gte_write_data(4, xport_load_le32(&corner[2].vertex));
    xport_gte_write_data(5, (uint16)corner[2].vertex.vz);
    xport_gte_execute(0x280030);
    corner[0].projected.xy = xport_gte_read_data(12);
    corner[1].projected.xy = xport_gte_read_data(13);
    corner[2].projected.xy = xport_gte_read_data(14);
    corner[0].projected.depth = xport_gte_read_data(17);
    corner[1].projected.depth = xport_gte_read_data(18);
    corner[2].projected.depth = xport_gte_read_data(19);
    xport_gte_write_data(0, xport_load_le32(&corner[3].vertex));
    xport_gte_write_data(1, (uint16)corner[3].vertex.vz);
    xport_gte_execute(0x180001);
    corner[3].projected.xy = xport_gte_read_data(14);
    corner[3].projected.depth = xport_gte_read_data(19);
    if (corner[0].projected.coordinates.x >= 320 && corner[1].projected.coordinates.x >= 320 && corner[2].projected.coordinates.x >= 320 && corner[3].projected.coordinates.x >= 320)
    {
        result = 0;
        goto done;
    }
    if (corner[0].projected.coordinates.y >= 240 && corner[1].projected.coordinates.y >= 240 && corner[2].projected.coordinates.y >= 240 && corner[3].projected.coordinates.y >= 240)
    {
        result = 0;
        goto done;
    }
    x = corner[0].projected.depth;
    y = corner[1].projected.depth;
    z = corner[2].projected.depth;
    value = corner[3].projected.depth;
    limit = render_view.far_distance;
    sum = x + y + z + value;
    result = (sint32)(sum << 1) < (sint32)limit;
    if (result == 0)
        goto done;
    x = render_view.fog_scale;
    value = (uint32)((sint32)sum >> 2);
    bucket = (sint32)(value * x) >> 19;
    if ((uint32)bucket >= 1024)
        bucket = bucket < 0 ? 0 : 1023;
    xport_gte_write_data(8, 0);
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
                goto done;
            product = (sint64)((sint32)render_view.depth_scale) * (sint32)result;
            fade = (sint32)((uint64)product >> 12) >> 8;
            xport_gte_write_data(8, (uint32)fade);
        }
        else
            xport_gte_write_data(8, 0);
    }
    else
        xport_gte_write_data(8, 0);
    bucket_offset = render_commands.ordering_table_offset + (uint32)bucket * 4;
    link = r_u32(bucket_offset);
    packet = render_commands.packet_offset;
    w_u32(packet, (link & 0xFFFFFF) | 0x09000000);
    link = r_u32(bucket_offset);
    w_u32(bucket_offset, (link & 0xFF000000) | (packet & 0xFFFFFF));
    descriptor = ((const TextureDescriptor *)(this->texture));
    packet = render_commands.packet_offset;
    texture = descriptor->payload.words[0];
    uv0 = texture & 0xFFFF;
    uv3 = texture >> 16;
    texture = descriptor->payload.words[1];
    flags = descriptor->flags;
    uv0 |= texture & 0xFFFF0000;
    uv1 = (uv0 & 0xFF00) | (uv3 & 0xFF) | (texture << 16);
    if ((flags & 0x80) != 0)
        uv1 |= (flags & 0x60) << 16;
    uv2 = (uv3 & 0xFF00) | (uv0 & 0xFF);
    xport_gte_write_data(6, descriptor->payload.words[3]);
    xport_gte_execute(0x780010);
    w_u32(packet + 4, xport_gte_read_data(22));
    w_u8(packet + 7, (descriptor->payload.bytes[15] & 3) | 0x2C);
    w_u32(packet + 12, uv0);
    w_u32(packet + 20, uv1);
    w_u32(packet + 28, uv2);
    w_u32(packet + 36, uv3);
    orientation = (node.state_flags & 0x180) >> 7;
    switch (orientation)
    {
        case 0:
            w_u32(packet + 8, corner[0].projected.xy);
            w_u32(packet + 16, corner[1].projected.xy);
            w_u32(packet + 24, corner[3].projected.xy);
            w_u32(packet + 32, corner[2].projected.xy);
            break;
        case 1:
            w_u32(packet + 8, corner[3].projected.xy);
            w_u32(packet + 16, corner[0].projected.xy);
            w_u32(packet + 24, corner[2].projected.xy);
            w_u32(packet + 32, corner[1].projected.xy);
            break;
        case 2:
            w_u32(packet + 8, corner[2].projected.xy);
            w_u32(packet + 16, corner[3].projected.xy);
            w_u32(packet + 24, corner[1].projected.xy);
            w_u32(packet + 32, corner[0].projected.xy);
            break;
        case 3:
            w_u32(packet + 8, corner[1].projected.xy);
            w_u32(packet + 16, corner[2].projected.xy);
            w_u32(packet + 24, corner[0].projected.xy);
            w_u32(packet + 32, corner[3].projected.xy);
            break;
    }
    result = packet + 40;
    render_commands.packet_offset = result;
done:
    return result;
}

uint32 UpdateSpriteExtent(SpriteRecord *sprite)
{
    FUNCTION_MARKER(0x8001BAA8, "SLUS_005.78");
    uint32 value, result;
    uint32 coordinates[3];
    sint32 limit;
    AnimationTrack *track = sprite->extent_track;
    if (track == 0)
        return 0xFFFFFFFF;
    value = (uint16)track->frame;
    limit = track->frame_limit;
    ++value;
    track->frame = value;
    if (limit < (sint16)value)
    {
        result = track->flags & 2;
        if (result == 0)
            return result;
        track->frame = 0;
    }
    SampleVectorTrack(track->frame, track, coordinates);
    if ((sint32)coordinates[1] < 0)
        coordinates[1] = 0;
    sprite->extent[0] = (sint32)coordinates[0] >> 5;
    result = (uint32)((sint32)coordinates[1] >> 5);
    sprite->extent[1] = result;
    return result;
}
