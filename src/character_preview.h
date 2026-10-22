#ifndef CHARACTER_PREVIEW_H
#define CHARACTER_PREVIEW_H
#include <stddef.h>

#include "psx.h"
#include "vector3.h"

struct AnimatedObject;

struct AnimatedObject;

struct PreviewAnimationStep;
struct PreviewPlayback;
struct CharacterPreview;

// The original preview script contains one step
struct PreviewAnimationStep
{
    uint32 animation;
    sint32 velocity[2];
    uint32 duration;
};

struct PreviewPlayback
{
    uint32 tick, reset_word;
    const PreviewAnimationStep *first_step, *cur_step;
};

struct CharacterPreview
{
    AnimatedObject *object;
    Vec3 pos;
    Vec3 velocity;

    static AnimatedObject *PrepareCompletionCharacter(void);
    void Update();
    static void DrawLoadingFrame(void);
    void Draw();
    static void DrawCompletionFrame(sint32 percent, uint32 flags, sint32 next_level, CharacterPreview *preview, sint8 mode);
    static uint32 ShowLevelCompletion(uint32 level, sint8 mode);
};

extern const PreviewAnimationStep preview_animation_steps[1];

extern PreviewPlayback preview_playback;

#endif
