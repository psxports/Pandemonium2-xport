#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "actor_model_registry.h"
#include "game.h"

// Original constructor order includes the reserved slot before the attack actor
static const ActorModelDefinition::Constructor actor_constructors[48] = {&AnimatedObject::InitializeCommonActor,
                                                                         &AnimatedObject::InitializeActorFacingAttack,
                                                                         &AnimatedObject::InitializeCommonActor,
                                                                         &AnimatedObject::InitializeCommonActor,
                                                                         &AnimatedObject::InitializeActorProximityAttack,
                                                                         &AnimatedObject::InitializeActorStationaryShots,
                                                                         &AnimatedObject::InitializeActorRandomPatrol,
                                                                         &AnimatedObject::InitializeActorTargetPursuit,
                                                                         &AnimatedObject::InitializeActorVerticalShot,
                                                                         &AnimatedObject::InitializeCommonActor,
                                                                         &AnimatedObject::InitializeActorChaseAttack,
                                                                         &AnimatedObject::InitializeCommonActor,
                                                                         &AnimatedObject::InitializeActorVerticalShot,
                                                                         &AnimatedObject::InitializeCommonActor,
                                                                         &AnimatedObject::InitializeCommonActor,
                                                                         &AnimatedObject::InitializeActorAimedShot,
                                                                         &AnimatedObject::InitializeActorPatrolPause,
                                                                         &AnimatedObject::InitializeActorPathSegMotion,
                                                                         &AnimatedObject::InitializeActorVerticalPursuit,
                                                                         &AnimatedObject::InitializeActorPathDetour,
                                                                         &AnimatedObject::InitializeCommonActor,
                                                                         &AnimatedObject::InitializeCommonActor,
                                                                         &AnimatedObject::InitializeCommonActor,
                                                                         &AnimatedObject::InitializeCommonActor,
                                                                         &AnimatedObject::InitializeCommonActor,
                                                                         &AnimatedObject::InitializeCommonActor,
                                                                         &AnimatedObject::InitializeCommonActor,
                                                                         &AnimatedObject::InitializeCommonActor,
                                                                         &AnimatedObject::InitializeCommonActor,
                                                                         &AnimatedObject::InitializeCommonActor,
                                                                         &AnimatedObject::InitializeCommonActor,
                                                                         &AnimatedObject::InitializeCommonActor,
                                                                         &AnimatedObject::InitializeCommonActor,
                                                                         &AnimatedObject::InitializeCommonActor,
                                                                         &AnimatedObject::InitializeCommonActor,
                                                                         &AnimatedObject::InitializeCommonActor,
                                                                         &AnimatedObject::InitializeCommonActor,
                                                                         &AnimatedObject::InitializeCommonActor,
                                                                         &AnimatedObject::InitializeCommonActor,
                                                                         &AnimatedObject::InitializeCommonActor,
                                                                         &AnimatedObject::InitializeCommonActor,
                                                                         &AnimatedObject::InitializeCommonActor,
                                                                         &AnimatedObject::InitializeCommonActor,
                                                                         &AnimatedObject::InitializeCommonActor,
                                                                         &AnimatedObject::InitializeCommonActor,
                                                                         &AnimatedObject::InitializeVolleyEnemy,
                                                                         0,
                                                                         &AnimatedObject::InitializeAttackEnemy};

ActorModelRegistry actor_models = {0};

uint32 ActorModelRegistry::LoadActorModel(uint32 idx)
{
    FUNCTION_MARKER(0x8003420C, "SLUS_005.78");
    uint32 asset, counter, saved_flags, val;
    sint32 callback_idx;
    asset = resource_load_cursor.asset;
    counter = (uint8)actor_models.count;
    actor_models.count = counter + 1;
    ActorModelDefinition *definition = actor_models.definitions + (sint8)counter;
    val = resource_archive.Size(asset + 4);
    if ((sint32)val <= 0)
        definition->sound_table = 0;
    else
    {
        uint32 sample_count, sample_idx;
        const uint8 *sound_data;
        SoundTableHeader *sound_table;
        uint8 *sound_input = (uint8 *)malloc(val);
        resource_archive.ReadAsset(resource_load_cursor.asset + 4, sound_input, 1, resource_load_cursor.container_asset);
        sound_data = sound_input;
        definition->sound_bank.header.count = ((const EncodedSoundHeader *)sound_data)->count;
        sample_count = definition->sound_bank.header.count;
        if ((sint32)sample_count > 0)
        {
            for (sample_idx = 0; sample_idx < sample_count; ++sample_idx)
            {
                SoundSample *sample = definition->sound_bank.samples + sample_idx;
                const uint8 *encoded = sound_data + sizeof(SoundTableHeader) + sample_idx * sizeof(SoundSample);
                const EncodedSoundSample *input_sample = (const EncodedSoundSample *)encoded;
                sample->spu_address = input_sample->spu_address;
                sample->pitch = input_sample->pitch;
                sample->metadata = input_sample->metadata;
            }
        }
        sound_table = definition->sound_bank.header.Relocate(val, definition->sound_bank.samples);
        definition->sound_table = sound_table != 0 ? &definition->sound_bank : 0;
        free(sound_input);
    }
    uint32 metadata_bytes = resource_archive.Size(asset);
    uint8 *metadata_input = (uint8 *)malloc(metadata_bytes);
    resource_archive.ReadAsset(asset, metadata_input, 1, resource_load_cursor.container_asset);
    const EncodedActorMetadata *rec = (const EncodedActorMetadata *)metadata_input;
    for (uint32 action_idx = 0; action_idx < 6; ++action_idx)
    {
        const EncodedActorAction *encoded = rec->actions + action_idx;
        ActorActionDefinition *action = definition->actions + action_idx;
        action->flags = encoded->flags;
        action->distance_threshold = encoded->distance_threshold;
        action->priority_mode = encoded->priority_mode;
        action->trigger = encoded->trigger;
        action->priority_frame = encoded->priority_frame;
        action->trigger_ticks = encoded->trigger_ticks;
        action->animation = encoded->animation;
        action->path_motion_mode = encoded->path_motion_mode;
        action->hold_ticks = encoded->hold_ticks;
        action->hold_animation = encoded->hold_animation;
        action->hold_speed = encoded->hold_speed;
        action->hold_motion = encoded->hold_motion;
        action->motion = encoded->motion;
        action->speed = encoded->speed;
        action->switch_frame = encoded->switch_frame;
        action->next_motion = encoded->next_motion;
        action->next_speed = encoded->next_speed;
        action->projectile_kind = encoded->projectile_kind;
        action->projectile_frame = encoded->projectile_frame;
        action->projectile_height = encoded->projectile_height;
        action->projectile_interval = encoded->projectile_interval;
        action->projectile_end_frame = encoded->projectile_end_frame;
        action->aim_mode = encoded->aim_mode;
        action->spread_angle = encoded->spread_angle;
        action->projectile_offset = encoded->projectile_offset;
    }
    for (uint32 animation_idx = 0; animation_idx < 14; ++animation_idx)
    {
        definition->animations[animation_idx] = 0;
    }
    definition->kind = rec->kind;
    definition->kind_override = rec->kind_override;
    definition->initial_health = rec->initial_health;
    definition->target_range[0] = rec->target_range[0];
    definition->target_range[1] = rec->target_range[1];
    definition->height_offset = rec->height_offset;
    definition->patrol_radius = rec->patrol_radius;
    definition->shot_interval = rec->shot_interval;
    definition->shot_frame = rec->shot_frame;
    definition->path_flag = rec->path_flag;
    definition->fixed_pitch = rec->fixed_pitch;
    definition->pitch_limit = rec->pitch_limit;
    definition->target_avoidance_mode = rec->target_avoidance_mode;
    definition->health_restore_count = rec->health_restore_count;

    definition->contact_damage = rec->contact_damage;
    definition->motion_speed[0] = rec->motion_speed[0];
    definition->motion_speed[1] = rec->motion_speed[1];
    definition->motion_accel = rec->motion_accel;
    definition->spr_extent[0] = rec->spr_extent[0];
    definition->spr_extent[1] = rec->spr_extent[1];
    definition->turn_step = rec->turn_step;
    definition->reaction_motion_mode = rec->reaction_motion_mode;
    definition->turn_motion_mode = rec->turn_motion_mode;
    definition->instance_cost = rec->instance_cost;
    definition->sound_radius = rec->sound_radius;
    definition->contact_mode = rec->contact_mode;
    definition->fog_color[0] = rec->fog_color[0];
    definition->fog_color[1] = rec->fog_color[1];
    definition->fog_color[2] = rec->fog_color[2];
    definition->action_damage = rec->action_damage;
    definition->sound_interval = rec->sound_interval;
    definition->interaction_limit_step = rec->interaction_limit_step;
    definition->interaction_delay_bias = rec->interaction_delay_bias;
    definition->interaction_animation_mode = rec->interaction_animation_mode;
    definition->interaction_frame = rec->interaction_frame;
    definition->route_action = rec->route_action;
    definition->turnaround_range = rec->turnaround_range;
    definition->action_counter_flags = rec->action_counter_flags;
    callback_idx = definition->kind;
    definition->constructor = actor_constructors[callback_idx < 44 ? callback_idx : callback_idx + 1];
    callback_idx = definition->kind_override;
    if (callback_idx != -1)
        definition->constructor = actor_constructors[callback_idx];
    asset = resource_load_cursor.asset;
    saved_flags = rec->flags;
    {
        uint32 material_bytes = resource_archive.Size(asset + 3);
        uint8 *material_input = (uint8 *)malloc(material_bytes);
        resource_archive.ReadAsset(asset + 3, material_input, 1, resource_load_cursor.container_asset);
        MaterialCatalog::RelocateMaterialResource(&definition->material_resource, material_input, material_bytes, -1);
        free(material_input);
    }
    uint32 model_bytes = resource_archive.Size(resource_load_cursor.asset + 1);
    uint8 *model_data = (uint8 *)malloc(model_bytes);
    resource_archive.ReadAsset(resource_load_cursor.asset + 1, model_data, 1, resource_load_cursor.container_asset);
    free(definition->model_vertices);
    definition->model_vertices = 0;
    free(definition->model_polygons);
    definition->model_polygons = 0;
    free(definition->model_data);
    definition->model_data = model_data;
    definition->model_bytes = model_bytes;
    definition->prototype = 0;
    AnimatedObject::DecodeModelResource(&definition->prototype, &definition->model_data, &definition->material_resource, definition->model_bytes, &definition->model_polygons, &definition->model_vertices);
    uint32 animation_bytes = resource_archive.Size(resource_load_cursor.asset + 2);
    uint8 *animation_data = (uint8 *)malloc(animation_bytes);
    AnimationHeader *header_storage = (AnimationHeader *)calloc(14, sizeof(AnimationHeader));
    resource_archive.ReadAsset(resource_load_cursor.asset + 2, animation_data, 1, resource_load_cursor.container_asset);
    free(definition->animation_data);
    if (definition->animation_headers != 0)
    {
        for (uint32 header_idx = 0; header_idx < 14; ++header_idx)
        {
            for (uint32 track_grp = 0; track_grp < 4; ++track_grp)
                free(definition->animation_headers[header_idx].tracks[track_grp].bytes);
        }
    }
    free(definition->animation_headers);
    free(definition->morph_restoration);
    definition->animation_data = animation_data;
    definition->animation_headers = header_storage;
    definition->animation_bytes = animation_bytes;
    AnimationHeader::LoadAnimationTable(definition->prototype, animation_data, animation_bytes, definition->animations, 14, header_storage, &definition->morph_restoration);
    AnimatedObject *prototype = definition->prototype;
    prototype->node.flags |= 0x80;
    prototype = definition->prototype;
    prototype->flags |= 0x20024 | saved_flags;
    prototype = definition->prototype;
    asset = resource_load_cursor.asset;
    val = prototype->clone_bytes;
    resource_load_cursor.asset = asset + 5;
    definition->instance_cost = val;
    free(metadata_input);
    return 1;
}

void ActorModelRegistry::ResetActorModels(void)
{
    FUNCTION_MARKER(0x80034B38, "SLUS_005.78");
    for (uint32 model_idx = 0; model_idx < sizeof(actor_models.definitions) / sizeof(actor_models.definitions[0]); ++model_idx)
    {
        free(actor_models.definitions[model_idx].material_resource.texs);
        free(actor_models.definitions[model_idx].material_resource.materials);
        free(actor_models.definitions[model_idx].material_resource.data);
        memset(&actor_models.definitions[model_idx].material_resource, 0, sizeof(actor_models.definitions[model_idx].material_resource));
        free(actor_models.definitions[model_idx].model_vertices);
        actor_models.definitions[model_idx].model_vertices = 0;
        free(actor_models.definitions[model_idx].model_polygons);
        actor_models.definitions[model_idx].model_polygons = 0;
        free(actor_models.definitions[model_idx].model_data);
        actor_models.definitions[model_idx].model_data = 0;
        actor_models.definitions[model_idx].model_bytes = 0;
        free(actor_models.definitions[model_idx].animation_data);
        if (actor_models.definitions[model_idx].animation_headers != 0)
        {
            for (uint32 header_idx = 0; header_idx < 14; ++header_idx)
            {
                for (uint32 track_grp = 0; track_grp < 4; ++track_grp)
                    free(actor_models.definitions[model_idx].animation_headers[header_idx].tracks[track_grp].bytes);
            }
        }
        free(actor_models.definitions[model_idx].animation_headers);
        free(actor_models.definitions[model_idx].morph_restoration);
        actor_models.definitions[model_idx].animation_data = 0;
        actor_models.definitions[model_idx].animation_headers = 0;
        actor_models.definitions[model_idx].morph_restoration = 0;
        actor_models.definitions[model_idx].animation_bytes = 0;
    }
    actor_models.count = 0;
}

void ActorModelRegistry::InitializeActorModels(void)
{
    FUNCTION_MARKER(0x80034B44, "SLUS_005.78");
    object_operations.collisions[2] = &AnimatedObject::CollectActorContact;
    {
        ObjectCallback operation = {0, &AnimatedObject::UpdateActorPathMotion};
        object_operations.motions[4] = operation;
    }
    {
        ObjectCallback operation = {0, &AnimatedObject::UpdateActorDeath};
        object_operations.variants[2] = operation;
    }
    {
        ObjectCallback operation = {0, &AnimatedObject::UpdateActorReactionSpin};
        object_operations.behaviors[16] = operation;
    }
    {
        ObjectCallback operation = {0, 0, 0, &AnimatedObject::UpdateActorReactionRecovery};
        object_operations.behaviors[17] = operation;
    }
    actor_models.count = 0;
}

uint32 ActorModelRegistry::ResetActorLoadState(void)
{
    FUNCTION_MARKER(0x80034F04, "SLUS_005.78");
    actor_models.load_reset_byte = 0;
    return 0;
}
