#include "resource_archive.h"
#include "xport.h"
#include "psx.h"
#include "xport_trace.h"
#include <limits.h>
#include <stdlib.h>

ResourceArchive resource_archive = {0};

static bool ReadPackage(unsigned int offset, void *destination, unsigned int bytes)
    {
        if (offset > LONG_MAX)
            return false;
        FILE *file = xport_fopen("DATA/JESTERS.PKG", "rb");
        if (!file)
            return false;
        if (fseek(file, (long)(offset), SEEK_SET) != 0)
        {
            fclose(file);
            return false;
        }
        const size_t transferred = xport_fread(destination, 1, bytes, file);
        bool failed = ferror(file) != 0;
        if (fclose(file) != 0)
            failed = true;
        return !failed && transferred == bytes;
    }

bool ResourceArchive::LoadIndex(void *workspace)
{
    FUNCTION_MARKER(0x8006A440, "SLUS_005.78");
    unsigned int attempt;
    FILE *file = 0;
    for (attempt = 0; attempt < 20 && !file; ++attempt) file = xport_fopen("DATA/JESTERS.PKG", "rb");
    if (!file) return false;
    fclose(file);
    while (!ReadPackage(0, workspace, IndexReadBytes)) psx_native_poll_events();
    const unsigned char *bytes = (const unsigned char *)(workspace);
    for (unsigned int i = 0; i < EntryCount; ++i)
    {
        entries[i].offset = xport_load_le32(bytes + i * 8);
        entries[i].size = xport_load_le32(bytes + i * 8 + 4);
    }
    return true;
}

unsigned int ResourceArchive::Size(unsigned int asset) const
{
    FUNCTION_MARKER(0x8006A3AC, "SLUS_005.78");
    return entries[asset].size;
}

unsigned int ResourceArchive::Offset(unsigned int asset) const
{
    FUNCTION_MARKER(0x8006A3C4, "SLUS_005.78");
    return entries[asset].offset;
}

void *ResourceArchive::ReadAsset(unsigned int asset, void *destination, unsigned int flags)
{
    FUNCTION_MARKER(0x8006A1AC, "SLUS_005.78");
    const unsigned int offset = (unsigned int)((int)(Offset(asset)) / 2048) * 2048;
    unsigned int bytes = Size(asset);
    if (bytes & 2047) bytes += 2048 - (bytes & 2047);
    while (!ReadPackage(offset, destination, bytes)) psx_native_poll_events();
    transfer_pending = (flags & 255) == 0;
    return destination;
}

bool ResourceArchive::Poll()
{
    FUNCTION_MARKER(0x8006A2AC, "SLUS_005.78");
    transfer_pending = false;
    return true;
}

// Remove this adapter when the remaining asset destinations are native buffers
unsigned int ReadAssetToGuest(unsigned int asset, unsigned int destination, unsigned int flags)
{
    ResourceArchive &archive = resource_archive;
    unsigned int bytes = archive.Size(asset);
    if (bytes & 2047) bytes += 2048 - (bytes & 2047);
    void *buffer = psx_addr(destination, bytes);
    archive.ReadAsset(asset, buffer, flags);
    return destination;
}
