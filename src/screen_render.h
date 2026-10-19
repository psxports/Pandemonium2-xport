#ifndef SCREEN_RENDER_H
#define SCREEN_RENDER_H

#include "psx.h"

struct ScreenRenderState
{
    uint16 fade_level;
    sint16 transition_ticks;
    uint32 brighten_ticks, brighten_duration;
    uint32 darken_ticks, darken_duration;
    uint16 background_angle;
    uint32 background_projection;
    uint8 background_scroll;
    uint32 startup_image_loaded;
    sint32 startup_image_x, startup_image_y;
};

struct BackgroundTile
{
    uint32 uv;
    uint32 attributes;
};

// Own file bytes and mutable background animation state
struct BackgroundResource
{
    uint8 *data;
    uint32 bytes;
    uint16 metadata;
    uint16 frame_index;
    uint16 frame_count;
    uint16 frame_rate;
    const uint8 *commands;
    const uint8 *command_cursor;
    const uint8 *command_end;
    uint32 last_tick;
    uint32 start_time;
    uint16 width;
    uint16 height;
    const uint8 *static_map;
    uint16 metadata_halfword;
    uint16 tile_count;
    BackgroundTile *tiles;

    BackgroundResource *Relocate();
};

// Demo input and animated maps share the original workspace
extern uint8 demo_background_workspace[8184];

extern ScreenRenderState screen_render;

extern "C"
{
    uint32 UploadTextureImage(uint8 *image, uint32 index, uint32 flags);
    void DrawSceneBackground(void);
    uint32 DrawBackgroundBorders(sint32 top, uint32 height);
    uint32 DrawSceneFade(void);
    uint32 DrawOverlayRectangle(uint32 position, uint32 extent, sint32 fraction, uint32 color);
    uint32 DrawBackgroundRectangle(uint32 position, uint32 extent, uint32 color);
}

uint32 DrawScreenBrighten(sint32 intensity);
uint32 DrawScreenDarken(sint32 intensity);
uint32 UpdateScreenEffects();
uint32 StartSceneFadeIn();
uint32 DrawSceneInterface();
sint32 DrawStartupImage(uint32 *image);
uint32 RestoreMenuImage(uint32 mode);
uint32 SaveMenuImage();

sint32 FadeMenuImage(uint32 color);
uint32 DrawBrightenRectangle(uint32 position, uint32 extent, sint32 fraction, uint32 color);

#endif
