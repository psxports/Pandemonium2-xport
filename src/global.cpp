#include "global.h"
#include "original_defaults.h"

ActionEffectResources action_effect_resources = {0};

ControlBindings control_bindings = {&OriginalDefaults::control_schemes[0], 0};

MenuArchive menu_archive = {0};

ObjectOperations object_operations = {0};

PathRouting path_routing = {0, 0, 1};

const PathTransitionRules path_transition_defaults = {
    1, 8192, 2560, 12, 24, 6, 3, 4, 10, 10, 1, 0, 0xFFFFD800, 4, 2, 512, 8192, 40960, 53248, 53248, 6, 1, 0, 2, ONE, {2048, 3072}, {2048, 2048}, 2, 12 * ONE,
};

PathTransitionRules path_transition_rules = {0};

RenderViewState render_view = {0};

SoundResources sound_resources = {0};

SpritePoolState spr_pool = {0, 0, 0};

CollisionContact col_contact = {OriginalDefaults::ContactSelection, OriginalDefaults::ContactFraction, OriginalDefaults::ContactOtherSelection, OriginalDefaults::ContactFlags};

CharacterGeometry character_geometry = {0};

CollisionTransform col_transform = {0};

TriggerContactMotion trigger_contact_motion = {0};
