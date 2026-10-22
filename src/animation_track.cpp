#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "animation_track.h"
#include "game.h"
#include "morph_blend_context.h"
#include "psx_gte.h"

static const uint32 key_rcp[257] = {0,    1048576, 524288, 349525, 262144, 209715, 174762, 149796, 131072, 116508, 104857, 95325, 87381, 80659, 74898, 69905, 65536, 61680, 58254, 55188, 52428, 49932, 47662, 45590, 43690, 41943, 40329, 38836, 37449, 36157, 34952, 33825, 32768, 31775, 30840, 29959, 29127, 28339, 27594, 26886, 26214, 25575, 24966, 24385, 23831, 23301, 22795, 22310, 21845, 21399, 20971, 20560, 20164, 19784, 19418, 19065, 18724, 18396, 18078, 17772, 17476, 17189, 16912, 16644, 16384, 16131, 15887, 15650, 15420, 15196, 14979, 14768, 14563, 14364, 14169, 13981, 13797, 13617, 13443, 13273, 13107, 12945, 12787, 12633, 12483, 12336, 12192, 12052, 11915, 11781, 11650, 11522, 11397, 11275, 11155, 11037, 10922, 10810, 10699, 10591, 10485, 10381, 10280, 10180, 10082, 9986, 9892, 9799, 9709, 9619, 9532, 9446, 9362, 9279, 9198, 9118, 9039, 8962, 8886, 8811, 8738, 8665, 8594, 8525, 8456, 8388, 8322, 8256, 8192,
                                    8128, 8065,    8004,   7943,   7884,   7825,   7767,   7710,   7653,   7598,   7543,   7489,  7436,  7384,  7332,  7281,  7231,  7182,  7133,  7084,  7037,  6990,  6944,  6898,  6853,  6808,  6765,  6721,  6678,  6636,  6594,  6553,  6512,  6472,  6432,  6393,  6355,  6316,  6278,  6241,  6204,  6168,  6132,  6096,  6061,  6026,  5991,  5957,  5924,  5890,  5857,  5825,  5793,  5761,  5729,  5698,  5667,  5637,  5607,  5577,  5548,  5518,  5489,  5461,  5433,  5405,  5377,  5349,  5322,  5295,  5269,  5242,  5216,  5190,  5165,  5140,  5115,  5090,  5065,  5041,  5017,  4993,  4969,  4946,  4922,  4899,  4877,  4854,  4832,  4809,  4788,  4766,  4744,  4723,  4702,  4681,  4660,  4639,  4619,  4599,  4578,  4559,  4539,  4519,  4500,  4481, 4462, 4443, 4424, 4405, 4387, 4369, 4350, 4332, 4315, 4297, 4279, 4262, 4245, 4228, 4211, 4194, 4177, 4161, 4144, 4128, 4112, 4096};

uint32 AnimationTrack::InterpolateKeyTime(uint32 frame, uint32 end, uint32 start_key, uint32 end_key, const uint8 *key_data)
{
    FUNCTION_MARKER(0x80017030, "SLUS_005.78");
    uint32 elapsed = (frame >> 20) - (start_key >> 20);
    uint32 duration = (end >> 20) - (start_key >> 20);
    sint32 rcp = duration < 0x101 ? key_rcp[duration] : 0x100000u / duration;
    uint8 left = start_key & 0xFF;
    uint8 right = end_key & 0xFF;
    sint32 val = elapsed << 12;
    sint32 amplitude, coefficient;
    sint32 threshold;
    const AnimationEaseCurve *left_curve, *right_curve;
    sint32 product;
    sint32 term;
    if ((left | right) == 0)
        goto finish;
    amplitude = ((start_key >> 8) & 0xFFF) + 2048;
    if (left != 0)
    {
        left_curve = (const AnimationEaseCurve *)(key_data - left * sizeof(AnimationEaseCurve));
        threshold = left_curve->incoming_end * duration;
        if (val < threshold)
        {
            coefficient = left_curve->incoming_coefficient;
            goto ease_in;
        }
        if (right != 0)
        {
            right_curve = (const AnimationEaseCurve *)(key_data - right * sizeof(AnimationEaseCurve));
            if (val >= (sint32)(right_curve->outgoing_start * duration))
                goto ease_out;
        }
        else if (val >= (sint32)(duration << 12))
        {
            val = duration << 12;
            goto finish;
        }
        term = mul12(amplitude, threshold);
        val = (amplitude << 1) * elapsed - term;
        goto finish;
    }
    right_curve = (const AnimationEaseCurve *)(key_data - right * sizeof(AnimationEaseCurve));
    if (val < (sint32)(right_curve->outgoing_start * duration))
    {
        val = (amplitude << 1) * elapsed;
        goto finish;
    }
ease_out:
    elapsed = duration - elapsed;
    coefficient = right_curve->outgoing_coefficient;
    product = amplitude * (elapsed * elapsed);
    term = mul12(product, coefficient);
    term = mul12(term, rcp);
    val = (duration << 12) - (term >> 8);
    goto finish;
ease_in:
    product = amplitude * (elapsed * elapsed);
    term = mul12(product, coefficient);
    term = mul12(term, rcp);
    val = term >> 8;
finish:
    return mul12(val, rcp);
}

void AnimationTrack::SampleVectorTrack(uint32 frame, const AnimationTrack *track, Vec3 &dst)
{

    FUNCTION_MARKER(0x80017364, "SLUS_005.78");
    const VectorAnimationKey *keys = (const VectorAnimationKey *)track->keys;
    const VectorAnimationKey *prev = keys + track->key_count - 1;
    const VectorAnimationKey *next = keys + 1;
    uint32 frame_val = (frame << 20) | 0xFFFFF;
    uint32 end_val = prev->time;
    sint32 val;
    uint32 sq;
    uint32 cube;
    sint32 prev_weight;
    sint32 next_weight;
    sint32 prev_tangent_weight;
    sint32 next_tangent_weight;
    uint32 flags;
    if (frame_val < end_val)
    {
        do
        {
            end_val = next->time;
            ++next;
        } while (frame_val >= end_val);
        --next;
        prev = next - 1;
    }
    else
    {
        --next;
        if (frame_val == (end_val | 0xFFFFF))
            goto copy_key;
        end_val = ((uint32)track->frame_limit << 20) | 0xFFFFF;
        if (frame_val >= end_val)
        {
            if ((track->flags & 3) == 3)
                prev = next;
            goto copy_key;
        }
    }
    val = AnimationTrack::InterpolateKeyTime(frame_val, end_val, prev->time, next->time, track->keys);
    sq = (uint32)mul12(val, val) >> 8;
    cube = (uint32)mul12((sint32)sq, val) >> 8;
    next_tangent_weight = (sint32)(cube - sq) >> 8;
    next_weight = (sint32)(3 * sq - 2 * cube) >> 8;
    prev_tangent_weight = (sint32)(cube - sq + val - sq) >> 8;
    prev_weight = 4096 - next_weight;
    flags = track->flags;
    if ((flags & 0x20) != 0)
    {
        dst.x = mul12(prev->pos.x, prev_weight)
            + mul12(next->pos.x, next_weight)
            + ((sint32)((uint32)prev->outgoing_tangent.x * prev_tangent_weight
                + (uint32)next->incoming_tangent.x * next_tangent_weight) >> 12);
        dst.y = mul12(prev->pos.y, prev_weight)
            + mul12(next->pos.y, next_weight)
            + ((sint32)((uint32)prev->outgoing_tangent.y * prev_tangent_weight
                + (uint32)next->incoming_tangent.y * next_tangent_weight) >> 12);
        dst.z = mul12(prev->pos.z, prev_weight)
            + mul12(next->pos.z, next_weight)
            + ((sint32)((uint32)prev->outgoing_tangent.z * prev_tangent_weight
                + (uint32)next->incoming_tangent.z * next_tangent_weight) >> 12);
    }
    else if ((flags & 0x10) != 0)
    {
        dst.x = (sint32)((uint32)prev->pos.x * prev_weight
            + (uint32)next->pos.x * next_weight
            + (uint32)prev->outgoing_tangent.x * prev_tangent_weight
            + (uint32)next->incoming_tangent.x * next_tangent_weight) >> 12;
        dst.y = (sint32)((uint32)prev->pos.y * prev_weight
            + (uint32)next->pos.y * next_weight
            + (uint32)prev->outgoing_tangent.y * prev_tangent_weight
            + (uint32)next->incoming_tangent.y * next_tangent_weight) >> 12;
        dst.z = (sint32)((uint32)prev->pos.z * prev_weight
            + (uint32)next->pos.z * next_weight
            + (uint32)prev->outgoing_tangent.z * prev_tangent_weight
            + (uint32)next->incoming_tangent.z * next_tangent_weight) >> 12;
    }
    else
    {
        dst.x = mul12(prev->pos.x, prev_weight)
            + mul12(next->pos.x, next_weight)
            + mul12(prev->outgoing_tangent.x, prev_tangent_weight)
            + mul12(next->incoming_tangent.x, next_tangent_weight);
        dst.y = mul12(prev->pos.y, prev_weight)
            + mul12(next->pos.y, next_weight)
            + mul12(prev->outgoing_tangent.y, prev_tangent_weight)
            + mul12(next->incoming_tangent.y, next_tangent_weight);
        dst.z = mul12(prev->pos.z, prev_weight)
            + mul12(next->pos.z, next_weight)
            + mul12(prev->outgoing_tangent.z, prev_tangent_weight)
            + mul12(next->incoming_tangent.z, next_tangent_weight);
    }
    return;
copy_key:
    dst = prev->pos;
}

void AnimationTrack::SampleQuatTrack(uint32 frame, const AnimationTrack *track, Quat &dst)
{

    FUNCTION_MARKER(0x80017938, "SLUS_005.78");
    const QuaternionAnimationKey *keys = (const QuaternionAnimationKey *)track->keys;
    const QuaternionAnimationKey *next = keys;
    const QuaternionAnimationKey *prev = keys + track->key_count - 1;
    uint32 frame_val = (frame << 20) | 0xFFFFF;
    uint32 end_val = prev->time;
    sint32 weight, inverse;

    sint32 turns;
    if (frame_val < end_val)
    {
        ++next;
        do
        {
            end_val = next->time;
            ++next;
        } while (frame_val >= end_val);
        --next;
        prev = next - 1;
    }
    else
    {
        if (frame_val == (end_val | 0xFFFFF))
            goto copy_key;
        end_val = ((uint32)track->frame_limit << 20) | 0xFFFFF;
        if (frame_val >= end_val)
        {
            if ((track->flags & 3) == 3)
                prev = next;
            goto copy_key;
        }
    }
    weight = AnimationTrack::InterpolateKeyTime(frame_val, end_val, prev->time, next->time, track->keys);
    turns = next->turns;
    weight >>= 6;
    if (turns == 0)
    {
        inverse = 0x4000 - weight;
        dst.w = mul14(prev->rotation.w, inverse) + mul14(next->rotation.w, weight);
        dst.x = mul14(prev->rotation.x, inverse) + mul14(next->rotation.x, weight);
        dst.y = mul14(prev->rotation.y, inverse) + mul14(next->rotation.y, weight);
        dst.z = mul14(prev->rotation.z, inverse) + mul14(next->rotation.z, weight);
        return;
    }
    else
    {
        sint32 angle = next->angle;
        sint32 extra = (uint32)(turns - 1) << 27;
        sint32 product = (sint64)weight * (angle + (extra >> 16));
        sint32 part = product >> 14;
        sint32 divisor = SinQ14(angle);
        sint32 rcp;
        sint32 s;
        sint32 src_weight;
        sint32 target_weight;
        if (divisor == 0)
            xport_mips_break(7);
        rcp = 0x10000000 / divisor;
        s = SinQ14(angle - part);
        src_weight = mul14(s, rcp);
        s = SinQ14(part);
        target_weight = mul14(s, rcp);
        dst.w = mul14(prev->rotation.w, src_weight) + mul14(next->rotation.w, target_weight);
        dst.x = mul14(prev->rotation.x, src_weight) + mul14(next->rotation.x, target_weight);
        dst.y = mul14(prev->rotation.y, src_weight) + mul14(next->rotation.y, target_weight);
        dst.z = mul14(prev->rotation.z, src_weight) + mul14(next->rotation.z, target_weight);
        if (rcp > 0x50000)
        {
            uint32 len = (uint32)(dst.x * dst.x) + dst.y * dst.y + dst.z * dst.z + dst.w * dst.w;
            len = (sint32)len >> 14;
            if (len + 0x3FF0u < 0x7FE0u)
            {
                sint32 root = SquareRoot12((len >> 2)) << 2;
                sint32 scale;
                if (root == 0)
                    xport_mips_break(7);
                scale = 0x10000000 / root;
                dst.w = mul14(dst.w, scale);
                dst.x = mul14(dst.x, scale);
                dst.y = mul14(dst.y, scale);
                dst.z = mul14(dst.z, scale);
            }
        }
        return;
    }
copy_key:
    dst = prev->rotation;
}

void AnimationTrack::SampleMorphTrack(uint32 frame, const MorphAnimationTrack *track, const AnimatedObject *owner, const MorphSpan *span)
{
    FUNCTION_MARKER(0x80019684, "SLUS_005.78");
    MorphBlendContext context;
    const MorphAnimationKey *keys, *prev, *next;
    uint32 frame_val, end_val;
    const SVECTOR *src;
    MeshVertex *dst;
    uint32 remaining;
    if ((owner->flags & 0x200) != 0 && (track->track.flags & 1) != 0)
    {
        return;
    }
    context.owner = owner;
    context.span = span;
    context.dst = owner->vertices + (uint32)span->first_vertex;
    keys = (const MorphAnimationKey *)track->track.keys;
    next = keys;
    prev = keys + track->track.key_count - 1;
    end_val = prev->time;
    frame_val = (frame << 20) | 0xFFFFF;
    ++next;
    if (frame_val < end_val)
    {
        do
        {
            end_val = next->time;
            ++next;
        } while (frame_val >= end_val);
        --next;
        prev = next - 1;
        goto interpolate;
    }
    --next;
    if (frame_val == (end_val | 0xFFFFF))
    {
        uint32 stride = track->vertex_stride;
        uint32 idx = prev->frame_idx;
        dst = context.dst;
        remaining = span->vertex_count;
        src = (const SVECTOR *)((const uint8 *)track->vertices + ((idx * stride) << 3));
        goto copy_vertices;
    }
    end_val = ((uint32)track->track.frame_limit << 20) | 0xFFFFF;
    if (frame_val < end_val)
    {
        goto interpolate;
    }

    {
        const MorphAnimationKey *key = ((track->track.flags & 3) == 3) ? next : prev;
        uint32 stride = track->vertex_stride;
        uint32 idx = key->frame_idx;
        src = (const SVECTOR *)((const uint8 *)track->vertices + ((idx * stride) << 3));
    }
    context.source0 = src;
    span = context.span;
    owner = context.owner;
    dst = context.dst;
    src = context.source0;
    remaining = span->vertex_count;
copy_vertices:
    do
    {
        dst->n.vz = 0;
        dst->n.vy = 0;
        dst->n.vx = 0;
        dst->pos = *src;
        ++src;
        ++dst;
        --remaining;
    } while (remaining != 0);
    owner->RebuildMorphNormals(span);
    return;
interpolate:
{
    const uint8 *key_data = track->track.keys;
    sint32 val = AnimationTrack::InterpolateKeyTime(frame_val, end_val, prev->time, next->time, key_data);
    uint32 stride, idx;
    context.weight = 4096u - (val >> 8);
    stride = track->vertex_stride;
    idx = prev->frame_idx;
    context.source0 = (const SVECTOR *)((const uint8 *)track->vertices + ((idx * stride) << 3));
    stride = track->vertex_stride;
    idx = next->frame_idx;
    context.source1 = (const SVECTOR *)((const uint8 *)track->vertices + ((idx * stride) << 3));
    context.Apply();
}
}

void AnimationTrack::BlendRotationTransition(sint16 frame, const AnimationTrack *track, Quat &dst)
{
    const uint8 *payload;
    const Quat *src;
    sint32 weight;
    sint32 src_weight;
    sint32 dst_weight;
    uint32 sum;
    sint32 product_low;
    uint32 dst_term;
    uint32 src_term;


    sint32 cur;
    sint32 next;
    sint32 target;
    sint32 dot;
    sint16 angle;
    sint32 part;
    sint32 divisor;
    sint32 rcp;
    uint32 spherical;
    FUNCTION_MARKER(0x80019924, "SLUS_005.78");
    payload = track->keys;
    weight = 0x4000 - 5461 * (frame + 1);
    src = payload == (const uint8 *)-1 ? &track->constant_quat : &((const QuaternionAnimationKey *)payload)->rotation;
    sum = (uint32)(dst.x * src->x) + dst.y * src->y + dst.z * src->z + dst.w * src->w;
    dot = (sint32)sum >> 14;
    spherical = (uint32)dot + 16368 < 0x7FE0;
    if (spherical != 0)
    {
        angle = AcosAngle(dot < 0 ? -dot : dot, 0x4000);
        product_low = (sint64)weight * angle;
        part = product_low >> 14;
        divisor = SinQ14(angle);
        if (divisor == 0)
            xport_mips_break(7);
        rcp = 0x10000000 / divisor;
        target = SinQ14(angle - part);
        dst_weight = mul14(target, rcp);
        target = SinQ14(part);
        src_weight = mul14(target, rcp);
        if (dot < 0)
            src_weight = -src_weight;
    }
    else
    {
        dst_weight = 0x4000 - weight;
        src_weight = dot < 0 ? -weight : weight;
    }
    cur = dst.w;
    dst_term = spherical != 0 ? mul14(cur, dst_weight) : (sint32)((sint64)cur * dst_weight) >> 14;
    target = src->w;
    src_term = spherical != 0 ? mul14(target, src_weight) : (sint32)((sint64)target * src_weight) >> 14;
    next = dst.x;
    dst.w = dst_term + src_term;
    cur = next;
    dst_term = spherical != 0 ? mul14(cur, dst_weight) : (sint32)((sint64)cur * dst_weight) >> 14;
    target = src->x;
    src_term = spherical != 0 ? mul14(target, src_weight) : (sint32)((sint64)target * src_weight) >> 14;
    next = dst.y;
    dst.x = dst_term + src_term;
    cur = next;
    dst_term = spherical != 0 ? mul14(cur, dst_weight) : (sint32)((sint64)cur * dst_weight) >> 14;
    target = src->y;
    src_term = spherical != 0 ? mul14(target, src_weight) : (sint32)((sint64)target * src_weight) >> 14;
    next = dst.z;
    dst.y = dst_term + src_term;
    cur = next;
    dst_term = spherical != 0 ? mul14(cur, dst_weight) : (sint32)((sint64)cur * dst_weight) >> 14;
    target = src->z;
    src_term = spherical != 0 ? mul14(target, src_weight) : (sint32)((sint64)target * src_weight) >> 14;

    dst.z = dst_term + src_term;
}

uint32 AnimationTrack::AdvanceVector(Vec3 &vector, sint32 step, sint32 bias)
{
    FUNCTION_MARKER(0x8001A76C, "SLUS_005.78");
    Vec3 sample;
    uint32 remaining = bias;
    for (;;)
    {
        Vec3 difference;
        sint32 len, dx, dy, dz;
        uint32 sq;

        AnimationTrack::SampleVectorTrack(frame, this, sample);
        difference.x = sample.x - (uint32)vector.x;
        difference.y = sample.y - (uint32)vector.y;
        difference.z = sample.z - (uint32)vector.z;
        dx = difference.x;
        dy = difference.y;
        dz = difference.z;
        if ((uint32)dx + 26754 > 0xD103 || (uint32)dy + 26754 > 0xD103 || (uint32)dz + 26754 > 0xD103)
        {
            sq = mul12(dx, dx);
            sq += (uint32)mul12(dy, dy);
            sq += (uint32)mul12(dz, dz);
        }
        else
        {
            sq = (uint32)dx * dx + dy * (uint32)dy + dz * (uint32)dz;
            sq = (sint32)sq >> 12;
        }
        if ((sint32)sq < 0)
            len = SquareRoot12((sq >> 2)) << 1;
        else
            len = SquareRoot12(sq);
        if ((sint32)remaining < len)
        {
            sint32 rcp;
            sint32 coefficient;
            if (len == 0)
                trap(7);
            rcp = 0x40000000 / len;
            coefficient = ((sint64)(sint32)remaining * rcp) >> 18;
            remaining = 0;
            fixed_math_result = coefficient;
            difference.x = mul12(difference.x, coefficient);
            difference.y = mul12(difference.y, coefficient);
            difference.z = mul12(difference.z, coefficient);
            vector.x = vector.x + (uint32)difference.x;
            vector.y = vector.y + (uint32)difference.y;
            vector.z = vector.z + (uint32)difference.z;
        }
        else
        {
            uint16 count;
            sint32 proposed;
            uint16 next;
            vector = sample;
            proposed = frame;
            count = frame_limit;
            remaining -= (uint32)len;
            proposed = (uint32)proposed + step;
            next = proposed;
            frame = next;
            if (next >= count)
            {
                if ((flags & 2) != 0)
                    frame = (sint16)next < 0 ? (uint32)count - 1 : 0;
                else if (count < next)
                {
                    frame = (uint32)proposed - step;
                    break;
                }
            }
            if (frame != proposed && (flags & 3) != 3)
                AnimationTrack::SampleVectorTrack(frame, this, vector);
        }
        if (remaining == 0)
            break;
    }
    return remaining != bias;
}

void AnimationTrack::BlendRotation(const AnimationTrack *rotation_track, const Vec3 &pos, Quat &quat, sint32 step) const
{

    FUNCTION_MARKER(0x8001AAA8, "SLUS_005.78");
    Vec3 cur_pos, prev_pos;
    Quat cur_quat, prev_quat;
    sint32 dx, dy, dz;
    uint32 sq;
    Vec3 difference;
    sint32 full_distance;
    sint32 cur_distance;
    sint32 rcp;
    uint32 coefficient;
    uint32 blend;
    uint32 inverse;


    if ((step == 1 && frame == 0) || (step == -1 && frame == frame_limit))
    {
        AnimationTrack::SampleQuatTrack(rotation_track->frame, rotation_track, quat);
        return;
    }
    AnimationTrack::SampleVectorTrack(frame, this, cur_pos);
    AnimationTrack::SampleQuatTrack(rotation_track->frame, rotation_track, cur_quat);
    AnimationTrack::SampleVectorTrack(((uint32)frame - step), this, prev_pos);
    AnimationTrack::SampleQuatTrack(((uint32)rotation_track->frame - step), rotation_track, prev_quat);
    difference.x = cur_pos.x - (uint32)prev_pos.x;
    difference.y = cur_pos.y - (uint32)prev_pos.y;
    difference.z = cur_pos.z - (uint32)prev_pos.z;
    dx = difference.x;
    dy = difference.y;
    dz = difference.z;
    if ((uint32)dx + 26754 > 0xD103 || (uint32)dy + 26754 > 0xD103 || (uint32)dz + 26754 > 0xD103)
    {
        sq = mul12(dx, dx);
        sq += (uint32)mul12(dy, dy);
        sq += (uint32)mul12(dz, dz);
    }
    else
    {
        sq = (uint32)dx * dx + dy * (uint32)dy + dz * (uint32)dz;
        sq = (sint32)sq >> 12;
    }
    if ((sint32)sq < 0)
        full_distance = SquareRoot12((sq >> 2)) << 1;
    else
        full_distance = SquareRoot12(sq);
    difference.x = pos.x - (uint32)prev_pos.x;
    difference.y = pos.y - (uint32)prev_pos.y;
    difference.z = pos.z - (uint32)prev_pos.z;
    dx = difference.x;
    dy = difference.y;
    dz = difference.z;
    if ((uint32)dx + 26754 > 0xD103 || (uint32)dy + 26754 > 0xD103 || (uint32)dz + 26754 > 0xD103)
    {
        sq = mul12(dx, dx);
        sq += (uint32)mul12(dy, dy);
        sq += (uint32)mul12(dz, dz);
    }
    else
    {
        sq = (uint32)dx * dx + dy * (uint32)dy + dz * (uint32)dz;
        sq = (sint32)sq >> 12;
    }
    if ((sint32)sq < 0)
        cur_distance = SquareRoot12((sq >> 2)) << 1;
    else
        cur_distance = SquareRoot12(sq);
    if (full_distance == 0)
        trap(7);
    rcp = 0x40000000 / full_distance;
    coefficient = ((sint64)cur_distance * rcp) >> 18;
    blend = coefficient * 4;
    inverse = 16384 - blend;
    quat.w = mul14(prev_quat.w, inverse) + mul14(cur_quat.w, blend);
    quat.x = mul14(prev_quat.x, inverse) + mul14(cur_quat.x, blend);
    quat.y = mul14(prev_quat.y, inverse) + mul14(cur_quat.y, blend);
    fixed_math_result = coefficient;
    quat.z = mul14(prev_quat.z, inverse) + mul14(cur_quat.z, blend);
}
