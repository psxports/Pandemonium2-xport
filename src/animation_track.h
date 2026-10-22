#ifndef ANIMATION_TRACK_H
#define ANIMATION_TRACK_H
#include <stddef.h>

#include "psx.h"
#include "quaternion16.h"
#include "vector3.h"

struct AnimatedObject;
struct MorphAnimationTrack;
struct MorphSpan;

struct VectorAnimationKey;
struct QuaternionAnimationKey;
struct AnimationEaseCurve;
struct EncodedAnimationTrack;
struct EncodedVectorAnimationTrack;
struct EncodedMorphAnimationTrack;
struct AnimationTrack;
struct VectorAnimationTrack;
struct MorphAnimationKey;
struct MorphAnimationTrack;
union AnimationTrackBinding;

struct VectorAnimationKey
{
    uint32 time;
    Vec3 pos;
    Vec3 outgoing_tangent;
    Vec3 incoming_tangent;
};

typedef char VectorAnimationKeySize[(sizeof(VectorAnimationKey) == 40) ? 1 : -1];

typedef char VectorAnimationKeyPositionOffset[(offsetof(VectorAnimationKey, pos) == 4) ? 1 : -1];

struct QuaternionAnimationKey
{
    uint32 time;
    sint16 angle;
    sint16 turns;

    Quat rotation;
};

struct AnimationEaseCurve
{
    sint16 outgoing_start;
    sint16 incoming_end;
    sint32 outgoing_coefficient;
    sint32 incoming_coefficient;
};

typedef char QuaternionAnimationKeySize[(sizeof(QuaternionAnimationKey) == 16) ? 1 : -1];

typedef char QuaternionAnimationKeyRotationOffset[(offsetof(QuaternionAnimationKey, rotation) == 8) ? 1 : -1];

typedef char AnimationEaseCurveSize[(sizeof(AnimationEaseCurve) == 12) ? 1 : -1];

struct EncodedAnimationTrack
{
    uint8 key_count;
    uint8 resource_byte1;
    uint8 flags;
    uint8 node_key;
    sint16 frame_limit;
    uint16 frame_count;
    sint16 frame;
    uint16 resource_halfword10;
    uint32 keys_offset;
};

struct EncodedVectorAnimationTrack
{
    EncodedAnimationTrack track;
    uint32 resource_word16;
};

struct EncodedMorphAnimationTrack
{
    EncodedAnimationTrack track;
    uint32 vertices_offset;
    uint16 resource_halfword20;
    sint16 vertex_stride;
};

typedef char EncodedAnimationTrackSize[(sizeof(EncodedAnimationTrack) == 16) ? 1 : -1];

typedef char EncodedAnimationTrackKeysOffset[(offsetof(EncodedAnimationTrack, keys_offset) == 12) ? 1 : -1];

typedef char EncodedVectorAnimationTrackSize[(sizeof(EncodedVectorAnimationTrack) == 20) ? 1 : -1];

typedef char EncodedMorphAnimationTrackSize[(sizeof(EncodedMorphAnimationTrack) == 24) ? 1 : -1];

typedef char EncodedMorphAnimationTrackDataOffset[(offsetof(EncodedMorphAnimationTrack, vertices_offset) == 16) ? 1 : -1];

struct AnimationTrack
{
    union
    {
        struct
        {
            uint8 key_count;
            uint8 resource_byte1;
            uint8 flags;
            uint8 node_key;
            sint16 frame_limit;
            sint16 frame_count;
            sint16 frame;
            uint16 resource_halfword10;
        };

        Vec3 constant;
        Quat constant_quat;
    };

    const uint8 *keys;

    static uint32 InterpolateKeyTime(uint32 frame, uint32 end, uint32 start_key, uint32 end_key, const uint8 *key_data);
    static void SampleVectorTrack(uint32 frame, const AnimationTrack *track, Vec3 &dst);
    static void SampleQuatTrack(uint32 frame, const AnimationTrack *track, Quat &dst);
    static void SampleMorphTrack(uint32 frame, const MorphAnimationTrack *track, const AnimatedObject *owner, const MorphSpan *span);
    static void BlendRotationTransition(sint16 frame, const AnimationTrack *track, Quat &dst);
    uint32 AdvanceVector(Vec3 &vector, sint32 step, sint32 bias);
    void BlendRotation(const AnimationTrack *rotation_track, const Vec3 &pos, Quat &quat, sint32 step) const;
};

typedef char AnimationTrackFrameOffset[(offsetof(AnimationTrack, frame) == 8) ? 1 : -1];

// Inline constants share the mutable twelve-byte state prefix
typedef char AnimationTrackStateEnd[(offsetof(AnimationTrack, resource_halfword10) + sizeof(uint16) == 12) ? 1 : -1];

// Vector records retain the original trailing word
struct VectorAnimationTrack
{
    AnimationTrack track;
    uint32 resource_word16;
};

typedef char VectorAnimationTrackPrefixOffset[(offsetof(VectorAnimationTrack, track) == 0) ? 1 : -1];

struct MorphAnimationKey
{
    uint32 time;
    uint32 frame_idx;
};

typedef char MorphAnimationKeySize[(sizeof(MorphAnimationKey) == 8) ? 1 : -1];

struct MorphAnimationTrack
{
    AnimationTrack track;
    const SVECTOR *vertices;
    uint16 resource_halfword20;
    sint16 vertex_stride;
};

typedef char MorphAnimationTrackPrefixOffset[(offsetof(MorphAnimationTrack, track) == 0) ? 1 : -1];

union AnimationTrackBinding
{
    uint32 word;
    sint8 tracks[4];
};

#endif
