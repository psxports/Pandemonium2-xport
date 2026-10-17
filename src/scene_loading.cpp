#include "game.h"

SceneLoader scene_loader = {0};
const WorldLevelName *world_level_names = 0;

uint32 BeginSceneResourceLoad(uint32 destination_offset, uint32 asset)
{
    FUNCTION_MARKER(0x80069F18, "SLUS_005.78");
    scene_loader.destination_offset = destination_offset;
    scene_loader.asset = asset;
    return StartSceneResourceLoad();
}

uint32 StartSceneResourceLoad(void)
{
    FUNCTION_MARKER(0x8006997C, "SLUS_005.78");
    uint32 result;
    uint32 destination, asset;
    scene_loader.sound_samples_offset = 0;
    scene_loader.sound_header_offset = 0;
    if ((sint32)resource_archive.Size(scene_loader.asset) > 0)
    {
        uint32 object = scene_loader.destination_offset;
        asset = scene_loader.asset;
        destination = ((const SceneSection *)psx_addr(object, sizeof(SceneSection)))->sound_header_offset;
        scene_loader.phase = 1;
        result = ReadAssetToGuest(asset, destination, 0);
        scene_loader.sound_header_offset = result;
    }
    else if ((sint32)resource_archive.Size(scene_loader.asset + 2) > 0 && r_u32(0x8008A48C) != 0)
    {
        uint32 object = scene_loader.destination_offset;
        asset = scene_loader.asset;
        destination = ((const SceneSection *)psx_addr(object, sizeof(SceneSection)))->sound_samples_offset;
        scene_loader.phase = 3;
        result = ReadAssetToGuest(asset + 2, destination, 0);
        scene_loader.sound_samples_offset = result;
    }
    else
    {
        uint32 bytes, rounded, count;
        asset = scene_loader.asset;
        scene_loader.phase = 5;
        bytes = resource_archive.Size(asset + 7);
        rounded = bytes + 2047;
        if ((sint32)rounded < 0)
            rounded = bytes + 4094;
        count = (uint32)((sint32)rounded >> 11);
        if ((sint32)count < 87)
            destination = scene_loader.destination_offset + 0x2B800;
        else
        {
            bytes = resource_archive.Size(scene_loader.asset + 7);
            rounded = bytes + 2047;
            if ((sint32)rounded < 0)
                rounded = bytes + 4094;
            destination = scene_loader.destination_offset + ((uint32)((sint32)rounded >> 11) << 11);
        }
        result = ReadAssetToGuest(scene_loader.asset + 5, destination, 0);
        scene_loader.materials_offset = result;
    }
    return result;
}

uint32 PollSceneResourceLoad(void)
{
    uint32 asset;
    uint32 base;
    uint32 object;
    uint32 source;
    uint32 destination;
    uint32 loaded;
    uint32 palette;
    uint32 sample;
    uint32 bytes;
    uint32 aligned;
    uint32 state;
    sint32 index;
    sint32 shifted;
    uint16 handle;
    SceneSection *section;
    FUNCTION_MARKER(0x80069AC0, "SLUS_005.78");
    if (scene_loader.phase == 0)
        return 0;
    if (((uint32)resource_archive.Poll() << 24) == 0)
        return 1;
    state = (uint32)(sint32)scene_loader.phase;
    switch ((sint32)state)
    {
        case -4:
        case 4:
            object = r_u32(0x8008A4E8);
            base = r_u32(0x8008A4E4);
            index = ((const SceneSection *)psx_addr(object, sizeof(SceneSection)))->section_index;
            asset = r_u32(base + 240) + 9 * (uint32)index;
            if ((sint32)resource_archive.Offset(asset + 8) < 0 || r_u32(0x8008A48C) == 0)
            {
                scene_loader.phase = 0;
                return 1;
            }
            destination = GetSceneLoadDestination(0);
            object = r_u32(0x8008A4E8);
            base = r_u32(0x8008A4E4);
            index = ((const SceneSection *)psx_addr(object, sizeof(SceneSection)))->section_index;
            asset = r_u32(base + 240);
            source = ((const SceneSection *)psx_addr(object, sizeof(SceneSection)))->sound_samples_offset;
            asset += 9 * (uint32)index;
            handle = (uint16)p2_func_8006ac44(asset + 2, asset + 8, source, destination);
            w_u16(0x8008A530, handle);
            if (handle == 0)
                return 1;
            p2_func_80066404();
            if (scene_loader.phase == -4)
            {
                index = scene_loader.pending_section;
                scene_loader.phase = 0;
                RequestSceneSection((uint32)index, 0);
            }
            else
                scene_loader.phase = 0;
            return 1;
        case 1:
            asset = scene_loader.asset;
            destination = scene_loader.destination_offset;
            scene_loader.phase = 2;
            loaded = ReadAssetToGuest(asset + 1, destination + 188, 0);
            scene_loader.image_offset = loaded;
            return 1;
        case 2:
            if ((sint32)resource_archive.Size(scene_loader.asset + 2) > 0 && r_u32(0x8008A48C) != 0)
            {
                base = scene_loader.destination_offset;
                asset = scene_loader.asset;
                destination = ((const SceneSection *)psx_addr(base, sizeof(SceneSection)))->sound_samples_offset;
                scene_loader.phase = 3;
                loaded = ReadAssetToGuest(asset + 2, destination, 0);
                source = scene_loader.image_offset;
                index = r_s8(0x8008A514);
                scene_loader.sound_samples_offset = loaded;
                p2_func_8006624c(source, (uint32)(index + 1), 0);
                index = r_s8(0x8008A514);
                source = scene_loader.sound_header_offset;
                loaded = p2_func_800602f4(source);
                scene_loader.sound_header_offset = loaded;
                return 1;
            }
            index = r_s8(0x8008A514);
            source = scene_loader.image_offset;
            p2_func_8006624c(source, (uint32)(index + 1), 0);
            index = r_s8(0x8008A514);
            source = scene_loader.sound_header_offset;
            loaded = p2_func_800602f4(source);
            asset = scene_loader.asset;
            scene_loader.sound_header_offset = loaded;
            scene_loader.phase = 5;
            goto load_model;
        case 3:
            asset = scene_loader.asset;
            scene_loader.phase = 5;
        load_model:
            bytes = resource_archive.Size(asset + 7);
            shifted = (sint32)(bytes + 2047);
            if (shifted < 0)
                shifted = (sint32)(bytes + 4094);
            shifted >>= 11;
            if (shifted < 87)
            {
                base = scene_loader.destination_offset;
                aligned = 178176;
            }
            else
            {
                bytes = resource_archive.Size(scene_loader.asset + 7);
                shifted = (sint32)(bytes + 2047);
                if (shifted < 0)
                    shifted = (sint32)(bytes + 4094);
                base = scene_loader.destination_offset;
                aligned = (uint32)(shifted >> 11) << 11;
            }
            asset = scene_loader.asset;
            loaded = ReadAssetToGuest(asset + 5, base + aligned, 0);
            scene_loader.materials_offset = loaded;
            return 1;
        case 5:
            if ((sint32)resource_archive.Size(scene_loader.asset + 6) > 0)
            {
                asset = scene_loader.asset;
                destination = scene_loader.destination_offset;
                scene_loader.phase = 6;
                loaded = ReadAssetToGuest(asset + 6, destination + 188, 0);
                scene_loader.image_offset = loaded;
                return 1;
            }
            object = scene_loader.materials_offset;
            scene_loader.phase = 7;
            scene_loader.image_offset = 0;
            ((MaterialResourceHeader *)psx_addr(object, sizeof(MaterialResourceHeader)))->Relocate(object, 0);
            asset = scene_loader.asset;
            destination = scene_loader.destination_offset;
            ReadAssetToGuest(asset + 7, destination, 0);
            return 1;
        case 6:
            asset = scene_loader.asset;
            scene_loader.phase = 7;
            if ((sint32)resource_archive.Size(asset) > 0)
            {
                source = scene_loader.image_offset;
                index = r_s8(0x8008A514);
                palette = (uint32)index;
            }
            else
            {
                index = r_s8(0x8008A514);
                source = scene_loader.image_offset;
                palette = (uint32)(index + 2);
            }
            p2_func_8006624c(source, palette, 0);
            object = scene_loader.materials_offset;
            index = r_s8(0x8008A514);
            ((MaterialResourceHeader *)psx_addr(object, sizeof(MaterialResourceHeader)))->Relocate(object, index);
            return 1;
        case 7:
            if (scene_loader.image_offset != 0)
            {
                destination = scene_loader.destination_offset;
                asset = scene_loader.asset;
                scene_loader.image_offset = 0;
                ReadAssetToGuest(asset + 7, destination, 0);
                return 1;
            }
            source = scene_loader.materials_offset;
            base = scene_loader.destination_offset;
            scene_loader.phase = 0;
            section = (SceneSection *)psx_addr(base, sizeof(SceneSection));
            section->data_offset = base + 188;
            section->materials = *(const MaterialResourceHeader *)psx_addr(source, sizeof(MaterialResourceHeader));
            section = (SceneSection *)psx_addr(scene_loader.destination_offset, sizeof(SceneSection));
            section->Relocate(&section->materials);
            ((SceneSection *)psx_addr(scene_loader.destination_offset, sizeof(SceneSection)))->Activate();
            base = scene_loader.destination_offset;
            sample = scene_loader.sound_samples_offset;
            source = scene_loader.sound_header_offset;
            section = (SceneSection *)psx_addr(base, sizeof(SceneSection));
            section->sound_samples_offset = sample;
            section->sound_header_offset = source;
            return 0;
        default:
            return 1;
    }
}

uint32 QueueSceneSection(uint32 scene)
{
    FUNCTION_MARKER(0x80069AAC, "SLUS_005.78");
    scene_loader.pending_section = scene;
    scene_loader.phase = -4;
    return 0xFFFFFFFC;
}


uint32 GetSceneLoadDestination(uint32 mode)
{
    FUNCTION_MARKER(0x80025BC0, "SLUS_005.78");
    return ((const SceneSection *)psx_addr(scene_streams[mode].pending_offset, sizeof(SceneSection)))->data_offset;
}

uint32 RequestSceneSection(uint32 scene, uint32 mode)
{
    FUNCTION_MARKER(0x80025D3C, "SLUS_005.78");
    SceneStream *stream;
    uint32 current, pending;
    sint32 state;
    SceneSection *section;
    if ((game_flow.transition_flags & 2) != 0 && r_u8(r_u32(0x8008A4E4) + 264) != 0)
        return 0;
    stream = scene_streams + mode;
    current = stream->current_offset;
    if (current != 0)
    {
        if (scene == 0xFFFFFFFF)
            scene = (uint32)(((const SceneSection *)psx_addr(current, sizeof(SceneSection)))->section_index + 1);
        if ((sint32)scene >= r_u8(r_u32(0x8008A4E4) + 265))
            return 0;
        if ((sint32)scene == ((const SceneSection *)psx_addr(current, sizeof(SceneSection)))->section_index)
            goto check_pending;
    }
    pending = stream->pending_offset;
    if (pending != 0 && (sint32)scene == ((const SceneSection *)psx_addr(pending, sizeof(SceneSection)))->section_index)
    {
    check_pending:
        if (r_u8(r_u32(0x8008A4E4) + 264) != 0)
            return 0;
    }
    state = scene_loader.phase;
    if (state != 0)
    {
        if (state == 4)
            QueueSceneSection(scene);
        return 1;
    }
    {
        uint32 level = r_u32(0x8008A4E4);
        uint32 count = r_u8(level + 264);
        w_u8(level + 264, count + 1);
        if (count >= 2)
        {
            uint32 data;
            uint32 total;
            level = r_u32(0x8008A4E4);
            data = stream->pending_offset;
            total = r_u16(level + 272);
            w_u16(level + 272, total + ((const SceneSection *)psx_addr(data, sizeof(SceneSection)))->collected_count);
        }
        section = (SceneSection *)psx_addr(stream->pending_offset, sizeof(SceneSection));
        section->section_index = scene;
        PrepareSceneSectionLoad();
    }
    return 1;
}

uint32 PrepareSceneSectionLoad(void)
{
    FUNCTION_MARKER(0x800258A0, "SLUS_005.78");
    uint32 pending = scene_streams[0].pending_offset;
    SceneSection *section;
    uint32 level = r_u32(0x8008A4E4);
    sint32 index = ((const SceneSection *)psx_addr(pending, sizeof(SceneSection)))->section_index;
    uint32 assets = r_u32(level + 240) + (uint32)index * 9;
    uint32 end, destination, bytes, rounded, shared, current;
    if (pending == r_u32(0x8008A4E0))
        w_u8(0x8008A514, 2);
    else
    {
        end = r_u32(0x8008A284) + 0x132400;
        bytes = resource_archive.Size(assets + 7);
        rounded = bytes + 2047;
        if ((sint32)rounded < 0)
            rounded = bytes + 4094;
        destination = end - ((uint32)((sint32)rounded >> 11) << 11);
        bytes = resource_archive.Size(assets + 5);
        if ((sint32)bytes > 0)
        {
            rounded = bytes + 2047;
            if ((sint32)rounded < 0)
                rounded = bytes + 4094;
            destination -= (uint32)((sint32)rounded >> 11) << 11;
        }
        if ((sint32)(end - destination) <= 0x497FF)
            destination = end - 0x49800;
        scene_streams[0].pending_offset = destination;
        w_u8(0x8008A514, 5);
    }
    pending = scene_streams[0].pending_offset;
    section = (SceneSection *)psx_addr(pending, sizeof(SceneSection));
    section->section_index = index;
    section->data_offset = pending + 188;
    end = r_u32(0x8008A284);
    section->state_flags = 0;
    section->links.sibling_next = 0;
    section->links.first_child = 0;
    if (scene_streams[0].header_offsets[1] != end + 0x155E0)
    {
        bytes = resource_archive.Size(assets);
        if ((sint32)bytes > 0)
        {
            end = r_u32(0x8008A284) + 0x205E0;
            rounded = bytes + 2047;
            if ((sint32)rounded < 0)
                rounded = bytes + 4094;
            scene_streams[0].header_offsets[1] = end - ((uint32)((sint32)rounded >> 11) << 11);
        }
    }
    section->sound_header_offset = scene_streams[0].header_offsets[1];
    current = scene_streams[0].current_offset;
    w_u32(0x8008A48C, 1);
    if (current != 0 && r_u32(r_u32(0x8008A4E4) + 276) == 0)
    {
        uint32 data = ((const SceneSection *)psx_addr(current, sizeof(SceneSection)))->sound_samples_offset;
        w_u32(0x8008A48C, 0);
        scene_streams[0].sample_offsets[1] = data;
    }
    end = r_u32(0x8008A284);
    shared = scene_streams[0].sample_offsets[1];
    if (shared != end + 0x15E0 && r_u32(0x8008A48C) != 0)
    {
        bytes = resource_archive.Size(assets + 2);
        if ((sint32)bytes > 0)
        {
            end = r_u32(0x8008A284) + 0x155E0;
            rounded = bytes + 2047;
            if ((sint32)rounded < 0)
                rounded = bytes + 4094;
            scene_streams[0].sample_offsets[1] = end - ((uint32)((sint32)rounded >> 11) << 11);
        }
        shared = scene_streams[0].sample_offsets[1];
    }
    index = (uint8)section->section_index;
    level = r_u32(0x8008A4E4);
    w_u8(0x8008A30C, (uint32)index);
    section->sound_samples_offset = shared;
    return BeginSceneResourceLoad(pending, r_u32(level + 240) + (uint32)(sint32)(sint8)index * 9);
}

uint32 CountSceneTreasure(uint32 player)
{
    FUNCTION_MARKER(0x80025620, "SLUS_005.78");
    uint32 scene = r_u32(0x8008A4E4);
    SceneStream *stream = scene_streams + player;
    uint32 source = stream->current_offset;
    uint32 multiplayer = r_u8(scene + 264);
    uint32 result = ((const SceneSection *)psx_addr(source, sizeof(SceneSection)))->collected_count;
    if (multiplayer >= 2)
        result += ((const SceneSection *)psx_addr(stream->pending_offset, sizeof(SceneSection)))->collected_count;
    return result;
}
