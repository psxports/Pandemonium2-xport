#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "texture_descriptor.h"
#include "game.h"

uint32 TextureDescriptor::WritePolygonUV(uint32 dst_offset, uint32 stride) const
{
    FUNCTION_MARKER(0x800612C0, "SLUS_005.78");
    uint32 cur_flags = flags;
    const TextureMapping *tex_fields;
    uint32 uv0, uv1, uv2, uv3;
    uint32 packed;
    if ((cur_flags & 0x1400) == 0x1000)
    {
        const TextureDescriptor *tex;
        uv0 = mapping.uv01;
        tex = src_tex;
        packed = mapping.uv32;
        tex_fields = &tex->mapping;
        uv1 = uv0 >> 16;
        uv2 = packed >> 16;
        uv3 = packed & 0xFFFF;
        if ((cur_flags & 0x1E000) != 0)
        {
            uint32 right = tex->mapping.right;
            uint32 left = tex->mapping.left;
            uint32 bottom = tex->mapping.bottom;
            uint32 top = tex->mapping.top;
            uint32 width = (sint32)(right - left + 1) >> 1;
            uint32 height = (sint32)(bottom - top + 1) >> 1;
            uint32 counter = tex->mapping.scroll_phase;
            uint32 u = 0, v = 0;
            if ((cur_flags & 0x6000) != 0)
            {
                uint32 remainder;
                if (width == 0)
                    xport_mips_break(7);
                remainder = width == 0 ? counter : counter % width;
                u = (cur_flags & 0x2000) != 0 ? remainder : width - remainder;
            }
            if ((cur_flags & 0x18000) != 0)
            {
                uint32 remainder;
                if (height == 0)
                    xport_mips_break(7);
                remainder = height == 0 ? counter : counter % height;
                v = (cur_flags & 0x8000) != 0 ? remainder : height - remainder;
            }
            packed = u | (v << 8);
            uv0 += packed;
            uv1 += packed;
            uv3 += packed;
            uv2 += packed;
        }
    }
    else
    {
        tex_fields = &mapping;
        if ((cur_flags & 0x1E000) != 0)
        {
            uint32 left = mapping.left;
            uint32 right = mapping.right;
            uint32 top = mapping.top;
            uint32 bottom = mapping.bottom;
            uint32 counter = mapping.scroll_phase;
            uint32 width = right - left + 1;
            uint32 height = bottom - top + 1;
            if ((cur_flags & 0x6000) != 0)
            {
                uint32 remainder;
                width >>= 1;
                if (width == 0)
                    xport_mips_break(7);
                remainder = width == 0 ? counter : counter % width;
                left += (cur_flags & 0x2000) != 0 ? remainder : width - remainder;
            }
            right = left + width - 1;
            if ((cur_flags & 0x18000) != 0)
            {
                uint32 remainder;
                height >>= 1;
                if (height == 0)
                    xport_mips_break(7);
                remainder = height == 0 ? counter : counter % height;
                top += (cur_flags & 0x8000) != 0 ? remainder : height - remainder;
            }
            bottom = top + height - 1;
            uv0 = left | (top << 8) | (right << 16) | (bottom << 24);
        }
        else
            uv0 = mapping.uv_rect;
        uv3 = uv0 >> 16;
        if ((cur_flags & 8) != 0)
        {
            uint32 right = uv3 & 0xFF;
            uv3 = (uv3 & 0xFFFFFF00) | (uv0 & 0xFF);
            uv0 = (uv0 & 0xFFFFFF00) | right;
        }
        if ((cur_flags & 0x10) != 0)
        {
            uint32 bottom = uv3 & 0xFF00;
            uv3 = (uv3 & 0xFFFF00FF) | (uv0 & 0xFF00);
            uv0 = (uv0 & 0xFFFF00FF) | bottom;
        }
        uv1 = (uv0 & 0xFF00) | (uv3 & 0xFF);
        uv2 = (uv3 & 0xFF00) | (uv0 & 0xFF);
    }
    packed = tex_fields->page_clut;
    uv1 |= ((cur_flags & 0x60) << 16) | (packed << 16);
    uv0 = (uv0 & 0xFFFF) | (packed & 0xFFFF0000);
    if (stride == 2)
    {
        POINT_T *p = (POINT_T *)psx_addr(dst_offset - 4, 4 * sizeof(*p));
        p[0].uv_attr = uv0;
        p[1].uv_attr = uv1;
        p[2].uv_attr = uv2;
        dst_offset += stride * 12;
        p[3].uv_attr = uv3;
    }
    else if (stride == 3)
    {
        POINT_CT *p = (POINT_CT *)psx_addr(dst_offset - 8, 4 * sizeof(*p));
        p[0].uv_attr = uv0;
        p[1].uv_attr = uv1;
        p[2].uv_attr = uv2;
        dst_offset += stride * 12;
        p[3].uv_attr = uv3;
    }
    else
    {
        uint32 *uv_words = (uint32 *)psx_addr(dst_offset, ((size_t)stride * 3 + 1) * sizeof(uint32));
        uv_words[0] = uv0;
        uv_words[stride] = uv1;
        uv_words[stride * 2] = uv2;
        dst_offset += stride * 12;
        uv_words[stride * 3] = uv3;
    }
    return dst_offset;
}
