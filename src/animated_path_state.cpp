#include <stdlib.h>

#include "animated_path_state.h"
#include "game.h"
#include "psx_gte.h"

uint32 AnimatedPathState::StopSound()
{
    FUNCTION_MARKER(0x800425F4, "SLUS_005.78");
    if ((sound_effects.IsPlaying(sound, sound_voice) & 0xFF) != 0)
        SoundEffects::StopSoundVoice(sound_voice);
    sound = 0;
    sound_voice = -1;
    return 0xFFFFFFFF;
}
