#include "game.h"

uint32 p2_func_80069ac0(void)
{
    uint32 asset;
    uint32 base;
    uint32 object;
    uint32 source;
    uint32 destination;
    uint32 loaded;
    uint32 palette;
    uint32 sample;
    uint32 bytes;
    uint32 aligned;
    uint32 state;
    sint32 index;
    sint32 shifted;
    uint16 handle;
    FUNCTION_MARKER(0x80069AC0, "SLUS_005.78");
    if (r_s8(0x8008A49C) == 0)
        return 0;
    if (((uint32)p2_func_8006a2ac() << 24) == 0)
        return 1;
    state = (uint32)(sint32)r_s8(0x8008A49C);
    switch ((sint32)state)
    {
        case -4:
        case 4:
            object = r_u32(0x8008A4E8);
            base = r_u32(0x8008A4E4);
            index = r_s8(object + 14);
            asset = r_u32(base + 240) + 9 * (uint32)index;
            if ((sint32)p2_func_8006a3c4(asset + 8) < 0 || r_u32(0x8008A48C) == 0)
            {
                w_u8(0x8008A49C, 0);
                return 1;
            }
            destination = p2_func_80025bc0(0);
            object = r_u32(0x8008A4E8);
            base = r_u32(0x8008A4E4);
            index = r_s8(object + 14);
            asset = r_u32(base + 240);
            source = r_u32(object + 128);
            asset += 9 * (uint32)index;
            handle = (uint16)p2_func_8006ac44(asset + 2, asset + 8, source, destination);
            w_u16(0x8008A530, handle);
            if (handle == 0)
                return 1;
            p2_func_80066404();
            if (r_s8(0x8008A49C) == -4)
            {
                index = r_s8(0x8008A394);
                w_u8(0x8008A49C, 0);
                p2_func_80025d3c((uint32)index, 0);
            }
            else
                w_u8(0x8008A49C, 0);
            return 1;
        case 1:
            asset = r_u16(0x8008A37C);
            destination = r_u32(0x8008A380);
            w_u8(0x8008A49C, 2);
            loaded = p2_func_8006a1ac(asset + 1, destination + 188, 0);
            w_u32(0x8008A384, loaded);
            return 1;
        case 2:
            if ((sint32)p2_func_8006a3ac(r_u16(0x8008A37C) + 2) > 0 && r_u32(0x8008A48C) != 0)
            {
                base = r_u32(0x8008A380);
                asset = r_u16(0x8008A37C);
                destination = r_u32(base + 128);
                w_u8(0x8008A49C, 3);
                loaded = p2_func_8006a1ac(asset + 2, destination, 0);
                source = r_u32(0x8008A384);
                index = r_s8(0x8008A514);
                w_u32(0x8008A38C, loaded);
                p2_func_8006624c(source, (uint32)(index + 1), 0);
                index = r_s8(0x8008A514);
                source = r_u32(0x8008A388);
                loaded = p2_func_800602f4(source);
                w_u32(0x8008A388, loaded);
                return 1;
            }
            index = r_s8(0x8008A514);
            source = r_u32(0x8008A384);
            p2_func_8006624c(source, (uint32)(index + 1), 0);
            index = r_s8(0x8008A514);
            source = r_u32(0x8008A388);
            loaded = p2_func_800602f4(source);
            asset = r_u16(0x8008A37C);
            w_u32(0x8008A388, loaded);
            w_u8(0x8008A49C, 5);
            goto load_model;
        case 3:
            asset = r_u16(0x8008A37C);
            w_u8(0x8008A49C, 5);
        load_model:
            bytes = p2_func_8006a3ac(asset + 7);
            shifted = (sint32)(bytes + 2047);
            if (shifted < 0)
                shifted = (sint32)(bytes + 4094);
            shifted >>= 11;
            if (shifted < 87)
            {
                base = r_u32(0x8008A380);
                aligned = 178176;
            }
            else
            {
                bytes = p2_func_8006a3ac(r_u16(0x8008A37C) + 7);
                shifted = (sint32)(bytes + 2047);
                if (shifted < 0)
                    shifted = (sint32)(bytes + 4094);
                base = r_u32(0x8008A380);
                aligned = (uint32)(shifted >> 11) << 11;
            }
            asset = r_u16(0x8008A37C);
            loaded = p2_func_8006a1ac(asset + 5, base + aligned, 0);
            w_u32(0x8008A390, loaded);
            return 1;
        case 5:
            if ((sint32)p2_func_8006a3ac(r_u16(0x8008A37C) + 6) > 0)
            {
                asset = r_u16(0x8008A37C);
                destination = r_u32(0x8008A380);
                w_u8(0x8008A49C, 6);
                loaded = p2_func_8006a1ac(asset + 6, destination + 188, 0);
                w_u32(0x8008A384, loaded);
                return 1;
            }
            object = r_u32(0x8008A390);
            w_u8(0x8008A49C, 7);
            w_u32(0x8008A384, 0);
            p2_func_80068790(object, object, 0);
            asset = r_u16(0x8008A37C);
            destination = r_u32(0x8008A380);
            p2_func_8006a1ac(asset + 7, destination, 0);
            return 1;
        case 6:
            asset = r_u16(0x8008A37C);
            w_u8(0x8008A49C, 7);
            if ((sint32)p2_func_8006a3ac(asset) > 0)
            {
                source = r_u32(0x8008A384);
                index = r_s8(0x8008A514);
                palette = (uint32)index;
            }
            else
            {
                index = r_s8(0x8008A514);
                source = r_u32(0x8008A384);
                palette = (uint32)(index + 2);
            }
            p2_func_8006624c(source, palette, 0);
            object = r_u32(0x8008A390);
            index = r_s8(0x8008A514);
            p2_func_80068790(object, object, index);
            return 1;
        case 7:
            if (r_u32(0x8008A384) != 0)
            {
                destination = r_u32(0x8008A380);
                asset = r_u16(0x8008A37C);
                w_u32(0x8008A384, 0);
                p2_func_8006a1ac(asset + 7, destination, 0);
                return 1;
            }
            source = r_u32(0x8008A390);
            base = r_u32(0x8008A380);
            w_u8(0x8008A49C, 0);
            w_u32(base + 20, base + 188);
            p2_func_8006637c((uint8 *)psx_addr(base + 156, 32), (const uint8 *)psx_addr(source, 32), 32);
            object = r_u32(0x8008A380);
            p2_func_80068a24(object, object + 156);
            p2_func_80025828(r_u32(0x8008A380));
            base = r_u32(0x8008A380);
            sample = r_u32(0x8008A38C);
            source = r_u32(0x8008A388);
            w_u32(base + 128, sample);
            w_u32(base + 124, source);
            return 0;
        default:
            return 1;
    }
}

uint32 p2_func_8006a970(uint16 voice)
{
    uint32 mask = (uint32)1 << (((uint32)voice + 16) & 31);
    uint32 row;
    sint32 scale;
    sint32 gain;
    sint64 product;
    FUNCTION_MARKER(0x8006A970, "SLUS_005.78");
    p2_sdk_80070bc0(0, mask);
    row = 0x800A9688 + 12 * (uint32)voice;
    w_u32(row + 8, 0);
    w_u32(0x800A96E8, mask);
    w_u32(0x800A96EC, 3);
    scale = r_s32(row + 8);
    gain = r_s32(row);
    product = (sint64)scale * gain;
    w_u16(0x800A96F0, (uint16)((uint64)product >> 12));
    scale = r_s32(row + 8);
    gain = r_s32(row + 4);
    product = (sint64)scale * gain;
    w_u16(0x800A96F2, (uint16)((uint64)product >> 12));
    return p2_sdk_8007101c(0x800A96E8);
}

uint32 p2_func_80026aa0(void)
{
    uint32 first;
    uint32 second;
    uint32 base;
    uint32 object;
    uint32 index;
    FUNCTION_MARKER(0x80026AA0, "SLUS_005.78");
    w_u32(0x8008A4E8, 0);
    p2_func_80066358(0x8008B1D0, 0, 36);
    first = r_u32(0x8008A4E0);
    base = r_u32(0x8008A284);
    second = base + 953344;
    w_u32(0x8008B1F0, base + 87520);
    w_u32(0x8008B1E8, base + 5600);
    for (index = 0; index < 2; ++index)
    {
        object = index == 0 ? first : second;
        w_u32(0x8008A304 + 4 * index, object);
        w_u32(object + 20, object + 188);
        w_u16(object + 18, 0);
    }
    object = r_u32(0x8008A308);
    w_u32(0x8008B1D4, object);
    w_u8(object + 14, 255);
    return 0xFFFFFFFF;
}

uint32 p2_func_80053ca0(uint32 camera_offset)
{
    uint32 current;
    uint32 next;
    uint32 duration;
    uint32 animation;
    uint32 object;
    uint32 entry;
    uint32 old_animation;
    uint32 flags;
    uint32 x;
    uint32 y;
    uint32 result;
    FUNCTION_MARKER(0x80053CA0, "SLUS_005.78");
    current = r_u32(0x8008A444);
    if ((sint32)current < 0)
    {
        next = r_u32(0x8008A19C);
        w_u32(0x8008A504, 0);
        w_u32(0x8008A444, 0);
        w_u32(0x8008A33C, next);
    }
    else
    {
        entry = r_u32(0x8008A33C);
        duration = r_u32(entry + 12);
        ++current;
        w_u32(0x8008A444, current);
        if ((sint32)duration < (sint32)current)
        {
            w_u32(0x8008A444, 0);
            w_u32(0x8008A33C, entry + 16);
        }
    }
    entry = r_u32(0x800AC028);
    animation = r_u32(0x8008B4B8 + ((entry >> 18) & 0x3C));
    object = r_u32(animation + 232);
    if (object == 0)
    {
        w_u8(0x7E, 1);
        w_u8(0x7D, 1);
        w_u8(0x7F, 0);
        w_u32(0xDC, 0xFFFFFFFF);
        w_u32(0xCC, 0xFFFFFFFF);
        w_u32(0xBC, 0xFFFFFFFF);
        w_u32(0xAC, 0xFFFFFFFF);
    }
    entry = r_u32(0x8008A33C);
    current = r_u32(entry);
    old_animation = r_u32(object + 104);
    next = r_u32(animation + 80 + 4 * current);
    if (old_animation != next)
    {
        flags = r_u32(object + 28);
        w_u32(object + 104, next);
        w_u16(object + 130, 65535);
        w_u32(object + 28, flags | 0x10000);
        w_u32(camera_offset + 16, r_u32(entry + 4));
        w_u32(camera_offset + 20, r_u32(entry + 8));
    }
    y = r_u32(camera_offset + 8);
    duration = r_u32(camera_offset + 20);
    x = r_u32(camera_offset + 4);
    flags = r_u32(camera_offset + 16);
    y += duration;
    x += flags;
    w_u32(camera_offset + 8, y);
    w_u32(camera_offset + 4, x);
    if ((sint32)x > 143359)
    {
        result = 0 - r_u32(camera_offset + 16);
        w_u32(camera_offset + 4, 143360);
        w_u32(camera_offset + 16, result);
    }
    else
    {
        result = (sint32)x > -143360;
        if ((sint32)x <= -143360)
        {
            result = 0 - r_u32(camera_offset + 16);
            w_u32(camera_offset + 4, (uint32)-143360);
            w_u32(camera_offset + 16, result);
        }
    }
    w_u32(camera_offset + 0, object);
    return result;
}

uint32 p2_func_800401a8(uint32 object)
{
    uint32 previous;
    FUNCTION_MARKER(0x800401A8, "SLUS_005.78");
    w_u8(object + 13, r_u8(object + 13) & 0x7F);
    previous = r_u32(0x8008A4EC);
    w_u32(0x8008A4EC, object);
    w_u32(object, previous);
    return previous;
}

uint32 p2_func_8005dc00(sint32 x, sint32 y)
{
    uint32 absolute_x = (uint32)x;
    uint32 absolute_y = (uint32)y;
    uint32 numerator;
    uint32 quotient;
    uint32 angle;
    FUNCTION_MARKER(0x8005DC00, "SLUS_005.78");
    if (x == 0 && y == 0)
        return 4096;
    if (x < 0)
        absolute_x = 0 - absolute_x;
    if (y < 0)
        absolute_y = 0 - absolute_y;
    if ((sint32)absolute_y < (sint32)absolute_x)
    {
        numerator = (absolute_y << 11) + (uint32)((sint32)absolute_x >> 1);
        quotient = absolute_x == 0 ? 0xFFFFFFFF : numerator / absolute_x;
        if (absolute_x == 0)
            xport_mips_break(7);
        angle = (uint32)(sint32)r_s16(0x800851A8 + 2 * quotient);
    }
    else
    {
        numerator = (absolute_x << 11) + (uint32)((sint32)absolute_y >> 1);
        quotient = absolute_y == 0 ? 0xFFFFFFFF : numerator / absolute_y;
        if (absolute_y == 0)
            xport_mips_break(7);
        angle = 1024 - (uint32)(sint32)r_s16(0x800851A8 + 2 * quotient);
    }
    if (x < 0)
        angle = 2048 - angle;
    if (y < 0)
        angle = 4096 - angle;
    return angle & 0xFFF;
}

uint32 p2_func_8001a6ac(uint32 object)
{
    uint32 flags;
    uint32 result;
    uint8 byte_flags;
    sint32 frame;
    sint32 count;
    FUNCTION_MARKER(0x8001A6AC, "SLUS_005.78");
    flags = r_u32(object + 28);
    if ((flags & 0x10000) != 0)
    {
        frame = r_s16(object + 130);
        if (frame == 2 || frame == r_s16(r_u32(object + 104) + 6))
        {
            flags = r_u32(object + 28);
            w_u16(object + 130, 0);
            w_u32(object + 28, (flags & 0xFFFEFFFF) | 0x20000);
            goto mark_changed;
        }
    }
    frame = r_s16(object + 130);
    if (frame < 0)
        w_u16(object + 130, 0);
    else
    {
        count = r_s16(r_u32(object + 104) + 6);
        if (frame >= count)
            w_u16(object + 130, (uint16)(count - 1));
    }
mark_changed:
    result = r_u32(object + 28);
    byte_flags = r_u8(object + 13);
    result |= 1;
    w_u32(object + 28, result);
    w_u8(object + 13, byte_flags | 0x40);
    return result;
}

P2PathProbeResult p2_setup_camera_projection_with_selection(sint32 scale)
{
    P2PathProbeResult output;
    uint32 camera;
    uint32 point;
    uint32 ax;
    uint32 ay;
    uint32 az;
    uint32 bx;
    uint32 by;
    uint32 bz;
    uint32 first;
    uint32 second;
    uint32 vertices[4][3];
    VECTOR normal;
    sint32 projection = scale >> 12;
    FUNCTION_MARKER(0x8002417C, "SLUS_005.78");
    camera = r_u32(0x8008A114);
    w_u16(camera + 304, 4096);
    w_u16(camera + 300, 0);
    w_u16(camera + 302, 0);
    vertices[0][0] = 0;
    vertices[0][1] = 0;
    vertices[0][2] = 0;
    vertices[1][0] = (uint32)-40960;
    vertices[1][1] = 30720;
    vertices[1][2] = (uint32)(scale >> 4);
    vertices[2][0] = (uint32)-40960;
    vertices[2][1] = (uint32)-30720;
    vertices[2][2] = (uint32)(scale >> 4);
    for (point = 0; point < 4; ++point)
    {
        if (point == 1)
        {
            vertices[1][0] = 40960;
            vertices[1][1] = (uint32)-30720;
        }
        else if (point == 2)
        {
            vertices[2][0] = 40960;
            vertices[2][1] = 30720;
        }
        else if (point == 3)
        {
            vertices[1][0] = (uint32)-40960;
            vertices[1][1] = 30720;
        }
        first = (point & 1) != 0 ? 2 : 1;
        second = (point & 1) != 0 ? 1 : 2;
        ax = vertices[first][0] - vertices[0][0];
        ay = vertices[first][1] - vertices[0][1];
        az = vertices[first][2] - vertices[0][2];
        bx = vertices[second][0] - vertices[first][0];
        by = vertices[second][1] - vertices[first][1];
        bz = vertices[second][2] - vertices[first][2];
        first = (uint32)(((sint64)(sint32)ay * (sint32)bz) >> 12);
        second = (uint32)(((sint64)(sint32)by * (sint32)az) >> 12);
        normal.vx = (sint32)(first - second);
        first = (uint32)(((sint64)(sint32)az * (sint32)bx) >> 12);
        second = (uint32)(((sint64)(sint32)bz * (sint32)ax) >> 12);
        normal.vy = (sint32)(first - second);
        first = (uint32)(((sint64)(sint32)ax * (sint32)by) >> 12);
        second = (uint32)(((sint64)(sint32)bx * (sint32)ay) >> 12);
        normal.vz = (sint32)(first - second);
        while ((uint32)normal.vx + 26754 > 0xD103 || (uint32)normal.vy + 26754 > 0xD103 || (uint32)normal.vz + 26754 > 0xD103)
        {
            normal.vx >>= 1;
            normal.vy >>= 1;
            normal.vz >>= 1;
        }
        VectorNormal(&normal, &normal);
        camera = r_u32(0x8008A114);
        w_u16(camera + 308 + 8 * point, (uint16)normal.vx);
        w_u16(camera + 310 + 8 * point, (uint16)normal.vy);
        w_u16(camera + 312 + 8 * point, (uint16)normal.vz);
    }
    xport_gte_write_control(26, (uint32)projection);
    xport_gte_write_control(21, 0);
    xport_gte_write_control(22, 0);
    xport_gte_write_control(23, 0);
    output.fallback_frame = (uint32)normal.vx;
    output.value = (sint32)SetFogNearFar(0x8000, 0xFFFF, projection);
    return output;
}

uint32 p2_func_8002417c(sint32 scale)
{
    return (uint32)p2_setup_camera_projection_with_selection(scale).value;
}

uint32 p2_func_8006de08(uint32 camera_offset)
{
    uint32 saved_stack = p2_guest_stack_pointer;
    uint32 context_address;
    uint32 light_address;
    uint32 object;
    uint32 saved_context;
    uint32 player;
    uint32 flags;
    uint32 x;
    uint32 y;
    uint32 z;
    uint32 angle;
    uint32 animation;
    uint32 child;
    uint32 light_context;
    uint32 projection;
    sint32 frame;
    sint32 count;
    uint8 byte_flags;
    FUNCTION_MARKER(0x8006DE08, "SLUS_005.78");
    /* Addressable local arrays retain PSX addresses in DRAM */
    p2_guest_stack_pointer -= 0x150;
    context_address = p2_guest_stack_pointer + 0x10;
    light_address = p2_guest_stack_pointer + 0xD0;
    object = r_u32(camera_offset + 0);
    byte_flags = r_u8(object + 13);
    flags = r_u32(object + 28);
    w_u8(object + 124, 0);
    w_u8(object + 13, byte_flags & 0xFD);
    saved_context = r_u32(0x8008A4E8);
    w_u32(object + 132, 0);
    w_u32(object + 28, flags | 0x2000);
    p2_func_80066358(context_address, 0, 188);
    player = r_u32(0x8008A114);
    w_u32(0x8008A4E8, context_address);
    w_u16(0x1F800040, 128);
    w_u16(0x1F80003E, 128);
    w_u16(0x1F80003C, 128);
    w_u32(player + 8, 0);
    w_u32(player + 4, 0);
    w_u32(player + 12, (uint32)-49152);
    w_u32(player + 24, 0);
    w_u32(player + 20, 0);
    w_u32(player + 16, 0);
    x = r_u32(player + 16);
    y = r_u32(player + 20);
    z = r_u32(player + 24);
    w_u32(object + 60, x);
    w_u32(object + 64, y);
    w_u32(object + 68, z);
    w_u32(object + 72, 40960);
    p2_func_800401a8(object);
    player = r_u32(0x8008A114);
    x = r_u32(player + 16);
    y = r_u32(player + 20);
    z = r_u32(player + 24);
    w_u32(object + 16, x);
    w_u32(object + 20, y);
    w_u32(object + 24, z);
    x = r_u32(object + 16);
    y = r_u32(camera_offset + 4);
    w_u32(object + 16, x + y);
    x = r_u32(object + 20);
    y = r_u32(camera_offset + 8);
    w_u32(object + 20, x + y);
    x = r_u32(object + 24);
    y = r_u32(camera_offset + 12);
    w_u32(object + 24, x + y);
    w_u16(object + 84, 0x7FFF);
    z = r_u32(camera_offset + 24);
    x = r_u32(camera_offset + 16);
    angle = p2_func_8005dc00((sint32)(0 - z), (sint32)(0 - x));
    animation = r_u32(object + 104);
    w_u16(object + 88, (angle + 2048) & 0xFFF);
    frame = 0;
    if (animation != 0)
    {
        frame = r_s16(object + 130);
        count = r_s16(animation + 6);
        frame = frame < count - 1 ? frame + 1 : 0;
    }
    w_u16(object + 130, (uint16)frame);
    p2_func_8001a6ac(object);
    light_context = r_u32(0x8008A4E8);
    projection = r_u32(0x1F80038C);
    w_u16(light_address + 36, (uint16)-4096);
    w_u32(light_address + 68, 2056);
    w_u32(light_address + 72, 2056);
    w_u32(light_address + 76, 2056);
    w_u32(light_address, 0);
    w_u8(light_address + 13, 0);
    w_u8(light_address + 14, 0);
    w_u16(light_address + 34, 0);
    w_u16(light_address + 32, 0);
    w_u32(light_context + 84, light_address);
    if (projection != 955733 || r_s16(r_u32(0x8008A114) + 304) != 4096)
    {
        w_u32(0x1F80038C, 955733);
        p2_func_8002417c(955733);
    }
    player = r_u32(0x8008A114);
    byte_flags = r_u8(player);
    w_u32(player + 340, 1228800);
    w_u8(player, byte_flags & 0xFE);
    child = r_u32(object + 156);
    w_u32(object + 156, 0);
    w_u8(child + 13, r_u8(child + 13) | 1);
    p2_func_80016ef4(0);
    w_u32(object + 156, child);
    w_u8(child + 13, r_u8(child + 13) & 0xFE);
    p2_func_800401c8(object);
    w_u8(object + 124, 1);
    w_u32(0x8008A4E8, saved_context);
    p2_guest_stack_pointer = saved_stack;
    return 1;
}

uint32 p2_func_8002e344(void)
{
    uint32 result;
    uint32 light;
    uint32 updated = 0;
    uint32 flags;
    uint32 kind;
    uint32 x;
    uint32 y;
    uint32 z;
    uint32 count;
    uint32 slot;
    uint8 byte_flags;
    FUNCTION_MARKER(0x8002E344, "SLUS_005.78");
    result = r_u32(0x8008A4E8);
    light = r_u32(result + 84);
    w_u8(0x1F800338, 0);
    while (light != 0)
    {
        flags = r_u8(light + 14);
        if ((flags & 4) != 0)
        {
            kind = r_u32(light + 16);
            w_u8(light + 14, flags & 0xFB);
            kind = ((kind >> 12) & 31) + 39;
            if (kind == 47)
                kind = (flags & 8) != 0 ? 42 : 43;
            switch (kind)
            {
                case 42:
                case 53:
                    byte_flags = r_u8(light + 13);
                    if ((byte_flags & 1) == 0)
                    {
                        flags = r_u8(light + 14);
                        w_u8(light + 13, byte_flags | 1);
                        w_u8(light + 14, flags | 1);
                    }
                    break;
                case 43:
                case 54:
                    byte_flags = r_u8(light + 13);
                    if ((byte_flags & 1) != 0)
                    {
                        flags = r_u8(light + 14);
                        w_u8(light + 13, byte_flags & 0xFE);
                        w_u8(light + 14, flags | 1);
                    }
                    break;
                case 51:
                    byte_flags = r_u8(light + 13);
                    flags = r_u8(light + 14);
                    w_u8(light + 13, byte_flags ^ 1);
                    w_u8(light + 14, flags | 1);
                    break;
                default:
                    break;
            }
        }
        if ((p2_func_8005bc58(light + 16, 1) << 24) != 0)
            w_u8(light + 14, r_u8(light + 14) | 4);
        flags = r_u8(light + 14);
        if ((flags & 1) != 0)
        {
            w_u8(light + 14, flags & 0xFE);
            if (updated == 0)
                p2_func_8002e00c(light);
            if ((r_u8(light + 14) & 2) != 0)
            {
                x = r_u32(light + 44);
                y = r_u32(light + 48);
                z = r_u32(light + 52);
                w_u32(light + 32, x);
                w_u32(light + 36, y);
                w_u32(light + 40, z);
            }
            else
            {
                updated = 1;
                if (r_u32(light + 44) + 26754 > 0xD103 || r_u32(light + 48) + 26754 > 0xD103 || r_u32(light + 52) + 26754 > 0xD103)
                {
                    for (;;)
                    {
                        x = r_u32(light + 44);
                        z = r_u32(light + 52);
                        w_u32(light + 44, (uint32)((sint32)x >> 1));
                        y = r_u32(light + 48);
                        x = r_u32(light + 44);
                        z = (uint32)((sint32)z >> 1);
                        w_u32(light + 52, z);
                        w_u32(light + 48, (uint32)((sint32)y >> 1));
                        if (x + 26754 > 0xD103)
                            continue;
                        if (r_u32(light + 48) + 26754 > 0xD103)
                            continue;
                        if (z + 26754 <= 0xD103)
                            break;
                    }
                }
                VectorNormalS((VECTOR *)psx_addr(light + 44, 12), (SVECTOR *)psx_addr(light + 32, 6));
            }
        }
        result = r_u32(light + 12) & 0x20100;
        if (result == 0)
        {
            count = r_u8(0x1F800338);
            result = count < 3;
            if (result != 0)
            {
                x = r_u32(light + 68);
                slot = 0x1F80033C + 2 * count;
                w_u16(slot, x);
                w_u16(slot + 6, r_u32(light + 72));
                count = r_u8(0x1F800338);
                z = r_u32(light + 76);
                result = count + 1;
                w_u8(0x1F800338, result);
                w_u16(slot + 12, z);
            }
        }
        light = r_u32(light);
    }
    w_u32(0x1F800334, 0);
    x = r_u16(0x1F80003C);
    y = r_u16(0x1F80003E);
    z = r_u16(0x1F800040);
    xport_gte_write_control(13, x << 4);
    xport_gte_write_control(14, y << 4);
    xport_gte_write_control(15, z << 4);
    x = r_u32(0x1F80033C);
    y = r_u32(0x1F800340);
    xport_gte_write_control(16, x);
    xport_gte_write_control(17, y);
    x = r_u32(0x1F800344);
    y = r_u32(0x1F800348);
    z = r_u32(0x1F80034C);
    xport_gte_write_control(18, x);
    xport_gte_write_control(19, y);
    xport_gte_write_control(20, z);
    return result;
}

uint32 p2_func_80016ef4(uint32 scene_offset)
{
    uint32 result;
    uint32 object;
    uint32 next;
    uint32 target;
    sint32 kind;
    FUNCTION_MARKER(0x80016EF4, "SLUS_005.78");
    if (scene_offset != 0)
    {
        result = p2_func_80016d0c() << 24;
        if (result == 0)
            return result;
    }
    w_u32(0x1F8002C8, 0x800B1830);
    w_u16(0x1F8002CC, 0);
    p2_func_800665d4();
    p2_func_800165c8(scene_offset);
    p2_func_80012eb8();
    if (scene_offset != 0)
    {
        p2_func_8005d170();
        object = r_u32(scene_offset + 108);
        if (object != 0)
        {
            do
            {
                next = r_u32(object + 4);
                p2_func_8002a210(scene_offset + 100, object);
                if (r_u8(object + 12) != 0)
                {
                    w_u32(object + 4, r_u32(scene_offset + 8));
                    w_u32(scene_offset + 8, object);
                }
                else
                {
                    p2_func_800401a8(object);
                    if (r_s8(object + 126) != 0)
                        w_u32(object + 144, 4096);
                    kind = r_s8(object + 124);
                    if (kind != 0)
                    {
                        target = r_u32(0x800B7A60 + 4 * (uint32)kind);
                        p2_dispatch_object_callback(target, object);
                    }
                }
                object = next;
            } while (next != 0);
            w_u32(scene_offset + 108, 0);
        }
    }
    return p2_func_80066698();
}

void p2_func_800165c8(uint32 scene_offset)
{
    uint32 saved_stack = p2_guest_stack_pointer;
    uint32 first_matrix;
    uint32 second_matrix;
    uint32 left;
    uint32 right;
    uint32 camera;
    uint32 matrix;
    uint32 target;
    uint32 index;
    uint32 point;
    uint32 distance;
    uint32 old_distance;
    uint32 difference;
    uint32 sum;
    uint32 x;
    uint32 y;
    uint32 z;
    uint32 words[3];
    uint16 transpose[9];
    sint32 cosine;
    sint32 sine;
    sint32 quotient;
    sint32 coefficient;
    sint64 product;
    uint8 flags;
    FUNCTION_MARKER(0x800165C8, "SLUS_005.78");
    p2_guest_stack_pointer -= 0x50;
    first_matrix = p2_guest_stack_pointer + 0x10;
    second_matrix = p2_guest_stack_pointer + 0x28;
    p2_func_80027228();
    p2_func_8002e344();
    camera = r_u32(0x8008A114);
    flags = r_u8(camera);
    if ((flags & 1) == 0)
    {
        w_u8(camera, flags | 1);
        p2_func_80022d38();
    }
    camera = r_u32(0x8008A114);
    cosine = p2_func_8005dbb8((sint16)(r_u16(camera + 40) + 1024)) >> 2;
    camera = r_u32(0x8008A114);
    sine = p2_func_8005dbb8((sint16)(0 - r_u16(camera + 40))) >> 2;
    w_u16(0x1F800008, (uint16)cosine);
    w_u16(0x1F80000E, (uint16)sine);
    w_u16(0x1F80000A, (uint16)(0 - (uint32)sine));
    w_u16(0x1F800010, (uint16)cosine);
    MulMatrix0((MATRIX *)psx_addr(0x80010180, 18), (MATRIX *)psx_addr(0x1F800000, 18), (MATRIX *)psx_addr(first_matrix, 18));
    camera = r_u32(0x8008A114);
    cosine = p2_func_8005dbb8((sint16)(r_u16(camera + 42) + 1024)) >> 2;
    camera = r_u32(0x8008A114);
    sine = p2_func_8005dbb8((sint16)(0 - r_u16(camera + 42))) >> 2;
    camera = r_u32(0x8008A114);
    w_u16(0x1F800014, (uint16)cosine);
    w_u16(0x1F800020, (uint16)(0 - (uint32)sine));
    w_u16(0x1F800018, (uint16)sine);
    w_u16(0x1F800024, (uint16)cosine);
    right = 0x1F800014;
    if (r_s16(camera + 44) == 2048)
        left = first_matrix;
    else
    {
        MulMatrix0((MATRIX *)psx_addr(first_matrix, 18), (MATRIX *)psx_addr(right, 18), (MATRIX *)psx_addr(second_matrix, 18));
        camera = r_u32(0x8008A114);
        cosine = p2_func_8005dbb8((sint16)(r_u16(camera + 44) - 1024)) >> 2;
        camera = r_u32(0x8008A114);
        sine = p2_func_8005dbb8((sint16)(2048 - r_u16(camera + 44))) >> 2;
        w_u16(0x1F800028, (uint16)cosine);
        w_u16(0x1F80002E, (uint16)sine);
        w_u16(0x1F80002A, (uint16)(0 - (uint32)sine));
        w_u16(0x1F800030, (uint16)cosine);
        left = 0x1F800028;
        right = second_matrix;
    }
    MulMatrix0((MATRIX *)psx_addr(left, 18), (MATRIX *)psx_addr(right, 18), (MATRIX *)psx_addr(0x1F800264, 18));
    x = r_u32(0x1F800264);
    y = r_u32(0x1F800268);
    xport_gte_write_control(0, x);
    xport_gte_write_control(1, y);
    x = r_u32(0x1F80026C);
    y = r_u32(0x1F800270);
    z = r_u32(0x1F800274);
    xport_gte_write_control(2, x);
    xport_gte_write_control(3, y);
    xport_gte_write_control(4, z);
    camera = r_u32(0x8008A114);
    p2_sdk_8007ec64(camera + 4, 0x1F800278);
    x = r_u32(0x1F800278);
    y = r_u32(0x1F80027C);
    z = r_u32(0x1F800280);
    w_u32(0x1F800278, 0 - x);
    w_u32(0x1F80027C, 0 - y);
    w_u32(0x1F800280, 0 - z);
    transpose[0] = r_u16(0x1F800264);
    transpose[1] = r_u16(0x1F80026A);
    transpose[2] = r_u16(0x1F800270);
    transpose[3] = r_u16(0x1F800266);
    transpose[4] = r_u16(0x1F80026C);
    transpose[5] = r_u16(0x1F800272);
    transpose[6] = r_u16(0x1F800268);
    transpose[7] = r_u16(0x1F80026E);
    transpose[8] = r_u16(0x1F800274);
    for (index = 0; index < 9; ++index)
        w_u16(0x1F800244 + 2 * index, transpose[index]);
    for (index = 0; index < 3; ++index)
        words[index] = r_u32(0x1F800244 + 4 * index);
    for (index = 0; index < 3; ++index)
        w_u32(0x1F800284 + 4 * index, words[index]);
    x = r_u32(0x1F800250);
    y = (uint32)(sint32)r_s16(0x1F800254);
    w_u32(0x1F800290, x);
    w_u16(0x1F800294, y);
    x = r_u32(0x1F800244);
    y = r_u32(0x1F800248);
    xport_gte_write_control(0, x);
    xport_gte_write_control(1, y);
    x = r_u32(0x1F80024C);
    y = r_u32(0x1F800250);
    z = r_u32(0x1F800254);
    xport_gte_write_control(2, x);
    xport_gte_write_control(3, y);
    xport_gte_write_control(4, z);
    for (point = 0; point < 5; ++point)
    {
        camera = r_u32(0x8008A114);
        xport_gte_write_data(0, r_u32(camera + 300 + 8 * point));
        xport_gte_write_data(1, r_u32(camera + 304 + 8 * point));
        xport_gte_mvmva(0x486012);
        w_u32(0x1F800390, xport_gte_read_data(25));
        w_u32(0x1F800394, xport_gte_read_data(26));
        w_u32(0x1F800398, xport_gte_read_data(27));
        target = 0x1F8002D4 + 12 * point;
        w_u16(target, r_u32(0x1F800390));
        w_u16(target + 2, r_u32(0x1F800394));
        z = r_u32(0x1F800398);
        w_u16(target + 4, z);
        camera = r_u32(0x8008A114);
        x = r_u32(camera + 4);
        coefficient = r_s16(target);
        sum = (uint32)((sint64)((sint32)x >> 6) * coefficient);
        y = r_u32(camera + 8);
        coefficient = r_s16(target + 2);
        sum += (uint32)((sint64)((sint32)y >> 6) * coefficient);
        x = r_u32(camera + 12);
        sum += (uint32)((sint64)((sint32)x >> 6) * (sint16)z);
        w_u32(0x1F8002D0 + 12 * point, 0 - (uint32)((sint32)sum >> 6));
    }
    x = r_u32(0x80010180);
    y = r_u32(0x80010184);
    xport_gte_write_control(0, x);
    xport_gte_write_control(1, y);
    x = r_u32(0x80010188);
    y = r_u32(0x8001018C);
    z = r_u32(0x80010190);
    xport_gte_write_control(2, x);
    xport_gte_write_control(3, y);
    xport_gte_write_control(4, z);
    matrix = r_u32(0x8008A0C8);
    for (index = 0; index < 3; ++index)
        words[index] = r_u32(0x80010180 + 4 * index);
    for (index = 0; index < 3; ++index)
        w_u32(matrix + 4 * index, words[index]);
    x = r_u32(0x8001018C);
    y = (uint32)(sint32)r_s16(0x80010190);
    w_u32(matrix + 12, x);
    w_u16(matrix + 16, y);
    matrix = r_u32(0x8008A0C8);
    camera = r_u32(0x8008A114);
    w_u32(matrix + 28, 0);
    w_u32(matrix + 24, 0);
    w_u32(matrix + 20, 0);
    distance = r_u32(camera + 340);
    old_distance = r_u32(0x1F8002B4);
    if (distance != old_distance)
    {
        w_u32(0x1F8002B4, distance);
        if (distance == 0)
            xport_mips_break(7);
        quotient = distance == 0 ? -1 : 0x40000000 / (sint32)distance;
        product = (sint64)0x400000 * quotient;
        coefficient = (sint32)(product >> 18);
        w_u32(0x1F80030C, (uint32)coefficient);
        w_u32(0x1F8002B8, (uint32)coefficient);
    }
    if (scene_offset != 0)
    {
        distance = r_u32(scene_offset + 152);
        if (distance != 0)
        {
            old_distance = r_u32(scene_offset + 148);
            difference = distance - old_distance;
            if (difference == 0)
                xport_mips_break(7);
            quotient = difference == 0 ? -1 : 0x40000000 / (sint32)difference;
            product = (sint64)0x100000 * quotient;
            coefficient = (sint32)(product >> 18);
            w_u32(0x1F80030C, (uint32)coefficient);
            w_u32(0x1F8002BC, (uint32)coefficient);
        }
        else
            xport_gte_write_data(8, 0);
        if (r_u16(0x8008A532) < 4096)
        {
            w_u32(0x1F8002AC, 0xFFFFFFFF);
            p2_func_80016c54(scene_offset, 0);
            p2_func_80011e04();
        }
    }
    p2_guest_stack_pointer = saved_stack;
}

uint32 p2_func_8005bc58(uint32 animation_offset, sint32 count)
{
    uint32 owner;
    uint32 result = 0;
    uint32 index = 0;
    uint32 value;
    uint32 key;
    uint32 cursor;
    uint32 remaining;
    FUNCTION_MARKER(0x8005BC58, "SLUS_005.78");
    owner = r_u32(0x8008A114);
    if (r_s8(owner + 1) == 0)
        return 0;
    if (count > 0)
    {
        do
        {
            key = r_u32(animation_offset) & 0xFFF;
            if (key != 0)
            {
                w_u32(animation_offset, key);
                value = r_u32(owner + 140);
                remaining = (uint32)(sint32)r_s8(owner + 1);
                cursor = owner + 144;
                for (;;)
                {
                    --remaining;
                    if (key == (value & 0xFFF))
                    {
                        w_u32(animation_offset, value);
                        result = 1;
                        break;
                    }
                    if (remaining == 0)
                        break;
                    value = r_u32(cursor);
                    cursor += 4;
                }
            }
            ++index;
            animation_offset += 4;
        } while ((sint32)index < count);
    }
    return result;
}

sint32 p2_func_8005dbb8(sint16 angle)
{
    uint32 phase = (uint16)angle & 0xFFF;
    FUNCTION_MARKER(0x8005DBB8, "SLUS_005.78");
    if (phase < 2048)
        return r_u16(0x800841A8 + 2 * phase);
    return -(sint32)r_u16(0x800841A8 + 2 * (phase & 0x7FF));
}

uint32 p2_func_80022d38(void)
{
    uint32 camera;
    uint32 x;
    uint32 y;
    uint32 z;
    uint32 base;
    uint32 sum;
    uint32 length;
    uint32 mode;
    uint32 angle;
    uint32 result;
    sint32 component;
    FUNCTION_MARKER(0x80022D38, "SLUS_005.78");
    camera = r_u32(0x8008A114);
    x = r_u32(camera + 16);
    base = r_u32(camera + 4);
    x -= base;
    y = r_u32(camera + 20);
    base = r_u32(camera + 8);
    y -= base;
    z = r_u32(camera + 24);
    base = r_u32(camera + 12);
    z -= base;
    mode = p2_func_80026c14();
    component = (sint32)x >> 6;
    sum = (uint32)((sint64)component * component);
    component = (sint32)y >> 6;
    sum += (uint32)((sint64)component * component);
    component = (sint32)z >> 6;
    sum += (uint32)((sint64)component * component);
    if (mode != 0)
        length = p2_func_8002ab38(sum);
    else if ((sint32)sum >= 0)
        length = p2_sdk_8007e794(sum);
    else
        length = p2_sdk_8007e794(sum >> 2) << 1;
    angle = p2_func_8005dc00((sint32)z, (sint32)x);
    if ((sint16)angle != 4096)
        w_u16(r_u32(0x8008A114) + 42, angle);
    angle = (uint32)p2_func_8005dcd4((sint32)y, (sint32)length);
    result = 0 - angle;
    if ((sint16)angle != 4096)
    {
        camera = r_u32(0x8008A114);
        result &= 0xFFF;
        w_u16(camera + 40, result);
    }
    return result;
}

uint32 p2_func_80026c14(void)
{
    FUNCTION_MARKER(0x80026C14, "SLUS_005.78");
    if (r_s32(0x8008B200) <= 0)
        return 0;
    if ((r_u32(0x8008B1F8) & 1) != 0)
        return 1;
    return 0xFFFFFFFF;
}

sint32 p2_func_8005dcd4(sint32 height, sint32 length)
{
    uint32 magnitude = (uint32)height;
    uint32 denominator = (uint32)length;
    uint32 numerator;
    uint32 index;
    uint32 angle;
    FUNCTION_MARKER(0x8005DCD4, "SLUS_005.78");
    if (length == 0)
        return 4096;
    if (height < 0)
        magnitude = 0 - magnitude;
    if (length < 0)
    {
        denominator = 0 - denominator;
        height = (sint32)(0 - (uint32)height);
    }
    if ((sint32)magnitude < (sint32)denominator)
    {
        numerator = (magnitude << 11) + (uint32)((sint32)denominator >> 1);
        index = numerator / denominator;
        angle = (uint32)(sint32)r_s16(0x800861AC + 2 * index);
    }
    else
        angle = 1024;
    if (height < 0 && (sint32)angle > 0)
        angle = 4096 - angle;
    return (sint16)angle;
}

void p2_func_80061174(void)
{
    FUNCTION_MARKER(0x80061174, "SLUS_005.78");
}

uint32 p2_func_80011338(uint32 point_offset, uint32 radius)
{
    sint32 x;
    sint32 y;
    sint32 z;
    sint32 coefficient;
    uint32 product_x;
    uint32 product_y;
    uint32 product_z;
    uint32 sum;
    uint32 bias;
    uint32 distance;
    uint32 limit = 0 - radius;
    uint32 index;
    uint32 plane;
    const uint32 planes[4] = {0x1F8002DC, 0x1F8002F4, 0x1F8002E8, 0x1F800300};
    FUNCTION_MARKER(0x80011338, "SLUS_005.78");
    sum = r_u32(point_offset);
    coefficient = r_s16(0x1F8002D4);
    x = (sint32)sum >> 6;
    product_x = (uint32)((sint64)x * coefficient);
    sum = r_u32(point_offset + 4);
    coefficient = r_s16(0x1F8002D6);
    y = (sint32)sum >> 6;
    product_y = (uint32)((sint64)y * coefficient);
    sum = r_u32(point_offset + 8);
    coefficient = r_s16(0x1F8002D8);
    z = (sint32)sum >> 6;
    product_z = (uint32)((sint64)z * coefficient);
    bias = r_u32(0x1F8002D0);
    sum = product_x + product_y + product_z;
    distance = (uint32)((sint32)sum >> 6) + bias;
    if ((sint32)limit >= (sint32)distance)
        return 0;
    sum = r_u32(0x1F8002B4) - distance;
    if ((sint32)limit >= (sint32)sum)
        return 0;
    for (index = 0; index < 4; ++index)
    {
        plane = planes[index];
        coefficient = r_s16(plane + 4);
        product_x = (uint32)((sint64)x * coefficient);
        coefficient = r_s16(plane + 6);
        product_y = (uint32)((sint64)y * coefficient);
        coefficient = r_s16(plane + 8);
        product_z = (uint32)((sint64)z * coefficient);
        bias = r_u32(plane);
        sum = product_x + product_y + product_z;
        distance = (uint32)((sint32)sum >> 6) + bias;
        if ((sint32)limit >= (sint32)distance)
            return 0;
    }
    return 1;
}

uint32 p2_func_80012eb8(void)
{
    uint32 saved_stack = p2_guest_stack_pointer;
    uint32 vector_offset;
    uint32 object;
    uint32 next;
    uint32 matrix;
    uint32 parent;
    uint32 target;
    uint32 flags;
    uint32 saved_ot;
    uint32 saved_fog;
    uint32 radius;
    uint32 scale;
    uint32 reciprocal;
    uint32 special;
    uint32 x;
    uint32 y;
    uint32 z;
    uint32 value;
    uint32 xx;
    uint32 yy;
    uint32 zz;
    uint32 xy;
    uint32 xz;
    uint32 yz;
    uint32 wz;
    uint32 wy;
    uint32 wx;
    uint32 words[4];
    uint32 index;
    uint32 offset;
    uint32 result;
    uint32 offsets[9] = {0, 6, 12, 2, 8, 14, 4, 10, 16};
    sint32 kind;
    sint32 variant;
    sint32 mode;
    sint32 qw;
    sint32 qx;
    sint32 qy;
    sint32 qz;
    sint32 cosine;
    sint32 sine;
    sint32 coefficient;
    sint64 product;
    uint8 byte_flags;
    FUNCTION_MARKER(0x80012EB8, "SLUS_005.78");
    p2_guest_stack_pointer -= 0x68;
    vector_offset = p2_guest_stack_pointer + 0x10;
    object = r_u32(0x8008A4EC);
    matrix = r_u32(0x8008A0C8);
    w_u32(0x1F8002AC, 0);
    w_u32(0x8008A0C8, matrix + 32);
    while (object != 0)
    {
        kind = r_s8(object + 125);
        saved_ot = 0;
        if (kind != 1)
        {
            variant = r_s8(object + 127);
            if (variant != 0)
                target = r_u32(0x800ABD50 + 4 * (uint32)variant);
            else
                target = r_u32(0x800BA380 + 4 * (uint32)kind);
            p2_dispatch_object_callback(target, object);
            if ((r_u32(object + 28) & 0x80000) != 0)
                goto red_tint;
            if ((r_u32(object + 228) & 0x4000) != 0)
            {
                parent = r_u32(r_u32(object + 132) + 44);
                parent = r_u32(parent + 72);
                flags = r_u32(0x1F800320);
                z = r_u8(parent + 630);
                y = r_u8(parent + 629);
                x = (uint32)(sint32)r_s8(parent + 628);
                w_u32(0x1F800320, flags | 0x20);
                w_u32(0x1F8002B0, (z << 16) | (y << 8) | x);
            }
        }
        else
        {
            value = r_u16(0x8008A532);
            if (value != 0)
            {
                if (value < 4096)
                {
                    saved_ot = r_u32(0x8008A44C);
                    saved_fog = r_u32(0x1F8002B8);
                    target = r_u32(0x8008A4D0);
                    w_u32(0x1F8002B8, 1310);
                    w_u32(0x8008A44C, target);
                }
            }
            else
            {
                if ((p2_func_800447f0() << 24) != 0)
                    w_u32(0x1F800320, r_u32(0x1F800320) | 4);
                mode = p2_func_80047b1c(object);
                if (mode >= 2)
                {
                    if (mode == 2)
                    {
                        flags = r_u32(0x1F800320);
                        w_u32(0x1F8002B0, 0xFFFFFF);
                        w_u32(0x1F800320, flags | 0x20);
                    }
                    else if (mode == 4)
                        w_u32(0x1F8002A8, r_u32(0x8008A418));
                    else if (mode == 5)
                    {
                        x = r_u32(0x8008A474);
                        flags = r_u32(0x1F800320);
                        target = r_u32(0x8008A520);
                        w_u32(0x1F8002B0, x);
                        w_u32(0x1F800320, flags | 0x20);
                        w_u32(0x1F8002A8, target);
                    }
                }
                else if ((r_u32(object + 28) & 0x80000) != 0)
                {
                red_tint:
                    flags = r_u32(0x1F800320);
                    w_u32(0x1F8002B0, 255);
                    w_u32(0x1F800320, flags | 0x20);
                }
            }
        }
        flags = r_u32(object + 28);
        next = r_u32(object);
        value = 0;
        if ((flags & 0x2000) == 0)
        {
            parent = r_u32(object + 132);
            if (parent != 0)
                value = r_u32(parent + 40);
        }
        if (value != 0)
        {
            x = r_u32(object + 16);
            y = r_u32(object + 20);
            z = r_u32(object + 24);
            w_u32(vector_offset, x);
            w_u32(vector_offset + 4, y);
            w_u32(vector_offset + 8, z);
            PushMatrix();
            parent = r_u32(r_u32(object + 132) + 40);
            x = r_u32(parent);
            y = r_u32(parent + 4);
            xport_gte_write_control(0, x);
            xport_gte_write_control(1, y);
            x = r_u32(parent + 8);
            y = r_u32(parent + 12);
            z = r_u32(parent + 16);
            xport_gte_write_control(2, x);
            xport_gte_write_control(3, y);
            xport_gte_write_control(4, z);
            p2_sdk_8007ec64(vector_offset, r_u32(0x8008A0C8) + 20);
            PopMatrix();
            matrix = r_u32(0x8008A0C8);
            for (index = 0; index < 3; ++index)
            {
                offset = 20 + 4 * index;
                parent = r_u32(r_u32(object + 132) + 40);
                x = r_u32(matrix + offset);
                y = r_u32(parent + offset);
                w_u32(matrix + offset, x + y);
            }
        }
        else
        {
            matrix = r_u32(0x8008A0C8);
            x = r_u32(object + 16);
            y = r_u32(object + 20);
            z = r_u32(object + 24);
            w_u32(matrix + 20, x);
            w_u32(matrix + 24, y);
            w_u32(matrix + 28, z);
        }
        qw = r_s16(object + 84);
        if (qw != 0x7FFF)
        {
            qx = r_s16(object + 86);
            qy = r_s16(object + 88);
            qz = r_s16(object + 90);
            xx = (uint32)(qx * qx);
            yy = (uint32)(qy * qy);
            zz = (uint32)(qz * qz);
            xy = (uint32)(qx * qy);
            xz = (uint32)(qx * qz);
            yz = (uint32)(qy * qz);
            wz = (uint32)(qw * qz);
            matrix = r_u32(0x8008A0C8);
            wy = (uint32)(qw * qy);
            w_u16(matrix, (uint32)((sint32)(0x8000000 - (yy + zz)) >> 15));
            w_u16(matrix + 8, (uint32)((sint32)(0x8000000 - (xx + zz)) >> 15));
            w_u16(matrix + 16, (uint32)((sint32)(0x8000000 - (xx + yy)) >> 15));
            wx = (uint32)(qw * qx);
            w_u16(matrix + 6, (uint32)((sint32)(xy - wz) >> 15));
            w_u16(matrix + 2, (uint32)((sint32)(xy + wz) >> 15));
            w_u16(matrix + 12, (uint32)((sint32)(xz + wy) >> 15));
            w_u16(matrix + 4, (uint32)((sint32)(xz - wy) >> 15));
            w_u16(matrix + 14, (uint32)((sint32)(yz - wx) >> 15));
            w_u16(matrix + 10, (uint32)((sint32)(yz + wx) >> 15));
            target = r_u32(0x8008A0C8);
        }
        else
        {
            matrix = r_u32(0x8008A0C8);
            x = r_u32(matrix - 32);
            y = r_u32(matrix - 28);
            xport_gte_write_control(0, x);
            xport_gte_write_control(1, y);
            x = r_u32(matrix - 24);
            y = r_u32(matrix - 20);
            z = r_u32(matrix - 16);
            xport_gte_write_control(2, x);
            xport_gte_write_control(3, y);
            xport_gte_write_control(4, z);
            if (r_s16(object + 86) != 0)
            {
                cosine = p2_func_8005dbb8((sint16)(1024 - r_u16(object + 88))) >> 2;
                sine = p2_func_8005dbb8(r_s16(object + 88)) >> 2;
                w_u16(0x1F800014, (uint16)cosine);
                w_u16(0x1F800020, (uint16)(0 - (uint32)sine));
                w_u16(0x1F800018, (uint16)sine);
                w_u16(0x1F800024, (uint16)cosine);
                target = p2_sdk_8007e944(0x1F800014);
                x = r_u32(target);
                y = r_u32(target + 4);
                xport_gte_write_control(0, x);
                xport_gte_write_control(1, y);
                x = r_u32(target + 8);
                y = r_u32(target + 12);
                z = r_u32(target + 16);
                xport_gte_write_control(2, x);
                xport_gte_write_control(3, y);
                xport_gte_write_control(4, z);
                cosine = p2_func_8005dbb8((sint16)(1024 - r_u16(object + 86))) >> 2;
                sine = p2_func_8005dbb8(r_s16(object + 86)) >> 2;
                w_u16(0x1F800008, (uint16)cosine);
                w_u16(0x1F80000E, (uint16)sine);
                w_u16(0x1F80000A, (uint16)(0 - (uint32)sine));
                w_u16(0x1F800010, (uint16)cosine);
                target = p2_sdk_8007ea34(0x1F800000, r_u32(0x8008A0C8));
            }
            else if (r_s16(object + 88) != 0)
            {
                cosine = p2_func_8005dbb8((sint16)(1024 - r_u16(object + 88))) >> 2;
                sine = p2_func_8005dbb8(r_s16(object + 88)) >> 2;
                w_u16(0x1F800014, (uint16)cosine);
                w_u16(0x1F800020, (uint16)(0 - (uint32)sine));
                w_u16(0x1F800018, (uint16)sine);
                w_u16(0x1F800024, (uint16)cosine);
                target = p2_sdk_8007ea34(0x1F800014, r_u32(0x8008A0C8));
            }
            else
            {
                for (index = 0; index < 4; ++index)
                    words[index] = r_u32(matrix - 32 + 4 * index);
                for (index = 0; index < 4; ++index)
                    w_u32(matrix + 4 * index, words[index]);
                w_u16(matrix + 16, (uint16)r_s16(matrix - 16));
                goto rotation_ready;
            }
        }
        x = r_u32(target);
        y = r_u32(target + 4);
        xport_gte_write_control(0, x);
        xport_gte_write_control(1, y);
        x = r_u32(target + 8);
        y = r_u32(target + 12);
        z = r_u32(target + 16);
        xport_gte_write_control(2, x);
        xport_gte_write_control(3, y);
        xport_gte_write_control(4, z);
    rotation_ready:
        special = (r_u32(object + 124) & 0xFF00FF00) == 0x02001100;
        w_u8(0x8008A2D8, special);
        if (special == 0 && (r_s8(object + 124) >= 4 || r_s8(object + 127) == 2) && (r_u32(object + 28) & 0x30000000) != 0)
        {
            scale = r_u16(object + 116);
            if (scale == 0)
                xport_mips_break(7);
            reciprocal = scale == 0 ? 0xFFFFFFFF : 0x1000000 / scale;
            w_u32(0x1F8002C0, scale);
            w_u32(0x1F8002C4, reciprocal);
            matrix = r_u32(0x8008A0C8);
            for (index = 0; index < 9; ++index)
            {
                if (index != 0)
                    scale = r_u32(0x1F8002C0);
                coefficient = r_s16(matrix + offsets[index]);
                product = (sint64)(sint32)scale * coefficient;
                w_u16(matrix + offsets[index], (uint16)((uint64)product >> 12));
            }
            target = r_u32(0x8008A0C8);
            x = r_u32(target);
            y = r_u32(target + 4);
            xport_gte_write_control(0, x);
            xport_gte_write_control(1, y);
            x = r_u32(target + 8);
            y = r_u32(target + 12);
            z = r_u32(target + 16);
            xport_gte_write_control(2, x);
            xport_gte_write_control(3, y);
            xport_gte_write_control(4, z);
        }
        flags = r_u32(object + 28);
        byte_flags = r_u8(object + 13);
        w_u32(object + 28, flags | 0x200);
        if ((byte_flags & 3) == 0 && (p2_func_80011338(object + 60, r_u32(object + 72)) << 24) != 0)
        {
            p2_func_800275bc(r_u32(object + 44));
            if ((r_u32(object + 28) & 0x40000) != 0)
                w_u8(0x1F8002A4, 1);
            matrix = r_u32(0x8008A0C8);
            radius = r_u32(object + 72);
            p2_func_8002e824(matrix + 20, (sint32)radius >> 5);
            flags = r_u32(object + 28);
            variant = r_s8(object + 127);
            w_u32(object + 28, flags & 0xFFFFFDFF);
            if (variant == 2 && r_s8(0x8008A2D8) == 0)
            {
                matrix = r_u32(0x8008A0C8);
                for (index = 0; index < 9; ++index)
                {
                    scale = r_u32(object + 92);
                    coefficient = r_s16(matrix + offsets[index]);
                    product = (sint64)(sint32)scale * coefficient;
                    w_u16(matrix + offsets[index], (uint16)((uint64)product >> 12));
                }
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
            }
            w_u16(0x1F8002A6, r_u16(object + 14));
            if (r_s16(object + 56) != 0)
                p2_func_80011980(object, object + 50);
            parent = r_u32(object + 156);
            if (parent != 0)
            {
                if (r_s8(object + 127) == 2 || r_s8(object + 125) == 17)
                    w_u8(parent + 13, r_u8(parent + 13) | 1);
                else
                    p2_func_8005d1b0(object);
            }
        }
        if ((r_u8(object + 13) & 2) != 0)
        {
            p2_func_800401c8(object);
            p2_func_80025eac(object);
        }
        else
        {
            p2_func_80012298(object);
            if ((r_u32(object + 28) & 0x20200) != 0x200)
            {
                w_u32(0x8008A2D4, object + 160);
                p2_func_80012424(object, object);
                byte_flags = r_u8(object + 13);
                flags = r_u32(object + 28);
                w_u8(object + 13, byte_flags & 0xBF);
                w_u32(object + 28, flags & 0xFFFDFFFF);
                w_u16(0x1F8002A6, 0);
                flags = r_u32(object + 28);
                target = r_u32(0x8008A2D4);
                w_u32(object + 28, flags & 0xFFFFFFFE);
                if (target != object + 160)
                {
                    x = r_u32(object + 60);
                    xx = r_u32(object + 72);
                    y = r_u32(object + 64);
                    yy = r_u32(object + 76);
                    z = r_u32(object + 68);
                    zz = r_u32(object + 80);
                    w_u32(object + 60, (uint32)((sint32)(x + xx) >> 1));
                    w_u32(object + 64, (uint32)((sint32)(y + yy) >> 1));
                    w_u32(object + 68, (uint32)((sint32)(z + zz) >> 1));
                    xx = r_u32(object + 72);
                    x = r_u32(object + 60);
                    yy = r_u32(object + 76);
                    y = r_u32(object + 64);
                    zz = r_u32(object + 80);
                    z = r_u32(object + 68);
                    w_u32(object + 72, xx - x);
                    w_u32(object + 76, yy - y);
                    w_u32(object + 80, zz - z);
                    x = r_u32(object + 72);
                    y = r_u32(object + 76);
                    w_u32(object + 72, p2_func_8002aa1c(x, y, zz - z));
                }
            }
            w_u16(object + 14, 0);
            x = r_u32(0x1F8002A8);
            y = r_u32(0x8008A520);
            if (x == y)
                w_u32(object + 92, 0);
            if (saved_ot != 0)
            {
                p2_func_80064a58();
                w_u32(0x8008A44C, saved_ot);
                w_u32(0x1F8002B8, saved_fog);
            }
            flags = r_u32(object + 28);
            variant = r_s8(object + 126);
            w_u32(object + 28, flags & 0xFFFBFFFF);
            if (variant != 0)
                w_u32(object + 144, 4096);
            kind = r_s8(object + 124);
            if (kind != 0)
                p2_dispatch_object_callback(r_u32(0x800B7A60 + 4 * (uint32)kind), object);
        }
        flags = r_u32(0x1F800320);
        object = next;
        w_u8(0x8008A2D8, 0);
        w_u32(0x1F8002A8, 0);
        w_u32(0x1F8002C0, 0);
        w_u8(0x1F8002A4, 0);
        w_u32(0x1F800320, flags & 0xFFFFFFDA);
    }
    result = r_u32(0x8008A0C8) - 32;
    w_u32(0x8008A0C8, result);
    p2_guest_stack_pointer = saved_stack;
    return result;
}

uint32 p2_func_8002e824(uint32 vector_offset, sint32 radius)
{
    uint32 flags;
    FUNCTION_MARKER(0x8002E824, "SLUS_005.78");
    flags = r_u32(0x1F800320);
    w_u32(0x8008A328, vector_offset);
    w_u16(0x8008A32C, (uint16)radius);
    w_u32(0x1F800320, flags & 0xFFFFFFFE);
    return 0xFFFFFFFE;
}

uint32 p2_func_80012298(uint32 object_offset)
{
    uint32 flags;
    uint32 matrix;
    uint32 x;
    uint32 y;
    uint32 z;
    uint32 value;
    uint32 dx;
    uint32 dy;
    uint32 dz;
    uint32 cursor;
    uint32 index;
    uint32 result;
    FUNCTION_MARKER(0x80012298, "SLUS_005.78");
    flags = r_u32(object_offset + 28);
    if ((flags & 0x20) != 0 && r_s8(object_offset + 126) != 0)
    {
        if ((flags & 0x20200) != 0x200)
        {
            w_u32(object_offset + 220, 0xFFFFFFFF);
            w_u32(object_offset + 204, 0xFFFFFFFF);
            w_u32(object_offset + 188, 0xFFFFFFFF);
            w_u32(object_offset + 172, 0xFFFFFFFF);
            return 0xFFFFFFFF;
        }
        matrix = r_u32(0x8008A0C8);
        x = r_u32(object_offset + 60);
        value = r_u32(matrix + 20);
        dx = value - x;
        value = r_u32(matrix + 24);
        y = r_u32(object_offset + 64);
        dy = value - y;
        value = r_u32(matrix + 28);
        z = r_u32(object_offset + 68);
        dz = value - z;
        dy += r_u32(object_offset + 76);
        w_u32(object_offset + 60, r_u32(object_offset + 60) + dx);
        w_u32(object_offset + 64, r_u32(object_offset + 64) + dy);
        w_u32(object_offset + 68, r_u32(object_offset + 68) + dz);
        cursor = object_offset;
        index = 0;
        for (;;)
        {
            result = r_u32(cursor + 172);
            ++index;
            if ((sint32)result <= 0)
                return result;
            w_u32(cursor + 160, r_u32(cursor + 160) + dx);
            w_u32(cursor + 164, r_u32(cursor + 164) + dy);
            w_u32(cursor + 168, r_u32(cursor + 168) + dz);
            result = index < 4;
            cursor += 16;
            if (result == 0)
                return result;
        }
    }
    matrix = r_u32(0x8008A0C8);
    x = r_u32(object_offset + 160);
    value = r_u32(matrix + 20);
    w_u32(object_offset + 60, x + value);
    y = r_u32(object_offset + 164);
    value = r_u32(matrix + 24);
    w_u32(object_offset + 64, y + value);
    z = r_u32(object_offset + 168);
    value = r_u32(matrix + 28);
    result = z + value;
    w_u32(object_offset + 68, result);
    return result;
}

sint32 p2_func_8005dd68(sint32 height, sint32 length)
{
    uint32 magnitude = (uint32)height;
    uint32 denominator = (uint32)length;
    uint32 numerator;
    uint32 index;
    uint32 angle;
    sint32 rotated;
    FUNCTION_MARKER(0x8005DD68, "SLUS_005.78");
    if (length == 0)
        angle = 4096;
    else
    {
        if (height < 0)
            magnitude = 0 - magnitude;
        if (length < 0)
        {
            denominator = 0 - denominator;
            height = (sint32)(0 - (uint32)height);
        }
        if ((sint32)magnitude < (sint32)denominator)
        {
            numerator = (magnitude << 11) + (uint32)((sint32)denominator >> 1);
            index = numerator / denominator;
            angle = (uint32)(sint32)r_s16(0x800861AC + 2 * index);
        }
        else
            angle = 1024;
        if (height < 0 && (sint32)angle > 0)
            angle = 4096 - angle;
    }
    rotated = 1024 - (sint16)angle;
    if (rotated < 0)
        rotated += 4096;
    else if (rotated >= 4097)
        rotated -= 4096;
    return (sint16)rotated;
}

uint32 p2_func_80012424(uint32 node_offset, uint32 owner_offset)
{
    uint32 saved_stack = p2_guest_stack_pointer;
    uint32 velocity_offset;
    uint32 first_matrix;
    uint32 second_matrix;
    uint32 node;
    uint32 matrix;
    uint32 target;
    uint32 left;
    uint32 right;
    uint32 kind;
    uint32 x;
    uint32 y;
    uint32 z;
    uint32 px;
    uint32 py;
    uint32 pz;
    uint32 value;
    uint32 packed;
    uint32 rate;
    uint32 phase;
    uint32 alpha;
    uint32 beta;
    uint32 gamma;
    uint32 xx;
    uint32 yy;
    uint32 zz;
    uint32 xy;
    uint32 xz;
    uint32 yz;
    uint32 wz;
    uint32 wy;
    uint32 wx;
    uint32 slot;
    uint32 radius;
    uint32 scale;
    uint32 minimum_offset;
    uint32 maximum_offset;
    uint32 index;
    uint32 words[4];
    uint32 result;
    sint32 qw;
    sint32 qx;
    sint32 qy;
    sint32 qz;
    sint32 cosine;
    sint32 sine;
    sint32 coefficient;
    sint64 product;
    FUNCTION_MARKER(0x80012424, "SLUS_005.78");
    matrix = r_u32(0x8008A0C8);
    p2_guest_stack_pointer -= 0x88;
    velocity_offset = p2_guest_stack_pointer + 0x10;
    first_matrix = p2_guest_stack_pointer + 0x18;
    second_matrix = p2_guest_stack_pointer + 0x30;
    minimum_offset = p2_guest_stack_pointer + 0x30;
    maximum_offset = p2_guest_stack_pointer + 0x48;
    node = r_u32(node_offset + 8);
    w_u32(0x8008A0C8, matrix + 32);
    while (node != 0)
    {
        if ((r_u8(owner_offset + 13) & 0x40) != 0)
            p2_func_80019cf4(owner_offset, node);
        matrix = r_u32(0x8008A0C8);
        x = r_u32(matrix - 32);
        y = r_u32(matrix - 28);
        xport_gte_write_control(0, x);
        xport_gte_write_control(1, y);
        x = r_u32(matrix - 24);
        y = r_u32(matrix - 20);
        z = r_u32(matrix - 16);
        xport_gte_write_control(2, x);
        xport_gte_write_control(3, y);
        xport_gte_write_control(4, z);
        kind = r_u8(node + 12);
        if (kind == 5)
        {
            if (r_s8(0x8008A2D8) != 0)
            {
                if (r_u32(owner_offset + 92) != 0)
                {
                    p2_sdk_8007ec64(node + 16, matrix + 20);
                    matrix = r_u32(0x8008A0C8);
                    x = r_u32(matrix + 20);
                    px = r_u32(matrix - 12);
                    y = r_u32(matrix + 24);
                    py = r_u32(matrix - 8);
                    z = r_u32(matrix + 28);
                    pz = r_u32(matrix - 4);
                    w_u32(matrix + 20, x + px);
                    w_u32(matrix + 24, y + py);
                    w_u32(matrix + 28, z + pz);
                    x = r_u32(matrix + 20);
                    y = r_u32(matrix + 24);
                    z = r_u32(matrix + 28);
                    w_u32(node + 16, x);
                    w_u32(node + 20, y);
                    w_u32(node + 24, z);
                    x = r_u32(node + 16);
                    px = r_u32(owner_offset + 60);
                    w_u16(velocity_offset, x - px);
                    y = r_u32(node + 20);
                    py = r_u32(owner_offset + 64);
                    w_u16(velocity_offset + 2, y - py);
                    z = r_u32(node + 24);
                    pz = r_u32(owner_offset + 68);
                    w_u16(velocity_offset + 4, z - pz);
                    value = p2_func_80068198();
                    w_u16(velocity_offset, 2730 - value % 5460);
                    value = p2_func_80068198();
                    coefficient = r_s16(velocity_offset + 2);
                    w_u16(velocity_offset + 4, 2730 - value % 5460);
                    if (coefficient < 8192)
                        w_u16(velocity_offset + 2, 8192);
                    x = r_u32(velocity_offset);
                    y = r_u32(velocity_offset + 4);
                    w_u32(node + 40, x);
                    w_u32(node + 44, y);
                    alpha = p2_func_80068198();
                    beta = p2_func_80068198();
                    gamma = p2_func_80068198();
                    w_u16(node + 46, (alpha & 31) | ((beta & 31) << 5) | ((gamma & 31) << 10));
                }
                else
                {
                    qx = r_s16(node + 40);
                    x = r_u32(node + 16);
                    qy = r_s16(node + 42);
                    y = r_u32(node + 20);
                    qz = r_s16(node + 44);
                    z = r_u32(node + 24);
                    w_u32(node + 16, x + (uint32)qx);
                    w_u32(node + 20, y + (uint32)qy);
                    w_u32(node + 24, z + (uint32)qz);
                    w_u16(node + 42, r_u16(node + 42) - 1228);
                    x = r_u32(node + 16);
                    y = r_u32(node + 20);
                    z = r_u32(node + 24);
                    w_u32(matrix + 20, x);
                    w_u32(matrix + 24, y);
                    w_u32(matrix + 28, z);
                }
                value = r_u32(owner_offset + 144);
                packed = (uint32)(sint32)r_s16(node + 46);
                rate = (15 - value) << 4;
                phase = (packed & 31) * rate;
                cosine = p2_func_8005dbb8((sint16)(1024 - phase)) >> 2;
                sine = p2_func_8005dbb8((sint16)phase) >> 2;
                w_u16(0x1F800008, (uint16)cosine);
                w_u16(0x1F80000E, (uint16)sine);
                w_u16(0x1F80000A, (uint16)(0 - (uint32)sine));
                w_u16(0x1F800010, (uint16)cosine);
                MulMatrix0((MATRIX *)psx_addr(0x1F800044, 18), (MATRIX *)psx_addr(0x1F800000, 18), (MATRIX *)psx_addr(first_matrix, 18));
                phase = ((packed >> 5) & 31) * rate;
                cosine = p2_func_8005dbb8((sint16)(1024 - phase)) >> 2;
                sine = p2_func_8005dbb8((sint16)phase) >> 2;
                phase = ((packed >> 10) & 31) * rate;
                w_u16(0x1F800014, (uint16)cosine);
                w_u16(0x1F800020, (uint16)(0 - (uint32)sine));
                w_u16(0x1F800018, (uint16)sine);
                w_u16(0x1F800024, (uint16)cosine);
                if (phase != 0)
                {
                    MulMatrix0((MATRIX *)psx_addr(first_matrix, 18), (MATRIX *)psx_addr(0x1F800014, 18), (MATRIX *)psx_addr(second_matrix, 18));
                    cosine = p2_func_8005dbb8((sint16)(1024 - phase)) >> 2;
                    sine = p2_func_8005dbb8((sint16)phase) >> 2;
                    w_u16(0x1F800028, (uint16)cosine);
                    w_u16(0x1F80002E, (uint16)sine);
                    w_u16(0x1F80002A, (uint16)(0 - (uint32)sine));
                    w_u16(0x1F800030, (uint16)cosine);
                    left = 0x1F800028;
                    right = second_matrix;
                }
                else
                {
                    left = first_matrix;
                    right = 0x1F800014;
                }
                matrix = r_u32(0x8008A0C8);
                MulMatrix0((MATRIX *)psx_addr(left, 18), (MATRIX *)psx_addr(right, 18), (MATRIX *)psx_addr(matrix, 18));
                target = matrix;
            }
            else
            {
                p2_sdk_8007ec64(node + 16, matrix + 20);
                matrix = r_u32(0x8008A0C8);
                x = r_u32(matrix + 20);
                px = r_u32(matrix - 12);
                y = r_u32(matrix + 24);
                py = r_u32(matrix - 8);
                z = r_u32(matrix + 28);
                pz = r_u32(matrix - 4);
                w_u32(matrix + 20, x + px);
                w_u32(matrix + 24, y + py);
                w_u32(matrix + 28, z + pz);
                qw = r_s16(node + 40);
                if (qw == 0x4000)
                {
                    for (index = 0; index < 4; ++index)
                        words[index] = r_u32(matrix - 32 + 4 * index);
                    for (index = 0; index < 4; ++index)
                        w_u32(matrix + 4 * index, words[index]);
                    w_u16(matrix + 16, (uint16)r_s16(matrix - 16));
                    goto node_rotation_ready;
                }
                qx = r_s16(node + 42);
                qy = r_s16(node + 44);
                qz = r_s16(node + 46);
                xx = (uint32)(qx * qx);
                yy = (uint32)(qy * qy);
                zz = (uint32)(qz * qz);
                xy = (uint32)(qx * qy);
                xz = (uint32)(qx * qz);
                yz = (uint32)(qy * qz);
                wz = (uint32)(qw * qz);
                wy = (uint32)(qw * qy);
                w_u16(matrix, (uint32)((sint32)(0x8000000 - (yy + zz)) >> 15));
                w_u16(matrix + 8, (uint32)((sint32)(0x8000000 - (xx + zz)) >> 15));
                w_u16(matrix + 16, (uint32)((sint32)(0x8000000 - (xx + yy)) >> 15));
                wx = (uint32)(qw * qx);
                w_u16(matrix + 6, (uint32)((sint32)(xy - wz) >> 15));
                w_u16(matrix + 2, (uint32)((sint32)(xy + wz) >> 15));
                w_u16(matrix + 12, (uint32)((sint32)(xz + wy) >> 15));
                w_u16(matrix + 4, (uint32)((sint32)(xz - wy) >> 15));
                w_u16(matrix + 14, (uint32)((sint32)(yz - wx) >> 15));
                w_u16(matrix + 10, (uint32)((sint32)(yz + wx) >> 15));
                target = p2_sdk_8007e944(matrix);
            }
            x = r_u32(target);
            y = r_u32(target + 4);
            xport_gte_write_control(0, x);
            xport_gte_write_control(1, y);
            x = r_u32(target + 8);
            y = r_u32(target + 12);
            z = r_u32(target + 16);
            xport_gte_write_control(2, x);
            xport_gte_write_control(3, y);
            xport_gte_write_control(4, z);
        node_rotation_ready:
            if ((r_u32(owner_offset + 28) & 0x200) == 0)
                p2_func_80011980(owner_offset, node + 32);
        }
        else if (kind == 7)
        {
            if (r_s8(0x8008A2D8) != 0)
            {
                x = r_u32(matrix - 12);
                y = r_u32(matrix - 8);
                z = r_u32(matrix - 4);
                w_u32(matrix + 20, x);
                w_u32(matrix + 24, y);
                w_u32(matrix + 28, z);
            }
            else
            {
                p2_sdk_8007ec64(node + 28, matrix + 20);
                matrix = r_u32(0x8008A0C8);
                x = r_u32(matrix + 20);
                px = r_u32(matrix - 12);
                y = r_u32(matrix + 24);
                py = r_u32(matrix - 8);
                z = r_u32(matrix + 28);
                pz = r_u32(matrix - 4);
                w_u32(matrix + 20, x + px);
                w_u32(matrix + 24, y + py);
                w_u32(matrix + 28, z + pz);
            }
            if ((r_u32(owner_offset + 28) & 0x20000) != 0 && (r_u16(node + 14) & 0x200) == 0)
            {
                value = r_u8(node + 13);
                target = r_u32(node + 20);
                w_u8(node + 13, value & 0xFE);
                p2_func_80027594(target);
            }
            if ((r_u8(node + 13) & 1) != 0)
                goto visit_children;
            if ((r_u16(node + 14) & 0x200) == 0)
            {
                matrix = r_u32(0x8008A0C8);
                for (index = 0; index < 4; ++index)
                    words[index] = r_u32(matrix - 32 + 4 * index);
                for (index = 0; index < 4; ++index)
                    w_u32(matrix + 4 * index, words[index]);
                w_u16(matrix + 16, (uint16)r_s16(matrix - 16));
            }
            if ((r_u32(owner_offset + 28) & 0x200) == 0)
            {
                if (((uint32)p2_func_800275bc(r_u32(node + 20)) << 24) != 0)
                {
                    w_u16(0x1F8002A6, r_u16(0x1F8002A6) - 256);
                    p2_func_8001156c(node);
                    w_u16(0x1F8002A6, r_u16(0x1F8002A6) + 256);
                }
                else
                    w_u8(node + 13, r_u8(node + 13) | 1);
            }
        }
        else if (kind == 6)
        {
            if (r_s8(owner_offset + 127) != 0)
                goto next_node;
            p2_sdk_8007ec64(node + 20, matrix + 20);
            matrix = r_u32(0x8008A0C8);
            x = r_u32(matrix + 20);
            px = r_u32(matrix - 12);
            y = r_u32(matrix + 24);
            py = r_u32(matrix - 8);
            z = r_u32(matrix + 28);
            pz = r_u32(matrix - 4);
            w_u32(matrix + 20, x + px);
            w_u32(matrix + 24, y + py);
            w_u32(matrix + 28, z + pz);
            slot = r_u32(0x8008A2D4);
            w_u32(0x8008A2D4, slot + 16);
            x = r_u32(matrix + 20);
            y = r_u32(matrix + 24);
            z = r_u32(matrix + 28);
            w_u32(slot, x);
            w_u32(slot + 4, y);
            w_u32(slot + 8, z);
            radius = r_u32(node + 32) & 0xFFFFFFF8;
            w_u32(slot + 12, radius);
            scale = r_u32(0x1F8002C0);
            if (scale != 0)
            {
                product = (sint64)(sint32)radius * (sint32)scale;
                w_u32(slot + 12, (uint32)((uint64)product >> 12));
            }
            value = r_u8(node + 15);
            radius = r_u32(slot + 12);
            matrix = r_u32(0x8008A0C8);
            radius |= value & 7;
            w_u32(slot + 12, radius);
            for (index = 0; index < 3; ++index)
            {
                value = r_u32(matrix + 20 + 4 * index);
                radius = index == 0 ? radius : r_u32(slot + 12);
                w_u32(minimum_offset + 4 * index, value - radius);
            }
            for (index = 0; index < 3; ++index)
            {
                value = r_u32(matrix + 20 + 4 * index);
                radius = r_u32(slot + 12);
                w_u32(maximum_offset + 4 * index, value + radius);
            }
            if (slot == owner_offset + 160)
            {
                x = r_u32(minimum_offset);
                y = r_u32(minimum_offset + 4);
                z = r_u32(minimum_offset + 8);
                w_u32(owner_offset + 60, x);
                w_u32(owner_offset + 64, y);
                w_u32(owner_offset + 68, z);
                x = r_u32(maximum_offset);
                y = r_u32(maximum_offset + 4);
                z = r_u32(maximum_offset + 8);
                w_u32(owner_offset + 72, x);
                w_u32(owner_offset + 76, y);
                w_u32(owner_offset + 80, z);
            }
            else
            {
                for (index = 0; index < 3; ++index)
                {
                    x = r_u32(minimum_offset + 4 * index);
                    y = r_u32(owner_offset + 60 + 4 * index);
                    if ((sint32)x < (sint32)y)
                        w_u32(owner_offset + 60 + 4 * index, x);
                    x = r_u32(maximum_offset + 4 * index);
                    y = r_u32(owner_offset + 72 + 4 * index);
                    if ((sint32)y < (sint32)x)
                        w_u32(owner_offset + 72 + 4 * index, x);
                }
            }
        }
    visit_children:
        p2_func_80012424(node, owner_offset);
    next_node:
        node = r_u32(node + 4);
    }
    result = r_u32(0x8008A0C8) - 32;
    w_u32(0x8008A0C8, result);
    p2_guest_stack_pointer = saved_stack;
    return result;
}
