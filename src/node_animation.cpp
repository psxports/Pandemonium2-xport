#include <stdlib.h>

#include "game.h"

void AnimatedObject::SampleNodeAnimation(SceneNode *node)
{
    const AnimationHeader *header;
    uint32 temporary_vector[3];
    AnimationHeader *animation;
    uint32 kind;
    MeshPartNode *part = (MeshPartNode *)node;
    SpriteRecord *sprite = (SpriteRecord *)node;
    SphereNode *sphere_node = (SphereNode *)node;
    uint32 *position;
    sint16 *rotation;
    sint32 position_track;
    const sint8 *rotation_binding;
    const AnimationTrack *track;
    const uint8 *source;
    const uint8 *payload;
    uint32 weight;
    uint32 x;
    uint32 y;
    uint32 z;
    uint32 product;
    uint32 index;
    uint32 flags;
    uint32 active;
    MorphRestorationHeader *restoration;
    MorphRestorationEntry *entry;
    const MorphAnimationTrack *morph_track;
    uint32 slot_flags;
    uint32 remaining;
    uint32 points;
    MeshVertex *vertex;
    const SVECTOR *source_vertex;
    uint32 point;
    sint32 track_index;
    sint32 count;
    sint32 shifted;
    sint32 frame;
    sint32 node_key;
    FUNCTION_MARKER(0x80019CF4, "SLUS_005.78");
    animation = this->animation;
    if (animation == 0)
        goto finish;
    header = animation;
    kind = node->kind;
    frame = this->frame;
    if (kind == 5)
    {
        position_track = part->animation_binding.tracks[0];
        position = part->position;
    }
    else if (kind == 6)
    {
        position_track = (sint8)sphere_node->node.state_flags;
        position = sphere_node->position;
    }
    else if (kind == 7)
    {
        position_track = sprite->animation_binding.tracks[0];
        position = sprite->position;
    }
    else if (kind == 2)
    {
        SceneTrigger *trigger = (SceneTrigger *)node;
        position_track = trigger->animation_binding.tracks[0];
        position = trigger->position;
    }
    else
        goto finish;
    track_index = position_track;
    if (track_index >= 0)
    {
        track = &((VectorAnimationTrack *)(header->tracks[0].vectors))[track_index].track;
        payload = track->keys;
        if (payload != 0)
        {
            flags = this->flags;
            if ((flags & 0x10000) != 0)
            {
                weight = 1365 * ((uint32)frame + 1);
                source = payload == (const uint8 *)-1 ? (const uint8 *)track : payload + 4;
                for (index = 0; index < 3; ++index)
                {
                    x = xport_load_le32(source + 4 * index);
                    y = position[index];
                    product = (uint32)((sint64)(sint32)(x - y) * (sint32)weight);
                    position[index] = y + (uint32)((sint32)product >> 12);
                }
            }
            else if (payload == (const uint8 *)-1)
            {
                x = xport_load_le32(track);
                y = xport_load_le32((const uint8 *)track + 4);
                z = xport_load_le32((const uint8 *)track + 8);
                position[0] = x;
                position[1] = y;
                position[2] = z;
            }
            else
                SampleVectorTrack(frame, track, position);
        }
    }
    if (kind == 6 || kind == 7)
    {
        track_index = kind == 6 ? sphere_node->animation_binding.tracks[0] : sprite->animation_binding.tracks[2];
        if (track_index >= 0)
        {
            track = ((AnimationTrack *)(header->tracks[2].records)) + track_index;
            payload = track->keys;
            if (payload != 0)
            {
                if (payload == (const uint8 *)-1)
                {
                    x = xport_load_le32(track);
                    y = xport_load_le32((const uint8 *)track + 4);
                    z = xport_load_le32((const uint8 *)track + 8);
                    temporary_vector[0] = x;
                    temporary_vector[1] = y;
                    temporary_vector[2] = z;
                }
                else
                    SampleVectorTrack(frame, track, temporary_vector);
                if (kind == 6)
                    sphere_node->radius = temporary_vector[0];
                else
                {
                    x = temporary_vector[0];
                    sprite->extent[0] = (sint32)x >> 5;
                    y = temporary_vector[1];
                    sprite->extent[1] = (sint32)y >> 5;
                }
            }
        }
        goto finish;
    }
    if (kind == 5)
    {
        rotation_binding = part->animation_binding.tracks + 1;
        rotation = part->rotation;
    }
    else
    {
        SceneTrigger *trigger = (SceneTrigger *)node;
        rotation_binding = trigger->animation_binding.tracks + 1;
        rotation = trigger->rotation;
    }
    track_index = *rotation_binding;
    if (track_index >= 0)
    {
        track = ((AnimationTrack *)(header->tracks[1].records)) + track_index;
        payload = track->keys;
        if (payload != 0)
        {
            shifted = gameplay_input.rotation_track_shift;
            if (shifted != 0)
            {
                shifted += track_index;
                count = header->counts[1];
                if (count == 0)
                    xport_mips_break(7);
                track_index = count == 0 ? shifted : shifted % count;
                track = ((AnimationTrack *)(header->tracks[1].records)) + track_index;
            }
            flags = this->flags;
            if ((flags & 0x10000) != 0)
                BlendRotationTransition(frame, track, rotation);
            else if (track->keys == (const uint8 *)-1)
            {
                x = xport_load_le32(track);
                y = xport_load_le32((const uint8 *)track + 4);
                xport_store_le32(rotation, x);
                xport_store_le32(rotation + 2, y);
            }
            else
                SampleQuaternionTrack(frame, track, rotation);
        }
    }
    if (kind != 5 || (this->flags & 0x20) == 0)
        goto finish;
    restoration = this->morph_restoration;
    if (restoration == 0)
        goto finish;
    active = (uint32)(sint32)header->counts[3];
    node_key = part->animation_binding.tracks[0];
    if (active != 0)
    {
        morph_track = ((MorphAnimationTrack *)(header->tracks[3].morphs));
        for (;;)
        {
            if ((sint32)morph_track->track.metadata3 == node_key)
            {
                SampleMorphTrack(frame, morph_track, this, &part->span);
                break;
            }
            --active;
            ++morph_track;
            if (active == 0)
                break;
        }
    }
    if (active == 0 && restoration->active_count == 0)
        goto finish;
    remaining = (uint32)(sint32)restoration->count;
    if (remaining == 0)
        goto finish;
    entry = restoration->entries;
    for (;;)
    {
        if ((sint32)entry->node_key == node_key)
            break;
        --remaining;
        if (remaining == 0)
            goto finish;
        ++entry;
    }
    slot_flags = entry->flags;
    if (active != 0)
    {
        if ((slot_flags & 1) == 0)
        {
            entry->flags = (uint16)(slot_flags | 1);
            ++restoration->active_count;
        }
    }
    else if ((slot_flags & 1) != 0)
    {
        entry->flags = (uint16)(slot_flags & 0xFFFE);
        --restoration->active_count;
        point = (uint32)(sint32)part->span.first_vertex;
        points = (uint32)(sint32)part->span.vertex_count;
        vertex = (((MeshVertex *)(this->vertices))) + point;
        source_vertex = (const SVECTOR *)(entry->vertices);
        do
        {
            vertex->normal.vz = 0;
            vertex->normal.vy = 0;
            vertex->normal.vx = 0;
            x = xport_load_le32(source_vertex);
            y = xport_load_le32((const uint8 *)source_vertex + 4);
            xport_store_le32(&vertex->position, x);
            xport_store_le32((uint8 *)&vertex->position + 4, y);
            ++source_vertex;
            ++vertex;
            --points;
        } while (points != 0);
        this->RebuildMorphNormals(&part->span);
    }
finish:
    return;
}

uint32 AnimatedObject::ResetAnimatedModel()
{
    FUNCTION_MARKER(0x80043DE4, "SLUS_005.78");
    uint32 count = animated_models.count;
    uint32 kind = this->state;
    AnimatedModelDefinition *const *table = animated_models.definitions;
    const AnimatedModelDefinition *selected = (const AnimatedModelDefinition *)0;
    AnimatedPathState *path_state;
    AnimatedObject *other;
    SceneNode *child_reference;
    uint32 flags, index;
    const PathDefinition *definition;
    this->ticks = 1;
    while (count-- != 0)
    {
        const AnimatedModelDefinition *candidate = *table;
        ++table;
        if ((uint32)(sint32)candidate->kind_id == kind)
        {
            selected = candidate;
            break;
        }
    }
    path_state = (AnimatedPathState *)(((const ActorPlacement *)((const PathDefinition *)((this->path_definition))))->state != 0 ? ((const ActorPlacement *)((const PathDefinition *)((this->path_definition))))->state : 0);
    if ((sound_effects.IsPlaying(path_state->sound, (uint32)(sint32)path_state->sound_voice) & 255) != 0)
        StopSoundVoice((uint16)path_state->sound_voice);
    path_state->sound = 0;
    path_state->sound_voice = -1;
    flags = this->flags;
    this->movement_mode = 0;
    this->metadata126[1] = 0;
    this->metadata126[0] = 4;
    if ((flags & 0x8000) == 0)
    {
        uint32 byte_flags = (uint8)this->node.flags & 254;
        uint32 state = this->auxiliary_words[8];
        uint32 frame;
        flags = this->flags & 0xFFF3BEFF;
        this->node.flags = byte_flags;
        definition = (this->path_definition);
        this->node.flags = byte_flags | (state & 1);
        this->flags = flags;
        this->flags = flags | (state & 0x40000);
        frame = ((const AnimatedPathState *)(((const ActorPlacement *)((const PathDefinition *)(definition)))->state != 0 ? ((const ActorPlacement *)((const PathDefinition *)(definition)))->state : 0))->start_frame;
        definition = (this->path_definition);
        this->path_frame = frame;
        SampleVectorTrack((sint16)frame, &((const PathDefinition *)(definition))->positions, this->position);
    }
    flags = this->flags;
    this->auxiliary_words[15] = (uint32)-1;
    this->auxiliary_words[11] = (uint32)-1;
    this->auxiliary_words[7] = (uint32)-1;
    this->auxiliary_words[3] = (uint32)-1;
    this->yaw = 4096;
    this->velocity[2] = 0;
    this->velocity[1] = 0;
    this->velocity[0] = 0;
    this->flags = flags & 0xFE1F13FF;
    definition = (this->path_definition);
    if (definition != 0 && (((const PathDefinition *)(definition))->transform) != 0)
    {
        union
        {
            VECTOR vector;
            uint32 words[4];
        } input;

        const MATRIX *matrix;
        uint32 position[3];
        for (index = 0; index < 3; ++index)
            position[index] = this->position[index];
        for (index = 0; index < 3; ++index)
            input.words[index] = position[index];
        PushMatrix();
        matrix = (const MATRIX *)((((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) != 0 ? &(((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform)->matrix : (MATRIX *)0);
        for (index = 0; index < 5; ++index)
            xport_gte_write_control(index, xport_load_le32((const uint8 *)matrix + index * 4));
        ApplyRotMatrixLV(&input.vector, (VECTOR *)(this->auxiliary_words + 4));
        PopMatrix();
        for (index = 0; index < 3; ++index)
        {
            matrix = (const MATRIX *)((((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) != 0 ? &(((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform)->matrix : (MATRIX *)0);
            this->auxiliary_words[4 + index] = this->auxiliary_words[4 + index] + (uint32)matrix->t[index];
        }
    }
    else
    {
        uint32 position[3];
        for (index = 0; index < 3; ++index)
            position[index] = this->position[index];
        for (index = 0; index < 3; ++index)
            this->auxiliary_words[4 + index] = position[index];
    }
    {
        uint32 position[3];
        for (index = 0; index < 3; ++index)
            position[index] = this->auxiliary_words[4 + index];
        for (index = 0; index < 3; ++index)
            this->world_position[index] = position[index];
    }
    child_reference = this->node.links.first_child;
    other = child_reference != 0 ? (AnimatedObject *)child_reference : (AnimatedObject *)0;
    flags = other->flags;
    other->behavior = 0;
    other->path_motion_scale = 0;
    other->flags = flags | 0x2020020;
    return UpdateAnimatedModelOrientation(selected, this);
}

uint32 SceneNode::ResetPlayback(const SceneNode *parent)
{
    uint8 *bytes = (uint8 *)this;
    FUNCTION_MARKER(0x80013CEC, "SLUS_005.78");
    uint32 category = this->kind;
    uint32 flags, value;
    sint32 kind;
    if (category == 1)
    {
        MeshGeometryNode *mesh = (MeshGeometryNode *)this;
        ResetMaterialAnimationList(mesh->material_animation_entries);
        this->flags = this->flags | 0x80;
        return 0;
    }
    if (category == 0)
    {
        AnimatedObject *animated = (AnimatedObject *)this;
        ResetMaterialAnimationList(animated->material_animation_entries);
        kind = animated->behavior;
        if (kind == 22)
        {
            animated->ResetAnimatedModel();
            this->flags = this->flags & 0x7F;
            return 1;
        }
        if ((((uint32)kind - 3) & 0xFF) < 2 || (((uint32)kind - 31) & 0xFF) < 2)
        {
            animated->flags = animated->flags & 0xFFF03EFF;
            if (parent != 0 && (parent->flags & 1) != 0)
                this->flags = this->flags | 1;
            else
            {
                flags = this->flags;
                value = animated->auxiliary_words[8] & 1;
                flags &= 0xFE;
                this->flags = flags;
                this->flags = flags | value;
            }
            value = animated->auxiliary_words[8];
            flags = animated->flags;
            kind = animated->behavior;
            animated->flags = flags | (value & 0x40000);
            if (kind == 4)
                animated->frame = animated->auxiliary_words[9];
            else if (kind == 31)
                animated->frame = (uint16)(((AnimatedObject *)((animated->linked_object))))->frame;
            else
                animated->frame = 0;
            animated->AdvanceTracks(0);
            this->flags = this->flags & 0x7F;
            return 1;
        }
        goto clear_hidden;
    }
    if (category == 7)
    {
        SpriteRecord *sprite = (SpriteRecord *)this;
        ResetMaterialAnimationList(sprite->animation_entries);
        if (parent == 0)
        {
            if ((this->state_flags & 2) != 0)
            {
                this->flags = this->flags | 1;
                return 1;
            }
            AnimationTrack *track = sprite->extent_track;
            if (track != 0)
                track->frame = -1;
        }
        goto clear_hidden;
    }
    if (category == 8)
    {
        BillboardNode *billboard = (BillboardNode *)this;
        flags = billboard->playback_flags;
        billboard->playback_flags = flags & 0x7FFF;
        if ((flags & 0x7F) == 2 && GetSceneHealthPickupFlag(level_resource.level_id) != 0)
        {
            this->flags = this->flags | 0x80;
            return 0;
        }
        if ((billboard->playback_flags & 0x200) != 0)
            return 0;
        goto clear_hidden;
    }
    if (category == 9)
    {
        flags = ((const ActorPlacement *)this)->path.flags;
        if ((flags & 8) == 0)
            goto clear_hidden;
        if ((this->flags & 0x80) != 0)
            return 0;
        ((ActorPlacement *)this)->path.flags = flags & 0xFFFFFFFA;
        if ((flags & 0x20) != 0 || (parent != 0 && (parent->flags & 1) != 0))
        {
            flags = this->flags | 1;
            value = ((const ActorPlacement *)this)->path.flags | 1;
            this->flags = flags;
            ((ActorPlacement *)this)->path.flags = value;
        }
        else
            this->flags = this->flags & 0xFE;
        goto clear_hidden;
    }
    if (category == 2)
    {
        SceneTrigger *trigger = (SceneTrigger *)this;
        uint32 subtype = (this->state_flags >> 8);
        if (subtype - 40 < 16)
        {
            uint32 first = trigger->parameters.words[0];
            uint32 second = trigger->parameters.words[1];
            uint32 packed = ((uint32)this->kind | ((uint32)this->flags << 8) | ((uint32)this->state_flags << 16)) & 0xFF020000;
            trigger->parameters.words[0] = (first & 0xFFF) | ((subtype - 39) << 12) | (second & 0x1FE0000);
            if (packed == 0x32000000 && trigger->animation_binding.tracks[2] >= 0)
            {
                trigger->animation_owner = (const AnimatedObject *)parent;
                trigger->UpdateExtentsFromAnimation();
            }
        }
        else if ((sint8)subtype == 5)
        {
            trigger->parameters.words[1] = 0;
            trigger->parameters.words[0] = 0;
        }
        if ((sint8)(this->state_flags >> 8) == 50 && ((trigger->parameters.sound.playback.word >> 16) & 0x3FFF) == 0)
            return 0;
        goto clear_hidden;
    }
    if (category == 4)
    {
        flags = (uint8)this->state_flags;
        this->state_flags = (this->state_flags & 0xFF00) | (flags & 0xFB);
        if ((flags & 8) != 0 || (parent != 0 && (parent->flags & 1) != 0))
            this->flags = this->flags | 1;
        else
            this->flags = this->flags & 0xFE;
    }
clear_hidden:
    this->flags = this->flags & 0x7F;
    return 0;
}

void ApplyNodeAnimation(AnimatedObject *parent, SceneNode *node)
{
    AnimatedObject *animated_owner;
    const AnimationHeader *header;
    uint32 extent[3];
    MeshPartNode *part = (MeshPartNode *)node;
    SpriteRecord *sprite = (SpriteRecord *)node;
    const sint8 *position_binding;
    const sint8 *rotation_binding = 0;
    const sint8 *morph_binding = 0;
    uint32 *position;
    sint16 *rotation;
    const MorphSpan *morph_span;
    FUNCTION_MARKER(0x8001AECC, "SLUS_005.78");
    AnimatedObject *owner = parent;
    uint32 category = node->kind;
    AnimationHeader *animation;
    uint32 flags;
    AnimationTrack *track;
    MorphAnimationTrack *morph_track;
    sint32 index, frame;
    if (category == 0)
        owner = (AnimatedObject *)node;
    if (owner == 0)
        goto done;
    animated_owner = owner;
    animation = animated_owner->animation;
    if (animation == 0)
        goto done;
    header = animation;
    flags = animated_owner->flags;
    if ((flags & 0x20) != 0)
    {
        animated_owner->flags = flags & 0xFFFFFFDF;
        owner->SampleNodeAnimation(node);
        animated_owner->flags |= 0x20;
        goto done;
    }
    switch (category)
    {
        case 0:
            if (animated_owner->animation_binding.word == 0xFFFFFFFF)
                goto done;
            position_binding = animated_owner->animation_binding.tracks;
            position = animated_owner->position;
            rotation_binding = animated_owner->animation_binding.tracks + 1;
            morph_binding = animated_owner->animation_binding.tracks + 3;
            break;
        case 5:
            position_binding = part->animation_binding.tracks;
            position = part->position;
            rotation_binding = part->animation_binding.tracks + 1;
            morph_binding = part->animation_binding.tracks + 3;
            break;
        case 7:
            position_binding = sprite->animation_binding.tracks;
            position = sprite->position;
            break;
        case 8:
            position_binding = (const sint8 *)&node->state_flags;
            position = ((PositionedBillboardNode *)node)->position;
            break;
        case 2:
        {
            SceneTrigger *trigger = (SceneTrigger *)node;
            position_binding = trigger->animation_binding.tracks;
            position = trigger->position;
            rotation_binding = trigger->animation_binding.tracks + 1;
            break;
        }
        case 4:
            position_binding = (const sint8 *)&node->state_flags + 1;
            position = ((SceneLight *)node)->position;
            break;
        default:
            goto done;
    }
    index = *position_binding;
    if (index >= 0)
    {
        track = &((VectorAnimationTrack *)(header->tracks[0].vectors))[index].track;
        flags = track->flags;
        if ((flags & 8) != 0)
        {
            frame = track->frame;
            track->flags = flags & 0xF7;
            SampleVectorTrack(frame, track, position);
        }
    }
    if (category == 7)
    {
        index = sprite->animation_binding.tracks[2];
        if (index >= 0)
        {
            track = ((AnimationTrack *)(header->tracks[2].records)) + index;
            flags = track->flags;
            if ((flags & 8) != 0)
            {
                track->flags = flags & 0xF7;
                extent[0] = (uint32)sprite->extent[0] << 5;
                extent[1] = (uint32)sprite->extent[1] << 5;
                SampleVectorTrack(track->frame, track, extent);
                sprite->extent[0] = (sint32)extent[0] >> 5;
                sprite->extent[1] = (sint32)extent[1] >> 5;
            }
        }
        goto done;
    }
    if (rotation_binding != 0)
    {
        index = *rotation_binding;
        if (index >= 0)
        {
            track = ((AnimationTrack *)(header->tracks[1].records)) + index;
            flags = track->flags;
            if ((flags & 8) != 0)
            {
                frame = track->frame;
                track->flags = flags & 0xF7;
                if (category == 0)
                    rotation = animated_owner->orientation;
                else if (category == 5)
                    rotation = part->rotation;
                else
                    rotation = ((SceneTrigger *)node)->rotation;
                SampleQuaternionTrack(frame, track, rotation);
            }
        }
    }
    if (morph_binding != 0)
    {
        index = *morph_binding;
        if (index >= 0)
        {
            morph_track = ((MorphAnimationTrack *)(header->tracks[3].morphs)) + index;
            track = &morph_track->track;
            flags = track->flags;
            if ((flags & 8) != 0)
            {
                frame = track->frame;
                track->flags = flags & 0xF7;
                morph_span = category == 0 ? &animated_owner->mesh_span : &part->span;
                SampleMorphTrack(frame, morph_track, animated_owner, morph_span);
            }
        }
    }
done:
    return;
}

uint32 ApplyModelTravelRotation(SceneNode *object, uint32 distance)
{
    FUNCTION_MARKER(0x80041EDC, "SLUS_005.78");
    SceneNode *root = (SceneNode *)(object);
    SceneNode *node = root->links.first_child;
    uint32 changed = 0, factor, result;
    sint32 denominator, quotient, angle;

    while (node != 0)
    {
        if (node->kind == 6 && root->kind == 5)
        {
            denominator = (sint32)(uint32)(((sint64)(sint32)((const SphereNode *)node)->radius * 25735) >> 12);
            if (denominator == 0)
            {
                trap(7);
                abort();
            }
            quotient = 0x40000000 / denominator;
            factor = (uint32)(((sint64)(sint32)distance * quotient) >> 18);
            angle = (sint32)factor >> 1;
            fixed_math_result = factor;
            ((MeshPartNode *)root)->animation_binding.tracks[1] = -1;
            result = (uint32)SinQ14((sint16)(1024 - (uint32)angle));
            ((MeshPartNode *)root)->rotation[0] = (sint16)result;
            result = (uint32)SinQ14((sint16)angle);
            changed = 1;
            ((MeshPartNode *)root)->rotation[1] = (sint16)result;
            ((MeshPartNode *)root)->rotation[3] = 0;
            ((MeshPartNode *)root)->rotation[2] = 0;
        }
        result = ApplyModelTravelRotation(node, distance);
        node = node->links.sibling_next;
        changed |= result;
    }
    return (uint32)(sint32)(sint8)changed;
}

uint32 AnimatedObject::CapturePathVelocity()
{
    FUNCTION_MARKER(0x800414D0, "SLUS_005.78");
    AnimatedPathState *path_state = (AnimatedPathState *)(((const ActorPlacement *)((const PathDefinition *)((this->path_definition))))->state != 0 ? ((const ActorPlacement *)((const PathDefinition *)((this->path_definition))))->state : 0);

    union
    {
        VECTOR vector;
        uint32 words[4];
    } motion;

    uint32 x, y, z, sum, result;
    x = this->velocity[0];
    y = this->velocity[1];
    z = this->velocity[2];
    motion.words[0] = x;
    motion.words[1] = y;
    motion.words[2] = z;
    path_state->speed[1] = y;
    x = (uint32)((sint32)motion.words[0] >> 6);
    z = (uint32)((sint32)motion.words[2] >> 6);
    sum = x * x + z * z;
    motion.words[1] = 0;
    if ((sint32)sum < 0)
        result = SquareRoot12((sint32)(sum >> 2)) << 1;
    else
        result = SquareRoot12((sint32)sum);
    path_state->speed[0] = result;
    if ((this->flags & 0x2000) != 0)
    {
        x = motion.words[0];
        y = motion.words[1];
        z = motion.words[2];
        while (x + 26754 > 0xD103 || y + 26754 > 0xD103 || z + 26754 > 0xD103)
        {
            x = (uint32)((sint32)x >> 1);
            y = (uint32)((sint32)y >> 1);
            z = (uint32)((sint32)z >> 1);
            motion.words[0] = x;
            motion.words[1] = y;
            motion.words[2] = z;
        }
        VectorNormal(&motion.vector, &motion.vector);
        x = motion.words[0];
        y = motion.words[1];
        z = motion.words[2];
        path_state->direction[0] = x;
        path_state->direction[1] = y;
        path_state->direction[2] = z;
    }
    result = this->flags | 0x400;
    this->flags = result;
    return result;
}

uint32 AnimatedObject::UpdateAnimatedModelPathMotion()
{
    FUNCTION_MARKER(0x8004165C, "SLUS_005.78");
    AnimatedPathState *path_state = (AnimatedPathState *)(((const ActorPlacement *)((const PathDefinition *)((this->path_definition))))->state != 0 ? ((const ActorPlacement *)((const PathDefinition *)((this->path_definition))))->state : 0);
    uint32 *speed = path_state->speed;
    AnimatedModelDefinition *const *cursor = animated_models.definitions;
    uint32 count = animated_models.count;
    const AnimatedModelDefinition *definition = (const AnimatedModelDefinition *)0;
    AnimatedObject *owner;
    SceneNode *child_reference;
    uint32 kind = this->state, flags, angle, reverse, value;
    uint32 index, x, y, z, sum, length, scaled_length, relative_y, gravity, factor, result;
    sint32 heading, quotient;
    while (count != 0)
    {
        definition = *cursor;
        ++cursor;
        if ((uint32)(sint32)definition->kind_id == kind)
            break;
        --count;
    }
    if (count == 0)
        definition = (const AnimatedModelDefinition *)0;
    flags = this->flags;
    child_reference = this->node.links.first_child;
    owner = child_reference != 0 ? (AnimatedObject *)child_reference : (AnimatedObject *)0;
    if ((flags & 0x400000) != 0)
    {
        angle = (uint16)this->target_yaw;
        if ((flags & 0x10) != 0)
            angle += 2048;
        reverse = 0;
        heading = (sint16)Atan2Angle((sint32)path_state->direction[2], (sint32)path_state->direction[0]);
        if (heading != 4096)
            reverse = (((uint32)(sint32)(sint16)angle - (uint32)heading + 512) & 0xFFF) >= 1025;
        for (index = 0; index < 3; ++index)
        {
            value = (uint32)(((sint64)(sint32)path_state->direction[index] * (sint32)definition->collision_scale) >> 12);
            path_state->direction[index] = value;
        }
        x = path_state->direction[0];
        y = path_state->direction[1];
        z = path_state->direction[2];
        this->velocity[0] = x;
        this->velocity[1] = y;
        this->velocity[2] = z;
        if ((this->flags & 0x400) != 0)
        {
            this->CapturePathVelocity();
            speed[1] = speed[1] + definition->gravity;
        }
        else
        {
            {
                uint32 x = (uint32)((sint32)this->velocity[0] >> 6);
                uint32 y = (uint32)((sint32)this->velocity[1] >> 6);
                uint32 z = (uint32)((sint32)this->velocity[2] >> 6);
                uint32 sum = x * x + y * y + z * z;
                if ((sint32)sum < 0)
                    speed[0] = SquareRoot12((sint32)(sum >> 2)) << 1;
                else
                    speed[0] = SquareRoot12((sint32)sum);
            }
            speed[1] = 0;
        }
        if (reverse != 0)
        {
            speed[0] = 0 - speed[0];
            x = path_state->direction[0];
            y = path_state->direction[1];
            z = path_state->direction[2];
            path_state->direction[0] = 0 - x;
            path_state->direction[1] = 0 - y;
            path_state->direction[2] = 0 - z;
            {
                AnimatedPathState *path_state = (AnimatedPathState *)(((const ActorPlacement *)((const PathDefinition *)((this->path_definition))))->state != 0 ? ((const ActorPlacement *)((const PathDefinition *)((this->path_definition))))->state : 0);
                uint32 flags;
                const PathDefinition *parent;
                sint32 count, frame;
                path_state->speed[0] = 0 - path_state->speed[0];
                flags = this->flags;
                parent = (this->path_definition);
                this->flags = flags ^ 0x10;
                count = (sint16)((const PathDefinition *)(parent))->positions.metadata6;
                frame = count - this->path_frame;
                if (frame >= count)
                    frame = count - 1;
                else if (frame < 0)
                    frame = 0;
                this->path_frame = frame;
            }
            owner->pitch_scale = (sint16)(0 - (uint16)owner->pitch_scale);
        }
        if ((sint32)speed[0] < 256)
            speed[0] = 0;
        this->flags = (this->flags & 0xFFBFFFFF) | 0x1000000;
    }
    flags = this->flags;
    if ((flags & 0x400) != 0)
        speed[1] = speed[1] - definition->gravity;
    else
    {
        angle = (uint16)owner->pitch_scale;
        if ((flags & 0x4000) != 0)
        {
            this->flags = flags & 0xFFFFBFFF;
            {
                uint32 x = (uint32)((sint32)speed[0] >> 6);
                uint32 y = (uint32)((sint32)speed[1] >> 6);
                uint32 z = (uint32)((sint32)speed[2] >> 6);
                uint32 sum = x * x + y * y + z * z;
                if ((sint32)sum < 0)
                    length = SquareRoot12((sint32)(sum >> 2)) << 1;
                else
                    length = SquareRoot12((sint32)sum);
            }
            scaled_length = (uint32)(((sint64)(sint32)length * (sint32)definition->collision_scale) >> 12);
            if (scaled_length != 0)
            {
                heading = AsinAngle((sint32)speed[1], (sint32)length);
                value = (angle - (uint32)heading) & 0xFFF;
                x = (uint32)(((sint64)(SinQ14((sint16)(1024 - value)) >> 2) * (sint32)scaled_length) >> 12);
                this->velocity[0] = x;
                y = (uint32)(((sint64)(SinQ14((sint16)value) >> 2) * (sint32)scaled_length) >> 12);
                x = this->velocity[0];
                this->velocity[1] = y;
                this->velocity[2] = 0;
                speed[0] = x;
            }
            speed[1] = 0;
        }
        if ((sint32)speed[0] < 0)
        {
            {
                AnimatedPathState *path_state = (AnimatedPathState *)(((const ActorPlacement *)((const PathDefinition *)((this->path_definition))))->state != 0 ? ((const ActorPlacement *)((const PathDefinition *)((this->path_definition))))->state : 0);
                uint32 flags;
                const PathDefinition *parent;
                sint32 count, frame;
                path_state->speed[0] = 0 - path_state->speed[0];
                flags = this->flags;
                parent = (this->path_definition);
                this->flags = flags ^ 0x10;
                count = (sint16)((const PathDefinition *)(parent))->positions.metadata6;
                frame = count - this->path_frame;
                if (frame >= count)
                    frame = count - 1;
                else if (frame < 0)
                    frame = 0;
                this->path_frame = frame;
            }
            angle = 0 - angle;
        }
        x = this->velocity[0];
        y = this->velocity[1];
        z = this->velocity[2];
        relative_y = y - definition->gravity;
        reverse = 0;
        if ((definition->flags & 4) != 0)
        {
            x = (uint32)((sint32)this->velocity[0] >> 6);
            y = (uint32)((sint32)this->velocity[1] >> 6);
            z = (uint32)((sint32)this->velocity[2] >> 6);
            sum = x * x + y * y + z * z;
            if ((sint32)sum >= 89)
            {
                x = (uint32)((sint32)this->velocity[0] >> 6);
                y = (uint32)((sint32)relative_y >> 6);
                z = (uint32)((sint32)this->velocity[2] >> 6);
                sum = x * x + y * y + z * z;
                if ((sint32)sum < 0)
                    length = SquareRoot12((sint32)(sum >> 2)) << 1;
                else
                    length = SquareRoot12((sint32)sum);
                heading = (sint16)AsinAngle((sint32)relative_y, (sint32)length);
                reverse = (((uint32)heading - (uint32)(sint32)(sint16)angle) & 0xFFF) < 2048;
            }
        }
        if (reverse != 0)
            this->CapturePathVelocity();
        else if ((sint16)this->path_frame > 0 || (this->flags & 0x1000000) == 0)
        {
            gravity = definition->gravity;
            value = (uint32)(((sint64)(sint32)gravity * (SinQ14((sint16)angle) >> 2)) >> 12);
            value = speed[0] - value;
            speed[0] = value;
            speed[0] = (uint32)(((sint64)(sint32)definition->damping * (sint32)value) >> 12);
        }
    }
    if ((sint32)speed[0] < 0)
    {
        AnimatedPathState *path_state = (AnimatedPathState *)(((const ActorPlacement *)((const PathDefinition *)((this->path_definition))))->state != 0 ? ((const ActorPlacement *)((const PathDefinition *)((this->path_definition))))->state : 0);
        uint32 flags;
        const PathDefinition *parent;
        sint32 count, frame;
        path_state->speed[0] = 0 - path_state->speed[0];
        flags = this->flags;
        parent = (this->path_definition);
        this->flags = flags ^ 0x10;
        count = (sint16)((const PathDefinition *)(parent))->positions.metadata6;
        frame = count - this->path_frame;
        if (frame >= count)
            frame = count - 1;
        else if (frame < 0)
            frame = 0;
        this->path_frame = frame;
    }
    {
        uint32 x = (uint32)((sint32)speed[0] >> 6);
        uint32 y = (uint32)((sint32)speed[1] >> 6);
        uint32 z = (uint32)((sint32)speed[2] >> 6);
        uint32 sum = x * x + y * y + z * z;
        if ((sint32)sum < 0)
            length = SquareRoot12((sint32)(sum >> 2)) << 1;
        else
            length = SquareRoot12((sint32)sum);
    }
    value = definition->speed_limit;
    if ((sint32)value < (sint32)length)
    {
        if (length == 0)
        {
            trap(7);
            abort();
        }
        quotient = 0x40000000 / (sint32)length;
        factor = (uint32)(((sint64)(sint32)value * quotient) >> 18);
        fixed_math_result = factor;
        for (index = 0; index < 3; ++index)
            speed[index] = (uint32)(((sint64)(sint32)speed[index] * (sint32)factor) >> 12);
    }
    this->flags = this->flags | 0x40;
    this->ComputePathMotion((sint32)speed[0], (sint32)speed[1], (const VECTOR *)path_state->direction);
    result = this->flags & 0xFFFFFFBF;
    this->flags = result;
    return result;
}

uint32 AnimatedObject::UpdateAnimatedModelState()
{
    const AnimationHeader *active_animation;
    FUNCTION_MARKER(0x8004264C, "SLUS_005.78");
    uint32 vector[3], motion[3], floor[3], input[3], world_input[3];
    uint32 inverse_input[3], event_input[3], sample_input[3], final_input[3];
    MATRIX rotation, inverse_rotation, final_rotation;
    AnimatedModelDefinition *const *cursor;
    AnimatedPathState *path_state;
    uint32 *speed;
    AnimatedObject *owner;
    SceneNode *child_reference;
    uint32 count, kind_id, initial_kind;
    const AnimatedModelDefinition *definition = 0;
    uint32 animation_index;
    uint32 flags, mode, value, index, x, y, z, result;
    const PathDefinition *parent;
    const SoundBankStorage *sound_bank;
    const SoundSample *sound, *previous;
    SceneNode *node;
    uint32 length, option = 0, volume = 4096, base, factor, distance, changed;
    sint32 variant, frame, pitch, denominator, quotient;
    variant = (sint8)this->metadata126[1];
    if (variant != 0)
    {
        const ObjectCallback operation = object_operations.variants[variant];
        return (this->*operation.member_handler)();
    }
    initial_kind = (uint8)this->movement_mode;
    child_reference = this->node.links.first_child;
    owner = child_reference != 0 ? (AnimatedObject *)child_reference : (AnimatedObject *)0;
    path_state = (AnimatedPathState *)(((const ActorPlacement *)((const PathDefinition *)((this->path_definition))))->state != 0 ? ((const ActorPlacement *)((const PathDefinition *)((this->path_definition))))->state : 0);
    speed = path_state->speed;
    count = animated_models.count;
    cursor = animated_models.definitions;
    motion[2] = 0;
    motion[1] = 0;
    motion[0] = 0;
    kind_id = this->state;
    while (count != 0)
    {
        definition = *cursor;
        ++cursor;
        if ((uint32)(sint32)definition->kind_id == kind_id)
            break;
        --count;
    }
    if (count == 0)
        definition = 0;
    if ((sint8)this->movement_mode == 3)
    {
        if (this->path_motion_scale != 0)
            this->flags = this->flags & 0xFEFFFFFF;
        flags = this->flags;
        if ((flags & 0x4000) != 0)
            this->flags = flags & 0xFFFFFBFF;
        if ((sint32)this->path_motion_scale < 4096)
        {
            if ((this->flags & 0x800000) != 0)
            {
                mode = definition->flags >> 13;
                if (mode - 4 < 2)
                {
                    if (mode == 5)
                        this->ticks = -2;
                    this->path_motion_scale = 4096;
                }
                else
                    this->ticks = 0;
                this->flags = this->flags & 0xFF7FFFFF;
            }
            value = (uint32)((sint32)(3686 * this->path_motion_scale) >> 12);
            this->path_motion_scale = value;
            for (index = 0; index < 3; ++index)
            {
                value = (uint32)(((sint64)(sint32)this->velocity[index] * (sint32)this->path_motion_scale) >> 12);
                vector[index] = value;
            }
        }
        else
        {
            x = this->velocity[0];
            y = this->velocity[1];
            z = this->velocity[2];
            vector[0] = x;
            vector[1] = y;
            vector[2] = z;
        }
        x = vector[0];
        y = vector[1];
        z = vector[2];
        motion[0] = x;
        motion[1] = y;
        motion[2] = z;
        if ((this->flags & 0x2000) != 0)
        {
            frame = (sint16)this->ticks;
            if (frame != 0)
                this->ticks = frame - 1;
        }
        else
        {
            parent = (this->path_definition);
            if (parent != 0 && (((const PathDefinition *)(parent))->transform) != 0)
            {
                {
                    const MATRIX *matrix;
                    uint32 index;
                    for (index = 0; index < 9; ++index)
                    {
                        matrix = (const MATRIX *)((((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) != 0 ? &(((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform)->matrix : (MATRIX *)0);
                        rotation.m[index / 3][index % 3] = matrix->m[index % 3][index / 3];
                    }
                }
                x = vector[0];
                y = vector[1];
                z = vector[2];
                input[0] = x;
                input[1] = y;
                input[2] = z;
                {
                    uint32 index;
                    PushMatrix();
                    for (index = 0; index < 4; ++index)
                        xport_gte_write_control(index, xport_load_le32((const uint8 *)&rotation + index * 4));
                    xport_gte_write_control(4, (uint16)rotation.m[2][2]);
                    ApplyRotMatrixLV((const VECTOR *)input, (VECTOR *)vector);
                    PopMatrix();
                }
            }
        }
        for (index = 0; index < 3; ++index)
            this->position[index] = this->position[index] + vector[index];
        if ((sint32)this->path_motion_scale < 4096 && (this->flags & 0x2000) == 0)
        {
            {
                const PathDefinition *parent = (this->path_definition);
                const MATRIX *matrix;
                uint32 index, x, y, z;
                if (parent != 0 && (((const PathDefinition *)(parent))->transform) != 0)
                {
                    x = this->position[0];
                    y = this->position[1];
                    z = this->position[2];
                    world_input[0] = x;
                    world_input[1] = y;
                    world_input[2] = z;
                    PushMatrix();
                    matrix = (const MATRIX *)((((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) != 0 ? &(((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform)->matrix : (MATRIX *)0);
                    for (index = 0; index < 5; ++index)
                        xport_gte_write_control(index, xport_load_le32((const uint8 *)matrix + index * 4));
                    ApplyRotMatrixLV((const VECTOR *)world_input, (VECTOR *)vector);
                    PopMatrix();
                    for (index = 0; index < 3; ++index)
                    {
                        matrix = (const MATRIX *)((((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) != 0 ? &(((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform)->matrix : (MATRIX *)0);
                        vector[index] = vector[index] + (uint32)matrix->t[index];
                    }
                }
                else
                {
                    x = this->position[0];
                    y = this->position[1];
                    z = this->position[2];
                    vector[0] = x;
                    vector[1] = y;
                    vector[2] = z;
                }
            }
            frame = (sint16)((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->FindHeightFrame(vector, this->flags & 0x10, -61440);
            this->path_frame = frame;
            if (frame >= 0)
            {
                this->path_frame = ((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->ProjectFrame(frame, vector, floor, this->flags & 0x10, 4096);
                flags = this->flags;
                if ((flags & 0x400) != 0)
                {
                    if ((sint32)floor[1] >= (sint32)(vector[1] - 682))
                        this->flags = flags ^ 0x4400;
                    else
                        floor[1] = vector[1];
                }
                {
                    const PathDefinition *parent = (this->path_definition);
                    const MATRIX *matrix;
                    uint32 index, x, y, z;
                    if (parent != 0 && (((const PathDefinition *)(parent))->transform) != 0)
                    {
                        for (index = 0; index < 3; ++index)
                        {
                            matrix = (const MATRIX *)((((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) != 0 ? &(((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform)->matrix : (MATRIX *)0);
                            inverse_input[index] = floor[index] - (uint32)matrix->t[index];
                        }
                        {
                            const MATRIX *matrix;
                            uint32 index;
                            for (index = 0; index < 9; ++index)
                            {
                                matrix = (const MATRIX *)((((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) != 0 ? &(((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform)->matrix : (MATRIX *)0);
                                inverse_rotation.m[index / 3][index % 3] = matrix->m[index % 3][index / 3];
                            }
                        }
                        {
                            uint32 index;
                            PushMatrix();
                            for (index = 0; index < 4; ++index)
                                xport_gte_write_control(index, xport_load_le32((const uint8 *)&inverse_rotation + index * 4));
                            xport_gte_write_control(4, (uint16)inverse_rotation.m[2][2]);
                            ApplyRotMatrixLV((const VECTOR *)inverse_input, (VECTOR *)this->position);
                            PopMatrix();
                        }
                    }
                    else
                    {
                        x = floor[0];
                        y = floor[1];
                        z = floor[2];
                        this->position[0] = x;
                        this->position[1] = y;
                        this->position[2] = z;
                    }
                }
            }
            if ((sint32)speed[0] < 0)
            {
                AnimatedPathState *path_state = (AnimatedPathState *)(((const ActorPlacement *)((const PathDefinition *)((this->path_definition))))->state != 0 ? ((const ActorPlacement *)((const PathDefinition *)((this->path_definition))))->state : 0);
                uint32 flags;
                const PathDefinition *parent;
                sint32 count, frame;
                path_state->speed[0] = 0 - path_state->speed[0];
                flags = this->flags;
                parent = (this->path_definition);
                this->flags = flags ^ 0x10;
                count = (sint16)((const PathDefinition *)(parent))->positions.metadata6;
                frame = count - this->path_frame;
                if (frame >= count)
                    frame = count - 1;
                else if (frame < 0)
                    frame = 0;
                this->path_frame = frame;
            }
        }
        flags = this->flags;
        if ((flags & 0x800) != 0)
        {
            mode = definition->flags >> 13;
            this->flags = flags & 0xFFFFF7FF;
            switch (mode)
            {
                case 0:
                case 1:
                    if ((((const uint8 *)((const PathDefinition *)((this->path_definition))))[22] & 2) != 0)
                        break;
                    this->path_frame = 0;
                    this->flags = this->flags ^ 0x10;
                    speed[0] = 0;
                    if (mode == 1)
                    {
                        this->velocity[2] = 0;
                        this->velocity[0] = 0;
                    }
                    else
                    {
                        x = this->velocity[0];
                        y = this->velocity[1];
                        z = this->velocity[2];
                        this->velocity[0] = 0 - x;
                        this->velocity[1] = 0 - y;
                        this->velocity[2] = 0 - z;
                        if ((this->flags & 0x400) != 0)
                        {
                            this->velocity[1] = 0 - this->velocity[1];
                            {
                                const PathDefinition *parent = (this->path_definition);
                                const MATRIX *matrix;
                                uint32 index, x, y, z;
                                if (parent != 0 && (((const PathDefinition *)(parent))->transform) != 0)
                                {
                                    x = this->position[0];
                                    y = this->position[1];
                                    z = this->position[2];
                                    event_input[0] = x;
                                    event_input[1] = y;
                                    event_input[2] = z;
                                    PushMatrix();
                                    matrix = (const MATRIX *)((((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) != 0 ? &(((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform)->matrix : (MATRIX *)0);
                                    for (index = 0; index < 5; ++index)
                                        xport_gte_write_control(index, xport_load_le32((const uint8 *)matrix + index * 4));
                                    ApplyRotMatrixLV((const VECTOR *)event_input, (VECTOR *)vector);
                                    PopMatrix();
                                    for (index = 0; index < 3; ++index)
                                    {
                                        matrix = (const MATRIX *)((((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) != 0 ? &(((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform)->matrix : (MATRIX *)0);
                                        vector[index] = vector[index] + (uint32)matrix->t[index];
                                    }
                                }
                                else
                                {
                                    x = this->position[0];
                                    y = this->position[1];
                                    z = this->position[2];
                                    vector[0] = x;
                                    vector[1] = y;
                                    vector[2] = z;
                                }
                            }
                        }
                        {
                            uint32 velocity_length;
                            {
                                uint32 x = (uint32)((sint32)this->velocity[0] >> 6);
                                uint32 y = (uint32)((sint32)this->velocity[1] >> 6);
                                uint32 z = (uint32)((sint32)this->velocity[2] >> 6);
                                uint32 sum = x * x + y * y + z * z;
                                if ((sint32)sum < 0)
                                    velocity_length = SquareRoot12((sint32)(sum >> 2)) << 1;
                                else
                                    velocity_length = SquareRoot12((sint32)sum);
                            }
                            if (velocity_length != 0)
                            {
                                x = this->velocity[0];
                                y = this->velocity[1];
                                z = this->velocity[2];
                                path_state->direction[0] = x;
                                path_state->direction[1] = y;
                                path_state->direction[2] = z;
                                this->flags = this->flags | 0x400000;
                            }
                        }
                    }
                    {
                        const PathDefinition *parent = (this->path_definition);
                        const MATRIX *matrix;
                        uint32 index, x, y, z;
                        if (parent != 0 && (((const PathDefinition *)(parent))->transform) != 0)
                        {
                            x = this->position[0];
                            y = this->position[1];
                            z = this->position[2];
                            event_input[0] = x;
                            event_input[1] = y;
                            event_input[2] = z;
                            PushMatrix();
                            matrix = (const MATRIX *)((((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) != 0 ? &(((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform)->matrix : (MATRIX *)0);
                            for (index = 0; index < 5; ++index)
                                xport_gte_write_control(index, xport_load_le32((const uint8 *)matrix + index * 4));
                            ApplyRotMatrixLV((const VECTOR *)event_input, (VECTOR *)floor);
                            PopMatrix();
                            for (index = 0; index < 3; ++index)
                            {
                                matrix = (const MATRIX *)((((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) != 0 ? &(((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform)->matrix : (MATRIX *)0);
                                floor[index] = floor[index] + (uint32)matrix->t[index];
                            }
                        }
                        else
                        {
                            x = this->position[0];
                            y = this->position[1];
                            z = this->position[2];
                            floor[0] = x;
                            floor[1] = y;
                            floor[2] = z;
                        }
                    }
                    x = motion[0] - floor[0];
                    y = motion[1] - floor[1];
                    z = motion[2] - floor[2];
                    motion[0] = x;
                    motion[1] = y;
                    motion[2] = z;
                    parent = (this->path_definition);
                    frame = (sint16)this->path_frame;
                    if ((this->flags & 0x10) == 0)
                        frame = (sint16)((const PathDefinition *)(parent))->positions.metadata6 - frame - 1;
                    SampleVectorTrack(frame, &((const PathDefinition *)(parent))->positions, this->position);
                    {
                        const PathDefinition *parent = (this->path_definition);
                        const MATRIX *matrix;
                        uint32 index, x, y, z;
                        if (parent != 0 && (((const PathDefinition *)(parent))->transform) != 0)
                        {
                            x = this->position[0];
                            y = this->position[1];
                            z = this->position[2];
                            sample_input[0] = x;
                            sample_input[1] = y;
                            sample_input[2] = z;
                            PushMatrix();
                            matrix = (const MATRIX *)((((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) != 0 ? &(((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform)->matrix : (MATRIX *)0);
                            for (index = 0; index < 5; ++index)
                                xport_gte_write_control(index, xport_load_le32((const uint8 *)matrix + index * 4));
                            ApplyRotMatrixLV((const VECTOR *)sample_input, (VECTOR *)floor);
                            PopMatrix();
                            for (index = 0; index < 3; ++index)
                            {
                                matrix = (const MATRIX *)((((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) != 0 ? &(((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform)->matrix : (MATRIX *)0);
                                floor[index] = floor[index] + (uint32)matrix->t[index];
                            }
                        }
                        else
                        {
                            x = this->position[0];
                            y = this->position[1];
                            z = this->position[2];
                            floor[0] = x;
                            floor[1] = y;
                            floor[2] = z;
                        }
                    }
                    x = motion[0] + floor[0];
                    y = motion[1] + floor[1];
                    z = motion[2] + floor[2];
                    motion[0] = x;
                    motion[1] = y;
                    motion[2] = z;
                    if ((this->flags & 0x400) != 0)
                    {
                        floor[1] = vector[1];
                        {
                            const PathDefinition *parent = (this->path_definition);
                            const MATRIX *matrix;
                            uint32 index, x, y, z;
                            if (parent != 0 && (((const PathDefinition *)(parent))->transform) != 0)
                            {
                                for (index = 0; index < 3; ++index)
                                {
                                    matrix = (const MATRIX *)((((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) != 0 ? &(((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform)->matrix : (MATRIX *)0);
                                    final_input[index] = floor[index] - (uint32)matrix->t[index];
                                }
                                {
                                    const MATRIX *matrix;
                                    uint32 index;
                                    for (index = 0; index < 9; ++index)
                                    {
                                        matrix = (const MATRIX *)((((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) != 0 ? &(((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform)->matrix : (MATRIX *)0);
                                        final_rotation.m[index / 3][index % 3] = matrix->m[index % 3][index / 3];
                                    }
                                }
                                {
                                    uint32 index;
                                    PushMatrix();
                                    for (index = 0; index < 4; ++index)
                                        xport_gte_write_control(index, xport_load_le32((const uint8 *)&final_rotation + index * 4));
                                    xport_gte_write_control(4, (uint16)final_rotation.m[2][2]);
                                    ApplyRotMatrixLV((const VECTOR *)final_input, (VECTOR *)this->position);
                                    PopMatrix();
                                }
                            }
                            else
                            {
                                x = floor[0];
                                y = floor[1];
                                z = floor[2];
                                this->position[0] = x;
                                this->position[1] = y;
                                this->position[2] = z;
                            }
                        }
                    }
                    break;
                case 2:
                    this->ticks = 0;
                    break;
                case 3:
                {
                    const PathDefinition *parent = (this->path_definition);
                    const MATRIX *matrix;
                    uint32 index, x, y, z;
                    if (parent != 0 && (((const PathDefinition *)(parent))->transform) != 0)
                    {
                        x = this->position[0];
                        y = this->position[1];
                        z = this->position[2];
                        world_input[0] = x;
                        world_input[1] = y;
                        world_input[2] = z;
                        PushMatrix();
                        matrix = (const MATRIX *)((((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) != 0 ? &(((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform)->matrix : (MATRIX *)0);
                        for (index = 0; index < 5; ++index)
                            xport_gte_write_control(index, xport_load_le32((const uint8 *)matrix + index * 4));
                        ApplyRotMatrixLV((const VECTOR *)world_input, (VECTOR *)this->position);
                        PopMatrix();
                        for (index = 0; index < 3; ++index)
                        {
                            matrix = (const MATRIX *)((((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) != 0 ? &(((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform)->matrix : (MATRIX *)0);
                            this->position[index] = this->position[index] + (uint32)matrix->t[index];
                        }
                    }
                    else
                    {
                        x = this->position[0];
                        y = this->position[1];
                        z = this->position[2];
                        this->position[0] = x;
                        this->position[1] = y;
                        this->position[2] = z;
                    }
                }
                    this->flags = this->flags | 0x2000;
                    this->CapturePathVelocity();
                    this->ticks = 150;
                    break;
                case 4:
                case 5:
                    if ((sint16)this->ticks == 0)
                        goto expire;
                    flags = this->flags;
                    value = (uint8)this->node.flags;
                    this->ticks = -1;
                    this->movement_mode = 0;
                    result = flags | 0x4000;
                    this->flags = result;
                    this->node.flags = value | 1;
                    goto finish;
            }
        }
    }
    if ((sint16)this->ticks > 0)
        this->ApplyEvents();
    else
    {
    expire:
        if ((sound_effects.IsPlaying(path_state->sound, (uint32)(sint32)path_state->sound_voice) & 0xFF) != 0)
            StopSoundVoice((uint16)path_state->sound_voice);
        path_state->sound = 0;
        path_state->sound_voice = -1;
        frame = (sint16)this->ticks;
        value = (uint8)this->node.flags | 1;
        this->flags = this->flags | 0x4000;
        this->node.flags = value;
        if (frame != -2)
        {
            frame = (sint16)this->ticks;
            this->movement_mode = 0;
            if (frame != 0)
            {
                parent = (this->path_definition);
                this->ticks = 1;
                this->path_frame = 0;
                SampleVectorTrack(0, &((const PathDefinition *)(parent))->positions, this->position);
                result = this->flags | 0x8000;
                value = (uint8)this->node.flags & 0xFE;
                this->flags = result;
                this->node.flags = value;
            }
            else
            {
                result = 4;
                this->metadata126[0] = 0;
                this->metadata126[1] = 4;
            }
            goto finish;
        }
    }
    if ((this->flags & 0x200000) == 0)
    {
        flags = owner->flags;
        owner->behavior = 0;
        owner->flags = flags | 0x20020;
        motion[2] = 0;
        motion[1] = 0;
        motion[0] = 0;
        this->velocity[2] = 0;
        this->velocity[1] = 0;
        this->velocity[0] = 0;
        speed[2] = 0;
        speed[1] = 0;
        speed[0] = 0;
        owner->path_motion_scale = 0;
        flags = this->flags;
        this->yaw = 4096;
        if ((flags & 0x40000) != 0)
        {
            speed[0] = (definition->initial_speed);
            this->movement_mode = 3;
        }
        else
            this->movement_mode = 0;
        initial_kind = 3;
        this->flags = this->flags | 0x200010;
    }
    animation_index = (sint8)this->movement_mode == 3 ? 1 : 0;
    if (owner->animation != (definition->animations[animation_index]))
    {
        if ((sound_effects.IsPlaying(path_state->sound, (uint32)(sint32)path_state->sound_voice) & 0xFF) != 0)
            StopSoundVoice((uint16)path_state->sound_voice);
        path_state->sound = 0;
        path_state->sound_voice = -1;
        flags = owner->flags;
        owner->animation = (definition->animations[animation_index]);
        owner->frame = (sint16)(0xFFFF);
        owner->flags = flags | 0x10000;
    }
    if (initial_kind == 3 && (this->flags & 0x2000) == 0)
        UpdateAnimatedModelOrientation(definition, this);
    value = (uint8)this->node.flags;
    result = value | 0x40;
    if ((value & 1) != 0)
        goto finish;
    this->node.flags = result;
    active_animation = owner->animation;
    if (active_animation != 0)
    {
        frame = owner->frame;
        frame = frame < active_animation->frame_count - 1 ? frame + 1 : 0;
        owner->frame = (sint16)(frame);
        owner->ClampAnimationFrame();
    }
    if ((this->flags & 0x400) != 0)
        motion[1] = 0;
    {
        uint32 x = (uint32)((sint32)motion[0] >> 6);
        uint32 y = (uint32)((sint32)motion[1] >> 6);
        uint32 z = (uint32)((sint32)motion[2] >> 6);
        uint32 sum = x * x + y * y + z * z;
        if ((sint32)sum < 0)
            length = SquareRoot12((sint32)(sum >> 2)) << 1;
        else
            length = SquareRoot12((sint32)sum);
    }
    vector[0] = length;
    if (length == 0)
    {
        sound_bank = definition->sound_table;
        sound = sound_bank != 0 && (sint32)sound_bank->header.count >= 1 ? sound_bank->samples + 0 : 0;
        pitch = ((const SoundSample *)(sound))->pitch;
        if (this->state == 4)
            option = 1;
    }
    else
    {
        if ((this->flags & 0x10) != 0)
            vector[0] = 0 - length;
        owner->path_motion_scale = owner->path_motion_scale + vector[0];
        owner->flags = owner->flags | 1;
        owner->node.flags = owner->node.flags | 0x40;
        sound_bank = definition->sound_table;
        sound = sound_bank != 0 && (sint32)sound_bank->header.count >= 2 ? sound_bank->samples + 1 : 0;
        if (this->state - 30 < 2)
            option = 1;
        base = (uint32)((const SoundSample *)(sound))->pitch << 12;
        value = ApproxVectorLength(motion[0], 0, motion[2]);
        length = (definition->speed_limit);
        if ((sint32)value < (sint32)length)
        {
            if (length == 0)
            {
                trap(7);
                abort();
            }
            quotient = 0x40000000 / (sint32)length;
            factor = (uint32)(((sint64)(sint32)value * quotient) >> 18);
            fixed_math_result = factor;
            base = (uint32)(((sint64)(sint32)base * 3072) >> 12);
            pitch = (sint32)(base + (uint32)(((sint64)(sint32)factor * (sint32)base) >> 12)) >> 12;
            volume = (uint32)((sint32)(factor * 3072) >> 12) + 1024;
        }
        else
            pitch = (sint32)(uint32)(((sint64)(sint32)base * 6144) >> 12) >> 12;
    }
    result = owner->flags & 0x2000000;
    if (result != 0)
    {
        node = owner->node.links.first_child;
        distance = owner->path_motion_scale;
        changed = 0;
        while (node != 0)
        {
            if (node->kind == 6 && owner->node.kind == 5)
            {
                denominator = (sint32)(uint32)(((sint64)25735 * (sint32)((const SphereNode *)node)->radius) >> 12);
                if (denominator == 0)
                {
                    trap(7);
                    abort();
                }
                quotient = 0x40000000 / denominator;
                factor = (uint32)(((sint64)(sint32)distance * quotient) >> 18);
                frame = (sint32)factor >> 1;
                fixed_math_result = factor;
                ((MeshPartNode *)owner)->animation_binding.tracks[1] = -1;
                ((MeshPartNode *)owner)->rotation[0] = (sint16)(SinQ14((sint16)(1024 - (uint32)frame)));
                ((MeshPartNode *)owner)->rotation[1] = (sint16)(SinQ14((sint16)frame));
                changed = 1;
                ((MeshPartNode *)owner)->rotation[3] = (sint16)(0);
                ((MeshPartNode *)owner)->rotation[2] = (sint16)(0);
            }
            value = ApplyModelTravelRotation(node, distance);
            node = node->links.sibling_next;
            changed |= value;
        }
        result = changed << 24;
        if (result == 0)
        {
            result = owner->flags & 0xFDFFFFFF;
            owner->flags = result;
        }
    }
    if (sound != 0)
    {
        previous = path_state->sound;
        if (sound != previous && previous != 0)
        {
            if ((sound_effects.IsPlaying(previous, (uint32)(sint32)path_state->sound_voice) & 0xFF) != 0)
                StopSoundVoice((uint16)path_state->sound_voice);
            path_state->sound = 0;
            path_state->sound_voice = -1;
        }
        path_state->sound = sound;
        result = PlaySpatialSound(sound, (sint32)(volume + 4096), 0, (uint32)pitch, this->world_position, (uint32)definition->sound_radius << 5, (uint32)(sint32)path_state->sound_voice, 1, option);
        path_state->sound_voice = result;
    }
finish:
    return result;
}

uint32 AnimatedObject::HideAnimatedModel()
{
    FUNCTION_MARKER(0x80044230, "SLUS_005.78");
    SceneNode *parent = owner_node;
    uint32 result;
    parent->flags = parent->flags | 0x80;
    result = this->node.flags | 0x80;
    this->node.flags = result;
    return result;
}

uint32 AnimatedObject::UpdateSceneAnimation()
{
    FUNCTION_MARKER(0x8006117C, "SLUS_005.78");
    uint32 flags, result;
    sint32 step;
    flags = this->flags;
    result = flags & 0x840000;
    if (result == 0)
        return result;
    if ((this->node.flags & 1) != 0)
        return 0x200000;
    step = (flags & 0x200000) != 0 ? -1 : 1;
    this->path_frame = step;
    return this->AdvanceTracks(step);
}

uint32 AnimatedObject::FollowSceneAnimation()
{
    FUNCTION_MARKER(0x800611E8, "SLUS_005.78");
    AnimatedObject *parent = ((AnimatedObject *)((this->linked_object)));
    uint32 result = (uint32)(sint32)parent->path_frame;
    uint32 flags, parent_flags, frame;
    if (result == 0)
        return result;
    result = 0x40000;
    if ((this->node.flags & 1) != 0)
        return result;
    flags = this->flags | 0x40000;
    this->flags = flags;
    parent_flags = parent->flags;
    if ((parent_flags & 0x1000000) != 0)
        this->flags = flags | 0x20000;
    else if ((parent_flags & 0x2000000) != 0)
        this->flags = flags | 0x10000;
    frame = (uint16)parent->path_frame;
    this->path_frame = frame;
    this->AdvanceTracks((sint16)frame);
    parent_flags = parent->flags;
    result = parent_flags & 0x840000;
    if (result == 0)
    {
        flags = this->flags;
        result = parent_flags | 0x40000;
        if ((flags & 0x40000) != 0)
            parent->flags = result;
    }
    return result;
}

uint32 AnimatedPathState::StopSound()
{
    FUNCTION_MARKER(0x800425F4, "SLUS_005.78");
    if ((sound_effects.IsPlaying(this->sound, sound_voice) & 0xFF) != 0)
        StopSoundVoice(sound_voice);
    this->sound = 0;
    sound_voice = -1;
    return 0xFFFFFFFF;
}
