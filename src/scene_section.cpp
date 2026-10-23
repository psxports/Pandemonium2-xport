#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "scene_section.h"
#include "game.h"
#include "scene_events.h"

const ScenePath *SceneSection::FindPath(sint32 idx) const
{
    FUNCTION_MARKER(0x800229FC, "SLUS_005.78");
    const ScenePath *path = (const ScenePath *)object_heads[3];
    while (path != 0)
    {
        if ((path->node.state_flags >> 6) == (uint32)idx)
            return path;
        path = (const ScenePath *)path->node.links.registry_next;
    }
    return 0;
}

sint32 SceneSection::FindObjectId(const SceneObjectName *name) const
{
    FUNCTION_MARKER(0x80022A38, "SLUS_005.78");
    uint32 kind, flags, name_idx = 0;
    const SceneObjectName *object_name;
    const SceneNode *path_node;
    path_node = object_heads[3];
    while (path_node != 0)
    {
        object_name = path_names + name_idx;
        if (strncmp(object_name->prefix, name->prefix, 2) == 0)
        {
            kind = object_name->kind;
            if (kind != 60 && kind != 62)
            {
                flags = path_node->state_flags;
                if ((flags & 8) == 0 || kind == name->kind)
                    return flags >> 6;
            }
        }
        ++name_idx;
        path_node = path_node->links.registry_next;
    }
    return -1;
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

uint32 SceneSection::GetSectionRespawnForm(const SceneSection *section)
{
    FUNCTION_MARKER(0x800255F8, "SLUS_005.78");
    sint32 idx;
    idx = section->section_idx;
    return (sint8)level_resource.sections[idx].respawn_form;
}

void SceneSection::Activate()
{
    FUNCTION_MARKER(0x80025828, "SLUS_005.78");
    sint32 idx;
    uint32 val, flags, result;
    const SectionDefinition *section;
    section_idx = scene_loader.loading_section;
    pal_bank = (uint8)scene_loader.tex_slot_base;
    idx = (sint8)scene_loader.loading_section;
    section = level_resource.sections + idx;
    val = section->far_distance;
    flags = state_flags;
    far_distance = val;
    fog_near = section->fog_near;
    val = section->fog_far;
    flags &= 0xFFE7;
    state_flags = flags;
    fog_far = val;
    result = (section->flags & 0x10) | 0x40;
    state_flags = flags | result;
}

void SceneSection::UpdateSceneObjects(void)
{
    SceneNode *node;
    SceneSection *section;
    FUNCTION_MARKER(0x8005CD24, "SLUS_005.78");
    uint32 flags = game_flow.transition_flags;
    SceneNode *registered;
    uint32 pending;
    scene_events.count = 0;
    if ((flags & 0x40) != 0)
    {
        game_flow.transition_flags = flags & 0xFFFFFFBF;
        SceneStream::ApplySceneSectionTransition();
    }
    if (scr_render.fade_level < 0x1000)
    {
        pending = scene_events.pending;
        if (pending != 0xFFFFFFFF)
        {
            sint32 count = scene_events.count;
            if (count < 40)
            {
                scene_events.count = count + 1;
                scene_events.entries[count] = pending | 0x80000;
            }
            scene_events.pending = 0xFFFFFFFF;
        }
        section = scene_stream.active_section;
        node = section->object_heads[4];
        SceneTrigger::ProcessSceneTriggers(node, section->object_heads[7]);
        section = scene_stream.active_section;
        SceneTrigger::ProcessSceneTriggers(section->object_heads[5], 0);
        CharacterContactPool::FinishCharacterContacts();
        registered = registered_object_head;
        while (registered != 0)
        {
            AnimatedObject *object = (AnimatedObject *)registered;
            if (((registered->flags & 0x81) == 0 || object->contact_kind == 1) && object->contact_kind != 0 && (object->contact_kind != 3 || object->state != 30))
                object->UpdateObjectCollisions();
            registered = registered->links.registry_next;
        }
        section = scene_stream.active_section;
        node = section->object_heads[1];
        while (node != 0)
        {
            if ((node->flags & 0x81) == 0 && ((const AnimatedObject *)node)->contact_kind != 0)
                ((AnimatedObject *)node)->UpdateObjectCollisions();
            node = node->links.registry_next;
        }
        SceneEvents::UpdateSceneEventObjects();
    }
    section = scene_stream.active_section != 0 ? scene_stream.active_section : 0;
    CameraProjection::DrawSceneFrame(section);
    sound_effects.Flush();
}

void SceneSection::Relocate(const MaterialCatalog *resource)
{
    FUNCTION_MARKER(0x80068A24, "SLUS_005.78");
    uint32 node, val, i, grp;
    uintptr_t src_begin = (uintptr_t)data;
    uint32 face_src_slots = data_bytes / sizeof(uint32);
    const TextureDescriptor **src_face_texs = face_src_slots != 0 ? (const TextureDescriptor **)calloc(face_src_slots, sizeof(const TextureDescriptor *)) : 0;
    uint32 animation_header_capacity = 0;
    uint32 *src_animation_offsets;
    uint32 actor_grp_counts[2] = {0, 0};
    animated_object_count = 0;
    for (uint32 scan_grp = 0; scan_grp < 2; ++scan_grp)
    {
        uint32 scan_node = object_heads[scan_grp] != 0 ? (((uintptr_t)object_heads[scan_grp] - src_begin)) : 0;
        while (scan_node != 0)
        {
            const uint8 *src_actor = data + scan_node;
            ++actor_grp_counts[scan_grp];
            ++animated_object_count;
            if (((const EncodedAnimatedObject *)src_actor)->animation_offset != 0)
                ++animation_header_capacity;
            uint32 next = ((const EncodedAnimatedObject *)src_actor)->node.registry_offset;
            scan_node = next != 0 ? next : 0;
        }
    }
    animations = animation_header_capacity != 0 ? (AnimationHeader *)calloc(animation_header_capacity, sizeof(AnimationHeader)) : 0;
    src_animation_offsets = animation_header_capacity != 0 ? (uint32 *)malloc(animation_header_capacity * sizeof(uint32)) : 0;
    animation_count = 0;
    animated_objects = animated_object_count != 0 ? (AnimatedObject *)malloc(animated_object_count * sizeof(AnimatedObject)) : 0;
    uint32 *src_actor_offsets = animated_object_count != 0 ? (uint32 *)malloc(animated_object_count * sizeof(uint32)) : 0;
    uint32 native_actor_idx = 0;
    node = object_heads[0] != 0 ? (((uintptr_t)object_heads[0] - src_begin)) : 0;
    transient_links.first_child = 0;
    for (grp = 0; grp < 2; ++grp)
    {
        uint32 grp_first = native_actor_idx;
        if (grp != 0)
            node = object_heads[1] != 0 ? (((uintptr_t)object_heads[1] - src_begin)) : 0;
        while (node != 0)
        {
            const uint8 *encoded = data + node;
            src_actor_offsets[native_actor_idx] = node;
            AnimatedObject *actor = animated_objects + native_actor_idx;
            const EncodedAnimatedObject *root = (const EncodedAnimatedObject *)encoded;
            actor->node.links.registry_next = native_actor_idx + 1 < grp_first + actor_grp_counts[grp] ? &animated_objects[native_actor_idx + 1].node : 0;
            actor->node.kind = root->node.kind;
            actor->node.flags = root->node.flags;
            actor->node.state_flags = root->node.state_flags;
            actor->flags = root->flags;
            actor->face_count = root->geometry_counts;
            actor->vertex_count = root->geometry_counts >> 16;
            actor->event_word = root->event_word;
            actor->behavior_data = root->behavior_data;
            actor->state = root->state_word;
            actor->animation_binding.word = root->animation_binding.word;
            actor->path_motion_scale = root->motion_scale;
            actor->clone_bytes = root->clone_bytes;
            actor->path_flags = root->path_flags;
            actor->lighting_idx = root->lighting_idx;
            actor->resource_halfword58 = root->resource_halfword58;
            actor->sound_voice = root->sound_voice;
            actor->mesh_span.first_vertex = root->mesh_span.first_vertex;
            actor->mesh_span.vertex_count = root->mesh_span.vertex_count;
            actor->mesh_span.first_polygon = root->mesh_span.first_polygon;
            actor->mesh_span.polygon_count = root->mesh_span.polygon_count;
            actor->ticks = root->ticks;
            actor->frame = root->frame;
            actor->path_frame = root->path_frame;
            actor->yaw = root->yaw;
            actor->target_yaw = root->target_yaw;
            actor->pitch = root->pitch;
            actor->pitch_scale = root->pitch_scale;
            actor->pos.x = root->pos.x;
            actor->pos.y = root->pos.y;
            actor->pos.z = root->pos.z;
            actor->world_pos.x = root->world_pos.x;
            actor->world_pos.y = root->world_pos.y;
            actor->world_pos.z = root->world_pos.z;
            actor->bounds.maximum.x = root->bounds.maximum.x;
            actor->bounds.maximum.y = root->bounds.maximum.y;
            actor->bounds.maximum.z = root->bounds.maximum.z;
            actor->velocity.x = root->velocity.x;
            actor->velocity.y = root->velocity.y;
            actor->velocity.z = root->velocity.z;
            actor->model_contacts.blocker = 0;
            actor->model_contacts.surface0 = 0;
            actor->model_contacts.surface1 = 0;
            actor->model_contacts.surface2 = 0;
            actor->model_contacts.fraction0 = 0;
            actor->model_contacts.fraction1 = 0;
            actor->model_contacts.fraction2 = 0;
            const CollisionSphere *src_spheres = root->spheres;
            for (uint32 sphere = 0; sphere < 4; ++sphere)
                actor->spheres[sphere] = src_spheres[sphere];
            actor->orientation[0] = root->rotation.w;
            actor->orientation[1] = root->rotation.x;
            actor->orientation[2] = root->rotation.y;
            actor->orientation[3] = root->rotation.z;
            actor->movement_mode = root->movement_mode;
            actor->behavior = root->behavior;
            actor->contact_kind = root->contact_kind;
            actor->behavior_suspended = root->behavior_suspended;
            actor->animation = 0;
            actor->unbound_animation_word = 0;
            actor->clone_storage_bytes = 0;
            actor->polygons = 0;
            actor->material_animation_entries = 0;
            SceneNode **link_fields[] = {&actor->node.links.sibling_next, &actor->node.links.first_child};
            for (i = 0; i < 2; ++i)
            {
                val = (i == 0 ? root->node.sibling_offset : root->node.child_offset);
                *link_fields[i] = val != 0 ? (SceneNode *)(data + val) : 0;
            }
            val = root->mesh_offset;
            actor->mesh_state = val != 0 ? (data + val) : 0;
            val = root->vertices_offset;
            actor->vertices = val != 0 ? (MeshVertex *)(data + val) : 0;
            val = root->definition_offset;
            actor->path_definition = val != 0 ? (const PathDefinition *)(data + val) : 0;
            val = root->polygon_word;
            if (val != 0)
            {
                uint32 polygon = val;
                uint32 flags = actor->flags & 12;
                actor->polygons = (MeshFace *)(data + polygon);
                if (flags != 8)
                {
                    sint32 face_count = actor->face_count;
                    uint32 count = face_count;
                    while (count != 0)
                    {
                        const EncodedMeshFace *face = (const EncodedMeshFace *)(data + polygon);
                        uint32 idx = (face)->tex_word;
                        --count;
                        src_face_texs[polygon / sizeof(uint32)] = resource->texs + idx;
                        polygon += sizeof(EncodedMeshFace);
                    }
                }
            }
            val = root->animation_offset;
            if (val != 0)
            {
                uint32 header_idx = 0;
                while (header_idx < animation_count && src_animation_offsets[header_idx] != val)
                    ++header_idx;
                if (header_idx == animation_count)
                {
                    src_animation_offsets[animation_count++] = val;
                    animations[header_idx].Relocate(data, data_bytes, (const EncodedAnimationHeader *)(data + val));
                }
                actor->animation = animations + header_idx;
            }
            val = root->materials_offset;
            if (val != 0)
                actor->material_animation_entries = (uint32 *)(data + val);
            if (actor->behavior == 31)
                actor->linked_object = (AnimatedObject *)(data + root->event_word);

            actor->spr = 0;
            uint32 next = root->node.registry_offset;
            node = next != 0 ? next : 0;
            ++native_actor_idx;
        }
        object_heads[grp] = actor_grp_counts[grp] != 0 ? &animated_objects[grp_first].node : 0;
    }
    node = object_heads[2] != 0 ? (((uintptr_t)object_heads[2] - src_begin)) : 0;
    while (node != 0)
    {
        const uint8 *encoded = data + node;
        val = ((const EncodedMeshGeometryNode *)encoded)->polygons_offset;
        if (val != 0)
        {
            uint32 polygon = val;
            sint32 active = (sint16)xport_load_le16(encoded + 34);
            if (active > 0 && (((const EncodedMeshGeometryNode *)encoded)->flags & 0x400) == 0)
            {
                sint32 face_count = (sint16)xport_load_le16(encoded + 32);
                uint32 count = face_count;
                while (count != 0)
                {
                    const EncodedMeshFace *face = (const EncodedMeshFace *)(data + polygon);
                    uint32 idx = (face)->tex_word;
                    --count;
                    src_face_texs[polygon / sizeof(uint32)] = resource->texs + idx;
                    polygon += sizeof(EncodedMeshFace);
                }
            }
        }
        uint32 next_offset = ((const EncodedSceneNode *)encoded)->registry_offset;
        node = next_offset != 0 ? next_offset : 0;
    }
    uint32 *src_path_offsets;
    {
        uint32 matrix_count = 0, unique_count = 0;
        node = object_heads[3] != 0 ? (((uintptr_t)object_heads[3] - src_begin)) : 0;
        path_count = 0;
        while (node != 0)
        {
            const uint8 *encoded = data + node;
            ++path_count;
            if (((const EncodedScenePath *)encoded)->matrix_offset != 0)
                ++matrix_count;
            uint32 next = ((const EncodedSceneNode *)encoded)->registry_offset;
            node = next != 0 ? next : 0;
        }
        paths = path_count != 0 ? (ScenePath *)malloc(path_count * sizeof(ScenePath)) : 0;
        src_path_offsets = path_count != 0 ? (uint32 *)malloc(path_count * sizeof(uint32)) : 0;
        path_offset_matrices = matrix_count != 0 ? (MATRIX *)malloc(matrix_count * sizeof(MATRIX)) : 0;
        uint32 *matrix_sources = matrix_count != 0 ? (uint32 *)malloc(matrix_count * sizeof(uint32)) : 0;
        node = object_heads[3] != 0 ? (((uintptr_t)object_heads[3] - src_begin)) : 0;
        for (uint32 path_idx = 0; path_idx < path_count; ++path_idx)
        {
            const uint8 *encoded = data + node;
            const EncodedScenePath *record = (const EncodedScenePath *)encoded;
            src_path_offsets[path_idx] = node;
            ScenePath *path = paths + path_idx;
            path->node.links.registry_next = path_idx + 1 < path_count ? &paths[path_idx + 1].node : 0;
            SceneNode **links_val[] = {&path->node.links.sibling_next, &path->node.links.first_child};
            for (uint32 link_idx = 0; link_idx < 2; ++link_idx)
            {
                uint32 offset = (link_idx == 0 ? record->node.sibling_offset : record->node.child_offset);
                *links_val[link_idx] = offset != 0 ? (SceneNode *)(data + offset) : 0;
            }
            path->node.kind = record->node.kind;
            path->node.flags = record->node.flags;
            path->node.state_flags = record->node.state_flags;
            for (uint32 side = 0; side < 2; ++side)
            {
                uint32 offset = record->adjacent_offsets[side];
                path->adjacent[side] = offset != 0 ? (const ScenePath *)(data + offset) : 0;
                path->motion_limits[side] = record->motion_limits[side];
            }
            uint32 definition = record->definition_offset;
            path->definition = (const PathDefinition *)(data + definition);
            uint32 definition_flags = ((const EncodedPathDefinition *)(data + definition))->flags;
            uint32 fallback = record->fallback_word;
            path->fallback = 0;
            path->slope_scale = 0;
            if ((definition_flags & 0x100010) != 0)
            {
                if (fallback != 0)
                {
                    uint32 flags = path->node.state_flags;
                    if ((flags & ((flags & 0x10) | 8)) == 0)
                    {
                        // Resolve the file reference before reading its path flags
                        const uint8 *reference = data + fallback;
                        if ((xport_load_le16(reference + 14) & 8) != 0)
                            fallback = ((const EncodedScenePath *)reference)->adjacent_offsets[0];
                    }
                    path->fallback = (const ScenePath *)(data + fallback);
                }
                else
                    path->fallback = 0;
            }
            else
                path->slope_scale = fallback;
            uint32 matrix = record->matrix_offset;
            path->offset_matrix = 0;
            if (matrix != 0)
            {
                uint32 slot = 0;
                while (slot < unique_count && matrix_sources[slot] != matrix)
                    ++slot;
                if (slot == unique_count)
                {
                    memcpy(path_offset_matrices + slot, (data + matrix), sizeof(MATRIX));
                    matrix_sources[slot] = matrix;
                    ++unique_count;
                }
                path->offset_matrix = path_offset_matrices + slot;
            }
            path->offset_track.key_count = record->offset_track.key_count;
            path->offset_track.resource_byte1 = record->offset_track.resource_byte1;
            path->offset_track.flags = record->offset_track.flags;
            path->offset_track.node_key = record->offset_track.node_key;
            path->offset_track.frame_limit = record->offset_track.frame_limit;
            path->offset_track.frame_count = record->offset_track.frame_count;
            path->offset_track.frame = record->offset_track.frame;
            path->offset_track.resource_halfword10 = record->offset_track.resource_halfword10;
            uint32 keys = record->offset_track.keys_offset;
            path->offset_track.keys = keys != 0 ? (data + keys) : 0;
            uint32 next = record->node.registry_offset;
            node = next != 0 ? next : 0;
        }
        free(matrix_sources);
        path_name_count = path_count;
        path_names = path_count != 0 ? (SceneObjectName *)malloc(path_count * sizeof(SceneObjectName)) : 0;
        for (uint32 path_idx = 0; path_idx < path_count; ++path_idx)
        {
            uint32 offset = src_path_offsets[path_idx];
            memcpy(path_names + path_idx, (data + (offset + 60)), sizeof(SceneObjectName));
        }
        object_heads[3] = path_count != 0 ? &paths[0].node : 0;
    }
    node = object_heads[4] != 0 ? (((uintptr_t)object_heads[4] - src_begin)) : 0;
    while (node != 0)
    {
        const uint8 *encoded = data + node;
        if (encoded[15] == 8)
        {
            SceneTrigger camera_input = {0};
            camera_input.parameters.cutscene.camera_id = ((const EncodedSceneTrigger *)encoded)->parameters.cutscene.camera_id;
            camera_input.parameters.cutscene.next_camera = ((const EncodedSceneTrigger *)encoded)->parameters.cutscene.next_camera;
            camera_input.parameters.cutscene.duration = ((const EncodedSceneTrigger *)encoded)->parameters.cutscene.duration;
            camera_input.cutscene_control.flags = xport_load_le16(encoded + 80);
            camera_input.cutscene_control.idx = xport_load_le16(encoded + 82);
            CutscenePlayback::RegisterCutsceneCamera(&camera_input);
        }
        uint32 next_offset = ((const EncodedSceneNode *)encoded)->registry_offset;
        node = next_offset != 0 ? next_offset : 0;
    }
    uint32 *src_placement_offsets;
    {
        uint32 transform_count = 0, unique_count = 0;
        placement_count = 0;
        node = object_heads[9] != 0 ? (((uintptr_t)object_heads[9] - src_begin)) : 0;
        while (node != 0)
        {
            const uint8 *encoded = data + node;
            ++placement_count;
            if (((const EncodedActorPlacement *)encoded)->path.transform_offset != 0)
                ++transform_count;
            uint32 next = ((const EncodedSceneNode *)encoded)->registry_offset;
            node = next != 0 ? next : 0;
        }
        placements = placement_count != 0 ? (ActorPlacement *)malloc(placement_count * sizeof(ActorPlacement)) : 0;
        src_placement_offsets = placement_count != 0 ? (uint32 *)malloc(placement_count * sizeof(uint32)) : 0;
        definition_transforms = transform_count != 0 ? (PathTransform *)malloc(transform_count * sizeof(PathTransform)) : 0;
        uint32 *transform_sources = transform_count != 0 ? (uint32 *)malloc(transform_count * sizeof(uint32)) : 0;
        node = object_heads[9] != 0 ? (((uintptr_t)object_heads[9] - src_begin)) : 0;
        for (uint32 placement_idx = 0; placement_idx < placement_count; ++placement_idx)
        {
            const uint8 *encoded = data + node;
            src_placement_offsets[placement_idx] = node;
            const EncodedActorPlacement *record = (const EncodedActorPlacement *)encoded;
            ActorPlacement *placement = placements + placement_idx;
            placement->path.node.links.registry_next = placement_idx + 1 < placement_count ? &placements[placement_idx + 1].path.node : 0;
            SceneNode **links_val0[] = {&placement->path.node.links.sibling_next, &placement->path.node.links.first_child};
            for (uint32 link_idx = 0; link_idx < 2; ++link_idx)
            {
                uint32 offset = (link_idx == 0 ? record->path.node.sibling_offset : record->path.node.child_offset);
                *links_val0[link_idx] = offset != 0 ? (SceneNode *)(data + offset) : 0;
            }
            placement->path.node.kind = record->path.node.kind;
            placement->path.node.flags = record->path.node.flags;
            placement->path.node.state_flags = record->path.node.state_flags;
            placement->path.flags = record->path.flags;
            uint32 keys = record->path.pos.track.keys_offset;
            if (keys != 0)
            {
                placement->path.pos_track.key_count = record->path.pos.track.key_count;
                placement->path.pos_track.resource_byte1 = record->path.pos.track.resource_byte1;
                placement->path.pos_track.flags = record->path.pos.track.flags;
                placement->path.pos_track.node_key = record->path.pos.track.node_key;
                placement->path.pos_track.frame_limit = record->path.pos.track.frame_limit;
                placement->path.pos_track.frame_count = record->path.pos.track.frame_count;
                placement->path.pos_track.frame = record->path.pos.track.frame;
                placement->path.pos_track.resource_halfword10 = record->path.pos.track.resource_halfword10;
            }
            else
            {
                placement->path.static_pos.x = record->path.pos.static_pos.x;
                placement->path.static_pos.y = record->path.pos.static_pos.y;
                placement->path.static_pos.z = record->path.pos.static_pos.z;
            }
            placement->path.pos_track.keys = keys != 0 ? (data + keys) : 0;
            placement->path.frame_scale = record->path.frame_scale;
            placement->path.search_radius = record->path.search_radius;
            uint32 transform = record->path.transform_offset;
            placement->path.transform = 0;
            if (transform != 0)
            {
                uint32 slot = 0;
                while (slot < unique_count && transform_sources[slot] != transform)
                    ++slot;
                if (slot == unique_count)
                {
                    const uint8 *src = data + transform;
                    memcpy(&definition_transforms[slot].matrix, src, sizeof(MATRIX));
                    definition_transforms[slot].displacement.x = ((const PathTransform *)src)->displacement.x;
                    definition_transforms[slot].displacement.y = ((const PathTransform *)src)->displacement.y;
                    definition_transforms[slot].displacement.z = ((const PathTransform *)src)->displacement.z;
                    transform_sources[slot] = transform;
                    ++unique_count;
                }
                placement->path.transform = definition_transforms + slot;
            }
            uint32 state = record->state_offset;
            placement->state = state != 0 ? (data + state) : 0;
            uint32 next = record->path.node.registry_offset;
            node = next != 0 ? next : 0;
        }
        free(transform_sources);
        object_heads[9] = placement_count != 0 ? &placements[0].path.node : 0;
    }
    uint32 path_key_bytes = 0;
    uint32 path_key_tracks = 0;
    uint32 path_key_cursor = 0;
    const uint8 *section_data = data;
    const uint32 path_key_heads[2] = {3, 9};
    for (uint32 pass = 0; pass < 2; ++pass)
    {
        if (pass != 0)
        {
            path_key_data = path_key_tracks != 0 ? (uint8 *)malloc(path_key_bytes != 0 ? path_key_bytes : 1) : 0;
        }
        for (uint32 key_grp = 0; key_grp < 2; ++key_grp)
        {
            SceneNode *key_node = object_heads[path_key_heads[key_grp]];
            while (key_node != 0)
            {
                AnimationTrack *track = key_grp == 0 ? &((ScenePath *)key_node)->offset_track : &((ActorPlacement *)key_node)->path.pos_track;
                if (track->keys != 0)
                {
                    uint32 key_offset = track->keys - section_data;
                    uint32 key_bytes = (uint32)track->key_count * sizeof(VectorAnimationKey);
                    uint32 prefix_bytes = 0;
                    for (uint32 key_idx = 0; key_idx < track->key_count; ++key_idx)
                    {
                        uint32 curve_bytes = track->keys[key_idx * sizeof(VectorAnimationKey)] * sizeof(AnimationEaseCurve);
                        if (curve_bytes > prefix_bytes)
                            prefix_bytes = curve_bytes;
                    }
                    uint32 bytes = prefix_bytes + key_bytes;
                    if (pass == 0)
                    {
                        path_key_bytes += bytes;
                        ++path_key_tracks;
                    }
                    else
                    {
                        uint8 *dst = path_key_data + path_key_cursor;
                        memcpy(dst, track->keys - prefix_bytes, bytes);
                        track->keys = dst + prefix_bytes;
                        path_key_cursor += bytes;
                    }
                }
                key_node = key_node->links.registry_next;
            }
        }
    }
    const EncodedActorBinding *src_binding = actor_source_offset != 0 ? (const EncodedActorBinding *)(data + actor_source_offset) : 0;
    uint32 binding_count = 0;
    while (src_binding != 0)
    {
        uint32 src_offset = ((uintptr_t)src_binding - src_begin);
        ++binding_count;
        val = src_binding->next_offset;
        src_binding = val != 0 ? (const EncodedActorBinding *)(data + val) : 0;
    }
    ActorBinding *bindings = binding_count != 0 ? (ActorBinding *)malloc(binding_count * sizeof(ActorBinding)) : 0;
    src_binding = actor_source_offset != 0 ? (const EncodedActorBinding *)(data + actor_source_offset) : 0;
    for (uint32 binding_idx = 0; binding_idx < binding_count; ++binding_idx)
    {
        ActorBinding *binding = bindings + binding_idx;
        const EncodedActorBinding *encoded = src_binding;
        uint32 next = encoded->next_offset;
        uint32 definition = encoded->placement_offset;
        uint32 payload = encoded->rotation_offset;
        binding->event_command = encoded->event_command;
        binding->distance_sq = encoded->distance_sq;
        binding->motion_speed = encoded->motion_speed;
        binding->reaction_flags = encoded->reaction_flags;
        binding->interaction_timer = encoded->interaction_timer;
        binding->interaction_heading_ticks = encoded->interaction_heading_ticks;
        binding->interaction_height = encoded->interaction_height;
        binding->pending_event = encoded->pending_event;
        binding->range_ticks = encoded->range_ticks;
        binding->action_speed = encoded->action_speed;
        binding->sampled_pitch = encoded->sampled_pitch;
        binding->reaction_state = encoded->reaction_state;
        binding->path_frame = encoded->path_frame;
        binding->initial_path_frame = encoded->initial_path_frame;
        binding->motion_frame = encoded->motion_frame;
        binding->path_pitch_speed = encoded->path_pitch_speed;
        binding->respawn_ticks = encoded->respawn_ticks;
        binding->respawn_delay = encoded->respawn_delay;
        binding->sound_pitch = encoded->sound_pitch;
        binding->sound_voice = encoded->sound_voice;
        binding->path_heading = encoded->path_heading;
        binding->route_heading = encoded->route_heading;
        binding->action_flags = encoded->action_flags;
        binding->action_effect_flags = encoded->action_effect_flags;
        binding->action_reset_half = encoded->action_reset_half;
        binding->desired_pitch = encoded->desired_pitch;
        binding->height_dir_changes = encoded->height_dir_changes;
        binding->blink_active = encoded->blink_active;
        binding->blink_ticks = encoded->blink_ticks;
        binding->model_kind = encoded->model_kind;
        binding->flags = encoded->flags;
        binding->action_idx = encoded->action_idx;
        binding->action_phase = encoded->action_phase;
        binding->turn_delay = encoded->turn_delay;
        binding->projectile_phase = encoded->projectile_phase;
        binding->next_projectile_frame = encoded->next_projectile_frame;
        binding->route_mode = encoded->route_mode;
        binding->route_dir_mode = encoded->route_dir_mode;
        binding->turn_ticks = encoded->turn_ticks;
        for (uint32 component = 0; component < 2; ++component)
            binding->path_parameters[component] = encoded->path_parameters[component];
        binding->path_dir = encoded->path_dir;
        binding->route_dir = encoded->route_dir;
        for (uint32 component = 0; component < 6; ++component)
            binding->trigger_ticks[component] = encoded->trigger_ticks[component];
        binding->projectile_velocity = encoded->projectile_velocity;
        binding->sampled_velocity = encoded->sampled_velocity;
        binding->next = binding_idx + 1 < binding_count ? bindings + binding_idx + 1 : 0;
        binding->model = 0;
        binding->instance = 0;
        binding->sound = 0;

        binding->event_node = 0;
        binding->definition = (ActorPlacement *)(data + definition);
        binding->rotation_track = 0;
        if (payload != 0)
        {
            const EncodedAnimationTrack *src_track = (const EncodedAnimationTrack *)(data + payload);
            uint32 key_offset = src_track->keys_offset;
            uint32 key_bytes = (uint32)src_track->key_count * sizeof(QuaternionAnimationKey);
            uint32 prefix_bytes = 0;
            const uint8 *src_keys = 0;
            if (key_offset != 0)
            {
                src_keys = data + key_offset;
                for (uint32 key_idx = 0; key_idx < src_track->key_count; ++key_idx)
                {
                    uint32 curve_bytes = src_keys[key_idx * sizeof(QuaternionAnimationKey)] * sizeof(AnimationEaseCurve);
                    if (curve_bytes > prefix_bytes)
                        prefix_bytes = curve_bytes;
                }
            }
            else
                key_bytes = 0;
            AnimationTrack *track = (AnimationTrack *)malloc(sizeof(AnimationTrack) + prefix_bytes + key_bytes);
            track->key_count = src_track->key_count;
            track->resource_byte1 = src_track->resource_byte1;
            track->flags = src_track->flags;
            track->node_key = src_track->node_key;
            track->frame_limit = src_track->frame_limit;
            track->frame_count = src_track->frame_count;
            track->frame = src_track->frame;
            track->resource_halfword10 = src_track->resource_halfword10;
            if (key_offset != 0)
            {
                uint8 *storage = (uint8 *)(track + 1);
                const uint8 *src_span = data + (key_offset - prefix_bytes);
                memcpy(storage, src_span, prefix_bytes + key_bytes);
                track->keys = storage + prefix_bytes;
            }
            else
                track->keys = 0;
            binding->rotation_track = track;
        }
        ActorPlacement *placement = (ActorPlacement *)object_heads[9];
        while (placement != 0)
        {
            if (placement->state == src_binding)
                placement->state = binding;
            placement = (ActorPlacement *)placement->path.node.links.registry_next;
        }
        src_binding = next != 0 ? (const EncodedActorBinding *)(data + next) : 0;
    }
    actor_bindings = bindings;
    actor_source_offset = 0;
    const AnimatedPathState *src_state = animated_paths;
    uint32 animated_path_count = 0;
    while (src_state != 0)
    {
        ++animated_path_count;
        val = ((const EncodedAnimatedPathState *)src_state)->next_offset;
        src_state = val != 0 ? (const AnimatedPathState *)(data + val) : 0;
    }
    AnimatedPathState *states = animated_path_count != 0 ? (AnimatedPathState *)malloc(animated_path_count * sizeof(AnimatedPathState)) : 0;
    src_state = animated_paths;
    for (uint32 path_idx = 0; path_idx < animated_path_count; ++path_idx)
    {
        AnimatedPathState *state = states + path_idx;
        const EncodedAnimatedPathState *encoded = (const EncodedAnimatedPathState *)src_state;
        uint32 next = encoded->next_offset;
        state->next = path_idx + 1 < animated_path_count ? states + path_idx + 1 : 0;
        state->start_frame = encoded->start_frame;
        state->sound_voice = encoded->sound_voice;
        state->sound = 0;
        state->speed[0] = encoded->speed[0];
        state->dir.x = encoded->dir.x;
        state->speed[1] = encoded->speed[1];
        state->dir.y = encoded->dir.y;
        state->speed[2] = encoded->speed[2];
        state->dir.z = encoded->dir.z;
        ActorPlacement *placement = (ActorPlacement *)object_heads[9];
        while (placement != 0)
        {
            if (placement->state == src_state)
                placement->state = state;
            placement = (ActorPlacement *)placement->path.node.links.registry_next;
        }
        src_state = next != 0 ? (const AnimatedPathState *)(data + next) : 0;
    }
    animated_paths = states;
    uint32 extent_capacity = 0;
    node = object_heads[12] != 0 ? (((uintptr_t)object_heads[12] - src_begin)) : 0;
    while (node != 0)
    {
        const uint8 *encoded = data + node;
        if ((sint8)encoded[27] != -1)
            ++extent_capacity;
        uint32 next = ((const EncodedSceneNode *)encoded)->registry_offset;
        node = next != 0 ? next : 0;
    }
    spr_extent_tracks = extent_capacity != 0 ? (AnimationTrack **)calloc(extent_capacity, sizeof(AnimationTrack *)) : 0;
    uint32 *src_extent_offsets = extent_capacity != 0 ? (uint32 *)malloc(extent_capacity * sizeof(uint32)) : 0;
    spr_extent_track_count = 0;
    {
        const SceneLight *src_light = (const SceneLight *)object_heads[14];
        const MeshGeometryNode *src_mesh = (const MeshGeometryNode *)object_heads[2];
        const PositionedBillboardNode *src_billboard = (const PositionedBillboardNode *)object_heads[8];
        const MeshPartNode *src_part = (const MeshPartNode *)object_heads[13];
        const SphereNode *src_sphere = (const SphereNode *)object_heads[15];
        const SpriteRecord *src_spr = (const SpriteRecord *)object_heads[12];
        const SceneNode **src_nodes;
        SceneNode **native_nodes;
        uint32 owner_count, owner_idx, mesh_idx, light_idx, path_idx, billboard_idx;
        uint32 part_idx, sphere_idx, placement_idx, actor_idx, spr_idx;
        uint32 actor_first, actor_limit, spr_first, spr_limit;
        uint32 trigger_idx, trigger_first, trigger_limit;
        light_count = 0;
        mesh_count = 0;
        billboard_count = 0;
        mesh_part_count = 0;
        sphere_node_count = 0;
        spr_count = 0;
        trigger_count = 0;
        while (src_light != 0)
        {
            uint32 src_offset = ((uintptr_t)src_light - src_begin);
            ++light_count;
            uint32 next_offset = ((const EncodedSceneNode *)src_light)->registry_offset;
            src_light = next_offset != 0 ? (const SceneLight *)(data + next_offset) : 0;
        }
        while (src_mesh != 0)
        {
            ++mesh_count;
            uint32 next_offset = ((const EncodedSceneNode *)src_mesh)->registry_offset;
            src_mesh = next_offset != 0 ? (const MeshGeometryNode *)(data + next_offset) : 0;
        }
        while (src_billboard != 0)
        {
            uint32 src_offset = ((uintptr_t)src_billboard - src_begin);
            ++billboard_count;
            uint32 next_offset = ((const EncodedSceneNode *)src_billboard)->registry_offset;
            src_billboard = next_offset != 0 ? (const PositionedBillboardNode *)(data + next_offset) : 0;
        }
        while (src_part != 0)
        {
            uint32 src_offset = ((uintptr_t)src_part - src_begin);
            ++mesh_part_count;
            uint32 next_offset = ((const EncodedSceneNode *)src_part)->registry_offset;
            src_part = next_offset != 0 ? (const MeshPartNode *)(data + next_offset) : 0;
        }
        while (src_sphere != 0)
        {
            uint32 src_offset = ((uintptr_t)src_sphere - src_begin);
            ++sphere_node_count;
            uint32 next_offset = ((const EncodedSceneNode *)src_sphere)->registry_offset;
            src_sphere = next_offset != 0 ? (const SphereNode *)(data + next_offset) : 0;
        }
        while (src_spr != 0)
        {
            ++spr_count;
            uint32 next_offset = ((const EncodedSceneNode *)src_spr)->registry_offset;
            src_spr = next_offset != 0 ? (const SpriteRecord *)(data + next_offset) : 0;
        }
        for (uint32 trigger_grp = 4; trigger_grp <= 6; ++trigger_grp)
        {
            const SceneNode *src_trigger = object_heads[trigger_grp];
            while (src_trigger != 0)
            {
                uint32 src_offset = ((uintptr_t)(src_trigger)-src_begin);
                ++trigger_count;
                uint32 next_offset = ((const EncodedSceneNode *)src_trigger)->registry_offset;
                src_trigger = next_offset != 0 ? (const SceneNode *)(data + next_offset) : 0;
            }
        }
        owner_count = light_count + mesh_count + path_count + billboard_count + mesh_part_count + sphere_node_count + trigger_count + spr_count + animated_object_count + placement_count;
        actor_limit = owner_count - placement_count;
        actor_first = actor_limit - animated_object_count;
        spr_limit = actor_first;
        spr_first = spr_limit - spr_count;
        trigger_limit = spr_first;
        trigger_first = trigger_limit - trigger_count;
        lights = light_count != 0 ? (SceneLight *)malloc(light_count * sizeof(SceneLight)) : 0;
        meshes = mesh_count != 0 ? (MeshGeometryNode *)malloc(mesh_count * sizeof(MeshGeometryNode)) : 0;
        billboards = billboard_count != 0 ? (PositionedBillboardNode *)malloc(billboard_count * sizeof(PositionedBillboardNode)) : 0;
        mesh_parts = mesh_part_count != 0 ? (MeshPartNode *)malloc(mesh_part_count * sizeof(MeshPartNode)) : 0;
        sphere_nodes = sphere_node_count != 0 ? (SphereNode *)malloc(sphere_node_count * sizeof(SphereNode)) : 0;
        triggers = trigger_count != 0 ? (SceneTrigger *)malloc(trigger_count * sizeof(SceneTrigger)) : 0;
        sprs = spr_count != 0 ? (SpriteRecord *)malloc(spr_count * sizeof(SpriteRecord)) : 0;
        src_nodes = owner_count != 0 ? (const SceneNode **)malloc(owner_count * sizeof(*src_nodes)) : 0;
        native_nodes = owner_count != 0 ? (SceneNode **)malloc(owner_count * sizeof(*native_nodes)) : 0;
        src_light = (const SceneLight *)object_heads[14];
        owner_idx = 0;
        for (light_idx = 0; light_idx < light_count; ++light_idx)
        {
            uint32 src_offset = ((uintptr_t)src_light - src_begin);
            src_nodes[owner_idx] = (const SceneNode *)src_light;
            native_nodes[owner_idx] = (SceneNode *)(lights + light_idx);
            const uint8 *encoded = (const uint8 *)src_light;
            const EncodedSceneLight *record = (const EncodedSceneLight *)encoded;
            SceneLight *decoded = lights + light_idx;
            SceneNode **link_fields[] = {&decoded->links.registry_next, &decoded->links.sibling_next, &decoded->links.first_child};
            for (uint32 link_idx = 0; link_idx < 3; ++link_idx)
            {
                uint32 link_offset = (link_idx == 0 ? record->registry_offset : (link_idx == 1 ? record->sibling_offset : record->child_offset));
                *link_fields[link_idx] = link_offset != 0 ? (SceneNode *)(data + link_offset) : 0;
            }
            decoded->kind = record->kind;
            decoded->flags = record->flags;
            decoded->event_flags = record->event_flags;
            decoded->id = record->id;
            decoded->event_id = record->event_id;
            decoded->pos.x = record->pos.x;
            decoded->src.pos.x = record->src.pos.x;
            decoded->prev_pos.x = record->prev_pos.x;
            decoded->relative_pos.x = record->relative_pos.x;
            decoded->color[0] = record->color[0];
            decoded->color_scale[0] = record->color_scale[0];
            decoded->pos.y = record->pos.y;
            decoded->src.pos.y = record->src.pos.y;
            decoded->prev_pos.y = record->prev_pos.y;
            decoded->relative_pos.y = record->relative_pos.y;
            decoded->color[1] = record->color[1];
            decoded->color_scale[1] = record->color_scale[1];
            decoded->pos.z = record->pos.z;
            decoded->src.pos.z = record->src.pos.z;
            decoded->prev_pos.z = record->prev_pos.z;
            decoded->relative_pos.z = record->relative_pos.z;
            decoded->color[2] = record->color[2];
            decoded->color_scale[2] = record->color_scale[2];
            decoded->radius = record->radius;
            decoded->attenuation_range = record->attenuation_range;
            decoded->metadata_word = record->metadata_word;
            ++owner_idx;
            uint32 next_offset = record->registry_offset;
            src_light = next_offset != 0 ? (const SceneLight *)(data + next_offset) : 0;
        }
        src_mesh = (const MeshGeometryNode *)object_heads[2];
        for (mesh_idx = 0; mesh_idx < mesh_count; ++mesh_idx)
        {
            uint32 src_offset = ((uintptr_t)src_mesh - src_begin);
            src_nodes[owner_idx] = &src_mesh->node;
            native_nodes[owner_idx] = &meshes[mesh_idx].node;
            const uint8 *encoded = (const uint8 *)src_mesh;
            const EncodedMeshGeometryNode *record = (const EncodedMeshGeometryNode *)encoded;
            MeshGeometryNode *mesh = meshes + mesh_idx;
            SceneNode **link_fields[] = {&mesh->node.links.registry_next, &mesh->node.links.sibling_next, &mesh->node.links.first_child};
            for (uint32 link_idx = 0; link_idx < 3; ++link_idx)
            {
                uint32 link_offset = (link_idx == 0 ? record->node.registry_offset : (link_idx == 1 ? record->node.sibling_offset : record->node.child_offset));
                *link_fields[link_idx] = link_offset != 0 ? (SceneNode *)(data + link_offset) : 0;
            }
            mesh->node.kind = record->node.kind;
            mesh->node.flags = record->node.flags;
            mesh->node.state_flags = record->node.state_flags;
            mesh->pos.x = record->pos.x;
            mesh->pos.y = record->pos.y;
            mesh->pos.z = record->pos.z;
            mesh->flags = record->flags;
            mesh->polygon_count = record->polygon_count;
            mesh->vertex_count = record->vertex_count;
            uint32 polygon_offset = record->polygons_offset;
            mesh->polygons = polygon_offset != 0 ? (MeshFace *)(data + polygon_offset) : 0;
            uint32 geometry_offset = record->geometry_offset;

            if (mesh->vertex_count > 0)
                mesh->vertices = geometry_offset != 0 ? (MeshVertex *)(data + geometry_offset) : 0;
            else
                mesh->src_mesh = geometry_offset != 0 ? (MeshGeometryNode *)(data + geometry_offset) : 0;
            uint32 material_offset = record->materials_offset;
            mesh->material_animation_entries = material_offset != 0 ? (uint32 *)(data + material_offset) : 0;
            mesh->lighting_idx = record->lighting_idx;
            mesh->first_vertex = record->first_vertex;
            ++owner_idx;
            uint32 next_offset = record->node.registry_offset;
            src_mesh = next_offset != 0 ? (const MeshGeometryNode *)(data + next_offset) : 0;
        }
        for (path_idx = 0; path_idx < path_count; ++path_idx)
        {
            src_nodes[owner_idx] = (const SceneNode *)(data + src_path_offsets[path_idx]);
            native_nodes[owner_idx] = &paths[path_idx].node;
            ++owner_idx;
        }
        src_billboard = (const PositionedBillboardNode *)object_heads[8];
        for (billboard_idx = 0; billboard_idx < billboard_count; ++billboard_idx)
        {
            uint32 src_offset = ((uintptr_t)src_billboard - src_begin);
            src_nodes[owner_idx] = &src_billboard->billboard.node;
            native_nodes[owner_idx] = &billboards[billboard_idx].billboard.node;
            const uint8 *encoded = (const uint8 *)src_billboard;
            const EncodedPositionedBillboardNode *record = (const EncodedPositionedBillboardNode *)encoded;
            PositionedBillboardNode *decoded = billboards + billboard_idx;
            SceneNode **link_fields[] = {&decoded->billboard.node.links.registry_next, &decoded->billboard.node.links.sibling_next, &decoded->billboard.node.links.first_child};
            for (uint32 link_idx = 0; link_idx < 3; ++link_idx)
            {
                uint32 link_offset = (link_idx == 0 ? record->billboard.node.registry_offset : (link_idx == 1 ? record->billboard.node.sibling_offset : record->billboard.node.child_offset));
                *link_fields[link_idx] = link_offset != 0 ? (SceneNode *)(data + link_offset) : 0;
            }
            decoded->billboard.node.kind = record->billboard.node.kind;
            decoded->billboard.node.flags = record->billboard.node.flags;
            decoded->billboard.node.state_flags = record->billboard.node.state_flags;
            decoded->billboard.event_id = record->billboard.event_id;
            decoded->billboard.playback_flags = record->billboard.playback_flags;
            uint32 type = decoded->billboard.playback_flags & 0x7F;
            decoded->billboard.tex = resource_grps[type + 13].tex;
            decoded->billboard.extent = record->billboard.extent;
            decoded->pos.x = record->pos.x;
            decoded->world_pos.x = record->world_pos.x;
            decoded->pos.y = record->pos.y;
            decoded->world_pos.y = record->world_pos.y;
            decoded->pos.z = record->pos.z;
            decoded->world_pos.z = record->world_pos.z;
            ++owner_idx;
            uint32 next_offset = record->billboard.node.registry_offset;
            src_billboard = next_offset != 0 ? (const PositionedBillboardNode *)(data + next_offset) : 0;
        }
        src_part = (const MeshPartNode *)object_heads[13];
        for (part_idx = 0; part_idx < mesh_part_count; ++part_idx)
        {
            uint32 src_offset = ((uintptr_t)src_part - src_begin);
            src_nodes[owner_idx] = &src_part->node;
            native_nodes[owner_idx] = &mesh_parts[part_idx].node;
            const uint8 *encoded = (const uint8 *)src_part;
            const EncodedMeshPartNode *record = (const EncodedMeshPartNode *)encoded;
            MeshPartNode *decoded = mesh_parts + part_idx;
            SceneNode **link_fields[] = {&decoded->node.links.registry_next, &decoded->node.links.sibling_next, &decoded->node.links.first_child};
            for (uint32 link_idx = 0; link_idx < 3; ++link_idx)
            {
                uint32 link_offset = (link_idx == 0 ? record->node.registry_offset : (link_idx == 1 ? record->node.sibling_offset : record->node.child_offset));
                *link_fields[link_idx] = link_offset != 0 ? (SceneNode *)(data + link_offset) : 0;
            }
            decoded->node.kind = record->node.kind;
            decoded->node.flags = record->node.flags;
            decoded->node.state_flags = record->node.state_flags;
            decoded->pos.x = record->pos.x;
            decoded->pos.y = record->pos.y;
            decoded->pos.z = record->pos.z;
            decoded->animation_binding.word = record->animation_binding.word;
            decoded->span.first_vertex = record->span.first_vertex;
            decoded->span.vertex_count = record->span.vertex_count;
            decoded->span.first_polygon = record->span.first_polygon;
            decoded->span.polygon_count = record->span.polygon_count;
            decoded->rotation[0] = record->rotation.w;
            decoded->rotation[1] = record->rotation.x;
            decoded->rotation[2] = record->rotation.y;
            decoded->rotation[3] = record->rotation.z;
            ++owner_idx;
            uint32 next_offset = record->node.registry_offset;
            src_part = next_offset != 0 ? (const MeshPartNode *)(data + next_offset) : 0;
        }
        src_sphere = (const SphereNode *)object_heads[15];
        for (sphere_idx = 0; sphere_idx < sphere_node_count; ++sphere_idx)
        {
            uint32 src_offset = ((uintptr_t)src_sphere - src_begin);
            src_nodes[owner_idx] = &src_sphere->node;
            native_nodes[owner_idx] = &sphere_nodes[sphere_idx].node;
            const uint8 *encoded = (const uint8 *)src_sphere;
            const EncodedSphereNode *record = (const EncodedSphereNode *)encoded;
            SphereNode *decoded = sphere_nodes + sphere_idx;
            SceneNode **link_fields[] = {&decoded->node.links.registry_next, &decoded->node.links.sibling_next, &decoded->node.links.first_child};
            for (uint32 link_idx = 0; link_idx < 3; ++link_idx)
            {
                uint32 link_offset = (link_idx == 0 ? record->node.registry_offset : (link_idx == 1 ? record->node.sibling_offset : record->node.child_offset));
                *link_fields[link_idx] = link_offset != 0 ? (SceneNode *)(data + link_offset) : 0;
            }
            decoded->node.kind = record->node.kind;
            decoded->node.flags = record->node.flags;
            decoded->node.state_flags = record->node.state_flags;
            decoded->pos.x = record->pos.x;
            decoded->pos.y = record->pos.y;
            decoded->pos.z = record->pos.z;
            decoded->animation_binding.word = record->animation_binding.word;
            decoded->radius = record->radius;
            ++owner_idx;
            uint32 next_offset = record->node.registry_offset;
            src_sphere = next_offset != 0 ? (const SphereNode *)(data + next_offset) : 0;
        }
        trigger_idx = 0;
        for (uint32 trigger_grp = 4; trigger_grp <= 6; ++trigger_grp)
        {
            const SceneTrigger *src_trigger = (const SceneTrigger *)object_heads[trigger_grp];
            while (src_trigger != 0)
            {
                uint32 src_offset = ((uintptr_t)(src_trigger)-src_begin);
                const uint8 *encoded = (const uint8 *)src_trigger;
                uint32 src_bytes = (xport_load_le16(encoded + 14) & 2) != 0 ? SceneTrigger::ControlBytes : SceneTrigger::AnimatedBytes;
                SceneTrigger *trigger = triggers + trigger_idx;
                const EncodedSceneTrigger *record = (const EncodedSceneTrigger *)encoded;
                memset(trigger, 0, sizeof(SceneTrigger));
                SceneNode **link_fields[] = {&trigger->node.links.registry_next, &trigger->node.links.sibling_next, &trigger->node.links.first_child};
                for (uint32 link_idx = 0; link_idx < 3; ++link_idx)
                {
                    uint32 link_offset = (link_idx == 0 ? ((const EncodedSceneNode *)encoded)->registry_offset : (link_idx == 1 ? ((const EncodedSceneNode *)encoded)->sibling_offset : ((const EncodedSceneNode *)encoded)->child_offset));
                    *link_fields[link_idx] = link_offset != 0 ? (SceneNode *)(data + link_offset) : 0;
                }
                trigger->node.kind = record->node.kind;
                trigger->node.flags = record->node.flags;
                trigger->node.state_flags = record->node.state_flags;
                for (uint32 column = 0; column < 3; ++column)
                    trigger->transform.m[0][column] = record->transform.m[0][column];
                trigger->transform.t[0] = record->transform.t[0];
                trigger->delta.x = record->delta.x;
                trigger->extent[0] = record->extent[0];
                for (uint32 column = 0; column < 3; ++column)
                    trigger->transform.m[1][column] = record->transform.m[1][column];
                trigger->transform.t[1] = record->transform.t[1];
                trigger->delta.y = record->delta.y;
                trigger->extent[1] = record->extent[1];
                for (uint32 column = 0; column < 3; ++column)
                    trigger->transform.m[2][column] = record->transform.m[2][column];
                trigger->transform.t[2] = record->transform.t[2];
                trigger->delta.z = record->delta.z;
                trigger->extent[2] = record->extent[2];
                trigger->radius = record->radius;
                trigger->control_word = record->control_word;
                sint32 trigger_kind = (sint8)(trigger->node.state_flags >> 8);
                const EncodedTriggerParams *control = &record->parameters;
                if (trigger_grp == 6)
                    trigger->parameters.path_pair.tangent_dir = control->path_pair.tangent_dir;
                else
                {
                    switch (trigger_kind)
                    {
                        case 2:
                            trigger->parameters.section_id = control->section_id;
                            break;
                        case 3:
                            trigger->parameters.exit = control->exit;
                            break;
                        case 4:
                            trigger->parameters.motion = control->motion;
                            break;
                        case 5:
                            trigger->parameters.damage.deadline = control->damage.deadline;
                            trigger->parameters.damage.flags = control->damage.flags;
                            break;
                        case 6:
                        case 9:
                            trigger->parameters.impulse_strength = control->impulse_strength;
                            break;
                        case 8:
                            trigger->parameters.cutscene = control->cutscene;
                            break;
                        case 10:
                        case 11:
                            trigger->parameters.jump = control->jump;
                            break;
                        case 13:
                            trigger->parameters.reaction.deadline = control->reaction.deadline;
                            trigger->parameters.reaction.flags = control->reaction.flags;
                            break;
                        case 17:
                            trigger->parameters.level = control->level;
                            break;
                        case 19:
                            trigger->parameters.far_distance = control->far_distance;
                            break;
                        case 20:
                            trigger->parameters.trail_effect = control->trail_effect;
                            break;
                        case 21:
                            trigger->parameters.form = control->form;
                            break;
                        case 23:
                            trigger->parameters.state_flags = control->state_flags;
                            break;
                        case 30:
                            trigger->parameters.animation = control->animation;
                            break;
                        case 50:
                            trigger->parameters.sound = control->sound;
                            break;
                        default:
                            trigger->parameters.event = control->event;
                            break;
                    }
                }
                uint32 path_count = trigger_grp == 6 ? 2 : (trigger_grp == 4 && (trigger_kind == 7 || ((uint32)(trigger_kind - 26) & 0xFF) < 2) ? 1 : 0);
                for (uint32 path_idx = 0; path_idx < path_count; ++path_idx)
                {
                    uint32 path_offset = (trigger_grp == 6 ? control->path_pair.target_offsets[path_idx] : control->path.target_offset);
                    const ScenePath *path = path_offset != 0 ? (const ScenePath *)(data + path_offset) : 0;
                    if (trigger_grp == 6)
                        trigger->parameters.path_pair.targets[path_idx] = path;
                    else
                        trigger->parameters.path.target = path;
                }
                if (trigger_grp == 6)
                    trigger->parameters.path_pair.tangent_dir = control->path_pair.tangent_dir;
                else if (path_count != 0)
                {
                    trigger->parameters.path.frame_word = control->path.frame_word;
                    trigger->parameters.path.duration_ticks = control->path.duration_ticks;
                }
                if (src_bytes == SceneTrigger::ControlBytes)
                    trigger->animation_binding.word = 0xFFFFFFFF;
                else
                {
                    trigger->animation_binding.word = record->animation_binding.word;
                    trigger->pos.x = record->pos.x;
                    trigger->pos.y = record->pos.y;
                    trigger->pos.z = record->pos.z;
                    trigger->rotation[0] = record->rotation.w;
                    trigger->rotation[1] = record->rotation.x;
                    trigger->rotation[2] = record->rotation.y;
                    trigger->rotation[3] = record->rotation.z;
                }
                src_nodes[owner_idx] = &src_trigger->node;
                native_nodes[owner_idx] = &trigger->node;
                ++owner_idx;
                ++trigger_idx;
                uint32 next_offset = ((const EncodedSceneNode *)encoded)->registry_offset;
                src_trigger = next_offset != 0 ? (const SceneTrigger *)(data + next_offset) : 0;
            }
        }
        src_spr = (const SpriteRecord *)object_heads[12];
        for (spr_idx = 0; spr_idx < spr_count; ++spr_idx)
        {
            uint32 src_offset = ((uintptr_t)src_spr - src_begin);
            src_nodes[owner_idx] = &src_spr->node;
            native_nodes[owner_idx] = &sprs[spr_idx].node;
            const uint8 *encoded = (const uint8 *)src_spr;
            const EncodedSpriteRecord *record = (const EncodedSpriteRecord *)encoded;
            SpriteRecord *spr = sprs + spr_idx;
            SceneNode **link_fields[] = {&spr->node.links.registry_next, &spr->node.links.sibling_next, &spr->node.links.first_child};
            for (uint32 link_idx = 0; link_idx < 3; ++link_idx)
            {
                uint32 link_offset = (link_idx == 0 ? record->node.registry_offset : (link_idx == 1 ? record->node.sibling_offset : record->node.child_offset));
                *link_fields[link_idx] = link_offset != 0 ? (SceneNode *)(data + link_offset) : 0;
            }
            spr->node.kind = record->node.kind;
            spr->node.flags = record->node.flags;
            spr->node.state_flags = record->node.state_flags;
            uint32 tex_idx = record->tex_idx;
            spr->tex = resource->texs + tex_idx;
            uint32 payload = record->animation_offset;
            spr->animation_entries = payload != 0 ? (uint32 *)(data + payload) : 0;
            spr->animation_binding.word = record->animation_binding.word;
            spr->extent_track = 0;
            spr->pos.x = record->pos.x;
            spr->pos.y = record->pos.y;
            spr->pos.z = record->pos.z;
            for (uint32 component = 0; component < 2; ++component)
            {
                spr->extent[component] = record->extent[component];
                spr->angles[component] = record->angles[component];
            }
            if (spr->animation_binding.tracks[3] != -1)
            {
                payload = spr->animation_binding.word;
                uint32 track_idx = 0;
                while (track_idx < spr_extent_track_count && src_extent_offsets[track_idx] != payload)
                    ++track_idx;
                if (track_idx == spr_extent_track_count)
                {
                    const EncodedAnimationTrack *encoded = (const EncodedAnimationTrack *)(data + payload);
                    uint32 key_offset = encoded->keys_offset;
                    uint32 key_count = encoded->key_count;
                    uint32 key_bytes = key_offset != 0 ? key_count * sizeof(VectorAnimationKey) : 0;
                    uint32 prefix_bytes = 0;
                    if (key_offset != 0)
                    {
                        const uint8 *keys = data + key_offset;
                        for (uint32 key_idx = 0; key_idx < key_count; ++key_idx)
                        {
                            uint32 prefix = keys[key_idx * sizeof(VectorAnimationKey)] * sizeof(AnimationEaseCurve);
                            if (prefix > prefix_bytes)
                                prefix_bytes = prefix;
                        }
                    }
                    AnimationTrack *track = (AnimationTrack *)malloc(sizeof(AnimationTrack) + prefix_bytes + key_bytes);
                    track->key_count = encoded->key_count;
                    track->resource_byte1 = encoded->resource_byte1;
                    track->flags = encoded->flags;
                    track->node_key = encoded->node_key;
                    track->frame_limit = encoded->frame_limit;
                    track->frame_count = encoded->frame_count;
                    track->frame = encoded->frame;
                    track->resource_halfword10 = encoded->resource_halfword10;
                    track->keys = 0;
                    if (key_offset != 0)
                    {
                        uint8 *storage = (uint8 *)(track + 1);
                        memcpy(storage, (data + (key_offset - prefix_bytes)), prefix_bytes + key_bytes);
                        track->keys = storage + prefix_bytes;
                    }
                    src_extent_offsets[track_idx] = payload;
                    spr_extent_tracks[track_idx] = track;
                    ++spr_extent_track_count;
                }
                spr->extent_track = spr_extent_tracks[track_idx];
            }
            ++owner_idx;
            uint32 next_offset = record->node.registry_offset;
            src_spr = next_offset != 0 ? (const SpriteRecord *)(data + next_offset) : 0;
        }
        free(src_extent_offsets);
        for (actor_idx = 0; actor_idx < animated_object_count; ++actor_idx)
        {
            src_nodes[owner_idx] = (const SceneNode *)(data + src_actor_offsets[actor_idx]);
            native_nodes[owner_idx] = &animated_objects[actor_idx].node;
            ++owner_idx;
        }
        for (placement_idx = 0; placement_idx < placement_count; ++placement_idx)
        {
            src_nodes[owner_idx] = (const SceneNode *)(data + src_placement_offsets[placement_idx]);
            native_nodes[owner_idx] = &placements[placement_idx].path.node;
            ++owner_idx;
        }
        if (owner_count != 0)
        {
            for (uint32 graph_grp = 0; graph_grp < 16; ++graph_grp)
            {
                SceneNode *src_node = object_heads[graph_grp];
                while (src_node != 0)
                {
                    SceneNode *working_node = src_node;
                    for (owner_idx = 0; owner_idx < owner_count; ++owner_idx)
                    {
                        if (src_node == src_nodes[owner_idx])
                        {
                            working_node = native_nodes[owner_idx];
                            break;
                        }
                    }
                    SceneNode *next_node = working_node->links.registry_next;
                    SceneNode **fields[] = {&working_node->links.registry_next, &working_node->links.sibling_next, &working_node->links.first_child};
                    for (uint32 field_idx = 0; field_idx < 3; ++field_idx)
                    {
                        for (owner_idx = 0; owner_idx < owner_count; ++owner_idx)
                        {
                            if (*fields[field_idx] == src_nodes[owner_idx])
                            {
                                *fields[field_idx] = native_nodes[owner_idx];
                                break;
                            }
                        }
                    }
                    src_node = next_node;
                }
            }
            for (uint32 graph_grp = 0; graph_grp < 16; ++graph_grp)
            {
                for (owner_idx = 0; owner_idx < owner_count; ++owner_idx)
                {
                    if (object_heads[graph_grp] == src_nodes[owner_idx])
                    {
                        object_heads[graph_grp] = native_nodes[owner_idx];
                        break;
                    }
                }
            }
            SceneNode **root_fields[] = {&links.registry_next, &links.sibling_next, &links.first_child, &transient_links.registry_next, &transient_links.sibling_next, &transient_links.first_child, &registered_object_head, &character_pad.support.owner, &character_pad.support.contact, &character_pad.surface_interaction.sel, &spr_pool.first, &spr_pool.free_head, &spr_pool.active_head};
            for (uint32 field_idx = 0; field_idx < sizeof(root_fields) / sizeof(root_fields[0]); ++field_idx)
            {
                for (owner_idx = 0; owner_idx < owner_count; ++owner_idx)
                {
                    if (*root_fields[field_idx] == src_nodes[owner_idx])
                    {
                        *root_fields[field_idx] = native_nodes[owner_idx];
                        break;
                    }
                }
            }
            for (uint32 slot = 0; slot < 12; ++slot)
            {
                SceneNode **fields[] = {&character_pad.trigger_objects[slot], &character_pad.pickup_objects[slot]};
                for (uint32 field_idx = 0; field_idx < 2; ++field_idx)
                {
                    for (owner_idx = 0; owner_idx < owner_count; ++owner_idx)
                    {
                        if (*fields[field_idx] == src_nodes[owner_idx])
                        {
                            *fields[field_idx] = native_nodes[owner_idx];
                            break;
                        }
                    }
                }
            }
            SceneTrigger **trigger_fields[] = {&character_pad.support.candidate, &character_pad.support.impulse_surface, &character_pad.path_transition.anchor, &character_pad.surface_interaction.environment, &character_pad.surface_interaction.jump_scale, &character_pad.surface_interaction.surface, &character_pad.surface_interaction.prev_surface, &character_pad.contacts.section_trigger, &character_pad.contacts.hide_trigger, &character_pad.contacts.show_trigger, &cutscene.trigger};
            for (uint32 field_idx = 0; field_idx < sizeof(trigger_fields) / sizeof(trigger_fields[0]); ++field_idx)
            {
                for (owner_idx = trigger_first; owner_idx < trigger_limit; ++owner_idx)
                {
                    if (*trigger_fields[field_idx] == (const SceneTrigger *)src_nodes[owner_idx])
                    {
                        *trigger_fields[field_idx] = (SceneTrigger *)native_nodes[owner_idx];
                        break;
                    }
                }
            }
            for (uint32 slot = 0; slot < SpritePoolState::EntryCount; ++slot)
            {
                for (owner_idx = spr_first; owner_idx < spr_limit; ++owner_idx)
                {
                    if (spr_pool.entries[slot].node == src_nodes[owner_idx])
                    {
                        spr_pool.entries[slot].node = native_nodes[owner_idx];
                        break;
                    }
                }
            }
            for (uint32 slot = 0; slot < CharacterContactPool::Capacity; ++slot)
            {
                for (owner_idx = 0; owner_idx < owner_count; ++owner_idx)
                {
                    if (character_contact_pool.nodes[slot].object == src_nodes[owner_idx])
                    {
                        character_contact_pool.nodes[slot].object = native_nodes[owner_idx];
                        break;
                    }
                }
            }
            for (ActorBinding *binding = actor_bindings; binding != 0; binding = binding->next)
            {
                for (owner_idx = 0; owner_idx < owner_count; ++owner_idx)
                {
                    if (binding->event_node == src_nodes[owner_idx])
                    {
                        binding->event_node = native_nodes[owner_idx];
                        break;
                    }
                }
            }
            for (path_idx = 0; path_idx < path_count; ++path_idx)
            {
                for (owner_idx = owner_count - placement_count; owner_idx < owner_count; ++owner_idx)
                {
                    if (paths[path_idx].definition == (const PathDefinition *)src_nodes[owner_idx])
                    {
                        paths[path_idx].definition = (const PathDefinition *)native_nodes[owner_idx];
                        break;
                    }
                }
            }
            for (ActorBinding *binding = actor_bindings; binding != 0; binding = binding->next)
            {
                for (owner_idx = owner_count - placement_count; owner_idx < owner_count; ++owner_idx)
                {
                    if (binding->definition == (const ActorPlacement *)src_nodes[owner_idx])
                    {
                        binding->definition = (ActorPlacement *)native_nodes[owner_idx];
                        break;
                    }
                }
            }
            {
                AnimatedObject **object_fields[3 + CharacterController::ModelCount + sizeof(action_effect_resources.templates) / sizeof(action_effect_resources.templates[0]) + sizeof(character_pad.action_effects) / sizeof(character_pad.action_effects[0]) + ModelGroupRegistry::Capacity * sizeof(model_grps.recs[0].models) / sizeof(model_grps.recs[0].models[0]) + sizeof(actor_models.definitions) / sizeof(actor_models.definitions[0])];
                uint32 object_count = 3;
                object_fields[0] = &character_pad.object;
                object_fields[1] = &character_action_owner;
                object_fields[2] = &character_action_target;
                for (uint32 slot = 0; slot < sizeof(action_effect_resources.templates) / sizeof(action_effect_resources.templates[0]); ++slot)
                    object_fields[object_count++] = &action_effect_resources.templates[slot];
                for (uint32 slot = 0; slot < sizeof(character_pad.action_effects) / sizeof(character_pad.action_effects[0]); ++slot)
                    object_fields[object_count++] = &character_pad.action_effects[slot];
                for (uint32 slot = 0; slot < CharacterController::ModelCount; ++slot)
                {
                    if (character_pad.model_slots[slot] != 0)
                        object_fields[object_count++] = &character_pad.model_slots[slot]->object;
                }
                for (uint32 grp_idx = 0; grp_idx < ModelGroupRegistry::Capacity; ++grp_idx)
                {
                    for (uint32 slot = 0; slot < sizeof(model_grps.recs[grp_idx].models) / sizeof(model_grps.recs[grp_idx].models[0]); ++slot)
                        object_fields[object_count++] = &model_grps.recs[grp_idx].models[slot];
                }
                for (uint32 slot = 0; slot < sizeof(actor_models.definitions) / sizeof(actor_models.definitions[0]); ++slot)
                    object_fields[object_count++] = &actor_models.definitions[slot].prototype;
                for (uint32 slot = 0; slot < object_count; ++slot)
                {
                    for (owner_idx = actor_first; owner_idx < actor_limit; ++owner_idx)
                    {
                        if (*object_fields[slot] == (const AnimatedObject *)src_nodes[owner_idx])
                        {
                            *object_fields[slot] = (AnimatedObject *)native_nodes[owner_idx];
                            break;
                        }
                    }
                }
                for (uint32 slot = 0; slot < animated_object_count + object_count; ++slot)
                {
                    AnimatedObject *actor = slot < animated_object_count ? animated_objects + slot : *object_fields[slot - animated_object_count];
                    if (actor == 0 || actor == NULL)
                        continue;
                    for (owner_idx = spr_first; owner_idx < spr_limit; ++owner_idx)
                    {
                        if (actor->spr == (const SpriteRecord *)src_nodes[owner_idx])
                        {
                            actor->spr = (SpriteRecord *)native_nodes[owner_idx];
                            break;
                        }
                    }
                    if (actor->contact_kind == 3)
                    {
                        SceneTrigger **contact_fields[] = {&actor->model_contacts.surface0, &actor->model_contacts.surface1, &actor->model_contacts.surface2};
                        for (uint32 field_idx = 0; field_idx < 3; ++field_idx)
                        {
                            for (owner_idx = trigger_first; owner_idx < trigger_limit; ++owner_idx)
                            {
                                if (*contact_fields[field_idx] == (const SceneTrigger *)src_nodes[owner_idx])
                                {
                                    *contact_fields[field_idx] = (SceneTrigger *)native_nodes[owner_idx];
                                    break;
                                }
                            }
                        }
                    }
                    SceneNode **node_fields[] = {&actor->owner_node, &actor->model_contacts.blocker};
                    uint32 node_field_count = actor->contact_kind == 3 ? 2 : 1;
                    for (uint32 field_idx = 0; field_idx < node_field_count; ++field_idx)
                    {
                        for (owner_idx = 0; owner_idx < owner_count; ++owner_idx)
                        {
                            if (*node_fields[field_idx] == src_nodes[owner_idx])
                            {
                                *node_fields[field_idx] = native_nodes[owner_idx];
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
                    for (uint32 field_idx = 0; field_idx < actor_field_count; ++field_idx)
                    {
                        for (owner_idx = actor_first; owner_idx < actor_limit; ++owner_idx)
                        {
                            if (*actor_fields[field_idx] == (const AnimatedObject *)src_nodes[owner_idx])
                            {
                                *actor_fields[field_idx] = (AnimatedObject *)native_nodes[owner_idx];
                                break;
                            }
                        }
                    }
                }
            }
            for (light_idx = 0; light_idx < light_count; ++light_idx)
            {
                const SceneLight *original_light = (const SceneLight *)src_nodes[light_idx];
                if (scene_lighting.lights == original_light)
                    scene_lighting.lights = lights + light_idx;
                for (uint32 point_idx = 0; point_idx < 4; ++point_idx)
                {
                    if (scene_lighting.point_lights[point_idx] == original_light)
                        scene_lighting.point_lights[point_idx] = lights + light_idx;
                }
            }
            for (mesh_idx = 0; mesh_idx < mesh_count; ++mesh_idx)
            {
                MeshGeometryNode *mesh = meshes + mesh_idx;
                if (mesh->vertex_count <= 0)
                {
                    for (owner_idx = light_count; owner_idx < light_count + mesh_count; ++owner_idx)
                    {
                        if (mesh->src_mesh == (const MeshGeometryNode *)src_nodes[owner_idx])
                        {
                            mesh->src_mesh = (MeshGeometryNode *)native_nodes[owner_idx];
                            break;
                        }
                    }
                }
                const MeshGeometryNode *original_mesh = (const MeshGeometryNode *)src_nodes[light_count + mesh_idx];
                if (scene_lighting.bounds_origin == original_mesh->pos.comp)
                    scene_lighting.bounds_origin = mesh->pos.comp;
            }
            for (path_idx = 0; path_idx < path_count; ++path_idx)
            {
                ScenePath *path = paths + path_idx;
                const ScenePath **fields[] = {&path->adjacent[0], &path->adjacent[1], &path->fallback};
                uint32 definition_flags = path->definition->flags;
                uint32 field_count = (definition_flags & 0x100010) != 0 ? 3 : 2;
                for (uint32 field_idx = 0; field_idx < field_count; ++field_idx)
                {
                    for (owner_idx = light_count + mesh_count; owner_idx < light_count + mesh_count + path_count; ++owner_idx)
                    {
                        if (*fields[field_idx] == (const ScenePath *)src_nodes[owner_idx])
                        {
                            *fields[field_idx] = (ScenePath *)native_nodes[owner_idx];
                            break;
                        }
                    }
                }
            }
            for (uint32 trigger_grp = 4; trigger_grp <= 6; trigger_grp += 2)
            {
                SceneTrigger *trigger = (SceneTrigger *)object_heads[trigger_grp];
                while (trigger != 0)
                {
                    const ScenePath **fields[2];
                    uint32 field_count = 0;
                    sint32 trigger_kind = (sint8)(trigger->node.state_flags >> 8);
                    if (trigger_grp == 6)
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
                    for (uint32 field_idx = 0; field_idx < field_count; ++field_idx)
                    {
                        for (owner_idx = light_count + mesh_count; owner_idx < light_count + mesh_count + path_count; ++owner_idx)
                        {
                            if (*fields[field_idx] == (const ScenePath *)src_nodes[owner_idx])
                            {
                                *fields[field_idx] = (ScenePath *)native_nodes[owner_idx];
                                break;
                            }
                        }
                    }
                    trigger = (SceneTrigger *)trigger->node.links.registry_next;
                }
            }
            const ScenePath **path_fields[] = {&camera_motion.path, &character_pad.path, &character_pad.path_transition.anchor_path, &character_pad.path_transition.path, &character_pad.path_transition.constrained_path, &cutscene.path};
            for (uint32 field_idx = 0; field_idx < sizeof(path_fields) / sizeof(path_fields[0]); ++field_idx)
            {
                for (owner_idx = light_count + mesh_count; owner_idx < light_count + mesh_count + path_count; ++owner_idx)
                {
                    if (*path_fields[field_idx] == (const ScenePath *)src_nodes[owner_idx])
                    {
                        *path_fields[field_idx] = (ScenePath *)native_nodes[owner_idx];
                        break;
                    }
                }
            }
        }

        // Merge source spans before copying so mutable aliases stay shared
        struct GeometryRange

        {
            uint32 first, last, dst;
        };

        uint32 range_capacity = mesh_count + animated_object_count;
        GeometryRange *face_ranges = range_capacity != 0 ? (GeometryRange *)malloc(range_capacity * sizeof(GeometryRange)) : 0;
        uint32 range_count = 0;
        for (uint32 geometry_idx = 0; geometry_idx < range_capacity; ++geometry_idx)
        {
            MeshFace *faces;
            sint32 count;
            if (geometry_idx < mesh_count)
            {
                faces = meshes[geometry_idx].polygons;
                count = meshes[geometry_idx].polygon_count;
            }
            else
            {
                AnimatedObject *actor = animated_objects + geometry_idx - mesh_count;
                faces = actor->polygons;
                count = actor->face_count;
            }
            if (count == 0)
                continue;
            uint32 span_count = count;
            uint32 first = ((uintptr_t)(faces)-src_begin);
            uint32 pos = range_count;
            while (pos != 0 && first < face_ranges[pos - 1].first)
            {
                face_ranges[pos] = face_ranges[pos - 1];
                --pos;
            }
            face_ranges[pos].first = first;
            face_ranges[pos].last = first + span_count * sizeof(EncodedMeshFace);
            face_ranges[pos].dst = 0;
            ++range_count;
        }
        uint32 merged_count = 0;
        for (uint32 range_idx = 0; range_idx < range_count; ++range_idx)
        {
            GeometryRange range = face_ranges[range_idx];
            if (merged_count != 0 && range.first <= face_ranges[merged_count - 1].last)
            {
                GeometryRange *prev = face_ranges + merged_count - 1;
                if (range.last > prev->last)
                    prev->last = range.last;
            }
            else
                face_ranges[merged_count++] = range;
        }
        face_count = 0;
        for (uint32 range_idx = 0; range_idx < merged_count; ++range_idx)
        {
            GeometryRange *range = face_ranges + range_idx;
            range->dst = face_count;
            face_count += (range->last - range->first) / sizeof(EncodedMeshFace);
        }
        face_storage = face_count != 0 ? (MeshFace *)malloc(face_count * sizeof(MeshFace)) : 0;
        for (uint32 range_idx = 0; range_idx < merged_count; ++range_idx)
        {
            const GeometryRange *range = face_ranges + range_idx;
            uint32 count = (range->last - range->first) / sizeof(EncodedMeshFace);
            for (uint32 face_idx = 0; face_idx < count; ++face_idx)
            {
                uint32 offset = range->first + face_idx * sizeof(EncodedMeshFace);
                const uint8 *encoded = data + offset;
                MeshFace *face = face_storage + range->dst + face_idx;
                face->tex = src_face_texs[offset / sizeof(uint32)];
                const EncodedMeshFace *src = (const EncodedMeshFace *)encoded;
                for (uint32 vertex = 0; vertex < 4; ++vertex)
                    face->vertex_idxs[vertex] = src->vertex_idxs[vertex];
                face->plane = src->plane;
                face->n = src->n;
            }
        }
        for (uint32 geometry_idx = 0; geometry_idx < range_capacity; ++geometry_idx)
        {
            MeshFace **faces = geometry_idx < mesh_count ? &meshes[geometry_idx].polygons : &animated_objects[geometry_idx - mesh_count].polygons;
            if (*faces == 0)
                continue;
            uint32 first = ((uintptr_t)(*faces) - src_begin);
            for (uint32 range_idx = 0; range_idx < merged_count; ++range_idx)
            {
                const GeometryRange *range = face_ranges + range_idx;
                if (first >= range->first && first < range->last)
                {
                    *faces = face_storage + range->dst + (first - range->first) / sizeof(EncodedMeshFace);
                    break;
                }
            }
        }
        free(face_ranges);
        free(src_face_texs);
        GeometryRange *vertex_ranges = range_capacity != 0 ? (GeometryRange *)malloc(range_capacity * sizeof(GeometryRange)) : 0;
        uint32 *vertex_extents = range_capacity != 0 ? (uint32 *)calloc(range_capacity, sizeof(uint32)) : 0;
        SceneNode **pending_nodes = animated_object_count != 0 ? (SceneNode **)malloc(owner_count * sizeof(SceneNode *)) : 0;
        uint32 vertex_range_count = 0;
        for (uint32 geometry_idx = 0; geometry_idx < range_capacity; ++geometry_idx)
        {
            MeshVertex *vertices;
            uint32 extent = 0;
            if (geometry_idx < mesh_count)
            {
                MeshGeometryNode *mesh = meshes + geometry_idx;
                if (mesh->vertex_count <= 0)
                    continue;
                vertices = mesh->vertices;
                extent = mesh->first_vertex + mesh->vertex_count;
            }
            else
            {
                AnimatedObject *actor = animated_objects + geometry_idx - mesh_count;
                vertices = actor->vertices;
                if (actor->mesh_span.vertex_count != 0)
                    extent = actor->mesh_span.first_vertex + actor->mesh_span.vertex_count;
                sint32 polygon_count = actor->face_count;
                for (sint32 polygon_idx = 0; polygon_idx < polygon_count; ++polygon_idx)
                {
                    for (uint32 corner = 0; corner < 4; ++corner)
                    {
                        uint32 end = actor->polygons[polygon_idx].vertex_idxs[corner] + 1;
                        if (end > extent)
                            extent = end;
                    }
                }
                uint32 pending_count = 0;
                if (actor->node.links.first_child != 0)
                    pending_nodes[pending_count++] = actor->node.links.first_child;
                while (pending_count != 0)
                {
                    SceneNode *part_node = pending_nodes[--pending_count];
                    if (part_node->kind == 5)
                    {
                        const MorphSpan *span = &((const MeshPartNode *)part_node)->span;
                        if (span->vertex_count != 0)
                        {
                            uint32 end = span->first_vertex + span->vertex_count;
                            if (end > extent)
                                extent = end;
                        }
                    }
                    SceneNode *next_nodes[] = {part_node->links.sibling_next, part_node->kind != 0 ? part_node->links.first_child : 0};
                    for (uint32 next_idx = 0; next_idx < 2; ++next_idx)
                    {
                        if (next_nodes[next_idx] != 0)
                        {
                            pending_nodes[pending_count++] = next_nodes[next_idx];
                        }
                    }
                }
            }
            vertex_extents[geometry_idx] = extent;
            if (extent == 0)
                continue;
            uint32 first = ((uintptr_t)(vertices)-src_begin);
            uint32 pos = vertex_range_count;
            while (pos != 0 && first < vertex_ranges[pos - 1].first)
            {
                vertex_ranges[pos] = vertex_ranges[pos - 1];
                --pos;
            }
            vertex_ranges[pos].first = first;
            vertex_ranges[pos].last = first + extent * sizeof(MeshVertex);
            vertex_ranges[pos].dst = 0;
            ++vertex_range_count;
        }
        uint32 merged_vertices = 0;
        for (uint32 range_idx = 0; range_idx < vertex_range_count; ++range_idx)
        {
            GeometryRange range = vertex_ranges[range_idx];
            if (merged_vertices != 0 && range.first <= vertex_ranges[merged_vertices - 1].last)
            {
                GeometryRange *prev = vertex_ranges + merged_vertices - 1;
                if (range.last > prev->last)
                    prev->last = range.last;
            }
            else
                vertex_ranges[merged_vertices++] = range;
        }
        vertex_count = 0;
        for (uint32 range_idx = 0; range_idx < merged_vertices; ++range_idx)
        {
            GeometryRange *range = vertex_ranges + range_idx;
            range->dst = vertex_count;
            vertex_count += (range->last - range->first) / sizeof(MeshVertex);
        }
        vertex_storage = vertex_count != 0 ? (MeshVertex *)malloc(vertex_count * sizeof(MeshVertex)) : 0;
        for (uint32 range_idx = 0; range_idx < merged_vertices; ++range_idx)
        {
            const GeometryRange *range = vertex_ranges + range_idx;
            memcpy(vertex_storage + range->dst, (data + range->first), range->last - range->first);
        }
        for (uint32 geometry_idx = 0; geometry_idx < range_capacity; ++geometry_idx)
        {
            if (geometry_idx < mesh_count && meshes[geometry_idx].vertex_count <= 0)
                continue;
            MeshVertex **vertices = geometry_idx < mesh_count ? &meshes[geometry_idx].vertices : &animated_objects[geometry_idx - mesh_count].vertices;
            if (*vertices == 0 || vertex_extents[geometry_idx] == 0)
                continue;
            uint32 first = ((uintptr_t)(*vertices) - src_begin);
            for (uint32 range_idx = 0; range_idx < merged_vertices; ++range_idx)
            {
                const GeometryRange *range = vertex_ranges + range_idx;
                if (first >= range->first && first < range->last)
                {
                    *vertices = vertex_storage + range->dst + (first - range->first) / sizeof(MeshVertex);
                    break;
                }
            }
        }
        free(pending_nodes);
        free(vertex_extents);
        free(vertex_ranges);
        GeometryRange *cache_ranges = animated_object_count != 0 ? (GeometryRange *)malloc(animated_object_count * sizeof(GeometryRange)) : 0;
        uint32 cache_range_count = 0;
        for (actor_idx = 0; actor_idx < animated_object_count; ++actor_idx)
        {
            const AnimatedObject *actor = animated_objects + actor_idx;
            if ((actor->flags & 0x100000) == 0 || actor->col_transforms == 0)
                continue;
            sint32 count = actor->col_transform_count;
            uint32 span_count = count;
            uint32 first = ((uintptr_t)actor->col_transforms - src_begin);
            uint32 pos = cache_range_count;
            while (pos != 0 && first < cache_ranges[pos - 1].first)
            {
                cache_ranges[pos] = cache_ranges[pos - 1];
                --pos;
            }
            cache_ranges[pos].first = first;
            cache_ranges[pos].last = first + span_count * sizeof(EncodedCollisionTransform);
            cache_ranges[pos].dst = 0;
            ++cache_range_count;
        }
        uint32 merged_caches = 0;
        for (uint32 range_idx = 0; range_idx < cache_range_count; ++range_idx)
        {
            GeometryRange range = cache_ranges[range_idx];
            if (merged_caches != 0 && range.first <= cache_ranges[merged_caches - 1].last)
            {
                GeometryRange *prev = cache_ranges + merged_caches - 1;
                if (range.last > prev->last)
                    prev->last = range.last;
            }
            else
                cache_ranges[merged_caches++] = range;
        }
        col_count = 0;
        for (uint32 range_idx = 0; range_idx < merged_caches; ++range_idx)
        {
            GeometryRange *range = cache_ranges + range_idx;
            range->dst = col_count;
            col_count += (range->last - range->first) / sizeof(EncodedCollisionTransform);
        }
        col_storage = col_count != 0 ? (CollisionTransform *)malloc(col_count * sizeof(CollisionTransform)) : 0;
        for (uint32 range_idx = 0; range_idx < merged_caches; ++range_idx)
        {
            const GeometryRange *range = cache_ranges + range_idx;
            const EncodedCollisionTransform *encoded = (const EncodedCollisionTransform *)(data + range->first);
            uint32 count = (range->last - range->first) / sizeof(EncodedCollisionTransform);
            for (uint32 idx = 0; idx < count; ++idx)
            {
                CollisionTransform *dst = col_storage + range->dst + idx;
                const EncodedCollisionTransform *src = encoded + idx;
                dst->first_face = src->first_face;
                dst->face_count = src->face_count;
                for (uint32 axis = 0; axis < 9; ++axis)
                    dst->rotation[axis] = src->rotation[axis];
                dst->pos = src->pos;
                dst->motion = src->motion;
            }
        }
        for (actor_idx = 0; actor_idx < animated_object_count; ++actor_idx)
        {
            AnimatedObject *actor = animated_objects + actor_idx;
            if ((actor->flags & 0x100000) == 0 || actor->col_transforms == 0)
                continue;
            uint32 first = ((uintptr_t)actor->col_transforms - src_begin);
            for (uint32 range_idx = 0; range_idx < merged_caches; ++range_idx)
            {
                const GeometryRange *range = cache_ranges + range_idx;
                if (first >= range->first && first < range->last)
                {
                    actor->col_transforms = col_storage + range->dst + (first - range->first) / sizeof(EncodedCollisionTransform);
                    break;
                }
            }
        }
        free(cache_ranges);
        AnimationHeader **animation_headers = animated_object_count != 0 ? (AnimationHeader **)malloc(animated_object_count * sizeof(AnimationHeader *)) : 0;
        uint32 animation_header_count = 0;
        for (actor_idx = 0; actor_idx < animated_object_count; ++actor_idx)
        {
            AnimationHeader *header = animated_objects[actor_idx].animation;
            if (header == 0)
                continue;
            uintptr_t header_offset = (uintptr_t)header - (uintptr_t)animations;
            uint32 header_idx = 0;
            while (header_idx < animation_header_count && animation_headers[header_idx] != header)
                ++header_idx;
            if (header_idx != animation_header_count)
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
        for (uint32 header_idx = 0; header_idx < animation_header_count; ++header_idx)
        {
            for (uint32 track_grp = 0; track_grp < 4; ++track_grp)
            {
                uint32 count = animation_headers[header_idx]->counts[track_grp];
                payload_reference_capacity += count;
            }
        }
        AnimationPayloadReference *payload_references = payload_reference_capacity != 0 ? (AnimationPayloadReference *)malloc(payload_reference_capacity * sizeof(AnimationPayloadReference)) : 0;
        GeometryRange *payload_ranges = payload_reference_capacity != 0 ? (GeometryRange *)malloc(payload_reference_capacity * 2u * sizeof(GeometryRange)) : 0;
        uint32 payload_reference_count = 0;
        uint32 payload_range_count = 0;
        for (uint32 header_idx = 0; header_idx < animation_header_count; ++header_idx)
        {
            AnimationHeader *header = animation_headers[header_idx];
            for (uint32 track_grp = 0; track_grp < 4; ++track_grp)
            {
                uint32 stride = track_grp == 0 ? sizeof(VectorAnimationTrack) : (track_grp == 3 ? sizeof(MorphAnimationTrack) : sizeof(AnimationTrack));
                uint32 key_stride = track_grp == 1 ? sizeof(QuaternionAnimationKey) : (track_grp == 3 ? sizeof(MorphAnimationKey) : sizeof(VectorAnimationKey));
                for (sint32 track_idx = 0; track_idx < header->counts[track_grp]; ++track_idx)
                {
                    AnimationTrack *track = (AnimationTrack *)(header->tracks[track_grp].bytes + track_idx * stride);
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
                        uint32 first = ((uintptr_t)track->keys - src_begin);
                        uint32 count = track->key_count;
                        uint32 curve_count = 0;
                        uint32 maximum_frame = 0;
                        for (uint32 key_idx = 0; key_idx < count; ++key_idx)
                        {
                            const uint8 *key = track->keys + key_idx * key_stride;
                            uint32 curve = key[0];
                            if (curve > curve_count)
                                curve_count = curve;
                            if (track_grp == 3)
                            {
                                uint32 frame_idx = ((const MorphAnimationKey *)key)->frame_idx;
                                if (frame_idx > maximum_frame)
                                    maximum_frame = frame_idx;
                            }
                        }
                        uint32 prefix = curve_count * sizeof(AnimationEaseCurve);
                        starts[range_slots] = first - prefix;
                        ends[range_slots++] = first + count * key_stride;
                        reference->key_offset = first;
                        reference->remap_keys = 1;
                        if (track_grp == 3)
                        {
                            MorphAnimationTrack *morph = (MorphAnimationTrack *)track;
                            first = ((uintptr_t)morph->vertices - src_begin);
                            uint32 frame_bytes = morph->vertex_stride * sizeof(SVECTOR);
                            starts[range_slots] = first;
                            ends[range_slots++] = first + (maximum_frame + 1u) * frame_bytes;
                            reference->morph_offset = first;
                            reference->remap_morph = 1;
                        }
                    }
                    for (uint32 range_slot = 0; range_slot < range_slots; ++range_slot)
                    {
                        uint32 pos = payload_range_count;
                        while (pos != 0 && starts[range_slot] < payload_ranges[pos - 1].first)
                        {
                            payload_ranges[pos] = payload_ranges[pos - 1];
                            --pos;
                        }
                        payload_ranges[pos].first = starts[range_slot];
                        payload_ranges[pos].last = ends[range_slot];
                        payload_ranges[pos].dst = 0;
                        ++payload_range_count;
                    }
                }
            }
        }
        uint32 merged_payloads = 0;
        for (uint32 range_idx = 0; range_idx < payload_range_count; ++range_idx)
        {
            GeometryRange range = payload_ranges[range_idx];
            if (merged_payloads != 0 && range.first <= payload_ranges[merged_payloads - 1].last)
            {
                if (range.last > payload_ranges[merged_payloads - 1].last)
                    payload_ranges[merged_payloads - 1].last = range.last;
            }
            else
                payload_ranges[merged_payloads++] = range;
        }
        animation_payload_bytes = 0;
        for (uint32 range_idx = 0; range_idx < merged_payloads; ++range_idx)
        {
            GeometryRange *range = payload_ranges + range_idx;
            range->dst = animation_payload_bytes;
            animation_payload_bytes += range->last - range->first;
        }
        animation_payload_data = animation_payload_bytes != 0 ? (uint8 *)malloc(animation_payload_bytes) : 0;
        for (uint32 range_idx = 0; range_idx < merged_payloads; ++range_idx)
        {
            const GeometryRange *range = payload_ranges + range_idx;
            memcpy(animation_payload_data + range->dst, (data + range->first), range->last - range->first);
        }
        for (uint32 reference_idx = 0; reference_idx < payload_reference_count; ++reference_idx)
        {
            const AnimationPayloadReference *reference = payload_references + reference_idx;
            for (uint32 range_idx = 0; range_idx < merged_payloads; ++range_idx)
            {
                const GeometryRange *range = payload_ranges + range_idx;
                if (reference->remap_keys != 0 && reference->key_offset >= range->first && reference->key_offset < range->last)
                    reference->track->keys = animation_payload_data + range->dst + reference->key_offset - range->first;
                if (reference->remap_morph != 0 && reference->morph_offset >= range->first && reference->morph_offset < range->last)
                    ((MorphAnimationTrack *)reference->track)->vertices = (const SVECTOR *)(animation_payload_data + range->dst + reference->morph_offset - range->first);
            }
        }
        free(payload_ranges);
        free(payload_references);
        {
            AnimatedObject *animation_objects[4 + CharacterController::ModelCount + sizeof(action_effect_resources.templates) / sizeof(action_effect_resources.templates[0]) + sizeof(character_pad.action_effects) / sizeof(character_pad.action_effects[0]) + ModelGroupRegistry::Capacity * sizeof(model_grps.recs[0].models) / sizeof(model_grps.recs[0].models[0]) + sizeof(actor_models.definitions) / sizeof(actor_models.definitions[0])];
            uint32 animation_object_count = 3;
            animation_objects[0] = character_pad.object;
            animation_objects[1] = character_action_owner;
            animation_objects[2] = character_action_target;
            if (character_pad.model != 0)
                animation_objects[animation_object_count++] = character_pad.model->object;
            for (uint32 slot = 0; slot < CharacterController::ModelCount; ++slot)
            {
                if (character_pad.model_slots[slot] != 0)
                    animation_objects[animation_object_count++] = character_pad.model_slots[slot]->object;
            }
            for (uint32 slot = 0; slot < sizeof(action_effect_resources.templates) / sizeof(action_effect_resources.templates[0]); ++slot)
                animation_objects[animation_object_count++] = action_effect_resources.templates[slot];
            for (uint32 slot = 0; slot < sizeof(character_pad.action_effects) / sizeof(character_pad.action_effects[0]); ++slot)
                animation_objects[animation_object_count++] = character_pad.action_effects[slot];
            for (uint32 grp_idx = 0; grp_idx < ModelGroupRegistry::Capacity; ++grp_idx)
            {
                for (uint32 slot = 0; slot < sizeof(model_grps.recs[grp_idx].models) / sizeof(model_grps.recs[grp_idx].models[0]); ++slot)
                    animation_objects[animation_object_count++] = model_grps.recs[grp_idx].models[slot];
            }
            for (uint32 slot = 0; slot < sizeof(actor_models.definitions) / sizeof(actor_models.definitions[0]); ++slot)
                animation_objects[animation_object_count++] = actor_models.definitions[slot].prototype;
            for (uint32 object_idx = 0; object_idx < animation_object_count; ++object_idx)
            {
                AnimatedObject *actor = animation_objects[object_idx];
                if (actor == 0 || actor == NULL)
                    continue;
                for (uint32 header_idx = 0; header_idx < animation_count; ++header_idx)
                {
                    if ((const void *)actor->animation == (data + src_animation_offsets[header_idx]))
                    {
                        actor->animation = animations + header_idx;
                        break;
                    }
                }
            }
            for (SceneNode *registered = registered_object_head; registered != 0; registered = registered->links.registry_next)
            {
                if (registered->kind != 0)
                    continue;
                AnimatedObject *actor = (AnimatedObject *)registered;
                for (uint32 header_idx = 0; header_idx < animation_count; ++header_idx)
                {
                    if ((const void *)actor->animation == (data + src_animation_offsets[header_idx]))
                    {
                        actor->animation = animations + header_idx;
                        break;
                    }
                }
            }
            for (uint32 model_idx = 0; model_idx <= CharacterController::ModelCount; ++model_idx)
            {
                CharacterModelRecord *model = model_idx < CharacterController::ModelCount ? character_pad.model_slots[model_idx] : character_pad.model;
                if (model == 0)
                    continue;
                for (uint32 slot = 0; slot < CharacterModelRecord::AnimationCount; ++slot)
                {
                    for (uint32 header_idx = 0; header_idx < animation_count; ++header_idx)
                    {
                        if ((const void *)model->animations[slot] == (data + src_animation_offsets[header_idx]))
                        {
                            model->animations[slot] = animations + header_idx;
                            break;
                        }
                    }
                }
            }
            for (uint32 model_idx = 0; model_idx < sizeof(actor_models.definitions) / sizeof(actor_models.definitions[0]); ++model_idx)
            {
                ActorModelDefinition *model = actor_models.definitions + model_idx;
                for (uint32 slot = 0; slot < sizeof(model->animations) / sizeof(model->animations[0]); ++slot)
                {
                    for (uint32 header_idx = 0; header_idx < animation_count; ++header_idx)
                    {
                        if ((const void *)model->animations[slot] == (data + src_animation_offsets[header_idx]))
                        {
                            model->animations[slot] = animations + header_idx;
                            break;
                        }
                    }
                }
            }
            for (uint32 model_idx = 0; model_idx < sizeof(animated_models.recs) / sizeof(animated_models.recs[0]); ++model_idx)
            {
                AnimatedModelDefinition *model = animated_models.recs + model_idx;
                for (uint32 slot = 0; slot < sizeof(model->animations) / sizeof(model->animations[0]); ++slot)
                {
                    for (uint32 header_idx = 0; header_idx < animation_count; ++header_idx)
                    {
                        if ((const void *)model->animations[slot] == (data + src_animation_offsets[header_idx]))
                        {
                            model->animations[slot] = animations + header_idx;
                            break;
                        }
                    }
                }
            }
        }
        free(animation_headers);
        free(src_animation_offsets);

        struct MaterialAnimationReference
        {
            uint32 **entries;
            uint32 offset;
        };

        uint32 material_reference_capacity = animated_object_count;
        material_reference_capacity += mesh_count + spr_count;
        MaterialAnimationReference *material_references = material_reference_capacity != 0 ? (MaterialAnimationReference *)malloc(material_reference_capacity * sizeof(MaterialAnimationReference)) : 0;
        GeometryRange *material_ranges = material_reference_capacity != 0 ? (GeometryRange *)malloc(material_reference_capacity * sizeof(GeometryRange)) : 0;
        uint32 material_reference_count = 0;
        for (uint32 owner_grp = 0; owner_grp < 3; ++owner_grp)
        {
            uint32 owner_count = owner_grp == 0 ? animated_object_count : (owner_grp == 1 ? mesh_count : spr_count);
            for (uint32 owner_idx = 0; owner_idx < owner_count; ++owner_idx)
            {
                uint32 **entries = owner_grp == 0 ? &animated_objects[owner_idx].material_animation_entries : (owner_grp == 1 ? &meshes[owner_idx].material_animation_entries : &sprs[owner_idx].animation_entries);
                if (*entries == 0)
                    continue;
                uint32 first = ((uintptr_t)(*entries) - src_begin);
                const uint8 *list_header = data + (first - sizeof(uint32));
                uint32 count = list_header[0];
                MaterialAnimationReference *reference = material_references + material_reference_count;
                reference->entries = entries;
                reference->offset = first;
                uint32 pos = material_reference_count;
                first -= sizeof(uint32);
                while (pos != 0 && first < material_ranges[pos - 1].first)
                {
                    material_ranges[pos] = material_ranges[pos - 1];
                    --pos;
                }
                material_ranges[pos].first = first;
                material_ranges[pos].last = first + (count + 1u) * sizeof(uint32);
                material_ranges[pos].dst = 0;
                ++material_reference_count;
            }
        }
        uint32 merged_material_ranges = 0;
        for (uint32 range_idx = 0; range_idx < material_reference_count; ++range_idx)
        {
            GeometryRange range = material_ranges[range_idx];
            if (merged_material_ranges != 0 && range.first <= material_ranges[merged_material_ranges - 1].last)
            {
                if (range.last > material_ranges[merged_material_ranges - 1].last)
                    material_ranges[merged_material_ranges - 1].last = range.last;
            }
            else
                material_ranges[merged_material_ranges++] = range;
        }
        uint32 material_bytes = 0;
        for (uint32 range_idx = 0; range_idx < merged_material_ranges; ++range_idx)
        {
            GeometryRange *range = material_ranges + range_idx;
            range->dst = material_bytes;
            material_bytes += range->last - range->first;
        }
        material_animation_word_count = material_bytes / sizeof(uint32);
        material_animation_words = material_bytes != 0 ? (uint32 *)malloc(material_bytes) : 0;
        for (uint32 range_idx = 0; range_idx < merged_material_ranges; ++range_idx)
        {
            const GeometryRange *range = material_ranges + range_idx;
            memcpy((uint8 *)material_animation_words + range->dst, (data + range->first), range->last - range->first);
        }
        for (uint32 reference_idx = 0; reference_idx < material_reference_count; ++reference_idx)
        {
            const MaterialAnimationReference *reference = material_references + reference_idx;
            for (uint32 range_idx = 0; range_idx < merged_material_ranges; ++range_idx)
            {
                const GeometryRange *range = material_ranges + range_idx;
                if (reference->offset >= range->first && reference->offset < range->last)
                {
                    *reference->entries = (uint32 *)((uint8 *)material_animation_words + range->dst + reference->offset - range->first);
                    break;
                }
            }
        }
        free(material_ranges);
        free(material_references);
        free(native_nodes);
        free(src_nodes);
        free(src_path_offsets);
        free(src_placement_offsets);
        free(src_actor_offsets);
    }
}
