#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "material_animation_list.h"
#include "game.h"
#include "hd.h"

void MaterialAnimationList::ResetMaterialAnimationList(uint32 *entries)
{
    uint32 frame;
    FUNCTION_MARKER(0x80027594, "SLUS_005.78");
    if (entries != 0)
    {
        frame = game_tick;
        MaterialAnimationListHeader *header = (MaterialAnimationListHeader *)entries - 1;
        header->start_tick = frame;
        hd_scroll_reset();
    }
}

sint32 MaterialAnimationList::UpdateMaterialAnimationList(uint32 *entries)
{
    uint32 *pointer = entries;
    const MaterialAnimationListHeader *header;
    uint32 remaining;
    uint32 frame;
    FUNCTION_MARKER(0x800275BC, "SLUS_005.78");
    if (entries == 0)
        return 1;
    header = (const MaterialAnimationListHeader *)entries - 1;
    remaining = header->count - 1;
    frame = game_tick - header->start_tick;
    if (remaining != 0)
    {
        do
        {
            MaterialRecord::UpdateMaterialAnimation(pointer, frame);
            --remaining;
            ++pointer;
        } while (remaining != 0);
    }
    return (sint8)MaterialRecord::UpdateMaterialAnimation(pointer, frame);
}
