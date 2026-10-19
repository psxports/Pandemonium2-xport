#include <stdio.h>
#include <stdlib.h>

#include "game.h"
#include "original_defaults.h"

const PreviewAnimationStep preview_animation_steps[1] = {{2, {0xFFFFF000, 0}, 1000}};
PreviewPlayback preview_playback = {0};

void CharacterPreview::Update()
{
    CharacterModelRecord *animation_record;
    uint32 current;
    AnimationHeader *next;
    const PreviewAnimationStep *step;
    uint32 duration;
    AnimatedObject *object;
    uint32 entry;
    AnimationHeader *old_animation;
    uint32 flags;
    uint32 x;
    uint32 y;
    uint32 result;
    FUNCTION_MARKER(0x80053CA0, "SLUS_005.78");
    current = preview_playback.tick;
    if ((sint32)current < 0)
    {
        step = preview_playback.first_step;
        preview_playback.reset_word = 0;
        preview_playback.tick = 0;
        preview_playback.cur_step = step;
    }
    else
    {
        step = preview_playback.cur_step;
        duration = step->duration;
        ++current;
        preview_playback.tick = current;
        if ((sint32)duration < (sint32)current)
        {
            preview_playback.tick = 0;
            // Advancing past the sole step would read the following sine table
            preview_playback.cur_step = preview_playback.first_step;
        }
    }
    entry = character_controller.motion.form_flags;
    {
        uint32 model_index = (((entry >> 18) & 0x3C)) / 4;
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
            this->object = 0;
            return;
        }
        animation_record = model_binding;
    }
    object = animation_record->object;
    if (object == 0)
    {
        this->object = 0;
        return;
    }
    step = preview_playback.cur_step;
    current = step->animation;
    old_animation = object->animation;
    next = animation_record->animations[current];
    if (object->unbound_animation_word != 0 || old_animation != next)
    {
        flags = object->flags;
        object->animation = next;
        object->unbound_animation_word = 0;
        object->frame = -1;
        object->flags = flags | 0x10000;
        velocity[0] = step->velocity[0];
        velocity[1] = step->velocity[1];
    }
    y = position[1];
    duration = velocity[1];
    x = position[0];
    flags = velocity[0];
    y += duration;
    x += flags;
    position[1] = y;
    position[0] = x;
    if ((sint32)x > 143359)
    {
        result = 0 - velocity[0];
        position[0] = 143360;
        velocity[0] = result;
    }
    else
    {
        if ((sint32)x <= -143360)
        {
            result = 0 - velocity[0];
            position[0] = (uint32)-143360;
            velocity[0] = result;
        }
    }
    this->object = object;
}

void CharacterPreview::Draw()
{
    SceneSection local_section = {0};
    AnimatedObject *object;
    SceneNode *sprite;
    SceneSection *saved_context;
    uint32 flags;
    uint32 x;
    uint32 y;
    uint32 z;
    uint32 angle;
    AnimationHeader *animation;
    SpriteRecord *child;
    SceneLight local_light = {0};
    SceneLight *light;
    SceneLight *saved_lights = scene_lighting.lights;
    uint32 projection;
    sint32 frame;
    sint32 count;
    uint8 byte_flags;
    FUNCTION_MARKER(0x8006DE08, "SLUS_005.78");
    light = &local_light;
    object = this->object;
    if (object == 0)
        return;
    byte_flags = object->node.flags;
    flags = object->flags;
    object->movement_mode = 0;
    object->node.flags = byte_flags & 0xFD;
    saved_context = scene_stream.active_section;
    object->path_definition = 0;
    object->flags = flags | 0x2000;
    scene_stream.active_section = &local_section;
    scene_lighting.lights = 0;
    scene_lighting.ambient[2] = (uint16)(128);
    scene_lighting.ambient[1] = (uint16)(128);
    scene_lighting.ambient[0] = (uint16)(128);
    camera_motion.position[1] = 0;
    camera_motion.position[0] = 0;
    camera_motion.position[2] = (uint32)-49152;
    camera_motion.focus_position[2] = 0;
    camera_motion.focus_position[1] = 0;
    camera_motion.focus_position[0] = 0;
    x = camera_motion.focus_position[0];
    y = camera_motion.focus_position[1];
    z = camera_motion.focus_position[2];
    object->world_position[0] = x;
    object->world_position[1] = y;
    object->world_position[2] = z;
    object->metadata_words72[0] = 40960;
    this->object->node.Register();
    x = camera_motion.focus_position[0];
    y = camera_motion.focus_position[1];
    z = camera_motion.focus_position[2];
    object->position[0] = x;
    object->position[1] = y;
    object->position[2] = z;
    x = object->position[0];
    y = position[0];
    object->position[0] = x + y;
    x = object->position[1];
    y = position[1];
    object->position[1] = x + y;
    x = object->position[2];
    y = position[2];
    object->position[2] = x + y;
    object->orientation[0] = 0x7FFF;
    z = velocity[2];
    x = velocity[0];
    angle = Atan2Angle((sint32)(0 - z), (sint32)(0 - x));
    animation = object->animation;
    object->orientation[2] = (angle + 2048) & 0xFFF;
    frame = 0;
    if (animation != 0)
    {
        frame = object->frame;
        count = animation->frame_count;
        frame = frame < count - 1 ? frame + 1 : 0;
    }
    object->frame = (uint16)frame;
    object->ClampAnimationFrame();
    projection = camera_projection.scale;
    light->source.direction.vz = -4096;
    light->color[0] = 2056;
    light->color[1] = 2056;
    light->color[2] = 2056;
    light->links.registry_next = 0;
    light->flags = 0;
    light->event_flags = 0;
    light->source.direction.vy = 0;
    light->source.direction.vx = 0;
    scene_lighting.lights = light;
    if (projection != 955733 || camera_projection.view_normals[0].vz != 4096)
    {
        camera_projection.scale = 955733;
        SetCameraProjection(955733);
    }
    byte_flags = camera_motion.flags;
    camera_projection.far_distance = 1228800;
    camera_motion.flags = byte_flags & 0xFE;
    child = (object->sprite);
    sprite = (SceneNode *)child;
    object->sprite = 0;
    if (sprite != 0)
        sprite->flags |= 1;
    DrawSceneFrame(0);
    object->sprite = child;
    if (sprite != 0)
        sprite->flags &= 0xFE;
    this->object->node.Unregister();
    object->movement_mode = 1;
    scene_stream.active_section = saved_context;
    scene_lighting.lights = saved_lights;
}

AnimatedObject *PrepareCompletionCharacter(void)
{
    CharacterModelRecord *model_record;
    FUNCTION_MARKER(0x80053BF0, "SLUS_005.78");
    AnimatedObject *object = character_controller.object;
    AnimationHeader *action;
    AnimatedObject *result = object;
    if (object == 0)
    {
        {
            uint32 model_index = (((character_controller.motion.form_flags >> 18) & 60)) / 4;
            CharacterModelRecord *model_binding;
            if (model_index < CharacterController::ModelCount)
                model_binding = character_controller.model_slots[model_index];
            else
            {
                fprintf(stderr, "P2: Invalid character model index %u\n", model_index);
                abort();
            }
            if (model_binding == 0)
                return 0;
            model_record = model_binding;
        }
        object = model_record->object;
        if (object == 0)
            return 0;
        result = object;
        object->metadata126[0] = 1;
        object->behavior = 1;
        object->metadata126[1] = 0;
        object->spheres[3].radius = (uint32)-1;
        object->spheres[2].radius = (uint32)-1;
        object->spheres[1].radius = (uint32)-1;
        object->spheres[0].radius = (uint32)-1;
    }
    {
        uint32 model_index = (((character_controller.motion.form_flags >> 18) & 60)) / 4;
        CharacterModelRecord *model_binding;
        if (model_index < CharacterController::ModelCount)
            model_binding = character_controller.model_slots[model_index];
        else
        {
            fprintf(stderr, "P2: Invalid character model index %u\n", model_index);
            abort();
        }
        if (model_binding == 0)
            return 0;
        model_record = model_binding;
    }
    action = model_record->animations[2];
    if (object->unbound_animation_word != 0 || object->animation != action)
    {
        uint32 flags = object->flags;
        object->frame = -1;
        object->animation = action;
        object->unbound_animation_word = 0;
        object->flags = flags | 0x10000;
    }
    return result;
}

uint32 DrawLoadingFrame(void)
{
    FUNCTION_MARKER(0x80066168, "SLUS_005.78");
    const TextureDescriptor *image;
    uint32 index;
    BeginDraw();
    ClearSceneOrderingTable();
    DrawBackgroundRectangle(0, 0xF00140, 0);
    image = resource_groups[2].texture;
    for (index = 0; index < 2; ++index)
    {
        sint32 width = (sint32)image->payload.bytes[2] - image->payload.bytes[0];
        sint32 height = (sint32)image->payload.bytes[3] - image->payload.bytes[1];
        uint32 x = (uint32)((319 - width) >> 1);
        uint32 y = (uint32)((239 - height) >> 1);
        DrawTexturedQuad(x | (y << 16), 0, image, 0);
    }
    return EndDraw();
}

uint32 DrawCompletionFrame(sint32 percent, uint32 flags, sint32 next_level, CharacterPreview *preview, sint8 mode)
{
    const MenuMaterialResources *resources;
    FUNCTION_MARKER(0x8006E098, "SLUS_005.78");
    char text[40];
    uint32 value, result;
    BeginDraw();
    preview->Draw();
    ClearSceneOrderingTable();
    DrawBackgroundRectangle(0, 0xF00140, flags);
    if (flags == 0x800000)
        DrawText(0x6E00A0, UiText::BossDefeated, 1, 0);
    else
    {
        if (flags == 0x8000)
        {
            sprintf(text, mode != 0 ? UiText::BonusOver : UiText::BonusComplete);
            DrawText(0x6E00A0, text, 1, 0);
        }
        else
            DrawText(0x5000A0, UiText::LevelComplete, 1, 0);
        sprintf(text, UiText::TreasureFormat, percent);
        DrawText(0x8200A0, text, 1, 0);
    }
    if ((sint32)preview->position[2] < 20481)
    {
        DrawTexturedQuad(0xC80037, 0, interface_resources->textures + 8, 0);
        if (next_level == 96)
            DrawText(0xCA005A, UiText::BoarderRun, 0, 0);
        else
        {
            DrawText(0x96001A, UiText::Password, 0, 0);
            DrawPassword(140, 148, 0);
            DrawText(0xCA005A, UiText::Continue, 0, 0);
        }
    }
    UpdateResourceMaterialAnimations();
    resources = menu_state.resources;
    if (resources != 0)
    {
        uint32 tick = frame_clock.render_tick;
        sint32 count = (sint32)resources->header.texture_count;
        sint32 index = 0;
        frame_clock.render_tick = frame_clock.menu_tick;
        while (index < count)
        {
            UpdateMaterialAnimationList(menu_state.material_groups[index].animation_entries);
            count = (sint32)menu_state.resources->header.texture_count;
            ++index;
        }
        frame_clock.render_tick = tick;
    }
    value = menu_state.fade_frames;
    if (value != 0)
    {
        --value;
        menu_state.fade_frames = value;
        if (value != 0)
            DrawOverlayRectangle(0, 0xF00140, (sint32)(value << 12) / 15, 0xFFFFFF);
    }
    EndDraw();
    result = menu_state.pacer.Wait(menu_state.last_frame);
    menu_state.last_frame = result;
    return result;
}

uint32 ShowLevelCompletion(uint32 level, sint8 mode)
{
    FUNCTION_MARKER(0x8006E344, "SLUS_005.78");
    CharacterPreview preview;
    uint32 flags, loaded, tick;
    const SoundBankStorage *table;
    const SoundSample *sound;
    sint32 next_level, percent, shown;
    if (level - 75u < 4)
    {
        next_level = 0;
        flags = 0x800000;
        shown = 0;
        percent = -1;
    }
    else
    {
        next_level = level - 96u < 2 ? 0 : -1;
        flags = level - 96u < 2 ? 0x8000 : 0;
        shown = 1;
        percent = 0;
    }
    GeneratePassword();
    if (level_resource.treasure_total == 0)
        level_resource.treasure_total = 1;
    if (shown != 0)
    {
        uint32 collected = CountSceneTreasure();
        uint32 total, numerator;
        uint16 cumulative;
        cumulative = (uint16)(level_resource.collected_total + collected);
        total = level_resource.treasure_total;
        numerator = 100u * cumulative + (total >> 1);
        if (total == 0)
        {
            fprintf(stderr, "P2: Original division-by-zero trap in 8006E344\n");
            abort();
        }
        percent = (sint32)numerator / (sint32)total;
        shown = 0;
        level_resource.collected_total = cumulative;
        if (percent >= 101)
            percent = 100;
        if (next_level != 0)
            next_level = percent >= 80 || (game_flow.mode_flags & 2) != 0 ? 96 : 0;
    }
    FadeMenuImage(flags);
    ResetSceneStreams();
    loaded = (uint32)PollLevelLoadResult(next_level);
    preview.object = PrepareCompletionCharacter();
    preview.position[1] = (uint32)-14336;
    preview.position[0] = 0;
    preview.position[2] = 430080;
    preview.velocity[1] = 0;
    preview.velocity[0] = 0;
    preview.velocity[2] = (uint32)-4096;
    menu_state.previous_buttons = ReadGamepadButtons();
    menu_state.pacer.fraction = 30;
    menu_state.last_frame = frame_clock.milliseconds;
    for (;;)
    {
        uint32 height;
        DrawCompletionFrame(shown, flags, next_level, &preview, mode);
        if (shown < percent)
            ++shown;
        height = preview.position[2];
        if ((sint32)height >= 20481)
        {
            height += preview.velocity[2];
            preview.position[2] = height;
            if ((sint32)height < 20481)
                preview.position[2] = 20480;
        }
        else
        {
            uint32 previous = ~menu_state.previous_buttons;
            uint32 pressed = ReadGamepadButtons();
            menu_state.previous_buttons = pressed;
            if ((pressed & previous & 64) != 0)
            {
                sint32 voice;
                table = sound_resources.common_table;
                tick = frame_clock.render_tick;
                sound = 0;
                gameplay_input.release_mask = 64;
                if (table != 0 && (sint32)table->header.count > 25)
                    sound = table->samples + 25;
                frame_clock.render_tick = frame_clock.menu_tick;
                voice = sound_effects.SelectVoice((uint32)-1, sound, 4);
                if (voice >= 0)
                    PlaySoundEffect(sound, (uint16)voice, 0, 0, 0, (sint16)(sound_settings.effects_volume), 0);
                frame_clock.render_tick = tick;
                break;
            }
        }
        if ((loaded & 255) == 0)
            loaded = (uint32)PollLevelLoadResult(next_level);
        frame_clock.menu_tick = frame_clock.menu_tick + 1u;
    }
    if ((loaded & 255) == 0)
    {
        DrawLoadingFrame();
        while (PollLevelLoadResult(next_level) == 0)
        {
        }
    }
    ActivateSceneSection();
    table = sound_resources.scene_table;
    sound = table != 0 && (sint32)table->header.count > 0 ? table->samples : 0;
    sound_effects.Queue(sound, 0, 0, 0, sound_settings.effects_volume, 4);
    return (uint32)next_level;
}
