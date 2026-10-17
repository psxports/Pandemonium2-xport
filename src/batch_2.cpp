#include "game.h"
#include <string.h>

uint32 p2_func_8002a3e0(uint32 mode, uint32 count)
{
    uint32 entries;
    uint32 bytes;
    uint32 allocation;
    FUNCTION_MARKER(0x8002A3E0, "SLUS_005.78");
    entries = (uint32)(sint16)(count + 1);
    bytes = entries << 2;
    allocation = p2_func_80064d28(bytes);
    p2_func_80066358(allocation, 255, bytes);
    w_u32(allocation, entries - 1);
    return allocation + 4;
}

uint32 p2_func_800750c4(sint16 sequence, sint16 separation)
{
    FUNCTION_MARKER(0x800750C4, "SLUS_005.78");
    return p2_sdk_800750f0(sequence, separation);
}

sint32 p2_func_8006e8d8(void)
{
    uint32 frames = 0;
    sint32 selection = 0;
    sint32 result = 1;
    uint32 deadline;
    uint32 old_buttons;
    uint32 pressed;
    uint32 sound;
    uint32 saved_frame;
    sint32 voice;
    FUNCTION_MARKER(0x8006E8D8, "SLUS_005.78");
    if (r_u32(0x8008A4FC) != 3)
        ReloadBaseCharacterModels(3);
    pressed = p2_func_80066d08();
    deadline = frame_clock.milliseconds;
    menu_state.previous_buttons = pressed;
    menu_state.pacer.fraction = 30;
    menu_state.fade_frames = 15;
    menu_state.last_frame = deadline;
    deadline += 30000;
    for (;;)
    {
        p2_func_80066de4(255);
        p2_func_8006e6fc((uint32)selection);
        ++frames;
        pressed = 0;
        if ((sint32)frames >= 15)
        {
            old_buttons = ~menu_state.previous_buttons;
            menu_state.previous_buttons = p2_func_80066d08();
            pressed = menu_state.previous_buttons & old_buttons;
        }
        if (pressed & 0x5000)
        {
            sound = 0;
            saved_frame = frame_clock.render_tick;
            if (r_u32(0x8008A408) != 0 && r_s32(r_u32(0x8008A408)) > 24)
                sound = r_u32(0x8008A408) + 196;
            frame_clock.render_tick = frame_clock.menu_tick;
            voice = sound_effects.SelectVoice(0xFFFFFFFF, sound, 4);
            if (voice >= 0)
                p2_func_8005e470(sound, (uint16)voice, 0, 0, 0, (sint16)(sound_settings.effects_volume), 0);
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
            if (r_u32(0x8008A408) != 0 && r_s32(r_u32(0x8008A408)) > 25)
                sound = r_u32(0x8008A408) + 204;
            frame_clock.render_tick = frame_clock.menu_tick;
            voice = sound_effects.SelectVoice(0xFFFFFFFF, sound, 4);
            if (voice >= 0)
                p2_func_8005e470(sound, (uint16)voice, 0, 0, 0, (sint16)(sound_settings.effects_volume), 0);
            frame_clock.render_tick = saved_frame;
            switch (selection)
            {
                case 0:
                    p2_func_800558f0();
                    w_u8(0x8008A134, 0);
                    goto finish;
                case 2:
                    if ((EditPassword(1) << 24) != 0)
                        goto finish;
                    break;
                case 1:
                    p2_func_8006ce38();
                    break;
                default:
                    w_u32(0x8008A294, 2);
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
    p2_func_80067160(0);
    sound_effects.Stop(0);
    return result;
}

uint32 p2_func_80066d08(void)
{
    uint32 high;
    uint32 low;
    FUNCTION_MARKER(0x80066D08, "SLUS_005.78");
    if (r_u8(0x800A5C28) == 255 || (r_u8(0x800A5C29) & 0xF0) != 0x40)
        return 0;
    high = r_u8(0x800A5C2A);
    low = r_u8(0x800A5C2B);
    return ((high << 8) | low) ^ 0xFFFF;
}

uint32 p2_func_80066de4(uint32 asset)
{
    uint32 result;
    uint32 offset;
    uint32 image;
    FUNCTION_MARKER(0x80066DE4, "SLUS_005.78");
    result = r_u32(0x8008A290);
    if (result != asset)
    {
        w_u32(0x8008A290, asset);
        DrawSync(0);
        VSync(0);
        offset = resource_archive.Offset(asset);
        image = r_u32(0x8008A4C8) + offset;
        LoadImagePSX((PSX_RECT *)psx_addr(0x8008A288, sizeof(PSX_RECT)), (uint32 *)psx_addr(image, sizeof(uint32)));
        return (uint32)DrawSync(0);
    }
    return result;
}

uint32 p2_func_8006e6fc(uint32 selection)
{
    MaterialResourceHeader *resources;
    uint32 position;
    uint32 group;
    uint32 saved_frame;
    uint32 index;
    uint32 fade;
    uint32 result;
    FUNCTION_MARKER(0x8006E6FC, "SLUS_005.78");
    BeginDraw();
    ClearSceneOrderingTable();
    p2_func_80066af0(0);
    if (selection == 0)
    {
        position = 0x640064;
        group = menu_state.resources->textures_offset + 40;
    }
    else if (selection == 1)
    {
        position = 0x7D0073;
        group = menu_state.resources->textures_offset + 60;
    }
    else if (selection == 2)
    {
        position = 0x960063;
        group = menu_state.resources->textures_offset + 80;
    }
    else
    {
        position = 0xB00070;
        group = menu_state.resources->textures_offset + 20 * (selection + 2);
    }
    DrawTexturedQuad(position, 0, group, 0);
    p2_func_80027228();
    resources = menu_state.resources;
    if (resources != 0)
    {
        saved_frame = frame_clock.render_tick;
        frame_clock.render_tick = frame_clock.menu_tick;
        if ((sint32)resources->texture_count > 0)
        {
            index = 0;
            do
            {
                UpdateMaterialAnimationList(menu_state.material_groups[index].animation_entries);
                ++index;
            } while ((sint32)index < (sint32)menu_state.resources->texture_count);
        }
        frame_clock.render_tick = saved_frame;
    }
    fade = menu_state.fade_frames;
    if (fade != 0)
    {
        --fade;
        menu_state.fade_frames = fade;
        if (fade != 0)
            p2_func_800651bc(0, 0xF00140, (sint32)(fade << 12) / 15, 0xFFFFFF);
    }
    EndDraw();
    result = menu_state.pacer.Wait(menu_state.last_frame);
    menu_state.last_frame = result;
    return result;
}





uint32 p2_func_80066af0(uint32 mode)
{
    uint32 active;
    uint32 origin;
    sint32 y;
    PSX_RECT rectangle;
    FUNCTION_MARKER(0x80066AF0, "SLUS_005.78");
    active = render_commands.active_buffer;
    origin = r_u32(0x8008A288);
    r_u32(0x8008A28C);
    rectangle.x = (sint16)origin;
    rectangle.y = (sint16)(origin >> 16);
    y = r_s16(0x80087202 + 132 * (1 - active));
    rectangle.w = 320;
    rectangle.h = 240;
    MoveImage(&rectangle, 0, y);
    return (uint32)DrawSync(0);
}



uint32 p2_func_80027228(void)
{
    uint32 index;
    uint32 saved_frame;
    uint32 result;
    FUNCTION_MARKER(0x80027228, "SLUS_005.78");
    for (index = 0; index < 28; ++index)
    {
        if (index == 1 || index == 5 || ((game_flow.transition_flags & 0x10) == 0 && index >= 13))
        {
            saved_frame = frame_clock.render_tick;
            if (index == 1 || index == 5)
                frame_clock.render_tick = frame_clock.menu_tick;
            UpdateMaterialAnimationList(resource_groups[index].animation_entries);
            frame_clock.render_tick = saved_frame;
        }
    }
    result = frame_clock.menu_tick + 1;
    frame_clock.menu_tick = result;
    return result;
}



uint32 p2_func_80066b78(uint16 clut, uint32 page, uint32 palette)
{
    PSX_RECT rectangle;
    FUNCTION_MARKER(0x80066B78, "SLUS_005.78");
    rectangle.x = (sint16)((clut & 0x3F) << 4);
    rectangle.y = (sint16)(clut >> 6);
    rectangle.w = ((page >> 7) & 3) == 1 ? 256 : 16;
    rectangle.h = 1;
    return (uint32)LoadImagePSX(&rectangle, (uint32 *)psx_addr(palette, 2 * (uint32)rectangle.w));
}

uint32 p2_func_800651bc(uint32 position, uint32 extent, sint32 fraction, uint32 color)
{
    uint32 red;
    uint32 green;
    uint32 blue;
    uint32 packet;
    uint32 next;
    FUNCTION_MARKER(0x800651BC, "SLUS_005.78");
    if (fraction >= 4096)
    {
        red = color & 255;
        green = (color >> 8) & 255;
        blue = (color >> 16) & 255;
    }
    else if (fraction > 0)
    {
        red = ((uint32)fraction * (color & 255) + 2048) >> 12;
        green = ((uint32)fraction * ((color >> 8) & 255) + 2048) >> 12;
        blue = ((uint32)fraction * ((color >> 16) & 255) + 2048) >> 12;
    }
    else
        red = green = blue = 0;
    color = red | (green << 8) | (blue << 16);
    if (render_commands.overlay_head == 0)
        render_commands.overlay_head = (render_commands.packet_offset);
    packet = render_commands.packet_offset;
    next = packet + 12;
    w_u32(packet, (next & 0xFFFFFF) | 0x02000000);
    w_u32(packet + 4, 0xE1000040);
    w_u32(packet + 8, 0);
    w_u32(packet + 16, color | 0x62000000);
    w_u32(packet + 12, ((packet + 28) & 0xFFFFFF) | 0x03000000);
    w_u32(packet + 20, position);
    w_u32(packet + 24, extent);
    render_commands.overlay_tail = (next);
    render_commands.packet_offset = (packet + 28);
    return 0x03000000;
}



uint32 p2_func_800663ac(uint32 destination, uint32 source, uint32 count)
{
    uint32 result = source < destination;
    FUNCTION_MARKER(0x800663AC, "SLUS_005.78");
    if (count != 0)
    {
        if (source < destination)
        {
            source += count - 1;
            destination += count - 1;
            do
            {
                result = r_u8(source);
                --source;
                --count;
                w_u8(destination, result);
                --destination;
            } while (count != 0);
        }
        else
        {
            do
            {
                result = r_u8(source);
                ++source;
                --count;
                w_u8(destination, result);
                ++destination;
            } while (count != 0);
        }
    }
    return result;
}

uint32 p2_func_8006a940(uint16 voice)
{
    uint32 mask;
    FUNCTION_MARKER(0x8006A940, "SLUS_005.78");
    mask = (uint32)1 << (((uint32)voice + 16) & 31);
    return p2_sdk_80070df8(mask) == 1;
}

void p2_func_8005e470(uint32 sound, uint16 voice, uint32 arg3, uint32 arg4, uint32 override_pitch, sint32 volume, uint16 program)
{
    uint32 pitch = override_pitch;
    uint16 pan;
    FUNCTION_MARKER(0x8005E470, "SLUS_005.78");
    if ((volume >> 4) != 0)
    {
        if (pitch == 0)
            pitch = r_u16(sound + 4);
        pan = program != 0 ? (uint16)(program - 1) : 4;
        p2_func_8006a740(voice, (uint16)(volume >> 4));
        p2_func_8006a638(voice, pan);
        p2_func_8006a814(voice, pitch);
        r_u16(sound + 6);
        p2_func_8006a878(voice, sound);
    }
}

uint32 p2_func_8006a740(uint16 voice, uint16 volume)
{
    uint32 row = 0x800A9688 + 12 * voice;
    uint32 gain;
    sint64 product;
    FUNCTION_MARKER(0x8006A740, "SLUS_005.78");
    gain = (16383 * (uint32)volume) / 256;
    w_u32(row + 8, gain);
    w_u32(0x800A96E8, (uint32)1 << (((uint32)voice + 16) & 31));
    w_u32(0x800A96EC, 3);
    product = (sint64)r_s32(row + 8) * r_s32(row);
    w_u16(0x800A96F0, (uint32)(product >> 12));
    product = (sint64)r_s32(row + 8) * r_s32(row + 4);
    w_u16(0x800A96F2, (uint32)(product >> 12));
    return p2_sdk_8007101c(0x800A96E8);
}

uint32 p2_func_8006a638(uint16 voice, uint16 pan)
{
    uint32 row;
    sint32 delta;
    sint64 product;
    FUNCTION_MARKER(0x8006A638, "SLUS_005.78");
    if (pan >= 9)
        pan = 8;
    delta = ((sint32)pan - 4) * 512;
    row = 0x800A9688 + 12 * voice;
    w_u32(row, (uint32)(2048 - delta));
    w_u32(row + 4, 4096 - r_u32(row));
    w_u32(0x800A96E8, (uint32)1 << (((uint32)voice + 16) & 31));
    w_u32(0x800A96EC, 3);
    product = (sint64)r_s32(row + 8) * r_s32(row);
    w_u16(0x800A96F0, (uint32)(product >> 12));
    product = (sint64)r_s32(row + 8) * r_s32(row + 4);
    w_u16(0x800A96F2, (uint32)(product >> 12));
    return p2_sdk_8007101c(0x800A96E8);
}

uint32 p2_func_8006a814(uint16 voice, uint32 pitch)
{
    FUNCTION_MARKER(0x8006A814, "SLUS_005.78");
    w_u32(0x800A96E8, (uint32)1 << (((uint32)voice + 16) & 31));
    w_u32(0x800A96EC, 16);
    w_u16(0x800A96FC, (95 * pitch) >> 10);
    return p2_sdk_8007101c(0x800A96E8);
}

void p2_func_8006a878(uint16 voice, uint32 sound)
{
    uint32 mask;
    uint32 address;
    FUNCTION_MARKER(0x8006A878, "SLUS_005.78");
    if (sound != 0)
    {
        mask = (uint32)1 << (((uint32)voice + 16) & 31);
        p2_sdk_80070bc0(0, mask);
        w_u32(0x800A96EC, 65408);
        w_u32(0x800A96E8, mask);
        address = r_u32(sound);
        w_u32(0x800A9714, 3);
        w_u32(0x800A970C, 1);
        w_u32(0x800A9710, 1);
        w_u16(0x800A9718, 0);
        w_u16(0x800A971A, 0);
        w_u16(0x800A971C, 0);
        w_u16(0x800A971E, 0);
        w_u16(0x800A9720, 15);
        w_u32(0x800A9704, address);
        p2_sdk_80070dc4(0x800A96E8);
    }
}

uint32 p2_func_8006ce38(void)
{
    sint32 sound_volume;
    sint32 music_volume;
    sint32 selection = 0;
    sint32 preview_voice = -1;
    sint32 voice;
    sint32 direction;
    uint32 preview_sound = 0;
    uint32 sound;
    uint32 saved_frame;
    uint32 old_buttons;
    uint32 buttons;
    uint32 pressed;
    uint32 clock;
    FUNCTION_MARKER(0x8006CE38, "SLUS_005.78");
    p2_func_80067160(0);
    sound = r_u32(0x8008A408);
    sound_volume = (sint16)(sound_settings.effects_volume);
    if (sound != 0 && r_s32(sound) > 0)
        preview_sound = sound + 4;
    music_volume = (sint16)(sound_settings.music_volume);
    clock = frame_clock.milliseconds;
    menu_state.pacer.fraction = 30;
    menu_state.fade_frames = 15;
    menu_state.last_frame = clock;
    for (;;)
    {
        p2_func_80066de4(257);
        p2_func_8006c7b4(selection, music_volume, sound_volume);
        old_buttons = ~menu_state.previous_buttons;
        buttons = p2_func_80066d08();
        menu_state.previous_buttons = buttons;
        pressed = buttons & old_buttons;
        if (pressed & 0x10)
        {
            sound = 0;
            saved_frame = frame_clock.render_tick;
            if (r_u32(0x8008A408) != 0 && r_s32(r_u32(0x8008A408)) > 25)
                sound = r_u32(0x8008A408) + 204;
            frame_clock.render_tick = frame_clock.menu_tick;
            voice = sound_effects.SelectVoice(0xFFFFFFFF, sound, 4);
            if (voice >= 0)
                p2_func_8005e470(sound, (uint16)voice, 0, 0, 0, (sint16)(sound_settings.effects_volume), 0);
            frame_clock.render_tick = saved_frame;
            break;
        }
        if (pressed & 0xA000)
        {
            sound = 0;
            saved_frame = frame_clock.render_tick;
            if (r_u32(0x8008A408) != 0 && r_s32(r_u32(0x8008A408)) > 24)
                sound = r_u32(0x8008A408) + 196;
            frame_clock.render_tick = frame_clock.menu_tick;
            voice = sound_effects.SelectVoice(0xFFFFFFFF, sound, 4);
            if (voice >= 0)
                p2_func_8005e470(sound, (uint16)voice, 0, 0, 0, (sint16)(sound_settings.effects_volume), 0);
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
                if (r_u32(0x8008A408) != 0 && r_s32(r_u32(0x8008A408)) > 25)
                    sound = r_u32(0x8008A408) + 204;
                frame_clock.render_tick = frame_clock.menu_tick;
                voice = sound_effects.SelectVoice(0xFFFFFFFF, sound, 4);
                if (voice >= 0)
                    p2_func_8005e470(sound, (uint16)voice, 0, 0, 0, (sint16)(sound_settings.effects_volume), 0);
                frame_clock.render_tick = saved_frame;
                {
                    uint32 cursor;
                    if (pressed & 0x1000)
                    {
                        cursor = (control_bindings.selected_index - 1) & 255;
                        control_bindings.selected_index = (uint8)cursor;
                        if (cursor & 0x80) control_bindings.selected_index = 23;
                    }
                    else
                    {
                        cursor = (control_bindings.selected_index + 1) & 255;
                        control_bindings.selected_index = (uint8)cursor;
                        if (cursor >= 24) control_bindings.selected_index = 0;
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
                    p2_func_8006a740((uint16)preview_voice, (uint16)(sound_volume >> 4));
                else
                {
                    preview_voice = sound_effects.SelectVoice(0xFFFFFFFF, preview_sound, 4);
                    if (preview_voice != -1)
                        p2_func_8005e470(preview_sound, (uint16)preview_voice, 0, 0, 20010, sound_volume, 0);
                }
                frame_clock.render_tick = saved_frame;
            }
        }
        frame_clock.menu_tick = frame_clock.menu_tick + 1;
    }
    p2_func_80067160(0);
    menu_state.fade_frames = 15;
    return 0;
}

sint32 p2_func_80067160(uint32 color)
{
    uint32 step;
    uint32 previous;
    uint32 deadline;
    uint32 wait;
    sint32 fraction = 30;
    sint32 strength;
    FUNCTION_MARKER(0x80067160, "SLUS_005.78");
    p2_func_80066978();
    frame_clock.frame_start = frame_clock.milliseconds;
    for (step = 2; step < 31; step += 2)
    {
        if (color == 0 || color == 0xFFFFFF)
        {
            previous = frame_clock.frame_start;
            BeginDraw();
            ClearSceneOrderingTable();
            p2_func_80066af0(1);
            strength = (sint32)(step << 12) / 30;
            if (color == 0)
                p2_func_800651bc(0, 0xF00140, strength, 0xFFFFFF);
            else
                p2_func_800652e4(0, 0xF00140, strength, 0xFFFFFF);
            EndDraw();
        }
        else
        {
            BeginDraw();
            ClearSceneOrderingTable();
            p2_func_80066af0(1);
            strength = (sint32)(step << 12) / 30;
            p2_func_800651bc(0, 0xF00140, strength, ~color | 0xFFFFFF);
            p2_func_800652e4(0, 0xF00140, strength, color);
            EndDraw();
            previous = frame_clock.frame_start;
        }
        fraction -= 10;
        wait = 33;
        if (fraction <= 0)
        {
            wait = 34;
            fraction += 30;
        }
        deadline = previous + wait;
        while (frame_clock.milliseconds < deadline)
            psx_native_poll_events();
        frame_clock.milliseconds;
        frame_clock.frame_start = frame_clock.milliseconds;
    }
    return 0;
}

uint32 p2_func_80066978(void)
{
    uint32 active;
    sint32 x;
    sint32 y;
    PSX_RECT rectangle;
    FUNCTION_MARKER(0x80066978, "SLUS_005.78");
    DrawSync(0);
    VSync(0);
    active = render_commands.active_buffer;
    x = r_s16(0x8008A288);
    y = r_s16(0x8008A28A);
    rectangle.x = 0;
    rectangle.y = (sint16)r_u16(0x80087202 + 132 * active);
    rectangle.w = 320;
    rectangle.h = 240;
    MoveImage(&rectangle, x, y);
    return (uint32)DrawSync(0);
}

uint32 p2_func_8006c7b4(sint32 selection, sint32 music_volume, sint32 sound_volume)
{
    MaterialResourceHeader *resources;
    uint32 position;
    uint32 group_index;
    uint32 groups;
    uint32 icon;
    uint32 button;
    uint32 table;
    uint32 offset;
    uint32 index;
    uint32 saved_frame;
    uint32 fade;
    uint32 result;
    FUNCTION_MARKER(0x8006C7B4, "SLUS_005.78");
    DrawSync(0);
    VSync(0);
    BeginDraw();
    ClearSceneOrderingTable();
    p2_func_80066af0(0);
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
    groups = menu_state.resources->textures_offset;
    DrawTexturedQuad(position, 0, groups + 20 * group_index, 0);
    groups = menu_state.resources->textures_offset;
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
                offset = 180;
                break;
            case 16:
                offset = 220;
                break;
            case 64:
                offset = 160;
                break;
            default:
                offset = 200;
                break;
        }
        table = interface_resources->textures_offset;
        if (table + offset != 0)
        {
            button = (*control_bindings.active_scheme)[index];
            switch (button)
            {
                case 128:
                    offset = 180;
                    break;
                case 16:
                    offset = 220;
                    break;
                case 64:
                    offset = 160;
                    break;
                default:
                    offset = 200;
                    break;
            }
            table = interface_resources->textures_offset;
            position = icon == 0 ? 0x90007D : icon == 1 ? 0xAA007D : 0xC5007D;
            DrawTexturedQuad(position, 0, table + offset, 0);
        }
    }
    p2_func_80027228();
    resources = menu_state.resources;
    if (resources != 0)
    {
        saved_frame = frame_clock.render_tick;
        groups = resources->texture_count;
        frame_clock.render_tick = frame_clock.menu_tick;
        if ((sint32)groups > 0)
        {
            index = 0;
            do
            {
                UpdateMaterialAnimationList(menu_state.material_groups[index].animation_entries);
                ++index;
            } while ((sint32)index < (sint32)menu_state.resources->texture_count);
        }
        frame_clock.render_tick = saved_frame;
    }
    fade = menu_state.fade_frames;
    if (fade != 0)
    {
        --fade;
        menu_state.fade_frames = fade;
        if (fade != 0)
            p2_func_800651bc(0, 0xF00140, (sint32)(fade << 12) / 15, 0xFFFFFF);
    }
    EndDraw();
    result = menu_state.pacer.Wait(menu_state.last_frame);
    menu_state.last_frame = result;
    return result;
}









uint32 p2_func_800652e4(uint32 position, uint32 extent, sint32 fraction, uint32 color)
{
    uint32 red;
    uint32 green;
    uint32 blue;
    uint32 packet;
    uint32 next;
    FUNCTION_MARKER(0x800652E4, "SLUS_005.78");
    if (fraction >= 4096)
    {
        red = color & 255;
        green = (color >> 8) & 255;
        blue = (color >> 16) & 255;
    }
    else if (fraction > 0)
    {
        red = ((uint32)fraction * (color & 255) + 2048) >> 12;
        green = ((uint32)fraction * ((color >> 8) & 255) + 2048) >> 12;
        blue = ((uint32)fraction * ((color >> 16) & 255) + 2048) >> 12;
    }
    else
        red = green = blue = 0;
    color = red | (green << 8) | (blue << 16);
    if (render_commands.overlay_head == 0)
        render_commands.overlay_head = (render_commands.packet_offset);
    packet = render_commands.packet_offset;
    next = packet + 12;
    w_u32(packet, (next & 0xFFFFFF) | 0x02000000);
    w_u32(packet + 4, 0xE1000020);
    w_u32(packet + 8, 0);
    w_u32(packet + 16, color | 0x62000000);
    w_u32(packet + 12, ((packet + 28) & 0xFFFFFF) | 0x03000000);
    w_u32(packet + 20, position);
    w_u32(packet + 24, extent);
    render_commands.overlay_tail = (next);
    render_commands.packet_offset = (packet + 28);
    return 0x03000000;
}

void p2_func_8002a448(uint32 mode, uint32 object)
{
    FUNCTION_MARKER(0x8002A448, "SLUS_005.78");
    if (object != 0)
        p2_func_80064d54(object - 4);
}

void p2_func_80064d54(uint32 allocation)
{
    uint32 total;
    uint32 bytes;
    FUNCTION_MARKER(0x80064D54, "SLUS_005.78");
    total = r_u32(0x8008A258);
    bytes = r_u32(allocation - 4);
    w_u32(0x8008A258, total - bytes);
    p2_sdk_80082a08(allocation);
}

void p2_func_80066430(void)
{
    uint16 handle;
    FUNCTION_MARKER(0x80066430, "SLUS_005.78");
    handle = r_u16(0x8008A530);
    if (handle != 0)
        p2_func_8006adc8((uint8)handle);
}

uint32 p2_func_8006adc8(uint8 handle)
{
    FUNCTION_MARKER(0x8006ADC8, "SLUS_005.78");
    p2_sdk_80075980((sint32)handle - 1);
    p2_func_800765b0();
    p2_sdk_80070a24(1);
    p2_sdk_800763ec(0);
    return p2_sdk_80076354(0, 0);
}

uint32 p2_func_8006ae90(uint16 handle)
{
    FUNCTION_MARKER(0x8006AE90, "SLUS_005.78");
    p2_func_80071a68((sint16)((sint32)(uint8)handle - 1));
    return p2_sdk_80079b14((sint32)(handle >> 8) - 1);
}

uint32 p2_func_80071a68(sint16 sequence)
{
    FUNCTION_MARKER(0x80071A68, "SLUS_005.78");
    return p2_func_800718ec(sequence);
}

uint32 p2_func_800718ec(sint16 sequence)
{
    uint32 result;
    uint32 table;
    uint32 offset = 0;
    uint32 index = 0;
    uint32 pointer;
    sint32 count;
    FUNCTION_MARKER(0x800718EC, "SLUS_005.78");
    p2_sdk_80078d1c(sequence, 0, 0, 1);
    p2_sdk_80079350(sequence);
    result = r_u32(0x800B7F3C);
    count = r_s16(0x800BA40A);
    result &= ~((uint32)1 << ((uint32)(sint32)sequence & 31));
    w_u32(0x800B7F3C, result);
    if (count > 0)
    {
        table = 0x800B9B00 + 4 * (uint32)(sint32)sequence;
        do
        {
            w_u32(offset + r_u32(table) + 144, 0);
            w_u8(offset + r_u32(table) + 60, 255);
            w_u8(offset + r_u32(table), 0);
            w_u16(offset + r_u32(table) + 62, 0);
            w_u16(offset + r_u32(table) + 64, 0);
            w_u32(offset + r_u32(table) + 148, 0);
            w_u32(offset + r_u32(table) + 152, 0);
            w_u16(offset + r_u32(table) + 66, 0);
            w_u32(offset + r_u32(table) + 164, 0);
            w_u32(offset + r_u32(table) + 160, 0);
            w_u32(offset + r_u32(table) + 156, 0);
            w_u16(offset + r_u32(table) + 68, 0);
            ++index;
            w_u16(offset + r_u32(table) + 116, 127);
            pointer = offset + r_u32(table);
            count = r_s16(0x800BA40A);
            offset += 172;
            result = (sint32)index < count;
            w_u16(pointer + 118, 127);
        } while (result != 0);
    }
    return result;
}










