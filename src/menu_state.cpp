#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "menu_state.h"
#include "game.h"

MenuState menu_state = {0, 0, {0}, 0, 0, 0, 0, OriginalDefaults::MenuHighlightStep, {{0}, 0, true}, 0, {{0}}, OriginalDefaults::MenuImageAsset};

struct CheatRecord
{
    uint32 password[8];
    uint32 message[21];
};

static const CheatRecord cheats[12] = {{{0x0000E443, 0x0000E50B, 0x0000DF2F, 0x0000E69B, 0x0000E5D3, 0x0000E5D3, 0x0000E50B, 0x0000DF93}, {0x0000DE03, 0x0000E123, 0x0000DFF7, 0x0000E24F, 0x0000E56F, 0x0000F0C3, 0x0000DF93, 0x0000F37F, 0x0000DD3B, 0x0000E123, 0x0000DECB, 0x0000DFF7, 0x0000F37F, 0x0000E123, 0x0000DD3B, 0x0000DF93, 0x0000DF2F, 0x0000E50B, 0x0000DFF7, 0x0000F31B, 0x00000000}}, {{0x0000DF93, 0x0000E2B3, 0x0000E69B, 0x0000DF2F, 0x0000E637, 0x0000E123, 0x0000DFF7, 0x0000E56F}, {0x0000E637, 0x0000E123, 0x0000E69B, 0x0000DFF7, 0x0000E56F, 0x0000E50B, 0x0000DFF7, 0x0000F37F, 0x0000DFF7, 0x0000DECB, 0x0000E187, 0x0000F31B, 0x0000F37F, 0x0000F37F, 0x0000F37F, 0x0000F37F, 0x0000F37F, 0x0000F37F, 0x0000F37F, 0x0000F37F, 0x00000000}},
                                       {{0x0000E3DF, 0x0000E123, 0x0000DFF7, 0x0000E1EB, 0x0000E123, 0x0000E187, 0x0000E50B, 0x0000DF93}, {0x0000DD3B, 0x0000E123, 0x0000DECB, 0x0000F37F, 0x0000E24F, 0x0000E123, 0x0000E123, 0x0000E2B3, 0x0000F37F, 0x0000DF93, 0x0000E123, 0x0000F37F, 0x0000E3DF, 0x0000E50B, 0x0000E69B, 0x0000E24F, 0x0000DF2F, 0x0000E3DF, 0x0000DD3B, 0x0000F31B, 0x00000000}}, {{0x0000E37B, 0x0000E1EB, 0x0000E1EB, 0x0000E123, 0x0000DFF7, 0x0000DF2F, 0x0000E69B, 0x0000E24F}, {0x0000E24F, 0x0000E37B, 0x0000E4A7, 0x0000E50B, 0x0000F37F, 0x0000E123, 0x0000E4A7, 0x0000F37F, 0x0000DF2F, 0x0000E3DF, 0x0000E50B, 0x0000F37F, 0x0000E0BF, 0x0000E69B, 0x0000DFF7, 0x0000DF2F, 0x0000DD3B, 0x0000F31B, 0x0000F37F, 0x0000F37F, 0x00000000}},
                                       {{0x0000E1EB, 0x0000E69B, 0x0000E2B3, 0x0000E1EB, 0x0000DD3B, 0x0000E56F, 0x0000E69B, 0x0000DD3B}, {0x0000E0BF, 0x0000E50B, 0x0000DFF7, 0x0000E1EB, 0x0000E69B, 0x0000E187, 0x0000E50B, 0x0000E187, 0x0000DF2F, 0x0000F37F, 0x0000DE03, 0x0000E50B, 0x0000E69B, 0x0000E0BF, 0x0000E123, 0x0000E187, 0x0000F31B, 0x0000F37F, 0x0000F37F, 0x0000F37F, 0x00000000}}, {{0x0000E187, 0x0000E50B, 0x0000DE67, 0x0000E50B, 0x0000DFF7, 0x0000E56F, 0x0000E37B, 0x0000E50B}, {0x0000E5D3, 0x0000E69B, 0x0000E187, 0x0000F0C3, 0x0000DF2F, 0x0000F37F, 0x0000DF2F, 0x0000E123, 0x0000DECB, 0x0000E5D3, 0x0000E3DF, 0x0000F37F, 0x0000DF2F, 0x0000E3DF, 0x0000E37B, 0x0000DF93, 0x0000F31B, 0x0000F37F, 0x0000F37F, 0x0000F37F, 0x00000000}},
                                       {{0x0000E443, 0x0000E50B, 0x0000E187, 0x0000E50B, 0x0000DF2F, 0x0000E37B, 0x0000E5D3, 0x0000DF93}, {0x0000E1EB, 0x0000DECB, 0x0000DF2F, 0x0000E69B, 0x0000E187, 0x0000DF2F, 0x0000F37F, 0x0000E1EB, 0x0000E69B, 0x0000E187, 0x0000E37B, 0x0000E69B, 0x0000F31B, 0x0000F37F, 0x0000F37F, 0x0000F37F, 0x0000F37F, 0x0000F37F, 0x0000F37F, 0x0000F37F, 0x00000000}}, {{0x0000E443, 0x0000E123, 0x0000E187, 0x0000E69B, 0x0000E3DF, 0x0000DECB, 0x0000DFF7, 0x0000E24F}, {0x0000DFF7, 0x0000E123, 0x0000E5D3, 0x0000E2B3, 0x0000F37F, 0x0000F0C3, 0x0000E187, 0x0000F0C3, 0x0000F37F, 0x0000DFF7, 0x0000E123, 0x0000E24F, 0x0000E24F, 0x0000F31B, 0x0000F37F, 0x0000F37F, 0x0000F37F, 0x0000F37F, 0x0000F37F, 0x0000F37F, 0x00000000}},
                                       {{0x0000E317, 0x0000DECB, 0x0000DF93, 0x0000DF2F, 0x0000E2B3, 0x0000E37B, 0x0000E56F, 0x0000E187}, {0x0000E37B, 0x0000E1EB, 0x0000E1EB, 0x0000E123, 0x0000DFF7, 0x0000DF2F, 0x0000E69B, 0x0000E24F, 0x0000F37F, 0x0000E50B, 0x0000E187, 0x0000E50B, 0x0000E1EB, 0x0000E37B, 0x0000E50B, 0x0000DF93, 0x0000F31B, 0x0000F37F, 0x0000F37F, 0x0000F37F, 0x00000000}}, {{0x0000E443, 0x0000E05B, 0x0000E37B, 0x0000DF93, 0x0000E4A7, 0x0000DECB, 0x0000E187, 0x0000DD9F}, {0x0000E443, 0x0000E123, 0x0000E123, 0x0000E187, 0x0000F37F, 0x0000E05B, 0x0000DECB, 0x0000E50B, 0x0000E50B, 0x0000E187, 0x0000F31B, 0x0000F37F, 0x0000F37F, 0x0000F37F, 0x0000F37F, 0x0000F37F, 0x0000F37F, 0x0000F37F, 0x0000F37F, 0x0000F37F, 0x00000000}},
                                       {{0x0000E69B, 0x0000E69B, 0x0000E69B, 0x0000E69B, 0x0000E69B, 0x0000E69B, 0x0000E69B, 0x0000E69B}, {0x0000E50B, 0x0000E187, 0x0000DF2F, 0x0000E50B, 0x0000DFF7, 0x0000F37F, 0x0000E0BF, 0x0000E69B, 0x0000DF93, 0x0000DF93, 0x0000DE03, 0x0000E123, 0x0000DFF7, 0x0000E56F, 0x0000F37F, 0x0000E4A7, 0x0000E37B, 0x0000DFF7, 0x0000DF93, 0x0000DF2F, 0x00000000}}, {{0x0000E69B, 0x0000E5D3, 0x0000E37B, 0x0000E56F, 0x0000E56F, 0x0000DECB, 0x0000E56F, 0x0000E50B}, {0x0000E637, 0x0000E37B, 0x0000DCD7, 0x0000DCD7, 0x0000E69B, 0x0000DFF7, 0x0000E50B, 0x0000F37F, 0x0000E24F, 0x0000E69B, 0x0000E187, 0x0000E56F, 0x0000F31B, 0x0000F37F, 0x0000F37F, 0x0000F37F, 0x0000F37F, 0x0000F37F, 0x0000F37F, 0x0000F37F, 0x00000000}}};

uint32 MenuState::FindMaterialEntry(const TextureDescriptor *tex, uint32 kind, const MaterialCatalog *header, uint32 base)
{
    const MaterialRecord *entry;
    const MaterialRecord *rec;
    uint32 remaining;
    uint32 idx;
    uint32 result;
    FUNCTION_MARKER(0x8002A34C, "SLUS_005.78");
    entry = header->materials;
    remaining = header->material_count;
    for (;;)
    {
        rec = entry;
        if (rec->tex == tex && rec->kind == kind)
            break;
        --remaining;
        ++entry;
        if (remaining == 0)
            return 0xFFFFFFFF;
    }
    idx = entry - header->materials;
    result = (idx + base) << 1;
    if (kind != 0)
        result |= (rec->flags & 15) << 26;
    return result;
}

void MenuState::LoadMenuImage(sint32 asset)
{
    uint32 offset;

    PSX_RECT image_rect = OriginalDefaults::menu_img_rect;
    FUNCTION_MARKER(0x80066DE4, "SLUS_005.78");
    if (menu_state.image_asset != asset)
    {
        menu_state.image_asset = asset;
        DrawSync(0);
        VSync(0);
        offset = resource_archive.Offset(asset);

        LoadImagePSX(&image_rect, (uint32 *)(menu_archive.data + offset));
        DrawSync(0);
        return;
    }
}

void MenuState::ReloadMenuArchive(void)
{
    FUNCTION_MARKER(0x80066E48, "SLUS_005.78");
    uint32 flags = game_flow.mode_flags;
    uint32 bytes = resource_archive.Size(254);
    uint32 capacity;
    menu_state.image_asset = -1;
    capacity = (bytes + 2047) & ~2047u;
    if (capacity > menu_archive.capacity)
    {
        uint8 *data = (uint8 *)realloc(menu_archive.data, capacity);
        menu_archive.data = data;
        menu_archive.capacity = capacity;
    }
    menu_archive.bytes = bytes;
    resource_archive.ReadAsset(254, menu_archive.data, (flags & 0x200) != 0);
}

void MenuState::LoadMenuResources(void)
{
    uint32 offset;
    uint32 len;
    uint32 demo1_bytes;
    uint32 demo2_bytes;
    MaterialCatalog *header;
    const uint8 *encoded_header;
    uint32 material_bytes;
    uint32 count;
    uint32 idx;
    uint32 kind;
    uint32 sel;
    MaterialRecord *material_dst;
    uint32 vals[6];
    FUNCTION_MARKER(0x80066E98, "SLUS_005.78");
    if ((game_flow.mode_flags & 0x200) == 0 && (sint32)resource_archive.Size(260) > 0 && (sint32)resource_archive.Size(261) > 0 && (sint32)resource_archive.Size(262) > 0)
    {
        offset = resource_archive.Offset(260);
        len = resource_archive.Size(260);
        demo1_bytes = resource_archive.Size(261);
        demo2_bytes = resource_archive.Size(262);
        len += demo1_bytes + demo2_bytes;
        CopyBytes(demo_background_workspace, menu_archive.data + offset, len);
    }
    sequence_playback.cur_handle = 0;
    if ((sint32)resource_archive.Offset(263) > 0)
    {
        do
        {
            uint32 bundle_bytes = menu_archive.bytes;
            offset = resource_archive.Offset(263);
            len = resource_archive.Size(263);
            sequence_playback.cur_handle = SequencePlayback::LoadSequence(257, 0, menu_archive.data, bundle_bytes, menu_archive.data + offset, len);
        } while (sequence_playback.cur_handle == 0);
        SequencePlayback::StartCurSequence();
    }
    offset = resource_archive.Offset(264);
    material_bytes = resource_archive.Size(264);
    encoded_header = menu_archive.data + offset;
    header = &menu_state.resource_view.header;
    MaterialCatalog::RelocateMaterialResource(header, encoded_header, material_bytes, -1);
    menu_state.resource_view.texs = header->tex_count != 0 ? header->texs : NULL;
    menu_state.resources = &menu_state.resource_view;
    count = header->material_count;
    if (count != 0 && (sint32)(material_registry.count + count) < 51)
    {
        material_dst = material_registry.recs + material_registry.count;
        for (uint32 rec_idx = 0; rec_idx < count; ++rec_idx)
        {
            material_dst[rec_idx] = header->materials[rec_idx];
        }
        material_registry.count = material_registry.count + header->material_count;
    }
    idx = 0;
    if ((sint32)header->tex_count > 0)
    {
        do
        {
            sel = 0;
            menu_state.material_grps[idx].tex = menu_state.resource_view.texs + idx;
            for (kind = 0; kind < 3; ++kind)
            {
                vals[sel] = MenuState::FindMaterialEntry(menu_state.material_grps[idx].tex, kind, header, material_registry.count - header->material_count);
                if (vals[sel] != 0xFFFFFFFF)
                    ++sel;
            }
            if (sel != 0)
            {
                memset(&menu_state.material_grps[idx].animation, 255, 4 * (sel + 1));
                menu_state.material_grps[idx].animation.header.count = sel;
                menu_state.material_grps[idx].animation.header.start_tick = 0;
                menu_state.material_grps[idx].animation_entries = menu_state.material_grps[idx].animation.entries;
                CopyBytes((uint8 *)menu_state.material_grps[idx].animation_entries, (const uint8 *)vals, 4 * sel);
            }
            ++idx;
        } while ((sint32)idx < (sint32)header->tex_count);
    }
    offset = resource_archive.Offset(265);
    ScreenRenderState::UploadTexImage(menu_archive.data + offset, 9, 1);
}

uint32 MenuState::DrawMenuSelection(uint32 pos, uint32 alternate, sint32 selection)
{
    FUNCTION_MARKER(0x8006B04C, "SLUS_005.78");
    uint32 grp;
    if (alternate != 0)
        grp = (uint32)selection + 2;
    else
    {
        switch (selection)
        {
            case 0:
                grp = 6;
                break;
            case 1:
                grp = 8;
                break;
            case 2:
                grp = 7;
                break;
        }
    }
    return MaterialCatalog::DrawTexturedQuad(pos, 0, menu_state.resources->texs + grp, 0);
}

void MenuState::DrawPassword(uint32 x, uint32 y, sint8 interactive)
{
    TextureDescriptor *tex;
    uint32 *entries;
    TextureDescriptor *grp;
    uint32 encoded;
    const uint8 *glyphs;
    uint32 pos;
    uint32 idx;
    const uint8 *src;
    TextureFrame glyph;
    sint32 amplitude;
    FUNCTION_MARKER(0x8006B0E0, "SLUS_005.78");
    entries = resource_grps[3].animation_entries;
    encoded = ((const MaterialAnimationListHeader *)entries - 1)->count;
    grp = resource_grps[3].tex;
    encoded = entries[encoded - 1];
    glyphs = material_registry.recs[(encoded >> 1) & 0x1FF].data;
    if (interactive == 0)
        ScreenRenderState::DrawBrightenRect((x + 2) | ((y - 1) << 16), 0x140078, 4096, 0x800080);
    x += 4;
    y += 2;
    pos = x + (y << 16);
    for (idx = 0; idx < 8; ++idx)
    {
        src = glyphs + 8 * menu_state.password.characters[idx];
        glyph = *(const TextureFrame *)(src + 264);
        tex = grp;
        tex->mapping.uv_rect = glyph.uv_rect;
        tex->mapping.page_clut = glyph.page_clut;
        if (grp != 0)
            MaterialCatalog::DrawTexturedQuad(x | (y << 16), 0, grp, 0);
        if (interactive != 0 && (sint32)idx == (sint8)menu_state.password.cursor)
        {
            {
                uint32 phase = menu_state.highlight_phase;
                uint32 step = menu_state.highlight_step;
                phase += step;
                menu_state.highlight_phase = phase;
                if ((sint32)phase >= 2048)
                {
                    menu_state.highlight_phase = 2048;
                    menu_state.highlight_step = -step;
                }
                else if ((sint32)phase < -2047)
                {
                    menu_state.highlight_phase = -2048;
                    menu_state.highlight_step = -step;
                }
                amplitude = menu_state.highlight_phase;
            }
            if (amplitude > 0)
                ScreenRenderState::DrawBrightenRect(pos - 65537, 0x10000E, amplitude, 0x808080);
            else if (amplitude < 0)
                ScreenRenderState::DrawOverlayRect(pos - 65537, 0x10000E, (-(uint32)amplitude), 0x808080);
        }
        pos += 15;
        x += 15;
    }
}

void MenuState::DrawPasswordScr(sint32 requested_mode, uint8 invalid, uint8 accepted)
{
    const MenuMaterialResources *resources;
    sint8 mode = requested_mode;
    const char *text;
    const TextureDescriptor *grp;
    uint32 saved_frame;
    sint32 count;
    uint32 idx;
    uint32 fade;
    uint32 result;
    FUNCTION_MARKER(0x8006B2B8, "SLUS_005.78");
    DrawSync(0);
    VSync(0);
    RenderCommands::BeginDraw();
    RenderCommands::ClearSceneOrderingTable();
    if (mode == 0)
        ScreenRenderState::DrawBackgroundRect(0, 0xF00140, 0);
    else
        ScreenRenderState::RestoreMenuImage(0);
    MenuState::DrawPassword(97, 110, mode);
    if (invalid != 0 || accepted != 0)
    {
        ScreenRenderState::DrawOverlayRect(0xBE0000, 0x150140, 4096, 0xFFFFFF);
        text = invalid != 0 ? UiText::invalid_password : menu_state.password.feedback;
        UiText::DrawText(0xC300A0, text, 1, 0);
    }
    else if (mode == 0)
    {
        UiText::DrawText(0x4600A0, UiText::password, 1, 0);
        UiText::DrawText(0xD20064, UiText::continue_text, 0, 0);
        MaterialCatalog::DrawTexturedQuad(0xC80041, 0, interface_resources->texs + 11, 0);
    }
    grp = menu_state.resources->texs + 1;
    if (grp != 0)
        MaterialCatalog::DrawTexturedQuad(0x2700E4, 0, grp, 0);
    MaterialRegistry::UpdateResourceMaterialAnimations();
    resources = menu_state.resources;
    if (resources != 0)
    {
        saved_frame = frame_clock.render_tick;
        count = resources->header.tex_count;
        frame_clock.render_tick = frame_clock.menu_tick;
        if (count > 0)
        {
            idx = 0;
            do
            {
                MaterialAnimationList::UpdateMaterialAnimationList(menu_state.material_grps[idx].animation_entries);
                ++idx;
            } while ((sint32)idx < (sint32)menu_state.resources->header.tex_count);
        }
        frame_clock.render_tick = saved_frame;
    }
    fade = menu_state.fade_frames;
    if (fade != 0)
    {
        --fade;
        menu_state.fade_frames = fade;
        if (fade != 0)
            ScreenRenderState::DrawOverlayRect(0, 0xF00140, (sint32)(fade << 12) / 15, 0xFFFFFF);
    }
    RenderCommands::EndDraw();
    result = menu_state.pacer.Wait(menu_state.last_frame);
    menu_state.last_frame = result;
}

void MenuState::GeneratePassword(void)
{
    FUNCTION_MARKER(0x8006B548, "SLUS_005.78");
    CharacterSettings settings;
    uint32 rotation, checkpoints, level, lives, pair, triple, checksum;
    uint32 bits, idx, mask, val;
    CharacterController::FindPlayerObject(0);
    CharacterController::CaptureCharacterSettings(&settings);
    val = settings.counters[CharacterSettings::CNT_LIVES] - 1u;
    settings.counters[CharacterSettings::CNT_LIVES] = val;
    if (val < 2)
        settings.counters[CharacterSettings::CNT_LIVES] = 2;
    settings.health_limit = settings.health_limit - 1u;
    rotation = random_generator.Next() & 3;
    checkpoints = game_flow.progress.unlocked_stage;
    if (GameFlowState::MapLevelProgression(78, 1) < (sint32)checkpoints)
        checkpoints = (sint8)game_flow.progress.sel_stage;
    bits = 0;
    for (idx = 0; idx < 5; ++idx)
        if ((checkpoints & (1u << idx)) != 0)
            bits |= 1u << ((idx + rotation + 1) % 5);
    checkpoints = bits;
    val = settings.counters[CharacterSettings::CNT_LIVES];
    level = 0;
    for (idx = 0; idx < 5; ++idx)
        if ((val & (1u << idx)) != 0)
            level |= 1u << ((idx + rotation + 1) % 5);
    settings.counters[CharacterSettings::CNT_LIVES] = level & 255;
    val = settings.health_limit;
    lives = 0;
    for (idx = 0; idx < 5; ++idx)
        if ((val & (1u << idx)) != 0)
            lives |= 1u << ((idx + rotation + 1) % 5);
    settings.health_limit = lives;
    val = settings.action_code;
    pair = 0;
    for (idx = 0; idx < 2; ++idx)
        if ((val & (1u << idx)) != 0)
            pair |= 1u << ((idx + rotation + 1) % 2);
    settings.action_code = pair;
    settings.flags = settings.flags & 1;
    val = settings.health_pickups;
    triple = 0;
    for (idx = 0; idx < 3; ++idx)
        if ((val & (1u << idx)) != 0)
            triple |= 1u << ((idx + rotation + 1) % 3);
    settings.health_pickups = triple;
    val = checkpoints + settings.counters[CharacterSettings::CNT_LIVES] + settings.health_limit + settings.action_code + (settings.flags & 1) + (triple & 255) + rotation;
    checksum = 0;
    for (idx = 0; idx < 9; ++idx)
        if ((val & (1u << idx)) != 0)
            checksum |= 1u << ((idx + rotation + 1) % 9);
    mask = (1u << rotation) - 1u;
    val = (checkpoints << rotation) | (settings.counters[CharacterSettings::CNT_LIVES] << (rotation + 5)) | (settings.health_limit << (rotation + 10)) | (settings.action_code << (rotation + 15)) | ((settings.flags & 1u) << (rotation + 17)) | (settings.health_pickups << (rotation + 18)) | (rotation << 30) | ((checksum >> rotation) << (rotation + 21)) | (checksum & mask);
    for (idx = 8; idx != 0; --idx)
    {
        menu_state.password.characters[idx - 1] = val & 15;
        val >>= 4;
    }
}

sint32 MenuState::ApplyCheatPassword(void)
{
    FUNCTION_MARKER(0x8006B9AC, "SLUS_005.78");
    uint8 password[8];
    uint32 row;
    uint32 idx;
    const uint32 *message;
    uint32 bit;
    uint32 count = 0;
    for (idx = 0; idx < 8; ++idx)
        password[idx] = menu_state.password.characters[idx] + 65;
    for (row = 0; row < 12; ++row)
    {
        for (idx = 0; idx < 8; ++idx)
        {
            uint32 encoded = ((uint32)password[idx] * 100) ^ 0xFFFF;
            if (encoded != cheats[row].password[idx])
                break;
        }
        if (idx == 8)
            break;
    }
    if (row == 12)
        return 0;
    bit = 1u << row;
    if (bit == 1)
        game_flow.progress.unlocked_stage = GameFlowState::MapLevelProgression(78, 1);
    else if (bit == 32)
        game_flow.transition_flags = game_flow.transition_flags & ~0x10000u;
    else
        game_flow.mode_flags = game_flow.mode_flags & ~2u;
    if (bit != 1024)
        game_flow.mode_flags = game_flow.mode_flags | bit;
    message = cheats[row].message;
    if (*message != 0)
    {
        uint32 encoded;
        do
        {
            encoded = *message ^ 0xFFFF;
            ++message;
            menu_state.password.feedback_buffer[count] = (uint8)((sint32)encoded / 100);
            encoded = *message;
            ++count;
        } while (encoded != 0);
    }
    --count;
    if (menu_state.password.feedback_buffer[count] == 32)
    {
        do
        {
            --count;
        } while (menu_state.password.feedback_buffer[count] == 32);
    }
    menu_state.password.feedback_buffer[count + 1] = 0;
    menu_state.password.feedback = menu_state.password.feedback_buffer;
    return 1;
}

uint32 MenuState::EditPassword(uint32 requested_mode)
{
    uint8 mode = requested_mode;
    uint32 success_until = 0;
    uint32 error_until = 0;
    uint32 pressed;
    uint32 buttons;
    const SoundSample *sound;
    const SoundBankStorage *sound_table;
    uint32 saved_frame;
    uint32 cursor;
    uint32 clock;

    uint32 encoded;
    uint32 power;
    uint32 rotation;
    uint32 checksum;
    uint32 fields[7];
    uint32 decoded[7];
    uint32 widths[7] = {5, 5, 5, 2, 1, 3, 9};
    uint32 field;
    uint32 bit;
    uint32 aligned;
    sint32 digit;
    sint32 voice;
    sint32 checked;
    sint32 idx;
    CharacterSettings settings;
    FUNCTION_MARKER(0x8006BBE8, "SLUS_005.78");
    if (mode != 0)
    {
        ScreenRenderState::FadeMenuImage(0);
        menu_state.fade_frames = 15;
        menu_state.password.cursor = 0;
        if (menu_state.password.reset_pending)
        {
            memset(menu_state.password.characters, 0, sizeof(menu_state.password.characters));
            menu_state.password.reset_pending = false;
        }
    }
    else
        MenuState::GeneratePassword();
    clock = frame_clock.milliseconds;
    menu_state.pacer.fraction = 30;
    menu_state.last_frame = clock;
    for (;;)
    {
        pressed = 0;
        if (success_until == 0 && error_until == 0)
        {
            buttons = GameplayInput::ReadGamepadButtons();
            {
                uint32 old_buttons = menu_state.prev_buttons;
                uint32 repeat;
                if (buttons & 0x1000)
                    ++menu_state.up_hold_frames;
                else
                    menu_state.up_hold_frames = 0;
                if (buttons & 0x4000)
                    ++menu_state.down_hold_frames;
                else
                    menu_state.down_hold_frames = 0;
                pressed = buttons & ~old_buttons;
                menu_state.prev_buttons = buttons;
                repeat = menu_state.up_hold_frames;
                if ((sint32)repeat >= 11 && (repeat & 3) == 0)
                    pressed |= 0x1000;
                repeat = menu_state.down_hold_frames;
                if ((sint32)repeat >= 11 && (repeat & 3) == 0)
                    pressed |= 0x4000;
            }
        }
        if (mode != 0)
            MenuState::LoadMenuImage(256);
        MenuState::DrawPasswordScr((sint8)mode, error_until != 0, success_until != 0);
        if (pressed & 0x10)
        {
            sound = 0;
            saved_frame = frame_clock.render_tick;
            sound_table = sound_resources.common_table;
            if (sound_table != 0 && sound_table->header.count > 25)
                sound = sound_table->samples + 25;
            frame_clock.render_tick = frame_clock.menu_tick;
            voice = sound_effects.SelectVoice(0xFFFFFFFF, sound, 4);
            if (voice >= 0)
                SoundEffects::PlaySoundEffect(sound, voice, 0, 0, 0, sound_settings.effects_volume, 0);
            frame_clock.render_tick = saved_frame;
            if (mode != 0)
            {
                ScreenRenderState::FadeMenuImage(0);
                menu_state.fade_frames = 15;
            }
            return 0;
        }
        if (pressed & 0x5000)
        {
            if (mode != 0)
            {
                sound = 0;
                saved_frame = frame_clock.render_tick;
                digit = menu_state.password.characters[menu_state.password.cursor];
                sound_table = sound_resources.common_table;
                if (sound_table != 0 && sound_table->header.count > 24)
                    sound = sound_table->samples + 24;
                frame_clock.render_tick = frame_clock.menu_tick;
                voice = sound_effects.SelectVoice(0xFFFFFFFF, sound, 4);
                if (voice >= 0)
                    SoundEffects::PlaySoundEffect(sound, voice, 0, 0, 0, sound_settings.effects_volume, 0);
                frame_clock.render_tick = saved_frame;
                if ((pressed ^ 0x5000) & 0x3000)
                {
                    --digit;
                    if (digit < 0)
                        digit = 25;
                }
                else
                {
                    ++digit;
                    if (digit == 26)
                        digit = 0;
                }
                menu_state.password.characters[menu_state.password.cursor] = digit;
            }
            goto finish_i;
        }
        if (pressed & 0xA000)
        {
            if (mode != 0)
            {
                sound = 0;
                saved_frame = frame_clock.render_tick;
                sound_table = sound_resources.common_table;
                if (sound_table != 0 && sound_table->header.count > 24)
                    sound = sound_table->samples + 24;
                frame_clock.render_tick = frame_clock.menu_tick;
                voice = sound_effects.SelectVoice(0xFFFFFFFF, sound, 4);
                if (voice >= 0)
                    SoundEffects::PlaySoundEffect(sound, voice, 0, 0, 0, sound_settings.effects_volume, 0);
                frame_clock.render_tick = saved_frame;
                if (pressed & 0x2000)
                {
                    cursor = (menu_state.password.cursor + 1) & 255;
                    menu_state.password.cursor = cursor;
                    if ((sint8)cursor == 8)
                        menu_state.password.cursor = 0;
                }
                else
                {
                    cursor = (menu_state.password.cursor - 1) & 255;
                    menu_state.password.cursor = cursor;
                    if (cursor & 0x80)
                        menu_state.password.cursor = 7;
                }
            }
            goto finish_i;
        }
        if ((pressed & 0x40) == 0)
            goto finish_i;
        sound = 0;
        saved_frame = frame_clock.render_tick;
        sound_table = sound_resources.common_table;
        if (sound_table != 0 && sound_table->header.count > 25)
            sound = sound_table->samples + 25;
        frame_clock.render_tick = frame_clock.menu_tick;
        voice = sound_effects.SelectVoice(0xFFFFFFFF, sound, 4);
        if (voice >= 0)
            SoundEffects::PlaySoundEffect(sound, voice, 0, 0, 0, sound_settings.effects_volume, 0);
        frame_clock.render_tick = saved_frame;
        if (mode == 0)
            goto finish_i;
        checked = MenuState::ApplyCheatPassword();
        if (checked != 0)
        {
            if (checked < 0)
                error_until = menu_state.last_frame + 1000;
            else
            {
                clock = menu_state.last_frame;
                success_until = clock + 1000;
                if ((game_flow.mode_flags & 0x200) == 0)
                    goto expire_messages;
                MenuState::DrawPasswordScr(1, 0, 1);
                SequencePlayback::StopCurSequence();
                if (sequence_playback.cur_handle != 0)
                    SequencePlayback::CloseSequence(sequence_playback.cur_handle);
                sequence_playback.cur_handle = 0;
                resource_load_cursor.container_asset = 47;
                {
                    uint32 atlas_bytes = resource_archive.Size(147);
                    uint32 atlas_capacity;
                    uint8 *atlas;
                    atlas_capacity = (atlas_bytes + 2047u) & ~2047u;
                    atlas = (uint8 *)malloc(atlas_capacity);
                    resource_archive.ReadAsset(147, atlas, 1);
                    uint32 image_idx = 0;

                    if (image_idx == 0)
                    {
                        uint32 img1_offset = resource_archive.Offset(252);
                        uint32 img1_extent = (uint32)OriginalDefaults::tex_img_rects[8].w * (uint32)OriginalDefaults::tex_img_rects[8].h * 2;
                        img1_extent += img1_offset;
                    }
                    ScreenRenderState::UploadTexImage(atlas, image_idx, 1);
                    free(atlas);
                }
                CameraProjection::ResetSceneRendering();
                ResourceGroup::LoadCommonGameResources();
                MenuState::ReloadMenuArchive();
                MenuState::LoadMenuResources();
            }
            goto finish_i;
        }
        encoded = 0;
        power = 1;
        for (idx = 7; idx >= 0; --idx)
        {
            encoded += menu_state.password.characters[idx] * power;
            power <<= 4;
        }
        rotation = encoded >> 30;
        fields[0] = (encoded >> rotation) & 31;
        fields[6] = (((encoded >> (rotation + 21)) & (((uint32)1 << (9 - rotation)) - 1)) << rotation) | (encoded & (((uint32)1 << rotation) - 1));
        fields[1] = (encoded >> (rotation + 5)) & 31;
        fields[2] = (encoded >> (rotation + 10)) & 31;
        fields[3] = (encoded >> (rotation + 15)) & 3;
        fields[4] = (encoded >> (rotation + 17)) & 1;
        fields[5] = (encoded >> (rotation + 18)) & 7;
        checksum = fields[0] + fields[1] + fields[2] + fields[3] + fields[4] + fields[5] + rotation;
        for (field = 0; field < 7; ++field)
        {
            decoded[field] = 0;
            for (bit = 0; bit < widths[field]; ++bit)
            {
                if (fields[field] & ((uint32)1 << bit))
                {
                    aligned = bit;
                    while (aligned < rotation + 1)
                        aligned += widths[field];
                    decoded[field] |= (uint32)1 << ((aligned - (rotation + 1)) % widths[field]);
                }
            }
        }
        settings.flags = decoded[4];
        settings.health_limit = decoded[2];
        settings.counters[CharacterSettings::CNT_LIVES] = decoded[1];
        settings.action_code = decoded[3];
        settings.health_pickups = decoded[5];
        if (decoded[6] != 0 && decoded[6] == checksum && (sint32)decoded[0] >= 0 && GameFlowState::MapLevelProgression(78, 1) >= (sint32)decoded[0] && decoded[1] < 32 && decoded[2] < 32 && decoded[3] < 4 && decoded[4] < 2 && decoded[5] < 8)
        {
            ++decoded[1];
            ++decoded[2];
            settings.counters[CharacterSettings::CNT_LIVES] = decoded[1];
            settings.health_limit = decoded[2];
            CharacterController::ApplyCharacterSettings(CharacterController::FindPlayerObject(0), &settings, decoded[0]);
            game_flow.requested_level = 0;
            game_flow.progress.sel_stage = decoded[0];
            CharacterModelRecord::ReloadBaseCharacterModels((settings.flags & 1) != 0 ? 2 : 1);
            return 1;
        }
        if (checksum != 0 && checksum == decoded[0] + decoded[1] + decoded[2] + decoded[3] && decoded[0] == 511 - (decoded[1] + decoded[2] + decoded[3]) && decoded[1] == 31 && decoded[2] == 31 && decoded[3] == 3)
        {
            settings.counters[CharacterSettings::CNT_LIVES] = decoded[3];
            settings.health_limit = 3;
            settings.action_code = 0;
            CharacterController::ApplyCharacterSettings(CharacterController::FindPlayerObject(0), &settings, 0);
        }
        else
            error_until = menu_state.last_frame + 3000;
    finish_i:
        clock = menu_state.last_frame;
    expire_messages:
        if (error_until < clock)
            error_until = 0;
        if (success_until < clock)
            success_until = 0;
        frame_clock.menu_tick = frame_clock.menu_tick + 1;
    }
}

void MenuState::DrawOptionsMenu(sint32 selection, sint32 music_volume, sint32 sound_volume)
{
    const MenuMaterialResources *resources;
    uint32 pos;
    uint32 grp_idx;
    const TextureDescriptor *grps;
    sint32 tex_count;
    uint32 icon;
    uint32 button;
    const TextureDescriptor *table;
    uint32 tex_idx;
    uint32 idx;
    uint32 saved_frame;
    uint32 fade;
    uint32 result;
    FUNCTION_MARKER(0x8006C7B4, "SLUS_005.78");
    DrawSync(0);
    VSync(0);
    RenderCommands::BeginDraw();
    RenderCommands::ClearSceneOrderingTable();
    ScreenRenderState::RestoreMenuImage(0);
    if (selection == 2)
        pos = 0x7A007E;
    else
        pos = selection == 0 ? 0x650036 : 0x6400DC;
    /* The reviewed options caller supplies selection 0..2 */
    if (selection == 1)
        grp_idx = 8;
    else if (selection >= 2)
    {
        if (selection == 2)
            grp_idx = 7;
    }
    else if (selection == 0)
        grp_idx = 6;
    grps = menu_state.resources->texs;
    MaterialCatalog::DrawTexturedQuad(pos, 0, grps + grp_idx, 0);
    grps = menu_state.resources->texs;
    if (grps != 0)
    {
        pos = ((uint32)(67 - ((sint32)(50 * (uint32)sound_volume) >> 12)) << 16) | 0x44;
        MaterialCatalog::DrawTexturedQuad(pos, 0, grps, 0);
    }
    if (grps != 0)
    {
        pos = ((uint32)(67 - ((sint32)(50 * (uint32)music_volume) >> 12)) << 16) | 0xDC;
        MaterialCatalog::DrawTexturedQuad(pos, 0, grps, 0);
    }
    for (icon = 0; icon < 3; ++icon)
    {
        idx = icon == 0 ? 1 : icon == 1 ? 0 : 2;
        button = (*control_bindings.active_scheme)[idx];
        switch (button)
        {
            case 128:
                tex_idx = 9;
                break;
            case 16:
                tex_idx = 11;
                break;
            case 64:
                tex_idx = 8;
                break;
            default:
                tex_idx = 10;
                break;
        }
        table = interface_resources->texs;
        if (table + tex_idx != 0)
        {
            button = (*control_bindings.active_scheme)[idx];
            switch (button)
            {
                case 128:
                    tex_idx = 9;
                    break;
                case 16:
                    tex_idx = 11;
                    break;
                case 64:
                    tex_idx = 8;
                    break;
                default:
                    tex_idx = 10;
                    break;
            }
            table = interface_resources->texs;
            pos = icon == 0 ? 0x90007D : icon == 1 ? 0xAA007D : 0xC5007D;
            MaterialCatalog::DrawTexturedQuad(pos, 0, table + tex_idx, 0);
        }
    }
    MaterialRegistry::UpdateResourceMaterialAnimations();
    resources = menu_state.resources;
    if (resources != 0)
    {
        saved_frame = frame_clock.render_tick;
        tex_count = resources->header.tex_count;
        frame_clock.render_tick = frame_clock.menu_tick;
        if (tex_count > 0)
        {
            idx = 0;
            do
            {
                MaterialAnimationList::UpdateMaterialAnimationList(menu_state.material_grps[idx].animation_entries);
                ++idx;
            } while ((sint32)idx < (sint32)menu_state.resources->header.tex_count);
        }
        frame_clock.render_tick = saved_frame;
    }
    fade = menu_state.fade_frames;
    if (fade != 0)
    {
        --fade;
        menu_state.fade_frames = fade;
        if (fade != 0)
            ScreenRenderState::DrawOverlayRect(0, 0xF00140, (sint32)(fade << 12) / 15, 0xFFFFFF);
    }
    RenderCommands::EndDraw();
    result = menu_state.pacer.Wait(menu_state.last_frame);
    menu_state.last_frame = result;
}

uint32 MenuState::RunOptionsMenu(void)
{
    sint32 sound_volume;
    sint32 music_volume;
    sint32 selection = 0;
    sint32 preview_voice = -1;
    sint32 voice;
    sint32 dir;
    const SoundSample *preview_sound = 0;
    const SoundSample *sound;
    const SoundBankStorage *sound_table;
    uint32 saved_frame;
    uint32 old_buttons;
    uint32 buttons;
    uint32 pressed;
    uint32 clock;
    FUNCTION_MARKER(0x8006CE38, "SLUS_005.78");
    ScreenRenderState::FadeMenuImage(0);
    sound_table = sound_resources.common_table;
    sound_volume = sound_settings.effects_volume;
    if (sound_table != 0 && sound_table->header.count > 0)
        preview_sound = sound_table->samples;
    music_volume = sound_settings.music_volume;
    clock = frame_clock.milliseconds;
    menu_state.pacer.fraction = 30;
    menu_state.fade_frames = 15;
    menu_state.last_frame = clock;
    for (;;)
    {
        MenuState::LoadMenuImage(257);
        MenuState::DrawOptionsMenu(selection, music_volume, sound_volume);
        old_buttons = ~menu_state.prev_buttons;
        buttons = GameplayInput::ReadGamepadButtons();
        menu_state.prev_buttons = buttons;
        pressed = buttons & old_buttons;
        if (pressed & 0x10)
        {
            sound = 0;
            saved_frame = frame_clock.render_tick;
            sound_table = sound_resources.common_table;
            if (sound_table != 0 && sound_table->header.count > 25)
                sound = sound_table->samples + 25;
            frame_clock.render_tick = frame_clock.menu_tick;
            voice = sound_effects.SelectVoice(0xFFFFFFFF, sound, 4);
            if (voice >= 0)
                SoundEffects::PlaySoundEffect(sound, voice, 0, 0, 0, sound_settings.effects_volume, 0);
            frame_clock.render_tick = saved_frame;
            break;
        }
        if (pressed & 0xA000)
        {
            sound = 0;
            saved_frame = frame_clock.render_tick;
            sound_table = sound_resources.common_table;
            if (sound_table != 0 && sound_table->header.count > 24)
                sound = sound_table->samples + 24;
            frame_clock.render_tick = frame_clock.menu_tick;
            voice = sound_effects.SelectVoice(0xFFFFFFFF, sound, 4);
            if (voice >= 0)
                SoundEffects::PlaySoundEffect(sound, voice, 0, 0, 0, sound_settings.effects_volume, 0);
            frame_clock.render_tick = saved_frame;
            if (pressed & 0x3000)
            {
                --selection;
                if (selection < 0)
                    selection = 2;
            }
            else
            {
                ++selection;
                if (selection == 3)
                    selection = 0;
            }
        }
        else if (pressed & 0x5000)
        {
            dir = (pressed & 0x1000) ? -1 : 1;
            if (selection == 1)
            {
                music_volume = sound_settings.AdjustMusic(-dir);
                menu_state.prev_buttons = menu_state.prev_buttons & 0xFFFFAFFF;
            }
            else if (selection == 2)
            {
                sound = 0;
                saved_frame = frame_clock.render_tick;
                sound_table = sound_resources.common_table;
                if (sound_table != 0 && sound_table->header.count > 25)
                    sound = sound_table->samples + 25;
                frame_clock.render_tick = frame_clock.menu_tick;
                voice = sound_effects.SelectVoice(0xFFFFFFFF, sound, 4);
                if (voice >= 0)
                    SoundEffects::PlaySoundEffect(sound, voice, 0, 0, 0, sound_settings.effects_volume, 0);
                frame_clock.render_tick = saved_frame;
                {
                    uint32 cursor;
                    if (pressed & 0x1000)
                    {
                        cursor = (control_bindings.sel_idx - 1) & 255;
                        control_bindings.sel_idx = cursor;
                        if (cursor & 0x80)
                            control_bindings.sel_idx = 23;
                    }
                    else
                    {
                        cursor = (control_bindings.sel_idx + 1) & 255;
                        control_bindings.sel_idx = cursor;
                        if (cursor >= 24)
                            control_bindings.sel_idx = 0;
                    }
                    control_bindings.active_scheme = &OriginalDefaults::control_schemes[control_bindings.sel_idx];
                }
            }
            else if (selection == 0)
            {
                saved_frame = frame_clock.render_tick;
                menu_state.prev_buttons = buttons & 0xFFFFAFFF;
                frame_clock.render_tick = frame_clock.menu_tick;
                sound_volume = sound_settings.AdjustEffects(-dir);
                if ((sound_effects.IsPlaying(preview_sound, preview_voice) << 24) != 0)
                    SoundEffects::SetSoundVoiceVolume(preview_voice, (sound_volume >> 4));
                else
                {
                    preview_voice = sound_effects.SelectVoice(0xFFFFFFFF, preview_sound, 4);
                    if (preview_voice != -1)
                        SoundEffects::PlaySoundEffect(preview_sound, preview_voice, 0, 0, 20010, sound_volume, 0);
                }
                frame_clock.render_tick = saved_frame;
            }
        }
        frame_clock.menu_tick = frame_clock.menu_tick + 1;
    }
    ScreenRenderState::FadeMenuImage(0);
    menu_state.fade_frames = 15;
    return 0;
}

uint32 MenuState::RunPasswordMenu(void)
{
    FUNCTION_MARKER(0x8006D2EC, "SLUS_005.78");
    return MenuState::EditPassword(0);
}

void MenuState::DrawQuitConfirmation(uint32 selection, uint32 mode)
{
    uint32 *tail_tag;

    FUNCTION_MARKER(0x8006D30C, "SLUS_005.78");
    uint32 buffer, pos, extent, pointer, result;
    sint32 fraction;
    buffer = render_commands.other_packet_base;
    render_commands.overlay_head = 0;
    render_commands.packet_offset = buffer;
    ScreenRenderState::DrawOverlayRect(0, 0x00F00140, 4096, 0xFFFFFF);
    if (mode)
    {
        UiText::DrawText(0x007800A0, UiText::quit_game, 1, 0);
        UiText::DrawText(0x009600A0, UiText::quit_choices, 1, 0);
    }
    else
        UiText::DrawText(0x007800A0, UiText::continue_quit, 1, 0);
    if (!selection)
    {
        pos = mode ? 0x00930067 : 0x007500C0;
        extent = mode ? 0x0014002E : 0x0014003E;
    }
    else
    {
        pos = mode ? 0x009300AB : 0x00750035;
        extent = mode ? 0x00140022 : 0x00140073;
    }
    {
        uint32 phase = menu_state.highlight_phase;
        uint32 step = menu_state.highlight_step;
        phase += step;
        menu_state.highlight_phase = phase;
        if ((sint32)phase >= 2048)
        {
            menu_state.highlight_phase = 2048;
            menu_state.highlight_step = -step;
        }
        else if ((sint32)phase < -2047)
        {
            menu_state.highlight_phase = -2048;
            menu_state.highlight_step = -step;
        }
        fraction = menu_state.highlight_phase;
    }
    if (fraction > 0)
        ScreenRenderState::DrawBrightenRect(pos, extent, fraction, 0x00808080);
    else if (fraction < 0)
        ScreenRenderState::DrawOverlayRect(pos, extent, (-(uint32)fraction), 0x00808080);
    if (!selection)
        UiText::DrawText(0x00CA005A, UiText::quit, 0, 0);
    else
        UiText::DrawText(0x00CA005A, UiText::continue_text, 0, 0);
    MaterialCatalog::DrawTexturedQuad(0x00C80037, 0, interface_resources->texs + 8, 0);
    pointer = render_commands.overlay_tail;
    tail_tag = (uint32 *)psx_addr(pointer, sizeof(uint32));
    *tail_tag |= 0xFFFFFF;
    RenderCommands::SubmitOverlayFrame(render_commands.other_packet_base, 0);
    result = menu_state.pacer.Wait(menu_state.last_frame);
    menu_state.last_frame = result;
}

uint32 MenuState::RunQuitConfirmation(uint32 mode)
{
    FUNCTION_MARKER(0x8006D678, "SLUS_005.78");
    uint32 cur, prev, pressed, tick, val;
    const SoundBankStorage *table;
    const SoundSample *sound;
    sint32 selection, countdown, voice;
    cur = GameplayInput::ReadGamepadButtons();
    val = frame_clock.milliseconds;
    countdown = 0;
    selection = 1;
    menu_state.prev_buttons = cur;
    menu_state.pacer.fraction = 30;
    menu_state.last_frame = val;
    for (;;)
    {
        MenuState::DrawQuitConfirmation(selection, mode);
        pressed = 0;
        if (!countdown)
        {
            prev = menu_state.prev_buttons;
            cur = GameplayInput::ReadGamepadButtons();
            menu_state.prev_buttons = cur;
            pressed = cur & ~prev;
        }
        if (pressed & 0xA000)
        {
            sound = 0;
            table = sound_resources.common_table;
            tick = frame_clock.render_tick;
            if (table && table->header.count > 24)
                sound = table->samples + 24;
            frame_clock.render_tick = frame_clock.menu_tick;
            voice = sound_effects.SelectVoice(0xFFFFFFFF, sound, 4);
            if (voice >= 0)
                SoundEffects::PlaySoundEffect(sound, voice, 0, 0, 0, sound_settings.effects_volume, 0);
            frame_clock.render_tick = tick;
            if (pressed & 0x3000)
            {
                --selection;
                if (selection < 0)
                    selection = 1;
            }
            else
            {
                ++selection;
                if (selection == 2)
                    selection = 0;
            }
        }
        else if (pressed & 0x40)
        {
            if (selection)
                return 0;
            sound = 0;
            table = sound_resources.common_table;
            tick = frame_clock.render_tick;
            if (table && table->header.count > 25)
                sound = table->samples + 25;
            frame_clock.render_tick = frame_clock.menu_tick;
            voice = sound_effects.SelectVoice(0xFFFFFFFF, sound, 4);
            if (voice >= 0)
                SoundEffects::PlaySoundEffect(sound, voice, 0, 0, 0, sound_settings.effects_volume, 0);
            frame_clock.render_tick = tick;
            if (!mode && !(MenuState::RunQuitConfirmation(1) & 0xFF))
                return 0;
            if (mode != 1)
                return 1;
            countdown = 10;
        }
        if (countdown)
        {
            --countdown;
            if (countdown <= 0)
                return 1;
        }
        frame_clock.menu_tick = frame_clock.menu_tick + 1;
    }
}

void MenuState::DrawPauseMenu(void)
{
    uint32 *tail_tag;

    FUNCTION_MARKER(0x8006D8EC, "SLUS_005.78");
    uint32 buffer, pointer, tick, cur_tick, count, idx, elapsed, ot0_offset, result;
    const MenuMaterialResources *objects;
    uint32 *entries;
    buffer = render_commands.other_packet_base;
    render_commands.overlay_head = 0;
    render_commands.packet_offset = buffer;
    render_commands.packet_limit = buffer + 41600;
    ScreenRenderState::DrawOverlayRect(0, 0x00F00140, 1638, 0xFFFFFF);
    HudState::DrawHud(CharacterController::FindPlayerObject(0));
    UiText::DrawText(0x007800A0, UiText::paused, 1, 0);
    if (!level_resource.level_id)
        ScreenRenderState::DrawOverlayRect(0x00C30000, 0x001D0140, 4096, 0xFFFFFF);
    UiText::DrawText(0x00CA00A0, UiText::select_quit, 1, 0);
    pointer = render_commands.overlay_tail;
    tail_tag = (uint32 *)psx_addr(pointer, sizeof(uint32));
    *tail_tag |= 0xFFFFFF;
    MaterialRegistry::UpdateResourceMaterialAnimations();
    objects = menu_state.resources;
    if (objects)
    {
        tick = frame_clock.render_tick;
        cur_tick = frame_clock.menu_tick;
        count = objects->header.tex_count;
        frame_clock.render_tick = cur_tick;
        if ((sint32)count > 0)
        {
            count = 0;
            idx = 0;
            do
            {
                entries = menu_state.material_grps[idx / 8].animation_entries;
                idx += 8;
                MaterialAnimationList::UpdateMaterialAnimationList(entries);
                objects = menu_state.resources;
                elapsed = objects->header.tex_count;
                ++count;
            } while ((sint32)count < (sint32)elapsed);
        }
        frame_clock.render_tick = tick;
    }
    elapsed = menu_state.fade_frames;
    if (elapsed)
    {
        --elapsed;
        menu_state.fade_frames = elapsed;
        if (elapsed)
            ScreenRenderState::DrawOverlayRect(0, 0x00F00140, (sint32)(elapsed << 12) / 15, 0xFFFFFF);
    }
    ot0_offset = render_commands.submitted_chain_offset;
    buffer = render_commands.other_packet_base;
    RenderCommands::SubmitOverlayFrame(ot0_offset, buffer);
    result = menu_state.pacer.Wait(menu_state.last_frame);
    menu_state.last_frame = result;
}

void MenuState::RunPauseMenu(uint32 level)
{
    uint32 *tail_tag;

    FUNCTION_MARKER(0x8006DAF4, "SLUS_005.78");
    uint32 flags, handle, tick, val, pointer, prev, cur, pressed;
    const SoundBankStorage *table;
    const SoundSample *sound;
    sint32 voice;
    flags = game_flow.transition_flags;
    handle = sequence_playback.cur_handle;
    game_flow.transition_flags = flags | 0x10;
    SequencePlayback::PauseSequence(handle);
    SoundEffects::StopSceneSounds();
    AnimatedObject::PauseCharacterSound();
    sound = 0;
    table = sound_resources.common_table;
    tick = frame_clock.render_tick;
    if (table && table->header.count > 23)
        sound = table->samples + 23;
    frame_clock.render_tick = frame_clock.menu_tick;
    voice = sound_effects.SelectVoice(0xFFFFFFFF, sound, 4);
    if (voice >= 0)
        SoundEffects::PlaySoundEffect(sound, voice, 0, 0, 0, sound_settings.effects_volume, 0);
    val = level_resource.level_id;
    frame_clock.render_tick = tick;
    if (!val)
    {
        pointer = hud.prompt_tail;
        if (pointer)
        {
            tail_tag = (uint32 *)psx_addr(pointer, sizeof(uint32));
            *tail_tag |= 0xFFFFFF;
        }
    }
    cur = GameplayInput::ReadGamepadButtons();
    val = frame_clock.milliseconds;
    menu_state.prev_buttons = cur;
    menu_state.pacer.fraction = 30;
    menu_state.last_frame = val;
    for (;;)
    {
        MenuState::DrawPauseMenu();
        prev = menu_state.prev_buttons;
        cur = GameplayInput::ReadGamepadButtons();
        menu_state.prev_buttons = cur;
        pressed = cur & ~prev;
        if (pressed & 0x800)
            break;
        if (pressed & 0x100)
        {
            sound = 0;
            table = sound_resources.common_table;
            tick = frame_clock.render_tick;
            if (table && table->header.count > 25)
                sound = table->samples + 25;
            frame_clock.render_tick = frame_clock.menu_tick;
            voice = sound_effects.SelectVoice(0xFFFFFFFF, sound, 4);
            if (voice >= 0)
                SoundEffects::PlaySoundEffect(sound, voice, 0, 0, 0, sound_settings.effects_volume, 0);
            frame_clock.render_tick = tick;
            if (MenuState::RunQuitConfirmation(0) & 0xFF)
            {
                SceneLoader::WaitSceneResourceLoad();
                goto finish;
            }
            break;
        }
        MaterialRegistry::UpdateResourceMaterialAnimations();
    }
    table = sound_resources.common_table;
    tick = frame_clock.render_tick;
    sound = 0;
    gameplay_input.release_mask = 64;
    if (table && table->header.count > 23)
        sound = table->samples + 23;
    frame_clock.render_tick = frame_clock.menu_tick;
    voice = sound_effects.SelectVoice(0xFFFFFFFF, sound, 4);
    if (voice >= 0)
        SoundEffects::PlaySoundEffect(sound, voice, 0, 0, 0, sound_settings.effects_volume, 0);
    handle = sequence_playback.cur_handle;
    frame_clock.render_tick = tick;
    SequencePlayback::ResumeSequence(handle);
    AnimatedObject::ResumeCharacterSound();
finish:
    game_flow.transition_flags = game_flow.transition_flags & 0xFFFFFFEF;
    sound_effects.Stop(0);
}

void MenuState::DrawMainMenu(uint32 selection)
{
    const MenuMaterialResources *resources;
    uint32 pos;
    const TextureDescriptor *grp;
    uint32 saved_frame;
    uint32 idx;
    uint32 fade;
    uint32 result;
    FUNCTION_MARKER(0x8006E6FC, "SLUS_005.78");
    RenderCommands::BeginDraw();
    RenderCommands::ClearSceneOrderingTable();
    ScreenRenderState::RestoreMenuImage(0);
    if (selection == 0)
    {
        pos = 0x640064;
        grp = menu_state.resources->texs + 2;
    }
    else if (selection == 1)
    {
        pos = 0x7D0073;
        grp = menu_state.resources->texs + 3;
    }
    else if (selection == 2)
    {
        pos = 0x960063;
        grp = menu_state.resources->texs + 4;
    }
    else
    {
        pos = 0xB00070;
        grp = menu_state.resources->texs + selection + 2;
    }
    MaterialCatalog::DrawTexturedQuad(pos, 0, grp, 0);
    MaterialRegistry::UpdateResourceMaterialAnimations();
    resources = menu_state.resources;
    if (resources != 0)
    {
        saved_frame = frame_clock.render_tick;
        frame_clock.render_tick = frame_clock.menu_tick;
        if ((sint32)resources->header.tex_count > 0)
        {
            idx = 0;
            do
            {
                MaterialAnimationList::UpdateMaterialAnimationList(menu_state.material_grps[idx].animation_entries);
                ++idx;
            } while ((sint32)idx < (sint32)menu_state.resources->header.tex_count);
        }
        frame_clock.render_tick = saved_frame;
    }
    fade = menu_state.fade_frames;
    if (fade != 0)
    {
        --fade;
        menu_state.fade_frames = fade;
        if (fade != 0)
            ScreenRenderState::DrawOverlayRect(0, 0xF00140, (sint32)(fade << 12) / 15, 0xFFFFFF);
    }
    RenderCommands::EndDraw();
    result = menu_state.pacer.Wait(menu_state.last_frame);
    menu_state.last_frame = result;
}

sint32 MenuState::RunMainMenu(void)
{
    uint32 frames = 0;
    sint32 selection = 0;
    sint32 result = 1;
    uint32 deadline;
    uint32 old_buttons;
    uint32 pressed;
    const SoundSample *sound;
    const SoundBankStorage *sound_table;
    uint32 saved_frame;
    sint32 voice;
    FUNCTION_MARKER(0x8006E8D8, "SLUS_005.78");
    if (character_pad.loaded_model_mask != 3)
        CharacterModelRecord::ReloadBaseCharacterModels(3);
    pressed = GameplayInput::ReadGamepadButtons();
    deadline = frame_clock.milliseconds;
    menu_state.prev_buttons = pressed;
    menu_state.pacer.fraction = 30;
    menu_state.fade_frames = 15;
    menu_state.last_frame = deadline;
    deadline += 30000;
    for (;;)
    {
        MenuState::LoadMenuImage(255);
        MenuState::DrawMainMenu(selection);
        ++frames;
        pressed = 0;
        if ((sint32)frames >= 15)
        {
            old_buttons = ~menu_state.prev_buttons;
            menu_state.prev_buttons = GameplayInput::ReadGamepadButtons();
            pressed = menu_state.prev_buttons & old_buttons;
        }
        if (pressed & 0x5000)
        {
            sound = 0;
            saved_frame = frame_clock.render_tick;
            sound_table = sound_resources.common_table;
            if (sound_table != 0 && sound_table->header.count > 24)
                sound = sound_table->samples + 24;
            frame_clock.render_tick = frame_clock.menu_tick;
            voice = sound_effects.SelectVoice(0xFFFFFFFF, sound, 4);
            if (voice >= 0)
                SoundEffects::PlaySoundEffect(sound, voice, 0, 0, 0, sound_settings.effects_volume, 0);
            frame_clock.render_tick = saved_frame;
            if (pressed & 0x3000)
            {
                --selection;
                if (selection < 0)
                    selection = 3;
            }
            else
            {
                ++selection;
                if (selection == 4)
                    selection = 0;
            }
        }
        else if (pressed & 0x40)
        {
            sound = 0;
            saved_frame = frame_clock.render_tick;
            sound_table = sound_resources.common_table;
            if (sound_table != 0 && sound_table->header.count > 25)
                sound = sound_table->samples + 25;
            frame_clock.render_tick = frame_clock.menu_tick;
            voice = sound_effects.SelectVoice(0xFFFFFFFF, sound, 4);
            if (voice >= 0)
                SoundEffects::PlaySoundEffect(sound, voice, 0, 0, 0, sound_settings.effects_volume, 0);
            frame_clock.render_tick = saved_frame;
            switch (selection)
            {
                case 0:
                    CharacterController::ResetNewGameCharacter();
                    game_flow.requested_level = 0;
                    goto finish;
                case 2:
                    if ((MenuState::EditPassword(1) << 24) != 0)
                        goto finish;
                    break;
                case 1:
                    MenuState::RunOptionsMenu();
                    break;
                default:
                    game_flow.attract_phase = 2;
                    result = 0;
                    goto finish;
            }
        }
        if (pressed != 0 || menu_state.prev_buttons != 0)
            deadline = menu_state.last_frame + 30000;
        if (menu_state.last_frame >= deadline)
            break;
        frame_clock.menu_tick = frame_clock.menu_tick + 1;
    }
    result = 0;
finish:
    ScreenRenderState::FadeMenuImage(0);
    sound_effects.Stop(0);
    return result;
}
