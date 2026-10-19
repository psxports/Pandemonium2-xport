#ifndef ANIMATION_DATA_H
#define ANIMATION_DATA_H

#include <stddef.h>

#include "psx.h"

struct VectorAnimationKey
{
    uint32 time;
    uint32 position[3];
    uint32 outgoing_tangent[3];
    uint32 incoming_tangent[3];
};

typedef char VectorAnimationKeySize[(sizeof(VectorAnimationKey) == 40) ? 1 : -1];
typedef char VectorAnimationKeyPositionOffset[(offsetof(VectorAnimationKey, position) == 4) ? 1 : -1];

struct QuaternionAnimationKey
{
    uint32 time;
    sint16 angle;
    sint16 turns;

    union
    {
        sint16 values[4];
        uint32 words[2];
    } rotation;
};

struct AnimationEaseCurve
{
    sint16 outgoing_start;
    sint16 incoming_end;
    uint32 outgoing_coefficient;
    uint32 incoming_coefficient;
};

typedef char QuaternionAnimationKeySize[(sizeof(QuaternionAnimationKey) == 16) ? 1 : -1];
typedef char QuaternionAnimationKeyRotationOffset[(offsetof(QuaternionAnimationKey, rotation) == 8) ? 1 : -1];
typedef char AnimationEaseCurveSize[(sizeof(AnimationEaseCurve) == 12) ? 1 : -1];

struct EncodedAnimationTrack
{
    uint8 state[12];
    uint32 keys_offset;
};

struct EncodedVectorAnimationTrack
{
    EncodedAnimationTrack track;
    uint32 metadata16;
};

struct EncodedMorphAnimationTrack
{
    EncodedAnimationTrack track;
    uint32 vertices_offset;
    uint16 metadata20;
    sint16 vertex_stride;
};

typedef char EncodedAnimationTrackSize[(sizeof(EncodedAnimationTrack) == 16) ? 1 : -1];
typedef char EncodedAnimationTrackKeysOffset[(offsetof(EncodedAnimationTrack, keys_offset) == 12) ? 1 : -1];
typedef char EncodedVectorAnimationTrackSize[(sizeof(EncodedVectorAnimationTrack) == 20) ? 1 : -1];
typedef char EncodedMorphAnimationTrackSize[(sizeof(EncodedMorphAnimationTrack) == 24) ? 1 : -1];
typedef char EncodedMorphAnimationTrackDataOffset[(offsetof(EncodedMorphAnimationTrack, vertices_offset) == 16) ? 1 : -1];

struct AnimationTrack
{
    uint8 key_count;
    uint8 metadata1;
    uint8 flags;
    uint8 metadata3;
    sint16 frame_limit;
    uint16 metadata6;
    sint16 frame;
    uint16 metadata10;
    const uint8 *keys;

    uint32 AdvanceVector(uint32 *vector, sint32 step, sint32 bias);
    uint32 BlendRotation(const AnimationTrack *rotation_track, const uint32 *position, sint16 *quaternion, sint32 step) const;
};

typedef char AnimationTrackFrameOffset[(offsetof(AnimationTrack, frame) == 8) ? 1 : -1];
// Inline constants share the mutable twelve-byte state prefix
typedef char AnimationTrackStateEnd[(offsetof(AnimationTrack, metadata10) + sizeof(uint16) == 12) ? 1 : -1];

// Vector records retain the original trailing word
struct VectorAnimationTrack
{
    AnimationTrack track;
    uint32 metadata16;
};

typedef char VectorAnimationTrackPrefixOffset[(offsetof(VectorAnimationTrack, track) == 0) ? 1 : -1];

struct MorphAnimationKey
{
    uint32 time;
    uint32 frame_index;
};

typedef char MorphAnimationKeySize[(sizeof(MorphAnimationKey) == 8) ? 1 : -1];

struct MorphAnimationTrack
{
    AnimationTrack track;
    const SVECTOR *vertices;
    uint16 metadata20;
    sint16 vertex_stride;
};

typedef char MorphAnimationTrackPrefixOffset[(offsetof(MorphAnimationTrack, track) == 0) ? 1 : -1];

struct EncodedMorphRestorationHeader
{
    sint16 count;
    uint16 active_count;
};

struct EncodedMorphRestorationEntry
{
    uint16 flags;
    uint16 node_key;
    uint32 vertices_offset;
};

typedef char EncodedMorphRestorationHeaderSize[(sizeof(EncodedMorphRestorationHeader) == 4) ? 1 : -1];
typedef char EncodedMorphRestorationEntrySize[(sizeof(EncodedMorphRestorationEntry) == 8) ? 1 : -1];

struct MorphRestorationEntry
{
    uint16 flags;
    uint16 node_key;
    const SVECTOR *vertices;
};

struct MorphRestorationHeader
{
    sint16 count;
    uint16 active_count;
    MorphRestorationEntry *entries;
};

union AnimationTrackBinding
{
    uint32 word;
    sint8 tracks[4];
};

// Typed views of decoded native animation groups
union AnimationTrackGroup
{
    uint8 *bytes;
    VectorAnimationTrack *vectors;
    AnimationTrack *records;
    MorphAnimationTrack *morphs;
};

struct EncodedAnimationHeader
{
    sint8 counts[4];
    uint16 frame_metadata;
    sint16 frame_count;
    uint32 track_offsets[4];
};

typedef char EncodedAnimationHeaderSize[(sizeof(EncodedAnimationHeader) == 24) ? 1 : -1];
typedef char EncodedAnimationGroupsOffset[(offsetof(EncodedAnimationHeader, track_offsets) == 8) ? 1 : -1];

typedef struct AnimationHeader AnimationHeader;

struct AnimationHeader
{
    sint8 counts[4];
    uint16 frame_metadata;
    sint16 frame_count;
    AnimationTrackGroup tracks[4];

    void Relocate(uint8 *base, uint32 bytes, const EncodedAnimationHeader *input);
};

#ifdef __cplusplus
extern "C"
{
#endif

    uint32 BlendRotationTransition(sint32 frame, const AnimationTrack *track, sint16 *destination);

    uint32 SampleVectorTrack(sint32 frame, const AnimationTrack *track, uint32 *destination);
    uint32 SampleQuaternionTrack(sint32 frame, const AnimationTrack *track, sint16 *destination);

    struct AnimatedObject;
    void LoadAnimationTable(AnimatedObject *object, uint8 *source, uint32 source_bytes, AnimationHeader **destination, uint32 destination_count, AnimationHeader *header_storage, MorphRestorationHeader **restoration_owner);

#ifdef __cplusplus
}
#endif

#endif
