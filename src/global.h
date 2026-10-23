#ifndef GLOBAL_H
#define GLOBAL_H
#include <stddef.h>

#include "psx.h"
#include "animated_object.h"
#include "material_catalog.h"
#include "mesh_geometry_node.h"
#include "sound_table_header.h"
#include "material_animation_list.h"
#include "collision.h"

extern uint64 game_field;
extern uint32 game_tick, menu_tick;

void InitializeGameClock(void);
void ShutdownGameClock(void);
uint64 CurrentField(void);
uint64 WaitGameFrame(uint64 previous);

#define mul12(a, b) (((sint64)(a) * (b)) >> 12)
#define mul14(a, b) (((sint64)(a) * (b)) >> 14)

struct AnimatedObject;
struct AnimationHeader;
struct MeshFace;
struct MeshVertex;

struct ActionEffectResources;

struct ActionEffectResources
{
    enum
    {
        Count = 15
    };

    AnimatedObject *templates[Count];
    uint8 *model_data[Count];
    uint32 model_bytes[Count];
    MeshFace *model_polygons[Count];
    MeshVertex *model_vertices[Count];
    MaterialCatalog material_resources[Count];
    uint8 *animation_data[Count];
    uint32 animation_bytes[Count];
    AnimationHeader *animation_headers[Count];
};

extern ActionEffectResources action_effect_resources;

class ControlBindings;

class ControlBindings
{
public:
    typedef uint8 Scheme[4];
    const Scheme *active_scheme;
    uint8 sel_idx;
};

extern ControlBindings control_bindings;

struct MenuArchive;

// Own the parent file while the material child retains its encoded graph
struct MenuArchive
{
    uint8 *data;
    uint32 bytes;
    uint32 capacity;
};

extern MenuArchive menu_archive;

struct ObjectCallback;
struct ObjectOperations;

struct ObjectCallback
{
    typedef void (*PlainHandler)();
    typedef intptr (AnimatedObject::*MemberHandler)();
    typedef AnimatedObject *(*FactoryHandler)(AnimatedObject *);
    typedef void (AnimatedObject::*VoidMemberHandler)();
    PlainHandler plain_handler;
    MemberHandler member_handler;
    FactoryHandler factory_handler;
    VoidMemberHandler void_member_handler;
};

struct ObjectOperations
{
    ObjectCallback behaviors[34];
    ObjectCallback motions[7];
    ObjectCallback variants[5];
    CollisionHandler collisions[5];
};

extern ObjectOperations object_operations;

struct PathRouting;

struct PathRouting
{
    uint32 flags;
    uint8 fallback_phase;
    sint8 reject_repeated;
};

extern PathRouting path_routing;

struct AnimatedObject;
struct ScenePath;
union Vec3;

struct PathTransitionRules;

struct PathTransitionRules
{
    uint8 follow_enabled;
    uint32 follow_height;
    uint32 follow_side_offset;
    uint8 animation_tail_frames;
    sint8 follow_limit;
    sint8 follow_side_ticks;
    uint8 reentry_ticks;
    sint8 exit_ticks;
    uint8 special_ticks;
    sint8 camera_duration;
    uint8 wait_animation_start;
    uint32 side_step;
    uint32 height_bias;
    sint8 release_ticks;
    sint8 exit_hold_ticks;
    uint16 angle_limit;
    uint32 approach_limit;
    uint32 min_height;
    uint32 max_height;
    uint32 air_height_limit;
    uint8 early_movement_ticks;
    sint8 release_mode;
    sint32 release_speed_scale;
    sint8 hold_motion_limit;
    // Q12 factors for released and constrained movement
    sint32 release_motion_scale;
    sint32 axis_horizontal_acceleration[2];
    sint32 axis_vertical_limits[2];
    uint8 axis_input_ticks;
    sint32 axis_speed_limit_scale;
};

extern const PathTransitionRules path_transition_defaults;

extern PathTransitionRules path_transition_rules;

struct CollisionSphere;
struct SceneSection;
struct TextureDescriptor;

struct FrustumPlane;
struct RenderMatrixSlot;
struct RenderViewState;

struct FrustumPlane
{
    uint32 bias;
    SVECTOR n;
};

struct RenderMatrixSlot
{
    MATRIX matrix;
    RenderMatrixSlot *prev;
    RenderMatrixSlot *next;
    uint32 depth;
};

struct RenderViewState
{
    enum
    {
        ProjectionCapacity = 65536
    };

    RenderMatrixSlot matrix_root;
    RenderMatrixSlot *matrix_cursor;
    uint32 fog_color;
    uint32 far_distance;
    uint32 fog_scale;
    uint32 depth_scale;
    uint32 object_scale;
    uint32 scale_rcp;
    ProjectedVertex proj_vertices[ProjectionCapacity];
    uint16 first_proj_vertex;
    FrustumPlane planes[5];
    MeshLighting *lighting_cache;
    bool lighting_enabled;
    const TextureDescriptor *material_override;
    sint16 object_depth_bias;
    CollisionSphere *sphere_output;
    uint8 special_transform;
    VECTOR transform_result;
    MATRIX camera_transform;
    MATRIX inverse_transform;
    MATRIX billboard_transform;
};

typedef char FrustumPlaneSize[(sizeof(FrustumPlane) == 12) ? 1 : -1];

typedef char FrustumPlaneNormalOffset[(offsetof(FrustumPlane, n) == 4) ? 1 : -1];

extern RenderViewState render_view;

struct SoundResources;

struct SoundResources
{
    uint32 transfer_address;
    const SoundBankStorage *common_table;
    const SoundBankStorage *scene_table;
    SoundBankStorage common_bank;
    SoundBankStorage scene_bank;
};

extern SoundResources sound_resources;

struct SceneNode;
struct SpriteRecord;

struct SpritePoolEntry;
struct SpritePoolState;

// Own pooled material playback separately from borrowed graph records
struct SpritePoolEntry
{
    SceneNode *node;
    MaterialAnimationList animation;
};

struct SpritePoolState
{
    enum
    {
        EntryCount = 100,
        AnimationCapacity = 3
    };

    SceneNode *first;
    SceneNode *free_head;
    SceneNode *active_head;
    SpriteRecord *recs;
    SpritePoolEntry entries[EntryCount];
};

extern SpritePoolState spr_pool;

extern CollisionContact col_contact;
extern CollisionTransform col_transform;
extern CharacterGeometry character_geometry;
extern TriggerContactMotion trigger_contact_motion;

#endif
