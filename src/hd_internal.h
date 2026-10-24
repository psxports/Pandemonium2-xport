#ifndef HD_INTERNAL_H
#define HD_INTERNAL_H

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hd.h"
#include "psx.h"
#include "psx_gpu.h"
#include "texture_descriptor.h"
#include "camera_projection.h"
#include "camera_motion.h"
#include "scene_path.h"
#include "vector3.h"
#include "mesh_geometry_node.h"
#include "scene_lighting.h"
#include "scene_section.h"
#include "animated_object.h"
#include "animation_header.h"
#include "animation_track.h"
#include "gameplay_input.h"
#include "background_resource.h"
#include "sprite_record.h"
#include "global.h"
#include "path_definition.h"
#include "scene_trigger.h"
#include "render_commands.h"
#include "screen_render_state.h"
#include "original_defaults.h"

enum
{
    HD_WORLD_UNIFORMS = 21,
    HD_NO_POSE = 0xFFFFFFFFu,
    HD_POSE_PARENT_POS = 1,
    HD_POSE_PARENT_ROT = 2,
    HD_POOL_SLOTS = SpritePoolState::EntryCount
};

struct HdMeshPass
{
    uint32 first, count, blend, mask;
    float alpha;
};

struct HdPoolRange
{
    HdPoolRange *next;
    const MeshVertex *vertices;
    const MeshFace *faces;
    uint32 first, count, refs, first_vertex, vertex_count;
};

struct HdScrollClock
{
    HdScrollClock *next;
    const TextureDescriptor *texture;
    uint32 frame;
    sint32 rate;
    uint64 epoch;
};

struct HdScroll
{
    const TextureDescriptor *texture, *source;
    TextureMapping mapping, source_mapping;
    uint32 flags, phase, frame;
    sint32 rate;
    uint64 epoch;
    uint16 fraction, period_u, period_v;
};

struct HdOgGeometry
{
    uint32 refs, vertex_count, face_count;
    MeshVertex *vertices;
    MeshFace *faces;
};

struct HdOgMaterial
{
    TextureDescriptor texture, source;
    bool present;
};

struct HdTexRegion
{
    uint16 left, top, width, height;
};

struct HdMesh
{
    HdMesh *next;
    const MeshVertex *vertices;
    const MeshFace *faces;
    const TextureDescriptor *material;
    uint32 vertex_count, first_vertex, face_count;
    HdOgGeometry *og_geometry;
    HdOgMaterial *og_materials;
    HdOgMaterial og_override;
    VID_GEOMETRY *geometry;
    VID_INDEX *indices;
    VID_INDEX *opaque_indices;
    uint32 opaque_count;
    HdMeshPass *passes;
    HdMeshPass *staging_passes;
    uint32 *source_indices;
    HdScroll *scrolls;
    HdTexRegion *regions;
    VID_TEXTURE *morph_texture;
    VID_MATERIAL *morph_materials[4];
    uint32 morph_width;
    uint32 pass_count;
    uint32 sample_refs;
    uint32 pool_first;
    HdPoolRange *pool_range;
    bool queued, pooled, pool_valid;
};

struct HdWorldDraw
{
    HdMesh *mesh;
    HdMesh *morph;
    HdMesh *scroll;
    float scroll_fract;
    uint32 first, count, blend;
    uint32 pose;
    MATRIXF model;
    HdLighting world_lights;
    bool relight;
    float uniforms[HD_WORLD_UNIFORMS * 4];
    VID_FOG fog;
    const SceneNode *group;
};

struct HdVisibleDraw
{
    uint32 first, count, material_first;
    bool eligible, duplicate, pooled;
};

struct HdUiPass
{
    uint32 first;
    uint32 count;
    uint32 blend;
    HdUiLayer layer;
};

struct HdPose
{
    MATRIXF model;
    const SceneNode *node;
    const AnimatedObject *owner;
    uint32 sample;
    bool valid;
};

struct HdAnimationCurve
{
    AnimationTrack track;
    const uint8 *source;
    uint32 refs;
    bool is_rotation;
};

struct HdAnimationChannel
{
    HdAnimationCurve *curve;
    const AnimationHeader *clip;
    sint32 frame, period;
    bool per_track;
};

struct HdAnimationPose
{
    const AnimatedObject *owner;
    Vec3 source_pos;
    Quat source_rotation;
    VECTORF pos;
    QUATF rotation;
    HdAnimationChannel position_channel, rotation_channel;
    bool pos_valid, rotation_valid, pending;
};

struct HdAnimationEntry
{
    const SceneNode *node;
    HdAnimationPose pose;
};

struct HdIdentity
{
    const SceneNode *node;
    uint64 id;
};

struct HdGraphWork
{
    const SceneNode *node;
    uint32 work;
};

struct HdPoseSample
{
    uint64 id, owner;
    uint32 parent, space;
    MATRIXF local;
    QUATF rotation;
    HdAnimationChannel position_channel, rotation_channel;
    VECTORF animation_scale;
};

struct HdPoseIndex
{
    uint64 id;
    uint32 index;
};

struct HdMixedPose
{
    MATRIXF model, source;
    uint32 previous;
};

struct HdPathPose
{
    const PathTransform *path;
    MATRIX source;
    MATRIXF model;
};

struct HdBackdrop
{
    const BackgroundResource *source;
    float pitch, yaw, width;
    float offset[2];
};

struct HdPreciseCamera
{
    Vec3 source[2];
    VECTORF value[2];
    bool valid[2];
    float angle[2];
    sint16 source_angle[2];
    Vec3 aim_source[2];
    bool aim_valid;
};

struct HdOgQuad
{
    uint32 pos, extent, blend;
    TextureDescriptor texture;
    HdOgQuadEmit emit;
};

struct HdOgRect
{
    uint32 pos, extent, color;
    sint32 fraction;
    HdOgRectEmit emit;
};

struct HdOgEffect
{
    sint32 intensity;
    HdOgEffectEmit emit;
};

struct HdOgSceneFade
{
    uint32 level;
    HdOgFadeEmit emit;
};

struct HdOgBackgroundRect
{
    uint32 pos, extent, color;
    HdOgBackgroundRectEmit emit;
};

struct HdOgBackgroundTile
{
    uint32 tile, xy, next_xy;
    BackgroundTile definition;
};

struct HdOgBackground
{
    uint32 refs, count;
    HdOgBackgroundTile *tiles;
};

enum HdOgKind
{
    HD_OG_QUAD, HD_OG_RECT, HD_OG_EFFECT, HD_OG_SCENE_FADE, HD_OG_BACKGROUND_RECT, HD_OG_BACKGROUND_TILES
};

struct HdOgDraw
{
    HdOgKind kind;
    union
    {
        HdOgQuad quad;
        HdOgRect rect;
        HdOgEffect effect;
        HdOgSceneFade fade;
        HdOgBackgroundRect background_rect;
        HdOgBackground *background;
    };
};

struct HdSnapshot
{
    HdOgDraw *og_draws;
    uint32 og_count, og_capacity;
    HdWorldDraw *world_draws;
    uint32 world_count, world_capacity;
    VID_VERTEX2D *ui_vertices, *sprite_vertices;
    uint32 ui_count, ui_capacity, sprite_count, sprite_capacity;
    HdUiPass *ui_passes;
    uint32 pass_count, pass_capacity;
    uint32 clear_color, vram_version, image_version;
    uint64 content_version, sprite_content_version;
    float ui_half_width;
    bool valid, restore_image;
    HdPoseSample *nodes;
    uint32 node_count, node_capacity;
    HdCamera camera;
    const SceneSection *scene;
    uint64 epoch, camera_epoch, collection, clock_index;
    bool camera_valid;
    bool pool_valid[HD_POOL_SLOTS];
    HdBackdrop backdrop;
};

struct HdState
{
    HdOgDraw *og_draws;
    uint32 og_count, og_capacity;
    bool og_background_skipped;
    HdOgDraw *frozen_og_draws;
    uint32 frozen_og_count, frozen_og_capacity;
    bool frozen_og_ui_valid;
    HdScrollClock *scroll_clocks;
    uint64 scroll_epoch;
    VID_TARGET *live_image, *saved_image;
    VID_GEOMETRY *image_mesh;
    VID_MATERIAL *image_material;
    VID_SHADER *image_shader;
    bool last_hd, image_valid, restore_image, paused;
    SceneSection *pause_scene;
    HdSceneRedraw pause_redraw;
    sint32 pause_width, pause_height;
    bool pause_valid, pause_hd, pause_collect;
    VID_SHADER *ui_shader;
    VID_SHADER *world_shader;
    VID_SHADER *sprite_shader;
    VID_TEXTURE *vram;
    VID_TEXTURE *empty_morph;
    VID_MATERIAL *ui_materials[4];
    VID_MATERIAL *world_materials[4];
    VID_MATERIAL *sprite_materials[4];
    VID_GEOMETRY *sprite_mesh;
    VID_VERTEX2D *sprite_vertices;
    uint32 sprite_count, sprite_capacity, sprite_mesh_capacity;
    HdMesh *meshes, *retired_meshes;
    VID_GEOMETRY *world_mesh;
    VID_INDEX *world_indices;
    uint32 world_mesh_capacity, world_index_count, world_vertex_count;
    HdPoolRange *pool_ranges;
    bool world_pool_dirty;
    VID_INDEX *visible_indices;
    uint32 visible_capacity, visible_count;
    VID_TEXTURE *world_params, *world_groups;
    uint32 world_param_height, world_group_height;
    VID_SHADER *pooled_shader;
    VID_MATERIAL *pooled_material;
    VID_SHADER *world_pool_shader;
    VID_MATERIAL *world_pool_materials[4];
    HdVisibleDraw *visible_draws;
    uint32 visible_draw_capacity;
    HdWorldDraw *world_draws;
    uint32 world_count, world_capacity;
    VID_GEOMETRY *ui_mesh;
    VID_VERTEX2D *ui_vertices;
    HdUiPass *ui_passes;
    uint32 ui_count;
    uint32 ui_capacity;
    uint32 pass_count;
    uint32 pass_capacity;
    uint32 mesh_capacity;
    uint32 vram_version;
    uint32 clear_color, image_version, latest_sample;
    uint64 next_content_version, content_version, ui_version, sprite_version;
    uint64 live_collection;
    uint64 sprite_content_version;
    HdSnapshot samples[2];
    HdBackdrop backdrop;
    HdPose *poses;
    uint32 pose_depth, pose_capacity;
    const AnimatedObject *pose_owner;
    HdPathPose *paths;
    uint32 path_count, path_capacity, graph_depth;
    bool pose_active;
    HdGraphWork *graph_work;
    uint32 graph_work_count, graph_work_capacity;
    HdIdentity *identities;
    uint32 identity_count, identity_capacity;
    HdAnimationEntry *animations;
    uint32 animation_count, animation_capacity;
    uint64 identity_serial, pose_epoch, collection;
    HdPoseSample *nodes;
    uint32 node_count, node_capacity;
    const SceneSection *scene;
    uint32 draw_pose;
    uint64 clock_index;
    HdWorldDraw *mixed_draws;
    uint32 mixed_draw_capacity;
    VID_VERTEX2D *mixed_sprites;
    uint32 mixed_sprite_capacity;
    HdMixedPose *mixed_poses;
    uint32 mixed_pose_capacity;
    HdPoseIndex *previous_index, *current_index;
    uint32 index_capacity;
    const SpriteRecord *sprite_pool;
    bool pool_valid[HD_POOL_SLOTS];
    uint64 camera_epoch;
    uint64 camera_sample_epoch;
    HdPreciseCamera precise_camera;
    float ui_half_width;
    bool vram_valid;
    bool frame_active;
};

struct HdWorldPass
{
    VID_MATERIAL *material;
    VID_FOG fog;
};

extern HdState hd_state;
void hd_filter_material(VID_MATERIAL *material);
void hd_filter_keys(void);
extern const char hd_ui_vertex[];
extern const char hd_world_vertex[];
extern const char hd_sprite_vertex[];
extern const char hd_texture_fragment[];

VECTORF hd_animation_vector(const AnimationTrack *track, float frame);
void hd_animation_release(HdAnimationChannel *channel);
void hd_animation_retain(HdAnimationChannel *channel);
void hd_animation_nodes_release(HdPoseSample *nodes, uint32 count);
bool hd_camera_aim_valid(void);
VID_SHADER *hd_variant_shader(const char *prefix);
void hd_upload_vram(void);
bool hd_og_material_equal(const HdOgMaterial *copy, const TextureDescriptor *texture);
void hd_og_material_copy(HdOgMaterial *copy, const TextureDescriptor *texture);
void hd_mesh_free(HdMesh *mesh);
void hd_free_retired(void);
void hd_forget_animation(const SceneNode *node);
void hd_clear_identities(void);
void hd_og_release(HdOgDraw *draws, uint32 count);
void hd_og_retain(HdOgDraw *draws, uint32 count);
void hd_sample_clear(HdSnapshot *sample);
void hd_clear_samples(void);
void *hd_grow_sample(void *data, uint32 *capacity, uint32 count, size_t stride);
bool hd_og_rebuild_background(const HdSnapshot *sample, uint32 base, uint32 limit);
void hd_og_save_ui(const HdSnapshot *sample);
HdScroll hd_scroll_source(const TextureDescriptor *texture);
bool hd_scroll_pair(const HdScroll *previous, const HdScroll *current);
bool hd_capture_sample(HdSnapshot *sample);
float hd_angle_delta(float from, float to);
HdWorldDraw *hd_new_world_draw(void);
void hd_source_clip(const SceneSection *scene, HdCamera *camera);
const VID_FOG *hd_source_fog(const SceneSection *scene, const HdCamera *camera, VID_FOG *fog);
void hd_select_lights(const SceneLighting *lighting, const VECTORF *origin, float radius, SceneLighting *selected);
void hd_draw_lights(uint32 first, const SceneLighting *source, const MATRIXF *model);
bool hd_submit_world(void);
bool hd_pose_world(const HdSnapshot *sample, uint32 index, MATRIXF *world);
bool hd_mix_poses(const HdSnapshot *previous, const HdSnapshot *current, float fract);
HdCamera hd_mix_camera(const HdCamera *previous, const HdCamera *current, float fract);
void hd_mix_draw(HdWorldDraw *draw, const MATRIXF *model, const HdCamera *camera);
bool hd_prepare_morph(HdMesh *mesh);
HdMesh *hd_previous_source(const HdSnapshot *previous, const HdWorldDraw *draw, uint32 pose);
HdMesh *hd_previous_mesh(const HdSnapshot *previous, const HdWorldDraw *draw, uint32 pose);
bool hd_mix_sprites(const HdSnapshot *previous, const HdSnapshot *current, float fract, HdSnapshot *view);
bool hd_render_sample(const HdSnapshot *sample, const HdSnapshot *previous);
void hd_source_model(const MATRIX *source, MATRIXF *model);
bool hd_reserve_poses(uint32 depth);
void hd_pose_camera_scratch(const MATRIXF *model);

#endif
