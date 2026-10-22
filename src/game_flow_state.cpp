#include <string.h>

#include "game_flow_state.h"
#include "game.h"

GameFlowState game_flow = {OriginalDefaults::GameTransitionFlags, OriginalDefaults::GameModeFlags};

sint32 GameFlowState::MapLevelProgression(uint32 level, uint32 mode)
{
    FUNCTION_MARKER(0x8002DFC8, "SLUS_005.78");
    const ProgressionEntry *entry = OriginalDefaults::progression;
    sint32 val;
    do
    {
        val = mode != 0 ? entry->level : entry->stage;
        ++entry;
    } while (val != level);
    --entry;
    return mode != 0 ? entry->stage : entry->level;
}

uint32 GameFlowState::PlayMovie(const char *path)
{
    FUNCTION_MARKER(0x80064B18, "SLUS_005.78");
    /* User-authorized bypass of the separate FMV module */
    return 0;
}

uint32 GameFlowState::CanOpenPauseMenu(void)
{
    FUNCTION_MARKER(0x800678A8, "SLUS_005.78");
    if ((gameplay_input.buttons & 0x800) == 0 || (gameplay_input.prev_buttons & 0x800) != 0)
    {
        if (GameplayInput::IsGamepadDisconnected() == 0)
            return 0;
    }
    if (sequence_playback.cur_handle == 0)
        return 0;
    return scr_render.transition_ticks == 0;
}

void GameFlowState::RunGame(void)
{
    sint32 entry_idx;
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
            if (default_offset < sizeof(OriginalDefaults::scratchpad_startup))
            {
                default_bytes = sizeof(OriginalDefaults::scratchpad_startup) - default_offset;
                if (default_bytes > sizeof(MATRIX))
                    default_bytes = sizeof(MATRIX);
                CopyBytes((uint8 *)&slot->matrix, OriginalDefaults::scratchpad_startup + default_offset, default_bytes);
            }
            slot = slot->next;
        }
    }
    CopyBytes((uint8 *)&rotation_x, OriginalDefaults::scratchpad_startup, 20);
    CopyBytes((uint8 *)&rotation_y, OriginalDefaults::scratchpad_startup + 0x14, 20);
    CopyBytes((uint8 *)&rotation_z, OriginalDefaults::scratchpad_startup + 0x28, 20);
    fixed_math_result = OriginalDefaults::MathResult;
    CopyBytes((uint8 *)&render_view.camera_transform, OriginalDefaults::scratchpad_startup + 0x264, sizeof(MATRIX));
    CopyBytes((uint8 *)&render_view.inverse_transform, OriginalDefaults::scratchpad_startup + 0x244, sizeof(MATRIX));
    CopyBytes((uint8 *)&render_view.billboard_transform, OriginalDefaults::scratchpad_startup + 0x284, sizeof(MATRIX));
    GameFlowState::PlayMovie("\\LOGO.STR;1");
    GameFlowState::PlayMovie("\\MIDWAY.STR;1");
    while ((sint8)game_flow.requested_level <= 0)
    {
        if (game_flow.attract_phase == 1)
        {
            SceneStream::ResetSceneStreams();
            ScreenRenderState::FadeMenuImage(0);
            GameplayInput::RunDemo();
            MenuState::ReloadMenuArchive();
            CharacterController::ResetNewGameCharacter();
            ScreenRenderState::FadeMenuImage(0);
            game_flow.attract_phase = 0;
        }
        else
        {
            if ((sint32)game_flow.attract_phase >= 2)
            {
                if (game_flow.attract_phase == 2)
                {
                    GameFlowState::PlayMovie("\\GEX.STR;1");
                    game_flow.attract_phase = 0;
                }
            }
            else if (game_flow.attract_phase == 0)
            {
                GameFlowState::PlayMovie("\\INTRO.STR;1");
                game_flow.attract_phase = 1;
            }
            InitializeGame(1);
        }
    resume_menu:
        while (!resource_archive.Poll())
            ;
        MenuState::LoadMenuResources();
        if (MenuState::RunMainMenu())
        {
            entry_idx = 0;
            if ((sint32)menu_state.resources->header.tex_count > 0)
            {
                do
                {
                    if (menu_state.material_grps[entry_idx].animation_entries != 0)
                        menu_state.material_grps[entry_idx].animation_entries = 0;
                    ++entry_idx;
                } while (entry_idx < (sint32)menu_state.resources->header.tex_count);
            }
            menu_state.resources = 0;
            SequencePlayback::StopCurSequence();
            if (sequence_playback.cur_handle)
                SequencePlayback::CloseSequence(sequence_playback.cur_handle);
            sequence_playback.cur_handle = 0;
            goto start_level;
        }
        entry_idx = 0;
        if ((sint32)menu_state.resources->header.tex_count > 0)
        {
            do
            {
                if (menu_state.material_grps[entry_idx].animation_entries != 0)
                    menu_state.material_grps[entry_idx].animation_entries = 0;
                ++entry_idx;
            } while (entry_idx < (sint32)menu_state.resources->header.tex_count);
        }
        menu_state.resources = 0;
        game_flow.requested_level = ~game_flow.requested_level;
        SequencePlayback::StopCurSequence();
        if (sequence_playback.cur_handle)
            SequencePlayback::CloseSequence(sequence_playback.cur_handle);
        sequence_playback.cur_handle = 0;
        if (game_flow.attract_phase != 1)
        {
            SequencePlayback::UnloadLoopSequence();
            ScreenRenderState::FadeMenuImage(0);
            ReleaseSoundEffectVoices();
            CharacterModelRecord::ClearCharacterModels();
            ResourceGroup::UnloadResourceGrps();
            ResourceArchive::EmptyArchiveOperation();
            FreeGameMemory(resource_memory.arena);
            RenderCommands::EmptyRenderOperation();
            FrameClock::ShutdownGameClock();
            CdControlBPSX(9, NULL, NULL);
            CdSync(0, NULL);
            RenderCommands::ResetRenderCommands();
            ShutdownSound();
            CdReadyCallback(NULL);
            SetCdReadCallback(NULL);
            CdSyncCallback(NULL);
        }
    }
    SceneStream::ResetSceneStreams();
    InitializeGame(0);
start_level:
    game_flow.attract_phase = 1;
    LevelLoader::LoadLevel((sint8)game_flow.requested_level);
    frame_fraction = 30;
    frame_clock.frame_start = frame_clock.milliseconds;
    for (;;)
    {
        if (game_flow.transition_flags & 0x2B008)
        {
            level = level_resource.level_id;
            ScreenRenderState::StartSceneFadeIn();
            LevelLoader::UnloadLevelResources();
            if (game_flow.transition_flags & 8)
            {
                ScreenRenderState::FadeMenuImage(0);
                game_flow.transition_flags = game_flow.transition_flags & 0xFFFD4FF7;
                goto start_level;
            }
            if ((game_flow.transition_flags & 0x9000) == 0)
            {
                MenuState::ReloadMenuArchive();
                game_flow.requested_level = 0;
                if (game_flow.transition_flags & 0x20000)
                {
                    ScreenRenderState::SaveMenuImage();
                    frame_count = 30;
                    fade_fraction = 30;
                    frame_clock.frame_start = frame_clock.milliseconds;
                    fade = 273;
                    do
                    {
                        style = fade < 4096 ? ((fade << 16) | 1) : 1;
                        RenderCommands::BeginDraw();
                        RenderCommands::ClearSceneOrderingTable();
                        ScreenRenderState::DrawBackgroundRect(0, 0xF00140, 0);
                        ScreenRenderState::RestoreMenuImage(1);
                        UiText::DrawText(0x7800A0, UiText::game_over, style, 0);
                        RenderCommands::EndDraw();
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
                CharacterController::ResetNewGameCharacter();
                game_flow.transition_flags = game_flow.transition_flags & 0xFFFD4FF7;
                ScreenRenderState::FadeMenuImage(0);
                goto resume_menu;
            }
            transition = (game_flow.transition_flags >> 15) & 1;
            game_flow.transition_flags = game_flow.transition_flags & 0xFFFD4FF7;
            if (level == 78)
            {
                game_flow.requested_level = 78;
                CharacterController::FindPlayerObject(0);
                CharacterController::CaptureCharacterSettings(&scene_info);
                ending = scene_info.flags & 1;
                SequencePlayback::UnloadLoopSequence();
                ScreenRenderState::FadeMenuImage(0);
                ReleaseSoundEffectVoices();
                CharacterModelRecord::ClearCharacterModels();
                ResourceGroup::UnloadResourceGrps();
                ResourceArchive::EmptyArchiveOperation();
                FreeGameMemory(resource_memory.arena);
                RenderCommands::EmptyRenderOperation();
                FrameClock::ShutdownGameClock();
                CdControlBPSX(9, NULL, NULL);
                CdSync(0, NULL);
                RenderCommands::ResetRenderCommands();
                ShutdownSound();
                CdReadyCallback(NULL);
                SetCdReadCallback(NULL);
                CdSyncCallback(NULL);
                GameFlowState::PlayMovie(ending ? "\\FOUTRO.STR;1" : "\\NOUTRO.STR;1");
                GameFlowState::PlayMovie("\\CREDITS.STR;1");
                InitializeGame(1);
                game_flow.requested_level = 0;
                CharacterController::ResetNewGameCharacter();
                ScreenRenderState::FadeMenuImage(0);
                goto resume_menu;
            }
            if (level - 96 >= 2)
            {
                if (game_flow.progress.unlocked_stage < GameFlowState::MapLevelProgression(level, 1))
                    game_flow.progress.unlocked_stage = GameFlowState::MapLevelProgression(level, 1);
            }
            CharacterPreview::ShowLevelCompletion(level, transition);
        }
        SceneSection::UpdateSceneObjects();
        SceneLoader::PollSceneResourceLoad();
        if (game_flow.transition_flags & 0x10)
        {
            MenuState::RunPauseMenu(level_resource.level_id);
            gameplay_input.buttons = gameplay_input.buttons | 0x840;
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
        frame_clock.render_tick = frame_clock.render_tick + 1;
    }
}
