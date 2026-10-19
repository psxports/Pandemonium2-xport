#ifndef MATERIAL_RECORD_H
#define MATERIAL_RECORD_H

#include <stddef.h>

#include "psx.h"

struct TextureDescriptor;

struct EncodedMaterialRecord
{
    uint32 texture_offset;
    uint8 kind;
    uint8 flags;
    uint16 cycle_count;
    uint32 animation_offset;
    uint32 data_offset;
};

typedef char EncodedMaterialRecordSize[(sizeof(EncodedMaterialRecord) == 16) ? 1 : -1];
typedef char EncodedMaterialRecordKindOffset[(offsetof(EncodedMaterialRecord, kind) == 4) ? 1 : -1];
typedef char EncodedMaterialRecordCountOffset[(offsetof(EncodedMaterialRecord, cycle_count) == 6) ? 1 : -1];
typedef char EncodedMaterialRecordAnimationOffset[(offsetof(EncodedMaterialRecord, animation_offset) == 8) ? 1 : -1];
typedef char EncodedMaterialRecordDataOffset[(offsetof(EncodedMaterialRecord, data_offset) == 12) ? 1 : -1];

struct MaterialRecord
{
    TextureDescriptor *texture;
    uint8 kind;
    uint8 flags;
    uint16 cycle_count;
    uint8 *animation;
    uint8 *data;

    unsigned int CyclePalette(uint8 *animation, unsigned int frame) const;
    unsigned int UploadPalette() const;
};

extern "C"
{
    uint32 UpdateMaterialAnimation(uint32 *entry, uint32 frame);
    void ResetMaterialAnimationList(uint32 *entries);
    sint32 UpdateMaterialAnimationList(uint32 *entries);
    uint32 SetMaterialAnimationFlags(uint32 *entry, uint32 flags);
}

#endif
