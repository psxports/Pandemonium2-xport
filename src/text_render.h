#ifndef TEXT_RENDER_H
#define TEXT_RENDER_H

#include "psx.h"

struct AnimatedObject;

struct UiText
{
    static const char SwitchPlayer[];
    static const char EnterLevel[];
    static const char GameOver[];
    static const char InvalidPassword[];
    static const char Password[];
    static const char Continue[];
    static const char ContinueQuit[];
    static const char QuitGame[];
    static const char SelectQuit[];
    static const char BossDefeated[];
    static const char LevelComplete[];
    static const char BonusOver[];
    static const char BonusComplete[];
    static const char TreasureFormat[];
    static const char BoarderRun[];
    static const char NumberFormat[];
    static const char QuitChoices[];
    static const char Quit[];
    static const char Paused[];
    static const char Demo[3][11];
};

struct HudCounter
{
    uint8 state;
    uint8 ticks;
    uint32 shown_value;
};

struct HudState
{
    HudCounter counters[3];
    uint32 suppress_counters;
    uint32 prompt_deadline;
    uint8 prompt_phase;
    uint32 prompt_tail;
};

extern HudState hud;

extern "C"
{
    uint32 DrawText(uint32 position, const char *text, uint32 style, uint32 flags);
    void ResetHud(void);
    uint32 DrawHudCounter(uint32 index, uint32 value, uint32 origin_x, uint32 origin_y, uint32 hidden_x);
    uint32 DrawHud(const AnimatedObject *object);
}

#endif
