#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "character_model_record.h"
#include "game.h"
#include "hd.h"

CharacterModelRecord character_model_headers[CharacterController::ModelCount];

void CharacterModelRecord::HideAlternateCharacterModel(void)
{
    CharacterModelRecord *object_rec;
    FUNCTION_MARKER(0x80044474, "SLUS_005.78");
    uint32 idx;
    AnimatedObject *object;
    if ((sint32)character_pad.flags >= 0)
        return;
    idx = 1 - ((uint16)(character_pad.motion.form_flags >> 16) & 15);
    {
        uint32 model_idx = idx;
        CharacterModelRecord *model_binding;
        model_binding = character_pad.model_slots[model_idx];
        object_rec = model_binding;
    }
    object = object_rec->object;
    SceneNode *node = &object->node;
    node->flags = node->flags | 1;
}

void CharacterModelRecord::ReloadBaseCharacterModels(uint32 mode)
{
    CharacterModelRecord *object_rec;
    AnimatedObject *loaded_object;
    SpriteRecord *spr;
    FUNCTION_MARKER(0x80044508, "SLUS_005.78");
    hd_invalidate_geometry();
    AnimatedObject *objects[2];
    uint32 selector, asset0, asset1, asset;
    uint32 slot, idx, val, selection;
    sint32 remaining;
    val = game_flow.mode_flags;
    asset0 = 23;
    asset1 = 29;
    remaining = 1;
    slot = 0;
    idx = 0;
    if ((val & 0x200) != 0)
    {
        asset0 = 41;
        asset1 = 35;
    }
    character_pad.loaded_model_mask = mode;
    asset = asset0 + 1;
    selection = CharacterController::GetCurCharacterForm();
    if (mode == 1)
    {
        selector = 0;
        --remaining;
    }
    else if ((sint32)mode < 2)
    {
        --remaining;
        if (mode == 0)
        {
            goto finish;
        }
    }
    else if (mode == 2)
    {
        asset = asset1 + 1;
        slot = 1;
        selector = 0;
        --remaining;
    }
    else if (mode == 3)
    {
        remaining = 2;
        selector = selection;
        --remaining;
    }
    else
        --remaining;
    while (remaining >= 0)
    {
        uint32 metadata_bytes = resource_archive.Size(asset);
        uint8 *metadata_input = (uint8 *)malloc(metadata_bytes);
        resource_archive.ReadAsset(asset, metadata_input, 1, asset - 1);
        const CharacterModelHeader *input_header = (const CharacterModelHeader *)metadata_input;
        uint32 field;
        object_rec = &character_model_headers[idx];
        object_rec->movement = input_header->movement;
        for (field = 0; field < 2; ++field)
            object_rec->extent[field] = input_header->extent[field];
        object_rec->motion_scale.x = input_header->motion_scale.x;
        object_rec->motion_scale.y = input_header->motion_scale.y;
        object_rec->motion_scale.z = input_header->motion_scale.z;
        for (field = 0; field < CharacterModelRecord::AnimationCount; ++field)
        {
            object_rec->animations[field] = 0;
        }
        object_rec->object = 0;
        character_pad.model_slots[slot] = object_rec;
        if ((sint32)resource_archive.Size(asset + 4) > 0)
        {
            uint32 sound_bytes = resource_archive.Size(asset + 4);
            uint8 *sound_input = (uint8 *)malloc(sound_bytes);
            resource_archive.ReadAsset(asset + 4, sound_input, 1, asset - 1);
            SoundTableHeader *sound_table = (SoundTableHeader *)sound_input;
            sound_table->Relocate(sound_bytes, (SoundSample *)(sound_table + 1));
            free(sound_input);
        }
        {
            uint32 material_bytes = resource_archive.Size(asset + 3);
            uint8 *material_input = (uint8 *)malloc(material_bytes);
            resource_archive.ReadAsset(asset + 3, material_input, 1, asset - 1);
            MaterialCatalog::RelocateMaterialResource(&object_rec->material_resource, material_input, material_bytes, -1);
            free(material_input);
        }
        uint32 model_bytes = resource_archive.Size(asset + 1);
        uint8 *model_data = (uint8 *)malloc(model_bytes);
        resource_archive.ReadAsset(asset + 1, model_data, 1, asset - 1);
        free(object_rec->model_vertices);
        object_rec->model_vertices = 0;
        free(object_rec->model_polygons);
        object_rec->model_polygons = 0;
        free(object_rec->model_data);
        object_rec->model_data = model_data;
        object_rec->model_bytes = model_bytes;
        object_rec->object = 0;
        ((EncodedAnimatedObject *)model_data)->flags |= 0x20034;
        AnimatedObject::DecodeModelResource(&object_rec->object, &object_rec->model_data, &object_rec->material_resource, object_rec->model_bytes, &object_rec->model_polygons, &object_rec->model_vertices);
        objects[idx] = object_rec->object;
        loaded_object = object_rec->object;
        val = object_rec->extent[0];
        spr = loaded_object->spr;
        spr->extent[0] = (sint32)val >> 5;
        loaded_object = object_rec->object;
        val = object_rec->extent[1];
        spr = loaded_object->spr;
        spr->extent[1] = (sint32)val >> 5;
        loaded_object = object_rec->object;
        ++idx;
        uint32 animation_bytes = resource_archive.Size(asset + 2);
        uint8 *animation_data = (uint8 *)malloc(animation_bytes);
        AnimationHeader *header_storage = (AnimationHeader *)calloc(CharacterModelRecord::AnimationCount, sizeof(AnimationHeader));
        resource_archive.ReadAsset(asset + 2, animation_data, 1, asset - 1);
        free(object_rec->animation_data);
        if (object_rec->animation_headers != 0)
        {
            for (uint32 header_idx = 0; header_idx < 38; ++header_idx)
            {
                for (uint32 track_grp = 0; track_grp < 4; ++track_grp)
                    free(object_rec->animation_headers[header_idx].tracks[track_grp].bytes);
            }
        }
        free(object_rec->animation_headers);
        free(object_rec->morph_restoration);
        object_rec->animation_data = animation_data;
        object_rec->animation_headers = header_storage;
        object_rec->animation_bytes = animation_bytes;
        AnimationHeader::LoadAnimationTable(loaded_object, animation_data, animation_bytes, object_rec->animations, CharacterModelRecord::AnimationCount, header_storage, &object_rec->morph_restoration);
        free(metadata_input);
        ++slot;
        --remaining;
        asset = asset1 + 1;
    }
    selection = selector;
    character_pad.object = objects[selection];
finish:
    return;
}

uint32 CharacterModelRecord::IsAlternateCharacterInteractionActive(void)
{
    FUNCTION_MARKER(0x800447F0, "SLUS_005.78");
    if (((uint16)(character_pad.motion.form_flags >> 16) & 0xF) == 1 && (player_interaction.flags != 0 || (character_pad.secondary_flags & 0x20) != 0))
        return 1;
    return 0;
}

CharacterModelRecord *CharacterModelRecord::RestoreCharacterModelAnimations(void)
{
    CharacterModelRecord *result_rec;
    CharacterModelRecord *target_rec;
    FUNCTION_MARKER(0x80055280, "SLUS_005.78");
    AnimationHeader *anim0, *anim1;
    CharacterModelRecord *result = 0;
    character_pad.ClearTransientFlags();
    if ((character_pad.secondary_flags & 0x40) != 0)
    {
        target_rec = character_pad.model;
        anim0 = target_rec->animations[22];
        anim1 = target_rec->animations[2];
        target_rec->animations[2] = anim0;
        result_rec = character_pad.model;
        result = result_rec;
        result_rec->animations[22] = anim1;
    }
    return result;
}

void CharacterModelRecord::LoadCharacterModel(uint32 idx)
{
    CharacterModelRecord *object_rec;
    AnimatedObject *loaded_object;
    SpriteRecord *spr;

    FUNCTION_MARKER(0x800552FC, "SLUS_005.78");
    hd_invalidate_geometry();
    uint32 metadata_bytes = resource_archive.Size(resource_load_cursor.asset);
    uint8 *metadata_input = (uint8 *)malloc(metadata_bytes);
    resource_archive.ReadAsset(resource_load_cursor.asset, metadata_input, 1, resource_load_cursor.container_asset);
    const CharacterModelHeader *input_header = (const CharacterModelHeader *)metadata_input;
    uint32 field;
    object_rec = &character_model_headers[idx];
    object_rec->movement = input_header->movement;
    for (field = 0; field < 2; ++field)
        object_rec->extent[field] = input_header->extent[field];
    object_rec->motion_scale.x = input_header->motion_scale.x;
    object_rec->motion_scale.y = input_header->motion_scale.y;
    object_rec->motion_scale.z = input_header->motion_scale.z;
    for (field = 0; field < CharacterModelRecord::AnimationCount; ++field)
    {
        object_rec->animations[field] = 0;
    }
    object_rec->object = 0;
    character_pad.model_slots[idx] = object_rec;
    if ((sint32)resource_archive.Size(resource_load_cursor.asset + 4) > 0)
    {
        uint32 sound_bytes = resource_archive.Size(resource_load_cursor.asset + 4);
        uint8 *sound_input = (uint8 *)malloc(sound_bytes);
        resource_archive.ReadAsset(resource_load_cursor.asset + 4, sound_input, 1, resource_load_cursor.container_asset);
        SoundTableHeader *sound_table = (SoundTableHeader *)sound_input;
        sound_table->Relocate(sound_bytes, (SoundSample *)(sound_table + 1));
        free(sound_input);
    }
    {
        uint32 material_bytes = resource_archive.Size(resource_load_cursor.asset + 3);
        uint8 *material_input = (uint8 *)malloc(material_bytes);
        resource_archive.ReadAsset(resource_load_cursor.asset + 3, material_input, 1, resource_load_cursor.container_asset);
        MaterialCatalog::RelocateMaterialResource(&object_rec->material_resource, material_input, material_bytes, -1);
        free(material_input);
    }
    uint32 model_bytes = resource_archive.Size(resource_load_cursor.asset + 1);
    uint8 *model_data = (uint8 *)malloc(model_bytes);
    resource_archive.ReadAsset(resource_load_cursor.asset + 1, model_data, 1, resource_load_cursor.container_asset);
    free(object_rec->model_vertices);
    object_rec->model_vertices = 0;
    free(object_rec->model_polygons);
    object_rec->model_polygons = 0;
    free(object_rec->model_data);
    object_rec->model_data = model_data;
    object_rec->model_bytes = model_bytes;
    object_rec->object = 0;
    ((EncodedAnimatedObject *)model_data)->flags |= 0x20034;
    AnimatedObject::DecodeModelResource(&object_rec->object, &object_rec->model_data, &object_rec->material_resource, object_rec->model_bytes, &object_rec->model_polygons, &object_rec->model_vertices);
    loaded_object = object_rec->object;
    spr = loaded_object->spr;
    spr->extent[0] = object_rec->extent[0] >> 5;
    loaded_object = object_rec->object;
    spr = loaded_object->spr;
    spr->extent[1] = object_rec->extent[1] >> 5;
    uint32 animation_bytes = resource_archive.Size(resource_load_cursor.asset + 2);
    uint8 *animation_data = (uint8 *)malloc(animation_bytes);
    AnimationHeader *header_storage = (AnimationHeader *)calloc(CharacterModelRecord::AnimationCount, sizeof(AnimationHeader));
    resource_archive.ReadAsset(resource_load_cursor.asset + 2, animation_data, 1, resource_load_cursor.container_asset);
    free(object_rec->animation_data);
    if (object_rec->animation_headers != 0)
    {
        for (uint32 header_idx = 0; header_idx < 38; ++header_idx)
        {
            for (uint32 track_grp = 0; track_grp < 4; ++track_grp)
                free(object_rec->animation_headers[header_idx].tracks[track_grp].bytes);
        }
    }
    free(object_rec->animation_headers);
    free(object_rec->morph_restoration);
    object_rec->animation_data = animation_data;
    object_rec->animation_headers = header_storage;
    object_rec->animation_bytes = animation_bytes;
    AnimationHeader::LoadAnimationTable(object_rec->object, animation_data, animation_bytes, object_rec->animations, CharacterModelRecord::AnimationCount, header_storage, &object_rec->morph_restoration);
    free(metadata_input);
    resource_load_cursor.asset = resource_load_cursor.asset + 5;
}

void CharacterModelRecord::ResetCharacterModels(void)
{
    FUNCTION_MARKER(0x800556E0, "SLUS_005.78");
    uint32 flags, val, idx;
    CharacterModelRecord **entry;
    AnimatedObject *object = character_pad.object;
    if (object != 0)
    {
        character_pad.ClearTransientFlags();
        character_pad.flags = character_pad.flags & 0xC0000000;
        flags = object->flags;
        val = object->path_flags;
        object->flags = flags & 0xC0F793FF;
        object->path_flags = val & 0xFC000000;
        flags = character_pad.secondary_flags;
        character_pad.motion.jump_phase = 0;
        character_pad.motion.vertical_speed = 0;
        character_pad.motion.horizontal_speed = 0;
        character_pad.secondary_flags = flags & 0xFFFFF932;
        object->velocity.z = 0;
        object->velocity.y = 0;
        object->velocity.x = 0;
        character_pad.timers.special_move_ticks = 0;
        character_pad.timers.transition_ticks = 0;
        character_pad.timers.score_bonus_ticks = 0;
        character_pad.timers.action_cooldown_ticks = 0;
        character_pad.vitals.damage_ticks = 0;
        character_pad.timers.state_ticks = 0;
        character_pad.section_height_offset = 0;
        character_pad.timers.contact_impulse_ticks = 0;
        character_pad.motion.environment_acceleration = 0;
    }
    entry = character_pad.model_slots + 2;
    idx = 2;
    do
    {
        if (*entry != 0)
            *entry = 0;
        val = character_pad.motion.form_flags;
        if (idx == ((val >> 16) & 15))
            CharacterModelRecord::AssignCharacterModel(&character_pad.object, (val >> 20) & 15);
        ++idx;
        ++entry;
    } while (idx < 15);
}

void CharacterModelRecord::InitializeCharacterModels(void)
{
    uint32 idx;
    uint32 next;
    FUNCTION_MARKER(0x80055AC4, "SLUS_005.78");
    hd_invalidate_geometry();
    {
        ObjectCallback operation = {0, &AnimatedObject::UpdateCharacterMovement};
        object_operations.motions[1] = operation;
    }
    {
        ObjectCallback operation = {0, &AnimatedObject::UpdateCharacterBehavior};
        object_operations.behaviors[1] = operation;
    }
    {
        ObjectCallback operation = {0, &AnimatedObject::UpdateAlternateCharacterFollow};
        object_operations.behaviors[33] = operation;
    }
    {
        ObjectCallback operation = {0, &AnimatedObject::UpdateCharacterDeathTransition};
        object_operations.variants[1] = operation;
    }
    object_operations.collisions[1] = &AnimatedObject::CollectCharacterContact;
    gameplay_input.rotation_track_shift = 0;
    scr_render.fade_level = 0;
    {
        ObjectCallback operation = {0, &AnimatedObject::UpdateCharacterCameraTransition};
        object_operations.behaviors[2] = operation;
    }
    CharacterContactPool::InitializeContactPool();
    game_flow.transition_flags = game_flow.transition_flags | 0x10000;
    for (idx = 0; idx < 15; ++idx)
    {
        if ((sint32)resource_archive.Size(resource_load_cursor.asset) > 0)
        {
            next = resource_load_cursor.asset + 2;
            uint32 model_bytes = resource_archive.Size(resource_load_cursor.asset);
            uint8 *model_data = (uint8 *)malloc(model_bytes);
            resource_archive.ReadAsset(resource_load_cursor.asset, model_data, 1, resource_load_cursor.container_asset);
            free(action_effect_resources.model_vertices[idx]);
            action_effect_resources.model_vertices[idx] = 0;
            free(action_effect_resources.model_polygons[idx]);
            action_effect_resources.model_polygons[idx] = 0;
            free(action_effect_resources.model_data[idx]);
            action_effect_resources.model_data[idx] = model_data;
            action_effect_resources.model_bytes[idx] = model_bytes;
            action_effect_resources.templates[idx] = 0;
            {
                uint32 material_bytes = resource_archive.Size(next);
                uint8 *material_input = (uint8 *)malloc(material_bytes);
                resource_archive.ReadAsset(next, material_input, 1, resource_load_cursor.container_asset);
                MaterialCatalog::RelocateMaterialResource(&action_effect_resources.material_resources[idx], material_input, material_bytes, -1);
                free(material_input);
            }
            AnimatedObject *effect;
            EncodedAnimatedObject *encoded_model = (EncodedAnimatedObject *)model_data;
            uint32 flags = encoded_model->flags;
            uint8 node_flags = encoded_model->node.flags;
            encoded_model->flags = flags | 0x20024;
            encoded_model->node.flags = node_flags | 0x80;
            AnimatedObject::DecodeModelResource(&action_effect_resources.templates[idx], &action_effect_resources.model_data[idx], &action_effect_resources.material_resources[idx], action_effect_resources.model_bytes[idx], &action_effect_resources.model_polygons[idx], &action_effect_resources.model_vertices[idx]);
            effect = action_effect_resources.templates[idx];
            uint32 animation_bytes = resource_archive.Size(resource_load_cursor.asset + 1);
            uint8 *animation_data = (uint8 *)malloc(animation_bytes);
            AnimationHeader *header_storage = (AnimationHeader *)calloc(1, sizeof(AnimationHeader));
            resource_archive.ReadAsset(resource_load_cursor.asset + 1, animation_data, 1, resource_load_cursor.container_asset);
            free(action_effect_resources.animation_data[idx]);
            if (action_effect_resources.animation_headers[idx] != 0)
            {
                for (uint32 header_idx = 0; header_idx < 1; ++header_idx)
                {
                    for (uint32 track_grp = 0; track_grp < 4; ++track_grp)
                        free(action_effect_resources.animation_headers[idx][header_idx].tracks[track_grp].bytes);
                }
            }
            free(action_effect_resources.animation_headers[idx]);
            action_effect_resources.animation_data[idx] = animation_data;
            action_effect_resources.animation_headers[idx] = header_storage;
            action_effect_resources.animation_bytes[idx] = animation_bytes;
            effect->AttachAnimation(animation_data, animation_bytes, header_storage);
        }
        next = resource_load_cursor.asset + 3;
        resource_load_cursor.asset = next;
    }
    CharacterModelRecord::LoadCharacterModel(0);
    CharacterModelRecord::LoadCharacterModel(1);
    character_pad.loaded_model_mask = 3;
    CharacterController::ResetNewGameCharacter();
    saved_material_registry = material_registry;
    {
        ObjectCallback operation = {0, &AnimatedObject::UpdateActionObject};
        object_operations.behaviors[26] = operation;
    }
}

void CharacterModelRecord::ClearPrimaryModelSlots(void)
{
    FUNCTION_MARKER(0x80055D6C, "SLUS_005.78");
    uint32 idx;
    for (idx = 0; idx < 2; ++idx)
    {
        CharacterModelRecord **slot = character_pad.model_slots + idx;
        if (*slot != 0)
            *slot = 0;
    }
    character_pad.object = 0;
}

uint32 CharacterModelRecord::AssignCharacterModel(AnimatedObject **object_slot, uint32 kind)
{
    CharacterModelRecord *model_rec;
    CharacterModelRecord *table_rec;
    FUNCTION_MARKER(0x80055DC0, "SLUS_005.78");
    {
        uint32 model_idx = kind;
        CharacterModelRecord *model_binding;
        model_binding = character_pad.model_slots[model_idx];
        model_rec = model_binding;
    }
    uint32 flags, cur_kind, val, idx = 0;
    AnimatedObject *src, *dst;
    uint32 x, y, z;
    {
        ObjectCallback operation = {0, kind == 5 ? &AnimatedObject::UpdateForm5Movement : &AnimatedObject::UpdateCharacterMovement};
        object_operations.motions[1] = operation;
    }
    val = (character_pad.motion.form_flags & 0xFFF0FFFF) | (kind << 16);
    character_pad.motion.form_flags = (val & 0xF0FFFFFF) | 0x0F000000;
    flags = character_pad.flags;
    cur_kind = (uint16)(character_pad.motion.form_flags >> 16) & 0xF;
    character_pad.flags = flags & 0xBFFFFFFF;
    switch (cur_kind)
    {
        case 0:
            if ((character_pad.secondary_flags & 0x20) != 0)
            {
                val = player_interaction.flags;
                flags = character_pad.secondary_flags;
                player_interaction.flags = val & 0xFFFFFFDB;
                character_pad.secondary_flags = flags & 0xFFFFFFDF;
                if (*object_slot != 0)
                {
                    AnimatedObject::SpawnActionEffect(*object_slot, &character_pad, 6);
                    src = *object_slot;
                }
                else
                {
                    AnimatedObject::SpawnActionEffect(model_rec->object, &character_pad, 6);
                    src = model_rec->object;
                }
                AnimatedObject::BeginCharacterInteraction(src);
            }
            break;
        case 1:
            if ((character_pad.secondary_flags & 0x100) != 0)
            {
                character_pad.RemoveAction(6);
                character_pad.EndInteraction();
                character_pad.secondary_flags = character_pad.secondary_flags | 0x20;
            }
            goto clear_action;
        case 4:
        case 5:
        case 6:
        case 7:
        case 8:
        case 9:
        case 10:
        case 11:
        case 13:
        case 14:
            if ((character_pad.secondary_flags & 0x20) != 0)
            {
                val = player_interaction.flags;
                flags = character_pad.secondary_flags;
                player_interaction.flags = val & 0xFFFFFFDB;
                character_pad.secondary_flags = flags & 0xFFFFFFDF;
            }
            if ((character_pad.secondary_flags & 0x100) != 0)
            {
                character_pad.RemoveAction(6);
                character_pad.EndInteraction();
            }
        clear_action:
            val = character_pad.weapon_state;
            if ((val & 0xFFFF) != 0)
                character_pad.RemoveAction((val >> 16) != 0 ? (val >> 16) - 7 : 3);
            character_pad.weapon_state = 0;
            break;
        default:
            break;
    }
    src = *object_slot;
    if (src == 0)
    {
        character_pad.timers.idle_ticks = 900;
        goto assign_model;
    }
    table_rec = character_pad.model;
    if (table_rec != 0)
    {
        AnimationHeader *active_animation = src->animation;
        while (src->unbound_animation_word != 0 || active_animation != table_rec->animations[idx])
        {
            ++idx;
            if (idx >= 38)
                break;
        }
        if (idx >= 7)
            idx = (src->flags & 0x400) != 0 ? 6 : 2;
        if (idx == 38)
        {
            idx = 0;
            character_pad.timers.idle_ticks = 900;
        }
    }
    else
        character_pad.timers.idle_ticks = 900;
    src = *object_slot;
    dst = model_rec->object;
    dst->node.flags = src->node.flags;

    dst->pos = src->pos;
    dst->flags = src->flags;
    dst->path_flags = src->path_flags;

    dst->world_pos = src->world_pos;
    x = src->bounds.radius;
    y = src->bounds.height;
    z = src->bounds.depth;
    dst->bounds.radius = x;
    dst->bounds.height = y;
    dst->bounds.depth = z;
    dst->rotation = src->rotation;

    dst->velocity = src->velocity;
    CopyBytes((uint8 *)dst + offsetof(AnimatedObject, event_word), (const uint8 *)src + offsetof(AnimatedObject, event_word), 12);
    dst->movement_mode = src->movement_mode;
    dst->behavior = src->behavior;
    dst->contact_kind = src->contact_kind;
    dst->behavior_suspended = src->behavior_suspended;
    dst->ticks = src->ticks;
    dst->frame = src->frame;
    dst->path_definition = src->path_definition;
    dst->path_frame = src->path_frame;
    dst->yaw = src->yaw;
    dst->target_yaw = src->target_yaw;
    dst->pitch = src->pitch;
    dst->pitch_scale = src->pitch_scale;
    dst->path_motion_scale = src->path_motion_scale;
    for (uint32 sphere = 0; sphere < 4; ++sphere)
        dst->spheres[sphere] = src->spheres[sphere];
    dst->model_contacts = src->model_contacts;
assign_model:
{
    uint32 model_idx = kind;
    CharacterModelRecord *model_binding;
    model_binding = character_pad.model_slots[model_idx];
    table_rec = model_binding;
    character_pad.model = model_binding;
}
    dst = model_rec->object;
    dst->animation = table_rec->animations[idx];
    dst->unbound_animation_word = 0;
    dst = model_rec->object;
    dst->flags = dst->flags | 0x20000;
    dst = model_rec->object;
    dst->flags = dst->flags & 0xFBFEFFFF;
    *object_slot = model_rec->object;
    character_pad.object = model_rec->object;
    CopyBytes((uint8 *)&character_pad.movement, (const uint8 *)&table_rec->movement, 60);
    val = character_pad.movement.turn_frames;
    if ((sint32)val > 0)
        character_pad.turn_step = 2048 / (sint32)val;
    else
        character_pad.turn_step = 4096;
    return idx;
}

AnimatedObject *CharacterModelRecord::GetCharacterModelObject(void)
{
    FUNCTION_MARKER(0x800562F8, "SLUS_005.78");
    uint32 offset = (character_pad.motion.form_flags >> 18) & 0x3C;
    AnimatedObject *object;
    {
        CharacterModelRecord *sel_model;
        {
            uint32 model_idx = offset / 4;
            CharacterModelRecord *model_binding;
            model_binding = character_pad.model_slots[model_idx];
            sel_model = model_binding;
        }
        object = sel_model->object;
    }
    return object;
}

void CharacterModelRecord::ClearCharacterModels(void)
{
    FUNCTION_MARKER(0x80060EBC, "SLUS_005.78");
    hd_invalidate_geometry();
    CharacterModelRecord::ClearPrimaryModelSlots();
    /* 80034BA0 returns without effects */
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
