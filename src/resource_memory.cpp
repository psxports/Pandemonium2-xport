#include <stdlib.h>

#include "resource_memory.h"
#include "game.h"

ResourceMemory resource_memory = {0, 0};

uint8 *AllocateGameMemory(uint32 bytes, uint32 storage_bytes)
{
    FUNCTION_MARKER(0x80064D28, "SLUS_005.78");
    resource_memory.allocated_bytes = resource_memory.allocated_bytes + bytes;
    uint32 aligned_bytes = (bytes + 3u) & ~3u;
    uint32 aligned_storage_bytes = storage_bytes != 0 ? (storage_bytes + 3u) & ~3u : aligned_bytes;
    size_t allocation_bytes = (size_t)aligned_storage_bytes + sizeof(uint32);
    if (allocation_bytes < aligned_storage_bytes)
        return 0;
    uint32 *allocation = (uint32 *)malloc(allocation_bytes);
    if (allocation == 0)
        return 0;
    *allocation = aligned_bytes;
    return (uint8 *)(allocation + 1);
}

void FreeGameMemory(void *allocation)
{
    uint32 total;
    uint32 bytes;
    FUNCTION_MARKER(0x80064D54, "SLUS_005.78");
    total = resource_memory.allocated_bytes;
    uint32 *header = (uint32 *)allocation - 1;
    bytes = *header;
    resource_memory.allocated_bytes = total - bytes;
    free(header);
}

static uint32 *AllocateWordList(uint32 mode, uint32 count)
{
    uint32 entries;
    uint32 bytes;
    uint32 *allocation;
    FUNCTION_MARKER(0x8002A3E0, "SLUS_005.78");
    entries = (sint16)(count + 1);
    bytes = entries << 2;
    allocation = (uint32 *)AllocateGameMemory(bytes);
    FillBytes((uint8 *)allocation, 255, bytes);
    *allocation = entries - 1;
    return allocation + 1;
}

static void FreeWordList(uint32 mode, uint32 *object)
{
    FUNCTION_MARKER(0x8002A448, "SLUS_005.78");
    if (object != 0)
        FreeGameMemory(object - 1);
}

void FillBytes(uint8 *dst, uint8 val, uint32 count)
{
    FUNCTION_MARKER(0x80066358, "SLUS_005.78");
    while (count != 0)
    {
        *dst++ = val;
        --count;
    }
}

void CopyBytes(uint8 *dst, const uint8 *src, uint32 count)
{
    FUNCTION_MARKER(0x8006637C, "SLUS_005.78");
    while (count != 0)
    {
        *dst++ = *src++;
        --count;
    }
}

static sint32 CompareByteRanges(const uint8 *data0, const uint8 *data1, sint32 count)
{
    FUNCTION_MARKER(0x80083310, "SLUS_005.78");
    uint8 byte0, byte1;
    for (;;)
    {
        byte0 = *data0;
        byte1 = *data1;
        ++data0;
        if (byte0 != byte1)
        {
            byte0 = *(data0 - 1);
            byte1 = *data1;
            return byte0 - byte1;
        }
        count = (uint32)count - 1;
        ++data1;
        if (count <= 0)
            return 0;
    }
}

void MoveBytes(uint8 *dst, const uint8 *src, uint32 count)
{
    uint32 result;
    FUNCTION_MARKER(0x800663AC, "SLUS_005.78");
    if (count != 0)
    {
        if ((uintptr_t)src < (uintptr_t)dst)
        {
            src += count - 1;
            dst += count - 1;
            do
            {
                result = *src;
                --count;
                *dst = result;
                if (count != 0)
                {
                    --src;
                    --dst;
                }
            } while (count != 0);
        }
        else
        {
            do
            {
                result = *src;
                ++src;
                --count;
                *dst = result;
                ++dst;
            } while (count != 0);
        }
    }
}
