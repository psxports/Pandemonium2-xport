#include <stdio.h>
#include <stdlib.h>

#include "screen_render_state.h"
#include "game.h"

ScreenRenderState scr_render = {0};

uint8 demo_background_workspace[8184] = {0};

void ScreenRenderState::StartSceneFadeIn(void)
{
    FUNCTION_MARKER(0x80025BE8, "SLUS_005.78");
    scr_render.transition_ticks = 15;
}

uint32 ScreenRenderState::UpdateScrEffects(void)
{
    FUNCTION_MARKER(0x8005CF3C, "SLUS_005.78");
    uint32 result, val;
    sint32 fade, numerator, remaining, duration, half, intensity;
    fade = scr_render.transition_ticks;
    if (fade != 0)
    {
        if (fade > 0)
        {
            val = fade - 1u;
            numerator = (sint32)(val << 16) >> 4;
            scr_render.transition_ticks = val;
            intensity = numerator / 15;
        }
        else
        {
            val = fade + 1u;
            fade = (sint16)val;
            numerator = (fade + 15u) << 12;
            scr_render.transition_ticks = val;
            intensity = numerator / 15;
            if (fade == 0)
                scr_render.transition_ticks = -1;
        }
        return ScreenRenderState::DrawScrDarken(intensity);
    }
    result = 0;
    remaining = scr_render.brighten_ticks;
    if (remaining > 0)
    {
        duration = scr_render.brighten_duration;
        result = duration < 2;
        if (duration >= 2)
        {
            --remaining;
            scr_render.brighten_ticks = remaining;
            half = duration / 2;
            if (remaining > half)
            {
                numerator = ((uint32)remaining - half) << 12;
                intensity = 4096u - (numerator / half);
            }
            else
            {
                numerator = (uint32)remaining << 12;
                intensity = numerator / half;
            }
            return ScreenRenderState::DrawScrBrighten(intensity);
        }
    }
    remaining = scr_render.darken_ticks;
    if (remaining <= 0)
        return result;
    duration = scr_render.darken_duration;
    result = duration < 2;
    if (duration < 2)
        return result;
    --remaining;
    scr_render.darken_ticks = remaining;
    half = duration / 2;
    if (remaining > half)
    {
        numerator = ((uint32)remaining - half) << 12;
        intensity = 4096u - (numerator / half);
    }
    else
    {
        numerator = (uint32)remaining << 12;
        intensity = numerator / half;
    }
    return ScreenRenderState::DrawScrDarken(intensity);
}

void ScreenRenderState::DrawSceneInterface(void)
{
    FUNCTION_MARKER(0x8005D170, "SLUS_005.78");

    BackgroundResource::DrawSceneBackground();
    HudState::DrawHud(CharacterController::FindPlayerObject(0));
    ScreenRenderState::UpdateScrEffects();
    ScreenRenderState::EmptyScrOperation();
}

void ScreenRenderState::EmptyScrOperation(void)
{
    FUNCTION_MARKER(0x800606F4, "SLUS_005.78");
    /* Original JR RA/NOP has no side effects */
}

void ScreenRenderState::DrawSceneFade(void)
{
    FUNCTION_MARKER(0x80064A58, "SLUS_005.78");
    uint32 brightness, table, packet, color, val;
    DR_MODE *mode;
    TILE *rect;
    uint32 *ot_entry;
    brightness = scr_render.fade_level;
    table = render_commands.small_ordering_table_offset;
    ot_entry = (uint32 *)psx_addr(table + 120, sizeof(uint32));
    packet = render_commands.packet_offset;
    brightness = (255 * brightness + 2048) >> 12;
    color = brightness | (brightness << 8) | (brightness << 16);
    mode = (DR_MODE *)psx_addr(packet, sizeof(DR_MODE));
    rect = (TILE *)psx_addr(packet + 12, sizeof(TILE));
    setTile(rect);
    mode->tag = ((packet + 12) & 0xFFFFFF) | 0x02000000;
    mode->code[0] = 0xE1000040;
    // Retain the original NOP after the draw-mode command
    mode->code[1] = 0;
    val = *ot_entry;
    rect->tag = (val & 0xFFFFFF) | 0x03000000;
    rect->rgbc.rgbc = color | 0x62000000;
    rect->p[0].xy = 0;
    rect->wh = 0x00F00140;
    val = *ot_entry;
    *ot_entry = (val & 0xFF000000) | (packet & 0xFFFFFF);
    render_commands.packet_offset = packet + 28;
}

void ScreenRenderState::DrawBackgroundBorders(sint32 top, uint32 height)
{
    FUNCTION_MARKER(0x80064DBC, "SLUS_005.78");
    const SceneSection *section;
    uint32 extent;
    sint32 bottom, remaining;
    if (top >= 0)
    {
        extent = 239;
        if (top < 239)
            extent = top;
        section = scene_stream.active_section;
        ScreenRenderState::DrawBackgroundRect(0, ((extent + 1u) << 16) | 320u, section->background_top_color);
    }
    bottom = top + height;
    if (bottom >= 240)
        return;
    extent = 239;
    remaining = 239u - bottom;
    if (remaining < 239)
        extent = remaining;
    else if (bottom < 0)
        bottom = 0;
    section = scene_stream.active_section;
    ScreenRenderState::DrawBackgroundRect((uint32)bottom << 16, ((extent + 1u) << 16) | 320u, section->background_bottom_color);
}

void ScreenRenderState::DrawOverlayRect(uint32 pos, uint32 extent, sint32 fraction, uint32 color)
{
    uint32 r;
    uint32 g;
    uint32 b;
    uint32 packet;
    uint32 next;
    DR_MODE *mode;
    TILE *rect;
    FUNCTION_MARKER(0x800651BC, "SLUS_005.78");
    if (fraction >= 4096)
    {
        r = color & 255;
        g = (color >> 8) & 255;
        b = (color >> 16) & 255;
    }
    else if (fraction > 0)
    {
        r = (fraction * (color & 255) + 2048) >> 12;
        g = (fraction * ((color >> 8) & 255) + 2048) >> 12;
        b = (fraction * ((color >> 16) & 255) + 2048) >> 12;
    }
    else
        r = g = b = 0;
    color = r | (g << 8) | (b << 16);
    if (render_commands.overlay_head == 0)
        render_commands.overlay_head = render_commands.packet_offset;
    packet = render_commands.packet_offset;
    next = packet + 12;
    mode = (DR_MODE *)psx_addr(packet, sizeof(DR_MODE));
    rect = (TILE *)psx_addr(packet + 12, sizeof(TILE));
    setTile(rect);
    mode->tag = (next & 0xFFFFFF) | 0x02000000;
    mode->code[0] = 0xE1000040;
    // Retain the original NOP after the draw-mode command
    mode->code[1] = 0;
    rect->rgbc.rgbc = color | 0x62000000;
    rect->tag = ((packet + 28) & 0xFFFFFF) | 0x03000000;
    rect->p[0].xy = pos;
    rect->wh = extent;
    render_commands.overlay_tail = next;
    render_commands.packet_offset = packet + 28;
}

void ScreenRenderState::DrawBrightenRect(uint32 pos, uint32 extent, sint32 fraction, uint32 color)
{
    uint32 r;
    uint32 g;
    uint32 b;
    uint32 packet;
    uint32 next;
    FUNCTION_MARKER(0x800652E4, "SLUS_005.78");
    if (fraction >= 4096)
    {
        r = color & 255;
        g = (color >> 8) & 255;
        b = (color >> 16) & 255;
    }
    else if (fraction > 0)
    {
        r = (fraction * (color & 255) + 2048) >> 12;
        g = (fraction * ((color >> 8) & 255) + 2048) >> 12;
        b = (fraction * ((color >> 16) & 255) + 2048) >> 12;
    }
    else
        r = g = b = 0;
    color = r | (g << 8) | (b << 16);
    if (render_commands.overlay_head == 0)
        render_commands.overlay_head = render_commands.packet_offset;
    packet = render_commands.packet_offset;
    next = packet + 12;
    DR_MODE *mode = (DR_MODE *)psx_addr(packet, sizeof(DR_MODE));
    TILE *rect = (TILE *)psx_addr(next, sizeof(TILE));
    mode->tag = (next & 0xFFFFFF) | 0x02000000;
    mode->code[0] = 0xE1000020;
    mode->code[1] = 0;
    rect->rgbc.rgbc = color | 0x62000000;
    rect->tag = ((packet + 28) & 0xFFFFFF) | 0x03000000;
    rect->p[0].xy = pos;
    rect->wh = extent;
    render_commands.overlay_tail = next;
    render_commands.packet_offset = packet + 28;
}

uint32 ScreenRenderState::DrawScrBrighten(sint32 intensity)
{
    FUNCTION_MARKER(0x80065F78, "SLUS_005.78");
    uint32 color, packet, next_packet;
    DR_MODE *mode;
    TILE *rect;
    if (intensity >= 4096)
        color = 255;
    else if (intensity > 0)
        color = (uint32)(255 * intensity + 2048) >> 12;
    else
        color = 0;
    color |= (color << 8) | (color << 16);
    if (render_commands.overlay_head == 0)
        render_commands.overlay_head = render_commands.packet_offset;
    packet = render_commands.packet_offset;
    next_packet = packet + 28;
    mode = (DR_MODE *)psx_addr(packet, sizeof(DR_MODE));
    rect = (TILE *)psx_addr(packet + 12, sizeof(TILE));
    setTile(rect);
    mode->tag = ((packet + 12) & 0xFFFFFF) | 0x02000000;
    mode->code[0] = 0xE1000020;
    mode->code[1] = 0;
    rect->rgbc.rgbc = color | 0x62000000;
    rect->tag = (next_packet & 0xFFFFFF) | 0x03000000;
    rect->p[0].xy = 0;
    rect->wh = 0x00F00140;
    render_commands.overlay_tail = packet + 12;
    render_commands.packet_offset = next_packet;
    return 0x03000000;
}

uint32 ScreenRenderState::DrawScrDarken(sint32 intensity)
{
    FUNCTION_MARKER(0x80066070, "SLUS_005.78");
    uint32 color, packet, next_packet;
    DR_MODE *mode;
    TILE *rect;
    if (intensity >= 4096)
        color = 255;
    else if (intensity > 0)
        color = (uint32)(255 * intensity + 2048) >> 12;
    else
        color = 0;
    color |= (color << 8) | (color << 16);
    if (render_commands.overlay_head == 0)
        render_commands.overlay_head = render_commands.packet_offset;
    packet = render_commands.packet_offset;
    next_packet = packet + 28;
    mode = (DR_MODE *)psx_addr(packet, sizeof(DR_MODE));
    rect = (TILE *)psx_addr(packet + 12, sizeof(TILE));
    setTile(rect);
    mode->tag = ((packet + 12) & 0xFFFFFF) | 0x02000000;
    mode->code[0] = 0xE1000040;
    mode->code[1] = 0;
    rect->rgbc.rgbc = color | 0x62000000;
    rect->tag = (next_packet & 0xFFFFFF) | 0x03000000;
    rect->p[0].xy = 0;
    rect->wh = 0x00F00140;
    render_commands.overlay_tail = packet + 12;
    render_commands.packet_offset = next_packet;
    return 0x03000000;
}

void ScreenRenderState::UploadTexImage(uint8 *image, uint32 idx, uint32 flags)
{
    PSX_RECT rect;
    uint32 offset;
    uint32 result;
    sint32 status;
    FUNCTION_MARKER(0x8006624C, "SLUS_005.78");
    rect = OriginalDefaults::tex_img_rects[idx];
    LoadImagePSX(&rect, (uint32 *)image);
    if (idx == 0)
    {
        offset = resource_archive.Offset(252);
        rect = OriginalDefaults::tex_img_rects[8];
        LoadImagePSX(&rect, (uint32 *)(image + offset));
    }
    if ((flags << 24) != 0)
    {
        do
        {
            status = DrawSync(1);
        } while (status > 0);
    }

    if (game_flow.mode_flags & 0x800)
    {
        if (idx >= 2)
        {
            if (idx != 9)
            {
                rect = OriginalDefaults::tex_img_rects[8];
                MoveImage(&rect, OriginalDefaults::tex_img_rects[idx].x, OriginalDefaults::tex_img_rects[idx].y);
                result = flags << 24;
                if (result != 0)
                {
                    do
                    {
                        result = DrawSync(1);
                    } while ((sint32)result > 0);
                }
            }
        }
    }
}

void ScreenRenderState::SaveMenuImage(void)
{
    uint32 active;
    sint32 x;
    sint32 y;
    PSX_RECT rect;
    FUNCTION_MARKER(0x80066978, "SLUS_005.78");
    DrawSync(0);
    VSync(0);
    active = render_commands.active_buffer;
    x = OriginalDefaults::menu_img_rect.x;
    y = OriginalDefaults::menu_img_rect.y;
    rect.x = 0;
    rect.y = render_buffers[active].draw.clip.y;
    rect.w = 320;
    rect.h = 240;
    MoveImage(&rect, x, y);
    DrawSync(0);
}

void ScreenRenderState::DrawStartupImage(uint32 *image)
{
    PSX_RECT origin;
    sint32 x;
    sint32 y;
    PSX_RECT rect;
    PSX_RECT image_rect = OriginalDefaults::startup_img_rect;
    FUNCTION_MARKER(0x800669F4, "SLUS_005.78");
    origin = render_buffers[render_commands.active_buffer].disp.screen;
    if (scr_render.startup_image_loaded)
    {
        rect.x = scr_render.startup_image_x;
        rect.y = scr_render.startup_image_y;
        rect.w = OriginalDefaults::startup_img_rect.w;
        rect.h = OriginalDefaults::startup_img_rect.h;
        VSync(0);
        ClearImage(&rect, 0, 0, 0);
    }
    else
    {
        LoadImagePSX(&image_rect, image);
    }
    x = origin.x + 112;
    y = origin.y + 84;
    scr_render.startup_image_x = x;
    scr_render.startup_image_y = y;
    MoveImage(&image_rect, x, y);
    DrawSync(0);
}

void ScreenRenderState::RestoreMenuImage(uint32 mode)
{
    uint32 active;
    sint32 y;
    PSX_RECT rect;
    FUNCTION_MARKER(0x80066AF0, "SLUS_005.78");
    active = render_commands.active_buffer;
    rect = OriginalDefaults::menu_img_rect;
    y = render_buffers[1 - active].draw.clip.y;
    rect.w = 320;
    rect.h = 240;
    MoveImage(&rect, 0, y);
    DrawSync(0);
}

void ScreenRenderState::DrawBackgroundRect(uint32 pos, uint32 extent, uint32 color)
{
    FUNCTION_MARKER(0x80066BDC, "SLUS_005.78");
    uint32 ordering_table = render_commands.scene_ordering_table_offset;
    uint32 packet = render_commands.aux_packet_offset;
    uint32 *ot_entry = (uint32 *)psx_addr(ordering_table + 2400, sizeof(uint32));
    uint32 link = *ot_entry;

    TILE *rect;
    rect = (TILE *)psx_addr(packet, sizeof(TILE));
    setTile(rect);
    rect->rgbc.rgbc = (color & 0xFFFFFF) | 0x60000000;
    rect->p[0].xy = pos;
    rect->wh = extent;
    rect->tag = (link & 0xFFFFFF) | 0x03000000;
    *ot_entry = (*ot_entry & 0xFF000000) | (packet & 0xFFFFFF);
    render_commands.aux_packet_offset = packet + 16;
}

void ScreenRenderState::FadeMenuImage(uint32 color)
{
    uint32 step;
    uint32 prev;
    uint32 deadline;
    uint32 wait;
    sint32 fraction = 30;
    sint32 strength;
    FUNCTION_MARKER(0x80067160, "SLUS_005.78");
    ScreenRenderState::SaveMenuImage();
    frame_clock.frame_start = frame_clock.milliseconds;
    for (step = 2; step < 31; step += 2)
    {
        if (color == 0 || color == 0xFFFFFF)
        {
            prev = frame_clock.frame_start;
            RenderCommands::BeginDraw();
            RenderCommands::ClearSceneOrderingTable();
            ScreenRenderState::RestoreMenuImage(1);
            strength = (sint32)(step << 12) / 30;
            if (color == 0)
                ScreenRenderState::DrawOverlayRect(0, 0xF00140, strength, 0xFFFFFF);
            else
                ScreenRenderState::DrawBrightenRect(0, 0xF00140, strength, 0xFFFFFF);
            RenderCommands::EndDraw();
        }
        else
        {
            RenderCommands::BeginDraw();
            RenderCommands::ClearSceneOrderingTable();
            ScreenRenderState::RestoreMenuImage(1);
            strength = (sint32)(step << 12) / 30;
            ScreenRenderState::DrawOverlayRect(0, 0xF00140, strength, ~color | 0xFFFFFF);
            ScreenRenderState::DrawBrightenRect(0, 0xF00140, strength, color);
            RenderCommands::EndDraw();
            prev = frame_clock.frame_start;
        }
        fraction -= 10;
        wait = 33;
        if (fraction <= 0)
        {
            wait = 34;
            fraction += 30;
        }
        deadline = prev + wait;
        while (frame_clock.milliseconds < deadline)
            psx_native_poll_events();
        frame_clock.milliseconds;
        frame_clock.frame_start = frame_clock.milliseconds;
    }
}

uint32 ScreenRenderState::DrawGameOverFade(void)
{
    FUNCTION_MARKER(0x80067778, "SLUS_005.78");
    sint32 remaining = 30, fraction = 30;
    uint32 fade = 273, result, delay, deadline;
    ScreenRenderState::SaveMenuImage();
    frame_clock.frame_start = frame_clock.milliseconds;
    do
    {
        uint32 style = fade < 4096 ? (fade << 16) | 1 : 1;
        RenderCommands::BeginDraw();
        RenderCommands::ClearSceneOrderingTable();
        ScreenRenderState::DrawBackgroundRect(0, 0xF00140, 0);
        ScreenRenderState::RestoreMenuImage(1);
        UiText::DrawText(0x7800A0, UiText::game_over, style, 0);
        RenderCommands::EndDraw();
        delay = 33;
        fraction -= 10;
        if (fraction <= 0)
        {
            delay = 34;
            fraction += 30;
        }
        deadline = frame_clock.frame_start + delay;
        while (frame_clock.milliseconds < deadline)
            psx_native_poll_events();
        result = frame_clock.milliseconds;
        frame_clock.frame_start = result;
        --remaining;
        fade += 273;
    } while (remaining != 0);
    return result;
}
