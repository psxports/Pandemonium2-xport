#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "model_group_registry.h"
#include "game.h"
#include "actor_model_registry.h"

ModelGroupRegistry model_grps = {0};

void ModelGroupRegistry::LoadModelGrp(uint8 kind)
{
    uint32 offset;
    ModelGroupDefinition *object;
    const uint8 *rec;
    uint32 idx;
    uint32 src_offset;
    uint32 prev;
    uint32 slot;

    uint32 flags;
    uint8 byte;
    FUNCTION_MARKER(0x8002B058, "SLUS_005.78");
    offset = resource_archive.Offset(resource_load_cursor.asset);
    slot = resource_load_cursor.asset + 1;
    idx = model_grps.count;
    src_offset = offset;
    uint32 metadata_bytes = resource_archive.Size(resource_load_cursor.asset);
    uint8 *metadata_input = (uint8 *)malloc(metadata_bytes);
    resource_archive.ReadAsset(resource_load_cursor.asset, metadata_input, 1, resource_load_cursor.container_asset);
    rec = metadata_input;
    object = model_grps.recs + idx;
    for (prev = 0; prev < idx; ++prev)
    {
        if (model_grps.src_packages[prev] == resource_load_cursor.container_asset && model_grps.src_offsets[prev] == src_offset)
        {
            object = model_grps.definitions[prev];
            break;
        }
    }
    model_grps.src_packages[idx] = resource_load_cursor.container_asset;
    model_grps.src_offsets[idx] = src_offset;
    model_grps.definitions[idx] = object;
    const EncodedModelGroupMetadata *metadata = (const EncodedModelGroupMetadata *)rec;
    object->flags = metadata->flags;
    object->kind = metadata->kind;
    object->lifetime = metadata->lifetime;
    for (uint32 model_idx = 0; model_idx < 4; ++model_idx)
    {
        object->models[model_idx] = 0;
    }
    object->launch_speed = metadata->launch_speed;
    object->metadata_word = metadata->metadata_word;
    object->gravity = metadata->gravity;
    object->sound_radius = metadata->sound_radius;
    object->doppler_radius = metadata->doppler_radius;
    object->contact_type = metadata->contact_type;
    model_grps.count = idx + 1;
    if ((sint32)resource_archive.Size(slot) <= 0)
        object->sound_table = 0;
    else
    {
        uint32 bytes = resource_archive.Size(resource_load_cursor.asset + 1);
        uint8 *sound_input = (uint8 *)malloc(bytes);
        resource_archive.ReadAsset(resource_load_cursor.asset + 1, sound_input, 1, resource_load_cursor.container_asset);
        const uint8 *src = sound_input;
        uint32 count = ((const EncodedSoundHeader *)src)->count;
        object->sound_bank.header.count = count;
        if ((sint32)count > 0)
        {
            for (uint32 sound_idx = 0; sound_idx < count; ++sound_idx)
            {
                const uint8 *encoded = src + sizeof(SoundTableHeader) + sound_idx * sizeof(SoundSample);
                SoundSample *sample = object->sound_bank.samples + sound_idx;
                const EncodedSoundSample *input_sample = (const EncodedSoundSample *)encoded;
                sample->spu_address = input_sample->spu_address;
                sample->pitch = input_sample->pitch;
                sample->metadata = input_sample->metadata;
            }
        }
        object->sound_table = object->sound_bank.header.Relocate(bytes, object->sound_bank.samples) != 0 ? &object->sound_bank : 0;
        free(sound_input);
    }
    object->kind = kind;
    resource_load_cursor.asset = resource_load_cursor.asset + 2;
    for (idx = 0; idx < 4; ++idx)
    {
        offset = resource_archive.Offset(resource_load_cursor.asset);
        if (offset != 0xFFFFFFFF)
        {
            {
                uint32 material_bytes = resource_archive.Size(resource_load_cursor.asset + 2);
                uint8 *material_input = (uint8 *)malloc(material_bytes);
                resource_archive.ReadAsset(resource_load_cursor.asset + 2, material_input, 1, resource_load_cursor.container_asset);
                MaterialCatalog::RelocateMaterialResource(&object->material_resources[idx], material_input, material_bytes, -1);
                free(material_input);
            }
            uint32 model_bytes = resource_archive.Size(resource_load_cursor.asset);
            uint8 *model_data = (uint8 *)malloc(model_bytes);
            resource_archive.ReadAsset(resource_load_cursor.asset, model_data, 1, resource_load_cursor.container_asset);
            free(object->model_vertices[idx]);
            object->model_vertices[idx] = 0;
            free(object->model_polygons[idx]);
            object->model_polygons[idx] = 0;
            free(object->model_data[idx]);
            object->model_data[idx] = model_data;
            object->model_bytes[idx] = model_bytes;
            object->models[idx] = 0;
            AnimatedObject *model_object;
            EncodedAnimatedObject *encoded_model = (EncodedAnimatedObject *)model_data;
            flags = encoded_model->flags;
            byte = encoded_model->node.flags;
            encoded_model->flags = flags | 0x20024;
            encoded_model->node.flags = byte | 0x80;
            AnimatedObject::DecodeModelResource(&object->models[idx], &object->model_data[idx], &object->material_resources[idx], object->model_bytes[idx], &object->model_polygons[idx], &object->model_vertices[idx]);
            model_object = object->models[idx];
            if ((sint32)resource_archive.Size(resource_load_cursor.asset + 1) <= 0)
            {
                free(object->animation_data[idx]);
                if (object->animation_headers[idx] != 0)
                {
                    for (uint32 header_idx = 0; header_idx < 1; ++header_idx)
                    {
                        for (uint32 track_grp = 0; track_grp < 4; ++track_grp)
                            free(object->animation_headers[idx][header_idx].tracks[track_grp].bytes);
                    }
                }
                free(object->animation_headers[idx]);
                object->animation_data[idx] = 0;
                object->animation_headers[idx] = 0;
                object->animation_bytes[idx] = 0;
                model_object->animation = 0;
                model_object->unbound_animation_word = 0;
            }
            else
            {
                resource_archive.Offset(resource_load_cursor.asset + 1);
                uint32 animation_bytes = resource_archive.Size(resource_load_cursor.asset + 1);
                uint8 *animation_data = (uint8 *)malloc(animation_bytes);
                AnimationHeader *header_storage = (AnimationHeader *)calloc(1, sizeof(AnimationHeader));
                resource_archive.ReadAsset(resource_load_cursor.asset + 1, animation_data, 1, resource_load_cursor.container_asset);
                free(object->animation_data[idx]);
                if (object->animation_headers[idx] != 0)
                {
                    for (uint32 header_idx = 0; header_idx < 1; ++header_idx)
                    {
                        for (uint32 track_grp = 0; track_grp < 4; ++track_grp)
                            free(object->animation_headers[idx][header_idx].tracks[track_grp].bytes);
                    }
                }
                free(object->animation_headers[idx]);
                object->animation_data[idx] = animation_data;
                object->animation_headers[idx] = header_storage;
                object->animation_bytes[idx] = animation_bytes;
                model_object->AttachAnimation(animation_data, animation_bytes, header_storage);
            }
        }
        resource_load_cursor.asset = resource_load_cursor.asset + 3;
    }
    free(metadata_input);
}

AnimatedObject *ModelGroupRegistry::UpdateModelGrp(AnimatedObject *animated_object)
{
    AnimationHeader *active_animation;
    const VectorAnimationTrack *vector_track;
    const VectorAnimationKey *vector_keys;
    FUNCTION_MARKER(0x8002C608, "SLUS_005.78");
    Vec3 vector;
    uint32 attenuation, pan, ending_attenuation, ending_pan;
    ModelGroupDefinition *const *table;
    const ModelGroupDefinition *definition, *entry;
    uint32 count, kind, phase, spawn_kind, frame_action;
    AnimatedObject *candidate, *spawned;
    const ScenePath *spawn_path;
    uint32 flags;
    uint32 val;

    uint32 x;
    uint32 y;
    uint32 z;
    sint32 a;
    sint32 b;
    sint32 c;
    uint32 sq;
    uint32 mag;
    uint32 factor;
    uint32 heading;
    uint32 pitch;
    uint32 radius;
    const SoundBankStorage *sound_table;
    const SoundSample *sound;
    sint32 next_frame, frame, end_frame, divisor, quotient, angle;
    count = model_grps.count;
    spawned = 0;
    next_frame = -1;
    table = model_grps.definitions;
    kind = animated_object->state;
    definition = 0;
    while (count != 0)
    {
        --count;
        entry = *table;
        if (entry != 0 && entry->kind == kind)
        {
            definition = entry;
            break;
        }
        ++table;
    }
    flags = animated_object->flags;
    if ((flags & 0x100) != 0)
    {
        if (animated_object->behavior == 6)
        {
            CharacterController::FindPlayerObject(0);

            x = camera_motion.focus_pos.x;
            y = animated_object->world_pos.x;
            val = x - y;
            vector.x = val;
            a = val;
            x = camera_motion.focus_pos.y;
            y = animated_object->world_pos.y;
            val = x - y;
            vector.y = val;
            b = val;
            x = camera_motion.focus_pos.z;
            y = animated_object->world_pos.z;
            val = x - y;
            vector.z = val;
            c = val;
            if ((definition->flags & 4) == 0)
            {
                x = a >> 6;
                y = b >> 6;
                z = c >> 6;
                sq = x * x + y * y + z * z;
                if ((sint32)sq > 163839999)
                    goto end_phase;
            }
        }
        candidate = definition->models[0];
        spawn_kind = 0;
        goto spawn_template;
    }
    if (animated_object->behavior != 6)
        goto apply_phase;
    if ((sint32)flags < 0)
    {
        frame_action = 0;
        goto calculate_frame;
    }
    if ((flags & 0x20000000) != 0)
    {
        frame_action = 1;
        goto calculate_frame;
    }
    if ((flags & 0x40000000) == 0)
        goto apply_phase;
    animated_object->ApplyModelGrpContactMotion();
    frame = animated_object->ticks;
    if (frame == 0)
        goto select_ending;
    if (frame > 0)
    {
        val = (uint32)frame - 1;
        animated_object->ticks = val;
        if ((uint16)val == 0)
        {
            if (definition->kind != 19)
                goto end_phase;
            if ((player_interaction.flags & 3) == 2)
            {
                player_interaction.flags = 0;
                player_interaction.target = 0;
                goto end_phase;
            }
            animated_object->ReleaseActorInteraction();
            goto apply_phase;
        }
    }
    // The looping sound is optional even when a Doppler radius is set
    if (definition->doppler_radius != 0 && definition->sound_table != 0 && definition->sound_table->header.count >= 3)
    {
        sound_table = definition->sound_table;
        sound = 0;
        if (sound_table != 0 && sound_table->header.count >= 3)
            sound = sound_table->samples + 2;
        pitch = sound->pitch;
        radius = definition->doppler_radius;
        frame = (sint16)animated_object->sound_voice;
        val = SoundEffects::PlaySpatialSound(sound, 8192, 0, pitch, animated_object->world_pos.comp, radius, frame, 1, 2);
        animated_object->sound_voice = val;
        if ((sint16)val >= 0)
        {
            x = animated_object->world_pos.x;
            a = camera_motion.focus_pos.x - x;
            vector.x = a;
            x = camera_motion.focus_pos.y;
            y = animated_object->world_pos.y;
            vector.y = x - y;
            z = camera_motion.focus_pos.z;
            c = z - animated_object->world_pos.z;
            vector.z = c;
            x = a >> 6;
            z = c >> 6;
            sq = x * x + z * z;
            vector.y = 0;
            mag = (sint32)sq >= 0 ? SquareRoot12(sq) : SquareRoot12((sq >> 2)) << 1;
            vector.y = mag;
            if (mag != 0)
            {
                divisor = definition->doppler_radius;
                if (divisor == 0)
                    xport_mips_break(7);
                quotient = 0x40000000 / divisor;
                val = ((sint64)(sint32)mag * quotient) >> 18;
                z = animated_object->velocity.z;
                x = animated_object->velocity.x;
                fixed_math_result = val;
                heading = Atan2Angle(z, x);
                z = vector.z;
                x = vector.x;
                val = Atan2Angle(z, x);
                val = (heading - val) & 0xFFF;
                if (val >= 2049)
                    val = 4096 - val;
                angle = (sint16)(val - 1024);
                factor = fixed_math_result;
                factor = mul12(angle, (sint32)factor);
                factor = 4096 - (uint32)(sint16)factor;
                val = mul12((sint32)(pitch << 12), (sint32)factor);
                SoundEffects::SetSoundVoicePitch(animated_object->sound_voice, ((sint32)val >> 12));
            }
        }
    }
    if (definition->launch_speed == 0 && (definition->flags & 1) == 0)
    {
        frame_action = 2;
        goto calculate_frame;
    }
    val = animated_object->ComputeObjectPitch(animated_object->velocity, 0);
    animated_object->orientation[1] = (val - 1024) & 0xFFF;
    if ((definition->flags & 1) != 0)
    {
        val = animated_object->velocity.y;
        factor = definition->gravity;
        animated_object->velocity.y = val - factor;
    }
    frame_action = 3;
calculate_frame:
    active_animation = animated_object->animation;
    next_frame = 0;
    if (active_animation != 0)
    {
        end_frame = active_animation->frame_count - 1;
        frame = animated_object->frame;
        if (frame < end_frame)
            next_frame = frame + 1;
    }
    if (frame_action == 0)
    {
        if (animated_object->frame >= next_frame)
        {
            animated_object->node.flags = animated_object->node.flags | 2;
            return 0;
        }
        goto apply_phase;
    }
    if (frame_action == 1)
    {
        animated_object->flags = animated_object->flags | 0x80000000;
        goto select_second;
    }
    if (frame_action == 2)
        goto apply_phase;
    divisor = definition->flags >> 2;
    if (divisor != 0)
    {
        frame = animated_object->ticks;
        end_frame = (sint16)definition->lifetime;
        if ((end_frame - (frame + 1)) % divisor != 0)
            goto apply_phase;
    }
    if (definition->kind == 19 && (player_interaction.flags & 4) != 0)
        goto apply_phase;
    candidate = definition->models[3];
    spawn_kind = 3;
    goto spawn_template;
select_second:
    candidate = definition->models[1];
    spawn_kind = 1;
    goto spawn_template;
select_ending:
    candidate = definition->models[2];
    spawn_kind = 2;
spawn_template:
    if (candidate != 0)
    {
        if ((candidate->node.flags & 0x80) != 0)
        {
            val = candidate->node.flags;
            candidate->node.links.sibling_next = 0;
            candidate->node.flags = val & 0x7F;
        }
        else
        {
            candidate = AnimatedObject::CloneObjectResource(candidate);
            if (candidate == 0)
                goto after_spawn;
            candidate->flags &= 0xFFFFFFFB;
        }
        flags = candidate->flags;
        candidate->spheres[0].radius = 0xFFFFFFFF;
        candidate->frame = 0;
        candidate->movement_mode = 0;
        candidate->velocity.z = 0;
        candidate->velocity.y = 0;
        candidate->velocity.x = 0;
        candidate->flags = (flags & 0x1FFD97EF) | 0x20000;
    }
after_spawn:
    if (spawn_kind == 3)
    {
        definition->InitializeSpawn(candidate, animated_object, 0);
        goto apply_phase;
    }
    spawned = candidate;
    if (spawn_kind == 0)
    {
        phase = 0x20000000;
        if (spawned == 0)
            goto select_second;
        goto initialize_motion;
    }
    if (spawn_kind == 1)
    {
        if (spawned == 0)
        {
            if ((animated_object->flags & 0x20000000) != 0)
                goto end_phase;
            goto select_ending;
        }
        phase = 0x40000000;
        if ((animated_object->flags & 0x2000) == 0)
            spawned->movement_mode = 2;

        spawn_path = camera_motion.path;
        spawned->contact_kind = 3;
        spawned->scene_path = spawn_path;
        val = (uint8)animated_object->animation_binding.tracks[0];
        spawned->animation_binding.tracks[1] = -3;
        spawned->animation_binding.tracks[0] = val;
        val = definition->lifetime;
        spawned->spheres[2].pos.z = 0;
        spawned->spheres[2].pos.x = 0;
        spawned->spheres[1].pos.z = 0;
        spawned->spheres[1].pos.x = 0;
        spawned->ticks = val - 1;
        if ((animated_object->flags & 0x20000000) != 0)
            goto apply_phase;
        goto initialize_motion;
    }
    if (spawned == 0)
        goto end_phase;
    phase = 0x80000000;
    if (definition->sound_table != 0)
    {
        if ((SoundEffects::ComputeSpatialSoundControls(animated_object->world_pos.comp, definition->sound_radius, &ending_attenuation, &ending_pan) & 0xFF) == 0)
            goto apply_phase;
        factor = sound_settings.effects_volume;
        val = ending_attenuation;
        val = (sint32)(val * factor) >> 12;
        ending_attenuation = val;
        if (val == 0)
            goto apply_phase;
        sound_table = definition->sound_table;
        sound = 0;
        if (sound_table != 0 && sound_table->header.count >= 2)
            sound = sound_table->samples + 1;
        val = (uint16)ending_attenuation;
        factor = (uint16)ending_pan;
        sound_effects.Queue(sound, 0, 0, val, factor, 2);
    }
    goto apply_phase;
initialize_motion:
    flags = animated_object->behavior == 6;
    if (flags == 0)
    {
        frame = animated_object->contact_kind;
        flags = frame == 3;
    }
    if (flags != 0)
    {
        flags = spawned->flags;
        spawned->animation_binding.tracks[0] = -3;
        spawned->flags = flags | 0x2000;
    }
    else
    {
        spawned->animation_binding.tracks[0] = -(uint32)frame;
        if (definition->launch_speed != 0 || (definition->flags & 1) != 0)
        {
            if (animated_object->contact_kind == 1)
            {
                val = (uint16)(character_pad.motion.form_flags >> 16) & 15;
                if (val - 5 < 2 || val == 7 || val - 10 < 2)
                    spawned->flags = spawned->flags | 0x2000;
            }
            heading = ((uint16)animated_object->target_yaw + 2048) & 0xFFF;
            factor = definition->launch_speed;
            val = SinQ14(heading);
            val = mul12((sint32)factor, ((sint32)val >> 2));
            spawned->velocity.x = val;
            spawned->velocity.y = 0;
            factor = definition->launch_speed;
            val = SinQ14((1024 - heading));
            spawned->velocity.z = mul12((sint32)factor, ((sint32)val >> 2));
        }
        else
        {
            spawned->velocity.z = 0;
            spawned->velocity.y = 0;
            spawned->velocity.x = 0;
        }
    }
    if (definition->sound_table != 0 && (SoundEffects::ComputeSpatialSoundControls(animated_object->world_pos.comp, definition->sound_radius, &attenuation, &pan) & 0xFF) != 0)
    {
        factor = sound_settings.effects_volume;
        val = attenuation;
        val = (sint32)(val * factor) >> 12;
        attenuation = val;
        if (val != 0)
        {
            sound_table = definition->sound_table;
            sound = 0;
            if (sound_table != 0 && sound_table->header.count > 0)
                sound = sound_table->samples;
            val = (uint16)attenuation;
            factor = (uint16)pan;
            sound_effects.Queue(sound, 0, 0, val, factor, 2);
        }
    }
apply_phase:
    if (spawned == 0)
    {
        if (next_frame != -1)
        {
            active_animation = animated_object->animation;
            animated_object->frame = next_frame;
            if (active_animation != 0)
                animated_object->ClampAnimationFrame();
        }
        goto animate_velocity;
    }
    definition->InitializeSpawn(spawned, animated_object, phase);
end_phase:
    if (animated_object->behavior != 6)
        goto finish;
    flags = animated_object->flags;
    if ((flags & 0x20000100) != 0)
    {
        animated_object->flags = flags & 0xDFFFFEFF;
        goto animate_velocity;
    }
    if (definition->kind == 19)
    {
        if (player_interaction.flags != 0)
        {
            animated_object->ReleaseActorInteraction();
            goto animate_velocity;
        }
    }
    else if ((flags & 0x40000020) == 0x40000020)
    {
        count = model_grps.count;
        table = model_grps.definitions;
        kind = animated_object->state;
        const ModelGroupDefinition *sound_definition = 0;
        while (count != 0)
        {
            --count;
            entry = *table;
            if (entry != 0 && entry->kind == kind)
            {
                sound_definition = entry;
                break;
            }
            ++table;
        }
        if (sound_definition->doppler_radius != 0)
        {
            sound_table = sound_definition->sound_table;
            sound = 0;
            if (sound_table != 0 && sound_table->header.count >= 3)
                sound = sound_table->samples + 2;
            frame = (sint16)animated_object->sound_voice;
            if ((sound_effects.IsPlaying(sound, frame) & 0xFF) != 0)
                SoundEffects::StopSoundVoice(animated_object->sound_voice);
            animated_object->sound_voice = 0xFFFF;
        }
    }
    animated_object->node.flags = animated_object->node.flags | 2;
animate_velocity:
    if (animated_object->behavior == 6 && (animated_object->flags & 0x20) == 0)
    {
        uint32 *event_slots[1] = {&animated_object->event_word};
        if ((SceneEvents::ApplySceneEvents(event_slots, 1) & 0xFF) != 0)
            animated_object->flags = animated_object->flags | 0x100;
        active_animation = animated_object->animation;
        frame = animated_object->animation_binding.tracks[0];
        vector_track = active_animation->tracks[0].vectors;
        vector_track = vector_track + frame;
        vector_keys = (const VectorAnimationKey *)vector_track->track.keys;
        x = vector_keys[1].pos.x;
        y = vector_keys[0].pos.x;
        vector.x = x - y;
        x = vector_keys[1].pos.y;
        y = vector_keys[0].pos.y;
        vector.y = x - y;
        x = vector_keys[1].pos.z;
        y = vector_keys[0].pos.z;
        vector.z = x - y;
        PushMatrix();
        MATRIX *parent_matrix = render_view.matrix_cursor->prev != 0 ? &render_view.matrix_cursor->prev->matrix : 0;
        SetRotMatrix(parent_matrix);
        {
            VECTOR rotation_input = {vector.x, vector.y, vector.z, 0};
            VECTOR rotation_output;
            ApplyRotMatrixLV(&rotation_input, &rotation_output);
            animated_object->velocity.x = rotation_output.vx;
            animated_object->velocity.y = rotation_output.vy;
            animated_object->velocity.z = rotation_output.vz;
        }
        PopMatrix();
    }
finish:
    return spawned;
}

void ModelGroupRegistry::RetainCommonModelGrps(void)
{
    FUNCTION_MARKER(0x8002D4C0, "SLUS_005.78");
    for (uint32 grp_idx = ModelGroupRegistry::CommonCount; grp_idx < ModelGroupRegistry::Capacity; ++grp_idx)
    {
        for (uint32 model_idx = 0; model_idx < 4; ++model_idx)
        {
            free(model_grps.recs[grp_idx].material_resources[model_idx].texs);
            free(model_grps.recs[grp_idx].material_resources[model_idx].materials);
            free(model_grps.recs[grp_idx].material_resources[model_idx].data);
            memset(&model_grps.recs[grp_idx].material_resources[model_idx], 0, sizeof(model_grps.recs[grp_idx].material_resources[model_idx]));
            free(model_grps.recs[grp_idx].model_vertices[model_idx]);
            model_grps.recs[grp_idx].model_vertices[model_idx] = 0;
            free(model_grps.recs[grp_idx].model_polygons[model_idx]);
            model_grps.recs[grp_idx].model_polygons[model_idx] = 0;
            free(model_grps.recs[grp_idx].model_data[model_idx]);
            model_grps.recs[grp_idx].model_data[model_idx] = 0;
            model_grps.recs[grp_idx].model_bytes[model_idx] = 0;
            free(model_grps.recs[grp_idx].animation_data[model_idx]);
            if (model_grps.recs[grp_idx].animation_headers[model_idx] != 0)
            {
                for (uint32 header_idx = 0; header_idx < 1; ++header_idx)
                {
                    for (uint32 track_grp = 0; track_grp < 4; ++track_grp)
                        free(model_grps.recs[grp_idx].animation_headers[model_idx][header_idx].tracks[track_grp].bytes);
                }
            }
            free(model_grps.recs[grp_idx].animation_headers[model_idx]);
            model_grps.recs[grp_idx].animation_data[model_idx] = 0;
            model_grps.recs[grp_idx].animation_headers[model_idx] = 0;
            model_grps.recs[grp_idx].animation_bytes[model_idx] = 0;
        }
    }
    // The common loader registers three groups
    model_grps.count = ModelGroupRegistry::CommonCount;
}

void ModelGroupRegistry::LoadCommonModelGrps(void)
{
    FUNCTION_MARKER(0x8002D4D0, "SLUS_005.78");
    {
        ObjectCallback operation = {0, 0, ModelGroupRegistry::UpdateModelGrp};
        object_operations.behaviors[6] = operation;
    }
    ModelGroupRegistry::LoadModelGrp(0);
    ModelGroupRegistry::LoadModelGrp(11);
    ModelGroupRegistry::LoadModelGrp(19);
}

void ModelGroupRegistry::ClearModelGrps(void)
{
    FUNCTION_MARKER(0x8002D510, "SLUS_005.78");
    for (uint32 grp_idx = ModelGroupRegistry::CommonCount; grp_idx < ModelGroupRegistry::Capacity; ++grp_idx)
    {
        for (uint32 model_idx = 0; model_idx < 4; ++model_idx)
        {
            free(model_grps.recs[grp_idx].material_resources[model_idx].texs);
            free(model_grps.recs[grp_idx].material_resources[model_idx].materials);
            free(model_grps.recs[grp_idx].material_resources[model_idx].data);
            memset(&model_grps.recs[grp_idx].material_resources[model_idx], 0, sizeof(model_grps.recs[grp_idx].material_resources[model_idx]));
            free(model_grps.recs[grp_idx].model_vertices[model_idx]);
            model_grps.recs[grp_idx].model_vertices[model_idx] = 0;
            free(model_grps.recs[grp_idx].model_polygons[model_idx]);
            model_grps.recs[grp_idx].model_polygons[model_idx] = 0;
            free(model_grps.recs[grp_idx].model_data[model_idx]);
            model_grps.recs[grp_idx].model_data[model_idx] = 0;
            model_grps.recs[grp_idx].model_bytes[model_idx] = 0;
            free(model_grps.recs[grp_idx].animation_data[model_idx]);
            if (model_grps.recs[grp_idx].animation_headers[model_idx] != 0)
            {
                for (uint32 header_idx = 0; header_idx < 1; ++header_idx)
                {
                    for (uint32 track_grp = 0; track_grp < 4; ++track_grp)
                        free(model_grps.recs[grp_idx].animation_headers[model_idx][header_idx].tracks[track_grp].bytes);
                }
            }
            free(model_grps.recs[grp_idx].animation_headers[model_idx]);
            model_grps.recs[grp_idx].animation_data[model_idx] = 0;
            model_grps.recs[grp_idx].animation_headers[model_idx] = 0;
            model_grps.recs[grp_idx].animation_bytes[model_idx] = 0;
        }
    }
    model_grps.count = 0;
}

ModelGroupDefinition *ModelGroupRegistry::FindRelatedModelGrp(uint32 kind)
{
    FUNCTION_MARKER(0x8002D51C, "SLUS_005.78");
    ModelGroupDefinition *const *slot;
    uint32 type, count;
    switch (kind)
    {
        case 15:
            type = 23;
            break;
        case 8:
            type = 28;
            break;
        case 12:
            type = 26;
            break;
        case 5:
            type = 30;
            break;
        case 31:
            type = 42;
            break;
        case 44:
            type = 4;
            break;
        default:
            return 0;
    }
    count = model_grps.count;
    slot = model_grps.definitions;
    while (count != 0)
    {
        ModelGroupDefinition *object = *slot;
        --count;
        if (object != 0 && object->kind == type)
            return object;
        ++slot;
    }
    return 0;
}

ModelGroupDefinition *ModelGroupRegistry::SetRelatedModelGrpContactType(uint32 kind, uint8 val)
{
    FUNCTION_MARKER(0x8002D71C, "SLUS_005.78");
    ModelGroupDefinition *result = ModelGroupRegistry::FindRelatedModelGrp(kind);
    if (result != 0)
        result->contact_type = val;
    return result;
}

uint32 ModelGroupRegistry::GetModelGrpContactType(uint32 kind)
{
    FUNCTION_MARKER(0x8002D750, "SLUS_005.78");
    uint32 remaining = model_grps.count;
    ModelGroupDefinition *const *slot = model_grps.definitions;
    const ModelGroupDefinition *found = 0;
    while (remaining != 0)
    {
        const ModelGroupDefinition *definition = *slot;
        --remaining;
        if (definition != 0 && definition->kind == kind)
        {
            found = definition;
            break;
        }
        ++slot;
    }
    return found->contact_type;
}

uint32 ModelGroupRegistry::GetModelGrpLifetime(uint32 kind)
{
    FUNCTION_MARKER(0x8002D7A4, "SLUS_005.78");
    ModelGroupDefinition *const *table = model_grps.definitions;
    const ModelGroupDefinition *found = 0;
    uint32 count = model_grps.count;
    while (count != 0)
    {
        const ModelGroupDefinition *candidate = *table;
        --count;
        if (candidate != 0 && candidate->kind == kind)
        {
            found = candidate;
            break;
        }
        ++table;
    }
    return (sint16)found->lifetime;
}
