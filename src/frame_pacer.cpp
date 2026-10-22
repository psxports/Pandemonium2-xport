#include "frame_pacer.h"
#include "game.h"
#include "psx.h"

uint32 FramePacer::Wait(uint32 prev)
{
    FUNCTION_MARKER(0x80066D78, "SLUS_005.78");
    const uint32 old_fraction = fraction;
    fraction -= 10;
    uint32 delay = 33;
    if (fraction == 0 || (fraction & 0x80000000u) != 0)
    {
        delay = 34;
        fraction = old_fraction + 20;
    }
    const uint32 deadline = prev + delay;
    while (frame_clock.milliseconds < deadline)
        psx_native_poll_events();
    return frame_clock.milliseconds;
}
