#include "render_commands.h"
#include "game.h"

const RenderBuffer render_buffer_defaults[2] = {{{0}, {0}, 0x8008B720, 0x800A1DF0, 0x8009FCF0, 0x800A1CF0, 0x800A52C0}, {{0}, {0}, 0x80095A08, 0x800A1DF0, 0x800A0CF0, 0x800A1D70, 0x800A52C0}};

RenderBuffer render_buffers[2] = {0};

RenderCommands render_commands = {0};

void RenderCommands::InitializeRenderBuffers(void)
{
    uint32 packet_base;
    RenderBuffer *buffer;
    RenderBuffer *active;
    uint32 idx;
    sint32 y;
    PSX_RECT rect;
    FUNCTION_MARKER(0x8006645C, "SLUS_005.78");
    ResetGraphDefault();
    buffer = render_buffers;
    y = 0;
    for (idx = 0; idx < 2; ++idx)
    {
        SetDefDrawEnv(&buffer->draw, 0, y, 320, 240);
        SetDefDispEnv(&buffer->disp, 0, y, 320, 240);
        y += 240;
        ++buffer;
    }
    active = render_buffers + render_commands.active_buffer;
    packet_base = active->packet_base;
    render_commands.pending_disp_env = &active->disp;
    rect.x = 0;
    rect.y = 0;
    rect.w = 1024;
    rect.h = 512;
    render_commands.other_packet_base = packet_base;
    ClearImage(&rect, 0, 0, 0);
    DrawSync(0);
    SetDispMask(1);
    psx_bios_vsync_callback(RenderCommands::ShowPendingDisp);
    SetGraphDrawSyncCallback(RenderCommands::MarkDrawComplete);
}

void RenderCommands::ResetRenderCommands(void)
{
    FUNCTION_MARKER(0x8006657C, "SLUS_005.78");
    SetDispMask(0);
    render_commands.active_buffer = 0;
    render_commands.pending_draw = 0;
    render_commands.pending_disp_env = 0;
    ResetGraphDefault();
}

void RenderCommands::ClearSceneOrderingTable(void)
{
    FUNCTION_MARKER(0x800665B0, "SLUS_005.78");
    ClearReverseOrderingTable(render_commands.scene_ordering_table_offset, 601);
}

void RenderCommands::BeginDraw(void)
{
    uint8 prev;
    RenderBuffer *buffer;
    uint32 draw_list;
    uint32 prims;
    uint32 next_prims;
    uint32 other_prims;
    uint32 order_table;
    FUNCTION_MARKER(0x800665D4, "SLUS_005.78");
    prev = render_commands.nesting;
    render_commands.nesting = prev + 1;
    if (prev != 0)
        return;
    buffer = render_buffers + (1 - (uint32)render_commands.active_buffer);
    draw_list = buffer->ordering_table_offset;
    order_table = buffer->small_ordering_table_offset;
    render_commands.ordering_table_offset = draw_list;
    render_commands.small_ordering_table_offset = order_table;
    ClearReverseOrderingTable(render_commands.ordering_table_offset, 1024);
    ClearReverseOrderingTable(render_commands.small_ordering_table_offset, 32);
    other_prims = render_commands.active_buffer;
    prims = buffer->packet_base;
    next_prims = buffer->aux_packet_base;
    other_prims = render_buffers[other_prims].packet_base;
    order_table = buffer->scene_ordering_table_offset;
    render_commands.overlay_head = 0;
    render_commands.packet_offset = prims;
    render_commands.packet_limit = prims + 41600;
    render_commands.aux_packet_offset = next_prims;
    render_commands.scene_ordering_table_offset = order_table;
    render_commands.other_packet_base = other_prims;
}

void RenderCommands::EndDraw(void)
{
    uint8 nesting;
    RenderBuffer *buffer;
    uint32 chain = 0;
    uint32 tail = 0;
    uint32 prims;
    uint32 overlay;
    uint32 tag;
    uint32 *tail_tag;
    uint32 active;
    FUNCTION_MARKER(0x80066698, "SLUS_005.78");
    nesting = render_commands.nesting;
    if (nesting == 0)
        return;
    --nesting;
    render_commands.nesting = nesting;
    if (nesting != 0)
        return;
    if (render_commands.pending_draw != 0)
        DrawSync(0);
    if (render_commands.pending_disp_env != 0)
        VSync(0);
    buffer = render_buffers + (1 - (uint32)render_commands.active_buffer);
    if (render_commands.aux_packet_offset != buffer->aux_packet_base)
    {
        tail = render_commands.scene_ordering_table_offset;
        chain = tail + 2400;
    }
    prims = render_commands.packet_offset;
    if (prims != buffer->packet_base)
    {
        overlay = render_commands.overlay_head;
        if (prims != overlay)
        {
            if (chain == 0)
                chain = render_commands.ordering_table_offset + 4092;
            if (tail != 0)
            {
                tail_tag = (uint32 *)psx_addr(tail, sizeof(uint32));
            tag = *tail_tag & 0xFF000000;
                *tail_tag = tag;
                *tail_tag = tag | ((render_commands.ordering_table_offset + 4092) & 0xFFFFFF);
            }
            tail = render_commands.ordering_table_offset;
            if (chain == 0)
                chain = render_commands.small_ordering_table_offset + 124;
            if (tail != 0)
            {
                tail_tag = (uint32 *)psx_addr(tail, sizeof(uint32));
            tag = *tail_tag & 0xFF000000;
                *tail_tag = tag;
                *tail_tag = tag | ((render_commands.small_ordering_table_offset + 124) & 0xFFFFFF);
            }
            tail = render_commands.small_ordering_table_offset;
            overlay = render_commands.overlay_head;
        }
    }
    else
        overlay = render_commands.overlay_head;
    if (overlay != 0)
    {
        if (chain == 0)
            chain = overlay;
        if (tail != 0)
        {
            tail_tag = (uint32 *)psx_addr(tail, sizeof(uint32));
            tag = *tail_tag & 0xFF000000;
            *tail_tag = tag;
            *tail_tag = tag | (render_commands.overlay_head & 0xFFFFFF);
        }
        tail = render_commands.overlay_tail;
    }
    tail_tag = (uint32 *)psx_addr(tail, sizeof(uint32));
    *tail_tag |= 0xFFFFFF;
    render_commands.submitted_chain_offset = chain;
    PutDrawEnv(&buffer->draw);
    render_commands.pending_draw = 1;
    DrawOTag((uint32 *)psx_addr(chain, sizeof(uint32)));
    active = render_commands.active_buffer;
    render_commands.active_buffer = 1 - active;
}

DISPENV *RenderCommands::SubmitOverlayFrame(uint32 ot0_offset, uint32 ot1_offset)
{
    FUNCTION_MARKER(0x800668A0, "SLUS_005.78");
    uint32 idx;
    if (render_commands.pending_draw)
        DrawSync(0);
    if (render_commands.pending_disp_env)
        VSync(0);
    idx = (1 - render_commands.active_buffer) & 0xFF;
    render_commands.active_buffer = idx;
    PutDrawEnv(&render_buffers[idx].draw);
    DrawOTag((uint32 *)psx_addr(ot0_offset, 4));
    if (ot1_offset)
        DrawOTag((uint32 *)psx_addr(ot1_offset, 4));
    DrawSync(0);
    VSync(0);
    idx = render_commands.active_buffer;
    return PutDispEnv(&render_buffers[idx].disp);
}

void RenderCommands::MarkDrawComplete(void)
{
    uint32 idx;
    FUNCTION_MARKER(0x80066C40, "SLUS_005.78");
    if (render_commands.pending_draw != 0)
    {
        idx = render_commands.active_buffer;
        render_commands.pending_draw = 0;
        render_commands.pending_disp_env = &render_buffers[idx].disp;
    }
}

void RenderCommands::ShowPendingDisp(void)
{
    DISPENV *environment;
    FUNCTION_MARKER(0x80066C7C, "SLUS_005.78");
    environment = render_commands.pending_disp_env;
    if (environment != 0)
    {
        PutDispEnv(environment);
        render_commands.pending_disp_env = 0;
    }
}

void RenderCommands::EmptyRenderOperation(void)
{
    FUNCTION_MARKER(0x80066CE8, "SLUS_005.78");
    /* Remove the stray event call that inherits a freed heap pointer as its class */
}
