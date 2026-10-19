#ifndef SEQUENCE_PLAYBACK_H
#define SEQUENCE_PLAYBACK_H

#include "psx.h"

struct SequencePlayback
{
    uint16 current_handle;
    uint16 saved_handle;
    uint16 loop_handle;
    uint16 loading_bank;
    sint8 loading_phase;
    uint8 *loading_samples;
    uint32 loading_sample_bytes;
};

extern SequencePlayback sequence_playback;

extern "C"
{
    uint32 LoadLoopSequence(const uint8 *bundle, uint32 bundle_bytes);
    void UnloadLoopSequence(void);
    uint32 PrepareSavedSequence(void);
    void PlayLoopSequence(void);
    uint32 LoadSequence(uint32 asset, uint32 mode, const uint8 *header, uint32 header_bytes, const uint8 *samples, uint32 sample_bytes);
    uint32 PauseSequence(uint16 handle);
    uint32 ResumeSequence(uint16 handle);
    uint32 StopSequence(uint8 handle);
    uint32 StartSequence(uint16 handle);
    uint32 CloseSequence(uint16 handle);
    uint32 SetSequenceVolume(uint32 handle, uint16 volume);
    void StartCurrentSequence(void);
    void StopCurrentSequence(void);
}

#endif
