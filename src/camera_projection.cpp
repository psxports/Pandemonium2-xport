#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "camera_projection.h"
#include "game.h"
#include "psx_gte.h"
#include "global.h"

CameraProjection camera_proj = {OriginalDefaults::CameraProjectionScale};

uint32 CameraProjection::IsSphereVisible(const sint32 *pos, uint32 radius)
{
    sint32 x;
    sint32 y;
    sint32 z;
    sint32 coefficient;
    uint32 product_x;
    uint32 product_y;
    uint32 product_z;
    sint32 sum;
    uint32 bias;
    sint32 distance;
    sint32 limit = -radius;
    uint32 idx;
    const FrustumPlane *plane;
    const uint32 plane_order[4] = {1, 3, 2, 4};
    FUNCTION_MARKER(0x80011338, "SLUS_005.78");
    sum = pos[0];
    coefficient = render_view.planes[0].n.vx;
    x = sum >> 6;
    product_x = (sint64)x * coefficient;
    sum = pos[1];
    coefficient = render_view.planes[0].n.vy;
    y = sum >> 6;
    product_y = (sint64)y * coefficient;
    sum = pos[2];
    coefficient = render_view.planes[0].n.vz;
    z = sum >> 6;
    product_z = (sint64)z * coefficient;
    bias = render_view.planes[0].bias;
    sum = product_x + product_y + product_z;
    distance = (sum >> 6) + bias;
    if (limit >= distance)
        return 0;
    sum = render_view.far_distance - distance;
    if (limit >= sum)
        return 0;
    for (idx = 0; idx < 4; ++idx)
    {
        plane = &render_view.planes[plane_order[idx]];
        coefficient = plane->n.vx;
        product_x = (sint64)x * coefficient;
        coefficient = plane->n.vy;
        product_y = (sint64)y * coefficient;
        coefficient = plane->n.vz;
        product_z = (sint64)z * coefficient;
        bias = plane->bias;
        sum = product_x + product_y + product_z;
        distance = (sum >> 6) + bias;
        if (limit >= distance)
            return 0;
    }
    return 1;
}

void CameraProjection::UpdateAndDrawRegisteredObjects(void)
{
    // Q27 unity becomes Q12 after the coefficient shift
    ObjectCallback object_operation;
    AnimatedObject *animated;
    VECTOR pos;
    SceneNode *object, *next;
    CollisionSphere *sphere_output;

    RenderMatrixSlot *matrix;
    MATRIX *cur_matrix, *rotation_matrix;
    MATRIX *parent_matrix;
    MATRIX *definition_matrix;
    const PathDefinition *definition;
    SceneNode *spr;

    uint32 target;
    uint32 flags;
    uint32 saved_ot;
    uint32 saved_fog;
    sint32 radius;
    uint32 scale;
    uint32 rcp;
    uint32 special;
    uint32 x;
    uint32 y;
    uint32 z;
    uint32 val;
    uint32 xx;
    uint32 yy;
    uint32 zz;
    uint32 xy;
    uint32 xz;
    uint32 yz;
    uint32 wz;
    uint32 wy;
    uint32 wx;
    uint32 idx;
    sint32 kind;
    sint32 variant;
    sint32 mode;
    sint32 qw;
    sint32 qx;
    sint32 qy;
    sint32 qz;
    sint16 c;
    sint16 s;
    sint32 coefficient;
    uint8 byte_flags;
    FUNCTION_MARKER(0x80012EB8, "SLUS_005.78");
    const ActorModelDefinition *bound_model;
    object = registered_object_head;
    matrix = render_view.matrix_cursor;
    cur_matrix = &matrix->matrix;
    parent_matrix = matrix->prev != 0 ? &matrix->prev->matrix : 0;
    render_view.lighting_cache = NULL;
    render_view.lighting_enabled = false;
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
    while (object != 0)
    {
        animated = (AnimatedObject *)object;
        kind = animated->behavior;
        saved_ot = 0;
        if (kind != 1)
        {
            variant = animated->behavior_suspended;
            if (variant != 0)
                object_operation = object_operations.variants[variant];
            else
            {
                object_operation = object_operations.behaviors[kind];
            }

            {
                const ObjectCallback operation = object_operation;
                if (operation.member_handler)
                    (animated->*operation.member_handler)();
                else if (operation.void_member_handler)
                    (animated->*operation.void_member_handler)();
                else if (operation.factory_handler)
                    operation.factory_handler(animated);
                else
                    operation.plain_handler();
            }
            if ((animated->flags & 0x80000) != 0)
                goto r_tint;
            if ((animated->path_flags & 0x4000) != 0)
            {
                bound_model = ((const ActorBinding *)(((const ActorPlacement *)animated->path_definition)->state))->model;
                flags = game_flow.transition_flags;
                z = bound_model->fog_color[2];
                y = bound_model->fog_color[1];
                x = (sint8)bound_model->fog_color[0];
                game_flow.transition_flags = flags | 0x20;
                render_view.fog_color = (z << 16) | (y << 8) | x;
            }
        }
        else
        {
            val = scr_render.fade_level;
            if (val != 0)
            {
                if (val < 4096)
                {
                    saved_ot = render_commands.ordering_table_offset;
                    saved_fog = render_view.fog_scale;
                    target = render_commands.small_ordering_table_offset;
                    render_view.fog_scale = 1310;
                    render_commands.ordering_table_offset = target;
                }
            }
            else
            {
                if ((CharacterModelRecord::IsAlternateCharacterInteractionActive() << 24) != 0)
                    game_flow.transition_flags = game_flow.transition_flags | 4;
                mode = animated->ClassifyCharacterContactResponse();
                if (mode >= 2)
                {
                    if (mode == 2)
                    {
                        flags = game_flow.transition_flags;
                        render_view.fog_color = 0xFFFFFF;
                        game_flow.transition_flags = flags | 0x20;
                    }
                    else if (mode == 4)
                        render_view.material_override = resource_grps[26].tex;
                    else if (mode == 5)
                    {
                        x = character_pad.appearance.tint;
                        flags = game_flow.transition_flags;
                        render_view.fog_color = x;
                        game_flow.transition_flags = flags | 0x20;
                        render_view.material_override = resource_grps[25].tex;
                    }
                }
                else if ((animated->flags & 0x80000) != 0)
                {
                r_tint:
                    flags = game_flow.transition_flags;
                    render_view.fog_color = 255;
                    game_flow.transition_flags = flags | 0x20;
                }
            }
        }
        flags = animated->flags;
        next = animated->node.links.registry_next;
        definition_matrix = 0;
        if ((flags & 0x2000) == 0)
        {
            definition = animated->path_definition;
            if (definition != 0)
            {
                PathTransform *captured_path_matrix = definition->transform;
                definition_matrix = captured_path_matrix != 0 ? &captured_path_matrix->matrix : 0;
            }
        }
        if (definition_matrix != 0)
        {
            x = animated->pos.x;
            y = animated->pos.y;
            z = animated->pos.z;
            pos.vx = x;
            pos.vy = y;
            pos.vz = z;
            PushMatrix();
            definition_matrix = animated->path_definition->transform != 0 ? &animated->path_definition->transform->matrix : NULL;
            SetRotMatrix(definition_matrix);
            MATRIX *pos_matrix = &render_view.matrix_cursor->matrix;
            {
                VECTOR rotation_output;
                ApplyRotMatrixLV(&pos, &rotation_output);
                pos_matrix->t[0] = rotation_output.vx;
                pos_matrix->t[1] = rotation_output.vy;
                pos_matrix->t[2] = rotation_output.vz;
            }
            PopMatrix();
            matrix = render_view.matrix_cursor;
            cur_matrix = &matrix->matrix;
            parent_matrix = matrix->prev != 0 ? &matrix->prev->matrix : 0;
            for (idx = 0; idx < 3; ++idx)
            {
                definition_matrix = animated->path_definition->transform != 0 ? &animated->path_definition->transform->matrix : NULL;
                x = cur_matrix->t[idx];
                y = definition_matrix->t[idx];
                cur_matrix->t[idx] = x + y;
            }
        }
        else
        {
            matrix = render_view.matrix_cursor;
            cur_matrix = &matrix->matrix;
            parent_matrix = matrix->prev != 0 ? &matrix->prev->matrix : 0;
            x = animated->pos.x;
            y = animated->pos.y;
            z = animated->pos.z;
            cur_matrix->t[0] = x;
            cur_matrix->t[1] = y;
            cur_matrix->t[2] = z;
        }
        qw = animated->orientation[0];
        if (qw != 0x7FFF)
        {
            qx = animated->orientation[1];
            qy = animated->orientation[2];
            qz = animated->orientation[3];
            xx = qx * qx;
            yy = qy * qy;
            zz = qz * qz;
            xy = qx * qy;
            xz = qx * qz;
            yz = qy * qz;
            wz = qw * qz;
            matrix = render_view.matrix_cursor;
            cur_matrix = &matrix->matrix;
            parent_matrix = matrix->prev != 0 ? &matrix->prev->matrix : 0;
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
            rotation_matrix = &render_view.matrix_cursor->matrix;
        }
        else
        {
            matrix = render_view.matrix_cursor;
            cur_matrix = &matrix->matrix;
            parent_matrix = matrix->prev != 0 ? &matrix->prev->matrix : 0;
            SetRotMatrix(parent_matrix);
            if (animated->orientation[1] != 0)
            {
                c = SinQ14((1024 - (uint16)animated->orientation[2])) >> 2;
                s = SinQ14(animated->orientation[2]) >> 2;
                rotation_y.m[0][0] = c;
                rotation_y.m[2][0] = -s;
                rotation_y.m[0][2] = s;
                rotation_y.m[2][2] = c;
                MulRotMatrix(&rotation_y);
                SetRotMatrix(&rotation_y);
                c = SinQ14((1024 - (uint16)animated->orientation[1])) >> 2;
                s = SinQ14(animated->orientation[1]) >> 2;
                rotation_x.m[1][1] = c;
                rotation_x.m[2][1] = s;
                rotation_x.m[1][2] = -s;
                rotation_x.m[2][2] = c;
                MulRotMatrix0(&rotation_x, cur_matrix);
                rotation_matrix = &render_view.matrix_cursor->matrix;
            }
            else if (animated->orientation[2] != 0)
            {
                c = SinQ14((1024 - (uint16)animated->orientation[2])) >> 2;
                s = SinQ14(animated->orientation[2]) >> 2;
                rotation_y.m[0][0] = c;
                rotation_y.m[2][0] = -s;
                rotation_y.m[0][2] = s;
                rotation_y.m[2][2] = c;
                MulRotMatrix0(&rotation_y, cur_matrix);
                rotation_matrix = &render_view.matrix_cursor->matrix;
            }
            else
            {
                if (cur_matrix != parent_matrix)
                    memcpy(cur_matrix->m, parent_matrix->m, sizeof(cur_matrix->m));

                goto rotation_ready;
            }
        }
        SetRotMatrix(rotation_matrix);
    rotation_ready:
        special = animated->behavior == 17 && animated->behavior_suspended == 2;
        render_view.special_transform = special;
        if (special == 0 && (animated->movement_mode >= 4 || animated->behavior_suspended == 2) && (animated->flags & 0x30000000) != 0)
        {
            scale = (uint16)animated->state;
            if (scale == 0)
                xport_mips_break(7);
            rcp = scale == 0 ? 0xFFFFFFFF : 0x1000000 / scale;
            render_view.object_scale = scale;
            render_view.scale_rcp = rcp;
            matrix = render_view.matrix_cursor;
            cur_matrix = &matrix->matrix;
            parent_matrix = matrix->prev != 0 ? &matrix->prev->matrix : 0;
            for (idx = 0; idx < 9; ++idx)
            {
                if (idx != 0)
                    scale = render_view.object_scale;
                coefficient = cur_matrix->m[idx % 3][idx / 3];
                cur_matrix->m[idx % 3][idx / 3] = mul12((sint32)scale, coefficient);
            }
            rotation_matrix = &render_view.matrix_cursor->matrix;
            SetRotMatrix(rotation_matrix);
        }
        flags = animated->flags;
        byte_flags = animated->node.flags;
        animated->flags = flags | 0x200;
        if ((byte_flags & 3) == 0 && (CameraProjection::IsSphereVisible(animated->world_pos.comp, animated->bounds.radius) << 24) != 0)
        {
            MaterialAnimationList::UpdateMaterialAnimationList(animated->material_animation_entries);
            if ((animated->flags & 0x40000) != 0)
                scene_lighting.skip_transform = 1;
            matrix = render_view.matrix_cursor;
            cur_matrix = &matrix->matrix;
            parent_matrix = matrix->prev != 0 ? &matrix->prev->matrix : 0;
            radius = animated->bounds.radius;
            SceneLighting::SetLightingBounds(cur_matrix->t, radius >> 5);
            flags = animated->flags;
            variant = animated->behavior_suspended;
            animated->flags = flags & 0xFFFFFDFF;
            if (variant == 2 && render_view.special_transform == 0)
            {
                matrix = render_view.matrix_cursor;
                cur_matrix = &matrix->matrix;
                parent_matrix = matrix->prev != 0 ? &matrix->prev->matrix : 0;
                for (idx = 0; idx < 9; ++idx)
                {
                    scale = animated->velocity.x;
                    coefficient = cur_matrix->m[idx % 3][idx / 3];
                    cur_matrix->m[idx % 3][idx / 3] = mul12((sint32)scale, coefficient);
                }
                SetRotMatrix(cur_matrix);
            }
            render_view.object_depth_bias = animated->node.state_flags;
            if (animated->mesh_span.polygon_count != 0)
                animated->DrawGeometry(&animated->mesh_span);
            spr = (SceneNode *)animated->spr;
            if (spr != 0)
            {
                if (animated->behavior_suspended == 2 || animated->behavior == 17)
                    spr->flags |= 1;
                else
                    animated->UpdateGroundShadow();
            }
        }
        if ((animated->node.flags & 2) != 0)
        {
            object->Unregister();
            SceneNode::ReleaseSceneObject(object);
        }
        else
        {
            animated->UpdateObjectBounds();
            if ((animated->flags & 0x20200) != 0x200)
            {
                render_view.sphere_output = animated->spheres;
                animated->node.DrawChildren(animated);
                byte_flags = animated->node.flags;
                flags = animated->flags;
                animated->node.flags = byte_flags & 0xBF;
                animated->flags = flags & 0xFFFDFFFF;
                render_view.object_depth_bias = 0;
                flags = animated->flags;
                sphere_output = render_view.sphere_output;
                animated->flags = flags & 0xFFFFFFFE;
                if (sphere_output != animated->spheres)
                {
                    x = animated->world_pos.x;
                    xx = animated->bounds.maximum.x;
                    y = animated->world_pos.y;
                    yy = animated->bounds.maximum.y;
                    z = animated->world_pos.z;
                    zz = animated->bounds.maximum.z;
                    animated->world_pos.x = (sint32)(x + xx) >> 1;
                    animated->world_pos.y = (sint32)(y + yy) >> 1;
                    animated->world_pos.z = (sint32)(z + zz) >> 1;
                    xx = animated->bounds.maximum.x;
                    x = animated->world_pos.x;
                    yy = animated->bounds.maximum.y;
                    y = animated->world_pos.y;
                    zz = animated->bounds.maximum.z;
                    z = animated->world_pos.z;
                    animated->bounds.radius = xx - x;
                    animated->bounds.height = yy - y;
                    animated->bounds.depth = zz - z;
                    x = animated->bounds.radius;
                    y = animated->bounds.height;
                    animated->bounds.radius = ApproxVectorLen(x, y, zz - z);
                }
            }
            animated->node.state_flags = 0;
            if (render_view.material_override == resource_grps[25].tex)
                animated->velocity.x = 0;
            if (saved_ot != 0)
            {
                ScreenRenderState::DrawSceneFade();
                render_commands.ordering_table_offset = saved_ot;
                render_view.fog_scale = saved_fog;
            }
            flags = animated->flags;
            variant = animated->contact_kind;
            animated->flags = flags & 0xFFFBFFFF;
            if (variant != 0)
                animated->path_motion_scale = 4096;
            kind = animated->movement_mode;
            if (kind != 0)
            {
                const ObjectCallback operation = object_operations.motions[kind];
                if (operation.member_handler)
                    (animated->*operation.member_handler)();
                else if (operation.void_member_handler)
                    (animated->*operation.void_member_handler)();
                else if (operation.factory_handler)
                    operation.factory_handler(animated);
                else
                    operation.plain_handler();
            }
        }
        flags = game_flow.transition_flags;
        object = next;
        render_view.special_transform = 0;
        render_view.material_override = NULL;
        render_view.object_scale = 0;
        scene_lighting.skip_transform = 0;
        game_flow.transition_flags = flags & 0xFFFFFFDA;
    }
    render_view.matrix_cursor = render_view.matrix_cursor->prev;
}

void CameraProjection::BuildCameraView(SceneSection *scene)
{
    MATRIX identity = psx_identity_matrix;
    MATRIX x_basis, xy_basis;
    MATRIX *left, *right;
    VECTOR camera_pos, rotated_pos;

    FrustumPlane *plane;
    uint32 idx;
    uint32 point;
    uint32 distance;
    uint32 old_distance;
    uint32 difference;
    uint32 sum;
    uint32 x;
    uint32 y;
    uint32 z;
    uint16 transpose[9];
    sint16 c;
    sint16 s;
    sint32 quotient;
    sint32 coefficient;
    sint64 product;
    uint8 flags;
    FUNCTION_MARKER(0x800165C8, "SLUS_005.78");
    MaterialRegistry::UpdateResourceMaterialAnimations();
    SceneLighting::UpdateSceneLights();
    flags = camera_motion.flags;
    if ((flags & 1) == 0)
    {
        camera_motion.flags = flags | 1;
        CameraMotion::UpdateCameraAim();
    }
    c = SinQ14((camera_proj.rotation[0] + 1024)) >> 2;
    s = SinQ14((-camera_proj.rotation[0])) >> 2;
    rotation_x.m[1][1] = c;
    rotation_x.m[2][1] = s;
    rotation_x.m[1][2] = -s;
    rotation_x.m[2][2] = c;
    MulMatrix0(&identity, &rotation_x, &x_basis);
    c = SinQ14((camera_proj.rotation[1] + 1024)) >> 2;
    s = SinQ14((-camera_proj.rotation[1])) >> 2;
    rotation_y.m[0][0] = c;
    rotation_y.m[2][0] = -s;
    rotation_y.m[0][2] = s;
    rotation_y.m[2][2] = c;
    right = &rotation_y;
    if (camera_proj.rotation[2] == 2048)
        left = &x_basis;
    else
    {
        MulMatrix0(&x_basis, right, &xy_basis);
        c = SinQ14((camera_proj.rotation[2] - 1024)) >> 2;
        s = SinQ14((2048 - camera_proj.rotation[2])) >> 2;
        rotation_z.m[0][0] = c;
        rotation_z.m[1][0] = s;
        rotation_z.m[0][1] = -s;
        rotation_z.m[1][1] = c;
        left = &rotation_z;
        right = &xy_basis;
    }
    MulMatrix0(left, right, &render_view.camera_transform);
    SetRotMatrix(&render_view.camera_transform);
    camera_pos.vx = camera_motion.pos.x;
    camera_pos.vy = camera_motion.pos.y;
    camera_pos.vz = camera_motion.pos.z;
    camera_pos.pad = 0;
    ApplyRotMatrixLV(&camera_pos, &rotated_pos);
    x = rotated_pos.vx;
    y = rotated_pos.vy;
    z = rotated_pos.vz;
    render_view.camera_transform.t[0] = -x;
    render_view.camera_transform.t[1] = -y;
    render_view.camera_transform.t[2] = -z;
    transpose[0] = render_view.camera_transform.m[0][0];
    transpose[1] = render_view.camera_transform.m[1][0];
    transpose[2] = render_view.camera_transform.m[2][0];
    transpose[3] = render_view.camera_transform.m[0][1];
    transpose[4] = render_view.camera_transform.m[1][1];
    transpose[5] = render_view.camera_transform.m[2][1];
    transpose[6] = render_view.camera_transform.m[0][2];
    transpose[7] = render_view.camera_transform.m[1][2];
    transpose[8] = render_view.camera_transform.m[2][2];
    for (idx = 0; idx < 9; ++idx)
        render_view.inverse_transform.m[idx / 3][idx % 3] = transpose[idx];
    memcpy(render_view.billboard_transform.m, render_view.inverse_transform.m, sizeof(render_view.billboard_transform.m));
    SetRotMatrix(&render_view.inverse_transform);
    for (point = 0; point < 5; ++point)
    {
        gte_ldv0(&camera_proj.view_normals[point]);
        xport_gte_mvmva(0x486012);
        render_view.transform_result.vx = xport_gte_read_data(25);
        render_view.transform_result.vy = xport_gte_read_data(26);
        render_view.transform_result.vz = xport_gte_read_data(27);
        plane = &render_view.planes[point];
        plane->n.vx = render_view.transform_result.vx;
        plane->n.vy = render_view.transform_result.vy;
        z = render_view.transform_result.vz;
        plane->n.vz = z;
        x = camera_motion.pos.x;
        coefficient = plane->n.vx;
        sum = (sint64)((sint32)x >> 6) * coefficient;
        y = camera_motion.pos.y;
        coefficient = plane->n.vy;
        sum += (uint32)((sint64)((sint32)y >> 6) * coefficient);
        x = camera_motion.pos.z;
        sum += (uint32)((sint64)((sint32)x >> 6) * (sint16)z);
        plane->bias = -(uint32)((sint32)sum >> 6);
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
    {
        MATRIX *root_matrix = &render_view.matrix_cursor->matrix;
        memcpy(root_matrix->m, psx_identity_matrix.m, sizeof(root_matrix->m));
        root_matrix = &render_view.matrix_cursor->matrix;
        root_matrix->t[2] = 0;
        root_matrix->t[1] = 0;
        root_matrix->t[0] = 0;
    }
    distance = camera_proj.far_distance;
    old_distance = render_view.far_distance;
    if (distance != old_distance)
    {
        render_view.far_distance = distance;
        if (distance == 0)
            xport_mips_break(7);
        quotient = 0x40000000 / (sint32)distance;
        product = (sint64)0x400000 * quotient;
        coefficient = product >> 18;
        fixed_math_result = coefficient;
        render_view.fog_scale = coefficient;
    }
    if (scene != 0)
    {
        distance = scene->fog_far;
        if (distance != 0)
        {
            old_distance = scene->fog_near;
            difference = distance - old_distance;
            if (difference == 0)
                xport_mips_break(7);
            quotient = 0x40000000 / (sint32)difference;
            product = (sint64)0x100000 * quotient;
            coefficient = product >> 18;
            fixed_math_result = coefficient;
            render_view.depth_scale = coefficient;
        }
        else
            xport_gte_write_data(8, 0);
        if (scr_render.fade_level < 4096)
        {
            render_view.lighting_enabled = true;
            render_view.lighting_cache = NULL;
            CameraProjection::TraverseSceneObjects(scene, 0);
            MeshGeometryNode::DrawSceneGeometry();
        }
    }
}

void CameraProjection::TraverseSceneObjects(SceneSection *scene, uint32 mode)
{
    FUNCTION_MARKER(0x80016C54, "SLUS_005.78");
    uint32 i;
    MATRIX *matrix;
    for (i = 0; i < 5; ++i)
        xport_gte_write_control(i, psx_identity_rotation_words[i]);
    matrix = &render_view.matrix_cursor->matrix;
    memcpy(matrix->m, psx_identity_matrix.m, sizeof(matrix->m));
    matrix = &render_view.matrix_cursor->matrix;
    matrix->t[2] = 0;
    matrix->t[1] = 0;
    matrix->t[0] = 0;
    scene->links.UpdateGraph(0, mode);
}

uint32 CameraProjection::UpdatePlayerFrame(void)
{
    FUNCTION_MARKER(0x80016D0C, "SLUS_005.78");
    ObjectCallback object_operation;
    AnimatedObject *player = CharacterController::FindPlayerObject(0);
    uint32 val, i;
    SceneSection *scene;
    MATRIX *matrix;
    sint32 kind;
    if (player != 0)
    {
        kind = player->behavior_suspended;
        if (kind != 0)
            object_operation = object_operations.variants[kind];
        else
        {
            object_operation = object_operations.behaviors[player->behavior];
        }

        {
            const ObjectCallback operation = object_operation;
            if (operation.member_handler)
                (player->*operation.member_handler)();
            else if (operation.void_member_handler)
                (player->*operation.void_member_handler)();
            else if (operation.factory_handler)
                operation.factory_handler(player);
            else
                operation.plain_handler();
        }
        val = game_flow.transition_flags;
        if ((val & 0x48) != 0)
        {
            if ((val & 8) != 0)
                return 0;
            scene = scene_stream.active_section;
            if ((scene->state_flags & 8) == 0)
                return 0;
            if (scr_render.fade_level < 0x1000)
            {
                render_view.planes[0].n.vz = 0;
                render_view.planes[0].n.vy = 0;
                render_view.planes[0].n.vx = 0;
                render_view.planes[0].bias = 0x80001000;
                for (i = 0; i < 5; ++i)
                    xport_gte_write_control(i, psx_identity_rotation_words[i]);
                matrix = &render_view.matrix_cursor->matrix;
                memcpy(matrix->m, psx_identity_matrix.m, sizeof(matrix->m));
                matrix = &render_view.matrix_cursor->matrix;
                matrix->t[2] = 0;
                matrix->t[1] = 0;
                matrix->t[0] = 0;
                scene->links.UpdateGraph(0, 0);
            }
            return 0;
        }
    }
    if ((camera_motion.smooth_ticks[0] | camera_motion.smooth_ticks[1] | camera_motion.smooth_ticks[2]) != 0)
        CameraMotion::UpdateCameraPos();
    if (camera_motion.target_ticks != 0)
        CameraMotion::UpdateCameraTarget();
    return 1;
}

void CameraProjection::DrawSceneFrame(SceneSection *scene)
{
    ObjectCallback object_operation;
    SceneSection *section;
    SceneNode *node, *next;
    AnimatedObject *animated;
    uint32 result;

    sint32 kind;
    FUNCTION_MARKER(0x80016EF4, "SLUS_005.78");
    if (scene != 0)
    {
        result = CameraProjection::UpdatePlayerFrame() << 24;
        if (result == 0)
            return;
    }
    render_view.first_proj_vertex = 0;
    RenderCommands::BeginDraw();
    CameraProjection::BuildCameraView(scene);
    CameraProjection::UpdateAndDrawRegisteredObjects();
    if (scene != 0)
    {
        ScreenRenderState::DrawSceneInterface();
        section = scene;
        node = section->transient_links.first_child;
        if (node != 0)
        {
            do
            {
                next = node->links.sibling_next;
                section->transient_links.DetachChild(node);
                if (node->kind != 0)
                {
                    node->links.sibling_next = section->links.first_child;
                    section->links.first_child = node;
                }
                else
                {
                    animated = (AnimatedObject *)node;
                    node->Register();
                    if (animated->contact_kind != 0)
                        animated->path_motion_scale = 4096;
                    kind = animated->movement_mode;
                    if (kind != 0)
                    {
                        object_operation = object_operations.motions[kind];
                        {
                            const ObjectCallback operation = object_operation;
                            if (operation.member_handler)
                                (animated->*operation.member_handler)();
                            else if (operation.void_member_handler)
                                (animated->*operation.void_member_handler)();
                            else if (operation.factory_handler)
                                operation.factory_handler(animated);
                            else
                                operation.plain_handler();
                        }
                    }
                }
                node = next;
            } while (next != 0);
            section->transient_links.first_child = 0;
        }
    }
    RenderCommands::EndDraw();
}

void CameraProjection::SetCameraProj(sint32 scale)
{
    FUNCTION_MARKER(0x8002417C, "SLUS_005.78");
    uint32 point;
    sint32 ax;
    sint32 ay;
    sint32 az;
    sint32 bx;
    sint32 by;
    sint32 bz;
    uint32 product0;
    uint32 product1;
    uint32 idx0, idx1;
    Vec3 vertices[4];
    VECTOR n;
    sint32 proj = scale >> 12;
    camera_proj.view_normals[0].vz = 4096;
    camera_proj.view_normals[0].vx = 0;
    camera_proj.view_normals[0].vy = 0;
    vertices[0].x = 0;
    vertices[0].y = 0;
    vertices[0].z = 0;
    vertices[1].x = -40960;
    vertices[1].y = 30720;
    vertices[1].z = scale >> 4;
    vertices[2].x = -40960;
    vertices[2].y = -30720;
    vertices[2].z = scale >> 4;
    for (point = 0; point < 4; ++point)
    {
        if (point == 1)
        {
            vertices[1].x = 40960;
            vertices[1].y = -30720;
        }
        else if (point == 2)
        {
            vertices[2].x = 40960;
            vertices[2].y = 30720;
        }
        else if (point == 3)
        {
            vertices[1].x = -40960;
            vertices[1].y = 30720;
        }
        idx0 = (point & 1) != 0 ? 2 : 1;
        idx1 = (point & 1) != 0 ? 1 : 2;
        ax = vertices[idx0].x - (uint32)vertices[0].x;
        ay = vertices[idx0].y - (uint32)vertices[0].y;
        az = vertices[idx0].z - (uint32)vertices[0].z;
        bx = vertices[idx1].x - (uint32)vertices[idx0].x;
        by = vertices[idx1].y - (uint32)vertices[idx0].y;
        bz = vertices[idx1].z - (uint32)vertices[idx0].z;
        product0 = mul12(ay, bz);
        product1 = mul12(by, az);
        n.vx = product0 - product1;
        product0 = mul12(az, bx);
        product1 = mul12(bz, ax);
        n.vy = product0 - product1;
        product0 = mul12(ax, by);
        product1 = mul12(bx, ay);
        n.vz = product0 - product1;
        while ((uint32)n.vx + 26754 > 0xD103 || (uint32)n.vy + 26754 > 0xD103 || (uint32)n.vz + 26754 > 0xD103)
        {
            n.vx >>= 1;
            n.vy >>= 1;
            n.vz >>= 1;
        }
        VectorNormal(&n, &n);
        camera_proj.view_normals[point + 1].vx = n.vx;
        camera_proj.view_normals[point + 1].vy = n.vy;
        camera_proj.view_normals[point + 1].vz = n.vz;
    }
    xport_gte_write_control(26, proj);
    xport_gte_write_control(21, 0);
    xport_gte_write_control(22, 0);
    xport_gte_write_control(23, 0);

    SetFogNearFar(0x8000, 0xFFFF, proj);
}

uint32 CameraProjection::ResetSceneRendering(void)
{
    FUNCTION_MARKER(0x80060EEC, "SLUS_005.78");
    MaterialRegistry::ResetMaterialRegistry();
    SpriteRecord::InitializeSprPool();
    {
        ObjectCallback operation = {AnimatedObject::EmptyObjectBehavior};
        object_operations.behaviors[0] = operation;
    }
    {
        ObjectCallback operation = {0, &AnimatedObject::UpdateSceneAnimation};
        object_operations.behaviors[3] = operation;
    }
    {
        ObjectCallback operation = {0, &AnimatedObject::UpdateAnimatedObjectCommands};
        object_operations.behaviors[4] = operation;
    }
    {
        ObjectCallback operation = {0, &AnimatedObject::FollowSceneAnimation};
        object_operations.behaviors[31] = operation;
    }
    {
        ObjectCallback operation = {0, &AnimatedObject::UpdateSceneScriptVariable};
        object_operations.behaviors[32] = operation;
    }
    camera_proj.view_normals[0].vz = 0;
    return 1;
}
