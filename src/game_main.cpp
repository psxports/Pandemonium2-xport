#include "game.h"
#include "psx.h"
#include "psx_gpu.h"
#include <stdlib.h>
#include <string.h>

/* Unresolved until the original PsyQ graph type storage is audited */
const uint32 xport_gpu_graph_type_address = 0u;

/* Unresolved CD bindings until original storage is audited */
const uint32 xport_cd_sync_callback_address = 0u;
const uint32 xport_cd_ready_callback_address = 0u;
const uint32 xport_cd_status_address = 0u;
const uint32 xport_cd_setloc_table_address = 0u;

/* Reviewed PS-30E BIOS Exec links the executable entry to BFC03D60 */
uint32 p2_boot_return_address = 0xBFC03D60;

/* Preserve the recovered guest bootstrap ABI carriers */
uint32 p2_guest_stack_pointer;
uint32 p2_guest_frame_pointer;
uint32 p2_guest_global_pointer;
uint32 p2_guest_return_address;

/* Forward the reviewed startup inputs to the saved native main */
extern void p2_main_8006792c(void);

void p2_entry_8001127c(uint32 boot_return_address)
{
    uint32 address = 0x8008A2D0;
    uint32 stack_top;
    uint32 heap_bytes;

    FUNCTION_MARKER(0x8001127C, "SLUS_005.78");

    do
    {
        w_u32(address, 0);
        address += 4;
    } while (address < 0x800BA520);

    stack_top = r_u32(0x8001132C);
    p2_guest_stack_pointer = stack_top | 0x80000000;
    heap_bytes = stack_top - r_u32(0x80088418) - 0xBA520;
    w_u32(0x8008A2D0, boot_return_address);
    p2_guest_global_pointer = 0x8008A0C8;
    p2_guest_frame_pointer = p2_guest_stack_pointer;
    p2_guest_return_address = 0x80011310;
    psx_bios_init_user_heap(0x800BA524, heap_bytes);
    p2_guest_return_address = r_u32(0x8008A2D0);
    p2_guest_return_address = 0x80011324;
    p2_main_8006792c();
    trap(1);
}

void xport_main(void)
{
    uint8 header[2048];
    FILE *image = xport_fopen("DATA/SLUS_005.78", "rb");
    sint32 loaded = 0;
    /* The game paces frames with root-counter events rather than VSync */
    gpu_set_frame_skip(0);
    if (image != NULL)
    {
        if (xport_fread(header, 1, sizeof(header), image) == sizeof(header) && memcmp(header, "PS-X EXE", 8) == 0 && xport_load_le32(header + 16) == 0x8001127C && xport_load_le32(header + 24) == 0x80010000 && xport_load_le32(header + 28) == 0xAB000)
            loaded = xport_fread(psx_addr(0x80010000, 0xAB000), 1, 0xAB000, image) == 0xAB000;
        if (fclose(image) != 0)
            loaded = 0;
    }
    if (!loaded)
    {
        fprintf(stderr, "P2: Cannot load reviewed executable data from DATA/SLUS_005.78\n");
        abort();
    }
    random_generator.seed = OriginalDefaults::RandomSeed;
    memset(&frame_clock, 0, sizeof(frame_clock));
    memset(&render_view, 0, sizeof(render_view));
    memset(&render_commands, 0, sizeof(render_commands));
    game_flow.transition_flags = OriginalDefaults::GameTransitionFlags;
    game_flow.mode_flags = OriginalDefaults::GameModeFlags;
    player_interaction.flags = OriginalDefaults::InteractionFlags;
    player_interaction.target_offset = OriginalDefaults::InteractionTarget;
    memset(&resource_archive, 0, sizeof(resource_archive));
    memset(&object_operations, 0, sizeof(object_operations));
    character_action_flags = OriginalDefaults::CharacterActionFlags;
    character_action_owner_offset = OriginalDefaults::CharacterActionOwner;
    character_action_target_offset = OriginalDefaults::CharacterActionTarget;
    character_sound_resume_flags = OriginalDefaults::CharacterSoundResumeFlags;
    memset(&character_controller, 0, sizeof(character_controller));
    character_prev_contacts = 0;
    character_cur_contacts = 0;
    contact_free_head = 0;
    memset(action_effect_templates, 0, sizeof(action_effect_templates));
    collision_contact.selection = OriginalDefaults::ContactSelection;
    collision_contact.fraction = OriginalDefaults::ContactFraction;
    collision_contact.other_selection = OriginalDefaults::ContactOtherSelection;
    collision_contact.flags = OriginalDefaults::ContactFlags;
    memset(&sound_effects, 0, sizeof(sound_effects));
    for (unsigned int voice = 0; voice < 8; ++voice)
        sound_effects.voice_order[voice] = OriginalDefaults::SoundVoiceOrder[voice];
    sound_settings.effects_volume = OriginalDefaults::EffectsVolume;
    sound_settings.music_volume = OriginalDefaults::MusicVolume;
    memset(&hud, 0, sizeof(hud));
    menu_state = MenuState();
    menu_state.highlight_step = OriginalDefaults::MenuHighlightStep;
    menu_state.password.reset_pending = OriginalDefaults::PasswordResetPending != 0;
    control_bindings.active_scheme = &OriginalDefaults::ControlSchemes[0];
    control_bindings.selected_index = 0;
    gameplay_input = GameplayInput();
    memset(resource_groups, 0, sizeof(resource_groups));
    memset(&scene_loader, 0, sizeof(scene_loader));
    memset(&level_loader, 0, sizeof(level_loader));
    memset(scene_streams, 0, sizeof(scene_streams));
    memset(&cutscene, 0, sizeof(cutscene));
    memset(cutscene_cameras, 0, sizeof(cutscene_cameras));
    world_level_names = 0;
    interface_resources = 0;
    memset(&material_registry, 0, sizeof(material_registry));
    memset(&saved_material_registry, 0, sizeof(saved_material_registry));
    sprite_pool.first = 0;
    sprite_pool.free_head = 0;
    sprite_pool.active_head = 0;
    registered_object_head = 0;
    p2_entry_8001127c(p2_boot_return_address);
}
