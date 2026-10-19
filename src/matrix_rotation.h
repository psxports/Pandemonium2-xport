#ifndef MATRIX_ROTATION_H
#define MATRIX_ROTATION_H

#include "psx.h"

extern MATRIX rotation_x;
extern MATRIX rotation_y;
extern MATRIX rotation_z;

MATRIX *RotateMatrixY(MATRIX *dst, MATRIX *src, sint16 angle);
MATRIX *RotateMatrixYX(MATRIX *dst, MATRIX *src, sint16 first_angle, sint16 second_angle);
MATRIX *RotateMatrixXY(MATRIX *dst, MATRIX *src, sint16 first_angle, sint16 second_angle);
MATRIX *RotateMatrixXYZ(MATRIX *dst, MATRIX *src, sint16 first_angle, sint16 second_angle, sint16 third_angle);

#endif
