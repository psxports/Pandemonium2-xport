#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "material_catalog.h"
#include "game.h"

MaterialCatalog *interface_resources = 0;

void MaterialCatalog::EmptyMaterialOperation(void)
{
    FUNCTION_MARKER(0x8002F180, "SLUS_005.78");
    /* Original function returns without side effects */
}

uint32 MaterialCatalog::DrawTexturedQuad(uint32 pos, uint32 extent, const TextureDescriptor *desc, uint32 blend)
{
    uint32 packet;
    POLY_FT4 *poly;
    uint32 tex;
    uint32 corner0;
    uint32 corner1;
    uint32 corner2;
    uint32 corner3;
    uint32 page;
    uint32 color;
    uint32 right;
    uint32 bottom;
    uint32 result;
    FUNCTION_MARKER(0x80064E74, "SLUS_005.78");
    packet = render_commands.packet_offset;
    result = packet < render_commands.packet_limit;
    if (result == 0)
        return result;
    tex = desc->mapping.uv_rect;
    corner2 = tex >> 16;
    corner1 = (tex & 0xFF00) | (corner2 & 255);
    corner3 = (corner2 & 0xFF00) | (tex & 255);
    if ((extent & 0xFFFF) == 0)
    {
        extent |= (corner2 & 255) - (tex & 255) + 1;
        ++corner1;
        ++corner2;
    }
    if ((extent >> 16) == 0)
    {
        extent |= ((((corner2 >> 8) & 255) - ((tex >> 8) & 255)) + 1) << 16;
        corner2 += 256;
        corner3 += 256;
    }
    page = desc->mapping.page_clut;
    corner0 = (tex & 0xFFFF) | (page & 0xFFFF0000);
    if (render_commands.overlay_head == 0)
        render_commands.overlay_head = packet;
    poly = (POLY_FT4 *)psx_addr(packet, sizeof(POLY_FT4));
    setPolyFT4(poly);
    poly->tag = ((packet + 40) & 0xFFFFFF) | 0x09000000;
    if (blend != 0)
    {
        corner1 |= (page | (((blend - 1) & 3) << 5)) << 16;
        color = 0x2F000000;
    }
    else
    {
        corner1 |= page << 16;
        color = 0x2D000000;
    }
    poly->rgbc.rgbc = color;
    poly->p[0].uv_attr = corner0;
    right = (pos & 0xFFFF) + (extent & 0xFFFF);
    bottom = (pos & 0xFFFF0000) + (extent & 0xFFFF0000);
    poly->p[1].xy.xy = right | (pos & 0xFFFF0000);
    result = packet + 40;
    poly->p[0].xy.xy = pos;
    poly->p[1].uv_attr = corner1;
    poly->p[2].xy.xy = (pos & 0xFFFF) | bottom;
    poly->p[2].uv_attr = corner3;
    poly->p[3].xy.xy = right | bottom;
    poly->p[3].uv_attr = corner2;
    render_commands.overlay_tail = packet;
    render_commands.packet_offset = result;
    return result;
}

void MaterialCatalog::UploadTexPal(uint16 clut, uint32 page, const uint8 *pal)
{
    PSX_RECT rect;
    FUNCTION_MARKER(0x80066B78, "SLUS_005.78");
    rect.x = (clut & 0x3F) << 4;
    rect.y = clut >> 6;
    rect.w = ((page >> 7) & 3) == 1 ? 256 : 16;
    rect.h = 1;
    LoadImagePSX(&rect, (uint32 *)pal);
}

void MaterialCatalog::Relocate(const uint8 *input, uint32 bytes, sint32 mode)
{
    FUNCTION_MARKER(0x80068790, "SLUS_005.78");
    const EncodedMaterialResourceHeader *header = (const EncodedMaterialResourceHeader *)input;
    uint32 new_material_count = header->material_count;
    uint32 material_offset = header->materials_offset;
    uint32 new_tex_count = header->tex_count;
    uint32 tex_offset = header->texs_offset;
    uint32 payload_offsets[2] = {header->metadata[1], header->metadata[3]};
    uint8 *new_data = (uint8 *)malloc(bytes);
    TextureDescriptor *new_texs = new_tex_count != 0 ? (TextureDescriptor *)calloc(new_tex_count, sizeof(TextureDescriptor)) : 0;
    MaterialRecord *new_materials = new_material_count != 0 ? (MaterialRecord *)calloc(new_material_count, sizeof(MaterialRecord)) : 0;
    memcpy(new_data, input, bytes);
    free(texs);
    free(materials);
    free(data);
    texs = new_texs;
    materials = new_materials;
    data = new_data;
    data_bytes = bytes;
    tex_count = new_tex_count;
    material_count = new_material_count;
    for (uint32 tex_idx = 0; tex_idx < tex_count; ++tex_idx)
    {
        const uint8 *encoded = data + tex_offset + tex_idx * sizeof(EncodedTextureDescriptor);
        const EncodedTextureDescriptor *encoded_tex = (const EncodedTextureDescriptor *)encoded;
        texs[tex_idx].flags = encoded_tex->flags;
        texs[tex_idx].mapping = encoded_tex->mapping;
    }
    uint32 remaining, flags, count;
    TextureDescriptor *tex;
    const TextureDescriptor *referenced_tex;
    uint32 uv_rect, page_clut, scroll_phase;
    uint16 preserved, half0, half1;
    uint8 x, y;
    if (tex_count != 0)
    {
        remaining = tex_count;
        tex = texs;
        do
        {
            flags = tex->flags;
            if ((flags & 0x1400) == 0x1000)
            {
                if (tex->mapping.tex_idx >= 2001)
                    tex->mapping.tex_idx = 0;
                tex->src_tex = texs + tex->mapping.tex_idx;
                referenced_tex = tex->src_tex;
                x = referenced_tex->mapping.left;
                y = referenced_tex->mapping.top;
                TextureMapping atlas_mapping = tex->mapping;
                tex->mapping.uv0.u = atlas_mapping.uv0.u + x;
                tex->mapping.uv0.v = atlas_mapping.uv0.v + y;
                tex->mapping.uv1.u = atlas_mapping.uv1.u + x;
                tex->mapping.uv1.v = atlas_mapping.uv1.v + y;
                tex->mapping.uv3.u = atlas_mapping.uv3.u + x;
                tex->mapping.uv3.v = atlas_mapping.uv3.v + y;
                tex->mapping.uv2.u = atlas_mapping.uv2.u + x;
                tex->mapping.uv2.v = atlas_mapping.uv2.v + y;
            }
            else if (flags & 0x400)
            {
                if (flags & 0x800)
                {
                    preserved = tex->mapping.clut;
                    referenced_tex = texs + tex->mapping.tex_idx;
                    uv_rect = referenced_tex->mapping.uv_rect;
                    page_clut = referenced_tex->mapping.page_clut;
                    scroll_phase = referenced_tex->mapping.scroll_phase;
                    tex->mapping.uv_rect = uv_rect;
                    tex->mapping.page_clut = page_clut;
                    tex->mapping.scroll_phase = scroll_phase;
                    if (tex->flags & 0x80000)
                        tex->mapping.clut = preserved;
                }
                else if (mode == 5)
                {
                    half0 = tex->mapping.tpage;
                    half1 = tex->mapping.clut;
                    tex->mapping.tpage = half0 | 0x10;
                    tex->mapping.clut = half1 | 0x4000;
                }
            }
            ++tex;
            --remaining;
        } while (remaining != 0);
    }
    for (uint32 material_idx = 0; material_idx < material_count; ++material_idx)
    {
        const EncodedMaterialRecord *encoded = (const EncodedMaterialRecord *)(data + material_offset) + material_idx;
        MaterialRecord *material_rec = materials + material_idx;
        uint32 tex_val = encoded->tex_offset;
        uint32 animation_val = encoded->animation_offset;
        uint32 data_val = encoded->data_offset;
        material_rec->kind = encoded->kind;
        material_rec->flags = encoded->flags;
        material_rec->cycle_count = encoded->cycle_count;
        if (tex_val != 0)
        {
            material_rec->tex = texs + (tex_val - tex_offset) / sizeof(EncodedTextureDescriptor);
        }
        if (animation_val != 0)
        {
            material_rec->animation = data + animation_val;
        }
        if (material_rec->kind == 0 || material_rec->kind == 2)
        {
            material_rec->data = data + data_val;
        }
        if (material_rec->kind == 2 && (material_rec->flags & 0x80) == 0 && mode == 5)
        {
            count = ((const TextureAnimation *)material_rec->animation)->last_frame;
            if (count != 0)
            {
                ++count;
                TextureFrame *coords = (TextureFrame *)material_rec->data;
                do
                {
                    --count;
                    coords->page_clut |= 0x40000010;
                    ++coords;
                } while (count != 0);
            }
        }
    }
}

void MaterialCatalog::RelocateMaterialResource(MaterialCatalog *resource, const uint8 *input, uint32 bytes, sint32 mode)
{
    FUNCTION_MARKER(0x80069F40, "SLUS_005.78");
    resource->Relocate(input, bytes, mode);
}
