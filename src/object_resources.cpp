#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "game.h"

void SceneNode::RelocateClone(uint8 *destination, const uint8 *source)
{
    FUNCTION_MARKER(0x8003FF6C, "SLUS_005.78");
    SceneNode *child;
    uint32 *entries;
    child = links.first_child;
    if (child != 0)
    {
        links.first_child = (SceneNode *)(destination + ((const uint8 *)child - source));
    }
    child = links.sibling_next;
    if (child != 0)
    {
        links.sibling_next = (SceneNode *)(destination + ((const uint8 *)child - source));
    }
    if (kind == 7)
    {
        SpriteRecord *sprite = (SpriteRecord *)this;
        entries = sprite->animation_entries;
        if (entries != 0)
        {
            entries = (uint32 *)(destination + ((const uint8 *)entries - source));
            sprite->animation_entries = entries;
            ResetMaterialAnimationList(entries);
        }
    }
    child = links.first_child;
    while (child != 0)
    {
        child->RelocateClone(destination, source);
        child = child->links.sibling_next;
    }
}

AnimatedObject *CloneObjectResource(const AnimatedObject *source)
{
    FUNCTION_MARKER(0x8004002C, "SLUS_005.78");
    uint8 *destination;
    uint32 bytes;
    SceneNode *scene_node, *child_node, *reference;
    AnimatedObject *clone;
    SpriteRecord *clone_sprite;
    if (!source)
        return 0;
    if (source->clone_storage_bytes < sizeof(AnimatedObject))
    {
        fprintf(stderr, "P2: Clone source has no native storage span\n");
        abort();
    }
    destination = AllocateGameMemory(source->clone_bytes, source->clone_storage_bytes);
    if (!destination)
        return 0;
    bytes = source->clone_storage_bytes;
    CopyBytes(destination, (const uint8 *)source, bytes);
    clone = (AnimatedObject *)destination;
    if (clone->material_animation_entries != 0)
    {
        clone->material_animation_entries = (uint32 *)((uint8 *)clone + ((const uint8 *)source->material_animation_entries - (const uint8 *)source));
        ResetMaterialAnimationList(clone->material_animation_entries);
    }
    ObjectLinks *root_links = &clone->node.links;
    reference = root_links->first_child;
    root_links->registry_next = 0;
    clone->scale = 4096;
    if (reference != 0)
    {
        root_links->first_child = (SceneNode *)((uint8 *)clone + ((const uint8 *)reference - (const uint8 *)source));
    }
    clone_sprite = clone->sprite;
    if (clone_sprite != 0)
    {
        clone->sprite = (SpriteRecord *)((uint8 *)clone + ((const uint8 *)clone_sprite - (const uint8 *)source));
    }
    scene_node = root_links->first_child;
    root_links->sibling_next = 0;
    while (scene_node != 0)
    {
        reference = scene_node->links.first_child;
        if (reference != 0)
        {
            scene_node->links.first_child = (SceneNode *)((uint8 *)clone + ((const uint8 *)reference - (const uint8 *)source));
        }
        reference = scene_node->links.sibling_next;
        if (reference != 0)
        {
            scene_node->links.sibling_next = (SceneNode *)((uint8 *)clone + ((const uint8 *)reference - (const uint8 *)source));
        }
        if (scene_node->kind == 7)
        {
            SpriteRecord *sprite = (SpriteRecord *)scene_node;
            uint32 *entries = sprite->animation_entries;
            if (entries != 0)
            {
                sprite->animation_entries = (uint32 *)((uint8 *)clone + ((const uint8 *)entries - (const uint8 *)source));
                ResetMaterialAnimationList(sprite->animation_entries);
            }
        }
        child_node = scene_node->links.first_child;
        while (child_node != 0)
        {
            child_node->RelocateClone((uint8 *)clone, (const uint8 *)source);
            child_node = child_node->links.sibling_next;
        }
        scene_node = scene_node->links.sibling_next;
    }
    return clone;
}

void ReleaseSceneObject(SceneNode *object)
{
    FUNCTION_MARKER(0x80025EAC, "SLUS_005.78");
    SceneNode *node = (SceneNode *)(object);
    AnimatedObject *actor;
    uint32 flags;
    if (node->kind == 7)
    {
        ReleaseSprite((SpriteRecord *)object);
        return;
    }
    actor = (AnimatedObject *)node;
    if ((sint8)actor->metadata126[1] == 2 || actor->movement_mode >= 4)
        ((ActorPlacement *)((const PathDefinition *)((actor->path_definition))))->ReleaseBoundActor();
    if ((actor->flags & 4) == 0)
        FreeGameMemory(object);
    else
    {
        flags = (node->flags | 0x80) & 0xFC;
        node->flags = flags;
    }
}

void SceneNode::RelocateResourceLinks(const ModelDecodeLayout *layout, const MaterialResourceHeader *resource, const SpriteRecord *default_sprite)
{
    uint8 *base = layout->storage;
    const uint8 *file_data = layout->input;
    uint32 bytes = layout->input_bytes;
    uint32 offset;
    uint8 *pointer;
    uint32 remaining;
    uint32 value;
    uint32 index;
    uint32 kind;
    SceneNode *child;
    uint32 clone_prefix;
    FUNCTION_MARKER(0x800683DC, "SLUS_005.78");
    if (base == 0 || bytes < 232 || layout->storage_bytes < sizeof(SceneNode) || (uintptr_t)this < (uintptr_t)base || (uintptr_t)this - (uintptr_t)base > layout->storage_bytes - sizeof(SceneNode))
    {
        fprintf(stderr, "P2: Model child node exceeds native storage\n");
        abort();
    }
    const ModelStorageRecord *record = 0;
    uint32 relative = (uint32)((uintptr_t)this - (uintptr_t)base);
    for (uint32 record_index = 0; record_index < layout->record_count; ++record_index)
    {
        const ModelStorageRecord *candidate = layout->records + record_index;
        if (candidate->storage_offset == relative && candidate->kind < ModelStorageRoot)
        {
            record = candidate;
            break;
        }
    }
    clone_prefix = layout->source_clone_bytes;
    if (record == 0 || clone_prefix < 232 || clone_prefix > bytes || record->source_offset > clone_prefix || record->source_bytes < 16 || record->source_bytes > clone_prefix - record->source_offset)
    {
        fprintf(stderr, "P2: Model child has no bounded source record\n");
        abort();
    }
    const uint8 *input = file_data + record->source_offset;
    uint32 record_bytes = input[12] == 5 || input[12] == 7 ? 48 : (input[12] == 6 ? 36 : 16);
    if (record->source_bytes < record_bytes)
    {
        fprintf(stderr, "P2: Encoded model child exceeds resource\n");
        abort();
    }
    uint8 encoded[48];
    memcpy(encoded, input, record_bytes);
    this->kind = encoded[12];
    this->flags = encoded[13];
    this->state_flags = xport_load_le16(encoded + 14);
    this->links.source_registry_word = xport_load_le32(encoded);
    this->links.first_child = 0;
    this->links.sibling_next = 0;
    if (this->kind == 5)
    {
        MeshPartNode *part = (MeshPartNode *)this;
        for (uint32 component = 0; component < 3; ++component)
            part->position[component] = xport_load_le32(encoded + 16 + component * 4);
        part->animation_binding.word = xport_load_le32(encoded + 28);
        part->span.first_vertex = (sint16)xport_load_le16(encoded + 32);
        part->span.vertex_count = (sint16)xport_load_le16(encoded + 34);
        part->span.first_polygon = (sint16)xport_load_le16(encoded + 36);
        part->span.polygon_count = (sint16)xport_load_le16(encoded + 38);
        for (uint32 component = 0; component < 4; ++component)
            part->rotation[component] = (sint16)xport_load_le16(encoded + 40 + component * 2);
    }
    else if (this->kind == 6)
    {
        SphereNode *sphere = (SphereNode *)this;
        sphere->animation_binding.word = xport_load_le32(encoded + 16);
        for (uint32 component = 0; component < 3; ++component)
            sphere->position[component] = xport_load_le32(encoded + 20 + component * 4);
        sphere->radius = xport_load_le32(encoded + 32);
    }
    else if (this->kind == 7)
    {
        SpriteRecord *sprite = (SpriteRecord *)this;
        sprite->animation_entries = 0;
        sprite->extent_track = 0;
        sprite->animation_binding.word = xport_load_le32(encoded + 24);
        for (uint32 component = 0; component < 3; ++component)
            sprite->position[component] = xport_load_le32(encoded + 28 + component * 4);
        for (uint32 component = 0; component < 2; ++component)
        {
            sprite->extent[component] = (sint16)xport_load_le16(encoded + 40 + component * 2);
            sprite->metadata_halfwords[component] = xport_load_le16(encoded + 44 + component * 2);
        }
    }
    offset = xport_load_le32(encoded + 8);
    if (offset != 0)
    {
        if ((offset & 3) != 0 || offset > clone_prefix || 16 > clone_prefix - offset)
        {
            fprintf(stderr, "P2: Model node link exceeds clone prefix\n");
            abort();
        }
        uint32 mapping_index = 0;
        while (mapping_index < layout->record_count && !(layout->records[mapping_index].source_offset == offset && layout->records[mapping_index].kind < ModelStorageRoot))
            ++mapping_index;
        if (mapping_index == layout->record_count)
        {
            fprintf(stderr, "P2: Model reference has no native record\n");
            abort();
        }
        this->links.first_child = (SceneNode *)(layout->storage + layout->records[mapping_index].storage_offset);
    }
    offset = xport_load_le32(encoded + 4);
    if (offset != 0)
    {
        if ((offset & 3) != 0 || offset > clone_prefix || 16 > clone_prefix - offset)
        {
            fprintf(stderr, "P2: Model node link exceeds clone prefix\n");
            abort();
        }
        uint32 mapping_index = 0;
        while (mapping_index < layout->record_count && !(layout->records[mapping_index].source_offset == offset && layout->records[mapping_index].kind < ModelStorageRoot))
            ++mapping_index;
        if (mapping_index == layout->record_count)
        {
            fprintf(stderr, "P2: Model reference has no native record\n");
            abort();
        }
        this->links.sibling_next = (SceneNode *)(layout->storage + layout->records[mapping_index].storage_offset);
    }
    if (this->kind == 7)
    {
        if (record->storage_bytes < sizeof(SpriteRecord))
        {
            fprintf(stderr, "P2: Model sprite exceeds its resource buffer\n");
            abort();
        }
        SpriteRecord *sprite = (SpriteRecord *)this;
        if (sprite != default_sprite)
        {
            index = xport_load_le32(encoded + 16);
            sprite->texture = resource->textures + index;
        }
        offset = xport_load_le32(encoded + 20);
        if (offset != 0)
        {
            if ((offset & 3) != 0 || offset < sizeof(uint32) || offset > clone_prefix)
            {
                fprintf(stderr, "P2: Model sprite animation header exceeds clone prefix\n");
                abort();
            }
            uint32 mapping_index = 0;
            while (mapping_index < layout->record_count && !(layout->records[mapping_index].source_offset == offset - 4 && layout->records[mapping_index].kind == ModelStorageWordList))
                ++mapping_index;
            if (mapping_index == layout->record_count)
            {
                fprintf(stderr, "P2: Model reference has no native record\n");
                abort();
            }
            pointer = (uint8 *)(layout->storage + layout->records[mapping_index].storage_offset + 4);
            sprite->animation_entries = (uint32 *)pointer;
            remaining = *(pointer - 4);
            if (remaining > (clone_prefix - offset) / sizeof(uint32))
            {
                fprintf(stderr, "P2: Model sprite animation list exceeds clone prefix\n");
                abort();
            }
            if (remaining != 0)
            {
                do
                {
                    uint8 *entry = pointer;
                    value = xport_load_le32(entry);
                    if (value != 0xFFFFFFFF)
                    {
                        index = (value >> 1) & 0x1FF;
                        kind = resource->materials[index].kind;
                        index += material_registry.count;
                        if (kind == 0)
                            xport_store_le32(entry, 2 * index);
                        else if (kind == 2)
                            xport_store_le32(entry, 2 * index | (value & 0x3FC00) | (value & 0x3FC0000) | (value & 0x3C000000));
                    }
                    pointer += 4;
                    --remaining;
                } while (remaining != 0);
            }
        }
    }
    child = this->links.first_child;
    while (child != 0)
    {
        SceneNode *child_node = child;
        child_node->RelocateResourceLinks(layout, resource, default_sprite);
        child = child_node->links.sibling_next;
    }
}

void AnimatedObject::RelocateResourceMaterials(const MaterialResourceHeader *resource, const ModelDecodeLayout *layout, MeshFace **polygon_owner, MeshVertex **vertex_owner)
{
    const uint8 *input = layout->input;
    uint32 bytes = layout->input_bytes;
    uint8 *object = (uint8 *)this;
    SceneNode *child_node;
    SpriteRecord *sprite;
    uint8 *pointer;
    uint32 offset;
    uint32 remaining;
    uint32 index;
    uint32 kind;
    uint32 value;
    FUNCTION_MARKER(0x80068570, "SLUS_005.78");
    if (object != layout->storage || layout->storage_bytes < sizeof(AnimatedObject))
    {
        fprintf(stderr, "P2: Model root exceeds native storage\n");
        abort();
    }
    free(*polygon_owner);
    *polygon_owner = 0;
    free(*vertex_owner);
    *vertex_owner = 0;
    if (bytes < 232)
    {
        fprintf(stderr, "P2: Truncated encoded model root\n");
        abort();
    }
    uint8 encoded[232];
    memcpy(encoded, input, 232);
    this->node.kind = encoded[12];
    this->node.flags = encoded[13];
    this->node.state_flags = xport_load_le16(encoded + 14);
    this->flags = xport_load_le32(encoded + 28);
    this->metadata32 = xport_load_le32(encoded + 32);
    this->event_word = xport_load_le32(encoded + 108);
    this->behavior_word = xport_load_le32(encoded + 112);
    this->state = xport_load_le32(encoded + 116);
    this->animation_binding.word = xport_load_le32(encoded + 120);
    this->path_motion_scale = xport_load_le32(encoded + 144);
    this->clone_bytes = xport_load_le32(encoded + 224);
    this->clone_storage_bytes = layout->storage_bytes;
    this->path_flags = xport_load_le32(encoded + 228);
    this->lighting_index = xport_load_le16(encoded + 48);
    this->metadata58 = xport_load_le16(encoded + 58);
    this->sound_voice = xport_load_le16(encoded + 138);
    this->mesh_span.first_vertex = (sint16)xport_load_le16(encoded + 50);
    this->mesh_span.vertex_count = (sint16)xport_load_le16(encoded + 52);
    this->mesh_span.first_polygon = (sint16)xport_load_le16(encoded + 54);
    this->mesh_span.polygon_count = (sint16)xport_load_le16(encoded + 56);
    this->ticks = (sint16)xport_load_le16(encoded + 128);
    this->frame = (sint16)xport_load_le16(encoded + 130);
    this->path_frame = (sint16)xport_load_le16(encoded + 136);
    this->yaw = (sint16)xport_load_le16(encoded + 148);
    this->target_yaw = (sint16)xport_load_le16(encoded + 150);
    this->pitch = (sint16)xport_load_le16(encoded + 152);
    this->pitch_scale = (sint16)xport_load_le16(encoded + 154);
    for (uint32 component = 0; component < 3; ++component)
        this->position[component] = xport_load_le32(encoded + 16 + component * 4);
    for (uint32 component = 0; component < 3; ++component)
        this->world_position[component] = xport_load_le32(encoded + 60 + component * 4);
    for (uint32 component = 0; component < 3; ++component)
        this->metadata_words72[component] = xport_load_le32(encoded + 72 + component * 4);
    for (uint32 component = 0; component < 3; ++component)
        this->velocity[component] = xport_load_le32(encoded + 92 + component * 4);
    for (uint32 component = 0; component < 16; ++component)
        this->auxiliary_words[component] = xport_load_le32(encoded + 160 + component * 4);
    for (uint32 component = 0; component < 4; ++component)
        this->orientation[component] = (sint16)xport_load_le16(encoded + 84 + component * 2);
    this->movement_mode = (sint8)encoded[124];
    this->behavior = (sint8)encoded[125];
    this->metadata126[0] = encoded[126];
    this->metadata126[1] = encoded[127];
    this->unbound_animation_word = xport_load_le32(encoded + 104);
    this->animation = 0;
    uint32 definition_word = xport_load_le32(encoded + 132);
    uint32 mesh_word = xport_load_le32(encoded + 140);
    if (definition_word != 0 || mesh_word != 0)
    {
        fprintf(stderr, "P2: Model contains unresolved runtime path or mesh references\n");
        abort();
    }
    this->path_definition = 0;
    this->mesh_state = 0;
    this->vertices = 0;
    this->polygons = 0;
    this->material_animation_entries = 0;
    this->node.links.first_child = 0;
    this->node.links.sibling_next = 0;
    this->sprite = 0;
    if (clone_bytes < 232 || clone_bytes > bytes)
    {
        fprintf(stderr, "P2: Model root or clone prefix exceeds resource\n");
        abort();
    }
    offset = xport_load_le32(encoded + 40);
    if (offset != 0 && ((offset & 3) != 0 || offset > bytes || sizeof(EncodedMeshVertex) > bytes - offset))
    {
        fprintf(stderr, "P2: Model vertex data exceeds resource\n");
        abort();
    }
    if (offset != 0)
    {
        sint32 vertex_count = (sint16)(this->metadata32 >> 16);
        if (vertex_count > 0)
        {
            if ((uint32)vertex_count > (bytes - offset) / sizeof(EncodedMeshVertex))
            {
                fprintf(stderr, "P2: Model vertex span exceeds resource\n");
                abort();
            }
            *vertex_owner = (MeshVertex *)malloc((uint32)vertex_count * sizeof(MeshVertex));
            if (*vertex_owner == 0)
            {
                fprintf(stderr, "P2: Cannot allocate native model vertices\n");
                abort();
            }
            this->vertices = *vertex_owner;
            for (uint32 vertex_index = 0; vertex_index < (uint32)vertex_count; ++vertex_index)
            {
                const uint8 *vertex_input = input + offset + vertex_index * sizeof(EncodedMeshVertex);
                MeshVertex *vertex = *vertex_owner + vertex_index;
                vertex->position.vx = (sint16)xport_load_le16(vertex_input);
                vertex->position.vy = (sint16)xport_load_le16(vertex_input + 2);
                vertex->position.vz = (sint16)xport_load_le16(vertex_input + 4);
                vertex->position.pad = (sint16)xport_load_le16(vertex_input + 6);
                vertex->normal.vx = (sint16)xport_load_le16(vertex_input + 8);
                vertex->normal.vy = (sint16)xport_load_le16(vertex_input + 10);
                vertex->normal.vz = (sint16)xport_load_le16(vertex_input + 12);
                vertex->normal.pad = (sint16)xport_load_le16(vertex_input + 14);
            }
        }
        else
        {
            if (layout->legacy_data == 0)
            {
                fprintf(stderr, "P2: Shared vertices have no retained payload\n");
                abort();
            }
            this->vertices = (MeshVertex *)(layout->legacy_data + offset);
        }
    }
    offset = xport_load_le32(encoded + 36);
    if (offset != 0)
    {
        if ((offset & 3) != 0 || offset > bytes || sizeof(EncodedMeshFace) > bytes - offset)
        {
            fprintf(stderr, "P2: Model polygon data exceeds resource\n");
            abort();
        }
        if ((this->flags & 12) == 8)
        {
            if (layout->legacy_data == 0)
            {
                fprintf(stderr, "P2: Shared model has no retained payload\n");
                abort();
            }
            this->polygons = (MeshFace *)(layout->legacy_data + offset);
            p2_sdk_80082a18("Share\n");
        }
        else
        {
            remaining = (uint32)(sint16)this->metadata32;
            if (remaining == 0 || remaining > (bytes - offset) / sizeof(EncodedMeshFace))
            {
                fprintf(stderr, "P2: Model polygon span exceeds resource\n");
                abort();
            }
            *polygon_owner = (MeshFace *)malloc(remaining * sizeof(MeshFace));
            if (*polygon_owner == 0)
            {
                fprintf(stderr, "P2: Cannot allocate native model polygons\n");
                abort();
            }
            this->polygons = *polygon_owner;
            MeshFace *face = *polygon_owner;
            const uint8 *face_input = input + offset;
            do
            {
                index = xport_load_le32(face_input);
                if (index >= resource->texture_count)
                {
                    fprintf(stderr, "P2: Model polygon texture exceeds native catalog\n");
                    abort();
                }
                --remaining;
                face->texture = resource->textures + index;
                for (uint32 vertex = 0; vertex < 4; ++vertex)
                    face->vertex_indices[vertex] = xport_load_le16(face_input + 4 + vertex * 2);
                face->plane = xport_load_le32(face_input + 12);
                face->normal.words[0] = xport_load_le32(face_input + 16);
                face->normal.words[1] = xport_load_le32(face_input + 20);
                ++face;
                face_input += sizeof(EncodedMeshFace);
            } while (remaining != 0);
        }
    }
    this->node.links.source_registry_word = xport_load_le32(encoded);
    offset = xport_load_le32(encoded + 8);
    if (offset != 0)
    {
        if ((offset & 3) != 0 || offset > clone_bytes || 16 > clone_bytes - offset)
        {
            fprintf(stderr, "P2: Model node link exceeds clone prefix\n");
            abort();
        }
        uint32 mapping_index = 0;
        while (mapping_index < layout->record_count && !(layout->records[mapping_index].source_offset == offset && layout->records[mapping_index].kind < ModelStorageRoot))
            ++mapping_index;
        if (mapping_index == layout->record_count)
        {
            fprintf(stderr, "P2: Model reference has no native record\n");
            abort();
        }
        this->node.links.first_child = (SceneNode *)(layout->storage + layout->records[mapping_index].storage_offset);
    }
    offset = xport_load_le32(encoded + 4);
    if (offset != 0)
    {
        if ((offset & 3) != 0 || offset > clone_bytes || 16 > clone_bytes - offset)
        {
            fprintf(stderr, "P2: Model node link exceeds clone prefix\n");
            abort();
        }
        uint32 mapping_index = 0;
        while (mapping_index < layout->record_count && !(layout->records[mapping_index].source_offset == offset && layout->records[mapping_index].kind < ModelStorageRoot))
            ++mapping_index;
        if (mapping_index == layout->record_count)
        {
            fprintf(stderr, "P2: Model reference has no native record\n");
            abort();
        }
        this->node.links.sibling_next = (SceneNode *)(layout->storage + layout->records[mapping_index].storage_offset);
    }
    offset = xport_load_le32(encoded + 156);
    if (offset != 0 && ((offset & 3) != 0 || offset > clone_bytes || 48 > clone_bytes - offset))
    {
        fprintf(stderr, "P2: Model root sprite exceeds resource\n");
        abort();
    }
    if (offset != 0)
    {
        uint32 mapping_index = 0;
        while (mapping_index < layout->record_count && !(layout->records[mapping_index].source_offset == offset && layout->records[mapping_index].kind == 7))
            ++mapping_index;
        if (mapping_index == layout->record_count)
        {
            fprintf(stderr, "P2: Model reference has no native record\n");
            abort();
        }
        this->sprite = (SpriteRecord *)(layout->storage + layout->records[mapping_index].storage_offset);
    }
    child_node = this->node.links.first_child;
    while (child_node != 0)
    {
        child_node->RelocateResourceLinks(layout, resource, this->sprite);
        child_node = child_node->links.sibling_next;
    }
    sprite = this->sprite;
    if (sprite != 0)
        sprite->texture = resource_groups[0].texture;
    offset = xport_load_le32(encoded + 44);
    if (offset != 0)
    {
        if ((offset & 3) != 0 || offset < sizeof(uint32) || offset > clone_bytes)
        {
            fprintf(stderr, "P2: Model material animation header exceeds clone prefix\n");
            abort();
        }
        uint32 mapping_index = 0;
        while (mapping_index < layout->record_count && !(layout->records[mapping_index].source_offset == offset - 4 && layout->records[mapping_index].kind == ModelStorageWordList))
            ++mapping_index;
        if (mapping_index == layout->record_count)
        {
            fprintf(stderr, "P2: Model reference has no native record\n");
            abort();
        }
        pointer = (uint8 *)(layout->storage + layout->records[mapping_index].storage_offset + 4);
        this->material_animation_entries = (uint32 *)pointer;
        remaining = *(pointer - 4);
        if (remaining > (clone_bytes - offset) / sizeof(uint32))
        {
            fprintf(stderr, "P2: Model material animation list exceeds clone prefix\n");
            abort();
        }
        if (remaining != 0)
        {
            do
            {
                uint8 *entry = pointer;
                value = xport_load_le32(entry);
                if (value != 0xFFFFFFFF)
                {
                    index = (value >> 1) & 0x1FF;
                    kind = resource->materials[index].kind;
                    index += material_registry.count;
                    if (kind == 0)
                        xport_store_le32(entry, 2 * index);
                    else if (kind == 2)
                        xport_store_le32(entry, 2 * index | (value & 0x3FC00) | (value & 0x3FC0000) | (value & 0x3C000000));
                }
                pointer += 4;
                --remaining;
            } while (remaining != 0);
        }
    }
}
