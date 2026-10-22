#ifndef FRAME_CLOCK_H
#define FRAME_CLOCK_H
#include <stddef.h>

#include "psx.h"

class FrameClock;

class FrameClock
{
public:
    uint32 milliseconds;
    uint32 frame_start;
    uint32 render_tick;
    uint32 menu_tick;
    uint32 event_handle;

    static void InitializeGameClock(void);
    static void ShutdownGameClock(void);
    static sint32 AdvanceGameClock(void);
};

extern FrameClock frame_clock;

#endif
