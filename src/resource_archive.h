#ifndef RESOURCE_ARCHIVE_H
#define RESOURCE_ARCHIVE_H

#include <stddef.h>

#include "psx.h"

struct AssetRecord
{
    unsigned int offset;
    unsigned int size;
};

class ResourceArchive
{
public:
    enum
    {
        EntryCount = 1820,
        IndexReadBytes = 16384
    };

    bool LoadIndex();
    unsigned int Size(unsigned int asset) const;
    unsigned int Offset(unsigned int asset) const;
    void *ReadAsset(unsigned int asset, void *destination, unsigned int flags, unsigned int container_asset = 0xFFFFFFFFu);
    bool Poll();

    AssetRecord entries[EntryCount];
    bool transfer_pending;
};

struct ResourceLoadCursor
{
    uint32 asset;
    // Original package identity for relative nested asset offsets
    uint32 container_asset;
};

extern ResourceLoadCursor resource_load_cursor;
extern ResourceArchive resource_archive;

void NoOp8006A528(void);

#endif
