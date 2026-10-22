#ifndef MODEL_GROUP_DEFINITION_H
#define MODEL_GROUP_DEFINITION_H
#include <stddef.h>

#include "psx.h"
#include "material_catalog.h"
#include "sound_table_header.h"

struct AnimatedObject;
struct AnimationHeader;
struct MeshFace;
struct MeshVertex;

struct ModelGroupDefinition;
struct EncodedModelGroupMetadata;

// Native definition decoded from the 48-byte resource record
struct ModelGroupDefinition
{
    uint8 flags;
    uint8 kind;
    uint16 lifetime;
    const SoundBankStorage *sound_table;
    SoundBankStorage sound_bank;
    AnimatedObject *models[4];
    uint32 launch_speed;
    uint32 metadata_word;
    uint32 gravity;
    uint32 sound_radius;
    uint32 doppler_radius;
    uint8 contact_type;

    // Own complete model payloads for the four prototype slots
    uint8 *model_data[4];
    uint32 model_bytes[4];
    MeshFace *model_polygons[4];
    MeshVertex *model_vertices[4];
    MaterialCatalog material_resources[4];
    // Own animation banks for the four prototype slots
    uint8 *animation_data[4];
    uint32 animation_bytes[4];
    AnimationHeader *animation_headers[4];

    void InitializeSpawn(AnimatedObject *spawned, AnimatedObject *object, uint32 phase) const;
};

struct EncodedModelGroupMetadata
{
    uint8 flags;
    uint8 kind;
    uint16 lifetime;
    uint32 reserved4;
    uint32 model_offsets[4];
    uint32 launch_speed;
    uint32 metadata_word;
    uint32 gravity;
    uint32 sound_radius;
    uint32 doppler_radius;
    uint8 contact_type;
    uint8 reserved45[3];
};

typedef char EncodedModelGroupMetadataSize[(sizeof(EncodedModelGroupMetadata) == 48) ? 1 : -1];

#endif
