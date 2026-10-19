#ifndef CHARACTER_PREVIEW_H
#define CHARACTER_PREVIEW_H

#include "psx.h"

// The original preview script contains one step
struct PreviewAnimationStep
{
    uint32 animation;
    uint32 velocity[2];
    uint32 duration;
};

struct PreviewPlayback
{
    uint32 tick, reset_word;
    const PreviewAnimationStep *first_step, *cur_step;
};

struct AnimatedObject;

struct CharacterPreview
{
    AnimatedObject *object;
    uint32 position[3];
    uint32 velocity[3];

    void Update();
    void Draw();
};

extern const PreviewAnimationStep preview_animation_steps[1];
extern PreviewPlayback preview_playback;

AnimatedObject *PrepareCompletionCharacter();
uint32 DrawLoadingFrame();
uint32 DrawCompletionFrame(sint32 percent, uint32 flags, sint32 next_level, CharacterPreview *preview, sint8 mode);
uint32 ShowLevelCompletion(uint32 level, sint8 mode);

#endif
