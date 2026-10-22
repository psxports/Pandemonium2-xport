#include <stdio.h>
#include <stdlib.h>

#include "sound_table_header.h"
#include "game.h"
#include "global.h"

uint32 SoundTableHeader::SetSoundTransferAddress(uint32 asset, uint32 mode)
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
    return SetSpuTransferAddressUnits(sound_resources.transfer_address);
}

SoundTableHeader *SoundTableHeader::Relocate(uint32 src_bytes, SoundSample *samples)
{
    uint32 remaining;
    SoundSample *sample;
    uint32 base;
    FUNCTION_MARKER(0x8006AA48, "SLUS_005.78");
    remaining = (uint32)count;
    if ((sint32)remaining <= 0)
        return 0;
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

uint32 SoundTableHeader::UploadSoundSamples(const void *src, uint32 bytes)
{
    FUNCTION_MARKER(0x8006AA88, "SLUS_005.78");
    return WriteSpuTransfer(src, bytes);
}
