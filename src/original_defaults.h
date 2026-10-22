#ifndef ORIGINAL_DEFAULTS_H
#define ORIGINAL_DEFAULTS_H
#include <stddef.h>

#include "psx.h"
#include "psx_spu.h"

struct LevelDefinition;
struct ProgressionEntry;
struct OriginalDefaults;

struct LevelDefinition
{
    uint16 level_id;
    uint16 resource_asset;
    uint16 image_asset;
    uint16 sound_asset;
    uint32 preload_both_characters;
};

struct ProgressionEntry
{
    sint8 level;
    sint8 stage;
};

struct OriginalDefaults
{
    enum
    {
        RandomSeed = 12345,
        VideoMode = 0,
        RenderMatrixCursorOffset = 0x1F800044,
        StartupImageReadBytes = 14336,
        BehaviorBundleReadBytes = 4096,
        GameTransitionFlags = 0,
        GameModeFlags = 0,
        InteractionFlags = 0,
        CharacterActionFlags = 0,
        CharacterModelRebindPending = 1,
        MarkTransferredPath = 1,
        CharacterActionOwner = 0,
        CharacterActionTarget = 0,
        CharacterActionTargetScore = 0,
        CharacterSoundResumeFlags = 0,
        CharacterDamageTicks = 0,
        InteractionTarget = 0,
        ContactSelection = 0,
        ContactFraction = 0,
        ContactOtherSelection = 0,
        ContactFlags = 0,
        MenuHighlightStep = 409,
        PasswordResetPending = 1,
        EffectsVolume = 3072,
        MusicVolume = 2048,
        FontResourceGroup = 3,
        VolleyResetPending = 1,
        VolleyTurnPending = 1,
        AttackSpecialPending = 1,
        AttackEventPending = 1,
        PendingSceneEvent = -1,
        CameraProjectionScale = 955733,
        SequenceLoadingBank = -1,
        MenuImageAsset = -1
    };

    static const uint32 scene_context_identity;
    static const SpuSequenceScheduler sequence_scheduler_startup;
    static const SpuSequenceClock sequence_clock_startup;
    static const SpuNativeTransferState spu_transfer_startup;
    static const uint8 scratchpad_startup[1024];
    static const uint32 MathResult;
    static const uint8 control_schemes[24][4];
    static const uint8 sound_voice_order[8];
    static const sint32 interaction_wave[16];
    static const sint8 demo_levels[3];
    static const sint8 demo_forms[3];
    static const PSX_RECT tex_img_rects[10];
    static const PSX_RECT startup_img_rect;
    static const PSX_RECT menu_img_rect;
    static const ProgressionEntry progression[20];
    static const uint32 hub_sections[20];
    static const uint32 resource_memory_words[6];
    static const LevelDefinition levels[22];
};

#endif
