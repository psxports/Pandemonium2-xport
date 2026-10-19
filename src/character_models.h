#ifndef CHARACTER_MODELS_H
#define CHARACTER_MODELS_H

#include <stddef.h>

#include "character_controller.h"
#include "material_resources.h"

struct AnimationHeader;
struct MorphRestorationHeader;
struct MeshFace;
struct MeshVertex;

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
    sint32 motion_scale[3];
    AnimationHeader *animations[AnimationCount];
    AnimatedObject *object;
    uint8 *model_data;
    uint32 model_bytes;
    MeshFace *model_polygons;
    MeshVertex *model_vertices;
    MaterialResourceHeader material_resource;
    // Own the decoded animation bank independently of guest resource buffers
    uint8 *animation_data;
    uint32 animation_bytes;
    AnimationHeader *animation_headers;
    MorphRestorationHeader *morph_restoration;
};

struct CharacterModelHeader
{
    MovementProfile movement;
    sint32 extent[2];
    sint32 motion_scale[3];
    uint32 animation_offsets[CharacterModelRecord::AnimationCount];
    uint32 object_offset;
    uint32 sound_table_word;
};

typedef char CharacterModelHeaderBinaryLayout[(sizeof(CharacterModelHeader) == 240) ? 1 : -1];
typedef char CharacterModelHeaderAnimationsOffset[(offsetof(CharacterModelHeader, animation_offsets) == 80) ? 1 : -1];
typedef char CharacterModelHeaderObjectOffset[(offsetof(CharacterModelHeader, object_offset) == 232) ? 1 : -1];
typedef char CharacterModelHeaderSoundOffset[(offsetof(CharacterModelHeader, sound_table_word) == 236) ? 1 : -1];

extern CharacterModelRecord character_model_headers[CharacterController::ModelCount];

struct SceneSection;

extern "C"
{
    CharacterModelRecord *RestoreCharacterModelAnimations(void);
    uint32 LoadCharacterModel(uint32 index);
    uint32 PlaceCharacterOnPath(const SceneSection *scene, sint32 path_id, sint16 frame_word);
    uint32 IsAlternateCharacterInteractionActive(void);
    uint32 ResetCharacterModels(void);
    uint32 InitializeCharacterModels(void);
    uint32 AssignCharacterModel(AnimatedObject **object_slot, uint32 kind);
    void ClearCharacterModels(void);
    AnimatedObject *GetCharacterModelObject(void);
    uint32 HideAlternateCharacterModel(void);
    uint32 SelectCharacterForm(uint32 mode);
    void ReloadBaseCharacterModels(uint32 mode);
}

uint32 ResetNewGameCharacter();
AnimatedObject *FindPlayerObject(uint32 index);

#endif
