#include <stdio.h>

#include "ui_text.h"
#include "game.h"

const char UiText::game_over[] = "GAME OVER";

const char UiText::invalid_password[] = "INVALID PASSWORD";

const char UiText::password[] = "PASSWORD";

const char UiText::continue_text[] = "TO CONTINUE";

const char UiText::continue_quit[] = "CONTINUE  QUIT";

const char UiText::quit_game[] = "QUIT GAME?";

const char UiText::select_quit[] = "SELECT TO QUIT";

const char UiText::boss_defeated[] = "BOSS DEFEATED";

const char UiText::level_complete[] = "LEVEL COMPLETE";

const char UiText::bonus_over[] = "BONUS LEVEL OVER";

const char UiText::bonus_complete[] = "BONUS LEVEL COMPLETE";

const char UiText::treasure_format[] = "TREASURE: %d%%";

const char UiText::boarder_run[] = "FOR BOARDER RUN";

const char UiText::quit_choices[] = "YES  NO";

const char UiText::quit[] = "TO QUIT";

const char UiText::paused[] = "PAUSED";

uint32 UiText::DrawText(uint32 pos, const char *text, uint32 style, uint32 flags)
{
    TextureDescriptor *tex;
    uint32 *animation_entries;
    FUNCTION_MARKER(0x80064FF4, "SLUS_005.78");
    TextureDescriptor *grp;
    uint32 idx, tile, right, left, top, bottom, width, height;
    uint32 alignment, len, extent, step, result;
    uint8 character;
    const uint8 *atlas, *glyph;
    const char *cursor;
    Glyph glyph_data;
    sint32 scale;
    idx = OriginalDefaults::FontResourceGroup << 3;
    animation_entries = resource_grps[idx / 8].animation_entries;
    grp = resource_grps[idx / 8].tex;
    tex = grp;
    tile = ((const MaterialAnimationListHeader *)animation_entries - 1)->count;
    scale = (sint32)style >> 16;
    alignment = style & 0xFFFF;
    right = tex->mapping.right;
    tile = animation_entries[tile - 1];
    left = tex->mapping.left;
    top = tex->mapping.top;
    tile = (tile >> 1) & 0x1FF;
    bottom = tex->mapping.bottom;
    width = right - left + 1;
    atlas = material_registry.recs[tile].data;
    height = bottom - top + 1;
    if (alignment != 0)
    {
        character = *text;
        len = 0;
        cursor = text + 1;
        if (character != 0)
        {
            do
            {
                character = *cursor;
                ++len;
                ++cursor;
            } while (character != 0);
        }
        extent = width * len + ((len - 1) << 1);
        if (scale != 0)
            extent = (sint32)(scale * extent + 2048) >> 12;
        if (alignment == 1)
            pos -= extent >> 1;
        else
            pos -= extent;
    }
    step = width + 2;
    if (scale != 0)
        step = (sint32)(scale * step + 2048) >> 12;
    character = *text;
    result = height >> 1;
    pos -= result;
    cursor = text + 1;
    while (character != 0)
    {
        result = character >= 32 && character <= 90;
        if (result != 0)
        {
            result = character << 3;
            if (character != 32)
            {
                glyph = atlas + result - 256;
                glyph_data = *(const Glyph *)glyph;
                tex->mapping.uv_rect = glyph_data.uv_rect;
                tex->mapping.page_clut = glyph_data.page_clut;
                result = MaterialCatalog::DrawTexturedQuad(pos, 0, tex, flags);
            }
            pos += step;
        }
        character = *cursor;
        ++cursor;
    }
    return result;
}
