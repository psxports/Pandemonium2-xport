#include <limits.h>
#include <stdlib.h>

#include "resource_archive.h"
#include "xport.h"
#include "psx.h"
#include "xport_trace.h"

ResourceArchive resource_archive = {0};
ResourceLoadCursor resource_load_cursor = {0};

bool ResourceArchive::LoadIndex()
{
    FUNCTION_MARKER(0x8006A440, "SLUS_005.78");
    unsigned char workspace[IndexReadBytes];
    unsigned int attempt;
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
                size_t transferred = xport_fread(workspace, 1, IndexReadBytes, file);
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
    const unsigned char *bytes = workspace;
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

void *ResourceArchive::ReadAsset(unsigned int asset, void *destination, unsigned int flags, unsigned int container_asset)
{
    FUNCTION_MARKER(0x8006A1AC, "SLUS_005.78");
    if (asset >= EntryCount || (container_asset != 0xFFFFFFFFu && container_asset >= EntryCount))
    {
        fprintf(stderr, "P2: Asset identity exceeds archive index\n");
        abort();
    }
    unsigned int offset;
    unsigned int bytes = Size(asset);
    if (container_asset == 0xFFFFFFFFu)
    {
        offset = (unsigned int)((int)(Offset(asset)) / 2048) * 2048;
        if (bytes & 2047)
            bytes += 2048 - (bytes & 2047);
    }
    else
    {
        unsigned int relative = Offset(asset);
        unsigned int container_bytes = Size(container_asset);
        unsigned int container_offset = Offset(container_asset);
        if (relative > container_bytes || bytes > container_bytes - relative || relative > 0xFFFFFFFFu - container_offset || container_offset + relative > LONG_MAX)
        {
            fprintf(stderr, "P2: Nested asset exceeds its package or file offset range\n");
            abort();
        }
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
                    size_t transferred = xport_fread(destination, 1, bytes, file);
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
    return destination;
}

bool ResourceArchive::Poll()
{
    FUNCTION_MARKER(0x8006A2AC, "SLUS_005.78");
    transfer_pending = false;
    return true;
}

sint32 FindDiscFile(CdlFILE *result, const char *path)
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

void NoOp8006A528(void)
{
    FUNCTION_MARKER(0x8006A528, "SLUS_005.78");
}
