#ifndef MENU_STATE_H
#define MENU_STATE_H
#include <stddef.h>

#include "psx.h"
#include "frame_pacer.h"
#include "material_animation_list.h"
#include "material_catalog.h"

struct MaterialCatalog;
struct TextureDescriptor;

struct TextureDescriptor;

struct MenuMaterialResources;
struct MenuMaterialGroup;
struct PasswordState;
struct MenuState;

struct MenuMaterialResources
{
    MaterialCatalog header;
    TextureDescriptor *texs;
};

struct MenuMaterialGroup
{
    TextureDescriptor *tex;
    uint32 *animation_entries;
    MaterialAnimationList animation;
};

struct PasswordState
{
    uint8 characters[8];
    uint8 cursor;
    bool reset_pending;
    char feedback_buffer[21];
    const char *feedback;
};

struct MenuState
{
    uint32 prev_buttons, last_frame;
    FramePacer pacer;
    uint32 fade_frames, up_hold_frames, down_hold_frames, highlight_phase, highlight_step;
    PasswordState password;
    MenuMaterialResources *resources;
    MenuMaterialGroup material_grps[9];
    sint32 image_asset;
    MenuMaterialResources resource_view;

    static uint32 FindMaterialEntry(const TextureDescriptor *tex, uint32 kind, const MaterialCatalog *header, uint32 base);
    static void LoadMenuImage(sint32 asset);
    static void ReloadMenuArchive(void);
    static void LoadMenuResources(void);
    static uint32 DrawMenuSelection(uint32 pos, uint32 alternate, sint32 selection);
    static void DrawPassword(uint32 x, uint32 y, sint8 interactive);
    static void DrawPasswordScr(sint32 requested_mode, uint8 invalid, uint8 accepted);
    static void GeneratePassword(void);
    static sint32 ApplyCheatPassword(void);
    static uint32 EditPassword(uint32 requested_mode);
    static void DrawOptionsMenu(sint32 selection, sint32 music_volume, sint32 sound_volume);
    static uint32 RunOptionsMenu(void);
    static uint32 RunPasswordMenu(void);
    static void DrawQuitConfirmation(uint32 selection, uint32 mode);
    static uint32 RunQuitConfirmation(uint32 mode);
    static void DrawPauseMenu(void);
    static void RunPauseMenu(uint32 level);
    static void DrawMainMenu(uint32 selection);
    static sint32 RunMainMenu(void);
};

extern MenuState menu_state;

#endif
