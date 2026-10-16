#include "game.h"
#include <limits.h>

sint32 p2_read_package(uint32 offset, uint32 destination, uint32 bytes)
{
    FILE *file;
    size_t transferred;
    sint32 failed;
    if (offset > LONG_MAX)
        return 0;
    file = xport_fopen("DATA/JESTERS.PKG", "rb");
    if (!file)
        return 0;
    if (fseek(file, (long)offset, SEEK_SET) != 0)
    {
        fclose(file);
        return 0;
    }
    transferred = xport_fread(psx_addr(destination, bytes), 1, bytes, file);
    failed = ferror(file);
    if (fclose(file) != 0)
        failed = 1;
    return !failed && transferred == bytes;
}

uint32 p2_func_8003af18(uint32 object_offset, uint32 path_offset, uint32 position_offset, uint32 floor_offset)
{
    uint32 saved_stack = p2_guest_stack_pointer;
    uint32 local, frame_offset, query_floor, input, inverse, transpose, visited;
    /* No candidate exists until this function selects a path */
    uint32 candidate = 0, fallback_frame = 0;
    uint32 initial_path = path_offset, direction, flags, definition, parent, matrix;
    uint32 kind_one_probe = 0, restrict_paths = 0, allow_special = 0, allow_link = 0;
    uint32 fallback_used = 0, result = 0, scene, x, y, z, value, index, repeated;
    sint32 kind, frame, probe_frame, retries = 2, previous_retries, gap;
    P2PathSelection selection;
    P2PathProbeResult probe;
    p2_guest_stack_pointer -= 112;
    local = p2_guest_stack_pointer;
    frame_offset = local;
    query_floor = local + 16;
    input = local + 32;
    inverse = local + 48;
    transpose = local + 64;
    visited = local + 88;
    kind = r_s8(object_offset + 126);
    if (kind == 1)
        kind_one_probe = (r_u32(0x800ABD68) & 0x60000) != 0;
    else if (r_s8(object_offset + 124) == 2 && (r_u32(object_offset + 116) == 19 || r_u32(0x8008A498) != 0))
    {
        parent = r_u32(p2_func_800536f0(0) + 132);
        value = r_u32(parent + 16) & 0x80000;
        fallback_frame = 1;
        restrict_paths = 1;
        if (value != 0)
            allow_special = 1;
        if ((r_u32(r_u32(object_offset + 132) + 16) & 0x10000) != 0)
        {
            fallback_frame = 1;
            allow_link = 1;
        }
    }
    direction = r_u32(object_offset + 28) & 0x10;
    if ((r_u32(object_offset + 228) & 0x26) != 0)
    {
        probe = p2_guard_path_change_with_selection(object_offset, position_offset, direction, fallback_frame);
        fallback_frame = probe.fallback_frame;
        if (((uint32)probe.value & 0xFF) == 0)
            goto done;
    }
    for (index = 0; index < 6; ++index)
        w_u32(visited + index * 4, 0);
select_path:
    frame = -1;
    if ((r_u32(object_offset + 28) & 0x40) != 0)
        frame = r_s16(object_offset + 136);
    w_u32(frame_offset, (uint32)frame);
    kind = r_s8(object_offset + 126);
    if (kind == 1 || (kind == 3 && r_s8(object_offset + 120) == -1))
    {
        if ((r_u32(r_u32(path_offset + 36) + 16) & 0x50) >= 0x41 && ((r_u32(object_offset + 28) & 0x4800) == 0 || retries < 2))
        {
            value = r_u32(path_offset + 28);
            if (value != 0)
            {
                candidate = value;
                if ((r_u16(path_offset + 14) & (direction | 8)) == 0 && (r_u16(value + 14) & 8) != 0)
                    candidate = r_u32(value + 16);
                if (candidate != 0)
                    goto check_candidate;
            }
        }
        selection = p2_func_8003adc0(path_offset, frame_offset, direction, r_u32(object_offset + 228));
        candidate = selection.segment_offset;
        fallback_frame = selection.first_frame;
        if (candidate != 0)
            goto check_candidate;
    }
    if (candidate == 0)
        goto fallback;
    if (r_s8(object_offset + 125) != 6 || r_u32(object_offset + 116) != 0 || r_u32(0x8008A498) == 0 || fallback_used != 0)
        goto done;
    if ((r_u32(r_u32(object_offset + 132) + 16) & 0x10000) == 0)
        goto done;
    value = r_u32(r_u32(r_u32(0x8008A114) + 136) + 20);
    fallback_used = 1;
    if (value == 0)
        goto done;
    candidate = value;
    goto check_candidate;
fallback:
    if (fallback_used == 1)
        goto done;
    fallback_used = 1;
    candidate = p2_func_8003bc08(object_offset, position_offset, floor_offset, direction);
    if (candidate == 0)
        goto done;
check_candidate:
    flags = r_u32(object_offset + 228);
    w_u32(object_offset + 228, flags & 0xFDFFFFFF);
    if ((flags & 0x20) != 0 && r_u32(r_u32(0x8008A114) + 136) == candidate)
        goto fallback;
    fallback_frame = initial_path;
    if (candidate != initial_path && candidate != 0)
    {
        repeated = 0;
        for (index = 0; index < 6; ++index)
        {
            fallback_frame = r_u32(visited + index * 4);
            if (candidate == fallback_frame)
            {
                repeated = 1;
                break;
            }
        }
        if (repeated != 0 && r_s8(0x8008A170) != 0)
            goto fallback;
        for (index = 0; index < 6; ++index)
        {
            fallback_frame = r_u32(visited + index * 4);
            if (fallback_frame == 0)
            {
                w_u32(visited + index * 4, candidate);
                break;
            }
        }
    }
    flags = r_u32(r_u32(path_offset + 36) + 16);
    if ((flags & 0x50) == 0 && retries >= 2)
        goto accept_path;
    if ((r_u32(object_offset + 28) & 0x6000) == 0x4000)
    {
        probe = p2_project_path_with_selection(r_u32(candidate + 36), r_s32(frame_offset), position_offset, query_floor, direction, 4096);
        fallback_frame = probe.fallback_frame;
        w_u32(frame_offset, (uint32)probe.value);
        if (probe.value < 0)
            goto done;
        goto check_floor;
    }
    if (((flags & 0x40) == 0 || candidate == path_offset) && retries == 2)
    {
        probe = p2_interpolate_path_with_selection(candidate, r_s32(frame_offset), query_floor, direction, fallback_frame);
        fallback_frame = probe.fallback_frame;
        goto floor_ready;
    }
    if ((r_u32(object_offset + 28) & 0x2000) == 0 || (r_u32(object_offset + 228) & 0x40000) != 0)
    {
        probe = p2_query_path_height_with_selection(r_u32(candidate + 36), position_offset, direction, -40960, fallback_frame);
        fallback_frame = probe.fallback_frame;
        w_u32(frame_offset, (uint32)probe.value);
    }
    if ((r_u32(object_offset + 228) & 0x24) != 0 && r_s32(frame_offset) == -1)
    {
        probe = p2_query_path_height_with_selection(r_u32(candidate + 36), position_offset, direction, -40960, fallback_frame);
        fallback_frame = probe.fallback_frame;
        if (probe.value != -1)
        {
            probe = p2_project_path_with_selection(r_u32(candidate + 36), probe.value, position_offset, query_floor, direction, 8192);
            fallback_frame = probe.fallback_frame;
            w_u32(frame_offset, (uint32)probe.value);
        }
    }
    fallback_frame = kind_one_probe;
    if (kind_one_probe != 0 && r_s32(frame_offset) == -1)
    {
        probe = p2_query_path_height_with_selection(r_u32(candidate + 36), position_offset, direction, -40960, fallback_frame);
        fallback_frame = probe.fallback_frame;
        if (probe.value != -1)
        {
            probe = p2_project_path_with_selection(r_u32(candidate + 36), probe.value, position_offset, query_floor, direction, 20480);
            fallback_frame = probe.fallback_frame;
            w_u32(frame_offset, (uint32)probe.value);
        }
    }
    frame = r_s32(frame_offset);
    if (frame < 0)
        probe_frame = -1;
    else
    {
        probe = p2_project_path_with_selection(r_u32(candidate + 36), frame, position_offset, query_floor, direction, 4096);
        fallback_frame = probe.fallback_frame;
        probe_frame = probe.value;
        if ((r_u32(object_offset + 228) & 0x24) != 0 && probe_frame == -1)
        {
            probe = p2_query_path_height_with_selection(r_u32(candidate + 36), position_offset, direction, -40960, fallback_frame);
            fallback_frame = probe.fallback_frame;
            if (probe.value != -1)
            {
                probe = p2_project_path_with_selection(r_u32(candidate + 36), probe.value, position_offset, query_floor, direction, 8192);
                fallback_frame = probe.fallback_frame;
                probe_frame = probe.value;
            }
        }
        if (probe_frame >= 0 && (r_u32(query_floor) - r_u32(position_offset) + 24576 > 0xC000 || r_u32(query_floor + 8) - r_u32(position_offset + 8) + 24576 > 0xC000))
            probe_frame = -1;
    }
    w_u32(frame_offset, (uint32)probe_frame);
floor_ready:
    if (r_s32(frame_offset) < 0)
        goto retry_path;
check_floor:
    if ((r_u32(r_u32(candidate + 36) + 16) & 0x10000) != 0)
        goto retry_path;
    gap = (sint32)(r_u32(position_offset + 4) - r_u32(query_floor + 4));
    if (gap < -1365 && ((r_u32(r_u32(path_offset + 36) + 16) & 0x40) == 0 || gap < -4096))
        goto retry_path;
    x = r_u32(query_floor);
    y = r_u32(query_floor + 4);
    z = r_u32(query_floor + 8);
    w_u32(floor_offset, x);
    w_u32(floor_offset + 4, y);
    w_u32(floor_offset + 8, z);
    goto accept_path;
retry_path:
    if (retries != 0)
    {
        flags = r_u32(object_offset + 28);
        if ((flags & 0x4000) == 0)
        {
            previous_retries = retries--;
            if (previous_retries != 2 || (r_u32(r_u32(path_offset + 36) + 16) & 0x10) == 0 || (flags & 0x800) == 0 || r_u32(path_offset + 28) == 0)
                path_offset = candidate;
            goto select_path;
        }
    }
    if (fallback_used == 0)
        goto fallback;
    goto done;
accept_path:
    if (candidate == initial_path && ((r_u32(object_offset + 28) & 0x800) == 0 || retries != 2))
        goto done;
    flags = r_u32(r_u32(candidate + 36) + 16);
    if ((flags & 0x10000) != 0)
        goto done;
    if (restrict_paths != 0)
    {
        if ((flags & 0x80000) != 0 && allow_special == 0)
            goto done;
        if (allow_link == 0 && (r_u32(r_u32(candidate + 36) + 16) & 0x10) != 0)
            goto done;
    }
    flags = r_u32(object_offset + 28);
    kind = r_s8(object_offset + 126);
    w_u32(object_offset + 28, flags & 0xFFFF9FFF);
    if (kind == 1 && r_s8(object_offset + 125) != 33)
    {
        scene = r_u32(0x8008A114);
        x = r_u32(position_offset);
        y = r_u32(position_offset + 4);
        z = r_u32(position_offset + 8);
        w_u32(scene + 16, x);
        w_u32(scene + 20, y);
        w_u32(scene + 24, z);
        p2_func_80046510(object_offset, candidate, r_s32(frame_offset));
    }
    else
    {
        if (kind != 1)
            p2_func_80046228(object_offset, r_u32(candidate + 36));
        w_u32(object_offset + 112, candidate);
    }
    w_u16(object_offset + 136, r_u32(frame_offset));
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
        p2_sdk_8007ec64(input, object_offset + 16);
        PopMatrix();
        parent = r_u32(object_offset + 132);
        matrix = r_u32(parent + 40);
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
    definition = r_u32(candidate + 36);
    w_u32(object_offset + 132, definition);
    if (definition != 0 && (matrix = r_u32(definition + 40)) != 0)
    {
        x = r_u32(object_offset + 16);
        w_u32(inverse, x - r_u32(matrix + 20));
        matrix = r_u32(r_u32(object_offset + 132) + 40);
        y = r_u32(object_offset + 20);
        w_u32(inverse + 4, y - r_u32(matrix + 24));
        matrix = r_u32(r_u32(object_offset + 132) + 40);
        z = r_u32(object_offset + 24);
        w_u32(inverse + 8, z - r_u32(matrix + 28));
        for (index = 0; index < 9; ++index)
        {
            matrix = r_u32(r_u32(object_offset + 132) + 40);
            value = r_u16(matrix + (index % 3) * 6 + (index / 3) * 2);
            w_u16(transpose + index * 2, value);
        }
        PushMatrix();
        x = r_u32(transpose);
        y = r_u32(transpose + 4);
        xport_gte_write_control(0, x);
        xport_gte_write_control(1, y);
        x = r_u32(transpose + 8);
        y = r_u32(transpose + 12);
        z = r_u32(transpose + 16);
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
    result = 1;
done:
    p2_guest_stack_pointer = saved_stack;
    return result;
}

uint32 p2_func_80058714(uint32 object_offset)
{
    uint32 flags = r_u32(object_offset + 228) | 0x800;
    w_u32(object_offset + 228, flags);
    return flags;
}

uint32 p2_func_8003bc08(uint32 object_offset, uint32 position_offset, uint32 floor_offset, uint32 direction)
{
    uint32 saved_stack = p2_guest_stack_pointer;
    uint32 local, flags, scene, current, parent, node, root, candidate, selected = 0;
    uint32 first, second, third, offset, scan, slot, phase, end, value, x, y, z;
    /* A skipped same-path probe has no sampled frame or output vector */
    uint32 first_frame = 0xFFFFFFFF, previous_frame = 0xFFFFFFFF, next_frame = 0xFFFFFFFF;
    uint32 query, displacement, input, output;
    sint32 status = -1, best, signed_y;
    flags = r_u32(object_offset + 228);
    p2_guest_stack_pointer -= 352;
    local = p2_guest_stack_pointer;
    if ((flags & 0x440000) != 0)
    {
        for (phase = 0; phase < 3; ++phase)
        {
            output = local + 16 + phase * 40;
            for (offset = 0; offset < 32; offset += 16)
            {
                value = r_u32(0x80010238 + offset);
                x = r_u32(0x8001023C + offset);
                y = r_u32(0x80010240 + offset);
                z = r_u32(0x80010244 + offset);
                w_u32(output + offset, value);
                w_u32(output + offset + 4, x);
                w_u32(output + offset + 8, y);
                w_u32(output + offset + 12, z);
            }
            value = r_u32(0x80010258);
            x = r_u32(0x8001025C);
            w_u32(output + 32, value);
            w_u32(output + 36, x);
        }
        scene = r_u32(0x8008A114);
        current = r_u32(scene + 136);
        node = r_u32(current + 16);
        if (node != 0 && (r_u32(r_u32(node + 36) + 16) & 0x10) != 0)
        {
            selected = r_u32(node + 28);
            if (selected != 0)
            {
                root = selected;
                if (selected != current)
                {
                    first_frame = p2_func_800231dc(r_u32(selected + 36), position_offset, direction, 0xFFFB0000);
                    if (first_frame != 0xFFFFFFFF)
                        first_frame = (uint32)p2_func_8003d3fc(r_u32(selected + 36), (sint32)first_frame, position_offset, local + 136, direction, 4096);
                }
                if (first_frame != 0xFFFFFFFF)
                {
                    value = r_u32(local + 136);
                    x = r_u32(position_offset);
                    y = r_u32(local + 140);
                    w_u32(local + 136, value - x);
                    x = r_u32(position_offset + 4);
                    y -= x;
                    z = r_u32(local + 144);
                    w_u32(local + 140, y);
                    x = r_u32(position_offset + 8);
                    w_u32(local + 144, z - x);
                    if ((sint32)y > 0)
                        first_frame = 0xFFFFFFFF;
                    if (first_frame != 0xFFFFFFFF)
                        goto return_selected;
                }
                node = r_u32(selected + 20);
                if (node != 0)
                {
                    current = r_u32(r_u32(0x8008A114) + 136);
                    selected = node;
                    if (selected != current)
                    {
                        previous_frame = p2_func_800231dc(r_u32(selected + 36), position_offset, direction, 0xFFFB0000);
                        if (previous_frame != 0xFFFFFFFF)
                            previous_frame = (uint32)p2_func_8003d3fc(r_u32(selected + 36), (sint32)previous_frame, position_offset, local + 136, direction, 4096);
                    }
                    if (previous_frame != 0xFFFFFFFF)
                    {
                        value = r_u32(local + 136);
                        x = r_u32(position_offset);
                        y = r_u32(local + 140);
                        w_u32(local + 136, value - x);
                        x = r_u32(position_offset + 4);
                        y -= x;
                        z = r_u32(local + 144);
                        w_u32(local + 140, y);
                        x = r_u32(position_offset + 8);
                        w_u32(local + 144, z - x);
                        if ((sint32)y > 0)
                            previous_frame = 0xFFFFFFFF;
                        if (previous_frame != 0xFFFFFFFF)
                            goto return_selected;
                    }
                }
                if ((r_u16(root + 14) & 0x10) != 0)
                {
                    node = r_u32(root + 28);
                    if (node != 0)
                    {
                        current = r_u32(r_u32(0x8008A114) + 136);
                        selected = node;
                        if (selected != current)
                        {
                            next_frame = p2_func_800231dc(r_u32(selected + 36), position_offset, direction, 0xFFFB0000);
                            if (next_frame != 0xFFFFFFFF)
                            {
                                next_frame = (uint32)p2_func_8003d3fc(r_u32(selected + 36), (sint32)next_frame, position_offset, local + 136, direction, 4096);
                            }
                        }
                        if (next_frame != 0xFFFFFFFF)
                        {
                            value = r_u32(local + 136);
                            x = r_u32(position_offset);
                            y = r_u32(local + 140);
                            w_u32(local + 136, value - x);
                            x = r_u32(position_offset + 4);
                            y -= x;
                            z = r_u32(local + 144);
                            w_u32(local + 140, y);
                            x = r_u32(position_offset + 8);
                            w_u32(local + 144, z - x);
                            if ((sint32)y > 0)
                                next_frame = 0xFFFFFFFF;
                            if (next_frame != 0xFFFFFFFF)
                                goto return_selected;
                        }
                    }
                }
            }
            scene = r_u32(0x8008A114);
        }
        else if (node != 0)
            scene = r_u32(0x8008A114);
        value = r_u32(scene + 136);
        w_u32(local + 16, value);
        value = r_u32(scene + 136);
        w_u32(local + 56, value);
        for (scan = 1; scan < 10; ++scan)
        {
            first = local + 16 + scan * 4;
            second = local + 56 + scan * 4;
            third = local + 96 + scan * 4;
            node = r_u32(first - 4);
            if (node != 0)
                w_u32(first, r_u32(node + 20));
            current = r_u32(r_u32(0x8008A114) + 136);
            if (r_u32(first) == current)
                w_u32(first, 0);
            node = r_u32(second - 4);
            if (node != 0)
                w_u32(second, r_u32(node + 16));
            current = r_u32(r_u32(0x8008A114) + 136);
            if (r_u32(second) == current)
            {
                node = r_u32(first);
                if ((r_u32(r_u32(node + 36) + 16) & 0x100011) != 0)
                    w_u32(second, r_u32(node + 28));
                else
                    w_u32(second, 0);
            }
            if (r_u32(second) != 0)
            {
                node = r_u32(second - 4);
                if ((r_u32(r_u32(node + 36) + 16) & 0x100011) != 0)
                    w_u32(third, r_u32(node + 28));
            }
            slot = 0;
            while (r_u32(first) != 0 || r_u32(second) != 0)
            {
                candidate = r_u32(slot == 0 ? first : slot == 1 ? second : third);
                if (candidate != 0)
                {
                    query = p2_func_800231dc(r_u32(candidate + 36), position_offset, direction, 0xFFFB0000);
                    if (query != 0xFFFFFFFF)
                    {
                        status = p2_func_8003d3fc(r_u32(candidate + 36), (sint32)query, position_offset, local, direction, 4096);
                        // Keep the copied query result separate from later status rejection
                        query = (uint32)status;
                    }
                    if (status >= 0)
                    {
                        x = r_u32(local) - r_u32(position_offset) + 24576;
                        if (x > 0xC000)
                            status = -1;
                        else
                        {
                            z = r_u32(local + 8) - r_u32(position_offset + 8) + 24576;
                            if (z > 0xC000)
                                status = -1;
                        }
                    }
                }
                else
                    query = 0xFFFFFFFF;
                if (query != 0xFFFFFFFF)
                {
                    x = r_u32(local) - r_u32(position_offset);
                    w_u32(local, x);
                    y = r_u32(local + 4) - r_u32(position_offset + 4);
                    z = r_u32(local + 8);
                    w_u32(local + 4, y);
                    z -= r_u32(position_offset + 8);
                    w_u32(local + 8, z);
                    if ((sint32)y > 0)
                        query = 0xFFFFFFFF;
                    if (query != 0xFFFFFFFF)
                    {
                        selected = candidate;
                        goto return_selected;
                    }
                }
                slot += 1;
                if (slot >= 3)
                    break;
            }
        }
        value = r_u32(0x80010260);
        x = r_u32(0x80010264);
        y = r_u32(0x80010268);
        w_u32(local + 152, value);
        w_u32(local + 156, x);
        w_u32(local + 160, y);
        value = r_u32(0x8001026C);
        x = r_u32(0x80010270);
        w_u32(local + 164, value);
        w_u32(local + 168, x);
        if (r_u32(local + 60) != 0)
        {
            if (r_u32(local + 64) != 0)
            {
                current = r_u32(r_u32(0x8008A114) + 136);
                value = r_u32(r_u32(r_u32(current + 16) + 16) + 20);
                w_u32(local + 152, value);
                current = r_u32(r_u32(0x8008A114) + 136);
                if (value == current)
                    w_u32(local + 152, 0);
            }
            scene = r_u32(0x8008A114);
            current = r_u32(scene + 136);
            node = r_u32(r_u32(current + 16) + 20);
            if (node != 0)
            {
                value = r_u32(node + 20);
                w_u32(local + 156, value);
                current = r_u32(scene + 136);
                if (value == current)
                {
                    w_u32(local + 156, 0);
                    current = r_u32(scene + 136);
                }
                node = r_u32(current + 16);
                value = r_u32(node + 20);
                if (value != 0)
                {
                    w_u32(local + 160, value);
                    if (value == r_u32(scene + 136))
                        w_u32(local + 160, 0);
                }
            }
        }
        node = r_u32(local + 28);
        if (node != 0)
        {
            value = r_u32(node + 16);
            w_u32(local + 164, value);
            if (value == r_u32(r_u32(0x8008A114) + 136))
                w_u32(local + 164, 0);
        }
        node = r_u32(local + 60);
        if (node != 0 && (r_u32(r_u32(node + 36) + 16) & 0x10) != 0)
        {
            node = r_u32(node + 28);
            if (node != 0)
            {
                value = r_u32(node + 20);
                w_u32(local + 168, value);
                if (value == r_u32(r_u32(0x8008A114) + 136))
                    w_u32(local + 168, 0);
            }
        }
        for (scan = 0; scan < 5; ++scan)
        {
            selected = r_u32(local + 152 + scan * 4);
            query = 0xFFFFFFFF;
            if (selected != 0)
            {
                query = p2_func_800231dc(r_u32(selected + 36), position_offset, direction, 0xFFFB0000);
                if (query != 0xFFFFFFFF)
                    query = (uint32)p2_func_8003d3fc(r_u32(selected + 36), (sint32)query, position_offset, local, direction, 4096);
            }
            if (query != 0xFFFFFFFF)
            {
                x = r_u32(local) - r_u32(position_offset);
                w_u32(local, x);
                y = r_u32(local + 4) - r_u32(position_offset + 4);
                z = r_u32(local + 8);
                w_u32(local + 4, y);
                z -= r_u32(position_offset + 8);
                w_u32(local + 8, z);
                if ((sint32)y > 0)
                    query = 0xFFFFFFFF;
                if (query != 0xFFFFFFFF)
                    goto return_selected;
            }
        }
        selected = 0;
        goto return_selected;
    }
    if ((flags & 4) != 0)
    {
        status = -1;
        current = r_u32(r_u32(0x8008A114) + 136);
        if (r_u32(current + 20) != 0)
        {
            value = r_u32(0x80010274);
            x = r_u32(0x80010278);
            y = r_u32(0x8001027C);
            w_u32(local + 176, value);
            w_u32(local + 180, x);
            w_u32(local + 184, y);
            w_u32(local + 188, r_u32(0x80010280));
            current = r_u32(r_u32(0x8008A114) + 136);
            value = r_u32(current + 20);
            w_u32(local + 176, value);
            if (value != 0)
            {
                current = r_u32(r_u32(0x8008A114) + 136);
                value = r_u32(r_u32(current + 20) + 20);
                w_u32(local + 180, value);
                if (value != 0)
                {
                    current = r_u32(r_u32(0x8008A114) + 136);
                    value = r_u32(r_u32(r_u32(current + 20) + 20) + 20);
                    w_u32(local + 184, value);
                    if (value != 0)
                    {
                        current = r_u32(r_u32(0x8008A114) + 136);
                        w_u32(local + 188, r_u32(r_u32(r_u32(r_u32(current + 20) + 20) + 20) + 20));
                    }
                }
            }
            for (scan = 0; scan < 4; ++scan)
            {
                if (status != -1)
                    goto return_selected;
                selected = r_u32(local + 176 + scan * 4);
                query = 0xFFFFFFFF;
                if (selected != 0)
                    query = p2_func_800231dc(r_u32(selected + 36), position_offset, direction, 0xFFFB0000);
                if (query != 0xFFFFFFFF)
                    status = p2_func_8003d3fc(r_u32(selected + 36), (sint32)query, position_offset, local + 136, direction, 4096);
                if (status != -1)
                {
                    x = r_u32(local + 136) - r_u32(position_offset);
                    w_u32(local + 136, x);
                    y = r_u32(local + 140) - r_u32(position_offset + 4);
                    z = r_u32(local + 144);
                    w_u32(local + 140, y);
                    z -= r_u32(position_offset + 8);
                    w_u32(local + 144, z);
                    if ((sint32)y > 0)
                        status = -1;
                    if (status != -1 && (r_u32(r_u32(selected + 36) + 16) & 0x10) != 0)
                    {
                        value = (uint32)p2_func_8005dbb8(r_s16(object_offset + 148));
                        displacement = (uint32)(((sint64)((sint32)value >> 2) * 20480) >> 12);
                        value = (uint32)p2_func_8005dbb8((sint16)(1024 - r_u16(object_offset + 148)));
                        value = (uint32)(((sint64)((sint32)value >> 2) * 20480) >> 12);
                        x = r_u32(position_offset);
                        y = r_u32(position_offset + 4);
                        z = r_u32(position_offset + 8);
                        w_u32(local + 192, x);
                        w_u32(local + 196, y);
                        w_u32(local + 200, z);
                        x = r_u32(local + 192);
                        z = r_u32(local + 200);
                        w_u32(local + 192, x - displacement);
                        w_u32(local + 200, z - value);
                        query = p2_func_800231dc(r_u32(selected + 36), local + 192, direction, 0xFFFB0000);
                        if (query == 0xFFFFFFFF)
                            status = -1;
                        else
                            status = p2_func_8003d3fc(r_u32(selected + 36), (sint32)query, local + 192, local + 136, direction, 8192);
                    }
                }
            }
        }
        if (status == -1)
            selected = 0;
        goto return_selected;
    }
    if ((flags & 0x20) == 0)
    {
        signed_y = r_s8(0x8008A50C);
        if (signed_y == 1)
        {
            current = r_u32(r_u32(0x8008A114) + 136);
            w_u8(0x8008A50C, 2);
            selected = r_u32(current + 28);
            goto return_selected;
        }
        selected = 0;
        if (r_s8(object_offset + 124) != 2 || r_u32(object_offset + 116) != 19)
            goto return_selected;
        current = r_u32(r_u32(0x8008A114) + 136);
        if ((r_u32(r_u32(current + 36) + 16) & 0x90010) == 0)
            goto return_selected;
        selected = r_u32(current + 20);
        status = -1;
        if (selected != 0)
            status = p2_func_8003d3fc(r_u32(selected + 36), (sint32)floor_offset, position_offset, local + 328, direction, 4096);
        if (status == -1)
            selected = 0;
        goto return_selected;
    }
    for (phase = 0; phase < 2; ++phase)
    {
        input = phase == 0 ? 0x80010284 : 0x800102A8;
        output = phase == 0 ? local + 208 : local + 248;
        for (offset = 0; offset < 32; offset += 16)
        {
            value = r_u32(input + offset);
            x = r_u32(input + offset + 4);
            y = r_u32(input + offset + 8);
            z = r_u32(input + offset + 12);
            w_u32(output + offset, value);
            w_u32(output + offset + 4, x);
            w_u32(output + offset + 8, y);
            w_u32(output + offset + 12, z);
        }
        w_u32(output + 32, r_u32(input + 32));
    }
    current = r_u32(r_u32(0x8008A114) + 136);
    selected = r_u32(current + 28);
    best = -1;
    status = -1;
    if (selected != 0 && (r_u32(r_u32(current + 36) + 16) & 0x100000) != 0)
    {
        query = p2_func_800231dc(r_u32(selected + 36), position_offset, direction, 0xFFFB0000);
        if (query != 0xFFFFFFFF)
            status = p2_func_8003d3fc(r_u32(selected + 36), (sint32)query, position_offset, local + 192, direction, 81920);
        if (status != -1)
        {
            x = r_u32(local + 192) - r_u32(position_offset);
            w_u32(local + 192, x);
            y = r_u32(local + 196) - r_u32(position_offset + 4);
            z = r_u32(local + 200);
            w_u32(local + 196, y);
            z -= r_u32(position_offset + 8);
            w_u32(local + 200, z);
            if ((sint32)y > 0)
                status = -1;
        }
    }
    parent = r_u32(0x800ABE60);
    node = r_u32(parent + 20);
    if (node != 0)
    {
        current = r_u32(r_u32(0x8008A114) + 136);
        if (current == node)
        {
            selected = current;
            query = p2_func_800231dc(r_u32(current + 36), position_offset, direction, 0xFFFB0000);
            status = p2_func_8003d3fc(r_u32(selected + 36), (sint32)query, position_offset, local + 192, direction, 4096);
            if (status >= 0)
            {
                x = r_u32(local + 192) - r_u32(position_offset) + 24576;
                if (x > 0xC000)
                    goto ranked_shortcut_finished;
                z = r_u32(local + 200) - r_u32(position_offset + 8) + 24576;
                if (z > 0xC000)
                    goto ranked_shortcut_finished;
            }
            x = r_u32(local + 192) - r_u32(position_offset);
            w_u32(local + 192, x);
            y = r_u32(local + 196) - r_u32(position_offset + 4);
            z = r_u32(local + 200);
            w_u32(local + 196, y);
            z -= r_u32(position_offset + 8);
            w_u32(local + 200, z);
            if ((sint32)y >= 4097)
                status = -1;
        }
    }
ranked_shortcut_finished:
    if (status != -1)
        goto ranked_finished;
    scene = r_u32(0x8008A114);
    current = r_u32(scene + 136);
    value = r_u32(current + 20);
    w_u32(local + 208, value);
    if (value != 0)
    {
        current = r_u32(scene + 136);
        w_u32(local + 212, r_u32(r_u32(current + 20) + 20));
    }
    current = r_u32(scene + 136);
    node = r_u32(current + 16);
    if (node != 0)
    {
        w_u32(local + 216, r_u32(node + 20));
        current = r_u32(scene + 136);
        node = r_u32(r_u32(current + 16) + 16);
        if (node != 0)
            w_u32(local + 220, r_u32(node + 20));
    }
    for (phase = 0; phase < 3; ++phase)
    {
        if (phase == 0)
        {
            scan = 0;
            end = 4;
        }
        else if (phase == 1)
        {
            if (r_u32(local + 212) != 0)
            {
                current = r_u32(r_u32(0x8008A114) + 136);
                w_u32(local + 224, r_u32(r_u32(r_u32(current + 20) + 20) + 20));
            }
            current = r_u32(r_u32(0x8008A114) + 136);
            node = r_u32(current + 16);
            if (node != 0)
            {
                node = r_u32(node + 16);
                if (node != 0)
                {
                    node = r_u32(node + 16);
                    if (node != 0)
                        w_u32(local + 228, r_u32(node + 20));
                }
            }
            scan = 4;
            end = 6;
        }
        else
        {
            if (r_u32(local + 208) != 0)
            {
                current = r_u32(r_u32(0x8008A114) + 136);
                w_u32(local + 232, r_u32(r_u32(current + 20) + 16));
            }
            scene = r_u32(0x8008A114);
            current = r_u32(scene + 136);
            node = r_u32(current + 16);
            if (node != 0)
                w_u32(local + 236, r_u32(node + 16));
            current = r_u32(scene + 136);
            w_u32(local + 240, r_u32(current + 16));
            scan = 6;
            end = 9;
        }
        for (; scan < end; ++scan)
        {
            candidate = r_u32(local + 208 + scan * 4);
            if (candidate == 0)
                continue;
            current = r_u32(r_u32(0x8008A114) + 136);
            parent = r_u32(candidate + 36);
            if (parent == r_u32(current + 36) || candidate == current)
                continue;
            query = p2_func_800231dc(parent, position_offset, direction, 0xFFFB0000);
            w_u32(local + 288 + scan * 4, query);
            if (query == 0xFFFFFFFF)
                continue;
            candidate = r_u32(local + 208 + scan * 4);
            status = p2_func_8003d3fc(r_u32(candidate + 36), r_s32(local + 288 + scan * 4), position_offset, local + 192, direction, 4096);
            if (status >= 0)
            {
                x = r_u32(local + 192) - r_u32(position_offset) + 24576;
                if (x > 0xC000)
                    status = -1;
                else
                {
                    z = r_u32(local + 200) - r_u32(position_offset + 8) + 24576;
                    if (z > 0xC000)
                        status = -1;
                }
            }
            if (status != -1)
            {
                x = r_u32(local + 192) - r_u32(position_offset);
                w_u32(local + 192, x);
                y = r_u32(local + 196) - r_u32(position_offset + 4);
                z = r_u32(local + 200);
                w_u32(local + 196, y);
                z -= r_u32(position_offset + 8);
                w_u32(local + 200, z);
                if ((sint32)y > 0)
                    status = -1;
                else
                {
                    value = 0 - y;
                    w_u32(local + 248 + scan * 4, value);
                    if (best == -1 || (sint32)value < r_s32(local + 248 + (uint32)best * 4))
                        best = (sint32)scan;
                }
            }
        }
        if (best != -1)
        {
            if (phase == 0)
                status = r_s32(local + 288 + (uint32)best * 4);
            selected = r_u32(local + 208 + (uint32)best * 4);
        }
        if (phase < 2 && status != -1)
            goto ranked_finished;
    }
ranked_finished:
    if (status < 0)
    {
        p2_func_80058714(object_offset);
        selected = 0;
    }
return_selected:
    p2_guest_stack_pointer = saved_stack;
    return selected;
}

uint32 p2_func_8004922c(uint32 object_offset, uint32 character_offset, uint32 decelerate)
{
    uint32 flags, object_flags, speed, cap, minimum;
    uint32 acceleration = 0, slope = 0, x, y, z, length, angle, folded, value, proposed, parent, result;
    object_flags = r_u32(object_offset + 28);
    speed = r_u32(character_offset + 24);
    decelerate &= 0xFF;
    if ((object_flags & 0x400) != 0)
        goto ground_speed;
    flags = r_u32(object_offset + 228);
    if ((flags & 0x1024) != 0)
    {
    ground_speed:
        /* Braking uses the same ground speed limit as acceleration */
        cap = r_u32(character_offset + 748);
        if (decelerate != 0)
        {
            if ((r_u32(character_offset) & 0x20000000) == 0)
            {
                if ((sint32)speed < 0)
                    acceleration = r_u32(character_offset + 752);
                else
                    acceleration = 0 - r_u32(character_offset + 752);
            }
        }
        else
        {
            minimum = r_u32(character_offset + 744);
            acceleration = r_u32(character_offset + 756);
            if ((sint32)speed < 0)
                acceleration += r_u32(character_offset + 752);
        }
        goto apply_speed;
    }
    cap = r_u32(character_offset + 740);
    if ((flags & 0x10) != 0)
    {
        parent = r_u32(0x800AC074);
        cap = (uint32)(((sint64)(sint32)cap * r_s32(parent + 76)) >> 12);
    }
    else if ((flags & 1) != 0)
    {
        parent = r_u32(character_offset + 780);
        cap = (uint32)(((sint64)(sint32)cap * r_s32(parent + 72)) >> 12);
    }
    else if ((flags & 4) != 0)
        cap = (uint32)(((sint64)(sint32)cap * r_s32(0x8008A234)) >> 12);
    if (decelerate != 0)
    {
        if ((sint32)speed <= 0)
            acceleration = r_u32(character_offset + 760);
        else
            acceleration = 0 - r_u32(character_offset + 760);
    }
    else
    {
        minimum = r_u32(character_offset + 736);
        acceleration = r_u32(character_offset + 764);
        if ((sint32)speed < 0)
            acceleration += r_u32(character_offset + 760);
    }
    if (r_u32(character_offset + 500) != 0 && (r_u32(object_offset + 228) & 0x1440100) == 0)
    {
        x = (uint32)(r_s32(character_offset + 692) >> 6);
        y = (uint32)(r_s32(character_offset + 696) >> 6);
        z = (uint32)(r_s32(character_offset + 700) >> 6);
        value = x * x + y * y + z * z;
        if ((sint32)value < 0)
            length = p2_sdk_8007e794(value >> 2) << 1;
        else
            length = p2_sdk_8007e794(value);
        angle = (0 - (uint32)(sint16)p2_func_8005dcd4(r_s32(character_offset + 696), (sint32)length)) & 0xFFF;
        if (angle != 0)
        {
            folded = angle;
            if (angle >= 1025)
                folded = 4096 - angle;
            if (r_s32(character_offset + 500) >= (sint32)folded)
            {
                value = (uint32)p2_func_8005dbb8((sint16)angle);
                slope = 0 - (uint32)(((sint64)1024 * ((sint32)value >> 2)) >> 12);
            }
        }
        if (decelerate != 0)
        {
            value = 4096 - r_u32(character_offset + 500);
            acceleration = (uint32)(((sint64)(sint32)acceleration * (sint32)value) >> 12);
        }
        else if ((sint32)slope < 0 && (sint32)cap < (sint32)speed)
            acceleration = 0;
        value = 15360 - cap;
        value = (uint32)(((sint64)(sint32)value * r_s32(character_offset + 500)) >> 12);
        cap += value;
        if (decelerate == 0)
        {
            value = 4096 - r_u32(character_offset + 500);
            minimum = (uint32)(((sint64)(sint32)minimum * (sint32)value) >> 12);
        }
        else if ((sint32)speed < 0)
            cap = 0 - cap;
        else if (speed == 0 && (sint32)(acceleration ^ slope) < 0)
            acceleration = 0 - acceleration;
    }
apply_speed:
    if (decelerate != 0)
    {
        if (speed != 0)
        {
            acceleration += slope;
            proposed = speed + acceleration;
            if ((sint32)(speed ^ proposed) < 0)
                speed = 0;
            else
            {
                speed = proposed;
                if ((r_u32(object_offset + 28) & 0x400) == 0)
                {
                    if ((sint32)proposed < 0 && (sint32)cap < 0 && (sint32)proposed < (sint32)cap)
                        speed = cap;
                    else if ((sint32)proposed > 0 && (sint32)cap > 0 && (sint32)cap < (sint32)proposed)
                        speed = cap;
                }
            }
        }
        else
        {
            proposed = slope - acceleration;
            if (slope != 0 && (sint32)(slope ^ proposed) >= 0)
                speed = proposed;
        }
    }
    else
    {
        acceleration += slope;
        if ((sint32)speed < 0)
            speed += acceleration;
        else if ((sint32)speed < (sint32)minimum)
            speed = minimum;
        else if ((sint32)speed < (sint32)cap)
        {
            speed += acceleration;
            if ((sint32)cap < (sint32)speed)
                speed = cap;
        }
        else if ((r_u32(object_offset + 28) & 0x400) == 0)
            speed = cap;
    }
    result = r_u32(character_offset) & 0x800000;
    w_u32(character_offset + 24, speed);
    if (result != 0)
    {
        parent = r_u32(character_offset + 124);
        value = r_u32(parent + 68);
        speed = (uint32)(((sint64)(sint32)speed * (sint32)value) >> 12);
        parent = r_u32(character_offset + 124);
        w_u32(character_offset + 24, speed);
        value = r_u32(parent + 68);
        result = r_u32(character_offset + 28);
        w_u32(character_offset + 28, (uint32)(((sint64)(sint32)result * (sint32)value) >> 12));
    }
    return result;
}

uint32 p2_func_80024890(void)
{
    uint32 saved_stack = p2_guest_stack_pointer;
    uint32 difference, scene, count, value, factor, divisor, result;
    p2_guest_stack_pointer -= 16;
    difference = p2_guest_stack_pointer;
    scene = r_u32(0x8008A114);
    value = r_u32(scene + 28) - r_u32(scene + 4);
    w_u32(difference, value);
    value = r_u32(scene + 32) - r_u32(scene + 8);
    w_u32(difference + 4, value);
    value = r_u32(scene + 36) - r_u32(scene + 12);
    w_u32(difference + 8, value);
    count = r_u8(scene + 72);
    if (count != 0)
    {
        count = (count - 1) & 0xFF;
        w_u8(scene + 72, count);
        if (count < 45)
        {
            scene = r_u32(0x8008A114);
            count = r_u8(scene + 72);
            factor = r_u16(0x80010054 + count * 2);
        }
        else
        {
            scene = r_u32(0x8008A114);
            count = r_u8(scene + 72);
            divisor = (count + 2) << 12;
            factor = 0x1000000 / divisor;
        }
        value = r_u32(difference);
        w_u32(difference, (uint32)(((sint64)(sint32)value * (sint32)factor) >> 12));
    }
    scene = r_u32(0x8008A114);
    count = r_u8(scene + 73);
    if (count != 0)
    {
        count = (count - 1) & 0xFF;
        w_u8(scene + 73, count);
        if (count < 45)
        {
            scene = r_u32(0x8008A114);
            count = r_u8(scene + 73);
            factor = r_u16(0x80010054 + count * 2);
        }
        else
        {
            /* Each axis derives its own long transition coefficient */
            scene = r_u32(0x8008A114);
            count = r_u8(scene + 73);
            divisor = (count + 2) << 12;
            factor = 0x1000000 / divisor;
        }
        value = r_u32(difference + 4);
        w_u32(difference + 4, (uint32)(((sint64)(sint32)value * (sint32)factor) >> 12));
        scene = r_u32(0x8008A114);
    }
    count = r_u8(scene + 74);
    if (count != 0)
    {
        count = (count - 1) & 0xFF;
        w_u8(scene + 74, count);
        if (count < 45)
        {
            scene = r_u32(0x8008A114);
            count = r_u8(scene + 74);
            factor = r_u16(0x80010054 + count * 2);
        }
        else
        {
            scene = r_u32(0x8008A114);
            count = r_u8(scene + 74);
            divisor = (count + 2) << 12;
            factor = 0x1000000 / divisor;
        }
        value = r_u32(difference + 8);
        w_u32(difference + 8, (uint32)(((sint64)(sint32)value * (sint32)factor) >> 12));
    }
    scene = r_u32(0x8008A114);
    value = r_u32(difference);
    w_u32(scene + 4, r_u32(scene + 4) + value);
    value = r_u32(scene + 8);
    value += r_u32(difference + 4);
    w_u32(scene + 8, value);
    value = r_u32(scene + 12);
    value += r_u32(difference + 8);
    w_u32(scene + 12, value);
    result = r_u8(scene) & 0xFE;
    w_u8(scene, result);
    p2_guest_stack_pointer = saved_stack;
    return result;
}

uint32 p2_func_800558ac(uint32 mode)
{
    uint32 flags = r_u32(0x800AC028);
    uint32 value = ((flags & 0xFFF0FFFF) | (mode << 16)) & 0xFF0FFFFF;
    w_u32(0x800AC028, value | (mode << 20));
    return (flags >> 20) & 0xF;
}

uint32 p2_func_800276dc(void)
{
    uint32 saved_stack = p2_guest_stack_pointer;
    uint32 local, old_mapping, flags, x, y, z, previous_camera, frames, ticks, deadline, wait, value, result;
    sint32 index = r_s8(0x8008A127);
    sint32 fraction = 30;
    old_mapping = r_u32(0x8008A2B4);
    p2_guest_stack_pointer -= 16;
    local = p2_guest_stack_pointer;
    x = (uint32)r_s8(0x8008A120);
    y = (uint32)r_s8(0x8008A121);
    z = (uint32)r_s8(0x8008A122);
    w_u8(local, x);
    w_u8(local + 1, y);
    w_u8(local + 2, z);
    x = (uint32)r_s8(0x8008A124);
    y = (uint32)r_s8(0x8008A125);
    z = (uint32)r_s8(0x8008A126);
    w_u8(local + 8, x);
    w_u8(local + 9, y);
    w_u8(local + 10, z);
    flags = r_u32(0x1F800320);
    w_u32(0x8008A2B4, 0x80087878);
    value = (uint32)index * 2728 + 0x800AC680;
    w_u32(0x8008A310, value);
    w_u32(0x1F800320, flags | 2);
    previous_camera = p2_func_800558ac((uint32)r_s8(local + 8 + (uint32)index));
    index = r_s8(0x8008A127);
    p2_func_8002dcd0(r_s8(local + (uint32)index), 0);
    ticks = r_u32(0x8008A434);
    frames = 682;
    do
    {
        if (p2_func_80066d08() != 0)
            break;
        fraction -= 10;
        p2_func_8005cd24();
        p2_func_80069ac0();
        wait = 33;
        if (fraction <= 0)
        {
            wait = 34;
            fraction += 30;
        }
        deadline = ticks + wait;
        while (r_u32(0x8008A434) < deadline)
            ;
        ticks = r_u32(0x8008A434);
        value = r_u32(0x8008A518);
        ticks = r_u32(0x8008A434);
        frames -= 1;
        w_u32(0x8008A518, value + 1);
    } while (frames != 0);
    while (p2_func_80069ac0() != 0)
        ;
    p2_func_8002dedc();
    p2_func_800558ac(previous_camera);
    flags = r_u32(0x1F800320);
    value = r_u8(0x8008A127);
    w_u32(0x8008A2B4, old_mapping);
    w_u32(0x8008A310, 0);
    flags &= 0xFFFFFFFD;
    value += 1;
    w_u8(0x8008A127, value);
    result = (uint32)(sint32)(sint8)value;
    w_u32(0x1F800320, flags);
    if ((sint32)result == 3)
        w_u8(0x8008A127, 0);
    p2_guest_stack_pointer = saved_stack;
    return result;
}

uint32 p2_func_80039614(uint32 object_offset, uint32 other_offset)
{
    uint32 saved_stack = p2_guest_stack_pointer;
    uint32 difference, extent, parent, matrix, x, y, z, value, flags, best, target, result;
    uint32 own_shape, other_shape, own_index, other_index;
    uint32 selected_other = 0, selected_own = 0, selected = 0;
    sint32 kind, other_kind;
    p2_guest_stack_pointer -= 32;
    difference = p2_guest_stack_pointer;
    extent = difference + 16;
    x = r_u32(other_offset + 92);
    y = r_u32(other_offset + 96);
    z = r_u32(other_offset + 100);
    w_u32(difference, x);
    w_u32(difference + 4, y);
    w_u32(difference + 8, z);
    parent = r_u32(other_offset + 132);
    if (parent != 0)
    {
        matrix = r_u32(parent + 40);
        if (matrix != 0 && (r_u32(other_offset + 28) & 0x2000) == 0)
        {
            x = r_u32(difference);
            w_u32(difference, x + r_u32(matrix + 32));
            matrix = r_u32(r_u32(other_offset + 132) + 40);
            y = r_u32(difference + 4);
            w_u32(difference + 4, y + r_u32(matrix + 36));
            matrix = r_u32(r_u32(other_offset + 132) + 40);
            z = r_u32(difference + 8);
            w_u32(difference + 8, z + r_u32(matrix + 40));
        }
    }
    x = r_u32(object_offset + 92) - r_u32(difference);
    w_u32(difference, x);
    y = r_u32(object_offset + 96) - r_u32(difference + 4);
    w_u32(difference + 4, y);
    z = r_u32(object_offset + 100) - r_u32(difference + 8);
    w_u32(difference + 8, z);
    result = p2_func_80039404(object_offset, other_offset + 60, difference, r_u32(other_offset + 72)) << 24;
    if (result == 0)
        goto finish;
    best = r_u32(object_offset + 144);
    own_shape = object_offset + 160;
    for (own_index = 0; own_index < 4; ++own_index, own_shape += 16)
    {
        if (r_u32(own_shape + 12) == 0xFFFFFFFF)
            break;
        other_shape = other_offset + 160;
        for (other_index = 0; other_index < 4; ++other_index, other_shape += 16)
        {
            if (r_u32(other_shape + 12) == 0xFFFFFFFF)
                break;
            if (p2_func_80038990(own_shape, other_offset, difference, extent, other_shape) != 0)
            {
                value = r_u32(extent);
                if ((sint32)value <= (sint32)best && (selected == 0 || (sint32)value < (sint32)best))
                {
                    best = value;
                    selected_other = other_shape;
                    selected_own = own_shape;
                    selected = 1;
                }
            }
        }
    }
    result = 1;
    if (selected == 0 || r_s32(object_offset + 144) < (sint32)best)
        goto finish;
    kind = r_s8(object_offset + 126);
    if (kind == 1)
    {
        other_kind = r_s8(other_offset + 126);
        if (other_kind == 2)
        {
            w_u32(0x1F800318, 0);
            if (p2_func_80030a34(other_offset) != 0)
                best = 4096;
            if ((r_u32(object_offset + 28) & 0x2400) != 0 || (r_u32(object_offset + 228) & 2) != 0)
            {
                x = r_u32(object_offset + 64);
                y = r_u32(object_offset + 76);
                value = r_u32(object_offset + 96);
                x -= y;
                value = (uint32)(((sint64)(sint32)value * (sint32)best) >> 12);
                y = r_u32(other_offset + 96);
                x += value + 6144;
                y = (uint32)(((sint64)(sint32)y * (sint32)best) >> 12);
                y += r_u32(other_offset + 64);
                if ((sint32)x >= (sint32)y)
                    w_u32(0x1F800318, 4096);
            }
            goto prepare_callback;
        }
    }
    else
        other_kind = r_s8(other_offset + 126);
    if (other_kind == 1 && r_s8(object_offset + 126) == 2)
    {
        w_u32(0x1F800310, 0);
        if ((r_u32(other_offset + 28) & 0x2400) != 0 || (r_u32(other_offset + 228) & 2) != 0)
        {
            x = r_u32(other_offset + 64);
            y = r_u32(other_offset + 76);
            value = r_u32(other_offset + 96);
            x -= y;
            value = (uint32)(((sint64)(sint32)value * (sint32)best) >> 12);
            y = r_u32(object_offset + 96);
            x += value + 6144;
            y = (uint32)(((sint64)(sint32)y * (sint32)best) >> 12);
            y += r_u32(object_offset + 64);
            if ((sint32)x >= (sint32)y)
                w_u32(0x1F800310, 4096);
        }
    }
prepare_callback:
    w_u32(0x1F800314, best);
    kind = r_s8(object_offset + 126);
    if (kind == 1)
        value = r_u32(selected_other + 12) & 7;
    else if (kind == 2)
        value = r_u32(selected_own + 12) & 7;
    else
        value = 0;
    w_u32(0x1F80031C, value);
    kind = r_s8(object_offset + 126);
    target = r_u32(0x800B7A48 + (uint32)kind * 4);
    result = p2_dispatch_collision_callback(target, object_offset, other_offset, 0x1F800310) << 24;
    if (result != 0)
    {
        flags = r_u32(object_offset + 28);
        w_u32(object_offset + 144, best);
        result = flags & 0xFFFFBFFF;
        w_u32(object_offset + 28, result);
    }
finish:
    p2_guest_stack_pointer = saved_stack;
    return result;
}

uint32 p2_sdk_8007e794(uint32 input)
{
    return SquareRoot12PSX(input, 0x80089AA4);
}

P2PathProbeResult p2_sqrt_with_selection(uint32 input, uint32 fallback_frame)
{
    P2PathProbeResult output;
    uint32 bits = (input & 0x80000000) != 0 ? ~input : input;
    uint32 leading = 0;
    while (leading < 32 && (bits & 0x80000000) == 0)
    {
        ++leading;
        bits <<= 1;
    }
    output.value = (sint32)p2_sdk_8007e794(input);
    output.fallback_frame = leading == 32 ? fallback_frame : leading & 1;
    return output;
}

uint32 p2_func_80053e7c(sint16 mode, uint32 scene_offset, sint32 first, sint16 second)
{
    uint32 saved_stack = p2_guest_stack_pointer;
    uint32 local, vector, sample, object_slot, inverse, world, secondary_vector, secondary_inverse;
    uint32 character, marker, node, definition, path = 0;
    uint32 search_path, search_frame, checkpoint_selected = 0;
    uint32 frame = (uint32)(sint32)second, best_distance, count, selected_frame;
    uint32 object, model, other_model, other_object, context, direction, matrix;
    uint32 x, y, z, value, flags, index, distance, result;
    sint32 sample_frame;
    FUNCTION_MARKER(0x80053E7C, "SLUS_005.78");
    p2_guest_stack_pointer -= 144;
    local = p2_guest_stack_pointer;
    vector = local;
    sample = local + 16;
    object_slot = local + 32;
    inverse = local + 40;
    world = local + 64;
    secondary_vector = local + 80;
    secondary_inverse = local + 96;
    character = 0x800ABD68 + (uint32)(sint32)mode * 928;
    w_u16(0x8008A532, 0);
    w_u8(character + 276, 0);
    w_u32(character + 920, 0);
    w_u32(character + 924, 0);
    if (r_u16(r_u32(0x8008A4E4) + 262) == 0 && r_s8(0x8008A43C) > 0 && r_u32(0x8008A130) == 0)
    {
        marker = r_u32(scene_offset + 44);
        w_u32(0x8008A130, 1);
        while (marker != 0)
        {
            if (r_s8(marker + 15) == 17 && (uint32)(sint32)r_s8(0x8008A43C) == r_u32(marker + 68))
            {
                x = r_u32(marker + 36);
                y = r_u32(marker + 40);
                z = r_u32(marker + 44);
                w_u32(vector, x);
                w_u32(vector + 4, y);
                w_u32(vector + 8, z);
                w_u32(vector + 4, y - ((uint32)r_u16(marker + 62) << 5));
                path = r_u32(scene_offset + 40);
                best_distance = 0x7FFFF000;
                while (path != 0)
                {
                    if ((r_u16(path + 14) & 0x10) != 0)
                    {
                        definition = r_u32(path + 36);
                        selected_frame = 0xFFFFFFFF;
                        if (r_u32(definition + 32) != 0)
                        {
                            frame = 0;
                            count = (uint32)(sint32)r_s16(definition + 26);
                            do
                            {
                                p2_func_800230f0(definition, (sint32)frame, sample, 16);
                                x = r_u32(sample) - r_u32(vector);
                                w_u32(sample, x);
                                if (x + 409600 < 0xC8000)
                                {
                                    y = r_u32(sample + 4) - r_u32(vector + 4);
                                    w_u32(sample + 4, y);
                                    if (y + 409600 < 0xC8000)
                                    {
                                        z = r_u32(sample + 8) - r_u32(vector + 8);
                                        w_u32(sample + 8, z);
                                        if (z + 409600 < 0xC8000)
                                        {
                                            x = (uint32)((sint32)x >> 6);
                                            y = (uint32)((sint32)y >> 6);
                                            z = (uint32)((sint32)z >> 6);
                                            distance = x * x + y * y + z * z;
                                            if ((sint32)distance < (sint32)best_distance)
                                            {
                                                best_distance = distance;
                                                selected_frame = frame;
                                            }
                                        }
                                    }
                                }
                                --count;
                                ++frame;
                            } while (count != 0);
                        }
                        if ((sint32)selected_frame >= 0)
                        {
                            search_path = path;
                            search_frame = selected_frame;
                            checkpoint_selected = 1;
                        }
                    }
                    path = r_u32(path);
                }
                if (checkpoint_selected != 0)
                {
                    path = search_path;
                    frame = search_frame;
                }
            }
            marker = r_u32(marker);
        }
    }
    if (path == 0)
    {
        /* A missing checkpoint path uses the ordinary scene spawn */
        frame = (uint32)(sint32)second;
        sample_frame = (sint16)first;
        if (sample_frame != -1)
            path = p2_func_800229fc(scene_offset, sample_frame);
        if (sample_frame == -1 || path == 0)
        {
            path = p2_func_80022ae4(scene_offset);
            if (path == 0)
                path = p2_func_800229fc(scene_offset, 0);
        }
    }
    index = r_u16(character + 706) & 0xF;
    model = r_u32(0x8008B4B8 + index * 4);
    object = r_u32(0x8008A340 + (uint32)(sint32)mode * 4);
    w_u32(object_slot, object);
    if (object != r_u32(model + 232) || (r_u32(0x1F800320) & 2) != 0 || r_u32(0x8008A1A0) != 0)
        p2_func_80055dc0(object_slot, character, r_u16(character + 706) & 0xF);
    model = r_u32(0x8008B4B8 + (r_u16(character + 706) & 0xF) * 4);
    w_u32(character + 780, model);
    flags = r_u16(path + 14);
    w_u32(0x8008A1A0, 0);
    direction = flags & 0x10;
    if ((frame & 0x8000) != 0)
    {
        frame = ~frame;
        if ((flags & 8) != 0)
            path = r_u32(path + 16);
        direction ^= 0x10;
    }
    object = r_u32(object_slot);
    if (direction != (r_u32(object + 28) & 0x10))
    {
        frame = r_u16(scene_offset + 16) - frame - 1;
        if ((r_u16(path + 14) & 8) != 0)
            path = r_u32(path + 16);
        direction ^= 0x10;
    }
    definition = r_u32(path + 36);
    value = (uint32)(sint32)r_s16(definition + 36);
    if (value != 0)
        frame = (value * (uint32)(sint32)(sint16)frame) >> 12;
    w_u8(object + 124, 1);
    w_u8(r_u32(object_slot) + 125, 1);
    w_u8(r_u32(object_slot) + 126, 1);
    w_u8(r_u32(object_slot) + 127, 0);
    object = r_u32(object_slot);
    flags = r_u32(object + 228);
    node = r_u32(object + 156);
    value = r_u32(object + 28);
    model = r_u32(character + 780);
    index = (uint32)(sint32)r_s8(character + 712);
    w_u32(object + 104, r_u32(model + 80 + index * 4));
    w_u32(object + 28, (value & 0xFFFDDBFF) | 0x20000);
    w_u32(object + 228, flags & 0xFC000000);
    w_u8(node + 13, r_u8(node + 13) & 0xFE);
    w_u32(0x8008A114, 0x800B99A0);
    p2_func_80024844(path);
    context = r_u32(0x8008A114);
    w_u32(character + 688, path);
    w_u16(context + 2, frame);
    w_u8(context + 74, 0);
    w_u8(context + 73, 0);
    w_u8(context + 72, 0);
    object = r_u32(object_slot);
    w_u32(object + 132, r_u32(path + 36));
    w_u16(object + 148, 4096);
    context = r_u32(0x8008A114);
    definition = r_u32(r_u32(context + 136) + 36);
    sample_frame = r_s16(context + 2);
    if (direction == 0)
        sample_frame = r_s16(definition + 26) - sample_frame - 1;
    p2_func_80017364(sample_frame, definition + 20, object + 16);
    context = r_u32(0x8008A114);
    object = r_u32(object_slot);
    w_u16(object + 136, r_u16(context + 2));
    if (r_u16(r_u32(0x8008A4E4) + 262) == 0)
    {
        w_u32(character, r_u32(character) | 0x80000000);
        p2_func_80048af0(object, character, 0);
    }
    else
    {
        value = r_u32(character + 784);
        w_u32(character, r_u32(character) & 0x7FFFFFFF);
        if ((sint32)value > 0)
        {
            definition = r_u32(object + 132);
            if (definition != 0 && r_u32(definition + 40) != 0)
            {
                x = r_u32(object + 16);
                y = r_u32(object + 20);
                z = r_u32(object + 24);
                w_u32(vector, x);
                w_u32(vector + 4, y);
                w_u32(vector + 8, z);
                PushMatrix();
                object = r_u32(object_slot);
                matrix = r_u32(r_u32(object + 132) + 40);
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
                p2_sdk_8007ec64(vector, object + 16);
                PopMatrix();
                object = r_u32(object_slot);
                matrix = r_u32(r_u32(object + 132) + 40);
                w_u32(object + 16, r_u32(object + 16) + r_u32(matrix + 20));
                matrix = r_u32(r_u32(object + 132) + 40);
                w_u32(object + 20, r_u32(object + 20) + r_u32(matrix + 24));
                matrix = r_u32(r_u32(object + 132) + 40);
                w_u32(object + 24, r_u32(object + 24) + r_u32(matrix + 28));
            }
            else
            {
                object = r_u32(object_slot);
                x = r_u32(object + 16);
                y = r_u32(object + 20);
                z = r_u32(object + 24);
                w_u32(object + 16, x);
                w_u32(object + 20, y);
                w_u32(object + 24, z);
            }
            object = r_u32(object_slot);
            w_u32(object + 20, r_u32(object + 20) + r_u32(character + 784));
            definition = r_u32(object + 132);
            if (definition != 0 && r_u32(definition + 40) != 0)
            {
                matrix = r_u32(definition + 40);
                w_u32(sample, r_u32(object + 16) - r_u32(matrix + 20));
                matrix = r_u32(r_u32(object + 132) + 40);
                w_u32(sample + 4, r_u32(object + 20) - r_u32(matrix + 24));
                matrix = r_u32(r_u32(object + 132) + 40);
                w_u32(sample + 8, r_u32(object + 24) - r_u32(matrix + 28));
                w_u16(inverse, r_u16(r_u32(r_u32(object + 132) + 40)));
                w_u16(inverse + 2, r_u16(r_u32(r_u32(object + 132) + 40) + 6));
                w_u16(inverse + 4, r_u16(r_u32(r_u32(object + 132) + 40) + 12));
                w_u16(inverse + 6, r_u16(r_u32(r_u32(object + 132) + 40) + 2));
                w_u16(inverse + 8, r_u16(r_u32(r_u32(object + 132) + 40) + 8));
                w_u16(inverse + 10, r_u16(r_u32(r_u32(object + 132) + 40) + 14));
                w_u16(inverse + 12, r_u16(r_u32(r_u32(object + 132) + 40) + 4));
                w_u16(inverse + 14, r_u16(r_u32(r_u32(object + 132) + 40) + 10));
                w_u16(inverse + 16, r_u16(r_u32(r_u32(object + 132) + 40) + 16));
                PushMatrix();
                x = r_u32(inverse);
                y = r_u32(inverse + 4);
                xport_gte_write_control(0, x);
                xport_gte_write_control(1, y);
                x = r_u32(inverse + 8);
                y = r_u32(inverse + 12);
                z = r_u32(inverse + 16);
                xport_gte_write_control(2, x);
                xport_gte_write_control(3, y);
                xport_gte_write_control(4, z);
                object = r_u32(object_slot);
                p2_sdk_8007ec64(sample, object + 16);
                PopMatrix();
            }
            else
            {
                object = r_u32(object_slot);
                x = r_u32(object + 16);
                y = r_u32(object + 20);
                z = r_u32(object + 24);
                w_u32(object + 16, x);
                w_u32(object + 20, y);
                w_u32(object + 24, z);
            }
            object = r_u32(object_slot);
            w_u32(object + 28, r_u32(object + 28) | 0x400);
        }
    }
    object = r_u32(object_slot);
    if ((r_u32(r_u32(object + 132) + 16) & 0x11) != 0)
        w_u32(object + 28, r_u32(object + 28) | 0x400);
    context = r_u32(0x8008A114);
    p2_func_80022c24(path, r_s16(context + 2), context + 4, direction);
    context = r_u32(0x8008A114);
    p2_func_80022b20(path, r_s16(context + 2), context + 16, direction);
    object = r_u32(object_slot);
    w_u32(object + 220, 0xFFFFFFFF);
    w_u32(object + 204, 0xFFFFFFFF);
    w_u32(object + 188, 0xFFFFFFFF);
    w_u32(object + 172, 0xFFFFFFFF);
    p2_func_800401a8(object);
    if (r_s32(character) < 0)
    {
        object = r_u32(object_slot);
        w_u32(object + 28, r_u32(object + 28) | 0x40);
        p2_func_8003db7c(object, 24576, 0, 0);
        object = r_u32(object_slot);
        w_u32(object + 28, r_u32(object + 28) & 0xFFFFFFBF);
        definition = r_u32(object + 132);
        if (definition != 0 && r_u32(definition + 40) != 0)
        {
            w_u16(inverse, r_u16(r_u32(definition + 40)));
            w_u16(inverse + 2, r_u16(r_u32(r_u32(object + 132) + 40) + 6));
            w_u16(inverse + 4, r_u16(r_u32(r_u32(object + 132) + 40) + 12));
            w_u16(inverse + 6, r_u16(r_u32(r_u32(object + 132) + 40) + 2));
            w_u16(inverse + 8, r_u16(r_u32(r_u32(object + 132) + 40) + 8));
            w_u16(inverse + 10, r_u16(r_u32(r_u32(object + 132) + 40) + 14));
            w_u16(inverse + 12, r_u16(r_u32(r_u32(object + 132) + 40) + 4));
            w_u16(inverse + 14, r_u16(r_u32(r_u32(object + 132) + 40) + 10));
            w_u16(inverse + 16, r_u16(r_u32(r_u32(object + 132) + 40) + 16));
            x = r_u32(object + 92);
            y = r_u32(object + 96);
            z = r_u32(object + 100);
            w_u32(sample, x);
            w_u32(sample + 4, y);
            w_u32(sample + 8, z);
            PushMatrix();
            x = r_u32(inverse);
            y = r_u32(inverse + 4);
            xport_gte_write_control(0, x);
            xport_gte_write_control(1, y);
            x = r_u32(inverse + 8);
            y = r_u32(inverse + 12);
            z = r_u32(inverse + 16);
            xport_gte_write_control(2, x);
            xport_gte_write_control(3, y);
            xport_gte_write_control(4, z);
            p2_sdk_8007ec64(sample, vector);
            PopMatrix();
        }
        else
        {
            x = r_u32(object + 92);
            y = r_u32(object + 96);
            z = r_u32(object + 100);
            w_u32(vector, x);
            w_u32(vector + 4, y);
            w_u32(vector + 8, z);
        }
        object = r_u32(object_slot);
        w_u32(object + 16, r_u32(object + 16) + r_u32(vector));
        w_u32(object + 20, r_u32(object + 20) + r_u32(vector + 4));
        w_u32(object + 24, r_u32(object + 24) + r_u32(vector + 8));
        w_u32(object + 100, 0);
        w_u32(object + 96, 0);
        w_u32(object + 92, 0);
        definition = r_u32(object + 132);
        if (definition != 0 && r_u32(definition + 40) != 0)
        {
            x = r_u32(object + 16);
            y = r_u32(object + 20);
            z = r_u32(object + 24);
            w_u32(world, x);
            w_u32(world + 4, y);
            w_u32(world + 8, z);
            PushMatrix();
            object = r_u32(object_slot);
            matrix = r_u32(r_u32(object + 132) + 40);
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
            context = r_u32(0x8008A114);
            p2_sdk_8007ec64(world, context + 16);
            PopMatrix();
            context = r_u32(0x8008A114);
            object = r_u32(object_slot);
            matrix = r_u32(r_u32(object + 132) + 40);
            w_u32(context + 16, r_u32(context + 16) + r_u32(matrix + 20));
            matrix = r_u32(r_u32(object + 132) + 40);
            w_u32(context + 20, r_u32(context + 20) + r_u32(matrix + 24));
            matrix = r_u32(r_u32(object + 132) + 40);
            w_u32(context + 24, r_u32(context + 24) + r_u32(matrix + 28));
        }
        else
        {
            context = r_u32(0x8008A114);
            object = r_u32(object_slot);
            x = r_u32(object + 16);
            y = r_u32(object + 20);
            z = r_u32(object + 24);
            w_u32(context + 16, x);
            w_u32(context + 20, y);
            w_u32(context + 24, z);
        }
        object = r_u32(object_slot);
        context = r_u32(0x8008A114);
        sample_frame = r_s16(object + 136);
        w_u16(context + 2, sample_frame);
        p2_func_800451cc(r_u32(context + 136), sample_frame, context + 4, r_u32(object + 28) & 0x10);
        object = r_u32(object_slot);
        other_model = r_u32(0x8008B4B8 + (1 - (r_u16(character + 706) & 0xF)) * 4);
        other_object = r_u32(other_model + 232);
        w_u8(other_object + 13, r_u8(object + 13));
        x = r_u32(object + 16);
        y = r_u32(object + 20);
        z = r_u32(object + 24);
        w_u32(other_object + 16, x);
        w_u32(other_object + 20, y);
        w_u32(other_object + 24, z);
        w_u32(other_object + 28, r_u32(object + 28));
        w_u32(other_object + 228, r_u32(object + 228));
        x = r_u32(object + 60);
        y = r_u32(object + 64);
        z = r_u32(object + 68);
        w_u32(other_object + 60, x);
        w_u32(other_object + 64, y);
        w_u32(other_object + 68, z);
        x = r_u32(object + 72);
        y = r_u32(object + 76);
        z = r_u32(object + 80);
        w_u32(other_object + 72, x);
        w_u32(other_object + 76, y);
        w_u32(other_object + 80, z);
        x = r_u32(object + 84);
        y = r_u32(object + 88);
        w_u32(other_object + 84, x);
        w_u32(other_object + 88, y);
        x = r_u32(object + 92);
        y = r_u32(object + 96);
        z = r_u32(object + 100);
        w_u32(other_object + 92, x);
        w_u32(other_object + 96, y);
        w_u32(other_object + 100, z);
        p2_func_8006637c((uint8 *)psx_addr(other_object + 108, 12), (const uint8 *)psx_addr(object + 108, 12), 12);
        w_u8(other_object + 124, r_u8(object + 124));
        w_u8(other_object + 125, r_u8(object + 125));
        w_u8(other_object + 126, r_u8(object + 126));
        w_u8(other_object + 127, r_u8(object + 127));
        w_u16(other_object + 128, r_u16(object + 128));
        w_u16(other_object + 130, r_u16(object + 130));
        w_u32(other_object + 132, r_u32(object + 132));
        w_u16(other_object + 136, r_u16(object + 136));
        w_u16(other_object + 148, r_u16(object + 148));
        w_u16(other_object + 150, r_u16(object + 150));
        w_u16(other_object + 152, r_u16(object + 152));
        w_u16(other_object + 154, r_u16(object + 154));
        w_u32(other_object + 144, r_u32(object + 144));
        p2_func_8006637c((uint8 *)psx_addr(other_object + 160, 64), (const uint8 *)psx_addr(object + 160, 64), 64);
        w_u8(other_object + 124, 0);
        w_u8(other_object + 126, 0);
        w_u8(other_object + 127, 0);
        w_u8(other_object + 125, 33);
        object = r_u32(object_slot);
        flags = r_u32(other_object + 28);
        w_u32(other_object + 112, path);
        w_u32(other_object + 116, object);
        value = r_u32(other_model + 80);
        w_u32(other_object + 28, flags | 0x20000);
        w_u32(other_object + 104, value);
        p2_func_800401a8(other_object);
        flags = r_u8(other_object + 13);
        value = r_u32(other_object + 28) & 0xFFFFFFBF;
        w_u8(other_object + 126, 1);
        w_u8(other_object + 13, flags | 0x41);
        w_u32(other_object + 28, value);
        p2_func_8003db7c(other_object, 45056, 0, 0);
        w_u8(other_object + 126, 0);
        definition = r_u32(other_object + 132);
        if (definition != 0 && r_u32(definition + 40) != 0)
        {
            w_u16(secondary_inverse, r_u16(r_u32(definition + 40)));
            w_u16(secondary_inverse + 2, r_u16(r_u32(r_u32(other_object + 132) + 40) + 6));
            w_u16(secondary_inverse + 4, r_u16(r_u32(r_u32(other_object + 132) + 40) + 12));
            w_u16(secondary_inverse + 6, r_u16(r_u32(r_u32(other_object + 132) + 40) + 2));
            w_u16(secondary_inverse + 8, r_u16(r_u32(r_u32(other_object + 132) + 40) + 8));
            w_u16(secondary_inverse + 10, r_u16(r_u32(r_u32(other_object + 132) + 40) + 14));
            w_u16(secondary_inverse + 12, r_u16(r_u32(r_u32(other_object + 132) + 40) + 4));
            w_u16(secondary_inverse + 14, r_u16(r_u32(r_u32(other_object + 132) + 40) + 10));
            w_u16(secondary_inverse + 16, r_u16(r_u32(r_u32(other_object + 132) + 40) + 16));
            x = r_u32(other_object + 92);
            y = r_u32(other_object + 96);
            z = r_u32(other_object + 100);
            w_u32(secondary_vector, x);
            w_u32(secondary_vector + 4, y);
            w_u32(secondary_vector + 8, z);
            PushMatrix();
            x = r_u32(secondary_inverse);
            y = r_u32(secondary_inverse + 4);
            xport_gte_write_control(0, x);
            xport_gte_write_control(1, y);
            x = r_u32(secondary_inverse + 8);
            y = r_u32(secondary_inverse + 12);
            z = r_u32(secondary_inverse + 16);
            xport_gte_write_control(2, x);
            xport_gte_write_control(3, y);
            xport_gte_write_control(4, z);
            p2_sdk_8007ec64(secondary_vector, vector);
            PopMatrix();
        }
        else
        {
            x = r_u32(other_object + 92);
            y = r_u32(other_object + 96);
            z = r_u32(other_object + 100);
            w_u32(vector, x);
            w_u32(vector + 4, y);
            w_u32(vector + 8, z);
        }
        w_u32(other_object + 16, r_u32(other_object + 16) + r_u32(vector));
        w_u32(other_object + 20, r_u32(other_object + 20) + r_u32(vector + 4));
        w_u32(other_object + 24, r_u32(other_object + 24) + r_u32(vector + 8));
        w_u32(vector, 0 - r_u32(other_object + 92));
        w_u32(vector + 4, 0 - r_u32(other_object + 96));
        w_u32(vector + 8, 0 - r_u32(other_object + 100));
        w_u32(other_object + 100, 0);
        w_u32(other_object + 96, 0);
        w_u32(other_object + 92, 0);
        p2_func_80040384(other_object, vector, 4096);
    }
    matrix = r_u32(0x8008A0C8);
    x = r_u32(0x80010180);
    y = r_u32(0x80010184);
    z = r_u32(0x80010188);
    w_u32(matrix, x);
    w_u32(matrix + 4, y);
    w_u32(matrix + 8, z);
    x = r_u32(0x8001018C);
    y = r_u16(0x80010190);
    w_u32(matrix + 12, x);
    w_u16(matrix + 16, y);
    matrix = r_u32(0x8008A0C8);
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
    matrix = r_u32(0x8008A0C8);
    object = r_u32(object_slot);
    w_u32(matrix + 28, 0);
    w_u32(matrix + 24, 0);
    w_u32(matrix + 20, 0);
    w_u8(object + 13, r_u8(object + 13) | 0x41);
    p2_func_80012eb8();
    object = r_u32(object_slot);
    w_u8(object + 13, r_u8(object + 13) & 0xFE);
    value = r_u32(character + 20);
    if ((value & 0xFFFF) != 0)
    {
        index = r_u16(character + 706) & 0xF;
        if (index != 5 && index - 10 >= 2)
        {
            value >>= 16;
            p2_func_80027b1c(r_u32(object_slot), character, value == 0 ? 3 : value - 7);
        }
    }
    if ((r_u32(character + 4) & 0x40) != 0)
    {
        model = r_u32(character + 780);
        value = r_u32(model + 168);
        x = r_u32(model + 88);
        w_u32(model + 88, value);
        model = r_u32(character + 780);
        w_u32(model + 168, x);
        p2_func_80027b1c(r_u32(object_slot), character, 7);
    }
    context = r_u32(0x8008A114);
    value = r_u32(context + 20);
    value += r_s32(character) < 0 ? 0x8000 : 14336;
    w_u32(context + 20, value);
    context = r_u32(0x8008A114);
    w_u8(context, r_u8(context) & 0xFE);
    object = r_u32(object_slot);
    p2_func_80022ec4(r_u32(object + 132), character + 72, (sint32)direction);
    value = r_u32(0x8008A52C);
    w_u32(0x8008A178, 0);
    if ((value & 0x200) == 0 || (r_u16(character + 706) & 0xF) != 0)
    {
        flags = r_u32(character + 4);
        result = flags & 0x120;
        if (result == 0)
            goto finish;
        result = flags | 0x20;
        if ((r_u16(character + 706) & 0xF) == 1)
        {
            w_u32(character + 4, result);
            goto finish;
        }
        p2_func_80027b1c(r_u32(object_slot), character, 6);
    }
    result = p2_func_8005a440(r_u32(object_slot), character);
finish:
    p2_guest_stack_pointer = saved_stack;
    return result;
}
