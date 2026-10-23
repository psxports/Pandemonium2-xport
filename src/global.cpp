#include "global.h"
#include "original_defaults.h"
#include "game.h"
#include "hd.h"

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

uint64 game_field = 0;
uint32 game_tick = 0, menu_tick = 0;

void InitializeGameClock(void)
{
    FUNCTION_MARKER(0x8006AF88, "SLUS_005.78");
    game_field = CurrentField() & ~(uint64)1;
}

void ShutdownGameClock(void)
{
    FUNCTION_MARKER(0x8006B000, "SLUS_005.78");
    // The shared field clock needs no game-owned BIOS event or counter
}

uint64 CurrentField(void)
{
    FUNCTION_MARKER(0x8006EBFC, "SLUS_005.78");
    return xport_frame();
}

uint64 WaitGameFrame(uint64 previous)
{
    FUNCTION_MARKER(0x80066D78, "SLUS_005.78");
    uint64 next = (previous & ~(uint64)1) + 2;
    uint64 current = CurrentField() & ~(uint64)1;
    if (next < current)
        next = current;
    hd_wait(next);
    return next;
}
