#ifndef SPRITE_OBJECTS_H
#define SPRITE_OBJECTS_H

#include <stddef.h>

#include "psx.h"
#include "scene_node.h"
#include "animation_data.h"

struct TextureDescriptor;

// Runtime sprite borrows its texture descriptor
struct SpriteRecord
{
    SceneNode node;
    const TextureDescriptor *texture;
    uint32 *animation_entries;

    AnimationTrackBinding animation_binding;

    uint32 position[3];
    sint16 extent[2];
    uint16 metadata_halfwords[2];
    AnimationTrack *extent_track;

    void Draw() const;
    uint32 DrawGroundQuad() const;
    uint32 DrawFacingQuad() const;
};

typedef char SpriteRecordTextureOffset[(offsetof(SpriteRecord, texture) == 16) ? 1 : -1];
typedef char SpriteRecordAnimationOffset[(offsetof(SpriteRecord, animation_entries) == 20) ? 1 : -1];
typedef char SpriteRecordPositionOffset[(offsetof(SpriteRecord, position) == 28) ? 1 : -1];
typedef char SpriteRecordExtentOffset[(offsetof(SpriteRecord, extent) == 40) ? 1 : -1];

extern "C" uint32 UpdateSpriteExtent(SpriteRecord *sprite);

void InitializeSpritePool();
void SpawnEffectSprite(const uint32 *position, uint32 scale, uint32 effect);
void ReleaseSprite(SpriteRecord *object);

struct BillboardNode
{
    SceneNode node;
    uint16 metadata16;
    uint16 playback_flags;
    const TextureDescriptor *texture;
    sint32 extent;
};

typedef char BillboardNodeSize[(sizeof(BillboardNode) == 28) ? 1 : -1];

// Borrow the positioned prefix only where both vectors are accessed
struct PositionedBillboardNode
{
    BillboardNode billboard;
    uint32 position[3];
    uint32 world_position[3];
};

typedef char PositionedBillboardPrefixSize[(sizeof(PositionedBillboardNode) == 52) ? 1 : -1];
typedef char PositionedBillboardPositionOffset[(offsetof(PositionedBillboardNode, position) == 28) ? 1 : -1];
typedef char PositionedBillboardWorldOffset[(offsetof(PositionedBillboardNode, world_position) == 40) ? 1 : -1];

#endif
