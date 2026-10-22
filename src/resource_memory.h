#ifndef RESOURCE_MEMORY_H
#define RESOURCE_MEMORY_H
#include <stddef.h>

#include "psx.h"

struct ResourceMemory;

struct ResourceMemory
{
    uint32 allocated_bytes;
    uint8 *arena;
};

extern ResourceMemory resource_memory;

uint8 *AllocateGameMemory(uint32 bytes, uint32 storage_bytes = 0);

void FreeGameMemory(void *allocation);

void FillBytes(uint8 *dst, uint8 val, uint32 count);

void CopyBytes(uint8 *dst, const uint8 *src, uint32 count);

void MoveBytes(uint8 *dst, const uint8 *src, uint32 count);

#endif
