#ifndef RESOURCE_ARCHIVE_H
#define RESOURCE_ARCHIVE_H
#include <stddef.h>

#include "psx.h"

struct AssetRecord;
class ResourceArchive;
struct ResourceLoadCursor;

struct AssetRecord
{
    uint32 offset;
    uint32 size;
};

class ResourceArchive
{
public:
    enum
    {
        EntryCount = 1820,
        IndexReadBytes = 16384
    };

    AssetRecord entries[EntryCount];
    bool transfer_pending;

    void *ReadAsset(uint32 asset, void *dst, uint32 flags, uint32 container_asset = 0xFFFFFFFFu);
    bool Poll();
    uint32 Size(uint32 asset) const;
    uint32 Offset(uint32 asset) const;
    static sint32 FindDiscFile(CdlFILE *result, const char *path);
    bool LoadIdx();
    static void EmptyArchiveOperation(void);
};

struct ResourceLoadCursor
{
    uint32 asset;
    // Original package identity for relative nested asset offsets
    uint32 container_asset;
};

extern ResourceLoadCursor resource_load_cursor;

extern ResourceArchive resource_archive;

#endif
