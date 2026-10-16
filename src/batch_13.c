#include "game.h"

uint32 p2_func_8002fb94(uint32 object_offset)
{
    uint32 result, saved_stack, speed, state, magnitude, x, y, z, squared;
    if (r_u32(object_offset + 116) == 19 && (r_u32(0x8008A178) & 3) != 3)
    {
        result = r_u16(object_offset + 128) + 1;
        w_u16(object_offset + 128, result);
        return result;
    }
    saved_stack = p2_guest_stack_pointer;
    p2_guest_stack_pointer -= 16;
    speed = p2_guest_stack_pointer;
    result = p2_func_80018144(object_offset, object_offset + 92, 0);
    w_u16(object_offset + 150, result);
    w_u16(object_offset + 88, (result + 2048) & 0xFFF);
    x = r_u32(object_offset + 92);
    y = r_u32(object_offset + 96);
    z = r_u32(object_offset + 100);
    if (r_u32(object_offset + 28) & 0x400)
        y = 0;
    if (x || z)
    {
        x = (uint32)((sint32)x >> 6);
        y = (uint32)((sint32)y >> 6);
        z = (uint32)((sint32)z >> 6);
        squared = x * x + y * y + z * z;
        if ((sint32)squared >= 0)
            result = p2_sdk_8007e794(squared);
        else
            result = p2_sdk_8007e794(squared >> 2) << 1;
        w_u32(speed, result);
    }
    else
        w_u32(speed, 0);
    state = r_u32(object_offset + 116);
    if (state == 19)
        p2_func_80044990(object_offset, speed);
    else if (!state)
        p2_func_80044cfc(object_offset, speed);
    result = r_u32(object_offset + 28) & 0x2000;
    if (!result)
    {
        magnitude = r_u32(speed);
        if (magnitude || r_u32(object_offset + 96))
            result = p2_func_8003db7c(object_offset, (sint32)magnitude, r_s32(object_offset + 96), 0);
        else
        {
            w_u32(object_offset + 100, 0);
            w_u32(object_offset + 96, 0);
            w_u32(object_offset + 92, 0);
        }
    }
    p2_guest_stack_pointer = saved_stack;
    return result;
}

uint32 p2_func_80044474(void)
{
    uint32 index, object, result;
    if (r_s32(0x800ABD68) >= 0)
        return 1;
    index = 1 - (r_u16(0x800AC02A) & 15);
    object = r_u32(0x8008B4B8 + (index << 2));
    object = r_u32(object + 232);
    result = r_u8(object + 13) | 1;
    w_u8(object + 13, result);
    return result;
}

uint32 p2_func_80024c44(void)
{
    uint32 scene, x, y, z, count, factor, result;
    scene = r_u32(0x8008A114);
    x = r_u32(0x800B16D8);
    x -= r_u32(scene + 16);
    y = r_u32(0x800B16DC);
    y -= r_u32(scene + 20);
    z = r_u32(0x800B16E0);
    z -= r_u32(scene + 24);
    count = r_u32(0x8008A4F4);
    if (count)
    {
        --count;
        factor = r_u16(0x80010054 + (count << 1));
        w_u32(0x8008A4F4, count);
        x = (uint32)(((sint64)(sint32)x * (sint32)factor) >> 12);
        y = (uint32)(((sint64)(sint32)y * (sint32)factor) >> 12);
        z = (uint32)(((sint64)(sint32)z * (sint32)factor) >> 12);
    }
    w_u32(scene + 16, r_u32(scene + 16) + x);
    w_u32(scene + 20, r_u32(scene + 20) + y);
    w_u32(scene + 24, r_u32(scene + 24) + z);
    if (!r_u32(0x8008A4F4))
    {
        p2_func_80044474();
        w_u32(0x1F800320, r_u32(0x1F800320) & 0xFFFFFEFF);
    }
    scene = r_u32(0x8008A114);
    result = r_u8(scene) & 0xFE;
    w_u8(scene, result);
    return result;
}

uint32 p2_func_8006d30c(uint32 selection, uint32 mode)
{
    uint32 buffer, position, extent, step, phase, pointer, result;
    sint32 fraction;
    buffer = r_u32(0x8008A454);
    w_u32(0x8008A4B8, 0);
    w_u32(0x8008A4F8, buffer);
    p2_func_800651bc(0, 0x00F00140, 4096, 0xFFFFFF);
    if (mode)
    {
        p2_func_80064ff4(0x007800A0, 0x800107C4, 1, 0);
        p2_func_80064ff4(0x009600A0, 0x8008A2B8, 1, 0);
    }
    else
        p2_func_80064ff4(0x007800A0, 0x800107B4, 1, 0);
    if (!selection)
    {
        position = mode ? 0x00930067 : 0x007500C0;
        extent = mode ? 0x0014002E : 0x0014003E;
    }
    else
    {
        position = mode ? 0x009300AB : 0x00750035;
        extent = mode ? 0x00140022 : 0x00140073;
    }
    phase = r_u32(0x8008A3C0);
    step = r_u32(0x8008A2A8);
    phase += step;
    w_u32(0x8008A3C0, phase);
    if ((sint32)phase >= 2048)
    {
        w_u32(0x8008A3C0, 2048);
        w_u32(0x8008A2A8, 0 - step);
    }
    else if ((sint32)phase < -2047)
    {
        w_u32(0x8008A3C0, (uint32)-2048);
        w_u32(0x8008A2A8, 0 - step);
    }
    fraction = r_s32(0x8008A3C0);
    if (fraction > 0)
        p2_func_800652e4(position, extent, fraction, 0x00808080);
    else if (fraction < 0)
        p2_func_800651bc(position, extent, (sint32)(0 - (uint32)fraction), 0x00808080);
    if (!selection)
        p2_func_80064ff4(0x00CA005A, 0x8008A2C0, 0, 0);
    else
        p2_func_80064ff4(0x00CA005A, 0x80010730, 0, 0);
    pointer = r_u32(0x8008A4A4);
    pointer = r_u32(pointer + 28);
    p2_func_80064e74(0x00C80037, 0, pointer + 160, 0);
    pointer = r_u32(0x8008A478);
    w_u32(pointer, r_u32(pointer) | 0xFFFFFF);
    p2_func_800668a0(r_u32(0x8008A454), 0);
    result = p2_func_80066d78(r_u32(0x8008A3C8), 0x8008A3CC);
    w_u32(0x8008A3C8, result);
    return result;
}

uint32 p2_func_8006d678(uint32 mode)
{
    uint32 current, previous, pressed, table, sound, tick, value;
    sint32 selection, countdown, voice;
    current = p2_func_80066d08();
    value = r_u32(0x8008A434);
    countdown = 0;
    selection = 1;
    w_u32(0x8008A3C4, current);
    w_u32(0x8008A3CC, 30);
    w_u32(0x8008A3C8, value);
    for (;;)
    {
        p2_func_8006d30c((uint32)selection, mode);
        pressed = 0;
        if (!countdown)
        {
            previous = r_u32(0x8008A3C4);
            current = p2_func_80066d08();
            w_u32(0x8008A3C4, current);
            pressed = current & ~previous;
        }
        if (pressed & 0xA000)
        {
            sound = 0;
            table = r_u32(0x8008A408);
            tick = r_u32(0x8008A518);
            if (table && r_s32(table) > 24)
                sound = table + 196;
            w_u32(0x8008A518, r_u32(0x8008A4F0));
            voice = p2_func_8005e628(0xFFFFFFFF, sound, 4);
            if (voice >= 0)
                p2_func_8005e470(sound, (uint16)voice, 0, 0, 0, r_s16(0x8008A24C), 0);
            w_u32(0x8008A518, tick);
            if (pressed & 0x3000)
            {
                --selection;
                if (selection < 0)
                    selection = 1;
            }
            else
            {
                ++selection;
                if (selection == 2)
                    selection = 0;
            }
        }
        else if (pressed & 0x40)
        {
            if (selection)
                return 0;
            sound = 0;
            table = r_u32(0x8008A408);
            tick = r_u32(0x8008A518);
            if (table && r_s32(table) > 25)
                sound = table + 204;
            w_u32(0x8008A518, r_u32(0x8008A4F0));
            voice = p2_func_8005e628(0xFFFFFFFF, sound, 4);
            if (voice >= 0)
                p2_func_8005e470(sound, (uint16)voice, 0, 0, 0, r_s16(0x8008A24C), 0);
            w_u32(0x8008A518, tick);
            if (!mode && !(p2_func_8006d678(1) & 0xFF))
                return 0;
            if (mode != 1)
                return 1;
            countdown = 10;
        }
        if (countdown)
        {
            --countdown;
            if (countdown <= 0)
                return 1;
        }
        w_u32(0x8008A4F0, r_u32(0x8008A4F0) + 1);
    }
}

uint32 p2_func_800668a0(uint32 first_ot_offset, uint32 second_ot_offset)
{
    uint32 index, environment;
    if (r_u32(0x8008A4B0))
        DrawSync(0);
    if (r_u32(0x8008A4A8))
        VSync(0);
    index = (1 - r_u8(0x8008A374)) & 0xFF;
    w_u8(0x8008A374, index);
    PutDrawEnv((DRAWENV *)psx_addr(0x80087200 + index * 132, sizeof(DRAWENV)));
    DrawOTag((uint32 *)psx_addr(first_ot_offset, 4));
    if (second_ot_offset)
        DrawOTag((uint32 *)psx_addr(second_ot_offset, 4));
    DrawSync(0);
    VSync(0);
    index = r_u8(0x8008A374);
    environment = 0x8008725C + index * 132;
    PutDispEnv((DISPENV *)psx_addr(environment, sizeof(DISPENV)));
    return environment;
}

uint32 p2_func_8006d8ec(void)
{
    uint32 buffer, object, pointer, objects, tick, current_tick, count, index, elapsed, first_ot, result;
    buffer = r_u32(0x8008A454);
    w_u32(0x8008A4B8, 0);
    w_u32(0x8008A4F8, buffer);
    w_u32(0x8008A4A0, buffer + 41600);
    p2_func_800651bc(0, 0x00F00140, 1638, 0xFFFFFF);
    object = p2_func_800536f0(0);
    p2_func_80065bc4(object);
    p2_func_80064ff4(0x007800A0, 0x8008A2C8, 1, 0);
    if (!r_u16(r_u32(0x8008A4E4) + 262))
        p2_func_800651bc(0x00C30000, 0x001D0140, 4096, 0xFFFFFF);
    p2_func_80064ff4(0x00CA00A0, 0x800107D0, 1, 0);
    pointer = r_u32(0x8008A478);
    w_u32(pointer, r_u32(pointer) | 0xFFFFFF);
    p2_func_80027228();
    objects = r_u32(0x8008A430);
    if (objects)
    {
        tick = r_u32(0x8008A518);
        current_tick = r_u32(0x8008A4F0);
        count = r_u32(objects + 24);
        w_u32(0x8008A518, current_tick);
        if ((sint32)count > 0)
        {
            count = 0;
            index = 0;
            do
            {
                pointer = r_u32(0x800B1524 + index);
                index += 8;
                p2_func_800275bc(pointer);
                objects = r_u32(0x8008A430);
                elapsed = r_u32(objects + 24);
                ++count;
            } while ((sint32)count < (sint32)elapsed);
        }
        w_u32(0x8008A518, tick);
    }
    elapsed = r_u32(0x8008A3D0);
    if (elapsed)
    {
        --elapsed;
        w_u32(0x8008A3D0, elapsed);
        if (elapsed)
            p2_func_800651bc(0, 0x00F00140, (sint32)(elapsed << 12) / 15, 0xFFFFFF);
    }
    first_ot = r_u32(0x8008A4AC);
    buffer = r_u32(0x8008A454);
    p2_func_800668a0(first_ot, buffer);
    result = p2_func_80066d78(r_u32(0x8008A3C8), 0x8008A3CC);
    w_u32(0x8008A3C8, result);
    return result;
}

uint32 p2_func_800504e4(void)
{
    uint32 table, sound, result;
    table = r_u32(0x8008A408);
    w_u32(0x8008A348, 0);
    sound = 0;
    if (table && r_s32(table) >= 7)
        sound = table + 52;
    if (p2_func_8005e868(sound, (uint32)(sint32)r_s8(0x800AC0F8)) & 0xFF)
        w_u32(0x8008A348, r_u32(0x8008A348) | 1);
    table = r_u32(0x8008A408);
    sound = 0;
    if (table && r_s32(table) >= 11)
        sound = table + 84;
    if (p2_func_8005e868(sound, (uint32)(sint32)r_s8(0x800AC0F8)) & 0xFF)
        w_u32(0x8008A348, r_u32(0x8008A348) | 2);
    result = r_u32(0x8008A348);
    if (result)
    {
        p2_func_8006a970((uint16)(sint16)r_s8(0x800AC0F8));
        result = 0xFFFFFFFF;
        w_u8(0x800AC0F8, result);
    }
    return result;
}

uint32 p2_func_80060f9c(void)
{
    uint32 result;
    while (p2_func_80069ac0())
    {
    }
    result = r_u32(0x1F800320) | 0x2000;
    w_u32(0x1F800320, result);
    return result;
}

uint32 p2_func_8006ad80(uint16 handle)
{
    return p2_sdk_80074210((sint16)((sint32)(uint8)handle - 1));
}

sint32 p2_func_8006daf4(uint32 level)
{
    uint32 flags, handle, sound, table, tick, value, pointer, previous, current, pressed;
    sint32 voice;
    flags = r_u32(0x1F800320);
    handle = r_u16(0x8008A530);
    w_u32(0x1F800320, flags | 0x10);
    p2_func_8006ad80((uint16)handle);
    p2_func_80025f58();
    p2_func_800504e4();
    sound = 0;
    table = r_u32(0x8008A408);
    tick = r_u32(0x8008A518);
    if (table && r_s32(table) > 23)
        sound = table + 188;
    w_u32(0x8008A518, r_u32(0x8008A4F0));
    voice = p2_func_8005e628(0xFFFFFFFF, sound, 4);
    if (voice >= 0)
        p2_func_8005e470(sound, (uint16)voice, 0, 0, 0, r_s16(0x8008A24C), 0);
    value = r_u16(r_u32(0x8008A4E4) + 262);
    w_u32(0x8008A518, tick);
    if (!value)
    {
        pointer = r_u32(0x8008A438);
        if (pointer)
            w_u32(pointer, r_u32(pointer) | 0xFFFFFF);
    }
    current = p2_func_80066d08();
    value = r_u32(0x8008A434);
    w_u32(0x8008A3C4, current);
    w_u32(0x8008A3CC, 30);
    w_u32(0x8008A3C8, value);
    for (;;)
    {
        p2_func_8006d8ec();
        previous = r_u32(0x8008A3C4);
        current = p2_func_80066d08();
        w_u32(0x8008A3C4, current);
        pressed = current & ~previous;
        if (pressed & 0x800)
            break;
        if (pressed & 0x100)
        {
            sound = 0;
            table = r_u32(0x8008A408);
            tick = r_u32(0x8008A518);
            if (table && r_s32(table) > 25)
                sound = table + 204;
            w_u32(0x8008A518, r_u32(0x8008A4F0));
            voice = p2_func_8005e628(0xFFFFFFFF, sound, 4);
            if (voice >= 0)
                p2_func_8005e470(sound, (uint16)voice, 0, 0, 0, r_s16(0x8008A24C), 0);
            w_u32(0x8008A518, tick);
            if (p2_func_8006d678(0) & 0xFF)
            {
                p2_func_80060f9c();
                goto finish_8006daf4;
            }
            break;
        }
        p2_func_80027228();
    }
    table = r_u32(0x8008A408);
    tick = r_u32(0x8008A518);
    sound = 0;
    w_u32(0x8008A174, 64);
    if (table && r_s32(table) > 23)
        sound = table + 188;
    w_u32(0x8008A518, r_u32(0x8008A4F0));
    voice = p2_func_8005e628(0xFFFFFFFF, sound, 4);
    if (voice >= 0)
        p2_func_8005e470(sound, (uint16)voice, 0, 0, 0, r_s16(0x8008A24C), 0);
    handle = r_u16(0x8008A530);
    w_u32(0x8008A518, tick);
    p2_func_8006ada4((uint16)handle);
    p2_func_800505e4();
finish_8006daf4:
    w_u32(0x1F800320, r_u32(0x1F800320) & 0xFFFFFFEF);
    return (sint32)p2_func_8005ed74(0);
}

uint32 p2_func_80064a58(void)
{
    uint32 brightness, table, packet, color, value, result;
    brightness = r_u16(0x8008A532);
    table = r_u32(0x8008A4D0);
    packet = r_u32(0x8008A4F8);
    brightness = (255 * brightness + 2048) >> 12;
    color = brightness | (brightness << 8) | (brightness << 16);
    w_u32(packet, ((packet + 12) & 0xFFFFFF) | 0x02000000);
    w_u32(packet + 4, 0xE1000040);
    w_u32(packet + 8, 0);
    value = r_u32(table + 120);
    w_u32(packet + 12, (value & 0xFFFFFF) | 0x03000000);
    w_u32(packet + 16, color | 0x62000000);
    w_u32(packet + 20, 0);
    w_u32(packet + 24, 0x00F00140);
    value = r_u32(table + 120);
    result = (value & 0xFF000000) | (packet & 0xFFFFFF);
    w_u32(table + 120, result);
    w_u32(0x8008A4F8, packet + 28);
    return result;
}

uint32 p2_func_800255f8(uint32 scene_offset)
{
    uint32 index, table;
    index = (uint32)(sint32)r_s8(scene_offset + 14);
    table = r_u32(0x8008A4E4);
    return (uint32)(sint32)r_s8(table + index * 24 + 1);
}

uint32 p2_func_80022ae4(uint32 object_offset)
{
    uint32 node;
    node = r_u32(object_offset + 40);
    while (node)
    {
        if (r_u16(node + 14) & 4)
            return node;
        node = r_u32(node);
    }
    return 0;
}

uint32 p2_func_8006aba0(void)
{
    uint32 sequence, volume;
    sequence = r_u8(0x8008A3AC);
    volume = r_u16(0x8008A24E);
    sequence = (sequence - 1) & 0xFF;
    volume = (127 * volume) >> 12;
    p2_sdk_80075d58((uint8)sequence, (sint16)volume, (sint16)volume);
    return r_u16(0x8008A3AC);
}

void p2_func_8006abe4(void)
{
    uint32 sequence, volume, previous, current;
    sequence = r_u8(0x8008A3B0);
    volume = r_u16(0x8008A24C);
    sequence = (sequence - 1) & 0xFF;
    volume = (127 * volume) >> 12;
    p2_sdk_80075d58((uint8)sequence, (sint16)volume, (sint16)volume);
    previous = r_u16(0x8008A530);
    current = r_u16(0x8008A3B0);
    w_u16(0x8008A3AC, previous);
    w_u16(0x8008A530, current);
    p2_func_80066404();
}

uint32 p2_func_8004fa54(uint32 object_offset, uint32 vector_offset)
{
    uint32 saved_stack, vector, parent, matrix, flags, x, y, z, frame, forward, reverse, value, clock, mode, result;
    saved_stack = p2_guest_stack_pointer;
    p2_guest_stack_pointer -= 16;
    vector = p2_guest_stack_pointer;
    p2_func_80046228(object_offset, 0);
    w_u32(object_offset + 28, (r_u32(object_offset + 28) & 0xFFFFF7FF) | 0x2000);
    parent = r_u32(object_offset + 132);
    matrix = parent ? r_u32(parent + 40) : 0;
    if (matrix)
    {
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
        p2_sdk_8007ec64(vector, object_offset + 16);
        PopMatrix();
        matrix = r_u32(r_u32(object_offset + 132) + 40);
        x = r_u32(object_offset + 16);
        y = r_u32(matrix + 20);
        parent = r_u32(object_offset + 132);
        w_u32(object_offset + 16, x + y);
        matrix = r_u32(parent + 40);
        x = r_u32(object_offset + 20);
        y = r_u32(matrix + 24);
        parent = r_u32(object_offset + 132);
        w_u32(object_offset + 20, x + y);
        matrix = r_u32(parent + 40);
        x = r_u32(object_offset + 24);
        y = r_u32(matrix + 28);
        w_u32(object_offset + 24, x + y);
    }
    else
    {
        x = r_u32(object_offset + 16);
        y = r_u32(object_offset + 20);
        z = r_u32(object_offset + 24);
        w_u32(object_offset + 16, x);
        w_u32(object_offset + 20, y);
        w_u32(object_offset + 24, z);
    }
    frame = r_u16(object_offset + 136);
    flags = r_u32(object_offset + 28);
    forward = (flags >> 4) & 1;
    reverse = (frame >> 15) ^ forward;
    p2_func_80022ec4(r_u32(object_offset + 132), 0x800ABDB0, (sint32)(reverse << 4));
    if (forward != reverse)
    {
        x = r_u32(0x800ABDB0);
        z = r_u32(0x800ABDB8);
        w_u32(0x800ABDB0, 0 - x);
        w_u32(0x800ABDB8, 0 - z);
    }
    flags = r_u32(object_offset + 28);
    if (flags & 0x400)
    {
        x = r_u32(0x800ABDB0);
        w_u32(0x800ABDB4, 0);
        if (x + 26754 > 0xD103 || r_u32(0x800ABDB8) + 26754 > 0xD103)
        {
            for (;;)
            {
                x = r_u32(0x800ABDB0);
                z = r_u32(0x800ABDB8);
                w_u32(0x800ABDB0, (uint32)((sint32)x >> 1));
                y = r_u32(0x800ABDB4);
                x = r_u32(0x800ABDB0);
                z = (uint32)((sint32)z >> 1);
                w_u32(0x800ABDB8, z);
                w_u32(0x800ABDB4, (uint32)((sint32)y >> 1));
                if (x + 26754 > 0xD103)
                    continue;
                if (r_u32(0x800ABDB4) + 26754 > 0xD103)
                    continue;
                if (z + 26754 <= 0xD103)
                    break;
            }
        }
        result = (uint32)VectorNormal((VECTOR *)psx_addr(0x800ABDB0, 12), (VECTOR *)psx_addr(0x800ABDB0, 12));
    }
    else
    {
        w_u32(object_offset + 28, flags | 0x400);
        clock = r_u32(0x800AC068);
        value = r_u32(0x800ABD68);
        mode = r_u16(0x800AC02A) & 15;
        w_u32(0x800ABD88, 0xFFFFFFFF);
        w_u32(0x800ABD84, 0);
        w_u16(0x800ABD98, 0);
        w_u16(0x800AC014, clock);
        w_u32(0x800ABD68, value | 0x08000000);
        result = 0;
        if (mode == 1)
            result = r_u32(object_offset + 28) & 0x04000000;
        if (!result)
        {
            value = r_u32(0x800ABD6C);
            result = 6;
            if (!(value & 0x80))
                w_u8(0x800AC030, 6);
        }
    }
    if (vector_offset)
    {
        result = r_u32(0x800ABDB0);
        y = r_u32(0x800ABDB4);
        z = r_u32(0x800ABDB8);
        w_u32(vector_offset, result);
        w_u32(vector_offset + 4, y);
        w_u32(vector_offset + 8, z);
    }
    p2_guest_stack_pointer = saved_stack;
    return result;
}

uint32 p2_func_80058ca0(uint32 object_offset, uint32 character_offset)
{
    uint32 flags, object_flags, value, character_flags, scene, result;
    flags = r_u32(object_offset + 228);
    object_flags = r_u32(object_offset + 28);
    w_u32(object_offset + 228, (flags & 0xFFFFFFB0) | 0x42);
    w_u32(object_offset + 28, object_flags & 0xFFFFFBFF);
    value = r_u32(character_offset + 200);
    character_flags = r_u32(character_offset + 4);
    w_u8(character_offset + 240, 0);
    w_u8(character_offset + 242, 0);
    w_u8(character_offset + 241, 0);
    w_u32(character_offset + 204, value);
    w_u32(character_offset + 4, character_flags | 4);
    flags = r_u32(object_offset + 228);
    scene = r_u32(0x8008A114);
    w_u32(object_offset + 228, flags | 12);
    w_u32(character_offset + 272, 0);
    result = r_u32(scene + 136);
    w_u32(character_offset + 248, result);
    return result;
}

uint32 p2_func_80058f50(uint32 object_offset, uint32 character_offset, uint32 input_flags, uint32 mode, uint32 horizontal_offset, uint32 vertical_offset)
{
    uint32 flags, result, pointer, delta, value, limit, count;
    sint32 coefficient, scale;
    sint64 product;
    flags = r_u32(object_offset + 228);
    if (!(flags & 4))
    {
        result = mode & 8;
        if (result)
            return result;
        if (!(r_u32(character_offset) & 0x40000))
        {
            delta = 0;
            if (input_flags & 0x800)
            {
                pointer = r_u32(character_offset + 780);
                coefficient = r_s32(pointer + 68);
                scale = r_s32(character_offset + 740);
                delta = (uint32)(((sint64)scale * coefficient) >> 12);
            }
            else if (input_flags & 0x1000)
            {
                pointer = r_u32(character_offset + 780);
                scale = r_s32(character_offset + 740);
                coefficient = r_s32(pointer + 72);
                delta = 0 - (uint32)(((sint64)scale * coefficient) >> 12);
            }
            result = r_u32(character_offset + 28) + delta;
            w_u32(character_offset + 28, result);
            w_u32(vertical_offset, result);
        }
        result = (input_flags & 0x800) ? 10 : ((input_flags & 0x1000) ? 12 : 11);
        if (!(r_u32(character_offset + 4) & 0x80))
            w_u8(character_offset + 712, result);
        return result;
    }
    result = r_u32(character_offset) & 0x40000;
    if (result)
        return result;
    limit = r_u8(0x8008A233);
    count = r_u32(character_offset + 272);
    result = input_flags & 0x30;
    if (count >= limit)
        return result;
    coefficient = r_s32((input_flags & 0x30) ? 0x8008A224 : 0x8008A220);
    scale = r_s32(character_offset + 740);
    product = (sint64)coefficient * scale;
    delta = (uint32)(product >> 12);
    result = r_u32(character_offset + 24) + delta;
    w_u32(character_offset + 24, result);
    w_u32(horizontal_offset, result);
    coefficient = r_s32(0x8008A228);
    scale = r_s32(character_offset + 740);
    product = (sint64)coefficient * scale;
    value = (uint32)(product >> 12);
    result = r_u32(character_offset + 28);
    if ((sint32)value >= (sint32)result)
        return result;
    coefficient = r_s32((input_flags & 0x30) ? 0x8008A22C : 0x8008A228);
    scale = r_s32(character_offset + 740);
    product = (sint64)coefficient * scale;
    value = (uint32)(product >> 12);
    result = (uint32)((uint64)product >> 32) << 20;
    w_u32(character_offset + 28, value);
    w_u32(vertical_offset, value);
    return result;
}

uint32 p2_func_8005873c(uint32 object_offset)
{
    uint32 saved_stack, world, first, second, delta, sample, destination, temporary, relative, inverse;
    uint32 flags, value, scene, selected, route, parent, matrix, x, y, z, a, b, c, result;
    sint32 frame, previous, count;
    saved_stack = p2_guest_stack_pointer;
    p2_guest_stack_pointer -= 160;
    world = p2_guest_stack_pointer;
    first = world + 16;
    second = world + 32;
    delta = world + 48;
    sample = world + 64;
    destination = world + 80;
    temporary = world + 96;
    relative = world + 112;
    inverse = world + 128;
    w_u32(0x800ABD68, r_u32(0x800ABD68) & 0xF7FFFFFF);
    w_u32(object_offset + 228, r_u32(object_offset + 228) | 1);
    flags = r_u32(object_offset + 28);
    value = r_u32(object_offset + 228);
    w_u32(object_offset + 28, flags & 0xFFFFFBFF);
    w_u32(object_offset + 228, value & 0xFFFFFFB1);
    value = r_u32(0x800ABE30);
    scene = r_u32(0x8008A114);
    w_u8(0x800ABE58, 0);
    w_u8(0x800ABE5B, 0);
    w_u8(0x800ABE70, 0);
    w_u32(0x800ABE34, value);
    w_u32(0x800ABE74, r_u32(object_offset + 28));
    selected = r_u32(scene + 136);
    route = r_u32(selected + 20);
    w_u32(0x8008A490, 0);
    w_u8(0x8008A524, 0);
    w_u32(0x8008A40C, 0);
    w_u32(0x8008A414, 0);
    w_u32(0x8008A464, 0);
    w_u32(0x8008A458, 0);
    parent = r_u32(object_offset + 132);
    matrix = parent ? r_u32(parent + 40) : 0;
    if (matrix)
    {
        x = r_u32(object_offset + 16);
        y = r_u32(object_offset + 20);
        z = r_u32(object_offset + 24);
        w_u32(temporary, x);
        w_u32(temporary + 4, y);
        w_u32(temporary + 8, z);
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
        p2_sdk_8007ec64(temporary, world);
        PopMatrix();
        matrix = r_u32(r_u32(object_offset + 132) + 40);
        x = r_u32(world);
        y = r_u32(matrix + 20);
        w_u32(world, x + y);
        matrix = r_u32(r_u32(object_offset + 132) + 40);
        x = r_u32(world + 4);
        y = r_u32(matrix + 24);
        w_u32(world + 4, x + y);
        matrix = r_u32(r_u32(object_offset + 132) + 40);
        x = r_u32(world + 8);
        y = r_u32(matrix + 28);
        w_u32(world + 8, x + y);
    }
    else
    {
        x = r_u32(object_offset + 16);
        y = r_u32(object_offset + 20);
        z = r_u32(object_offset + 24);
        w_u32(world, x);
        w_u32(world + 4, y);
        w_u32(world + 8, z);
    }
    frame = (sint32)p2_func_800231dc(r_u32(route + 36), world, 64, -40960);
    if (frame != 0)
    {
        count = r_s16(r_u32(route + 36) + 26);
        value = (uint32)count - (uint32)frame - 1;
        if ((uint32)frame != value)
            frame = (sint32)((uint32)frame + 1);
        previous = (sint32)((uint32)frame - 1);
    }
    else
    {
        count = r_s16(r_u32(route + 36) + 26);
        frame = count - 1;
        previous = count - 2;
    }
    p2_func_80022b20(route, previous, first, 64);
    p2_func_80022b20(route, frame, second, 64);
    x = r_u32(second);
    a = r_u32(first);
    y = r_u32(second + 4);
    b = r_u32(first + 4);
    z = r_u32(second + 8);
    c = r_u32(first + 8);
    x -= a;
    y -= b;
    z -= c;
    w_u32(delta, x);
    w_u32(delta + 4, y);
    w_u32(delta + 8, z);
    value = p2_func_8005dc00((sint32)(0 - z), (sint32)(0 - x));
    scene = r_u32(0x8008A114);
    selected = r_u32(scene + 136);
    w_u16(0x800ABE3C, value);
    w_u16(0x800ABE72, value);
    w_u32(0x800ABE38, selected);
    selected = r_u32(scene + 136);
    p2_func_80022b20(selected, 0, sample, 64);
    scene = r_u32(0x8008A114);
    parent = r_u32(r_u32(scene + 136) + 36);
    matrix = parent ? r_u32(parent + 40) : 0;
    if (matrix)
    {
        x = r_u32(sample);
        y = r_u32(matrix + 20);
        w_u32(relative, x - y);
        matrix = r_u32(r_u32(r_u32(scene + 136) + 36) + 40);
        x = r_u32(sample + 4);
        y = r_u32(matrix + 24);
        w_u32(relative + 4, x - y);
        matrix = r_u32(r_u32(r_u32(scene + 136) + 36) + 40);
        x = r_u32(sample + 8);
        y = r_u32(matrix + 28);
        w_u32(relative + 8, x - y);
        matrix = r_u32(r_u32(r_u32(scene + 136) + 36) + 40);
        w_u16(inverse, r_u16(matrix));
        matrix = r_u32(r_u32(r_u32(scene + 136) + 36) + 40);
        w_u16(inverse + 2, r_u16(matrix + 6));
        matrix = r_u32(r_u32(r_u32(scene + 136) + 36) + 40);
        w_u16(inverse + 4, r_u16(matrix + 12));
        matrix = r_u32(r_u32(r_u32(scene + 136) + 36) + 40);
        w_u16(inverse + 6, r_u16(matrix + 2));
        matrix = r_u32(r_u32(r_u32(scene + 136) + 36) + 40);
        w_u16(inverse + 8, r_u16(matrix + 8));
        matrix = r_u32(r_u32(r_u32(scene + 136) + 36) + 40);
        w_u16(inverse + 10, r_u16(matrix + 14));
        matrix = r_u32(r_u32(r_u32(scene + 136) + 36) + 40);
        w_u16(inverse + 12, r_u16(matrix + 4));
        matrix = r_u32(r_u32(r_u32(scene + 136) + 36) + 40);
        w_u16(inverse + 14, r_u16(matrix + 10));
        matrix = r_u32(r_u32(r_u32(scene + 136) + 36) + 40);
        w_u16(inverse + 16, r_u16(matrix + 16));
        PushMatrix();
        x = r_u32(inverse);
        y = r_u32(inverse + 4);
        xport_gte_write_control(0, x);
        xport_gte_write_control(1, y);
        x = r_u32(inverse + 8);
        y = r_u32(inverse + 12);
        z = r_u16(inverse + 16);
        xport_gte_write_control(2, x);
        xport_gte_write_control(3, y);
        xport_gte_write_control(4, z);
        p2_sdk_8007ec64(relative, destination);
        PopMatrix();
    }
    else
    {
        x = r_u32(sample);
        y = r_u32(sample + 4);
        z = r_u32(sample + 8);
        w_u32(destination, x);
        w_u32(destination + 4, y);
        w_u32(destination + 8, z);
    }
    w_u32(object_offset + 16, r_u32(destination));
    result = r_u32(destination + 8);
    w_u32(object_offset + 24, result);
    p2_guest_stack_pointer = saved_stack;
    return result;
}

uint32 p2_func_8005b588(uint32 object_offset, uint32 character_offset)
{
    uint32 result, limit, count;
    if (!r_u32(character_offset + 272))
        w_u16(character_offset + 58, 0xFFFF);
    result = r_u16(character_offset + 58);
    limit = r_u8(0x8008A230);
    count = r_u32(character_offset + 272);
    ++result;
    w_u16(character_offset + 58, result);
    if (count >= limit)
    {
        w_u32(object_offset + 228, r_u32(object_offset + 228) & 0xFFFFFFB1);
        w_u32(character_offset + 4, r_u32(character_offset + 4) & 0xFFFFFFFB);
        w_u32(object_offset + 28, r_u32(object_offset + 28) | 0x400);
        result = r_u32(character_offset) | 0x2000;
        w_u32(character_offset, result);
    }
    return result;
}

uint32 p2_func_800585e0(uint32 object_offset)
{
    uint32 flags, scene, result;
    flags = r_u32(object_offset + 228);
    scene = r_u32(0x8008A114);
    w_u32(object_offset + 228, (flags | 0x10) & 0xFFFFFF91);
    w_u8(0x800ABE5C, 0);
    result = r_u32(scene + 136);
    w_u32(0x800ABE60, result);
    return result;
}

uint32 p2_func_80058618(uint32 object_offset)
{
    uint32 result;
    result = r_u32(object_offset + 228) & 0xFFFFFFEF;
    w_u32(object_offset + 228, result);
    w_u8(0x800ABE5C, 0);
    w_u32(0x800ABE78, 0);
    return result;
}

uint32 p2_func_80058640(uint32 object_offset)
{
    uint32 flags, scene, path;
    sint32 mode, first, second;
    w_u32(object_offset + 228, r_u32(object_offset + 228) & 0xFFFFFFEF);
    w_u8(0x800ABE5C, 0);
    w_u32(0x800ABE78, 0);
    flags = r_u32(object_offset + 228);
    scene = r_u32(0x8008A114);
    w_u32(object_offset + 228, (flags | 0x68) & 0xFFFF7FFF);
    path = r_u32(scene + 136);
    mode = r_s8(0x8008A210);
    w_u8(0x800ABE5C, 0);
    w_u32(0x800ABE60, path);
    if (mode == 1)
    {
        first = r_s32(0x800AC04C);
        second = r_s32(0x8008A218);
        w_u32(0x800ABD84, (uint32)(((sint64)second * first) >> 12));
    }
    w_u32(0x800ABD88, 0xFFFFFFFF);
    return 0xFFFFFFFF;
}

uint32 p2_func_800586e8(uint32 object_offset)
{
    uint32 result;
    result = r_u32(object_offset + 228) & 0xFFFF7F97;
    w_u32(object_offset + 228, result);
    w_u32(0x800ABE60, 0);
    w_u8(0x800ABE5C, 0);
    return result;
}

uint32 p2_func_8003ac94(uint32 path_offset, uint32 frame_offset, uint32 direction)
{
    uint32 forward, selected, definition;
    forward = r_s32(frame_offset) >= 0;
    if (!forward)
    {
        if (r_u16(path_offset + 14) & 8)
            path_offset = r_u32(path_offset + 16);
        direction ^= 0x10;
    }
    if (r_u16(path_offset + 14) & (direction | 8))
        selected = r_u32(path_offset + 20);
    else
        selected = r_u32(path_offset + 16);
    if (selected)
    {
        if (forward)
            w_u32(frame_offset, 0);
        else
        {
            if (r_u16(selected + 14) & 8)
                selected = r_u32(selected + 16);
            definition = r_u32(selected + 36);
            w_u32(frame_offset, (uint32)(sint32)r_s16(definition + 26) - 1);
        }
    }
    return selected;
}

uint32 p2_func_8005a430(void)
{
    return r_u32(0x800ABE38);
}

uint32 p2_func_8005a3fc(void)
{
    uint32 scene, selected, path;
    scene = r_u32(0x8008A114);
    selected = r_u32(0x800ABE60);
    path = r_u32(scene + 136);
    return selected == path;
}

uint32 p2_func_8003d190(uint32 object_offset, uint32 position_offset, uint32 direction)
{
    return (uint32)p2_guard_path_change_with_selection(object_offset, position_offset, direction, 0).value;
}

P2PathProbeResult p2_guard_path_change_with_selection(uint32 object_offset, uint32 position_offset, uint32 direction, uint32 fallback_frame)
{
    P2PathProbeResult output, probe;
    uint32 scene, flags, path, saved_stack, floor, x, y, z, result;
    sint32 frame;
    scene = r_u32(0x8008A114);
    flags = r_u32(object_offset + 228);
    path = r_u32(scene + 136);
    if ((flags & 6) == 2)
    {
        output.value = 0;
        output.fallback_frame = fallback_frame;
        return output;
    }
    saved_stack = p2_guest_stack_pointer;
    p2_guest_stack_pointer -= 16;
    floor = p2_guest_stack_pointer;
    if ((flags & 4) && p2_func_8005a430() != path)
    {
        probe = p2_query_path_height_with_selection(r_u32(path + 36), position_offset, direction, -40960, fallback_frame);
        fallback_frame = probe.fallback_frame;
        frame = probe.value;
        if (frame != -1)
        {
            probe = p2_project_path_with_selection(r_u32(path + 36), frame, position_offset, floor, direction, 4096);
            fallback_frame = probe.fallback_frame;
            frame = probe.value;
        }
        if (frame >= 0)
        {
            x = r_u32(floor);
            x -= r_u32(position_offset);
            if (x + 24576 > 0xC000)
                frame = -1;
            else
            {
                z = r_u32(floor + 8);
                z -= r_u32(position_offset + 8);
                if (z + 24576 > 0xC000)
                    frame = -1;
            }
        }
        if (frame != -1)
        {
            x = r_u32(floor);
            x -= r_u32(position_offset);
            y = r_u32(floor + 4);
            w_u32(floor, x);
            y -= r_u32(position_offset + 4);
            z = r_u32(floor + 8);
            w_u32(floor + 4, y);
            z -= r_u32(position_offset + 8);
            w_u32(floor + 8, z);
            if ((sint32)y <= 0)
            {
                p2_guest_stack_pointer = saved_stack;
                output.value = 0;
                output.fallback_frame = fallback_frame;
                return output;
            }
        }
    }
    result = 1;
    if ((r_u32(object_offset + 228) & 0x20) && (p2_func_8005a3fc() & 0xFF) == 0)
    {
        probe = p2_query_path_height_with_selection(r_u32(path + 36), position_offset, direction, -40960, fallback_frame);
        fallback_frame = probe.fallback_frame;
        frame = probe.value;
        if (frame != -1)
        {
            probe = p2_project_path_with_selection(r_u32(path + 36), frame, position_offset, floor, direction, 4096);
            fallback_frame = probe.fallback_frame;
            frame = probe.value;
        }
        if (frame >= 0)
        {
            x = r_u32(floor);
            x -= r_u32(position_offset);
            if (x + 24576 > 0xC000)
                frame = -1;
            else
            {
                z = r_u32(floor + 8);
                z -= r_u32(position_offset + 8);
                if (z + 24576 > 0xC000)
                    frame = -1;
            }
        }
        if (frame != -1)
        {
            x = r_u32(floor);
            x -= r_u32(position_offset);
            y = r_u32(floor + 4);
            w_u32(floor, x);
            y -= r_u32(position_offset + 4);
            z = r_u32(floor + 8);
            w_u32(floor + 4, y);
            z -= r_u32(position_offset + 8);
            w_u32(floor + 8, z);
            if ((sint32)y <= 0)
                result = 0;
        }
    }
    p2_guest_stack_pointer = saved_stack;
    output.value = (sint32)result;
    output.fallback_frame = fallback_frame;
    return output;
}

uint32 p2_func_8002995c(uint32 object_offset)
{
    uint32 owner, pointer, state, animation, value, flags, x, y, z, parent, matrix, result;
    uint32 saved_stack, world, local, relative, inverse;
    sint32 frame, previous, length, timer;
    pointer = r_u32(object_offset + 112);
    owner = pointer ? r_u32(pointer + 4) : p2_func_800536f0(0);
    state = r_u32(object_offset + 116);
    if (state == 9)
        return p2_func_80028720(object_offset, owner);
    if (state == 12)
        return p2_func_800289d4(object_offset, owner);
    saved_stack = p2_guest_stack_pointer;
    p2_guest_stack_pointer -= 80;
    world = p2_guest_stack_pointer;
    local = world + 16;
    relative = world + 32;
    inverse = world + 48;
    if (state == 13 || state == 14)
    {
        pointer = p2_func_800536f0(0);
        animation = r_u32(object_offset + 104);
        frame = 0;
        if (animation)
        {
            previous = r_s16(object_offset + 130);
            length = r_s16(animation + 6);
            if (previous < length - 1)
                frame = previous + 1;
        }
        value = r_u32(0x8008A510);
        w_u16(object_offset + 130, (uint32)frame);
        if (value)
            p2_func_800296a4(object_offset, pointer, 0x800ABD68, state);
        goto finish_8002995c;
    }
    if (state == 11)
    {
        animation = r_u32(object_offset + 104);
        frame = 0;
        if (animation)
        {
            previous = r_s16(object_offset + 130);
            length = r_s16(animation + 6);
            if (previous < length - 1)
                frame = previous + 1;
        }
        value = r_u32(object_offset + 100);
        w_u16(object_offset + 130, (uint32)frame);
        ++value;
        w_u32(object_offset + 100, value);
        if ((sint32)value >= 4)
            goto expire_8002995c;
        parent = r_u32(owner + 132);
        matrix = parent ? r_u32(parent + 40) : 0;
        if (matrix)
        {
            x = r_u32(owner + 16);
            y = r_u32(owner + 20);
            z = r_u32(owner + 24);
            w_u32(local, x);
            w_u32(local + 4, y);
            w_u32(local + 8, z);
            PushMatrix();
            matrix = r_u32(r_u32(owner + 132) + 40);
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
            matrix = r_u32(r_u32(owner + 132) + 40);
            x = r_u32(world);
            y = r_u32(matrix + 20);
            w_u32(world, x + y);
            matrix = r_u32(r_u32(owner + 132) + 40);
            x = r_u32(world + 4);
            y = r_u32(matrix + 24);
            w_u32(world + 4, x + y);
            matrix = r_u32(r_u32(owner + 132) + 40);
            x = r_u32(world + 8);
            y = r_u32(matrix + 28);
            w_u32(world + 8, x + y);
        }
        else
        {
            x = r_u32(owner + 16);
            y = r_u32(owner + 20);
            z = r_u32(owner + 24);
            w_u32(world, x);
            w_u32(world + 4, y);
            w_u32(world + 8, z);
        }
        parent = r_u32(object_offset + 132);
        matrix = parent ? r_u32(parent + 40) : 0;
        if (matrix)
        {
            x = r_u32(world);
            y = r_u32(matrix + 20);
            w_u32(relative, x - y);
            matrix = r_u32(r_u32(object_offset + 132) + 40);
            x = r_u32(world + 4);
            y = r_u32(matrix + 24);
            w_u32(relative + 4, x - y);
            matrix = r_u32(r_u32(object_offset + 132) + 40);
            x = r_u32(world + 8);
            y = r_u32(matrix + 28);
            w_u32(relative + 8, x - y);
            matrix = r_u32(r_u32(object_offset + 132) + 40);
            w_u16(inverse, r_u16(matrix));
            matrix = r_u32(r_u32(object_offset + 132) + 40);
            w_u16(inverse + 2, r_u16(matrix + 6));
            matrix = r_u32(r_u32(object_offset + 132) + 40);
            w_u16(inverse + 4, r_u16(matrix + 12));
            matrix = r_u32(r_u32(object_offset + 132) + 40);
            w_u16(inverse + 6, r_u16(matrix + 2));
            matrix = r_u32(r_u32(object_offset + 132) + 40);
            w_u16(inverse + 8, r_u16(matrix + 8));
            matrix = r_u32(r_u32(object_offset + 132) + 40);
            w_u16(inverse + 10, r_u16(matrix + 14));
            matrix = r_u32(r_u32(object_offset + 132) + 40);
            w_u16(inverse + 12, r_u16(matrix + 4));
            matrix = r_u32(r_u32(object_offset + 132) + 40);
            w_u16(inverse + 14, r_u16(matrix + 10));
            matrix = r_u32(r_u32(object_offset + 132) + 40);
            w_u16(inverse + 16, r_u16(matrix + 16));
            PushMatrix();
            x = r_u32(inverse);
            y = r_u32(inverse + 4);
            xport_gte_write_control(0, x);
            xport_gte_write_control(1, y);
            x = r_u32(inverse + 8);
            y = r_u32(inverse + 12);
            z = r_u16(inverse + 16);
            xport_gte_write_control(2, x);
            xport_gte_write_control(3, y);
            xport_gte_write_control(4, z);
            p2_sdk_8007ec64(relative, object_offset + 16);
            PopMatrix();
        }
        else
        {
            x = r_u32(world);
            y = r_u32(world + 4);
            z = r_u32(world + 8);
            w_u32(object_offset + 16, x);
            w_u32(object_offset + 20, y);
            w_u32(object_offset + 24, z);
        }
        goto finish_8002995c;
    }
    animation = r_u32(object_offset + 104);
    frame = 0;
    if (animation)
    {
        length = r_s16(animation + 6);
        previous = r_s16(object_offset + 130);
        if (previous < length - 1)
            frame = previous + 1;
    }
    state = r_u32(object_offset + 116);
    if (state == 2)
    {
        x = r_u32(object_offset + 16);
        value = r_u32(object_offset + 92);
        y = r_u32(object_offset + 20);
        flags = r_u32(object_offset + 96);
        z = r_u32(object_offset + 24);
        pointer = r_u32(object_offset + 100);
        w_u32(object_offset + 16, x + value);
        w_u32(object_offset + 20, y + flags);
        w_u32(object_offset + 24, z + pointer);
    }
    else if (state == 8 && frame == 15)
        p2_func_80044ffc(11, 3);
    if ((r_u32(object_offset + 28) & 0x8000) && owner)
    {
        w_u16(object_offset + 14, r_u16(owner + 14) + 64);
        matrix = 0;
        if (!(r_u32(owner + 28) & 0x2000))
        {
            parent = r_u32(owner + 132);
            if (parent)
                matrix = r_u32(parent + 40);
        }
        if (matrix)
        {
            x = r_u32(owner + 16);
            y = r_u32(owner + 20);
            z = r_u32(owner + 24);
            w_u32(local, x);
            w_u32(local + 4, y);
            w_u32(local + 8, z);
            PushMatrix();
            matrix = r_u32(r_u32(owner + 132) + 40);
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
            p2_sdk_8007ec64(local, object_offset + 16);
            PopMatrix();
            matrix = r_u32(r_u32(owner + 132) + 40);
            x = r_u32(object_offset + 16);
            y = r_u32(matrix + 20);
            w_u32(object_offset + 16, x + y);
            matrix = r_u32(r_u32(owner + 132) + 40);
            x = r_u32(object_offset + 20);
            y = r_u32(matrix + 24);
            w_u32(object_offset + 20, x + y);
            matrix = r_u32(r_u32(owner + 132) + 40);
            x = r_u32(object_offset + 24);
            y = r_u32(matrix + 28);
            w_u32(object_offset + 24, x + y);
        }
        else
        {
            x = r_u32(owner + 16);
            y = r_u32(owner + 20);
            z = r_u32(owner + 24);
            w_u32(object_offset + 16, x);
            w_u32(object_offset + 20, y);
            w_u32(object_offset + 24, z);
        }
        x = r_u32(object_offset + 20);
        y = r_u32(owner + 76);
        pointer = r_u32(object_offset + 112);
        w_u32(object_offset + 20, x + y);
        if (!pointer)
        {
            flags = r_u32(owner + 228);
            if (flags & 1)
                p2_func_800596cc(owner, object_offset);
            else if (flags & 0x100)
                p2_func_80059284(owner, object_offset);
        }
        if ((r_u8(owner + 13) & 1) || (!r_u32(object_offset + 112) && (r_u16(0x800AC02A) & 15) >= 2 && r_u32(object_offset + 116) - 3 < 2))
            w_u8(object_offset + 13, r_u8(object_offset + 13) | 1);
        else
            w_u8(object_offset + 13, r_u8(object_offset + 13) & 0xFE);
    }
    else if (frame < r_s16(object_offset + 130))
        goto expire_8002995c;
    state = r_u32(object_offset + 116);
    w_u16(object_offset + 130, (uint32)frame);
    if (state == 7)
    {
        timer = r_s16(0x800ABDA8);
        if (timer < 64)
        {
            value = r_u8(object_offset + 13);
            w_u8(object_offset + 13, (timer & 4) ? value & 0xFE : value | 1);
        }
    }
finish_8002995c:
    result = r_u8(object_offset + 13) & 1;
    if (!result)
        result = p2_func_8001a6ac(object_offset);
    p2_guest_stack_pointer = saved_stack;
    return result;
expire_8002995c:
    value = r_u8(object_offset + 13);
    w_u8(object_offset + 125, 0);
    result = value | 2;
    w_u8(object_offset + 13, result);
    p2_guest_stack_pointer = saved_stack;
    return result;
}

uint32 p2_func_80056328(void)
{
    uint32 limit, count;
    limit = r_u8(0x8008A230);
    count = r_u32(0x800ABE78);
    return count < limit;
}

uint32 p2_func_8004df14(uint32 object_offset, uint32 character_offset)
{
    uint32 result, saved_stack, vector, parent, matrix, x, y, z;
    result = r_u32(object_offset + 28) & 0x2000;
    if (result)
        return result;
    saved_stack = p2_guest_stack_pointer;
    p2_guest_stack_pointer -= 16;
    vector = p2_guest_stack_pointer;
    parent = r_u32(object_offset + 132);
    matrix = parent ? r_u32(parent + 40) : 0;
    if (matrix)
    {
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
        p2_sdk_8007ec64(vector, object_offset + 16);
        PopMatrix();
        matrix = r_u32(r_u32(object_offset + 132) + 40);
        x = r_u32(object_offset + 16);
        y = r_u32(matrix + 20);
        parent = r_u32(object_offset + 132);
        w_u32(object_offset + 16, x + y);
        matrix = r_u32(parent + 40);
        x = r_u32(object_offset + 20);
        y = r_u32(matrix + 24);
        parent = r_u32(object_offset + 132);
        w_u32(object_offset + 20, x + y);
        matrix = r_u32(parent + 40);
        x = r_u32(object_offset + 24);
        y = r_u32(matrix + 28);
        w_u32(object_offset + 24, x + y);
    }
    else
    {
        x = r_u32(object_offset + 16);
        y = r_u32(object_offset + 20);
        z = r_u32(object_offset + 24);
        w_u32(object_offset + 16, x);
        w_u32(object_offset + 20, y);
        w_u32(object_offset + 24, z);
    }
    w_u32(object_offset + 28, r_u32(object_offset + 28) | 0x2400);
    x = r_u32(character_offset + 692);
    y = r_u32(character_offset + 696);
    z = r_u32(character_offset + 700);
    w_u32(character_offset + 72, 0 - x);
    w_u32(character_offset + 76, 0 - y);
    w_u32(character_offset + 80, 0 - z);
    w_u32(character_offset + 76, 0);
    if (r_u32(character_offset + 72) + 26754 > 0xD103 || r_u32(character_offset + 80) + 26754 > 0xD103)
    {
        for (;;)
        {
            x = r_u32(character_offset + 72);
            z = r_u32(character_offset + 80);
            w_u32(character_offset + 72, (uint32)((sint32)x >> 1));
            y = r_u32(character_offset + 76);
            x = r_u32(character_offset + 72);
            z = (uint32)((sint32)z >> 1);
            w_u32(character_offset + 80, z);
            w_u32(character_offset + 76, (uint32)((sint32)y >> 1));
            if (x + 26754 > 0xD103)
                continue;
            if (r_u32(character_offset + 76) + 26754 > 0xD103)
                continue;
            if (z + 26754 <= 0xD103)
                break;
        }
    }
    result = (uint32)VectorNormal((VECTOR *)psx_addr(character_offset + 72, 12), (VECTOR *)psx_addr(character_offset + 72, 12));
    p2_guest_stack_pointer = saved_stack;
    return result;
}

uint32 p2_func_80032ddc(uint32 object_offset, uint32 other_offset, uint32 contact_offset)
{
    uint32 value, flags, data, contact_distance, category, resource, movement, angle, x, z, timeout;
    sint32 kind, variant, result, auxiliary;
    if (!contact_offset)
    {
        category = r_u8(other_offset + 12);
        if (category)
        {
            if (category != 2)
                return 0;
            kind = r_s8(other_offset + 15);
            if (kind == 13)
            {
                value = r_u32(0x8008A518);
                timeout = r_u32(other_offset + 68);
                if (value >= timeout)
                    return 1;
            }
            if (kind == 14 || (uint8)(kind - 22) < 2)
                return 1;
            if ((uint8)(kind - 40) >= 16 || (r_u8(other_offset + 13) & 0x80))
                return 0;
            if (r_u8(other_offset + 71) & 0xFE)
                return 0;
            return ((r_u32(other_offset + 72) >> 12) & 31) == 2;
        }
        kind = r_s8(other_offset + 126);
        if (kind == 3)
        {
            variant = r_s8(other_offset + 120);
            if (variant == -2)
                return 0;
            data = r_u32(r_u32(object_offset + 132) + 44);
            if (r_s8(data + 37) < 47 || variant != -1)
                return 1;
        }
        if (kind == 1)
        {
            if (r_s8(object_offset + 125) != 24)
                return 1;
            if (r_u32(r_u32(object_offset + 132) + 16) & 0x40)
                return 1;
        }
        if (kind != 2 || r_s8(other_offset + 125) != 19)
            return 0;
        if (r_s8(object_offset + 125) == 19)
            return 0;
        return (r_u32(other_offset + 28) & 0x40) != 0;
    }
    data = r_u32(r_u32(object_offset + 132) + 44);
    category = r_u8(other_offset + 12);
    contact_distance = r_u32(contact_offset + 4);
    if (category)
    {
        kind = r_s8(other_offset + 15);
        if (kind == 13)
        {
            flags = r_u32(data + 44) | 8;
            w_u32(data + 44, flags);
            value = r_u32(other_offset + 76);
            w_u32(data + 44, (flags & 0xFFFF01FF) | ((value & 0x7F) << 9));
            timeout = r_u16(other_offset + 78);
            value = r_u32(0x8008A518);
            w_u32(other_offset + 68, value + timeout);
        }
        else if (kind == 14)
        {
            flags = r_u32(data + 44) | 0x10;
            w_u32(data + 44, flags);
            return (flags & 0x80) == 0;
        }
        else if (kind == 22)
        {
            if (contact_distance)
                return 0;
            w_u32(object_offset + 28, r_u32(object_offset + 28) | 0x40000);
        }
        else if (kind == 23)
        {
            if (contact_distance)
                return 0;
            w_u16(object_offset + 14, r_u32(other_offset + 68));
        }
        else if (!contact_distance)
            w_u8(other_offset + 14, r_u8(other_offset + 14) | 8);
        else
        {
            if ((r_u32(data + 44) & 4) && (sint32)contact_distance >= (sint32)r_u16(data + 40))
                return 0;
            flags = r_u32(data + 44);
            w_u32(data + 52, other_offset);
            w_u16(data + 40, contact_distance);
            w_u32(data + 44, flags | 4);
        }
        return 0;
    }
    kind = r_s8(other_offset + 126);
    if (kind == 3)
    {
        resource = r_u32(other_offset + 116);
        if (resource == 19)
        {
            if ((r_u32(0x8008A178) & 0x20) && r_u16(r_u32(data + 72) + 624))
            {
                value = r_u8(data + 38);
                if (!(value & 2))
                {
                    w_u8(data + 38, value | 2);
                    w_u32(data + 76, 0);
                }
                w_u32(0x8008A17C, data);
                return 1;
            }
        }
        else if (resource == 11)
        {
            if (r_u32(object_offset + 28) & 0x30000000)
                return 1;
            w_u32(data + 44, r_u32(data + 44) | 1);
            return 1;
        }
        else if (r_s8(object_offset + 125) == 17 || (r_u32(object_offset + 28) & 0x30000000))
        {
            w_u32(data + 44, r_u32(data + 44) | 0x20);
            return 1;
        }
        movement = r_u32(object_offset + 28) & 0x05800000;
        if (movement != 0x01800000)
        {
            if (!movement || (movement & 0x04000000))
                goto mark_push_80032ddc;
            z = r_u32(other_offset + 100);
            x = r_u32(other_offset + 92);
            angle = p2_func_8005dc00((sint32)z, (sint32)x);
            value = r_u16(object_offset + 148) - angle;
            if (movement & 0x00800000)
            {
                if ((uint16)(value + 1024) >= 2049)
                    goto mark_push_80032ddc;
            }
            else if ((movement & 0x01000000) && (uint16)(value + 1023) < 2047)
                goto mark_push_80032ddc;
        }
        goto finish_contact_80032ddc;
    }
    if (kind != 1)
    {
        w_u32(data + 44, r_u32(data + 44) | 0x60);
        goto finish_contact_80032ddc;
    }
    if (r_s8(object_offset + 125) == 17 || (r_u32(object_offset + 28) & 0x10000000))
    {
        w_u32(data + 44, r_u32(data + 44) | 0x22);
        return 1;
    }
    if (!contact_distance && (r_u32(other_offset + 28) & 0x02000000))
        goto mark_contact_80032ddc;
    result = p2_func_80047b1c(other_offset);
    value = r_u32(other_offset + 228);
    auxiliary = 0;
    if (value & 1)
        auxiliary = p2_func_8005b604(other_offset, object_offset);
    if (result <= 0 && ((r_s8(object_offset + 125) == 20 && (r_u32(object_offset + 28) & 0x40)) || (r_u32(contact_offset) < 2048 && result == 0) || (r_u32(other_offset + 28) & 0x18000000) || (r_u32(object_offset + 28) & 0x04400000) == 0x00400000))
    {
        if (auxiliary && r_s8(object_offset + 125) != 20)
            w_u32(data + 44, r_u32(data + 44) | 8);
    }
    else
    {
        flags = r_u32(data + 44);
        w_u32(data + 44, flags | (result > 0 ? 0x20 : 8));
    }
    if (r_s8(0x800ABE7C) > 0)
        w_u32(data + 44, r_u32(data + 44) & 0xFFFFFFD7);
mark_contact_80032ddc:
    flags = r_u32(0x800ABD6C);
    if (flags & 0x40)
        w_u32(data + 44, r_u32(data + 44) | 0x20000);
    else if (flags & 0x200)
        w_u32(data + 44, (r_u32(data + 44) | 1) & 0xFFFFFFDF);
    w_u32(data + 44, r_u32(data + 44) | 2);
    goto finish_contact_80032ddc;
mark_push_80032ddc:
    w_u32(data + 44, r_u32(data + 44) | 8);
finish_contact_80032ddc:
    flags = r_u32(data + 44);
    if (flags & 8)
    {
        if ((r_u32(contact_offset + 12) & 1) || (r_s8(object_offset + 125) == 20 && !(r_u32(object_offset + 28) & 0x40)))
        {
            value = r_u8(data + 38);
            flags = r_u32(data + 44);
            flags = (value & 2) ? flags & 0xFFFF01F7 : (flags & 0xFFFF01FF) | 0x200;
        }
        else
            flags &= 0xFFFFFFF7;
        w_u32(data + 44, flags);
        if ((r_u32(data + 44) & 8) && !(r_u32(0x800ABD6C) & 0x40))
            w_u8(0x800ABE7C, 5);
    }
    return kind != 1;
}

uint32 p2_func_8002d7a4(uint32 kind)
{
    uint32 count, table, candidate;
    count = r_u32(0x8008A320);
    table = 0x8008B420;
    while (count != 0)
    {
        --count;
        candidate = r_u32(table);
        if (candidate != 0 && r_u8(candidate + 1) == kind)
            return (uint32)(sint32)r_s16(candidate + 2);
        table += 4;
    }
    return (uint32)(sint32)r_s16(2);
}

sint32 p2_func_800185c8(uint32 object_offset, uint32 vector_offset, uint32 axes_offset)
{
    uint32 saved_stack, transformed, quaternion, matrix, parent_quaternion, composed, parent, track, source;
    uint32 index, address, shift, mask, upper, lower, first_word, second_word, value, destination, old, result;
    uint32 xx, yy, zz, xy, xz, yz, wx, wy, wz, x, y, z;
    uint32 rotated, squared, magnitude, height;
    sint32 frame, q0, q1, q2, q3, p0, p1, p2, p3;
    rotated = 0;
    saved_stack = p2_guest_stack_pointer;
    p2_guest_stack_pointer -= 64;
    transformed = p2_guest_stack_pointer;
    quaternion = axes_offset != 0 ? axes_offset : transformed + 16;
    matrix = transformed + 24;
    parent_quaternion = transformed + 48;
    composed = transformed + 56;
    parent = r_u32(object_offset + 132);
    if (parent != 0 && (r_u32(parent + 16) & 8) != 0)
    {
        track = r_u32(r_u32(parent + 44) + 56);
        if (track != 0)
        {
            if ((r_u32(object_offset + 28) & 0x10) != 0)
                frame = r_s16(object_offset + 136);
            else
            {
                frame = r_s16(parent + 26);
                frame = (sint32)((uint32)frame - (uint32)(sint32)r_s16(object_offset + 136) - 1);
            }
            p2_func_80017938(frame, track, quaternion);
            parent = r_u32(r_u32(object_offset + 132) + 40);
            if (parent != 0)
            {
                p2_func_80017e4c(parent, parent_quaternion);
                q0 = r_s16(quaternion);
                source = 0;
                if (q0 == 0x4000)
                    source = parent_quaternion;
                else
                {
                    p0 = r_s16(parent_quaternion);
                    if (p0 == 0x4000)
                        source = quaternion;
                    else
                    {
                        q1 = r_s16(quaternion + 2);
                        p1 = r_s16(parent_quaternion + 2);
                        q2 = r_s16(quaternion + 4);
                        p2 = r_s16(parent_quaternion + 4);
                        q3 = r_s16(quaternion + 6);
                        p3 = r_s16(parent_quaternion + 6);
                        x = (uint32)(q0 * p0) - ((uint32)(q1 * p1) + (uint32)(q2 * p2) + (uint32)(q3 * p3));
                        y = (uint32)(q0 * p1) + (uint32)(p0 * q1) + (uint32)(q2 * p3) - (uint32)(q3 * p2);
                        z = (uint32)(q0 * p2) + (uint32)(p0 * q2) + (uint32)(q3 * p1) - (uint32)(q1 * p3);
                        value = (uint32)(q0 * p3) + (uint32)(p0 * q3) + (uint32)(q1 * p2) - (uint32)(q2 * p1);
                        w_u16(composed, x >> 14);
                        w_u16(composed + 2, y >> 14);
                        w_u16(composed + 4, z >> 14);
                        w_u16(composed + 6, (uint32)((sint32)value >> 14));
                    }
                }
                if (source != 0)
                {
                    /* Preserve both aligned reads of every LWL/LWR pair */
                    for (index = 0; index < 2; ++index)
                    {
                        address = source + index * 4;
                        upper = r_u32((address + 3) & 0xFFFFFFFC);
                        lower = r_u32(address & 0xFFFFFFFC);
                        shift = (address & 3) * 8;
                        value = shift == 0 ? lower : (upper << (32 - shift)) | (lower >> shift);
                        if (index == 0)
                            first_word = value;
                        else
                            second_word = value;
                    }
                    for (index = 0; index < 2; ++index)
                    {
                        value = index == 0 ? first_word : second_word;
                        destination = composed + index * 4;
                        shift = ((destination + 3) & 3) * 8;
                        mask = 0xFFFFFFFF >> (24 - shift);
                        old = r_u32((destination + 3) & 0xFFFFFFFC);
                        w_u32((destination + 3) & 0xFFFFFFFC, (old & ~mask) | (value >> (24 - shift)));
                        shift = (destination & 3) * 8;
                        mask = 0xFFFFFFFF << shift;
                        old = r_u32(destination & 0xFFFFFFFC);
                        w_u32(destination & 0xFFFFFFFC, (old & ~mask) | (value << shift));
                    }
                }
                first_word = r_u32(composed);
                first_word = r_u32(composed);
                second_word = r_u32(composed + 4);
                second_word = r_u32(composed + 4);
                for (index = 0; index < 2; ++index)
                {
                    /* Merge SWL then SWR without changing neighboring bytes */
                    value = index == 0 ? first_word : second_word;
                    destination = quaternion + index * 4;
                    shift = ((destination + 3) & 3) * 8;
                    mask = 0xFFFFFFFF >> (24 - shift);
                    old = r_u32((destination + 3) & 0xFFFFFFFC);
                    w_u32((destination + 3) & 0xFFFFFFFC, (old & ~mask) | (value >> (24 - shift)));
                    shift = (destination & 3) * 8;
                    mask = 0xFFFFFFFF << shift;
                    old = r_u32(destination & 0xFFFFFFFC);
                    w_u32(destination & 0xFFFFFFFC, (old & ~mask) | (value << shift));
                }
            }
            q1 = r_s16(quaternion + 2);
            q2 = r_s16(quaternion + 4);
            q3 = r_s16(quaternion + 6);
            q0 = r_s16(quaternion);
            xx = (uint32)(q1 * q1);
            yy = (uint32)(q2 * q2);
            zz = (uint32)(q3 * q3);
            xy = (uint32)(q1 * q2);
            xz = (uint32)(q1 * q3);
            yz = (uint32)(q2 * q3);
            wz = (uint32)(q0 * q3);
            wy = (uint32)(q0 * q2);
            wx = (uint32)(q0 * q1);
            w_u16(matrix, (uint32)((sint32)((uint32)0x08000000 - yy - zz) >> 15));
            w_u16(matrix + 8, (uint32)((sint32)((uint32)0x08000000 - xx - zz) >> 15));
            w_u16(matrix + 16, (uint32)((sint32)((uint32)0x08000000 - xx - yy) >> 15));
            w_u16(matrix + 2, (uint32)((sint32)(xy - wz) >> 15));
            w_u16(matrix + 6, (uint32)((sint32)(xy + wz) >> 15));
            w_u16(matrix + 4, (uint32)((sint32)(xz + wy) >> 15));
            w_u16(matrix + 12, (uint32)((sint32)(xz - wy) >> 15));
            w_u16(matrix + 10, (uint32)((sint32)(yz - wx) >> 15));
            w_u16(matrix + 14, (uint32)((sint32)(yz + wx) >> 15));
            PushMatrix();
            x = r_u32(matrix);
            y = r_u32(matrix + 4);
            xport_gte_write_control(0, x);
            xport_gte_write_control(1, y);
            x = r_u32(matrix + 8);
            y = r_u32(matrix + 12);
            z = r_u16(matrix + 16);
            xport_gte_write_control(2, x);
            xport_gte_write_control(3, y);
            xport_gte_write_control(4, z);
            p2_sdk_8007ec64(vector_offset, transformed);
            PopMatrix();
            vector_offset = transformed;
            rotated = 1;
        }
    }
    if (rotated != 0)
    {
        x = (uint32)(r_s32(vector_offset) >> 6);
        y = (uint32)(r_s32(vector_offset + 4) >> 6);
        z = (uint32)(r_s32(vector_offset + 8) >> 6);
    }
    else
    {
        height = r_u32(vector_offset + 4);
        if (height + 32 < 65)
        {
            p2_guest_stack_pointer = saved_stack;
            return 4096;
        }
        x = (uint32)(r_s32(vector_offset) >> 6);
        y = (uint32)((sint32)height >> 6);
        z = (uint32)(r_s32(vector_offset + 8) >> 6);
    }
    squared = x * x + y * y + z * z;
    if ((sint32)squared >= 0)
        magnitude = p2_sdk_8007e794(squared);
    else
        magnitude = p2_sdk_8007e794(squared >> 2) << 1;
    if (rotated == 0)
    {
        height = r_u32(vector_offset + 4);
        if ((sint32)height < 0)
            height = 0 - height;
        if ((sint32)magnitude < (sint32)height)
            magnitude = height;
    }
    height = r_u32(vector_offset + 4);
    result = (uint32)p2_func_8005dcd4((sint32)height, (sint32)magnitude);
    if ((sint16)result != 4096)
        result = (0 - result) & 0xFFF;
    p2_guest_stack_pointer = saved_stack;
    return (sint32)(sint16)result;
}

uint32 p2_func_8002f75c(uint32 object_offset, uint32 other_offset)
{
    uint32 kind, variant, context, scene;
    sint32 subtype, state, facing, other_facing;
    kind = r_u8(other_offset + 12);
    if (kind == 2)
    {
        subtype = r_s8(other_offset + 15);
        if (subtype == 9 || subtype == 14 || subtype == 29)
            return 1;
        if ((uint8)((uint32)subtype - 40) < 16 && (r_u8(other_offset + 13) & 0x80) == 0 && (r_u8(other_offset + 71) & 0xFE) == 0)
        {
            if (((r_u32(other_offset + 72) >> 12) & 31) == 4)
                return 1;
            subtype = r_s8(other_offset + 15);
        }
        if (subtype != 1)
            return 0;
        state = r_s8(object_offset + 126);
        if (state == 1)
        {
            if (r_s8(object_offset + 125) == 33)
                context = r_u32(object_offset + 112);
            else
            {
                scene = r_u32(0x8008A114);
                context = r_u32(scene + 136);
            }
        }
        else
        {
            context = 0;
            if (state == 3 && r_s8(object_offset + 120) == -1)
                context = r_u32(object_offset + 112);
        }
        return (p2_func_80047058(object_offset, other_offset, context) & 0xFF) != 0;
    }
    if (kind >= 3)
        return kind == 8;
    if (kind != 0)
        return 0;
    state = r_s8(object_offset + 121);
    variant = r_u8(other_offset + 126);
    if (state != -3)
        return 0;
    if (variant - 1 >= 2 || r_s8(object_offset + 120) == -(sint32)(sint8)variant)
    {
        if ((sint8)variant != 3 || r_s8(other_offset + 121) != state)
            return 0;
        facing = r_s8(object_offset + 120);
        other_facing = r_s8(other_offset + 120);
        if (facing == other_facing)
            return 0;
    }
    if ((sint8)variant == 2 && r_u32(object_offset + 116) == 11)
        return r_s8(other_offset + 125) == 17;
    return 1;
}

uint32 p2_func_8002f944(uint32 object_offset, uint32 other_offset, uint32 contact_offset)
{
    uint32 kind, contact, previous, value, variant;
    sint32 subtype;
    if (contact_offset == 0)
        return (uint32)(sint32)(sint8)p2_func_8002f75c(object_offset, other_offset);
    kind = r_u8(other_offset + 12);
    contact = r_u32(contact_offset + 4);
    if (kind == 2)
    {
        subtype = r_s8(other_offset + 15);
        if (subtype == 9 || subtype == 14 || subtype == 29)
        {
            w_u32(object_offset + 176, other_offset);
            return 1;
        }
        if (subtype == 1)
        {
            previous = r_u32(object_offset + 184);
            if (previous != 0 && r_s8(previous + 15) != 1)
            {
                if (r_u32(object_offset + 192) == 0)
                {
                    value = r_u32(object_offset + 204);
                    w_u32(object_offset + 192, previous);
                    w_u32(object_offset + 196, value);
                }
                else if (r_u32(object_offset + 200) == 0)
                {
                    value = r_u32(object_offset + 204);
                    w_u32(object_offset + 200, previous);
                    w_u32(object_offset + 220, value);
                }
                w_u32(object_offset + 184, 0);
            }
            if ((p2_func_800471f4(object_offset, other_offset, contact) & 0xFF) == 0)
                return 0;
            previous = r_u32(object_offset + 184);
            if (previous != 0 && r_s8(previous + 15) == 1 && (sint32)contact < r_s32(object_offset + 204))
                return 0;
            w_u32(object_offset + 184, other_offset);
            w_u32(object_offset + 204, contact);
        }
        else
        {
            if ((uint8)((uint32)subtype - 40) >= 16)
                return 0;
            if (r_u32(object_offset + 184) == 0)
            {
                w_u32(object_offset + 184, other_offset);
                w_u32(object_offset + 204, contact);
            }
            else if (r_u32(object_offset + 192) == 0)
            {
                w_u32(object_offset + 192, other_offset);
                w_u32(object_offset + 196, contact);
            }
            else
            {
                if (r_u32(object_offset + 200) != 0)
                    return 0;
                w_u32(object_offset + 200, other_offset);
                w_u32(object_offset + 220, contact);
            }
        }
        return 0;
    }
    if (kind < 3)
    {
        w_u32(object_offset + 176, other_offset);
        return 1;
    }
    if (kind != 8)
        return 0;
    p2_func_80047d18(other_offset, 0);
    variant = r_u16(other_offset + 18) & 0x7F;
    if (variant - 3 < 3)
        w_u16(object_offset + 128, r_u16(object_offset + 128) + 30);
    if (variant - 6 < 2)
        return 1;
    return variant == 10;
}
