#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "scene_node.h"
#include "game.h"
#include "collision.h"
#include "psx_gte.h"

SceneNode *registered_object_head = 0;

void SceneNode::DrawChildren(AnimatedObject *owner)
{
    // Q27 unity becomes Q12 after the coefficient shift
    SceneNode *child;
    MeshPartNode *part;
    SpriteRecord *spr;
    SphereNode *sphere_node;
    CollisionSphere *sphere;
    SVECTOR velocity;
    MATRIX x_basis = {0};
    MATRIX bounds_matrix = {0};
    Vec3 bounds_min;
    MATRIX *root_matrix = &render_view.matrix_root.matrix;
    MATRIX *cur_matrix;
    MATRIX *parent_matrix;
    MATRIX *rotation_matrix;
    RenderMatrixSlot *matrix;

    MATRIX *left;
    MATRIX *right;
    uint32 kind;
    uint32 x;
    uint32 y;
    uint32 z;
    uint32 px;
    uint32 py;
    uint32 pz;
    uint32 val;
    uint32 packed;
    uint32 rate;
    uint32 phase;
    uint32 a;
    uint32 beta;
    uint32 gamma;
    uint32 xx;
    uint32 yy;
    uint32 zz;
    uint32 xy;
    uint32 xz;
    uint32 yz;
    uint32 wz;
    uint32 wy;
    uint32 wx;
    uint32 radius;
    uint32 scale;
    Vec3 maximum;

    sint32 qw;
    sint32 qx;
    sint32 qy;
    sint32 qz;
    sint16 c;
    sint16 s;
    sint32 coefficient;
    FUNCTION_MARKER(0x80012424, "SLUS_005.78");
    matrix = render_view.matrix_cursor;
    cur_matrix = &matrix->matrix;
    parent_matrix = matrix->prev != 0 ? &matrix->prev->matrix : 0;
    // GTE rotation register 4 ignores the upper padding halfword
    memset((uint8 *)&x_basis + sizeof(x_basis.m), 0, offsetof(MATRIX, t) - sizeof(x_basis.m));
    child = links.first_child;
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
    while (child != 0)
    {
        if ((owner->node.flags & 0x40) != 0)
            owner->SampleNodeAnimation(child);
        matrix = render_view.matrix_cursor;
        cur_matrix = &matrix->matrix;
        parent_matrix = matrix->prev != 0 ? &matrix->prev->matrix : 0;
        SetRotMatrix(parent_matrix);
        kind = child->kind;
        if (kind == 5)
        {
            part = (MeshPartNode *)child;
            if (render_view.special_transform != 0)
            {
                if (owner->velocity.x != 0)
                {
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
                    x = cur_matrix->t[0];
                    px = parent_matrix->t[0];
                    y = cur_matrix->t[1];
                    py = parent_matrix->t[1];
                    z = cur_matrix->t[2];
                    pz = parent_matrix->t[2];
                    cur_matrix->t[0] = x + px;
                    cur_matrix->t[1] = y + py;
                    cur_matrix->t[2] = z + pz;
                    x = cur_matrix->t[0];
                    y = cur_matrix->t[1];
                    z = cur_matrix->t[2];
                    part->pos.x = x;
                    part->pos.y = y;
                    part->pos.z = z;
                    x = part->pos.x;
                    px = owner->world_pos.x;
                    velocity.vx = x - px;
                    y = part->pos.y;
                    py = owner->world_pos.y;
                    velocity.vy = y - py;
                    z = part->pos.z;
                    pz = owner->world_pos.z;
                    velocity.vz = z - pz;
                    val = random_generator.Next();
                    velocity.vx = 2730 - val % 5460;
                    val = random_generator.Next();
                    coefficient = velocity.vy;
                    velocity.vz = 2730 - val % 5460;
                    if (coefficient < 8192)
                        velocity.vy = 8192;
                    part->velocity.vx = velocity.vx;
                    part->velocity.vy = velocity.vy;
                    part->velocity.vz = velocity.vz;
                    a = random_generator.Next();
                    beta = random_generator.Next();
                    gamma = random_generator.Next();
                    part->rotation[3] = (a & 31) | ((beta & 31) << 5) | ((gamma & 31) << 10);
                }
                else
                {
                    qx = part->rotation[0];
                    x = part->pos.x;
                    qy = part->rotation[1];
                    y = part->pos.y;
                    qz = part->rotation[2];
                    z = part->pos.z;
                    part->pos.x = x + qx;
                    part->pos.y = y + qy;
                    part->pos.z = z + qz;
                    part->rotation[1] = (uint16)part->rotation[1] - 1228;
                    x = part->pos.x;
                    y = part->pos.y;
                    z = part->pos.z;
                    cur_matrix->t[0] = x;
                    cur_matrix->t[1] = y;
                    cur_matrix->t[2] = z;
                }
                val = owner->path_motion_scale;
                packed = part->rotation[3];
                rate = (15 - val) << 4;
                phase = (packed & 31) * rate;
                c = SinQ14((1024 - phase)) >> 2;
                s = SinQ14(phase) >> 2;
                rotation_x.m[1][1] = c;
                rotation_x.m[2][1] = s;
                rotation_x.m[1][2] = -s;
                rotation_x.m[2][2] = c;
                MulMatrix0(root_matrix, &rotation_x, &x_basis);
                phase = ((packed >> 5) & 31) * rate;
                c = SinQ14((1024 - phase)) >> 2;
                s = SinQ14(phase) >> 2;
                phase = ((packed >> 10) & 31) * rate;
                rotation_y.m[0][0] = c;
                rotation_y.m[2][0] = -s;
                rotation_y.m[0][2] = s;
                rotation_y.m[2][2] = c;
                if (phase != 0)
                {
                    MulMatrix0(&x_basis, &rotation_y, &bounds_matrix);
                    c = SinQ14((1024 - phase)) >> 2;
                    s = SinQ14(phase) >> 2;
                    rotation_z.m[0][0] = c;
                    rotation_z.m[1][0] = s;
                    rotation_z.m[0][1] = -s;
                    rotation_z.m[1][1] = c;
                    left = &rotation_z;
                    right = &bounds_matrix;
                }
                else
                {
                    left = &x_basis;
                    right = &rotation_y;
                }
                matrix = render_view.matrix_cursor;
                cur_matrix = &matrix->matrix;
                parent_matrix = matrix->prev != 0 ? &matrix->prev->matrix : 0;
                MulMatrix0(left, right, cur_matrix);
                rotation_matrix = cur_matrix;
            }
            else
            {
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
                x = cur_matrix->t[0];
                px = parent_matrix->t[0];
                y = cur_matrix->t[1];
                py = parent_matrix->t[1];
                z = cur_matrix->t[2];
                pz = parent_matrix->t[2];
                cur_matrix->t[0] = x + px;
                cur_matrix->t[1] = y + py;
                cur_matrix->t[2] = z + pz;
                qw = part->rotation[0];
                if (qw == 0x4000)
                {
                    memcpy(cur_matrix->m, parent_matrix->m, sizeof(cur_matrix->m));

                    goto node_rotation_ready;
                }
                qx = part->rotation[1];
                qy = part->rotation[2];
                qz = part->rotation[3];
                xx = qx * qx;
                yy = qy * qy;
                zz = qz * qz;
                xy = qx * qy;
                xz = qx * qz;
                yz = qy * qz;
                wz = qw * qz;
                wy = qw * qy;
                cur_matrix->m[0][0] = (sint32)((1u << 27) - (yy + zz)) >> 15;
                cur_matrix->m[1][1] = (sint32)((1u << 27) - (xx + zz)) >> 15;
                cur_matrix->m[2][2] = (sint32)((1u << 27) - (xx + yy)) >> 15;
                wx = qw * qx;
                cur_matrix->m[1][0] = (sint32)(xy - wz) >> 15;
                cur_matrix->m[0][1] = (sint32)(xy + wz) >> 15;
                cur_matrix->m[2][0] = (sint32)(xz + wy) >> 15;
                cur_matrix->m[0][2] = (sint32)(xz - wy) >> 15;
                cur_matrix->m[2][1] = (sint32)(yz - wx) >> 15;
                cur_matrix->m[1][2] = (sint32)(yz + wx) >> 15;
                MulRotMatrix(cur_matrix);
                rotation_matrix = cur_matrix;
            }
            SetRotMatrix(rotation_matrix);
        node_rotation_ready:
            if ((owner->flags & 0x200) == 0)
                owner->DrawGeometry(&part->span);
        }
        else if (kind == 7)
        {
            spr = (SpriteRecord *)child;
            if (render_view.special_transform != 0)
            {
                x = parent_matrix->t[0];
                y = parent_matrix->t[1];
                z = parent_matrix->t[2];
                cur_matrix->t[0] = x;
                cur_matrix->t[1] = y;
                cur_matrix->t[2] = z;
            }
            else
            {
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
                x = cur_matrix->t[0];
                px = parent_matrix->t[0];
                y = cur_matrix->t[1];
                py = parent_matrix->t[1];
                z = cur_matrix->t[2];
                pz = parent_matrix->t[2];
                cur_matrix->t[0] = x + px;
                cur_matrix->t[1] = y + py;
                cur_matrix->t[2] = z + pz;
            }
            if ((owner->flags & 0x20000) != 0 && (child->state_flags & 0x200) == 0)
            {
                val = child->flags;
                child->flags = val & 0xFE;
                MaterialAnimationList::ResetMaterialAnimationList(spr->animation_entries);
            }
            if ((child->flags & 1) != 0)
                goto visit_children;
            if ((child->state_flags & 0x200) == 0)
            {
                matrix = render_view.matrix_cursor;
                cur_matrix = &matrix->matrix;
                parent_matrix = matrix->prev != 0 ? &matrix->prev->matrix : 0;
                memcpy(cur_matrix->m, parent_matrix->m, sizeof(cur_matrix->m));
            }
            if ((owner->flags & 0x200) == 0)
            {
                if (((uint32)MaterialAnimationList::UpdateMaterialAnimationList(spr->animation_entries) << 24) != 0)
                {
                    render_view.object_depth_bias = (uint16)render_view.object_depth_bias - 256;
                    spr->Draw();
                    render_view.object_depth_bias = (uint16)render_view.object_depth_bias + 256;
                }
                else
                    child->flags = child->flags | 1;
            }
        }
        else if (kind == 6)
        {
            sphere_node = (SphereNode *)child;
            if (owner->behavior_suspended != 0)
            {
                goto next_node;
            }

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
            x = cur_matrix->t[0];
            px = parent_matrix->t[0];
            y = cur_matrix->t[1];
            py = parent_matrix->t[1];
            z = cur_matrix->t[2];
            pz = parent_matrix->t[2];
            cur_matrix->t[0] = x + px;
            cur_matrix->t[1] = y + py;
            cur_matrix->t[2] = z + pz;
            sphere = render_view.sphere_output;
            render_view.sphere_output = (CollisionSphere *)((uint8 *)sphere + sizeof(CollisionSphere));
            x = cur_matrix->t[0];
            y = cur_matrix->t[1];
            z = cur_matrix->t[2];
            sphere->pos.x = x;
            sphere->pos.y = y;
            sphere->pos.z = z;
            radius = sphere_node->radius & 0xFFFFFFF8;
            sphere->radius = radius;
            scale = render_view.object_scale;
            if (scale != 0)
            {
                sphere->radius = mul12((sint32)radius, (sint32)scale);
            }
            val = sphere_node->node.state_flags >> 8;
            radius = sphere->radius;
            matrix = render_view.matrix_cursor;
            cur_matrix = &matrix->matrix;
            parent_matrix = matrix->prev != 0 ? &matrix->prev->matrix : 0;
            radius |= val & 7;
            sphere->radius = radius;
            val = cur_matrix->t[0];
            radius = 0 == 0 ? radius : sphere->radius;
            bounds_min.x = val - radius;
            val = cur_matrix->t[1];
            radius = 1 == 0 ? radius : sphere->radius;
            bounds_min.y = val - radius;
            val = cur_matrix->t[2];
            radius = 2 == 0 ? radius : sphere->radius;
            bounds_min.z = val - radius;
            val = cur_matrix->t[0];
            radius = sphere->radius;
            maximum.x = val + radius;
            val = cur_matrix->t[1];
            radius = sphere->radius;
            maximum.y = val + radius;
            val = cur_matrix->t[2];
            radius = sphere->radius;
            maximum.z = val + radius;
            if (sphere == owner->spheres)
            {
                owner->world_pos = bounds_min;

                owner->bounds.maximum = maximum;
            }
            else
            {
                x = bounds_min.x;
                y = owner->world_pos.x;
                if ((sint32)x < (sint32)y)
                    owner->world_pos.x = x;
                x = maximum.x;
                y = owner->bounds.maximum.x;
                if ((sint32)y < (sint32)x)
                    owner->bounds.maximum.x = x;
                x = bounds_min.y;
                y = owner->world_pos.y;
                if ((sint32)x < (sint32)y)
                    owner->world_pos.y = x;
                x = maximum.y;
                y = owner->bounds.maximum.y;
                if ((sint32)y < (sint32)x)
                    owner->bounds.maximum.y = x;
                x = bounds_min.z;
                y = owner->world_pos.z;
                if ((sint32)x < (sint32)y)
                    owner->world_pos.z = x;
                x = maximum.z;
                y = owner->bounds.maximum.z;
                if ((sint32)y < (sint32)x)
                    owner->bounds.maximum.z = x;
            }
        }
    visit_children:
        child->DrawChildren(owner);
    next_node:
        child = child->links.sibling_next;
    }
    render_view.matrix_cursor = render_view.matrix_cursor->prev;
}

uint32 SceneNode::ResetPlayback(const SceneNode *parent)
{
    FUNCTION_MARKER(0x80013CEC, "SLUS_005.78");
    uint32 category = kind;
    uint32 cur_flags, val;
    sint32 kind_val;
    if (category == 1)
    {
        MeshGeometryNode *mesh = (MeshGeometryNode *)this;
        MaterialAnimationList::ResetMaterialAnimationList(mesh->material_animation_entries);
        flags = flags | 0x80;
        return 0;
    }
    if (category == 0)
    {
        AnimatedObject *animated = (AnimatedObject *)this;
        MaterialAnimationList::ResetMaterialAnimationList(animated->material_animation_entries);
        kind_val = animated->behavior;
        if (kind_val == 22)
        {
            animated->ResetAnimatedModel();
            flags = flags & 0x7F;
            return 1;
        }
        if ((((uint32)kind_val - 3) & 0xFF) < 2 || (((uint32)kind_val - 31) & 0xFF) < 2)
        {
            animated->flags = animated->flags & 0xFFF03EFF;
            if (parent != 0 && (parent->flags & 1) != 0)
                flags = flags | 1;
            else
            {
                cur_flags = flags;
                val = animated->playback.flags & 1;
                cur_flags &= 0xFE;
                flags = cur_flags;
                flags = cur_flags | val;
            }
            val = animated->playback.flags;
            cur_flags = animated->flags;
            kind_val = animated->behavior;
            animated->flags = cur_flags | (val & 0x40000);
            if (kind_val == 4)
                animated->frame = animated->playback.frame;
            else if (kind_val == 31)
                animated->frame = animated->linked_object->frame;
            else
                animated->frame = 0;
            animated->AdvanceTracks(0);
            flags = flags & 0x7F;
            return 1;
        }
        goto clear_hidden;
    }
    if (category == 7)
    {
        SpriteRecord *spr = (SpriteRecord *)this;
        MaterialAnimationList::ResetMaterialAnimationList(spr->animation_entries);
        if (parent == 0)
        {
            if ((state_flags & 2) != 0)
            {
                flags = flags | 1;
                return 1;
            }
            AnimationTrack *track = spr->extent_track;
            if (track != 0)
                track->frame = -1;
        }
        goto clear_hidden;
    }
    if (category == 8)
    {
        BillboardNode *billboard = (BillboardNode *)this;
        cur_flags = billboard->playback_flags;
        billboard->playback_flags = cur_flags & 0x7FFF;
        if ((cur_flags & 0x7F) == 2 && CharacterController::GetSceneHealthPickupFlag(level_resource.level_id) != 0)
        {
            flags = flags | 0x80;
            return 0;
        }
        if ((billboard->playback_flags & 0x200) != 0)
            return 0;
        goto clear_hidden;
    }
    if (category == 9)
    {
        cur_flags = ((const ActorPlacement *)this)->path.flags;
        if ((cur_flags & 8) == 0)
            goto clear_hidden;
        if ((flags & 0x80) != 0)
            return 0;
        ((ActorPlacement *)this)->path.flags = cur_flags & 0xFFFFFFFA;
        if ((cur_flags & 0x20) != 0 || (parent != 0 && (parent->flags & 1) != 0))
        {
            cur_flags = flags | 1;
            val = ((const ActorPlacement *)this)->path.flags | 1;
            flags = cur_flags;
            ((ActorPlacement *)this)->path.flags = val;
        }
        else
            flags = flags & 0xFE;
        goto clear_hidden;
    }
    if (category == 2)
    {
        SceneTrigger *trigger = (SceneTrigger *)this;
        uint32 subtype = state_flags >> 8;
        if (subtype - 40 < 16)
        {
            uint32 word0 = trigger->parameters.event.command;
            uint32 word1 = trigger->parameters.event.schedule;
            uint32 packed = (kind | (flags << 8) | ((uint32)state_flags << 16)) & 0xFF020000;
            trigger->parameters.event.command = (word0 & 0xFFF) | ((subtype - 39) << 12) | (word1 & 0x1FE0000);
            if (packed == 0x32000000 && trigger->animation_binding.tracks[2] >= 0)
            {
                trigger->animation_owner = (const AnimatedObject *)parent;
                trigger->UpdateExtentsFromAnimation();
            }
        }
        else if ((sint8)subtype == 5)
        {
            trigger->parameters.damage.deadline = 0;
        }
        if ((sint8)(state_flags >> 8) == 50 && (trigger->parameters.sound.volume_flags & 0x3FFF) == 0)
            return 0;
        goto clear_hidden;
    }
    if (category == 4)
    {
        cur_flags = (uint8)state_flags;
        state_flags = (state_flags & 0xFF00) | (cur_flags & 0xFB);
        if ((cur_flags & 8) != 0 || (parent != 0 && (parent->flags & 1) != 0))
            flags = flags | 1;
        else
            flags = flags & 0xFE;
    }
clear_hidden:
    flags = flags & 0x7F;
    return 0;
}

const MeshFace *SceneNode::SelectColFace(const SceneNode *object, uint32 selection, CollisionTransform *desc)
{
    FUNCTION_MARKER(0x8001C544, "SLUS_005.78");
    uint32 x, y, z, idx;
    const CollisionTransform *cached_transform;
    const MeshGeometryNode *mesh = object != 0 ? (const MeshGeometryNode *)object : NULL;
    const MeshGeometryNode *pos_src;
    if (desc != 0)
    {
        if (mesh->node.kind == 1)
        {
            for (idx = 0; idx < 9; ++idx)
                desc->rotation[idx] = psx_identity_matrix.m[idx / 3][idx % 3];
            if (mesh->vertex_count > 0)
            {
                x = mesh->pos.x;
                y = mesh->pos.y;
                z = mesh->pos.z;
            }
            else
            {
                pos_src = mesh->src_mesh;
                x = pos_src->pos.x;
                y = pos_src->pos.y;
                z = pos_src->pos.z;
            }
            desc->pos.x = x;
            desc->pos.y = y;
            desc->pos.z = z;
            desc->motion.z = 0;
            desc->motion.y = 0;
            desc->motion.x = 0;
        }
        else
        {
            const AnimatedObject *geometry_object = (const AnimatedObject *)object;
            cached_transform = geometry_object->col_transforms;
            cached_transform = cached_transform + (selection >> 16);

            *desc = *cached_transform;
        }
        desc->face_count = 1;
        desc->first_face = selection;
    }
    uint32 face_idx = selection & 0xFFFF;
    if (mesh->polygons == 0 && face_idx == 0)
        return 0;
    return mesh->polygons + face_idx;
}

void SceneNode::ReleaseSceneObject(SceneNode *object)
{
    FUNCTION_MARKER(0x80025EAC, "SLUS_005.78");
    SceneNode *node = object;
    AnimatedObject *actor;

    if (node->kind == 7)
    {
        SpriteRecord::ReleaseSpr((SpriteRecord *)object);
        return;
    }
    actor = (AnimatedObject *)node;
    if (actor->behavior_suspended == 2 || actor->movement_mode >= 4)
        ((ActorPlacement *)actor->path_definition)->ReleaseBoundActor();
    if ((actor->flags & 4) == 0)
        FreeGameMemory(object);
    else
    {
        node->flags = (node->flags | 0x80) & 0xFC;
    }
}

void SceneNode::MoveBetweenLists(SceneNode **src_head, SceneNode **dst_head)
{
    FUNCTION_MARKER(0x8002A144, "SLUS_005.78");
    SceneNode *cur, *prev = 0, *next;
    cur = *src_head;
    if (this != cur)
    {
        do
        {
            prev = cur;
            cur = prev->links.registry_next;
        } while (cur != 0 && this != cur);
    }
    if (prev != 0)
    {
        next = links.registry_next;
        prev->links.registry_next = next;
    }
    else if (this == *src_head)
        *src_head = links.registry_next;
    if (dst_head != 0)
    {
        cur = *dst_head;
        links.registry_next = cur;
        *dst_head = this;
    }
}

void SceneNode::RelocateClone(uint8 *dst, const uint8 *src)
{
    FUNCTION_MARKER(0x8003FF6C, "SLUS_005.78");
    SceneNode *child;
    uint32 *entries;
    child = links.first_child;
    if (child != 0)
    {
        links.first_child = (SceneNode *)(dst + ((const uint8 *)child - src));
    }
    child = links.sibling_next;
    if (child != 0)
    {
        links.sibling_next = (SceneNode *)(dst + ((const uint8 *)child - src));
    }
    if (kind == 7)
    {
        SpriteRecord *spr = (SpriteRecord *)this;
        entries = spr->animation_entries;
        if (entries != 0)
        {
            entries = (uint32 *)(dst + ((const uint8 *)entries - src));
            spr->animation_entries = entries;
            MaterialAnimationList::ResetMaterialAnimationList(entries);
        }
    }
    child = links.first_child;
    while (child != 0)
    {
        child->RelocateClone(dst, src);
        child = child->links.sibling_next;
    }
}

void SceneNode::Register()
{
    SceneNode *prev;
    FUNCTION_MARKER(0x800401A8, "SLUS_005.78");
    flags &= 0x7F;
    prev = registered_object_head;
    registered_object_head = this;
    links.registry_next = prev;
}

void SceneNode::Unregister()
{
    FUNCTION_MARKER(0x800401C8, "SLUS_005.78");
    MoveBetweenLists(&registered_object_head, 0);
}

void SceneNode::DrawBillboard() const
{
    uint32 *bucket_entry;

    POLY_FT4 *poly;
    const SceneSection *section;
    FUNCTION_MARKER(0x80064358, "SLUS_005.78");
    const SpriteRecord *spr = 0;
    const BillboardNode *billboard;
    const TextureDescriptor *desc;
    SpriteCorner corner[3];
    SVECTOR n;
    uint32 packet, limit, result, x, y, z, scale, depth, val, end, start, bucket_offset, link;
    uint32 orientation, tex, uv0, uv1, uv2, uv3, flags;
    const MATRIX *cur_matrix;
    MATRIX *work_matrix;
    sint32 width, height, bucket, distance, fade, radius;
    packet = render_commands.packet_offset;
    limit = render_commands.packet_limit;
    if (packet >= limit)
        return;
    if (kind == 8)
    {
        billboard = (const BillboardNode *)this;
        width = billboard->extent >> 5;
        height = width;
    }
    else
    {
        spr = (const SpriteRecord *)this;
        width = spr->extent[0];
        height = spr->extent[1];
        scale = render_view.object_scale;
        if (scale != 0)
        {
            width = (sint32)(width * scale) >> 12;
            height = (sint32)(height * scale) >> 12;
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
    cur_matrix = &render_view.matrix_cursor->matrix;
    x = cur_matrix->t[0];
    y = cur_matrix->t[1];
    z = cur_matrix->t[2];
    xport_gte_write_control(5, ((sint32)x >> 5));
    xport_gte_write_control(6, ((sint32)y >> 5));
    xport_gte_write_control(7, ((sint32)z >> 5));
    corner[0].vertex.vx = -(uint16)width;
    corner[0].vertex.vy = height;
    corner[0].vertex.vz = 0;
    corner[1].vertex.vx = width;
    corner[1].vertex.vy = -(uint32)height;
    corner[1].vertex.vz = 0;
    gte_ldv0(&corner[0].vertex);
    gte_ldv1(&corner[1].vertex);
    xport_gte_execute(0x280030);
    corner[0].proj.xy = xport_gte_read_data(12);
    corner[1].proj.xy = xport_gte_read_data(13);
    corner[0].proj.depth = xport_gte_read_data(17);
    corner[1].proj.depth = xport_gte_read_data(18);
    corner[2].proj.depth = xport_gte_read_data(19);
    depth = corner[0].proj.depth + corner[1].proj.depth;
    limit = render_view.far_distance;
    result = (sint32)(depth << 2) < (sint32)limit;
    if (result == 0)
        goto done;
    val = render_view.object_depth_bias;
    scale = render_view.fog_scale;
    depth <<= 1;
    val += (uint32)((sint32)depth >> 2);
    bucket = (sint32)(val * scale) >> 19;
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
    orientation = 0;
    if (kind == 8)
        desc = ((const BillboardNode *)this)->tex != 0 ? ((const BillboardNode *)this)->tex : NULL;
    else
    {
        desc = render_view.material_override;
        if (desc == NULL)
        {
            val = spr->node.state_flags;
            desc = spr->tex;
            orientation = (val & 0x180) >> 7;
        }
    }
    packet = render_commands.packet_offset;
    tex = desc->mapping.uv_rect;
    uv0 = tex & 0xFFFF;
    uv3 = tex >> 16;
    tex = desc->mapping.page_clut;
    flags = desc->flags;
    uv0 |= tex & 0xFFFF0000;
    uv1 = (uv0 & 0xFF00) | (uv3 & 0xFF) | (tex << 16);
    if ((flags & 0x80) != 0)
        uv1 |= (flags & 0x60) << 16;
    uv2 = (uv3 & 0xFF00) | (uv0 & 0xFF);
    if (kind == 8 || (spr->node.state_flags & 1) != 0)
    {
        xport_gte_write_data(6, desc->mapping.color.rgbc);
        xport_gte_execute(0x780010);
        poly->rgbc.rgbc = xport_gte_read_data(22);
    }
    else
    {
        work_matrix = &render_view.matrix_cursor->next->matrix;
        x = spr->pos.x;
        y = spr->pos.y;
        z = spr->pos.z;
        work_matrix->t[0] = x;
        work_matrix->t[1] = y;
        work_matrix->t[2] = z;
        if ((game_flow.transition_flags & 0x201) != 1)
        {
            width = spr->extent[0];
            radius = spr->extent[1];
            if (radius < width)
                radius = width;
            const MATRIX *parent_matrix = render_view.matrix_cursor->prev != 0 ? &render_view.matrix_cursor->prev->matrix : 0;
            SceneLighting::SetLightingBounds(parent_matrix->t, radius);
            SceneLighting::TransformObjectLights(0x80000000);
            val = game_flow.transition_flags;
            work_matrix = &render_view.matrix_cursor->next->matrix;
            game_flow.transition_flags = val | 1;
            work_matrix->t[2] = 0;
            work_matrix->t[1] = 0;
            work_matrix->t[0] = 0;
        }
        work_matrix = &render_view.matrix_cursor->next->matrix;
        n.vx = work_matrix->t[0] >> 5;
        n.vy = work_matrix->t[1] >> 5;
        n.vz = work_matrix->t[2] >> 5;
        xport_gte_write_data(6, desc->mapping.color.rgbc);
        SceneLighting::ApplyVertexLights(&n, (desc->flags >> 24) | 0x80000000);
        PushMatrix();
        SetRotMatrix(&scene_lighting.inverse_matrix);
        xport_gte_write_data(0, ((uint16)render_view.planes[0].n.vx | ((uint32)render_view.planes[0].n.vy << 16)));
        xport_gte_write_data(1, ((uint16)render_view.planes[0].n.vz | ((uint32)render_view.planes[0].n.pad << 16)));
        xport_gte_execute(0x486012);
        render_view.transform_result.vx = xport_gte_read_data(25);
        render_view.transform_result.vy = xport_gte_read_data(26);
        render_view.transform_result.vz = xport_gte_read_data(27);
        x = render_view.transform_result.vx;
        y = render_view.transform_result.vy;
        z = render_view.transform_result.vz;
        n.vx = x;
        n.vy = y;
        n.vz = z;
        PopMatrix();
        gte_ldv0(&n);
        xport_gte_execute(0xE80413);
        poly->rgbc.rgbc = xport_gte_read_data(22);
    }
    val = desc->mapping.color.code;
    poly->p[0].uv_attr = uv0;
    poly->p[1].uv_attr = uv1;
    poly->p[2].uv_attr = uv2;
    poly->p[3].uv_attr = uv3;
    poly->rgbc.code = (val & 3) | 0x2C;
    switch (orientation)
    {
        case 0:
            poly->p[0].xy.xy = corner[0].proj.xy;
            poly->p[1].xy.xy = (corner[0].proj.xy & 0xFFFF0000) | corner[1].proj.coords.x;
            poly->p[2].xy.xy = (corner[1].proj.xy & 0xFFFF0000) | corner[0].proj.coords.x;
            poly->p[3].xy.xy = corner[1].proj.xy;
            break;
        case 1:
            poly->p[0].xy.xy = (corner[1].proj.xy & 0xFFFF0000) | corner[0].proj.coords.x;
            poly->p[1].xy.xy = corner[0].proj.xy;
            poly->p[2].xy.xy = corner[1].proj.xy;
            poly->p[3].xy.xy = (corner[0].proj.xy & 0xFFFF0000) | corner[1].proj.coords.x;
            break;
        case 2:
            poly->p[0].xy.xy = corner[1].proj.xy;
            poly->p[1].xy.xy = (corner[1].proj.xy & 0xFFFF0000) | corner[0].proj.coords.x;
            poly->p[2].xy.xy = (corner[0].proj.xy & 0xFFFF0000) | corner[1].proj.coords.x;
            poly->p[3].xy.xy = corner[0].proj.xy;
            break;
        case 3:
            poly->p[0].xy.xy = (corner[0].proj.xy & 0xFFFF0000) | corner[1].proj.coords.x;
            poly->p[1].xy.xy = corner[1].proj.xy;
            poly->p[2].xy.xy = corner[0].proj.xy;
            poly->p[3].xy.xy = (corner[1].proj.xy & 0xFFFF0000) | corner[0].proj.coords.x;
            break;
    }
    result = packet + sizeof(*poly);
    render_commands.packet_offset = result;
done:
    return;
}

void SceneNode::RelocateResourceLinks(const ModelDecodeLayout *layout, const MaterialCatalog *resource, const SpriteRecord *default_spr)
{
    uint8 *base = layout->storage;
    const uint8 *file_data = layout->input;

    uint32 offset;
    uint32 *pointer;
    uint32 remaining;
    uint32 val;
    uint32 idx;
    uint32 kind_val;
    SceneNode *child;

    FUNCTION_MARKER(0x800683DC, "SLUS_005.78");
    const ModelStorageRecord *rec = 0;
    uint32 relative = (uintptr_t)this - (uintptr_t)base;
    for (uint32 rec_idx = 0; rec_idx < layout->rec_count; ++rec_idx)
    {
        const ModelStorageRecord *candidate = layout->recs + rec_idx;
        if (candidate->storage_offset == relative && candidate->kind < ModelStorageRoot)
        {
            rec = candidate;
            break;
        }
    }

    const uint8 *input = file_data + rec->src_offset;
    uint32 rec_bytes = input[12] == 5 || input[12] == 7 ? 48 : (input[12] == 6 ? 36 : 16);

    union
    {
        EncodedSceneNode node;
        EncodedMeshPartNode part;
        EncodedSphereNode sphere;
        EncodedSpriteRecord spr;
    } encoded;

    memcpy(&encoded, input, rec_bytes);
    kind = encoded.node.kind;
    flags = encoded.node.flags;
    state_flags = encoded.node.state_flags;
    links.src_registry_word = encoded.node.registry_offset;
    links.first_child = 0;
    links.sibling_next = 0;
    if (kind == 5)
    {
        MeshPartNode *part = (MeshPartNode *)this;
        part->pos.x = encoded.part.pos.x;
        part->pos.y = encoded.part.pos.y;
        part->pos.z = encoded.part.pos.z;
        part->animation_binding = encoded.part.animation_binding;
        part->span.first_vertex = encoded.part.span.first_vertex;
        part->span.vertex_count = encoded.part.span.vertex_count;
        part->span.first_polygon = encoded.part.span.first_polygon;
        part->span.polygon_count = encoded.part.span.polygon_count;
        part->rotation[0] = encoded.part.rotation.w;
        part->rotation[1] = encoded.part.rotation.x;
        part->rotation[2] = encoded.part.rotation.y;
        part->rotation[3] = encoded.part.rotation.z;
    }
    else if (kind == 6)
    {
        SphereNode *sphere = (SphereNode *)this;
        sphere->animation_binding = encoded.sphere.animation_binding;
        sphere->pos.x = encoded.sphere.pos.x;
        sphere->pos.y = encoded.sphere.pos.y;
        sphere->pos.z = encoded.sphere.pos.z;
        sphere->radius = encoded.sphere.radius;
    }
    else if (kind == 7)
    {
        SpriteRecord *spr = (SpriteRecord *)this;
        spr->animation_entries = 0;
        spr->extent_track = 0;
        spr->animation_binding = encoded.spr.animation_binding;
        spr->pos.x = encoded.spr.pos.x;
        spr->pos.y = encoded.spr.pos.y;
        spr->pos.z = encoded.spr.pos.z;
        for (uint32 component = 0; component < 2; ++component)
        {
            spr->extent[component] = encoded.spr.extent[component];
            spr->angles[component] = encoded.spr.angles[component];
        }
    }
    offset = encoded.node.child_offset;
    if (offset != 0)
    {
        uint32 mapping_idx = 0;
        while (mapping_idx < layout->rec_count && !(layout->recs[mapping_idx].src_offset == offset && layout->recs[mapping_idx].kind < ModelStorageRoot))
            ++mapping_idx;
        links.first_child = (SceneNode *)(layout->storage + layout->recs[mapping_idx].storage_offset);
    }
    offset = encoded.node.sibling_offset;
    if (offset != 0)
    {
        uint32 mapping_idx = 0;
        while (mapping_idx < layout->rec_count && !(layout->recs[mapping_idx].src_offset == offset && layout->recs[mapping_idx].kind < ModelStorageRoot))
            ++mapping_idx;
        links.sibling_next = (SceneNode *)(layout->storage + layout->recs[mapping_idx].storage_offset);
    }
    if (kind == 7)
    {
        SpriteRecord *spr = (SpriteRecord *)this;
        if (spr != default_spr)
        {
            idx = encoded.spr.tex_idx;
            spr->tex = resource->texs + idx;
        }
        offset = encoded.spr.animation_offset;
        if (offset != 0)
        {
            uint32 mapping_idx = 0;
            while (mapping_idx < layout->rec_count && !(layout->recs[mapping_idx].src_offset == offset - 4 && layout->recs[mapping_idx].kind == ModelStorageMaterialAnimations))
                ++mapping_idx;
            pointer = (uint32 *)(layout->storage + layout->recs[mapping_idx].storage_offset) + 1;
            spr->animation_entries = pointer;
            remaining = (uint8)((const MaterialAnimationListHeader *)pointer - 1)->count;
            if (remaining != 0)
            {
                do
                {
                    uint32 *entry = pointer;
                    val = *entry;
                    if (val != 0xFFFFFFFF)
                    {
                        idx = (val >> 1) & 0x1FF;
                        kind_val = resource->materials[idx].kind;
                        idx += material_registry.count;
                        if (kind_val == 0)
                            *entry = 2 * idx;
                        else if (kind_val == 2)
                            *entry = 2 * idx | (val & 0x3FC00) | (val & 0x3FC0000) | (val & 0x3C000000);
                    }
                    ++pointer;
                    --remaining;
                } while (remaining != 0);
            }
        }
    }
    child = links.first_child;
    while (child != 0)
    {
        SceneNode *child_node = child;
        child_node->RelocateResourceLinks(layout, resource, default_spr);
        child = child_node->links.sibling_next;
    }
}
