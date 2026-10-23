#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "material_record.h"
#include "game.h"
#include "hd.h"

void MaterialRecord::CyclePal(uint8 *animation, uint32 frame) const
{
    uint32 remaining;
    uint16 *pal;
    PaletteCycle *cycle;
    uint32 phase;
    sint32 delta;
    sint32 difference;
    sint32 distance;
    uint8 *dst;
    uint8 *src;
    uint32 len;
    uint32 bytes0;
    uint32 bytes1;
    uint16 saved;
    uint8 tmp[512];
    FUNCTION_MARKER(0x80027034, "SLUS_005.78");
    remaining = cycle_count;
    pal = (uint16 *)data;
    cycle = (PaletteCycle *)animation;
    do
    {
        phase = ((sint64)(sint32)frame * cycle->rate) >> 16;
        difference = cycle->last - cycle->first;
        distance = difference > 0 ? difference : -difference;
        phase %= (uint32)distance + 1;
        delta = cycle->phase - (sint32)phase;
        if (delta != 0)
        {
            cycle->phase = phase;
            if (delta == -1 || delta == distance)
            {
                dst = (uint8 *)(pal + cycle->last);
                saved = pal[cycle->last];
                if (difference > 0)
                {
                    src = (uint8 *)(pal + cycle->first);
                    dst = src + 2;
                }
                else
                    src = dst + 2;
                MoveBytes(dst, src, 2 * (uint32)distance);
                pal[cycle->first] = saved;
            }
            else
            {
                if (difference >= 0)
                {
                    delta = -delta;
                    dst = (uint8 *)(pal + cycle->first);
                }
                else
                    dst = (uint8 *)(pal + cycle->last);
                len = (uint32)distance + 1;
                CopyBytes(tmp, dst, 2 * len);
                if (delta <= 0)
                {
                    delta = -delta;
                    bytes1 = 2 * (uint32)delta;
                    bytes0 = 2 * (len - delta);
                    src = tmp + bytes1;
                }
                else
                {
                    bytes1 = 2 * (len - delta);
                    bytes0 = 2 * (uint32)delta;
                    src = tmp + bytes1;
                }
                CopyBytes(dst, src, bytes0);
                CopyBytes(dst + bytes0, tmp, bytes1);
            }
        }
        --remaining;
        if (remaining != 0)
        {
            ++cycle;
        }
    } while (remaining != 0);
    UploadPal();
}

uint32 MaterialRecord::UpdateMaterialAnimation(uint32 *entry, uint32 frame)
{
    uint32 encoded;
    uint32 idx;
    MaterialRecord *rec;
    TextureDescriptor *tex;
    uint32 kind;
    uint8 *animation;
    uint32 flags;
    uint32 phase;
    uint32 last_frame;
    uint32 period;
    uint32 remainder;
    uint32 sel;
    uint32 random_frame;
    uint32 prev_random;
    const uint8 *src;
    TextureFrame tex_frame;
    uint16 page;
    FUNCTION_MARKER(0x80027314, "SLUS_005.78");
    encoded = *entry;
    if (encoded == 0xFFFFFFFF)
        return 1;
    idx = (encoded >> 1) & 0x1FF;
    if (encoded & 1)
    {
        SceneSection *section = scene_stream.active_section;
        rec = section->materials.materials + idx;
    }
    else
        rec = material_registry.recs + idx;
    kind = rec->kind;
    animation = rec->animation;
    if (kind == 1)
    {
        phase = ((sint64)(sint32)frame * ((const TextureAnimation *)animation)->rate) >> 16;
        rec->tex->mapping.scroll_phase = phase;
        hd_scroll_update(rec->tex, frame, ((const TextureAnimation *)animation)->rate);
        return 1;
    }
    if (kind == 0)
    {
        rec->CyclePal(animation, frame);
        return 1;
    }
    if (kind != 2)
        return 1;
    flags = (*entry >> 26) & 15;
    phase = ((sint64)(sint32)frame * ((const TextureAnimation *)animation)->rate) >> 16;
    last_frame = ((const TextureAnimation *)animation)->last_frame;
    period = (flags & 2) ? 2 * last_frame : last_frame + 1;
    if ((flags & 8) != 0 && phase >= period)
        return 0;
    if (period == 0)
        trap(7);
    remainder = phase % period;
    sel = remainder;
    random_frame = 0;
    if (flags & 1)
        sel = ((const TextureAnimation *)animation)->last_frame - remainder;
    else if (flags & 2)
    {
        last_frame = ((const TextureAnimation *)animation)->last_frame;
        if ((sint32)(last_frame - remainder) < 0)
            sel = 2 * last_frame - remainder;
    }
    else if ((flags & 4) != 0 && remainder != ((*entry >> 10) & 255))
    {
        prev_random = (*entry >> 18) & 255;
        do
        {
            sel = random_generator.Next() % period;
        } while (sel == prev_random);
        random_frame = sel;
    }
    tex = rec->tex;
    src = rec->data + 8 * sel;
    page = tex->mapping.clut;
    tex_frame = *(const TextureFrame *)src;
    tex->mapping.uv_rect = tex_frame.uv_rect;
    tex->mapping.page_clut = tex_frame.page_clut;
    rec->tex->mapping.clut = page;
    *entry = (*entry & 1) | (idx << 1) | (remainder << 10) | (random_frame << 18) | (flags << 26);
    return 1;
}

void MaterialRecord::SetMaterialAnimationFlags(uint32 *entry, uint32 flags)
{
    FUNCTION_MARKER(0x80027648, "SLUS_005.78");
    uint32 val, idx, result;
    uint32 kind;
    val = *entry;
    result = val >> 1;
    if (val == 0xFFFFFFFF)
        return;
    idx = result & 0x1FF;
    if ((val & 1) != 0)
    {
        const SceneSection *section = scene_stream.active_section;
        kind = section->materials.materials[idx].kind;
    }
    else
        kind = material_registry.recs[idx].kind;
    result = idx * 2;
    if (kind == 2)
    {
        result |= (val & 1) | (flags << 26);
        *entry = result;
    }
}

void MaterialRecord::UploadPal() const
{
    const TextureDescriptor *tex_val;
    const uint8 *pal;
    uint16 clut;
    uint16 page;
    FUNCTION_MARKER(0x80064D88, "SLUS_005.78");
    tex_val = tex;
    clut = tex_val->mapping.clut;
    page = tex_val->mapping.tpage;
    pal = data;
    MaterialCatalog::UploadTexPal(clut, page, pal);
}
