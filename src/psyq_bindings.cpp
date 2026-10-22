#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "game.h"
#include "psx_gpu.h"
#include "xport_trace.h"

static uint32 SetTonePriority(sint16 bank, sint16 program, sint16 tone, VagAtr attributes, uint8 val);
static uint32 SetToneReverbMode(sint16 bank, sint16 program, sint16 tone, VagAtr attributes, uint8 val);
static uint32 SetToneMinimumNote(sint16 bank, sint16 program, sint16 tone, VagAtr attributes, uint8 val);
static uint32 SetToneMaximumNote(sint16 bank, sint16 program, sint16 tone, VagAtr attributes, uint8 val);
static uint32 SetToneLinearAttackRate(sint16 bank, sint16 program, sint16 tone, VagAtr attributes, uint8 val);
static uint32 SetToneExponentialAttackRate(sint16 bank, sint16 program, sint16 tone, VagAtr attributes, uint8 val);
static uint32 SetToneDecayRate(sint16 bank, sint16 program, sint16 tone, VagAtr attributes, uint8 val);
static uint32 SetToneSustainLevel(sint16 bank, sint16 program, sint16 tone, VagAtr attributes, uint8 val);
static uint32 SetToneLinearSustainRate(sint16 bank, sint16 program, sint16 tone, VagAtr attributes, uint8 val);
static uint32 SetToneExponentialSustainRate(sint16 bank, sint16 program, sint16 tone, VagAtr attributes, uint8 val);
static uint32 SetToneLinearReleaseRate(sint16 bank, sint16 program, sint16 tone, VagAtr attributes, uint8 val);
static uint32 SetToneExponentialReleaseRate(sint16 bank, sint16 program, sint16 tone, VagAtr attributes, uint8 val);
static uint32 SetToneSustainDir(sint16 bank, sint16 program, sint16 tone, VagAtr attributes, uint8 val);
static uint32 SetToneVibratoTime(sint16 bank, sint16 program, sint16 tone, VagAtr attributes, uint8 val);
static uint32 SetTonePortamentoWidth(sint16 bank, sint16 program, sint16 tone, VagAtr attributes, uint8 val);
static uint32 ProcessSequenceMetaEvent(sint16 sequence, sint16 separation);
static uint32 ProcessSequenceVolume(sint16 sequence, sint16 separation, uint8 val);
static uint32 ProcessSequencePan(sint16 sequence, sint16 separation, uint8 val);
static uint32 ResetSequenceControllers(sint16 sequence, sint16 separation);
static uint32 ProcessSequenceNrpnLsb(sint16 sequence, sint16 separation, uint8 val);
static uint32 ProcessSequenceDamper(sint16 sequence, sint16 separation, uint8 val);
static uint32 ProcessSequenceReverbDepth(sint16 sequence, sint16 separation, uint8 val);
static uint32 ProcessSequenceNrpnMsb(sint16 sequence, sint16 separation, uint8 val);
static uint32 ProcessSequenceRpnLsb(sint16 sequence, sint16 separation, uint8 val);
static uint32 ProcessSequenceRpnMsb(sint16 sequence, sint16 separation, uint8 val);
static uint32 ProcessSequenceControlChange(sint16 sequence, sint16 separation, uint8 pad);
static uint32 ProcessSequenceEndOfTrack(sint16 sequence, sint16 separation);
static uint32 ReadSequenceDelta(sint16 sequence, sint16 separation);
static uint32 ProcessSequenceProgramChange(sint16 sequence, sint16 separation, uint8 program);
static uint32 SetSequenceReverbEnabled(uint32 enabled);
static uint32 SetSequenceVoiceVolume(sint16 sequence, sint16 bank, sint16 program, uint16 volume, uint16 pan);
static uint32 ReleaseSequenceVoices(sint16 sequence);
static uint32 StopSequenceTrack(sint16 sequence, sint16 track);
static uint32 GetSequenceVolume(sint16 handle, uint16 *left, uint16 *right);
static uint32 SetSequenceTrackPlayMode(sint16 sequence, sint16 track, uint8 mode, sint16 repeat);
static uint32 AdvanceSequenceTrack(sint16 sequence, sint16 separation);
static uint32 FlushSequenceVoices(void);
static uint32 ProcessSequenceNoteOn(sint16 sequence, sint16 separation, uint8 note, uint8 velocity);
static uint32 KeyOffSequenceVoice(sint16 sequence, sint16 bank, sint16 program, uint16 note);
static uint32 ProcessSequencePitchBend(sint16 sequence, sint16 separation);
static uint32 ProcessSequenceDataEntry(sint16 sequence, sint16 separation, uint8 val);
static uint32 AdvanceSequenceCrescendo(sint16 sequence, sint16 separation);
static uint32 AdvanceSequenceDecrescendo(sint16 sequence, sint16 separation);
static uint32 PauseSequenceTrack(sint16 sequence, sint16 separation);
static void ResumeSequenceTrack(sint16 sequence, sint16 separation);
static void AdvanceSequenceTempoRamp(sint16 sequence, sint16 separation);
static uint32 ProcessReverbTypeCommand(uint8 val);
static uint32 ProcessReverbDepthCommand(uint8 val);
static uint32 ProcessReverbFeedbackCommand(uint8 val);
static uint32 ProcessReverbDelayCommand(uint8 val);
static uint32 ProcessReverbDelayAliasCommand(uint8 val);
static uint32 ReadVideoMode(void);

static void ResetSpuDevice(uint32 mode);
static void HandleSpuDma(void);
static PsxInterruptCallback RegisterSpuDmaCallback(PsxInterruptCallback callback);
static void QuitSpuDevice(void);
static void StopSequenceTimer(void);
static uint16 ComputeSequenceTonePitch(uint16 note, uint16 fraction);
static uint32 BendSequenceVoicePitch(sint16 voice, sint16 sequence, sint16 bank, sint16 program, uint16 bend);
static uint32 BendSequencePitch(sint16 sequence, sint16 bank, sint16 program, uint16 bend);
static uint32 InitSequenceTrack(sint16 sequence, sint16 bank, const uint8 *data);
static void HandleSequenceTimerIrq(void);
static void HandleSequenceHalfRateTick(void);

static const SpuSequenceHandlers sequence_default_handlers = {ProcessSequenceNoteOn, ProcessSequenceProgramChange, ProcessSequencePitchBend, ProcessSequenceMetaEvent, ProcessSequenceControlChange, ProcessSequenceDataEntry, ProcessSequenceVolume, ProcessSequencePan, ProcessSequenceDamper, ProcessSequenceNrpnLsb, ProcessSequenceNrpnMsb, ProcessSequenceRpnLsb, ProcessSequenceRpnMsb, ProcessSequenceReverbDepth, ResetSequenceControllers};

static const SpuToneCommand sequence_default_commands[20] = {{SetTonePriority, NULL}, {SetToneReverbMode, NULL}, {SetToneMinimumNote, NULL}, {SetToneMaximumNote, NULL}, {SetToneLinearAttackRate, NULL}, {SetToneExponentialAttackRate, NULL}, {SetToneDecayRate, NULL}, {SetToneSustainLevel, NULL}, {SetToneLinearSustainRate, NULL}, {SetToneExponentialSustainRate, NULL}, {SetToneLinearReleaseRate, NULL}, {SetToneExponentialReleaseRate, NULL}, {SetToneSustainDir, NULL}, {SetToneVibratoTime, NULL}, {SetTonePortamentoWidth, NULL}, {NULL, ProcessReverbTypeCommand}, {NULL, ProcessReverbDepthCommand}, {NULL, ProcessReverbFeedbackCommand}, {NULL, ProcessReverbDelayCommand}, {NULL, ProcessReverbDelayAliasCommand}};

static const SpuSequencePumpOps sequence_pump_ops = {FlushSequenceVoices, AdvanceSequenceTrack, AdvanceSequenceCrescendo, AdvanceSequenceDecrescendo, AdvanceSequenceTempoRamp, PauseSequenceTrack, ResumeSequenceTrack, StopSequenceTrack};

static uint32 RequestSequenceTrackPause(uint32 sequence, uint32 track)
{
    FUNCTION_MARKER(0x80074138, "SLUS_005.78");
    return spu_sequence_scheduler_request_pause(&spu_sequence_scheduler, sequence, track, GetSequenceVolume);
}

static sint32 SelectVabProgram(uint16 bank, sint16 program)
{
    FUNCTION_MARKER(0x80079A18, "SLUS_005.78");
    return spu_vab_select_native(bank, program);
}

static sint32 GetVabProgramAttributes(sint16 bank, sint16 program, uint8 *attributes)
{
    FUNCTION_MARKER(0x80075FD0, "SLUS_005.78");
    return SsUtGetProgAtr(bank, program, (ProgAtr *)attributes);
}

static sint32 GetVabToneAttributes(sint16 bank, sint16 program, uint32 tone, uint8 *attributes)
{
    FUNCTION_MARKER(0x800760D8, "SLUS_005.78");
    return spu_vab_get_vag_atr_native(bank, program, tone, (VagAtr *)attributes);
}

static sint32 SetVabToneAttributes(sint16 bank, sint16 program, uint32 tone, const uint8 *attributes)
{
    FUNCTION_MARKER(0x800765F0, "SLUS_005.78");
    return SsUtSetVagAtr(bank, program, tone, (const VagAtr *)attributes);
}

static uint32 DecodeToneAdsr(uint16 adsr0, uint16 adsr1, uint16 *fields)
{
    FUNCTION_MARKER(0x80072E7C, "SLUS_005.78");
    return _SsUtResolveADSR(adsr0, adsr1, fields);
}

static uint32 EncodeToneAdsr(const uint16 *fields, uint16 *adsr0, uint16 *adsr1)
{
    FUNCTION_MARKER(0x80072ED8, "SLUS_005.78");
    return _SsUtBuildADSR(fields, adsr0, adsr1);
}

static uint32 SetTonePriority(sint16 bank, sint16 program, sint16 tone, VagAtr attributes, uint8 val)
{
    FUNCTION_MARKER(0x80072B5C, "SLUS_005.78");
    return spu_vab_nrpn_attr_by_value(0, bank, program, tone, attributes, val);
}

static uint32 SetToneReverbMode(sint16 bank, sint16 program, sint16 tone, VagAtr attributes, uint8 val)
{
    FUNCTION_MARKER(0x80072BEC, "SLUS_005.78");
    return spu_vab_nrpn_attr_by_value(1, bank, program, tone, attributes, val);
}

static uint32 SetToneMinimumNote(sint16 bank, sint16 program, sint16 tone, VagAtr attributes, uint8 val)
{
    FUNCTION_MARKER(0x80072CA8, "SLUS_005.78");
    return spu_vab_nrpn_attr_by_value(2, bank, program, tone, attributes, val);
}

static uint32 SetToneMaximumNote(sint16 bank, sint16 program, sint16 tone, VagAtr attributes, uint8 val)
{
    FUNCTION_MARKER(0x80072D38, "SLUS_005.78");
    return spu_vab_nrpn_attr_by_value(3, bank, program, tone, attributes, val);
}

static uint32 SetToneLinearAttackRate(sint16 bank, sint16 program, sint16 tone, VagAtr attributes, uint8 val)
{
    FUNCTION_MARKER(0x80072DC8, "SLUS_005.78");
    return spu_vab_nrpn_attr_by_value(4, bank, program, tone, attributes, val);
}

static uint32 SetToneExponentialAttackRate(sint16 bank, sint16 program, sint16 tone, VagAtr attributes, uint8 val)
{
    FUNCTION_MARKER(0x80072F64, "SLUS_005.78");
    return spu_vab_nrpn_attr_by_value(5, bank, program, tone, attributes, val);
}

static uint32 SetToneDecayRate(sint16 bank, sint16 program, sint16 tone, VagAtr attributes, uint8 val)
{
    FUNCTION_MARKER(0x8007301C, "SLUS_005.78");
    return spu_vab_nrpn_attr_by_value(6, bank, program, tone, attributes, val);
}

static uint32 SetToneSustainLevel(sint16 bank, sint16 program, sint16 tone, VagAtr attributes, uint8 val)
{
    FUNCTION_MARKER(0x800730CC, "SLUS_005.78");
    return spu_vab_nrpn_attr_by_value(7, bank, program, tone, attributes, val);
}

static uint32 SetToneLinearSustainRate(sint16 bank, sint16 program, sint16 tone, VagAtr attributes, uint8 val)
{
    FUNCTION_MARKER(0x8007317C, "SLUS_005.78");
    return spu_vab_nrpn_attr_by_value(8, bank, program, tone, attributes, val);
}

static uint32 SetToneExponentialSustainRate(sint16 bank, sint16 program, sint16 tone, VagAtr attributes, uint8 val)
{
    FUNCTION_MARKER(0x80073230, "SLUS_005.78");
    return spu_vab_nrpn_attr_by_value(9, bank, program, tone, attributes, val);
}

static uint32 SetToneLinearReleaseRate(sint16 bank, sint16 program, sint16 tone, VagAtr attributes, uint8 val)
{
    FUNCTION_MARKER(0x800732E8, "SLUS_005.78");
    return spu_vab_nrpn_attr_by_value(10, bank, program, tone, attributes, val);
}

static uint32 SetToneExponentialReleaseRate(sint16 bank, sint16 program, sint16 tone, VagAtr attributes, uint8 val)
{
    FUNCTION_MARKER(0x8007339C, "SLUS_005.78");
    return spu_vab_nrpn_attr_by_value(11, bank, program, tone, attributes, val);
}

static uint32 SetToneSustainDir(sint16 bank, sint16 program, sint16 tone, VagAtr attributes, uint8 val)
{
    FUNCTION_MARKER(0x80073444, "SLUS_005.78");
    return spu_vab_nrpn_attr_by_value(12, bank, program, tone, attributes, val);
}

static uint32 SetToneVibratoTime(sint16 bank, sint16 program, sint16 tone, VagAtr attributes, uint8 val)
{
    FUNCTION_MARKER(0x80073520, "SLUS_005.78");
    return spu_vab_nrpn_attr_by_value(13, bank, program, tone, attributes, val);
}

static uint32 SetTonePortamentoWidth(sint16 bank, sint16 program, sint16 tone, VagAtr attributes, uint8 val)
{
    FUNCTION_MARKER(0x800735C0, "SLUS_005.78");
    return spu_vab_nrpn_attr_by_value(14, bank, program, tone, attributes, val);
}

static uint32 AdvanceSequenceSeparation(sint16 sequence, sint16 separation)
{
    FUNCTION_MARKER(0x800756A8, "SLUS_005.78");
    return spu_sequence_scheduler_next_separation(&spu_sequence_scheduler, sequence, separation);
}

static uint32 ProcessSequenceMetaEvent(sint16 sequence, sint16 separation)
{
    FUNCTION_MARKER(0x80073A10, "SLUS_005.78");
    return spu_sequence_scheduler_meta_event(&spu_sequence_scheduler, sequence, separation, ReadSequenceDelta);
}

static uint32 ProcessSequenceVolume(sint16 sequence, sint16 separation, uint8 val)
{
    FUNCTION_MARKER(0x80072444, "SLUS_005.78");
    return spu_sequence_scheduler_volume(&spu_sequence_scheduler, sequence, separation, val, SetSequenceVoiceVolume, ReadSequenceDelta);
}

static uint32 ProcessSequencePan(sint16 sequence, sint16 separation, uint8 val)
{
    FUNCTION_MARKER(0x80072518, "SLUS_005.78");
    return spu_sequence_scheduler_pan(&spu_sequence_scheduler, sequence, separation, val, SetSequenceVoiceVolume, ReadSequenceDelta);
}

static uint32 ResetSequenceControllers(sint16 sequence, sint16 separation)
{
    FUNCTION_MARKER(0x80072A94, "SLUS_005.78");
    return spu_sequence_scheduler_reset_controllers(&spu_sequence_scheduler, sequence, separation, SetSequenceReverbEnabled, ReadSequenceDelta);
}

static uint32 ProcessSequenceNrpnLsb(sint16 sequence, sint16 separation, uint8 val)
{
    FUNCTION_MARKER(0x80072728, "SLUS_005.78");
    return spu_sequence_scheduler_nrpn_lsb(&spu_sequence_scheduler, sequence, separation, val, spu_sequence_dispatch_mark, ReadSequenceDelta);
}

static uint32 ProcessSequenceDamper(sint16 sequence, sint16 separation, uint8 val)
{
    FUNCTION_MARKER(0x800725E8, "SLUS_005.78");
    return spu_sequence_scheduler_damper(&spu_sequence_scheduler, sequence, separation, val, ReadSequenceDelta);
}

static uint32 ProcessSequenceReverbDepth(sint16 sequence, sint16 separation, uint8 val)
{
    FUNCTION_MARKER(0x80072698, "SLUS_005.78");
    return spu_sequence_scheduler_reverb_depth(&spu_sequence_scheduler, sequence, separation, val, SetSequenceReverbDepth, ReadSequenceDelta);
}

static uint32 ProcessSequenceNrpnMsb(sint16 sequence, sint16 separation, uint8 val)
{
    FUNCTION_MARKER(0x80072860, "SLUS_005.78");
    return spu_sequence_scheduler_nrpn_msb(&spu_sequence_scheduler, sequence, separation, val, ReadSequenceDelta);
}

static uint32 ProcessSequenceRpnLsb(sint16 sequence, sint16 separation, uint8 val)
{
    FUNCTION_MARKER(0x800729A4, "SLUS_005.78");
    return spu_sequence_scheduler_rpn_lsb(&spu_sequence_scheduler, sequence, separation, val, ReadSequenceDelta);
}

static uint32 ProcessSequenceRpnMsb(sint16 sequence, sint16 separation, uint8 val)
{
    FUNCTION_MARKER(0x80072A1C, "SLUS_005.78");
    return spu_sequence_scheduler_rpn_msb(&spu_sequence_scheduler, sequence, separation, val, ReadSequenceDelta);
}

static uint32 ProcessSequenceControlChange(sint16 sequence, sint16 separation, uint8 pad)
{
    FUNCTION_MARKER(0x800737D0, "SLUS_005.78");
    return spu_sequence_scheduler_control_change(&spu_sequence_scheduler, sequence, separation, pad, &spu_sequence_handlers, ReadSequenceDelta);
}

static uint32 ReadSequenceEvent(sint16 sequence, sint16 separation)
{
    FUNCTION_MARKER(0x800753C4, "SLUS_005.78");
    return spu_sequence_scheduler_read_event(&spu_sequence_scheduler, sequence, separation, &spu_sequence_handlers, ReadSequenceDelta, ProcessSequenceEndOfTrack);
}

static uint32 ProcessSequenceEndOfTrack(sint16 sequence, sint16 separation)
{
    FUNCTION_MARKER(0x80075200, "SLUS_005.78");
    return spu_sequence_scheduler_end_of_track(&spu_sequence_scheduler, sequence, separation, AdvanceSequenceSeparation, ReleaseSequenceVoices);
}

static uint32 ReadSequenceDelta(sint16 sequence, sint16 separation)
{
    FUNCTION_MARKER(0x80073D58, "SLUS_005.78");
    return spu_sequence_scheduler_read_delta(&spu_sequence_scheduler, sequence, separation);
}

static uint32 ProcessSequenceProgramChange(sint16 sequence, sint16 separation, uint8 program)
{
    FUNCTION_MARKER(0x80073CE0, "SLUS_005.78");
    return spu_sequence_scheduler_program_change(&spu_sequence_scheduler, sequence, separation, program, ReadSequenceDelta);
}

sint32 SetSpuReverbAttributes(SpuReverbAttr *attributes)
{
    FUNCTION_MARKER(0x8006FF38, "SLUS_005.78");
    return SpuSetReverbModeParam(attributes);
}

uint32 WaitForVabTransfer(uint32 mode)
{
    FUNCTION_MARKER(0x8007A740, "SLUS_005.78");
    return (sint16)SpuIsTransferCompleted((sint16)mode);
}

sint32 GetSpuKeyStatus(uint32 mask)
{
    FUNCTION_MARKER(0x80070DF8, "SLUS_005.78");
    return SpuGetKeyStatus(mask);
}

uint32 SetSpuTransferAddressUnits(uint32 address)
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

static uint32 SetSequenceReverbEnabled(uint32 enabled)
{
    FUNCTION_MARKER(0x800764A0, "SLUS_005.78");
    return SpuSetReverb(enabled);
}

static uint32 PlaySequenceTrackTick(sint16 sequence, sint16 separation)
{
    FUNCTION_MARKER(0x800750F0, "SLUS_005.78");
    return spu_sequence_scheduler_play_tick(&spu_sequence_scheduler, sequence, separation, ReadSequenceEvent);
}

static uint32 SetSequenceTrackReplayMode(sint16 sequence, sint16 track)
{
    FUNCTION_MARKER(0x800742F4, "SLUS_005.78");
    return spu_sequence_scheduler_replay_mode(&spu_sequence_scheduler, sequence, track);
}

uint32 ReplaySequence(sint16 sequence)
{
    FUNCTION_MARKER(0x80074398, "SLUS_005.78");
    return SetSequenceTrackReplayMode(sequence, 0);
}

static uint32 SetSequenceVoiceVolume(sint16 sequence, sint16 bank, sint16 program, uint16 volume, uint16 pan)
{
    FUNCTION_MARKER(0x800794A0, "SLUS_005.78");
    return spu_sequence_vm_set_volume(&spu_sequence_scheduler, sequence, bank, program, volume, pan);
}

uint32 SetSpuKeys(uint32 enabled, uint32 voices)
{
    FUNCTION_MARKER(0x80070BC0, "SLUS_005.78");
    return spu_set_key(enabled, voices);
}

static uint32 ReleasePendingSequenceVoices(void)
{
    FUNCTION_MARKER(0x80078474, "SLUS_005.78");
    return spu_sequence_vm_key_off_now();
}

static uint32 ReleaseSequenceVoices(sint16 sequence)
{
    FUNCTION_MARKER(0x80079350, "SLUS_005.78");
    return spu_sequence_vm_sequence_key_off(sequence);
}

static uint32 StopSequenceTrack(sint16 sequence, sint16 track)
{
    FUNCTION_MARKER(0x80075818, "SLUS_005.78");
    return spu_sequence_scheduler_stop(&spu_sequence_scheduler, sequence, track, ReleaseSequenceVoices);
}

uint32 StopSequenceById(sint32 sequence)
{
    FUNCTION_MARKER(0x80075980, "SLUS_005.78");
    return StopSequenceTrack(sequence, 0);
}

uint32 SetSequenceReverbDepth(sint16 left, sint16 right)
{
    FUNCTION_MARKER(0x80076354, "SLUS_005.78");
    return spu_sequence_vm_reverb_depth(left, right);
}

uint32 SetSequenceReverbType(uint32 mode)
{
    FUNCTION_MARKER(0x800763EC, "SLUS_005.78");
    return spu_sequence_vm_reverb_type(mode);
}

uint32 SetSequenceTrackTable(SpuSequenceSchedulerTrack *table, sint16 sequences, sint16 separations)
{
    FUNCTION_MARKER(0x800759D4, "SLUS_005.78");
    return spu_sequence_scheduler_set_table(&spu_sequence_scheduler, table, sequences, separations);
}

static uint32 SetSequenceVolumeMode(sint16 handle, sint16 left, sint16 right, sint16 mode)
{
    FUNCTION_MARKER(0x80078D1C, "SLUS_005.78");
    return spu_sequence_vm_set_sequence_volume(&spu_sequence_scheduler, handle, left, right, mode);
}

uint32 PrintSdkDiagnostic(const char *format, ...)
{
    FUNCTION_MARKER(0x80082A18, "SLUS_005.78");
    va_list arguments;
    int result;
    va_start(arguments, format);
    result = vprintf(format, arguments);
    va_end(arguments);
    return result;
}

uint32 SetSequenceVoiceCount(uint8 voices)
{
    FUNCTION_MARKER(0x80079ADC, "SLUS_005.78");
    if (voices == 0 || voices >= 25)
        return 255;
    spu_vm_state()->events.voice_count = voices;
    return spu_vm_state()->events.voice_count;
}

static uint32 GetSequenceVolume(sint16 handle, uint16 *left, uint16 *right)
{
    FUNCTION_MARKER(0x80079244, "SLUS_005.78");
    return spu_sequence_vm_get_sequence_volume(&spu_sequence_scheduler, handle, left, right);
}

uint32 RequestSequencePause(sint16 sequence)
{
    FUNCTION_MARKER(0x80074210, "SLUS_005.78");
    return RequestSequenceTrackPause(sequence, 0);
}

uint32 PlaySequence(sint32 sequence, sint32 mode, sint32 repeat)
{
    FUNCTION_MARKER(0x80074264, "SLUS_005.78");
    return SetSequenceTrackPlayMode(sequence, 0, mode, repeat);
}

uint32 SetSequenceVolumeById(sint16 sequence, sint16 left, sint16 right)
{
    FUNCTION_MARKER(0x80075D58, "SLUS_005.78");
    return SetSequenceVolumeMode(sequence, left, right, 1);
}

static uint32 SetSequenceTrackPlayMode(sint16 sequence, sint16 track, uint8 mode, sint16 repeat)
{
    FUNCTION_MARKER(0x800743EC, "SLUS_005.78");
    return spu_sequence_scheduler_play_mode(&spu_sequence_scheduler, sequence, track, mode, repeat, SetSequenceVolumeMode);
}

static uint32 AdvanceSequenceTrack(sint16 sequence, sint16 separation)
{
    FUNCTION_MARKER(0x800750C4, "SLUS_005.78");
    return PlaySequenceTrackTick(sequence, separation);
}

static uint32 CloseSequenceHandle(sint16 sequence)
{
    FUNCTION_MARKER(0x800718EC, "SLUS_005.78");
    return spu_sequence_scheduler_close(&spu_sequence_scheduler, sequence, SetSequenceVolumeMode, ReleaseSequenceVoices);
}

static uint32 WriteSpuSamples(const void *src, uint32 bytes)
{
    FUNCTION_MARKER(0x8006FB68, "SLUS_005.78");
    return SpuWrite(src, bytes);
}

uint32 CloseVabBank(sint32 bank)
{
    FUNCTION_MARKER(0x80079B14, "SLUS_005.78");
    return spu_vab_close_sticky(bank);
}

uint32 OpenVabHeader(const uint8 *header, uint32 bytes, sint32 bank, uint32 address)
{
    FUNCTION_MARKER(0x80079F48, "SLUS_005.78");
    return spu_vab_open_head_sticky(header, bytes, bank, address);
}

uint32 TransferVabSamples(const uint8 *samples, uint32 sample_bytes, sint16 bank)
{
    FUNCTION_MARKER(0x8007A680, "SLUS_005.78");
    return spu_vab_trans_body_native(samples, sample_bytes, bank);
}

static uint32 FlushSequenceVoices(void)
{
    FUNCTION_MARKER(0x800767DC, "SLUS_005.78");
    return spu_sequence_vm_flush();
}

static uint32 InitSpuAllocator(sint32 count, SpuMallocRecord *table)
{
    FUNCTION_MARKER(0x800770D0, "SLUS_005.78");
    return SpuInitMalloc(count, (char *)table);
}

static uint32 InitSequenceVoices(uint8 voices)
{
    FUNCTION_MARKER(0x80076DC0, "SLUS_005.78");
    return spu_sequence_vm_initialize(voices);
}

static uint32 InitSequenceScheduler(void)
{
    FUNCTION_MARKER(0x80071B74, "SLUS_005.78");
    return spu_sequence_scheduler_initialize(&spu_sequence_scheduler, InitSequenceVoices);
}

static uint32 KeyOnSequenceVoice(sint16 sequence, sint16 bank, sint16 program, uint16 note, uint16 velocity, uint16 pan)
{
    FUNCTION_MARKER(0x80077168, "SLUS_005.78");
    return spu_sequence_vm_key_on(&spu_sequence_scheduler, sequence, bank, program, note, velocity, pan);
}

static uint32 ProcessSequenceNoteOn(sint16 sequence, sint16 separation, uint8 note, uint8 velocity)
{
    FUNCTION_MARKER(0x80073BF0, "SLUS_005.78");
    return spu_sequence_scheduler_note_on(&spu_sequence_scheduler, sequence, separation, note, velocity, KeyOnSequenceVoice, KeyOffSequenceVoice);
}

static uint32 EnableSequenceNoise(uint8 voice)
{
    FUNCTION_MARKER(0x80077F60, "SLUS_005.78");
    return spu_sequence_vm_noise_on(&spu_sequence_scheduler, voice);
}

static uint32 CommitSequenceKeyOn(uint32 layers, sint16 pitch)
{
    FUNCTION_MARKER(0x80078544, "SLUS_005.78");
    return spu_sequence_vm_key_on_now(&spu_sequence_scheduler, layers, pitch);
}

static uint32 SetSequenceNoiseVoices(uint32 mode, uint32 voices)
{
    FUNCTION_MARKER(0x80076B94, "SLUS_005.78");
    return spu_sequence_vm_noise_voice(mode, voices);
}

static uint32 AllocateSequenceVoice(void)
{
    FUNCTION_MARKER(0x80077930, "SLUS_005.78");
    return spu_sequence_vm_allocate();
}

static uint32 DisableSequenceNoise(uint8 voice)
{
    FUNCTION_MARKER(0x80078430, "SLUS_005.78");
    return spu_sequence_vm_noise_off(voice);
}

static uint32 KeyOffSequenceVoice(sint16 sequence, sint16 bank, sint16 program, uint16 note)
{
    FUNCTION_MARKER(0x800776BC, "SLUS_005.78");
    return spu_sequence_vm_key_off(sequence, bank, program, note);
}

static uint32 SelectSequenceTones(uint8 *tones, uint8 *samples)
{
    FUNCTION_MARKER(0x800793EC, "SLUS_005.78");
    return spu_sequence_vm_select_tones(tones, samples);
}

static uint32 PrepareSequenceVoice(void)
{
    FUNCTION_MARKER(0x80077B98, "SLUS_005.78");
    return spu_sequence_vm_prepare_voice();
}

static uint16 ComputeSequenceNotePitch(void)
{
    FUNCTION_MARKER(0x80077DAC, "SLUS_005.78");
    return spu_sequence_vm_note_pitch();
}

static uint16 ComputeSequenceTonePitch(uint16 note, uint16 fraction)
{
    FUNCTION_MARKER(0x80077E64, "SLUS_005.78");
    return spu_sequence_vm_tone_pitch(note, fraction);
}

static uint32 BendSequenceVoicePitch(sint16 voice, sint16 sequence, sint16 bank, sint16 program, uint16 bend)
{
    FUNCTION_MARKER(0x80078A28, "SLUS_005.78");
    return spu_sequence_vm_voice_bend(voice, sequence, bank, program, bend);
}

static uint32 BendSequencePitch(sint16 sequence, sint16 bank, sint16 program, uint16 bend)
{
    FUNCTION_MARKER(0x80078C2C, "SLUS_005.78");
    return spu_sequence_vm_sequence_bend(sequence, bank, program, bend);
}

static uint32 ProcessSequencePitchBend(sint16 sequence, sint16 separation)
{
    FUNCTION_MARKER(0x80073718, "SLUS_005.78");
    return spu_sequence_scheduler_pitch_bend(&spu_sequence_scheduler, sequence, separation, BendSequencePitch, ReadSequenceDelta);
}

static uint32 ProcessSequenceDataEntry(sint16 sequence, sint16 separation, uint8 val)
{
    FUNCTION_MARKER(0x8007201C, "SLUS_005.78");
    SpuSequenceSchedulerTrack *rec = spu_sequence_scheduler.tracks[(uint32)sequence] + (uint32)separation;
    return spu_sequence_scheduler_data_entry(rec, sequence, separation, val, spu_tone_commands, 20, ReadSequenceDelta);
}

static uint32 InitSequenceTrack(sint16 sequence, sint16 bank, const uint8 *data)
{
    FUNCTION_MARKER(0x80073E10, "SLUS_005.78");
    return spu_sequence_scheduler_initialize_track(&spu_sequence_scheduler, sequence, bank, data, ReadSequenceDelta);
}

uint32 OpenSequence(const uint8 *sequence, uint32 bytes, sint16 bank)
{
    FUNCTION_MARKER(0x80071C8C, "SLUS_005.78");
    return spu_sequence_scheduler_open(&spu_sequence_scheduler, sequence, bytes, bank, InitSequenceTrack, &spu_sequence_handlers, &sequence_default_handlers, spu_tone_commands, sequence_default_commands);
}

PsxInterruptCallback SetGraphDrawSyncCallback(PsxInterruptCallback callback)
{
    FUNCTION_MARKER(0x8007FEBC, "SLUS_005.78");
    if ((uint8)graph_debug_level >= 2)
        printf("DrawSyncCallback(%p)...\n", (void *)callback);
    return DrawSyncCallback(callback);
}

uint32 ClearReverseOrderingTable(uint32 table, sint32 count)
{
    FUNCTION_MARKER(0x800804BC, "SLUS_005.78");
    if ((uint8)graph_debug_level >= 2)
        printf("ClearOTagR(%08x,%d)...\n", table, count);
    uint32 *entries = (uint32 *)psx_addr(table, (size_t)(uint32)count * sizeof(uint32));
    ClearOTagR(entries, count);
    entries[0] = 0x8008A040 & 0xFFFFFF;
    return table;
}

uint32 ClearSpuReverbWorkArea(uint32 mode)
{
    FUNCTION_MARKER(0x80070A24, "SLUS_005.78");
    return SpuClearReverbWorkArea(mode);
}

static void HandleSequenceTimerIrq(void)
{
    FUNCTION_MARKER(0x80074790, "SLUS_005.78");
    spu_sequence_scheduler_irq0(spu_sequence_clock());
}

static void HandleSequenceHalfRateTick(void)
{
    FUNCTION_MARKER(0x800747D0, "SLUS_005.78");
    spu_sequence_scheduler_half_rate(spu_sequence_clock());
}

static void StartSequenceTimer(uint32 mode)
{
    FUNCTION_MARKER(0x800744EC, "SLUS_005.78");
    spu_sequence_scheduler_start_timer(spu_sequence_clock(), mode, HandleSequenceTimerIrq, HandleSequenceHalfRateTick);
}

void PumpSequences(void)
{
    FUNCTION_MARKER(0x80074820, "SLUS_005.78");
    spu_sequence_scheduler_pump(&spu_sequence_scheduler, &sequence_pump_ops);
}

static uint32 AdvanceSequenceCrescendo(sint16 sequence, sint16 separation)
{
    FUNCTION_MARKER(0x80074A98, "SLUS_005.78");
    return spu_sequence_scheduler_crescendo(&spu_sequence_scheduler, sequence, separation, GetSequenceVolume, SetSequenceVolumeMode);
}

static uint32 AdvanceSequenceDecrescendo(sint16 sequence, sint16 separation)
{
    FUNCTION_MARKER(0x80074D74, "SLUS_005.78");
    return spu_sequence_scheduler_decrescendo(&spu_sequence_scheduler, sequence, separation, GetSequenceVolume, SetSequenceVolumeMode);
}

static uint32 PauseSequenceTrack(sint16 sequence, sint16 separation)
{
    FUNCTION_MARKER(0x80075020, "SLUS_005.78");
    return spu_sequence_scheduler_pause(&spu_sequence_scheduler, sequence, separation, ReleaseSequenceVoices);
}

static void ResumeSequenceTrack(sint16 sequence, sint16 separation)
{
    FUNCTION_MARKER(0x800757B0, "SLUS_005.78");
    spu_sequence_scheduler_resume(&spu_sequence_scheduler, sequence, separation);
}

static void AdvanceSequenceTempoRamp(sint16 sequence, sint16 separation)
{
    FUNCTION_MARKER(0x80075DF4, "SLUS_005.78");
    spu_sequence_scheduler_tempo_ramp(&spu_sequence_scheduler, sequence, separation);
}

uint32 SetSequenceTickMode(uint32 mode)
{
    FUNCTION_MARKER(0x80075BB4, "SLUS_005.78");
    return spu_sequence_scheduler_tick_mode(&spu_sequence_scheduler, spu_sequence_clock(), mode, ReadVideoMode());
}

static uint32 ProcessReverbTypeCommand(uint8 val)
{
    FUNCTION_MARKER(0x80073660, "SLUS_005.78");
    return SetSequenceReverbType(val);
}

static uint32 ProcessReverbDepthCommand(uint8 val)
{
    FUNCTION_MARKER(0x80073684, "SLUS_005.78");
    return SetSequenceReverbDepth(val, val);
}

static uint32 ProcessReverbFeedbackCommand(uint8 val)
{
    FUNCTION_MARKER(0x800736AC, "SLUS_005.78");
    return spu_sequence_vm_reverb_feedback(val);
}

static uint32 ProcessReverbDelayCommand(uint8 val)
{
    FUNCTION_MARKER(0x800736D0, "SLUS_005.78");
    return spu_sequence_vm_reverb_delay(val);
}

static uint32 ProcessReverbDelayAliasCommand(uint8 val)
{
    FUNCTION_MARKER(0x800736F4, "SLUS_005.78");
    return ProcessReverbDelayCommand(val);
}

uint32 SetSpuVoiceAttributes(const SpuVoiceAttr *attributes)
{
    FUNCTION_MARKER(0x8007101C, "SLUS_005.78");
    SpuSetVoiceAttr(attributes);
    return 0;
}

uint32 SetSpuVoiceAttributesAndKeyOn(const SpuVoiceAttr *attributes)
{
    FUNCTION_MARKER(0x80070DC4, "SLUS_005.78");
    SetSpuVoiceAttributes(attributes);
    return SetSpuKeys(1, attributes->voice);
}

PsxCallbackDriver *ResetCallbackDriver(void)
{
    FUNCTION_MARKER(0x8007A950, "SLUS_005.78");
    return psx_callback_reset_driver();
}

static void ResetSpuDevice(uint32 mode)
{
    FUNCTION_MARKER(0x8006F2EC, "SLUS_005.78");
    spu_sequence_vm_reset_device(mode);
}

static void HandleSpuDma(void)
{
    FUNCTION_MARKER(0x8006F74C, "SLUS_005.78");
    spu_sequence_vm_dma();
}

static PsxInterruptCallback RegisterSpuDmaCallback(PsxInterruptCallback callback)
{
    FUNCTION_MARKER(0x8006FE98, "SLUS_005.78");
    return spu_sequence_vm_register_dma(callback);
}

static void QuitSpuDevice(void)
{
    FUNCTION_MARKER(0x8006FEBC, "SLUS_005.78");
    spu_sequence_vm_quit();
}

static uint32 InitSpuDevice(uint32 mode)
{
    FUNCTION_MARKER(0x8006F178, "SLUS_005.78");
    return spu_sequence_vm_initialize_device(mode, HandleSpuDma);
}

static void StopSequenceTimer(void)
{
    FUNCTION_MARKER(0x80071AB0, "SLUS_005.78");
    spu_sequence_scheduler_stop_timer(spu_sequence_clock());
}

void ShutdownSound(void)
{
    FUNCTION_MARKER(0x8006AF4C, "SLUS_005.78");
    SetSpuKeys(0, 0x00FF0000);
    StopSequenceTimer();
    QuitSpuDevice();
    QuitSpuDevice();
}

static void QuitSequenceDevice(void)
{
    FUNCTION_MARKER(0x800742D4, "SLUS_005.78");
    QuitSpuDevice();
}

static void StartSequenceAutomaticTimer(void)
{
    FUNCTION_MARKER(0x80074770, "SLUS_005.78");
    StartSequenceTimer(0);
}

void ExitBiosCriticalSection(void)
{
    FUNCTION_MARKER(0x8008347C, "SLUS_005.78");
    xport_bios_exit_critical();
}

static sint32 CdPosToSector(const uint8 *pos)
{
    CdlLOC location;
    FUNCTION_MARKER(0x8007BC54, "SLUS_005.78");
    location.minute = pos[0];
    location.second = pos[1];
    location.sector = pos[2];
    location.track = 0;
    return CdPosToInt(&location);
}

uint32 EnterBiosCriticalSection(void)
{
    FUNCTION_MARKER(0x8008346C, "SLUS_005.78");
    return xport_bios_enter_critical();
}

uint32 InitSpuCold(void)
{
    FUNCTION_MARKER(0x8006F158, "SLUS_005.78");
    return InitSpuDevice(0);
}

uint32 InitSound(void)
{
    FUNCTION_MARKER(0x80071C5C, "SLUS_005.78");
    ResetCallbackDriver();
    InitSpuCold();
    return InitSequenceScheduler();
}

static uint32 SetSequenceTransferMode(uint32 mode)
{
    FUNCTION_MARKER(0x80077124, "SLUS_005.78");
    return spu_sequence_vm_transfer_mode(mode);
}

static uint32 ReadVideoMode(void)
{
    FUNCTION_MARKER(0x8007B448, "SLUS_005.78");
    return GetVideoMode();
}

void StartSequenceVBlankTimer(void)
{
    FUNCTION_MARKER(0x80074750, "SLUS_005.78");
    StartSequenceTimer(1);
}

static uint32 InitPadBuffers(uint32 buffer0, uint32 size0, uint32 buffer1, uint32 size1)
{
    FUNCTION_MARKER(0x8008371C, "SLUS_005.78");
    return InitPAD(buffer0, size0, buffer1, size1);
}

static uint32 StartPadDriver(void)
{
    FUNCTION_MARKER(0x8008372C, "SLUS_005.78");
    return StartPAD();
}

static uint32 IsSpuTransferPending(void)
{
    FUNCTION_MARKER(0x80077150, "SLUS_005.78");
    return spu_native_transfer_state->completion != 1;
}

uint32 WriteSpuTransfer(const void *src, uint32 bytes)
{
    uint32 len = bytes;
    FUNCTION_MARKER(0x80070E8C, "SLUS_005.78");
    if (len > 0x7EFF0)
        len = 0x7EFF0;
    WriteSpuSamples(src, len);
    if (spu_native_transfer_state->completion_callback == 0)
        spu_native_transfer_state->completion = 0;
    return len;
}

uint32 DisableSequenceReverb(void)
{
    FUNCTION_MARKER(0x800765B0, "SLUS_005.78");
    return SetSequenceReverbEnabled(0);
}

uint32 EnableSequenceReverb(void)
{
    FUNCTION_MARKER(0x800765D0, "SLUS_005.78");
    return SetSequenceReverbEnabled(1);
}

uint32 DoubleWordsDiffer(uint32 left_low, uint32 left_high, uint32 right_low, uint32 right_high)
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

uint32 ReleaseSoundEffectVoices(void)
{
    FUNCTION_MARKER(0x8006A614, "SLUS_005.78");
    return SetSpuKeys(0, 0xFF0000);
}

uint32 CloseSequenceById(sint16 sequence)
{
    FUNCTION_MARKER(0x80071A68, "SLUS_005.78");
    return CloseSequenceHandle(sequence);
}

uint32 ResetGraphDefault(void)
{
    FUNCTION_MARKER(0x8007FB14, "SLUS_005.78");
    uint32 mask, graph_type;

    printf("ResetGraph:jtb=%08x,env=%08x\n", 0x80089F3C, 0x80089F84);
    gpu_graph_control.type = 0;
    gpu_graph_control.queue_enabled = 0;
    SetGraphDebug(0);
    gpu_graph_control.reverse = 0;
    DrawSyncCallback(NULL);
    ResetCallbackDriver();
    mask = psx_irq_native_mask_exchange(0);
    ResetGraph(0);
    SetDispMask(0);
    memset(gpu_graph_control.command_cache, 0, sizeof(gpu_graph_control.command_cache));
    psx_irq_native_mask_exchange(mask);
    graph_type = (uint8)GetGraphType();
    gpu_graph_control.type = graph_type;
    gpu_graph_control.queue_enabled = 1;
    return graph_type;
}

static uint32 CloseSeparationSequence(sint16 sequence)
{
    FUNCTION_MARKER(0x80071A8C, "SLUS_005.78");
    return CloseSequenceHandle(sequence);
}

static uint32 RequestSeparationPause(sint16 sequence, sint16 track)
{
    FUNCTION_MARKER(0x80074238, "SLUS_005.78");
    return RequestSequenceTrackPause(sequence, track);
}

static uint32 ReplaySeparationSequence(sint16 sequence, sint16 track)
{
    FUNCTION_MARKER(0x800743C0, "SLUS_005.78");
    return SetSequenceTrackReplayMode(sequence, track);
}

static uint32 StopSeparationSequence(sint16 sequence, sint16 track)
{
    FUNCTION_MARKER(0x800759A8, "SLUS_005.78");
    return StopSequenceTrack(sequence, track);
}

static void UseImmediateVoiceRelease(void)
{
    FUNCTION_MARKER(0x800767B8, "SLUS_005.78");
    spu_vm_state()->release_offset = 0;
}

static uint32 UseDeferredVoiceRelease(void)
{
    FUNCTION_MARKER(0x800767C8, "SLUS_005.78");
    spu_vm_state()->release_offset = 2;
    return 2;
}

static uint32 StartSpuDmaTransfer(uint32 src, uint16 address_units, uint32 blocks, uint32 port)
{
    FUNCTION_MARKER(0x8006F824, "SLUS_005.78");
    uint32 control;
    xport_memory_write(port + 422, 2u, address_units, 3u);
    port = SPU_REGISTER_BASE;
    xport_memory_write(port + 426, 2u, xport_memory_read(port + 426, 2u) | 0x30, 3u);
    control = SPU_DMA_DELAY_REGISTER;
    xport_memory_write(control, 4u, (xport_memory_read(control, 4u) & 0xD0FFFFFF) | 0x22000000, 15u);
    xport_memory_write(SPU_DMA_ADDRESS_REGISTER, 4u, src, 15u);
    xport_memory_write(SPU_DMA_BLOCK_REGISTER, 4u, (blocks << 16) | 0x10, 15u);
    xport_memory_write(SPU_DMA_CONTROL_REGISTER, 4u, 0x01000200, 15u);
    return 1;
}

static uint32 ClearForwardOrderingTable(uint32 ordering_table, sint32 count)
{
    FUNCTION_MARKER(0x8008040C, "SLUS_005.78");
    uint32 cursor = ordering_table;
    uint32 *entry = (uint32 *)psx_addr(ordering_table, (size_t)(uint32)count * sizeof(uint32));
    if ((uint8)graph_debug_level >= 2)
        PrintSdkDiagnostic("ClearOTag(%08x,%d)...\n", ordering_table, (uint32)count);
    while (--count != 0)
    {
        *entry &= 0xFFFFFF;
        *entry = (*entry & 0xFF000000) | ((cursor + 4) & 0xFFFFFF);
        cursor += 4;
        ++entry;
    }
    *entry = 0x0008A040;
    return cursor;
}

static void StopPadCommunication(void)
{
    FUNCTION_MARKER(0x8008342C, "SLUS_005.78");
    PadStopCom();
}

static PsxInterruptCallback SetVBlankInterruptCallback(uint32 channel, PsxInterruptCallback callback)
{
    FUNCTION_MARKER(0x8007AA1C, "SLUS_005.78");
    return setIntrVSync(channel, callback);
}

static PsxCallbackDriver *RestartInterruptDriver(void)
{
    FUNCTION_MARKER(0x8007AA7C, "SLUS_005.78");
    return restartIntr();
}

static uint32 SetGraphReverseMode(uint32 reverse)
{
    FUNCTION_MARKER(0x8007FC78, "SLUS_005.78");
    uint32 prev = gpu_graph_control.reverse, mode;
    if ((uint8)graph_debug_level >= 2)
        PrintSdkDiagnostic("SetGraphReverse(%d)...\n", reverse);
    gpu_graph_control.reverse = reverse;
    mode = _getctl(8);
    _ctl(mode | (gpu_graph_control.reverse != 0 ? 0x08000080 : 0x08000000));
    if (gpu_graph_control.type == 2)
        _ctl(gpu_graph_control.reverse != 0 ? 0x20000501 : 0x20000504);
    return prev;
}

static uint32 SetGraphQueueMode(uint32 enabled)
{
    FUNCTION_MARKER(0x8007FDF8, "SLUS_005.78");
    uint32 prev = gpu_graph_control.queue_enabled;
    uint16 mask;
    if ((uint8)graph_debug_level >= 2)
        PrintSdkDiagnostic("SetGrapQue(%d)...\n", enabled);
    if (enabled == gpu_graph_control.queue_enabled)
        return prev;
    mask = psx_irq_native_mask_exchange(0);
    xport_memory_write(0x1F8010A8u, 4u, 1025, 15u);
    xport_memory_write(0x1F8010F0u, 4u, xport_memory_read(0x1F8010F0u, 4u) | 0x800, 15u);
    _ctl(0x02000000);
    _ctl(0x01000000);
    psx_irq_native_mask_exchange(mask);
    gpu_graph_control.queue_enabled = enabled;
    if (psx_callback_driver.initialized != 0)
        InterruptCallback(2, NULL);
    return prev;
}

static uint32 ReadGpuStatusHighBit(void)
{
    FUNCTION_MARKER(0x80080CF0, "SLUS_005.78");
    return _status() >> 31;
}

static uint32 InitPadDriver(uint32 type, uint32 dst)
{
    FUNCTION_MARKER(0x8008373C, "SLUS_005.78");
    return PAD_init2(type, dst);
}

uint32 IsSoundTransferComplete(void)
{
    FUNCTION_MARKER(0x8006AAA8, "SLUS_005.78");
    return SpuIsTransferCompleted(0) != 0;
}
