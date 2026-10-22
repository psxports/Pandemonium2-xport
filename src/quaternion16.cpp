#include <string.h>

#include "quaternion16.h"
#include "game.h"

void Quat::MatrixToQuat(const MATRIX *matrix, Quat *quat)
{
    FUNCTION_MARKER(0x80017E4C, "SLUS_005.78");
    sint32 diag0 = matrix->m[0][0];
    sint32 diag1 = matrix->m[1][1];
    sint32 diag2 = matrix->m[2][2];
    sint32 trace = diag0 + diag1 + diag2;
    sint32 denominator;
    sint32 rcp;
    sint32 coefficient;
    sint32 product;
    uint32 result;
    if (trace > 0)
    {
        denominator = SquareRoot12((((uint32)trace * 4 + 16384) >> 2)) * 4;
        if (denominator == 0)
            trap(7);
        rcp = 0x40000000 / denominator;
        quat->w = denominator >> 1;
        coefficient = mul14(8192, rcp);
        product = (uint32)(matrix->m[1][2] - matrix->m[2][1]) * 4 * coefficient;
        quat->x = product >> 14;
        product = (uint32)(matrix->m[2][0] - matrix->m[0][2]) * 4 * coefficient;
        quat->y = product >> 14;
        product = (uint32)(matrix->m[0][1] - matrix->m[1][0]) * 4 * coefficient;
        fixed_math_result = coefficient;
        result = product >> 14;
        quat->z = result;
    }
    else
    {
        uint32 idx0 = diag0 < diag1;
        uint32 idx1;
        uint32 idx2;
        const sint16 *row0;
        const sint16 *row1;
        const sint16 *row2;
        uint32 diagonal;
        if (matrix->m[idx0][idx0] < diag2)
            idx0 = 2;
        idx1 = idx0 + 1;
        if (idx1 == 3)
            idx1 = 0;
        idx2 = idx1 + 1;
        if (idx2 == 3)
            idx2 = 0;
        row0 = matrix->m[idx0];
        row1 = matrix->m[idx1];
        row2 = matrix->m[idx2];
        diagonal = row0[idx0] - (row1[idx1] + row2[idx2]);
        denominator = SquareRoot12(((diagonal * 4 + 16384) >> 2)) * 4;
        if (denominator == 0)
            trap(7);
        rcp = 0x40000000 / denominator;
        quat->comp[idx0 + 1] = denominator >> 1;
        coefficient = mul14(8192, rcp);
        product = (uint32)(row1[idx2] - row2[idx1]) * 4 * coefficient;
        fixed_math_result = coefficient;
        quat->w = product >> 14;
        product = (uint32)(row0[idx1] + row1[idx0]) * 4 * coefficient;
        quat->comp[idx1 + 1] = product >> 14;
        product = (uint32)(row0[idx2] + row2[idx0]) * 4 * coefficient;
        result = product >> 14;
        quat->comp[idx2 + 1] = result;
    }
}

void Quat::ComposeHeadingQuat(sint16 angle, const Quat *base, Quat *dst)
{
    FUNCTION_MARKER(0x80018FEC, "SLUS_005.78");
    Quat quat;
    const Quat *src;
    uint32 result;
    uint32 x, y, z;
    sint32 half_angle, s, q0, q1, q2, q3, p0, p1, p2, p3;
    half_angle = (sint32)(((uint32)2048 - angle) << 16) >> 17;
    if (base == 0)
    {
        result = SinQ14((1024 - half_angle));
        dst->w = result;
        dst->z = 0;
        dst->x = 0;
        result = SinQ14(half_angle);
        dst->y = result;
        return;
    }
    result = SinQ14((1024 - half_angle));
    quat.w = result;
    quat.z = 0;
    quat.x = 0;
    s = SinQ14(half_angle);
    quat.y = s;
    q0 = quat.w;
    src = 0;
    if (q0 == 0x4000)
        src = base;
    else
    {
        p0 = base->w;
        if (p0 == 0x4000)
            src = &quat;
        else
        {
            q1 = quat.x;
            p1 = base->x;
            p2 = base->y;
            q2 = (sint16)s;
            q3 = quat.z;
            p3 = base->z;
            x = (q0 * p0) - ((q1 * p1) + (uint32)(q2 * p2) + (q3 * p3));
            y = (q0 * p1) + (uint32)(p0 * q1) + (q2 * p3) - (q3 * p2);
            z = (q0 * p2) + (uint32)(p0 * q2) + (q3 * p1) - (q1 * p3);
            result = (q0 * p3) + (uint32)(p0 * q3) + (q1 * p2) - (q2 * p1);
            dst->w = x >> 14;
            dst->x = y >> 14;
            dst->y = z >> 14;
            result = (sint32)result >> 14;
            dst->z = result;
        }
    }
    if (src != 0)
    {
        *dst = *src;
    }
}

void Quat::RotateMatrixByQuat(MATRIX *dst, const MATRIX *src, const Quat *quat)
{
    // Q27 unity becomes Q12 after the coefficient shift
    FUNCTION_MARKER(0x8002F188, "SLUS_005.78");
    sint32 scalar = quat->w;
    MATRIX rotation = {0};
    sint32 x, y, z;
    uint32 xx, yy, zz, xy, xz, yz, sx, sy, sz;
    if (scalar == 0x4000)
    {
        if (dst != src)
            memcpy(dst->m, src->m, sizeof(dst->m));
        return;
    }
    x = quat->x;
    y = quat->y;
    z = quat->z;
    xx = x * x;
    yy = y * y;
    zz = z * z;
    xy = x * y;
    xz = x * z;
    yz = y * z;
    sx = scalar * x;
    sy = scalar * y;
    sz = scalar * z;
    rotation.m[0][0] = (sint32)((1u << 27) - (yy + zz)) >> 15;
    rotation.m[1][1] = (sint32)((1u << 27) - (xx + zz)) >> 15;
    rotation.m[2][2] = (sint32)((1u << 27) - (xx + yy)) >> 15;
    rotation.m[1][0] = (sint32)(xy - sz) >> 15;
    rotation.m[0][1] = (sint32)(xy + sz) >> 15;
    rotation.m[2][0] = (sint32)(xz + sy) >> 15;
    rotation.m[0][2] = (sint32)(xz - sy) >> 15;
    rotation.m[2][1] = (sint32)(yz - sx) >> 15;
    rotation.m[1][2] = (sint32)(yz + sx) >> 15;
    MulMatrix0((MATRIX *)src, &rotation, dst);
}
