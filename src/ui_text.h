#ifndef UI_TEXT_H
#define UI_TEXT_H
#include <stddef.h>

#include "psx.h"

struct UiText;
struct Glyph;

struct UiText
{
    static const char game_over[];
    static const char invalid_password[];
    static const char password[];
    static const char continue_text[];
    static const char continue_quit[];
    static const char quit_game[];
    static const char select_quit[];
    static const char boss_defeated[];
    static const char level_complete[];
    static const char bonus_over[];
    static const char bonus_complete[];
    static const char treasure_format[];
    static const char boarder_run[];
    static const char quit_choices[];
    static const char quit[];
    static const char paused[];

    static uint32 DrawText(uint32 pos, const char *text, uint32 style, uint32 flags);
};

struct Glyph
{
    uint32 uv_rect;
    uint32 page_clut;
};

#endif
