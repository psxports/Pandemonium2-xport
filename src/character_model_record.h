#ifndef CHARACTER_MODEL_RECORD_H
#define CHARACTER_MODEL_RECORD_H
#include <stddef.h>

#include "psx.h"
#include "character_controller.h"
#include "material_catalog.h"
#include "vector3.h"

struct AnimatedObject;

struct AnimatedObject;
struct AnimationHeader;
struct MeshFace;
struct MeshVertex;
struct MorphRestorationHeader;
struct SceneSection;

struct CharacterModelRecord;
struct CharacterModelHeader;

// Copy the binary header into native ownership before runtime relocation
// Animation headers are native references; their payload ownership is separate
struct CharacterModelRecord
{
    enum
    {
        AnimationCount = 38
    };

    MovementProfile movement;
    sint32 extent[2];
    Vec3 motion_scale;
    AnimationHeader *animations[AnimationCount];
    AnimatedObject *object;
    uint8 *model_data;
    uint32 model_bytes;
    MeshFace *model_polygons;
    MeshVertex *model_vertices;
    MaterialCatalog material_resource;
    // Own the decoded animation bank independently of guest resource buffers
    uint8 *animation_data;
    uint32 animation_bytes;
    AnimationHeader *animation_headers;
    MorphRestorationHeader *morph_restoration;

    static void HideAlternateCharacterModel(void);
    static void ReloadBaseCharacterModels(uint32 mode);
    static uint32 IsAlternateCharacterInteractionActive(void);
    static CharacterModelRecord *RestoreCharacterModelAnimations(void);
    static void LoadCharacterModel(uint32 idx);
    static void ResetCharacterModels(void);
    static void InitializeCharacterModels(void);
    static void ClearPrimaryModelSlots(void);
    static uint32 AssignCharacterModel(AnimatedObject **object_slot, uint32 kind);
    static AnimatedObject *GetCharacterModelObject(void);
    static void ClearCharacterModels(void);
};

struct CharacterModelHeader
{
    MovementProfile movement;
    sint32 extent[2];
    Vec3 motion_scale;
    uint32 animation_offsets[CharacterModelRecord::AnimationCount];
    uint32 object_offset;
    uint32 sound_table_word;
};

typedef char CharacterModelHeaderBinaryLayout[(sizeof(CharacterModelHeader) == 240) ? 1 : -1];

typedef char CharacterModelHeaderAnimationsOffset[(offsetof(CharacterModelHeader, animation_offsets) == 80) ? 1 : -1];

typedef char CharacterModelHeaderObjectOffset[(offsetof(CharacterModelHeader, object_offset) == 232) ? 1 : -1];

typedef char CharacterModelHeaderSoundOffset[(offsetof(CharacterModelHeader, sound_table_word) == 236) ? 1 : -1];

extern CharacterModelRecord character_model_headers[CharacterController::ModelCount];

#endif
