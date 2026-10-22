#ifndef QUATERNION16_H
#define QUATERNION16_H
#include <stddef.h>

#include "psx.h"

union Quat;

union Quat
{
    struct
    {
        sint16 w, x, y, z;
    };

    sint16 comp[4];

    static void MatrixToQuat(const MATRIX *matrix, Quat *quat);
    static void ComposeHeadingQuat(sint16 angle, const Quat *base, Quat *dst);
    static void RotateMatrixByQuat(MATRIX *dst, const MATRIX *src, const Quat *quat);
};

#endif
