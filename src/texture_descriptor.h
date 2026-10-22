#ifndef TEXTURE_DESCRIPTOR_H
#define TEXTURE_DESCRIPTOR_H
#include <stddef.h>

#include "psx.h"

struct TextureMapping;
struct EncodedTextureDescriptor;
struct TextureDescriptor;

struct TextureMapping
{
    union
    {
        uint32 uv_rect;
        uint32 tex_idx;

        struct
        {
            uint8 left, top, right, bottom;
        };
    };

    union
    {
        uint32 page_clut;
        uint32 uv01;

        struct
        {
            uint16 tpage, clut;
        };

        struct
        {
            UV uv0, uv1;
        };
    };

    union
    {
        uint32 scroll_phase;
        uint32 uv32;

        struct
        {
            UV uv3, uv2;
        };
    };

    RGBC color;
};

struct EncodedTextureDescriptor
{
    uint32 flags;
    TextureMapping mapping;
};

struct TextureDescriptor
{
    uint32 flags;
    TextureMapping mapping;
    const TextureDescriptor *src_tex;

    uint32 WritePolygonUV(uint32 dst_offset, uint32 stride) const;
};

typedef char EncodedTextureDescriptorSize[(sizeof(EncodedTextureDescriptor) == 20) ? 1 : -1];

#endif
