#include "game.h"

FrameClock frame_clock = {0};

sint32 AdvanceGameClock(void)
{
    FUNCTION_MARKER(0x8006EBFC, "SLUS_005.78");
    return (sint32)++frame_clock.milliseconds;
}

void InitializeGameClock(void)
{
    uint32 handle;
    FUNCTION_MARKER(0x8006AF88, "SLUS_005.78");
    p2_func_8008346c();
    handle = (uint32)OpenEvent(0xF2000002, 2, 0x1000, AdvanceGameClock);
    frame_clock.event_handle = handle;
    EnableEvent((sint32)handle);
    SetRCnt(0xF2000002, 0x1088, 0x1000);
    StartRCnt(0xF2000002);
    p2_func_8008347c();
    frame_clock.milliseconds = 0;
}

void ShutdownGameClock(void)
{
    FUNCTION_MARKER(0x8006B000, "SLUS_005.78");
    p2_func_8008346c();
    psx_root_counter_stop(0xF2000002);
    DisableEvent((sint32)frame_clock.event_handle);
    CloseEvent((sint32)frame_clock.event_handle);
    p2_func_8008347c();
}
