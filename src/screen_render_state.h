#ifndef SCREEN_RENDER_STATE_H
#define SCREEN_RENDER_STATE_H
#include <stddef.h>

#include "psx.h"

struct ScreenRenderState;

struct ScreenRenderState
{
    uint16 fade_level;
    sint16 transition_ticks;
    uint32 brighten_ticks, brighten_duration;
    uint32 darken_ticks, darken_duration;
    uint16 background_angle;
    uint32 background_proj;
    uint8 background_scroll;
    uint32 startup_image_loaded;
    sint32 startup_image_x, startup_image_y;

    static void StartSceneFadeIn(void);
    static uint32 UpdateScrEffects(void);
    static void DrawSceneInterface(void);
    static void EmptyScrOperation(void);
    static void DrawSceneFade(void);
    static void DrawBackgroundBorders(sint32 top, uint32 height);
    static void DrawOverlayRect(uint32 pos, uint32 extent, sint32 fraction, uint32 color);
    static void DrawBrightenRect(uint32 pos, uint32 extent, sint32 fraction, uint32 color);
    static uint32 DrawScrBrighten(sint32 intensity);
    static uint32 DrawScrDarken(sint32 intensity);
    static void UploadTexImage(uint8 *image, uint32 idx, uint32 flags);
    static void SaveMenuImage(void);
    static void DrawStartupImage(uint32 *image);
    static void RestoreMenuImage(uint32 mode);
    static void DrawBackgroundRect(uint32 pos, uint32 extent, uint32 color);
    static void FadeMenuImage(uint32 color);
    static uint32 DrawGameOverFade(void);
};

// Demo input and animated maps share the original workspace
extern uint8 demo_background_workspace[8184];

extern ScreenRenderState scr_render;

#endif
