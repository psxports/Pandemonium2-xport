#ifndef MATERIAL_RECORD_H
#define MATERIAL_RECORD_H
#include <stddef.h>

#include "psx.h"

struct TextureDescriptor;

struct EncodedMaterialRecord;
struct TextureFrame;
struct PaletteCycle;
struct TextureAnimation;
struct MaterialRecord;

struct EncodedMaterialRecord
{
    uint32 tex_offset;
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

struct TextureFrame
{
    uint32 uv_rect;
    uint32 page_clut;
};

struct PaletteCycle
{
    sint32 rate;
    uint8 first;
    uint8 last;
    uint8 phase;
    uint8 reserved;
};

struct TextureAnimation
{
    sint32 rate;
    uint8 flags;
    uint8 last_frame;
    uint16 reserved;
};

struct MaterialRecord
{
    TextureDescriptor *tex;
    uint8 kind;
    uint8 flags;
    uint16 cycle_count;
    uint8 *animation;
    uint8 *data;

    void CyclePal(uint8 *animation, uint32 frame) const;
    static uint32 UpdateMaterialAnimation(uint32 *entry, uint32 frame);
    static void SetMaterialAnimationFlags(uint32 *entry, uint32 flags);
    void UploadPal() const;
};

#endif
