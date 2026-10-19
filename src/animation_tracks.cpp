#include <stdio.h>
#include <stdlib.h>

#include "game.h"

static const uint32 key_reciprocal[257] = {0,    1048576, 524288, 349525, 262144, 209715, 174762, 149796, 131072, 116508, 104857, 95325, 87381, 80659, 74898, 69905, 65536, 61680, 58254, 55188, 52428, 49932, 47662, 45590, 43690, 41943, 40329, 38836, 37449, 36157, 34952, 33825, 32768, 31775, 30840, 29959, 29127, 28339, 27594, 26886, 26214, 25575, 24966, 24385, 23831, 23301, 22795, 22310, 21845, 21399, 20971, 20560, 20164, 19784, 19418, 19065, 18724, 18396, 18078, 17772, 17476, 17189, 16912, 16644, 16384, 16131, 15887, 15650, 15420, 15196, 14979, 14768, 14563, 14364, 14169, 13981, 13797, 13617, 13443, 13273, 13107, 12945, 12787, 12633, 12483, 12336, 12192, 12052, 11915, 11781, 11650, 11522, 11397, 11275, 11155, 11037, 10922, 10810, 10699, 10591, 10485, 10381, 10280, 10180, 10082, 9986, 9892, 9799, 9709, 9619, 9532, 9446, 9362, 9279, 9198, 9118, 9039, 8962, 8886, 8811, 8738, 8665, 8594, 8525, 8456, 8388, 8322, 8256, 8192,
                                           8128, 8065,    8004,   7943,   7884,   7825,   7767,   7710,   7653,   7598,   7543,   7489,  7436,  7384,  7332,  7281,  7231,  7182,  7133,  7084,  7037,  6990,  6944,  6898,  6853,  6808,  6765,  6721,  6678,  6636,  6594,  6553,  6512,  6472,  6432,  6393,  6355,  6316,  6278,  6241,  6204,  6168,  6132,  6096,  6061,  6026,  5991,  5957,  5924,  5890,  5857,  5825,  5793,  5761,  5729,  5698,  5667,  5637,  5607,  5577,  5548,  5518,  5489,  5461,  5433,  5405,  5377,  5349,  5322,  5295,  5269,  5242,  5216,  5190,  5165,  5140,  5115,  5090,  5065,  5041,  5017,  4993,  4969,  4946,  4922,  4899,  4877,  4854,  4832,  4809,  4788,  4766,  4744,  4723,  4702,  4681,  4660,  4639,  4619,  4599,  4578,  4559,  4539,  4519,  4500,  4481, 4462, 4443, 4424, 4405, 4387, 4369, 4350, 4332, 4315, 4297, 4279, 4262, 4245, 4228, 4211, 4194, 4177, 4161, 4144, 4128, 4112, 4096};

uint32 InterpolateKeyTime(uint32 frame, uint32 end, uint32 start_key, uint32 end_key, const uint8 *key_data)
{
    FUNCTION_MARKER(0x80017030, "SLUS_005.78");
    uint32 elapsed = (frame >> 20) - (start_key >> 20);
    uint32 duration = (end >> 20) - (start_key >> 20);
    uint32 reciprocal = duration < 0x101 ? key_reciprocal[duration] : 0x100000u / duration;
    uint32 left = start_key & 0xFF;
    uint32 right = end_key & 0xFF;
    uint32 value = elapsed << 12;
    uint32 amplitude, threshold, coefficient;
    const AnimationEaseCurve *left_curve, *right_curve;
    uint32 product, term;
    if ((left | right) == 0)
        goto finish;
    amplitude = ((start_key >> 8) & 0xFFF) + 2048;
    if (left != 0)
    {
        left_curve = (const AnimationEaseCurve *)(key_data - left * sizeof(AnimationEaseCurve));
        threshold = (uint32)left_curve->incoming_end * duration;
        if ((sint32)value < (sint32)threshold)
        {
            coefficient = left_curve->incoming_coefficient;
            goto ease_in;
        }
        if (right != 0)
        {
            right_curve = (const AnimationEaseCurve *)(key_data - right * sizeof(AnimationEaseCurve));
            if ((sint32)value >= (sint32)((uint32)right_curve->outgoing_start * duration))
                goto ease_out;
        }
        else if ((sint32)value >= (sint32)(duration << 12))
        {
            value = duration << 12;
            goto finish;
        }
        term = (uint32)(((sint64)(sint32)amplitude * (sint32)threshold) >> 12);
        value = (amplitude << 1) * elapsed - term;
        goto finish;
    }
    right_curve = (const AnimationEaseCurve *)(key_data - right * sizeof(AnimationEaseCurve));
    if ((sint32)value < (sint32)((uint32)right_curve->outgoing_start * duration))
    {
        value = (amplitude << 1) * elapsed;
        goto finish;
    }
ease_out:
    elapsed = duration - elapsed;
    coefficient = right_curve->outgoing_coefficient;
    product = amplitude * (elapsed * elapsed);
    term = (uint32)(((sint64)(sint32)product * (sint32)coefficient) >> 12);
    term = (uint32)(((sint64)(sint32)term * (sint32)reciprocal) >> 12);
    value = (duration << 12) - (uint32)((sint32)term >> 8);
    goto finish;
ease_in:
    product = amplitude * (elapsed * elapsed);
    term = (uint32)(((sint64)(sint32)product * (sint32)coefficient) >> 12);
    term = (uint32)(((sint64)(sint32)term * (sint32)reciprocal) >> 12);
    value = (uint32)((sint32)term >> 8);
finish:
    return (uint32)(((sint64)(sint32)value * (sint32)reciprocal) >> 12);
}

uint32 SampleVectorTrack(sint32 frame, const AnimationTrack *track, uint32 *destination)
{
    FUNCTION_MARKER(0x80017364, "SLUS_005.78");
    const VectorAnimationKey *keys = (const VectorAnimationKey *)track->keys;
    const VectorAnimationKey *previous = keys + track->key_count - 1;
    const VectorAnimationKey *next = keys + 1;
    uint32 frame_value = ((uint32)frame << 20) | 0xFFFFF;
    uint32 end_value = previous->time;
    uint32 value, square, cube;
    uint32 previous_weight, next_weight, previous_tangent_weight, next_tangent_weight;
    uint32 flags, result = 0, i;
    if (frame_value < end_value)
    {
        do
        {
            end_value = next->time;
            ++next;
        } while (frame_value >= end_value);
        --next;
        previous = next - 1;
    }
    else
    {
        --next;
        if (frame_value == (end_value | 0xFFFFF))
            goto copy_key;
        end_value = ((uint32)track->frame_limit << 20) | 0xFFFFF;
        if (frame_value >= end_value)
        {
            if ((track->flags & 3) == 3)
                previous = next;
            goto copy_key;
        }
    }
    value = InterpolateKeyTime(frame_value, end_value, previous->time, next->time, track->keys);
    square = (uint32)(((sint64)(sint32)value * (sint32)value) >> 12) >> 8;
    cube = (uint32)(((sint64)(sint32)square * (sint32)value) >> 12) >> 8;
    next_tangent_weight = (uint32)((sint32)(cube - square) >> 8);
    next_weight = (uint32)((sint32)(3 * square - 2 * cube) >> 8);
    previous_tangent_weight = (uint32)((sint32)(cube - square + value - square) >> 8);
    previous_weight = 4096u - next_weight;
    flags = track->flags;
    for (i = 0; i < 3; ++i)
    {
        uint32 first, second, tangent0, tangent1, sum;
        if ((flags & 0x20) != 0)
        {
            first = (uint32)(((sint64)(sint32)previous->position[i] * (sint32)previous_weight) >> 12);
            second = (uint32)(((sint64)(sint32)next->position[i] * (sint32)next_weight) >> 12);
            tangent0 = previous->outgoing_tangent[i] * previous_tangent_weight;
            tangent1 = next->incoming_tangent[i] * next_tangent_weight;
            result = (uint32)((sint32)(tangent0 + tangent1) >> 12);
            sum = first + second + result;
        }
        else if ((flags & 0x10) != 0)
        {
            first = previous->position[i] * previous_weight;
            second = next->position[i] * next_weight;
            tangent0 = previous->outgoing_tangent[i] * previous_tangent_weight;
            tangent1 = next->incoming_tangent[i] * next_tangent_weight;
            result = (uint32)((sint32)(first + second + tangent0 + tangent1) >> 12);
            sum = result;
        }
        else
        {
            first = (uint32)(((sint64)(sint32)previous->position[i] * (sint32)previous_weight) >> 12);
            second = (uint32)(((sint64)(sint32)next->position[i] * (sint32)next_weight) >> 12);
            tangent0 = (uint32)(((sint64)(sint32)previous->outgoing_tangent[i] * (sint32)previous_tangent_weight) >> 12);
            result = (uint32)(((sint64)(sint32)next->incoming_tangent[i] * (sint32)next_tangent_weight) >> 12);
            sum = first + second + tangent0 + result;
        }
        destination[i] = sum;
    }
    return result;
copy_key:
{
    uint32 x = previous->position[0];
    uint32 y = previous->position[1];
    uint32 z = previous->position[2];
    destination[0] = x;
    destination[1] = y;
    destination[2] = z;
    return x;
}
}

uint32 SampleQuaternionTrack(sint32 frame, const AnimationTrack *track, sint16 *destination)
{
    FUNCTION_MARKER(0x80017938, "SLUS_005.78");
    const QuaternionAnimationKey *keys = (const QuaternionAnimationKey *)track->keys;
    const QuaternionAnimationKey *next = keys;
    const QuaternionAnimationKey *previous = keys + track->key_count - 1;
    uint32 frame_value = ((uint32)frame << 20) | 0xFFFFF;
    uint32 end_value = previous->time;
    uint32 weight, inverse, result = 0, i;
    sint32 turns;
    if (frame_value < end_value)
    {
        ++next;
        do
        {
            end_value = next->time;
            ++next;
        } while (frame_value >= end_value);
        --next;
        previous = next - 1;
    }
    else
    {
        if (frame_value == (end_value | 0xFFFFF))
            goto copy_key;
        end_value = ((uint32)track->frame_limit << 20) | 0xFFFFF;
        if (frame_value >= end_value)
        {
            if ((track->flags & 3) == 3)
                previous = next;
            goto copy_key;
        }
    }
    weight = InterpolateKeyTime(frame_value, end_value, previous->time, next->time, track->keys);
    turns = next->turns;
    weight = (uint32)((sint32)weight >> 6);
    if (turns == 0)
    {
        inverse = 0x4000u - weight;
        for (i = 0; i < 4; ++i)
        {
            uint32 first = (uint32)previous->rotation.values[i] * inverse;
            uint32 second = (uint32)next->rotation.values[i] * weight;
            result = (uint32)((sint32)first >> 14) + (uint32)((sint32)second >> 14);
            destination[i] = result;
        }
        return result;
    }
    else
    {
        sint32 angle = next->angle;
        uint32 extra = (uint32)(turns - 1) << 27;
        uint32 product = weight * ((uint32)angle + (uint32)((sint32)extra >> 16));
        sint32 part = (sint32)product >> 14;
        sint32 divisor = SinQ14((sint16)angle);
        sint32 reciprocal;
        sint32 sine;
        uint32 source_weight, target_weight;
        if (divisor == 0)
            xport_mips_break(7);
        reciprocal = divisor == 0 ? -1 : 0x10000000 / divisor;
        sine = SinQ14((sint16)((uint32)angle - (uint32)part));
        source_weight = (uint32)(((sint64)sine * reciprocal) >> 14);
        sine = SinQ14((sint16)part);
        target_weight = (uint32)(((sint64)sine * reciprocal) >> 14);
        for (i = 0; i < 4; ++i)
        {
            uint32 first = (uint32)(((sint64)previous->rotation.values[i] * (sint32)source_weight) >> 14);
            uint32 second = (uint32)(((sint64)next->rotation.values[i] * (sint32)target_weight) >> 14);
            destination[i] = first + second;
        }
        if (reciprocal > 0x50000)
        {
            sint32 y = destination[1];
            sint32 z = destination[2];
            sint32 w = destination[3];
            sint32 x = destination[0];
            uint32 length = (uint32)(y * y) + (uint32)(z * z) + (uint32)(w * w) + (uint32)(x * x);
            length = (uint32)((sint32)length >> 14);
            if (length + 0x3FF0u < 0x7FE0u)
            {
                uint32 root = SquareRoot12((sint32)(length >> 2)) << 2;
                sint32 scale;
                uint32 px, py, pz, pw;
                if (root == 0)
                    xport_mips_break(7);
                scale = root == 0 ? -1 : 0x10000000 / (sint32)root;
                px = (uint32)destination[0] * (uint32)scale;
                py = (uint32)destination[1] * (uint32)scale;
                pz = (uint32)destination[2] * (uint32)scale;
                pw = (uint32)destination[3] * (uint32)scale;
                destination[0] = (sint32)px >> 14;
                destination[1] = (sint32)py >> 14;
                destination[2] = (sint32)pz >> 14;
                result = (uint32)((sint32)pw >> 14);
                destination[3] = result;
            }
        }
        return result;
    }
copy_key:
{
    uint32 first = previous->rotation.words[0];
    uint32 second = previous->rotation.words[1];
    xport_store_le32(destination, first);
    xport_store_le32(destination + 2, second);
    return first;
}
}

uint32 AnimationTrack::AdvanceVector(uint32 *vector, sint32 step, sint32 bias)
{
    FUNCTION_MARKER(0x8001A76C, "SLUS_005.78");
    uint32 sample[3];
    uint32 remaining = (uint32)bias;
    for (;;)
    {
        uint32 difference[3];
        sint32 length, dx, dy, dz;
        uint32 square;
        uint32 index;
        SampleVectorTrack(this->frame, this, sample);
        for (index = 0; index < 3; ++index)
            difference[index] = sample[index] - vector[index];
        dx = (sint32)difference[0];
        dy = (sint32)difference[1];
        dz = (sint32)difference[2];
        if ((uint32)dx + 26754 > 0xD103 || (uint32)dy + 26754 > 0xD103 || (uint32)dz + 26754 > 0xD103)
        {
            square = (uint32)(((sint64)dx * dx) >> 12);
            square += (uint32)(((sint64)dy * dy) >> 12);
            square += (uint32)(((sint64)dz * dz) >> 12);
        }
        else
        {
            square = (uint32)dx * (uint32)dx + (uint32)dy * (uint32)dy + (uint32)dz * (uint32)dz;
            square = (uint32)((sint32)square >> 12);
        }
        if ((sint32)square < 0)
            length = (sint32)(SquareRoot12((sint32)(square >> 2)) << 1);
        else
            length = (sint32)SquareRoot12((sint32)square);
        if ((sint32)remaining < length)
        {
            sint32 reciprocal;
            sint32 coefficient;
            if (length == 0)
                trap(7);
            reciprocal = 0x40000000 / length;
            coefficient = (sint32)(uint32)(((sint64)(sint32)remaining * reciprocal) >> 18);
            remaining = 0;
            fixed_math_result = coefficient;
            for (index = 0; index < 3; ++index)
                difference[index] = (uint32)(((sint64)(sint32)difference[index] * coefficient) >> 12);
            for (index = 0; index < 3; ++index)
                vector[index] = vector[index] + difference[index];
        }
        else
        {
            uint32 position[3];
            uint16 count;
            sint32 proposed;
            uint16 next;
            position[0] = sample[0];
            position[1] = sample[1];
            position[2] = sample[2];
            vector[0] = position[0];
            vector[1] = position[1];
            vector[2] = position[2];
            proposed = this->frame;
            count = (uint16)this->frame_limit;
            remaining -= (uint32)length;
            proposed = (sint32)((uint32)proposed + (uint32)step);
            next = (uint16)proposed;
            this->frame = next;
            if (next >= count)
            {
                if ((this->flags & 2) != 0)
                    this->frame = (sint16)next < 0 ? (uint32)count - 1 : 0;
                else if (count < next)
                {
                    this->frame = (uint32)proposed - (uint32)step;
                    break;
                }
            }
            if (this->frame != proposed && (this->flags & 3) != 3)
                SampleVectorTrack(this->frame, this, vector);
        }
        if (remaining == 0)
            break;
    }
    return remaining != (uint32)bias;
}

uint32 AnimationTrack::BlendRotation(const AnimationTrack *rotation_track, const uint32 *position, sint16 *quaternion, sint32 step) const
{
    FUNCTION_MARKER(0x8001AAA8, "SLUS_005.78");
    uint32 first[3], second[3];
    sint16 first_rotation[4], second_rotation[4];
    sint32 dx, dy, dz;
    uint32 square;
    uint32 difference[3];
    sint32 full_distance;
    sint32 current_distance;
    sint32 reciprocal;
    uint32 coefficient;
    uint32 blend;
    uint32 inverse;
    uint32 result = 0;
    uint32 index;
    if ((step == 1 && this->frame == 0) || (step == -1 && this->frame == this->frame_limit))
        return SampleQuaternionTrack(rotation_track->frame, rotation_track, quaternion);
    SampleVectorTrack(this->frame, this, first);
    SampleQuaternionTrack(rotation_track->frame, rotation_track, first_rotation);
    SampleVectorTrack((sint32)((uint32)(sint32)this->frame - (uint32)step), this, second);
    SampleQuaternionTrack((sint32)((uint32)(sint32)rotation_track->frame - (uint32)step), rotation_track, second_rotation);
    for (index = 0; index < 3; ++index)
        difference[index] = first[index] - second[index];
    dx = (sint32)difference[0];
    dy = (sint32)difference[1];
    dz = (sint32)difference[2];
    if ((uint32)dx + 26754 > 0xD103 || (uint32)dy + 26754 > 0xD103 || (uint32)dz + 26754 > 0xD103)
    {
        square = (uint32)(((sint64)dx * dx) >> 12);
        square += (uint32)(((sint64)dy * dy) >> 12);
        square += (uint32)(((sint64)dz * dz) >> 12);
    }
    else
    {
        square = (uint32)dx * (uint32)dx + (uint32)dy * (uint32)dy + (uint32)dz * (uint32)dz;
        square = (uint32)((sint32)square >> 12);
    }
    if ((sint32)square < 0)
        full_distance = (sint32)(SquareRoot12((sint32)(square >> 2)) << 1);
    else
        full_distance = (sint32)SquareRoot12((sint32)square);
    for (index = 0; index < 3; ++index)
        difference[index] = position[index] - second[index];
    dx = (sint32)difference[0];
    dy = (sint32)difference[1];
    dz = (sint32)difference[2];
    if ((uint32)dx + 26754 > 0xD103 || (uint32)dy + 26754 > 0xD103 || (uint32)dz + 26754 > 0xD103)
    {
        square = (uint32)(((sint64)dx * dx) >> 12);
        square += (uint32)(((sint64)dy * dy) >> 12);
        square += (uint32)(((sint64)dz * dz) >> 12);
    }
    else
    {
        square = (uint32)dx * (uint32)dx + (uint32)dy * (uint32)dy + (uint32)dz * (uint32)dz;
        square = (uint32)((sint32)square >> 12);
    }
    if ((sint32)square < 0)
        current_distance = (sint32)(SquareRoot12((sint32)(square >> 2)) << 1);
    else
        current_distance = (sint32)SquareRoot12((sint32)square);
    if (full_distance == 0)
        trap(7);
    reciprocal = 0x40000000 / full_distance;
    coefficient = (uint32)(((sint64)current_distance * reciprocal) >> 18);
    blend = coefficient * 4;
    inverse = 16384 - blend;
    for (index = 0; index < 4; ++index)
    {
        uint32 old_product = (uint32)(sint32)second_rotation[index] * inverse;
        uint32 new_product = (uint32)(sint32)first_rotation[index] * blend;
        if (index == 3)
            fixed_math_result = coefficient;
        result = (uint32)((sint32)old_product >> 14) + (uint32)((sint32)new_product >> 14);
        quaternion[index] = result;
    }
    return result;
}

uint32 AnimatedObject::AdvanceTracks(sint32 step)
{
    FUNCTION_MARKER(0x8001B334, "SLUS_005.78");
    AnimationHeader *animation;
    uint32 group, count, stride, first, flags, result;
    uint8 *track;
    uint32 changed;
    if ((this->flags & 0x30000) != 0)
        step = 0;
    animation = this->animation;
    changed = step == 0;
    if (animation != 0)
    {
        const AnimationHeader *header = animation;
        for (group = 0; group < 4; ++group)
        {
            count = (uint32)header->counts[group];
            if (count == 0)
                continue;
            stride = group == 0 ? sizeof(VectorAnimationTrack) : (group == 3 ? sizeof(MorphAnimationTrack) : sizeof(AnimationTrack));
            track = header->tracks[group].bytes != 0 ? header->tracks[group].bytes : (uint8 *)0;
            if (group < 2)
                track += count * stride - stride;
            first = 1;
            do
            {
                AnimationTrack *current_track = (AnimationTrack *)track;
                if (first != 0 && group < 2 && (current_track->flags & 4) != 0 && step != 0)
                {
                    if (group == 0)
                    {
                        if (current_track->frame == 0 && (sint32)velocity[0] < 0)
                            changed = 1;
                        else if ((current_track->AdvanceVector(position, step, (sint32)velocity[0]) & 0xFF) != 0)
                            changed = 1;
                    }
                    else
                    {
                        current_track->frame = (uint16)((VectorAnimationTrack *)(header->tracks[0].vectors))->track.frame;
                        changed = 1;
                        ((VectorAnimationTrack *)(header->tracks[0].vectors))->track.BlendRotation(current_track, position, orientation, step);
                    }
                    goto next_track;
                }
                if (step == 0)
                {
                    flags = this->flags;
                    if ((flags & 0x30000) != 0)
                    {
                        if ((flags & 0x20000) != 0)
                            current_track->frame = 0;
                        else if ((current_track->flags & 2) != 0)
                            current_track->frame = (uint16)current_track->frame_limit - 1;
                        else
                            current_track->frame = (uint16)current_track->frame_limit;
                    }
                    else if ((current_track->flags & 2) != 0)
                    {
                        sint32 initial = this->frame;
                        sint32 limit = current_track->frame_limit;
                        sint32 remainder;
                        if (limit == 0)
                        {
                            xport_mips_break(7);
                            remainder = initial;
                        }
                        else
                            remainder = initial % limit;
                        current_track->frame = (uint32)remainder;
                    }
                    else
                    {
                        sint32 initial = this->frame;
                        sint32 limit = current_track->frame_limit;
                        current_track->frame = (uint32)(initial < limit ? initial : limit);
                    }
                }
                else
                {
                    uint32 old = (uint16)current_track->frame;
                    uint32 limit = (uint16)current_track->frame_limit;
                    uint32 sum = old + (uint32)step;
                    uint32 frame = sum & 0xFFFF;
                    current_track->frame = sum;
                    if (frame >= limit)
                    {
                        if ((current_track->flags & 2) != 0)
                            current_track->frame = (sum & 0x8000) != 0 ? limit - 1 : 0;
                        else if (limit < frame)
                        {
                            current_track->frame = sum - (uint32)step;
                            goto next_track;
                        }
                    }
                    changed = 1;
                }
                current_track->flags = current_track->flags | 8;
            next_track:
                first = 0;
                --count;
                if (count != 0)
                {
                    if (group < 2)
                        track -= stride;
                    else
                        track += stride;
                }
            } while (count != 0);
        }
    }
    flags = this->flags;
    if (changed != 0)
        flags = (flags & 0xFFFCFFFF) | 1;
    else
        flags &= 0xFFFBFFFF;
    this->flags = flags;
    result = node.flags | 0x40;
    node.flags = result;
    return result;
}

uint32 BlendRotationTransition(sint32 frame, const AnimationTrack *track, sint16 *destination)
{
    const uint8 *payload;
    const sint16 *source;
    uint32 weight;
    uint32 source_weight;
    uint32 destination_weight;
    uint32 sum;
    uint32 product_low;
    uint32 first;
    uint32 second;
    uint32 result = 0;
    uint32 index;
    sint32 current;
    sint32 next;
    sint32 target;
    sint32 dot;
    sint32 angle;
    sint32 part;
    sint32 divisor;
    sint32 reciprocal;
    sint64 product;
    uint32 spherical;
    FUNCTION_MARKER(0x80019924, "SLUS_005.78");
    payload = track->keys;
    weight = 0x4000 - 5461 * ((uint32)frame + 1);
    if (payload == 0)
    {
        fprintf(stderr, "P2: Rotation transition has no selected key data\n");
        abort();
    }
    source = payload == (const uint8 *)-1 ? (const sint16 *)track : (const sint16 *)(payload + 8);
    current = destination[1];
    target = source[1];
    sum = (uint32)(current * target);
    current = destination[2];
    target = source[2];
    sum += (uint32)(current * target);
    current = destination[3];
    target = source[3];
    sum += (uint32)(current * target);
    current = destination[0];
    target = source[0];
    sum += (uint32)(current * target);
    dot = (sint32)sum >> 14;
    spherical = (uint32)dot + 16368 < 0x7FE0;
    if (spherical != 0)
    {
        angle = AcosAngle(dot < 0 ? -dot : dot, 0x4000);
        product_low = (uint32)((sint64)(sint32)weight * (sint16)angle);
        part = (sint32)product_low >> 14;
        divisor = SinQ14((sint16)angle);
        if (divisor == 0)
            xport_mips_break(7);
        reciprocal = divisor == 0 ? -1 : 0x10000000 / divisor;
        target = SinQ14((sint16)((uint32)angle - (uint32)part));
        product = (sint64)target * reciprocal;
        destination_weight = (uint32)((uint64)product >> 14);
        target = SinQ14((sint16)part);
        product = (sint64)target * reciprocal;
        source_weight = (uint32)((uint64)product >> 14);
        if (dot < 0)
            source_weight = 0 - source_weight;
    }
    else
    {
        destination_weight = 0x4000 - weight;
        source_weight = dot < 0 ? 0 - weight : weight;
    }
    current = destination[0];
    for (index = 0; index < 4; ++index)
    {
        product = (sint64)current * (sint32)destination_weight;
        first = spherical != 0 ? (uint32)((uint64)product >> 14) : (uint32)((sint32)(uint32)product >> 14);
        target = source[index];
        product = (sint64)target * (sint32)source_weight;
        second = spherical != 0 ? (uint32)((uint64)product >> 14) : (uint32)((sint32)(uint32)product >> 14);
        if (index < 3)
            next = destination[index + 1];
        result = first + second;
        destination[index] = result;
        if (index < 3)
            current = next;
    }
    return spherical != 0 ? second : result;
}
