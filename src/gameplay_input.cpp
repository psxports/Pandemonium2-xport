#include "gameplay_input.h"
#include "game.h"

GameplayInput gameplay_input = {0};

void GameplayInput::ApplyDemo()
{
    FUNCTION_MARKER(0x800276B4, "SLUS_005.78");
    const DemoFrame *src = demo_cursor;

    if (src != 0)
    {
        demo_cursor = src + 1;
        buttons = src->buttons;
    }
}

void GameplayInput::RunDemo(void)
{
    FUNCTION_MARKER(0x800276DC, "SLUS_005.78");
    uint32 flags, prev_form, frames, ticks, deadline, wait, val, result;
    sint32 idx = (sint8)gameplay_input.demo_idx;
    sint32 fraction = 30;
    const ControlBindings::Scheme *old_mapping = control_bindings.active_scheme;
    sint8 levels[3] = {OriginalDefaults::demo_levels[0], OriginalDefaults::demo_levels[1], OriginalDefaults::demo_levels[2]};
    sint8 forms[3] = {OriginalDefaults::demo_forms[0], OriginalDefaults::demo_forms[1], OriginalDefaults::demo_forms[2]};
    flags = game_flow.transition_flags;
    control_bindings.active_scheme = &OriginalDefaults::control_schemes[0];
    gameplay_input.demo_cursor = (const DemoFrame *)(demo_background_workspace + idx * 2728);
    game_flow.transition_flags = flags | 2;
    prev_form = CharacterController::SelectCharacterForm(forms[idx]);
    idx = (sint8)gameplay_input.demo_idx;
    LevelLoader::LoadLevel(levels[idx]);
    ticks = frame_clock.milliseconds;
    frames = 682;
    do
    {
        if (GameplayInput::ReadGamepadButtons() != 0)
            break;
        fraction -= 10;
        SceneSection::UpdateSceneObjects();
        SceneLoader::PollSceneResourceLoad();
        wait = 33;
        if (fraction <= 0)
        {
            wait = 34;
            fraction += 30;
        }
        deadline = ticks + wait;
        while (frame_clock.milliseconds < deadline)
            ;

        val = frame_clock.render_tick;
        ticks = frame_clock.milliseconds;
        frames -= 1;
        frame_clock.render_tick = val + 1;
    } while (frames != 0);
    while (SceneLoader::PollSceneResourceLoad() != 0)
        ;
    LevelLoader::UnloadLevelResources();
    CharacterController::SelectCharacterForm(prev_form);
    flags = game_flow.transition_flags;
    val = gameplay_input.demo_idx;
    control_bindings.active_scheme = old_mapping;
    gameplay_input.demo_cursor = 0;
    flags &= 0xFFFFFFFD;
    val += 1;
    gameplay_input.demo_idx = val;
    result = (sint8)val;
    game_flow.transition_flags = flags;
    if (result == 3)
        gameplay_input.demo_idx = 0;
}

void GameplayInput::UpdateGameplayInput(void)
{
    FUNCTION_MARKER(0x8004A928, "SLUS_005.78");
    const ScenePath *path;
    uint32 timer, held;
    character_pad.motion.prev_state_code = character_pad.motion.state_code;
    path = camera_motion.path;
    character_pad.movement_flags = character_pad.movement_flags & 0xF83FFFFF;
    timer = (sint16)character_pad.timers.score_bonus_ticks;
    character_pad.path = path;
    if (timer != 0)
    {
        character_pad.timers.score_bonus_ticks = timer - 1;
        if ((sint16)(timer - 1) == 0)
            character_pad.flags = character_pad.flags & 0xFFBFFFFF;
    }
    gameplay_input.prev_buttons = gameplay_input.buttons;
    if ((game_flow.transition_flags & 0x102) != 0)
    {
        gameplay_input.buttons = 0;
        GameplayInput::ReadGamepadButtons();
    }
    else
        gameplay_input.buttons = GameplayInput::FilterDebugInput(GameplayInput::ReadGamepadButtons());
    held = gameplay_input.release_mask;
    if (held != 0)
    {
        uint32 input = gameplay_input.buttons;
        gameplay_input.release_mask = held & input;
        gameplay_input.buttons = input & ~held;
    }
    if ((sint32)CutscenePlayback::GetCutsceneMode() < 0)
        gameplay_input.buttons = gameplay_input.buttons & 0x900;
    gameplay_input.ApplyDemo();
}

uint32 GameplayInput::FilterDebugInput(uint32 input)
{
    FUNCTION_MARKER(0x80060FDC, "SLUS_005.78");
    uint32 flags, consumed = 0, val;
    if ((input & 0xF) == 0)
    {
        gameplay_input.debug_prev_buttons = 0;
        return input;
    }
    flags = game_flow.mode_flags;
    if (flags == 0)
    {
        gameplay_input.debug_prev_buttons = 0;
        return input;
    }
    if ((input & 5) == 5)
    {
        if ((flags & 0x80) != 0 && (input & 0xE000) != 0)
        {
            consumed = 1;
            if ((input & 0x8000) != 0)
            {
                camera_proj.rotation[2] = camera_proj.rotation[2] - 8;
            }
            else if ((input & 0x2000) != 0)
            {
                camera_proj.rotation[2] = camera_proj.rotation[2] + 8;
            }
            else if ((input & 0x4000) != 0)
                camera_proj.rotation[2] = 0;
            camera_proj.rotation[2] = camera_proj.rotation[2] & 0xFFF;
        }
    }
    else if ((input & 0x21) == 0x21)
    {
        if ((gameplay_input.debug_prev_buttons & 0x21) != 0x21 && (flags & 0x40) != 0)
        {
            val = gameplay_input.rotation_track_shift + 1;
            gameplay_input.rotation_track_shift = val;
            consumed = 1;
            if ((val & 0xFF) == 31)
                gameplay_input.rotation_track_shift = 1;
        }
    }
    else if ((input & 0x41) == 0x41 && (gameplay_input.debug_prev_buttons & 0x41) != 0x41 && (flags & 0x40) != 0)
    {
        consumed = 1;
        gameplay_input.rotation_track_shift = 0;
    }
    gameplay_input.debug_prev_buttons = input;
    return consumed != 0 ? 0 : input;
}

void GameplayInput::InitializeGamepad(void)
{
    FUNCTION_MARKER(0x80066CA8, "SLUS_005.78");
    InitPADNative(gameplay_input.pad_packet, 34, 0, 0);
    StartPAD();
    ChangeClearPAD(0);
}

uint32 GameplayInput::ReadGamepadButtons(void)
{
    uint32 high;
    uint32 low;
    FUNCTION_MARKER(0x80066D08, "SLUS_005.78");
    if (gameplay_input.pad_packet[0] == 255 || (gameplay_input.pad_packet[1] & 0xF0) != 0x40)
        return 0;
    high = gameplay_input.pad_packet[2];
    low = gameplay_input.pad_packet[3];
    return ((high << 8) | low) ^ 0xFFFF;
}

uint32 GameplayInput::IsGamepadDisconnected(void)
{
    FUNCTION_MARKER(0x80066D60, "SLUS_005.78");
    return gameplay_input.pad_packet[0] == 0xFF;
}
