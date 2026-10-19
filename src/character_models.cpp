#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "game.h"
#include "original_defaults.h"

CharacterModelRecord character_model_headers[CharacterController::ModelCount];

uint32 HideAlternateCharacterModel(void)
{
    CharacterModelRecord *object_record;
    FUNCTION_MARKER(0x80044474, "SLUS_005.78");
    uint32 index, result;
    AnimatedObject *object;
    if ((sint32)character_controller.flags >= 0)
        return 1;
    index = 1 - ((uint16)(character_controller.motion.form_flags >> 16) & 15);
    {
        uint32 model_index = index;
        CharacterModelRecord *model_binding;
        if (model_index < CharacterController::ModelCount)
            model_binding = character_controller.model_slots[model_index];
        else
        {
            fprintf(stderr, "P2: Invalid character model index %u\n", model_index);
            abort();
        }
        if (model_binding == 0)
        {
            fprintf(stderr, "P2: Character model %u is not loaded\n", model_index);
            abort();
        }
        object_record = model_binding;
    }
    object = object_record->object;
    SceneNode *node = &((AnimatedObject *)(object))->node;
    result = node->flags | 1;
    node->flags = (uint8)result;
    return result;
}

void ReloadBaseCharacterModels(uint32 mode)
{
    CharacterModelRecord *object_record;
    AnimatedObject *loaded_object;
    SpriteRecord *sprite;
    FUNCTION_MARKER(0x80044508, "SLUS_005.78");
    AnimatedObject *objects[2];
    uint32 selector, first_asset, second_asset, asset;
    uint32 slot, index, value, selection;
    sint32 remaining;
    value = game_flow.mode_flags;
    first_asset = 23;
    second_asset = 29;
    remaining = 1;
    slot = 0;
    index = 0;
    if ((value & 0x200) != 0)
    {
        first_asset = 41;
        second_asset = 35;
    }
    character_controller.loaded_model_mask = mode;
    asset = first_asset + 1;
    selection = GetCurrentCharacterForm();
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
        asset = second_asset + 1;
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
        if (resource_archive.Size(asset) < sizeof(CharacterModelHeader))
        {
            fprintf(stderr, "P2: Truncated base character model header\n");
            abort();
        }
        uint32 metadata_bytes = resource_archive.Size(asset);
        uint8 *metadata_input = (uint8 *)malloc(metadata_bytes);
        if (metadata_input == 0)
        {
            fprintf(stderr, "P2: Cannot allocate character model metadata\n");
            abort();
        }
        resource_archive.ReadAsset(asset, metadata_input, 1, asset - 1);
        const CharacterModelHeader *input_header = (const CharacterModelHeader *)metadata_input;
        uint32 field;
        object_record = &character_model_headers[index];
        object_record->movement = input_header->movement;
        for (field = 0; field < 2; ++field)
            object_record->extent[field] = input_header->extent[field];
        for (field = 0; field < 3; ++field)
            object_record->motion_scale[field] = input_header->motion_scale[field];
        for (field = 0; field < CharacterModelRecord::AnimationCount; ++field)
        {
            if (input_header->animation_offsets[field] != 0)
            {
                fprintf(stderr, "P2: Character metadata contains a prebound animation address\n");
                abort();
            }
            object_record->animations[field] = 0;
        }
        object_record->object = 0;
        character_controller.model_slots[slot] = object_record;
        if ((sint32)resource_archive.Size(asset + 4) > 0)
        {
            uint32 sound_bytes = resource_archive.Size(asset + 4);
            if (sound_bytes < sizeof(SoundTableHeader))
            {
                fprintf(stderr, "P2: Truncated character sound table\n");
                abort();
            }
            uint8 *sound_input = (uint8 *)malloc(sound_bytes);
            if (sound_input == 0)
            {
                fprintf(stderr, "P2: Cannot allocate native sound table input\n");
                abort();
            }
            resource_archive.ReadAsset(asset + 4, sound_input, 1, asset - 1);
            SoundTableHeader *sound_table = (SoundTableHeader *)sound_input;
            sound_table->Relocate(sound_bytes, (SoundSample *)(sound_table + 1));
            free(sound_input);
        }
        {
            uint32 material_bytes = resource_archive.Size(asset + 3);
            if (material_bytes < sizeof(EncodedMaterialResourceHeader))
            {
                fprintf(stderr, "P2: Truncated nested material asset\n");
                abort();
            }
            uint8 *material_input = (uint8 *)malloc(material_bytes);
            if (material_input == 0)
            {
                fprintf(stderr, "P2: Cannot allocate nested material input\n");
                abort();
            }
            resource_archive.ReadAsset(asset + 3, material_input, 1, asset - 1);
            RelocateMaterialResource(&object_record->material_resource, material_input, material_bytes, -1);
            free(material_input);
        }
        uint32 model_bytes = resource_archive.Size(asset + 1);
        if (model_bytes < 232u)
        {
            fprintf(stderr, "P2: Model prototype asset is shorter than its root\n");
            abort();
        }
        uint8 *model_data = (uint8 *)malloc(model_bytes);
        if (model_data == 0)
        {
            fprintf(stderr, "P2: Cannot allocate native model prototype\n");
            abort();
        }
        resource_archive.ReadAsset(asset + 1, model_data, 1, asset - 1);
        free(object_record->model_vertices);
        object_record->model_vertices = 0;
        free(object_record->model_polygons);
        object_record->model_polygons = 0;
        free(object_record->model_data);
        object_record->model_data = model_data;
        object_record->model_bytes = model_bytes;
        object_record->object = 0;
        xport_store_le32(model_data + 28, xport_load_le32(model_data + 28) | 0x20034);
        BindModelMaterials(&object_record->object, &object_record->model_data, &object_record->material_resource, object_record->model_bytes, &object_record->model_polygons, &object_record->model_vertices);
        objects[index] = object_record->object;
        loaded_object = (AnimatedObject *)(object_record->object);
        value = (uint32)object_record->extent[0];
        sprite = (SpriteRecord *)(((SpriteRecord *)(loaded_object->sprite)));
        sprite->extent[0] = (sint16)((sint32)value >> 5);
        loaded_object = (AnimatedObject *)(object_record->object);
        value = (uint32)object_record->extent[1];
        sprite = (SpriteRecord *)(((SpriteRecord *)(loaded_object->sprite)));
        sprite->extent[1] = (sint16)((sint32)value >> 5);
        value = resource_archive.Offset(asset + 2);
        loaded_object = object_record->object;
        ++index;
        uint32 animation_bytes = resource_archive.Size(asset + 2);
        if (animation_bytes < 4)
        {
            fprintf(stderr, "P2: Invalid model animation asset extent\n");
            abort();
        }
        uint8 *animation_data = (uint8 *)malloc(animation_bytes);
        AnimationHeader *header_storage = (AnimationHeader *)calloc(CharacterModelRecord::AnimationCount, sizeof(AnimationHeader));
        if (animation_data == 0 || header_storage == 0)
        {
            fprintf(stderr, "P2: Cannot allocate native model animation bank\n");
            abort();
        }
        resource_archive.ReadAsset(asset + 2, animation_data, 1, asset - 1);
        free(object_record->animation_data);
        if (object_record->animation_headers != 0)
        {
            for (uint32 header_index = 0; header_index < 38; ++header_index)
            {
                for (uint32 track_group = 0; track_group < 4; ++track_group)
                    free(object_record->animation_headers[header_index].tracks[track_group].bytes);
            }
        }
        free(object_record->animation_headers);
        free(object_record->morph_restoration);
        object_record->animation_data = animation_data;
        object_record->animation_headers = header_storage;
        object_record->animation_bytes = animation_bytes;
        LoadAnimationTable(loaded_object, animation_data, animation_bytes, object_record->animations, CharacterModelRecord::AnimationCount, header_storage, &object_record->morph_restoration);
        free(metadata_input);
        ++slot;
        --remaining;
        asset = second_asset + 1;
    }
    selection = selector;
    character_controller.object = objects[selection];
finish:
    return;
}

CharacterModelRecord *RestoreCharacterModelAnimations(void)
{
    CharacterModelRecord *result_record;
    CharacterModelRecord *target_record;
    FUNCTION_MARKER(0x80055280, "SLUS_005.78");
    AnimationHeader *first, *second;
    CharacterModelRecord *result = 0;
    character_controller.ClearTransientFlags();
    if ((character_controller.secondary_flags & 0x40) != 0)
    {
        target_record = character_controller.model;
        first = target_record->animations[22];
        second = target_record->animations[2];
        target_record->animations[2] = first;
        result_record = character_controller.model;
        result = result_record;
        result_record->animations[22] = second;
    }
    return result;
}

uint32 LoadCharacterModel(uint32 index)
{
    CharacterModelRecord *object_record;
    AnimatedObject *loaded_object;
    SpriteRecord *sprite;
    uint32 result;
    FUNCTION_MARKER(0x800552FC, "SLUS_005.78");
    if (resource_archive.Size(resource_load_cursor.asset) < sizeof(CharacterModelHeader))
    {
        fprintf(stderr, "P2: Truncated character model header\n");
        abort();
    }
    if (index >= CharacterController::ModelCount)
    {
        fprintf(stderr, "P2: Invalid character model load index %u\n", index);
        abort();
    }
    uint32 metadata_bytes = resource_archive.Size(resource_load_cursor.asset);
    uint8 *metadata_input = (uint8 *)malloc(metadata_bytes);
    if (metadata_input == 0)
    {
        fprintf(stderr, "P2: Cannot allocate character model metadata\n");
        abort();
    }
    resource_archive.ReadAsset(resource_load_cursor.asset, metadata_input, 1, resource_load_cursor.container_asset);
    const CharacterModelHeader *input_header = (const CharacterModelHeader *)metadata_input;
    uint32 field;
    object_record = &character_model_headers[index];
    object_record->movement = input_header->movement;
    for (field = 0; field < 2; ++field)
        object_record->extent[field] = input_header->extent[field];
    for (field = 0; field < 3; ++field)
        object_record->motion_scale[field] = input_header->motion_scale[field];
    for (field = 0; field < CharacterModelRecord::AnimationCount; ++field)
    {
        if (input_header->animation_offsets[field] != 0)
        {
            fprintf(stderr, "P2: Character metadata contains a prebound animation address\n");
            abort();
        }
        object_record->animations[field] = 0;
    }
    object_record->object = 0;
    character_controller.model_slots[index] = object_record;
    if ((sint32)resource_archive.Size(resource_load_cursor.asset + 4) > 0)
    {
        uint32 sound_bytes = resource_archive.Size(resource_load_cursor.asset + 4);
        if (sound_bytes < sizeof(SoundTableHeader))
        {
            fprintf(stderr, "P2: Truncated character sound table\n");
            abort();
        }
        uint8 *sound_input = (uint8 *)malloc(sound_bytes);
        if (sound_input == 0)
        {
            fprintf(stderr, "P2: Cannot allocate native sound table input\n");
            abort();
        }
        resource_archive.ReadAsset(resource_load_cursor.asset + 4, sound_input, 1, resource_load_cursor.container_asset);
        SoundTableHeader *sound_table = (SoundTableHeader *)sound_input;
        sound_table->Relocate(sound_bytes, (SoundSample *)(sound_table + 1));
        free(sound_input);
    }
    {
        uint32 material_bytes = resource_archive.Size(resource_load_cursor.asset + 3);
        if (material_bytes < sizeof(EncodedMaterialResourceHeader))
        {
            fprintf(stderr, "P2: Truncated nested material asset\n");
            abort();
        }
        uint8 *material_input = (uint8 *)malloc(material_bytes);
        if (material_input == 0)
        {
            fprintf(stderr, "P2: Cannot allocate nested material input\n");
            abort();
        }
        resource_archive.ReadAsset(resource_load_cursor.asset + 3, material_input, 1, resource_load_cursor.container_asset);
        RelocateMaterialResource(&object_record->material_resource, material_input, material_bytes, -1);
        free(material_input);
    }
    uint32 model_bytes = resource_archive.Size(resource_load_cursor.asset + 1);
    if (model_bytes < 232u)
    {
        fprintf(stderr, "P2: Model prototype asset is shorter than its root\n");
        abort();
    }
    uint8 *model_data = (uint8 *)malloc(model_bytes);
    if (model_data == 0)
    {
        fprintf(stderr, "P2: Cannot allocate native model prototype\n");
        abort();
    }
    resource_archive.ReadAsset(resource_load_cursor.asset + 1, model_data, 1, resource_load_cursor.container_asset);
    free(object_record->model_vertices);
    object_record->model_vertices = 0;
    free(object_record->model_polygons);
    object_record->model_polygons = 0;
    free(object_record->model_data);
    object_record->model_data = model_data;
    object_record->model_bytes = model_bytes;
    object_record->object = 0;
    xport_store_le32(model_data + 28, xport_load_le32(model_data + 28) | 0x20034);
    BindModelMaterials(&object_record->object, &object_record->model_data, &object_record->material_resource, object_record->model_bytes, &object_record->model_polygons, &object_record->model_vertices);
    loaded_object = ((AnimatedObject *)(object_record->object));
    sprite = (SpriteRecord *)(((SpriteRecord *)(loaded_object->sprite)));
    sprite->extent[0] = (sint16)(object_record->extent[0] >> 5);
    loaded_object = ((AnimatedObject *)(object_record->object));
    sprite = (SpriteRecord *)(((SpriteRecord *)(loaded_object->sprite)));
    sprite->extent[1] = (sint16)(object_record->extent[1] >> 5);
    uint32 animation_bytes = resource_archive.Size(resource_load_cursor.asset + 2);
    if (animation_bytes < 4)
    {
        fprintf(stderr, "P2: Invalid model animation asset extent\n");
        abort();
    }
    uint8 *animation_data = (uint8 *)malloc(animation_bytes);
    AnimationHeader *header_storage = (AnimationHeader *)calloc(CharacterModelRecord::AnimationCount, sizeof(AnimationHeader));
    if (animation_data == 0 || header_storage == 0)
    {
        fprintf(stderr, "P2: Cannot allocate native model animation bank\n");
        abort();
    }
    resource_archive.ReadAsset(resource_load_cursor.asset + 2, animation_data, 1, resource_load_cursor.container_asset);
    free(object_record->animation_data);
    if (object_record->animation_headers != 0)
    {
        for (uint32 header_index = 0; header_index < 38; ++header_index)
        {
            for (uint32 track_group = 0; track_group < 4; ++track_group)
                free(object_record->animation_headers[header_index].tracks[track_group].bytes);
        }
    }
    free(object_record->animation_headers);
    free(object_record->morph_restoration);
    object_record->animation_data = animation_data;
    object_record->animation_headers = header_storage;
    object_record->animation_bytes = animation_bytes;
    LoadAnimationTable(object_record->object, animation_data, animation_bytes, object_record->animations, CharacterModelRecord::AnimationCount, header_storage, &object_record->morph_restoration);
    free(metadata_input);
    result = resource_load_cursor.asset + 5;
    resource_load_cursor.asset = result;
    return result;
}

uint32 ResetCharacterModels(void)
{
    FUNCTION_MARKER(0x800556E0, "SLUS_005.78");
    uint32 flags, value, index;
    CharacterModelRecord **entry;
    AnimatedObject *object = character_controller.object;
    if (object != 0)
    {
        character_controller.ClearTransientFlags();
        character_controller.flags = character_controller.flags & 0xC0000000;
        flags = object->flags;
        value = object->path_flags;
        object->flags = flags & 0xC0F793FF;
        object->path_flags = value & 0xFC000000;
        flags = character_controller.secondary_flags;
        character_controller.motion.jump_phase = 0;
        character_controller.motion.vertical_speed = 0;
        character_controller.motion.horizontal_speed = 0;
        character_controller.secondary_flags = flags & 0xFFFFF932;
        object->velocity[2] = 0;
        object->velocity[1] = 0;
        object->velocity[0] = 0;
        character_controller.timers.special_move_ticks = 0;
        character_controller.timers.transition_ticks = 0;
        character_controller.timers.score_bonus_ticks = 0;
        character_controller.timers.action_cooldown_ticks = 0;
        character_controller.vitals.damage_ticks = 0;
        character_controller.timers.state_ticks = 0;
        character_controller.section_height_offset = 0;
        character_controller.timers.contact_impulse_ticks = 0;
        character_controller.motion.environment_acceleration = 0;
    }
    entry = character_controller.model_slots + 2;
    index = 2;
    do
    {
        if (*entry != 0)
            *entry = 0;
        value = character_controller.motion.form_flags;
        if (index == ((value >> 16) & 15))
            AssignCharacterModel(&character_controller.object, (value >> 20) & 15);
        ++index;
        ++entry;
    } while (index < 15);
    return 0;
}

uint32 SelectCharacterForm(uint32 mode)
{
    FUNCTION_MARKER(0x800558AC, "SLUS_005.78");
    uint32 flags = character_controller.motion.form_flags;
    uint32 value = ((flags & 0xFFF0FFFF) | (mode << 16)) & 0xFF0FFFFF;
    character_controller.motion.form_flags = value | (mode << 20);
    return (flags >> 20) & 0xF;
}

uint32 InitializeCharacterModels(void)
{
    uint32 index;
    uint32 next;
    FUNCTION_MARKER(0x80055AC4, "SLUS_005.78");
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
    gameplay_input.rotation_track_shift = (uint8)(0);
    screen_render.fade_level = 0;
    {
        ObjectCallback operation = {0, &AnimatedObject::UpdateCharacterCameraTransition};
        object_operations.behaviors[2] = operation;
    }
    InitializeContactPool();
    game_flow.transition_flags = game_flow.transition_flags | 0x10000;
    for (index = 0; index < 15; ++index)
    {
        if ((sint32)resource_archive.Size(resource_load_cursor.asset) > 0)
        {
            next = resource_load_cursor.asset + 2;
            uint32 model_bytes = resource_archive.Size(resource_load_cursor.asset);
            if (model_bytes < 232u)
            {
                fprintf(stderr, "P2: Model prototype asset is shorter than its root\n");
                abort();
            }
            uint8 *model_data = (uint8 *)malloc(model_bytes);
            if (model_data == 0)
            {
                fprintf(stderr, "P2: Cannot allocate native model prototype\n");
                abort();
            }
            resource_archive.ReadAsset(resource_load_cursor.asset, model_data, 1, resource_load_cursor.container_asset);
            free(action_effect_resources.model_vertices[index]);
            action_effect_resources.model_vertices[index] = 0;
            free(action_effect_resources.model_polygons[index]);
            action_effect_resources.model_polygons[index] = 0;
            free(action_effect_resources.model_data[index]);
            action_effect_resources.model_data[index] = model_data;
            action_effect_resources.model_bytes[index] = model_bytes;
            action_effect_resources.templates[index] = 0;
            {
                uint32 material_bytes = resource_archive.Size(next);
                if (material_bytes < sizeof(EncodedMaterialResourceHeader))
                {
                    fprintf(stderr, "P2: Truncated nested material asset\n");
                    abort();
                }
                uint8 *material_input = (uint8 *)malloc(material_bytes);
                if (material_input == 0)
                {
                    fprintf(stderr, "P2: Cannot allocate nested material input\n");
                    abort();
                }
                resource_archive.ReadAsset(next, material_input, 1, resource_load_cursor.container_asset);
                RelocateMaterialResource(&action_effect_resources.material_resources[index], material_input, material_bytes, -1);
                free(material_input);
            }
            AnimatedObject *effect;
            uint32 flags = xport_load_le32(model_data + 28);
            uint8 node_flags = model_data[13];
            xport_store_le32(model_data + 28, flags | 0x20024);
            model_data[13] = node_flags | 0x80;
            BindModelMaterials(&action_effect_resources.templates[index], &action_effect_resources.model_data[index], &action_effect_resources.material_resources[index], action_effect_resources.model_bytes[index], &action_effect_resources.model_polygons[index], &action_effect_resources.model_vertices[index]);
            effect = action_effect_resources.templates[index];
            uint32 animation_bytes = resource_archive.Size(resource_load_cursor.asset + 1);
            if (animation_bytes < sizeof(EncodedAnimationHeader))
            {
                fprintf(stderr, "P2: Invalid attached animation asset extent\n");
                abort();
            }
            uint8 *animation_data = (uint8 *)malloc(animation_bytes);
            AnimationHeader *header_storage = (AnimationHeader *)calloc(1, sizeof(AnimationHeader));
            if (animation_data == 0 || header_storage == 0)
            {
                fprintf(stderr, "P2: Cannot allocate native attached animation\n");
                abort();
            }
            resource_archive.ReadAsset(resource_load_cursor.asset + 1, animation_data, 1, resource_load_cursor.container_asset);
            free(action_effect_resources.animation_data[index]);
            if (action_effect_resources.animation_headers[index] != 0)
            {
                for (uint32 header_index = 0; header_index < 1; ++header_index)
                {
                    for (uint32 track_group = 0; track_group < 4; ++track_group)
                        free(action_effect_resources.animation_headers[index][header_index].tracks[track_group].bytes);
                }
            }
            free(action_effect_resources.animation_headers[index]);
            action_effect_resources.animation_data[index] = animation_data;
            action_effect_resources.animation_headers[index] = header_storage;
            action_effect_resources.animation_bytes[index] = animation_bytes;
            effect->AttachAnimation(animation_data, animation_bytes, header_storage);
        }
        next = resource_load_cursor.asset + 3;
        resource_load_cursor.asset = next;
    }
    LoadCharacterModel(0);
    LoadCharacterModel(1);
    character_controller.loaded_model_mask = 3;
    ResetNewGameCharacter();
    saved_material_registry = material_registry;
    {
        ObjectCallback operation = {0, &AnimatedObject::UpdateActionObject};
        object_operations.behaviors[26] = operation;
    }
    return 0x8002995C;
}

static void ClearPrimaryModelSlots(void)
{
    FUNCTION_MARKER(0x80055D6C, "SLUS_005.78");
    uint32 index;
    for (index = 0; index < 2; ++index)
    {
        CharacterModelRecord **slot = character_controller.model_slots + index;
        if (*slot != 0)
            *slot = 0;
    }
    character_controller.object = 0;
}

uint32 AssignCharacterModel(AnimatedObject **object_slot, uint32 kind)
{
    CharacterModelRecord *model_record;
    CharacterModelRecord *table_record;
    FUNCTION_MARKER(0x80055DC0, "SLUS_005.78");
    {
        uint32 model_index = kind;
        CharacterModelRecord *model_binding;
        if (model_index < CharacterController::ModelCount)
            model_binding = character_controller.model_slots[model_index];
        else
        {
            fprintf(stderr, "P2: Invalid character model index %u\n", model_index);
            abort();
        }
        if (model_binding == 0)
        {
            fprintf(stderr, "P2: Character model %u is not loaded\n", model_index);
            abort();
        }
        model_record = model_binding;
    }
    uint32 flags, current_kind, value, index = 0;
    AnimatedObject *source, *destination;
    uint32 x, y, z;
    {
        ObjectCallback operation = {0, kind == 5 ? &AnimatedObject::UpdateForm5Movement : &AnimatedObject::UpdateCharacterMovement};
        object_operations.motions[1] = operation;
    }
    value = (character_controller.motion.form_flags & 0xFFF0FFFF) | (kind << 16);
    character_controller.motion.form_flags = (value & 0xF0FFFFFF) | 0x0F000000;
    flags = character_controller.flags;
    current_kind = (uint16)(character_controller.motion.form_flags >> 16) & 0xF;
    character_controller.flags = flags & 0xBFFFFFFF;
    switch (current_kind)
    {
        case 0:
            if ((character_controller.secondary_flags & 0x20) != 0)
            {
                value = player_interaction.flags;
                flags = character_controller.secondary_flags;
                player_interaction.flags = value & 0xFFFFFFDB;
                character_controller.secondary_flags = flags & 0xFFFFFFDF;
                if (*object_slot != 0)
                {
                    SpawnActionEffect(*object_slot, &character_controller, 6);
                    source = *object_slot;
                }
                else
                {
                    SpawnActionEffect(model_record->object, &character_controller, 6);
                    source = model_record->object;
                }
                BeginCharacterInteraction(source);
            }
            break;
        case 1:
            if ((character_controller.secondary_flags & 0x100) != 0)
            {
                character_controller.RemoveAction(6);
                character_controller.EndInteraction();
                character_controller.secondary_flags = character_controller.secondary_flags | 0x20;
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
            if ((character_controller.secondary_flags & 0x20) != 0)
            {
                value = player_interaction.flags;
                flags = character_controller.secondary_flags;
                player_interaction.flags = value & 0xFFFFFFDB;
                character_controller.secondary_flags = flags & 0xFFFFFFDF;
            }
            if ((character_controller.secondary_flags & 0x100) != 0)
            {
                character_controller.RemoveAction(6);
                character_controller.EndInteraction();
            }
        clear_action:
            value = character_controller.weapon_state;
            if ((value & 0xFFFF) != 0)
                character_controller.RemoveAction((value >> 16) != 0 ? (value >> 16) - 7 : 3);
            character_controller.weapon_state = 0;
            break;
        default:
            break;
    }
    source = *object_slot;
    if (source == 0)
    {
        character_controller.timers.idle_ticks = 900;
        goto assign_model;
    }
    table_record = character_controller.model;
    if (table_record != 0)
    {
        AnimationHeader *active_animation = source->animation;
        while (source->unbound_animation_word != 0 || active_animation != table_record->animations[index])
        {
            ++index;
            if (index >= 38)
                break;
        }
        if (index >= 7)
            index = (source->flags & 0x400) != 0 ? 6 : 2;
        if (index == 38)
        {
            index = 0;
            character_controller.timers.idle_ticks = 900;
        }
    }
    else
        character_controller.timers.idle_ticks = 900;
    source = *object_slot;
    destination = ((AnimatedObject *)(model_record->object));
    destination->node.flags = source->node.flags;
    x = source->position[0];
    y = source->position[1];
    z = source->position[2];
    destination->position[0] = x;
    destination->position[1] = y;
    destination->position[2] = z;
    destination->flags = source->flags;
    destination->path_flags = source->path_flags;
    x = source->world_position[0];
    y = source->world_position[1];
    z = source->world_position[2];
    destination->world_position[0] = x;
    destination->world_position[1] = y;
    destination->world_position[2] = z;
    x = source->metadata_words72[0];
    y = source->metadata_words72[1];
    z = source->metadata_words72[2];
    destination->metadata_words72[0] = x;
    destination->metadata_words72[1] = y;
    destination->metadata_words72[2] = z;
    x = source->orientation_words[0];
    y = source->orientation_words[1];
    destination->orientation_words[0] = x;
    destination->orientation_words[1] = y;
    x = source->velocity[0];
    y = source->velocity[1];
    z = source->velocity[2];
    destination->velocity[0] = x;
    destination->velocity[1] = y;
    destination->velocity[2] = z;
    CopyBytes((uint8 *)destination + offsetof(AnimatedObject, event_word), (const uint8 *)source + offsetof(AnimatedObject, event_word), 12);
    destination->movement_mode = source->movement_mode;
    destination->behavior = source->behavior;
    destination->metadata126[0] = source->metadata126[0];
    destination->metadata126[1] = source->metadata126[1];
    destination->ticks = source->ticks;
    destination->frame = source->frame;
    destination->path_definition = (source->path_definition);
    destination->path_frame = source->path_frame;
    destination->yaw = source->yaw;
    destination->target_yaw = source->target_yaw;
    destination->pitch = source->pitch;
    destination->pitch_scale = source->pitch_scale;
    destination->path_motion_scale = source->path_motion_scale;
    CopyBytes((uint8 *)destination->spheres, (const uint8 *)source->spheres, 64);
assign_model:
{
    uint32 model_index = kind;
    CharacterModelRecord *model_binding;
    if (model_index < CharacterController::ModelCount)
        model_binding = character_controller.model_slots[model_index];
    else
    {
        fprintf(stderr, "P2: Invalid character model index %u\n", model_index);
        abort();
    }
    if (model_binding == 0)
    {
        fprintf(stderr, "P2: Character model %u is not loaded\n", model_index);
        abort();
    }
    table_record = model_binding;
    character_controller.model = model_binding;
}
    destination = ((AnimatedObject *)(model_record->object));
    destination->animation = table_record->animations[index];
    destination->unbound_animation_word = 0;
    destination = ((AnimatedObject *)(model_record->object));
    destination->flags = destination->flags | 0x20000;
    destination = ((AnimatedObject *)(model_record->object));
    destination->flags = destination->flags & 0xFBFEFFFF;
    *object_slot = model_record->object;
    character_controller.object = model_record->object;
    CopyBytes((uint8 *)&character_controller.movement, (const uint8 *)&table_record->movement, 60);
    value = (uint32)character_controller.movement.turn_frames;
    if ((sint32)value > 0)
        character_controller.turn_step = 2048 / (sint32)value;
    else
        character_controller.turn_step = 4096;
    return index;
}

AnimatedObject *GetCharacterModelObject(void)
{
    FUNCTION_MARKER(0x800562F8, "SLUS_005.78");
    uint32 offset = (character_controller.motion.form_flags >> 18) & 0x3C;
    AnimatedObject *object;
    {
        CharacterModelRecord *selected_model;
        {
            uint32 model_index = offset / 4;
            CharacterModelRecord *model_binding;
            if (model_index < CharacterController::ModelCount)
                model_binding = character_controller.model_slots[model_index];
            else
            {
                fprintf(stderr, "P2: Invalid character model index %u\n", model_index);
                abort();
            }
            if (model_binding == 0)
            {
                fprintf(stderr, "P2: Character model %u is not loaded\n", model_index);
                abort();
            }
            selected_model = model_binding;
        }
        object = selected_model->object;
    }
    return object;
}

void ClearCharacterModels(void)
{
    FUNCTION_MARKER(0x80060EBC, "SLUS_005.78");
    ClearPrimaryModelSlots();
    /* 80034BA0 returns without effects */
    for (uint32 group_index = ModelGroupRegistry::CommonCount; group_index < ModelGroupRegistry::Capacity; ++group_index)
    {
        for (uint32 model_index = 0; model_index < 4; ++model_index)
        {
            free(model_groups.records[group_index].material_resources[model_index].textures);
            free(model_groups.records[group_index].material_resources[model_index].materials);
            free(model_groups.records[group_index].material_resources[model_index].data);
            memset(&model_groups.records[group_index].material_resources[model_index], 0, sizeof(model_groups.records[group_index].material_resources[model_index]));
            free(model_groups.records[group_index].model_vertices[model_index]);
            model_groups.records[group_index].model_vertices[model_index] = 0;
            free(model_groups.records[group_index].model_polygons[model_index]);
            model_groups.records[group_index].model_polygons[model_index] = 0;
            free(model_groups.records[group_index].model_data[model_index]);
            model_groups.records[group_index].model_data[model_index] = 0;
            model_groups.records[group_index].model_bytes[model_index] = 0;
            free(model_groups.records[group_index].animation_data[model_index]);
            if (model_groups.records[group_index].animation_headers[model_index] != 0)
            {
                for (uint32 header_index = 0; header_index < 1; ++header_index)
                {
                    for (uint32 track_group = 0; track_group < 4; ++track_group)
                        free(model_groups.records[group_index].animation_headers[model_index][header_index].tracks[track_group].bytes);
                }
            }
            free(model_groups.records[group_index].animation_headers[model_index]);
            model_groups.records[group_index].animation_data[model_index] = 0;
            model_groups.records[group_index].animation_headers[model_index] = 0;
            model_groups.records[group_index].animation_bytes[model_index] = 0;
        }
    }
    model_groups.count = 0;
}

void AnimatedObject::UpdateCharacterForm()
{
    FUNCTION_MARKER(0x8004FD44, "SLUS_005.78");
    const ScenePath *camera_path, *object_path;

    union
    {
        VECTOR value;
        uint32 words[4];
    } position, input;

    AnimatedObject *object_slot = this;
    AnimatedObject *current;
    const PathDefinition *definition;
    const MATRIX *matrix;
    uint32 value, flags, scene, x, y, z, result, kind;
    if ((sint32)character_controller.flags >= 0 || (gameplay_input.buttons & 0x10) == 0 || (gameplay_input.prev_buttons & 0x10) != 0)
        goto check_form;
    value = character_controller.motion.form_flags;
    if (((value >> 16) & 15) >= 2)
        goto check_form;
    if (((value >> 24) & 15) != 15)
        goto change_form;
    if ((game_flow.transition_flags & 0x48) != 0)
        goto check_form;
    value &= 0xFF0FFFFF;
    value |= (1 - ((value >> 16) & 15)) << 20;
    value &= 0xF0FFFFFF;
    flags = character_controller.flags;
    value |= (value >> 20) << 24;
    character_controller.motion.form_flags = value;
    if ((flags & 0x80000000) == 0)
        goto check_form;
    character_controller.ClearTransientFlags();
    value = character_controller.motion.form_flags;
    character_controller.motion.jump_phase = 0;
    character_controller.motion.vertical_speed = 0;
    character_controller.motion.horizontal_speed = 0;
    value = ((value & 0xF0FFFFFF) | 0xF000000) & 0xFFF0FFFF;
    current = (AnimatedObject *)(object_slot);
    value |= ((value >> 20) & 15) << 16;
    character_controller.motion.form_flags = value;
    current->movement_mode = 0;
    current = (AnimatedObject *)(object_slot);
    current->metadata126[0] = 0;
    scene = OriginalDefaults::SceneContextIdentity;
    object_path = camera_motion.path;
    current = (AnimatedObject *)(object_slot);
    current->scene_path = object_path;
    value = (character_controller.motion.form_flags >> 18) & 0x3C;
    {
        CharacterModelRecord *selected_model;
        {
            uint32 model_index = value / 4;
            CharacterModelRecord *model_binding;
            if (model_index < CharacterController::ModelCount)
                model_binding = character_controller.model_slots[model_index];
            else
            {
                fprintf(stderr, "P2: Invalid character model index %u\n", model_index);
                abort();
            }
            if (model_binding == 0)
            {
                fprintf(stderr, "P2: Character model %u is not loaded\n", model_index);
                abort();
            }
            selected_model = model_binding;
        }
        object_slot = selected_model->object;
    }
    current = (AnimatedObject *)(object_slot);
    current->movement_mode = 1;
    current = (AnimatedObject *)(object_slot);
    current->metadata126[0] = 1;
    current = (AnimatedObject *)(object_slot);
    current->node.flags = current->node.flags & 0xFE;
    current = (AnimatedObject *)(object_slot);
    if ((current->flags & 0x2000) != 0)
        goto local_position;
    definition = (current->path_definition);
    if (definition == 0 || (definition->transform) == 0)
        goto local_position;
    x = current->position[0];
    y = current->position[1];
    z = current->position[2];
    input.words[0] = x;
    input.words[1] = y;
    input.words[2] = z;
    PushMatrix();
    current = (AnimatedObject *)(object_slot);
    definition = (const PathDefinition *)((const PathDefinition *)((current->path_definition)));
    matrix = (const MATRIX *)((definition->transform) != 0 ? &(definition->transform)->matrix : (MATRIX *)0);
    x = xport_load_le32((const uint8 *)matrix);
    y = xport_load_le32((const uint8 *)matrix + 4);
    xport_gte_write_control(0, x);
    xport_gte_write_control(1, y);
    x = xport_load_le32((const uint8 *)matrix + 8);
    y = xport_load_le32((const uint8 *)matrix + 12);
    z = xport_load_le32((const uint8 *)matrix + 16);
    xport_gte_write_control(2, x);
    xport_gte_write_control(3, y);
    xport_gte_write_control(4, z);
    ApplyRotMatrixLV(&input.value, &position.value);
    PopMatrix();
    current = (AnimatedObject *)(object_slot);
    definition = (const PathDefinition *)((const PathDefinition *)((current->path_definition)));
    matrix = (const MATRIX *)((definition->transform) != 0 ? &(definition->transform)->matrix : (MATRIX *)0);
    x = position.words[0];
    position.words[0] = x + (uint32)matrix->t[0];
    definition = (const PathDefinition *)((const PathDefinition *)((current->path_definition)));
    matrix = (const MATRIX *)((definition->transform) != 0 ? &(definition->transform)->matrix : (MATRIX *)0);
    y = position.words[1];
    position.words[1] = y + (uint32)matrix->t[1];
    definition = (const PathDefinition *)((const PathDefinition *)((current->path_definition)));
    matrix = (const MATRIX *)((definition->transform) != 0 ? &(definition->transform)->matrix : (MATRIX *)0);
    z = position.words[2];
    position.words[2] = z + (uint32)matrix->t[2];
    goto position_ready;
local_position:
    current = (AnimatedObject *)(object_slot);
    x = current->position[0];
    y = current->position[1];
    z = current->position[2];
    position.words[0] = x;
    position.words[1] = y;
    position.words[2] = z;
position_ready:
    position.words[1] = position.words[1] + 12288;
    SpawnEffectSprite(position.words, 10240, 7);
    QueueCharacterActionSound(14, 3);
    current = (AnimatedObject *)(object_slot);
    flags = current->flags;
    object_path = (current->scene_path);
    current->flags = flags & 0xFFFFF7FF;
    SelectScenePath(object_path);
    scene = OriginalDefaults::SceneContextIdentity;
    x = camera_motion.focus_position[0];
    y = camera_motion.focus_position[1];
    z = camera_motion.focus_position[2];
    camera_motion.target[0] = x;
    camera_motion.target[1] = y;
    camera_motion.target[2] = z;
    scene = OriginalDefaults::SceneContextIdentity;
    current = (AnimatedObject *)(object_slot);
    x = current->position[0];
    y = current->position[1];
    z = current->position[2];
    camera_motion.focus_position[0] = x;
    camera_motion.focus_position[1] = y;
    camera_motion.focus_position[2] = z;
    scene = OriginalDefaults::SceneContextIdentity;
    current = (AnimatedObject *)(object_slot);
    camera_path = camera_motion.path;
    flags = current->flags;
    ((const ScenePath *)(camera_path))->ProjectOffset(current->path_frame, (uint32 *)camera_motion.desired_position, flags & 0x10);
    scene = OriginalDefaults::SceneContextIdentity;
    x = camera_motion.target[0];
    y = camera_motion.target[1];
    z = camera_motion.target[2];
    camera_motion.focus_position[0] = x;
    camera_motion.focus_position[1] = y;
    camera_motion.focus_position[2] = z;
    scene = OriginalDefaults::SceneContextIdentity;
    camera_motion.smooth_ticks[2] = (uint8)(15);
    camera_motion.smooth_ticks[1] = (uint8)(15);
    camera_motion.smooth_ticks[0] = (uint8)(15);
    current = (AnimatedObject *)(object_slot);
    x = current->position[0];
    y = current->position[1];
    z = current->position[2];
    camera_motion.target[0] = x;
    camera_motion.target[1] = y;
    camera_motion.target[2] = z;
    flags = character_controller.flags;
    y = camera_motion.target[1];
    camera_motion.target[1] = y + ((sint32)flags >= 0 ? 14336 : 32768);
    value = game_flow.transition_flags;
    flags = character_controller.secondary_flags;
    camera_motion.target_ticks = 15;
    game_flow.transition_flags = value | 0x100;
    if ((flags & 0x20) != 0)
    {
        BeginCharacterInteraction(object_slot);
        flags = character_controller.secondary_flags;
        value = game_flow.mode_flags;
        character_controller.secondary_flags = flags & 0xFFFFFFDF;
        if ((value & 0x200) != 0)
            goto ensure_form;
        SpawnActionEffect(object_slot, &character_controller, 6);
    }
    else if ((flags & 0x100) != 0)
    {
        character_controller.secondary_flags = (flags | 0x20) & 0xFFFFFEFF;
        character_controller.RemoveAction(6);
        character_controller.EndInteraction();
    }
    result = game_flow.mode_flags & 0x200;
    if (result == 0)
        goto done;
ensure_form:
    result = (uint16)(character_controller.motion.form_flags >> 16) & 15;
    if (result == 0)
    {
        result = character_controller.secondary_flags & 0x100;
        if (result == 0)
            result = BeginCharacterInteraction(object_slot);
    }
    goto done;
check_form:
    result = (uint8)(character_controller.motion.form_flags >> 24) & 15;
    if (result == 15)
        goto done;
change_form:
    value = character_controller.motion.form_flags;
    kind = (value >> 24) & 15;
    result = (value >> 16) & 15;
    if (kind == result)
        goto done;
    (object_slot != 0 ? &object_slot->node : 0)->Unregister();
    kind = (uint8)(character_controller.motion.form_flags >> 24) & 15;
    kind = AssignCharacterModel(&object_slot, kind);
    (object_slot != 0 ? &object_slot->node : 0)->Register();
    current = (AnimatedObject *)(object_slot);
    flags = current->flags;
    if ((flags & 0x8000000) != 0 && character_controller.model->animations[7] == 0)
    {
        current->flags = flags & 0xF7FFFFFF;
        current = (AnimatedObject *)(object_slot);
        flags = current->flags;
    }
    if ((flags & 0x1000000) != 0 && character_controller.model->animations[5] == 0)
        current->flags = flags & 0xFEFFFFFF;
    current = (AnimatedObject *)(object_slot);
    if ((current->flags & 0x400) != 0 && character_controller.model->animations[3] == 0)
    {
        value = character_controller.movement.jump_ticks;
        flags = character_controller.flags;
        character_controller.motion.jump_phase = -1;
        character_controller.motion.vertical_speed = 0;
        character_controller.timers.state_ticks = 0;
        character_controller.timers.jump_duration_ticks = value;
        character_controller.flags = flags | 0x8000000;
    }
    character_controller.motion.prev_state_code = 38;
    current = (AnimatedObject *)(object_slot);
    if (current->animation != 0)
        goto done;
    switch (kind)
    {
        case 3:
        case 10:
        case 11:
        case 12:
            if ((character_controller.secondary_flags & 0x80) == 0)
                character_controller.motion.state_code = 6;
            break;
        case 5:
        case 7:
            current = (AnimatedObject *)(object_slot);
            if ((current->flags & 0x400) != 0 && (character_controller.secondary_flags & 0x80) == 0)
                character_controller.motion.state_code = 3;
            break;
        case 9:
            if ((character_controller.secondary_flags & 0x80) == 0)
                character_controller.motion.state_code = 2;
            break;
    }
    current = (AnimatedObject *)(object_slot);
    if (current->animation == 0)
    {
        result = character_controller.secondary_flags & 0x80;
        if (result == 0)
            character_controller.motion.state_code = 0;
    }
done:
    return;
}

uint32 ResetNewGameCharacter(void)
{
    uint32 index = 0;
    uint32 offset = 0;
    uint32 object;
    uint32 flags;
    uint32 kind;
    AnimatedObject *model;
    uint32 result;
    uint32 value;
    FUNCTION_MARKER(0x800558F0, "SLUS_005.78");
    if ((game_flow.mode_flags & 1) == 0)
        game_flow.progress.unlocked_stage = 0;
    game_flow.progress.selected_stage = 0;
    character_controller.vitals.health_pickups = 0;
    screen_render.fade_level = 0;
    do
    {
        object = 0x800ABD68 + offset;
        character_controller.vitals.lives = (game_flow.mode_flags & 8) ? 32 : 3;
        value = (game_flow.mode_flags & 4) ? 24 : 4;
        character_controller.vitals.health_limit = value;
        character_controller.vitals.health = value;
        flags = character_controller.motion.form_flags;
        character_controller.weapon_state = 0;
        character_controller.vitals.score = 0;
        character_controller.timers.idle_ticks = 900;
        flags &= 0xFFF0FFFF;
        flags |= ((flags >> 20) & 15) << 16;
        flags &= 0xF0FFFFFF;
        flags |= 0x0F000000;
        character_controller.motion.form_flags = flags;
        character_controller.object = 0;
        kind = (character_controller.motion.form_flags >> 20) & 15;
        if (character_controller.loaded_model_mask & (kind == 0 ? 1 : 2))
        {
            {
                CharacterModelRecord *selected_model;
                {
                    uint32 model_index = kind;
                    CharacterModelRecord *model_binding;
                    if (model_index < CharacterController::ModelCount)
                        model_binding = character_controller.model_slots[model_index];
                    else
                    {
                        fprintf(stderr, "P2: Invalid character model index %u\n", model_index);
                        abort();
                    }
                    if (model_binding == 0)
                    {
                        fprintf(stderr, "P2: Character model %u is not loaded\n", model_index);
                        abort();
                    }
                    selected_model = model_binding;
                }
                model = ((AnimatedObject *)(selected_model->object));
            }
            model->event_word = index;
        }
        if (character_controller.secondary_flags & 0x100)
        {
            character_controller.action_effects[6] = 0;
            character_controller.EndInteraction();
        }
        if (character_controller.secondary_flags & 0x20)
        {
            flags = player_interaction.flags;
            value = character_controller.secondary_flags;
            player_interaction.flags = flags & 0xFFFFFFDB;
            character_controller.secondary_flags = value & 0xFFFFFFDF;
        }
        value = character_controller.weapon_state;
        result = value >> 16;
        if ((value & 0xFFFF) != 0)
        {
            result = object + 4 * result;
            character_controller.action_effects[value >> 16] = 0;
        }
        character_controller.weapon_state = 0;
        ++index;
        offset += 928;
    } while ((sint32)index <= 0);
    return result;
}

AnimatedObject *FindPlayerObject(uint32 index)
{
    FUNCTION_MARKER(0x800536F0, "SLUS_005.78");
    if (index == 0)
        return character_controller.object;
    return 0;
}

uint32 IsAlternateCharacterInteractionActive(void)
{
    FUNCTION_MARKER(0x800447F0, "SLUS_005.78");
    if (((uint16)(character_controller.motion.form_flags >> 16) & 0xF) == 1 && (player_interaction.flags != 0 || (character_controller.secondary_flags & 0x20) != 0))
        return 1;
    return 0;
}

uint32 PlaceCharacterOnPath(const SceneSection *scene, sint32 path_id, sint16 frame_word)
{
    CharacterModelRecord *model_record;
    CharacterModelRecord *other_model_record;
    AnimatedObject *placed, *placed_view, *alternate;
    SceneNode *sprite;
    const PathDefinition *definition, *captured_definition;
    const MATRIX *matrix;

    union
    {
        VECTOR value;
        uint32 words[4];
    } pos, sample, world, other_velocity;

    uint16 inverse[9], other_inverse[9];
    const SceneTrigger *marker;
    const ScenePath *path = 0, *path_view, *search_path = 0;
    uint32 character;
    uint32 search_frame, checkpoint_selected = 0;
    uint32 frame = (uint32)(sint32)frame_word, best_distance, count, selected_frame;
    AnimatedObject *other_object;
    uint32 direction;
    uint32 x, y, z, value, flags, index, distance, result;
    sint32 sample_frame;
    FUNCTION_MARKER(0x80053E7C, "SLUS_005.78");
    character = 0x800ABD68;
    screen_render.fade_level = 0;
    character_controller.path_transition.contact_delay = 0;
    character_contact_pool.prev = 0;
    character_contact_pool.cur = 0;
    if (level_resource.level_id == 0 && (sint8)game_flow.progress.selected_stage > 0 && game_flow.progress.hub_spawn_consumed == 0)
    {
        marker = (const SceneTrigger *)scene->object_heads[4];
        game_flow.progress.hub_spawn_consumed = 1;
        while (marker != 0)
        {
            if ((sint8)(marker->node.state_flags >> 8) == 17 && (uint32)(sint32)(sint8)game_flow.progress.selected_stage == marker->parameters.words[0])
            {
                x = marker->transform.t[0];
                y = marker->transform.t[1];
                z = marker->transform.t[2];
                pos.words[0] = x;
                pos.words[1] = y;
                pos.words[2] = z;
                pos.words[1] = y - ((uint32)marker->extent[1] << 5);
                path = (const ScenePath *)scene->object_heads[3];
                best_distance = 0x7FFFF000;
                while (path != 0)
                {
                    if ((path->node.state_flags & 0x10) != 0)
                    {
                        captured_definition = (path->definition);
                        definition = (const PathDefinition *)(captured_definition);
                        selected_frame = 0xFFFFFFFF;
                        if (definition->positions.keys != 0)
                        {
                            frame = 0;
                            count = (uint32)(sint32)(sint16)definition->positions.metadata6;
                            do
                            {
                                definition->SamplePosition((sint32)frame, sample.words, 16);
                                x = sample.words[0] - pos.words[0];
                                sample.words[0] = x;
                                if (x + 409600 < 0xC8000)
                                {
                                    y = sample.words[1] - pos.words[1];
                                    sample.words[1] = y;
                                    if (y + 409600 < 0xC8000)
                                    {
                                        z = sample.words[2] - pos.words[2];
                                        sample.words[2] = z;
                                        if (z + 409600 < 0xC8000)
                                        {
                                            x = (uint32)((sint32)x >> 6);
                                            y = (uint32)((sint32)y >> 6);
                                            z = (uint32)((sint32)z >> 6);
                                            distance = x * x + y * y + z * z;
                                            if ((sint32)distance < (sint32)best_distance)
                                            {
                                                best_distance = distance;
                                                selected_frame = frame;
                                            }
                                        }
                                    }
                                }
                                --count;
                                ++frame;
                            } while (count != 0);
                        }
                        if ((sint32)selected_frame >= 0)
                        {
                            search_path = path;
                            search_frame = selected_frame;
                            checkpoint_selected = 1;
                        }
                    }
                    path = (const ScenePath *)path->node.links.registry_next;
                }
                if (checkpoint_selected != 0)
                {
                    path = search_path;
                    frame = search_frame;
                }
            }
            marker = (const SceneTrigger *)marker->node.links.registry_next;
        }
    }
    if (path == 0)
    {
        /* A missing checkpoint path uses the ordinary scene spawn */
        frame = (uint32)(sint32)frame_word;
        sample_frame = (sint16)path_id;
        if (sample_frame != -1)
            path = scene->FindPath(sample_frame);
        if (sample_frame == -1 || path == 0)
        {
            path = scene->FindDefaultPath();
            if (path == 0)
                path = scene->FindPath(0);
        }
    }
    index = (uint16)(character_controller.motion.form_flags >> 16) & 0xF;
    {
        uint32 model_index = index;
        CharacterModelRecord *model_binding;
        if (model_index < CharacterController::ModelCount)
            model_binding = character_controller.model_slots[model_index];
        else
        {
            fprintf(stderr, "P2: Invalid character model index %u\n", model_index);
            abort();
        }
        if (model_binding == 0)
        {
            fprintf(stderr, "P2: Character model %u is not loaded\n", model_index);
            abort();
        }
        model_record = model_binding;
    }
    placed = character_controller.object;
    if (placed != (model_record->object) || (game_flow.transition_flags & 2) != 0 || character_controller.model_rebind_pending != 0)
        AssignCharacterModel(&placed, (uint16)(character_controller.motion.form_flags >> 16) & 0xF);
    {
        uint32 model_index = ((uint16)(character_controller.motion.form_flags >> 16) & 0xF);
        CharacterModelRecord *model_binding;
        if (model_index < CharacterController::ModelCount)
            model_binding = character_controller.model_slots[model_index];
        else
        {
            fprintf(stderr, "P2: Invalid character model index %u\n", model_index);
            abort();
        }
        if (model_binding == 0)
        {
            fprintf(stderr, "P2: Character model %u is not loaded\n", model_index);
            abort();
        }
        model_record = model_binding;
        character_controller.model = model_binding;
    }
    placed_view = (AnimatedObject *)(placed);
    path_view = (const ScenePath *)(path);
    flags = path_view->node.state_flags;
    character_controller.model_rebind_pending = 0;
    direction = flags & 0x10;
    if ((frame & 0x8000) != 0)
    {
        frame = ~frame;
        if ((flags & 8) != 0)
        {
            path = (path_view->adjacent[0]);
            path_view = (const ScenePath *)(path);
        }
        direction ^= 0x10;
    }
    if (direction != (placed_view->flags & 0x10))
    {
        frame = (uint16)scene->transition_scale - frame - 1;
        if ((path_view->node.state_flags & 8) != 0)
        {
            path = (path_view->adjacent[0]);
            path_view = (const ScenePath *)(path);
        }
        direction ^= 0x10;
    }
    captured_definition = (path->definition);
    definition = (const PathDefinition *)(captured_definition);
    value = (uint32)(sint32)(sint16)definition->metadata36;
    if (value != 0)
        frame = (value * (uint32)(sint32)(sint16)frame) >> 12;
    placed_view->movement_mode = 1;
    placed_view->behavior = 1;
    placed_view->metadata126[0] = 1;
    placed_view->metadata126[1] = 0;
    flags = placed_view->path_flags;
    sprite = (SceneNode *)(((SpriteRecord *)(placed_view->sprite)));
    value = placed_view->flags;
    model_record = character_controller.model;
    index = (uint32)(sint32)(sint8)character_controller.motion.state_code;
    placed_view->animation = model_record->animations[(sint32)index];
    placed_view->unbound_animation_word = 0;
    placed_view->flags = (value & 0xFFFDDBFF) | 0x20000;
    placed_view->path_flags = flags & 0xFC000000;
    sprite->flags = sprite->flags & 0xFE;
    SelectScenePath(path);
    character_controller.path = path;
    camera_motion.frame = frame;
    camera_motion.smooth_ticks[2] = (uint8)(0);
    camera_motion.smooth_ticks[1] = (uint8)(0);
    camera_motion.smooth_ticks[0] = (uint8)(0);
    placed_view->path_definition = (path_view->definition);
    placed_view->yaw = (sint16)(4096);
    captured_definition = (path->definition);
    definition = (const PathDefinition *)(captured_definition);
    sample_frame = camera_motion.frame;
    if (direction == 0)
        sample_frame = (sint16)definition->positions.metadata6 - sample_frame - 1;
    SampleVectorTrack(sample_frame, &definition->positions, placed_view->position);
    placed_view->path_frame = (sint16)((uint16)camera_motion.frame);
    if (level_resource.level_id == 0)
    {
        character_controller.flags = character_controller.flags | 0x80000000;
        placed_view->ResetCharacterForScene(0);
    }
    else
    {
        value = character_controller.section_height_offset;
        character_controller.flags = character_controller.flags & 0x7FFFFFFF;
        if ((sint32)value > 0)
        {
            captured_definition = (path->definition);
            definition = (const PathDefinition *)(captured_definition);
            if (captured_definition != 0 && (definition->transform) != 0)
            {
                x = placed_view->position[0];
                y = placed_view->position[1];
                z = placed_view->position[2];
                pos.words[0] = x;
                pos.words[1] = y;
                pos.words[2] = z;
                PushMatrix();
                matrix = (const MATRIX *)((((const PathDefinition *)((const PathDefinition *)((placed_view->path_definition))))->transform) != 0 ? &(((const PathDefinition *)((const PathDefinition *)((placed_view->path_definition))))->transform)->matrix : (MATRIX *)0);
                x = xport_load_le32(matrix);
                y = xport_load_le32((const uint8 *)matrix + 4);
                xport_gte_write_control(0, x);
                xport_gte_write_control(1, y);
                x = xport_load_le32((const uint8 *)matrix + 8);
                y = xport_load_le32((const uint8 *)matrix + 12);
                z = xport_load_le32((const uint8 *)matrix + 16);
                xport_gte_write_control(2, x);
                xport_gte_write_control(3, y);
                xport_gte_write_control(4, z);
                ApplyRotMatrixLV(&pos.value, (VECTOR *)placed_view->position);
                PopMatrix();
                matrix = (const MATRIX *)((((const PathDefinition *)((const PathDefinition *)((placed_view->path_definition))))->transform) != 0 ? &(((const PathDefinition *)((const PathDefinition *)((placed_view->path_definition))))->transform)->matrix : (MATRIX *)0);
                placed_view->position[0] = placed_view->position[0] + (uint32)matrix->t[0];
                matrix = (const MATRIX *)((((const PathDefinition *)((const PathDefinition *)((placed_view->path_definition))))->transform) != 0 ? &(((const PathDefinition *)((const PathDefinition *)((placed_view->path_definition))))->transform)->matrix : (MATRIX *)0);
                placed_view->position[1] = placed_view->position[1] + (uint32)matrix->t[1];
                matrix = (const MATRIX *)((((const PathDefinition *)((const PathDefinition *)((placed_view->path_definition))))->transform) != 0 ? &(((const PathDefinition *)((const PathDefinition *)((placed_view->path_definition))))->transform)->matrix : (MATRIX *)0);
                placed_view->position[2] = placed_view->position[2] + (uint32)matrix->t[2];
            }
            else
            {
                x = placed_view->position[0];
                y = placed_view->position[1];
                z = placed_view->position[2];
                placed_view->position[0] = x;
                placed_view->position[1] = y;
                placed_view->position[2] = z;
            }
            placed_view->position[1] = placed_view->position[1] + character_controller.section_height_offset;
            captured_definition = (path->definition);
            definition = (const PathDefinition *)(captured_definition);
            if (captured_definition != 0 && (definition->transform) != 0)
            {
                matrix = (const MATRIX *)((definition->transform) != 0 ? &(definition->transform)->matrix : (MATRIX *)0);
                sample.words[0] = placed_view->position[0] - (uint32)matrix->t[0];
                matrix = (const MATRIX *)((((const PathDefinition *)((const PathDefinition *)((placed_view->path_definition))))->transform) != 0 ? &(((const PathDefinition *)((const PathDefinition *)((placed_view->path_definition))))->transform)->matrix : (MATRIX *)0);
                sample.words[1] = placed_view->position[1] - (uint32)matrix->t[1];
                matrix = (const MATRIX *)((((const PathDefinition *)((const PathDefinition *)((placed_view->path_definition))))->transform) != 0 ? &(((const PathDefinition *)((const PathDefinition *)((placed_view->path_definition))))->transform)->matrix : (MATRIX *)0);
                sample.words[2] = placed_view->position[2] - (uint32)matrix->t[2];
                matrix = (const MATRIX *)((definition->transform) != 0 ? &(definition->transform)->matrix : (MATRIX *)0);
                inverse[0] = (uint16)matrix->m[0][0];
                inverse[1] = (uint16)matrix->m[1][0];
                inverse[2] = (uint16)matrix->m[2][0];
                inverse[3] = (uint16)matrix->m[0][1];
                inverse[4] = (uint16)matrix->m[1][1];
                inverse[5] = (uint16)matrix->m[2][1];
                inverse[6] = (uint16)matrix->m[0][2];
                inverse[7] = (uint16)matrix->m[1][2];
                inverse[8] = (uint16)matrix->m[2][2];
                PushMatrix();
                x = ((uint32)inverse[0] | ((uint32)inverse[1] << 16));
                y = ((uint32)inverse[2] | ((uint32)inverse[3] << 16));
                xport_gte_write_control(0, x);
                xport_gte_write_control(1, y);
                x = ((uint32)inverse[4] | ((uint32)inverse[5] << 16));
                y = ((uint32)inverse[6] | ((uint32)inverse[7] << 16));
                // Control register 4 consumes only the ninth halfword
                z = inverse[8];
                xport_gte_write_control(2, x);
                xport_gte_write_control(3, y);
                xport_gte_write_control(4, z);
                ApplyRotMatrixLV(&sample.value, (VECTOR *)placed_view->position);
                PopMatrix();
            }
            else
            {
                x = placed_view->position[0];
                y = placed_view->position[1];
                z = placed_view->position[2];
                placed_view->position[0] = x;
                placed_view->position[1] = y;
                placed_view->position[2] = z;
            }
            placed_view->flags = placed_view->flags | 0x400;
        }
    }
    if ((((const PathDefinition *)((const PathDefinition *)((placed_view->path_definition))))->flags & 0x11) != 0)
        placed_view->flags = placed_view->flags | 0x400;
    path_view->SampleOffset(camera_motion.frame, (uint32 *)camera_motion.position, direction);
    path_view->SamplePosition(camera_motion.frame, (uint32 *)camera_motion.focus_position, direction);
    placed_view->spheres[3].radius = 0xFFFFFFFF;
    placed_view->spheres[2].radius = 0xFFFFFFFF;
    placed_view->spheres[1].radius = 0xFFFFFFFF;
    placed_view->spheres[0].radius = 0xFFFFFFFF;
    (placed != 0 ? &placed_view->node : 0)->Register();
    if ((sint32)character_controller.flags < 0)
    {
        placed_view->flags = placed_view->flags | 0x40;
        placed_view->ComputePathMotion(24576, 0, NULL);
        placed_view->flags = placed_view->flags & 0xFFFFFFBF;
        captured_definition = (path->definition);
        definition = (const PathDefinition *)(captured_definition);
        if (captured_definition != 0 && (definition->transform) != 0)
        {
            matrix = (const MATRIX *)((definition->transform) != 0 ? &(definition->transform)->matrix : (MATRIX *)0);
            inverse[0] = (uint16)matrix->m[0][0];
            inverse[1] = (uint16)matrix->m[1][0];
            inverse[2] = (uint16)matrix->m[2][0];
            inverse[3] = (uint16)matrix->m[0][1];
            inverse[4] = (uint16)matrix->m[1][1];
            inverse[5] = (uint16)matrix->m[2][1];
            inverse[6] = (uint16)matrix->m[0][2];
            inverse[7] = (uint16)matrix->m[1][2];
            inverse[8] = (uint16)matrix->m[2][2];
            x = placed_view->velocity[0];
            y = placed_view->velocity[1];
            z = placed_view->velocity[2];
            sample.words[0] = x;
            sample.words[1] = y;
            sample.words[2] = z;
            PushMatrix();
            x = ((uint32)inverse[0] | ((uint32)inverse[1] << 16));
            y = ((uint32)inverse[2] | ((uint32)inverse[3] << 16));
            xport_gte_write_control(0, x);
            xport_gte_write_control(1, y);
            x = ((uint32)inverse[4] | ((uint32)inverse[5] << 16));
            y = ((uint32)inverse[6] | ((uint32)inverse[7] << 16));
            z = inverse[8];
            xport_gte_write_control(2, x);
            xport_gte_write_control(3, y);
            xport_gte_write_control(4, z);
            ApplyRotMatrixLV(&sample.value, &pos.value);
            PopMatrix();
        }
        else
        {
            x = placed_view->velocity[0];
            y = placed_view->velocity[1];
            z = placed_view->velocity[2];
            pos.words[0] = x;
            pos.words[1] = y;
            pos.words[2] = z;
        }
        placed_view->position[0] = placed_view->position[0] + pos.words[0];
        placed_view->position[1] = placed_view->position[1] + pos.words[1];
        placed_view->position[2] = placed_view->position[2] + pos.words[2];
        placed_view->velocity[2] = 0;
        placed_view->velocity[1] = 0;
        placed_view->velocity[0] = 0;
        captured_definition = (path->definition);
        definition = (const PathDefinition *)(captured_definition);
        if (captured_definition != 0 && (definition->transform) != 0)
        {
            x = placed_view->position[0];
            y = placed_view->position[1];
            z = placed_view->position[2];
            world.words[0] = x;
            world.words[1] = y;
            world.words[2] = z;
            PushMatrix();
            matrix = (const MATRIX *)((((const PathDefinition *)((const PathDefinition *)((placed_view->path_definition))))->transform) != 0 ? &(((const PathDefinition *)((const PathDefinition *)((placed_view->path_definition))))->transform)->matrix : (MATRIX *)0);
            x = xport_load_le32(matrix);
            y = xport_load_le32((const uint8 *)matrix + 4);
            xport_gte_write_control(0, x);
            xport_gte_write_control(1, y);
            x = xport_load_le32((const uint8 *)matrix + 8);
            y = xport_load_le32((const uint8 *)matrix + 12);
            z = xport_load_le32((const uint8 *)matrix + 16);
            xport_gte_write_control(2, x);
            xport_gte_write_control(3, y);
            xport_gte_write_control(4, z);
            ApplyRotMatrixLV(&world.value, (VECTOR *)camera_motion.focus_position);
            PopMatrix();
            matrix = (const MATRIX *)((((const PathDefinition *)((const PathDefinition *)((placed_view->path_definition))))->transform) != 0 ? &(((const PathDefinition *)((const PathDefinition *)((placed_view->path_definition))))->transform)->matrix : (MATRIX *)0);
            camera_motion.focus_position[0] = camera_motion.focus_position[0] + (uint32)matrix->t[0];
            matrix = (const MATRIX *)((((const PathDefinition *)((const PathDefinition *)((placed_view->path_definition))))->transform) != 0 ? &(((const PathDefinition *)((const PathDefinition *)((placed_view->path_definition))))->transform)->matrix : (MATRIX *)0);
            camera_motion.focus_position[1] = camera_motion.focus_position[1] + (uint32)matrix->t[1];
            matrix = (const MATRIX *)((((const PathDefinition *)((const PathDefinition *)((placed_view->path_definition))))->transform) != 0 ? &(((const PathDefinition *)((const PathDefinition *)((placed_view->path_definition))))->transform)->matrix : (MATRIX *)0);
            camera_motion.focus_position[2] = camera_motion.focus_position[2] + (uint32)matrix->t[2];
        }
        else
        {
            x = placed_view->position[0];
            y = placed_view->position[1];
            z = placed_view->position[2];
            camera_motion.focus_position[0] = x;
            camera_motion.focus_position[1] = y;
            camera_motion.focus_position[2] = z;
        }
        sample_frame = placed_view->path_frame;
        camera_motion.frame = sample_frame;
        ((const ScenePath *)(camera_motion.path))->ProjectOffset(sample_frame, (uint32 *)camera_motion.position, placed_view->flags & 0x10);
        {
            uint32 model_index = (1 - ((uint16)(character_controller.motion.form_flags >> 16) & 0xF));
            CharacterModelRecord *model_binding;
            if (model_index < CharacterController::ModelCount)
                model_binding = character_controller.model_slots[model_index];
            else
            {
                fprintf(stderr, "P2: Invalid character model index %u\n", model_index);
                abort();
            }
            if (model_binding == 0)
            {
                fprintf(stderr, "P2: Character model %u is not loaded\n", model_index);
                abort();
            }
            other_model_record = model_binding;
        }
        other_object = other_model_record->object;
        alternate = (AnimatedObject *)(other_object);
        alternate->node.flags = placed_view->node.flags;
        x = placed_view->position[0];
        y = placed_view->position[1];
        z = placed_view->position[2];
        alternate->position[0] = x;
        alternate->position[1] = y;
        alternate->position[2] = z;
        alternate->flags = placed_view->flags;
        alternate->path_flags = placed_view->path_flags;
        x = placed_view->world_position[0];
        y = placed_view->world_position[1];
        z = placed_view->world_position[2];
        alternate->world_position[0] = x;
        alternate->world_position[1] = y;
        alternate->world_position[2] = z;
        x = placed_view->metadata_words72[0];
        y = placed_view->metadata_words72[1];
        z = placed_view->metadata_words72[2];
        alternate->metadata_words72[0] = x;
        alternate->metadata_words72[1] = y;
        alternate->metadata_words72[2] = z;
        x = placed_view->orientation_words[0];
        y = placed_view->orientation_words[1];
        alternate->orientation_words[0] = x;
        alternate->orientation_words[1] = y;
        x = placed_view->velocity[0];
        y = placed_view->velocity[1];
        z = placed_view->velocity[2];
        alternate->velocity[0] = x;
        alternate->velocity[1] = y;
        alternate->velocity[2] = z;
        alternate->event_word = placed_view->event_word;
        alternate->behavior_word = placed_view->behavior_word;
        alternate->state = placed_view->state;
        alternate->movement_mode = (uint8)placed_view->movement_mode;
        alternate->behavior = (uint8)placed_view->behavior;
        alternate->metadata126[0] = placed_view->metadata126[0];
        alternate->metadata126[1] = placed_view->metadata126[1];
        alternate->ticks = (uint16)placed_view->ticks;
        alternate->frame = (uint16)placed_view->frame;
        alternate->path_definition = (placed_view->path_definition);
        alternate->path_frame = (sint16)((uint16)placed_view->path_frame);
        alternate->yaw = (sint16)((uint16)placed_view->yaw);
        alternate->target_yaw = (sint16)((uint16)placed_view->target_yaw);
        alternate->pitch = (sint16)((uint16)placed_view->pitch);
        alternate->pitch_scale = (sint16)((uint16)placed_view->pitch_scale);
        alternate->path_motion_scale = placed_view->path_motion_scale;
        CopyBytes((uint8 *)alternate->auxiliary_words, (const uint8 *)placed_view->auxiliary_words, 64);
        alternate->movement_mode = 0;
        alternate->metadata126[0] = 0;
        alternate->metadata126[1] = 0;
        alternate->behavior = 33;
        flags = alternate->flags;
        alternate->scene_path = path;
        alternate->follow_target = placed;
        AnimationHeader *initial_animation = other_model_record->animations[0];
        alternate->flags = flags | 0x20000;
        alternate->animation = initial_animation;
        alternate->unbound_animation_word = 0;
        (other_object != 0 ? &alternate->node : 0)->Register();
        flags = alternate->node.flags;
        value = alternate->flags & 0xFFFFFFBF;
        alternate->metadata126[0] = 1;
        alternate->node.flags = flags | 0x41;
        alternate->flags = value;
        alternate->ComputePathMotion(45056, 0, NULL);
        alternate->metadata126[0] = 0;
        captured_definition = (path->definition);
        definition = (const PathDefinition *)(captured_definition);
        if (captured_definition != 0 && (definition->transform) != 0)
        {
            matrix = (const MATRIX *)((definition->transform) != 0 ? &(definition->transform)->matrix : (MATRIX *)0);
            other_inverse[0] = (uint16)matrix->m[0][0];
            other_inverse[1] = (uint16)matrix->m[1][0];
            other_inverse[2] = (uint16)matrix->m[2][0];
            other_inverse[3] = (uint16)matrix->m[0][1];
            other_inverse[4] = (uint16)matrix->m[1][1];
            other_inverse[5] = (uint16)matrix->m[2][1];
            other_inverse[6] = (uint16)matrix->m[0][2];
            other_inverse[7] = (uint16)matrix->m[1][2];
            other_inverse[8] = (uint16)matrix->m[2][2];
            x = alternate->velocity[0];
            y = alternate->velocity[1];
            z = alternate->velocity[2];
            other_velocity.words[0] = x;
            other_velocity.words[1] = y;
            other_velocity.words[2] = z;
            PushMatrix();
            x = ((uint32)other_inverse[0] | ((uint32)other_inverse[1] << 16));
            y = ((uint32)other_inverse[2] | ((uint32)other_inverse[3] << 16));
            xport_gte_write_control(0, x);
            xport_gte_write_control(1, y);
            x = ((uint32)other_inverse[4] | ((uint32)other_inverse[5] << 16));
            y = ((uint32)other_inverse[6] | ((uint32)other_inverse[7] << 16));
            z = other_inverse[8];
            xport_gte_write_control(2, x);
            xport_gte_write_control(3, y);
            xport_gte_write_control(4, z);
            ApplyRotMatrixLV(&other_velocity.value, &pos.value);
            PopMatrix();
        }
        else
        {
            x = alternate->velocity[0];
            y = alternate->velocity[1];
            z = alternate->velocity[2];
            pos.words[0] = x;
            pos.words[1] = y;
            pos.words[2] = z;
        }
        alternate->position[0] = alternate->position[0] + pos.words[0];
        alternate->position[1] = alternate->position[1] + pos.words[1];
        alternate->position[2] = alternate->position[2] + pos.words[2];
        pos.words[0] = 0 - alternate->velocity[0];
        pos.words[1] = 0 - alternate->velocity[1];
        pos.words[2] = 0 - alternate->velocity[2];
        alternate->velocity[2] = 0;
        alternate->velocity[1] = 0;
        alternate->velocity[0] = 0;
        alternate->TurnObjectFacing(pos.words, 4096);
    }
    {
        MATRIX *root_matrix = &render_view.matrix_cursor->matrix;
        x = psx_identity_rotation_words[0];
        y = psx_identity_rotation_words[1];
        z = psx_identity_rotation_words[2];
        xport_store_le32((uint8 *)root_matrix, x);
        xport_store_le32((uint8 *)root_matrix + 4, y);
        xport_store_le32((uint8 *)root_matrix + 8, z);
        x = psx_identity_rotation_words[3];
        y = (uint16)psx_identity_matrix.m[2][2];
        xport_store_le32((uint8 *)root_matrix + 12, x);
        root_matrix->m[2][2] = (sint16)y;
        root_matrix = &render_view.matrix_cursor->matrix;
        x = xport_load_le32((const uint8 *)root_matrix);
        y = xport_load_le32((const uint8 *)root_matrix + 4);
        xport_gte_write_control(0, x);
        xport_gte_write_control(1, y);
        x = xport_load_le32((const uint8 *)root_matrix + 8);
        y = xport_load_le32((const uint8 *)root_matrix + 12);
        z = xport_load_le32((const uint8 *)root_matrix + 16);
        xport_gte_write_control(2, x);
        xport_gte_write_control(3, y);
        xport_gte_write_control(4, z);
        root_matrix = &render_view.matrix_cursor->matrix;
        root_matrix->t[2] = 0;
        root_matrix->t[1] = 0;
        root_matrix->t[0] = 0;
    }
    placed_view->node.flags = placed_view->node.flags | 0x41;
    UpdateAndDrawRegisteredObjects();
    placed_view->node.flags = placed_view->node.flags & 0xFE;
    value = character_controller.weapon_state;
    if ((value & 0xFFFF) != 0)
    {
        index = (uint16)(character_controller.motion.form_flags >> 16) & 0xF;
        if (index != 5 && index - 10 >= 2)
        {
            value >>= 16;
            SpawnActionEffect(placed, character != 0 ? (const void *)&character_controller : 0, value == 0 ? 3 : value - 7);
        }
    }
    if ((character_controller.secondary_flags & 0x40) != 0)
    {
        model_record = character_controller.model;
        AnimationHeader *animation22 = model_record->animations[22];
        AnimationHeader *animation2 = model_record->animations[2];
        model_record->animations[2] = animation22;
        model_record = character_controller.model;
        model_record->animations[22] = animation2;
        SpawnActionEffect(placed, character != 0 ? (const void *)&character_controller : 0, 7);
    }
    value = camera_motion.focus_position[1];
    value += (sint32)character_controller.flags < 0 ? 0x8000 : 14336;
    camera_motion.focus_position[1] = value;
    camera_motion.flags = camera_motion.flags & 0xFE;
    ((const PathDefinition *)((const PathDefinition *)((placed_view->path_definition))))->SampleTangent(&character_controller.path_tangent, (sint32)direction);
    value = game_flow.mode_flags;
    player_interaction.flags = 0;
    if ((value & 0x200) == 0 || ((uint16)(character_controller.motion.form_flags >> 16) & 0xF) != 0)
    {
        flags = character_controller.secondary_flags;
        result = flags & 0x120;
        if (result == 0)
            goto finish;
        result = flags | 0x20;
        if (((uint16)(character_controller.motion.form_flags >> 16) & 0xF) == 1)
        {
            character_controller.secondary_flags = result;
            goto finish;
        }
        SpawnActionEffect(placed, character != 0 ? (const void *)&character_controller : 0, 6);
    }
    result = BeginCharacterInteraction(placed);
finish:
    return result;
}

uint32 AnimatedObject::UpdateCharacterAnimationState()
{
    CharacterModelRecord *animation_data_record;
    AnimationHeader *selected_animation;
    FUNCTION_MARKER(0x80052BA4, "SLUS_005.78");
    uint32 flags, value, result, kind, pitch;
    const SoundBankStorage *sound_table;
    const SoundSample *sound;
    sint32 previous, animation, voice, volume;
    if ((sint8)character_controller.vitals.health != 0 && (character_controller.flags & 0x180000) != 0 && (character_controller.secondary_flags & 0x80) == 0)
        character_controller.motion.state_code = 8;
    previous = (sint8)character_controller.motion.prev_state_code;
    animation = (sint8)character_controller.motion.state_code;
    if (previous == animation)
    {
        if ((uint32)previous - 1 >= 34)
            character_controller.timers.idle_ticks = (uint32)character_controller.timers.idle_ticks - 1;
        if ((sint8)character_controller.motion.state_code != 7)
            return 7;
        result = character_controller.motion.horizontal_speed;
        if (result != 0)
            return result;
        result = character_controller.secondary_flags & 0x40;
        if (result != 0)
            return result;

        sound_table = sound_resources.common_table;
        sound = sound_table != 0 && (sint32)sound_table->header.count >= 7 ? sound_table->samples + 6 : 0;
        result = sound_effects.IsPlaying(sound, (uint32)(sint32)character_controller.sound_voice) << 24;
        if (result == 0)
        {
            sound_table = sound_resources.common_table;
            sound = sound_table != 0 && (sint32)sound_table->header.count >= 11 ? sound_table->samples + 10 : 0;
            result = sound_effects.IsPlaying(sound, (uint32)(sint32)character_controller.sound_voice) << 24;
            if (result == 0)
                return result;
        }
        StopSoundVoice((uint16)(sint16)character_controller.sound_voice);
        character_controller.sound_voice = (sint8)(0xFF);
        return 0xFFFFFFFF;
    }
    if (previous == 7 && (this->flags & 0x8000000) != 0 && ((uint16)(character_controller.motion.form_flags >> 16) & 15) == 4)
    {
        character_controller.motion.state_code = 7;
        goto finish;
    }
    animation = (sint8)character_controller.motion.state_code;
    if (animation == 7)
    {
        kind = (uint16)(character_controller.motion.form_flags >> 16) & 15;
        if (kind != 5 && kind - 10 >= 2)
        {
            animation_data_record = character_controller.model;
            flags = this->flags;
            selected_animation = animation_data_record->animations[7];
            this->frame = 4;
            this->flags = (flags & 0xFFFEFFFF) | 0x20000;
            this->animation = selected_animation;
            goto finish;
        }
        animation = (sint8)character_controller.motion.state_code;
    }
    if (animation == 20)
    {
        animation_data_record = character_controller.model;
        flags = this->flags;
        selected_animation = animation_data_record->animations[20];
        goto set_animation_data;
    }
    if (character_controller.path_transition.reentry_ticks != 0)
    {
        animation_data_record = character_controller.model;
        flags = this->flags;
        selected_animation = animation_data_record->animations[3];
        goto set_animation_data;
    }
    if ((character_controller.secondary_flags & 0x40) == 0)
    {
        sound_table = sound_resources.common_table;
        sound = sound_table != 0 && (sint32)sound_table->header.count >= 7 ? sound_table->samples + 6 : 0;
        result = sound_effects.IsPlaying(sound, (uint32)(sint32)character_controller.sound_voice) << 24;
        if (result == 0)
        {
            sound_table = sound_resources.common_table;
            sound = sound_table != 0 && (sint32)sound_table->header.count >= 11 ? sound_table->samples + 10 : 0;
            result = sound_effects.IsPlaying(sound, (uint32)(sint32)character_controller.sound_voice) << 24;
        }
        if (result != 0)
        {
            StopSoundVoice((uint16)(sint16)character_controller.sound_voice);
            character_controller.sound_voice = (sint8)(0xFF);
        }
    }
    animation = (sint8)character_controller.motion.state_code;
    if (animation == 0)
    {
        previous = (sint8)character_controller.motion.prev_state_code;
        if ((uint32)previous - 1 < 34)
            character_controller.timers.idle_ticks = 900;
        animation = (sint8)character_controller.motion.state_code;
    }
    animation_data_record = character_controller.model;
    selected_animation = animation_data_record->animations[animation];
    flags = this->flags;
    this->frame = -1;
    this->flags = flags | 0x10000;
    this->animation = selected_animation;
    animation = (sint8)character_controller.motion.state_code;
    if (animation == 8)
    {
        if ((character_controller.secondary_flags & 0x40) == 0)
        {
            sound_table = sound_resources.common_table;
            sound = sound_table != 0 && (sint32)sound_table->header.count >= 7 ? sound_table->samples + 6 : 0;
            voice = sound_effects.SelectVoice(0xFFFFFFFF, sound, 1);
            character_controller.sound_voice = (sint8)(voice);
            voice = (sint8)voice;
            if (voice >= 0)
            {
                const SoundSample *sample = ((const SoundSample *)(sound));
                value = sample->metadata;
                pitch = sample->pitch;
                volume = (sint16)(sound_settings.effects_volume);
                PlaySoundEffect(sound, (uint16)voice, 0, value, pitch, volume, 0);
            }
        }
    }
    else if (animation == 9 && ((uint16)(character_controller.motion.form_flags >> 16) & 15) == 1)
        QueueCharacterActionSound(4, 3);
    else if ((this->path_flags & 0x40000) != 0 && (sint8)character_controller.path_transition.phase == 8)
        this->flags = this->flags & 0xFFFEFFFF;
    goto finish;
set_animation_data:
    this->flags = flags & 0xFFFEFFFF;
    this->animation = selected_animation;
finish:
    flags = this->path_flags;
    result = 0xFFDF0000;
    if ((flags & 0x200000) != 0)
    {
        this->path_flags = flags & 0xFFDFFFFF;
        result = this->flags;
        flags = this->path_flags;
        result &= 0xFFFEFFFF;
        this->flags = result;
        this->path_flags = flags & 0xFFBFFFFF;
    }
    return result;
}
