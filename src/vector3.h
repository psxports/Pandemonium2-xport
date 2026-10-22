#ifndef VECTOR3_H
#define VECTOR3_H
#include <stddef.h>

#include "psx.h"

union Vec3;

union Vec3
{
    struct
    {
        sint32 x, y, z;
    };

    sint32 comp[3];
};

#endif
