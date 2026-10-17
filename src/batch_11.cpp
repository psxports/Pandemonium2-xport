#include "game.h"
#include <string.h>

uint32 p2_func_80045d0c(uint32 object_offset, uint32 character_offset)
{
    FUNCTION_MARKER(0x80045D0C, "SLUS_005.78");
    uint32 state, x, y, z, parent, matrix, saved_stack, vector, result;
    state = r_u32(0x8008A114);
    x = r_u32(state + 28);
    y = r_u32(state + 32);
    z = r_u32(state + 36);
    w_u32(state + 4, x);
    w_u32(state + 8, y);
    w_u32(state + 12, z);
    parent = r_u32(object_offset + 132);
    if (parent != 0 && r_u32(parent + 40) != 0)
    {
        saved_stack = p2_guest_stack_pointer;
        p2_guest_stack_pointer -= 16;
        vector = p2_guest_stack_pointer;
        x = r_u32(object_offset + 16);
        y = r_u32(object_offset + 20);
        z = r_u32(object_offset + 24);
        w_u32(vector, x);
        w_u32(vector + 4, y);
        w_u32(vector + 8, z);
        PushMatrix();
        matrix = r_u32(r_u32(object_offset + 132) + 40);
        x = r_u32(matrix);
        y = r_u32(matrix + 4);
        xport_gte_write_control(0, x);
        xport_gte_write_control(1, y);
        x = r_u32(matrix + 8);
        y = r_u32(matrix + 12);
        z = r_u32(matrix + 16);
        xport_gte_write_control(2, x);
        xport_gte_write_control(3, y);
        xport_gte_write_control(4, z);
        state = r_u32(0x8008A114);
        p2_sdk_8007ec64(vector, state + 16);
        PopMatrix();
        state = r_u32(0x8008A114);
        matrix = r_u32(r_u32(object_offset + 132) + 40);
        w_u32(state + 16, r_u32(state + 16) + r_u32(matrix + 20));
        matrix = r_u32(r_u32(object_offset + 132) + 40);
        w_u32(state + 20, r_u32(state + 20) + r_u32(matrix + 24));
        matrix = r_u32(r_u32(object_offset + 132) + 40);
        w_u32(state + 24, r_u32(state + 24) + r_u32(matrix + 28));
        p2_guest_stack_pointer = saved_stack;
    }
    else
    {
        state = r_u32(0x8008A114);
        x = r_u32(object_offset + 16);
        y = r_u32(object_offset + 20);
        z = r_u32(object_offset + 24);
        w_u32(state + 16, x);
        w_u32(state + 20, y);
        w_u32(state + 24, z);
    }
    state = r_u32(0x8008A114);
    result = character_controller.flags;
    y = r_u32(state + 20);
    w_u32(state + 20, y + ((sint32)result < 0 ? 0x8000 : 0x3800));
    character_controller.motion.jump_phase = 0;
    character_controller.motion.vertical_speed = 0;
    character_controller.motion.horizontal_speed = 0;
    w_u32(object_offset + 100, 0);
    w_u32(object_offset + 96, 0);
    w_u32(object_offset + 92, 0);
    w_u8(object_offset + 124, 1);
    result = character_controller.flags & 0xF7FFFFFF;
    character_controller.flags = result;
    return result;
}

void p2_func_80046628(uint32 object_offset, uint32 first, uint32 second, uint32 third, uint32 kind)
{
    FUNCTION_MARKER(0x80046628, "SLUS_005.78");
    uint32 saved_stack, initial, target, state, parent, matrix, x, y, z, value, direction, path, frame;
    uint32 flags, definition, duration, denominator;
    sint32 coefficient, sample_frame, reciprocal;
    sint64 product;
    if (kind == 87)
    {
        w_u32(0x8008A47C, 30);
        w_u32(0x8008A508, 15);
    }
    else if (kind == 66)
    {
        w_u32(0x8008A470, 30);
        w_u32(0x8008A4D4, 15);
    }
    value = player_interaction.flags;
    if (value != 0)
        player_interaction.flags = value | 0x40;
    character_controller.ClearTransientFlags();
    if (character_action_flags != 0)
    {
        character_controller.interaction_phase = 0;
        character_controller.ClearInteractionActions();
    }
    saved_stack = p2_guest_stack_pointer;
    p2_guest_stack_pointer -= 32;
    initial = p2_guest_stack_pointer;
    target = initial + 16;
    if ((r_u32(object_offset + 28) & 0x2000) == 0 && (parent = r_u32(object_offset + 132)) != 0 && r_u32(parent + 40) != 0)
    {
        x = r_u32(object_offset + 16);
        y = r_u32(object_offset + 20);
        z = r_u32(object_offset + 24);
        w_u32(initial, x);
        w_u32(initial + 4, y);
        w_u32(initial + 8, z);
        PushMatrix();
        matrix = r_u32(r_u32(object_offset + 132) + 40);
        x = r_u32(matrix);
        y = r_u32(matrix + 4);
        xport_gte_write_control(0, x);
        xport_gte_write_control(1, y);
        x = r_u32(matrix + 8);
        y = r_u32(matrix + 12);
        z = r_u32(matrix + 16);
        xport_gte_write_control(2, x);
        xport_gte_write_control(3, y);
        xport_gte_write_control(4, z);
        state = r_u32(0x8008A114);
        p2_sdk_8007ec64(initial, state + 16);
        PopMatrix();
        state = r_u32(0x8008A114);
        matrix = r_u32(r_u32(object_offset + 132) + 40);
        w_u32(state + 16, r_u32(state + 16) + r_u32(matrix + 20));
        matrix = r_u32(r_u32(object_offset + 132) + 40);
        w_u32(state + 20, r_u32(state + 20) + r_u32(matrix + 24));
        matrix = r_u32(r_u32(object_offset + 132) + 40);
        w_u32(state + 24, r_u32(state + 24) + r_u32(matrix + 28));
    }
    else
    {
        state = r_u32(0x8008A114);
        x = r_u32(object_offset + 16);
        y = r_u32(object_offset + 20);
        z = r_u32(object_offset + 24);
        w_u32(state + 16, x);
        w_u32(state + 20, y);
        w_u32(state + 24, z);
    }
    w_u32(object_offset + 28, r_u32(object_offset + 28) & 0xEFFF93EF);
    x = character_controller.motion.vertical_speed;
    y = (uint32)character_controller.movement.fall_speed_limit;
    if ((sint32)y >= (sint32)x)
        character_controller.timers.state_ticks = 0;
    path = first;
    frame = second;
    duration = third;
    flags = r_u16(path + 14);
    direction = flags & 0x10;
    if ((sint32)frame < 0)
    {
        frame = ~frame;
        if ((flags & 8) != 0)
            path = r_u32(path + 16);
        direction ^= 0x10;
    }
    w_u32(object_offset + 28, r_u32(object_offset + 28) | 0x20000 | direction);
    if ((r_u32(r_u32(path + 36) + 16) & 0x11) != 0)
    {
        x = character_controller.movement.jump_ticks;
        flags = character_controller.flags;
        character_controller.motion.jump_phase = -1;
        character_controller.motion.vertical_speed = 0;
        character_controller.timers.state_ticks = 0;
        character_controller.timers.jump_duration_ticks = x;
        character_controller.flags = flags | 0x08000000;
        if ((character_controller.secondary_flags & 0x80) == 0)
            character_controller.motion.state_code = 6;
        w_u32(object_offset + 28, (r_u32(object_offset + 28) | 0x400) & 0xF7FFFFFF);
    }
    w_u32(object_offset + 132, r_u32(path + 36));
    coefficient = r_s16(r_u32(path + 36) + 36);
    if (coefficient != 0)
        frame = (uint32)((sint32)((uint32)coefficient * frame) >> 12);
    state = r_u32(0x8008A114);
    w_u16(state + 2, frame);
    w_u16(object_offset + 136, frame);
    w_u16(object_offset + 148, 4096);
    character_controller.flags = character_controller.flags & 0xFCC5F459;
    SelectScenePath(path);
    state = r_u32(0x8008A114);
    SamplePathOffset(path, (sint32)frame, state + 28, direction);
    state = r_u32(0x8008A114);
    definition = r_u32(r_u32(state + 136) + 36);
    if (direction != 0)
        sample_frame = r_s16(state + 2);
    else
        sample_frame = (sint32)((uint32)(sint32)r_s16(definition + 26) - (uint32)(sint32)r_s16(state + 2) - 1);
    SampleVectorTrack(sample_frame, definition + 20, object_offset + 16);
    w_u16(object_offset + 136, r_u16(r_u32(0x8008A114) + 2));
    parent = r_u32(object_offset + 132);
    if (parent != 0 && r_u32(parent + 40) != 0)
    {
        x = r_u32(object_offset + 16);
        y = r_u32(object_offset + 20);
        z = r_u32(object_offset + 24);
        w_u32(target, x);
        w_u32(target + 4, y);
        w_u32(target + 8, z);
        PushMatrix();
        matrix = r_u32(r_u32(object_offset + 132) + 40);
        x = r_u32(matrix);
        y = r_u32(matrix + 4);
        xport_gte_write_control(0, x);
        xport_gte_write_control(1, y);
        x = r_u32(matrix + 8);
        y = r_u32(matrix + 12);
        z = r_u32(matrix + 16);
        xport_gte_write_control(2, x);
        xport_gte_write_control(3, y);
        xport_gte_write_control(4, z);
        p2_sdk_8007ec64(target, object_offset + 92);
        PopMatrix();
        parent = r_u32(object_offset + 132);
        matrix = r_u32(parent + 40);
        x = r_u32(object_offset + 92);
        x += r_u32(matrix + 20);
        parent = r_u32(object_offset + 132);
        w_u32(object_offset + 92, x);
        matrix = r_u32(parent + 40);
        y = r_u32(object_offset + 96);
        y += r_u32(matrix + 24);
        parent = r_u32(object_offset + 132);
        w_u32(object_offset + 96, y);
        matrix = r_u32(parent + 40);
        w_u32(object_offset + 100, r_u32(object_offset + 100) + r_u32(matrix + 28));
    }
    else
    {
        x = r_u32(object_offset + 16);
        y = r_u32(object_offset + 20);
        z = r_u32(object_offset + 24);
        w_u32(object_offset + 92, x);
        w_u32(object_offset + 96, y);
        w_u32(object_offset + 100, z);
    }
    state = r_u32(0x8008A114);
    w_u32(object_offset + 92, r_u32(object_offset + 92) - r_u32(state + 16));
    w_u32(object_offset + 96, r_u32(object_offset + 96) - r_u32(state + 20));
    w_u32(object_offset + 100, r_u32(object_offset + 100) - r_u32(state + 24));
    w_u8(state + 74, duration);
    w_u8(state + 73, duration);
    w_u8(state + 72, duration);
    if (duration == 0)
    {
        x = r_u32(object_offset + 92) << 1;
        y = r_u32(object_offset + 96) << 1;
        z = r_u32(object_offset + 100) << 1;
        w_u32(object_offset + 60, x);
        w_u32(object_offset + 64, y);
        w_u32(object_offset + 68, z);
        p2_func_80045d0c(object_offset, 0x800ABD68);
        state = r_u32(0x8008A114);
        w_u8(state, r_u8(state) & 0xFE);
    }
    else
    {
        character_controller.timers.transition_ticks = duration;
        x = r_u32(object_offset + 92);
        denominator = duration << 12;
        if (denominator == 0)
            xport_mips_break(7);
        reciprocal = 0x01000000 / (sint32)denominator;
        product = (sint64)(sint32)x * reciprocal;
        x = (uint32)((uint64)product >> 12);
        y = r_u32(object_offset + 96);
        w_u32(object_offset + 92, x);
        product = (sint64)(sint32)y * reciprocal;
        y = (uint32)((uint64)product >> 12);
        z = r_u32(object_offset + 100);
        w_u32(object_offset + 96, y);
        product = (sint64)(sint32)z * reciprocal;
        w_u32(object_offset + 100, (uint32)((uint64)product >> 12));
        state = r_u32(0x8008A114);
        flags = character_controller.flags;
        y = r_u32(state + 20);
        w_u32(state + 20, y + ((sint32)flags < 0 ? 0x8000 : 0x3800));
        w_u8(object_offset + 13, r_u8(object_offset + 13) | 1);
        w_u8(object_offset + 124, 0);
        character_controller.flags = character_controller.flags | 0x04000000;
    }
    SamplePathTangent(r_u32(object_offset + 132), &character_controller.path_tangent, (sint32)direction);
    p2_guest_stack_pointer = saved_stack;
    return;
}

uint32 p2_func_80044780(void)
{
    FUNCTION_MARKER(0x80044780, "SLUS_005.78");
    return (uint16)(character_controller.motion.form_flags >> 16) & 15;
}

uint32 p2_func_8003420c(uint32 index)
{
    FUNCTION_MARKER(0x8003420C, "SLUS_005.78");
    uint32 offset, asset, counter, base, object, table, header, saved_flags, node, value;
    sint32 callback_index;
    offset = resource_archive.Offset(r_u32(0x8008A46C));
    asset = r_u32(0x8008A46C);
    counter = r_u8(0x8008A330);
    base = r_u32(0x8008A488);
    object = base + offset;
    w_u8(0x8008A330, counter + 1);
    w_u32(0x8008B450 + ((uint32)(sint32)(sint8)counter << 2), object);
    value = resource_archive.Size(asset + 4);
    if ((sint32)value <= 0)
        w_u32(object + 128, 0);
    else
    {
        offset = resource_archive.Offset(r_u32(0x8008A46C) + 4);
        base = r_u32(0x8008A488);
        value = p2_func_8006aa48(base + offset);
        w_u32(object + 128, value);
    }
    callback_index = r_s8(object + 61);
    table = callback_index < 44 ? 0x80083D6C : 0x80083D70;
    w_u32(object, r_u32(table + ((uint32)callback_index << 2)));
    callback_index = r_s8(object + 633);
    if (callback_index != -1)
        w_u32(object, r_u32(0x80083D6C + ((uint32)callback_index << 2)));
    asset = r_u32(0x8008A46C);
    saved_flags = r_u32(object + 48);
    offset = resource_archive.Offset(asset + 3);
    header = r_u32(0x8008A488) + offset;
    RelocateMaterialResource(header, -1);
    offset = resource_archive.Offset(r_u32(0x8008A46C) + 1);
    node = r_u32(0x8008A488) + offset;
    w_u32(object + 48, node);
    BindModelMaterials(node, header);
    offset = resource_archive.Offset(r_u32(0x8008A46C) + 2);
    base = r_u32(0x8008A488);
    node = r_u32(object + 48);
    LoadAnimationTable(node, base + offset, (uint32 *)psx_addr(object + 72, (r_u32(base + offset) - 1) * 4));
    node = r_u32(object + 48);
    w_u8(node + 13, r_u8(node + 13) | 0x80);
    node = r_u32(object + 48);
    w_u32(node + 28, r_u32(node + 28) | 0x20024 | saved_flags);
    node = r_u32(object + 48);
    asset = r_u32(0x8008A46C);
    value = r_u32(node + 224);
    w_u32(0x8008A46C, asset + 5);
    w_u16(object + 70, value);
    return 1;
}







uint32 p2_func_80025be8(void)
{
    FUNCTION_MARKER(0x80025BE8, "SLUS_005.78");
    w_u16(0x800B99CE, 15);
    return 15;
}



uint32 p2_func_8005c4f4(uint32 object_offset)
{
    FUNCTION_MARKER(0x8005C4F4, "SLUS_005.78");
    uint32 table, voice, index, result;
    table = r_u32(0x8008A460);
    voice = r_u8(object_offset + 76);
    if (table != 0)
    {
        index = r_u32(object_offset + 68);
        result = r_u32(table);
        index &= 0xFFF;
        if (index < result)
            table += index * 8 + 4;
        else
            table = 0;
    }
    result = sound_effects.IsPlaying(table, voice);
    if ((result << 24) != 0)
        p2_func_8006a970((uint16)voice);
    result = r_u32(object_offset + 76) | 0xFF;
    w_u32(object_offset + 76, result);
    return result;
}

uint32 p2_func_80030bd4(uint32 object_offset)
{
    FUNCTION_MARKER(0x80030BD4, "SLUS_005.78");
    uint32 sound, voice, result;
    sound = r_u32(object_offset + 68);
    voice = (uint32)(sint32)r_s16(object_offset + 66);
    result = sound_effects.IsPlaying(sound, voice);
    if ((result << 24) != 0)
        p2_func_8006a970(r_u16(object_offset + 66));
    w_u32(object_offset + 68, 0);
    w_u16(object_offset + 66, 0xFFFF);
    return 0xFFFFFFFF;
}

uint32 p2_func_80025f58(void)
{
    FUNCTION_MARKER(0x80025F58, "SLUS_005.78");
    uint32 node, scene, kind, flags, result;
    node = registered_object_head;
    while (node != 0)
    {
        if (r_s8(node + 125) == 6)
            p2_func_8002c510(node);
        node = r_u32(node);
    }
    scene = r_u32(0x8008A4E8);
    node = r_u32(scene + 44);
    if (node != 0)
    {
        do
        {
            kind = r_u8(node + 15);
            if (kind - 40 >= 16)
                break;
            if ((sint8)kind == 50)
            {
                flags = r_u32(node + 72);
                if (((flags >> 17) & 0xFF) == 0 && (flags >> 25) == 0 && r_u8(node + 76) < 8)
                    p2_func_8005c4f4(node);
            }
            node = r_u32(node);
        } while (node != 0);
        scene = r_u32(0x8008A4E8);
    }
    node = r_u32(scene + 32);
    while (node != 0)
    {
        p2_func_800425f4(r_u32(r_u32(node + 132) + 44));
        node = r_u32(node);
    }
    result = r_u32(0x8008A4E8);
    node = r_u32(result + 68);
    while (node != 0)
    {
        result = p2_func_80030bd4(node);
        node = r_u32(node);
    }
    return result;
}

uint32 p2_func_80063e78(uint32 node_offset)
{
    FUNCTION_MARKER(0x80063E78, "SLUS_005.78");
    uint32 saved_stack, corners, packet, matrix, x, y, z, value, sum, limit, result, context, end, start, link, bucket_offset;
    uint32 material, texture, flags, uv0, uv1, uv2, uv3, orientation;
    sint32 height, bucket, distance, fade;
    sint64 product;
    packet = render_commands.packet_offset;
    limit = render_commands.packet_limit;
    if (packet >= limit)
        return 0;
    saved_stack = p2_guest_stack_pointer;
    p2_guest_stack_pointer -= 0x20;
    corners = p2_guest_stack_pointer;
    matrix = r_u32(0x8008A0C8);
    x = r_u32(matrix + 20);
    y = r_u32(matrix + 24);
    z = r_u32(matrix + 28);
    xport_gte_write_control(5, (uint32)((sint32)x >> 5));
    xport_gte_write_control(6, (uint32)((sint32)y >> 5));
    xport_gte_write_control(7, (uint32)((sint32)z >> 5));
    value = (uint32)(sint32)r_s16(node_offset + 40);
    height = r_s16(node_offset + 42);
    w_u16(corners + 4, 0);
    w_u16(corners + 12, 0);
    w_u16(corners + 20, 0);
    w_u16(corners + 28, 0);
    x = 0 - (value & 0xFFFF);
    height += 10;
    y = 0 - (uint32)height;
    w_u16(corners, x);
    w_u16(corners + 2, y);
    w_u16(corners + 8, value);
    w_u16(corners + 10, y);
    w_u16(corners + 16, value);
    w_u16(corners + 18, height);
    w_u16(corners + 24, x);
    w_u16(corners + 26, height);
    xport_gte_write_data(0, r_u32(corners));
    xport_gte_write_data(1, r_u16(corners + 4));
    xport_gte_write_data(2, r_u32(corners + 8));
    xport_gte_write_data(3, r_u16(corners + 12));
    xport_gte_write_data(4, r_u32(corners + 16));
    xport_gte_write_data(5, r_u16(corners + 20));
    xport_gte_execute(0x280030);
    w_u32(corners, xport_gte_read_data(12));
    w_u32(corners + 8, xport_gte_read_data(13));
    w_u32(corners + 16, xport_gte_read_data(14));
    w_u32(corners + 4, xport_gte_read_data(17));
    w_u32(corners + 12, xport_gte_read_data(18));
    w_u32(corners + 20, xport_gte_read_data(19));
    xport_gte_write_data(0, r_u32(corners + 24));
    xport_gte_write_data(1, r_u16(corners + 28));
    xport_gte_execute(0x180001);
    w_u32(corners + 24, xport_gte_read_data(14));
    w_u32(corners + 28, xport_gte_read_data(19));
    if (r_u16(corners) >= 320 && r_u16(corners + 8) >= 320 && r_u16(corners + 16) >= 320 && r_u16(corners + 24) >= 320)
    {
        result = 0;
        goto done;
    }
    if (r_u16(corners + 2) >= 240 && r_u16(corners + 10) >= 240 && r_u16(corners + 18) >= 240 && r_u16(corners + 26) >= 240)
    {
        result = 0;
        goto done;
    }
    x = r_u32(corners + 4);
    y = r_u32(corners + 12);
    z = r_u32(corners + 20);
    value = r_u32(corners + 28);
    limit = render_view.far_distance;
    sum = x + y + z + value;
    result = (sint32)(sum << 1) < (sint32)limit;
    if (result == 0)
        goto done;
    x = render_view.fog_scale;
    value = (uint32)((sint32)sum >> 2);
    bucket = (sint32)(value * x) >> 19;
    if ((uint32)bucket >= 1024)
        bucket = bucket < 0 ? 0 : 1023;
    xport_gte_write_data(8, 0);
    context = r_u32(0x8008A4E8);
    end = r_u32(context + 152);
    if (end != 0)
    {
        value = render_view.far_distance;
        product = (sint64)(sint32)value * (sint32)((uint32)bucket << 2);
        distance = (sint32)((uint64)product >> 12);
        start = r_u32(context + 148);
        if ((sint32)start < distance)
        {
            result = (uint32)distance - start;
            if (distance >= (sint32)end)
                goto done;
            product = (sint64)((sint32)render_view.depth_scale) * (sint32)result;
            fade = (sint32)((uint64)product >> 12) >> 8;
            xport_gte_write_data(8, (uint32)fade);
        }
        else
            xport_gte_write_data(8, 0);
    }
    else
        xport_gte_write_data(8, 0);
    bucket_offset = render_commands.ordering_table_offset + (uint32)bucket * 4;
    link = r_u32(bucket_offset);
    packet = render_commands.packet_offset;
    w_u32(packet, (link & 0xFFFFFF) | 0x09000000);
    link = r_u32(bucket_offset);
    w_u32(bucket_offset, (link & 0xFF000000) | (packet & 0xFFFFFF));
    material = r_u32(node_offset + 16);
    packet = render_commands.packet_offset;
    texture = r_u32(material + 4);
    uv0 = texture & 0xFFFF;
    uv3 = texture >> 16;
    texture = r_u32(material + 8);
    flags = r_u32(material);
    uv0 |= texture & 0xFFFF0000;
    uv1 = (uv0 & 0xFF00) | (uv3 & 0xFF) | (texture << 16);
    if ((flags & 0x80) != 0)
        uv1 |= (flags & 0x60) << 16;
    uv2 = (uv3 & 0xFF00) | (uv0 & 0xFF);
    xport_gte_write_data(6, r_u32(material + 16));
    xport_gte_execute(0x780010);
    w_u32(packet + 4, xport_gte_read_data(22));
    w_u8(packet + 7, (r_u8(material + 19) & 3) | 0x2C);
    w_u32(packet + 12, uv0);
    w_u32(packet + 20, uv1);
    w_u32(packet + 28, uv2);
    w_u32(packet + 36, uv3);
    orientation = (r_u16(node_offset + 14) & 0x180) >> 7;
    switch (orientation)
    {
        case 0:
            w_u32(packet + 8, r_u32(corners));
            w_u32(packet + 16, r_u32(corners + 8));
            w_u32(packet + 24, r_u32(corners + 24));
            w_u32(packet + 32, r_u32(corners + 16));
            break;
        case 1:
            w_u32(packet + 8, r_u32(corners + 24));
            w_u32(packet + 16, r_u32(corners));
            w_u32(packet + 24, r_u32(corners + 16));
            w_u32(packet + 32, r_u32(corners + 8));
            break;
        case 2:
            w_u32(packet + 8, r_u32(corners + 16));
            w_u32(packet + 16, r_u32(corners + 24));
            w_u32(packet + 24, r_u32(corners + 8));
            w_u32(packet + 32, r_u32(corners));
            break;
        case 3:
            w_u32(packet + 8, r_u32(corners + 8));
            w_u32(packet + 16, r_u32(corners + 16));
            w_u32(packet + 24, r_u32(corners));
            w_u32(packet + 32, r_u32(corners + 24));
            break;
    }
    result = packet + 40;
    render_commands.packet_offset = result;
done:
    p2_guest_stack_pointer = saved_stack;
    return result;
}



uint32 p2_func_8002ab38(uint32 value)
{
    FUNCTION_MARKER(0x8002AB38, "SLUS_005.78");
    uint32 remainder, result, trial, count;
    remainder = 0;
    result = 0;
    count = 22;
    do
    {
        remainder = (remainder << 2) | (value >> 30);
        result <<= 1;
        trial = (result << 1) + 1;
        value <<= 2;
        if (remainder >= trial)
        {
            remainder -= trial;
            ++result;
        }
        --count;
    } while (count != 0);
    return result;
}

uint32 p2_func_80044ffc(sint32 action, uint32 audio_value)
{
    FUNCTION_MARKER(0x80044FFC, "SLUS_005.78");
    uint32 mode, table, sound, index, minimum;
    if (action == 2)
    {
        mode = (uint16)(character_controller.motion.form_flags >> 16) & 15;
        if (mode == 1 || mode == 7 || mode == 9 || mode == 14 || mode == 11)
            action = 3;
    }
    table = r_u32(0x8008A408);
    if (table != 0 && action < r_s32(table))
        sound = table + ((uint32)action << 3) + 4;
    else
        sound = 0;
    mode = (uint16)(character_controller.motion.form_flags >> 16) & 15;
    minimum = 0;
    index = 0;
    if (mode == 4)
    {
        if (action == 0)
        {
            minimum = 12;
            index = 11;
        }
        else if (action == 1)
        {
            minimum = 13;
            index = 12;
        }
        else if (action == 2)
        {
            minimum = 14;
            index = 13;
        }
        else if (action == 7)
        {
            minimum = 4;
            index = 3;
        }
    }
    else if (mode == 5 && action == 2)
    {
        minimum = 14;
        index = 13;
    }
    if (minimum != 0)
    {
        table = r_u32(0x8008A460);
        sound = 0;
        if (table != 0 && r_s32(table) >= (sint32)minimum)
            sound = table + index * 8 + 4;
    }
    mode = sound_settings.effects_volume;
    return sound_effects.Queue(sound, 0, 0, mode, 0, audio_value);
}

uint32 p2_func_80046510(uint32 object_offset, uint32 path_offset, sint32 frame)
{
    FUNCTION_MARKER(0x80046510, "SLUS_005.78");
    uint32 state, result, direction, previous, definition, value;
    sint32 previous_frame;
    state = r_u32(0x8008A114);
    result = r_u32(state + 136);
    if (path_offset != result)
    {
        direction = r_u32(object_offset + 28) & 0x10;
        ProjectPathOffset(path_offset, frame, state + 28, direction);
        state = r_u32(0x8008A114);
        previous = r_u32(state + 136);
        definition = r_u32(path_offset + 36);
        if (definition == r_u32(previous + 36))
            direction ^= 0x10;
        else
            ApplyPathVelocityChange(object_offset, definition);
        state = r_u32(0x8008A114);
        if ((r_u32(state + 72) & 0xFFFFFF) == 0)
        {
            previous = r_u32(state + 136);
            value = r_u32(0x800AC018);
            if (previous == value && (r_u32(object_offset + 28) & 0x2000) == 0)
            {
                previous_frame = r_s16(object_offset + 136);
                if (previous_frame >= 0)
                    ProjectPathOffset(previous, previous_frame, state + 4, direction);
            }
        }
        result = SelectScenePath(path_offset);
    }
    w_u16(object_offset + 136, frame);
    return result;
}



uint32 p2_func_80024b3c(uint32 vector_offset)
{
    FUNCTION_MARKER(0x80024B3C, "SLUS_005.78");
    uint32 axis, value, state;
    sint32 count;
    for (axis = 0; axis < 3; ++axis)
    {
        value = r_u32(vector_offset + axis * 4);
        if ((sint32)value < 0)
            value = 0 - value;
        count = (sint32)value >> 12;
        if (count != 0)
        {
            count = (count >> 1) + 1;
            if (count >= 46)
                count = 45;
        }
        else
        {
            state = r_u32(0x8008A114);
            if (r_u8(state + 72 + axis) != 0)
                count = 1;
        }
        state = r_u32(0x8008A114);
        w_u8(state + 72 + axis, count);
    }
    return state;
}



uint32 p2_func_80056700(uint32 object_offset)
{
    FUNCTION_MARKER(0x80056700, "SLUS_005.78");
    uint32 flags, state;
    flags = r_u32(object_offset + 228);
    state = r_u32(object_offset + 28);
    w_u32(object_offset + 228, flags | 0x100);
    w_u32(object_offset + 28, state & 0xFFFFFBFF);
    flags = character_controller.flags;
    w_u8(0x800ABE64, 1);
    w_u32(0x800ABE80, 0xFFFFFFFF);
    w_u8(0x800ABE8C, 0);
    w_u8(0x800ABE85, 0);
    w_u8(0x800ABE84, 0);
    character_controller.motion.jump_phase = -1;
    character_controller.flags = flags & 0xE7FFFFBF;
    return 0xFFFFFFFF;
}

sint32 p2_func_8005814c(uint32 object_offset, uint32 character_offset, uint32 input_offset)
{
    FUNCTION_MARKER(0x8005814C, "SLUS_005.78");
    uint32 saved_stack, current, target, first, second, data, path, matrix, x, y, z, flags, value;
    uint32 height, distance, dx, dz, angle, difference, result;
    sint32 first_frame, second_frame;
    saved_stack = p2_guest_stack_pointer;
    p2_guest_stack_pointer -= 64;
    current = p2_guest_stack_pointer;
    target = current + 16;
    first = current + 32;
    second = current + 48;
    data = r_u32(character_offset + 256);
    path = r_u32(data + 68);
    result = 0;
    if (r_u32(r_u32(path + 36) + 40) != 0)
    {
        flags = r_u32(object_offset + 228);
        SamplePathPosition(path, 0, target, (flags & 0x400) == 0 ? 64 : 0);
    }
    else
    {
        x = r_u32(data + 88);
        y = r_u32(data + 92);
        z = r_u32(data + 96);
        w_u32(target, x);
        w_u32(target + 4, y);
        w_u32(target + 8, z);
    }
    value = r_u32(object_offset + 132);
    if (value != 0 && r_u32(value + 40) != 0)
    {
        x = r_u32(object_offset + 16);
        y = r_u32(object_offset + 20);
        z = r_u32(object_offset + 24);
        w_u32(first, x);
        w_u32(first + 4, y);
        w_u32(first + 8, z);
        PushMatrix();
        matrix = r_u32(r_u32(object_offset + 132) + 40);
        x = r_u32(matrix);
        y = r_u32(matrix + 4);
        xport_gte_write_control(0, x);
        xport_gte_write_control(1, y);
        x = r_u32(matrix + 8);
        y = r_u32(matrix + 12);
        z = r_u32(matrix + 16);
        xport_gte_write_control(2, x);
        xport_gte_write_control(3, y);
        xport_gte_write_control(4, z);
        p2_sdk_8007ec64(first, current);
        PopMatrix();
        matrix = r_u32(r_u32(object_offset + 132) + 40);
        x = r_u32(current);
        x += r_u32(matrix + 20);
        w_u32(current, x);
        matrix = r_u32(r_u32(object_offset + 132) + 40);
        y = r_u32(current + 4);
        y += r_u32(matrix + 24);
        w_u32(current + 4, y);
        matrix = r_u32(r_u32(object_offset + 132) + 40);
        z = r_u32(current + 8);
        z += r_u32(matrix + 28);
        w_u32(current + 8, z);
    }
    else
    {
        x = r_u32(object_offset + 16);
        y = r_u32(object_offset + 20);
        z = r_u32(object_offset + 24);
        w_u32(current, x);
        w_u32(current + 4, y);
        w_u32(current + 8, z);
    }
    y = r_u32(target + 4);
    y -= r_u32(current + 4);
    value = r_u32(0x8008A1C8);
    x = r_u32(current);
    height = y - value;
    x -= r_u32(target);
    z = r_u32(current + 8);
    value = r_u32(target + 8);
    if ((sint32)x < 0)
        x = 0 - x;
    y = r_u32(0x8008A1E0);
    z -= value;
    if ((sint32)z < 0)
        z = 0 - z;
    distance = x + z;
    if ((sint32)y < (sint32)distance)
    {
        result = 0xFFFFFFFF;
        goto done;
    }
    if ((r_u32(object_offset + 228) & 1) == 0)
    {
        if (character_controller.motion.vertical_speed < 0 && r_s32(0x8008A1E8) < (sint32)height)
        {
            result = 0xFFFFFFFF;
            goto done;
        }
        if ((r_u32(object_offset + 228) & 1) != 0 && r_s32(0x8008A1E8) < (sint32)height)
        {
            result = 0xFFFFFFFF;
            goto done;
        }
    }
    else if (r_s32(0x8008A1E8) < (sint32)height)
    {
        result = 0xFFFFFFFF;
        goto done;
    }
    second_frame = 1;
    if ((r_u32(object_offset + 228) & 0x400) != 0)
    {
        first_frame = r_s16(r_u32(path + 36) + 26);
        second_frame = first_frame - 2;
        --first_frame;
    }
    else
        first_frame = 0;
    SamplePathPosition(path, first_frame, first, 64);
    SamplePathPosition(path, second_frame, second, 64);
    x = r_u32(second);
    dx = x - r_u32(first);
    y = r_u32(second + 4);
    y -= r_u32(first + 4);
    z = r_u32(second + 8);
    dz = z - r_u32(first + 8);
    angle = Atan2Angle((sint32)(0 - dz), (sint32)(0 - dx));
    w_u16(character_offset + 392, angle);
    x = r_u32(0x800B14A0);
    y = r_u32(0x800B14A4);
    z = r_u32(0x800B14A8);
    w_u32(character_offset + 396, x);
    w_u32(character_offset + 400, y);
    w_u32(character_offset + 404, z);
    value = (uint32)(sint32)r_s16(object_offset + 148);
    difference = value - (uint32)(sint32)(sint16)angle;
    if ((sint32)difference < 0)
        difference = 0 - difference;
    if ((sint16)difference >= 2049)
        difference = 4096 - difference;
    w_u32(character_offset + 280, distance);
    flags = r_u32(object_offset + 228);
    if ((flags & 1) != 0)
    {
        if (r_s32(0x8008A1EC) >= (sint32)height)
            result = 1;
        goto done;
    }
    if (character_controller.motion.vertical_speed > 0)
        goto done;
    if ((flags & 0x400) != 0)
    {
        if ((r_u32(object_offset + 28) & 0x10) != 0)
            goto done;
    }
    else if ((r_u32(object_offset + 28) & 0x10) == 0)
        goto done;
    if ((sint16)difference >= r_s16(0x8008A1DA))
        goto done;
    if ((sint32)height < r_s32(0x8008A1E4) || r_s32(0x8008A1E8) < (sint32)height)
        goto done;
    if ((r_u32(object_offset + 228) & 0x400) != 0)
    {
        if ((r_u32(input_offset) & 0x2000) == 0)
            result = 1;
    }
    else if ((r_u32(input_offset) & 0x8000) == 0)
        result = 1;
done:
    p2_guest_stack_pointer = saved_stack;
    return (sint32)result;
}

uint32 p2_func_80058d08(uint32 object_offset, uint32 character_offset, uint32 input_flags, uint32 mode, uint32 horizontal_offset, uint32 vertical_offset)
{
    FUNCTION_MARKER(0x80058D08, "SLUS_005.78");
    uint32 flags, input, result, mask, animation;
    sint32 state, count, threshold;
    flags = r_u32(object_offset + 228);
    input = gameplay_input.buttons;
    if ((flags & 0x200000) != 0 && (character_controller.secondary_flags & 0x80) == 0)
        character_controller.motion.state_code = 3;
    flags = r_u32(object_offset + 228);
    if ((flags & 0x40000) != 0)
    {
        if (r_s8(character_offset + 252) != 6)
            return 6;
        if (r_s16(object_offset + 130) != 0)
            return 7;
        w_u8(character_offset + 252, 7);
        if ((character_controller.secondary_flags & 0x80) == 0)
            character_controller.motion.state_code = 3;
        result = r_u32(object_offset + 228) | 0x200000;
        w_u32(object_offset + 228, result);
        return result;
    }
    if ((flags & 0x200000) != 0)
        return 1;
    state = r_s8(character_offset + 252);
    if (state == 1)
    {
        character_controller.motion.vertical_speed = 0;
        w_u32(vertical_offset, 0);
        if ((character_controller.secondary_flags & 0x240) != 0)
            w_u8(character_offset + 284, 0);
        count = r_s8(character_offset + 285);
        threshold = r_s8(0x8008A1D8);
        if (count < threshold)
        {
            w_u8(character_offset + 285, count + 1);
            mask = (r_u32(object_offset + 228) & 0x400) != 0 ? 0x8000 : 0x2000;
            if ((input & mask) == 0)
                w_u8(character_offset + 284, 1);
        }
        else if (count == threshold)
        {
            if (r_s8(character_offset + 284) == 0)
                w_u8(character_offset + 252, 2);
            else
            {
                mask = (r_u32(object_offset + 228) & 0x400) != 0 ? 0x8000 : 0x2000;
                if ((input & mask) != 0)
                    w_u8(character_offset + 252, 2);
            }
        }
        state = r_s8(character_offset + 252);
    }
    if (state == 2)
    {
        character_controller.motion.vertical_speed = 0;
        w_u32(vertical_offset, 0);
        w_u8(character_offset + 252, 3);
    }
    else if (state >= 3)
    {
        if (state == 3)
        {
            if ((sint8)character_controller.motion.state_code == 20)
            {
                animation = r_u32(object_offset + 104);
                count = r_s16(animation + 6);
                threshold = r_s16(object_offset + 130);
                if (threshold != count - 1)
                {
                    w_u32(vertical_offset, 0);
                    character_controller.motion.vertical_speed = 0;
                    goto animation;
                }
            }
            w_u32(horizontal_offset, 0);
            character_controller.motion.horizontal_speed = 0;
            w_u32(vertical_offset, 0);
            character_controller.motion.vertical_speed = 0;
        }
        w_u8(character_offset + 252, 4);
    }
    else if (state < 0)
        w_u8(character_offset + 252, 4);
animation:
    if ((character_controller.secondary_flags & 0x80) == 0)
        character_controller.motion.state_code = 20;
    return 20;
}

void p2_func_80059ae4(uint32 character_offset)
{
    FUNCTION_MARKER(0x80059AE4, "SLUS_005.78");
}

uint32 p2_func_80056774(uint32 object_offset)
{
    FUNCTION_MARKER(0x80056774, "SLUS_005.78");
    uint32 result, trigger;
    w_u32(object_offset + 228, r_u32(object_offset + 228) & 0xFFFFFC7F);
    result = character_controller.flags;
    trigger = r_u32(0x800ABF5C);
    result &= 0xF7FFFFFF;
    character_controller.flags = result;
    if (trigger != 0)
        result = p2_func_800567d0(object_offset);
    return result;
}

uint32 p2_func_800567fc(uint32 object_offset)
{
    FUNCTION_MARKER(0x800567FC, "SLUS_005.78");
    uint32 result;
    result = r_u32(object_offset + 228) & 0xFFFFFC37;
    w_u32(object_offset + 228, result);
    return result;
}

uint32 p2_func_80059aec(uint32 destination_offset, uint32 object_offset)
{
    FUNCTION_MARKER(0x80059AEC, "SLUS_005.78");
    uint32 saved_stack, local, world, operation, value, counter, state, parent, matrix;
    uint32 x, y, z, previous_x, previous_y, previous_z, dx, dy, dz, enabled;
    sint32 duration, threshold;
    saved_stack = p2_guest_stack_pointer;
    p2_guest_stack_pointer -= 32;
    local = p2_guest_stack_pointer;
    world = local + 16;
    operation = 0;
    if ((r_u32(object_offset + 228) & 0x40000) != 0)
    {
        if ((sint8)character_controller.motion.state_code == 20)
        {
            duration = r_s16(object_offset + 130) + 1;
            operation = 3;
        }
        else if ((r_u32(object_offset + 28) & 0x2000) != 0)
        {
            x = r_u32(object_offset + 16);
            y = r_u32(object_offset + 20);
            z = r_u32(object_offset + 24);
            goto publish;
        }
    }
    else
    {
        counter = r_u8(0x800ABE8C);
        threshold = r_s8(0x8008A1BF);
        if (counter < (uint32)threshold)
        {
            enabled = 0;
            if (r_s8(0x8008A1C0) == 0)
                enabled = 1;
            else if ((sint8)character_controller.motion.state_code == 20 && r_s16(object_offset + 130) != 0)
            {
                value = r_u32(object_offset + 104);
                counter = r_u8(0x8008A1B8);
                value = r_u8(value + 6);
                w_u8(0x8008A1BF, value - counter);
                enabled = 1;
            }
            if (enabled != 0)
            {
                threshold = r_s8(0x8008A1BF);
                counter = r_u8(0x800ABE8C);
                previous_x = r_u32(0x800ABEC0);
                previous_y = r_u32(0x800ABEC4);
                previous_z = r_u32(0x800ABEC8);
                duration = threshold - (sint32)counter + 1;
                if (duration <= 0)
                    duration = 1;
                ++counter;
                w_u8(0x800ABE8C, counter);
                if ((counter & 0xFF) != 0)
                {
                    state = r_u32(0x8008A114);
                    previous_x = r_u32(state + 16);
                    previous_y = r_u32(state + 20);
                    previous_z = r_u32(state + 24);
                }
                operation = 1;
            }
            else
            {
                state = r_u32(0x8008A114);
                value = r_u32(state + 136);
                value = r_u32(value + 36);
                if (r_u32(value + 40) == 0)
                {
                    x = r_u32(0x800ABECC);
                    y = r_u32(0x800ABED0);
                    z = r_u32(0x800ABED4);
                    goto publish;
                }
                operation = 2;
            }
        }
    }
    parent = r_u32(object_offset + 132);
    if (parent != 0 && r_u32(parent + 40) != 0)
    {
        x = r_u32(object_offset + 16);
        y = r_u32(object_offset + 20);
        z = r_u32(object_offset + 24);
        w_u32(local, x);
        w_u32(local + 4, y);
        w_u32(local + 8, z);
        PushMatrix();
        matrix = r_u32(r_u32(object_offset + 132) + 40);
        x = r_u32(matrix);
        y = r_u32(matrix + 4);
        xport_gte_write_control(0, x);
        xport_gte_write_control(1, y);
        x = r_u32(matrix + 8);
        y = r_u32(matrix + 12);
        z = r_u32(matrix + 16);
        xport_gte_write_control(2, x);
        xport_gte_write_control(3, y);
        xport_gte_write_control(4, z);
        p2_sdk_8007ec64(local, world);
        PopMatrix();
        matrix = r_u32(r_u32(object_offset + 132) + 40);
        x = r_u32(world);
        x += r_u32(matrix + 20);
        w_u32(world, x);
        matrix = r_u32(r_u32(object_offset + 132) + 40);
        y = r_u32(world + 4);
        y += r_u32(matrix + 24);
        w_u32(world + 4, y);
        matrix = r_u32(r_u32(object_offset + 132) + 40);
        z = r_u32(world + 8);
        z += r_u32(matrix + 28);
        w_u32(world + 8, z);
    }
    else
    {
        x = r_u32(object_offset + 16);
        y = r_u32(object_offset + 20);
        z = r_u32(object_offset + 24);
    }
    if (operation == 1)
    {
        dx = x - previous_x;
        dy = y - previous_y;
        dz = z - previous_z;
        x = previous_x + (uint32)((sint32)dx / duration);
        y = previous_y + (uint32)((sint32)dy / duration);
        z = previous_z + (uint32)((sint32)dz / duration);
    }
    else if (operation == 2)
    {
        dx = x - r_u32(0x800ABE9C);
        dy = y - r_u32(0x800ABEA0);
        dz = z - r_u32(0x800ABEA4);
        x = r_u32(0x800ABED8);
        y = r_u32(0x800ABEDC);
        z = r_u32(0x800ABEE0);
        x += dx;
        y += dy;
        z += dz;
    }
    else if (operation == 3)
    {
        if (duration == 0)
            xport_mips_break(7);
        y += (uint32)(-32768 / duration);
    }
publish:
    w_u32(0x800ABECC, x);
    w_u32(0x800ABED0, y);
    w_u32(0x800ABED4, z);
    w_u32(destination_offset, x);
    w_u32(destination_offset + 4, y);
    w_u32(destination_offset + 8, z);
    p2_guest_stack_pointer = saved_stack;
    return destination_offset;
}

uint32 p2_func_80057208(uint32 object_offset, uint32 character_offset)
{
    FUNCTION_MARKER(0x80057208, "SLUS_005.78");
    uint32 flags, state, definition, matrix, x, y, z, saved_stack, position, floor, local, value;
    flags = r_u32(object_offset + 228);
    state = r_u32(object_offset + 28);
    w_u32(object_offset + 228, flags & 0xFFFFFC37);
    w_u32(object_offset + 28, state | 0x400);
    if ((sint8)character_controller.motion.state_code == 20 && r_s16(object_offset + 130) == 0)
    {
        w_u32(object_offset + 228, r_u32(object_offset + 228) | 0x400000);
        w_u8(character_offset + 252, 10);
    }
    else
    {
        ReverseScenePath(object_offset, character_offset);
        saved_stack = p2_guest_stack_pointer;
        p2_guest_stack_pointer -= 48;
        position = p2_guest_stack_pointer;
        floor = position + 16;
        local = position + 32;
        state = r_u32(0x8008A114);
        definition = r_u32(r_u32(state + 136) + 36);
        if (definition != 0 && r_u32(definition + 40) != 0)
        {
            x = r_u32(object_offset + 16);
            y = r_u32(object_offset + 20);
            z = r_u32(object_offset + 24);
            w_u32(local, x);
            w_u32(local + 4, y);
            w_u32(local + 8, z);
            PushMatrix();
            state = r_u32(0x8008A114);
            matrix = r_u32(r_u32(r_u32(state + 136) + 36) + 40);
            x = r_u32(matrix);
            y = r_u32(matrix + 4);
            xport_gte_write_control(0, x);
            xport_gte_write_control(1, y);
            x = r_u32(matrix + 8);
            y = r_u32(matrix + 12);
            z = r_u32(matrix + 16);
            xport_gte_write_control(2, x);
            xport_gte_write_control(3, y);
            xport_gte_write_control(4, z);
            p2_sdk_8007ec64(local, position);
            PopMatrix();
            state = r_u32(0x8008A114);
            matrix = r_u32(r_u32(r_u32(state + 136) + 36) + 40);
            x = r_u32(position);
            x += r_u32(matrix + 20);
            w_u32(position, x);
            matrix = r_u32(r_u32(r_u32(state + 136) + 36) + 40);
            y = r_u32(position + 4);
            y += r_u32(matrix + 24);
            w_u32(position + 4, y);
            matrix = r_u32(r_u32(r_u32(state + 136) + 36) + 40);
            z = r_u32(position + 8);
            z += r_u32(matrix + 28);
            w_u32(position + 8, z);
        }
        else
        {
            x = r_u32(object_offset + 16);
            y = r_u32(object_offset + 20);
            z = r_u32(object_offset + 24);
            w_u32(position, x);
            w_u32(position + 4, y);
            w_u32(position + 8, z);
        }
        x = r_u32(position);
        y = r_u32(position + 4);
        z = r_u32(position + 8);
        w_u32(floor, x);
        w_u32(floor + 4, y);
        w_u32(floor + 8, z);
        w_u32(position + 4, r_u32(position + 4) - 0x4000);
        flags = r_u32(object_offset + 228);
        state = r_u32(0x8008A114);
        w_u32(object_offset + 228, flags | 0x02200000);
        ChangeActorPath(object_offset, r_u32(state + 136), position, floor);
        state = r_u32(0x8008A114);
        value = r_u32(state + 136);
        if (value == r_u32(character_offset + 408))
            w_u32(object_offset + 28, r_u32(object_offset + 28) | 0x2000);
        p2_guest_stack_pointer = saved_stack;
    }
    value = r_u8(0x8008A1BB);
    w_u8(character_offset + 294, value);
    return value;
}







sint32 p2_func_80083310(uint32 first_offset, uint32 second_offset, sint32 count)
{
    FUNCTION_MARKER(0x80083310, "SLUS_005.78");
    uint32 first, second;
    for (;;)
    {
        first = r_u8(first_offset);
        second = r_u8(second_offset);
        ++first_offset;
        if (first != second)
        {
            first = r_u8(first_offset - 1);
            second = r_u8(second_offset);
            return (sint32)(first - second);
        }
        count = (sint32)((uint32)count - 1);
        ++second_offset;
        if (count <= 0)
            return 0;
    }
}
