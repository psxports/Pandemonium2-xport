#include "hd_internal.h"

static bool hd_reserve_ui(uint32 count, uint32 passes)
{
    if (count > hd_state.ui_capacity)
    {
        uint32 capacity = hd_state.ui_capacity != 0 ? hd_state.ui_capacity : 1536;
        while (capacity < count)
        {
            if (capacity > 0x7FFFFFFFu)
                return false;
            capacity *= 2;
        }
        if (capacity > (size_t)-1 / sizeof(VID_VERTEX2D))
            return false;
        VID_VERTEX2D *vertices = (VID_VERTEX2D *)realloc(hd_state.ui_vertices, (size_t)capacity * sizeof(VID_VERTEX2D));
        if (vertices == NULL)
            return false;
        hd_state.ui_vertices = vertices;
        hd_state.ui_capacity = capacity;
    }
    if (passes > hd_state.pass_capacity)
    {
        uint32 capacity = hd_state.pass_capacity != 0 ? hd_state.pass_capacity : 32;
        while (capacity < passes)
        {
            if (capacity > 0x7FFFFFFFu)
                return false;
            capacity *= 2;
        }
        if (capacity > (size_t)-1 / sizeof(HdUiPass))
            return false;
        HdUiPass *commands = (HdUiPass *)realloc(hd_state.ui_passes, (size_t)capacity * sizeof(HdUiPass));
        if (commands == NULL)
            return false;
        hd_state.ui_passes = commands;
        hd_state.pass_capacity = capacity;
    }
    return true;
}

void hd_og_release(HdOgDraw *draws, uint32 count)
{
    for (uint32 idx = 0; idx < count; ++idx)
    {
        if (draws[idx].kind == HD_OG_BACKGROUND_TILES && --draws[idx].background->refs == 0)
            free(draws[idx].background);
    }
}

void hd_og_retain(HdOgDraw *draws, uint32 count)
{
    for (uint32 idx = 0; idx < count; ++idx)
        if (draws[idx].kind == HD_OG_BACKGROUND_TILES)
            ++draws[idx].background->refs;
}

static HdOgDraw *hd_og_append(void)
{
    if (!hd_frame.collecting || hd_state.og_count == 0xFFFFFFFFu)
        return NULL;
    HdOgDraw *draws = (HdOgDraw *)hd_grow_sample(hd_state.og_draws, &hd_state.og_capacity, hd_state.og_count + 1, sizeof(HdOgDraw));
    if (draws == NULL)
        return NULL;
    hd_state.og_draws = draws;
    HdOgDraw *draw = draws + hd_state.og_count++;
    memset(draw, 0, sizeof(*draw));
    return draw;
}

bool hd_og_textured_quad(uint32 pos, uint32 extent, const TextureDescriptor *texture, uint32 blend, HdOgQuadEmit emit)
{
    if (texture == NULL || emit == NULL)
        return false;
    HdOgDraw *draw = hd_og_append();
    if (draw == NULL)
        return false;
    draw->kind = HD_OG_QUAD;
    HdOgQuad *quad = &draw->quad;
    quad->pos = pos;
    quad->extent = extent;
    quad->blend = blend;
    quad->texture = *texture;
    // The original UI emitter reads the copied mapping without following src_tex
    quad->texture.src_tex = NULL;
    quad->emit = emit;
    return true;
}

bool hd_og_rect(uint32 pos, uint32 extent, sint32 fraction, uint32 color, HdOgRectEmit emit)
{
    if (emit == NULL)
        return false;
    HdOgDraw *draw = hd_og_append();
    if (draw == NULL)
        return false;
    draw->kind = HD_OG_RECT;
    draw->rect.pos = pos;
    draw->rect.extent = extent;
    draw->rect.fraction = fraction;
    draw->rect.color = color;
    draw->rect.emit = emit;
    return true;
}

bool hd_og_effect(sint32 intensity, HdOgEffectEmit emit)
{
    if (emit == NULL)
        return false;
    HdOgDraw *draw = hd_og_append();
    if (draw == NULL)
        return false;
    draw->kind = HD_OG_EFFECT;
    draw->effect.intensity = intensity;
    draw->effect.emit = emit;
    return true;
}

bool hd_og_scene_fade(uint32 level, HdOgFadeEmit emit)
{
    if (emit == NULL)
        return false;
    HdOgDraw *draw = hd_og_append();
    if (draw == NULL)
        return false;
    draw->kind = HD_OG_SCENE_FADE;
    draw->fade.level = level;
    draw->fade.emit = emit;
    return true;
}

bool hd_og_background_rect(uint32 pos, uint32 extent, uint32 color, HdOgBackgroundRectEmit emit)
{
    if (emit == NULL)
        return false;
    HdOgDraw *draw = hd_og_append();
    if (draw == NULL)
        return false;
    draw->kind = HD_OG_BACKGROUND_RECT;
    draw->background_rect.pos = pos;
    draw->background_rect.extent = extent;
    draw->background_rect.color = color;
    draw->background_rect.emit = emit;
    return true;
}

void hd_og_clear_background(void)
{
    if (!hd_frame.collecting)
        return;
    hd_state.og_background_skipped = false;
    uint32 count = 0;
    for (uint32 idx = 0; idx < hd_state.og_count; ++idx)
    {
        HdOgDraw *draw = hd_state.og_draws + idx;
        if (draw->kind == HD_OG_BACKGROUND_TILES || draw->kind == HD_OG_BACKGROUND_RECT)
            hd_og_release(draw, 1);
        else
            hd_state.og_draws[count++] = *draw;
    }
    hd_state.og_count = count;
}

bool hd_og_skip_background(void)
{
    if (!hd_frame.active || !hd_frame.collecting || hd_frame.failed)
        return false;
    hd_state.og_background_skipped = true;
    return true;
}

bool hd_og_background_pending(void)
{
    return hd_state.og_background_skipped;
}

void hd_og_background_state(void)
{
    if (!hd_state.og_background_skipped)
        return;
    uint32 last = 0xFFFFFFFF, last_mode = 0xFFFFFFFF;
    uint16 page = 0, mode = 0;
    // Reverse OT traversal ends with the earliest insertion in the lowest bucket
    for (uint32 idx = 0; idx < hd_state.og_count; ++idx)
    {
        const HdOgDraw *draw = hd_state.og_draws + idx;
        if (draw->kind != HD_OG_BACKGROUND_TILES)
            continue;
        const HdOgBackground *input = draw->background;
        for (uint32 cell = 0; cell < input->count; ++cell)
        {
            const HdOgBackgroundTile *tile = input->tiles + cell;
            uint32 bucket = tile->tile & 0x1FFF;
            if (bucket < last)
            {
                last = bucket;
                page = tile->definition.attributes;
            }
            if (!(tile->tile & 0xC000) && bucket < last_mode)
            {
                last_mode = bucket;
                mode = tile->definition.attributes;
            }
        }
    }
    if (last_mode != 0xFFFFFFFF)
    {
        DR_MODE packet;
        packet.tag = 0x02FFFFFF;
        packet.code[0] = 0xE1000000 | mode;
        packet.code[1] = 0;
        gpu_packet(&packet);
    }
    if (last != 0xFFFFFFFF)
        gpu_set_polygon_tpage(page);
}

bool hd_og_background_tiles(const BackgroundResource *background, const uint8 *map, sint32 row, sint32 column, sint32 x, uint32 y, sint32 rows, sint32 columns)
{
    if (background == NULL || map == NULL || rows <= 0 || columns <= 0 || row < 0 || column < 0 ||
        background->width == 0 || background->height == 0 || column >= background->width || row >= background->height || rows > background->height - row)
        return false;
    uint64 count = (uint64)rows * columns;
    if (count > 0xFFFFFFFFu || count > ((size_t)-1 - sizeof(HdOgBackground)) / sizeof(HdOgBackgroundTile))
        return false;
    HdOgBackground *input = (HdOgBackground *)malloc(sizeof(HdOgBackground) + (size_t)count * sizeof(HdOgBackgroundTile));
    if (input == NULL)
        return false;
    input->refs = 1;
    input->count = (uint32)count;
    input->tiles = (HdOgBackgroundTile *)(input + 1);
    uint32 cursor = 0;
    for (sint32 line = 0; line < rows; ++line)
    {
        sint32 current = column;
        for (sint32 cell = 0; cell < columns; ++cell)
        {
            if (current == background->width)
                current = 0;
            uint16 tile = ((const uint16 *)map)[(uint32)current * background->height + row + line];
            if ((tile & 0x1FFF) >= background->tile_count)
            {
                free(input);
                return false;
            }
            HdOgBackgroundTile *dst = input->tiles + cursor++;
            dst->tile = tile;
            dst->definition = background->tiles[tile & 0x1FFF];
            dst->xy = (((uint32)x + (uint32)cell * 16) & 0xFFFF) | ((y + (uint32)line * 16) << 16);
            dst->next_xy = (((uint32)x + (uint32)cell * 16 + 16) & 0xFFFF) | ((y + (uint32)line * 16 + 16) << 16);
            ++current;
        }
    }
    HdOgDraw *draw = hd_og_append();
    if (draw == NULL)
    {
        free(input);
        return false;
    }
    draw->kind = HD_OG_BACKGROUND_TILES;
    draw->background = input;
    return true;
}

void hd_og_emit_background_tile(uint32 tile, uint32 xy, uint32 next_xy, const BackgroundTile *definition)
{
    uint32 uv0, uv1, attributes, tmp, pal, packet, dst;
    uint32 ordering = render_commands.scene_ordering_table_offset + (tile & 0x1FFF) * 4;
    uint32 *ot_entry = (uint32 *)psx_addr(ordering + 4, sizeof(uint32));
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
}

bool hd_og_rebuild_background(const HdSnapshot *sample, uint32 base, uint32 limit)
{
    if (!sample->valid || hd_state.frame_active || base >= limit)
        return false;
    uint64 size = 0;
    for (uint32 idx = 0; idx < sample->og_count; ++idx)
    {
        const HdOgDraw *draw = sample->og_draws + idx;
        if (draw->kind == HD_OG_BACKGROUND_RECT)
            size += sizeof(TILE);
        else if (draw->kind == HD_OG_BACKGROUND_TILES)
        {
            const HdOgBackground *input = draw->background;
            for (uint32 cell = 0; cell < input->count; ++cell)
            {
                uint32 tile = input->tiles[cell].tile;
                if ((tile & 0x1FFF) >= 600)
                    return false;
                size += (tile & 0xC000) ? sizeof(POLY_FT4) : sizeof(DR_MODE) + sizeof(SPRT_16);
            }
        }
    }
    // Validate the entire replacement before changing packets or ordering links
    if (size == 0 || size > limit - base)
        return false;
    uint32 table = render_commands.scene_ordering_table_offset;
    uint32 *tail = (uint32 *)psx_addr(table, sizeof(uint32));
    uint32 next = *tail;
    for (uint32 bucket = 1; bucket <= 600; ++bucket)
        *(uint32 *)psx_addr(table + bucket * 4, sizeof(uint32)) = (table + (bucket - 1) * 4) & 0xFFFFFF;
    render_commands.aux_packet_offset = base;
    bool collecting = hd_frame.collecting;
    hd_frame.collecting = false;
    for (uint32 idx = 0; idx < sample->og_count; ++idx)
    {
        const HdOgDraw *draw = sample->og_draws + idx;
        if (draw->kind == HD_OG_BACKGROUND_RECT)
        {
            const HdOgBackgroundRect *rect = &draw->background_rect;
            rect->emit(rect->pos, rect->extent, rect->color);
        }
        else if (draw->kind == HD_OG_BACKGROUND_TILES)
        {
            const HdOgBackground *input = draw->background;
            for (uint32 cell = 0; cell < input->count; ++cell)
            {
                const HdOgBackgroundTile *tile = input->tiles + cell;
                hd_og_emit_background_tile(tile->tile, tile->xy, tile->next_xy, &tile->definition);
            }
        }
    }
    *tail = next;
    hd_frame.collecting = collecting;
    return true;
}

static bool hd_og_replay_ui(const HdSnapshot *sample)
{
    if (!sample->valid || hd_state.frame_active)
        return false;
    bool collecting = hd_frame.collecting;
    hd_frame.collecting = false;
    bool result = true;
    for (uint32 idx = 0; idx < sample->og_count; ++idx)
    {
        const HdOgDraw *draw = sample->og_draws + idx;
        if (draw->kind == HD_OG_BACKGROUND_TILES)
        {
            const HdOgBackground *input = draw->background;
            for (uint32 cell = 0; cell < input->count; ++cell)
            {
                const HdOgBackgroundTile *tile = input->tiles + cell;
                hd_og_emit_background_tile(tile->tile, tile->xy, tile->next_xy, &tile->definition);
            }
            continue;
        }
        if (draw->kind == HD_OG_SCENE_FADE)
        {
            uint32 level = scr_render.fade_level;
            scr_render.fade_level = draw->fade.level;
            draw->fade.emit();
            scr_render.fade_level = level;
            continue;
        }
        if (draw->kind == HD_OG_EFFECT)
        {
            draw->effect.emit(draw->effect.intensity);
            continue;
        }
        if (draw->kind == HD_OG_BACKGROUND_RECT)
        {
            const HdOgBackgroundRect *rect = &draw->background_rect;
            rect->emit(rect->pos, rect->extent, rect->color);
            continue;
        }
        if (draw->kind == HD_OG_RECT)
        {
            const HdOgRect *rect = &draw->rect;
            rect->emit(rect->pos, rect->extent, rect->fraction, rect->color);
            continue;
        }
        const HdOgQuad *quad = &draw->quad;
        if (quad->emit(quad->pos, quad->extent, &quad->texture, quad->blend) == 0)
        {
            result = false;
            break;
        }
    }
    hd_frame.collecting = collecting;
    return result;
}

void hd_og_save_ui(const HdSnapshot *sample)
{
    hd_og_release(hd_state.frozen_og_draws, hd_state.frozen_og_count);
    hd_state.frozen_og_ui_valid = false;
    hd_state.frozen_og_count = 0;
    if (!sample->valid || sample->restore_image || hd_state.frame_active || sample->collection != hd_state.collection)
        return;
    HdOgDraw *draws = (HdOgDraw *)hd_grow_sample(hd_state.frozen_og_draws, &hd_state.frozen_og_capacity, sample->og_count, sizeof(HdOgDraw));
    if (sample->og_count != 0 && draws == NULL)
        return;
    hd_state.frozen_og_draws = draws;
    hd_state.frozen_og_count = sample->og_count;
    if (sample->og_count != 0)
    {
        memcpy(draws, sample->og_draws, sample->og_count * sizeof(HdOgDraw));
        hd_og_retain(draws, sample->og_count);
    }
    hd_state.frozen_og_ui_valid = true;
}

static bool hd_og_restore_ui(void)
{
    HdSnapshot view;
    memset(&view, 0, sizeof(view));
    view.og_draws = hd_state.frozen_og_draws;
    view.og_count = hd_state.frozen_og_count;
    view.valid = hd_state.frozen_og_ui_valid;
    return hd_og_replay_ui(&view);
}

static bool hd_append_ui(const VID_VERTEX2D *vertices, uint32 count, uint32 blend, HdUiLayer layer)
{
    if (!hd_state.frame_active || vertices == NULL || count % 3 != 0 || blend > VID_BLEND_SUBTRACT || layer < HD_BACKGROUND || layer >= HD_UI_LAYERS)
        return false;
    if (count == 0)
        return true;
    if (count > 0xFFFFFFFFu - hd_state.ui_count || hd_state.pass_count == 0xFFFFFFFFu)
        return false;
    if (!hd_reserve_ui(hd_state.ui_count + count, hd_state.pass_count + 1))
        return false;
    memcpy(hd_state.ui_vertices + hd_state.ui_count, vertices, (size_t)count * sizeof(VID_VERTEX2D));
    HdUiPass *pass;
    if (hd_state.pass_count != 0 && hd_state.ui_passes[hd_state.pass_count - 1].blend == blend && hd_state.ui_passes[hd_state.pass_count - 1].layer == layer)
        pass = hd_state.ui_passes + hd_state.pass_count - 1;
    else
    {
        pass = hd_state.ui_passes + hd_state.pass_count++;
        pass->first = hd_state.ui_count;
        pass->count = 0;
        pass->blend = blend;
        pass->layer = layer;
    }
    pass->count += count;
    hd_state.ui_count += count;
    return true;
}

bool hd_ui(const VID_VERTEX2D *vertices, uint32 count, uint32 blend)
{
    return hd_append_ui(vertices, count, blend, HD_OVERLAY);
}

static void hd_ui_corners(VID_VERTEX2D *vertices, float x, float y, float width, float height, uint32 color, HdUiAnchor anchor)
{
    static const uint8 order[6] = {0, 1, 2, 2, 1, 3};
    static const uint16 flags[4] = {HD_UI_COORDS, HD_UI_COORDS | HD_UI_LEFT, HD_UI_COORDS | HD_UI_RIGHT, HD_UI_COORDS | HD_UI_FILL};
    memset(vertices, 0, 6 * sizeof(VID_VERTEX2D));
    for (uint32 idx = 0; idx < 6; ++idx)
    {
        VID_VERTEX2D *vertex = vertices + idx;
        uint32 corner = order[idx];
        vertex->pos[0] = x + ((corner & 1) != 0 ? width : 0.0f);
        vertex->pos[1] = y + ((corner & 2) != 0 ? height : 0.0f);
        vertex->pos[3] = 1.0f;
        vertex->alpha = 1.0f;
        vertex->color[0] = color;
        vertex->color[1] = color >> 8;
        vertex->color[2] = color >> 16;
        vertex->tex[2] = flags[anchor];
    }
}

bool hd_textured_quad(uint32 pos, uint32 extent, const TextureDescriptor *texture, uint32 blend, HdUiAnchor anchor, HdUiLayer layer)
{
    if (texture == NULL || blend > 4 || anchor < HD_ANCHOR_CENTER || anchor > HD_ANCHOR_FILL)
        return false;
    const TextureMapping *mapping = &texture->mapping;
    uint32 width = extent & 0xFFFF;
    uint32 height = extent >> 16;
    float left = mapping->left;
    float top = mapping->top;
    float right = mapping->right;
    float bottom = mapping->bottom;
    if (width == 0)
    {
        width = mapping->right - mapping->left + 1;
        right += 1.0f;
    }
    if (height == 0)
    {
        height = mapping->bottom - mapping->top + 1;
        bottom += 1.0f;
    }
    VID_VERTEX2D vertices[6];
    static const uint8 order[6] = {0, 1, 2, 2, 1, 3};
    hd_ui_corners(vertices, (sint16)pos, (sint16)(pos >> 16), width, height, 0x808080, anchor);
    for (uint32 idx = 0; idx < 6; ++idx)
    {
        vertices[idx].uv[0] = (order[idx] & 1) != 0 ? right : left;
        vertices[idx].uv[1] = (order[idx] & 2) != 0 ? bottom : top;
        vertices[idx].tex[0] = mapping->tpage;
        vertices[idx].tex[1] = mapping->clut;
        vertices[idx].tex[2] |= HD_TEXTURED | HD_RAW_TEXTURE;
        if (blend != 0)
            vertices[idx].tex[2] |= HD_OPAQUE_ONLY;
    }
    if (!hd_append_ui(vertices, 6, VID_BLEND_NONE, layer))
        return false;
    if (blend == 0)
        return true;
    static const uint32 blends[4] = {VID_BLEND_ALPHA, VID_BLEND_ADD, VID_BLEND_SUBTRACT, VID_BLEND_ADD};
    static const float alpha[4] = {0.5f, 1.0f, 1.0f, 0.25f};
    for (uint32 idx = 0; idx < 6; ++idx)
    {
        vertices[idx].tex[2] = (vertices[idx].tex[2] & ~HD_OPAQUE_ONLY) | HD_STP_ONLY;
        vertices[idx].alpha = alpha[blend - 1];
    }
    return hd_append_ui(vertices, 6, blends[blend - 1], layer);
}

bool hd_rect(float x, float y, float width, float height, uint32 color, uint32 blend, HdUiAnchor anchor, HdUiLayer layer)
{
    if (anchor < HD_ANCHOR_CENTER || anchor > HD_ANCHOR_FILL)
        return false;
    VID_VERTEX2D vertices[6];
    hd_ui_corners(vertices, x, y, width, height, color, anchor);
    return hd_append_ui(vertices, 6, blend, layer);
}

bool hd_vram_image(const PSX_RECT *source)
{
    if (source == NULL || source->x < 0 || source->y < 0 || source->w <= 0 || source->h <= 0 ||
        source->x + source->w > 1024 || source->y + source->h > 512)
        return false;
    static const uint8 order[6] = {0, 1, 2, 2, 1, 3};
    sint32 top = 0;
    while (top < source->h)
    {
        sint32 v = (source->y + top) & 255;
        sint32 height = source->h - top;
        if (height > 256 - v)
            height = 256 - v;
        sint32 left = 0;
        while (left < source->w)
        {
            sint32 x = source->x + left;
            sint32 u = x & 63;
            sint32 width = source->w - left;
            if (width > 256 - u)
                width = 256 - u;
            VID_VERTEX2D vertices[6];
            hd_ui_corners(vertices, left, top, width, height, 0x808080, HD_ANCHOR_CENTER);
            for (uint32 idx = 0; idx < 6; ++idx)
            {
                vertices[idx].uv[0] = u + ((order[idx] & 1) != 0 ? width : 0);
                vertices[idx].uv[1] = v + ((order[idx] & 2) != 0 ? height : 0);
                vertices[idx].tex[0] = (x >> 6) | (((source->y + top) >> 8) << 4) | (2 << 7);
                vertices[idx].tex[2] |= HD_TEXTURED | HD_RAW_TEXTURE | HD_IMAGE_OPAQUE;
            }
            if (!hd_append_ui(vertices, 6, VID_BLEND_NONE, HD_BACKGROUND))
                return false;
            left += width;
        }
        top += height;
    }
    return true;
}

bool hd_background(const BackgroundResource *background, const uint16 *map, sint32 pitch, sint32 yaw, sint32 pitch_offset, uint32 scroll, uint32 top_color, uint32 bottom_color)
{
    if (!hd_state.frame_active)
        return false;
    if (background == NULL)
        return hd_rect(0, 0, 320, 240, top_color, VID_BLEND_NONE, HD_ANCHOR_FILL, HD_BACKGROUND);
    if (background->width == 0 || background->height == 0 || map == NULL || background->tiles == NULL)
        return false;
    HdBackdrop *backdrop = &hd_state.backdrop;
    backdrop->source = background;
    backdrop->pitch = (float)(pitch & 4095);
    backdrop->yaw = (float)(yaw & 4095);
    if (hd_camera_aim_valid() && pitch == camera_proj.rotation[0] && yaw == camera_proj.rotation[1])
    {
        backdrop->pitch = fmodf(hd_state.precise_camera.angle[0] + 4096.0f, 4096.0f);
        backdrop->yaw = fmodf(hd_state.precise_camera.angle[1] + 4096.0f, 4096.0f);
    }
    backdrop->width = background->width;
    float margin_x = 16.0f, margin_y = 16.0f;
    const HdSnapshot *previous = hd_state.samples + hd_state.latest_sample;
    if (previous->valid && previous->backdrop.source == background)
    {
        margin_x += fabsf(hd_angle_delta(previous->backdrop.yaw, backdrop->yaw)) * background->width / 256.0f;
        margin_y += fabsf(hd_angle_delta(previous->backdrop.pitch, backdrop->pitch)) * 640.0f / 2048.0f;
    }
    float angle = fmodf(1024.0f + backdrop->pitch + pitch_offset, 4096.0f);
    if (angle < 0.0f)
        angle += 4096.0f;
    if (angle > 2048.0f)
        angle -= 4096.0f;
    float vertical = 640.0f * angle / 2048.0f;
    float top = ((40 - (sint32)background->height) >> 1) * 16.0f - vertical;
    float bottom = top + background->height * 16.0f;
    if (top >= -margin_y && !hd_rect(0, -margin_y, 320, top + margin_y, top_color, VID_BLEND_NONE, HD_ANCHOR_FILL, HD_BACKGROUND))
        return false;
    if (bottom < 240.0f + margin_y && !hd_rect(0, bottom, 320, 240.0f + margin_y - bottom, bottom_color, VID_BLEND_NONE, HD_ANCHOR_FILL, HD_BACKGROUND))
        return false;
    float horizontal = fmodf(4096.0f - backdrop->yaw, 4096.0f) * background->width / 256.0f;
    sint32 column = (sint32)floorf(horizontal / 16.0f) + scroll;
    float x = -fmodf(horizontal, 16.0f);
    sint32 extra = (sint32)floorf((160.0f - hd_state.ui_half_width - margin_x - x) / 16.0f);
    column += extra;
    x += extra * 16.0f;
    column %= background->width;
    if (column < 0)
        column += background->width;
    sint32 row = (sint32)floorf((-margin_y - top) / 16.0f);
    if (row < 0)
        row = 0;
    float y = top + row * 16.0f;
    for (; row < background->height && y < 240.0f + margin_y; ++row, y += 16.0f)
    {
        sint32 current = column;
        for (float left = x; left < 160.0f + hd_state.ui_half_width + margin_x; left += 16.0f)
        {
            uint32 tile = map[current * background->height + row];
            if ((tile & 8191) >= background->tile_count)
                return false;
            const BackgroundTile *source = background->tiles + (tile & 8191);
            float u0 = source->uv & 255, v0 = source->uv >> 8 & 255;
            float u1 = u0 + 16.0f, v1 = v0 + 16.0f;
            if ((tile & 0xC000) != 0)
            {
                u1 = (source->uv >> 16 & 255) + 1.0f;
                v1 = (source->uv >> 24) + 1.0f;
                if ((tile & 0x4000) != 0)
                {
                    float swap = u0;
                    u0 = u1;
                    u1 = swap;
                }
                if ((tile & 0x8000) != 0)
                {
                    float swap = v0;
                    v0 = v1;
                    v1 = swap;
                }
            }
            VID_VERTEX2D vertices[6];
            static const uint8 order[6] = {0, 1, 2, 2, 1, 3};
            hd_ui_corners(vertices, left, y, 16, 16, 0x808080, HD_ANCHOR_CENTER);
            for (uint32 idx = 0; idx < 6; ++idx)
            {
                vertices[idx].uv[0] = (order[idx] & 1) != 0 ? u1 : u0;
                vertices[idx].uv[1] = (order[idx] & 2) != 0 ? v1 : v0;
                vertices[idx].tex[0] = source->attributes;
                vertices[idx].tex[1] = source->attributes >> 16;
                vertices[idx].tex[2] |= HD_TEXTURED | HD_RAW_TEXTURE;
                vertices[idx].tex[2] |= HD_FILTERED;
                vertices[idx].tex[3] = source->uv & 0xFFFF;
                vertices[idx].rect[0] = (tile & 0xC000) != 0 ? ((source->uv >> 16 & 255) - (source->uv & 255) + 1) : 16;
                vertices[idx].rect[1] = (tile & 0xC000) != 0 ? ((source->uv >> 24) - (source->uv >> 8 & 255) + 1) : 16;
            }
            if (!hd_append_ui(vertices, 6, VID_BLEND_NONE, HD_BACKGROUND))
                return false;
            if (++current == background->width)
                current = 0;
        }
    }
    return true;
}
