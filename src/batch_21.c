#include "game.h"
#include <stdlib.h>

uint32 p2_sdk_800767dc(void)
{
    uint32 ring = r_u32(0x800B8128);
    uint32 capacity = r_u8(0x800B7A40);
    uint32 history, voice, record, registers, value, quiet = 0xFFFFFFFF;
    uint32 buffered, off_low, off_high, on_low, on_high, reverb_low, reverb_high;
    ring = (ring + 1) & 15;
    w_u32(0x800B8128, ring);
    history = 0x800BA4D8 + ring * 4;
    w_u32(history, 0);
    if (capacity != 0)
    {
        registers = r_u32(0x80088290);
        for (voice = 0; voice < capacity; ++voice)
        {
            record = 0x800AC1A6 + voice * 52;
            w_u16(record, r_u16(registers + voice * 16 + 12));
            if (r_u16(record) == 0)
                w_u32(history, r_u32(history) | (1u << (voice & 31)));
        }
    }
    if (r_u8(0x800B8124) == 0)
    {
        for (voice = 0; voice < 15; ++voice)
            quiet &= r_u32(0x800BA4D8 + voice * 4);
        voice = 0;
        if (r_u8(0x800B7A40) != 0)
        {
            do
            {
                if ((quiet & (1u << (voice & 31))) != 0)
                {
                    record = 0x800AC1BB + voice * 52;
                    if (r_u8(record) == 2)
                        p2_sdk_80076b94(0, 0xFFFFFF);
                    w_u8(record, 0);
                }
                ++voice;
            } while ((sint32)voice < r_u8(0x800B7A40));
        }
    }
    value = r_u16(0x800BA518);
    on_low = r_u16(0x800ABD48);
    off_high = r_u16(0x800BA51A);
    w_u16(0x800ABD48, on_low & ~value);
    w_u16(0x800ABD4A, r_u16(0x800ABD4A) & ~off_high);
    for (voice = 0; voice < 24; ++voice)
    {
        record = 0x800AC1A0 + voice * 52;
        if (r_s16(record + 28) != 0)
        {
            /* TODO: Resolve the original volume modulation callback */
            fprintf(stderr, "P2: Unresolved SPU volume callback %08X for voice %u\n", r_u32(0x800B8120), voice);
            abort();
        }
        if (r_s16(record + 40) != 0)
        {
            /* TODO: Resolve the original pitch modulation callback */
            fprintf(stderr, "P2: Unresolved SPU pitch callback %08X for voice %u\n", r_u32(0x800B7F4C), voice);
            abort();
        }
    }
    for (voice = 0; voice < 24; ++voice)
    {
        buffered = 0x800B7FA0 + voice * 16;
        if ((r_u8(0x800B14F8 + voice) & 1) != 0)
        {
            registers = r_u32(0x80088290) + voice * 16;
            w_u16(registers, r_u16(buffered));
            w_u16(registers + 2, r_u16(buffered + 2));
        }
        if ((r_u8(0x800B14F8 + voice) & 4) != 0)
        {
            registers = r_u32(0x80088290) + voice * 16;
            w_u16(registers + 4, r_u16(buffered + 4));
        }
        if ((r_u8(0x800B14F8 + voice) & 8) != 0)
        {
            registers = r_u32(0x80088290) + voice * 16;
            w_u16(registers + 6, r_u16(buffered + 6));
        }
        if ((r_u8(0x800B14F8 + voice) & 16) != 0)
        {
            registers = r_u32(0x80088290) + voice * 16;
            w_u16(registers + 8, r_u16(buffered + 8));
            w_u16(registers + 10, r_u16(buffered + 10));
        }
        w_u8(0x800B14F8 + voice, 0);
    }
    registers = r_u32(0x80088290);
    off_low = r_u16(0x800BA518);
    off_high = r_u16(0x800BA51A);
    on_low = r_u16(0x800ABD48);
    on_high = r_u16(0x800ABD4A);
    reverb_low = r_u16(0x800ABD4C);
    reverb_high = r_u16(0x800ABD4E);
    w_u16(0x800BA518, 0);
    w_u16(0x800BA51A, 0);
    w_u16(0x800ABD48, 0);
    w_u16(0x800ABD4A, 0);
    w_u16(registers + 396, off_low);
    w_u16(registers + 398, off_high);
    w_u16(registers + 392, on_low);
    w_u16(registers + 394, on_high);
    w_u16(registers + 408, reverb_low);
    w_u16(registers + 410, reverb_high);
    return registers;
}

uint32 p2_sdk_800770d0(sint32 count, uint32 table)
{
    uint32 shift;
    if (count <= 0)
        return 0;
    shift = r_u32(0x80087D84);
    w_u32(table, 0x40001010);
    w_u32(0x80087DC0, table);
    w_u32(0x80087DBC, 0);
    w_u32(0x80087DB8, (uint32)count);
    w_u32(table + 4, (0x10000u << (shift & 31)) - 0x1010);
    return (uint32)count;
}

uint32 p2_sdk_80076dc0(uint8 voices)
{
    static const uint8 clear_fields[] = {28, 30, 32, 34, 40, 42, 44, 46, 48, 36};
    uint32 index, record, registers, field;
    p2_func_80077124(0);
    w_u16(0x800B1690, 0);
    p2_sdk_800770d0(32, 0x800A9750);
    for (index = 0; index < 192; ++index)
        w_u16(0x800B7FA0 + index * 2, 0);
    for (index = 0; index < 24; ++index)
        w_u8(0x800B14F8 + index, 0);
    w_u16(0x800BA450, 0);
    for (index = 0; index < 16; ++index)
        w_u8(0x800B7F90 + index, 0);
    w_u8(0x800B7A40, voices < 24 ? voices : 24);
    index = 0;
    if (r_u8(0x800B7A40) != 0)
    {
        do
        {
            record = 0x800AC1A0 + (index & 0xFFFF) * 52;
            w_u16(record + 2, 24);
            w_u16(record + 14, 0xFFFF);
            w_u16(record, 255);
            w_u8(record + 27, 0);
            w_u16(record + 4, 0);
            w_u16(record + 6, 0);
            w_u16(record + 16, 0);
            w_u16(record + 18, 0);
            w_u16(record + 20, 255);
            w_u16(record + 8, 0);
            w_u8(record + 10, 64);
            for (field = 0; field < sizeof(clear_fields); ++field)
                w_u16(record + clear_fields[field], 0);
            registers = r_u32(0x80088290) + (uint16)(index * 8) * 2;
            w_u16(registers + 6, 512);
            w_u16(registers + 4, 4096);
            w_u16(registers + 8, 0x80FF);
            w_u16(registers, 0);
            w_u16(registers + 2, 0);
            w_u16(registers + 10, 0x4000);
            w_u16(0x800B7F8A, index);
            p2_sdk_80078474();
            ++index;
        } while ((index & 0xFFFF) < r_u8(0x800B7A40));
    }
    w_u16(0x800B14B8, 0x3FFF);
    w_u16(0x800B14BA, 0x3FFF);
    w_u16(0x800ABD48, 0);
    w_u16(0x800ABD4A, 0);
    w_u16(0x800BA518, 0);
    w_u16(0x800ABD4C, 0);
    w_u16(0x800ABD4E, 0);
    w_u32(0x800B14B0, 0);
    w_u32(0x800B14B4, 0);
    w_u8(0x800B8124, 0);
    w_u16(0x800B16FC, 0);
    w_u16(0x800B1704, 128);
    return p2_sdk_800767dc();
}

uint32 p2_sdk_80071b74(void)
{
    uint32 voice, field, record;
    for (voice = 0; voice < 24; ++voice)
    {
        for (field = 0; field < 8; ++field)
            w_u16(0x1F801C00 + voice * 16 + field * 2, r_u16(0x80088248 + field * 2));
    }
    for (field = 0; field < 16; ++field)
        w_u16(0x1F801D80 + field * 2, r_u16(0x80088258 + field * 2));
    p2_sdk_80076dc0(24);
    for (voice = 0; voice < 32; ++voice)
    {
        record = 0x800B9B80 + voice * 64;
        for (field = 16; field != 0; --field)
            w_u32(record + (field - 1) * 4, 0);
    }
    w_u32(0x800B9930, 60);
    w_u32(0x800B7F3C, 0);
    w_u32(0x800B1700, 0);
    return 60;
}

uint32 p2_sdk_80077168(sint16 sequence, sint16 bank, sint16 program, uint16 note, uint16 velocity, uint16 pan)
{
    uint32 packed = (uint16)sequence;
    uint32 record = r_u32(0x800B9B00 + (packed & 255) * 4) + (packed >> 8) * 172;
    uint32 saved_stack, samples, tones, count, index, data, voice, result = 0;
    uint32 program_data, value;
    sint32 scaled;
    if (p2_sdk_80079a18((uint16)bank, program) != 0)
        return (uint32)-1;
    w_u16(0x800B7F86, packed);
    w_u8(0x800B7F72, note);
    w_u8(0x800B7F73, 0);
    if (sequence == 33)
        w_u8(0x800B7F74, velocity);
    else
    {
        value = r_u8(record + 18);
        scaled = (sint32)velocity * r_s16(record + 78 + value * 2);
        w_u8(0x800B7F74, scaled / 127);
    }
    w_u8(0x800B7F75, pan);
    program_data = r_u32(0x800B1820) + (uint32)(sint32)program * 16;
    w_u8(0x800B7F7A, r_u8(program_data + 1));
    w_u8(0x800B7F7B, r_u8(program_data + 4));
    value = r_u8(program_data);
    data = r_u32(0x800B1828);
    w_u8(0x800B7F70, value);
    value = r_u8(0x800B7F77);
    if (value >= r_u16(data + 18))
        return (uint32)-1;
    if (velocity == 0)
        return p2_sdk_800776bc(sequence, bank, program, note);
    saved_stack = p2_guest_stack_pointer;
    p2_guest_stack_pointer -= 256;
    samples = p2_guest_stack_pointer;
    tones = samples + 128;
    count = p2_sdk_800793ec(tones, samples) & 255;
    index = 0;
    if (count != 0)
    {
        do
        {
            w_u16(0x800B7F88, r_u8(samples + (index & 255)));
            w_u8(0x800B7F7C, r_u8(tones + (index & 255)));
            value = 16 * r_u8(0x800B7F77) + r_u8(0x800B7F7C);
            data = r_u32(0x800B182C) + value * 32;
            w_u8(0x800B7F7F, r_u8(data));
            w_u8(0x800B7F7D, r_u8(data + 2));
            w_u8(0x800B7F7E, r_u8(data + 3));
            w_u8(0x800B7F80, r_u8(data + 4));
            w_u8(0x800B7F81, r_u8(data + 5));
            w_u8(0x800B7F84, r_u8(data + 1));
            w_u8(0x800B7F82, r_u8(data + 6));
            w_u8(0x800B7F83, r_u8(data + 7));
            voice = p2_sdk_80077930() & 255;
            value = r_u8(0x800B7A40);
            w_u16(0x800B7F8A, voice);
            if (voice >= value)
                result = (uint32)-1;
            else
            {
                w_u8(0x800AC1BB + voice * 52, 1);
                voice = (uint32)(sint32)r_s16(0x800B7F8A);
                w_u16(0x800AC1A2 + voice * 52, 0);
                voice = (uint32)(sint32)r_s16(0x800B7F8A);
                w_u16(0x800AC1AE + voice * 52, packed);
                voice = (uint32)(sint32)r_s16(0x800B7F8A);
                w_u16(0x800AC1B6 + voice * 52, r_u8(0x800B7F71));
                voice = (uint32)(sint32)r_s16(0x800B7F8A);
                w_u16(0x800AC1B0 + voice * 52, r_u8(0x800B7F77));
                voice = (uint32)(sint32)r_s16(0x800B7F8A);
                w_u16(0x800AC1B2 + voice * 52, (uint16)program);
                if (sequence != 33)
                {
                    voice = (uint32)(sint32)r_s16(0x800B7F8A);
                    w_u16(0x800AC1A8 + voice * 52, velocity);
                }
                voice = (uint32)(sint32)r_s16(0x800B7F8A);
                w_u8(0x800AC1AA + voice * 52, pan);
                voice = (uint32)(sint32)r_s16(0x800B7F8A);
                w_u16(0x800AC1B4 + voice * 52, r_u8(0x800B7F7C));
                voice = (uint32)(sint32)r_s16(0x800B7F8A);
                w_u16(0x800AC1AC + voice * 52, note);
                voice = (uint32)(sint32)r_s16(0x800B7F8A);
                w_u16(0x800AC1B8 + voice * 52, r_u8(0x800B7F7F));
                voice = (uint32)(sint32)r_s16(0x800B7F8A);
                w_u16(0x800AC1A0 + voice * 52, r_u16(0x800B7F88));
                p2_sdk_80077b98();
                if (r_s16(0x800B7F88) == 255)
                    p2_sdk_80077f60(r_u8(0x800B7F8A));
                else
                {
                    value = p2_sdk_80077dac();
                    p2_sdk_80078544(count, (sint16)value);
                }
                voice = (uint32)(sint32)r_s16(0x800B7F8A);
                result |= 1u << (voice & 31);
            }
            ++index;
        } while ((index & 255) < count);
    }
    p2_guest_stack_pointer = saved_stack;
    return result;
}

uint32 p2_sdk_80073bf0(sint16 sequence, sint16 separation, uint8 note, uint8 velocity)
{
    uint32 record = r_u32(0x800B9B00 + 4 * (uint32)(sint32)sequence) + 172 * (uint32)(sint32)separation;
    uint32 channel = r_u8(record + 18);
    uint32 result = (uint32)((r_s16(record + 170) >> (channel & 31)) & 1);
    uint32 pan = r_u8(record + channel + 23);
    sint16 packed;
    if (result != 0)
        return result;
    result = r_u16(record + 116);
    if (result == 0)
        return velocity;
    packed = (sint16)((uint32)(sint32)sequence | ((uint32)(sint32)separation << 8));
    if (velocity != 0)
    {
        result = p2_sdk_80077168(packed, r_s16(record + 76), (sint16)r_u8(record + channel + 44), note, velocity, (uint16)pan);
        w_u16(record + 168, velocity);
        return result;
    }
    return p2_sdk_800776bc(packed, r_s16(record + 76), (sint16)r_u8(record + channel + 44), note);
}

uint32 p2_sdk_80077f60(uint8 voice)
{
    uint32 packed = r_u16(0x800B7F86);
    uint32 record = r_u32(0x800B9B00 + (packed & 255) * 4) + (packed >> 8) * 172;
    uint32 track_left = r_u16(record + 116);
    uint32 program_volume = r_u8(0x800B7F7A);
    uint32 left = track_left * 129 * program_volume;
    uint32 right = r_u16(record + 118) * 129 * program_volume;
    uint32 tone_volume = r_u8(0x800B7F7D);
    uint32 control, pan, registers, index, low, high, value, upper, pending;
    left = (left / 127) * tone_volume;
    right = (right / 127) * tone_volume;
    control = r_u16(r_u32(0x80088290) + 426);
    left /= 127;
    right /= 127;
    pan = r_u8(0x800B7F7E);
    if (pan < 64)
        right = (right * pan) / 63;
    else
        left = (left * (127 - pan)) / 63;
    pan = r_u8(0x800B7F7B);
    if (pan < 64)
        right = (right * pan) / 63;
    else
        left = (left * (127 - pan)) / 63;
    pan = r_u8(0x800B7F75);
    if (pan < 64)
        right = (right * pan) / 63;
    else
        left = (left * (127 - pan)) / 63;
    if (r_s16(0x800B16FC) == 1)
    {
        if (left < right)
            left = right;
        else
            right = left;
    }
    value = r_u8(0x800B7F72) - r_u8(0x800B7F80);
    control = (control & 0xC0FF) | ((value & 63) << 8);
    registers = r_u32(0x80088290);
    w_u16(registers + 426, control);
    w_u16(0x800B7FA2 + (uint32)voice * 16, right);
    value = r_u8(0x800B14F8 + voice);
    w_u16(0x800B7FA0 + (uint32)voice * 16, left);
    w_u8(0x800B14F8 + voice, value | 3);
    low = voice < 16 ? 1u << (voice & 31) : 0;
    high = voice < 16 ? 0 : 1u << ((voice - 16) & 31);
    value = r_u8(0x800B7A40);
    w_u16(0x800AC1A4 + (uint32)voice * 52, 10);
    if (value != 0)
    {
        index = 0;
        do
        {
            record = 0x800AC1BB + (uint32)(sint32)(sint16)index * 52;
            w_u8(record, r_u8(record) & 1);
            ++index;
        } while ((sint16)index < r_u8(0x800B7A40));
    }
    w_u8(0x800AC1BB + (uint32)voice * 52, 2);
    value = r_u16(0x800ABD48);
    upper = r_u16(0x800ABD4A);
    pending = r_u16(0x800BA518);
    value |= low;
    upper |= high;
    w_u16(0x800ABD48, value);
    pending &= ~value;
    w_u16(0x800ABD4A, upper);
    w_u16(0x800BA518, pending);
    pending = r_u16(0x800BA51A);
    value = r_u8(0x800B7F84) & 4;
    w_u16(0x800BA51A, pending & ~upper);
    if (value != 0)
    {
        value = r_u16(0x800ABD4C);
        upper = r_u16(0x800ABD4E);
        w_u16(0x800ABD4C, value | low);
        w_u16(0x800ABD4E, upper | high);
    }
    else
    {
        w_u16(0x800ABD4C, r_u16(0x800ABD4C) & ~low);
        w_u16(0x800ABD4E, r_u16(0x800ABD4E) & ~high);
    }
    registers = r_u32(0x80088290);
    w_u16(registers + 404, low);
    w_u16(registers + 406, high);
    return registers;
}

uint32 p2_sdk_80078544(uint32 layers, sint16 pitch)
{
    uint32 header_volume = r_u8(r_u32(0x800B1828) + 24);
    uint32 gain = r_u8(0x800B7F74) * (header_volume * 16383);
    uint32 packed, record, left, right, pan, index, voice, low, high, value, upper;
    sint32 signed_voice;
    gain /= 16129;
    gain *= r_u8(0x800B7F7A);
    gain *= r_u8(0x800B7F7D);
    signed_voice = r_s16(0x800B7F8A);
    index = (uint16)((uint32)signed_voice * 8);
    gain /= 16129;
    packed = r_u16(0x800B7F86);
    record = r_u32(0x800B9B00 + (packed & 255) * 4) + (packed >> 8) * 172;
    left = gain;
    right = gain;
    if ((sint16)packed != 33)
    {
        left = (gain * r_u16(record + 116)) / 127;
        right = (gain * r_u16(record + 118)) / 127;
    }
    pan = r_u8(0x800B7F7E);
    if (pan < 64)
        right = (right * pan) / 63;
    else
        left = (left * (127 - pan)) / 63;
    pan = r_u8(0x800B7F7B);
    if (pan < 64)
        right = (right * pan) / 63;
    else
        left = (left * (127 - pan)) / 63;
    pan = r_u8(0x800B7F75);
    if (pan < 64)
        right = (right * pan) / 63;
    else
        left = (left * (127 - pan)) / 63;
    if (r_s16(0x800B16FC) == 1)
    {
        if (left < right)
            left = right;
        else
            right = left;
    }
    left = (left * left) / 16383;
    right = (right * right) / 16383;
    w_u16(0x800B7FA4 + index * 2, (uint16)pitch);
    w_u16(0x800B7FA0 + index * 2, left);
    w_u16(0x800B7FA2 + index * 2, right);
    voice = (uint32)(sint32)r_s16(0x800B7F8A);
    w_u8(0x800B14F8 + voice, r_u8(0x800B14F8 + voice) | 7);
    voice = (uint32)(sint32)r_s16(0x800B7F8A);
    w_u16(0x800AC1A4 + voice * 52, (uint16)pitch);
    voice = (uint32)(sint32)r_s16(0x800B7F8A);
    w_u8(0x800AC1BB + voice * 52, 1);
    signed_voice = r_s16(0x800B7F8A);
    low = signed_voice < 16 ? 1u << ((uint32)signed_voice & 31) : 0;
    high = signed_voice < 16 ? 0 : 1u << (((uint32)signed_voice - 16) & 31);
    if ((r_u8(0x800B7F84) & 4) != 0)
    {
        value = r_u16(0x800ABD4C);
        upper = r_u16(0x800ABD4E);
        w_u16(0x800ABD4C, value | low);
        w_u16(0x800ABD4E, upper | high);
    }
    else
    {
        w_u16(0x800ABD4C, r_u16(0x800ABD4C) & ~low);
        w_u16(0x800ABD4E, r_u16(0x800ABD4E) & ~high);
    }
    value = r_u16(0x800ABD48);
    upper = r_u16(0x800ABD4A);
    left = r_u16(0x800BA518);
    value |= low;
    w_u16(0x800ABD48, value);
    w_u16(0x800BA518, left & ~value);
    right = r_u16(0x800BA51A);
    upper |= high;
    w_u16(0x800ABD4A, upper);
    value = right & ~upper;
    w_u16(0x800BA51A, value);
    return (uint16)value;
}

uint32 p2_sdk_80076b94(uint32 mode, uint32 voices)
{
    uint32 base, high, low, previous, queued, result;
    base = (r_u32(0x80087D54) & 1) != 0 ? 0x800B1560 : r_u32(0x80087D5C);
    high = r_u16(base + 406);
    low = r_u16(base + 404);
    previous = low | ((high & 255) << 16);
    if (mode != 0 && mode != 1)
        return previous & 0xFFFFFF;
    queued = r_u32(0x80087D54) & 1;
    base = queued != 0 ? 0x800B1560 : r_u32(0x80087D5C);
    low = r_u16(base + 404);
    w_u16(base + 404, mode == 1 ? low | voices : low & ~voices);
    high = r_u16(base + 406);
    high = mode == 1 ? high | ((voices >> 16) & 255) : high & ~((voices >> 16) & 255);
    w_u16(base + 406, high);
    if (queued != 0)
        w_u32(0x80087920, r_u32(0x80087920) | 4);
    result = mode == 1 ? previous | (voices & 0xFFFFFF) : previous & ~(voices & 0xFFFFFF);
    return result & 0xFFFFFF;
}

uint32 p2_sdk_80077930(void)
{
    uint32 selected = 99, minimum_envelope = 0xFFFF, matches = 0;
    uint32 candidate = 99, minimum_priority, voice = 0, record, envelope, capacity;
    sint32 age = 0, priority, current_age;
    capacity = r_u8(0x800B7A40);
    minimum_priority = r_u8(0x800B7F7F);
    if (capacity != 0)
    {
        do
        {
            record = 0x800AC1A0 + (voice & 255) * 52;
            if (r_u8(record + 27) == 0 && r_u16(record + 6) == 0)
            {
                selected = voice;
                break;
            }
            priority = r_s16(record + 24);
            if (priority < (sint32)(uint16)minimum_priority)
            {
                minimum_priority = (uint32)priority;
                candidate = voice;
                minimum_envelope = r_u16(record + 6);
                age = r_u16(record + 2);
                matches = 1;
            }
            else if (priority == (sint32)(uint16)minimum_priority)
            {
                envelope = r_u16(record + 6);
                ++matches;
                if (envelope < (uint16)minimum_envelope)
                {
                    age = r_u16(record + 2);
                    minimum_envelope = envelope;
                    candidate = voice;
                }
                else if (envelope == (uint16)minimum_envelope)
                {
                    current_age = r_s16(record + 2);
                    if (age < current_age)
                    {
                        age = current_age;
                        candidate = voice;
                    }
                }
            }
            ++voice;
        } while ((voice & 255) < r_u8(0x800B7A40));
    }
    if ((selected & 255) == 99)
    {
        selected = candidate;
        if ((matches & 255) == 0)
            selected = r_u8(0x800B7A40);
    }
    capacity = r_u8(0x800B7A40);
    selected &= 255;
    if (selected < capacity)
    {
        for (voice = 0; (voice & 255) < capacity; ++voice)
        {
            record = 0x800AC1A2 + (voice & 255) * 52;
            w_u16(record, r_u16(record) + 1);
        }
        record = 0x800AC1A0 + selected * 52;
        w_u16(record + 2, 0);
        w_u16(record + 24, r_u8(0x800B7F7F));
        if (r_u8(record + 27) == 2)
            p2_sdk_80076b94(0, 0xFFFFFF);
    }
    return selected;
}

uint32 p2_sdk_80078430(uint8 voice)
{
    uint32 offset = (uint32)voice * 52;
    uint32 registers;
    w_u8(0x800AC1BB + offset, 0);
    registers = r_u32(0x80088290);
    w_u16(0x800AC1A4 + offset, 0);
    w_u16(registers + 404, 0);
    w_u16(registers + 406, 0);
    return offset;
}

uint32 p2_sdk_800776bc(sint16 sequence, sint16 bank, sint16 program, uint16 note)
{
    uint32 voice = 0, result = 0, record;
    if (r_u8(0x800B7A40) != 0)
    {
        do
        {
            record = 0x800AC1A0 + (voice & 255) * 52;
            if (r_s16(record + 12) == (sint32)note && r_s16(record + 18) == program && r_s16(record + 14) == sequence && r_s16(record + 22) == bank)
            {
                if (r_s16(record) == 255)
                {
                    ++result;
                    p2_sdk_80078430((uint8)voice);
                }
                else
                {
                    w_u16(0x800B7F8A, voice & 255);
                    p2_sdk_80078474();
                    ++result;
                }
            }
            ++voice;
        } while ((voice & 255) < r_u8(0x800B7A40));
    }
    return result;
}

uint32 p2_sdk_800793ec(uint32 tones_offset, uint32 samples_offset)
{
    uint32 tone = 0, count = 0, data, note, sample;
    if (r_u8(0x800B7F70) != 0)
    {
        do
        {
            data = r_u32(0x800B182C) + (16 * r_u8(0x800B7F77) + (tone & 255)) * 32;
            sample = r_u8(data + 6);
            note = r_u8(0x800B7F72);
            if (note >= sample)
            {
                sample = r_u8(data + 7);
                if (sample >= note)
                {
                    sample = r_u8(data + 22);
                    w_u8(samples_offset + (count & 255), sample);
                    w_u8(tones_offset + (count & 255), tone);
                    ++count;
                }
            }
            ++tone;
        } while ((tone & 255) < r_u8(0x800B7F70));
    }
    return count & 255;
}

uint32 p2_sdk_80077b98(void)
{
    uint32 voice = (uint32)(sint32)r_s16(0x800B7F8A);
    uint32 tone, index, data, value, table;
    sint32 sample;
    w_u16(0x800B7F8C, voice * 8);
    tone = 16 * r_u8(0x800B7F77) + r_u8(0x800B7F7C);
    w_u16(0x800B7F8E, tone);
    w_u16(0x800AC1A6 + voice * 52, 0x7FFF);
    for (index = 0; index < 16; ++index)
    {
        voice = (uint32)(sint32)r_s16(0x800B7F8A);
        value = r_u32(0x800BA4D8 + index * 4);
        w_u32(0x800BA4D8 + index * 4, value & ~(1u << (voice & 31)));
    }
    value = r_u16(0x800B7F88);
    sample = ((sint32)(sint16)value - 1) / 2;
    data = r_u32(0x800B1820) + (uint32)sample * 16;
    index = (uint32)(sint32)r_s16(0x800B7F8C);
    value = r_u16(data + ((value & 1) != 0 ? 12 : 14));
    w_u16(0x800B7FA6 + index * 2, value);
    voice = (uint32)(sint32)r_s16(0x800B7F8A);
    w_u8(0x800B14F8 + voice, r_u8(0x800B14F8 + voice) | 8);
    tone = 16 * r_u8(0x800B7F77) + r_u8(0x800B7F7C);
    table = r_u32(0x800B182C);
    index = (uint32)(sint32)r_s16(0x800B7F8C);
    value = r_u16(table + tone * 32 + 16);
    w_u16(0x800B7FA8 + index * 2, value);
    tone = 16 * r_u8(0x800B7F77) + r_u8(0x800B7F7C);
    index = (uint32)(sint32)r_s16(0x800B7F8C);
    value = r_u16(table + tone * 32 + 18);
    value += r_u16(0x800B1690);
    w_u16(0x800B7FAA + index * 2, value);
    voice = (uint32)(sint32)r_s16(0x800B7F8A);
    value = r_u8(0x800B14F8 + voice) | 0x30;
    w_u8(0x800B14F8 + voice, value);
    return value;
}

uint16 p2_sdk_80077dac(void)
{
    sint32 semitone = (sint16)(r_u8(0x800B7F72) + 60 - r_u8(0x800B7F80));
    sint32 octave;
    sint32 remainder, shift;
    uint32 fine = r_u8(0x800B7F81) >> 3;
    uint32 index, pitch;
    octave = semitone / 12;
    remainder = (sint16)(semitone - octave * 12);
    if (fine >= 16)
        fine = 15;
    index = (uint32)(remainder * 16) + fine;
    pitch = r_u16(0x80088294 + index * 2);
    shift = (sint16)(octave - 5);
    if (shift > 0)
        pitch <<= (uint32)shift & 31;
    else if (shift < 0)
        pitch >>= (uint32)(0u - (uint32)shift) & 31;
    return (uint16)pitch;
}

static uint16 p2_tone_pitch_80077e64(uint16 note, uint16 fraction)
{
    uint32 tone = r_u8(0x800B7F77) * 16 + r_u8(0x800B7F7C);
    uint32 data = r_u32(0x800B182C) + tone * 32;
    sint32 fine = (fraction + r_u8(data + 5)) / 8;
    sint32 carry = fine >= 16;
    sint32 semitone, octave, remainder, shift;
    uint32 index, pitch;
    if (carry != 0)
        fine -= 16;
    semitone = (sint16)(carry + note + 60 - r_u8(data + 4));
    octave = semitone / 12;
    remainder = (sint16)(semitone - octave * 12);
    index = (uint32)(remainder * 16 + (sint16)fine);
    pitch = r_u16(0x80088294 + index * 2);
    shift = (sint16)(octave - 5);
    if (shift > 0)
        pitch <<= (uint32)shift & 31;
    else if (shift < 0)
        pitch >>= (uint32)(0u - (uint32)shift) & 31;
    return (uint16)pitch;
}

static uint32 p2_voice_bend_80078a28(sint16 voice, sint16 sequence, sint16 bank, sint16 program, uint16 bend)
{
    uint32 index = (uint32)(sint32)voice;
    uint32 record = 0x800AC1A0 + index * 52;
    uint32 tone, note, pitch;
    sint32 amount = (sint16)(bend - 64);
    sint32 scaled, coarse, fraction;
    if (r_s16(record + 14) != sequence)
        return 0;
    if (r_s16(record + 22) != bank)
        return 0;
    if (r_s16(record + 18) != program)
        return 0;
    tone = (uint16)(r_u16(record + 20) + 16 * r_u8(0x800B7F77));
    note = r_u16(record + 12);
    if (amount > 0)
    {
        scaled = amount * r_u8(r_u32(0x800B182C) + tone * 32 + 13);
        coarse = (sint32)note + scaled / 63;
        fraction = (scaled % 63) * 2;
    }
    else if (amount < 0)
    {
        scaled = amount * r_u8(r_u32(0x800B182C) + tone * 32 + 12);
        coarse = (sint32)note + scaled / 64 - 1;
        fraction = (scaled % 64) * 2 + 127;
    }
    else
    {
        coarse = (sint32)note;
        fraction = 0;
    }
    tone = r_u8(record + 20);
    w_u16(0x800B7F8A, (uint16)voice);
    w_u8(0x800B7F7C, tone);
    pitch = p2_tone_pitch_80077e64((uint16)coarse, (uint16)fraction);
    w_u16(0x800B7FA4 + index * 16, pitch);
    w_u8(0x800B14F8 + index, r_u8(0x800B14F8 + index) | 4);
    return 1;
}

static uint32 p2_sequence_bend_80078c2c(sint16 sequence, sint16 bank, sint16 program, uint16 bend)
{
    uint32 voice = 0, result = 0;
    p2_sdk_80079a18((uint16)bank, program);
    w_u16(0x800B7F86, (uint16)sequence);
    if (r_u8(0x800B7A40) != 0)
    {
        do
        {
            result += (uint32)(sint32)(sint16)p2_voice_bend_80078a28((sint16)voice, sequence, bank, program, bend);
            ++voice;
        } while ((sint16)voice < r_u8(0x800B7A40));
    }
    return result;
}

uint32 p2_sdk_80073718(sint16 sequence, sint16 separation)
{
    uint32 record = r_u32(0x800B9B00 + 4 * (uint32)(sint32)sequence) + 172 * (uint32)(sint32)separation;
    uint32 cursor = r_u32(record + 4);
    sint16 bank = r_s16(record + 76);
    sint16 packed = (sint16)((uint32)(sint32)sequence | ((uint32)(sint32)separation << 8));
    uint32 channel, bend, program, result;
    w_u32(record + 4, cursor + 1);
    channel = r_u8(record + 18);
    bend = r_u8(cursor);
    program = r_u8(record + channel + 44);
    p2_sequence_bend_80078c2c(packed, bank, (sint16)program, (uint16)bend);
    result = p2_sdk_80073d58(sequence, separation);
    w_u32(record + 136, result);
    return result;
}

static void p2_sequence_tone_event(uint32 target, sint16 bank, sint16 program, sint16 tone, uint32 input, uint8 value)
{
    uint32 saved_stack, attributes, index, first, second;
    uint16 fields[9];
    switch (target)
    {
        case 0x80072B5C:
            p2_sdk_80072b5c(bank, program, tone, input, value);
            return;
        case 0x80072BEC:
            p2_sdk_80072bec(bank, program, tone, input, value);
            return;
        case 0x80072CA8:
            p2_sdk_80072ca8(bank, program, tone, input, value);
            return;
        case 0x80072D38:
            p2_sdk_80072d38(bank, program, tone, input, value);
            return;
        case 0x80073660:
            p2_func_80073660(value);
            return;
        case 0x80073684:
            p2_sdk_80076354(value, value);
            return;
        case 0x800736AC:
            p2_func_800736ac(value);
            return;
        case 0x800736D0:
            p2_func_800736d0(value);
            return;
        case 0x800736F4:
            p2_func_800736f4(value);
            return;
        case 0x80072DC8:
        case 0x80072F64:
        case 0x8007301C:
        case 0x800730CC:
        case 0x8007317C:
        case 0x80073230:
        case 0x800732E8:
        case 0x8007339C:
        case 0x80073444:
        case 0x80073520:
        case 0x800735C0:
            break;
        default:
            fprintf(stderr, "P2: Unsupported SEQ tone callback %08X\n", target);
            abort();
    }
    saved_stack = p2_guest_stack_pointer;
    p2_guest_stack_pointer -= 32;
    attributes = p2_guest_stack_pointer;
    for (index = 0; index < 32; index += 4)
        w_u32(attributes + index, r_u32(input + index));
    p2_sdk_800760d8(bank, program, (uint32)(sint32)tone, attributes);
    first = r_u16(attributes + 16);
    second = r_u16(attributes + 18);
    /* Initialize the original missing ADSR fields from the current tone */
    fields[5] = first & 0x8000;
    fields[6] = second & 0x8000;
    fields[8] = second & 0x4000;
    fields[7] = second & 0x20;
    fields[0] = (first >> 8) & 0x7F;
    fields[1] = (first >> 4) & 15;
    fields[2] = first & 15;
    fields[3] = (second >> 6) & 0x7F;
    fields[4] = second & 31;
    switch (target)
    {
        case 0x80072DC8:
            fields[5] = 0;
            fields[0] = value;
            break;
        case 0x80072F64:
            fields[5] = 1;
            fields[0] = value;
            break;
        case 0x8007301C:
            fields[1] = value;
            break;
        case 0x800730CC:
            fields[2] = value;
            break;
        case 0x8007317C:
            fields[6] = 0;
            fields[3] = value;
            break;
        case 0x80073230:
            fields[6] = 1;
            fields[3] = value;
            break;
        case 0x800732E8:
            fields[7] = 0;
            fields[4] = value;
            break;
        case 0x8007339C:
            fields[7] = 1;
            fields[4] = value;
            break;
        case 0x80073444:
            if (value != 0 && value < 64)
                fields[8] = 0;
            else if ((uint32)value - 64 < 64)
                fields[8] = 1;
            break;
        case 0x80073520:
            w_u8(attributes + 9, value);
            break;
        case 0x800735C0:
            w_u8(attributes + 10, value);
            break;
    }
    first = (fields[5] != 0 ? 0x8000 : 0) | ((fields[0] << 8) & 0x7F00) | ((fields[1] << 4) & 0xF0) | (fields[2] & 15);
    /* Preserve the release mode omitted by the original ADSR encoder */
    second = (fields[6] != 0 ? 0x8000 : 0) | (fields[8] != 0 ? 0x4000 : 0) | ((fields[3] << 6) & 0x1FC0) | (fields[7] != 0 ? 0x20 : 0) | (fields[4] & 31);
    w_u16(attributes + 16, first);
    w_u16(attributes + 18, second);
    p2_sdk_800765f0(bank, program, (uint32)(sint32)tone, attributes);
    p2_guest_stack_pointer = saved_stack;
}

uint32 p2_sdk_8007201c(sint16 sequence, sint16 separation, uint8 value)
{
    uint32 saved_stack = p2_guest_stack_pointer;
    uint32 record = r_u32(0x800B9B00 + 4 * (uint32)(sint32)sequence) + 172 * (uint32)(sint32)separation;
    uint32 channel = r_u8(record + 18);
    uint32 program, attributes, index, kind, tone, target, result;
    p2_guest_stack_pointer -= 48;
    program = p2_guest_stack_pointer;
    attributes = program + 16;
    p2_sdk_80075fd0(r_s16(record + 76), r_u8(record + channel + 44), program);
    if (r_u8(record + 39) == 1 && r_u8(record + 16) == 0)
    {
        w_u8(record + 40, value);
        w_u8(record + 39, 0);
        w_u8(record + 16, 1);
    }
    else if (r_u8(record + 41) == 2)
    {
        if (r_u8(record + 20) == 0 && r_u8(program) != 0)
        {
            index = 0;
            do
            {
                p2_sdk_800760d8(r_s16(record + 76), r_u8(record + channel + 44), (uint32)(sint32)(sint16)index, attributes);
                kind = r_u8(record + 19);
                if (kind == 0)
                {
                    w_u8(attributes + 13, value & 127);
                    w_u8(attributes + 12, value & 127);
                }
                else if (kind == 1)
                    w_u8(attributes + 5, r_u8(attributes + 5));
                else if (kind == 2)
                    w_u8(attributes + 4, r_u8(attributes + 4));
                p2_sdk_800765f0(r_s16(record + 76), r_u8(record + channel + 44), (uint32)(sint32)(sint16)index, attributes);
                ++index;
            } while ((sint32)index < r_u8(program));
        }
        result = p2_sdk_80073d58(sequence, separation);
        w_u32(record + 136, result);
        w_u8(record + 41, 0);
        p2_guest_stack_pointer = saved_stack;
        return result;
    }
    else if (r_u8(record + 42) == 2)
    {
        tone = r_u8(record + 22);
        if (tone == 16)
        {
            index = 0;
            if (r_u8(program) != 0)
            {
                do
                {
                    kind = r_u8(record + 21);
                    target = r_u32(0x800AC14C + kind * 4);
                    p2_sequence_tone_event(target, r_s16(record + 76), r_u8(record + channel + 44), (sint16)index, attributes, value);
                    ++index;
                } while ((sint32)index < r_u8(program));
            }
        }
        else
        {
            kind = r_u8(record + 21);
            target = r_u32(0x800AC14C + kind * 4);
            p2_sequence_tone_event(target, r_s16(record + 76), r_u8(record + channel + 44), (sint16)tone, attributes, value);
        }
        result = p2_sdk_80073d58(sequence, separation);
        w_u32(record + 136, result);
        w_u8(record + 42, 0);
        p2_guest_stack_pointer = saved_stack;
        return result;
    }
    result = p2_sdk_80073d58(sequence, separation);
    w_u32(record + 136, result);
    p2_guest_stack_pointer = saved_stack;
    return result;
}

static uint32 p2_sequence_init_80073e10(sint16 sequence, sint16 bank, uint32 data)
{
    uint32 record = r_u32(0x800B9B00 + 4 * (uint32)(sint32)sequence);
    uint32 index, cursor, high, middle, tempo, quotient, remainder, delta;
    uint32 product, ticks, threshold, numerator;
    w_u16(record + 76, bank);
    w_u16(record + 74, 0);
    w_u8(record + 19, 0);
    w_u8(record + 20, 0);
    w_u8(record + 41, 0);
    w_u8(record + 21, 0);
    w_u8(record + 22, 0);
    w_u8(record + 42, 0);
    w_u8(record + 18, 0);
    w_u32(record + 124, 0);
    w_u32(record + 128, 0);
    w_u32(record + 132, 0);
    w_u16(record + 114, 0);
    w_u16(record + 72, 0);
    w_u8(record + 43, 0);
    w_u32(record + 136, 0);
    w_u8(record + 39, 0);
    w_u8(record + 40, 0);
    w_u8(record + 16, 0);
    w_u8(record + 17, 0);
    w_u16(record + 168, 127);
    w_u16(record + 170, 0);
    for (index = 0; index < 16; ++index)
    {
        w_u8(record + 44 + index, index);
        w_u8(record + 23 + index, 64);
        w_u16(record + 78 + index * 2, 127);
    }
    w_u16(record + 110, 1);
    w_u32(record + 4, data);
    high = r_u8(data);
    if (high != 83 && high != 112)
    {
        printf("This is an old SEQ data\n");
        return 0;
    }
    w_u32(record + 4, data + 8);
    if (r_u8(data + 7) != 1)
    {
        printf("This is not SEQ data\n");
        return (uint32)-1;
    }
    w_u32(record + 4, data + 9);
    high = r_u8(data + 8);
    w_u32(record + 4, data + 10);
    middle = r_u8(data + 9);
    cursor = r_u32(record + 4);
    w_u16(record + 74, (high << 8) | middle);
    w_u32(record + 4, cursor + 1);
    high = r_u8(cursor);
    w_u32(record + 4, cursor + 2);
    middle = r_u8(cursor + 1);
    w_u32(record + 4, cursor + 3);
    tempo = (high << 16) | (middle << 8) | r_u8(cursor + 2);
    if (tempo == 0)
    {
        fprintf(stderr, "P2: SEQ tempo division by zero at 80073F9C\n");
        abort();
    }
    quotient = 60000000 / tempo;
    remainder = 60000000 % tempo;
    w_u32(record + 132, tempo);
    if ((tempo >> 1) < remainder)
        ++quotient;
    w_u32(record + 132, quotient);
    cursor = r_u32(record + 4);
    w_u32(record + 140, r_u32(record + 132));
    w_u32(record + 4, cursor + 2);
    delta = p2_sdk_80073d58(sequence, 0);
    product = (uint32)(sint32)r_s16(record + 74) * r_u32(record + 132);
    cursor = r_u32(record + 4);
    high = r_u32(record + 4);
    w_u32(record + 12, cursor);
    ticks = r_u32(0x800B9930);
    w_u32(record + 124, delta);
    w_u32(record + 136, delta);
    w_u32(record + 8, high);
    threshold = ticks * 60;
    if (product * 10 < threshold)
    {
        if (product == 0)
        {
            fprintf(stderr, "P2: SEQ tick division by zero at 80074078\n");
            abort();
        }
        quotient = (ticks * 600) / product;
        w_u16(record + 110, quotient);
        w_u16(record + 112, quotient);
    }
    else
    {
        if (threshold == 0)
        {
            fprintf(stderr, "P2: SEQ tick division by zero at 800740BC\n");
            abort();
        }
        numerator = (uint32)(sint32)r_s16(record + 74) * r_u32(record + 132) * 10;
        quotient = numerator / threshold;
        numerator = (uint32)(sint32)r_s16(record + 74) * r_u32(record + 132) * 10;
        remainder = numerator % threshold;
        w_u16(record + 110, 0xFFFF);
        w_u16(record + 112, quotient);
        if (ticks * 30 < remainder)
            w_u16(record + 112, quotient + 1);
    }
    w_u16(record + 114, r_u16(record + 112));
    return 0;
}

uint32 p2_sdk_80071c8c(uint32 sequence, sint16 bank)
{
    uint32 used = r_u32(0x800B7F3C);
    uint32 slot = 0;
    sint16 result;
    if (used == 0xFFFFFFFF)
    {
        printf("Can't open sequence\n");
        return (uint32)-1;
    }
    while ((used & (1u << slot)) != 0)
        ++slot;
    w_u32(0x800B7F3C, r_u32(0x800B7F3C) | (1u << slot));
    result = (sint16)p2_sequence_init_80073e10((sint16)slot, bank, sequence);
    w_u32(0x800AC108, 0x80073BF0);
    w_u32(0x800AC10C, 0x80073CE0);
    w_u32(0x800AC114, 0x80073A10);
    w_u32(0x800AC110, 0x80073718);
    w_u32(0x800AC118, 0x800737D0);
    w_u32(0x800AC11C, 0x80071F9C);
    w_u32(0x800AC124, 0x80072444);
    w_u32(0x800AC128, 0x80072518);
    w_u32(0x800AC12C, 0x800725E8);
    w_u32(0x800AC130, 0x80072728);
    w_u32(0x800AC134, 0x80072860);
    w_u32(0x800AC138, 0x800729A4);
    w_u32(0x800AC13C, 0x80072A1C);
    w_u32(0x800AC140, 0x80072698);
    w_u32(0x800AC144, 0x80072A94);
    w_u32(0x800AC120, 0x8007201C);
    w_u32(0x800AC14C, 0x80072B5C);
    w_u32(0x800AC150, 0x80072BEC);
    w_u32(0x800AC154, 0x80072CA8);
    w_u32(0x800AC158, 0x80072D38);
    w_u32(0x800AC15C, 0x80072DC8);
    w_u32(0x800AC160, 0x80072F64);
    w_u32(0x800AC164, 0x8007301C);
    w_u32(0x800AC168, 0x800730CC);
    w_u32(0x800AC16C, 0x8007317C);
    w_u32(0x800AC170, 0x80073230);
    w_u32(0x800AC174, 0x800732E8);
    w_u32(0x800AC178, 0x8007339C);
    w_u32(0x800AC17C, 0x80073444);
    w_u32(0x800AC180, 0x80073520);
    w_u32(0x800AC184, 0x800735C0);
    w_u32(0x800AC188, 0x80073660);
    w_u32(0x800AC18C, 0x80073684);
    w_u32(0x800AC190, 0x800736AC);
    w_u32(0x800AC194, 0x800736D0);
    w_u32(0x800AC198, 0x800736F4);
    return result == -1 ? (uint32)-1 : slot;
}

static uint16 p2_note_pitch_800716a0(uint16 sample, uint16 note)
{
    sint32 fine = ((sample & 255) + (note & 255)) / 8;
    sint32 semitone = (sint16)(fine / 16 + (note >> 8) + 60 - (sample >> 8));
    sint32 octave = semitone / 12;
    sint32 remainder = semitone - octave * 12;
    sint32 shift = (sint16)(octave - 5);
    uint32 index = (uint32)(remainder * 16 + fine % 16);
    uint32 pitch = r_u16(0x800880BC + index * 2);
    if (shift > 0)
        pitch <<= (uint32)shift & 31;
    else if (shift < 0)
        pitch >>= (uint32)(0u - (uint32)shift) & 31;
    return (uint16)pitch;
}

static uint32 p2_voice_address_8006fca0(uint32 address)
{
    if (r_u32(0x80087D80) != 0)
    {
        uint32 alignment = r_u32(0x80087D88);
        if (alignment == 0)
        {
            fprintf(stderr, "P2: Original voice-address division-by-zero trap\n");
            abort();
        }
        if (address % alignment != 0)
            address = (address + alignment) & ~r_u32(0x80087D8C);
    }
    return (address >> (r_u32(0x80087D84) & 31)) & 0xFFFF;
}

uint32 p2_sdk_8007101c(uint32 attributes)
{
    uint32 mask = r_u32(attributes + 4);
    uint32 voice;
    for (voice = 0; voice < 24; ++voice)
    {
        SpuVoiceAttr native = {0};
        uint32 all = mask == 0;
        if ((r_u32(attributes) & (1u << voice)) == 0)
            continue;
        native.voice = 1u << voice;
        native.mask = (all != 0 ? 0x7FFFFu : mask) & ~0x20u;
        if (all != 0 || (mask & 0x10) != 0)
            native.pitch = r_u16(attributes + 20);
        if (all != 0 || (mask & 0x40) != 0)
            w_u16(0x80087924 + voice * 2, r_u16(attributes + 24));
        native.sample_note = r_u16(0x80087924 + voice * 2);
        native.mask |= 0x40;
        if (all != 0 || (mask & 0x20) != 0)
        {
            native.pitch = p2_note_pitch_800716a0(native.sample_note, r_u16(attributes + 22));
            native.mask |= 0x10;
        }
        if (all != 0 || (mask & 0x20000) != 0)
            native.adsr1 = r_u16(attributes + 58);
        if (all != 0 || (mask & 0x40000) != 0)
            native.adsr2 = r_u16(attributes + 60);
        if (all != 0 || (mask & 1) != 0)
        {
            native.volume.left = r_s16(attributes + 8);
            if (all != 0 || (mask & 4) != 0)
                native.volmode.left = r_s16(attributes + 12);
        }
        if (all != 0 || (mask & 2) != 0)
        {
            native.volume.right = r_s16(attributes + 10);
            if (all != 0 || (mask & 8) != 0)
                native.volmode.right = r_s16(attributes + 14);
        }
        if (all != 0 || (mask & 0x10000) != 0)
            native.loop_addr = p2_voice_address_8006fca0(r_u32(attributes + 32)) << 3;
        if (all != 0 || (mask & 0x80) != 0)
            native.addr = p2_voice_address_8006fca0(r_u32(attributes + 28)) << 3;
        if (all != 0 || (mask & 0x800) != 0)
        {
            if (all != 0 || (mask & 0x100) != 0)
                native.a_mode = r_s32(attributes + 36);
            native.ar = r_u16(attributes + 48);
        }
        if (all != 0 || (mask & 0x1000) != 0)
            native.dr = r_u16(attributes + 50);
        if (all != 0 || (mask & 0x8000) != 0)
            native.sl = r_u16(attributes + 56);
        if (all != 0 || (mask & 0x2000) != 0)
        {
            if (all != 0 || (mask & 0x200) != 0)
                native.s_mode = r_s32(attributes + 40);
            else
                native.s_mode = 1;
            native.sr = r_u16(attributes + 52);
        }
        if (all != 0 || (mask & 0x4000) != 0)
        {
            if (all != 0 || (mask & 0x400) != 0)
                native.r_mode = r_s32(attributes + 44);
            native.rr = r_u16(attributes + 54);
        }
        SpuSetVoiceAttr(&native);
    }
    return 0;
}

uint32 p2_sdk_80070dc4(uint32 attributes)
{
    p2_sdk_8007101c(attributes);
    return p2_sdk_80070bc0(1, r_u32(attributes));
}

static void p2_draw_completion(void)
{
    uint32 callback = r_u32(0x80089F90);
    if (callback == 0x80066C40)
        p2_func_80066c40();
    else if (callback != 0)
    {
        fprintf(stderr, "P2: Untranslated DrawSync callback %08X\n", callback);
        abort();
    }
}

uint32 p2_sdk_8007febc(uint32 callback)
{
    uint32 previous;
    if (r_u8(0x80089F86) >= 2)
        printf("DrawSyncCallback(%08x)...\n", callback);
    if (callback != 0 && callback != 0x80066C40)
    {
        fprintf(stderr, "P2: Untranslated DrawSync callback %08X\n", callback);
        abort();
    }
    previous = r_u32(0x80089F90);
    w_u32(0x80089F90, callback);
    DrawSyncCallback(callback == 0 ? NULL : p2_draw_completion);
    return previous;
}

uint32 p2_sdk_800804bc(uint32 table, sint32 count)
{
    if (r_u8(0x80089F86) >= 2)
        printf("ClearOTagR(%08x,%d)...\n", table, count);
    if (count <= 0 || (uint32)count > PSX_DRAM_SIZE / 4)
    {
        fprintf(stderr, "P2: Invalid ClearOTagR count %d\n", count);
        abort();
    }
    ClearOTagR((uint32 *)psx_addr(table, (size_t)(uint32)count * 4), count);
    w_u32(table, 0x8008A040 & 0xFFFFFF);
    return table;
}

static uint32 p2_reverb_occupied_80070998(uint32 units)
{
    uint32 shift = r_u32(0x80087D84) & 31;
    uint32 table = r_u32(0x80087DC0);
    uint32 boundary = units << shift;
    if (table == 0)
        return 0;
    for (;;)
    {
        uint32 address = r_u32(table);
        if ((address & 0x80000000) == 0)
        {
            uint32 base = address & 0x0FFFFFFF;
            if ((address & 0x40000000) != 0)
                return 0;
            if (base >= boundary || boundary < base + r_u32(table + 4))
                return 1;
        }
        table += 8;
    }
}

uint32 p2_sdk_80070a24(uint32 mode)
{
    uint32 units, shift, begin, bytes;
    sint32 result;
    SpuNativeTransferGuestBinding binding;
    if (mode >= 10)
        return (uint32)-1;
    units = r_u32(0x80087DC4 + mode * 4);
    if (p2_reverb_occupied_80070998(units) != 0)
        return (uint32)-1;
    shift = r_u32(0x80087D84) & 31;
    if (mode == 0)
    {
        bytes = 16u << shift;
        begin = 65520u << shift;
    }
    else
    {
        units = r_u32(0x80087DC4 + mode * 4);
        bytes = (65536u - units) << shift;
        begin = units << shift;
    }
    binding.requested_mode = 0x800878F8;
    binding.normalized_mode = 0x80087D78;
    binding.address_units = 0x80087D74;
    binding.completion = 0x80087D90;
    binding.completion_callback = 0x80087D94;
    binding.source_address = 0x80087DB0;
    binding.dma_blocks = 0x80087DB4;
    binding.address_shift = shift;
    result = spu_clear_reverb_guest(&binding, begin, bytes, 0x80087954, 0x80087DAC);
    if (result != 0)
    {
        fprintf(stderr, "P2: Reverb area transfer failed\n");
        abort();
    }
    return 0;
}

static void p2_remove_counted_action(uint32 character, uint32 action, uint32 count_offset)
{
    sint32 index = 0;
    while (index < r_s8(character + count_offset))
    {
        p2_func_80027ac0(character, action);
        ++index;
    }
    w_u8(character + count_offset, 0);
}

static void p2_clear_interaction_actions(uint32 character, uint32 clear_remaining)
{
    uint32 flags = r_u32(0x8008A1A4);
    if (flags != 0)
    {
        uint32 target = r_u32(0x8008A510);
        if (target != 0)
        {
            uint32 data = r_u32(r_u32(target + 132) + 44);
            w_u8(data + 38, r_u8(data + 38) & 0xF7);
        }
        flags = r_u32(0x8008A1A4);
    }
    if ((flags & 8) != 0)
        p2_func_80027ac0(character, 12);
    flags = r_u32(0x8008A1A4);
    w_u32(0x8008A1A4, flags & 0xFFFFEFF7);
    if ((flags & 16) != 0)
        p2_remove_counted_action(character, 13, 448);
    if ((r_u32(0x8008A1A4) & 32) != 0)
        p2_remove_counted_action(character, 14, 449);
    if (clear_remaining != 0)
        w_u32(0x8008A1A4, r_u32(0x8008A1A4) & ~48u);
}

uint32 p2_func_8005a7f4(uint32 object_offset, uint32 character_offset)
{
    uint32 flags, result;
    if (r_s8(0x8008A1A8) == 0)
        w_u16(character_offset + 66, r_u16(character_offset + 66) - 1u);
    w_u8(character_offset + 438, r_u8(character_offset + 438) + 1u);
    flags = r_u32(0x8008A1A4);
    if ((flags & 0x800) != 0)
    {
        w_u32(0x8008A1A4, flags & ~0x800u);
        w_u8(character_offset + 438, 0);
        w_u8(character_offset + 436, 0);
        p2_clear_interaction_actions(character_offset, 1);
    }
    switch (r_s8(character_offset + 436))
    {
        case 0:
            if ((r_u32(object_offset + 28) & 0x1000400) == 0x1000000)
            {
                w_u8(character_offset + 438, 0);
                w_u8(character_offset + 436, 1);
            }
            break;
        case 1:
        {
            sint32 tick = r_s8(character_offset + 438);
            if (tick == 1)
            {
                p2_func_80044ffc(16, 3);
                p2_func_80027b1c(object_offset, character_offset, 11);
            }
            else if (tick == 4)
            {
                flags = r_u32(0x8008A1A4);
                w_u32(0x8008A510, 0);
                w_u32(0x8008A1A4, flags | 0x200);
            }
            else if (tick == 5 || tick == 6)
            {
                uint32 target = r_u32(0x8008A510);
                uint32 data, config, saved_stack, world, input, other, index, definition, matrix;
                uint32 position[3];
                if (tick == 5 && target == 0)
                {
                    w_u32(0x8008A1A4, r_u32(0x8008A1A4) | 0x200);
                    break;
                }
                w_u8(character_offset + 438, 0);
                target = r_u32(0x8008A510);
                if (target == 0)
                {
                    w_u8(character_offset + 436, 0);
                    break;
                }
                data = r_u32(r_u32(target + 132) + 44);
                flags = r_u32(0x8008A1A4);
                config = r_u32(data + 72);
                w_u32(0x8008A1A4, flags & ~0x200u);
                saved_stack = p2_guest_stack_pointer;
                p2_guest_stack_pointer -= 48;
                world = p2_guest_stack_pointer;
                other = world + 16;
                input = world + 32;
                definition = r_u32(target + 132);
                if (definition != 0 && r_u32(definition + 40) != 0)
                {
                    for (index = 0; index < 3; ++index)
                        position[index] = r_u32(target + 16 + index * 4);
                    for (index = 0; index < 3; ++index)
                        w_u32(input + index * 4, position[index]);
                    PushMatrix();
                    target = r_u32(0x8008A510);
                    matrix = r_u32(r_u32(target + 132) + 40);
                    for (index = 0; index < 5; ++index)
                        xport_gte_write_control(index, r_u32(matrix + index * 4));
                    p2_sdk_8007ec64(input, world);
                    PopMatrix();
                    target = r_u32(0x8008A510);
                    for (index = 0; index < 3; ++index)
                    {
                        matrix = r_u32(r_u32(target + 132) + 40);
                        w_u32(world + index * 4, r_u32(world + index * 4) + r_u32(matrix + 20 + index * 4));
                    }
                }
                else
                {
                    target = r_u32(0x8008A510);
                    for (index = 0; index < 3; ++index)
                        position[index] = r_u32(target + 16 + index * 4);
                    for (index = 0; index < 3; ++index)
                        w_u32(world + index * 4, position[index]);
                }
                w_u32(data + 84, r_u32(world + 4));
                if (r_s16(config + 654) != -1 || r_s16(config + 646) != 0)
                {
                    definition = r_u32(object_offset + 132);
                    if (definition != 0 && r_u32(definition + 40) != 0)
                    {
                        for (index = 0; index < 3; ++index)
                            position[index] = r_u32(object_offset + 16 + index * 4);
                        for (index = 0; index < 3; ++index)
                            w_u32(input + index * 4, position[index]);
                        PushMatrix();
                        matrix = r_u32(r_u32(object_offset + 132) + 40);
                        for (index = 0; index < 5; ++index)
                            xport_gte_write_control(index, r_u32(matrix + index * 4));
                        p2_sdk_8007ec64(input, other);
                        PopMatrix();
                        for (index = 0; index < 3; ++index)
                        {
                            matrix = r_u32(r_u32(object_offset + 132) + 40);
                            w_u32(other + index * 4, r_u32(other + index * 4) + r_u32(matrix + 20 + index * 4));
                        }
                    }
                    else
                    {
                        for (index = 0; index < 3; ++index)
                            position[index] = r_u32(object_offset + 16 + index * 4);
                        for (index = 0; index < 3; ++index)
                            w_u32(other + index * 4, position[index]);
                    }
                    result = (uint32)p2_func_8005dc00((sint32)(r_u32(world + 8) - r_u32(other + 8)), (sint32)(r_u32(world) - r_u32(other)));
                }
                else
                    result = r_u16(r_u32(0x8008A510) + 148);
                w_u16(0x8008A484, result);
                w_u32(0x8008A1A4, r_u32(0x8008A1A4) | 8);
                w_u8(data + 38, r_u8(data + 38) | 8);
                p2_func_80027b1c(r_u32(0x8008A510), character_offset, 12);
                if ((r_u32(0x8008A1A4) & 0x1000) != 0 || r_s16(config + 646) == 0)
                    w_u8(character_offset + 436, 2);
                else
                {
                    w_u8(character_offset + 436, 4);
                    w_u32(character_offset + 468, 4096);
                    w_u8(character_offset + 446, 0);
                    w_u8(character_offset + 437, 0);
                    w_u8(character_offset + 441, 0);
                    w_u8(character_offset + 442, 0);
                    w_u8(character_offset + 448, 0);
                    w_u8(character_offset + 449, 0);
                    w_u8(character_offset + 450, 0);
                    w_u8(character_offset + 451, 0);
                    w_u8(character_offset + 447, 0);
                    w_u32(character_offset + 452, (uint32)-1);
                    w_u32(character_offset + 456, (uint32)-1);
                    w_u32(character_offset + 460, (uint32)-1);
                    w_u8(character_offset + 444, r_u16(config + 646));
                    w_u8(character_offset + 445, r_u16(config + 648));
                }
                p2_guest_stack_pointer = saved_stack;
            }
            break;
        }
        case 2:
            if (r_s16(r_u32(0x8008A510) + 128) <= 0)
            {
                if ((r_u32(0x8008A1A4) & 8) != 0)
                    p2_func_80027ac0(character_offset, 12);
                w_u8(character_offset + 436, 0);
                p2_clear_interaction_actions(character_offset, 1);
            }
            else
            {
                if ((r_u32(object_offset + 28) & 0x1000400) == 0x1000000)
                {
                    p2_func_80044ffc(16, 3);
                    p2_func_80027b1c(object_offset, character_offset, 11);
                    w_u8(character_offset + 438, 0);
                    p2_func_80027b1c(object_offset, character_offset, 13);
                    w_u32(0x8008A1A4, r_u32(0x8008A1A4) | 16);
                }
                if (r_s8(character_offset + 447) == 1)
                {
                    uint32 target, data;
                    w_u8(character_offset + 447, 0);
                    target = r_u32(0x8008A510);
                    if (r_s16(target + 128) != 0)
                    {
                        data = r_u32(r_u32(target + 132) + 44);
                        flags = r_u32(data + 44);
                        result = (uint32)(sint32)r_s8(0x8008A1A9);
                        w_u32(data + 44, (flags & 0xFFFF01FF) | 0x208);
                        if (result != 0)
                        {
                            w_u32(target + 20, r_u32(target + 20) + 3072u);
                            w_u8(data + 38, r_u8(data + 38) | 16);
                        }
                    }
                }
            }
            break;
        case 4:
            if (r_s16(r_u32(0x8008A510) + 128) <= 0)
            {
                w_u8(character_offset + 438, 0);
                w_u8(character_offset + 436, 0);
                p2_clear_interaction_actions(character_offset, 1);
                break;
            }
            if ((r_u32(object_offset + 28) & 0x1000400) == 0x1000000)
            {
                p2_func_80044ffc(16, 3);
                p2_func_80027b1c(object_offset, character_offset, 11);
                w_u8(character_offset + 438, 0);
                p2_func_80027b1c(object_offset, character_offset, 13);
                w_u32(0x8008A1A4, r_u32(0x8008A1A4) | 16);
                w_u8(character_offset + 441, 1);
            }
            if (r_s8(character_offset + 446) != 0)
            {
                sint32 table_index, delay;
                w_u32(character_offset + 472, r_s8(character_offset + 441) == 1 ? 4096 : 2048);
                p2_func_80044ffc(16, 0);
                p2_func_80027b1c(object_offset, character_offset, 14);
                w_u32(0x8008A1A4, r_u32(0x8008A1A4) | 32);
                table_index = r_s8(character_offset + 439) % 16;
                w_u8(character_offset + 446, 0);
                delay = r_s32(0x80083BAC + (uint32)(table_index * 4)) / 2048;
                w_u8(character_offset + 437, delay);
                delay += r_u8(character_offset + 445);
                w_u8(character_offset + 437, (sint32)(sint8)delay * 4);
            }
            else
            {
                if (r_s8(character_offset + 442) != 0 && r_s8(character_offset + 441) != 1)
                    goto interaction_result;
                result = r_u8(character_offset + 437) - 1u;
                w_u8(character_offset + 437, result);
                if ((sint8)result > 0 || r_s8(0x8008A1AA) != 0 || (r_s8(character_offset + 442) != 0 && r_s8(character_offset + 441) != 1))
                    goto interaction_result;
                {
                    uint32 pending = r_u8(character_offset + 446);
                    sint32 table_index = r_s8(character_offset + 439) % 16;
                    sint32 delay;
                    w_u8(character_offset + 442, 1);
                    w_u8(character_offset + 446, pending + 1u);
                    delay = r_s32(0x80083BAC + (uint32)(table_index * 4)) / 2048;
                    result = r_u8(character_offset + 445);
                    w_u8(character_offset + 437, delay);
                    w_u8(character_offset + 437, (uint32)delay + result);
                }
            }
        interaction_result:
            if (r_s8(character_offset + 447) == 1)
            {
                w_u8(character_offset + 447, 0);
                if ((r_u32(0x8008A1A4) & 16) != 0)
                    p2_remove_counted_action(character_offset, 13, 448);
                if ((r_u32(0x8008A1A4) & 32) != 0)
                    p2_remove_counted_action(character_offset, 14, 449);
                w_u32(0x8008A1A4, (r_u32(0x8008A1A4) & ~48u) | 0x3000);
                w_u8(character_offset + 436, 5);
                w_u8(character_offset + 439, 0);
            }
            else if (r_s8(character_offset + 447) == 2)
            {
                w_u8(character_offset + 447, 0);
                flags = r_u32(0x8008A1A4);
                w_u32(character_offset + 180, 0);
                w_u32(character_offset + 668, 0);
                w_u32(0x8008A1A4, flags | 0x2100);
            }
            break;
        case 5:
            result = r_u8(character_offset + 439) + 1u;
            w_u8(character_offset + 439, result);
            if ((sint8)result >= 3)
            {
                w_u32(0x8008A1A4, r_u32(0x8008A1A4) & ~0x2000u);
                w_u8(character_offset + 439, 0);
                w_u8(character_offset + 436, 2);
            }
            break;
        default:
            w_u8(character_offset + 438, 0);
            w_u8(character_offset + 439, 0);
            w_u8(character_offset + 436, 0);
            break;
    }
    if (r_s16(character_offset + 66) == 0)
    {
        p2_clear_interaction_actions(character_offset, 0);
        flags = r_u32(character_offset + 4);
        w_u32(0x8008A1A4, 0);
        w_u32(character_offset + 4, flags & ~0x100u);
    }
    flags = r_u32(object_offset + 28);
    result = r_u8(character_offset + 438);
    w_u8(0x8008A50D, result);
    w_u32(object_offset + 28, flags & ~0x1000000u);
    return result;
}

uint32 p2_func_80056884(uint32 object_offset, uint32 character_offset)
{
    uint32 saved_stack, world, input, inverse, position_vector, floor_vector;
    uint32 flags = r_u32(object_offset + 228);
    uint32 special = flags & 0x400000;
    uint32 index, definition, matrix, path, result;
    uint32 position[3], delta[3];
    if (special == 0)
    {
        if (r_s8(character_offset + 252) != 7)
            return (uint32)-969;
        result = r_u32(object_offset + 28);
        w_u32(object_offset + 228, flags & 0xFFFFFC37);
        w_u32(object_offset + 28, result | 0x400);
        w_u32(character_offset, r_u32(character_offset) | 0x2000);
    }
    else if (r_s8(character_offset + 252) != 10)
        return 10;
    saved_stack = p2_guest_stack_pointer;
    p2_guest_stack_pointer -= 96;
    world = p2_guest_stack_pointer;
    input = world + 16;
    inverse = world + 32;
    position_vector = world + 64;
    floor_vector = world + 80;
    if (r_u32(r_u32(object_offset + 132) + 40) != 0)
    {
        path = r_u32(r_u32(0x8008A114) + 136);
        definition = r_u32(path + 36);
        if (definition != 0 && r_u32(definition + 40) != 0)
        {
            uint32 target;
            for (index = 0; index < 3; ++index)
                position[index] = r_u32(object_offset + 16 + index * 4);
            for (index = 0; index < 3; ++index)
                w_u32(input + index * 4, position[index]);
            PushMatrix();
            path = r_u32(r_u32(0x8008A114) + 136);
            matrix = r_u32(r_u32(path + 36) + 40);
            for (index = 0; index < 5; ++index)
                xport_gte_write_control(index, r_u32(matrix + index * 4));
            p2_sdk_8007ec64(input, world);
            PopMatrix();
            target = r_u32(0x8008A114);
            for (index = 0; index < 3; ++index)
            {
                path = r_u32(target + 136);
                matrix = r_u32(r_u32(path + 36) + 40);
                w_u32(world + index * 4, r_u32(world + index * 4) + r_u32(matrix + 20 + index * 4));
            }
        }
        else
        {
            for (index = 0; index < 3; ++index)
                position[index] = r_u32(object_offset + 16 + index * 4);
            for (index = 0; index < 3; ++index)
                w_u32(world + index * 4, position[index]);
        }
        for (index = 0; index < 3; ++index)
            delta[index] = r_u32(character_offset + 308 + index * 4) - r_u32(character_offset + 296 + index * 4);
        for (index = 0; index < 3; ++index)
            position[index] = r_u32(world + index * 4) - delta[index];
        for (index = 0; index < 3; ++index)
            w_u32(character_offset + 296 + index * 4, position[index]);
    }
    definition = r_u32(object_offset + 132);
    if (definition != 0 && r_u32(definition + 40) != 0)
    {
        for (index = 0; index < 3; ++index)
        {
            matrix = r_u32(r_u32(object_offset + 132) + 40);
            w_u32(input + index * 4, r_u32(character_offset + 296 + index * 4) - r_u32(matrix + 20 + index * 4));
        }
        for (index = 0; index < 9; ++index)
        {
            matrix = r_u32(r_u32(object_offset + 132) + 40);
            w_u16(inverse + index * 2, r_u16(matrix + (index % 3) * 6 + (index / 3) * 2));
        }
        PushMatrix();
        for (index = 0; index < 5; ++index)
            xport_gte_write_control(index, r_u32(inverse + index * 4));
        p2_sdk_8007ec64(input, object_offset + 16);
        PopMatrix();
    }
    else
    {
        for (index = 0; index < 3; ++index)
            position[index] = r_u32(character_offset + 296 + index * 4);
        for (index = 0; index < 3; ++index)
            w_u32(object_offset + 16 + index * 4, position[index]);
    }
    for (index = 0; index < 3; ++index)
        position[index] = r_u32(character_offset + 296 + index * 4);
    for (index = 0; index < 3; ++index)
        w_u32(position_vector + index * 4, position[index]);
    for (index = 0; index < 3; ++index)
        w_u32(floor_vector + index * 4, position[index]);
    if (special == 0)
    {
        p2_func_80046cbc(object_offset, character_offset);
        if (r_s8(0x8008A1AB) != 0)
            w_u32(object_offset + 228, r_u32(object_offset + 228) | 0x2000000);
    }
    else
        w_u32(object_offset + 228, r_u32(object_offset + 228) | 0x2200000);
    path = r_u32(r_u32(0x8008A114) + 136);
    p2_func_8003af18(object_offset, path, position_vector, floor_vector);
    if (special == 0)
    {
        p2_func_80046cbc(object_offset, character_offset);
        /* The original normal branch keeps its transition flag zero */
        w_u32(object_offset + 228, r_u32(object_offset + 228) & ~0x40000u);
    }
    path = r_u32(r_u32(0x8008A114) + 136);
    result = r_u32(character_offset + 408);
    if (path == result)
    {
        result = r_u32(object_offset + 28) | 0x2000;
        w_u32(object_offset + 28, result);
    }
    if (special != 0)
    {
        result = 11;
        w_u8(character_offset + 252, result);
        w_u32(character_offset + 288, 0);
    }
    p2_guest_stack_pointer = saved_stack;
    return result;
}

sint32 p2_func_8005b604(uint32 object_offset, uint32 other_offset)
{
    uint32 saved_stack = p2_guest_stack_pointer;
    uint32 positions, input, inverse, side, index;
    sint32 result = 0;
    p2_guest_stack_pointer -= 96;
    positions = p2_guest_stack_pointer;
    input = positions + 32;
    inverse = positions + 48;
    for (side = 0; side < 2; ++side)
    {
        uint32 object = side == 0 ? object_offset : other_offset;
        uint32 destination = positions + side * 16;
        uint32 definition = r_u32(object + 132);
        uint32 matrix;
        if (definition != 0 && r_u32(definition + 40) != 0)
        {
            for (index = 0; index < 3; ++index)
            {
                matrix = r_u32(r_u32(object + 132) + 40);
                w_u32(input + index * 4, r_u32(object + 16 + index * 4) - r_u32(matrix + 20 + index * 4));
            }
            for (index = 0; index < 9; ++index)
            {
                matrix = r_u32(r_u32(object + 132) + 40);
                w_u16(inverse + index * 2, r_u16(matrix + (index % 3) * 6 + (index / 3) * 2));
            }
            PushMatrix();
            for (index = 0; index < 5; ++index)
                xport_gte_write_control(index, r_u32(inverse + index * 4));
            p2_sdk_8007ec64(input, destination);
            PopMatrix();
        }
        else
        {
            uint32 position[3];
            for (index = 0; index < 3; ++index)
                position[index] = r_u32(object + 16 + index * 4);
            for (index = 0; index < 3; ++index)
                w_u32(destination + index * 4, position[index]);
        }
    }
    if (r_s32(object_offset + 96) < 0 && r_s32(positions + 20) < r_s32(positions + 4) && r_u32(positions) - r_u32(positions + 16) + 6144u < 0x3001 && r_u32(positions + 8) - r_u32(positions + 24) + 6144u < 0x3001)
        result = -1;
    p2_guest_stack_pointer = saved_stack;
    return result;
}

uint32 p2_func_8006e344(uint32 level, sint8 mode)
{
    uint32 saved_stack = p2_guest_stack_pointer;
    uint32 camera, flags, scene, loaded, sound, table, tick;
    sint32 next_level, percent, shown;
    p2_guest_stack_pointer -= 32;
    camera = p2_guest_stack_pointer;
    if (level - 75u < 4)
    {
        next_level = 0;
        flags = 0x800000;
        shown = 0;
        percent = -1;
    }
    else
    {
        next_level = level - 96u < 2 ? 0 : -1;
        flags = level - 96u < 2 ? 0x8000 : 0;
        shown = 1;
        percent = 0;
    }
    p2_func_8006b548();
    scene = r_u32(0x8008A4E4);
    if (r_u16(scene + 270) == 0)
        w_u16(scene + 270, 1);
    if (shown != 0)
    {
        uint32 collected = p2_func_80025620(0);
        uint32 total, numerator;
        uint16 cumulative;
        scene = r_u32(0x8008A4E4);
        cumulative = (uint16)(r_u16(scene + 272) + collected);
        total = r_u16(scene + 270);
        numerator = 100u * cumulative + (total >> 1);
        if (total == 0)
        {
            fprintf(stderr, "P2: Original division-by-zero trap in 8006E344\n");
            abort();
        }
        percent = (sint32)numerator / (sint32)total;
        shown = 0;
        w_u16(scene + 272, cumulative);
        if (percent >= 101)
            percent = 100;
        if (next_level != 0)
            next_level = percent >= 80 || (r_u32(0x8008A52C) & 2) != 0 ? 96 : 0;
    }
    p2_func_80067160(flags);
    p2_func_80026aa0();
    loaded = (uint32)p2_func_8002deb8(next_level, 0);
    w_u32(camera, p2_func_80053bf0());
    w_u32(camera + 8, (uint32)-14336);
    w_u32(camera + 4, 0);
    w_u32(camera + 12, 430080);
    w_u32(camera + 20, 0);
    w_u32(camera + 16, 0);
    w_u32(camera + 24, (uint32)-4096);
    w_u32(0x8008A3C4, p2_func_80066d08());
    w_u32(0x8008A3CC, 30);
    w_u32(0x8008A3C8, r_u32(0x8008A434));
    for (;;)
    {
        uint32 height;
        p2_func_8006e098(shown, flags, next_level, camera, mode);
        if (shown < percent)
            ++shown;
        height = r_u32(camera + 12);
        if ((sint32)height >= 20481)
        {
            height += r_u32(camera + 24);
            w_u32(camera + 12, height);
            if ((sint32)height < 20481)
                w_u32(camera + 12, 20480);
        }
        else
        {
            uint32 previous = ~r_u32(0x8008A3C4);
            uint32 pressed = p2_func_80066d08();
            w_u32(0x8008A3C4, pressed);
            if ((pressed & previous & 64) != 0)
            {
                sint32 voice;
                table = r_u32(0x8008A408);
                tick = r_u32(0x8008A518);
                sound = 0;
                w_u32(0x8008A174, 64);
                if (table != 0 && r_s32(table) > 25)
                    sound = table + 204;
                w_u32(0x8008A518, r_u32(0x8008A4F0));
                voice = p2_func_8005e628((uint32)-1, sound, 4);
                if (voice >= 0)
                    p2_func_8005e470(sound, (uint16)voice, 0, 0, 0, r_s16(0x8008A24C), 0);
                w_u32(0x8008A518, tick);
                break;
            }
        }
        if ((loaded & 255) == 0)
            loaded = (uint32)p2_func_8002deb8(next_level, 0);
        w_u32(0x8008A4F0, r_u32(0x8008A4F0) + 1u);
    }
    if ((loaded & 255) == 0)
    {
        p2_func_80066168();
        while (p2_func_8002deb8(next_level, 0) == 0)
        {
        }
    }
    p2_func_80025bfc(0);
    table = r_u32(0x8008A460);
    sound = table != 0 && r_s32(table) > 0 ? table + 4 : 0;
    p2_func_8005e538(sound, 0, 0, 0, r_u16(0x8008A24C), 4);
    p2_guest_stack_pointer = saved_stack;
    return (uint32)next_level;
}
