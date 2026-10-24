#include "hd_internal.h"

static bool linear_filter = true;

void hd_set_filter(bool linear)
{
    linear_filter = linear;
}

bool hd_filter(void)
{
    return linear_filter;
}

static sint32 hd_function_key(uint32 key)
{
    if (key != 4)
        return 0;
    linear_filter = !linear_filter;
    return 1;
}

void hd_filter_keys(void)
{
    xport_set_function_key_handler(hd_function_key);
}

void hd_filter_material(VID_MATERIAL *material)
{
    VID_MATERIAL_DATA *data = vid_mat_data(material);
    float enabled = linear_filter ? 1.0f : 0.0f;
    if (data->params[7] != enabled)
    {
        data->params[7] = enabled;
        vid_mat_commit(material);
    }
}

void hd_upload_vram(void)
{
    uint32 version = gpu_vram_version();
    if (hd_state.vram_valid && version == hd_state.vram_version)
        return;
    memcpy(vid_tex_data(hd_state.vram), VRAM, sizeof(VRAM));
    vid_tex_commit(hd_state.vram);
    hd_state.vram_version = version;
    hd_state.vram_valid = true;
}

bool hd_og_material_equal(const HdOgMaterial *copy, const TextureDescriptor *texture)
{
    if (texture == NULL)
        return !copy->present;
    if (!copy->present || copy->texture.flags != texture->flags ||
        memcmp(&copy->texture.mapping, &texture->mapping, sizeof(TextureMapping)) != 0)
        return false;
    if ((texture->flags & 0x1400) != 0x1000)
        return true;
    if (texture->src_tex == NULL)
        return copy->texture.src_tex == NULL;
    return copy->texture.src_tex != NULL && copy->source.flags == texture->src_tex->flags &&
        memcmp(&copy->source.mapping, &texture->src_tex->mapping, sizeof(TextureMapping)) == 0;
}

void hd_og_material_copy(HdOgMaterial *copy, const TextureDescriptor *texture)
{
    memset(copy, 0, sizeof(*copy));
    if (texture == NULL)
        return;
    copy->present = true;
    copy->texture = *texture;
    copy->texture.src_tex = NULL;
    if ((texture->flags & 0x1400) == 0x1000 && texture->src_tex != NULL)
    {
        copy->source = *texture->src_tex;
        copy->source.src_tex = NULL;
        copy->texture.src_tex = &copy->source;
    }
}

void hd_scroll_reset(void)
{
    ++hd_state.scroll_epoch;
}

void hd_scroll_update(const TextureDescriptor *texture, uint32 frame, sint32 rate)
{
    HdScrollClock *clock = hd_state.scroll_clocks;
    while (clock != NULL && clock->texture != texture)
        clock = clock->next;
    if (clock == NULL)
    {
        clock = (HdScrollClock *)malloc(sizeof(HdScrollClock));
        if (clock == NULL)
            return;
        clock->texture = texture;
        clock->next = hd_state.scroll_clocks;
        hd_state.scroll_clocks = clock;
    }
    clock->frame = frame;
    clock->rate = rate;
    clock->epoch = hd_state.scroll_epoch;
}

HdScroll hd_scroll_source(const TextureDescriptor *texture)
{
    HdScroll scroll;
    memset(&scroll, 0, sizeof(scroll));
    if ((texture->flags & 4) == 0 || (texture->flags & 0x1E000) == 0)
        return scroll;
    bool explicit_uv = (texture->flags & 0x1400) == 0x1000;
    const TextureDescriptor *source = explicit_uv ? texture->src_tex : texture;
    if (source == NULL)
        return scroll;
    const HdScrollClock *clock = hd_state.scroll_clocks;
    while (clock != NULL && clock->texture != source)
        clock = clock->next;
    if (clock == NULL)
        return scroll;
    scroll.texture = texture;
    scroll.source = source;
    scroll.mapping = texture->mapping;
    scroll.source_mapping = source->mapping;
    if (!explicit_uv)
        scroll.mapping.scroll_phase = 0;
    scroll.source_mapping.scroll_phase = 0;
    scroll.flags = texture->flags;
    scroll.phase = source->mapping.scroll_phase;
    scroll.frame = clock->frame;
    scroll.rate = clock->rate;
    scroll.epoch = clock->epoch;
    sint64 phase = (sint64)(sint32)clock->frame * clock->rate;
    // Reject a stale clock when a descriptor was changed without its animation owner
    if ((uint32)(phase >> 16) != scroll.phase)
    {
        memset(&scroll, 0, sizeof(scroll));
        return scroll;
    }
    scroll.fraction = (uint16)phase;
    if ((scroll.flags & 0x6000) != 0)
        scroll.period_u = (source->mapping.right - source->mapping.left + 1) >> 1;
    if ((scroll.flags & 0x18000) != 0)
        scroll.period_v = (source->mapping.bottom - source->mapping.top + 1) >> 1;
    return scroll;
}

bool hd_scroll_pair(const HdScroll *previous, const HdScroll *current)
{
    if (current->texture == NULL || previous->texture != current->texture || previous->source != current->source ||
        previous->flags != current->flags || previous->rate != current->rate || previous->epoch != current->epoch ||
        current->frame < previous->frame || current->frame > 0x7FFFFFFFu ||
        previous->period_u != current->period_u || previous->period_v != current->period_v ||
        memcmp(&previous->mapping, &current->mapping, sizeof(current->mapping)) != 0 ||
        memcmp(&previous->source_mapping, &current->source_mapping, sizeof(current->source_mapping)) != 0)
        return false;
    sint64 old_phase = ((sint64)(sint32)previous->frame * previous->rate) >> 16;
    sint64 phase = ((sint64)(sint32)current->frame * current->rate) >> 16;
    sint64 delta = phase - old_phase;
    return delta >= -(sint64)2147483648u && delta <= 2147483647;
}

bool hd_texture(const TextureDescriptor *texture, HdTexture *result)
{
    uint32 flags = texture->flags;
    const TextureMapping *mapping = &texture->mapping;
    uint32 uv0, uv1, uv2, uv3;
    if ((flags & 0x1400) == 0x1000)
    {
        if (texture->src_tex == NULL)
            return false;
        uv0 = mapping->uv01;
        uv1 = uv0 >> 16;
        uv2 = mapping->uv32 >> 16;
        uv3 = mapping->uv32 & 0xFFFF;
        mapping = &texture->src_tex->mapping;
        uint32 width = (sint32)(mapping->right - mapping->left + 1) >> 1;
        uint32 height = (sint32)(mapping->bottom - mapping->top + 1) >> 1;
        uint32 u = 0, v = 0;
        if ((flags & 0x6000) != 0)
        {
            if (width == 0)
                return false;
            uint32 remainder = mapping->scroll_phase % width;
            u = (flags & 0x2000) != 0 ? remainder : width - remainder;
        }
        if ((flags & 0x18000) != 0)
        {
            if (height == 0)
                return false;
            uint32 remainder = mapping->scroll_phase % height;
            v = (flags & 0x8000) != 0 ? remainder : height - remainder;
        }
        uint32 offset = u | (v << 8);
        uv0 += offset;
        uv1 += offset;
        uv2 += offset;
        uv3 += offset;
    }
    else
    {
        uv0 = mapping->uv_rect;
        if ((flags & 0x1E000) != 0)
        {
            uint32 left = mapping->left, top = mapping->top;
            uint32 width = mapping->right - left + 1;
            uint32 height = mapping->bottom - top + 1;
            if ((flags & 0x6000) != 0)
            {
                width >>= 1;
                if (width == 0)
                    return false;
                uint32 remainder = mapping->scroll_phase % width;
                left += (flags & 0x2000) != 0 ? remainder : width - remainder;
            }
            if ((flags & 0x18000) != 0)
            {
                height >>= 1;
                if (height == 0)
                    return false;
                uint32 remainder = mapping->scroll_phase % height;
                top += (flags & 0x8000) != 0 ? remainder : height - remainder;
            }
            uv0 = left | (top << 8) | ((left + width - 1) << 16) | ((top + height - 1) << 24);
        }
        uv3 = uv0 >> 16;
        if ((flags & 8) != 0)
        {
            uint32 right = uv3 & 255;
            uv3 = (uv3 & 0xFFFFFF00) | (uv0 & 255);
            uv0 = (uv0 & 0xFFFFFF00) | right;
        }
        if ((flags & 16) != 0)
        {
            uint32 bottom = uv3 & 0xFF00;
            uv3 = (uv3 & 0xFFFF00FF) | (uv0 & 0xFF00);
            uv0 = (uv0 & 0xFFFF00FF) | bottom;
        }
        uv1 = (uv0 & 0xFF00) | (uv3 & 255);
        uv2 = (uv3 & 0xFF00) | (uv0 & 255);
    }
    result->uv[0].u = uv0;
    result->uv[0].v = uv0 >> 8;
    result->uv[1].u = uv1;
    result->uv[1].v = uv1 >> 8;
    result->uv[2].u = uv2;
    result->uv[2].v = uv2 >> 8;
    result->uv[3].u = uv3;
    result->uv[3].v = uv3 >> 8;
    result->left = mapping->left;
    result->top = mapping->top;
    result->width = ((mapping->right - mapping->left) & 255) + 1;
    result->height = ((mapping->bottom - mapping->top) & 255) + 1;
    result->flags = HD_FILTERED;
    if ((flags & 0x6000) != 0)
    {
        result->width >>= 1;
        result->flags |= HD_REPEAT_U;
    }
    if ((flags & 0x18000) != 0)
    {
        result->height >>= 1;
        result->flags |= HD_REPEAT_V;
    }
    if (result->width == 0 || result->height == 0)
        return false;
    result->page = mapping->tpage | (flags & 0x60) | (uv1 >> 16);
    result->clut = mapping->clut;
    return true;
}
