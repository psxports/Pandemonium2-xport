#include <stdio.h>
#include <stdlib.h>

#include "game.h"

ScreenRenderState screen_render = {0};
uint8 demo_background_workspace[8184] = {0};

BackgroundResource *BackgroundResource::Relocate()
{
    FUNCTION_MARKER(0x800602F4, "SLUS_005.78");
    const uint8 *header = data;
    uint32 payload = 40;
    uint32 size, pixels, i;
    if (data == NULL || bytes < 40)
    {
        fprintf(stderr, "P2: Truncated background header\n");
        abort();
    }
    metadata = xport_load_le16(header);
    frame_index = xport_load_le16(header + 2);
    frame_count = xport_load_le16(header + 4);
    frame_rate = xport_load_le16(header + 6);
    last_tick = xport_load_le32(header + 16);
    start_time = xport_load_le32(header + 20);
    width = xport_load_le16(header + 24);
    height = xport_load_le16(header + 26);
    metadata_halfword = xport_load_le16(header + 32);
    tile_count = xport_load_le16(header + 34);
    commands = data + 40;
    command_cursor = NULL;
    if (width == 0 || height == 0 || tile_count == 0)
    {
        fprintf(stderr, "P2: Invalid background dimensions\n");
        abort();
    }
    if (frame_count == 0)
    {
        pixels = (uint32)width * height;
        payload = 44;
        if ((pixels & 1) != 0)
            ++pixels;
        if (bytes < payload || pixels > (bytes - payload) / 2)
        {
            fprintf(stderr, "P2: Truncated background map\n");
            abort();
        }
        static_map = data + payload;
        size = pixels << 1;
    }
    else
    {
        // The reviewed streams reset their zero cursor on the first frame advance
        if (xport_load_le32(header + 12) != 0 || frame_count == 0xFFFF || (uint32)width * height > sizeof(demo_background_workspace) / 2)
        {
            fprintf(stderr, "P2: Invalid animated background layout\n");
            abort();
        }
        static_map = demo_background_workspace;
        size = frame_index;
    }
    if (size > bytes - payload || (uint32)tile_count > (bytes - payload - size) / 8)
    {
        fprintf(stderr, "P2: Truncated background tile table\n");
        abort();
    }
    command_end = data + payload + size;
    free(tiles);
    tiles = (BackgroundTile *)malloc((uint32)tile_count * sizeof(BackgroundTile));
    if (tiles == NULL)
    {
        fprintf(stderr, "P2: Cannot allocate background tiles\n");
        abort();
    }
    for (i = 0; i < tile_count; ++i)
    {
        tiles[i].uv = xport_load_le32(command_end + i * 8);
        tiles[i].attributes = xport_load_le32(command_end + i * 8 + 4);
    }
    frame_index = frame_count;
    return this;
}

void DrawSceneBackground(void)
{
    uint32 *ot_entry;
    FUNCTION_MARKER(0x8005FB40, "SLUS_005.78");
    SceneSection *section;
    BackgroundResource *background;
    const uint8 *map;
    uint8 *map_destination;
    BackgroundTile *table;
    const BackgroundTile *definition;
    const uint8 *cursor;
    const uint8 *map_cursor, *row_start;
    uint32 saved_color, width, height, command, kind, count;
    uint32 map_bytes, map_offset, span;
    uint32 index, elapsed, tick, destination, angle, palette, tile, ordering, packet;
    uint32 uv0, uv1, attributes, temporary, xy, next_xy, next_row, result, remaining;
    sint32 vertical, horizontal, row, column, x, y, rows, columns, current_column;

    result = game_flow.mode_flags & 0x800;
    if (result != 0)
        return;
    ClearSceneOrderingTable();
    if (screen_render.fade_level == 4096)
    {
        section = scene_stream.active_section;
        saved_color = section->background_top_color;
        section->background_top_color = 0;
        DrawBackgroundBorders(240, 1);
        section = scene_stream.active_section;
        section->background_top_color = saved_color;
        return;
    }
    section = scene_stream.active_section;
    background = section->background;
    if (background == 0)
    {
        DrawBackgroundRectangle(0, 0x00F00140, section->background_top_color);
        return;
    }
    if ((section->state_flags & 0x40) != 0)
    {
        section->state_flags &= 0xFFBF;
        temporary = (uint32)(sint32)section->palette_bank;
        table = background->tiles;
        remaining = background->tile_count;
        if (temporary == 2)
        {
            do
            {
                --remaining;
                table->attributes &= 0xBFFFFFEF;
                ++table;
            } while (remaining != 0);
        }
        else
        {
            do
            {
                --remaining;
                table->attributes |= 0x40000010;
                ++table;
            } while (remaining != 0);
        }
    }
    width = background->width;
    height = background->height;
    if (screen_render.background_projection != 0x000E9555)
    {
        screen_render.background_projection = 0x000E9555;
        result = SquareRoot12(0x00170C59);
        result = (uint32)AsinAngle(30720, (sint32)result);
        screen_render.background_angle = (0u - result) & 0xFFF;
    }
    angle = (1024u + camera_projection.rotation[0] + screen_render.background_angle) & 0xFFF;
    if (angle >= 0x801)
        angle |= 0xF000;
    vertical = (640 * (sint32)(sint16)angle) / 2048;
    row = vertical / 16 - ((sint32)(40u - height) >> 1);
    y = -(vertical & 15);
    DrawBackgroundBorders((sint32)((uint32)y - ((uint32)row << 4) - 1u), height << 4);
    angle = (0u - (uint32)(sint32)(sint16)camera_projection.rotation[1]) & 0xFFF;
    horizontal = (sint32)(angle * width) / 256;
    column = horizontal / 16;
    x = -(horizontal & 15);
    map = background->static_map;
    map_bytes = width * height * 2;
    if (background->frame_count != 0)
    {
        elapsed = frame_clock.milliseconds - background->start_time;
        tick = (elapsed * (uint32)background->frame_rate) / 1000u;
        if (tick != background->last_tick)
        {
            index = (background->frame_index + 1u) & 0xFFFF;
            count = background->frame_count;
            background->last_tick = tick;
            background->frame_index = index;
            if (count < index)
            {
                cursor = background->commands;
                background->frame_index = 0;
            }
            else
                cursor = background->command_cursor;
            for (;;)
            {
                if (cursor == NULL || cursor < background->commands || cursor > background->command_end || background->command_end - cursor < 4)
                {
                    fprintf(stderr, "P2: Truncated background command\n");
                    abort();
                }
                command = xport_load_le32(cursor);
                cursor += 4;
                if (command == 0)
                    break;
                kind = command >> 30;
                if (kind == 0)
                {
                    count = (((command >> 20) & 0x3FF) + 1u) * (((command >> 10) & 0x3FF) + 1u) * 2u;
                    if (count > map_bytes || count > (uint32)(background->command_end - cursor))
                    {
                        fprintf(stderr, "P2: Invalid background map command\n");
                        abort();
                    }
                    CopyBytes(demo_background_workspace, cursor, count);
                    cursor += count;
                    break;
                }
                count = (command & 0x3FF) + 1u;
                if (kind == 1 || kind == 2)
                {
                    map_offset = 2u * (((command >> 20) & 0x3FF) * height + ((command >> 10) & 0x3FF));
                    span = kind == 1 ? (count - 1u) * height * 2u + 2u : count * 2u;
                    if (map_offset > map_bytes || span > map_bytes - map_offset || count * 2u > (uint32)(background->command_end - cursor))
                    {
                        fprintf(stderr, "P2: Invalid background delta command\n");
                        abort();
                    }
                    map_destination = demo_background_workspace + map_offset;
                    if (kind == 1)
                    {
                        do
                        {
                            temporary = xport_load_le16(cursor);
                            cursor += 2;
                            --count;
                            xport_store_le16(map_destination, temporary);
                            if (count != 0)
                                map_destination += height * 2;
                        } while (count != 0);
                    }
                    else
                    {
                        count *= 2;
                        CopyBytes(map_destination, cursor, count);
                        cursor += count;
                    }
                }
            }
            background->command_cursor = cursor;
        }
    }
    if (column < 0)
        column = (sint32)((uint32)column + width);
    if (row < 0)
    {
        y = (sint32)((uint32)y - ((uint32)row << 4));
        row = 0;
    }
    index = level_resource.level_id;
    if (index == 78 || (index == 14 && (scene_stream.active_section)->section_index == 0))
    {
        index = screen_render.background_scroll;
        column = (sint32)(index + (uint32)column);
        screen_render.background_scroll = index + 1;
        if (((index + 1) & 0xFF) == width)
            screen_render.background_scroll = 0;
        if (column >= (sint32)width)
            column = (sint32)((uint32)column - width);
    }
    rows = (sint32)(255u - (uint32)y) / 16;
    vertical = (sint32)(height - (uint32)row);
    if (vertical < rows)
        rows = vertical;
    result = 335;
    if (rows <= 0)
        return;
    result = 335u - (uint32)x;
    columns = (sint32)result / 16;
    if ((sint32)result < 0)
        result += 15;
    if (columns <= 0)
        return;
    next_row = ((uint32)y << 16) + 0x100000;
    table = background->tiles;
    row_start = map + 2u * (uint32)column * height + 2u * (uint32)row;
    do
    {
        current_column = column;
        map_cursor = row_start;
        horizontal = x;
        remaining = (uint32)columns;
        do
        {
            if (current_column == (sint32)width)
            {
                current_column = 0;
                map_cursor = map + 2u * (uint32)row;
            }
            tile = xport_load_le16(map_cursor);
            xy = ((uint32)horizontal & 0xFFFF) | ((uint32)y << 16);
            index = (tile & 0x1FFF) * 8;
            if ((tile & 0x1FFF) >= background->tile_count)
            {
                fprintf(stderr, "P2: Invalid background tile index\n");
                abort();
            }
            definition = table + (tile & 0x1FFF);
            ordering = render_commands.scene_ordering_table_offset + (index >> 1);
            ot_entry = (uint32 *)psx_addr(ordering + 4, sizeof(uint32));
            if ((tile & 0xC000) != 0)
            {
                uv0 = definition->uv;
                attributes = definition->attributes;
                uv1 = uv0 >> 16;
                if ((tile & 0x4000) != 0)
                {
                    temporary = uv1 & 0xFF;
                    uv1 = (uv1 & 0xFFFFFF00) | (uv0 & 0xFF);
                    uv0 = (uv0 & 0xFFFFFF00) | temporary;
                }
                if ((tile & 0x8000) != 0)
                {
                    temporary = uv1 & 0xFF00;
                    uv1 = (uv1 & 0xFFFF00FF) | (uv0 & 0xFF00);
                    uv0 = (uv0 & 0xFFFF00FF) | temporary;
                }
                uv0 = (uv0 & 0xFFFF) | (attributes & 0xFFFF0000);
                palette = (uv1 & 0xFF00) | (uv0 & 0xFF);
                next_xy = (((uint32)horizontal + 16u) & 0xFFFF) | next_row;
                temporary = *ot_entry;
                packet = render_commands.aux_packet_offset;
                POLY_FT4 *poly = (POLY_FT4 *)psx_addr(packet, sizeof(POLY_FT4));
                setPolyFT4(poly);
                poly->rgbc.rgbc = 0x2D000000;
                poly->tag = (temporary & 0xFFFFFF) | 0x09000000;
                poly->p[1].xy.xy = (next_xy & 0xFFFF) | (xy & 0xFFFF0000);
                xport_store_le32(&poly->p[1].uv, (uv0 & 0xFF00) | (uv1 & 0xFF) | (attributes << 16));
                poly->p[0].xy.xy = xy;
                xport_store_le32(&poly->p[0].uv, uv0);
                poly->p[2].xy.xy = (xy & 0xFFFF) | (next_xy & 0xFFFF0000);
                xport_store_le32(&poly->p[2].uv, palette);
                poly->p[3].xy.xy = next_xy;
                xport_store_le32(&poly->p[3].uv, uv1);
                temporary = *ot_entry;
                destination = packet + 40;
            }
            else
            {
                uv0 = definition->uv;
                attributes = definition->attributes;
                packet = render_commands.aux_packet_offset;
                DR_MODE *mode = (DR_MODE *)psx_addr(packet, sizeof(DR_MODE));
                SPRT_16 *spr = (SPRT_16 *)psx_addr(packet + 12, sizeof(SPRT_16));
                setSprt16(spr);
                mode->tag = ((packet + 12) & 0xFFFFFF) | 0x02000000;
                mode->code[0] = (attributes & 0xFFFF) | 0xE1000000;
                mode->code[1] = 0;
                spr->tag = (*ot_entry & 0xFFFFFF) | 0x03000000;
                spr->rgbc.rgbc = 0x7D000000;
                spr->p[0].xy.xy = xy;
                xport_store_le32(&spr->p[0].uv, (uv0 & 0xFFFF) | (attributes & 0xFFFF0000));
                temporary = *ot_entry;
                destination = packet + 28;
            }
            *ot_entry = (temporary & 0xFF000000) | (packet & 0xFFFFFF);
            render_commands.aux_packet_offset = destination;
            current_column = (sint32)((uint32)current_column + 1u);
            horizontal = (sint32)((uint32)horizontal + 16u);
            --remaining;
            map_cursor += height * 2;
        } while (remaining != 0);
        row = (sint32)((uint32)row + 1u);
        result = 0x100000;
        next_row += 0x100000;
        y = (sint32)((uint32)y + 16u);
        --rows;
        row_start += 2;
    } while (rows != 0);
    return;
}

uint32 DrawBackgroundBorders(sint32 top, uint32 height)
{
    FUNCTION_MARKER(0x80064DBC, "SLUS_005.78");
    const SceneSection *section;
    uint32 extent;
    sint32 bottom, remaining;
    if (top >= 0)
    {
        extent = 239;
        if (top < 239)
            extent = (uint32)top;
        section = scene_stream.active_section;
        DrawBackgroundRectangle(0, ((extent + 1u) << 16) | 320u, section->background_top_color);
    }
    bottom = (sint32)((uint32)top + height);
    if (bottom >= 240)
        return 0;
    extent = 239;
    remaining = (sint32)(239u - (uint32)bottom);
    if (remaining < 239)
        extent = (uint32)remaining;
    else if (bottom < 0)
        bottom = 0;
    section = scene_stream.active_section;
    return DrawBackgroundRectangle((uint32)bottom << 16, ((extent + 1u) << 16) | 320u, section->background_bottom_color);
}

uint32 DrawSceneFade(void)
{
    FUNCTION_MARKER(0x80064A58, "SLUS_005.78");
    uint32 brightness, table, packet, color, value, result;
    DR_MODE *mode;
    TILE *rect;
    uint32 *ot_entry;
    brightness = screen_render.fade_level;
    table = render_commands.small_ordering_table_offset;
    ot_entry = (uint32 *)psx_addr(table + 120, sizeof(uint32));
    packet = render_commands.packet_offset;
    brightness = (255 * brightness + 2048) >> 12;
    color = brightness | (brightness << 8) | (brightness << 16);
    mode = (DR_MODE *)psx_addr(packet, sizeof(DR_MODE));
    rect = (TILE *)psx_addr(packet + 12, sizeof(TILE));
    setTile(rect);
    mode->tag = ((packet + 12) & 0xFFFFFF) | 0x02000000;
    mode->code[0] = 0xE1000040;
    // Retain the original NOP after the draw-mode command
    mode->code[1] = 0;
    value = *ot_entry;
    rect->tag = (value & 0xFFFFFF) | 0x03000000;
    rect->rgbc.rgbc = color | 0x62000000;
    rect->p[0].xy = 0;
    xport_store_le32(&rect->w, 0x00F00140);
    value = *ot_entry;
    result = (value & 0xFF000000) | (packet & 0xFFFFFF);
    *ot_entry = result;
    render_commands.packet_offset = (packet + 28);
    return result;
}

uint32 DrawOverlayRectangle(uint32 position, uint32 extent, sint32 fraction, uint32 color)
{
    uint32 red;
    uint32 green;
    uint32 blue;
    uint32 packet;
    uint32 next;
    DR_MODE *mode;
    TILE *rect;
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
    if (render_commands.overlay_head == 0)
        render_commands.overlay_head = (render_commands.packet_offset);
    packet = render_commands.packet_offset;
    next = packet + 12;
    mode = (DR_MODE *)psx_addr(packet, sizeof(DR_MODE));
    rect = (TILE *)psx_addr(packet + 12, sizeof(TILE));
    setTile(rect);
    mode->tag = (next & 0xFFFFFF) | 0x02000000;
    mode->code[0] = 0xE1000040;
    // Retain the original NOP after the draw-mode command
    mode->code[1] = 0;
    rect->rgbc.rgbc = color | 0x62000000;
    rect->tag = ((packet + 28) & 0xFFFFFF) | 0x03000000;
    rect->p[0].xy = position;
    xport_store_le32(&rect->w, extent);
    render_commands.overlay_tail = (next);
    render_commands.packet_offset = (packet + 28);
    return 0x03000000;
}

uint32 DrawBackgroundRectangle(uint32 position, uint32 extent, uint32 color)
{
    FUNCTION_MARKER(0x80066BDC, "SLUS_005.78");
    uint32 ordering_table = render_commands.scene_ordering_table_offset;
    uint32 packet = render_commands.aux_packet_offset;
    uint32 *ot_entry = (uint32 *)psx_addr(ordering_table + 2400, sizeof(uint32));
    uint32 link = *ot_entry;
    uint32 result;
    TILE *rect;
    rect = (TILE *)psx_addr(packet, sizeof(TILE));
    setTile(rect);
    rect->rgbc.rgbc = (color & 0xFFFFFF) | 0x60000000;
    rect->p[0].xy = position;
    xport_store_le32(&rect->w, extent);
    rect->tag = (link & 0xFFFFFF) | 0x03000000;
    result = (*ot_entry & 0xFF000000) | (packet & 0xFFFFFF);
    *ot_entry = result;
    render_commands.aux_packet_offset = (packet + 16);
    return result;
}

uint32 DrawScreenBrighten(sint32 intensity)
{
    FUNCTION_MARKER(0x80065F78, "SLUS_005.78");
    uint32 color, packet, next_packet;
    DR_MODE *mode;
    TILE *rect;
    if (intensity >= 4096)
        color = 255;
    else if (intensity > 0)
        color = (uint32)(255 * intensity + 2048) >> 12;
    else
        color = 0;
    color |= (color << 8) | (color << 16);
    if (render_commands.overlay_head == 0)
        render_commands.overlay_head = render_commands.packet_offset;
    packet = render_commands.packet_offset;
    next_packet = packet + 28;
    mode = (DR_MODE *)psx_addr(packet, sizeof(DR_MODE));
    rect = (TILE *)psx_addr(packet + 12, sizeof(TILE));
    setTile(rect);
    mode->tag = ((packet + 12) & 0xFFFFFF) | 0x02000000;
    mode->code[0] = 0xE1000020;
    mode->code[1] = 0;
    rect->rgbc.rgbc = color | 0x62000000;
    rect->tag = (next_packet & 0xFFFFFF) | 0x03000000;
    rect->p[0].xy = 0;
    xport_store_le32(&rect->w, 0x00F00140);
    render_commands.overlay_tail = packet + 12;
    render_commands.packet_offset = next_packet;
    return 0x03000000;
}

uint32 DrawScreenDarken(sint32 intensity)
{
    FUNCTION_MARKER(0x80066070, "SLUS_005.78");
    uint32 color, packet, next_packet;
    DR_MODE *mode;
    TILE *rect;
    if (intensity >= 4096)
        color = 255;
    else if (intensity > 0)
        color = (uint32)(255 * intensity + 2048) >> 12;
    else
        color = 0;
    color |= (color << 8) | (color << 16);
    if (render_commands.overlay_head == 0)
        render_commands.overlay_head = render_commands.packet_offset;
    packet = render_commands.packet_offset;
    next_packet = packet + 28;
    mode = (DR_MODE *)psx_addr(packet, sizeof(DR_MODE));
    rect = (TILE *)psx_addr(packet + 12, sizeof(TILE));
    setTile(rect);
    mode->tag = ((packet + 12) & 0xFFFFFF) | 0x02000000;
    mode->code[0] = 0xE1000040;
    mode->code[1] = 0;
    rect->rgbc.rgbc = color | 0x62000000;
    rect->tag = (next_packet & 0xFFFFFF) | 0x03000000;
    rect->p[0].xy = 0;
    xport_store_le32(&rect->w, 0x00F00140);
    render_commands.overlay_tail = packet + 12;
    render_commands.packet_offset = next_packet;
    return 0x03000000;
}

uint32 UpdateScreenEffects(void)
{
    FUNCTION_MARKER(0x8005CF3C, "SLUS_005.78");
    uint32 result, value;
    sint32 fade, numerator, remaining, duration, half, intensity;
    fade = screen_render.transition_ticks;
    if (fade != 0)
    {
        if (fade > 0)
        {
            value = (uint32)fade - 1u;
            numerator = (sint32)(value << 16) >> 4;
            screen_render.transition_ticks = (sint16)(value);
            intensity = numerator / 15;
        }
        else
        {
            value = (uint32)fade + 1u;
            fade = (sint16)value;
            numerator = (sint32)(((uint32)fade + 15u) << 12);
            screen_render.transition_ticks = (sint16)(value);
            intensity = numerator / 15;
            if (fade == 0)
                screen_render.transition_ticks = (sint16)(0xFFFF);
        }
        return DrawScreenDarken(intensity);
    }
    result = 0;
    remaining = (sint32)screen_render.brighten_ticks;
    if (remaining > 0)
    {
        duration = (sint32)screen_render.brighten_duration;
        result = duration < 2;
        if (duration >= 2)
        {
            --remaining;
            screen_render.brighten_ticks = (uint32)remaining;
            half = duration / 2;
            if (remaining > half)
            {
                numerator = (sint32)(((uint32)remaining - (uint32)half) << 12);
                intensity = (sint32)(4096u - (uint32)(numerator / half));
            }
            else
            {
                numerator = (sint32)((uint32)remaining << 12);
                intensity = numerator / half;
            }
            return DrawScreenBrighten(intensity);
        }
    }
    remaining = (sint32)screen_render.darken_ticks;
    if (remaining <= 0)
        return result;
    duration = (sint32)screen_render.darken_duration;
    result = duration < 2;
    if (duration < 2)
        return result;
    --remaining;
    screen_render.darken_ticks = (uint32)remaining;
    half = duration / 2;
    if (remaining > half)
    {
        numerator = (sint32)(((uint32)remaining - (uint32)half) << 12);
        intensity = (sint32)(4096u - (uint32)(numerator / half));
    }
    else
    {
        numerator = (sint32)((uint32)remaining << 12);
        intensity = numerator / half;
    }
    return DrawScreenDarken(intensity);
}

uint32 StartSceneFadeIn(void)
{
    FUNCTION_MARKER(0x80025BE8, "SLUS_005.78");
    screen_render.transition_ticks = 15;
    return 15;
}

uint32 DrawSceneInterface(void)
{
    FUNCTION_MARKER(0x8005D170, "SLUS_005.78");
    uint32 result;
    DrawSceneBackground();
    DrawHud(FindPlayerObject(0));
    result = UpdateScreenEffects();
    NoOp800606F4();
    return result;
}

sint32 DrawStartupImage(uint32 *image)
{
    PSX_RECT origin;
    sint32 x;
    sint32 y;
    PSX_RECT rectangle;
    PSX_RECT image_rect = OriginalDefaults::StartupImageRect;
    FUNCTION_MARKER(0x800669F4, "SLUS_005.78");
    origin = render_buffers[render_commands.active_buffer].disp.screen;
    if (screen_render.startup_image_loaded)
    {
        rectangle.x = (sint16)screen_render.startup_image_x;
        rectangle.y = (sint16)screen_render.startup_image_y;
        rectangle.w = OriginalDefaults::StartupImageRect.w;
        rectangle.h = OriginalDefaults::StartupImageRect.h;
        VSync(0);
        ClearImage(&rectangle, 0, 0, 0);
    }
    else
    {
        LoadImagePSX(&image_rect, image);
    }
    x = origin.x + 112;
    y = origin.y + 84;
    screen_render.startup_image_x = (uint32)x;
    screen_render.startup_image_y = (uint32)y;
    MoveImage(&image_rect, x, y);
    return DrawSync(0);
}

uint32 RestoreMenuImage(uint32 mode)
{
    uint32 active;
    sint32 y;
    PSX_RECT rectangle;
    FUNCTION_MARKER(0x80066AF0, "SLUS_005.78");
    active = render_commands.active_buffer;
    rectangle = OriginalDefaults::MenuImageRect;
    y = render_buffers[1 - active].draw.clip.y;
    rectangle.w = 320;
    rectangle.h = 240;
    MoveImage(&rectangle, 0, y);
    return (uint32)DrawSync(0);
}

uint32 SaveMenuImage(void)
{
    uint32 active;
    sint32 x;
    sint32 y;
    PSX_RECT rectangle;
    FUNCTION_MARKER(0x80066978, "SLUS_005.78");
    DrawSync(0);
    VSync(0);
    active = render_commands.active_buffer;
    x = OriginalDefaults::MenuImageRect.x;
    y = OriginalDefaults::MenuImageRect.y;
    rectangle.x = 0;
    rectangle.y = render_buffers[active].draw.clip.y;
    rectangle.w = 320;
    rectangle.h = 240;
    MoveImage(&rectangle, x, y);
    return (uint32)DrawSync(0);
}

sint32 FadeMenuImage(uint32 color)
{
    uint32 step;
    uint32 previous;
    uint32 deadline;
    uint32 wait;
    sint32 fraction = 30;
    sint32 strength;
    FUNCTION_MARKER(0x80067160, "SLUS_005.78");
    SaveMenuImage();
    frame_clock.frame_start = frame_clock.milliseconds;
    for (step = 2; step < 31; step += 2)
    {
        if (color == 0 || color == 0xFFFFFF)
        {
            previous = frame_clock.frame_start;
            BeginDraw();
            ClearSceneOrderingTable();
            RestoreMenuImage(1);
            strength = (sint32)(step << 12) / 30;
            if (color == 0)
                DrawOverlayRectangle(0, 0xF00140, strength, 0xFFFFFF);
            else
                DrawBrightenRectangle(0, 0xF00140, strength, 0xFFFFFF);
            EndDraw();
        }
        else
        {
            BeginDraw();
            ClearSceneOrderingTable();
            RestoreMenuImage(1);
            strength = (sint32)(step << 12) / 30;
            DrawOverlayRectangle(0, 0xF00140, strength, ~color | 0xFFFFFF);
            DrawBrightenRectangle(0, 0xF00140, strength, color);
            EndDraw();
            previous = frame_clock.frame_start;
        }
        fraction -= 10;
        wait = 33;
        if (fraction <= 0)
        {
            wait = 34;
            fraction += 30;
        }
        deadline = previous + wait;
        while (frame_clock.milliseconds < deadline)
            psx_native_poll_events();
        frame_clock.milliseconds;
        frame_clock.frame_start = frame_clock.milliseconds;
    }
    return 0;
}

uint32 DrawBrightenRectangle(uint32 position, uint32 extent, sint32 fraction, uint32 color)
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
    if (render_commands.overlay_head == 0)
        render_commands.overlay_head = (render_commands.packet_offset);
    packet = render_commands.packet_offset;
    next = packet + 12;
    DR_MODE *mode = (DR_MODE *)psx_addr(packet, sizeof(DR_MODE));
    TILE *rect = (TILE *)psx_addr(next, sizeof(TILE));
    mode->tag = (next & 0xFFFFFF) | 0x02000000;
    mode->code[0] = 0xE1000020;
    mode->code[1] = 0;
    rect->rgbc.rgbc = color | 0x62000000;
    rect->tag = ((packet + 28) & 0xFFFFFF) | 0x03000000;
    rect->p[0].xy = position;
    xport_store_le32(&rect->w, extent);
    render_commands.overlay_tail = (next);
    render_commands.packet_offset = (packet + 28);
    return 0x03000000;
}

uint32 UploadTextureImage(uint8 *image, uint32 index, uint32 flags)
{
    PSX_RECT rectangle;
    uint32 offset;
    uint32 result;
    sint32 status;
    FUNCTION_MARKER(0x8006624C, "SLUS_005.78");
    rectangle = OriginalDefaults::TextureImageRects[index];
    LoadImagePSX(&rectangle, (uint32 *)image);
    if (index == 0)
    {
        offset = resource_archive.Offset(252);
        rectangle = OriginalDefaults::TextureImageRects[8];
        LoadImagePSX(&rectangle, (uint32 *)(image + offset));
    }
    if ((flags << 24) != 0)
    {
        do
        {
            status = DrawSync(1);
        } while (status > 0);
    }
    result = index < 2;
    if (game_flow.mode_flags & 0x800)
    {
        result = 9;
        if (index >= 2)
        {
            result = 8 * index;
            if (index != 9)
            {
                rectangle = OriginalDefaults::TextureImageRects[8];
                MoveImage(&rectangle, OriginalDefaults::TextureImageRects[index].x, OriginalDefaults::TextureImageRects[index].y);
                result = flags << 24;
                if (result != 0)
                {
                    do
                    {
                        result = (uint32)DrawSync(1);
                    } while ((sint32)result > 0);
                }
            }
        }
    }
    return result;
}

void NoOp800606F4(void)
{
    FUNCTION_MARKER(0x800606F4, "SLUS_005.78");
    /* Original JR RA/NOP has no side effects */
}

uint32 DrawGameOverFade(void)
{
    FUNCTION_MARKER(0x80067778, "SLUS_005.78");
    sint32 remaining = 30, fraction = 30;
    uint32 fade = 273, result, delay, deadline;
    SaveMenuImage();
    frame_clock.frame_start = frame_clock.milliseconds;
    do
    {
        uint32 style = fade < 4096 ? (fade << 16) | 1 : 1;
        BeginDraw();
        ClearSceneOrderingTable();
        DrawBackgroundRectangle(0, 0xF00140, 0);
        RestoreMenuImage(1);
        DrawText(0x7800A0, UiText::GameOver, style, 0);
        EndDraw();
        delay = 33;
        fraction -= 10;
        if (fraction <= 0)
        {
            delay = 34;
            fraction += 30;
        }
        deadline = frame_clock.frame_start + delay;
        while (frame_clock.milliseconds < deadline)
            psx_native_poll_events();
        result = frame_clock.milliseconds;
        frame_clock.frame_start = result;
        --remaining;
        fade += 273;
    } while (remaining != 0);
    return result;
}
