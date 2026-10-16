#include "game.h"

uint32 p2_func_8001b334(uint32 object_offset, sint32 step)
{
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

uint32 p2_func_800229fc(uint32 scene_offset, sint32 index)
{
    uint32 node = r_u32(scene_offset + 40);
    while (node != 0)
    {
        if ((r_u16(node + 14) >> 6) == (uint32)index)
            return node;
        node = r_u32(node);
    }
    return 0;
}

uint32 p2_func_80055dc0(uint32 object_slot_offset, uint32 character_offset, uint32 kind)
{
    uint32 model = r_u32(0x8008B4B8 + kind * 4);
    uint32 flags, current_kind, value, source, destination, table, index = 0;
    uint32 x, y, z;
    w_u32(0x800B7A64, kind == 5 ? 0x8002AD4C : 0x8004DCD0);
    value = (r_u32(character_offset + 704) & 0xFFF0FFFF) | (kind << 16);
    w_u32(character_offset + 704, (value & 0xF0FFFFFF) | 0x0F000000);
    flags = r_u32(character_offset);
    current_kind = r_u16(character_offset + 706) & 0xF;
    w_u32(character_offset, flags & 0xBFFFFFFF);
    switch (current_kind)
    {
        case 0:
            if ((r_u32(character_offset + 4) & 0x20) != 0)
            {
                value = r_u32(0x8008A178);
                flags = r_u32(character_offset + 4);
                w_u32(0x8008A178, value & 0xFFFFFFDB);
                w_u32(character_offset + 4, flags & 0xFFFFFFDF);
                if (r_u32(object_slot_offset) != 0)
                {
                    p2_func_80027b1c(r_u32(object_slot_offset), character_offset, 6);
                    source = r_u32(object_slot_offset);
                }
                else
                {
                    p2_func_80027b1c(r_u32(model + 232), character_offset, 6);
                    source = r_u32(model + 232);
                }
                p2_func_8005a440(source, character_offset);
            }
            break;
        case 1:
            if ((r_u32(character_offset + 4) & 0x100) != 0)
            {
                p2_func_80027ac0(character_offset, 6);
                p2_func_8005a668(character_offset);
                w_u32(character_offset + 4, r_u32(character_offset + 4) | 0x20);
            }
            goto clear_action;
        case 4:
        case 5:
        case 6:
        case 7:
        case 8:
        case 9:
        case 10:
        case 11:
        case 13:
        case 14:
            if ((r_u32(character_offset + 4) & 0x20) != 0)
            {
                value = r_u32(0x8008A178);
                flags = r_u32(character_offset + 4);
                w_u32(0x8008A178, value & 0xFFFFFFDB);
                w_u32(character_offset + 4, flags & 0xFFFFFFDF);
            }
            if ((r_u32(character_offset + 4) & 0x100) != 0)
            {
                p2_func_80027ac0(character_offset, 6);
                p2_func_8005a668(character_offset);
            }
        clear_action:
            value = r_u32(character_offset + 20);
            if ((value & 0xFFFF) != 0)
                p2_func_80027ac0(character_offset, (value >> 16) != 0 ? (value >> 16) - 7 : 3);
            w_u32(character_offset + 20, 0);
            break;
        default:
            break;
    }
    source = r_u32(object_slot_offset);
    if (source == 0)
    {
        w_u16(character_offset + 62, 900);
        goto assign_model;
    }
    table = r_u32(character_offset + 780);
    if (table != 0)
    {
        value = r_u32(source + 104);
        while (value != r_u32(table + 80))
        {
            ++index;
            table += 4;
            if (index >= 38)
                break;
        }
        if (index >= 7)
            index = (r_u32(source + 28) & 0x400) != 0 ? 6 : 2;
        if (index == 38)
        {
            index = 0;
            w_u16(character_offset + 62, 900);
        }
    }
    else
        w_u16(character_offset + 62, 900);
    source = r_u32(object_slot_offset);
    destination = r_u32(model + 232);
    w_u8(destination + 13, r_u8(source + 13));
    x = r_u32(source + 16);
    y = r_u32(source + 20);
    z = r_u32(source + 24);
    w_u32(destination + 16, x);
    w_u32(destination + 20, y);
    w_u32(destination + 24, z);
    w_u32(destination + 28, r_u32(source + 28));
    w_u32(destination + 228, r_u32(source + 228));
    x = r_u32(source + 60);
    y = r_u32(source + 64);
    z = r_u32(source + 68);
    w_u32(destination + 60, x);
    w_u32(destination + 64, y);
    w_u32(destination + 68, z);
    x = r_u32(source + 72);
    y = r_u32(source + 76);
    z = r_u32(source + 80);
    w_u32(destination + 72, x);
    w_u32(destination + 76, y);
    w_u32(destination + 80, z);
    x = r_u32(source + 84);
    y = r_u32(source + 88);
    w_u32(destination + 84, x);
    w_u32(destination + 88, y);
    x = r_u32(source + 92);
    y = r_u32(source + 96);
    z = r_u32(source + 100);
    w_u32(destination + 92, x);
    w_u32(destination + 96, y);
    w_u32(destination + 100, z);
    p2_func_8006637c((uint8 *)psx_addr(destination + 108, 12), (const uint8 *)psx_addr(source + 108, 12), 12);
    w_u8(destination + 124, r_u8(source + 124));
    w_u8(destination + 125, r_u8(source + 125));
    w_u8(destination + 126, r_u8(source + 126));
    w_u8(destination + 127, r_u8(source + 127));
    w_u16(destination + 128, r_u16(source + 128));
    w_u16(destination + 130, r_u16(source + 130));
    w_u32(destination + 132, r_u32(source + 132));
    w_u16(destination + 136, r_u16(source + 136));
    w_u16(destination + 148, r_u16(source + 148));
    w_u16(destination + 150, r_u16(source + 150));
    w_u16(destination + 152, r_u16(source + 152));
    w_u16(destination + 154, r_u16(source + 154));
    w_u32(destination + 144, r_u32(source + 144));
    p2_func_8006637c((uint8 *)psx_addr(destination + 160, 64), (const uint8 *)psx_addr(source + 160, 64), 64);
assign_model:
    table = r_u32(0x8008B4B8 + kind * 4);
    w_u32(character_offset + 780, table);
    destination = r_u32(model + 232);
    w_u32(destination + 104, r_u32(table + index * 4 + 80));
    destination = r_u32(model + 232);
    w_u32(destination + 28, r_u32(destination + 28) | 0x20000);
    destination = r_u32(model + 232);
    w_u32(destination + 28, r_u32(destination + 28) & 0xFBFEFFFF);
    w_u32(object_slot_offset, r_u32(model + 232));
    w_u32(0x8008A340, r_u32(model + 232));
    p2_func_8006637c((uint8 *)psx_addr(character_offset + 716, 60), (const uint8 *)psx_addr(r_u32(character_offset + 780), 60), 60);
    value = r_u32(character_offset + 772);
    if ((sint32)value > 0)
        w_u16(character_offset + 776, 2048 / (sint32)value);
    else
        w_u16(character_offset + 776, 4096);
    return index;
}

P2PathProbeResult p2_select_scene_path_with_selection(uint32 path_offset, uint32 fallback_frame)
{
    P2PathProbeResult output;
    FUNCTION_MARKER(0x80024844, "SLUS_005.78");
    uint32 context = r_u32(0x8008A114);
    output.fallback_frame = fallback_frame;
    if (r_s16(context + 304) != 4096)
        output.fallback_frame = p2_setup_camera_projection_with_selection(0xE9555).fallback_frame;
    context = r_u32(0x8008A114);
    w_u32(context + 136, path_offset);
    output.value = (sint32)context;
    return output;
}

uint32 p2_func_80024844(uint32 path_offset)
{
    return (uint32)p2_select_scene_path_with_selection(path_offset, 0).value;
}

uint32 p2_func_80048af0(uint32 object_offset, uint32 character_offset, sint32 mode)
{
    uint32 model, flags, value, packed, category, result = 1;
    sint32 selected;
    p2_func_8006540c();
    category = r_u16(character_offset + 706) & 0xF;
    w_u32(character_offset + 500, 0);
    w_u32(0x800B7A64, category == 5 ? 0x8002AD4C : 0x8004DCD0);
    w_u32(0x800BA384, 0x800530A0);
    model = r_u32(character_offset + 780);
    w_u8(character_offset + 714, 0);
    w_u16(r_u32(r_u32(model + 232) + 156) + 40, (uint32)(r_s32(model + 60) >> 5));
    model = r_u32(character_offset + 780);
    w_u16(r_u32(r_u32(model + 232) + 156) + 42, (uint32)(r_s32(model + 64) >> 5));
    value = r_u32(0x1F800320);
    flags = r_u32(character_offset);
    w_u32(0x1F800320, value & 0xFFFFFEFF);
    w_u32(character_offset, flags & 0xBFFFFFFF);
    p2_func_8001c684(character_offset);
    w_u32(character_offset, r_u32(character_offset) & 0xC0000000);
    flags = r_u32(object_offset + 28);
    value = r_u32(object_offset + 228);
    w_u32(object_offset + 28, flags & 0xC0F793FF);
    w_u32(object_offset + 228, value & 0xFC000000);
    flags = r_u32(character_offset + 4);
    value = r_u32(0x8008A52C);
    w_u32(character_offset + 32, 0);
    w_u32(character_offset + 28, 0);
    w_u32(character_offset + 24, 0);
    w_u32(character_offset + 4, flags & 0xFFFFF932);
    w_u32(object_offset + 100, 0);
    w_u32(object_offset + 96, 0);
    w_u32(object_offset + 92, 0);
    w_u16(character_offset + 60, 0);
    w_u16(character_offset + 56, 0);
    w_u16(character_offset + 54, 0);
    w_u16(character_offset + 52, 0);
    w_u16(character_offset + 50, 0);
    w_u16(character_offset + 48, 0);
    w_u32(character_offset + 784, 0);
    w_u16(character_offset + 676, 0);
    w_u32(character_offset + 552, 0);
    if ((value & 0x10) == 0 && mode != 0)
        w_u32(character_offset + 20, 0);
    if (mode == 2)
    {
        w_u8(character_offset + 12, 3);
        w_u8(character_offset + 13, 4);
        w_u8(character_offset + 14, 4);
        w_u32(character_offset + 16, 0);
        if ((r_u32(character_offset + 4) & 0x80) == 0)
            w_u8(character_offset + 712, 0);
        result = r_u32(0x1F800320) | 0x20000;
        w_u32(0x1F800320, result);
    }
    else if (mode == 1)
    {
        value = p2_func_800255f8(r_u32(0x8008A4E8));
        w_u8(character_offset + 14, value);
        selected = (sint8)value;
        if (selected == -1)
        {
            packed = r_u32(character_offset + 704) & 0xFFF0FFFF;
            category = ((packed >> 20) & 0xF) << 16;
        }
        else
        {
            value = p2_func_800473f0(selected, character_offset);
            w_u8(character_offset + 14, value);
            packed = r_u32(character_offset + 704);
            selected = r_s8(character_offset + 14);
            packed &= 0xFFF0FFFF;
            category = (uint32)selected << 16;
            w_u32(character_offset + 704, packed);
        }
        w_u32(character_offset + 704, packed | category);
        w_u32(object_offset + 28, r_u32(object_offset + 28) | 0x10);
        w_u8(character_offset + 14, r_u8(character_offset + 13));
        if ((r_u32(character_offset + 4) & 0x80) == 0)
            w_u8(character_offset + 712, 0);
        p2_func_80025674(0, 0, r_u32(0x8008A4E8), -2, 0);
        result = 15;
        w_u16(r_u32(0x8008A114) + 46, result);
    }
    return result;
}

uint32 p2_func_8001c684(uint32 character_offset)
{
    uint32 result = r_u32(character_offset) & 0xFFFE7FBF;
    w_u32(character_offset, result);
    return result;
}

uint32 p2_func_80022c24(uint32 path_offset, sint32 frame, uint32 destination_offset, uint32 direction)
{
    return (uint32)p2_interpolate_aux_path_with_selection(path_offset, frame, destination_offset, direction, 0).value;
}

P2PathProbeResult p2_interpolate_aux_path_with_selection(uint32 path_offset, sint32 frame, uint32 destination_offset, uint32 direction, uint32 fallback_frame)
{
    P2PathProbeResult output;
    uint32 saved_stack = p2_guest_stack_pointer;
    uint32 matrix, vector, i, result;
    p2_guest_stack_pointer -= 0x20;
    vector = p2_guest_stack_pointer + 0x10;
    if ((r_u16(path_offset + 14) & (direction | 8)) == 0)
        frame = (sint32)((uint32)r_s16(r_u32(path_offset + 36) + 26) - (uint32)frame - 1);
    if (r_u32(path_offset + 32) != 0)
    {
        p2_func_80017364(frame, path_offset + 40, vector);
        PushMatrix();
        matrix = r_u32(path_offset + 32);
        for (i = 0; i < 5; ++i)
            xport_gte_write_control(i, r_u32(matrix + i * 4));
        p2_sdk_8007ec64(vector, destination_offset);
        PopMatrix();
        /* TODO: Audit PopMatrix diagnostic exits for the secondary output */
        fallback_frame = xport_gte_read_translation(0);
        for (i = 0; i < 3; ++i)
        {
            matrix = r_u32(path_offset + 32);
            result = r_u32(destination_offset + i * 4) + r_u32(matrix + 20 + i * 4);
            w_u32(destination_offset + i * 4, result);
        }
    }
    else
    {
        output = p2_sample_track_with_selection(frame, path_offset + 40, destination_offset, fallback_frame);
        result = (uint32)output.value;
        fallback_frame = output.fallback_frame;
    }
    p2_guest_stack_pointer = saved_stack;
    output.value = (sint32)result;
    output.fallback_frame = fallback_frame;
    return output;
}

uint32 p2_func_80022b20(uint32 path_offset, sint32 frame, uint32 destination_offset, uint32 direction)
{
    return (uint32)p2_interpolate_path_with_selection(path_offset, frame, destination_offset, direction, 0).value;
}

P2PathProbeResult p2_interpolate_path_with_selection(uint32 path_offset, sint32 frame, uint32 destination_offset, uint32 direction, uint32 fallback_frame)
{
    P2PathProbeResult output;
    uint32 saved_stack = p2_guest_stack_pointer;
    uint32 definition, matrix, vector, i, result;
    p2_guest_stack_pointer -= 0x20;
    vector = p2_guest_stack_pointer + 0x10;
    if (direction == 0)
        frame = (sint32)((uint32)r_s16(r_u32(path_offset + 36) + 26) - (uint32)frame - 1);
    definition = r_u32(path_offset + 36);
    matrix = r_u32(definition + 40);
    if (matrix != 0)
    {
        p2_func_80017364(frame, definition + 20, vector);
        PushMatrix();
        for (i = 0; i < 5; ++i)
            xport_gte_write_control(i, r_u32(matrix + i * 4));
        p2_sdk_8007ec64(vector, destination_offset);
        PopMatrix();
        /* TODO: Audit PopMatrix diagnostic exits for the secondary output */
        fallback_frame = xport_gte_read_translation(0);
        for (i = 0; i < 3; ++i)
        {
            result = r_u32(destination_offset + i * 4) + r_u32(matrix + 20 + i * 4);
            w_u32(destination_offset + i * 4, result);
        }
    }
    else
    {
        output = p2_sample_track_with_selection(frame, definition + 20, destination_offset, fallback_frame);
        result = (uint32)output.value;
        fallback_frame = output.fallback_frame;
    }
    p2_guest_stack_pointer = saved_stack;
    output.value = (sint32)result;
    output.fallback_frame = fallback_frame;
    return output;
}

uint32 p2_func_8003db7c(uint32 object_offset, sint32 distance, sint32 height, uint32 direction_offset)
{
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
        p2_func_8003af18(object_offset, r_u32(r_u32(0x8008A114) + 136), working, current);
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
    p2_func_8003af18(object_offset, r_u32(r_u32(0x8008A114) + 136), working, current);
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
        result = (uint32)p2_func_8003d3fc(r_u32(object_offset + 132), r_s16(object_offset + 136), working, current, direction, 4096);
        if ((sint32)result >= 0)
            break;
        frame = r_s16(object_offset + 136);
        if (frame != r_s16(r_u32(object_offset + 132) + 26) - 1)
        {
            direction_offset = working;
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
    if (drop < -1365 && (r_u32(object_offset + 228) & 0x20) == 0 && (special_path & special) == 0 && (character == 0 || (r_u16(character + 706) & 0xF) != 5) && (r_u32(r_u32(object_offset + 132) + 16) & 0x11) == 0)
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
        p2_func_800230f0(matrix, (sint16)frame, next, track_direction);
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
    if ((p2_func_8003af18(object_offset, path, working, current) & 0xFF) == 0)
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
    value = p2_func_8005dc00(r_s32(0x1F800240), r_s32(0x1F800238));
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
        p2_func_80022ec4(r_u32(object_offset + 132), 0x1F800238, (sint32)direction);
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
        if (drop < -169 && (base_height <= 0 || (first_segment == 0 && (uint32)drop + 5461 >= 0x1001)) && (character == 0 || (r_u16(character + 706) & 0xF) != 5) && (r_u32(r_u32(object_offset + 132) + 16) & 0x11) == 0 && ((r_u32(r_u32(r_u32(r_u32(0x8008A114) + 136) + 36) + 16) & 0x80000) == 0 || (r_u32(object_offset + 228) & 0x20) == 0) && (special_path & special) == 0)
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
        w_u32(character + 24, 0);
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
            result = (uint32)p2_func_8003d3fc(r_u32(object_offset + 132), 0, working, current, direction, 4096);
            w_u16(object_offset + 136, result);
        }
    }
    goto done;
direct_vector:
    if ((r_u32(object_offset + 28) & 0x40) == 0)
        distance = (sint32)(0 - (uint32)distance);
    for (i = 0; i < 3; ++i)
        w_u32(object_offset + 92 + i * 4, (uint32)(((sint64)r_s32(direction_offset + i * 4) * distance) >> 12));
    result = r_u32(object_offset + 96) + (uint32)height;
    w_u32(object_offset + 96, result);
done:
    p2_guest_stack_pointer = saved_stack;
    return result;
}

P2PathProbeResult p2_sample_world_track_with_selection(uint32 definition_offset, sint32 frame, uint32 destination_offset, uint32 direction, uint32 fallback_frame)
{
    P2PathProbeResult output;
    uint32 saved_stack = p2_guest_stack_pointer;
    uint32 matrix, vector, i, result;
    p2_guest_stack_pointer -= 0x20;
    vector = p2_guest_stack_pointer + 0x10;
    if (direction == 0)
        frame = (sint32)((uint32)r_s16(definition_offset + 26) - (uint32)frame - 1);
    matrix = r_u32(definition_offset + 40);
    if (matrix != 0)
    {
        output = p2_sample_track_with_selection(frame, definition_offset + 20, vector, fallback_frame);
        PushMatrix();
        for (i = 0; i < 5; ++i)
            xport_gte_write_control(i, r_u32(matrix + i * 4));
        p2_sdk_8007ec64(vector, destination_offset);
        PopMatrix();
        /* TODO: Bind the original matrix-stack diagnostic exits */
        output.fallback_frame = xport_gte_read_translation(0);
        for (i = 0; i < 3; ++i)
        {
            result = r_u32(destination_offset + i * 4) + r_u32(matrix + 20 + i * 4);
            w_u32(destination_offset + i * 4, result);
        }
    }
    else
    {
        output = p2_sample_track_with_selection(frame, definition_offset + 20, destination_offset, fallback_frame);
        result = (uint32)output.value;
    }
    p2_guest_stack_pointer = saved_stack;
    output.value = (sint32)result;
    return output;
}

uint32 p2_func_800230f0(uint32 definition_offset, sint32 frame, uint32 destination_offset, uint32 direction)
{
    return (uint32)p2_sample_world_track_with_selection(definition_offset, frame, destination_offset, direction, 0).value;
}

uint32 p2_func_800451cc(uint32 path_offset, sint32 frame, uint32 destination_offset, uint32 direction)
{
    return (uint32)p2_project_path_position_with_selection(path_offset, frame, destination_offset, direction).value;
}

P2PathProbeResult p2_project_path_position_with_selection(uint32 path_offset, sint32 frame, uint32 destination_offset, uint32 direction)
{
    P2PathProbeResult output;
    uint32 fallback_frame;
    uint32 saved_stack = p2_guest_stack_pointer;
    uint32 point, previous, following, x, y, z, numerator, denominator, ratio, scale, result, i;
    sint32 selected, count, gap, reciprocal, lift;
    p2_guest_stack_pointer -= 0x48;
    point = p2_guest_stack_pointer + 0x18;
    previous = p2_guest_stack_pointer + 0x28;
    following = p2_guest_stack_pointer + 0x38;
    output = p2_project_path_with_selection(r_u32(path_offset + 36), frame, r_u32(0x8008A114) + 16, point, direction, 4096);
    fallback_frame = output.fallback_frame;
    selected = output.value;
    if (selected < 0)
        selected = frame;
    gap = (sint32)(r_u32(r_u32(0x8008A114) + 20) - r_u32(point + 4));
    if (selected <= 0)
    {
        output = p2_interpolate_path_with_selection(path_offset, 0, point, direction, fallback_frame);
        fallback_frame = output.fallback_frame;
        output = p2_interpolate_aux_path_with_selection(path_offset, 0, destination_offset, direction, fallback_frame);
        fallback_frame = output.fallback_frame;
    }
    else
    {
        count = r_s16(r_u32(path_offset + 36) + 26);
        if (selected < count)
        {
            output = p2_interpolate_path_with_selection(path_offset, (sint32)((uint32)selected - 1), previous, direction, fallback_frame);
            fallback_frame = output.fallback_frame;
            x = r_u32(point) - r_u32(previous);
            y = r_u32(point + 4) - r_u32(previous + 4);
            z = r_u32(point + 8) - r_u32(previous + 8);
            w_u32(point, x);
            w_u32(point + 4, y);
            w_u32(point + 8, z);
            x = (uint32)((sint32)x >> 6);
            y = (uint32)((sint32)y >> 6);
            z = (uint32)((sint32)z >> 6);
            fallback_frame = x * x;
            numerator = x * x + y * y + z * z;
            output = p2_interpolate_aux_path_with_selection(path_offset, (sint32)((uint32)selected - 1), destination_offset, direction, fallback_frame);
            fallback_frame = output.fallback_frame;
            output = p2_interpolate_path_with_selection(path_offset, selected, following, direction, fallback_frame);
            fallback_frame = output.fallback_frame;
            x = r_u32(following) - r_u32(previous);
            y = r_u32(following + 4) - r_u32(previous + 4);
            z = r_u32(following + 8) - r_u32(previous + 8);
            w_u32(following, x);
            w_u32(following + 4, y);
            w_u32(following + 8, z);
            x = (uint32)((sint32)x >> 6);
            y = (uint32)((sint32)y >> 6);
            z = (uint32)((sint32)z >> 6);
            denominator = x * x + y * y + z * z;
            if (denominator == 0)
            {
                output = p2_interpolate_aux_path_with_selection(path_offset, selected, destination_offset, direction, fallback_frame);
                fallback_frame = output.fallback_frame;
            }
            else
            {
                reciprocal = 0x40000000 / (sint32)denominator;
                ratio = (uint32)(((sint64)(sint32)numerator * reciprocal) >> 18);
                w_u32(0x1F80030C, ratio);
                output = p2_interpolate_aux_path_with_selection(path_offset, selected, point, direction, fallback_frame);
                fallback_frame = output.fallback_frame;
                for (i = 0; i < 3; ++i)
                    w_u32(point + i * 4, r_u32(point + i * 4) - r_u32(destination_offset + i * 4));
                ratio = r_u32(0x1F80030C);
                if ((sint32)ratio >= 0)
                {
                    output = p2_sqrt_with_selection(ratio, fallback_frame);
                    scale = (uint32)output.value;
                }
                else
                {
                    output = p2_sqrt_with_selection(ratio >> 2, fallback_frame);
                    scale = (uint32)output.value << 1;
                }
                fallback_frame = output.fallback_frame;
                for (i = 0; i < 3; ++i)
                    w_u32(point + i * 4, (uint32)(((sint64)r_s32(point + i * 4) * (sint32)scale) >> 12));
                for (i = 0; i < 3; ++i)
                    w_u32(destination_offset + i * 4, r_u32(destination_offset + i * 4) + r_u32(point + i * 4));
            }
        }
        else
        {
            output = p2_interpolate_path_with_selection(path_offset, count - 1, point, direction, fallback_frame);
            fallback_frame = output.fallback_frame;
            output = p2_interpolate_aux_path_with_selection(path_offset, r_s16(r_u32(path_offset + 36) + 26) - 1, destination_offset, direction, fallback_frame);
            fallback_frame = output.fallback_frame;
        }
    }
    result = r_u16(path_offset + 14) & 1;
    if (result != 0)
    {
        result = 5120;
        if (gap > 0)
        {
            lift = (sint32)(uint32)(((sint64)gap * 5120) >> 12);
            if ((sint32)((uint32)lift - (uint32)gap) > 61440)
                lift = (sint32)((uint32)gap + 61440);
            result = r_u32(destination_offset + 4) + (uint32)lift;
            w_u32(destination_offset + 4, result);
        }
    }
    p2_guest_stack_pointer = saved_stack;
    output.value = (sint32)result;
    output.fallback_frame = fallback_frame;
    return output;
}

sint32 p2_func_8003d3fc(uint32 definition_offset, sint32 frame, uint32 position_offset, uint32 floor_offset, uint32 direction, sint32 scale)
{
    return p2_project_path_with_selection(definition_offset, frame, position_offset, floor_offset, direction, scale).value;
}

P2PathProbeResult p2_project_path_with_selection(uint32 definition_offset, sint32 frame, uint32 position_offset, uint32 floor_offset, uint32 direction, sint32 scale)
{
    uint32 saved_stack = p2_guest_stack_pointer;
    P2PathProbeResult output;
    uint32 fallback_frame;
    uint32 vector, side, i, x, y, z, floor_x, floor_z, point_x, point_z, input_x, input_z;
    uint32 dx, dz, point_dot, floor_dot, ratio, squared, limit, last_endpoint = 0;
    sint32 reciprocal, factor, result;
    p2_guest_stack_pointer -= 0x20;
    vector = p2_guest_stack_pointer + 0x10;
    if (frame > 0)
    {
        sint32 count = r_s16(definition_offset + 26);
        if (frame >= count)
            frame = count - 1;
    }
    else
        frame = 0;
    p2_func_800230f0(definition_offset, frame, floor_offset, direction);
    for (side = 0; side < 2; ++side)
    {
        if (side == 0 && frame == 0)
            continue;
        p2_func_800230f0(definition_offset, (sint32)((uint32)frame + (side == 0 ? 0xFFFFFFFF : 1)), vector, direction);
        floor_z = r_u32(floor_offset + 8);
        point_z = r_u32(vector + 8);
        point_x = r_u32(vector);
        floor_x = r_u32(floor_offset);
        input_x = r_u32(position_offset);
        dz = floor_z - point_z;
        dx = point_x - floor_x;
        x = point_x - input_x;
        point_dot = (uint32)(((sint64)(sint32)x * (sint32)dx) >> 12);
        input_z = r_u32(position_offset + 8);
        z = point_z - input_z;
        fallback_frame = (uint32)((uint64)((sint64)(sint32)z * (sint32)dz) >> 32) << 20;
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
            if (frame != r_s16(definition_offset + 26) - 1)
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
    if (((uint32)scale << 1) < r_u32(floor_offset) - r_u32(position_offset) + (uint32)scale && ((uint32)scale << 1) < r_u32(floor_offset + 8) - r_u32(position_offset + 8) + (uint32)scale)
    {
        result = -1;
        goto done;
    }
    if (point_dot == floor_dot)
    {
        for (i = 0; i < 3; ++i)
            w_u32(vector + i * 4, r_u32(floor_offset + i * 4) - r_u32(position_offset + i * 4));
    }
    else
    {
        reciprocal = 0x40000000 / (sint32)(point_dot - floor_dot);
        ratio = (uint32)(((sint64)(sint32)point_dot * reciprocal) >> 18);
        w_u32(0x1F80030C, ratio);
        x = r_u32(floor_offset) - r_u32(vector);
        w_u32(vector, x);
        y = r_u32(floor_offset + 4) - r_u32(vector + 4);
        w_u32(vector + 4, y);
        z = r_u32(floor_offset + 8) - r_u32(vector + 8);
        w_u32(vector + 8, z);
        factor = (sint32)(ratio - 4096);
        w_u32(vector, (uint32)(((sint64)(sint32)x * factor) >> 12));
        w_u32(vector + 4, (uint32)(((sint64)(sint32)y * factor) >> 12));
        w_u32(vector + 8, (uint32)(((sint64)(sint32)z * factor) >> 12));
    }
    x = (uint32)(r_s32(vector) >> 6);
    z = (uint32)(r_s32(vector + 8) >> 6);
    w_u32(vector + 4, 0);
    squared = x * x + z * z;
    fallback_frame = z * z;
    limit = (uint32)(((sint64)scale * scale) >> 12);
    if (last_endpoint != 0)
        result = (sint32)limit < (sint32)squared ? -1 : r_s16(definition_offset + 26);
    else
        result = (sint32)limit < (sint32)squared ? -1 : 0;
    goto done;
interpolate:
    if (point_dot != floor_dot)
    {
        reciprocal = 0x40000000 / (sint32)(floor_dot - point_dot);
        ratio = (uint32)(((sint64)(sint32)floor_dot * reciprocal) >> 18);
        w_u32(0x1F80030C, ratio);
        x = r_u32(vector) - r_u32(floor_offset);
        w_u32(vector, x);
        y = r_u32(vector + 4) - r_u32(floor_offset + 4);
        w_u32(vector + 4, y);
        z = r_u32(vector + 8) - r_u32(floor_offset + 8);
        w_u32(vector + 8, z);
        w_u32(vector, (uint32)(((sint64)(sint32)x * (sint32)ratio) >> 12));
        w_u32(vector + 4, (uint32)(((sint64)(sint32)y * (sint32)ratio) >> 12));
        w_u32(vector + 8, (uint32)(((sint64)(sint32)z * (sint32)ratio) >> 12));
        for (i = 0; i < 3; ++i)
            w_u32(floor_offset + i * 4, r_u32(floor_offset + i * 4) + r_u32(vector + i * 4));
    }
    result = frame;
done:
    p2_guest_stack_pointer = saved_stack;
    output.value = result;
    output.fallback_frame = fallback_frame;
    return output;
}

uint32 p2_func_80040384(uint32 object_offset, uint32 vector_offset, sint16 step)
{
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
        value = p2_func_8005dc00((sint32)(0 - r_u32(vector_offset + 8)), (sint32)(0 - r_u32(vector_offset)));
    fast_turn = 0;
    if (r_s8(object_offset + 126) == 1)
    {
        kind = r_u16(0x800AC02A) & 0xF;
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
        value = (uint32)p2_func_8005dcd4(r_s32(vector_offset + 4), length);
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
        model = r_u32(0x8008B4B8 + ((r_u32(0x800AC028) >> 18) & 0x3C));
        w_u32(0x800AC074, model);
        w_u32(object_offset + 100, 0);
        w_u32(object_offset + 96, 0);
        w_u32(object_offset + 92, 0);
        w_u32(parent + 100, 0);
        w_u32(parent + 96, 0);
        w_u32(parent + 92, 0);
        w_u8(0x800AC030, 0);
        goto update_animation;
    }
    model = r_u32(0x8008B4B8 + (1 - (r_u16(0x800AC02A) & 0xF)) * 4);
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
        cap = r_s32(model + 24);
        w_u32(vector, (uint32)cap);
        if (cap < speed)
            w_u32(vector, (uint32)speed + (uint32)(speed >> 3));
    }
    flags = r_u32(object_offset + 28);
    w_u8(object_offset + 126, 1);
    w_u32(object_offset + 28, flags | 0x40);
    p2_func_8003db7c(object_offset, r_s32(vector), 0, 0);
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
    p2_func_80040384(object_offset, vector, r_s16(0x800AC070));
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
        definition = r_u32(model + 88);
    }
    else
    {
        animation = r_u32(object_offset + 104);
        definition = r_u32(model + 80);
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
    if ((r_u16(0x800AC02A) & 0xF) == 1 && (r_u32(0x8008A178) != 0 || (r_u32(0x800ABD6C) & 0x20) != 0))
        return 1;
    return 0;
}

sint32 p2_func_80047b1c(uint32 object_offset)
{
    uint32 flags = r_u32(object_offset + 28);
    uint32 character_flags, value;
    sint32 timer;
    if ((flags & 0x20000000) != 0 || (r_u32(0x800ABD6C) & 0x240) != 0)
    {
        character_flags = r_u32(0x800ABD6C);
        if ((character_flags & 0x40) != 0)
            return 4;
        if ((character_flags & 0x200) != 0)
            return 5;
        timer = r_s16(0x800ABD9A);
        if (timer >= 150)
            return 3;
        if (timer >= 75)
            value = (uint32)timer & 7;
        else if (timer >= 37)
            value = (uint32)timer & 3;
        else
            value = (uint32)timer & 1;
        return value == 0 ? 2 : 3;
    }
    if ((flags & 0x4000000) != 0 && (r_u16(0x800AC02A) & 0xF) == 1)
        return -1;
    if (r_s8(0x800AC031) == 9)
        return (r_u16(0x800AC02A) & 0xF) == 1 ? -1 : 0;
    return 0;
}

uint32 p2_func_8004dcd0(uint32 object_offset)
{
    uint32 saved_stack = p2_guest_stack_pointer;
    uint32 previous_kind = r_u8(0x800AC030);
    uint32 context = r_u32(0x8008A114);
    sint32 timer = r_s16(0x800ABD9E);
    uint32 local, flags, path, value, input, mask, result;
    p2_guest_stack_pointer -= 0x20;
    local = p2_guest_stack_pointer + 0x10;
    w_u8(0x800AC031, previous_kind);
    flags = r_u32(0x800ABD70);
    path = r_u32(context + 136);
    w_u32(0x800ABD70, flags & 0xF83FFFFF);
    w_u32(0x800AC018, path);
    if (timer != 0)
    {
        value = (uint32)timer - 1;
        w_u16(0x800ABD9E, value);
        if ((value & 0xFFFF) == 0)
            w_u32(0x800ABD68, r_u32(0x800ABD68) & 0xFFBFFFFF);
    }
    flags = r_u32(0x1F800320);
    input = r_u32(0x1F800384);
    w_u32(0x1F800388, input);
    if ((flags & 0x102) != 0)
    {
        w_u32(0x1F800384, 0);
        p2_func_80066d08();
    }
    else
    {
        value = p2_func_80066d08();
        w_u32(0x1F800384, p2_func_80060fdc(value));
    }
    mask = r_u32(0x8008A174);
    if (mask != 0)
    {
        input = r_u32(0x1F800384);
        w_u32(0x8008A174, mask & input);
        w_u32(0x1F800384, input & ~mask);
    }
    if ((sint32)p2_func_80026c14() < 0)
        w_u32(0x1F800384, r_u32(0x1F800384) & 0x900);
    p2_func_800276b4(0x1F800384);
    flags = r_u32(0x800ABD6C);
    if ((flags & 0x400) != 0 && r_s16(object_offset + 130) == r_s16(r_u32(object_offset + 104) + 6) - 1)
        w_u32(0x800ABD6C, flags & 0xFFFFFBFF);
    value = p2_func_8004aa68(object_offset, 0x800ABD68);
    flags = r_u32(object_offset + 28);
    w_u32(local, value);
    if ((flags & 0x8000000) != 0 && (r_u16(0x800AC02A) & 0xF) == 4)
        w_u32(local, value & 0xFFFFFFF7);
    p2_func_8004d5c0(object_offset, 0x800ABD68, local);
    p2_func_8004b8f8(object_offset, 0x800ABD68, local);
    p2_func_8004bfd0(object_offset, 0x800ABD68, local);
    p2_func_8004c3b0(object_offset, 0x800ABD68, local);
    p2_func_8004ca00(object_offset, 0x800ABD68, local);
    result = p2_func_8004cba4(object_offset, 0x800ABD68, r_u32(local));
    p2_guest_stack_pointer = saved_stack;
    return result;
}

uint32 p2_func_80060fdc(uint32 input)
{
    uint32 flags, consumed = 0, context, value;
    if ((input & 0xF) == 0)
    {
        w_u32(0x8008A360, 0);
        return input;
    }
    flags = r_u32(0x8008A52C);
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

uint32 p2_func_800276b4(uint32 destination_offset)
{
    uint32 source = r_u32(0x8008A310);
    uint32 result = source + 4;
    if (source != 0)
    {
        w_u32(0x8008A310, result);
        result = r_u32(source);
        w_u32(destination_offset, result);
    }
    return result;
}

uint32 p2_func_8004aa68(uint32 object_offset, uint32 character_offset)
{
    uint32 saved_stack = p2_guest_stack_pointer;
    uint32 local, result = 0, previous, flags, value, mask, matrix, filtered, x, y, z;
    sint32 timer, state, limit, response;
    p2_guest_stack_pointer -= 0x48;
    local = p2_guest_stack_pointer + 0x10;
    if ((r_u32(character_offset) & 0x4000000) != 0)
    {
        w_u8(object_offset + 13, r_u8(object_offset + 13) & 0xFE);
        w_u32(character_offset, r_u32(character_offset) & 0xFBFFFFFF);
        w_u32(0x1F800384, 0);
        goto finish;
    }
    if (r_s16(character_offset + 56) != 0 || (r_u8(object_offset + 13) & 1) != 0)
        goto finish;
    w_u32(local, r_u32(0x1F800384));
    previous = r_u32(0x1F800388);
    timer = r_s8(character_offset + 276);
    if (timer > 0)
        w_u8(character_offset + 276, (uint32)timer - 1);
    state = r_s8(character_offset + 476);
    if (state == 0)
    {
        if ((r_u32(0x8008A178) & 0x10) != 0 || (r_u32(0x8008A498) & 0x10) != 0)
            w_u8(character_offset + 476, 1);
    }
    else if (state == 1)
    {
        if ((r_u32(0x8008A178) & 0x10) == 0 && (r_u32(0x8008A498) & 0x10) == 0)
            w_u8(character_offset + 476, 2);
    }
    else
    {
        w_u8(character_offset + 476, 2);
        mask = r_u8(r_u32(0x8008A2B4));
        if ((r_u32(0x1F800384) & mask) == 0)
            w_u8(character_offset + 476, 0);
    }
    if ((r_u32(object_offset + 228) & 4) != 0)
        p2_func_8005b588(object_offset, character_offset);
    if ((r_u32(object_offset + 28) & 0x400) != 0)
        w_u32(character_offset + 272, r_u32(character_offset + 272) + 1);
    else if ((r_u32(object_offset + 228) & 4) == 0)
        w_u32(character_offset + 272, 0);
    value = r_u8(character_offset + 294);
    if (value != 0)
        w_u8(character_offset + 294, value - 1);
    if ((r_u32(object_offset + 228) & 0x40000) != 0 && r_s8(character_offset + 252) == 8)
    {
        value = r_u32(character_offset + 288) + 1;
        limit = r_s8(0x8008A1BC);
        w_u32(character_offset + 288, value);
        if ((uint32)limit < value)
            w_u32(object_offset + 228, r_u32(object_offset + 228) & 0xFFFBFFFF);
    }
    if ((r_u32(object_offset + 228) & 0x80) != 0 && r_s8(character_offset + 252) == 0)
    {
        response = p2_func_8005814c(object_offset, character_offset, local);
        if (response == -1)
            p2_func_800567fc(object_offset);
        else if (response == 1)
        {
            flags = r_u32(object_offset + 228);
            if ((flags & 1) != 0)
            {
                if ((flags & 0x400) != 0)
                {
                    value = r_u16(character_offset + 266);
                    result = 2;
                    w_u16(object_offset + 148, value);
                    w_u16(object_offset + 150, value);
                }
                p2_func_80058c78(object_offset);
            }
            if ((r_u32(object_offset + 228) & 0x820) != 0)
            {
                p2_func_800586e8(object_offset);
                p2_func_80058728(object_offset);
            }
            p2_func_80056700(object_offset);
        }
    }
    if ((r_u32(object_offset + 228) & 0x1000000) != 0 && ((r_u32(local) & (r_u8(r_u32(0x8008A2B4) + 1) | 0xA000)) != 0 || (r_u32(character_offset) & 0x60000) != 0))
        p2_func_800567e4(object_offset);
    if ((r_u32(object_offset + 228) & 0x20) != 0)
    {
        timer = r_s8(character_offset + 244);
        if (timer < r_s8(0x8008A21C))
        {
            value = (uint32)timer + 1;
            w_u8(character_offset + 244, value);
            if ((sint8)value < r_s8(0x8008A21C))
                w_u32(object_offset + 228, r_u32(object_offset + 228) | 0x8000);
        }
    }
    if ((r_u32(character_offset + 4) & 0x240) != 0 || ((uint32)r_u16(character_offset + 706) & 0xF) - 13 < 2)
        result |= 1;
    if ((r_u32(object_offset + 228) & 0x3C8) == 0)
        w_u32(character_offset + 408, 0);
    flags = r_u32(object_offset + 228);
    if ((flags & 0x100) != 0 || ((flags & 0x80) != 0 && r_s8(character_offset + 252) > 0))
    {
        if ((r_u32(character_offset + 4) & 0x240) != 0 && r_s8(character_offset + 252) < 4)
            result &= 0xFFFFFFFE;
        flags = r_u32(object_offset + 228);
        if ((flags & 0x640000) != 0 || r_u32(r_u32(0x8008A114) + 136) != r_u32(character_offset + 408))
            goto post_transition;
        if (r_s8(character_offset + 252) >= 4)
        {
            p2_func_80056774(object_offset);
            goto post_transition;
        }
        if (r_s8(character_offset + 285) >= r_s8(0x8008A1D9))
        {
            if (((flags & 0x400) != 0 && (r_u32(local) & 0x2000) != 0) || ((flags & 0x400) == 0 && (r_u32(local) & 0x8000) != 0))
            {
                p2_func_80057208(object_offset, character_offset);
                if ((r_u32(local) & 0x2000) != 0)
                    w_u32(object_offset + 228, r_u32(object_offset + 228) | 0x10000);
                else
                    w_u32(object_offset + 228, r_u32(object_offset + 228) | 0x20000);
                goto post_transition;
            }
        }
        if (r_s8(character_offset + 252) < 4 && (r_u32(character_offset + 4) & 0x240) == 0 && (r_u32(local) & 0x4000) != 0 && (previous & 0x4000) == 0)
            p2_func_80056810(object_offset, character_offset);
    post_transition:
        if ((r_u32(object_offset + 228) & 0x40100) != 0)
            result |= 0x20000;
    }
    if ((r_u32(r_u32(r_u32(r_u32(0x8008A114) + 136) + 36) + 16) & 0x80000) != 0 && (r_u32(object_offset + 228) & 0x30) == 0)
        p2_func_800585e0(object_offset);
    flags = r_u32(object_offset + 228);
    if ((flags & 0x10) != 0)
    {
        if ((r_u32(local) & 0x4000) != 0 && (previous & 0x4000) == 0 && r_s8(character_offset + 476) == 0 && (r_u32(local) & 0xA000) == 0 && (r_u32(object_offset + 28) & 0x400) == 0 && (r_u32(character_offset) & 0x800000) == 0)
        {
            if ((flags & 0x20) == 0)
                p2_func_80058640(object_offset);
        }
        else if ((r_u32(object_offset + 228) & 0x20) == 0)
        {
            if ((r_u32(r_u32(r_u32(r_u32(0x8008A114) + 136) + 36) + 16) & 0x80000) != 0)
                w_u8(character_offset + 244, 0);
            else
            {
                timer = r_s8(character_offset + 244);
                if (timer < 4)
                    w_u8(character_offset + 244, (uint32)timer + 1);
                else
                    p2_func_80058618(object_offset);
            }
        }
    }
    if ((r_u32(object_offset + 228) & 0x20) != 0)
        result |= 0x10000;
    if ((r_u32(object_offset + 228) & 0x1081) == 0 && (r_u32(r_u32(r_u32(r_u32(0x8008A114) + 136) + 36) + 16) & 0x10000) != 0)
    {
        p2_func_8005873c(object_offset);
        if ((r_u32(local) & 0xA000) != 0)
            value = (r_u32(local) & 0x2000) != 0 ? 16 : 32;
        else
            value = (r_u32(object_offset + 28) & 0x10) != 0 ? 16 : 32;
        w_u8(0x8008A490, value);
    }
    if ((r_u32(r_u32(r_u32(r_u32(0x8008A114) + 136) + 36) + 16) & 0x10000) == 0)
    {
        flags = r_u32(object_offset + 228);
        if ((flags & 1) == 0)
            goto ungrounded;
        if ((flags & 0x80) == 0 || (result & 0x800) == 0)
        {
            timer = r_s8(0x8008A524);
            if (timer < 4)
                w_u8(0x8008A524, (uint32)timer + 1);
            else
                p2_func_80058c78(object_offset);
        }
    }
    if ((r_u32(object_offset + 228) & 1) != 0)
    {
        mask = r_u8(r_u32(0x8008A2B4) + 1);
        if ((r_u32(local) & mask) != 0 && (previous & mask) == 0)
        {
            p2_func_80058ca0(object_offset, character_offset);
            state = r_s8(0x8008A490);
            w_u32(character_offset + 32, 1);
            w_u32(character_offset + 28, 0);
            w_u16(character_offset + 48, 0);
            w_u16(object_offset + 130, 0xFFFF);
            result |= 0x2000;
            if ((state == 16 && (r_u32(object_offset + 28) & 0x10) != 0) || (state == 32 && (r_u32(object_offset + 28) & 0x10) == 0))
            {
                p2_func_80046cbc(object_offset, character_offset);
                value = r_u16(object_offset + 150);
                w_u16(object_offset + 148, value);
                w_u16(object_offset + 88, (value + 2048) & 0xFFF);
            }
            goto movement;
        }
        if (r_s8(character_offset + 264) == 0)
        {
            w_u8(character_offset + 264, 1);
            goto movement;
        }
        if (r_s8(character_offset + 476) != 0)
            goto movement;
        matrix = r_u32(r_u32(object_offset + 132) + 40);
        filtered = r_u32(local);
        value = matrix != 0 && r_u32(matrix + 36) != 0;
        if (value == 0 && ((r_u32(character_offset) & 0x40) != 0 || r_u32(0x8008A40C) != 0))
        {
            if (r_u32(0x8008A40C) != 0)
            {
                w_u32(0x8008A40C, 2);
                filtered &= 0xFFFFEFFF;
            }
            else
            {
                value = r_u32(0x8008A464);
                if ((value & 0x800) != 0)
                {
                    filtered &= 0xFFFFEFFF;
                    w_u32(0x8008A458, value);
                    w_u32(0x8008A40C, 1);
                    w_u32(0x8008A414, 0);
                }
            }
            if (r_u32(0x8008A40C) != 0 && (r_u32(local) & 0x4000) != 0)
            {
                value = r_u32(0x8008A414) + 1;
                w_u32(0x8008A414, value);
                if ((sint32)value >= 2)
                    w_u32(0x8008A40C, 0);
            }
        }
        if ((r_u32(object_offset + 28) & 0x10) != 0)
        {
            if ((filtered & 0x1000) != 0)
                result |= 0x801;
            else if ((filtered & 0x4000) != 0)
                result |= 2;
        }
        else
        {
            if ((filtered & 0x4000) != 0)
                result |= 0x1001;
            else if ((filtered & 0x1000) != 0)
                result |= 2;
        }
        value = result & 0x1800;
        if (value != 0)
            w_u32(0x8008A464, value);
        if ((r_u32(local) & 0x2000) != 0)
            w_u8(0x8008A490, 32);
        if ((r_u32(local) & 0x8000) != 0)
            w_u8(0x8008A490, 16);
        result |= (uint32)r_s8(0x8008A490);
        goto movement;
    }
ungrounded:
    if ((r_u32(object_offset + 228) & 4) != 0 && (r_u32(object_offset + 28) & 0x400) == 0)
        p2_func_80058c78(object_offset);
movement:
    flags = r_u32(object_offset + 228);
    if ((flags & 6) != 2 && (flags & 0x101) == 0)
    {
        if ((r_u32(object_offset + 28) & 0x10) != 0)
        {
            if ((r_u32(local) & 0x2000) != 0)
                result |= 1;
            else if ((r_u32(local) & 0x8000) != 0)
                result |= 2;
        }
        else
        {
            if ((r_u32(local) & 0x8000) != 0)
                result |= 1;
            else if ((r_u32(local) & 0x2000) != 0)
                result |= 2;
        }
    }
    if ((r_u32(object_offset + 228) & 0x10) != 0)
    {
        result |= 0x8000;
        if ((r_u32(local) & 0x1000) != 0 && (r_u32(object_offset + 28) & 0x400) == 0)
            result = (result | 0x800) & 0xFFFFFFFC;
    }
    if (r_s32(character_offset) < 0)
        goto finish;
    flags = r_u32(object_offset + 228);
    if ((flags & 0x1120) != 0)
        goto finish;
    if ((r_u32(character_offset + 4) & 0x240) != 0)
        goto button_actions;
    if ((r_u32(local) & 0x4000) != 0 && (flags & 0x15) == 0)
        result |= 0x80;
    mask = r_u8(r_u32(0x8008A2B4));
    if ((r_u32(local) & mask) != 0)
    {
        if ((previous & mask) == 0)
            result |= 0x40;
        else
        {
            value = r_u16(character_offset + 706) & 0xF;
            if (value == 4 || value == 5 || value - 10 < 2)
                result |= 0x40;
        }
    }
    if ((result & 0x40) != 0)
    {
        if ((r_u32(character_offset + 4) & 0x100) != 0)
            w_u32(object_offset + 28, r_u32(object_offset + 28) | 0x1000000);
        flags = r_u32(0x8008A178);
        if ((flags & 4) != 0)
        {
            w_u32(0x8008A178, flags | 8);
            result ^= 0x40;
        }
    }
    else
    {
        flags = r_u32(0x8008A178);
        mask = r_u32(0x8008A2B4);
        value = flags & 0xFFFFFFF7;
        w_u32(0x8008A178, value);
        mask = r_u8(mask);
        if ((r_u32(local) & mask) != 0)
        {
            if ((flags & 0x10) != 0)
                result &= 0xFFFFFFFD;
        }
        else
        {
            x = r_u32(0x8008A498);
            w_u32(0x8008A178, value & 0xFFFFFFEF);
            w_u32(0x8008A498, x & 0xFFFFFFEF);
        }
    }
    value = r_u16(character_offset + 706) & 0xF;
    if (value == 5 || value - 10 < 2)
    {
        if ((result & 0x80) != 0)
        {
            flags = r_u32(object_offset + 28);
            result &= 0xFFFFFF7F;
            if ((flags & 0x400) != 0)
                result |= 0x200;
        }
        else if ((r_u32(local) & 0x1000) != 0)
            result |= 0x100;
    }
    else if (value == 1)
    {
        if ((r_u32(object_offset + 28) & 0x4000000) != 0)
            result = 1024;
        else
        {
            mask = r_u8(r_u32(0x8008A2B4) + 2);
            value = r_u32(local) & mask;
            if (value == mask && (previous & value) != value)
                result |= 0x400;
        }
    }
button_actions:
    mask = r_u8(r_u32(0x8008A2B4) + 1);
    if ((r_u32(local) & mask) != 0)
    {
        result |= 8;
        if ((previous & mask) == 0)
        {
            if (r_u32(character_offset + 104) != 0)
            {
                if (r_s16(character_offset + 58) < 6)
                {
                    x = r_u32(character_offset + 96);
                    y = r_u32(character_offset + 100);
                    z = r_u32(character_offset + 104);
                    flags = r_u32(character_offset);
                    w_u32(character_offset + 32, 1);
                    w_u16(character_offset + 48, 0);
                    w_u32(character_offset + 24, x);
                    w_u32(character_offset + 28, y);
                    w_u32(character_offset + 728, z);
                    w_u32(character_offset, flags | 0x10000000);
                }
                w_u32(character_offset + 104, 0);
            }
            w_u16(character_offset + 58, 0xFFFF);
        }
    }
    timer = r_s16(character_offset + 58);
    if (timer < 6)
        w_u16(character_offset + 58, (uint32)timer + 1);
finish:
    if (r_u32(0x8008A184) == 1)
    {
        flags = r_u32(character_offset);
        w_u32(0x8008A184, 0);
        w_u32(character_offset, flags & 0xFFFFF7EF);
    }
    p2_guest_stack_pointer = saved_stack;
    return result;
}
