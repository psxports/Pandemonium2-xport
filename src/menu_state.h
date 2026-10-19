#ifndef MENU_STATE_H
#define MENU_STATE_H
#include "frame_pacer.h"
#include "material_resources.h"
#include "original_defaults.h"
#include "resource_groups.h"
struct TextureDescriptor;

// Own the parent file while the material child retains its encoded graph
struct MenuArchive
{
    uint8 *data;
    uint32 bytes;
    uint32 capacity;
};

extern MenuArchive menu_archive;

struct MenuMaterialResources
{
    MaterialResourceHeader header;
    TextureDescriptor *textures;
};

struct MenuMaterialGroup
{
    TextureDescriptor *texture;
    unsigned int *animation_entries;
    unsigned int animation_words[4];
};

struct PasswordState
{
    unsigned char characters[8];
    unsigned char cursor;
    bool reset_pending;
    char feedback_buffer[21];
    const char *feedback;
};

struct MenuState
{
    unsigned int previous_buttons, last_frame;
    FramePacer pacer;
    unsigned int fade_frames, up_hold_frames, down_hold_frames, highlight_phase, highlight_step;
    PasswordState password;
    MenuMaterialResources *resources;
    MenuMaterialGroup material_groups[9];
    unsigned int image_asset;
    MenuMaterialResources resource_view;
};

extern MenuState menu_state;
unsigned int LoadMenuImage(unsigned int asset);
void ReloadMenuArchive();
int RunMainMenu();
unsigned int DrawMainMenu(unsigned int selection);
unsigned int RunOptionsMenu();
unsigned int DrawOptionsMenu(int selection, int music_volume, int sound_volume);
unsigned int DrawQuitConfirmation(unsigned int selection, unsigned int mode);
unsigned int RunQuitConfirmation(unsigned int mode);
unsigned int DrawPauseMenu();
int RunPauseMenu(unsigned int level);
unsigned int RunPasswordMenu();
unsigned int DrawMenuSelection(unsigned int position, unsigned int alternate, int selection);

#endif
