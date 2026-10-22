#include <stdio.h>
#include <stdlib.h>

#include "character_preview.h"
#include "game.h"

const PreviewAnimationStep preview_animation_steps[1] = {{2, {-4096, 0}, 1000}};

PreviewPlayback preview_playback = {0};

AnimatedObject *CharacterPreview::PrepareCompletionCharacter(void)
{
    CharacterModelRecord *model_rec;
    FUNCTION_MARKER(0x80053BF0, "SLUS_005.78");
    AnimatedObject *object = character_pad.object;
    AnimationHeader *action;
    AnimatedObject *result = object;
    if (object == 0)
    {
        {
            uint32 model_idx = (((character_pad.motion.form_flags >> 18) & 60)) / 4;
            CharacterModelRecord *model_binding;
            model_binding = character_pad.model_slots[model_idx];
            if (model_binding == 0)
                return 0;
            model_rec = model_binding;
        }
        object = model_rec->object;
        if (object == 0)
            return 0;
        result = object;
        object->contact_kind = 1;
        object->behavior = 1;
        object->behavior_suspended = 0;
        object->spheres[3].radius = -1;
        object->spheres[2].radius = -1;
        object->spheres[1].radius = -1;
        object->spheres[0].radius = -1;
    }
    {
        uint32 model_idx = (((character_pad.motion.form_flags >> 18) & 60)) / 4;
        CharacterModelRecord *model_binding;
        model_binding = character_pad.model_slots[model_idx];
        if (model_binding == 0)
            return 0;
        model_rec = model_binding;
    }
    action = model_rec->animations[2];
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

void CharacterPreview::Update()
{
    CharacterModelRecord *animation_rec;
    uint32 cur;
    AnimationHeader *next;
    const PreviewAnimationStep *step;
    sint32 duration;
    AnimatedObject *preview_object;
    uint32 entry;
    AnimationHeader *old_animation;
    uint32 flags;
    sint32 x;
    sint32 y;
    FUNCTION_MARKER(0x80053CA0, "SLUS_005.78");
    cur = preview_playback.tick;
    if ((sint32)cur < 0)
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
        ++cur;
        preview_playback.tick = cur;
        if (duration < (sint32)cur)
        {
            preview_playback.tick = 0;
            // Advancing past the sole step would read the following sine table
            preview_playback.cur_step = preview_playback.first_step;
        }
    }
    entry = character_pad.motion.form_flags;
    {
        uint32 model_idx = (((entry >> 18) & 0x3C)) / 4;
        CharacterModelRecord *model_binding;
        model_binding = character_pad.model_slots[model_idx];
        if (model_binding == 0)
        {
            object = 0;
            return;
        }
        animation_rec = model_binding;
    }
    preview_object = animation_rec->object;
    if (preview_object == 0)
    {
        object = 0;
        return;
    }
    step = preview_playback.cur_step;
    cur = step->animation;
    old_animation = preview_object->animation;
    next = animation_rec->animations[cur];
    if (preview_object->unbound_animation_word != 0 || old_animation != next)
    {
        flags = preview_object->flags;
        preview_object->animation = next;
        preview_object->unbound_animation_word = 0;
        preview_object->frame = -1;
        preview_object->flags = flags | 0x10000;
        velocity.x = step->velocity[0];
        velocity.y = step->velocity[1];
    }
    y = pos.y + velocity.y;
    x = pos.x + velocity.x;
    pos.y = y;
    pos.x = x;
    if (x > 143359)
    {
        pos.x = 143360;
        velocity.x = -velocity.x;
    }
    else
    {
        if (x <= -143360)
        {
            pos.x = -143360;
            velocity.x = -velocity.x;
        }
    }
    object = preview_object;
}

void CharacterPreview::DrawLoadingFrame(void)
{
    FUNCTION_MARKER(0x80066168, "SLUS_005.78");
    const TextureDescriptor *image;
    uint32 idx;
    RenderCommands::BeginDraw();
    RenderCommands::ClearSceneOrderingTable();
    ScreenRenderState::DrawBackgroundRect(0, 0xF00140, 0);
    image = resource_grps[2].tex;
    for (idx = 0; idx < 2; ++idx)
    {
        sint32 width = image->mapping.right - image->mapping.left;
        sint32 height = image->mapping.bottom - image->mapping.top;
        uint32 x = (319 - width) >> 1;
        uint32 y = (239 - height) >> 1;
        MaterialCatalog::DrawTexturedQuad(x | (y << 16), 0, image, 0);
    }
    RenderCommands::EndDraw();
}

void CharacterPreview::Draw()
{
    SceneSection local_section = {0};
    AnimatedObject *preview_object;
    SceneNode *spr;
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
    uint32 proj;
    sint32 frame;
    sint32 count;
    uint8 byte_flags;
    FUNCTION_MARKER(0x8006DE08, "SLUS_005.78");
    light = &local_light;
    preview_object = object;
    if (preview_object == 0)
        return;
    byte_flags = preview_object->node.flags;
    flags = preview_object->flags;
    preview_object->movement_mode = 0;
    preview_object->node.flags = byte_flags & 0xFD;
    saved_context = scene_stream.active_section;
    preview_object->path_definition = 0;
    preview_object->flags = flags | 0x2000;
    scene_stream.active_section = &local_section;
    scene_lighting.lights = 0;
    scene_lighting.ambient[2] = 128;
    scene_lighting.ambient[1] = 128;
    scene_lighting.ambient[0] = 128;
    camera_motion.pos.y = 0;
    camera_motion.pos.x = 0;
    camera_motion.pos.z = -49152;
    camera_motion.focus_pos.z = 0;
    camera_motion.focus_pos.y = 0;
    camera_motion.focus_pos.x = 0;

    preview_object->world_pos = camera_motion.focus_pos;
    preview_object->bounds.radius = 40960;
    object->node.Register();

    preview_object->pos = camera_motion.focus_pos;
    x = preview_object->pos.x;
    y = (uint32)pos.x;
    preview_object->pos.x = x + y;
    x = preview_object->pos.y;
    y = (uint32)pos.y;
    preview_object->pos.y = x + y;
    x = preview_object->pos.z;
    y = (uint32)pos.z;
    preview_object->pos.z = x + y;
    preview_object->orientation[0] = 0x7FFF;
    z = (uint32)velocity.z;
    x = (uint32)velocity.x;
    angle = Atan2Angle((-z), (-x));
    animation = preview_object->animation;
    preview_object->orientation[2] = (angle + 2048) & 0xFFF;
    frame = 0;
    if (animation != 0)
    {
        frame = preview_object->frame;
        count = animation->frame_count;
        frame = frame < count - 1 ? frame + 1 : 0;
    }
    preview_object->frame = frame;
    preview_object->ClampAnimationFrame();
    proj = camera_proj.scale;
    light->src.dir.vz = -4096;
    light->color[0] = 2056;
    light->color[1] = 2056;
    light->color[2] = 2056;
    light->links.registry_next = 0;
    light->flags = 0;
    light->event_flags = 0;
    light->src.dir.vy = 0;
    light->src.dir.vx = 0;
    scene_lighting.lights = light;
    if (proj != 955733 || camera_proj.view_normals[0].vz != 4096)
    {
        camera_proj.scale = 955733;
        CameraProjection::SetCameraProj(955733);
    }
    byte_flags = camera_motion.flags;
    camera_proj.far_distance = 1228800;
    camera_motion.flags = byte_flags & 0xFE;
    child = preview_object->spr;
    spr = (SceneNode *)child;
    preview_object->spr = 0;
    if (spr != 0)
        spr->flags |= 1;
    CameraProjection::DrawSceneFrame(0);
    preview_object->spr = child;
    if (spr != 0)
        spr->flags &= 0xFE;
    object->node.Unregister();
    preview_object->movement_mode = 1;
    scene_stream.active_section = saved_context;
    scene_lighting.lights = saved_lights;
}

void CharacterPreview::DrawCompletionFrame(sint32 percent, uint32 flags, sint32 next_level, CharacterPreview *preview, sint8 mode)
{
    const MenuMaterialResources *resources;
    FUNCTION_MARKER(0x8006E098, "SLUS_005.78");
    char text[40];
    uint32 val, result;
    RenderCommands::BeginDraw();
    preview->Draw();
    RenderCommands::ClearSceneOrderingTable();
    ScreenRenderState::DrawBackgroundRect(0, 0xF00140, flags);
    if (flags == 0x800000)
        UiText::DrawText(0x6E00A0, UiText::boss_defeated, 1, 0);
    else
    {
        if (flags == 0x8000)
        {
            sprintf(text, mode != 0 ? UiText::bonus_over : UiText::bonus_complete);
            UiText::DrawText(0x6E00A0, text, 1, 0);
        }
        else
            UiText::DrawText(0x5000A0, UiText::level_complete, 1, 0);
        sprintf(text, UiText::treasure_format, percent);
        UiText::DrawText(0x8200A0, text, 1, 0);
    }
    if (preview->pos.z < 20481)
    {
        MaterialCatalog::DrawTexturedQuad(0xC80037, 0, interface_resources->texs + 8, 0);
        if (next_level == 96)
            UiText::DrawText(0xCA005A, UiText::boarder_run, 0, 0);
        else
        {
            UiText::DrawText(0x96001A, UiText::password, 0, 0);
            MenuState::DrawPassword(140, 148, 0);
            UiText::DrawText(0xCA005A, UiText::continue_text, 0, 0);
        }
    }
    MaterialRegistry::UpdateResourceMaterialAnimations();
    resources = menu_state.resources;
    if (resources != 0)
    {
        uint32 tick = frame_clock.render_tick;
        sint32 count = resources->header.tex_count;
        sint32 idx = 0;
        frame_clock.render_tick = frame_clock.menu_tick;
        while (idx < count)
        {
            MaterialAnimationList::UpdateMaterialAnimationList(menu_state.material_grps[idx].animation_entries);
            count = menu_state.resources->header.tex_count;
            ++idx;
        }
        frame_clock.render_tick = tick;
    }
    val = menu_state.fade_frames;
    if (val != 0)
    {
        --val;
        menu_state.fade_frames = val;
        if (val != 0)
            ScreenRenderState::DrawOverlayRect(0, 0xF00140, (sint32)(val << 12) / 15, 0xFFFFFF);
    }
    RenderCommands::EndDraw();
    result = menu_state.pacer.Wait(menu_state.last_frame);
    menu_state.last_frame = result;
}

uint32 CharacterPreview::ShowLevelCompletion(uint32 level, sint8 mode)
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
    MenuState::GeneratePassword();
    if (level_resource.treasure_total == 0)
        level_resource.treasure_total = 1;
    if (shown != 0)
    {
        uint32 collected = SceneLoader::CountSceneTreasure();
        uint32 total;
        sint32 numerator;
        uint16 cumulative;
        cumulative = level_resource.collected_total + collected;
        total = level_resource.treasure_total;
        numerator = 100u * cumulative + (total >> 1);
        if (total == 0)
            trap(7);
        percent = numerator / (sint32)total;
        shown = 0;
        level_resource.collected_total = cumulative;
        if (percent >= 101)
            percent = 100;
        if (next_level != 0)
            next_level = percent >= 80 || (game_flow.mode_flags & 2) != 0 ? 96 : 0;
    }
    ScreenRenderState::FadeMenuImage(flags);
    SceneStream::ResetSceneStreams();
    loaded = LevelLoader::PollLevelLoadResult(next_level);
    preview.object = CharacterPreview::PrepareCompletionCharacter();
    preview.pos.y = -14336;
    preview.pos.x = 0;
    preview.pos.z = 430080;
    preview.velocity.y = 0;
    preview.velocity.x = 0;
    preview.velocity.z = -4096;
    menu_state.prev_buttons = GameplayInput::ReadGamepadButtons();
    menu_state.pacer.fraction = 30;
    menu_state.last_frame = frame_clock.milliseconds;
    for (;;)
    {
        sint32 height;
        CharacterPreview::DrawCompletionFrame(shown, flags, next_level, &preview, mode);
        if (shown < percent)
            ++shown;
        height = preview.pos.z;
        if (height >= 20481)
        {
            height += preview.velocity.z;
            preview.pos.z = height;
            if (height < 20481)
                preview.pos.z = 20480;
        }
        else
        {
            uint32 prev = ~menu_state.prev_buttons;
            uint32 pressed = GameplayInput::ReadGamepadButtons();
            menu_state.prev_buttons = pressed;
            if ((pressed & prev & 64) != 0)
            {
                sint32 voice;
                table = sound_resources.common_table;
                tick = frame_clock.render_tick;
                sound = 0;
                gameplay_input.release_mask = 64;
                if (table != 0 && table->header.count > 25)
                    sound = table->samples + 25;
                frame_clock.render_tick = frame_clock.menu_tick;
                voice = sound_effects.SelectVoice(-1, sound, 4);
                if (voice >= 0)
                    SoundEffects::PlaySoundEffect(sound, voice, 0, 0, 0, sound_settings.effects_volume, 0);
                frame_clock.render_tick = tick;
                break;
            }
        }
        if ((loaded & 255) == 0)
            loaded = LevelLoader::PollLevelLoadResult(next_level);
        frame_clock.menu_tick = frame_clock.menu_tick + 1u;
    }
    if ((loaded & 255) == 0)
    {
        CharacterPreview::DrawLoadingFrame();
        while (LevelLoader::PollLevelLoadResult(next_level) == 0)
        {
        }
    }
    SceneStream::ActivateSceneSection();
    table = sound_resources.scene_table;
    sound = table != 0 && table->header.count > 0 ? table->samples : 0;
    sound_effects.Queue(sound, 0, 0, 0, (uint16)sound_settings.effects_volume, 4);
    return next_level;
}
