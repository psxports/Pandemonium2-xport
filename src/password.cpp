#include "game.h"

struct CheatRecord
{
    uint32 password[8];
    uint32 message[21];
};

static const CheatRecord cheats[12] = {
    {{0x0000E443, 0x0000E50B, 0x0000DF2F, 0x0000E69B, 0x0000E5D3, 0x0000E5D3, 0x0000E50B, 0x0000DF93},
     {0x0000DE03, 0x0000E123, 0x0000DFF7, 0x0000E24F, 0x0000E56F, 0x0000F0C3, 0x0000DF93, 0x0000F37F, 0x0000DD3B, 0x0000E123, 0x0000DECB, 0x0000DFF7, 0x0000F37F, 0x0000E123, 0x0000DD3B, 0x0000DF93, 0x0000DF2F, 0x0000E50B, 0x0000DFF7, 0x0000F31B, 0x00000000}},
    {{0x0000DF93, 0x0000E2B3, 0x0000E69B, 0x0000DF2F, 0x0000E637, 0x0000E123, 0x0000DFF7, 0x0000E56F},
     {0x0000E637, 0x0000E123, 0x0000E69B, 0x0000DFF7, 0x0000E56F, 0x0000E50B, 0x0000DFF7, 0x0000F37F, 0x0000DFF7, 0x0000DECB, 0x0000E187, 0x0000F31B, 0x0000F37F, 0x0000F37F, 0x0000F37F, 0x0000F37F, 0x0000F37F, 0x0000F37F, 0x0000F37F, 0x0000F37F, 0x00000000}},
    {{0x0000E3DF, 0x0000E123, 0x0000DFF7, 0x0000E1EB, 0x0000E123, 0x0000E187, 0x0000E50B, 0x0000DF93},
     {0x0000DD3B, 0x0000E123, 0x0000DECB, 0x0000F37F, 0x0000E24F, 0x0000E123, 0x0000E123, 0x0000E2B3, 0x0000F37F, 0x0000DF93, 0x0000E123, 0x0000F37F, 0x0000E3DF, 0x0000E50B, 0x0000E69B, 0x0000E24F, 0x0000DF2F, 0x0000E3DF, 0x0000DD3B, 0x0000F31B, 0x00000000}},
    {{0x0000E37B, 0x0000E1EB, 0x0000E1EB, 0x0000E123, 0x0000DFF7, 0x0000DF2F, 0x0000E69B, 0x0000E24F},
     {0x0000E24F, 0x0000E37B, 0x0000E4A7, 0x0000E50B, 0x0000F37F, 0x0000E123, 0x0000E4A7, 0x0000F37F, 0x0000DF2F, 0x0000E3DF, 0x0000E50B, 0x0000F37F, 0x0000E0BF, 0x0000E69B, 0x0000DFF7, 0x0000DF2F, 0x0000DD3B, 0x0000F31B, 0x0000F37F, 0x0000F37F, 0x00000000}},
    {{0x0000E1EB, 0x0000E69B, 0x0000E2B3, 0x0000E1EB, 0x0000DD3B, 0x0000E56F, 0x0000E69B, 0x0000DD3B},
     {0x0000E0BF, 0x0000E50B, 0x0000DFF7, 0x0000E1EB, 0x0000E69B, 0x0000E187, 0x0000E50B, 0x0000E187, 0x0000DF2F, 0x0000F37F, 0x0000DE03, 0x0000E50B, 0x0000E69B, 0x0000E0BF, 0x0000E123, 0x0000E187, 0x0000F31B, 0x0000F37F, 0x0000F37F, 0x0000F37F, 0x00000000}},
    {{0x0000E187, 0x0000E50B, 0x0000DE67, 0x0000E50B, 0x0000DFF7, 0x0000E56F, 0x0000E37B, 0x0000E50B},
     {0x0000E5D3, 0x0000E69B, 0x0000E187, 0x0000F0C3, 0x0000DF2F, 0x0000F37F, 0x0000DF2F, 0x0000E123, 0x0000DECB, 0x0000E5D3, 0x0000E3DF, 0x0000F37F, 0x0000DF2F, 0x0000E3DF, 0x0000E37B, 0x0000DF93, 0x0000F31B, 0x0000F37F, 0x0000F37F, 0x0000F37F, 0x00000000}},
    {{0x0000E443, 0x0000E50B, 0x0000E187, 0x0000E50B, 0x0000DF2F, 0x0000E37B, 0x0000E5D3, 0x0000DF93},
     {0x0000E1EB, 0x0000DECB, 0x0000DF2F, 0x0000E69B, 0x0000E187, 0x0000DF2F, 0x0000F37F, 0x0000E1EB, 0x0000E69B, 0x0000E187, 0x0000E37B, 0x0000E69B, 0x0000F31B, 0x0000F37F, 0x0000F37F, 0x0000F37F, 0x0000F37F, 0x0000F37F, 0x0000F37F, 0x0000F37F, 0x00000000}},
    {{0x0000E443, 0x0000E123, 0x0000E187, 0x0000E69B, 0x0000E3DF, 0x0000DECB, 0x0000DFF7, 0x0000E24F},
     {0x0000DFF7, 0x0000E123, 0x0000E5D3, 0x0000E2B3, 0x0000F37F, 0x0000F0C3, 0x0000E187, 0x0000F0C3, 0x0000F37F, 0x0000DFF7, 0x0000E123, 0x0000E24F, 0x0000E24F, 0x0000F31B, 0x0000F37F, 0x0000F37F, 0x0000F37F, 0x0000F37F, 0x0000F37F, 0x0000F37F, 0x00000000}},
    {{0x0000E317, 0x0000DECB, 0x0000DF93, 0x0000DF2F, 0x0000E2B3, 0x0000E37B, 0x0000E56F, 0x0000E187},
     {0x0000E37B, 0x0000E1EB, 0x0000E1EB, 0x0000E123, 0x0000DFF7, 0x0000DF2F, 0x0000E69B, 0x0000E24F, 0x0000F37F, 0x0000E50B, 0x0000E187, 0x0000E50B, 0x0000E1EB, 0x0000E37B, 0x0000E50B, 0x0000DF93, 0x0000F31B, 0x0000F37F, 0x0000F37F, 0x0000F37F, 0x00000000}},
    {{0x0000E443, 0x0000E05B, 0x0000E37B, 0x0000DF93, 0x0000E4A7, 0x0000DECB, 0x0000E187, 0x0000DD9F},
     {0x0000E443, 0x0000E123, 0x0000E123, 0x0000E187, 0x0000F37F, 0x0000E05B, 0x0000DECB, 0x0000E50B, 0x0000E50B, 0x0000E187, 0x0000F31B, 0x0000F37F, 0x0000F37F, 0x0000F37F, 0x0000F37F, 0x0000F37F, 0x0000F37F, 0x0000F37F, 0x0000F37F, 0x0000F37F, 0x00000000}},
    {{0x0000E69B, 0x0000E69B, 0x0000E69B, 0x0000E69B, 0x0000E69B, 0x0000E69B, 0x0000E69B, 0x0000E69B},
     {0x0000E50B, 0x0000E187, 0x0000DF2F, 0x0000E50B, 0x0000DFF7, 0x0000F37F, 0x0000E0BF, 0x0000E69B, 0x0000DF93, 0x0000DF93, 0x0000DE03, 0x0000E123, 0x0000DFF7, 0x0000E56F, 0x0000F37F, 0x0000E4A7, 0x0000E37B, 0x0000DFF7, 0x0000DF93, 0x0000DF2F, 0x00000000}},
    {{0x0000E69B, 0x0000E5D3, 0x0000E37B, 0x0000E56F, 0x0000E56F, 0x0000DECB, 0x0000E56F, 0x0000E50B},
     {0x0000E637, 0x0000E37B, 0x0000DCD7, 0x0000DCD7, 0x0000E69B, 0x0000DFF7, 0x0000E50B, 0x0000F37F, 0x0000E24F, 0x0000E69B, 0x0000E187, 0x0000E56F, 0x0000F31B, 0x0000F37F, 0x0000F37F, 0x0000F37F, 0x0000F37F, 0x0000F37F, 0x0000F37F, 0x0000F37F, 0x00000000}}
};

uint32 DrawPassword(uint32 x, uint32 y, sint8 interactive)
{
    TextureDescriptor *texture;
    uint32 *entries;
    uint32 group;
    uint32 encoded;
    uint32 glyphs;
    uint32 position;
    uint32 index;
    uint32 source;
    uint32 words[2];
    sint32 amplitude;
    FUNCTION_MARKER(0x8006B0E0, "SLUS_005.78");
    entries = resource_groups[3].animation_entries;
    encoded = (uint8)entries[-1];
    group = resource_groups[3].material_offset;
    encoded = entries[encoded - 1];
    glyphs = material_registry.records[(encoded >> 1) & 0x1FF].data_offset;
    if (interactive == 0)
        p2_func_800652e4((x + 2) | ((y - 1) << 16), 0x140078, 4096, 0x800080);
    x += 4;
    y += 2;
    position = x + (y << 16);
    for (index = 0; index < 8; ++index)
    {
        source = glyphs + 8 * menu_state.password.characters[index];
        words[0] = r_u32(source + 264);
        words[1] = r_u32(source + 268);
        texture = (TextureDescriptor *)psx_addr(group, sizeof(TextureDescriptor));
        texture->payload.words[0] = words[0];
        texture->payload.words[1] = words[1];
        if (group != 0)
            DrawTexturedQuad(x | (y << 16), 0, group, 0);
        if (interactive != 0 && (sint32)index == (sint8)(menu_state.password.cursor))
        {
            {
                uint32 phase = menu_state.highlight_phase;
                uint32 step = menu_state.highlight_step;
                phase += step;
                menu_state.highlight_phase = phase;
                if ((sint32)phase >= 2048)
                {
                    menu_state.highlight_phase = 2048;
                    menu_state.highlight_step = 0 - step;
                }
                else if ((sint32)phase < -2047)
                {
                    menu_state.highlight_phase = (uint32)-2048;
                    menu_state.highlight_step = 0 - step;
                }
                amplitude = (sint32)menu_state.highlight_phase;
            }
            if (amplitude > 0)
                p2_func_800652e4(position - 65537, 0x10000E, amplitude, 0x808080);
            else if (amplitude < 0)
                p2_func_800651bc(position - 65537, 0x10000E, (sint32)(0 - (uint32)amplitude), 0x808080);
        }
        position += 15;
        x += 15;
    }
    return 0;
}

uint32 DrawPasswordScreen(sint32 requested_mode, uint32 invalid, uint32 accepted)
{
    MaterialResourceHeader *resources;
    sint8 mode = (sint8)requested_mode;
    const char *text;
    uint32 group;
    uint32 saved_frame;
    uint32 count;
    uint32 index;
    uint32 fade;
    uint32 result;
    FUNCTION_MARKER(0x8006B2B8, "SLUS_005.78");
    DrawSync(0);
    VSync(0);
    BeginDraw();
    ClearSceneOrderingTable();
    if (mode == 0)
        p2_func_80066bdc(0, 0xF00140, 0);
    else
        p2_func_80066af0(0);
    DrawPassword(97, 110, mode);
    if ((uint8)invalid != 0 || (uint8)accepted != 0)
    {
        p2_func_800651bc(0xBE0000, 0x150140, 4096, 0xFFFFFF);
        text = (uint8)invalid != 0 ? UiText::InvalidPassword : menu_state.password.feedback;
        DrawText(0xC300A0, text, 1, 0);
    }
    else if (mode == 0)
    {
        DrawText(0x4600A0, UiText::Password, 1, 0);
        DrawText(0xD20064, UiText::Continue, 0, 0);
        DrawTexturedQuad(0xC80041, 0, interface_resources->textures_offset + 220, 0);
    }
    group = menu_state.resources->textures_offset + 20;
    if (group != 0)
        DrawTexturedQuad(0x2700E4, 0, group, 0);
    p2_func_80027228();
    resources = menu_state.resources;
    if (resources != 0)
    {
        saved_frame = frame_clock.render_tick;
        count = resources->texture_count;
        frame_clock.render_tick = frame_clock.menu_tick;
        if ((sint32)count > 0)
        {
            index = 0;
            do
            {
                UpdateMaterialAnimationList(menu_state.material_groups[index].animation_entries);
                ++index;
            } while ((sint32)index < (sint32)menu_state.resources->texture_count);
        }
        frame_clock.render_tick = saved_frame;
    }
    fade = menu_state.fade_frames;
    if (fade != 0)
    {
        --fade;
        menu_state.fade_frames = fade;
        if (fade != 0)
            p2_func_800651bc(0, 0xF00140, (sint32)(fade << 12) / 15, 0xFFFFFF);
    }
    EndDraw();
    result = menu_state.pacer.Wait(menu_state.last_frame);
    menu_state.last_frame = result;
    return result;
}

uint32 GeneratePassword(void)
{
    FUNCTION_MARKER(0x8006B548, "SLUS_005.78");
    CharacterSettings settings;
    uint32 rotation, checkpoints, level, lives, pair, single, triple, checksum;
    uint32 bits, index, mask, value, result = 0;
    p2_func_800536f0(0);
    CaptureCharacterSettings(&settings);
    value = settings.counters[CharacterSettings::Lives] - 1u;
    settings.counters[CharacterSettings::Lives] = value;
    if (value < 2)
        settings.counters[CharacterSettings::Lives] = 2;
    settings.health_limit = settings.health_limit - 1u;
    rotation = random_generator.Next() & 3;
    checkpoints = r_u8(0x800B999A);
    if (p2_func_8002dfc8(78, 1) < (sint32)checkpoints)
        checkpoints = (uint32)(sint32)r_s8(0x8008A43C);
    bits = 0;
    for (index = 0; index < 5; ++index)
        if ((checkpoints & (1u << index)) != 0)
            bits |= 1u << ((index + rotation + 1) % 5);
    checkpoints = bits;
    value = settings.counters[CharacterSettings::Lives];
    level = 0;
    for (index = 0; index < 5; ++index)
        if ((value & (1u << index)) != 0)
            level |= 1u << ((index + rotation + 1) % 5);
    settings.counters[CharacterSettings::Lives] = level & 255;
    value = settings.health_limit;
    lives = 0;
    for (index = 0; index < 5; ++index)
        if ((value & (1u << index)) != 0)
            lives |= 1u << ((index + rotation + 1) % 5);
    settings.health_limit = lives;
    value = settings.action_code;
    pair = 0;
    for (index = 0; index < 2; ++index)
        if ((value & (1u << index)) != 0)
            pair |= 1u << ((index + rotation + 1) % 2);
    settings.action_code = pair;
    single = settings.flags & 1;
    settings.flags = single;
    value = settings.health_pickups;
    triple = 0;
    for (index = 0; index < 3; ++index)
        if ((value & (1u << index)) != 0)
            triple |= 1u << ((index + rotation + 1) % 3);
    settings.health_pickups = triple;
    value = checkpoints + settings.counters[CharacterSettings::Lives] + settings.health_limit + settings.action_code + (settings.flags & 1) + (triple & 255) + rotation;
    checksum = 0;
    for (index = 0; index < 9; ++index)
        if ((value & (1u << index)) != 0)
            checksum |= 1u << ((index + rotation + 1) % 9);
    mask = (1u << rotation) - 1u;
    value = (checkpoints << rotation) | (settings.counters[CharacterSettings::Lives] << (rotation + 5)) | ((uint32)settings.health_limit << (rotation + 10)) | ((uint32)settings.action_code << (rotation + 15)) | ((settings.flags & 1u) << (rotation + 17)) | ((uint32)settings.health_pickups << (rotation + 18)) | (rotation << 30) | ((checksum >> rotation) << (rotation + 21)) | (checksum & mask);
    for (index = 8; index != 0; --index)
    {
        result = value & 15;
        menu_state.password.characters[index - 1] = (uint8)(result);
        value >>= 4;
    }
    return result;
}

sint32 ApplyCheatPassword(void)
{
    FUNCTION_MARKER(0x8006B9AC, "SLUS_005.78");
    uint8 password[8];
    uint32 row;
    uint32 index;
    const uint32 *message;
    uint32 bit;
    uint32 count = 0;
    for (index = 0; index < 8; ++index)
        password[index] = (uint8)(menu_state.password.characters[index] + 65);
    for (row = 0; row < 12; ++row)
    {
        for (index = 0; index < 8; ++index)
        {
            uint32 encoded = ((uint32)password[index] * 100) ^ 0xFFFF;
            if (encoded != cheats[row].password[index])
                break;
        }
        if (index == 8)
            break;
    }
    if (row == 12)
        return 0;
    bit = 1u << row;
    if (bit == 1)
        w_u8(0x800B999A, p2_func_8002dfc8(78, 1));
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
    if ((uint8)menu_state.password.feedback_buffer[count] == 32)
    {
        do
        {
            --count;
        } while ((uint8)menu_state.password.feedback_buffer[count] == 32);
    }
    menu_state.password.feedback_buffer[count + 1] = 0;
    menu_state.password.feedback = menu_state.password.feedback_buffer;
    return 1;
}

uint32 EditPassword(uint32 requested_mode)
{
    uint8 mode = (uint8)requested_mode;
    uint32 success_until = 0;
    uint32 error_until = 0;
    uint32 pressed;
    uint32 buttons;
    uint32 sound;
    uint32 saved_frame;
    uint32 cursor;
    uint32 clock;
    uint32 offset;
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
    uint32 scene;
    sint32 digit;
    sint32 voice;
    sint32 checked;
    sint32 index;
    CharacterSettings settings;
    FUNCTION_MARKER(0x8006BBE8, "SLUS_005.78");
    if (mode != 0)
    {
        p2_func_80067160(0);
        menu_state.fade_frames = 15;
        menu_state.password.cursor = (uint8)(0);
        if (menu_state.password.reset_pending)
        {
            memset(menu_state.password.characters, 0, sizeof(menu_state.password.characters));
            menu_state.password.reset_pending = false;
        }
    }
    else
        GeneratePassword();
    clock = frame_clock.milliseconds;
    menu_state.pacer.fraction = 30;
    menu_state.last_frame = clock;
    for (;;)
    {
        pressed = 0;
        if (success_until == 0 && error_until == 0)
        {
            buttons = p2_func_80066d08();
            {
                uint32 old_buttons = menu_state.previous_buttons;
                uint32 repeat;
                if (buttons & 0x1000) ++menu_state.up_hold_frames;
                else menu_state.up_hold_frames = 0;
                if (buttons & 0x4000) ++menu_state.down_hold_frames;
                else menu_state.down_hold_frames = 0;
                pressed = buttons & ~old_buttons;
                menu_state.previous_buttons = buttons;
                repeat = menu_state.up_hold_frames;
                if ((sint32)repeat >= 11 && (repeat & 3) == 0) pressed |= 0x1000;
                repeat = menu_state.down_hold_frames;
                if ((sint32)repeat >= 11 && (repeat & 3) == 0) pressed |= 0x4000;
            }
        }
        if ((sint8)mode != 0)
            p2_func_80066de4(256);
        DrawPasswordScreen((sint8)mode, error_until != 0, success_until != 0);
        if (pressed & 0x10)
        {
            sound = 0;
            saved_frame = frame_clock.render_tick;
            if (r_u32(0x8008A408) != 0 && r_s32(r_u32(0x8008A408)) > 25)
                sound = r_u32(0x8008A408) + 204;
            frame_clock.render_tick = frame_clock.menu_tick;
            voice = sound_effects.SelectVoice(0xFFFFFFFF, sound, 4);
            if (voice >= 0)
                p2_func_8005e470(sound, (uint16)voice, 0, 0, 0, (sint16)(sound_settings.effects_volume), 0);
            frame_clock.render_tick = saved_frame;
            if (mode != 0)
            {
                p2_func_80067160(0);
                menu_state.fade_frames = 15;
            }
            return 0;
        }
        if (pressed & 0x5000)
        {
            if ((sint8)mode != 0)
            {
                sound = 0;
                saved_frame = frame_clock.render_tick;
                digit = menu_state.password.characters[menu_state.password.cursor];
                if (r_u32(0x8008A408) != 0 && r_s32(r_u32(0x8008A408)) > 24)
                    sound = r_u32(0x8008A408) + 196;
                frame_clock.render_tick = frame_clock.menu_tick;
                voice = sound_effects.SelectVoice(0xFFFFFFFF, sound, 4);
                if (voice >= 0)
                    p2_func_8005e470(sound, (uint16)voice, 0, 0, 0, (sint16)(sound_settings.effects_volume), 0);
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
                menu_state.password.characters[menu_state.password.cursor] = (uint8)(digit);
            }
            goto finish_iteration;
        }
        if (pressed & 0xA000)
        {
            if ((sint8)mode != 0)
            {
                sound = 0;
                saved_frame = frame_clock.render_tick;
                if (r_u32(0x8008A408) != 0 && r_s32(r_u32(0x8008A408)) > 24)
                    sound = r_u32(0x8008A408) + 196;
                frame_clock.render_tick = frame_clock.menu_tick;
                voice = sound_effects.SelectVoice(0xFFFFFFFF, sound, 4);
                if (voice >= 0)
                    p2_func_8005e470(sound, (uint16)voice, 0, 0, 0, (sint16)(sound_settings.effects_volume), 0);
                frame_clock.render_tick = saved_frame;
                if (pressed & 0x2000)
                {
                    cursor = (menu_state.password.cursor + 1) & 255;
                    menu_state.password.cursor = (uint8)(cursor);
                    if ((sint8)cursor == 8)
                        menu_state.password.cursor = (uint8)(0);
                }
                else
                {
                    cursor = (menu_state.password.cursor - 1) & 255;
                    menu_state.password.cursor = (uint8)(cursor);
                    if (cursor & 0x80)
                        menu_state.password.cursor = (uint8)(7);
                }
            }
            goto finish_iteration;
        }
        if ((pressed & 0x40) == 0)
            goto finish_iteration;
        sound = 0;
        saved_frame = frame_clock.render_tick;
        if (r_u32(0x8008A408) != 0 && r_s32(r_u32(0x8008A408)) > 25)
            sound = r_u32(0x8008A408) + 204;
        frame_clock.render_tick = frame_clock.menu_tick;
        voice = sound_effects.SelectVoice(0xFFFFFFFF, sound, 4);
        if (voice >= 0)
            p2_func_8005e470(sound, (uint16)voice, 0, 0, 0, (sint16)(sound_settings.effects_volume), 0);
        frame_clock.render_tick = saved_frame;
        if ((sint8)mode == 0)
            goto finish_iteration;
        checked = ApplyCheatPassword();
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
                DrawPasswordScreen(1, 0, 1);
                p2_func_80066430();
                if (r_u16(0x8008A530) != 0)
                    p2_func_8006ae90(r_u16(0x8008A530));
                w_u16(0x8008A530, 0);
                offset = resource_archive.Size(47);
                w_u32(0x8008A4DC, r_u32(0x8008A284) + offset + 132576);
                w_u32(0x8008A488, ReadAssetToGuest(47, r_u32(0x8008A284) + 132576, 1));
                ReadAssetToGuest(147, r_u32(0x8008A4C8), 1);
                p2_func_8006624c(r_u32(0x8008A4C8), 0, 1);
                p2_func_80060eec();
                p2_func_80060df4();
                p2_func_80066e48();
                LoadMenuResources();
            }
            goto finish_iteration;
        }
        encoded = 0;
        power = 1;
        for (index = 7; index >= 0; --index)
        {
            encoded += menu_state.password.characters[index] * power;
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
        settings.flags = (uint8)decoded[4];
        settings.health_limit = (uint8)decoded[2];
        settings.counters[CharacterSettings::Lives] = decoded[1];
        settings.action_code = (uint8)decoded[3];
        settings.health_pickups = (uint8)decoded[5];
        if (decoded[6] != 0 && decoded[6] == checksum && (sint32)decoded[0] >= 0 && p2_func_8002dfc8(78, 1) >= (sint32)decoded[0] && decoded[1] < 32 && decoded[2] < 32 && decoded[3] < 4 && decoded[4] < 2 && decoded[5] < 8)
        {
            ++decoded[1];
            ++decoded[2];
            settings.counters[CharacterSettings::Lives] = decoded[1];
            settings.health_limit = (uint8)decoded[2];
            scene = p2_func_800536f0(0);
            ApplyCharacterSettings(scene, &settings, (uint8)decoded[0]);
            w_u8(0x8008A134, 0);
            w_u8(0x8008A43C, decoded[0]);
            ReloadBaseCharacterModels((settings.flags & 1) != 0 ? 2 : 1);
            return 1;
        }
        if (checksum != 0 && checksum == decoded[0] + decoded[1] + decoded[2] + decoded[3] && decoded[0] == 511 - (decoded[1] + decoded[2] + decoded[3]) && decoded[1] == 31 && decoded[2] == 31 && decoded[3] == 3)
        {
            settings.counters[CharacterSettings::Lives] = decoded[3];
            settings.health_limit = 3;
            settings.action_code = 0;
            scene = p2_func_800536f0(0);
            ApplyCharacterSettings(scene, &settings, 0);
        }
        else
            error_until = menu_state.last_frame + 3000;
    finish_iteration:
        clock = menu_state.last_frame;
    expire_messages:
        if (error_until < clock)
            error_until = 0;
        if (success_until < clock)
            success_until = 0;
        frame_clock.menu_tick = frame_clock.menu_tick + 1;
    }
}
