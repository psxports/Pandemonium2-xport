#ifndef ORIGINAL_DEFAULTS_H
#define ORIGINAL_DEFAULTS_H

#include "psx.h"
#include "psx_spu.h"

struct LevelDefinition
{
    unsigned short level_id;
    unsigned short resource_asset;
    unsigned short image_asset;
    unsigned short sound_asset;
    unsigned int preload_both_characters;
};

struct ProgressionEntry
{
    signed char level;
    signed char stage;
};

struct OriginalDefaults
{
    enum
    {
        RandomSeed = 12345,
        VideoMode = 0,
        GuestStackTop = 0x200000,
        GuestStackReserve = 0x8000,
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

    static const uint32 SceneContextIdentity;
    static const SpuSequenceScheduler SequenceSchedulerStartup;
    static const SpuSequenceClock SequenceClockStartup;
    static const SpuNativeTransferState SpuTransferStartup;
    static const unsigned char ScratchpadStartup[1024];
    static const unsigned char ControlSchemes[24][4];
    static const unsigned char SoundVoiceOrder[8];
    static const sint32 InteractionWave[16];
    static const signed char DemoLevels[3];
    static const signed char DemoForms[3];
    static const PSX_RECT TextureImageRects[10];
    static const PSX_RECT StartupImageRect;
    static const PSX_RECT MenuImageRect;
    static const ProgressionEntry Progression[20];
    static const unsigned int HubSections[20];
    static const unsigned int ResourceMemoryWords[6];
    static const LevelDefinition Levels[22];
};

#endif
