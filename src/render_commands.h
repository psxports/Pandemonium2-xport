#ifndef RENDER_COMMANDS_H
#define RENDER_COMMANDS_H
#include <stddef.h>

#include "psx.h"

struct RenderBuffer;
struct RenderCommands;

struct RenderBuffer
{
    DRAWENV draw;
    DISPENV disp;
    uint32 packet_base;
    uint32 aux_packet_base;
    uint32 ordering_table_offset;
    uint32 small_ordering_table_offset;
    uint32 scene_ordering_table_offset;
};

extern const RenderBuffer render_buffer_defaults[2];

extern RenderBuffer render_buffers[2];

struct RenderCommands
{
    uint32 ordering_table_offset;
    uint32 scene_ordering_table_offset;
    uint32 other_packet_base;
    uint32 small_ordering_table_offset;
    uint32 packet_offset;
    uint32 packet_limit;
    uint32 aux_packet_offset;
    uint32 overlay_head;
    uint32 overlay_tail;
    DISPENV *pending_disp_env;
    uint32 submitted_chain_offset;
    uint32 pending_draw;
    uint8 nesting;
    uint8 active_buffer;
    DISPENV *submitted_disp_env;

    static void InitializeRenderBuffers(void);
    static void ResetRenderCommands(void);
    static void ClearSceneOrderingTable(void);
    static void BeginDraw(void);
    static void EndDraw(void);
    static DISPENV *SubmitOverlayFrame(uint32 ot0_offset, uint32 ot1_offset);
    static void MarkDrawComplete(void);
    static void ShowPendingDisp(void);
    static void EmptyRenderOperation(void);
};

extern RenderCommands render_commands;

#endif
