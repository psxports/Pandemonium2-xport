#ifndef MATERIAL_RESOURCES_H
#define MATERIAL_RESOURCES_H

#include "psx.h"

struct TextureDescriptor;
struct MaterialRecord;
struct MeshFace;
struct MeshVertex;

enum ModelStorageKind
{
    ModelStorageRoot = 256,
    ModelStorageWordList,
    ModelStorageBytes
};

struct ModelStorageRecord
{
    uint32 source_offset;
    uint32 source_bytes;
    uint32 storage_offset;
    uint32 storage_bytes;
    uint32 kind;
};

struct ModelDecodeLayout
{
    const uint8 *input;
    uint32 input_bytes;
    uint32 source_clone_bytes;
    uint8 *storage;
    uint32 storage_bytes;
    uint8 *legacy_data;
    const ModelStorageRecord *records;
    uint32 record_count;
};

union TexturePayload
{
    uint32 words[4];
    uint16 halfwords[8];
    uint8 bytes[16];
};

struct EncodedTextureDescriptor
{
    uint32 flags;
    TexturePayload payload;
};

struct TextureDescriptor
{
    uint32 flags;
    TexturePayload payload;
    const TextureDescriptor *source_texture;

    uint32 WritePolygonUV(uint32 destination_offset, uint32 stride) const;
};

typedef char EncodedTextureDescriptorSize[(sizeof(EncodedTextureDescriptor) == 20) ? 1 : -1];

struct EncodedMaterialResourceHeader
{
    uint32 material_count;
    uint32 materials_offset;
    uint32 metadata[4];
    uint32 texture_count;
    uint32 textures_offset;
};

typedef char EncodedMaterialResourceHeaderSize[(sizeof(EncodedMaterialResourceHeader) == 32) ? 1 : -1];

// Own decoded catalogs and the byte payload shared by their native views
struct MaterialResourceHeader
{
    uint32 material_count;
    MaterialRecord *materials;
    uint32 metadata_words[2];
    uint8 *metadata_payloads[2];
    uint32 texture_count;
    TextureDescriptor *textures;
    uint8 *data;
    uint32 data_bytes;

    void Relocate(const uint8 *input, uint32 bytes, sint32 mode);
};

struct AnimatedObject;

extern "C"
{
    void RelocateMaterialResource(MaterialResourceHeader *resource, const uint8 *input, uint32 bytes, sint32 mode);
    void BindModelMaterials(AnimatedObject **object_slot, uint8 **model_owner, const MaterialResourceHeader *resource, uint32 bytes, MeshFace **polygon_owner, MeshVertex **vertex_owner);
    uint32 DrawTexturedQuad(uint32 position, uint32 extent, const TextureDescriptor *descriptor, uint32 blend);
}

#endif
