#ifndef FRAME_CLOCK_H
#define FRAME_CLOCK_H

#include <stddef.h>

class FrameClock
{
public:
    unsigned int milliseconds;
    unsigned int frame_start;
    unsigned int render_tick;
    unsigned int menu_tick;

};

extern FrameClock frame_clock;

#endif
