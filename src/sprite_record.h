#ifndef SPRITE_RECORD_H
#define SPRITE_RECORD_H
#include <stddef.h>

#include "psx.h"
#include "animation_track.h"
#include "mesh_geometry_node.h"
#include "scene_node.h"
#include "vector3.h"

struct TextureDescriptor;

struct EncodedSpriteRecord;
struct SpriteRecord;
struct EncodedBillboardNode;
struct EncodedPositionedBillboardNode;
struct BillboardNode;
struct PositionedBillboardNode;
union SpriteCorner;

struct EncodedSpriteRecord
{
    EncodedSceneNode node;
    uint32 tex_idx;
    uint32 animation_offset;
    AnimationTrackBinding animation_binding;
    Vec3 pos;
    sint16 extent[2];
    uint16 angles[2];
};

typedef char EncodedSpriteRecordSize[(sizeof(EncodedSpriteRecord) == 48) ? 1 : -1];

// Runtime sprite borrows its texture descriptor
struct SpriteRecord
{
    SceneNode node;
    const TextureDescriptor *tex;
    uint32 *animation_entries;

    AnimationTrackBinding animation_binding;

    Vec3 pos;
    sint16 extent[2];
    uint16 angles[2];
    AnimationTrack *extent_track;

    void Draw() const;
    static void UpdateSprExtent(SpriteRecord *spr);
    static void ReleaseSpr(SpriteRecord *object);
    static void InitializeSprPool(void);
    static void SpawnEffectSpr(const sint32 *pos, uint32 scale, uint32 effect);
    void DrawGroundQuad() const;
    void DrawFacingQuad() const;
};

typedef char SpriteRecordTextureOffset[(offsetof(SpriteRecord, tex) == 16) ? 1 : -1];

typedef char SpriteRecordAnimationOffset[(offsetof(SpriteRecord, animation_entries) == 20) ? 1 : -1];

typedef char SpriteRecordPositionOffset[(offsetof(SpriteRecord, pos) == 28) ? 1 : -1];

typedef char SpriteRecordExtentOffset[(offsetof(SpriteRecord, extent) == 40) ? 1 : -1];

struct EncodedBillboardNode
{
    EncodedSceneNode node;
    uint16 event_id;
    uint16 playback_flags;
    uint32 tex_word;
    sint32 extent;
};

typedef char EncodedBillboardNodeSize[(sizeof(EncodedBillboardNode) == 28) ? 1 : -1];

struct EncodedPositionedBillboardNode
{
    EncodedBillboardNode billboard;
    Vec3 pos;
    Vec3 world_pos;
};

typedef char EncodedPositionedBillboardNodeSize[(sizeof(EncodedPositionedBillboardNode) == 52) ? 1 : -1];

struct BillboardNode
{
    SceneNode node;
    uint16 event_id;
    uint16 playback_flags;
    const TextureDescriptor *tex;
    sint32 extent;
};

typedef char BillboardNodeSize[(sizeof(BillboardNode) == 28) ? 1 : -1];

// Borrow the positioned prefix only where both vectors are accessed
struct PositionedBillboardNode
{
    BillboardNode billboard;
    Vec3 pos;
    Vec3 world_pos;
};

typedef char PositionedBillboardPrefixSize[(sizeof(PositionedBillboardNode) == 52) ? 1 : -1];

typedef char PositionedBillboardPositionOffset[(offsetof(PositionedBillboardNode, pos) == 28) ? 1 : -1];

typedef char PositionedBillboardWorldOffset[(offsetof(PositionedBillboardNode, world_pos) == 40) ? 1 : -1];

union SpriteCorner
{
    SVECTOR vertex;
    ProjectedVertex proj;
};

#endif
