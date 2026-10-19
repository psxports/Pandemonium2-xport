#include <stdio.h>
#include <stdlib.h>

#include "game.h"

uint32 UpdateMaterialAnimation(uint32 *entry, uint32 frame)
{
    uint32 encoded;
    uint32 index;
    MaterialRecord *record;
    TextureDescriptor *texture;
    uint32 kind;
    uint8 *animation;
    uint32 flags;
    uint32 phase;
    uint32 last_frame;
    uint32 period;
    uint32 remainder;
    uint32 selected;
    uint32 random_frame;
    uint32 previous_random;
    const uint8 *source;
    uint32 words[2];
    uint16 page;
    FUNCTION_MARKER(0x80027314, "SLUS_005.78");
    encoded = *entry;
    if (encoded == 0xFFFFFFFF)
        return 1;
    index = (encoded >> 1) & 0x1FF;
    if (encoded & 1)
    {
        SceneSection *section = scene_stream.active_section;
        if (index >= section->materials.material_count || section->materials.materials == NULL)
        {
            fprintf(stderr, "P2: Invalid scene material index\n");
            abort();
        }
        record = section->materials.materials + index;
    }
    else
        record = material_registry.records + index;
    kind = record->kind;
    animation = record->animation;
    if (kind == 1)
    {
        phase = (uint32)(((sint64)(sint32)frame * (sint32)xport_load_le32(animation)) >> 16);
        record->texture->payload.words[2] = phase;
        return 1;
    }
    if (kind == 0)
    {
        record->CyclePalette(animation, frame);
        return 1;
    }
    if (kind != 2)
        return 1;
    flags = (*entry >> 26) & 15;
    phase = (uint32)(((sint64)(sint32)frame * (sint32)xport_load_le32(animation)) >> 16);
    last_frame = animation[5];
    period = (flags & 2) ? 2 * last_frame : last_frame + 1;
    if ((flags & 8) != 0 && phase >= period)
        return 0;
    if (period == 0)
        trap(7);
    remainder = phase % period;
    selected = remainder;
    random_frame = 0;
    if (flags & 1)
        selected = animation[5] - remainder;
    else if (flags & 2)
    {
        last_frame = animation[5];
        if ((sint32)(last_frame - remainder) < 0)
            selected = 2 * last_frame - remainder;
    }
    else if ((flags & 4) != 0 && remainder != ((*entry >> 10) & 255))
    {
        previous_random = (*entry >> 18) & 255;
        do
        {
            selected = random_generator.Next() % period;
        } while (selected == previous_random);
        random_frame = selected;
    }
    texture = record->texture;
    source = record->data + 8 * selected;
    page = texture->payload.halfwords[3];
    words[0] = xport_load_le32(source);
    words[1] = xport_load_le32(source + 4);
    texture->payload.words[0] = words[0];
    texture->payload.words[1] = words[1];
    record->texture->payload.halfwords[3] = page;
    *entry = (*entry & 1) | (index << 1) | (remainder << 10) | (random_frame << 18) | (flags << 26);
    return 1;
}

unsigned int MaterialRecord::CyclePalette(uint8 *animation, unsigned int frame) const
{
    uint32 remaining;
    uint8 *palette;
    uint8 *cycle;
    uint32 phase;
    sint32 delta;
    sint32 difference;
    sint32 distance;
    uint8 *destination;
    uint8 *source;
    uint32 length;
    uint32 first_bytes;
    uint32 second_bytes;
    uint16 saved;
    uint8 temporary[512];
    FUNCTION_MARKER(0x80027034, "SLUS_005.78");
    remaining = cycle_count;
    palette = data;
    cycle = animation + 4;
    do
    {
        phase = (uint32)(((sint64)(sint32)frame * (sint32)xport_load_le32(animation)) >> 16);
        difference = (sint32)cycle[1] - cycle[0];
        distance = difference > 0 ? difference : -difference;
        phase %= (uint32)distance + 1;
        delta = (sint32)cycle[2] - (sint32)phase;
        if (delta != 0)
        {
            cycle[2] = (uint8)phase;
            if (delta == -1 || delta == distance)
            {
                destination = palette + 2 * cycle[1];
                saved = xport_load_le16(destination);
                if (difference > 0)
                {
                    source = palette + 2 * cycle[0];
                    destination = source + 2;
                }
                else
                    source = destination + 2;
                MoveBytes(destination, source, 2 * (uint32)distance);
                xport_store_le16(palette + 2 * cycle[0], saved);
            }
            else
            {
                if (difference >= 0)
                {
                    delta = -delta;
                    destination = palette + 2 * cycle[0];
                }
                else
                    destination = palette + 2 * cycle[1];
                length = (uint32)distance + 1;
                CopyBytes(temporary, destination, 2 * length);
                if (delta <= 0)
                {
                    delta = -delta;
                    second_bytes = 2 * (uint32)delta;
                    first_bytes = 2 * (length - (uint32)delta);
                    source = temporary + second_bytes;
                }
                else
                {
                    second_bytes = 2 * (length - (uint32)delta);
                    first_bytes = 2 * (uint32)delta;
                    source = temporary + second_bytes;
                }
                CopyBytes(destination, source, first_bytes);
                CopyBytes(destination + first_bytes, temporary, second_bytes);
            }
        }
        --remaining;
        if (remaining != 0)
        {
            cycle += 8;
            animation += 8;
        }
    } while (remaining != 0);
    return UploadPalette();
}

unsigned int MaterialRecord::UploadPalette() const
{
    const TextureDescriptor *texture;
    const uint8 *palette;
    uint16 clut;
    uint16 page;
    FUNCTION_MARKER(0x80064D88, "SLUS_005.78");
    texture = this->texture;
    clut = texture->payload.halfwords[3];
    page = texture->payload.halfwords[2];
    palette = data;
    return UploadTexturePalette(clut, page, palette);
}

uint32 SetMaterialAnimationFlags(uint32 *entry, uint32 flags)
{
    FUNCTION_MARKER(0x80027648, "SLUS_005.78");
    uint32 value, index, result;
    uint32 kind;
    value = *entry;
    result = value >> 1;
    if (value == 0xFFFFFFFF)
        return result;
    index = result & 0x1FF;
    if ((value & 1) != 0)
    {
        const SceneSection *section = scene_stream.active_section;
        if (index >= section->materials.material_count || section->materials.materials == NULL)
        {
            fprintf(stderr, "P2: Invalid scene material flag index\n");
            abort();
        }
        kind = section->materials.materials[index].kind;
    }
    else
        kind = material_registry.records[index].kind;
    result = index * 2;
    if (kind == 2)
    {
        result |= (value & 1) | (flags << 26);
        *entry = result;
    }
    return result;
}

void ResetMaterialAnimationList(uint32 *entries)
{
    uint32 frame;
    uint32 count;
    FUNCTION_MARKER(0x80027594, "SLUS_005.78");
    if (entries != 0)
    {
        frame = frame_clock.render_tick;
        count = (uint8)entries[-1];
        entries[-1] = count | (frame << 8);
    }
}

sint32 UpdateMaterialAnimationList(uint32 *entries)
{
    uint32 *pointer = entries;
    uint32 header;
    uint32 remaining;
    uint32 frame;
    FUNCTION_MARKER(0x800275BC, "SLUS_005.78");
    if (entries == 0)
        return 1;
    header = entries[-1];
    remaining = (header & 255) - 1;
    frame = frame_clock.render_tick - (header >> 8);
    if (remaining != 0)
    {
        do
        {
            UpdateMaterialAnimation(pointer, frame);
            --remaining;
            ++pointer;
        } while (remaining != 0);
    }
    return (sint8)UpdateMaterialAnimation(pointer, frame);
}

uint32 UpdateResourceMaterialAnimations(void)
{
    uint32 index;
    uint32 saved_frame;
    uint32 result;
    FUNCTION_MARKER(0x80027228, "SLUS_005.78");
    for (index = 0; index < 28; ++index)
    {
        if (index == 1 || index == 5 || ((game_flow.transition_flags & 0x10) == 0 && index >= 13))
        {
            saved_frame = frame_clock.render_tick;
            if (index == 1 || index == 5)
                frame_clock.render_tick = frame_clock.menu_tick;
            UpdateMaterialAnimationList(resource_groups[index].animation_entries);
            frame_clock.render_tick = saved_frame;
        }
    }
    result = frame_clock.menu_tick + 1;
    frame_clock.menu_tick = result;
    return result;
}
