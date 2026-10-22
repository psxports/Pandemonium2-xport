#ifndef BACKGROUND_RESOURCE_H
#define BACKGROUND_RESOURCE_H
#include <stddef.h>

#include "psx.h"

struct BackgroundTile;
struct EncodedBackgroundHeader;
struct BackgroundCommand;
struct BackgroundResource;

struct BackgroundTile
{
    uint32 uv;
    uint32 attributes;
};

struct EncodedBackgroundHeader
{
    uint16 metadata, frame_idx, frame_count, frame_rate;
    uint32 commands_offset, cursor_offset;
    uint32 last_tick, start_time;
    uint16 width, height;
    uint32 map_offset;
    uint16 metadata_halfword, tile_count;
    uint32 tiles_offset;
};

struct BackgroundCommand
{
    uint32 command;
};

// Own file bytes and mutable background animation state
struct BackgroundResource
{
    uint8 *data;
    uint32 bytes;
    uint16 metadata;
    uint16 frame_idx;
    uint16 frame_count;
    uint16 frame_rate;
    const uint8 *commands;
    const uint8 *command_cursor;
    const uint8 *command_end;
    uint32 last_tick;
    uint32 start_time;
    uint16 width;
    uint16 height;
    const uint8 *static_map;
    uint16 metadata_halfword;
    uint16 tile_count;
    BackgroundTile *tiles;

    static void DrawSceneBackground(void);
    BackgroundResource *Relocate();
};

#endif
