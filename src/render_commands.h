#ifndef RENDER_COMMANDS_H
#define RENDER_COMMANDS_H

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
    unsigned int pending_display_env_offset;
    unsigned int submitted_chain_offset;
    unsigned int pending_draw;
    unsigned char nesting;
    unsigned char active_buffer;
};

extern RenderCommands render_commands;

#endif
