#include <stdlib.h>
#include <stdio.h>

#include "game.h"



uint32 p2_func_80053bf0(void)
{
    CharacterModelRecord *model_record;
    FUNCTION_MARKER(0x80053BF0, "SLUS_005.78");
    uint32 object = r_u32(0x8008A340);
    uint32 model, action;
    if (object == 0)
    {
        model = character_controller.model_slots[(((character_controller.motion.form_flags >> 18) & 60)) / 4];
        model_record = (CharacterModelRecord *)psx_addr(model, 240);
        object = model_record->object_offset;
        w_u8(object + 126, 1);
        w_u8(object + 125, 1);
        w_u8(object + 127, 0);
        w_u32(object + 220, (uint32)-1);
        w_u32(object + 204, (uint32)-1);
        w_u32(object + 188, (uint32)-1);
        w_u32(object + 172, (uint32)-1);
    }
    model = character_controller.model_slots[(((character_controller.motion.form_flags >> 18) & 60)) / 4];
    model_record = (CharacterModelRecord *)psx_addr(model, 240);
    action = model_record->animation_offsets[2];
    if (r_u32(object + 104) != action)
    {
        uint32 flags = r_u32(object + 28);
        w_u16(object + 130, (uint32)-1);
        w_u32(object + 104, action);
        w_u32(object + 28, flags | 0x10000);
    }
    return object;
}



uint32 p2_func_80066168(void)
{
    FUNCTION_MARKER(0x80066168, "SLUS_005.78");
    uint32 image, index;
    BeginDraw();
    ClearSceneOrderingTable();
    p2_func_80066bdc(0, 0xF00140, 0);
    image = resource_groups[2].material_offset;
    for (index = 0; index < 2; ++index)
    {
        sint32 width = (sint32)r_u8(image + 6) - r_u8(image + 4);
        sint32 height = (sint32)r_u8(image + 7) - r_u8(image + 5);
        uint32 x = (uint32)((319 - width) >> 1);
        uint32 y = (uint32)((239 - height) >> 1);
        DrawTexturedQuad(x | (y << 16), 0, image, 0);
    }
    return EndDraw();
}

uint32 p2_func_8006e098(sint32 percent, uint32 flags, sint32 next_level, uint32 camera_offset, sint8 mode)
{
    MaterialResourceHeader *resources;
    FUNCTION_MARKER(0x8006E098, "SLUS_005.78");
    char text[40];
    uint32 value, result;
    BeginDraw();
    p2_func_8006de08(camera_offset);
    ClearSceneOrderingTable();
    p2_func_80066bdc(0, 0xF00140, flags);
    if (flags == 0x800000)
        DrawText(0x6E00A0, UiText::BossDefeated, 1, 0);
    else
    {
        if (flags == 0x8000)
        {
            sprintf(text, mode != 0 ? UiText::BonusOver : UiText::BonusComplete);
            DrawText(0x6E00A0, text, 1, 0);
        }
        else
            DrawText(0x5000A0, UiText::LevelComplete, 1, 0);
        sprintf(text, UiText::TreasureFormat, percent);
        DrawText(0x8200A0, text, 1, 0);
    }
    if (r_s32(camera_offset + 12) < 20481)
    {
        DrawTexturedQuad(0xC80037, 0, interface_resources->textures_offset + 160, 0);
        if (next_level == 96)
            DrawText(0xCA005A, UiText::BoarderRun, 0, 0);
        else
        {
            DrawText(0x96001A, UiText::Password, 0, 0);
            DrawPassword(140, 148, 0);
            DrawText(0xCA005A, UiText::Continue, 0, 0);
        }
    }
    p2_func_80027228();
    resources = menu_state.resources;
    if (resources != 0)
    {
        uint32 tick = frame_clock.render_tick;
        sint32 count = (sint32)resources->texture_count;
        sint32 index = 0;
        frame_clock.render_tick = frame_clock.menu_tick;
        while (index < count)
        {
            UpdateMaterialAnimationList(menu_state.material_groups[index].animation_entries);
            count = (sint32)menu_state.resources->texture_count;
            ++index;
        }
        frame_clock.render_tick = tick;
    }
    value = menu_state.fade_frames;
    if (value != 0)
    {
        --value;
        menu_state.fade_frames = value;
        if (value != 0)
            p2_func_800651bc(0, 0xF00140, (sint32)(value << 12) / 15, 0xFFFFFF);
    }
    EndDraw();
    result = menu_state.pacer.Wait(menu_state.last_frame);
    menu_state.last_frame = result;
    return result;
}



uint32 p2_func_80059284(uint32 owner_offset, uint32 object_offset)
{
    FUNCTION_MARKER(0x80059284, "SLUS_005.78");
    uint32 saved_stack, world, input, inverse;
    uint32 position[3], movement[3];
    uint32 definition, matrix, index, result;
    if (r_s8(0x8008A1AD) == 0)
        return 0;
    if (r_s8(0x8008A1B9) < (sint32)r_u8(0x800ABE8C))
        return 1;
    w_u8(0x800ABF14, 1);
    saved_stack = p2_guest_stack_pointer;
    p2_guest_stack_pointer -= 64;
    world = p2_guest_stack_pointer;
    input = world + 16;
    inverse = world + 32;
    definition = r_u32(object_offset + 132);
    if (definition != 0 && r_u32(definition + 40) != 0)
    {
        for (index = 0; index < 3; ++index)
            position[index] = r_u32(object_offset + 16 + index * 4);
        for (index = 0; index < 3; ++index)
            w_u32(input + index * 4, position[index]);
        PushMatrix();
        matrix = r_u32(r_u32(object_offset + 132) + 40);
        for (index = 0; index < 5; ++index)
            xport_gte_write_control(index, r_u32(matrix + index * 4));
        p2_sdk_8007ec64(input, world);
        PopMatrix();
        for (index = 0; index < 3; ++index)
        {
            matrix = r_u32(r_u32(object_offset + 132) + 40);
            w_u32(world + index * 4, r_u32(world + index * 4) + r_u32(matrix + 20 + index * 4));
        }
    }
    else
    {
        for (index = 0; index < 3; ++index)
            position[index] = r_u32(object_offset + 16 + index * 4);
        for (index = 0; index < 3; ++index)
            w_u32(world + index * 4, position[index]);
    }
    for (index = 0; index < 3; ++index)
        movement[index] = r_u32(0x800ABECC + index * 4) - r_u32(world + index * 4);
    movement[1] += r_u32(0x8008A1B0);
    if (r_s8(0x8008A1BA) >= (sint32)r_u8(0x800ABE8C))
    {
        uint32 heading = r_u16(0x800ABEF0);
        if (heading - 512u >= 0xC01)
            movement[2] -= r_u32(0x8008A1B4);
        else if ((sint16)heading < 1536)
            movement[0] += r_u32(0x8008A1B4);
        else if ((sint16)heading < 2560)
            movement[2] += r_u32(0x8008A1B4);
        else
            movement[0] -= r_u32(0x8008A1B4);
    }
    for (index = 0; index < 3; ++index)
        w_u32(world + index * 4, r_u32(world + index * 4) + movement[index]);
    for (index = 0; index < 3; ++index)
        w_u32(0x800ABF08 + index * 4, movement[index]);
    definition = r_u32(object_offset + 132);
    if (definition != 0 && r_u32(definition + 40) != 0)
    {
        for (index = 0; index < 3; ++index)
        {
            matrix = r_u32(r_u32(object_offset + 132) + 40);
            w_u32(input + index * 4, r_u32(world + index * 4) - r_u32(matrix + 20 + index * 4));
        }
        for (index = 0; index < 9; ++index)
        {
            matrix = r_u32(r_u32(object_offset + 132) + 40);
            w_u16(inverse + index * 2, r_u16(matrix + (index % 3) * 6 + (index / 3) * 2));
        }
        PushMatrix();
        for (index = 0; index < 5; ++index)
            xport_gte_write_control(index, r_u32(inverse + index * 4));
        result = p2_sdk_8007ec64(input, object_offset + 16);
        PopMatrix();
    }
    else
    {
        for (index = 0; index < 3; ++index)
            position[index] = r_u32(world + index * 4);
        for (index = 0; index < 3; ++index)
            w_u32(object_offset + 16 + index * 4, position[index]);
        result = position[0];
    }
    p2_guest_stack_pointer = saved_stack;
    return result;
}

uint32 p2_func_800596cc(uint32 owner_offset, uint32 object_offset)
{
    FUNCTION_MARKER(0x800596CC, "SLUS_005.78");
    uint32 saved_stack, world, input, inverse;
    uint32 position[3];
    uint32 definition, matrix, index, result, amplitude, step_x, step_z;
    sint16 heading;
    sint32 state;
    if (r_s8(0x8008A1AD) == 0)
        return 0;
    w_u8(0x800ABF14, 2);
    saved_stack = p2_guest_stack_pointer;
    p2_guest_stack_pointer -= 64;
    world = p2_guest_stack_pointer;
    input = world + 16;
    inverse = world + 32;
    definition = r_u32(object_offset + 132);
    if (definition != 0 && r_u32(definition + 40) != 0)
    {
        for (index = 0; index < 3; ++index)
            position[index] = r_u32(object_offset + 16 + index * 4);
        for (index = 0; index < 3; ++index)
            w_u32(input + index * 4, position[index]);
        PushMatrix();
        matrix = r_u32(r_u32(object_offset + 132) + 40);
        for (index = 0; index < 5; ++index)
            xport_gte_write_control(index, r_u32(matrix + index * 4));
        p2_sdk_8007ec64(input, world);
        PopMatrix();
        for (index = 0; index < 3; ++index)
        {
            matrix = r_u32(r_u32(object_offset + 132) + 40);
            w_u32(world + index * 4, r_u32(world + index * 4) + r_u32(matrix + 20 + index * 4));
        }
    }
    else
    {
        for (index = 0; index < 3; ++index)
            position[index] = r_u32(object_offset + 16 + index * 4);
        for (index = 0; index < 3; ++index)
            w_u32(world + index * 4, position[index]);
    }
    state = (sint8)character_controller.motion.state_code;
    amplitude = state == 11 ? 4096 : (state == 10 ? 2048 : 3072);
    heading = r_s16(owner_offset + 148);
    step_x = (uint32)(((sint64)(SinQ14(heading) >> 2) * amplitude) >> 12);
    step_z = (uint32)(((sint64)(SinQ14((sint16)(1024 - heading)) >> 2) * amplitude) >> 12);
    position[0] = r_u32(world) + step_x;
    position[1] = r_u32(world + 4);
    position[2] = r_u32(world + 8) + step_z;
    for (index = 0; index < 3; ++index)
        w_u32(world + index * 4, position[index]);
    definition = r_u32(object_offset + 132);
    if (definition != 0 && r_u32(definition + 40) != 0)
    {
        for (index = 0; index < 3; ++index)
        {
            matrix = r_u32(r_u32(object_offset + 132) + 40);
            w_u32(input + index * 4, position[index] - r_u32(matrix + 20 + index * 4));
        }
        for (index = 0; index < 9; ++index)
        {
            matrix = r_u32(r_u32(object_offset + 132) + 40);
            w_u16(inverse + index * 2, r_u16(matrix + (index % 3) * 6 + (index / 3) * 2));
        }
        PushMatrix();
        for (index = 0; index < 5; ++index)
            xport_gte_write_control(index, r_u32(inverse + index * 4));
        result = p2_sdk_8007ec64(input, object_offset + 16);
        PopMatrix();
    }
    else
    {
        for (index = 0; index < 3; ++index)
            w_u32(object_offset + 16 + index * 4, position[index]);
        result = position[0];
    }
    p2_guest_stack_pointer = saved_stack;
    return result;
}





uint32 p2_func_800378c8(uint32 object_offset, uint32 distance, uint32 distance_offset, uint32 height_offset, uint32 flags_offset)
{
    FUNCTION_MARKER(0x800378C8, "SLUS_005.78");
    uint32 data = r_u32(r_u32(object_offset + 132) + 44);
    uint32 owner = GetCharacterModelObject();
    uint32 target = r_u32(0x8008A114);
    uint32 target_position[3], position[3], difference[3];
    uint32 definition, matrix, index, result, angle = 0;
    uint32 horizontal = distance, vertical = 0;
    for (index = 0; index < 3; ++index)
        target_position[index] = r_u32(target + 16 + index * 4);
    definition = r_u32(object_offset + 132);
    if (definition != 0 && r_u32(definition + 40) != 0)
    {
        uint32 saved_stack = p2_guest_stack_pointer;
        uint32 input, output;
        p2_guest_stack_pointer -= 32;
        input = p2_guest_stack_pointer;
        output = input + 16;
        for (index = 0; index < 3; ++index)
            position[index] = r_u32(object_offset + 16 + index * 4);
        for (index = 0; index < 3; ++index)
            w_u32(input + index * 4, position[index]);
        PushMatrix();
        matrix = r_u32(r_u32(object_offset + 132) + 40);
        for (index = 0; index < 5; ++index)
            xport_gte_write_control(index, r_u32(matrix + index * 4));
        p2_sdk_8007ec64(input, output);
        PopMatrix();
        for (index = 0; index < 3; ++index)
        {
            matrix = r_u32(r_u32(object_offset + 132) + 40);
            position[index] = r_u32(output + index * 4) + r_u32(matrix + 20 + index * 4);
            w_u32(output + index * 4, position[index]);
        }
        p2_guest_stack_pointer = saved_stack;
    }
    else
    {
        for (index = 0; index < 3; ++index)
            position[index] = r_u32(object_offset + 16 + index * 4);
    }
    for (index = 0; index < 3; ++index)
        difference[index] = position[index] - target_position[index];
    if ((r_u16(data + 122) & 0x400) == 0)
    {
        uint32 threshold = r_u32(0x8008A160);
        if ((sint32)difference[1] < (sint32)(0u - threshold))
            vertical = distance;
        result = (sint32)threshold < (sint32)difference[1];
        if (result != 0)
            vertical = 0u - distance;
    }
    else if (r_s32(object_offset + 20) < (sint32)target_position[1] && (sint32)difference[1] < r_s32(0x8008A164))
    {
        result = 0xFFFF0000;
        if (r_s32(owner + 96) < 0)
            vertical = 0u - distance;
        else
        {
            result = (sint32)difference[1] < -40960;
            if (result != 0)
                vertical = 0u - distance;
        }
    }
    else
    {
        result = (sint32)(0u - r_u32(0x8008A164)) < (sint32)difference[1];
        if (result != 0)
        {
            sint32 velocity = r_s32(owner + 96);
            result = 40960;
            if (velocity >= 0)
                vertical = distance;
            else
            {
                result = (sint32)difference[1] > 40960;
                if (result != 0)
                    vertical = distance;
            }
        }
    }
    if (vertical != 0)
    {
        uint32 height = difference[1];
        uint32 reach;
        sint32 wave;
        sint64 product;
        uint32 flags = r_u16(data + 120);
        w_u16(data + 176, 0);
        w_u16(data + 120, flags | 0x800);
        if ((sint32)height < 0)
            height = 0u - height;
        if ((sint32)difference[2] < (sint32)difference[0])
            reach = difference[0] + (uint32)((sint32)difference[2] / 2);
        else
            reach = (uint32)((sint32)difference[0] / 2) + difference[2];
        angle = (uint32)Atan2Angle((sint32)reach, (sint32)height);
        if ((sint32)vertical < 0)
            angle = 0u - angle;
        wave = SinQ14((sint16)angle) >> 2;
        vertical = (uint32)(((sint64)(sint32)distance * wave) >> 12);
        wave = SinQ14((sint16)(1024u - angle)) >> 2;
        product = (sint64)(sint32)distance * wave;
        horizontal = (uint32)(product >> 12);
        result = (uint32)((uint64)product >> 32) << 20;
    }
    w_u32(height_offset, vertical);
    w_u32(distance_offset, horizontal);
    w_u16(flags_offset, angle);
    return result;
}

uint32 p2_func_80037c50(uint32 object_offset, uint32 distance, uint32 distance_offset, uint32 height_offset, uint32 flags_offset)
{
    FUNCTION_MARKER(0x80037C50, "SLUS_005.78");
    uint32 definition = r_u32(object_offset + 132);
    uint32 data = r_u32(definition + 44);
    uint32 vertical = 0, result;
    if (r_s8(0x8008A16C) == 0)
    {
        result = r_u16(data + 120) & 0xF7FF;
        w_u16(data + 120, result);
    }
    else
    {
        uint32 saved_stack = p2_guest_stack_pointer;
        uint32 position, floor, input, index, matrix;
        p2_guest_stack_pointer -= 48;
        position = p2_guest_stack_pointer;
        floor = position + 16;
        input = position + 32;
        if (definition != 0 && r_u32(definition + 40) != 0)
        {
            uint32 local_position[3];
            for (index = 0; index < 3; ++index)
                local_position[index] = r_u32(object_offset + 16 + index * 4);
            for (index = 0; index < 3; ++index)
                w_u32(input + index * 4, local_position[index]);
            PushMatrix();
            matrix = r_u32(r_u32(object_offset + 132) + 40);
            for (index = 0; index < 5; ++index)
                xport_gte_write_control(index, r_u32(matrix + index * 4));
            p2_sdk_8007ec64(input, position);
            PopMatrix();
            for (index = 0; index < 3; ++index)
            {
                matrix = r_u32(r_u32(object_offset + 132) + 40);
                w_u32(position + index * 4, r_u32(position + index * 4) + r_u32(matrix + 20 + index * 4));
            }
        }
        else
        {
            uint32 local_position[3];
            for (index = 0; index < 3; ++index)
                local_position[index] = r_u32(object_offset + 16 + index * 4);
            for (index = 0; index < 3; ++index)
                w_u32(position + index * 4, local_position[index]);
        }
        result = FindPathHeightFrame(r_u32(object_offset + 132), position, r_u32(object_offset + 28) & 16, 4096000);
        if (result != (uint32)-1)
        {
            uint32 direction = r_u32(object_offset + 28) & 16;
            uint32 position_y, floor_y, difference;
            result = (uint32)ProjectPathFrame(r_u32(object_offset + 132), (sint32)result, position, floor, direction, 4096);
            floor_y = r_u32(floor + 4);
            position_y = r_u32(position + 4);
            difference = floor_y - position_y;
            if (result != (uint32)-1)
            {
                uint32 tolerance = r_u32(0x8008A168);
                if (((sint32)difference < (sint32)tolerance && (sint32)(0u - tolerance) < (sint32)difference) || r_s16(data + 178) >= 5)
                {
                    result = r_u16(data + 120) & 0xF7FF;
                    w_u16(data + 120, result);
                }
                else
                {
                    uint32 flags = r_u16(data + 120);
                    result = flags & 32;
                    if ((sint32)position_y < (sint32)floor_y)
                    {
                        vertical = distance;
                        if (result == 0)
                        {
                            result = r_u16(data + 178);
                            w_u16(data + 120, flags | 32);
                            ++result;
                            w_u16(data + 178, result);
                        }
                        else
                            w_u16(data + 178, 0);
                    }
                    else
                    {
                        vertical = 0u - distance;
                        if (result != 0)
                        {
                            result = r_u16(data + 178);
                            w_u16(data + 120, flags & 0xFFDF);
                            ++result;
                            w_u16(data + 178, result);
                        }
                        else
                            w_u16(data + 178, 0);
                    }
                }
            }
        }
        p2_guest_stack_pointer = saved_stack;
    }
    w_u32(height_offset, vertical);
    w_u32(distance_offset, distance);
    w_u16(flags_offset, 0);
    return result;
}

uint32 p2_func_80044cfc(uint32 object_offset, uint32 speed_offset)
{
    FUNCTION_MARKER(0x80044CFC, "SLUS_005.78");
    uint32 flags, input, speed, value;
    sint32 delta = 0;
    p2_func_800536f0(0);
    if (((uint16)(character_controller.motion.form_flags >> 16) & 15) != 0)
        return 0x20000;
    flags = r_u32(object_offset + 28);
    if ((flags & 0x20200) == 512 || (sint8)character_controller.motion.state_code == 4)
        return 4;
    if ((r_u32(0x8008A498) & 16) == 0)
        return 0;
    input = gameplay_input.buttons;
    if ((input & 0x8000) != 0)
    {
        delta = (flags & 16) != 0 ? -819 : 819;
        gameplay_input.buttons = input & ~0x8000u;
    }
    else if ((input & 0x2000) != 0)
    {
        delta = (flags & 16) != 0 ? 819 : -819;
        gameplay_input.buttons = input & ~0x2000u;
    }
    speed = r_u32(speed_offset) + (uint32)delta;
    w_u32(speed_offset, speed);
    if ((sint32)speed >= 12289)
        w_u32(speed_offset, 12288);
    if (delta != 0)
    {
        if (r_s32(speed_offset) < 0)
        {
            uint32 definition;
            sint32 count, frame;
            w_u32(speed_offset, 0u - r_u32(speed_offset));
            flags = r_u32(object_offset + 28);
            definition = r_u32(object_offset + 132);
            w_u32(object_offset + 28, flags ^ 16);
            count = r_s16(definition + 26);
            frame = count - r_s16(object_offset + 136);
            if (frame >= count)
                frame = count - 1;
            else if (frame < 0)
                frame = 0;
            w_u16(object_offset + 136, frame);
        }
        flags = r_u32(object_offset + 28);
        if ((flags & 0x2000) != 0)
        {
            uint32 saved_stack = p2_guest_stack_pointer;
            uint32 vector, index, x;
            p2_guest_stack_pointer -= 16;
            vector = p2_guest_stack_pointer;
            SamplePathTangent(r_u32(object_offset + 132), (VECTOR *)psx_addr(vector, 12), flags & 16);
            w_u32(vector + 4, 0);
            if (r_u32(vector) + 26754u > 0xD103 || r_u32(vector + 8) + 26754u > 0xD103)
            {
                do
                {
                    for (index = 0; index < 3; ++index)
                        w_u32(vector + index * 4, r_s32(vector + index * 4) >> 1);
                } while (r_u32(vector) + 26754u > 0xD103 || r_u32(vector + 4) + 26754u > 0xD103 || r_u32(vector + 8) + 26754u > 0xD103);
            }
            VectorNormal((VECTOR *)psx_addr(vector, 16), (VECTOR *)psx_addr(vector, 16));
            for (index = 0; index < 3; ++index)
                w_u32(vector + index * 4, (uint32)(((sint64)r_s32(vector + index * 4) * r_s32(speed_offset)) >> 12));
            x = r_u32(vector);
            w_u32(object_offset + 92, x);
            w_u32(object_offset + 100, r_u32(vector + 8));
            p2_guest_stack_pointer = saved_stack;
        }
    }
    input = gameplay_input.buttons;
    delta = (input & 0x1000) != 0 ? 819 : ((input & 0x4000) != 0 ? -819 : 0);
    value = r_u32(object_offset + 96) + (uint32)delta;
    w_u32(object_offset + 96, value);
    if ((sint32)value >= 12289)
    {
        w_u32(object_offset + 96, 12288);
        return 12288;
    }
    if ((sint32)value < -12288)
        w_u32(object_offset + 96, (uint32)-12288);
    return (uint32)-12288;
}

uint32 p2_func_80043de4(uint32 object_offset)
{
    FUNCTION_MARKER(0x80043DE4, "SLUS_005.78");
    uint32 count = r_u32(0x8008A338);
    uint32 kind = r_u32(object_offset + 116);
    uint32 table = 0x8008B4A8;
    uint32 selected = 0, definition, data, flags, index, other;
    w_u16(object_offset + 128, 1);
    while (count-- != 0)
    {
        uint32 candidate = r_u32(table);
        table += 4;
        if ((uint32)(sint32)r_s8(candidate + 2) == kind)
        {
            selected = candidate;
            break;
        }
    }
    data = r_u32(r_u32(object_offset + 132) + 44);
    if ((sound_effects.IsPlaying(r_u32(data + 8), (uint32)(sint32)r_s16(data + 6)) & 255) != 0)
        p2_func_8006a970(r_u16(data + 6));
    w_u32(data + 8, 0);
    w_u16(data + 6, (uint32)-1);
    flags = r_u32(object_offset + 28);
    w_u8(object_offset + 124, 0);
    w_u8(object_offset + 127, 0);
    w_u8(object_offset + 126, 4);
    if ((flags & 0x8000) == 0)
    {
        uint32 byte_flags = r_u8(object_offset + 13) & 254;
        uint32 state = r_u32(object_offset + 192);
        uint32 frame;
        flags = r_u32(object_offset + 28) & 0xFFF3BEFF;
        w_u8(object_offset + 13, byte_flags);
        definition = r_u32(object_offset + 132);
        w_u8(object_offset + 13, byte_flags | (state & 1));
        w_u32(object_offset + 28, flags);
        w_u32(object_offset + 28, flags | (state & 0x40000));
        frame = r_u16(r_u32(definition + 44) + 4);
        definition = r_u32(object_offset + 132);
        w_u16(object_offset + 136, frame);
        SampleVectorTrack((sint16)frame, definition + 20, object_offset + 16);
    }
    flags = r_u32(object_offset + 28);
    w_u32(object_offset + 220, (uint32)-1);
    w_u32(object_offset + 204, (uint32)-1);
    w_u32(object_offset + 188, (uint32)-1);
    w_u32(object_offset + 172, (uint32)-1);
    w_u16(object_offset + 148, 4096);
    w_u32(object_offset + 100, 0);
    w_u32(object_offset + 96, 0);
    w_u32(object_offset + 92, 0);
    w_u32(object_offset + 28, flags & 0xFE1F13FF);
    definition = r_u32(object_offset + 132);
    if (definition != 0 && r_u32(definition + 40) != 0)
    {
        uint32 saved_stack = p2_guest_stack_pointer;
        uint32 vector, matrix;
        uint32 position[3];
        p2_guest_stack_pointer -= 16;
        vector = p2_guest_stack_pointer;
        for (index = 0; index < 3; ++index)
            position[index] = r_u32(object_offset + 16 + index * 4);
        for (index = 0; index < 3; ++index)
            w_u32(vector + index * 4, position[index]);
        PushMatrix();
        matrix = r_u32(r_u32(object_offset + 132) + 40);
        for (index = 0; index < 5; ++index)
            xport_gte_write_control(index, r_u32(matrix + index * 4));
        p2_sdk_8007ec64(vector, object_offset + 176);
        PopMatrix();
        for (index = 0; index < 3; ++index)
        {
            matrix = r_u32(r_u32(object_offset + 132) + 40);
            w_u32(object_offset + 176 + index * 4, r_u32(object_offset + 176 + index * 4) + r_u32(matrix + 20 + index * 4));
        }
        p2_guest_stack_pointer = saved_stack;
    }
    else
    {
        uint32 position[3];
        for (index = 0; index < 3; ++index)
            position[index] = r_u32(object_offset + 16 + index * 4);
        for (index = 0; index < 3; ++index)
            w_u32(object_offset + 176 + index * 4, position[index]);
    }
    {
        uint32 position[3];
        for (index = 0; index < 3; ++index)
            position[index] = r_u32(object_offset + 176 + index * 4);
        for (index = 0; index < 3; ++index)
            w_u32(object_offset + 60 + index * 4, position[index]);
    }
    other = r_u32(object_offset + 8);
    flags = r_u32(other + 28);
    w_u8(other + 125, 0);
    w_u32(other + 144, 0);
    w_u32(other + 28, flags | 0x2020020);
    return p2_func_80042320(selected, object_offset);
}

uint32 p2_func_80028720(uint32 object_offset, uint32 owner_offset)
{
    FUNCTION_MARKER(0x80028720, "SLUS_005.78");
    uint32 flags = player_interaction.flags;
    uint32 amplitude = 8192;
    uint32 step[3];
    uint32 position[3] = {0, 0, 0};
    uint32 child;
    uint32 index;
    if ((flags & 32) == 0 || (r_u32(object_offset + 28) & 0x20200) == 512)
    {
        uint8 object_flags;
        player_interaction.flags = flags & ~36u;
        player_interaction.target_offset = 0;
        object_flags = r_u8(object_offset + 13);
        w_u8(object_offset + 125, 0);
        w_u8(object_offset + 13, object_flags | 2);
        return object_flags | 2;
    }
    if ((flags & 8) != 0)
        w_u16(object_offset + 136, 4);
    else
    {
        sint16 countdown = r_s16(object_offset + 136);
        if (countdown != 0)
        {
            countdown = (sint16)(countdown - 1);
            w_u16(object_offset + 136, countdown);
            if (countdown >= 3)
                amplitude = (uint32)((4 - countdown) * 2048 + 8192);
            else if (countdown >= 0)
            {
                amplitude = 12288;
                if (countdown == 0)
                    w_u16(object_offset + 136, (uint32)-1);
            }
            else if (countdown == -5)
                w_u16(object_offset + 136, 0);
            else
                amplitude = (uint32)(((countdown + 6) * 4096) / 5 + 8192);
        }
    }
    for (index = 0; index < 3; ++index)
    {
        uint32 difference = r_u32(owner_offset + 60 + index * 4) - r_u32(object_offset + 16 + index * 4);
        step[index] = (uint32)(((sint64)(sint32)difference * 256) >> 12);
    }
    child = r_u32(object_offset + 8);
    for (index = 0; index < 16; ++index)
    {
        uint32 weight = (index < 9 ? index : 15 - index) * 1024;
        sint16 phase;
        sint32 wave;
        uint32 height;
        w_u32(child + 28, position[0]);
        w_u32(child + 32, position[1]);
        w_u32(child + 36, position[2]);
        phase = (sint16)((index + (uint32)(sint32)r_s16(object_offset + 130)) << 9);
        wave = SinQ14(phase) >> 2;
        height = r_u32(child + 32);
        w_u16(child + 42, amplitude >> 5);
        w_u16(child + 40, amplitude >> 5);
        w_u32(child + 32, height + (uint32)(((sint64)wave * weight) >> 12));
        position[0] += step[0];
        position[1] += step[1];
        position[2] += step[2];
        child = r_u32(child + 4);
    }
    index = r_u16(object_offset + 130) + 1;
    w_u16(object_offset + 130, index);
    return index;
}

uint32 p2_func_80034ba8(uint32 object_offset, uint32 data_offset, uint32 resource_offset)
{
    FUNCTION_MARKER(0x80034BA8, "SLUS_005.78");
    uint32 target = r_u32(0x8008A114);
    uint32 target_position[3];
    uint32 world_position[3];
    uint32 difference[3];
    uint32 definition;
    uint32 index;
    uint32 angle;
    sint32 state;
    target_position[0] = r_u32(target + 16);
    target_position[1] = r_u32(target + 20);
    target_position[2] = r_u32(target + 24);
    definition = r_u32(object_offset + 132);
    if (definition != 0 && r_u32(definition + 40) != 0)
    {
        uint32 saved_stack = p2_guest_stack_pointer;
        uint32 input;
        uint32 output;
        uint32 matrix;
        p2_guest_stack_pointer -= 32;
        input = p2_guest_stack_pointer;
        output = input + 16;
        for (index = 0; index < 3; ++index)
            world_position[index] = r_u32(object_offset + 16 + index * 4);
        for (index = 0; index < 3; ++index)
            w_u32(input + index * 4, world_position[index]);
        PushMatrix();
        matrix = r_u32(r_u32(object_offset + 132) + 40);
        for (index = 0; index < 5; ++index)
            xport_gte_write_control(index, r_u32(matrix + index * 4));
        p2_sdk_8007ec64(input, output);
        PopMatrix();
        for (index = 0; index < 3; ++index)
        {
            matrix = r_u32(r_u32(object_offset + 132) + 40);
            world_position[index] = r_u32(output + index * 4) + r_u32(matrix + 20 + index * 4);
            w_u32(output + index * 4, world_position[index]);
        }
        p2_guest_stack_pointer = saved_stack;
    }
    else
    {
        for (index = 0; index < 3; ++index)
            world_position[index] = r_u32(object_offset + 16 + index * 4);
    }
    for (index = 0; index < 3; ++index)
        difference[index] = target_position[index] - world_position[index];
    angle = Atan2Angle((sint32)(0u - difference[2]), (sint32)(0u - difference[0]));
    angle = (r_u16(data_offset + 104) - angle) & 4095;
    if (angle > 2048)
        angle = (4096 - angle) & 4095;
    state = r_s8(data_offset + 180);
    if (state == 2)
    {
        uint32 countdown = r_u8(data_offset + 181) - 1u;
        w_u8(data_offset + 181, countdown);
        if ((sint8)countdown <= 0)
        {
            w_u8(data_offset + 180, 3);
            w_u8(data_offset + 181, 4);
        }
    }
    else if (state == 1 && angle < 512)
    {
        uint32 definition_offset = r_u32(object_offset + 132);
        uint32 flags = r_u32(object_offset + 28);
        uint16 frame = r_u16(object_offset + 136);
        uint16 count = r_u16(definition_offset + 26);
        sint32 divisor;
        w_u32(object_offset + 28, (flags & ~0x800u) ^ 16);
        w_u16(object_offset + 136, (uint32)count - frame);
        w_u8(data_offset + 180, 2);
        divisor = r_s16(resource_offset + 68);
        if (divisor == 0)
            trap(7);
        w_u8(data_offset + 181, (uint32)(2048 / divisor) + 5);
    }
    else if (state == 3 && angle < 512)
    {
        uint32 countdown;
        if ((r_u32(data_offset + 44) & 16) != 0)
            return 4096;
        countdown = r_u8(data_offset + 181) - 1u;
        w_u8(data_offset + 181, countdown);
        if ((sint8)countdown > 0)
            return 4096;
        w_u8(data_offset + 180, 0);
    }
    return 0;
}

uint32 p2_func_80018b40(uint32 object_offset, uint32 vector_offset, uint32 quaternion_offset)
{
    FUNCTION_MARKER(0x80018B40, "SLUS_005.78");
    uint32 saved_stack = p2_guest_stack_pointer;
    uint32 matrix;
    uint32 rotated;
    uint32 object_rotation;
    uint32 definition;
    uint32 track;
    sint32 frame;
    uint32 square = 0;
    uint32 length;
    sint32 angle;
    uint32 result;
    uint32 index;
    p2_guest_stack_pointer -= 64;
    if (quaternion_offset == 0)
        quaternion_offset = p2_guest_stack_pointer;
    matrix = p2_guest_stack_pointer + 8;
    rotated = p2_guest_stack_pointer + 32;
    object_rotation = p2_guest_stack_pointer + 48;
    if ((r_u32(object_offset + 28) & 16) != 0)
    {
        definition = r_u32(object_offset + 132);
        track = r_u32(r_u32(definition + 44) + 56);
        frame = r_s16(object_offset + 136);
    }
    else
    {
        definition = r_u32(object_offset + 132);
        track = r_u32(r_u32(definition + 44) + 56);
        frame = r_s16(definition + 26) - r_s16(object_offset + 136) - 1;
    }
    p2_func_80017938(frame, track, quaternion_offset);
    definition = r_u32(r_u32(object_offset + 132) + 40);
    if (definition != 0)
    {
        uint32 first;
        uint32 second;
        p2_func_80017e4c(definition, object_rotation);
        if (r_s16(quaternion_offset) == 16384)
        {
            first = r_u32(object_rotation);
            second = r_u32(object_rotation + 4);
        }
        else if (r_s16(object_rotation) == 16384)
        {
            first = r_u32(quaternion_offset);
            second = r_u32(quaternion_offset + 4);
        }
        else
        {
            uint32 w = (uint32)(sint32)r_s16(quaternion_offset);
            uint32 x = (uint32)(sint32)r_s16(quaternion_offset + 2);
            uint32 y = (uint32)(sint32)r_s16(quaternion_offset + 4);
            uint32 z = (uint32)(sint32)r_s16(quaternion_offset + 6);
            uint32 mw = (uint32)(sint32)r_s16(object_rotation);
            uint32 mx = (uint32)(sint32)r_s16(object_rotation + 2);
            uint32 my = (uint32)(sint32)r_s16(object_rotation + 4);
            uint32 mz = (uint32)(sint32)r_s16(object_rotation + 6);
            uint32 combined_w = (w * mw - (x * mx + y * my + z * mz)) >> 14;
            uint32 combined_x = (w * mx + mw * x + y * mz - z * my) >> 14;
            uint32 combined_y = (w * my + mw * y + z * mx - x * mz) >> 14;
            uint32 combined_z = (uint32)((sint32)(w * mz + mw * z + x * my - y * mx) >> 14);
            first = (uint16)combined_w | ((uint32)(uint16)combined_x << 16);
            second = (uint16)combined_y | ((uint32)(uint16)combined_z << 16);
        }
        w_u32(quaternion_offset, first);
        w_u32(quaternion_offset + 4, second);
    }
    {
        uint32 x = (uint32)(sint32)r_s16(quaternion_offset + 2);
        uint32 y = (uint32)(sint32)r_s16(quaternion_offset + 4);
        uint32 z = (uint32)(sint32)r_s16(quaternion_offset + 6);
        uint32 w = (uint32)(sint32)r_s16(quaternion_offset);
        uint32 xx = x * x;
        uint32 yy = y * y;
        uint32 zz = z * z;
        uint32 xy = x * y;
        uint32 xz = x * z;
        uint32 yz = y * z;
        uint32 wx = w * x;
        uint32 wy = w * y;
        uint32 wz = w * z;
        w_u16(matrix, (sint32)(0x8000000u - (yy + zz)) >> 15);
        w_u16(matrix + 8, (sint32)(0x8000000u - (xx + zz)) >> 15);
        w_u16(matrix + 16, (sint32)(0x8000000u - (xx + yy)) >> 15);
        w_u16(matrix + 2, (sint32)(xy - wz) >> 15);
        w_u16(matrix + 6, (sint32)(xy + wz) >> 15);
        w_u16(matrix + 4, (sint32)(xz + wy) >> 15);
        w_u16(matrix + 12, (sint32)(xz - wy) >> 15);
        w_u16(matrix + 10, (sint32)(yz - wx) >> 15);
        w_u16(matrix + 14, (sint32)(yz + wx) >> 15);
    }
    PushMatrix();
    for (index = 0; index < 5; ++index)
        xport_gte_write_control(index, r_u32(matrix + index * 4));
    p2_sdk_8007ec64(vector_offset, rotated);
    PopMatrix();
    for (index = 0; index < 3; ++index)
    {
        uint32 scaled = (uint32)(r_s32(rotated + index * 4) >> 6);
        square += scaled * scaled;
    }
    if ((sint32)square >= 0)
        length = p2_sdk_8007e794(square);
    else
        length = p2_sdk_8007e794(square >> 2) * 2;
    angle = AsinAngle(r_s32(rotated + 4), (sint32)length);
    result = (sint16)angle == 4096 ? (uint32)angle : (0u - (uint32)angle) & 4095;
    p2_guest_stack_pointer = saved_stack;
    return (uint32)(sint32)(sint16)result;
}

uint32 p2_func_80017e4c(uint32 matrix_offset, uint32 quaternion_offset)
{
    FUNCTION_MARKER(0x80017E4C, "SLUS_005.78");
    sint32 first_diagonal = r_s16(matrix_offset);
    sint32 second_diagonal = r_s16(matrix_offset + 8);
    sint32 third_diagonal = r_s16(matrix_offset + 16);
    sint32 trace = first_diagonal + second_diagonal + third_diagonal;
    sint32 denominator;
    sint32 reciprocal;
    sint32 coefficient;
    uint32 product;
    uint32 result;
    if (trace > 0)
    {
        denominator = (sint32)(p2_sdk_8007e794(((uint32)trace * 4 + 16384) >> 2) * 4);
        if (denominator == 0)
            trap(7);
        reciprocal = 0x40000000 / denominator;
        w_u16(quaternion_offset, denominator >> 1);
        coefficient = (sint32)(uint32)(((sint64)8192 * reciprocal) >> 14);
        product = (uint32)(r_s16(matrix_offset + 10) - r_s16(matrix_offset + 14)) * 4 * (uint32)coefficient;
        w_u16(quaternion_offset + 2, (sint32)product >> 14);
        product = (uint32)(r_s16(matrix_offset + 12) - r_s16(matrix_offset + 4)) * 4 * (uint32)coefficient;
        w_u16(quaternion_offset + 4, (sint32)product >> 14);
        product = (uint32)(r_s16(matrix_offset + 2) - r_s16(matrix_offset + 6)) * 4 * (uint32)coefficient;
        w_u32(0x1F80030C, coefficient);
        result = (uint32)((sint32)product >> 14);
        w_u16(quaternion_offset + 6, result);
    }
    else
    {
        uint32 first = first_diagonal < second_diagonal;
        uint32 second;
        uint32 third;
        uint32 first_row;
        uint32 second_row;
        uint32 third_row;
        sint32 diagonal;
        if (r_s16(matrix_offset + first * 8) < third_diagonal)
            first = 2;
        second = first + 1;
        if (second == 3)
            second = 0;
        third = second + 1;
        if (third == 3)
            third = 0;
        first_row = matrix_offset + first * 6;
        second_row = matrix_offset + second * 6;
        third_row = matrix_offset + third * 6;
        diagonal = r_s16(first_row + first * 2) - (r_s16(second_row + second * 2) + r_s16(third_row + third * 2));
        denominator = (sint32)(p2_sdk_8007e794(((uint32)diagonal * 4 + 16384) >> 2) * 4);
        if (denominator == 0)
            trap(7);
        reciprocal = 0x40000000 / denominator;
        w_u16(quaternion_offset + first * 2 + 2, denominator >> 1);
        coefficient = (sint32)(uint32)(((sint64)8192 * reciprocal) >> 14);
        product = (uint32)(r_s16(second_row + third * 2) - r_s16(third_row + second * 2)) * 4 * (uint32)coefficient;
        w_u32(0x1F80030C, coefficient);
        w_u16(quaternion_offset, (sint32)product >> 14);
        product = (uint32)(r_s16(first_row + second * 2) + r_s16(second_row + first * 2)) * 4 * (uint32)coefficient;
        w_u16(quaternion_offset + second * 2 + 2, (sint32)product >> 14);
        product = (uint32)(r_s16(first_row + third * 2) + r_s16(third_row + first * 2)) * 4 * (uint32)coefficient;
        result = (uint32)((sint32)product >> 14);
        w_u16(quaternion_offset + third * 2 + 2, result);
    }
    return result;
}

static uint32 p2_track_distance_12(sint32 x, sint32 y, sint32 z)
{
    uint32 square;
    if ((uint32)x + 26754 > 0xD103 || (uint32)y + 26754 > 0xD103 || (uint32)z + 26754 > 0xD103)
    {
        square = (uint32)(((sint64)x * x) >> 12);
        square += (uint32)(((sint64)y * y) >> 12);
        square += (uint32)(((sint64)z * z) >> 12);
    }
    else
    {
        square = (uint32)x * (uint32)x + (uint32)y * (uint32)y + (uint32)z * (uint32)z;
        square = (uint32)((sint32)square >> 12);
    }
    return p2_sdk_8007e794(square);
}

uint32 p2_func_8001aaa8(uint32 position_track_offset, uint32 rotation_track_offset, uint32 position_offset, uint32 quaternion_offset, sint32 step)
{
    FUNCTION_MARKER(0x8001AAA8, "SLUS_005.78");
    uint32 saved_stack;
    uint32 first;
    uint32 second;
    uint32 first_rotation;
    uint32 second_rotation;
    uint32 difference[3];
    sint32 full_distance;
    sint32 current_distance;
    sint32 reciprocal;
    uint32 coefficient;
    uint32 blend;
    uint32 inverse;
    uint32 result = 0;
    uint32 index;
    if ((step == 1 && r_s16(position_track_offset + 8) == 0) || (step == -1 && r_s16(position_track_offset + 8) == r_s16(position_track_offset + 4)))
        return p2_func_80017938(r_s16(rotation_track_offset + 8), rotation_track_offset, quaternion_offset);
    saved_stack = p2_guest_stack_pointer;
    p2_guest_stack_pointer -= 48;
    first = p2_guest_stack_pointer;
    second = first + 16;
    first_rotation = first + 32;
    second_rotation = first + 40;
    SampleVectorTrack(r_s16(position_track_offset + 8), position_track_offset, first);
    p2_func_80017938(r_s16(rotation_track_offset + 8), rotation_track_offset, first_rotation);
    SampleVectorTrack((sint32)((uint32)(sint32)r_s16(position_track_offset + 8) - (uint32)step), position_track_offset, second);
    p2_func_80017938((sint32)((uint32)(sint32)r_s16(rotation_track_offset + 8) - (uint32)step), rotation_track_offset, second_rotation);
    for (index = 0; index < 3; ++index)
        difference[index] = r_u32(first + index * 4) - r_u32(second + index * 4);
    full_distance = (sint32)p2_track_distance_12((sint32)difference[0], (sint32)difference[1], (sint32)difference[2]);
    for (index = 0; index < 3; ++index)
        difference[index] = r_u32(position_offset + index * 4) - r_u32(second + index * 4);
    current_distance = (sint32)p2_track_distance_12((sint32)difference[0], (sint32)difference[1], (sint32)difference[2]);
    if (full_distance == 0)
        trap(7);
    reciprocal = 0x40000000 / full_distance;
    coefficient = (uint32)(((sint64)current_distance * reciprocal) >> 18);
    blend = coefficient * 4;
    inverse = 16384 - blend;
    for (index = 0; index < 4; ++index)
    {
        uint32 old_product = (uint32)(sint32)r_s16(second_rotation + index * 2) * inverse;
        uint32 new_product = (uint32)(sint32)r_s16(first_rotation + index * 2) * blend;
        if (index == 3)
            w_u32(0x1F80030C, coefficient);
        result = (uint32)((sint32)old_product >> 14) + (uint32)((sint32)new_product >> 14);
        w_u16(quaternion_offset + index * 2, result);
    }
    p2_guest_stack_pointer = saved_stack;
    return result;
}

uint32 p2_func_8001a76c(uint32 track_offset, uint32 vector_offset, sint32 step, sint32 bias)
{
    FUNCTION_MARKER(0x8001A76C, "SLUS_005.78");
    uint32 saved_stack = p2_guest_stack_pointer;
    uint32 sample;
    uint32 remaining = (uint32)bias;
    p2_guest_stack_pointer -= 16;
    sample = p2_guest_stack_pointer;
    for (;;)
    {
        uint32 difference[3];
        sint32 length;
        uint32 index;
        SampleVectorTrack(r_s16(track_offset + 8), track_offset, sample);
        for (index = 0; index < 3; ++index)
            difference[index] = r_u32(sample + index * 4) - r_u32(vector_offset + index * 4);
        length = (sint32)p2_track_distance_12((sint32)difference[0], (sint32)difference[1], (sint32)difference[2]);
        if ((sint32)remaining < length)
        {
            sint32 reciprocal;
            sint32 coefficient;
            if (length == 0)
                trap(7);
            reciprocal = 0x40000000 / length;
            coefficient = (sint32)(uint32)(((sint64)(sint32)remaining * reciprocal) >> 18);
            remaining = 0;
            w_u32(0x1F80030C, coefficient);
            for (index = 0; index < 3; ++index)
                difference[index] = (uint32)(((sint64)(sint32)difference[index] * coefficient) >> 12);
            for (index = 0; index < 3; ++index)
                w_u32(vector_offset + index * 4, r_u32(vector_offset + index * 4) + difference[index]);
        }
        else
        {
            uint32 position[3];
            uint16 count;
            sint32 proposed;
            uint16 next;
            position[0] = r_u32(sample);
            position[1] = r_u32(sample + 4);
            position[2] = r_u32(sample + 8);
            w_u32(vector_offset, position[0]);
            w_u32(vector_offset + 4, position[1]);
            w_u32(vector_offset + 8, position[2]);
            proposed = r_s16(track_offset + 8);
            count = r_u16(track_offset + 4);
            remaining -= (uint32)length;
            proposed = (sint32)((uint32)proposed + (uint32)step);
            next = (uint16)proposed;
            w_u16(track_offset + 8, next);
            if (next >= count)
            {
                if ((r_u8(track_offset + 2) & 2) != 0)
                    w_u16(track_offset + 8, (sint16)next < 0 ? (uint32)count - 1 : 0);
                else if (count < next)
                {
                    w_u16(track_offset + 8, (uint32)proposed - (uint32)step);
                    break;
                }
            }
            if (r_s16(track_offset + 8) != proposed && (r_u8(track_offset + 2) & 3) != 3)
                SampleVectorTrack(r_s16(track_offset + 8), track_offset, vector_offset);
        }
        if (remaining == 0)
            break;
    }
    p2_guest_stack_pointer = saved_stack;
    return remaining != (uint32)bias;
}

uint32 p2_sdk_8006fb68(const void *source, uint32 bytes)
{
    static uint32 bound;
    if (bound == 0)
    {
        SpuNativeTransferGuestBinding binding;
        binding.requested_mode = 0x800878F8;
        binding.normalized_mode = 0x80087D78;
        binding.address_units = 0x80087D74;
        binding.completion = 0x80087D90;
        binding.completion_callback = 0x80087D94;
        binding.source_address = 0x80087DB0;
        binding.dma_blocks = 0x80087DB4;
        binding.address_shift = r_u32(0x80087D84);
        if (!spu_bind_native_transfer_guest(&binding))
        {
            fprintf(stderr, "P2: Cannot bind SPU transfer state\n");
            abort();
        }
        bound = 1;
    }
    SpuWrite(source, bytes);
    if (spu_transfer_failed())
    {
        fprintf(stderr, "P2: SPU transfer failed for %u bytes\n", bytes);
        abort();
    }
    return bytes;
}

static uint32 p2_spu_collect_80079c14(void)
{
    sint32 last = r_s32(0x80087DBC);
    sint32 index = 0;
    uint32 table;
    uint32 current;
    uint32 result;
    if (last >= 0)
    {
        table = r_u32(0x80087DC0);
        current = table;
        do
        {
            uint32 merged = 0;
            if ((r_u32(current) & 0x80000000) != 0)
            {
                sint32 next_index = index + 1;
                uint32 next = table + (uint32)next_index * 8;
                while (r_u32(next) == 0x2FFFFFFF)
                {
                    next += 8;
                    ++next_index;
                }
                next = table + (uint32)next_index * 8;
                if ((r_u32(next) & 0x80000000) != 0)
                {
                    uint32 next_address = r_u32(next) & 0x0FFFFFFF;
                    uint32 end = (r_u32(current) & 0x0FFFFFFF) + r_u32(current + 4);
                    if (next_address == end)
                    {
                        w_u32(next, 0x2FFFFFFF);
                        w_u32(current + 4, r_u32(current + 4) + r_u32(next + 4));
                        merged = 1;
                    }
                }
            }
            if (merged == 0)
            {
                current += 8;
                ++index;
            }
        } while (index <= r_s32(0x80087DBC));
        last = r_s32(0x80087DBC);
    }
    if (last >= 0)
    {
        current = r_u32(0x80087DC0);
        index = 0;
        do
        {
            uint32 size = r_u32(current + 4);
            ++index;
            if (size == 0)
                w_u32(current, 0x2FFFFFFF);
            current += 8;
        } while (index <= last);
    }
    last = r_s32(0x80087DBC);
    if (last >= 0)
    {
        table = r_u32(0x80087DC0);
        current = table;
        index = 0;
        do
        {
            sint32 next_index;
            if ((r_u32(current) & 0x40000000) != 0)
                break;
            next_index = index + 1;
            if (next_index <= last)
            {
                sint32 inner_last = r_s32(0x80087DBC);
                uint32 next = table + (uint32)next_index * 8;
                do
                {
                    uint32 next_address = r_u32(next);
                    uint32 old_address;
                    if ((next_address & 0x40000000) != 0)
                        break;
                    old_address = r_u32(current);
                    ++next_index;
                    if ((next_address & 0x0FFFFFFF) < (old_address & 0x0FFFFFFF))
                    {
                        uint32 next_size;
                        uint32 old_size;
                        w_u32(current, next_address);
                        next_size = r_u32(next + 4);
                        old_size = r_u32(current + 4);
                        w_u32(current + 4, next_size);
                        w_u32(next, old_address);
                        w_u32(next + 4, old_size);
                    }
                    next += 8;
                } while (next_index <= inner_last);
            }
            last = r_s32(0x80087DBC);
            ++index;
            current += 8;
        } while (index <= last);
    }
    last = r_s32(0x80087DBC);
    if (last >= 0)
    {
        table = r_u32(0x80087DC0);
        current = table;
        index = 0;
        do
        {
            uint32 address = r_u32(current);
            if ((address & 0x40000000) != 0)
                break;
            if (address == 0x2FFFFFFF)
            {
                uint32 tail = table + (uint32)last * 8;
                uint32 size;
                w_u32(current, r_u32(tail));
                size = r_u32(tail + 4);
                w_u32(0x80087DBC, index);
                w_u32(current + 4, size);
                break;
            }
            last = r_s32(0x80087DBC);
            ++index;
            current += 8;
        } while (index <= last);
    }
    index = (sint32)(r_u32(0x80087DBC) - 1);
    result = (uint32)index * 8;
    if (index >= 0)
    {
        table = r_u32(0x80087DC0);
        current = table + result;
        do
        {
            uint32 address = r_u32(current);
            uint32 old_last;
            uint32 size;
            result = address & 0x0FFFFFFF;
            if ((address & 0x80000000) == 0)
                break;
            old_last = r_u32(0x80087DBC);
            w_u32(current, result | 0x40000000);
            size = r_u32(current + 4);
            w_u32(0x80087DBC, index);
            --index;
            result = size + r_u32(table + old_last * 8 + 4);
            w_u32(current + 4, result);
            current -= 8;
        } while (index >= 0);
    }
    return result;
}

static uint32 p2_spu_free_80079b98(uint32 address)
{
    sint32 limit = r_s32(0x80087DB8);
    if (limit > 0)
    {
        uint32 entry = r_u32(0x80087DC0);
        sint32 index = 0;
        do
        {
            uint32 allocated = r_u32(entry);
            if ((allocated & 0x40000000) != 0)
                break;
            ++index;
            if (allocated == address)
            {
                w_u32(entry, address | 0x80000000);
                break;
            }
            entry += 8;
        } while (index < limit);
    }
    return p2_spu_collect_80079c14();
}

uint32 p2_sdk_80079b14(sint32 bank)
{
    uint32 index;
    uint32 result;
    if ((uint16)bank >= 16)
        return 0;
    index = (uint32)(sint32)(sint16)bank;
    result = index * 4;
    if (r_u8(0x800B7F90 + index) == 1)
    {
        p2_spu_free_80079b98(r_u32(0x800BA458 + result));
        w_u8(0x800B7F90 + index, 0);
        result = r_u16(0x800BA450) - 1u;
        w_u16(0x800BA450, result);
    }
    return result;
}

static uint32 p2_spu_malloc_8007a3b8(uint32 bytes)
{
    uint32 reserved = 0;
    uint32 shift;
    uint32 mask;
    uint32 table;
    uint32 entry;
    uint32 address;
    uint32 size;
    sint32 selected = -1;
    sint32 index;
    if (r_u32(0x80087900) != 0)
        reserved = (0x10000 - r_u32(0x80087904)) << (r_u32(0x80087D84) & 31);
    mask = r_u32(0x80087D8C);
    if ((bytes & ~mask) != 0)
        bytes += mask;
    shift = r_u32(0x80087D84) & 31;
    bytes = (uint32)((sint32)bytes >> shift) << shift;
    table = r_u32(0x80087DC0);
    if ((r_u32(table) & 0x40000000) != 0)
        selected = 0;
    else
    {
        sint32 limit;
        p2_spu_collect_80079c14();
        limit = r_s32(0x80087DB8);
        if (limit > 0)
        {
            entry = r_u32(0x80087DC0);
            for (index = 0; index < limit; ++index, entry += 8)
            {
                address = r_u32(entry);
                if ((address & 0x40000000) != 0 || ((address & 0x80000000) != 0 && r_u32(entry + 4) >= bytes))
                {
                    selected = index;
                    break;
                }
            }
        }
    }
    if (selected == -1)
        return (uint32)-1;
    table = r_u32(0x80087DC0);
    entry = table + (uint32)selected * 8;
    address = r_u32(entry);
    if ((address & 0x40000000) != 0)
    {
        uint32 next;
        uint32 original;
        if (selected >= r_s32(0x80087DB8))
            return (uint32)-1;
        if (r_u32(entry + 4) - reserved < bytes)
            return (uint32)-1;
        next = table + (uint32)(selected + 1) * 8;
        w_u32(next, ((r_u32(entry) & 0x0FFFFFFF) + bytes) | 0x40000000);
        w_u32(next + 4, r_u32(entry + 4) - bytes);
        original = r_u32(entry);
        w_u32(0x80087DBC, selected + 1);
        w_u32(entry + 4, bytes);
        w_u32(entry, original & 0x0FFFFFFF);
    }
    else
    {
        size = r_u32(entry + 4);
        if (bytes < size)
        {
            sint32 last = r_s32(0x80087DBC);
            if (last < r_s32(0x80087DB8))
            {
                uint32 tail = table + (uint32)last * 8;
                uint32 tail_address = r_u32(tail);
                uint32 tail_size = r_u32(tail + 4);
                w_u32(tail, (bytes + address) | 0x80000000);
                w_u32(tail + 4, size - bytes);
                w_u32(0x80087DBC, (uint32)last + 1);
                w_u32(tail + 8, tail_address);
                w_u32(tail + 12, tail_size);
            }
        }
        entry = r_u32(0x80087DC0) + (uint32)selected * 8;
        address = r_u32(entry);
        w_u32(entry + 4, bytes);
        w_u32(entry, address & 0x0FFFFFFF);
    }
    p2_spu_collect_80079c14();
    return r_u32(r_u32(0x80087DC0) + (uint32)selected * 8);
}

static uint32 p2_vab_open_80079fb0(uint32 header, sint16 bank, sint16 sticky, uint32 body)
{
    sint32 selected = 16;
    uint32 slot;
    uint32 bank_index;
    uint32 magic;
    sint32 capacity;
    uint32 programs;
    uint32 tones;
    uint32 sample_count;
    uint32 lengths[256];
    uint32 total = 0;
    uint32 index;
    uint32 active;
    uint32 lengths_table;
    if (p2_func_80077150() == 1)
        return (uint32)-1;
    p2_func_80077124(1);
    if (bank < 16)
    {
        if (bank == -1)
        {
            for (selected = 0; selected < 16; ++selected)
                if (r_u8(0x800B7F90 + (uint32)selected) == 0)
                    break;
        }
        else if (r_u8(0x800B7F90 + (uint32)(sint32)bank) == 0)
            selected = bank;
        if (selected < 16)
        {
            w_u8(0x800B7F90 + (uint32)selected, 1);
            w_u16(0x800BA450, r_u16(0x800BA450) + 1);
        }
    }
    if (selected >= 16)
    {
        p2_func_80077124(0);
        return (uint32)-1;
    }
    slot = 0x800B7F90 + (uint32)(sint32)(sint16)selected;
    bank_index = (uint32)(sint32)(sint16)selected * 4;
    w_u32(0x800B15B0 + bank_index, header);
    magic = r_u32(header);
    programs = header + 32;
    if ((magic >> 8) != 0x564142)
        goto failed;
    capacity = 64;
    if ((uint8)magic == 112 && r_s32(header + 4) >= 5)
        capacity = 128;
    w_u16(0x800B1704, capacity);
    capacity = r_s16(0x800B1704);
    if (capacity < r_u16(header + 18))
        goto failed;
    w_u32(0x800B1570 + bank_index, programs);
    tones = programs + (uint32)capacity * 16;
    active = 0;
    for (index = 0; (sint32)index < capacity; ++index)
    {
        uint8 used = r_u8(programs + index * 16);
        w_u32(programs + index * 16 + 8, active);
        if (used != 0)
            ++active;
    }
    w_u32(0x800B1698 + bank_index, tones);
    lengths_table = tones + (uint32)r_u16(header + 18) * 512;
    sample_count = r_u8(header + 22);
    for (index = 0; index < 256; ++index)
    {
        if (index <= sample_count)
        {
            sint32 version = r_s32(header + 4);
            uint32 length = r_u16(lengths_table + index * 2);
            lengths[index] = length << (version >= 5 ? 3 : 2);
            total += lengths[index];
        }
    }
    w_u32(0x800BA498 + bank_index, lengths_table + 512);
    if (sticky == 0)
    {
        body = p2_spu_malloc_8007a3b8(total);
        if (body == (uint32)-1)
            goto failed;
    }
    if (body + total > 0x80000)
        goto failed;
    w_u32(0x800BA458 + bank_index, body);
    total = 0;
    for (index = 0; index <= sample_count; ++index)
    {
        total += lengths[index];
        w_u16(programs + (index / 2) * 16 + ((index & 1) != 0 ? 14 : 12), (body + total) >> 3);
    }
    w_u32(0x800BA410 + bank_index, total);
    w_u8(slot, 2);
    return (uint32)(sint32)(sint16)selected;
failed:
    w_u8(slot, 0);
    p2_func_80077124(0);
    w_u16(0x800BA450, r_u16(0x800BA450) - 1u);
    return (uint32)-1;
}

uint32 p2_sdk_80079f48(uint32 header, sint32 bank, uint32 address)
{
    return (uint32)(sint32)(sint16)p2_vab_open_80079fb0(header, (sint16)bank, 1, address);
}

static uint32 p2_transfer_mode_80070f40(uint32 mode)
{
    uint32 normalized = mode == 1;
    w_u32(0x800878F8, mode);
    w_u32(0x80087D78, normalized);
    return normalized;
}

uint32 p2_sdk_8007a680(uint32 samples, sint16 bank)
{
    uint32 index = (uint32)(sint32)bank;
    if ((uint16)bank < 17 && r_u8(0x800B7F90 + index) == 2)
    {
        uint32 address = r_u32(0x800BA458 + index * 4);
        uint32 bytes;
        p2_transfer_mode_80070f40(0);
        SpuSetTransferMode(0);
        p2_sdk_80070eec(address);
        bytes = r_u32(0x800BA410 + index * 4);
        p2_func_80070e8c(psx_addr(samples, bytes > 0x7EFF0 ? 0x7EFF0 : bytes), bytes);
        w_u8(0x800B7F90 + index, 1);
        return index;
    }
    p2_func_80077124(0);
    return (uint32)-1;
}
