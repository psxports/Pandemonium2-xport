#ifndef RESOURCE_MEMORY_H
#define RESOURCE_MEMORY_H

#include "psx.h"

struct ResourceMemory
{
    uint32 allocated_bytes;
    uint8 *arena;
};

extern ResourceMemory resource_memory;

uint8 *AllocateGameMemory(uint32 bytes, uint32 storage_bytes = 0);
void FreeGameMemory(void *allocation);
uint32 *AllocateWordList(uint32 mode, uint32 count);
void FreeWordList(uint32 mode, uint32 *object);

uint32 FillBytes(uint8 *destination, uint8 value, uint32 count);
uint32 CopyBytes(uint8 *destination, const uint8 *source, uint32 count);
sint32 CompareByteRanges(const uint8 *first_data, const uint8 *second_data, sint32 count);

#endif
