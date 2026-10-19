#include "game.h"

GameplayInput gameplay_input = {0};

unsigned int GameplayInput::ApplyDemo()
{
    FUNCTION_MARKER(0x800276B4, "SLUS_005.78");
    const unsigned char *source = demo_cursor;
    unsigned int result = 4;
    if (source != 0)
    {
        demo_cursor = source + 4;
        result = xport_load_le32(source);
        buttons = result;
    }
    return result;
}

uint32 FilterDebugInput(uint32 input)
{
    FUNCTION_MARKER(0x80060FDC, "SLUS_005.78");
    uint32 flags, consumed = 0, value;
    if ((input & 0xF) == 0)
    {
        gameplay_input.debug_previous_buttons = 0;
        return input;
    }
    flags = game_flow.mode_flags;
    if (flags == 0)
    {
        gameplay_input.debug_previous_buttons = 0;
        return input;
    }
    if ((input & 5) == 5)
    {
        if ((flags & 0x80) != 0 && (input & 0xE000) != 0)
        {
            consumed = 1;
            if ((input & 0x8000) != 0)
            {
                camera_projection.rotation[2] = (uint16)(camera_projection.rotation[2] - 8);
            }
            else if ((input & 0x2000) != 0)
            {
                camera_projection.rotation[2] = (uint16)(camera_projection.rotation[2] + 8);
            }
            else if ((input & 0x4000) != 0)
                camera_projection.rotation[2] = 0;
            camera_projection.rotation[2] = (uint16)(camera_projection.rotation[2] & 0xFFF);
        }
    }
    else if ((input & 0x21) == 0x21)
    {
        if ((gameplay_input.debug_previous_buttons & 0x21) != 0x21 && (flags & 0x40) != 0)
        {
            value = gameplay_input.rotation_track_shift + 1;
            gameplay_input.rotation_track_shift = (uint8)(value);
            consumed = 1;
            if ((value & 0xFF) == 31)
                gameplay_input.rotation_track_shift = (uint8)(1);
        }
    }
    else if ((input & 0x41) == 0x41 && (gameplay_input.debug_previous_buttons & 0x41) != 0x41 && (flags & 0x40) != 0)
    {
        consumed = 1;
        gameplay_input.rotation_track_shift = (uint8)(0);
    }
    gameplay_input.debug_previous_buttons = input;
    return consumed != 0 ? 0 : input;
}

uint32 InitializeGamepad(void)
{
    FUNCTION_MARKER(0x80066CA8, "SLUS_005.78");
    InitPADNative(gameplay_input.pad_packet, 34, 0, 0);
    StartPAD();
    return ChangeClearPAD(0);
}

uint32 ReadGamepadButtons(void)
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

uint32 IsGamepadDisconnected(void)
{
    FUNCTION_MARKER(0x80066D60, "SLUS_005.78");
    return gameplay_input.pad_packet[0] == 0xFF;
}

uint32 UpdateGameplayInput(void)
{
    FUNCTION_MARKER(0x8004A928, "SLUS_005.78");
    const ScenePath *path;
    uint32 timer, held;
    character_controller.motion.prev_state_code = character_controller.motion.state_code;
    path = camera_motion.path;
    character_controller.movement_flags = character_controller.movement_flags & 0xF83FFFFF;
    timer = (uint32)(sint16)character_controller.timers.score_bonus_ticks;
    character_controller.path = path;
    if (timer != 0)
    {
        character_controller.timers.score_bonus_ticks = timer - 1;
        if ((sint16)(timer - 1) == 0)
            character_controller.flags = character_controller.flags & 0xFFBFFFFF;
    }
    gameplay_input.prev_buttons = gameplay_input.buttons;
    if ((game_flow.transition_flags & 0x102) != 0)
    {
        gameplay_input.buttons = 0;
        ReadGamepadButtons();
    }
    else
        gameplay_input.buttons = FilterDebugInput(ReadGamepadButtons());
    held = gameplay_input.release_mask;
    if (held != 0)
    {
        uint32 input = gameplay_input.buttons;
        gameplay_input.release_mask = held & input;
        gameplay_input.buttons = input & ~held;
    }
    if ((sint32)GetCutsceneMode() < 0)
        gameplay_input.buttons = gameplay_input.buttons & 0x900;
    return gameplay_input.ApplyDemo();
}

uint32 RunDemo(void)
{
    FUNCTION_MARKER(0x800276DC, "SLUS_005.78");
    uint32 flags, prev_form, frames, ticks, deadline, wait, value, result;
    sint32 index = (sint8)gameplay_input.demo_index;
    sint32 fraction = 30;
    const ControlBindings::Scheme *old_mapping = control_bindings.active_scheme;
    sint8 levels[3] = {OriginalDefaults::DemoLevels[0], OriginalDefaults::DemoLevels[1], OriginalDefaults::DemoLevels[2]};
    sint8 forms[3] = {OriginalDefaults::DemoForms[0], OriginalDefaults::DemoForms[1], OriginalDefaults::DemoForms[2]};
    flags = game_flow.transition_flags;
    control_bindings.active_scheme = &OriginalDefaults::ControlSchemes[0];
    gameplay_input.demo_cursor = demo_background_workspace + index * 2728;
    game_flow.transition_flags = flags | 2;
    prev_form = SelectCharacterForm((uint32)(sint32)forms[index]);
    index = (sint8)gameplay_input.demo_index;
    LoadLevel(levels[index]);
    ticks = frame_clock.milliseconds;
    frames = 682;
    do
    {
        if (ReadGamepadButtons() != 0)
            break;
        fraction -= 10;
        UpdateSceneObjects();
        PollSceneResourceLoad();
        wait = 33;
        if (fraction <= 0)
        {
            wait = 34;
            fraction += 30;
        }
        deadline = ticks + wait;
        while (frame_clock.milliseconds < deadline)
            ;
        ticks = frame_clock.milliseconds;
        value = frame_clock.render_tick;
        ticks = frame_clock.milliseconds;
        frames -= 1;
        frame_clock.render_tick = value + 1;
    } while (frames != 0);
    while (PollSceneResourceLoad() != 0)
        ;
    UnloadLevelResources();
    SelectCharacterForm(prev_form);
    flags = game_flow.transition_flags;
    value = gameplay_input.demo_index;
    control_bindings.active_scheme = old_mapping;
    gameplay_input.demo_cursor = 0;
    flags &= 0xFFFFFFFD;
    value += 1;
    gameplay_input.demo_index = value;
    result = (uint32)(sint32)(sint8)value;
    game_flow.transition_flags = flags;
    if ((sint32)result == 3)
        gameplay_input.demo_index = 0;
    return result;
}
