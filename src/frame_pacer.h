#ifndef FRAME_PACER_H
#define FRAME_PACER_H
#include <stddef.h>

#include "psx.h"

class FramePacer;

class FramePacer
{
public:
    uint32 fraction;

    uint32 Wait(uint32 prev);
};

#endif
