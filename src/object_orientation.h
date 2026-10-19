#ifndef OBJECT_ORIENTATION_H
#define OBJECT_ORIENTATION_H

#include "psx.h"

union Quaternion16
{
    sint16 components[4];
    uint32 words[2];
};

struct PathDefinition;
struct ScenePath;
struct AnimatedObject;
extern uint32 path_heading_delta[3];

extern "C"
{
    sint32 SampleDefinitionHeading(sint32 frame0, sint32 frame1, const PathDefinition *definition);
    sint32 SampleScenePathHeading(sint32 frame0, sint32 frame1, const ScenePath *path);
    uint32 MatrixToQuaternion(const MATRIX *matrix, Quaternion16 *quaternion);
    void RotateMatrixByQuaternion(MATRIX *destination, const MATRIX *source, const Quaternion16 *quaternion);
    uint32 ComposeHeadingQuaternion(sint16 angle, const Quaternion16 *base, Quaternion16 *destination);
    uint32 SampleObjectPathDirection(uint16 *angle, uint32 *direction_vector, const AnimatedObject *object, sint32 frame, uint32 reverse);
}

#endif
