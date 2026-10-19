#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "game.h"
#include "original_defaults.h"

void UpdateAndDrawRegisteredObjects(void)
{
    ObjectCallback object_operation;
    AnimatedObject *animated;
    VECTOR position;
    SceneNode *object, *next;
    CollisionSphere *sphere_output;

    RenderMatrixSlot *matrix;
    MATRIX *current_matrix, *rotation_matrix;
    const MATRIX *parent_matrix;
    const MATRIX *definition_matrix;
    const PathDefinition *definition;
    SceneNode *sprite;

    uint32 target;
    uint32 flags;
    uint32 saved_ot;
    uint32 saved_fog;
    uint32 radius;
    uint32 scale;
    uint32 reciprocal;
    uint32 special;
    uint32 x;
    uint32 y;
    uint32 z;
    uint32 value;
    uint32 xx;
    uint32 yy;
    uint32 zz;
    uint32 xy;
    uint32 xz;
    uint32 yz;
    uint32 wz;
    uint32 wy;
    uint32 wx;
    uint32 words[4];
    uint32 index;
    sint32 kind;
    sint32 variant;
    sint32 mode;
    sint32 qw;
    sint32 qx;
    sint32 qy;
    sint32 qz;
    sint32 cosine;
    sint32 sine;
    sint32 coefficient;
    sint64 product;
    uint8 byte_flags;
    FUNCTION_MARKER(0x80012EB8, "SLUS_005.78");
    const ActorModelDefinition *bound_model;
    object = registered_object_head;
    matrix = render_view.matrix_cursor;
    current_matrix = &matrix->matrix;
    parent_matrix = (matrix->prev != 0 ? &matrix->prev->matrix : 0);
    render_view.lighting_cache = NULL;
    render_view.lighting_enabled = false;
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
    while (object != 0)
    {
        animated = (AnimatedObject *)object;
        kind = animated->behavior;
        saved_ot = 0;
        if (kind != 1)
        {
            variant = (sint8)animated->metadata126[1];
            if (variant != 0)
                object_operation = object_operations.variants[variant];
            else
                object_operation = object_operations.behaviors[kind];
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
                goto red_tint;
            if ((animated->path_flags & 0x4000) != 0)
            {
                bound_model = ((const ActorBinding *)(((const ActorPlacement *)((const PathDefinition *)((animated->path_definition))))->state != 0 ? ((const ActorPlacement *)((const PathDefinition *)((animated->path_definition))))->state : 0))->model;
                flags = game_flow.transition_flags;
                z = bound_model->fog_color[2];
                y = bound_model->fog_color[1];
                x = (uint32)(sint32)((sint8)bound_model->fog_color[0]);
                game_flow.transition_flags = flags | 0x20;
                render_view.fog_color = (z << 16) | (y << 8) | x;
            }
        }
        else
        {
            value = screen_render.fade_level;
            if (value != 0)
            {
                if (value < 4096)
                {
                    saved_ot = render_commands.ordering_table_offset;
                    saved_fog = render_view.fog_scale;
                    target = render_commands.small_ordering_table_offset;
                    render_view.fog_scale = 1310;
                    render_commands.ordering_table_offset = (target);
                }
            }
            else
            {
                if ((IsAlternateCharacterInteractionActive() << 24) != 0)
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
                        render_view.material_override = resource_groups[26].texture;
                    else if (mode == 5)
                    {
                        x = character_controller.appearance.tint;
                        flags = game_flow.transition_flags;
                        render_view.fog_color = x;
                        game_flow.transition_flags = flags | 0x20;
                        render_view.material_override = resource_groups[25].texture;
                    }
                }
                else if ((animated->flags & 0x80000) != 0)
                {
                red_tint:
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
            definition = (animated->path_definition);
            if (definition != 0)
            {
                const PathTransform *captured_path_matrix = (definition->transform);
                definition_matrix = captured_path_matrix != 0 ? (const MATRIX *)(captured_path_matrix != 0 ? &captured_path_matrix->matrix : (MATRIX *)0) : 0;
            }
        }
        if (definition_matrix != 0)
        {
            x = animated->position[0];
            y = animated->position[1];
            z = animated->position[2];
            position.vx = x;
            position.vy = y;
            position.vz = z;
            PushMatrix();
            definition_matrix = (const MATRIX *)((((const PathDefinition *)((const PathDefinition *)((animated->path_definition))))->transform) != 0 ? &(((const PathDefinition *)((const PathDefinition *)((animated->path_definition))))->transform)->matrix : (MATRIX *)0);
            x = xport_load_le32((const uint8 *)definition_matrix);
            y = xport_load_le32((const uint8 *)definition_matrix + 4);
            xport_gte_write_control(0, x);
            xport_gte_write_control(1, y);
            x = xport_load_le32((const uint8 *)definition_matrix + 8);
            y = xport_load_le32((const uint8 *)definition_matrix + 12);
            z = xport_load_le32((const uint8 *)definition_matrix + 16);
            xport_gte_write_control(2, x);
            xport_gte_write_control(3, y);
            xport_gte_write_control(4, z);
            MATRIX *position_matrix = &render_view.matrix_cursor->matrix;
            ApplyRotMatrixLV(&position, (VECTOR *)position_matrix->t);
            PopMatrix();
            matrix = render_view.matrix_cursor;
            current_matrix = &matrix->matrix;
            parent_matrix = (matrix->prev != 0 ? &matrix->prev->matrix : 0);
            for (index = 0; index < 3; ++index)
            {
                definition_matrix = (const MATRIX *)((((const PathDefinition *)((const PathDefinition *)((animated->path_definition))))->transform) != 0 ? &(((const PathDefinition *)((const PathDefinition *)((animated->path_definition))))->transform)->matrix : (MATRIX *)0);
                x = (uint32)current_matrix->t[index];
                y = (uint32)definition_matrix->t[index];
                current_matrix->t[index] = (sint32)(x + y);
            }
        }
        else
        {
            matrix = render_view.matrix_cursor;
            current_matrix = &matrix->matrix;
            parent_matrix = (matrix->prev != 0 ? &matrix->prev->matrix : 0);
            x = animated->position[0];
            y = animated->position[1];
            z = animated->position[2];
            current_matrix->t[0] = (sint32)(x);
            current_matrix->t[1] = (sint32)(y);
            current_matrix->t[2] = (sint32)(z);
        }
        qw = animated->orientation[0];
        if (qw != 0x7FFF)
        {
            qx = animated->orientation[1];
            qy = animated->orientation[2];
            qz = animated->orientation[3];
            xx = (uint32)(qx * qx);
            yy = (uint32)(qy * qy);
            zz = (uint32)(qz * qz);
            xy = (uint32)(qx * qy);
            xz = (uint32)(qx * qz);
            yz = (uint32)(qy * qz);
            wz = (uint32)(qw * qz);
            matrix = render_view.matrix_cursor;
            current_matrix = &matrix->matrix;
            parent_matrix = (matrix->prev != 0 ? &matrix->prev->matrix : 0);
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
            rotation_matrix = &render_view.matrix_cursor->matrix;
        }
        else
        {
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
            if (animated->orientation[1] != 0)
            {
                cosine = SinQ14((sint16)(1024 - (uint16)animated->orientation[2])) >> 2;
                sine = SinQ14(animated->orientation[2]) >> 2;
                rotation_y.m[0][0] = (sint16)((uint16)cosine);
                rotation_y.m[2][0] = (sint16)((uint16)(0 - (uint32)sine));
                rotation_y.m[0][2] = (sint16)((uint16)sine);
                rotation_y.m[2][2] = (sint16)((uint16)cosine);
                MulRotMatrix(&rotation_y);
                x = xport_load_le32((const uint8 *)&rotation_y);
                y = xport_load_le32((const uint8 *)&rotation_y + 4);
                xport_gte_write_control(0, x);
                xport_gte_write_control(1, y);
                x = xport_load_le32((const uint8 *)&rotation_y + 8);
                y = xport_load_le32((const uint8 *)&rotation_y + 12);
                z = xport_load_le32((const uint8 *)&rotation_y + 16);
                xport_gte_write_control(2, x);
                xport_gte_write_control(3, y);
                xport_gte_write_control(4, z);
                cosine = SinQ14((sint16)(1024 - (uint16)animated->orientation[1])) >> 2;
                sine = SinQ14(animated->orientation[1]) >> 2;
                rotation_x.m[1][1] = (sint16)((uint16)cosine);
                rotation_x.m[2][1] = (sint16)((uint16)sine);
                rotation_x.m[1][2] = (sint16)((uint16)(0 - (uint32)sine));
                rotation_x.m[2][2] = (sint16)((uint16)cosine);
                MulRotMatrix0(&rotation_x, current_matrix);
                rotation_matrix = &render_view.matrix_cursor->matrix;
            }
            else if (animated->orientation[2] != 0)
            {
                cosine = SinQ14((sint16)(1024 - (uint16)animated->orientation[2])) >> 2;
                sine = SinQ14(animated->orientation[2]) >> 2;
                rotation_y.m[0][0] = (sint16)((uint16)cosine);
                rotation_y.m[2][0] = (sint16)((uint16)(0 - (uint32)sine));
                rotation_y.m[0][2] = (sint16)((uint16)sine);
                rotation_y.m[2][2] = (sint16)((uint16)cosine);
                MulRotMatrix0(&rotation_y, current_matrix);
                rotation_matrix = &render_view.matrix_cursor->matrix;
            }
            else
            {
                for (index = 0; index < 4; ++index)
                    words[index] = xport_load_le32((const uint8 *)parent_matrix + 4 * index);
                for (index = 0; index < 4; ++index)
                    xport_store_le32((uint8 *)current_matrix + 4 * index, words[index]);
                current_matrix->m[2][2] = (sint16)((uint16)parent_matrix->m[2][2]);
                goto rotation_ready;
            }
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
    rotation_ready:
        special = animated->behavior == 17 && animated->metadata126[1] == 2;
        render_view.special_transform = special;
        if (special == 0 && (animated->movement_mode >= 4 || (sint8)animated->metadata126[1] == 2) && (animated->flags & 0x30000000) != 0)
        {
            scale = (uint16)animated->state;
            if (scale == 0)
                xport_mips_break(7);
            reciprocal = scale == 0 ? 0xFFFFFFFF : 0x1000000 / scale;
            render_view.object_scale = scale;
            render_view.scale_reciprocal = reciprocal;
            matrix = render_view.matrix_cursor;
            current_matrix = &matrix->matrix;
            parent_matrix = (matrix->prev != 0 ? &matrix->prev->matrix : 0);
            for (index = 0; index < 9; ++index)
            {
                if (index != 0)
                    scale = render_view.object_scale;
                coefficient = current_matrix->m[index % 3][index / 3];
                product = (sint64)(sint32)scale * coefficient;
                current_matrix->m[index % 3][index / 3] = (sint16)((uint64)product >> 12);
            }
            rotation_matrix = &render_view.matrix_cursor->matrix;
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
        }
        flags = animated->flags;
        byte_flags = animated->node.flags;
        animated->flags = flags | 0x200;
        if ((byte_flags & 3) == 0 && (IsSphereVisible(animated->world_position, animated->metadata_words72[0]) << 24) != 0)
        {
            UpdateMaterialAnimationList(animated->material_animation_entries);
            if ((animated->flags & 0x40000) != 0)
                scene_lighting.skip_transform = 1;
            matrix = render_view.matrix_cursor;
            current_matrix = &matrix->matrix;
            parent_matrix = (matrix->prev != 0 ? &matrix->prev->matrix : 0);
            radius = animated->metadata_words72[0];
            SetLightingBounds((const uint32 *)current_matrix->t, (sint32)radius >> 5);
            flags = animated->flags;
            variant = (sint8)animated->metadata126[1];
            animated->flags = flags & 0xFFFFFDFF;
            if (variant == 2 && (sint8)render_view.special_transform == 0)
            {
                matrix = render_view.matrix_cursor;
                current_matrix = &matrix->matrix;
                parent_matrix = (matrix->prev != 0 ? &matrix->prev->matrix : 0);
                for (index = 0; index < 9; ++index)
                {
                    scale = animated->velocity[0];
                    coefficient = current_matrix->m[index % 3][index / 3];
                    product = (sint64)(sint32)scale * coefficient;
                    current_matrix->m[index % 3][index / 3] = (sint16)((uint64)product >> 12);
                }
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
            render_view.object_depth_bias = animated->node.state_flags;
            if (animated->mesh_span.polygon_count != 0)
                animated->DrawGeometry(&animated->mesh_span);
            sprite = (SceneNode *)(animated->sprite);
            if (sprite != 0)
            {
                if ((sint8)animated->metadata126[1] == 2 || animated->behavior == 17)
                    sprite->flags |= 1;
                else
                    animated->UpdateGroundShadow();
            }
        }
        if ((animated->node.flags & 2) != 0)
        {
            object->Unregister();
            ReleaseSceneObject(object);
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
                    x = animated->world_position[0];
                    xx = animated->metadata_words72[0];
                    y = animated->world_position[1];
                    yy = animated->metadata_words72[1];
                    z = animated->world_position[2];
                    zz = animated->metadata_words72[2];
                    animated->world_position[0] = (uint32)((sint32)(x + xx) >> 1);
                    animated->world_position[1] = (uint32)((sint32)(y + yy) >> 1);
                    animated->world_position[2] = (uint32)((sint32)(z + zz) >> 1);
                    xx = animated->metadata_words72[0];
                    x = animated->world_position[0];
                    yy = animated->metadata_words72[1];
                    y = animated->world_position[1];
                    zz = animated->metadata_words72[2];
                    z = animated->world_position[2];
                    animated->metadata_words72[0] = xx - x;
                    animated->metadata_words72[1] = yy - y;
                    animated->metadata_words72[2] = zz - z;
                    x = animated->metadata_words72[0];
                    y = animated->metadata_words72[1];
                    animated->metadata_words72[0] = ApproxVectorLength(x, y, zz - z);
                }
            }
            animated->node.state_flags = 0;
            if (render_view.material_override == resource_groups[25].texture)
                animated->velocity[0] = 0;
            if (saved_ot != 0)
            {
                DrawSceneFade();
                render_commands.ordering_table_offset = (saved_ot);
                render_view.fog_scale = saved_fog;
            }
            flags = animated->flags;
            variant = (sint8)animated->metadata126[0];
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

uint32 DrawSceneFrame(SceneSection *scene)
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
        result = UpdatePlayerFrame() << 24;
        if (result == 0)
            return result;
    }
    render_view.first_projected_vertex = 0;
    BeginDraw();
    BuildCameraView(scene);
    UpdateAndDrawRegisteredObjects();
    if (scene != 0)
    {
        DrawSceneInterface();
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
                    if ((sint8)animated->metadata126[0] != 0)
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
    return EndDraw();
}

uint32 UpdatePlayerFrame(void)
{
    FUNCTION_MARKER(0x80016D0C, "SLUS_005.78");
    ObjectCallback object_operation;
    AnimatedObject *player = FindPlayerObject(0);
    uint32 value, x, y, z, i;
    SceneSection *scene;
    MATRIX *matrix;
    sint32 kind;
    if (player != 0)
    {
        kind = (sint8)player->metadata126[1];
        if (kind != 0)
            object_operation = object_operations.variants[kind];
        else
            object_operation = object_operations.behaviors[player->behavior];
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
        value = game_flow.transition_flags;
        if ((value & 0x48) != 0)
        {
            if ((value & 8) != 0)
                return 0;
            scene = scene_stream.active_section;
            if (((scene)->state_flags & 8) == 0)
                return 0;
            if (screen_render.fade_level < 0x1000)
            {
                render_view.planes[0].normal.vz = (sint16)(0);
                render_view.planes[0].normal.vy = (sint16)(0);
                render_view.planes[0].normal.vx = (sint16)(0);
                render_view.planes[0].bias = 0x80001000;
                for (i = 0; i < 5; ++i)
                    xport_gte_write_control(i, psx_identity_rotation_words[i]);
                matrix = &render_view.matrix_cursor->matrix;
                x = psx_identity_rotation_words[0];
                y = psx_identity_rotation_words[1];
                z = psx_identity_rotation_words[2];
                xport_store_le32((uint8 *)matrix, x);
                xport_store_le32((uint8 *)matrix + 4, y);
                xport_store_le32((uint8 *)matrix + 8, z);
                value = psx_identity_rotation_words[3];
                x = (uint32)psx_identity_matrix.m[2][2];
                xport_store_le32((uint8 *)matrix + 12, value);
                matrix->m[2][2] = (sint16)(x);
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
        UpdateCameraPosition();
    if (camera_motion.target_ticks != 0)
        UpdateCameraTarget();
    return 1;
}

void TraverseSceneObjects(SceneSection *scene, uint32 mode)
{
    FUNCTION_MARKER(0x80016C54, "SLUS_005.78");
    uint32 i;
    MATRIX *matrix;
    uint32 first, second, third;
    for (i = 0; i < 5; ++i)
        xport_gte_write_control(i, psx_identity_rotation_words[i]);
    matrix = &render_view.matrix_cursor->matrix;
    first = psx_identity_rotation_words[0];
    second = psx_identity_rotation_words[1];
    third = psx_identity_rotation_words[2];
    xport_store_le32((uint8 *)matrix, first);
    xport_store_le32((uint8 *)matrix + 4, second);
    xport_store_le32((uint8 *)matrix + 8, third);
    first = psx_identity_rotation_words[3];
    second = (uint32)psx_identity_matrix.m[2][2];
    xport_store_le32((uint8 *)matrix + 12, first);
    matrix->m[2][2] = (sint16)(second);
    matrix = &render_view.matrix_cursor->matrix;
    matrix->t[2] = 0;
    matrix->t[1] = 0;
    matrix->t[0] = 0;
    scene->links.UpdateGraph(0, mode);
}
