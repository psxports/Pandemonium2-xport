#include <string.h>

#include "game.h"

SceneStream scene_streams[1] = {{0}};

uint32 ResetSceneStreams(void)
{
    uint32 first;
    uint32 second;
    uint32 base;
    uint32 object;
    uint32 index;
    SceneSection *section;
    FUNCTION_MARKER(0x80026AA0, "SLUS_005.78");
    w_u32(0x8008A4E8, 0);
    memset(&scene_streams[0], 0, sizeof(scene_streams[0]));
    first = r_u32(0x8008A4E0);
    base = r_u32(0x8008A284);
    second = base + 953344;
    scene_streams[0].header_offsets[1] = base + 87520;
    scene_streams[0].sample_offsets[1] = base + 5600;
    for (index = 0; index < 2; ++index)
    {
        object = index == 0 ? first : second;
        w_u32(0x8008A304 + 4 * index, object);
        section = (SceneSection *)psx_addr(object, sizeof(SceneSection));
        section->data_offset = object + 188;
        section->state_flags = 0;
    }
    object = r_u32(0x8008A308);
    scene_streams[0].pending_offset = object;
    ((SceneSection *)psx_addr(object, sizeof(SceneSection)))->section_index = -1;
    return 0xFFFFFFFF;
}

uint32 ActivateSceneSection(uint32 mode)
{
    FUNCTION_MARKER(0x80025BFC, "SLUS_005.78");
    SceneStream *stream = scene_streams + mode;
    uint32 pending = stream->pending_offset;
    uint32 current = stream->current_offset;
    uint32 spare = r_u32(0x8008A4E0);
    uint32 index = (uint32)((const SceneSection *)psx_addr(pending, sizeof(SceneSection)))->section_index;
    uint32 next = stream->pending_offset;
    uint32 parameters, value, flags, scale;
    sint32 factor;
    stream->pending_offset = current;
    stream->current_offset = next;
    parameters = r_u32(0x8008A4E4);
    current = stream->current_offset;
    parameters += index * 24;
    if (current == spare)
        stream->pending_offset = r_u32(0x8008A284) + 0xE8C00;
    else
        stream->pending_offset = spare;
    pending = stream->pending_offset;
    ((SceneSection *)psx_addr(pending, sizeof(SceneSection)))->data_offset = pending + 188;
    ((SceneSection *)psx_addr(stream->pending_offset, sizeof(SceneSection)))->section_index = -1;
    value = r_u32(parameters + 4);
    current = stream->current_offset;
    flags = game_flow.transition_flags;
    stream->checkpoint = value;
    scale = r_u16(parameters + 2);
    factor = ((const SceneSection *)psx_addr(current, sizeof(SceneSection)))->transition_scale;
    game_flow.transition_flags = flags | (1u << ((mode + 6) & 31));
    scale = (uint32)(((sint64)(sint32)scale * factor) >> 12);
    next = stream->current_offset;
    current = stream->current_offset;
    stream->transition_ticks = scale;
    stream->previous_offset = 0;
    stream->target_offset = next;
    flags = ((const SceneSection *)psx_addr(current, sizeof(SceneSection)))->state_flags;
    ((SceneSection *)psx_addr(current, sizeof(SceneSection)))->state_flags = flags & ~((1u << (mode & 31)) | 8);
    w_u16(0x800B99CE, 15);
    return 15;
}

uint32 ApplySceneSectionTransition(uint32 mode)
{
    FUNCTION_MARKER(0x800264C8, "SLUS_005.78");
    SceneStream *stream;
    uint32 scene, node, flags, process_new, x, y;
    p2_func_800260b0();
    InitializeContactPool();
    stream = scene_streams + mode;
    ExchangeSceneAudioBuffers(stream->target_offset, stream->previous_offset, mode);
    scene = stream->target_offset;
    stream->previous_offset = 0;
    w_u32(0x8008A4E8, scene);
    x = ((const SceneSection *)psx_addr(scene, sizeof(SceneSection)))->runtime_words[1];
    y = ((const SceneSection *)psx_addr(scene, sizeof(SceneSection)))->runtime_words[2];
    w_u32(0x1F80003C, x);
    w_u32(0x1F800040, y);
    if (GetCutsceneMode() != 0)
        StopCutscene();
    flags = ((const SceneSection *)psx_addr(stream->target_offset, sizeof(SceneSection)))->state_flags;
    process_new = ((flags >> 3) ^ 1) & 1;
    if (process_new != 0)
    {
        if ((flags & 0x10) == 0)
            p2_func_80066358(0x800B9938, 0, 50);
        scene = stream->target_offset;
        node = ((const SceneSection *)psx_addr(scene, sizeof(SceneSection)))->object_heads[14];
        while (node != 0)
        {
            flags = r_u8(node + 14);
            w_u8(node + 14, flags & 0xFB);
            if ((flags & 8) != 0)
                w_u8(node + 13, r_u8(node + 13) | 1);
            else
                w_u8(node + 13, r_u8(node + 13) & 0xFE);
            node = r_u32(node);
            if (node == 0)
                scene = stream->target_offset;
        }
        node = ((const SceneSection *)psx_addr(scene, sizeof(SceneSection)))->object_heads[4];
        while (node != 0)
        {
            uint32 kind;
            flags = r_u8(node + 14);
            kind = r_u8(node + 15);
            w_u8(node + 14, flags & 0xB7);
            if (kind - 40 < 16)
            {
                uint32 first = r_u32(node + 68);
                uint32 second = r_u32(node + 72);
                w_u32(node + 68, (first & 0xFFF) | ((kind - 39) << 12) | (second & 0x1FE0000));
            }
            else if ((sint8)kind == 2)
                process_new = 0;
            else if ((sint8)kind == 5)
            {
                w_u32(node + 72, 0);
                w_u32(node + 68, 0);
            }
            if (r_s8(node + 15) != 50 || (r_u16(node + 78) & 0x3FFF) != 0)
                w_u8(node + 13, r_u8(node + 13) & 0x7F);
            node = r_u32(node);
        }
        node = ((const SceneSection *)psx_addr(stream->target_offset, sizeof(SceneSection)))->object_heads[5];
        while (node != 0)
        {
            uint32 kind;
            flags = r_u8(node + 13);
            kind = r_u8(node + 15);
            w_u8(node + 13, flags & 0x7F);
            if (kind - 40 < 16)
            {
                uint32 first = r_u32(node + 68);
                uint32 second = r_u32(node + 72);
                sint32 current_kind = r_s8(node + 15);
                w_u32(node + 68, (first & 0xFFF) | ((kind - 39) << 12) | (second & 0x1FE0000));
                if (current_kind == 50)
                    w_u32(node + 76, r_u32(node + 76) | 0xFF);
            }
            node = r_u32(node);
        }
        node = ((const SceneSection *)psx_addr(r_u32(0x8008A4E8), sizeof(SceneSection)))->object_heads[10];
        while (node != 0)
        {
            uint32 object;
            p2_func_80034658(r_u32(node + 60));
            object = r_u32(node + 60);
            w_u32(object + 16, r_u32(object + 16) & 0xFFFFFFFA);
            object = r_u32(node + 60);
            if ((r_u32(object + 16) & 0x20) != 0)
            {
                w_u8(object + 13, r_u8(object + 13) | 1);
                object = r_u32(node + 60);
                w_u32(object + 16, r_u32(object + 16) | 1);
            }
            else
                w_u8(object + 13, r_u8(object + 13) & 0xFE);
            node = r_u32(node);
        }
        p2_func_80016c54(stream->target_offset, 4);
        scene = stream->target_offset;
        ((SceneSection *)psx_addr(scene, sizeof(SceneSection)))->state_flags = ((const SceneSection *)psx_addr(scene, sizeof(SceneSection)))->state_flags | 8;
        ((SceneSection *)psx_addr(stream->target_offset, sizeof(SceneSection)))->collected_count = 0;
        scene = stream->target_offset;
        node = r_u32(0x8008A114);
        x = ((const SceneSection *)psx_addr(scene, sizeof(SceneSection)))->origin[0];
        w_u32(node + 344, x);
        w_u32(node + 340, x);
    }
    scene = stream->target_offset;
    x = (uint32)(sint16)stream->checkpoint;
    y = (uint32)(sint16)stream->transition_ticks;
    p2_func_80053e7c((sint16)mode, scene, (sint32)x, (sint16)y);
    scene = stream->target_offset;
    flags = ((const SceneSection *)psx_addr(scene, sizeof(SceneSection)))->state_flags;
    x = r_u16(0x8008A530);
    ((SceneSection *)psx_addr(scene, sizeof(SceneSection)))->state_flags = flags | (1u << (mode & 31));
    if (x == 0)
    {
        scene = stream->target_offset;
        if (((const SceneSection *)psx_addr(scene, sizeof(SceneSection)))->sound_samples_offset != 0)
        {
            uint32 level = r_u32(0x8008A4E4);
            uint32 index = (uint32)((const SceneSection *)psx_addr(scene, sizeof(SceneSection)))->section_index;
            uint32 asset = r_u32(level + 240) + index * 9;
            if ((sint32)resource_archive.Offset(asset + 8) >= 0)
            {
                uint32 samples, header;
                scene = stream->target_offset;
                level = r_u32(0x8008A4E4);
                header = scene_streams[0].pending_offset;
                index = (uint32)((const SceneSection *)psx_addr(scene, sizeof(SceneSection)))->section_index;
                samples = ((const SceneSection *)psx_addr(scene, sizeof(SceneSection)))->sound_samples_offset;
                asset = r_u32(level + 240);
                header = ((const SceneSection *)psx_addr(header, sizeof(SceneSection)))->data_offset;
                asset += index * 9;
                x = p2_func_8006ac44(asset + 2, asset + 8, samples, header);
                w_u16(0x8008A530, x);
                if ((x & 0xFFFF) != 0)
                    p2_func_80066404();
                else
                    scene_loader.phase = 4;
            }
        }
    }
    if (r_u16(r_u32(0x8008A4E4) + 262) != 0)
    {
        if (process_new == 0)
            return 0;
        flags = ((const SceneSection *)psx_addr(stream->target_offset, sizeof(SceneSection)))->state_flags & 0x10;
        if (flags != 0)
            return flags;
        return RequestSceneSection(0xFFFFFFFF, mode);
    }
    if (r_s8(0x8008A43C) > 0 && r_u32(0x8008A130) == 0)
    {
        uint32 index = (uint32)r_s8(0x8008A43C);
        uint32 next = r_u32(0x80083CF4 + index * 4);
        next = (sint32)next < 3 ? next + 1 : next - 1;
        RequestSceneSection(next, mode);
    }
    {
        sint32 number = r_s8(0x8008A43C);
        sint32 high = (sint32)(((sint64)number * 0x66666667) >> 32);
        sint32 quotient = (high >> 2) - (number >> 31);
        uint32 level = r_u32(0x8008A4E4);
        uint32 event;
        w_u16(level + 266, stream->checkpoint);
        w_u16(level + 268, stream->transition_ticks);
        event = p2_func_8005bb90((sint8)quotient + 48, (sint8)(number - quotient * 10) + 48);
        return p2_func_8005bb48(event | 0x4000, 4);
    }
}

void ExchangeSceneAudioBuffers(uint32 scene_offset, uint32 previous_offset, uint32 mode)
{
    FUNCTION_MARKER(0x80026370, "SLUS_005.78");
    uint32 changed = 0;
    if (scene_offset == 0)
        return;
    if (previous_offset == 0 || ((const SceneSection *)psx_addr(scene_offset, sizeof(SceneSection)))->metadata_words[0] != ((const SceneSection *)psx_addr(previous_offset, sizeof(SceneSection)))->metadata_words[0])
        changed = 1;
    if (scene_offset == previous_offset)
        return;
    if (((const SceneSection *)psx_addr(scene_offset, sizeof(SceneSection)))->sound_samples_offset != 0 && changed != 0)
    {
        uint32 first, second;
        if (r_u16(0x8008A530) != 0)
        {
            p2_func_80066430();
            p2_func_8006ae90(r_u16(0x8008A530));
            w_u16(0x8008A530, 0);
        }
        first = scene_streams[mode].sample_offsets[0];
        second = scene_streams[mode].sample_offsets[1];
        scene_streams[mode].sample_offsets[1] = first;
        scene_streams[mode].sample_offsets[0] = second;
    }
    if (((const SceneSection *)psx_addr(scene_offset, sizeof(SceneSection)))->sound_header_offset != 0)
    {
        uint32 first = scene_streams[mode].header_offsets[0];
        uint32 second = scene_streams[mode].header_offsets[1];
        scene_streams[mode].header_offsets[1] = first;
        scene_streams[mode].header_offsets[0] = second;
    }
    else if (previous_offset != 0)
        ((SceneSection *)psx_addr(scene_offset, sizeof(SceneSection)))->sound_header_offset = 0;
}

uint32 HasPendingSceneSection(uint32 index)
{
    FUNCTION_MARKER(0x80025B30, "SLUS_005.78");
    if (scene_streams[index].pending_offset != 0)
        return 1;
    return r_u16(r_u32(0x8008A4E4) + 262) != 0;
}

uint32 SwapSceneSections(uint32 index)
{
    FUNCTION_MARKER(0x80025B7C, "SLUS_005.78");
    uint32 first, result;
    first = scene_streams[index].current_offset;
    result = scene_streams[index].pending_offset;
    scene_streams[index].pending_offset = first;
    scene_streams[index].current_offset = result;
    return result;
}

uint32 SetSceneTransition(uint32 slot, uint32 source_offset, uint32 scene_offset, sint32 index, uint32 flags)
{
    FUNCTION_MARKER(0x80025674, "SLUS_005.78");
    SceneStream *stream;
    uint32 first, second, value, replacement, record, coefficient, result;
    sint32 kind;
    stream = scene_streams + slot;
    stream->previous_offset = scene_offset;
    if (source_offset != 0)
        stream->target_offset = source_offset;
    else
    {
        first = stream->current_offset;
        ((SceneSection *)psx_addr(first, sizeof(SceneSection)))->state_flags = ((const SceneSection *)psx_addr(first, sizeof(SceneSection)))->state_flags & 0xFFF7;
        first = stream->current_offset;
        if ((((const SceneSection *)psx_addr(first, sizeof(SceneSection)))->state_flags & 0x10) != 0)
        {
            second = stream->pending_offset;
            if (second != 0)
            {
                kind = ((const SceneSection *)psx_addr(second, sizeof(SceneSection)))->section_index;
                if (kind < ((const SceneSection *)psx_addr(first, sizeof(SceneSection)))->section_index)
                {
                    ((SceneSection *)psx_addr(second, sizeof(SceneSection)))->state_flags = ((const SceneSection *)psx_addr(second, sizeof(SceneSection)))->state_flags & 0xFFF7;
                    second = stream->pending_offset;
                    if ((((const SceneSection *)psx_addr(second, sizeof(SceneSection)))->state_flags & 0x10) == 0)
                    {
                        first = stream->current_offset;
                        stream->target_offset = second;
                        scene_offset = stream->target_offset;
                        replacement = stream->target_offset;
                        stream->previous_offset = first;
                        stream->pending_offset = first;
                        stream->current_offset = replacement;
                    }
                }
            }
        }
        else
        {
            second = stream->pending_offset;
            if (second != 0)
                ((SceneSection *)psx_addr(second, sizeof(SceneSection)))->state_flags = ((const SceneSection *)psx_addr(second, sizeof(SceneSection)))->state_flags & 0xFFF7;
        }
    }
    stream->checkpoint = index;
    if ((sint16)index == -1)
        stream->transition_ticks = 10;
    else if ((sint16)index == -2)
    {
        kind = ((const SceneSection *)psx_addr(scene_offset, sizeof(SceneSection)))->section_index;
        record = r_u32(0x8008A4E4) + (uint32)kind * 24;
        value = r_u32(record + 4);
        stream->checkpoint = value;
        coefficient = r_u16(record + 2);
        kind = ((const SceneSection *)psx_addr(scene_offset, sizeof(SceneSection)))->transition_scale;
        value = (uint32)(((sint64)coefficient * kind) >> 12);
        stream->transition_ticks = value;
    }
    else
        stream->transition_ticks = flags;
    result = game_flow.transition_flags;
    result |= (uint32)1 << ((slot + 6) & 31);
    game_flow.transition_flags = result;
    return result;
}
