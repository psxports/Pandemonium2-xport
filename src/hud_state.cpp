#include <stdio.h>

#include "hud_state.h"
#include "game.h"

HudState hud = {0};

static const char switch_player[] = "TO SWITCH PLAYER";

static const char enter_level[] = "TO ENTER LEVEL";

static const char number_format[] = "%d";

static const char demo[3][11] = {"DEMO", "D E M O", "D  E  M  O"};

void HudState::ResetHud(void)
{
    uint32 idx;
    uint32 *entries;
    FUNCTION_MARKER(0x8006540C, "SLUS_005.78");
    for (idx = 0; idx < 3; ++idx)
    {
        hud.counters[idx].state = 0;
        hud.counters[idx].shown_val = 0xFFFFFFFF;
    }
    entries = resource_grps[1].animation_entries;
    frame_clock.menu_tick = 0;
    MaterialAnimationList::ResetMaterialAnimationList(entries);
    MaterialAnimationList::ResetMaterialAnimationList(resource_grps[5].animation_entries);
}

void HudState::DrawHudCounter(uint32 idx, uint32 val, uint32 ox, uint32 oy, uint32 hidden_x)
{
    TextureDescriptor *tex;
    uint32 *animation_entries;
    FUNCTION_MARKER(0x80065478, "SLUS_005.78");
    char text[40];
    TextureDescriptor *icon, *grp;
    uint32 x, y, distance, phase, timer, count, right, left, top, width;
    sint32 product;
    uint32 result;
    uint32 remainder;
    uint32 grps;
    uint32 pass;
    uint32 saved_half;
    uint32 blend;
    const uint8 *atlas, *glyph;
    uint32 tile;
    Glyph glyph_data;
    x = ox;
    y = oy;
    if (idx == 1)
    {
        icon = resource_grps[28].tex;
        tex = icon;
        right = tex->mapping.right;
        left = tex->mapping.left;
        x = 20;
        y = 19;
        hidden_x = ~(right - left);
    }
    else if ((sint32)idx < 2)
    {
        if (idx == 0)
        {
            hidden_x = 320;
            x = 240;
            y = 25;
        }
    }
    else if (idx == 2)
    {
        icon = resource_grps[5].tex;
        tex = icon;
        right = tex->mapping.right;
        left = tex->mapping.left;
        icon = resource_grps[28].tex;
        x = 23;
        tex = icon;
        y = tex->mapping.bottom;
        top = tex->mapping.top;
        hidden_x = -(right - left + 37);
        y = y - top + 24;
    }
    phase = hud.counters[idx].state;
    distance = x - hidden_x;
    if (phase == 1)
        goto slide_in;
    if (phase == 0)
    {
        result = scr_render.transition_ticks;
        if (result == 0 && val != 0xFFFFFFFF)
        {
            result = hud.counters[idx].shown_val;
            hud.counters[idx].state = 1;
            hud.counters[idx].ticks = 7;
            if (result == 0xFFFFFFFF)
                hud.counters[idx].shown_val = val;
        }
        goto finish_hud;
    }
    if (phase == 2)
    {
        if ((game_flow.transition_flags & 0x10) == 0 && level_resource.level_id != 0)
        {
            timer = hud.counters[idx].ticks - 1;
            hud.counters[idx].ticks = timer;
            if ((timer & 0xFF) == 0 || val == 0xFFFFFFFF)
            {
                hud.counters[idx].state = 3;
                hud.counters[idx].ticks = 7;
                goto draw_hud;
            }
            if (hud.counters[idx].shown_val == val)
                goto draw_hud;
        }
        hud.counters[idx].ticks = 1;
    slide_in:
        timer = hud.counters[idx].ticks - 1;
        hud.counters[idx].ticks = timer;
        if ((timer & 0xFF) != 0 && val != 0xFFFFFFFF)
        {
            timer = hud.counters[idx].ticks;
            product = distance * timer;
            x -= (uint32)(product / 7);
        }
        else
        {
            hud.counters[idx].state = 2;
            hud.counters[idx].ticks = 90;
            if (val == 0xFFFFFFFF)
                goto draw_hud;
            count = hud.counters[idx].shown_val;
            if (count < val)
                hud.counters[idx].shown_val = count + 1;
            else if (val < count)
            {
                if (count == 0xFFFFFFFF)
                    hud.counters[idx].shown_val = val;
                else if (count - val >= 5)
                    hud.counters[idx].shown_val = count - 5;
                else
                    hud.counters[idx].shown_val = count - 1;
            }
        }
        goto draw_hud;
    }
    if (phase == 3)
    {
        timer = hud.counters[idx].ticks - 1;
        hud.counters[idx].ticks = timer;
        if ((timer & 0xFF) == 0)
        {
            hud.counters[idx].state = 0;
            goto finish_hud;
        }
        if (val != 0xFFFFFFFF && (hud.counters[idx].shown_val != val || (game_flow.transition_flags & 0x10) != 0 || level_resource.level_id == 0))
        {
            timer = 7 - hud.counters[idx].ticks;
            product = distance * (timer & 0xFF);
            hud.counters[idx].ticks = timer;
            hud.counters[idx].state = 1;
            x -= (uint32)(product / 7);
        }
        else
        {
            timer = hud.counters[idx].ticks;
            product = distance * timer;
            x = hidden_x + (product / 7);
        }
    }
draw_hud:
    count = hud.counters[idx].shown_val;

    if (idx == 1)
    {
        icon = resource_grps[28].tex;
        if (count == 0 || icon == 0)
            goto finish_hud;
        if (count == 1)
            hud.counters[1].shown_val = 0xFFFFFFFF;
        else if (count == 0xFFFFFFFF)
            count = 1;
        MaterialCatalog::DrawTexturedQuad(x | (y << 16), 0, icon, 0);
        remainder = count - 1;
        grps = remainder >> 2;
        for (pass = 0; pass < 2; ++pass)
        {
            if (pass == 0)
            {
                if (grps == 0)
                    continue;
                animation_entries = resource_grps[29].animation_entries;
                tile = ((const MaterialAnimationListHeader *)animation_entries - 1)->count;
                grp = resource_grps[((grps + 28) << 3) / 8].tex;
                tile = animation_entries[tile - 1];
                atlas = material_registry.recs[(tile >> 1) & 0x1FF].data;
                tex = grp;
                saved_half = tex->mapping.clut;
                glyph = atlas + 24;
                blend = 1;
            }
            else
            {
                animation_entries = resource_grps[29].animation_entries;
                grp = resource_grps[((grps + 29) << 3) / 8].tex;
                tile = ((const MaterialAnimationListHeader *)animation_entries - 1)->count;
                tile = animation_entries[tile - 1];
                tex = grp;
                saved_half = tex->mapping.clut;
                atlas = material_registry.recs[(tile >> 1) & 0x1FF].data;
                glyph = atlas + ((remainder & 3) << 3);
                blend = 0;
            }
            glyph_data = *(const Glyph *)glyph;
            tex->mapping.uv_rect = glyph_data.uv_rect;
            tex->mapping.page_clut = glyph_data.page_clut;
            tex->mapping.clut = saved_half;
            MaterialCatalog::DrawTexturedQuad((x + 3) | ((y + 3) << 16), 0, grp, blend);
        }
        goto finish_hud;
    }
    if ((sint32)idx < 2)
    {
        if (idx != 0)
            goto finish_hud;
        icon = resource_grps[1].tex;
        if (icon != 0)
            MaterialCatalog::DrawTexturedQuad(x | (y << 16), 0x180018, icon, 0);
        x += 32;
        y += 14;
        val = count;
    }
    else
    {
        if (idx != 2)
            goto finish_hud;
        icon = resource_grps[5].tex;
        tex = icon;
        right = tex->mapping.right;
        left = tex->mapping.left;
        width = right - left + 1;
        if (icon != 0)
            MaterialCatalog::DrawTexturedQuad(x | (y << 16), 0, icon, 0);
        x += 4 + width;
        y += 15;
        val = count == 0 ? 0 : count - 1;
    }
    sprintf(text, number_format, (sint32)val);
    UiText::DrawText(x | (y << 16), text, 0, 0);
finish_hud:
    return;
}

void HudState::DrawHud(const AnimatedObject *object)
{
    FUNCTION_MARKER(0x80065BC4, "SLUS_005.78");
    uint32 result, val, idx, unchanged, mode;
    const char *text;
    CharacterSettings settings;
    const char *prompt;
    const TextureDescriptor *target;
    uint32 *grp_entries;
    result = game_flow.transition_flags & 0x10;
    if (result == 0)
    {
        hud.suppress_counters = 0;
        result = GameFlowState::CanOpenPauseMenu() << 24;
        if (result != 0)
        {
            val = game_flow.transition_flags;
            mode = hud.counters[0].state;
            game_flow.transition_flags = val | 0x10;

            if (mode == 1 || mode == 3)
                hud.suppress_counters = 1;
        }
    }
    if (object == 0)
        return;
    CharacterController::CaptureCharacterSettings(&settings);
    if ((game_flow.transition_flags & 0x20000) != 0)
    {
        settings.counters[CharacterSettings::CNT_HEALTH] = 0;
        settings.counters[CharacterSettings::CNT_LIVES] = 0;
    }
    unchanged = 1;
    if (hud.suppress_counters == 0)
    {
        for (idx = 0; idx < 3; ++idx)
        {
            if (hud.counters[idx].state != 0 || hud.counters[idx].shown_val != settings.counters[idx] || (game_flow.transition_flags & 0x10) != 0 || level_resource.level_id == 0)
            {
                // Indices0..2 overwrite all three incoming layout values
                HudState::DrawHudCounter(idx, settings.counters[idx], 0, 0, 0);
                unchanged = 0;
            }
        }
    }
    if (unchanged != 0)
    {
        grp_entries = resource_grps[1].animation_entries;
        frame_clock.menu_tick = 0;
        MaterialAnimationList::ResetMaterialAnimationList(grp_entries);
        MaterialAnimationList::ResetMaterialAnimationList(resource_grps[5].animation_entries);
    }
    if ((game_flow.transition_flags & 2) != 0)
    {
        mode = (frame_clock.render_tick & 6) >> 1;
        if (mode == 0)
            text = demo[0];
        else if (mode == 2)
            text = demo[2];
        else
            text = demo[1];
        UiText::DrawText(0x00CA00A0, text, 1, 0);
    }
    result = level_resource.level_id;
    if (result != 0)
    {
        return;
    }
    idx = scene_events.world_name_idx;
    if (idx != 0)
    {
        UiText::DrawText(0x005500A0, level_resource.world_names[idx - 1].text, 1, 0);
        idx = scene_events.world_name_idx;
    }
    if (idx == 0)
        hud.prompt_phase = 0;
    else
    {
        val = frame_clock.render_tick;
        if (hud.prompt_deadline < val)
        {
            mode = hud.prompt_phase;
            hud.prompt_deadline = val + 60;
            hud.prompt_phase = 1u - mode;
        }
    }
    val = render_commands.overlay_tail;
    mode = hud.prompt_phase;
    hud.prompt_tail = val;
    if (mode == 0)
    {
        target = interface_resources->texs + 11;
        if (target != 0)
            MaterialCatalog::DrawTexturedQuad(0x00C8001E, 0, target, 0);
        prompt = switch_player;
    }
    else
    {
        target = interface_resources->texs + 8;
        if (target != 0)
            MaterialCatalog::DrawTexturedQuad(0x00C8001E, 0, target, 0);
        prompt = enter_level;
    }
    UiText::DrawText(0x00CA003C, prompt, 0, 0);
}
