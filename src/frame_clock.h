#ifndef FRAME_CLOCK_H
#define FRAME_CLOCK_H

#include <stddef.h>

#include "psx.h"

class FrameClock
{
public:
    unsigned int milliseconds;
    unsigned int frame_start;
    unsigned int render_tick;
    unsigned int menu_tick;
    unsigned int event_handle;
};

extern FrameClock frame_clock;

sint32 AdvanceGameClock();

extern "C"
{
    void InitializeGameClock(void);
    void ShutdownGameClock(void);
}

#endif
