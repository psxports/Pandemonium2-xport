#include "game.h"



uint32 p2_func_8001c69c(uint32 object_offset, uint32 sphere, uint32 first_offset, uint32 last_offset)
{
    FUNCTION_MARKER(0x8001C69C, "SLUS_005.78");
    uint32 result, last = 0;
    if (r_s8(object_offset + 126) != 1 || r_u32(0x800AC084) != 0)
    {
        w_u8(last_offset, 1);
        w_u8(first_offset, 1);
        return 1;
    }
    w_u8(first_offset, sphere == 1);
    result = 4;
    if (sphere == 4)
        last = 1;
    else
    {
        last = r_u32(0x8008A544 + sphere * 16);
        result = 0xFFFFFFFF;
        last = last == result;
    }
    w_u8(last_offset, last);
    return result;
}

uint32 p2_func_80038990(uint32 bounds_offset, uint32 object_offset, uint32 delta_offset, uint32 extent_offset, uint32 geometry_offset)
{
    FUNCTION_MARKER(0x80038990, "SLUS_005.78");
    uint32 saved_stack = p2_guest_stack_pointer;
    uint32 start, end, relative, category, hx, hy, hz, radius, value, old, reciprocal;
    uint32 sx, sy, sz, ex, ey, ez, dx, dy, dz, lower_x, upper_x, lower_y, upper_y, lower_z, upper_z;
    uint32 enter_x, exit_x, enter_y, exit_y, enter_z, exit_z, time, leave, side, best, normal, nx, ny, nz, dot, i, result = 0;
    p2_guest_stack_pointer -= 0x50;
    start = p2_guest_stack_pointer + 0x10;
    end = p2_guest_stack_pointer + 0x20;
    relative = p2_guest_stack_pointer + 0x40;
    category = r_u8(object_offset + 12);
    if (category == 2)
    {
        hx = r_u16(object_offset + 60) << 5;
        hy = r_u16(object_offset + 62) << 5;
        hz = r_u16(object_offset + 64) << 5;
        w_u32(relative, r_u32(bounds_offset) - r_u32(object_offset + 36));
        w_u32(relative + 4, r_u32(bounds_offset + 4) - r_u32(object_offset + 40));
        w_u32(relative + 8, r_u32(bounds_offset + 8) - r_u32(object_offset + 44));
        PushMatrix();
        for (i = 0; i < 5; ++i)
            xport_gte_write_control(i, r_u32(object_offset + 16 + i * 4));
        p2_sdk_8007ec64(relative, start);
        w_u32(relative, r_u32(relative) + r_u32(delta_offset));
        w_u32(relative + 4, r_u32(relative + 4) + r_u32(delta_offset + 4));
        w_u32(relative + 8, r_u32(relative + 8) + r_u32(delta_offset + 8));
        p2_sdk_8007ec64(relative, end);
        PopMatrix();
    }
    else
    {
        if (category == 8)
        {
            sx = r_u32(bounds_offset) - r_u32(object_offset + 40);
            w_u32(start, sx);
            sy = r_u32(bounds_offset + 4) - r_u32(object_offset + 44);
            w_u32(start + 4, sy);
            sz = r_u32(bounds_offset + 8) - r_u32(object_offset + 48);
            w_u32(start + 8, sz);
            hz = r_u32(object_offset + 24);
        }
        else if (category == 0)
        {
            sx = r_u32(bounds_offset) - r_u32(geometry_offset);
            w_u32(start, sx);
            sy = r_u32(bounds_offset + 4) - r_u32(geometry_offset + 4);
            w_u32(start + 4, sy);
            sz = r_u32(bounds_offset + 8) - r_u32(geometry_offset + 8);
            w_u32(start + 8, sz);
            hz = r_u32(geometry_offset + 12);
        }
        else
            goto done;
        hy = hz;
        hx = hz;
        w_u32(end, sx + r_u32(delta_offset));
        w_u32(end + 4, sy + r_u32(delta_offset + 4));
        w_u32(end + 8, sz + r_u32(delta_offset + 8));
    }
    radius = r_u32(bounds_offset + 12);
    sx = r_u32(start);
    ex = r_u32(end);
    value = (0 - hx) - (sx - radius);
    lower_x = value - (radius << 1);
    upper_x = value + (hx << 1);
    dx = ex - sx;
    if (!((sint32)lower_x > 0 ? (sint32)dx >= (sint32)lower_x : (sint32)upper_x >= 0))
    {
        if ((sint32)upper_x >= 0 || (sint32)upper_x < (sint32)dx)
            goto done;
    }
    sy = r_u32(start + 4);
    ey = r_u32(end + 4);
    value = (0 - hy) - (sy - radius);
    lower_y = value - (radius << 1);
    upper_y = value + (hy << 1);
    dy = ey - sy;
    if (!((sint32)lower_y > 0 ? (sint32)dy >= (sint32)lower_y : (sint32)upper_y >= 0))
    {
        if ((sint32)upper_y >= 0 || (sint32)upper_y < (sint32)dy)
            goto done;
    }
    sz = r_u32(start + 8);
    ez = r_u32(end + 8);
    value = (0 - hz) - (sz - radius);
    lower_z = value - (radius << 1);
    upper_z = value + (hz << 1);
    dz = ez - sz;
    if (!((sint32)lower_z > 0 ? (sint32)dz >= (sint32)lower_z : (sint32)upper_z >= 0))
    {
        if ((sint32)upper_z >= 0 || (sint32)upper_z < (sint32)dz)
            goto done;
    }
    if (dx + 256 < 0x201)
    {
        enter_x = 0;
        exit_x = 4096;
    }
    else
    {
        old = lower_x;
        if ((sint32)upper_x < 0)
        {
            lower_x = 0 - upper_x;
            upper_x = 0 - old;
        }
        else if ((sint32)lower_x <= 0)
        {
            if ((sint32)dx < 0)
                upper_x = 0 - lower_x;
            lower_x = 0;
        }
        if ((sint32)dx < 0)
            dx = 0 - dx;
        if ((sint32)dx < (sint32)upper_x)
            upper_x = dx;
        if (dx == 0)
            xport_mips_break(7);
        reciprocal = dx == 0 ? 0xFFFFFFFF : (uint32)(0x1000000 / (sint32)dx);
        enter_x = (uint32)(((sint64)(sint32)lower_x * (sint32)reciprocal) >> 12);
        exit_x = (uint32)(((sint64)(sint32)upper_x * (sint32)reciprocal) >> 12);
    }
    time = enter_x;
    leave = exit_x;
    if (dy + 256 < 0x201)
    {
        enter_y = 0;
    }
    else
    {
        old = lower_y;
        if ((sint32)upper_y < 0)
        {
            lower_y = 0 - upper_y;
            upper_y = 0 - old;
        }
        else if ((sint32)lower_y <= 0)
        {
            if ((sint32)dy < 0)
                upper_y = 0 - lower_y;
            lower_y = 0;
        }
        if ((sint32)dy < 0)
            dy = 0 - dy;
        if ((sint32)dy < (sint32)upper_y)
            upper_y = dy;
        if (dy == 0)
            xport_mips_break(7);
        reciprocal = dy == 0 ? 0xFFFFFFFF : (uint32)(0x1000000 / (sint32)dy);
        enter_y = (uint32)(((sint64)(sint32)lower_y * (sint32)reciprocal) >> 12);
        exit_y = (uint32)(((sint64)(sint32)upper_y * (sint32)reciprocal) >> 12);
        if ((sint32)time < (sint32)enter_y)
            time = enter_y;
        if ((sint32)exit_y < (sint32)leave)
            leave = exit_y;
    }
    if (dz + 256 < 0x201)
    {
        enter_z = 0;
    }
    else
    {
        old = lower_z;
        if ((sint32)upper_z < 0)
        {
            lower_z = 0 - upper_z;
            upper_z = 0 - old;
        }
        else if ((sint32)lower_z <= 0)
        {
            if ((sint32)dz < 0)
                upper_z = 0 - lower_z;
            lower_z = 0;
        }
        if ((sint32)dz < 0)
            dz = 0 - dz;
        if ((sint32)dz < (sint32)upper_z)
            upper_z = dz;
        if (dz == 0)
            xport_mips_break(7);
        reciprocal = dz == 0 ? 0xFFFFFFFF : (uint32)(0x1000000 / (sint32)dz);
        enter_z = (uint32)(((sint64)(sint32)lower_z * (sint32)reciprocal) >> 12);
        exit_z = (uint32)(((sint64)(sint32)upper_z * (sint32)reciprocal) >> 12);
        if ((sint32)time < (sint32)enter_z)
            time = enter_z;
        if ((sint32)exit_z < (sint32)leave)
            leave = exit_z;
    }
    if ((sint32)leave < (sint32)time)
        goto done;
    result = 1;
    if (r_u8(object_offset + 12) != 2)
        goto hit;
    value = (uint32)r_s8(object_offset + 15);
    if (value != 9 && value != 6)
        goto hit;
    if (time != 0)
    {
        if (time == enter_y)
            side = (sint32)sy < 0 ? 2 : 3;
        else if (time == enter_z)
            side = (sint32)sz < 0 ? 4 : 5;
        else
            side = (sint32)sx >= 0;
        goto side_hit;
    }
    value = (sint32)sx < 0 ? sx + hx : sx - hx;
    w_u32(end, value);
    if ((sint32)value < 0)
        w_u32(end, 0 - value);
    value = (sint32)sy < 0 ? sy + hy : sy - hy;
    w_u32(end + 4, value);
    if ((sint32)value < 0)
        w_u32(end + 4, 0 - value);
    value = (sint32)sz < 0 ? sz + hz : sz - hz;
    w_u32(end + 8, value);
    if ((sint32)value < 0)
        w_u32(end + 8, 0 - value);
    value = r_u32(end + 4);
    if (r_s32(end) >= (sint32)value && r_s32(end + 8) >= (sint32)value)
        side = (sint32)sy < 0 ? 2 : 3;
    else
    {
        value = r_u32(end + 8);
        if (r_s32(end) >= (sint32)value && r_s32(end + 4) >= (sint32)value)
            side = (sint32)sz < 0 ? 4 : 5;
        else
            side = (sint32)sx >= 0;
    }
    if (r_u32(delta_offset) == 0 && r_u32(delta_offset + 4) == 0 && r_u32(delta_offset + 8) == 0)
        goto side_hit;
    best = enter_z;
    for (i = 0; i < 6; ++i)
    {
        normal = object_offset + 16 + (i >> 1) * 6;
        nx = (uint32)(sint32)r_s16(normal);
        ny = (uint32)(sint32)r_s16(normal + 2);
        nz = (uint32)(sint32)r_s16(normal + 4);
        if ((i & 1) == 0)
        {
            nx = 0 - nx;
            ny = 0 - ny;
            nz = 0 - nz;
        }
        dot = (uint32)(((sint64)r_s32(delta_offset) * (sint32)nx) >> 12);
        dot += (uint32)(((sint64)r_s32(delta_offset + 4) * (sint32)ny) >> 12);
        dot += (uint32)(((sint64)r_s32(delta_offset + 8) * (sint32)nz) >> 12);
        if ((sint32)dot < (sint32)time)
        {
            time = dot;
            best = i;
        }
    }
    if (best != side)
    {
        normal = object_offset + 16 + (uint32)((sint32)best >> 1) * 6;
        nx = (uint32)(sint32)r_s16(normal);
        ny = (uint32)(sint32)r_s16(normal + 2);
        nz = (uint32)(sint32)r_s16(normal + 4);
        if ((best & 1) == 0)
        {
            nx = 0 - nx;
            ny = 0 - ny;
            nz = 0 - nz;
        }
        if (ny + 128 >= 0x101)
        {
            normal = object_offset + 16 + (side >> 1) * 6;
            nx = (uint32)(sint32)r_s16(normal);
            ny = (uint32)(sint32)r_s16(normal + 2);
            nz = (uint32)(sint32)r_s16(normal + 4);
            if ((side & 1) == 0)
            {
                nx = 0 - nx;
                ny = 0 - ny;
                nz = 0 - nz;
            }
            if (ny + 128 < 0x101)
                best = side;
        }
    }
    side = best;
    time = 0;
side_hit:
    result = side + 1;
hit:
    w_u32(extent_offset, time);
done:
    p2_guest_stack_pointer = saved_stack;
    return result;
}

uint32 p2_func_80047058(uint32 object_offset, uint32 other_offset, uint32 context_offset)
{
    FUNCTION_MARKER(0x80047058, "SLUS_005.78");
    uint32 flags, mode, node, target, other_target;
    if (r_s8(object_offset + 124) == 2)
    {
        mode = r_u32(object_offset + 116);
        if ((mode == 19 || (mode == 0 && r_u32(0x8008A498) != 0)) && other_offset != 0)
        {
            if ((r_u32(object_offset + 28) & 0x10) != 0)
                node = r_u32(other_offset + 68);
            else
                node = r_u32(other_offset + 72);
            if ((r_u32(r_u32(node + 36) + 16) & 0x90000) != 0)
                return 0;
        }
    }
    flags = r_u32(object_offset + 28);
    if ((flags & 0xC00) == 0 && (r_u32(object_offset + 228) & 0xA0) == 0)
        return 0;
    if (context_offset == 0)
        return 0;
    if (other_offset == 0)
        return 1;
    if ((flags & 0x10) != 0)
        node = r_u32(other_offset + 68);
    else
        node = r_u32(other_offset + 72);
    mode = r_u32(object_offset + 228);
    if ((mode & 0x20) != 0)
    {
        other_target = r_u32(0x800ABE60);
        target = r_u32(node + 36);
        if (target != r_u32(other_target + 36) || (mode & 0x8000) != 0)
            return 1;
        return (r_u32(object_offset + 28) & 0x2000) != 0;
    }
    target = r_u32(node + 36);
    other_target = r_u32(context_offset + 36);
    if (target != other_target)
    {
        if (r_s8(object_offset + 126) != 1)
            return 1;
        if (target != r_u32(r_u32(0x800AC018) + 36))
            return 1;
    }
    return (r_u32(object_offset + 28) & 0x2000) != 0;
}

uint32 p2_func_8005bff4(void)
{
    FUNCTION_MARKER(0x8005BFF4, "SLUS_005.78");
    uint32 saved_stack = p2_guest_stack_pointer;
    uint32 record, limit, scene, source, count, value, node, object, score, head = 0, cursor, previous;
    uint32 x, y, z, flags, result, slots, budget, cost, rank;
    p2_guest_stack_pointer -= 0x280;
    record = p2_guest_stack_pointer + 0x10;
    limit = p2_guest_stack_pointer + 0x268;
    scene = r_u32(0x8008A114);
    if (r_s8(scene + 75) != 0)
    {
        source = scene + 76;
        do
        {
            scene = r_u32(0x8008A114);
            count = (uint32)(sint32)r_s8(scene + 1);
            value = r_u32(source);
            source += 4;
            if ((sint32)count < 40)
            {
                w_u8(scene + 1, count + 1);
                w_u32(scene + count * 4 + 140, value | 0x20000);
            }
            scene = r_u32(0x8008A114);
            count = r_u8(scene + 75) - 1;
            w_u8(scene + 75, count);
        } while ((count & 0xFF) != 0);
    }
    result = r_u32(0x8008A4E8);
    for (node = r_u32(result + 68); node != 0; node = r_u32(node))
    {
        object = r_u32(node + 60);
        if ((r_u8(object + 13) & 0x80) != 0)
        {
            result = r_u16(node + 48);
            if (result == 0)
                continue;
            value = r_u16(node + 40) - 1;
            w_u16(node + 40, value);
            result = value & 0xFFFF;
            if (result != 0)
                continue;
            value = r_u16(node + 26);
            object = r_u32(node + 60);
            w_u16(node + 24, value);
            w_u32(object + 16, r_u32(object + 16) & 0xFFFFFFFB);
            object = r_u32(node + 60);
            w_u8(object + 13, r_u8(object + 13) & 0x7F);
        }
        object = r_u32(node + 60);
        if ((r_u8(object + 13) & 1) != 0)
        {
            result = p2_func_8005bcfc(object);
            continue;
        }
        value = r_u32(node + 12);
        if (value == 0x7FFFF000)
            value = p2_func_800343f0(node);
        w_u32(record + 8, value);
        if ((character_action_flags & 0x200) != 0)
        {
            object = r_u32(node + 4);
            if (object != 0 && (r_s8(node + 37) != 12 || r_s8(node + 124) != 0) && (character_action_target_offset == 0 || r_s32(record + 8) < r_s32(0x8008A41C)))
            {
                x = r_u32(0x800B1510) - r_u32(object + 16);
                w_u32(limit, x);
                object = r_u32(node + 4);
                y = r_u32(0x800B1514) - r_u32(object + 20);
                w_u32(limit + 4, y);
                object = r_u32(node + 4);
                z = r_u32(0x800B1518) - r_u32(object + 24);
                w_u32(limit + 8, z);
                value = Atan2Angle((sint32)z, (sint32)x);
                value = (uint32)(sint32)(sint16)value - (uint32)(sint32)r_s16(character_action_owner_offset + 148);
                if ((sint32)value < 0)
                    value = 0 - value;
                if ((sint16)value < 512)
                {
                    object = r_u32(node + 4);
                    value = r_u32(record + 8);
                    character_action_target_offset = object;
                    w_u32(0x8008A41C, value);
                }
            }
        }
        cursor = head;
        w_u32(record + 4, node);
        if (cursor == 0)
        {
            head = record;
            w_u32(record, 0);
        }
        else
        {
            score = r_u32(record + 8);
            if (r_s32(cursor + 8) < (sint32)score)
            {
                do
                {
                    previous = cursor;
                    cursor = r_u32(cursor);
                } while (cursor != 0 && r_s32(cursor + 8) < (sint32)score);
            }
            if (cursor == head)
                head = record;
            else
                w_u32(previous, record);
            w_u32(record, cursor);
        }
        record += 12;
        result = limit;
        if (record == limit)
            break;
    }
    rank = 0xFFFFFFFF;
    slots = 12;
    budget = 102400;
    for (cursor = head; cursor != 0; cursor = r_u32(cursor))
    {
        if (slots != 0)
        {
            node = r_u32(cursor + 4);
            object = r_u32(node + 72);
            if (object != 0)
            {
                cost = r_u16(object + 70);
                if ((sint32)budget >= (sint32)cost)
                {
                    budget -= cost;
                    --rank;
                    w_u32(node + 12, rank);
                    --slots;
                }
            }
        }
        node = r_u32(cursor + 4);
        p2_func_8005bcfc(r_u32(node + 60));
    }
    if (head != 0)
    {
        flags = character_action_flags;
        result = flags ^ 0x200;
        if ((flags & 0x200) != 0)
        {
            result |= 0x400;
            character_action_flags = result;
        }
    }
    p2_guest_stack_pointer = saved_stack;
    return result;
}

uint32 p2_func_80016d0c(void)
{
    FUNCTION_MARKER(0x80016D0C, "SLUS_005.78");
    ObjectCallback object_operation;
    uint32 object = p2_func_800536f0(0);
    uint32 value, scene, matrix, x, y, z, i;
    sint32 kind;
    if (object != 0)
    {
        kind = r_s8(object + 127);
        if (kind != 0)
            object_operation = object_operations.variants[kind];
        else
            object_operation = object_operations.behaviors[r_s8(object + 125)];
        {
            const ObjectCallback operation = object_operation;
            if (operation.value_handler)
                operation.value_handler(object);
            else if (operation.void_handler)
                operation.void_handler(object);
            else
                operation.plain_handler();
        }
        value = game_flow.transition_flags;
        if ((value & 0x48) != 0)
        {
            if ((value & 8) != 0)
                return 0;
            scene = r_u32(0x8008A4E8);
            if ((r_u16(scene + 18) & 8) == 0)
                return 0;
            if (r_u16(0x8008A532) < 0x1000)
            {
                render_view.planes[0].normal.vz = (sint16)(0);
                render_view.planes[0].normal.vy = (sint16)(0);
                render_view.planes[0].normal.vx = (sint16)(0);
                render_view.planes[0].bias = 0x80001000;
                for (i = 0; i < 5; ++i)
                    xport_gte_write_control(i, r_u32(0x80010180 + i * 4));
                matrix = r_u32(0x8008A0C8);
                x = r_u32(0x80010180);
                y = r_u32(0x80010184);
                z = r_u32(0x80010188);
                w_u32(matrix, x);
                w_u32(matrix + 4, y);
                w_u32(matrix + 8, z);
                value = r_u32(0x8001018C);
                x = (uint32)r_s16(0x80010190);
                w_u32(matrix + 12, value);
                w_u16(matrix + 16, x);
                matrix = r_u32(0x8008A0C8);
                w_u32(matrix + 28, 0);
                w_u32(matrix + 24, 0);
                w_u32(matrix + 20, 0);
                p2_func_800140f8(scene, 0, 0);
            }
            return 0;
        }
    }
    if ((r_u32(r_u32(0x8008A114) + 72) & 0xFFFFFF) != 0)
        p2_func_80024890();
    if (r_u32(0x8008A4F4) != 0)
        p2_func_80024c44();
    return 1;
}

uint32 p2_func_800536f0(uint32 index)
{
    FUNCTION_MARKER(0x800536F0, "SLUS_005.78");
    if (index == 0)
        return r_u32(0x8008A340);
    return 0;
}

uint32 p2_func_800530a0(uint32 object_offset)
{
    FUNCTION_MARKER(0x800530A0, "SLUS_005.78");
    uint32 state = 0x800ABD68;
    uint32 flags, value, source, first, second, third, fourth, animation, kind, result;
    sint32 frame, next = 0;
    if (r_s8(object_offset + 127) == 1)
        return 1;
    if ((character_controller.flags & 0x4000000) != 0)
        return p2_func_80045ed8(object_offset);
    p2_func_8004eb74(object_offset, state);
    if ((r_u32(object_offset + 28) & 0x2000000) != 0)
    {
        value = character_controller.vitals.damage_ticks - 1;
        character_controller.vitals.damage_ticks = value;
        if ((sint16)value <= 0)
        {
            w_u32(object_offset + 28, r_u32(object_offset + 28) & 0xEDF7FFFF);
            character_controller.vitals.damage_ticks = 0;
        }
        else
            w_u32(object_offset + 28, r_u32(object_offset + 28) ^ 0x80000);
    }
    ProcessCharacterPickups(object_offset, state);
    ProcessCharacterTriggers(object_offset, state);
    p2_func_800521bc(object_offset, state);
    p2_func_800524ac(object_offset, state);
    if ((p2_func_800526f8(object_offset, state) & 0xFF) == 0)
        goto finish;
    result = p2_func_80050c94(object_offset, state) << 24;
    if (result != 0)
        return result;
    if ((character_controller.flags & 0x20) != 0)
    {
        source = r_u32(state + 116);
        fourth = r_u32(source + 80);
        first = r_u32(source + 68);
        second = r_u32(source + 72);
        third = r_u32(source + 76);
        p2_func_80046628(object_offset, first, second, third, fourth);
        character_controller.flags = character_controller.flags & 0xFFFFFFDD;
        goto finish;
    }
    p2_func_800515f0(object_offset, state);
    if ((sint8)character_controller.motion.state_code == 23)
        goto finish;
    source = r_u32(object_offset + 104);
    if (source != 0)
    {
        value = (uint32)(sint32)r_s16(source + 6);
        frame = r_s16(object_offset + 130);
        if (frame < (sint32)value - 1)
            next = frame + 1;
    }
    frame = r_s16(object_offset + 130);
    if (next < frame)
    {
        if ((r_u32(object_offset + 228) & 0x40000) != 0)
            goto reverse;
        animation = (uint32)(sint32)(sint8)character_controller.motion.state_code;
        if (animation - 1 >= 34 && ((uint16)(character_controller.motion.form_flags >> 16) & 0xF) < 2)
        {
            w_u16(object_offset + 130, (uint32)next);
            value = 0;
            if ((sint8)character_controller.motion.prev_state_code == 0 && (sint16)character_controller.timers.idle_ticks <= 0)
            {
                value = 37;
                if ((sint16)character_controller.timers.idle_ticks < -4499)
                    character_controller.timers.idle_ticks = (uint16)-4500;
                else
                {
                    flags = random_generator.Next() & 0x7F;
                    value = flags < 50 ? 0 : (flags < 101 ? 35 : 36);
                }
            }
            goto set_animation;
        }
        animation = character_controller.motion.state_code;
        if (animation - 4 < 2)
        {
            if ((r_u32(object_offset + 28) & 0x400) != 0)
                value = 6;
            else
                value = (sint8)character_controller.vitals.health <= 0 ? 1 : 0;
            goto set_animation;
        }
        if ((sint8)animation == 3)
        {
            value = 6;
            goto set_animation;
        }
        kind = (uint16)(character_controller.motion.form_flags >> 16) & 0xF;
        if ((kind == 0 || kind == 8) && (sint8)animation == 9)
        {
            value = 6;
            goto set_animation;
        }
        animation = (uint32)(sint32)(sint8)character_controller.motion.state_code;
        if (animation == 7)
        {
            if (((uint16)(character_controller.motion.form_flags >> 16) & 0xF) != 5)
                goto last_frame;
            animation = (uint32)(sint32)(sint8)character_controller.motion.state_code;
        }
        if (animation == 17 || animation == 20)
        {
        last_frame:
            source = r_u32(object_offset + 104);
            w_u16(object_offset + 130, r_u16(source + 6) - 1);
            goto finish;
        }
    }
    else
    {
        if ((r_u32(object_offset + 228) & 0x40000) != 0)
            goto reverse;
        if ((sint8)character_controller.motion.state_code == 20 && r_s8(state + 252) == 1)
        {
            w_u16(object_offset + 130, 0);
            goto finish;
        }
    }
    w_u16(object_offset + 130, (uint32)next);
    goto finish;
reverse:
    frame = r_s16(object_offset + 130);
    if (frame > 0)
        w_u16(object_offset + 130, (uint32)frame - 1);
    goto finish;
set_animation:
    if ((character_controller.secondary_flags & 0x80) == 0)
        character_controller.motion.state_code = value;
finish:
    p2_func_80052ba4(object_offset, state);
    p2_func_8004fd44(object_offset, state);
    return p2_func_8001a6ac(object_offset);
}





uint32 p2_func_8004eb74(uint32 object_offset, uint32 character_offset)
{
    FUNCTION_MARKER(0x8004EB74, "SLUS_005.78");
    uint32 saved_stack = p2_guest_stack_pointer;
    uint32 velocity, parent_velocity, input, transpose, temporary;
    uint32 x, y, z, flags, fraction, parent, matrix, scene, current, support, definition, direction, kind, moved, was_grounded, was_detached, special;
    uint32 first, second, third;
    sint32 frame;
    p2_guest_stack_pointer -= 0x70;
    velocity = p2_guest_stack_pointer + 0x10;
    parent_velocity = p2_guest_stack_pointer + 0x20;
    input = p2_guest_stack_pointer + 0x30;
    transpose = p2_guest_stack_pointer + 0x40;
    temporary = p2_guest_stack_pointer + 0x58;
    if ((r_u32(object_offset + 228) & 0x100) != 0)
        p2_func_80057650(object_offset);
    x = r_u32(object_offset + 92);
    y = r_u32(object_offset + 96);
    z = r_u32(object_offset + 100);
    w_u32(velocity, x);
    w_u32(velocity + 4, y);
    w_u32(velocity + 8, z);
    if (r_s32(object_offset + 144) < 4096)
    {
        character_controller.flags = character_controller.flags & 0xFFFFEFFF;
        w_u32(parent_velocity + 8, 0);
        w_u32(parent_velocity + 4, 0);
        w_u32(parent_velocity, 0);
        flags = r_u32(object_offset + 28);
        w_u32(object_offset + 28, flags & 0xFFFFF7FF);
        if ((flags & 0x4000) != 0)
        {
            if ((r_u32(object_offset + 228) & 2) != 0 && (flags & 0x400) != 0)
            {
                scene = r_u32(0x8008A114);
                current = r_u32(scene + 136);
                definition = r_u32(current + 36);
                if ((r_u32(definition + 16) & 0x10000) == 0)
                    p2_func_80058c8c(object_offset);
            }
            scene = r_u32(0x8008A114);
            current = r_u32(scene + 136);
            definition = r_u32(current + 36);
            if ((r_u32(definition + 16) & 0x80000) == 0)
            {
                if ((r_u32(object_offset + 228) & 0x20) != 0)
                    p2_func_800586e8(object_offset);
                if ((r_u32(object_offset + 228) & 0x10) != 0)
                    p2_func_80058618(object_offset);
            }
            if ((r_u32(object_offset + 228) & 0x200) != 0)
                p2_func_80056774(object_offset);
            if ((r_u32(object_offset + 228) & 0x1000) != 0)
                p2_func_80059268(object_offset);
            w_u32(character_offset + 44, 0);
            w_u32(character_offset + 40, 0);
            w_u32(character_offset + 36, 0);
            w_u32(object_offset + 28, r_u32(object_offset + 28) & 0xFFFFFBFF);
        }
        else
        {
            parent = r_u32(object_offset + 132);
            if (parent != 0)
            {
                matrix = r_u32(parent + 40);
                if (matrix != 0 && (flags & 0x2000) == 0)
                {
                    x = r_u32(matrix + 32);
                    y = r_u32(matrix + 36);
                    z = r_u32(matrix + 40);
                    w_u32(parent_velocity, x);
                    w_u32(parent_velocity + 4, y);
                    w_u32(parent_velocity + 8, z);
                }
            }
        }
        x = r_u32(velocity) + r_u32(parent_velocity);
        y = r_u32(velocity + 4) + r_u32(parent_velocity + 4);
        z = r_u32(velocity + 8) + r_u32(parent_velocity + 8);
        w_u32(velocity, x);
        w_u32(velocity + 4, y);
        w_u32(velocity + 8, z);
        fraction = r_u32(object_offset + 144);
        if (fraction != 0 || (r_u32(object_offset + 228) & 6) != 2)
        {
            w_u32(velocity, (uint32)(((sint64)(sint32)x * (sint32)fraction) >> 12));
            w_u32(velocity + 4, (uint32)(((sint64)(sint32)y * (sint32)fraction) >> 12));
            w_u32(velocity + 8, (uint32)(((sint64)(sint32)z * (sint32)fraction) >> 12));
        }
        x = r_u32(velocity) - r_u32(parent_velocity);
        y = r_u32(velocity + 4) - r_u32(parent_velocity + 4);
        z = r_u32(velocity + 8) - r_u32(parent_velocity + 8);
        w_u32(velocity, x);
        w_u32(velocity + 4, y);
        w_u32(velocity + 8, z);
        fraction = 4096 - r_u32(object_offset + 144);
        if ((character_controller.flags & 0x18040) != 0)
        {
            flags = character_controller.flags;
            w_u32(character_offset + 44, 0);
            w_u32(character_offset + 36, 0);
            if ((flags & 0x18000) != 0)
                w_u32(character_offset + 40, 0);
        }
        x = (uint32)(((sint64)r_s32(character_offset + 36) * (sint32)fraction) >> 12);
        y = r_u32(character_offset + 40);
        w_u32(character_offset + 36, x);
        y = (uint32)(((sint64)(sint32)y * (sint32)fraction) >> 12);
        z = r_u32(character_offset + 44);
        w_u32(character_offset + 40, y);
        w_u32(character_offset + 44, (uint32)(((sint64)(sint32)z * (sint32)fraction) >> 12));
        w_u32(velocity, r_u32(velocity) + r_u32(character_offset + 36));
        w_u32(velocity + 4, r_u32(velocity + 4) + r_u32(character_offset + 40));
        w_u32(velocity + 8, r_u32(velocity + 8) + r_u32(character_offset + 44));
    }
    moved = r_u32(velocity) != 0 || r_u32(velocity + 8) != 0;
    if (moved != 0)
        w_u32(character_offset + 8, r_u32(character_offset + 8) | 0x2000000);
    else if (r_s32(velocity + 4) <= 0)
        goto rotate_velocity;
    if ((frame_clock.render_tick & 1) != 0 && r_s8(character_offset + 714) >= 0 && (r_u32(object_offset + 228) & 0x400011) == 0)
    {
        scene = r_u32(0x8008A114);
        current = r_u32(scene + 136);
        definition = r_u32(current + 36);
        if ((r_u32(definition + 16) & 0x80000) == 0)
        {
            if ((r_u32(object_offset + 28) & 0x2000) != 0)
                goto effect_position;
            parent = r_u32(object_offset + 132);
            if (parent == 0 || r_u32(parent + 40) == 0)
                goto effect_position;
            x = r_u32(object_offset + 16);
            y = r_u32(object_offset + 20);
            z = r_u32(object_offset + 24);
            w_u32(input, x);
            w_u32(input + 4, y);
            w_u32(input + 8, z);
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
            p2_sdk_8007ec64(input, parent_velocity);
            PopMatrix();
            matrix = r_u32(r_u32(object_offset + 132) + 40);
            x = r_u32(parent_velocity);
            w_u32(parent_velocity, x + r_u32(matrix + 20));
            matrix = r_u32(r_u32(object_offset + 132) + 40);
            y = r_u32(parent_velocity + 4);
            w_u32(parent_velocity + 4, y + r_u32(matrix + 24));
            matrix = r_u32(r_u32(object_offset + 132) + 40);
            z = r_u32(parent_velocity + 8);
            w_u32(parent_velocity + 8, z + r_u32(matrix + 28));
            goto emit_effect;
        effect_position:
            x = r_u32(object_offset + 16);
            y = r_u32(object_offset + 20);
            z = r_u32(object_offset + 24);
            w_u32(parent_velocity, x);
            w_u32(parent_velocity + 4, y);
            w_u32(parent_velocity + 8, z);
        emit_effect:
            kind = (uint16)(character_controller.motion.form_flags >> 16) & 15;
            if (kind != 5 && kind - 10 >= 2)
                p2_func_8002a80c(parent_velocity, 4096, r_s8(character_offset + 714) + 7);
        }
    }
rotate_velocity:
    if ((r_u32(object_offset + 28) & 0x2000) != 0)
    {
        if (r_u32(object_offset + 144) == 4096)
            w_u32(object_offset + 144, 4095);
    }
    else
    {
        parent = r_u32(object_offset + 132);
        if (parent != 0)
        {
            matrix = r_u32(parent + 40);
            if (matrix != 0)
            {
                w_u16(transpose, r_u16(matrix));
                w_u16(transpose + 2, r_u16(r_u32(r_u32(object_offset + 132) + 40) + 6));
                w_u16(transpose + 4, r_u16(r_u32(r_u32(object_offset + 132) + 40) + 12));
                w_u16(transpose + 6, r_u16(r_u32(r_u32(object_offset + 132) + 40) + 2));
                w_u16(transpose + 8, r_u16(r_u32(r_u32(object_offset + 132) + 40) + 8));
                w_u16(transpose + 10, r_u16(r_u32(r_u32(object_offset + 132) + 40) + 14));
                w_u16(transpose + 12, r_u16(r_u32(r_u32(object_offset + 132) + 40) + 4));
                w_u16(transpose + 14, r_u16(r_u32(r_u32(object_offset + 132) + 40) + 10));
                w_u16(transpose + 16, r_u16(r_u32(r_u32(object_offset + 132) + 40) + 16));
                x = r_u32(velocity);
                y = r_u32(velocity + 4);
                z = r_u32(velocity + 8);
                w_u32(input, x);
                w_u32(input + 4, y);
                w_u32(input + 8, z);
                PushMatrix();
                x = r_u32(transpose);
                y = r_u32(transpose + 4);
                xport_gte_write_control(0, x);
                xport_gte_write_control(1, y);
                x = r_u32(transpose + 8);
                y = r_u32(transpose + 12);
                z = r_u16(transpose + 16);
                xport_gte_write_control(2, x);
                xport_gte_write_control(3, y);
                xport_gte_write_control(4, z);
                p2_sdk_8007ec64(input, velocity);
                PopMatrix();
            }
        }
    }
    w_u32(object_offset + 16, r_u32(object_offset + 16) + r_u32(velocity));
    w_u32(object_offset + 20, r_u32(object_offset + 20) + r_u32(velocity + 4));
    w_u32(object_offset + 24, r_u32(object_offset + 24) + r_u32(velocity + 8));
    if ((r_u32(object_offset + 28) & 0x4000) != 0 || (r_u32(object_offset + 228) & 0x100) != 0)
        goto final_position;
    flags = character_controller.flags;
    if ((flags & 0x22) != 0)
        goto final_position;
    fraction = r_u32(object_offset + 144);
    if ((sint32)fraction >= 4096 && ((flags & 1) == 0 || (sint32)fraction < r_s32(character_offset + 672)))
        goto final_position;
    scene = r_u32(0x8008A114);
    w_u16(scene + 2, r_u16(object_offset + 136));
    flags = r_u32(object_offset + 28);
    x = character_controller.flags;
    was_grounded = (flags >> 10) & 1;
    was_detached = (flags >> 13) & 1;
    if ((x & 1) == 0 || r_s32(object_offset + 144) < r_s32(character_offset + 672))
        w_u32(character_offset + 188, 0);
    moved = 1;
    support = p2_func_8004e118(object_offset, r_u32(character_offset + 188));
    if (support == 0)
    {
        if ((r_u32(object_offset + 28) & 0x2000) != 0 || r_u32(character_offset + 188) != 0)
            goto grounded_transition;
        parent = r_u32(object_offset + 132);
        if (parent != 0)
        {
            matrix = r_u32(parent + 40);
            if (matrix != 0)
            {
                x = r_u32(object_offset + 92);
                y = r_u32(matrix + 32);
                parent = r_u32(object_offset + 132);
                w_u32(object_offset + 92, x + y);
                matrix = r_u32(parent + 40);
                x = r_u32(object_offset + 96);
                y = r_u32(matrix + 36);
                parent = r_u32(object_offset + 132);
                w_u32(object_offset + 96, x + y);
                matrix = r_u32(parent + 40);
                x = r_u32(object_offset + 100);
                w_u32(object_offset + 100, x + r_u32(matrix + 40));
            }
        }
        p2_func_8004df14(object_offset, character_offset);
        goto grounded_transition;
    }
    flags = r_u32(object_offset + 28);
    if ((flags & 0x4000) != 0)
    {
        if (r_u32(character_offset + 188) != 0)
            character_controller.flags = character_controller.flags & 0xFCC5F459;
        else
        {
            w_u32(object_offset + 28, flags & 0xFFFFBBFF);
            character_controller.flags = character_controller.flags & 0xF7FFFFFF;
        }
    }
    special = 0;
    if ((r_u32(object_offset + 228) & 0x20) != 0 && support == r_u32(character_offset + 248))
        special = 1;
    scene = r_u32(0x8008A114);
    if (support == r_u32(scene + 136) || special != 0)
        goto grounded_transition;
    parent = r_u32(object_offset + 132);
    if (parent != 0 && r_u32(parent + 40) != 0)
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
        scene = r_u32(0x8008A114);
        p2_sdk_8007ec64(temporary, scene + 16);
        PopMatrix();
        scene = r_u32(0x8008A114);
        matrix = r_u32(r_u32(object_offset + 132) + 40);
        x = r_u32(scene + 16);
        w_u32(scene + 16, x + r_u32(matrix + 20));
        matrix = r_u32(r_u32(object_offset + 132) + 40);
        y = r_u32(scene + 20);
        w_u32(scene + 20, y + r_u32(matrix + 24));
        matrix = r_u32(r_u32(object_offset + 132) + 40);
        z = r_u32(scene + 24);
        w_u32(scene + 24, z + r_u32(matrix + 28));
    }
    else
    {
        scene = r_u32(0x8008A114);
        x = r_u32(object_offset + 16);
        y = r_u32(object_offset + 20);
        z = r_u32(object_offset + 24);
        w_u32(scene + 16, x);
        w_u32(scene + 20, y);
        w_u32(scene + 24, z);
    }
    frame = r_s16(object_offset + 136);
    if (was_detached != 0)
        w_u32(object_offset + 28, r_u32(object_offset + 28) | 0x2000);
    else
    {
        scene = r_u32(0x8008A114);
        current = r_u32(scene + 136);
        flags = r_u32(object_offset + 28);
        definition = r_u32(current + 36);
        w_u16(object_offset + 136, FindPathHeightFrame(definition, scene + 16, flags & 0x10, -40960));
    }
    scene = r_u32(0x8008A114);
    if (support != r_u32(scene + 136))
    {
        direction = r_u32(object_offset + 28) & 0x10;
        ProjectPathOffset(support, frame, scene + 28, direction);
        scene = r_u32(0x8008A114);
        current = r_u32(scene + 136);
        definition = r_u32(support + 36);
        if (definition == r_u32(current + 36))
            direction ^= 0x10;
        else
            ApplyPathVelocityChange(object_offset, definition);
        scene = r_u32(0x8008A114);
        if ((r_u32(scene + 72) & 0xFFFFFF) == 0)
        {
            current = r_u32(scene + 136);
            if (current == r_u32(0x800AC018) && (r_u32(object_offset + 28) & 0x2000) == 0)
            {
                kind = (uint32)(sint32)r_s16(object_offset + 136);
                if ((sint32)kind >= 0)
                    ProjectPathOffset(current, (sint32)kind, scene + 4, direction);
            }
        }
        SelectScenePath(support);
    }
    flags = r_u32(object_offset + 28);
    w_u16(object_offset + 136, frame);
    w_u32(object_offset + 28, flags & 0xFFFFDFFF);
    x = character_controller.motion.vertical_speed;
    if (character_controller.movement.fall_speed_limit >= (sint32)x)
        character_controller.timers.state_ticks = 0;
    if (r_u32(character_offset + 188) != 0 && (character_controller.flags & 0x8040) != 0)
        character_controller.ClearTransientFlags();
grounded_transition:
    if ((r_u32(object_offset + 28) & 0x400) != 0 && was_grounded == 0)
    {
        x = character_controller.movement.jump_ticks;
        flags = character_controller.flags;
        kind = (uint16)(character_controller.motion.form_flags >> 16);
        character_controller.motion.jump_phase = -1;
        character_controller.motion.vertical_speed = 0;
        character_controller.timers.state_ticks = 0;
        character_controller.timers.jump_duration_ticks = x;
        character_controller.flags = flags | 0x8000000;
        if (((kind & 15) != 1 || (r_u32(object_offset + 28) & 0x4000000) == 0) && (character_controller.secondary_flags & 0x80) == 0)
            character_controller.motion.state_code = 6;
    }
final_position:
    scene = r_u32(0x8008A114);
    current = r_u32(character_offset + 688);
    if (r_u32(scene + 136) != current)
    {
        w_u32(temporary, r_u32(scene + 28) - r_u32(scene + 4));
        w_u32(temporary + 4, r_u32(scene + 32) - r_u32(scene + 8));
        w_u32(temporary + 8, r_u32(scene + 36) - r_u32(scene + 12));
        p2_func_80024b3c(temporary);
        scene = r_u32(0x8008A114);
        current = r_u32(scene + 136);
        parent = r_u32(character_offset + 688);
        definition = r_u32(current + 36);
        if (definition != r_u32(parent + 36) && (r_u32(scene + 72) & 0xFFFFFF) != 0)
        {
            first = r_u8(scene + 72);
            second = r_u8(scene + 73);
            third = r_u8(scene + 74);
            x = r_u32(object_offset + 92) * 2;
            y = r_u32(object_offset + 96) * 2;
            z = r_u32(object_offset + 100) * 2;
            w_u32(temporary, r_u32(temporary) - x);
            w_u32(temporary + 4, r_u32(temporary + 4) - y);
            w_u32(temporary + 8, r_u32(temporary + 8) - z);
            p2_func_80024b3c(temporary);
            scene = r_u32(0x8008A114);
            if (first < r_u8(scene + 72))
            {
                w_u8(scene + 72, first);
                scene = r_u32(0x8008A114);
            }
            if (second < r_u8(scene + 73))
                w_u8(scene + 73, second);
            scene = r_u32(0x8008A114);
            if (third < r_u8(scene + 74))
            {
                w_u8(scene + 74, third);
                scene = r_u32(0x8008A114);
            }
        }
        else
            scene = r_u32(0x8008A114);
        moved = 1;
        w_u8(scene, r_u8(scene) & 0xFE);
    }
    scene = r_u32(0x8008A114);
    w_u16(scene + 2, r_u16(object_offset + 136));
    w_u16(character_offset + 682, moved != 0 ? 1 : 0);
    flags = character_controller.flags;
    x = character_controller.secondary_flags;
    character_controller.flags = flags & 0xFFFFFFFE;
    character_controller.secondary_flags = x & 0xFFFFFFF6;
    p2_guest_stack_pointer = saved_stack;
    return 0xFFFFFFF6;
}
