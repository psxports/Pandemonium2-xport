#include "game.h"

LevelLoader level_loader = {0};

uint32 LoadLevel(sint32 level, uint32 mode)
{
    uint32 player;
    uint32 previous;
    uint32 saved_stack = p2_guest_stack_pointer;
    uint32 camera_offset;
    FramePacer pacer = {30};
    uint32 result;
    uint32 sprite;
    uint32 position;
    uint32 first_x;
    uint32 first_y;
    uint32 last_x;
    uint32 last_y;
    uint32 copy;
    FUNCTION_MARKER(0x8002DCD0, "SLUS_005.78");
    p2_guest_stack_pointer -= 0x58;
    camera_offset = p2_guest_stack_pointer + 0x10;
    ResetHud();
    ResetCutsceneCameras();
    InitializeContactPool();
    player = r_u32(0x8008A114);
    w_u32(0x8008A4F4, 0);
    w_u8(player + 74, 0);
    w_u8(player + 73, 0);
    w_u8(player + 72, 0);
    previous = frame_clock.milliseconds;
    w_u32(camera_offset + 16, (uint32)-4096);
    w_u32(0x8008A444, 0xFFFFFFFF);
    player_interaction.flags = 0;
    player_interaction.target_offset = 0;
    w_u32(camera_offset + 4, 143360);
    w_u32(camera_offset + 8, (uint32)-61440);
    w_u32(camera_offset + 12, 122880);
    w_u32(camera_offset + 24, 0);
    w_u32(camera_offset + 20, 0);
    while ((PollLevelLoad(level, mode) << 24) == 0)
    {
        p2_func_80053ca0(camera_offset);
        BeginDraw();
        p2_func_8006de08(camera_offset);
        ClearSceneOrderingTable();
        p2_func_80066bdc(0, 0xF00140, 0);
        sprite = resource_groups[2].material_offset;
        for (copy = 0; copy < 2; ++copy)
        {
            last_x = r_u8(sprite + 6);
            first_x = r_u8(sprite + 4);
            last_y = r_u8(sprite + 7);
            first_y = r_u8(sprite + 5);
            position = (uint32)((319 - ((sint32)last_x - (sint32)first_x)) >> 1);
            position |= (uint32)((239 - ((sint32)last_y - (sint32)first_y)) >> 1) << 16;
            DrawTexturedQuad(position, 0, sprite, 0);
        }
        p2_func_80027228();
        EndDraw();
        previous = pacer.Wait(previous);
    }
    result = ActivateSceneSection(mode);
    p2_guest_stack_pointer = saved_stack;
    return result;
}

uint32 PollLevelLoad(sint32 level, uint32 mode)
{
    FUNCTION_MARKER(0x8002D7F8, "SLUS_005.78");
    const LevelDefinition *table;
    uint32 entry;
    uint32 active;
    uint32 asset;
    uint32 other;
    uint32 destination;
    uint32 bytes;
    uint32 base;
    uint32 capacity;
    uint32 loaded;
    uint32 index;
    uint32 offset;
    LevelResource *resource;
    uint32 flags;
    sint32 state;
    state = level_loader.phase;
    switch (state)
    {
        case 0:
            while (PollSceneResourceLoad() != 0)
                ;
            if (level == 0)
            {
                w_u32(0x8008A130, 0);
                flags = r_u32(0x1F800388 + 4 * mode);
                w_u32(0x1F800388 + 4 * mode, flags | 0x40);
            }
            else if ((game_flow.transition_flags & 2) == 0 && (uint32)level - 96 >= 2)
                w_u8(0x8008A43C, (uint8)p2_func_8002dfc8((uint32)level, 1));
            sound_effects.Stop(1);
            entry = OriginalDefaults::Levels[0].level_id;
            level_loader.definition = OriginalDefaults::Levels;
            if (entry != (uint32)level)
            {
                do
                {
                    table = level_loader.definition;
                    entry = table[1].level_id;
                    level_loader.definition = table + 1;
                } while (entry != 0xFFFF && entry != (uint32)level);
            }
            active = 3;
            if (level_loader.definition->preload_both_characters == 0)
            {
                active = 2;
                if (p2_func_80044780() == 0)
                    active = 1;
            }
            if (active != r_u32(0x8008A4FC))
            {
                ReloadBaseCharacterModels(active);
                w_u32(0x8008A4FC, active);
            }
            table = level_loader.definition;
            base = r_u32(0x8008A424);
            offset = r_u32(0x800B7A30 + 4 * active);
            asset = table->resource_asset;
            w_u32(0x8008A4DC, base + offset);
            bytes = resource_archive.Size(asset);
            base = r_u32(0x8008A4DC);
            capacity = r_u32(0x8008A284);
            w_u32(0x8008A4E0, base + bytes);
            w_u32(0x8008A420, capacity - (base + bytes - 1254400));
            ResetSceneStreams();
            table = level_loader.definition;
            destination = r_u32(0x8008A4DC);
            asset = table->resource_asset;
            loaded = ReadAssetToGuest(asset, destination, 0);
            asset = level_loader.definition->resource_asset;
            w_u32(0x8008A4E4, loaded);
            level_loader.phase = 1;
            ((LevelResource *)psx_addr(loaded, sizeof(LevelResource)))->asset_base = asset;
            break;
        case 1:
            if (((uint32)resource_archive.Poll() << 24) == 0)
                break;
            table = level_loader.definition;
            loaded = r_u32(0x8008A4E4);
            asset = table->sound_asset;
            other = table->resource_asset;
            w_u32(0x8008A488, loaded);
            w_u32(0x8008A46C, other + 2);
            p2_func_8006a530(asset, 1);
            ((const LevelResource *)psx_addr(r_u32(0x8008A4E4), sizeof(LevelResource)))->LoadAnimatedModels();
            ((const LevelResource *)psx_addr(r_u32(0x8008A4E4), sizeof(LevelResource)))->LoadModelGroups();
            ((const LevelResource *)psx_addr(r_u32(0x8008A4E4), sizeof(LevelResource)))->LoadActorModels();
            ((const LevelResource *)psx_addr(r_u32(0x8008A4E4), sizeof(LevelResource)))->LoadCharacterModels();
            if (((const LevelResource *)psx_addr(r_u32(0x8008A4E4), sizeof(LevelResource)))->level_id != 0 && (sint32)resource_archive.Size(level_loader.definition->image_asset) > 0)
            {
                destination = GetSceneLoadDestination(mode);
                asset = level_loader.definition->image_asset;
                ReadAssetToGuest(asset, destination, 0);
                level_loader.phase = 2;
                break;
            }
            goto load_sound;
        case 2:
            if (((uint32)resource_archive.Poll() << 24) != 0)
            {
                destination = GetSceneLoadDestination(mode);
                p2_func_8006624c(destination, 1, 0);
                level_loader.phase = 3;
            }
            break;
        case 3:
            if (DrawSync(1) > 0)
                break;
        load_sound:
            destination = GetSceneLoadDestination(mode);
            asset = level_loader.definition->sound_asset;
            ReadAssetToGuest(asset, destination, 0);
            level_loader.phase = 4;
            break;
        case 4:
            if (((uint32)resource_archive.Poll() << 24) != 0)
            {
                destination = GetSceneLoadDestination(mode);
                asset = level_loader.definition->sound_asset;
                bytes = resource_archive.Size(asset);
                p2_func_8006aa88(destination, bytes);
                level_loader.phase = 5;
            }
            break;
        case 5:
            if (p2_func_8006aaa8() == 0)
                break;
            if (((const LevelResource *)psx_addr(r_u32(0x8008A4E4), sizeof(LevelResource)))->level_id == 0)
            {
                index = r_u32(0x8008A46C);
                w_u32(0x8008A46C, index + 1);
                offset = resource_archive.Offset(index);
                base = r_u32(0x8008A488);
                world_level_names = (const WorldLevelName *)psx_addr(base + offset, sizeof(WorldLevelName));
            }
            if ((sint32)resource_archive.Offset(r_u32(0x8008A46C)) > 0)
            {
                offset = resource_archive.Offset(r_u32(0x8008A46C));
                base = r_u32(0x8008A488);
                loaded = p2_func_8006aa48(base + offset);
                w_u32(0x8008A460, loaded);
            }
            table = level_loader.definition;
            index = r_u32(0x8008A46C);
            resource = (LevelResource *)psx_addr(r_u32(0x8008A4E4), sizeof(LevelResource));
            asset = table->sound_asset;
            w_u32(0x8008A46C, index + 1);
            resource->collected_total = 0;
            resource->active_sections = 0;
            resource->asset_base = asset + 1;
            p2_func_80066358(0x800B996A, 0, 25);
            destination = 0;
            if (((const LevelResource *)psx_addr(r_u32(0x8008A4E4), sizeof(LevelResource)))->level_id == 0)
                destination = r_u32(0x80083CF4 + 4 * (uint32)(sint32)r_s8(0x8008A43C));
            RequestSceneSection(destination, mode);
            level_loader.phase = 6;
            break;
        default:
            if (state == 6 && PollSceneResourceLoad() == 0)
                level_loader.phase = 0;
            break;
    }
    return level_loader.phase == 0;
}

sint32 PollLevelLoadResult(sint32 level, uint32 mode)
{
    FUNCTION_MARKER(0x8002DEB8, "SLUS_005.78");
    return (sint8)PollLevelLoad(level, mode);
}
