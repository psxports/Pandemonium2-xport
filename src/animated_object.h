#ifndef ANIMATED_OBJECT_H
#define ANIMATED_OBJECT_H
#include <stddef.h>

#include "psx.h"
#include "animation_track.h"
#include "collision.h"
#include "mesh_geometry_node.h"
#include "quaternion16.h"
#include "scene_node.h"
#include "vector3.h"

struct AnimatedModelDefinition;
struct CollisionSphere;
struct MaterialCatalog;
struct MeshFace;
struct MeshVertex;
struct SceneNode;
union Vec3;

struct ActorBinding;
struct ActorModelDefinition;
struct AnimationHeader;
struct AttackScript;
struct CharacterController;
struct MaterialCatalog;
struct MorphRestorationHeader;
struct PathDefinition;
struct ScenePath;
struct SceneTrigger;
struct SpriteRecord;
struct VolleyScript;

enum ModelStorageKind
{
    ModelStorageRoot = 256,
    ModelStorageMaterialAnimations,
    ModelStorageBytes
};

struct ModelStorageRecord;
struct ModelDecodeLayout;
struct ModelGroupContacts;
struct ModelPlaybackState;
struct EncodedAnimatedObject;
struct AnimatedObject;

struct ModelStorageRecord
{
    uint32 src_offset;
    uint32 src_bytes;
    uint32 storage_offset;
    uint32 storage_bytes;
    uint32 kind;
};

struct ModelDecodeLayout
{
    const uint8 *input;
    uint32 input_bytes;
    uint32 src_clone_bytes;
    uint8 *storage;
    uint32 storage_bytes;
    uint8 *legacy_data;
    const ModelStorageRecord *recs;
    uint32 rec_count;
};

// Transient contacts borrow their scene nodes
struct ModelGroupContacts
{
    SceneNode *blocker;
    SceneTrigger *surface0;
    SceneTrigger *surface1;
    uint32 fraction1;
    SceneTrigger *surface2;
    uint32 fraction0;
    uint32 fraction2;
};

// Animated model state shares the two spatial records with its collision representation
struct ModelPlaybackState
{
    CollisionSphere root;
    CollisionSphere prev;
    uint32 flags;
    uint32 frame;
};

struct EncodedAnimatedObject
{
    EncodedSceneNode node;
    Vec3 pos;
    uint32 flags;
    uint32 geometry_counts;
    uint32 polygon_word;
    uint32 vertices_offset;
    uint32 materials_offset;
    uint16 lighting_idx;
    MorphSpan mesh_span;
    uint16 resource_halfword58;
    Vec3 world_pos;
    ObjectBounds bounds;
    Quat rotation;
    Vec3 velocity;
    uint32 animation_offset;
    uint32 event_word;
    uint32 behavior_data;
    uint32 state_word;
    AnimationTrackBinding animation_binding;
    sint8 movement_mode;
    sint8 behavior;
    uint8 contact_kind;
    uint8 behavior_suspended;
    sint16 ticks;
    sint16 frame;
    uint32 definition_offset;
    sint16 path_frame;
    uint16 sound_voice;
    uint32 mesh_offset;
    uint32 motion_scale;
    sint16 yaw;
    sint16 target_yaw;
    sint16 pitch;
    sint16 pitch_scale;
    uint32 sprite_offset;
    CollisionSphere spheres[4];
    uint32 clone_bytes;
    uint32 path_flags;
};

typedef char EncodedAnimatedObjectSize[(sizeof(EncodedAnimatedObject) == 232) ? 1 : -1];

struct AnimatedObject
{
    SceneNode node;
    Vec3 pos;
    uint32 flags;
    sint16 face_count;
    sint16 vertex_count;

    union
    {
        MeshFace *polygons;
        uint32 action_ox;
    };

    union
    {
        MeshVertex *vertices;
        uint32 action_oy;
    };

    union
    {
        uint32 *material_animation_entries;
        uint32 action_oz;
    };

    uint16 lighting_idx;
    MorphSpan mesh_span;
    uint16 resource_halfword58;
    Vec3 world_pos;

    ObjectBounds bounds;

    union
    {
        sint16 orientation[4];
        Quat rotation;
    };

    Vec3 velocity;
    AnimationHeader *animation;

    union
    {
        uint32 event_word;
        AnimatedObject *linked_object;
    };

    // The active behavior selects the timer, contact, script command or borrowed context
    union
    {
        uint32 behavior_data;
        uint32 state_delay;
        uint32 detour_target;
        uint32 surface_contact;
        uint32 script_mode;
        uint32 event_command;
        const ScenePath *scene_path;
        const ActorBinding *action_context;
    };

    union
    {
        uint32 state;
        AnimatedObject *follow_target;
    };

    AnimationTrackBinding animation_binding;
    sint8 movement_mode;
    sint8 behavior;
    sint8 contact_kind;
    sint8 behavior_suspended;
    sint16 ticks;
    sint16 frame;

    // The owning context selects a path definition or a scene node
    union
    {
        const PathDefinition *path_definition;
        SceneNode *owner_node;
    };

    sint16 path_frame;

    // The same halfword counts cached transforms for collidable geometry
    union
    {
        uint16 sound_voice;
        sint16 col_transform_count;
    };

    // Shared by morph restoration and cached collision transforms
    union
    {
        void *mesh_state;
        MorphRestorationHeader *morph_restoration;
        CollisionTransform *col_transforms;
    };

    union
    {
        uint32 path_motion_scale;
        uint32 death_ticks;
    };

    union
    {
        sint16 yaw;
        uint16 scale;
    };

    sint16 target_yaw;
    sint16 pitch;
    sint16 pitch_scale;
    SpriteRecord *spr;

    // The original reuses this region for collisions, saved state and animation work
    union
    {
        CollisionSphere spheres[4];
        ModelPlaybackState playback;
    };

    ModelGroupContacts model_contacts;

    uint32 clone_bytes;
    uint32 path_flags;
    // Native storage span for private clone state
    uint32 clone_storage_bytes;
    // Preserve the encoded identity until the first animation binding
    uint32 unbound_animation_word;

    void DrawGeometry(const MorphSpan *span) const;
    void UpdateObjectBounds();
    sint16 ComputeObjectFacing(const Vec3 &dir_vector, Quat *axes) const;
    sint16 ComputeObjectPitch(const Vec3 &dir, Quat *axes) const;
    sint16 SampleObjectPitch(const Vec3 &dir_vector, Quat *quat) const;
    void RebuildMorphNormals(const MorphSpan *span) const;
    void SampleNodeAnimation(SceneNode *node);
    uint32 ClampAnimationFrame();
    static void ApplyNodeAnimation(AnimatedObject *parent, SceneNode *node);
    uint32 AdvanceTracks(sint32 step);
    CollisionSphere *BuildCharacterGeometry();
    void GetCharacterSphereEnds(uint32 sphere, uint8 *first, uint8 *last_output) const;
    uint32 AcceptVerticalFaceContact(const CollisionTransform *desc, uint32 selection) const;

private:
    void ResolveSurfaceMotion(uint32 *move_flags);
    void TranslateCharacterWithBounds(sint32 x, sint32 y, sint32 z);

public:
    void UpdateCharacterSupportMotion();
    void ResolveCharacterSupport(uint8 grounded);
    void ProbeCharacterSupport();
    uint32 UpdateActorAimedShot();
    uint32 InitializeActorAimedShot();
    uint32 UpdateVolleyEnemy();
    void SpawnVolleyProjectile(const VolleyScript *script, const Vec3 &target, sint32 idx);

private:
    uint32 UpdateVolleyPath(const Vec3 &origin, const Vec3 &target, sint32 distance, ActorModelDefinition *resource_model, const VolleyScript *script);
    uint32 UpdateVolleyFiring(const Vec3 &origin, const Vec3 &target, sint32 distance, ActorModelDefinition *resource_model, const VolleyScript *script);

public:
    uint32 InitializeVolleyEnemy();
    uint32 UpdateAttackEnemy();

private:
    uint32 UpdateAttackRepeat(const Vec3 &origin, const Vec3 &target, sint32 distance, ActorModelDefinition *resource_model, const AttackScript *script);
    uint32 UpdateAttackApproach(const Vec3 &origin, const Vec3 &target, sint32 distance, ActorModelDefinition *resource_model, const AttackScript *script);
    uint32 UpdateAttackAnimation(ActorModelDefinition *resource_model, const AttackScript *script);

public:
    uint32 InitializeAttackEnemy();
    uint32 UpdateActorChaseSpeed();
    uint32 UpdateActorChaseAttack();
    uint32 InitializeActorChaseAttack();
    uint32 UpdateActorVerticalShot();
    uint32 InitializeActorVerticalShot();
    sint32 FindActorPathSeg() const;
    void UpdateActorPathSegMotion();
    uint32 InitializeActorPathSegMotion();
    uint32 UpdateBasicActor();
    uint32 UpdateActorPatrolPause();
    uint32 InitializeActorPatrolPause();
    static void SpawnActionEffect(AnimatedObject *object, const void *context, uint32 action);

private:
    uint32 UpdateInteractionChain(const AnimatedObject *owner);
    uint32 UpdateInteractionWave(const AnimatedObject *owner);

public:
    static uint32 ActionDistance(const Vec3 &v0, const Vec3 &v1);
    uint32 AdvanceActionEffect(const AnimatedObject *owner, uint32 action, uint32 distance, uint32 *progress_output);
    void UpdateActionEffect(const AnimatedObject *owner, uint32 action);
    uint32 UpdateActionObject();

private:
    uint32 ComputePathBoundaryExcess(const Vec3 &pos) const;

public:
    uint32 UpdateForm5Movement();
    uint32 InitializeCommonActor();
    uint32 UpdateCommonActor();

private:
    uint32 GetModelGrpLaunchSpeed() const;

public:
    uint32 StopModelGrpSound();

private:
    uint32 CanModelGrpContact(const SceneNode *other_node) const;

public:
    uint32 CollectModelGrpContact(SceneNode *other_node, CollisionContact *contact_data);
    uint32 UpdateActorVelocity();
    void ApplyModelGrpContactMotion();
    static void RegisterActorVelocityOperations(void);
    sint16 SampleActorPathHeading() const;
    uint32 ReadObjectImpulseScale() const;
    uint32 UpdateActorReactionSpin();
    void UpdateActorShrinkEffect();
    uint32 UpdateActorStateAndTarget(Vec3 &pos, Vec3 &target, uint32 *distance);
    uint32 UpdateActorFacingAndAnimation(const Vec3 *target, const Vec3 &origin, ActorModelDefinition *resource_model);
    uint32 CollectActorContact(SceneNode *other_node, CollisionContact *contact_data);
    uint32 UpdateActorPathMotion();
    uint32 ResetActorInstance();
    uint32 UpdateActorDeath();
    sint32 ClassifyActorPathDir(const Vec3 &pos, const Vec3 &target) const;
    uint32 UpdateActorTurnaround(ActorBinding *binding, const ActorModelDefinition *resource_model);
    uint32 GetActorContactDamage() const;
    static void SampleObjectPathDir(uint16 *angle, Vec3 &dir_vector, const AnimatedObject *object, sint32 frame, uint32 reverse);
    static void CaptureActorRouteDir(uint16 *angle, Vec3 &dir, const AnimatedObject *object);
    void UpdateActorReactionRecovery();
    void InitializeActorAction(ActorBinding *binding);
    uint32 CanActorTurnTowardTarget(const Vec3 &pos, const Vec3 &target, const uint32 *distance, const ActorBinding *binding, const ActorModelDefinition *model, uint8 allowed) const;

private:
    void UpdateActorAction(const Vec3 &pos, Vec3 &target, uint32 *distance);

public:
    ActorBinding *ResetActorAction();
    void CalculateActorTargetMotion(uint32 distance, uint32 *distance_ret, uint32 *height_ret, uint16 *angle_ret) const;
    void CalculateActorPathHeightMotion(uint32 distance, uint32 *distance_ret, uint32 *height_ret, uint16 *angle_ret) const;
    void ReverseObjectFacingFromVelocity();
    sint32 ClassifyActorTargetDir(const Vec3 &pos, const Vec3 &target) const;
    sint32 FindSphereFaceCol(const CollisionSphere *geometry, const SceneNode *other, const CollisionTransform *transform, uint32 *fraction_limit) const;
    static uint32 SweepSphereAgainstObject(const CollisionSphere *bounds, const SceneNode *object, const Vec3 &delta, uint32 *fraction, const CollisionSphere *geometry);
    uint32 TestSweptObjectBounds(const sint32 *pos, const sint32 *velocity, uint32 radius) const;
    void CollideObjectSpheres(AnimatedObject *other_object);
    uint32 CollideObjectWithTrigger(const SceneTrigger *trigger);
    void FindObjectGeometryCol();

private:
    void ProcessObjectTriggerContacts();

public:
    void UpdateObjectCollisions();
    uint32 ChangeActorPath(const ScenePath *path, const Vec3 &pos, Vec3 &floor);

private:
    const ScenePath *FindFallbackPath(const Vec3 &pos, uint32 dir);
    uint32 CanChangePath(const Vec3 &pos, uint32 dir) const;

public:
    uint32 ComputePathMotion(sint32 distance, sint32 height, const Vec3 *dir_vector);

private:
    void WrapObjectSpeed();
    void ApplyObjectSpeedAction(uint32 action);

public:
    uint32 UpdateActorTargetPursuit();
    uint32 InitializeActorTargetPursuit();
    static AnimatedObject *CloneObjectResource(const AnimatedObject *src);
    uint32 TurnObjectYaw(uint32 target, sint16 step);
    uint32 TurnObjectPitch(uint32 target, sint16 step);
    uint32 TurnObjectFacing(const Vec3 &dir_vector, sint16 step);
    void ComputeColResponse(const SceneNode *other_node, uint32 mode, uint32 val);
    uint32 HandleModelCol(SceneNode *other_node, CollisionContact *contact_data);
    void CapturePathVelocity();
    uint32 UpdateAnimatedModelPathMotion();
    static uint32 ApplyModelTravelRotation(SceneNode *object, uint32 distance);
    void ApplyEvents();
    static void UpdateAnimatedModelOrientation(const AnimatedModelDefinition *definition, AnimatedObject *object);
    uint32 UpdateAnimatedModelState();
    void ResetAnimatedModel();
    uint32 HideAnimatedModel();
    uint32 UpdateActorFacingAttack();
    uint32 InitializeActorFacingAttack();
    uint32 ReleaseActorInteraction();
    void ApplyInteractionDirectionalInput(uint32 *speed);
    void ApplyActorDirectionalInput(uint32 *speed);
    static void QueueCharacterActionSound(sint32 action, uint32 priority);
    void ApplyCharacterPathMotion(sint32 horizontal_speed, sint32 vertical_speed);
    void UpdateCharacterCameraPath();

private:
    void FinishCharacterCameraTransition();

public:
    uint32 UpdateCharacterCameraTransition();
    void ApplyPathVelocityChange(const PathDefinition *definition);
    void UpdateObjectScenePath(const ScenePath *path, sint32 frame);
    void BeginCharacterCameraTransition(const ScenePath *input_path, uint32 frame_word, uint32 duration_ticks, uint32 kind);
    void ReverseScenePath();

private:
    uint32 CanObjectChangePath(const SceneTrigger *trigger, const ScenePath *context) const;
    uint32 TestCharacterSurfaceContact(const SceneTrigger *trigger, uint32 contact) const;
    uint32 CanCharacterContact(const SceneNode *other_node) const;

public:
    sint32 ClassifyCharacterContactResponse() const;
    uint32 CollectCharacterContact(SceneNode *other_node, CollisionContact *contact_data);
    uint32 ResetCharacterForScene(sint32 mode);
    uint32 UpdateCharacterDeathTransition();

private:
    void UpdateCharacterHorizontalSpeed(uint32 decelerate);
    void ApplyEnvironmentMotion(uint32 *horizontal, uint32 *vertical);
    void ApplyDirectionalImpulse(uint32 *move_flags, uint32 mode, const Vec3 &dir_vector, sint32 strength, sint32 limit);
    uint32 ApplyContactImpulse(uint32 *move_flags);

public:
    void EndCharacterSpecialMove();

private:
    uint32 BuildMovementFlags();
    void UpdateJumpMotion(uint32 *move_flags);
    void UpdateSpecialMovement(uint32 *move_flags);
    void UpdateMovementState(uint32 *move_flags);
    void UpdateActionInput(uint32 *move_flags);
    uint32 IntegrateCharacterMovement(uint32 input_flags);
    void UpdateMovementImpulse(uint32 *move_flags);

public:
    uint32 UpdateCharacterMovement();

private:
    void LeaveCharacterSupport();

public:
    const ScenePath *FindCharacterSupportPath(const SceneTrigger *support);
    void ApplyCharacterContactMotion();
    void DetachCharacterFromPath(Vec3 *dir);
    void UpdateCharacterForm();
    static void PauseCharacterSound(void);
    static void ResumeCharacterSound(void);
    uint32 ProcessCharacterPickups();
    uint32 ProcessCharacterSceneEvent();
    void ProcessCharacterTriggers();
    void UpdateCharacterActionEffects();
    void UpdateCharacterContactState();
    void UpdateCharacterFallTimeout();
    uint32 UpdateCharacterDeathState();
    void UpdateCharacterAnimationState();
    uint32 UpdateCharacterBehavior();
    uint32 UpdateAlternateCharacterFollow();
    void BeginCharacterPathTransition();
    void CancelCharacterPathTransition();

private:
    uint32 SetCharacterPathExitLatch();

public:
    void ClearCharacterPathExitLatch();
    void ClearCharacterPathTransitionFlags();
    void RequestCharacterPathExit();
    void CompleteCharacterPathTransfer(CharacterController *pad);
    void ExitCharacterPathTransition();
    void SampleActorPathStart(const ScenePath *path, Vec3 &dst) const;
    void BeginCharacterPathTransfer();

private:
    sint32 EvaluatePathTransition(const uint32 *input) const;

public:
    const ScenePath *ArmCharacterPathHold();
    void EndCharacterPathHold();
    void BeginCharacterPathRelease();
    void ClearCharacterPathRelease();

private:
    void MarkPathSearchFailed();

public:
    void ClearCharacterPathStep();
    void BeginCharacterConstrainedPath();
    void EndCharacterConstrainedPath();
    void ClearCharacterSurfaceMotion();
    const ScenePath *BeginCharacterSurfaceMotion();

private:
    void UpdateTransitionMotion(uint32 input_flags, uint32 mode, uint32 *horizontal, uint32 *vertical);
    void UpdateSpecialAxisMotion(uint32 input_flags, uint32 mode, uint32 *horizontal, uint32 *vertical);

public:
    void ClearCharacterPathAuxiliaryMode();
    static void EmptyActionOperation0(void);
    static void ApplyCameraPosCorrection(const AnimatedObject *owner, AnimatedObject *object);
    static void ApplyHeadingPosCorrection(const AnimatedObject *owner, AnimatedObject *object);
    static void EmptyActionOperation1(void);
    static void EmptyActionOperation2(void);
    static void EmptyActionOperation3(void);
    static Vec3 *UpdateCharacterCameraTarget(Vec3 *dst, const AnimatedObject *object);
    static uint32 BeginCharacterInteraction(AnimatedObject *object);
    void UpdateCharacterInteraction();
    void UpdateCharacterPathTimeout();

private:
    sint32 ClassifyConstrainedContact(const AnimatedObject *other) const;

public:
    void UpdateGroundShadow();
    uint32 UpdateActorStationaryShots();
    uint32 InitializeActorStationaryShots();
    uint32 UpdateActorProximityAttack();
    uint32 InitializeActorProximityAttack();
    uint32 UpdateActorRandomPatrol();
    uint32 InitializeActorRandomPatrol();
    uint32 UpdateActorPathDetour();
    uint32 InitializeActorPathDetour();
    uint32 UpdateActorVerticalPursuit();
    uint32 InitializeActorVerticalPursuit();

private:
    sint32 ResolveObjectSpeed() const;

public:
    uint32 UpdateAnimatedObjectCommands();
    uint32 UpdateSceneScriptVariable();
    static void EmptyObjectBehavior(void);
    uint32 UpdateSceneAnimation();
    uint32 FollowSceneAnimation();
    void RelocateResourceMaterials(const MaterialCatalog *resource, const ModelDecodeLayout *layout, MeshFace **polygon_owner, MeshVertex **vertex_owner);
    static void DecodeModelResource(AnimatedObject **object_slot, uint8 **model_owner, const MaterialCatalog *resource, uint32 bytes, MeshFace **polygon_owner, MeshVertex **vertex_owner);
    void AttachAnimation(uint8 *animation_data, uint32 bytes, AnimationHeader *header);
};

typedef uint32 (AnimatedObject::*CollisionHandler)(SceneNode *, CollisionContact *);

extern AnimatedObject *character_action_owner;

extern AnimatedObject *character_action_target;

#endif
