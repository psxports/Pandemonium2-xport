#include "game.h"

void p2_func_80019cf4(uint32 owner_offset, uint32 node_offset)
{
    uint32 saved_stack = p2_guest_stack_pointer;
    uint32 animation;
    uint32 kind;
    uint32 position_index_offset;
    uint32 position_offset;
    uint32 rotation_index_offset;
    uint32 rotation_offset;
    uint32 track;
    uint32 source;
    uint32 payload;
    uint32 destination;
    uint32 weight;
    uint32 x;
    uint32 y;
    uint32 z;
    uint32 product;
    uint32 index;
    uint32 temporary_offset;
    uint32 flags;
    uint32 active;
    uint32 descriptor;
    uint32 slot;
    uint32 slot_flags;
    uint32 remaining;
    uint32 cursor;
    uint32 points;
    uint32 point;
    sint32 track_index;
    sint32 count;
    sint32 shifted;
    sint32 frame;
    sint32 node_key;
    FUNCTION_MARKER(0x80019CF4, "SLUS_005.78");
    p2_guest_stack_pointer -= 0x70;
    animation = r_u32(owner_offset + 104);
    if (animation == 0)
        goto finish;
    kind = r_u8(node_offset + 12);
    frame = r_s16(owner_offset + 130);
    if (kind == 5)
    {
        position_index_offset = 28;
        position_offset = 16;
        rotation_index_offset = 29;
        rotation_offset = 40;
    }
    else if (kind == 6)
    {
        position_index_offset = 14;
        position_offset = 20;
    }
    else if (kind == 7)
    {
        position_index_offset = 24;
        position_offset = 28;
    }
    else if (kind == 2)
    {
        position_index_offset = 84;
        position_offset = 88;
        rotation_index_offset = 85;
        rotation_offset = 100;
    }
    else
        goto finish;
    track_index = r_s8(node_offset + position_index_offset);
    if (track_index >= 0)
    {
        track = r_u32(animation + 8) + 20 * (uint32)track_index;
        payload = r_u32(track + 12);
        if (payload != 0)
        {
            flags = r_u32(owner_offset + 28);
            destination = node_offset + position_offset;
            if ((flags & 0x10000) != 0)
            {
                weight = 1365 * ((uint32)frame + 1);
                source = payload == 0xFFFFFFFF ? track : payload + 4;
                for (index = 0; index < 3; ++index)
                {
                    x = r_u32(source + 4 * index);
                    y = r_u32(destination + 4 * index);
                    product = (uint32)((sint64)(sint32)(x - y) * (sint32)weight);
                    w_u32(destination + 4 * index, y + (uint32)((sint32)product >> 12));
                }
            }
            else if (payload == 0xFFFFFFFF)
            {
                x = r_u32(track);
                y = r_u32(track + 4);
                z = r_u32(track + 8);
                w_u32(destination, x);
                w_u32(destination + 4, y);
                w_u32(destination + 8, z);
            }
            else
                SampleVectorTrack(frame, track, destination);
        }
    }
    if (kind == 6 || kind == 7)
    {
        track_index = r_s8(node_offset + (kind == 6 ? 16 : 26));
        if (track_index >= 0)
        {
            track = r_u32(animation + 16) + 16 * (uint32)track_index;
            payload = r_u32(track + 12);
            if (payload != 0)
            {
                temporary_offset = p2_guest_stack_pointer + (kind == 6 ? 0x18 : 0x30);
                if (payload == 0xFFFFFFFF)
                {
                    x = r_u32(track);
                    y = r_u32(track + 4);
                    z = r_u32(track + 8);
                    w_u32(temporary_offset, x);
                    w_u32(temporary_offset + 4, y);
                    w_u32(temporary_offset + 8, z);
                }
                else
                    SampleVectorTrack(frame, track, temporary_offset);
                if (kind == 6)
                    w_u32(node_offset + 32, r_u32(temporary_offset));
                else
                {
                    x = r_u32(temporary_offset);
                    w_u16(node_offset + 40, (uint32)((sint32)x >> 5));
                    y = r_u32(temporary_offset + 4);
                    w_u16(node_offset + 42, (uint32)((sint32)y >> 5));
                }
            }
        }
        goto finish;
    }
    track_index = r_s8(node_offset + rotation_index_offset);
    if (track_index >= 0)
    {
        track = r_u32(animation + 12) + 16 * (uint32)track_index;
        payload = r_u32(track + 12);
        if (payload != 0)
        {
            shifted = r_u8(0x8008A45C);
            if (shifted != 0)
            {
                shifted += track_index;
                count = r_s8(animation + 1);
                if (count == 0)
                    xport_mips_break(7);
                track_index = count == 0 ? shifted : shifted % count;
                track = r_u32(animation + 12) + 16 * (uint32)track_index;
            }
            flags = r_u32(owner_offset + 28);
            if ((flags & 0x10000) != 0)
                p2_func_80019924(frame, track, node_offset + rotation_offset);
            else if (r_u32(track + 12) == 0xFFFFFFFF)
            {
                x = r_u32(track);
                y = r_u32(track + 4);
                w_u32(node_offset + rotation_offset, x);
                w_u32(node_offset + rotation_offset + 4, y);
            }
            else
                p2_func_80017938(frame, track, node_offset + rotation_offset);
        }
    }
    if (kind != 5 || (r_u32(owner_offset + 28) & 0x20) == 0)
        goto finish;
    descriptor = r_u32(owner_offset + 140);
    if (descriptor == 0)
        goto finish;
    active = (uint32)(sint32)r_s8(animation + 3);
    node_key = (sint8)r_u8(node_offset + 28);
    if (active != 0)
    {
        track = r_u32(animation + 20);
        for (;;)
        {
            if ((sint32)r_u8(track + 3) == node_key)
            {
                p2_func_80019684(frame, track, owner_offset, node_offset + 32);
                break;
            }
            --active;
            track += 24;
            if (active == 0)
                break;
        }
    }
    if (active == 0 && r_u16(descriptor + 2) == 0)
        goto finish;
    remaining = (uint32)(sint32)r_s16(descriptor);
    slot = descriptor + 4;
    cursor = descriptor + 8;
    for (;;)
    {
        if ((sint32)r_u16(cursor - 2) == node_key)
            break;
        cursor += 8;
        --remaining;
        slot += 8;
        if (remaining == 0)
            goto finish;
    }
    slot_flags = r_u16(slot);
    if (active != 0)
    {
        if ((slot_flags & 1) == 0)
        {
            w_u16(slot, slot_flags | 1);
            w_u16(descriptor + 2, r_u16(descriptor + 2) + 1);
        }
    }
    else if ((slot_flags & 1) != 0)
    {
        w_u16(slot, slot_flags & 0xFFFE);
        w_u16(descriptor + 2, r_u16(descriptor + 2) - 1);
        point = (uint32)(sint32)r_s16(node_offset + 32);
        destination = r_u32(owner_offset + 40) + 16 * point;
        source = r_u32(cursor);
        points = (uint32)(sint32)r_s16(node_offset + 34);
        do
        {
            w_u16(destination + 12, 0);
            w_u16(destination + 10, 0);
            w_u16(destination + 8, 0);
            x = r_u32(source);
            y = r_u32(source + 4);
            w_u32(destination, x);
            w_u32(destination + 4, y);
            source += 8;
            destination += 16;
            --points;
        } while (points != 0);
        p2_func_80019260(owner_offset, node_offset + 32);
    }
finish:
    p2_guest_stack_pointer = saved_stack;
}

uint32 p2_func_80019924(sint32 frame, uint32 track_offset, uint32 destination_offset)
{
    uint32 payload;
    uint32 source;
    uint32 weight;
    uint32 source_weight;
    uint32 destination_weight;
    uint32 sum;
    uint32 product_low;
    uint32 first;
    uint32 second;
    uint32 result = 0;
    uint32 index;
    sint32 current;
    sint32 next;
    sint32 target;
    sint32 dot;
    sint32 angle;
    sint32 part;
    sint32 divisor;
    sint32 reciprocal;
    sint64 product;
    uint32 spherical;
    FUNCTION_MARKER(0x80019924, "SLUS_005.78");
    payload = r_u32(track_offset + 12);
    weight = 0x4000 - 5461 * ((uint32)frame + 1);
    source = payload == 0xFFFFFFFF ? track_offset : payload + 8;
    current = r_s16(destination_offset + 2);
    target = r_s16(source + 2);
    sum = (uint32)(current * target);
    current = r_s16(destination_offset + 4);
    target = r_s16(source + 4);
    sum += (uint32)(current * target);
    current = r_s16(destination_offset + 6);
    target = r_s16(source + 6);
    sum += (uint32)(current * target);
    current = r_s16(destination_offset);
    target = r_s16(source);
    sum += (uint32)(current * target);
    dot = (sint32)sum >> 14;
    spherical = (uint32)dot + 16368 < 0x7FE0;
    if (spherical != 0)
    {
        angle = AcosAngle(dot < 0 ? -dot : dot, 0x4000);
        product_low = (uint32)((sint64)(sint32)weight * (sint16)angle);
        part = (sint32)product_low >> 14;
        divisor = SinQ14((sint16)angle);
        if (divisor == 0)
            xport_mips_break(7);
        reciprocal = divisor == 0 ? -1 : 0x10000000 / divisor;
        target = SinQ14((sint16)((uint32)angle - (uint32)part));
        product = (sint64)target * reciprocal;
        destination_weight = (uint32)((uint64)product >> 14);
        target = SinQ14((sint16)part);
        product = (sint64)target * reciprocal;
        source_weight = (uint32)((uint64)product >> 14);
        if (dot < 0)
            source_weight = 0 - source_weight;
    }
    else
    {
        destination_weight = 0x4000 - weight;
        source_weight = dot < 0 ? 0 - weight : weight;
    }
    current = r_s16(destination_offset);
    for (index = 0; index < 4; ++index)
    {
        product = (sint64)current * (sint32)destination_weight;
        first = spherical != 0 ? (uint32)((uint64)product >> 14) : (uint32)((sint32)(uint32)product >> 14);
        target = r_s16(source + 2 * index);
        product = (sint64)target * (sint32)source_weight;
        second = spherical != 0 ? (uint32)((uint64)product >> 14) : (uint32)((sint32)(uint32)product >> 14);
        if (index < 3)
            next = r_s16(destination_offset + 2 * index + 2);
        result = first + second;
        w_u16(destination_offset + 2 * index, result);
        if (index < 3)
            current = next;
    }
    return spherical != 0 ? second : result;
}

void p2_func_80011980(uint32 owner_offset, uint32 span_offset)
{
    sint32 polygon_count;
    uint32 matrix;
    uint32 target;
    uint32 output;
    uint32 previous_count;
    uint32 point;
    uint32 remaining;
    uint32 vertex_offset;
    uint32 polygon_offset;
    uint32 lighting;
    uint32 index;
    uint32 value;
    uint32 x;
    uint32 y;
    uint32 z;
    uint32 flags;
    const uint32 transpose_offsets[9] = {0, 6, 12, 2, 8, 14, 4, 10, 16};
    FUNCTION_MARKER(0x80011980, "SLUS_005.78");
    polygon_count = r_s16(span_offset + 6);
    if (polygon_count == 0)
        return;
    matrix = r_u32(0x8008A0C8);
    w_u32(0x8008A0C8, matrix + 32);
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
    p2_sdk_8007ec64(matrix + 20, matrix + 52);
    matrix = r_u32(0x8008A0C8);
    value = r_u32(0x1F800278);
    x = r_u32(matrix + 20);
    w_u32(matrix + 20, x + value);
    y = r_u32(matrix + 24);
    value = r_u32(0x1F80027C);
    w_u32(matrix + 24, y + value);
    z = r_u32(matrix + 28);
    value = r_u32(0x1F800280);
    w_u32(matrix + 28, z + value);
    target = p2_sdk_8007ea34(matrix - 32, matrix);
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
    previous_count = render_view.projection_count;
    output = render_view.projection_offset + 8 * previous_count;
    render_view.projection_offset = output;
    matrix = r_u32(0x8008A0C8);
    x = r_u32(matrix + 20);
    y = r_u32(matrix + 24);
    z = r_u32(matrix + 28);
    xport_gte_write_control(5, (uint32)((sint32)x >> 5));
    xport_gte_write_control(6, (uint32)((sint32)y >> 5));
    xport_gte_write_control(7, (uint32)((sint32)z >> 5));
    remaining = (uint32)(sint32)r_s16(span_offset + 2);
    point = (uint32)(sint32)r_s16(span_offset);
    vertex_offset = r_u32(owner_offset + 40) + 16 * point;
    while ((sint32)(remaining - 3) >= 0)
    {
        xport_gte_write_data(0, r_u32(vertex_offset));
        xport_gte_write_data(1, r_u32(vertex_offset + 4));
        xport_gte_write_data(2, r_u32(vertex_offset + 16));
        xport_gte_write_data(3, r_u32(vertex_offset + 20));
        xport_gte_write_data(4, r_u32(vertex_offset + 32));
        xport_gte_write_data(5, r_u32(vertex_offset + 36));
        xport_gte_execute(0x280030);
        w_u32(output, xport_gte_read_data(12));
        w_u32(output + 8, xport_gte_read_data(13));
        w_u32(output + 16, xport_gte_read_data(14));
        w_u32(output + 4, xport_gte_read_data(17));
        w_u32(output + 12, xport_gte_read_data(18));
        w_u32(output + 20, xport_gte_read_data(19));
        vertex_offset += 48;
        remaining -= 3;
        output += 24;
    }
    while (remaining != 0)
    {
        xport_gte_write_data(0, r_u32(vertex_offset));
        xport_gte_write_data(1, r_u32(vertex_offset + 4));
        xport_gte_execute(0x180001);
        w_u32(output, xport_gte_read_data(14));
        w_u32(output + 4, xport_gte_read_data(19));
        vertex_offset += 16;
        output += 8;
        --remaining;
    }
    point = r_u16(span_offset);
    output = render_view.projection_offset;
    render_view.projection_count_aux = (uint16)(point);
    render_view.projection_count = (uint16)(point);
    render_view.projection_offset = output - 8 * point;
    flags = game_flow.transition_flags;
    matrix = r_u32(0x8008A0C8);
    for (index = 0; index < 8; ++index)
        w_u16(0x1F800244 + 2 * index, r_u16(matrix + transpose_offsets[index]));
    value = r_u16(matrix + 16);
    game_flow.transition_flags = flags & 0xFFFFFFFE;
    w_u16(0x1F800254, value);
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
    p2_sdk_8007ec64(matrix + 20, 0x1F800258);
    x = r_u32(0x1F800258);
    y = r_u32(0x1F80025C);
    z = r_u32(0x1F800260);
    w_u32(0x1F800258, 0 - x);
    w_u32(0x1F80025C, 0 - y);
    w_u32(0x1F800260, 0 - z);
    lighting = r_u32(0x1F8002AC);
    if (lighting != 0)
    {
        target = r_u32(0x8008A4E8);
        value = r_u16(owner_offset + 48);
        point = (uint32)(sint32)r_s16(span_offset + 4);
        lighting = r_u32(target + 92) + 4 * value + 16 * point;
        w_u32(0x1F8002AC, lighting);
        if ((r_u32(owner_offset + 28) & 1) != 0)
        {
            point = (uint32)(sint32)r_s16(span_offset + 4);
            vertex_offset = r_u32(owner_offset + 40);
            polygon_offset = r_u32(owner_offset + 36) + 24 * point;
            p2_func_800615c4(vertex_offset, polygon_offset, polygon_count);
        }
        else
        {
            point = (uint32)(sint32)r_s16(span_offset + 4);
            vertex_offset = r_u32(owner_offset + 40);
            polygon_offset = r_u32(owner_offset + 36) + 24 * point;
            p2_func_80061f50(vertex_offset, polygon_offset, polygon_count);
        }
    }
    else
    {
        flags = game_flow.transition_flags;
        point = (uint32)(sint32)r_s16(span_offset + 4);
        vertex_offset = r_u32(owner_offset + 40);
        polygon_offset = r_u32(owner_offset + 36) + 24 * point;
        if ((flags & 0x20) != 0)
            p2_func_80062e3c(vertex_offset, polygon_offset, polygon_count);
        else
            p2_func_800631f0(vertex_offset, polygon_offset, polygon_count);
    }
    matrix = r_u32(0x8008A0C8) - 32;
    w_u32(0x8008A0C8, matrix);
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

void p2_func_800631f0(uint32 vertices_offset, uint32 polygons_offset, sint32 count)
{
    uint32 saved_material;
    uint32 normal_offset;
    uint32 material;
    uint32 material_flags;
    uint32 flags;
    uint32 first_index;
    uint32 second_index;
    uint32 third_index;
    uint32 fourth_index;
    uint32 screen_base;
    uint32 screen0;
    uint32 screen1;
    uint32 screen2;
    uint32 screen3;
    uint32 vertex0;
    uint32 vertex1;
    uint32 vertex2;
    uint32 vertex3;
    uint32 depth;
    uint32 far_distance;
    uint32 sum;
    uint32 product_low;
    uint32 extra;
    uint32 packet;
    uint32 bucket_offset;
    uint32 link;
    uint32 kind;
    uint32 tag;
    uint32 context;
    uint32 end;
    uint32 start;
    uint32 remaining = (uint32)count;
    sint32 coordinate;
    sint32 coefficient;
    sint32 surface;
    sint32 bucket;
    sint32 distance;
    sint32 fade;
    sint64 product;
    FUNCTION_MARKER(0x800631F0, "SLUS_005.78");
    flags = game_flow.transition_flags;
    saved_material = r_u32(0x1F8002A8);
    if ((flags & 4) != 0 && (r_u32(r_u32(polygons_offset)) & 0x40000) != 0)
    {
        if (player_interaction.flags != 0)
            return;
        w_u32(0x1F8002A8, resource_groups[27].material_offset);
    }
    p2_func_8002e84c(0);
    xport_gte_write_data(8, 0);
    normal_offset = polygons_offset + 16;
    do
    {
        packet = render_commands.packet_offset;
        end = render_commands.packet_limit;
        if (packet >= end)
            return;
        coordinate = r_s32(0x1F800258);
        coefficient = r_s16(normal_offset);
        sum = (uint32)((sint64)(coordinate >> 6) * coefficient);
        coordinate = r_s32(0x1F80025C);
        coefficient = r_s16(normal_offset + 2);
        sum += (uint32)((sint64)(coordinate >> 6) * coefficient);
        coordinate = r_s32(0x1F800260);
        coefficient = r_s16(normal_offset + 4);
        sum += (uint32)((sint64)(coordinate >> 6) * coefficient);
        depth = r_u32(normal_offset - 4);
        surface = (sint32)((uint32)((sint32)sum >> 6) + depth);
        if (surface <= 0)
            goto next_polygon;
        first_index = r_u16(normal_offset - 12);
        screen_base = render_view.projection_offset;
        second_index = r_u16(normal_offset - 10);
        third_index = r_u16(normal_offset - 8);
        fourth_index = r_u16(normal_offset - 6);
        screen0 = screen_base + 8 * first_index;
        screen1 = screen_base + 8 * second_index;
        screen2 = screen_base + 8 * third_index;
        screen3 = screen_base + 8 * fourth_index;
        vertex0 = vertices_offset + 16 * first_index;
        vertex1 = vertices_offset + 16 * second_index;
        vertex2 = vertices_offset + 16 * third_index;
        vertex3 = vertices_offset + 16 * fourth_index;
        if (!(r_u16(screen0) < 320 || r_u16(screen1) < 320 || r_u16(screen2) < 320 || r_u16(screen3) < 320))
            goto next_polygon;
        if (!(r_u16(screen0 + 2) < 240 || r_u16(screen1 + 2) < 240 || r_u16(screen2 + 2) < 240 || r_u16(screen3 + 2) < 240))
            goto next_polygon;
        depth = r_u32(screen0 + 4);
        sum = r_u32(screen1 + 4);
        depth += sum;
        sum = r_u32(screen2 + 4);
        depth += sum;
        sum = r_u32(screen3 + 4);
        depth += sum;
        far_distance = render_view.far_distance;
        if ((sint32)(depth << 1) >= (sint32)far_distance)
            goto next_polygon;
        extra = 0;
        material = r_u32(0x1F8002A8);
        if (material == 0)
            material = r_u32(polygons_offset);
        material_flags = r_u32(material);
        if ((material_flags & 1) != 0 && surface <= 0x3FFFF800)
            extra = 0x80000000;
        coefficient = r_s16(normal_offset + 6);
        coordinate = r_s16(0x1F8002A6);
        sum = render_view.fog_scale;
        product_low = (uint32)((sint64)(sint32)((uint32)((sint32)depth >> 2) + (uint32)(coefficient >> 1) + (uint32)coordinate) * (sint32)sum);
        bucket = (sint32)product_low >> 19;
        if ((uint32)bucket >= 1024)
            bucket = bucket < 0 ? 0 : 1023;
        p2_func_8002ede4(vertex0, (material_flags >> 24) | extra);
        xport_gte_write_data(6, r_u32(material + 16));
        context = r_u32(0x8008A4E8);
        end = r_u32(context + 152);
        if (end != 0)
        {
            far_distance = render_view.far_distance;
            product = (sint64)(sint32)far_distance * (sint32)((uint32)bucket << 2);
            distance = (sint32)((uint64)product >> 12);
            start = r_u32(context + 148);
            if ((sint32)start < distance)
            {
                if (distance >= (sint32)end)
                    goto next_polygon;
                sum = (uint32)distance - start;
                coefficient = ((sint32)render_view.depth_scale);
                product = (sint64)coefficient * (sint32)sum;
                fade = (sint32)((uint64)product >> 12) >> 8;
                xport_gte_write_data(8, (uint32)fade);
            }
            else
                xport_gte_write_data(8, 0);
        }
        else
            xport_gte_write_data(8, 0);
        kind = material_flags & 6;
        if (kind == 6 || kind == 2)
        {
            xport_gte_write_data(0, r_u32(vertex0 + 8));
            xport_gte_write_data(1, r_u32(vertex0 + 12));
            xport_gte_write_data(2, r_u32(vertex1 + 8));
            xport_gte_write_data(3, r_u32(vertex1 + 12));
            xport_gte_write_data(4, r_u32(vertex3 + 8));
            xport_gte_write_data(5, r_u32(vertex3 + 12));
            xport_gte_execute(0xF80416);
            tag = kind == 6 ? 0x0C000000 : 0x08000000;
        }
        else
        {
            xport_gte_write_data(0, r_u32(normal_offset));
            xport_gte_write_data(1, r_u32(normal_offset + 4));
            xport_gte_execute(0xE80413);
            tag = kind == 4 ? 0x09000000 : 0x05000000;
        }
        bucket_offset = render_commands.ordering_table_offset + 4 * (uint32)bucket;
        link = r_u32(bucket_offset);
        packet = render_commands.packet_offset;
        w_u32(packet, (link & 0xFFFFFF) | tag);
        link = r_u32(bucket_offset);
        w_u32(bucket_offset, (link & 0xFF000000) | (packet & 0xFFFFFF));
        if (kind == 6)
        {
            p2_func_800612c0(material, render_commands.packet_offset + 12, 3);
            packet = render_commands.packet_offset;
            w_u32(packet + 4, xport_gte_read_data(20));
            w_u32(packet + 16, xport_gte_read_data(21));
            w_u32(packet + 28, xport_gte_read_data(22));
            xport_gte_write_data(0, r_u32(vertex2 + 8));
            xport_gte_write_data(1, r_u32(vertex2 + 12));
            xport_gte_execute(0xE80413);
            packet = render_commands.packet_offset;
            w_u32(packet + 8, r_u32(screen0));
            w_u32(packet + 20, r_u32(screen1));
            w_u32(packet + 32, r_u32(screen3));
            w_u32(packet + 44, r_u32(screen2));
            w_u32(packet + 40, xport_gte_read_data(22));
            render_commands.packet_offset = (render_commands.packet_offset + 52);
        }
        else if (kind == 4)
        {
            packet = render_commands.packet_offset;
            p2_func_800612c0(material, packet + 12, 2);
            w_u32(packet + 8, r_u32(screen0));
            w_u32(packet + 16, r_u32(screen1));
            w_u32(packet + 24, r_u32(screen3));
            w_u32(packet + 32, r_u32(screen2));
            w_u32(packet + 4, xport_gte_read_data(22));
            render_commands.packet_offset = (packet + 40);
        }
        else
        {
            packet = render_commands.packet_offset;
            if ((material_flags & 0x80) != 0)
            {
                link = r_u32(packet);
                w_u32(packet + 12, (link & 0xFFFFFF) | tag);
                w_u32(packet, ((packet + 12) & 0xFFFFFF) | 0x02000000);
                w_u32(packet + 4, (material_flags & 0x60) | 0xE1000000);
                w_u32(packet + 8, 0);
                packet += 12;
            }
            if (kind == 2)
            {
                w_u32(packet + 4, xport_gte_read_data(20));
                w_u32(packet + 12, xport_gte_read_data(21));
                w_u32(packet + 20, xport_gte_read_data(22));
                xport_gte_write_data(0, r_u32(vertex2 + 8));
                xport_gte_write_data(1, r_u32(vertex2 + 12));
                xport_gte_execute(0xE80413);
                w_u32(packet + 8, r_u32(screen0));
                w_u32(packet + 16, r_u32(screen1));
                w_u32(packet + 24, r_u32(screen3));
                w_u32(packet + 32, r_u32(screen2));
                w_u32(packet + 28, xport_gte_read_data(22));
                render_commands.packet_offset = (packet + 36);
            }
            else
            {
                w_u32(packet + 4, xport_gte_read_data(22));
                w_u32(packet + 8, r_u32(screen0));
                w_u32(packet + 12, r_u32(screen1));
                w_u32(packet + 16, r_u32(screen3));
                w_u32(packet + 20, r_u32(screen2));
                render_commands.packet_offset = (packet + 24);
            }
        }
    next_polygon:
        polygons_offset += 24;
        normal_offset += 24;
        --remaining;
    } while (remaining != 0);
    w_u32(0x1F8002A8, saved_material);
}

uint32 p2_func_8002e84c(uint32 mode)
{
    FUNCTION_MARKER(0x8002E84C, "SLUS_005.78");
    uint32 saved_stack = p2_guest_stack_pointer;
    uint32 vector_offset;
    uint32 matrix;
    uint32 light;
    uint32 output = 0x1F800350;
    uint32 table;
    uint32 result;
    uint32 i;
    uint32 x, y, z;
    p2_guest_stack_pointer -= 0x38;
    vector_offset = p2_guest_stack_pointer + 0x10;
    if (r_u32(0x8008A328) != 0)
    {
        p2_func_8002e684();
        w_u32(0x8008A328, 0);
    }
    if (r_u8(0x1F8002A4) == 0)
    {
        matrix = r_u32(0x8008A0C8);
        for (i = 0; i < 9; ++i)
            w_u16(0x1F800364 + i * 2, r_u16(matrix - 32 + (i % 3) * 6 + (i / 3) * 2));
        for (i = 0; i < 5; ++i)
            xport_gte_write_control(i, r_u32(0x1F800364 + i * 4));
        light = r_u32(r_u32(0x8008A4E8) + 84);
        while (light != 0 && (r_u8(light + 14) & 2) == 0)
        {
            if ((r_u8(light + 13) & 1) == 0)
            {
                xport_gte_write_data(0, r_u32(light + 32));
                xport_gte_write_data(1, r_u32(light + 36));
                xport_gte_execute(0x486012);
                w_u32(0x1F800390, xport_gte_read_data(25));
                w_u32(0x1F800394, xport_gte_read_data(26));
                w_u32(0x1F800398, xport_gte_read_data(27));
                w_u16(output, r_u32(0x1F800390));
                w_u16(output + 2, r_u32(0x1F800394));
                w_u16(output + 4, r_u32(0x1F800398));
                output += 6;
            }
            light = r_u32(light);
        }
        table = 0x1F800324;
        light = r_u32(table);
        while (light != 0)
        {
            matrix = r_u32(0x8008A0C8);
            w_u32(vector_offset, r_u32(light + 32) - r_u32(matrix - 12));
            w_u32(vector_offset + 4, r_u32(light + 36) - r_u32(matrix - 8));
            w_u32(vector_offset + 8, r_u32(light + 40) - r_u32(matrix - 4));
            if ((game_flow.transition_flags & 0x200) != 0)
            {
                w_u32(light + 56, r_u32(light + 32) - r_u32(matrix + 52));
                w_u32(light + 60, r_u32(light + 36) - r_u32(matrix + 56));
                w_u32(light + 64, r_u32(light + 40) - r_u32(matrix + 60));
                x = r_u32(light + 56);
                y = r_u32(light + 60);
                z = r_u32(light + 64);
                w_u32(light + 56, (sint32)x >> 5);
                w_u32(light + 60, (sint32)y >> 5);
                w_u32(light + 64, (sint32)z >> 5);
            }
            else
            {
                if (table == 0x1F800324)
                {
                    p2_sdk_8007ec64(matrix - 12, 0x1F800378);
                    x = r_u32(0x1F800378);
                    y = r_u32(0x1F80037C);
                    z = r_u32(0x1F800380);
                    w_u32(0x1F800378, 0u - x);
                    w_u32(0x1F80037C, 0u - y);
                    w_u32(0x1F800380, 0u - z);
                }
                p2_sdk_8007ec64(light + 32, vector_offset);
                x = r_u32(vector_offset);
                x += r_u32(0x1F800378);
                y = r_u32(0x1F80037C);
                z = r_u32(0x1F800380);
                y += r_u32(vector_offset + 4);
                z += r_u32(vector_offset + 8);
                w_u32(vector_offset, x);
                w_u32(vector_offset + 4, y);
                w_u32(vector_offset + 8, z);
                w_u32(light + 56, (sint32)x >> 5);
                w_u32(light + 60, r_s32(vector_offset + 4) >> 5);
                w_u32(light + 64, r_s32(vector_offset + 8) >> 5);
            }
            while (r_u32(vector_offset) + 26754u > 0xD103u || r_u32(vector_offset + 4) + 26754u > 0xD103u || r_u32(vector_offset + 8) + 26754u > 0xD103u)
            {
                x = (uint32)(r_s32(vector_offset) >> 1);
                y = (uint32)(r_s32(vector_offset + 4) >> 1);
                z = (uint32)(r_s32(vector_offset + 8) >> 1);
                w_u32(vector_offset, x);
                w_u32(vector_offset + 4, y);
                w_u32(vector_offset + 8, z);
            }
            VectorNormalS((VECTOR *)psx_addr(vector_offset, 12), (SVECTOR *)psx_addr(output, 6));
            table += 4;
            light = r_u32(table);
            output += 6;
        }
        matrix = r_u32(0x8008A0C8);
        for (i = 0; i < 5; ++i)
            xport_gte_write_control(i, r_u32(matrix + i * 4));
    }
    result = 0x80000000;
    if (mode != 0)
    {
        uint32 flags = r_u32(0x1F800334);
        uint16 values[9];
        for (i = 0; i < 9; ++i)
            values[i] = r_u16(0x1F800350 + i * 2);
        w_u32(0x1F800334, flags | 0x80000000);
        for (i = 0; i < 9; ++i)
            w_u16(0x1F800350 + i * 2, 0u - values[i]);
    }
    else
    {
        result = r_u32(0x1F800334) & 0x7FFFFFFF;
        w_u32(0x1F800334, result);
    }
    for (i = 0; i < 5; ++i)
        xport_gte_write_control(8 + i, r_u32(0x1F800350 + i * 4));
    p2_guest_stack_pointer = saved_stack;
    return result;
}

uint32 p2_func_8002e684(void)
{
    FUNCTION_MARKER(0x8002E684, "SLUS_005.78");
    uint32 table = 0x1F800324;
    uint32 output = 0x1F800350;
    uint32 light;
    uint32 result;
    if (r_u8(0x1F8002A4) == 0)
    {
        light = r_u32(r_u32(0x8008A4E8) + 84);
        while (light != 0)
        {
            if ((r_u8(light + 13) & 1) == 0)
            {
                if ((r_u8(light + 14) & 2) != 0)
                {
                    uint32 radius = (uint32)r_s16(0x8008A32C) + r_u32(light + 80);
                    uint32 extent = radius << 5;
                    uint32 origin = r_u32(0x8008A328);
                    uint32 dx = r_u32(light + 32) - r_u32(origin);
                    uint32 dy = r_u32(light + 36) - r_u32(origin + 4);
                    uint32 dz = r_u32(light + 40) - r_u32(origin + 8);
                    sint32 sx, sy, sz;
                    uint32 distance;
                    sint32 limit;
                    if (dx + extent > (radius << 6) || dy + extent > (radius << 6) || dz + extent > (radius << 6))
                        goto next_light;
                    sx = (sint32)dx >> 6;
                    sy = (sint32)dy >> 6;
                    sz = (sint32)dz >> 6;
                    distance = (uint32)((sint64)sx * sx) + (uint32)((sint64)sy * sy) + (uint32)((sint64)sz * sz);
                    limit = (sint32)((uint64)((sint64)(sint32)extent * (sint32)extent) >> 12);
                    if (limit < (sint32)distance)
                        goto next_light;
                    w_u32(table, light);
                    table += 4;
                }
                output += 6;
                if (output == 0x1F800362)
                    break;
            }
        next_light:
            light = r_u32(light);
        }
    }
    result = p2_func_80066358(output, 0, 0x1F800362 - output);
    w_u32(table, 0);
    return result;
}

uint32 p2_func_80019684(sint32 frame, uint32 track_offset, uint32 owner_offset, uint32 span_offset)
{
    FUNCTION_MARKER(0x80019684, "SLUS_005.78");
    uint32 saved_stack = p2_guest_stack_pointer;
    uint32 context;
    uint32 previous, next, frame_value, end_value;
    uint32 source, destination, remaining, result;
    uint32 first, second;
    p2_guest_stack_pointer -= 0x40;
    context = p2_guest_stack_pointer + 0x18;
    if ((r_u32(owner_offset + 28) & 0x200) != 0 && (r_u8(track_offset + 2) & 1) != 0)
    {
        p2_guest_stack_pointer = saved_stack;
        return 1;
    }
    w_u32(context, owner_offset);
    w_u32(context + 4, span_offset);
    next = r_u32(track_offset + 12);
    previous = next + ((uint32)r_u8(track_offset) << 3) - 8;
    end_value = r_u32(previous);
    frame_value = ((uint32)frame << 20) | 0xFFFFF;
    next += 8;
    if (frame_value < end_value)
    {
        do
        {
            end_value = r_u32(next);
            next += 8;
        } while (frame_value >= end_value);
        next -= 8;
        previous = next - 8;
        goto interpolate;
    }
    next -= 8;
    if (frame_value == (end_value | 0xFFFFF))
    {
        uint32 stride = (uint32)r_s16(track_offset + 22);
        uint32 index = r_u32(previous + 4);
        destination = r_u32(owner_offset + 40) + ((uint32)r_s16(span_offset) << 4);
        remaining = (uint32)r_s16(span_offset + 2);
        source = r_u32(track_offset + 16) + ((index * stride) << 3);
        goto copy_vertices;
    }
    end_value = ((uint32)r_s16(track_offset + 4) << 20) | 0xFFFFF;
    if (frame_value < end_value)
        goto interpolate;
    {
        uint32 key = ((r_u8(track_offset + 2) & 3) == 3) ? next : previous;
        uint32 stride = (uint32)r_s16(track_offset + 22);
        uint32 index = r_u32(key + 4);
        source = r_u32(track_offset + 16) + ((index * stride) << 3);
    }
    w_u32(context + 8, source);
    span_offset = r_u32(context + 4);
    owner_offset = r_u32(context);
    destination = r_u32(owner_offset + 40) + ((uint32)r_s16(span_offset) << 4);
    source = r_u32(context + 8);
    remaining = (uint32)r_s16(span_offset + 2);
copy_vertices:
    do
    {
        w_u16(destination + 12, 0);
        w_u16(destination + 10, 0);
        w_u16(destination + 8, 0);
        first = r_u32(source);
        second = r_u32(source + 4);
        w_u32(destination, first);
        w_u32(destination + 4, second);
        source += 8;
        destination += 16;
        --remaining;
    } while (remaining != 0);
    result = p2_func_80019260(owner_offset, span_offset);
    p2_guest_stack_pointer = saved_stack;
    return result;
interpolate:
{
    uint32 keys = r_u32(track_offset + 12);
    uint32 value = InterpolateKeyTime(frame_value, end_value, r_u32(previous), r_u32(next), keys);
    uint32 stride, index;
    w_u32(context + 16, 4096u - (uint32)((sint32)value >> 8));
    stride = (uint32)r_s16(track_offset + 22);
    index = r_u32(previous + 4);
    w_u32(context + 8, r_u32(track_offset + 16) + ((index * stride) << 3));
    stride = (uint32)r_s16(track_offset + 22);
    index = r_u32(next + 4);
    w_u32(context + 12, r_u32(track_offset + 16) + ((index * stride) << 3));
    result = p2_func_8001bc44(context);
}
    p2_guest_stack_pointer = saved_stack;
    return result;
}



uint32 p2_func_8001bc44(uint32 context_offset)
{
    FUNCTION_MARKER(0x8001BC44, "SLUS_005.78");
    uint32 source0 = r_u32(context_offset + 8);
    uint32 span = r_u32(context_offset + 4);
    uint32 owner = r_u32(context_offset);
    uint32 source1 = r_u32(context_offset + 12);
    uint32 weight = r_u32(context_offset + 16);
    uint32 index = (uint32)r_s16(span);
    uint32 destination = r_u32(owner + 40);
    uint32 inverse = 4096u - weight;
    uint32 remaining = (uint32)r_s16(span + 2);
    uint32 x, y, z;
    destination += index << 4;
    do
    {
        w_u16(destination + 12, 0);
        w_u16(destination + 10, 0);
        w_u16(destination + 8, 0);
        xport_gte_write_data(8, weight);
        x = r_u16(source0);
        y = r_u16(source0 + 2);
        z = r_u16(source0 + 4);
        xport_gte_write_data(9, x);
        xport_gte_write_data(10, y);
        xport_gte_write_data(11, z);
        xport_gte_execute(0x198003D);
        xport_gte_write_data(8, inverse);
        x = r_u16(source1);
        y = r_u16(source1 + 2);
        z = r_u16(source1 + 4);
        xport_gte_write_data(9, x);
        xport_gte_write_data(10, y);
        xport_gte_write_data(11, z);
        xport_gte_execute(0x1A8003E);
        x = xport_gte_read_data(9);
        y = xport_gte_read_data(10);
        z = xport_gte_read_data(11);
        w_u16(destination, x);
        w_u16(destination + 2, y);
        w_u16(destination + 4, z);
        destination += 16;
        source0 += 8;
        --remaining;
        source1 += 8;
    } while (remaining != 0);
    owner = r_u32(context_offset);
    span = r_u32(context_offset + 4);
    return p2_func_80019260(owner, span);
}

uint32 p2_func_80019260(uint32 owner_offset, uint32 span_offset)
{
    FUNCTION_MARKER(0x80019260, "SLUS_005.78");
    uint32 saved_stack = p2_guest_stack_pointer;
    uint32 vertices = r_u32(owner_offset + 40);
    uint32 polygon = r_u32(owner_offset + 36) + (uint32)r_s16(span_offset + 4) * 24;
    uint32 remaining = (uint32)r_s16(span_offset + 6);
    uint32 edge0, edge1, normal;
    uint32 vertex0, vertex1, vertex2, vertex3;
    uint32 x, y, z, dot, plane, i;
    uint32 matrix;
    p2_guest_stack_pointer -= 0x70;
    edge0 = p2_guest_stack_pointer + 0x10;
    edge1 = p2_guest_stack_pointer + 0x20;
    normal = p2_guest_stack_pointer + 0x30;
    if (remaining != 0)
    {
        do
        {
            vertex0 = vertices + ((uint32)r_u16(polygon + 4) << 4);
            vertex1 = vertices + ((uint32)r_u16(polygon + 6) << 4);
            vertex3 = vertices + ((uint32)r_u16(polygon + 10) << 4);
            vertex2 = vertices + ((uint32)r_u16(polygon + 8) << 4);
            w_u32(edge0, (uint32)r_s16(vertex2) - (uint32)r_s16(vertex0));
            w_u32(edge0 + 4, (uint32)r_s16(vertex2 + 2) - (uint32)r_s16(vertex0 + 2));
            w_u32(edge0 + 8, (uint32)r_s16(vertex2 + 4) - (uint32)r_s16(vertex0 + 4));
            w_u32(edge1, (uint32)r_s16(vertex3) - (uint32)r_s16(vertex1));
            w_u32(edge1 + 4, (uint32)r_s16(vertex3 + 2) - (uint32)r_s16(vertex1 + 2));
            w_u32(edge1 + 8, (uint32)r_s16(vertex3 + 4) - (uint32)r_s16(vertex1 + 4));
            xport_gte_write_control(0, r_u32(edge0));
            xport_gte_write_control(2, r_u32(edge0 + 4));
            xport_gte_write_control(4, r_u32(edge0 + 8));
            xport_gte_write_data(11, r_u32(edge1 + 8));
            xport_gte_write_data(9, r_u32(edge1));
            xport_gte_write_data(10, r_u32(edge1 + 4));
            xport_gte_execute(0x178000C);
            w_u32(normal, xport_gte_read_data(25));
            w_u32(normal + 4, xport_gte_read_data(26));
            w_u32(normal + 8, xport_gte_read_data(27));
            while (r_u32(normal) + 26754u > 0xD103u || r_u32(normal + 4) + 26754u > 0xD103u || r_u32(normal + 8) + 26754u > 0xD103u)
            {
                x = (uint32)(r_s32(normal) >> 1);
                y = (uint32)(r_s32(normal + 4) >> 1);
                z = (uint32)(r_s32(normal + 8) >> 1);
                w_u32(normal, x);
                w_u32(normal + 4, y);
                w_u32(normal + 8, z);
            }
            VectorNormalS((VECTOR *)psx_addr(normal, 12), (SVECTOR *)psx_addr(polygon + 16, 6));
            dot = (uint32)(r_s16(vertex0) * r_s16(polygon + 16));
            dot += (uint32)(r_s16(vertex0 + 2) * r_s16(polygon + 18));
            dot += (uint32)(r_s16(vertex0 + 4) * r_s16(polygon + 20));
            plane = 0u - (uint32)((sint32)dot >> 7);
            w_u32(polygon + 12, plane);
            if ((r_u32(r_u32(polygon)) & 1) != 0)
                w_u32(polygon + 12, plane + 0x3FFFF800);
            for (i = 0; i < 4; ++i)
            {
                uint32 vertex = i == 0 ? vertex0 : i == 1 ? vertex1 : i == 2 ? vertex2 : vertex3;
                w_u16(vertex + 8, r_u16(vertex + 8) + r_u16(polygon + 16));
                w_u16(vertex + 10, r_u16(vertex + 10) + r_u16(polygon + 18));
                w_u16(vertex + 12, r_u16(vertex + 12) + r_u16(polygon + 20));
            }
            polygon += 24;
            --remaining;
        } while (remaining != 0);
    }
    vertices += (uint32)r_s16(span_offset) << 4;
    remaining = (uint32)r_s16(span_offset + 2);
    if (remaining != 0)
    {
        uint32 vertex = vertices + 8;
        do
        {
            VectorNormalSS((SVECTOR *)psx_addr(vertex, 6), (SVECTOR *)psx_addr(vertex, 6));
            vertex += 16;
            --remaining;
        } while (remaining != 0);
    }
    matrix = r_u32(0x8008A0C8) - 32;
    for (i = 0; i < 5; ++i)
        xport_gte_write_control(i, r_u32(matrix + i * 4));
    p2_guest_stack_pointer = saved_stack;
    return matrix;
}

uint32 p2_func_8002aa1c(uint32 x, uint32 y, uint32 z)
{
    FUNCTION_MARKER(0x8002AA1C, "SLUS_005.78");
    uint32 low, middle, high;
    if ((sint32)x < 0)
        x = 0u - x;
    if ((sint32)y < 0)
        y = 0u - y;
    if ((sint32)z < 0)
        z = 0u - z;
    low = y;
    if ((sint32)y < (sint32)x)
        high = x;
    else
    {
        low = x;
        high = y;
    }
    middle = high;
    if ((sint32)high < (sint32)z)
        high = z;
    else
    {
        middle = low;
        if ((sint32)z < (sint32)low)
            low = z;
        else
            middle = z;
    }
    if (low != 0)
        return (uint32)(((sint64)(sint32)low * 1152) >> 12) + (uint32)(((sint64)(sint32)middle * 1536) >> 12) + (uint32)(((sint64)(sint32)high * 3840) >> 12);
    if (middle != 0)
        return (uint32)(((sint64)(sint32)middle * 1632) >> 12) + (uint32)(((sint64)(sint32)high * 3936) >> 12);
    return high;
}

uint32 p2_func_800401c8(uint32 object)
{
    FUNCTION_MARKER(0x800401C8, "SLUS_005.78");
    return p2_func_8002a144(object, &registered_object_head, 0);
}

uint32 p2_func_80066bdc(uint32 mode, uint32 extent, uint32 flags)
{
    FUNCTION_MARKER(0x80066BDC, "SLUS_005.78");
    uint32 ordering_table = render_commands.scene_ordering_table_offset;
    uint32 packet = render_commands.aux_packet_offset;
    uint32 link = r_u32(ordering_table + 2400);
    uint32 result;
    w_u32(packet + 4, (flags & 0xFFFFFF) | 0x60000000);
    w_u32(packet + 8, mode);
    w_u32(packet + 12, extent);
    w_u32(packet, (link & 0xFFFFFF) | 0x03000000);
    result = (r_u32(ordering_table + 2400) & 0xFF000000) | (packet & 0xFFFFFF);
    w_u32(ordering_table + 2400, result);
    render_commands.aux_packet_offset = (packet + 16);
    return result;
}







uint32 p2_func_80034f04(void)
{
    FUNCTION_MARKER(0x80034F04, "SLUS_005.78");
    uint32 i;
    w_u8(0x8008A43D, 0);
    for (i = 0; i < 8; ++i)
    {
        w_u32(0x800B15F4 + i * 20, 0);
        w_u32(0x800B15F8 + i * 20, 0);
        w_u32(0x800B15FC + i * 20, 0);
    }
    return 0;
}







uint32 p2_func_80017938(sint32 frame, uint32 track_offset, uint32 destination_offset)
{
    FUNCTION_MARKER(0x80017938, "SLUS_005.78");
    uint32 next = r_u32(track_offset + 12);
    uint32 previous = next + ((uint32)r_u8(track_offset) << 4) - 16;
    uint32 frame_value = ((uint32)frame << 20) | 0xFFFFF;
    uint32 end_value = r_u32(previous);
    uint32 weight, inverse, result = 0, i;
    sint32 turns;
    if (frame_value < end_value)
    {
        next += 16;
        do
        {
            end_value = r_u32(next);
            next += 16;
        } while (frame_value >= end_value);
        next -= 16;
        previous = next - 16;
    }
    else
    {
        if (frame_value == (end_value | 0xFFFFF))
            goto copy_key;
        end_value = ((uint32)r_s16(track_offset + 4) << 20) | 0xFFFFF;
        if (frame_value >= end_value)
        {
            if ((r_u8(track_offset + 2) & 3) == 3)
                previous = next;
            goto copy_key;
        }
    }
    weight = InterpolateKeyTime(frame_value, end_value, r_u32(previous), r_u32(next), r_u32(track_offset + 12));
    turns = r_s16(next + 6);
    weight = (uint32)((sint32)weight >> 6);
    if (turns == 0)
    {
        inverse = 0x4000u - weight;
        for (i = 0; i < 4; ++i)
        {
            uint32 first = (uint32)r_s16(previous + 8 + i * 2) * inverse;
            uint32 second = (uint32)r_s16(next + 8 + i * 2) * weight;
            result = (uint32)((sint32)first >> 14) + (uint32)((sint32)second >> 14);
            w_u16(destination_offset + i * 2, result);
        }
        return result;
    }
    else
    {
        sint32 angle = r_s16(next + 4);
        uint32 extra = (uint32)(turns - 1) << 27;
        uint32 product = weight * ((uint32)angle + (uint32)((sint32)extra >> 16));
        sint32 part = (sint32)product >> 14;
        sint32 divisor = SinQ14((sint16)angle);
        sint32 reciprocal;
        sint32 sine;
        uint32 source_weight, target_weight;
        if (divisor == 0)
            xport_mips_break(7);
        reciprocal = divisor == 0 ? -1 : 0x10000000 / divisor;
        sine = SinQ14((sint16)((uint32)angle - (uint32)part));
        source_weight = (uint32)(((sint64)sine * reciprocal) >> 14);
        sine = SinQ14((sint16)part);
        target_weight = (uint32)(((sint64)sine * reciprocal) >> 14);
        for (i = 0; i < 4; ++i)
        {
            uint32 first = (uint32)(((sint64)r_s16(previous + 8 + i * 2) * (sint32)source_weight) >> 14);
            uint32 second = (uint32)(((sint64)r_s16(next + 8 + i * 2) * (sint32)target_weight) >> 14);
            w_u16(destination_offset + i * 2, first + second);
        }
        if (reciprocal > 0x50000)
        {
            sint32 y = r_s16(destination_offset + 2);
            sint32 z = r_s16(destination_offset + 4);
            sint32 w = r_s16(destination_offset + 6);
            sint32 x = r_s16(destination_offset);
            uint32 length = (uint32)(y * y) + (uint32)(z * z) + (uint32)(w * w) + (uint32)(x * x);
            length = (uint32)((sint32)length >> 14);
            if (length + 0x3FF0u < 0x7FE0u)
            {
                uint32 root = p2_sdk_8007e794(length >> 2) << 2;
                sint32 scale;
                uint32 px, py, pz, pw;
                if (root == 0)
                    xport_mips_break(7);
                scale = root == 0 ? -1 : 0x10000000 / (sint32)root;
                px = (uint32)r_s16(destination_offset) * (uint32)scale;
                py = (uint32)r_s16(destination_offset + 2) * (uint32)scale;
                pz = (uint32)r_s16(destination_offset + 4) * (uint32)scale;
                pw = (uint32)r_s16(destination_offset + 6) * (uint32)scale;
                w_u16(destination_offset, (sint32)px >> 14);
                w_u16(destination_offset + 2, (sint32)py >> 14);
                w_u16(destination_offset + 4, (sint32)pz >> 14);
                result = (uint32)((sint32)pw >> 14);
                w_u16(destination_offset + 6, result);
            }
        }
        return result;
    }
copy_key:
{
    uint32 first = r_u32(previous + 8);
    uint32 second = r_u32(previous + 12);
    w_u32(destination_offset, first);
    w_u32(destination_offset + 4, second);
    return first;
}
}

uint32 p2_func_8002ede4(uint32 vertex_offset, uint32 flags)
{
    FUNCTION_MARKER(0x8002EDE4, "SLUS_005.78");
    uint32 previous_flags = r_u32(0x1F800334);
    uint32 table = 0x1F800324;
    uint32 light = r_u32(table);
    uint32 result = flags ^ previous_flags;
    uint32 i;
    if (result != 0)
    {
        w_u32(0x1F800334, flags);
        if ((sint32)result < 0)
        {
            uint16 values[9];
            for (i = 0; i < 9; ++i)
                values[i] = r_u16(0x1F800350 + i * 2);
            for (i = 0; i < 9; ++i)
                w_u16(0x1F800350 + i * 2, 0u - values[i]);
            for (i = 0; i < 5; ++i)
                xport_gte_write_control(8 + i, r_u32(0x1F800350 + i * 4));
        }
        {
            uint32 ambient = flags & 0x7FFFFFFF;
            uint32 x = (uint32)r_s16(0x1F80003C);
            uint32 y = (uint32)r_s16(0x1F80003E);
            uint32 z = (uint32)r_s16(0x1F800040);
            result = z + ambient;
            xport_gte_write_control(13, (x + ambient) << 4);
            xport_gte_write_control(14, (y + ambient) << 4);
            xport_gte_write_control(15, result << 4);
        }
    }
    if (light != 0)
    {
        uint32 output = 0x1F80033C + (uint32)r_u8(0x1F800338) * 2;
        do
        {
            uint32 vx = (uint32)r_s16(vertex_offset);
            uint32 x = r_u32(light + 56);
            uint32 vy = (uint32)r_s16(vertex_offset + 2);
            uint32 y = r_u32(light + 60);
            uint32 vz = (uint32)r_s16(vertex_offset + 4);
            uint32 z = r_u32(light + 64);
            uint32 temporary, distance, attenuation;
            x -= vx;
            y -= vy;
            z -= vz;
            if ((sint32)x < 0)
                x = 0u - x;
            if ((sint32)y < 0)
                y = 0u - y;
            if ((sint32)z < 0)
                z = 0u - z;
            if ((sint32)y < (sint32)x)
            {
                temporary = x;
                x = y;
                y = temporary;
            }
            if ((sint32)y < (sint32)z)
            {
                temporary = z;
                z = y;
                y = temporary;
            }
            else if ((sint32)z < (sint32)x)
            {
                temporary = z;
                z = x;
                x = temporary;
            }
            distance = y;
            if (x != 0)
                distance = (uint32)((sint32)(36 * x + 48 * z + 120 * y) >> 7);
            else if (z != 0)
                distance = (uint32)((sint32)(51 * z + 123 * y) >> 7);
            result = r_u32(light + 80);
            attenuation = result - distance;
            if ((sint32)attenuation <= 0)
            {
                w_u16(output, 0);
                w_u16(output + 6, 0);
                w_u16(output + 12, 0);
            }
            else if ((sint32)attenuation < r_s32(light + 84))
            {
                result = (uint32)((sint32)(attenuation * r_u32(light + 92)) >> 2);
                w_u16(output, result);
                result = (uint32)((sint32)(attenuation * r_u32(light + 96)) >> 2);
                w_u16(output + 6, result);
                result = (uint32)((sint32)(attenuation * r_u32(light + 100)) >> 2);
                w_u16(output + 12, result);
            }
            else
            {
                result = r_u32(light + 68);
                w_u16(output, result);
                result = r_u32(light + 72);
                w_u16(output + 6, result);
                result = r_u32(light + 76);
                w_u16(output + 12, result);
            }
            table += 4;
            light = r_u32(table);
            output += 2;
        } while (light != 0);
        for (i = 0; i < 5; ++i)
            xport_gte_write_control(16 + i, r_u32(0x1F80033C + i * 4));
    }
    return result;
}

uint32 p2_func_800612c0(uint32 material_offset, uint32 destination_offset, uint32 stride)
{
    FUNCTION_MARKER(0x800612C0, "SLUS_005.78");
    uint32 flags = r_u32(material_offset);
    uint32 texture_fields;
    uint32 uv0, uv1, uv2, uv3;
    uint32 packed;
    if ((flags & 0x1400) == 0x1000)
    {
        uint32 texture;
        uv0 = r_u32(material_offset + 8);
        texture = r_u32(material_offset + 4);
        packed = r_u32(material_offset + 12);
        texture_fields = texture + 4;
        uv1 = uv0 >> 16;
        uv2 = packed >> 16;
        uv3 = packed & 0xFFFF;
        if ((flags & 0x1E000) != 0)
        {
            uint32 right = r_u8(texture + 6);
            uint32 left = r_u8(texture + 4);
            uint32 bottom = r_u8(texture + 7);
            uint32 top = r_u8(texture + 5);
            uint32 width = (uint32)((sint32)(right - left + 1) >> 1);
            uint32 height = (uint32)((sint32)(bottom - top + 1) >> 1);
            uint32 counter = r_u32(texture + 12);
            uint32 u = 0, v = 0;
            if ((flags & 0x6000) != 0)
            {
                uint32 remainder;
                if (width == 0)
                    xport_mips_break(7);
                remainder = width == 0 ? counter : counter % width;
                u = (flags & 0x2000) != 0 ? remainder : width - remainder;
            }
            if ((flags & 0x18000) != 0)
            {
                uint32 remainder;
                if (height == 0)
                    xport_mips_break(7);
                remainder = height == 0 ? counter : counter % height;
                v = (flags & 0x8000) != 0 ? remainder : height - remainder;
            }
            packed = u | (v << 8);
            uv0 += packed;
            uv1 += packed;
            uv3 += packed;
            uv2 += packed;
        }
    }
    else
    {
        texture_fields = material_offset + 4;
        if ((flags & 0x1E000) != 0)
        {
            uint32 left = r_u8(material_offset + 4);
            uint32 right = r_u8(material_offset + 6);
            uint32 top = r_u8(material_offset + 5);
            uint32 bottom = r_u8(material_offset + 7);
            uint32 counter = r_u32(material_offset + 12);
            uint32 width = right - left + 1;
            uint32 height = bottom - top + 1;
            if ((flags & 0x6000) != 0)
            {
                uint32 remainder;
                width >>= 1;
                if (width == 0)
                    xport_mips_break(7);
                remainder = width == 0 ? counter : counter % width;
                left += (flags & 0x2000) != 0 ? remainder : width - remainder;
            }
            right = left + width - 1;
            if ((flags & 0x18000) != 0)
            {
                uint32 remainder;
                height >>= 1;
                if (height == 0)
                    xport_mips_break(7);
                remainder = height == 0 ? counter : counter % height;
                top += (flags & 0x8000) != 0 ? remainder : height - remainder;
            }
            bottom = top + height - 1;
            uv0 = left | (top << 8) | (right << 16) | (bottom << 24);
        }
        else
            uv0 = r_u32(material_offset + 4);
        uv3 = uv0 >> 16;
        if ((flags & 8) != 0)
        {
            uint32 right = uv3 & 0xFF;
            uv3 = (uv3 & 0xFFFFFF00) | (uv0 & 0xFF);
            uv0 = (uv0 & 0xFFFFFF00) | right;
        }
        if ((flags & 0x10) != 0)
        {
            uint32 bottom = uv3 & 0xFF00;
            uv3 = (uv3 & 0xFFFF00FF) | (uv0 & 0xFF00);
            uv0 = (uv0 & 0xFFFF00FF) | bottom;
        }
        uv1 = (uv0 & 0xFF00) | (uv3 & 0xFF);
        uv2 = (uv3 & 0xFF00) | (uv0 & 0xFF);
    }
    packed = r_u32(texture_fields + 4);
    uv1 |= ((flags & 0x60) << 16) | (packed << 16);
    uv0 = (uv0 & 0xFFFF) | (packed & 0xFFFF0000);
    w_u32(destination_offset, uv0);
    w_u32(destination_offset + stride * 4, uv1);
    w_u32(destination_offset + stride * 8, uv2);
    destination_offset += stride * 12;
    w_u32(destination_offset, uv3);
    return destination_offset;
}

uint32 p2_func_8006aaa8(void)
{
    FUNCTION_MARKER(0x8006AAA8, "SLUS_005.78");
    return SpuIsTransferCompleted(0) != 0;
}




