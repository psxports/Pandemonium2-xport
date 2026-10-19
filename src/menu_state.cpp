#include <stdlib.h>

#include "game.h"

MenuState menu_state = {0, 0, {0}, 0, 0, 0, 0, OriginalDefaults::MenuHighlightStep, {{0}, 0, true}, 0, {{0}}, (uint32)OriginalDefaults::MenuImageAsset};

sint32 RunMainMenu(void)
{
    uint32 frames = 0;
    sint32 selection = 0;
    sint32 result = 1;
    uint32 deadline;
    uint32 old_buttons;
    uint32 pressed;
    const SoundSample *sound;
    const SoundBankStorage *sound_table;
    uint32 saved_frame;
    sint32 voice;
    FUNCTION_MARKER(0x8006E8D8, "SLUS_005.78");
    if (character_controller.loaded_model_mask != 3)
        ReloadBaseCharacterModels(3);
    pressed = ReadGamepadButtons();
    deadline = frame_clock.milliseconds;
    menu_state.previous_buttons = pressed;
    menu_state.pacer.fraction = 30;
    menu_state.fade_frames = 15;
    menu_state.last_frame = deadline;
    deadline += 30000;
    for (;;)
    {
        LoadMenuImage(255);
        DrawMainMenu((uint32)selection);
        ++frames;
        pressed = 0;
        if ((sint32)frames >= 15)
        {
            old_buttons = ~menu_state.previous_buttons;
            menu_state.previous_buttons = ReadGamepadButtons();
            pressed = menu_state.previous_buttons & old_buttons;
        }
        if (pressed & 0x5000)
        {
            sound = 0;
            saved_frame = frame_clock.render_tick;
            sound_table = sound_resources.common_table;
            if (sound_table != 0 && (sint32)sound_table->header.count > 24)
                sound = sound_table->samples + 24;
            frame_clock.render_tick = frame_clock.menu_tick;
            voice = sound_effects.SelectVoice(0xFFFFFFFF, sound, 4);
            if (voice >= 0)
                PlaySoundEffect(sound, (uint16)voice, 0, 0, 0, (sint16)(sound_settings.effects_volume), 0);
            frame_clock.render_tick = saved_frame;
            if (pressed & 0x3000)
            {
                --selection;
                if (selection < 0)
                    selection = 3;
            }
            else
            {
                ++selection;
                if (selection == 4)
                    selection = 0;
            }
        }
        else if (pressed & 0x40)
        {
            sound = 0;
            saved_frame = frame_clock.render_tick;
            sound_table = sound_resources.common_table;
            if (sound_table != 0 && (sint32)sound_table->header.count > 25)
                sound = sound_table->samples + 25;
            frame_clock.render_tick = frame_clock.menu_tick;
            voice = sound_effects.SelectVoice(0xFFFFFFFF, sound, 4);
            if (voice >= 0)
                PlaySoundEffect(sound, (uint16)voice, 0, 0, 0, (sint16)(sound_settings.effects_volume), 0);
            frame_clock.render_tick = saved_frame;
            switch (selection)
            {
                case 0:
                    ResetNewGameCharacter();
                    game_flow.requested_level = 0;
                    goto finish;
                case 2:
                    if ((EditPassword(1) << 24) != 0)
                        goto finish;
                    break;
                case 1:
                    RunOptionsMenu();
                    break;
                default:
                    game_flow.attract_phase = 2;
                    result = 0;
                    goto finish;
            }
        }
        if (pressed != 0 || menu_state.previous_buttons != 0)
            deadline = menu_state.last_frame + 30000;
        if (menu_state.last_frame >= deadline)
            break;
        frame_clock.menu_tick = frame_clock.menu_tick + 1;
    }
    result = 0;
finish:
    FadeMenuImage(0);
    sound_effects.Stop(0);
    return result;
}

uint32 DrawMainMenu(uint32 selection)
{
    const MenuMaterialResources *resources;
    uint32 position;
    const TextureDescriptor *group;
    uint32 saved_frame;
    uint32 index;
    uint32 fade;
    uint32 result;
    FUNCTION_MARKER(0x8006E6FC, "SLUS_005.78");
    BeginDraw();
    ClearSceneOrderingTable();
    RestoreMenuImage(0);
    if (selection == 0)
    {
        position = 0x640064;
        group = menu_state.resources->textures + 2;
    }
    else if (selection == 1)
    {
        position = 0x7D0073;
        group = menu_state.resources->textures + 3;
    }
    else if (selection == 2)
    {
        position = 0x960063;
        group = menu_state.resources->textures + 4;
    }
    else
    {
        position = 0xB00070;
        group = menu_state.resources->textures + selection + 2;
    }
    DrawTexturedQuad(position, 0, group, 0);
    UpdateResourceMaterialAnimations();
    resources = menu_state.resources;
    if (resources != 0)
    {
        saved_frame = frame_clock.render_tick;
        frame_clock.render_tick = frame_clock.menu_tick;
        if ((sint32)resources->header.texture_count > 0)
        {
            index = 0;
            do
            {
                UpdateMaterialAnimationList(menu_state.material_groups[index].animation_entries);
                ++index;
            } while ((sint32)index < (sint32)menu_state.resources->header.texture_count);
        }
        frame_clock.render_tick = saved_frame;
    }
    fade = menu_state.fade_frames;
    if (fade != 0)
    {
        --fade;
        menu_state.fade_frames = fade;
        if (fade != 0)
            DrawOverlayRectangle(0, 0xF00140, (sint32)(fade << 12) / 15, 0xFFFFFF);
    }
    EndDraw();
    result = menu_state.pacer.Wait(menu_state.last_frame);
    menu_state.last_frame = result;
    return result;
}

uint32 RunOptionsMenu(void)
{
    sint32 sound_volume;
    sint32 music_volume;
    sint32 selection = 0;
    sint32 preview_voice = -1;
    sint32 voice;
    sint32 direction;
    const SoundSample *preview_sound = 0;
    const SoundSample *sound;
    const SoundBankStorage *sound_table;
    uint32 saved_frame;
    uint32 old_buttons;
    uint32 buttons;
    uint32 pressed;
    uint32 clock;
    FUNCTION_MARKER(0x8006CE38, "SLUS_005.78");
    FadeMenuImage(0);
    sound_table = sound_resources.common_table;
    sound_volume = (sint16)(sound_settings.effects_volume);
    if (sound_table != 0 && (sint32)sound_table->header.count > 0)
        preview_sound = sound_table->samples;
    music_volume = (sint16)(sound_settings.music_volume);
    clock = frame_clock.milliseconds;
    menu_state.pacer.fraction = 30;
    menu_state.fade_frames = 15;
    menu_state.last_frame = clock;
    for (;;)
    {
        LoadMenuImage(257);
        DrawOptionsMenu(selection, music_volume, sound_volume);
        old_buttons = ~menu_state.previous_buttons;
        buttons = ReadGamepadButtons();
        menu_state.previous_buttons = buttons;
        pressed = buttons & old_buttons;
        if (pressed & 0x10)
        {
            sound = 0;
            saved_frame = frame_clock.render_tick;
            sound_table = sound_resources.common_table;
            if (sound_table != 0 && (sint32)sound_table->header.count > 25)
                sound = sound_table->samples + 25;
            frame_clock.render_tick = frame_clock.menu_tick;
            voice = sound_effects.SelectVoice(0xFFFFFFFF, sound, 4);
            if (voice >= 0)
                PlaySoundEffect(sound, (uint16)voice, 0, 0, 0, (sint16)(sound_settings.effects_volume), 0);
            frame_clock.render_tick = saved_frame;
            break;
        }
        if (pressed & 0xA000)
        {
            sound = 0;
            saved_frame = frame_clock.render_tick;
            sound_table = sound_resources.common_table;
            if (sound_table != 0 && (sint32)sound_table->header.count > 24)
                sound = sound_table->samples + 24;
            frame_clock.render_tick = frame_clock.menu_tick;
            voice = sound_effects.SelectVoice(0xFFFFFFFF, sound, 4);
            if (voice >= 0)
                PlaySoundEffect(sound, (uint16)voice, 0, 0, 0, (sint16)(sound_settings.effects_volume), 0);
            frame_clock.render_tick = saved_frame;
            if (pressed & 0x3000)
            {
                --selection;
                if (selection < 0)
                    selection = 2;
            }
            else
            {
                ++selection;
                if (selection == 3)
                    selection = 0;
            }
        }
        else if (pressed & 0x5000)
        {
            direction = (pressed & 0x1000) ? -1 : 1;
            if (selection == 1)
            {
                music_volume = sound_settings.AdjustMusic(-direction);
                menu_state.previous_buttons = menu_state.previous_buttons & 0xFFFFAFFF;
            }
            else if (selection == 2)
            {
                sound = 0;
                saved_frame = frame_clock.render_tick;
                sound_table = sound_resources.common_table;
                if (sound_table != 0 && (sint32)sound_table->header.count > 25)
                    sound = sound_table->samples + 25;
                frame_clock.render_tick = frame_clock.menu_tick;
                voice = sound_effects.SelectVoice(0xFFFFFFFF, sound, 4);
                if (voice >= 0)
                    PlaySoundEffect(sound, (uint16)voice, 0, 0, 0, (sint16)(sound_settings.effects_volume), 0);
                frame_clock.render_tick = saved_frame;
                {
                    uint32 cursor;
                    if (pressed & 0x1000)
                    {
                        cursor = (control_bindings.selected_index - 1) & 255;
                        control_bindings.selected_index = (uint8)cursor;
                        if (cursor & 0x80)
                            control_bindings.selected_index = 23;
                    }
                    else
                    {
                        cursor = (control_bindings.selected_index + 1) & 255;
                        control_bindings.selected_index = (uint8)cursor;
                        if (cursor >= 24)
                            control_bindings.selected_index = 0;
                    }
                    control_bindings.active_scheme = &OriginalDefaults::ControlSchemes[control_bindings.selected_index];
                }
            }
            else if (selection == 0)
            {
                saved_frame = frame_clock.render_tick;
                menu_state.previous_buttons = buttons & 0xFFFFAFFF;
                frame_clock.render_tick = frame_clock.menu_tick;
                sound_volume = sound_settings.AdjustEffects(-direction);
                if ((sound_effects.IsPlaying(preview_sound, (uint32)preview_voice) << 24) != 0)
                    SetSoundVoiceVolume((uint16)preview_voice, (uint16)(sound_volume >> 4));
                else
                {
                    preview_voice = sound_effects.SelectVoice(0xFFFFFFFF, preview_sound, 4);
                    if (preview_voice != -1)
                        PlaySoundEffect(preview_sound, (uint16)preview_voice, 0, 0, 20010, sound_volume, 0);
                }
                frame_clock.render_tick = saved_frame;
            }
        }
        frame_clock.menu_tick = frame_clock.menu_tick + 1;
    }
    FadeMenuImage(0);
    menu_state.fade_frames = 15;
    return 0;
}

uint32 DrawOptionsMenu(sint32 selection, sint32 music_volume, sint32 sound_volume)
{
    const MenuMaterialResources *resources;
    uint32 position;
    uint32 group_index;
    const TextureDescriptor *groups;
    uint32 texture_count;
    uint32 icon;
    uint32 button;
    const TextureDescriptor *table;
    uint32 texture_index;
    uint32 index;
    uint32 saved_frame;
    uint32 fade;
    uint32 result;
    FUNCTION_MARKER(0x8006C7B4, "SLUS_005.78");
    DrawSync(0);
    VSync(0);
    BeginDraw();
    ClearSceneOrderingTable();
    RestoreMenuImage(0);
    if (selection == 2)
        position = 0x7A007E;
    else
        position = selection == 0 ? 0x650036 : 0x6400DC;
    /* The reviewed options caller supplies selection 0..2 */
    if (selection == 1)
        group_index = 8;
    else if (selection >= 2)
    {
        if (selection == 2)
            group_index = 7;
    }
    else if (selection == 0)
        group_index = 6;
    groups = menu_state.resources->textures;
    DrawTexturedQuad(position, 0, groups + group_index, 0);
    groups = menu_state.resources->textures;
    if (groups != 0)
    {
        position = ((uint32)(67 - ((sint32)(50 * (uint32)sound_volume) >> 12)) << 16) | 0x44;
        DrawTexturedQuad(position, 0, groups, 0);
    }
    if (groups != 0)
    {
        position = ((uint32)(67 - ((sint32)(50 * (uint32)music_volume) >> 12)) << 16) | 0xDC;
        DrawTexturedQuad(position, 0, groups, 0);
    }
    for (icon = 0; icon < 3; ++icon)
    {
        index = icon == 0 ? 1 : icon == 1 ? 0 : 2;
        button = (*control_bindings.active_scheme)[index];
        switch (button)
        {
            case 128:
                texture_index = 9;
                break;
            case 16:
                texture_index = 11;
                break;
            case 64:
                texture_index = 8;
                break;
            default:
                texture_index = 10;
                break;
        }
        table = interface_resources->textures;
        if (table + texture_index != 0)
        {
            button = (*control_bindings.active_scheme)[index];
            switch (button)
            {
                case 128:
                    texture_index = 9;
                    break;
                case 16:
                    texture_index = 11;
                    break;
                case 64:
                    texture_index = 8;
                    break;
                default:
                    texture_index = 10;
                    break;
            }
            table = interface_resources->textures;
            position = icon == 0 ? 0x90007D : icon == 1 ? 0xAA007D : 0xC5007D;
            DrawTexturedQuad(position, 0, table + texture_index, 0);
        }
    }
    UpdateResourceMaterialAnimations();
    resources = menu_state.resources;
    if (resources != 0)
    {
        saved_frame = frame_clock.render_tick;
        texture_count = resources->header.texture_count;
        frame_clock.render_tick = frame_clock.menu_tick;
        if ((sint32)texture_count > 0)
        {
            index = 0;
            do
            {
                UpdateMaterialAnimationList(menu_state.material_groups[index].animation_entries);
                ++index;
            } while ((sint32)index < (sint32)menu_state.resources->header.texture_count);
        }
        frame_clock.render_tick = saved_frame;
    }
    fade = menu_state.fade_frames;
    if (fade != 0)
    {
        --fade;
        menu_state.fade_frames = fade;
        if (fade != 0)
            DrawOverlayRectangle(0, 0xF00140, (sint32)(fade << 12) / 15, 0xFFFFFF);
    }
    EndDraw();
    result = menu_state.pacer.Wait(menu_state.last_frame);
    menu_state.last_frame = result;
    return result;
}

uint32 DrawQuitConfirmation(uint32 selection, uint32 mode)
{
    FUNCTION_MARKER(0x8006D30C, "SLUS_005.78");
    uint32 buffer, position, extent, pointer, result;
    sint32 fraction;
    buffer = render_commands.other_packet_base;
    render_commands.overlay_head = (0);
    render_commands.packet_offset = (buffer);
    DrawOverlayRectangle(0, 0x00F00140, 4096, 0xFFFFFF);
    if (mode)
    {
        DrawText(0x007800A0, UiText::QuitGame, 1, 0);
        DrawText(0x009600A0, UiText::QuitChoices, 1, 0);
    }
    else
        DrawText(0x007800A0, UiText::ContinueQuit, 1, 0);
    if (!selection)
    {
        position = mode ? 0x00930067 : 0x007500C0;
        extent = mode ? 0x0014002E : 0x0014003E;
    }
    else
    {
        position = mode ? 0x009300AB : 0x00750035;
        extent = mode ? 0x00140022 : 0x00140073;
    }
    {
        uint32 phase = menu_state.highlight_phase;
        uint32 step = menu_state.highlight_step;
        phase += step;
        menu_state.highlight_phase = phase;
        if ((sint32)phase >= 2048)
        {
            menu_state.highlight_phase = 2048;
            menu_state.highlight_step = 0 - step;
        }
        else if ((sint32)phase < -2047)
        {
            menu_state.highlight_phase = (uint32)-2048;
            menu_state.highlight_step = 0 - step;
        }
        fraction = (sint32)menu_state.highlight_phase;
    }
    if (fraction > 0)
        DrawBrightenRectangle(position, extent, fraction, 0x00808080);
    else if (fraction < 0)
        DrawOverlayRectangle(position, extent, (sint32)(0 - (uint32)fraction), 0x00808080);
    if (!selection)
        DrawText(0x00CA005A, UiText::Quit, 0, 0);
    else
        DrawText(0x00CA005A, UiText::Continue, 0, 0);
    DrawTexturedQuad(0x00C80037, 0, interface_resources->textures + 8, 0);
    pointer = render_commands.overlay_tail;
    w_u32(pointer, r_u32(pointer) | 0xFFFFFF);
    SubmitOverlayFrame(render_commands.other_packet_base, 0);
    result = menu_state.pacer.Wait(menu_state.last_frame);
    menu_state.last_frame = result;
    return result;
}

uint32 RunQuitConfirmation(uint32 mode)
{
    FUNCTION_MARKER(0x8006D678, "SLUS_005.78");
    uint32 current, previous, pressed, tick, value;
    const SoundBankStorage *table;
    const SoundSample *sound;
    sint32 selection, countdown, voice;
    current = ReadGamepadButtons();
    value = frame_clock.milliseconds;
    countdown = 0;
    selection = 1;
    menu_state.previous_buttons = current;
    menu_state.pacer.fraction = 30;
    menu_state.last_frame = value;
    for (;;)
    {
        DrawQuitConfirmation((uint32)selection, mode);
        pressed = 0;
        if (!countdown)
        {
            previous = menu_state.previous_buttons;
            current = ReadGamepadButtons();
            menu_state.previous_buttons = current;
            pressed = current & ~previous;
        }
        if (pressed & 0xA000)
        {
            sound = 0;
            table = sound_resources.common_table;
            tick = frame_clock.render_tick;
            if (table && (sint32)table->header.count > 24)
                sound = table->samples + 24;
            frame_clock.render_tick = frame_clock.menu_tick;
            voice = sound_effects.SelectVoice(0xFFFFFFFF, sound, 4);
            if (voice >= 0)
                PlaySoundEffect(sound, (uint16)voice, 0, 0, 0, (sint16)(sound_settings.effects_volume), 0);
            frame_clock.render_tick = tick;
            if (pressed & 0x3000)
            {
                --selection;
                if (selection < 0)
                    selection = 1;
            }
            else
            {
                ++selection;
                if (selection == 2)
                    selection = 0;
            }
        }
        else if (pressed & 0x40)
        {
            if (selection)
                return 0;
            sound = 0;
            table = sound_resources.common_table;
            tick = frame_clock.render_tick;
            if (table && (sint32)table->header.count > 25)
                sound = table->samples + 25;
            frame_clock.render_tick = frame_clock.menu_tick;
            voice = sound_effects.SelectVoice(0xFFFFFFFF, sound, 4);
            if (voice >= 0)
                PlaySoundEffect(sound, (uint16)voice, 0, 0, 0, (sint16)(sound_settings.effects_volume), 0);
            frame_clock.render_tick = tick;
            if (!mode && !(RunQuitConfirmation(1) & 0xFF))
                return 0;
            if (mode != 1)
                return 1;
            countdown = 10;
        }
        if (countdown)
        {
            --countdown;
            if (countdown <= 0)
                return 1;
        }
        frame_clock.menu_tick = frame_clock.menu_tick + 1;
    }
}

uint32 DrawPauseMenu(void)
{
    FUNCTION_MARKER(0x8006D8EC, "SLUS_005.78");
    uint32 buffer, pointer, tick, current_tick, count, index, elapsed, first_ot, result;
    const MenuMaterialResources *objects;
    uint32 *entries;
    buffer = render_commands.other_packet_base;
    render_commands.overlay_head = (0);
    render_commands.packet_offset = (buffer);
    render_commands.packet_limit = (buffer + 41600);
    DrawOverlayRectangle(0, 0x00F00140, 1638, 0xFFFFFF);
    DrawHud(FindPlayerObject(0));
    DrawText(0x007800A0, UiText::Paused, 1, 0);
    if (!level_resource.level_id)
        DrawOverlayRectangle(0x00C30000, 0x001D0140, 4096, 0xFFFFFF);
    DrawText(0x00CA00A0, UiText::SelectQuit, 1, 0);
    pointer = render_commands.overlay_tail;
    w_u32(pointer, r_u32(pointer) | 0xFFFFFF);
    UpdateResourceMaterialAnimations();
    objects = menu_state.resources;
    if (objects)
    {
        tick = frame_clock.render_tick;
        current_tick = frame_clock.menu_tick;
        count = objects->header.texture_count;
        frame_clock.render_tick = current_tick;
        if ((sint32)count > 0)
        {
            count = 0;
            index = 0;
            do
            {
                entries = menu_state.material_groups[index / 8].animation_entries;
                index += 8;
                UpdateMaterialAnimationList(entries);
                objects = menu_state.resources;
                elapsed = objects->header.texture_count;
                ++count;
            } while ((sint32)count < (sint32)elapsed);
        }
        frame_clock.render_tick = tick;
    }
    elapsed = menu_state.fade_frames;
    if (elapsed)
    {
        --elapsed;
        menu_state.fade_frames = elapsed;
        if (elapsed)
            DrawOverlayRectangle(0, 0x00F00140, (sint32)(elapsed << 12) / 15, 0xFFFFFF);
    }
    first_ot = render_commands.submitted_chain_offset;
    buffer = render_commands.other_packet_base;
    SubmitOverlayFrame(first_ot, buffer);
    result = menu_state.pacer.Wait(menu_state.last_frame);
    menu_state.last_frame = result;
    return result;
}

sint32 RunPauseMenu(uint32 level)
{
    FUNCTION_MARKER(0x8006DAF4, "SLUS_005.78");
    uint32 flags, handle, tick, value, pointer, previous, current, pressed;
    const SoundBankStorage *table;
    const SoundSample *sound;
    sint32 voice;
    flags = game_flow.transition_flags;
    handle = sequence_playback.current_handle;
    game_flow.transition_flags = flags | 0x10;
    PauseSequence((uint16)handle);
    StopSceneSounds();
    PauseCharacterSound();
    sound = 0;
    table = sound_resources.common_table;
    tick = frame_clock.render_tick;
    if (table && (sint32)table->header.count > 23)
        sound = table->samples + 23;
    frame_clock.render_tick = frame_clock.menu_tick;
    voice = sound_effects.SelectVoice(0xFFFFFFFF, sound, 4);
    if (voice >= 0)
        PlaySoundEffect(sound, (uint16)voice, 0, 0, 0, (sint16)(sound_settings.effects_volume), 0);
    value = level_resource.level_id;
    frame_clock.render_tick = tick;
    if (!value)
    {
        pointer = hud.prompt_tail;
        if (pointer)
            w_u32(pointer, r_u32(pointer) | 0xFFFFFF);
    }
    current = ReadGamepadButtons();
    value = frame_clock.milliseconds;
    menu_state.previous_buttons = current;
    menu_state.pacer.fraction = 30;
    menu_state.last_frame = value;
    for (;;)
    {
        DrawPauseMenu();
        previous = menu_state.previous_buttons;
        current = ReadGamepadButtons();
        menu_state.previous_buttons = current;
        pressed = current & ~previous;
        if (pressed & 0x800)
            break;
        if (pressed & 0x100)
        {
            sound = 0;
            table = sound_resources.common_table;
            tick = frame_clock.render_tick;
            if (table && (sint32)table->header.count > 25)
                sound = table->samples + 25;
            frame_clock.render_tick = frame_clock.menu_tick;
            voice = sound_effects.SelectVoice(0xFFFFFFFF, sound, 4);
            if (voice >= 0)
                PlaySoundEffect(sound, (uint16)voice, 0, 0, 0, (sint16)(sound_settings.effects_volume), 0);
            frame_clock.render_tick = tick;
            if (RunQuitConfirmation(0) & 0xFF)
            {
                WaitSceneResourceLoad();
                goto finish_8006daf4;
            }
            break;
        }
        UpdateResourceMaterialAnimations();
    }
    table = sound_resources.common_table;
    tick = frame_clock.render_tick;
    sound = 0;
    gameplay_input.release_mask = 64;
    if (table && (sint32)table->header.count > 23)
        sound = table->samples + 23;
    frame_clock.render_tick = frame_clock.menu_tick;
    voice = sound_effects.SelectVoice(0xFFFFFFFF, sound, 4);
    if (voice >= 0)
        PlaySoundEffect(sound, (uint16)voice, 0, 0, 0, (sint16)(sound_settings.effects_volume), 0);
    handle = sequence_playback.current_handle;
    frame_clock.render_tick = tick;
    ResumeSequence((uint16)handle);
    ResumeCharacterSound();
finish_8006daf4:
    game_flow.transition_flags = game_flow.transition_flags & 0xFFFFFFEF;
    return (sint32)sound_effects.Stop(0);
}

uint32 RunPasswordMenu(void)
{
    FUNCTION_MARKER(0x8006D2EC, "SLUS_005.78");
    return EditPassword(0);
}

uint32 DrawMenuSelection(uint32 position, uint32 alternate, sint32 selection)
{
    FUNCTION_MARKER(0x8006B04C, "SLUS_005.78");
    uint32 group;
    if (alternate != 0)
        group = (uint32)selection + 2;
    else
    {
        switch (selection)
        {
            case 0:
                group = 6;
                break;
            case 1:
                group = 8;
                break;
            case 2:
                group = 7;
                break;
            default:
                fprintf(stderr, "P2: Invalid interface selection %d\n", selection);
                abort();
        }
    }
    return DrawTexturedQuad(position, 0, menu_state.resources->textures + group, 0);
}
