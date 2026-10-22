#ifndef PSYQ_BINDINGS_H
#define PSYQ_BINDINGS_H
#include <stddef.h>

#include "psx.h"
#include "psx_spu.h"

uint32 ResetGraphDefault(void);

PsxCallbackDriver *ResetCallbackDriver(void);

uint32 RequestSequencePause(sint16 sequence);

void ExitBiosCriticalSection(void);

uint32 EnterBiosCriticalSection(void);

CdlCB SetCdReadCallback(CdlCB callback);

uint32 ReleaseSoundEffectVoices(void);

void ShutdownSound(void);

uint32 InitSpuCold(void);

uint32 InitSound(void);

void StartSequenceVBlankTimer(void);

uint32 WriteSpuTransfer(const void *src, uint32 bytes);

sint32 SetSpuReverbAttributes(SpuReverbAttr *attributes);

uint32 SetSequenceTrackTable(SpuSequenceSchedulerTrack *table, sint16 sequences, sint16 separations);

uint32 SetSequenceVoiceCount(uint8 voices);

PsxInterruptCallback SetGraphDrawSyncCallback(PsxInterruptCallback callback);

uint32 OpenVabHeader(const uint8 *header, uint32 bytes, sint32 bank, uint32 address);

uint32 TransferVabSamples(const uint8 *samples, uint32 sample_bytes, sint16 bank);

uint32 WaitForVabTransfer(uint32 mode);

uint32 OpenSequence(const uint8 *sequence, uint32 bytes, sint16 bank);

uint32 SetSpuTransferAddressUnits(uint32 address);

uint32 PrintSdkDiagnostic(const char *format, ...);

uint32 DisableSequenceReverb(void);

uint32 EnableSequenceReverb(void);

uint32 SetSequenceVolumeById(sint16 sequence, sint16 left, sint16 right);

uint32 ReplaySequence(sint16 sequence);

uint32 ClearSpuReverbWorkArea(uint32 enabled);

uint32 SetSequenceReverbType(uint32 enabled);

uint32 SetSequenceReverbDepth(sint16 left, sint16 right);

uint32 PlaySequence(sint32 sequence, sint32 mode, sint32 repeat);

uint32 ClearReverseOrderingTable(uint32 table, sint32 count);

sint32 GetSpuKeyStatus(uint32 mask);

uint32 SetSpuKeys(uint32 enabled, uint32 voices);

uint32 StopSequenceById(sint32 sequence);

uint32 CloseVabBank(sint32 bank);

uint32 SetSequenceTickMode(uint32 mode);

void PumpSequences(void);

uint32 IsSoundTransferComplete(void);

uint32 CloseSequenceById(sint16 sequence);

uint32 DoubleWordsDiffer(uint32 left_low, uint32 left_high, uint32 right_low, uint32 right_high);

uint32 SetSpuVoiceAttributes(const SpuVoiceAttr *attributes);

uint32 SetSpuVoiceAttributesAndKeyOn(const SpuVoiceAttr *attributes);

extern "C" uint32 sprintfPSX(uint32 dst, uint32 format, uint32 val);

#endif
