#ifndef CHARACTER_MODELS_H
#define CHARACTER_MODELS_H

#include <stddef.h>

#include "character_controller.h"

// This record views the original mutable resource buffer
struct CharacterModelRecord
{
    MovementProfile movement;
    sint32 extent[2];
    sint32 motion_scale[3];
    uint32 animation_offsets[38];
    uint32 object_offset;
    uint32 sound_table_offset;
};

typedef char CharacterModelRecordBinaryLayout[(sizeof(CharacterModelRecord) == 240) ? 1 : -1];
typedef char CharacterModelRecordAnimationsOffset[(offsetof(CharacterModelRecord, animation_offsets) == 80) ? 1 : -1];
typedef char CharacterModelRecordObjectOffset[(offsetof(CharacterModelRecord, object_offset) == 232) ? 1 : -1];
typedef char CharacterModelRecordSoundOffset[(offsetof(CharacterModelRecord, sound_table_offset) == 236) ? 1 : -1];

extern "C"
{
uint32 RestoreCharacterModelAnimations(uint32 mode, uint32 scene_offset);
uint32 LoadCharacterModel(uint32 index);
uint32 ResetCharacterModels(void);
uint32 InitializeCharacterModels(void);
uint32 AssignCharacterModel(uint32 object_slot_offset, uint32 character_offset, uint32 kind);
void ClearCharacterModels(void);
uint32 GetCharacterModelObject(void);
uint32 HideAlternateCharacterModel(void);
uint32 SelectCharacterForm(uint32 mode);
uint32 ReloadBaseCharacterModels(uint32 mode);
}

#endif
