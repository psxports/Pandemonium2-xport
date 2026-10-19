#ifndef RENDER_COMMANDS_H
#define RENDER_COMMANDS_H

#include "psx.h"

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
    unsigned int ordering_table_offset;
    unsigned int scene_ordering_table_offset;
    unsigned int other_packet_base;
    unsigned int small_ordering_table_offset;
    unsigned int packet_offset;
    unsigned int packet_limit;
    unsigned int aux_packet_offset;
    unsigned int overlay_head;
    unsigned int overlay_tail;
    DISPENV *pending_disp_env;
    unsigned int submitted_chain_offset;
    unsigned int pending_draw;
    unsigned char nesting;
    unsigned char active_buffer;
};

extern RenderCommands render_commands;

DISPENV *SubmitOverlayFrame(uint32 first_ot_offset, uint32 second_ot_offset);

extern "C"
{
    void InitializeRenderBuffers(void);
    void ShowPendingDisplay(void);
    void MarkDrawComplete(void);
}

#endif
