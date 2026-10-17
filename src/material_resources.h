#ifndef MATERIAL_RESOURCES_H
#define MATERIAL_RESOURCES_H

#include "psx.h"

union TexturePayload
{
    uint32 words[4];
    uint16 halfwords[8];
    uint8 bytes[16];
};

struct TextureDescriptor
{
    uint32 flags;
    TexturePayload payload;
};

typedef char TextureDescriptorBinaryLayout[(sizeof(TextureDescriptor) == 20) ? 1 : -1];

// Encoded offsets remain in the original mutable resource buffer
struct MaterialResourceHeader
{
    uint32 material_count;
    uint32 materials_offset;
    uint32 metadata[4];
    uint32 texture_count;
    uint32 textures_offset;

    uint32 Relocate(uint32 base, sint32 mode);
};

typedef char MaterialResourceHeaderBinaryLayout[(sizeof(MaterialResourceHeader) == 32) ? 1 : -1];

extern "C"
{
uint32 RelocateMaterialResource(uint32 object, sint32 mode);
uint32 BindModelMaterials(uint32 object, uint32 header);
uint32 DrawTexturedQuad(uint32 position, uint32 extent, uint32 group, uint32 blend);
}

#endif
