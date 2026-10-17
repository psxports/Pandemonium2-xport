#ifndef P2_GAME_H
#define P2_GAME_H

#include "xport.h"
#include "psx.h"
#include "psx_spu.h"
#include "scene_path.h"
#include "animation_data.h"
#include "collision_contact.h"

#ifdef __cplusplus
#include "random_generator.h"
#include "frame_clock.h"
#include "render_view.h"
#include "render_commands.h"
#include "game_flow.h"
#include "player_interaction.h"
#include "resource_archive.h"
#include "resource_groups.h"
#include "scene_loading.h"
#include "scene_section.h"
#include "level_resource.h"
#include "cutscene.h"
#include "material_registry.h"
#include "material_record.h"
#include "material_resources.h"
#include "text_render.h"
#include "character_controller.h"
#include "character_models.h"
#include "character_vitals.h"
#include "character_timers.h"
#include "character_motion.h"
#include "object_operations.h"
#include "object_links.h"
#include "sound_effects.h"
#include "menu_state.h"
#include "password.h"
#include "control_bindings.h"
#include "gameplay_input.h"
#include "sound_settings.h"
#endif

#ifdef __cplusplus
extern "C"
{
#endif

uint32 p2_reset_graph_zero(void);
uint32 p2_reset_callback(void);

uint32 p2_func_8002fb94(uint32 object_offset);
uint32 p2_func_80044990(uint32 object_offset, uint32 speed_offset);
uint32 p2_func_80044cfc(uint32 object_offset, uint32 speed_offset);


uint32 p2_func_8006d30c(uint32 selection, uint32 mode);

uint32 p2_func_800668a0(uint32 first_ot_offset, uint32 second_ot_offset);

uint32 p2_func_8006ad80(uint16 handle);
uint32 p2_sdk_80074210(sint16 sequence);
uint32 PauseCharacterSound(void);
void ResumeCharacterSound(void);
uint32 p2_func_8006d8ec(void);
uint32 p2_func_8006d678(uint32 mode);
uint32 p2_func_80060f9c(void);
uint32 p2_func_8006ada4(uint16 handle);

uint32 p2_func_80022ae4(uint32 object_offset);

uint32 p2_func_8003ac94(uint32 path_offset, uint32 frame_offset, uint32 direction);
uint32 p2_func_8005a430(void);
uint32 p2_func_8005a3fc(void);

uint32 p2_func_8002995c(uint32 object_offset);
uint32 p2_func_80028720(uint32 object_offset, uint32 owner_offset);
uint32 p2_func_800289d4(uint32 object_offset, uint32 owner_offset);
uint32 UpdateActionEffect(uint32 effect_offset, uint32 owner_offset, uint32 character_offset, uint32 action);
uint32 p2_func_800596cc(uint32 owner_offset, uint32 object_offset);
uint32 p2_func_80059284(uint32 owner_offset, uint32 object_offset);

uint32 p2_func_80032ddc(uint32 object_offset, uint32 other_offset, CollisionContact *contact_data);

void p2_main_8006792c(void);
uint32 p2_func_8006637c(uint8 *destination, const uint8 *source, uint32 count);
uint32 p2_func_80064b18(uint32 path);
void p2_func_8008347c(void);
sint32 p2_func_8006a3dc(CdlFILE *result, const char *path);
sint32 p2_func_8007bc54(const uint8 *position);
uint32 p2_func_8008346c(void);

uint32 p2_func_8007e340(uint32 callback);

/* Forward declarations retain the original identity until native integration */
void p2_sdk_80011274(void);
uint32 p2_func_80025be8(void);
uint32 p2_func_800276dc(void);
void p2_func_8002a448(uint32 mode, uint32 object);
uint32 p2_func_8002dedc(void);
sint32 p2_func_8002dfc8(uint32 level, uint32 mode);
uint32 CaptureCharacterSettings(CharacterSettings *settings);
uint32 p2_func_800536f0(uint32 index);
uint32 p2_func_800558f0(void);
void p2_func_8005cd24(void);
void p2_func_80064d54(uint32 allocation);
void p2_func_80066430(void);
uint32 p2_func_8006657c(void);
uint32 ClearSceneOrderingTable(void);
uint32 p2_func_80045d0c(uint32 object_offset, uint32 character_offset);
uint32 p2_func_8005c4f4(uint32 object_offset);
uint32 p2_func_800425f4(uint32 allocation_offset);
uint32 p2_func_80030bd4(uint32 object_offset);
void p2_func_80034b38(void);
void p2_func_800441dc(void);
uint32 p2_func_8002d4c0(void);
uint32 p2_func_8002ab84(uint32 object_offset, uint32 position_offset);
uint32 p2_func_80065f78(sint32 intensity);
uint32 p2_func_80066070(sint32 intensity);
uint32 p2_func_800678a8(void);
uint32 p2_func_80066d60(void);
uint32 p2_func_80064dbc(sint32 top, uint32 height);
uint32 BeginDraw(void);
uint32 EndDraw(void);
uint32 p2_func_80066978(void);
uint32 p2_func_80066af0(uint32 mode);
uint32 p2_func_80066bdc(uint32 mode, uint32 extent, uint32 flags);
void p2_func_80066ce8(void);
uint32 LoadMenuResources(void);
sint32 p2_func_80067160(uint32 color);
uint32 p2_func_80067400(uint32 mode);
void p2_func_8006a528(void);
uint32 p2_func_8006a614(void);
void p2_func_8006ab74(void);
uint32 p2_func_8006ae90(uint16 handle);
void p2_func_8006af4c(void);
void p2_func_8006b000(void);
sint32 p2_func_8006daf4(uint32 level);
uint32 p2_func_8006e344(uint32 level, sint8 mode);
uint32 p2_func_80053bf0(void);
uint32 p2_func_80066168(void);
uint32 p2_func_8006e098(sint32 percent, uint32 flags, sint32 next_level, uint32 camera_offset, sint8 mode);
sint32 p2_func_8006e8d8(void);

void p2_func_8006aed0(void);
uint32 p2_func_8006f158(void);
uint32 p2_func_80071c5c(void);
uint32 p2_func_80077124(uint32 mode);
uint32 p2_func_8007b448(void);
void p2_func_80074750(void);
uint32 p2_func_8006645c(void);
void p2_func_80066c7c(void);
void p2_func_8006af88(void);
uint32 p2_func_80066ca8(void);
uint32 p2_func_8008371c(uint32 buffer0, uint32 size0, uint32 buffer1, uint32 size1);
uint32 p2_func_8008372c(void);
uint32 p2_func_8006ebfc(void);
uint32 p2_func_80064d28(uint32 bytes);
sint32 p2_func_800669f4(uint32 image);
uint32 p2_func_80066c40(void);
uint32 p2_func_80020ae8(void);
uint32 p2_func_80021538(void);
uint32 p2_func_8003fd28(void);
uint32 p2_func_8006aac8(void);
uint32 p2_func_80077150(void);
uint32 p2_func_80070e8c(const void *source, uint32 bytes);
uint32 p2_func_8006624c(uint32 image, uint32 index, uint32 flags);
uint32 p2_func_8006a530(uint32 asset, uint32 mode);
uint32 p2_func_8006aa88(uint32 source, uint32 bytes);
uint32 p2_func_80060eec(void);
uint32 p2_func_80066358(uint32 offset, uint8 value, uint32 count);
uint32 p2_func_8002a74c(void);
uint32 p2_func_80060df4(void);
uint32 p2_sdk_800829f8(uint32 bytes);
void p2_sdk_80082a08(uint32 address);
void p2_sdk_800833a4(void);
sint32 p2_sdk_8006ff38(uint32 attributes_offset);
uint32 p2_sdk_800759d4(uint32 table, sint16 sequences, sint16 separations);
uint32 p2_sdk_80079adc(uint8 voices);
uint32 p2_sdk_8006f178(uint32 mode);
uint32 p2_sdk_80071b74(void);
void p2_sdk_800744ec(uint32 mode);
uint32 p2_sdk_8007febc(uint32 callback);
uint32 p2_sdk_800834ac(uint32 counter, uint32 target, uint32 mode);
uint32 p2_sdk_80083584(uint32 counter);
sint32 p2_sdk_8007e16c(sint32 sectors, uint32 destination, uint32 mode);
sint32 p2_sdk_8007e274(sint32 mode, uint8 *result);
uint32 p2_sdk_80079f48(uint32 header, sint32 bank, uint32 address);
uint32 p2_sdk_8007a680(uint32 samples, sint16 bank);
uint32 p2_sdk_8007a740(uint32 mode);
uint32 p2_sdk_80071c8c(uint32 sequence, sint16 bank);
uint32 p2_sdk_8006fb68(const void *source, uint32 bytes);
uint32 p2_sdk_80070eec(uint32 address);
uint32 p2_func_800441e8(void);
uint32 p2_func_8002d4d0(void);
uint32 p2_func_80034b44(void);
uint32 p2_func_8006aa48(uint32 table);
uint32 p2_func_80030994(void);
uint32 p2_func_8002b058(uint8 kind);
uint32 p2_func_80068570(uint32 object, uint32 header);
uint32 p2_func_800683dc(uint32 node, uint32 base, uint32 header);
uint32 p2_sdk_80082a18(const char *format, ...);
uint32 InitializeContactPool(void);
uint32 p2_func_8006ac44(uint32 asset, uint32 mode, uint32 header, uint32 samples);
void p2_func_80066404(void);
uint32 p2_func_8002a3e0(uint32 mode, uint32 count);
uint32 p2_func_8006ae10(uint16 handle);
uint32 p2_func_800765b0(void);
uint32 p2_func_800765d0(void);
uint32 p2_func_800750c4(sint16 sequence, sint16 separation);
uint32 p2_sdk_80075d58(sint16 sequence, sint16 left, sint16 right);
uint32 p2_sdk_800743ec(sint16 sequence, sint16 track, uint8 mode, sint16 repeat);
uint32 p2_sdk_800742f4(sint16 sequence, sint16 track);
uint32 p2_sdk_80074398(sint16 sequence);
uint32 p2_sdk_80070a24(uint32 enabled);
uint32 p2_sdk_800763ec(uint32 enabled);
uint32 p2_sdk_80076354(sint16 left, sint16 right);
uint32 p2_sdk_80074264(sint32 sequence, sint32 mode, sint32 repeat);
uint32 p2_sdk_800764a0(uint32 serial);
uint32 p2_sdk_800750f0(sint16 sequence, sint16 separation);
uint32 p2_sdk_800753c4(sint16 sequence, sint16 separation);
uint32 p2_sdk_80073d58(sint16 sequence, sint16 separation);
uint32 p2_sdk_80073ce0(sint16 sequence, sint16 separation, uint8 program);
uint32 p2_sdk_80073718(sint16 sequence, sint16 separation);
uint32 p2_sdk_800737d0(sint16 sequence, sint16 separation, uint8 controller);
uint32 p2_sdk_80073a10(sint16 sequence, sint16 separation);
uint32 p2_sdk_80073bf0(sint16 sequence, sint16 separation, uint8 note, uint8 velocity);
uint32 p2_sdk_80075200(sint16 sequence, sint16 separation);
uint32 p2_sdk_800756a8(sint16 sequence, sint16 separation);
uint32 p2_sdk_8007201c(sint16 sequence, sint16 separation, uint8 value);
uint32 p2_sdk_80072444(sint16 sequence, sint16 separation, uint8 value);
uint32 p2_sdk_80072518(sint16 sequence, sint16 separation, uint8 value);
uint32 p2_sdk_800725e8(sint16 sequence, sint16 separation, uint8 value);
uint32 p2_sdk_80072698(sint16 sequence, sint16 separation, uint8 value);
uint32 p2_sdk_80072728(sint16 sequence, sint16 separation, uint8 value);
uint32 p2_sdk_80072860(sint16 sequence, sint16 separation, uint8 value);
uint32 p2_sdk_800729a4(sint16 sequence, sint16 separation, uint8 value);
uint32 p2_sdk_80072a1c(sint16 sequence, sint16 separation, uint8 value);
uint32 p2_sdk_80072a94(sint16 sequence, sint16 separation);
uint32 p2_sdk_800794a0(sint16 sequence, sint16 bank, sint16 program, uint16 volume, uint16 pan);
sint32 p2_sdk_80079a18(uint16 bank, sint16 program);
sint32 p2_sdk_80075fd0(sint16 bank, sint16 program, uint32 destination_offset);
sint32 p2_sdk_800760d8(sint16 bank, sint16 program, uint32 tone, uint32 destination_offset);
sint32 p2_sdk_800765f0(sint16 bank, sint16 program, uint32 tone, uint32 source_offset);
uint32 p2_sdk_80072b5c(sint16 bank, sint16 program, sint16 tone, uint32 attributes_offset, uint8 value);
uint32 p2_sdk_80072bec(sint16 bank, sint16 program, sint16 tone, uint32 attributes_offset, uint8 value);
uint32 p2_sdk_80072ca8(sint16 bank, sint16 program, sint16 tone, uint32 attributes_offset, uint8 value);
uint32 p2_sdk_80072d38(sint16 bank, sint16 program, sint16 tone, uint32 attributes_offset, uint8 value);
uint32 p2_func_80066d08(void);
uint32 p2_func_80066de4(uint32 asset);
uint32 p2_func_8006e6fc(uint32 selection);
uint32 p2_func_80027228(void);
uint32 p2_func_80066b78(uint16 clut, uint32 page, uint32 palette);
uint32 p2_func_800651bc(uint32 position, uint32 extent, sint32 fraction, uint32 color);
uint32 p2_func_800663ac(uint32 destination, uint32 source, uint32 count);
uint32 p2_func_8006a940(uint16 voice);
void p2_func_8005e470(uint32 sound, uint16 voice, uint32 arg3, uint32 arg4, uint32 override_pitch, sint32 volume, uint16 program);
uint32 p2_func_8006ce38(void);
uint32 p2_sdk_800804bc(uint32 table, sint32 count);
sint32 p2_sdk_80070df8(uint32 mask);
uint32 p2_func_8006a740(uint16 voice, uint16 volume);
uint32 p2_func_8006a638(uint16 voice, uint16 pan);
uint32 p2_func_8006a814(uint16 voice, uint32 pitch);
void p2_func_8006a878(uint16 voice, uint32 sound);
uint32 p2_func_8006c7b4(sint32 selection, sint32 music_volume, sint32 sound_volume);
uint32 p2_sdk_8007101c(uint32 attributes);
uint32 p2_sdk_80070bc0(uint32 enabled, uint32 voices);
uint32 p2_sdk_80070dc4(uint32 attributes);
uint32 p2_func_8006a5e0(uint32 handle, uint16 volume);
uint32 p2_func_800652e4(uint32 position, uint32 extent, sint32 fraction, uint32 color);
uint32 p2_func_80066e48(void);
uint32 ApplyCharacterSettings(uint32 scene, const CharacterSettings *settings, uint8 score);
uint32 p2_func_8006a970(uint16 voice);
uint32 p2_sdk_800829d8(uint32 destination, uint32 value, uint32 bytes);
uint32 p2_sdk_80075980(sint32 sequence);
uint32 p2_sdk_80079b14(sint32 bank);
uint32 p2_sdk_80078d1c(sint16 sequence, sint16 separation, sint16 volume, sint16 mode);
uint32 p2_sdk_80079350(sint16 sequence);
uint32 p2_sdk_80078474(void);
uint32 p2_sdk_800793ec(uint32 tones_offset, uint32 samples_offset);
uint32 p2_sdk_80077b98(void);
uint16 p2_sdk_80077dac(void);
uint32 p2_sdk_80076b94(uint32 mode, uint32 voices);
uint32 p2_sdk_80077930(void);
uint32 p2_sdk_80078430(uint8 voice);
uint32 p2_sdk_800776bc(sint16 sequence, sint16 bank, sint16 program, uint16 note);
uint32 p2_sdk_80078544(uint32 layers, sint16 pitch);
uint32 p2_sdk_80077f60(uint8 voice);
uint32 p2_sdk_80077168(sint16 sequence, sint16 bank, sint16 program, uint16 note, uint16 velocity, uint16 pan);
uint32 p2_sdk_800770d0(sint32 count, uint32 table);
uint32 p2_sdk_80076dc0(uint8 voices);
uint32 p2_sdk_800767dc(void);
uint32 p2_sdk_80075bb4(uint32 mode);
uint32 p2_sdk_80075020(sint16 sequence, sint16 separation);
uint32 p2_sdk_800757b0(sint16 sequence, sint16 separation);
uint32 p2_sdk_80075df4(sint16 sequence, sint16 separation);
uint32 p2_sdk_80074a98(sint16 sequence, sint16 separation);
uint32 p2_sdk_80074d74(sint16 sequence, sint16 separation);
uint32 p2_sdk_80074820(void);
uint32 p2_sdk_80075818(sint16 sequence, sint16 track);
uint32 p2_func_80053ca0(uint32 camera_offset);
uint32 p2_func_8006de08(uint32 camera_offset);
uint32 p2_func_80044780(void);
uint32 p2_func_8006aaa8(void);
uint32 p2_func_8006adc8(uint8 handle);
uint32 p2_func_80071a68(sint16 sequence);
uint32 p2_func_800718ec(sint16 sequence);
uint32 p2_func_800602f4(uint32 object);
uint32 p2_func_800401a8(uint32 object);
uint32 Atan2Angle(sint32 x, sint32 y);
uint32 p2_func_8001a6ac(uint32 object);
extern uint32 p2_guest_stack_pointer;
uint32 p2_func_80016ef4(uint32 scene_offset);
uint32 p2_func_800401c8(uint32 object);
uint32 p2_func_8005bc58(uint32 animation_offset, sint32 count);
uint32 p2_func_8002e00c(uint32 light_offset);
uint32 p2_func_8002417c(sint32 scale);
uint32 p2_func_8002e344(void);
void p2_func_800165c8(uint32 scene_offset);
uint32 p2_func_80016d0c(void);
uint32 p2_func_80012eb8(void);
uint32 p2_func_8005d170(void);
uint32 p2_func_8005fb40(void);
uint32 p2_func_8005cf3c(void);
void p2_func_800606f4(void);
uint32 p2_func_800611e8(uint32 object_offset);
uint32 p2_func_80044230(uint32 object_offset);
uint32 p2_func_80030ac8(uint32 object_offset);
uint32 p2_func_800351a4(uint32 object_offset);
uint32 p2_func_80041060(uint32 object_offset, uint32 other_offset, CollisionContact *contact_data);
uint32 p2_func_80040a1c(uint32 object_offset, uint32 other_offset, uint32 mode, uint32 value);
uint32 p2_func_80045ed8(uint32 object_offset);
uint32 p2_func_800414d0(uint32 object_offset);
uint32 p2_func_8004165c(uint32 object_offset);
uint32 p2_func_8004264c(uint32 object_offset);
uint32 p2_func_80042024(uint32 object_offset);
uint32 p2_func_80042320(uint32 definition_offset, uint32 object_offset);
uint32 p2_func_80041edc(uint32 object_offset, uint32 distance);
uint32 p2_func_80022d38(void);
sint32 SinQ14(sint16 angle);
uint32 p2_sdk_8007ec64(uint32 source_offset, uint32 destination_offset);
uint32 p2_func_80016c54(uint32 scene_offset, uint32 mode);
uint32 p2_func_80011e04(void);
uint32 p2_func_8002ab38(uint32 value);
sint32 AsinAngle(sint32 height, sint32 length);
void p2_func_80061174(void);
uint32 p2_func_80011338(uint32 point_offset, uint32 radius);
uint32 p2_sdk_8007e794(uint32 input);
uint32 p2_sdk_8007e944(uint32 matrix_offset);
uint32 p2_sdk_8007ea34(uint32 matrix_offset, uint32 destination_offset);
uint32 p2_func_800447f0(void);
sint32 ClassifyCharacterContactResponse(uint32 object_offset);
uint32 p2_func_8002e824(uint32 vector_offset, sint32 radius);
void p2_func_80011980(uint32 owner_offset, uint32 span_offset);
uint32 p2_func_8005d1b0(uint32 object_offset);
void p2_func_80025eac(uint32 object_offset);
uint32 p2_func_80012298(uint32 object_offset);
uint32 p2_func_80012424(uint32 node_offset, uint32 owner_offset);
uint32 p2_func_8002aa1c(uint32 x, uint32 y, uint32 z);
uint32 p2_func_80064a58(void);
sint32 AcosAngle(sint32 height, sint32 length);
void p2_func_80019cf4(uint32 owner_offset, uint32 node_offset);
uint32 p2_func_8001156c(uint32 node_offset);
uint32 p2_sdk_8007e834(uint32 left_offset, uint32 right_offset, uint32 destination_offset);
uint32 p2_func_80063a80(uint32 node_offset);
uint32 p2_func_80063e78(uint32 node_offset);
uint32 p2_func_80019924(sint32 frame, uint32 track_offset, uint32 destination_offset);
uint32 p2_func_80017938(sint32 frame, uint32 track_offset, uint32 destination_offset);
uint32 p2_func_80019684(sint32 frame, uint32 track_offset, uint32 owner_offset, uint32 span_offset);
uint32 p2_func_80019260(uint32 owner_offset, uint32 span_offset);
uint32 p2_func_800615c4(uint32 vertices_offset, uint32 polygons_offset, sint32 count);
uint32 p2_func_80061f50(uint32 vertices_offset, uint32 polygons_offset, sint32 count);
uint32 p2_func_80062e3c(uint32 vertices_offset, uint32 polygons_offset, sint32 count);
void p2_func_800631f0(uint32 vertices_offset, uint32 polygons_offset, sint32 count);
uint32 p2_func_8002e84c(uint32 mode);
uint32 p2_func_8002ede4(uint32 vertex_offset, uint32 flags);
uint32 p2_func_800612c0(uint32 material_offset, uint32 destination_offset, uint32 stride);
uint32 p2_func_8002e684(void);

uint32 p2_func_8001bc44(uint32 context_offset);

uint32 p2_func_8002a144(uint32 object_offset, uint32 *source_head, uint32 *destination_head);

uint32 p2_func_80034f04(void);
uint32 p2_func_8003420c(uint32 index);




void p2_func_8005c588(uint32 object_offset, uint32 auxiliary_offset);
uint32 FinishCharacterContacts(void);
uint32 p2_func_8003a930(uint32 object_offset);
uint32 p2_func_8005bff4(void);

void p2_func_800260b0(void);
uint32 p2_func_80034658(uint32 object_offset);
void p2_func_800346c8(uint32 object_offset);
uint32 p2_func_8006117c(uint32 object_offset);
uint32 p2_func_8004002c(uint32 source_offset);
uint32 p2_func_80033d7c(uint32 object_offset);
uint32 p2_func_80033564(uint32 object_offset);
uint32 p2_func_800534ec(void);
void p2_func_8005927c(void);
uint32 p2_func_80034ba8(uint32 object_offset, uint32 data_offset, uint32 resource_offset);
uint32 p2_func_800378c8(uint32 object_offset, uint32 distance, uint32 distance_offset, uint32 height_offset, uint32 flags_offset);
uint32 p2_func_80037c50(uint32 object_offset, uint32 distance, uint32 distance_offset, uint32 height_offset, uint32 flags_offset);
uint32 p2_func_8002afa8(uint32 object_offset);
uint32 p2_func_8002b000(uint32 object_offset);
uint32 p2_func_80030dcc(uint32 object_offset, uint32 position_offset, uint32 target_offset, uint32 distance_offset);
uint32 p2_func_80032248(uint32 object_offset, uint32 target_offset, uint32 origin_offset, uint32 resource_offset);
sint32 p2_func_8005642c(sint32 first_frame, sint32 second_frame, uint32 definition_offset);
uint32 p2_func_80056340(uint32 definition_offset, sint32 frame, uint32 destination_offset, uint32 direction);
uint32 p2_func_800352f4(uint32 object_offset, uint32 data_offset);
uint32 p2_func_800378a0(uint32 object_offset);
uint32 p2_func_80053e7c(sint16 mode, uint32 scene_offset, sint32 first, sint16 second);
uint32 p2_func_8005bb90(sint32 first, sint32 second);
uint32 p2_func_8005bb48(uint32 event, uint32 kind);

uint32 p2_func_8002c510(uint32 object_offset);
uint32 p2_func_8002a2f4(uint32 owner_offset, uint32 object_offset);
void p2_func_80034848(uint32 allocation_offset);
void p2_func_8003ff6c(uint32 node_offset, uint32 destination_offset, uint32 source_offset);
uint32 p2_func_80025f58(void);
sint32 p2_func_80083310(uint32 first_offset, uint32 second_offset, sint32 count);
uint32 p2_func_800140f8(uint32 scene_offset, uint32 parent_offset, uint32 mode);

uint32 p2_func_80013cec(uint32 object_offset, uint32 parent_offset);
void p2_func_8001aecc(uint32 parent_offset, uint32 object_offset);
uint32 p2_func_8001baa8(uint32 object_offset);
uint32 p2_func_80064358(uint32 object_offset);
uint32 p2_func_80018fec(sint16 angle, uint32 quaternion_offset, uint32 destination_offset);
uint32 p2_func_80034044(uint32 object_offset);
uint32 p2_func_80024b3c(uint32 vector_offset);

uint32 p2_func_80043de4(uint32 object_offset);
uint32 p2_func_80055848(uint32 scene);
uint32 p2_func_8001b334(uint32 object_offset, sint32 step);
uint32 p2_func_8001bb58(uint32 object_offset);
uint32 p2_func_8001a76c(uint32 track_offset, uint32 vector_offset, sint32 step, sint32 bias);
uint32 p2_func_8001aaa8(uint32 position_track_offset, uint32 rotation_track_offset, uint32 position_offset, uint32 quaternion_offset, sint32 step);

uint32 SpawnActionEffect(uint32 object_offset, uint32 character_offset, uint32 action);
uint32 BeginCharacterInteraction(uint32 object_offset, uint32 character_offset);

uint32 p2_func_80048af0(uint32 object_offset, uint32 character_offset, sint32 mode);
uint32 p2_func_800255f8(uint32 scene_offset);
uint32 ResolveCharacterFormVariant(sint32 kind, uint32 character_offset);


uint32 p2_func_8003db7c(uint32 object_offset, sint32 distance, sint32 height, const VECTOR *direction_vector);
uint32 p2_func_80056884(uint32 object_offset, uint32 character_offset);
uint32 p2_func_8004fa54(uint32 object_offset, uint32 vector_offset);
uint32 p2_func_80056328(void);
uint32 p2_func_80058728(uint32 object_offset);


uint32 p2_func_80040384(uint32 object_offset, uint32 vector_offset, sint16 step);
uint32 p2_func_80018144(uint32 object_offset, uint32 vector_offset, uint32 axes_offset);
uint32 p2_func_80017e4c(uint32 matrix_offset, uint32 quaternion_offset);

uint32 p2_func_80053704(uint32 object_offset);

uint32 UpdateCharacterMovement(uint32 object_offset);
uint32 p2_func_80060fdc(uint32 input);

uint32 p2_func_8005b588(uint32 object_offset, uint32 character_offset);
sint32 p2_func_8005814c(uint32 object_offset, uint32 character_offset, uint32 input_offset);
uint32 p2_func_800567fc(uint32 object_offset);
uint32 p2_func_80058c78(uint32 object_offset);
uint32 p2_func_800586e8(uint32 object_offset);
uint32 p2_func_80056700(uint32 object_offset);
uint32 p2_func_800567e4(uint32 object_offset);
uint32 p2_func_80056774(uint32 object_offset);
uint32 p2_func_800567d0(uint32 object_offset);
uint32 p2_func_80057208(uint32 object_offset, uint32 character_offset);
uint32 p2_func_80056810(uint32 object_offset, uint32 character_offset);
uint32 p2_func_800585e0(uint32 object_offset);
uint32 p2_func_80058618(uint32 object_offset);
uint32 p2_func_80058640(uint32 object_offset);
uint32 p2_func_8005873c(uint32 object_offset);
uint32 p2_func_80058ca0(uint32 object_offset, uint32 character_offset);

uint32 p2_func_80044ffc(sint32 action, uint32 audio_value);

uint32 p2_func_8004922c(uint32 object_offset, uint32 character_offset, uint32 decelerate);

uint32 p2_func_8001d698(uint32 object_offset, uint32 character_offset);

uint32 p2_func_80058d08(uint32 object_offset, uint32 character_offset, uint32 input_flags, uint32 mode, uint32 horizontal_offset, uint32 vertical_offset);

uint32 p2_func_80058f50(uint32 object_offset, uint32 character_offset, uint32 input_flags, uint32 mode, uint32 horizontal_offset, uint32 vertical_offset);

uint32 p2_func_80049730(uint32 object_offset, uint32 character_offset, uint32 horizontal_offset, uint32 vertical_offset);

uint32 p2_func_800455dc(uint32 object_offset, uint32 character_offset, sint32 horizontal_speed, sint32 vertical_speed);

uint32 p2_func_8001db60(uint32 object_offset, uint32 character_offset, uint32 grounded);

uint32 p2_func_8001eb44(uint32 object_offset, uint32 character_offset);

uint32 p2_func_80038990(uint32 bounds_offset, uint32 object_offset, uint32 delta_offset, uint32 extent_offset, uint32 geometry_offset);

uint32 p2_func_8005eb18(uint32 sound_offset, sint32 volume, uint32 pan, uint32 pitch, uint32 position_offset, uint32 radius, uint32 voice, uint32 mode, uint32 option);

uint32 p2_func_8005e8bc(uint32 position_offset, uint32 radius, uint32 attenuation_offset, uint32 pan_offset);

void ReleaseContacts(uint32 *head);

uint32 ContainsContact(uint32 head_offset, uint32 object_offset);

uint32 p2_func_80039ea4(uint32 object_offset);

uint32 p2_func_8003a384(uint32 object_offset);

uint32 p2_func_80039614(uint32 object_offset, uint32 other_offset);

uint32 p2_func_8001bd30(uint32 object_offset);

uint32 p2_func_8001ecec(void);

uint32 p2_func_8001ed4c(void);

uint32 p2_func_80039404(uint32 object_offset, uint32 position_offset, uint32 velocity_offset, uint32 radius);

sint32 p2_func_800380ac(uint32 object_offset, uint32 geometry_offset, uint32 other_offset, uint32 collision_offset, uint32 state_offset);

uint32 p2_func_80047058(uint32 object_offset, uint32 other_offset, uint32 context_offset);

uint32 p2_func_80039b20(uint32 object_offset, uint32 other_offset);

uint32 CollectCharacterContact(uint32 object_offset, uint32 other_offset, CollisionContact *contact_data);

uint32 CanCharacterContact(uint32 object_offset, uint32 other_offset);

uint32 p2_func_8001c544(uint32 object_offset, uint32 selection, uint32 descriptor_offset);

uint32 p2_func_8001c710(uint32 object_offset, uint32 descriptor_offset, uint32 selection);

uint32 QueueContact(uint32 *head, uint32 object_offset, uint32 empty_pool_node);

uint32 TestCharacterSurfaceContact(uint32 object_offset, uint32 other_offset, uint32 contact);


sint32 p2_func_8005b604(uint32 object_offset, uint32 other_offset);

uint32 p2_func_80030a34(uint32 object_offset);

uint32 p2_func_80034eac(uint32 object_offset);

uint32 p2_func_8002d750(uint32 kind);


uint32 p2_func_8001c69c(uint32 object_offset, uint32 sphere, uint32 first_offset, uint32 last_offset);

uint32 p2_func_8005bcfc(uint32 object_offset);

uint32 p2_func_800343f0(uint32 object_offset);

uint32 p2_func_80024890(void);

uint32 p2_func_80024c44(void);

uint32 p2_func_800530a0(uint32 object_offset);

uint32 p2_func_80045ed8(uint32 object_offset);

uint32 p2_func_8004eb74(uint32 object_offset, uint32 character_offset);
uint32 p2_func_80057650(uint32 object_offset);
uint32 p2_func_80058c8c(uint32 object_offset);
uint32 p2_func_80059268(uint32 object_offset);
uint32 p2_func_8004e118(uint32 object_offset, uint32 support_offset);
uint32 p2_func_8004df14(uint32 object_offset, uint32 character_offset);
uint32 p2_func_80046510(uint32 object_offset, uint32 path_offset, sint32 frame);

uint32 ProcessCharacterPickups(uint32 object_offset, uint32 character_offset);
uint32 p2_func_8002a80c(uint32 position_offset, uint32 scale, uint32 effect);

uint32 ProcessCharacterTriggers(uint32 object_offset, uint32 character_offset);

uint32 p2_func_800521bc(uint32 object_offset, uint32 character_offset);

uint32 p2_func_800524ac(uint32 object_offset, uint32 character_offset);

uint32 p2_func_800526f8(uint32 object_offset, uint32 character_offset);
uint32 p2_func_800457c8(uint32 object_offset);
uint32 p2_func_80059aec(uint32 destination_offset, uint32 object_offset);
void p2_func_80059ae4(uint32 character_offset);
void p2_func_8006abe4(void);
uint32 p2_func_8006aba0(void);

uint32 p2_func_80050c94(uint32 object_offset, uint32 character_offset);

void p2_func_80046628(uint32 object_offset, uint32 first, uint32 second, uint32 third, uint32 kind);

uint32 p2_func_800515f0(uint32 object_offset, uint32 character_offset);
uint32 UpdateCharacterInteraction(uint32 object_offset, uint32 character_offset);
uint32 p2_func_8006ec24(uint32 left_low, uint32 left_high, uint32 right_low, uint32 right_high);

uint32 p2_func_80052ba4(uint32 object_offset, uint32 character_offset);

uint32 p2_func_8004fd44(uint32 object_offset, uint32 character_offset);
uint32 p2_func_800607bc(uint32 object_offset);
uint32 p2_func_80060c0c(uint32 object_offset);
uint32 p2_func_8003fbb0(uint32 object_offset);
uint32 p2_func_8003fb58(uint32 descriptor);
uint32 p2_func_800606fc(uint32 object_offset);
uint32 p2_func_8003fc44(uint32 object_offset, uint32 action);

uint32 p2_func_8002d7a4(uint32 kind);
sint32 p2_func_800185c8(uint32 object_offset, uint32 vector_offset, uint32 axes_offset);

uint32 p2_func_8002f75c(uint32 object_offset, uint32 other_offset);
uint32 p2_func_8002f944(uint32 object_offset, uint32 other_offset, CollisionContact *contact_data);
void QueueCharacterPickup(uint32 object_offset, uint32 context_offset);

uint32 p2_func_8004483c(uint32 object_offset);

uint32 p2_func_80074138(uint32 sequence, uint32 track);
uint32 p2_sdk_80079244(sint16 handle, uint32 left_offset, uint32 right_offset);

uint32 p2_func_80048db4(uint32 object_offset);

void p2_func_80059ac4(void);
void p2_func_80059acc(void);
uint32 p2_func_80035058(uint32 angle_offset, uint32 vector_offset, uint32 object_offset, sint32 frame, uint32 reverse);
uint32 AdvanceActionEffect(uint32 object_offset, uint32 owner_offset, uint32 character_offset, uint32 action, uint32 distance, uint32 output_offset);

uint32 p2_func_80018b40(uint32 object_offset, uint32 vector_offset, uint32 quaternion_offset);
uint32 p2_func_80037f08(uint32 object_offset);

uint32 p2_func_8002fd54(uint32 object_offset);

uint32 p2_func_8002c608(uint32 object_offset);
void p2_func_8002b278(uint32 definition_offset, uint32 spawned_offset, uint32 object_offset, uint32 phase);

uint32 p2_func_8004a888(uint32 object_offset, uint32 character_offset);
uint32 p2_func_8001d3f0(uint32 object_offset, uint32 character_offset, sint32 x, sint32 y, sint32 z);

uint32 RestoreCharacterHealthPoint(void);
uint32 p2_func_80030c2c(uint32 object_offset);
uint32 p2_func_80035154(uint32 angle_offset, uint32 vector_offset, uint32 object_offset);
uint32 p2_func_8003557c(uint32 object_offset, uint32 position_offset, uint32 target_offset, uint32 distance_offset, uint32 incoming_action_offset);
sint32 p2_func_80037f78(uint32 object_offset, uint32 position_offset, uint32 target_offset);


uint32 sprintfPSX(uint32 destination, uint32 format, uint32 value);

uint32 p2_func_8003bc08(uint32 object_offset, uint32 position_offset, uint32 floor_offset, uint32 direction);
uint32 p2_func_80058714(uint32 object_offset);

uint32 p2_func_8001f6ac(uint32 object_offset);
uint32 p2_func_80022358(uint32 object_offset);
uint32 p2_func_8003ff3c(uint32 object_offset);
uint32 p2_func_80044444(uint32 object_offset);
uint32 p2_func_80020c08(uint32 object_offset);
uint32 p2_func_80021678(uint32 object_offset);
uint32 p2_func_80021e28(uint32 object_offset);
uint32 p2_func_8002293c(uint32 object_offset);
uint32 p2_func_80027a8c(uint32 object_offset);
uint32 p2_func_8005dfb8(uint32 object_offset);
uint32 p2_func_8005e418(uint32 object_offset);
uint32 p2_func_8005fadc(uint32 object_offset);
uint32 p2_func_80060688(uint32 object_offset);
uint32 p2_func_8005f490(uint32 object_offset);
void p2_func_80026b64(void);
void p2_func_8002f180(void);
void p2_func_80034ba0(void);
void p2_func_80059ad4(void);
void p2_func_8002d510(void);
uint32 p2_func_800352cc(void);

uint32 p2_func_8002f188(uint32 destination, uint32 source, uint32 quaternion);
uint32 p2_func_8002f318(uint32 destination, uint32 source, sint16 angle);
uint32 p2_func_8002f3b0(uint32 destination, uint32 source, sint16 first_angle, sint16 second_angle);
uint32 p2_func_8002f4c0(uint32 destination, uint32 source, sint16 first_angle, sint16 second_angle);
uint32 p2_func_8002f5d0(uint32 destination, uint32 source, sint16 first_angle, sint16 second_angle, sint16 third_angle);

uint32 p2_func_8002b220(uint32 object_offset);
uint32 p2_func_8002d51c(uint32 kind);
uint32 p2_func_8002d71c(uint32 kind, uint8 value);

sint32 p2_func_800309bc(uint32 object_offset);
sint32 p2_func_80034958(uint32 object_offset, uint32 position, uint32 target);
uint32 p2_func_800401f0(uint32 object_offset, uint32 target, sint16 step);
uint32 p2_func_800402bc(uint32 object_offset, uint32 target, sint16 step);
sint32 p2_func_80056660(sint32 first_frame, sint32 second_frame, uint32 path_offset);
uint32 p2_func_80057480(uint32 object_offset, uint32 path_offset, uint32 destination);

uint32 p2_func_8004a928(uint32 object_offset, uint32 character_offset);
uint32 UpdateForm5Movement(uint32 object_offset);

uint32 p2_func_8006d2ec(void);
uint32 p2_func_80071a8c(sint16 sequence);
uint32 p2_func_80074238(sint16 sequence, sint16 track);
void p2_func_800742d4(void);
uint32 p2_func_800743c0(sint16 sequence, sint16 track);
void p2_func_80074770(void);
uint32 p2_func_800759a8(sint16 sequence, sint16 track);
void p2_func_800767b8(void);
uint32 p2_func_800767c8(void);

uint32 p2_func_80067778(void);
uint32 p2_func_8006b04c(uint32 position, uint32 alternate, sint32 selection);
uint32 p2_func_80032048(uint32 state, uint32 object, uint32 sound, uint32 mode);
uint32 p2_func_8005c40c(uint32 object);

uint32 p2_func_80035494(uint32 object, uint32 position, uint32 target, uint32 distance, uint32 definition, uint32 character, uint8 allowed);
uint32 p2_func_8003bb18(uint32 object, uint32 position, uint32 direction, uint32 context);
sint32 p2_func_800574b4(uint32 path, uint32 position, uint32 floor, uint32 direction);

uint32 p2_func_8001fa08(void);
sint32 p2_func_80022388(uint32 object);
uint32 p2_func_800229a4(uint32 object);
uint32 p2_func_800278e8(uint32 object);
uint32 p2_func_8003fdc8(uint32 object);
uint32 p2_func_8004425c(uint32 object);

uint32 p2_func_800216cc(uint32 object);
uint32 p2_func_8005de30(uint32 object);
uint32 p2_func_80060360(uint32 object);

uint32 p2_func_8001fa24(uint32 object, uint32 script, uint32 target, sint32 index);

uint32 p2_func_80020094(uint32 object, uint32 origin, uint32 target, sint32 distance, uint32 resource, uint32 script);

uint32 p2_func_800204cc(uint32 object, uint32 origin, uint32 target, sint32 distance, uint32 resource, uint32 script);
uint32 p2_func_8001f6dc(uint32 object);

uint32 p2_func_80021254(uint32 object, uint32 origin, uint32 target, sint32 distance, uint32 resource, uint32 script);
uint32 p2_func_800213d8(uint32 object, uint32 resource, uint32 script);
uint32 p2_func_80020f44(uint32 object, uint32 origin, uint32 target, sint32 distance, uint32 resource, uint32 script);
uint32 p2_func_80020c6c(uint32 object);

uint32 p2_func_8005f154(uint32 object);

uint32 p2_func_8005f510(uint32 object);

uint32 p2_func_8005e010(uint32 object);

uint32 p2_func_800224ac(uint32 object);

uint32 p2_func_80021888(uint32 object);

uint32 p2_func_80021e70(uint32 object);

uint32 p2_func_8001edac(uint32 object);

void p2_func_80073660(uint8 value);
void p2_func_800736ac(uint8 value);
void p2_func_800736d0(uint8 value);
void p2_func_800736f4(uint8 value);
uint32 p2_func_8006f824(uint32 source, uint16 address_units, uint32 blocks, uint32 port);
uint32 p2_func_8008040c(uint32 ordering_table, sint32 count, uint32 debug_level);
void p2_func_8008342c(void);

uint32 p2_func_8007aa1c(uint32 driver, uint32 channel, uint32 callback);
uint32 p2_func_8007aa7c(uint32 driver);
uint32 p2_func_8007fc78(uint32 reverse, uint32 debug_level);
uint32 p2_func_8007fdf8(uint32 enabled, uint32 debug_level);
uint32 p2_func_80080cf0(uint32 driver);
uint32 p2_func_8008373c(uint32 type, uint32 destination);

#ifdef __cplusplus
}
#endif

#endif
