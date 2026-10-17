#ifndef MENU_STATE_H
#define MENU_STATE_H
#include "frame_pacer.h"
#include "original_defaults.h"
#include "resource_groups.h"
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
    MaterialResourceHeader *resources;
    ResourceGroup material_groups[9];
};
extern MenuState menu_state;
#endif
