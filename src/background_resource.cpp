#include <stdio.h>
#include <stdlib.h>

#include "background_resource.h"
#include "game.h"
#include "screen_render_state.h"

void BackgroundResource::DrawSceneBackground(void)
{
    uint32 *ot_entry;
    FUNCTION_MARKER(0x8005FB40, "SLUS_005.78");
    SceneSection *section;
    BackgroundResource *background;
    const uint8 *map;
    uint16 *map_dst;
    BackgroundTile *table;
    const BackgroundTile *definition;
    const uint8 *cursor;
    const uint8 *map_cursor, *row_start;
    uint32 saved_color, width, height, command, kind, count;
    uint32 map_offset;
    uint32 idx, elapsed, tick, dst, angle, pal, tile, ordering, packet;
    uint32 uv0, uv1, attributes, tmp, xy, next_xy, next_row, result, remaining;
    sint32 vertical;
    sint32 horizontal;
    sint32 row;
    sint32 column;
    sint32 x;
    uint32 y;
    sint32 rows;
    sint32 columns;
    sint32 cur_column;

    result = game_flow.mode_flags & 0x800;
    if (result != 0)
        return;
    RenderCommands::ClearSceneOrderingTable();
    if (scr_render.fade_level == 4096)
    {
        section = scene_stream.active_section;
        saved_color = section->background_top_color;
        section->background_top_color = 0;
        ScreenRenderState::DrawBackgroundBorders(240, 1);
        section = scene_stream.active_section;
        section->background_top_color = saved_color;
        return;
    }
    section = scene_stream.active_section;
    background = section->background;
    if (background == 0)
    {
        ScreenRenderState::DrawBackgroundRect(0, 0x00F00140, section->background_top_color);
        return;
    }
    if ((section->state_flags & 0x40) != 0)
    {
        section->state_flags &= 0xFFBF;
        tmp = section->pal_bank;
        table = background->tiles;
        remaining = background->tile_count;
        if (tmp == 2)
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
    if (scr_render.background_proj != 0x000E9555)
    {
        scr_render.background_proj = 0x000E9555;
        result = SquareRoot12(0x00170C59);
        result = AsinAngle(30720, result);
        scr_render.background_angle = (-result) & 0xFFF;
    }
    angle = (1024u + camera_proj.rotation[0] + scr_render.background_angle) & 0xFFF;
    if (angle >= 0x801)
        angle |= 0xF000;
    vertical = (640 * (sint16)angle) / 2048;
    row = vertical / 16 - ((sint32)(40u - height) >> 1);
    y = -(vertical & 15);
    ScreenRenderState::DrawBackgroundBorders((y - ((uint32)row << 4) - 1u), height << 4);
    angle = (-(sint16)camera_proj.rotation[1]) & 0xFFF;
    horizontal = (sint32)(angle * width) / 256;
    column = horizontal / 16;
    x = -(horizontal & 15);
    map = background->static_map;

    if (background->frame_count != 0)
    {
        elapsed = frame_clock.milliseconds - background->start_time;
        tick = (elapsed * background->frame_rate) / 1000u;
        if (tick != background->last_tick)
        {
            idx = (background->frame_idx + 1u) & 0xFFFF;
            count = background->frame_count;
            background->last_tick = tick;
            background->frame_idx = idx;
            if (count < idx)
            {
                cursor = background->commands;
                background->frame_idx = 0;
            }
            else
                cursor = background->command_cursor;
            for (;;)
            {
                command = ((const BackgroundCommand *)cursor)->command;
                cursor += 4;
                if (command == 0)
                    break;
                kind = command >> 30;

                if (kind == 0)
                {
                    count = (((command >> 20) & 0x3FF) + 1u) * (((command >> 10) & 0x3FF) + 1u) * 2u;
                    CopyBytes(demo_background_workspace, cursor, count);
                    cursor += count;
                    break;
                }
                count = (command & 0x3FF) + 1u;
                if (kind == 1 || kind == 2)
                {
                    map_offset = 2u * (((command >> 20) & 0x3FF) * height + ((command >> 10) & 0x3FF));

                    map_dst = (uint16 *)(demo_background_workspace + map_offset);
                    if (kind == 1)
                    {
                        do
                        {
                            tmp = xport_load_le16(cursor);
                            cursor += 2;
                            --count;
                            *map_dst = tmp;
                            if (count != 0)
                                map_dst += height;
                        } while (count != 0);
                    }
                    else
                    {
                        count *= 2;
                        CopyBytes((uint8 *)map_dst, cursor, count);
                        cursor += count;
                    }
                }
            }
            background->command_cursor = cursor;
        }
    }
    if (column < 0)
        column = column + width;
    if (row < 0)
    {
        y = y - ((uint32)row << 4);
        row = 0;
    }
    idx = level_resource.level_id;
    if (idx == 78 || (idx == 14 && scene_stream.active_section->section_idx == 0))
    {
        idx = scr_render.background_scroll;
        column = idx + column;
        scr_render.background_scroll = idx + 1;
        if (((idx + 1) & 0xFF) == width)
            scr_render.background_scroll = 0;
        if (column >= (sint32)width)
            column = column - width;
    }
    rows = (sint32)(255u - y) / 16;
    vertical = height - row;
    if (vertical < rows)
        rows = vertical;

    if (rows <= 0)
        return;
    result = 335u - x;
    columns = (sint32)result / 16;
    if ((sint32)result < 0)
        result += 15;
    if (columns <= 0)
        return;
    next_row = (y << 16) + 0x100000;
    table = background->tiles;
    row_start = map + 2u * column * height + 2u * row;
    do
    {
        cur_column = column;
        map_cursor = row_start;
        horizontal = x;
        remaining = columns;
        do
        {
            if (cur_column == (sint32)width)
            {
                cur_column = 0;
                map_cursor = map + 2u * row;
            }
            tile = xport_load_le16(map_cursor);
            xy = ((uint32)horizontal & 0xFFFF) | (y << 16);
            idx = (tile & 0x1FFF) * 8;
            definition = table + (tile & 0x1FFF);
            ordering = render_commands.scene_ordering_table_offset + (idx >> 1);
            ot_entry = (uint32 *)psx_addr(ordering + 4, sizeof(uint32));
            if ((tile & 0xC000) != 0)
            {
                uv0 = definition->uv;
                attributes = definition->attributes;
                uv1 = uv0 >> 16;
                if ((tile & 0x4000) != 0)
                {
                    tmp = uv1 & 0xFF;
                    uv1 = (uv1 & 0xFFFFFF00) | (uv0 & 0xFF);
                    uv0 = (uv0 & 0xFFFFFF00) | tmp;
                }
                if ((tile & 0x8000) != 0)
                {
                    tmp = uv1 & 0xFF00;
                    uv1 = (uv1 & 0xFFFF00FF) | (uv0 & 0xFF00);
                    uv0 = (uv0 & 0xFFFF00FF) | tmp;
                }
                uv0 = (uv0 & 0xFFFF) | (attributes & 0xFFFF0000);
                pal = (uv1 & 0xFF00) | (uv0 & 0xFF);
                next_xy = ((horizontal + 16u) & 0xFFFF) | next_row;
                tmp = *ot_entry;
                packet = render_commands.aux_packet_offset;
                POLY_FT4 *poly = (POLY_FT4 *)psx_addr(packet, sizeof(POLY_FT4));
                setPolyFT4(poly);
                poly->rgbc.rgbc = 0x2D000000;
                poly->tag = (tmp & 0xFFFFFF) | 0x09000000;
                poly->p[1].xy.xy = (next_xy & 0xFFFF) | (xy & 0xFFFF0000);
                poly->p[1].uv_attr = (uv0 & 0xFF00) | (uv1 & 0xFF) | (attributes << 16);
                poly->p[0].xy.xy = xy;
                poly->p[0].uv_attr = uv0;
                poly->p[2].xy.xy = (xy & 0xFFFF) | (next_xy & 0xFFFF0000);
                poly->p[2].uv_attr = pal;
                poly->p[3].xy.xy = next_xy;
                poly->p[3].uv_attr = uv1;
                tmp = *ot_entry;
                dst = packet + 40;
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
                spr->p[0].uv_attr = (uv0 & 0xFFFF) | (attributes & 0xFFFF0000);
                tmp = *ot_entry;
                dst = packet + 28;
            }
            *ot_entry = (tmp & 0xFF000000) | (packet & 0xFFFFFF);
            render_commands.aux_packet_offset = dst;
            cur_column = cur_column + 1u;
            horizontal = horizontal + 16u;
            --remaining;
            map_cursor += height * 2;
        } while (remaining != 0);
        row = row + 1u;

        next_row += 0x100000;
        y = y + 16u;
        --rows;
        row_start += 2;
    } while (rows != 0);
}

BackgroundResource *BackgroundResource::Relocate()
{
    FUNCTION_MARKER(0x800602F4, "SLUS_005.78");
    const EncodedBackgroundHeader *header = (const EncodedBackgroundHeader *)data;
    uint32 payload = 40;
    uint32 size, pixels, i;
    metadata = header->metadata;
    frame_idx = header->frame_idx;
    frame_count = header->frame_count;
    frame_rate = header->frame_rate;
    last_tick = header->last_tick;
    start_time = header->start_time;
    width = header->width;
    height = header->height;
    metadata_halfword = header->metadata_halfword;
    tile_count = header->tile_count;
    commands = data + 40;
    command_cursor = NULL;
    if (frame_count == 0)
    {
        pixels = (uint32)width * height;
        payload = 44;
        if ((pixels & 1) != 0)
            ++pixels;
        static_map = data + payload;
        size = pixels << 1;
    }
    else
    {
        // The reviewed streams reset their zero cursor on the first frame advance
        static_map = demo_background_workspace;
        size = frame_idx;
    }
    command_end = data + payload + size;
    free(tiles);
    tiles = (BackgroundTile *)malloc((uint32)tile_count * sizeof(BackgroundTile));
    for (i = 0; i < tile_count; ++i)
    {
        tiles[i] = ((const BackgroundTile *)command_end)[i];
    }
    frame_idx = frame_count;
    return this;
}
