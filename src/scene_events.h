#ifndef SCENE_EVENTS_H
#define SCENE_EVENTS_H
#include <stddef.h>

#include "psx.h"
#include "animation_track.h"
#include "quaternion16.h"
#include "scene_node.h"
#include "vector3.h"

struct ScenePath;

struct SceneSoundControl;
struct SceneCutsceneControl;
struct SceneCutsceneControlTail;
struct ScenePathControl;
struct ScenePathPairControl;
struct SceneEventControl;
struct SceneExitControl;
struct SceneTimedControl;
struct SceneMotionControl;
struct SceneJumpControl;
union SceneTriggerParams;
struct EncodedTriggerPath;
struct EncodedTriggerPathPair;
struct EncodedTimedControl;
union EncodedTriggerParams;
struct EncodedSceneTrigger;
struct SceneEvents;

struct SceneSoundControl
{
    uint32 sample_id;
    uint32 flags;

    uint8 voice;
    uint8 pan;
    uint16 volume_flags;
};

typedef char SceneSoundControlSize[(sizeof(SceneSoundControl) == 12) ? 1 : -1];

typedef char SceneSoundControlVoiceOffset[(offsetof(SceneSoundControl, voice) == 8) ? 1 : -1];

struct SceneCutsceneControl
{
    uint32 camera_id;
    uint32 next_camera;
    uint32 duration;
};

struct SceneCutsceneControlTail
{
    uint16 flags;
    sint16 idx;
};

struct ScenePathControl
{
    const ScenePath *target;
    uint32 frame_word;
    uint32 duration_ticks;
};

struct ScenePathPairControl
{
    const ScenePath *targets[2];
    uint32 tangent_dir;
};

struct SceneEventControl
{
    uint32 command;
    uint32 schedule;
};

struct SceneExitControl
{
    SceneObjectName target;
    uint32 flags;
    uint32 swap;
};

struct SceneTimedControl
{
    uint32 deadline;
    uint32 flags;
};

struct SceneMotionControl
{
    sint32 horizontal_accel;
    sint32 vertical_accel;
};

struct SceneJumpControl
{
    sint32 motion_scale;
    sint32 rise_scale;
};

union SceneTriggerParams
{
    SceneSoundControl sound;
    SceneCutsceneControl cutscene;
    ScenePathControl path;
    ScenePathPairControl path_pair;
    SceneEventControl event;
    SceneExitControl exit;
    SceneTimedControl reaction;
    SceneMotionControl motion;
    SceneJumpControl jump;
    SceneTimedControl damage;
    uint32 section_id;
    uint32 impulse_strength;
    uint32 level;
    uint32 far_distance;
    uint32 trail_effect;
    uint32 form;
    uint32 state_flags;
    uint32 animation;
};

struct EncodedTriggerPath
{
    uint32 target_offset;
    uint32 frame_word;
    uint32 duration_ticks;
};

struct EncodedTriggerPathPair
{
    uint32 target_offsets[2];
    uint32 tangent_dir;
};

struct EncodedTimedControl
{
    uint32 deadline;
    uint32 reserved4;
    uint32 flags;
};

union EncodedTriggerParams
{
    SceneSoundControl sound;
    SceneCutsceneControl cutscene;
    EncodedTriggerPath path;
    EncodedTriggerPathPair path_pair;
    SceneEventControl event;
    SceneExitControl exit;
    EncodedTimedControl reaction;
    SceneMotionControl motion;
    SceneJumpControl jump;
    EncodedTimedControl damage;
    uint32 section_id;
    uint32 impulse_strength;
    uint32 level;
    uint32 far_distance;
    uint32 trail_effect;
    uint32 form;
    uint32 state_flags;
    uint32 animation;
};

struct EncodedSceneTrigger
{
    EncodedSceneNode node;
    MATRIX transform;
    Vec3 delta;
    uint16 extent[3];
    uint16 radius;
    EncodedTriggerParams parameters;
    uint32 control_word;
    AnimationTrackBinding animation_binding;
    Vec3 pos;
    Quat rotation;
};

typedef char EncodedTriggerParamsSize[(sizeof(EncodedTriggerParams) == 12) ? 1 : -1];

typedef char EncodedSceneTriggerSize[(sizeof(EncodedSceneTrigger) == 108) ? 1 : -1];

typedef char SceneCutsceneControlSize[(sizeof(SceneCutsceneControl) == 12) ? 1 : -1];

typedef char SceneCutsceneControlTailSize[(sizeof(SceneCutsceneControlTail) == 4) ? 1 : -1];

struct SceneEvents
{
    uint8 script_vals[98];
    uint8 world_name_idx;
    uint32 pending;
    sint8 count;
    uint32 entries[40];
    uint8 deferred_count;
    uint32 deferred_entries[16];

    static void QueueSceneEvent(uint32 event, uint32 kind);
    static uint32 EncodeObjectName(sint32 char0, sint32 char1);
    static uint32 ApplySceneEvents(uint32 *const *destinations, sint32 count);
    static void UpdateSceneEventObjects(void);
};

extern SceneEvents scene_events;

#endif
