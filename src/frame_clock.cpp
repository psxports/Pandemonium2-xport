#include "frame_clock.h"
#include "game.h"
#include "psx.h"

FrameClock frame_clock = {0};

void FrameClock::InitializeGameClock(void)
{
    uint32 handle;
    FUNCTION_MARKER(0x8006AF88, "SLUS_005.78");
    EnterBiosCriticalSection();
    handle = OpenEvent(0xF2000002, 2, 0x1000, FrameClock::AdvanceGameClock);
    frame_clock.event_handle = handle;
    EnableEvent(handle);
    SetRCnt(0xF2000002, 0x1088, 0x1000);
    StartRCnt(0xF2000002);
    ExitBiosCriticalSection();
    frame_clock.milliseconds = 0;
}

void FrameClock::ShutdownGameClock(void)
{
    FUNCTION_MARKER(0x8006B000, "SLUS_005.78");
    EnterBiosCriticalSection();
    psx_root_counter_stop(0xF2000002);
    DisableEvent(frame_clock.event_handle);
    CloseEvent(frame_clock.event_handle);
    ExitBiosCriticalSection();
}

sint32 FrameClock::AdvanceGameClock(void)
{
    FUNCTION_MARKER(0x8006EBFC, "SLUS_005.78");
    return ++frame_clock.milliseconds;
}
