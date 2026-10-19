#include <stdio.h>

#include "game.h"
#include "original_defaults.h"

struct FontGlyph
{
    uint32 uv_corners;
    uint32 page_palette;
};

HudState hud = {0};

const char UiText::SwitchPlayer[] = "TO SWITCH PLAYER";
const char UiText::EnterLevel[] = "TO ENTER LEVEL";
const char UiText::GameOver[] = "GAME OVER";
const char UiText::InvalidPassword[] = "INVALID PASSWORD";
const char UiText::Password[] = "PASSWORD";
const char UiText::Continue[] = "TO CONTINUE";
const char UiText::ContinueQuit[] = "CONTINUE  QUIT";
const char UiText::QuitGame[] = "QUIT GAME?";
const char UiText::SelectQuit[] = "SELECT TO QUIT";
const char UiText::BossDefeated[] = "BOSS DEFEATED";
const char UiText::LevelComplete[] = "LEVEL COMPLETE";
const char UiText::BonusOver[] = "BONUS LEVEL OVER";
const char UiText::BonusComplete[] = "BONUS LEVEL COMPLETE";
const char UiText::TreasureFormat[] = "TREASURE: %d%%";
const char UiText::BoarderRun[] = "FOR BOARDER RUN";
const char UiText::NumberFormat[] = "%d";
const char UiText::QuitChoices[] = "YES  NO";
const char UiText::Quit[] = "TO QUIT";
const char UiText::Paused[] = "PAUSED";
const char UiText::Demo[3][11] = {"DEMO", "D E M O", "D  E  M  O"};

uint32 DrawText(uint32 position, const char *text, uint32 style, uint32 flags)
{
    TextureDescriptor *texture;
    uint32 *metadata;
    FUNCTION_MARKER(0x80064FF4, "SLUS_005.78");
    TextureDescriptor *group;
    uint32 index, tile, right, left, top, bottom, width, height;
    uint32 alignment, length, character, extent, step, result;
    const uint8 *atlas, *glyph;
    const char *cursor;
    FontGlyph glyph_data;
    const uint8 *glyph_bytes;
    sint32 scale;
    index = OriginalDefaults::FontResourceGroup << 3;
    metadata = resource_groups[index / 8].animation_entries;
    group = resource_groups[index / 8].texture;
    texture = group;
    tile = (uint8)metadata[-1];
    scale = (sint32)style >> 16;
    alignment = style & 0xFFFF;
    right = texture->payload.bytes[2];
    tile = metadata[tile - 1];
    left = texture->payload.bytes[0];
    top = texture->payload.bytes[1];
    tile = (tile >> 1) & 0x1FF;
    bottom = texture->payload.bytes[3];
    width = right - left + 1;
    atlas = material_registry.records[tile].data;
    height = bottom - top + 1;
    if (alignment != 0)
    {
        character = (uint8)*text;
        length = 0;
        cursor = text + 1;
        if (character != 0)
        {
            do
            {
                character = (uint8)*cursor;
                ++length;
                ++cursor;
            } while (character != 0);
        }
        extent = width * length + ((length - 1) << 1);
        if (scale != 0)
            extent = (uint32)((sint32)((uint32)scale * extent + 2048) >> 12);
        if (alignment == 1)
            position -= extent >> 1;
        else
            position -= extent;
    }
    step = width + 2;
    if (scale != 0)
        step = (uint32)((sint32)((uint32)scale * step + 2048) >> 12);
    character = (uint8)*text;
    result = height >> 1;
    position -= result;
    cursor = text + 1;
    while (character != 0)
    {
        result = character - 32 < 59;
        if (result != 0)
        {
            result = character << 3;
            if (character != 32)
            {
                glyph = atlas + result - 256;
                glyph_bytes = glyph;
                glyph_data.uv_corners = xport_load_le32(glyph_bytes);
                glyph_data.page_palette = xport_load_le32(glyph_bytes + 4);
                texture->payload.words[0] = glyph_data.uv_corners;
                texture->payload.words[1] = glyph_data.page_palette;
                result = DrawTexturedQuad(position, 0, texture, flags);
            }
            position += step;
        }
        character = (uint8)*cursor;
        ++cursor;
    }
    return result;
}

void ResetHud(void)
{
    uint32 index;
    uint32 *entries;
    FUNCTION_MARKER(0x8006540C, "SLUS_005.78");
    for (index = 0; index < 3; ++index)
    {
        hud.counters[index].state = 0;
        hud.counters[index].shown_value = 0xFFFFFFFF;
    }
    entries = resource_groups[1].animation_entries;
    frame_clock.menu_tick = 0;
    ResetMaterialAnimationList(entries);
    ResetMaterialAnimationList(resource_groups[5].animation_entries);
}

uint32 DrawHudCounter(uint32 index, uint32 value, uint32 origin_x, uint32 origin_y, uint32 hidden_x)
{
    TextureDescriptor *texture;
    uint32 *metadata;
    FUNCTION_MARKER(0x80065478, "SLUS_005.78");
    char text[40];
    TextureDescriptor *icon, *group;
    uint32 x, y, distance, phase, timer, count, right, left, top, width;
    uint32 product, result, remainder, groups, pass, saved_half, blend;
    const uint8 *atlas, *glyph;
    uint32 first, second, tile;
    x = origin_x;
    y = origin_y;
    if (index == 1)
    {
        icon = resource_groups[28].texture;
        texture = icon;
        right = texture->payload.bytes[2];
        left = texture->payload.bytes[0];
        x = 20;
        y = 19;
        hidden_x = ~(right - left);
    }
    else if ((sint32)index < 2)
    {
        if (index == 0)
        {
            hidden_x = 320;
            x = 240;
            y = 25;
        }
    }
    else if (index == 2)
    {
        icon = resource_groups[5].texture;
        texture = icon;
        right = texture->payload.bytes[2];
        left = texture->payload.bytes[0];
        icon = resource_groups[28].texture;
        x = 23;
        texture = icon;
        y = texture->payload.bytes[3];
        top = texture->payload.bytes[1];
        hidden_x = 0 - (right - left + 37);
        y = y - top + 24;
    }
    phase = hud.counters[index].state;
    distance = x - hidden_x;
    if (phase == 1)
        goto slide_in;
    if (phase == 0)
    {
        result = (uint32)screen_render.transition_ticks;
        if (result == 0 && value != 0xFFFFFFFF)
        {
            result = hud.counters[index].shown_value;
            hud.counters[index].state = 1;
            hud.counters[index].ticks = 7;
            if (result == 0xFFFFFFFF)
                hud.counters[index].shown_value = value;
        }
        goto finish_hud;
    }
    if (phase == 2)
    {
        if ((game_flow.transition_flags & 0x10) == 0 && level_resource.level_id != 0)
        {
            timer = hud.counters[index].ticks - 1;
            hud.counters[index].ticks = timer;
            if ((timer & 0xFF) == 0 || value == 0xFFFFFFFF)
            {
                hud.counters[index].state = 3;
                hud.counters[index].ticks = 7;
                goto draw_hud;
            }
            if (hud.counters[index].shown_value == value)
                goto draw_hud;
        }
        hud.counters[index].ticks = 1;
    slide_in:
        timer = hud.counters[index].ticks - 1;
        hud.counters[index].ticks = timer;
        if ((timer & 0xFF) != 0 && value != 0xFFFFFFFF)
        {
            timer = hud.counters[index].ticks;
            product = distance * timer;
            x -= (uint32)((sint32)product / 7);
        }
        else
        {
            hud.counters[index].state = 2;
            hud.counters[index].ticks = 90;
            if (value == 0xFFFFFFFF)
                goto draw_hud;
            count = hud.counters[index].shown_value;
            if (count < value)
                hud.counters[index].shown_value = count + 1;
            else if (value < count)
            {
                if (count == 0xFFFFFFFF)
                    hud.counters[index].shown_value = value;
                else if (count - value >= 5)
                    hud.counters[index].shown_value = count - 5;
                else
                    hud.counters[index].shown_value = count - 1;
            }
        }
        goto draw_hud;
    }
    if (phase == 3)
    {
        timer = hud.counters[index].ticks - 1;
        hud.counters[index].ticks = timer;
        if ((timer & 0xFF) == 0)
        {
            result = 0xFFFFFFFF;
            hud.counters[index].state = 0;
            goto finish_hud;
        }
        if (value != 0xFFFFFFFF && (hud.counters[index].shown_value != value || (game_flow.transition_flags & 0x10) != 0 || level_resource.level_id == 0))
        {
            timer = 7 - hud.counters[index].ticks;
            product = distance * (timer & 0xFF);
            hud.counters[index].ticks = timer;
            hud.counters[index].state = 1;
            x -= (uint32)((sint32)product / 7);
        }
        else
        {
            timer = hud.counters[index].ticks;
            product = distance * timer;
            x = hidden_x + (uint32)((sint32)product / 7);
        }
    }
draw_hud:
    count = hud.counters[index].shown_value;
    result = (sint32)index < 2;
    if (index == 1)
    {
        icon = resource_groups[28].texture;
        if (count == 0 || icon == 0)
            goto finish_hud;
        if (count == 1)
            hud.counters[1].shown_value = 0xFFFFFFFF;
        else if (count == 0xFFFFFFFF)
            count = 1;
        DrawTexturedQuad(x | (y << 16), 0, icon, 0);
        remainder = count - 1;
        groups = remainder >> 2;
        for (pass = 0; pass < 2; ++pass)
        {
            if (pass == 0)
            {
                if (groups == 0)
                    continue;
                metadata = resource_groups[29].animation_entries;
                tile = (uint8)metadata[-1];
                group = resource_groups[((groups + 28) << 3) / 8].texture;
                tile = metadata[tile - 1];
                atlas = material_registry.records[(tile >> 1) & 0x1FF].data;
                texture = group;
                saved_half = texture->payload.halfwords[3];
                glyph = atlas + 24;
                blend = 1;
            }
            else
            {
                metadata = resource_groups[29].animation_entries;
                group = resource_groups[((groups + 29) << 3) / 8].texture;
                tile = (uint8)metadata[-1];
                tile = metadata[tile - 1];
                texture = group;
                saved_half = texture->payload.halfwords[3];
                atlas = material_registry.records[(tile >> 1) & 0x1FF].data;
                glyph = atlas + ((remainder & 3) << 3);
                blend = 0;
            }
            first = xport_load_le32(glyph);
            second = xport_load_le32(glyph + 4);
            texture->payload.words[0] = first;
            texture->payload.words[1] = second;
            texture->payload.halfwords[3] = saved_half;
            result = DrawTexturedQuad((x + 3) | ((y + 3) << 16), 0, group, blend);
        }
        goto finish_hud;
    }
    if ((sint32)index < 2)
    {
        if (index != 0)
            goto finish_hud;
        icon = resource_groups[1].texture;
        if (icon != 0)
            DrawTexturedQuad(x | (y << 16), 0x180018, icon, 0);
        x += 32;
        y += 14;
        value = count;
    }
    else
    {
        result = 2;
        if (index != 2)
            goto finish_hud;
        icon = resource_groups[5].texture;
        texture = icon;
        right = texture->payload.bytes[2];
        left = texture->payload.bytes[0];
        width = right - left + 1;
        if (icon != 0)
            DrawTexturedQuad(x | (y << 16), 0, icon, 0);
        x += 4 + width;
        y += 15;
        value = count == 0 ? 0 : count - 1;
    }
    sprintf(text, UiText::NumberFormat, (sint32)value);
    result = DrawText(x | (y << 16), text, 0, 0);
finish_hud:
    return result;
}

uint32 DrawHud(const AnimatedObject *object)
{
    FUNCTION_MARKER(0x80065BC4, "SLUS_005.78");
    uint32 result, value, index, unchanged, mode;
    const char *text;
    CharacterSettings settings;
    const char *prompt;
    const TextureDescriptor *target;
    uint32 *group_entries;
    result = game_flow.transition_flags & 0x10;
    if (result == 0)
    {
        hud.suppress_counters = 0;
        result = CanOpenPauseMenu() << 24;
        if (result != 0)
        {
            value = game_flow.transition_flags;
            mode = hud.counters[0].state;
            game_flow.transition_flags = value | 0x10;
            result = 3;
            if (mode == 1 || mode == 3)
                hud.suppress_counters = 1;
        }
    }
    if (object == 0)
        return result;
    CaptureCharacterSettings(&settings);
    if ((game_flow.transition_flags & 0x20000) != 0)
    {
        settings.counters[CharacterSettings::Health] = 0;
        settings.counters[CharacterSettings::Lives] = 0;
    }
    unchanged = 1;
    if (hud.suppress_counters == 0)
    {
        for (index = 0; index < 3; ++index)
        {
            if (hud.counters[index].state != 0 || hud.counters[index].shown_value != settings.counters[index] || (game_flow.transition_flags & 0x10) != 0 || level_resource.level_id == 0)
            {
                // Indices0..2 overwrite all three incoming layout values
                DrawHudCounter(index, settings.counters[index], 0, 0, 0);
                unchanged = 0;
            }
        }
    }
    if (unchanged != 0)
    {
        group_entries = resource_groups[1].animation_entries;
        frame_clock.menu_tick = 0;
        ResetMaterialAnimationList(group_entries);
        ResetMaterialAnimationList(resource_groups[5].animation_entries);
    }
    if ((game_flow.transition_flags & 2) != 0)
    {
        mode = (frame_clock.render_tick & 6) >> 1;
        if (mode == 0)
            text = UiText::Demo[0];
        else if (mode == 2)
            text = UiText::Demo[2];
        else
            text = UiText::Demo[1];
        DrawText(0x00CA00A0, text, 1, 0);
    }
    result = level_resource.level_id;
    if (result != 0)
    {
        return result;
    }
    index = scene_events.world_name_index;
    if (index != 0)
    {
        DrawText(0x005500A0, level_resource.world_names[index - 1].text, 1, 0);
        index = scene_events.world_name_index;
    }
    if (index == 0)
        hud.prompt_phase = 0;
    else
    {
        value = frame_clock.render_tick;
        if (hud.prompt_deadline < value)
        {
            mode = hud.prompt_phase;
            hud.prompt_deadline = value + 60;
            hud.prompt_phase = 1u - mode;
        }
    }
    value = render_commands.overlay_tail;
    mode = hud.prompt_phase;
    hud.prompt_tail = value;
    if (mode == 0)
    {
        target = interface_resources->textures + 11;
        if (target != 0)
            DrawTexturedQuad(0x00C8001E, 0, target, 0);
        prompt = UiText::SwitchPlayer;
    }
    else
    {
        target = interface_resources->textures + 8;
        if (target != 0)
            DrawTexturedQuad(0x00C8001E, 0, target, 0);
        prompt = UiText::EnterLevel;
    }
    result = DrawText(0x00CA003C, prompt, 0, 0);
    return result;
}
