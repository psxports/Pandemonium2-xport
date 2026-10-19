#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "game.h"
#include "original_defaults.h"

union NodeTransformScratch
{
    MATRIX matrix;
    uint32 minimum[3];
};

uint32 AnimatedObject::UpdateObjectBounds()
{
    CollisionSphere *spheres = this->spheres;
    CollisionSphere *sphere;
    uint32 flags;
    const MATRIX *matrix;
    uint32 x;
    uint32 y;
    uint32 z;
    uint32 value;
    uint32 dx;
    uint32 dy;
    uint32 dz;
    uint32 index;
    uint32 result;
    FUNCTION_MARKER(0x80012298, "SLUS_005.78");
    flags = this->flags;
    if ((flags & 0x20) != 0 && (sint8)this->metadata126[0] != 0)
    {
        if ((flags & 0x20200) != 0x200)
        {
            spheres[3].radius = 0xFFFFFFFF;
            spheres[2].radius = 0xFFFFFFFF;
            spheres[1].radius = 0xFFFFFFFF;
            spheres[0].radius = 0xFFFFFFFF;
            return 0xFFFFFFFF;
        }
        matrix = &render_view.matrix_cursor->matrix;
        x = this->world_position[0];
        value = (uint32)matrix->t[0];
        dx = value - x;
        value = (uint32)matrix->t[1];
        y = this->world_position[1];
        dy = value - y;
        value = (uint32)matrix->t[2];
        z = this->world_position[2];
        dz = value - z;
        dy += this->metadata_words72[1];
        this->world_position[0] = this->world_position[0] + dx;
        this->world_position[1] = this->world_position[1] + dy;
        this->world_position[2] = this->world_position[2] + dz;
        sphere = spheres;
        index = 0;
        for (;;)
        {
            result = sphere->radius;
            ++index;
            if ((sint32)result <= 0)
                return result;
            sphere->position[0] = sphere->position[0] + dx;
            sphere->position[1] = sphere->position[1] + dy;
            sphere->position[2] = sphere->position[2] + dz;
            result = index < 4;
            ++sphere;
            if (result == 0)
                return result;
        }
    }
    matrix = &render_view.matrix_cursor->matrix;
    x = spheres[0].position[0];
    value = (uint32)matrix->t[0];
    this->world_position[0] = x + value;
    y = spheres[0].position[1];
    value = (uint32)matrix->t[1];
    this->world_position[1] = y + value;
    z = spheres[0].position[2];
    value = (uint32)matrix->t[2];
    result = z + value;
    this->world_position[2] = result;
    return result;
}

void SceneNode::DrawChildren(AnimatedObject *owner)
{
    SceneNode *child;
    MeshPartNode *part;
    SpriteRecord *sprite;
    SphereNode *sphere_node;
    CollisionSphere *sphere;
    SVECTOR velocity;
    MATRIX first_matrix = {0};
    NodeTransformScratch scratch;
    const MATRIX *root_matrix = &render_view.matrix_root.matrix;
    MATRIX *current_matrix;
    const MATRIX *parent_matrix;
    const MATRIX *rotation_matrix;
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
    uint32 value;
    uint32 packed;
    uint32 rate;
    uint32 phase;
    uint32 alpha;
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
    uint32 maximum[3];
    uint32 index;
    uint32 words[4];
    sint32 qw;
    sint32 qx;
    sint32 qy;
    sint32 qz;
    sint32 cosine;
    sint32 sine;
    sint32 coefficient;
    sint64 product;
    FUNCTION_MARKER(0x80012424, "SLUS_005.78");
    matrix = render_view.matrix_cursor;
    current_matrix = &matrix->matrix;
    parent_matrix = (matrix->prev != 0 ? &matrix->prev->matrix : 0);
    // GTE rotation register 4 ignores the upper padding halfword
    xport_store_le16((uint8 *)&first_matrix + 18, 0);
    xport_store_le16((uint8 *)&scratch.matrix + 18, 0);
    child = links.first_child;
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
    while (child != 0)
    {
        if ((owner->node.flags & 0x40) != 0)
            owner->SampleNodeAnimation(child);
        matrix = render_view.matrix_cursor;
        current_matrix = &matrix->matrix;
        parent_matrix = (matrix->prev != 0 ? &matrix->prev->matrix : 0);
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
        kind = child->kind;
        if (kind == 5)
        {
            part = (MeshPartNode *)child;
            if ((sint8)render_view.special_transform != 0)
            {
                if (owner->velocity[0] != 0)
                {
                    ApplyRotMatrixLV((const VECTOR *)part->position, (VECTOR *)current_matrix->t);
                    matrix = render_view.matrix_cursor;
                    current_matrix = &matrix->matrix;
                    parent_matrix = (matrix->prev != 0 ? &matrix->prev->matrix : 0);
                    x = (uint32)current_matrix->t[0];
                    px = (uint32)parent_matrix->t[0];
                    y = (uint32)current_matrix->t[1];
                    py = (uint32)parent_matrix->t[1];
                    z = (uint32)current_matrix->t[2];
                    pz = (uint32)parent_matrix->t[2];
                    current_matrix->t[0] = (sint32)(x + px);
                    current_matrix->t[1] = (sint32)(y + py);
                    current_matrix->t[2] = (sint32)(z + pz);
                    x = (uint32)current_matrix->t[0];
                    y = (uint32)current_matrix->t[1];
                    z = (uint32)current_matrix->t[2];
                    part->position[0] = x;
                    part->position[1] = y;
                    part->position[2] = z;
                    x = part->position[0];
                    px = owner->world_position[0];
                    velocity.vx = (sint16)(x - px);
                    y = part->position[1];
                    py = owner->world_position[1];
                    velocity.vy = (sint16)(y - py);
                    z = part->position[2];
                    pz = owner->world_position[2];
                    velocity.vz = (sint16)(z - pz);
                    value = random_generator.Next();
                    velocity.vx = (sint16)(2730 - value % 5460);
                    value = random_generator.Next();
                    coefficient = velocity.vy;
                    velocity.vz = (sint16)(2730 - value % 5460);
                    if (coefficient < 8192)
                        velocity.vy = 8192;
                    x = xport_load_le32(&velocity);
                    xport_store_le32(&part->velocity, x);
                    part->velocity.vz = velocity.vz;
                    alpha = random_generator.Next();
                    beta = random_generator.Next();
                    gamma = random_generator.Next();
                    part->rotation[3] = (sint16)((alpha & 31) | ((beta & 31) << 5) | ((gamma & 31) << 10));
                }
                else
                {
                    qx = part->rotation[0];
                    x = part->position[0];
                    qy = part->rotation[1];
                    y = part->position[1];
                    qz = part->rotation[2];
                    z = part->position[2];
                    part->position[0] = x + (uint32)qx;
                    part->position[1] = y + (uint32)qy;
                    part->position[2] = z + (uint32)qz;
                    part->rotation[1] = (sint16)((uint16)part->rotation[1] - 1228);
                    x = part->position[0];
                    y = part->position[1];
                    z = part->position[2];
                    current_matrix->t[0] = (sint32)(x);
                    current_matrix->t[1] = (sint32)(y);
                    current_matrix->t[2] = (sint32)(z);
                }
                value = owner->path_motion_scale;
                packed = (uint32)(sint32)part->rotation[3];
                rate = (15 - value) << 4;
                phase = (packed & 31) * rate;
                cosine = SinQ14((sint16)(1024 - phase)) >> 2;
                sine = SinQ14((sint16)phase) >> 2;
                rotation_x.m[1][1] = (sint16)((uint16)cosine);
                rotation_x.m[2][1] = (sint16)((uint16)sine);
                rotation_x.m[1][2] = (sint16)((uint16)(0 - (uint32)sine));
                rotation_x.m[2][2] = (sint16)((uint16)cosine);
                MulMatrix0((MATRIX *)root_matrix, &rotation_x, &first_matrix);
                phase = ((packed >> 5) & 31) * rate;
                cosine = SinQ14((sint16)(1024 - phase)) >> 2;
                sine = SinQ14((sint16)phase) >> 2;
                phase = ((packed >> 10) & 31) * rate;
                rotation_y.m[0][0] = (sint16)((uint16)cosine);
                rotation_y.m[2][0] = (sint16)((uint16)(0 - (uint32)sine));
                rotation_y.m[0][2] = (sint16)((uint16)sine);
                rotation_y.m[2][2] = (sint16)((uint16)cosine);
                if (phase != 0)
                {
                    MulMatrix0(&first_matrix, &rotation_y, &scratch.matrix);
                    cosine = SinQ14((sint16)(1024 - phase)) >> 2;
                    sine = SinQ14((sint16)phase) >> 2;
                    rotation_z.m[0][0] = (sint16)((uint16)cosine);
                    rotation_z.m[1][0] = (sint16)((uint16)sine);
                    rotation_z.m[0][1] = (sint16)((uint16)(0 - (uint32)sine));
                    rotation_z.m[1][1] = (sint16)((uint16)cosine);
                    left = &rotation_z;
                    right = &scratch.matrix;
                }
                else
                {
                    left = &first_matrix;
                    right = &rotation_y;
                }
                matrix = render_view.matrix_cursor;
                current_matrix = &matrix->matrix;
                parent_matrix = (matrix->prev != 0 ? &matrix->prev->matrix : 0);
                MulMatrix0(left, right, current_matrix);
                rotation_matrix = current_matrix;
            }
            else
            {
                ApplyRotMatrixLV((const VECTOR *)part->position, (VECTOR *)current_matrix->t);
                matrix = render_view.matrix_cursor;
                current_matrix = &matrix->matrix;
                parent_matrix = (matrix->prev != 0 ? &matrix->prev->matrix : 0);
                x = (uint32)current_matrix->t[0];
                px = (uint32)parent_matrix->t[0];
                y = (uint32)current_matrix->t[1];
                py = (uint32)parent_matrix->t[1];
                z = (uint32)current_matrix->t[2];
                pz = (uint32)parent_matrix->t[2];
                current_matrix->t[0] = (sint32)(x + px);
                current_matrix->t[1] = (sint32)(y + py);
                current_matrix->t[2] = (sint32)(z + pz);
                qw = part->rotation[0];
                if (qw == 0x4000)
                {
                    for (index = 0; index < 4; ++index)
                        words[index] = xport_load_le32((const uint8 *)parent_matrix + 4 * index);
                    for (index = 0; index < 4; ++index)
                        xport_store_le32((uint8 *)current_matrix + 4 * index, words[index]);
                    current_matrix->m[2][2] = (sint16)((uint16)parent_matrix->m[2][2]);
                    goto node_rotation_ready;
                }
                qx = part->rotation[1];
                qy = part->rotation[2];
                qz = part->rotation[3];
                xx = (uint32)(qx * qx);
                yy = (uint32)(qy * qy);
                zz = (uint32)(qz * qz);
                xy = (uint32)(qx * qy);
                xz = (uint32)(qx * qz);
                yz = (uint32)(qy * qz);
                wz = (uint32)(qw * qz);
                wy = (uint32)(qw * qy);
                current_matrix->m[0][0] = (sint16)((uint32)((sint32)(0x8000000 - (yy + zz)) >> 15));
                current_matrix->m[1][1] = (sint16)((uint32)((sint32)(0x8000000 - (xx + zz)) >> 15));
                current_matrix->m[2][2] = (sint16)((uint32)((sint32)(0x8000000 - (xx + yy)) >> 15));
                wx = (uint32)(qw * qx);
                current_matrix->m[1][0] = (sint16)((uint32)((sint32)(xy - wz) >> 15));
                current_matrix->m[0][1] = (sint16)((uint32)((sint32)(xy + wz) >> 15));
                current_matrix->m[2][0] = (sint16)((uint32)((sint32)(xz + wy) >> 15));
                current_matrix->m[0][2] = (sint16)((uint32)((sint32)(xz - wy) >> 15));
                current_matrix->m[2][1] = (sint16)((uint32)((sint32)(yz - wx) >> 15));
                current_matrix->m[1][2] = (sint16)((uint32)((sint32)(yz + wx) >> 15));
                MulRotMatrix(current_matrix);
                rotation_matrix = current_matrix;
            }
            x = xport_load_le32((const uint8 *)rotation_matrix);
            y = xport_load_le32((const uint8 *)rotation_matrix + 4);
            xport_gte_write_control(0, x);
            xport_gte_write_control(1, y);
            x = xport_load_le32((const uint8 *)rotation_matrix + 8);
            y = xport_load_le32((const uint8 *)rotation_matrix + 12);
            z = xport_load_le32((const uint8 *)rotation_matrix + 16);
            xport_gte_write_control(2, x);
            xport_gte_write_control(3, y);
            xport_gte_write_control(4, z);
        node_rotation_ready:
            if ((owner->flags & 0x200) == 0)
                owner->DrawGeometry(&part->span);
        }
        else if (kind == 7)
        {
            sprite = (SpriteRecord *)child;
            if ((sint8)render_view.special_transform != 0)
            {
                x = (uint32)parent_matrix->t[0];
                y = (uint32)parent_matrix->t[1];
                z = (uint32)parent_matrix->t[2];
                current_matrix->t[0] = (sint32)(x);
                current_matrix->t[1] = (sint32)(y);
                current_matrix->t[2] = (sint32)(z);
            }
            else
            {
                ApplyRotMatrixLV((const VECTOR *)sprite->position, (VECTOR *)current_matrix->t);
                matrix = render_view.matrix_cursor;
                current_matrix = &matrix->matrix;
                parent_matrix = (matrix->prev != 0 ? &matrix->prev->matrix : 0);
                x = (uint32)current_matrix->t[0];
                px = (uint32)parent_matrix->t[0];
                y = (uint32)current_matrix->t[1];
                py = (uint32)parent_matrix->t[1];
                z = (uint32)current_matrix->t[2];
                pz = (uint32)parent_matrix->t[2];
                current_matrix->t[0] = (sint32)(x + px);
                current_matrix->t[1] = (sint32)(y + py);
                current_matrix->t[2] = (sint32)(z + pz);
            }
            if ((owner->flags & 0x20000) != 0 && (child->state_flags & 0x200) == 0)
            {
                value = child->flags;
                child->flags = value & 0xFE;
                ResetMaterialAnimationList(sprite->animation_entries);
            }
            if ((child->flags & 1) != 0)
                goto visit_children;
            if ((child->state_flags & 0x200) == 0)
            {
                matrix = render_view.matrix_cursor;
                current_matrix = &matrix->matrix;
                parent_matrix = (matrix->prev != 0 ? &matrix->prev->matrix : 0);
                for (index = 0; index < 4; ++index)
                    words[index] = xport_load_le32((const uint8 *)parent_matrix + 4 * index);
                for (index = 0; index < 4; ++index)
                    xport_store_le32((uint8 *)current_matrix + 4 * index, words[index]);
                current_matrix->m[2][2] = (sint16)((uint16)parent_matrix->m[2][2]);
            }
            if ((owner->flags & 0x200) == 0)
            {
                if (((uint32)UpdateMaterialAnimationList(sprite->animation_entries) << 24) != 0)
                {
                    render_view.object_depth_bias = render_view.object_depth_bias - 256;
                    sprite->Draw();
                    render_view.object_depth_bias = render_view.object_depth_bias + 256;
                }
                else
                    child->flags = child->flags | 1;
            }
        }
        else if (kind == 6)
        {
            sphere_node = (SphereNode *)child;
            if ((sint8)owner->metadata126[1] != 0)
                goto next_node;
            ApplyRotMatrixLV((const VECTOR *)sphere_node->position, (VECTOR *)current_matrix->t);
            matrix = render_view.matrix_cursor;
            current_matrix = &matrix->matrix;
            parent_matrix = (matrix->prev != 0 ? &matrix->prev->matrix : 0);
            x = (uint32)current_matrix->t[0];
            px = (uint32)parent_matrix->t[0];
            y = (uint32)current_matrix->t[1];
            py = (uint32)parent_matrix->t[1];
            z = (uint32)current_matrix->t[2];
            pz = (uint32)parent_matrix->t[2];
            current_matrix->t[0] = (sint32)(x + px);
            current_matrix->t[1] = (sint32)(y + py);
            current_matrix->t[2] = (sint32)(z + pz);
            sphere = (CollisionSphere *)(render_view.sphere_output);
            render_view.sphere_output = (CollisionSphere *)((uint8 *)sphere + sizeof(CollisionSphere));
            x = (uint32)current_matrix->t[0];
            y = (uint32)current_matrix->t[1];
            z = (uint32)current_matrix->t[2];
            sphere->position[0] = x;
            sphere->position[1] = y;
            sphere->position[2] = z;
            radius = sphere_node->radius & 0xFFFFFFF8;
            sphere->radius = radius;
            scale = render_view.object_scale;
            if (scale != 0)
            {
                product = (sint64)(sint32)radius * (sint32)scale;
                sphere->radius = (uint32)((uint64)product >> 12);
            }
            value = sphere_node->node.state_flags >> 8;
            radius = sphere->radius;
            matrix = render_view.matrix_cursor;
            current_matrix = &matrix->matrix;
            parent_matrix = (matrix->prev != 0 ? &matrix->prev->matrix : 0);
            radius |= value & 7;
            sphere->radius = radius;
            for (index = 0; index < 3; ++index)
            {
                value = (uint32)current_matrix->t[index];
                radius = index == 0 ? radius : sphere->radius;
                scratch.minimum[index] = value - radius;
            }
            for (index = 0; index < 3; ++index)
            {
                value = (uint32)current_matrix->t[index];
                radius = sphere->radius;
                maximum[index] = value + radius;
            }
            if (sphere == owner->spheres)
            {
                x = scratch.minimum[0];
                y = scratch.minimum[1];
                z = scratch.minimum[2];
                owner->world_position[0] = x;
                owner->world_position[1] = y;
                owner->world_position[2] = z;
                x = maximum[0];
                y = maximum[1];
                z = maximum[2];
                owner->metadata_words72[0] = x;
                owner->metadata_words72[1] = y;
                owner->metadata_words72[2] = z;
            }
            else
            {
                for (index = 0; index < 3; ++index)
                {
                    x = scratch.minimum[index];
                    y = owner->world_position[index];
                    if ((sint32)x < (sint32)y)
                        owner->world_position[index] = x;
                    x = maximum[index];
                    y = owner->metadata_words72[index];
                    if ((sint32)y < (sint32)x)
                        owner->metadata_words72[index] = x;
                }
            }
        }
    visit_children:
        child->DrawChildren(owner);
    next_node:
        child = child->links.sibling_next;
    }
    render_view.matrix_cursor = render_view.matrix_cursor->prev;
}

void ObjectLinks::UpdateGraph(AnimatedObject *parent, uint32 mode)
{
    FUNCTION_MARKER(0x800140F8, "SLUS_005.78");
    AnimatedObject *parent_view = (AnimatedObject *)(parent);
    const uint32 transpose_offsets[9] = {0, 6, 12, 2, 8, 14, 4, 10, 16};

    union
    {
        VECTOR vector;
        uint32 words[4];
    } delta, input, before, relative;

    union
    {
        MATRIX matrix;
        VECTOR vector;
        uint32 words[8];
    } tmp;

    MATRIX transposed;
    SceneNode *node, *next_node;
    uint32 base_mode, work, category, old_active, flags;
    const MATRIX *root_matrix = &render_view.matrix_root.matrix;
    const MATRIX *stack_source;
    MATRIX *current_matrix;
    const MATRIX *parent_matrix;
    RenderMatrixSlot *matrix;
    CollisionTransform *collision_cache;
    const PathDefinition *attachment;
    uint32 target, follow;
    AnimatedObject *player, *actor, *linked_child;
    SceneNode *child_reference;
    MeshPartNode *part;
    SpriteRecord *sprite;
    SphereNode *sphere_node;
    ScenePath *path;
    MATRIX *path_matrix;
    PathDefinition *definition;
    const PathDefinition *player_definition, *trigger_definition;
    const ScenePath *trigger_path;
    PathTransform *path_transform;
    const MATRIX *definition_matrix;
    SceneLight *light;
    PositionedBillboardNode *billboard;
    uint32 x, y, z, w, i;
    sint32 index, angle, cosine, sine;
    matrix = render_view.matrix_cursor;
    current_matrix = &matrix->matrix;
    parent_matrix = (matrix->prev != 0 ? &matrix->prev->matrix : 0);
    node = first_child;
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
                    ((actor)->*operation.member_handler)();
                else if (operation.void_member_handler)
                    ((actor)->*operation.void_member_handler)();
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
            ApplyNodeAnimation(parent, node);
        matrix = render_view.matrix_cursor;
        current_matrix = &matrix->matrix;
        parent_matrix = (matrix->prev != 0 ? &matrix->prev->matrix : 0);
        stack_source = parent_matrix;
        for (i = 0; i < 5; ++i)
            xport_gte_write_control(i, xport_load_le32((const uint8 *)stack_source + i * 4));

        if (category == 5)
        {
            part = (MeshPartNode *)node;
            ApplyRotMatrixLV((const VECTOR *)part->position, (VECTOR *)current_matrix->t);
            matrix = render_view.matrix_cursor;
            current_matrix = &matrix->matrix;
            parent_matrix = (matrix->prev != 0 ? &matrix->prev->matrix : 0);
            x = (uint32)current_matrix->t[0] + (uint32)parent_matrix->t[0];
            y = (uint32)current_matrix->t[1] + (uint32)parent_matrix->t[1];
            z = (uint32)current_matrix->t[2] + (uint32)parent_matrix->t[2];
            current_matrix->t[0] = (sint32)(x);
            current_matrix->t[1] = (sint32)(y);
            current_matrix->t[2] = (sint32)(z);

            if (part->rotation[0] == 0x4000)
            {
                stack_source = parent_matrix;
                {
                    uint32 c0 = xport_load_le32((const uint8 *)stack_source), c1 = xport_load_le32((const uint8 *)stack_source + 4), c2 = xport_load_le32((const uint8 *)stack_source + 8), c3 = xport_load_le32((const uint8 *)stack_source + 12);
                    xport_store_le32((uint8 *)current_matrix, c0);
                    xport_store_le32((uint8 *)current_matrix + 4, c1);
                    xport_store_le32((uint8 *)current_matrix + 8, c2);
                    xport_store_le32((uint8 *)current_matrix + 12, c3);
                    current_matrix->m[2][2] = (sint16)((uint16)stack_source->m[2][2]);
                }
            }
            else
            {
                {
                    uint32 qw = (uint32)part->rotation[0], qx = (uint32)part->rotation[1], qy = (uint32)part->rotation[2], qz = (uint32)part->rotation[3];
                    uint32 xx = qx * qx, yy = qy * qy, zz = qz * qz, xy = qx * qy, xz = qx * qz, yz = qy * qz, wz = qw * qz, wy = qw * qy, wx = qw * qx;
                    current_matrix->m[0][0] = (sint16)((sint32)(0x08000000u - (yy + zz)) >> 15);
                    current_matrix->m[1][1] = (sint16)((sint32)(0x08000000u - (xx + zz)) >> 15);
                    current_matrix->m[2][2] = (sint16)((sint32)(0x08000000u - (xx + yy)) >> 15);
                    current_matrix->m[1][0] = (sint16)((sint32)(xy - wz) >> 15);
                    current_matrix->m[0][1] = (sint16)((sint32)(xy + wz) >> 15);
                    current_matrix->m[2][0] = (sint16)((sint32)(xz + wy) >> 15);
                    current_matrix->m[0][2] = (sint16)((sint32)(xz - wy) >> 15);
                    current_matrix->m[2][1] = (sint16)((sint32)(yz - wx) >> 15);
                    current_matrix->m[1][2] = (sint16)((sint32)(yz + wx) >> 15);
                }
                MulRotMatrix(current_matrix);
                stack_source = current_matrix;
                for (i = 0; i < 5; ++i)
                    xport_gte_write_control(i, xport_load_le32((const uint8 *)stack_source + i * 4));
            }
            index = part->animation_binding.tracks[2];
            if (index != -1)
            {
                if (index < 0)
                {
                    index = -(index + 2);
                    work &= 0xFFFFFFEF;
                }
                if ((work & 0xCC) != 0)
                {
                    collision_cache = parent_view->collision_transforms;
                    collision_cache = ((CollisionTransform *)(collision_cache)) + (uint32)index;

                    if ((work & 0x8C) != 0)
                    {
                        collision_cache->fields.motion[2] = 0;
                        collision_cache->fields.motion[1] = 0;
                        collision_cache->fields.motion[0] = 0;
                    }
                    else
                    {
                        matrix = render_view.matrix_cursor;
                        current_matrix = &matrix->matrix;
                        parent_matrix = (matrix->prev != 0 ? &matrix->prev->matrix : 0);
                        collision_cache->fields.motion[0] = (uint32)current_matrix->t[0] - collision_cache->fields.position[0];
                        collision_cache->fields.motion[1] = (uint32)current_matrix->t[1] - collision_cache->fields.position[1];
                        collision_cache->fields.motion[2] = (uint32)current_matrix->t[2] - collision_cache->fields.position[2];
                    }
                    matrix = render_view.matrix_cursor;
                    current_matrix = &matrix->matrix;
                    parent_matrix = (matrix->prev != 0 ? &matrix->prev->matrix : 0);
                    {
                        uint32 c0 = (uint32)current_matrix->t[0], c1 = (uint32)current_matrix->t[1], c2 = (uint32)current_matrix->t[2];
                        collision_cache->fields.position[0] = c0;
                        collision_cache->fields.position[1] = c1;
                        collision_cache->fields.position[2] = c2;
                    }
                    matrix = render_view.matrix_cursor;
                    current_matrix = &matrix->matrix;
                    parent_matrix = (matrix->prev != 0 ? &matrix->prev->matrix : 0);
                    for (i = 0; i < 9; ++i)
                        collision_cache->fields.rotation[i] = current_matrix->m[i % 3][i / 3];

                    if (parent != 0)
                    {
                        if ((parent_view->flags & 0x200) != 0)
                            node->flags = node->flags | 4;
                        else if ((node->flags & 4) != 0)
                        {
                            if ((work & 2) == 0)
                            {
                                collision_cache->fields.motion[2] = 0;
                                collision_cache->fields.motion[1] = 0;
                                collision_cache->fields.motion[0] = 0;
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
                        uint32 c0 = actor->position[0], c1 = actor->position[1], c2 = actor->position[2];
                        current_matrix->t[0] = (sint32)(c0);
                        current_matrix->t[1] = (sint32)(c1);
                        current_matrix->t[2] = (sint32)(c2);
                    }
                }
                else
                {
                    attachment = (actor->path_definition);
                    if (attachment != 0 && ((attachment)->transform) != 0)
                    {
                        {
                            uint32 c0 = actor->position[0], c1 = actor->position[1], c2 = actor->position[2];
                            input.words[0] = c0;
                            input.words[1] = c1;
                            input.words[2] = c2;
                        }
                        PushMatrix();
                        definition_matrix = (const MATRIX *)((((const PathDefinition *)((const PathDefinition *)((actor->path_definition))))->transform) != 0 ? &(((const PathDefinition *)((const PathDefinition *)((actor->path_definition))))->transform)->matrix : (MATRIX *)0);
                        for (i = 0; i < 5; ++i)
                            xport_gte_write_control(i, xport_load_le32((const uint8 *)definition_matrix + i * 4));
                        ApplyRotMatrixLV(&input.vector, (VECTOR *)(&render_view.matrix_cursor->matrix)->t);
                        PopMatrix();
                        matrix = render_view.matrix_cursor;
                        current_matrix = &matrix->matrix;
                        parent_matrix = (matrix->prev != 0 ? &matrix->prev->matrix : 0);
                        for (i = 0; i < 3; ++i)
                        {
                            definition_matrix = (const MATRIX *)((((const PathDefinition *)((const PathDefinition *)((actor->path_definition))))->transform) != 0 ? &(((const PathDefinition *)((const PathDefinition *)((actor->path_definition))))->transform)->matrix : (MATRIX *)0);
                            current_matrix->t[i] = (sint32)((uint32)current_matrix->t[i] + (uint32)definition_matrix->t[i]);
                        }
                    }
                    else
                    {
                        matrix = render_view.matrix_cursor;
                        current_matrix = &matrix->matrix;
                        parent_matrix = (matrix->prev != 0 ? &matrix->prev->matrix : 0);
                        {
                            uint32 c0 = actor->position[0], c1 = actor->position[1], c2 = actor->position[2];
                            current_matrix->t[0] = (sint32)(c0);
                            current_matrix->t[1] = (sint32)(c1);
                            current_matrix->t[2] = (sint32)(c2);
                        }
                    }
                }
                matrix = render_view.matrix_cursor;
                current_matrix = &matrix->matrix;
                parent_matrix = (matrix->prev != 0 ? &matrix->prev->matrix : 0);
                {
                    uint32 qw = (uint32)actor->orientation[0], qx = (uint32)actor->orientation[1], qy = (uint32)actor->orientation[2], qz = (uint32)actor->orientation[3];
                    uint32 xx = qx * qx, yy = qy * qy, zz = qz * qz, xy = qx * qy, xz = qx * qz, yz = qy * qz, wz = qw * qz, wy = qw * qy, wx = qw * qx;
                    current_matrix->m[0][0] = (sint16)((sint32)(0x08000000u - (yy + zz)) >> 15);
                    current_matrix->m[1][1] = (sint16)((sint32)(0x08000000u - (xx + zz)) >> 15);
                    current_matrix->m[2][2] = (sint16)((sint32)(0x08000000u - (xx + yy)) >> 15);
                    current_matrix->m[1][0] = (sint16)((sint32)(xy - wz) >> 15);
                    current_matrix->m[0][1] = (sint16)((sint32)(xy + wz) >> 15);
                    current_matrix->m[2][0] = (sint16)((sint32)(xz + wy) >> 15);
                    current_matrix->m[0][2] = (sint16)((sint32)(xz - wy) >> 15);
                    current_matrix->m[2][1] = (sint16)((sint32)(yz - wx) >> 15);
                    current_matrix->m[1][2] = (sint16)((sint32)(yz + wx) >> 15);
                }
                for (i = 0; i < 5; ++i)
                    xport_gte_write_control(i, xport_load_le32((const uint8 *)current_matrix + 4 * i));

                for (i = 0; i < 3; ++i)
                    delta.words[i] = (uint32)current_matrix->t[i] - actor->auxiliary_words[4 + i];
                for (i = 0; i < 3; ++i)
                    actor->auxiliary_words[i] = actor->auxiliary_words[i] + delta.words[i];
                for (i = 0; i < 3; ++i)
                    actor->world_position[i] = actor->world_position[i] + delta.words[i];
                child_reference = node->links.first_child;
                linked_child = child_reference != 0 ? (AnimatedObject *)child_reference : (AnimatedObject *)0;
                {
                    uint32 c0 = actor->world_position[0], c1 = actor->world_position[1], c2 = actor->world_position[2];
                    linked_child->world_position[0] = c0;
                    linked_child->world_position[1] = c1;
                    linked_child->world_position[2] = c2;
                }
                matrix = render_view.matrix_cursor;
                current_matrix = &matrix->matrix;
                parent_matrix = (matrix->prev != 0 ? &matrix->prev->matrix : 0);
                for (i = 0; i < 3; ++i)
                    linked_child->spheres[0].position[i] = linked_child->world_position[i] - (uint32)current_matrix->t[i];
                goto cache_actor;
            }
            ApplyRotMatrixLV((const VECTOR *)actor->position, (VECTOR *)current_matrix->t);
            matrix = render_view.matrix_cursor;
            current_matrix = &matrix->matrix;
            parent_matrix = (matrix->prev != 0 ? &matrix->prev->matrix : 0);
            x = (uint32)current_matrix->t[0] + (uint32)parent_matrix->t[0];
            y = (uint32)current_matrix->t[1] + (uint32)parent_matrix->t[1];
            z = (uint32)current_matrix->t[2] + (uint32)parent_matrix->t[2];
            current_matrix->t[0] = (sint32)(x);
            current_matrix->t[1] = (sint32)(y);
            current_matrix->t[2] = (sint32)(z);

            if (actor->orientation[0] == 0x4000)
            {
                stack_source = parent_matrix;
                {
                    uint32 c0 = xport_load_le32((const uint8 *)stack_source), c1 = xport_load_le32((const uint8 *)stack_source + 4), c2 = xport_load_le32((const uint8 *)stack_source + 8), c3 = xport_load_le32((const uint8 *)stack_source + 12);
                    xport_store_le32((uint8 *)current_matrix, c0);
                    xport_store_le32((uint8 *)current_matrix + 4, c1);
                    xport_store_le32((uint8 *)current_matrix + 8, c2);
                    xport_store_le32((uint8 *)current_matrix + 12, c3);
                    current_matrix->m[2][2] = (sint16)((uint16)stack_source->m[2][2]);
                }
            }
            else
            {
                {
                    uint32 qw = (uint32)actor->orientation[0], qx = (uint32)actor->orientation[1], qy = (uint32)actor->orientation[2], qz = (uint32)actor->orientation[3];
                    uint32 xx = qx * qx, yy = qy * qy, zz = qz * qz, xy = qx * qy, xz = qx * qz, yz = qy * qz, wz = qw * qz, wy = qw * qy, wx = qw * qx;
                    current_matrix->m[0][0] = (sint16)((sint32)(0x08000000u - (yy + zz)) >> 15);
                    current_matrix->m[1][1] = (sint16)((sint32)(0x08000000u - (xx + zz)) >> 15);
                    current_matrix->m[2][2] = (sint16)((sint32)(0x08000000u - (xx + yy)) >> 15);
                    current_matrix->m[1][0] = (sint16)((sint32)(xy - wz) >> 15);
                    current_matrix->m[0][1] = (sint16)((sint32)(xy + wz) >> 15);
                    current_matrix->m[2][0] = (sint16)((sint32)(xz + wy) >> 15);
                    current_matrix->m[0][2] = (sint16)((sint32)(xz - wy) >> 15);
                    current_matrix->m[2][1] = (sint16)((sint32)(yz - wx) >> 15);
                    current_matrix->m[1][2] = (sint16)((sint32)(yz + wx) >> 15);
                }
                MulRotMatrix(current_matrix);
                stack_source = current_matrix;
                for (i = 0; i < 5; ++i)
                    xport_gte_write_control(i, xport_load_le32((const uint8 *)stack_source + i * 4));
            }
            if ((work & 0xCC) != 0)
            {
                matrix = render_view.matrix_cursor;
                current_matrix = &matrix->matrix;
                parent_matrix = (matrix->prev != 0 ? &matrix->prev->matrix : 0);
                for (i = 0; i < 3; ++i)
                    actor->world_position[i] = actor->auxiliary_words[i] + (uint32)current_matrix->t[i];
            cache_actor:
                if ((actor->flags & 0x100000) != 0 && actor->mesh_span.polygon_count != 0)
                {
                    collision_cache = actor->collision_transforms;
                    collision_cache = ((CollisionTransform *)(collision_cache));
                    if (actor->mesh_span.first_polygon == collision_cache->fields.first_face)
                    {
                        if ((work & 0x8C) != 0)
                        {
                            collision_cache->fields.motion[2] = 0;
                            collision_cache->fields.motion[1] = 0;
                            collision_cache->fields.motion[0] = 0;
                        }
                        else
                        {
                            matrix = render_view.matrix_cursor;
                            current_matrix = &matrix->matrix;
                            parent_matrix = (matrix->prev != 0 ? &matrix->prev->matrix : 0);
                            collision_cache->fields.motion[0] = (uint32)current_matrix->t[0] - collision_cache->fields.position[0];
                            collision_cache->fields.motion[1] = (uint32)current_matrix->t[1] - collision_cache->fields.position[1];
                            collision_cache->fields.motion[2] = (uint32)current_matrix->t[2] - collision_cache->fields.position[2];
                        }
                        matrix = render_view.matrix_cursor;
                        current_matrix = &matrix->matrix;
                        parent_matrix = (matrix->prev != 0 ? &matrix->prev->matrix : 0);
                        {
                            uint32 c0 = (uint32)current_matrix->t[0], c1 = (uint32)current_matrix->t[1], c2 = (uint32)current_matrix->t[2];
                            collision_cache->fields.position[0] = c0;
                            collision_cache->fields.position[1] = c1;
                            collision_cache->fields.position[2] = c2;
                        }
                        matrix = render_view.matrix_cursor;
                        current_matrix = &matrix->matrix;
                        parent_matrix = (matrix->prev != 0 ? &matrix->prev->matrix : 0);
                        for (i = 0; i < 9; ++i)
                            collision_cache->fields.rotation[i] = current_matrix->m[i % 3][i / 3];
                    }
                }
                node->flags = node->flags & 0xBF;
            }
            if ((work & 0x14) == 0x10)
            {
                if ((IsSphereVisible(actor->world_position, actor->metadata_words72[0]) & 0xFF) != 0)
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
                        UpdateMaterialAnimationList(actor->material_animation_entries);
                        matrix = render_view.matrix_cursor;
                        current_matrix = &matrix->matrix;
                        parent_matrix = (matrix->prev != 0 ? &matrix->prev->matrix : 0);
                        SetLightingBounds((const uint32 *)current_matrix->t, (sint32)actor->metadata_words72[0] >> 5);
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
                linked_child = child_reference != 0 ? (AnimatedObject *)child_reference : (AnimatedObject *)0;
                flags = linked_child->flags;
                if ((flags & 0x20200) != 0x200)
                {
                    linked_child->flags = flags & 0xFFFDFFFF;
                    {
                        uint32 c0 = linked_child->spheres[0].position[0], c1 = linked_child->spheres[0].position[1], c2 = linked_child->spheres[0].position[2];
                        linked_child->world_position[0] = c0;
                        linked_child->world_position[1] = c1;
                        linked_child->world_position[2] = c2;
                    }
                    x = linked_child->spheres[0].radius;
                    linked_child->metadata_words72[2] = x;
                    linked_child->metadata_words72[1] = x;
                    linked_child->metadata_words72[0] = x;
                    x = linked_child->spheres[0].position[0];
                    y = linked_child->spheres[0].position[1];
                    z = linked_child->spheres[0].position[2];
                    w = linked_child->spheres[0].radius;
                    actor->auxiliary_words[0] = x;
                    actor->auxiliary_words[1] = y;
                    actor->auxiliary_words[2] = z;
                    actor->auxiliary_words[3] = w;
                    {
                        uint32 c0 = linked_child->world_position[0], c1 = linked_child->world_position[1], c2 = linked_child->world_position[2];
                        actor->world_position[0] = c0;
                        actor->world_position[1] = c1;
                        actor->world_position[2] = c2;
                    }
                    {
                        uint32 c0 = linked_child->metadata_words72[0], c1 = linked_child->metadata_words72[1], c2 = linked_child->metadata_words72[2];
                        actor->metadata_words72[0] = c0;
                        actor->metadata_words72[1] = c1;
                        actor->metadata_words72[2] = c2;
                    }
                }
                matrix = render_view.matrix_cursor;
                current_matrix = &matrix->matrix;
                parent_matrix = (matrix->prev != 0 ? &matrix->prev->matrix : 0);
                {
                    uint32 c0 = (uint32)current_matrix->t[0], c1 = (uint32)current_matrix->t[1], c2 = (uint32)current_matrix->t[2];
                    actor->auxiliary_words[4] = c0;
                    actor->auxiliary_words[5] = c1;
                    actor->auxiliary_words[6] = c2;
                }
                if ((sint8)actor->metadata126[0] != 0)
                    actor->path_motion_scale = 4096;
                index = actor->movement_mode;
                if (index != 0)
                {
                    const ObjectCallback operation = object_operations.motions[index];
                    if (operation.member_handler)
                        ((actor)->*operation.member_handler)();
                    else if (operation.void_member_handler)
                        ((actor)->*operation.void_member_handler)();
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
            sprite = (SpriteRecord *)node;
            ApplyRotMatrixLV((const VECTOR *)sprite->position, (VECTOR *)current_matrix->t);
            matrix = render_view.matrix_cursor;
            current_matrix = &matrix->matrix;
            parent_matrix = (matrix->prev != 0 ? &matrix->prev->matrix : 0);
            x = (uint32)current_matrix->t[0] + (uint32)parent_matrix->t[0];
            y = (uint32)current_matrix->t[1] + (uint32)parent_matrix->t[1];
            z = (uint32)current_matrix->t[2] + (uint32)parent_matrix->t[2];
            current_matrix->t[0] = (sint32)(x);
            current_matrix->t[1] = (sint32)(y);
            current_matrix->t[2] = (sint32)(z);

            if ((work & 0x14) == 0x10)
            {
                if (parent == 0)
                {
                    UpdateSpriteExtent(sprite);
                    if ((node->state_flags & 0x10) != 0)
                        render_view.far_distance = 0x1FFFFC00;
                    matrix = render_view.matrix_cursor;
                    current_matrix = &matrix->matrix;
                    parent_matrix = (matrix->prev != 0 ? &matrix->prev->matrix : 0);
                    if ((IsSphereVisible((const uint32 *)current_matrix->t, (uint32)(sprite->extent[0] + sprite->extent[1]) << 5) & 0xFF) != 0)
                    {
                        if ((UpdateMaterialAnimationList(sprite->animation_entries) & 0xFF) == 0)
                        {
                            DetachChild(node);
                            ReleaseSceneObject(node);
                        }
                        else
                        {
                            game_flow.transition_flags = game_flow.transition_flags | 0x200;
                            sprite->Draw();
                            game_flow.transition_flags = game_flow.transition_flags & 0xFFFFFDFF;
                        }
                    }
                    render_view.far_distance = camera_projection.far_distance;
                    goto next_object;
                }
                stack_source = parent_matrix;
                {
                    uint32 c0 = xport_load_le32((const uint8 *)stack_source), c1 = xport_load_le32((const uint8 *)stack_source + 4), c2 = xport_load_le32((const uint8 *)stack_source + 8), c3 = xport_load_le32((const uint8 *)stack_source + 12);
                    xport_store_le32((uint8 *)current_matrix, c0);
                    xport_store_le32((uint8 *)current_matrix + 4, c1);
                    xport_store_le32((uint8 *)current_matrix + 8, c2);
                    xport_store_le32((uint8 *)current_matrix + 12, c3);
                    current_matrix->m[2][2] = (sint16)((uint16)stack_source->m[2][2]);
                }

                if ((UpdateMaterialAnimationList(sprite->animation_entries) & 0xFF) != 0)
                    sprite->Draw();
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
                ReleaseSceneObject(node);
                goto next_object;
            }
            if (node->links.first_child != 0)
            {
                matrix = render_view.matrix_cursor;
                current_matrix = &matrix->matrix;
                parent_matrix = (matrix->prev != 0 ? &matrix->prev->matrix : 0);
                stack_source = parent_matrix;
                {
                    uint32 c0 = xport_load_le32((const uint8 *)stack_source), c1 = xport_load_le32((const uint8 *)stack_source + 4), c2 = xport_load_le32((const uint8 *)stack_source + 8), c3 = xport_load_le32((const uint8 *)stack_source + 12);
                    xport_store_le32((uint8 *)current_matrix, c0);
                    xport_store_le32((uint8 *)current_matrix + 4, c1);
                    xport_store_le32((uint8 *)current_matrix + 8, c2);
                    xport_store_le32((uint8 *)current_matrix + 12, c3);
                    current_matrix->m[2][2] = (sint16)((uint16)stack_source->m[2][2]);
                }
                goto visit_children;
            }
            goto next_object;
        }
        if (category == 8)
        {
            billboard = (PositionedBillboardNode *)node;
            ApplyRotMatrixLV((const VECTOR *)billboard->position, (VECTOR *)billboard->world_position);
            matrix = render_view.matrix_cursor;
            current_matrix = &matrix->matrix;
            parent_matrix = (matrix->prev != 0 ? &matrix->prev->matrix : 0);
            for (i = 0; i < 3; ++i)
                billboard->world_position[i] = billboard->world_position[i] + (uint32)parent_matrix->t[i];
            if ((work & 0x14) != 0x10 || (parent == 0 && (IsSphereVisible(billboard->world_position, billboard->billboard.extent) & 0xFF) == 0))
            {
                node->flags = node->flags | 4;
                goto next_object;
            }
            for (i = 0; i < 5; ++i)
                xport_gte_write_control(i, xport_load_le32((const uint8 *)&render_view.camera_transform + i * 4));
            ApplyRotMatrixLV((const VECTOR *)billboard->world_position, (VECTOR *)(&render_view.matrix_cursor->matrix)->t);
            matrix = render_view.matrix_cursor;
            current_matrix = &matrix->matrix;
            parent_matrix = (matrix->prev != 0 ? &matrix->prev->matrix : 0);
            for (i = 0; i < 3; ++i)
                current_matrix->t[i] = (sint32)((uint32)current_matrix->t[i] + (uint32)render_view.camera_transform.t[i]);
            node->DrawBillboard();
            node->flags = node->flags & 0xFB;
            goto next_object;
        }
        if (category == 2)
        {
            SceneTrigger *trigger = (SceneTrigger *)node;
            ApplyRotMatrixLV((const VECTOR *)trigger->position, (VECTOR *)current_matrix->t);
            matrix = render_view.matrix_cursor;
            current_matrix = &matrix->matrix;
            parent_matrix = (matrix->prev != 0 ? &matrix->prev->matrix : 0);
            x = (uint32)current_matrix->t[0] + (uint32)parent_matrix->t[0];
            y = (uint32)current_matrix->t[1] + (uint32)parent_matrix->t[1];
            z = (uint32)current_matrix->t[2] + (uint32)parent_matrix->t[2];
            current_matrix->t[0] = (sint32)(x);
            current_matrix->t[1] = (sint32)(y);
            current_matrix->t[2] = (sint32)(z);

            if ((trigger->node.state_flags & 0xFF20) != 0x0120)
            {
                if (trigger->rotation[0] == 0x4000)
                {
                    stack_source = parent_matrix;
                    {
                        uint32 c0 = xport_load_le32((const uint8 *)stack_source), c1 = xport_load_le32((const uint8 *)stack_source + 4), c2 = xport_load_le32((const uint8 *)stack_source + 8), c3 = xport_load_le32((const uint8 *)stack_source + 12);
                        xport_store_le32((uint8 *)current_matrix, c0);
                        xport_store_le32((uint8 *)current_matrix + 4, c1);
                        xport_store_le32((uint8 *)current_matrix + 8, c2);
                        xport_store_le32((uint8 *)current_matrix + 12, c3);
                        current_matrix->m[2][2] = (sint16)((uint16)stack_source->m[2][2]);
                    }
                }
                else
                {
                    {
                        uint32 qw = (uint32)trigger->rotation[0], qx = (uint32)trigger->rotation[1], qy = (uint32)trigger->rotation[2], qz = (uint32)trigger->rotation[3];
                        uint32 xx = qx * qx, yy = qy * qy, zz = qz * qz, xy = qx * qy, xz = qx * qz, yz = qy * qz, wz = qw * qz, wy = qw * qy, wx = qw * qx;
                        current_matrix->m[0][0] = (sint16)((sint32)(0x08000000u - (yy + zz)) >> 15);
                        current_matrix->m[1][1] = (sint16)((sint32)(0x08000000u - (xx + zz)) >> 15);
                        current_matrix->m[2][2] = (sint16)((sint32)(0x08000000u - (xx + yy)) >> 15);
                        current_matrix->m[1][0] = (sint16)((sint32)(xy - wz) >> 15);
                        current_matrix->m[0][1] = (sint16)((sint32)(xy + wz) >> 15);
                        current_matrix->m[2][0] = (sint16)((sint32)(xz + wy) >> 15);
                        current_matrix->m[0][2] = (sint16)((sint32)(xz - wy) >> 15);
                        current_matrix->m[2][1] = (sint16)((sint32)(yz - wx) >> 15);
                        current_matrix->m[1][2] = (sint16)((sint32)(yz + wx) >> 15);
                    }
                    MulRotMatrix(current_matrix);
                    stack_source = current_matrix;
                    for (i = 0; i < 5; ++i)
                        xport_gte_write_control(i, xport_load_le32((const uint8 *)stack_source + i * 4));
                }
            }
            else
            {
                trigger_path = (trigger->parameters.path_pair.targets[0]);
                trigger_path = ((const ScenePath *)(trigger_path));
                trigger_definition = trigger_path->definition;
                x = trigger->parameters.path_pair.metadata;
                ((const PathDefinition *)(trigger_definition))->SampleTangent(&input.vector, (sint32)x < 0 ? 0 : (sint32)x);
                if ((sint32)x >= 0)
                {
                    angle = (sint16)Atan2Angle((sint32)input.words[2], (sint32)input.words[0]);
                    ComposeHeadingQuaternion((sint16)angle, 0, (Quaternion16 *)trigger->rotation);
                    matrix = render_view.matrix_cursor;
                    current_matrix = &matrix->matrix;
                    parent_matrix = (matrix->prev != 0 ? &matrix->prev->matrix : 0);
                    if (trigger->rotation[0] == 0x4000)
                    {
                        stack_source = root_matrix;
                        {
                            uint32 c0 = xport_load_le32((const uint8 *)stack_source), c1 = xport_load_le32((const uint8 *)stack_source + 4), c2 = xport_load_le32((const uint8 *)stack_source + 8);
                            xport_store_le32((uint8 *)current_matrix, c0);
                            xport_store_le32((uint8 *)current_matrix + 4, c1);
                            xport_store_le32((uint8 *)current_matrix + 8, c2);
                            c0 = xport_load_le32((const uint8 *)stack_source + 12);
                            c1 = (uint32)stack_source->m[2][2];
                            xport_store_le32((uint8 *)current_matrix + 12, c0);
                            current_matrix->m[2][2] = (sint16)(c1);
                        }
                    }
                    else
                    {
                        {
                            uint32 qw = (uint32)trigger->rotation[0], qx = (uint32)trigger->rotation[1], qy = (uint32)trigger->rotation[2], qz = (uint32)trigger->rotation[3];
                            uint32 xx = qx * qx, yy = qy * qy, zz = qz * qz, xy = qx * qy, xz = qx * qz, yz = qy * qz, wz = qw * qz, wy = qw * qy, wx = qw * qx;
                            tmp.matrix.m[0][0] = (sint16)((sint32)(0x08000000u - (yy + zz)) >> 15);
                            tmp.matrix.m[1][1] = (sint16)((sint32)(0x08000000u - (xx + zz)) >> 15);
                            tmp.matrix.m[2][2] = (sint16)((sint32)(0x08000000u - (xx + yy)) >> 15);
                            tmp.matrix.m[1][0] = (sint16)((sint32)(xy - wz) >> 15);
                            tmp.matrix.m[0][1] = (sint16)((sint32)(xy + wz) >> 15);
                            tmp.matrix.m[2][0] = (sint16)((sint32)(xz + wy) >> 15);
                            tmp.matrix.m[0][2] = (sint16)((sint32)(xz - wy) >> 15);
                            tmp.matrix.m[2][1] = (sint16)((sint32)(yz - wx) >> 15);
                            tmp.matrix.m[1][2] = (sint16)((sint32)(yz + wx) >> 15);
                        }
                        MulMatrix0((MATRIX *)root_matrix, &tmp.matrix, &render_view.matrix_cursor->matrix);
                    }
                }
                else
                {
                    angle = AsinAngle((sint32)(0u - input.words[1]), 4096);
                    cosine = SinQ14((sint16)(1024 - (angle & 0xFFF))) >> 2;
                    angle = AsinAngle((sint32)(0u - input.words[1]), 4096);
                    sine = SinQ14((sint16)(angle & 0xFFF)) >> 2;
                    rotation_x.m[1][1] = (sint16)(cosine);
                    rotation_x.m[2][1] = (sint16)(sine);
                    rotation_x.m[1][2] = (sint16)(0u - (uint32)sine);
                    rotation_x.m[2][2] = (sint16)(cosine);
                    angle = (sint16)Atan2Angle((sint32)input.words[2], (sint32)input.words[0]);
                    cosine = SinQ14((sint16)(1024 - angle)) >> 2;
                    angle = (sint16)Atan2Angle((sint32)input.words[2], (sint32)input.words[0]);
                    sine = SinQ14((sint16)angle) >> 2;
                    rotation_y.m[0][0] = (sint16)(cosine);
                    rotation_y.m[2][0] = (sint16)(0u - (uint32)sine);
                    rotation_y.m[0][2] = (sint16)(sine);
                    rotation_y.m[2][2] = (sint16)(cosine);
                    MulMatrix0(&rotation_y, &rotation_x, &tmp.matrix);
                    MulMatrix0((MATRIX *)root_matrix, &tmp.matrix, &render_view.matrix_cursor->matrix);
                    stack_source = &render_view.matrix_cursor->matrix;
                    for (i = 0; i < 5; ++i)
                        xport_gte_write_control(i, xport_load_le32((const uint8 *)stack_source + i * 4));
                }
            }
            trigger->node.state_flags = (trigger->node.state_flags & 0xFF00) | (uint8)((trigger->node.state_flags & 0xFF) | 0x40);

            if ((work & 0x8C) != 0)
            {
                trigger->delta[2] = 0;
                trigger->delta[1] = 0;
                trigger->delta[0] = 0;
            }
            else
            {
                matrix = render_view.matrix_cursor;
                current_matrix = &matrix->matrix;
                parent_matrix = (matrix->prev != 0 ? &matrix->prev->matrix : 0);
                trigger->delta[0] = (uint32)current_matrix->t[0] - trigger->transform.t[0];
                trigger->delta[1] = (uint32)current_matrix->t[1] - trigger->transform.t[1];
                trigger->delta[2] = (uint32)current_matrix->t[2] - trigger->transform.t[2];
            }
            matrix = render_view.matrix_cursor;
            current_matrix = &matrix->matrix;
            parent_matrix = (matrix->prev != 0 ? &matrix->prev->matrix : 0);
            {
                uint32 c0 = (uint32)current_matrix->t[0], c1 = (uint32)current_matrix->t[1], c2 = (uint32)current_matrix->t[2];
                trigger->transform.t[0] = c0;
                trigger->transform.t[1] = c1;
                trigger->transform.t[2] = c2;
            }
            matrix = render_view.matrix_cursor;
            current_matrix = &matrix->matrix;
            parent_matrix = (matrix->prev != 0 ? &matrix->prev->matrix : 0);
            for (i = 0; i < 9; ++i)
                trigger->transform.m[i / 3][i % 3] = (sint16)(uint16)current_matrix->m[i % 3][i / 3];

            if (parent != 0)
            {
                if ((parent_view->flags & 0x200) != 0)
                    trigger->node.flags = trigger->node.flags | 4;
                else if ((trigger->node.flags & 4) != 0)
                {
                    if ((work & 2) == 0)
                    {
                        trigger->delta[2] = 0;
                        trigger->delta[1] = 0;
                        trigger->delta[0] = 0;
                    }
                    trigger->node.flags = trigger->node.flags & 0xFB;
                }
            }
            goto visit_children;
        }
        if (category == 6)
        {
            sphere_node = (SphereNode *)node;
            ApplyRotMatrixLV((const VECTOR *)sphere_node->position, (VECTOR *)current_matrix->t);
            matrix = render_view.matrix_cursor;
            current_matrix = &matrix->matrix;
            parent_matrix = (matrix->prev != 0 ? &matrix->prev->matrix : 0);
            x = (uint32)current_matrix->t[0] + (uint32)parent_matrix->t[0];
            y = (uint32)current_matrix->t[1] + (uint32)parent_matrix->t[1];
            z = (uint32)current_matrix->t[2] + (uint32)parent_matrix->t[2];
            current_matrix->t[0] = (sint32)(x);
            current_matrix->t[1] = (sint32)(y);
            current_matrix->t[2] = (sint32)(z);

            if ((work & 0x40) != 0 && this == (ObjectLinks *)parent)
            {
                {
                    uint32 c0 = (uint32)current_matrix->t[0], c1 = (uint32)current_matrix->t[1], c2 = (uint32)current_matrix->t[2];
                    parent_view->auxiliary_words[0] = c0;
                    parent_view->auxiliary_words[1] = c1;
                    parent_view->auxiliary_words[2] = c2;
                }
                flags = sphere_node->radius & 0xFFFFFFF8;
                parent_view->auxiliary_words[3] = flags;
                parent_view->auxiliary_words[3] = flags | ((node->state_flags >> 8) & 7);
            }
            goto visit_children;
        }
        if (category == 3)
        {
            path = (ScenePath *)node;
            follow = (work & 4) == 0 && path == camera_motion.path;
            if (follow != 0)
            {
                path_matrix = (MATRIX *)(path->offset_matrix);
                for (i = 0; i < 3; ++i)
                    delta.words[i] = (uint32)path_matrix->t[i];
            }
            path_matrix = (MATRIX *)(path->offset_matrix);
            matrix = render_view.matrix_cursor;
            current_matrix = &matrix->matrix;
            parent_matrix = (matrix->prev != 0 ? &matrix->prev->matrix : 0);
            // Copy all eight original words including MATRIX padding
            memcpy(path_matrix, parent_matrix, sizeof(MATRIX));
            if (follow != 0)
            {
                player = FindPlayerObject(0);
                if (player != 0 && (player->flags & 0x2000) == 0)
                {
                    target = OriginalDefaults::SceneContextIdentity;
                    for (i = 0; i < 3; ++i)
                        camera_motion.desired_position[i] = camera_motion.desired_position[i] + delta.words[i];
                    for (i = 0; i < 3; ++i)
                    {
                        path_matrix = (MATRIX *)(path->offset_matrix);
                        delta.words[i] = (uint32)path_matrix->t[i] - delta.words[i];
                    }
                    if ((parent_view->flags & 0x20) != 0)
                    {
                        for (i = 0; i < 3; ++i)
                            tmp.words[i] = camera_motion.desired_position[i] - camera_motion.position[i];
                        SetCameraTransition(tmp.words);
                        target = OriginalDefaults::SceneContextIdentity;
                        if (camera_motion.smooth_ticks[0] < 4)
                        {
                            camera_motion.smooth_ticks[0] = 4;
                            target = OriginalDefaults::SceneContextIdentity;
                        }
                        if (camera_motion.smooth_ticks[1] < 4)
                            camera_motion.smooth_ticks[1] = 4;
                        target = OriginalDefaults::SceneContextIdentity;
                        if (camera_motion.smooth_ticks[2] < 4)
                            camera_motion.smooth_ticks[2] = 4;
                    }
                    target = OriginalDefaults::SceneContextIdentity;
                    for (i = 0; i < 3; ++i)
                        camera_motion.position[i] = camera_motion.position[i] + delta.words[i];
                }
            }
            goto visit_children;
        }
        if (category == 9)
        {
            definition = (PathDefinition *)node;
            if ((definition->transform) != 0)
            {
                follow = 0;
                player = 0;
                if ((work & 4) == 0 && definition == (camera_motion.path->definition))
                {
                    player = FindPlayerObject(0);
                    if (player != 0 && definition == (player->path_definition))
                        follow = (player->flags & 0x2400) == 0x400;
                }
                if (follow != 0)
                {
                    player_definition = (player->path_definition);
                    if (player_definition != 0 && (player_definition->transform) != 0)
                    {
                        {
                            uint32 c0 = player->position[0], c1 = player->position[1], c2 = player->position[2];
                            tmp.words[0] = c0;
                            tmp.words[1] = c1;
                            tmp.words[2] = c2;
                        }
                        PushMatrix();
                        definition_matrix = (const MATRIX *)((((const PathDefinition *)((const PathDefinition *)((player->path_definition))))->transform) != 0 ? &(((const PathDefinition *)((const PathDefinition *)((player->path_definition))))->transform)->matrix : (MATRIX *)0);
                        for (i = 0; i < 5; ++i)
                            xport_gte_write_control(i, xport_load_le32((const uint8 *)definition_matrix + i * 4));
                        ApplyRotMatrixLV(&tmp.vector, &delta.vector);
                        PopMatrix();
                        for (i = 0; i < 3; ++i)
                        {
                            definition_matrix = (const MATRIX *)((((const PathDefinition *)((const PathDefinition *)((player->path_definition))))->transform) != 0 ? &(((const PathDefinition *)((const PathDefinition *)((player->path_definition))))->transform)->matrix : (MATRIX *)0);
                            delta.words[i] = delta.words[i] + (uint32)definition_matrix->t[i];
                        }
                    }
                    else
                    {
                        {
                            uint32 c0 = player->position[0], c1 = player->position[1], c2 = player->position[2];
                            delta.words[0] = c0;
                            delta.words[1] = c1;
                            delta.words[2] = c2;
                        }
                    }
                }
                matrix = render_view.matrix_cursor;
                current_matrix = &matrix->matrix;
                parent_matrix = (matrix->prev != 0 ? &matrix->prev->matrix : 0);
                path_transform = (PathTransform *)((PathTransform *)((definition->transform)));
                render_view.matrix_cursor = matrix->prev;

                if ((work & 0x8C) != 0)
                {
                    path_transform->displacement[2] = 0;
                    path_transform->displacement[1] = 0;
                    path_transform->displacement[0] = 0;
                }
                else
                {
                    matrix = render_view.matrix_cursor;
                    current_matrix = &matrix->matrix;
                    parent_matrix = (matrix->prev != 0 ? &matrix->prev->matrix : 0);
                    path_transform->displacement[0] = (uint32)current_matrix->t[0] - (uint32)path_transform->matrix.t[0];
                    path_transform->displacement[1] = (uint32)current_matrix->t[1] - (uint32)path_transform->matrix.t[1];
                    path_transform->displacement[2] = (uint32)current_matrix->t[2] - (uint32)path_transform->matrix.t[2];
                }
                matrix = render_view.matrix_cursor;
                current_matrix = &matrix->matrix;
                parent_matrix = (matrix->prev != 0 ? &matrix->prev->matrix : 0);
                for (i = 0; i < 3; ++i)
                    path_transform->matrix.t[i] = current_matrix->t[i];
                matrix = render_view.matrix_cursor;
                current_matrix = &matrix->matrix;
                parent_matrix = (matrix->prev != 0 ? &matrix->prev->matrix : 0);
                for (i = 0; i < 9; ++i)
                    path_transform->matrix.m[i / 3][i % 3] = current_matrix->m[i % 3][i / 3];

                if (parent != 0)
                {
                    if ((parent_view->flags & 0x200) != 0)
                        node->flags = node->flags | 4;
                    else if ((node->flags & 4) != 0)
                    {
                        if ((work & 2) == 0)
                        {
                            path_transform->displacement[2] = 0;
                            path_transform->displacement[1] = 0;
                            path_transform->displacement[0] = 0;
                        }
                        node->flags = node->flags & 0xFB;
                    }
                }
                path_transform = (PathTransform *)((PathTransform *)((definition->transform)));
                stack_source = &render_view.matrix_cursor->matrix;
                for (i = 0; i < 3; ++i)
                {
                    for (uint32 column = 0; column < 3; ++column)
                        path_transform->matrix.m[i][column] = stack_source->m[i][column];
                }
                render_view.matrix_cursor = render_view.matrix_cursor->next;
                if (follow != 0)
                {
                    parent_view->flags = parent_view->flags & 0xFFFFFDFF;
                    path_transform = (PathTransform *)((PathTransform *)((definition->transform)));
                    node->flags = node->flags & 0xFB;
                    if ((sint32)path_transform->displacement[1] < 0 || (work & 8) != 0)
                    {
                        player_definition = (player->path_definition);
                        if (player_definition != 0 && (player_definition->transform) != 0)
                        {
                            {
                                uint32 c0 = player->position[0], c1 = player->position[1], c2 = player->position[2];
                                before.words[0] = c0;
                                before.words[1] = c1;
                                before.words[2] = c2;
                            }
                            PushMatrix();
                            definition_matrix = (const MATRIX *)((((const PathDefinition *)((const PathDefinition *)((player->path_definition))))->transform) != 0 ? &(((const PathDefinition *)((const PathDefinition *)((player->path_definition))))->transform)->matrix : (MATRIX *)0);
                            for (i = 0; i < 5; ++i)
                                xport_gte_write_control(i, xport_load_le32((const uint8 *)definition_matrix + i * 4));
                            ApplyRotMatrixLV(&before.vector, (VECTOR *)player->position);
                            PopMatrix();
                            player_definition = (const PathDefinition *)((const PathDefinition *)((player->path_definition)));
                            definition_matrix = (const MATRIX *)((player_definition->transform) != 0 ? &(player_definition->transform)->matrix : (MATRIX *)0);
                            player->position[0] = player->position[0] + (uint32)definition_matrix->t[0];
                            definition_matrix = (const MATRIX *)((player_definition->transform) != 0 ? &(player_definition->transform)->matrix : (MATRIX *)0);
                            x = (uint32)definition_matrix->t[1];
                            player_definition = (const PathDefinition *)((const PathDefinition *)((player->path_definition)));
                            player->position[1] = player->position[1] + x;
                            definition_matrix = (const MATRIX *)((player_definition->transform) != 0 ? &(player_definition->transform)->matrix : (MATRIX *)0);
                            player->position[2] = player->position[2] + (uint32)definition_matrix->t[2];
                        }
                        else
                        {
                            {
                                uint32 c0 = player->position[0], c1 = player->position[1], c2 = player->position[2];
                                player->position[0] = c0;
                                player->position[1] = c1;
                                player->position[2] = c2;
                            }
                        }
                        player->position[1] = delta.words[1];
                        player_definition = (player->path_definition);
                        if (player_definition != 0 && (player_definition->transform) != 0)
                        {
                            for (i = 0; i < 3; ++i)
                            {
                                definition_matrix = (const MATRIX *)((((const PathDefinition *)((const PathDefinition *)((player->path_definition))))->transform) != 0 ? &(((const PathDefinition *)((const PathDefinition *)((player->path_definition))))->transform)->matrix : (MATRIX *)0);
                                relative.words[i] = player->position[i] - (uint32)definition_matrix->t[i];
                            }
                            for (i = 0; i < 9; ++i)
                            {
                                definition_matrix = (const MATRIX *)((((const PathDefinition *)((const PathDefinition *)((player->path_definition))))->transform) != 0 ? &(((const PathDefinition *)((const PathDefinition *)((player->path_definition))))->transform)->matrix : (MATRIX *)0);
                                transposed.m[i / 3][i % 3] = definition_matrix->m[i % 3][i / 3];
                            }
                            PushMatrix();
                            for (i = 0; i < 4; ++i)
                                xport_gte_write_control(i, xport_load_le32((const uint8 *)&transposed + i * 4));
                            xport_gte_write_control(4, (uint16)transposed.m[2][2]);
                            ApplyRotMatrixLV(&relative.vector, (VECTOR *)player->position);
                            PopMatrix();
                        }
                        else
                        {
                            {
                                uint32 c0 = player->position[0], c1 = player->position[1], c2 = player->position[2];
                                player->position[0] = c0;
                                player->position[1] = c1;
                                player->position[2] = c2;
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
            ApplyRotMatrixLV((const VECTOR *)light->position, &tmp.vector);
            matrix = render_view.matrix_cursor;
            current_matrix = &matrix->matrix;
            parent_matrix = (matrix->prev != 0 ? &matrix->prev->matrix : 0);
            for (i = 0; i < 3; ++i)
                tmp.words[i] = tmp.words[i] + (uint32)parent_matrix->t[i];
            if ((work & 0x8D) != 0 || light->prev_position[0] != tmp.words[0] || light->prev_position[1] != tmp.words[1] || light->prev_position[2] != tmp.words[2])
            {
                {
                    uint32 c0 = tmp.words[0], c1 = tmp.words[1], c2 = tmp.words[2];
                    light->prev_position[0] = c0;
                    light->prev_position[1] = c1;
                    light->prev_position[2] = c2;
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
