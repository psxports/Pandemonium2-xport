#ifndef SCENE_TRIGGER_H
#define SCENE_TRIGGER_H
#include <stddef.h>

#include "psx.h"
#include "animation_track.h"
#include "quaternion16.h"
#include "scene_events.h"
#include "scene_node.h"
#include "vector3.h"

struct SceneNode;

struct AnimatedObject;

struct SceneTrigger;

// Borrow the common region record; cutscene inputs use its first 84 bytes
struct SceneTrigger
{
    enum
    {
        ControlBytes = 84,
        AnimatedBytes = 108,
        CutsceneBytes = ControlBytes
    };

    SceneNode node;
    MATRIX transform;
    Vec3 delta;
    uint16 extent[3];
    uint16 radius;

    SceneTriggerParams parameters;

    union
    {
        uint32 control_word;
        const AnimatedObject *animation_owner;
        SceneCutsceneControlTail cutscene_control;
    };

    AnimationTrackBinding animation_binding;
    Vec3 pos;

    union
    {
        sint16 rotation[4];
        Quat quat;
    };

    void UpdateExtentsFromAnimation();
    uint32 PlaySound();
    uint32 StopSound();
    static void ProcessSceneTriggers(SceneNode *objects, SceneNode *auxiliary);
};










#endif
