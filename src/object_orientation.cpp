#include "game.h"
#include "original_defaults.h"

uint32 path_heading_delta[3] = {0, 0, 0};

uint32 AnimatedObject::ComputeObjectFacing(const uint32 *direction_vector, Quaternion16 *axes) const
{
    const PathTransform *captured_path_transform;
    FUNCTION_MARKER(0x80018144, "SLUS_005.78");
    Quaternion16 local_quaternion, parent_quaternion, composed;
    Quaternion16 *quaternion = axes != 0 ? axes : &local_quaternion;
    const Quaternion16 *source;

    union
    {
        VECTOR vector;
        uint32 words[4];
    } transformed;

    union
    {
        uint32 words[5];
        uint16 halves[10];
    } matrix;

    const PathDefinition *definition;
    const ActorPlacement *placement;
    const ActorBinding *binding;
    const MATRIX *parent_matrix;
    const PathTransform *matrix_transform;
    const AnimationTrack *track;
    uint32 first_word, second_word, value, result;
    uint32 xx, yy, zz, xy, xz, yz, wx, wy, wz, x, y, z;
    sint32 frame, q0, q1, q2, q3, p0, p1, p2, p3;
    definition = (this->path_definition);
    if (definition != 0 && (definition->flags & 8) != 0)
    {
        placement = (const ActorPlacement *)definition;
        binding = (const ActorBinding *)(placement->state);
        track = binding->rotation_track;
        if (track != 0)
        {
            if ((this->flags & 0x10) != 0)
                frame = this->path_frame;
            else
            {
                frame = (sint16)definition->positions.metadata6;
                frame = (sint32)((uint32)frame - (uint32)(sint32)this->path_frame - 1);
            }
            SampleQuaternionTrack(frame, ((const AnimationTrack *)(track)), quaternion->components);
            matrix_transform = ((captured_path_transform = (((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform)) != 0 ? (const PathTransform *)((PathTransform *)(captured_path_transform)) : 0);
            parent_matrix = matrix_transform != 0 ? &matrix_transform->matrix : 0;
            if (parent_matrix != 0)
            {
                MatrixToQuaternion(parent_matrix, &parent_quaternion);
                q0 = quaternion->components[0];
                source = 0;
                if (q0 == 0x4000)
                    source = &parent_quaternion;
                else
                {
                    p0 = parent_quaternion.components[0];
                    if (p0 == 0x4000)
                        source = quaternion;
                    else
                    {
                        q1 = quaternion->components[1];
                        p1 = parent_quaternion.components[1];
                        q2 = quaternion->components[2];
                        p2 = parent_quaternion.components[2];
                        q3 = quaternion->components[3];
                        p3 = parent_quaternion.components[3];
                        x = (uint32)(q0 * p0) - ((uint32)(q1 * p1) + (uint32)(q2 * p2) + (uint32)(q3 * p3));
                        y = (uint32)(q0 * p1) + (uint32)(p0 * q1) + (uint32)(q2 * p3) - (uint32)(q3 * p2);
                        z = (uint32)(q0 * p2) + (uint32)(p0 * q2) + (uint32)(q3 * p1) - (uint32)(q1 * p3);
                        value = (uint32)(q0 * p3) + (uint32)(p0 * q3) + (uint32)(q1 * p2) - (uint32)(q2 * p1);
                        composed.components[0] = x >> 14;
                        composed.components[1] = y >> 14;
                        composed.components[2] = z >> 14;
                        composed.components[3] = (uint32)((sint32)value >> 14);
                    }
                }
                if (source != 0)
                {
                    first_word = source->words[0];
                    second_word = source->words[1];
                    composed.words[0] = first_word;
                    composed.words[1] = second_word;
                }
                first_word = composed.words[0];
                second_word = composed.words[1];
                quaternion->words[0] = first_word;
                quaternion->words[1] = second_word;
            }
            q1 = quaternion->components[1];
            q2 = quaternion->components[2];
            q3 = quaternion->components[3];
            q0 = quaternion->components[0];
            xx = (uint32)(q1 * q1);
            yy = (uint32)(q2 * q2);
            zz = (uint32)(q3 * q3);
            xy = (uint32)(q1 * q2);
            xz = (uint32)(q1 * q3);
            yz = (uint32)(q2 * q3);
            wz = (uint32)(q0 * q3);
            wy = (uint32)(q0 * q2);
            wx = (uint32)(q0 * q1);
            matrix.halves[0] = (uint32)((sint32)((uint32)0x08000000 - yy - zz) >> 15);
            matrix.halves[4] = (uint32)((sint32)((uint32)0x08000000 - xx - zz) >> 15);
            matrix.halves[8] = (uint32)((sint32)((uint32)0x08000000 - xx - yy) >> 15);
            matrix.halves[1] = (uint32)((sint32)(xy - wz) >> 15);
            matrix.halves[3] = (uint32)((sint32)(xy + wz) >> 15);
            matrix.halves[2] = (uint32)((sint32)(xz + wy) >> 15);
            matrix.halves[6] = (uint32)((sint32)(xz - wy) >> 15);
            matrix.halves[5] = (uint32)((sint32)(yz - wx) >> 15);
            matrix.halves[7] = (uint32)((sint32)(yz + wx) >> 15);
            PushMatrix();
            x = matrix.words[0];
            y = matrix.words[1];
            xport_gte_write_control(0, x);
            xport_gte_write_control(1, y);
            x = matrix.words[2];
            y = matrix.words[3];
            z = matrix.halves[8];
            xport_gte_write_control(2, x);
            xport_gte_write_control(3, y);
            xport_gte_write_control(4, z);
            ApplyRotMatrixLV((const VECTOR *)direction_vector, &transformed.vector);
            PopMatrix();
            direction_vector = transformed.words;
        }
    }
    if (direction_vector[0] + 32 >= 65 || direction_vector[2] + 32 >= 65)
    {
        z = direction_vector[2];
        x = direction_vector[0];
        result = (uint32)(sint32)(sint16)Atan2Angle((sint32)(0 - z), (sint32)(0 - x));
    }
    else
        result = 4096;
    return result;
}

uint32 AnimatedObject::TurnObjectFacing(const uint32 *direction_vector, sint16 step)
{
    const PathDefinition *definition;
    FUNCTION_MARKER(0x80040384, "SLUS_005.78");
    Quaternion16 axes;
    uint32 target, value, kind, x, y, z, squared, fast_turn, result;
    sint32 old_yaw = this->yaw;
    sint32 old_pitch = this->pitch;
    sint32 current_angle, delta, yaw_step, pitch_step, length, vertical;
    definition = (this->path_definition);
    if (definition != 0 && (((const PathDefinition *)(definition))->flags & 8) != 0 && ((const ActorBinding *)(((const ActorPlacement *)((const PathDefinition *)(definition)))->state != 0 ? ((const ActorPlacement *)((const PathDefinition *)(definition)))->state : 0))->rotation_track != 0)
    {
        target = ComputeObjectFacing(direction_vector, &axes);
        current_angle = this->yaw;
        this->target_yaw = (sint16)(target);
        if (current_angle == 4096)
            this->yaw = (sint16)(target);
        else
        {
            delta = (sint16)(target - (uint32)current_angle);
            if (delta < -2048)
                delta += 4096;
            else if (delta > 2048)
                delta -= 4096;
            delta = (sint16)delta;
            if (delta < step && -((sint32)step) < delta)
                this->yaw = (sint16)(target);
            else if (delta < 0)
                this->yaw = (sint16)((uint16)this->yaw - (uint32)(sint32)step);
            else
                this->yaw = (sint16)((uint16)this->yaw + (uint32)(sint32)step);
            this->yaw = (sint16)((uint16)this->yaw & 0xFFF);
        }
        ComposeHeadingQuaternion(this->yaw, &axes, (Quaternion16 *)orientation);
        goto finish;
    }
    if ((this->flags & 0x2000) != 0 || (direction_vector[0] + 32 < 65 && direction_vector[2] + 32 < 65))
        value = (uint16)this->target_yaw;
    else
        value = Atan2Angle((sint32)(0 - direction_vector[2]), (sint32)(0 - direction_vector[0]));
    fast_turn = 0;
    if ((sint8)metadata126[0] == 1)
    {
        kind = (uint16)(character_controller.motion.form_flags >> 16) & 0xF;
        fast_turn = kind - 5 < 2 || kind == 7 || kind - 10 < 2;
    }
    current_angle = this->yaw;
    if (fast_turn != 0)
    {
        target = (value + ((this->flags & 0x10) != 0 ? 1024 : (uint32)-1024)) & 0xFFF;
        yaw_step = 4096;
    }
    else
    {
        if ((sint16)value == current_angle)
            goto pitch;
        target = value & 0xFFF;
        yaw_step = step;
    }
    this->target_yaw = (sint16)(target);
    if (current_angle == 4096)
        this->yaw = (sint16)(target);
    else
    {
        delta = (sint16)(target - (uint32)current_angle);
        if (delta < -2048)
            delta += 4096;
        else if (delta > 2048)
            delta -= 4096;
        delta = (sint16)delta;
        if (delta < yaw_step && -yaw_step < delta)
            this->yaw = (sint16)(target);
        else if (delta < 0)
            this->yaw = (sint16)((uint16)this->yaw - (uint32)yaw_step);
        else
            this->yaw = (sint16)((uint16)this->yaw + (uint32)yaw_step);
        this->yaw = (sint16)((uint16)this->yaw & 0xFFF);
    }
    this->orientation[2] = (sint16)(((uint16)this->yaw + 2048) & 0xFFF);
pitch:
    if (this->pitch_scale == 4096)
        this->orientation[1] = (sint16)(0);
    else
    {
        x = (uint32)((sint32)direction_vector[0] >> 6);
        y = (uint32)((sint32)direction_vector[1] >> 6);
        z = (uint32)((sint32)direction_vector[2] >> 6);
        squared = x * x + y * y + z * z;
        if ((sint32)squared >= 0)
            length = (sint32)SquareRoot12((sint32)squared);
        else
            length = (sint32)(SquareRoot12((sint32)(squared >> 2)) << 1);
        vertical = (sint32)direction_vector[1];
        if (vertical < 0)
            vertical = (sint32)(0 - (uint32)vertical);
        if (length < vertical)
            length = vertical;
        value = (uint32)AsinAngle((sint32)direction_vector[1], length);
        target = (sint16)value == 4096 ? 0 : (0 - value) & 0xFFF;
        current_angle = this->pitch;
        if ((sint16)target != current_angle)
        {
            pitch_step = ((sint32)step) >> 1;
            target &= 0xFFF;
            if (current_angle == 4096)
                this->pitch = (sint16)(target);
            else
            {
                delta = (sint16)(target - (uint32)current_angle);
                if (delta < -2048)
                    delta += 4096;
                else if (delta > 2048)
                    delta -= 4096;
                delta = (sint16)delta;
                if (delta < pitch_step && -pitch_step < delta)
                    this->pitch = (sint16)(target);
                else if (delta < 0)
                    this->pitch = (sint16)((uint16)this->pitch - (uint32)pitch_step);
                else
                    this->pitch = (sint16)((uint16)this->pitch + (uint32)pitch_step);
                this->pitch = (sint16)((uint16)this->pitch & 0xFFF);
            }
            this->orientation[1] = (sint16)((uint16)this->pitch & 0xFFF);
        }
    }
    this->orientation[0] = (sint16)(0x7FFF);
finish:
    value = this->flags;
    current_angle = this->yaw;
    this->flags = value | 1;
    result = current_angle == old_yaw && this->pitch == old_pitch;
    return result;
}

uint32 SampleObjectPathDirection(uint16 *angle, uint32 *direction_vector, const AnimatedObject *object, sint32 frame, uint32 reverse)
{
    FUNCTION_MARKER(0x80035058, "SLUS_005.78");
    sint32 first_frame = frame;
    sint32 second_frame = (sint32)((uint32)frame + 1);
    uint32 first[3], second[3];
    uint32 x;
    uint32 y;
    uint32 z;
    uint32 result;
    /* Initialize the adjacent sample for the original undefined reverse-zero branch */
    if (reverse != 0 && object->path_frame >= (sint16)((const PathDefinition *)((object->path_definition)))->positions.metadata6)
    {
        second_frame = frame;
        first_frame = (sint32)((uint32)frame - 1);
    }
    ((const PathDefinition *)((const PathDefinition *)((object->path_definition))))->SampleActorPosition(first_frame, first, reverse);
    ((const PathDefinition *)((const PathDefinition *)((object->path_definition))))->SampleActorPosition(second_frame, second, reverse);
    x = second[0] - first[0];
    y = second[1] - first[1];
    z = second[2] - first[2];
    direction_vector[0] = x;
    direction_vector[1] = y;
    direction_vector[2] = z;
    result = Atan2Angle((sint32)z, (sint32)x);
    *angle = result;
    return result;
}

uint32 MatrixToQuaternion(const MATRIX *matrix, Quaternion16 *quaternion)
{
    FUNCTION_MARKER(0x80017E4C, "SLUS_005.78");
    sint32 first_diagonal = matrix->m[0][0];
    sint32 second_diagonal = matrix->m[1][1];
    sint32 third_diagonal = matrix->m[2][2];
    sint32 trace = first_diagonal + second_diagonal + third_diagonal;
    sint32 denominator;
    sint32 reciprocal;
    sint32 coefficient;
    uint32 product;
    uint32 result;
    if (trace > 0)
    {
        denominator = (sint32)(SquareRoot12((sint32)(((uint32)trace * 4 + 16384) >> 2)) * 4);
        if (denominator == 0)
            trap(7);
        reciprocal = 0x40000000 / denominator;
        quaternion->components[0] = denominator >> 1;
        coefficient = (sint32)(uint32)(((sint64)8192 * reciprocal) >> 14);
        product = (uint32)(matrix->m[1][2] - matrix->m[2][1]) * 4 * (uint32)coefficient;
        quaternion->components[1] = (sint32)product >> 14;
        product = (uint32)(matrix->m[2][0] - matrix->m[0][2]) * 4 * (uint32)coefficient;
        quaternion->components[2] = (sint32)product >> 14;
        product = (uint32)(matrix->m[0][1] - matrix->m[1][0]) * 4 * (uint32)coefficient;
        fixed_math_result = coefficient;
        result = (uint32)((sint32)product >> 14);
        quaternion->components[3] = result;
    }
    else
    {
        uint32 first = first_diagonal < second_diagonal;
        uint32 second;
        uint32 third;
        const sint16 *first_row;
        const sint16 *second_row;
        const sint16 *third_row;
        sint32 diagonal;
        if (matrix->m[first][first] < third_diagonal)
            first = 2;
        second = first + 1;
        if (second == 3)
            second = 0;
        third = second + 1;
        if (third == 3)
            third = 0;
        first_row = matrix->m[first];
        second_row = matrix->m[second];
        third_row = matrix->m[third];
        diagonal = first_row[first] - (second_row[second] + third_row[third]);
        denominator = (sint32)(SquareRoot12((sint32)(((uint32)diagonal * 4 + 16384) >> 2)) * 4);
        if (denominator == 0)
            trap(7);
        reciprocal = 0x40000000 / denominator;
        quaternion->components[first + 1] = denominator >> 1;
        coefficient = (sint32)(uint32)(((sint64)8192 * reciprocal) >> 14);
        product = (uint32)(second_row[third] - third_row[second]) * 4 * (uint32)coefficient;
        fixed_math_result = coefficient;
        quaternion->components[0] = (sint32)product >> 14;
        product = (uint32)(first_row[second] + second_row[first]) * 4 * (uint32)coefficient;
        quaternion->components[second + 1] = (sint32)product >> 14;
        product = (uint32)(first_row[third] + third_row[first]) * 4 * (uint32)coefficient;
        result = (uint32)((sint32)product >> 14);
        quaternion->components[third + 1] = result;
    }
    return result;
}

uint32 ComposeHeadingQuaternion(sint16 angle, const Quaternion16 *base, Quaternion16 *destination)
{
    FUNCTION_MARKER(0x80018FEC, "SLUS_005.78");
    Quaternion16 quaternion;
    const Quaternion16 *source;
    uint32 result;
    uint32 first_word, second_word, x, y, z;
    sint32 half_angle, sine, q0, q1, q2, q3, p0, p1, p2, p3;
    half_angle = (sint32)(((uint32)2048 - (uint32)(sint32)angle) << 16) >> 17;
    if (base == 0)
    {
        result = (uint32)SinQ14((sint16)(1024 - half_angle));
        destination->components[0] = result;
        destination->components[3] = 0;
        destination->components[1] = 0;
        result = (uint32)SinQ14((sint16)half_angle);
        destination->components[2] = result;
        return result;
    }
    result = (uint32)SinQ14((sint16)(1024 - half_angle));
    quaternion.components[0] = result;
    quaternion.components[3] = 0;
    quaternion.components[1] = 0;
    sine = SinQ14((sint16)half_angle);
    quaternion.components[2] = (uint32)sine;
    q0 = quaternion.components[0];
    source = 0;
    if (q0 == 0x4000)
        source = base;
    else
    {
        p0 = base->components[0];
        if (p0 == 0x4000)
            source = &quaternion;
        else
        {
            q1 = quaternion.components[1];
            p1 = base->components[1];
            p2 = base->components[2];
            q2 = (sint16)sine;
            q3 = quaternion.components[3];
            p3 = base->components[3];
            x = (uint32)(q0 * p0) - ((uint32)(q1 * p1) + (uint32)(q2 * p2) + (uint32)(q3 * p3));
            y = (uint32)(q0 * p1) + (uint32)(p0 * q1) + (uint32)(q2 * p3) - (uint32)(q3 * p2);
            z = (uint32)(q0 * p2) + (uint32)(p0 * q2) + (uint32)(q3 * p1) - (uint32)(q1 * p3);
            result = (uint32)(q0 * p3) + (uint32)(p0 * q3) + (uint32)(q1 * p2) - (uint32)(q2 * p1);
            destination->components[0] = x >> 14;
            destination->components[1] = y >> 14;
            destination->components[2] = z >> 14;
            result = (uint32)((sint32)result >> 14);
            destination->components[3] = result;
        }
    }
    if (source != 0)
    {
        first_word = source->words[0];
        second_word = source->words[1];
        result = first_word;
        destination->words[0] = first_word;
        destination->words[1] = second_word;
    }
    return result;
}

sint32 AnimatedObject::ComputeObjectPitch(const uint32 *direction_vector, Quaternion16 *axes) const
{
    const PathTransform *captured_path_transform;
    FUNCTION_MARKER(0x800185C8, "SLUS_005.78");
    Quaternion16 local_quaternion, parent_quaternion, composed;
    Quaternion16 *quaternion = axes != 0 ? axes : &local_quaternion;
    const Quaternion16 *source;

    union
    {
        VECTOR vector;
        uint32 words[4];
    } transformed;

    union
    {
        uint32 words[5];
        uint16 halves[10];
    } matrix;

    const PathDefinition *definition;
    const ActorPlacement *placement;
    const ActorBinding *binding;
    const MATRIX *parent_matrix;
    const PathTransform *matrix_transform;
    const AnimationTrack *track;
    uint32 first_word, second_word, value, result;
    uint32 xx, yy, zz, xy, xz, yz, wx, wy, wz, x, y, z;
    uint32 rotated, squared, magnitude, height;
    sint32 frame, q0, q1, q2, q3, p0, p1, p2, p3;
    rotated = 0;
    definition = (this->path_definition);
    if (definition != 0 && (definition->flags & 8) != 0)
    {
        placement = (const ActorPlacement *)definition;
        binding = (const ActorBinding *)(placement->state);
        track = binding->rotation_track;
        if (track != 0)
        {
            if ((this->flags & 0x10) != 0)
                frame = this->path_frame;
            else
            {
                frame = (sint16)definition->positions.metadata6;
                frame = (sint32)((uint32)frame - (uint32)(sint32)this->path_frame - 1);
            }
            SampleQuaternionTrack(frame, ((const AnimationTrack *)(track)), quaternion->components);
            matrix_transform = ((captured_path_transform = (((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform)) != 0 ? (const PathTransform *)((PathTransform *)(captured_path_transform)) : 0);
            parent_matrix = matrix_transform != 0 ? &matrix_transform->matrix : 0;
            if (parent_matrix != 0)
            {
                MatrixToQuaternion(parent_matrix, &parent_quaternion);
                q0 = quaternion->components[0];
                source = 0;
                if (q0 == 0x4000)
                    source = &parent_quaternion;
                else
                {
                    p0 = parent_quaternion.components[0];
                    if (p0 == 0x4000)
                        source = quaternion;
                    else
                    {
                        q1 = quaternion->components[1];
                        p1 = parent_quaternion.components[1];
                        q2 = quaternion->components[2];
                        p2 = parent_quaternion.components[2];
                        q3 = quaternion->components[3];
                        p3 = parent_quaternion.components[3];
                        x = (uint32)(q0 * p0) - ((uint32)(q1 * p1) + (uint32)(q2 * p2) + (uint32)(q3 * p3));
                        y = (uint32)(q0 * p1) + (uint32)(p0 * q1) + (uint32)(q2 * p3) - (uint32)(q3 * p2);
                        z = (uint32)(q0 * p2) + (uint32)(p0 * q2) + (uint32)(q3 * p1) - (uint32)(q1 * p3);
                        value = (uint32)(q0 * p3) + (uint32)(p0 * q3) + (uint32)(q1 * p2) - (uint32)(q2 * p1);
                        composed.components[0] = x >> 14;
                        composed.components[1] = y >> 14;
                        composed.components[2] = z >> 14;
                        composed.components[3] = (uint32)((sint32)value >> 14);
                    }
                }
                if (source != 0)
                {
                    first_word = source->words[0];
                    second_word = source->words[1];
                    composed.words[0] = first_word;
                    composed.words[1] = second_word;
                }
                first_word = composed.words[0];
                second_word = composed.words[1];
                quaternion->words[0] = first_word;
                quaternion->words[1] = second_word;
            }
            q1 = quaternion->components[1];
            q2 = quaternion->components[2];
            q3 = quaternion->components[3];
            q0 = quaternion->components[0];
            xx = (uint32)(q1 * q1);
            yy = (uint32)(q2 * q2);
            zz = (uint32)(q3 * q3);
            xy = (uint32)(q1 * q2);
            xz = (uint32)(q1 * q3);
            yz = (uint32)(q2 * q3);
            wz = (uint32)(q0 * q3);
            wy = (uint32)(q0 * q2);
            wx = (uint32)(q0 * q1);
            matrix.halves[0] = (uint32)((sint32)((uint32)0x08000000 - yy - zz) >> 15);
            matrix.halves[4] = (uint32)((sint32)((uint32)0x08000000 - xx - zz) >> 15);
            matrix.halves[8] = (uint32)((sint32)((uint32)0x08000000 - xx - yy) >> 15);
            matrix.halves[1] = (uint32)((sint32)(xy - wz) >> 15);
            matrix.halves[3] = (uint32)((sint32)(xy + wz) >> 15);
            matrix.halves[2] = (uint32)((sint32)(xz + wy) >> 15);
            matrix.halves[6] = (uint32)((sint32)(xz - wy) >> 15);
            matrix.halves[5] = (uint32)((sint32)(yz - wx) >> 15);
            matrix.halves[7] = (uint32)((sint32)(yz + wx) >> 15);
            PushMatrix();
            x = matrix.words[0];
            y = matrix.words[1];
            xport_gte_write_control(0, x);
            xport_gte_write_control(1, y);
            x = matrix.words[2];
            y = matrix.words[3];
            z = matrix.halves[8];
            xport_gte_write_control(2, x);
            xport_gte_write_control(3, y);
            xport_gte_write_control(4, z);
            ApplyRotMatrixLV((const VECTOR *)direction_vector, &transformed.vector);
            PopMatrix();
            direction_vector = transformed.words;
            rotated = 1;
        }
    }
    if (rotated != 0)
    {
        x = (uint32)((sint32)direction_vector[0] >> 6);
        y = (uint32)((sint32)direction_vector[1] >> 6);
        z = (uint32)((sint32)direction_vector[2] >> 6);
    }
    else
    {
        height = direction_vector[1];
        if (height + 32 < 65)
        {
            return 4096;
        }
        x = (uint32)((sint32)direction_vector[0] >> 6);
        y = (uint32)((sint32)height >> 6);
        z = (uint32)((sint32)direction_vector[2] >> 6);
    }
    squared = x * x + y * y + z * z;
    if ((sint32)squared >= 0)
        magnitude = SquareRoot12((sint32)squared);
    else
        magnitude = SquareRoot12((sint32)(squared >> 2)) << 1;
    if (rotated == 0)
    {
        height = direction_vector[1];
        if ((sint32)height < 0)
            height = 0 - height;
        if ((sint32)magnitude < (sint32)height)
            magnitude = height;
    }
    height = direction_vector[1];
    result = (uint32)AsinAngle((sint32)height, (sint32)magnitude);
    if ((sint16)result != 4096)
        result = (0 - result) & 0xFFF;
    return (sint32)(sint16)result;
}

uint32 AnimatedObject::SampleObjectPitch(const uint32 *direction_vector, Quaternion16 *quaternion) const
{
    const PathTransform *captured_path_transform;
    FUNCTION_MARKER(0x80018B40, "SLUS_005.78");
    Quaternion16 local_quaternion, object_rotation;

    union
    {
        uint32 words[5];
        uint16 halves[10];
    } matrix;

    union
    {
        VECTOR vector;
        uint32 words[4];
    } rotated;

    const ActorPlacement *placement;
    const ActorBinding *binding;
    const MATRIX *parent_matrix;
    const PathTransform *matrix_transform;
    const AnimationTrack *track;
    sint32 frame;
    uint32 square = 0;
    uint32 length;
    sint32 angle;
    uint32 result;
    uint32 index;
    if (quaternion == 0)
        quaternion = &local_quaternion;
    if ((this->flags & 16) != 0)
    {
        placement = (const ActorPlacement *)((const PathDefinition *)((this->path_definition)));
        binding = (const ActorBinding *)(placement->state);
        track = binding->rotation_track;
        frame = this->path_frame;
    }
    else
    {
        placement = (const ActorPlacement *)((const PathDefinition *)((this->path_definition)));
        binding = (const ActorBinding *)(placement->state);
        track = binding->rotation_track;
        frame = (sint16)placement->path.positions.metadata6 - this->path_frame - 1;
    }
    SampleQuaternionTrack(frame, ((const AnimationTrack *)(track)), quaternion->components);
    matrix_transform = ((captured_path_transform = (((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform)) != 0 ? (const PathTransform *)((PathTransform *)(captured_path_transform)) : 0);
    parent_matrix = matrix_transform != 0 ? &matrix_transform->matrix : 0;
    if (parent_matrix != 0)
    {
        uint32 first;
        uint32 second;
        MatrixToQuaternion(parent_matrix, &object_rotation);
        if (quaternion->components[0] == 16384)
        {
            first = object_rotation.words[0];
            second = object_rotation.words[1];
        }
        else if (object_rotation.components[0] == 16384)
        {
            first = quaternion->words[0];
            second = quaternion->words[1];
        }
        else
        {
            uint32 w = (uint32)(sint32)quaternion->components[0];
            uint32 x = (uint32)(sint32)quaternion->components[1];
            uint32 y = (uint32)(sint32)quaternion->components[2];
            uint32 z = (uint32)(sint32)quaternion->components[3];
            uint32 mw = (uint32)(sint32)object_rotation.components[0];
            uint32 mx = (uint32)(sint32)object_rotation.components[1];
            uint32 my = (uint32)(sint32)object_rotation.components[2];
            uint32 mz = (uint32)(sint32)object_rotation.components[3];
            uint32 combined_w = (w * mw - (x * mx + y * my + z * mz)) >> 14;
            uint32 combined_x = (w * mx + mw * x + y * mz - z * my) >> 14;
            uint32 combined_y = (w * my + mw * y + z * mx - x * mz) >> 14;
            uint32 combined_z = (uint32)((sint32)(w * mz + mw * z + x * my - y * mx) >> 14);
            first = (uint16)combined_w | ((uint32)(uint16)combined_x << 16);
            second = (uint16)combined_y | ((uint32)(uint16)combined_z << 16);
        }
        quaternion->words[0] = first;
        quaternion->words[1] = second;
    }
    {
        uint32 x = (uint32)(sint32)quaternion->components[1];
        uint32 y = (uint32)(sint32)quaternion->components[2];
        uint32 z = (uint32)(sint32)quaternion->components[3];
        uint32 w = (uint32)(sint32)quaternion->components[0];
        uint32 xx = x * x;
        uint32 yy = y * y;
        uint32 zz = z * z;
        uint32 xy = x * y;
        uint32 xz = x * z;
        uint32 yz = y * z;
        uint32 wx = w * x;
        uint32 wy = w * y;
        uint32 wz = w * z;
        matrix.halves[0] = (sint32)(0x8000000u - (yy + zz)) >> 15;
        matrix.halves[4] = (sint32)(0x8000000u - (xx + zz)) >> 15;
        matrix.halves[8] = (sint32)(0x8000000u - (xx + yy)) >> 15;
        matrix.halves[1] = (sint32)(xy - wz) >> 15;
        matrix.halves[3] = (sint32)(xy + wz) >> 15;
        matrix.halves[2] = (sint32)(xz + wy) >> 15;
        matrix.halves[6] = (sint32)(xz - wy) >> 15;
        matrix.halves[5] = (sint32)(yz - wx) >> 15;
        matrix.halves[7] = (sint32)(yz + wx) >> 15;
    }
    PushMatrix();
    for (index = 0; index < 4; ++index)
        xport_gte_write_control(index, matrix.words[index]);
    xport_gte_write_control(4, matrix.halves[8]);
    ApplyRotMatrixLV((const VECTOR *)direction_vector, &rotated.vector);
    PopMatrix();
    for (index = 0; index < 3; ++index)
    {
        uint32 scaled = (uint32)((sint32)rotated.words[index] >> 6);
        square += scaled * scaled;
    }
    if ((sint32)square >= 0)
        length = SquareRoot12((sint32)square);
    else
        length = SquareRoot12((sint32)(square >> 2)) * 2;
    angle = AsinAngle(rotated.vector.vy, (sint32)length);
    result = (sint16)angle == 4096 ? (uint32)angle : (0u - (uint32)angle) & 4095;
    return (uint32)(sint32)(sint16)result;
}

uint32 AnimatedObject::ReverseObjectFacingFromVelocity()
{
    FUNCTION_MARKER(0x80037F08, "SLUS_005.78");
    uint32 vector[3];
    uint32 result;
    vector[0] = 0u - this->velocity[0];
    vector[2] = 0u - this->velocity[2];
    vector[1] = this->velocity[1];
    TurnObjectFacing(vector, 4096);
    result = ((uint16)this->yaw + 2048) & 4095;
    this->target_yaw = (sint16)(result);
    this->yaw = (sint16)(result);
    return result;
}

sint32 SampleDefinitionHeading(sint32 frame0, sint32 frame1, const PathDefinition *definition)
{
    const PathTransform *captured_path_transform;
    FUNCTION_MARKER(0x8005642C, "SLUS_005.78");

    union
    {
        VECTOR vector;
        uint32 words[4];
    } pos[2], input;

    uint32 *dst;
    const MATRIX *matrix;
    uint32 sample, x, y, z, dx, dy, dz, result;
    const PathTransform *matrix_transform;
    sint32 frame;
    for (sample = 0; sample < 2; ++sample)
    {
        frame = sample == 0 ? frame0 : frame1;
        dst = pos[sample].words;
        matrix_transform = ((captured_path_transform = (definition->transform)) != 0 ? (const PathTransform *)((PathTransform *)(captured_path_transform)) : 0);
        if (matrix_transform != 0)
        {
            matrix = &matrix_transform->matrix;
            SampleVectorTrack(frame, &definition->positions, input.words);
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
            ApplyRotMatrixLV(&input.vector, (VECTOR *)dst);
            PopMatrix();
            x = dst[0];
            x += matrix->t[0];
            dst[0] = x;
            y = dst[1];
            y += matrix->t[1];
            dst[1] = y;
            z = dst[2];
            z += matrix->t[2];
            dst[2] = z;
        }
        else
            SampleVectorTrack(frame, &definition->positions, dst);
    }
    x = pos[1].words[0];
    dx = x - pos[0].words[0];
    y = pos[1].words[1];
    dy = y - pos[0].words[1];
    z = pos[1].words[2];
    dz = z - pos[0].words[2];
    path_heading_delta[0] = dx;
    path_heading_delta[1] = dy;
    path_heading_delta[2] = dz;
    if (dx == 0)
        result = (sint32)dz >= 0 ? 2048 : 0;
    else
        result = Atan2Angle((sint32)(0 - dz), (sint32)(0 - dx));
    return (sint16)result;
}

sint32 SampleScenePathHeading(sint32 frame0, sint32 frame1, const ScenePath *path)
{
    FUNCTION_MARKER(0x80056660, "SLUS_005.78");

    union
    {
        VECTOR vector;
        uint32 words[4];
    } pos[2];

    sint32 result;
    path->SamplePosition(frame0, pos[0].words, 64);
    path->SamplePosition(frame1, pos[1].words, 64);
    result = (sint16)Atan2Angle((sint32)(pos[0].words[2] - pos[1].words[2]), (sint32)(pos[0].words[0] - pos[1].words[0]));
    return result;
}

sint32 AnimatedObject::SampleActorPathHeading() const
{
    const PathDefinition *definition;
    FUNCTION_MARKER(0x800309BC, "SLUS_005.78");
    sint32 first = this->path_frame, second;
    definition = (this->path_definition);
    if ((this->flags & 0x10) != 0)
    {
        second = first + 1;
        if (first >= (sint16)((const PathDefinition *)(definition))->positions.metadata6)
        {
            second = first;
            --first;
        }
    }
    else
    {
        second = first - 1;
        if (first <= 0)
        {
            second = first;
            ++first;
        }
    }
    return (sint16)SampleDefinitionHeading(first, second, ((const PathDefinition *)(definition)));
}

uint32 AnimatedObject::SampleActorPathStart(const ScenePath *path, uint32 *dst) const
{
    FUNCTION_MARKER(0x80057480, "SLUS_005.78");
    return path->SamplePosition(0, dst, (this->path_flags & 0x400) == 0 ? 64 : 0);
}

uint32 AnimatedObject::TurnObjectYaw(uint32 target, sint16 step)
{
    FUNCTION_MARKER(0x800401F0, "SLUS_005.78");
    sint32 previous = this->yaw;
    sint32 difference;
    this->target_yaw = (sint16)(target);
    if (previous == 4096)
    {
        this->yaw = (sint16)(target);
        return 4096;
    }
    difference = (sint16)(target - (uint32)previous);
    if (difference < -2048)
        difference = (sint16)(difference + 4096);
    else if (difference > 2048)
        difference = (sint16)(difference - 4096);
    if (difference < step && difference > -step)
        this->yaw = (sint16)(target);
    else
        this->yaw = (sint16)((uint16)this->yaw + (difference < 0 ? -step : step));
    target = (uint16)this->yaw & 4095;
    this->yaw = (sint16)(target);
    return target;
}

uint32 AnimatedObject::TurnObjectPitch(uint32 target, sint16 step)
{
    FUNCTION_MARKER(0x800402BC, "SLUS_005.78");
    sint32 previous = this->pitch;
    sint32 difference;
    if (previous == 4096)
    {
        this->pitch = (sint16)(target);
        return 4096;
    }
    difference = (sint16)(target - (uint32)previous);
    if (difference < -2048)
        difference = (sint16)(difference + 4096);
    else if (difference > 2048)
        difference = (sint16)(difference - 4096);
    if (difference < step && difference > -step)
        this->pitch = (sint16)(target);
    else
        this->pitch = (sint16)((uint16)this->pitch + (difference < 0 ? -step : step));
    target = (uint16)this->pitch & 4095;
    this->pitch = (sint16)(target);
    return target;
}

uint32 UpdateAnimatedModelOrientation(const AnimatedModelDefinition *definition, AnimatedObject *object)
{
    FUNCTION_MARKER(0x80042320, "SLUS_005.78");
    uint32 first[4], delta[4];
    AnimatedObject *owner;
    SceneNode *sibling;
    uint32 direction, sample_frame, x, y, z, length, value, result;
    sint32 frame, heading, half_angle, complement;
    frame = object->path_frame;
    direction = object->flags & 0x10;
    owner = (AnimatedObject *)((SceneNode *)(object->node.links.first_child));
    if (frame > 0)
        --frame;
    sample_frame = (uint32)frame;
    ((const PathDefinition *)((const PathDefinition *)((object->path_definition))))->SamplePosition((sint16)sample_frame, first, direction);
    do
    {
        ++sample_frame;
        frame = (sint16)sample_frame;
        ((const PathDefinition *)((const PathDefinition *)((object->path_definition))))->SamplePosition(frame, delta, direction);
        x = first[0] - delta[0];
        y = first[1] - delta[1];
        z = first[2] - delta[2];
        delta[0] = x;
        delta[1] = y;
        delta[2] = z;
        heading = (sint16)Atan2Angle((sint32)z, (sint32)x);
        object->target_yaw = heading;
        if (heading != 4096)
            break;
    } while (frame < (sint16)((const PathDefinition *)((object->path_definition)))->positions.metadata6 - 1);
    if (direction == 0)
    {
        if (heading == 4096)
            goto update_pitch;
        object->target_yaw = ((uint32)heading + 2048) & 0xFFF;
    }
    heading = object->target_yaw;
    if (heading != 4096 && object->yaw != heading)
    {
        half_angle = (sint32)((2048 - (uint32)heading) << 16) >> 17;
        object->yaw = heading;
        result = (uint32)SinQ14((sint16)(1024 - (uint32)half_angle));
        object->orientation[0] = result;
        object->orientation[3] = 0;
        object->orientation[1] = 0;
        result = (uint32)SinQ14((sint16)half_angle);
        object->orientation[2] = result;
    }
update_pitch:
    if ((object->flags & 0x400) == 0)
    {
        if (delta[1] + 32 < 65)
            owner->pitch_scale = 0;
        else
        {
            x = (uint32)((sint32)delta[0] >> 6);
            y = (uint32)((sint32)delta[1] >> 6);
            z = (uint32)((sint32)delta[2] >> 6);
            value = x * x + y * y + z * z;
            if ((sint32)value < 0)
                length = SquareRoot12((sint32)(value >> 2)) << 1;
            else
                length = SquareRoot12((sint32)value);
            result = 0 - (uint32)AsinAngle((sint32)delta[1], (sint32)length);
            owner->pitch_scale = result & 0xFFF;
        }
    }
    result = definition->flags & 2;
    if (result != 0)
    {
        value = (uint16)owner->pitch_scale;
        if ((object->flags & 0x10) == 0)
            value = 0 - value;
        half_angle = (sint32)(value << 16) >> 17;
        complement = (sint32)(1024 - (uint32)half_angle);
        result = (uint32)SinQ14((sint16)complement);
        owner->orientation[0] = result;
        result = (uint32)SinQ14((sint16)half_angle);
        owner->orientation[1] = result;
        owner->orientation[3] = 0;
        owner->orientation[2] = 0;
        sibling = owner->node.links.sibling_next;
        if (sibling != 0)
        {
            owner = (AnimatedObject *)sibling;
            result = (uint32)SinQ14((sint16)complement);
            owner->orientation[0] = result;
            result = (uint32)SinQ14((sint16)half_angle);
            owner->orientation[1] = result;
            owner->orientation[3] = 0;
            owner->orientation[2] = 0;
        }
    }
    return result;
}

uint32 AnimatedObject::UpdateGroundShadow()
{
    const PathDefinition *parent;

    FUNCTION_MARKER(0x8005D1B0, "SLUS_005.78");
    uint32 floor[3], normal[3], velocity[3], input[3], extent0, extent1;
    uint32 floor_valid = 0;
    CollisionSphere position;
    SpriteRecord *shadow;
    SceneTrigger *trigger;
    const SceneTrigger *hit;
    const SceneSection *section;
    const MATRIX *matrix;
    const PathDefinition *owner, *candidate_owner, *next_definition;
    uint32 frame, direction, side, flags, value, x, y, z, a, b, c;
    const ScenePath *path, *first_seen, *second_seen, *next_path;
    const ScenePath *next;
    uint32 scene, walk_scene, best;
    uint32 radius, index, squared, magnitude, result;
    const sint16 *normal_axis;
    sint32 kind;
    shadow = (SpriteRecord *)(((SpriteRecord *)(this->sprite)));
    if (shadow->extent[0] == 0 || shadow->extent[1] == 0)
        goto hide;
    flags = this->path_flags;
    if ((flags & 0x11) != 0 || (flags & 0x300) == 0x100)
        goto hide;
    flags = this->flags;
    shadow->position[2] = 0;
    shadow->position[1] = 0;
    shadow->position[0] = 0;
    owner = (this->path_definition);
    side = 0xFFFFFFFF;
    frame = (uint16)this->path_frame;
    direction = flags & 0x10;
    if (owner == 0)
        goto no_path;
    if (((((const PathDefinition *)(owner)))->flags & 8) != 0 && ((const ActorBinding *)(((const ActorPlacement *)((const PathDefinition *)(owner)))->state != 0 ? ((const ActorPlacement *)((const PathDefinition *)(owner)))->state : 0))->rotation_track != 0)
        goto hide;
    if ((((const PathDefinition *)(owner)))->positions.keys == 0)
        goto no_path;
    flags = this->flags;
    hit = 0;
    if ((flags & 0x2400) == 0)
        goto orient;
    best = 0x7FFFF000;
    if ((flags & 0x2000) != 0)
    {
        x = this->position[0];
        y = this->position[1];
        z = this->position[2];
        position.position[0] = x;
        position.position[1] = y;
        position.position[2] = z;
        goto search_objects;
    }
    first_seen = 0;
    second_seen = 0;
    parent = (this->path_definition);
    if (parent != 0 && ((((const PathDefinition *)(parent)))->transform) != 0)
    {
        x = this->position[0];
        y = this->position[1];
        z = this->position[2];
        input[0] = x;
        input[1] = y;
        input[2] = z;
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
        ApplyRotMatrixLV((const VECTOR *)input, (VECTOR *)position.position);
        PopMatrix();
        for (index = 0; index < 3; ++index)
        {
            matrix = (const MATRIX *)((((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) != 0 ? &(((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform)->matrix : (MATRIX *)0);
            value = position.position[index];
            position.position[index] = value + matrix->t[index];
        }
    }
    else
    {
        x = this->position[0];
        y = this->position[1];
        z = this->position[2];
        position.position[0] = x;
        position.position[1] = y;
        position.position[2] = z;
    }
    scene = OriginalDefaults::SceneContextIdentity;
    path = camera_motion.path;
    if (path != 0)
    {
        walk_scene = scene;
        for (;;)
        {
            flags = ((const PathDefinition *)((const PathDefinition *)((path->definition))))->flags;
            if ((flags & 0x51) < 0x41)
                break;
            if ((flags & 0x10) == 0)
            {
                if ((path->node.state_flags & (direction | 8)) != 0)
                    path = (path->adjacent[1]);
                else
                    path = (path->adjacent[0]);
                if (first_seen == 0)
                    first_seen = path;
                else if (second_seen == 0)
                    second_seen = path;
                else if (path == first_seen || path == second_seen)
                    path = 0;
            }
            else
            {
                next_path = path->fallback;
                if (next_path == 0)
                    path = 0;
                else if ((path->node.state_flags & (direction | 8)) != 0)
                    path = next_path;
                else
                {
                    value = next_path->node.state_flags;
                    path = next_path;
                    if ((value & 8) != 0)
                        path = (next_path->adjacent[0]);
                }
            }
            if (path == 0)
                break;
            next_path = camera_motion.path;
            candidate_owner = (path->definition);
            next_definition = (((const ScenePath *)(next_path))->definition);
            if (candidate_owner == next_definition)
                path = 0;
            if (path == 0)
                break;
        }
        scene = OriginalDefaults::SceneContextIdentity;
    }
    if (path != camera_motion.path && path != 0)
    {
        frame = ((const PathDefinition *)((const PathDefinition *)((path->definition))))->FindHeightFrame(position.position, direction, -40960);
        if ((sint16)frame >= 0)
            owner = (path->definition);
    }
    frame = (uint32)(((const PathDefinition *)(owner)))->ProjectFrame((sint16)frame, position.position, floor, direction, 4096);
    floor_valid = 1;
    if ((sint16)frame < 0)
        owner = (this->path_definition);
    else
    {
        value = position.position[1];
        value -= floor[1];
        best = (uint32)((sint32)value / 50);
    }
search_objects:
    value = position.position[1];
    section = scene_stream.active_section;
    position.radius = 341;
    position.position[1] = value - 341;
    for (trigger = (SceneTrigger *)section->object_heads[4]; trigger != 0; trigger = (SceneTrigger *)trigger->node.links.registry_next)
    {
        if ((trigger->node.flags & 0x81) != 0)
            continue;
        kind = (sint8)(trigger->node.state_flags >> 8);
        if (kind != 6 && kind != 9)
            continue;
        radius = trigger->radius << 5;
        x = position.position[0];
        a = trigger->transform.t[0];
        radius += 20480;
        value = x - a + radius;
        extent0 = radius;
        if (value > (radius << 1))
            continue;
        value = position.position[2];
        value = value - trigger->transform.t[2] + radius;
        if (value > (radius << 1))
            continue;
        value = position.position[1];
        value = value - trigger->transform.t[1] + 204800;
        if (value > 409600)
            continue;
        velocity[2] = 0;
        velocity[0] = 0;
        velocity[1] = (uint32)-204800;
        value = SweepSphereAgainstObject(&position, &trigger->node, velocity, &extent0, 0);
        if (value != 0)
        {
            radius = extent0;
            if ((sint32)radius < (sint32)best)
            {
                best = radius;
                hit = trigger;
                side = value;
            }
        }
    }
    section = scene_stream.active_section;
    for (trigger = (SceneTrigger *)section->object_heads[6]; trigger != 0; trigger = (SceneTrigger *)trigger->node.links.registry_next)
    {
        if ((trigger->node.flags & 0x81) != 0)
            continue;
        next = (trigger->parameters.path_pair.targets[0]);
        if (next == 0)
            continue;
        candidate_owner = (((const ScenePath *)(next))->definition);
        if (candidate_owner == owner || (candidate_owner->flags & 0x11) != 0)
            continue;
        radius = trigger->radius << 5;
        x = position.position[0];
        a = trigger->transform.t[0];
        radius += 20480;
        value = x - a + radius;
        extent1 = radius;
        if (value > (radius << 1))
            continue;
        value = position.position[2];
        value = value - trigger->transform.t[2] + radius;
        if (value > (radius << 1))
            continue;
        value = position.position[1];
        value = value - trigger->transform.t[1] + 204800;
        if (value > 409600)
            continue;
        velocity[2] = 0;
        velocity[0] = 0;
        velocity[1] = (uint32)-204800;
        if (SweepSphereAgainstObject(&position, &trigger->node, velocity, &extent1, 0) != 0)
        {
            radius = extent1;
            if ((sint32)radius < (sint32)best)
            {
                side = 0xFFFFFFFF;
                best = radius;
                hit = trigger;
                if (direction != 0)
                    next = (hit->parameters.path_pair.targets[0]);
                else
                    next = (hit->parameters.path_pair.targets[1]);
                owner = (((const ScenePath *)(next))->definition);
            }
        }
    }
    if (hit != 0)
    {
        frame = (((const PathDefinition *)(owner)))->FindHeightFrame(position.position, direction, -40960);
        floor_valid = 0;
        if ((sint16)frame >= 0)
        {
            frame = (uint32)(((const PathDefinition *)(owner)))->ProjectFrame((sint16)frame, position.position, floor, direction, 4096);
            floor_valid = 1;
        }
    }
    else if ((this->flags & 0x2000) != 0 || ((((const PathDefinition *)(owner)))->flags & 0x11) != 0)
        goto hide;
    if (hit != 0)
    {
        kind = (sint8)(hit->node.state_flags >> 8);
        if (kind != 1 && (uint8)(kind - 24) >= 2)
        {
            shadow->position[1] = 0 - best * 50;
            --side;
            normal_axis = hit->transform.m[(sint32)side >> 1];
            value = (uint32)(sint32)normal_axis[0];
            normal[0] = (side & 1) != 0 ? value : 0 - value;
            value = (uint32)(sint32)normal_axis[1];
            normal[1] = (side & 1) != 0 ? value : 0 - value;
            value = (uint32)(sint32)normal_axis[2];
            normal[2] = (side & 1) != 0 ? value : 0 - value;
            goto orient;
        }
    }
    if (floor_valid == 0)
        goto hide;
    value = floor[1];
    shadow->position[1] = value - position.position[1];
    goto orient;
no_path:
    owner = 0;
    shadow->metadata_halfwords[0] = 0;
orient:
    if ((sint32)side >= 0)
    {
        value = (uint32)AsinAngle((sint32)normal[1], 4096);
        shadow->metadata_halfwords[0] = (1024 - value) & 0xFFF;
    }
    else if (owner != 0)
    {
        if ((sint16)frame > 0)
            --frame;
        (((const PathDefinition *)(owner)))->SamplePosition((sint16)frame, velocity, direction);
        do
        {
            ++frame;
            (((const PathDefinition *)(owner)))->SamplePosition((sint16)frame, floor, direction);
            x = floor[0];
            a = velocity[0];
            y = floor[1];
            b = velocity[1];
            z = floor[2];
            c = velocity[2];
            x -= a;
            y -= b;
            z -= c;
            normal[0] = x;
            normal[1] = y;
            normal[2] = z;
            x = (uint32)((sint32)x >> 6);
            y = (uint32)((sint32)y >> 6);
            z = (uint32)((sint32)z >> 6);
            squared = x * x + y * y + z * z;
            if ((sint32)squared < 0)
                magnitude = SquareRoot12((sint32)(squared >> 2)) << 1;
            else
                magnitude = SquareRoot12((sint32)squared);
        } while ((sint32)magnitude < 1024 && (sint16)frame < (sint16)(((const PathDefinition *)(owner)))->positions.metadata6 - 1);
        value = (uint32)AsinAngle((sint32)normal[1], (sint32)magnitude);
        shadow->metadata_halfwords[0] = value & 0xFFF;
    }
    shadow->metadata_halfwords[1] = (uint16)this->target_yaw;
    if (direction == 0)
    {
        value = shadow->metadata_halfwords[0];
        flags = shadow->metadata_halfwords[1];
        shadow->metadata_halfwords[0] = (0 - value) & 0xFFF;
        shadow->metadata_halfwords[1] = (flags + 2048) & 0xFFF;
    }
    result = shadow->node.flags & 0xFE;
    goto finish_model;
hide:
    result = shadow->node.flags | 1;
finish_model:
    shadow->node.flags = result;
    return result;
}

void RotateMatrixByQuaternion(MATRIX *destination, const MATRIX *source, const Quaternion16 *quaternion)
{
    FUNCTION_MARKER(0x8002F188, "SLUS_005.78");
    sint32 scalar = quaternion->components[0];
    MATRIX rotation = {0};
    sint32 x, y, z;
    uint32 xx, yy, zz, xy, xz, yz, sx, sy, sz;
    if (scalar == 0x4000)
    {
        uint32 first = xport_load_le32((const uint8 *)source + 0), second = xport_load_le32((const uint8 *)source + 4);
        uint32 third = xport_load_le32((const uint8 *)source + 8), fourth = xport_load_le32((const uint8 *)source + 12);
        sint16 last;
        xport_store_le32((uint8 *)destination + 0, first);
        xport_store_le32((uint8 *)destination + 4, second);
        xport_store_le32((uint8 *)destination + 8, third);
        xport_store_le32((uint8 *)destination + 12, fourth);
        last = source->m[2][2];
        destination->m[2][2] = last;
        return;
    }
    x = quaternion->components[1];
    y = quaternion->components[2];
    z = quaternion->components[3];
    xx = (uint32)(x * x);
    yy = (uint32)(y * y);
    zz = (uint32)(z * z);
    xy = (uint32)(x * y);
    xz = (uint32)(x * z);
    yz = (uint32)(y * z);
    sx = (uint32)(scalar * x);
    sy = (uint32)(scalar * y);
    sz = (uint32)(scalar * z);
    rotation.m[0][0] = (sint16)((sint32)(0x08000000 - (yy + zz)) >> 15);
    rotation.m[1][1] = (sint16)((sint32)(0x08000000 - (xx + zz)) >> 15);
    rotation.m[2][2] = (sint16)((sint32)(0x08000000 - (xx + yy)) >> 15);
    rotation.m[1][0] = (sint16)((sint32)(xy - sz) >> 15);
    rotation.m[0][1] = (sint16)((sint32)(xy + sz) >> 15);
    rotation.m[2][0] = (sint16)((sint32)(xz + sy) >> 15);
    rotation.m[0][2] = (sint16)((sint32)(xz - sy) >> 15);
    rotation.m[2][1] = (sint16)((sint32)(yz - sx) >> 15);
    rotation.m[1][2] = (sint16)((sint32)(yz + sx) >> 15);
    MulMatrix0((MATRIX *)source, &rotation, destination);
}
