#ifndef ORIGINAL_DEFAULTS_H
#define ORIGINAL_DEFAULTS_H

struct LevelDefinition
{
    unsigned short level_id;
    unsigned short resource_asset;
    unsigned short image_asset;
    unsigned short sound_asset;
    unsigned int preload_both_characters;
};

struct OriginalDefaults
{
    enum
    {
        RandomSeed = 12345,
        GameTransitionFlags = 0,
        GameModeFlags = 0,
        InteractionFlags = 0,
        CharacterActionFlags = 0,
        CharacterActionOwner = 0,
        CharacterActionTarget = 0,
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
        FontResourceGroup = 3
    };
    static const unsigned char ControlSchemes[24][4];
    static const unsigned char SoundVoiceOrder[8];
    static const LevelDefinition Levels[22];
};

#endif
