#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "game.h"
#include "psx_gpu.h"
#include "xport_trace.h"

static void p2_sound_reset_8006f2ec(uint32 mode);
static void p2_sound_dma_8006f74c(void);
static PsxInterruptCallback p2_sound_register_dma_8006fe98(PsxInterruptCallback callback);
static void p2_sound_quit_8006febc(void);
static void p2_sound_stop_timer_80071ab0(void);
static uint16 p2_tone_pitch_80077e64(uint16 note, uint16 fraction);
static uint32 p2_voice_bend_80078a28(sint16 voice, sint16 sequence, sint16 bank, sint16 program, uint16 bend);
static uint32 p2_sequence_bend_80078c2c(sint16 sequence, sint16 bank, sint16 program, uint16 bend);
static uint32 p2_sequence_init_80073e10(sint16 sequence, sint16 bank, const uint8 *data);
static void p2_sound_irq0_80074790(void);
static void p2_sound_half_rate_800747d0(void);

static const SpuSequenceHandlers sequence_default_handlers = {p2_sdk_80073bf0, p2_sdk_80073ce0, p2_sdk_80073718, p2_sdk_80073a10, p2_sdk_800737d0, p2_sdk_8007201c, p2_sdk_80072444, p2_sdk_80072518, p2_sdk_800725e8, p2_sdk_80072728, p2_sdk_80072860, p2_sdk_800729a4, p2_sdk_80072a1c, p2_sdk_80072698, p2_sdk_80072a94};

static const SpuToneCommand sequence_default_commands[20] = {{p2_sdk_80072b5c, NULL}, {p2_sdk_80072bec, NULL}, {p2_sdk_80072ca8, NULL}, {p2_sdk_80072d38, NULL}, {p2_sdk_80072dc8, NULL}, {p2_sdk_80072f64, NULL}, {p2_sdk_8007301c, NULL}, {p2_sdk_800730cc, NULL}, {p2_sdk_8007317c, NULL}, {p2_sdk_80073230, NULL}, {p2_sdk_800732e8, NULL}, {p2_sdk_8007339c, NULL}, {p2_sdk_80073444, NULL}, {p2_sdk_80073520, NULL}, {p2_sdk_800735c0, NULL}, {NULL, p2_func_80073660}, {NULL, SetSequenceMasterVolume}, {NULL, p2_func_800736ac}, {NULL, p2_func_800736d0}, {NULL, p2_func_800736f4}};

static const SpuSequencePumpOps sequence_pump_ops = {p2_sdk_800767dc, p2_func_800750c4, p2_sdk_80074a98, p2_sdk_80074d74, AdvanceSequenceTempoRamp, p2_sdk_80075020, ResumeSequenceTrack, p2_sdk_80075818};

uint32 p2_func_80074138(uint32 sequence, uint32 track)
{
    FUNCTION_MARKER(0x80074138, "SLUS_005.78");
    return spu_sequence_scheduler_request_pause(&spu_sequence_scheduler, sequence, track, p2_sdk_80079244);
}

sint32 p2_sdk_80079a18(uint16 bank, sint16 program)
{
    FUNCTION_MARKER(0x80079A18, "SLUS_005.78");
    return spu_vab_select_native(bank, program);
}

sint32 p2_sdk_80075fd0(sint16 bank, sint16 program, uint8 *attributes)
{
    FUNCTION_MARKER(0x80075FD0, "SLUS_005.78");
    return SsUtGetProgAtr(bank, program, (ProgAtr *)attributes);
}

sint32 p2_sdk_800760d8(sint16 bank, sint16 program, uint32 tone, uint8 *attributes)
{
    FUNCTION_MARKER(0x800760D8, "SLUS_005.78");
    return spu_vab_get_vag_atr_native(bank, program, (sint16)tone, (VagAtr *)attributes);
}

sint32 p2_sdk_800765f0(sint16 bank, sint16 program, uint32 tone, const uint8 *attributes)
{
    FUNCTION_MARKER(0x800765F0, "SLUS_005.78");
    return SsUtSetVagAtr(bank, program, (sint16)tone, (const VagAtr *)attributes);
}

uint32 DecodeToneAdsr(uint16 first, uint16 second, uint16 *fields)
{
    FUNCTION_MARKER(0x80072E7C, "SLUS_005.78");
    return _SsUtResolveADSR(first, second, fields);
}

uint32 EncodeToneAdsr(const uint16 *fields, uint16 *first, uint16 *second)
{
    FUNCTION_MARKER(0x80072ED8, "SLUS_005.78");
    return _SsUtBuildADSR(fields, first, second);
}

uint32 p2_sdk_80072b5c(sint16 bank, sint16 program, sint16 tone, VagAtr attributes, uint8 value)
{
    FUNCTION_MARKER(0x80072B5C, "SLUS_005.78");
    return spu_vab_nrpn_attr_by_value(0, bank, program, tone, attributes, value);
}

uint32 p2_sdk_80072bec(sint16 bank, sint16 program, sint16 tone, VagAtr attributes, uint8 value)
{
    FUNCTION_MARKER(0x80072BEC, "SLUS_005.78");
    return spu_vab_nrpn_attr_by_value(1, bank, program, tone, attributes, value);
}

uint32 p2_sdk_80072ca8(sint16 bank, sint16 program, sint16 tone, VagAtr attributes, uint8 value)
{
    FUNCTION_MARKER(0x80072CA8, "SLUS_005.78");
    return spu_vab_nrpn_attr_by_value(2, bank, program, tone, attributes, value);
}

uint32 p2_sdk_80072d38(sint16 bank, sint16 program, sint16 tone, VagAtr attributes, uint8 value)
{
    FUNCTION_MARKER(0x80072D38, "SLUS_005.78");
    return spu_vab_nrpn_attr_by_value(3, bank, program, tone, attributes, value);
}

uint32 p2_sdk_80072dc8(sint16 bank, sint16 program, sint16 tone, VagAtr attributes, uint8 value)
{
    FUNCTION_MARKER(0x80072DC8, "SLUS_005.78");
    return spu_vab_nrpn_attr_by_value(4, bank, program, tone, attributes, value);
}

uint32 p2_sdk_80072f64(sint16 bank, sint16 program, sint16 tone, VagAtr attributes, uint8 value)
{
    FUNCTION_MARKER(0x80072F64, "SLUS_005.78");
    return spu_vab_nrpn_attr_by_value(5, bank, program, tone, attributes, value);
}

uint32 p2_sdk_8007301c(sint16 bank, sint16 program, sint16 tone, VagAtr attributes, uint8 value)
{
    FUNCTION_MARKER(0x8007301C, "SLUS_005.78");
    return spu_vab_nrpn_attr_by_value(6, bank, program, tone, attributes, value);
}

uint32 p2_sdk_800730cc(sint16 bank, sint16 program, sint16 tone, VagAtr attributes, uint8 value)
{
    FUNCTION_MARKER(0x800730CC, "SLUS_005.78");
    return spu_vab_nrpn_attr_by_value(7, bank, program, tone, attributes, value);
}

uint32 p2_sdk_8007317c(sint16 bank, sint16 program, sint16 tone, VagAtr attributes, uint8 value)
{
    FUNCTION_MARKER(0x8007317C, "SLUS_005.78");
    return spu_vab_nrpn_attr_by_value(8, bank, program, tone, attributes, value);
}

uint32 p2_sdk_80073230(sint16 bank, sint16 program, sint16 tone, VagAtr attributes, uint8 value)
{
    FUNCTION_MARKER(0x80073230, "SLUS_005.78");
    return spu_vab_nrpn_attr_by_value(9, bank, program, tone, attributes, value);
}

uint32 p2_sdk_800732e8(sint16 bank, sint16 program, sint16 tone, VagAtr attributes, uint8 value)
{
    FUNCTION_MARKER(0x800732E8, "SLUS_005.78");
    return spu_vab_nrpn_attr_by_value(10, bank, program, tone, attributes, value);
}

uint32 p2_sdk_8007339c(sint16 bank, sint16 program, sint16 tone, VagAtr attributes, uint8 value)
{
    FUNCTION_MARKER(0x8007339C, "SLUS_005.78");
    return spu_vab_nrpn_attr_by_value(11, bank, program, tone, attributes, value);
}

uint32 p2_sdk_80073444(sint16 bank, sint16 program, sint16 tone, VagAtr attributes, uint8 value)
{
    FUNCTION_MARKER(0x80073444, "SLUS_005.78");
    return spu_vab_nrpn_attr_by_value(12, bank, program, tone, attributes, value);
}

uint32 p2_sdk_80073520(sint16 bank, sint16 program, sint16 tone, VagAtr attributes, uint8 value)
{
    FUNCTION_MARKER(0x80073520, "SLUS_005.78");
    return spu_vab_nrpn_attr_by_value(13, bank, program, tone, attributes, value);
}

uint32 p2_sdk_800735c0(sint16 bank, sint16 program, sint16 tone, VagAtr attributes, uint8 value)
{
    FUNCTION_MARKER(0x800735C0, "SLUS_005.78");
    return spu_vab_nrpn_attr_by_value(14, bank, program, tone, attributes, value);
}

uint32 p2_sdk_800756a8(sint16 sequence, sint16 separation)
{
    FUNCTION_MARKER(0x800756A8, "SLUS_005.78");
    return spu_sequence_scheduler_next_separation(&spu_sequence_scheduler, sequence, separation);
}

uint32 p2_sdk_80073a10(sint16 sequence, sint16 separation)
{
    FUNCTION_MARKER(0x80073A10, "SLUS_005.78");
    return spu_sequence_scheduler_meta_event(&spu_sequence_scheduler, sequence, separation, p2_sdk_80073d58);
}

uint32 p2_sdk_80072444(sint16 sequence, sint16 separation, uint8 value)
{
    FUNCTION_MARKER(0x80072444, "SLUS_005.78");
    return spu_sequence_scheduler_volume(&spu_sequence_scheduler, sequence, separation, value, p2_sdk_800794a0, p2_sdk_80073d58);
}

uint32 p2_sdk_80072518(sint16 sequence, sint16 separation, uint8 value)
{
    FUNCTION_MARKER(0x80072518, "SLUS_005.78");
    return spu_sequence_scheduler_pan(&spu_sequence_scheduler, sequence, separation, value, p2_sdk_800794a0, p2_sdk_80073d58);
}

uint32 p2_sdk_80072a94(sint16 sequence, sint16 separation)
{
    FUNCTION_MARKER(0x80072A94, "SLUS_005.78");
    return spu_sequence_scheduler_reset_controllers(&spu_sequence_scheduler, sequence, separation, p2_sdk_800764a0, p2_sdk_80073d58);
}

uint32 p2_sdk_80072728(sint16 sequence, sint16 separation, uint8 value)
{
    FUNCTION_MARKER(0x80072728, "SLUS_005.78");
    return spu_sequence_scheduler_nrpn_lsb(&spu_sequence_scheduler, sequence, separation, value, spu_sequence_dispatch_mark, p2_sdk_80073d58);
}

uint32 p2_sdk_800725e8(sint16 sequence, sint16 separation, uint8 value)
{
    FUNCTION_MARKER(0x800725E8, "SLUS_005.78");
    return spu_sequence_scheduler_damper(&spu_sequence_scheduler, sequence, separation, value, p2_sdk_80073d58);
}

uint32 p2_sdk_80072698(sint16 sequence, sint16 separation, uint8 value)
{
    FUNCTION_MARKER(0x80072698, "SLUS_005.78");
    return spu_sequence_scheduler_reverb_depth(&spu_sequence_scheduler, sequence, separation, value, p2_sdk_80076354, p2_sdk_80073d58);
}

uint32 p2_sdk_80072860(sint16 sequence, sint16 separation, uint8 value)
{
    FUNCTION_MARKER(0x80072860, "SLUS_005.78");
    return spu_sequence_scheduler_nrpn_msb(&spu_sequence_scheduler, sequence, separation, value, p2_sdk_80073d58);
}

uint32 p2_sdk_800729a4(sint16 sequence, sint16 separation, uint8 value)
{
    FUNCTION_MARKER(0x800729A4, "SLUS_005.78");
    return spu_sequence_scheduler_rpn_lsb(&spu_sequence_scheduler, sequence, separation, value, p2_sdk_80073d58);
}

uint32 p2_sdk_80072a1c(sint16 sequence, sint16 separation, uint8 value)
{
    FUNCTION_MARKER(0x80072A1C, "SLUS_005.78");
    return spu_sequence_scheduler_rpn_msb(&spu_sequence_scheduler, sequence, separation, value, p2_sdk_80073d58);
}

uint32 p2_sdk_800737d0(sint16 sequence, sint16 separation, uint8 controller)
{
    FUNCTION_MARKER(0x800737D0, "SLUS_005.78");
    return spu_sequence_scheduler_control_change(&spu_sequence_scheduler, sequence, separation, controller, &spu_sequence_handlers, p2_sdk_80073d58);
}

uint32 p2_sdk_800753c4(sint16 sequence, sint16 separation)
{
    FUNCTION_MARKER(0x800753C4, "SLUS_005.78");
    return spu_sequence_scheduler_read_event(&spu_sequence_scheduler, sequence, separation, &spu_sequence_handlers, p2_sdk_80073d58, p2_sdk_80075200);
}

uint32 p2_sdk_80075200(sint16 sequence, sint16 separation)
{
    FUNCTION_MARKER(0x80075200, "SLUS_005.78");
    return spu_sequence_scheduler_end_of_track(&spu_sequence_scheduler, sequence, separation, p2_sdk_800756a8, p2_sdk_80079350);
}

uint32 p2_sdk_80073d58(sint16 sequence, sint16 separation)
{
    FUNCTION_MARKER(0x80073D58, "SLUS_005.78");
    return spu_sequence_scheduler_read_delta(&spu_sequence_scheduler, sequence, separation);
}

uint32 p2_sdk_80073ce0(sint16 sequence, sint16 separation, uint8 program)
{
    FUNCTION_MARKER(0x80073CE0, "SLUS_005.78");
    return spu_sequence_scheduler_program_change(&spu_sequence_scheduler, sequence, separation, program, p2_sdk_80073d58);
}

sint32 p2_sdk_8006ff38(SpuReverbAttr *attributes)
{
    FUNCTION_MARKER(0x8006FF38, "SLUS_005.78");
    return SpuSetReverbModeParam(attributes);
}

uint32 p2_sdk_8007a740(uint32 mode)
{
    FUNCTION_MARKER(0x8007A740, "SLUS_005.78");
    return (uint32)(sint32)(sint16)SpuIsTransferCompleted((sint16)mode);
}

sint32 p2_sdk_80070df8(uint32 mask)
{
    FUNCTION_MARKER(0x80070DF8, "SLUS_005.78");
    return SpuGetKeyStatus(mask);
}

uint32 p2_sdk_80070eec(uint32 address)
{
    FUNCTION_MARKER(0x80070EEC, "SLUS_005.78");
    uint32 aligned, units;
    if (address - 0x1010 > 0x7EFE8)
        return 0;
    aligned = SpuSetTransferStartAddr(address);
    if (aligned == 0)
        return 0;
    units = (aligned >> (SPU_RAM_ADDRESS_SHIFT & 31)) & 0xFFFF;
    return units;
}

uint32 p2_sdk_800764a0(uint32 enabled)
{
    FUNCTION_MARKER(0x800764A0, "SLUS_005.78");
    return (uint32)SpuSetReverb((sint32)enabled);
}

uint32 p2_sdk_800750f0(sint16 sequence, sint16 separation)
{
    FUNCTION_MARKER(0x800750F0, "SLUS_005.78");
    return spu_sequence_scheduler_play_tick(&spu_sequence_scheduler, sequence, separation, p2_sdk_800753c4);
}

uint32 p2_sdk_800742f4(sint16 sequence, sint16 track)
{
    FUNCTION_MARKER(0x800742F4, "SLUS_005.78");
    return spu_sequence_scheduler_replay_mode(&spu_sequence_scheduler, sequence, track);
}

uint32 p2_sdk_80074398(sint16 sequence)
{
    FUNCTION_MARKER(0x80074398, "SLUS_005.78");
    return p2_sdk_800742f4(sequence, 0);
}

uint32 p2_sdk_800794a0(sint16 sequence, sint16 bank, sint16 program, uint16 volume, uint16 pan)
{
    FUNCTION_MARKER(0x800794A0, "SLUS_005.78");
    return spu_sequence_vm_set_volume(&spu_sequence_scheduler, sequence, bank, program, volume, pan);
}

uint32 p2_sdk_80070bc0(uint32 enabled, uint32 voices)
{
    FUNCTION_MARKER(0x80070BC0, "SLUS_005.78");
    return spu_set_key((sint32)enabled, voices);
}

uint32 p2_sdk_80078474(void)
{
    FUNCTION_MARKER(0x80078474, "SLUS_005.78");
    return spu_sequence_vm_key_off_now();
}

uint32 p2_sdk_80079350(sint16 sequence)
{
    FUNCTION_MARKER(0x80079350, "SLUS_005.78");
    return spu_sequence_vm_sequence_key_off(sequence);
}

uint32 p2_sdk_80075818(sint16 sequence, sint16 track)
{
    FUNCTION_MARKER(0x80075818, "SLUS_005.78");
    return spu_sequence_scheduler_stop(&spu_sequence_scheduler, sequence, track, p2_sdk_80079350);
}

uint32 p2_sdk_80075980(sint32 sequence)
{
    FUNCTION_MARKER(0x80075980, "SLUS_005.78");
    return p2_sdk_80075818((sint16)sequence, 0);
}

uint32 p2_sdk_80076354(sint16 left, sint16 right)
{
    FUNCTION_MARKER(0x80076354, "SLUS_005.78");
    return spu_sequence_vm_reverb_depth(left, right);
}

uint32 p2_sdk_800763ec(uint32 mode)
{
    FUNCTION_MARKER(0x800763EC, "SLUS_005.78");
    return spu_sequence_vm_reverb_type(mode);
}

uint32 p2_sdk_800759d4(SpuSequenceSchedulerTrack *table, sint16 sequences, sint16 separations)
{
    FUNCTION_MARKER(0x800759D4, "SLUS_005.78");
    return spu_sequence_scheduler_set_table(&spu_sequence_scheduler, table, sequences, separations);
}

uint32 p2_sdk_80078d1c(sint16 handle, sint16 left, sint16 right, sint16 mode)
{
    FUNCTION_MARKER(0x80078D1C, "SLUS_005.78");
    return spu_sequence_vm_set_sequence_volume(&spu_sequence_scheduler, handle, left, right, mode);
}

uint32 p2_sdk_80082a18(const char *format, ...)
{
    FUNCTION_MARKER(0x80082A18, "SLUS_005.78");
    va_list arguments;
    int result;
    va_start(arguments, format);
    result = vprintf(format, arguments);
    va_end(arguments);
    return (uint32)result;
}

uint32 p2_sdk_80079adc(uint8 voices)
{
    FUNCTION_MARKER(0x80079ADC, "SLUS_005.78");
    if (voices == 0 || voices >= 25)
        return 255;
    spu_vm_state()->events.voice_count = (uint8)(voices);
    return spu_vm_state()->events.voice_count;
}

uint32 p2_sdk_80079244(sint16 handle, uint16 *left, uint16 *right)
{
    FUNCTION_MARKER(0x80079244, "SLUS_005.78");
    return spu_sequence_vm_get_sequence_volume(&spu_sequence_scheduler, handle, left, right);
}

uint32 p2_sdk_80074210(sint16 sequence)
{
    FUNCTION_MARKER(0x80074210, "SLUS_005.78");
    return p2_func_80074138((uint32)(sint32)sequence, 0);
}

uint32 p2_sdk_80074264(sint32 sequence, sint32 mode, sint32 repeat)
{
    FUNCTION_MARKER(0x80074264, "SLUS_005.78");
    return p2_sdk_800743ec((sint16)sequence, 0, (uint8)mode, (sint16)repeat);
}

uint32 p2_sdk_80075d58(sint16 sequence, sint16 left, sint16 right)
{
    FUNCTION_MARKER(0x80075D58, "SLUS_005.78");
    return p2_sdk_80078d1c(sequence, left, right, 1);
}

uint32 p2_sdk_800743ec(sint16 sequence, sint16 track, uint8 mode, sint16 repeat)
{
    FUNCTION_MARKER(0x800743EC, "SLUS_005.78");
    return spu_sequence_scheduler_play_mode(&spu_sequence_scheduler, sequence, track, mode, repeat, p2_sdk_80078d1c);
}

uint32 p2_func_800750c4(sint16 sequence, sint16 separation)
{
    FUNCTION_MARKER(0x800750C4, "SLUS_005.78");
    return p2_sdk_800750f0(sequence, separation);
}

uint32 p2_func_800718ec(sint16 sequence)
{
    FUNCTION_MARKER(0x800718EC, "SLUS_005.78");
    return spu_sequence_scheduler_close(&spu_sequence_scheduler, sequence, p2_sdk_80078d1c, p2_sdk_80079350);
}

uint32 p2_sdk_8006fb68(const void *source, uint32 bytes)
{
    FUNCTION_MARKER(0x8006FB68, "SLUS_005.78");
    return SpuWrite(source, bytes);
}

uint32 p2_sdk_80079b14(sint32 bank)
{
    FUNCTION_MARKER(0x80079B14, "SLUS_005.78");
    return spu_vab_close_sticky((sint16)bank);
}

uint32 p2_sdk_80079f48(const uint8 *header, uint32 bytes, sint32 bank, uint32 address)
{
    FUNCTION_MARKER(0x80079F48, "SLUS_005.78");
    return (uint32)(sint32)spu_vab_open_head_sticky(header, bytes, (sint16)bank, address);
}

uint32 p2_sdk_8007a680(const uint8 *samples, uint32 sample_bytes, sint16 bank)
{
    FUNCTION_MARKER(0x8007A680, "SLUS_005.78");
    return (uint32)(sint32)spu_vab_trans_body_native(samples, sample_bytes, bank);
}

uint32 p2_sdk_800767dc(void)
{
    FUNCTION_MARKER(0x800767DC, "SLUS_005.78");
    return spu_sequence_vm_flush();
}

uint32 p2_sdk_800770d0(sint32 count, SpuMallocRecord *table)
{
    FUNCTION_MARKER(0x800770D0, "SLUS_005.78");
    return (uint32)SpuInitMalloc(count, (char *)table);
}

uint32 p2_sdk_80076dc0(uint8 voices)
{
    FUNCTION_MARKER(0x80076DC0, "SLUS_005.78");
    return spu_sequence_vm_initialize(voices);
}

uint32 p2_sdk_80071b74(void)
{
    FUNCTION_MARKER(0x80071B74, "SLUS_005.78");
    return spu_sequence_scheduler_initialize(&spu_sequence_scheduler, p2_sdk_80076dc0);
}

uint32 p2_sdk_80077168(sint16 sequence, sint16 bank, sint16 program, uint16 note, uint16 velocity, uint16 pan)
{
    FUNCTION_MARKER(0x80077168, "SLUS_005.78");
    return spu_sequence_vm_key_on(&spu_sequence_scheduler, sequence, bank, program, note, velocity, pan);
}

uint32 p2_sdk_80073bf0(sint16 sequence, sint16 separation, uint8 note, uint8 velocity)
{
    FUNCTION_MARKER(0x80073BF0, "SLUS_005.78");
    return spu_sequence_scheduler_note_on(&spu_sequence_scheduler, sequence, separation, note, velocity, p2_sdk_80077168, p2_sdk_800776bc);
}

uint32 p2_sdk_80077f60(uint8 voice)
{
    FUNCTION_MARKER(0x80077F60, "SLUS_005.78");
    return spu_sequence_vm_noise_on(&spu_sequence_scheduler, voice);
}

uint32 p2_sdk_80078544(uint32 layers, sint16 pitch)
{
    FUNCTION_MARKER(0x80078544, "SLUS_005.78");
    return spu_sequence_vm_key_on_now(&spu_sequence_scheduler, layers, pitch);
}

uint32 p2_sdk_80076b94(uint32 mode, uint32 voices)
{
    FUNCTION_MARKER(0x80076B94, "SLUS_005.78");
    return spu_sequence_vm_noise_voice(mode, voices);
}

uint32 p2_sdk_80077930(void)
{
    FUNCTION_MARKER(0x80077930, "SLUS_005.78");
    return spu_sequence_vm_allocate();
}

uint32 p2_sdk_80078430(uint8 voice)
{
    FUNCTION_MARKER(0x80078430, "SLUS_005.78");
    return spu_sequence_vm_noise_off(voice);
}

uint32 p2_sdk_800776bc(sint16 sequence, sint16 bank, sint16 program, uint16 note)
{
    FUNCTION_MARKER(0x800776BC, "SLUS_005.78");
    return spu_sequence_vm_key_off(sequence, bank, program, note);
}

uint32 p2_sdk_800793ec(uint8 *tones, uint8 *samples)
{
    FUNCTION_MARKER(0x800793EC, "SLUS_005.78");
    return spu_sequence_vm_select_tones(tones, samples);
}

uint32 p2_sdk_80077b98(void)
{
    FUNCTION_MARKER(0x80077B98, "SLUS_005.78");
    return spu_sequence_vm_prepare_voice();
}

uint16 p2_sdk_80077dac(void)
{
    FUNCTION_MARKER(0x80077DAC, "SLUS_005.78");
    return spu_sequence_vm_note_pitch();
}

static uint16 p2_tone_pitch_80077e64(uint16 note, uint16 fraction)
{
    FUNCTION_MARKER(0x80077E64, "SLUS_005.78");
    return spu_sequence_vm_tone_pitch(note, fraction);
}

static uint32 p2_voice_bend_80078a28(sint16 voice, sint16 sequence, sint16 bank, sint16 program, uint16 bend)
{
    FUNCTION_MARKER(0x80078A28, "SLUS_005.78");
    return spu_sequence_vm_voice_bend(voice, sequence, bank, program, bend);
}

static uint32 p2_sequence_bend_80078c2c(sint16 sequence, sint16 bank, sint16 program, uint16 bend)
{
    FUNCTION_MARKER(0x80078C2C, "SLUS_005.78");
    return spu_sequence_vm_sequence_bend(sequence, bank, program, bend);
}

uint32 p2_sdk_80073718(sint16 sequence, sint16 separation)
{
    FUNCTION_MARKER(0x80073718, "SLUS_005.78");
    return spu_sequence_scheduler_pitch_bend(&spu_sequence_scheduler, sequence, separation, p2_sequence_bend_80078c2c, p2_sdk_80073d58);
}

uint32 p2_sdk_8007201c(sint16 sequence, sint16 separation, uint8 value)
{
    FUNCTION_MARKER(0x8007201C, "SLUS_005.78");
    SpuSequenceSchedulerTrack *record = spu_sequence_scheduler.tracks[(uint32)(sint32)sequence] + (uint32)(sint32)separation;
    return spu_sequence_scheduler_data_entry(record, sequence, separation, value, spu_tone_commands, 20, p2_sdk_80073d58);
}

static uint32 p2_sequence_init_80073e10(sint16 sequence, sint16 bank, const uint8 *data)
{
    FUNCTION_MARKER(0x80073E10, "SLUS_005.78");
    return spu_sequence_scheduler_initialize_track(&spu_sequence_scheduler, sequence, bank, data, p2_sdk_80073d58);
}

uint32 p2_sdk_80071c8c(const uint8 *sequence, uint32 bytes, sint16 bank)
{
    FUNCTION_MARKER(0x80071C8C, "SLUS_005.78");
    return spu_sequence_scheduler_open(&spu_sequence_scheduler, sequence, bytes, bank, p2_sequence_init_80073e10, &spu_sequence_handlers, &sequence_default_handlers, spu_tone_commands, sequence_default_commands);
}

PsxInterruptCallback p2_sdk_8007febc(PsxInterruptCallback callback)
{
    FUNCTION_MARKER(0x8007FEBC, "SLUS_005.78");
    if ((uint8)graph_debug_level >= 2)
        printf("DrawSyncCallback(%p)...\n", (void *)callback);
    return DrawSyncCallback(callback);
}

uint32 p2_sdk_800804bc(uint32 table, sint32 count)
{
    FUNCTION_MARKER(0x800804BC, "SLUS_005.78");
    if ((uint8)graph_debug_level >= 2)
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

uint32 p2_sdk_80070a24(uint32 mode)
{
    FUNCTION_MARKER(0x80070A24, "SLUS_005.78");
    return (uint32)SpuClearReverbWorkArea((sint32)mode);
}

static void p2_sound_irq0_80074790(void)
{
    FUNCTION_MARKER(0x80074790, "SLUS_005.78");
    spu_sequence_scheduler_irq0(spu_sequence_clock());
}

static void p2_sound_half_rate_800747d0(void)
{
    FUNCTION_MARKER(0x800747D0, "SLUS_005.78");
    spu_sequence_scheduler_half_rate(spu_sequence_clock());
}

void p2_sdk_800744ec(uint32 mode)
{
    FUNCTION_MARKER(0x800744EC, "SLUS_005.78");
    spu_sequence_scheduler_start_timer(spu_sequence_clock(), mode, p2_sound_irq0_80074790, p2_sound_half_rate_800747d0);
}

void p2_sdk_80074820(void)
{
    FUNCTION_MARKER(0x80074820, "SLUS_005.78");
    spu_sequence_scheduler_pump(&spu_sequence_scheduler, &sequence_pump_ops);
}

uint32 p2_sdk_80074a98(sint16 sequence, sint16 separation)
{
    FUNCTION_MARKER(0x80074A98, "SLUS_005.78");
    return spu_sequence_scheduler_crescendo(&spu_sequence_scheduler, sequence, separation, p2_sdk_80079244, p2_sdk_80078d1c);
}

uint32 p2_sdk_80074d74(sint16 sequence, sint16 separation)
{
    FUNCTION_MARKER(0x80074D74, "SLUS_005.78");
    return spu_sequence_scheduler_decrescendo(&spu_sequence_scheduler, sequence, separation, p2_sdk_80079244, p2_sdk_80078d1c);
}

uint32 p2_sdk_80075020(sint16 sequence, sint16 separation)
{
    FUNCTION_MARKER(0x80075020, "SLUS_005.78");
    return spu_sequence_scheduler_pause(&spu_sequence_scheduler, sequence, separation, p2_sdk_80079350);
}

void ResumeSequenceTrack(sint16 sequence, sint16 separation)
{
    FUNCTION_MARKER(0x800757B0, "SLUS_005.78");
    spu_sequence_scheduler_resume(&spu_sequence_scheduler, sequence, separation);
}

void AdvanceSequenceTempoRamp(sint16 sequence, sint16 separation)
{
    FUNCTION_MARKER(0x80075DF4, "SLUS_005.78");
    spu_sequence_scheduler_tempo_ramp(&spu_sequence_scheduler, sequence, separation);
}

uint32 p2_sdk_80075bb4(uint32 mode)
{
    FUNCTION_MARKER(0x80075BB4, "SLUS_005.78");
    return spu_sequence_scheduler_tick_mode(&spu_sequence_scheduler, spu_sequence_clock(), mode, p2_func_8007b448());
}

uint32 p2_func_80073660(uint8 value)
{
    FUNCTION_MARKER(0x80073660, "SLUS_005.78");
    return p2_sdk_800763ec(value);
}

uint32 SetSequenceMasterVolume(uint8 value)
{
    FUNCTION_MARKER(0x80073684, "SLUS_005.78");
    return p2_sdk_80076354(value, value);
}

uint32 p2_func_800736ac(uint8 value)
{
    FUNCTION_MARKER(0x800736AC, "SLUS_005.78");
    return spu_sequence_vm_reverb_feedback(value);
}

uint32 p2_func_800736d0(uint8 value)
{
    FUNCTION_MARKER(0x800736D0, "SLUS_005.78");
    return spu_sequence_vm_reverb_delay(value);
}

uint32 p2_func_800736f4(uint8 value)
{
    FUNCTION_MARKER(0x800736F4, "SLUS_005.78");
    return p2_func_800736d0(value);
}

uint32 p2_sdk_8007101c(const SpuVoiceAttr *attributes)
{
    FUNCTION_MARKER(0x8007101C, "SLUS_005.78");
    SpuSetVoiceAttr(attributes);
    return 0;
}

uint32 p2_sdk_80070dc4(const SpuVoiceAttr *attributes)
{
    FUNCTION_MARKER(0x80070DC4, "SLUS_005.78");
    p2_sdk_8007101c(attributes);
    return p2_sdk_80070bc0(1, attributes->voice);
}

PsxCallbackDriver *p2_reset_callback(void)
{
    FUNCTION_MARKER(0x8007A950, "SLUS_005.78");
    return psx_callback_reset_driver();
}

static void p2_sound_reset_8006f2ec(uint32 mode)
{
    FUNCTION_MARKER(0x8006F2EC, "SLUS_005.78");
    spu_sequence_vm_reset_device(mode);
}

static void p2_sound_dma_8006f74c(void)
{
    FUNCTION_MARKER(0x8006F74C, "SLUS_005.78");
    spu_sequence_vm_dma();
}

static PsxInterruptCallback p2_sound_register_dma_8006fe98(PsxInterruptCallback callback)
{
    FUNCTION_MARKER(0x8006FE98, "SLUS_005.78");
    return spu_sequence_vm_register_dma(callback);
}

static void p2_sound_quit_8006febc(void)
{
    FUNCTION_MARKER(0x8006FEBC, "SLUS_005.78");
    spu_sequence_vm_quit();
}

uint32 p2_sdk_8006f178(uint32 mode)
{
    FUNCTION_MARKER(0x8006F178, "SLUS_005.78");
    return spu_sequence_vm_initialize_device(mode, p2_sound_dma_8006f74c);
}

static void p2_sound_stop_timer_80071ab0(void)
{
    FUNCTION_MARKER(0x80071AB0, "SLUS_005.78");
    spu_sequence_scheduler_stop_timer(spu_sequence_clock());
}

void p2_func_8006af4c(void)
{
    FUNCTION_MARKER(0x8006AF4C, "SLUS_005.78");
    p2_sdk_80070bc0(0, 0x00FF0000);
    p2_sound_stop_timer_80071ab0();
    p2_sound_quit_8006febc();
    p2_sound_quit_8006febc();
}

void p2_func_800742d4(void)
{
    FUNCTION_MARKER(0x800742D4, "SLUS_005.78");
    p2_sound_quit_8006febc();
}

void p2_func_80074770(void)
{
    FUNCTION_MARKER(0x80074770, "SLUS_005.78");
    p2_sdk_800744ec(0);
}

void p2_func_8008347c(void)
{
    FUNCTION_MARKER(0x8008347C, "SLUS_005.78");
    xport_bios_exit_critical();
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
    return spu_sequence_vm_transfer_mode(mode);
}

uint32 p2_func_8007b448(void)
{
    FUNCTION_MARKER(0x8007B448, "SLUS_005.78");
    return GetVideoMode();
}

void p2_func_80074750(void)
{
    FUNCTION_MARKER(0x80074750, "SLUS_005.78");
    p2_sdk_800744ec(1);
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

uint32 p2_func_80077150(void)
{
    FUNCTION_MARKER(0x80077150, "SLUS_005.78");
    return spu_native_transfer_state->completion != 1;
}

uint32 p2_func_80070e8c(const void *source, uint32 bytes)
{
    uint32 length = bytes;
    FUNCTION_MARKER(0x80070E8C, "SLUS_005.78");
    if (length > 0x7EFF0)
        length = 0x7EFF0;
    p2_sdk_8006fb68(source, length);
    if (spu_native_transfer_state->completion_callback == 0)
        spu_native_transfer_state->completion = 0;
    return length;
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

uint32 p2_func_8006ec24(uint32 left_low, uint32 left_high, uint32 right_low, uint32 right_high)
{
    FUNCTION_MARKER(0x8006EC24, "SLUS_005.78");
    if (left_high == right_high && left_low == right_low)
        return 0;
    return (left_high & 0x7FFFFFFF) != 0 || left_low != 0 || (right_high & 0x7FFFFFFF) != 0 || right_low != 0;
}

CdlCB SetCdReadCallback(CdlCB callback)
{
    FUNCTION_MARKER(0x8007E340, "SLUS_005.78");
    return CdReadCallback(callback);
}

uint32 p2_func_8006a614(void)
{
    FUNCTION_MARKER(0x8006A614, "SLUS_005.78");
    return p2_sdk_80070bc0(0, 0xFF0000);
}

uint32 p2_func_80071a68(sint16 sequence)
{
    FUNCTION_MARKER(0x80071A68, "SLUS_005.78");
    return p2_func_800718ec(sequence);
}

uint32 p2_reset_graph_zero(void)
{
    FUNCTION_MARKER(0x8007FB14, "SLUS_005.78");
    uint32 mask, graph_type;
    sint32 result;
    printf("ResetGraph:jtb=%08x,env=%08x\n", 0x80089F3C, 0x80089F84);
    gpu_graph_control.type = 0;
    gpu_graph_control.queue_enabled = 0;
    SetGraphDebug(0);
    gpu_graph_control.reverse = 0;
    DrawSyncCallback(NULL);
    p2_reset_callback();
    mask = psx_irq_native_mask_exchange(0);
    result = ResetGraph(0);
    if (result < 0)
    {
        fprintf(stderr, "P2: Native ResetGraph failed\n");
        abort();
    }
    SetDispMask(0);
    memset(gpu_graph_control.command_cache, 0, sizeof(gpu_graph_control.command_cache));
    psx_irq_native_mask_exchange((uint16)mask);
    graph_type = (uint8)GetGraphType();
    gpu_graph_control.type = (uint8)graph_type;
    gpu_graph_control.queue_enabled = 1;
    return graph_type;
}

uint32 p2_func_80071a8c(sint16 sequence)
{
    FUNCTION_MARKER(0x80071A8C, "SLUS_005.78");
    return p2_func_800718ec(sequence);
}

uint32 p2_func_80074238(sint16 sequence, sint16 track)
{
    FUNCTION_MARKER(0x80074238, "SLUS_005.78");
    return p2_func_80074138((uint32)(sint32)sequence, (uint32)(sint32)track);
}

uint32 p2_func_800743c0(sint16 sequence, sint16 track)
{
    FUNCTION_MARKER(0x800743C0, "SLUS_005.78");
    return p2_sdk_800742f4(sequence, track);
}

uint32 p2_func_800759a8(sint16 sequence, sint16 track)
{
    FUNCTION_MARKER(0x800759A8, "SLUS_005.78");
    return p2_sdk_80075818(sequence, track);
}

void p2_func_800767b8(void)
{
    FUNCTION_MARKER(0x800767B8, "SLUS_005.78");
    spu_vm_state()->release_offset = 0;
}

uint32 p2_func_800767c8(void)
{
    FUNCTION_MARKER(0x800767C8, "SLUS_005.78");
    spu_vm_state()->release_offset = 2;
    return 2;
}

uint32 p2_func_8006f824(uint32 source, uint16 address_units, uint32 blocks, uint32 port)
{
    FUNCTION_MARKER(0x8006F824, "SLUS_005.78");
    uint32 control;
    w_u16(port + 422, address_units);
    port = SPU_REGISTER_BASE;
    w_u16(port + 426, r_u16(port + 426) | 0x30);
    control = SPU_DMA_DELAY_REGISTER;
    w_u32(control, (r_u32(control) & 0xD0FFFFFF) | 0x22000000);
    w_u32(SPU_DMA_ADDRESS_REGISTER, source);
    w_u32(SPU_DMA_BLOCK_REGISTER, (blocks << 16) | 0x10);
    w_u32(SPU_DMA_CONTROL_REGISTER, 0x01000200);
    return 1;
}

uint32 p2_func_8008040c(uint32 ordering_table, sint32 count)
{
    FUNCTION_MARKER(0x8008040C, "SLUS_005.78");
    uint32 cursor = ordering_table;
    if ((uint8)graph_debug_level >= 2)
        p2_sdk_80082a18("ClearOTag(%08x,%d)...\n", ordering_table, (uint32)count);
    if (count <= 0)
    {
        fprintf(stderr, "P2: Invalid ClearOTag count %d\n", count);
        abort();
    }
    while (--count != 0)
    {
        w_u8(cursor + 3, 0);
        w_u32(cursor, (r_u32(cursor) & 0xFF000000) | ((cursor + 4) & 0xFFFFFF));
        cursor += 4;
    }
    w_u32(cursor, 0x0008A040);
    return cursor;
}

void p2_func_8008342c(void)
{
    FUNCTION_MARKER(0x8008342C, "SLUS_005.78");
    PadStopCom();
}

PsxInterruptCallback p2_func_8007aa1c(uint32 channel, PsxInterruptCallback callback)
{
    FUNCTION_MARKER(0x8007AA1C, "SLUS_005.78");
    return setIntrVSync(channel, callback);
}

PsxCallbackDriver *p2_func_8007aa7c(void)
{
    FUNCTION_MARKER(0x8007AA7C, "SLUS_005.78");
    return restartIntr();
}

uint32 p2_func_8007fc78(uint32 reverse)
{
    FUNCTION_MARKER(0x8007FC78, "SLUS_005.78");
    uint32 previous = gpu_graph_control.reverse, mode;
    if ((uint8)graph_debug_level >= 2)
        p2_sdk_80082a18("SetGraphReverse(%d)...\n", reverse);
    gpu_graph_control.reverse = (uint8)reverse;
    mode = _getctl(8);
    _ctl(mode | (gpu_graph_control.reverse != 0 ? 0x08000080 : 0x08000000));
    if (gpu_graph_control.type == 2)
        _ctl(gpu_graph_control.reverse != 0 ? 0x20000501 : 0x20000504);
    return previous;
}

uint32 p2_func_8007fdf8(uint32 enabled)
{
    FUNCTION_MARKER(0x8007FDF8, "SLUS_005.78");
    uint32 previous = gpu_graph_control.queue_enabled;
    uint16 mask;
    if ((uint8)graph_debug_level >= 2)
        p2_sdk_80082a18("SetGrapQue(%d)...\n", enabled);
    if (enabled == gpu_graph_control.queue_enabled)
        return previous;
    mask = psx_irq_native_mask_exchange(0);
    w_u32(0x1F8010A8u, 1025);
    w_u32(0x1F8010F0u, r_u32(0x1F8010F0u) | 0x800);
    w_u32(0x1F801814u, 0x02000000);
    w_u32(0x1F801814u, 0x01000000);
    psx_irq_native_mask_exchange(mask);
    gpu_graph_control.queue_enabled = (uint8)enabled;
    if (psx_callback_driver.initialized != 0)
        InterruptCallback(2, NULL);
    return previous;
}

uint32 p2_func_80080cf0(void)
{
    FUNCTION_MARKER(0x80080CF0, "SLUS_005.78");
    return _status() >> 31;
}

uint32 p2_func_8008373c(uint32 type, uint32 destination)
{
    FUNCTION_MARKER(0x8008373C, "SLUS_005.78");
    return PAD_init2(type, destination);
}

uint32 p2_func_8006aaa8(void)
{
    FUNCTION_MARKER(0x8006AAA8, "SLUS_005.78");
    return SpuIsTransferCompleted(0) != 0;
}
