#ifndef SOUND_TABLE_HEADER_H
#define SOUND_TABLE_HEADER_H
#include <stddef.h>

#include "psx.h"

struct EncodedSoundHeader;
struct EncodedSoundSample;
struct SoundSample;
struct SoundTableHeader;
struct SoundBankStorage;

struct EncodedSoundHeader
{
    uint32 count;
};

struct EncodedSoundSample
{
    uint32 spu_address;
    uint16 pitch;
    uint16 metadata;
};

typedef char EncodedSoundSampleSize[(sizeof(EncodedSoundSample) == 8) ? 1 : -1];

// Keep device sample fields in the reviewed eight-byte format
struct SoundSample
{
    uint32 spu_address;
    uint16 pitch;
    uint16 metadata;
};

struct SoundTableHeader
{
    sint32 count;

    static uint32 SetSoundTransferAddress(uint32 asset, uint32 mode);
    SoundTableHeader *Relocate(uint32 src_bytes, SoundSample *samples);
    static uint32 UploadSoundSamples(const void *src, uint32 bytes);
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

#endif
