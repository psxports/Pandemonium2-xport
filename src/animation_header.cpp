#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "animation_header.h"
#include "game.h"

void AnimationHeader::Relocate(uint8 *base, uint32 bytes, const EncodedAnimationHeader *input)
{
    FUNCTION_MARKER(0x80068294, "SLUS_005.78");
    uint32 remaining, offset;
    const uint8 *pointer;
    EncodedAnimationHeader encoded = *input;
    for (uint32 grp = 0; grp < 4; ++grp)
    {
        counts[grp] = encoded.counts[grp];
        tracks[grp].bytes = 0;
    }
    frame_metadata = encoded.frame_metadata;
    frame_count = encoded.frame_count;
    for (uint32 grp = 0; grp < 3; ++grp)
    {
        remaining = counts[grp];
        if (remaining == 0)
            continue;
        uint32 stride = grp == 0 ? sizeof(VectorAnimationTrack) : sizeof(AnimationTrack);
        uint32 encoded_stride = grp == 0 ? sizeof(EncodedVectorAnimationTrack) : sizeof(EncodedAnimationTrack);

        offset = encoded.track_offsets[grp];
        pointer = base + offset;
        tracks[grp].bytes = (uint8 *)calloc(remaining, stride);
        uint8 *dst = tracks[grp].bytes;
        do
        {
            AnimationTrack *track = (AnimationTrack *)dst;
            const EncodedAnimationTrack *src = (const EncodedAnimationTrack *)pointer;
            track->key_count = src->key_count;
            track->resource_byte1 = src->resource_byte1;
            track->flags = src->flags;
            track->node_key = src->node_key;
            track->frame_limit = src->frame_limit;
            track->frame_count = src->frame_count;
            track->frame = src->frame;
            track->resource_halfword10 = src->resource_halfword10;
            if (grp == 0)
                ((VectorAnimationTrack *)track)->resource_word16 = ((const EncodedVectorAnimationTrack *)pointer)->resource_word16;
            offset = src->keys_offset;
            if (offset != 0 && offset != 0xFFFFFFFF)
            {
                track->keys = base + offset;
            }
            else
                track->keys = offset != 0 ? (const uint8 *)-1 : 0;
            --remaining;
            pointer += encoded_stride;
            dst += stride;
        } while (remaining != 0);
    }
    offset = encoded.track_offsets[3];
    if (offset != 0)
    {
        remaining = counts[3];
        if (remaining != 0)
        {
            pointer = base + offset;
            tracks[3].morphs = (MorphAnimationTrack *)calloc(remaining, sizeof(MorphAnimationTrack));
            MorphAnimationTrack *track = tracks[3].morphs;
            do
            {
                const EncodedMorphAnimationTrack *src = (const EncodedMorphAnimationTrack *)pointer;
                track->track.key_count = src->track.key_count;
                track->track.resource_byte1 = src->track.resource_byte1;
                track->track.flags = src->track.flags;
                track->track.node_key = src->track.node_key;
                track->track.frame_limit = src->track.frame_limit;
                track->track.frame_count = src->track.frame_count;
                track->track.frame = src->track.frame;
                track->track.resource_halfword10 = src->track.resource_halfword10;
                track->resource_halfword20 = src->resource_halfword20;
                track->vertex_stride = src->vertex_stride;
                offset = src->track.keys_offset;
                if (offset != 0)
                {
                    track->track.keys = base + offset;
                }
                else
                    track->track.keys = 0;
                offset = src->vertices_offset;
                if (offset != 0)
                {
                    track->vertices = (const SVECTOR *)(base + offset);
                }
                --remaining;
                pointer += sizeof(EncodedMorphAnimationTrack);
                ++track;
            } while (remaining != 0);
        }
    }
}

void AnimationHeader::LoadAnimationTable(AnimatedObject *object, uint8 *src, uint32 src_bytes, AnimationHeader **dst, uint32 dst_count, AnimationHeader *header_storage, MorphRestorationHeader **restoration_owner)
{
    uint32 remaining;
    const EncodedAnimationTableEntry *rec;
    uint32 offset;
    MorphRestorationHeader *restoration = 0;
    MorphRestorationEntry *entry;
    uint32 declared_count;
    sint16 restoration_count;
    FUNCTION_MARKER(0x80069FD8, "SLUS_005.78");
    const EncodedAnimationTableHeader *table = (const EncodedAnimationTableHeader *)src;
    declared_count = table->count;
    remaining = declared_count - 1;
    rec = (const EncodedAnimationTableEntry *)(table + 1);
    while (remaining != 0)
    {
        offset = rec->offset;
        if (offset != 0)
        {
            AnimationHeader *header = header_storage + (declared_count - 1 - remaining);
            header->Relocate(src + offset, src_bytes - offset, (const EncodedAnimationHeader *)(src + offset));
            ++rec;
            *dst = header;
            ++dst;
        }
        else
        {
            *dst = 0;
            ++dst;
            ++rec;
        }
        --remaining;
    }
    offset = rec->offset;
    if (offset != 0)
    {
        const EncodedMorphRestorationHeader *encoded_header = (const EncodedMorphRestorationHeader *)(src + offset);
        restoration_count = encoded_header->count;
        uint32 allocation_bytes = sizeof(MorphRestorationHeader) + (uint32)restoration_count * sizeof(MorphRestorationEntry);
        restoration = (MorphRestorationHeader *)malloc(allocation_bytes);
        restoration->count = restoration_count;
        restoration->active_count = encoded_header->active_count;
        restoration->entries = restoration_count != 0 ? (MorphRestorationEntry *)(restoration + 1) : 0;
        remaining = restoration_count;
        const EncodedMorphRestorationEntry *encoded = (const EncodedMorphRestorationEntry *)(encoded_header + 1);
        entry = restoration->entries;
        while (remaining != 0)
        {
            uint32 vertex_offset = encoded->vertices_offset;
            entry->flags = encoded->flags;
            entry->node_key = encoded->node_key;
            entry->vertices = (const SVECTOR *)(src + vertex_offset);
            ++entry;
            ++encoded;
            --remaining;
        }
    }
    *restoration_owner = restoration;
    if (object != 0)
        object->morph_restoration = restoration;
}
