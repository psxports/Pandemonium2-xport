#include <stdlib.h>

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

uint32 *AllocateWordList(uint32 mode, uint32 count)
{
    uint32 entries;
    uint32 bytes;
    uint32 *allocation;
    FUNCTION_MARKER(0x8002A3E0, "SLUS_005.78");
    entries = (uint32)(sint16)(count + 1);
    bytes = entries << 2;
    allocation = (uint32 *)AllocateGameMemory(bytes);
    FillBytes((uint8 *)allocation, 255, bytes);
    *allocation = entries - 1;
    return allocation + 1;
}

void FreeWordList(uint32 mode, uint32 *object)
{
    FUNCTION_MARKER(0x8002A448, "SLUS_005.78");
    if (object != 0)
        FreeGameMemory(object - 1);
}

uint32 FillBytes(uint8 *destination, uint8 value, uint32 count)
{
    FUNCTION_MARKER(0x80066358, "SLUS_005.78");
    while (count != 0)
    {
        *destination++ = value;
        --count;
    }
    return 0;
}

uint32 CopyBytes(uint8 *destination, const uint8 *source, uint32 count)
{
    uint32 result = count;
    FUNCTION_MARKER(0x8006637C, "SLUS_005.78");
    while (count != 0)
    {
        result = *source++;
        *destination++ = (uint8)result;
        --count;
    }
    return result;
}

sint32 CompareByteRanges(const uint8 *first_data, const uint8 *second_data, sint32 count)
{
    FUNCTION_MARKER(0x80083310, "SLUS_005.78");
    uint32 first, second;
    for (;;)
    {
        first = *first_data;
        second = *second_data;
        ++first_data;
        if (first != second)
        {
            first = *(first_data - 1);
            second = *second_data;
            return (sint32)(first - second);
        }
        count = (sint32)((uint32)count - 1);
        ++second_data;
        if (count <= 0)
            return 0;
    }
}

uint32 MoveBytes(uint8 *destination, const uint8 *source, uint32 count)
{
    uint32 result = (uintptr_t)source < (uintptr_t)destination;
    FUNCTION_MARKER(0x800663AC, "SLUS_005.78");
    if (count != 0)
    {
        if ((uintptr_t)source < (uintptr_t)destination)
        {
            source += count - 1;
            destination += count - 1;
            do
            {
                result = *source;
                --count;
                *destination = (uint8)result;
                if (count != 0)
                {
                    --source;
                    --destination;
                }
            } while (count != 0);
        }
        else
        {
            do
            {
                result = *source;
                ++source;
                --count;
                *destination = (uint8)result;
                ++destination;
            } while (count != 0);
        }
    }
    return result;
}
