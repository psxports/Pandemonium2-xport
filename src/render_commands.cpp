#include "game.h"

const RenderBuffer render_buffer_defaults[2] = {{{0}, {0}, 0x8008B720, 0x800A1DF0, 0x8009FCF0, 0x800A1CF0, 0x800A52C0}, {{0}, {0}, 0x80095A08, 0x800A1DF0, 0x800A0CF0, 0x800A1D70, 0x800A52C0}};

RenderBuffer render_buffers[2] = {0};

RenderCommands render_commands = {0};

uint32 BeginDraw(void)
{
    uint32 previous;
    RenderBuffer *buffer;
    uint32 draw_list;
    uint32 primitives;
    uint32 next_primitives;
    uint32 other_primitives;
    uint32 order_table;
    FUNCTION_MARKER(0x800665D4, "SLUS_005.78");
    previous = render_commands.nesting;
    render_commands.nesting = (uint8)(previous + 1);
    if (previous != 0)
        return 1;
    buffer = render_buffers + (1 - (uint32)render_commands.active_buffer);
    draw_list = buffer->ordering_table_offset;
    order_table = buffer->small_ordering_table_offset;
    render_commands.ordering_table_offset = (draw_list);
    render_commands.small_ordering_table_offset = (order_table);
    p2_sdk_800804bc(render_commands.ordering_table_offset, 1024);
    p2_sdk_800804bc(render_commands.small_ordering_table_offset, 32);
    other_primitives = render_commands.active_buffer;
    primitives = buffer->packet_base;
    next_primitives = buffer->aux_packet_base;
    other_primitives = render_buffers[other_primitives].packet_base;
    order_table = buffer->scene_ordering_table_offset;
    render_commands.overlay_head = (0);
    render_commands.packet_offset = (primitives);
    render_commands.packet_limit = (primitives + 41600);
    render_commands.aux_packet_offset = (next_primitives);
    render_commands.scene_ordering_table_offset = (order_table);
    render_commands.other_packet_base = (other_primitives);
    return 41600;
}

uint32 EndDraw(void)
{
    uint32 nesting;
    RenderBuffer *buffer;
    uint32 chain = 0;
    uint32 tail = 0;
    uint32 primitives;
    uint32 first;
    uint32 tag;
    uint32 active;
    FUNCTION_MARKER(0x80066698, "SLUS_005.78");
    nesting = render_commands.nesting;
    if (nesting == 0)
        return nesting;
    nesting = (nesting - 1) & 255;
    render_commands.nesting = (uint8)(nesting);
    if (nesting != 0)
        return nesting;
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
    primitives = render_commands.packet_offset;
    if (primitives != buffer->packet_base)
    {
        first = render_commands.overlay_head;
        if (primitives != first)
        {
            if (chain == 0)
                chain = render_commands.ordering_table_offset + 4092;
            if (tail != 0)
            {
                tag = r_u32(tail) & 0xFF000000;
                w_u32(tail, tag);
                w_u32(tail, tag | ((render_commands.ordering_table_offset + 4092) & 0xFFFFFF));
            }
            tail = render_commands.ordering_table_offset;
            if (chain == 0)
                chain = render_commands.small_ordering_table_offset + 124;
            if (tail != 0)
            {
                tag = r_u32(tail) & 0xFF000000;
                w_u32(tail, tag);
                w_u32(tail, tag | ((render_commands.small_ordering_table_offset + 124) & 0xFFFFFF));
            }
            tail = render_commands.small_ordering_table_offset;
            first = render_commands.overlay_head;
        }
    }
    else
        first = render_commands.overlay_head;
    if (first != 0)
    {
        if (chain == 0)
            chain = first;
        if (tail != 0)
        {
            tag = r_u32(tail) & 0xFF000000;
            w_u32(tail, tag);
            w_u32(tail, tag | (render_commands.overlay_head & 0xFFFFFF));
        }
        tail = render_commands.overlay_tail;
    }
    w_u32(tail, r_u32(tail) | 0xFFFFFF);
    render_commands.submitted_chain_offset = (chain);
    PutDrawEnv(&buffer->draw);
    render_commands.pending_draw = (1);
    DrawOTag((uint32 *)psx_addr(chain, sizeof(uint32)));
    active = render_commands.active_buffer;
    render_commands.active_buffer = (uint8)(1 - active);
    return active;
}

uint32 ClearSceneOrderingTable(void)
{
    FUNCTION_MARKER(0x800665B0, "SLUS_005.78");
    return p2_sdk_800804bc(render_commands.scene_ordering_table_offset, 601);
}

DISPENV *SubmitOverlayFrame(uint32 first_ot_offset, uint32 second_ot_offset)
{
    FUNCTION_MARKER(0x800668A0, "SLUS_005.78");
    uint32 index;
    if (render_commands.pending_draw)
        DrawSync(0);
    if (render_commands.pending_disp_env)
        VSync(0);
    index = (1 - render_commands.active_buffer) & 0xFF;
    render_commands.active_buffer = (uint8)(index);
    PutDrawEnv(&render_buffers[index].draw);
    DrawOTag((uint32 *)psx_addr(first_ot_offset, 4));
    if (second_ot_offset)
        DrawOTag((uint32 *)psx_addr(second_ot_offset, 4));
    DrawSync(0);
    VSync(0);
    index = render_commands.active_buffer;
    return PutDispEnv(&render_buffers[index].disp);
}

void InitializeRenderBuffers(void)
{
    uint32 packet_base;
    RenderBuffer *buffer;
    RenderBuffer *active;
    uint32 index;
    sint32 y;
    PSX_RECT rectangle;
    FUNCTION_MARKER(0x8006645C, "SLUS_005.78");
    p2_reset_graph_zero();
    buffer = render_buffers;
    y = 0;
    for (index = 0; index < 2; ++index)
    {
        SetDefDrawEnv(&buffer->draw, 0, y, 320, 240);
        SetDefDispEnv(&buffer->disp, 0, y, 320, 240);
        y += 240;
        ++buffer;
    }
    active = render_buffers + render_commands.active_buffer;
    packet_base = active->packet_base;
    render_commands.pending_disp_env = &active->disp;
    rectangle.x = 0;
    rectangle.y = 0;
    rectangle.w = 1024;
    rectangle.h = 512;
    render_commands.other_packet_base = packet_base;
    ClearImage(&rectangle, 0, 0, 0);
    DrawSync(0);
    SetDispMask(1);
    psx_bios_vsync_callback(ShowPendingDisplay);
    p2_sdk_8007febc(MarkDrawComplete);
}

void ShowPendingDisplay(void)
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

void MarkDrawComplete(void)
{
    uint32 index;
    FUNCTION_MARKER(0x80066C40, "SLUS_005.78");
    if (render_commands.pending_draw != 0)
    {
        index = render_commands.active_buffer;
        render_commands.pending_draw = 0;
        render_commands.pending_disp_env = &render_buffers[index].disp;
    }
}

uint32 ResetRenderCommands(void)
{
    FUNCTION_MARKER(0x8006657C, "SLUS_005.78");
    SetDispMask(0);
    render_commands.active_buffer = (uint8)(0);
    render_commands.pending_draw = (0);
    render_commands.pending_disp_env = (0);
    return p2_reset_graph_zero();
}

void NoOp80066CE8(void)
{
    FUNCTION_MARKER(0x80066CE8, "SLUS_005.78");
    /* Remove the stray event call that inherits a freed heap pointer as its class */
}
