#include "game.h"

uint32 RelocateAnimationData(AnimationHeader *animation, uint32 base)
{
    uint32 remaining;
    uint32 pointer;
    uint32 offset;
    uint32 result;
    FUNCTION_MARKER(0x80068294, "SLUS_005.78");
    remaining = (uint32)animation->counts[0];
    if (remaining != 0)
    {
        pointer = base + animation->track_offsets[0];
        animation->track_offsets[0] = pointer;
        pointer += 12;
        do
        {
            offset = r_u32(pointer);
            if (offset != 0 && offset != 0xFFFFFFFF)
                w_u32(pointer, base + offset);
            --remaining;
            pointer += 20;
        } while (remaining != 0);
    }
    remaining = (uint32)animation->counts[1];
    if (remaining != 0)
    {
        pointer = base + animation->track_offsets[1];
        animation->track_offsets[1] = pointer;
        pointer += 12;
        do
        {
            offset = r_u32(pointer);
            if (offset != 0 && offset != 0xFFFFFFFF)
                w_u32(pointer, base + offset);
            --remaining;
            pointer += 16;
        } while (remaining != 0);
    }
    remaining = (uint32)animation->counts[2];
    if (remaining != 0)
    {
        pointer = base + animation->track_offsets[2];
        animation->track_offsets[2] = pointer;
        pointer += 12;
        do
        {
            offset = r_u32(pointer);
            if (offset != 0 && offset != 0xFFFFFFFF)
                w_u32(pointer, base + offset);
            --remaining;
            pointer += 16;
        } while (remaining != 0);
    }
    result = animation->track_offsets[3];
    if (result != 0)
    {
        remaining = (uint32)animation->counts[3];
        result += base;
        if (remaining != 0)
        {
            animation->track_offsets[3] = result;
            pointer = result + 16;
            do
            {
                offset = r_u32(pointer - 4);
                if (offset != 0)
                    w_u32(pointer - 4, base + offset);
                offset = r_u32(pointer);
                result = base + offset;
                if (offset != 0)
                    w_u32(pointer, result);
                --remaining;
                pointer += 24;
            } while (remaining != 0);
        }
    }
    return result;
}

uint32 AttachObjectAnimation(uint32 object, uint32 animation)
{
    uint32 result;
    FUNCTION_MARKER(0x8006A170, "SLUS_005.78");
    result = RelocateAnimationData((AnimationHeader *)psx_addr(animation, 24), animation);
    w_u32(object + 104, animation);
    return result;
}

uint32 LoadAnimationTable(uint32 object, uint32 source, uint32 *destination)
{
    uint32 remaining;
    uint32 record;
    uint32 offset;
    uint32 animation;
    uint32 pointer;
    uint32 table;
    uint32 result;
    FUNCTION_MARKER(0x80069FD8, "SLUS_005.78");
    remaining = r_u32(source) - 1;
    record = source + 4;
    do
    {
        offset = r_u32(record);
        animation = source + offset;
        if (offset != 0)
        {
            RelocateAnimationData((AnimationHeader *)psx_addr(animation, 24), animation);
            record += 4;
            *destination = animation;
            ++destination;
        }
        else
        {
            *destination = 0;
            ++destination;
            record += 4;
        }
        --remaining;
    } while (remaining != 0);
    offset = r_u32(record);
    table = source + offset;
    result = offset;
    if (offset != 0)
    {
        remaining = (uint32)r_s16(table);
        result = remaining;
        if (remaining != 0)
        {
            pointer = table + 8;
            do
            {
                result = source + r_u32(pointer);
                w_u32(pointer, result);
                pointer += 8;
                --remaining;
            } while (remaining != 0);
        }
    }
    if (object != 0)
    {
        result = offset != 0 ? table : 0;
        w_u32(object + 140, result);
    }
    return result;
}
