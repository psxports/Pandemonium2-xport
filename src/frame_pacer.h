#ifndef FRAME_PACER_H
#define FRAME_PACER_H

#include <stddef.h>

class FramePacer
{
public:

    unsigned int fraction;
    unsigned int Wait(unsigned int previous);
};

#endif
