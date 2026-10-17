#include "game.h"
#include "original_defaults.h"
#include <stdlib.h>

CharacterController character_controller = {0};

unsigned int character_action_flags = OriginalDefaults::CharacterActionFlags;
unsigned int action_effect_templates[15] = {0};
unsigned int character_action_owner_offset = OriginalDefaults::CharacterActionOwner;
unsigned int character_action_target_offset = OriginalDefaults::CharacterActionTarget;
unsigned int character_sound_resume_flags = OriginalDefaults::CharacterSoundResumeFlags;

static uint32 ActionDistance(uint32 first_offset, uint32 second_offset)
{
    FUNCTION_MARKER(0x80028CC0, "SLUS_005.78");
    uint32 x = r_u32(second_offset) - r_u32(first_offset);
    uint32 y = r_u32(second_offset + 4) - r_u32(first_offset + 4);
    uint32 z = r_u32(second_offset + 8) - r_u32(first_offset + 8);
    if ((sint32)x < 0)
        x = 0u - x;
    if ((sint32)y < 0)
        y = 0u - y;
    if ((sint32)z < 0)
        z = 0u - z;
    return 2 * (x + z) + y;
}

uint32 CharacterController::ClearTransientFlags()
{
    FUNCTION_MARKER(0x8001C684, "SLUS_005.78");
    uint32 result = flags & 0xFFFE7FBF;
    flags = result;
    return result;
}

void CharacterController::RemoveAction(uint32 action)
{
    FUNCTION_MARKER(0x80027AC0, "SLUS_005.78");
    uint32 object_offset = action_effects[action];
    if (object_offset != 0)
    {
        uint8 flags = r_u8(object_offset + 13);
        w_u8(object_offset + 125, 0);
        w_u8(object_offset + 13, flags | 2);
        action_effects[action] = 0;
    }
}

uint32 CharacterController::CancelCountedActions(uint32 incoming_flags)
{
    FUNCTION_MARKER(0x8005A480, "SLUS_005.78");
    sint32 index;
    if ((incoming_flags & 0x10) != 0)
    {
        for (index = 0; index < action13_count; ++index)
            RemoveAction(13);
        action13_count = 0;
    }
    if ((character_action_flags & 0x20) != 0)
    {
        for (index = 0; index < action14_count; ++index)
            RemoveAction(14);
        action14_count = 0;
    }
    incoming_flags = character_action_flags & 0xFFFFFFCF;
    character_action_flags = incoming_flags;
    return incoming_flags;
}

uint32 SpawnActionEffect(uint32 object_offset, uint32 character_offset, uint32 action)
{
    FUNCTION_MARKER(0x80027B1C, "SLUS_005.78");
    uint32 saved_stack, local, vector, quaternion, world, other_world, input, inverse, matrix_local, output;
    uint32 spawned, flags, value, x, y, z, a, b, c, parent, matrix, source, destination;
    uint32 owner, pass, passes, global_source, index, result;
    sint32 mode;
    result = action << 2;
    spawned = action_effect_templates[action];
    if (spawned == 0)
        return result;
    if ((r_u8(spawned + 13) & 0x80) != 0)
    {
        value = r_u8(spawned + 13);
        w_u32(spawned + 4, 0);
        w_u8(spawned + 13, value & 0x7F);
    }
    else
    {
        spawned = p2_func_8004002c(spawned);
        if (spawned == 0)
            return 0;
        w_u32(spawned + 28, r_u32(spawned + 28) & 0xFFFFFFFB);
    }
    flags = r_u32(spawned + 28);
    w_u16(spawned + 130, 0);
    w_u32(spawned + 112, 0);
    w_u32(spawned + 116, action);
    w_u32(spawned + 28, flags | 0x20000);
    saved_stack = p2_guest_stack_pointer;
    p2_guest_stack_pointer -= 144;
    local = p2_guest_stack_pointer;
    vector = local + 16;
    quaternion = local + 32;
    world = local + 48;
    other_world = local + 64;
    input = local + 80;
    inverse = local + 96;
    matrix_local = local + 112;
    output = local + 136;
    if (action == 10)
    {
        x = r_u32(object_offset + 40);
        y = r_u32(object_offset + 44);
        z = r_u32(object_offset + 48);
        w_u32(spawned + 16, x);
        w_u32(spawned + 20, y);
        w_u32(spawned + 24, z);
    }
    else
    {
        x = r_u32(object_offset + 60);
        y = r_u32(object_offset + 64);
        z = r_u32(object_offset + 68);
        w_u32(spawned + 16, x);
        w_u32(spawned + 20, y);
        w_u32(spawned + 24, z);
        if (character_offset != 0 && action != 2)
        {
            p2_func_8005927c();
            mode = (sint8)character_controller.motion.state_code;
            if (mode >= 10 && mode < 13)
                p2_func_80059acc();
            else if (mode == 20)
                p2_func_80059ac4();
        }
        switch (action)
        {
            case 0:
                x = r_u32(spawned + 20);
                value = r_u32(object_offset + 76);
                w_u32(spawned + 20, x + value);
                break;
            case 1:
                x = r_u32(spawned + 20);
                value = r_u32(object_offset + 76);
                w_u32(spawned + 20, x - value);
                break;
            case 2:
                w_u32(spawned + 112, character_offset);
                value = random_generator.Next() & 0x7FF;
                w_u32(spawned + 92, value - 1024);
                w_u32(spawned + 92, value >= 1024 ? value + 1024 : value - 3072);
                value = (random_generator.Next() & 0x3FF) + 2048;
                w_u32(spawned + 96, value);
                value = random_generator.Next() & 0x7FF;
                w_u32(spawned + 100, value - 1024);
                w_u32(spawned + 100, value >= 1024 ? value + 1024 : value - 3072);
                break;
            case 5:
            case 8:
                break;
            case 9:
            case 12:
                x = r_u32(spawned + 16);
                y = r_u32(spawned + 20);
                z = r_u32(spawned + 24);
                w_u32(spawned + 60, x);
                w_u32(spawned + 64, y);
                w_u32(spawned + 68, z);
                w_u32(spawned + 72, 12288);
                w_u16(spawned + 130, 0);
                w_u16(spawned + 136, 0);
                if (action == 12)
                {
                    character_controller.action_effects[action] = spawned;
                    w_u32(spawned + 28, r_u32(spawned + 28) | 0x8000);
                }
                break;
            case 11:
                mode = r_s8(0x8008A128);
                w_u32(spawned + 100, 0);
                if (mode == 1)
                {
                    owner = p2_func_800536f0(0);
                    flags = r_u32(owner + 28);
                    mode = r_s16(owner + 136);
                    p2_func_80035058(quaternion, vector, owner, mode, flags & 0x10);
                    p2_func_80040384(spawned, vector, 4096);
                }
                else if (mode == 2)
                {
                    source = r_u32(0x8008A0C8);
                    p2_sdk_8007ec64(spawned + 28, source + 20);
                    source = r_u32(0x8008A0C8);
                    x = r_u32(source + 20);
                    a = r_u32(source - 12);
                    y = r_u32(source + 24);
                    b = r_u32(source - 8);
                    z = r_u32(source + 28);
                    c = r_u32(source - 4);
                    w_u32(source + 20, x + a);
                    w_u32(source + 24, y + b);
                    w_u32(source + 28, z + c);
                    p2_func_80064358(spawned);
                }
                break;
            case 13:
            case 14:
                owner = p2_func_800536f0(0);
                passes = action == 13 ? 2 : 1;
                for (pass = 0; pass < passes; ++pass)
                {
                    global_source = action == 14 || pass != 0;
                    source = global_source != 0 ? character_action_target_offset : owner;
                    destination = pass == 0 ? world : other_world;
                    parent = r_u32(source + 132);
                    if (parent != 0 && r_u32(parent + 40) != 0)
                    {
                        x = r_u32(source + 16);
                        y = r_u32(source + 20);
                        z = r_u32(source + 24);
                        w_u32(input, x);
                        w_u32(input + 4, y);
                        w_u32(input + 8, z);
                        PushMatrix();
                        if (global_source != 0)
                            source = character_action_target_offset;
                        matrix = r_u32(r_u32(source + 132) + 40);
                        x = r_u32(matrix);
                        y = r_u32(matrix + 4);
                        xport_gte_write_control(0, x);
                        xport_gte_write_control(1, y);
                        x = r_u32(matrix + 8);
                        y = r_u32(matrix + 12);
                        z = r_u32(matrix + 16);
                        xport_gte_write_control(2, x);
                        xport_gte_write_control(3, y);
                        xport_gte_write_control(4, z);
                        p2_sdk_8007ec64(input, destination);
                        PopMatrix();
                        if (global_source != 0)
                            source = character_action_target_offset;
                        for (index = 0; index < 3; ++index)
                        {
                            matrix = r_u32(r_u32(source + 132) + 40);
                            value = r_u32(destination + index * 4);
                            x = r_u32(matrix + 20 + index * 4);
                            w_u32(destination + index * 4, value + x);
                        }
                    }
                    else
                    {
                        if (global_source != 0)
                            source = character_action_target_offset;
                        x = r_u32(source + 16);
                        y = r_u32(source + 20);
                        z = r_u32(source + 24);
                        w_u32(destination, x);
                        w_u32(destination + 4, y);
                        w_u32(destination + 8, z);
                    }
                    if (pass == 0)
                    {
                        parent = r_u32(spawned + 132);
                        matrix = parent != 0 ? r_u32(parent + 40) : 0;
                        if (matrix != 0)
                        {
                            for (index = 0; index < 3; ++index)
                            {
                                if (index != 0)
                                    matrix = r_u32(r_u32(spawned + 132) + 40);
                                value = r_u32(world + index * 4);
                                x = r_u32(matrix + 20 + index * 4);
                                w_u32(inverse + index * 4, value - x);
                            }
                            for (index = 0; index < 9; ++index)
                            {
                                matrix = r_u32(r_u32(spawned + 132) + 40);
                                value = r_u16(matrix + (index % 3) * 6 + (index / 3) * 2);
                                w_u16(matrix_local + index * 2, value);
                            }
                            PushMatrix();
                            x = r_u32(matrix_local);
                            y = r_u32(matrix_local + 4);
                            xport_gte_write_control(0, x);
                            xport_gte_write_control(1, y);
                            x = r_u32(matrix_local + 8);
                            y = r_u32(matrix_local + 12);
                            z = r_u16(matrix_local + 16);
                            xport_gte_write_control(2, x);
                            xport_gte_write_control(3, y);
                            xport_gte_write_control(4, z);
                            p2_sdk_8007ec64(inverse, spawned + 16);
                            PopMatrix();
                        }
                        else
                        {
                            x = r_u32(world);
                            y = r_u32(world + 4);
                            z = r_u32(world + 8);
                            w_u32(spawned + 16, x);
                            w_u32(spawned + 20, y);
                            w_u32(spawned + 24, z);
                        }
                        w_u32(spawned + 20, r_u32(spawned + 20) + 0x4000);
                    }
                }
                value = ActionDistance(action == 13 ? other_world : world, spawned + 16);
                w_u32(spawned + 92, value);
                value = action == 13 ? character_controller.action13_scale : character_controller.action14_scale;
                AdvanceActionEffect(spawned, owner, 0x800ABD68, action, value << 1, output);
                x = r_u32(spawned + 16);
                y = r_u32(spawned + 20);
                z = r_u32(spawned + 24);
                w_u32(spawned + 60, x);
                w_u32(spawned + 64, y);
                w_u32(spawned + 68, z);
                w_u32(spawned + 72, 12288);
                w_u16(spawned + 130, 0);
                w_u16(spawned + 136, 0);
                character_controller.action_effects[action] = spawned;
                flags = r_u32(spawned + 28);
                if (action == 14)
                    w_u32(spawned + 100, 0);
                w_u32(spawned + 28, flags | 0x8000);
                if (action == 13)
                    character_controller.action13_count += 1;
                else
                    character_controller.action14_count += 1;
                if (action == 13)
                    w_u32(spawned + 100, 0);
                break;
            default:
                character_controller.action_effects[action] = spawned;
                w_u32(spawned + 28, r_u32(spawned + 28) | 0x8000);
                break;
        }
    }
    w_u8(spawned + 125, 26);
    source = r_u32(0x8008A4E8);
    result = r_u32(source + 108);
    w_u32(spawned + 4, result);
    w_u32(source + 108, spawned);
    p2_guest_stack_pointer = saved_stack;
    return result;
}

uint32 CharacterController::ClearInteractionActions()
{
    FUNCTION_MARKER(0x8005A540, "SLUS_005.78");
    uint32 flags;
    sint32 index;
    if (character_action_flags != 0)
    {
        uint32 active = character_action_target_offset;
        if (active != 0)
        {
            uint32 track = r_u32(r_u32(active + 132) + 44);
            w_u8(track + 38, r_u8(track + 38) & 0xF7);
        }
    }
    if ((character_action_flags & 8) != 0)
        RemoveAction(12);
    flags = character_action_flags;
    character_action_flags = flags & 0xFFFFEFF7;
    if ((flags & 16) != 0)
    {
        if (action13_count > 0)
        {
            index = 0;
            do
            {
                RemoveAction(13);
                ++index;
            } while (index < action13_count);
        }
        action13_count = 0;
    }
    if ((character_action_flags & 32) != 0)
    {
        if (action14_count > 0)
        {
            index = 0;
            do
            {
                RemoveAction(14);
                ++index;
            } while (index < action14_count);
        }
        action14_count = 0;
    }
    flags = character_action_flags & ~48u;
    character_action_flags = flags;
    return flags;
}

uint32 CharacterController::EndInteraction()
{
    FUNCTION_MARKER(0x8005A668, "SLUS_005.78");
    uint32 flags;
    sint32 index;
    if (character_action_flags != 0)
    {
        uint32 active = character_action_target_offset;
        if (active != 0)
        {
            uint32 track = r_u32(r_u32(active + 132) + 44);
            w_u8(track + 38, r_u8(track + 38) & 0xF7);
        }
    }
    if ((character_action_flags & 8) != 0)
        RemoveAction(12);
    flags = character_action_flags;
    character_action_flags = flags & 0xFFFFEFF7;
    if ((flags & 16) != 0)
    {
        if (action13_count > 0)
        {
            index = 0;
            do
            {
                RemoveAction(13);
                ++index;
            } while (index < action13_count);
        }
        action13_count = 0;
    }
    if ((character_action_flags & 32) != 0)
    {
        if (action14_count > 0)
        {
            index = 0;
            do
            {
                RemoveAction(14);
                ++index;
            } while (index < action14_count);
        }
        action14_count = 0;
    }
    flags = secondary_flags;
    character_action_flags = 0;
    flags &= ~256u;
    secondary_flags = flags;
    return flags;
}

uint32 BeginCharacterInteraction(uint32 object_offset, uint32 character_offset)
{
    FUNCTION_MARKER(0x8005A440, "SLUS_005.78");
    CharacterController *character = &character_controller;
    uint32 flags = character->secondary_flags;
    character_action_flags = 4;
    character_controller.timers.interaction_ticks = 900;
    character_controller.action_ticks = 0;
    character_controller.phase_ticks = 0;
    character_controller.interaction_phase = 0;
    character_action_owner_offset = object_offset;
    character_action_target_offset = 0;
    character_controller.action13_count = 0;
    character_controller.action14_count = 0;
    character->secondary_flags = flags | 0x100;
    return 900;
}

uint32 UpdateCharacterInteraction(uint32 object_offset, uint32 character_offset)
{
    FUNCTION_MARKER(0x8005A7F4, "SLUS_005.78");
    uint32 flags, result;
    uint32 cleanup_flags, cleanup_target, cleanup_data;
    sint32 action_index;
    CharacterController *character = &character_controller;
    if (r_s8(0x8008A1A8) == 0)
        character_controller.timers.interaction_ticks = character_controller.timers.interaction_ticks - 1u;
    character_controller.phase_ticks = (uint8)character_controller.phase_ticks + 1u;
    flags = character_action_flags;
    if ((flags & 0x800) != 0)
    {
        character_action_flags = flags & ~0x800u;
        character_controller.phase_ticks = 0;
        character_controller.interaction_phase = 0;
        cleanup_flags = character_action_flags;
        if (cleanup_flags != 0)
        {
            cleanup_target = character_action_target_offset;
            if (cleanup_target != 0)
            {
                cleanup_data = r_u32(r_u32(cleanup_target + 132) + 44);
                w_u8(cleanup_data + 38, r_u8(cleanup_data + 38) & 0xF7);
            }
            cleanup_flags = character_action_flags;
        }
        if ((cleanup_flags & 8) != 0)
            character_controller.RemoveAction(12);
        cleanup_flags = character_action_flags;
        character_action_flags = cleanup_flags & 0xFFFFEFF7;
        if ((cleanup_flags & 16) != 0)
        {
            action_index = 0;
            while (action_index < character_controller.action13_count)
            {
                character_controller.RemoveAction(13);
                ++action_index;
            }
            character_controller.action13_count = 0;
        }
        if ((character_action_flags & 32) != 0)
        {
            action_index = 0;
            while (action_index < character_controller.action14_count)
            {
                character_controller.RemoveAction(14);
                ++action_index;
            }
            character_controller.action14_count = 0;
        }
        character_action_flags = character_action_flags & ~48u;
    }
    switch (character_controller.interaction_phase)
    {
        case 0:
            if ((r_u32(object_offset + 28) & 0x1000400) == 0x1000000)
            {
                character_controller.phase_ticks = 0;
                character_controller.interaction_phase = 1;
            }
            break;
        case 1:
        {
            sint32 tick = character_controller.phase_ticks;
            if (tick == 1)
            {
                p2_func_80044ffc(16, 3);
                SpawnActionEffect(object_offset, character_offset, 11);
            }
            else if (tick == 4)
            {
                flags = character_action_flags;
                character_action_target_offset = 0;
                character_action_flags = flags | 0x200;
            }
            else if (tick == 5 || tick == 6)
            {
                uint32 target = character_action_target_offset;
                uint32 data, config, saved_stack, world, input, other, index, definition, matrix;
                uint32 position[3];
                if (tick == 5 && target == 0)
                {
                    character_action_flags = character_action_flags | 0x200;
                    break;
                }
                character_controller.phase_ticks = 0;
                target = character_action_target_offset;
                if (target == 0)
                {
                    character_controller.interaction_phase = 0;
                    break;
                }
                data = r_u32(r_u32(target + 132) + 44);
                flags = character_action_flags;
                config = r_u32(data + 72);
                character_action_flags = flags & ~0x200u;
                saved_stack = p2_guest_stack_pointer;
                p2_guest_stack_pointer -= 48;
                world = p2_guest_stack_pointer;
                other = world + 16;
                input = world + 32;
                definition = r_u32(target + 132);
                if (definition != 0 && r_u32(definition + 40) != 0)
                {
                    for (index = 0; index < 3; ++index)
                        position[index] = r_u32(target + 16 + index * 4);
                    for (index = 0; index < 3; ++index)
                        w_u32(input + index * 4, position[index]);
                    PushMatrix();
                    target = character_action_target_offset;
                    matrix = r_u32(r_u32(target + 132) + 40);
                    for (index = 0; index < 5; ++index)
                        xport_gte_write_control(index, r_u32(matrix + index * 4));
                    p2_sdk_8007ec64(input, world);
                    PopMatrix();
                    target = character_action_target_offset;
                    for (index = 0; index < 3; ++index)
                    {
                        matrix = r_u32(r_u32(target + 132) + 40);
                        w_u32(world + index * 4, r_u32(world + index * 4) + r_u32(matrix + 20 + index * 4));
                    }
                }
                else
                {
                    target = character_action_target_offset;
                    for (index = 0; index < 3; ++index)
                        position[index] = r_u32(target + 16 + index * 4);
                    for (index = 0; index < 3; ++index)
                        w_u32(world + index * 4, position[index]);
                }
                w_u32(data + 84, r_u32(world + 4));
                if (r_s16(config + 654) != -1 || r_s16(config + 646) != 0)
                {
                    definition = r_u32(object_offset + 132);
                    if (definition != 0 && r_u32(definition + 40) != 0)
                    {
                        for (index = 0; index < 3; ++index)
                            position[index] = r_u32(object_offset + 16 + index * 4);
                        for (index = 0; index < 3; ++index)
                            w_u32(input + index * 4, position[index]);
                        PushMatrix();
                        matrix = r_u32(r_u32(object_offset + 132) + 40);
                        for (index = 0; index < 5; ++index)
                            xport_gte_write_control(index, r_u32(matrix + index * 4));
                        p2_sdk_8007ec64(input, other);
                        PopMatrix();
                        for (index = 0; index < 3; ++index)
                        {
                            matrix = r_u32(r_u32(object_offset + 132) + 40);
                            w_u32(other + index * 4, r_u32(other + index * 4) + r_u32(matrix + 20 + index * 4));
                        }
                    }
                    else
                    {
                        for (index = 0; index < 3; ++index)
                            position[index] = r_u32(object_offset + 16 + index * 4);
                        for (index = 0; index < 3; ++index)
                            w_u32(other + index * 4, position[index]);
                    }
                    result = (uint32)Atan2Angle((sint32)(r_u32(world + 8) - r_u32(other + 8)), (sint32)(r_u32(world) - r_u32(other)));
                }
                else
                    result = r_u16(character_action_target_offset + 148);
                w_u16(0x8008A484, result);
                character_action_flags = character_action_flags | 8;
                w_u8(data + 38, r_u8(data + 38) | 8);
                SpawnActionEffect(character_action_target_offset, character_offset, 12);
                if ((character_action_flags & 0x1000) != 0 || r_s16(config + 646) == 0)
                    character_controller.interaction_phase = 2;
                else
                {
                    character_controller.interaction_phase = 4;
                    character_controller.action13_scale = 4096;
                    character_controller.action_pending = 0;
                    character_controller.action_delay = 0;
                    character_controller.action_mode = 0;
                    character_controller.action_latched = 0;
                    character_controller.action13_count = 0;
                    character_controller.action14_count = 0;
                    character_controller.action13_limit_reached = 0;
                    character_controller.action14_limit_reached = 0;
                    character_controller.action_completion = 0;
                    character_controller.action13_endpoint = (uint32)-1;
                    character_controller.action14_endpoint = (uint32)-1;
                    character_controller.action_shared_limit = (uint32)-1;
                    character_controller.action_limit_step = (sint8)r_u16(config + 646);
                    character_controller.action_delay_bias = (sint8)r_u16(config + 648);
                }
                p2_guest_stack_pointer = saved_stack;
            }
            break;
        }
        case 2:
            if (r_s16(character_action_target_offset + 128) <= 0)
            {
                if ((character_action_flags & 8) != 0)
                    character_controller.RemoveAction(12);
                character_controller.interaction_phase = 0;
                cleanup_flags = character_action_flags;
                if (cleanup_flags != 0)
                {
                    cleanup_target = character_action_target_offset;
                    if (cleanup_target != 0)
                    {
                        cleanup_data = r_u32(r_u32(cleanup_target + 132) + 44);
                        w_u8(cleanup_data + 38, r_u8(cleanup_data + 38) & 0xF7);
                    }
                    cleanup_flags = character_action_flags;
                }
                if ((cleanup_flags & 8) != 0)
                    character_controller.RemoveAction(12);
                cleanup_flags = character_action_flags;
                character_action_flags = cleanup_flags & 0xFFFFEFF7;
                if ((cleanup_flags & 16) != 0)
                {
                    action_index = 0;
                    while (action_index < character_controller.action13_count)
                    {
                        character_controller.RemoveAction(13);
                        ++action_index;
                    }
                    character_controller.action13_count = 0;
                }
                if ((character_action_flags & 32) != 0)
                {
                    action_index = 0;
                    while (action_index < character_controller.action14_count)
                    {
                        character_controller.RemoveAction(14);
                        ++action_index;
                    }
                    character_controller.action14_count = 0;
                }
                character_action_flags = character_action_flags & ~48u;
            }
            else
            {
                if ((r_u32(object_offset + 28) & 0x1000400) == 0x1000000)
                {
                    p2_func_80044ffc(16, 3);
                    SpawnActionEffect(object_offset, character_offset, 11);
                    character_controller.phase_ticks = 0;
                    SpawnActionEffect(object_offset, character_offset, 13);
                    character_action_flags = character_action_flags | 16;
                }
                if (character_controller.action_completion == 1)
                {
                    uint32 target, data;
                    character_controller.action_completion = 0;
                    target = character_action_target_offset;
                    if (r_s16(target + 128) != 0)
                    {
                        data = r_u32(r_u32(target + 132) + 44);
                        flags = r_u32(data + 44);
                        result = (uint32)(sint32)r_s8(0x8008A1A9);
                        w_u32(data + 44, (flags & 0xFFFF01FF) | 0x208);
                        if (result != 0)
                        {
                            w_u32(target + 20, r_u32(target + 20) + 3072u);
                            w_u8(data + 38, r_u8(data + 38) | 16);
                        }
                    }
                }
            }
            break;
        case 4:
            if (r_s16(character_action_target_offset + 128) <= 0)
            {
                character_controller.phase_ticks = 0;
                character_controller.interaction_phase = 0;
                cleanup_flags = character_action_flags;
                if (cleanup_flags != 0)
                {
                    cleanup_target = character_action_target_offset;
                    if (cleanup_target != 0)
                    {
                        cleanup_data = r_u32(r_u32(cleanup_target + 132) + 44);
                        w_u8(cleanup_data + 38, r_u8(cleanup_data + 38) & 0xF7);
                    }
                    cleanup_flags = character_action_flags;
                }
                if ((cleanup_flags & 8) != 0)
                    character_controller.RemoveAction(12);
                cleanup_flags = character_action_flags;
                character_action_flags = cleanup_flags & 0xFFFFEFF7;
                if ((cleanup_flags & 16) != 0)
                {
                    action_index = 0;
                    while (action_index < character_controller.action13_count)
                    {
                        character_controller.RemoveAction(13);
                        ++action_index;
                    }
                    character_controller.action13_count = 0;
                }
                if ((character_action_flags & 32) != 0)
                {
                    action_index = 0;
                    while (action_index < character_controller.action14_count)
                    {
                        character_controller.RemoveAction(14);
                        ++action_index;
                    }
                    character_controller.action14_count = 0;
                }
                character_action_flags = character_action_flags & ~48u;
                break;
            }
            if ((r_u32(object_offset + 28) & 0x1000400) == 0x1000000)
            {
                p2_func_80044ffc(16, 3);
                SpawnActionEffect(object_offset, character_offset, 11);
                character_controller.phase_ticks = 0;
                SpawnActionEffect(object_offset, character_offset, 13);
                character_action_flags = character_action_flags | 16;
                character_controller.action_mode = 1;
            }
            if (character_controller.action_pending != 0)
            {
                sint32 table_index, delay;
                character_controller.action14_scale = character_controller.action_mode == 1 ? 4096 : 2048;
                p2_func_80044ffc(16, 0);
                SpawnActionEffect(object_offset, character_offset, 14);
                character_action_flags = character_action_flags | 32;
                table_index = character_controller.action_ticks % 16;
                character_controller.action_pending = 0;
                delay = r_s32(0x80083BAC + (uint32)(table_index * 4)) / 2048;
                character_controller.action_delay = delay;
                delay += (uint8)character_controller.action_delay_bias;
                character_controller.action_delay = (sint32)(sint8)delay * 4;
            }
            else
            {
                if (character_controller.action_latched != 0 && character_controller.action_mode != 1)
                    goto interaction_result;
                result = (uint8)character_controller.action_delay - 1u;
                character_controller.action_delay = result;
                if ((sint8)result > 0 || r_s8(0x8008A1AA) != 0 || (character_controller.action_latched != 0 && character_controller.action_mode != 1))
                    goto interaction_result;
                {
                    uint32 pending = (uint8)character_controller.action_pending;
                    sint32 table_index = character_controller.action_ticks % 16;
                    sint32 delay;
                    character_controller.action_latched = 1;
                    character_controller.action_pending = pending + 1u;
                    delay = r_s32(0x80083BAC + (uint32)(table_index * 4)) / 2048;
                    result = (uint8)character_controller.action_delay_bias;
                    character_controller.action_delay = delay;
                    character_controller.action_delay = (uint32)delay + result;
                }
            }
        interaction_result:
            if (character_controller.action_completion == 1)
            {
                character_controller.action_completion = 0;
                if ((character_action_flags & 16) != 0)
                {
                    action_index = 0;
                    while (action_index < character_controller.action13_count)
                    {
                        character_controller.RemoveAction(13);
                        ++action_index;
                    }
                    character_controller.action13_count = 0;
                }
                if ((character_action_flags & 32) != 0)
                {
                    action_index = 0;
                    while (action_index < character_controller.action14_count)
                    {
                        character_controller.RemoveAction(14);
                        ++action_index;
                    }
                    character_controller.action14_count = 0;
                }
                character_action_flags = (character_action_flags & ~48u) | 0x3000;
                character_controller.interaction_phase = 5;
                character_controller.action_ticks = 0;
            }
            else if (character_controller.action_completion == 2)
            {
                character_controller.action_completion = 0;
                flags = character_action_flags;
                w_u32(character_offset + 180, 0);
                w_u32(character_offset + 668, 0);
                character_action_flags = flags | 0x2100;
            }
            break;
        case 5:
            result = (uint8)character_controller.action_ticks + 1u;
            character_controller.action_ticks = result;
            if ((sint8)result >= 3)
            {
                character_action_flags = character_action_flags & ~0x2000u;
                character_controller.action_ticks = 0;
                character_controller.interaction_phase = 2;
            }
            break;
        default:
            character_controller.phase_ticks = 0;
            character_controller.action_ticks = 0;
            character_controller.interaction_phase = 0;
            break;
    }
    if (character_controller.timers.interaction_ticks == 0)
    {
        cleanup_flags = character_action_flags;
        if (cleanup_flags != 0)
        {
            cleanup_target = character_action_target_offset;
            if (cleanup_target != 0)
            {
                cleanup_data = r_u32(r_u32(cleanup_target + 132) + 44);
                w_u8(cleanup_data + 38, r_u8(cleanup_data + 38) & 0xF7);
            }
            cleanup_flags = character_action_flags;
        }
        if ((cleanup_flags & 8) != 0)
            character_controller.RemoveAction(12);
        cleanup_flags = character_action_flags;
        character_action_flags = cleanup_flags & 0xFFFFEFF7;
        if ((cleanup_flags & 16) != 0)
        {
            action_index = 0;
            while (action_index < character_controller.action13_count)
            {
                character_controller.RemoveAction(13);
                ++action_index;
            }
            character_controller.action13_count = 0;
        }
        if ((character_action_flags & 32) != 0)
        {
            action_index = 0;
            while (action_index < character_controller.action14_count)
            {
                character_controller.RemoveAction(14);
                ++action_index;
            }
            character_controller.action14_count = 0;
        }
        flags = character->secondary_flags;
        character_action_flags = 0;
        character->secondary_flags = flags & ~0x100u;
    }
    flags = r_u32(object_offset + 28);
    result = (uint8)character_controller.phase_ticks;
    w_u8(0x8008A50D, result);
    w_u32(object_offset + 28, flags & ~0x1000000u);
    return result;
}

uint32 AdvanceActionEffect(uint32 object_offset, uint32 owner_offset, uint32 character_offset, uint32 action, uint32 distance, uint32 output_offset)
{
    FUNCTION_MARKER(0x80028D60, "SLUS_005.78");
    uint32 saved_stack = p2_guest_stack_pointer;
    uint32 world, input, relative, transpose;
    uint32 source_index, index, definition, matrix;
    uint32 difference[3], absolute[3], interpolated[3];
    uint32 length, progress, coefficient, initialized = 0, result;
    sint32 reciprocal;
    CharacterController *character;
    p2_guest_stack_pointer -= 112;
    world = p2_guest_stack_pointer;
    input = world + 48;
    relative = world + 64;
    transpose = world + 80;
    for (source_index = 0; source_index < 3; ++source_index)
    {
        uint32 source = source_index == 0 ? owner_offset : (source_index == 1 ? character_action_target_offset : object_offset);
        uint32 destination = world + source_index * 16;
        uint32 position[3];
        definition = r_u32(source + 132);
        if (definition != 0 && r_u32(definition + 40) != 0)
        {
            for (index = 0; index < 3; ++index)
                position[index] = r_u32(source + 16 + index * 4);
            for (index = 0; index < 3; ++index)
                w_u32(input + index * 4, position[index]);
            PushMatrix();
            if (source_index == 1)
                source = character_action_target_offset;
            matrix = r_u32(r_u32(source + 132) + 40);
            for (index = 0; index < 5; ++index)
                xport_gte_write_control(index, r_u32(matrix + index * 4));
            p2_sdk_8007ec64(input, destination);
            PopMatrix();
            if (source_index == 1)
                source = character_action_target_offset;
            for (index = 0; index < 3; ++index)
            {
                matrix = r_u32(r_u32(source + 132) + 40);
                w_u32(destination + index * 4, r_u32(destination + index * 4) + r_u32(matrix + 20 + index * 4));
            }
        }
        else
        {
            if (source_index == 1)
                source = character_action_target_offset;
            for (index = 0; index < 3; ++index)
                position[index] = r_u32(source + 16 + index * 4);
            for (index = 0; index < 3; ++index)
                w_u32(destination + index * 4, position[index]);
        }
    }
    w_u32(world + 36, r_u32(world + 36) - 16384);
    for (index = 0; index < 3; ++index)
    {
        difference[index] = r_u32(world + index * 4) - r_u32(world + 16 + index * 4);
        w_u32(relative + index * 4, difference[index]);
        absolute[index] = (sint32)difference[index] < 0 ? 0u - difference[index] : difference[index];
    }
    length = 2u * (absolute[0] + absolute[2]) + absolute[1];
    progress = r_u32(object_offset + 92);
    progress = action == 13 ? progress - distance : progress + distance;
    if (length == 0)
    {
        fprintf(stderr, "P2: Original division-by-zero trap in 80028D60\n");
        abort();
    }
    reciprocal = 0x40000000 / (sint32)length;
    w_u32(object_offset + 92, progress);
    coefficient = (uint32)(((sint64)(sint32)progress * reciprocal) >> 18);
    w_u32(0x1F80030C, coefficient);
    character = &character_controller;
    if (character_controller.interaction_phase == 4 && character_controller.action_mode == 1)
    {
        uint32 limit = character_controller.action_shared_limit;
        if (limit == (uint32)-1)
        {
            if (action == 13)
            {
                uint32 endpoint = character_controller.action14_endpoint;
                if ((sint32)progress < (sint32)endpoint && endpoint != (uint32)-1)
                {
                    initialized = 1;
                    character_controller.action_shared_limit = endpoint;
                    character_controller.action13_limit_reached = 1;
                    w_u32(object_offset + 100, 1);
                    progress = character_controller.action_shared_limit;
                    character_controller.action13_endpoint = progress;
                }
            }
            else
            {
                uint32 endpoint = character_controller.action13_endpoint;
                if ((sint32)endpoint < (sint32)progress && character_controller.action14_endpoint != (uint32)-1)
                {
                    initialized = 1;
                    character_controller.action_shared_limit = endpoint;
                    character_controller.action14_limit_reached = 1;
                    w_u32(object_offset + 100, 1);
                    progress = character_controller.action_shared_limit;
                    character_controller.action14_endpoint = progress;
                }
            }
        }
        else if (r_u32(object_offset + 100) == 1)
        {
            if (action == 13 ? (sint32)progress < (sint32)limit : (sint32)limit < (sint32)progress)
                progress = limit;
        }
        else if (action == 13)
        {
            if ((sint32)progress < (sint32)limit)
            {
                if (character_controller.action13_limit_reached != 0)
                {
                    result = (uint32)-2;
                    goto finished;
                }
                w_u32(object_offset + 100, 1);
                progress = character_controller.action_shared_limit;
                character_controller.action13_limit_reached = 1;
            }
        }
        else if ((sint32)limit < (sint32)progress)
        {
            if (character_controller.action14_limit_reached != 0)
            {
                result = (uint32)-2;
                goto finished;
            }
            w_u32(object_offset + 100, 1);
            progress = character_controller.action_shared_limit;
            character_controller.action14_limit_reached = 1;
        }
    }
    if ((sint32)progress < (sint32)length && (sint32)progress >= 0 && initialized == 0)
    {
        coefficient = (uint32)(((sint64)(sint32)progress * reciprocal) >> 18);
        w_u32(0x1F80030C, coefficient);
        for (index = 0; index < 3; ++index)
            interpolated[index] = (uint32)(((sint64)(sint32)coefficient * (sint32)difference[index]) >> 12);
        interpolated[1] += 16384;
        for (index = 0; index < 3; ++index)
            interpolated[index] += r_u32(world + 16 + index * 4);
        definition = r_u32(object_offset + 132);
        if (definition != 0 && r_u32(definition + 40) != 0)
        {
            for (index = 0; index < 3; ++index)
            {
                matrix = r_u32(r_u32(object_offset + 132) + 40);
                w_u32(relative + index * 4, interpolated[index] - r_u32(matrix + 20 + index * 4));
            }
            for (index = 0; index < 9; ++index)
            {
                uint32 source_offset = (index % 3) * 6 + (index / 3) * 2;
                matrix = r_u32(r_u32(object_offset + 132) + 40);
                w_u16(transpose + index * 2, r_u16(matrix + source_offset));
            }
            PushMatrix();
            for (index = 0; index < 5; ++index)
                xport_gte_write_control(index, r_u32(transpose + index * 4));
            p2_sdk_8007ec64(relative, object_offset + 16);
            PopMatrix();
        }
        else
        {
            for (index = 0; index < 3; ++index)
                w_u32(object_offset + 16 + index * 4, interpolated[index]);
        }
    }
    w_u32(output_offset, progress);
    if (action == 13)
        result = (sint32)progress < 0 ? (uint32)-1 : progress;
    else
        result = (sint32)progress < (sint32)length ? length - progress : (uint32)-1;
finished:
    p2_guest_stack_pointer = saved_stack;
    return result;
}

uint32 UpdateActionEffect(uint32 effect_offset, uint32 owner_offset, uint32 character_offset, uint32 action)
{
    FUNCTION_MARKER(0x800296A4, "SLUS_005.78");
    uint32 saved_stack = p2_guest_stack_pointer;
    uint32 output, result, distance;
    p2_guest_stack_pointer -= 8;
    output = p2_guest_stack_pointer;
    if ((character_action_flags & 0x2008) != 8)
        goto remove_object;
    if (character_controller.interaction_phase != 4)
    {
        AdvanceActionEffect(effect_offset, owner_offset, character_offset, action, 20480, output);
        result = 1;
        if (r_s32(output) >= 2048)
            goto finished;
        character_controller.action_completion = 1;
        goto remove_primary;
    }
    distance = AdvanceActionEffect(effect_offset, owner_offset, character_offset, action, action == 13 ? 20480 : 4096, output);
    if (action == 13)
    {
        uint32 prev = character_controller.action_shared_limit;
        uint32 check_status = prev != (uint32)-1;
        if (prev == (uint32)-1)
        {
            uint32 sample = r_u32(output);
            uint32 cur = character_controller.action_shared_limit;
            character_controller.action13_endpoint = sample;
            check_status = cur != prev;
        }
        if (check_status != 0 && distance == (uint32)-2)
        {
            uint32 flags;
            character_controller.action_shared_limit = character_controller.action_shared_limit - 8192;
            flags = r_u8(effect_offset + 13);
            w_u8(effect_offset + 125, 0);
            w_u8(effect_offset + 13, flags | 2);
            character_controller.action13_count = (uint8)character_controller.action13_count - 1u;
        }
        if ((sint32)distance < 2048 && (r_u8(effect_offset + 13) & 2) == 0)
            character_controller.action_completion = 1;
    }
    else
    {
        if (character_controller.action_shared_limit == (uint32)-1)
            character_controller.action14_endpoint = r_u32(output);
        if (character_controller.action_mode == 1 && distance == (uint32)-2 && r_u32(effect_offset + 100) != 1)
        {
            uint32 flags;
            sint32 displacement = (sint32)character_controller.action_limit_step * 4096;
            character_controller.action_shared_limit = character_controller.action_shared_limit + (uint32)displacement;
            flags = r_u8(effect_offset + 13);
            w_u8(effect_offset + 125, 0);
            w_u8(effect_offset + 13, flags | 2);
            character_controller.action14_count = (uint8)character_controller.action14_count - 1u;
        }
        if ((sint32)distance < 2048 && (r_u8(effect_offset + 13) & 2) == 0)
        {
            uint32 flags;
            character_controller.action_completion = 2;
            flags = r_u8(effect_offset + 13);
            w_u8(effect_offset + 125, 0);
            w_u8(effect_offset + 13, flags | 2);
            character_controller.action14_count = (uint8)character_controller.action14_count - 1u;
        }
    }
    result = distance + 2u < 2;
    if (result == 0)
        goto finished;
    result = r_u8(effect_offset + 13) & 2;
    if (result != 0)
        goto finished;
remove_object:
    if (action != 13)
    {
        uint32 flags = r_u8(effect_offset + 13);
        w_u8(effect_offset + 125, 0);
        w_u8(effect_offset + 13, flags | 2);
        result = (uint8)character_controller.action14_count - 1u;
        character_controller.action14_count = result;
        goto finished;
    }
remove_primary:
{
    uint32 flags = r_u8(effect_offset + 13);
    w_u8(effect_offset + 125, 0);
    w_u8(effect_offset + 13, flags | 2);
    result = (uint8)character_controller.action13_count - 1u;
    character_controller.action13_count = result;
}
finished:
    p2_guest_stack_pointer = saved_stack;
    return result;
}

uint32 PauseCharacterSound(void)
{
    FUNCTION_MARKER(0x800504E4, "SLUS_005.78");
    uint32 table, sound, result;
    table = r_u32(0x8008A408);
    character_sound_resume_flags = 0;
    sound = 0;
    if (table && r_s32(table) >= 7)
        sound = table + 52;
    if (sound_effects.IsPlaying(sound, character_controller.sound_voice) & 0xFF)
        character_sound_resume_flags = character_sound_resume_flags | 1;
    table = r_u32(0x8008A408);
    sound = 0;
    if (table && r_s32(table) >= 11)
        sound = table + 84;
    if (sound_effects.IsPlaying(sound, character_controller.sound_voice) & 0xFF)
        character_sound_resume_flags = character_sound_resume_flags | 2;
    result = character_sound_resume_flags;
    if (result)
    {
        p2_func_8006a970(character_controller.sound_voice);
        result = 0xFFFFFFFF;
        character_controller.sound_voice = (sint8)result;
    }
    return result;
}

void ResumeCharacterSound(void)
{
    FUNCTION_MARKER(0x800505E4, "SLUS_005.78");
    uint32 flags = character_sound_resume_flags;
    uint32 sound = 0;
    uint32 sounds;
    sint32 voice;
    if ((flags & 1) != 0)
    {
        sounds = r_u32(0x8008A408);
        if (sounds != 0 && r_s32(sounds) >= 7)
            sound = sounds + 52;
    }
    else
    {
        if ((flags & 2) == 0)
            return;
        sounds = r_u32(0x8008A408);
        if (sounds != 0 && r_s32(sounds) >= 11)
            sound = sounds + 84;
    }
    voice = sound_effects.SelectVoice((uint32)-1, sound, 1);
    character_controller.sound_voice = (sint8)voice;
    voice = (sint8)voice;
    if (voice >= 0)
    {
        uint16 pitch = r_u16(sound + 6);
        uint16 override_pitch = r_u16(sound + 4);
        sint16 volume = (sint16)(sound_settings.effects_volume);
        p2_func_8005e470(sound, (uint16)voice, 0, pitch, override_pitch, volume, 0);
    }
}
