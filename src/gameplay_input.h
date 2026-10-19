#ifndef GAMEPLAY_INPUT_H
#define GAMEPLAY_INPUT_H

#include <stddef.h>

class GameplayInput
{
public:
    unsigned int buttons;
    unsigned int prev_buttons;
    const unsigned char *demo_cursor;
    unsigned char demo_index;
    unsigned int release_mask;
    unsigned int debug_previous_buttons;
    unsigned char rotation_track_shift;
    unsigned char pad_packet[34];

    unsigned int ApplyDemo();
};

extern GameplayInput gameplay_input;

unsigned int RunDemo();
unsigned int UpdateGameplayInput(void);
unsigned int InitializeGamepad();
unsigned int ReadGamepadButtons();
unsigned int IsGamepadDisconnected();

unsigned int FilterDebugInput(unsigned int input);

#endif
