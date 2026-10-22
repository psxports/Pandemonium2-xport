#include <stdio.h>
#include <stdlib.h>

#include "scene_loader.h"
#include "game.h"

SceneLoader scene_loader = {0};

uint32 SceneLoader::CountSceneTreasure(void)
{
    FUNCTION_MARKER(0x80025620, "SLUS_005.78");
    SceneStream *stream = &scene_stream;
    const SceneSection *section = &stream->cur_buffer->section;
    uint32 active_sections = level_resource.active_sections;
    uint32 result = section->collected_count;
    if (active_sections >= 2)
        result += stream->pending_buffer->section.collected_count;
    return result;
}

void SceneLoader::PrepareSceneSectionLoad(void)
{
    FUNCTION_MARKER(0x800258A0, "SLUS_005.78");
    SceneSectionBuffer *pending_buffer = scene_stream.pending_buffer;
    SceneSection *section = &pending_buffer->section;
    sint32 idx = section->section_idx;
    uint32 assets = level_resource.asset_base + (uint32)idx * 9;
    sint32 bytes;
    SceneSequenceBundle *sequence_bundle;
    SceneSectionBuffer *cur;
    if (pending_buffer == &scene_stream.buffers[0])
        scene_loader.tex_slot_base = 2;
    else
        scene_loader.tex_slot_base = 5;
    section->section_idx = idx;
    section->data = 0;
    section->data_bytes = 0;
    section->state_flags = 0;
    section->links.sibling_next = 0;
    section->links.first_child = 0;
    section->background = scene_stream.background_slots[1];
    cur = scene_stream.cur_buffer;
    scene_loader.load_sequence = 1;
    if (cur != 0 && level_resource.audio_mode == 0)
    {
        SceneSequenceBundle *cur_slot = cur->section.sequence_bundle;
        scene_loader.load_sequence = 0;
        scene_stream.sequence_slots[1] = cur_slot;
    }
    sequence_bundle = scene_stream.sequence_slots[1];
    if (sequence_bundle != &scene_stream.sequence_bundles[0] && scene_loader.load_sequence != 0)
    {
        bytes = resource_archive.Size(assets + 2);
        if (bytes > 0)
            scene_stream.sequence_slots[1] = &scene_stream.sequence_bundles[1];
        sequence_bundle = scene_stream.sequence_slots[1];
    }
    idx = (uint8)section->section_idx;
    scene_loader.loading_section = idx;
    section->sequence_bundle = sequence_bundle;
    SceneLoader::BeginSceneResourceLoad(pending_buffer, level_resource.asset_base + (uint32)(sint8)idx * 9);
}

uint8 *SceneLoader::GetSceneLoadDst(void)
{
    FUNCTION_MARKER(0x80025BC0, "SLUS_005.78");
    return scene_stream.pending_buffer->section.data;
}

uint32 SceneLoader::RequestSceneSection(uint32 scene)
{
    FUNCTION_MARKER(0x80025D3C, "SLUS_005.78");
    SceneStream *stream;
    SceneSectionBuffer *cur, *pending;
    sint32 state;
    SceneSection *section;
    if ((game_flow.transition_flags & 2) != 0 && level_resource.active_sections != 0)
        return 0;
    stream = &scene_stream;
    cur = stream->cur_buffer;
    if (cur != 0)
    {
        if (scene == 0xFFFFFFFF)
            scene = cur->section.section_idx + 1;
        if ((sint32)scene >= level_resource.section_count)
            return 0;
        if ((sint32)scene == cur->section.section_idx)
            goto check_pending;
    }
    pending = stream->pending_buffer;
    if (pending != 0 && (sint32)scene == pending->section.section_idx)
    {
    check_pending:
        if (level_resource.active_sections != 0)
            return 0;
    }
    state = scene_loader.phase;
    if (state != 0)
    {
        if (state == 4)
            SceneLoader::QueueSceneSection(scene);
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
        section->section_idx = scene;
        SceneLoader::PrepareSceneSectionLoad();
    }
    return 1;
}

void SceneLoader::WaitSceneResourceLoad(void)
{
    FUNCTION_MARKER(0x80060F9C, "SLUS_005.78");

    while (SceneLoader::PollSceneResourceLoad())
    {
    }
    game_flow.transition_flags = game_flow.transition_flags | 0x2000;
}

void SceneLoader::StartSceneResourceLoad(void)
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
        BackgroundResource *background = scene_loader.dst_buffer->section.background;
        uint32 bytes = resource_archive.Size(asset);
        uint32 capacity;
        free(background->data);
        free(background->tiles);
        background->data = NULL;
        background->tiles = NULL;
        background->bytes = 0;
        capacity = (bytes + 2047) & ~2047u;
        background->data = (uint8 *)malloc(capacity);
        background->bytes = bytes;
        scene_loader.phase = 1;
        resource_archive.ReadAsset(asset, background->data, 0);
        scene_loader.background = background;
    }
    else if ((sint32)resource_archive.Size(scene_loader.asset + 2) > 0 && scene_loader.load_sequence != 0)
    {
        asset = scene_loader.asset;
        bundle = scene_loader.dst_buffer->section.sequence_bundle;
        uint32 bytes = resource_archive.Size(asset + 2);
        uint32 capacity;
        free(bundle->data);
        bundle->data = NULL;
        bundle->bytes = 0;
        capacity = (bytes + 2047) & ~2047u;
        bundle->data = (uint8 *)malloc(capacity != 0 ? capacity : 1);
        bundle->bytes = bytes;
        scene_loader.phase = 3;
        resource_archive.ReadAsset(asset + 2, bundle->data, 0);
        scene_loader.sequence_bundle = bundle;
    }
    else
    {
        scene_loader.phase = 5;
        scene_loader.materials_bytes = resource_archive.Size(scene_loader.asset + 5);
        uint32 material_capacity = (scene_loader.materials_bytes + 2047) & ~2047u;
        free(scene_loader.materials_data);
        scene_loader.materials_data = (uint8 *)malloc(material_capacity);
        resource_archive.ReadAsset(scene_loader.asset + 5, scene_loader.materials_data, 0);
    }
}

void SceneLoader::QueueSceneSection(uint32 scene)
{
    FUNCTION_MARKER(0x80069AAC, "SLUS_005.78");
    scene_loader.pending_section = scene;
    scene_loader.phase = -4;
}

uint32 SceneLoader::PollSceneResourceLoad(void)
{
    uint32 asset;
    uint8 *base;
    uint32 pal;
    uint32 bytes;
    sint32 state;
    sint32 idx;
    uint16 handle;
    SceneSection *section;
    SceneSequenceBundle *bundle;
    FUNCTION_MARKER(0x80069AC0, "SLUS_005.78");
    if (scene_loader.phase == 0)
        return 0;
    if (!resource_archive.Poll())
        return 1;
    state = scene_loader.phase;
    switch (state)
    {
        case -4:
        case 4:
            section = scene_stream.active_section;
            idx = section->section_idx;
            asset = level_resource.asset_base + 9 * (uint32)idx;
            if ((sint32)resource_archive.Offset(asset + 8) < 0 || scene_loader.load_sequence == 0)
            {
                scene_loader.phase = 0;
                return 1;
            }
            section = scene_stream.active_section;
            idx = section->section_idx;
            asset = level_resource.asset_base;
            bundle = section->sequence_bundle;
            asset += 9 * (uint32)idx;
            handle = SequencePlayback::LoadSequence(asset + 2, asset + 8, bundle->data, bundle->bytes, NULL, 0);
            sequence_playback.cur_handle = handle;
            if (handle == 0)
                return 1;
            SequencePlayback::StartCurSequence();
            if (scene_loader.phase == -4)
            {
                idx = scene_loader.pending_section;
                scene_loader.phase = 0;
                SceneLoader::RequestSceneSection(idx);
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
            uint32 image_capacity = (scene_loader.image_bytes + 2047u) & ~2047u;
            scene_loader.image_data = (uint8 *)malloc(image_capacity != 0 ? image_capacity : 1);
            resource_archive.ReadAsset(asset + 1, scene_loader.image_data, 0);
            return 1;
        }
        case 2:
            if ((sint32)resource_archive.Size(scene_loader.asset + 2) > 0 && scene_loader.load_sequence != 0)
            {
                asset = scene_loader.asset;
                bundle = scene_loader.dst_buffer->section.sequence_bundle;
                uint32 capacity;
                bytes = resource_archive.Size(asset + 2);
                free(bundle->data);
                bundle->data = NULL;
                bundle->bytes = 0;
                capacity = (bytes + 2047) & ~2047u;
                bundle->data = (uint8 *)malloc(capacity != 0 ? capacity : 1);
                bundle->bytes = bytes;
                scene_loader.phase = 3;
                resource_archive.ReadAsset(asset + 2, bundle->data, 0);
                idx = scene_loader.tex_slot_base;
                scene_loader.sequence_bundle = bundle;
                {
                    uint32 image_idx = idx + 1;
                    uint32 image_bytes = (uint32)OriginalDefaults::tex_img_rects[image_idx].w * (uint32)OriginalDefaults::tex_img_rects[image_idx].h * 2;
                    if (image_idx == 0)
                    {
                        uint32 img1_extent = resource_archive.Offset(252) + (uint32)OriginalDefaults::tex_img_rects[8].w * (uint32)OriginalDefaults::tex_img_rects[8].h * 2;
                        if (image_bytes < img1_extent)
                            image_bytes = img1_extent;
                    }
                    ScreenRenderState::UploadTexImage(scene_loader.image_data, image_idx, 0);
                }

                scene_loader.background = scene_loader.background->Relocate();
                return 1;
            }
            idx = scene_loader.tex_slot_base;
            {
                uint32 image_idx = idx + 1;
                uint32 image_bytes = (uint32)OriginalDefaults::tex_img_rects[image_idx].w * (uint32)OriginalDefaults::tex_img_rects[image_idx].h * 2;
                if (image_idx == 0)
                {
                    uint32 img1_extent = resource_archive.Offset(252) + (uint32)OriginalDefaults::tex_img_rects[8].w * (uint32)OriginalDefaults::tex_img_rects[8].h * 2;
                    if (image_bytes < img1_extent)
                        image_bytes = img1_extent;
                }
                ScreenRenderState::UploadTexImage(scene_loader.image_data, image_idx, 0);
            }

            scene_loader.background = scene_loader.background->Relocate();

            scene_loader.phase = 5;
            goto load_model;
        case 3:

            scene_loader.phase = 5;
        load_model:
        {
            scene_loader.materials_bytes = resource_archive.Size(scene_loader.asset + 5);
            uint32 material_capacity = (scene_loader.materials_bytes + 2047) & ~2047u;
            free(scene_loader.materials_data);
            scene_loader.materials_data = (uint8 *)malloc(material_capacity);
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
                uint32 image_capacity = (scene_loader.image_bytes + 2047u) & ~2047u;
                scene_loader.image_data = (uint8 *)malloc(image_capacity != 0 ? image_capacity : 1);
                resource_archive.ReadAsset(asset + 6, scene_loader.image_data, 0);
                return 1;
            }
            scene_loader.phase = 7;
            free(scene_loader.image_data);
            scene_loader.image_data = 0;
            scene_loader.image_bytes = 0;
            scene_loader.dst_buffer->section.materials.Relocate(scene_loader.materials_data, scene_loader.materials_bytes, 0);
            free(scene_loader.materials_data);
            scene_loader.materials_data = 0;
            scene_loader.materials_bytes = 0;
            asset = scene_loader.asset;
            {
                free(scene_loader.geometry_data);
                scene_loader.geometry_data = 0;
                scene_loader.geometry_bytes = resource_archive.Size(asset + 7);
                uint32 geometry_capacity = (scene_loader.geometry_bytes + 2047u) & ~2047u;
                scene_loader.geometry_data = (uint8 *)malloc(geometry_capacity);
                resource_archive.ReadAsset(asset + 7, scene_loader.geometry_data, 0);
            }
            return 1;
        case 6:
            asset = scene_loader.asset;
            scene_loader.phase = 7;
            if ((sint32)resource_archive.Size(asset) > 0)
            {
                idx = scene_loader.tex_slot_base;
                pal = idx;
            }
            else
            {
                idx = scene_loader.tex_slot_base;
                pal = idx + 2;
            }
            {
                uint32 image_idx = pal;
                uint32 image_bytes = (uint32)OriginalDefaults::tex_img_rects[image_idx].w * (uint32)OriginalDefaults::tex_img_rects[image_idx].h * 2;
                if (image_idx == 0)
                {
                    uint32 img1_extent = resource_archive.Offset(252) + (uint32)OriginalDefaults::tex_img_rects[8].w * (uint32)OriginalDefaults::tex_img_rects[8].h * 2;
                    if (image_bytes < img1_extent)
                        image_bytes = img1_extent;
                }
                ScreenRenderState::UploadTexImage(scene_loader.image_data, image_idx, 0);
            }
            idx = scene_loader.tex_slot_base;
            scene_loader.dst_buffer->section.materials.Relocate(scene_loader.materials_data, scene_loader.materials_bytes, idx);
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
                    uint32 geometry_capacity = (scene_loader.geometry_bytes + 2047u) & ~2047u;
                    scene_loader.geometry_data = (uint8 *)malloc(geometry_capacity);
                    resource_archive.ReadAsset(asset + 7, scene_loader.geometry_data, 0);
                }
                return 1;
            }
            base = scene_loader.geometry_data;
            scene_loader.phase = 0;
            section = &scene_loader.dst_buffer->section;
            // Retire borrowed cutscene links only when their section is recycled
            for (uint32 trigger_idx = 0; trigger_idx < section->trigger_count; ++trigger_idx)
            {
                if (cutscene.trigger == section->triggers + trigger_idx)
                {
                    CutscenePlayback::StopCutscene();
                    cutscene.trigger = 0;
                    cutscene.path = 0;
                    break;
                }
            }
            for (uint32 path_idx = 0; path_idx < section->path_count; ++path_idx)
            {
                if (cutscene.path == section->paths + path_idx)
                {
                    CutscenePlayback::StopCutscene();
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
            free(section->col_storage);
            free(section->material_animation_words);
            free(section->animation_payload_data);
            if (section->animations != 0)
            {
                for (uint32 header_idx = 0; header_idx < section->animation_count; ++header_idx)
                {
                    for (uint32 track_grp = 0; track_grp < 4; ++track_grp)
                        free(section->animations[header_idx].tracks[track_grp].bytes);
                }
            }
            free(section->animations);
            free(section->billboards);
            free(section->mesh_parts);
            free(section->sphere_nodes);
            free(section->placements);
            free(section->animated_objects);
            free(section->sprs);
            for (uint32 track_idx = 0; track_idx < section->spr_extent_track_count; ++track_idx)
                free(section->spr_extent_tracks[track_idx]);
            free(section->spr_extent_tracks);
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
            section->col_storage = 0;
            section->col_count = 0;
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
            section->sprs = 0;
            section->spr_count = 0;
            section->spr_extent_tracks = 0;
            section->spr_extent_track_count = 0;
            section->triggers = 0;
            section->trigger_count = 0;
            section->path_names = 0;
            section->path_name_count = 0;
            section->paths = 0;
            section->path_count = 0;
            section->path_key_data = 0;
            section->actor_bindings = 0;
            section->actor_source_offset = 0;
            section->animated_paths = 0;
            free(scene_loader.dst_buffer->resource_data);
            scene_loader.dst_buffer->resource_data = scene_loader.geometry_data;
            scene_loader.dst_buffer->resource_bytes = scene_loader.geometry_bytes;
            scene_loader.geometry_data = 0;
            scene_loader.geometry_bytes = 0;
            section->data = base + 188;
            section->data_bytes = scene_loader.dst_buffer->resource_bytes - 188;
            {
                const EncodedSectionHeader *header = (const EncodedSectionHeader *)base;
                uint32 i, encoded_link;
                // Decode the file header without copying native object layout
                section->links.registry_next = 0;
                section->links.sibling_next = 0;
                section->links.first_child = (encoded_link = header->child_offset) != 0 ? (SceneNode *)(section->data + encoded_link) : 0;
                section->kind = header->kind;
                section->flags = header->flags;
                section->section_idx = header->section_idx;
                section->pal_bank = header->pal_bank;
                section->transition_scale = header->transition_scale;
                section->state_flags = header->state_flags;
                section->metadata = header->metadata;
                for (i = 0; i < 16; ++i)
                {
                    uint32 offset = header->object_offsets[i];
                    if (i == 10)
                    {
                        section->actor_source_offset = offset;
                        section->object_heads[i] = 0;
                    }
                    else if (i == 11)
                    {
                        section->animated_paths = offset != 0 ? (AnimatedPathState *)(section->data + offset) : 0;
                        section->object_heads[i] = 0;
                    }
                    else
                        section->object_heads[i] = offset != 0 ? (SceneNode *)(section->data + offset) : 0;
                }
                section->collected_count = header->collected_count;
                section->metadata_halfword = header->metadata_halfword;
                section->transient_links.registry_next = 0;
                section->transient_links.sibling_next = 0;
                section->transient_links.first_child = 0;
                for (i = 0; i < 4; ++i)
                    section->ambient[i] = header->ambient[i];
                section->resource_identity = header->resource_identity;
                section->background_top_color = header->background_top_color;
                section->background_bottom_color = header->background_bottom_color;
                section->far_distance = header->far_distance;
                section->fog_near = header->fog_near;
                section->fog_far = header->fog_far;
            }
            {
                uint32 lighting_offset = ((const EncodedSectionHeader *)base)->lighting_offset;
                uint32 payload_bytes = resource_archive.Size(scene_loader.asset + 7) - 188;
                uint32 lighting_bytes = 0;
                const uint8 *encoded_lighting = NULL;
                if (lighting_offset != 0)
                {
                    lighting_bytes = payload_bytes - lighting_offset;
                    encoded_lighting = section->data + lighting_offset;
                }
                free(section->lighting_recs);
                section->lighting_recs = NULL;
                section->lighting_count = lighting_bytes / sizeof(MeshLighting);
                if (section->lighting_count != 0)
                {
                    section->lighting_recs = (MeshLighting *)malloc(lighting_bytes);
                    for (uint32 i = 0; i < section->lighting_count; ++i)
                    {
                        MeshLighting *rec = section->lighting_recs + i;
                        const EncodedMeshLighting *encoded = (const EncodedMeshLighting *)encoded_lighting + i;
                        for (uint32 color = 0; color < 3; ++color)
                            rec->colors[color] = encoded->colors[color];
                        rec->control = encoded->control;
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

void SceneLoader::BeginSceneResourceLoad(SceneSectionBuffer *dst, uint32 asset)
{
    FUNCTION_MARKER(0x80069F18, "SLUS_005.78");
    scene_loader.dst_buffer = dst;
    scene_loader.asset = asset;
    SceneLoader::StartSceneResourceLoad();
}
