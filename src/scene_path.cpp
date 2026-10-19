#include "game.h"
#include "original_defaults.h"

PathRouting path_routing = {0, 0, 1};

union PathVector
{
    VECTOR vector;
    uint32 words[4];
};

uint32 PathDefinition::SamplePosition(sint32 frame, uint32 *destination, uint32 direction) const
{
    const PathTransform *captured_path_transform;
    FUNCTION_MARKER(0x800230F0, "SLUS_005.78");
    const MATRIX *matrix;
    uint32 i, result;
    const PathTransform *matrix_transform;
    PathVector vector;
    if (direction == 0)
        frame = (sint32)((uint32)(sint16)positions.metadata6 - (uint32)frame - 1);
    matrix_transform = ((captured_path_transform = (this->transform)) != 0 ? (const PathTransform *)((PathTransform *)(captured_path_transform)) : 0);
    matrix = (matrix_transform != 0 ? &matrix_transform->matrix : (const MATRIX *)0);
    if (matrix_transform != 0)
    {
        SampleVectorTrack(frame, &positions, vector.words);
        PushMatrix();
        for (i = 0; i < 5; ++i)
            xport_gte_write_control(i, xport_load_le32((const uint8 *)matrix + i * 4));
        ApplyRotMatrixLV(&vector.vector, (VECTOR *)destination);
        PopMatrix();
        for (i = 0; i < 3; ++i)
        {
            result = destination[i] + (uint32)matrix->t[i];
            destination[i] = result;
        }
    }
    else
    {
        result = SampleVectorTrack(frame, &positions, destination);
    }
    return result;
}

uint32 ScenePath::SampleOffset(sint32 frame, uint32 *destination, uint32 direction) const
{
    FUNCTION_MARKER(0x80022C24, "SLUS_005.78");
    const MATRIX *matrix;
    uint32 i, result;
    PathVector vector;
    if ((node.state_flags & (direction | 8)) == 0)
        frame = (sint32)((uint32)(sint16)((const PathDefinition *)((const PathDefinition *)((this->definition))))->positions.metadata6 - (uint32)frame - 1);
    matrix = offset_matrix;
    if (matrix != 0)
    {
        SampleVectorTrack(frame, &offset_track, vector.words);
        PushMatrix();
        matrix = offset_matrix;
        for (i = 0; i < 5; ++i)
            xport_gte_write_control(i, xport_load_le32((const uint8 *)matrix + i * 4));
        ApplyRotMatrixLV(&vector.vector, (VECTOR *)destination);
        PopMatrix();
        for (i = 0; i < 3; ++i)
        {
            matrix = offset_matrix;
            result = destination[i] + (uint32)matrix->t[i];
            destination[i] = result;
        }
    }
    else
        result = SampleVectorTrack(frame, &offset_track, destination);
    return result;
}

uint32 ScenePath::SamplePosition(sint32 frame, uint32 *destination, uint32 direction) const
{
    const PathTransform *captured_path_transform;
    FUNCTION_MARKER(0x80022B20, "SLUS_005.78");
    const MATRIX *matrix;
    const PathDefinition *definition;
    uint32 i, result;
    const PathTransform *matrix_transform;
    PathVector vector;
    if (direction == 0)
        frame = (sint32)((uint32)(sint16)((const PathDefinition *)((const PathDefinition *)((this->definition))))->positions.metadata6 - (uint32)frame - 1);
    definition = (const PathDefinition *)((const PathDefinition *)((this->definition)));
    matrix_transform = ((captured_path_transform = (definition->transform)) != 0 ? (const PathTransform *)((PathTransform *)(captured_path_transform)) : 0);
    matrix = (matrix_transform != 0 ? &matrix_transform->matrix : (const MATRIX *)0);
    if (matrix_transform != 0)
    {
        SampleVectorTrack(frame, &definition->positions, vector.words);
        PushMatrix();
        for (i = 0; i < 5; ++i)
            xport_gte_write_control(i, xport_load_le32((const uint8 *)matrix + i * 4));
        ApplyRotMatrixLV(&vector.vector, (VECTOR *)destination);
        PopMatrix();
        for (i = 0; i < 3; ++i)
        {
            result = destination[i] + (uint32)matrix->t[i];
            destination[i] = result;
        }
    }
    else
    {
        result = SampleVectorTrack(frame, &definition->positions, destination);
    }
    return result;
}

sint32 PathDefinition::ProjectFrame(sint32 frame, const uint32 *position, uint32 *floor, uint32 direction, sint32 scale) const
{
    FUNCTION_MARKER(0x8003D3FC, "SLUS_005.78");
    PathVector vector;
    uint32 side, i, x, y, z, floor_x, floor_z, point_x, point_z, input_x, input_z;
    uint32 dx, dz, point_dot, floor_dot, ratio, squared, limit, last_endpoint = 0;
    sint32 reciprocal, factor, result;
    if (frame > 0)
    {
        sint32 count = (sint16)positions.metadata6;
        if (frame >= count)
            frame = count - 1;
    }
    else
        frame = 0;
    SamplePosition(frame, floor, direction);
    for (side = 0; side < 2; ++side)
    {
        if (side == 0 && frame == 0)
            continue;
        SamplePosition((sint32)((uint32)frame + (side == 0 ? 0xFFFFFFFF : 1)), vector.words, direction);
        floor_z = floor[2];
        point_z = vector.words[2];
        point_x = vector.words[0];
        floor_x = floor[0];
        input_x = position[0];
        dz = floor_z - point_z;
        dx = point_x - floor_x;
        x = point_x - input_x;
        point_dot = (uint32)(((sint64)(sint32)x * (sint32)dx) >> 12);
        input_z = position[2];
        z = point_z - input_z;
        point_dot -= (uint32)(((sint64)(sint32)z * (sint32)dz) >> 12);
        x = floor_x - input_x;
        floor_dot = (uint32)(((sint64)(sint32)x * (sint32)dx) >> 12);
        z = floor_z - input_z;
        floor_dot -= (uint32)(((sint64)(sint32)z * (sint32)dz) >> 12);
        if (((point_dot ^ floor_dot) & 0x80000000) != 0 || point_dot + 16 < 33 || floor_dot + 16 < 33)
        {
            if (side != 0)
                frame = (sint32)((uint32)frame + 1);
            goto interpolate;
        }
        if (side == 0)
        {
            if (frame != (sint16)positions.metadata6 - 1)
                continue;
            last_endpoint = 1;
        }
        else if (frame != 0)
        {
            result = (sint32)((uint32)frame + 1);
            goto done;
        }
        goto endpoint;
    }
    result = frame;
    goto done;
endpoint:
    if (((uint32)scale << 1) < floor[0] - position[0] + (uint32)scale && ((uint32)scale << 1) < floor[2] - position[2] + (uint32)scale)
    {
        result = -1;
        goto done;
    }
    if (point_dot == floor_dot)
    {
        for (i = 0; i < 3; ++i)
            vector.words[i] = floor[i] - position[i];
    }
    else
    {
        reciprocal = 0x40000000 / (sint32)(point_dot - floor_dot);
        ratio = (uint32)(((sint64)(sint32)point_dot * reciprocal) >> 18);
        fixed_math_result = ratio;
        x = floor[0] - vector.words[0];
        vector.words[0] = x;
        y = floor[1] - vector.words[1];
        vector.words[1] = y;
        z = floor[2] - vector.words[2];
        vector.words[2] = z;
        factor = (sint32)(ratio - 4096);
        vector.words[0] = (uint32)(((sint64)(sint32)x * factor) >> 12);
        vector.words[1] = (uint32)(((sint64)(sint32)y * factor) >> 12);
        vector.words[2] = (uint32)(((sint64)(sint32)z * factor) >> 12);
    }
    x = (uint32)((sint32)vector.words[0] >> 6);
    z = (uint32)((sint32)vector.words[2] >> 6);
    vector.words[1] = 0;
    squared = x * x + z * z;
    limit = (uint32)(((sint64)scale * scale) >> 12);
    if (last_endpoint != 0)
        result = (sint32)limit < (sint32)squared ? -1 : (sint16)positions.metadata6;
    else
        result = (sint32)limit < (sint32)squared ? -1 : 0;
    goto done;
interpolate:
    if (point_dot != floor_dot)
    {
        reciprocal = 0x40000000 / (sint32)(floor_dot - point_dot);
        ratio = (uint32)(((sint64)(sint32)floor_dot * reciprocal) >> 18);
        fixed_math_result = ratio;
        x = vector.words[0] - floor[0];
        vector.words[0] = x;
        y = vector.words[1] - floor[1];
        vector.words[1] = y;
        z = vector.words[2] - floor[2];
        vector.words[2] = z;
        vector.words[0] = (uint32)(((sint64)(sint32)x * (sint32)ratio) >> 12);
        vector.words[1] = (uint32)(((sint64)(sint32)y * (sint32)ratio) >> 12);
        vector.words[2] = (uint32)(((sint64)(sint32)z * (sint32)ratio) >> 12);
        for (i = 0; i < 3; ++i)
            floor[i] = floor[i] + vector.words[i];
    }
    result = frame;
done:
    return result;
}

uint32 PathDefinition::FindLinkedHeightFrame(const uint32 *position, uint32 direction, sint32 height_offset) const
{
    const PathTransform *captured_path_transform;
    FUNCTION_MARKER(0x80024DE0, "SLUS_005.78");
    const MATRIX *matrix;
    PathVector delta, first, second, sample, temporary;
    uint32 value, x, y, z;
    const PathTransform *matrix_transform;
    uint32 first_distance, second_distance, distance, best_distance, radius, extent, effective_direction;
    sint32 frames, low, high, frame, best_frame, nearer_first, choose_high;
    sint32 c0, c1, c2, c3, c4, c5, c6, c7, c8;
    if (!positions.keys)
    {
        return 0;
    }
    effective_direction = direction ? direction : 16;
    frames = (sint16)positions.metadata6;
    if (height_offset < 0)
    {
        low = 0;
        matrix_transform = ((captured_path_transform = (this->transform)) != 0 ? (const PathTransform *)((PathTransform *)(captured_path_transform)) : 0);
        matrix = (matrix_transform != 0 ? &matrix_transform->matrix : (const MATRIX *)0);
        high = frames - 1;
        if (matrix_transform)
        {
            SampleVectorTrack(0, &positions, temporary.words);
            PushMatrix();
            x = xport_load_le32((const uint8 *)matrix);
            y = xport_load_le32((const uint8 *)matrix + 4);
            xport_gte_write_control(0, x);
            xport_gte_write_control(1, y);
            x = xport_load_le32((const uint8 *)matrix + 8);
            y = xport_load_le32((const uint8 *)matrix + 12);
            z = xport_load_le32((const uint8 *)matrix + 16);
            xport_gte_write_control(2, x);
            xport_gte_write_control(3, y);
            xport_gte_write_control(4, z);
            ApplyRotMatrixLV(&temporary.vector, &first.vector);
            PopMatrix();
            x = first.words[0];
            y = (uint32)matrix->t[0];
            first.words[0] = x + y;
            x = first.words[1];
            y = (uint32)matrix->t[1];
            first.words[1] = x + y;
            x = first.words[2];
            y = (uint32)matrix->t[2];
            first.words[2] = x + y;
        }
        else
        {
            SampleVectorTrack(0, &positions, first.words);
        }
        x = first.words[0];
        x -= position[0];
        y = first.words[1];
        delta.words[0] = x;
        y -= position[1];
        z = first.words[2];
        delta.words[1] = y;
        z -= position[2];
        delta.words[2] = z;
        delta.words[1] = 0;
        x = (uint32)((sint32)x >> 6);
        z = (uint32)((sint32)z >> 6);
        first_distance = x * x + z * z;
        matrix_transform = ((captured_path_transform = (this->transform)) != 0 ? (const PathTransform *)((PathTransform *)(captured_path_transform)) : 0);
        matrix = (matrix_transform != 0 ? &matrix_transform->matrix : (const MATRIX *)0);
        if (matrix_transform)
        {
            SampleVectorTrack(high, &positions, temporary.words);
            PushMatrix();
            x = xport_load_le32((const uint8 *)matrix);
            y = xport_load_le32((const uint8 *)matrix + 4);
            xport_gte_write_control(0, x);
            xport_gte_write_control(1, y);
            x = xport_load_le32((const uint8 *)matrix + 8);
            y = xport_load_le32((const uint8 *)matrix + 12);
            z = xport_load_le32((const uint8 *)matrix + 16);
            xport_gte_write_control(2, x);
            xport_gte_write_control(3, y);
            xport_gte_write_control(4, z);
            ApplyRotMatrixLV(&temporary.vector, &second.vector);
            PopMatrix();
            x = second.words[0];
            y = (uint32)matrix->t[0];
            second.words[0] = x + y;
            x = second.words[1];
            y = (uint32)matrix->t[1];
            second.words[1] = x + y;
            x = second.words[2];
            y = (uint32)matrix->t[2];
            second.words[2] = x + y;
        }
        else
        {
            SampleVectorTrack(high, &positions, second.words);
        }
        x = second.words[0];
        x -= position[0];
        y = second.words[1];
        delta.words[0] = x;
        y -= position[1];
        z = second.words[2];
        delta.words[1] = y;
        z -= position[2];
        delta.words[2] = z;
        delta.words[1] = 0;
        x = (uint32)((sint32)x >> 6);
        z = (uint32)((sint32)z >> 6);
        second_distance = x * x + z * z;
        nearer_first = first_distance < second_distance;
        if (nearer_first)
        {
            frame = high >> 1;
            if ((sint32)(first_distance << 2) >= (sint32)second_distance)
                frame = (sint32)((uint32)(high >> 2) + (uint32)(high >> 1));
        }
        else
        {
            frame = high >> 1;
            if ((sint32)(second_distance << 2) >= (sint32)first_distance)
                frame = high >> 2;
        }
        value = position[1];
        if ((sint32)value < (sint32)first.words[1] || (sint32)second.words[1] < (sint32)value)
        {
            return -1;
        }
        for (;;)
        {
            matrix_transform = ((captured_path_transform = (this->transform)) != 0 ? (const PathTransform *)((PathTransform *)(captured_path_transform)) : 0);
            matrix = (matrix_transform != 0 ? &matrix_transform->matrix : (const MATRIX *)0);
            if (matrix_transform)
            {
                SampleVectorTrack(frame, &positions, temporary.words);
                PushMatrix();
                x = xport_load_le32((const uint8 *)matrix);
                y = xport_load_le32((const uint8 *)matrix + 4);
                xport_gte_write_control(0, x);
                xport_gte_write_control(1, y);
                x = xport_load_le32((const uint8 *)matrix + 8);
                y = xport_load_le32((const uint8 *)matrix + 12);
                z = xport_load_le32((const uint8 *)matrix + 16);
                xport_gte_write_control(2, x);
                xport_gte_write_control(3, y);
                xport_gte_write_control(4, z);
                ApplyRotMatrixLV(&temporary.vector, &sample.vector);
                PopMatrix();
                x = sample.words[0];
                y = (uint32)matrix->t[0];
                sample.words[0] = x + y;
                x = sample.words[1];
                y = (uint32)matrix->t[1];
                sample.words[1] = x + y;
                x = sample.words[2];
                y = (uint32)matrix->t[2];
                sample.words[2] = x + y;
            }
            else
            {
                SampleVectorTrack(frame, &positions, sample.words);
            }
            value = position[1];
            y = sample.words[1];
            choose_high = nearer_first ? (sint32)value < (sint32)y : (sint32)y >= (sint32)value;
            x = sample.words[0];
            y = sample.words[1];
            z = sample.words[2];
            if (choose_high)
            {
                second.words[0] = x;
                second.words[1] = y;
                second.words[2] = z;
                high = frame;
            }
            else
            {
                first.words[0] = x;
                first.words[1] = y;
                first.words[2] = z;
                low = frame;
            }
            frame = (sint32)((uint32)low + (uint32)high) >> 1;
            if (low == frame)
            {
                if (!direction)
                    frame = (sint32)((uint32)(sint32)(sint16)positions.metadata6 - (uint32)frame - 1);
                return frame;
            }
        }
    }
    radius = (uint32)height_offset;
    extent = radius << 1;
    matrix_transform = ((captured_path_transform = (this->transform)) != 0 ? (const PathTransform *)((PathTransform *)(captured_path_transform)) : 0);
    matrix = (matrix_transform != 0 ? &matrix_transform->matrix : (const MATRIX *)0);
    if (matrix_transform)
    {
        value = (uint16)matrix->m[0][0];
        c0 = (sint16)value;
        value = (uint16)((const MATRIX *)((this->transform) != 0 ? &(this->transform)->matrix : (MATRIX *)0))->m[1][0];
        c1 = (sint16)value;
        value = (uint16)((const MATRIX *)((this->transform) != 0 ? &(this->transform)->matrix : (MATRIX *)0))->m[2][0];
        c2 = (sint16)value;
        value = (uint16)((const MATRIX *)((this->transform) != 0 ? &(this->transform)->matrix : (MATRIX *)0))->m[0][1];
        c3 = (sint16)value;
        value = (uint16)((const MATRIX *)((this->transform) != 0 ? &(this->transform)->matrix : (MATRIX *)0))->m[1][1];
        c4 = (sint16)value;
        value = (uint16)((const MATRIX *)((this->transform) != 0 ? &(this->transform)->matrix : (MATRIX *)0))->m[2][1];
        c5 = (sint16)value;
        value = (uint16)((const MATRIX *)((this->transform) != 0 ? &(this->transform)->matrix : (MATRIX *)0))->m[0][2];
        c6 = (sint16)value;
        value = (uint16)((const MATRIX *)((this->transform) != 0 ? &(this->transform)->matrix : (MATRIX *)0))->m[1][2];
        c7 = (sint16)value;
        value = (uint16)((const MATRIX *)((this->transform) != 0 ? &(this->transform)->matrix : (MATRIX *)0))->m[2][2];
        c8 = (sint16)value;
        x = 0 - (uint32)((const MATRIX *)((this->transform) != 0 ? &(this->transform)->matrix : (MATRIX *)0))->t[0];
        delta.words[0] = x;
        y = 0 - (uint32)((const MATRIX *)((this->transform) != 0 ? &(this->transform)->matrix : (MATRIX *)0))->t[1];
        delta.words[1] = y;
        z = 0 - (uint32)((const MATRIX *)((this->transform) != 0 ? &(this->transform)->matrix : (MATRIX *)0))->t[2];
        delta.words[2] = z;
        x += position[0];
        y += position[1];
        z += position[2];
        value = (uint32)(((sint64)(sint32)x * c0) >> 12);
        value += (uint32)(((sint64)(sint32)y * c1) >> 12);
        value += (uint32)(((sint64)(sint32)z * c2) >> 12);
        first.words[0] = value;
        value = (uint32)(((sint64)(sint32)x * c3) >> 12);
        value += (uint32)(((sint64)(sint32)y * c4) >> 12);
        value += (uint32)(((sint64)(sint32)z * c5) >> 12);
        first.words[1] = value;
        value = (uint32)(((sint64)(sint32)x * c6) >> 12);
        value += (uint32)(((sint64)(sint32)y * c7) >> 12);
        value += (uint32)(((sint64)(sint32)z * c8) >> 12);
        first.words[2] = value;
        position = first.words;
    }
    frame = 0;
    best_frame = -1;
    best_distance = 0x7FFFF000;
    do
    {
        SampleVectorTrack(frame, &positions, delta.words);
        x = delta.words[0];
        x -= position[0];
        delta.words[0] = x;
        if (x + radius < extent)
        {
            y = delta.words[1];
            y -= position[1];
            delta.words[1] = y;
            if (y + radius < extent)
            {
                z = delta.words[2];
                z -= position[2];
                delta.words[2] = z;
                if (z + radius < extent)
                {
                    x = (uint32)((sint32)x >> 6);
                    y = (uint32)((sint32)y >> 6);
                    z = (uint32)((sint32)z >> 6);
                    distance = x * x + y * y + z * z;
                    if ((sint32)distance < (sint32)best_distance)
                    {
                        best_distance = distance;
                        best_frame = frame;
                    }
                }
            }
        }
        --frames;
        frame = (sint32)((uint32)frame + 1);
    } while (frames > 0);
    if (best_frame >= 0 && !effective_direction)
        best_frame = (sint32)((uint32)(sint32)(sint16)positions.metadata6 - (uint32)best_frame - 1);
    return best_frame;
}

uint32 PathDefinition::FindHeightFrame(const uint32 *position, uint32 direction, sint32 height_offset) const
{
    const PathTransform *captured_path_transform;
    FUNCTION_MARKER(0x800231DC, "SLUS_005.78");
    const MATRIX *matrix;
    PathVector point, left, right, middle, raw, query;
    uint32 *destination;
    uint32 flags, index;
    const PathTransform *matrix_transform;
    uint32 x, y, z, a, b, c, d, dx, dy, dz, distance, left_distance, right_distance;
    uint32 first_distance, best_distance, radius, span, forward, outside, unresolved, move_left;
    sint32 best, remaining, frame, low, high, candidate, ca, cb, c0, c1, c2, c3, c4, c5, c6, c7, c8;
    sint64 product;
    if (positions.keys == 0)
    {
        return 0;
    }
    flags = this->flags;
    if ((flags & 0x10000) != 0)
        return FindLinkedHeightFrame(position, direction, height_offset);
    best = -1;
    best_distance = 0x7FFFF000;
    remaining = (sint16)positions.metadata6;
    frame = 0;
    if (height_offset >= 0)
    {
        radius = (uint32)height_offset;
        span = radius << 1;
        matrix_transform = ((captured_path_transform = (this->transform)) != 0 ? (const PathTransform *)((PathTransform *)(captured_path_transform)) : 0);
        matrix = (matrix_transform != 0 ? &matrix_transform->matrix : (const MATRIX *)0);
        if (matrix_transform != 0)
        {
            c0 = matrix->m[0][0];
            c1 = ((const MATRIX *)((this->transform) != 0 ? &(this->transform)->matrix : (MATRIX *)0))->m[1][0];
            c2 = ((const MATRIX *)((this->transform) != 0 ? &(this->transform)->matrix : (MATRIX *)0))->m[2][0];
            c3 = ((const MATRIX *)((this->transform) != 0 ? &(this->transform)->matrix : (MATRIX *)0))->m[0][1];
            c4 = ((const MATRIX *)((this->transform) != 0 ? &(this->transform)->matrix : (MATRIX *)0))->m[1][1];
            c5 = ((const MATRIX *)((this->transform) != 0 ? &(this->transform)->matrix : (MATRIX *)0))->m[2][1];
            c6 = ((const MATRIX *)((this->transform) != 0 ? &(this->transform)->matrix : (MATRIX *)0))->m[0][2];
            c7 = ((const MATRIX *)((this->transform) != 0 ? &(this->transform)->matrix : (MATRIX *)0))->m[1][2];
            c8 = ((const MATRIX *)((this->transform) != 0 ? &(this->transform)->matrix : (MATRIX *)0))->m[2][2];
            x = 0u - (uint32)((const MATRIX *)((this->transform) != 0 ? &(this->transform)->matrix : (MATRIX *)0))->t[0];
            y = 0u - (uint32)((const MATRIX *)((this->transform) != 0 ? &(this->transform)->matrix : (MATRIX *)0))->t[1];
            z = 0u - (uint32)((const MATRIX *)((this->transform) != 0 ? &(this->transform)->matrix : (MATRIX *)0))->t[2];
            x += position[0];
            y += position[1];
            z += position[2];
            product = (sint64)(sint32)x * c0;
            a = (uint32)((uint64)product >> 12);
            product = (sint64)(sint32)y * c1;
            a += (uint32)((uint64)product >> 12);
            product = (sint64)(sint32)z * c2;
            a += (uint32)((uint64)product >> 12);
            query.words[0] = a;
            product = (sint64)(sint32)x * c3;
            a = (uint32)((uint64)product >> 12);
            product = (sint64)(sint32)y * c4;
            a += (uint32)((uint64)product >> 12);
            product = (sint64)(sint32)z * c5;
            a += (uint32)((uint64)product >> 12);
            query.words[1] = a;
            product = (sint64)(sint32)x * c6;
            a = (uint32)((uint64)product >> 12);
            product = (sint64)(sint32)y * c7;
            a += (uint32)((uint64)product >> 12);
            product = (sint64)(sint32)z * c8;
            a += (uint32)((uint64)product >> 12);
            query.words[2] = a;
            position = query.words;
        }
        do
        {
            SampleVectorTrack(frame, &positions, point.words);
            dx = point.words[0] - position[0];
            point.words[0] = dx;
            if (dx + radius < span)
            {
                dy = point.words[1] - position[1];
                point.words[1] = dy;
                if (dy + radius < span)
                {
                    dz = point.words[2] - position[2];
                    point.words[2] = dz;
                    if (dz + radius < span)
                    {
                        a = (uint32)((sint32)dx >> 6);
                        b = (uint32)((sint32)dy >> 6);
                        c = (uint32)((sint32)dz >> 6);
                        distance = a * a + b * b + c * c;
                        if ((sint32)distance < (sint32)best_distance)
                        {
                            best_distance = distance;
                            best = frame;
                        }
                    }
                }
            }
            remaining = (sint32)((uint32)remaining - 1);
            frame = (sint32)((uint32)frame + 1);
        } while (remaining > 0);
        if (best >= 0 && direction == 0)
            best = (sint32)((uint32)(sint32)(sint16)positions.metadata6 - (uint32)best - 1);
        goto finish;
    }
    radius = (uint32)(sint32)search_radius << 5;
    span = radius << 1;
    if ((flags & 2) != 0)
    {
        first_distance = 0x7FFFF000;
        matrix_transform = ((captured_path_transform = (this->transform)) != 0 ? (const PathTransform *)((PathTransform *)(captured_path_transform)) : 0);
        matrix = (matrix_transform != 0 ? &matrix_transform->matrix : (const MATRIX *)0);
        if (matrix_transform != 0)
        {
            SampleVectorTrack(0, &positions, raw.words);
            PushMatrix();
            x = xport_load_le32((const uint8 *)matrix);
            y = xport_load_le32((const uint8 *)matrix + 4);
            xport_gte_write_control(0, x);
            xport_gte_write_control(1, y);
            x = xport_load_le32((const uint8 *)matrix + 8);
            y = xport_load_le32((const uint8 *)matrix + 12);
            z = xport_load_le32((const uint8 *)matrix + 16);
            xport_gte_write_control(2, x);
            xport_gte_write_control(3, y);
            xport_gte_write_control(4, z);
            ApplyRotMatrixLV(&raw.vector, &left.vector);
            PopMatrix();
            left.words[0] = left.words[0] + (uint32)matrix->t[0];
            left.words[1] = left.words[1] + (uint32)matrix->t[1];
            left.words[2] = left.words[2] + (uint32)matrix->t[2];
        }
        else
        {
            SampleVectorTrack(0, &positions, left.words);
        }
        for (;;)
        {
            matrix_transform = ((captured_path_transform = (this->transform)) != 0 ? (const PathTransform *)((PathTransform *)(captured_path_transform)) : 0);
            matrix = (matrix_transform != 0 ? &matrix_transform->matrix : (const MATRIX *)0);
            if (matrix_transform != 0)
            {
                SampleVectorTrack(frame, &positions, raw.words);
                PushMatrix();
                x = xport_load_le32((const uint8 *)matrix);
                y = xport_load_le32((const uint8 *)matrix + 4);
                xport_gte_write_control(0, x);
                xport_gte_write_control(1, y);
                x = xport_load_le32((const uint8 *)matrix + 8);
                y = xport_load_le32((const uint8 *)matrix + 12);
                z = xport_load_le32((const uint8 *)matrix + 16);
                xport_gte_write_control(2, x);
                xport_gte_write_control(3, y);
                xport_gte_write_control(4, z);
                ApplyRotMatrixLV(&raw.vector, &point.vector);
                PopMatrix();
                point.words[0] = point.words[0] + (uint32)matrix->t[0];
                point.words[1] = point.words[1] + (uint32)matrix->t[1];
                point.words[2] = point.words[2] + (uint32)matrix->t[2];
            }
            else
            {
                SampleVectorTrack(frame, &positions, point.words);
            }
            dx = point.words[0] - position[0];
            dy = point.words[1] - position[1];
            dz = point.words[2] - position[2];
            if (frame == 0)
            {
            repeat_last_point:
                if ((sint32)(dy - 40960) <= 0 || (this->flags & 0x11) != 0)
                    dy = 0;
            }
            if (dx + radius < span && dz + radius < span && ((sint32)(dy - 40960) <= 0 || (this->flags & 0x11) != 0))
            {
                x = left.words[0];
                z = left.words[2];
                a = point.words[0];
                b = point.words[2];
                c = position[0];
                d = position[2];
                product = (sint64)(sint32)(x - c) * (sint32)(x - a);
                x = (uint32)((uint64)product >> 12);
                product = (sint64)(sint32)(z - d) * (sint32)(b - z);
                ca = (sint32)(x - (uint32)((uint64)product >> 12));
                product = (sint64)(sint32)(a - c) * (sint32)(left.words[0] - a);
                x = (uint32)((uint64)product >> 12);
                product = (sint64)(sint32)(b - d) * (sint32)(b - z);
                cb = (sint32)(x - (uint32)((uint64)product >> 12));
                if ((ca ^ cb) < 0 || (uint32)ca + 16 < 33 || (uint32)cb + 16 < 33)
                {
                    a = (uint32)((sint32)dx >> 6);
                    b = (uint32)((sint32)dy >> 6);
                    c = (uint32)((sint32)dz >> 6);
                    distance = a * a + b * b + c * c;
                    if ((sint32)distance < (sint32)best_distance)
                    {
                        best = frame;
                        if (frame != 0)
                            best_distance = distance;
                        else
                            first_distance = distance;
                    }
                }
            }
            x = point.words[0];
            y = point.words[1];
            z = point.words[2];
            left.words[0] = x;
            left.words[1] = y;
            left.words[2] = z;
            remaining = (sint32)((uint32)remaining - 1);
            frame = (sint32)((uint32)frame + 1);
            if (remaining > 0)
                continue;
            if (best > 0)
                break;
            if (remaining != 0)
                break;
            best_distance = first_distance;
            goto repeat_last_point;
        }
        if (best >= 0 && direction == 0)
            best = (sint32)((uint32)(sint32)(sint16)positions.metadata6 - (uint32)best);
        goto finish;
    }
    low = 0;
    high = (sint32)((uint32)remaining - 1);
    for (index = 0; index < 2; ++index)
    {
        candidate = index == 0 ? 0 : high;
        destination = index == 0 ? left.words : right.words;
        matrix_transform = ((captured_path_transform = (this->transform)) != 0 ? (const PathTransform *)((PathTransform *)(captured_path_transform)) : 0);
        matrix = (matrix_transform != 0 ? &matrix_transform->matrix : (const MATRIX *)0);
        if (matrix_transform != 0)
        {
            SampleVectorTrack(candidate, &positions, raw.words);
            PushMatrix();
            x = xport_load_le32((const uint8 *)matrix);
            y = xport_load_le32((const uint8 *)matrix + 4);
            xport_gte_write_control(0, x);
            xport_gte_write_control(1, y);
            x = xport_load_le32((const uint8 *)matrix + 8);
            y = xport_load_le32((const uint8 *)matrix + 12);
            z = xport_load_le32((const uint8 *)matrix + 16);
            xport_gte_write_control(2, x);
            xport_gte_write_control(3, y);
            xport_gte_write_control(4, z);
            ApplyRotMatrixLV(&raw.vector, (VECTOR *)destination);
            PopMatrix();
            destination[0] = destination[0] + (uint32)matrix->t[0];
            destination[1] = destination[1] + (uint32)matrix->t[1];
            destination[2] = destination[2] + (uint32)matrix->t[2];
        }
        else
        {
            SampleVectorTrack(candidate, &positions, destination);
        }
        dx = destination[0] - position[0];
        dy = destination[1] - position[1];
        dz = destination[2] - position[2];
        a = (uint32)((sint32)dx >> 6);
        b = (uint32)((sint32)dz >> 6);
        distance = a * a + b * b;
        if (index == 0)
        {
            left_distance = distance;
            if (distance < 0x4000)
                best = 0;
        }
        else
        {
            right_distance = distance;
            if (distance < 0x4000 && (sint32)distance < (sint32)left_distance)
                best = (sint16)positions.metadata6;
        }
    }
    forward = left_distance < right_distance;
    if (forward != 0)
    {
        if ((sint32)(left_distance << 2) < (sint32)right_distance)
            candidate = high >> 1;
        else
            candidate = (sint32)((uint32)(high >> 2) + (uint32)(high >> 1));
    }
    else if ((sint32)(right_distance << 2) < (sint32)left_distance)
        candidate = high >> 1;
    else
        candidate = high >> 2;
    unresolved = 1;
    do
    {
        matrix_transform = ((captured_path_transform = (this->transform)) != 0 ? (const PathTransform *)((PathTransform *)(captured_path_transform)) : 0);
        matrix = (matrix_transform != 0 ? &matrix_transform->matrix : (const MATRIX *)0);
        if (matrix_transform != 0)
        {
            SampleVectorTrack(candidate, &positions, raw.words);
            PushMatrix();
            x = xport_load_le32((const uint8 *)matrix);
            y = xport_load_le32((const uint8 *)matrix + 4);
            xport_gte_write_control(0, x);
            xport_gte_write_control(1, y);
            x = xport_load_le32((const uint8 *)matrix + 8);
            y = xport_load_le32((const uint8 *)matrix + 12);
            z = xport_load_le32((const uint8 *)matrix + 16);
            xport_gte_write_control(2, x);
            xport_gte_write_control(3, y);
            xport_gte_write_control(4, z);
            ApplyRotMatrixLV(&raw.vector, &middle.vector);
            PopMatrix();
            middle.words[0] = middle.words[0] + (uint32)matrix->t[0];
            middle.words[1] = middle.words[1] + (uint32)matrix->t[1];
            middle.words[2] = middle.words[2] + (uint32)matrix->t[2];
        }
        else
        {
            SampleVectorTrack(candidate, &positions, middle.words);
        }
        destination = forward != 0 ? left.words : right.words;
        x = destination[0];
        z = destination[2];
        a = middle.words[0];
        b = middle.words[2];
        c = position[0];
        d = position[2];
        product = (sint64)(sint32)(x - c) * (sint32)(x - a);
        y = (uint32)((uint64)product >> 12);
        product = (sint64)(sint32)(z - d) * (sint32)(b - z);
        ca = (sint32)(y - (uint32)((uint64)product >> 12));
        product = (sint64)(sint32)(a - c) * (sint32)(x - a);
        y = (uint32)((uint64)product >> 12);
        product = (sint64)(sint32)(b - d) * (sint32)(b - z);
        cb = (sint32)(y - (uint32)((uint64)product >> 12));
        outside = (ca ^ cb) < 0 || (ca > 0 ? ca < cb : cb < ca || (ca == 0 && ca < cb));
        if (outside != 0 && unresolved != 0)
            unresolved = (ca ^ cb) > 0 && ca != 0 && cb != 0;
        move_left = forward != 0 ? outside == 0 : outside != 0;
        x = middle.words[0];
        y = middle.words[1];
        z = middle.words[2];
        if (move_left != 0)
        {
            left.words[0] = x;
            left.words[1] = y;
            left.words[2] = z;
            low = candidate;
        }
        else
        {
            right.words[0] = x;
            right.words[1] = y;
            right.words[2] = z;
            high = candidate;
        }
        candidate = (sint32)((uint32)low + (uint32)high) >> 1;
    } while (low != candidate);
    if (unresolved == 0 && (forward != 0 ? high < (sint16)positions.metadata6 - 1 : low > 0))
    {
        dx = right.words[0] - position[0];
        dy = right.words[1] - position[1];
        dz = right.words[2] - position[2];
        if (dx + radius < span && dz + radius < span && ((sint32)(dy - 40960) <= 0 || (this->flags & 0x11) != 0))
        {
            if (direction != 0)
                high = (sint32)((uint32)high - 1);
            if (high <= 0)
                best = 1;
            else
            {
                best = high;
                remaining = (sint16)positions.metadata6;
                if (high >= remaining)
                    best = remaining - 1;
            }
        }
        else
        {
            dx = left.words[0] - position[0];
            dy = left.words[1] - position[1];
            dz = left.words[2] - position[2];
            if (dx + radius < span && dz + radius < span && ((sint32)(dy - 40960) <= 0 || (this->flags & 0x11) != 0))
            {
                if (low <= 0)
                    low = 1;
                else
                {
                    remaining = (sint16)positions.metadata6;
                    if (low >= remaining)
                        low = remaining - 1;
                }
                best = low;
            }
        }
    }
    if (best >= 0)
    {
        if (direction == 0)
            best = (sint32)((uint32)(sint32)(sint16)positions.metadata6 - (uint32)best);
        remaining = (sint16)positions.metadata6;
        if (best == remaining - 1)
            best = remaining - 2;
    }
finish:
    return best;
}

uint32 AnimatedObject::CanChangePath(const uint32 *position, uint32 direction) const
{
    FUNCTION_MARKER(0x8003D190, "SLUS_005.78");
    PathVector floor;
    const ScenePath *path, *path_view;
    uint32 scene, flags, x, y, z, result;
    sint32 frame;
    scene = OriginalDefaults::SceneContextIdentity;
    flags = this->path_flags;
    path = camera_motion.path;
    path_view = (const ScenePath *)(path);
    if ((flags & 6) == 2)
    {
        return 0;
    }
    if ((flags & 4) && ReadCharacterConstrainedPath() != path)
    {
        frame = ((const PathDefinition *)((const PathDefinition *)((path_view->definition))))->FindHeightFrame(position, direction, -40960);
        if (frame != -1)
        {
            frame = ((const PathDefinition *)((const PathDefinition *)((path_view->definition))))->ProjectFrame(frame, position, floor.words, direction, 4096);
        }
        if (frame >= 0)
        {
            x = floor.words[0];
            x -= position[0];
            if (x + 24576 > 0xC000)
                frame = -1;
            else
            {
                z = floor.words[2];
                z -= position[2];
                if (z + 24576 > 0xC000)
                    frame = -1;
            }
        }
        if (frame != -1)
        {
            x = floor.words[0];
            x -= position[0];
            y = floor.words[1];
            floor.words[0] = x;
            y -= position[1];
            z = floor.words[2];
            floor.words[1] = y;
            z -= position[2];
            floor.words[2] = z;
            if ((sint32)y <= 0)
            {
                return 0;
            }
        }
    }
    result = 1;
    if ((this->path_flags & 0x20) && (IsCharacterAnchorPathActive() & 0xFF) == 0)
    {
        frame = ((const PathDefinition *)((const PathDefinition *)((path_view->definition))))->FindHeightFrame(position, direction, -40960);
        if (frame != -1)
        {
            frame = ((const PathDefinition *)((const PathDefinition *)((path_view->definition))))->ProjectFrame(frame, position, floor.words, direction, 4096);
        }
        if (frame >= 0)
        {
            x = floor.words[0];
            x -= position[0];
            if (x + 24576 > 0xC000)
                frame = -1;
            else
            {
                z = floor.words[2];
                z -= position[2];
                if (z + 24576 > 0xC000)
                    frame = -1;
            }
        }
        if (frame != -1)
        {
            x = floor.words[0];
            x -= position[0];
            y = floor.words[1];
            floor.words[0] = x;
            y -= position[1];
            z = floor.words[2];
            floor.words[1] = y;
            z -= position[2];
            floor.words[2] = z;
            if ((sint32)y <= 0)
                result = 0;
        }
    }
    return result;
}

const ScenePath *ScenePath::SelectAdjacent(uint32 *frame_word, uint32 direction, uint32 flags) const
{
    FUNCTION_MARKER(0x8003ADC0, "SLUS_005.78");
    const ScenePath *current;
    const ScenePath *selected;
    const ScenePath *selected_view;
    const PathDefinition *definition;
    uint32 old_frame, mask, first_frame;
    old_frame = *frame_word;
    // Forced transitions use the same frame side as ordinary transitions
    first_frame = (sint32)old_frame >= 0;
    if ((flags & 0x2000000) != 0)
    {
        mask = node.state_flags;
        if ((mask & (((flags >> 6) & 0x10) | 8)) != 0)
            selected = (this->adjacent[1]);
        else
            selected = (this->adjacent[0]);
        if (selected != 0)
        {
            if (first_frame != 0)
                *frame_word = 0;
            else
            {
                definition = (const PathDefinition *)((const PathDefinition *)((selected->definition)));
                *frame_word = (uint32)(sint16)definition->positions.metadata6 - 1;
            }
        }
    }
    else
    {
        current = this;
        if (first_frame == 0)
        {
            if ((node.state_flags & 8) != 0)
            {
                current = (this->adjacent[0]);
                current = (const ScenePath *)(current);
            }
            direction ^= 0x10;
        }
        mask = current->node.state_flags;
        if ((mask & (direction | 8)) != 0)
            selected = (current->adjacent[1]);
        else
            selected = (current->adjacent[0]);
        if (selected != 0)
        {
            if (first_frame != 0)
                *frame_word = 0;
            else
            {
                if ((selected->node.state_flags & 8) != 0)
                {
                    selected = (selected->adjacent[0]);
                }
                selected_view = (const ScenePath *)(selected);
                definition = (const PathDefinition *)((const PathDefinition *)((selected_view->definition)));
                *frame_word = (uint32)(sint16)definition->positions.metadata6 - 1;
            }
        }
    }
    if (selected != 0)
    {
        definition = (const PathDefinition *)((const PathDefinition *)((selected->definition)));
        if ((definition->flags & 0x10000) != 0)
        {
            *frame_word = old_frame;
            selected = 0;
        }
    }
    return selected;
}

void AnimatedObject::ApplyPathVelocityChange(const PathDefinition *definition)
{
    const PathTransform *captured_path_transform;
    FUNCTION_MARKER(0x80046228, "SLUS_005.78");
    const PathDefinition *previous_definition;
    const PathTransform *transform;
    const ScenePath *parent;
    uint32 x, y, z, value, speed, angle, flags, secondary;
    const PathTransform *matrix_transform;
    sint32 kind;

    if ((this->flags & 0x2000) != 0)
    {
        return;
    }
    kind = (sint8)this->metadata126[0];
    parent = 0;
    if (kind == 1)
    {
        if (this->behavior != 33)
        {
            parent = camera_motion.path;
        }
        else
            parent = (this->scene_path);
    }
    else if (kind == 3)
    {
        if (this->animation_binding.tracks[0] == -1)
            parent = (this->scene_path);
    }
    if (parent == 0)
    {
        return;
    }
    previous_definition = (parent->definition);
    matrix_transform = ((captured_path_transform = (previous_definition->transform)) != 0 ? (const PathTransform *)((PathTransform *)(captured_path_transform)) : 0);
    if (matrix_transform == 0)
    {
        return;
    }
    transform = ((const PathTransform *)(matrix_transform));
    x = transform->displacement[0];
    y = transform->displacement[1];
    z = transform->displacement[2];
    if (definition != 0)
    {
        matrix_transform = ((captured_path_transform = (definition->transform)) != 0 ? (const PathTransform *)((PathTransform *)(captured_path_transform)) : 0);
        if (matrix_transform != 0)
        {
            transform = ((const PathTransform *)(matrix_transform));
            x -= transform->displacement[0];
            matrix_transform = ((captured_path_transform = (definition->transform)) != 0 ? (const PathTransform *)((PathTransform *)(captured_path_transform)) : 0);
            transform = ((const PathTransform *)(matrix_transform));
            y -= transform->displacement[1];
            matrix_transform = ((captured_path_transform = (definition->transform)) != 0 ? (const PathTransform *)((PathTransform *)(captured_path_transform)) : 0);
            transform = ((const PathTransform *)(matrix_transform));
            z -= transform->displacement[2];
        }
    }
    this->velocity[0] = this->velocity[0] + x;
    this->velocity[1] = this->velocity[1] + y;
    this->velocity[2] = this->velocity[2] + z;
    if ((sint8)this->metadata126[0] != 1)
    {
        return;
    }
    if (x == 0 && y == 0 && z == 0)
    {
        return;
    }
    if (y != 0)
    {
        value = character_controller.motion.vertical_speed;
        value += y;
        y = 0;
        character_controller.motion.vertical_speed = value;
        this->flags = this->flags | 0x400;
    }
    value = (uint32)((sint32)x >> 6);
    value *= value;
    speed = (uint32)((sint32)y >> 6);
    value += speed * speed;
    speed = (uint32)((sint32)z >> 6);
    value += speed * speed;
    if ((sint32)value < 0)
    {
        speed = SquareRoot12((sint32)(value >> 2)) << 1;
    }
    else
    {
        speed = SquareRoot12((sint32)value);
    }

    if (speed != 0)
    {
        angle = Atan2Angle((sint32)z, (sint32)x);
        value = (uint32)(sint32)this->target_yaw;
        angle = (uint32)(sint32)(sint16)angle - value - 1024;
        if ((angle & 0xFFF) < 2048)
        {
            if (speed != 0 && (this->flags & 0x400) == 0)
                speed += 4096;
        }
        else
            speed = 0 - speed;
    }
    value = character_controller.motion.horizontal_speed;
    flags = character_controller.flags;
    secondary = character_controller.secondary_flags;
    character_controller.motion.horizontal_speed = value + speed;
    character_controller.flags = flags & 0xFF7FFFFF;
    character_controller.secondary_flags = secondary & 0xFFFFFF7F;
    return;
}

uint32 AnimatedObject::ChangeActorPath(const ScenePath *path, const uint32 *position, uint32 *floor)
{
    const PathTransform *captured_path_transform;
    const ScenePath *adjacent_path;
    FUNCTION_MARKER(0x8003AF18, "SLUS_005.78");
    const MATRIX *matrix;
    const PathDefinition *parent;
    const ScenePath *path_view;
    const AnimatedObject *player;
    uint32 frame_word;
    PathVector query_floor, input, inverse;
    MATRIX transpose = {0};
    const ScenePath *visited[6];
    /* No candidate exists until this function selects a path */
    const ScenePath *candidate = 0, *visited_path;
    const ScenePath *initial_path = path;
    uint32 direction, flags;
    const PathDefinition *definition;
    const PathTransform *matrix_transform;
    uint32 kind_one_probe = 0, restrict_paths = 0, allow_special = 0, allow_link = 0;
    uint32 fallback_used = 0, result = 0, scene, x, y, z, value, index, repeated;
    sint32 kind, frame, probe_frame, retries = 2, previous_retries, gap;
    sint32 probe;
    xport_store_le16((uint8 *)&transpose + 18, 0);
    kind = (sint8)this->metadata126[0];
    if (kind == 1)
        kind_one_probe = (character_controller.flags & 0x60000) != 0;
    else if (this->movement_mode == 2 && (this->state == 19 || path_routing.flags != 0))
    {
        player = FindPlayerObject(0);
        parent = (const PathDefinition *)((const PathDefinition *)((player->path_definition)));
        value = parent->flags & 0x80000;
        restrict_paths = 1;
        if (value != 0)
            allow_special = 1;
        if ((((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->flags & 0x10000) != 0)
        {
            allow_link = 1;
        }
    }
    direction = this->flags & 0x10;
    if ((this->path_flags & 0x26) != 0)
    {
        probe = CanChangePath(position, direction);
        if (((uint32)probe & 0xFF) == 0)
            goto done;
    }
    for (index = 0; index < 6; ++index)
        visited[index] = 0;
select_path:
    path_view = (const ScenePath *)(path);
    frame = -1;
    if ((this->flags & 0x40) != 0)
        frame = this->path_frame;
    frame_word = frame;
    kind = (sint8)this->metadata126[0];
    if (kind == 1 || (kind == 3 && this->animation_binding.tracks[0] == -1))
    {
        if ((((const PathDefinition *)((const PathDefinition *)((path_view->definition))))->flags & 0x50) >= 0x41 && ((this->flags & 0x4800) == 0 || retries < 2))
        {
            candidate = path_view->fallback;
            if (candidate != 0)
            {
                if ((path_view->node.state_flags & (direction | 8)) == 0 && (candidate->node.state_flags & 8) != 0)
                {
                    adjacent_path = (candidate->adjacent[0]);
                    candidate = adjacent_path;
                }
                if (candidate != 0)
                    goto check_candidate;
            }
        }
        candidate = path_view->SelectAdjacent(&frame_word, direction, this->path_flags);
        if (candidate != 0)
            goto check_candidate;
    }
    if (candidate == 0)
        goto fallback;
    if (this->behavior != 6 || this->state != 0 || path_routing.flags == 0 || fallback_used != 0)
        goto done;
    if ((((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->flags & 0x10000) == 0)
        goto done;
    adjacent_path = (camera_motion.path->adjacent[1]);
    fallback_used = 1;
    if (adjacent_path == 0)
        goto done;
    candidate = adjacent_path;
    goto check_candidate;
fallback:
    if (fallback_used == 1)
        goto done;
    fallback_used = 1;
    candidate = FindFallbackPath(position, direction);
    if (candidate == 0)
        goto done;
check_candidate:
    flags = this->path_flags;
    this->path_flags = flags & 0xFDFFFFFF;
    if ((flags & 0x20) != 0 && camera_motion.path == candidate)
        goto fallback;
    if (candidate != initial_path && candidate != 0)
    {
        repeated = 0;
        for (index = 0; index < 6; ++index)
        {
            visited_path = visited[index];
            if (candidate == visited_path)
            {
                repeated = 1;
                break;
            }
        }
        if (repeated != 0 && path_routing.reject_repeated != 0)
            goto fallback;
        for (index = 0; index < 6; ++index)
        {
            visited_path = visited[index];
            if (visited_path == 0)
            {
                visited[index] = candidate;
                break;
            }
        }
    }
    flags = ((const PathDefinition *)((const PathDefinition *)((path_view->definition))))->flags;
    if ((flags & 0x50) == 0 && retries >= 2)
        goto accept_path;
    if ((this->flags & 0x6000) == 0x4000)
    {
        probe = ((const PathDefinition *)((const PathDefinition *)((candidate->definition))))->ProjectFrame((sint32)frame_word, position, query_floor.words, direction, 4096);
        frame_word = probe;
        if (probe < 0)
            goto done;
        goto check_floor;
    }
    if (((flags & 0x40) == 0 || candidate == path) && retries == 2)
    {
        probe = candidate->SamplePosition((sint32)frame_word, query_floor.words, direction);
        goto floor_ready;
    }
    if ((this->flags & 0x2000) == 0 || (this->path_flags & 0x40000) != 0)
    {
        probe = ((const PathDefinition *)((const PathDefinition *)((candidate->definition))))->FindHeightFrame(position, direction, -40960);
        frame_word = probe;
    }
    if ((this->path_flags & 0x24) != 0 && (sint32)frame_word == -1)
    {
        probe = ((const PathDefinition *)((const PathDefinition *)((candidate->definition))))->FindHeightFrame(position, direction, -40960);
        if (probe != -1)
        {
            probe = ((const PathDefinition *)((const PathDefinition *)((candidate->definition))))->ProjectFrame(probe, position, query_floor.words, direction, 8192);
            frame_word = probe;
        }
    }
    if (kind_one_probe != 0 && (sint32)frame_word == -1)
    {
        probe = ((const PathDefinition *)((const PathDefinition *)((candidate->definition))))->FindHeightFrame(position, direction, -40960);
        if (probe != -1)
        {
            probe = ((const PathDefinition *)((const PathDefinition *)((candidate->definition))))->ProjectFrame(probe, position, query_floor.words, direction, 20480);
            frame_word = probe;
        }
    }
    frame = (sint32)frame_word;
    if (frame < 0)
        probe_frame = -1;
    else
    {
        probe = ((const PathDefinition *)((const PathDefinition *)((candidate->definition))))->ProjectFrame(frame, position, query_floor.words, direction, 4096);
        probe_frame = probe;
        if ((this->path_flags & 0x24) != 0 && probe_frame == -1)
        {
            probe = ((const PathDefinition *)((const PathDefinition *)((candidate->definition))))->FindHeightFrame(position, direction, -40960);
            if (probe != -1)
            {
                probe = ((const PathDefinition *)((const PathDefinition *)((candidate->definition))))->ProjectFrame(probe, position, query_floor.words, direction, 8192);
                probe_frame = probe;
            }
        }
        if (probe_frame >= 0 && (query_floor.words[0] - position[0] + 24576 > 0xC000 || query_floor.words[2] - position[2] + 24576 > 0xC000))
            probe_frame = -1;
    }
    frame_word = probe_frame;
floor_ready:
    if ((sint32)frame_word < 0)
        goto retry_path;
check_floor:
    if ((((const PathDefinition *)((const PathDefinition *)((candidate->definition))))->flags & 0x10000) != 0)
        goto retry_path;
    gap = (sint32)(position[1] - query_floor.words[1]);
    if (gap < -1365 && ((((const PathDefinition *)((const PathDefinition *)((path_view->definition))))->flags & 0x40) == 0 || gap < -4096))
        goto retry_path;
    x = query_floor.words[0];
    y = query_floor.words[1];
    z = query_floor.words[2];
    floor[0] = x;
    floor[1] = y;
    floor[2] = z;
    goto accept_path;
retry_path:
    if (retries != 0)
    {
        flags = this->flags;
        if ((flags & 0x4000) == 0)
        {
            previous_retries = retries--;
            if (previous_retries != 2 || (((const PathDefinition *)((const PathDefinition *)((path_view->definition))))->flags & 0x10) == 0 || (flags & 0x800) == 0 || path_view->fallback == 0)
                path = candidate;
            goto select_path;
        }
    }
    if (fallback_used == 0)
        goto fallback;
    goto done;
accept_path:
    if (candidate == initial_path && ((this->flags & 0x800) == 0 || retries != 2))
        goto done;
    flags = ((const PathDefinition *)((const PathDefinition *)((candidate->definition))))->flags;
    if ((flags & 0x10000) != 0)
        goto done;
    if (restrict_paths != 0)
    {
        if ((flags & 0x80000) != 0 && allow_special == 0)
            goto done;
        if (allow_link == 0 && (((const PathDefinition *)((const PathDefinition *)((candidate->definition))))->flags & 0x10) != 0)
            goto done;
    }
    flags = this->flags;
    kind = (sint8)this->metadata126[0];
    this->flags = flags & 0xFFFF9FFF;
    if (kind == 1 && this->behavior != 33)
    {
        scene = OriginalDefaults::SceneContextIdentity;
        x = position[0];
        y = position[1];
        z = position[2];
        camera_motion.focus_position[0] = x;
        camera_motion.focus_position[1] = y;
        camera_motion.focus_position[2] = z;
        UpdateObjectScenePath(candidate, (sint32)frame_word);
    }
    else
    {
        if (kind != 1)
            ApplyPathVelocityChange((candidate->definition));
        this->scene_path = candidate;
    }
    this->path_frame = (sint16)(frame_word);
    parent = (this->path_definition);
    if (parent != 0 && (parent->transform) != 0)
    {
        x = this->position[0];
        y = this->position[1];
        z = this->position[2];
        input.words[0] = x;
        input.words[1] = y;
        input.words[2] = z;
        PushMatrix();
        matrix = (const MATRIX *)((((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) != 0 ? &(((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform)->matrix : (MATRIX *)0);
        x = xport_load_le32((const uint8 *)matrix);
        y = xport_load_le32((const uint8 *)matrix + 4);
        xport_gte_write_control(0, x);
        xport_gte_write_control(1, y);
        x = xport_load_le32((const uint8 *)matrix + 8);
        y = xport_load_le32((const uint8 *)matrix + 12);
        z = xport_load_le32((const uint8 *)matrix + 16);
        xport_gte_write_control(2, x);
        xport_gte_write_control(3, y);
        xport_gte_write_control(4, z);
        ApplyRotMatrixLV(&input.vector, (VECTOR *)this->position);
        PopMatrix();
        parent = (const PathDefinition *)((const PathDefinition *)((this->path_definition)));
        matrix = (const MATRIX *)((parent->transform) != 0 ? &(parent->transform)->matrix : (MATRIX *)0);
        x = this->position[0];
        value = matrix->t[0];
        parent = (const PathDefinition *)((const PathDefinition *)((this->path_definition)));
        this->position[0] = x + value;
        matrix = (const MATRIX *)((parent->transform) != 0 ? &(parent->transform)->matrix : (MATRIX *)0);
        y = this->position[1];
        value = matrix->t[1];
        parent = (const PathDefinition *)((const PathDefinition *)((this->path_definition)));
        this->position[1] = y + value;
        matrix = (const MATRIX *)((parent->transform) != 0 ? &(parent->transform)->matrix : (MATRIX *)0);
        z = this->position[2];
        value = matrix->t[2];
        this->position[2] = z + value;
    }
    else
    {
        x = this->position[0];
        y = this->position[1];
        z = this->position[2];
        this->position[0] = x;
        this->position[1] = y;
        this->position[2] = z;
    }
    definition = (candidate->definition);
    this->path_definition = definition;
    if (definition != 0 && (matrix_transform = ((captured_path_transform = (((const PathDefinition *)((const PathDefinition *)(definition)))->transform)) != 0 ? (const PathTransform *)((PathTransform *)(captured_path_transform)) : 0)) != 0)
    {
        matrix = (matrix_transform != 0 ? &matrix_transform->matrix : (const MATRIX *)0);
        x = this->position[0];
        inverse.words[0] = x - matrix->t[0];
        matrix = (const MATRIX *)((((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) != 0 ? &(((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform)->matrix : (MATRIX *)0);
        y = this->position[1];
        inverse.words[1] = y - matrix->t[1];
        matrix = (const MATRIX *)((((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) != 0 ? &(((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform)->matrix : (MATRIX *)0);
        z = this->position[2];
        inverse.words[2] = z - matrix->t[2];
        for (index = 0; index < 9; ++index)
        {
            matrix = (const MATRIX *)((((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) != 0 ? &(((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform)->matrix : (MATRIX *)0);
            value = (uint16)matrix->m[index % 3][index / 3];
            transpose.m[index / 3][index % 3] = value;
        }
        PushMatrix();
        x = xport_load_le32((const uint8 *)&transpose + 0);
        y = xport_load_le32((const uint8 *)&transpose + 4);
        xport_gte_write_control(0, x);
        xport_gte_write_control(1, y);
        x = xport_load_le32((const uint8 *)&transpose + 8);
        y = xport_load_le32((const uint8 *)&transpose + 12);
        z = xport_load_le32((const uint8 *)&transpose + 16);
        xport_gte_write_control(2, x);
        xport_gte_write_control(3, y);
        xport_gte_write_control(4, z);
        ApplyRotMatrixLV(&inverse.vector, (VECTOR *)this->position);
        PopMatrix();
    }
    else
    {
        x = this->position[0];
        y = this->position[1];
        z = this->position[2];
        this->position[0] = x;
        this->position[1] = y;
        this->position[2] = z;
    }
    result = 1;
done:
    return result;
}

uint32 SelectScenePath(const ScenePath *path)
{
    FUNCTION_MARKER(0x80024844, "SLUS_005.78");
    uint32 context = OriginalDefaults::SceneContextIdentity;

    if (camera_projection.view_normals[0].vz != 4096)
        SetCameraProjection(0xE9555);
    context = OriginalDefaults::SceneContextIdentity;
    camera_motion.path = path;
    return context;
}

uint32 ScenePath::ProjectOffset(sint32 frame, uint32 *destination, uint32 direction) const
{
    FUNCTION_MARKER(0x800451CC, "SLUS_005.78");
    PathVector point, previous, following;
    uint32 x, y, z, numerator, denominator, ratio, scale, result, i;
    sint32 selected, count, gap, reciprocal, lift;
    selected = ((const PathDefinition *)((const PathDefinition *)((this->definition))))->ProjectFrame(frame, camera_motion.focus_position, point.words, direction, 4096);
    if (selected < 0)
        selected = frame;
    gap = (sint32)(camera_motion.focus_position[1] - point.words[1]);
    if (selected <= 0)
    {
        SamplePosition(0, point.words, direction);

        SampleOffset(0, destination, direction);
    }
    else
    {
        count = (sint16)((const PathDefinition *)((const PathDefinition *)((this->definition))))->positions.metadata6;
        if (selected < count)
        {
            SamplePosition((sint32)((uint32)selected - 1), previous.words, direction);

            x = point.words[0] - previous.words[0];
            y = point.words[1] - previous.words[1];
            z = point.words[2] - previous.words[2];
            point.words[0] = x;
            point.words[1] = y;
            point.words[2] = z;
            x = (uint32)((sint32)x >> 6);
            y = (uint32)((sint32)y >> 6);
            z = (uint32)((sint32)z >> 6);

            numerator = x * x + y * y + z * z;
            SampleOffset((sint32)((uint32)selected - 1), destination, direction);

            SamplePosition(selected, following.words, direction);

            x = following.words[0] - previous.words[0];
            y = following.words[1] - previous.words[1];
            z = following.words[2] - previous.words[2];
            following.words[0] = x;
            following.words[1] = y;
            following.words[2] = z;
            x = (uint32)((sint32)x >> 6);
            y = (uint32)((sint32)y >> 6);
            z = (uint32)((sint32)z >> 6);
            denominator = x * x + y * y + z * z;
            if (denominator == 0)
            {
                SampleOffset(selected, destination, direction);
            }
            else
            {
                reciprocal = 0x40000000 / (sint32)denominator;
                ratio = (uint32)(((sint64)(sint32)numerator * reciprocal) >> 18);
                fixed_math_result = ratio;
                SampleOffset(selected, point.words, direction);

                for (i = 0; i < 3; ++i)
                    point.words[i] = point.words[i] - destination[i];
                ratio = fixed_math_result;
                if ((sint32)ratio >= 0)
                {
                    scale = SquareRoot12((sint32)ratio);
                }
                else
                {
                    scale = SquareRoot12((sint32)(ratio >> 2)) << 1;
                }

                for (i = 0; i < 3; ++i)
                    point.words[i] = (uint32)(((sint64)(sint32)point.words[i] * (sint32)scale) >> 12);
                for (i = 0; i < 3; ++i)
                    destination[i] = destination[i] + point.words[i];
            }
        }
        else
        {
            SamplePosition(count - 1, point.words, direction);

            SampleOffset((sint16)((const PathDefinition *)((const PathDefinition *)((this->definition))))->positions.metadata6 - 1, destination, direction);
        }
    }
    result = node.state_flags & 1;
    if (result != 0)
    {
        result = 5120;
        if (gap > 0)
        {
            lift = (sint32)(uint32)(((sint64)gap * 5120) >> 12);
            if ((sint32)((uint32)lift - (uint32)gap) > 61440)
                lift = (sint32)((uint32)gap + 61440);
            result = destination[1] + (uint32)lift;
            destination[1] = result;
        }
    }
    return result;
}

void AnimatedObject::UpdateObjectScenePath(const ScenePath *path, sint32 frame)
{
    FUNCTION_MARKER(0x80046510, "SLUS_005.78");
    const ScenePath *path_view, *previous;
    uint32 direction;
    uint32 same_definition;
    const PathDefinition *definition;
    sint32 previous_frame;
    previous = camera_motion.path;
    if (path != previous)
    {
        path_view = (const ScenePath *)(path);
        direction = this->flags & 0x10;
        path_view->ProjectOffset(frame, camera_motion.desired_position, direction);
        previous = camera_motion.path;
        definition = path_view->definition;
        same_definition = definition == previous->definition;
        if (same_definition != 0)
            direction ^= 0x10;
        else
            ApplyPathVelocityChange(definition);
        if ((camera_motion.smooth_ticks[0] | camera_motion.smooth_ticks[1] | camera_motion.smooth_ticks[2]) == 0)
        {
            previous = camera_motion.path;
            if (previous == character_controller.path && (this->flags & 0x2000) == 0)
            {
                previous_frame = this->path_frame;
                if (previous_frame >= 0)
                    ((const ScenePath *)(previous))->ProjectOffset(previous_frame, camera_motion.position, direction);
            }
        }
        SelectScenePath(path);
    }
    this->path_frame = (sint16)(frame);
}

uint32 AnimatedObject::ReverseScenePath()
{
    uint32 *support_speed;
    FUNCTION_MARKER(0x80046CBC, "SLUS_005.78");
    const MATRIX *matrix;
    const PathDefinition *parent;
    const ScenePath *path, *path_view, *previous;
    uint32 state, result, flags, x, y, z, frame, old_frame, heading;
    PathVector local, delta;
    uint32 direction;
    const PathDefinition *definition, *captured_parent, *previous_definition;
    sint32 signed_frame;

    state = OriginalDefaults::SceneContextIdentity;
    path = camera_motion.path;
    path_view = (const ScenePath *)(path);
    result = path_view->node.state_flags & 8;
    if (result != 0)
    {
        path = (path_view->adjacent[0]);
    }
    if (path == 0)
    {
        return result;
    }
    result = character_controller.secondary_flags & 0x80;
    if (result != 0)
    {
        return result;
    }
    this->flags = this->flags ^ 0x10;
    support_speed = character_controller.support.speed;
    x = support_speed[0];
    y = character_controller.motion.horizontal_speed;
    support_speed[0] = 0 - x;
    character_controller.motion.horizontal_speed = 0 - y;
    heading = (uint16)this->target_yaw;
    captured_parent = (this->path_definition);
    parent = (const PathDefinition *)(captured_parent);
    old_frame = (uint16)this->path_frame;
    this->target_yaw = (sint16)((heading + 2048) & 0xFFF);
    frame = parent->positions.metadata6;
    flags = this->flags;
    frame -= old_frame;
    if ((flags & 0x2000) != 0)
    {
        x = (uint32)character_controller.path_tangent.vx;
        y = (uint32)character_controller.path_tangent.vy;
        z = (uint32)character_controller.path_tangent.vz;
        character_controller.path_tangent.vx = 0 - x;
        character_controller.path_tangent.vy = 0 - y;
        character_controller.path_tangent.vz = 0 - z;
        x = this->position[0];
        y = this->position[1];
        z = this->position[2];
        camera_motion.focus_position[0] = x;
        camera_motion.focus_position[1] = y;
        camera_motion.focus_position[2] = z;
    }
    else if (captured_parent != 0 && (parent->transform) != 0)
    {
        x = this->position[0];
        y = this->position[1];
        z = this->position[2];
        local.words[0] = x;
        local.words[1] = y;
        local.words[2] = z;
        PushMatrix();
        matrix = (const MATRIX *)((((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) != 0 ? &(((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform)->matrix : (MATRIX *)0);
        x = xport_load_le32((const uint8 *)matrix);
        y = xport_load_le32((const uint8 *)matrix + 4);
        xport_gte_write_control(0, x);
        xport_gte_write_control(1, y);
        x = xport_load_le32((const uint8 *)matrix + 8);
        y = xport_load_le32((const uint8 *)matrix + 12);
        z = xport_load_le32((const uint8 *)matrix + 16);
        xport_gte_write_control(2, x);
        xport_gte_write_control(3, y);
        xport_gte_write_control(4, z);
        state = OriginalDefaults::SceneContextIdentity;
        ApplyRotMatrixLV(&local.vector, (VECTOR *)camera_motion.focus_position);
        PopMatrix();

        state = OriginalDefaults::SceneContextIdentity;
        matrix = (const MATRIX *)((((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) != 0 ? &(((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform)->matrix : (MATRIX *)0);
        x = camera_motion.focus_position[0];
        x += matrix->t[0];
        camera_motion.focus_position[0] = x;
        matrix = (const MATRIX *)((((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) != 0 ? &(((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform)->matrix : (MATRIX *)0);
        y = camera_motion.focus_position[1];
        y += matrix->t[1];
        camera_motion.focus_position[1] = y;
        matrix = (const MATRIX *)((((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) != 0 ? &(((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform)->matrix : (MATRIX *)0);
        z = camera_motion.focus_position[2];
        z += matrix->t[2];
        camera_motion.focus_position[2] = z;
    }
    else
    {
        state = OriginalDefaults::SceneContextIdentity;
        x = this->position[0];
        y = this->position[1];
        z = this->position[2];
        camera_motion.focus_position[0] = x;
        camera_motion.focus_position[1] = y;
        camera_motion.focus_position[2] = z;
    }
    state = OriginalDefaults::SceneContextIdentity;
    previous = camera_motion.path;
    signed_frame = (sint16)frame;
    if (path != previous)
    {
        direction = this->flags & 0x10;
        path->ProjectOffset(signed_frame, (uint32 *)camera_motion.desired_position, direction);
        state = OriginalDefaults::SceneContextIdentity;
        previous = camera_motion.path;
        definition = (path->definition);
        previous_definition = (previous->definition);
        if (definition == previous_definition)
            direction ^= 0x10;
        else
            ApplyPathVelocityChange(definition);
        state = OriginalDefaults::SceneContextIdentity;
        if ((camera_motion.smooth_ticks[0] | camera_motion.smooth_ticks[1] | camera_motion.smooth_ticks[2]) == 0)
        {
            previous = camera_motion.path;
            if (previous == character_controller.path && (this->flags & 0x2000) == 0)
            {
                signed_frame = this->path_frame;
                if (signed_frame >= 0)
                    ((const ScenePath *)(previous))->ProjectOffset(signed_frame, (uint32 *)camera_motion.position, direction);
            }
        }
        SelectScenePath(path);
    }
    this->path_frame = (sint16)(frame);
    if (path != character_controller.path)
    {
        character_controller.path = path;
        state = OriginalDefaults::SceneContextIdentity;
        x = camera_motion.desired_position[0];
        x -= camera_motion.position[0];
        delta.words[0] = x;
        y = camera_motion.desired_position[1];
        y -= camera_motion.position[1];
        delta.words[1] = y;
        z = camera_motion.desired_position[2];
        z -= camera_motion.position[2];
        delta.words[2] = z;
        SetCameraTransition(delta.words);
    }
    result = OriginalDefaults::SceneContextIdentity;
    camera_motion.frame = frame;
    return result;
}

void PathDefinition::SampleTangent(VECTOR *destination, sint32 direction) const
{
    FUNCTION_MARKER(0x80022EC4, "SLUS_005.78");
    VECTOR vector;
    const VectorAnimationKey *first_key, *second_key;
    const MATRIX *matrix;
    uint32 index, value, i;
    sint32 x, y, z;
    for (i = 0; i < 3; ++i)
    {
        if (direction != 0)
        {
            index = positions.key_count;
            first_key = (const VectorAnimationKey *)positions.keys + index - 1;
            second_key = (const VectorAnimationKey *)positions.keys + index - 2;
            value = first_key->position[i];
            value -= second_key->position[i];
        }
        else
        {
            first_key = (const VectorAnimationKey *)positions.keys;
            second_key = (const VectorAnimationKey *)positions.keys + 1;
            value = first_key->position[i];
            value -= second_key->position[i];
        }
        if (i == 0)
            vector.vx = value;
        else if (i == 1)
            vector.vy = value;
        else
            vector.vz = value;
    }
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
    if ((this->transform) != 0)
    {
        PushMatrix();
        matrix = (const MATRIX *)((this->transform) != 0 ? &(this->transform)->matrix : (MATRIX *)0);
        for (i = 0; i < 5; ++i)
            xport_gte_write_control(i, xport_load_le32((const uint8 *)matrix + i * 4));
        ApplyRotMatrixLV(&vector, destination);
        PopMatrix();
    }
    else
    {
        destination->vx = vector.vx;
        destination->vy = vector.vy;
        destination->vz = vector.vz;
    }
}

struct FallbackWorkspace
{
    PathVector scratch;
    const ScenePath *chains[3][10];
    PathVector probe;
    const ScenePath *candidates[5];
    const ScenePath *alternate[4];
    PathVector displaced;
    const ScenePath *ranked[10];
    uint32 costs[10];
    uint32 frames[10];
    PathVector endpoint;
};

struct FallbackDefaults
{
    const ScenePath *chains[10];
    const ScenePath *candidates[5];
    const ScenePath *alternate[4];
    const ScenePath *ranked[9];
    uint32 costs[9];
};

static const FallbackDefaults fallback_defaults = {{0}, {0}, {0}, {0}, {0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF}};

const ScenePath *AnimatedObject::FindFallbackPath(const uint32 *position, uint32 direction)
{
    const ScenePath *adjacent_path;
    FUNCTION_MARKER(0x8003BC08, "SLUS_005.78");
    const ScenePath *anchor_path;
    FallbackWorkspace work;
    const ScenePath *current, *node, *root, *candidate, *selected = 0;
    uint32 flags, scene;
    const PathDefinition *parent;
    uint32 same_definition;
    const ScenePath **first, **second, **third;
    uint32 offset, scan, slot, phase, end, value, x, y, z;
    /* A skipped same-path probe has no sampled frame or output vector */
    uint32 first_frame = 0xFFFFFFFF, previous_frame = 0xFFFFFFFF, next_frame = 0xFFFFFFFF;
    uint32 query, displacement;
    sint32 status = -1, best, signed_y;
    flags = this->path_flags;
    if ((flags & 0x440000) != 0)
    {
        for (phase = 0; phase < 3; ++phase)
        {
            for (offset = 0; offset < 10; ++offset)
                work.chains[phase][offset] = fallback_defaults.chains[offset];
        }
        scene = OriginalDefaults::SceneContextIdentity;
        current = camera_motion.path;
        node = (current->adjacent[0]);
        if (node != 0 && (((const PathDefinition *)((((const ScenePath *)(node))->definition) != 0 ? (((const ScenePath *)(node))->definition) : (const PathDefinition *)0))->flags & 0x10) != 0)
        {
            selected = node->fallback;
            if (selected != 0)
            {
                root = selected;
                if (selected != current)
                {
                    first_frame = ((const PathDefinition *)((const PathDefinition *)((selected->definition))))->FindHeightFrame(position, direction, 0xFFFB0000);
                    if (first_frame != 0xFFFFFFFF)
                        first_frame = (uint32)((const PathDefinition *)((const PathDefinition *)((selected->definition))))->ProjectFrame((sint32)first_frame, position, work.probe.words, direction, 4096);
                }
                if (first_frame != 0xFFFFFFFF)
                {
                    value = work.probe.words[0];
                    x = position[0];
                    y = work.probe.words[1];
                    work.probe.words[0] = value - x;
                    x = position[1];
                    y -= x;
                    z = work.probe.words[2];
                    work.probe.words[1] = y;
                    x = position[2];
                    work.probe.words[2] = z - x;
                    if ((sint32)y > 0)
                        first_frame = 0xFFFFFFFF;
                    if (first_frame != 0xFFFFFFFF)
                        goto return_selected;
                }
                node = (selected->adjacent[1]);
                if (node != 0)
                {
                    current = camera_motion.path;
                    selected = node;
                    if (selected != current)
                    {
                        previous_frame = ((const PathDefinition *)((const PathDefinition *)((selected->definition))))->FindHeightFrame(position, direction, 0xFFFB0000);
                        if (previous_frame != 0xFFFFFFFF)
                            previous_frame = (uint32)((const PathDefinition *)((const PathDefinition *)((selected->definition))))->ProjectFrame((sint32)previous_frame, position, work.probe.words, direction, 4096);
                    }
                    if (previous_frame != 0xFFFFFFFF)
                    {
                        value = work.probe.words[0];
                        x = position[0];
                        y = work.probe.words[1];
                        work.probe.words[0] = value - x;
                        x = position[1];
                        y -= x;
                        z = work.probe.words[2];
                        work.probe.words[1] = y;
                        x = position[2];
                        work.probe.words[2] = z - x;
                        if ((sint32)y > 0)
                            previous_frame = 0xFFFFFFFF;
                        if (previous_frame != 0xFFFFFFFF)
                            goto return_selected;
                    }
                }
                if ((root->node.state_flags & 0x10) != 0)
                {
                    node = root->fallback;
                    if (node != 0)
                    {
                        current = camera_motion.path;
                        selected = node;
                        if (selected != current)
                        {
                            next_frame = ((const PathDefinition *)((const PathDefinition *)((selected->definition))))->FindHeightFrame(position, direction, 0xFFFB0000);
                            if (next_frame != 0xFFFFFFFF)
                            {
                                next_frame = (uint32)((const PathDefinition *)((const PathDefinition *)((selected->definition))))->ProjectFrame((sint32)next_frame, position, work.probe.words, direction, 4096);
                            }
                        }
                        if (next_frame != 0xFFFFFFFF)
                        {
                            value = work.probe.words[0];
                            x = position[0];
                            y = work.probe.words[1];
                            work.probe.words[0] = value - x;
                            x = position[1];
                            y -= x;
                            z = work.probe.words[2];
                            work.probe.words[1] = y;
                            x = position[2];
                            work.probe.words[2] = z - x;
                            if ((sint32)y > 0)
                                next_frame = 0xFFFFFFFF;
                            if (next_frame != 0xFFFFFFFF)
                                goto return_selected;
                        }
                    }
                }
            }
            scene = OriginalDefaults::SceneContextIdentity;
        }
        else if (node != 0)
            scene = OriginalDefaults::SceneContextIdentity;
        work.chains[0][0] = camera_motion.path;
        work.chains[1][0] = camera_motion.path;
        for (scan = 1; scan < 10; ++scan)
        {
            first = work.chains[0] + scan;
            second = work.chains[1] + scan;
            third = work.chains[2] + scan;
            node = first[-1];
            if (node != 0)
                *first = (node->adjacent[1]);
            current = camera_motion.path;
            if (*first == current)
                *first = 0;
            node = second[-1];
            if (node != 0)
                *second = (node->adjacent[0]);
            current = camera_motion.path;
            if (*second == current)
            {
                node = *first;
                if ((((const PathDefinition *)((((const ScenePath *)(node))->definition) != 0 ? (((const ScenePath *)(node))->definition) : (const PathDefinition *)0))->flags & 0x100011) != 0)
                    *second = (node->definition->flags & 0x100010) != 0 ? node->fallback : 0;
                else
                    *second = 0;
            }
            if (*second != 0)
            {
                node = second[-1];
                if ((((const PathDefinition *)((((const ScenePath *)(node))->definition) != 0 ? (((const ScenePath *)(node))->definition) : (const PathDefinition *)0))->flags & 0x100011) != 0)
                    *third = (node->definition->flags & 0x100010) != 0 ? node->fallback : 0;
            }
            slot = 0;
            while (*first != 0 || *second != 0)
            {
                candidate = *(slot == 0 ? first : slot == 1 ? second : third);
                if (candidate != 0)
                {
                    query = ((const PathDefinition *)((const PathDefinition *)((candidate->definition))))->FindHeightFrame(position, direction, 0xFFFB0000);
                    if (query != 0xFFFFFFFF)
                    {
                        status = ((const PathDefinition *)((const PathDefinition *)((candidate->definition))))->ProjectFrame((sint32)query, position, work.scratch.words, direction, 4096);
                        // Keep the copied query result separate from later status rejection
                        query = (uint32)status;
                    }
                    if (status >= 0)
                    {
                        x = work.scratch.words[0] - position[0] + 24576;
                        if (x > 0xC000)
                            status = -1;
                        else
                        {
                            z = work.scratch.words[2] - position[2] + 24576;
                            if (z > 0xC000)
                                status = -1;
                        }
                    }
                }
                else
                    query = 0xFFFFFFFF;
                if (query != 0xFFFFFFFF)
                {
                    x = work.scratch.words[0] - position[0];
                    work.scratch.words[0] = x;
                    y = work.scratch.words[1] - position[1];
                    z = work.scratch.words[2];
                    work.scratch.words[1] = y;
                    z -= position[2];
                    work.scratch.words[2] = z;
                    if ((sint32)y > 0)
                        query = 0xFFFFFFFF;
                    if (query != 0xFFFFFFFF)
                    {
                        selected = candidate;
                        goto return_selected;
                    }
                }
                slot += 1;
                if (slot >= 3)
                    break;
            }
        }
        for (offset = 0; offset < 5; ++offset)
            work.candidates[offset] = fallback_defaults.candidates[offset];
        if (work.chains[1][1] != 0)
        {
            if (work.chains[1][2] != 0)
            {
                current = camera_motion.path;
                adjacent_path = (current->adjacent[0]);
                adjacent_path = (adjacent_path->adjacent[0]);
                adjacent_path = (adjacent_path->adjacent[1]);
                work.candidates[0] = adjacent_path;
                current = camera_motion.path;
                if (adjacent_path == current)
                    work.candidates[0] = 0;
            }
            scene = OriginalDefaults::SceneContextIdentity;
            current = camera_motion.path;
            adjacent_path = (current->adjacent[0]);
            node = (adjacent_path->adjacent[1]);
            if (node != 0)
            {
                adjacent_path = (node->adjacent[1]);
                work.candidates[1] = adjacent_path;
                current = camera_motion.path;
                if (adjacent_path == current)
                {
                    work.candidates[1] = 0;
                    current = camera_motion.path;
                }
                node = (current->adjacent[0]);
                adjacent_path = (node->adjacent[1]);
                if (adjacent_path != 0)
                {
                    work.candidates[2] = adjacent_path;
                    if (adjacent_path == camera_motion.path)
                        work.candidates[2] = 0;
                }
            }
        }
        node = work.chains[0][3];
        if (node != 0)
        {
            adjacent_path = (node->adjacent[0]);
            work.candidates[3] = adjacent_path;
            if (adjacent_path == camera_motion.path)
                work.candidates[3] = 0;
        }
        node = work.chains[1][1];
        if (node != 0 && (((const PathDefinition *)((((const ScenePath *)(node))->definition) != 0 ? (((const ScenePath *)(node))->definition) : (const PathDefinition *)0))->flags & 0x10) != 0)
        {
            node = node->fallback;
            if (node != 0)
            {
                adjacent_path = (node->adjacent[1]);
                work.candidates[4] = adjacent_path;
                if (adjacent_path == camera_motion.path)
                    work.candidates[4] = 0;
            }
        }
        for (scan = 0; scan < 5; ++scan)
        {
            selected = work.candidates[scan];
            query = 0xFFFFFFFF;
            if (selected != 0)
            {
                query = ((const PathDefinition *)((const PathDefinition *)((selected->definition))))->FindHeightFrame(position, direction, 0xFFFB0000);
                if (query != 0xFFFFFFFF)
                    query = (uint32)((const PathDefinition *)((const PathDefinition *)((selected->definition))))->ProjectFrame((sint32)query, position, work.scratch.words, direction, 4096);
            }
            if (query != 0xFFFFFFFF)
            {
                x = work.scratch.words[0] - position[0];
                work.scratch.words[0] = x;
                y = work.scratch.words[1] - position[1];
                z = work.scratch.words[2];
                work.scratch.words[1] = y;
                z -= position[2];
                work.scratch.words[2] = z;
                if ((sint32)y > 0)
                    query = 0xFFFFFFFF;
                if (query != 0xFFFFFFFF)
                    goto return_selected;
            }
        }
        selected = 0;
        goto return_selected;
    }
    if ((flags & 4) != 0)
    {
        status = -1;
        current = camera_motion.path;
        if ((current->adjacent[1]) != 0)
        {
            for (offset = 0; offset < 4; ++offset)
                work.alternate[offset] = fallback_defaults.alternate[offset];
            current = camera_motion.path;
            adjacent_path = (current->adjacent[1]);
            work.alternate[0] = adjacent_path;
            if (adjacent_path != 0)
            {
                current = camera_motion.path;
                adjacent_path = (current->adjacent[1]);
                adjacent_path = (adjacent_path->adjacent[1]);
                work.alternate[1] = adjacent_path;
                if (adjacent_path != 0)
                {
                    current = camera_motion.path;
                    adjacent_path = (current->adjacent[1]);
                    adjacent_path = (adjacent_path->adjacent[1]);
                    adjacent_path = (adjacent_path->adjacent[1]);
                    work.alternate[2] = adjacent_path;
                    if (adjacent_path != 0)
                    {
                        current = camera_motion.path;
                        adjacent_path = (current->adjacent[1]);
                        adjacent_path = (adjacent_path->adjacent[1]);
                        adjacent_path = (adjacent_path->adjacent[1]);
                        work.alternate[3] = (adjacent_path->adjacent[1]);
                    }
                }
            }
            for (scan = 0; scan < 4; ++scan)
            {
                if (status != -1)
                    goto return_selected;
                selected = work.alternate[scan];
                query = 0xFFFFFFFF;
                if (selected != 0)
                    query = ((const PathDefinition *)((const PathDefinition *)((selected->definition))))->FindHeightFrame(position, direction, 0xFFFB0000);
                if (query != 0xFFFFFFFF)
                    status = ((const PathDefinition *)((const PathDefinition *)((selected->definition))))->ProjectFrame((sint32)query, position, work.probe.words, direction, 4096);
                if (status != -1)
                {
                    x = work.probe.words[0] - position[0];
                    work.probe.words[0] = x;
                    y = work.probe.words[1] - position[1];
                    z = work.probe.words[2];
                    work.probe.words[1] = y;
                    z -= position[2];
                    work.probe.words[2] = z;
                    if ((sint32)y > 0)
                        status = -1;
                    if (status != -1 && (((const PathDefinition *)((const PathDefinition *)((selected->definition))))->flags & 0x10) != 0)
                    {
                        value = (uint32)SinQ14(yaw);
                        displacement = (uint32)(((sint64)((sint32)value >> 2) * 20480) >> 12);
                        value = (uint32)SinQ14((sint16)(1024 - (uint16)yaw));
                        value = (uint32)(((sint64)((sint32)value >> 2) * 20480) >> 12);
                        x = position[0];
                        y = position[1];
                        z = position[2];
                        work.displaced.words[0] = x;
                        work.displaced.words[1] = y;
                        work.displaced.words[2] = z;
                        x = work.displaced.words[0];
                        z = work.displaced.words[2];
                        work.displaced.words[0] = x - displacement;
                        work.displaced.words[2] = z - value;
                        query = ((const PathDefinition *)((const PathDefinition *)((selected->definition))))->FindHeightFrame(work.displaced.words, direction, 0xFFFB0000);
                        if (query == 0xFFFFFFFF)
                            status = -1;
                        else
                            status = ((const PathDefinition *)((const PathDefinition *)((selected->definition))))->ProjectFrame((sint32)query, work.displaced.words, work.probe.words, direction, 8192);
                    }
                }
            }
        }
        if (status == -1)
            selected = 0;
        goto return_selected;
    }
    if ((flags & 0x20) == 0)
    {
        signed_y = (sint8)path_routing.fallback_phase;
        if (signed_y == 1)
        {
            current = camera_motion.path;
            path_routing.fallback_phase = 2;
            selected = current->fallback;
            goto return_selected;
        }
        selected = 0;
        if (movement_mode != 2 || this->state != 19)
            goto return_selected;
        current = camera_motion.path;
        if ((((const PathDefinition *)((((const ScenePath *)(current))->definition) != 0 ? (((const ScenePath *)(current))->definition) : (const PathDefinition *)0))->flags & 0x90010) == 0)
            goto return_selected;
        selected = (current->adjacent[1]);
        status = -1;
        // Original stack-pointer frame argument is negative and clamps to zero
        if (selected != 0)
            status = ((const PathDefinition *)((const PathDefinition *)((selected->definition))))->ProjectFrame(0, position, work.endpoint.words, direction, 4096);
        if (status == -1)
            selected = 0;
        goto return_selected;
    }
    for (offset = 0; offset < 9; ++offset)
        work.ranked[offset] = fallback_defaults.ranked[offset];
    for (offset = 0; offset < 9; ++offset)
        work.costs[offset] = fallback_defaults.costs[offset];
    current = camera_motion.path;
    selected = current->fallback;
    best = -1;
    status = -1;
    if (selected != 0 && (((const PathDefinition *)((((const ScenePath *)(current))->definition) != 0 ? (((const ScenePath *)(current))->definition) : (const PathDefinition *)0))->flags & 0x100000) != 0)
    {
        query = ((const PathDefinition *)((const PathDefinition *)((selected->definition))))->FindHeightFrame(position, direction, 0xFFFB0000);
        if (query != 0xFFFFFFFF)
            status = ((const PathDefinition *)((const PathDefinition *)((selected->definition))))->ProjectFrame((sint32)query, position, work.displaced.words, direction, 81920);
        if (status != -1)
        {
            x = work.displaced.words[0] - position[0];
            work.displaced.words[0] = x;
            y = work.displaced.words[1] - position[1];
            z = work.displaced.words[2];
            work.displaced.words[1] = y;
            z -= position[2];
            work.displaced.words[2] = z;
            if ((sint32)y > 0)
                status = -1;
        }
    }
    anchor_path = (const ScenePath *)(character_controller.path_transition.anchor_path);
    node = (anchor_path->adjacent[1]);
    if (node != 0)
    {
        current = camera_motion.path;
        if (current == node)
        {
            selected = current;
            query = ((const PathDefinition *)((((const ScenePath *)(current))->definition) != 0 ? (((const ScenePath *)(current))->definition) : (const PathDefinition *)0))->FindHeightFrame(position, direction, 0xFFFB0000);
            status = ((const PathDefinition *)((const PathDefinition *)((selected->definition))))->ProjectFrame((sint32)query, position, work.displaced.words, direction, 4096);
            if (status >= 0)
            {
                x = work.displaced.words[0] - position[0] + 24576;
                if (x > 0xC000)
                    goto ranked_shortcut_finished;
                z = work.displaced.words[2] - position[2] + 24576;
                if (z > 0xC000)
                    goto ranked_shortcut_finished;
            }
            x = work.displaced.words[0] - position[0];
            work.displaced.words[0] = x;
            y = work.displaced.words[1] - position[1];
            z = work.displaced.words[2];
            work.displaced.words[1] = y;
            z -= position[2];
            work.displaced.words[2] = z;
            if ((sint32)y >= 4097)
                status = -1;
        }
    }
ranked_shortcut_finished:
    if (status != -1)
        goto ranked_finished;
    scene = OriginalDefaults::SceneContextIdentity;
    current = camera_motion.path;
    adjacent_path = (current->adjacent[1]);
    work.ranked[0] = adjacent_path;
    if (adjacent_path != 0)
    {
        current = camera_motion.path;
        adjacent_path = (current->adjacent[1]);
        work.ranked[1] = (adjacent_path->adjacent[1]);
    }
    current = camera_motion.path;
    node = (current->adjacent[0]);
    if (node != 0)
    {
        work.ranked[2] = (node->adjacent[1]);
        current = camera_motion.path;
        adjacent_path = (current->adjacent[0]);
        node = (adjacent_path->adjacent[0]);
        if (node != 0)
            work.ranked[3] = (node->adjacent[1]);
    }
    for (phase = 0; phase < 3; ++phase)
    {
        if (phase == 0)
        {
            scan = 0;
            end = 4;
        }
        else if (phase == 1)
        {
            if (work.ranked[1] != 0)
            {
                current = camera_motion.path;
                adjacent_path = (current->adjacent[1]);
                adjacent_path = (adjacent_path->adjacent[1]);
                work.ranked[4] = (adjacent_path->adjacent[1]);
            }
            current = camera_motion.path;
            node = (current->adjacent[0]);
            if (node != 0)
            {
                node = (node->adjacent[0]);
                if (node != 0)
                {
                    node = (node->adjacent[0]);
                    if (node != 0)
                        work.ranked[5] = (node->adjacent[1]);
                }
            }
            scan = 4;
            end = 6;
        }
        else
        {
            if (work.ranked[0] != 0)
            {
                current = camera_motion.path;
                adjacent_path = (current->adjacent[1]);
                work.ranked[6] = (adjacent_path->adjacent[0]);
            }
            scene = OriginalDefaults::SceneContextIdentity;
            current = camera_motion.path;
            node = (current->adjacent[0]);
            if (node != 0)
                work.ranked[7] = (node->adjacent[0]);
            current = camera_motion.path;
            work.ranked[8] = (current->adjacent[0]);
            scan = 6;
            end = 9;
        }
        for (; scan < end; ++scan)
        {
            candidate = work.ranked[scan];
            if (candidate == 0)
                continue;
            current = camera_motion.path;
            parent = candidate->definition;
            same_definition = parent == current->definition;
            if (same_definition != 0 || candidate == current)
                continue;
            query = ((const PathDefinition *)(parent))->FindHeightFrame(position, direction, 0xFFFB0000);
            work.frames[scan] = query;
            if (query == 0xFFFFFFFF)
                continue;
            candidate = work.ranked[scan];
            status = ((const PathDefinition *)((const PathDefinition *)((candidate->definition))))->ProjectFrame((sint32)work.frames[scan], position, work.displaced.words, direction, 4096);
            if (status >= 0)
            {
                x = work.displaced.words[0] - position[0] + 24576;
                if (x > 0xC000)
                    status = -1;
                else
                {
                    z = work.displaced.words[2] - position[2] + 24576;
                    if (z > 0xC000)
                        status = -1;
                }
            }
            if (status != -1)
            {
                x = work.displaced.words[0] - position[0];
                work.displaced.words[0] = x;
                y = work.displaced.words[1] - position[1];
                z = work.displaced.words[2];
                work.displaced.words[1] = y;
                z -= position[2];
                work.displaced.words[2] = z;
                if ((sint32)y > 0)
                    status = -1;
                else
                {
                    value = 0 - y;
                    work.costs[scan] = value;
                    if (best == -1 || (sint32)value < (sint32)work.costs[(uint32)best])
                        best = (sint32)scan;
                }
            }
        }
        if (best != -1)
        {
            if (phase == 0)
                status = (sint32)work.frames[(uint32)best];
            selected = work.ranked[(uint32)best];
        }
        if (phase < 2 && status != -1)
            goto ranked_finished;
    }
ranked_finished:
    if (status < 0)
    {
        MarkPathSearchFailed();
        selected = 0;
    }
return_selected:
    return selected;
}

uint32 AnimatedObject::DetachCharacterFromPath(uint32 *direction)
{
    FUNCTION_MARKER(0x8004FA54, "SLUS_005.78");
    PathVector vector;
    const PathDefinition *parent;
    const MATRIX *matrix;
    uint32 flags, x, y, z, frame, forward, reverse, value, clock, mode, result;
    ApplyPathVelocityChange(0);
    this->flags = (this->flags & 0xFFFFF7FF) | 0x2000;
    parent = (this->path_definition);
    if (parent != 0 && (parent->transform) != 0)
    {
        x = this->position[0];
        y = this->position[1];
        z = this->position[2];
        vector.words[0] = x;
        vector.words[1] = y;
        vector.words[2] = z;
        PushMatrix();
        matrix = (const MATRIX *)((((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) != 0 ? &(((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform)->matrix : (MATRIX *)0);
        x = xport_load_le32((const uint8 *)matrix);
        y = xport_load_le32((const uint8 *)matrix + 4);
        xport_gte_write_control(0, x);
        xport_gte_write_control(1, y);
        x = xport_load_le32((const uint8 *)matrix + 8);
        y = xport_load_le32((const uint8 *)matrix + 12);
        z = xport_load_le32((const uint8 *)matrix + 16);
        xport_gte_write_control(2, x);
        xport_gte_write_control(3, y);
        xport_gte_write_control(4, z);
        ApplyRotMatrixLV(&vector.vector, (VECTOR *)this->position);
        PopMatrix();
        matrix = (const MATRIX *)((((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) != 0 ? &(((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform)->matrix : (MATRIX *)0);
        x = this->position[0];
        y = matrix->t[0];
        parent = (const PathDefinition *)((const PathDefinition *)((this->path_definition)));
        this->position[0] = x + y;
        matrix = (const MATRIX *)((parent->transform) != 0 ? &(parent->transform)->matrix : (MATRIX *)0);
        x = this->position[1];
        y = matrix->t[1];
        parent = (const PathDefinition *)((const PathDefinition *)((this->path_definition)));
        this->position[1] = x + y;
        matrix = (const MATRIX *)((parent->transform) != 0 ? &(parent->transform)->matrix : (MATRIX *)0);
        x = this->position[2];
        y = matrix->t[2];
        this->position[2] = x + y;
    }
    else
    {
        x = this->position[0];
        y = this->position[1];
        z = this->position[2];
        this->position[0] = x;
        this->position[1] = y;
        this->position[2] = z;
    }
    frame = (uint16)this->path_frame;
    flags = this->flags;
    forward = (flags >> 4) & 1;
    reverse = (frame >> 15) ^ forward;
    ((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->SampleTangent(&character_controller.path_tangent, (sint32)(reverse << 4));
    if (forward != reverse)
    {
        x = (uint32)character_controller.path_tangent.vx;
        z = (uint32)character_controller.path_tangent.vz;
        character_controller.path_tangent.vx = 0 - x;
        character_controller.path_tangent.vz = 0 - z;
    }
    flags = this->flags;
    if (flags & 0x400)
    {
        x = (uint32)character_controller.path_tangent.vx;
        character_controller.path_tangent.vy = 0;
        if (x + 26754 > 0xD103 || (uint32)character_controller.path_tangent.vz + 26754 > 0xD103)
        {
            for (;;)
            {
                x = (uint32)character_controller.path_tangent.vx;
                z = (uint32)character_controller.path_tangent.vz;
                character_controller.path_tangent.vx = (uint32)((sint32)x >> 1);
                y = (uint32)character_controller.path_tangent.vy;
                x = (uint32)character_controller.path_tangent.vx;
                z = (uint32)((sint32)z >> 1);
                character_controller.path_tangent.vz = z;
                character_controller.path_tangent.vy = (uint32)((sint32)y >> 1);
                if (x + 26754 > 0xD103)
                    continue;
                if ((uint32)character_controller.path_tangent.vy + 26754 > 0xD103)
                    continue;
                if (z + 26754 <= 0xD103)
                    break;
            }
        }
        result = (uint32)VectorNormal(&character_controller.path_tangent, &character_controller.path_tangent);
    }
    else
    {
        this->flags = flags | 0x400;
        clock = character_controller.movement.jump_ticks;
        value = character_controller.flags;
        mode = (uint16)(character_controller.motion.form_flags >> 16) & 15;
        character_controller.motion.jump_phase = -1;
        character_controller.motion.vertical_speed = 0;
        character_controller.timers.state_ticks = 0;
        character_controller.timers.jump_duration_ticks = clock;
        character_controller.flags = value | 0x08000000;
        result = 0;
        if (mode == 1)
            result = this->flags & 0x04000000;
        if (!result)
        {
            value = character_controller.secondary_flags;
            result = 6;
            if (!(value & 0x80))
                character_controller.motion.state_code = 6;
        }
    }
    if (direction)
    {
        result = (uint32)character_controller.path_tangent.vx;
        y = (uint32)character_controller.path_tangent.vy;
        z = (uint32)character_controller.path_tangent.vz;
        direction[0] = result;
        direction[1] = y;
        direction[2] = z;
    }
    return result;
}

uint32 AnimatedObject::ComputePathMotion(sint32 distance, sint32 height, const VECTOR *direction_vector)
{
    FUNCTION_MARKER(0x8003DB7C, "SLUS_005.78");
    PathVector working, original, next, current, temporary, motion;
    const ScenePath *initial_path = 0, *path, *path_view, *candidate, *candidate_view, *current_path;
    uint32 character = 0, special = 0, special_path = 0;
    const PathDefinition *definition;
    const MATRIX *matrix;
    uint32 flags, value, x, y, z, squared, direction, track_direction;
    uint32 i, stage, stop, valid, first_segment;
    uint32 previous_x, previous_y, previous_z, result;
    sint32 category, frame, limit, drop = 0, base_height, initial_distance, reciprocal, factor, length, height_step;
    category = (sint8)metadata126[0];
    if (category == 1)
    {
        if (behavior != 33)
            initial_path = camera_motion.path;
        else
            initial_path = (this->scene_path);
    }
    else if (category == 3 && animation_binding.tracks[0] == -1)
        initial_path = (this->scene_path);
    path_routing.fallback_phase = 0;
    category = (sint8)metadata126[0];
    if (category == 1)
        character = 0x800ABD68;
    else if (movement_mode == 2 && (this->state == 19 || path_routing.flags != 0))
    {
        FindPlayerObject(0);
        special = 1;
        if ((((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->flags & 0x80000) != 0)
            special_path = 1;
    }
    if (special != 0 && (((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->flags & 0x10000) != 0)
    {
        definition = (this->path_definition);
        if (definition != 0 && (definition->transform) != 0)
        {
            x = this->position[0];
            y = this->position[1];
            z = this->position[2];
            temporary.words[0] = x;
            temporary.words[1] = y;
            temporary.words[2] = z;
            PushMatrix();
            matrix = (const MATRIX *)((((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) != 0 ? &(((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform)->matrix : (MATRIX *)0);
            for (i = 0; i < 5; ++i)
                xport_gte_write_control(i, xport_load_le32((const uint8 *)matrix + i * 4));
            ApplyRotMatrixLV(&temporary.vector, &original.vector);
            PopMatrix();
            for (i = 0; i < 3; ++i)
            {
                matrix = (const MATRIX *)((((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) != 0 ? &(((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform)->matrix : (MATRIX *)0);
                original.words[i] = original.words[i] + matrix->t[i];
            }
        }
        else
        {
            x = this->position[0];
            y = this->position[1];
            z = this->position[2];
            original.words[0] = x;
            original.words[1] = y;
            original.words[2] = z;
        }
        x = original.words[0];
        y = original.words[1];
        z = original.words[2];
        current.words[0] = x;
        current.words[1] = y;
        current.words[2] = z;
        x = current.words[0];
        y = current.words[1];
        z = current.words[2];
        working.words[0] = x;
        working.words[1] = y;
        working.words[2] = z;
        ChangeActorPath(camera_motion.path, working.words, current.words);
    }
    initial_distance = distance;
    base_height = height;
    if (distance < 0)
    {
        initial_distance = 0;
        distance = 0;
    }
    flags = this->path_flags;
    if ((flags & 0x440000) != 0)
    {
        CompleteCharacterPathTransfer(character != 0 ? &character_controller : 0);
        goto check_direct;
    }
    if ((this->flags & 0x2000) != 0 && (flags & 4) != 0)
        goto prepare_contact;
    if ((this->path_flags & 0x20) == 0)
        goto check_direct;
    if ((this->flags & 0x2000) == 0)
        goto prepare_position;
prepare_contact:
    definition = (this->path_definition);
    if (definition != 0 && (definition->transform) != 0)
    {
        x = this->position[0];
        y = this->position[1];
        z = this->position[2];
        temporary.words[0] = x;
        temporary.words[1] = y;
        temporary.words[2] = z;
        PushMatrix();
        matrix = (const MATRIX *)((((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) != 0 ? &(((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform)->matrix : (MATRIX *)0);
        for (i = 0; i < 5; ++i)
            xport_gte_write_control(i, xport_load_le32((const uint8 *)matrix + i * 4));
        ApplyRotMatrixLV(&temporary.vector, &original.vector);
        PopMatrix();
        for (i = 0; i < 3; ++i)
        {
            matrix = (const MATRIX *)((((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) != 0 ? &(((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform)->matrix : (MATRIX *)0);
            original.words[i] = original.words[i] + matrix->t[i];
        }
    }
    else
    {
        x = this->position[0];
        y = this->position[1];
        z = this->position[2];
        original.words[0] = x;
        original.words[1] = y;
        original.words[2] = z;
    }
    x = original.words[0];
    y = original.words[1];
    z = original.words[2];
    current.words[0] = x;
    current.words[1] = y;
    current.words[2] = z;
    x = current.words[0];
    y = current.words[1];
    z = current.words[2];
    working.words[0] = x;
    working.words[1] = y;
    working.words[2] = z;
    ChangeActorPath(camera_motion.path, working.words, current.words);
check_direct:
    if ((this->flags & 0x2000) != 0)
        goto direct_vector;
prepare_position:
    definition = (this->path_definition);
    if (definition != 0 && (definition->transform) != 0)
    {
        x = this->position[0];
        y = this->position[1];
        z = this->position[2];
        temporary.words[0] = x;
        temporary.words[1] = y;
        temporary.words[2] = z;
        PushMatrix();
        matrix = (const MATRIX *)((((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) != 0 ? &(((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform)->matrix : (MATRIX *)0);
        for (i = 0; i < 5; ++i)
            xport_gte_write_control(i, xport_load_le32((const uint8 *)matrix + i * 4));
        ApplyRotMatrixLV(&temporary.vector, &original.vector);
        PopMatrix();
        for (i = 0; i < 3; ++i)
        {
            matrix = (const MATRIX *)((((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) != 0 ? &(((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform)->matrix : (MATRIX *)0);
            original.words[i] = original.words[i] + matrix->t[i];
        }
    }
    else
    {
        x = this->position[0];
        y = this->position[1];
        z = this->position[2];
        original.words[0] = x;
        original.words[1] = y;
        original.words[2] = z;
    }
    x = original.words[0];
    y = original.words[1];
    z = original.words[2];
    current.words[0] = x;
    current.words[1] = y;
    current.words[2] = z;
    x = current.words[0];
    y = current.words[1];
    z = current.words[2];
    working.words[0] = x;
    working.words[1] = y;
    working.words[2] = z;
    direction = this->flags;
    direction &= 0x10;
    if ((this->flags & 0x400) == 0 && (this->path_flags & 0x1222) == 0)
        goto prepare_next;
    for (;;)
    {
        result = (uint32)((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->ProjectFrame(this->path_frame, working.words, current.words, direction, 4096);
        if ((sint32)result >= 0)
            break;
        frame = this->path_frame;
        if (frame != (sint16)((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->positions.metadata6 - 1)
        {
            direction_vector = &working.vector;
            if ((sint8)metadata126[0] == 1)
            {
                DetachCharacterFromPath(working.words);
                goto direct_vector;
            }
            break;
        }
        this->path_frame = (sint16)((uint32)frame - 1);
    }
    if ((sint32)working.words[1] < (sint32)current.words[1] && base_height >= 0)
        current.words[1] = working.words[1];
    if (distance == 0 && base_height < 0 && (this->path_flags & 0x20) == 0)
    {
        x = current.words[0] - working.words[0];
        y = current.words[1] - working.words[1];
        z = current.words[2] - working.words[2];
        motion.words[0] = x;
        motion.words[1] = y;
        motion.words[2] = z;
        if ((sint32)y < 0)
        {
            motion.words[1] = 0;
            reciprocal = 0x40000000 / (sint32)y;
            if (x + 26754 > 0xD103 || z + 26754 > 0xD103)
            {
                x = motion.words[0];
                y = motion.words[1];
                z = motion.words[2];
                squared = (uint32)(((sint64)(sint32)x * (sint32)x) >> 12);
                squared += (uint32)(((sint64)(sint32)y * (sint32)y) >> 12);
                squared += (uint32)(((sint64)(sint32)z * (sint32)z) >> 12);
            }
            else
                squared = (uint32)((sint32)(x * x + z * z) >> 12);
            if ((sint32)squared < 0)
                value = SquareRoot12((sint32)(squared >> 2)) << 1;
            else
                value = SquareRoot12((sint32)squared);
            factor = (sint32)(uint32)(((sint64)(sint32)value * base_height) >> 12);
            distance = (sint32)(uint32)(((sint64)factor * reciprocal) >> 18);
            fixed_math_result = (uint32)distance;
            initial_distance = distance;
            if (distance < 0)
            {
                distance = 0;
                initial_distance = 0;
            }
        }
    }
    drop = (sint32)(working.words[1] - current.words[1]);
    current.words[0] = working.words[0];
    current.words[2] = working.words[2];
    if (drop < -1365 && (this->path_flags & 0x20) == 0 && (special_path & special) == 0 && (character == 0 || ((uint16)(character_controller.motion.form_flags >> 16) & 0xF) != 5) && (((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->flags & 0x11) == 0)
    {
        x = current.words[0];
        y = current.words[1];
        z = current.words[2];
        working.words[0] = x;
        working.words[1] = y;
        working.words[2] = z;
        initial_distance = 0;
        base_height = 0;
        first_segment = 0;
        goto finish_fall;
    }
prepare_next:
    x = current.words[0];
    y = current.words[1];
    z = current.words[2];
    next.words[0] = x;
    next.words[1] = y;
    next.words[2] = z;
restart_path:
    frame = this->path_frame;
    first_segment = frame < 1;
    if (frame >= 1 && (this->flags & 0x40) != 0)
        this->path_frame = (sint16)((uint32)frame - 1);
    category = (sint8)metadata126[0];
    path = 0;
    if (category == 1)
    {
        if (behavior != 33)
            path = camera_motion.path;
        else
            path = (this->scene_path);
    }
    else if (category == 3 && animation_binding.tracks[0] == -1)
        path = (this->scene_path);
    path_view = ((const ScenePath *)(path));
    stop = 0;
    if (base_height <= 0)
    {
        flags = ((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->flags;
        if ((flags & 0x51) >= 0x41 && ((flags & 0x10) == 0 || (path_view != 0 && (path_view->fallback != 0 || path_view->slope_scale != 0))))
            stop = 1;
    }
next_segment:
    track_direction = direction;
    candidate = path;
    frame = this->path_frame;
    if ((this->flags & 0x40) != 0)
        frame += 1;
    else
        frame -= 1;
    definition = (const PathDefinition *)((const PathDefinition *)((this->path_definition)));
    this->path_frame = (sint16)((uint32)frame);
    valid = 1;
    if (((uint32)frame & 0xFFFF) < definition->positions.metadata6)
    {
        definition->SamplePosition((sint16)frame, next.words, track_direction);
        if (path != 0 && ((this->flags & 0x400) != 0 || (this->path_flags & 0x1022) != 0))
        {
            value = (uint32)this->path_frame;
            if ((this->flags & 0x40) == 0)
            {
                if ((candidate->node.state_flags & 8) != 0)
                {
                    candidate = (candidate->adjacent[0]);
                }
                track_direction ^= 0x10;
                value = (uint32)(sint16)((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->positions.metadata6 - value - 1;
            }
            candidate_view = ((const ScenePath *)(candidate));
            if ((candidate_view->node.state_flags & (track_direction | 8)) != 0)
                limit = candidate_view->motion_limits[0];
            else
                limit = candidate_view->motion_limits[1];
            if (value >= (uint32)limit && limit != -1)
            {
                if ((uint32)limit < value)
                    this->flags = this->flags | 0x2000;
                valid = 0;
                this->flags = this->flags | 0x4000;
            }
        }
    }
    else
    {
        valid = 0;
        this->flags = this->flags | 0x800;
    }
    if (valid != 0 && (stop == 0 || drop >= 0) && (this->path_flags & 8) == 0)
        goto advance;
    flags = this->path_flags;
    if ((flags & 0x80000) != 0)
    {
        result = (flags & 0xFFF7FFFF) | 0x100000;
        this->path_flags = result;
        this->velocity[0] = 0;
        this->velocity[1] = 0;
        goto done;
    }
    flags = this->flags;
    if ((flags & 0x6000) != 0 && drop < 0)
    {
        this->flags = flags & 0xFFFF9FFF;
        if ((flags & 0x800) == 0 && stop == 0)
            goto advance;
    }
    if (distance == 0 && (this->path_flags & 8) == 0)
    {
        x = current.words[0];
        y = current.words[1];
        z = current.words[2];
        next.words[0] = x;
        next.words[1] = y;
        next.words[2] = z;
        flags = this->flags;
        this->flags = flags & 0xFFFFF7FF;
        if ((flags & 0x6000) == 0 && (stop == 0 || drop >= 0))
            goto advance;
    }
    previous_x = this->velocity[0];
    previous_y = this->velocity[1];
    previous_z = this->velocity[2];
    if ((this->path_flags & 2) != 0 && (((const PathDefinition *)((((const ScenePath *)(camera_motion.path))->definition) != 0 ? (((const ScenePath *)(camera_motion.path))->definition) : (const PathDefinition *)0))->flags & 0x10000) == 0 && CanContinuePathStep() != 0)
        goto end_contact;
    if (character != 0 && character_controller.motion.slope_scale != 0 && (uint32)initial_distance - 1 < 0x3FF && base_height < 512)
        goto end_contact;
    if ((ChangeActorPath(path, working.words, current.words) & 0xFF) == 0)
        goto end_contact;
    if ((((const PathDefinition *)((const PathDefinition *)((path_view->definition))))->flags & 0x40) != 0)
    {
        drop = (sint32)(working.words[1] - current.words[1]);
        if (drop < 1366)
        {
            if ((this->flags & 0x400) != 0)
                goto adjust_velocity;
            if ((this->path_flags & 0x1022) == 0)
            {
                x = current.words[0];
                y = current.words[1];
                z = current.words[2];
                working.words[0] = x;
                working.words[1] = y;
                working.words[2] = z;
            }
        }
        else
            this->flags = this->flags | 0x400;
    }
    if ((this->flags & 0x400) == 0 && (this->path_flags & 0x1022) == 0)
        goto check_path_change;
adjust_velocity:
    x = this->velocity[0] - previous_x;
    motion.words[0] = x;
    y = this->velocity[1] - previous_y;
    motion.words[1] = y;
    z = this->velocity[2] - previous_z;
    motion.words[2] = z;
    base_height = (sint32)((uint32)base_height + y);
    motion.words[1] = 0;
    height = (sint32)((uint32)height + y);
    if (x + 26754 > 0xD103 || z + 26754 > 0xD103)
    {
        x = motion.words[0];
        y = motion.words[1];
        z = motion.words[2];
        squared = (uint32)(((sint64)(sint32)x * (sint32)x) >> 12);
        squared += (uint32)(((sint64)(sint32)y * (sint32)y) >> 12);
        squared += (uint32)(((sint64)(sint32)z * (sint32)z) >> 12);
    }
    else
        squared = (uint32)((sint32)(x * x + z * z) >> 12);
    if ((sint32)squared < 0)
        motion.words[1] = SquareRoot12((sint32)(squared >> 2)) << 1;
    else
        motion.words[1] = SquareRoot12((sint32)squared);
    value = Atan2Angle((sint32)motion.words[2], (sint32)motion.words[0]);
    if ((((uint32)(sint16)value - (uint32)this->target_yaw - 1024) & 0xFFF) >= 0x800)
        motion.words[1] = 0 - motion.words[1];
    if ((sint32)((uint32)distance + motion.words[1]) <= 0)
        motion.words[1] = 1 - (uint32)distance;
    initial_distance = (sint32)((uint32)initial_distance + motion.words[1]);
    distance = (sint32)((uint32)distance + motion.words[1]);
check_path_change:
    for (stage = 2; stage <= 5; ++stage)
    {
        if (path == initial_path)
            break;
        category = (sint8)metadata126[0];
        current_path = 0;
        if (category == 1)
        {
            if (behavior != 33)
                current_path = camera_motion.path;
            else
                current_path = (this->scene_path);
        }
        else if (category == 3 && animation_binding.tracks[0] == -1)
            current_path = (this->scene_path);
        if (current_path != initial_path)
            continue;
        if (stage == 2 && (((const PathDefinition *)((const PathDefinition *)((path_view->definition))))->flags & 0x10) == 0)
            continue;
        if (stage == 3 && (((const PathDefinition *)((((const ScenePath *)(camera_motion.path))->definition) != 0 ? (((const ScenePath *)(camera_motion.path))->definition) : (const PathDefinition *)0))->flags & 0x80000) == 0)
            continue;
        if (stage == 4 && (((const PathDefinition *)((((const ScenePath *)(initial_path))->definition) != 0 ? (((const ScenePath *)(initial_path))->definition) : (const PathDefinition *)0))->flags & 0x10) == 0)
            continue;
        if (stage == 5 && special == 0)
            continue;
        if ((sint8)path_routing.fallback_phase == 0)
        {
            flags = this->flags;
            path_routing.fallback_phase = stage;
            this->flags = flags | 0x800;
            goto end_contact;
        }
    }
    this->flags = this->flags & 0xFFFFF7FF;
    goto restart_path;
end_contact:
    flags = this->flags;
    this->flags = flags & 0xFFFF9FFF;
    if ((flags & 0x800) == 0)
        goto advance;
    category = (sint8)metadata126[0];
    if (category == 1 || (((uint32)category - 3) & 0xFF) < 2)
    {
        ((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->SampleTangent(&motion.vector, (sint32)direction);
        if ((this->flags & 0x400) != 0 || (this->path_flags & 0x1002) != 0)
        {
            x = motion.words[0];
            motion.words[1] = 0;
            if (x + 26754 > 0xD103 || motion.words[2] + 26754 > 0xD103)
            {
                do
                {
                    x = (uint32)((sint32)motion.words[0] >> 1);
                    y = (uint32)((sint32)motion.words[1] >> 1);
                    z = (uint32)((sint32)motion.words[2] >> 1);
                    motion.words[0] = x;
                    motion.words[1] = y;
                    motion.words[2] = z;
                } while (x + 26754 > 0xD103 || y + 26754 > 0xD103 || z + 26754 > 0xD103);
            }
            VectorNormal(&motion.vector, &motion.vector);
        }
        if ((this->flags & 0x40) == 0)
            distance = (sint32)(0 - (uint32)distance);
        for (i = 0; i < 3; ++i)
            motion.words[i] = (uint32)(((sint64)(sint32)motion.words[i] * distance) >> 12);
        x = working.words[0] + motion.words[0];
        y = working.words[1] + motion.words[1];
        z = working.words[2] + motion.words[2];
        working.words[0] = x;
        working.words[1] = y;
        working.words[2] = z;
        working.words[1] = y + (uint32)height;
    }
    this->velocity[0] = working.words[0] - original.words[0];
    this->velocity[1] = working.words[1] - original.words[1];
    result = original.words[2];
    this->velocity[2] = working.words[2] - result;
    goto done;
advance:
    if ((this->flags & 0x400) != 0 || (this->path_flags & 0x1222) != 0)
    {
        x = next.words[0] - current.words[0];
        y = next.words[1] - current.words[1];
        z = next.words[2] - current.words[2];
        motion.words[0] = x;
        motion.words[1] = y;
        motion.words[2] = z;
        x = next.words[0];
        y = next.words[1];
        z = next.words[2];
        current.words[0] = x;
        current.words[1] = y;
        current.words[2] = z;
        value = motion.words[1];
        x = motion.words[0];
        motion.words[1] = 0;
        next.words[1] = value;
        z = motion.words[2];
        if (x + 26754 > 0xD103 || z + 26754 > 0xD103)
        {
            x = motion.words[0];
            y = motion.words[1];
            z = motion.words[2];
            squared = (uint32)(((sint64)(sint32)x * (sint32)x) >> 12);
            squared += (uint32)(((sint64)(sint32)y * (sint32)y) >> 12);
            squared += (uint32)(((sint64)(sint32)z * (sint32)z) >> 12);
        }
        else
            squared = (uint32)((sint32)(x * x + z * z) >> 12);
        if ((sint32)squared < 0)
            length = (sint32)(SquareRoot12((sint32)(squared >> 2)) << 1);
        else
            length = (sint32)SquareRoot12((sint32)squared);
        motion.words[1] = next.words[1];
    }
    else
    {
        x = next.words[0] - working.words[0];
        y = next.words[1] - working.words[1];
        z = next.words[2] - working.words[2];
        motion.words[0] = x;
        motion.words[1] = y;
        motion.words[2] = z;
        if (x + 26754 > 0xD103 || y + 26754 > 0xD103 || z + 26754 > 0xD103)
        {
            x = motion.words[0];
            y = motion.words[1];
            z = motion.words[2];
            squared = (uint32)(((sint64)(sint32)x * (sint32)x) >> 12);
            squared += (uint32)(((sint64)(sint32)y * (sint32)y) >> 12);
            squared += (uint32)(((sint64)(sint32)z * (sint32)z) >> 12);
        }
        else
            squared = (uint32)((sint32)(x * x + y * y + z * z) >> 12);
        if ((sint32)squared < 0)
            length = (sint32)(SquareRoot12((sint32)(squared >> 2)) << 1);
        else
            length = (sint32)SquareRoot12((sint32)squared);
    }
    if (distance < length)
    {
        if (length == 0)
        {
            xport_mips_break(7);
            reciprocal = -1;
        }
        else
            reciprocal = 0x40000000 / length;
        factor = (sint32)(uint32)(((sint64)distance * reciprocal) >> 18);
        flags = this->flags;
        fixed_math_result = (uint32)factor;
        distance = 0;
        if ((flags & 0x40) == 0)
            this->path_frame = (sint16)((uint16)this->path_frame + 1);
        factor = (sint32)fixed_math_result;
        for (i = 0; i < 3; ++i)
            motion.words[i] = (uint32)(((sint64)(sint32)motion.words[i] * factor) >> 12);
        height_step = height;
    }
    else if (height != 0)
    {
        if (distance != 0)
        {
            reciprocal = 0x40000000 / distance;
            factor = (sint32)(uint32)(((sint64)length * reciprocal) >> 18);
            distance = (sint32)((uint32)distance - (uint32)length);
            fixed_math_result = (uint32)factor;
            height_step = (sint32)(uint32)(((sint64)height * factor) >> 12);
        }
        else
            height_step = height;
    }
    else
    {
        distance = (sint32)((uint32)distance - (uint32)length);
        height_step = 0;
    }
    x = working.words[0] + motion.words[0];
    y = working.words[1] + motion.words[1];
    z = working.words[2] + motion.words[2];
    working.words[0] = x;
    working.words[1] = y;
    working.words[2] = z;
    if ((this->flags & 0x400) != 0 || (this->path_flags & 0x1222) != 0)
    {
        height = (sint32)((uint32)height - (uint32)height_step);
        value = (uint32)height_step - motion.words[1];
        y += value;
        drop = (sint32)((uint32)drop + value);
        working.words[1] = y;
        if (drop < -169 && (base_height <= 0 || (first_segment == 0 && (uint32)drop + 5461 >= 0x1001)) && (character == 0 || ((uint16)(character_controller.motion.form_flags >> 16) & 0xF) != 5) && (((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->flags & 0x11) == 0 && ((((const PathDefinition *)((((const ScenePath *)(camera_motion.path))->definition) != 0 ? (((const ScenePath *)(camera_motion.path))->definition) : (const PathDefinition *)0))->flags & 0x80000) == 0 || (this->path_flags & 0x20) == 0) && (special_path & special) == 0)
        {
            working.words[1] = y - (uint32)drop;
            goto finish_fall;
        }
    }
    if (distance != 0)
        goto next_segment;
    if (drop <= 0 && behavior != 1)
    {
        flags = this->flags;
        if ((flags & 0x400) != 0 && (special == 0 || (((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->flags & 0x80010) == 0))
            this->flags = flags | 0x4000;
    }
    this->velocity[0] = working.words[0] - original.words[0];
    this->velocity[1] = working.words[1] - original.words[1];
    this->velocity[2] = working.words[2] - original.words[2];
    if ((this->path_flags & 1) != 0 && (((const PathDefinition *)((((const ScenePath *)(camera_motion.path))->definition) != 0 ? (((const ScenePath *)(camera_motion.path))->definition) : (const PathDefinition *)0))->flags & 0x10000) == 0)
    {
        this->velocity[2] = 0;
        this->velocity[0] = 0;
        character_controller.motion.horizontal_speed = 0;
    }
    goto finish_segment;
finish_fall:
    if ((this->path_flags & 0x800) != 0)
        ClearCharacterPathStep();
    this->flags = this->flags | 0x4000;
    this->velocity[0] = working.words[0] - original.words[0];
    this->velocity[1] = working.words[1] - original.words[1];
    this->velocity[2] = working.words[2] - original.words[2];
    motion.words[0] = (uint32)initial_distance;
    motion.words[2] = 0;
    motion.words[1] = (uint32)base_height;
    squared = (uint32)(((sint64)initial_distance * initial_distance) >> 12);
    squared += (uint32)(((sint64)base_height * base_height) >> 12);
    if (squared != 0)
    {
        x = this->velocity[0];
        reciprocal = 0x40000000 / (sint32)squared;
        value = (uint32)(((sint64)(sint32)x * (sint32)x) >> 12);
        y = this->velocity[1];
        value += (uint32)(((sint64)(sint32)y * (sint32)y) >> 12);
        z = this->velocity[2];
        value += (uint32)(((sint64)(sint32)z * (sint32)z) >> 12);
        factor = (sint32)(uint32)(((sint64)(sint32)value * reciprocal) >> 18);
        fixed_math_result = (uint32)factor;
        if (factor >= 0)
            result = SquareRoot12((sint32)((uint32)factor));
        else
            result = SquareRoot12((sint32)((uint32)factor >> 2)) << 1;
    }
    else
        result = 4095;
    this->path_motion_scale = result;
    value = this->path_motion_scale;
    if (value != 0)
    {
        reciprocal = 0x1000000 / (sint32)value;
        x = (uint32)(((sint64)(sint32)this->velocity[0] * reciprocal) >> 12);
        value = this->path_motion_scale;
        if (value == 0)
        {
            xport_mips_break(7);
            reciprocal = -1;
        }
        else
            reciprocal = 0x1000000 / (sint32)value;
        this->velocity[0] = x;
        y = (uint32)(((sint64)(sint32)this->velocity[1] * reciprocal) >> 12);
        value = this->path_motion_scale;
        z = this->velocity[2];
        this->velocity[1] = y;
        if (value == 0)
        {
            xport_mips_break(7);
            reciprocal = -1;
        }
        else
            reciprocal = 0x1000000 / (sint32)value;
        this->velocity[2] = (uint32)(((sint64)(sint32)z * reciprocal) >> 12);
    }
finish_segment:
    result = 1;
    if (first_segment != 0)
    {
        result = 4096;
        if (this->path_frame == 1)
        {
            result = (uint32)((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->ProjectFrame(0, working.words, current.words, direction, 4096);
            this->path_frame = (sint16)(result);
        }
    }
    goto done;
direct_vector:
    if ((this->flags & 0x40) == 0)
        distance = (sint32)(0 - (uint32)distance);
    this->velocity[0] = (uint32)(((sint64)direction_vector->vx * distance) >> 12);
    this->velocity[1] = (uint32)(((sint64)direction_vector->vy * distance) >> 12);
    this->velocity[2] = (uint32)(((sint64)direction_vector->vz * distance) >> 12);
    result = this->velocity[1] + (uint32)height;
    this->velocity[1] = result;
done:
    return result;
}

const ScenePath *ScenePath::Step(uint32 *frame_word, uint32 direction) const
{
    FUNCTION_MARKER(0x8003AC94, "SLUS_005.78");
    uint32 forward;
    const ScenePath *current = this;
    const ScenePath *selected;
    const ScenePath *selected_view;
    const PathDefinition *definition;
    forward = (sint32)*frame_word >= 0;
    if (!forward)
    {
        if (current->node.state_flags & 8)
        {
            current = (current->adjacent[0]);
            current = (const ScenePath *)(current);
        }
        direction ^= 0x10;
    }
    if (current->node.state_flags & (direction | 8))
        selected = (current->adjacent[1]);
    else
        selected = (current->adjacent[0]);
    if (selected != 0)
    {
        if (forward)
            *frame_word = 0;
        else
        {
            if (selected->node.state_flags & 8)
            {
                selected = (selected->adjacent[0]);
            }
            selected_view = (const ScenePath *)(selected);
            definition = (const PathDefinition *)((const PathDefinition *)((selected_view->definition)));
            *frame_word = (uint32)(sint16)definition->positions.metadata6 - 1;
        }
    }
    return selected;
}

uint32 PathDefinition::SampleActorPosition(sint32 frame, uint32 *destination, uint32 direction) const
{
    const PathTransform *captured_path_transform;
    FUNCTION_MARKER(0x80056340, "SLUS_005.78");
    const MATRIX *matrix;
    PathVector vector;
    uint32 x, y, z, result;
    const PathTransform *matrix_transform;
    if (direction == 0)
        frame = (sint32)((uint32)(sint32)(sint16)positions.metadata6 - (uint32)frame - 1);
    matrix_transform = ((captured_path_transform = (this->transform)) != 0 ? (const PathTransform *)((PathTransform *)(captured_path_transform)) : 0);
    matrix = (matrix_transform != 0 ? &matrix_transform->matrix : (const MATRIX *)0);
    if (matrix_transform == 0)
        return SampleVectorTrack(frame, &positions, destination);
    SampleVectorTrack(frame, &positions, vector.words);
    PushMatrix();
    x = xport_load_le32((const uint8 *)matrix);
    y = xport_load_le32((const uint8 *)matrix + 4);
    xport_gte_write_control(0, x);
    xport_gte_write_control(1, y);
    x = xport_load_le32((const uint8 *)matrix + 8);
    y = xport_load_le32((const uint8 *)matrix + 12);
    z = xport_load_le32((const uint8 *)matrix + 16);
    xport_gte_write_control(2, x);
    xport_gte_write_control(3, y);
    xport_gte_write_control(4, z);
    ApplyRotMatrixLV(&vector.vector, (VECTOR *)destination);
    PopMatrix();
    x = destination[0];
    x += (uint32)matrix->t[0];
    destination[0] = x;
    y = destination[1];
    y += (uint32)matrix->t[1];
    destination[1] = y;
    result = destination[2];
    result += (uint32)matrix->t[2];
    destination[2] = result;
    return result;
}

sint32 ProjectPathStart(const ScenePath *path, uint32 *position, uint32 *floor, uint32 direction)
{
    FUNCTION_MARKER(0x800574B4, "SLUS_005.78");
    sint32 frame, previous_frame;
    uint32 previous[3], start[3];
    frame = (sint32)((const PathDefinition *)((const PathDefinition *)((path->definition))))->FindHeightFrame(position, direction, -40960);
    if (frame >= 0)
        frame = ((const PathDefinition *)((const PathDefinition *)((path->definition))))->ProjectFrame(frame, position, floor, direction, 8192);
    previous[0] = floor[0];
    previous[1] = floor[1];
    previous[2] = floor[2];
    previous_frame = frame;
    path->SamplePosition(0, floor, direction);
    start[0] = floor[0];
    start[1] = floor[1];
    start[2] = floor[2];
    position[0] = start[0];
    position[1] = start[1];
    position[2] = start[2];
    frame = ((const PathDefinition *)((const PathDefinition *)((path->definition))))->ProjectFrame(0, position, floor, direction, 8192);
    if (frame < 0)
    {
        frame = 0;
        floor[0] = start[0];
        floor[1] = start[1];
        floor[2] = start[2];
    }
    if (floor[0] - previous[0] + 24576 > 49152 || floor[2] - previous[2] + 24576 > 49152)
    {
        floor[0] = previous[0];
        floor[1] = previous[1];
        floor[2] = previous[2];
        return previous_frame;
    }
    return frame;
}

uint32 AnimatedObject::MarkPathSearchFailed()
{
    FUNCTION_MARKER(0x80058714, "SLUS_005.78");
    uint32 flags = path_flags | 0x800;
    path_flags = flags;
    return flags;
}

uint32 ScenePath::IsProjectionWithinHeight(const uint32 *position, uint32 direction) const
{
    FUNCTION_MARKER(0x8003BB10, "SLUS_005.78");
    FUNCTION_MARKER(0x8003BB18, "SLUS_005.78");
    uint32 floor[3];
    sint32 frame, result;
    if (this == camera_motion.path)
        return 0;
    frame = (sint32)definition->FindHeightFrame(position, direction, -327680);
    if (frame == -1)
        return 0;
    result = definition->ProjectFrame(frame, position, floor, direction, 4096);
    if (result != -1 && (sint32)(floor[1] - position[1]) > 0)
        result = -1;
    return result != -1;
}
