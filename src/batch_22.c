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

uint32 p2_func_8001f6ac(uint32 object_offset)
{
    if (r_u32(0x800BA3C8) == 0)
        w_u32(0x800BA3C8, 0x8001EDAC);
    w_u8(object_offset + 125, 18);
    return 18;
}

uint32 p2_func_80022358(uint32 object_offset)
{
    if (r_u32(0x800BA3B8) == 0)
        w_u32(0x800BA3B8, 0x80021E70);
    w_u8(object_offset + 125, 14);
    return 14;
}

uint32 p2_func_8003ff3c(uint32 object_offset)
{
    if (r_u32(0x800BA3B4) == 0)
        w_u32(0x800BA3B4, 0x8003FDC8);
    w_u8(object_offset + 125, 13);
    return 13;
}

uint32 p2_func_80044444(uint32 object_offset)
{
    if (r_u32(0x800BA3A0) == 0)
        w_u32(0x800BA3A0, 0x8004425C);
    w_u8(object_offset + 125, 8);
    return 8;
}

uint32 p2_func_80020c08(uint32 object_offset)
{
    w_u32(0x800BA3F8, 0x8001F6DC);
    w_u8(object_offset + 125, 30);
    w_u32(0x8008A2EC, p2_func_8005bb90(74, 80));
    w_u32(0x8008A0DC, 30);
    w_u32(0x8008A0CC, 0);
    w_u32(0x8008A0D0, 0);
    w_u32(0x8008A0D4, 0);
    w_u32(0x8008A0D8, 0);
    w_u32(0x8008A0E4, 1);
    w_u32(0x8008A0E0, 1);
    return 1;
}

uint32 p2_func_80021678(uint32 object_offset)
{
    uint32 result;
    w_u32(0x800BA3F8, 0x80020C6C);
    w_u8(object_offset + 125, 30);
    w_u32(0x8008A2FC, p2_func_8005bb90(70, 71));
    result = p2_func_8005bb90(117, 112);
    w_u32(0x8008A300, result);
    w_u32(0x8008A0F4, 0);
    return result;
}

uint32 p2_func_80021e28(uint32 object_offset)
{
    if (r_u32(0x800BA3CC) == 0)
    {
        w_u32(0x800B7A78, 0x800216CC);
        w_u32(0x800BA3CC, 0x80021888);
    }
    w_u8(object_offset + 125, 19);
    w_u8(object_offset + 124, 6);
    return 6;
}

uint32 p2_func_8002293c(uint32 object_offset)
{
    uint32 result;
    if (r_u32(0x800BA3D4) == 0)
    {
        w_u32(0x800B7A74, 0x800224AC);
        w_u32(0x800BA3D4, 0x800229A4);
    }
    result = r_u32(object_offset + 132);
    w_u8(object_offset + 125, 21);
    result = r_u32(result + 32);
    if (result != 0)
    {
        result = r_u32(object_offset + 28);
        w_u8(object_offset + 124, 5);
        result |= 0x40;
        w_u32(object_offset + 28, result);
    }
    w_u32(object_offset + 112, 0);
    return result;
}

uint32 p2_func_80027a8c(uint32 object_offset)
{
    if (r_u32(0x800BA3D0) == 0)
        w_u32(0x800BA3D0, 0x800278E8);
    w_u8(object_offset + 125, 20);
    w_u32(object_offset + 112, 0);
    return 20;
}

uint32 p2_func_8005dfb8(uint32 object_offset)
{
    uint32 result;
    if (r_u32(0x800BA3BC) == 0)
        w_u32(0x800BA3BC, 0x8005DE30);
    result = r_u32(object_offset + 132);
    w_u8(object_offset + 125, 15);
    result = r_u32(result + 32);
    if (result != 0)
    {
        result = r_u32(object_offset + 28) | 0x40;
        w_u32(object_offset + 28, result);
    }
    w_u32(object_offset + 112, 0);
    return result;
}

uint32 p2_func_8005e418(uint32 object_offset)
{
    uint32 result;
    if (r_u32(0x800BA3A8) == 0)
        w_u32(0x800BA3A8, 0x8005E010);
    result = r_u32(object_offset + 132);
    w_u8(object_offset + 125, 10);
    result = r_u32(result + 32);
    if (result != 0)
    {
        result = r_u32(object_offset + 28) | 0x40;
        w_u32(object_offset + 28, result);
    }
    return result;
}

uint32 p2_func_8005fadc(uint32 object_offset)
{
    uint32 result;
    if (r_u32(0x800BA3E0) == 0)
        w_u32(0x800BA3E0, 0x8005F510);
    result = r_u32(object_offset + 132);
    w_u8(object_offset + 125, 24);
    result = r_u32(result + 32);
    if (result != 0)
    {
        result = r_u32(object_offset + 28) | 0x400;
        w_u32(object_offset + 28, result);
    }
    result = r_u32(r_u32(object_offset + 132) + 44);
    w_u16(result + 30, 0);
    return result;
}

uint32 p2_func_80060688(uint32 object_offset)
{
    uint32 result;
    if (r_u32(0x800BA3DC) == 0)
        w_u32(0x800BA3DC, 0x80060360);
    result = r_u32(object_offset + 132);
    w_u8(object_offset + 125, 23);
    result = r_u32(result + 32);
    if (result != 0)
    {
        result = r_u32(object_offset + 28) | 0x440;
        w_u32(object_offset + 28, result);
    }
    result = r_u32(r_u32(object_offset + 132) + 44);
    w_u16(result + 30, 0);
    w_u16(object_offset + 154, 0);
    return result;
}

uint32 p2_func_8005f490(uint32 object_offset)
{
    uint32 result;
    if (r_u32(0x800BA3B0) == 0)
        w_u32(0x800BA3B0, 0x8005F154);
    w_u8(object_offset + 125, 12);
    result = p2_func_80068198() % 75 + 15;
    w_u32(object_offset + 112, result);
    return result;
}

void p2_func_80026b64(void)
{
    /* Original function returns without side effects */
}

void p2_func_8002f180(void)
{
    /* Original function returns without side effects */
}

void p2_func_80034ba0(void)
{
    /* Original function returns without side effects */
}

void p2_func_80059ad4(void)
{
    /* Original function returns without side effects */
}

void p2_func_8002d510(void)
{
    w_u32(0x8008A320, 0);
}

uint32 p2_func_80068264(void)
{
    uint32 previous = r_u32(0x8008A298);
    w_u32(0x8008A298, 1);
    return previous;
}

uint32 p2_func_8005f124(uint32 handle, sint16 volume)
{
    w_u16(0x8008A24E, volume);
    p2_func_8006a5e0(handle - 1, volume);
    return (uint32)r_s16(0x8008A24E);
}

uint32 p2_func_800352cc(void)
{
    return p2_sdk_80082a18((const char *)psx_addr(0x800101B4, 1));
}

uint32 p2_func_8002f188(uint32 destination, uint32 source, uint32 quaternion)
{
    sint32 scalar = r_s16(quaternion);
    MATRIX rotation = {0};
    sint32 x, y, z;
    uint32 xx, yy, zz, xy, xz, yz, sx, sy, sz;
    if (scalar == 0x4000)
    {
        uint32 first = r_u32(source), second = r_u32(source + 4);
        uint32 third = r_u32(source + 8), fourth = r_u32(source + 12);
        sint16 last;
        w_u32(destination, first);
        w_u32(destination + 4, second);
        w_u32(destination + 8, third);
        w_u32(destination + 12, fourth);
        last = r_s16(source + 16);
        w_u16(destination + 16, last);
        return (uint32)(sint32)last;
    }
    x = r_s16(quaternion + 2);
    y = r_s16(quaternion + 4);
    z = r_s16(quaternion + 6);
    xx = (uint32)(x * x); yy = (uint32)(y * y); zz = (uint32)(z * z);
    xy = (uint32)(x * y); xz = (uint32)(x * z); yz = (uint32)(y * z);
    sx = (uint32)(scalar * x); sy = (uint32)(scalar * y); sz = (uint32)(scalar * z);
    rotation.m[0][0] = (sint16)((sint32)(0x08000000 - (yy + zz)) >> 15);
    rotation.m[1][1] = (sint16)((sint32)(0x08000000 - (xx + zz)) >> 15);
    rotation.m[2][2] = (sint16)((sint32)(0x08000000 - (xx + yy)) >> 15);
    rotation.m[1][0] = (sint16)((sint32)(xy - sz) >> 15);
    rotation.m[0][1] = (sint16)((sint32)(xy + sz) >> 15);
    rotation.m[2][0] = (sint16)((sint32)(xz + sy) >> 15);
    rotation.m[0][2] = (sint16)((sint32)(xz - sy) >> 15);
    rotation.m[2][1] = (sint16)((sint32)(yz - sx) >> 15);
    rotation.m[1][2] = (sint16)((sint32)(yz + sx) >> 15);
    MulMatrix0((MATRIX *)psx_addr(source, 20), &rotation, (MATRIX *)psx_addr(destination, 20));
    return destination;
}

uint32 p2_func_8002f318(uint32 destination, uint32 source, sint16 angle)
{
    sint32 cosine = p2_func_8005dbb8((sint16)(1024 - angle)) >> 2;
    sint32 sine = p2_func_8005dbb8(angle) >> 2;
    w_u16(0x1F800014, cosine);
    w_u16(0x1F800020, -sine);
    w_u16(0x1F800018, sine);
    w_u16(0x1F800024, cosine);
    return p2_sdk_8007e834(source, 0x1F800014, destination);
}

uint32 p2_func_8002f3b0(uint32 destination, uint32 source, sint16 first_angle, sint16 second_angle)
{
    MATRIX temporary;
    sint32 cosine = p2_func_8005dbb8((sint16)(1024 - second_angle)) >> 2;
    sint32 sine = p2_func_8005dbb8(second_angle) >> 2;
    w_u16(0x1F800014, cosine);
    w_u16(0x1F800020, -sine);
    w_u16(0x1F800018, sine);
    w_u16(0x1F800024, cosine);
    MulMatrix0((MATRIX *)psx_addr(source, 20), (MATRIX *)psx_addr(0x1F800014, 20), &temporary);
    cosine = p2_func_8005dbb8((sint16)(1024 - first_angle)) >> 2;
    sine = p2_func_8005dbb8(first_angle) >> 2;
    w_u16(0x1F800008, cosine);
    w_u16(0x1F80000E, sine);
    w_u16(0x1F80000A, -sine);
    w_u16(0x1F800010, cosine);
    MulMatrix0(&temporary, (MATRIX *)psx_addr(0x1F800000, 20), (MATRIX *)psx_addr(destination, 20));
    return destination;
}

uint32 p2_func_8002f4c0(uint32 destination, uint32 source, sint16 first_angle, sint16 second_angle)
{
    MATRIX temporary;
    sint32 cosine = p2_func_8005dbb8((sint16)(1024 - first_angle)) >> 2;
    sint32 sine = p2_func_8005dbb8(first_angle) >> 2;
    w_u16(0x1F800008, cosine);
    w_u16(0x1F80000E, sine);
    w_u16(0x1F80000A, -sine);
    w_u16(0x1F800010, cosine);
    cosine = p2_func_8005dbb8((sint16)(1024 - second_angle)) >> 2;
    sine = p2_func_8005dbb8(second_angle) >> 2;
    w_u16(0x1F800014, cosine);
    w_u16(0x1F800020, -sine);
    w_u16(0x1F800018, sine);
    w_u16(0x1F800024, cosine);
    MulMatrix0((MATRIX *)psx_addr(0x1F800014, 20), (MATRIX *)psx_addr(0x1F800000, 20), &temporary);
    MulMatrix0((MATRIX *)psx_addr(source, 20), &temporary, (MATRIX *)psx_addr(destination, 20));
    return destination;
}

uint32 p2_func_8002f5d0(uint32 destination, uint32 source, sint16 first_angle, sint16 second_angle, sint16 third_angle)
{
    MATRIX first, second;
    sint32 cosine = p2_func_8005dbb8((sint16)(1024 - first_angle)) >> 2;
    sint32 sine = p2_func_8005dbb8(first_angle) >> 2;
    w_u16(0x1F800008, cosine);
    w_u16(0x1F80000E, sine);
    w_u16(0x1F80000A, -sine);
    w_u16(0x1F800010, cosine);
    MulMatrix0((MATRIX *)psx_addr(source, 20), (MATRIX *)psx_addr(0x1F800000, 20), &first);
    cosine = p2_func_8005dbb8((sint16)(1024 - second_angle)) >> 2;
    sine = p2_func_8005dbb8(second_angle) >> 2;
    w_u16(0x1F800014, cosine);
    w_u16(0x1F800020, -sine);
    w_u16(0x1F800018, sine);
    w_u16(0x1F800024, cosine);
    if (third_angle != 0)
    {
        MulMatrix0(&first, (MATRIX *)psx_addr(0x1F800014, 20), &second);
        cosine = p2_func_8005dbb8((sint16)(1024 - third_angle)) >> 2;
        sine = p2_func_8005dbb8(third_angle) >> 2;
        w_u16(0x1F800028, cosine);
        w_u16(0x1F80002E, sine);
        w_u16(0x1F80002A, -sine);
        w_u16(0x1F800030, cosine);
        MulMatrix0((MATRIX *)psx_addr(0x1F800028, 20), &second, (MATRIX *)psx_addr(destination, 20));
    }
    else
        MulMatrix0(&first, (MATRIX *)psx_addr(0x1F800014, 20), (MATRIX *)psx_addr(destination, 20));
    return destination;
}

uint32 p2_func_80026fb4(uint32 name_offset)
{
    uint32 index;
    for (index = 0; index < 32; ++index)
        if (p2_func_80083310(0x8008B228 + index * 16, name_offset, 3) == 0)
            return 1;
    return 0;
}

uint32 p2_func_8002a6d8(void)
{
    uint32 index;
    for (index = 0; index < 35; ++index)
    {
        uint32 slot = 0x800B170C + index * 8;
        uint32 data = r_u32(slot);
        if (data != 0)
        {
            p2_func_80064d54(data - 4);
            w_u32(slot, 0);
        }
    }
    return 0;
}

uint32 p2_func_8002b220(uint32 object_offset)
{
    uint32 count = r_u32(0x8008A320);
    uint32 type = r_u32(object_offset + 116);
    uint32 slot = 0x8008B420;
    while (count != 0)
    {
        uint32 object = r_u32(slot);
        --count;
        if (object != 0 && r_u8(object + 1) == type)
            return r_u32(object + 24);
        slot += 4;
    }
    /* Return no match instead of the original read through address zero */
    return 0;
}

uint32 p2_func_8002d51c(uint32 kind)
{
    uint32 type, count, slot;
    switch (kind)
    {
        case 15: type = 23; break;
        case 8: type = 28; break;
        case 12: type = 26; break;
        case 5: type = 30; break;
        case 31: type = 42; break;
        case 44: type = 4; break;
        default: return 0;
    }
    count = r_u32(0x8008A320);
    slot = 0x8008B420;
    while (count != 0)
    {
        uint32 object = r_u32(slot);
        --count;
        if (object != 0 && r_u8(object + 1) == type)
            return object;
        slot += 4;
    }
    return 0;
}

uint32 p2_func_8002d71c(uint32 kind, uint8 value)
{
    uint32 result = p2_func_8002d51c(kind);
    if (result != 0)
        w_u8(result + 44, value);
    return result;
}

uint32 p2_func_8005a480(uint32 character_offset, uint32 incoming_flags)
{
    sint32 index;
    if ((incoming_flags & 0x10) != 0)
    {
        for (index = 0; index < r_s8(character_offset + 448); ++index)
            p2_func_80027ac0(character_offset, 13);
        w_u8(character_offset + 448, 0);
    }
    if ((r_u32(0x8008A1A4) & 0x20) != 0)
    {
        for (index = 0; index < r_s8(character_offset + 449); ++index)
            p2_func_80027ac0(character_offset, 14);
        w_u8(character_offset + 449, 0);
    }
    incoming_flags = r_u32(0x8008A1A4) & 0xFFFFFFCF;
    w_u32(0x8008A1A4, incoming_flags);
    return incoming_flags;
}

sint32 p2_func_800309bc(uint32 object_offset)
{
    sint32 first = r_s16(object_offset + 136), second;
    uint32 definition = r_u32(object_offset + 132);
    if ((r_u32(object_offset + 28) & 0x10) != 0)
    {
        second = first + 1;
        if (first >= r_s16(definition + 26))
        {
            second = first;
            --first;
        }
    }
    else
    {
        second = first - 1;
        if (first <= 0)
        {
            second = first;
            ++first;
        }
    }
    return (sint16)p2_func_8005642c(first, second, definition);
}

sint32 p2_func_80034958(uint32 object_offset, uint32 position, uint32 target)
{
    uint32 definition = r_u32(object_offset + 132);
    uint32 saved_stack, temporary;
    sint32 count, frame, lookahead;
    sint16 path_angle, target_angle;
    if (r_u32(definition + 32) == 0)
        return 0;
    if (r_u32(position) - r_u32(target) + 4096 < 8193 &&
        r_u32(position + 8) - r_u32(target + 8) + 4096 < 8193)
        return 0;
    count = r_s16(definition + 26);
    frame = r_s16(object_offset + 136);
    lookahead = frame + count / 8;
    if (lookahead == frame || lookahead > count - 1)
        lookahead = count - 1;
    saved_stack = p2_guest_stack_pointer;
    p2_guest_stack_pointer -= 16;
    temporary = p2_guest_stack_pointer;
    p2_func_800230f0(definition, lookahead, temporary, r_u32(object_offset + 28) & 0x10);
    path_angle = (sint16)p2_func_8005dc00((sint32)(r_u32(temporary + 8) - r_u32(position + 8)), (sint32)(r_u32(temporary) - r_u32(position)));
    target_angle = (sint16)p2_func_8005dc00((sint32)(r_u32(target + 8) - r_u32(position + 8)), (sint32)(r_u32(target) - r_u32(position)));
    p2_guest_stack_pointer = saved_stack;
    return (((uint32)((sint32)path_angle - target_angle + 1024) & 4095) < 2049) ? 1 : -1;
}

uint32 p2_func_800401f0(uint32 object_offset, uint32 target, sint16 step)
{
    sint32 previous = r_s16(object_offset + 148);
    sint32 difference;
    w_u16(object_offset + 150, target);
    if (previous == 4096)
    {
        w_u16(object_offset + 148, target);
        return 4096;
    }
    difference = (sint16)(target - (uint32)previous);
    if (difference < -2048)
        difference = (sint16)(difference + 4096);
    else if (difference > 2048)
        difference = (sint16)(difference - 4096);
    if (difference < step && difference > -step)
        w_u16(object_offset + 148, target);
    else
        w_u16(object_offset + 148, r_u16(object_offset + 148) + (difference < 0 ? -step : step));
    target = r_u16(object_offset + 148) & 4095;
    w_u16(object_offset + 148, target);
    return target;
}

uint32 p2_func_800402bc(uint32 object_offset, uint32 target, sint16 step)
{
    sint32 previous = r_s16(object_offset + 152);
    sint32 difference;
    if (previous == 4096)
    {
        w_u16(object_offset + 152, target);
        return 4096;
    }
    difference = (sint16)(target - (uint32)previous);
    if (difference < -2048)
        difference = (sint16)(difference + 4096);
    else if (difference > 2048)
        difference = (sint16)(difference - 4096);
    if (difference < step && difference > -step)
        w_u16(object_offset + 152, target);
    else
        w_u16(object_offset + 152, r_u16(object_offset + 152) + (difference < 0 ? -step : step));
    target = r_u16(object_offset + 152) & 4095;
    w_u16(object_offset + 152, target);
    return target;
}

sint32 p2_func_80056660(sint32 first_frame, sint32 second_frame, uint32 path_offset)
{
    uint32 saved_stack = p2_guest_stack_pointer;
    uint32 first, second;
    sint32 result;
    p2_guest_stack_pointer -= 32;
    first = p2_guest_stack_pointer;
    second = first + 16;
    p2_func_80022b20(path_offset, first_frame, first, 64);
    p2_func_80022b20(path_offset, second_frame, second, 64);
    result = (sint16)p2_func_8005dc00((sint32)(r_u32(first + 8) - r_u32(second + 8)), (sint32)(r_u32(first) - r_u32(second)));
    p2_guest_stack_pointer = saved_stack;
    return result;
}

uint32 p2_func_80057480(uint32 object_offset, uint32 path_offset, uint32 destination)
{
    return p2_func_80022b20(path_offset, 0, destination, (r_u32(object_offset + 228) & 0x400) == 0 ? 64 : 0);
}

uint32 p2_func_8004a928(uint32 object_offset, uint32 character_offset)
{
    uint32 timer, held;
    (void)object_offset;
    w_u8(character_offset + 713, r_u8(character_offset + 712));
    held = r_u32(r_u32(0x8008A114) + 136);
    w_u32(character_offset + 8, r_u32(character_offset + 8) & 0xF83FFFFF);
    timer = (uint32)r_s16(character_offset + 54);
    w_u32(character_offset + 688, held);
    if (timer != 0)
    {
        w_u16(character_offset + 54, timer - 1);
        if ((sint16)(timer - 1) == 0)
            w_u32(character_offset, r_u32(character_offset) & 0xFFBFFFFF);
    }
    w_u32(0x1F800388, r_u32(0x1F800384));
    if ((r_u32(0x1F800320) & 0x102) != 0)
    {
        w_u32(0x1F800384, 0);
        p2_func_80066d08();
    }
    else
        w_u32(0x1F800384, p2_func_80060fdc(p2_func_80066d08()));
    held = r_u32(0x8008A174);
    if (held != 0)
    {
        uint32 input = r_u32(0x1F800384);
        w_u32(0x8008A174, held & input);
        w_u32(0x1F800384, input & ~held);
    }
    if ((sint32)p2_func_80026c14() < 0)
        w_u32(0x1F800384, r_u32(0x1F800384) & 0x900);
    return p2_func_800276b4(0x1F800384);
}

uint32 p2_func_8002ad4c(uint32 object_offset)
{
    uint32 character = 0x800ABD68;
    uint32 input, flags, saved_stack, flags_offset, result, speed, acceleration, limit;
    if ((r_u32(0x800ABD6C) & 0x400) != 0 &&
        r_s16(object_offset + 130) == r_s16(r_u32(object_offset + 104) + 6) - 1)
        w_u32(0x800ABD6C, r_u32(0x800ABD6C) & 0xFFFFFBFF);
    w_u32(object_offset + 28, r_u32(object_offset + 28) | 0x400);
    p2_func_8004a928(object_offset, character);
    input = r_u32(0x1F800384);
    flags = 0;
    speed = r_u32(character + 24);
    acceleration = r_u32(character + 756);
    if ((input & 0x2000) != 0)
        speed = (r_u32(object_offset + 28) & 0x10) != 0 ? speed + acceleration : speed - acceleration;
    else if ((input & 0x8000) != 0)
        speed = (r_u32(object_offset + 28) & 0x10) != 0 ? speed - acceleration : speed + acceleration;
    else
        speed = (uint32)((sint32)(speed * 7) / 8);
    w_u32(character + 24, speed);
    if ((sint32)speed < 0)
        flags |= 2;
    limit = r_u32(character + 756) << 2;
    if ((sint32)limit < (sint32)speed)
        w_u32(character + 24, limit);
    speed = r_u32(character + 28);
    if ((input & 0x1000) != 0)
    {
        speed += r_u32(character + 756);
        w_u32(character + 28, speed);
        limit = r_u32(character + 756) << 2;
        if ((sint32)limit < (sint32)speed)
            w_u32(character + 28, limit);
    }
    else if ((input & 0x4000) != 0)
    {
        speed -= r_u32(character + 756);
        w_u32(character + 28, speed);
        limit = (0 - r_u32(character + 756)) << 2;
        if ((sint32)speed < (sint32)limit)
            w_u32(character + 28, limit);
    }
    else
        w_u32(character + 28, (uint32)((sint32)(speed * 7) / 8));
    w_u32(character + 32, 0);
    if ((input & r_u8(r_u32(0x8008A2B4))) != 0)
        flags |= 0x40;
    saved_stack = p2_guest_stack_pointer;
    p2_guest_stack_pointer -= 8;
    flags_offset = p2_guest_stack_pointer;
    w_u32(flags_offset, flags);
    p2_func_8004d5c0(object_offset, character, flags_offset);
    p2_func_8004ca00(object_offset, character, flags_offset);
    result = p2_func_8004cba4(object_offset, character, r_u32(flags_offset));
    p2_guest_stack_pointer = saved_stack;
    return result;
}

uint32 p2_func_8006d2ec(void)
{
    return p2_func_8006bbe8(0);
}

uint32 p2_func_80071a8c(sint16 sequence)
{
    return p2_func_800718ec(sequence);
}

uint32 p2_func_80074238(sint16 sequence, sint16 track)
{
    return p2_func_80074138((uint32)(sint32)sequence, (uint32)(sint32)track);
}

void p2_func_800742d4(void)
{
    p2_sound_quit_8006febc();
}

uint32 p2_func_800743c0(sint16 sequence, sint16 track)
{
    return p2_sdk_800742f4(sequence, track);
}

void p2_func_80074770(void)
{
    p2_sdk_800744ec(0);
}

uint32 p2_func_800759a8(sint16 sequence, sint16 track)
{
    return p2_sdk_80075818(sequence, track);
}

void p2_func_800767b8(void)
{
    w_u16(0x800B1690, 0);
}

uint32 p2_func_800767c8(void)
{
    w_u16(0x800B1690, 2);
    return 2;
}

uint32 p2_func_80067778(void)
{
    sint32 remaining = 30, fraction = 30;
    uint32 fade = 273, result, delay, deadline;
    p2_func_80066978();
    w_u32(0x8008A378, r_u32(0x8008A434));
    do
    {
        uint32 style = fade < 4096 ? (fade << 16) | 1 : 1;
        p2_func_800665d4();
        p2_func_800665b0();
        p2_func_80066bdc(0, 0xF00140, 0);
        p2_func_80066af0(1);
        p2_func_80064ff4(0x7800A0, 0x8001065C, style, 0);
        p2_func_80066698();
        delay = 33;
        fraction -= 10;
        if (fraction <= 0)
        {
            delay = 34;
            fraction += 30;
        }
        deadline = r_u32(0x8008A378) + delay;
        while (r_u32(0x8008A434) < deadline)
            psx_native_poll_events();
        result = r_u32(0x8008A434);
        w_u32(0x8008A378, result);
        --remaining;
        fade += 273;
    } while (remaining != 0);
    return result;
}

uint32 p2_func_8006b04c(uint32 position, uint32 alternate, sint32 selection)
{
    uint32 group;
    if (alternate != 0)
        group = (uint32)selection + 2;
    else
    {
        switch (selection)
        {
            case 0: group = 6; break;
            case 1: group = 8; break;
            case 2: group = 7; break;
            default:
                fprintf(stderr, "P2: Invalid interface selection %d\n", selection);
                abort();
        }
    }
    return p2_func_80064e74(position, 0, r_u32(r_u32(0x8008A430) + 28) + group * 20, 0);
}

uint32 p2_func_80032048(uint32 state, uint32 object, uint32 sound, uint32 mode)
{
    uint32 previous = r_u32(state + 68), pitch, rate, random, result;
    sint32 scale = 4096;
    if (sound != previous && previous != 0)
    {
        if ((p2_func_8005e868(previous, (uint32)r_s16(state + 66)) & 255) != 0)
            p2_func_8006a970(r_u16(state + 66));
        w_u32(state + 68, 0);
        w_u16(state + 66, -1);
    }
    if (r_s16(state + 66) < 0)
    {
        rate = r_u16(sound + 4);
        if (rate == 0)
            return p2_sdk_80082a18("Monster type %d bad SFX rate.\n", r_s8(state + 37));
        random = p2_func_80068198();
        if ((rate >> 2) == 0)
        {
            fprintf(stderr, "P2: Invalid monster SFX rate %u\n", rate);
            abort();
        }
        pitch = rate + random % (rate >> 2) - (rate >> 3);
        w_u16(state + 64, pitch);
    }
    else
        pitch = r_u16(state + 64);
    if ((r_u32(object + 28) & 0x30000000) != 0)
    {
        rate = r_u16(object + 116);
        if (rate < 2048)
            rate = 2048;
        scale = (sint32)rate;
        pitch = (uint32)(((sint64)(sint32)pitch * (0x1000000 / (sint32)rate)) >> 12);
    }
    w_u32(state + 68, sound);
    result = p2_func_8005eb18(sound, scale + 4096, 0, pitch, object + 60,
        (uint32)r_s16(r_u32(state + 72) + 132) << 5, (uint32)r_s16(state + 66), mode, mode != 0);
    w_u16(state + 66, result);
    return result;
}

uint32 p2_func_8005c40c(uint32 object)
{
    uint32 table, index, sound = 0, packed, result;
    if ((r_u8(object + 14) & 2) == 0 && r_s8(object + 86) >= 0)
        p2_func_8001bb58(object);
    table = r_u32(0x8008A460);
    index = r_u32(object + 68) & 4095;
    if (table != 0 && index < r_u32(table))
        sound = table + index * 8 + 4;
    /* An invalid sound identifier must not read the null guest descriptor */
    if (sound == 0)
        return 0;
    packed = r_u32(object + 76);
    result = p2_func_8005eb18(sound, (packed >> 16) & 0x3FFF, (packed >> 8) & 255,
        r_u16(sound + 4), object + 36, (uint32)r_u16(object + 66) << 5, packed & 255, 1, 1) & 255;
    w_u32(object + 76, (r_u32(object + 76) & 0xFFFFFF00) | result);
    return result;
}

uint32 p2_func_80035494(uint32 object, uint32 position, uint32 target, uint32 distance, uint32 definition, uint32 character, uint8 allowed)
{
    sint32 range;
    uint32 angle;
    if (r_s8(0x8008A144) == 0 || r_s16(character + 658) == 0)
        return 0;
    if (r_u8(definition + 166) != 0 && allowed == 0)
        return 0;
    range = (sint32)((uint32)(sint32)r_s16(character + 660) << 5);
    if (range < r_s32(distance))
        return 0;
    angle = p2_func_8005dc00((sint32)(r_u32(target + 8) - r_u32(position + 8)), (sint32)(r_u32(target) - r_u32(position)));
    return ((r_u16(object + 148) - angle + 1024) & 4095) < 2049;
}

uint32 p2_func_8003bb18(uint32 object, uint32 position, uint32 direction, uint32 context)
{
    uint32 saved_stack, floor;
    sint32 frame, result;
    /* The original self-object branch reads an unwritten local result */
    if (object == r_u32(context + 136))
        return 0;
    frame = (sint32)p2_func_800231dc(r_u32(object + 36), position, direction, -327680);
    if (frame == -1)
        return 0;
    saved_stack = p2_guest_stack_pointer;
    p2_guest_stack_pointer -= 16;
    floor = p2_guest_stack_pointer;
    result = p2_func_8003d3fc(r_u32(object + 36), frame, position, floor, direction, 4096);
    if (result != -1 && (sint32)(r_u32(floor + 4) - r_u32(position + 4)) > 0)
        result = -1;
    p2_guest_stack_pointer = saved_stack;
    return result != -1;
}

sint32 p2_func_800574b4(uint32 path, uint32 position, uint32 floor, uint32 direction)
{
    sint32 frame, previous_frame;
    uint32 previous[3], start[3];
    frame = (sint32)p2_func_800231dc(r_u32(path + 36), position, direction, -40960);
    if (frame >= 0)
        frame = p2_func_8003d3fc(r_u32(path + 36), frame, position, floor, direction, 8192);
    previous[0] = r_u32(floor);
    previous[1] = r_u32(floor + 4);
    previous[2] = r_u32(floor + 8);
    previous_frame = frame;
    p2_func_80022b20(path, 0, floor, direction);
    start[0] = r_u32(floor);
    start[1] = r_u32(floor + 4);
    start[2] = r_u32(floor + 8);
    w_u32(position, r_u32(floor));
    w_u32(position + 4, r_u32(floor + 4));
    w_u32(position + 8, r_u32(floor + 8));
    frame = p2_func_8003d3fc(r_u32(path + 36), 0, position, floor, direction, 8192);
    if (frame < 0)
    {
        frame = 0;
        w_u32(floor, start[0]);
        w_u32(floor + 4, start[1]);
        w_u32(floor + 8, start[2]);
    }
    if (r_u32(floor) - previous[0] + 24576 > 49152 || r_u32(floor + 8) - previous[2] + 24576 > 49152)
    {
        w_u32(floor, previous[0]);
        w_u32(floor + 4, previous[1]);
        w_u32(floor + 8, previous[2]);
        return previous_frame;
    }
    return frame;
}

uint32 p2_func_8001fa08(void)
{
    w_u32(0x8008A0D0, 0);
    w_u32(0x8008A0D4, 0);
    w_u32(0x8008A0D8, 0);
    w_u32(0x8008A0E4, 1);
    return 1;
}

sint32 p2_func_80022388(uint32 object)
{
    uint32 definition = r_u32(object + 132), path, segment;
    sint32 frame;
    if (definition == 0)
        return 0;
    path = r_u32(definition + 32);
    if (path == 0)
        return 0;
    frame = r_u16(object + 136);
    if ((r_u32(object + 28) & 0x10) == 0)
        frame = (sint16)(r_u16(definition + 26) - (frame + 1) - 1);
    else
        frame = (sint16)frame;
    segment = path + r_u8(definition + 20) * 40 - 40;
    if (frame >= r_s16(definition + 24))
    {
        if ((r_u8(definition + 22) & 3) == 3)
            return 0;
    }
    else if ((uint32)frame < (r_u32(segment) >> 20))
    {
        segment = path;
        if ((uint32)frame >= (r_u32(path + 40) >> 20))
        {
            segment += 40;
            do
            {
                uint32 next_start = r_u32(segment + 40) >> 20;
                segment += 40;
                if ((uint32)frame < next_start)
                    break;
            } while (1);
            segment -= 40;
        }
    }
    return (sint32)((segment - r_u32(definition + 32)) * 0xCCCCCCCD) >> 3;
}

uint32 p2_func_800229a4(uint32 object)
{
    uint32 saved_stack = p2_guest_stack_pointer, result;
    p2_guest_stack_pointer -= 48;
    p2_func_80030dcc(object, p2_guest_stack_pointer, p2_guest_stack_pointer + 16, p2_guest_stack_pointer + 32);
    result = p2_func_80032248(object, 0, p2_guest_stack_pointer, r_u32(r_u32(r_u32(object + 132) + 44) + 72));
    p2_guest_stack_pointer = saved_stack;
    return result;
}

uint32 p2_func_800278e8(uint32 object)
{
    uint32 definition = r_u32(object + 132), state = r_u32(definition + 44);
    uint32 resource = r_u32(state + 72), saved_stack = p2_guest_stack_pointer, result;
    p2_guest_stack_pointer -= 48;
    p2_func_80030dcc(object, p2_guest_stack_pointer, p2_guest_stack_pointer + 16, p2_guest_stack_pointer + 32);
    if ((r_u32(state + 36) & 0xA0000) == 0)
    {
        if ((r_u32(r_u32(object + 132) + 16) & 0x40) != 0)
        {
            uint32 timer = r_u32(object + 112);
            if (timer != 0)
                w_u32(object + 112, --timer);
            if (timer == 0)
            {
                uint32 animation, delay;
                if ((r_u32(object + 28) & 0x40) != 0)
                {
                    animation = r_u32(resource + 72);
                    delay = 60;
                }
                else
                {
                    w_u32(r_u32(r_u32(object + 132) + 44) + 16, 0);
                    animation = r_u32(resource + 80);
                    delay = 0;
                }
                w_u32(object + 104, animation);
                w_u16(object + 130, -1);
                w_u32(object + 28, r_u32(object + 28) | 0x10000);
                if (delay == 0)
                    delay = p2_func_80068198() % 60 + 90;
                w_u32(object + 112, delay);
                w_u32(object + 28, r_u32(object + 28) ^ 0x40);
            }
        }
        else if (r_u32(object + 104) != r_u32(resource + 72))
        {
            w_u32(object + 112, 0);
            w_u32(object + 28, r_u32(object + 28) & 0xFFFFFFBF);
            w_u32(object + 104, r_u32(resource + 72));
            w_u16(object + 130, -1);
            w_u32(object + 28, r_u32(object + 28) | 0x10000);
        }
    }
    result = p2_func_80032248(object, 0, p2_guest_stack_pointer, resource);
    p2_guest_stack_pointer = saved_stack;
    return result;
}

uint32 p2_func_8003fdc8(uint32 object)
{
    uint32 saved_stack = p2_guest_stack_pointer, definition, state, resource, result;
    p2_guest_stack_pointer -= 48;
    p2_func_80030dcc(object, p2_guest_stack_pointer, p2_guest_stack_pointer + 16, p2_guest_stack_pointer + 32);
    definition = r_u32(object + 132);
    state = r_u32(definition + 44);
    resource = r_u32(state + 72);
    if ((r_u8(state + 38) & 2) == 0)
    {
        if (r_s32(resource + 4) < r_s32(p2_guest_stack_pointer + 32) || (r_u32(definition + 16) & 0x40) == 0)
        {
            if ((r_u32(object + 28) & 0x40) != 0)
            {
                w_u32(object + 28, r_u32(object + 28) & 0xFFFFFFBF);
                w_u32(object + 104, r_u32(resource + 72));
                w_u16(object + 130, -1);
                w_u32(object + 28, r_u32(object + 28) | 0x10000);
            }
        }
        else if ((r_u32(object + 28) & 0x40) == 0)
        {
            sint32 frame, previous;
            w_u32(object + 28, r_u32(object + 28) | 0x40);
            w_u32(object + 104, r_u32(resource + 80));
            w_u16(object + 130, -1);
            w_u32(object + 28, r_u32(object + 28) | 0x10000);
            frame = (sint32)p2_func_800231dc(r_u32(object + 132), p2_guest_stack_pointer + 16, r_u32(object + 28) & 0x10, r_s32(resource + 16));
            previous = r_s16(object + 136);
            if (frame < previous)
            {
                uint32 count = r_u16(r_u32(object + 132) + 26);
                w_u32(object + 28, (r_u32(object + 28) & 0xFFFFF7FF) ^ 0x10);
                w_u16(object + 136, count - previous);
            }
        }
    }
    result = p2_func_80032248(object, 0, p2_guest_stack_pointer, resource);
    p2_guest_stack_pointer = saved_stack;
    return result;
}

uint32 p2_func_8004425c(uint32 object)
{
    uint32 state = r_u32(r_u32(object + 132) + 44), resource = r_u32(state + 72);
    uint32 saved_stack = p2_guest_stack_pointer, target, animation, result;
    sint32 frame;
    p2_guest_stack_pointer -= 48;
    p2_func_80030dcc(object, p2_guest_stack_pointer, p2_guest_stack_pointer + 16, p2_guest_stack_pointer + 32);
    target = p2_guest_stack_pointer + 16;
    if ((r_u8(state + 38) & 2) != 0)
        target = 0;
    else
    {
        if ((r_u32(r_u32(object + 132) + 16) & 0x40) == 0 && r_u32(object + 104) != r_u32(resource + 72))
        {
            w_u32(object + 104, r_u32(resource + 72));
            w_u16(object + 130, -1);
            w_u32(object + 28, r_u32(object + 28) | 0x10000);
        }
        animation = r_u32(object + 104);
        frame = r_s16(object + 130);
        if (animation == r_u32(resource + 80))
        {
            if ((animation == 0 || frame >= r_s16(animation + 6) - 1) && frame > 0)
            {
                w_u16(object + 130, -1);
                w_u32(object + 104, r_u32(resource + 72));
                w_u32(object + 28, r_u32(object + 28) | 0x10000);
            }
        }
        else if ((r_u32(r_u32(object + 132) + 16) & 0x40) != 0 &&
            r_s32(resource + 4) >= r_s32(p2_guest_stack_pointer + 32) && r_s32(resource + 8) >= r_s32(p2_guest_stack_pointer + 32))
        {
            sint16 angle = (sint16)(r_u16(object + 148) - p2_func_8005dc00(
                (sint32)(r_u32(p2_guest_stack_pointer + 8) - r_u32(target + 8)),
                (sint32)(r_u32(p2_guest_stack_pointer) - r_u32(target))));
            if (angle >= -63 && angle < 64)
            {
                w_u16(object + 130, -1);
                w_u32(object + 104, r_u32(resource + 80));
                w_u32(object + 28, r_u32(object + 28) | 0x10000);
            }
        }
    }
    result = p2_func_80032248(object, target, p2_guest_stack_pointer, resource);
    p2_guest_stack_pointer = saved_stack;
    return result;
}
