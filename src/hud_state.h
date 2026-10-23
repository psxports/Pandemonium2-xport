#ifndef HUD_STATE_H
#define HUD_STATE_H
#include <stddef.h>

#include "psx.h"
#include "texture_descriptor.h"

struct AnimatedObject;

struct AnimatedObject;

struct HudCounter;
struct HudState;

struct HudCounter
{
    uint8 state;
    uint8 ticks;
    uint32 shown_val;
    // Last emitted counter layout, independent of the advanced timer state
    uint32 render_x, render_y, render_count;
    bool render_visible;
};

struct HudState
{
    HudCounter counters[3];
    uint32 suppress_counters;
    uint32 prompt_deadline;
    uint8 prompt_phase;
    uint32 prompt_tail;
    uint8 render_counters;
    TextureMapping render_font_mapping;

    static void ResetHud(void);
    static void DrawHudCounter(uint32 idx, uint32 val, uint32 ox, uint32 oy, uint32 hidden_x, bool advance = true);
    static void DrawHud(const AnimatedObject *object, bool advance = true);
};

extern HudState hud;

#endif
