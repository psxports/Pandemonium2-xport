#include "game.h"

uint32 p2_func_8001b334(uint32 object_offset, sint32 step)
{
    FUNCTION_MARKER(0x8001B334, "SLUS_005.78");
    uint32 animation, group, track, count, stride, first, flags, result;
    uint32 changed;
    if ((r_u32(object_offset + 28) & 0x30000) != 0)
        step = 0;
    animation = r_u32(object_offset + 104);
    changed = step == 0;
    if (animation != 0)
    {
        for (group = 0; group < 4; ++group)
        {
            count = (uint32)r_s8(animation + group);
            if (count == 0)
                continue;
            stride = group == 0 ? 20 : (group == 3 ? 24 : 16);
            track = r_u32(animation + 8 + group * 4);
            if (group < 2)
                track += count * stride - stride;
            first = 1;
            do
            {
                if (first != 0 && group < 2 && (r_u8(track + 2) & 4) != 0 && step != 0)
                {
                    if (group == 0)
                    {
                        if (r_s16(track + 8) == 0 && r_s32(object_offset + 92) < 0)
                            changed = 1;
                        else if ((p2_func_8001a76c(track, object_offset + 16, step, r_s32(object_offset + 92)) & 0xFF) != 0)
                            changed = 1;
                    }
                    else
                    {
                        w_u16(track + 8, r_u16(r_u32(animation + 8) + 8));
                        changed = 1;
                        p2_func_8001aaa8(r_u32(animation + 8), track, object_offset + 16, object_offset + 84, step);
                    }
                    goto next_track;
                }
                if (step == 0)
                {
                    flags = r_u32(object_offset + 28);
                    if ((flags & 0x30000) != 0)
                    {
                        if ((flags & 0x20000) != 0)
                            w_u16(track + 8, 0);
                        else if ((r_u8(track + 2) & 2) != 0)
                            w_u16(track + 8, r_u16(track + 4) - 1);
                        else
                            w_u16(track + 8, r_u16(track + 4));
                    }
                    else if ((r_u8(track + 2) & 2) != 0)
                    {
                        sint32 initial = r_s16(object_offset + 130);
                        sint32 limit = r_s16(track + 4);
                        sint32 remainder;
                        if (limit == 0)
                        {
                            xport_mips_break(7);
                            remainder = initial;
                        }
                        else
                            remainder = initial % limit;
                        w_u16(track + 8, (uint32)remainder);
                    }
                    else
                    {
                        sint32 initial = r_s16(object_offset + 130);
                        sint32 limit = r_s16(track + 4);
                        w_u16(track + 8, (uint32)(initial < limit ? initial : limit));
                    }
                }
                else
                {
                    uint32 old = r_u16(track + 8);
                    uint32 limit = r_u16(track + 4);
                    uint32 sum = old + (uint32)step;
                    uint32 frame = sum & 0xFFFF;
                    w_u16(track + 8, sum);
                    if (frame >= limit)
                    {
                        if ((r_u8(track + 2) & 2) != 0)
                            w_u16(track + 8, (sum & 0x8000) != 0 ? limit - 1 : 0);
                        else if (limit < frame)
                        {
                            w_u16(track + 8, sum - (uint32)step);
                            goto next_track;
                        }
                    }
                    changed = 1;
                }
                w_u8(track + 2, r_u8(track + 2) | 8);
            next_track:
                first = 0;
                --count;
                if (group < 2)
                    track -= stride;
                else
                    track += stride;
            } while (count != 0);
        }
    }
    flags = r_u32(object_offset + 28);
    if (changed != 0)
        flags = (flags & 0xFFFCFFFF) | 1;
    else
        flags &= 0xFFFBFFFF;
    w_u32(object_offset + 28, flags);
    result = r_u8(object_offset + 13) | 0x40;
    w_u8(object_offset + 13, result);
    return result;
}

uint32 p2_func_80048af0(uint32 object_offset, uint32 character_offset, sint32 mode)
{
    CharacterModelRecord *model_record;
    FUNCTION_MARKER(0x80048AF0, "SLUS_005.78");
    uint32 model, flags, value, packed, category, result = 1;
    sint32 selected;
    ResetHud();
    category = (uint16)(character_controller.motion.form_flags >> 16) & 0xF;
    w_u32(character_offset + 500, 0);
    {
        ObjectCallback operation = {category == 5 ? UpdateForm5Movement : UpdateCharacterMovement, 0, 0};
        object_operations.motions[1] = operation;
    }
    {
        ObjectCallback operation = {p2_func_800530a0, 0, 0};
        object_operations.behaviors[1] = operation;
    }
    model = character_controller.model_offset;
    model_record = (CharacterModelRecord *)psx_addr(model, 240);
    w_u8(character_offset + 714, 0);
    w_u16(r_u32(model_record->object_offset + 156) + 40, (uint32)(model_record->extent[0] >> 5));
    model = character_controller.model_offset;
    model_record = (CharacterModelRecord *)psx_addr(model, 240);
    w_u16(r_u32(model_record->object_offset + 156) + 42, (uint32)(model_record->extent[1] >> 5));
    value = game_flow.transition_flags;
    flags = character_controller.flags;
    game_flow.transition_flags = value & 0xFFFFFEFF;
    character_controller.flags = flags & 0xBFFFFFFF;
    character_controller.ClearTransientFlags();
    character_controller.flags = character_controller.flags & 0xC0000000;
    flags = r_u32(object_offset + 28);
    value = r_u32(object_offset + 228);
    w_u32(object_offset + 28, flags & 0xC0F793FF);
    w_u32(object_offset + 228, value & 0xFC000000);
    flags = character_controller.secondary_flags;
    value = game_flow.mode_flags;
    character_controller.motion.jump_phase = 0;
    character_controller.motion.vertical_speed = 0;
    character_controller.motion.horizontal_speed = 0;
    character_controller.secondary_flags = flags & 0xFFFFF932;
    w_u32(object_offset + 100, 0);
    w_u32(object_offset + 96, 0);
    w_u32(object_offset + 92, 0);
    character_controller.timers.special_move_ticks = 0;
    character_controller.timers.transition_ticks = 0;
    character_controller.timers.score_bonus_ticks = 0;
    character_controller.timers.action_cooldown_ticks = 0;
    character_controller.vitals.damage_ticks = 0;
    character_controller.timers.state_ticks = 0;
    w_u32(character_offset + 784, 0);
    character_controller.timers.contact_impulse_ticks = 0;
    w_u32(character_offset + 552, 0);
    if ((value & 0x10) == 0 && mode != 0)
        w_u32(character_offset + 20, 0);
    if (mode == 2)
    {
        character_controller.vitals.lives = 3;
        character_controller.vitals.health_limit = 4;
        character_controller.vitals.health = 4;
        character_controller.vitals.score = 0;
        if ((character_controller.secondary_flags & 0x80) == 0)
            character_controller.motion.state_code = 0;
        result = game_flow.transition_flags | 0x20000;
        game_flow.transition_flags = result;
    }
    else if (mode == 1)
    {
        value = p2_func_800255f8(r_u32(0x8008A4E8));
        character_controller.vitals.health = value;
        selected = (sint8)value;
        if (selected == -1)
        {
            packed = character_controller.motion.form_flags & 0xFFF0FFFF;
            category = ((packed >> 20) & 0xF) << 16;
        }
        else
        {
            value = ResolveCharacterFormVariant(selected, character_offset);
            character_controller.vitals.health = value;
            packed = character_controller.motion.form_flags;
            selected = (sint8)character_controller.vitals.health;
            packed &= 0xFFF0FFFF;
            category = (uint32)selected << 16;
            character_controller.motion.form_flags = packed;
        }
        character_controller.motion.form_flags = packed | category;
        w_u32(object_offset + 28, r_u32(object_offset + 28) | 0x10);
        character_controller.vitals.health = character_controller.vitals.health_limit;
        if ((character_controller.secondary_flags & 0x80) == 0)
            character_controller.motion.state_code = 0;
        SetSceneTransition(0, 0, r_u32(0x8008A4E8), -2, 0);
        result = 15;
        w_u16(r_u32(0x8008A114) + 46, result);
    }
    return result;
}





uint32 p2_func_8003db7c(uint32 object_offset, sint32 distance, sint32 height, const VECTOR *direction_vector)
{
    FUNCTION_MARKER(0x8003DB7C, "SLUS_005.78");
    uint32 saved_stack = p2_guest_stack_pointer;
    uint32 working, original, next, current, temporary;
    uint32 initial_path = 0, path, character = 0, special = 0, special_path = 0;
    uint32 matrix, flags, value, x, y, z, squared, direction, track_direction, candidate;
    uint32 i, stage, stop, valid, first_segment;
    uint32 previous_x, previous_y, previous_z, result;
    sint32 category, frame, limit, drop = 0, base_height, initial_distance, reciprocal, factor, length, height_step;
    p2_guest_stack_pointer -= 0x68;
    working = p2_guest_stack_pointer + 0x18;
    original = p2_guest_stack_pointer + 0x28;
    next = p2_guest_stack_pointer + 0x38;
    current = p2_guest_stack_pointer + 0x48;
    temporary = p2_guest_stack_pointer + 0x58;
    category = r_s8(object_offset + 126);
    if (category == 1)
    {
        if (r_s8(object_offset + 125) != 33)
            initial_path = r_u32(r_u32(0x8008A114) + 136);
        else
            initial_path = r_u32(object_offset + 112);
    }
    else if (category == 3 && r_s8(object_offset + 120) == -1)
        initial_path = r_u32(object_offset + 112);
    w_u8(0x8008A50C, 0);
    category = r_s8(object_offset + 126);
    if (category == 1)
        character = 0x800ABD68;
    else if (r_s8(object_offset + 124) == 2 && (r_u32(object_offset + 116) == 19 || r_u32(0x8008A498) != 0))
    {
        p2_func_800536f0(0);
        special = 1;
        if ((r_u32(r_u32(object_offset + 132) + 16) & 0x80000) != 0)
            special_path = 1;
    }
    if (special != 0 && (r_u32(r_u32(object_offset + 132) + 16) & 0x10000) != 0)
    {
        matrix = r_u32(object_offset + 132);
        if (matrix != 0 && r_u32(matrix + 40) != 0)
        {
            x = r_u32(object_offset + 16);
            y = r_u32(object_offset + 20);
            z = r_u32(object_offset + 24);
            w_u32(temporary, x);
            w_u32(temporary + 4, y);
            w_u32(temporary + 8, z);
            PushMatrix();
            matrix = r_u32(r_u32(object_offset + 132) + 40);
            for (i = 0; i < 5; ++i)
                xport_gte_write_control(i, r_u32(matrix + i * 4));
            p2_sdk_8007ec64(temporary, original);
            PopMatrix();
            for (i = 0; i < 3; ++i)
            {
                matrix = r_u32(r_u32(object_offset + 132) + 40);
                w_u32(original + i * 4, r_u32(original + i * 4) + r_u32(matrix + 20 + i * 4));
            }
        }
        else
        {
            x = r_u32(object_offset + 16);
            y = r_u32(object_offset + 20);
            z = r_u32(object_offset + 24);
            w_u32(original, x);
            w_u32(original + 4, y);
            w_u32(original + 8, z);
        }
        x = r_u32(original);
        y = r_u32(original + 4);
        z = r_u32(original + 8);
        w_u32(current, x);
        w_u32(current + 4, y);
        w_u32(current + 8, z);
        x = r_u32(current);
        y = r_u32(current + 4);
        z = r_u32(current + 8);
        w_u32(working, x);
        w_u32(working + 4, y);
        w_u32(working + 8, z);
        ChangeActorPath(object_offset, r_u32(r_u32(0x8008A114) + 136), working, current);
    }
    initial_distance = distance;
    base_height = height;
    if (distance < 0)
    {
        initial_distance = 0;
        distance = 0;
    }
    flags = r_u32(object_offset + 228);
    if ((flags & 0x440000) != 0)
    {
        p2_func_80056884(object_offset, character);
        goto check_direct;
    }
    if ((r_u32(object_offset + 28) & 0x2000) != 0 && (flags & 4) != 0)
        goto prepare_contact;
    if ((r_u32(object_offset + 228) & 0x20) == 0)
        goto check_direct;
    if ((r_u32(object_offset + 28) & 0x2000) == 0)
        goto prepare_position;
prepare_contact:
    matrix = r_u32(object_offset + 132);
    if (matrix != 0 && r_u32(matrix + 40) != 0)
    {
        x = r_u32(object_offset + 16);
        y = r_u32(object_offset + 20);
        z = r_u32(object_offset + 24);
        w_u32(temporary, x);
        w_u32(temporary + 4, y);
        w_u32(temporary + 8, z);
        PushMatrix();
        matrix = r_u32(r_u32(object_offset + 132) + 40);
        for (i = 0; i < 5; ++i)
            xport_gte_write_control(i, r_u32(matrix + i * 4));
        p2_sdk_8007ec64(temporary, original);
        PopMatrix();
        for (i = 0; i < 3; ++i)
        {
            matrix = r_u32(r_u32(object_offset + 132) + 40);
            w_u32(original + i * 4, r_u32(original + i * 4) + r_u32(matrix + 20 + i * 4));
        }
    }
    else
    {
        x = r_u32(object_offset + 16);
        y = r_u32(object_offset + 20);
        z = r_u32(object_offset + 24);
        w_u32(original, x);
        w_u32(original + 4, y);
        w_u32(original + 8, z);
    }
    x = r_u32(original);
    y = r_u32(original + 4);
    z = r_u32(original + 8);
    w_u32(current, x);
    w_u32(current + 4, y);
    w_u32(current + 8, z);
    x = r_u32(current);
    y = r_u32(current + 4);
    z = r_u32(current + 8);
    w_u32(working, x);
    w_u32(working + 4, y);
    w_u32(working + 8, z);
    ChangeActorPath(object_offset, r_u32(r_u32(0x8008A114) + 136), working, current);
check_direct:
    if ((r_u32(object_offset + 28) & 0x2000) != 0)
        goto direct_vector;
prepare_position:
    matrix = r_u32(object_offset + 132);
    if (matrix != 0 && r_u32(matrix + 40) != 0)
    {
        x = r_u32(object_offset + 16);
        y = r_u32(object_offset + 20);
        z = r_u32(object_offset + 24);
        w_u32(temporary, x);
        w_u32(temporary + 4, y);
        w_u32(temporary + 8, z);
        PushMatrix();
        matrix = r_u32(r_u32(object_offset + 132) + 40);
        for (i = 0; i < 5; ++i)
            xport_gte_write_control(i, r_u32(matrix + i * 4));
        p2_sdk_8007ec64(temporary, original);
        PopMatrix();
        for (i = 0; i < 3; ++i)
        {
            matrix = r_u32(r_u32(object_offset + 132) + 40);
            w_u32(original + i * 4, r_u32(original + i * 4) + r_u32(matrix + 20 + i * 4));
        }
    }
    else
    {
        x = r_u32(object_offset + 16);
        y = r_u32(object_offset + 20);
        z = r_u32(object_offset + 24);
        w_u32(original, x);
        w_u32(original + 4, y);
        w_u32(original + 8, z);
    }
    x = r_u32(original);
    y = r_u32(original + 4);
    z = r_u32(original + 8);
    w_u32(current, x);
    w_u32(current + 4, y);
    w_u32(current + 8, z);
    x = r_u32(current);
    y = r_u32(current + 4);
    z = r_u32(current + 8);
    w_u32(working, x);
    w_u32(working + 4, y);
    w_u32(working + 8, z);
    direction = r_u32(object_offset + 28);
    direction &= 0x10;
    if ((r_u32(object_offset + 28) & 0x400) == 0 && (r_u32(object_offset + 228) & 0x1222) == 0)
        goto prepare_next;
    for (;;)
    {
        result = (uint32)ProjectPathFrame(r_u32(object_offset + 132), r_s16(object_offset + 136), working, current, direction, 4096);
        if ((sint32)result >= 0)
            break;
        frame = r_s16(object_offset + 136);
        if (frame != r_s16(r_u32(object_offset + 132) + 26) - 1)
        {
            direction_vector = (const VECTOR *)psx_addr(working, 12);
            if (r_s8(object_offset + 126) == 1)
            {
                p2_func_8004fa54(object_offset, working);
                goto direct_vector;
            }
            break;
        }
        w_u16(object_offset + 136, (uint32)frame - 1);
    }
    if (r_s32(working + 4) < r_s32(current + 4) && base_height >= 0)
        w_u32(current + 4, r_u32(working + 4));
    if (distance == 0 && base_height < 0 && (r_u32(object_offset + 228) & 0x20) == 0)
    {
        x = r_u32(current) - r_u32(working);
        y = r_u32(current + 4) - r_u32(working + 4);
        z = r_u32(current + 8) - r_u32(working + 8);
        w_u32(0x1F800238, x);
        w_u32(0x1F80023C, y);
        w_u32(0x1F800240, z);
        if ((sint32)y < 0)
        {
            w_u32(0x1F80023C, 0);
            reciprocal = 0x40000000 / (sint32)y;
            if (x + 26754 > 0xD103 || z + 26754 > 0xD103)
            {
                x = r_u32(0x1F800238);
                y = r_u32(0x1F80023C);
                z = r_u32(0x1F800240);
                squared = (uint32)(((sint64)(sint32)x * (sint32)x) >> 12);
                squared += (uint32)(((sint64)(sint32)y * (sint32)y) >> 12);
                squared += (uint32)(((sint64)(sint32)z * (sint32)z) >> 12);
            }
            else
                squared = (uint32)((sint32)(x * x + z * z) >> 12);
            value = p2_sdk_8007e794(squared);
            factor = (sint32)(uint32)(((sint64)(sint32)value * base_height) >> 12);
            distance = (sint32)(uint32)(((sint64)factor * reciprocal) >> 18);
            w_u32(0x1F80030C, (uint32)distance);
            initial_distance = distance;
            if (distance < 0)
            {
                distance = 0;
                initial_distance = 0;
            }
        }
    }
    drop = (sint32)(r_u32(working + 4) - r_u32(current + 4));
    w_u32(current, r_u32(working));
    w_u32(current + 8, r_u32(working + 8));
    if (drop < -1365 && (r_u32(object_offset + 228) & 0x20) == 0 && (special_path & special) == 0 && (character == 0 || ((uint16)(character_controller.motion.form_flags >> 16) & 0xF) != 5) && (r_u32(r_u32(object_offset + 132) + 16) & 0x11) == 0)
    {
        x = r_u32(current);
        y = r_u32(current + 4);
        z = r_u32(current + 8);
        w_u32(working, x);
        w_u32(working + 4, y);
        w_u32(working + 8, z);
        initial_distance = 0;
        base_height = 0;
        first_segment = 0;
        goto finish_fall;
    }
prepare_next:
    x = r_u32(current);
    y = r_u32(current + 4);
    z = r_u32(current + 8);
    w_u32(next, x);
    w_u32(next + 4, y);
    w_u32(next + 8, z);
restart_path:
    frame = r_s16(object_offset + 136);
    first_segment = frame < 1;
    if (frame >= 1 && (r_u32(object_offset + 28) & 0x40) != 0)
        w_u16(object_offset + 136, (uint32)frame - 1);
    category = r_s8(object_offset + 126);
    path = 0;
    if (category == 1)
    {
        if (r_s8(object_offset + 125) != 33)
            path = r_u32(r_u32(0x8008A114) + 136);
        else
            path = r_u32(object_offset + 112);
    }
    else if (category == 3 && r_s8(object_offset + 120) == -1)
        path = r_u32(object_offset + 112);
    stop = 0;
    if (base_height <= 0)
    {
        flags = r_u32(r_u32(object_offset + 132) + 16);
        if ((flags & 0x51) >= 0x41 && ((flags & 0x10) == 0 || r_u32(path + 28) != 0))
            stop = 1;
    }
next_segment:
    track_direction = direction;
    candidate = path;
    frame = r_s16(object_offset + 136);
    if ((r_u32(object_offset + 28) & 0x40) != 0)
        frame += 1;
    else
        frame -= 1;
    matrix = r_u32(object_offset + 132);
    w_u16(object_offset + 136, (uint32)frame);
    valid = 1;
    if (((uint32)frame & 0xFFFF) < r_u16(matrix + 26))
    {
        SampleWorldPath(matrix, (sint16)frame, next, track_direction);
        if (path != 0 && ((r_u32(object_offset + 28) & 0x400) != 0 || (r_u32(object_offset + 228) & 0x1022) != 0))
        {
            value = (uint32)r_s16(object_offset + 136);
            if ((r_u32(object_offset + 28) & 0x40) == 0)
            {
                if ((r_u16(candidate + 14) & 8) != 0)
                    candidate = r_u32(candidate + 16);
                track_direction ^= 0x10;
                value = (uint32)r_s16(r_u32(object_offset + 132) + 26) - value - 1;
            }
            if ((r_u16(candidate + 14) & (track_direction | 8)) != 0)
                limit = r_s16(candidate + 24);
            else
                limit = r_s16(candidate + 26);
            if (value >= (uint32)limit && limit != -1)
            {
                if ((uint32)limit < value)
                    w_u32(object_offset + 28, r_u32(object_offset + 28) | 0x2000);
                valid = 0;
                w_u32(object_offset + 28, r_u32(object_offset + 28) | 0x4000);
            }
        }
    }
    else
    {
        valid = 0;
        w_u32(object_offset + 28, r_u32(object_offset + 28) | 0x800);
    }
    if (valid != 0 && (stop == 0 || drop >= 0) && (r_u32(object_offset + 228) & 8) == 0)
        goto advance;
    flags = r_u32(object_offset + 228);
    if ((flags & 0x80000) != 0)
    {
        result = (flags & 0xFFF7FFFF) | 0x100000;
        w_u32(object_offset + 228, result);
        w_u32(object_offset + 92, 0);
        w_u32(object_offset + 96, 0);
        goto done;
    }
    flags = r_u32(object_offset + 28);
    if ((flags & 0x6000) != 0 && drop < 0)
    {
        w_u32(object_offset + 28, flags & 0xFFFF9FFF);
        if ((flags & 0x800) == 0 && stop == 0)
            goto advance;
    }
    if (distance == 0 && (r_u32(object_offset + 228) & 8) == 0)
    {
        x = r_u32(current);
        y = r_u32(current + 4);
        z = r_u32(current + 8);
        w_u32(next, x);
        w_u32(next + 4, y);
        w_u32(next + 8, z);
        flags = r_u32(object_offset + 28);
        w_u32(object_offset + 28, flags & 0xFFFFF7FF);
        if ((flags & 0x6000) == 0 && (stop == 0 || drop >= 0))
            goto advance;
    }
    previous_x = r_u32(object_offset + 92);
    previous_y = r_u32(object_offset + 96);
    previous_z = r_u32(object_offset + 100);
    if ((r_u32(object_offset + 228) & 2) != 0 && (r_u32(r_u32(r_u32(r_u32(0x8008A114) + 136) + 36) + 16) & 0x10000) == 0 && p2_func_80056328() != 0)
        goto end_contact;
    if (character != 0 && r_u32(character + 500) != 0 && (uint32)initial_distance - 1 < 0x3FF && base_height < 512)
        goto end_contact;
    if ((ChangeActorPath(object_offset, path, working, current) & 0xFF) == 0)
        goto end_contact;
    if ((r_u32(r_u32(path + 36) + 16) & 0x40) != 0)
    {
        drop = (sint32)(r_u32(working + 4) - r_u32(current + 4));
        if (drop < 1366)
        {
            if ((r_u32(object_offset + 28) & 0x400) != 0)
                goto adjust_velocity;
            if ((r_u32(object_offset + 228) & 0x1022) == 0)
            {
                x = r_u32(current);
                y = r_u32(current + 4);
                z = r_u32(current + 8);
                w_u32(working, x);
                w_u32(working + 4, y);
                w_u32(working + 8, z);
            }
        }
        else
            w_u32(object_offset + 28, r_u32(object_offset + 28) | 0x400);
    }
    if ((r_u32(object_offset + 28) & 0x400) == 0 && (r_u32(object_offset + 228) & 0x1022) == 0)
        goto check_path_change;
adjust_velocity:
    x = r_u32(object_offset + 92) - previous_x;
    w_u32(0x1F800238, x);
    y = r_u32(object_offset + 96) - previous_y;
    w_u32(0x1F80023C, y);
    z = r_u32(object_offset + 100) - previous_z;
    w_u32(0x1F800240, z);
    base_height = (sint32)((uint32)base_height + y);
    w_u32(0x1F80023C, 0);
    height = (sint32)((uint32)height + y);
    if (x + 26754 > 0xD103 || z + 26754 > 0xD103)
    {
        x = r_u32(0x1F800238);
        y = r_u32(0x1F80023C);
        z = r_u32(0x1F800240);
        squared = (uint32)(((sint64)(sint32)x * (sint32)x) >> 12);
        squared += (uint32)(((sint64)(sint32)y * (sint32)y) >> 12);
        squared += (uint32)(((sint64)(sint32)z * (sint32)z) >> 12);
    }
    else
        squared = (uint32)((sint32)(x * x + z * z) >> 12);
    w_u32(0x1F80023C, p2_sdk_8007e794(squared));
    value = Atan2Angle(r_s32(0x1F800240), r_s32(0x1F800238));
    if ((((uint32)(sint16)value - (uint32)r_s16(object_offset + 150) - 1024) & 0xFFF) >= 0x800)
        w_u32(0x1F80023C, 0 - r_u32(0x1F80023C));
    if ((sint32)((uint32)distance + r_u32(0x1F80023C)) <= 0)
        w_u32(0x1F80023C, 1 - (uint32)distance);
    initial_distance = (sint32)((uint32)initial_distance + r_u32(0x1F80023C));
    distance = (sint32)((uint32)distance + r_u32(0x1F80023C));
check_path_change:
    for (stage = 2; stage <= 5; ++stage)
    {
        if (path == initial_path)
            break;
        category = r_s8(object_offset + 126);
        value = 0;
        if (category == 1)
        {
            if (r_s8(object_offset + 125) != 33)
                value = r_u32(r_u32(0x8008A114) + 136);
            else
                value = r_u32(object_offset + 112);
        }
        else if (category == 3 && r_s8(object_offset + 120) == -1)
            value = r_u32(object_offset + 112);
        if (value != initial_path)
            continue;
        if (stage == 2 && (r_u32(r_u32(path + 36) + 16) & 0x10) == 0)
            continue;
        if (stage == 3 && (r_u32(r_u32(r_u32(r_u32(0x8008A114) + 136) + 36) + 16) & 0x80000) == 0)
            continue;
        if (stage == 4 && (r_u32(r_u32(initial_path + 36) + 16) & 0x10) == 0)
            continue;
        if (stage == 5 && special == 0)
            continue;
        if (r_s8(0x8008A50C) == 0)
        {
            flags = r_u32(object_offset + 28);
            w_u8(0x8008A50C, stage);
            w_u32(object_offset + 28, flags | 0x800);
            goto end_contact;
        }
    }
    w_u32(object_offset + 28, r_u32(object_offset + 28) & 0xFFFFF7FF);
    goto restart_path;
end_contact:
    flags = r_u32(object_offset + 28);
    w_u32(object_offset + 28, flags & 0xFFFF9FFF);
    if ((flags & 0x800) == 0)
        goto advance;
    category = r_s8(object_offset + 126);
    if (category == 1 || (((uint32)category - 3) & 0xFF) < 2)
    {
        SamplePathTangent(r_u32(object_offset + 132), (VECTOR *)psx_addr(0x1F800238, 12), (sint32)direction);
        if ((r_u32(object_offset + 28) & 0x400) != 0 || (r_u32(object_offset + 228) & 0x1002) != 0)
        {
            x = r_u32(0x1F800238);
            w_u32(0x1F80023C, 0);
            if (x + 26754 > 0xD103 || r_u32(0x1F800240) + 26754 > 0xD103)
            {
                do
                {
                    x = (uint32)(r_s32(0x1F800238) >> 1);
                    y = (uint32)(r_s32(0x1F80023C) >> 1);
                    z = (uint32)(r_s32(0x1F800240) >> 1);
                    w_u32(0x1F800238, x);
                    w_u32(0x1F80023C, y);
                    w_u32(0x1F800240, z);
                } while (x + 26754 > 0xD103 || y + 26754 > 0xD103 || z + 26754 > 0xD103);
            }
            VectorNormal((VECTOR *)psx_addr(0x1F800238, 12), (VECTOR *)psx_addr(0x1F800238, 12));
        }
        if ((r_u32(object_offset + 28) & 0x40) == 0)
            distance = (sint32)(0 - (uint32)distance);
        for (i = 0; i < 3; ++i)
            w_u32(0x1F800238 + i * 4, (uint32)(((sint64)r_s32(0x1F800238 + i * 4) * distance) >> 12));
        x = r_u32(working) + r_u32(0x1F800238);
        y = r_u32(working + 4) + r_u32(0x1F80023C);
        z = r_u32(working + 8) + r_u32(0x1F800240);
        w_u32(working, x);
        w_u32(working + 4, y);
        w_u32(working + 8, z);
        w_u32(working + 4, y + (uint32)height);
    }
    w_u32(object_offset + 92, r_u32(working) - r_u32(original));
    w_u32(object_offset + 96, r_u32(working + 4) - r_u32(original + 4));
    result = r_u32(original + 8);
    w_u32(object_offset + 100, r_u32(working + 8) - result);
    goto done;
advance:
    if ((r_u32(object_offset + 28) & 0x400) != 0 || (r_u32(object_offset + 228) & 0x1222) != 0)
    {
        x = r_u32(next) - r_u32(current);
        y = r_u32(next + 4) - r_u32(current + 4);
        z = r_u32(next + 8) - r_u32(current + 8);
        w_u32(0x1F800238, x);
        w_u32(0x1F80023C, y);
        w_u32(0x1F800240, z);
        x = r_u32(next);
        y = r_u32(next + 4);
        z = r_u32(next + 8);
        w_u32(current, x);
        w_u32(current + 4, y);
        w_u32(current + 8, z);
        value = r_u32(0x1F80023C);
        x = r_u32(0x1F800238);
        w_u32(0x1F80023C, 0);
        w_u32(next + 4, value);
        z = r_u32(0x1F800240);
        if (x + 26754 > 0xD103 || z + 26754 > 0xD103)
        {
            x = r_u32(0x1F800238);
            y = r_u32(0x1F80023C);
            z = r_u32(0x1F800240);
            squared = (uint32)(((sint64)(sint32)x * (sint32)x) >> 12);
            squared += (uint32)(((sint64)(sint32)y * (sint32)y) >> 12);
            squared += (uint32)(((sint64)(sint32)z * (sint32)z) >> 12);
        }
        else
            squared = (uint32)((sint32)(x * x + z * z) >> 12);
        length = (sint32)p2_sdk_8007e794(squared);
        w_u32(0x1F80023C, r_u32(next + 4));
    }
    else
    {
        x = r_u32(next) - r_u32(working);
        y = r_u32(next + 4) - r_u32(working + 4);
        z = r_u32(next + 8) - r_u32(working + 8);
        w_u32(0x1F800238, x);
        w_u32(0x1F80023C, y);
        w_u32(0x1F800240, z);
        if (x + 26754 > 0xD103 || y + 26754 > 0xD103 || z + 26754 > 0xD103)
        {
            x = r_u32(0x1F800238);
            y = r_u32(0x1F80023C);
            z = r_u32(0x1F800240);
            squared = (uint32)(((sint64)(sint32)x * (sint32)x) >> 12);
            squared += (uint32)(((sint64)(sint32)y * (sint32)y) >> 12);
            squared += (uint32)(((sint64)(sint32)z * (sint32)z) >> 12);
        }
        else
            squared = (uint32)((sint32)(x * x + y * y + z * z) >> 12);
        length = (sint32)p2_sdk_8007e794(squared);
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
        flags = r_u32(object_offset + 28);
        w_u32(0x1F80030C, (uint32)factor);
        distance = 0;
        if ((flags & 0x40) == 0)
            w_u16(object_offset + 136, r_u16(object_offset + 136) + 1);
        factor = r_s32(0x1F80030C);
        for (i = 0; i < 3; ++i)
            w_u32(0x1F800238 + i * 4, (uint32)(((sint64)r_s32(0x1F800238 + i * 4) * factor) >> 12));
        height_step = height;
    }
    else if (height != 0)
    {
        if (distance != 0)
        {
            reciprocal = 0x40000000 / distance;
            factor = (sint32)(uint32)(((sint64)length * reciprocal) >> 18);
            distance = (sint32)((uint32)distance - (uint32)length);
            w_u32(0x1F80030C, (uint32)factor);
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
    x = r_u32(working) + r_u32(0x1F800238);
    y = r_u32(working + 4) + r_u32(0x1F80023C);
    z = r_u32(working + 8) + r_u32(0x1F800240);
    w_u32(working, x);
    w_u32(working + 4, y);
    w_u32(working + 8, z);
    if ((r_u32(object_offset + 28) & 0x400) != 0 || (r_u32(object_offset + 228) & 0x1222) != 0)
    {
        height = (sint32)((uint32)height - (uint32)height_step);
        value = (uint32)height_step - r_u32(0x1F80023C);
        y += value;
        drop = (sint32)((uint32)drop + value);
        w_u32(working + 4, y);
        if (drop < -169 && (base_height <= 0 || (first_segment == 0 && (uint32)drop + 5461 >= 0x1001)) && (character == 0 || ((uint16)(character_controller.motion.form_flags >> 16) & 0xF) != 5) && (r_u32(r_u32(object_offset + 132) + 16) & 0x11) == 0 && ((r_u32(r_u32(r_u32(r_u32(0x8008A114) + 136) + 36) + 16) & 0x80000) == 0 || (r_u32(object_offset + 228) & 0x20) == 0) && (special_path & special) == 0)
        {
            w_u32(working + 4, y - (uint32)drop);
            goto finish_fall;
        }
    }
    if (distance != 0)
        goto next_segment;
    if (drop <= 0 && r_s8(object_offset + 125) != 1)
    {
        flags = r_u32(object_offset + 28);
        if ((flags & 0x400) != 0 && (special == 0 || (r_u32(r_u32(object_offset + 132) + 16) & 0x80010) == 0))
            w_u32(object_offset + 28, flags | 0x4000);
    }
    w_u32(object_offset + 92, r_u32(working) - r_u32(original));
    w_u32(object_offset + 96, r_u32(working + 4) - r_u32(original + 4));
    w_u32(object_offset + 100, r_u32(working + 8) - r_u32(original + 8));
    if ((r_u32(object_offset + 228) & 1) != 0 && (r_u32(r_u32(r_u32(r_u32(0x8008A114) + 136) + 36) + 16) & 0x10000) == 0)
    {
        w_u32(object_offset + 100, 0);
        w_u32(object_offset + 92, 0);
        character_controller.motion.horizontal_speed = 0;
    }
    goto finish_segment;
finish_fall:
    if ((r_u32(object_offset + 228) & 0x800) != 0)
        p2_func_80058728(object_offset);
    w_u32(object_offset + 28, r_u32(object_offset + 28) | 0x4000);
    w_u32(object_offset + 92, r_u32(working) - r_u32(original));
    w_u32(object_offset + 96, r_u32(working + 4) - r_u32(original + 4));
    w_u32(object_offset + 100, r_u32(working + 8) - r_u32(original + 8));
    w_u32(0x1F800238, (uint32)initial_distance);
    w_u32(0x1F800240, 0);
    w_u32(0x1F80023C, (uint32)base_height);
    squared = (uint32)(((sint64)initial_distance * initial_distance) >> 12);
    squared += (uint32)(((sint64)base_height * base_height) >> 12);
    if (squared != 0)
    {
        x = r_u32(object_offset + 92);
        reciprocal = 0x40000000 / (sint32)squared;
        value = (uint32)(((sint64)(sint32)x * (sint32)x) >> 12);
        y = r_u32(object_offset + 96);
        value += (uint32)(((sint64)(sint32)y * (sint32)y) >> 12);
        z = r_u32(object_offset + 100);
        value += (uint32)(((sint64)(sint32)z * (sint32)z) >> 12);
        factor = (sint32)(uint32)(((sint64)(sint32)value * reciprocal) >> 18);
        w_u32(0x1F80030C, (uint32)factor);
        if (factor >= 0)
            result = p2_sdk_8007e794((uint32)factor);
        else
            result = p2_sdk_8007e794((uint32)factor >> 2) << 1;
    }
    else
        result = 4095;
    w_u32(object_offset + 144, result);
    value = r_u32(object_offset + 144);
    if (value != 0)
    {
        reciprocal = 0x1000000 / (sint32)value;
        x = (uint32)(((sint64)r_s32(object_offset + 92) * reciprocal) >> 12);
        value = r_u32(object_offset + 144);
        if (value == 0)
        {
            xport_mips_break(7);
            reciprocal = -1;
        }
        else
            reciprocal = 0x1000000 / (sint32)value;
        w_u32(object_offset + 92, x);
        y = (uint32)(((sint64)r_s32(object_offset + 96) * reciprocal) >> 12);
        value = r_u32(object_offset + 144);
        z = r_u32(object_offset + 100);
        w_u32(object_offset + 96, y);
        if (value == 0)
        {
            xport_mips_break(7);
            reciprocal = -1;
        }
        else
            reciprocal = 0x1000000 / (sint32)value;
        w_u32(object_offset + 100, (uint32)(((sint64)(sint32)z * reciprocal) >> 12));
    }
finish_segment:
    result = 1;
    if (first_segment != 0)
    {
        result = 4096;
        if (r_s16(object_offset + 136) == 1)
        {
            result = (uint32)ProjectPathFrame(r_u32(object_offset + 132), 0, working, current, direction, 4096);
            w_u16(object_offset + 136, result);
        }
    }
    goto done;
direct_vector:
    if ((r_u32(object_offset + 28) & 0x40) == 0)
        distance = (sint32)(0 - (uint32)distance);
    w_u32(object_offset + 92, (uint32)(((sint64)direction_vector->vx * distance) >> 12));
    w_u32(object_offset + 96, (uint32)(((sint64)direction_vector->vy * distance) >> 12));
    w_u32(object_offset + 100, (uint32)(((sint64)direction_vector->vz * distance) >> 12));
    result = r_u32(object_offset + 96) + (uint32)height;
    w_u32(object_offset + 96, result);
done:
    p2_guest_stack_pointer = saved_stack;
    return result;
}







uint32 p2_func_80040384(uint32 object_offset, uint32 vector_offset, sint16 step)
{
    FUNCTION_MARKER(0x80040384, "SLUS_005.78");
    uint32 saved_stack = p2_guest_stack_pointer;
    uint32 axes, definition, target, value, kind, x, y, z, squared, fast_turn, result;
    sint32 old_yaw = r_s16(object_offset + 148);
    sint32 old_pitch = r_s16(object_offset + 152);
    sint32 current_angle, delta, yaw_step, pitch_step, length, vertical;
    p2_guest_stack_pointer -= 0x48;
    axes = p2_guest_stack_pointer + 0x10;
    definition = r_u32(object_offset + 132);
    if (definition != 0 && (r_u32(definition + 16) & 8) != 0 && r_u32(r_u32(definition + 44) + 56) != 0)
    {
        target = p2_func_80018144(object_offset, vector_offset, axes);
        current_angle = r_s16(object_offset + 148);
        w_u16(object_offset + 150, target);
        if (current_angle == 4096)
            w_u16(object_offset + 148, target);
        else
        {
            delta = (sint16)(target - (uint32)current_angle);
            if (delta < -2048)
                delta += 4096;
            else if (delta > 2048)
                delta -= 4096;
            delta = (sint16)delta;
            if (delta < step && -((sint32)step) < delta)
                w_u16(object_offset + 148, target);
            else if (delta < 0)
                w_u16(object_offset + 148, r_u16(object_offset + 148) - (uint32)(sint32)step);
            else
                w_u16(object_offset + 148, r_u16(object_offset + 148) + (uint32)(sint32)step);
            w_u16(object_offset + 148, r_u16(object_offset + 148) & 0xFFF);
        }
        p2_func_80018fec(r_s16(object_offset + 148), axes, object_offset + 84);
        goto finish;
    }
    if ((r_u32(object_offset + 28) & 0x2000) != 0 || (r_u32(vector_offset) + 32 < 65 && r_u32(vector_offset + 8) + 32 < 65))
        value = r_u16(object_offset + 150);
    else
        value = Atan2Angle((sint32)(0 - r_u32(vector_offset + 8)), (sint32)(0 - r_u32(vector_offset)));
    fast_turn = 0;
    if (r_s8(object_offset + 126) == 1)
    {
        kind = (uint16)(character_controller.motion.form_flags >> 16) & 0xF;
        fast_turn = kind - 5 < 2 || kind == 7 || kind - 10 < 2;
    }
    current_angle = r_s16(object_offset + 148);
    if (fast_turn != 0)
    {
        target = (value + ((r_u32(object_offset + 28) & 0x10) != 0 ? 1024 : (uint32)-1024)) & 0xFFF;
        yaw_step = 4096;
    }
    else
    {
        if ((sint16)value == current_angle)
            goto pitch;
        target = value & 0xFFF;
        yaw_step = step;
    }
    w_u16(object_offset + 150, target);
    if (current_angle == 4096)
        w_u16(object_offset + 148, target);
    else
    {
        delta = (sint16)(target - (uint32)current_angle);
        if (delta < -2048)
            delta += 4096;
        else if (delta > 2048)
            delta -= 4096;
        delta = (sint16)delta;
        if (delta < yaw_step && -yaw_step < delta)
            w_u16(object_offset + 148, target);
        else if (delta < 0)
            w_u16(object_offset + 148, r_u16(object_offset + 148) - (uint32)yaw_step);
        else
            w_u16(object_offset + 148, r_u16(object_offset + 148) + (uint32)yaw_step);
        w_u16(object_offset + 148, r_u16(object_offset + 148) & 0xFFF);
    }
    w_u16(object_offset + 88, (r_u16(object_offset + 148) + 2048) & 0xFFF);
pitch:
    if (r_s16(object_offset + 154) == 4096)
        w_u16(object_offset + 86, 0);
    else
    {
        x = (uint32)(r_s32(vector_offset) >> 6);
        y = (uint32)(r_s32(vector_offset + 4) >> 6);
        z = (uint32)(r_s32(vector_offset + 8) >> 6);
        squared = x * x + y * y + z * z;
        if ((sint32)squared >= 0)
            length = (sint32)p2_sdk_8007e794(squared);
        else
            length = (sint32)(p2_sdk_8007e794(squared >> 2) << 1);
        vertical = r_s32(vector_offset + 4);
        if (vertical < 0)
            vertical = (sint32)(0 - (uint32)vertical);
        if (length < vertical)
            length = vertical;
        value = (uint32)AsinAngle(r_s32(vector_offset + 4), length);
        target = (sint16)value == 4096 ? 0 : (0 - value) & 0xFFF;
        current_angle = r_s16(object_offset + 152);
        if ((sint16)target != current_angle)
        {
            pitch_step = ((sint32)step) >> 1;
            target &= 0xFFF;
            if (current_angle == 4096)
                w_u16(object_offset + 152, target);
            else
            {
                delta = (sint16)(target - (uint32)current_angle);
                if (delta < -2048)
                    delta += 4096;
                else if (delta > 2048)
                    delta -= 4096;
                delta = (sint16)delta;
                if (delta < pitch_step && -pitch_step < delta)
                    w_u16(object_offset + 152, target);
                else if (delta < 0)
                    w_u16(object_offset + 152, r_u16(object_offset + 152) - (uint32)pitch_step);
                else
                    w_u16(object_offset + 152, r_u16(object_offset + 152) + (uint32)pitch_step);
                w_u16(object_offset + 152, r_u16(object_offset + 152) & 0xFFF);
            }
            w_u16(object_offset + 86, r_u16(object_offset + 152) & 0xFFF);
        }
    }
    w_u16(object_offset + 84, 0x7FFF);
finish:
    value = r_u32(object_offset + 28);
    current_angle = r_s16(object_offset + 148);
    w_u32(object_offset + 28, value | 1);
    result = current_angle == old_yaw && r_s16(object_offset + 152) == old_pitch;
    p2_guest_stack_pointer = saved_stack;
    return result;
}

uint32 p2_func_80053704(uint32 object_offset)
{
    CharacterModelRecord *model_record;
    FUNCTION_MARKER(0x80053704, "SLUS_005.78");
    uint32 saved_stack = p2_guest_stack_pointer;
    uint32 parent = r_u32(object_offset + 116);
    uint32 vector, model, flags, path, definition, x, y, z, squared, animation, result;
    sint32 distance, speed, cap, target_frame, frame;
    p2_guest_stack_pointer -= 0x20;
    vector = p2_guest_stack_pointer + 0x10;
    flags = r_u32(object_offset + 28);
    if ((flags & 0x10) != (r_u32(parent + 28) & 0x10))
    {
        path = r_u32(object_offset + 112);
        w_u32(object_offset + 28, flags ^ 0x10);
        if ((r_u16(path + 14) & 8) != 0)
            path = r_u32(path + 16);
        definition = r_u32(object_offset + 132);
        w_u32(object_offset + 112, path);
        frame = r_u16(object_offset + 136);
        w_u16(object_offset + 136, r_u16(definition + 26) - (uint32)frame);
    }
    if (r_s8(object_offset + 124) == 1)
    {
        w_u8(object_offset + 125, 1);
        w_u32(0x8008A340, object_offset);
        w_u8(parent + 125, 33);
        w_u32(parent + 116, object_offset);
        model = character_controller.model_slots[(((character_controller.motion.form_flags >> 18) & 0x3C)) / 4];
        model_record = (CharacterModelRecord *)psx_addr(model, 240);
        character_controller.model_offset =  model;
        w_u32(object_offset + 100, 0);
        w_u32(object_offset + 96, 0);
        w_u32(object_offset + 92, 0);
        w_u32(parent + 100, 0);
        w_u32(parent + 96, 0);
        w_u32(parent + 92, 0);
        character_controller.motion.state_code = 0;
        goto update_animation;
    }
    model = character_controller.model_slots[(1 - ((uint16)(character_controller.motion.form_flags >> 16) & 0xF))];
    model_record = (CharacterModelRecord *)psx_addr(model, 240);
    w_u32(vector, r_u32(parent + 16) - r_u32(object_offset + 16));
    w_u32(vector + 4, r_u32(parent + 20) - r_u32(object_offset + 20));
    w_u32(vector + 8, r_u32(parent + 24) - r_u32(object_offset + 24));
    if (r_u32(parent + 92) == 0 && r_u32(parent + 96) == 0 && r_u32(parent + 100) == 0 && r_u32(object_offset + 92) == 0 && r_u32(object_offset + 96) == 0 && r_u32(object_offset + 100) == 0)
        goto orient;
    x = (uint32)(r_s32(vector) >> 6);
    y = (uint32)(r_s32(vector + 4) >> 6);
    z = (uint32)(r_s32(vector + 8) >> 6);
    squared = x * x + y * y + z * z;
    if ((sint32)squared >= 0)
        distance = (sint32)p2_sdk_8007e794(squared);
    else
        distance = (sint32)(p2_sdk_8007e794(squared >> 2) << 1);
    w_u32(vector + 4, (uint32)distance);
    if (distance <= 49151 && (distance <= 36863 || (r_u32(object_offset + 92) == 0 && r_u32(object_offset + 96) == 0 && r_u32(object_offset + 100) == 0)))
        goto stop;
    target_frame = r_s16(parent + 136);
    if (r_u32(object_offset + 112) != r_u32(r_u32(0x8008A114) + 136))
    {
        if (target_frame < (r_s16(r_u32(parent + 132) + 26) >> 1))
            target_frame = r_s16(object_offset + 136) + 1;
        else
            target_frame = r_s16(object_offset + 136) - 1;
    }
    if (r_s16(object_offset + 136) >= target_frame)
        goto stop;
    x = (uint32)(r_s32(parent + 92) >> 6);
    y = (uint32)(r_s32(parent + 96) >> 6);
    z = (uint32)(r_s32(parent + 100) >> 6);
    squared = x * x + y * y + z * z;
    if ((sint32)squared >= 0)
        speed = (sint32)p2_sdk_8007e794(squared);
    else
        speed = (sint32)(p2_sdk_8007e794(squared >> 2) << 1);
    w_u32(vector + 8, (uint32)speed);
    w_u32(vector, (uint32)speed);
    if (r_s32(vector + 4) > 49152)
    {
        cap = model_record->movement.speed_limits[1];
        w_u32(vector, (uint32)cap);
        if (cap < speed)
            w_u32(vector, (uint32)speed + (uint32)(speed >> 3));
    }
    flags = r_u32(object_offset + 28);
    w_u8(object_offset + 126, 1);
    w_u32(object_offset + 28, flags | 0x40);
    p2_func_8003db7c(object_offset, r_s32(vector), 0, (const VECTOR *)psx_addr(0, 12));
    flags = r_u32(object_offset + 28);
    w_u8(object_offset + 126, 0);
    w_u32(object_offset + 28, flags & 0xFFFFFFBF);
    x = r_u32(object_offset + 92);
    y = r_u32(object_offset + 96);
    z = r_u32(object_offset + 100);
    w_u32(vector, x);
    w_u32(vector + 4, y);
    w_u32(vector + 8, z);
    goto orient;
stop:
    w_u32(object_offset + 100, 0);
    w_u32(object_offset + 96, 0);
    w_u32(object_offset + 92, 0);
orient:
    p2_func_80040384(object_offset, vector, (sint16)character_controller.turn_step);
update_animation:
    if (r_u32(object_offset + 92) != 0 || r_u32(object_offset + 96) != 0 || r_u32(object_offset + 100) != 0)
    {
        x = r_u32(object_offset + 16) + r_u32(object_offset + 92);
        y = r_u32(object_offset + 20) + r_u32(object_offset + 96);
        z = r_u32(object_offset + 24) + r_u32(object_offset + 100);
        w_u32(object_offset + 16, x);
        w_u32(object_offset + 20, y);
        w_u32(object_offset + 24, z);
        animation = r_u32(object_offset + 104);
        definition = model_record->animation_offsets[2];
    }
    else
    {
        animation = r_u32(object_offset + 104);
        definition = model_record->animation_offsets[0];
    }
    if (animation != definition)
    {
        flags = r_u32(object_offset + 28);
        w_u32(object_offset + 104, definition);
        w_u16(object_offset + 130, 0xFFFF);
        w_u32(object_offset + 28, flags | 0x10000);
    }
    animation = r_u32(object_offset + 104);
    if (animation != 0)
    {
        frame = r_s16(object_offset + 130);
        if (frame < r_s16(animation + 6) - 1)
            frame += 1;
        else
            frame = 0;
    }
    else
        frame = 0;
    w_u16(object_offset + 130, (uint32)frame);
    result = p2_func_8001a6ac(object_offset);
    p2_guest_stack_pointer = saved_stack;
    return result;
}

uint32 p2_func_800447f0(void)
{
    FUNCTION_MARKER(0x800447F0, "SLUS_005.78");
    if (((uint16)(character_controller.motion.form_flags >> 16) & 0xF) == 1 && (player_interaction.flags != 0 || (character_controller.secondary_flags & 0x20) != 0))
        return 1;
    return 0;
}

uint32 p2_func_80060fdc(uint32 input)
{
    FUNCTION_MARKER(0x80060FDC, "SLUS_005.78");
    uint32 flags, consumed = 0, context, value;
    if ((input & 0xF) == 0)
    {
        w_u32(0x8008A360, 0);
        return input;
    }
    flags = game_flow.mode_flags;
    if (flags == 0)
    {
        w_u32(0x8008A360, 0);
        return input;
    }
    if ((input & 5) == 5)
    {
        if ((flags & 0x80) != 0 && (input & 0xE000) != 0)
        {
            consumed = 1;
            if ((input & 0x8000) != 0)
            {
                context = r_u32(0x8008A114);
                w_u16(context + 44, r_u16(context + 44) - 8);
            }
            else if ((input & 0x2000) != 0)
            {
                context = r_u32(0x8008A114);
                w_u16(context + 44, r_u16(context + 44) + 8);
            }
            else if ((input & 0x4000) != 0)
                w_u16(r_u32(0x8008A114) + 44, 0);
            context = r_u32(0x8008A114);
            w_u16(context + 44, r_u16(context + 44) & 0xFFF);
        }
    }
    else if ((input & 0x21) == 0x21)
    {
        if ((r_u32(0x8008A360) & 0x21) != 0x21 && (flags & 0x40) != 0)
        {
            value = r_u8(0x8008A45C) + 1;
            w_u8(0x8008A45C, value);
            consumed = 1;
            if ((value & 0xFF) == 31)
                w_u8(0x8008A45C, 1);
        }
    }
    else if ((input & 0x41) == 0x41 && (r_u32(0x8008A360) & 0x41) != 0x41 && (flags & 0x40) != 0)
    {
        consumed = 1;
        w_u8(0x8008A45C, 0);
    }
    w_u32(0x8008A360, input);
    return consumed != 0 ? 0 : input;
}

