#include <stdio.h>
#include <stdlib.h>

#include "sequence_playback.h"
#include "game.h"
#include "global.h"

SpuSequenceSchedulerTrack sequence_track_storage[2];

SequencePlayback sequence_playback = {0, 0, 0, OriginalDefaults::SequenceLoadingBank, 0, NULL, 0};

void SequencePlayback::StartCurSequence(void)
{
    uint16 handle;
    FUNCTION_MARKER(0x80066404, "SLUS_005.78");
    handle = sequence_playback.cur_handle;
    if (handle != 0)
        SequencePlayback::StartSequence(handle);
}

void SequencePlayback::StopCurSequence(void)
{
    uint16 handle;
    FUNCTION_MARKER(0x80066430, "SLUS_005.78");
    handle = sequence_playback.cur_handle;
    if (handle != 0)
        SequencePlayback::StopSequence(handle);
}

void SequencePlayback::SetSequenceVolume(uint8 handle, uint16 volume)
{
    FUNCTION_MARKER(0x8006A5E0, "SLUS_005.78");
    sint16 scaled = ((uint32)volume * 127) >> 12;
    SetSequenceVolumeById(handle, scaled, scaled);
}

void SequencePlayback::LoadLoopSequence(const uint8 *bundle, uint32 bundle_bytes)
{
    uint32 bank;
    uint32 offset;
    uint32 sequence;
    uint32 bytes;
    uint32 result;
    FUNCTION_MARKER(0x8006AAC8, "SLUS_005.78");
    bytes = resource_archive.Size(18);
    bank = OpenVabHeader(bundle, bytes, -1, 4112);
    offset = resource_archive.Offset(20);
    bytes = resource_archive.Size(20);
    TransferVabSamples(bundle + offset, bytes, bank);
    while ((WaitForVabTransfer(0) << 16) == 0)
        ;
    offset = resource_archive.Offset(19);
    bytes = resource_archive.Size(19);
    sequence = OpenSequence(bundle + offset, bytes, bank);
    result = ((sequence + 1) & 255) << 8;
    sequence_playback.loop_handle = ((bank + 1) & 255) | result;
}

void SequencePlayback::UnloadLoopSequence(void)
{
    FUNCTION_MARKER(0x8006AB74, "SLUS_005.78");
    uint16 handle = sequence_playback.loop_handle;
    if (handle != 0)
    {
        SequencePlayback::CloseSequence(handle);
        sequence_playback.loop_handle = 0;
    }
}

uint32 SequencePlayback::PrepareSavedSequence(void)
{
    FUNCTION_MARKER(0x8006ABA0, "SLUS_005.78");
    uint32 sequence, volume;
    sequence = (uint8)sequence_playback.saved_handle;
    volume = (uint16)sound_settings.music_volume;
    sequence = (sequence - 1) & 0xFF;
    volume = (127 * volume) >> 12;
    SetSequenceVolumeById((uint8)sequence, volume, volume);
    return sequence_playback.saved_handle;
}

void SequencePlayback::PlayLoopSequence(void)
{
    FUNCTION_MARKER(0x8006ABE4, "SLUS_005.78");
    uint32 sequence, volume, prev, cur;
    sequence = (uint8)sequence_playback.loop_handle;
    volume = (uint16)sound_settings.effects_volume;
    sequence = (sequence - 1) & 0xFF;
    volume = (127 * volume) >> 12;
    SetSequenceVolumeById((uint8)sequence, volume, volume);
    prev = sequence_playback.cur_handle;
    cur = sequence_playback.loop_handle;
    sequence_playback.saved_handle = prev;
    sequence_playback.cur_handle = cur;
    SequencePlayback::StartCurSequence();
}

uint32 SequencePlayback::LoadSequence(uint32 asset, uint32 mode, const uint8 *header, uint32 header_bytes, const uint8 *samples, uint32 sample_bytes)
{
    sint8 phase;
    uint32 offset;
    uint32 sequence;
    uint32 bytes;
    uint32 volume;
    uint32 result;
    FUNCTION_MARKER(0x8006AC44, "SLUS_005.78");
    phase = sequence_playback.loading_phase;
    if (phase == 0)
    {
        offset = resource_archive.Offset(asset + 1);
        bytes = resource_archive.Size(asset + 1);
        sequence_playback.loading_bank = OpenVabHeader(header + offset, bytes, -1, 11568);
        if (mode != 0)
        {
            uint32 capacity;
            bytes = resource_archive.Size(mode);
            capacity = (bytes + 2047) & ~2047u;
            sequence_playback.loading_samples = (uint8 *)malloc(capacity != 0 ? capacity : 1);
            sequence_playback.loading_sample_bytes = bytes;
            resource_archive.ReadAsset(mode, sequence_playback.loading_samples, 0);
            sequence_playback.loading_phase = 2;
            return 0;
        }
        TransferVabSamples(samples, sample_bytes, sequence_playback.loading_bank);
        sequence_playback.loading_phase = 1;
        return 0;
    }
    if (phase == 2)
    {
        if (!resource_archive.Poll())
            return 0;
        TransferVabSamples(sequence_playback.loading_samples, sequence_playback.loading_sample_bytes, sequence_playback.loading_bank);
        sequence_playback.loading_phase = 1;
        return 0;
    }
    if (phase == 1 && (WaitForVabTransfer(0) << 16) == 0)
        return 0;
    sequence_playback.loading_phase = 0;
    free(sequence_playback.loading_samples);
    sequence_playback.loading_samples = NULL;
    sequence_playback.loading_sample_bytes = 0;
    offset = resource_archive.Offset(asset + 2);
    bytes = resource_archive.Size(asset + 2);
    sequence = OpenSequence(header + offset, bytes, sequence_playback.loading_bank);
    volume = (127 * (uint16)sound_settings.music_volume) >> 12;
    SetSequenceVolumeById((uint8)sequence, volume, volume);
    result = (((uint8)sequence_playback.loading_bank + 1) & 255) | (((sequence + 1) & 255) << 8);
    sequence_playback.saved_handle = result;
    return result;
}

void SequencePlayback::PauseSequence(uint16 handle)
{
    FUNCTION_MARKER(0x8006AD80, "SLUS_005.78");
    RequestSequencePause(((uint8)handle - 1));
}

void SequencePlayback::ResumeSequence(uint16 handle)
{
    FUNCTION_MARKER(0x8006ADA4, "SLUS_005.78");
    ReplaySequence(((uint8)handle - 1));
}

void SequencePlayback::StopSequence(uint8 handle)
{
    FUNCTION_MARKER(0x8006ADC8, "SLUS_005.78");
    StopSequenceById(handle - 1);
    DisableSequenceReverb();
    ClearSpuReverbWorkArea(1);
    SetSequenceReverbType(0);
    SetSequenceReverbDepth(0, 0);
}

void SequencePlayback::StartSequence(uint16 handle)
{
    sint32 repeat;
    FUNCTION_MARKER(0x8006AE10, "SLUS_005.78");
    DisableSequenceReverb();
    ClearSpuReverbWorkArea(1);
    SetSequenceReverbType(1);
    SetSequenceReverbDepth(36, 36);
    EnableSequenceReverb();
    repeat = handle == sequence_playback.loop_handle ? -1 : 0;
    PlaySequence((uint8)handle - 1, 1, repeat);
}

void SequencePlayback::CloseSequence(uint16 handle)
{
    FUNCTION_MARKER(0x8006AE90, "SLUS_005.78");
    CloseSequenceById(((uint8)handle - 1));
    CloseVabBank((handle >> 8) - 1);
}

void SequencePlayback::InitGameSound(void)
{
    SpuReverbAttr attributes = {0};
    FUNCTION_MARKER(0x8006AED0, "SLUS_005.78");
    _96_remove();
    InitSpuCold();
    // Initialize the mask omitted by the original startup code
    attributes.mask = 7;
    attributes.mode = 256;
    SetSpuReverbAttributes(&attributes);
    SpuSetTransferMode(0);
    InitSound();
    SetSequenceTrackTable(sequence_track_storage, 2, 1);
    SetSequenceTickMode(1);
    SetSequenceVoiceCount(16);
    StartSequenceVBlankTimer();
}
