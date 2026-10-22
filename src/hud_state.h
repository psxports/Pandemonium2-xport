#ifndef HUD_STATE_H
#define HUD_STATE_H
#include <stddef.h>

#include "psx.h"

struct AnimatedObject;

struct AnimatedObject;

struct HudCounter;
struct HudState;

struct HudCounter
{
    uint8 state;
    uint8 ticks;
    uint32 shown_val;
};

struct HudState
{
    HudCounter counters[3];
    uint32 suppress_counters;
    uint32 prompt_deadline;
    uint8 prompt_phase;
    uint32 prompt_tail;

    static void ResetHud(void);
    static void DrawHudCounter(uint32 idx, uint32 val, uint32 ox, uint32 oy, uint32 hidden_x);
    static void DrawHud(const AnimatedObject *object);
};

extern HudState hud;

#endif
