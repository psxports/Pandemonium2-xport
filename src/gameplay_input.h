#ifndef GAMEPLAY_INPUT_H
#define GAMEPLAY_INPUT_H

#include <stddef.h>

class GameplayInput
{
public:
    unsigned int buttons;
    unsigned int demo_offset;
    unsigned int release_mask;

    unsigned int ApplyDemo();
};

extern GameplayInput gameplay_input;

#endif
