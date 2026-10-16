#include "game.h"

void p2_func_8002b278(uint32 definition_offset, uint32 spawned_offset, uint32 object_offset, uint32 phase)
{
    uint32 saved_stack, vector, world, inverse, input, matrix_local;
    uint32 flags, value, x, y, z, a, b, c, parent, matrix, owner, factor, index, kind, result, reverse;
    uint32 scaled_x, scaled_z;
    sint32 state;
    if (spawned_offset == 0)
        return;
    saved_stack = p2_guest_stack_pointer;
    p2_guest_stack_pointer -= 96;
    vector = p2_guest_stack_pointer;
    world = vector + 16;
    inverse = vector + 32;
    input = vector + 48;
    matrix_local = vector + 64;
    w_u32(spawned_offset + 188, 0xFFFFFFFF);
    w_u32(spawned_offset + 172, 0xFFFFFFFF);
    w_u8(spawned_offset + 125, 6);
    w_u32(spawned_offset + 108, 0);
    w_u32(spawned_offset + 116, r_u32(object_offset + 116));
    w_u32(spawned_offset + 132, r_u32(object_offset + 132));
    w_u16(spawned_offset + 136, r_u16(object_offset + 136));
    x = r_u32(object_offset + 16);
    y = r_u32(object_offset + 20);
    z = r_u32(object_offset + 24);
    w_u32(spawned_offset + 16, x);
    w_u32(spawned_offset + 20, y);
    w_u32(spawned_offset + 24, z);
    flags = r_u32(spawned_offset + 28);
    value = r_u32(object_offset + 28);
    w_u32(spawned_offset + 28, flags | 0x20440 | phase | (value & 0x2010));
    if (r_s8(object_offset + 125) == 6)
    {
        flags = r_u32(object_offset + 28);
        if ((flags & 0x40000000) != 0)
        {
            if ((flags & 0x2000) == 0)
            {
                parent = r_u32(spawned_offset + 132);
                if (parent != 0 && r_u32(parent + 40) != 0)
                {
                    x = r_u32(spawned_offset + 16);
                    y = r_u32(spawned_offset + 20);
                    z = r_u32(spawned_offset + 24);
                    w_u32(vector, x);
                    w_u32(vector + 4, y);
                    w_u32(vector + 8, z);
                    PushMatrix();
                    matrix = r_u32(r_u32(spawned_offset + 132) + 40);
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
                    p2_sdk_8007ec64(vector, spawned_offset + 16);
                    PopMatrix();
                    matrix = r_u32(r_u32(spawned_offset + 132) + 40);
                    x = r_u32(spawned_offset + 16);
                    value = r_u32(matrix + 20);
                    parent = r_u32(spawned_offset + 132);
                    w_u32(spawned_offset + 16, x + value);
                    matrix = r_u32(parent + 40);
                    y = r_u32(spawned_offset + 20);
                    value = r_u32(matrix + 24);
                    parent = r_u32(spawned_offset + 132);
                    w_u32(spawned_offset + 20, y + value);
                    matrix = r_u32(parent + 40);
                    z = r_u32(spawned_offset + 24);
                    value = r_u32(matrix + 28);
                    w_u32(spawned_offset + 24, z + value);
                }
                else
                {
                    x = r_u32(spawned_offset + 16);
                    y = r_u32(spawned_offset + 20);
                    z = r_u32(spawned_offset + 24);
                    w_u32(spawned_offset + 16, x);
                    w_u32(spawned_offset + 20, y);
                    w_u32(spawned_offset + 24, z);
                }
                w_u32(spawned_offset + 28, r_u32(spawned_offset + 28) | 0x2000);
            }
            if (phase == 0)
            {
                w_u32(spawned_offset + 28, r_u32(spawned_offset + 28) | 0x80000000);
                if (r_u8(definition_offset + 1) != 19)
                {
                    for (index = 0; index < 3; ++index)
                    {
                        x = r_u32(spawned_offset + 16 + index * 4);
                        y = r_u32(object_offset + 92 + index * 4);
                        w_u32(spawned_offset + 16 + index * 4, x + y);
                    }
                }
            }
        }
    }
    if (phase == 0x40000000)
    {
        if (r_s8(object_offset + 125) == 6)
        {
            flags = r_u32(object_offset + 28);
            if ((flags & 0x20000000) != 0)
            {
                if ((flags & 0x400) == 0)
                    w_u32(spawned_offset + 28, r_u32(spawned_offset + 28) & 0xFFFFFBFF);
                if (r_u32(object_offset + 92) + 26754 > 0xD103 || r_u32(object_offset + 96) + 26754 > 0xD103 || r_u32(object_offset + 100) + 26754 > 0xD103)
                {
                    for (;;)
                    {
                        x = r_u32(object_offset + 92);
                        z = r_u32(object_offset + 100);
                        w_u32(object_offset + 92, (uint32)((sint32)x >> 1));
                        y = r_u32(object_offset + 96);
                        x = r_u32(object_offset + 92);
                        z = (uint32)((sint32)z >> 1);
                        w_u32(object_offset + 100, z);
                        w_u32(object_offset + 96, (uint32)((sint32)y >> 1));
                        if (x + 26754 > 0xD103)
                            continue;
                        if (r_u32(object_offset + 96) + 26754 > 0xD103 || z + 26754 > 0xD103)
                            continue;
                        break;
                    }
                }
                VectorNormal((VECTOR *)psx_addr(object_offset + 92, 12), (VECTOR *)psx_addr(object_offset + 92, 12));
                state = r_s8(object_offset + 120);
                if (state == -1)
                {
                    owner = p2_func_800536f0(0);
                    x = r_u32(object_offset + 92);
                    factor = r_u32(definition_offset + 24);
                    w_u32(spawned_offset + 92, (uint32)(((sint64)(sint32)x * (sint32)factor) >> 12));
                    factor = r_u32(definition_offset + 24);
                    y = r_u32(object_offset + 96);
                    w_u32(spawned_offset + 96, (uint32)(((sint64)(sint32)y * (sint32)factor) >> 12));
                    factor = r_u32(definition_offset + 24);
                    z = r_u32(object_offset + 100);
                    w_u32(spawned_offset + 100, (uint32)(((sint64)(sint32)z * (sint32)factor) >> 12));
                    value = r_u16(0x800AC02A) & 15;
                    if (value - 5 >= 2 && value != 7 && value - 10 >= 2)
                    {
                        x = r_u32(spawned_offset + 92);
                        y = r_u32(owner + 92);
                        w_u32(spawned_offset + 92, x + y);
                        x = r_u32(spawned_offset + 100);
                        y = r_u32(owner + 100);
                        w_u32(spawned_offset + 100, x + y);
                    }
                    if ((r_u32(object_offset + 28) & 0x2000) == 0)
                        w_u32(spawned_offset + 96, 0);
                }
                else
                {
                    for (index = 0; index < 3; ++index)
                    {
                        factor = r_u32(definition_offset + 24);
                        value = r_u32(object_offset + 92 + index * 4);
                        w_u32(spawned_offset + 92 + index * 4, (uint32)(((sint64)(sint32)value * (sint32)factor) >> 12));
                    }
                }
                if ((r_u32(object_offset + 28) & 0x20000000) != 0)
                {
                    w_u32(object_offset + 100, 0);
                    w_u32(object_offset + 96, 0);
                    w_u32(object_offset + 92, 0);
                }
                goto orient;
            }
        }
        goto place_projectile;
    }
    if (r_s8(spawned_offset + 120) != -3)
    {
        if (phase == 0x20000000)
            goto place_projectile;
        goto copy_velocity;
    }
    if (phase == 0x20000000)
    {
        x = r_u32(object_offset + 60);
        y = r_u32(object_offset + 64);
        z = r_u32(object_offset + 68);
        w_u32(spawned_offset + 16, x);
        w_u32(spawned_offset + 20, y);
        w_u32(spawned_offset + 24, z);
    }
    if ((r_u8(definition_offset) & 2) == 0)
        goto copy_velocity;
    if (phase != 0x20000000)
        goto orient;
    owner = p2_func_800536f0(0);
    if (owner == 0)
        goto orient;
    if ((r_u32(owner + 228) & 0x100) != 0)
    {
        parent = r_u32(0x8008A114);
        x = r_u32(parent + 16);
        y = r_u32(parent + 20);
        z = r_u32(parent + 24);
        w_u32(spawned_offset + 92, x);
        w_u32(spawned_offset + 96, y);
        w_u32(spawned_offset + 100, z);
        goto aim_first;
    }
    if ((r_u32(owner + 28) & 0x2000) == 0 && (parent = r_u32(owner + 132)) != 0 && r_u32(parent + 40) != 0)
    {
        x = r_u32(owner + 16);
        y = r_u32(owner + 20);
        z = r_u32(owner + 24);
        w_u32(vector, x);
        w_u32(vector + 4, y);
        w_u32(vector + 8, z);
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
        p2_sdk_8007ec64(vector, spawned_offset + 92);
        PopMatrix();
        for (index = 0; index < 3; ++index)
        {
            matrix = r_u32(r_u32(owner + 132) + 40);
            value = r_u32(spawned_offset + 92 + index * 4);
            x = r_u32(matrix + 20 + index * 4);
            w_u32(spawned_offset + 92 + index * 4, value + x);
        }
    }
    else
    {
        x = r_u32(owner + 16);
        y = r_u32(owner + 20);
        z = r_u32(owner + 24);
        w_u32(spawned_offset + 92, x);
        w_u32(spawned_offset + 96, y);
        w_u32(spawned_offset + 100, z);
    }
aim_first:
    w_u32(spawned_offset + 96, r_u32(spawned_offset + 96) + 12288);
    x = r_u32(spawned_offset + 92);
    a = r_u32(spawned_offset + 16);
    y = r_u32(spawned_offset + 96);
    b = r_u32(spawned_offset + 20);
    z = r_u32(spawned_offset + 100);
    c = r_u32(spawned_offset + 24);
    w_u32(spawned_offset + 92, x - a);
    w_u32(spawned_offset + 96, y - b);
    w_u32(spawned_offset + 100, z - c);
    goto orient;
copy_velocity:
    x = r_u32(object_offset + 92);
    y = r_u32(object_offset + 96);
    z = r_u32(object_offset + 100);
    w_u32(spawned_offset + 92, x);
    w_u32(spawned_offset + 96, y);
    w_u32(spawned_offset + 100, z);
    goto orient;
place_projectile:
    if ((r_u32(spawned_offset + 28) & 0x2000) == 0)
    {
        parent = r_u32(spawned_offset + 132);
        if (parent != 0 && r_u32(parent + 40) != 0)
        {
            x = r_u32(spawned_offset + 16);
            y = r_u32(spawned_offset + 20);
            z = r_u32(spawned_offset + 24);
            w_u32(world, x);
            w_u32(world + 4, y);
            w_u32(world + 8, z);
            PushMatrix();
            matrix = r_u32(r_u32(spawned_offset + 132) + 40);
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
            p2_sdk_8007ec64(world, spawned_offset + 16);
            PopMatrix();
            matrix = r_u32(r_u32(spawned_offset + 132) + 40);
            x = r_u32(spawned_offset + 16);
            value = r_u32(matrix + 20);
            parent = r_u32(spawned_offset + 132);
            w_u32(spawned_offset + 16, x + value);
            matrix = r_u32(parent + 40);
            y = r_u32(spawned_offset + 20);
            value = r_u32(matrix + 24);
            parent = r_u32(spawned_offset + 132);
            w_u32(spawned_offset + 20, y + value);
            matrix = r_u32(parent + 40);
            z = r_u32(spawned_offset + 24);
            value = r_u32(matrix + 28);
            w_u32(spawned_offset + 24, z + value);
        }
        else
        {
            x = r_u32(spawned_offset + 16);
            y = r_u32(spawned_offset + 20);
            z = r_u32(spawned_offset + 24);
            w_u32(spawned_offset + 16, x);
            w_u32(spawned_offset + 20, y);
            w_u32(spawned_offset + 24, z);
        }
        if (r_u8(definition_offset + 1) == 19 && r_u32(r_u32(spawned_offset + 132) + 40) != 0)
            w_u32(spawned_offset + 28, r_u32(spawned_offset + 28) | 0x2000);
    }
    if ((r_u8(definition_offset) & 2) != 0)
    {
        owner = p2_func_800536f0(0);
        if (owner != object_offset)
        {
            if ((r_u32(owner + 28) & 0x2000) == 0 && (parent = r_u32(owner + 132)) != 0 && r_u32(parent + 40) != 0)
            {
                x = r_u32(owner + 16);
                y = r_u32(owner + 20);
                z = r_u32(owner + 24);
                w_u32(world, x);
                w_u32(world + 4, y);
                w_u32(world + 8, z);
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
                p2_sdk_8007ec64(world, spawned_offset + 92);
                PopMatrix();
                for (index = 0; index < 3; ++index)
                {
                    matrix = r_u32(r_u32(owner + 132) + 40);
                    value = r_u32(spawned_offset + 92 + index * 4);
                    x = r_u32(matrix + 20 + index * 4);
                    w_u32(spawned_offset + 92 + index * 4, value + x);
                }
            }
            else
            {
                x = r_u32(owner + 16);
                y = r_u32(owner + 20);
                z = r_u32(owner + 24);
                w_u32(spawned_offset + 92, x);
                w_u32(spawned_offset + 96, y);
                w_u32(spawned_offset + 100, z);
            }
            w_u32(spawned_offset + 96, r_u32(spawned_offset + 96) + 12288);
            x = r_u32(spawned_offset + 92);
            a = r_u32(spawned_offset + 16);
            y = r_u32(spawned_offset + 96);
            b = r_u32(spawned_offset + 20);
            z = r_u32(spawned_offset + 100);
            c = r_u32(spawned_offset + 24);
            w_u32(spawned_offset + 92, x - a);
            w_u32(spawned_offset + 96, y - b);
            w_u32(spawned_offset + 100, z - c);
        }
    }
    x = r_u32(spawned_offset + 92);
    y = r_u32(spawned_offset + 96);
    z = r_u32(spawned_offset + 100);
    w_u32(vector, x);
    w_u32(vector + 4, y);
    w_u32(vector + 8, z);
    x = r_u32(vector);
    w_u32(vector + 4, 0);
    if (x + 26754 > 0xD103 || r_u32(vector + 8) + 26754 > 0xD103)
    {
        do
        {
            x = (uint32)(r_s32(vector) >> 1);
            y = (uint32)(r_s32(vector + 4) >> 1);
            z = (uint32)(r_s32(vector + 8) >> 1);
            w_u32(vector, x);
            w_u32(vector + 4, y);
            w_u32(vector + 8, z);
        } while (x + 26754 > 0xD103 || y + 26754 > 0xD103 || z + 26754 > 0xD103);
    }
    VectorNormal((VECTOR *)psx_addr(vector, 12), (VECTOR *)psx_addr(vector, 12));
    w_u32(spawned_offset + 20, r_u32(spawned_offset + 20) + 20480);
    kind = r_u8(definition_offset + 1);
    if (kind == 19)
    {
        if ((r_u32(r_u32(object_offset + 132) + 16) & 0x10000) != 0)
        {
            parent = r_u32(spawned_offset + 132);
            if ((r_u32(parent + 16) & 0x10000) != 0)
            {
                if (parent != 0 && r_u32(parent + 40) != 0)
                {
                    x = r_u32(spawned_offset + 16);
                    y = r_u32(spawned_offset + 20);
                    z = r_u32(spawned_offset + 24);
                    w_u32(input, x);
                    w_u32(input + 4, y);
                    w_u32(input + 8, z);
                    PushMatrix();
                    matrix = r_u32(r_u32(spawned_offset + 132) + 40);
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
                    p2_sdk_8007ec64(input, world);
                    PopMatrix();
                    for (index = 0; index < 3; ++index)
                    {
                        matrix = r_u32(r_u32(spawned_offset + 132) + 40);
                        value = r_u32(world + index * 4);
                        x = r_u32(matrix + 20 + index * 4);
                        w_u32(world + index * 4, value + x);
                    }
                }
                else
                {
                    x = r_u32(spawned_offset + 16);
                    y = r_u32(spawned_offset + 20);
                    z = r_u32(spawned_offset + 24);
                    w_u32(world, x);
                    w_u32(world + 4, y);
                    w_u32(world + 8, z);
                }
                owner = r_u32(0x8008A114);
                x = r_u32(world);
                y = r_u32(world + 4);
                z = r_u32(world + 8);
                w_u32(inverse, x);
                w_u32(inverse + 4, y);
                w_u32(inverse + 8, z);
                p2_func_8003af18(spawned_offset, r_u32(owner + 136), world, inverse);
            }
            kind = r_u8(definition_offset + 1);
        }
    }
    if (kind == 19)
    {
        reverse = (r_u32(r_u32(object_offset + 132) + 16) & 0x10000) != 0;
        x = r_u32(vector);
        scaled_x = (uint32)(((sint64)(sint32)x * 20480) >> 12);
        value = r_u32(spawned_offset + 16);
        w_u32(spawned_offset + 16, reverse != 0 ? value - scaled_x : value + scaled_x);
        z = r_u32(vector + 8);
        scaled_z = (uint32)(((sint64)(sint32)z * 20480) >> 12);
        value = r_u32(spawned_offset + 24);
        w_u32(spawned_offset + 24, reverse != 0 ? value - scaled_z : value + scaled_z);
    }
    else if (r_u8(definition_offset + 1) == 0 && r_u32(0x8008A498) != 0 && (r_u32(r_u32(object_offset + 132) + 16) & 0x10000) != 0)
    {
        x = r_u32(vector);
        factor = r_u32(0x8008A12C);
        scaled_x = (uint32)(((sint64)(sint32)x * (sint32)factor) >> 12);
        value = r_u32(spawned_offset + 16);
        factor = r_u32(0x8008A12C);
        w_u32(spawned_offset + 16, value + scaled_x);
        z = r_u32(vector + 8);
        scaled_z = (uint32)(((sint64)(sint32)z * (sint32)factor) >> 12);
        w_u32(spawned_offset + 24, r_u32(spawned_offset + 24) + scaled_z);
    }
    else
    {
        x = r_u32(vector);
        value = r_u32(spawned_offset + 16);
        w_u32(spawned_offset + 16, value);
        z = r_u32(vector + 8);
        w_u32(spawned_offset + 24, r_u32(spawned_offset + 24));
    }
    flags = r_u32(spawned_offset + 28);
    if ((flags & 0x2000) == 0)
    {
        parent = r_u32(spawned_offset + 132);
        result = p2_func_800231dc(parent, spawned_offset + 16, flags & 0x10, -40960);
        parent = r_u32(spawned_offset + 132);
        w_u16(spawned_offset + 136, result);
        if (r_u32(parent + 32) == 0 || (sint16)result < 0)
            goto no_surface;
        flags = r_u32(spawned_offset + 28);
        parent = r_u32(spawned_offset + 132);
        result = (uint32)p2_func_8003d3fc(parent, (sint16)result, spawned_offset + 16, vector, flags & 0x10, 4096);
        w_u16(spawned_offset + 136, result);
        if ((sint16)result < 0)
            goto no_surface;
        y = r_u32(spawned_offset + 20);
        w_u32(vector + 4, y);
        parent = r_u32(spawned_offset + 132);
        if (parent != 0 && (matrix = r_u32(parent + 40)) != 0)
        {
            x = r_u32(vector);
            value = r_u32(matrix + 20);
            w_u32(inverse, x - value);
            matrix = r_u32(r_u32(spawned_offset + 132) + 40);
            value = r_u32(matrix + 24);
            w_u32(inverse + 4, y - value);
            matrix = r_u32(r_u32(spawned_offset + 132) + 40);
            z = r_u32(vector + 8);
            value = r_u32(matrix + 28);
            w_u32(inverse + 8, z - value);
            for (index = 0; index < 9; ++index)
            {
                matrix = r_u32(r_u32(spawned_offset + 132) + 40);
                value = r_u16(matrix + (index % 3) * 6 + (index / 3) * 2);
                w_u16(matrix_local + index * 2, value);
            }
            PushMatrix();
            x = r_u32(matrix_local);
            y = r_u32(matrix_local + 4);
            xport_gte_write_control(0, x);
            xport_gte_write_control(1, y);
            x = r_u32(matrix_local + 8);
            y = r_u32(matrix_local + 12);
            z = r_u16(matrix_local + 16);
            xport_gte_write_control(2, x);
            xport_gte_write_control(3, y);
            xport_gte_write_control(4, z);
            p2_sdk_8007ec64(inverse, spawned_offset + 16);
            PopMatrix();
        }
        else
        {
            x = r_u32(vector);
            y = r_u32(vector + 4);
            z = r_u32(vector + 8);
            w_u32(spawned_offset + 16, x);
            w_u32(spawned_offset + 20, y);
            w_u32(spawned_offset + 24, z);
        }
    }
    goto orient;
no_surface:
    w_u32(spawned_offset + 28, r_u32(spawned_offset + 28) | 0x2000);
orient:
    if (phase != 0)
    {
        value = r_u32(spawned_offset + 92);
        w_u16(spawned_offset + 84, 0x7FFF);
        if (value != 0 || r_u32(spawned_offset + 96) != 0 || r_u32(spawned_offset + 100) != 0)
        {
            value = (uint32)p2_func_800185c8(spawned_offset, spawned_offset + 92, 0);
            w_u16(spawned_offset + 86, (value - 1024) & 0xFFF);
            value = p2_func_80018144(spawned_offset, spawned_offset + 92, 0);
            w_u16(spawned_offset + 88, (value + 2048) & 0xFFF);
        }
        else if (r_s8(object_offset + 125) == 6)
        {
            w_u16(spawned_offset + 150, r_u16(object_offset + 150));
            w_u16(spawned_offset + 154, r_u16(object_offset + 154));
            w_u16(spawned_offset + 88, (r_u16(object_offset + 150) + 2048) & 0xFFF);
            w_u16(spawned_offset + 86, r_u16(object_offset + 154) & 0xFFF);
        }
        else
        {
            w_u16(spawned_offset + 150, r_u16(object_offset + 150));
            if (r_u32(object_offset + 92) != 0 || r_u32(object_offset + 96) != 0 || r_u32(object_offset + 100) != 0)
                w_u16(spawned_offset + 154, p2_func_800185c8(object_offset, object_offset + 92, 0));
            else
                w_u16(spawned_offset + 154, 0);
            value = r_u16(spawned_offset + 154);
            flags = r_u16(spawned_offset + 150);
            w_u16(spawned_offset + 154, (value - 1024) & 0xFFF);
            value = r_u16(spawned_offset + 154);
            w_u16(spawned_offset + 88, (flags + 2048) & 0xFFF);
            w_u16(spawned_offset + 86, value & 0xFFF);
        }
    }
    if (phase == 0x40000000 || (phase == 0x80000000 && r_s8(object_offset + 125) != 6))
    {
        x = r_u32(spawned_offset + 16);
        y = r_u32(spawned_offset + 20);
        z = r_u32(spawned_offset + 24);
        w_u32(spawned_offset + 160, x);
        w_u32(spawned_offset + 164, y);
        w_u32(spawned_offset + 168, z);
        x = r_u32(spawned_offset + 160);
        y = r_u32(spawned_offset + 164);
        z = r_u32(spawned_offset + 168);
        w_u32(spawned_offset + 60, x);
        w_u32(spawned_offset + 64, y);
        w_u32(spawned_offset + 68, z);
        w_u32(spawned_offset + 172, 12288);
        w_u32(spawned_offset + 80, 12288);
        w_u32(spawned_offset + 76, 12288);
        w_u32(spawned_offset + 72, 12288);
    }
    if (phase == 0x40000000 && (r_s8(object_offset + 125) != 6 || (r_u32(object_offset + 28) & 0x20) != 0))
    {
        w_u32(spawned_offset, r_u32(object_offset));
        w_u32(object_offset, spawned_offset);
        w_u32(spawned_offset + 144, 0);
        w_u32(spawned_offset + 200, 0);
        w_u32(spawned_offset + 192, 0);
        w_u32(spawned_offset + 184, 0);
        w_u32(spawned_offset + 176, 0);
        w_u16(spawned_offset + 138, 0xFFFF);
    }
    else
    {
        parent = r_u32(0x8008A4E8);
        w_u32(spawned_offset + 4, r_u32(parent + 108));
        w_u32(parent + 108, spawned_offset);
    }
    p2_guest_stack_pointer = saved_stack;
}

uint32 p2_func_8001db60(uint32 object_offset, uint32 character_offset, uint32 grounded)
{
    uint32 saved_stack, record, fraction, old_velocity, world, inverse, matrix_local, floor, input;
    uint32 owner, contact, type, pointer, triangle, parent, matrix, index, row, axis;
    uint32 x, y, z, a, b, c, d, e, f, g, h, i, flags, value, aligned, factor, result;
    sint32 query, correction, heading, angle;
    saved_stack = p2_guest_stack_pointer;
    p2_guest_stack_pointer -= 176;
    record = p2_guest_stack_pointer;
    fraction = record + 48;
    old_velocity = record + 64;
    world = record + 80;
    inverse = record + 96;
    matrix_local = record + 112;
    floor = record + 144;
    input = record + 160;
    owner = r_u32(character_offset + 112);
    type = r_u8(owner + 12);
    contact = r_u32(character_offset + 108);
    if (type == 1)
    {
        x = r_u32(0x80010180);
        x = r_u32(0x80010180);
        y = r_u32(0x80010184);
        y = r_u32(0x80010184);
        z = r_u32(0x80010188);
        z = r_u32(0x80010188);
        w_u32(record + 4, x);
        w_u32(record + 4, x);
        w_u32(record + 8, y);
        w_u32(record + 8, y);
        w_u32(record + 12, z);
        w_u32(record + 12, z);
        x = r_u32(0x8001018C);
        x = r_u32(0x8001018C);
        y = (uint32)(sint32)r_s16(0x80010190);
        w_u32(record + 16, x);
        w_u32(record + 16, x);
        w_u16(record + 20, y);
        pointer = r_s16(owner + 34) > 0 ? owner : r_u32(owner + 40);
        x = r_u32(pointer + 16);
        y = r_u32(pointer + 20);
        z = r_u32(pointer + 24);
        w_u32(record + 24, x);
        w_u32(record + 28, y);
        w_u32(record + 32, z);
        w_u32(record + 44, 0);
        w_u32(record + 40, 0);
        w_u32(record + 36, 0);
    }
    else
    {
        pointer = r_u32(owner + 140) + (contact >> 16) * 48;
        for (index = 0; index < 48; index += 16)
        {
            x = r_u32(pointer + index);
            y = r_u32(pointer + index + 4);
            z = r_u32(pointer + index + 8);
            a = r_u32(pointer + index + 12);
            w_u32(record + index, x);
            w_u32(record + index + 4, y);
            w_u32(record + index + 8, z);
            w_u32(record + index + 12, a);
        }
    }
    w_u16(record + 2, 1);
    w_u16(record, contact);
    w_u32(fraction, 4096);
    triangle = r_u32(owner + 36) + (uint16)contact * 24;
    if ((uint8)grounded != 0)
    {
        x = r_u32(object_offset + 92);
        y = r_u32(object_offset + 96);
        z = r_u32(object_offset + 100);
        w_u32(object_offset + 92, x << 2);
        w_u32(object_offset + 96, y << 2);
        w_u32(object_offset + 100, z << 2);
    }
    owner = r_u32(character_offset + 112);
    query = p2_func_800380ac(object_offset, 0x8008A538, owner, record, fraction);
    if ((uint8)grounded != 0)
    {
        x = (uint32)(r_s32(object_offset + 92) >> 2);
        y = (uint32)(r_s32(object_offset + 96) >> 2);
        z = (uint32)(r_s32(object_offset + 100) >> 2);
        w_u32(object_offset + 92, x);
        w_u32(object_offset + 96, y);
        w_u32(object_offset + 100, z);
        if (query < 0)
            goto no_contact;
        if ((p2_func_8001c710(object_offset, record, (uint32)query) & 0xFF) != 0)
            query = -1;
    }
    if (query < 0)
        goto no_contact;
    flags = r_u32(character_offset);
    pointer = r_u32(character_offset + 688);
    w_u32(character_offset + 188, 0);
    w_u32(character_offset + 672, 0);
    w_u32(character_offset, flags | 1);
    value = r_u32(pointer + 36);
    parent = r_u32(object_offset + 132);
    if (value != parent)
    {
        if (parent != 0 && r_u32(parent + 40) != 0)
        {
            x = r_u32(object_offset + 16);
            y = r_u32(object_offset + 20);
            z = r_u32(object_offset + 24);
            w_u32(world, x);
            w_u32(world + 4, y);
            w_u32(world + 8, z);
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
            p2_sdk_8007ec64(world, object_offset + 16);
            PopMatrix();
            matrix = r_u32(r_u32(object_offset + 132) + 40);
            x = r_u32(object_offset + 16);
            value = r_u32(matrix + 20);
            parent = r_u32(object_offset + 132);
            w_u32(object_offset + 16, x + value);
            matrix = r_u32(parent + 40);
            y = r_u32(object_offset + 20);
            value = r_u32(matrix + 24);
            parent = r_u32(object_offset + 132);
            w_u32(object_offset + 20, y + value);
            matrix = r_u32(parent + 40);
            z = r_u32(object_offset + 24);
            value = r_u32(matrix + 28);
            w_u32(object_offset + 24, z + value);
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
        p2_func_80024844(r_u32(character_offset + 688));
        pointer = r_u32(character_offset + 688);
        parent = r_u32(pointer + 36);
        w_u32(object_offset + 132, parent);
        if (parent != 0 && (matrix = r_u32(parent + 40)) != 0)
        {
            for (index = 0; index < 3; ++index)
            {
                if (index != 0)
                    matrix = r_u32(r_u32(object_offset + 132) + 40);
                x = r_u32(object_offset + 16 + index * 4);
                value = r_u32(matrix + 20 + index * 4);
                w_u32(inverse + index * 4, x - value);
            }
            for (index = 0; index < 9; ++index)
            {
                matrix = r_u32(r_u32(object_offset + 132) + 40);
                value = r_u16(matrix + (index % 3) * 6 + (index / 3) * 2);
                w_u16(matrix_local + index * 2, value);
            }
            PushMatrix();
            x = r_u32(matrix_local);
            y = r_u32(matrix_local + 4);
            xport_gte_write_control(0, x);
            xport_gte_write_control(1, y);
            x = r_u32(matrix_local + 8);
            y = r_u32(matrix_local + 12);
            z = r_u16(matrix_local + 16);
            xport_gte_write_control(2, x);
            xport_gte_write_control(3, y);
            xport_gte_write_control(4, z);
            p2_sdk_8007ec64(inverse, object_offset + 16);
            PopMatrix();
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
        w_u32(object_offset + 28, r_u32(object_offset + 28) & 0xFFFFBFFF);
    }
    x = r_u32(object_offset + 92);
    y = r_u32(object_offset + 96);
    z = r_u32(object_offset + 100);
    w_u32(old_velocity, x);
    w_u32(old_velocity + 4, y);
    w_u32(old_velocity + 8, z);
    w_u32(object_offset + 100, 0);
    w_u32(object_offset + 92, 0);
    if ((uint8)grounded == 0)
    {
        if ((r_u16(character_offset + 706) & 15) == 1 && (r_u32(object_offset + 28) & 0x04000000) != 0)
            p2_func_8004a888(object_offset, character_offset);
        flags = r_u32(object_offset + 28);
        w_u32(object_offset + 96, 0);
        w_u32(object_offset + 28, flags & 0xFFFFFBFF);
        if ((flags & 0x2000) == 0)
        {
            parent = r_u32(object_offset + 132);
            if (parent != 0 && r_u32(parent + 40) != 0)
            {
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
                p2_sdk_8007ec64(input, world);
                PopMatrix();
                for (index = 0; index < 3; ++index)
                {
                    matrix = r_u32(r_u32(object_offset + 132) + 40);
                    x = r_u32(world + index * 4);
                    value = r_u32(matrix + 20 + index * 4);
                    w_u32(world + index * 4, x + value);
                }
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
            parent = r_u32(object_offset + 132);
            flags = r_u32(object_offset + 28);
            correction = (sint32)p2_func_800231dc(parent, world, flags & 0x10, -40960);
            if (correction >= 0)
            {
                flags = r_u32(object_offset + 28);
                parent = r_u32(object_offset + 132);
                correction = p2_func_8003d3fc(parent, correction, world, floor, flags & 0x10, 4096);
                if (correction >= 0)
                {
                    pointer = r_u32(0x8008A114);
                    w_u16(pointer + 2, correction);
                    y = r_u32(floor + 4);
                    value = r_u32(world + 4);
                    w_u32(object_offset + 96, y - value);
                }
            }
        }
    }
    else if (r_s32(object_offset + 96) > 0)
    {
        value = ((uint32)query & 0x7000) >> 12;
        if (r_u32(character_offset + 796) != 0 || value >= 4 || r_u32(0x8008A544 + value * 16) == 0xFFFFFFFF)
        {
            a = r_u16(record + 4);
            b = r_u16(record + 10);
            c = r_u16(record + 16);
            d = r_u16(record + 6);
            e = r_u16(record + 12);
            f = r_u16(record + 18);
            g = r_u16(record + 8);
            h = r_u16(record + 14);
            i = r_u16(record + 20);
            w_u16(matrix_local, a);
            w_u16(matrix_local + 2, b);
            w_u16(matrix_local + 4, c);
            w_u16(matrix_local + 6, d);
            w_u16(matrix_local + 8, e);
            w_u16(matrix_local + 10, f);
            w_u16(matrix_local + 12, g);
            w_u16(matrix_local + 14, h);
            w_u16(matrix_local + 16, i);
            PushMatrix();
            x = r_u32(matrix_local);
            y = r_u32(matrix_local + 4);
            xport_gte_write_control(0, x);
            xport_gte_write_control(1, y);
            x = r_u32(matrix_local + 8);
            y = r_u32(matrix_local + 12);
            z = r_u16(matrix_local + 16);
            xport_gte_write_control(2, x);
            xport_gte_write_control(3, y);
            xport_gte_write_control(4, z);
            xport_gte_write_data(0, r_u32(triangle + 16));
            xport_gte_write_data(1, r_u32(triangle + 20));
            xport_gte_execute(0x486012);
            w_u32(floor, xport_gte_read_data(25));
            w_u32(floor + 4, xport_gte_read_data(26));
            w_u32(floor + 8, xport_gte_read_data(27));
            PopMatrix();
            if (r_s32(floor + 4) < -256)
            {
                value = r_u32(character_offset + 768);
                flags = r_u32(character_offset);
                w_u32(character_offset + 32, 0xFFFFFFFF);
                w_u32(character_offset + 28, 0);
                w_u16(character_offset + 48, 0);
                w_u16(character_offset + 684, value);
                w_u32(character_offset, flags | 0x08000000);
                if ((r_u32(character_offset + 4) & 0x80) == 0)
                    w_u8(character_offset + 712, 6);
                w_u32(object_offset + 96, 0);
                result = 6;
                goto finish;
            }
        }
    }
    parent = r_u32(object_offset + 132);
    if (parent != 0 && (matrix = r_u32(parent + 40)) != 0 && (r_u32(object_offset + 28) & 0x2000) == 0)
    {
        for (index = 0; index < 3; ++index)
        {
            if (index != 0)
                matrix = r_u32(r_u32(object_offset + 132) + 40);
            x = r_u32(record + 36 + index * 4);
            value = r_u32(matrix + 32 + index * 4);
            w_u32(record + 36 + index * 4, x - value);
        }
    }
    if (r_u32(record + 36) == 0 && r_u32(record + 44) == 0)
    {
        if (r_s16(character_offset + 676) == 0)
            w_u32(character_offset + 24, 0);
        goto save_frame;
    }
    x = r_u32(old_velocity);
    aligned = 0;
    if (x != 0 || r_u32(old_velocity + 8) != 0)
    {
        z = r_u32(old_velocity + 8);
        heading = (sint16)p2_func_8005dc00((sint32)z, (sint32)x);
        z = r_u32(record + 44);
        x = r_u32(record + 36);
        angle = (sint16)p2_func_8005dc00((sint32)z, (sint32)x);
        aligned = (((uint32)heading - (uint32)angle + 1024) & 0xFFF) < 2048;
    }
    if (r_u32(fraction) != 0)
    {
        w_u32(old_velocity + 8, 0);
        w_u32(old_velocity + 4, 0);
        w_u32(old_velocity, 0);
        goto damp_velocity;
    }
    x = r_u32(record + 36);
    y = r_u32(record + 40);
    z = r_u32(record + 44);
    w_u32(old_velocity, x);
    w_u32(old_velocity + 4, y);
    w_u32(old_velocity + 8, z);
    if (aligned != 0)
    {
        x = r_u32(old_velocity);
        y = r_u32(old_velocity + 4);
        z = r_u32(old_velocity + 8);
        w_u32(old_velocity, 0 - x);
        w_u32(old_velocity + 4, 0 - y);
        w_u32(old_velocity + 8, 0 - z);
    }
    x = r_u32(old_velocity);
    a = (uint32)(((sint64)(sint32)x * 40960) >> 12);
    y = r_u32(old_velocity + 4);
    w_u32(old_velocity, a);
    b = (uint32)(((sint64)(sint32)y * 40960) >> 12);
    z = r_u32(old_velocity + 8);
    w_u32(old_velocity + 4, b);
    w_u32(old_velocity + 8, (uint32)(((sint64)(sint32)z * 40960) >> 12));
    for (row = 0; row < 4; ++row)
    {
        for (axis = 0; axis < 3; ++axis)
        {
            pointer = object_offset + 160 + row * 16 + axis * 4;
            x = r_u32(pointer);
            y = r_u32(old_velocity + axis * 4);
            w_u32(pointer, x + y);
        }
    }
    for (axis = 0; axis < 3; ++axis)
    {
        x = r_u32(object_offset + 92 + axis * 4);
        y = r_u32(old_velocity + axis * 4);
        w_u32(object_offset + 92 + axis * 4, x - y);
    }
    w_u32(fraction, 4096);
    owner = r_u32(character_offset + 112);
    type = r_u8(owner + 12);
    contact = r_u32(character_offset + 108);
    if (type == 1)
    {
        x = r_u32(0x80010180);
        x = r_u32(0x80010180);
        y = r_u32(0x80010184);
        y = r_u32(0x80010184);
        z = r_u32(0x80010188);
        z = r_u32(0x80010188);
        w_u32(record + 4, x);
        w_u32(record + 4, x);
        w_u32(record + 8, y);
        w_u32(record + 8, y);
        w_u32(record + 12, z);
        w_u32(record + 12, z);
        x = r_u32(0x8001018C);
        x = r_u32(0x8001018C);
        y = (uint32)(sint32)r_s16(0x80010190);
        w_u32(record + 16, x);
        w_u32(record + 16, x);
        w_u16(record + 20, y);
        pointer = r_s16(owner + 34) > 0 ? owner : r_u32(owner + 40);
        x = r_u32(pointer + 16);
        y = r_u32(pointer + 20);
        z = r_u32(pointer + 24);
        w_u32(record + 24, x);
        w_u32(record + 28, y);
        w_u32(record + 32, z);
        w_u32(record + 44, 0);
        w_u32(record + 40, 0);
        w_u32(record + 36, 0);
    }
    else
    {
        pointer = r_u32(owner + 140) + (contact >> 16) * 48;
        for (index = 0; index < 48; index += 16)
        {
            x = r_u32(pointer + index);
            y = r_u32(pointer + index + 4);
            z = r_u32(pointer + index + 8);
            a = r_u32(pointer + index + 12);
            w_u32(record + index, x);
            w_u32(record + index + 4, y);
            w_u32(record + index + 8, z);
            w_u32(record + index + 12, a);
        }
    }
    w_u16(record + 2, 1);
    w_u16(record, contact);
    owner = r_u32(character_offset + 112);
    query = p2_func_800380ac(object_offset, 0x8008A538, owner, record, fraction);
    for (axis = 0; axis < 3; ++axis)
    {
        x = r_u32(object_offset + 92 + axis * 4);
        y = r_u32(old_velocity + axis * 4);
        w_u32(object_offset + 92 + axis * 4, x + y);
    }
    for (row = 0; row < 4; ++row)
    {
        for (axis = 0; axis < 3; ++axis)
        {
            pointer = object_offset + 160 + row * 16 + axis * 4;
            x = r_u32(pointer);
            y = r_u32(old_velocity + axis * 4);
            w_u32(pointer, x - y);
        }
    }
    if (query >= 0 && (factor = r_u32(fraction)) != 0)
    {
        factor = 4096 - factor;
        w_u32(fraction, factor);
        x = r_u32(old_velocity);
        a = (uint32)(((sint64)(sint32)x * (sint32)factor) >> 12);
        y = r_u32(old_velocity + 4);
        w_u32(old_velocity, a);
        b = (uint32)(((sint64)(sint32)y * (sint32)factor) >> 12);
        z = r_u32(old_velocity + 8);
        w_u32(old_velocity + 4, b);
        w_u32(old_velocity + 8, (uint32)(((sint64)(sint32)z * (sint32)factor) >> 12));
    }
    else
    {
        w_u32(old_velocity + 8, 0);
        w_u32(old_velocity + 4, 0);
        w_u32(old_velocity, 0);
    }
    if ((r_u32(object_offset + 28) & 0x2000) == 0)
    {
        parent = r_u32(object_offset + 132);
        if (parent != 0 && (matrix = r_u32(parent + 40)) != 0)
        {
            for (index = 0; index < 3; ++index)
            {
                if (index != 0)
                    matrix = r_u32(r_u32(object_offset + 132) + 40);
                x = r_u32(record + 36 + index * 4);
                value = r_u32(matrix + 32 + index * 4);
                w_u32(record + 36 + index * 4, x - value);
            }
        }
    }
damp_velocity:
    factor = aligned != 0 ? 3993 : 4198;
    x = r_u32(record + 36);
    a = (uint32)(((sint64)(sint32)x * (sint32)factor) >> 12);
    y = r_u32(record + 40);
    w_u32(record + 36, a);
    b = (uint32)(((sint64)(sint32)y * (sint32)factor) >> 12);
    z = r_u32(record + 44);
    w_u32(record + 40, b);
    c = (uint32)(((sint64)(sint32)z * (sint32)factor) >> 12);
    w_u32(record + 44, c);
    if (aligned != 0)
    {
        w_u32(object_offset + 92, r_u32(object_offset + 92) + a);
        y = r_u32(object_offset + 96);
        value = r_u32(record + 40);
        w_u32(object_offset + 96, y + value);
        z = r_u32(object_offset + 100);
        value = r_u32(record + 44);
        w_u32(object_offset + 100, z + value);
    }
    else
    {
        x = r_u32(old_velocity);
        y = r_u32(old_velocity + 4);
        z = r_u32(old_velocity + 8);
        w_u32(old_velocity, a + x);
        w_u32(old_velocity + 4, b + y);
        w_u32(old_velocity + 8, c + z);
        if (r_s16(character_offset + 676) == 0)
            w_u32(character_offset + 24, 0);
    }
    z = r_u32(old_velocity + 8);
    x = r_u32(old_velocity);
    y = r_u32(old_velocity + 4);
    p2_func_8001d3f0(object_offset, character_offset, (sint32)x, (sint32)y, (sint32)z);
save_frame:
    pointer = r_u32(0x8008A114);
    flags = r_u32(object_offset + 28);
    value = r_u16(pointer + 2);
    result = flags & 0xFFFFF7FF;
    w_u32(object_offset + 28, result);
    w_u16(object_offset + 136, value);
    goto finish;
no_contact:
    result = r_u32(object_offset + 92);
    if (result == 0)
        result = r_u32(object_offset + 100);
    if (result != 0)
    {
        result = r_u32(character_offset) & 0xFFFFFFBF;
        w_u32(character_offset, result);
    }
finish:
    p2_guest_stack_pointer = saved_stack;
    return result;
}

uint32 p2_func_80030dcc(uint32 object_offset, uint32 position_offset, uint32 target_offset, uint32 distance_offset)
{
    uint32 saved_stack, delta, input, matrix_local, definition, resource;
    uint32 flags, value, parent, matrix, x, y, z, amount, repetitions;
    uint32 heading, candidate, angle, animation, alternate, sound_table, sound, pitch;
    uint32 owner, scene, squared, result, index, next;
    sint32 kind, frame, limit, timer;
    definition = r_u32(r_u32(object_offset + 132) + 44);
    flags = r_u32(definition + 44);
    resource = r_u32(definition + 72);
    saved_stack = p2_guest_stack_pointer;
    p2_guest_stack_pointer -= 64;
    delta = p2_guest_stack_pointer;
    input = delta + 16;
    matrix_local = delta + 32;
    if ((flags & 0x20000) != 0)
    {
        p2_func_80027b1c(object_offset, 0, 8);
        w_u16(object_offset + 128, 0);
    }
    if ((r_u32(definition + 44) & 4) != 0)
    {
        limit = r_u16(definition + 40);
        if (r_s32(object_offset + 144) >= limit)
        {
            parent = r_u32(definition + 52);
            w_u8(parent + 14, r_u8(parent + 14) | 8);
        }
    }
    if ((r_u32(definition + 44) & 0x28) != 0)
    {
        kind = r_s8(resource + 633);
        if (kind == -1)
            kind = r_s8(definition + 37);
        if (kind < 44)
        {
            sound = r_u32(definition + 68);
            value = (uint32)r_s16(definition + 66);
            if ((p2_func_8005e868(sound, value) & 0xFF) != 0)
                p2_func_8006a970(r_u16(definition + 66));
            w_u32(definition + 68, 0);
            w_u16(definition + 66, 0xFFFF);
        }
        w_u32(0x8008A184, 0);
        kind = r_s8(resource + 633);
        if (kind == -1)
            kind = r_s8(definition + 37);
        if (kind != 30)
        {
            kind = r_s8(resource + 633);
            if (kind == -1)
                kind = r_s8(definition + 37);
            if (kind != 31)
                goto consume_timer;
        }
        flags = r_u32(definition + 44) & 0xFFFFFFDF;
        w_u32(definition + 44, flags);
        frame = r_s16(object_offset + 130);
        if (frame == 0 || frame == r_s16(r_u32(object_offset + 104) + 6) - 1)
        {
            w_u32(definition + 44, flags | 0x20);
            w_u32(0x8008A184, 1);
        }
    consume_timer:
        flags = r_u32(definition + 44);
        amount = flags >> 9;
        if ((flags & 0x20) != 0 || r_s16(object_offset + 128) < (sint32)amount)
            amount = (uint32)r_s16(object_offset + 128);
        if (amount != 0)
        {
            w_u16(object_offset + 128, r_u16(object_offset + 128) - amount);
            repetitions = (uint32)r_s16(resource + 662);
            if (repetitions != 0)
            {
                --repetitions;
                do
                {
                    if (r_s16(object_offset + 128) == 0)
                    {
                        p2_func_80027b1c(object_offset, definition, 2);
                        p2_func_800444d0();
                    }
                    --repetitions;
                } while (repetitions != 0xFFFFFFFF);
            }
            if (r_s16(object_offset + 128) == 0)
            {
                result = 0;
                goto finish_state;
            }
        }
        kind = r_s8(resource + 633);
        if (kind == -1)
            kind = r_s8(definition + 37);
        if (kind != 19)
        {
            parent = r_u32(object_offset + 132);
            w_u32(parent + 16, r_u32(parent + 16) | 0x40);
        }
    }
    if (r_u32(object_offset + 144) != 0 && (r_u32(object_offset + 92) != 0 || r_u32(object_offset + 96) != 0 || r_u32(object_offset + 100) != 0))
    {
        heading = p2_func_80018144(object_offset, object_offset + 92, 0);
        if ((sint16)heading == 4096)
            candidate = 0;
        else
        {
            candidate = r_u16(object_offset + 148) - heading;
            if ((sint16)candidate < 0)
                candidate = 0 - candidate;
            if ((((uint32)(sint32)(sint16)candidate - 1024) & 0xFFF) < 2048 && r_u32(object_offset + 132) != 0 && r_u32(definition + 56) != 0)
            {
                z = r_u32(object_offset + 100);
                x = r_u32(object_offset + 92);
                angle = p2_func_8005dc00((sint32)(0 - z), (sint32)(0 - x));
                if (((((uint32)(sint32)(sint16)heading - (uint32)(sint32)(sint16)angle) + 128) & 0xFFF) >= 257)
                    candidate = 0;
            }
        }
        parent = r_u32(definition + 72);
        if (r_s16(parent + 68) < (sint16)candidate && r_s16(parent + 652) == 0)
        {
            value = r_u16(definition + 28);
            w_u32(definition + 16, 0);
            w_u16(object_offset + 136, value);
            goto after_motion;
        }
        parent = r_u32(object_offset + 132);
        if (parent != 0 && (matrix = r_u32(parent + 40)) != 0)
        {
            w_u16(matrix_local, r_u16(matrix));
            for (index = 1; index < 9; ++index)
            {
                matrix = r_u32(r_u32(object_offset + 132) + 40);
                w_u16(matrix_local + index * 2, r_u16(matrix + (index % 3) * 6 + (index / 3) * 2));
            }
            x = r_u32(object_offset + 92);
            y = r_u32(object_offset + 96);
            z = r_u32(object_offset + 100);
            w_u32(input, x);
            w_u32(input + 4, y);
            w_u32(input + 8, z);
            PushMatrix();
            x = r_u32(matrix_local);
            y = r_u32(matrix_local + 4);
            xport_gte_write_control(0, x);
            xport_gte_write_control(1, y);
            x = r_u32(matrix_local + 8);
            y = r_u32(matrix_local + 12);
            z = r_u16(matrix_local + 16);
            xport_gte_write_control(2, x);
            xport_gte_write_control(3, y);
            xport_gte_write_control(4, z);
            p2_sdk_8007ec64(input, delta);
            PopMatrix();
        }
        else
        {
            x = r_u32(object_offset + 92);
            y = r_u32(object_offset + 96);
            z = r_u32(object_offset + 100);
            w_u32(delta, x);
            w_u32(delta + 4, y);
            w_u32(delta + 8, z);
        }
        x = (uint32)(((sint64)r_s32(delta) * r_s32(object_offset + 144)) >> 12);
        value = r_u32(delta + 4);
        w_u32(delta, x);
        y = (uint32)(((sint64)(sint32)value * r_s32(object_offset + 144)) >> 12);
        value = r_u32(delta + 8);
        w_u32(delta + 4, y);
        z = (uint32)(((sint64)(sint32)value * r_s32(object_offset + 144)) >> 12);
        w_u32(delta + 8, z);
        kind = r_s8(object_offset + 125);
        if (r_u32(0x800BA380 + (uint32)kind * 4) == 0x80020C6C && (r_u32(object_offset + 28) & 0x10) == 0)
        {
            w_u32(delta, 0 - x);
            w_u32(delta + 4, 0 - y);
            w_u32(delta + 8, 0 - z);
        }
        x = r_u32(object_offset + 16);
        w_u32(object_offset + 16, x + r_u32(delta));
        y = r_u32(object_offset + 20);
        w_u32(object_offset + 20, y + r_u32(delta + 4));
        z = r_u32(object_offset + 24);
        w_u32(object_offset + 24, z + r_u32(delta + 8));
    }
    else
        w_u16(object_offset + 136, r_u16(definition + 28));
after_motion:
    flags = r_u32(definition + 44);
    if ((flags & 1) != 0)
    {
        kind = r_s8(object_offset + 125);
        w_u8(object_offset + 125, 17);
        w_u32(object_offset + 116, ((uint32)kind << 8) | 0x96);
        goto after_reaction;
    }
    if ((flags & 8) != 0)
    {
        kind = r_s8(resource + 633);
        if (kind == -1)
        {
            if (r_s8(definition + 37) >= 44)
                goto large_kind_reaction;
            kind = r_s8(resource + 633);
        }
        else if (kind >= 44)
            goto large_kind_reaction;
        if (kind == -1)
            kind = r_s8(definition + 37);
        if (kind == 10)
            goto blink;
        if (r_s16(resource + 636) != 1 || r_s16(object_offset + 128) == 0)
        {
            flags = r_u32(object_offset + 28);
            angle = (uint32)r_s16(object_offset + 148);
            kind = r_s8(object_offset + 125);
            w_u32(object_offset + 28, flags | 0x2000000);
            w_u32(definition + 220, (angle << 16) | ((uint32)kind << 8) | 10);
            angle = r_u16(object_offset + 148);
            w_u8(object_offset + 126, 0);
            w_u8(object_offset + 125, 16);
            w_u16(object_offset + 150, angle << 3);
        }
        goto blink;
    large_kind_reaction:
        if ((r_u32(definition + 44) & 8) == 0)
            goto after_reaction;
        kind = r_s8(resource + 633);
        if (kind == -1)
            kind = r_s8(definition + 37);
        if (kind >= 47)
            goto after_reaction;
    blink:
        w_u16(definition + 224, 1);
        w_u16(definition + 226, 0);
        w_u32(object_offset + 28, r_u32(object_offset + 28) | 0x80000);
    }
after_reaction:
    if ((r_u32(object_offset + 28) & 0x30000000) != 0)
        p2_func_80030c2c(object_offset);
    flags = r_u32(definition + 44);
    if ((flags & 0x10) != 0)
    {
        if ((flags & 0x80) != 0)
            goto active_route;
        if (r_u32(r_u32(object_offset + 132) + 32) != 0 && (r_u32(object_offset + 92) != 0 || r_u32(object_offset + 96) != 0 || r_u32(object_offset + 100) != 0))
        {
            if (r_s8(definition + 180) == 0)
            {
                w_u32(definition + 44, r_u32(definition + 44) | 0x80);
                p2_func_80035154(definition + 104, definition + 108, object_offset);
                w_u8(definition + 180, 1);
                if ((r_u32(object_offset + 28) & 0x10) != 0)
                    value = r_u8(definition + 38) | 0x20;
                else
                    value = r_u8(definition + 38) & 0xDF;
                w_u8(definition + 38, value);
            }
            goto active_route;
        }
        parent = r_u32(object_offset + 132);
        flags = r_u32(object_offset + 28);
        frame = r_u16(object_offset + 136);
        value = r_u16(parent + 26);
        w_u32(object_offset + 28, (flags & 0xFFFFF7FF) ^ 0x10);
        w_u16(object_offset + 136, value - frame);
        value = r_u32(definition + 44) | 0x80;
    }
    else
        value = flags & 0xFFFFFF7F;
    w_u32(definition + 44, value);
active_route:
    if ((r_u32(object_offset + 28) & 0x800) != 0)
    {
        flags = r_u32(object_offset + 28);
        parent = r_u32(object_offset + 132);
        w_u16(object_offset + 136, 0);
        flags &= 0xFFFFF7FF;
        w_u32(object_offset + 28, flags);
        if ((r_u8(parent + 22) & 2) == 0)
            w_u32(object_offset + 28, flags ^ 0x10);
    }
    if ((r_u8(definition + 38) & 2) == 0)
        goto animation_done;
    if ((r_u32(0x8008A178) & 4) == 0 && r_u32(0x8008A17C) == definition)
    {
        p2_func_80027b1c(object_offset, 0, 9);
        w_u32(0x8008A178, r_u32(0x8008A178) | 12);
    }
    timer = r_s32(definition + 76);
    if (timer == 0)
    {
        sound = r_u32(definition + 68);
        value = (uint32)r_s16(definition + 66);
        if ((p2_func_8005e868(sound, value) & 0xFF) != 0)
            p2_func_8006a970(r_u16(definition + 66));
        sound_table = r_u32(0x8008A408);
        w_u32(definition + 68, 0);
        w_u16(definition + 66, 0xFFFF);
        sound = 0;
        if (sound_table != 0 && r_s32(sound_table) >= 13)
            sound = sound_table + 100;
        pitch = r_u16(sound + 4);
        p2_func_8005eb18(sound, 8192, 0, pitch, object_offset + 60, 0x4B000, 0xFFFFFFFF, 0, 0);
        animation = r_u32(r_u32(definition + 72) + 88);
        w_u32(object_offset + 104, animation);
        animation = r_u32(object_offset + 104);
        w_u32(object_offset + 100, 0);
        w_u32(object_offset + 96, 0);
        w_u32(object_offset + 92, 0);
        w_u16(object_offset + 130, 0xFFFF);
        value = r_u16(animation + 6);
        w_u32(definition + 76, (uint32)(((sint32)(sint16)value >> 1) + 45));
        goto animation_done;
    }
    if (timer < 0)
    {
        next = (uint32)timer + 1;
        w_u32(definition + 76, next);
        if (next == 0)
        {
            parent = r_u32(definition + 72);
            animation = r_u32(object_offset + 104);
            alternate = r_u32(parent + 92);
            if (animation == alternate)
            {
                animation = r_u32(parent + 88);
                w_u32(object_offset + 104, animation);
                value = r_u16(animation + 6);
                w_u16(object_offset + 130, value);
                w_u32(definition + 76, 0 - (uint32)(sint32)(sint16)value);
                goto animation_done;
            }
            animation = r_u32(parent + 72);
            w_u16(object_offset + 130, 0xFFFF);
            w_u32(object_offset + 104, animation);
            kind = r_s8(resource + 633);
            if (kind == -1)
                kind = r_s8(definition + 37);
            if (kind == 10)
                w_u32(object_offset + 28, r_u32(object_offset + 28) & 0xFFFFFFBF);
            w_u8(definition + 38, r_u8(definition + 38) ^ 2);
            value = r_u32(0x8008A17C);
            w_u32(definition + 44, 0);
            if (value == definition)
                w_u32(0x8008A178, r_u32(0x8008A178) & 0xFFFFFFDB);
            w_u32(object_offset + 28, r_u32(object_offset + 28) | 0x20000);
            goto animation_done;
        }
        if (next == (uint32)-45)
        {
            animation = r_u32(r_u32(definition + 72) + 88);
            w_u32(object_offset + 104, animation);
            w_u16(object_offset + 130, r_u16(animation + 6) - 1);
            w_u32(definition + 76, 45);
            goto animation_done;
        }
        if ((r_u32(0x8008A178) & 8) == 0 || r_u32(0x8008A17C) != definition)
            goto animation_done;
        animation = r_u32(object_offset + 104);
        frame = r_s16(object_offset + 130);
        limit = r_s16(animation + 6);
        sound_table = r_u32(0x8008A408);
        w_u32(definition + 76, (uint32)(((limit - frame) >> 1) + 45));
        sound = 0;
        if (sound_table != 0 && r_s32(sound_table) >= 13)
            sound = sound_table + 100;
        parent = r_u32(definition + 72);
        animation = r_u32(object_offset + 104);
        alternate = r_u32(parent + 92);
        pitch = r_u16(sound + 4);
        if (animation == alternate)
            pitch += pitch >> 3;
        goto play_state_sound;
    }
    next = (uint32)timer - 1;
    w_u32(definition + 76, next);
    if ((sint32)next < 45 && (r_u32(0x8008A178) & 8) != 0 && r_u32(0x8008A17C) == definition)
    {
        parent = r_u32(definition + 72);
        animation = r_u32(object_offset + 104);
        alternate = r_u32(parent + 92);
        if (animation == alternate)
        {
            sound_table = r_u32(0x8008A408);
            sound = 0;
            if (sound_table != 0 && r_s32(sound_table) >= 15)
                sound = sound_table + 116;
            pitch = r_u16(sound + 4);
            p2_func_8005eb18(sound, 8192, 0, pitch, object_offset + 60, 0x4B000, 0xFFFFFFFF, 0, 0);
            w_u16(object_offset + 128, 0);
            goto animation_done;
        }
        if (animation != r_u32(parent + 88))
            goto animation_done;
        limit = r_s16(animation + 6);
        frame = r_s16(object_offset + 130);
        if (frame != limit - 1)
            goto animation_done;
        w_u32(object_offset + 104, alternate);
        animation = r_u32(object_offset + 104);
        w_u16(object_offset + 130, 0xFFFF);
        value = r_u16(animation + 6);
        sound_table = r_u32(0x8008A408);
        w_u32(definition + 76, (uint32)(((sint32)(sint16)value >> 1) + 45));
        sound = 0;
        if (sound_table != 0 && r_s32(sound_table) >= 13)
            sound = sound_table + 100;
        pitch = r_u16(sound + 4);
        pitch += pitch >> 3;
        goto play_state_sound;
    }
    if (r_u32(definition + 76) != 0)
        goto animation_done;
    animation = r_u32(object_offset + 104);
    limit = r_s16(animation + 6);
    sound_table = r_u32(0x8008A408);
    w_u32(definition + 76, 0 - (uint32)limit);
    sound = 0;
    if (sound_table != 0 && r_s32(sound_table) >= 14)
        sound = sound_table + 108;
    parent = r_u32(definition + 72);
    animation = r_u32(object_offset + 104);
    alternate = r_u32(parent + 92);
    pitch = r_u16(sound + 4);
    if (animation == alternate)
    {
        value = r_u32(definition + 76);
        pitch += pitch >> 3;
        w_u32(definition + 76, value - 45);
    }
play_state_sound:
    p2_func_8005eb18(sound, 8192, 0, pitch, object_offset + 60, 0x4B000, 0xFFFFFFFF, 0, 0);
animation_done:
    if (r_s16(definition + 224) != 0)
    {
        value = r_u16(definition + 226);
        w_u16(definition + 226, value + 1);
        if ((sint16)value >= 5)
        {
            w_u16(definition + 226, 0);
            w_u32(object_offset + 28, r_u32(object_offset + 28) & 0xFFF7FFFF);
            w_u16(definition + 224, 0);
        }
    }
    owner = p2_func_800536f0(0);
    w_u32(distance_offset, 0x7FFFF000);
    parent = r_u32(object_offset + 132);
    if (parent != 0 && r_u32(parent + 40) != 0)
    {
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
        p2_sdk_8007ec64(input, position_offset);
        PopMatrix();
        for (index = 0; index < 3; ++index)
        {
            matrix = r_u32(r_u32(object_offset + 132) + 40);
            value = r_u32(position_offset + index * 4);
            w_u32(position_offset + index * 4, value + r_u32(matrix + 20 + index * 4));
        }
    }
    else
    {
        x = r_u32(object_offset + 16);
        y = r_u32(object_offset + 20);
        z = r_u32(object_offset + 24);
        w_u32(position_offset, x);
        w_u32(position_offset + 4, y);
        w_u32(position_offset + 8, z);
    }
    scene = r_u32(0x8008A114);
    x = r_u32(position_offset);
    x -= r_u32(scene + 16);
    y = r_u32(position_offset + 4);
    y -= r_u32(scene + 20);
    z = r_u32(position_offset + 8);
    z -= r_u32(scene + 24);
    x = (uint32)((sint32)x >> 12);
    y = (uint32)((sint32)y >> 12);
    z = (uint32)((sint32)z >> 12);
    squared = x * x + y * y + z * z;
    x = r_u32(scene + 16);
    y = r_u32(scene + 20);
    z = r_u32(scene + 24);
    w_u32(target_offset, x);
    w_u32(target_offset + 4, y);
    w_u32(target_offset + 8, z);
    w_u32(distance_offset, squared);
    w_u32(definition + 12, squared);
    if ((r_u32(owner + 228) & 0x100) != 0 && r_s16(resource + 644) == 0 && r_s32(distance_offset) <= 409599)
    {
        scene = r_u32(0x8008A114);
        y = r_u32(scene + 20);
        z = r_u32(position_offset + 4);
        if ((sint32)z >= (sint32)(y - 8192))
        {
            kind = r_s8(0x8008A138);
            if (kind == 0 || kind == 2)
                w_u32(distance_offset, 409600);
            if (r_u8(0x8008A138) < 2)
            {
                x = r_u32(target_offset);
                value = r_u32(position_offset);
                w_u32(target_offset, x + ((sint32)value < (sint32)x ? 409600 : (uint32)-409600));
                z = r_u32(target_offset + 8);
                value = r_u32(position_offset + 8);
                w_u32(target_offset + 8, z + ((sint32)value < (sint32)z ? 409600 : (uint32)-409600));
            }
        }
    }
    if ((r_u8(definition + 38) & 10) == 0)
        p2_func_8003557c(object_offset, position_offset, target_offset, distance_offset, target_offset);
    angle = (uint32)r_s16(definition + 176);
    if (angle != 0 && (limit = r_s16(resource + 642)) != 0)
    {
        value = angle << 2;
        frame = (sint16)value;
        if (frame > 0)
        {
            if (limit < frame)
                value = (uint32)limit;
        }
        else if (frame < -limit)
            value = 0 - (uint32)limit;
        w_u16(object_offset + 86, value & 0xFFF);
    }
    else
    {
        value = (uint32)r_s16(resource + 640);
        if (value != 0)
            w_u16(object_offset + 86, value & 0xFFF);
    }
    result = p2_func_800534ec();
finish_state:
    p2_guest_stack_pointer = saved_stack;
    return result;
}

uint32 p2_func_80053638(uint32 output_offset)
{
    uint32 flags, value, result;
    flags = (r_u32(0x800ABD68) >> 21) & 2;
    w_u8(output_offset, flags);
    value = (r_u32(0x800AC028) >> 20) & 15;
    flags |= value;
    w_u8(output_offset, flags);
    w_u32(output_offset + 8, (uint32)r_s8(0x800ABD76));
    w_u8(output_offset + 1, r_u8(0x800ABD75));
    w_u32(output_offset + 12, r_u8(0x800ABD74));
    w_u32(output_offset + 4, r_u16(0x800ABD78));
    value = r_u32(0x800ABD7C);
    if ((value & 0xFFFF) != 0)
    {
        value >>= 16;
        if (value == 0)
            value = 1;
        else
            value -= 9;
    }
    else
        value = 0;
    w_u8(output_offset + 16, value);
    result = r_u8(0x8008A344);
    w_u8(output_offset + 17, result);
    return result;
}

uint32 p2_func_80064ff4(uint32 position, uint32 text_offset, uint32 style, uint32 flags)
{
    uint32 index, metadata, group, atlas, tile, right, left, top, bottom, width, height;
    uint32 alignment, length, cursor, character, extent, step, result, glyph;
    uint32 first, second, value, address, shift;
    sint32 scale;
    index = r_u32(0x8008A25C) << 3;
    metadata = r_u32(0x800B170C + index);
    group = r_u32(0x800B1708 + index);
    tile = r_u8(metadata - 4);
    scale = (sint32)style >> 16;
    alignment = style & 0xFFFF;
    right = r_u8(group + 6);
    tile = r_u32(metadata + tile * 4 - 4);
    left = r_u8(group + 4);
    atlas = r_u32(0x800B7F54);
    top = r_u8(group + 5);
    tile = (tile << 3) & 0x1FF0;
    bottom = r_u8(group + 7);
    width = right - left + 1;
    atlas = r_u32(atlas + tile + 12);
    height = bottom - top + 1;
    if (alignment != 0)
    {
        character = r_u8(text_offset);
        length = 0;
        cursor = text_offset + 1;
        if (character != 0)
        {
            do
            {
                character = r_u8(cursor);
                ++length;
                ++cursor;
            } while (character != 0);
        }
        extent = width * length + ((length - 1) << 1);
        if (scale != 0)
            extent = (uint32)((sint32)((uint32)scale * extent + 2048) >> 12);
        if (alignment == 1)
            position -= extent >> 1;
        else
            position -= extent;
    }
    step = width + 2;
    if (scale != 0)
        step = (uint32)((sint32)((uint32)scale * step + 2048) >> 12);
    character = r_u8(text_offset);
    result = height >> 1;
    position -= result;
    cursor = text_offset + 1;
    while (character != 0)
    {
        result = character - 32 < 59;
        if (result != 0)
        {
            result = character << 3;
            if (character != 32)
            {
                glyph = atlas + result - 256;
                first = r_u32((glyph + 3) & 0xFFFFFFFC);
                first <<= 24 - ((glyph + 3) & 3) * 8;
                value = r_u32(glyph & 0xFFFFFFFC);
                shift = (glyph & 3) * 8;
                if (shift == 0)
                    first = value;
                else
                    first = (first & (0xFFFFFFFF << (32 - shift))) | (value >> shift);
                second = r_u32((glyph + 7) & 0xFFFFFFFC);
                second <<= 24 - ((glyph + 7) & 3) * 8;
                value = r_u32((glyph + 4) & 0xFFFFFFFC);
                if (shift == 0)
                    second = value;
                else
                    second = (second & (0xFFFFFFFF << (32 - shift))) | (value >> shift);
                address = group + 7;
                shift = (3 - (address & 3)) * 8;
                if (shift == 0)
                    w_u32(address & 0xFFFFFFFC, first);
                else
                {
                    value = r_u32(address & 0xFFFFFFFC);
                    w_u32(address & 0xFFFFFFFC, (value & (0xFFFFFFFF << (32 - shift))) | (first >> shift));
                }
                address = group + 4;
                shift = (address & 3) * 8;
                if (shift == 0)
                    w_u32(address & 0xFFFFFFFC, first);
                else
                {
                    value = r_u32(address & 0xFFFFFFFC);
                    w_u32(address & 0xFFFFFFFC, (value & (0xFFFFFFFF >> (32 - shift))) | (first << shift));
                }
                address = group + 11;
                shift = (3 - (address & 3)) * 8;
                if (shift == 0)
                    w_u32(address & 0xFFFFFFFC, second);
                else
                {
                    value = r_u32(address & 0xFFFFFFFC);
                    w_u32(address & 0xFFFFFFFC, (value & (0xFFFFFFFF << (32 - shift))) | (second >> shift));
                }
                address = group + 8;
                shift = (address & 3) * 8;
                if (shift == 0)
                    w_u32(address & 0xFFFFFFFC, second);
                else
                {
                    value = r_u32(address & 0xFFFFFFFC);
                    w_u32(address & 0xFFFFFFFC, (value & (0xFFFFFFFF >> (32 - shift))) | (second << shift));
                }
                result = p2_func_80064e74(position, 0, group, flags);
            }
            position += step;
        }
        character = r_u8(cursor);
        ++cursor;
    }
    return result;
}

P2PathSelection p2_func_8003adc0(uint32 segment_offset, uint32 frame_offset, uint32 direction, uint32 flags)
{
    P2PathSelection result;
    uint32 old_frame, current, selected, mask, definition, value, first_frame;
    old_frame = r_u32(frame_offset);
    /* Forced transitions use the same frame side as ordinary transitions */
    first_frame = (sint32)old_frame >= 0;
    if ((flags & 0x2000000) != 0)
    {
        mask = r_u16(segment_offset + 14);
        if ((mask & (((flags >> 6) & 0x10) | 8)) != 0)
            selected = r_u32(segment_offset + 20);
        else
            selected = r_u32(segment_offset + 16);
        if (selected != 0)
        {
            if ((first_frame & 0xFF) != 0)
                w_u32(frame_offset, 0);
            else
            {
                definition = r_u32(selected + 36);
                value = (uint32)r_s16(definition + 26) - 1;
                w_u32(frame_offset, value);
            }
        }
    }
    else
    {
        current = segment_offset;
        if (first_frame == 0)
        {
            if ((r_u16(segment_offset + 14) & 8) != 0)
                current = r_u32(segment_offset + 16);
            direction ^= 0x10;
        }
        mask = r_u16(current + 14);
        if ((mask & (direction | 8)) != 0)
            selected = r_u32(current + 20);
        else
            selected = r_u32(current + 16);
        if (selected != 0)
        {
            if (first_frame != 0)
                w_u32(frame_offset, 0);
            else
            {
                if ((r_u16(selected + 14) & 8) != 0)
                    selected = r_u32(selected + 16);
                definition = r_u32(selected + 36);
                value = (uint32)r_s16(definition + 26) - 1;
                w_u32(frame_offset, value);
            }
        }
    }
    if (selected != 0)
    {
        definition = r_u32(selected + 36);
        if ((r_u32(definition + 16) & 0x10000) != 0)
        {
            w_u32(frame_offset, old_frame);
            selected = 0;
        }
    }
    result.segment_offset = selected;
    result.first_frame = first_frame;
    return result;
}

uint32 p2_func_8001bd30(uint32 object_offset)
{
    uint32 saved_stack, matrix_local, world, input, minimum, maximum, low, high;
    uint32 parent, matrix, shape, output, count, offset, index, value, x, y, z, a, b, c;
    uint32 radius, half_y, half_z, ratio, spacing, source, destination, angle, cosine, sine;
    sint32 quotient;
    saved_stack = p2_guest_stack_pointer;
    p2_guest_stack_pointer -= 128;
    matrix_local = p2_guest_stack_pointer;
    world = matrix_local + 32;
    input = matrix_local + 48;
    minimum = matrix_local + 64;
    maximum = matrix_local + 80;
    low = matrix_local + 96;
    high = matrix_local + 112;
    if ((r_u32(object_offset + 28) & 0x2000) == 0 && (parent = r_u32(object_offset + 132)) != 0 && r_u32(parent + 40) != 0)
    {
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
        p2_sdk_8007ec64(input, world);
        PopMatrix();
        for (index = 0; index < 3; ++index)
        {
            matrix = r_u32(r_u32(object_offset + 132) + 40);
            value = r_u32(world + index * 4);
            w_u32(world + index * 4, value + r_u32(matrix + 20 + index * 4));
        }
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
    if ((r_u32(0x800ABD68) & 0x40000000) == 0)
    {
        w_u32(0x8008A574, 0xFFFFFFFF);
        w_u32(0x8008A564, 0xFFFFFFFF);
        w_u32(0x8008A554, 0xFFFFFFFF);
        w_u32(0x8008A544, 0xFFFFFFFF);
        shape = r_u32(object_offset + 8);
        count = 0;
        offset = 0;
        output = 0x8008A538;
        while (shape != 0)
        {
            if (r_u8(shape + 12) == 6)
            {
                x = r_u32(shape + 20);
                y = r_u32(shape + 24);
                z = r_u32(shape + 28);
                w_u32(output, x);
                w_u32(output + 4, y);
                w_u32(output + 8, z);
                radius = r_u32(shape + 32) & 0xFFFFFFF8;
                w_u32(0x8008A544 + offset, radius);
                w_u32(high + 8, radius);
                w_u32(high + 4, radius);
                w_u32(high, radius);
                x = r_u32(output) - radius;
                w_u32(low, x);
                y = r_u32(output + 4) - radius;
                w_u32(low + 4, y);
                z = r_u32(output + 8) - radius;
                w_u32(low + 8, z);
                a = r_u32(output) + radius;
                w_u32(high, a);
                b = r_u32(output + 4) + radius;
                w_u32(high + 4, b);
                c = r_u32(output + 8) + radius;
                w_u32(high + 8, c);
                if (count == 0)
                {
                    x = r_u32(low);
                    y = r_u32(low + 4);
                    z = r_u32(low + 8);
                    w_u32(minimum, x);
                    w_u32(minimum + 4, y);
                    w_u32(minimum + 8, z);
                    x = r_u32(high);
                    y = r_u32(high + 4);
                    z = r_u32(high + 8);
                    w_u32(maximum, x);
                    w_u32(maximum + 4, y);
                    w_u32(maximum + 8, z);
                }
                else
                {
                    if ((sint32)x < r_s32(minimum))
                        w_u32(minimum, x);
                    if (r_s32(maximum) < (sint32)a)
                        w_u32(maximum, a);
                    if ((sint32)y < r_s32(minimum + 4))
                        w_u32(minimum + 4, y);
                    if (r_s32(maximum + 4) < (sint32)b)
                        w_u32(maximum + 4, b);
                    if ((sint32)z < r_s32(minimum + 8))
                        w_u32(minimum + 8, z);
                    if (r_s32(maximum + 8) < (sint32)c)
                        w_u32(maximum + 8, c);
                }
                output += 16;
                offset += 16;
                ++count;
            }
            shape = r_u32(shape + 4);
        }
        // Keep untouched bounds when the original list has no type6 shape
        x = r_u32(minimum);
        a = r_u32(maximum);
        value = 0 - x;
        if ((sint32)value < (sint32)a)
            w_u32(minimum, 0 - a);
        else
            w_u32(maximum, value);
        z = r_u32(minimum + 8);
        c = r_u32(maximum + 8);
        value = 0 - z;
        if ((sint32)value < (sint32)c)
            w_u32(minimum + 8, 0 - c);
        else
            w_u32(maximum + 8, value);
        b = r_u32(maximum + 4);
        y = r_u32(minimum + 4);
        c = r_u32(maximum + 8);
        z = r_u32(minimum + 8);
        half_y = (uint32)((sint32)(b - y) >> 1);
        half_z = (uint32)((sint32)(c - z) >> 1);
        if (half_z == 0)
            xport_mips_break(7);
        quotient = 0x40000000 / (sint32)half_z;
        w_u32(world + 4, half_y);
        w_u32(world + 8, half_z);
        ratio = (uint32)(((sint64)(sint32)half_y * quotient) >> 18);
        w_u32(0x1F80030C, ratio);
        w_u32(0x8008A538, 0);
        w_u32(world, ratio);
        if (ratio - 3277 < 0x666)
        {
            radius = (sint32)ratio < 4096 ? half_z : half_y;
            w_u32(0x8008A53C, y + radius);
            w_u32(0x8008A544, radius);
            w_u32(0x8008A540, 0);
            w_u32(0x8008A574, 0xFFFFFFFF);
            w_u32(0x8008A564, 0xFFFFFFFF);
            w_u32(0x8008A554, 0xFFFFFFFF);
        }
        else if ((sint32)half_z < (sint32)half_y)
        {
            spacing = (uint32)((sint32)half_y / 4);
            w_u32(world + 4, spacing);
            if ((sint32)spacing < (sint32)half_z)
            {
                value = (b - half_z) - (y + half_z);
                w_u32(0x8008A544, half_z);
                spacing = (uint32)((sint32)value / 6);
                w_u32(world + 4, spacing);
            }
            else
                w_u32(0x8008A544, spacing);
            spacing = r_u32(world + 4);
            y = r_u32(minimum + 4);
            radius = r_u32(0x8008A544);
            w_u32(0x8008A540, 0);
            spacing <<= 1;
            w_u32(world + 4, spacing);
            w_u32(0x8008A53C, y + radius);
            for (offset = 16; offset < 64; offset += 16)
            {
                if (r_u32(0x8008A544 + offset) != 0xFFFFFFFF)
                {
                    source = 0x8008A538 + offset - 16;
                    destination = source + 16;
                    x = r_u32(source);
                    y = r_u32(source + 4);
                    z = r_u32(source + 8);
                    radius = r_u32(source + 12);
                    w_u32(destination, x);
                    w_u32(destination + 4, y);
                    w_u32(destination + 8, z);
                    w_u32(destination + 12, radius);
                    value = r_u32(destination + 4);
                    w_u32(destination + 4, value + r_u32(world + 4));
                }
            }
        }
        else
        {
            spacing = (uint32)((sint32)half_z / 4);
            w_u32(world + 8, spacing);
            if ((sint32)spacing < (sint32)half_y)
            {
                value = (c - half_y) - (z + half_y);
                w_u32(0x8008A544, half_y);
                spacing = (uint32)((sint32)value / 6);
                w_u32(world + 8, spacing);
            }
            else
                w_u32(0x8008A544, spacing);
            spacing = r_u32(world + 8);
            y = r_u32(minimum + 4);
            radius = r_u32(0x8008A544);
            z = r_u32(minimum + 8);
            spacing <<= 1;
            w_u32(world + 8, spacing);
            w_u32(0x8008A53C, y + radius);
            w_u32(0x8008A540, z + radius);
            for (offset = 16; offset < 64; offset += 16)
            {
                if (r_u32(0x8008A544 + offset) != 0xFFFFFFFF)
                {
                    source = 0x8008A538 + offset - 16;
                    destination = source + 16;
                    x = r_u32(source);
                    y = r_u32(source + 4);
                    z = r_u32(source + 8);
                    radius = r_u32(source + 12);
                    w_u32(destination, x);
                    w_u32(destination + 4, y);
                    w_u32(destination + 8, z);
                    w_u32(destination + 12, radius);
                    value = r_u32(destination + 8);
                    w_u32(destination + 8, value + r_u32(world + 8));
                }
            }
        }
        p2_func_8006637c((uint8 *)psx_addr(0x800AC07C, 64), (const uint8 *)psx_addr(0x8008A538, 64), 64);
        if ((r_u32(object_offset + 28) & 0x10000) == 0)
            w_u32(0x800ABD68, r_u32(0x800ABD68) | 0x40000000);
    }
    value = r_u32(object_offset + 28);
    angle = r_u16(object_offset + 150);
    if ((value & 0x10) != 0)
        angle += 2048;
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
    cosine = (uint32)(p2_func_8005dbb8((sint16)(1024 - angle)) >> 2);
    sine = (uint32)(p2_func_8005dbb8((sint16)angle) >> 2);
    w_u16(0x1F800014, cosine);
    w_u16(0x1F800020, 0 - sine);
    w_u16(0x1F800018, sine);
    w_u16(0x1F800024, cosine);
    matrix = p2_sdk_8007ea34(0x1F800014, matrix_local);
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
    for (offset = 0; offset < 64; offset += 16)
    {
        value = r_u32(0x800AC088 + offset);
        output = 0x8008A538 + offset;
        w_u32(output + 12, value);
        if (value != 0xFFFFFFFF)
        {
            p2_sdk_8007ec64(0x800AC07C + offset, output);
            value = r_u32(output);
            w_u32(output, value + r_u32(world));
            value = r_u32(output + 4);
            w_u32(output + 4, value + r_u32(world + 4));
            value = r_u32(output + 8);
            w_u32(output + 8, value + r_u32(world + 8));
        }
    }
    p2_guest_stack_pointer = saved_stack;
    return 0x8008A538;
}
