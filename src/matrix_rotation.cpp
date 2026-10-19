#include "game.h"

MATRIX rotation_x = {0};
MATRIX rotation_y = {0};
MATRIX rotation_z = {0};

MATRIX *RotateMatrixY(MATRIX *dst, MATRIX *src, sint16 angle)
{
    FUNCTION_MARKER(0x8002F318, "SLUS_005.78");
    sint32 cosine = SinQ14((sint16)(1024 - angle)) >> 2;
    sint32 sine = SinQ14(angle) >> 2;
    rotation_y.m[0][0] = (sint16)(cosine);
    rotation_y.m[2][0] = (sint16)(-sine);
    rotation_y.m[0][2] = (sint16)(sine);
    rotation_y.m[2][2] = (sint16)(cosine);
    MulMatrix0(src, &rotation_y, dst);
    return dst;
}

MATRIX *RotateMatrixYX(MATRIX *dst, MATRIX *src, sint16 first_angle, sint16 second_angle)
{
    FUNCTION_MARKER(0x8002F3B0, "SLUS_005.78");
    MATRIX temporary;
    sint32 cosine = SinQ14((sint16)(1024 - second_angle)) >> 2;
    sint32 sine = SinQ14(second_angle) >> 2;
    rotation_y.m[0][0] = (sint16)(cosine);
    rotation_y.m[2][0] = (sint16)(-sine);
    rotation_y.m[0][2] = (sint16)(sine);
    rotation_y.m[2][2] = (sint16)(cosine);
    MulMatrix0(src, &rotation_y, &temporary);
    cosine = SinQ14((sint16)(1024 - first_angle)) >> 2;
    sine = SinQ14(first_angle) >> 2;
    rotation_x.m[1][1] = (sint16)(cosine);
    rotation_x.m[2][1] = (sint16)(sine);
    rotation_x.m[1][2] = (sint16)(-sine);
    rotation_x.m[2][2] = (sint16)(cosine);
    MulMatrix0(&temporary, &rotation_x, dst);
    return dst;
}

MATRIX *RotateMatrixXY(MATRIX *dst, MATRIX *src, sint16 first_angle, sint16 second_angle)
{
    FUNCTION_MARKER(0x8002F4C0, "SLUS_005.78");
    MATRIX temporary;
    sint32 cosine = SinQ14((sint16)(1024 - first_angle)) >> 2;
    sint32 sine = SinQ14(first_angle) >> 2;
    rotation_x.m[1][1] = (sint16)(cosine);
    rotation_x.m[2][1] = (sint16)(sine);
    rotation_x.m[1][2] = (sint16)(-sine);
    rotation_x.m[2][2] = (sint16)(cosine);
    cosine = SinQ14((sint16)(1024 - second_angle)) >> 2;
    sine = SinQ14(second_angle) >> 2;
    rotation_y.m[0][0] = (sint16)(cosine);
    rotation_y.m[2][0] = (sint16)(-sine);
    rotation_y.m[0][2] = (sint16)(sine);
    rotation_y.m[2][2] = (sint16)(cosine);
    MulMatrix0(&rotation_y, &rotation_x, &temporary);
    MulMatrix0(src, &temporary, dst);
    return dst;
}

MATRIX *RotateMatrixXYZ(MATRIX *dst, MATRIX *src, sint16 first_angle, sint16 second_angle, sint16 third_angle)
{
    FUNCTION_MARKER(0x8002F5D0, "SLUS_005.78");
    MATRIX first, second;
    sint32 cosine = SinQ14((sint16)(1024 - first_angle)) >> 2;
    sint32 sine = SinQ14(first_angle) >> 2;
    rotation_x.m[1][1] = (sint16)(cosine);
    rotation_x.m[2][1] = (sint16)(sine);
    rotation_x.m[1][2] = (sint16)(-sine);
    rotation_x.m[2][2] = (sint16)(cosine);
    MulMatrix0(src, &rotation_x, &first);
    cosine = SinQ14((sint16)(1024 - second_angle)) >> 2;
    sine = SinQ14(second_angle) >> 2;
    rotation_y.m[0][0] = (sint16)(cosine);
    rotation_y.m[2][0] = (sint16)(-sine);
    rotation_y.m[0][2] = (sint16)(sine);
    rotation_y.m[2][2] = (sint16)(cosine);
    if (third_angle != 0)
    {
        MulMatrix0(&first, &rotation_y, &second);
        cosine = SinQ14((sint16)(1024 - third_angle)) >> 2;
        sine = SinQ14(third_angle) >> 2;
        rotation_z.m[0][0] = (sint16)(cosine);
        rotation_z.m[1][0] = (sint16)(sine);
        rotation_z.m[0][1] = (sint16)(-sine);
        rotation_z.m[1][1] = (sint16)(cosine);
        MulMatrix0(&rotation_z, &second, dst);
    }
    else
        MulMatrix0(&first, &rotation_y, dst);
    return dst;
}
