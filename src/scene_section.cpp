#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "game.h"

void SceneSection::Relocate(const MaterialResourceHeader *resource)
{
    FUNCTION_MARKER(0x80068A24, "SLUS_005.78");
    uint32 node, value, i, group;
    uint32 node_budget;
    if (data == 0 && data_bytes != 0)
    {
        fprintf(stderr, "P2: Missing native scene input\n");
        abort();
    }
    uintptr_t source_begin = (uintptr_t)data;
    uint32 face_source_slots = data_bytes / sizeof(uint32);
    if (face_source_slots > 0xFFFFFFFFu / sizeof(const TextureDescriptor *))
    {
        fprintf(stderr, "P2: Scene polygon reference allocation overflows\n");
        abort();
    }
    const TextureDescriptor **source_face_textures = face_source_slots != 0 ? (const TextureDescriptor **)calloc(face_source_slots, sizeof(const TextureDescriptor *)) : 0;
    if (face_source_slots != 0 && source_face_textures == 0)
    {
        fprintf(stderr, "P2: Cannot allocate scene polygon references\n");
        abort();
    }
    uint32 animation_header_capacity = 0;
    uint32 *source_animation_offsets;
    uint32 actor_group_counts[2] = {0, 0};
    animated_object_count = 0;
    for (uint32 scan_group = 0; scan_group < 2; ++scan_group)
    {
        uint32 scan_node = object_heads[scan_group] != 0 ? ((uintptr_t)(object_heads[scan_group]) >= source_begin && ((uintptr_t)(object_heads[scan_group]) - source_begin) <= data_bytes && (sizeof(SceneNode)) <= data_bytes - ((uintptr_t)(object_heads[scan_group]) - source_begin) ? (uint32)((uintptr_t)(object_heads[scan_group]) - source_begin) : (fprintf(stderr, "P2: Scene source pointer exceeds native input\n"), abort(), 0u)) : 0;
        uint32 scan_budget = data_bytes / sizeof(uint32);
        while (scan_node != 0)
        {
            if (scan_budget == 0 || (scan_node & 3) != 0 || data_bytes < 232 || scan_node > data_bytes - 232)
            {
                fprintf(stderr, "P2: Scene animation source graph exceeds resource or is cyclic\n");
                abort();
            }
            --scan_budget;
            const uint8 *source_actor = (const uint8 *)((scan_node) <= data_bytes && (232) <= data_bytes - (scan_node) ? data + (scan_node) : (fprintf(stderr, "P2: Scene file span exceeds native input\n"), abort(), data));
            ++actor_group_counts[scan_group];
            ++animated_object_count;
            if (xport_load_le32(source_actor + 104) != 0)
                ++animation_header_capacity;
            uint32 next = xport_load_le32(source_actor);
            if (next != 0 && (next > data_bytes || data_bytes - next < 232))
            {
                fprintf(stderr, "P2: Scene animation source link exceeds resource\n");
                abort();
            }
            scan_node = next != 0 ? next : 0;
        }
    }
    if (animation_header_capacity > 0xFFFFFFFFu / sizeof(AnimationHeader) || animation_header_capacity > 0xFFFFFFFFu / sizeof(uint32))
    {
        fprintf(stderr, "P2: Scene animation header capacity overflows\n");
        abort();
    }
    animations = animation_header_capacity != 0 ? (AnimationHeader *)calloc(animation_header_capacity, sizeof(AnimationHeader)) : 0;
    source_animation_offsets = animation_header_capacity != 0 ? (uint32 *)malloc(animation_header_capacity * sizeof(uint32)) : 0;
    if (animation_header_capacity != 0 && (animations == 0 || source_animation_offsets == 0))
    {
        fprintf(stderr, "P2: Cannot allocate native scene animation headers\n");
        abort();
    }
    animation_count = 0;
    if (animated_object_count > 0xFFFFFFFFu / sizeof(AnimatedObject) || animated_object_count > 0xFFFFFFFFu / sizeof(uint32))
    {
        fprintf(stderr, "P2: Native animated scene allocation overflows\n");
        abort();
    }
    animated_objects = animated_object_count != 0 ? (AnimatedObject *)malloc(animated_object_count * sizeof(AnimatedObject)) : 0;
    uint32 *source_actor_offsets = animated_object_count != 0 ? (uint32 *)malloc(animated_object_count * sizeof(uint32)) : 0;
    if (animated_object_count != 0 && (animated_objects == 0 || source_actor_offsets == 0))
    {
        fprintf(stderr, "P2: Cannot allocate native animated scene objects\n");
        abort();
    }
    uint32 native_actor_index = 0;
    node = object_heads[0] != 0 ? ((uintptr_t)(object_heads[0]) >= source_begin && ((uintptr_t)(object_heads[0]) - source_begin) <= data_bytes && (sizeof(SceneNode)) <= data_bytes - ((uintptr_t)(object_heads[0]) - source_begin) ? (uint32)((uintptr_t)(object_heads[0]) - source_begin) : (fprintf(stderr, "P2: Scene source pointer exceeds native input\n"), abort(), 0u)) : 0;
    transient_links.first_child = 0;
    for (group = 0; group < 2; ++group)
    {
        uint32 group_first = native_actor_index;
        if (group != 0)
            node = object_heads[1] != 0 ? ((uintptr_t)(object_heads[1]) >= source_begin && ((uintptr_t)(object_heads[1]) - source_begin) <= data_bytes && (sizeof(SceneNode)) <= data_bytes - ((uintptr_t)(object_heads[1]) - source_begin) ? (uint32)((uintptr_t)(object_heads[1]) - source_begin) : (fprintf(stderr, "P2: Scene source pointer exceeds native input\n"), abort(), 0u)) : 0;
        node_budget = data_bytes / sizeof(uint32);
        while (node != 0)
        {
            if (node_budget == 0 || (node & 3) != 0 || data_bytes < 232 || node > data_bytes - 232)
            {
                fprintf(stderr, "P2: Animated scene node is outside payload or cyclic\n");
                abort();
            }
            --node_budget;
            const uint8 *encoded = (const uint8 *)((node) <= data_bytes && (232) <= data_bytes - (node) ? data + (node) : (fprintf(stderr, "P2: Scene file span exceeds native input\n"), abort(), data));
            source_actor_offsets[native_actor_index] = node;
            AnimatedObject *actor = animated_objects + native_actor_index;
            actor->node.links.registry_next = native_actor_index + 1 < group_first + actor_group_counts[group] ? &animated_objects[native_actor_index + 1].node : 0;
            actor->node.kind = encoded[12];
            actor->node.flags = encoded[13];
            actor->node.state_flags = xport_load_le16(encoded + 14);
            actor->flags = xport_load_le32(encoded + 28);
            actor->metadata32 = xport_load_le32(encoded + 32);
            actor->event_word = xport_load_le32(encoded + 108);
            actor->behavior_word = xport_load_le32(encoded + 112);
            actor->state = xport_load_le32(encoded + 116);
            actor->animation_binding.word = xport_load_le32(encoded + 120);
            actor->path_motion_scale = xport_load_le32(encoded + 144);
            actor->clone_bytes = xport_load_le32(encoded + 224);
            actor->path_flags = xport_load_le32(encoded + 228);
            actor->lighting_index = xport_load_le16(encoded + 48);
            actor->metadata58 = xport_load_le16(encoded + 58);
            actor->sound_voice = xport_load_le16(encoded + 138);
            actor->mesh_span.first_vertex = (sint16)xport_load_le16(encoded + 50);
            actor->mesh_span.vertex_count = (sint16)xport_load_le16(encoded + 52);
            actor->mesh_span.first_polygon = (sint16)xport_load_le16(encoded + 54);
            actor->mesh_span.polygon_count = (sint16)xport_load_le16(encoded + 56);
            actor->ticks = (sint16)xport_load_le16(encoded + 128);
            actor->frame = (sint16)xport_load_le16(encoded + 130);
            actor->path_frame = (sint16)xport_load_le16(encoded + 136);
            actor->yaw = (sint16)xport_load_le16(encoded + 148);
            actor->target_yaw = (sint16)xport_load_le16(encoded + 150);
            actor->pitch = (sint16)xport_load_le16(encoded + 152);
            actor->pitch_scale = (sint16)xport_load_le16(encoded + 154);
            for (uint32 component = 0; component < 3; ++component)
                actor->position[component] = xport_load_le32(encoded + 16 + component * 4);
            for (uint32 component = 0; component < 3; ++component)
                actor->world_position[component] = xport_load_le32(encoded + 60 + component * 4);
            for (uint32 component = 0; component < 3; ++component)
                actor->metadata_words72[component] = xport_load_le32(encoded + 72 + component * 4);
            for (uint32 component = 0; component < 3; ++component)
                actor->velocity[component] = xport_load_le32(encoded + 92 + component * 4);
            for (uint32 component = 0; component < 16; ++component)
                actor->auxiliary_words[component] = xport_load_le32(encoded + 160 + component * 4);
            for (uint32 component = 0; component < 4; ++component)
                actor->orientation[component] = (sint16)xport_load_le16(encoded + 84 + component * 2);
            actor->movement_mode = (sint8)encoded[124];
            actor->behavior = (sint8)encoded[125];
            actor->metadata126[0] = encoded[126];
            actor->metadata126[1] = encoded[127];
            actor->animation = 0;
            actor->unbound_animation_word = 0;
            actor->clone_storage_bytes = 0;
            actor->polygons = 0;
            actor->material_animation_entries = 0;
            SceneNode **link_fields[] = {&actor->node.links.sibling_next, &actor->node.links.first_child};
            for (i = 0; i < 2; ++i)
            {
                value = xport_load_le32(encoded + 4 + i * 4);
                if (value != 0 && (value > data_bytes || 16 > data_bytes - value))
                {
                    fprintf(stderr, "P2: Animated scene link exceeds resource\n");
                    abort();
                }
                *link_fields[i] = value != 0 ? (SceneNode *)((value) <= data_bytes && (16) <= data_bytes - (value) ? data + (value) : (fprintf(stderr, "P2: Scene file span exceeds native input\n"), abort(), data)) : 0;
            }
            value = xport_load_le32(encoded + 140);
            actor->mesh_state = value != 0 ? ((value) <= data_bytes && (sizeof(EncodedMorphRestorationHeader)) <= data_bytes - (value) ? data + (value) : (fprintf(stderr, "P2: Scene file span exceeds native input\n"), abort(), data)) : 0;
            value = xport_load_le32(encoded + 40);
            actor->vertices = value != 0 ? (MeshVertex *)((value) <= data_bytes && (sizeof(MeshVertex)) <= data_bytes - (value) ? data + (value) : (fprintf(stderr, "P2: Scene file span exceeds native input\n"), abort(), data)) : 0;
            value = xport_load_le32(encoded + 132);
            actor->path_definition = value != 0 ? (const PathDefinition *)((value) <= data_bytes && (sizeof(PathDefinition)) <= data_bytes - (value) ? data + (value) : (fprintf(stderr, "P2: Scene file span exceeds native input\n"), abort(), data)) : 0;
            value = xport_load_le32(encoded + 36);
            if (value != 0)
            {
                uint32 polygon = value;
                uint32 flags = actor->flags & 12;
                actor->polygons = (MeshFace *)((polygon) <= data_bytes && (sizeof(EncodedMeshFace)) <= data_bytes - (polygon) ? data + (polygon) : (fprintf(stderr, "P2: Scene file span exceeds native input\n"), abort(), data));
                if (flags != 8)
                {
                    sint32 face_count = (sint16)actor->metadata32;
                    if (face_count < 0 || (value & 3) != 0 || value > data_bytes || (uint32)face_count > (data_bytes - value) / sizeof(EncodedMeshFace))
                    {
                        fprintf(stderr, "P2: Animated scene polygons are outside payload\n");
                        abort();
                    }
                    uint32 count = (uint32)face_count;
                    while (count != 0)
                    {
                        const EncodedMeshFace *face = (const EncodedMeshFace *)((polygon) <= data_bytes && (sizeof(EncodedMeshFace)) <= data_bytes - (polygon) ? data + (polygon) : (fprintf(stderr, "P2: Scene file span exceeds native input\n"), abort(), data));
                        uint32 index = xport_load_le32(face);
                        if (index >= resource->texture_count)
                        {
                            fprintf(stderr, "P2: Scene polygon texture index is outside resource\n");
                            abort();
                        }
                        --count;
                        if (source_face_textures[(polygon) / sizeof(uint32)] != 0)
                        {
                            fprintf(stderr, "P2: Scene polygon was relocated twice\n");
                            abort();
                        }
                        source_face_textures[(polygon) / sizeof(uint32)] = resource->textures + index;
                        polygon += sizeof(EncodedMeshFace);
                    }
                }
            }
            value = xport_load_le32(encoded + 104);
            if (value != 0)
            {
                uint32 header_index = 0;
                while (header_index < animation_count && source_animation_offsets[header_index] != value)
                    ++header_index;
                if (header_index == animation_count)
                {
                    if (animation_count == animation_header_capacity || (value & 3) != 0 || value > data_bytes || sizeof(EncodedAnimationHeader) > data_bytes - value)
                    {
                        fprintf(stderr, "P2: Encoded scene animation header exceeds resource\n");
                        abort();
                    }
                    source_animation_offsets[animation_count++] = value;
                    animations[header_index].Relocate((uint8 *)data, data_bytes, (const EncodedAnimationHeader *)((value) <= data_bytes && (sizeof(EncodedAnimationHeader)) <= data_bytes - (value) ? data + (value) : (fprintf(stderr, "P2: Scene file span exceeds native input\n"), abort(), data)));
                }
                actor->animation = animations + header_index;
            }
            value = xport_load_le32(encoded + 44);
            if (value != 0)
                actor->material_animation_entries = (uint32 *)((value) <= data_bytes && (sizeof(uint32)) <= data_bytes - (value) ? data + (value) : (fprintf(stderr, "P2: Scene file span exceeds native input\n"), abort(), data));
            if (actor->behavior == 31)
                actor->linked_object = (AnimatedObject *)((xport_load_le32(encoded + 108)) <= data_bytes && (232) <= data_bytes - (xport_load_le32(encoded + 108)) ? data + (xport_load_le32(encoded + 108)) : (fprintf(stderr, "P2: Scene file span exceeds native input\n"), abort(), data));
            value = xport_load_le32(encoded + 156);
            if (value != 0)
            {
                fprintf(stderr, "P2: Scene object contains an unresolved runtime sprite reference\n");
                abort();
            }
            actor->sprite = 0;
            uint32 next = xport_load_le32(encoded);
            node = next != 0 ? next : 0;
            ++native_actor_index;
        }
        object_heads[group] = actor_group_counts[group] != 0 ? &animated_objects[group_first].node : 0;
    }
    node = object_heads[2] != 0 ? ((uintptr_t)(object_heads[2]) >= source_begin && ((uintptr_t)(object_heads[2]) - source_begin) <= data_bytes && (52) <= data_bytes - ((uintptr_t)(object_heads[2]) - source_begin) ? (uint32)((uintptr_t)(object_heads[2]) - source_begin) : (fprintf(stderr, "P2: Scene source pointer exceeds native input\n"), abort(), 0u)) : 0;
    node_budget = data_bytes / sizeof(uint32);
    while (node != 0)
    {
        if (node_budget == 0 || (node & 3) != 0 || data_bytes < 52 || node > data_bytes - 52)
        {
            fprintf(stderr, "P2: Scene mesh node is outside payload or cyclic\n");
            abort();
        }
        --node_budget;
        const uint8 *encoded = (const uint8 *)((node) <= data_bytes && (52) <= data_bytes - (node) ? data + (node) : (fprintf(stderr, "P2: Scene file span exceeds native input\n"), abort(), data));
        value = xport_load_le32(encoded + 36);
        if (value != 0)
        {
            uint32 polygon = value;
            sint32 active = (sint16)xport_load_le16(encoded + 34);
            if (active > 0 && (xport_load_le32(encoded + 28) & 0x400) == 0)
            {
                sint32 face_count = (sint16)xport_load_le16(encoded + 32);
                if (face_count < 0 || (value & 3) != 0 || value > data_bytes || (uint32)face_count > (data_bytes - value) / sizeof(EncodedMeshFace))
                {
                    fprintf(stderr, "P2: Scene mesh polygons are outside payload\n");
                    abort();
                }
                uint32 count = (uint32)face_count;
                while (count != 0)
                {
                    const EncodedMeshFace *face = (const EncodedMeshFace *)((polygon) <= data_bytes && (sizeof(EncodedMeshFace)) <= data_bytes - (polygon) ? data + (polygon) : (fprintf(stderr, "P2: Scene file span exceeds native input\n"), abort(), data));
                    uint32 index = xport_load_le32(face);
                    if (index >= resource->texture_count)
                    {
                        fprintf(stderr, "P2: Scene polygon texture index is outside resource\n");
                        abort();
                    }
                    --count;
                    if (source_face_textures[(polygon) / sizeof(uint32)] != 0)
                    {
                        fprintf(stderr, "P2: Scene polygon was relocated twice\n");
                        abort();
                    }
                    source_face_textures[(polygon) / sizeof(uint32)] = resource->textures + index;
                    polygon += sizeof(EncodedMeshFace);
                }
            }
        }
        uint32 next_offset = xport_load_le32(encoded);
        node = next_offset != 0 ? next_offset : 0;
    }
    uint32 *source_path_offsets;
    {
        uint32 matrix_count = 0, unique_count = 0;
        node = object_heads[3] != 0 ? ((uintptr_t)(object_heads[3]) >= source_begin && ((uintptr_t)(object_heads[3]) - source_begin) <= data_bytes && (56) <= data_bytes - ((uintptr_t)(object_heads[3]) - source_begin) ? (uint32)((uintptr_t)(object_heads[3]) - source_begin) : (fprintf(stderr, "P2: Scene source pointer exceeds native input\n"), abort(), 0u)) : 0;
        path_count = 0;
        node_budget = data_bytes / sizeof(uint32);
        while (node != 0)
        {
            if (node_budget == 0 || (node & 3) != 0 || data_bytes < 56 || node > data_bytes - 56)
            {
                fprintf(stderr, "P2: Scene path registry exceeds resource or is cyclic\n");
                abort();
            }
            --node_budget;
            const uint8 *encoded = (const uint8 *)((node) <= data_bytes && (56) <= data_bytes - (node) ? data + (node) : (fprintf(stderr, "P2: Scene file span exceeds native input\n"), abort(), data));
            ++path_count;
            if (xport_load_le32(encoded + 32) != 0)
                ++matrix_count;
            uint32 next = xport_load_le32(encoded);
            node = next != 0 ? next : 0;
        }
        if (path_count > 0xFFFFFFFFu / sizeof(ScenePath) || path_count > 0xFFFFFFFFu / sizeof(uint32) || matrix_count > 0xFFFFFFFFu / sizeof(MATRIX))
        {
            fprintf(stderr, "P2: Native scene path allocation overflows\n");
            abort();
        }
        paths = path_count != 0 ? (ScenePath *)malloc(path_count * sizeof(ScenePath)) : 0;
        source_path_offsets = path_count != 0 ? (uint32 *)malloc(path_count * sizeof(uint32)) : 0;
        path_offset_matrices = matrix_count != 0 ? (MATRIX *)malloc(matrix_count * sizeof(MATRIX)) : 0;
        uint32 *matrix_sources = matrix_count != 0 ? (uint32 *)malloc(matrix_count * sizeof(uint32)) : 0;
        if ((path_count != 0 && (paths == 0 || source_path_offsets == 0)) || (matrix_count != 0 && (path_offset_matrices == 0 || matrix_sources == 0)))
        {
            fprintf(stderr, "P2: Cannot allocate native scene paths\n");
            abort();
        }
        node = object_heads[3] != 0 ? ((uintptr_t)(object_heads[3]) >= source_begin && ((uintptr_t)(object_heads[3]) - source_begin) <= data_bytes && (56) <= data_bytes - ((uintptr_t)(object_heads[3]) - source_begin) ? (uint32)((uintptr_t)(object_heads[3]) - source_begin) : (fprintf(stderr, "P2: Scene source pointer exceeds native input\n"), abort(), 0u)) : 0;
        for (uint32 path_index = 0; path_index < path_count; ++path_index)
        {
            const uint8 *encoded = (const uint8 *)((node) <= data_bytes && (56) <= data_bytes - (node) ? data + (node) : (fprintf(stderr, "P2: Scene file span exceeds native input\n"), abort(), data));
            source_path_offsets[path_index] = node;
            ScenePath *path = paths + path_index;
            path->node.links.registry_next = path_index + 1 < path_count ? &paths[path_index + 1].node : 0;
            SceneNode **links[] = {&path->node.links.sibling_next, &path->node.links.first_child};
            for (uint32 link_index = 0; link_index < 2; ++link_index)
            {
                uint32 offset = xport_load_le32(encoded + 4 + link_index * 4);
                if (offset != 0 && (offset > data_bytes || 16 > data_bytes - offset))
                {
                    fprintf(stderr, "P2: Scene path node link exceeds resource\n");
                    abort();
                }
                *links[link_index] = offset != 0 ? (SceneNode *)((offset) <= data_bytes && (16) <= data_bytes - (offset) ? data + (offset) : (fprintf(stderr, "P2: Scene file span exceeds native input\n"), abort(), data)) : 0;
            }
            path->node.kind = encoded[12];
            path->node.flags = encoded[13];
            path->node.state_flags = xport_load_le16(encoded + 14);
            for (uint32 side = 0; side < 2; ++side)
            {
                uint32 offset = xport_load_le32(encoded + 16 + side * 4);
                if (offset != 0 && (offset > data_bytes || 56 > data_bytes - offset))
                {
                    fprintf(stderr, "P2: Adjacent scene path exceeds resource\n");
                    abort();
                }
                path->adjacent[side] = offset != 0 ? (const ScenePath *)((offset) <= data_bytes && (56) <= data_bytes - (offset) ? data + (offset) : (fprintf(stderr, "P2: Scene file span exceeds native input\n"), abort(), data)) : 0;
                path->motion_limits[side] = (sint16)xport_load_le16(encoded + 24 + side * 2);
            }
            uint32 definition = xport_load_le32(encoded + 36);
            if (definition == 0 || (definition & 3) != 0 || definition > data_bytes || 44 > data_bytes - definition)
            {
                fprintf(stderr, "P2: Scene path has no valid resource definition\n");
                abort();
            }
            path->definition = (const PathDefinition *)(data + definition);
            uint32 definition_flags = xport_load_le32(data + definition + 16);
            uint32 fallback = xport_load_le32(encoded + 28);
            path->fallback = 0;
            path->slope_scale = 0;
            if ((definition_flags & 0x100010) != 0)
            {
                if (fallback != 0)
                {
                    if ((fallback & 3) != 0 || fallback > data_bytes || 56 > data_bytes - fallback)
                    {
                        fprintf(stderr, "P2: Scene fallback reference exceeds resource\n");
                        abort();
                    }
                    uint32 flags = path->node.state_flags;
                    if ((flags & ((flags & 0x10) | 8)) == 0)
                    {
                        // Resolve the file reference before reading its path flags
                        const uint8 *reference = data + fallback;
                        if ((xport_load_le16(reference + 14) & 8) != 0)
                            fallback = xport_load_le32(reference + 16);
                    }
                    if (fallback > data_bytes || 56 > data_bytes - fallback)
                    {
                        fprintf(stderr, "P2: Scene fallback path exceeds resource\n");
                        abort();
                    }
                    path->fallback = (const ScenePath *)((fallback) <= data_bytes && (56) <= data_bytes - (fallback) ? data + (fallback) : (fprintf(stderr, "P2: Scene file span exceeds native input\n"), abort(), data));
                }
                else
                    path->fallback = 0;
            }
            else
                path->slope_scale = fallback;
            uint32 matrix = xport_load_le32(encoded + 32);
            path->offset_matrix = 0;
            if (matrix != 0)
            {
                if ((matrix & 3) != 0 || matrix > data_bytes || sizeof(MATRIX) > data_bytes - matrix)
                {
                    fprintf(stderr, "P2: Scene path matrix exceeds resource\n");
                    abort();
                }
                uint32 slot = 0;
                while (slot < unique_count && matrix_sources[slot] != matrix)
                    ++slot;
                if (slot == unique_count)
                {
                    memcpy(path_offset_matrices + slot, ((matrix) <= data_bytes && (sizeof(MATRIX)) <= data_bytes - (matrix) ? data + (matrix) : (fprintf(stderr, "P2: Scene file span exceeds native input\n"), abort(), data)), sizeof(MATRIX));
                    matrix_sources[slot] = matrix;
                    ++unique_count;
                }
                path->offset_matrix = path_offset_matrices + slot;
            }
            memcpy(&path->offset_track, encoded + 40, 12);
            uint32 keys = xport_load_le32(encoded + 52);
            if (keys != 0 && keys >= data_bytes)
            {
                fprintf(stderr, "P2: Scene path keys exceed resource\n");
                abort();
            }
            path->offset_track.keys = keys != 0 ? (const uint8 *)((keys) <= data_bytes && (1) <= data_bytes - (keys) ? data + (keys) : (fprintf(stderr, "P2: Scene file span exceeds native input\n"), abort(), data)) : 0;
            uint32 next = xport_load_le32(encoded);
            node = next != 0 ? next : 0;
        }
        free(matrix_sources);
        path_name_count = path_count;
        path_names = path_count != 0 ? (SceneObjectName *)malloc(path_count * sizeof(SceneObjectName)) : 0;
        if (path_count != 0 && path_names == 0)
        {
            fprintf(stderr, "P2: Cannot allocate scene path names\n");
            abort();
        }
        for (uint32 path_index = 0; path_index < path_count; ++path_index)
        {
            uint32 offset = source_path_offsets[path_index];
            if (offset > data_bytes || 60 + sizeof(SceneObjectName) > data_bytes - offset)
            {
                fprintf(stderr, "P2: Scene path name exceeds resource\n");
                abort();
            }
            memcpy(path_names + path_index, ((offset + 60) <= data_bytes && (sizeof(SceneObjectName)) <= data_bytes - (offset + 60) ? data + (offset + 60) : (fprintf(stderr, "P2: Scene file span exceeds native input\n"), abort(), data)), sizeof(SceneObjectName));
        }
        object_heads[3] = path_count != 0 ? &paths[0].node : 0;
    }
    node = object_heads[4] != 0 ? ((uintptr_t)(object_heads[4]) >= source_begin && ((uintptr_t)(object_heads[4]) - source_begin) <= data_bytes && (16) <= data_bytes - ((uintptr_t)(object_heads[4]) - source_begin) ? (uint32)((uintptr_t)(object_heads[4]) - source_begin) : (fprintf(stderr, "P2: Scene source pointer exceeds native input\n"), abort(), 0u)) : 0;
    node_budget = data_bytes / sizeof(uint32);
    while (node != 0)
    {
        if (node_budget == 0 || data_bytes < SceneTrigger::ControlBytes || node > data_bytes - SceneTrigger::ControlBytes)
        {
            fprintf(stderr, "P2: Scene camera trigger registry exceeds resource or is cyclic\n");
            abort();
        }
        --node_budget;
        const uint8 *encoded = (const uint8 *)((node) <= data_bytes && (SceneTrigger::ControlBytes) <= data_bytes - (node) ? data + (node) : (fprintf(stderr, "P2: Scene file span exceeds native input\n"), abort(), data));
        if ((sint8)encoded[15] == 8)
        {
            SceneTrigger camera_input = {0};
            camera_input.parameters.cutscene.camera_id = xport_load_le32(encoded + 68);
            camera_input.parameters.cutscene.next_camera = xport_load_le32(encoded + 72);
            camera_input.parameters.cutscene.duration = xport_load_le32(encoded + 76);
            camera_input.cutscene_control.flags = xport_load_le16(encoded + 80);
            camera_input.cutscene_control.index = (sint16)xport_load_le16(encoded + 82);
            RegisterCutsceneCamera(&camera_input);
        }
        uint32 next_offset = xport_load_le32(encoded);
        node = next_offset != 0 ? next_offset : 0;
    }
    uint32 *source_placement_offsets;
    {
        uint32 transform_count = 0, unique_count = 0;
        placement_count = 0;
        node = object_heads[9] != 0 ? ((uintptr_t)(object_heads[9]) >= source_begin && ((uintptr_t)(object_heads[9]) - source_begin) <= data_bytes && (48) <= data_bytes - ((uintptr_t)(object_heads[9]) - source_begin) ? (uint32)((uintptr_t)(object_heads[9]) - source_begin) : (fprintf(stderr, "P2: Scene source pointer exceeds native input\n"), abort(), 0u)) : 0;
        node_budget = data_bytes / sizeof(uint32);
        while (node != 0)
        {
            if (node_budget == 0 || (node & 3) != 0 || data_bytes < 48 || node > data_bytes - 48)
            {
                fprintf(stderr, "P2: Scene placement registry exceeds resource or is cyclic\n");
                abort();
            }
            --node_budget;
            const uint8 *encoded = (const uint8 *)((node) <= data_bytes && (48) <= data_bytes - (node) ? data + (node) : (fprintf(stderr, "P2: Scene file span exceeds native input\n"), abort(), data));
            ++placement_count;
            if (xport_load_le32(encoded + 40) != 0)
                ++transform_count;
            uint32 next = xport_load_le32(encoded);
            node = next != 0 ? next : 0;
        }
        if (placement_count > 0xFFFFFFFFu / sizeof(ActorPlacement) || placement_count > 0xFFFFFFFFu / sizeof(uint32) || transform_count > 0xFFFFFFFFu / sizeof(PathTransform))
        {
            fprintf(stderr, "P2: Native scene placement allocation overflows\n");
            abort();
        }
        placements = placement_count != 0 ? (ActorPlacement *)malloc(placement_count * sizeof(ActorPlacement)) : 0;
        source_placement_offsets = placement_count != 0 ? (uint32 *)malloc(placement_count * sizeof(uint32)) : 0;
        definition_transforms = transform_count != 0 ? (PathTransform *)malloc(transform_count * sizeof(PathTransform)) : 0;
        uint32 *transform_sources = transform_count != 0 ? (uint32 *)malloc(transform_count * sizeof(uint32)) : 0;
        if ((placement_count != 0 && (placements == 0 || source_placement_offsets == 0)) || (transform_count != 0 && (definition_transforms == 0 || transform_sources == 0)))
        {
            fprintf(stderr, "P2: Cannot allocate native scene placements\n");
            abort();
        }
        node = object_heads[9] != 0 ? ((uintptr_t)(object_heads[9]) >= source_begin && ((uintptr_t)(object_heads[9]) - source_begin) <= data_bytes && (48) <= data_bytes - ((uintptr_t)(object_heads[9]) - source_begin) ? (uint32)((uintptr_t)(object_heads[9]) - source_begin) : (fprintf(stderr, "P2: Scene source pointer exceeds native input\n"), abort(), 0u)) : 0;
        for (uint32 placement_index = 0; placement_index < placement_count; ++placement_index)
        {
            const uint8 *encoded = (const uint8 *)((node) <= data_bytes && (48) <= data_bytes - (node) ? data + (node) : (fprintf(stderr, "P2: Scene file span exceeds native input\n"), abort(), data));
            source_placement_offsets[placement_index] = node;
            ActorPlacement *placement = placements + placement_index;
            placement->path.node.links.registry_next = placement_index + 1 < placement_count ? &placements[placement_index + 1].path.node : 0;
            SceneNode **links[] = {&placement->path.node.links.sibling_next, &placement->path.node.links.first_child};
            for (uint32 link_index = 0; link_index < 2; ++link_index)
            {
                uint32 offset = xport_load_le32(encoded + 4 + link_index * 4);
                if (offset != 0 && (offset > data_bytes || 16 > data_bytes - offset))
                {
                    fprintf(stderr, "P2: Scene placement node link exceeds resource\n");
                    abort();
                }
                *links[link_index] = offset != 0 ? (SceneNode *)((offset) <= data_bytes && (16) <= data_bytes - (offset) ? data + (offset) : (fprintf(stderr, "P2: Scene file span exceeds native input\n"), abort(), data)) : 0;
            }
            placement->path.node.kind = encoded[12];
            placement->path.node.flags = encoded[13];
            placement->path.node.state_flags = xport_load_le16(encoded + 14);
            placement->path.flags = xport_load_le32(encoded + 16);
            uint32 keys = xport_load_le32(encoded + 32);
            if (keys != 0)
                memcpy(&placement->path.positions, encoded + 20, 12);
            else
            {
                for (uint32 axis = 0; axis < 3; ++axis)
                    placement->path.static_position[axis] = xport_load_le32(encoded + 20 + axis * 4);
            }
            if (keys != 0 && keys >= data_bytes)
            {
                fprintf(stderr, "P2: Scene placement keys exceed resource\n");
                abort();
            }
            placement->path.positions.keys = keys != 0 ? (const uint8 *)((keys) <= data_bytes && (1) <= data_bytes - (keys) ? data + (keys) : (fprintf(stderr, "P2: Scene file span exceeds native input\n"), abort(), data)) : 0;
            placement->path.metadata36 = xport_load_le16(encoded + 36);
            placement->path.search_radius = (sint16)xport_load_le16(encoded + 38);
            uint32 transform = xport_load_le32(encoded + 40);
            placement->path.transform = 0;
            if (transform != 0)
            {
                if ((transform & 3) != 0 || transform > data_bytes || 44 > data_bytes - transform)
                {
                    fprintf(stderr, "P2: Scene placement transform exceeds resource\n");
                    abort();
                }
                uint32 slot = 0;
                while (slot < unique_count && transform_sources[slot] != transform)
                    ++slot;
                if (slot == unique_count)
                {
                    const uint8 *source = (const uint8 *)((transform) <= data_bytes && (44) <= data_bytes - (transform) ? data + (transform) : (fprintf(stderr, "P2: Scene file span exceeds native input\n"), abort(), data));
                    memcpy(&definition_transforms[slot].matrix, source, sizeof(MATRIX));
                    for (uint32 component = 0; component < 3; ++component)
                        definition_transforms[slot].displacement[component] = xport_load_le32(source + 32 + component * 4);
                    transform_sources[slot] = transform;
                    ++unique_count;
                }
                placement->path.transform = definition_transforms + slot;
            }
            uint32 state = xport_load_le32(encoded + 44);
            if (state != 0 && (state > data_bytes || 4 > data_bytes - state))
            {
                fprintf(stderr, "P2: Scene placement state exceeds resource\n");
                abort();
            }
            placement->state = state != 0 ? ((state) <= data_bytes && (4) <= data_bytes - (state) ? data + (state) : (fprintf(stderr, "P2: Scene file span exceeds native input\n"), abort(), data)) : 0;
            uint32 next = xport_load_le32(encoded);
            node = next != 0 ? next : 0;
        }
        free(transform_sources);
        object_heads[9] = placement_count != 0 ? &placements[0].path.node : 0;
    }
    uint32 path_key_bytes = 0;
    uint32 path_key_tracks = 0;
    uint32 path_key_cursor = 0;
    const uint8 *section_data = (const uint8 *)data;
    const uint32 path_key_heads[2] = {3, 9};
    for (uint32 pass = 0; pass < 2; ++pass)
    {
        if (pass != 0)
        {
            path_key_data = path_key_tracks != 0 ? (uint8 *)malloc(path_key_bytes != 0 ? path_key_bytes : 1) : 0;
            if (path_key_tracks != 0 && path_key_data == 0)
            {
                fprintf(stderr, "P2: Cannot allocate scene path keys\n");
                abort();
            }
        }
        for (uint32 key_group = 0; key_group < 2; ++key_group)
        {
            SceneNode *key_node = object_heads[path_key_heads[key_group]];
            while (key_node != 0)
            {
                AnimationTrack *track = key_group == 0 ? &((ScenePath *)key_node)->offset_track : &((ActorPlacement *)key_node)->path.positions;
                if (track->keys != 0)
                {
                    uint32 key_offset = (uint32)(track->keys - section_data);
                    uint32 key_bytes = (uint32)track->key_count * sizeof(VectorAnimationKey);
                    uint32 prefix_bytes = 0;
                    if (key_offset > data_bytes || key_bytes > data_bytes - key_offset)
                    {
                        fprintf(stderr, "P2: Scene path keys exceed section data\n");
                        abort();
                    }
                    for (uint32 key_index = 0; key_index < track->key_count; ++key_index)
                    {
                        uint32 curve_bytes = (xport_load_le32(track->keys + key_index * sizeof(VectorAnimationKey)) & 255) * sizeof(AnimationEaseCurve);
                        if (curve_bytes > prefix_bytes)
                            prefix_bytes = curve_bytes;
                    }
                    if (prefix_bytes > key_offset)
                    {
                        fprintf(stderr, "P2: Scene path easing prefix exceeds section data\n");
                        abort();
                    }
                    uint32 bytes = prefix_bytes + key_bytes;
                    if (pass == 0)
                    {
                        if (bytes > 0xFFFFFFFFu - path_key_bytes)
                        {
                            fprintf(stderr, "P2: Scene path key allocation overflows\n");
                            abort();
                        }
                        path_key_bytes += bytes;
                        ++path_key_tracks;
                    }
                    else
                    {
                        uint8 *destination = path_key_data + path_key_cursor;
                        memcpy(destination, track->keys - prefix_bytes, bytes);
                        track->keys = destination + prefix_bytes;
                        path_key_cursor += bytes;
                    }
                }
                key_node = key_node->links.registry_next;
            }
        }
    }
    const ActorBinding *source_binding = actor_bindings;
    uint32 binding_count = 0;
    while (source_binding != 0)
    {
        uint32 source_offset = ((uintptr_t)((void *)source_binding) >= source_begin && ((uintptr_t)((void *)source_binding) - source_begin) <= data_bytes && (228) <= data_bytes - ((uintptr_t)((void *)source_binding) - source_begin) ? (uint32)((uintptr_t)((void *)source_binding) - source_begin) : (fprintf(stderr, "P2: Scene source pointer exceeds native input\n"), abort(), 0u));
        ++binding_count;
        if (binding_count > data_bytes / 228 || source_offset > data_bytes || 228 > data_bytes - source_offset)
        {
            fprintf(stderr, "P2: Invalid actor state list extent\n");
            abort();
        }
        value = xport_load_le32((const uint8 *)source_binding);
        source_binding = value != 0 ? (const ActorBinding *)((value) <= data_bytes && (228) <= data_bytes - (value) ? data + (value) : (fprintf(stderr, "P2: Scene file span exceeds native input\n"), abort(), data)) : 0;
    }
    ActorBinding *bindings = binding_count != 0 ? (ActorBinding *)malloc(binding_count * sizeof(ActorBinding)) : 0;
    if (binding_count != 0 && bindings == 0)
    {
        fprintf(stderr, "P2: Cannot allocate scene actor states\n");
        abort();
    }
    source_binding = actor_bindings;
    for (uint32 binding_index = 0; binding_index < binding_count; ++binding_index)
    {
        ActorBinding *binding = bindings + binding_index;
        const uint8 *encoded = (const uint8 *)source_binding;
        uint32 next = xport_load_le32(encoded);
        uint32 definition = xport_load_le32(encoded + 60);
        uint32 payload = xport_load_le32(encoded + 56);
        binding->event_command = xport_load_le32(encoded + 8);
        binding->distance_squared = xport_load_le32(encoded + 12);
        binding->motion_speed = xport_load_le32(encoded + 16);
        binding->reaction_flags = xport_load_le32(encoded + 44);
        binding->interaction_timer = xport_load_le32(encoded + 76);
        binding->interaction_heading_ticks = xport_load_le32(encoded + 80);
        binding->interaction_height = xport_load_le32(encoded + 84);
        binding->pending_event = xport_load_le32(encoded + 128);
        binding->range_ticks = xport_load_le32(encoded + 136);
        binding->action_speed = xport_load_le32(encoded + 168);
        binding->sampled_pitch = xport_load_le32(encoded + 172);
        binding->reaction_state = xport_load_le32(encoded + 220);
        binding->path_frame = xport_load_le16(encoded + 24);
        binding->initial_path_frame = xport_load_le16(encoded + 26);
        binding->motion_frame = xport_load_le16(encoded + 28);
        binding->path_pitch_speed = xport_load_le16(encoded + 30);
        binding->respawn_ticks = xport_load_le16(encoded + 40);
        binding->respawn_delay = xport_load_le16(encoded + 48);
        binding->sound_pitch = xport_load_le16(encoded + 64);
        binding->sound_voice = xport_load_le16(encoded + 66);
        binding->path_heading = xport_load_le16(encoded + 88);
        binding->route_heading = xport_load_le16(encoded + 104);
        binding->action_flags = xport_load_le16(encoded + 120);
        binding->action_effect_flags = xport_load_le16(encoded + 122);
        binding->action_reset_half = xport_load_le16(encoded + 164);
        binding->desired_pitch = xport_load_le16(encoded + 176);
        binding->height_direction_changes = xport_load_le16(encoded + 178);
        binding->blink_active = xport_load_le16(encoded + 224);
        binding->blink_ticks = xport_load_le16(encoded + 226);
        binding->model_kind = encoded[37];
        binding->flags = encoded[38];
        binding->action_index = encoded[124];
        binding->action_phase = encoded[125];
        binding->turn_delay = encoded[132];
        binding->projectile_phase = encoded[133];
        binding->next_projectile_frame = encoded[135];
        binding->route_mode = encoded[166];
        binding->route_direction_mode = encoded[180];
        binding->turn_ticks = encoded[181];
        for (uint32 component = 0; component < 2; ++component)
            binding->path_parameters[component] = xport_load_le16(encoded + 20 + component * 2);
        for (uint32 component = 0; component < 3; ++component)
            binding->path_direction[component] = xport_load_le32(encoded + 92 + component * 4);
        for (uint32 component = 0; component < 3; ++component)
            binding->route_direction[component] = xport_load_le32(encoded + 108 + component * 4);
        for (uint32 component = 0; component < 6; ++component)
            binding->trigger_ticks[component] = xport_load_le32(encoded + 140 + component * 4);
        for (uint32 component = 0; component < 3; ++component)
            binding->projectile_velocity[component] = xport_load_le32(encoded + 184 + component * 4);
        for (uint32 component = 0; component < 3; ++component)
            binding->sampled_velocity[component] = xport_load_le32(encoded + 196 + component * 4);
        memcpy(binding->resource_bytes32, encoded + 32, 5);
        memcpy(binding->resource_bytes39, encoded + 39, 1);
        memcpy(binding->resource_bytes42, encoded + 42, 2);
        memcpy(binding->resource_bytes50, encoded + 50, 2);
        memcpy(binding->resource_bytes90, encoded + 90, 2);
        memcpy(binding->resource_bytes106, encoded + 106, 2);
        memcpy(binding->resource_bytes126, encoded + 126, 2);
        memcpy(binding->resource_bytes134, encoded + 134, 1);
        memcpy(binding->resource_bytes167, encoded + 167, 1);
        memcpy(binding->resource_bytes182, encoded + 182, 2);
        memcpy(binding->resource_bytes208, encoded + 208, 12);
        binding->next = binding_index + 1 < binding_count ? bindings + binding_index + 1 : 0;
        binding->model = 0;
        binding->instance = 0;
        binding->sound = 0;
        value = xport_load_le32(encoded + 52);
        if (value != 0)
        {
            fprintf(stderr, "P2: Actor state contains an unresolved runtime event reference\n");
            abort();
        }
        binding->event_node = 0;
        binding->definition = (ActorPlacement *)((definition) <= data_bytes && (sizeof(ActorPlacement)) <= data_bytes - (definition) ? data + (definition) : (fprintf(stderr, "P2: Scene file span exceeds native input\n"), abort(), data));
        binding->rotation_track = 0;
        if (payload != 0)
        {
            const EncodedAnimationTrack *source_track = (const EncodedAnimationTrack *)((payload) <= data_bytes && (sizeof(EncodedAnimationTrack)) <= data_bytes - (payload) ? data + (payload) : (fprintf(stderr, "P2: Scene file span exceeds native input\n"), abort(), data));
            uint32 key_offset = xport_load_le32((const uint8 *)source_track + 12);
            uint32 key_bytes = (uint32)source_track->state[0] * sizeof(QuaternionAnimationKey);
            uint32 prefix_bytes = 0;
            const uint8 *source_keys = 0;
            if (key_offset != 0)
            {
                if (key_offset > data_bytes || key_bytes > data_bytes - key_offset)
                {
                    fprintf(stderr, "P2: Actor quaternion keys exceed section data\n");
                    abort();
                }
                source_keys = (const uint8 *)((key_offset) <= data_bytes && (key_bytes) <= data_bytes - (key_offset) ? data + (key_offset) : (fprintf(stderr, "P2: Scene file span exceeds native input\n"), abort(), data));
                for (uint32 key_index = 0; key_index < source_track->state[0]; ++key_index)
                {
                    uint32 curve_bytes = (xport_load_le32(source_keys + key_index * sizeof(QuaternionAnimationKey)) & 255) * sizeof(AnimationEaseCurve);
                    if (curve_bytes > prefix_bytes)
                        prefix_bytes = curve_bytes;
                }
                if (prefix_bytes > key_offset)
                {
                    fprintf(stderr, "P2: Actor quaternion easing prefix exceeds section data\n");
                    abort();
                }
            }
            else
                key_bytes = 0;
            AnimationTrack *track = (AnimationTrack *)malloc(sizeof(AnimationTrack) + prefix_bytes + key_bytes);
            if (track == 0)
            {
                fprintf(stderr, "P2: Cannot allocate actor rotation track\n");
                abort();
            }
            memcpy(track, source_track->state, sizeof(source_track->state));
            if (key_offset != 0)
            {
                uint8 *storage = (uint8 *)(track + 1);
                const uint8 *source_span = (const uint8 *)((key_offset - prefix_bytes) <= data_bytes && (prefix_bytes + key_bytes) <= data_bytes - (key_offset - prefix_bytes) ? data + (key_offset - prefix_bytes) : (fprintf(stderr, "P2: Scene file span exceeds native input\n"), abort(), data));
                memcpy(storage, source_span, prefix_bytes + key_bytes);
                track->keys = storage + prefix_bytes;
            }
            else
                track->keys = 0;
            binding->rotation_track = track;
        }
        ActorPlacement *placement = (ActorPlacement *)object_heads[9];
        while (placement != 0)
        {
            if (placement->state == source_binding)
                placement->state = binding;
            placement = (ActorPlacement *)placement->path.node.links.registry_next;
        }
        source_binding = next != 0 ? (const ActorBinding *)((next) <= data_bytes && (228) <= data_bytes - (next) ? data + (next) : (fprintf(stderr, "P2: Scene file span exceeds native input\n"), abort(), data)) : 0;
    }
    actor_bindings = bindings;
    const AnimatedPathState *source_state = animated_paths;
    uint32 animated_path_count = 0;
    while (source_state != 0)
    {
        ++animated_path_count;
        if (animated_path_count > data_bytes / sizeof(AnimatedPathState))
        {
            fprintf(stderr, "P2: Invalid animated path state list extent\n");
            abort();
        }
        value = xport_load_le32((const uint8 *)source_state);
        source_state = value != 0 ? (const AnimatedPathState *)((value) <= data_bytes && (sizeof(AnimatedPathState)) <= data_bytes - (value) ? data + (value) : (fprintf(stderr, "P2: Scene file span exceeds native input\n"), abort(), data)) : 0;
    }
    AnimatedPathState *states = animated_path_count != 0 ? (AnimatedPathState *)malloc(animated_path_count * sizeof(AnimatedPathState)) : 0;
    if (animated_path_count != 0 && states == 0)
    {
        fprintf(stderr, "P2: Cannot allocate scene animated path states\n");
        abort();
    }
    source_state = animated_paths;
    for (uint32 path_index = 0; path_index < animated_path_count; ++path_index)
    {
        AnimatedPathState *state = states + path_index;
        const uint8 *encoded = (const uint8 *)source_state;
        uint32 next = xport_load_le32(encoded);
        state->next = path_index + 1 < animated_path_count ? states + path_index + 1 : 0;
        state->start_frame = xport_load_le16(encoded + 4);
        state->sound_voice = (sint16)xport_load_le16(encoded + 6);
        state->sound = 0;
        for (uint32 component = 0; component < 3; ++component)
        {
            state->speed[component] = xport_load_le32(encoded + 12 + component * 4);
            state->direction[component] = xport_load_le32(encoded + 24 + component * 4);
        }
        ActorPlacement *placement = (ActorPlacement *)object_heads[9];
        while (placement != 0)
        {
            if (placement->state == source_state)
                placement->state = state;
            placement = (ActorPlacement *)placement->path.node.links.registry_next;
        }
        source_state = next != 0 ? (const AnimatedPathState *)((next) <= data_bytes && (sizeof(AnimatedPathState)) <= data_bytes - (next) ? data + (next) : (fprintf(stderr, "P2: Scene file span exceeds native input\n"), abort(), data)) : 0;
    }
    animated_paths = states;
    uint32 extent_capacity = 0;
    node = object_heads[12] != 0 ? ((uintptr_t)(object_heads[12]) >= source_begin && ((uintptr_t)(object_heads[12]) - source_begin) <= data_bytes && (sizeof(SceneNode)) <= data_bytes - ((uintptr_t)(object_heads[12]) - source_begin) ? (uint32)((uintptr_t)(object_heads[12]) - source_begin) : (fprintf(stderr, "P2: Scene source pointer exceeds native input\n"), abort(), 0u)) : 0;
    node_budget = data_bytes / sizeof(uint32);
    while (node != 0)
    {
        if (node_budget == 0 || (node & 3) != 0 || data_bytes < 48 || node > data_bytes - 48)
        {
            fprintf(stderr, "P2: Sprite extent registry exceeds scene payload or is cyclic\n");
            abort();
        }
        --node_budget;
        const uint8 *encoded = (const uint8 *)((node) <= data_bytes && (48) <= data_bytes - (node) ? data + (node) : (fprintf(stderr, "P2: Scene file span exceeds native input\n"), abort(), data));
        if ((sint8)encoded[27] != -1)
            ++extent_capacity;
        uint32 next = xport_load_le32(encoded);
        node = next != 0 ? next : 0;
    }
    if (extent_capacity > 0xFFFFFFFFu / sizeof(AnimationTrack *) || extent_capacity > 0xFFFFFFFFu / sizeof(uint32))
    {
        fprintf(stderr, "P2: Sprite extent owner capacity overflows\n");
        abort();
    }
    sprite_extent_tracks = extent_capacity != 0 ? (AnimationTrack **)calloc(extent_capacity, sizeof(AnimationTrack *)) : 0;
    uint32 *source_extent_offsets = extent_capacity != 0 ? (uint32 *)malloc(extent_capacity * sizeof(uint32)) : 0;
    if (extent_capacity != 0 && (sprite_extent_tracks == 0 || source_extent_offsets == 0))
    {
        fprintf(stderr, "P2: Cannot allocate sprite extent owners\n");
        abort();
    }
    sprite_extent_track_count = 0;
    {
        const SceneLight *source_light = (const SceneLight *)object_heads[14];
        const MeshGeometryNode *source_mesh = (const MeshGeometryNode *)object_heads[2];
        const PositionedBillboardNode *source_billboard = (const PositionedBillboardNode *)object_heads[8];
        const MeshPartNode *source_part = (const MeshPartNode *)object_heads[13];
        const SphereNode *source_sphere = (const SphereNode *)object_heads[15];
        const SpriteRecord *source_sprite = (const SpriteRecord *)object_heads[12];
        const SceneNode **source_nodes;
        SceneNode **native_nodes;
        uint32 owner_count, owner_index, mesh_index, light_index, path_index, billboard_index;
        uint32 part_index, sphere_index, placement_index, actor_index, sprite_index;
        uint32 actor_first, actor_limit, sprite_first, sprite_limit;
        uint32 trigger_index, trigger_first, trigger_limit;
        light_count = 0;
        mesh_count = 0;
        billboard_count = 0;
        mesh_part_count = 0;
        sphere_node_count = 0;
        sprite_count = 0;
        trigger_count = 0;
        while (source_light != 0)
        {
            uint32 source_offset = ((uintptr_t)((void *)source_light) >= source_begin && ((uintptr_t)((void *)source_light) - source_begin) <= data_bytes && (104) <= data_bytes - ((uintptr_t)((void *)source_light) - source_begin) ? (uint32)((uintptr_t)((void *)source_light) - source_begin) : (fprintf(stderr, "P2: Scene source pointer exceeds native input\n"), abort(), 0u));
            if (light_count >= data_bytes / sizeof(uint32) || source_offset > data_bytes || 104 > data_bytes - source_offset)
            {
                fprintf(stderr, "P2: Scene light registry exceeds resource or is cyclic\n");
                abort();
            }
            ++light_count;
            uint32 next_offset = xport_load_le32((const uint8 *)source_light);
            source_light = next_offset != 0 ? (const SceneLight *)((next_offset) <= data_bytes && (104) <= data_bytes - (next_offset) ? data + (next_offset) : (fprintf(stderr, "P2: Scene file span exceeds native input\n"), abort(), data)) : 0;
        }
        while (source_mesh != 0)
        {
            ++mesh_count;
            uint32 next_offset = xport_load_le32((const uint8 *)source_mesh);
            source_mesh = next_offset != 0 ? (const MeshGeometryNode *)((next_offset) <= data_bytes && (52) <= data_bytes - (next_offset) ? data + (next_offset) : (fprintf(stderr, "P2: Scene file span exceeds native input\n"), abort(), data)) : 0;
        }
        while (source_billboard != 0)
        {
            uint32 source_offset = ((uintptr_t)((void *)source_billboard) >= source_begin && ((uintptr_t)((void *)source_billboard) - source_begin) <= data_bytes && (52) <= data_bytes - ((uintptr_t)((void *)source_billboard) - source_begin) ? (uint32)((uintptr_t)((void *)source_billboard) - source_begin) : (fprintf(stderr, "P2: Scene source pointer exceeds native input\n"), abort(), 0u));
            if (billboard_count >= data_bytes / sizeof(uint32) || source_offset > data_bytes || 52 > data_bytes - source_offset)
            {
                fprintf(stderr, "P2: Scene billboard registry exceeds resource or is cyclic\n");
                abort();
            }
            ++billboard_count;
            uint32 next_offset = xport_load_le32((const uint8 *)source_billboard);
            source_billboard = next_offset != 0 ? (const PositionedBillboardNode *)((next_offset) <= data_bytes && (52) <= data_bytes - (next_offset) ? data + (next_offset) : (fprintf(stderr, "P2: Scene file span exceeds native input\n"), abort(), data)) : 0;
        }
        while (source_part != 0)
        {
            uint32 source_offset = ((uintptr_t)((void *)source_part) >= source_begin && ((uintptr_t)((void *)source_part) - source_begin) <= data_bytes && (48) <= data_bytes - ((uintptr_t)((void *)source_part) - source_begin) ? (uint32)((uintptr_t)((void *)source_part) - source_begin) : (fprintf(stderr, "P2: Scene source pointer exceeds native input\n"), abort(), 0u));
            if (mesh_part_count >= data_bytes / sizeof(uint32) || source_offset > data_bytes || 48 > data_bytes - source_offset)
            {
                fprintf(stderr, "P2: Scene mesh part registry exceeds resource or is cyclic\n");
                abort();
            }
            ++mesh_part_count;
            uint32 next_offset = xport_load_le32((const uint8 *)source_part);
            source_part = next_offset != 0 ? (const MeshPartNode *)((next_offset) <= data_bytes && (48) <= data_bytes - (next_offset) ? data + (next_offset) : (fprintf(stderr, "P2: Scene file span exceeds native input\n"), abort(), data)) : 0;
        }
        while (source_sphere != 0)
        {
            uint32 source_offset = ((uintptr_t)((void *)source_sphere) >= source_begin && ((uintptr_t)((void *)source_sphere) - source_begin) <= data_bytes && (36) <= data_bytes - ((uintptr_t)((void *)source_sphere) - source_begin) ? (uint32)((uintptr_t)((void *)source_sphere) - source_begin) : (fprintf(stderr, "P2: Scene source pointer exceeds native input\n"), abort(), 0u));
            if (sphere_node_count >= data_bytes / sizeof(uint32) || source_offset > data_bytes || 36 > data_bytes - source_offset)
            {
                fprintf(stderr, "P2: Scene sphere registry exceeds resource or is cyclic\n");
                abort();
            }
            ++sphere_node_count;
            uint32 next_offset = xport_load_le32((const uint8 *)source_sphere);
            source_sphere = next_offset != 0 ? (const SphereNode *)((next_offset) <= data_bytes && (36) <= data_bytes - (next_offset) ? data + (next_offset) : (fprintf(stderr, "P2: Scene file span exceeds native input\n"), abort(), data)) : 0;
        }
        while (source_sprite != 0)
        {
            ++sprite_count;
            uint32 next_offset = xport_load_le32((const uint8 *)source_sprite);
            source_sprite = next_offset != 0 ? (const SpriteRecord *)((next_offset) <= data_bytes && (48) <= data_bytes - (next_offset) ? data + (next_offset) : (fprintf(stderr, "P2: Scene file span exceeds native input\n"), abort(), data)) : 0;
        }
        for (uint32 trigger_group = 4; trigger_group <= 6; ++trigger_group)
        {
            const SceneNode *source_trigger = object_heads[trigger_group];
            while (source_trigger != 0)
            {
                uint32 source_offset = ((uintptr_t)((void *)source_trigger) >= source_begin && ((uintptr_t)((void *)source_trigger) - source_begin) <= data_bytes && (SceneTrigger::ControlBytes) <= data_bytes - ((uintptr_t)((void *)source_trigger) - source_begin) ? (uint32)((uintptr_t)((void *)source_trigger) - source_begin) : (fprintf(stderr, "P2: Scene source pointer exceeds native input\n"), abort(), 0u));
                if (trigger_count >= data_bytes / sizeof(uint32) || source_offset > data_bytes || SceneTrigger::ControlBytes > data_bytes - source_offset)
                {
                    fprintf(stderr, "P2: Scene trigger registry exceeds resource or is cyclic\n");
                    abort();
                }
                ++trigger_count;
                uint32 next_offset = xport_load_le32((const uint8 *)source_trigger);
                source_trigger = next_offset != 0 ? (const SceneNode *)((next_offset) <= data_bytes && (SceneTrigger::ControlBytes) <= data_bytes - (next_offset) ? data + (next_offset) : (fprintf(stderr, "P2: Scene file span exceeds native input\n"), abort(), data)) : 0;
            }
        }
        owner_count = light_count + mesh_count + path_count + billboard_count + mesh_part_count + sphere_node_count + trigger_count + sprite_count + animated_object_count + placement_count;
        actor_limit = owner_count - placement_count;
        actor_first = actor_limit - animated_object_count;
        sprite_limit = actor_first;
        sprite_first = sprite_limit - sprite_count;
        trigger_limit = sprite_first;
        trigger_first = trigger_limit - trigger_count;
        lights = light_count != 0 ? (SceneLight *)malloc(light_count * sizeof(SceneLight)) : 0;
        meshes = mesh_count != 0 ? (MeshGeometryNode *)malloc(mesh_count * sizeof(MeshGeometryNode)) : 0;
        billboards = billboard_count != 0 ? (PositionedBillboardNode *)malloc(billboard_count * sizeof(PositionedBillboardNode)) : 0;
        mesh_parts = mesh_part_count != 0 ? (MeshPartNode *)malloc(mesh_part_count * sizeof(MeshPartNode)) : 0;
        sphere_nodes = sphere_node_count != 0 ? (SphereNode *)malloc(sphere_node_count * sizeof(SphereNode)) : 0;
        triggers = trigger_count != 0 ? (SceneTrigger *)malloc(trigger_count * sizeof(SceneTrigger)) : 0;
        sprites = sprite_count != 0 ? (SpriteRecord *)malloc(sprite_count * sizeof(SpriteRecord)) : 0;
        source_nodes = owner_count != 0 ? (const SceneNode **)malloc(owner_count * sizeof(*source_nodes)) : 0;
        native_nodes = owner_count != 0 ? (SceneNode **)malloc(owner_count * sizeof(*native_nodes)) : 0;
        if ((light_count != 0 && lights == 0) || (mesh_count != 0 && meshes == 0) || (path_count != 0 && paths == 0) || (billboard_count != 0 && billboards == 0) || (mesh_part_count != 0 && mesh_parts == 0) || (sphere_node_count != 0 && sphere_nodes == 0) || (placement_count != 0 && placements == 0) || (animated_object_count != 0 && animated_objects == 0) || (sprite_count != 0 && sprites == 0) || (trigger_count != 0 && triggers == 0) || (owner_count != 0 && (source_nodes == 0 || native_nodes == 0)))
        {
            fprintf(stderr, "P2: Cannot allocate native scene headers\n");
            abort();
        }
        source_light = (const SceneLight *)object_heads[14];
        owner_index = 0;
        for (light_index = 0; light_index < light_count; ++light_index)
        {
            uint32 source_offset = ((uintptr_t)((void *)source_light) >= source_begin && ((uintptr_t)((void *)source_light) - source_begin) <= data_bytes && (104) <= data_bytes - ((uintptr_t)((void *)source_light) - source_begin) ? (uint32)((uintptr_t)((void *)source_light) - source_begin) : (fprintf(stderr, "P2: Scene source pointer exceeds native input\n"), abort(), 0u));
            if (source_offset > data_bytes || 104 > data_bytes - source_offset)
            {
                fprintf(stderr, "P2: Scene light header exceeds resource extent\n");
                abort();
            }
            source_nodes[owner_index] = (const SceneNode *)source_light;
            native_nodes[owner_index] = (SceneNode *)(lights + light_index);
            const uint8 *encoded = (const uint8 *)source_light;
            SceneLight *decoded = lights + light_index;
            SceneNode **link_fields[] = {&decoded->links.registry_next, &decoded->links.sibling_next, &decoded->links.first_child};
            for (uint32 link_index = 0; link_index < 3; ++link_index)
            {
                uint32 link_offset = xport_load_le32(encoded + link_index * 4);
                if (link_offset != 0 && (link_offset > data_bytes || 16 > data_bytes - link_offset))
                {
                    fprintf(stderr, "P2: Scene light link exceeds resource\n");
                    abort();
                }
                *link_fields[link_index] = link_offset != 0 ? (SceneNode *)((link_offset) <= data_bytes && (16) <= data_bytes - (link_offset) ? data + (link_offset) : (fprintf(stderr, "P2: Scene file span exceeds native input\n"), abort(), data)) : 0;
            }
            decoded->kind = encoded[12];
            decoded->flags = encoded[13];
            decoded->event_flags = encoded[14];
            decoded->id = encoded[15];
            decoded->event_id = xport_load_le32(encoded + 16);
            for (uint32 component = 0; component < 3; ++component)
            {
                decoded->position[component] = xport_load_le32(encoded + 20 + component * 4);
                decoded->source.position[component] = xport_load_le32(encoded + 32 + component * 4);
                decoded->prev_position[component] = xport_load_le32(encoded + 44 + component * 4);
                decoded->relative_position[component] = xport_load_le32(encoded + 56 + component * 4);
                decoded->color[component] = xport_load_le32(encoded + 68 + component * 4);
                decoded->color_scale[component] = xport_load_le32(encoded + 92 + component * 4);
            }
            decoded->radius = xport_load_le32(encoded + 80);
            decoded->attenuation_range = xport_load_le32(encoded + 84);
            decoded->metadata_word = xport_load_le32(encoded + 88);
            ++owner_index;
            uint32 next_offset = xport_load_le32(encoded);
            source_light = next_offset != 0 ? (const SceneLight *)((next_offset) <= data_bytes && (104) <= data_bytes - (next_offset) ? data + (next_offset) : (fprintf(stderr, "P2: Scene file span exceeds native input\n"), abort(), data)) : 0;
        }
        source_mesh = (const MeshGeometryNode *)object_heads[2];
        for (mesh_index = 0; mesh_index < mesh_count; ++mesh_index)
        {
            uint32 source_offset = ((uintptr_t)((void *)source_mesh) >= source_begin && ((uintptr_t)((void *)source_mesh) - source_begin) <= data_bytes && (52) <= data_bytes - ((uintptr_t)((void *)source_mesh) - source_begin) ? (uint32)((uintptr_t)((void *)source_mesh) - source_begin) : (fprintf(stderr, "P2: Scene source pointer exceeds native input\n"), abort(), 0u));
            if (source_offset > data_bytes || 52 > data_bytes - source_offset)
            {
                fprintf(stderr, "P2: Scene mesh header exceeds resource extent\n");
                abort();
            }
            source_nodes[owner_index] = &source_mesh->node;
            native_nodes[owner_index] = &meshes[mesh_index].node;
            const uint8 *encoded = (const uint8 *)source_mesh;
            MeshGeometryNode *mesh = meshes + mesh_index;
            SceneNode **link_fields[] = {&mesh->node.links.registry_next, &mesh->node.links.sibling_next, &mesh->node.links.first_child};
            for (uint32 link_index = 0; link_index < 3; ++link_index)
            {
                uint32 link_offset = xport_load_le32(encoded + link_index * 4);
                if (link_offset != 0 && (link_offset > data_bytes || 16 > data_bytes - link_offset))
                {
                    fprintf(stderr, "P2: Scene mesh link exceeds resource\n");
                    abort();
                }
                *link_fields[link_index] = link_offset != 0 ? (SceneNode *)((link_offset) <= data_bytes && (16) <= data_bytes - (link_offset) ? data + (link_offset) : (fprintf(stderr, "P2: Scene file span exceeds native input\n"), abort(), data)) : 0;
            }
            mesh->node.kind = encoded[12];
            mesh->node.flags = encoded[13];
            mesh->node.state_flags = xport_load_le16(encoded + 14);
            for (uint32 component = 0; component < 3; ++component)
                mesh->position[component] = xport_load_le32(encoded + 16 + component * 4);
            mesh->flags = xport_load_le32(encoded + 28);
            mesh->polygon_count = (sint16)xport_load_le16(encoded + 32);
            mesh->vertex_count = (sint16)xport_load_le16(encoded + 34);
            uint32 polygon_offset = xport_load_le32(encoded + 36);
            if (polygon_offset != 0 && (polygon_offset > data_bytes || 24 > data_bytes - polygon_offset))
            {
                fprintf(stderr, "P2: Scene mesh polygon source exceeds resource\n");
                abort();
            }
            mesh->polygons = polygon_offset != 0 ? (MeshFace *)((polygon_offset) <= data_bytes && (24) <= data_bytes - (polygon_offset) ? data + (polygon_offset) : (fprintf(stderr, "P2: Scene file span exceeds native input\n"), abort(), data)) : 0;
            uint32 geometry_offset = xport_load_le32(encoded + 40);
            uint32 geometry_bytes = mesh->vertex_count > 0 ? 16 : 52;
            if (geometry_offset != 0 && (geometry_offset > data_bytes || geometry_bytes > data_bytes - geometry_offset))
            {
                fprintf(stderr, "P2: Scene mesh vertex or shared mesh source exceeds resource\n");
                abort();
            }
            if (mesh->vertex_count > 0)
                mesh->vertices = geometry_offset != 0 ? (MeshVertex *)((geometry_offset) <= data_bytes && (16) <= data_bytes - (geometry_offset) ? data + (geometry_offset) : (fprintf(stderr, "P2: Scene file span exceeds native input\n"), abort(), data)) : 0;
            else
                mesh->source_mesh = geometry_offset != 0 ? (MeshGeometryNode *)((geometry_offset) <= data_bytes && (52) <= data_bytes - (geometry_offset) ? data + (geometry_offset) : (fprintf(stderr, "P2: Scene file span exceeds native input\n"), abort(), data)) : 0;
            uint32 material_offset = xport_load_le32(encoded + 44);
            if (material_offset != 0 && (material_offset > data_bytes || sizeof(uint32) > data_bytes - material_offset))
            {
                fprintf(stderr, "P2: Scene mesh material list exceeds resource\n");
                abort();
            }
            mesh->material_animation_entries = material_offset != 0 ? (uint32 *)((material_offset) <= data_bytes && (sizeof(uint32)) <= data_bytes - (material_offset) ? data + (material_offset) : (fprintf(stderr, "P2: Scene file span exceeds native input\n"), abort(), data)) : 0;
            mesh->lighting_index = xport_load_le16(encoded + 48);
            mesh->first_vertex = (sint16)xport_load_le16(encoded + 50);
            ++owner_index;
            uint32 next_offset = xport_load_le32(encoded);
            source_mesh = next_offset != 0 ? (const MeshGeometryNode *)((next_offset) <= data_bytes && (52) <= data_bytes - (next_offset) ? data + (next_offset) : (fprintf(stderr, "P2: Scene file span exceeds native input\n"), abort(), data)) : 0;
        }
        for (path_index = 0; path_index < path_count; ++path_index)
        {
            source_nodes[owner_index] = (const SceneNode *)((source_path_offsets[path_index]) <= data_bytes && (56) <= data_bytes - (source_path_offsets[path_index]) ? data + (source_path_offsets[path_index]) : (fprintf(stderr, "P2: Scene file span exceeds native input\n"), abort(), data));
            native_nodes[owner_index] = &paths[path_index].node;
            ++owner_index;
        }
        source_billboard = (const PositionedBillboardNode *)object_heads[8];
        for (billboard_index = 0; billboard_index < billboard_count; ++billboard_index)
        {
            uint32 source_offset = ((uintptr_t)((void *)source_billboard) >= source_begin && ((uintptr_t)((void *)source_billboard) - source_begin) <= data_bytes && (52) <= data_bytes - ((uintptr_t)((void *)source_billboard) - source_begin) ? (uint32)((uintptr_t)((void *)source_billboard) - source_begin) : (fprintf(stderr, "P2: Scene source pointer exceeds native input\n"), abort(), 0u));
            if (source_offset > data_bytes || 52 > data_bytes - source_offset)
            {
                fprintf(stderr, "P2: Scene billboard header exceeds resource extent\n");
                abort();
            }
            source_nodes[owner_index] = &source_billboard->billboard.node;
            native_nodes[owner_index] = &billboards[billboard_index].billboard.node;
            const uint8 *encoded = (const uint8 *)source_billboard;
            PositionedBillboardNode *decoded = billboards + billboard_index;
            SceneNode **link_fields[] = {&decoded->billboard.node.links.registry_next, &decoded->billboard.node.links.sibling_next, &decoded->billboard.node.links.first_child};
            for (uint32 link_index = 0; link_index < 3; ++link_index)
            {
                uint32 link_offset = xport_load_le32(encoded + link_index * 4);
                if (link_offset != 0 && (link_offset > data_bytes || 16 > data_bytes - link_offset))
                {
                    fprintf(stderr, "P2: Scene billboard link exceeds resource\n");
                    abort();
                }
                *link_fields[link_index] = link_offset != 0 ? (SceneNode *)((link_offset) <= data_bytes && (16) <= data_bytes - (link_offset) ? data + (link_offset) : (fprintf(stderr, "P2: Scene file span exceeds native input\n"), abort(), data)) : 0;
            }
            decoded->billboard.node.kind = encoded[12];
            decoded->billboard.node.flags = encoded[13];
            decoded->billboard.node.state_flags = xport_load_le16(encoded + 14);
            decoded->billboard.metadata16 = xport_load_le16(encoded + 16);
            decoded->billboard.playback_flags = xport_load_le16(encoded + 18);
            uint32 type = decoded->billboard.playback_flags & 0x7F;
            if (type >= sizeof(resource_groups) / sizeof(resource_groups[0]) - 13)
            {
                fprintf(stderr, "P2: Scene billboard texture group exceeds table\n");
                abort();
            }
            decoded->billboard.texture = resource_groups[type + 13].texture;
            decoded->billboard.extent = (sint32)xport_load_le32(encoded + 24);
            for (uint32 component = 0; component < 3; ++component)
            {
                decoded->position[component] = xport_load_le32(encoded + 28 + component * 4);
                decoded->world_position[component] = xport_load_le32(encoded + 40 + component * 4);
            }
            ++owner_index;
            uint32 next_offset = xport_load_le32(encoded);
            source_billboard = next_offset != 0 ? (const PositionedBillboardNode *)((next_offset) <= data_bytes && (52) <= data_bytes - (next_offset) ? data + (next_offset) : (fprintf(stderr, "P2: Scene file span exceeds native input\n"), abort(), data)) : 0;
        }
        source_part = (const MeshPartNode *)object_heads[13];
        for (part_index = 0; part_index < mesh_part_count; ++part_index)
        {
            uint32 source_offset = ((uintptr_t)((void *)source_part) >= source_begin && ((uintptr_t)((void *)source_part) - source_begin) <= data_bytes && (48) <= data_bytes - ((uintptr_t)((void *)source_part) - source_begin) ? (uint32)((uintptr_t)((void *)source_part) - source_begin) : (fprintf(stderr, "P2: Scene source pointer exceeds native input\n"), abort(), 0u));
            if (source_offset > data_bytes || 48 > data_bytes - source_offset)
            {
                fprintf(stderr, "P2: Scene mesh part header exceeds resource extent\n");
                abort();
            }
            source_nodes[owner_index] = &source_part->node;
            native_nodes[owner_index] = &mesh_parts[part_index].node;
            const uint8 *encoded = (const uint8 *)source_part;
            MeshPartNode *decoded = mesh_parts + part_index;
            SceneNode **link_fields[] = {&decoded->node.links.registry_next, &decoded->node.links.sibling_next, &decoded->node.links.first_child};
            for (uint32 link_index = 0; link_index < 3; ++link_index)
            {
                uint32 link_offset = xport_load_le32(encoded + link_index * 4);
                if (link_offset != 0 && (link_offset > data_bytes || 16 > data_bytes - link_offset))
                {
                    fprintf(stderr, "P2: Scene mesh part link exceeds resource\n");
                    abort();
                }
                *link_fields[link_index] = link_offset != 0 ? (SceneNode *)((link_offset) <= data_bytes && (16) <= data_bytes - (link_offset) ? data + (link_offset) : (fprintf(stderr, "P2: Scene file span exceeds native input\n"), abort(), data)) : 0;
            }
            decoded->node.kind = encoded[12];
            decoded->node.flags = encoded[13];
            decoded->node.state_flags = xport_load_le16(encoded + 14);
            for (uint32 component = 0; component < 3; ++component)
                decoded->position[component] = xport_load_le32(encoded + 16 + component * 4);
            decoded->animation_binding.word = xport_load_le32(encoded + 28);
            decoded->span.first_vertex = (sint16)xport_load_le16(encoded + 32);
            decoded->span.vertex_count = (sint16)xport_load_le16(encoded + 34);
            decoded->span.first_polygon = (sint16)xport_load_le16(encoded + 36);
            decoded->span.polygon_count = (sint16)xport_load_le16(encoded + 38);
            for (uint32 component = 0; component < 4; ++component)
                decoded->rotation[component] = (sint16)xport_load_le16(encoded + 40 + component * 2);
            ++owner_index;
            uint32 next_offset = xport_load_le32(encoded);
            source_part = next_offset != 0 ? (const MeshPartNode *)((next_offset) <= data_bytes && (48) <= data_bytes - (next_offset) ? data + (next_offset) : (fprintf(stderr, "P2: Scene file span exceeds native input\n"), abort(), data)) : 0;
        }
        source_sphere = (const SphereNode *)object_heads[15];
        for (sphere_index = 0; sphere_index < sphere_node_count; ++sphere_index)
        {
            uint32 source_offset = ((uintptr_t)((void *)source_sphere) >= source_begin && ((uintptr_t)((void *)source_sphere) - source_begin) <= data_bytes && (36) <= data_bytes - ((uintptr_t)((void *)source_sphere) - source_begin) ? (uint32)((uintptr_t)((void *)source_sphere) - source_begin) : (fprintf(stderr, "P2: Scene source pointer exceeds native input\n"), abort(), 0u));
            if (source_offset > data_bytes || 36 > data_bytes - source_offset)
            {
                fprintf(stderr, "P2: Scene sphere header exceeds resource extent\n");
                abort();
            }
            source_nodes[owner_index] = &source_sphere->node;
            native_nodes[owner_index] = &sphere_nodes[sphere_index].node;
            const uint8 *encoded = (const uint8 *)source_sphere;
            SphereNode *decoded = sphere_nodes + sphere_index;
            SceneNode **link_fields[] = {&decoded->node.links.registry_next, &decoded->node.links.sibling_next, &decoded->node.links.first_child};
            for (uint32 link_index = 0; link_index < 3; ++link_index)
            {
                uint32 link_offset = xport_load_le32(encoded + link_index * 4);
                if (link_offset != 0 && (link_offset > data_bytes || 16 > data_bytes - link_offset))
                {
                    fprintf(stderr, "P2: Scene sphere link exceeds resource\n");
                    abort();
                }
                *link_fields[link_index] = link_offset != 0 ? (SceneNode *)((link_offset) <= data_bytes && (16) <= data_bytes - (link_offset) ? data + (link_offset) : (fprintf(stderr, "P2: Scene file span exceeds native input\n"), abort(), data)) : 0;
            }
            decoded->node.kind = encoded[12];
            decoded->node.flags = encoded[13];
            decoded->node.state_flags = xport_load_le16(encoded + 14);
            for (uint32 component = 0; component < 3; ++component)
                decoded->position[component] = xport_load_le32(encoded + 20 + component * 4);
            decoded->animation_binding.word = xport_load_le32(encoded + 16);
            decoded->radius = xport_load_le32(encoded + 32);
            ++owner_index;
            uint32 next_offset = xport_load_le32(encoded);
            source_sphere = next_offset != 0 ? (const SphereNode *)((next_offset) <= data_bytes && (36) <= data_bytes - (next_offset) ? data + (next_offset) : (fprintf(stderr, "P2: Scene file span exceeds native input\n"), abort(), data)) : 0;
        }
        trigger_index = 0;
        for (uint32 trigger_group = 4; trigger_group <= 6; ++trigger_group)
        {
            const SceneTrigger *source_trigger = (const SceneTrigger *)object_heads[trigger_group];
            while (source_trigger != 0)
            {
                uint32 source_offset = ((uintptr_t)((void *)source_trigger) >= source_begin && ((uintptr_t)((void *)source_trigger) - source_begin) <= data_bytes && (SceneTrigger::ControlBytes) <= data_bytes - ((uintptr_t)((void *)source_trigger) - source_begin) ? (uint32)((uintptr_t)((void *)source_trigger) - source_begin) : (fprintf(stderr, "P2: Scene source pointer exceeds native input\n"), abort(), 0u));
                const uint8 *encoded = (const uint8 *)source_trigger;
                uint32 source_bytes = (xport_load_le16(encoded + 14) & 2) != 0 ? SceneTrigger::ControlBytes : SceneTrigger::AnimatedBytes;
                if (source_offset > data_bytes || source_bytes > data_bytes - source_offset)
                {
                    fprintf(stderr, "P2: Scene trigger exceeds resource extent\n");
                    abort();
                }
                SceneTrigger *trigger = triggers + trigger_index;
                memset(trigger, 0, sizeof(SceneTrigger));
                SceneNode **link_fields[] = {&trigger->node.links.registry_next, &trigger->node.links.sibling_next, &trigger->node.links.first_child};
                for (uint32 link_index = 0; link_index < 3; ++link_index)
                {
                    uint32 link_offset = xport_load_le32(encoded + link_index * 4);
                    if (link_offset != 0 && (link_offset > data_bytes || 16 > data_bytes - link_offset))
                    {
                        fprintf(stderr, "P2: Scene trigger link exceeds resource\n");
                        abort();
                    }
                    *link_fields[link_index] = link_offset != 0 ? (SceneNode *)((link_offset) <= data_bytes && (16) <= data_bytes - (link_offset) ? data + (link_offset) : (fprintf(stderr, "P2: Scene file span exceeds native input\n"), abort(), data)) : 0;
                }
                trigger->node.kind = encoded[12];
                trigger->node.flags = encoded[13];
                trigger->node.state_flags = xport_load_le16(encoded + 14);
                for (uint32 row = 0; row < 3; ++row)
                {
                    for (uint32 column = 0; column < 3; ++column)
                        trigger->transform.m[row][column] = (sint16)xport_load_le16(encoded + 16 + (row * 3 + column) * 2);
                    trigger->transform.t[row] = (sint32)xport_load_le32(encoded + 36 + row * 4);
                    trigger->delta[row] = xport_load_le32(encoded + 48 + row * 4);
                    trigger->extent[row] = xport_load_le16(encoded + 60 + row * 2);
                    trigger->parameters.words[row] = xport_load_le32(encoded + 68 + row * 4);
                }
                trigger->radius = xport_load_le16(encoded + 66);
                trigger->control_word = xport_load_le32(encoded + 80);
                sint32 trigger_kind = (sint8)(trigger->node.state_flags >> 8);
                uint32 path_count = trigger_group == 6 ? 2 : (trigger_group == 4 && (trigger_kind == 7 || ((uint32)(trigger_kind - 26) & 0xFF) < 2) ? 1 : 0);
                for (uint32 path_index = 0; path_index < path_count; ++path_index)
                {
                    uint32 path_offset = xport_load_le32(encoded + 68 + path_index * 4);
                    if (path_offset != 0 && (path_offset > data_bytes || 56 > data_bytes - path_offset))
                    {
                        fprintf(stderr, "P2: Scene trigger path exceeds resource\n");
                        abort();
                    }
                    const ScenePath *path = path_offset != 0 ? (const ScenePath *)((path_offset) <= data_bytes && (56) <= data_bytes - (path_offset) ? data + (path_offset) : (fprintf(stderr, "P2: Scene file span exceeds native input\n"), abort(), data)) : 0;
                    if (trigger_group == 6)
                        trigger->parameters.path_pair.targets[path_index] = path;
                    else
                        trigger->parameters.path.target = path;
                }
                if (trigger_group == 6)
                    trigger->parameters.path_pair.metadata = xport_load_le32(encoded + 76);
                else if (path_count != 0)
                {
                    trigger->parameters.path.values[0] = xport_load_le32(encoded + 72);
                    trigger->parameters.path.values[1] = xport_load_le32(encoded + 76);
                }
                if (source_bytes == SceneTrigger::ControlBytes)
                    trigger->animation_binding.word = 0xFFFFFFFF;
                else
                {
                    trigger->animation_binding.word = xport_load_le32(encoded + 84);
                    for (uint32 component = 0; component < 3; ++component)
                        trigger->position[component] = xport_load_le32(encoded + 88 + component * 4);
                    for (uint32 component = 0; component < 4; ++component)
                        trigger->rotation[component] = (sint16)xport_load_le16(encoded + 100 + component * 2);
                }
                source_nodes[owner_index] = &source_trigger->node;
                native_nodes[owner_index] = &trigger->node;
                ++owner_index;
                ++trigger_index;
                uint32 next_offset = xport_load_le32(encoded);
                source_trigger = next_offset != 0 ? (const SceneTrigger *)((next_offset) <= data_bytes && (SceneTrigger::ControlBytes) <= data_bytes - (next_offset) ? data + (next_offset) : (fprintf(stderr, "P2: Scene file span exceeds native input\n"), abort(), data)) : 0;
            }
        }
        source_sprite = (const SpriteRecord *)object_heads[12];
        for (sprite_index = 0; sprite_index < sprite_count; ++sprite_index)
        {
            uint32 source_offset = ((uintptr_t)((void *)source_sprite) >= source_begin && ((uintptr_t)((void *)source_sprite) - source_begin) <= data_bytes && (48) <= data_bytes - ((uintptr_t)((void *)source_sprite) - source_begin) ? (uint32)((uintptr_t)((void *)source_sprite) - source_begin) : (fprintf(stderr, "P2: Scene source pointer exceeds native input\n"), abort(), 0u));
            if (source_offset > data_bytes || 48 > data_bytes - source_offset)
            {
                fprintf(stderr, "P2: Scene sprite exceeds resource extent\n");
                abort();
            }
            source_nodes[owner_index] = &source_sprite->node;
            native_nodes[owner_index] = &sprites[sprite_index].node;
            const uint8 *encoded = (const uint8 *)source_sprite;
            SpriteRecord *sprite = sprites + sprite_index;
            SceneNode **link_fields[] = {&sprite->node.links.registry_next, &sprite->node.links.sibling_next, &sprite->node.links.first_child};
            for (uint32 link_index = 0; link_index < 3; ++link_index)
            {
                uint32 link_offset = xport_load_le32(encoded + link_index * 4);
                if (link_offset != 0 && (link_offset > data_bytes || 16 > data_bytes - link_offset))
                {
                    fprintf(stderr, "P2: Scene sprite link exceeds resource\n");
                    abort();
                }
                *link_fields[link_index] = link_offset != 0 ? (SceneNode *)((link_offset) <= data_bytes && (16) <= data_bytes - (link_offset) ? data + (link_offset) : (fprintf(stderr, "P2: Scene file span exceeds native input\n"), abort(), data)) : 0;
            }
            sprite->node.kind = encoded[12];
            sprite->node.flags = encoded[13];
            sprite->node.state_flags = xport_load_le16(encoded + 14);
            uint32 texture_index = xport_load_le32(encoded + 16);
            if (texture_index >= resource->texture_count)
            {
                fprintf(stderr, "P2: Scene sprite texture exceeds catalog\n");
                abort();
            }
            sprite->texture = resource->textures + texture_index;
            uint32 payload = xport_load_le32(encoded + 20);
            if (payload != 0 && (payload > data_bytes || sizeof(uint32) > data_bytes - payload))
            {
                fprintf(stderr, "P2: Scene sprite material list exceeds resource\n");
                abort();
            }
            sprite->animation_entries = payload != 0 ? (uint32 *)((payload) <= data_bytes && (sizeof(uint32)) <= data_bytes - (payload) ? data + (payload) : (fprintf(stderr, "P2: Scene file span exceeds native input\n"), abort(), data)) : 0;
            sprite->animation_binding.word = xport_load_le32(encoded + 24);
            sprite->extent_track = 0;
            for (uint32 component = 0; component < 3; ++component)
                sprite->position[component] = xport_load_le32(encoded + 28 + component * 4);
            for (uint32 component = 0; component < 2; ++component)
            {
                sprite->extent[component] = (sint16)xport_load_le16(encoded + 40 + component * 2);
                sprite->metadata_halfwords[component] = xport_load_le16(encoded + 44 + component * 2);
            }
            if (sprite->animation_binding.tracks[3] != -1)
            {
                payload = sprite->animation_binding.word;
                uint32 track_index = 0;
                while (track_index < sprite_extent_track_count && source_extent_offsets[track_index] != payload)
                    ++track_index;
                if (track_index == sprite_extent_track_count)
                {
                    if (sprite_extent_track_count == extent_capacity || (payload & 3) != 0 || payload > data_bytes || sizeof(EncodedAnimationTrack) > data_bytes - payload)
                    {
                        fprintf(stderr, "P2: Sprite extent track exceeds scene payload\n");
                        abort();
                    }
                    const EncodedAnimationTrack *encoded = (const EncodedAnimationTrack *)((payload) <= data_bytes && (sizeof(EncodedAnimationTrack)) <= data_bytes - (payload) ? data + (payload) : (fprintf(stderr, "P2: Scene file span exceeds native input\n"), abort(), data));
                    uint32 key_offset = xport_load_le32(&encoded->keys_offset);
                    uint32 key_count = encoded->state[0];
                    uint32 key_bytes = key_offset != 0 ? key_count * sizeof(VectorAnimationKey) : 0;
                    uint32 prefix_bytes = 0;
                    if (key_offset != 0)
                    {
                        if ((key_offset & 3) != 0 || key_offset > data_bytes || key_bytes > data_bytes - key_offset)
                        {
                            fprintf(stderr, "P2: Sprite extent keys exceed scene payload\n");
                            abort();
                        }
                        const uint8 *keys = (const uint8 *)((key_offset) <= data_bytes && (key_bytes) <= data_bytes - (key_offset) ? data + (key_offset) : (fprintf(stderr, "P2: Scene file span exceeds native input\n"), abort(), data));
                        for (uint32 key_index = 0; key_index < key_count; ++key_index)
                        {
                            uint32 prefix = (xport_load_le32(keys + key_index * sizeof(VectorAnimationKey)) & 0xFF) * sizeof(AnimationEaseCurve);
                            if (prefix > prefix_bytes)
                                prefix_bytes = prefix;
                        }
                        if (prefix_bytes > key_offset)
                        {
                            fprintf(stderr, "P2: Sprite extent easing prefix exceeds scene payload\n");
                            abort();
                        }
                    }
                    AnimationTrack *track = (AnimationTrack *)malloc(sizeof(AnimationTrack) + prefix_bytes + key_bytes);
                    if (track == 0)
                    {
                        fprintf(stderr, "P2: Cannot allocate native sprite extent track\n");
                        abort();
                    }
                    memcpy(track, encoded->state, sizeof(encoded->state));
                    track->keys = 0;
                    if (key_offset != 0)
                    {
                        uint8 *storage = (uint8 *)(track + 1);
                        memcpy(storage, ((key_offset - prefix_bytes) <= data_bytes && (prefix_bytes + key_bytes) <= data_bytes - (key_offset - prefix_bytes) ? data + (key_offset - prefix_bytes) : (fprintf(stderr, "P2: Scene file span exceeds native input\n"), abort(), data)), prefix_bytes + key_bytes);
                        track->keys = storage + prefix_bytes;
                    }
                    source_extent_offsets[track_index] = payload;
                    sprite_extent_tracks[track_index] = track;
                    ++sprite_extent_track_count;
                }
                sprite->extent_track = sprite_extent_tracks[track_index];
            }
            ++owner_index;
            uint32 next_offset = xport_load_le32(encoded);
            source_sprite = next_offset != 0 ? (const SpriteRecord *)((next_offset) <= data_bytes && (48) <= data_bytes - (next_offset) ? data + (next_offset) : (fprintf(stderr, "P2: Scene file span exceeds native input\n"), abort(), data)) : 0;
        }
        free(source_extent_offsets);
        for (actor_index = 0; actor_index < animated_object_count; ++actor_index)
        {
            source_nodes[owner_index] = (const SceneNode *)((source_actor_offsets[actor_index]) <= data_bytes && (232) <= data_bytes - (source_actor_offsets[actor_index]) ? data + (source_actor_offsets[actor_index]) : (fprintf(stderr, "P2: Scene file span exceeds native input\n"), abort(), data));
            native_nodes[owner_index] = &animated_objects[actor_index].node;
            ++owner_index;
        }
        for (placement_index = 0; placement_index < placement_count; ++placement_index)
        {
            source_nodes[owner_index] = (const SceneNode *)((source_placement_offsets[placement_index]) <= data_bytes && (48) <= data_bytes - (source_placement_offsets[placement_index]) ? data + (source_placement_offsets[placement_index]) : (fprintf(stderr, "P2: Scene file span exceeds native input\n"), abort(), data));
            native_nodes[owner_index] = &placements[placement_index].path.node;
            ++owner_index;
        }
        if (owner_count != 0)
        {
            for (uint32 graph_group = 0; graph_group < 16; ++graph_group)
            {
                SceneNode *source_node = object_heads[graph_group];
                while (source_node != 0)
                {
                    SceneNode *working_node = source_node;
                    for (owner_index = 0; owner_index < owner_count; ++owner_index)
                    {
                        if (source_node == source_nodes[owner_index])
                        {
                            working_node = native_nodes[owner_index];
                            break;
                        }
                    }
                    SceneNode *next_node = working_node->links.registry_next;
                    SceneNode **fields[] = {&working_node->links.registry_next, &working_node->links.sibling_next, &working_node->links.first_child};
                    for (uint32 field_index = 0; field_index < 3; ++field_index)
                    {
                        for (owner_index = 0; owner_index < owner_count; ++owner_index)
                        {
                            if (*fields[field_index] == source_nodes[owner_index])
                            {
                                *fields[field_index] = native_nodes[owner_index];
                                break;
                            }
                        }
                    }
                    source_node = next_node;
                }
            }
            for (uint32 graph_group = 0; graph_group < 16; ++graph_group)
            {
                for (owner_index = 0; owner_index < owner_count; ++owner_index)
                {
                    if (object_heads[graph_group] == source_nodes[owner_index])
                    {
                        object_heads[graph_group] = native_nodes[owner_index];
                        break;
                    }
                }
            }
            SceneNode **root_fields[] = {&this->links.registry_next, &this->links.sibling_next, &this->links.first_child, &transient_links.registry_next, &transient_links.sibling_next, &transient_links.first_child, &registered_object_head, &character_controller.support.owner, &character_controller.support.contact, &character_controller.surface_interaction.selected, &sprite_pool.first, &sprite_pool.free_head, &sprite_pool.active_head};
            for (uint32 field_index = 0; field_index < sizeof(root_fields) / sizeof(root_fields[0]); ++field_index)
            {
                for (owner_index = 0; owner_index < owner_count; ++owner_index)
                {
                    if (*root_fields[field_index] == source_nodes[owner_index])
                    {
                        *root_fields[field_index] = native_nodes[owner_index];
                        break;
                    }
                }
            }
            for (uint32 slot = 0; slot < 12; ++slot)
            {
                SceneNode **fields[] = {&character_controller.trigger_objects[slot], &character_controller.pickup_objects[slot]};
                for (uint32 field_index = 0; field_index < 2; ++field_index)
                {
                    for (owner_index = 0; owner_index < owner_count; ++owner_index)
                    {
                        if (*fields[field_index] == source_nodes[owner_index])
                        {
                            *fields[field_index] = native_nodes[owner_index];
                            break;
                        }
                    }
                }
            }
            SceneTrigger **trigger_fields[] = {&character_controller.support.candidate, &character_controller.support.impulse_surface, &character_controller.path_transition.anchor, &character_controller.surface_interaction.environment, &character_controller.surface_interaction.jump_scale, &character_controller.surface_interaction.surface, &character_controller.surface_interaction.prev_surface, &character_controller.contacts.section_trigger, &character_controller.contacts.hide_trigger, &character_controller.contacts.show_trigger, &cutscene.trigger};
            for (uint32 field_index = 0; field_index < sizeof(trigger_fields) / sizeof(trigger_fields[0]); ++field_index)
            {
                for (owner_index = trigger_first; owner_index < trigger_limit; ++owner_index)
                {
                    if (*trigger_fields[field_index] == (const SceneTrigger *)source_nodes[owner_index])
                    {
                        *trigger_fields[field_index] = (SceneTrigger *)native_nodes[owner_index];
                        break;
                    }
                }
            }
            for (uint32 slot = 0; slot < SpritePoolState::EntryCount; ++slot)
            {
                for (owner_index = sprite_first; owner_index < sprite_limit; ++owner_index)
                {
                    if (sprite_pool.entries[slot].node == source_nodes[owner_index])
                    {
                        sprite_pool.entries[slot].node = native_nodes[owner_index];
                        break;
                    }
                }
            }
            for (uint32 slot = 0; slot < CharacterContactPool::Capacity; ++slot)
            {
                for (owner_index = 0; owner_index < owner_count; ++owner_index)
                {
                    if (character_contact_pool.nodes[slot].object == source_nodes[owner_index])
                    {
                        character_contact_pool.nodes[slot].object = native_nodes[owner_index];
                        break;
                    }
                }
            }
            for (ActorBinding *binding = actor_bindings; binding != 0; binding = binding->next)
            {
                for (owner_index = 0; owner_index < owner_count; ++owner_index)
                {
                    if (binding->event_node == source_nodes[owner_index])
                    {
                        binding->event_node = native_nodes[owner_index];
                        break;
                    }
                }
            }
            for (path_index = 0; path_index < path_count; ++path_index)
            {
                for (owner_index = owner_count - placement_count; owner_index < owner_count; ++owner_index)
                {
                    if (paths[path_index].definition == (const PathDefinition *)source_nodes[owner_index])
                    {
                        paths[path_index].definition = (const PathDefinition *)native_nodes[owner_index];
                        break;
                    }
                }
            }
            for (ActorBinding *binding = actor_bindings; binding != 0; binding = binding->next)
            {
                for (owner_index = owner_count - placement_count; owner_index < owner_count; ++owner_index)
                {
                    if (binding->definition == (const ActorPlacement *)source_nodes[owner_index])
                    {
                        binding->definition = (ActorPlacement *)native_nodes[owner_index];
                        break;
                    }
                }
            }
            {
                AnimatedObject **object_fields[3 + CharacterController::ModelCount + sizeof(action_effect_resources.templates) / sizeof(action_effect_resources.templates[0]) + sizeof(character_controller.action_effects) / sizeof(character_controller.action_effects[0]) + ModelGroupRegistry::Capacity * sizeof(model_groups.records[0].models) / sizeof(model_groups.records[0].models[0]) + sizeof(actor_models.definitions) / sizeof(actor_models.definitions[0])];
                uint32 object_count = 3;
                object_fields[0] = &character_controller.object;
                object_fields[1] = &character_action_owner;
                object_fields[2] = &character_action_target;
                for (uint32 slot = 0; slot < sizeof(action_effect_resources.templates) / sizeof(action_effect_resources.templates[0]); ++slot)
                    object_fields[object_count++] = &action_effect_resources.templates[slot];
                for (uint32 slot = 0; slot < sizeof(character_controller.action_effects) / sizeof(character_controller.action_effects[0]); ++slot)
                    object_fields[object_count++] = &character_controller.action_effects[slot];
                for (uint32 slot = 0; slot < CharacterController::ModelCount; ++slot)
                {
                    if (character_controller.model_slots[slot] != 0)
                        object_fields[object_count++] = &character_controller.model_slots[slot]->object;
                }
                for (uint32 group_index = 0; group_index < ModelGroupRegistry::Capacity; ++group_index)
                {
                    for (uint32 slot = 0; slot < sizeof(model_groups.records[group_index].models) / sizeof(model_groups.records[group_index].models[0]); ++slot)
                        object_fields[object_count++] = &model_groups.records[group_index].models[slot];
                }
                for (uint32 slot = 0; slot < sizeof(actor_models.definitions) / sizeof(actor_models.definitions[0]); ++slot)
                    object_fields[object_count++] = &actor_models.definitions[slot].prototype;
                for (uint32 slot = 0; slot < object_count; ++slot)
                {
                    for (owner_index = actor_first; owner_index < actor_limit; ++owner_index)
                    {
                        if (*object_fields[slot] == (const AnimatedObject *)source_nodes[owner_index])
                        {
                            *object_fields[slot] = (AnimatedObject *)native_nodes[owner_index];
                            break;
                        }
                    }
                }
                for (uint32 slot = 0; slot < animated_object_count + object_count; ++slot)
                {
                    AnimatedObject *actor = slot < animated_object_count ? animated_objects + slot : *object_fields[slot - animated_object_count];
                    if (actor == 0 || actor == (AnimatedObject *)0)
                        continue;
                    for (owner_index = sprite_first; owner_index < sprite_limit; ++owner_index)
                    {
                        if (actor->sprite == (const SpriteRecord *)source_nodes[owner_index])
                        {
                            actor->sprite = (SpriteRecord *)native_nodes[owner_index];
                            break;
                        }
                    }
                    if ((sint8)actor->metadata126[0] == 3)
                    {
                        SceneTrigger **contact_fields[] = {&actor->model_contacts.surface0, &actor->model_contacts.surface1, &actor->model_contacts.surface2};
                        for (uint32 field_index = 0; field_index < 3; ++field_index)
                        {
                            for (owner_index = trigger_first; owner_index < trigger_limit; ++owner_index)
                            {
                                if (*contact_fields[field_index] == (const SceneTrigger *)source_nodes[owner_index])
                                {
                                    *contact_fields[field_index] = (SceneTrigger *)native_nodes[owner_index];
                                    break;
                                }
                            }
                        }
                    }
                    SceneNode **node_fields[] = {&actor->owner_node, &actor->model_contacts.blocker};
                    uint32 node_field_count = (sint8)actor->metadata126[0] == 3 ? 2 : 1;
                    for (uint32 field_index = 0; field_index < node_field_count; ++field_index)
                    {
                        for (owner_index = 0; owner_index < owner_count; ++owner_index)
                        {
                            if (*node_fields[field_index] == source_nodes[owner_index])
                            {
                                *node_fields[field_index] = native_nodes[owner_index];
                                break;
                            }
                        }
                    }
                    AnimatedObject **actor_fields[2];
                    uint32 actor_field_count = 0;
                    if (actor->behavior == 31)
                        actor_fields[actor_field_count++] = &actor->linked_object;
                    if (actor->behavior == 33)
                        actor_fields[actor_field_count++] = &actor->follow_target;
                    for (uint32 field_index = 0; field_index < actor_field_count; ++field_index)
                    {
                        for (owner_index = actor_first; owner_index < actor_limit; ++owner_index)
                        {
                            if (*actor_fields[field_index] == (const AnimatedObject *)source_nodes[owner_index])
                            {
                                *actor_fields[field_index] = (AnimatedObject *)native_nodes[owner_index];
                                break;
                            }
                        }
                    }
                }
            }
            for (light_index = 0; light_index < light_count; ++light_index)
            {
                const SceneLight *original_light = (const SceneLight *)source_nodes[light_index];
                if (scene_lighting.lights == original_light)
                    scene_lighting.lights = lights + light_index;
                for (uint32 point_index = 0; point_index < 4; ++point_index)
                {
                    if (scene_lighting.point_lights[point_index] == original_light)
                        scene_lighting.point_lights[point_index] = lights + light_index;
                }
            }
            for (mesh_index = 0; mesh_index < mesh_count; ++mesh_index)
            {
                MeshGeometryNode *mesh = meshes + mesh_index;
                if (mesh->vertex_count <= 0)
                {
                    for (owner_index = light_count; owner_index < light_count + mesh_count; ++owner_index)
                    {
                        if (mesh->source_mesh == (const MeshGeometryNode *)source_nodes[owner_index])
                        {
                            mesh->source_mesh = (MeshGeometryNode *)native_nodes[owner_index];
                            break;
                        }
                    }
                }
                const MeshGeometryNode *original_mesh = (const MeshGeometryNode *)source_nodes[light_count + mesh_index];
                if (scene_lighting.bounds_origin == original_mesh->position)
                    scene_lighting.bounds_origin = mesh->position;
            }
            for (path_index = 0; path_index < path_count; ++path_index)
            {
                ScenePath *path = paths + path_index;
                const ScenePath **fields[] = {&path->adjacent[0], &path->adjacent[1], &path->fallback};
                uint32 definition_flags = path->definition->flags;
                uint32 field_count = (definition_flags & 0x100010) != 0 ? 3 : 2;
                for (uint32 field_index = 0; field_index < field_count; ++field_index)
                {
                    for (owner_index = light_count + mesh_count; owner_index < light_count + mesh_count + path_count; ++owner_index)
                    {
                        if (*fields[field_index] == (const ScenePath *)source_nodes[owner_index])
                        {
                            *fields[field_index] = (ScenePath *)native_nodes[owner_index];
                            break;
                        }
                    }
                }
            }
            for (uint32 trigger_group = 4; trigger_group <= 6; trigger_group += 2)
            {
                SceneTrigger *trigger = (SceneTrigger *)object_heads[trigger_group];
                while (trigger != 0)
                {
                    const ScenePath **fields[2];
                    uint32 field_count = 0;
                    sint32 trigger_kind = (sint8)(trigger->node.state_flags >> 8);
                    if (trigger_group == 6)
                    {
                        fields[0] = &trigger->parameters.path_pair.targets[0];
                        fields[1] = &trigger->parameters.path_pair.targets[1];
                        field_count = 2;
                    }
                    else if (trigger_kind == 7 || ((uint32)(trigger_kind - 26) & 0xFF) < 2)
                    {
                        fields[0] = &trigger->parameters.path.target;
                        field_count = 1;
                    }
                    for (uint32 field_index = 0; field_index < field_count; ++field_index)
                    {
                        for (owner_index = light_count + mesh_count; owner_index < light_count + mesh_count + path_count; ++owner_index)
                        {
                            if (*fields[field_index] == (const ScenePath *)source_nodes[owner_index])
                            {
                                *fields[field_index] = (ScenePath *)native_nodes[owner_index];
                                break;
                            }
                        }
                    }
                    trigger = (SceneTrigger *)trigger->node.links.registry_next;
                }
            }
            const ScenePath **path_fields[] = {&camera_motion.path, &character_controller.path, &character_controller.path_transition.anchor_path, &character_controller.path_transition.path, &character_controller.path_transition.constrained_path, &cutscene.path};
            for (uint32 field_index = 0; field_index < sizeof(path_fields) / sizeof(path_fields[0]); ++field_index)
            {
                for (owner_index = light_count + mesh_count; owner_index < light_count + mesh_count + path_count; ++owner_index)
                {
                    if (*path_fields[field_index] == (const ScenePath *)source_nodes[owner_index])
                    {
                        *path_fields[field_index] = (ScenePath *)native_nodes[owner_index];
                        break;
                    }
                }
            }
        }

        // Merge source spans before copying so mutable aliases stay shared
        struct GeometryRange
        {
            uint32 first, last, destination;
        };

        uint32 range_capacity = mesh_count + animated_object_count;
        if (range_capacity < mesh_count || range_capacity > 0xFFFFFFFFu / sizeof(GeometryRange))
        {
            fprintf(stderr, "P2: Scene face range capacity overflows\n");
            abort();
        }
        GeometryRange *face_ranges = range_capacity != 0 ? (GeometryRange *)malloc(range_capacity * sizeof(GeometryRange)) : 0;
        if (range_capacity != 0 && face_ranges == 0)
        {
            fprintf(stderr, "P2: Cannot allocate scene face ranges\n");
            abort();
        }
        uint32 range_count = 0;
        for (uint32 geometry_index = 0; geometry_index < range_capacity; ++geometry_index)
        {
            MeshFace *faces;
            sint32 count;
            if (geometry_index < mesh_count)
            {
                faces = meshes[geometry_index].polygons;
                count = meshes[geometry_index].polygon_count;
            }
            else
            {
                AnimatedObject *actor = animated_objects + geometry_index - mesh_count;
                faces = actor->polygons;
                count = (sint16)actor->metadata32;
            }
            if (count < 0 || (count != 0 && faces == 0))
            {
                fprintf(stderr, "P2: Invalid scene polygon span\n");
                abort();
            }
            if (count == 0)
                continue;
            uint32 first = ((uintptr_t)(faces) >= source_begin && ((uintptr_t)(faces)-source_begin) <= data_bytes && (sizeof(EncodedMeshFace)) <= data_bytes - ((uintptr_t)(faces)-source_begin) ? (uint32)((uintptr_t)(faces)-source_begin) : (fprintf(stderr, "P2: Scene source pointer exceeds native input\n"), abort(), 0u));
            if ((first & 3) != 0 || first > data_bytes || (uint32)count > (data_bytes - first) / sizeof(EncodedMeshFace))
            {
                fprintf(stderr, "P2: Scene polygon span exceeds resource\n");
                abort();
            }
            uint32 position = range_count;
            while (position != 0 && first < face_ranges[position - 1].first)
            {
                face_ranges[position] = face_ranges[position - 1];
                --position;
            }
            face_ranges[position].first = first;
            face_ranges[position].last = first + count * sizeof(EncodedMeshFace);
            face_ranges[position].destination = 0;
            ++range_count;
        }
        uint32 merged_count = 0;
        for (uint32 range_index = 0; range_index < range_count; ++range_index)
        {
            GeometryRange range = face_ranges[range_index];
            if (merged_count != 0 && range.first <= face_ranges[merged_count - 1].last)
            {
                GeometryRange *previous = face_ranges + merged_count - 1;
                if ((range.first - previous->first) % sizeof(EncodedMeshFace) != 0)
                {
                    fprintf(stderr, "P2: Overlapping scene polygon spans are misaligned\n");
                    abort();
                }
                if (range.last > previous->last)
                    previous->last = range.last;
            }
            else
                face_ranges[merged_count++] = range;
        }
        face_count = 0;
        for (uint32 range_index = 0; range_index < merged_count; ++range_index)
        {
            GeometryRange *range = face_ranges + range_index;
            range->destination = face_count;
            face_count += (range->last - range->first) / sizeof(EncodedMeshFace);
        }
        if (face_count > 0xFFFFFFFFu / sizeof(MeshFace))
        {
            fprintf(stderr, "P2: Native scene polygon allocation overflows\n");
            abort();
        }
        face_storage = face_count != 0 ? (MeshFace *)malloc(face_count * sizeof(MeshFace)) : 0;
        if (face_count != 0 && face_storage == 0)
        {
            fprintf(stderr, "P2: Cannot allocate native scene polygons\n");
            abort();
        }
        for (uint32 range_index = 0; range_index < merged_count; ++range_index)
        {
            const GeometryRange *range = face_ranges + range_index;
            uint32 count = (range->last - range->first) / sizeof(EncodedMeshFace);
            for (uint32 face_index = 0; face_index < count; ++face_index)
            {
                uint32 offset = range->first + face_index * sizeof(EncodedMeshFace);
                const uint8 *encoded = (const uint8 *)((offset) <= data_bytes && (sizeof(EncodedMeshFace)) <= data_bytes - (offset) ? data + (offset) : (fprintf(stderr, "P2: Scene file span exceeds native input\n"), abort(), data));
                MeshFace *face = face_storage + range->destination + face_index;
                face->texture = source_face_textures[offset / sizeof(uint32)];
                for (uint32 vertex = 0; vertex < 4; ++vertex)
                    face->vertex_indices[vertex] = xport_load_le16(encoded + 4 + vertex * 2);
                face->plane = xport_load_le32(encoded + 12);
                face->normal.words[0] = xport_load_le32(encoded + 16);
                face->normal.words[1] = xport_load_le32(encoded + 20);
            }
        }
        for (uint32 geometry_index = 0; geometry_index < range_capacity; ++geometry_index)
        {
            MeshFace **faces = geometry_index < mesh_count ? &meshes[geometry_index].polygons : &animated_objects[geometry_index - mesh_count].polygons;
            if (*faces == 0)
                continue;
            uint32 first = ((uintptr_t)(*faces) >= source_begin && ((uintptr_t)(*faces) - source_begin) <= data_bytes && (sizeof(EncodedMeshFace)) <= data_bytes - ((uintptr_t)(*faces) - source_begin) ? (uint32)((uintptr_t)(*faces) - source_begin) : (fprintf(stderr, "P2: Scene source pointer exceeds native input\n"), abort(), 0u));
            for (uint32 range_index = 0; range_index < merged_count; ++range_index)
            {
                const GeometryRange *range = face_ranges + range_index;
                if (first >= range->first && first < range->last)
                {
                    *faces = face_storage + range->destination + (first - range->first) / sizeof(EncodedMeshFace);
                    break;
                }
            }
        }
        free(face_ranges);
        free(source_face_textures);
        GeometryRange *vertex_ranges = range_capacity != 0 ? (GeometryRange *)malloc(range_capacity * sizeof(GeometryRange)) : 0;
        uint32 *vertex_extents = range_capacity != 0 ? (uint32 *)calloc(range_capacity, sizeof(uint32)) : 0;
        SceneNode **pending_nodes = animated_object_count != 0 ? (SceneNode **)malloc(owner_count * sizeof(SceneNode *)) : 0;
        if ((range_capacity != 0 && (vertex_ranges == 0 || vertex_extents == 0)) || (animated_object_count != 0 && pending_nodes == 0))
        {
            fprintf(stderr, "P2: Cannot allocate scene vertex ranges\n");
            abort();
        }
        uint32 vertex_range_count = 0;
        for (uint32 geometry_index = 0; geometry_index < range_capacity; ++geometry_index)
        {
            MeshVertex *vertices;
            uint32 extent = 0;
            if (geometry_index < mesh_count)
            {
                MeshGeometryNode *mesh = meshes + geometry_index;
                if (mesh->vertex_count <= 0)
                    continue;
                if (mesh->first_vertex < 0)
                {
                    fprintf(stderr, "P2: Negative scene vertex range\n");
                    abort();
                }
                vertices = mesh->vertices;
                extent = mesh->first_vertex + mesh->vertex_count;
            }
            else
            {
                AnimatedObject *actor = animated_objects + geometry_index - mesh_count;
                vertices = actor->vertices;
                if (actor->mesh_span.first_vertex < 0 || actor->mesh_span.vertex_count < 0)
                {
                    fprintf(stderr, "P2: Negative scene morph range\n");
                    abort();
                }
                if (actor->mesh_span.vertex_count != 0)
                    extent = actor->mesh_span.first_vertex + actor->mesh_span.vertex_count;
                sint32 polygon_count = (sint16)actor->metadata32;
                for (sint32 polygon_index = 0; polygon_index < polygon_count; ++polygon_index)
                {
                    for (uint32 corner = 0; corner < 4; ++corner)
                    {
                        uint32 end = actor->polygons[polygon_index].vertex_indices[corner] + 1;
                        if (end > extent)
                            extent = end;
                    }
                }
                uint32 pending_count = 0;
                uint32 traversal_budget = data_bytes / sizeof(uint32);
                if (actor->node.links.first_child != 0)
                    pending_nodes[pending_count++] = actor->node.links.first_child;
                while (pending_count != 0)
                {
                    SceneNode *part_node = pending_nodes[--pending_count];
                    if (traversal_budget == 0)
                    {
                        fprintf(stderr, "P2: Scene morph graph is cyclic or exceeds resource\n");
                        abort();
                    }
                    --traversal_budget;
                    if (part_node->kind == 5)
                    {
                        const MorphSpan *span = &((const MeshPartNode *)part_node)->span;
                        if (span->first_vertex < 0 || span->vertex_count < 0)
                        {
                            fprintf(stderr, "P2: Negative scene part vertex range\n");
                            abort();
                        }
                        if (span->vertex_count != 0)
                        {
                            uint32 end = span->first_vertex + span->vertex_count;
                            if (end > extent)
                                extent = end;
                        }
                    }
                    SceneNode *next_nodes[] = {part_node->links.sibling_next, part_node->kind != 0 ? part_node->links.first_child : 0};
                    for (uint32 next_index = 0; next_index < 2; ++next_index)
                    {
                        if (next_nodes[next_index] != 0)
                        {
                            if (pending_count == owner_count)
                            {
                                fprintf(stderr, "P2: Scene morph traversal exceeds node capacity\n");
                                abort();
                            }
                            pending_nodes[pending_count++] = next_nodes[next_index];
                        }
                    }
                }
            }
            vertex_extents[geometry_index] = extent;
            if (extent == 0)
                continue;
            if (vertices == 0)
            {
                fprintf(stderr, "P2: Scene vertex range has no buffer\n");
                abort();
            }
            uint32 first = ((uintptr_t)(vertices) >= source_begin && ((uintptr_t)(vertices)-source_begin) <= data_bytes && (sizeof(MeshVertex)) <= data_bytes - ((uintptr_t)(vertices)-source_begin) ? (uint32)((uintptr_t)(vertices)-source_begin) : (fprintf(stderr, "P2: Scene source pointer exceeds native input\n"), abort(), 0u));
            if ((first & 3) != 0 || first > data_bytes || extent > (data_bytes - first) / sizeof(MeshVertex))
            {
                fprintf(stderr, "P2: Scene vertex range exceeds resource\n");
                abort();
            }
            uint32 position = vertex_range_count;
            while (position != 0 && first < vertex_ranges[position - 1].first)
            {
                vertex_ranges[position] = vertex_ranges[position - 1];
                --position;
            }
            vertex_ranges[position].first = first;
            vertex_ranges[position].last = first + extent * sizeof(MeshVertex);
            vertex_ranges[position].destination = 0;
            ++vertex_range_count;
        }
        uint32 merged_vertices = 0;
        for (uint32 range_index = 0; range_index < vertex_range_count; ++range_index)
        {
            GeometryRange range = vertex_ranges[range_index];
            if (merged_vertices != 0 && range.first <= vertex_ranges[merged_vertices - 1].last)
            {
                GeometryRange *previous = vertex_ranges + merged_vertices - 1;
                if ((range.first - previous->first) % sizeof(MeshVertex) != 0)
                {
                    fprintf(stderr, "P2: Overlapping scene vertex ranges are misaligned\n");
                    abort();
                }
                if (range.last > previous->last)
                    previous->last = range.last;
            }
            else
                vertex_ranges[merged_vertices++] = range;
        }
        vertex_count = 0;
        for (uint32 range_index = 0; range_index < merged_vertices; ++range_index)
        {
            GeometryRange *range = vertex_ranges + range_index;
            range->destination = vertex_count;
            vertex_count += (range->last - range->first) / sizeof(MeshVertex);
        }
        vertex_storage = vertex_count != 0 ? (MeshVertex *)malloc(vertex_count * sizeof(MeshVertex)) : 0;
        if (vertex_count != 0 && vertex_storage == 0)
        {
            fprintf(stderr, "P2: Cannot allocate native scene vertices\n");
            abort();
        }
        for (uint32 range_index = 0; range_index < merged_vertices; ++range_index)
        {
            const GeometryRange *range = vertex_ranges + range_index;
            memcpy(vertex_storage + range->destination, ((range->first) <= data_bytes && (range->last - range->first) <= data_bytes - (range->first) ? data + (range->first) : (fprintf(stderr, "P2: Scene file span exceeds native input\n"), abort(), data)), range->last - range->first);
        }
        for (uint32 geometry_index = 0; geometry_index < range_capacity; ++geometry_index)
        {
            if (geometry_index < mesh_count && meshes[geometry_index].vertex_count <= 0)
                continue;
            MeshVertex **vertices = geometry_index < mesh_count ? &meshes[geometry_index].vertices : &animated_objects[geometry_index - mesh_count].vertices;
            if (*vertices == 0 || vertex_extents[geometry_index] == 0)
                continue;
            uint32 first = ((uintptr_t)(*vertices) >= source_begin && ((uintptr_t)(*vertices) - source_begin) <= data_bytes && (sizeof(MeshVertex)) <= data_bytes - ((uintptr_t)(*vertices) - source_begin) ? (uint32)((uintptr_t)(*vertices) - source_begin) : (fprintf(stderr, "P2: Scene source pointer exceeds native input\n"), abort(), 0u));
            for (uint32 range_index = 0; range_index < merged_vertices; ++range_index)
            {
                const GeometryRange *range = vertex_ranges + range_index;
                if (first >= range->first && first < range->last)
                {
                    *vertices = vertex_storage + range->destination + (first - range->first) / sizeof(MeshVertex);
                    break;
                }
            }
        }
        free(pending_nodes);
        free(vertex_extents);
        free(vertex_ranges);
        GeometryRange *cache_ranges = animated_object_count != 0 ? (GeometryRange *)malloc(animated_object_count * sizeof(GeometryRange)) : 0;
        if (animated_object_count != 0 && cache_ranges == 0)
        {
            fprintf(stderr, "P2: Cannot allocate scene collision ranges\n");
            abort();
        }
        uint32 cache_range_count = 0;
        for (actor_index = 0; actor_index < animated_object_count; ++actor_index)
        {
            const AnimatedObject *actor = animated_objects + actor_index;
            if ((actor->flags & 0x100000) == 0 || actor->collision_transforms == 0)
                continue;
            sint32 count = actor->collision_transform_count;
            uint32 first = ((uintptr_t)(actor->collision_transforms) >= source_begin && ((uintptr_t)(actor->collision_transforms) - source_begin) <= data_bytes && (sizeof(CollisionTransform)) <= data_bytes - ((uintptr_t)(actor->collision_transforms) - source_begin) ? (uint32)((uintptr_t)(actor->collision_transforms) - source_begin) : (fprintf(stderr, "P2: Scene source pointer exceeds native input\n"), abort(), 0u));
            if (count <= 0 || (first & 3) != 0 || first > data_bytes || (uint32)count > (data_bytes - first) / sizeof(CollisionTransform))
            {
                fprintf(stderr, "P2: Scene collision cache exceeds resource\n");
                abort();
            }
            uint32 position = cache_range_count;
            while (position != 0 && first < cache_ranges[position - 1].first)
            {
                cache_ranges[position] = cache_ranges[position - 1];
                --position;
            }
            cache_ranges[position].first = first;
            cache_ranges[position].last = first + count * sizeof(CollisionTransform);
            cache_ranges[position].destination = 0;
            ++cache_range_count;
        }
        uint32 merged_caches = 0;
        for (uint32 range_index = 0; range_index < cache_range_count; ++range_index)
        {
            GeometryRange range = cache_ranges[range_index];
            if (merged_caches != 0 && range.first <= cache_ranges[merged_caches - 1].last)
            {
                GeometryRange *previous = cache_ranges + merged_caches - 1;
                if ((range.first - previous->first) % sizeof(CollisionTransform) != 0)
                {
                    fprintf(stderr, "P2: Overlapping scene collision caches are misaligned\n");
                    abort();
                }
                if (range.last > previous->last)
                    previous->last = range.last;
            }
            else
                cache_ranges[merged_caches++] = range;
        }
        collision_count = 0;
        for (uint32 range_index = 0; range_index < merged_caches; ++range_index)
        {
            GeometryRange *range = cache_ranges + range_index;
            range->destination = collision_count;
            collision_count += (range->last - range->first) / sizeof(CollisionTransform);
        }
        collision_storage = collision_count != 0 ? (CollisionTransform *)malloc(collision_count * sizeof(CollisionTransform)) : 0;
        if (collision_count != 0 && collision_storage == 0)
        {
            fprintf(stderr, "P2: Cannot allocate native scene collision cache\n");
            abort();
        }
        for (uint32 range_index = 0; range_index < merged_caches; ++range_index)
        {
            const GeometryRange *range = cache_ranges + range_index;
            memcpy(collision_storage + range->destination, ((range->first) <= data_bytes && (range->last - range->first) <= data_bytes - (range->first) ? data + (range->first) : (fprintf(stderr, "P2: Scene file span exceeds native input\n"), abort(), data)), range->last - range->first);
        }
        for (actor_index = 0; actor_index < animated_object_count; ++actor_index)
        {
            AnimatedObject *actor = animated_objects + actor_index;
            if ((actor->flags & 0x100000) == 0 || actor->collision_transforms == 0)
                continue;
            uint32 first = ((uintptr_t)(actor->collision_transforms) >= source_begin && ((uintptr_t)(actor->collision_transforms) - source_begin) <= data_bytes && (sizeof(CollisionTransform)) <= data_bytes - ((uintptr_t)(actor->collision_transforms) - source_begin) ? (uint32)((uintptr_t)(actor->collision_transforms) - source_begin) : (fprintf(stderr, "P2: Scene source pointer exceeds native input\n"), abort(), 0u));
            for (uint32 range_index = 0; range_index < merged_caches; ++range_index)
            {
                const GeometryRange *range = cache_ranges + range_index;
                if (first >= range->first && first < range->last)
                {
                    actor->collision_transforms = collision_storage + range->destination + (first - range->first) / sizeof(CollisionTransform);
                    break;
                }
            }
        }
        free(cache_ranges);
        if (animated_object_count > 0xFFFFFFFFu / sizeof(AnimationHeader *))
        {
            fprintf(stderr, "P2: Scene animation range capacity overflows\n");
            abort();
        }
        AnimationHeader **animation_headers = animated_object_count != 0 ? (AnimationHeader **)malloc(animated_object_count * sizeof(AnimationHeader *)) : 0;
        if (animated_object_count != 0 && animation_headers == 0)
        {
            fprintf(stderr, "P2: Cannot allocate scene animation ranges\n");
            abort();
        }
        uint32 animation_header_count = 0;
        for (actor_index = 0; actor_index < animated_object_count; ++actor_index)
        {
            AnimationHeader *header = animated_objects[actor_index].animation;
            if (header == 0)
                continue;
            uintptr_t header_offset = (uintptr_t)header - (uintptr_t)animations;
            if (animations == 0 || (uintptr_t)header < (uintptr_t)animations || header_offset % sizeof(AnimationHeader) != 0 || header_offset / sizeof(AnimationHeader) >= animation_count)
            {
                fprintf(stderr, "P2: Scene animation header exceeds its native owner\n");
                abort();
            }
            uint32 header_index = 0;
            while (header_index < animation_header_count && animation_headers[header_index] != header)
                ++header_index;
            if (header_index != animation_header_count)
                continue;
            animation_headers[animation_header_count++] = header;
        }

        struct AnimationPayloadReference
        {
            AnimationTrack *track;
            uint32 key_offset, morph_offset;
            uint8 remap_keys, remap_morph;
        };

        uint32 payload_reference_capacity = 0;
        for (uint32 header_index = 0; header_index < animation_header_count; ++header_index)
        {
            for (uint32 track_group = 0; track_group < 4; ++track_group)
            {
                uint32 count = animation_headers[header_index]->counts[track_group];
                if (count > 0xFFFFFFFFu - payload_reference_capacity)
                {
                    fprintf(stderr, "P2: Animation payload reference count overflows\n");
                    abort();
                }
                payload_reference_capacity += count;
            }
        }
        if (payload_reference_capacity > 0xFFFFFFFFu / sizeof(AnimationPayloadReference) || payload_reference_capacity > 0xFFFFFFFFu / (2u * sizeof(GeometryRange)))
        {
            fprintf(stderr, "P2: Animation payload capacity overflows\n");
            abort();
        }
        AnimationPayloadReference *payload_references = payload_reference_capacity != 0 ? (AnimationPayloadReference *)malloc(payload_reference_capacity * sizeof(AnimationPayloadReference)) : 0;
        GeometryRange *payload_ranges = payload_reference_capacity != 0 ? (GeometryRange *)malloc(payload_reference_capacity * 2u * sizeof(GeometryRange)) : 0;
        if (payload_reference_capacity != 0 && (payload_references == 0 || payload_ranges == 0))
        {
            fprintf(stderr, "P2: Cannot allocate scene animation payload ranges\n");
            abort();
        }
        uint32 payload_reference_count = 0;
        uint32 payload_range_count = 0;
        for (uint32 header_index = 0; header_index < animation_header_count; ++header_index)
        {
            AnimationHeader *header = animation_headers[header_index];
            for (uint32 track_group = 0; track_group < 4; ++track_group)
            {
                uint32 stride = track_group == 0 ? sizeof(VectorAnimationTrack) : (track_group == 3 ? sizeof(MorphAnimationTrack) : sizeof(AnimationTrack));
                uint32 key_stride = track_group == 1 ? sizeof(QuaternionAnimationKey) : (track_group == 3 ? sizeof(MorphAnimationKey) : sizeof(VectorAnimationKey));
                for (sint32 track_index = 0; track_index < header->counts[track_group]; ++track_index)
                {
                    AnimationTrack *track = (AnimationTrack *)(header->tracks[track_group].bytes + track_index * stride);
                    AnimationPayloadReference *reference = payload_references + payload_reference_count++;
                    reference->track = track;
                    reference->key_offset = 0;
                    reference->morph_offset = 0;
                    reference->remap_keys = 0;
                    reference->remap_morph = 0;
                    uint32 starts[2], ends[2];
                    uint32 range_slots = 0;
                    if (track->keys != 0 && track->keys != (const uint8 *)-1)
                    {
                        uint32 first = ((uintptr_t)((void *)track->keys) >= source_begin && ((uintptr_t)((void *)track->keys) - source_begin) <= data_bytes && (sizeof(uint32)) <= data_bytes - ((uintptr_t)((void *)track->keys) - source_begin) ? (uint32)((uintptr_t)((void *)track->keys) - source_begin) : (fprintf(stderr, "P2: Scene source pointer exceeds native input\n"), abort(), 0u));
                        uint32 count = track->key_count;
                        if (count == 0 || (first & 3) != 0 || first > data_bytes || count > (data_bytes - first) / key_stride)
                        {
                            fprintf(stderr, "P2: Animation key range exceeds scene resource\n");
                            abort();
                        }
                        uint32 curve_count = 0;
                        uint32 maximum_frame = 0;
                        for (uint32 key_index = 0; key_index < count; ++key_index)
                        {
                            const uint8 *key = track->keys + key_index * key_stride;
                            uint32 curve = xport_load_le32(key) & 0xFF;
                            if (curve > curve_count)
                                curve_count = curve;
                            if (track_group == 3)
                            {
                                uint32 frame_index = xport_load_le32(key + 4);
                                if (frame_index > maximum_frame)
                                    maximum_frame = frame_index;
                            }
                        }
                        uint32 prefix = curve_count * sizeof(AnimationEaseCurve);
                        if (prefix > first)
                        {
                            fprintf(stderr, "P2: Animation easing prefix exceeds scene resource\n");
                            abort();
                        }
                        starts[range_slots] = first - prefix;
                        ends[range_slots++] = first + count * key_stride;
                        reference->key_offset = first;
                        reference->remap_keys = 1;
                        if (track_group == 3)
                        {
                            MorphAnimationTrack *morph = (MorphAnimationTrack *)track;
                            if (morph->vertices == 0 || morph->vertex_stride <= 0)
                            {
                                fprintf(stderr, "P2: Scene morph payload has no positive frame stride\n");
                                abort();
                            }
                            first = ((uintptr_t)((void *)morph->vertices) >= source_begin && ((uintptr_t)((void *)morph->vertices) - source_begin) <= data_bytes && (sizeof(SVECTOR)) <= data_bytes - ((uintptr_t)((void *)morph->vertices) - source_begin) ? (uint32)((uintptr_t)((void *)morph->vertices) - source_begin) : (fprintf(stderr, "P2: Scene source pointer exceeds native input\n"), abort(), 0u));
                            uint32 frame_bytes = morph->vertex_stride * sizeof(SVECTOR);
                            if ((first & 3) != 0 || first > data_bytes || maximum_frame >= (data_bytes - first) / frame_bytes)
                            {
                                fprintf(stderr, "P2: Scene morph frame range exceeds resource\n");
                                abort();
                            }
                            starts[range_slots] = first;
                            ends[range_slots++] = first + (maximum_frame + 1u) * frame_bytes;
                            reference->morph_offset = first;
                            reference->remap_morph = 1;
                        }
                    }
                    for (uint32 range_slot = 0; range_slot < range_slots; ++range_slot)
                    {
                        uint32 position = payload_range_count;
                        while (position != 0 && starts[range_slot] < payload_ranges[position - 1].first)
                        {
                            payload_ranges[position] = payload_ranges[position - 1];
                            --position;
                        }
                        payload_ranges[position].first = starts[range_slot];
                        payload_ranges[position].last = ends[range_slot];
                        payload_ranges[position].destination = 0;
                        ++payload_range_count;
                    }
                }
            }
        }
        uint32 merged_payloads = 0;
        for (uint32 range_index = 0; range_index < payload_range_count; ++range_index)
        {
            GeometryRange range = payload_ranges[range_index];
            if (merged_payloads != 0 && range.first <= payload_ranges[merged_payloads - 1].last)
            {
                if (range.last > payload_ranges[merged_payloads - 1].last)
                    payload_ranges[merged_payloads - 1].last = range.last;
            }
            else
                payload_ranges[merged_payloads++] = range;
        }
        animation_payload_bytes = 0;
        for (uint32 range_index = 0; range_index < merged_payloads; ++range_index)
        {
            GeometryRange *range = payload_ranges + range_index;
            range->destination = animation_payload_bytes;
            animation_payload_bytes += range->last - range->first;
        }
        animation_payload_data = animation_payload_bytes != 0 ? (uint8 *)malloc(animation_payload_bytes) : 0;
        if (animation_payload_bytes != 0 && animation_payload_data == 0)
        {
            fprintf(stderr, "P2: Cannot allocate native scene animation payload\n");
            abort();
        }
        for (uint32 range_index = 0; range_index < merged_payloads; ++range_index)
        {
            const GeometryRange *range = payload_ranges + range_index;
            memcpy(animation_payload_data + range->destination, ((range->first) <= data_bytes && (range->last - range->first) <= data_bytes - (range->first) ? data + (range->first) : (fprintf(stderr, "P2: Scene file span exceeds native input\n"), abort(), data)), range->last - range->first);
        }
        for (uint32 reference_index = 0; reference_index < payload_reference_count; ++reference_index)
        {
            const AnimationPayloadReference *reference = payload_references + reference_index;
            for (uint32 range_index = 0; range_index < merged_payloads; ++range_index)
            {
                const GeometryRange *range = payload_ranges + range_index;
                if (reference->remap_keys != 0 && reference->key_offset >= range->first && reference->key_offset < range->last)
                    reference->track->keys = animation_payload_data + range->destination + reference->key_offset - range->first;
                if (reference->remap_morph != 0 && reference->morph_offset >= range->first && reference->morph_offset < range->last)
                    ((MorphAnimationTrack *)reference->track)->vertices = (const SVECTOR *)(animation_payload_data + range->destination + reference->morph_offset - range->first);
            }
        }
        free(payload_ranges);
        free(payload_references);
        if (animation_header_count != animation_count)
        {
            fprintf(stderr, "P2: Scene animation owner lost a root reference\n");
            abort();
        }
        {
            AnimatedObject *animation_objects[4 + CharacterController::ModelCount + sizeof(action_effect_resources.templates) / sizeof(action_effect_resources.templates[0]) + sizeof(character_controller.action_effects) / sizeof(character_controller.action_effects[0]) + ModelGroupRegistry::Capacity * sizeof(model_groups.records[0].models) / sizeof(model_groups.records[0].models[0]) + sizeof(actor_models.definitions) / sizeof(actor_models.definitions[0])];
            uint32 animation_object_count = 3;
            animation_objects[0] = character_controller.object;
            animation_objects[1] = character_action_owner;
            animation_objects[2] = character_action_target;
            if (character_controller.model != 0)
                animation_objects[animation_object_count++] = character_controller.model->object;
            for (uint32 slot = 0; slot < CharacterController::ModelCount; ++slot)
            {
                if (character_controller.model_slots[slot] != 0)
                    animation_objects[animation_object_count++] = character_controller.model_slots[slot]->object;
            }
            for (uint32 slot = 0; slot < sizeof(action_effect_resources.templates) / sizeof(action_effect_resources.templates[0]); ++slot)
                animation_objects[animation_object_count++] = action_effect_resources.templates[slot];
            for (uint32 slot = 0; slot < sizeof(character_controller.action_effects) / sizeof(character_controller.action_effects[0]); ++slot)
                animation_objects[animation_object_count++] = character_controller.action_effects[slot];
            for (uint32 group_index = 0; group_index < ModelGroupRegistry::Capacity; ++group_index)
            {
                for (uint32 slot = 0; slot < sizeof(model_groups.records[group_index].models) / sizeof(model_groups.records[group_index].models[0]); ++slot)
                    animation_objects[animation_object_count++] = model_groups.records[group_index].models[slot];
            }
            for (uint32 slot = 0; slot < sizeof(actor_models.definitions) / sizeof(actor_models.definitions[0]); ++slot)
                animation_objects[animation_object_count++] = actor_models.definitions[slot].prototype;
            for (uint32 object_index = 0; object_index < animation_object_count; ++object_index)
            {
                AnimatedObject *actor = animation_objects[object_index];
                if (actor == 0 || actor == (AnimatedObject *)0)
                    continue;
                for (uint32 header_index = 0; header_index < animation_count; ++header_index)
                {
                    if ((const void *)actor->animation == ((source_animation_offsets[header_index]) <= data_bytes && (sizeof(EncodedAnimationHeader)) <= data_bytes - (source_animation_offsets[header_index]) ? data + (source_animation_offsets[header_index]) : (fprintf(stderr, "P2: Scene file span exceeds native input\n"), abort(), data)))
                    {
                        actor->animation = animations + header_index;
                        break;
                    }
                }
            }
            for (SceneNode *registered = registered_object_head; registered != 0; registered = registered->links.registry_next)
            {
                if (registered->kind != 0)
                    continue;
                AnimatedObject *actor = (AnimatedObject *)registered;
                for (uint32 header_index = 0; header_index < animation_count; ++header_index)
                {
                    if ((const void *)actor->animation == ((source_animation_offsets[header_index]) <= data_bytes && (sizeof(EncodedAnimationHeader)) <= data_bytes - (source_animation_offsets[header_index]) ? data + (source_animation_offsets[header_index]) : (fprintf(stderr, "P2: Scene file span exceeds native input\n"), abort(), data)))
                    {
                        actor->animation = animations + header_index;
                        break;
                    }
                }
            }
            for (uint32 model_index = 0; model_index <= CharacterController::ModelCount; ++model_index)
            {
                CharacterModelRecord *model = model_index < CharacterController::ModelCount ? character_controller.model_slots[model_index] : character_controller.model;
                if (model == 0)
                    continue;
                for (uint32 slot = 0; slot < CharacterModelRecord::AnimationCount; ++slot)
                {
                    for (uint32 header_index = 0; header_index < animation_count; ++header_index)
                    {
                        if ((const void *)model->animations[slot] == ((source_animation_offsets[header_index]) <= data_bytes && (sizeof(EncodedAnimationHeader)) <= data_bytes - (source_animation_offsets[header_index]) ? data + (source_animation_offsets[header_index]) : (fprintf(stderr, "P2: Scene file span exceeds native input\n"), abort(), data)))
                        {
                            model->animations[slot] = animations + header_index;
                            break;
                        }
                    }
                }
            }
            for (uint32 model_index = 0; model_index < sizeof(actor_models.definitions) / sizeof(actor_models.definitions[0]); ++model_index)
            {
                ActorModelDefinition *model = actor_models.definitions + model_index;
                for (uint32 slot = 0; slot < sizeof(model->animations) / sizeof(model->animations[0]); ++slot)
                {
                    for (uint32 header_index = 0; header_index < animation_count; ++header_index)
                    {
                        if ((const void *)model->animations[slot] == ((source_animation_offsets[header_index]) <= data_bytes && (sizeof(EncodedAnimationHeader)) <= data_bytes - (source_animation_offsets[header_index]) ? data + (source_animation_offsets[header_index]) : (fprintf(stderr, "P2: Scene file span exceeds native input\n"), abort(), data)))
                        {
                            model->animations[slot] = animations + header_index;
                            break;
                        }
                    }
                }
            }
            for (uint32 model_index = 0; model_index < sizeof(animated_models.records) / sizeof(animated_models.records[0]); ++model_index)
            {
                AnimatedModelDefinition *model = animated_models.records + model_index;
                for (uint32 slot = 0; slot < sizeof(model->animations) / sizeof(model->animations[0]); ++slot)
                {
                    for (uint32 header_index = 0; header_index < animation_count; ++header_index)
                    {
                        if ((const void *)model->animations[slot] == ((source_animation_offsets[header_index]) <= data_bytes && (sizeof(EncodedAnimationHeader)) <= data_bytes - (source_animation_offsets[header_index]) ? data + (source_animation_offsets[header_index]) : (fprintf(stderr, "P2: Scene file span exceeds native input\n"), abort(), data)))
                        {
                            model->animations[slot] = animations + header_index;
                            break;
                        }
                    }
                }
            }
        }
        free(animation_headers);
        free(source_animation_offsets);

        struct MaterialAnimationReference
        {
            uint32 **entries;
            uint32 offset;
        };

        uint32 material_reference_capacity = animated_object_count;
        if (mesh_count > 0xFFFFFFFFu - material_reference_capacity || sprite_count > 0xFFFFFFFFu - material_reference_capacity - mesh_count)
        {
            fprintf(stderr, "P2: Material animation reference count overflows\n");
            abort();
        }
        material_reference_capacity += mesh_count + sprite_count;
        if (material_reference_capacity > 0xFFFFFFFFu / sizeof(MaterialAnimationReference) || material_reference_capacity > 0xFFFFFFFFu / sizeof(GeometryRange))
        {
            fprintf(stderr, "P2: Material animation range capacity overflows\n");
            abort();
        }
        MaterialAnimationReference *material_references = material_reference_capacity != 0 ? (MaterialAnimationReference *)malloc(material_reference_capacity * sizeof(MaterialAnimationReference)) : 0;
        GeometryRange *material_ranges = material_reference_capacity != 0 ? (GeometryRange *)malloc(material_reference_capacity * sizeof(GeometryRange)) : 0;
        if (material_reference_capacity != 0 && (material_references == 0 || material_ranges == 0))
        {
            fprintf(stderr, "P2: Cannot allocate material animation ranges\n");
            abort();
        }
        uint32 material_reference_count = 0;
        for (uint32 owner_group = 0; owner_group < 3; ++owner_group)
        {
            uint32 owner_count = owner_group == 0 ? animated_object_count : (owner_group == 1 ? mesh_count : sprite_count);
            for (uint32 owner_index = 0; owner_index < owner_count; ++owner_index)
            {
                uint32 **entries = owner_group == 0 ? &animated_objects[owner_index].material_animation_entries : (owner_group == 1 ? &meshes[owner_index].material_animation_entries : &sprites[owner_index].animation_entries);
                if (*entries == 0)
                    continue;
                uint32 first = ((uintptr_t)(*entries) >= source_begin && ((uintptr_t)(*entries) - source_begin) <= data_bytes && (sizeof(uint32)) <= data_bytes - ((uintptr_t)(*entries) - source_begin) ? (uint32)((uintptr_t)(*entries) - source_begin) : (fprintf(stderr, "P2: Scene source pointer exceeds native input\n"), abort(), 0u));
                if ((first & 3) != 0 || first < sizeof(uint32) || first > data_bytes)
                {
                    fprintf(stderr, "P2: Material animation header exceeds scene payload\n");
                    abort();
                }
                uint32 count = xport_load_le32(((first - sizeof(uint32)) <= data_bytes && (sizeof(uint32)) <= data_bytes - (first - sizeof(uint32)) ? data + (first - sizeof(uint32)) : (fprintf(stderr, "P2: Scene file span exceeds native input\n"), abort(), data))) & 0xFF;
                if (count > (data_bytes - first) / sizeof(uint32))
                {
                    fprintf(stderr, "P2: Material animation entries exceed scene payload\n");
                    abort();
                }
                MaterialAnimationReference *reference = material_references + material_reference_count;
                reference->entries = entries;
                reference->offset = first;
                uint32 position = material_reference_count;
                first -= sizeof(uint32);
                while (position != 0 && first < material_ranges[position - 1].first)
                {
                    material_ranges[position] = material_ranges[position - 1];
                    --position;
                }
                material_ranges[position].first = first;
                material_ranges[position].last = first + (count + 1u) * sizeof(uint32);
                material_ranges[position].destination = 0;
                ++material_reference_count;
            }
        }
        uint32 merged_material_ranges = 0;
        for (uint32 range_index = 0; range_index < material_reference_count; ++range_index)
        {
            GeometryRange range = material_ranges[range_index];
            if (merged_material_ranges != 0 && range.first <= material_ranges[merged_material_ranges - 1].last)
            {
                if (range.last > material_ranges[merged_material_ranges - 1].last)
                    material_ranges[merged_material_ranges - 1].last = range.last;
            }
            else
                material_ranges[merged_material_ranges++] = range;
        }
        uint32 material_bytes = 0;
        for (uint32 range_index = 0; range_index < merged_material_ranges; ++range_index)
        {
            GeometryRange *range = material_ranges + range_index;
            range->destination = material_bytes;
            material_bytes += range->last - range->first;
        }
        material_animation_word_count = material_bytes / sizeof(uint32);
        material_animation_words = material_bytes != 0 ? (uint32 *)malloc(material_bytes) : 0;
        if (material_bytes != 0 && material_animation_words == 0)
        {
            fprintf(stderr, "P2: Cannot allocate native material animation lists\n");
            abort();
        }
        for (uint32 range_index = 0; range_index < merged_material_ranges; ++range_index)
        {
            const GeometryRange *range = material_ranges + range_index;
            memcpy((uint8 *)material_animation_words + range->destination, ((range->first) <= data_bytes && (range->last - range->first) <= data_bytes - (range->first) ? data + (range->first) : (fprintf(stderr, "P2: Scene file span exceeds native input\n"), abort(), data)), range->last - range->first);
        }
        for (uint32 reference_index = 0; reference_index < material_reference_count; ++reference_index)
        {
            const MaterialAnimationReference *reference = material_references + reference_index;
            for (uint32 range_index = 0; range_index < merged_material_ranges; ++range_index)
            {
                const GeometryRange *range = material_ranges + range_index;
                if (reference->offset >= range->first && reference->offset < range->last)
                {
                    *reference->entries = (uint32 *)((uint8 *)material_animation_words + range->destination + reference->offset - range->first);
                    break;
                }
            }
        }
        free(material_ranges);
        free(material_references);
        free(native_nodes);
        free(source_nodes);
        free(source_path_offsets);
        free(source_placement_offsets);
        free(source_actor_offsets);
    }
}

uint32 SceneSection::Activate()
{
    FUNCTION_MARKER(0x80025828, "SLUS_005.78");
    sint32 index;
    uint32 value, flags, result;
    const SectionDefinition *section;
    section_index = scene_loader.loading_section;
    palette_bank = (uint8)scene_loader.texture_slot_base;
    index = (sint8)scene_loader.loading_section;
    section = level_resource.sections + index;
    value = section->origin[0];
    flags = state_flags;
    origin[0] = value;
    origin[1] = section->origin[1];
    value = section->origin[2];
    flags &= 0xFFE7;
    state_flags = flags;
    origin[2] = value;
    result = (section->flags & 0x10) | 0x40;
    state_flags = flags | result;
    return result;
}

sint32 SceneSection::FindObjectId(const SceneObjectName *name) const
{
    FUNCTION_MARKER(0x80022A38, "SLUS_005.78");
    uint32 kind, flags, name_index = 0;
    const SceneObjectName *object_name;
    const SceneNode *path_node;
    path_node = object_heads[3];
    while (path_node != 0)
    {
        object_name = path_names + name_index;
        if (strncmp(object_name->prefix, name->prefix, 2) == 0)
        {
            kind = object_name->kind;
            if (kind != 60 && kind != 62)
            {
                flags = path_node->state_flags;
                if ((flags & 8) == 0 || kind == name->kind)
                    return (sint32)(flags >> 6);
            }
        }
        ++name_index;
        path_node = path_node->links.registry_next;
    }
    return -1;
}

const ScenePath *SceneSection::FindPath(sint32 index) const
{
    FUNCTION_MARKER(0x800229FC, "SLUS_005.78");
    const ScenePath *path = (const ScenePath *)object_heads[3];
    while (path != 0)
    {
        if ((path->node.state_flags >> 6) == (uint32)index)
            return path;
        path = (const ScenePath *)path->node.links.registry_next;
    }
    return 0;
}

uint32 GetSectionRespawnForm(const SceneSection *section)
{
    FUNCTION_MARKER(0x800255F8, "SLUS_005.78");
    sint32 index;
    index = section->section_index;
    return (uint32)(sint32)(sint8)level_resource.sections[index].metadata;
}

const ScenePath *SceneSection::FindDefaultPath() const
{
    FUNCTION_MARKER(0x80022AE4, "SLUS_005.78");
    const ScenePath *path = (const ScenePath *)object_heads[3];
    while (path != 0)
    {
        if ((path->node.state_flags & 4) != 0)
            return path;
        path = (const ScenePath *)path->node.links.registry_next;
    }
    return 0;
}
