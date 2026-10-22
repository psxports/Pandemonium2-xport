#include <string.h>

#include "path_definition.h"
#include "game.h"

Vec3 path_heading_delta = {0};

void PathDefinition::SampleTangent(VECTOR *dst, sint32 dir) const
{
    FUNCTION_MARKER(0x80022EC4, "SLUS_005.78");
    VECTOR vector;
    const VectorAnimationKey *key0, *key1;
    MATRIX *matrix;
    uint32 idx;
    sint32 x, y, z;
    if (dir != 0)
    {
        idx = pos_track.key_count;
        key0 = (const VectorAnimationKey *)pos_track.keys + idx - 1;
        key1 = (const VectorAnimationKey *)pos_track.keys + idx - 2;
    }
    else
    {
        key0 = (const VectorAnimationKey *)pos_track.keys;
        key1 = (const VectorAnimationKey *)pos_track.keys + 1;
    }
    vector.vx = key0->pos.x - (uint32)key1->pos.x;
    vector.vy = key0->pos.y - (uint32)key1->pos.y;
    vector.vz = key0->pos.z - (uint32)key1->pos.z;
    while ((uint32)vector.vx + 26754 > 0xD103 || (uint32)vector.vy + 26754 > 0xD103 || (uint32)vector.vz + 26754 > 0xD103)
    {
        x = vector.vx >> 1;
        y = vector.vy >> 1;
        z = vector.vz >> 1;
        vector.vx = x;
        vector.vy = y;
        vector.vz = z;
    }
    VectorNormal(&vector, &vector);
    if (transform != 0)
    {
        PushMatrix();
        matrix = transform != 0 ? &transform->matrix : NULL;
        {
            SetRotMatrix(matrix);
        }
        ApplyRotMatrixLV(&vector, dst);
        PopMatrix();
    }
    else
    {
        dst->vx = vector.vx;
        dst->vy = vector.vy;
        dst->vz = vector.vz;
    }
}

void PathDefinition::SamplePos(sint32 frame, Vec3 &dst, uint32 dir) const
{
    FUNCTION_MARKER(0x800230F0, "SLUS_005.78");
    MATRIX *matrix;

    PathTransform *matrix_transform;
    Vec3 vector;
    if (dir == 0)
        frame = (uint32)pos_track.frame_count - frame - 1;
    matrix_transform = transform;
    matrix = matrix_transform != 0 ? &matrix_transform->matrix : NULL;
    if (matrix_transform != 0)
    {
        AnimationTrack::SampleVectorTrack(frame, &pos_track, vector);
        PushMatrix();
        {
            SetRotMatrix(matrix);
        }
        {
            VECTOR sdk_input = {vector.x, vector.y, vector.z, 0};
            VECTOR sdk_output;
            ApplyRotMatrixLV(&sdk_input, &sdk_output);
            vector.x = sdk_output.vx;
            vector.y = sdk_output.vy;
            vector.z = sdk_output.vz;
        }
        dst = vector;
        PopMatrix();
        dst.x = (uint32)dst.x + matrix->t[0];
        dst.y = (uint32)dst.y + matrix->t[1];
        dst.z = (uint32)dst.z + matrix->t[2];
    }
    else
    {
        AnimationTrack::SampleVectorTrack(frame, &pos_track, dst);
    }
}

uint32 PathDefinition::FindHeightFrame(const Vec3 &input_pos, uint32 dir, sint32 height_offset) const
{
    const Vec3 *pos = &input_pos;
    FUNCTION_MARKER(0x800231DC, "SLUS_005.78");
    MATRIX *matrix;
    Vec3 point, left, right, middle, raw, query;
    Vec3 *dst;
    uint32 cur_flags, idx;
    PathTransform *matrix_transform;
    uint32 x, y, z, a, b, c, d, dx, dy, dz, distance, left_distance, right_distance;
    uint32 first_distance;
    sint32 best_distance;
    uint32 radius;
    uint32 span;
    uint32 forward;
    uint32 outside;
    uint32 unresolved;
    uint32 move_left;
    sint32 best, remaining, frame, low, high, candidate, ca, cb, c0, c1, c2, c3, c4, c5, c6, c7, c8;
    if (pos_track.keys == 0)
    {
        return 0;
    }
    cur_flags = flags;
    if ((cur_flags & 0x10000) != 0)
        return FindLinkedHeightFrame(*pos, dir, height_offset);
    best = -1;
    best_distance = 0x7FFFF000;
    remaining = pos_track.frame_count;
    frame = 0;
    if (height_offset >= 0)
    {
        radius = height_offset;
        span = radius << 1;
        matrix_transform = transform;
        matrix = matrix_transform != 0 ? &matrix_transform->matrix : NULL;
        if (matrix_transform != 0)
        {
            c0 = matrix->m[0][0];
            c1 = matrix->m[1][0];
            c2 = matrix->m[2][0];
            c3 = matrix->m[0][1];
            c4 = matrix->m[1][1];
            c5 = matrix->m[2][1];
            c6 = matrix->m[0][2];
            c7 = matrix->m[1][2];
            c8 = matrix->m[2][2];
            x = -(uint32)matrix->t[0];
            y = -(uint32)matrix->t[1];
            z = -(uint32)matrix->t[2];
            x += (uint32)pos->x;
            y += (uint32)pos->y;
            z += (uint32)pos->z;
            a = mul12((sint32)x, c0);
            a += (uint32)mul12((sint32)y, c1);
            a += (uint32)mul12((sint32)z, c2);
            query.x = a;
            a = mul12((sint32)x, c3);
            a += (uint32)mul12((sint32)y, c4);
            a += (uint32)mul12((sint32)z, c5);
            query.y = a;
            a = mul12((sint32)x, c6);
            a += (uint32)mul12((sint32)y, c7);
            a += (uint32)mul12((sint32)z, c8);
            query.z = a;
            pos = &query;
        }
        do
        {
            AnimationTrack::SampleVectorTrack(frame, &pos_track, point);
            dx = point.x - (uint32)pos->x;
            point.x = dx;
            if (dx + radius < span)
            {
                dy = point.y - (uint32)pos->y;
                point.y = dy;
                if (dy + radius < span)
                {
                    dz = point.z - (uint32)pos->z;
                    point.z = dz;
                    if (dz + radius < span)
                    {
                        a = (sint32)dx >> 6;
                        b = (sint32)dy >> 6;
                        c = (sint32)dz >> 6;
                        distance = a * a + b * b + c * c;
                        if ((sint32)distance < best_distance)
                        {
                            best_distance = distance;
                            best = frame;
                        }
                    }
                }
            }
            remaining = (uint32)remaining - 1;
            frame = (uint32)frame + 1;
        } while (remaining > 0);
        if (best >= 0 && dir == 0)
            best = (uint32)pos_track.frame_count - best - 1;
        goto finish;
    }
    radius = (uint32)search_radius << 5;
    span = radius << 1;
    if ((cur_flags & 2) != 0)
    {
        first_distance = 0x7FFFF000;
        matrix_transform = transform;
        matrix = matrix_transform != 0 ? &matrix_transform->matrix : NULL;
        if (matrix_transform != 0)
        {
            AnimationTrack::SampleVectorTrack(0, &pos_track, raw);
            PushMatrix();
            SetRotMatrix(matrix);
            {
                VECTOR sdk_input = {raw.x, raw.y, raw.z, 0};
                VECTOR sdk_output;
                ApplyRotMatrixLV(&sdk_input, &sdk_output);
                left.x = sdk_output.vx;
                left.y = sdk_output.vy;
                left.z = sdk_output.vz;
            }
            PopMatrix();
            left.x = (uint32)left.x + matrix->t[0];
            left.y = (uint32)left.y + matrix->t[1];
            left.z = (uint32)left.z + matrix->t[2];
        }
        else
        {
            AnimationTrack::SampleVectorTrack(0, &pos_track, left);
        }
        for (;;)
        {
            matrix_transform = transform;
            matrix = matrix_transform != 0 ? &matrix_transform->matrix : NULL;
            if (matrix_transform != 0)
            {
                AnimationTrack::SampleVectorTrack(frame, &pos_track, raw);
                PushMatrix();
                SetRotMatrix(matrix);
                {
                    VECTOR sdk_input = {raw.x, raw.y, raw.z, 0};
                    VECTOR sdk_output;
                    ApplyRotMatrixLV(&sdk_input, &sdk_output);
                    point.x = sdk_output.vx;
                    point.y = sdk_output.vy;
                    point.z = sdk_output.vz;
                }
                PopMatrix();
                point.x = (uint32)point.x + matrix->t[0];
                point.y = (uint32)point.y + matrix->t[1];
                point.z = (uint32)point.z + matrix->t[2];
            }
            else
            {
                AnimationTrack::SampleVectorTrack(frame, &pos_track, point);
            }
            dx = point.x - (uint32)pos->x;
            dy = point.y - (uint32)pos->y;
            dz = point.z - (uint32)pos->z;
            if (frame == 0)
            {
            repeat_last_point:
                if ((sint32)(dy - 40960) <= 0 || (flags & 0x11) != 0)
                    dy = 0;
            }
            if (dx + radius < span && dz + radius < span && ((sint32)(dy - 40960) <= 0 || (flags & 0x11) != 0))
            {
                x = left.x;
                z = left.z;
                a = point.x;
                b = point.z;
                c = pos->x;
                d = pos->z;
                x = mul12((sint32)(x - c), (sint32)(x - a));
                ca = x - (uint32)mul12((sint32)(z - d), (sint32)(b - z));
                x = mul12((sint32)(a - c), (sint32)(left.x - a));
                cb = x - (uint32)mul12((sint32)(b - d), (sint32)(b - z));
                if ((ca ^ cb) < 0 || (uint32)ca + 16 < 33 || (uint32)cb + 16 < 33)
                {
                    a = (sint32)dx >> 6;
                    b = (sint32)dy >> 6;
                    c = (sint32)dz >> 6;
                    distance = a * a + b * b + c * c;
                    if ((sint32)distance < best_distance)
                    {
                        best = frame;
                        if (frame != 0)
                            best_distance = distance;
                        else
                            first_distance = distance;
                    }
                }
            }

            left = point;
            remaining = (uint32)remaining - 1;
            frame = (uint32)frame + 1;
            if (remaining > 0)
                continue;
            if (best > 0)
                break;
            if (remaining != 0)
                break;
            best_distance = first_distance;
            goto repeat_last_point;
        }
        if (best >= 0 && dir == 0)
            best = (uint32)pos_track.frame_count - best;
        goto finish;
    }
    low = 0;
    high = (uint32)remaining - 1;
    for (idx = 0; idx < 2; ++idx)
    {
        candidate = idx == 0 ? 0 : high;
        dst = idx == 0 ? &left : &right;
        matrix_transform = transform;
        matrix = matrix_transform != 0 ? &matrix_transform->matrix : NULL;
        if (matrix_transform != 0)
        {
            AnimationTrack::SampleVectorTrack(candidate, &pos_track, raw);
            PushMatrix();
            SetRotMatrix(matrix);
            {
                VECTOR sdk_input = {raw.x, raw.y, raw.z, 0};
                VECTOR sdk_output;
                ApplyRotMatrixLV(&sdk_input, &sdk_output);
                raw.x = sdk_output.vx;
                raw.y = sdk_output.vy;
                raw.z = sdk_output.vz;
            }
            *dst = raw;
            PopMatrix();
            dst->x = (uint32)dst->x + matrix->t[0];
            dst->y = (uint32)dst->y + matrix->t[1];
            dst->z = (uint32)dst->z + matrix->t[2];
        }
        else
        {
            AnimationTrack::SampleVectorTrack(candidate, &pos_track, *dst);
        }
        dx = dst->x - (uint32)pos->x;

        dz = dst->z - (uint32)pos->z;
        a = (sint32)dx >> 6;
        b = (sint32)dz >> 6;
        distance = a * a + b * b;
        if (idx == 0)
        {
            left_distance = distance;
            if (distance < 0x4000)
                best = 0;
        }
        else
        {
            right_distance = distance;
            if (distance < 0x4000 && (sint32)distance < (sint32)left_distance)
                best = pos_track.frame_count;
        }
    }
    forward = left_distance < right_distance;
    if (forward != 0)
    {
        if ((sint32)(left_distance << 2) < (sint32)right_distance)
            candidate = high >> 1;
        else
            candidate = (high >> 2) + (uint32)(high >> 1);
    }
    else if ((sint32)(right_distance << 2) < (sint32)left_distance)
        candidate = high >> 1;
    else
        candidate = high >> 2;
    unresolved = 1;
    do
    {
        matrix_transform = transform;
        matrix = matrix_transform != 0 ? &matrix_transform->matrix : NULL;
        if (matrix_transform != 0)
        {
            AnimationTrack::SampleVectorTrack(candidate, &pos_track, raw);
            PushMatrix();
            SetRotMatrix(matrix);
            {
                VECTOR sdk_input = {raw.x, raw.y, raw.z, 0};
                VECTOR sdk_output;
                ApplyRotMatrixLV(&sdk_input, &sdk_output);
                middle.x = sdk_output.vx;
                middle.y = sdk_output.vy;
                middle.z = sdk_output.vz;
            }
            PopMatrix();
            middle.x = (uint32)middle.x + matrix->t[0];
            middle.y = (uint32)middle.y + matrix->t[1];
            middle.z = (uint32)middle.z + matrix->t[2];
        }
        else
        {
            AnimationTrack::SampleVectorTrack(candidate, &pos_track, middle);
        }
        dst = forward != 0 ? &left : &right;
        x = dst->x;
        z = dst->z;
        a = middle.x;
        b = middle.z;
        c = pos->x;
        d = pos->z;
        y = mul12((sint32)(x - c), (sint32)(x - a));
        ca = y - (uint32)mul12((sint32)(z - d), (sint32)(b - z));
        y = mul12((sint32)(a - c), (sint32)(x - a));
        cb = y - (uint32)mul12((sint32)(b - d), (sint32)(b - z));
        outside = (ca ^ cb) < 0 || (ca > 0 ? ca < cb : cb < ca || (ca == 0 && ca < cb));
        if (outside != 0 && unresolved != 0)
            unresolved = (ca ^ cb) > 0 && ca != 0 && cb != 0;
        move_left = forward != 0 ? outside == 0 : outside != 0;
        x = middle.x;
        y = middle.y;
        z = middle.z;
        if (move_left != 0)
        {
            left.x = x;
            left.y = y;
            left.z = z;
            low = candidate;
        }
        else
        {
            right.x = x;
            right.y = y;
            right.z = z;
            high = candidate;
        }
        candidate = (sint32)((uint32)low + high) >> 1;
    } while (low != candidate);
    if (unresolved == 0 && (forward != 0 ? high < pos_track.frame_count - 1 : low > 0))
    {
        dx = right.x - (uint32)pos->x;
        dy = right.y - (uint32)pos->y;
        dz = right.z - (uint32)pos->z;
        if (dx + radius < span && dz + radius < span && ((sint32)(dy - 40960) <= 0 || (flags & 0x11) != 0))
        {
            if (dir != 0)
                high = (uint32)high - 1;
            if (high <= 0)
                best = 1;
            else
            {
                best = high;
                remaining = pos_track.frame_count;
                if (high >= remaining)
                    best = remaining - 1;
            }
        }
        else
        {
            dx = left.x - (uint32)pos->x;
            dy = left.y - (uint32)pos->y;
            dz = left.z - (uint32)pos->z;
            if (dx + radius < span && dz + radius < span && ((sint32)(dy - 40960) <= 0 || (flags & 0x11) != 0))
            {
                if (low <= 0)
                    low = 1;
                else
                {
                    remaining = pos_track.frame_count;
                    if (low >= remaining)
                        low = remaining - 1;
                }
                best = low;
            }
        }
    }
    if (best >= 0)
    {
        if (dir == 0)
            best = (uint32)pos_track.frame_count - best;
        remaining = pos_track.frame_count;
        if (best == remaining - 1)
            best = remaining - 2;
    }
finish:
    return best;
}

uint32 PathDefinition::FindLinkedHeightFrame(const Vec3 &input_pos, uint32 dir, sint32 height_offset) const
{
    const Vec3 *pos = &input_pos;
    FUNCTION_MARKER(0x80024DE0, "SLUS_005.78");
    MATRIX *matrix;
    Vec3 delta, first, second, sample, tmp;
    uint32 val, x, y, z;
    PathTransform *matrix_transform;
    uint32 first_distance;
    uint32 second_distance;
    sint32 distance;
    sint32 best_distance;
    uint32 radius;
    uint32 extent;
    uint32 effective_dir;
    sint32 frames, low, high, frame, best_frame, nearer_first, choose_high;
    sint32 c0, c1, c2, c3, c4, c5, c6, c7, c8;
    if (!pos_track.keys)
    {
        return 0;
    }
    effective_dir = dir ? dir : 16;
    frames = pos_track.frame_count;
    if (height_offset < 0)
    {
        low = 0;
        matrix_transform = transform;
        matrix = matrix_transform != 0 ? &matrix_transform->matrix : NULL;
        high = frames - 1;
        if (matrix_transform)
        {
            AnimationTrack::SampleVectorTrack(0, &pos_track, tmp);
            PushMatrix();
            SetRotMatrix(matrix);
            {
                VECTOR sdk_input = {tmp.x, tmp.y, tmp.z, 0};
                VECTOR sdk_output;
                ApplyRotMatrixLV(&sdk_input, &sdk_output);
                first.x = sdk_output.vx;
                first.y = sdk_output.vy;
                first.z = sdk_output.vz;
            }
            PopMatrix();
            x = first.x;
            y = matrix->t[0];
            first.x = x + y;
            x = first.y;
            y = matrix->t[1];
            first.y = x + y;
            x = first.z;
            y = matrix->t[2];
            first.z = x + y;
        }
        else
        {
            AnimationTrack::SampleVectorTrack(0, &pos_track, first);
        }
        x = first.x;
        x -= (uint32)pos->x;
        y = first.y;
        delta.x = x;
        y -= (uint32)pos->y;
        z = first.z;
        delta.y = y;
        z -= (uint32)pos->z;
        delta.z = z;
        delta.y = 0;
        x = (sint32)x >> 6;
        z = (sint32)z >> 6;
        first_distance = x * x + z * z;
        matrix_transform = transform;
        matrix = matrix_transform != 0 ? &matrix_transform->matrix : NULL;
        if (matrix_transform)
        {
            AnimationTrack::SampleVectorTrack(high, &pos_track, tmp);
            PushMatrix();
            SetRotMatrix(matrix);
            {
                VECTOR sdk_input = {tmp.x, tmp.y, tmp.z, 0};
                VECTOR sdk_output;
                ApplyRotMatrixLV(&sdk_input, &sdk_output);
                second.x = sdk_output.vx;
                second.y = sdk_output.vy;
                second.z = sdk_output.vz;
            }
            PopMatrix();
            x = second.x;
            y = matrix->t[0];
            second.x = x + y;
            x = second.y;
            y = matrix->t[1];
            second.y = x + y;
            x = second.z;
            y = matrix->t[2];
            second.z = x + y;
        }
        else
        {
            AnimationTrack::SampleVectorTrack(high, &pos_track, second);
        }
        x = second.x;
        x -= (uint32)pos->x;
        y = second.y;
        delta.x = x;
        y -= (uint32)pos->y;
        z = second.z;
        delta.y = y;
        z -= (uint32)pos->z;
        delta.z = z;
        delta.y = 0;
        x = (sint32)x >> 6;
        z = (sint32)z >> 6;
        second_distance = x * x + z * z;
        nearer_first = first_distance < second_distance;
        if (nearer_first)
        {
            frame = high >> 1;
            if ((sint32)(first_distance << 2) >= (sint32)second_distance)
                frame = (high >> 2) + (uint32)(high >> 1);
        }
        else
        {
            frame = high >> 1;
            if ((sint32)(second_distance << 2) >= (sint32)first_distance)
                frame = high >> 2;
        }
        val = pos->y;
        if ((sint32)val < first.y || second.y < (sint32)val)
        {
            return -1;
        }
        for (;;)
        {
            matrix_transform = transform;
            matrix = matrix_transform != 0 ? &matrix_transform->matrix : NULL;
            if (matrix_transform)
            {
                AnimationTrack::SampleVectorTrack(frame, &pos_track, tmp);
                PushMatrix();
                SetRotMatrix(matrix);
                {
                    VECTOR sdk_input = {tmp.x, tmp.y, tmp.z, 0};
                    VECTOR sdk_output;
                    ApplyRotMatrixLV(&sdk_input, &sdk_output);
                    sample.x = sdk_output.vx;
                    sample.y = sdk_output.vy;
                    sample.z = sdk_output.vz;
                }
                PopMatrix();
                x = sample.x;
                y = matrix->t[0];
                sample.x = x + y;
                x = sample.y;
                y = matrix->t[1];
                sample.y = x + y;
                x = sample.z;
                y = matrix->t[2];
                sample.z = x + y;
            }
            else
            {
                AnimationTrack::SampleVectorTrack(frame, &pos_track, sample);
            }
            val = pos->y;
            y = sample.y;
            choose_high = nearer_first ? (sint32)val < (sint32)y : (sint32)y >= (sint32)val;
            x = sample.x;
            y = sample.y;
            z = sample.z;
            if (choose_high)
            {
                second.x = x;
                second.y = y;
                second.z = z;
                high = frame;
            }
            else
            {
                first.x = x;
                first.y = y;
                first.z = z;
                low = frame;
            }
            frame = (sint32)((uint32)low + high) >> 1;
            if (low == frame)
            {
                if (!dir)
                    frame = (uint32)pos_track.frame_count - frame - 1;
                return frame;
            }
        }
    }
    radius = height_offset;
    extent = radius << 1;
    matrix_transform = transform;
    matrix = matrix_transform != 0 ? &matrix_transform->matrix : NULL;
    if (matrix_transform)
    {
        c0 = matrix->m[0][0];
        c1 = matrix->m[1][0];
        c2 = matrix->m[2][0];
        c3 = matrix->m[0][1];
        c4 = matrix->m[1][1];
        c5 = matrix->m[2][1];
        c6 = matrix->m[0][2];
        c7 = matrix->m[1][2];
        c8 = matrix->m[2][2];
        x = -(uint32)matrix->t[0];
        delta.x = x;
        y = -(uint32)matrix->t[1];
        delta.y = y;
        z = -(uint32)matrix->t[2];
        delta.z = z;
        x += (uint32)pos->x;
        y += (uint32)pos->y;
        z += (uint32)pos->z;
        val = mul12((sint32)x, c0);
        val += (uint32)mul12((sint32)y, c1);
        val += (uint32)mul12((sint32)z, c2);
        first.x = val;
        val = mul12((sint32)x, c3);
        val += (uint32)mul12((sint32)y, c4);
        val += (uint32)mul12((sint32)z, c5);
        first.y = val;
        val = mul12((sint32)x, c6);
        val += (uint32)mul12((sint32)y, c7);
        val += (uint32)mul12((sint32)z, c8);
        first.z = val;
        pos = &first;
    }
    frame = 0;
    best_frame = -1;
    best_distance = 0x7FFFF000;
    do
    {
        AnimationTrack::SampleVectorTrack(frame, &pos_track, delta);
        x = delta.x;
        x -= (uint32)pos->x;
        delta.x = x;
        if (x + radius < extent)
        {
            y = delta.y;
            y -= (uint32)pos->y;
            delta.y = y;
            if (y + radius < extent)
            {
                z = delta.z;
                z -= (uint32)pos->z;
                delta.z = z;
                if (z + radius < extent)
                {
                    x = (sint32)x >> 6;
                    y = (sint32)y >> 6;
                    z = (sint32)z >> 6;
                    distance = x * x + y * y + z * z;
                    if (distance < best_distance)
                    {
                        best_distance = distance;
                        best_frame = frame;
                    }
                }
            }
        }
        --frames;
        frame = (uint32)frame + 1;
    } while (frames > 0);
    if (best_frame >= 0 && !effective_dir)
        best_frame = (uint32)pos_track.frame_count - best_frame - 1;
    return best_frame;
}

sint32 PathDefinition::ProjectFrame(sint32 frame, const Vec3 &pos, Vec3 &floor, uint32 dir, sint32 scale) const
{
    FUNCTION_MARKER(0x8003D3FC, "SLUS_005.78");
    Vec3 vector;
    uint32 side;

    uint32 x;
    sint32 y;
    uint32 z;
    uint32 floor_x;
    uint32 floor_z;
    uint32 point_x;
    uint32 point_z;
    uint32 input_x;
    uint32 input_z;
    sint32 dx;
    sint32 dz;
    uint32 point_dot;
    uint32 floor_dot;
    uint32 ratio;
    sint32 sq;
    sint32 limit;
    uint32 last_endpoint = 0;
    sint32 rcp, factor, result;
    if (frame > 0)
    {
        sint32 count = pos_track.frame_count;
        if (frame >= count)
            frame = count - 1;
    }
    else
        frame = 0;
    SamplePos(frame, floor, dir);
    for (side = 0; side < 2; ++side)
    {
        if (side == 0 && frame == 0)
            continue;
        SamplePos((uint32)frame + (side == 0 ? 0xFFFFFFFF : 1), vector, dir);
        floor_z = floor.z;
        point_z = vector.z;
        point_x = vector.x;
        floor_x = floor.x;
        input_x = pos.x;
        dz = floor_z - point_z;
        dx = point_x - floor_x;
        x = point_x - input_x;
        point_dot = mul12((sint32)x, dx);
        input_z = pos.z;
        z = point_z - input_z;
        point_dot -= (uint32)mul12((sint32)z, dz);
        x = floor_x - input_x;
        floor_dot = mul12((sint32)x, dx);
        z = floor_z - input_z;
        floor_dot -= (uint32)mul12((sint32)z, dz);
        if (((point_dot ^ floor_dot) & 0x80000000) != 0 || point_dot + 16 < 33 || floor_dot + 16 < 33)
        {
            if (side != 0)
                frame = (uint32)frame + 1;
            goto interpolate;
        }
        if (side == 0)
        {
            if (frame != pos_track.frame_count - 1)
                continue;
            last_endpoint = 1;
        }
        else if (frame != 0)
        {
            result = (uint32)frame + 1;
            goto done;
        }
        goto endpoint;
    }
    result = frame;
    goto done;
endpoint:
    if (((uint32)scale << 1) < floor.x - (uint32)pos.x + scale && ((uint32)scale << 1) < floor.z - (uint32)pos.z + scale)
    {
        result = -1;
        goto done;
    }
    if (point_dot == floor_dot)
    {
        vector.x = floor.x - (uint32)pos.x;
        vector.y = floor.y - (uint32)pos.y;
        vector.z = floor.z - (uint32)pos.z;
    }
    else
    {
        rcp = 0x40000000 / (sint32)(point_dot - floor_dot);
        ratio = ((sint64)(sint32)point_dot * rcp) >> 18;
        fixed_math_result = ratio;
        x = floor.x - (uint32)vector.x;
        vector.x = x;
        y = floor.y - (uint32)vector.y;
        vector.y = y;
        z = floor.z - (uint32)vector.z;
        vector.z = z;
        factor = ratio - 4096;
        vector.x = mul12((sint32)x, factor);
        vector.y = mul12(y, factor);
        vector.z = mul12((sint32)z, factor);
    }
    x = vector.x >> 6;
    z = vector.z >> 6;
    vector.y = 0;
    sq = x * x + z * z;
    limit = mul12(scale, scale);
    if (last_endpoint != 0)
        result = limit < sq ? -1 : pos_track.frame_count;
    else
        result = limit < sq ? -1 : 0;
    goto done;
interpolate:
    if (point_dot != floor_dot)
    {
        rcp = 0x40000000 / (sint32)(floor_dot - point_dot);
        ratio = ((sint64)(sint32)floor_dot * rcp) >> 18;
        fixed_math_result = ratio;
        x = vector.x - (uint32)floor.x;
        vector.x = x;
        y = vector.y - (uint32)floor.y;
        vector.y = y;
        z = vector.z - (uint32)floor.z;
        vector.z = z;
        vector.x = mul12((sint32)x, (sint32)ratio);
        vector.y = mul12(y, (sint32)ratio);
        vector.z = mul12((sint32)z, (sint32)ratio);
        floor.x = floor.x + (uint32)vector.x;
        floor.y = floor.y + (uint32)vector.y;
        floor.z = floor.z + (uint32)vector.z;
    }
    result = frame;
done:
    return result;
}

void PathDefinition::SampleActorPos(sint32 frame, Vec3 &dst, uint32 dir) const
{
    FUNCTION_MARKER(0x80056340, "SLUS_005.78");
    MATRIX *matrix;
    Vec3 vector;
    uint32 x, y, result;
    PathTransform *matrix_transform;
    if (dir == 0)
        frame = (uint32)pos_track.frame_count - frame - 1;
    matrix_transform = transform;
    matrix = matrix_transform != 0 ? &matrix_transform->matrix : NULL;
    if (matrix_transform == 0)
    {
        AnimationTrack::SampleVectorTrack(frame, &pos_track, dst);
        return;
    }
    AnimationTrack::SampleVectorTrack(frame, &pos_track, vector);
    PushMatrix();
    SetRotMatrix(matrix);
    {
        VECTOR sdk_input = {vector.x, vector.y, vector.z, 0};
        VECTOR sdk_output;
        ApplyRotMatrixLV(&sdk_input, &sdk_output);
        vector.x = sdk_output.vx;
        vector.y = sdk_output.vy;
        vector.z = sdk_output.vz;
    }
    dst = vector;
    PopMatrix();
    x = dst.x;
    x += (uint32)matrix->t[0];
    dst.x = x;
    y = dst.y;
    y += (uint32)matrix->t[1];
    dst.y = y;
    result = dst.z;
    result += (uint32)matrix->t[2];
    dst.z = result;
}

sint16 PathDefinition::SampleDefinitionHeading(sint32 frame0, sint32 frame1, const PathDefinition *definition)
{
    FUNCTION_MARKER(0x8005642C, "SLUS_005.78");

    Vec3 pos[2], input;

    Vec3 *dst;
    MATRIX *matrix;
    uint32 sample;
    uint32 x;
    uint32 y;
    uint32 z;
    uint32 dx;
    uint32 dy;
    uint32 dz;
    sint16 result;
    PathTransform *matrix_transform;
    sint32 frame;
    for (sample = 0; sample < 2; ++sample)
    {
        frame = sample == 0 ? frame0 : frame1;
        dst = &pos[sample];
        matrix_transform = definition->transform;
        if (matrix_transform != 0)
        {
            matrix = &matrix_transform->matrix;
            AnimationTrack::SampleVectorTrack(frame, &definition->pos_track, input);
            PushMatrix();
            SetRotMatrix(matrix);
            {
                VECTOR sdk_input = {input.x, input.y, input.z, 0};
                VECTOR sdk_output;
                ApplyRotMatrixLV(&sdk_input, &sdk_output);
                input.x = sdk_output.vx;
                input.y = sdk_output.vy;
                input.z = sdk_output.vz;
            }
            *dst = input;
            PopMatrix();
            x = dst->x;
            x += matrix->t[0];
            dst->x = x;
            y = dst->y;
            y += matrix->t[1];
            dst->y = y;
            z = dst->z;
            z += matrix->t[2];
            dst->z = z;
        }
        else
            AnimationTrack::SampleVectorTrack(frame, &definition->pos_track, *dst);
    }
    x = pos[1].x;
    dx = x - pos[0].x;
    y = pos[1].y;
    dy = y - pos[0].y;
    z = pos[1].z;
    dz = z - pos[0].z;
    path_heading_delta.x = dx;
    path_heading_delta.y = dy;
    path_heading_delta.z = dz;
    if (dx == 0)
        result = (sint32)dz >= 0 ? 2048 : 0;
    else
        result = Atan2Angle((-dz), (-dx));
    return result;
}
