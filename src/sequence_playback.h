#ifndef SEQUENCE_PLAYBACK_H
#define SEQUENCE_PLAYBACK_H
#include <stddef.h>

#include "psx.h"
#include "psx_spu.h"

struct SequencePlayback;

struct SequencePlayback
{
    uint16 cur_handle;
    uint16 saved_handle;
    uint16 loop_handle;
    sint16 loading_bank;
    sint8 loading_phase;
    uint8 *loading_samples;
    uint32 loading_sample_bytes;

    static void StartCurSequence(void);
    static void StopCurSequence(void);
    static void SetSequenceVolume(uint8 handle, uint16 volume);
    static void LoadLoopSequence(const uint8 *bundle, uint32 bundle_bytes);
    static void UnloadLoopSequence(void);
    static uint32 PrepareSavedSequence(void);
    static void PlayLoopSequence(void);
    static uint32 LoadSequence(uint32 asset, uint32 mode, const uint8 *header, uint32 header_bytes, const uint8 *samples, uint32 sample_bytes);
    static void PauseSequence(uint16 handle);
    static void ResumeSequence(uint16 handle);
    static void StopSequence(uint8 handle);
    static void StartSequence(uint16 handle);
    static void CloseSequence(uint16 handle);
    static void InitGameSound(void);
};

extern SequencePlayback sequence_playback;

extern SpuSequenceSchedulerTrack sequence_track_storage[2];

#endif
