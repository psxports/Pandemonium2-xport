#ifndef GAMEPLAY_INPUT_H
#define GAMEPLAY_INPUT_H
#include <stddef.h>

#include "psx.h"

struct DemoFrame;
class GameplayInput;

struct DemoFrame
{
    uint32 buttons;
};

class GameplayInput
{
public:
    uint32 buttons;
    uint32 prev_buttons;
    const DemoFrame *demo_cursor;
    uint8 demo_idx;
    uint32 release_mask;
    uint32 debug_prev_buttons;
    uint8 rotation_track_shift;
    uint8 pad_packet[34];

    void ApplyDemo();
    static void RunDemo(void);
    static void UpdateGameplayInput(void);
    static uint32 FilterDebugInput(uint32 input);
    static void InitializeGamepad(void);
    static uint32 ReadGamepadButtons(void);
    static uint32 IsGamepadDisconnected(void);
};

extern GameplayInput gameplay_input;

#endif
