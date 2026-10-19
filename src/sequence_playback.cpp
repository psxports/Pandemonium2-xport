#include <stdio.h>
#include <stdlib.h>

#include "game.h"

SpuSequenceSchedulerTrack sequence_track_storage[2];

SequencePlayback sequence_playback = {0, 0, 0, (uint16)OriginalDefaults::SequenceLoadingBank, 0, NULL, 0};

uint32 LoadLoopSequence(const uint8 *bundle, uint32 bundle_bytes)
{
    uint32 bank;
    uint32 offset;
    uint32 sequence;
    uint32 bytes;
    uint32 result;
    FUNCTION_MARKER(0x8006AAC8, "SLUS_005.78");
    bytes = resource_archive.Size(18);
    if (bundle == NULL || bytes > bundle_bytes)
    {
        fprintf(stderr, "P2: Invalid loop VH span\n");
        abort();
    }
    bank = p2_sdk_80079f48(bundle, bytes, -1, 4112);
    offset = resource_archive.Offset(20);
    bytes = resource_archive.Size(20);
    if (offset > bundle_bytes || bytes > bundle_bytes - offset)
    {
        fprintf(stderr, "P2: Invalid loop VB span\n");
        abort();
    }
    p2_sdk_8007a680(bundle + offset, bytes, (sint16)bank);
    while ((p2_sdk_8007a740(0) << 16) == 0)
        ;
    offset = resource_archive.Offset(19);
    bytes = resource_archive.Size(19);
    if (offset > bundle_bytes || bytes > bundle_bytes - offset)
    {
        fprintf(stderr, "P2: Invalid loop SEQ span\n");
        abort();
    }
    sequence = p2_sdk_80071c8c(bundle + offset, bytes, (sint16)bank);
    result = ((sequence + 1) & 255) << 8;
    sequence_playback.loop_handle = (uint16)(((bank + 1) & 255) | result);
    return result;
}

void UnloadLoopSequence(void)
{
    FUNCTION_MARKER(0x8006AB74, "SLUS_005.78");
    uint16 handle = sequence_playback.loop_handle;
    if (handle != 0)
    {
        CloseSequence(handle);
        sequence_playback.loop_handle = 0;
    }
}

uint32 PrepareSavedSequence(void)
{
    FUNCTION_MARKER(0x8006ABA0, "SLUS_005.78");
    uint32 sequence, volume;
    sequence = (uint8)sequence_playback.saved_handle;
    volume = sound_settings.music_volume;
    sequence = (sequence - 1) & 0xFF;
    volume = (127 * volume) >> 12;
    p2_sdk_80075d58((uint8)sequence, (sint16)volume, (sint16)volume);
    return sequence_playback.saved_handle;
}

void PlayLoopSequence(void)
{
    FUNCTION_MARKER(0x8006ABE4, "SLUS_005.78");
    uint32 sequence, volume, previous, current;
    sequence = (uint8)sequence_playback.loop_handle;
    volume = sound_settings.effects_volume;
    sequence = (sequence - 1) & 0xFF;
    volume = (127 * volume) >> 12;
    p2_sdk_80075d58((uint8)sequence, (sint16)volume, (sint16)volume);
    previous = sequence_playback.current_handle;
    current = sequence_playback.loop_handle;
    sequence_playback.saved_handle = previous;
    sequence_playback.current_handle = current;
    StartCurrentSequence();
}

uint32 LoadSequence(uint32 asset, uint32 mode, const uint8 *header, uint32 header_bytes, const uint8 *samples, uint32 sample_bytes)
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
        if (header == NULL || offset > header_bytes || bytes > header_bytes - offset)
        {
            fprintf(stderr, "P2: Invalid scene/menu VH span\n");
            abort();
        }
        sequence_playback.loading_bank = p2_sdk_80079f48(header + offset, bytes, -1, 11568);
        if (mode != 0)
        {
            uint32 capacity;
            bytes = resource_archive.Size(mode);
            if (bytes > 0xFFFFF800u)
            {
                fprintf(stderr, "P2: Invalid VB asset size %u\n", bytes);
                abort();
            }
            capacity = (bytes + 2047) & ~2047u;
            sequence_playback.loading_samples = (uint8 *)malloc(capacity != 0 ? capacity : 1);
            if (sequence_playback.loading_samples == NULL)
            {
                fprintf(stderr, "P2: Cannot allocate VB asset buffer\n");
                abort();
            }
            sequence_playback.loading_sample_bytes = bytes;
            resource_archive.ReadAsset(mode, sequence_playback.loading_samples, 0);
            sequence_playback.loading_phase = 2;
            return 0;
        }
        p2_sdk_8007a680(samples, sample_bytes, (sint16)sequence_playback.loading_bank);
        sequence_playback.loading_phase = 1;
        return 0;
    }
    if (phase == 2)
    {
        if (((uint32)resource_archive.Poll() << 24) == 0)
            return 0;
        p2_sdk_8007a680(sequence_playback.loading_samples, sequence_playback.loading_sample_bytes, (sint16)sequence_playback.loading_bank);
        sequence_playback.loading_phase = 1;
        return 0;
    }
    if (phase == 1 && (p2_sdk_8007a740(0) << 16) == 0)
        return 0;
    sequence_playback.loading_phase = 0;
    free(sequence_playback.loading_samples);
    sequence_playback.loading_samples = NULL;
    sequence_playback.loading_sample_bytes = 0;
    offset = resource_archive.Offset(asset + 2);
    bytes = resource_archive.Size(asset + 2);
    if (header == NULL || offset > header_bytes || bytes > header_bytes - offset)
    {
        fprintf(stderr, "P2: Invalid scene/menu SEQ span\n");
        abort();
    }
    sequence = p2_sdk_80071c8c(header + offset, bytes, (sint16)sequence_playback.loading_bank);
    volume = (127 * sound_settings.music_volume) >> 12;
    p2_sdk_80075d58((uint8)sequence, (sint16)volume, (sint16)volume);
    result = (((uint8)sequence_playback.loading_bank + 1) & 255) | (((sequence + 1) & 255) << 8);
    sequence_playback.saved_handle = result;
    return result;
}

uint32 PauseSequence(uint16 handle)
{
    FUNCTION_MARKER(0x8006AD80, "SLUS_005.78");
    return p2_sdk_80074210((sint16)((sint32)(uint8)handle - 1));
}

uint32 ResumeSequence(uint16 handle)
{
    FUNCTION_MARKER(0x8006ADA4, "SLUS_005.78");
    return p2_sdk_80074398((sint16)((sint32)(uint8)handle - 1));
}

uint32 StopSequence(uint8 handle)
{
    FUNCTION_MARKER(0x8006ADC8, "SLUS_005.78");
    p2_sdk_80075980((sint32)handle - 1);
    p2_func_800765b0();
    p2_sdk_80070a24(1);
    p2_sdk_800763ec(0);
    return p2_sdk_80076354(0, 0);
}

uint32 StartSequence(uint16 handle)
{
    sint32 repeat;
    FUNCTION_MARKER(0x8006AE10, "SLUS_005.78");
    p2_func_800765b0();
    p2_sdk_80070a24(1);
    p2_sdk_800763ec(1);
    p2_sdk_80076354(36, 36);
    p2_func_800765d0();
    repeat = handle == sequence_playback.loop_handle ? -1 : 0;
    return p2_sdk_80074264((sint32)(uint8)handle - 1, 1, repeat);
}

uint32 CloseSequence(uint16 handle)
{
    FUNCTION_MARKER(0x8006AE90, "SLUS_005.78");
    p2_func_80071a68((sint16)((sint32)(uint8)handle - 1));
    return p2_sdk_80079b14((sint32)(handle >> 8) - 1);
}

uint32 SetSequenceVolume(uint32 handle, uint16 volume)
{
    FUNCTION_MARKER(0x8006A5E0, "SLUS_005.78");
    sint16 scaled = (sint16)(((uint32)volume * 127) >> 12);
    return p2_sdk_80075d58((uint8)handle, scaled, scaled);
}

void StartCurrentSequence(void)
{
    uint16 handle;
    FUNCTION_MARKER(0x80066404, "SLUS_005.78");
    handle = sequence_playback.current_handle;
    if (handle != 0)
        StartSequence(handle);
}

void StopCurrentSequence(void)
{
    uint16 handle;
    FUNCTION_MARKER(0x80066430, "SLUS_005.78");
    handle = sequence_playback.current_handle;
    if (handle != 0)
        StopSequence((uint8)handle);
}
