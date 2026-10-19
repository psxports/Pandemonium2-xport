#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "game.h"

void AnimationHeader::Relocate(uint8 *base, uint32 bytes, const EncodedAnimationHeader *input)
{
    FUNCTION_MARKER(0x80068294, "SLUS_005.78");
    uint32 remaining, offset;
    const uint8 *pointer;
    if (input == 0 || base == 0 || bytes < sizeof(EncodedAnimationHeader))
    {
        fprintf(stderr, "P2: Animation header exceeds resource buffer\n");
        abort();
    }
    EncodedAnimationHeader encoded = *input;
    for (uint32 group = 0; group < 4; ++group)
    {
        counts[group] = encoded.counts[group];
        tracks[group].bytes = 0;
    }
    frame_metadata = encoded.frame_metadata;
    frame_count = encoded.frame_count;
    for (uint32 group = 0; group < 3; ++group)
    {
        if (counts[group] < 0)
        {
            fprintf(stderr, "P2: Negative animation group count\n");
            abort();
        }
        remaining = (uint32)counts[group];
        if (remaining == 0)
            continue;
        uint32 stride = group == 0 ? sizeof(VectorAnimationTrack) : sizeof(AnimationTrack);
        uint32 encoded_stride = group == 0 ? sizeof(EncodedVectorAnimationTrack) : sizeof(EncodedAnimationTrack);
        uint32 key_stride = group == 1 ? sizeof(QuaternionAnimationKey) : sizeof(VectorAnimationKey);
        offset = encoded.track_offsets[group];
        if ((offset & 3) != 0 || offset > bytes || remaining > (bytes - offset) / encoded_stride)
        {
            fprintf(stderr, "P2: Animation group exceeds resource buffer\n");
            abort();
        }
        pointer = base + offset;
        tracks[group].bytes = (uint8 *)calloc(remaining, stride);
        if (tracks[group].bytes == 0)
        {
            fprintf(stderr, "P2: Cannot allocate native animation tracks\n");
            abort();
        }
        uint8 *destination = tracks[group].bytes;
        do
        {
            AnimationTrack *track = (AnimationTrack *)destination;
            memcpy(track, pointer, sizeof(((EncodedAnimationTrack *)0)->state));
            if (group == 0)
                ((VectorAnimationTrack *)track)->metadata16 = xport_load_le32(pointer + 16);
            offset = xport_load_le32(pointer + 12);
            if (offset != 0 && offset != 0xFFFFFFFF)
            {
                uint32 key_bytes = track->key_count != 0 ? track->key_count * key_stride : 1;
                if ((offset & 3) != 0 || offset > bytes || key_bytes > bytes - offset)
                {
                    fprintf(stderr, "P2: Animation keys exceed resource buffer\n");
                    abort();
                }
                uint32 curve_count = 0;
                for (uint32 key_index = 0; key_index < track->key_count; ++key_index)
                {
                    uint32 curve = xport_load_le32(base + offset + key_index * key_stride) & 0xFF;
                    if (curve > curve_count)
                        curve_count = curve;
                }
                if (curve_count * sizeof(AnimationEaseCurve) > offset)
                {
                    fprintf(stderr, "P2: Animation easing prefix exceeds resource buffer\n");
                    abort();
                }
                track->keys = base + offset;
            }
            else
                track->keys = offset != 0 ? (const uint8 *)-1 : 0;
            --remaining;
            pointer += encoded_stride;
            destination += stride;
        } while (remaining != 0);
    }
    offset = encoded.track_offsets[3];
    if (offset != 0)
    {
        if (counts[3] < 0)
        {
            fprintf(stderr, "P2: Negative morph animation group count\n");
            abort();
        }
        remaining = (uint32)counts[3];
        if (remaining != 0)
        {
            if ((offset & 3) != 0 || offset > bytes || remaining > (bytes - offset) / sizeof(EncodedMorphAnimationTrack))
            {
                fprintf(stderr, "P2: Morph animation group exceeds resource buffer\n");
                abort();
            }
            pointer = base + offset;
            tracks[3].morphs = (MorphAnimationTrack *)calloc(remaining, sizeof(MorphAnimationTrack));
            if (tracks[3].morphs == 0)
            {
                fprintf(stderr, "P2: Cannot allocate native morph animation tracks\n");
                abort();
            }
            MorphAnimationTrack *track = tracks[3].morphs;
            do
            {
                memcpy(&track->track, pointer, sizeof(((EncodedAnimationTrack *)0)->state));
                track->metadata20 = xport_load_le16(pointer + 20);
                track->vertex_stride = (sint16)xport_load_le16(pointer + 22);
                offset = xport_load_le32(pointer + 12);
                if (offset != 0)
                {
                    uint32 key_bytes = track->track.key_count != 0 ? track->track.key_count * sizeof(MorphAnimationKey) : 1;
                    if ((offset & 3) != 0 || offset > bytes || key_bytes > bytes - offset)
                    {
                        fprintf(stderr, "P2: Morph animation keys exceed resource buffer\n");
                        abort();
                    }
                    uint32 curve_count = 0;
                    for (uint32 key_index = 0; key_index < track->track.key_count; ++key_index)
                    {
                        uint32 curve = xport_load_le32(base + offset + key_index * sizeof(MorphAnimationKey)) & 0xFF;
                        if (curve > curve_count)
                            curve_count = curve;
                    }
                    if (curve_count * sizeof(AnimationEaseCurve) > offset)
                    {
                        fprintf(stderr, "P2: Morph animation easing prefix exceeds resource buffer\n");
                        abort();
                    }
                    track->track.keys = base + offset;
                }
                else
                    track->track.keys = 0;
                offset = xport_load_le32(pointer + 16);
                if (offset != 0)
                {
                    if ((offset & 1) != 0 || offset > bytes || sizeof(SVECTOR) > bytes - offset)
                    {
                        fprintf(stderr, "P2: Morph vertex data exceeds resource buffer\n");
                        abort();
                    }
                    track->vertices = (const SVECTOR *)(base + offset);
                }
                --remaining;
                pointer += sizeof(EncodedMorphAnimationTrack);
                ++track;
            } while (remaining != 0);
        }
    }
}

void AnimatedObject::AttachAnimation(uint8 *animation_data, uint32 bytes, AnimationHeader *header)
{
    FUNCTION_MARKER(0x8006A170, "SLUS_005.78");
    if (header == 0 || animation_data == 0)
    {
        fprintf(stderr, "P2: Animation attachment has no resource buffer\n");
        abort();
    }
    header->Relocate(animation_data, bytes, (const EncodedAnimationHeader *)animation_data);
    this->animation = header;
    this->unbound_animation_word = 0;
}

void LoadAnimationTable(AnimatedObject *object, uint8 *source, uint32 source_bytes, AnimationHeader **destination, uint32 destination_count, AnimationHeader *header_storage, MorphRestorationHeader **restoration_owner)
{
    uint32 remaining;
    const uint8 *record;
    uint32 offset;
    MorphRestorationHeader *restoration = 0;
    MorphRestorationEntry *entry;
    uint32 declared_count;
    sint16 restoration_count;
    FUNCTION_MARKER(0x80069FD8, "SLUS_005.78");
    if (restoration_owner == 0)
    {
        fprintf(stderr, "P2: Animation restoration owner is missing\n");
        abort();
    }
    if (source == 0 || source_bytes < 4)
    {
        fprintf(stderr, "P2: Truncated animation table header\n");
        abort();
    }
    const uint8 *table_bytes = source;
    declared_count = xport_load_le32(table_bytes);
    if (declared_count == 0 || declared_count > (source_bytes - 4) / 4 || declared_count - 1 > destination_count || (declared_count > 1 && (destination == NULL || header_storage == NULL)))
    {
        fprintf(stderr, "P2: Invalid animation table extent\n");
        abort();
    }
    remaining = declared_count - 1;
    record = table_bytes + 4;
    while (remaining != 0)
    {
        offset = xport_load_le32(record);
        if (offset != 0)
        {
            if ((offset & 3) != 0 || offset > source_bytes || sizeof(EncodedAnimationHeader) > source_bytes - offset)
            {
                fprintf(stderr, "P2: Animation header is outside table resource\n");
                abort();
            }
            AnimationHeader *header = header_storage + (declared_count - 1 - remaining);
            header->Relocate(source + offset, source_bytes - offset, (const EncodedAnimationHeader *)(source + offset));
            record += 4;
            *destination = header;
            ++destination;
        }
        else
        {
            *destination = 0;
            ++destination;
            record += 4;
        }
        --remaining;
    }
    offset = xport_load_le32(record);
    if (offset != 0)
    {
        if (offset > source_bytes || source_bytes - offset < sizeof(EncodedMorphRestorationHeader))
        {
            fprintf(stderr, "P2: Truncated morph restoration header\n");
            abort();
        }
        const uint8 *encoded = source + offset;
        restoration_count = (sint16)xport_load_le16(encoded);
        if (restoration_count < 0 || (uint32)restoration_count > (source_bytes - offset - sizeof(EncodedMorphRestorationHeader)) / sizeof(EncodedMorphRestorationEntry) || (uint32)restoration_count > (0xFFFFFFFFu - sizeof(MorphRestorationHeader)) / sizeof(MorphRestorationEntry))
        {
            fprintf(stderr, "P2: Invalid morph restoration extent\n");
            abort();
        }
        uint32 allocation_bytes = sizeof(MorphRestorationHeader) + (uint32)restoration_count * sizeof(MorphRestorationEntry);
        restoration = (MorphRestorationHeader *)malloc(allocation_bytes);
        if (restoration == 0)
        {
            fprintf(stderr, "P2: Cannot allocate native morph restoration state\n");
            abort();
        }
        restoration->count = restoration_count;
        restoration->active_count = xport_load_le16(encoded + 2);
        restoration->entries = restoration_count != 0 ? (MorphRestorationEntry *)(restoration + 1) : 0;
        remaining = (uint32)restoration_count;
        encoded += sizeof(EncodedMorphRestorationHeader);
        entry = restoration->entries;
        while (remaining != 0)
        {
            uint32 vertex_offset = xport_load_le32(encoded + 4);
            if ((vertex_offset & 1) != 0 || vertex_offset > source_bytes || sizeof(SVECTOR) > source_bytes - vertex_offset)
            {
                fprintf(stderr, "P2: Morph restoration vertices exceed animation bank\n");
                abort();
            }
            entry->flags = xport_load_le16(encoded);
            entry->node_key = xport_load_le16(encoded + 2);
            entry->vertices = (const SVECTOR *)(source + vertex_offset);
            ++entry;
            encoded += sizeof(EncodedMorphRestorationEntry);
            --remaining;
        }
    }
    *restoration_owner = restoration;
    if (object != 0)
        object->morph_restoration = restoration;
}

uint32 AnimatedObject::ClampAnimationFrame()
{
    uint32 flags;
    uint32 result;
    uint8 byte_flags;
    sint32 frame;
    sint32 count;
    FUNCTION_MARKER(0x8001A6AC, "SLUS_005.78");
    flags = this->flags;
    if ((flags & 0x10000) != 0)
    {
        frame = this->frame;
        if (frame == 2 || frame == (((AnimationHeader *)(this->animation)))->frame_count)
        {
            flags = this->flags;
            this->frame = 0;
            this->flags = (flags & 0xFFFEFFFF) | 0x20000;
            goto mark_changed;
        }
    }
    frame = this->frame;
    if (frame < 0)
        this->frame = 0;
    else
    {
        count = (((AnimationHeader *)(this->animation)))->frame_count;
        if (frame >= count)
            this->frame = (uint16)(count - 1);
    }
mark_changed:
    result = this->flags;
    byte_flags = node.flags;
    result |= 1;
    this->flags = result;
    node.flags = byte_flags | 0x40;
    return result;
}
