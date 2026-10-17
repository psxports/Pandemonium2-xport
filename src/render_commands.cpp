#include "game.h"

RenderCommands render_commands = {0};

uint32 BeginDraw(void)
{
    uint32 previous;
    uint32 buffer;
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
    buffer = 0x80087200 + 132 * (1 - (uint32)render_commands.active_buffer);
    draw_list = r_u32(buffer + 120);
    order_table = r_u32(buffer + 124);
    render_commands.ordering_table_offset = (draw_list);
    render_commands.small_ordering_table_offset = (order_table);
    p2_sdk_800804bc(render_commands.ordering_table_offset, 1024);
    p2_sdk_800804bc(render_commands.small_ordering_table_offset, 32);
    other_primitives = render_commands.active_buffer;
    primitives = r_u32(buffer + 112);
    next_primitives = r_u32(buffer + 116);
    other_primitives = r_u32(0x80087270 + 132 * other_primitives);
    order_table = r_u32(buffer + 128);
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
    uint32 buffer;
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
    if (render_commands.pending_display_env_offset != 0)
        VSync(0);
    buffer = 0x80087200 + 132 * (1 - (uint32)render_commands.active_buffer);
    if (render_commands.aux_packet_offset != r_u32(buffer + 116))
    {
        tail = render_commands.scene_ordering_table_offset;
        chain = tail + 2400;
    }
    primitives = render_commands.packet_offset;
    if (primitives != r_u32(buffer + 112))
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
    PutDrawEnv((DRAWENV *)psx_addr(buffer, sizeof(DRAWENV)));
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
