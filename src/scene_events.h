#ifndef SCENE_EVENTS_H
#define SCENE_EVENTS_H

#include <stddef.h>

#include "psx.h"
#include "scene_node.h"
#include "animation_data.h"

struct SceneSoundControl
{
    uint32 sample_id;
    uint32 flags;

    union
    {
        uint32 word;
        uint8 bytes[4];
    } playback;
};

typedef char SceneSoundControlSize[(sizeof(SceneSoundControl) == 12) ? 1 : -1];
typedef char SceneSoundControlPlaybackOffset[(offsetof(SceneSoundControl, playback) == 8) ? 1 : -1];

struct SceneCutsceneControl
{
    uint32 camera_id;
    uint32 next_camera;
    uint32 duration;
};

struct SceneCutsceneControlTail
{
    uint16 flags;
    sint16 index;
};

struct ScenePath;
struct AnimatedObject;

struct ScenePathControl
{
    const ScenePath *target;
    uint32 values[2];
};

struct ScenePathPairControl
{
    const ScenePath *targets[2];
    uint32 metadata;
};

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
    uint32 delta[3];
    uint16 extent[3];
    uint16 radius;

    union
    {
        SceneSoundControl sound;
        SceneCutsceneControl cutscene;
        ScenePathControl path;
        ScenePathPairControl path_pair;
        uint32 words[3];
    } parameters;

    union
    {
        uint32 control_word;
        const AnimatedObject *animation_owner;
        SceneCutsceneControlTail cutscene_control;
    };

    AnimationTrackBinding animation_binding;
    uint32 position[3];
    sint16 rotation[4];

    void UpdateExtentsFromAnimation();
    uint32 PlaySound();
    uint32 StopSound();
};

typedef char SceneCutsceneControlSize[(sizeof(SceneCutsceneControl) == 12) ? 1 : -1];
typedef char SceneCutsceneControlTailSize[(sizeof(SceneCutsceneControlTail) == 4) ? 1 : -1];
typedef char SceneTriggerCutsceneTailOffset[(offsetof(SceneTrigger, cutscene_control) == 80) ? 1 : -1];
typedef char SceneTriggerBinarySpan[(sizeof(SceneTrigger) == 108) ? 1 : -1];
typedef char SceneTriggerWorldOffset[(offsetof(SceneTrigger, transform) + offsetof(MATRIX, t) == 36) ? 1 : -1];
typedef char SceneTriggerExtentOffset[(offsetof(SceneTrigger, extent) == 60) ? 1 : -1];
typedef char SceneTriggerParametersOffset[(offsetof(SceneTrigger, parameters) == 68) ? 1 : -1];
typedef char SceneTriggerControlWordOffset[(offsetof(SceneTrigger, control_word) == 80) ? 1 : -1];
typedef char SceneTriggerAnimationOffset[(offsetof(SceneTrigger, animation_binding) == 84) ? 1 : -1];
typedef char SceneTriggerPositionOffset[(offsetof(SceneTrigger, position) == 88) ? 1 : -1];
typedef char SceneTriggerRotationOffset[(offsetof(SceneTrigger, rotation) == 100) ? 1 : -1];

struct SceneEvents
{
    uint8 script_values[98];
    uint8 world_name_index;
    uint32 pending;
    uint8 count;
    uint32 entries[40];
    uint8 deferred_count;
    uint32 deferred_entries[16];
};

extern SceneEvents scene_events;

extern "C"
{
    uint32 EncodeObjectName(sint32 first, sint32 second);
    uint32 QueueSceneEvent(uint32 event, uint32 kind);
    uint32 ApplySceneEvents(uint32 *const *destinations, sint32 count);
    void UpdateSceneObjects(void);
    void UpdateSceneEventObjects(void);
}

void ProcessSceneTriggers(SceneNode *objects, SceneNode *auxiliary);

#endif
