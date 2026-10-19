#ifndef SCENE_SECTION_H
#define SCENE_SECTION_H

#include "psx.h"
#include "object_links.h"
#include "material_resources.h"

struct ActorBinding;
struct AnimatedPathState;
struct BackgroundResource;
struct SceneSequenceBundle;
struct MaterialRecord;
struct MeshLighting;
struct ScenePath;
struct PathTransform;
struct SceneLight;
struct MeshGeometryNode;
struct MeshFace;
struct MeshVertex;
union CollisionTransform;
struct AnimationHeader;
struct PositionedBillboardNode;
struct MeshPartNode;
struct SphereNode;
struct ActorPlacement;
struct AnimatedObject;
struct SceneTrigger;

struct SceneObjectName
{
    char prefix[2];
    uint8 kind;
};

typedef char SceneObjectNameBinaryLayout[(sizeof(SceneObjectName) == 3) ? 1 : -1];

// Native section state borrows immutable bytes from its section buffer
struct SceneSection
{
    ObjectLinks links;
    uint8 kind;
    uint8 flags;
    sint8 section_index;
    sint8 palette_bank;
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
    CollisionTransform *collision_storage;
    uint32 collision_count;
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
    SpriteRecord *sprites;
    uint32 sprite_count;
    // Own independent sprite extent tracks with their immutable key payloads
    AnimationTrack **sprite_extent_tracks;
    uint32 sprite_extent_track_count;
    // Own unique zones; the auxiliary registry shares these records
    SceneTrigger *triggers;
    uint32 trigger_count;
    // Own contiguous runtime state arrays in original list order
    ActorBinding *actor_bindings;
    AnimatedPathState *animated_paths;
    uint16 collected_count;
    uint16 metadata_halfword;
    ObjectLinks transient_links;
    uint32 runtime_words[3];
    BackgroundResource *background;
    // Borrow the stream-owned bundle only when a sequence was loaded
    SceneSequenceBundle *sequence_bundle;
    uint32 resource_identity;
    uint32 background_top_color;
    uint32 background_bottom_color;
    uint32 origin[3];
    MaterialResourceHeader materials;
    MeshLighting *lighting_records;
    uint32 lighting_count;
    // Own packed path keys and their easing prefixes
    uint8 *path_key_data;
    // Own mutable offset matrices shared by path references
    MATRIX *path_offset_matrices;
    // Own mutable definition matrices and surface displacement
    PathTransform *definition_transforms;

    void Relocate(const MaterialResourceHeader *resource);
    uint32 Activate();
    sint32 FindObjectId(const SceneObjectName *name) const;
    const ScenePath *FindPath(sint32 index) const;
    const ScenePath *FindDefaultPath() const;
};

extern "C" uint32 GetSectionRespawnForm(const SceneSection *section);

#endif
