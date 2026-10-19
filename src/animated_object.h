#ifndef ANIMATED_OBJECT_H
#define ANIMATED_OBJECT_H

#include <stddef.h>

#include "scene_node.h"
#include "animation_data.h"
#include "mesh_morph.h"
#include "collision_geometry.h"

// Scene roots are section-owned; model headers use native clone arenas
struct MorphSpan;
union Quaternion16;
struct ScenePath;
struct PathDefinition;
struct ActorBinding;
struct ActorModelDefinition;
struct MaterialResourceHeader;
struct VolleyScript;
struct AttackScript;
struct CollisionContact;
struct SceneTrigger;
struct CharacterController;

// Contact view of the object's shared work region
struct ModelGroupContacts
{
    uint32 unused0[4];
    SceneNode *blocker;
    uint32 unused20;
    SceneTrigger *surface0;
    uint32 unused28;
    SceneTrigger *surface1;
    uint32 fraction1;
    SceneTrigger *surface2;
    uint32 fraction0;
    uint32 unused48[3];
    uint32 fraction2;
};

struct AnimatedObject
{
    SceneNode node;
    uint32 position[3];
    uint32 flags;
    uint32 metadata32;

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

    uint16 lighting_index;
    MorphSpan mesh_span;
    uint16 metadata58;
    uint32 world_position[3];

    union
    {
        uint8 metadata72[12];
        uint32 metadata_words72[3];
    };

    union
    {
        sint16 orientation[4];
        uint32 orientation_words[2];
    };

    uint32 velocity[3];
    AnimationHeader *animation;

    union
    {
        uint32 event_word;
        AnimatedObject *linked_object;
    };

    // The original word holds a path, timer or action context by behavior
    union
    {
        uint32 behavior_word;
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
    uint8 metadata126[2];
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
        sint16 collision_transform_count;
    };

    // Shared by morph restoration and cached collision transforms
    union
    {
        void *mesh_state;
        MorphRestorationHeader *morph_restoration;
        CollisionTransform *collision_transforms;
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
    SpriteRecord *sprite;

    // The original reuses this region for collisions, saved state and animation work
    union
    {
        CollisionSphere spheres[4];
        uint32 auxiliary_words[16];
        ModelGroupContacts model_contacts;
    };

    uint32 clone_bytes;
    uint32 path_flags;
    // Native storage span for private clone state
    uint32 clone_storage_bytes;
    // Preserve the encoded identity until the first animation binding
    uint32 unbound_animation_word;

    CollisionSphere *BuildCharacterGeometry();
    uint32 FindObjectGeometryCollision();
    void UpdateObjectCollisions();
    uint32 GetCharacterSphereEnds(uint32 sphere, uint8 *first, uint8 *last_output) const;
    uint32 TestSweptObjectBounds(const uint32 *position, const uint32 *velocity, uint32 radius) const;
    uint32 CollideObjectWithTrigger(const SceneTrigger *trigger);
    uint32 CollideObjectSpheres(AnimatedObject *other_object);
    void DrawGeometry(const MorphSpan *span) const;
    void RebuildMorphNormals(const MorphSpan *span) const;
    uint32 AdvanceTracks(sint32 step);
    uint32 ClampAnimationFrame();
    void AttachAnimation(uint8 *animation_data, uint32 bytes, AnimationHeader *header);
    uint32 ComputeCollisionResponse(const SceneNode *other_node, uint32 mode, uint32 value);
    sint32 ClassifyCharacterContactResponse() const;
    uint32 ReadObjectImpulseScale() const;
    uint32 CollectCharacterContact(SceneNode *other_node, CollisionContact *contact_data);
    uint32 HandleModelCollision(SceneNode *other_node, CollisionContact *contact_data);
    uint32 CollectActorContact(SceneNode *other_node, CollisionContact *contact_data);
    uint32 CollectModelGroupContact(SceneNode *other_node, CollisionContact *contact_data);
    sint32 FindSphereFaceCollision(const CollisionSphere *geometry, const SceneNode *other, const CollisionTransform *transform, uint32 *fraction_limit) const;
    uint32 AcceptVerticalFaceContact(const CollisionTransform *descriptor, uint32 selection) const;
    void RelocateResourceMaterials(const MaterialResourceHeader *resource, const ModelDecodeLayout *layout, MeshFace **polygon_owner, MeshVertex **vertex_owner);
    void SampleNodeAnimation(SceneNode *node);
    uint32 UpdateObjectBounds();
    uint32 UpdateGroundShadow();
    uint32 UpdateSceneAnimation();
    uint32 FollowSceneAnimation();
    uint32 UpdateSceneScriptVariable();
    uint32 UpdateAnimatedObjectCommands();
    uint32 UpdateAttackEnemy();
    uint32 UpdateVolleyEnemy();
    uint32 UpdateAlternateCharacterFollow();
    uint32 UpdateActorVelocity();
    uint32 UpdateActorReactionSpin();
    void UpdateActorReactionRecovery();
    uint32 UpdateCharacterDeathTransition();
    uint32 ResetCharacterForScene(sint32 mode);
    void ApplyModelGroupContactMotion();
    uint32 UpdateActorPathMotion();
    uint32 UpdateActorStateAndTarget(uint32 *pos, uint32 *target, uint32 *distance);
    uint32 UpdateActorFacingAndAnimation(const uint32 *target, const uint32 *origin, ActorModelDefinition *resource_model);
    uint32 UpdateActorStationaryShots();
    uint32 UpdateActorVerticalPursuit();
    uint32 UpdateActorRandomPatrol();
    uint32 UpdateActorPathDetour();
    uint32 UpdateActorProximityAttack();
    void UpdateActorPathSegmentMotion();
    uint32 UpdateActorVerticalShot();
    uint32 UpdateActorAimedShot();
    uint32 UpdateActorChaseSpeed();
    uint32 UpdateActorChaseAttack();
    uint32 UpdateBasicActor();
    uint32 UpdateActorPatrolPause();
    uint32 UpdateActorTargetPursuit();
    uint32 UpdateActorFacingAttack();
    uint32 UpdateCommonActor();
    uint32 UpdateAnimatedModelPathMotion();
    uint32 UpdateAnimatedModelState();
    uint32 HideAnimatedModel();
    uint32 ResetAnimatedModel();
    uint32 ApplyEvents();
    uint32 CapturePathVelocity();
    uint32 ComputePathMotion(sint32 distance, sint32 height, const VECTOR *direction_vector);
    uint32 ChangeActorPath(const ScenePath *path, const uint32 *position, uint32 *floor);
    uint32 DetachCharacterFromPath(uint32 *direction);
    void CompleteCharacterPathTransfer(CharacterController *controller);
    uint32 BeginCharacterPathTransfer();
    uint32 ExitCharacterPathTransition();
    uint32 BeginCharacterPathTransition();
    uint32 RequestCharacterPathExit();
    uint32 UpdateCharacterPathTimeout();
    const ScenePath *BeginCharacterSurfaceMotion();
    uint32 BeginCharacterConstrainedPath();
    const ScenePath *ArmCharacterPathHold();
    uint32 EndCharacterPathHold();
    uint32 BeginCharacterPathRelease();
    uint32 ClearCharacterPathRelease();
    uint32 CancelCharacterPathTransition();
    uint32 ClearCharacterPathTransitionFlags();
    uint32 ClearCharacterPathExitLatch();
    uint32 ClearCharacterSurfaceMotion();
    uint32 ClearCharacterPathAuxiliaryMode();
    void ApplyPathVelocityChange(const PathDefinition *definition);
    void UpdateObjectScenePath(const ScenePath *path, sint32 frame);
    uint32 ReverseScenePath();
    uint32 ClearCharacterPathStep();
    uint32 EndCharacterConstrainedPath();
    uint32 SpawnVolleyProjectile(const VolleyScript *script, const uint32 *target, sint32 index);
    uint32 UpdateActionObject();
    uint32 AdvanceActionEffect(const AnimatedObject *owner, uint32 action, uint32 distance, uint32 *progress_output);
    uint32 UpdateActionEffect(const AnimatedObject *owner, uint32 action);
    void UpdateCharacterActionEffects();
    uint32 UpdateCharacterInteraction();
    uint32 UpdateCharacterBehavior();
    uint32 UpdateCharacterMovement();
    uint32 UpdateForm5Movement();
    uint32 EndCharacterSpecialMove();
    uint32 ApplyCharacterPathMotion(sint32 horizontal_speed, sint32 vertical_speed);
    uint32 ApplyCharacterContactMotion();
    uint32 UpdateCharacterSupportMotion();
    uint32 ProbeCharacterSupport();
    uint32 ResolveCharacterSupport(uint32 grounded);
    const ScenePath *FindCharacterSupportPath(const SceneTrigger *support);
    uint32 ProcessCharacterPickups();
    void ProcessCharacterTriggers();
    uint32 ProcessCharacterSceneEvent();
    uint32 UpdateCharacterCameraPath();
    uint32 UpdateCharacterCameraTransition();
    void BeginCharacterCameraTransition(const ScenePath *input_path, uint32 frame_word, uint32 duration_ticks, uint32 kind);
    uint32 UpdateCharacterDeathState();
    void UpdateCharacterFallTimeout();
    uint32 UpdateCharacterAnimationState();
    void UpdateCharacterForm();
    uint32 UpdateCharacterContactState();
    uint32 UpdateActorDeath();
    uint32 ResetActorInstance();
    uint32 GetActorContactDamage() const;
    uint32 InitializeCommonActor();
    uint32 InitializeActorAimedShot();
    uint32 InitializeActorVerticalShot();
    uint32 InitializeActorTargetPursuit();
    uint32 InitializeActorFacingAttack();
    uint32 InitializeActorChaseAttack();
    uint32 InitializeActorPathSegmentMotion();
    uint32 InitializeActorPatrolPause();
    uint32 InitializeActorStationaryShots();
    uint32 InitializeActorProximityAttack();
    uint32 InitializeActorPathDetour();
    uint32 InitializeActorVerticalPursuit();
    uint32 InitializeActorRandomPatrol();
    uint32 InitializeVolleyEnemy();
    uint32 InitializeAttackEnemy();
    uint32 InitializeActorAction(ActorBinding *binding);
    ActorBinding *ResetActorAction();
    uint32 ReleaseActorInteraction();
    uint32 StopModelGroupSound();
    uint32 ApplyInteractionDirectionalInput(uint32 *speed);
    uint32 ApplyActorDirectionalInput(uint32 *speed);
    uint32 CalculateActorTargetMotion(uint32 distance, uint32 *distance_ret, uint32 *height_ret, uint16 *angle_ret) const;
    uint32 CalculateActorPathHeightMotion(uint32 distance, uint32 *distance_ret, uint32 *height_ret, uint16 *angle_ret) const;
    uint32 UpdateActorTurnaround(ActorBinding *binding, const ActorModelDefinition *resource_model);
    uint32 CanActorTurnTowardTarget(const uint32 *position, const uint32 *target, const uint32 *distance, const ActorBinding *binding, const ActorModelDefinition *model, uint8 allowed) const;
    sint32 ClassifyActorTargetDirection(const uint32 *pos, const uint32 *target) const;
    sint32 ClassifyActorPathDirection(const uint32 *pos, const uint32 *target) const;
    uint32 UpdateActorShrinkEffect();
    sint32 FindActorPathSegment() const;
    uint32 ComputeObjectFacing(const uint32 *direction_vector, Quaternion16 *axes) const;
    uint32 TurnObjectFacing(const uint32 *direction_vector, sint16 step);
    sint32 ComputeObjectPitch(const uint32 *direction_vector, Quaternion16 *axes) const;
    uint32 SampleObjectPitch(const uint32 *direction_vector, Quaternion16 *quaternion) const;
    uint32 ReverseObjectFacingFromVelocity();
    sint32 SampleActorPathHeading() const;
    uint32 SampleActorPathStart(const ScenePath *path, uint32 *dst) const;
    uint32 TurnObjectYaw(uint32 target, sint16 step);
    uint32 TurnObjectPitch(uint32 target, sint16 step);

private:
    uint32 WrapObjectSpeed();
    uint32 ApplyObjectSpeedAction(uint32 action);
    uint32 ResolveObjectSpeed() const;
    uint32 UpdateAttackApproach(const uint32 *origin, const uint32 *target, sint32 distance, ActorModelDefinition *resource_model, const AttackScript *script);
    uint32 UpdateAttackAnimation(ActorModelDefinition *resource_model, const AttackScript *script);
    uint32 UpdateAttackRepeat(const uint32 *origin, const uint32 *target, sint32 distance, ActorModelDefinition *resource_model, const AttackScript *script);
    uint32 UpdateVolleyPath(const uint32 *origin, const uint32 *target, sint32 distance, ActorModelDefinition *resource_model, const VolleyScript *script);
    uint32 UpdateVolleyFiring(const uint32 *origin, const uint32 *target, sint32 distance, ActorModelDefinition *resource_model, const VolleyScript *script);
    uint32 CanCharacterContact(const SceneNode *other_node) const;
    uint32 TestCharacterSurfaceContact(const SceneTrigger *trigger, uint32 contact) const;
    uint32 CanObjectChangePath(const SceneTrigger *trigger, const ScenePath *context) const;
    uint32 CanModelGroupContact(const SceneNode *other_node) const;
    sint32 ClassifyConstrainedContact(const AnimatedObject *other) const;
    void ProcessObjectTriggerContacts();
    uint32 TranslateCharacterWithBounds(sint32 x, sint32 y, sint32 z);
    uint32 UpdateCharacterHorizontalSpeed(uint32 decelerate);
    uint32 ApplyDirectionalImpulse(uint32 *move_flags, uint32 mode, const uint32 *direction_vector, sint32 strength, sint32 limit);
    uint32 ResolveSurfaceMotion(uint32 *move_flags);
    uint32 ApplyContactImpulse(uint32 *move_flags);
    uint32 UpdateMovementImpulse(uint32 *move_flags);
    uint32 UpdateJumpMotion(uint32 *move_flags);
    uint32 UpdateSpecialMovement(uint32 *move_flags);
    uint32 UpdateMovementState(uint32 *move_flags);
    uint32 UpdateActionInput(uint32 *move_flags);
    sint32 EvaluatePathTransition(const uint32 *input) const;
    uint32 BuildMovementFlags();
    uint32 LeaveCharacterSupport();
    uint32 ComputePathBoundaryExcess(const uint32 *position) const;
    uint32 UpdateTransitionMotion(uint32 input_flags, uint32 mode, uint32 *horizontal, uint32 *vertical);
    uint32 UpdateSpecialAxisMotion(uint32 input_flags, uint32 mode, uint32 *horizontal, uint32 *vertical);
    void ApplyEnvironmentMotion(uint32 *horizontal, uint32 *vertical);
    uint32 IntegrateCharacterMovement(uint32 input_flags);
    uint32 SetCharacterPathExitLatch();
    uint32 FinishCharacterCameraTransition();
    uint32 UpdateActorAction(const uint32 *pos, uint32 *target, uint32 *distance);
    uint32 GetModelGroupLaunchSpeed() const;
    uint32 UpdateInteractionWave(const AnimatedObject *owner);
    uint32 UpdateInteractionChain(const AnimatedObject *owner);
    uint32 CanChangePath(const uint32 *position, uint32 direction) const;
    const ScenePath *FindFallbackPath(const uint32 *position, uint32 direction);
    uint32 MarkPathSearchFailed();
};

typedef char AnimatedObjectPathDefinitionOffset[(offsetof(AnimatedObject, path_definition) == 132) ? 1 : -1];
typedef char AnimatedObjectSpheresOffset[(offsetof(AnimatedObject, spheres) == 160) ? 1 : -1];
typedef char AnimatedObjectCloneBytesOffset[(offsetof(AnimatedObject, clone_bytes) == 224) ? 1 : -1];
typedef char AnimatedObjectPathFlagsOffset[(offsetof(AnimatedObject, path_flags) == 228) ? 1 : -1];
typedef char AnimatedObjectFlagsOffset[(offsetof(AnimatedObject, flags) == 28) ? 1 : -1];
typedef char AnimatedObjectAnimationOffset[(offsetof(AnimatedObject, animation) == 104) ? 1 : -1];
typedef char AnimatedObjectFrameOffset[(offsetof(AnimatedObject, frame) == 130) ? 1 : -1];

#endif
