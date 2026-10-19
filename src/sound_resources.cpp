#include <stdio.h>
#include <stdlib.h>

#include "game.h"

SoundResources sound_resources = {0};

uint32 SetSoundTransferAddress(uint32 asset, uint32 mode)
{
    uint32 size;
    uint32 rounded;
    uint32 address;
    FUNCTION_MARKER(0x8006A530, "SLUS_005.78");
    if (mode == 0)
        address = 192576;
    else
    {
        size = resource_archive.Size(mode == 1 ? asset : 253);
        rounded = size + 15;
        if ((sint32)rounded < 0)
            rounded = size + 30;
        rounded = (uint32)((sint32)rounded >> 4) << 4;
        if (mode == 1)
            address = 512288 - rounded;
        else
            address = rounded + 192576;
    }
    sound_resources.transfer_address = address;
    return p2_sdk_80070eec(sound_resources.transfer_address);
}

SoundTableHeader *SoundTableHeader::Relocate(uint32 source_bytes, SoundSample *samples)
{
    uint32 remaining;
    SoundSample *sample;
    uint32 base;
    FUNCTION_MARKER(0x8006AA48, "SLUS_005.78");
    if (source_bytes < sizeof(SoundTableHeader))
    {
        fprintf(stderr, "P2: Truncated sound table header\n");
        abort();
    }
    remaining = count;
    if ((sint32)remaining <= 0)
        return 0;
    if (remaining > (source_bytes - sizeof(SoundTableHeader)) / sizeof(SoundSample))
    {
        fprintf(stderr, "P2: Sound table entries exceed resource extent\n");
        abort();
    }
    sample = samples;
    base = sound_resources.transfer_address;
    do
    {
        --remaining;
        sample->spu_address += base;
        ++sample;
    } while (remaining != 0);
    return this;
}

uint32 UploadSoundSamples(const void *source, uint32 bytes)
{
    FUNCTION_MARKER(0x8006AA88, "SLUS_005.78");
    return p2_func_80070e8c(source, bytes);
}

void InitGameSound(void)
{
    SpuReverbAttr attributes = {0};
    FUNCTION_MARKER(0x8006AED0, "SLUS_005.78");
    _96_remove();
    p2_func_8006f158();
    // Initialize the mask omitted by the original startup code
    attributes.mask = 7;
    attributes.mode = 256;
    p2_sdk_8006ff38(&attributes);
    SpuSetTransferMode(0);
    p2_func_80071c5c();
    p2_sdk_800759d4(sequence_track_storage, 2, 1);
    p2_sdk_80075bb4(1);
    p2_sdk_80079adc(16);
    p2_func_80074750();
}
