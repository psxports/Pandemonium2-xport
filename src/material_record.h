#ifndef MATERIAL_RECORD_H
#define MATERIAL_RECORD_H

#include <stddef.h>

#include "psx.h"

struct MaterialRecord
{
    unsigned int texture_offset;
    unsigned char kind;
    unsigned char flags;
    unsigned short cycle_count;
    unsigned int animation_offset;
    unsigned int data_offset;

    unsigned int CyclePalette(unsigned int animation, unsigned int frame) const;
    unsigned int UploadPalette() const;
};

typedef char MaterialRecordSize[(sizeof(MaterialRecord) == 16) ? 1 : -1];
typedef char MaterialRecordKindOffset[(offsetof(MaterialRecord, kind) == 4) ? 1 : -1];
typedef char MaterialRecordCountOffset[(offsetof(MaterialRecord, cycle_count) == 6) ? 1 : -1];
typedef char MaterialRecordAnimationOffset[(offsetof(MaterialRecord, animation_offset) == 8) ? 1 : -1];
typedef char MaterialRecordDataOffset[(offsetof(MaterialRecord, data_offset) == 12) ? 1 : -1];

extern "C"
{
uint32 UpdateMaterialAnimation(uint32 *entry, uint32 frame);
void ResetMaterialAnimationList(uint32 *entries);
sint32 UpdateMaterialAnimationList(uint32 *entries);
uint32 SetMaterialAnimationFlags(uint32 *entry, uint32 flags);
}

#endif
