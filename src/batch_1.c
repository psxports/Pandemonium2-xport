#include "game.h"
#include <stdlib.h>

void p2_main_8006792c(void)
{
    sint32 entry_index;
    uint32 entry_address;
    uint32 entry;
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
    uint32 scene_info;
    uint32 scene_info_stack;

    FUNCTION_MARKER(0x8006792C, "SLUS_005.78");
    p2_sdk_80011274();
    p2_func_8006637c(((uint8 *)psx_addr(0x1F800000, 1024)), ((uint8 *)psx_addr(r_u32(0x80010000), 1024)), 1024);
    p2_func_80064b18(0x80010668);
    p2_func_80064b18(0x80010674);
    while (r_s8(0x8008A134) <= 0)
    {
        if (r_u32(0x8008A294) == 1)
        {
            p2_func_80026aa0();
            p2_func_80067160(0);
            p2_func_800276dc();
            w_u32(0x8008A290, 0xFFFFFFFF);
            p2_func_8006a1ac(254, r_u32(0x8008A4C8), (r_u32(0x8008A52C) & 0x200) != 0);
            p2_func_800558f0();
            p2_func_80067160(0);
            w_u32(0x8008A294, 0);
        }
        else
        {
            if (r_s32(0x8008A294) >= 2)
            {
                if (r_u32(0x8008A294) == 2)
                {
                    p2_func_80064b18(0x80010694);
                    w_u32(0x8008A294, 0);
                }
            }
            else if (r_u32(0x8008A294) == 0)
            {
                p2_func_80064b18(0x80010684);
                w_u32(0x8008A294, 1);
            }
            p2_func_80067400(1);
        }
    resume_menu:
        while ((uint8)p2_func_8006a2ac() == 0)
            ;
        p2_func_80066e98();
        if (p2_func_8006e8d8())
        {
            entry_index = 0;
            entry_address = 0x800B1524;
            if (r_s32(r_u32(0x8008A430) + 24) > 0)
            {
                do
                {
                    entry = r_u32(entry_address);
                    if (entry)
                    {
                        p2_func_8002a448(0, entry);
                        w_u32(entry_address, 0);
                    }
                    ++entry_index;
                    entry_address += 8;
                } while (entry_index < r_s32(r_u32(0x8008A430) + 24));
            }
            w_u32(0x8008A430, 0);
            p2_func_80066430();
            if (r_u16(0x8008A530))
                p2_func_8006ae90(r_u16(0x8008A530));
            w_u16(0x8008A530, 0);
            goto start_level;
        }
        entry_index = 0;
        entry_address = 0x800B1524;
        if (r_s32(r_u32(0x8008A430) + 24) > 0)
        {
            do
            {
                entry = r_u32(entry_address);
                if (entry)
                {
                    p2_func_8002a448(0, entry);
                    w_u32(entry_address, 0);
                }
                ++entry_index;
                entry_address += 8;
            } while (entry_index < r_s32(r_u32(0x8008A430) + 24));
        }
        w_u32(0x8008A430, 0);
        w_u8(0x8008A134, (uint8)~r_u8(0x8008A134));
        p2_func_80066430();
        if (r_u16(0x8008A530))
            p2_func_8006ae90(r_u16(0x8008A530));
        w_u16(0x8008A530, 0);
        if (r_u32(0x8008A294) != 1)
        {
            p2_func_8006ab74();
            p2_func_80067160(0);
            p2_func_8006a614();
            p2_func_80060ebc();
            p2_func_80060f6c();
            p2_func_8006a528();
            p2_func_80064d54(r_u32(0x8008A284));
            p2_func_80066ce8();
            p2_func_8006b000();
            CdControlBPSX(9, NULL, NULL);
            CdSync(0, NULL);
            p2_func_8006657c();
            p2_func_8006af4c();
            CdReadyCallbackPSX(0);
            p2_func_8007e340(0);
            CdSyncCallbackPSX(0);
        }
    }
    p2_func_80026aa0();
    p2_func_80067400(0);
start_level:
    w_u32(0x8008A294, 1);
    p2_func_8002dcd0(r_s8(0x8008A134), 0);
    frame_fraction = 30;
    w_u32(0x8008A378, r_u32(0x8008A434));
    for (;;)
    {
        if (r_u32(0x1F800320) & 0x2B008)
        {
            level = r_u16(r_u32(0x8008A4E4) + 262);
            p2_func_80025be8();
            p2_func_8002dedc();
            if (r_u32(0x1F800320) & 8)
            {
                p2_func_80067160(0);
                w_u32(0x1F800320, r_u32(0x1F800320) & (0xFFFD4FF7));
                goto start_level;
            }
            if ((r_u32(0x1F800320) & 0x9000) == 0)
            {
                w_u32(0x8008A290, 0xFFFFFFFF);
                p2_func_8006a1ac(254, r_u32(0x8008A4C8), (r_u32(0x8008A52C) & 0x200) != 0);
                w_u8(0x8008A134, 0);
                if (r_u32(0x1F800320) & 0x20000)
                {
                    p2_func_80066978();
                    frame_count = 30;
                    fade_fraction = 30;
                    w_u32(0x8008A378, r_u32(0x8008A434));
                    fade = 273;
                    do
                    {
                        style = fade < 4096 ? ((fade << 16) | 1) : 1;
                        p2_func_800665d4();
                        p2_func_800665b0();
                        p2_func_80066bdc(0, 0xF00140, 0);
                        p2_func_80066af0(1);
                        p2_func_80064ff4(0x7800A0, 0x8001065C, style, 0);
                        p2_func_80066698();
                        frame_wait = 33;
                        fade_fraction -= 10;
                        if (fade_fraction <= 0)
                        {
                            frame_wait = 34;
                            fade_fraction += 30;
                        }
                        deadline = r_u32(0x8008A378) + frame_wait;
                        while (r_u32(0x8008A434) < deadline)
                            psx_native_poll_events();
                        --frame_count;
                        w_u32(0x8008A378, r_u32(0x8008A434));
                        fade += 273;
                    } while (frame_count);
                }
                p2_func_800558f0();
                w_u32(0x1F800320, r_u32(0x1F800320) & (0xFFFD4FF7));
                p2_func_80067160(0);
                goto resume_menu;
            }
            transition = (r_u32(0x1F800320) >> 15) & 1;
            w_u32(0x1F800320, r_u32(0x1F800320) & (0xFFFD4FF7));
            if (level == 78)
            {
                w_u8(0x8008A134, 78);
                p2_func_800536f0(0);
                scene_info_stack = p2_guest_stack_pointer;
                p2_guest_stack_pointer -= 24;
                scene_info = p2_guest_stack_pointer;
                p2_func_80053638(scene_info);
                ending = r_u8(scene_info) & 1;
                p2_guest_stack_pointer = scene_info_stack;
                p2_func_8006ab74();
                p2_func_80067160(0);
                p2_func_8006a614();
                p2_func_80060ebc();
                p2_func_80060f6c();
                p2_func_8006a528();
                p2_func_80064d54(r_u32(0x8008A284));
                p2_func_80066ce8();
                p2_func_8006b000();
                CdControlBPSX(9, NULL, NULL);
                CdSync(0, NULL);
                p2_func_8006657c();
                p2_func_8006af4c();
                CdReadyCallbackPSX(0);
                p2_func_8007e340(0);
                CdSyncCallbackPSX(0);
                p2_func_80064b18(ending ? 0x800106B0 : 0x800106A0);
                if ((r_u32(0x801F0000) & 0x800) == 0)
                    p2_func_80064b18(0x800106C0);
                p2_func_80067400(1);
                w_u8(0x8008A134, 0);
                p2_func_800558f0();
                p2_func_80067160(0);
                goto resume_menu;
            }
            if (level - 96 >= 2)
            {
                if ((sint32)r_u8(0x800B999A) < p2_func_8002dfc8(level, 1))
                    w_u8(0x800B999A, (uint8)p2_func_8002dfc8(level, 1));
            }
            p2_func_8006e344(level, (sint8)transition);
        }
        p2_func_8005cd24();
        p2_func_80069ac0();
        if (r_u32(0x1F800320) & 0x10)
        {
            p2_func_8006daf4(r_u16(r_u32(0x8008A4E4) + 262));
            w_u32(0x1F800384, r_u32(0x1F800384) | (0x840));
            w_u32(0x1F800388, r_u32(0x1F800388) | (0x840));
        }
        frame_wait = 33;
        frame_fraction -= 10;
        if (frame_fraction <= 0)
        {
            frame_wait = 34;
            frame_fraction += 30;
        }
        deadline = r_u32(0x8008A378) + frame_wait;
        while (r_u32(0x8008A434) < deadline)
            psx_native_poll_events();
        w_u32(0x8008A378, r_u32(0x8008A434));
        w_u32(0x8008A518, r_u32(0x8008A518) + (1));
    }
}

uint32 p2_func_8006637c(uint8 *destination, const uint8 *source, uint32 count)
{
    uint32 result = count;
    FUNCTION_MARKER(0x8006637C, "SLUS_005.78");
    while (count != 0)
    {
        result = *source++;
        *destination++ = (uint8)result;
        --count;
    }
    return result;
}

uint32 p2_func_80064b18(uint32 path)
{
    FUNCTION_MARKER(0x80064B18, "SLUS_005.78");
    /* User-authorized bypass of the separate FMV module */
    return 0;
}

void p2_func_8008347c(void)
{
    FUNCTION_MARKER(0x8008347C, "SLUS_005.78");
    xport_bios_exit_critical();
}

sint32 p2_func_8006a3dc(CdlFILE *result, const char *path)
{
    uint32 attempts = 20;
    FUNCTION_MARKER(0x8006A3DC, "SLUS_005.78");
    do
    {
        if (CdSearchFile(result, path))
            return 1;
        --attempts;
    } while (attempts != 0);
    return 0;
}

sint32 p2_func_8007bc54(const uint8 *position)
{
    uint32 minutes;
    uint32 seconds;
    uint32 frames;
    FUNCTION_MARKER(0x8007BC54, "SLUS_005.78");
    minutes = 10 * (position[0] >> 4) + (position[0] & 15);
    seconds = 10 * (position[1] >> 4) + (position[1] & 15);
    frames = 10 * (position[2] >> 4) + (position[2] & 15);
    return (sint32)(75 * (60 * minutes + seconds) + frames - 150);
}

uint32 p2_func_8008346c(void)
{
    FUNCTION_MARKER(0x8008346C, "SLUS_005.78");
    return (uint32)xport_bios_enter_critical();
}

uint32 p2_func_80067400(uint32 mode)
{
    uint32 source;
    uint32 destination;
    uint32 end;
    uint32 words[4];
    uint32 asset;
    uint32 loaded;
    uint32 offset;
    uint32 sound_data;
    uint32 sound_size;
    uint32 result;
    FUNCTION_MARKER(0x80067400, "SLUS_005.78");
    w_u32(0x1F800320, 0);
    FlushCache();
    p2_reset_callback();
    if (r_u32(0x8008A284) == 0)
    {
        w_u32(0x8008A284, p2_sdk_800829f8(4));
        p2_sdk_80082a08(r_u32(0x8008A284));
        p2_func_8008346c();
        psx_bios_init_user_heap(r_u32(0x8008A284), 0x801FF000u - r_u32(0x8008A284));
        p2_func_8008347c();
    }
    InitGeom();
    xport_gte_write_control(24, 160u << 16);
    xport_gte_write_control(25, 120u << 16);
    p2_func_8006aed0();
    p2_func_8006645c();
    CdInit();
    w_u32(0x8008A280, 1);
    p2_func_8006af88();
    p2_func_80066ca8();
    w_u32(0x8008A284, p2_func_80064d28(1254400));
    w_u32(0x8008A4C8, r_u32(0x8008A284) + 384000);
    p2_func_8006a440();
    p2_func_8006a1ac(22, r_u32(0x8008A4C8), 1);
    w_u32(0x8008A4C0, 0);
    p2_func_800669f4(r_u32(0x8008A4C8));
    w_u32(0x8008A4C0, r_u32(0x8008A4C0) + (1));
    offset = p2_func_8006a3ac(150);
    w_u32(0x8008A4DC, r_u32(0x8008A284) + offset + 132576);
    p2_func_8006a1ac(0, r_u32(0x8008A4C8), 1);
    w_u32(0x8008A46C, 1);
    w_u32(0x8008A488, r_u32(0x8008A4C8));
    p2_func_80020ae8();
    p2_func_80021538();
    p2_func_8006a1ac(3, r_u32(0x8008A4C8), 1);
    source = r_u32(0x8008A4C8);
    destination = 0x800B7A80;
    end = source + 1152;
    do
    {
        words[0] = r_u32(source);
        words[1] = r_u32(source + 4);
        words[2] = r_u32(source + 8);
        words[3] = r_u32(source + 12);
        w_u32(destination, words[0]);
        w_u32(destination + 4, words[1]);
        w_u32(destination + 8, words[2]);
        w_u32(destination + 12, words[3]);
        source += 16;
        destination += 16;
    } while (source != end);
    w_u32(0x8008A488, 0x800B7A80);
    w_u32(0x8008A46C, 4);
    p2_func_8003fd28();
    p2_func_8006a1ac(17, 0x800B4A30, 1);
    p2_func_8006aac8();
    if (r_u32(0x8008A52C) & 0x200)
    {
        loaded = p2_func_8006a1ac(47, r_u32(0x8008A284) + 132576, 1);
        asset = 147;
    }
    else
    {
        loaded = p2_func_8006a1ac(150, r_u32(0x8008A284) + 132576, 1);
        asset = 250;
    }
    destination = r_u32(0x8008A4C8);
    w_u32(0x8008A488, loaded);
    p2_func_8006a1ac(asset, destination, 1);
    offset = p2_func_8006a3c4(251);
    p2_func_8006624c(r_u32(0x8008A4C8) + offset, 0, 1);
    p2_func_8006a530(253, 0);
    offset = p2_func_8006a3c4(253);
    sound_data = r_u32(0x8008A4C8) + offset;
    sound_size = p2_func_8006a3ac(253);
    p2_func_8006aa88(sound_data, sound_size);
    SpuIsTransferCompleted(1);
    p2_func_80060eec();
    p2_func_80060df4();
    result = mode << 24;
    w_u32(0x8008A52C, 0);
    if (result != 0)
    {
        w_u32(0x8008A290, 0xFFFFFFFF);
        return p2_func_8006a1ac(254, r_u32(0x8008A4C8), 0);
    }
    return result;
}

void p2_func_8006aed0(void)
{
    uint32 saved_stack = p2_guest_stack_pointer;
    uint32 attributes_offset;
    FUNCTION_MARKER(0x8006AED0, "SLUS_005.78");
    p2_guest_stack_pointer -= 0x30;
    attributes_offset = p2_guest_stack_pointer + 0x10;
    p2_sdk_800833a4();
    p2_func_8006f158();
    /* Preserve the original unwritten fields in DRAM */
    w_u32(attributes_offset + 4, 256);
    w_u16(attributes_offset + 8, 0);
    w_u16(attributes_offset + 10, 0);
    p2_sdk_8006ff38(attributes_offset);
    SpuSetTransferMode(0);
    p2_func_80071c5c();
    p2_sdk_800759d4(0x800A9530, 2, 1);
    p2_sdk_80075bb4(1);
    p2_sdk_80079adc(16);
    p2_func_80074750();
    p2_guest_stack_pointer = saved_stack;
}

uint32 p2_func_8006f158(void)
{
    FUNCTION_MARKER(0x8006F158, "SLUS_005.78");
    return p2_sdk_8006f178(0);
}

uint32 p2_func_80071c5c(void)
{
    FUNCTION_MARKER(0x80071C5C, "SLUS_005.78");
    p2_reset_callback();
    p2_func_8006f158();
    return p2_sdk_80071b74();
}

uint32 p2_func_80077124(uint32 mode)
{
    FUNCTION_MARKER(0x80077124, "SLUS_005.78");
    w_u32(0x80087D90, mode != 1);
    return 1;
}

uint32 p2_func_8007b448(void)
{
    FUNCTION_MARKER(0x8007B448, "SLUS_005.78");
    return r_u32(0x80089524);
}

void p2_func_80074750(void)
{
    FUNCTION_MARKER(0x80074750, "SLUS_005.78");
    p2_sdk_800744ec(1);
}

uint32 p2_func_8006645c(void)
{
    uint32 buffer;
    uint32 active;
    uint32 index;
    sint32 y;
    PSX_RECT rectangle;
    FUNCTION_MARKER(0x8006645C, "SLUS_005.78");
    p2_reset_graph_zero();
    buffer = 0x80087200;
    y = 0;
    for (index = 0; index < 2; ++index)
    {
        SetDefDrawEnv(((DRAWENV *)psx_addr(buffer, sizeof(DRAWENV))), 0, y, 320, 240);
        SetDefDispEnv(((DISPENV *)psx_addr(buffer + 92, sizeof(DISPENV))), 0, y, 320, 240);
        y += 240;
        buffer += 132;
    }
    active = 0x80087200 + 132 * r_u8(0x8008A374);
    buffer = r_u32(active + 112);
    w_u32(0x8008A4A8, active + 92);
    rectangle.x = 0;
    rectangle.y = 0;
    rectangle.w = 1024;
    rectangle.h = 512;
    w_u32(0x8008A454, buffer);
    ClearImage(&rectangle, 0, 0, 0);
    DrawSync(0);
    SetDispMask(1);
    VSyncCallbackPSX(0x80066C7C);
    return p2_sdk_8007febc(0x80066C40);
}

void p2_func_80066c7c(void)
{
    uint32 environment;
    FUNCTION_MARKER(0x80066C7C, "SLUS_005.78");
    environment = r_u32(0x8008A4A8);
    if (environment != 0)
    {
        PutDispEnv(((DISPENV *)psx_addr(environment, sizeof(DISPENV))));
        w_u32(0x8008A4A8, 0);
    }
}

void p2_func_8006af88(void)
{
    uint32 handle;
    FUNCTION_MARKER(0x8006AF88, "SLUS_005.78");
    p2_func_8008346c();
    handle = (uint32)OpenEventPSX(0xF2000002, 2, 0x1000, 0x8006EBFC);
    w_u32(0x8008A3B4, handle);
    EnableEventPSX(handle);
    p2_sdk_800834ac(0xF2000002, 0x1088, 0x1000);
    p2_sdk_80083584(0xF2000002);
    p2_func_8008347c();
    w_u32(0x8008A434, 0);
}

uint32 p2_func_80066ca8(void)
{
    FUNCTION_MARKER(0x80066CA8, "SLUS_005.78");
    InitPAD(0x800A5C28, 34, 0, 0);
    StartPAD();
    return ChangeClearPAD(0);
}

uint32 p2_func_8008371c(uint32 buffer0, uint32 size0, uint32 buffer1, uint32 size1)
{
    FUNCTION_MARKER(0x8008371C, "SLUS_005.78");
    return InitPAD(buffer0, size0, buffer1, size1);
}

uint32 p2_func_8008372c(void)
{
    FUNCTION_MARKER(0x8008372C, "SLUS_005.78");
    return StartPAD();
}

uint32 p2_func_8006ebfc(void)
{
    FUNCTION_MARKER(0x8006EBFC, "SLUS_005.78");
    w_u32(0x8008A434, r_u32(0x8008A434) + (1));
    return r_u32(0x8008A434);
}

uint32 p2_func_80064d28(uint32 bytes)
{
    FUNCTION_MARKER(0x80064D28, "SLUS_005.78");
    w_u32(0x8008A258, r_u32(0x8008A258) + (bytes));
    return p2_sdk_800829f8(bytes);
}

sint32 p2_func_8006a440(void)
{
    FUNCTION_MARKER(0x8006A440, "SLUS_005.78");
    if (!p2_read_package(0, r_u32(0x8008A284), 16384))
        return 0;
    p2_func_8006637c((uint8 *)psx_addr(0x800A5C50, 14560), (const uint8 *)psx_addr(r_u32(0x8008A284), 14560), 14560);
    return 1;
}

uint32 p2_func_8006a1ac(uint32 asset, uint32 destination, uint32 flags)
{
    uint32 record = 0x800A5C50 + 8 * asset;
    uint32 bytes;
    uint32 offset;
    FUNCTION_MARKER(0x8006A1AC, "SLUS_005.78");
    /* Retain the original sector truncation and transfer rounding */
    offset = (uint32)(r_s32(record) / 2048) * 2048;
    bytes = r_u32(record + 4);
    if ((bytes & 2047) != 0)
        bytes = bytes + 2048 - (bytes & 2047);
    if (!p2_read_package(offset, destination, bytes))
    {
        fprintf(stderr, "P2: DATA/JESTERS.PKG read failed: asset %u offset %u bytes %u\n", asset, offset, bytes);
        abort();
    }
    if ((flags << 24) == 0)
    {
        w_u32(0x8008A3A0, asset);
        w_u32(0x8008A3A4, destination);
    }
    /* File reads finish synchronously; retain completion polling for callers */
    w_u8(0x8008A39C, (flags << 24) == 0);
    return destination;
}

sint32 p2_func_800669f4(uint32 image)
{
    uint32 origin;
    sint32 x;
    sint32 y;
    PSX_RECT rectangle;
    FUNCTION_MARKER(0x800669F4, "SLUS_005.78");
    origin = r_u32(0x80087264 + 132 * r_u8(0x8008A374));
    if (r_u32(0x8008A4C0))
    {
        rectangle.x = (sint16)r_u32(0x8008A36C);
        rectangle.y = (sint16)r_u32(0x8008A370);
        rectangle.w = (sint16)r_u16(0x8008A27C);
        rectangle.h = (sint16)r_u16(0x8008A27E);
        VSync(0);
        ClearImage(&rectangle, 0, 0, 0);
    }
    else
    {
        LoadImagePSX(((PSX_RECT *)psx_addr(0x8008A278, sizeof(PSX_RECT))), ((uint32 *)psx_addr(image, sizeof(uint32))));
    }
    x = (sint16)origin + 112;
    y = (sint16)(origin >> 16) + 84;
    w_u32(0x8008A36C, (uint32)x);
    w_u32(0x8008A370, (uint32)y);
    MoveImage(((PSX_RECT *)psx_addr(0x8008A278, sizeof(PSX_RECT))), x, y);
    return DrawSync(0);
}

uint32 p2_func_80066c40(void)
{
    uint32 result;
    uint32 index;
    FUNCTION_MARKER(0x80066C40, "SLUS_005.78");
    result = r_u32(0x8008A4B0);
    if (result != 0)
    {
        index = r_u8(0x8008A374);
        w_u32(0x8008A4B0, 0);
        result = 0x8008725C;
        w_u32(0x8008A4A8, result + 132 * index);
    }
    return result;
}

uint32 p2_func_8006a3ac(uint32 asset)
{
    FUNCTION_MARKER(0x8006A3AC, "SLUS_005.78");
    return r_u32(0x800A5C54 + 8 * asset);
}

uint32 p2_func_80020ae8(void)
{
    uint32 offset;
    uint32 source;
    uint32 destination;
    uint32 end;
    uint32 words[4];
    uint32 result;
    FUNCTION_MARKER(0x80020AE8, "SLUS_005.78");
    offset = p2_func_8006a3c4(r_u32(0x8008A46C));
    w_u32(0x8008A2E8, r_u32(r_u32(0x8008A488) + offset));
    offset = p2_func_8006a3c4(r_u32(0x8008A46C));
    source = r_u32(0x8008A488) + offset + 4;
    end = r_u32(0x8008A488) + offset + 2324;
    destination = 0x8008A578;
    do
    {
        words[0] = r_u32(source);
        words[1] = r_u32(source + 4);
        words[2] = r_u32(source + 8);
        words[3] = r_u32(source + 12);
        w_u32(destination, words[0]);
        w_u32(destination + 4, words[1]);
        w_u32(destination + 8, words[2]);
        w_u32(destination + 12, words[3]);
        source += 16;
        destination += 16;
    } while (source != end);
    result = r_u32(0x8008A46C) + 1;
    w_u32(0x8008A46C, result);
    return result;
}

uint32 p2_func_8006a3c4(uint32 asset)
{
    FUNCTION_MARKER(0x8006A3C4, "SLUS_005.78");
    return r_u32(0x800A5C50 + 8 * asset);
}

uint32 p2_func_80021538(void)
{
    uint32 offset;
    uint32 source;
    uint32 destination;
    uint32 end;
    uint32 words[4];
    uint32 result;
    FUNCTION_MARKER(0x80021538, "SLUS_005.78");
    offset = p2_func_8006a3c4(r_u32(0x8008A46C));
    w_u32(0x8008A2F8, r_u32(r_u32(0x8008A488) + offset));
    offset = p2_func_8006a3c4(r_u32(0x8008A46C));
    source = r_u32(0x8008A488) + offset + 4;
    end = r_u32(0x8008A488) + offset + 836;
    destination = 0x8008AE88;
    do
    {
        words[0] = r_u32(source);
        words[1] = r_u32(source + 4);
        words[2] = r_u32(source + 8);
        words[3] = r_u32(source + 12);
        w_u32(destination, words[0]);
        w_u32(destination + 4, words[1]);
        w_u32(destination + 8, words[2]);
        w_u32(destination + 12, words[3]);
        source += 16;
        destination += 16;
    } while (source != end);
    words[0] = r_u32(source);
    words[1] = r_u32(source + 4);
    w_u32(destination, words[0]);
    w_u32(destination + 4, words[1]);
    result = r_u32(0x8008A46C) + 1;
    w_u32(0x8008A46C, result);
    return result;
}

uint32 p2_func_8003fd28(void)
{
    uint32 asset = 4;
    uint32 result;
    uint32 offset;
    uint32 index;
    uint32 next;
    uint32 pointer;
    FUNCTION_MARKER(0x8003FD28, "SLUS_005.78");
    do
    {
        result = p2_func_8006a3ac(asset);
        if (result != 72)
            break;
        ++asset;
        offset = p2_func_8006a3c4(r_u32(0x8008A46C));
        index = r_u32(0x8008A334);
        next = r_u32(0x8008A46C) + 1;
        w_u32(0x8008A334, index + 1);
        pointer = r_u32(0x8008A488) + offset;
        w_u32(0x8008A46C, next);
        result = asset < 150;
        w_u32(0x8008B468 + 4 * index, pointer);
    } while (asset < 150);
    return result;
}

uint32 p2_func_8006aac8(void)
{
    uint32 bank;
    uint32 offset;
    uint32 sequence;
    uint32 result;
    FUNCTION_MARKER(0x8006AAC8, "SLUS_005.78");
    bank = p2_sdk_80079f48(0x800B4A30, -1, 4112);
    offset = p2_func_8006a3c4(20);
    p2_sdk_8007a680(0x800B4A30 + offset, (sint16)bank);
    while ((p2_sdk_8007a740(0) << 16) == 0)
        ;
    offset = p2_func_8006a3c4(19);
    sequence = p2_sdk_80071c8c(0x800B4A30 + offset, (sint16)bank);
    result = ((sequence + 1) & 255) << 8;
    w_u16(0x8008A3B0, (uint16)(((bank + 1) & 255) | result));
    return result;
}

uint32 p2_func_80077150(void)
{
    FUNCTION_MARKER(0x80077150, "SLUS_005.78");
    return r_u32(0x80087D90) != 1;
}

uint32 p2_func_80070e8c(const void *source, uint32 bytes)
{
    uint32 length = bytes;
    FUNCTION_MARKER(0x80070E8C, "SLUS_005.78");
    if (length > 0x7EFF0)
        length = 0x7EFF0;
    p2_sdk_8006fb68(source, length);
    if (r_u32(0x80087D94) == 0)
        w_u32(0x80087D90, 0);
    return length;
}

uint32 p2_func_8006624c(uint32 image, uint32 index, uint32 flags)
{
    uint32 offset;
    uint32 result;
    sint32 status;
    FUNCTION_MARKER(0x8006624C, "SLUS_005.78");
    LoadImagePSX(((PSX_RECT *)psx_addr(0x800871B0 + 8 * index, sizeof(PSX_RECT))), ((uint32 *)psx_addr(image, sizeof(uint32))));
    if (index == 0)
    {
        offset = p2_func_8006a3c4(252);
        LoadImagePSX(((PSX_RECT *)psx_addr(0x800871F0, sizeof(PSX_RECT))), ((uint32 *)psx_addr(image + offset, sizeof(uint32))));
    }
    if ((flags << 24) != 0)
    {
        do
        {
            status = DrawSync(1);
        } while (status > 0);
    }
    result = index < 2;
    if (r_u32(0x8008A52C) & 0x800)
    {
        result = 9;
        if (index >= 2)
        {
            result = 8 * index;
            if (index != 9)
            {
                MoveImage(((PSX_RECT *)psx_addr(0x800871F0, sizeof(PSX_RECT))), r_s16(0x800871B0 + result), r_s16(0x800871B2 + result));
                result = flags << 24;
                if (result != 0)
                {
                    do
                    {
                        result = (uint32)DrawSync(1);
                    } while ((sint32)result > 0);
                }
            }
        }
    }
    return result;
}

uint32 p2_func_8006a530(uint32 asset, uint32 mode)
{
    uint32 size;
    uint32 rounded;
    uint32 address;
    FUNCTION_MARKER(0x8006A530, "SLUS_005.78");
    if (mode == 0)
        address = 192576;
    else
    {
        size = p2_func_8006a3ac(mode == 1 ? asset : 253);
        rounded = size + 15;
        if ((sint32)rounded < 0)
            rounded = size + 30;
        rounded = (uint32)((sint32)rounded >> 4) << 4;
        if (mode == 1)
            address = 512288 - rounded;
        else
            address = rounded + 192576;
    }
    w_u32(0x8008A3A8, address);
    return p2_sdk_80070eec(r_u32(0x8008A3A8));
}

uint32 p2_func_8006aa88(uint32 source, uint32 bytes)
{
    FUNCTION_MARKER(0x8006AA88, "SLUS_005.78");
    return p2_func_80070e8c(((uint8 *)psx_addr(source, bytes > 0x7EFF0 ? 0x7EFF0 : bytes)), bytes);
}

uint32 p2_func_80060eec(void)
{
    FUNCTION_MARKER(0x80060EEC, "SLUS_005.78");
    p2_func_8002f144();
    p2_func_8002a74c();
    w_u32(0x800BA380, 0x80061174);
    w_u32(0x800BA38C, 0x8006117C);
    w_u32(0x800BA390, 0x800607BC);
    w_u32(0x800BA3FC, 0x800611E8);
    w_u32(0x800BA400, 0x80060C0C);
    w_u16(0x800B9AD0, 0);
    return 1;
}

uint32 p2_func_8002f144(void)
{
    uint32 result;
    FUNCTION_MARKER(0x8002F144, "SLUS_005.78");
    p2_func_80066358(0x800B7F50, 0, 32);
    result = r_u32(0x8008A284);
    w_u32(0x800B7F54, result);
    return result;
}

uint32 p2_func_80066358(uint32 offset, uint8 value, uint32 count)
{
    FUNCTION_MARKER(0x80066358, "SLUS_005.78");
    while (count != 0)
    {
        w_u8(offset, value);
        ++offset;
        --count;
    }
    return 0;
}

uint32 p2_func_8002a74c(void)
{
    uint32 remaining = 99;
    uint32 current;
    uint32 next;
    uint32 result;
    FUNCTION_MARKER(0x8002A74C, "SLUS_005.78");
    current = r_u32(0x8008A284) + 800;
    w_u32(0x8008A314, current);
    w_u32(0x8008A318, current);
    do
    {
        current = r_u32(0x8008A318);
        --remaining;
        next = current + 48;
        w_u32(current, next);
        w_u32(0x8008A318, next);
    } while (remaining != 0);
    result = r_u32(0x8008A314);
    w_u32(current + 48, 0);
    w_u32(0x8008A318, result);
    return result;
}

uint32 p2_func_80060df4(void)
{
    uint32 offset;
    FUNCTION_MARKER(0x80060DF4, "SLUS_005.78");
    w_u32(0x8008A46C, (r_u32(0x8008A52C) & 0x200) ? 48 : 151);
    p2_func_8002a470();
    p2_func_800441e8();
    p2_func_8002d4d0();
    p2_func_80034b44();
    w_u32(0x8008A408, 0);
    if ((sint32)p2_func_8006a3c4(r_u32(0x8008A46C)) > 0)
    {
        offset = p2_func_8006a3c4(r_u32(0x8008A46C));
        w_u32(0x8008A408, p2_func_8006aa48(r_u32(0x8008A488) + offset));
    }
    w_u32(0x8008A46C, r_u32(0x8008A46C) + (1));
    p2_func_80055ac4();
    return p2_func_80030994();
}

uint32 p2_func_8002a470(void)
{
    uint32 offset;
    uint32 header;
    uint32 count;
    uint32 group;
    uint32 kind;
    uint32 entry;
    uint32 remaining;
    uint32 found;
    uint32 selected;
    uint32 data;
    uint32 values[4];
    uint32 result;
    FUNCTION_MARKER(0x8002A470, "SLUS_005.78");
    offset = p2_func_8006a3c4(r_u32(0x8008A46C));
    header = r_u32(0x8008A488) + offset;
    w_u32(0x8008A4A4, header);
    p2_func_80069f40(header, -1);
    header = r_u32(0x8008A4A4);
    count = r_u32(header);
    w_u32(0x8008A46C, r_u32(0x8008A46C) + 1);
    if (count != 0)
    {
        p2_func_8006637c((uint8 *)psx_addr(r_u32(0x800B7F54) + 16 * r_u32(0x800B7F50), 16 * count), (const uint8 *)psx_addr(r_u32(header + 4), 16 * count), 16 * count);
        w_u32(0x800B7F50, r_u32(0x800B7F50) + r_u32(r_u32(0x8008A4A4)));
    }
    for (group = 0; group < 35; ++group)
    {
        selected = 0;
        header = r_u32(0x8008A4A4);
        w_u32(0x800B1708 + 8 * group, r_u32(header + 28) + 20 * group);
        for (kind = 0; kind < 3; ++kind)
        {
            entry = r_u32(header + 4);
            remaining = r_u32(header);
            for (;;)
            {
                if (r_u32(entry) == r_u32(0x800B1708 + 8 * group) && r_u8(entry + 4) == kind)
                {
                    found = (uint32)((sint32)(entry - r_u32(header + 4)) >> 4) << 1;
                    if (kind != 0)
                        found |= (r_u8(entry + 5) & 15) << 26;
                    break;
                }
                --remaining;
                entry += 16;
                if (remaining == 0)
                {
                    found = 0xFFFFFFFF;
                    break;
                }
            }
            values[selected] = found;
            if (found != 0xFFFFFFFF)
                ++selected;
        }
        if (selected != 0)
        {
            count = selected + 1;
            data = p2_func_80064d28(4 * count);
            p2_func_80066358(data, 255, 4 * count);
            w_u32(data, count - 1);
            w_u32(0x800B170C + 8 * group, data + 4);
            p2_func_8006637c((uint8 *)psx_addr(data + 4, 4 * selected), (const uint8 *)values, 4 * selected);
        }
    }
    result = r_u32(0x800B17D0);
    data = r_u32(0x800B17D8);
    offset = r_u32(0x800B17E0);
    w_u32(0x8008A520, result);
    w_u32(0x8008A418, data);
    w_u32(0x8008A410, offset);
    return result;
}

uint32 p2_func_80069f40(uint32 object, sint32 mode)
{
    FUNCTION_MARKER(0x80069F40, "SLUS_005.78");
    return p2_func_80068790(object, object, mode);
}

uint32 p2_func_80068790(uint32 object, uint32 base, sint32 mode)
{
    uint32 pointer;
    uint32 remaining;
    uint32 record;
    uint32 source;
    uint32 flags;
    uint32 count;
    uint32 index;
    uint32 result;
    uint32 words[3];
    uint16 preserved;
    uint16 first;
    uint16 second;
    uint8 coordinates[8];
    uint8 x;
    uint8 y;
    FUNCTION_MARKER(0x80068790, "SLUS_005.78");
    pointer = r_u32(object + 20);
    if (pointer != 0)
        w_u32(object + 20, base + pointer);
    if (r_u32(object + 24) != 0)
    {
        remaining = r_u32(object + 24);
        record = base + r_u32(object + 28);
        w_u32(object + 28, record);
        do
        {
            flags = r_u32(record);
            if ((flags & 0x1400) == 0x1000)
            {
                if (r_u32(record + 4) >= 2001)
                    w_u32(record + 4, 0);
                source = r_u32(object + 28) + 20 * r_u32(record + 4);
                coordinates[3] = r_u8(record + 11);
                coordinates[4] = r_u8(record + 12);
                coordinates[5] = r_u8(record + 13);
                coordinates[6] = r_u8(record + 14);
                w_u32(record + 4, source);
                x = r_u8(source + 4);
                y = r_u8(source + 5);
                coordinates[0] = r_u8(record + 8);
                coordinates[1] = r_u8(record + 9);
                coordinates[2] = r_u8(record + 10);
                coordinates[7] = r_u8(record + 15);
                for (index = 0; index < 8; ++index)
                    w_u8(record + 8 + index, coordinates[index] + ((index & 1) ? y : x));
            }
            else if (flags & 0x400)
            {
                if (flags & 0x800)
                {
                    preserved = r_u16(record + 10);
                    source = r_u32(object + 28) + 20 * r_u32(record + 4);
                    words[0] = r_u32(source + 4);
                    words[1] = r_u32(source + 8);
                    words[2] = r_u32(source + 12);
                    w_u32(record + 4, words[0]);
                    w_u32(record + 8, words[1]);
                    w_u32(record + 12, words[2]);
                    if (r_u32(record) & 0x80000)
                        w_u16(record + 10, preserved);
                }
                else if (mode == 5)
                {
                    first = r_u16(record + 8);
                    second = r_u16(record + 10);
                    w_u16(record + 8, first | 0x10);
                    w_u16(record + 10, second | 0x4000);
                }
            }
            record += 20;
            --remaining;
        } while (remaining != 0);
    }
    pointer = r_u32(object + 12);
    if (pointer != 0)
        w_u32(object + 12, base + pointer);
    pointer = r_u32(object + 4);
    result = base + pointer;
    if (pointer != 0)
    {
        record = result;
        remaining = r_u32(object);
        w_u32(object + 4, record);
        do
        {
            pointer = r_u32(record);
            if (pointer != 0)
                w_u32(record, base + pointer);
            pointer = r_u32(record + 8);
            if (pointer != 0)
                w_u32(record + 8, base + pointer);
            result = r_u8(record + 4);
            if (result == 0)
            {
                result = base + r_u32(record + 12);
                w_u32(record + 12, result);
            }
            else if (result == 2)
            {
                source = base + r_u32(record + 12);
                result = r_u8(record + 5) & 0x80;
                w_u32(record + 12, source);
                if (result == 0 && mode == 5)
                {
                    result = r_u32(record + 8);
                    count = r_u8(result + 5);
                    if (count != 0)
                    {
                        ++count;
                        pointer = source + 6;
                        do
                        {
                            first = r_u16(pointer - 2);
                            second = r_u16(pointer);
                            --count;
                            result = first | 0x10;
                            w_u16(pointer - 2, result);
                            w_u16(pointer, second | 0x4000);
                            pointer += 8;
                        } while (count != 0);
                    }
                }
            }
            record += 16;
            --remaining;
        } while (remaining != 0);
    }
    return result;
}

uint32 p2_func_800441e8(void)
{
    FUNCTION_MARKER(0x800441E8, "SLUS_005.78");
    w_u32(0x800B7A6C, 0x8004165C);
    w_u32(0x800BA3D8, 0x8004264C);
    w_u32(0x800ABD60, 0x80044230);
    w_u32(0x800B7A58, 0x80041060);
    return 0x80041060;
}

uint32 p2_func_8002d4d0(void)
{
    FUNCTION_MARKER(0x8002D4D0, "SLUS_005.78");
    w_u32(0x800BA398, 0x8002C608);
    p2_func_8002b058(0);
    p2_func_8002b058(11);
    return p2_func_8002b058(19);
}

uint32 p2_func_8002b058(uint8 kind)
{
    uint32 offset;
    uint32 object;
    uint32 index;
    uint32 slot;
    uint32 model;
    uint32 header;
    uint32 field;
    uint32 flags;
    uint8 byte;
    FUNCTION_MARKER(0x8002B058, "SLUS_005.78");
    offset = p2_func_8006a3c4(r_u32(0x8008A46C));
    slot = r_u32(0x8008A46C) + 1;
    index = r_u32(0x8008A320);
    object = r_u32(0x8008A488) + offset;
    w_u32(0x8008A320, index + 1);
    w_u32(0x8008B420 + 4 * index, object);
    if ((sint32)p2_func_8006a3ac(slot) <= 0)
        w_u32(object + 4, 0);
    else
    {
        offset = p2_func_8006a3c4(r_u32(0x8008A46C) + 1);
        w_u32(object + 4, p2_func_8006aa48(r_u32(0x8008A488) + offset));
    }
    w_u8(object + 1, kind);
    w_u32(0x8008A46C, r_u32(0x8008A46C) + 2);
    field = object;
    for (index = 0; index < 4; ++index)
    {
        offset = p2_func_8006a3c4(r_u32(0x8008A46C));
        if (offset != 0xFFFFFFFF)
        {
            slot = p2_func_8006a3c4(r_u32(0x8008A46C) + 2);
            header = r_u32(0x8008A488) + slot;
            p2_func_80069f40(header, -1);
            model = r_u32(0x8008A488) + offset;
            w_u32(field + 8, model);
            flags = r_u32(model + 28);
            byte = r_u8(model + 13);
            w_u32(model + 28, flags | 0x20024);
            w_u8(model + 13, byte | 0x80);
            p2_func_80069f64(model, header);
            if ((sint32)p2_func_8006a3ac(r_u32(0x8008A46C) + 1) <= 0)
                w_u32(model + 104, 0);
            else
            {
                slot = p2_func_8006a3c4(r_u32(0x8008A46C) + 1);
                p2_func_8006a170(model, r_u32(0x8008A488) + slot);
            }
        }
        w_u32(0x8008A46C, r_u32(0x8008A46C) + 3);
        field += 4;
    }
    return 0;
}

uint32 p2_func_8006aa48(uint32 table)
{
    uint32 remaining;
    uint32 pointer;
    uint32 base;
    FUNCTION_MARKER(0x8006AA48, "SLUS_005.78");
    remaining = r_u32(table);
    pointer = table + 4;
    if ((sint32)remaining <= 0)
        return 0;
    base = r_u32(0x8008A3A8);
    do
    {
        --remaining;
        w_u32(pointer, r_u32(pointer) + base);
        pointer += 8;
    } while (remaining != 0);
    return table;
}

uint32 p2_func_80069f64(uint32 object, uint32 header)
{
    uint32 result;
    uint32 count;
    uint32 destination;
    uint32 source;
    FUNCTION_MARKER(0x80069F64, "SLUS_005.78");
    result = p2_func_80068570(object, header);
    count = r_u32(header);
    if ((sint32)count > 0)
    {
        destination = r_u32(0x800B7F54) + 16 * r_u32(0x800B7F50);
        source = r_u32(header + 4);
        p2_func_8006637c((uint8 *)psx_addr(destination, 16 * count), (const uint8 *)psx_addr(source, 16 * count), 16 * count);
        result = r_u32(0x800B7F50) + r_u32(header);
        w_u32(0x800B7F50, result);
    }
    return result;
}

uint32 p2_func_80068570(uint32 object, uint32 header)
{
    uint32 pointer;
    uint32 offset;
    uint32 remaining;
    uint32 index;
    uint32 kind;
    uint32 value;
    uint32 result;
    FUNCTION_MARKER(0x80068570, "SLUS_005.78");
    offset = r_u32(object + 40);
    if (offset != 0)
        w_u32(object + 40, object + offset);
    offset = r_u32(object + 36);
    if (offset != 0)
    {
        pointer = object + offset;
        w_u32(object + 36, pointer);
        if ((r_u32(object + 28) & 12) == 8)
            p2_sdk_80082a18((const char *)psx_addr(0x8008A29C, sizeof("Share\n")));
        else
        {
            remaining = (uint32)r_s16(object + 32);
            do
            {
                index = r_u32(pointer);
                --remaining;
                w_u32(pointer, r_u32(header + 28) + 20 * index);
                pointer += 24;
            } while (remaining != 0);
        }
    }
    offset = r_u32(object + 8);
    if (offset != 0)
        w_u32(object + 8, object + offset);
    offset = r_u32(object + 4);
    if (offset != 0)
        w_u32(object + 4, object + offset);
    offset = r_u32(object + 156);
    if (offset != 0)
        w_u32(object + 156, object + offset);
    pointer = r_u32(object + 8);
    while (pointer != 0)
    {
        p2_func_800683dc(pointer, object, header);
        pointer = r_u32(pointer + 4);
    }
    pointer = r_u32(object + 156);
    if (pointer != 0)
        w_u32(pointer + 16, r_u32(0x800B1708));
    offset = r_u32(object + 44);
    result = object + offset;
    if (offset != 0)
    {
        pointer = result;
        w_u32(object + 44, pointer);
        remaining = r_u8(pointer - 4);
        result = remaining;
        if (remaining != 0)
        {
            do
            {
                value = r_u32(pointer);
                if (value != 0xFFFFFFFF)
                {
                    index = (value >> 1) & 0x1FF;
                    kind = r_u8(r_u32(header + 4) + 16 * index + 4);
                    index += r_u32(0x800B7F50);
                    if (kind == 0)
                        w_u32(pointer, 2 * index);
                    else if (kind == 2)
                        w_u32(pointer, 2 * index | (value & 0x3FC00) | (value & 0x3FC0000) | (value & 0x3C000000));
                }
                pointer += 4;
                --remaining;
                result = remaining;
            } while (remaining != 0);
        }
    }
    return result;
}

uint32 p2_func_800683dc(uint32 node, uint32 base, uint32 header)
{
    uint32 offset;
    uint32 result;
    uint32 pointer;
    uint32 remaining;
    uint32 value;
    uint32 index;
    uint32 kind;
    uint32 child;
    FUNCTION_MARKER(0x800683DC, "SLUS_005.78");
    offset = r_u32(node + 8);
    if (offset != 0)
        w_u32(node + 8, base + offset);
    offset = r_u32(node + 4);
    if (offset != 0)
        w_u32(node + 4, base + offset);
    result = 7;
    if (r_u8(node + 12) == 7)
    {
        result = r_u32(node + 16);
        offset = r_u32(node + 20);
        w_u32(node + 16, r_u32(header + 28) + 20 * result);
        if (offset != 0)
        {
            pointer = base + offset;
            w_u32(node + 20, pointer);
            remaining = r_u8(pointer - 4);
            result = remaining;
            if (remaining != 0)
            {
                do
                {
                    value = r_u32(pointer);
                    if (value != 0xFFFFFFFF)
                    {
                        index = (value >> 1) & 0x1FF;
                        kind = r_u8(r_u32(header + 4) + 16 * index + 4);
                        index += r_u32(0x800B7F50);
                        if (kind == 0)
                            w_u32(pointer, 2 * index);
                        else if (kind == 2)
                            w_u32(pointer, 2 * index | (value & 0x3FC00) | (value & 0x3FC0000) | (value & 0x3C000000));
                    }
                    pointer += 4;
                    --remaining;
                    result = remaining;
                } while (remaining != 0);
            }
        }
    }
    child = r_u32(node + 8);
    while (child != 0)
    {
        result = p2_func_800683dc(child, base, header);
        child = r_u32(child + 4);
    }
    return result;
}

uint32 p2_func_8006a170(uint32 object, uint32 animation)
{
    uint32 result;
    FUNCTION_MARKER(0x8006A170, "SLUS_005.78");
    result = p2_func_80068294(animation, animation);
    w_u32(object + 104, animation);
    return result;
}

uint32 p2_func_80068294(uint32 animation, uint32 base)
{
    uint32 remaining;
    uint32 pointer;
    uint32 offset;
    uint32 result;
    FUNCTION_MARKER(0x80068294, "SLUS_005.78");
    remaining = (uint32)r_s8(animation);
    if (remaining != 0)
    {
        pointer = base + r_u32(animation + 8);
        w_u32(animation + 8, pointer);
        pointer += 12;
        do
        {
            offset = r_u32(pointer);
            if (offset != 0 && offset != 0xFFFFFFFF)
                w_u32(pointer, base + offset);
            --remaining;
            pointer += 20;
        } while (remaining != 0);
    }
    remaining = (uint32)r_s8(animation + 1);
    if (remaining != 0)
    {
        pointer = base + r_u32(animation + 12);
        w_u32(animation + 12, pointer);
        pointer += 12;
        do
        {
            offset = r_u32(pointer);
            if (offset != 0 && offset != 0xFFFFFFFF)
                w_u32(pointer, base + offset);
            --remaining;
            pointer += 16;
        } while (remaining != 0);
    }
    remaining = (uint32)r_s8(animation + 2);
    if (remaining != 0)
    {
        pointer = base + r_u32(animation + 16);
        w_u32(animation + 16, pointer);
        pointer += 12;
        do
        {
            offset = r_u32(pointer);
            if (offset != 0 && offset != 0xFFFFFFFF)
                w_u32(pointer, base + offset);
            --remaining;
            pointer += 16;
        } while (remaining != 0);
    }
    result = r_u32(animation + 20);
    if (result != 0)
    {
        remaining = (uint32)r_s8(animation + 3);
        result += base;
        if (remaining != 0)
        {
            w_u32(animation + 20, result);
            pointer = result + 16;
            do
            {
                offset = r_u32(pointer - 4);
                if (offset != 0)
                    w_u32(pointer - 4, base + offset);
                offset = r_u32(pointer);
                result = base + offset;
                if (offset != 0)
                    w_u32(pointer, result);
                --remaining;
                pointer += 24;
            } while (remaining != 0);
        }
    }
    return result;
}

uint32 p2_func_80034b44(void)
{
    FUNCTION_MARKER(0x80034B44, "SLUS_005.78");
    w_u32(0x800B7A50, 0x80032DDC);
    w_u32(0x800B7A70, 0x80033564);
    w_u32(0x800ABD58, 0x80034044);
    w_u32(0x800BA3C0, 0x80030AC8);
    w_u32(0x800BA3C4, 0x800351A4);
    w_u8(0x8008A330, 0);
    return 0x800351A4;
}

uint32 p2_func_80055ac4(void)
{
    uint32 index;
    uint32 offset;
    uint32 object;
    uint32 header;
    uint32 flags;
    uint32 next;
    uint32 words[2];
    uint8 byte;
    FUNCTION_MARKER(0x80055AC4, "SLUS_005.78");
    w_u32(0x800B7A64, 0x8004DCD0);
    w_u32(0x800BA384, 0x800530A0);
    w_u32(0x800BA404, 0x80053704);
    w_u32(0x800ABD54, 0x80048DB4);
    w_u32(0x800B7A4C, 0x80047D6C);
    w_u8(0x8008A45C, 0);
    w_u16(0x8008A532, 0);
    w_u32(0x800BA388, 0x80045ED8);
    p2_func_8005ba34();
    w_u32(0x1F800320, r_u32(0x1F800320) | 0x10000);
    for (index = 0; index < 15; ++index)
    {
        if ((sint32)p2_func_8006a3ac(r_u32(0x8008A46C)) > 0)
        {
            offset = p2_func_8006a3c4(r_u32(0x8008A46C));
            next = r_u32(0x8008A46C) + 2;
            object = r_u32(0x8008A488) + offset;
            w_u32(0x800B7F00 + 4 * index, object);
            offset = p2_func_8006a3c4(next);
            header = r_u32(0x8008A488) + offset;
            p2_func_80069f40(header, -1);
            flags = r_u32(object + 28);
            byte = r_u8(object + 13);
            w_u32(object + 28, flags | 0x20024);
            w_u8(object + 13, byte | 0x80);
            p2_func_80069f64(object, header);
            offset = p2_func_8006a3c4(r_u32(0x8008A46C) + 1);
            p2_func_8006a170(object, r_u32(0x8008A488) + offset);
        }
        next = r_u32(0x8008A46C) + 3;
        w_u32(0x8008A46C, next);
    }
    offset = p2_func_8006a3c4(next);
    w_u32(0x800B7A30, 0);
    w_u32(0x8008A424, r_u32(0x8008A488) + offset);
    p2_func_800552fc(0);
    if (r_u32(0x8008A52C) & 0x200)
    {
        offset = p2_func_8006a3ac(41);
        next = 35;
    }
    else
    {
        offset = p2_func_8006a3ac(23);
        next = 29;
    }
    w_u32(0x800B7A34, offset);
    w_u32(0x800B7A38, p2_func_8006a3ac(next));
    p2_func_800552fc(1);
    w_u32(0x8008A4FC, 3);
    w_u32(0x800B7A3C, r_u32(0x800B7A34) + r_u32(0x800B7A38));
    p2_func_800558f0();
    for (index = 0; index < 32; index += 8)
    {
        words[0] = r_u32(0x800B7F50 + index);
        words[1] = r_u32(0x800B7F54 + index);
        w_u32(0x800B14D8 + index, words[0]);
        w_u32(0x800B14DC + index, words[1]);
    }
    w_u32(0x800BA3E8, 0x8002995C);
    return 0x8002995C;
}

uint32 p2_func_8005ba34(void)
{
    uint32 pointer = 0x8008B600;
    sint32 offset;
    FUNCTION_MARKER(0x8005BA34, "SLUS_005.78");
    for (offset = 240; offset >= 0; offset -= 8)
    {
        w_u32(0x8008B50C + (uint32)offset, pointer);
        pointer -= 8;
    }
    w_u32(0x8008B604, 0);
    w_u32(0x8008A34C, 0x8008B508);
    return 0x8008B508;
}

uint32 p2_func_800552fc(uint32 index)
{
    uint32 offset;
    uint32 object;
    uint32 header;
    uint32 model;
    uint32 result;
    FUNCTION_MARKER(0x800552FC, "SLUS_005.78");
    offset = p2_func_8006a3c4(r_u32(0x8008A46C));
    object = r_u32(0x8008A488) + offset;
    w_u32(0x8008B4B8 + 4 * index, object);
    if ((sint32)p2_func_8006a3ac(r_u32(0x8008A46C) + 4) <= 0)
        w_u32(object + 236, 0);
    else
    {
        offset = p2_func_8006a3c4(r_u32(0x8008A46C) + 4);
        w_u32(object + 236, p2_func_8006aa48(r_u32(0x8008A488) + offset));
    }
    offset = p2_func_8006a3c4(r_u32(0x8008A46C) + 3);
    header = r_u32(0x8008A488) + offset;
    p2_func_80069f40(header, -1);
    offset = p2_func_8006a3c4(r_u32(0x8008A46C) + 1);
    model = r_u32(0x8008A488) + offset;
    w_u32(object + 232, model);
    w_u32(model + 28, r_u32(model + 28) | 0x20034);
    p2_func_80069f64(r_u32(object + 232), header);
    w_u16(r_u32(r_u32(object + 232) + 156) + 40, (uint32)(r_s32(object + 60) >> 5));
    w_u16(r_u32(r_u32(object + 232) + 156) + 42, (uint32)(r_s32(object + 64) >> 5));
    offset = p2_func_8006a3c4(r_u32(0x8008A46C) + 2);
    p2_func_80069fd8(r_u32(object + 232), r_u32(0x8008A488) + offset, object + 80);
    result = r_u32(0x8008A46C) + 5;
    w_u32(0x8008A46C, result);
    return result;
}

uint32 p2_func_80069fd8(uint32 object, uint32 source, uint32 destination)
{
    uint32 remaining;
    uint32 record;
    uint32 offset;
    uint32 animation;
    uint32 pointer;
    uint32 table;
    uint32 result;
    FUNCTION_MARKER(0x80069FD8, "SLUS_005.78");
    remaining = r_u32(source) - 1;
    record = source + 4;
    do
    {
        offset = r_u32(record);
        animation = source + offset;
        if (offset != 0)
        {
            p2_func_80068294(animation, animation);
            record += 4;
            w_u32(destination, animation);
            destination += 4;
        }
        else
        {
            w_u32(destination, 0);
            destination += 4;
            record += 4;
        }
        --remaining;
    } while (remaining != 0);
    offset = r_u32(record);
    table = source + offset;
    result = offset;
    if (offset != 0)
    {
        remaining = (uint32)r_s16(table);
        result = remaining;
        if (remaining != 0)
        {
            pointer = table + 8;
            do
            {
                result = source + r_u32(pointer);
                w_u32(pointer, result);
                pointer += 8;
                --remaining;
            } while (remaining != 0);
        }
    }
    if (object != 0)
    {
        result = offset != 0 ? table : 0;
        w_u32(object + 140, result);
    }
    return result;
}

uint32 p2_func_800558f0(void)
{
    uint32 index = 0;
    uint32 offset = 0;
    uint32 object;
    uint32 flags;
    uint32 kind;
    uint32 model;
    uint32 result;
    uint32 value;
    FUNCTION_MARKER(0x800558F0, "SLUS_005.78");
    if ((r_u32(0x8008A52C) & 1) == 0)
        w_u8(0x800B999A, 0);
    w_u8(0x8008A43C, 0);
    w_u8(0x8008A344, 0);
    w_u16(0x8008A532, 0);
    do
    {
        object = 0x800ABD68 + offset;
        w_u8(object + 12, (r_u32(0x8008A52C) & 8) ? 32 : 3);
        value = (r_u32(0x8008A52C) & 4) ? 24 : 4;
        w_u8(object + 13, value);
        w_u8(object + 14, value);
        flags = r_u32(object + 704);
        w_u32(object + 20, 0);
        w_u32(object + 16, 0);
        w_u16(object + 62, 900);
        flags &= 0xFFF0FFFF;
        flags |= ((flags >> 20) & 15) << 16;
        flags &= 0xF0FFFFFF;
        flags |= 0x0F000000;
        w_u32(object + 704, flags);
        w_u32(0x8008A340 + 4 * index, 0);
        kind = (r_u32(object + 704) >> 20) & 15;
        if (r_u32(0x8008A4FC) & (kind == 0 ? 1 : 2))
        {
            model = r_u32(r_u32(0x8008B4B8 + 4 * kind) + 232);
            w_u32(model + 108, index);
        }
        if (r_u32(object + 4) & 0x100)
        {
            w_u32(object + 876, 0);
            p2_func_8005a668(object);
        }
        if (r_u32(object + 4) & 0x20)
        {
            flags = r_u32(0x8008A178);
            value = r_u32(object + 4);
            w_u32(0x8008A178, flags & 0xFFFFFFDB);
            w_u32(object + 4, value & 0xFFFFFFDF);
        }
        value = r_u32(object + 20);
        result = value >> 16;
        if ((value & 0xFFFF) != 0)
        {
            result = object + 4 * result;
            w_u32(result + 852, 0);
        }
        w_u32(object + 20, 0);
        ++index;
        offset += 928;
    } while ((sint32)index <= 0);
    return result;
}

uint32 p2_func_80030994(void)
{
    FUNCTION_MARKER(0x80030994, "SLUS_005.78");
    w_u32(0x800B7A68, 0x8002FB94);
    w_u32(0x800B7A54, 0x8002F944);
    return 0x8002F944;
}

sint32 p2_func_8006a2ac(void)
{
    FUNCTION_MARKER(0x8006A2AC, "SLUS_005.78");
    /* A pending native package transfer has already completed successfully */
    if (r_s8(0x8008A39C) != 0)
        w_u8(0x8008A39C, 0);
    return r_s8(0x8008A39C) == 0;
}

uint32 p2_func_80066e98(void)
{
    uint32 offset;
    uint32 source;
    uint32 length;
    uint32 second_length;
    uint32 third_length;
    uint32 header;
    uint32 count;
    uint32 index;
    uint32 kind;
    uint32 selected;
    uint32 destination;
    uint32 values[6];
    FUNCTION_MARKER(0x80066E98, "SLUS_005.78");
    if ((r_u32(0x8008A52C) & 0x200) == 0 && (sint32)p2_func_8006a3ac(260) > 0 && (sint32)p2_func_8006a3ac(261) > 0 && (sint32)p2_func_8006a3ac(262) > 0)
    {
        offset = p2_func_8006a3c4(260);
        source = r_u32(0x8008A4C8) + offset;
        length = p2_func_8006a3ac(260);
        second_length = p2_func_8006a3ac(261);
        third_length = p2_func_8006a3ac(262);
        length += second_length + third_length;
        p2_func_8006637c((uint8 *)psx_addr(0x800AC680, length), (const uint8 *)psx_addr(source, length), length);
    }
    w_u16(0x8008A530, 0);
    if ((sint32)p2_func_8006a3c4(263) > 0)
    {
        do
        {
            offset = p2_func_8006a3c4(263);
            source = r_u32(0x8008A4C8);
            w_u16(0x8008A530, p2_func_8006ac44(257, 0, source, source + offset));
        } while (r_u16(0x8008A530) == 0);
        p2_func_80066404();
    }
    offset = p2_func_8006a3c4(264);
    header = r_u32(0x8008A4C8) + offset;
    w_u32(0x8008A430, header);
    p2_func_80069f40(header, -1);
    header = r_u32(0x8008A430);
    count = r_u32(header);
    if (count != 0 && (sint32)(r_u32(0x800B7F50) + count) < 51)
    {
        destination = r_u32(0x800B7F54) + 16 * r_u32(0x800B7F50);
        source = r_u32(header + 4);
        p2_func_8006637c((uint8 *)psx_addr(destination, 16 * count), (const uint8 *)psx_addr(source, 16 * count), 16 * count);
        w_u32(0x800B7F50, r_u32(0x800B7F50) + r_u32(r_u32(0x8008A430)));
        header = r_u32(0x8008A430);
    }
    index = 0;
    if (r_s32(header + 24) > 0)
    {
        do
        {
            selected = 0;
            w_u32(0x800B1520 + 8 * index, r_u32(header + 28) + 20 * index);
            for (kind = 0; kind < 3; ++kind)
            {
                header = r_u32(0x8008A430);
                values[selected] = p2_func_8002a34c(r_u32(0x800B1520 + 8 * index), kind, header, r_u32(0x800B7F50) - r_u32(header));
                if (values[selected] != 0xFFFFFFFF)
                    ++selected;
            }
            if (selected != 0)
            {
                destination = p2_func_8002a3e0(0, selected);
                w_u32(0x800B1524 + 8 * index, destination);
                p2_func_8006637c((uint8 *)psx_addr(destination, 4 * selected), (const uint8 *)values, 4 * selected);
            }
            header = r_u32(0x8008A430);
            ++index;
        } while ((sint32)index < r_s32(header + 24));
    }
    offset = p2_func_8006a3c4(265);
    return p2_func_8006624c(r_u32(0x8008A4C8) + offset, 9, 1);
}

uint32 p2_func_8006ac44(uint32 asset, uint32 mode, uint32 header, uint32 samples)
{
    sint8 phase;
    uint32 offset;
    uint32 sequence;
    uint32 volume;
    uint32 result;
    FUNCTION_MARKER(0x8006AC44, "SLUS_005.78");
    phase = r_s8(0x8008A2A6);
    if (phase == 0)
    {
        offset = p2_func_8006a3c4(asset + 1);
        w_u16(0x8008A2A4, p2_sdk_80079f48(header + offset, -1, 11568));
        if (mode != 0)
        {
            p2_func_8006a1ac(mode, samples, 0);
            w_u8(0x8008A2A6, 2);
            return 0;
        }
        p2_sdk_8007a680(samples, r_s16(0x8008A2A4));
        w_u8(0x8008A2A6, 1);
        return 0;
    }
    if (phase == 2)
    {
        if (((uint32)p2_func_8006a2ac() << 24) == 0)
            return 0;
        p2_sdk_8007a680(samples, r_s16(0x8008A2A4));
        w_u8(0x8008A2A6, 1);
        return 0;
    }
    if (phase == 1 && (p2_sdk_8007a740(0) << 16) == 0)
        return 0;
    w_u8(0x8008A2A6, 0);
    offset = p2_func_8006a3c4(asset + 2);
    sequence = p2_sdk_80071c8c(header + offset, r_s16(0x8008A2A4));
    volume = (127 * r_u16(0x8008A24E)) >> 12;
    p2_sdk_80075d58((uint8)sequence, (sint16)volume, (sint16)volume);
    result = ((r_u8(0x8008A2A4) + 1) & 255) | (((sequence + 1) & 255) << 8);
    w_u16(0x8008A3AC, result);
    return result;
}

void p2_func_80066404(void)
{
    uint16 handle;
    FUNCTION_MARKER(0x80066404, "SLUS_005.78");
    handle = r_u16(0x8008A530);
    if (handle != 0)
        p2_func_8006ae10(handle);
}

uint32 p2_func_8006ae10(uint16 handle)
{
    sint32 repeat;
    FUNCTION_MARKER(0x8006AE10, "SLUS_005.78");
    p2_func_800765b0();
    p2_sdk_80070a24(1);
    p2_sdk_800763ec(1);
    p2_sdk_80076354(36, 36);
    p2_func_800765d0();
    repeat = handle == r_u16(0x8008A3B0) ? -1 : 0;
    return p2_sdk_80074264((sint32)(uint8)handle - 1, 1, repeat);
}

uint32 p2_func_800765b0(void)
{
    FUNCTION_MARKER(0x800765B0, "SLUS_005.78");
    return p2_sdk_800764a0(0);
}

uint32 p2_func_800765d0(void)
{
    FUNCTION_MARKER(0x800765D0, "SLUS_005.78");
    return p2_sdk_800764a0(1);
}

uint32 p2_func_8002a34c(uint32 group, uint32 kind, uint32 header, uint32 base)
{
    uint32 entry;
    uint32 remaining;
    uint32 index;
    uint32 result;
    FUNCTION_MARKER(0x8002A34C, "SLUS_005.78");
    entry = r_u32(header + 4);
    remaining = r_u32(header);
    for (;;)
    {
        if (r_u32(entry) == group && r_u8(entry + 4) == kind)
            break;
        --remaining;
        entry += 16;
        if (remaining == 0)
            return 0xFFFFFFFF;
    }
    index = (uint32)((sint32)(entry - r_u32(header + 4)) >> 4);
    result = (index + base) << 1;
    if (kind != 0)
        result |= (r_u8(entry + 5) & 15) << 26;
    return result;
}
