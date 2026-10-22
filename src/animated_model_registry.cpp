#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "animated_model_registry.h"
#include "game.h"
#include "actor_model_registry.h"

AnimatedModelRegistry animated_models = {0};

void AnimatedModelRegistry::ResetAnimatedModels(void)
{
    FUNCTION_MARKER(0x800441DC, "SLUS_005.78");
    for (uint32 model_idx = 0; model_idx < sizeof(animated_models.recs) / sizeof(animated_models.recs[0]); ++model_idx)
    {
        free(animated_models.recs[model_idx].animation_data);
        if (animated_models.recs[model_idx].animation_headers != 0)
        {
            for (uint32 header_idx = 0; header_idx < 2; ++header_idx)
            {
                for (uint32 track_grp = 0; track_grp < 4; ++track_grp)
                    free(animated_models.recs[model_idx].animation_headers[header_idx].tracks[track_grp].bytes);
            }
        }
        free(animated_models.recs[model_idx].animation_headers);
        free(animated_models.recs[model_idx].morph_restoration);
        animated_models.recs[model_idx].animation_data = 0;
        animated_models.recs[model_idx].animation_headers = 0;
        animated_models.recs[model_idx].morph_restoration = 0;
        animated_models.recs[model_idx].animation_bytes = 0;
    }
    animated_models.count = 0;
}

void AnimatedModelRegistry::RegisterAnimatedModelOperations(void)
{
    FUNCTION_MARKER(0x800441E8, "SLUS_005.78");
    {
        ObjectCallback operation = {0, &AnimatedObject::UpdateAnimatedModelPathMotion};
        object_operations.motions[3] = operation;
    }
    {
        ObjectCallback operation = {0, &AnimatedObject::UpdateAnimatedModelState};
        object_operations.behaviors[22] = operation;
    }
    {
        ObjectCallback operation = {0, &AnimatedObject::HideAnimatedModel};
        object_operations.variants[4] = operation;
    }
    object_operations.collisions[4] = &AnimatedObject::HandleModelCol;
}
