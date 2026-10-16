#include "game.h"

uint32 p2_func_8002a3e0(uint32 mode, uint32 count)
{
    uint32 entries;
    uint32 bytes;
    uint32 allocation;
    FUNCTION_MARKER(0x8002A3E0, "SLUS_005.78");
    entries = (uint32)(sint16)(count + 1);
    bytes = entries << 2;
    allocation = p2_func_80064d28(bytes);
    p2_func_80066358(allocation, 255, bytes);
    w_u32(allocation, entries - 1);
    return allocation + 4;
}

uint32 p2_func_800750c4(sint16 sequence, sint16 separation)
{
    FUNCTION_MARKER(0x800750C4, "SLUS_005.78");
    return p2_sdk_800750f0(sequence, separation);
}

sint32 p2_func_8006e8d8(void)
{
    uint32 frames = 0;
    sint32 selection = 0;
    sint32 result = 1;
    uint32 deadline;
    uint32 old_buttons;
    uint32 pressed;
    uint32 sound;
    uint32 saved_frame;
    sint32 voice;
    FUNCTION_MARKER(0x8006E8D8, "SLUS_005.78");
    if (r_u32(0x8008A4FC) != 3)
        p2_func_80044508(3);
    pressed = p2_func_80066d08();
    deadline = r_u32(0x8008A434);
    w_u32(0x8008A3C4, pressed);
    w_u32(0x8008A3CC, 30);
    w_u32(0x8008A3D0, 15);
    w_u32(0x8008A3C8, deadline);
    deadline += 30000;
    for (;;)
    {
        p2_func_80066de4(255);
        p2_func_8006e6fc((uint32)selection);
        ++frames;
        pressed = 0;
        if ((sint32)frames >= 15)
        {
            old_buttons = ~r_u32(0x8008A3C4);
            w_u32(0x8008A3C4, p2_func_80066d08());
            pressed = r_u32(0x8008A3C4) & old_buttons;
        }
        if (pressed & 0x5000)
        {
            sound = 0;
            saved_frame = r_u32(0x8008A518);
            if (r_u32(0x8008A408) != 0 && r_s32(r_u32(0x8008A408)) > 24)
                sound = r_u32(0x8008A408) + 196;
            w_u32(0x8008A518, r_u32(0x8008A4F0));
            voice = p2_func_8005e628(0xFFFFFFFF, sound, 4);
            if (voice >= 0)
                p2_func_8005e470(sound, (uint16)voice, 0, 0, 0, r_s16(0x8008A24C), 0);
            w_u32(0x8008A518, saved_frame);
            if (pressed & 0x3000)
            {
                --selection;
                if (selection < 0)
                    selection = 3;
            }
            else
            {
                ++selection;
                if (selection == 4)
                    selection = 0;
            }
        }
        else if (pressed & 0x40)
        {
            sound = 0;
            saved_frame = r_u32(0x8008A518);
            if (r_u32(0x8008A408) != 0 && r_s32(r_u32(0x8008A408)) > 25)
                sound = r_u32(0x8008A408) + 204;
            w_u32(0x8008A518, r_u32(0x8008A4F0));
            voice = p2_func_8005e628(0xFFFFFFFF, sound, 4);
            if (voice >= 0)
                p2_func_8005e470(sound, (uint16)voice, 0, 0, 0, r_s16(0x8008A24C), 0);
            w_u32(0x8008A518, saved_frame);
            switch (selection)
            {
                case 0:
                    p2_func_800558f0();
                    w_u8(0x8008A134, 0);
                    goto finish;
                case 2:
                    if ((p2_func_8006bbe8(1) << 24) != 0)
                        goto finish;
                    break;
                case 1:
                    p2_func_8006ce38();
                    break;
                default:
                    w_u32(0x8008A294, 2);
                    result = 0;
                    goto finish;
            }
        }
        if (pressed != 0 || r_u32(0x8008A3C4) != 0)
            deadline = r_u32(0x8008A3C8) + 30000;
        if (r_u32(0x8008A3C8) >= deadline)
            break;
        w_u32(0x8008A4F0, r_u32(0x8008A4F0) + 1);
    }
    result = 0;
finish:
    p2_func_80067160(0);
    p2_func_8005ed74(0);
    return result;
}

uint32 p2_func_80066d08(void)
{
    uint32 high;
    uint32 low;
    FUNCTION_MARKER(0x80066D08, "SLUS_005.78");
    if (r_u8(0x800A5C28) == 255 || (r_u8(0x800A5C29) & 0xF0) != 0x40)
        return 0;
    high = r_u8(0x800A5C2A);
    low = r_u8(0x800A5C2B);
    return ((high << 8) | low) ^ 0xFFFF;
}

uint32 p2_func_80066de4(uint32 asset)
{
    uint32 result;
    uint32 offset;
    uint32 image;
    FUNCTION_MARKER(0x80066DE4, "SLUS_005.78");
    result = r_u32(0x8008A290);
    if (result != asset)
    {
        w_u32(0x8008A290, asset);
        DrawSync(0);
        VSync(0);
        offset = p2_func_8006a3c4(asset);
        image = r_u32(0x8008A4C8) + offset;
        LoadImagePSX((PSX_RECT *)psx_addr(0x8008A288, sizeof(PSX_RECT)), (uint32 *)psx_addr(image, sizeof(uint32)));
        return (uint32)DrawSync(0);
    }
    return result;
}

uint32 p2_func_8006e6fc(uint32 selection)
{
    uint32 position;
    uint32 group;
    uint32 saved_frame;
    uint32 index;
    uint32 fade;
    uint32 result;
    FUNCTION_MARKER(0x8006E6FC, "SLUS_005.78");
    p2_func_800665d4();
    p2_func_800665b0();
    p2_func_80066af0(0);
    if (selection == 0)
    {
        position = 0x640064;
        group = r_u32(r_u32(0x8008A430) + 28) + 40;
    }
    else if (selection == 1)
    {
        position = 0x7D0073;
        group = r_u32(r_u32(0x8008A430) + 28) + 60;
    }
    else if (selection == 2)
    {
        position = 0x960063;
        group = r_u32(r_u32(0x8008A430) + 28) + 80;
    }
    else
    {
        position = 0xB00070;
        group = r_u32(r_u32(0x8008A430) + 28) + 20 * (selection + 2);
    }
    p2_func_80064e74(position, 0, group, 0);
    p2_func_80027228();
    group = r_u32(0x8008A430);
    if (group != 0)
    {
        saved_frame = r_u32(0x8008A518);
        w_u32(0x8008A518, r_u32(0x8008A4F0));
        if (r_s32(group + 24) > 0)
        {
            index = 0;
            do
            {
                p2_func_800275bc(r_u32(0x800B1524 + 8 * index));
                ++index;
            } while ((sint32)index < r_s32(r_u32(0x8008A430) + 24));
        }
        w_u32(0x8008A518, saved_frame);
    }
    fade = r_u32(0x8008A3D0);
    if (fade != 0)
    {
        --fade;
        w_u32(0x8008A3D0, fade);
        if (fade != 0)
            p2_func_800651bc(0, 0xF00140, (sint32)(fade << 12) / 15, 0xFFFFFF);
    }
    p2_func_80066698();
    result = p2_func_80066d78(r_u32(0x8008A3C8), 0x8008A3CC);
    w_u32(0x8008A3C8, result);
    return result;
}

uint32 p2_func_800665d4(void)
{
    uint32 previous;
    uint32 buffer;
    uint32 draw_list;
    uint32 primitives;
    uint32 next_primitives;
    uint32 other_primitives;
    uint32 order_table;
    FUNCTION_MARKER(0x800665D4, "SLUS_005.78");
    previous = r_u8(0x8008A468);
    w_u8(0x8008A468, previous + 1);
    if (previous != 0)
        return 1;
    buffer = 0x80087200 + 132 * (1 - (uint32)r_u8(0x8008A374));
    draw_list = r_u32(buffer + 120);
    order_table = r_u32(buffer + 124);
    w_u32(0x8008A44C, draw_list);
    w_u32(0x8008A4D0, order_table);
    p2_sdk_800804bc(r_u32(0x8008A44C), 1024);
    p2_sdk_800804bc(r_u32(0x8008A4D0), 32);
    other_primitives = r_u8(0x8008A374);
    primitives = r_u32(buffer + 112);
    next_primitives = r_u32(buffer + 116);
    other_primitives = r_u32(0x80087270 + 132 * other_primitives);
    order_table = r_u32(buffer + 128);
    w_u32(0x8008A4B8, 0);
    w_u32(0x8008A4F8, primitives);
    w_u32(0x8008A4A0, primitives + 41600);
    w_u32(0x8008A500, next_primitives);
    w_u32(0x8008A450, order_table);
    w_u32(0x8008A454, other_primitives);
    return 41600;
}

uint32 p2_func_800665b0(void)
{
    FUNCTION_MARKER(0x800665B0, "SLUS_005.78");
    return p2_sdk_800804bc(r_u32(0x8008A450), 601);
}

uint32 p2_func_80066af0(uint32 mode)
{
    uint32 active;
    uint32 origin;
    sint32 y;
    PSX_RECT rectangle;
    FUNCTION_MARKER(0x80066AF0, "SLUS_005.78");
    active = r_u8(0x8008A374);
    origin = r_u32(0x8008A288);
    r_u32(0x8008A28C);
    rectangle.x = (sint16)origin;
    rectangle.y = (sint16)(origin >> 16);
    y = r_s16(0x80087202 + 132 * (1 - active));
    rectangle.w = 320;
    rectangle.h = 240;
    MoveImage(&rectangle, 0, y);
    return (uint32)DrawSync(0);
}

uint32 p2_func_80064e74(uint32 position, uint32 extent, uint32 group, uint32 blend)
{
    uint32 packet;
    uint32 texture;
    uint32 corner0;
    uint32 corner1;
    uint32 corner2;
    uint32 corner3;
    uint32 page;
    uint32 color;
    uint32 right;
    uint32 bottom;
    uint32 result;
    FUNCTION_MARKER(0x80064E74, "SLUS_005.78");
    packet = r_u32(0x8008A4F8);
    result = packet < r_u32(0x8008A4A0);
    if (result == 0)
        return result;
    texture = r_u32(group + 4);
    corner2 = texture >> 16;
    corner1 = (texture & 0xFF00) | (corner2 & 255);
    corner3 = (corner2 & 0xFF00) | (texture & 255);
    if ((extent & 0xFFFF) == 0)
    {
        extent |= (corner2 & 255) - (texture & 255) + 1;
        ++corner1;
        ++corner2;
    }
    if ((extent >> 16) == 0)
    {
        extent |= ((((corner2 >> 8) & 255) - ((texture >> 8) & 255)) + 1) << 16;
        corner2 += 256;
        corner3 += 256;
    }
    page = r_u32(group + 8);
    corner0 = (texture & 0xFFFF) | (page & 0xFFFF0000);
    if (r_u32(0x8008A4B8) == 0)
        w_u32(0x8008A4B8, packet);
    w_u32(packet, ((packet + 40) & 0xFFFFFF) | 0x09000000);
    if (blend != 0)
    {
        corner1 |= (page | (((blend - 1) & 3) << 5)) << 16;
        color = 0x2F000000;
    }
    else
    {
        corner1 |= page << 16;
        color = 0x2D000000;
    }
    w_u32(packet + 4, color);
    w_u32(packet + 12, corner0);
    right = (position & 0xFFFF) + (extent & 0xFFFF);
    bottom = (position & 0xFFFF0000) + (extent & 0xFFFF0000);
    w_u32(packet + 16, right | (position & 0xFFFF0000));
    result = packet + 40;
    w_u32(packet + 8, position);
    w_u32(packet + 20, corner1);
    w_u32(packet + 24, (position & 0xFFFF) | bottom);
    w_u32(packet + 28, corner3);
    w_u32(packet + 32, right | bottom);
    w_u32(packet + 36, corner2);
    w_u32(0x8008A478, packet);
    w_u32(0x8008A4F8, result);
    return result;
}

uint32 p2_func_80027228(void)
{
    uint32 index;
    uint32 saved_frame;
    uint32 result;
    FUNCTION_MARKER(0x80027228, "SLUS_005.78");
    for (index = 0; index < 28; ++index)
    {
        if (index == 1 || index == 5 || ((r_u32(0x1F800320) & 0x10) == 0 && index >= 13))
        {
            saved_frame = r_u32(0x8008A518);
            if (index == 1 || index == 5)
                w_u32(0x8008A518, r_u32(0x8008A4F0));
            p2_func_800275bc(r_u32(0x800B170C + 8 * index));
            w_u32(0x8008A518, saved_frame);
        }
    }
    result = r_u32(0x8008A4F0) + 1;
    w_u32(0x8008A4F0, result);
    return result;
}

sint32 p2_func_800275bc(uint32 entries)
{
    uint32 pointer = entries;
    uint32 header;
    uint32 remaining;
    uint32 frame;
    FUNCTION_MARKER(0x800275BC, "SLUS_005.78");
    if (entries == 0)
        return 1;
    header = r_u32(entries - 4);
    remaining = (header & 255) - 1;
    frame = r_u32(0x8008A518) - (header >> 8);
    if (remaining != 0)
    {
        do
        {
            p2_func_80027314(pointer, frame);
            --remaining;
            pointer += 4;
        } while (remaining != 0);
    }
    return (sint8)p2_func_80027314(pointer, frame);
}

uint32 p2_func_80027314(uint32 entry, uint32 frame)
{
    uint32 encoded;
    uint32 index;
    uint32 base;
    uint32 record;
    uint32 kind;
    uint32 animation;
    uint32 flags;
    uint32 phase;
    uint32 last_frame;
    uint32 period;
    uint32 remainder;
    uint32 selected;
    uint32 random_frame;
    uint32 previous_random;
    uint32 packet;
    uint32 source;
    uint32 words[2];
    uint16 page;
    FUNCTION_MARKER(0x80027314, "SLUS_005.78");
    encoded = r_u32(entry);
    if (encoded == 0xFFFFFFFF)
        return 1;
    index = (encoded >> 1) & 0x1FF;
    if (encoded & 1)
        base = r_u32(r_u32(0x8008A4E8) + 160);
    else
        base = r_u32(0x800B7F54);
    record = base + 16 * index;
    kind = r_u8(record + 4);
    animation = r_u32(record + 8);
    if (kind == 1)
    {
        phase = (uint32)(((sint64)(sint32)frame * r_s32(animation)) >> 16);
        w_u32(r_u32(record) + 12, phase);
        return 1;
    }
    if (kind == 0)
    {
        p2_func_80027034(record, animation, frame);
        return 1;
    }
    if (kind != 2)
        return 1;
    flags = (r_u32(entry) >> 26) & 15;
    phase = (uint32)(((sint64)(sint32)frame * r_s32(animation)) >> 16);
    last_frame = r_u8(animation + 5);
    period = (flags & 2) ? 2 * last_frame : last_frame + 1;
    if ((flags & 8) != 0 && phase >= period)
        return 0;
    if (period == 0)
        trap(7);
    remainder = phase % period;
    selected = remainder;
    random_frame = 0;
    if (flags & 1)
        selected = r_u8(animation + 5) - remainder;
    else if (flags & 2)
    {
        last_frame = r_u8(animation + 5);
        if ((sint32)(last_frame - remainder) < 0)
            selected = 2 * last_frame - remainder;
    }
    else if ((flags & 4) != 0 && remainder != ((r_u32(entry) >> 10) & 255))
    {
        previous_random = (r_u32(entry) >> 18) & 255;
        do
        {
            selected = p2_func_80068198() % period;
        } while (selected == previous_random);
        random_frame = selected;
    }
    packet = r_u32(record);
    source = r_u32(record + 12) + 8 * selected;
    page = r_u16(packet + 10);
    words[0] = r_u32(source);
    words[1] = r_u32(source + 4);
    w_u32(packet + 4, words[0]);
    w_u32(packet + 8, words[1]);
    w_u16(r_u32(record) + 10, page);
    w_u32(entry, (r_u32(entry) & 1) | (index << 1) | (remainder << 10) | (random_frame << 18) | (flags << 26));
    return 1;
}

uint32 p2_func_80027034(uint32 record, uint32 animation, uint32 frame)
{
    uint32 remaining;
    uint32 palette;
    uint32 cycle;
    uint32 phase;
    sint32 delta;
    sint32 difference;
    sint32 distance;
    uint32 destination;
    uint32 source;
    uint32 length;
    uint32 first_bytes;
    uint32 second_bytes;
    uint16 saved;
    uint8 temporary[512];
    FUNCTION_MARKER(0x80027034, "SLUS_005.78");
    remaining = r_u16(record + 6);
    palette = r_u32(record + 12);
    cycle = animation + 4;
    do
    {
        phase = (uint32)(((sint64)(sint32)frame * r_s32(animation)) >> 16);
        difference = (sint32)r_u8(cycle + 1) - r_u8(cycle);
        distance = difference > 0 ? difference : -difference;
        phase %= (uint32)distance + 1;
        delta = (sint32)r_u8(cycle + 2) - (sint32)phase;
        if (delta != 0)
        {
            w_u8(cycle + 2, phase);
            if (delta == -1 || delta == distance)
            {
                destination = palette + 2 * r_u8(cycle + 1);
                saved = r_u16(destination);
                if (difference > 0)
                {
                    source = palette + 2 * r_u8(cycle);
                    destination = source + 2;
                }
                else
                    source = destination + 2;
                p2_func_800663ac(destination, source, 2 * (uint32)distance);
                w_u16(palette + 2 * r_u8(cycle), saved);
            }
            else
            {
                if (difference >= 0)
                {
                    delta = -delta;
                    destination = palette + 2 * r_u8(cycle);
                }
                else
                    destination = palette + 2 * r_u8(cycle + 1);
                length = (uint32)distance + 1;
                p2_func_8006637c(temporary, (const uint8 *)psx_addr(destination, 2 * length), 2 * length);
                if (delta <= 0)
                {
                    delta = -delta;
                    second_bytes = 2 * (uint32)delta;
                    first_bytes = 2 * (length - (uint32)delta);
                    source = second_bytes;
                }
                else
                {
                    second_bytes = 2 * (length - (uint32)delta);
                    first_bytes = 2 * (uint32)delta;
                    source = second_bytes;
                }
                p2_func_8006637c((uint8 *)psx_addr(destination, first_bytes), temporary + source, first_bytes);
                p2_func_8006637c((uint8 *)psx_addr(destination + first_bytes, second_bytes), temporary, second_bytes);
            }
        }
        cycle += 8;
        animation += 8;
        --remaining;
    } while (remaining != 0);
    return p2_func_80064d88(record);
}

uint32 p2_func_80064d88(uint32 record)
{
    uint32 texture;
    uint32 palette;
    uint16 clut;
    uint16 page;
    FUNCTION_MARKER(0x80064D88, "SLUS_005.78");
    texture = r_u32(record);
    palette = r_u32(record + 12);
    clut = r_u16(texture + 10);
    page = r_u16(texture + 8);
    return p2_func_80066b78(clut, page, palette);
}

uint32 p2_func_80066b78(uint16 clut, uint32 page, uint32 palette)
{
    PSX_RECT rectangle;
    FUNCTION_MARKER(0x80066B78, "SLUS_005.78");
    rectangle.x = (sint16)((clut & 0x3F) << 4);
    rectangle.y = (sint16)(clut >> 6);
    rectangle.w = ((page >> 7) & 3) == 1 ? 256 : 16;
    rectangle.h = 1;
    return (uint32)LoadImagePSX(&rectangle, (uint32 *)psx_addr(palette, 2 * (uint32)rectangle.w));
}

uint32 p2_func_800651bc(uint32 position, uint32 extent, sint32 fraction, uint32 color)
{
    uint32 red;
    uint32 green;
    uint32 blue;
    uint32 packet;
    uint32 next;
    FUNCTION_MARKER(0x800651BC, "SLUS_005.78");
    if (fraction >= 4096)
    {
        red = color & 255;
        green = (color >> 8) & 255;
        blue = (color >> 16) & 255;
    }
    else if (fraction > 0)
    {
        red = ((uint32)fraction * (color & 255) + 2048) >> 12;
        green = ((uint32)fraction * ((color >> 8) & 255) + 2048) >> 12;
        blue = ((uint32)fraction * ((color >> 16) & 255) + 2048) >> 12;
    }
    else
        red = green = blue = 0;
    color = red | (green << 8) | (blue << 16);
    if (r_u32(0x8008A4B8) == 0)
        w_u32(0x8008A4B8, r_u32(0x8008A4F8));
    packet = r_u32(0x8008A4F8);
    next = packet + 12;
    w_u32(packet, (next & 0xFFFFFF) | 0x02000000);
    w_u32(packet + 4, 0xE1000040);
    w_u32(packet + 8, 0);
    w_u32(packet + 16, color | 0x62000000);
    w_u32(packet + 12, ((packet + 28) & 0xFFFFFF) | 0x03000000);
    w_u32(packet + 20, position);
    w_u32(packet + 24, extent);
    w_u32(0x8008A478, next);
    w_u32(0x8008A4F8, packet + 28);
    return 0x03000000;
}

uint32 p2_func_80066698(void)
{
    uint32 nesting;
    uint32 buffer;
    uint32 chain = 0;
    uint32 tail = 0;
    uint32 primitives;
    uint32 first;
    uint32 tag;
    uint32 active;
    FUNCTION_MARKER(0x80066698, "SLUS_005.78");
    nesting = r_u8(0x8008A468);
    if (nesting == 0)
        return nesting;
    nesting = (nesting - 1) & 255;
    w_u8(0x8008A468, nesting);
    if (nesting != 0)
        return nesting;
    if (r_u32(0x8008A4B0) != 0)
        DrawSync(0);
    if (r_u32(0x8008A4A8) != 0)
        VSync(0);
    buffer = 0x80087200 + 132 * (1 - (uint32)r_u8(0x8008A374));
    if (r_u32(0x8008A500) != r_u32(buffer + 116))
    {
        tail = r_u32(0x8008A450);
        chain = tail + 2400;
    }
    primitives = r_u32(0x8008A4F8);
    if (primitives != r_u32(buffer + 112))
    {
        first = r_u32(0x8008A4B8);
        if (primitives != first)
        {
            if (chain == 0)
                chain = r_u32(0x8008A44C) + 4092;
            if (tail != 0)
            {
                tag = r_u32(tail) & 0xFF000000;
                w_u32(tail, tag);
                w_u32(tail, tag | ((r_u32(0x8008A44C) + 4092) & 0xFFFFFF));
            }
            tail = r_u32(0x8008A44C);
            if (chain == 0)
                chain = r_u32(0x8008A4D0) + 124;
            if (tail != 0)
            {
                tag = r_u32(tail) & 0xFF000000;
                w_u32(tail, tag);
                w_u32(tail, tag | ((r_u32(0x8008A4D0) + 124) & 0xFFFFFF));
            }
            tail = r_u32(0x8008A4D0);
            first = r_u32(0x8008A4B8);
        }
    }
    else
        first = r_u32(0x8008A4B8);
    if (first != 0)
    {
        if (chain == 0)
            chain = first;
        if (tail != 0)
        {
            tag = r_u32(tail) & 0xFF000000;
            w_u32(tail, tag);
            w_u32(tail, tag | (r_u32(0x8008A4B8) & 0xFFFFFF));
        }
        tail = r_u32(0x8008A478);
    }
    w_u32(tail, r_u32(tail) | 0xFFFFFF);
    w_u32(0x8008A4AC, chain);
    PutDrawEnv((DRAWENV *)psx_addr(buffer, sizeof(DRAWENV)));
    w_u32(0x8008A4B0, 1);
    DrawOTag((uint32 *)psx_addr(chain, sizeof(uint32)));
    active = r_u8(0x8008A374);
    w_u8(0x8008A374, 1 - active);
    return active;
}

uint32 p2_func_80066d78(uint32 previous, uint32 fraction_offset)
{
    uint32 old_fraction;
    uint32 fraction;
    uint32 wait = 33;
    uint32 deadline;
    FUNCTION_MARKER(0x80066D78, "SLUS_005.78");
    old_fraction = r_u32(fraction_offset);
    fraction = old_fraction - 10;
    w_u32(fraction_offset, fraction);
    if ((sint32)fraction <= 0)
    {
        wait = 34;
        w_u32(fraction_offset, old_fraction + 20);
    }
    deadline = previous + wait;
    while (r_u32(0x8008A434) < deadline)
        psx_native_poll_events();
    r_u32(0x8008A434);
    return r_u32(0x8008A434);
}

uint32 p2_func_800663ac(uint32 destination, uint32 source, uint32 count)
{
    uint32 result = source < destination;
    FUNCTION_MARKER(0x800663AC, "SLUS_005.78");
    if (count != 0)
    {
        if (source < destination)
        {
            source += count - 1;
            destination += count - 1;
            do
            {
                result = r_u8(source);
                --source;
                --count;
                w_u8(destination, result);
                --destination;
            } while (count != 0);
        }
        else
        {
            do
            {
                result = r_u8(source);
                ++source;
                --count;
                w_u8(destination, result);
                ++destination;
            } while (count != 0);
        }
    }
    return result;
}

sint32 p2_func_8005e628(uint32 requested, uint32 sound, uint32 priority)
{
    uint32 voice = requested;
    uint32 first;
    uint32 row;
    uint32 index;
    uint32 previous;
    uint32 frame;
    FUNCTION_MARKER(0x8005E628, "SLUS_005.78");
    if (requested >= 8)
    {
        first = priority != 1 ? 3 : 0;
        voice = first;
        row = 0x8008B6AC + 12 * voice;
        while (voice < 8)
        {
            if (r_u32(row) < r_u32(0x8008A518))
            {
                if (p2_func_8006a940((uint16)voice) == 0)
                    goto assign;
                previous = r_u32(row);
                frame = r_u32(0x8008A518);
                if (previous < frame && first == 3 && frame - previous >= 301)
                    goto assign_with_frame;
            }
            ++voice;
            row += 12;
        }
        voice = r_u8(0x8008A244);
        index = 0;
        if (priority == 1)
        {
            if ((sint32)voice < (sint32)first)
                goto next_candidate;
            if (voice < 3)
                goto assign;
            voice = r_u8(0x8008A245);
            index = 1;
            goto reserved_candidate;
        }
    priority_candidate:
        if (r_u32(0x8008B6B0 + 12 * voice) < priority && (sint32)voice >= (sint32)first)
            goto assign;
    next_candidate:
        ++index;
        if (index >= 8)
            return -1;
        voice = r_u8(0x8008A244 + index);
        if (priority != 1)
            goto priority_candidate;
    reserved_candidate:
        if ((sint32)voice < (sint32)first || voice >= 3)
            goto next_candidate;
    }
assign:
    frame = r_u32(0x8008A518);
assign_with_frame:
    row = 12 * voice;
    w_u32(0x8008B6A8 + row, sound);
    w_u32(0x8008B6B0 + row, priority);
    w_u32(0x8008B6AC + row, frame);
    index = 0;
    while (r_u8(0x8008A244 + index) != voice)
    {
        ++index;
        if (index >= 7)
            goto finish;
    }
    p2_func_800663ac(0x8008A244 + index, 0x8008A245 + index, 7 - index);
finish:
    w_u8(0x8008A24B, voice);
    return (sint32)voice;
}

uint32 p2_func_8006a940(uint16 voice)
{
    uint32 mask;
    FUNCTION_MARKER(0x8006A940, "SLUS_005.78");
    mask = (uint32)1 << (((uint32)voice + 16) & 31);
    return p2_sdk_80070df8(mask) == 1;
}

void p2_func_8005e470(uint32 sound, uint16 voice, uint32 arg3, uint32 arg4, uint32 override_pitch, sint32 volume, uint16 program)
{
    uint32 pitch = override_pitch;
    uint16 pan;
    FUNCTION_MARKER(0x8005E470, "SLUS_005.78");
    if ((volume >> 4) != 0)
    {
        if (pitch == 0)
            pitch = r_u16(sound + 4);
        pan = program != 0 ? (uint16)(program - 1) : 4;
        p2_func_8006a740(voice, (uint16)(volume >> 4));
        p2_func_8006a638(voice, pan);
        p2_func_8006a814(voice, pitch);
        r_u16(sound + 6);
        p2_func_8006a878(voice, sound);
    }
}

uint32 p2_func_8006a740(uint16 voice, uint16 volume)
{
    uint32 row = 0x800A9688 + 12 * voice;
    uint32 gain;
    sint64 product;
    FUNCTION_MARKER(0x8006A740, "SLUS_005.78");
    gain = (16383 * (uint32)volume) / 256;
    w_u32(row + 8, gain);
    w_u32(0x800A96E8, (uint32)1 << (((uint32)voice + 16) & 31));
    w_u32(0x800A96EC, 3);
    product = (sint64)r_s32(row + 8) * r_s32(row);
    w_u16(0x800A96F0, (uint32)(product >> 12));
    product = (sint64)r_s32(row + 8) * r_s32(row + 4);
    w_u16(0x800A96F2, (uint32)(product >> 12));
    return p2_sdk_8007101c(0x800A96E8);
}

uint32 p2_func_8006a638(uint16 voice, uint16 pan)
{
    uint32 row;
    sint32 delta;
    sint64 product;
    FUNCTION_MARKER(0x8006A638, "SLUS_005.78");
    if (pan >= 9)
        pan = 8;
    delta = ((sint32)pan - 4) * 512;
    row = 0x800A9688 + 12 * voice;
    w_u32(row, (uint32)(2048 - delta));
    w_u32(row + 4, 4096 - r_u32(row));
    w_u32(0x800A96E8, (uint32)1 << (((uint32)voice + 16) & 31));
    w_u32(0x800A96EC, 3);
    product = (sint64)r_s32(row + 8) * r_s32(row);
    w_u16(0x800A96F0, (uint32)(product >> 12));
    product = (sint64)r_s32(row + 8) * r_s32(row + 4);
    w_u16(0x800A96F2, (uint32)(product >> 12));
    return p2_sdk_8007101c(0x800A96E8);
}

uint32 p2_func_8006a814(uint16 voice, uint32 pitch)
{
    FUNCTION_MARKER(0x8006A814, "SLUS_005.78");
    w_u32(0x800A96E8, (uint32)1 << (((uint32)voice + 16) & 31));
    w_u32(0x800A96EC, 16);
    w_u16(0x800A96FC, (95 * pitch) >> 10);
    return p2_sdk_8007101c(0x800A96E8);
}

void p2_func_8006a878(uint16 voice, uint32 sound)
{
    uint32 mask;
    uint32 address;
    FUNCTION_MARKER(0x8006A878, "SLUS_005.78");
    if (sound != 0)
    {
        mask = (uint32)1 << (((uint32)voice + 16) & 31);
        p2_sdk_80070bc0(0, mask);
        w_u32(0x800A96EC, 65408);
        w_u32(0x800A96E8, mask);
        address = r_u32(sound);
        w_u32(0x800A9714, 3);
        w_u32(0x800A970C, 1);
        w_u32(0x800A9710, 1);
        w_u16(0x800A9718, 0);
        w_u16(0x800A971A, 0);
        w_u16(0x800A971C, 0);
        w_u16(0x800A971E, 0);
        w_u16(0x800A9720, 15);
        w_u32(0x800A9704, address);
        p2_sdk_80070dc4(0x800A96E8);
    }
}

uint32 p2_func_8006ce38(void)
{
    sint32 sound_volume;
    sint32 music_volume;
    sint32 selection = 0;
    sint32 preview_voice = -1;
    sint32 voice;
    sint32 direction;
    uint32 preview_sound = 0;
    uint32 sound;
    uint32 saved_frame;
    uint32 old_buttons;
    uint32 buttons;
    uint32 pressed;
    uint32 cursor;
    uint32 clock;
    FUNCTION_MARKER(0x8006CE38, "SLUS_005.78");
    p2_func_80067160(0);
    sound = r_u32(0x8008A408);
    sound_volume = r_s16(0x8008A24C);
    if (sound != 0 && r_s32(sound) > 0)
        preview_sound = sound + 4;
    music_volume = r_s16(0x8008A24E);
    clock = r_u32(0x8008A434);
    w_u32(0x8008A3CC, 30);
    w_u32(0x8008A3D0, 15);
    w_u32(0x8008A3C8, clock);
    for (;;)
    {
        p2_func_80066de4(257);
        p2_func_8006c7b4(selection, music_volume, sound_volume);
        old_buttons = ~r_u32(0x8008A3C4);
        buttons = p2_func_80066d08();
        w_u32(0x8008A3C4, buttons);
        pressed = buttons & old_buttons;
        if (pressed & 0x10)
        {
            sound = 0;
            saved_frame = r_u32(0x8008A518);
            if (r_u32(0x8008A408) != 0 && r_s32(r_u32(0x8008A408)) > 25)
                sound = r_u32(0x8008A408) + 204;
            w_u32(0x8008A518, r_u32(0x8008A4F0));
            voice = p2_func_8005e628(0xFFFFFFFF, sound, 4);
            if (voice >= 0)
                p2_func_8005e470(sound, (uint16)voice, 0, 0, 0, r_s16(0x8008A24C), 0);
            w_u32(0x8008A518, saved_frame);
            break;
        }
        if (pressed & 0xA000)
        {
            sound = 0;
            saved_frame = r_u32(0x8008A518);
            if (r_u32(0x8008A408) != 0 && r_s32(r_u32(0x8008A408)) > 24)
                sound = r_u32(0x8008A408) + 196;
            w_u32(0x8008A518, r_u32(0x8008A4F0));
            voice = p2_func_8005e628(0xFFFFFFFF, sound, 4);
            if (voice >= 0)
                p2_func_8005e470(sound, (uint16)voice, 0, 0, 0, r_s16(0x8008A24C), 0);
            w_u32(0x8008A518, saved_frame);
            if (pressed & 0x3000)
            {
                --selection;
                if (selection < 0)
                    selection = 2;
            }
            else
            {
                ++selection;
                if (selection == 3)
                    selection = 0;
            }
        }
        else if (pressed & 0x5000)
        {
            direction = (pressed & 0x1000) ? -1 : 1;
            if (selection == 1)
            {
                music_volume = p2_func_8005f070(-direction);
                w_u32(0x8008A3C4, r_u32(0x8008A3C4) & 0xFFFFAFFF);
            }
            else if (selection == 2)
            {
                sound = 0;
                saved_frame = r_u32(0x8008A518);
                if (r_u32(0x8008A408) != 0 && r_s32(r_u32(0x8008A408)) > 25)
                    sound = r_u32(0x8008A408) + 204;
                w_u32(0x8008A518, r_u32(0x8008A4F0));
                voice = p2_func_8005e628(0xFFFFFFFF, sound, 4);
                if (voice >= 0)
                    p2_func_8005e470(sound, (uint16)voice, 0, 0, 0, r_s16(0x8008A24C), 0);
                w_u32(0x8008A518, saved_frame);
                if (pressed & 0x1000)
                {
                    cursor = (r_u8(0x8008A3E4) - 1) & 255;
                    w_u8(0x8008A3E4, cursor);
                    if (cursor & 0x80)
                        w_u8(0x8008A3E4, 23);
                }
                else
                {
                    cursor = (r_u8(0x8008A3E4) + 1) & 255;
                    w_u8(0x8008A3E4, cursor);
                    if (cursor >= 24)
                        w_u8(0x8008A3E4, 0);
                }
                w_u32(0x8008A2B4, 0x80087878 + 4 * (uint32)r_s8(0x8008A3E4));
            }
            else if (selection == 0)
            {
                saved_frame = r_u32(0x8008A518);
                w_u32(0x8008A3C4, buttons & 0xFFFFAFFF);
                w_u32(0x8008A518, r_u32(0x8008A4F0));
                sound_volume = p2_func_8005eff4(-direction);
                if ((p2_func_8005e868(preview_sound, (uint32)preview_voice) << 24) != 0)
                    p2_func_8006a740((uint16)preview_voice, (uint16)(sound_volume >> 4));
                else
                {
                    preview_voice = p2_func_8005e628(0xFFFFFFFF, preview_sound, 4);
                    if (preview_voice != -1)
                        p2_func_8005e470(preview_sound, (uint16)preview_voice, 0, 0, 20010, sound_volume, 0);
                }
                w_u32(0x8008A518, saved_frame);
            }
        }
        w_u32(0x8008A4F0, r_u32(0x8008A4F0) + 1);
    }
    p2_func_80067160(0);
    w_u32(0x8008A3D0, 15);
    return 0;
}

sint32 p2_func_80067160(uint32 color)
{
    uint32 step;
    uint32 previous;
    uint32 deadline;
    uint32 wait;
    sint32 fraction = 30;
    sint32 strength;
    FUNCTION_MARKER(0x80067160, "SLUS_005.78");
    p2_func_80066978();
    w_u32(0x8008A378, r_u32(0x8008A434));
    for (step = 2; step < 31; step += 2)
    {
        if (color == 0 || color == 0xFFFFFF)
        {
            previous = r_u32(0x8008A378);
            p2_func_800665d4();
            p2_func_800665b0();
            p2_func_80066af0(1);
            strength = (sint32)(step << 12) / 30;
            if (color == 0)
                p2_func_800651bc(0, 0xF00140, strength, 0xFFFFFF);
            else
                p2_func_800652e4(0, 0xF00140, strength, 0xFFFFFF);
            p2_func_80066698();
        }
        else
        {
            p2_func_800665d4();
            p2_func_800665b0();
            p2_func_80066af0(1);
            strength = (sint32)(step << 12) / 30;
            p2_func_800651bc(0, 0xF00140, strength, ~color | 0xFFFFFF);
            p2_func_800652e4(0, 0xF00140, strength, color);
            p2_func_80066698();
            previous = r_u32(0x8008A378);
        }
        fraction -= 10;
        wait = 33;
        if (fraction <= 0)
        {
            wait = 34;
            fraction += 30;
        }
        deadline = previous + wait;
        while (r_u32(0x8008A434) < deadline)
            psx_native_poll_events();
        r_u32(0x8008A434);
        w_u32(0x8008A378, r_u32(0x8008A434));
    }
    return 0;
}

uint32 p2_func_80066978(void)
{
    uint32 active;
    sint32 x;
    sint32 y;
    PSX_RECT rectangle;
    FUNCTION_MARKER(0x80066978, "SLUS_005.78");
    DrawSync(0);
    VSync(0);
    active = r_u8(0x8008A374);
    x = r_s16(0x8008A288);
    y = r_s16(0x8008A28A);
    rectangle.x = 0;
    rectangle.y = (sint16)r_u16(0x80087202 + 132 * active);
    rectangle.w = 320;
    rectangle.h = 240;
    MoveImage(&rectangle, x, y);
    return (uint32)DrawSync(0);
}

uint32 p2_func_8006c7b4(sint32 selection, sint32 music_volume, sint32 sound_volume)
{
    uint32 position;
    uint32 group_index;
    uint32 groups;
    uint32 icon;
    uint32 button;
    uint32 table;
    uint32 offset;
    uint32 index;
    uint32 saved_frame;
    uint32 fade;
    uint32 result;
    FUNCTION_MARKER(0x8006C7B4, "SLUS_005.78");
    DrawSync(0);
    VSync(0);
    p2_func_800665d4();
    p2_func_800665b0();
    p2_func_80066af0(0);
    if (selection == 2)
        position = 0x7A007E;
    else
        position = selection == 0 ? 0x650036 : 0x6400DC;
    /* The reviewed options caller supplies selection 0..2 */
    if (selection == 1)
        group_index = 8;
    else if (selection >= 2)
    {
        if (selection == 2)
            group_index = 7;
    }
    else if (selection == 0)
        group_index = 6;
    groups = r_u32(r_u32(0x8008A430) + 28);
    p2_func_80064e74(position, 0, groups + 20 * group_index, 0);
    groups = r_u32(r_u32(0x8008A430) + 28);
    if (groups != 0)
    {
        position = ((uint32)(67 - ((sint32)(50 * (uint32)sound_volume) >> 12)) << 16) | 0x44;
        p2_func_80064e74(position, 0, groups, 0);
    }
    if (groups != 0)
    {
        position = ((uint32)(67 - ((sint32)(50 * (uint32)music_volume) >> 12)) << 16) | 0xDC;
        p2_func_80064e74(position, 0, groups, 0);
    }
    for (icon = 0; icon < 3; ++icon)
    {
        index = icon == 0 ? 1 : icon == 1 ? 0 : 2;
        button = r_u8(r_u32(0x8008A2B4) + index);
        switch (button)
        {
            case 128:
                offset = 180;
                break;
            case 16:
                offset = 220;
                break;
            case 64:
                offset = 160;
                break;
            default:
                offset = 200;
                break;
        }
        table = r_u32(r_u32(0x8008A4A4) + 28);
        if (table + offset != 0)
        {
            button = r_u8(r_u32(0x8008A2B4) + index);
            switch (button)
            {
                case 128:
                    offset = 180;
                    break;
                case 16:
                    offset = 220;
                    break;
                case 64:
                    offset = 160;
                    break;
                default:
                    offset = 200;
                    break;
            }
            table = r_u32(r_u32(0x8008A4A4) + 28);
            position = icon == 0 ? 0x90007D : icon == 1 ? 0xAA007D : 0xC5007D;
            p2_func_80064e74(position, 0, table + offset, 0);
        }
    }
    p2_func_80027228();
    table = r_u32(0x8008A430);
    if (table != 0)
    {
        saved_frame = r_u32(0x8008A518);
        groups = r_u32(table + 24);
        w_u32(0x8008A518, r_u32(0x8008A4F0));
        if ((sint32)groups > 0)
        {
            index = 0;
            do
            {
                p2_func_800275bc(r_u32(0x800B1524 + 8 * index));
                ++index;
            } while ((sint32)index < r_s32(r_u32(0x8008A430) + 24));
        }
        w_u32(0x8008A518, saved_frame);
    }
    fade = r_u32(0x8008A3D0);
    if (fade != 0)
    {
        --fade;
        w_u32(0x8008A3D0, fade);
        if (fade != 0)
            p2_func_800651bc(0, 0xF00140, (sint32)(fade << 12) / 15, 0xFFFFFF);
    }
    p2_func_80066698();
    result = p2_func_80066d78(r_u32(0x8008A3C8), 0x8008A3CC);
    w_u32(0x8008A3C8, result);
    return result;
}

sint32 p2_func_8005eff4(sint32 direction)
{
    sint32 value;
    sint32 next;
    FUNCTION_MARKER(0x8005EFF4, "SLUS_005.78");
    value = r_s16(0x8008A24C);
    if (direction < 0)
    {
        if (value <= 0)
            return value;
        next = value - 64;
        w_u16(0x8008A24C, (uint32)next);
        if ((sint16)next < 0)
            w_u16(0x8008A24C, 0);
    }
    else if (value < 4096)
    {
        next = value + 64;
        w_u16(0x8008A24C, (uint32)next);
        if ((sint16)next >= 4097)
            w_u16(0x8008A24C, 4096);
    }
    return r_s16(0x8008A24C);
}

uint32 p2_func_8005e868(uint32 sound, uint32 voice)
{
    FUNCTION_MARKER(0x8005E868, "SLUS_005.78");
    if (voice < 8 && r_u32(0x8008B6A8 + 12 * voice) == sound)
        return p2_func_8006a940((uint16)voice) != 0;
    return 0;
}

uint32 p2_func_8006bbe8(uint32 requested_mode)
{
    uint8 mode = (uint8)requested_mode;
    uint32 success_until = 0;
    uint32 error_until = 0;
    uint32 pressed;
    uint32 buttons;
    uint32 old_buttons;
    uint32 repeat;
    uint32 sound;
    uint32 saved_frame;
    uint32 cursor;
    uint32 clock;
    uint32 offset;
    uint32 encoded;
    uint32 power;
    uint32 rotation;
    uint32 checksum;
    uint32 fields[7];
    uint32 decoded[7];
    uint32 widths[7] = {5, 5, 5, 2, 1, 3, 9};
    uint32 field;
    uint32 bit;
    uint32 aligned;
    uint32 scene;
    sint32 digit;
    sint32 voice;
    sint32 checked;
    sint32 index;
    uint8 settings[18];
    FUNCTION_MARKER(0x8006BBE8, "SLUS_005.78");
    if (mode != 0)
    {
        p2_func_80067160(0);
        w_u32(0x8008A3D0, 15);
        w_u8(0x8008A3DC, 0);
        if (r_u32(0x8008A2B0) != 0)
        {
            p2_func_80066358(0x8008A3D4, 0, 8);
            w_u32(0x8008A2B0, 0);
        }
    }
    else
        p2_func_8006b548();
    clock = r_u32(0x8008A434);
    w_u32(0x8008A3CC, 30);
    w_u32(0x8008A3C8, clock);
    for (;;)
    {
        pressed = 0;
        if (success_until == 0 && error_until == 0)
        {
            old_buttons = r_u32(0x8008A3C4);
            buttons = p2_func_80066d08();
            if (buttons & 0x1000)
                w_u32(0x8008A3B8, r_u32(0x8008A3B8) + 1);
            else
                w_u32(0x8008A3B8, 0);
            if (buttons & 0x4000)
                w_u32(0x8008A3BC, r_u32(0x8008A3BC) + 1);
            else
                w_u32(0x8008A3BC, 0);
            repeat = r_u32(0x8008A3B8);
            pressed = buttons & ~old_buttons;
            w_u32(0x8008A3C4, buttons);
            if ((sint32)repeat >= 11 && (repeat & 3) == 0)
                pressed |= 0x1000;
            repeat = r_u32(0x8008A3BC);
            if ((sint32)repeat >= 11 && (repeat & 3) == 0)
                pressed |= 0x4000;
        }
        if ((sint8)mode != 0)
            p2_func_80066de4(256);
        p2_func_8006b2b8((sint8)mode, error_until != 0, success_until != 0);
        if (pressed & 0x10)
        {
            sound = 0;
            saved_frame = r_u32(0x8008A518);
            if (r_u32(0x8008A408) != 0 && r_s32(r_u32(0x8008A408)) > 25)
                sound = r_u32(0x8008A408) + 204;
            w_u32(0x8008A518, r_u32(0x8008A4F0));
            voice = p2_func_8005e628(0xFFFFFFFF, sound, 4);
            if (voice >= 0)
                p2_func_8005e470(sound, (uint16)voice, 0, 0, 0, r_s16(0x8008A24C), 0);
            w_u32(0x8008A518, saved_frame);
            if (mode != 0)
            {
                p2_func_80067160(0);
                w_u32(0x8008A3D0, 15);
            }
            return 0;
        }
        if (pressed & 0x5000)
        {
            if ((sint8)mode != 0)
            {
                sound = 0;
                saved_frame = r_u32(0x8008A518);
                digit = r_u8(0x8008A3D4 + (uint32)r_s8(0x8008A3DC));
                if (r_u32(0x8008A408) != 0 && r_s32(r_u32(0x8008A408)) > 24)
                    sound = r_u32(0x8008A408) + 196;
                w_u32(0x8008A518, r_u32(0x8008A4F0));
                voice = p2_func_8005e628(0xFFFFFFFF, sound, 4);
                if (voice >= 0)
                    p2_func_8005e470(sound, (uint16)voice, 0, 0, 0, r_s16(0x8008A24C), 0);
                w_u32(0x8008A518, saved_frame);
                if ((pressed ^ 0x5000) & 0x3000)
                {
                    --digit;
                    if (digit < 0)
                        digit = 25;
                }
                else
                {
                    ++digit;
                    if (digit == 26)
                        digit = 0;
                }
                w_u8(0x8008A3D4 + (uint32)r_s8(0x8008A3DC), (uint32)digit);
            }
            goto finish_iteration;
        }
        if (pressed & 0xA000)
        {
            if ((sint8)mode != 0)
            {
                sound = 0;
                saved_frame = r_u32(0x8008A518);
                if (r_u32(0x8008A408) != 0 && r_s32(r_u32(0x8008A408)) > 24)
                    sound = r_u32(0x8008A408) + 196;
                w_u32(0x8008A518, r_u32(0x8008A4F0));
                voice = p2_func_8005e628(0xFFFFFFFF, sound, 4);
                if (voice >= 0)
                    p2_func_8005e470(sound, (uint16)voice, 0, 0, 0, r_s16(0x8008A24C), 0);
                w_u32(0x8008A518, saved_frame);
                if (pressed & 0x2000)
                {
                    cursor = (r_u8(0x8008A3DC) + 1) & 255;
                    w_u8(0x8008A3DC, cursor);
                    if ((sint8)cursor == 8)
                        w_u8(0x8008A3DC, 0);
                }
                else
                {
                    cursor = (r_u8(0x8008A3DC) - 1) & 255;
                    w_u8(0x8008A3DC, cursor);
                    if (cursor & 0x80)
                        w_u8(0x8008A3DC, 7);
                }
            }
            goto finish_iteration;
        }
        if ((pressed & 0x40) == 0)
            goto finish_iteration;
        sound = 0;
        saved_frame = r_u32(0x8008A518);
        if (r_u32(0x8008A408) != 0 && r_s32(r_u32(0x8008A408)) > 25)
            sound = r_u32(0x8008A408) + 204;
        w_u32(0x8008A518, r_u32(0x8008A4F0));
        voice = p2_func_8005e628(0xFFFFFFFF, sound, 4);
        if (voice >= 0)
            p2_func_8005e470(sound, (uint16)voice, 0, 0, 0, r_s16(0x8008A24C), 0);
        w_u32(0x8008A518, saved_frame);
        if ((sint8)mode == 0)
            goto finish_iteration;
        checked = p2_func_8006b9ac();
        if (checked != 0)
        {
            if (checked < 0)
                error_until = r_u32(0x8008A3C8) + 1000;
            else
            {
                clock = r_u32(0x8008A3C8);
                success_until = clock + 1000;
                if ((r_u32(0x8008A52C) & 0x200) == 0)
                    goto expire_messages;
                p2_func_8006b2b8(1, 0, 1);
                p2_func_80066430();
                if (r_u16(0x8008A530) != 0)
                    p2_func_8006ae90(r_u16(0x8008A530));
                w_u16(0x8008A530, 0);
                offset = p2_func_8006a3ac(47);
                w_u32(0x8008A4DC, r_u32(0x8008A284) + offset + 132576);
                w_u32(0x8008A488, p2_func_8006a1ac(47, r_u32(0x8008A284) + 132576, 1));
                p2_func_8006a1ac(147, r_u32(0x8008A4C8), 1);
                p2_func_8006624c(r_u32(0x8008A4C8), 0, 1);
                p2_func_80060eec();
                p2_func_80060df4();
                p2_func_80066e48();
                p2_func_80066e98();
            }
            goto finish_iteration;
        }
        encoded = 0;
        power = 1;
        for (index = 7; index >= 0; --index)
        {
            encoded += r_u8(0x8008A3D4 + (uint32)index) * power;
            power <<= 4;
        }
        rotation = encoded >> 30;
        fields[0] = (encoded >> rotation) & 31;
        fields[6] = (((encoded >> (rotation + 21)) & (((uint32)1 << (9 - rotation)) - 1)) << rotation) | (encoded & (((uint32)1 << rotation) - 1));
        fields[1] = (encoded >> (rotation + 5)) & 31;
        fields[2] = (encoded >> (rotation + 10)) & 31;
        fields[3] = (encoded >> (rotation + 15)) & 3;
        fields[4] = (encoded >> (rotation + 17)) & 1;
        fields[5] = (encoded >> (rotation + 18)) & 7;
        checksum = fields[0] + fields[1] + fields[2] + fields[3] + fields[4] + fields[5] + rotation;
        for (field = 0; field < 7; ++field)
        {
            decoded[field] = 0;
            for (bit = 0; bit < widths[field]; ++bit)
            {
                if (fields[field] & ((uint32)1 << bit))
                {
                    aligned = bit;
                    while (aligned < rotation + 1)
                        aligned += widths[field];
                    decoded[field] |= (uint32)1 << ((aligned - (rotation + 1)) % widths[field]);
                }
            }
        }
        settings[0] = (uint8)decoded[4];
        settings[1] = (uint8)decoded[2];
        xport_store_le32(settings + 12, decoded[1]);
        settings[16] = (uint8)decoded[3];
        settings[17] = (uint8)decoded[5];
        if (decoded[6] != 0 && decoded[6] == checksum && (sint32)decoded[0] >= 0 && p2_func_8002dfc8(78, 1) >= (sint32)decoded[0] && decoded[1] < 32 && decoded[2] < 32 && decoded[3] < 4 && decoded[4] < 2 && decoded[5] < 8)
        {
            ++decoded[1];
            ++decoded[2];
            xport_store_le32(settings + 12, decoded[1]);
            settings[1] = (uint8)decoded[2];
            scene = p2_func_800536f0(0);
            p2_func_80053530(scene, settings, (uint8)decoded[0]);
            w_u8(0x8008A134, 0);
            w_u8(0x8008A43C, decoded[0]);
            p2_func_80044508((settings[0] & 1) != 0 ? 2 : 1);
            return 1;
        }
        if (checksum != 0 && checksum == decoded[0] + decoded[1] + decoded[2] + decoded[3] && decoded[0] == 511 - (decoded[1] + decoded[2] + decoded[3]) && decoded[1] == 31 && decoded[2] == 31 && decoded[3] == 3)
        {
            xport_store_le32(settings + 12, decoded[3]);
            settings[1] = 3;
            settings[16] = 0;
            scene = p2_func_800536f0(0);
            p2_func_80053530(scene, settings, 0);
        }
        else
            error_until = r_u32(0x8008A3C8) + 3000;
    finish_iteration:
        clock = r_u32(0x8008A3C8);
    expire_messages:
        if (error_until < clock)
            error_until = 0;
        if (success_until < clock)
            success_until = 0;
        w_u32(0x8008A4F0, r_u32(0x8008A4F0) + 1);
    }
}

uint32 p2_func_8006b2b8(sint32 requested_mode, uint32 invalid, uint32 accepted)
{
    sint8 mode = (sint8)requested_mode;
    uint32 text;
    uint32 group;
    uint32 saved_frame;
    uint32 count;
    uint32 index;
    uint32 fade;
    uint32 result;
    FUNCTION_MARKER(0x8006B2B8, "SLUS_005.78");
    DrawSync(0);
    VSync(0);
    p2_func_800665d4();
    p2_func_800665b0();
    if (mode == 0)
        p2_func_80066bdc(0, 0xF00140, 0);
    else
        p2_func_80066af0(0);
    p2_func_8006b0e0(97, 110, mode);
    if ((uint8)invalid != 0 || (uint8)accepted != 0)
    {
        p2_func_800651bc(0xBE0000, 0x150140, 4096, 0xFFFFFF);
        text = (uint8)invalid != 0 ? 0x80010710 : r_u32(0x8008A3E0);
        p2_func_80064ff4(0xC300A0, text, 1, 0);
    }
    else if (mode == 0)
    {
        p2_func_80064ff4(0x4600A0, 0x80010724, 1, 0);
        p2_func_80064ff4(0xD20064, 0x80010730, 0, 0);
        p2_func_80064e74(0xC80041, 0, r_u32(r_u32(0x8008A4A4) + 28) + 220, 0);
    }
    group = r_u32(r_u32(0x8008A430) + 28) + 20;
    if (group != 0)
        p2_func_80064e74(0x2700E4, 0, group, 0);
    p2_func_80027228();
    group = r_u32(0x8008A430);
    if (group != 0)
    {
        saved_frame = r_u32(0x8008A518);
        count = r_u32(group + 24);
        w_u32(0x8008A518, r_u32(0x8008A4F0));
        if ((sint32)count > 0)
        {
            index = 0;
            do
            {
                p2_func_800275bc(r_u32(0x800B1524 + 8 * index));
                ++index;
            } while ((sint32)index < r_s32(r_u32(0x8008A430) + 24));
        }
        w_u32(0x8008A518, saved_frame);
    }
    fade = r_u32(0x8008A3D0);
    if (fade != 0)
    {
        --fade;
        w_u32(0x8008A3D0, fade);
        if (fade != 0)
            p2_func_800651bc(0, 0xF00140, (sint32)(fade << 12) / 15, 0xFFFFFF);
    }
    p2_func_80066698();
    result = p2_func_80066d78(r_u32(0x8008A3C8), 0x8008A3CC);
    w_u32(0x8008A3C8, result);
    return result;
}

uint32 p2_func_8006b0e0(uint32 x, uint32 y, sint8 interactive)
{
    uint32 entries;
    uint32 group;
    uint32 encoded;
    uint32 glyphs;
    uint32 position;
    uint32 index;
    uint32 source;
    uint32 words[2];
    uint32 delta;
    sint32 amplitude;
    uint32 old_amplitude;
    FUNCTION_MARKER(0x8006B0E0, "SLUS_005.78");
    entries = r_u32(0x800B1724);
    encoded = r_u8(entries - 4);
    group = r_u32(0x800B1720);
    encoded = r_u32(entries + 4 * encoded - 4);
    glyphs = r_u32(r_u32(0x800B7F54) + ((encoded << 3) & 0x1FF0) + 12);
    if (interactive == 0)
        p2_func_800652e4((x + 2) | ((y - 1) << 16), 0x140078, 4096, 0x800080);
    x += 4;
    y += 2;
    position = x + (y << 16);
    for (index = 0; index < 8; ++index)
    {
        source = glyphs + 8 * r_u8(0x8008A3D4 + index);
        words[0] = r_u32(source + 264);
        words[1] = r_u32(source + 268);
        w_u32(group + 4, words[0]);
        w_u32(group + 8, words[1]);
        if (group != 0)
            p2_func_80064e74(x | (y << 16), 0, group, 0);
        if (interactive != 0 && (sint32)index == r_s8(0x8008A3DC))
        {
            old_amplitude = r_u32(0x8008A3C0);
            delta = r_u32(0x8008A2A8);
            amplitude = (sint32)(old_amplitude + delta);
            w_u32(0x8008A3C0, (uint32)amplitude);
            if (amplitude >= 2048)
            {
                w_u32(0x8008A3C0, 2048);
                w_u32(0x8008A2A8, 0 - delta);
            }
            else if (amplitude < -2047)
            {
                w_u32(0x8008A3C0, (uint32)-2048);
                w_u32(0x8008A2A8, 0 - delta);
            }
            amplitude = r_s32(0x8008A3C0);
            if (amplitude > 0)
                p2_func_800652e4(position - 65537, 0x10000E, amplitude, 0x808080);
            else if (amplitude < 0)
                p2_func_800651bc(position - 65537, 0x10000E, (sint32)(0 - (uint32)amplitude), 0x808080);
        }
        position += 15;
        x += 15;
    }
    return 0;
}

uint32 p2_func_800652e4(uint32 position, uint32 extent, sint32 fraction, uint32 color)
{
    uint32 red;
    uint32 green;
    uint32 blue;
    uint32 packet;
    uint32 next;
    FUNCTION_MARKER(0x800652E4, "SLUS_005.78");
    if (fraction >= 4096)
    {
        red = color & 255;
        green = (color >> 8) & 255;
        blue = (color >> 16) & 255;
    }
    else if (fraction > 0)
    {
        red = ((uint32)fraction * (color & 255) + 2048) >> 12;
        green = ((uint32)fraction * ((color >> 8) & 255) + 2048) >> 12;
        blue = ((uint32)fraction * ((color >> 16) & 255) + 2048) >> 12;
    }
    else
        red = green = blue = 0;
    color = red | (green << 8) | (blue << 16);
    if (r_u32(0x8008A4B8) == 0)
        w_u32(0x8008A4B8, r_u32(0x8008A4F8));
    packet = r_u32(0x8008A4F8);
    next = packet + 12;
    w_u32(packet, (next & 0xFFFFFF) | 0x02000000);
    w_u32(packet + 4, 0xE1000020);
    w_u32(packet + 8, 0);
    w_u32(packet + 16, color | 0x62000000);
    w_u32(packet + 12, ((packet + 28) & 0xFFFFFF) | 0x03000000);
    w_u32(packet + 20, position);
    w_u32(packet + 24, extent);
    w_u32(0x8008A478, next);
    w_u32(0x8008A4F8, packet + 28);
    return 0x03000000;
}

uint32 p2_func_8005ed74(uint32 all_voices)
{
    uint32 voice;
    uint32 record;
    FUNCTION_MARKER(0x8005ED74, "SLUS_005.78");
    if (all_voices != 0)
    {
        w_u8(0x8008A350, 0);
        p2_func_80066358(0x8008B6A8, 0, 96);
        for (voice = 0; voice < 8; ++voice)
        {
            p2_func_8006a970((uint16)voice);
            w_u8(0x8008A244 + voice, voice);
        }
    }
    else
    {
        record = 0x8008B6CC;
        for (voice = 3; voice < 8; ++voice)
        {
            p2_sdk_800829d8(record, 0, 12);
            record += 12;
        }
    }
    return 0;
}

void p2_func_8002a448(uint32 mode, uint32 object)
{
    FUNCTION_MARKER(0x8002A448, "SLUS_005.78");
    if (object != 0)
        p2_func_80064d54(object - 4);
}

void p2_func_80064d54(uint32 allocation)
{
    uint32 total;
    uint32 bytes;
    FUNCTION_MARKER(0x80064D54, "SLUS_005.78");
    total = r_u32(0x8008A258);
    bytes = r_u32(allocation - 4);
    w_u32(0x8008A258, total - bytes);
    p2_sdk_80082a08(allocation);
}

void p2_func_80066430(void)
{
    uint16 handle;
    FUNCTION_MARKER(0x80066430, "SLUS_005.78");
    handle = r_u16(0x8008A530);
    if (handle != 0)
        p2_func_8006adc8((uint8)handle);
}

uint32 p2_func_8006adc8(uint8 handle)
{
    FUNCTION_MARKER(0x8006ADC8, "SLUS_005.78");
    p2_sdk_80075980((sint32)handle - 1);
    p2_func_800765b0();
    p2_sdk_80070a24(1);
    p2_sdk_800763ec(0);
    return p2_sdk_80076354(0, 0);
}

uint32 p2_func_8006ae90(uint16 handle)
{
    FUNCTION_MARKER(0x8006AE90, "SLUS_005.78");
    p2_func_80071a68((sint16)((sint32)(uint8)handle - 1));
    return p2_sdk_80079b14((sint32)(handle >> 8) - 1);
}

uint32 p2_func_80071a68(sint16 sequence)
{
    FUNCTION_MARKER(0x80071A68, "SLUS_005.78");
    return p2_func_800718ec(sequence);
}

uint32 p2_func_800718ec(sint16 sequence)
{
    uint32 result;
    uint32 table;
    uint32 offset = 0;
    uint32 index = 0;
    uint32 pointer;
    sint32 count;
    FUNCTION_MARKER(0x800718EC, "SLUS_005.78");
    p2_sdk_80078d1c(sequence, 0, 0, 1);
    p2_sdk_80079350(sequence);
    result = r_u32(0x800B7F3C);
    count = r_s16(0x800BA40A);
    result &= ~((uint32)1 << ((uint32)(sint32)sequence & 31));
    w_u32(0x800B7F3C, result);
    if (count > 0)
    {
        table = 0x800B9B00 + 4 * (uint32)(sint32)sequence;
        do
        {
            w_u32(offset + r_u32(table) + 144, 0);
            w_u8(offset + r_u32(table) + 60, 255);
            w_u8(offset + r_u32(table), 0);
            w_u16(offset + r_u32(table) + 62, 0);
            w_u16(offset + r_u32(table) + 64, 0);
            w_u32(offset + r_u32(table) + 148, 0);
            w_u32(offset + r_u32(table) + 152, 0);
            w_u16(offset + r_u32(table) + 66, 0);
            w_u32(offset + r_u32(table) + 164, 0);
            w_u32(offset + r_u32(table) + 160, 0);
            w_u32(offset + r_u32(table) + 156, 0);
            w_u16(offset + r_u32(table) + 68, 0);
            ++index;
            w_u16(offset + r_u32(table) + 116, 127);
            pointer = offset + r_u32(table);
            count = r_s16(0x800BA40A);
            offset += 172;
            result = (sint32)index < count;
            w_u16(pointer + 118, 127);
        } while (result != 0);
    }
    return result;
}

uint32 p2_func_8002dcd0(sint32 level, uint32 mode)
{
    uint32 player;
    uint32 previous;
    uint32 saved_stack = p2_guest_stack_pointer;
    uint32 camera_offset;
    uint32 fraction_offset;
    uint32 result;
    uint32 sprite;
    uint32 position;
    uint32 first_x;
    uint32 first_y;
    uint32 last_x;
    uint32 last_y;
    uint32 copy;
    FUNCTION_MARKER(0x8002DCD0, "SLUS_005.78");
    p2_guest_stack_pointer -= 0x58;
    camera_offset = p2_guest_stack_pointer + 0x10;
    fraction_offset = p2_guest_stack_pointer + 0x30;
    p2_func_8006540c();
    p2_func_80026f88();
    p2_func_8005ba34();
    player = r_u32(0x8008A114);
    w_u32(0x8008A4F4, 0);
    w_u8(player + 74, 0);
    w_u8(player + 73, 0);
    w_u8(player + 72, 0);
    previous = r_u32(0x8008A434);
    w_u32(camera_offset + 16, (uint32)-4096);
    w_u32(0x8008A444, 0xFFFFFFFF);
    w_u32(0x8008A178, 0);
    w_u32(0x8008A17C, 0);
    w_u32(camera_offset + 4, 143360);
    w_u32(camera_offset + 8, (uint32)-61440);
    w_u32(camera_offset + 12, 122880);
    w_u32(camera_offset + 24, 0);
    w_u32(camera_offset + 20, 0);
    w_u32(fraction_offset, 30);
    while ((p2_func_8002d7f8(level, mode) << 24) == 0)
    {
        p2_func_80053ca0(camera_offset);
        p2_func_800665d4();
        p2_func_8006de08(camera_offset);
        p2_func_800665b0();
        p2_func_80066bdc(0, 0xF00140, 0);
        sprite = r_u32(0x800B1718);
        for (copy = 0; copy < 2; ++copy)
        {
            last_x = r_u8(sprite + 6);
            first_x = r_u8(sprite + 4);
            last_y = r_u8(sprite + 7);
            first_y = r_u8(sprite + 5);
            position = (uint32)((319 - ((sint32)last_x - (sint32)first_x)) >> 1);
            position |= (uint32)((239 - ((sint32)last_y - (sint32)first_y)) >> 1) << 16;
            p2_func_80064e74(position, 0, sprite, 0);
        }
        p2_func_80027228();
        p2_func_80066698();
        previous = p2_func_80066d78(previous, fraction_offset);
    }
    result = p2_func_80025bfc(mode);
    p2_guest_stack_pointer = saved_stack;
    return result;
}

void p2_func_8006540c(void)
{
    uint32 offset;
    uint32 entries;
    FUNCTION_MARKER(0x8006540C, "SLUS_005.78");
    for (offset = 0; offset < 24; offset += 8)
    {
        w_u8(0x8008B708 + offset, 0);
        w_u32(0x8008B70C + offset, 0xFFFFFFFF);
    }
    entries = r_u32(0x800B1714);
    w_u32(0x8008A4F0, 0);
    p2_func_80027594(entries);
    p2_func_80027594(r_u32(0x800B1734));
}

void p2_func_80027594(uint32 entries)
{
    uint32 frame;
    uint32 count;
    FUNCTION_MARKER(0x80027594, "SLUS_005.78");
    if (entries != 0)
    {
        frame = r_u32(0x8008A518);
        count = r_u8(entries - 4);
        w_u32(entries - 4, count | (frame << 8));
    }
}

uint32 p2_func_80026f88(void)
{
    sint32 offset;
    FUNCTION_MARKER(0x80026F88, "SLUS_005.78");
    w_u32(0x8008A11C, 0);
    for (offset = 496; offset >= 0; offset -= 16)
        w_u32(0x8008B228 + (uint32)offset, 0xFFFFFFFF);
    return (uint32)-16;
}

uint32 p2_func_8002d7f8(sint32 level, uint32 mode)
{
    uint32 table;
    uint32 entry;
    uint32 active;
    uint32 asset;
    uint32 other;
    uint32 destination;
    uint32 bytes;
    uint32 base;
    uint32 capacity;
    uint32 loaded;
    uint32 index;
    uint32 offset;
    uint32 object;
    uint32 flags;
    sint32 state;
    FUNCTION_MARKER(0x8002D7F8, "SLUS_005.78");
    state = r_s8(0x8008A135);
    switch (state)
    {
        case 0:
            while (p2_func_80069ac0() != 0)
                ;
            if (level == 0)
            {
                w_u32(0x8008A130, 0);
                flags = r_u32(0x1F800388 + 4 * mode);
                w_u32(0x1F800388 + 4 * mode, flags | 0x40);
            }
            else if ((r_u32(0x1F800320) & 2) == 0 && (uint32)level - 96 >= 2)
                w_u8(0x8008A43C, (uint8)p2_func_8002dfc8((uint32)level, 1));
            p2_func_8005ed74(1);
            entry = r_u16(0x80083BEC);
            w_u32(0x8008A324, 0x80083BEC);
            if (entry != (uint32)level)
            {
                do
                {
                    table = r_u32(0x8008A324);
                    entry = r_u16(table + 12);
                    w_u32(0x8008A324, table + 12);
                } while (entry != 0xFFFF && entry != (uint32)level);
            }
            active = 3;
            if (r_u32(r_u32(0x8008A324) + 8) == 0)
            {
                active = 2;
                if (p2_func_80044780() == 0)
                    active = 1;
            }
            if (active != r_u32(0x8008A4FC))
            {
                p2_func_80044508(active);
                w_u32(0x8008A4FC, active);
            }
            table = r_u32(0x8008A324);
            base = r_u32(0x8008A424);
            offset = r_u32(0x800B7A30 + 4 * active);
            asset = r_u16(table + 2);
            w_u32(0x8008A4DC, base + offset);
            bytes = p2_func_8006a3ac(asset);
            base = r_u32(0x8008A4DC);
            capacity = r_u32(0x8008A284);
            w_u32(0x8008A4E0, base + bytes);
            w_u32(0x8008A420, capacity - (base + bytes - 1254400));
            p2_func_80026aa0();
            table = r_u32(0x8008A324);
            destination = r_u32(0x8008A4DC);
            asset = r_u16(table + 2);
            loaded = p2_func_8006a1ac(asset, destination, 0);
            asset = r_u16(r_u32(0x8008A324) + 2);
            w_u32(0x8008A4E4, loaded);
            w_u8(0x8008A135, 1);
            w_u32(loaded + 240, asset);
            break;
        case 1:
            if (((uint32)p2_func_8006a2ac() << 24) == 0)
                break;
            table = r_u32(0x8008A324);
            loaded = r_u32(0x8008A4E4);
            asset = r_u16(table + 6);
            other = r_u16(table + 2);
            w_u32(0x8008A488, loaded);
            w_u32(0x8008A46C, other + 2);
            p2_func_8006a530(asset, 1);
            p2_func_800440a8(r_u32(0x8008A4E4));
            p2_func_8002d468(r_u32(0x8008A4E4));
            p2_func_80034a9c(r_u32(0x8008A4E4));
            p2_func_80055468(r_u32(0x8008A4E4));
            if (r_u16(r_u32(0x8008A4E4) + 262) != 0 && (sint32)p2_func_8006a3ac(r_u16(r_u32(0x8008A324) + 4)) > 0)
            {
                destination = p2_func_80025bc0(mode);
                asset = r_u16(r_u32(0x8008A324) + 4);
                p2_func_8006a1ac(asset, destination, 0);
                w_u8(0x8008A135, 2);
                break;
            }
            goto load_sound;
        case 2:
            if (((uint32)p2_func_8006a2ac() << 24) != 0)
            {
                destination = p2_func_80025bc0(mode);
                p2_func_8006624c(destination, 1, 0);
                w_u8(0x8008A135, 3);
            }
            break;
        case 3:
            if (DrawSync(1) > 0)
                break;
        load_sound:
            destination = p2_func_80025bc0(mode);
            asset = r_u16(r_u32(0x8008A324) + 6);
            p2_func_8006a1ac(asset, destination, 0);
            w_u8(0x8008A135, 4);
            break;
        case 4:
            if (((uint32)p2_func_8006a2ac() << 24) != 0)
            {
                destination = p2_func_80025bc0(mode);
                asset = r_u16(r_u32(0x8008A324) + 6);
                bytes = p2_func_8006a3ac(asset);
                p2_func_8006aa88(destination, bytes);
                w_u8(0x8008A135, 5);
            }
            break;
        case 5:
            if (p2_func_8006aaa8() == 0)
                break;
            if (r_u16(r_u32(0x8008A4E4) + 262) == 0)
            {
                index = r_u32(0x8008A46C);
                w_u32(0x8008A46C, index + 1);
                offset = p2_func_8006a3c4(index);
                base = r_u32(0x8008A488);
                w_u32(0x8008A428, base + offset);
            }
            if ((sint32)p2_func_8006a3c4(r_u32(0x8008A46C)) > 0)
            {
                offset = p2_func_8006a3c4(r_u32(0x8008A46C));
                base = r_u32(0x8008A488);
                loaded = p2_func_8006aa48(base + offset);
                w_u32(0x8008A460, loaded);
            }
            table = r_u32(0x8008A324);
            index = r_u32(0x8008A46C);
            object = r_u32(0x8008A4E4);
            asset = r_u16(table + 6);
            w_u32(0x8008A46C, index + 1);
            w_u16(object + 272, 0);
            w_u8(object + 264, 0);
            w_u32(object + 240, asset + 1);
            p2_func_80066358(0x800B996A, 0, 25);
            destination = 0;
            if (r_u16(r_u32(0x8008A4E4) + 262) == 0)
                destination = r_u32(0x80083CF4 + 4 * (uint32)(sint32)r_s8(0x8008A43C));
            p2_func_80025d3c(destination, mode);
            w_u8(0x8008A135, 6);
            break;
        default:
            if (state == 6 && p2_func_80069ac0() == 0)
                w_u8(0x8008A135, 0);
            break;
    }
    return r_s8(0x8008A135) == 0;
}
