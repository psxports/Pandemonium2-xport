#include <string.h>

#include "scene_path.h"
#include "game.h"
#include "hd.h"

void ScenePath::SamplePos(sint32 frame, Vec3 &dst, uint32 dir) const
{
    FUNCTION_MARKER(0x80022B20, "SLUS_005.78");
    MATRIX *matrix;
    const PathDefinition *definition_val;

    PathTransform *matrix_transform;
    Vec3 vector;
    if (dir == 0)
        frame = (uint32)definition->pos_track.frame_count - frame - 1;
    definition_val = definition;
    matrix_transform = definition_val->transform;
    matrix = matrix_transform != 0 ? &matrix_transform->matrix : NULL;
    if (matrix_transform != 0)
    {
        AnimationTrack::SampleVectorTrack(frame, &definition_val->pos_track, vector);
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
        AnimationTrack::SampleVectorTrack(frame, &definition_val->pos_track, dst);
    }
    hd_camera_sample(&definition_val->pos_track, matrix, (float)frame, &dst);
}

void ScenePath::SampleOffset(sint32 frame, Vec3 &dst, uint32 dir) const
{
    FUNCTION_MARKER(0x80022C24, "SLUS_005.78");
    MATRIX *matrix;

    Vec3 vector;
    if ((node.state_flags & (dir | 8)) == 0)
        frame = (uint32)definition->pos_track.frame_count - frame - 1;
    matrix = offset_matrix;
    if (matrix != 0)
    {
        AnimationTrack::SampleVectorTrack(frame, &offset_track, vector);
        PushMatrix();
        matrix = offset_matrix;
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
        matrix = offset_matrix;
        dst.x = (uint32)dst.x + matrix->t[0];
        matrix = offset_matrix;
        dst.y = (uint32)dst.y + matrix->t[1];
        matrix = offset_matrix;
        dst.z = (uint32)dst.z + matrix->t[2];
    }
    else
        AnimationTrack::SampleVectorTrack(frame, &offset_track, dst);
    hd_camera_sample(&offset_track, matrix, (float)frame, &dst);
}

const ScenePath *ScenePath::Step(uint32 *frame_word, uint32 dir) const
{
    FUNCTION_MARKER(0x8003AC94, "SLUS_005.78");
    uint32 forward;
    const ScenePath *cur = this;
    const ScenePath *sel;
    const ScenePath *sel_view;
    const PathDefinition *definition;
    forward = (sint32)*frame_word >= 0;
    if (!forward)
    {
        if (cur->node.state_flags & 8)
        {
            cur = cur->adjacent[0];
            cur = cur;
        }
        dir ^= 0x10;
    }
    if (cur->node.state_flags & (dir | 8))
        sel = cur->adjacent[1];
    else
        sel = cur->adjacent[0];
    if (sel != 0)
    {
        if (forward)
            *frame_word = 0;
        else
        {
            if (sel->node.state_flags & 8)
            {
                sel = sel->adjacent[0];
            }
            sel_view = sel;
            definition = sel_view->definition;
            *frame_word = (uint32)definition->pos_track.frame_count - 1;
        }
    }
    return sel;
}

const ScenePath *ScenePath::SelectAdjacent(uint32 *frame_word, uint32 dir, uint32 flags) const
{
    FUNCTION_MARKER(0x8003ADC0, "SLUS_005.78");
    const ScenePath *cur;
    const ScenePath *sel;
    const ScenePath *sel_view;
    const PathDefinition *definition;
    uint32 old_frame, mask, first_frame;
    old_frame = *frame_word;
    // Forced transitions use the same frame side as ordinary transitions
    first_frame = (sint32)old_frame >= 0;
    if ((flags & 0x2000000) != 0)
    {
        mask = node.state_flags;
        if ((mask & (((flags >> 6) & 0x10) | 8)) != 0)
            sel = adjacent[1];
        else
            sel = adjacent[0];
        if (sel != 0)
        {
            if (first_frame != 0)
                *frame_word = 0;
            else
            {
                definition = sel->definition;
                *frame_word = (uint32)definition->pos_track.frame_count - 1;
            }
        }
    }
    else
    {
        cur = this;
        if (first_frame == 0)
        {
            if ((node.state_flags & 8) != 0)
            {
                cur = adjacent[0];
                cur = cur;
            }
            dir ^= 0x10;
        }
        mask = cur->node.state_flags;
        if ((mask & (dir | 8)) != 0)
            sel = cur->adjacent[1];
        else
            sel = cur->adjacent[0];
        if (sel != 0)
        {
            if (first_frame != 0)
                *frame_word = 0;
            else
            {
                if ((sel->node.state_flags & 8) != 0)
                {
                    sel = sel->adjacent[0];
                }
                sel_view = sel;
                definition = sel_view->definition;
                *frame_word = (uint32)definition->pos_track.frame_count - 1;
            }
        }
    }
    if (sel != 0)
    {
        definition = sel->definition;
        if ((definition->flags & 0x10000) != 0)
        {
            *frame_word = old_frame;
            sel = 0;
        }
    }
    return sel;
}

uint32 ScenePath::IsProjWithinHeight(const Vec3 &pos, uint32 dir) const
{
    FUNCTION_MARKER(0x8003BB10, "SLUS_005.78");
    FUNCTION_MARKER(0x8003BB18, "SLUS_005.78");
    Vec3 floor;
    sint32 frame, result;
    if (this == camera_motion.path)
        return 0;
    frame = definition->FindHeightFrame(pos, dir, -327680);
    if (frame == -1)
        return 0;
    result = definition->ProjectFrame(frame, pos, floor, dir, 4096);
    if (result != -1 && (sint32)(floor.y - (uint32)pos.y) > 0)
        result = -1;
    return result != -1;
}

uint32 ScenePath::ProjectOffset(sint32 frame, Vec3 &dst, uint32 dir) const
{
    FUNCTION_MARKER(0x800451CC, "SLUS_005.78");
    Vec3 point, prev, following;
    uint32 x;
    uint32 y;
    uint32 z;
    sint32 numerator;
    uint32 denominator;
    uint32 ratio;
    sint32 scale;
    uint32 result;

    sint32 sel;
    sint32 count;
    sint32 gap;
    sint32 rcp;
    uint32 lift;
    sel = definition->ProjectFrame(frame, camera_motion.focus_pos, point, dir, 4096);
    if (sel < 0)
        sel = frame;
    gap = camera_motion.focus_pos.y - (uint32)point.y;
    if (sel <= 0)
    {
        SamplePos(0, point, dir);

        SampleOffset(0, dst, dir);
    }
    else
    {
        count = definition->pos_track.frame_count;
        if (sel < count)
        {
            SamplePos(((uint32)sel - 1), prev, dir);

            x = point.x - (uint32)prev.x;
            y = point.y - (uint32)prev.y;
            z = point.z - (uint32)prev.z;
            point.x = x;
            point.y = y;
            point.z = z;
            x = (sint32)x >> 6;
            y = (sint32)y >> 6;
            z = (sint32)z >> 6;

            numerator = x * x + y * y + z * z;
            SampleOffset(((uint32)sel - 1), dst, dir);

            SamplePos(sel, following, dir);

            x = following.x - (uint32)prev.x;
            y = following.y - (uint32)prev.y;
            z = following.z - (uint32)prev.z;
            following.x = x;
            following.y = y;
            following.z = z;
            x = (sint32)x >> 6;
            y = (sint32)y >> 6;
            z = (sint32)z >> 6;
            denominator = x * x + y * y + z * z;
            if (denominator == 0)
            {
                SampleOffset(sel, dst, dir);
            }
            else
            {
                rcp = 0x40000000 / (sint32)denominator;
                ratio = ((sint64)numerator * rcp) >> 18;
                fixed_math_result = ratio;
                SampleOffset(sel, point, dir);

                point.x = point.x - (uint32)dst.x;
                point.y = point.y - (uint32)dst.y;
                point.z = point.z - (uint32)dst.z;
                ratio = fixed_math_result;
                if ((sint32)ratio >= 0)
                {
                    scale = SquareRoot12(ratio);
                }
                else
                {
                    scale = SquareRoot12((ratio >> 2)) << 1;
                }

                point.x = mul12(point.x, scale);
                point.y = mul12(point.y, scale);
                point.z = mul12(point.z, scale);
                dst.x = dst.x + (uint32)point.x;
                dst.y = dst.y + (uint32)point.y;
                dst.z = dst.z + (uint32)point.z;
            }
        }
        else
        {
            SamplePos(count - 1, point, dir);

            SampleOffset(definition->pos_track.frame_count - 1, dst, dir);
        }
    }
    result = node.state_flags & 1;
    if (result != 0)
    {
        result = 5120;
        if (gap > 0)
        {
            lift = mul12(gap, 5120);
            if ((sint32)(lift - gap) > 61440)
                lift = (uint32)gap + 61440;
            result = dst.y + lift;
            dst.y = result;
        }
    }
    hd_camera_project(this, sel, dir, &dst);
    return result;
}

sint16 ScenePath::SampleScenePathHeading(sint32 frame0, sint32 frame1, const ScenePath *path)
{
    FUNCTION_MARKER(0x80056660, "SLUS_005.78");

    Vec3 pos[2];

    sint32 result;
    path->SamplePos(frame0, pos[0], 64);
    path->SamplePos(frame1, pos[1], 64);
    result = Atan2Angle((pos[0].z - (uint32)pos[1].z), (pos[0].x - (uint32)pos[1].x));
    return result;
}

sint32 ScenePath::ProjectPathStart(const ScenePath *path, Vec3 &pos, Vec3 &floor, uint32 dir)
{
    FUNCTION_MARKER(0x800574B4, "SLUS_005.78");
    sint32 frame, prev_frame;
    Vec3 prev;
    Vec3 start;
    frame = path->definition->FindHeightFrame(pos, dir, -40960);
    if (frame >= 0)
        frame = path->definition->ProjectFrame(frame, pos, floor, dir, 8192);
    prev = floor;
    prev_frame = frame;
    path->SamplePos(0, floor, dir);
    start = floor;
    pos = start;
    frame = path->definition->ProjectFrame(0, pos, floor, dir, 8192);
    if (frame < 0)
    {
        frame = 0;
        floor = start;
    }
    if (floor.x - (uint32)prev.x + 24576 > 49152 || floor.z - (uint32)prev.z + 24576 > 49152)
    {
        floor = prev;
        return prev_frame;
    }
    return frame;
}
