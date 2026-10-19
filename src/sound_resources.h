#ifndef SOUND_RESOURCES_H
#define SOUND_RESOURCES_H

#include <stddef.h>

#include "psx.h"

// Keep device sample fields in the reviewed eight-byte format
struct SoundSample
{
    uint32 spu_address;
    uint16 pitch;
    uint16 metadata;
};

struct SoundTableHeader
{
    uint32 count;

    SoundTableHeader *Relocate(uint32 source_bytes, SoundSample *samples);
};

typedef char SoundSampleBinarySize[(sizeof(SoundSample) == 8) ? 1 : -1];
typedef char SoundSamplePitchOffset[(offsetof(SoundSample, pitch) == 4) ? 1 : -1];
typedef char SoundSampleMetadataOffset[(offsetof(SoundSample, metadata) == 6) ? 1 : -1];
typedef char SoundTableHeaderBinarySize[(sizeof(SoundTableHeader) == 4) ? 1 : -1];

// Own reviewed sample records independently of loaded resource buffers
struct SoundBankStorage
{
    enum
    {
        SampleCapacity = 26
    };

    SoundSample samples[SampleCapacity];
    SoundTableHeader header;
};

struct SoundResources
{
    uint32 transfer_address;
    const SoundBankStorage *common_table;
    const SoundBankStorage *scene_table;
    SoundBankStorage common_bank;
    SoundBankStorage scene_bank;
};

extern SoundResources sound_resources;

extern "C"
{
    uint32 SetSoundTransferAddress(uint32 asset, uint32 mode);
    uint32 UploadSoundSamples(const void *source, uint32 bytes);
}

#endif
