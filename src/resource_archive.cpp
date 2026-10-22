#include <limits.h>
#include <stdlib.h>

#include "resource_archive.h"
#include "game.h"
#include "psx.h"
#include "xport.h"
#include "xport_trace.h"

ResourceArchive resource_archive = {0};

ResourceLoadCursor resource_load_cursor = {0};

void *ResourceArchive::ReadAsset(uint32 asset, void *dst, uint32 flags, uint32 container_asset)
{
    FUNCTION_MARKER(0x8006A1AC, "SLUS_005.78");
    uint32 offset;
    uint32 bytes = Size(asset);
    if (container_asset == 0xFFFFFFFFu)
    {
        offset = ((sint32)Offset(asset) / 2048) * 2048;
        if (bytes & 2047)
            bytes += 2048 - (bytes & 2047);
    }
    else
    {
        uint32 relative = Offset(asset);
        Size(container_asset);
        uint32 container_offset = Offset(container_asset);
        offset = container_offset + relative;
    }
    bool complete = false;
    while (!complete)
    {
        if (offset <= LONG_MAX)
        {
            FILE *file = xport_fopen("DATA/JESTERS.PKG", "rb");
            if (file)
            {
                if (fseek(file, (long)offset, SEEK_SET) == 0)
                {
                    size_t transferred = xport_fread(dst, 1, bytes, file);
                    bool failed = ferror(file) != 0;
                    if (fclose(file) != 0)
                        failed = true;
                    complete = !failed && transferred == bytes;
                }
                else
                    fclose(file);
            }
        }
        if (!complete)
            psx_native_poll_events();
    }
    transfer_pending = (flags & 255) == 0;
    return dst;
}

bool ResourceArchive::Poll()
{
    FUNCTION_MARKER(0x8006A2AC, "SLUS_005.78");
    transfer_pending = false;
    return true;
}

uint32 ResourceArchive::Size(uint32 asset) const
{
    FUNCTION_MARKER(0x8006A3AC, "SLUS_005.78");
    return entries[asset].size;
}

uint32 ResourceArchive::Offset(uint32 asset) const
{
    FUNCTION_MARKER(0x8006A3C4, "SLUS_005.78");
    return entries[asset].offset;
}

sint32 ResourceArchive::FindDiscFile(CdlFILE *result, const char *path)
{
    uint32 attempts = 20;
    FUNCTION_MARKER(0x8006A3DC, "SLUS_005.78");
    do
    {
        if (CdSearchFile(result, path))
            return 1;
        --attempts;
    } while (attempts != 0);
    return 0;
}

bool ResourceArchive::LoadIdx()
{
    FUNCTION_MARKER(0x8006A440, "SLUS_005.78");

    struct
    {
        AssetRecord entries[EntryCount];
        uint8 padding[IndexReadBytes - EntryCount * sizeof(AssetRecord)];
    } index;

    uint32 attempt;
    FILE *file = 0;
    for (attempt = 0; attempt < 20 && !file; ++attempt)
        file = xport_fopen("DATA/JESTERS.PKG", "rb");
    if (!file)
        return false;
    fclose(file);
    bool complete = false;
    while (!complete)
    {
        file = xport_fopen("DATA/JESTERS.PKG", "rb");
        if (file)
        {
            if (fseek(file, 0, SEEK_SET) == 0)
            {
                size_t transferred = xport_fread(&index, 1, IndexReadBytes, file);
                bool failed = ferror(file) != 0;
                if (fclose(file) != 0)
                    failed = true;
                complete = !failed && transferred == IndexReadBytes;
            }
            else
                fclose(file);
        }
        if (!complete)
            psx_native_poll_events();
    }
    for (uint32 i = 0; i < EntryCount; ++i)
        entries[i] = index.entries[i];
    return true;
}

void ResourceArchive::EmptyArchiveOperation(void)
{
    FUNCTION_MARKER(0x8006A528, "SLUS_005.78");
}
