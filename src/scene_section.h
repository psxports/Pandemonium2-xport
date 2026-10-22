#ifndef SCENE_SECTION_H
#define SCENE_SECTION_H
#include <stddef.h>

#include "psx.h"
#include "material_catalog.h"
#include "object_links.h"

struct ActorBinding;
struct ActorPlacement;
struct AnimatedObject;
struct AnimatedPathState;
struct AnimationHeader;
struct AnimationTrack;
struct BackgroundResource;
struct CollisionTransform;
struct MeshFace;
struct MeshGeometryNode;
struct MeshLighting;
struct MeshPartNode;
struct MeshVertex;
struct PathTransform;
struct PositionedBillboardNode;
struct SceneLight;
struct SceneNode;
struct SceneObjectName;
struct ScenePath;
struct SceneSequenceBundle;
struct SceneTrigger;
struct SphereNode;
struct SpriteRecord;

struct EncodedSectionHeader;
struct SceneSection;

// Serialized header precedes the section payload
struct EncodedSectionHeader
{
    uint32 registry_offset;
    uint32 sibling_offset;
    uint32 child_offset;
    uint8 kind;
    uint8 flags;
    sint8 section_idx;
    sint8 pal_bank;
    sint16 transition_scale;
    uint16 state_flags;
    uint32 data_offset;
    uint32 metadata;
    uint32 object_offsets[16];
    uint32 lighting_offset;
    uint16 collected_count;
    uint16 metadata_halfword;
    uint32 transient_registry;
    uint32 transient_sibling;
    uint32 transient_child;
    uint32 reserved112;
    uint16 ambient[4];
    uint32 background_offset;
    uint32 sequence_offset;
    uint32 resource_identity;
    uint32 background_top_color;
    uint32 background_bottom_color;
    uint32 far_distance;
    uint32 fog_near;
    uint32 fog_far;
    EncodedMaterialResourceHeader materials;
};

typedef char EncodedSectionHeaderSize[(sizeof(EncodedSectionHeader) == 188) ? 1 : -1];

// Native section state borrows immutable bytes from its section buffer
struct SceneSection
{
    ObjectLinks links;
    uint8 kind;
    uint8 flags;
    sint8 section_idx;
    sint8 pal_bank;
    sint16 transition_scale;
    uint16 state_flags;
    uint8 *data;
    uint32 data_bytes;
    uint32 metadata;
    SceneNode *object_heads[16];
    // Own original path names in registry order
    SceneObjectName *path_names;
    uint32 path_name_count;
    // Own mutable path headers independently of their resource records
    ScenePath *paths;
    uint32 path_count;
    // Own mutable light headers separately from the resource payload
    SceneLight *lights;
    uint32 light_count;
    // Own mutable mesh headers separately from vertex and face payloads
    MeshGeometryNode *meshes;
    uint32 mesh_count;
    // Own merged polygon ranges shared by scene geometry
    MeshFace *face_storage;
    uint32 face_count;
    // Own merged vertex ranges including shared morph destinations
    MeshVertex *vertex_storage;
    uint32 vertex_count;
    // Own cached transforms shared by collision and animation views
    CollisionTransform *col_storage;
    uint32 col_count;
    // Own mutable material lists including their packed render start ticks
    uint32 *material_animation_words;
    uint32 material_animation_word_count;
    // Own immutable keys, easing prefixes and source morph frames
    uint8 *animation_payload_data;
    uint32 animation_payload_bytes;
    // Own animation headers after all track and payload pointers are resolved
    AnimationHeader *animations;
    uint32 animation_count;
    // Own positioned billboards independently of the scene payload
    PositionedBillboardNode *billboards;
    uint32 billboard_count;
    // Own mutable child geometry and collision sphere headers
    MeshPartNode *mesh_parts;
    uint32 mesh_part_count;
    SphereNode *sphere_nodes;
    uint32 sphere_node_count;
    // Own placement headers with their native state and path references
    ActorPlacement *placements;
    uint32 placement_count;
    // Own scene root headers separately from model resource payloads
    AnimatedObject *animated_objects;
    uint32 animated_object_count;
    // Own resource sprites independently of the reusable effect pool
    SpriteRecord *sprs;
    uint32 spr_count;
    // Own independent sprite extent tracks with their immutable key payloads
    AnimationTrack **spr_extent_tracks;
    uint32 spr_extent_track_count;
    // Own unique zones; the auxiliary registry shares these records
    SceneTrigger *triggers;
    uint32 trigger_count;
    // Own contiguous runtime state arrays in original list order
    ActorBinding *actor_bindings;
    // Source list offset consumed by Relocate
    uint32 actor_source_offset;
    AnimatedPathState *animated_paths;
    uint16 collected_count;
    uint16 metadata_halfword;
    ObjectLinks transient_links;
    uint16 ambient[4];
    BackgroundResource *background;
    // Borrow the stream-owned bundle only when a sequence was loaded
    SceneSequenceBundle *sequence_bundle;
    uint32 resource_identity;
    uint32 background_top_color;
    uint32 background_bottom_color;
    // Camera range and fog depth interval
    uint32 far_distance;
    uint32 fog_near;
    uint32 fog_far;
    MaterialCatalog materials;
    MeshLighting *lighting_recs;
    uint32 lighting_count;
    // Own packed path keys and their easing prefixes
    uint8 *path_key_data;
    // Own mutable offset matrices shared by path references
    MATRIX *path_offset_matrices;
    // Own mutable definition matrices and surface displacement
    PathTransform *definition_transforms;

    const ScenePath *FindPath(sint32 idx) const;
    sint32 FindObjectId(const SceneObjectName *name) const;
    const ScenePath *FindDefaultPath() const;
    static uint32 GetSectionRespawnForm(const SceneSection *section);
    void Activate();
    static void UpdateSceneObjects(void);
    void Relocate(const MaterialCatalog *resource);
};

#endif
