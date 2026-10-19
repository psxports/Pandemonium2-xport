#include <stdio.h>
#include <stdlib.h>

#include "game.h"

SceneLoader scene_loader = {0};

void BeginSceneResourceLoad(SceneSectionBuffer *destination, uint32 asset)
{
    FUNCTION_MARKER(0x80069F18, "SLUS_005.78");
    scene_loader.destination_buffer = destination;
    scene_loader.asset = asset;
    StartSceneResourceLoad();
}

void StartSceneResourceLoad(void)
{
    FUNCTION_MARKER(0x8006997C, "SLUS_005.78");
    free(scene_loader.geometry_data);
    scene_loader.geometry_data = 0;
    scene_loader.geometry_bytes = 0;
    free(scene_loader.image_data);
    scene_loader.image_data = 0;
    scene_loader.image_bytes = 0;
    free(scene_loader.materials_data);
    scene_loader.materials_data = 0;
    scene_loader.materials_bytes = 0;
    uint32 asset;
    SceneSequenceBundle *bundle;
    scene_loader.sequence_bundle = 0;
    scene_loader.background = 0;
    if ((sint32)resource_archive.Size(scene_loader.asset) > 0)
    {
        asset = scene_loader.asset;
        BackgroundResource *background = scene_loader.destination_buffer->section.background;
        uint32 bytes = resource_archive.Size(asset);
        uint32 capacity;
        if (background == 0 || bytes < 40 || bytes > 0xFFFFF800u)
        {
            fprintf(stderr, "P2: Invalid scene background extent\n");
            abort();
        }
        free(background->data);
        free(background->tiles);
        background->data = NULL;
        background->tiles = NULL;
        background->bytes = 0;
        capacity = (bytes + 2047) & ~2047u;
        background->data = (uint8 *)malloc(capacity);
        if (background->data == NULL)
        {
            fprintf(stderr, "P2: Cannot allocate scene background\n");
            abort();
        }
        background->bytes = bytes;
        scene_loader.phase = 1;
        resource_archive.ReadAsset(asset, background->data, 0);
        scene_loader.background = background;
    }
    else if ((sint32)resource_archive.Size(scene_loader.asset + 2) > 0 && scene_loader.load_sequence != 0)
    {
        asset = scene_loader.asset;
        bundle = scene_loader.destination_buffer->section.sequence_bundle;
        uint32 bytes = resource_archive.Size(asset + 2);
        uint32 capacity;
        if ((bundle != &scene_stream.sequence_bundles[0] && bundle != &scene_stream.sequence_bundles[1]) || bytes > 0xFFFFF800u)
        {
            fprintf(stderr, "P2: Invalid scene sequence slot/extent\n");
            abort();
        }
        free(bundle->data);
        bundle->data = NULL;
        bundle->bytes = 0;
        capacity = (bytes + 2047) & ~2047u;
        bundle->data = (uint8 *)malloc(capacity != 0 ? capacity : 1);
        if (bundle->data == NULL)
        {
            fprintf(stderr, "P2: Cannot allocate scene sequence bundle\n");
            abort();
        }
        bundle->bytes = bytes;
        scene_loader.phase = 3;
        resource_archive.ReadAsset(asset + 2, bundle->data, 0);
        scene_loader.sequence_bundle = bundle;
    }
    else
    {
        asset = scene_loader.asset;
        scene_loader.phase = 5;
        scene_loader.materials_bytes = resource_archive.Size(scene_loader.asset + 5);
        if (scene_loader.materials_bytes < sizeof(EncodedMaterialResourceHeader) || scene_loader.materials_bytes > 0xFFFFF800u)
        {
            fprintf(stderr, "P2: Invalid scene material asset extent\n");
            abort();
        }
        uint32 material_capacity = (scene_loader.materials_bytes + 2047) & ~2047u;
        free(scene_loader.materials_data);
        scene_loader.materials_data = (uint8 *)malloc(material_capacity);
        if (scene_loader.materials_data == 0)
        {
            fprintf(stderr, "P2: Cannot allocate native scene material input\n");
            abort();
        }
        resource_archive.ReadAsset(scene_loader.asset + 5, scene_loader.materials_data, 0);
    }
}

uint32 PollSceneResourceLoad(void)
{
    uint32 asset;
    uint8 *base;
    uint32 palette;
    uint32 bytes;
    uint32 state;
    sint32 index;
    uint16 handle;
    SceneSection *section;
    SceneSequenceBundle *bundle;
    FUNCTION_MARKER(0x80069AC0, "SLUS_005.78");
    if (scene_loader.phase == 0)
        return 0;
    if (((uint32)resource_archive.Poll() << 24) == 0)
        return 1;
    state = (uint32)(sint32)scene_loader.phase;
    switch ((sint32)state)
    {
        case -4:
        case 4:
            section = scene_stream.active_section;
            index = section->section_index;
            asset = level_resource.asset_base + 9 * (uint32)index;
            if ((sint32)resource_archive.Offset(asset + 8) < 0 || scene_loader.load_sequence == 0)
            {
                scene_loader.phase = 0;
                return 1;
            }
            section = scene_stream.active_section;
            index = section->section_index;
            asset = level_resource.asset_base;
            bundle = section->sequence_bundle;
            if (bundle != &scene_stream.sequence_bundles[0] && bundle != &scene_stream.sequence_bundles[1])
            {
                fprintf(stderr, "P2: Invalid scene sequence bundle\n");
                abort();
            }
            asset += 9 * (uint32)index;
            handle = (uint16)LoadSequence(asset + 2, asset + 8, bundle->data, bundle->bytes, NULL, 0);
            sequence_playback.current_handle = handle;
            if (handle == 0)
                return 1;
            StartCurrentSequence();
            if (scene_loader.phase == -4)
            {
                index = scene_loader.pending_section;
                scene_loader.phase = 0;
                RequestSceneSection((uint32)index);
            }
            else
                scene_loader.phase = 0;
            return 1;
        case 1:
        {
            asset = scene_loader.asset;
            scene_loader.phase = 2;
            free(scene_loader.image_data);
            scene_loader.image_data = 0;
            scene_loader.image_bytes = resource_archive.Size(asset + 1);
            if (scene_loader.image_bytes > 0xFFFFF800u)
            {
                fprintf(stderr, "P2: Scene image exceeds native capacity\n");
                abort();
            }
            uint32 image_capacity = (scene_loader.image_bytes + 2047u) & ~2047u;
            scene_loader.image_data = (uint8 *)malloc(image_capacity != 0 ? image_capacity : 1);
            if (scene_loader.image_data == 0)
            {
                fprintf(stderr, "P2: Cannot allocate native scene image\n");
                abort();
            }
            resource_archive.ReadAsset(asset + 1, scene_loader.image_data, 0);
            return 1;
        }
        case 2:
            if ((sint32)resource_archive.Size(scene_loader.asset + 2) > 0 && scene_loader.load_sequence != 0)
            {
                asset = scene_loader.asset;
                bundle = scene_loader.destination_buffer->section.sequence_bundle;
                uint32 capacity;
                bytes = resource_archive.Size(asset + 2);
                if ((bundle != &scene_stream.sequence_bundles[0] && bundle != &scene_stream.sequence_bundles[1]) || bytes > 0xFFFFF800u)
                {
                    fprintf(stderr, "P2: Invalid scene sequence slot/extent\n");
                    abort();
                }
                free(bundle->data);
                bundle->data = NULL;
                bundle->bytes = 0;
                capacity = (bytes + 2047) & ~2047u;
                bundle->data = (uint8 *)malloc(capacity != 0 ? capacity : 1);
                if (bundle->data == NULL)
                {
                    fprintf(stderr, "P2: Cannot allocate scene sequence bundle\n");
                    abort();
                }
                bundle->bytes = bytes;
                scene_loader.phase = 3;
                resource_archive.ReadAsset(asset + 2, bundle->data, 0);
                index = scene_loader.texture_slot_base;
                scene_loader.sequence_bundle = bundle;
                {
                    uint32 image_index = (uint32)(index + 1);
                    uint32 image_bytes = (uint32)OriginalDefaults::TextureImageRects[image_index].w * (uint32)OriginalDefaults::TextureImageRects[image_index].h * 2;
                    if (image_index == 0)
                    {
                        uint32 second_image_bytes = resource_archive.Offset(252) + (uint32)OriginalDefaults::TextureImageRects[8].w * (uint32)OriginalDefaults::TextureImageRects[8].h * 2;
                        if (image_bytes < second_image_bytes)
                            image_bytes = second_image_bytes;
                    }
                    if (image_bytes > scene_loader.image_bytes)
                    {
                        fprintf(stderr, "P2: Truncated scene texture input\n");
                        abort();
                    }
                    UploadTextureImage(scene_loader.image_data, image_index, 0);
                }
                index = scene_loader.texture_slot_base;
                scene_loader.background = scene_loader.background->Relocate();
                return 1;
            }
            index = scene_loader.texture_slot_base;
            {
                uint32 image_index = (uint32)(index + 1);
                uint32 image_bytes = (uint32)OriginalDefaults::TextureImageRects[image_index].w * (uint32)OriginalDefaults::TextureImageRects[image_index].h * 2;
                if (image_index == 0)
                {
                    uint32 second_image_bytes = resource_archive.Offset(252) + (uint32)OriginalDefaults::TextureImageRects[8].w * (uint32)OriginalDefaults::TextureImageRects[8].h * 2;
                    if (image_bytes < second_image_bytes)
                        image_bytes = second_image_bytes;
                }
                if (image_bytes > scene_loader.image_bytes)
                {
                    fprintf(stderr, "P2: Truncated scene texture input\n");
                    abort();
                }
                UploadTextureImage(scene_loader.image_data, image_index, 0);
            }
            index = scene_loader.texture_slot_base;
            scene_loader.background = scene_loader.background->Relocate();
            asset = scene_loader.asset;
            scene_loader.phase = 5;
            goto load_model;
        case 3:
            asset = scene_loader.asset;
            scene_loader.phase = 5;
        load_model:
        {
            scene_loader.materials_bytes = resource_archive.Size(scene_loader.asset + 5);
            if (scene_loader.materials_bytes < sizeof(EncodedMaterialResourceHeader) || scene_loader.materials_bytes > 0xFFFFF800u)
            {
                fprintf(stderr, "P2: Invalid scene material asset extent\n");
                abort();
            }
            uint32 material_capacity = (scene_loader.materials_bytes + 2047) & ~2047u;
            free(scene_loader.materials_data);
            scene_loader.materials_data = (uint8 *)malloc(material_capacity);
            if (scene_loader.materials_data == 0)
            {
                fprintf(stderr, "P2: Cannot allocate native scene material input\n");
                abort();
            }
            resource_archive.ReadAsset(scene_loader.asset + 5, scene_loader.materials_data, 0);
        }
            return 1;
        case 5:
            if ((sint32)resource_archive.Size(scene_loader.asset + 6) > 0)
            {
                asset = scene_loader.asset;
                scene_loader.phase = 6;
                free(scene_loader.image_data);
                scene_loader.image_data = 0;
                scene_loader.image_bytes = resource_archive.Size(asset + 6);
                if (scene_loader.image_bytes > 0xFFFFF800u)
                {
                    fprintf(stderr, "P2: Scene image exceeds native capacity\n");
                    abort();
                }
                uint32 image_capacity = (scene_loader.image_bytes + 2047u) & ~2047u;
                scene_loader.image_data = (uint8 *)malloc(image_capacity != 0 ? image_capacity : 1);
                if (scene_loader.image_data == 0)
                {
                    fprintf(stderr, "P2: Cannot allocate native scene image\n");
                    abort();
                }
                resource_archive.ReadAsset(asset + 6, scene_loader.image_data, 0);
                return 1;
            }
            scene_loader.phase = 7;
            free(scene_loader.image_data);
            scene_loader.image_data = 0;
            scene_loader.image_bytes = 0;
            scene_loader.destination_buffer->section.materials.Relocate(scene_loader.materials_data, scene_loader.materials_bytes, 0);
            free(scene_loader.materials_data);
            scene_loader.materials_data = 0;
            scene_loader.materials_bytes = 0;
            asset = scene_loader.asset;
            {
                free(scene_loader.geometry_data);
                scene_loader.geometry_data = 0;
                scene_loader.geometry_bytes = resource_archive.Size(asset + 7);
                if (scene_loader.geometry_bytes < 188 || scene_loader.geometry_bytes > 0xFFFFF800u)
                {
                    fprintf(stderr, "P2: Invalid native scene geometry extent\n");
                    abort();
                }
                uint32 geometry_capacity = (scene_loader.geometry_bytes + 2047u) & ~2047u;
                scene_loader.geometry_data = (uint8 *)malloc(geometry_capacity);
                if (scene_loader.geometry_data == 0)
                {
                    fprintf(stderr, "P2: Cannot allocate native scene geometry input\n");
                    abort();
                }
                resource_archive.ReadAsset(asset + 7, scene_loader.geometry_data, 0);
            }
            return 1;
        case 6:
            asset = scene_loader.asset;
            scene_loader.phase = 7;
            if ((sint32)resource_archive.Size(asset) > 0)
            {
                index = scene_loader.texture_slot_base;
                palette = (uint32)index;
            }
            else
            {
                index = scene_loader.texture_slot_base;
                palette = (uint32)(index + 2);
            }
            {
                uint32 image_index = palette;
                uint32 image_bytes = (uint32)OriginalDefaults::TextureImageRects[image_index].w * (uint32)OriginalDefaults::TextureImageRects[image_index].h * 2;
                if (image_index == 0)
                {
                    uint32 second_image_bytes = resource_archive.Offset(252) + (uint32)OriginalDefaults::TextureImageRects[8].w * (uint32)OriginalDefaults::TextureImageRects[8].h * 2;
                    if (image_bytes < second_image_bytes)
                        image_bytes = second_image_bytes;
                }
                if (image_bytes > scene_loader.image_bytes)
                {
                    fprintf(stderr, "P2: Truncated scene texture input\n");
                    abort();
                }
                UploadTextureImage(scene_loader.image_data, image_index, 0);
            }
            index = scene_loader.texture_slot_base;
            scene_loader.destination_buffer->section.materials.Relocate(scene_loader.materials_data, scene_loader.materials_bytes, index);
            free(scene_loader.materials_data);
            scene_loader.materials_data = 0;
            scene_loader.materials_bytes = 0;
            return 1;
        case 7:
            if (scene_loader.image_data != 0)
            {
                asset = scene_loader.asset;
                free(scene_loader.image_data);
                scene_loader.image_data = 0;
                scene_loader.image_bytes = 0;
                {
                    free(scene_loader.geometry_data);
                    scene_loader.geometry_data = 0;
                    scene_loader.geometry_bytes = resource_archive.Size(asset + 7);
                    if (scene_loader.geometry_bytes < 188 || scene_loader.geometry_bytes > 0xFFFFF800u)
                    {
                        fprintf(stderr, "P2: Invalid native scene geometry extent\n");
                        abort();
                    }
                    uint32 geometry_capacity = (scene_loader.geometry_bytes + 2047u) & ~2047u;
                    scene_loader.geometry_data = (uint8 *)malloc(geometry_capacity);
                    if (scene_loader.geometry_data == 0)
                    {
                        fprintf(stderr, "P2: Cannot allocate native scene geometry input\n");
                        abort();
                    }
                    resource_archive.ReadAsset(asset + 7, scene_loader.geometry_data, 0);
                }
                return 1;
            }
            base = scene_loader.geometry_data;
            scene_loader.phase = 0;
            if (base == 0 || scene_loader.geometry_bytes < 188)
            {
                fprintf(stderr, "P2: Truncated scene section header\n");
                abort();
            }
            section = &scene_loader.destination_buffer->section;
            // Retire borrowed cutscene links only when their section is recycled
            for (uint32 trigger_index = 0; trigger_index < section->trigger_count; ++trigger_index)
            {
                if (cutscene.trigger == section->triggers + trigger_index)
                {
                    StopCutscene();
                    cutscene.trigger = 0;
                    cutscene.path = 0;
                    break;
                }
            }
            for (uint32 path_index = 0; path_index < section->path_count; ++path_index)
            {
                if (cutscene.path == section->paths + path_index)
                {
                    StopCutscene();
                    cutscene.trigger = 0;
                    cutscene.path = 0;
                    break;
                }
            }
            for (ActorBinding *binding = section->actor_bindings; binding != 0; binding = binding->next)
                free(binding->rotation_track);
            free(section->actor_bindings);
            free(section->animated_paths);
            free(section->lights);
            free(section->meshes);
            free(section->face_storage);
            free(section->vertex_storage);
            free(section->collision_storage);
            free(section->material_animation_words);
            free(section->animation_payload_data);
            if (section->animations != 0)
            {
                for (uint32 header_index = 0; header_index < section->animation_count; ++header_index)
                {
                    for (uint32 track_group = 0; track_group < 4; ++track_group)
                        free(section->animations[header_index].tracks[track_group].bytes);
                }
            }
            free(section->animations);
            free(section->billboards);
            free(section->mesh_parts);
            free(section->sphere_nodes);
            free(section->placements);
            free(section->animated_objects);
            free(section->sprites);
            for (uint32 track_index = 0; track_index < section->sprite_extent_track_count; ++track_index)
                free(section->sprite_extent_tracks[track_index]);
            free(section->sprite_extent_tracks);
            free(section->triggers);
            free(section->path_names);
            free(section->paths);
            free(section->path_key_data);
            free(section->path_offset_matrices);
            free(section->definition_transforms);
            section->definition_transforms = 0;
            section->path_offset_matrices = 0;
            section->lights = 0;
            section->light_count = 0;
            section->meshes = 0;
            section->mesh_count = 0;
            section->face_storage = 0;
            section->face_count = 0;
            section->vertex_storage = 0;
            section->vertex_count = 0;
            section->collision_storage = 0;
            section->collision_count = 0;
            section->material_animation_words = 0;
            section->material_animation_word_count = 0;
            section->animation_payload_data = 0;
            section->animation_payload_bytes = 0;
            section->animations = 0;
            section->animation_count = 0;
            section->billboards = 0;
            section->billboard_count = 0;
            section->mesh_parts = 0;
            section->mesh_part_count = 0;
            section->sphere_nodes = 0;
            section->sphere_node_count = 0;
            section->placements = 0;
            section->placement_count = 0;
            section->animated_objects = 0;
            section->animated_object_count = 0;
            section->sprites = 0;
            section->sprite_count = 0;
            section->sprite_extent_tracks = 0;
            section->sprite_extent_track_count = 0;
            section->triggers = 0;
            section->trigger_count = 0;
            section->path_names = 0;
            section->path_name_count = 0;
            section->paths = 0;
            section->path_count = 0;
            section->path_key_data = 0;
            section->actor_bindings = 0;
            section->animated_paths = 0;
            free(scene_loader.destination_buffer->resource_data);
            scene_loader.destination_buffer->resource_data = scene_loader.geometry_data;
            scene_loader.destination_buffer->resource_bytes = scene_loader.geometry_bytes;
            scene_loader.geometry_data = 0;
            scene_loader.geometry_bytes = 0;
            section->data = base + 188;
            section->data_bytes = scene_loader.destination_buffer->resource_bytes - 188;
            {
                const uint8 *header = base;
                uint32 i, encoded_link;
                // Decode the file header without copying native object layout
                if (xport_load_le32(header) != 0 || xport_load_le32(header + 4) != 0 || xport_load_le32(header + 100) != 0 || xport_load_le32(header + 104) != 0 || xport_load_le32(header + 108) != 0)
                {
                    fprintf(stderr, "P2: Scene header contains unresolved runtime list links\n");
                    abort();
                }
                section->links.registry_next = 0;
                section->links.sibling_next = 0;
                section->links.first_child = (encoded_link = xport_load_le32(header + 8)) != 0 ? (SceneNode *)((encoded_link) <= section->data_bytes && (sizeof(SceneNode)) <= section->data_bytes - (encoded_link) ? section->data + (encoded_link) : (fprintf(stderr, "P2: Scene file span exceeds native input\n"), abort(), section->data)) : 0;
                section->kind = header[12];
                section->flags = header[13];
                section->section_index = (sint8)header[14];
                section->palette_bank = (sint8)header[15];
                section->transition_scale = (sint16)xport_load_le16(header + 16);
                section->state_flags = xport_load_le16(header + 18);
                section->metadata = xport_load_le32(header + 24);
                for (i = 0; i < 16; ++i)
                {
                    uint32 offset = xport_load_le32(header + 28 + i * 4);
                    if (i == 10)
                    {
                        section->actor_bindings = offset != 0 ? (ActorBinding *)((offset) <= section->data_bytes && (sizeof(ActorBinding)) <= section->data_bytes - (offset) ? section->data + (offset) : (fprintf(stderr, "P2: Scene file span exceeds native input\n"), abort(), section->data)) : 0;
                        section->object_heads[i] = 0;
                    }
                    else if (i == 11)
                    {
                        section->animated_paths = offset != 0 ? (AnimatedPathState *)((offset) <= section->data_bytes && (sizeof(AnimatedPathState)) <= section->data_bytes - (offset) ? section->data + (offset) : (fprintf(stderr, "P2: Scene file span exceeds native input\n"), abort(), section->data)) : 0;
                        section->object_heads[i] = 0;
                    }
                    else
                        section->object_heads[i] = offset != 0 ? (SceneNode *)((offset) <= section->data_bytes && (sizeof(SceneNode)) <= section->data_bytes - (offset) ? section->data + (offset) : (fprintf(stderr, "P2: Scene file span exceeds native input\n"), abort(), section->data)) : 0;
                }
                section->collected_count = xport_load_le16(header + 96);
                section->metadata_halfword = xport_load_le16(header + 98);
                section->transient_links.registry_next = 0;
                section->transient_links.sibling_next = 0;
                section->transient_links.first_child = 0;
                for (i = 0; i < 3; ++i)
                    section->runtime_words[i] = xport_load_le32(header + 112 + i * 4);
                section->resource_identity = xport_load_le32(header + 132);
                section->background_top_color = xport_load_le32(header + 136);
                section->background_bottom_color = xport_load_le32(header + 140);
                for (i = 0; i < 3; ++i)
                    section->origin[i] = xport_load_le32(header + 144 + i * 4);
            }
            {
                uint32 lighting_offset = xport_load_le32((base + 92));
                uint32 payload_bytes = resource_archive.Size(scene_loader.asset + 7) - 188;
                uint32 lighting_bytes = 0;
                const uint8 *encoded_lighting = NULL;
                if (lighting_offset != 0)
                {
                    if (lighting_offset > payload_bytes || ((payload_bytes - lighting_offset) & 15) != 0)
                    {
                        fprintf(stderr, "P2: Invalid scene lighting cache extent\n");
                        abort();
                    }
                    lighting_bytes = payload_bytes - lighting_offset;
                    encoded_lighting = (const uint8 *)((lighting_offset) <= section->data_bytes && (lighting_bytes) <= section->data_bytes - (lighting_offset) ? section->data + (lighting_offset) : (fprintf(stderr, "P2: Scene file span exceeds native input\n"), abort(), section->data));
                }
                free(section->lighting_records);
                section->lighting_records = NULL;
                section->lighting_count = lighting_bytes / sizeof(MeshLighting);
                if (section->lighting_count != 0)
                {
                    section->lighting_records = (MeshLighting *)malloc(lighting_bytes);
                    if (section->lighting_records == NULL)
                    {
                        fprintf(stderr, "P2: Cannot allocate scene lighting cache\n");
                        abort();
                    }
                    for (uint32 i = 0; i < section->lighting_count; ++i)
                    {
                        MeshLighting *record = section->lighting_records + i;
                        const uint8 *encoded = encoded_lighting + i * 16;
                        for (uint32 color = 0; color < 3; ++color)
                            record->colors[color] = xport_load_le32(encoded + color * 4);
                        record->control = xport_load_le32(encoded + 12);
                    }
                }
            }
            section->Relocate(&section->materials);
            section->Activate();
            section->sequence_bundle = scene_loader.sequence_bundle;
            section->background = scene_loader.background;
            return 0;
        default:
            return 1;
    }
}

uint32 QueueSceneSection(uint32 scene)
{
    FUNCTION_MARKER(0x80069AAC, "SLUS_005.78");
    scene_loader.pending_section = scene;
    scene_loader.phase = -4;
    return 0xFFFFFFFC;
}

uint8 *GetSceneLoadDestination(void)
{
    FUNCTION_MARKER(0x80025BC0, "SLUS_005.78");
    return scene_stream.pending_buffer->section.data;
}

uint32 RequestSceneSection(uint32 scene)
{
    FUNCTION_MARKER(0x80025D3C, "SLUS_005.78");
    SceneStream *stream;
    SceneSectionBuffer *current, *pending;
    sint32 state;
    SceneSection *section;
    if ((game_flow.transition_flags & 2) != 0 && level_resource.active_sections != 0)
        return 0;
    stream = &scene_stream;
    current = stream->current_buffer;
    if (current != 0)
    {
        if (scene == 0xFFFFFFFF)
            scene = (uint32)(current->section.section_index + 1);
        if ((sint32)scene >= level_resource.section_count)
            return 0;
        if ((sint32)scene == current->section.section_index)
            goto check_pending;
    }
    pending = stream->pending_buffer;
    if (pending != 0 && (sint32)scene == pending->section.section_index)
    {
    check_pending:
        if (level_resource.active_sections != 0)
            return 0;
    }
    state = scene_loader.phase;
    if (state != 0)
    {
        if (state == 4)
            QueueSceneSection(scene);
        return 1;
    }
    {
        uint32 count = level_resource.active_sections;
        level_resource.active_sections = count + 1;
        if (count >= 2)
        {
            uint32 total;
            total = level_resource.collected_total;
            level_resource.collected_total = total + stream->pending_buffer->section.collected_count;
        }
        section = &stream->pending_buffer->section;
        section->section_index = scene;
        PrepareSceneSectionLoad();
    }
    return 1;
}

void PrepareSceneSectionLoad(void)
{
    FUNCTION_MARKER(0x800258A0, "SLUS_005.78");
    SceneSectionBuffer *pending_buffer = scene_stream.pending_buffer;
    SceneSection *section = &pending_buffer->section;
    sint32 index = section->section_index;
    uint32 assets = level_resource.asset_base + (uint32)index * 9;
    uint32 bytes;
    SceneSequenceBundle *sequence_bundle;
    SceneSectionBuffer *current;
    if (pending_buffer == &scene_stream.buffers[0])
        scene_loader.texture_slot_base = 2;
    else
        scene_loader.texture_slot_base = 5;
    section->section_index = index;
    section->data = 0;
    section->data_bytes = 0;
    section->state_flags = 0;
    section->links.sibling_next = 0;
    section->links.first_child = 0;
    section->background = scene_stream.background_slots[1];
    current = scene_stream.current_buffer;
    scene_loader.load_sequence = 1;
    if (current != 0 && level_resource.audio_mode == 0)
    {
        SceneSequenceBundle *current_slot = current->section.sequence_bundle;
        scene_loader.load_sequence = 0;
        scene_stream.sequence_slots[1] = current_slot;
    }
    sequence_bundle = scene_stream.sequence_slots[1];
    if (sequence_bundle != &scene_stream.sequence_bundles[0] && scene_loader.load_sequence != 0)
    {
        bytes = resource_archive.Size(assets + 2);
        if ((sint32)bytes > 0)
            scene_stream.sequence_slots[1] = &scene_stream.sequence_bundles[1];
        sequence_bundle = scene_stream.sequence_slots[1];
    }
    index = (uint8)section->section_index;
    scene_loader.loading_section = index;
    section->sequence_bundle = sequence_bundle;
    BeginSceneResourceLoad(pending_buffer, level_resource.asset_base + (uint32)(sint32)(sint8)index * 9);
}

uint32 CountSceneTreasure(void)
{
    FUNCTION_MARKER(0x80025620, "SLUS_005.78");
    SceneStream *stream = &scene_stream;
    const SceneSection *section = &stream->current_buffer->section;
    uint32 active_sections = level_resource.active_sections;
    uint32 result = section->collected_count;
    if (active_sections >= 2)
        result += stream->pending_buffer->section.collected_count;
    return result;
}

uint32 WaitSceneResourceLoad(void)
{
    FUNCTION_MARKER(0x80060F9C, "SLUS_005.78");
    uint32 result;
    while (PollSceneResourceLoad())
    {
    }
    result = game_flow.transition_flags | 0x2000;
    game_flow.transition_flags = result;
    return result;
}
