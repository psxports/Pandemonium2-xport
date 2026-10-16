#include "game.h"
#include <stdlib.h>
#include "xport_trace.h"

uint32 p2_reset_graph_zero(void)
{
    uint32 address, mask, graph_type;
    sint32 result;
    printf("ResetGraph:jtb=%08x,env=%08x\n", 0x80089F3C, 0x80089F84);
    for (address = 0x80089F84; address < 0x8008A004; ++address)
        w_u8(address, 0);
    p2_reset_callback();
    mask = psx_irq_native_mask_exchange(0);
    w_u32(0x8008A08C, 0);
    w_u32(0x8008A098, mask);
    w_u32(0x8008A088, 0);
    result = ResetGraph(0);
    if (result < 0)
    {
        fprintf(stderr, "P2: Native ResetGraph failed\n");
        abort();
    }
    SetDispMask(0);
    for (address = 0x800ABC44; address < 0x800ABD44; ++address)
        w_u8(address, 0);
    for (address = 0x800B8130; address < 0x800B9930; address += 4)
        w_u32(address, 0);
    psx_irq_native_mask_exchange((uint16)r_u32(0x8008A098));
    graph_type = (uint8)GetGraphType();
    w_u8(0x80089F84, graph_type);
    w_u8(0x80089F85, 1);
    w_u16(0x80089F88, r_u16(0x8008A004 + graph_type * 4));
    w_u16(0x80089F8A, r_u16(0x8008A018 + graph_type * 4));
    for (address = 0x80089F94; address < 0x80089FF0; address += 4)
        w_u32(address, 0xFFFFFFFF);
    for (address = 0x80089FF0; address < 0x8008A004; address += 4)
        w_u32(address, 0xFFFFFFFF);
    return graph_type;
}

uint32 p2_func_8006657c(void)
{
    uint32 address;
    FUNCTION_MARKER(0x8006657C, "SLUS_005.78");
    SetDispMask(0);
    for (address = 0x80089FF0; address < 0x8008A004; address += 4)
        w_u32(address, 0xFFFFFFFF);
    w_u8(0x8008A374, 0);
    w_u32(0x8008A4B0, 0);
    w_u32(0x8008A4A8, 0);
    return p2_reset_graph_zero();
}

void p2_func_80066ce8(void)
{
    FUNCTION_MARKER(0x80066CE8, "SLUS_005.78");
    /* Remove the stray event call that inherits a freed heap pointer as its class */
}

uint32 p2_sdk_800829f8(uint32 bytes)
{
    return mallocPSX(bytes);
}

void p2_sdk_80082a08(uint32 address)
{
    freePSX(address);
}

void p2_sdk_800833a4(void)
{
    _96_remove();
}

static PsxInterruptCallback p2_sound_previous_irq0;
static uint32 p2_sound_previous_irq0_target;
static void p2_sound_irq0_80074790(void);
static void p2_sound_half_rate_800747d0(void);
static void p2_sound_dma_8006f74c(void);
static void p2_native_vblank_8007b0b8(void);

static sint32 p2_native_bios_callback(void *context, uint32 target)
{
    switch (target)
    {
        case 0x80066C7C:
            p2_func_80066c7c();
            return 1;
        case 0x80066C40:
            p2_func_80066c40();
            return 1;
        case 0x8006EBFC:
            w_u32(0x8008A434, r_u32(0x8008A434) + 1);
            return 1;
        case 0x80074820:
            p2_sdk_80074820();
            return 1;
        default:
            return 0;
    }
}

static void p2_native_vblank_8007b0b8(void)
{
    uint32 index, target;
    w_u32(0x800894F4, r_u32(0x800894F4) + 1);
    for (index = 0; index < 8; ++index)
    {
        target = r_u32(0x800894D4 + index * 4);
        if (target != 0 && !p2_native_bios_callback(NULL, target))
        {
            fprintf(stderr, "P2: Unbound VBlank callback %08X\n", target);
            abort();
        }
    }
}

uint32 p2_reset_callback(void)
{
    uint32 address, driver;
    if (r_u16(0x80088438) != 0)
        return 0;
    ResetCallbackPSX();
    psx_bios_bind_guest_callback_service(p2_native_bios_callback, NULL);
    psx_root_counter_bind(0, 0);
    w_u16(r_u32(0x800894C8), 0);
    w_u16(r_u32(0x800894C4), r_u16(r_u32(0x800894C8)));
    w_u32(r_u32(0x800894CC), 0x33333333);
    for (address = 0x80088438; address < 0x800894A0; address += 4)
        w_u32(address, 0);
    w_u32(0x80088474, 0x80089450);
    w_u16(0x80088438, 1);
    w_u32(r_u32(0x800894F8), 0x107);
    w_u32(0x800894F4, 0);
    for (address = 0x800894D4; address < 0x800894F4; address += 4)
        w_u32(address, 0);
    w_u32(0x8008843C, 0x8007B0B8);
    w_u16(0x80088468, r_u16(0x80088468) | 1);
    InterruptCallback(0, p2_native_vblank_8007b0b8);
    driver = r_u32(0x800894C0);
    w_u32(driver + 20, 0x8007B130);
    for (address = 0x80089500; address < 0x80089520; address += 4)
        w_u32(address, 0);
    w_u32(r_u32(0x800894FC), 0);
    w_u32(0x80088448, 0x8007B1D8);
    w_u16(0x80088468, r_u16(0x80088468) | 8);
    w_u32(driver + 4, 0x8007B35C);
    _96_remove();
    ExitCriticalSection();
    return 0x80088438;
}

static void p2_sound_tick(void)
{
    uint32 target = r_u32(0x80088280);
    if (target != 0x80074820)
    {
        fprintf(stderr, "P2: Unsupported sound tick callback %08X\n", target);
        abort();
    }
    p2_sdk_80074820();
}

static PsxInterruptCallback p2_sound_callback(uint32 target)
{
    switch (target)
    {
        case 0:
            return NULL;
        case 0x80074820:
            return p2_sound_tick;
        case 0x80074790:
            return p2_sound_irq0_80074790;
        case 0x800747D0:
            return p2_sound_half_rate_800747d0;
        case 0x8007B0B8:
            return p2_native_vblank_8007b0b8;
        default:
            fprintf(stderr, "P2: Unsupported sound IRQ callback %08X\n", target);
            abort();
    }
}

static void p2_sound_irq0_80074790(void)
{
    uint32 target = r_u32(0x80088284);
    PsxInterruptCallback callback;
    if (p2_sound_previous_irq0 != NULL && target == p2_sound_previous_irq0_target)
        p2_sound_previous_irq0();
    else if (target != 0)
    {
        callback = p2_sound_callback(target);
        callback();
    }
    p2_sound_tick();
}

static void p2_sound_half_rate_800747d0(void)
{
    if (r_u32(0x8008828C) == 0)
        w_u32(0x8008828C, 1);
    else
    {
        uint32 target = r_u32(0x80088280);
        w_u32(0x8008828C, 0);
        if (target != 0x80074820)
        {
            fprintf(stderr, "P2: Unsupported half-rate sound callback %08X\n", target);
            abort();
        }
        p2_sdk_80074820();
    }
}

static uint32 p2_sound_register_interrupt(uint32 interrupt, uint32 target)
{
    uint32 slot, previous, bit, mask;
    PsxInterruptCallback native_previous, native_target;
    if (interrupt != 0 && interrupt != 6)
    {
        fprintf(stderr, "P2: Unsupported sound interrupt %u\n", interrupt);
        abort();
    }
    slot = 0x8008843C + interrupt * 4;
    previous = r_u32(slot);
    if (previous == target || r_u16(0x80088438) == 0)
        return previous;
    bit = 1u << interrupt;
    w_u32(slot, target);
    mask = r_u16(0x80088468);
    w_u16(0x80088468, target != 0 ? mask | bit : mask & ~bit);
    if (interrupt == 0 && target != 0 && target == p2_sound_previous_irq0_target && p2_sound_previous_irq0 != NULL)
        native_target = p2_sound_previous_irq0;
    else
        native_target = p2_sound_callback(target);
    native_previous = InterruptCallback((sint32)interrupt, native_target);
    if (interrupt == 0 && target == 0)
    {
        p2_sound_previous_irq0 = native_previous;
        p2_sound_previous_irq0_target = previous;
    }
    return previous;
}

static void p2_sound_stop_timer_80071ab0(void)
{
    if (r_u32(0x8008827C) != 0)
        return;
    w_u8(0x80088289, 0);
    if (r_u8(0x8008828A) == 127)
        return;
    EnterCriticalSectionPSX();
    if (r_u8(0x80088288) != 0)
    {
        VSyncCallback(NULL);
        w_u8(0x80088288, 0);
    }
    else if (r_u8(0x8008828A) == 0)
    {
        p2_sound_register_interrupt(0, r_u32(0x80088284));
        w_u32(0x80088284, 0);
    }
    else
        p2_sound_register_interrupt(6, 0);
    ExitCriticalSection();
    w_u8(0x8008828A, 127);
}

static void p2_sound_register_dma_8006fe98(uint32 target)
{
    uint32 driver = r_u32(r_u32(0x800894C0) + 4);
    uint32 previous, control, value;
    if (driver != 0x8007B35C)
    {
        fprintf(stderr, "P2: Unsupported sound DMA driver %08X\n", driver);
        abort();
    }
    previous = r_u32(0x80089510);
    if (target != previous)
    {
        if (target != 0 && target != 0x8006F74C)
        {
            fprintf(stderr, "P2: Unsupported sound DMA callback %08X\n", target);
            abort();
        }
        control = r_u32(0x800894FC);
        w_u32(0x80089510, target);
        value = r_u32(control);
        value = (value & 0x00FFFFFF) | 0x00800000;
        w_u32(control, target != 0 ? value | 0x00100000 : value & ~0x00100000u);
        DMACallback(4, target != 0 ? p2_sound_dma_8006f74c : NULL);
    }
}

static void p2_sound_quit_8006febc(void)
{
    if (r_u32(0x80087D58) != 1)
        return;
    w_u32(0x80087D58, 0);
    EnterCriticalSectionPSX();
    w_u32(0x80087D94, 0);
    w_u32(0x80087D98, 0);
    p2_sound_register_dma_8006fe98(0);
    CloseEventPSX(r_u32(0x800878F0));
    DisableEventPSX(r_u32(0x800878F0));
    ExitCriticalSection();
}

void p2_func_8006af4c(void)
{
    FUNCTION_MARKER(0x8006AF4C, "SLUS_005.78");
    p2_sdk_80070bc0(0, 0x00FF0000);
    p2_sound_stop_timer_80071ab0();
    p2_sound_quit_8006febc();
    p2_sound_quit_8006febc();
}

static void p2_sound_dma_8006f74c(void)
{
    uint32 port = r_u32(0x80087D5C);
    uint32 count = 0, callback;
    w_u16(port + 426, r_u16(port + 426) & 0xFFCF);
    if ((r_u16(port + 426) & 0x30) != 0)
    {
        count = 1;
        while (count < 0xF01)
        {
            if ((r_u16(port + 426) & 0x30) == 0)
                break;
            ++count;
        }
    }
    callback = r_u32(0x80087D94);
    if (callback != 0)
    {
        fprintf(stderr, "P2: Unbound SPU transfer completion callback %08X\n", callback);
        abort();
    }
    DeliverEvent(0xF0000009, 0x20);
}

static void p2_sound_write_fifo_8006f580(uint32 source, uint32 bytes)
{
    uint32 port = r_u32(0x80087D5C);
    uint32 original_status = r_u16(port + 430) & 0x7FF;
    uint32 chunk, written, count, status;
    w_u16(port + 422, r_u16(0x80087D74));
    while (bytes != 0)
    {
        chunk = bytes < 65 ? bytes : 64;
        port = r_u32(0x80087D5C);
        for (written = 0; (sint32)written < (sint32)chunk; written += 2)
        {
            w_u16(port + 424, r_u16(source));
            source += 2;
        }
        port = r_u32(0x80087D5C);
        w_u16(port + 426, (r_u16(port + 426) & 0xFFCF) | 0x10);
        status = r_u16(r_u32(0x80087D5C) + 430) & 0x400;
        count = 1;
        while (status != 0 && count < 0xF01)
        {
            status = r_u16(r_u32(0x80087D5C) + 430) & 0x400;
            ++count;
        }
        if (status != 0)
            printf("SPU:T/O [%s]\n", "wait (wrdy H -> L)");
        bytes -= chunk;
    }
    port = r_u32(0x80087D5C);
    w_u16(port + 426, r_u16(port + 426) & 0xFFCF);
    status = r_u16(port + 430) & 0x7FF;
    count = 1;
    while (status != original_status && count < 0xF01)
    {
        status = r_u16(r_u32(0x80087D5C) + 430) & 0x7FF;
        ++count;
    }
    if (status != original_status)
        printf("SPU:T/O [%s]\n", "wait (dmaf clear/W)");
}

static void p2_sound_reset_8006f2ec(uint32 mode)
{
    uint32 control = r_u32(0x80087D6C);
    uint32 port, status, count, index;
    w_u32(control, r_u32(control) | 0xB0000);
    port = r_u32(0x80087D5C);
    w_u32(0x80087D78, 0);
    w_u32(0x80087D7C, 0);
    w_u16(0x80087D74, 0);
    w_u16(port + 384, 0);
    w_u16(port + 386, 0);
    w_u16(port + 426, 0);
    port = r_u32(0x80087D5C);
    w_u16(port + 384, 0);
    w_u16(port + 386, 0);
    status = r_u16(port + 430) & 0x7FF;
    count = 1;
    while (status != 0 && count < 0xF01)
    {
        status = r_u16(r_u32(0x80087D5C) + 430) & 0x7FF;
        ++count;
    }
    if (status != 0)
        printf("SPU:T/O [%s]\n", "wait (reset)");
    w_u32(0x80087D80, 2);
    w_u32(0x80087D84, 3);
    w_u32(0x80087D88, 8);
    w_u32(0x80087D8C, 7);
    port = r_u32(0x80087D5C);
    w_u16(port + 428, 4);
    w_u16(port + 388, 0);
    w_u16(port + 390, 0);
    w_u16(port + 396, 0xFFFF);
    w_u16(port + 398, 0xFFFF);
    w_u16(port + 408, 0);
    w_u16(port + 410, 0);
    for (index = 0; index < 10; ++index)
        w_u16(0x800B16E8 + index * 2, 0);
    if (mode == 0)
    {
        port = r_u32(0x80087D5C);
        w_u16(0x80087D74, 512);
        w_u16(port + 400, 0);
        w_u16(port + 402, 0);
        w_u16(port + 404, 0);
        w_u16(port + 406, 0);
        w_u16(port + 432, 0);
        w_u16(port + 434, 0);
        w_u16(port + 436, 0);
        w_u16(port + 438, 0);
        p2_sound_write_fifo_8006f580(0x80087D9C, 16);
        port = r_u32(0x80087D5C);
        for (index = 0; index < 24; ++index)
        {
            w_u16(port, 0);
            w_u16(port + 2, 0);
            w_u16(port + 4, 0x3FFF);
            w_u16(port + 6, 512);
            w_u16(port + 8, 0);
            w_u16(port + 10, 0);
            port += 16;
        }
        port = r_u32(0x80087D5C);
        w_u16(port + 392, 0xFFFF);
        w_u16(port + 394, 255);
        port = r_u32(0x80087D5C);
        w_u16(port + 396, 0xFFFF);
        w_u16(port + 398, 255);
    }
    port = r_u32(0x80087D5C);
    w_u32(0x80087D90, 1);
    w_u16(port + 426, 0xC000);
    w_u32(0x80087D94, 0);
    w_u32(0x80087D98, 0);
}

uint32 p2_sdk_8006f178(uint32 mode)
{
    static uint32 device_initialized;
    uint32 index, table, result;
    if (device_initialized == 0)
    {
        SpuInit();
        xport_memory_bind_mmio(0x1F801C00, 512, spu_mmio_read, spu_mmio_write, NULL);
        device_initialized = 1;
    }
    p2_reset_callback();
    p2_sound_reset_8006f2ec(mode);
    if (mode == 0)
        for (index = 0; index < 24; ++index)
            w_u16(0x80087952 - index * 2, 0xC000);
    if (r_u32(0x80087D58) == 0)
    {
        w_u32(0x80087D58, 1);
        EnterCriticalSectionPSX();
        p2_sound_register_dma_8006fe98(0x8006F74C);
        w_u32(0x800878F0, (uint32)OpenEventPSX(0xF0000009, 0x20, 0x2000, 0));
        EnableEventPSX(r_u32(0x800878F0));
        ExitCriticalSection();
    }
    table = r_u32(0x80087DC4);
    w_u32(0x800878FC, 0);
    w_u32(0x80087900, 0);
    w_u32(0x8008790C, 0);
    w_u16(0x80087910, 0);
    w_u16(0x80087912, 0);
    w_u32(0x80087914, 0);
    w_u32(0x80087918, 0);
    w_u32(0x80087904, table);
    result = r_u32(0x80087D5C) + 418;
    w_u16(result, table);
    w_u32(0x80087DB8, 0);
    w_u32(0x80087DBC, 0);
    w_u32(0x80087DC0, 0);
    w_u32(0x800878F8, 0);
    w_u32(0x80087D78, 0);
    w_u32(0x800878F4, 0);
    w_u32(0x80087920, 0);
    w_u32(0x8008791C, 0);
    w_u32(0x80087D54, 0);
    return result;
}

void p2_sdk_800744ec(uint32 mode)
{
    uint32 counter = 0xF2000002;
    uint32 target = 0x44E8;
    uint32 callback, interrupt, previous, use_vsync;
    sint32 tick_mode;
    w_u8(0x80088288, 0);
    tick_mode = r_s32(0x80088278);
    w_u8(0x8008828A, 6);
    w_u8(0x80088289, 0);
    w_u32(0x80088284, 0);
    if (tick_mode == 0)
    {
        w_u8(0x8008828A, 127);
        return;
    }
    if (tick_mode == 3)
        target = 0x89D0;
    else if (tick_mode == 5)
    {
        w_u8(0x8008828A, 0);
        if (mode == 0)
            w_u8(0x80088288, 1);
        else
        {
            counter = 0xF2000003;
            target = 1;
        }
    }
    else if (tick_mode != 2)
    {
        if (r_u32(0x8008827C) != 0)
            return;
        tick_mode = r_s32(0x80088278);
        if (tick_mode == 0)
        {
            fprintf(stderr, "P2: Sound timer division by zero at 80074604\n");
            abort();
        }
        if (tick_mode < 70)
        {
            target = (uint32)(2116800 / tick_mode);
            w_u8(0x80088289, r_u8(0x80088289) + 1);
        }
        else
            target = (uint32)(4233600 / tick_mode);
    }
    use_vsync = r_u8(0x80088288);
    EnterCriticalSectionPSX();
    if (use_vsync != 0)
    {
        callback = r_u32(0x80088280);
        VSyncCallback(p2_sound_callback(callback));
    }
    else
    {
        ResetRCnt(counter);
        SetRCnt(counter, (uint16)target, 0x1000);
        interrupt = r_u8(0x8008828A);
        if (interrupt == 0)
        {
            previous = p2_sound_register_interrupt(0, 0);
            interrupt = r_u8(0x8008828A);
            w_u32(0x80088284, previous);
            callback = 0x80074790;
        }
        else
        {
            callback = 0x800747D0;
            if (r_u8(0x80088289) == 0)
                callback = r_u32(0x80088280);
        }
        p2_sound_register_interrupt(interrupt, callback);
    }
    ExitCriticalSection();
}

uint32 p2_sdk_80074820(void)
{
    uint32 result = r_u32(0x800B1700);
    uint32 sequence = 0, separation, slot, offset, record;
    if (result == 1)
        return result;
    w_u32(0x800B1700, 1);
    p2_sdk_800767dc();
    result = (uint32)(sint32)r_s16(0x800BA408);
    if ((sint32)result > 0)
    {
        do
        {
            if ((r_u32(0x800B7F3C) & (1u << (sequence & 31))) != 0 && r_s16(0x800BA40A) > 0)
            {
                slot = 0x800B9B00 + sequence * 4;
                separation = 0;
                do
                {
                    offset = separation * 172;
                    record = r_u32(slot) + offset;
                    if ((r_u32(record + 144) & 1) != 0)
                    {
                        p2_func_800750c4((sint16)sequence, (sint16)separation);
                        if ((r_u32(r_u32(slot) + offset + 144) & 16) != 0)
                            p2_sdk_80074a98((sint16)sequence, (sint16)separation);
                        if ((r_u32(r_u32(slot) + offset + 144) & 32) != 0)
                            p2_sdk_80074d74((sint16)sequence, (sint16)separation);
                        if ((r_u32(r_u32(slot) + offset + 144) & 64) != 0)
                            p2_sdk_80075df4((sint16)sequence, (sint16)separation);
                        if ((r_u32(r_u32(slot) + offset + 144) & 128) != 0)
                            p2_sdk_80075df4((sint16)sequence, (sint16)separation);
                    }
                    if ((r_u32(r_u32(slot) + offset + 144) & 2) != 0)
                        p2_sdk_80075020((sint16)sequence, (sint16)separation);
                    if ((r_u32(r_u32(slot) + offset + 144) & 8) != 0)
                        p2_sdk_800757b0((sint16)sequence, (sint16)separation);
                    if ((r_u32(r_u32(slot) + offset + 144) & 4) != 0)
                    {
                        p2_sdk_80075818((sint16)sequence, (sint16)separation);
                        w_u32(r_u32(slot) + offset + 144, 0);
                    }
                    ++separation;
                } while ((sint32)separation < r_s16(0x800BA40A));
            }
            ++sequence;
            result = (sint32)sequence < r_s16(0x800BA408);
        } while (result != 0);
    }
    w_u32(0x800B1700, 0);
    return result;
}

uint32 p2_sdk_80074a98(sint16 sequence, sint16 separation)
{
    uint32 saved_stack = p2_guest_stack_pointer;
    uint32 slot = 0x800B9B00 + (uint32)(sint32)sequence * 4;
    uint32 offset = (uint32)(sint32)separation * 172;
    uint32 record = r_u32(slot) + offset;
    uint32 countdown = r_u32(record + 152) - 1;
    sint32 period = r_s16(record + 66);
    sint16 packed = (sint16)((uint32)(sint32)sequence | ((uint32)(sint32)separation << 8));
    uint32 local, left, right, value, elapsed, result;
    sint32 remaining;
    p2_guest_stack_pointer -= 4;
    local = p2_guest_stack_pointer;
    w_u32(record + 152, countdown);
    if (period > 0)
    {
        if (countdown % (uint32)period != 0)
            goto volume_up_return;
        value = r_u16(record + 64) - 1;
        w_u16(record + 64, value);
        if ((sint16)value < 0)
            goto volume_up_reset;
        p2_sdk_80079244(packed, local, local + 2);
        left = r_u16(local);
        remaining = r_s16(record + 64);
        if ((sint32)left + remaining >= (sint32)left + 1)
        {
            left = r_u16(local);
            right = r_u16(local + 2);
            p2_sdk_80078d1c(packed, (sint16)(left + 1), (sint16)(right + 1), 1);
        }
    }
    else if (period < 0)
    {
        value = r_u16(record + 64) + (uint32)period;
        w_u16(record + 64, value);
        if ((sint16)value < 0)
            goto volume_up_reset;
        p2_sdk_80079244(packed, local, local + 2);
        left = r_u16(local);
        period = r_s16(record + 66);
        if ((sint32)left - period >= 127)
        {
            right = r_u16(local + 2);
            if ((sint32)right - period >= 127)
                p2_sdk_80078d1c(packed, 127, 127, 1);
        }
        elapsed = r_u32(record + 148) - r_u32(record + 152);
        period = r_s16(record + 66);
        value = (uint32)(sint32)r_s16(record + 62);
        if (elapsed * (0u - (uint32)period) < value)
        {
            left = r_u16(local);
            right = r_u16(local + 2);
            p2_sdk_80078d1c(packed, (sint16)(left - (uint32)period), (sint16)(right - (uint32)period), 1);
        }
    }
    else
        goto volume_up_return;
    goto volume_up_finish;
volume_up_reset:
    p2_sdk_80078d1c(packed, 127, 127, 1);
    value = r_u32(slot) + offset;
    w_u32(value + 144, r_u32(value + 144) & ~0x10);
volume_up_finish:
    if (r_u32(record + 152) == 0 || r_s16(record + 64) == 0)
    {
        value = r_u32(slot) + offset;
        w_u32(value + 144, r_u32(value + 144) & ~0x10);
    }
volume_up_return:
    result = p2_sdk_80079244(packed, record + 120, record + 122);
    p2_guest_stack_pointer = saved_stack;
    return result;
}

uint32 p2_sdk_80074d74(sint16 sequence, sint16 separation)
{
    uint32 saved_stack = p2_guest_stack_pointer;
    uint32 slot = 0x800B9B00 + (uint32)(sint32)sequence * 4;
    uint32 offset = (uint32)(sint32)separation * 172;
    uint32 record = r_u32(slot) + offset;
    uint32 countdown = r_u32(record + 152) - 1;
    sint32 period = r_s16(record + 66);
    sint16 packed = (sint16)((uint32)(sint32)sequence | ((uint32)(sint32)separation << 8));
    uint32 local, left, right, value, elapsed, result;
    sint32 remaining, magnitude;
    p2_guest_stack_pointer -= 4;
    local = p2_guest_stack_pointer;
    w_u32(record + 152, countdown);
    if (period > 0)
    {
        if (countdown % (uint32)period != 0)
            goto volume_down_return;
        value = r_u16(record + 64) - 1;
        w_u16(record + 64, value);
        if ((sint16)value <= 0)
            goto volume_down_clear;
        p2_sdk_80079244(packed, local, local + 2);
        left = r_u16(local);
        remaining = r_s16(record + 64);
        if ((sint32)left - remaining <= 0)
            goto volume_down_minimum;
        right = r_u16(local + 2);
        if ((sint32)right - remaining <= 0 || left == 1)
            goto volume_down_minimum;
        left = r_u16(local);
        right = r_u16(local + 2);
        p2_sdk_80078d1c(packed, (sint16)(left - 1), (sint16)(right - 1), 1);
    }
    else
    {
        value = r_u16(record + 64) + (uint32)period;
        w_u16(record + 64, value);
        if ((sint16)value <= 0)
            goto volume_down_clear;
        p2_sdk_80079244(packed, local, local + 2);
        elapsed = r_u32(record + 148) - r_u32(record + 152);
        period = r_s16(record + 66);
        magnitude = -period;
        value = (uint32)(sint32)r_s16(record + 62);
        if (value < elapsed * (uint32)magnitude)
            goto volume_down_minimum;
        left = r_u16(local);
        if (magnitude >= (sint32)left)
            goto volume_down_minimum;
        left = r_u16(local);
        right = r_u16(local + 2);
        p2_sdk_80078d1c(packed, (sint16)(left + (uint32)period), (sint16)(right + (uint32)period), 1);
    }
    goto volume_down_finish;
volume_down_minimum:
    p2_sdk_80078d1c(packed, 1, 1, 1);
    goto volume_down_finish;
volume_down_clear:
    value = r_u32(slot) + offset;
    w_u32(value + 144, r_u32(value + 144) & ~0x20);
volume_down_finish:
    if (r_u32(record + 152) == 0 || r_s16(record + 64) == 0)
    {
        value = r_u32(slot) + offset;
        w_u32(value + 144, r_u32(value + 144) & ~0x20);
    }
volume_down_return:
    result = p2_sdk_80079244(packed, record + 120, record + 122);
    p2_guest_stack_pointer = saved_stack;
    return result;
}

uint32 p2_sdk_80075020(sint16 sequence, sint16 separation)
{
    uint32 slot = 0x800B9B00 + (uint32)(sint32)sequence * 4;
    uint32 offset = (uint32)(sint32)separation * 172;
    uint32 record = r_u32(slot) + offset;
    uint32 result;
    sint16 packed = (sint16)((uint32)(sint32)sequence | ((uint32)(sint32)separation << 8));
    p2_sdk_80079350(packed);
    w_u8(record + 43, 0);
    record = r_u32(slot) + offset;
    result = r_u32(record + 144) & ~2;
    w_u32(record + 144, result);
    return result;
}

uint32 p2_sdk_800757b0(sint16 sequence, sint16 separation)
{
    uint32 slot = 0x800B9B00 + (uint32)(sint32)sequence * 4;
    uint32 offset = (uint32)(sint32)separation * 172;
    uint32 record = r_u32(slot) + offset;
    w_u8(record + 43, 1);
    record = r_u32(slot) + offset;
    w_u32(record + 144, r_u32(record + 144) & ~8);
    return record;
}

uint32 p2_sdk_80075df4(sint16 sequence, sint16 separation)
{
    uint32 slot = 0x800B9B00 + (uint32)(sint32)sequence * 4;
    uint32 offset = (uint32)(sint32)separation * 172;
    uint32 record = r_u32(slot) + offset;
    uint32 countdown = r_u32(record + 160) - 1;
    sint32 period = r_s16(record + 68);
    uint32 current, target, value, remainder, numerator, denominator;
    w_u32(record + 160, countdown);
    if (period > 0)
    {
        remainder = countdown % (uint32)period;
        if (remainder != 0)
            return remainder;
        current = r_u32(record + 140);
        target = r_u32(record + 164);
        if (target < current)
            w_u32(record + 140, current - 1);
        else if (current < target)
            w_u32(record + 140, current + 1);
    }
    else
    {
        current = r_u32(record + 140);
        target = r_u32(record + 164);
        if (target < current)
        {
            value = current + (uint32)period;
            w_u32(record + 140, value);
            if (value < target)
                w_u32(record + 140, target);
        }
        else if (current < target)
        {
            value = current - (uint32)period;
            target = r_u32(record + 164);
            w_u32(record + 140, value);
            if (target < value)
                w_u32(record + 140, target);
        }
    }
    numerator = (uint32)(sint32)r_s16(record + 74) * r_u32(record + 140);
    denominator = r_u32(0x800B9930) * 60;
    numerator *= 10;
    if (denominator == 0)
    {
        fprintf(stderr, "P2: SEQ tempo ramp division by zero at 80075F1C\n");
        abort();
    }
    value = numerator / denominator;
    w_u16(record + 112, value);
    if ((sint16)value <= 0)
        w_u16(record + 112, 1);
    if (r_u32(record + 160) != 0)
    {
        current = r_u32(record + 140);
        target = r_u32(record + 164);
        if (current != target)
            return target;
    }
    record = r_u32(slot) + offset;
    w_u32(record + 144, r_u32(record + 144) & ~0x40);
    record = r_u32(slot) + offset;
    w_u32(record + 144, r_u32(record + 144) & ~0x80);
    return record;
}

uint32 p2_sdk_80075bb4(uint32 mode)
{
    uint32 video = p2_func_8007b448();
    sint32 selected;
    uint32 result;
    if ((mode & 0x1000) != 0)
    {
        w_u32(0x8008827C, 1);
        w_u32(0x80088278, mode & 0xFFF);
    }
    else
    {
        w_u32(0x8008827C, 0);
        w_u32(0x80088278, mode);
    }
    selected = r_s32(0x80088278);
    if (selected >= 6)
    {
        w_u32(0x800B9930, (uint32)selected);
        return 0;
    }
    switch (selected)
    {
        case 4:
            w_u32(0x800B9930, 50);
            w_u32(0x80088278, video == 1 ? 5 : 50);
            return 5;
        case 1:
            w_u32(0x800B9930, 60);
            result = video == 0 ? 5 : 60;
            w_u32(0x80088278, result);
            return result;
        case 3:
            w_u32(0x800B9930, 120);
            return 120;
        case 2:
            w_u32(0x800B9930, 240);
            return 240;
        case 0:
        case 5:
            result = video == 1 ? 50 : 60;
            w_u32(0x800B9930, result);
            return result;
        default:
            w_u32(0x800B9930, 60);
            return 60;
    }
}
