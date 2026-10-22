#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "object_links.h"
#include "game.h"
#include "psx_gte.h"

void ObjectLinks::UpdateGraph(AnimatedObject *parent, uint32 mode)
{
    // Q27 unity becomes Q12 after the coefficient shift
    FUNCTION_MARKER(0x800140F8, "SLUS_005.78");
    AnimatedObject *parent_view = parent;
    const uint32 transpose_offsets[9] = {0, 6, 12, 2, 8, 14, 4, 10, 16};

    Vec3 delta, input, before, relative;

    MATRIX tmp_matrix = {0};
    Vec3 tmp;

    MATRIX transposed;
    SceneNode *node, *next_node;
    uint32 base_mode, work, category, old_active, flags;
    MATRIX *root_matrix = &render_view.matrix_root.matrix;
    MATRIX *stack_src;
    MATRIX *cur_matrix;
    MATRIX *parent_matrix;
    RenderMatrixSlot *matrix;
    CollisionTransform *col_cache;
    const PathDefinition *attachment;
    uint32 follow;
    AnimatedObject *player, *actor, *linked_child;
    SceneNode *child_reference;
    MeshPartNode *part;
    SpriteRecord *spr;
    SphereNode *sphere_node;
    ScenePath *path;
    MATRIX *path_matrix;
    PathDefinition *definition;
    const PathDefinition *player_definition, *trigger_definition;
    const ScenePath *trigger_path;
    PathTransform *path_transform;
    MATRIX *definition_matrix;
    SceneLight *light;
    PositionedBillboardNode *billboard;
    uint32 x, y, z, w, i;
    sint32 idx, angle;
    sint16 c, s;
    matrix = render_view.matrix_cursor;
    cur_matrix = &matrix->matrix;
    parent_matrix = matrix->prev != 0 ? &matrix->prev->matrix : 0;
    node = first_child;
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
    base_mode = mode & 0xFFFFFFCD;
    while (node != 0)
    {
        flags = node->flags;
        work = base_mode;
        next_node = node->links.sibling_next;
        old_active = flags & 1;
        if ((work & 1) != 0)
            node->flags = flags | 1;
        else if ((work & 8) != 0)
            node->flags = flags & 0xFE;
        category = node->kind;
        if (category == 0 && (work & 0x85) == 0)
        {
            actor = (AnimatedObject *)node;
            {
                const ObjectCallback operation = object_operations.behaviors[actor->behavior];
                if (operation.member_handler)
                    (actor->*operation.member_handler)();
                else if (operation.void_member_handler)
                    (actor->*operation.void_member_handler)();
                else if (operation.factory_handler)
                    operation.factory_handler(actor);
                else
                    operation.plain_handler();
            }
            flags = actor->flags;
            if ((flags & 0x8C000) != 0)
            {
                work &= 0xFFFFFFF7;
                if ((flags & 0x8000) != 0)
                    work |= 0x80;
                else
                {
                    if ((node->flags & 1) != old_active)
                        work |= old_active != 0 ? 0x88 : 1;
                    else if ((flags & 0x80002) == 0x80002 && old_active == 0)
                        work |= 8;
                    else if (actor->behavior == 22)
                        old_active = 0;
                    actor->flags = actor->flags & 0xFFF7BFFF;
                }
            }
            if (old_active != 0 && work == 0)
                goto next_object;
        }
        if ((work & 0x84) != 0 && (node->ResetPlayback((const SceneNode *)parent) & 0xFF) != 0)
        {
            if ((work & 8) != 0 && (node->flags & 1) != 0 && (base_mode & 8) == 0)
                node->flags = node->flags & 0xFE;
            flags = node->flags;
            work &= 0xFFFFFFF6;
            work |= (flags & 1) != 0 ? 0xC1 : 0xC8;
        }
        if ((node->flags & 0x91) == 0x10 && (parent == 0 || category == 0 || (parent_view->flags & 0x200) == 0))
            work |= 0x10;
        flags = node->flags & 0xE2;
        if (flags >= 0x80)
            goto next_object;
        work |= flags;
        if (work == 0)
            goto next_object;
        if ((work & 0x62) != 0)
            AnimatedObject::ApplyNodeAnimation(parent, node);
        matrix = render_view.matrix_cursor;
        cur_matrix = &matrix->matrix;
        parent_matrix = matrix->prev != 0 ? &matrix->prev->matrix : 0;
        stack_src = parent_matrix;
        {
            SetRotMatrix(stack_src);
        }

        if (category == 5)
        {
            part = (MeshPartNode *)node;
            {
                VECTOR rotation_input = {part->pos.x, part->pos.y, part->pos.z, 0};
                VECTOR rotation_output;
                ApplyRotMatrixLV(&rotation_input, &rotation_output);
                cur_matrix->t[0] = rotation_output.vx;
                cur_matrix->t[1] = rotation_output.vy;
                cur_matrix->t[2] = rotation_output.vz;
            }
            matrix = render_view.matrix_cursor;
            cur_matrix = &matrix->matrix;
            parent_matrix = matrix->prev != 0 ? &matrix->prev->matrix : 0;
            x = cur_matrix->t[0] + (uint32)parent_matrix->t[0];
            y = cur_matrix->t[1] + (uint32)parent_matrix->t[1];
            z = cur_matrix->t[2] + (uint32)parent_matrix->t[2];
            cur_matrix->t[0] = x;
            cur_matrix->t[1] = y;
            cur_matrix->t[2] = z;

            if (part->rotation[0] == 0x4000)
            {
                stack_src = parent_matrix;
                memcpy(cur_matrix->m, stack_src->m, sizeof(cur_matrix->m));
            }
            else
            {
                {
                    uint32 qw = part->rotation[0], qx = part->rotation[1], qy = part->rotation[2], qz = part->rotation[3];
                    uint32 xx = qx * qx, yy = qy * qy, zz = qz * qz, xy = qx * qy, xz = qx * qz, yz = qy * qz, wz = qw * qz, wy = qw * qy, wx = qw * qx;
                    cur_matrix->m[0][0] = (sint32)((1u << 27) - (yy + zz)) >> 15;
                    cur_matrix->m[1][1] = (sint32)((1u << 27) - (xx + zz)) >> 15;
                    cur_matrix->m[2][2] = (sint32)((1u << 27) - (xx + yy)) >> 15;
                    cur_matrix->m[1][0] = (sint32)(xy - wz) >> 15;
                    cur_matrix->m[0][1] = (sint32)(xy + wz) >> 15;
                    cur_matrix->m[2][0] = (sint32)(xz + wy) >> 15;
                    cur_matrix->m[0][2] = (sint32)(xz - wy) >> 15;
                    cur_matrix->m[2][1] = (sint32)(yz - wx) >> 15;
                    cur_matrix->m[1][2] = (sint32)(yz + wx) >> 15;
                }
                MulRotMatrix(cur_matrix);
                stack_src = cur_matrix;
                {
                    SetRotMatrix(stack_src);
                }
            }
            idx = part->animation_binding.tracks[2];
            if (idx != -1)
            {
                if (idx < 0)
                {
                    idx = -(idx + 2);
                    work &= 0xFFFFFFEF;
                }
                if ((work & 0xCC) != 0)
                {
                    col_cache = parent_view->col_transforms;
                    col_cache = col_cache + (uint32)idx;

                    if ((work & 0x8C) != 0)
                    {
                        col_cache->motion.z = 0;
                        col_cache->motion.y = 0;
                        col_cache->motion.x = 0;
                    }
                    else
                    {
                        matrix = render_view.matrix_cursor;
                        cur_matrix = &matrix->matrix;
                        parent_matrix = matrix->prev != 0 ? &matrix->prev->matrix : 0;
                        col_cache->motion.x = cur_matrix->t[0] - (uint32)col_cache->pos.x;
                        col_cache->motion.y = cur_matrix->t[1] - (uint32)col_cache->pos.y;
                        col_cache->motion.z = cur_matrix->t[2] - (uint32)col_cache->pos.z;
                    }
                    matrix = render_view.matrix_cursor;
                    cur_matrix = &matrix->matrix;
                    parent_matrix = matrix->prev != 0 ? &matrix->prev->matrix : 0;
                    {
                        uint32 c0 = cur_matrix->t[0], c1 = cur_matrix->t[1], c2 = cur_matrix->t[2];
                        col_cache->pos.x = c0;
                        col_cache->pos.y = c1;
                        col_cache->pos.z = c2;
                    }
                    matrix = render_view.matrix_cursor;
                    cur_matrix = &matrix->matrix;
                    parent_matrix = matrix->prev != 0 ? &matrix->prev->matrix : 0;
                    for (i = 0; i < 9; ++i)
                        col_cache->rotation[i] = cur_matrix->m[i % 3][i / 3];

                    if (parent != 0)
                    {
                        if ((parent_view->flags & 0x200) != 0)
                            node->flags = node->flags | 4;
                        else if ((node->flags & 4) != 0)
                        {
                            if ((work & 2) == 0)
                            {
                                col_cache->motion.z = 0;
                                col_cache->motion.y = 0;
                                col_cache->motion.x = 0;
                            }
                            node->flags = node->flags & 0xFB;
                        }
                    }
                }
            }
            if ((work & 0x14) == 0x10)
                parent_view->DrawGeometry(&part->span);
            goto visit_children;
        }
        if (category == 0)
        {
            actor = (AnimatedObject *)node;
            if (actor->behavior == 22)
            {
                if ((actor->flags & 0x2000) != 0)
                {
                    {
                        uint32 c0 = (uint32)actor->pos.x, c1 = (uint32)actor->pos.y, c2 = actor->pos.z;
                        cur_matrix->t[0] = c0;
                        cur_matrix->t[1] = c1;
                        cur_matrix->t[2] = c2;
                    }
                }
                else
                {
                    attachment = actor->path_definition;
                    if (attachment != 0 && attachment->transform != 0)
                    {
                        {
                            uint32 c0 = (uint32)actor->pos.x, c1 = (uint32)actor->pos.y, c2 = actor->pos.z;
                            input.x = c0;
                            input.y = c1;
                            input.z = c2;
                        }
                        PushMatrix();
                        definition_matrix = actor->path_definition->transform != 0 ? &actor->path_definition->transform->matrix : NULL;
                        SetRotMatrix(definition_matrix);

                        {
                            VECTOR rotation_output;
                            {
                                VECTOR sdk_input = {input.x, input.y, input.z, 0};
                                ApplyRotMatrixLV(&sdk_input, &rotation_output);
                            }
                            (&render_view.matrix_cursor->matrix)->t[0] = rotation_output.vx;
                            (&render_view.matrix_cursor->matrix)->t[1] = rotation_output.vy;
                            (&render_view.matrix_cursor->matrix)->t[2] = rotation_output.vz;
                        }
                        PopMatrix();
                        matrix = render_view.matrix_cursor;
                        cur_matrix = &matrix->matrix;
                        parent_matrix = matrix->prev != 0 ? &matrix->prev->matrix : 0;
                        for (i = 0; i < 3; ++i)
                        {
                            definition_matrix = actor->path_definition->transform != 0 ? &actor->path_definition->transform->matrix : NULL;
                            cur_matrix->t[i] = cur_matrix->t[i] + (uint32)definition_matrix->t[i];
                        }
                    }
                    else
                    {
                        matrix = render_view.matrix_cursor;
                        cur_matrix = &matrix->matrix;
                        parent_matrix = matrix->prev != 0 ? &matrix->prev->matrix : 0;
                        {
                            uint32 c0 = (uint32)actor->pos.x, c1 = (uint32)actor->pos.y, c2 = actor->pos.z;
                            cur_matrix->t[0] = c0;
                            cur_matrix->t[1] = c1;
                            cur_matrix->t[2] = c2;
                        }
                    }
                }
                matrix = render_view.matrix_cursor;
                cur_matrix = &matrix->matrix;
                parent_matrix = matrix->prev != 0 ? &matrix->prev->matrix : 0;
                {
                    uint32 qw = actor->orientation[0], qx = actor->orientation[1], qy = actor->orientation[2], qz = actor->orientation[3];
                    uint32 xx = qx * qx, yy = qy * qy, zz = qz * qz, xy = qx * qy, xz = qx * qz, yz = qy * qz, wz = qw * qz, wy = qw * qy, wx = qw * qx;
                    cur_matrix->m[0][0] = (sint32)((1u << 27) - (yy + zz)) >> 15;
                    cur_matrix->m[1][1] = (sint32)((1u << 27) - (xx + zz)) >> 15;
                    cur_matrix->m[2][2] = (sint32)((1u << 27) - (xx + yy)) >> 15;
                    cur_matrix->m[1][0] = (sint32)(xy - wz) >> 15;
                    cur_matrix->m[0][1] = (sint32)(xy + wz) >> 15;
                    cur_matrix->m[2][0] = (sint32)(xz + wy) >> 15;
                    cur_matrix->m[0][2] = (sint32)(xz - wy) >> 15;
                    cur_matrix->m[2][1] = (sint32)(yz - wx) >> 15;
                    cur_matrix->m[1][2] = (sint32)(yz + wx) >> 15;
                }
                SetRotMatrix(cur_matrix);

                delta.x = cur_matrix->t[0] - (uint32)actor->spheres[1].pos.x;
                delta.y = cur_matrix->t[1] - (uint32)actor->spheres[1].pos.y;
                delta.z = cur_matrix->t[2] - (uint32)actor->spheres[1].pos.z;
                actor->spheres[0].pos.x = actor->spheres[0].pos.x + (uint32)delta.x;
                actor->spheres[0].pos.y = actor->spheres[0].pos.y + (uint32)delta.y;
                actor->spheres[0].pos.z = actor->spheres[0].pos.z + (uint32)delta.z;
                actor->world_pos.x = actor->world_pos.x + (uint32)delta.x;
                actor->world_pos.y = actor->world_pos.y + (uint32)delta.y;
                actor->world_pos.z = actor->world_pos.z + (uint32)delta.z;
                child_reference = node->links.first_child;
                linked_child = child_reference != 0 ? (AnimatedObject *)child_reference : NULL;
                {
                    uint32 c0 = (uint32)actor->world_pos.x, c1 = (uint32)actor->world_pos.y, c2 = actor->world_pos.z;
                    linked_child->world_pos.x = c0;
                    linked_child->world_pos.y = c1;
                    linked_child->world_pos.z = c2;
                }
                matrix = render_view.matrix_cursor;
                cur_matrix = &matrix->matrix;
                parent_matrix = matrix->prev != 0 ? &matrix->prev->matrix : 0;
                linked_child->spheres[0].pos.x = (uint32)linked_child->world_pos.x - cur_matrix->t[0];
                linked_child->spheres[0].pos.y = (uint32)linked_child->world_pos.y - cur_matrix->t[1];
                linked_child->spheres[0].pos.z = (uint32)linked_child->world_pos.z - cur_matrix->t[2];
                goto cache_actor;
            }
            {
                VECTOR rotation_input = {actor->pos.x, actor->pos.y, actor->pos.z, 0};
                VECTOR rotation_output;
                ApplyRotMatrixLV(&rotation_input, &rotation_output);
                cur_matrix->t[0] = rotation_output.vx;
                cur_matrix->t[1] = rotation_output.vy;
                cur_matrix->t[2] = rotation_output.vz;
            }
            matrix = render_view.matrix_cursor;
            cur_matrix = &matrix->matrix;
            parent_matrix = matrix->prev != 0 ? &matrix->prev->matrix : 0;
            x = cur_matrix->t[0] + (uint32)parent_matrix->t[0];
            y = cur_matrix->t[1] + (uint32)parent_matrix->t[1];
            z = cur_matrix->t[2] + (uint32)parent_matrix->t[2];
            cur_matrix->t[0] = x;
            cur_matrix->t[1] = y;
            cur_matrix->t[2] = z;

            if (actor->orientation[0] == 0x4000)
            {
                stack_src = parent_matrix;
                memcpy(cur_matrix->m, stack_src->m, sizeof(cur_matrix->m));
            }
            else
            {
                {
                    uint32 qw = actor->orientation[0], qx = actor->orientation[1], qy = actor->orientation[2], qz = actor->orientation[3];
                    uint32 xx = qx * qx, yy = qy * qy, zz = qz * qz, xy = qx * qy, xz = qx * qz, yz = qy * qz, wz = qw * qz, wy = qw * qy, wx = qw * qx;
                    cur_matrix->m[0][0] = (sint32)((1u << 27) - (yy + zz)) >> 15;
                    cur_matrix->m[1][1] = (sint32)((1u << 27) - (xx + zz)) >> 15;
                    cur_matrix->m[2][2] = (sint32)((1u << 27) - (xx + yy)) >> 15;
                    cur_matrix->m[1][0] = (sint32)(xy - wz) >> 15;
                    cur_matrix->m[0][1] = (sint32)(xy + wz) >> 15;
                    cur_matrix->m[2][0] = (sint32)(xz + wy) >> 15;
                    cur_matrix->m[0][2] = (sint32)(xz - wy) >> 15;
                    cur_matrix->m[2][1] = (sint32)(yz - wx) >> 15;
                    cur_matrix->m[1][2] = (sint32)(yz + wx) >> 15;
                }
                MulRotMatrix(cur_matrix);
                stack_src = cur_matrix;
                {
                    SetRotMatrix(stack_src);
                }
            }
            if ((work & 0xCC) != 0)
            {
                matrix = render_view.matrix_cursor;
                cur_matrix = &matrix->matrix;
                parent_matrix = matrix->prev != 0 ? &matrix->prev->matrix : 0;
                actor->world_pos.x = (uint32)actor->spheres[0].pos.x + cur_matrix->t[0];
                actor->world_pos.y = (uint32)actor->spheres[0].pos.y + cur_matrix->t[1];
                actor->world_pos.z = (uint32)actor->spheres[0].pos.z + cur_matrix->t[2];
            cache_actor:
                if ((actor->flags & 0x100000) != 0 && actor->mesh_span.polygon_count != 0)
                {
                    col_cache = actor->col_transforms;
                    col_cache = col_cache;
                    if (actor->mesh_span.first_polygon == col_cache->first_face)
                    {
                        if ((work & 0x8C) != 0)
                        {
                            col_cache->motion.z = 0;
                            col_cache->motion.y = 0;
                            col_cache->motion.x = 0;
                        }
                        else
                        {
                            matrix = render_view.matrix_cursor;
                            cur_matrix = &matrix->matrix;
                            parent_matrix = matrix->prev != 0 ? &matrix->prev->matrix : 0;
                            col_cache->motion.x = cur_matrix->t[0] - (uint32)col_cache->pos.x;
                            col_cache->motion.y = cur_matrix->t[1] - (uint32)col_cache->pos.y;
                            col_cache->motion.z = cur_matrix->t[2] - (uint32)col_cache->pos.z;
                        }
                        matrix = render_view.matrix_cursor;
                        cur_matrix = &matrix->matrix;
                        parent_matrix = matrix->prev != 0 ? &matrix->prev->matrix : 0;
                        {
                            uint32 c0 = cur_matrix->t[0], c1 = cur_matrix->t[1], c2 = cur_matrix->t[2];
                            col_cache->pos.x = c0;
                            col_cache->pos.y = c1;
                            col_cache->pos.z = c2;
                        }
                        matrix = render_view.matrix_cursor;
                        cur_matrix = &matrix->matrix;
                        parent_matrix = matrix->prev != 0 ? &matrix->prev->matrix : 0;
                        for (i = 0; i < 9; ++i)
                            col_cache->rotation[i] = cur_matrix->m[i % 3][i / 3];
                    }
                }
                node->flags = node->flags & 0xBF;
            }
            if ((work & 0x14) == 0x10)
            {
                if ((CameraProjection::IsSphereVisible(actor->world_pos.comp, actor->bounds.radius) & 0xFF) != 0)
                {
                    flags = actor->flags;
                    if ((flags & 0x200) != 0)
                    {
                        work |= 0x40;
                        actor->flags = flags & 0xFFFFFDFF;
                    }
                    if ((actor->flags & 0x80) != 0)
                        work &= 0xFFFFFFEF;
                    else
                    {
                        MaterialAnimationList::UpdateMaterialAnimationList(actor->material_animation_entries);
                        matrix = render_view.matrix_cursor;
                        cur_matrix = &matrix->matrix;
                        parent_matrix = matrix->prev != 0 ? &matrix->prev->matrix : 0;
                        SceneLighting::SetLightingBounds(cur_matrix->t, (sint32)actor->bounds.radius >> 5);
                        if (actor->mesh_span.polygon_count != 0)
                            (actor)->DrawGeometry(&actor->mesh_span);
                    }
                }
                else
                {
                    work &= 0xFFFFFFEF;
                    goto mark_actor_hidden;
                }
            }
            else if (parent != 0 && (parent_view->flags & 0x200) != 0)
            {
            mark_actor_hidden:
                if ((work & 0x40) != 0 && (actor->flags & 0x30200) == 0x200)
                {
                    work &= 0xFFFFFFBF;
                    node->flags = node->flags | 0x40;
                }
                else
                {
                    work &= 0xFFFFFFFD;
                    actor->flags = actor->flags | 0x200;
                }
            }
            else
            {
                flags = actor->flags;
                if ((flags & 0x200) != 0)
                {
                    work |= 0x40;
                    actor->flags = flags & 0xFFFFFDFF;
                }
            }
            if (work != 0)
            {
                node->links.UpdateGraph(actor, work);
                if ((work & 0x14) == 0x10)
                    actor->flags = actor->flags & 0xFFFFFFFE;
            }
            if (actor->behavior == 22)
            {
                child_reference = node->links.first_child;
                linked_child = child_reference != 0 ? (AnimatedObject *)child_reference : NULL;
                flags = linked_child->flags;
                if ((flags & 0x20200) != 0x200)
                {
                    linked_child->flags = flags & 0xFFFDFFFF;
                    {
                        uint32 c0 = (uint32)linked_child->spheres[0].pos.x, c1 = (uint32)linked_child->spheres[0].pos.y, c2 = linked_child->spheres[0].pos.z;
                        linked_child->world_pos.x = c0;
                        linked_child->world_pos.y = c1;
                        linked_child->world_pos.z = c2;
                    }
                    x = linked_child->spheres[0].radius;
                    linked_child->bounds.depth = x;
                    linked_child->bounds.height = x;
                    linked_child->bounds.radius = x;
                    x = linked_child->spheres[0].pos.x;
                    y = linked_child->spheres[0].pos.y;
                    z = linked_child->spheres[0].pos.z;
                    w = linked_child->spheres[0].radius;
                    actor->spheres[0].pos.x = x;
                    actor->spheres[0].pos.y = y;
                    actor->spheres[0].pos.z = z;
                    actor->spheres[0].radius = w;
                    {
                        uint32 c0 = (uint32)linked_child->world_pos.x, c1 = (uint32)linked_child->world_pos.y, c2 = linked_child->world_pos.z;
                        actor->world_pos.x = c0;
                        actor->world_pos.y = c1;
                        actor->world_pos.z = c2;
                    }
                    {
                        uint32 c0 = linked_child->bounds.radius, c1 = linked_child->bounds.height, c2 = linked_child->bounds.depth;
                        actor->bounds.radius = c0;
                        actor->bounds.height = c1;
                        actor->bounds.depth = c2;
                    }
                }
                matrix = render_view.matrix_cursor;
                cur_matrix = &matrix->matrix;
                parent_matrix = matrix->prev != 0 ? &matrix->prev->matrix : 0;
                {
                    uint32 c0 = cur_matrix->t[0], c1 = cur_matrix->t[1], c2 = cur_matrix->t[2];
                    actor->spheres[1].pos.x = c0;
                    actor->spheres[1].pos.y = c1;
                    actor->spheres[1].pos.z = c2;
                }
                if (actor->contact_kind != 0)
                    actor->path_motion_scale = 4096;
                idx = actor->movement_mode;
                if (idx != 0)
                {
                    const ObjectCallback operation = object_operations.motions[idx];
                    if (operation.member_handler)
                        (actor->*operation.member_handler)();
                    else if (operation.void_member_handler)
                        (actor->*operation.void_member_handler)();
                    else if (operation.factory_handler)
                        operation.factory_handler(actor);
                    else
                        operation.plain_handler();
                }
            }
            goto next_object;
        }
        if (category == 7)
        {
            spr = (SpriteRecord *)node;
            {
                VECTOR rotation_input = {spr->pos.x, spr->pos.y, spr->pos.z, 0};
                VECTOR rotation_output;
                ApplyRotMatrixLV(&rotation_input, &rotation_output);
                cur_matrix->t[0] = rotation_output.vx;
                cur_matrix->t[1] = rotation_output.vy;
                cur_matrix->t[2] = rotation_output.vz;
            }
            matrix = render_view.matrix_cursor;
            cur_matrix = &matrix->matrix;
            parent_matrix = matrix->prev != 0 ? &matrix->prev->matrix : 0;
            x = cur_matrix->t[0] + (uint32)parent_matrix->t[0];
            y = cur_matrix->t[1] + (uint32)parent_matrix->t[1];
            z = cur_matrix->t[2] + (uint32)parent_matrix->t[2];
            cur_matrix->t[0] = x;
            cur_matrix->t[1] = y;
            cur_matrix->t[2] = z;

            if ((work & 0x14) == 0x10)
            {
                if (parent == 0)
                {
                    SpriteRecord::UpdateSprExtent(spr);
                    if ((node->state_flags & 0x10) != 0)
                        render_view.far_distance = 0x1FFFFC00;
                    matrix = render_view.matrix_cursor;
                    cur_matrix = &matrix->matrix;
                    parent_matrix = matrix->prev != 0 ? &matrix->prev->matrix : 0;
                    if ((CameraProjection::IsSphereVisible(cur_matrix->t, (uint32)(spr->extent[0] + spr->extent[1]) << 5) & 0xFF) != 0)
                    {
                        if ((MaterialAnimationList::UpdateMaterialAnimationList(spr->animation_entries) & 0xFF) == 0)
                        {
                            DetachChild(node);
                            SceneNode::ReleaseSceneObject(node);
                        }
                        else
                        {
                            game_flow.transition_flags = game_flow.transition_flags | 0x200;
                            spr->Draw();
                            game_flow.transition_flags = game_flow.transition_flags & 0xFFFFFDFF;
                        }
                    }
                    render_view.far_distance = camera_proj.far_distance;
                    goto next_object;
                }
                stack_src = parent_matrix;
                memcpy(cur_matrix->m, stack_src->m, sizeof(cur_matrix->m));

                if ((MaterialAnimationList::UpdateMaterialAnimationList(spr->animation_entries) & 0xFF) != 0)
                    spr->Draw();
                else
                {
                    node->ResetPlayback((const SceneNode *)parent);
                    work = (work & 0xFFFFFF36) | 0x81;
                    node->flags = node->flags | 1;
                }
            }
            else if ((node->state_flags & 2) != 0)
            {
                DetachChild(node);
                SceneNode::ReleaseSceneObject(node);
                goto next_object;
            }
            if (node->links.first_child != 0)
            {
                matrix = render_view.matrix_cursor;
                cur_matrix = &matrix->matrix;
                parent_matrix = matrix->prev != 0 ? &matrix->prev->matrix : 0;
                stack_src = parent_matrix;
                memcpy(cur_matrix->m, stack_src->m, sizeof(cur_matrix->m));
                goto visit_children;
            }
            goto next_object;
        }
        if (category == 8)
        {
            billboard = (PositionedBillboardNode *)node;
            {
                VECTOR rotation_input = {billboard->pos.x, billboard->pos.y, billboard->pos.z, 0};
                VECTOR rotation_output;
                ApplyRotMatrixLV(&rotation_input, &rotation_output);
                billboard->world_pos.x = rotation_output.vx;
                billboard->world_pos.y = rotation_output.vy;
                billboard->world_pos.z = rotation_output.vz;
            }
            matrix = render_view.matrix_cursor;
            cur_matrix = &matrix->matrix;
            parent_matrix = matrix->prev != 0 ? &matrix->prev->matrix : 0;
            billboard->world_pos.x = (uint32)billboard->world_pos.x + parent_matrix->t[0];
            billboard->world_pos.y = (uint32)billboard->world_pos.y + parent_matrix->t[1];
            billboard->world_pos.z = (uint32)billboard->world_pos.z + parent_matrix->t[2];
            if ((work & 0x14) != 0x10 || (parent == 0 && (CameraProjection::IsSphereVisible(billboard->world_pos.comp, billboard->billboard.extent) & 0xFF) == 0))
            {
                node->flags = node->flags | 4;
                goto next_object;
            }
            SetRotMatrix(&render_view.camera_transform);

            {
                VECTOR rotation_input = {billboard->world_pos.x, billboard->world_pos.y, billboard->world_pos.z, 0};
                VECTOR rotation_output;
                ApplyRotMatrixLV(&rotation_input, &rotation_output);
                (&render_view.matrix_cursor->matrix)->t[0] = rotation_output.vx;
                (&render_view.matrix_cursor->matrix)->t[1] = rotation_output.vy;
                (&render_view.matrix_cursor->matrix)->t[2] = rotation_output.vz;
            }
            matrix = render_view.matrix_cursor;
            cur_matrix = &matrix->matrix;
            parent_matrix = matrix->prev != 0 ? &matrix->prev->matrix : 0;
            for (i = 0; i < 3; ++i)
                cur_matrix->t[i] = cur_matrix->t[i] + (uint32)render_view.camera_transform.t[i];
            node->DrawBillboard();
            node->flags = node->flags & 0xFB;
            goto next_object;
        }
        if (category == 2)
        {
            SceneTrigger *trigger = (SceneTrigger *)node;
            {
                VECTOR rotation_input = {trigger->pos.x, trigger->pos.y, trigger->pos.z, 0};
                VECTOR rotation_output;
                ApplyRotMatrixLV(&rotation_input, &rotation_output);
                cur_matrix->t[0] = rotation_output.vx;
                cur_matrix->t[1] = rotation_output.vy;
                cur_matrix->t[2] = rotation_output.vz;
            }
            matrix = render_view.matrix_cursor;
            cur_matrix = &matrix->matrix;
            parent_matrix = matrix->prev != 0 ? &matrix->prev->matrix : 0;
            x = cur_matrix->t[0] + (uint32)parent_matrix->t[0];
            y = cur_matrix->t[1] + (uint32)parent_matrix->t[1];
            z = cur_matrix->t[2] + (uint32)parent_matrix->t[2];
            cur_matrix->t[0] = x;
            cur_matrix->t[1] = y;
            cur_matrix->t[2] = z;

            if ((trigger->node.state_flags & 0xFF20) != 0x0120)
            {
                if (trigger->rotation[0] == 0x4000)
                {
                    stack_src = parent_matrix;
                    memcpy(cur_matrix->m, stack_src->m, sizeof(cur_matrix->m));
                }
                else
                {
                    {
                        uint32 qw = trigger->rotation[0], qx = trigger->rotation[1], qy = trigger->rotation[2], qz = trigger->rotation[3];
                        uint32 xx = qx * qx, yy = qy * qy, zz = qz * qz, xy = qx * qy, xz = qx * qz, yz = qy * qz, wz = qw * qz, wy = qw * qy, wx = qw * qx;
                        cur_matrix->m[0][0] = (sint32)((1u << 27) - (yy + zz)) >> 15;
                        cur_matrix->m[1][1] = (sint32)((1u << 27) - (xx + zz)) >> 15;
                        cur_matrix->m[2][2] = (sint32)((1u << 27) - (xx + yy)) >> 15;
                        cur_matrix->m[1][0] = (sint32)(xy - wz) >> 15;
                        cur_matrix->m[0][1] = (sint32)(xy + wz) >> 15;
                        cur_matrix->m[2][0] = (sint32)(xz + wy) >> 15;
                        cur_matrix->m[0][2] = (sint32)(xz - wy) >> 15;
                        cur_matrix->m[2][1] = (sint32)(yz - wx) >> 15;
                        cur_matrix->m[1][2] = (sint32)(yz + wx) >> 15;
                    }
                    MulRotMatrix(cur_matrix);
                    stack_src = cur_matrix;
                    {
                        SetRotMatrix(stack_src);
                    }
                }
            }
            else
            {
                trigger_path = trigger->parameters.path_pair.targets[0];
                trigger_path = trigger_path;
                trigger_definition = trigger_path->definition;
                x = trigger->parameters.path_pair.tangent_dir;
                {
                    VECTOR sdk_output;
                    trigger_definition->SampleTangent(&sdk_output, (sint32)x < 0 ? 0 : (sint32)x);
                    input.x = sdk_output.vx;
                    input.y = sdk_output.vy;
                    input.z = sdk_output.vz;
                }
                if ((sint32)x >= 0)
                {
                    angle = Atan2Angle(input.z, input.x);
                    Quat::ComposeHeadingQuat(angle, 0, &trigger->quat);
                    matrix = render_view.matrix_cursor;
                    cur_matrix = &matrix->matrix;
                    parent_matrix = matrix->prev != 0 ? &matrix->prev->matrix : 0;
                    if (trigger->rotation[0] == 0x4000)
                    {
                        stack_src = root_matrix;
                        memcpy(cur_matrix->m, stack_src->m, sizeof(cur_matrix->m));
                    }
                    else
                    {
                        {
                            uint32 qw = trigger->rotation[0], qx = trigger->rotation[1], qy = trigger->rotation[2], qz = trigger->rotation[3];
                            uint32 xx = qx * qx, yy = qy * qy, zz = qz * qz, xy = qx * qy, xz = qx * qz, yz = qy * qz, wz = qw * qz, wy = qw * qy, wx = qw * qx;
                            tmp_matrix.m[0][0] = (sint32)((1u << 27) - (yy + zz)) >> 15;
                            tmp_matrix.m[1][1] = (sint32)((1u << 27) - (xx + zz)) >> 15;
                            tmp_matrix.m[2][2] = (sint32)((1u << 27) - (xx + yy)) >> 15;
                            tmp_matrix.m[1][0] = (sint32)(xy - wz) >> 15;
                            tmp_matrix.m[0][1] = (sint32)(xy + wz) >> 15;
                            tmp_matrix.m[2][0] = (sint32)(xz + wy) >> 15;
                            tmp_matrix.m[0][2] = (sint32)(xz - wy) >> 15;
                            tmp_matrix.m[2][1] = (sint32)(yz - wx) >> 15;
                            tmp_matrix.m[1][2] = (sint32)(yz + wx) >> 15;
                        }
                        MulMatrix0(root_matrix, &tmp_matrix, &render_view.matrix_cursor->matrix);
                    }
                }
                else
                {
                    angle = AsinAngle(-(uint32)input.y, 4096);
                    c = SinQ14((1024 - (angle & 0xFFF))) >> 2;
                    angle = AsinAngle(-(uint32)input.y, 4096);
                    s = SinQ14((angle & 0xFFF)) >> 2;
                    rotation_x.m[1][1] = c;
                    rotation_x.m[2][1] = s;
                    rotation_x.m[1][2] = -s;
                    rotation_x.m[2][2] = c;
                    angle = Atan2Angle(input.z, input.x);
                    c = SinQ14((1024 - angle)) >> 2;
                    angle = Atan2Angle(input.z, input.x);
                    s = SinQ14(angle) >> 2;
                    rotation_y.m[0][0] = c;
                    rotation_y.m[2][0] = -s;
                    rotation_y.m[0][2] = s;
                    rotation_y.m[2][2] = c;
                    MulMatrix0(&rotation_y, &rotation_x, &tmp_matrix);
                    MulMatrix0(root_matrix, &tmp_matrix, &render_view.matrix_cursor->matrix);
                    stack_src = &render_view.matrix_cursor->matrix;
                    {
                        SetRotMatrix(stack_src);
                    }
                }
            }
            trigger->node.state_flags = (trigger->node.state_flags & 0xFF00) | (uint8)((trigger->node.state_flags & 0xFF) | 0x40);

            if ((work & 0x8C) != 0)
            {
                trigger->delta.z = 0;
                trigger->delta.y = 0;
                trigger->delta.x = 0;
            }
            else
            {
                matrix = render_view.matrix_cursor;
                cur_matrix = &matrix->matrix;
                parent_matrix = matrix->prev != 0 ? &matrix->prev->matrix : 0;
                trigger->delta.x = (uint32)cur_matrix->t[0] - trigger->transform.t[0];
                trigger->delta.y = (uint32)cur_matrix->t[1] - trigger->transform.t[1];
                trigger->delta.z = (uint32)cur_matrix->t[2] - trigger->transform.t[2];
            }
            matrix = render_view.matrix_cursor;
            cur_matrix = &matrix->matrix;
            parent_matrix = matrix->prev != 0 ? &matrix->prev->matrix : 0;
            {
                uint32 c0 = cur_matrix->t[0], c1 = cur_matrix->t[1], c2 = cur_matrix->t[2];
                trigger->transform.t[0] = c0;
                trigger->transform.t[1] = c1;
                trigger->transform.t[2] = c2;
            }
            matrix = render_view.matrix_cursor;
            cur_matrix = &matrix->matrix;
            parent_matrix = matrix->prev != 0 ? &matrix->prev->matrix : 0;
            for (i = 0; i < 9; ++i)
                trigger->transform.m[i / 3][i % 3] = cur_matrix->m[i % 3][i / 3];

            if (parent != 0)
            {
                if ((parent_view->flags & 0x200) != 0)
                    trigger->node.flags = trigger->node.flags | 4;
                else if ((trigger->node.flags & 4) != 0)
                {
                    if ((work & 2) == 0)
                    {
                        trigger->delta.z = 0;
                        trigger->delta.y = 0;
                        trigger->delta.x = 0;
                    }
                    trigger->node.flags = trigger->node.flags & 0xFB;
                }
            }
            goto visit_children;
        }
        if (category == 6)
        {
            sphere_node = (SphereNode *)node;
            {
                VECTOR rotation_input = {sphere_node->pos.x, sphere_node->pos.y, sphere_node->pos.z, 0};
                VECTOR rotation_output;
                ApplyRotMatrixLV(&rotation_input, &rotation_output);
                cur_matrix->t[0] = rotation_output.vx;
                cur_matrix->t[1] = rotation_output.vy;
                cur_matrix->t[2] = rotation_output.vz;
            }
            matrix = render_view.matrix_cursor;
            cur_matrix = &matrix->matrix;
            parent_matrix = matrix->prev != 0 ? &matrix->prev->matrix : 0;
            x = cur_matrix->t[0] + (uint32)parent_matrix->t[0];
            y = cur_matrix->t[1] + (uint32)parent_matrix->t[1];
            z = cur_matrix->t[2] + (uint32)parent_matrix->t[2];
            cur_matrix->t[0] = x;
            cur_matrix->t[1] = y;
            cur_matrix->t[2] = z;

            if ((work & 0x40) != 0 && this == (ObjectLinks *)parent)
            {
                {
                    uint32 c0 = cur_matrix->t[0], c1 = cur_matrix->t[1], c2 = cur_matrix->t[2];
                    parent_view->spheres[0].pos.x = c0;
                    parent_view->spheres[0].pos.y = c1;
                    parent_view->spheres[0].pos.z = c2;
                }
                flags = sphere_node->radius & 0xFFFFFFF8;
                parent_view->spheres[0].radius = flags;
                parent_view->spheres[0].radius = flags | ((node->state_flags >> 8) & 7);
            }
            goto visit_children;
        }
        if (category == 3)
        {
            path = (ScenePath *)node;
            follow = (work & 4) == 0 && path == camera_motion.path;
            if (follow != 0)
            {
                path_matrix = path->offset_matrix;
                delta.x = path_matrix->t[0];
                delta.y = path_matrix->t[1];
                delta.z = path_matrix->t[2];
            }
            path_matrix = path->offset_matrix;
            matrix = render_view.matrix_cursor;
            cur_matrix = &matrix->matrix;
            parent_matrix = matrix->prev != 0 ? &matrix->prev->matrix : 0;
            *path_matrix = *parent_matrix;
            if (follow != 0)
            {
                player = CharacterController::FindPlayerObject(0);
                if (player != 0 && (player->flags & 0x2000) == 0)
                {
                    camera_motion.desired_pos.x = camera_motion.desired_pos.x + (uint32)delta.x;
                    camera_motion.desired_pos.y = camera_motion.desired_pos.y + (uint32)delta.y;
                    camera_motion.desired_pos.z = camera_motion.desired_pos.z + (uint32)delta.z;
                    path_matrix = path->offset_matrix;
                    delta.x = path_matrix->t[0] - (uint32)delta.x;
                    path_matrix = path->offset_matrix;
                    delta.y = path_matrix->t[1] - (uint32)delta.y;
                    path_matrix = path->offset_matrix;
                    delta.z = path_matrix->t[2] - (uint32)delta.z;
                    if ((parent_view->flags & 0x20) != 0)
                    {
                        tmp.x = camera_motion.desired_pos.x - (uint32)camera_motion.pos.x;
                        tmp.y = camera_motion.desired_pos.y - (uint32)camera_motion.pos.y;
                        tmp.z = camera_motion.desired_pos.z - (uint32)camera_motion.pos.z;
                        CameraMotion::SetCameraTransition(tmp);

                        if (camera_motion.smooth_ticks[0] < 4)
                        {
                            camera_motion.smooth_ticks[0] = 4;
                        }
                        if (camera_motion.smooth_ticks[1] < 4)
                            camera_motion.smooth_ticks[1] = 4;

                        if (camera_motion.smooth_ticks[2] < 4)
                            camera_motion.smooth_ticks[2] = 4;
                    }

                    camera_motion.pos.x = camera_motion.pos.x + (uint32)delta.x;
                    camera_motion.pos.y = camera_motion.pos.y + (uint32)delta.y;
                    camera_motion.pos.z = camera_motion.pos.z + (uint32)delta.z;
                }
            }
            goto visit_children;
        }
        if (category == 9)
        {
            definition = (PathDefinition *)node;
            if (definition->transform != 0)
            {
                follow = 0;
                player = 0;
                if ((work & 4) == 0 && definition == camera_motion.path->definition)
                {
                    player = CharacterController::FindPlayerObject(0);
                    if (player != 0 && definition == player->path_definition)
                        follow = (player->flags & 0x2400) == 0x400;
                }
                if (follow != 0)
                {
                    player_definition = player->path_definition;
                    if (player_definition != 0 && player_definition->transform != 0)
                    {
                        {
                            uint32 c0 = (uint32)player->pos.x, c1 = (uint32)player->pos.y, c2 = player->pos.z;
                            tmp.x = c0;
                            tmp.y = c1;
                            tmp.z = c2;
                        }
                        PushMatrix();
                        definition_matrix = player->path_definition->transform != 0 ? &player->path_definition->transform->matrix : NULL;
                        {
                            SetRotMatrix(definition_matrix);
                        }
                        {
                            VECTOR sdk_output;
                            {
                                VECTOR sdk_input = {tmp.x, tmp.y, tmp.z, 0};
                                ApplyRotMatrixLV(&sdk_input, &sdk_output);
                            }
                            delta.x = sdk_output.vx;
                            delta.y = sdk_output.vy;
                            delta.z = sdk_output.vz;
                        }
                        PopMatrix();
                        definition_matrix = player->path_definition->transform != 0 ? &player->path_definition->transform->matrix : NULL;
                        delta.x = delta.x + (uint32)definition_matrix->t[0];
                        definition_matrix = player->path_definition->transform != 0 ? &player->path_definition->transform->matrix : NULL;
                        delta.y = delta.y + (uint32)definition_matrix->t[1];
                        definition_matrix = player->path_definition->transform != 0 ? &player->path_definition->transform->matrix : NULL;
                        delta.z = delta.z + (uint32)definition_matrix->t[2];
                    }
                    else
                    {
                        {
                            uint32 c0 = (uint32)player->pos.x, c1 = (uint32)player->pos.y, c2 = player->pos.z;
                            delta.x = c0;
                            delta.y = c1;
                            delta.z = c2;
                        }
                    }
                }
                matrix = render_view.matrix_cursor;
                cur_matrix = &matrix->matrix;
                parent_matrix = matrix->prev != 0 ? &matrix->prev->matrix : 0;
                path_transform = definition->transform;
                render_view.matrix_cursor = matrix->prev;

                if ((work & 0x8C) != 0)
                {
                    path_transform->displacement.z = 0;
                    path_transform->displacement.y = 0;
                    path_transform->displacement.x = 0;
                }
                else
                {
                    matrix = render_view.matrix_cursor;
                    cur_matrix = &matrix->matrix;
                    parent_matrix = matrix->prev != 0 ? &matrix->prev->matrix : 0;
                    path_transform->displacement.x = (uint32)cur_matrix->t[0] - path_transform->matrix.t[0];
                    path_transform->displacement.y = (uint32)cur_matrix->t[1] - path_transform->matrix.t[1];
                    path_transform->displacement.z = (uint32)cur_matrix->t[2] - path_transform->matrix.t[2];
                }
                matrix = render_view.matrix_cursor;
                cur_matrix = &matrix->matrix;
                parent_matrix = matrix->prev != 0 ? &matrix->prev->matrix : 0;
                for (i = 0; i < 3; ++i)
                    path_transform->matrix.t[i] = cur_matrix->t[i];
                matrix = render_view.matrix_cursor;
                cur_matrix = &matrix->matrix;
                parent_matrix = matrix->prev != 0 ? &matrix->prev->matrix : 0;
                for (i = 0; i < 9; ++i)
                    path_transform->matrix.m[i / 3][i % 3] = cur_matrix->m[i % 3][i / 3];

                if (parent != 0)
                {
                    if ((parent_view->flags & 0x200) != 0)
                        node->flags = node->flags | 4;
                    else if ((node->flags & 4) != 0)
                    {
                        if ((work & 2) == 0)
                        {
                            path_transform->displacement.z = 0;
                            path_transform->displacement.y = 0;
                            path_transform->displacement.x = 0;
                        }
                        node->flags = node->flags & 0xFB;
                    }
                }
                path_transform = definition->transform;
                stack_src = &render_view.matrix_cursor->matrix;
                for (i = 0; i < 3; ++i)
                {
                    for (uint32 column = 0; column < 3; ++column)
                        path_transform->matrix.m[i][column] = stack_src->m[i][column];
                }
                render_view.matrix_cursor = render_view.matrix_cursor->next;
                if (follow != 0)
                {
                    parent_view->flags = parent_view->flags & 0xFFFFFDFF;
                    path_transform = definition->transform;
                    node->flags = node->flags & 0xFB;
                    if (path_transform->displacement.y < 0 || (work & 8) != 0)
                    {
                        player_definition = player->path_definition;
                        if (player_definition != 0 && player_definition->transform != 0)
                        {
                            {
                                uint32 c0 = (uint32)player->pos.x, c1 = (uint32)player->pos.y, c2 = player->pos.z;
                                before.x = c0;
                                before.y = c1;
                                before.z = c2;
                            }
                            PushMatrix();
                            definition_matrix = player->path_definition->transform != 0 ? &player->path_definition->transform->matrix : NULL;
                            SetRotMatrix(definition_matrix);

                            {
                                VECTOR rotation_output;
                                {
                                    VECTOR sdk_input = {before.x, before.y, before.z, 0};
                                    ApplyRotMatrixLV(&sdk_input, &rotation_output);
                                }
                                player->pos.x = rotation_output.vx;
                                player->pos.y = rotation_output.vy;
                                player->pos.z = rotation_output.vz;
                            }
                            PopMatrix();
                            player_definition = player->path_definition;
                            definition_matrix = player_definition->transform != 0 ? &player_definition->transform->matrix : NULL;
                            player->pos.x = (uint32)player->pos.x + definition_matrix->t[0];
                            definition_matrix = player_definition->transform != 0 ? &player_definition->transform->matrix : NULL;
                            x = definition_matrix->t[1];
                            player_definition = player->path_definition;
                            player->pos.y = player->pos.y + x;
                            definition_matrix = player_definition->transform != 0 ? &player_definition->transform->matrix : NULL;
                            player->pos.z = (uint32)player->pos.z + definition_matrix->t[2];
                        }
                        else
                        {
                            {
                                uint32 c0 = (uint32)player->pos.x, c1 = (uint32)player->pos.y, c2 = player->pos.z;
                                player->pos.x = c0;
                                player->pos.y = c1;
                                player->pos.z = c2;
                            }
                        }
                        player->pos.y = delta.y;
                        player_definition = player->path_definition;
                        if (player_definition != 0 && player_definition->transform != 0)
                        {
                            definition_matrix = player->path_definition->transform != 0 ? &player->path_definition->transform->matrix : NULL;
                            relative.x = (uint32)player->pos.x - definition_matrix->t[0];
                            definition_matrix = player->path_definition->transform != 0 ? &player->path_definition->transform->matrix : NULL;
                            relative.y = (uint32)player->pos.y - definition_matrix->t[1];
                            definition_matrix = player->path_definition->transform != 0 ? &player->path_definition->transform->matrix : NULL;
                            relative.z = (uint32)player->pos.z - definition_matrix->t[2];
                            for (i = 0; i < 9; ++i)
                            {
                                definition_matrix = player->path_definition->transform != 0 ? &player->path_definition->transform->matrix : NULL;
                                transposed.m[i / 3][i % 3] = definition_matrix->m[i % 3][i / 3];
                            }
                            PushMatrix();
                            SetRotMatrix(&transposed);

                            {
                                VECTOR rotation_output;
                                {
                                    VECTOR sdk_input = {relative.x, relative.y, relative.z, 0};
                                    ApplyRotMatrixLV(&sdk_input, &rotation_output);
                                }
                                player->pos.x = rotation_output.vx;
                                player->pos.y = rotation_output.vy;
                                player->pos.z = rotation_output.vz;
                            }
                            PopMatrix();
                        }
                        else
                        {
                            {
                                uint32 c0 = (uint32)player->pos.x, c1 = (uint32)player->pos.y, c2 = player->pos.z;
                                player->pos.x = c0;
                                player->pos.y = c1;
                                player->pos.z = c2;
                            }
                        }
                    }
                }
            }
            if ((node->flags & 1) != 0)
                definition->flags = definition->flags | 1;
            else
                definition->flags = definition->flags & 0xFFFFFFFE;
            goto visit_children;
        }
        if (category == 4)
        {
            light = (SceneLight *)node;
            {
                VECTOR rotation_input = {light->pos.x, light->pos.y, light->pos.z, 0};
                {
                    VECTOR sdk_output;
                    ApplyRotMatrixLV(&rotation_input, &sdk_output);
                    tmp.x = sdk_output.vx;
                    tmp.y = sdk_output.vy;
                    tmp.z = sdk_output.vz;
                }
            }
            matrix = render_view.matrix_cursor;
            cur_matrix = &matrix->matrix;
            parent_matrix = matrix->prev != 0 ? &matrix->prev->matrix : 0;
            tmp.x = tmp.x + (uint32)parent_matrix->t[0];
            tmp.y = tmp.y + (uint32)parent_matrix->t[1];
            tmp.z = tmp.z + (uint32)parent_matrix->t[2];
            if ((work & 0x8D) != 0 || (uint32)light->prev_pos.x != (uint32)tmp.x || (uint32)light->prev_pos.y != (uint32)tmp.y || (uint32)light->prev_pos.z != (uint32)tmp.z)
            {
                {
                    uint32 c0 = (uint32)tmp.x, c1 = (uint32)tmp.y, c2 = tmp.z;
                    light->prev_pos.x = c0;
                    light->prev_pos.y = c1;
                    light->prev_pos.z = c2;
                }
                light->event_flags = light->event_flags | 1;
            }
        }
    visit_children:
        if (node->links.first_child != 0)
            node->links.UpdateGraph(parent, work);
    next_object:
        node = next_node;
    }
    matrix = render_view.matrix_cursor->prev;
    render_view.matrix_cursor = matrix;
}

void ObjectLinks::DetachChild(SceneNode *object)
{
    FUNCTION_MARKER(0x8002A210, "SLUS_005.78");
    SceneNode *next, *node;
    SceneNode *child = object;
    node = first_child;
    if (node == 0)
        return;
    if (object == node)
    {
        next = child->links.sibling_next;
        first_child = next;
    }
    else
    {
        for (;;)
        {
            next = node->links.sibling_next;
            if (next == object)
            {
                next = child->links.sibling_next;
                node->links.sibling_next = next;
                break;
            }
            node = next;
            if (node == 0)
                break;
        }
    }
    child->links.sibling_next = 0;
}
