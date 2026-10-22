#ifndef FIXED_MATH_H
#define FIXED_MATH_H
#include <stddef.h>

#include "psx.h"

extern MATRIX rotation_x;

extern MATRIX rotation_y;

extern MATRIX rotation_z;

extern uint32 fixed_math_result;

sint16 Atan2Angle(sint32 x, sint32 y);

sint16 SinQ14(sint16 angle);

uint32 SqRootQ12Exact(uint32 val);

sint16 AsinAngle(sint32 height, sint32 len);

uint32 ApproxVectorLen(uint32 x, uint32 y, uint32 z);

sint16 AcosAngle(sint32 height, sint32 len);

#endif
