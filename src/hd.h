#ifndef HD_H
#define HD_H

#include "vid.h"
#include "psx.h"

struct TextureDescriptor;
union Vec3;
struct CameraProjection;
struct MeshVertex;
struct MeshFace;
struct HdMesh;
struct SceneLighting;
struct SceneSection;
struct AnimatedObject;
struct MorphSpan;
struct BackgroundResource;
struct BackgroundTile;
struct SpriteRecord;
struct BillboardNode;
struct AnimationTrack;
struct SceneNode;
struct PathTransform;
typedef void (*HdSceneRedraw)(SceneSection *scene, bool advance);

struct HdCamera
{
    MATRIXF view;
    float focal;
    float near_z;
    float far_z;
};

struct HdLight
{
    VECTORF dir;
    VECTORF color;
    VECTORF pos;
    VECTORF color_scale;
    float radius;
    float attenuation_range;
    bool point;
};

struct HdLighting
{
    HdLight lights[3];
    VECTORF ambient;
};

struct HdTexture
{
    UV uv[4];
    uint16 page, clut;
};

enum HdTextureFlags
{
    HD_TEXTURED = 1,
    HD_RAW_TEXTURE = 2,
    HD_STP_ONLY = 4,
    HD_OPAQUE_ONLY = 8,
    HD_UI_COORDS = 16,
    HD_UI_LEFT = 32,
    HD_UI_RIGHT = 64,
    HD_UI_FILL = 128,
    HD_TWO_SIDED = 256,
    HD_IMAGE_OPAQUE = 512
};

enum HdUiAnchor
{
    HD_ANCHOR_CENTER,
    HD_ANCHOR_LEFT,
    HD_ANCHOR_RIGHT,
    HD_ANCHOR_FILL
};

enum HdUiLayer
{
    HD_BACKGROUND,
    HD_SCENE_FADE,
    HD_OVERLAY,
    HD_UI_LAYERS
};

enum HdSpriteMode
{
    HD_BILLBOARD,
    HD_GROUND_SPRITE,
    HD_FACING_SPRITE
};

struct HdSprite
{
    VECTORF pos;
    float width, height;
    uint16 pitch, yaw;
    uint8 orientation;
    HdSpriteMode mode;
};

struct HdFrameState
{
    HdCamera camera;
    bool active, camera_valid, failed, returning_og, collecting, capture_only;
    float aspect;
};

extern HdFrameState hd_frame;

typedef uint32 (*HdOgQuadEmit)(uint32 pos, uint32 extent, const TextureDescriptor *texture, uint32 blend);
typedef void (*HdOgRectEmit)(uint32 pos, uint32 extent, sint32 fraction, uint32 color);
typedef uint32 (*HdOgEffectEmit)(sint32 intensity);
typedef void (*HdOgFadeEmit)(void);
typedef void (*HdOgBackgroundRectEmit)(uint32 pos, uint32 extent, uint32 color);

// Own semantic UI inputs for reconstruction through their original OG function
bool hd_og_textured_quad(uint32 pos, uint32 extent, const TextureDescriptor *texture, uint32 blend, HdOgQuadEmit emit);
bool hd_og_rect(uint32 pos, uint32 extent, sint32 fraction, uint32 color, HdOgRectEmit emit);
bool hd_og_effect(sint32 intensity, HdOgEffectEmit emit);
bool hd_og_scene_fade(uint32 level, HdOgFadeEmit emit);
bool hd_og_background_rect(uint32 pos, uint32 extent, uint32 color, HdOgBackgroundRectEmit emit);
void hd_og_clear_background(void);
bool hd_og_skip_background(void);
bool hd_og_background_pending(void);
void hd_og_background_state(void);
bool hd_og_background_tiles(const BackgroundResource *background, const uint8 *map, sint32 row, sint32 column, sint32 x, uint32 y, sint32 rows, sint32 columns);
void hd_og_emit_background_tile(uint32 tile, uint32 xy, uint32 next_xy, const BackgroundTile *definition);

void hd_frame_begin(void);
bool hd_frame_end(void);
// Render the completed pair while its aspect and image generations match
bool hd_replay(void);
void hd_wait(uint64 deadline);
void hd_frame_camera(const Vec3 *pos, const CameraProjection *projection, const SceneSection *scene = NULL);
void hd_root_pose(const AnimatedObject *actor, MATRIXF *model);
void hd_node_pose(const SceneNode *node, const AnimatedObject *owner, const MATRIXF *parent, MATRIXF *model);
void hd_frame_root(const AnimatedObject *actor);
void hd_frame_root_end(void);
void hd_frame_scale(float scale);
void hd_frame_push(const MATRIX *initial);
void hd_frame_node(const SceneNode *node, const AnimatedObject *owner);
void hd_frame_pop(void);
void hd_graph_pose(const SceneNode *node, const MATRIXF *parent, bool inherit_sprite, const Vec3 *heading, MATRIXF *model);
void hd_frame_graph_begin(const MATRIX *initial);
void hd_frame_graph_node(const SceneNode *node, const AnimatedObject *parent, uint32 work, const Vec3 *heading = NULL);
void hd_frame_graph_path(const PathTransform *path);
void hd_frame_graph_end(void);
uint32 hd_graph_work(const SceneNode *node, uint32 work, bool advance);
void hd_invalidate_paths(void);
void hd_sprite_track_loop(const AnimatedObject *owner, const AnimationTrack *track);
void hd_forget_pose(const SceneNode *node, bool children = true);
void hd_sprite_pool(const SpriteRecord *records);
void hd_camera_cut(void);
void hd_frame_actor(const AnimatedObject *actor, const MorphSpan *span, const SceneLighting *lighting, const TextureDescriptor *material, float depth_bias = 0.0f, bool flat_color = false, uint32 color = 0);
void hd_frame_sprite(const SpriteRecord *sprite, float scale, const TextureDescriptor *material, const SceneLighting *lighting, const MATRIX *scratch, float depth_bias = 0.0f);
void hd_frame_billboard(const BillboardNode *billboard, const MATRIX *view, float depth_bias = 0.0f);
void hd_frame_rect(uint32 pos, uint32 extent, uint32 color, uint32 blend, HdUiLayer layer);
void hd_save_image(void);
void hd_restore_image(const PSX_RECT *source = NULL);
void hd_menu_image_changed(void);
void hd_scroll_update(const TextureDescriptor *texture, uint32 frame, sint32 rate);
void hd_scroll_reset(void);
void hd_pause_begin(SceneSection *scene, HdSceneRedraw redraw);
void hd_pause_end(void);
void hd_overlay_begin(void);

bool hd_init(void);
bool hd_camera(const Vec3 *pos, const CameraProjection *projection, float near_z, HdCamera *camera);
bool hd_visible(const HdCamera *camera, const VECTORF *pos, float radius, float aspect);
bool hd_texture(const TextureDescriptor *texture, HdTexture *result);
void hd_lighting(const SceneLighting *source, const MATRIXF *model, const VECTORF *origin, HdLighting *lighting);
void hd_reset(void);
void hd_level_reset(void);
void hd_invalidate_geometry(void);
void hd_free(void);
bool hd_begin(uint32 color);
bool hd_ui(const VID_VERTEX2D *vertices, uint32 count, uint32 blend);
bool hd_textured_quad(uint32 pos, uint32 extent, const TextureDescriptor *texture, uint32 blend, HdUiAnchor anchor, HdUiLayer layer);
bool hd_rect(float x, float y, float width, float height, uint32 color, uint32 blend, HdUiAnchor anchor, HdUiLayer layer);
bool hd_vram_image(const PSX_RECT *source);
HdMesh *hd_mesh(const MeshVertex *vertices, uint32 vertex_count, uint32 first_vertex, const MeshFace *faces, uint32 face_count, const TextureDescriptor *material);
bool hd_mesh_update(HdMesh *mesh);
bool hd_world(HdMesh *mesh, const MATRIXF *model, const HdCamera *camera, const HdLighting *lighting, const VID_FOG *fog, bool original_planes = false);
bool hd_scene(const SceneSection *scene, const HdCamera *camera, const SceneLighting *lighting, const VID_FOG *fog);
bool hd_actor(const AnimatedObject *actor, const MorphSpan *span, const MATRIXF *model, const HdCamera *camera, const SceneLighting *lighting, const TextureDescriptor *material, const VID_FOG *fog);
bool hd_background(const BackgroundResource *background, const uint16 *map, sint32 pitch, sint32 yaw, sint32 pitch_offset, uint32 scroll, uint32 top_color, uint32 bottom_color);
bool hd_sprite(const HdSprite *sprite, const TextureDescriptor *texture, const HdCamera *camera, const VID_FOG *fog, const HdLighting *lighting = NULL, const VECTORF *normal = NULL);
bool hd_sprite_record(const SpriteRecord *sprite, const MATRIXF *model, const HdCamera *camera, float scale, const TextureDescriptor *material, const VID_FOG *fog, const SceneLighting *lighting = NULL);
bool hd_billboard_node(const BillboardNode *billboard, const VECTORF *pos, const HdCamera *camera, const VID_FOG *fog);
bool hd_end(void);

#endif

