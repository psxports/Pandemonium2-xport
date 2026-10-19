#include <string.h>

#include "game.h"

GameFlowState game_flow = {OriginalDefaults::GameTransitionFlags, OriginalDefaults::GameModeFlags};

uint32 CanOpenPauseMenu(void)
{
    FUNCTION_MARKER(0x800678A8, "SLUS_005.78");
    if ((gameplay_input.buttons & 0x800) == 0 || (gameplay_input.prev_buttons & 0x800) != 0)
    {
        if (IsGamepadDisconnected() == 0)
            return 0;
    }
    if (sequence_playback.current_handle == 0)
        return 0;
    return screen_render.transition_ticks == 0;
}

void RunGame(void)
{
    sint32 entry_index;
    uint32 level;
    uint32 transition;
    uint32 fade;
    uint32 style;
    uint32 frame_count;
    uint32 frame_wait;
    uint32 deadline;
    sint32 frame_fraction;
    sint32 fade_fraction;
    uint32 ending;
    CharacterSettings scene_info;

    FUNCTION_MARKER(0x8006792C, "SLUS_005.78");
    {
        RenderMatrixSlot *slot = &render_view.matrix_root;
        while (slot != 0)
        {
            uint32 default_offset = OriginalDefaults::RenderMatrixCursorOffset - 0x1F800000u + slot->depth * 32;
            uint32 default_bytes;
            memset(&slot->matrix, 0, sizeof(MATRIX));
            if (default_offset < sizeof(OriginalDefaults::ScratchpadStartup))
            {
                default_bytes = sizeof(OriginalDefaults::ScratchpadStartup) - default_offset;
                if (default_bytes > sizeof(MATRIX))
                    default_bytes = sizeof(MATRIX);
                CopyBytes((uint8 *)&slot->matrix, OriginalDefaults::ScratchpadStartup + default_offset, default_bytes);
            }
            slot = slot->next;
        }
    }
    CopyBytes((uint8 *)&rotation_x, OriginalDefaults::ScratchpadStartup, 20);
    CopyBytes((uint8 *)&rotation_y, OriginalDefaults::ScratchpadStartup + 0x14, 20);
    CopyBytes((uint8 *)&rotation_z, OriginalDefaults::ScratchpadStartup + 0x28, 20);
    fixed_math_result = xport_load_le32(OriginalDefaults::ScratchpadStartup + 0x30C);
    CopyBytes((uint8 *)&render_view.camera_transform, OriginalDefaults::ScratchpadStartup + 0x264, sizeof(MATRIX));
    CopyBytes((uint8 *)&render_view.inverse_transform, OriginalDefaults::ScratchpadStartup + 0x244, sizeof(MATRIX));
    CopyBytes((uint8 *)&render_view.billboard_transform, OriginalDefaults::ScratchpadStartup + 0x284, sizeof(MATRIX));
    PlayMovie("\\LOGO.STR;1");
    PlayMovie("\\MIDWAY.STR;1");
    while ((sint8)game_flow.requested_level <= 0)
    {
        if (game_flow.attract_phase == 1)
        {
            ResetSceneStreams();
            FadeMenuImage(0);
            RunDemo();
            ReloadMenuArchive();
            ResetNewGameCharacter();
            FadeMenuImage(0);
            game_flow.attract_phase = 0;
        }
        else
        {
            if ((sint32)game_flow.attract_phase >= 2)
            {
                if (game_flow.attract_phase == 2)
                {
                    PlayMovie("\\GEX.STR;1");
                    game_flow.attract_phase = 0;
                }
            }
            else if (game_flow.attract_phase == 0)
            {
                PlayMovie("\\INTRO.STR;1");
                game_flow.attract_phase = 1;
            }
            InitializeGame(1);
        }
    resume_menu:
        while ((uint8)resource_archive.Poll() == 0)
            ;
        LoadMenuResources();
        if (RunMainMenu())
        {
            entry_index = 0;
            if ((sint32)menu_state.resources->header.texture_count > 0)
            {
                do
                {
                    if (menu_state.material_groups[entry_index].animation_entries != 0)
                        menu_state.material_groups[entry_index].animation_entries = 0;
                    ++entry_index;
                } while (entry_index < (sint32)menu_state.resources->header.texture_count);
            }
            menu_state.resources = 0;
            StopCurrentSequence();
            if (sequence_playback.current_handle)
                CloseSequence(sequence_playback.current_handle);
            sequence_playback.current_handle = 0;
            goto start_level;
        }
        entry_index = 0;
        if ((sint32)menu_state.resources->header.texture_count > 0)
        {
            do
            {
                if (menu_state.material_groups[entry_index].animation_entries != 0)
                    menu_state.material_groups[entry_index].animation_entries = 0;
                ++entry_index;
            } while (entry_index < (sint32)menu_state.resources->header.texture_count);
        }
        menu_state.resources = 0;
        game_flow.requested_level = ~game_flow.requested_level;
        StopCurrentSequence();
        if (sequence_playback.current_handle)
            CloseSequence(sequence_playback.current_handle);
        sequence_playback.current_handle = 0;
        if (game_flow.attract_phase != 1)
        {
            UnloadLoopSequence();
            FadeMenuImage(0);
            p2_func_8006a614();
            ClearCharacterModels();
            UnloadResourceGroups();
            NoOp8006A528();
            FreeGameMemory(resource_memory.arena);
            NoOp80066CE8();
            ShutdownGameClock();
            CdControlBPSX(9, NULL, NULL);
            CdSync(0, NULL);
            ResetRenderCommands();
            p2_func_8006af4c();
            CdReadyCallbackPSX(0);
            SetCdReadCallback(NULL);
            CdSyncCallbackPSX(0);
        }
    }
    ResetSceneStreams();
    InitializeGame(0);
start_level:
    game_flow.attract_phase = 1;
    LoadLevel((sint8)game_flow.requested_level);
    frame_fraction = 30;
    frame_clock.frame_start = frame_clock.milliseconds;
    for (;;)
    {
        if (game_flow.transition_flags & 0x2B008)
        {
            level = level_resource.level_id;
            StartSceneFadeIn();
            UnloadLevelResources();
            if (game_flow.transition_flags & 8)
            {
                FadeMenuImage(0);
                game_flow.transition_flags = game_flow.transition_flags & (0xFFFD4FF7);
                goto start_level;
            }
            if ((game_flow.transition_flags & 0x9000) == 0)
            {
                ReloadMenuArchive();
                game_flow.requested_level = 0;
                if (game_flow.transition_flags & 0x20000)
                {
                    SaveMenuImage();
                    frame_count = 30;
                    fade_fraction = 30;
                    frame_clock.frame_start = frame_clock.milliseconds;
                    fade = 273;
                    do
                    {
                        style = fade < 4096 ? ((fade << 16) | 1) : 1;
                        BeginDraw();
                        ClearSceneOrderingTable();
                        DrawBackgroundRectangle(0, 0xF00140, 0);
                        RestoreMenuImage(1);
                        DrawText(0x7800A0, UiText::GameOver, style, 0);
                        EndDraw();
                        frame_wait = 33;
                        fade_fraction -= 10;
                        if (fade_fraction <= 0)
                        {
                            frame_wait = 34;
                            fade_fraction += 30;
                        }
                        deadline = frame_clock.frame_start + frame_wait;
                        while (frame_clock.milliseconds < deadline)
                            psx_native_poll_events();
                        --frame_count;
                        frame_clock.frame_start = frame_clock.milliseconds;
                        fade += 273;
                    } while (frame_count);
                }
                ResetNewGameCharacter();
                game_flow.transition_flags = game_flow.transition_flags & (0xFFFD4FF7);
                FadeMenuImage(0);
                goto resume_menu;
            }
            transition = (game_flow.transition_flags >> 15) & 1;
            game_flow.transition_flags = game_flow.transition_flags & (0xFFFD4FF7);
            if (level == 78)
            {
                game_flow.requested_level = 78;
                FindPlayerObject(0);
                CaptureCharacterSettings(&scene_info);
                ending = scene_info.flags & 1;
                UnloadLoopSequence();
                FadeMenuImage(0);
                p2_func_8006a614();
                ClearCharacterModels();
                UnloadResourceGroups();
                NoOp8006A528();
                FreeGameMemory(resource_memory.arena);
                NoOp80066CE8();
                ShutdownGameClock();
                CdControlBPSX(9, NULL, NULL);
                CdSync(0, NULL);
                ResetRenderCommands();
                p2_func_8006af4c();
                CdReadyCallbackPSX(0);
                SetCdReadCallback(NULL);
                CdSyncCallbackPSX(0);
                PlayMovie(ending ? "\\FOUTRO.STR;1" : "\\NOUTRO.STR;1");
                PlayMovie("\\CREDITS.STR;1");
                InitializeGame(1);
                game_flow.requested_level = 0;
                ResetNewGameCharacter();
                FadeMenuImage(0);
                goto resume_menu;
            }
            if (level - 96 >= 2)
            {
                if ((sint32)game_flow.progress.unlocked_stage < MapLevelProgression(level, 1))
                    game_flow.progress.unlocked_stage = (uint8)MapLevelProgression(level, 1);
            }
            ShowLevelCompletion(level, (sint8)transition);
        }
        UpdateSceneObjects();
        PollSceneResourceLoad();
        if (game_flow.transition_flags & 0x10)
        {
            RunPauseMenu(level_resource.level_id);
            gameplay_input.buttons = gameplay_input.buttons | (0x840);
            gameplay_input.prev_buttons |= 0x840;
        }
        frame_wait = 33;
        frame_fraction -= 10;
        if (frame_fraction <= 0)
        {
            frame_wait = 34;
            frame_fraction += 30;
        }
        deadline = frame_clock.frame_start + frame_wait;
        while (frame_clock.milliseconds < deadline)
            psx_native_poll_events();
        frame_clock.frame_start = frame_clock.milliseconds;
        frame_clock.render_tick = frame_clock.render_tick + (1);
    }
}

sint32 MapLevelProgression(uint32 level, uint32 mode)
{
    FUNCTION_MARKER(0x8002DFC8, "SLUS_005.78");
    const ProgressionEntry *entry = OriginalDefaults::Progression;
    sint32 value;
    do
    {
        value = mode != 0 ? entry->level : entry->stage;
        ++entry;
    } while ((uint32)value != level);
    --entry;
    return mode != 0 ? entry->stage : entry->level;
}

uint32 PlayMovie(const char *path)
{
    FUNCTION_MARKER(0x80064B18, "SLUS_005.78");
    /* User-authorized bypass of the separate FMV module */
    return 0;
}
