#include "frame_pacer.h"
#include "frame_clock.h"
#include "psx.h"

unsigned int FramePacer::Wait(unsigned int previous)
{
    FUNCTION_MARKER(0x80066D78, "SLUS_005.78");
    const unsigned int old_fraction = fraction;
    fraction -= 10;
    unsigned int delay = 33;
    if (fraction == 0 || (fraction & 0x80000000u) != 0)
    {
        delay = 34;
        fraction = old_fraction + 20;
    }
    const unsigned int deadline = previous + delay;
    while (frame_clock.milliseconds < deadline)
        psx_native_poll_events();
    return frame_clock.milliseconds;
}
