#ifndef CHARACTER_PATH_TRANSITION_H
#define CHARACTER_PATH_TRANSITION_H

#include "psx.h"

struct AnimatedObject;
struct ScenePath;

struct PathTransitionRules
{
    uint8 follow_enabled;
    uint32 follow_height;
    uint32 follow_side_offset;
    uint8 animation_tail_frames;
    uint8 follow_limit;
    uint8 follow_side_ticks;
    uint8 reentry_ticks;
    uint8 exit_ticks;
    uint8 special_ticks;
    uint8 camera_duration;
    uint8 wait_animation_start;
    uint32 side_step;
    uint32 height_bias;
    uint8 release_ticks;
    uint8 exit_hold_ticks;
    uint16 angle_limit;
    uint32 approach_limit;
    uint32 min_height;
    uint32 max_height;
    uint32 air_height_limit;
    uint8 early_movement_ticks;
    sint8 release_mode;
    sint32 release_speed_scale;
    sint8 hold_motion_limit;
    // Q12 factors for released and constrained movement
    sint32 release_motion_scale;
    sint32 axis_horizontal_acceleration[2];
    sint32 axis_vertical_limits[2];
    uint8 axis_input_ticks;
    sint32 axis_speed_limit_scale;
};

extern const PathTransitionRules path_transition_defaults;
extern PathTransitionRules path_transition_rules;

uint32 *UpdateCharacterCameraTarget(uint32 *destination, const AnimatedObject *object);

uint32 CanContinuePathStep(void);
void ApplyCameraPositionCorrection(const AnimatedObject *owner, AnimatedObject *object);
void ApplyHeadingPositionCorrection(const AnimatedObject *owner, AnimatedObject *object);

const ScenePath *ReadCharacterConstrainedPath(void);
uint32 IsCharacterAnchorPathActive(void);

#endif
