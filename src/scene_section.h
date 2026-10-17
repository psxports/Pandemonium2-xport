#ifndef SCENE_SECTION_H
#define SCENE_SECTION_H

#include <stddef.h>

#include "psx.h"
#include "object_links.h"
#include "material_resources.h"

struct SceneObjectName
{
    char prefix[2];
    uint8 kind;
};

typedef char SceneObjectNameBinaryLayout[(sizeof(SceneObjectName) == 3) ? 1 : -1];

// This header still views the original mutable section buffer
struct SceneSection
{
    ObjectLinks links;
    uint8 kind;
    uint8 flags;
    sint8 section_index;
    sint8 palette_bank;
    sint16 transition_scale;
    uint16 state_flags;
    uint32 data_offset;
    uint32 metadata;
    uint32 object_heads[17];
    uint16 collected_count;
    uint16 metadata_halfword;
    ObjectLinks transient_links;
    uint32 runtime_words[3];
    uint32 sound_header_offset;
    uint32 sound_samples_offset;
    uint32 metadata_words[3];
    uint32 origin[3];
    MaterialResourceHeader materials;

    void Relocate(const MaterialResourceHeader *resource);
    uint32 Activate();
    sint32 FindObjectId(const SceneObjectName *name) const;
    uint32 FindObjectOffset(sint32 index) const;
};

typedef char SceneSectionBinaryLayout[(sizeof(SceneSection) == 188) ? 1 : -1];
typedef char SceneSectionHeadsOffset[(offsetof(SceneSection, object_heads) == 28) ? 1 : -1];
typedef char SceneSectionTransientOffset[(offsetof(SceneSection, transient_links) == 100) ? 1 : -1];
typedef char SceneSectionMaterialsOffset[(offsetof(SceneSection, materials) == 156) ? 1 : -1];

#endif
