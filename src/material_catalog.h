#ifndef MATERIAL_CATALOG_H
#define MATERIAL_CATALOG_H
#include <stddef.h>

#include "psx.h"

struct TextureDescriptor;

struct MaterialRecord;
struct TextureDescriptor;

struct EncodedMaterialResourceHeader;
struct MaterialCatalog;

struct EncodedMaterialResourceHeader
{
    uint32 material_count;
    uint32 materials_offset;
    uint32 metadata[4];
    uint32 tex_count;
    uint32 texs_offset;
};

typedef char EncodedMaterialResourceHeaderSize[(sizeof(EncodedMaterialResourceHeader) == 32) ? 1 : -1];

// Own decoded catalogs and the byte payload shared by their native views
struct MaterialCatalog
{
    uint32 material_count;
    MaterialRecord *materials;
    uint32 tex_count;
    TextureDescriptor *texs;
    uint8 *data;
    uint32 data_bytes;

    static void EmptyMaterialOperation(void);
    static uint32 DrawTexturedQuad(uint32 pos, uint32 extent, const TextureDescriptor *desc, uint32 blend);
    static void UploadTexPal(uint16 clut, uint32 page, const uint8 *pal);
    void Relocate(const uint8 *input, uint32 bytes, sint32 mode);
    static void RelocateMaterialResource(MaterialCatalog *resource, const uint8 *input, uint32 bytes, sint32 mode);
};

extern MaterialCatalog *interface_resources;

#endif
