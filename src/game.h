#ifndef P2_GAME_H
#define P2_GAME_H

#include "xport.h"
#include "psx.h"
#include "psx_spu.h"
#include "scene_path.h"
#include "animation_data.h"
#include "collision_contact.h"
#include "collision_geometry.h"
#include "matrix_rotation.h"
#include "object_orientation.h"

#ifdef __cplusplus
    #include "random_generator.h"
    #include "frame_clock.h"
    #include "render_view.h"
    #include "scene_frame.h"
    #include "camera_view.h"
    #include "scene_lighting.h"
    #include "render_commands.h"
    #include "screen_render.h"
    #include "game_flow.h"
    #include "player_interaction.h"
    #include "resource_archive.h"
    #include "resource_memory.h"
    #include "resource_groups.h"
    #include "scene_loading.h"
    #include "scene_events.h"
    #include "scene_section.h"
    #include "level_resource.h"
    #include "model_registry.h"
    #include "actor_lifecycle.h"
    #include "actor_behavior.h"
    #include "actor_binding.h"
    #include "model_collision.h"
    #include "object_speed.h"
    #include "volley_script.h"
    #include "attack_script.h"
    #include "cutscene.h"
    #include "material_registry.h"
    #include "material_record.h"
    #include "material_resources.h"
    #include "text_render.h"
    #include "character_controller.h"
    #include "character_contacts.h"
    #include "character_models.h"
    #include "character_support.h"
    #include "character_path_transition.h"
    #include "character_vitals.h"
    #include "character_timers.h"
    #include "character_motion.h"
    #include "object_operations.h"
    #include "object_links.h"
    #include "scene_node.h"
    #include "animated_object.h"
    #include "mesh_morph.h"
    #include "sprite_objects.h"
    #include "sound_effects.h"
    #include "sound_resources.h"
    #include "spu_attributes.h"
    #include "menu_state.h"
    #include "character_preview.h"
    #include "password.h"
    #include "control_bindings.h"
    #include "gameplay_input.h"
    #include "sound_settings.h"
    #include "sequence_playback.h"
#endif

#ifdef __cplusplus
extern "C"
{
#endif

    extern uint32 fixed_math_result;

    uint32 p2_reset_graph_zero(void);
    PsxCallbackDriver *p2_reset_callback(void);

    uint32 p2_sdk_80074210(sint16 sequence);
    uint32 PauseCharacterSound(void);
    void ResumeCharacterSound(void);
    uint32 WaitSceneResourceLoad(void);

    uint32 PlayMovie(const char *path);
    void p2_func_8008347c(void);
    sint32 FindDiscFile(CdlFILE *result, const char *path);
    sint32 p2_func_8007bc54(const uint8 *position);
    uint32 p2_func_8008346c(void);

    CdlCB SetCdReadCallback(CdlCB callback);

    /* Forward declarations retain the original identity until native integration */
    uint32 CaptureCharacterSettings(CharacterSettings *settings);
    uint32 ResetRenderCommands(void);
    uint32 ClearSceneOrderingTable(void);
    uint32 BeginDraw(void);
    uint32 EndDraw(void);
    void NoOp80066CE8(void);
    uint32 LoadMenuResources(void);
    void InitializeGame(uint32 mode);
    uint32 p2_func_8006a614(void);
    void p2_func_8006af4c(void);

    void InitGameSound(void);
    uint32 p2_func_8006f158(void);
    uint32 p2_func_80071c5c(void);
    uint32 p2_func_80077124(uint32 mode);
    uint32 p2_func_8007b448(void);
    void p2_func_80074750(void);
    uint32 p2_func_8008371c(uint32 buffer0, uint32 size0, uint32 buffer1, uint32 size1);
    uint32 p2_func_8008372c(void);
    uint32 p2_func_80077150(void);
    uint32 p2_func_80070e8c(const void *source, uint32 bytes);
    sint32 p2_sdk_8006ff38(SpuReverbAttr *attributes);
    uint32 p2_sdk_800759d4(SpuSequenceSchedulerTrack *table, sint16 sequences, sint16 separations);
    uint32 p2_sdk_80079adc(uint8 voices);
    uint32 p2_sdk_8006f178(uint32 mode);
    uint32 p2_sdk_80071b74(void);
    void p2_sdk_800744ec(uint32 mode);
    PsxInterruptCallback p2_sdk_8007febc(PsxInterruptCallback callback);
    sint32 p2_sdk_8007e16c(sint32 sectors, uint32 destination, uint32 mode);
    sint32 p2_sdk_8007e274(sint32 mode, uint8 *result);
    uint32 p2_sdk_80079f48(const uint8 *header, uint32 bytes, sint32 bank, uint32 address);
    uint32 p2_sdk_8007a680(const uint8 *samples, uint32 sample_bytes, sint16 bank);
    uint32 p2_sdk_8007a740(uint32 mode);
    uint32 p2_sdk_80071c8c(const uint8 *sequence, uint32 bytes, sint16 bank);
    uint32 p2_sdk_8006fb68(const void *source, uint32 bytes);
    uint32 p2_sdk_80070eec(uint32 address);
    uint32 p2_sdk_80082a18(const char *format, ...);
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
    sint32 p2_sdk_80075fd0(sint16 bank, sint16 program, uint8 *attributes);
    sint32 p2_sdk_800760d8(sint16 bank, sint16 program, uint32 tone, uint8 *attributes);
    sint32 p2_sdk_800765f0(sint16 bank, sint16 program, uint32 tone, const uint8 *attributes);
    uint32 DecodeToneAdsr(uint16 first, uint16 second, uint16 *fields);
    uint32 EncodeToneAdsr(const uint16 *fields, uint16 *first, uint16 *second);
    uint32 SetSequenceMasterVolume(uint8 value);
    uint32 p2_sdk_80072dc8(sint16 bank, sint16 program, sint16 tone, VagAtr attributes, uint8 value);
    uint32 p2_sdk_80072f64(sint16 bank, sint16 program, sint16 tone, VagAtr attributes, uint8 value);
    uint32 p2_sdk_8007301c(sint16 bank, sint16 program, sint16 tone, VagAtr attributes, uint8 value);
    uint32 p2_sdk_800730cc(sint16 bank, sint16 program, sint16 tone, VagAtr attributes, uint8 value);
    uint32 p2_sdk_8007317c(sint16 bank, sint16 program, sint16 tone, VagAtr attributes, uint8 value);
    uint32 p2_sdk_80073230(sint16 bank, sint16 program, sint16 tone, VagAtr attributes, uint8 value);
    uint32 p2_sdk_800732e8(sint16 bank, sint16 program, sint16 tone, VagAtr attributes, uint8 value);
    uint32 p2_sdk_8007339c(sint16 bank, sint16 program, sint16 tone, VagAtr attributes, uint8 value);
    uint32 p2_sdk_80073444(sint16 bank, sint16 program, sint16 tone, VagAtr attributes, uint8 value);
    uint32 p2_sdk_80073520(sint16 bank, sint16 program, sint16 tone, VagAtr attributes, uint8 value);
    uint32 p2_sdk_800735c0(sint16 bank, sint16 program, sint16 tone, VagAtr attributes, uint8 value);
    uint32 p2_sdk_80072b5c(sint16 bank, sint16 program, sint16 tone, VagAtr attributes, uint8 value);
    uint32 p2_sdk_80072bec(sint16 bank, sint16 program, sint16 tone, VagAtr attributes, uint8 value);
    uint32 p2_sdk_80072ca8(sint16 bank, sint16 program, sint16 tone, VagAtr attributes, uint8 value);
    uint32 p2_sdk_80072d38(sint16 bank, sint16 program, sint16 tone, VagAtr attributes, uint8 value);
    uint32 UpdateResourceMaterialAnimations(void);
    uint32 UploadTexturePalette(uint16 clut, uint32 page, const uint8 *palette);
    uint32 MoveBytes(uint8 *destination, const uint8 *source, uint32 count);
    uint32 p2_sdk_800804bc(uint32 table, sint32 count);
    sint32 p2_sdk_80070df8(uint32 mask);
    uint32 p2_sdk_80070bc0(uint32 enabled, uint32 voices);
    uint32 ApplyCharacterSettings(const AnimatedObject *object, const CharacterSettings *settings, uint8 score);
    uint32 p2_sdk_80075980(sint32 sequence);
    uint32 p2_sdk_80079b14(sint32 bank);
    uint32 p2_sdk_80078d1c(sint16 sequence, sint16 separation, sint16 volume, sint16 mode);
    uint32 p2_sdk_80079350(sint16 sequence);
    uint32 p2_sdk_80078474(void);
    uint32 p2_sdk_800793ec(uint8 *tones, uint8 *samples);
    uint32 p2_sdk_80077b98(void);
    uint16 p2_sdk_80077dac(void);
    uint32 p2_sdk_80076b94(uint32 mode, uint32 voices);
    uint32 p2_sdk_80077930(void);
    uint32 p2_sdk_80078430(uint8 voice);
    uint32 p2_sdk_800776bc(sint16 sequence, sint16 bank, sint16 program, uint16 note);
    uint32 p2_sdk_80078544(uint32 layers, sint16 pitch);
    uint32 p2_sdk_80077f60(uint8 voice);
    uint32 p2_sdk_80077168(sint16 sequence, sint16 bank, sint16 program, uint16 note, uint16 velocity, uint16 pan);
    extern SpuSequenceSchedulerTrack sequence_track_storage[2];
    uint32 p2_sdk_800770d0(sint32 count, SpuMallocRecord *table);
    uint32 p2_sdk_80076dc0(uint8 voices);
    uint32 p2_sdk_800767dc(void);
    uint32 p2_sdk_80075bb4(uint32 mode);
    uint32 p2_sdk_80075020(sint16 sequence, sint16 separation);
    void ResumeSequenceTrack(sint16 sequence, sint16 separation);
    void AdvanceSequenceTempoRamp(sint16 sequence, sint16 separation);
    uint32 p2_sdk_80074a98(sint16 sequence, sint16 separation);
    uint32 p2_sdk_80074d74(sint16 sequence, sint16 separation);
    void p2_sdk_80074820(void);
    uint32 p2_sdk_80075818(sint16 sequence, sint16 track);
    uint32 GetCurrentCharacterForm(void);
    uint32 p2_func_8006aaa8(void);
    uint32 p2_func_80071a68(sint16 sequence);
    uint32 p2_func_800718ec(sint16 sequence);
    uint32 Atan2Angle(sint32 x, sint32 y);
    void NoOp800606F4(void);
    uint32 UpdateAnimatedModelOrientation(const AnimatedModelDefinition *definition, AnimatedObject *object);
    uint32 ApplyModelTravelRotation(SceneNode *object, uint32 distance);
    sint32 SinQ14(sint16 angle);
    uint32 SquareRootQ12Exact(uint32 value);
    sint32 AsinAngle(sint32 height, sint32 length);
    void NoOp80061174(void);
    uint32 ApproxVectorLength(uint32 x, uint32 y, uint32 z);
    sint32 AcosAngle(sint32 height, sint32 length);

    void NoOp8005927C(void);

    uint32 GetSceneHealthPickupFlag(uint32 scene);

    uint32 SpawnActionEffect(AnimatedObject *object, const void *context, uint32 action);
    uint32 BeginCharacterInteraction(AnimatedObject *object);

    uint32 ResolveCharacterFormVariant(sint32 kind);

    uint32 QueueCharacterActionSound(sint32 action, uint32 priority);

    void NoOp80059AE4();

    uint32 p2_func_8006ec24(uint32 left_low, uint32 left_high, uint32 right_low, uint32 right_high);

    void QueueCharacterPickup(SceneNode *object, uint32 context_offset);

    uint32 p2_func_80074138(uint32 sequence, uint32 track);
    uint32 p2_sdk_80079244(sint16 handle, uint16 *left, uint16 *right);

    void NoOp80059AC4(void);
    void NoOp80059ACC(void);

    uint32 RestoreCharacterHealthPoint(void);

    uint32 sprintfPSX(uint32 destination, uint32 format, uint32 value);

    void NoOp80026B64(void);
    void NoOp8002F180(void);
    void NoOp80034BA0(void);
    void NoOp80059AD4(void);
    uint32 LoadMonsterScriptEntry(void);

    uint32 p2_func_80071a8c(sint16 sequence);
    uint32 p2_func_80074238(sint16 sequence, sint16 track);
    void p2_func_800742d4(void);
    uint32 p2_func_800743c0(sint16 sequence, sint16 track);
    void p2_func_80074770(void);
    uint32 p2_func_800759a8(sint16 sequence, sint16 track);
    void p2_func_800767b8(void);
    uint32 p2_func_800767c8(void);

    uint32 DrawGameOverFade(void);

    uint32 p2_func_80073660(uint8 value);
    uint32 p2_func_800736ac(uint8 value);
    uint32 p2_func_800736d0(uint8 value);
    uint32 p2_func_800736f4(uint8 value);
    uint32 p2_func_8006f824(uint32 source, uint16 address_units, uint32 blocks, uint32 port);
    uint32 p2_func_8008040c(uint32 ordering_table, sint32 count);
    void p2_func_8008342c(void);

    PsxInterruptCallback p2_func_8007aa1c(uint32 channel, PsxInterruptCallback callback);
    PsxCallbackDriver *p2_func_8007aa7c(void);
    uint32 p2_func_8007fc78(uint32 reverse);
    uint32 p2_func_8007fdf8(uint32 enabled);
    uint32 p2_func_80080cf0(void);
    uint32 p2_func_8008373c(uint32 type, uint32 destination);

#ifdef __cplusplus
}
#endif

#endif
