#ifndef RESOURCE_ARCHIVE_H
#define RESOURCE_ARCHIVE_H

#include <stddef.h>

struct AssetRecord
{
    unsigned int offset;
    unsigned int size;
};

class ResourceArchive
{
public:
    enum { EntryCount = 1820, IndexReadBytes = 16384 };

    bool LoadIndex(void *workspace);
    unsigned int Size(unsigned int asset) const;
    unsigned int Offset(unsigned int asset) const;
    void *ReadAsset(unsigned int asset, void *destination, unsigned int flags);
    bool Poll();

    AssetRecord entries[EntryCount];
    bool transfer_pending;
};

extern ResourceArchive resource_archive;
unsigned int ReadAssetToGuest(unsigned int asset, unsigned int destination, unsigned int flags);

#endif
