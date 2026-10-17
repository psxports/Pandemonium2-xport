#include "game.h"

static uint32 ApplyDirectionalImpulse(uint32 object_offset, uint32 character_offset, uint32 *move_flags, uint32 mode, uint32 vector_offset, sint32 strength, sint32 limit)
{
    FUNCTION_MARKER(0x80049928, "SLUS_005.78");
    uint32 value, x, y, z, parent, matrix, dot, rx, ry, rz, nx, ny, nz, speed, coefficient;
    uint32 flags, vertical, minimum, angle, saved_stack, vector, result;
    sint64 product;
    value = (uint32)AsinAngle(r_s32(vector_offset + 4), 4096);
    flags = character_controller.flags;
    if ((value & 0x3FF) - 64 < 0x380)
        flags |= 0x20000000;
    else
        flags &= 0xDFFFFFFF;
    character_controller.flags = flags;
    x = r_u32(object_offset + 92);
    y = r_u32(object_offset + 96);
    z = r_u32(object_offset + 100);
    if ((*move_flags & 8) == 0)
    {
        x = (uint32)(((sint64)(sint32)x * 3276) >> 12);
        y = (uint32)(((sint64)(sint32)y * 3276) >> 12);
        z = (uint32)(((sint64)(sint32)z * 3276) >> 12);
    }
    parent = r_u32(object_offset + 132);
    if (parent != 0)
    {
        matrix = r_u32(parent + 40);
        if (matrix != 0 && (r_u32(object_offset + 28) & 0x2000) == 0)
        {
            x += r_u32(matrix + 32);
            matrix = r_u32(r_u32(object_offset + 132) + 40);
            y += r_u32(matrix + 36);
            matrix = r_u32(r_u32(object_offset + 132) + 40);
            z += r_u32(matrix + 40);
        }
    }
    x -= r_u32(character_offset + 36);
    y -= r_u32(character_offset + 40);
    z -= r_u32(character_offset + 44);
    dot = (uint32)(((sint64)(sint32)x * r_s32(vector_offset)) >> 12);
    dot += (uint32)(((sint64)(sint32)y * r_s32(vector_offset + 4)) >> 12);
    dot += (uint32)(((sint64)(sint32)z * r_s32(vector_offset + 8)) >> 12);
    dot = 0 - (dot << 1);
    rx = (uint32)(((sint64)r_s32(vector_offset) * (sint32)dot) >> 12);
    ry = (uint32)(((sint64)r_s32(vector_offset + 4) * (sint32)dot) >> 12);
    rz = (uint32)(((sint64)r_s32(vector_offset + 8) * (sint32)dot) >> 12);
    rx += x;
    ry += y;
    rz += z;
    nx = (uint32)(((sint64)r_s32(vector_offset) * strength) >> 12);
    ny = (uint32)(((sint64)r_s32(vector_offset + 4) * strength) >> 12);
    nz = (uint32)(((sint64)r_s32(vector_offset + 8) * strength) >> 12);
    rx += nx;
    ry += ny;
    rz += nz;
    rx += r_u32(character_offset + 36);
    ry += r_u32(character_offset + 40);
    rz += r_u32(character_offset + 44);
    x = (uint32)((sint32)rx >> 6);
    y = (uint32)((sint32)ry >> 6);
    z = (uint32)((sint32)rz >> 6);
    value = x * x + y * y + z * z;
    if ((sint32)value < 0)
        speed = p2_sdk_8007e794(value >> 2) << 1;
    else
        speed = p2_sdk_8007e794(value);
    if (limit < (sint32)speed)
    {
        if (mode == 0xFFFFFFFF && ry + 12288 >= 0x6001)
        {
            value = (sint32)ry < 0 ? 0 - ry : ry;
            if (value == 0)
                xport_mips_break(7);
            coefficient = (uint32)(0x40000000 / (sint32)value);
            product = (sint64)12288 * (sint32)coefficient;
            coefficient = (uint32)((uint64)product >> 18);
            w_u32(0x1F80030C, coefficient);
            rx = (uint32)(((sint64)(sint32)rx * (sint32)coefficient) >> 12);
            ry = (uint32)(((sint64)(sint32)ry * (sint32)coefficient) >> 12);
            rz = (uint32)(((sint64)(sint32)rz * (sint32)coefficient) >> 12);
        }
        if (speed == 0)
            xport_mips_break(7);
        coefficient = (uint32)(0x40000000 / (sint32)speed);
        product = (sint64)limit * (sint32)coefficient;
        coefficient = (uint32)((uint64)product >> 18);
        w_u32(0x1F80030C, coefficient);
        rx = (uint32)(((sint64)(sint32)rx * (sint32)coefficient) >> 12);
        ry = (uint32)(((sint64)(sint32)ry * (sint32)coefficient) >> 12);
        rz = (uint32)(((sint64)(sint32)rz * (sint32)coefficient) >> 12);
    }
    *move_flags = *move_flags & 0xFFFFFFF7;
    value = (uint16)(character_controller.motion.form_flags >> 16) & 15;
    character_controller.timers.state_ticks = 0;
    character_controller.motion.jump_phase = 0;
    if (value != 5 && (character_controller.secondary_flags & 0x80) == 0)
        character_controller.motion.state_code = 3;
    character_controller.motion.vertical_speed = ry;
    character_controller.motion.jump_phase = (sint32)ry < 0 ? 0xFFFFFFFF : 1;
    ry = 0;
    if (((uint16)(character_controller.motion.form_flags >> 16) & 15) != 4 && ((r_u32(object_offset + 28) & 0x400) != 0 || (r_u32(object_offset + 228) & 4) != 0))
    {
        vertical = character_controller.motion.vertical_speed;
        minimum = r_u32(0x8008A188);
        if ((sint32)vertical < (sint32)minimum && (sint32)vertical > 0)
        {
            value = character_controller.motion.horizontal_speed;
            coefficient = 0x40000000 / (sint32)vertical;
            value = (uint32)(((sint64)(sint32)value * (sint32)minimum) >> 12);
            product = (sint64)(sint32)value * (sint32)coefficient;
            value = (uint32)((uint64)product >> 18);
            w_u32(0x1F80030C, value);
            character_controller.motion.horizontal_speed = value;
            character_controller.motion.vertical_speed = minimum;
        }
    }
    if (((uint16)(character_controller.motion.form_flags >> 16) & 15) == 5)
        rz = 0;
    x = (uint32)((sint32)rx >> 6);
    y = (uint32)((sint32)ry >> 6);
    z = (uint32)((sint32)rz >> 6);
    value = x * x + y * y + z * z;
    if ((sint32)value < 0)
        speed = p2_sdk_8007e794(value >> 2) << 1;
    else
        speed = p2_sdk_8007e794(value);
    character_controller.motion.horizontal_speed = speed;
    if (speed != 0)
    {
        angle = Atan2Angle((sint32)rz, (sint32)rx);
        value = (uint32)(sint32)r_s16(object_offset + 150);
        angle = (uint32)(sint32)(sint16)angle - value - 1024;
        if ((angle & 0xFFF) >= 2048)
        {
            character_controller.motion.horizontal_speed = 0 - (uint32)character_controller.motion.horizontal_speed;
            rx = 0 - rx;
            ry = 0 - ry;
            rz = 0 - rz;
        }
        flags = r_u32(object_offset + 28);
        if ((flags & 0x2000) != 0)
        {
            while (rx + 26754 > 0xD103 || ry + 26754 > 0xD103 || rz + 26754 > 0xD103)
            {
                rx = (uint32)((sint32)rx >> 1);
                ry = (uint32)((sint32)ry >> 1);
                rz = (uint32)((sint32)rz >> 1);
            }
            saved_stack = p2_guest_stack_pointer;
            p2_guest_stack_pointer -= 16;
            vector = p2_guest_stack_pointer;
            w_u32(vector, rx);
            w_u32(vector + 4, ry);
            w_u32(vector + 8, rz);
            VectorNormal((VECTOR *)psx_addr(vector, 12), (VECTOR *)psx_addr(vector, 12));
            x = r_u32(vector);
            y = r_u32(vector + 4);
            z = r_u32(vector + 8);
            character_controller.path_tangent.vx =  x;
            character_controller.path_tangent.vy =  y;
            character_controller.path_tangent.vz =  z;
            p2_guest_stack_pointer = saved_stack;
        }
        else
        {
            vertical = character_controller.motion.vertical_speed;
            value = (uint32)character_controller.movement.jump_speed;
            if ((sint32)value >= (sint32)vertical && (flags & 0x400) == 0)
            {
                character_controller.motion.jump_phase = 1;
                character_controller.motion.vertical_speed = value;
                character_controller.timers.state_ticks = 0;
                w_u16(object_offset + 130, 0xFFFF);
            }
        }
    }
    result = r_u32(object_offset + 28) | 0x400;
    w_u32(object_offset + 28, result);
    return result;
}

static uint32 ResolveSurfaceMotion(uint32 object_offset, uint32 character_offset, uint32 *move_flags)
{
    FUNCTION_MARKER(0x8001C8CC, "SLUS_005.78");
    uint32 saved_stack, record, matrix_local, normal, relative, tangent, owner, contact, source, index;
    uint32 x, y, z, a, b, c, d, e, f, g, h, i, pointer, parent, matrix, value, flags, up, down;
    uint32 dot, squared, magnitude, scaled_x, scaled_y, scaled_z, result;
    sint32 heading, angle;
    saved_stack = p2_guest_stack_pointer;
    p2_guest_stack_pointer -= 128;
    record = p2_guest_stack_pointer;
    matrix_local = record + 48;
    normal = record + 80;
    relative = record + 96;
    tangent = record + 112;
    owner = r_u32(character_offset + 116);
    contact = r_u32(object_offset + 112) & 0xFFFF8FFF;
    if (r_u8(owner + 12) == 1)
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
        if (r_s16(owner + 34) > 0)
            pointer = owner;
        else
            pointer = r_u32(owner + 40);
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
        source = r_u32(owner + 140) + (contact >> 16) * 48;
        for (index = 0; index < 48; index += 16)
        {
            x = r_u32(source + index);
            y = r_u32(source + index + 4);
            z = r_u32(source + index + 8);
            a = r_u32(source + index + 12);
            w_u32(record + index, x);
            w_u32(record + index + 4, y);
            w_u32(record + index + 8, z);
            w_u32(record + index + 12, a);
        }
    }
    w_u16(record + 2, 1);
    w_u16(record, contact);
    source = r_u32(owner + 36);
    a = r_u16(record + 16);
    b = r_u16(record + 4);
    c = r_u16(record + 10);
    d = r_u16(record + 6);
    e = r_u16(record + 12);
    f = r_u16(record + 18);
    g = r_u16(record + 8);
    h = r_u16(record + 14);
    i = r_u16(record + 20);
    w_u16(matrix_local + 4, a);
    w_u16(matrix_local + 2, c);
    source += (uint16)contact * 24;
    w_u16(matrix_local, b);
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
    source += 16;
    xport_gte_write_data(0, r_u32(source));
    xport_gte_write_data(1, r_u32(source + 4));
    xport_gte_execute(0x486012);
    w_u32(normal, xport_gte_read_data(25));
    w_u32(normal + 4, xport_gte_read_data(26));
    w_u32(normal + 8, xport_gte_read_data(27));
    PopMatrix();
    x = r_u32(object_offset + 92);
    a = r_u32(record + 36);
    x -= a;
    w_u32(relative, x);
    y = r_u32(object_offset + 96);
    b = r_u32(record + 40);
    y -= b;
    w_u32(relative + 4, y);
    z = r_u32(object_offset + 100);
    c = r_u32(record + 44);
    z -= c;
    w_u32(relative + 8, z);
    parent = r_u32(object_offset + 132);
    if (parent != 0 && (matrix = r_u32(parent + 40)) != 0 && (r_u32(object_offset + 28) & 0x2000) == 0)
    {
        w_u32(relative, x + r_u32(matrix + 32));
        matrix = r_u32(r_u32(object_offset + 132) + 40);
        w_u32(relative + 4, y + r_u32(matrix + 36));
        matrix = r_u32(r_u32(object_offset + 132) + 40);
        w_u32(relative + 8, z + r_u32(matrix + 40));
    }
    value = r_u32(object_offset + 112);
    heading = r_s8(object_offset + 126);
    value = (value & 0x7000) >> 12;
    if (heading == 1 && r_u32(0x800AC084) == 0)
    {
        up = value == 1;
        down = value == 4 || r_u32(0x8008A544 + value * 16) == 0xFFFFFFFF;
    }
    else
    {
        up = 1;
        down = 1;
    }
    if (r_u32(normal + 4) + 128 < 257 || (r_u32(object_offset + 28) & 0x400) == 0 || (r_u32(object_offset + 92) == 0 && r_u32(object_offset + 96) == 0 && r_u32(object_offset + 100) == 0) || (up == 0 && down == 0))
        goto reject_contact;
    value = r_u32(normal + 4);
    if ((sint32)value < 0 && (down == 0 || r_s32(relative + 4) < 0))
        goto reject_contact;
    if ((sint32)value > 0 && up == 0)
        goto reject_contact;
    if (r_s32(normal + 4) >= 0 && up != 0)
        goto slide_contact;
    if ((character_controller.flags & 0x10000) != 0)
        goto reverse_speed;
    value = character_controller.movement.jump_ticks;
    flags = character_controller.flags;
    character_controller.motion.jump_phase = -1;
    character_controller.motion.vertical_speed = 0;
    character_controller.timers.state_ticks = 0;
    character_controller.timers.jump_duration_ticks = value;
    character_controller.flags = flags | 0x08000000;
    value = r_u32(record + 40);
    if ((sint32)value < 0)
        character_controller.motion.vertical_speed = value;
    if ((character_controller.secondary_flags & 0x80) == 0)
        character_controller.motion.state_code = 6;
    flags = character_controller.flags;
    if ((flags & 0x40) == 0 && ((uint16)(character_controller.motion.form_flags >> 16) & 15) == 5 && (sint32)(r_u32(normal + 4) - 128) < -4095)
    {
        character_controller.flags = (flags & 0xFFFE7FBF) | 0x8000;
        goto save_contact;
    }
    result = (uint16)(character_controller.motion.form_flags >> 16) & 15;
    character_controller.motion.horizontal_speed = 0;
    if (result != 5)
    {
        result = *move_flags & 0xFFFFFFFE;
        *move_flags = result;
    }
    goto finish;
reject_contact:
    flags = character_controller.flags;
    if ((flags & 0x10000) != 0)
        goto reverse_speed;
    if ((flags & 0x40) != 0)
    {
        result = 0xFFFE0000;
        goto finish;
    }
    character_controller.flags = (flags & 0xFFFE7FBF) | 0x40;
    goto save_contact;
reverse_speed:
    x = r_u32(character_offset + 84);
    y = r_u32(character_offset + 88);
    result = 0 - x;
    w_u32(character_offset + 84, result);
    w_u32(character_offset + 88, 0 - y);
    goto finish;
slide_contact:
    value = character_controller.movement.jump_ticks;
    flags = character_controller.flags;
    character_controller.motion.jump_phase = -1;
    character_controller.motion.vertical_speed = 0;
    character_controller.timers.state_ticks = 0;
    character_controller.motion.horizontal_speed = 0;
    character_controller.timers.jump_duration_ticks = value;
    character_controller.flags = flags | 0x08000000;
    dot = 0;
    for (index = 0; index < 3; ++index)
    {
        value = r_u32(object_offset + 92 + index * 4);
        x = r_u32(normal + index * 4);
        dot += (uint32)(((sint64)(sint32)value * (sint32)x) >> 12);
    }
    x = r_u32(normal);
    scaled_x = (uint32)(((sint64)(sint32)x * (sint32)dot) >> 12);
    y = r_u32(normal + 4);
    w_u32(tangent, scaled_x);
    scaled_y = (uint32)(((sint64)(sint32)y * (sint32)dot) >> 12);
    z = r_u32(normal + 8);
    w_u32(tangent + 4, scaled_y);
    scaled_z = (uint32)(((sint64)(sint32)z * (sint32)dot) >> 12);
    w_u32(tangent + 8, scaled_z);
    x = r_u32(object_offset + 92) - scaled_x;
    w_u32(tangent, x);
    y = r_u32(object_offset + 96) - scaled_y;
    w_u32(tangent + 4, y);
    z = r_u32(object_offset + 100) - scaled_z;
    w_u32(tangent + 8, z);
    squared = (uint32)(((sint64)(sint32)x * (sint32)x) >> 12) + (uint32)(((sint64)(sint32)y * (sint32)y) >> 12) + (uint32)(((sint64)(sint32)z * (sint32)z) >> 12);
    if ((sint32)squared < 16)
    {
        a = r_u32(normal);
        x = r_u32(object_offset + 92);
        dot = (uint32)(((sint64)(sint32)a * (sint32)x) >> 12);
        b = r_u32(normal + 8);
        z = r_u32(object_offset + 100);
        dot += (uint32)(((sint64)(sint32)b * (sint32)z) >> 12);
        x = r_u32(normal);
        scaled_x = (uint32)(((sint64)(sint32)x * (sint32)dot) >> 12);
        y = r_u32(normal + 4);
        w_u32(tangent, scaled_x);
        scaled_y = (uint32)(((sint64)(sint32)y * (sint32)dot) >> 12);
        z = r_u32(normal + 8);
        w_u32(tangent + 4, scaled_y);
        scaled_z = (uint32)(((sint64)(sint32)z * (sint32)dot) >> 12);
        w_u32(tangent + 8, scaled_z);
        x = r_u32(object_offset + 92) - scaled_x;
        y = 0 - scaled_y;
        w_u32(tangent, x);
        w_u32(tangent + 4, y);
        z = r_u32(object_offset + 100) - scaled_z;
        w_u32(tangent + 8, z);
        squared = (uint32)(((sint64)(sint32)x * (sint32)x) >> 12) + (uint32)(((sint64)(sint32)y * (sint32)y) >> 12) + (uint32)(((sint64)(sint32)z * (sint32)z) >> 12);
    }
    if ((sint32)squared < 2)
    {
        w_u32(character_offset + 88, 0);
        w_u32(tangent, 6144);
        goto adjust_sign;
    }
    y = r_u32(tangent + 4);
    if ((sint32)y > 0)
    {
        x = r_u32(tangent);
        z = r_u32(tangent + 8);
        w_u32(tangent + 4, 0 - y);
        w_u32(tangent, 0 - x);
        w_u32(tangent + 8, 0 - z);
    }
    if (r_u32(tangent) + 26754 > 0xD103 || r_u32(tangent + 4) + 26754 > 0xD103 || r_u32(tangent + 8) + 26754 > 0xD103)
    {
        do
        {
            x = (uint32)(r_s32(tangent) >> 1);
            y = (uint32)(r_s32(tangent + 4) >> 1);
            z = (uint32)(r_s32(tangent + 8) >> 1);
            w_u32(tangent, x);
            w_u32(tangent + 4, y);
            w_u32(tangent + 8, z);
        } while (x + 26754 > 0xD103 || y + 26754 > 0xD103 || z + 26754 > 0xD103);
    }
    VectorNormal((VECTOR *)psx_addr(tangent, 12), (VECTOR *)psx_addr(tangent, 12));
    x = r_u32(tangent);
    scaled_x = (uint32)(((sint64)(sint32)x * 6144) >> 12);
    y = r_u32(tangent + 4);
    w_u32(tangent, scaled_x);
    scaled_y = (uint32)(((sint64)(sint32)y * 6144) >> 12);
    z = r_u32(tangent + 8);
    w_u32(tangent + 4, scaled_y);
    scaled_z = (uint32)(((sint64)(sint32)z * 6144) >> 12);
    w_u32(tangent + 8, scaled_z);
    w_u32(character_offset + 88, scaled_y);
    x = (uint32)(r_s32(tangent) >> 6);
    z = (uint32)(r_s32(tangent + 8) >> 6);
    squared = x * x + z * z;
    w_u32(tangent + 4, 0);
    magnitude = (sint32)squared >= 0 ? p2_sdk_8007e794(squared) : p2_sdk_8007e794(squared >> 2) << 1;
    z = r_u32(object_offset + 100);
    x = r_u32(object_offset + 92);
    heading = (sint16)Atan2Angle((sint32)z, (sint32)x);
    if (heading == 4096)
        heading = (r_u16(object_offset + 150) + 2048) & 0xFFF;
    z = r_u32(tangent + 8);
    x = r_u32(tangent);
    angle = (sint16)Atan2Angle((sint32)z, (sint32)x);
    value = ((uint32)angle - (uint32)(sint32)(sint16)heading + 1024) & 0xFFF;
    w_u32(tangent, value < 2048 ? magnitude : 0 - magnitude);
adjust_sign:
    if (r_s32(character_offset + 84) < 0)
        w_u32(tangent, 0 - r_u32(tangent));
    if ((character_controller.flags & 0x10000) != 0)
    {
        x = r_u32(character_offset + 84);
        y = r_u32(tangent);
        if ((sint32)(x ^ y) < 0)
        {
            w_u32(tangent, 0 - y);
            w_u32(character_offset + 88, 0 - r_u32(character_offset + 88));
        }
    }
    if ((character_controller.flags & 0x40) != 0)
    {
        x = r_u32(character_offset + 84);
        y = r_u32(tangent);
        if ((sint32)(x ^ y) >= 0)
        {
            w_u32(tangent, 0 - y);
            w_u32(character_offset + 88, 0 - r_u32(character_offset + 88));
        }
    }
    flags = character_controller.flags;
    value = r_u32(tangent);
    w_u32(character_offset + 84, value);
    character_controller.flags = (flags & 0xFFFE7FBF) | 0x10000;
    w_u32(object_offset + 28, r_u32(object_offset + 28) | 0x400);
save_contact:
    result = r_u32(object_offset + 112);
    owner = r_u32(character_offset + 116);
    result &= 0xFFFF8FFF;
    w_u32(character_offset + 108, result);
    w_u32(character_offset + 112, owner);
finish:
    p2_guest_stack_pointer = saved_stack;
    return result;
}

static uint32 ApplyContactImpulse(uint32 object_offset, uint32 character_offset, uint32 *move_flags)
{
    FUNCTION_MARKER(0x8004A218, "SLUS_005.78");
    uint32 saved_stack, vector, flags, auxiliary_flags, other, parent, data, parent_flag, special;
    uint32 sound, volume, pointer, x, y, z, relative_x, relative_y, relative_z, strength, value, mode, index;
    sint32 kind, direction, angle;
    saved_stack = p2_guest_stack_pointer;
    p2_guest_stack_pointer -= 16;
    vector = p2_guest_stack_pointer;
    flags = character_controller.flags;
    auxiliary_flags = character_controller.secondary_flags;
    other = r_u32(character_offset + 180);
    character_controller.flags = (flags | 0x40000) & 0xFF7FFFFF;
    character_controller.secondary_flags = auxiliary_flags & 0xFFFFFF7F;
    if (!other || r_u8(other + 12))
    {
        pointer = r_u32(character_offset + 116);
        strength = r_u32(pointer + 68);
        relative_x = r_u32(pointer + 36);
        relative_y = r_u32(pointer + 40);
        relative_z = r_u32(pointer + 44);
        direction = r_s16(character_offset + 678);
        goto resolve_direction_8004a218;
    }
    parent = r_u32(other + 132);
    parent_flag = 0;
    data = 0;
    if (parent)
    {
        data = r_u32(parent + 44);
        parent_flag = r_u16(data + 122) & 0x1000;
    }
    kind = r_s8(other + 125);
    special = 0;
    if (kind == 20 && (r_u32(other + 28) & 0x40))
        special = 1;
    if (!special && parent_flag && (r_u32(other + 28) & 0x40))
    {
        kind = r_s8(other + 125);
        special = 1;
    }
    if (special)
    {
        if (kind == 20 || (r_u16(data + 122) & 0x1000))
        {
            pointer = r_u32(r_u32(r_u32(other + 132) + 44) + 72);
            pointer = r_u32(pointer + 128);
            sound = 0;
            if (pointer && r_s32(pointer) >= 2)
                sound = pointer + 12;
            volume = sound_settings.effects_volume;
            sound_effects.Queue(sound, sound, 0, volume, 0, 3);
            character_controller.flags = character_controller.flags | 0x08000000;
        }
        if ((r_u32(object_offset + 28) & 0x400) && r_s8(other + 125) != 30)
        {
            w_u32(vector + 8, 0);
            w_u32(vector, 0);
            w_u32(vector + 4, 4096);
            w_u32(object_offset + 96, 0);
        }
        else
        {
            x = r_u32(object_offset + 60);
            y = r_u32(object_offset + 64);
            z = r_u32(object_offset + 68);
            w_u32(vector, x);
            w_u32(vector + 4, y);
            w_u32(vector + 8, z);
            pointer = r_u32(character_offset + 180);
            relative_x = r_u32(pointer + 60);
            relative_y = r_u32(pointer + 64);
            relative_z = r_u32(pointer + 68);
            relative_x = x - relative_x;
            relative_y = y - relative_y;
            relative_z = z - relative_z;
            if (r_s8(other + 125) == 30)
            {
                w_u32(vector, (sint32)relative_x < 0 ? (uint32)-1672 : 1672);
                w_u32(vector + 8, (sint32)relative_z < 0 ? (uint32)-1672 : 1672);
                w_u32(vector + 4, 3344);
                SpawnActionEffect(object_offset, character_offset, 1);
            }
            else
            {
                w_u32(vector, (sint32)relative_x < 0 ? (uint32)-2364 : 2364);
                w_u32(vector + 8, (sint32)relative_z < 0 ? (uint32)-2364 : 2364);
                w_u32(vector + 4, 2364);
            }
        }
        if (r_s8(other + 125) == 30)
            strength = 409;
        else
            strength = (r_u32(object_offset + 28) & 0x400) ? 16384 : 40960;
        mode = 0xFFFFFFFF;
        w_u32(character_offset + 44, 0);
        w_u32(character_offset + 40, 0);
        w_u32(character_offset + 36, 0);
        goto apply_vector_8004a218;
    }
    pointer = r_u32(character_offset + 180);
    relative_x = r_u32(pointer + 60);
    relative_y = r_u32(pointer + 64);
    relative_z = r_u32(pointer + 68);
    kind = r_s8(other + 125);
    if (kind == 19 && (r_u32(other + 28) & 0x40))
    {
        strength = 40960;
        direction = -1;
        goto resolve_direction_8004a218;
    }
    if (kind == 23)
    {
        pointer = r_u32(r_u32(r_u32(other + 132) + 44) + 72);
        pointer = r_u32(pointer + 128);
        sound = 0;
        if (pointer && r_s32(pointer) >= 2)
            sound = pointer + 12;
        volume = sound_settings.effects_volume;
        sound_effects.Queue(sound, sound, 0, volume, 0, 3);
    }
    strength = r_s8(other + 125) == 19 ? 12288 : 8192;
    direction = -1;
    if (r_s8(other + 126) == 2 && (r_u32(other + 28) & 0x40))
    {
        x = r_u32(object_offset + 60);
        y = r_u32(object_offset + 64);
        z = r_u32(object_offset + 68);
        w_u32(vector, x);
        w_u32(vector + 4, y);
        w_u32(vector + 8, z);
        value = Atan2Angle((sint32)(relative_z - z), (sint32)(relative_x - x));
        angle = r_s16(other + 148) - (sint16)value;
        w_u32(vector, (uint32)angle);
        if (angle < 0)
            w_u32(vector, 0 - (uint32)angle);
        if (r_s32(vector) < 1024)
            strength += r_u32(r_u32(r_u32(other + 132) + 44) + 16);
        relative_x += r_u32(other + 92);
        relative_y += r_u32(other + 96);
        relative_z += r_u32(other + 100);
    }
resolve_direction_8004a218:
    mode = (uint32)direction - 1;
    if (direction < 0)
    {
        pointer = r_u32(0x8008A114);
        x = r_u32(pointer + 16);
        relative_x = x - relative_x;
        y = r_u32(pointer + 20);
        relative_y = y - relative_y;
        z = r_u32(pointer + 24);
        relative_z = z - relative_z;
        value = Atan2Angle((sint32)relative_z, (sint32)relative_x);
        angle = r_s16(object_offset + 150);
        value = ((uint32)(sint32)(sint16)value - (uint32)angle - 1024) & 0xFFF;
        character_controller.motion.horizontal_speed = value < 2048 ? strength : 0 - strength;
        goto finish_8004a218;
    }
    index = (uint32)((sint32)mode >> 1) * 6;
    pointer = r_u32(character_offset + 116);
    value = (uint32)(sint32)r_s16(pointer + index + 16);
    w_u32(vector, (mode & 1) ? value : 0 - value);
    pointer = r_u32(character_offset + 116);
    value = (uint32)(sint32)r_s16(pointer + index + 18);
    w_u32(vector + 4, (mode & 1) ? value : 0 - value);
    pointer = r_u32(character_offset + 116);
    value = (uint32)(sint32)r_s16(pointer + index + 20);
    w_u32(vector + 8, (mode & 1) ? value : 0 - value);
apply_vector_8004a218:
    *move_flags = *move_flags | 8;
    ApplyDirectionalImpulse(object_offset, character_offset, move_flags, mode, vector, (sint32)strength, 36044);
finish_8004a218:
    value = (uint16)(character_controller.motion.form_flags >> 16);
    character_controller.timers.contact_impulse_ticks = 10;
    flags = *move_flags;
    value = flags & ((value & 15) == 3 ? 0xFFFFFFBE : 0xFFFFFF3E);
    *move_flags = value;
    p2_guest_stack_pointer = saved_stack;
    return value;
}

static uint32 UpdateMovementImpulse(uint32 object_offset, uint32 character_offset, uint32 *move_flags)
{
    FUNCTION_MARKER(0x8004D5C0, "SLUS_005.78");
    uint32 saved_stack = p2_guest_stack_pointer;
    uint32 vector, flags, value, mask, mode, snapshot, table, i, result;
    sint32 index = 0, strength, applied;
    p2_guest_stack_pointer -= 0x30;
    vector = p2_guest_stack_pointer + 0x20;
    if ((character_controller.flags & 4) != 0)
    {
        ResolveSurfaceMotion(object_offset, character_offset, move_flags);
        character_controller.flags = character_controller.flags & 0xFFFFFFFB;
    }
    flags = character_controller.flags;
    if ((flags & 0x18040) != 0)
    {
        if ((r_u8(r_u32(character_offset + 112) + 13) & 1) != 0)
            character_controller.ClearTransientFlags();
        else if ((flags & 0x10000) != 0)
        {
            if ((character_controller.secondary_flags & 0x80) == 0)
                character_controller.motion.state_code = 8;
            *move_flags = *move_flags & 0xFFFFFBFE;
        }
    }
    mode = character_controller.flags & 0x60000;
    if (mode == 0x40000)
    {
        value = character_controller.timers.contact_impulse_ticks;
        character_controller.timers.contact_impulse_ticks = value - 1;
        if (value == 0)
        {
            flags = character_controller.flags;
            character_controller.timers.contact_impulse_ticks = 0;
            character_controller.flags = flags & 0xFFFBFFFF;
            w_u32(object_offset + 28, r_u32(object_offset + 28) & 0xEFFFFFFF);
            character_controller.flags = character_controller.flags & 0xFFEFFFFF;
        }
        else
            goto suppress;
    }
    else if (mode == 0x60000 && (r_u32(object_offset + 28) & 0x400) != 0)
    {
    suppress:
        *move_flags = 0;
        mask = (*control_bindings.active_scheme)[1];
        gameplay_input.buttons = gameplay_input.buttons & ~mask;
        mask = (*control_bindings.active_scheme)[1];
        w_u32(0x1F800388, r_u32(0x1F800388) & ~mask);
    }
    if ((character_controller.flags & 0x80000) != 0)
        *move_flags = 0;
    if ((r_u32(object_offset + 228) & 0x101) != 0 && (character_controller.flags & 0x20000) != 0)
        character_controller.flags = character_controller.flags & 0xFFFDFFFF;
    flags = character_controller.flags;
    if ((flags & 0x810) == 0 && (flags & 0x60000) != 0x20000)
    {
        result = r_u32(object_offset + 28) & 0x400;
        if (result == 0)
        {
            result = r_u32(object_offset + 228) & 0x1220;
            if (result == 0)
            {
                result = (uint32)((CharacterModelRecord *)psx_addr(character_controller.model_offset, 240))->movement.rise_speed_limit;
                character_controller.motion.vertical_speed = 0;
                character_controller.movement.rise_speed_limit =  result;
            }
        }
        goto done;
    }
    if ((r_u32(object_offset + 28) & 0x400) == 0)
        *move_flags = *move_flags | 8;
    value = (uint16)(character_controller.motion.form_flags >> 16);
    character_controller.motion.jump_phase = 1;
    character_controller.timers.state_ticks = 0;
    if ((value & 0xF) != 4 || (r_u32(object_offset + 28) & 0x8000000) == 0)
        w_u16(object_offset + 130, 0xFFFF);
    if (((uint16)(character_controller.motion.form_flags >> 16) & 0xF) == 1)
    {
        flags = r_u32(object_offset + 28);
        if ((flags & 0x4000000) != 0)
        {
            w_u32(object_offset + 28, flags & 0xFBFFFFFF);
            if ((sint8)character_controller.motion.state_code == 9 && (character_controller.secondary_flags & 0x80) == 0)
                character_controller.motion.state_code = 2;
            for (i = 0; i < 4; ++i)
                character_controller.movement.speed_limits[i] =  character_controller.model_slots[15 + i];
            character_controller.motion.horizontal_speed = character_controller.model_slots[16];
            character_controller.model_slots[16] =  (uint32)-15;
        }
    }
    value = character_controller.movement.jump_ticks;
    flags = character_controller.flags;
    character_controller.timers.jump_duration_ticks = value;
    if ((flags & 0x60000) == 0x20000)
    {
        *move_flags = *move_flags | 8;
        character_controller.flags = character_controller.flags | 0x2000;
        result = ApplyContactImpulse(object_offset, character_offset, move_flags);
        goto done;
    }
    value = (uint32)((CharacterModelRecord *)psx_addr(character_controller.model_offset, 240))->movement.rise_speed_limit;
    flags = character_controller.flags;
    character_controller.timers.jump_duration_ticks = 7;
    character_controller.movement.rise_speed_limit =  value;
    if ((flags & 0x10) != 0)
    {
        index = r_s16(character_offset + 678) - 1;
        for (i = 0; i < 3; ++i)
        {
            table = r_u32(character_offset + 184) + (uint32)(index >> 1) * 6;
            value = (uint32)r_s16(table + 16 + i * 2);
            if (((uint32)index & 1) == 0)
                value = 0 - value;
            w_u32(vector + i * 4, value);
        }
        strength = r_s32(r_u32(character_offset + 184) + 68);
    }
    else
    {
        w_u32(vector + 8, 0);
        w_u32(vector, 0);
        w_u32(vector + 4, 4096);
        w_u32(object_offset + 96, 0);
        if ((character_controller.flags & 0x200000) == 0 && r_u32(0x8008A180) == 0 && r_s8(character_offset + 276) == 4)
        {
            p2_func_80044ffc(7, 3);
            SpawnActionEffect(object_offset, character_offset, 1);
        }
        strength = character_controller.movement.jump_speed;
        value = r_u32(0x8008A180);
        w_u32(character_offset + 44, 0);
        w_u32(character_offset + 40, 0);
        w_u32(character_offset + 36, 0);
        if (value != 0)
            strength = (sint32)(uint32)(((sint64)strength * (sint32)value) >> 12);
    }
    value = (sint16)character_controller.timers.jump_input_ticks < 3;
    w_u32(character_offset + 104, 0);
    if (value != 0)
    {
        character_controller.timers.jump_input_ticks = 6;
        *move_flags = *move_flags | 8;
    }
    else if ((*move_flags & 8) == 0)
    {
        character_controller.timers.jump_input_ticks = 3;
        *move_flags = *move_flags | 8;
        ApplyDirectionalImpulse(object_offset, character_offset, move_flags, (uint32)index, vector, strength, (sint32)((uint32)strength << 1));
        if ((character_controller.flags & 0x10) != 0 && character_controller.motion.vertical_speed < character_controller.movement.jump_speed)
            character_controller.motion.vertical_speed = (uint32)character_controller.movement.jump_speed;
        flags = (uint32)character_controller.movement.rise_speed_limit;
        value = character_controller.motion.horizontal_speed;
        mode = character_controller.motion.vertical_speed;
        snapshot = character_controller.motion.vertical_speed;
        w_u32(character_offset + 104, flags);
        w_u32(character_offset + 96, value);
        w_u32(character_offset + 100, mode);
        if ((sint32)flags < (sint32)snapshot)
            w_u32(character_offset + 104, snapshot);
    }
    snapshot = *move_flags;
    applied = strength;
    if ((*move_flags & 8) == 0)
        applied = strength >> 2;
    ApplyDirectionalImpulse(object_offset, character_offset, move_flags, snapshot, vector, applied, (sint32)((uint32)strength << 1));
    if ((character_controller.flags & 0x10) != 0 && character_controller.motion.vertical_speed < character_controller.movement.jump_speed)
        character_controller.motion.vertical_speed = (uint32)character_controller.movement.jump_speed;
    value = character_controller.motion.vertical_speed;
    if (character_controller.movement.rise_speed_limit < (sint32)value)
        character_controller.movement.rise_speed_limit =  value;
    if ((snapshot & 8) != 0)
    {
        if (character_controller.motion.jump_phase == 1)
            character_controller.motion.jump_phase = 2;
        character_controller.flags = character_controller.flags | 0x10000000;
    }
    *move_flags = *move_flags | 8;
    result = (character_controller.flags | 0x2000) & 0xF7FFFFFF;
    character_controller.flags = result;
done:
    p2_guest_stack_pointer = saved_stack;
    return result;
}

static uint32 UpdateJumpMotion(uint32 object_offset, uint32 character_offset, uint32 *move_flags)
{
    FUNCTION_MARKER(0x8004B8F8, "SLUS_005.78");
    uint32 flags, value, mask, x, y, z, squared, angle, result;
    sint32 old_frame, speed, length, cosine, duration, timer;
    if (((*move_flags & 8) == 0 && character_controller.motion.jump_phase < 2 && (character_controller.flags & 0x10000000) == 0) || (character_controller.flags & 0x10000) != 0 || (r_u32(object_offset + 228) & 0x200) != 0)
    {
        if ((r_u32(object_offset + 28) & 0x400) != 0)
        {
            if (character_controller.motion.jump_phase > 0)
            {
                if ((character_controller.flags & 0x10000000) == 0)
                {
                    character_controller.timers.state_ticks = 0;
                    character_controller.motion.jump_phase = 0;
                }
            }
            else
                character_controller.flags = character_controller.flags & 0xEFFFFFFF;
        }
        goto done;
    }
    flags = character_controller.flags;
    if ((flags & 0x2000) != 0 && (r_u32(object_offset + 28) & 0x400) == 0)
        goto done;
    if ((flags & 0x10000000) == 0)
        character_controller.timers.jump_duration_ticks = character_controller.movement.jump_ticks;
    *move_flags = *move_flags | 8;
    if (((uint16)(character_controller.motion.form_flags >> 16) & 0xF) == 5 && (character_controller.secondary_flags & 0x80) == 0)
        character_controller.motion.state_code = 3;
    if ((r_u32(object_offset + 28) & 0x400) == 0)
    {
        character_controller.flags = character_controller.flags | 0x2000;
        if ((r_u32(object_offset + 28) & 0x8000000) != 0)
        {
            *move_flags = *move_flags & 0xFFFFFBFE;
            w_u32(object_offset + 28, r_u32(object_offset + 28) & 0xF7FFFFFF);
        }
        if (character_controller.motion.vertical_speed <= 0)
        {
            old_frame = r_s16(object_offset + 130);
            value = (uint32)character_controller.movement.jump_speed;
            character_controller.motion.jump_phase = 1;
            character_controller.timers.state_ticks = 0;
            character_controller.motion.vertical_speed = value;
            w_u16(object_offset + 130, 0xFFFF);
            if ((character_controller.flags & 0x800000) != 0)
            {
                value = character_controller.motion.vertical_speed;
                speed = r_s32(r_u32(character_offset + 124) + 72);
                speed = (sint32)(uint32)(((sint64)(sint32)value * speed) >> 12);
                character_controller.motion.vertical_speed = (uint32)speed;
                if (speed <= 0)
                {
                    *move_flags = *move_flags & 0xFFFFFFF7;
                    value = character_controller.movement.jump_ticks;
                    flags = character_controller.flags;
                    character_controller.motion.vertical_speed = 0;
                    character_controller.motion.jump_phase = -1;
                    character_controller.timers.state_ticks = 0;
                    character_controller.timers.jump_duration_ticks = value;
                    character_controller.flags = flags | 0x8000000;
                    w_u16(object_offset + 130, (uint32)old_frame);
                }
            }
            if (character_controller.motion.vertical_speed == 0)
                goto done;
        }
        w_u32(character_offset + 8, r_u32(character_offset + 8) | 0x400000);
        flags = r_u32(object_offset + 28) | 0x400;
        w_u32(object_offset + 28, flags);
        if ((flags & 0x2000) == 0)
        {
            x = (uint32)(character_controller.path_velocity.vx >> 6);
            y = (uint32)(character_controller.path_velocity.vy >> 6);
            z = (uint32)(character_controller.path_velocity.vz >> 6);
            squared = x * x + y * y + z * z;
            if ((sint32)squared >= 0)
                length = (sint32)p2_sdk_8007e794(squared);
            else
                length = (sint32)(p2_sdk_8007e794(squared >> 2) << 1);
            angle = (0 - (uint32)AsinAngle(character_controller.path_velocity.vy, length)) & 0xFFF;
            if (character_controller.motion.horizontal_speed < 0)
                angle = (0 - angle) & 0xFFF;
            if ((sint16)angle >= 1025)
                angle += (uint32)((4096 - (sint16)angle) >> 1);
            else
            {
                angle <<= 1;
                if ((sint16)angle >= 1025)
                    angle = 1024;
            }
            if ((r_u32(object_offset + 228) & 4) != 0)
            {
                angle = 3584;
                character_controller.motion.horizontal_speed = (uint32)character_controller.movement.speed_limits[1];
            }
            if ((sint16)angle >= 2048)
            {
                cosine = SinQ14((sint16)(1024 - angle)) >> 2;
                speed = character_controller.motion.horizontal_speed;
                speed = (sint32)(uint32)(((sint64)cosine * speed) >> 12);
                if (((uint32)speed ^ (uint32)character_controller.motion.horizontal_speed) >= 0x80000000)
                    speed = 0;
                character_controller.motion.horizontal_speed = (uint32)speed;
            }
            else if ((sint16)angle >= 512)
            {
                character_controller.motion.horizontal_speed = 0;
                *move_flags = *move_flags & 0xFFFFFBFE;
            }
        }
        flags = r_u32(object_offset + 28);
        if ((flags & 0x8000000) != 0 && ((uint16)(character_controller.motion.form_flags >> 16) & 0xF) == 4)
            goto done;
        value = (uint32)(uint16)(character_controller.motion.form_flags >> 16) & 0xF;
        if (value - 6 < 2)
        {
            if (character_controller.motion.horizontal_speed != 0)
                value = (flags & 0x10) != 0 ? 34 : 33;
            else
                value = 3;
        }
        else
            value = 3;
        if ((character_controller.secondary_flags & 0x80) == 0)
            character_controller.motion.state_code = value;
        p2_func_80044ffc(0, 3);
        goto done;
    }
    duration = (sint16)character_controller.timers.jump_duration_ticks;
    if (character_controller.motion.jump_phase > 0)
    {
        value = character_controller.timers.state_ticks + 1;
        character_controller.timers.state_ticks = value;
        timer = (sint16)value;
        if (duration < timer || ((character_controller.flags & 0x8000000) != 0 && (duration >> 1) < timer))
        {
            flags = character_controller.flags;
            character_controller.timers.state_ticks = 0;
            character_controller.motion.jump_phase = 0;
            character_controller.flags = flags & 0xEFFFFFFF;
        }
    }
    if (((uint16)(character_controller.motion.form_flags >> 16) & 0xF) == 0)
    {
        mask = (*control_bindings.active_scheme)[1];
        if ((r_u32(0x1F800388) & mask) == 0 && (gameplay_input.buttons & mask) != 0 && (character_controller.flags & 0x8000000) == 0 && (uint32)character_controller.motion.vertical_speed + 4504 < 0x4198)
            goto reactivate;
    }
    if ((game_flow.mode_flags & 0x400) == 0)
        goto done;
    mask = (*control_bindings.active_scheme)[1];
    if ((r_u32(0x1F800388) & mask) != 0 || (gameplay_input.buttons & mask) == 0)
        goto done;
reactivate:
    value = (uint32)((CharacterModelRecord *)psx_addr(character_controller.model_offset, 240))->movement.rise_speed_limit;
    flags = character_controller.movement.jump_ticks;
    x = (uint32)character_controller.movement.jump_speed;
    character_controller.timers.jump_duration_ticks = flags;
    character_controller.timers.state_ticks = 0;
    character_controller.motion.vertical_speed = x;
    character_controller.motion.jump_phase = 1;
    character_controller.movement.rise_speed_limit =  value;
    w_u16(object_offset + 130, 0xFFFF);
    if ((character_controller.secondary_flags & 0x80) == 0)
        character_controller.motion.state_code = 9;
    p2_func_80044ffc(5, 3);
    character_controller.flags = (character_controller.flags | 0x8002000) & 0xEFFFFFFF;
done:
    result = *move_flags & 8;
    if (result == 0)
    {
        result = character_controller.flags & 0xFFFFDFFF;
        character_controller.flags = result;
    }
    return result;
}

static uint32 UpdateSpecialMovement(uint32 object_offset, uint32 character_offset, uint32 *move_flags)
{
    FUNCTION_MARKER(0x8004BFD0, "SLUS_005.78");
    uint32 flags, value, kind, i, result;
    sint32 timer;
    if ((r_u32(object_offset + 28) & 0x8000000) != 0 && (((uint32)(uint16)(character_controller.motion.form_flags >> 16) & 0xF) - 13 >= 2))
    {
        *move_flags = *move_flags & 0xFFFFFBFE;
        goto secondary;
    }
    if (((uint16)(character_controller.motion.form_flags >> 16) & 0xF) != 1)
    {
        w_u32(object_offset + 28, r_u32(object_offset + 28) & 0xFBFFFFFF);
        goto secondary;
    }
    if ((r_u32(object_offset + 28) & 0x4000000) != 0)
    {
        if (((*move_flags & 0x400) == 0 && (r_u8(object_offset + 13) & 1) == 0) || (character_controller.flags & 0x18040) != 0 || (sint8)character_controller.motion.state_code != 9)
            goto restore;
        value = character_controller.timers.special_move_ticks - 1;
        character_controller.timers.special_move_ticks = value;
        if ((value & 0xFFFF) != 0)
            goto secondary;
    restore:
        w_u32(object_offset + 28, r_u32(object_offset + 28) & 0xFBFFFFFF);
        if ((sint8)character_controller.motion.state_code == 9 && (character_controller.secondary_flags & 0x80) == 0)
            character_controller.motion.state_code = 2;
        for (i = 0; i < 4; ++i)
            character_controller.movement.speed_limits[i] =  character_controller.model_slots[15 + i];
        character_controller.motion.horizontal_speed = character_controller.model_slots[16];
        character_controller.model_slots[16] =  (uint32)-15;
    }
    else
    {
        timer = (sint32)character_controller.model_slots[16];
        if (timer > 0)
        {
            character_controller.model_slots[16] =  0;
            timer = (sint32)character_controller.model_slots[16];
        }
        value = 0;
        if (timer == 0)
            value = 1;
        else
        {
            timer = (sint32)((uint32)timer + 1);
            character_controller.model_slots[16] =  (uint32)timer;
            if (timer == 0)
                value = 1;
        }
        if (value == 0 || (*move_flags & 0x400) == 0 || (r_u32(object_offset + 28) & 0x400) != 0)
        {
            *move_flags = *move_flags & 0xFFFFFBFF;
            goto secondary;
        }
        w_u32(object_offset + 28, r_u32(object_offset + 28) | 0x4000000);
        if ((character_controller.secondary_flags & 0x80) == 0)
            character_controller.motion.state_code = 9;
        value = (uint32)character_controller.movement.speed_limits[0];
        character_controller.timers.special_move_ticks = 10;
        character_controller.model_slots[15] =  value;
        value = (uint32)character_controller.movement.speed_limits[1];
        character_controller.model_slots[16] =  value;
        value = (uint32)(((sint64)(sint32)value * 5120) >> 12);
        flags = (uint32)character_controller.movement.speed_limits[2];
        character_controller.movement.speed_limits[1] =  value;
        character_controller.movement.speed_limits[0] =  value;
        character_controller.model_slots[17] =  flags;
        value = (uint32)character_controller.movement.speed_limits[3];
        character_controller.model_slots[18] =  value;
        value = (uint32)(((sint64)(sint32)value * 5120) >> 12);
        character_controller.movement.speed_limits[3] =  value;
        character_controller.movement.speed_limits[2] =  value;
    }
secondary:
    if ((*move_flags & 0x80) != 0 && ((CharacterModelRecord *)psx_addr(character_controller.model_offset, 240))->animation_offsets[7] != 0)
    {
        flags = r_u32(object_offset + 28);
        if ((flags & 0x5000000) == 0 && (((uint16)(character_controller.motion.form_flags >> 16) & 0xF) == 3 || (flags & 0x400) == 0))
            w_u32(object_offset + 28, flags | 0x8000000);
        result = r_u32(object_offset + 28) & 0x8000000;
        if (result != 0)
        {
            if ((character_controller.secondary_flags & 0x80) == 0)
                character_controller.motion.state_code = 7;
            result = r_u32(character_offset + 500);
            if (result == 0)
                character_controller.motion.horizontal_speed = 0;
        }
    }
    else
    {
        result = r_u32(object_offset + 28) & 0x8010000;
        if (result == 0x8000000)
        {
            kind = (uint16)(character_controller.motion.form_flags >> 16) & 0xF;
            if (kind == 4)
            {
                result = character_controller.flags & 0x60000;
                if (result != 0)
                    return result;
            }
            if ((sint8)character_controller.motion.state_code == 7 && (character_controller.secondary_flags & 0x80) == 0)
                character_controller.motion.state_code = 0;
            result = r_u32(object_offset + 28) & 0xF7FFFFFF;
            w_u32(object_offset + 28, result);
        }
    }
    return result;
}

static uint32 UpdateMovementState(uint32 object_offset, uint32 character_offset, uint32 *move_flags)
{
    FUNCTION_MARKER(0x8004C3B0, "SLUS_005.78");
    uint32 flags, value, kind, animation, i, result, decelerate;
    sint32 speed, state;
    if ((character_controller.flags & 0x10000) != 0)
        goto input_actions;
    if ((r_u32(object_offset + 228) & 0x30000) != 0)
    {
        value = r_u32(character_offset + 288) + 1;
        kind = r_u8(0x8008A1BE);
        w_u32(character_offset + 288, value);
        if (value >= kind)
            w_u32(object_offset + 228, r_u32(object_offset + 228) & 0xFFFCFFFF);
    }
    decelerate = 0;
    if ((*move_flags & 0x401) == 0)
        decelerate = (r_u32(object_offset + 228) & 0x30000) == 0;
    p2_func_8004922c(object_offset, character_offset, decelerate);
    if ((*move_flags & 0x10000) != 0)
    {
        if ((character_controller.secondary_flags & 0x80) == 0)
            character_controller.motion.state_code = 6;
    }
    else if ((*move_flags & 0x8000) != 0 && (r_u32(object_offset + 28) & 0x400) == 0)
    {
        if ((*move_flags & 0x800) != 0)
            animation = 17;
        else if ((r_u32(character_offset + 500) != 0 && r_s32(object_offset + 96) < 0) || character_controller.motion.horizontal_speed <= 0)
            animation = 16;
        else
            animation = 15;
        if ((character_controller.secondary_flags & 0x80) == 0)
            character_controller.motion.state_code = animation;
    }
    else if ((r_u32(object_offset + 228) & 1) != 0)
    {
        if (character_controller.motion.horizontal_speed != 0)
            animation = (gameplay_input.buttons & 0x1000) != 0 ? 10 : 12;
        else
            animation = 11;
        if ((character_controller.secondary_flags & 0x80) == 0)
            character_controller.motion.state_code = animation;
        if ((*move_flags & 0x800) != 0)
            character_controller.motion.horizontal_speed = (uint32)(((sint64)character_controller.movement.speed_limits[1] * ((CharacterModelRecord *)psx_addr(character_controller.model_offset, 240))->motion_scale[0]) >> 12);
        else if ((*move_flags & 0x1000) != 0)
            character_controller.motion.horizontal_speed = (uint32)(((sint64)character_controller.movement.speed_limits[1] * ((CharacterModelRecord *)psx_addr(character_controller.model_offset, 240))->motion_scale[1]) >> 12);
        else
            character_controller.motion.horizontal_speed = 0;
    }
    flags = r_u32(object_offset + 28);
    if ((flags & 0x9000400) != 0 || (r_u32(object_offset + 228) & 0x1133) != 0)
        goto input_actions;
    speed = character_controller.motion.horizontal_speed;
    if (speed != 0)
    {
        if ((*move_flags & 0x401) != 0)
        {
            kind = (uint16)(character_controller.motion.form_flags >> 16) & 0xF;
            if (kind == 1 && (flags & 0x4000000) != 0)
                goto input_actions;
            goto moving_animation;
        }
        animation = character_controller.motion.state_code;
        if (animation - 4 < 2)
            goto input_actions;
        if (r_u32(character_offset + 500) != 0 || (character_controller.flags & 8) != 0)
        {
            animation = 8;
            goto set_animation;
        }
        if (speed > 0)
        {
            kind = (uint16)(character_controller.motion.form_flags >> 16) & 0xF;
            goto moving_animation;
        }
        goto idle_animation;
    }
    if (((uint16)(character_controller.motion.form_flags >> 16) & 0xF) == 1 && (flags & 0x4000000) != 0)
    {
        w_u32(object_offset + 28, flags & 0xFBFFFFFF);
        if ((sint8)character_controller.motion.state_code == 9 && (character_controller.secondary_flags & 0x80) == 0)
            character_controller.motion.state_code = 2;
        for (i = 0; i < 4; ++i)
            character_controller.movement.speed_limits[i] =  character_controller.model_slots[15 + i];
        character_controller.motion.horizontal_speed = character_controller.model_slots[16];
        character_controller.model_slots[16] =  (uint32)-15;
        character_controller.motion.horizontal_speed = 0;
    }
    animation = character_controller.motion.state_code;
    if (animation - 4 < 2)
        goto input_actions;
    if ((character_controller.flags & 8) != 0)
    {
        animation = 8;
        goto set_animation;
    }
    if ((*move_flags & 1) != 0)
    {
        kind = (uint16)(character_controller.motion.form_flags >> 16) & 0xF;
        flags = r_u32(object_offset + 28);
        goto moving_animation;
    }
idle_animation:
    if ((uint32)(sint32)(sint8)animation - 1 < 34)
    {
        animation = 0;
        goto set_animation;
    }
    goto input_actions;
moving_animation:
    if (kind - 6 < 2)
        animation = (flags & 0x10) != 0 ? 32 : 31;
    else
        animation = 2;
set_animation:
    if ((character_controller.secondary_flags & 0x80) == 0)
        character_controller.motion.state_code = animation;
input_actions:
    if ((*move_flags & 0x100) != 0)
    {
        state = (sint8)character_controller.motion.state_code;
        character_controller.motion.jump_phase = 1;
        character_controller.timers.state_ticks = 0;
        if (state != 9 && state != 5)
        {
            if ((character_controller.secondary_flags & 0x80) == 0)
                character_controller.motion.state_code = 9;
            w_u32(object_offset + 28, r_u32(object_offset + 28) | 0x400);
            if ((character_controller.flags & 0x10000) != 0)
            {
                value = r_u32(character_offset + 88);
                character_controller.motion.horizontal_speed = 0;
                character_controller.motion.vertical_speed = value;
                if ((sint32)value < 0)
                    character_controller.motion.vertical_speed = 0 - value;
                character_controller.ClearTransientFlags();
            }
        }
    }
    result = 16;
    if ((r_u32(object_offset + 228) & 1) != 0)
    {
        if (r_s8(0x8008A490) == 16)
            result = r_u16(character_offset + 266);
        else
            result = (r_u16(character_offset + 266) + 2048) & 0xFFF;
        w_u16(object_offset + 148, result);
        w_u16(object_offset + 150, result);
    }
    return result;
}

static uint32 UpdateActionInput(uint32 object_offset, uint32 character_offset, uint32 *move_flags)
{
    FUNCTION_MARKER(0x8004CA00, "SLUS_005.78");
    uint32 kind, flags, result;
    if ((uint16)character_controller.weapon_state != 0)
        goto action;
    kind = (uint16)(character_controller.motion.form_flags >> 16) & 0xF;
    if (kind == 5 || kind - 10 < 2)
        goto action;
    if (kind == 1 && player_interaction.flags == 0 && (r_u32(object_offset + 28) & 0x8000000) == 0 && (sint8)character_controller.motion.state_code != 4)
        goto action;
    result = (uint16)(character_controller.motion.form_flags >> 16) & 0xF;
    if (result != 4)
        return result;
action:
    result = *move_flags & 0x40;
    if (result == 0)
        return result;
    if ((sint16)character_controller.timers.action_cooldown_ticks != 0)
        return 0x5000000;
    flags = r_u32(object_offset + 28);
    result = flags & 0x5000000;
    if (result != 0)
        return result;
    result = r_u8(object_offset + 13) & 1;
    if (result != 0)
        return result;
    if (((CharacterModelRecord *)psx_addr(character_controller.model_offset, 240))->animation_offsets[5] == 0)
        return 0x1000000;
    flags |= 0x1000000;
    w_u32(object_offset + 28, flags);
    kind = (uint16)(character_controller.motion.form_flags >> 16) & 0xF;
    if (kind == 1)
    {
        if ((character_controller.secondary_flags & 0x80) == 0)
            character_controller.motion.state_code = 5;
        w_u16(object_offset + 130, 0xFFFF);
        w_u32(0x8008A4CC, 5);
        return 5;
    }
    if (kind != 4 && kind != 5 && kind - 10 >= 2)
    {
        if ((flags & 0x8000400) != 0 || (sint8)character_controller.motion.state_code == 2)
            return 2;
    }
    if ((character_controller.secondary_flags & 0x80) == 0)
        character_controller.motion.state_code = 5;
    w_u16(object_offset + 130, 0xFFFF);
    return 0xFFFFFFFF;
}

static uint32 IntegrateCharacterMovement(uint32 object_offset, uint32 character_offset, uint32 input_flags)
{
    FUNCTION_MARKER(0x8004CBA4, "SLUS_005.78");
    uint32 saved_stack = p2_guest_stack_pointer;
    uint32 horizontal, vertical, position, floor, original, flags, value, kind, state, matrix, node, i, grounded, result;
    sint32 lower_limit, acceleration, upper_limit, rise, speed, direction, frame;
    p2_guest_stack_pointer -= 0x50;
    horizontal = p2_guest_stack_pointer + 0x18;
    vertical = p2_guest_stack_pointer + 0x1C;
    position = p2_guest_stack_pointer + 0x20;
    floor = p2_guest_stack_pointer + 0x30;
    original = p2_guest_stack_pointer + 0x40;
    if ((input_flags & 2) != 0 && (r_u32(object_offset + 28) & 0x9000000) == 0 && (((uint32)(uint16)(character_controller.motion.form_flags >> 16) & 0xF) - 13 >= 2))
        goto prepare;
    if ((input_flags & 0x20) != 0 && (r_u32(object_offset + 228) & 2) != 0)
    {
    prepare:
        ReverseScenePath(object_offset, character_offset);
    }
    flags = character_controller.flags;
    if ((flags & 0x10000) != 0)
    {
        p2_func_8001d698(object_offset, character_offset);
        goto finish_motion;
    }
    if ((r_u32(object_offset + 28) & 0x400) == 0 && character_controller.motion.horizontal_speed == 0 && (flags & 0x99048) == 0 && r_u32(character_offset + 552) == 0 && (r_u32(object_offset + 228) & 0x122) == 0)
    {
        w_u32(object_offset + 100, 0);
        w_u32(object_offset + 96, 0);
        w_u32(object_offset + 92, 0);
        goto finish_motion;
    }
    flags = r_u32(object_offset + 28);
    value = character_controller.motion.horizontal_speed;
    w_u32(horizontal, value);
    if ((flags & 0x400) == 0 && (r_u32(object_offset + 228) & 0x122) == 0)
    {
        w_u32(vertical, 0);
        goto adjust_horizontal;
    }
    lower_limit = character_controller.movement.fall_speed_limit;
    acceleration = character_controller.movement.fall_acceleration;
    if ((input_flags & 0x20000) != 0 || (r_u32(object_offset + 228) & 0x200000) != 0)
    {
        p2_func_80058d08(object_offset, character_offset, input_flags, 0, horizontal, vertical);
        goto integrate_vertical;
    }
    if ((input_flags & 0x10000) != 0 && r_s8(0x8008A210) == 0)
    {
        value = (uint32)(((sint64)r_s32(0x8008A214) * character_controller.movement.speed_limits[1]) >> 12);
        character_controller.motion.vertical_speed = value;
        flags = character_controller.motion.vertical_speed;
        w_u32(vertical, value);
        value = 0 - flags;
        character_controller.motion.vertical_speed = value;
        w_u32(vertical, value);
        goto integrate_vertical;
    }
    if ((input_flags & 0x2000) != 0 || (r_u32(object_offset + 228) & 4) != 0)
    {
        p2_func_80058f50(object_offset, character_offset, input_flags, (input_flags & 0x2000) != 0 ? 8 : 0, horizontal, vertical);
        goto integrate_vertical;
    }
    if ((input_flags & 0x200) != 0)
    {
        state = (uint32)(sint8)character_controller.motion.state_code;
        character_controller.motion.jump_phase = -1;
        character_controller.timers.state_ticks = 0;
        if (state != 5 && (character_controller.secondary_flags & 0x80) == 0)
            character_controller.motion.state_code = 7;
        goto integrate_vertical;
    }
    kind = (uint16)(character_controller.motion.form_flags >> 16) & 0xF;
    if ((kind != 5 && kind - 10 >= 2) || (input_flags & 0x100) != 0)
        goto integrate_vertical;
    if (character_controller.motion.vertical_speed < lower_limit)
    {
        character_controller.motion.jump_phase = 1;
        input_flags |= 0x100;
    }
    state = (uint32)(sint8)character_controller.motion.state_code;
    if (state != 3 || (input_flags & 8) != 0)
        goto check_animation;
    if ((character_controller.flags & 0x60000) != 0)
    {
        state = (uint32)(sint8)character_controller.motion.state_code;
    check_animation:
        if (state == 5 || state == 3)
            goto integrate_vertical;
    }
    flags = character_controller.secondary_flags;
    if ((flags & 0x400) == 0 && (flags & 0x80) == 0)
        character_controller.motion.state_code = (input_flags & 3) != 0 ? 9 : 6;
integrate_vertical:
    if ((character_controller.flags & 0x800000) != 0)
    {
        node = r_u32(character_offset + 124);
        upper_limit = (sint32)(((sint64)character_controller.movement.rise_speed_limit * r_s32(node + 72)) >> 12);
        rise = (sint32)(((sint64)character_controller.movement.rise_acceleration * r_s32(node + 72)) >> 12);
    }
    else
    {
        upper_limit = character_controller.movement.rise_speed_limit;
        rise = character_controller.movement.rise_acceleration;
    }
    flags = r_u32(object_offset + 228);
    if ((flags & 2) != 0 && (flags & 4) == 0)
    {
        direction = character_controller.motion.jump_phase;
        if (direction > 0)
        {
            speed = character_controller.motion.vertical_speed;
            if (speed < upper_limit)
                character_controller.motion.vertical_speed = (uint32)speed;
            direction = character_controller.motion.jump_phase;
        }
        if (direction < 0)
        {
            speed = character_controller.motion.vertical_speed;
            if (lower_limit < speed)
            {
                character_controller.motion.vertical_speed = (uint32)speed;
                if (speed < lower_limit)
                    goto clamp_lower;
            }
        }
        goto save_vertical;
    }
    direction = character_controller.motion.jump_phase;
    if (direction > 0)
    {
        speed = character_controller.motion.vertical_speed;
        if (speed < upper_limit)
        {
            character_controller.motion.vertical_speed = (uint32)speed;
            if (((flags & 2) != 0 && (input_flags & 0x4000) != 0) || ((flags & 2) == 0 && (input_flags & 0x108) != 0))
                character_controller.motion.vertical_speed = (uint32)speed + (uint32)rise;
            speed = character_controller.motion.vertical_speed;
            character_controller.motion.vertical_speed = (uint32)speed;
            if (speed >= upper_limit)
            {
                character_controller.motion.vertical_speed = (uint32)upper_limit;
                w_u32(vertical, (uint32)upper_limit);
                goto adjust_horizontal;
            }
        }
        direction = character_controller.motion.jump_phase;
    }
    if (direction < 0)
    {
        speed = character_controller.motion.vertical_speed;
        if (lower_limit < speed)
        {
            speed = (sint32)((uint32)speed + (uint32)acceleration);
            character_controller.motion.vertical_speed = (uint32)speed;
            if (speed < lower_limit)
            {
            clamp_lower:
                character_controller.motion.vertical_speed = (uint32)lower_limit;
                w_u32(vertical, (uint32)lower_limit);
                goto adjust_horizontal;
            }
        }
    }
save_vertical:
    w_u32(vertical, (uint32)character_controller.motion.vertical_speed);
adjust_horizontal:
    if ((character_controller.flags & 0x80008) != 0)
        p2_func_80049730(object_offset, character_offset, horizontal, vertical);
    else
    {
        value = r_u32(character_offset + 552);
        if (value != 0)
        {
            if ((r_u32(object_offset + 28) & 0x10) != 0)
                value = (uint32)character_controller.motion.horizontal_speed + value;
            else
                value = (uint32)character_controller.motion.horizontal_speed - value;
            character_controller.motion.horizontal_speed = value;
            value = character_controller.motion.horizontal_speed;
            w_u32(character_offset + 552, 0);
            w_u32(horizontal, value);
        }
    }
    speed = r_s32(horizontal);
    if (speed > 15360)
        w_u32(horizontal, 15360);
    else if (speed < -15360)
        w_u32(horizontal, (uint32)-15360);
    value = r_u32(horizontal);
    flags = r_u32(vertical);
    w_u32(character_offset + 84, value);
    w_u32(character_offset + 88, flags);
    flags = r_u32(object_offset + 28);
    grounded = (flags >> 10) & 1;
    if (grounded != 0)
        character_controller.flags = character_controller.flags & 0xFFFFEFFF;
    else if ((character_controller.flags & 0x1000) != 0)
        w_u32(object_offset + 28, flags | 0x400);
    p2_func_800455dc(object_offset, character_offset, r_s32(horizontal), r_s32(vertical));
    flags = character_controller.flags;
    if ((flags & 0x40) != 0)
        p2_func_8001db60(object_offset, character_offset, grounded);
    else if ((flags & 0x8000) != 0)
        p2_func_8001eb44(object_offset, character_offset);
    if (grounded != 0)
        goto finish_motion;
    if ((r_u32(r_u32(object_offset + 132) + 16) & 0x11) != 0)
        w_u32(object_offset + 28, r_u32(object_offset + 28) | 0x400);
    if ((r_u32(object_offset + 28) & 0x400) == 0)
        goto finish;
    if (r_s32(horizontal) <= 0 || (character_controller.flags & 0x1000) != 0)
        goto stop_vertical;
    node = r_u32(r_u32(character_offset + 688) + 36);
    if ((r_u32(node + 16) & 0x11) != 0)
    {
    stop_vertical:
        value = character_controller.movement.jump_ticks;
        flags = character_controller.flags;
        kind = (uint16)(character_controller.motion.form_flags >> 16) & 0xF;
        character_controller.motion.jump_phase = -1;
        character_controller.motion.vertical_speed = 0;
        character_controller.timers.state_ticks = 0;
        character_controller.timers.jump_duration_ticks = value;
        character_controller.flags = flags | 0x8000000;
        if ((kind != 1 || (r_u32(object_offset + 28) & 0x4000000) == 0) && (r_u32(object_offset + 228) & 0x100) == 0 && (character_controller.secondary_flags & 0x80) == 0)
            character_controller.motion.state_code = 6;
        goto finish_motion;
    }
    w_u32(horizontal, (uint32)r_s16(node + 26));
    node = r_u32(object_offset + 132);
    if (node != 0 && r_u32(node + 40) != 0)
    {
        value = r_u32(object_offset + 16);
        flags = r_u32(object_offset + 20);
        kind = r_u32(object_offset + 24);
        w_u32(original, value);
        w_u32(original + 4, flags);
        w_u32(original + 8, kind);
        PushMatrix();
        matrix = r_u32(r_u32(object_offset + 132) + 40);
        for (i = 0; i < 5; ++i)
            xport_gte_write_control(i, r_u32(matrix + i * 4));
        p2_sdk_8007ec64(original, position);
        PopMatrix();
        for (i = 0; i < 3; ++i)
        {
            matrix = r_u32(r_u32(object_offset + 132) + 40);
            value = r_u32(position + i * 4);
            value += r_u32(matrix + 20 + i * 4);
            w_u32(position + i * 4, value);
        }
    }
    else
    {
        value = r_u32(object_offset + 16);
        flags = r_u32(object_offset + 20);
        kind = r_u32(object_offset + 24);
        w_u32(position, value);
        w_u32(position + 4, flags);
        w_u32(position + 8, kind);
    }
    for (i = 0; i < 3; ++i)
    {
        value = r_u32(position + i * 4);
        value += r_u32(object_offset + 92 + i * 4);
        w_u32(position + i * 4, value);
    }
    node = r_u32(r_u32(character_offset + 688) + 36);
    frame = r_s32(horizontal);
    direction = ProjectPathFrame(node, frame, position, floor, r_u32(object_offset + 28) & 0x10, 6144);
    if (direction == r_s32(horizontal))
    {
        w_u32(object_offset + 28, r_u32(object_offset + 28) & 0xFFFFFBFF);
        character_controller.flags = character_controller.flags | 0x1000;
    }
finish_motion:
    if ((r_u32(object_offset + 28) & 0x400) != 0 && character_controller.motion.jump_phase == 0)
    {
        flags = character_controller.flags;
        character_controller.motion.jump_phase = -1;
        character_controller.flags = flags & 0xEFFFFFFF;
    }
finish:
    node = r_u32(0x8008A114);
    w_u32(character_offset + 180, 0);
    node = r_u32(node + 136);
    if ((r_u32(r_u32(node + 36) + 16) & 0x100011) != 0)
        w_u32(character_offset + 500, 0);
    else
        w_u32(character_offset + 500, r_u32(node + 28));
    result = character_controller.flags & 0xF855B041;
    character_controller.flags = result;
    p2_guest_stack_pointer = saved_stack;
    return result;
}

static uint32 BuildMovementFlags(uint32 object_offset, uint32 character_offset)
{
    FUNCTION_MARKER(0x8004AA68, "SLUS_005.78");
    uint32 saved_stack = p2_guest_stack_pointer;
    uint32 local, result = 0, previous, flags, value, mask, matrix, filtered, x, y, z;
    sint32 timer, state, limit, response;
    p2_guest_stack_pointer -= 0x48;
    local = p2_guest_stack_pointer + 0x10;
    if ((character_controller.flags & 0x4000000) != 0)
    {
        w_u8(object_offset + 13, r_u8(object_offset + 13) & 0xFE);
        character_controller.flags = character_controller.flags & 0xFBFFFFFF;
        gameplay_input.buttons = 0;
        goto finish;
    }
    if ((sint16)character_controller.timers.transition_ticks != 0 || (r_u8(object_offset + 13) & 1) != 0)
        goto finish;
    w_u32(local, gameplay_input.buttons);
    previous = r_u32(0x1F800388);
    timer = r_s8(character_offset + 276);
    if (timer > 0)
        w_u8(character_offset + 276, (uint32)timer - 1);
    state = r_s8(character_offset + 476);
    if (state == 0)
    {
        if ((player_interaction.flags & 0x10) != 0 || (r_u32(0x8008A498) & 0x10) != 0)
            w_u8(character_offset + 476, 1);
    }
    else if (state == 1)
    {
        if ((player_interaction.flags & 0x10) == 0 && (r_u32(0x8008A498) & 0x10) == 0)
            w_u8(character_offset + 476, 2);
    }
    else
    {
        w_u8(character_offset + 476, 2);
        mask = (*control_bindings.active_scheme)[0];
        if ((gameplay_input.buttons & mask) == 0)
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
    if ((r_u32(object_offset + 228) & 0x1000000) != 0 && ((r_u32(local) & ((*control_bindings.active_scheme)[1] | 0xA000)) != 0 || (character_controller.flags & 0x60000) != 0))
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
    if ((character_controller.secondary_flags & 0x240) != 0 || ((uint32)(uint16)(character_controller.motion.form_flags >> 16) & 0xF) - 13 < 2)
        result |= 1;
    if ((r_u32(object_offset + 228) & 0x3C8) == 0)
        w_u32(character_offset + 408, 0);
    flags = r_u32(object_offset + 228);
    if ((flags & 0x100) != 0 || ((flags & 0x80) != 0 && r_s8(character_offset + 252) > 0))
    {
        if ((character_controller.secondary_flags & 0x240) != 0 && r_s8(character_offset + 252) < 4)
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
        if (r_s8(character_offset + 252) < 4 && (character_controller.secondary_flags & 0x240) == 0 && (r_u32(local) & 0x4000) != 0 && (previous & 0x4000) == 0)
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
        if ((r_u32(local) & 0x4000) != 0 && (previous & 0x4000) == 0 && r_s8(character_offset + 476) == 0 && (r_u32(local) & 0xA000) == 0 && (r_u32(object_offset + 28) & 0x400) == 0 && (character_controller.flags & 0x800000) == 0)
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
        mask = (*control_bindings.active_scheme)[1];
        if ((r_u32(local) & mask) != 0 && (previous & mask) == 0)
        {
            p2_func_80058ca0(object_offset, character_offset);
            state = r_s8(0x8008A490);
            character_controller.motion.jump_phase = 1;
            character_controller.motion.vertical_speed = 0;
            character_controller.timers.state_ticks = 0;
            w_u16(object_offset + 130, 0xFFFF);
            result |= 0x2000;
            if ((state == 16 && (r_u32(object_offset + 28) & 0x10) != 0) || (state == 32 && (r_u32(object_offset + 28) & 0x10) == 0))
            {
                ReverseScenePath(object_offset, character_offset);
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
        if (value == 0 && ((character_controller.flags & 0x40) != 0 || r_u32(0x8008A40C) != 0))
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
    if ((sint32)character_controller.flags < 0)
        goto finish;
    flags = r_u32(object_offset + 228);
    if ((flags & 0x1120) != 0)
        goto finish;
    if ((character_controller.secondary_flags & 0x240) != 0)
        goto button_actions;
    if ((r_u32(local) & 0x4000) != 0 && (flags & 0x15) == 0)
        result |= 0x80;
    mask = (*control_bindings.active_scheme)[0];
    if ((r_u32(local) & mask) != 0)
    {
        if ((previous & mask) == 0)
            result |= 0x40;
        else
        {
            value = (uint16)(character_controller.motion.form_flags >> 16) & 0xF;
            if (value == 4 || value == 5 || value - 10 < 2)
                result |= 0x40;
        }
    }
    if ((result & 0x40) != 0)
    {
        if ((character_controller.secondary_flags & 0x100) != 0)
            w_u32(object_offset + 28, r_u32(object_offset + 28) | 0x1000000);
        flags = player_interaction.flags;
        if ((flags & 4) != 0)
        {
            player_interaction.flags = flags | 8;
            result ^= 0x40;
        }
    }
    else
    {
        flags = player_interaction.flags;
        value = flags & 0xFFFFFFF7;
        player_interaction.flags = value;
        mask = (*control_bindings.active_scheme)[0];
        if ((r_u32(local) & mask) != 0)
        {
            if ((flags & 0x10) != 0)
                result &= 0xFFFFFFFD;
        }
        else
        {
            x = r_u32(0x8008A498);
            player_interaction.flags = value & 0xFFFFFFEF;
            w_u32(0x8008A498, x & 0xFFFFFFEF);
        }
    }
    value = (uint16)(character_controller.motion.form_flags >> 16) & 0xF;
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
            mask = (*control_bindings.active_scheme)[2];
            value = r_u32(local) & mask;
            if (value == mask && (previous & value) != value)
                result |= 0x400;
        }
    }
button_actions:
    mask = (*control_bindings.active_scheme)[1];
    if ((r_u32(local) & mask) != 0)
    {
        result |= 8;
        if ((previous & mask) == 0)
        {
            if (r_u32(character_offset + 104) != 0)
            {
                if ((sint16)character_controller.timers.jump_input_ticks < 6)
                {
                    x = r_u32(character_offset + 96);
                    y = r_u32(character_offset + 100);
                    z = r_u32(character_offset + 104);
                    flags = character_controller.flags;
                    character_controller.motion.jump_phase = 1;
                    character_controller.timers.state_ticks = 0;
                    character_controller.motion.horizontal_speed = x;
                    character_controller.motion.vertical_speed = y;
                    character_controller.movement.rise_speed_limit =  z;
                    character_controller.flags = flags | 0x10000000;
                }
                w_u32(character_offset + 104, 0);
            }
            character_controller.timers.jump_input_ticks = 0xFFFF;
        }
    }
    timer = (sint16)character_controller.timers.jump_input_ticks;
    if (timer < 6)
        character_controller.timers.jump_input_ticks = (uint32)timer + 1;
finish:
    if (r_u32(0x8008A184) == 1)
    {
        flags = character_controller.flags;
        w_u32(0x8008A184, 0);
        character_controller.flags = flags & 0xFFFFF7EF;
    }
    p2_guest_stack_pointer = saved_stack;
    return result;
}

uint32 UpdateCharacterMovement(uint32 object_offset)
{
    FUNCTION_MARKER(0x8004DCD0, "SLUS_005.78");
    uint32 saved_stack = p2_guest_stack_pointer;
    uint32 previous_kind = character_controller.motion.state_code;
    uint32 context = r_u32(0x8008A114);
    sint32 timer = (sint16)character_controller.timers.score_bonus_ticks;
    uint32 move_flags, flags, path, value, input, mask, result;
    // Preserve mapped scratch offsets used by nested operations
    p2_guest_stack_pointer -= 0x20;
    character_controller.motion.prev_state_code = previous_kind;
    flags = r_u32(0x800ABD70);
    path = r_u32(context + 136);
    w_u32(0x800ABD70, flags & 0xF83FFFFF);
    w_u32(0x800AC018, path);
    if (timer != 0)
    {
        value = (uint32)timer - 1;
        character_controller.timers.score_bonus_ticks = value;
        if ((value & 0xFFFF) == 0)
            character_controller.flags = character_controller.flags & 0xFFBFFFFF;
    }
    flags = game_flow.transition_flags;
    input = gameplay_input.buttons;
    w_u32(0x1F800388, input);
    if ((flags & 0x102) != 0)
    {
        gameplay_input.buttons = 0;
        p2_func_80066d08();
    }
    else
    {
        value = p2_func_80066d08();
        gameplay_input.buttons = p2_func_80060fdc(value);
    }
    mask = gameplay_input.release_mask;
    if (mask != 0)
    {
        input = gameplay_input.buttons;
        gameplay_input.release_mask = mask & input;
        gameplay_input.buttons = input & ~mask;
    }
    if ((sint32)GetCutsceneMode() < 0)
        gameplay_input.buttons = gameplay_input.buttons & 0x900;
    gameplay_input.ApplyDemo();
    flags = character_controller.secondary_flags;
    if ((flags & 0x400) != 0 && r_s16(object_offset + 130) == r_s16(r_u32(object_offset + 104) + 6) - 1)
        character_controller.secondary_flags = flags & 0xFFFFFBFF;
    value = BuildMovementFlags(object_offset, 0x800ABD68);
    flags = r_u32(object_offset + 28);
    move_flags = value;
    if ((flags & 0x8000000) != 0 && ((uint16)(character_controller.motion.form_flags >> 16) & 0xF) == 4)
        move_flags = value & 0xFFFFFFF7;
    UpdateMovementImpulse(object_offset, 0x800ABD68, &move_flags);
    UpdateJumpMotion(object_offset, 0x800ABD68, &move_flags);
    UpdateSpecialMovement(object_offset, 0x800ABD68, &move_flags);
    UpdateMovementState(object_offset, 0x800ABD68, &move_flags);
    UpdateActionInput(object_offset, 0x800ABD68, &move_flags);
    result = IntegrateCharacterMovement(object_offset, 0x800ABD68, move_flags);
    p2_guest_stack_pointer = saved_stack;
    return result;
}

uint32 UpdateForm5Movement(uint32 object_offset)
{
    FUNCTION_MARKER(0x8002AD4C, "SLUS_005.78");
    uint32 character = 0x800ABD68;
    uint32 input, flags, saved_stack, result, speed, acceleration, limit;
    if ((character_controller.secondary_flags & 0x400) != 0 &&
        r_s16(object_offset + 130) == r_s16(r_u32(object_offset + 104) + 6) - 1)
        character_controller.secondary_flags = character_controller.secondary_flags & 0xFFFFFBFF;
    w_u32(object_offset + 28, r_u32(object_offset + 28) | 0x400);
    p2_func_8004a928(object_offset, character);
    input = gameplay_input.buttons;
    flags = 0;
    speed = character_controller.motion.horizontal_speed;
    acceleration = (uint32)character_controller.movement.acceleration0;
    if ((input & 0x2000) != 0)
        speed = (r_u32(object_offset + 28) & 0x10) != 0 ? speed + acceleration : speed - acceleration;
    else if ((input & 0x8000) != 0)
        speed = (r_u32(object_offset + 28) & 0x10) != 0 ? speed - acceleration : speed + acceleration;
    else
        speed = (uint32)((sint32)(speed * 7) / 8);
    character_controller.motion.horizontal_speed = speed;
    if ((sint32)speed < 0)
        flags |= 2;
    limit = (uint32)character_controller.movement.acceleration0 << 2;
    if ((sint32)limit < (sint32)speed)
        character_controller.motion.horizontal_speed = limit;
    speed = character_controller.motion.vertical_speed;
    if ((input & 0x1000) != 0)
    {
        speed += (uint32)character_controller.movement.acceleration0;
        character_controller.motion.vertical_speed = speed;
        limit = (uint32)character_controller.movement.acceleration0 << 2;
        if ((sint32)limit < (sint32)speed)
            character_controller.motion.vertical_speed = limit;
    }
    else if ((input & 0x4000) != 0)
    {
        speed -= (uint32)character_controller.movement.acceleration0;
        character_controller.motion.vertical_speed = speed;
        limit = (0 - (uint32)character_controller.movement.acceleration0) << 2;
        if ((sint32)speed < (sint32)limit)
            character_controller.motion.vertical_speed = limit;
    }
    else
        character_controller.motion.vertical_speed = (uint32)((sint32)(speed * 7) / 8);
    character_controller.motion.jump_phase = 0;
    if ((input & (*control_bindings.active_scheme)[0]) != 0)
        flags |= 0x40;
    saved_stack = p2_guest_stack_pointer;
    // Preserve mapped scratch offsets used by nested operations
    p2_guest_stack_pointer -= 8;
    UpdateMovementImpulse(object_offset, character, &flags);
    UpdateActionInput(object_offset, character, &flags);
    result = IntegrateCharacterMovement(object_offset, character, flags);
    p2_guest_stack_pointer = saved_stack;
    return result;
}
