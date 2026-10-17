#include "game.h"


uint32 ApplyCharacterSettings(uint32 scene, const CharacterSettings *settings, uint8 score)
{
    FUNCTION_MARKER(0x80053530, "SLUS_005.78");
    uint32 value;
    uint32 flags;
    if ((game_flow.mode_flags & 1) == 0)
        w_u8(0x800B999A, score);
    value = 32;
    if ((game_flow.mode_flags & 8) == 0)
        value = settings->counters[CharacterSettings::Lives];
    character_controller.vitals.lives = value;
    value = 24;
    if ((game_flow.mode_flags & 4) == 0)
        value = settings->health_limit;
    character_controller.vitals.health_limit = value;
    character_controller.vitals.health = value;
    flags = character_controller.motion.form_flags & 0xFF0FFFFF;
    character_controller.vitals.score = 0;
    character_controller.motion.form_flags = flags;
    flags = (flags | ((uint32)(settings->flags & 1) << 20)) & 0xFFF0FFFF;
    flags |= ((flags >> 20) & 15) << 16;
    character_controller.motion.form_flags = flags;
    value = settings->action_code;
    if (value == 0)
        character_controller.weapon_state = 0;
    else
        character_controller.weapon_state = value == 1 ? 1 : ((value + 9) << 16) | 1;
    character_controller.timers.idle_ticks = 900;
    value = settings->health_pickups;
    character_controller.vitals.health_pickups = value;
    return value;
}

uint32 CaptureCharacterSettings(CharacterSettings *settings)
{
    FUNCTION_MARKER(0x80053638, "SLUS_005.78");
    uint32 flags, value, result;
    flags = (character_controller.flags >> 21) & 2;
    settings->flags = flags;
    value = (character_controller.motion.form_flags >> 20) & 15;
    flags |= value;
    settings->flags = flags;
    settings->counters[CharacterSettings::Health] = (uint32)(sint8)character_controller.vitals.health;
    settings->health_limit = character_controller.vitals.health_limit;
    settings->counters[CharacterSettings::Lives] = character_controller.vitals.lives;
    settings->counters[CharacterSettings::Score] = (uint16)character_controller.vitals.score;
    value = character_controller.weapon_state;
    if ((value & 0xFFFF) != 0)
    {
        value >>= 16;
        if (value == 0)
            value = 1;
        else
            value -= 9;
    }
    else
        value = 0;
    settings->action_code = value;
    result = character_controller.vitals.health_pickups;
    settings->health_pickups = result;
    return result;
}

uint32 RestoreCharacterHealthPoint(void)
{
    FUNCTION_MARKER(0x800444D0, "SLUS_005.78");
    sint32 current = (sint8)character_controller.vitals.health;
    sint32 limit = (sint8)character_controller.vitals.health_limit;
    uint32 result = current < limit;
    if (result != 0)
    {
        result = (uint32)(current + 1);
        character_controller.vitals.health = result;
    }
    return result;
}
