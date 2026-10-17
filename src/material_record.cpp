#include "game.h"

uint32 UpdateMaterialAnimation(uint32 *entry, uint32 frame)
{
    uint32 encoded;
    uint32 index;
    uint32 base;
    MaterialRecord *record;
    TextureDescriptor *texture;
    uint32 kind;
    uint32 animation;
    uint32 flags;
    uint32 phase;
    uint32 last_frame;
    uint32 period;
    uint32 remainder;
    uint32 selected;
    uint32 random_frame;
    uint32 previous_random;
    uint32 packet;
    uint32 source;
    uint32 words[2];
    uint16 page;
    FUNCTION_MARKER(0x80027314, "SLUS_005.78");
    encoded = *entry;
    if (encoded == 0xFFFFFFFF)
        return 1;
    index = (encoded >> 1) & 0x1FF;
    if (encoded & 1)
    {
        base = ((const SceneSection *)psx_addr(r_u32(0x8008A4E8), sizeof(SceneSection)))->materials.materials_offset;
        record = (MaterialRecord *)psx_addr(base + 16 * index, sizeof(MaterialRecord));
    }
    else
        record = material_registry.records + index;
    kind = record->kind;
    animation = record->animation_offset;
    if (kind == 1)
    {
        phase = (uint32)(((sint64)(sint32)frame * r_s32(animation)) >> 16);
        ((TextureDescriptor *)psx_addr(record->texture_offset, sizeof(TextureDescriptor)))->payload.words[2] = phase;
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
    phase = (uint32)(((sint64)(sint32)frame * r_s32(animation)) >> 16);
    last_frame = r_u8(animation + 5);
    period = (flags & 2) ? 2 * last_frame : last_frame + 1;
    if ((flags & 8) != 0 && phase >= period)
        return 0;
    if (period == 0)
        trap(7);
    remainder = phase % period;
    selected = remainder;
    random_frame = 0;
    if (flags & 1)
        selected = r_u8(animation + 5) - remainder;
    else if (flags & 2)
    {
        last_frame = r_u8(animation + 5);
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
    packet = record->texture_offset;
    texture = (TextureDescriptor *)psx_addr(packet, sizeof(TextureDescriptor));
    source = record->data_offset + 8 * selected;
    page = texture->payload.halfwords[3];
    words[0] = r_u32(source);
    words[1] = r_u32(source + 4);
    texture->payload.words[0] = words[0];
    texture->payload.words[1] = words[1];
    ((TextureDescriptor *)psx_addr(record->texture_offset, sizeof(TextureDescriptor)))->payload.halfwords[3] = page;
    *entry = (*entry & 1) | (index << 1) | (remainder << 10) | (random_frame << 18) | (flags << 26);
    return 1;
}

unsigned int MaterialRecord::CyclePalette(unsigned int animation, unsigned int frame) const
{
    uint32 remaining;
    uint32 palette;
    uint32 cycle;
    uint32 phase;
    sint32 delta;
    sint32 difference;
    sint32 distance;
    uint32 destination;
    uint32 source;
    uint32 length;
    uint32 first_bytes;
    uint32 second_bytes;
    uint16 saved;
    uint8 temporary[512];
    FUNCTION_MARKER(0x80027034, "SLUS_005.78");
    remaining = cycle_count;
    palette = data_offset;
    cycle = animation + 4;
    do
    {
        phase = (uint32)(((sint64)(sint32)frame * r_s32(animation)) >> 16);
        difference = (sint32)r_u8(cycle + 1) - r_u8(cycle);
        distance = difference > 0 ? difference : -difference;
        phase %= (uint32)distance + 1;
        delta = (sint32)r_u8(cycle + 2) - (sint32)phase;
        if (delta != 0)
        {
            w_u8(cycle + 2, phase);
            if (delta == -1 || delta == distance)
            {
                destination = palette + 2 * r_u8(cycle + 1);
                saved = r_u16(destination);
                if (difference > 0)
                {
                    source = palette + 2 * r_u8(cycle);
                    destination = source + 2;
                }
                else
                    source = destination + 2;
                p2_func_800663ac(destination, source, 2 * (uint32)distance);
                w_u16(palette + 2 * r_u8(cycle), saved);
            }
            else
            {
                if (difference >= 0)
                {
                    delta = -delta;
                    destination = palette + 2 * r_u8(cycle);
                }
                else
                    destination = palette + 2 * r_u8(cycle + 1);
                length = (uint32)distance + 1;
                p2_func_8006637c(temporary, (const uint8 *)psx_addr(destination, 2 * length), 2 * length);
                if (delta <= 0)
                {
                    delta = -delta;
                    second_bytes = 2 * (uint32)delta;
                    first_bytes = 2 * (length - (uint32)delta);
                    source = second_bytes;
                }
                else
                {
                    second_bytes = 2 * (length - (uint32)delta);
                    first_bytes = 2 * (uint32)delta;
                    source = second_bytes;
                }
                p2_func_8006637c((uint8 *)psx_addr(destination, first_bytes), temporary + source, first_bytes);
                p2_func_8006637c((uint8 *)psx_addr(destination + first_bytes, second_bytes), temporary, second_bytes);
            }
        }
        cycle += 8;
        animation += 8;
        --remaining;
    } while (remaining != 0);
    return UploadPalette();
}

unsigned int MaterialRecord::UploadPalette() const
{
    const TextureDescriptor *texture;
    uint32 palette;
    uint16 clut;
    uint16 page;
    FUNCTION_MARKER(0x80064D88, "SLUS_005.78");
    texture = (const TextureDescriptor *)psx_addr(texture_offset, sizeof(TextureDescriptor));
    palette = data_offset;
    clut = texture->payload.halfwords[3];
    page = texture->payload.halfwords[2];
    return p2_func_80066b78(clut, page, palette);
}

uint32 SetMaterialAnimationFlags(uint32 *entry, uint32 flags)
{
    FUNCTION_MARKER(0x80027648, "SLUS_005.78");
    uint32 value, index, table, result;
    MaterialRecord *record;
    value = *entry;
    result = value >> 1;
    if (value == 0xFFFFFFFF)
        return result;
    index = result & 0x1FF;
    if ((value & 1) != 0)
    {
        table = ((const SceneSection *)psx_addr(r_u32(0x8008A4E8), sizeof(SceneSection)))->materials.materials_offset;
        record = (MaterialRecord *)psx_addr(table + index * 16, sizeof(MaterialRecord));
    }
    else
        record = material_registry.records + index;
    result = index * 2;
    if (record->kind == 2)
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
