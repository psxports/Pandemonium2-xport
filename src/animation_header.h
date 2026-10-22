#ifndef ANIMATION_HEADER_H
#define ANIMATION_HEADER_H
#include <stddef.h>

#include "psx.h"

struct AnimatedObject;
struct MorphRestorationHeader;

struct AnimatedObject;
struct AnimationTrack;
struct MorphAnimationTrack;
struct VectorAnimationTrack;

struct EncodedAnimationTableHeader;
struct EncodedAnimationTableEntry;
struct EncodedMorphRestorationHeader;
struct EncodedMorphRestorationEntry;
struct MorphRestorationEntry;
struct MorphRestorationHeader;
union AnimationTrackGroup;
struct EncodedAnimationHeader;
struct AnimationHeader;

struct EncodedAnimationTableHeader
{
    uint32 count;
};

struct EncodedAnimationTableEntry
{
    uint32 offset;
};

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

// Typed views of decoded native animation groups
union AnimationTrackGroup
{
    uint8 *bytes;
    VectorAnimationTrack *vectors;
    AnimationTrack *recs;
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
    static void LoadAnimationTable(AnimatedObject *object, uint8 *src, uint32 src_bytes, AnimationHeader **dst, uint32 dst_count, AnimationHeader *header_storage, MorphRestorationHeader **restoration_owner);
};

#endif
