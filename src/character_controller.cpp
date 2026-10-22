#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "character_controller.h"
#include "game.h"
#include "animated_object.h"
#include "character_model_record.h"
#include "global.h"
#include "psx_gte.h"

uint32 character_action_flags = OriginalDefaults::CharacterActionFlags;

uint32 character_action_target_score = OriginalDefaults::CharacterActionTargetScore;

uint32 character_sound_resume_flags = OriginalDefaults::CharacterSoundResumeFlags;

CharacterController character_pad = {0};

PlayerInteractionState player_interaction = {OriginalDefaults::InteractionFlags, 0};

void CharacterController::ClearTransientFlags()
{
    FUNCTION_MARKER(0x8001C684, "SLUS_005.78");
    uint32 result = flags & 0xFFFE7FBF;
    flags = result;
}

void CharacterController::DisableCharacterSupportFace(void)
{
    FUNCTION_MARKER(0x8001ECEC, "SLUS_005.78");
    uint32 result = character_pad.flags & 0x18040;
    uint32 idx;
    MeshFace *face;
    if (result != 0)
    {
        const SceneNode *owner = character_pad.support.owner;
        MeshFace *polygons = ((const MeshGeometryNode *)owner)->polygons;
        idx = character_pad.support.face_ref.face_idx;
        face = polygons + idx;
        result = (uint16)face->n.depth_bias & 0xFFFE;
        face->n.depth_bias = result;
    }
}

void CharacterController::RestoreCharacterSupportFace(void)
{
    FUNCTION_MARKER(0x8001ED4C, "SLUS_005.78");
    uint32 result = character_pad.flags & 0x18040;
    uint32 idx;
    MeshFace *face;
    if (result != 0)
    {
        const SceneNode *owner = character_pad.support.owner;
        MeshFace *polygons = ((const MeshGeometryNode *)owner)->polygons;
        idx = character_pad.support.face_ref.face_idx;
        face = polygons + idx;
        result = (uint16)face->n.depth_bias | 1;
        face->n.depth_bias = result;
    }
}

void CharacterController::RemoveAction(uint32 action)
{
    FUNCTION_MARKER(0x80027AC0, "SLUS_005.78");
    AnimatedObject *effect = action_effects[action];
    if (effect != 0)
    {
        uint8 flags = effect->node.flags;
        effect->behavior = 0;
        effect->node.flags = flags | 2;
        action_effects[action] = 0;
    }
}

void CharacterController::RestoreCharacterHealthPoint(void)
{
    FUNCTION_MARKER(0x800444D0, "SLUS_005.78");
    sint32 cur = (sint8)character_pad.vitals.health;
    sint32 limit = (sint8)character_pad.vitals.health_limit;
    uint32 result = cur < limit;
    if (result != 0)
    {
        result = cur + 1;
        character_pad.vitals.health = result;
    }
}

uint32 CharacterController::GetCurCharacterForm(void)
{
    FUNCTION_MARKER(0x80044780, "SLUS_005.78");
    return (uint16)(character_pad.motion.form_flags >> 16) & 15;
}

uint32 CharacterController::ReadCharacterMovementFlags(void)
{
    FUNCTION_MARKER(0x800534EC, "SLUS_005.78");
    return character_pad.movement_flags;
}

void CharacterController::ApplyCharacterSettings(const AnimatedObject *object, const CharacterSettings *settings, uint8 score)
{
    FUNCTION_MARKER(0x80053530, "SLUS_005.78");
    uint32 val;
    uint32 flags;
    if ((game_flow.mode_flags & 1) == 0)
        game_flow.progress.unlocked_stage = score;
    val = 32;
    if ((game_flow.mode_flags & 8) == 0)
        val = settings->counters[CharacterSettings::CNT_LIVES];
    character_pad.vitals.lives = val;
    val = 24;
    if ((game_flow.mode_flags & 4) == 0)
        val = settings->health_limit;
    character_pad.vitals.health_limit = val;
    character_pad.vitals.health = val;
    flags = character_pad.motion.form_flags & 0xFF0FFFFF;
    character_pad.vitals.score = 0;
    character_pad.motion.form_flags = flags;
    flags = (flags | ((settings->flags & 1) << 20)) & 0xFFF0FFFF;
    flags |= ((flags >> 20) & 15) << 16;
    character_pad.motion.form_flags = flags;
    val = settings->action_code;
    if (val == 0)
        character_pad.weapon_state = 0;
    else
        character_pad.weapon_state = val == 1 ? 1 : ((val + 9) << 16) | 1;
    character_pad.timers.idle_ticks = 900;
    character_pad.vitals.health_pickups = settings->health_pickups;
}

void CharacterController::CaptureCharacterSettings(CharacterSettings *settings)
{
    FUNCTION_MARKER(0x80053638, "SLUS_005.78");
    uint32 flags, val;
    flags = (character_pad.flags >> 21) & 2;
    settings->flags = flags;
    val = (character_pad.motion.form_flags >> 20) & 15;
    flags |= val;
    settings->flags = flags;
    settings->counters[CharacterSettings::CNT_HEALTH] = (sint8)character_pad.vitals.health;
    settings->health_limit = character_pad.vitals.health_limit;
    settings->counters[CharacterSettings::CNT_LIVES] = character_pad.vitals.lives;
    settings->counters[CharacterSettings::CNT_SCORE] = (uint16)character_pad.vitals.score;
    val = character_pad.weapon_state;
    if ((val & 0xFFFF) != 0)
    {
        val >>= 16;
        if (val == 0)
            val = 1;
        else
            val -= 9;
    }
    else
        val = 0;
    settings->action_code = val;
    settings->health_pickups = character_pad.vitals.health_pickups;
}

AnimatedObject *CharacterController::FindPlayerObject(uint32 idx)
{
    FUNCTION_MARKER(0x800536F0, "SLUS_005.78");
    if (idx == 0)
        return character_pad.object;
    return 0;
}

void CharacterController::PlaceCharacterOnPath(const SceneSection *scene, sint16 path_id, sint16 frame_word)
{
    CharacterModelRecord *model_rec;
    CharacterModelRecord *other_model_rec;
    AnimatedObject *placed, *placed_view, *alternate;
    SceneNode *spr;
    const PathDefinition *definition, *captured_definition;
    MATRIX *matrix;

    Vec3 pos, sample, world, other_velocity;

    uint16 inverse[9], other_inverse[9];
    const SceneTrigger *marker;
    const ScenePath *path = 0, *path_view, *search_path = 0;
    uint32 character;
    uint32 search_frame, checkpoint_sel = 0;
    uint32 frame = frame_word;
    sint32 best_distance;
    uint32 count;
    sint32 sel_frame;
    AnimatedObject *other_object;
    uint32 dir;
    uint32 x;
    uint32 y;
    uint32 z;
    uint32 val;
    uint32 flags;
    uint32 idx;
    sint32 distance;
    uint32 result;
    sint32 sample_frame;
    FUNCTION_MARKER(0x80053E7C, "SLUS_005.78");
    character = 0x800ABD68;
    scr_render.fade_level = 0;
    character_pad.path_transition.contact_delay = 0;
    character_contact_pool.prev = 0;
    character_contact_pool.cur = 0;
    if (level_resource.level_id == 0 && (sint8)game_flow.progress.sel_stage > 0 && game_flow.progress.hub_spawn_consumed == 0)
    {
        marker = (const SceneTrigger *)scene->object_heads[4];
        game_flow.progress.hub_spawn_consumed = 1;
        while (marker != 0)
        {
            if ((sint8)(marker->node.state_flags >> 8) == 17 && (uint32)(sint8)game_flow.progress.sel_stage == marker->parameters.level)
            {
                x = marker->transform.t[0];
                y = marker->transform.t[1];
                z = marker->transform.t[2];
                pos.x = x;
                pos.y = y;
                pos.z = z;
                pos.y = y - ((uint32)marker->extent[1] << 5);
                path = (const ScenePath *)scene->object_heads[3];
                best_distance = 0x7FFFF000;
                while (path != 0)
                {
                    if ((path->node.state_flags & 0x10) != 0)
                    {
                        captured_definition = path->definition;
                        definition = captured_definition;
                        sel_frame = 0xFFFFFFFF;
                        if (definition->pos_track.keys != 0)
                        {
                            frame = 0;
                            count = definition->pos_track.frame_count;
                            do
                            {
                                definition->SamplePos(frame, sample, 16);
                                x = sample.x - (uint32)pos.x;
                                sample.x = x;
                                if (x + 409600 < 0xC8000)
                                {
                                    y = sample.y - (uint32)pos.y;
                                    sample.y = y;
                                    if (y + 409600 < 0xC8000)
                                    {
                                        z = sample.z - (uint32)pos.z;
                                        sample.z = z;
                                        if (z + 409600 < 0xC8000)
                                        {
                                            x = (sint32)x >> 6;
                                            y = (sint32)y >> 6;
                                            z = (sint32)z >> 6;
                                            distance = x * x + y * y + z * z;
                                            if (distance < best_distance)
                                            {
                                                best_distance = distance;
                                                sel_frame = frame;
                                            }
                                        }
                                    }
                                }
                                --count;
                                ++frame;
                            } while (count != 0);
                        }
                        if (sel_frame >= 0)
                        {
                            search_path = path;
                            search_frame = sel_frame;
                            checkpoint_sel = 1;
                        }
                    }
                    path = (const ScenePath *)path->node.links.registry_next;
                }
                if (checkpoint_sel != 0)
                {
                    path = search_path;
                    frame = search_frame;
                }
            }
            marker = (const SceneTrigger *)marker->node.links.registry_next;
        }
    }
    if (path == 0)
    {
        /* A missing checkpoint path uses the ordinary scene spawn */
        frame = frame_word;
        sample_frame = path_id;
        if (sample_frame != -1)
            path = scene->FindPath(sample_frame);
        if (sample_frame == -1 || path == 0)
        {
            path = scene->FindDefaultPath();
            if (path == 0)
                path = scene->FindPath(0);
        }
    }
    idx = (uint16)(character_pad.motion.form_flags >> 16) & 0xF;
    {
        uint32 model_idx = idx;
        CharacterModelRecord *model_binding;
        model_binding = character_pad.model_slots[model_idx];
        model_rec = model_binding;
    }
    placed = character_pad.object;
    if (placed != model_rec->object || (game_flow.transition_flags & 2) != 0 || character_pad.model_rebind_pending != 0)
    {
        CharacterModelRecord::AssignCharacterModel(&placed, (uint16)(character_pad.motion.form_flags >> 16) & 0xF);
    }

    {
        uint32 model_idx = (uint16)(character_pad.motion.form_flags >> 16) & 0xF;
        CharacterModelRecord *model_binding;
        model_binding = character_pad.model_slots[model_idx];
        model_rec = model_binding;
        character_pad.model = model_binding;
    }
    placed_view = placed;
    path_view = path;
    flags = path_view->node.state_flags;
    character_pad.model_rebind_pending = 0;
    dir = flags & 0x10;
    if ((frame & 0x8000) != 0)
    {
        frame = ~frame;
        if ((flags & 8) != 0)
        {
            path = path_view->adjacent[0];
            path_view = path;
        }
        dir ^= 0x10;
    }
    if (dir != (placed_view->flags & 0x10))
    {
        frame = (uint16)scene->transition_scale - frame - 1;
        if ((path_view->node.state_flags & 8) != 0)
        {
            path = path_view->adjacent[0];
            path_view = path;
        }
        dir ^= 0x10;
    }
    captured_definition = path->definition;
    definition = captured_definition;
    val = definition->frame_scale;
    if (val != 0)
        frame = (val * (sint16)frame) >> 12;
    placed_view->movement_mode = 1;
    placed_view->behavior = 1;
    placed_view->contact_kind = 1;
    placed_view->behavior_suspended = 0;
    flags = placed_view->path_flags;
    spr = (SceneNode *)placed_view->spr;
    val = placed_view->flags;
    model_rec = character_pad.model;
    idx = character_pad.motion.state_code;
    placed_view->animation = model_rec->animations[(sint32)idx];
    placed_view->unbound_animation_word = 0;
    placed_view->flags = (val & 0xFFFDDBFF) | 0x20000;
    placed_view->path_flags = flags & 0xFC000000;
    spr->flags = spr->flags & 0xFE;
    CameraMotion::SelectScenePath(path);
    character_pad.path = path;
    camera_motion.frame = frame;
    camera_motion.smooth_ticks[2] = 0;
    camera_motion.smooth_ticks[1] = 0;
    camera_motion.smooth_ticks[0] = 0;
    placed_view->path_definition = path_view->definition;
    placed_view->yaw = 4096;
    captured_definition = path->definition;
    definition = captured_definition;
    sample_frame = camera_motion.frame;
    if (dir == 0)
        sample_frame = definition->pos_track.frame_count - sample_frame - 1;
    AnimationTrack::SampleVectorTrack(sample_frame, &definition->pos_track, placed_view->pos);
    placed_view->path_frame = camera_motion.frame;
    if (level_resource.level_id == 0)
    {
        character_pad.flags = character_pad.flags | 0x80000000;
        placed_view->ResetCharacterForScene(0);
    }
    else
    {
        character_pad.flags = character_pad.flags & 0x7FFFFFFF;
        if (character_pad.section_height_offset > 0)
        {
            captured_definition = path->definition;
            definition = captured_definition;
            if (captured_definition != 0 && definition->transform != 0)
            {
                pos = placed_view->pos;
                PushMatrix();
                matrix = placed_view->path_definition->transform != 0 ? &placed_view->path_definition->transform->matrix : NULL;
                SetRotMatrix(matrix);
                {
                    VECTOR rotation_output;
                    {
                        VECTOR sdk_input = {pos.x, pos.y, pos.z, 0};
                        ApplyRotMatrixLV(&sdk_input, &rotation_output);
                    }
                    placed_view->pos.x = rotation_output.vx;
                    placed_view->pos.y = rotation_output.vy;
                    placed_view->pos.z = rotation_output.vz;
                }
                PopMatrix();
                matrix = placed_view->path_definition->transform != 0 ? &placed_view->path_definition->transform->matrix : NULL;
                placed_view->pos.x = (uint32)placed_view->pos.x + matrix->t[0];
                matrix = placed_view->path_definition->transform != 0 ? &placed_view->path_definition->transform->matrix : NULL;
                placed_view->pos.y = (uint32)placed_view->pos.y + matrix->t[1];
                matrix = placed_view->path_definition->transform != 0 ? &placed_view->path_definition->transform->matrix : NULL;
                placed_view->pos.z = (uint32)placed_view->pos.z + matrix->t[2];
            }
            placed_view->pos.y = (uint32)placed_view->pos.y + character_pad.section_height_offset;
            captured_definition = path->definition;
            definition = captured_definition;
            if (captured_definition != 0 && definition->transform != 0)
            {
                matrix = definition->transform != 0 ? &definition->transform->matrix : NULL;
                sample.x = (uint32)placed_view->pos.x - matrix->t[0];
                matrix = placed_view->path_definition->transform != 0 ? &placed_view->path_definition->transform->matrix : NULL;
                sample.y = (uint32)placed_view->pos.y - matrix->t[1];
                matrix = placed_view->path_definition->transform != 0 ? &placed_view->path_definition->transform->matrix : NULL;
                sample.z = (uint32)placed_view->pos.z - matrix->t[2];
                matrix = definition->transform != 0 ? &definition->transform->matrix : NULL;
                inverse[0] = matrix->m[0][0];
                inverse[1] = matrix->m[1][0];
                inverse[2] = matrix->m[2][0];
                inverse[3] = matrix->m[0][1];
                inverse[4] = matrix->m[1][1];
                inverse[5] = matrix->m[2][1];
                inverse[6] = matrix->m[0][2];
                inverse[7] = matrix->m[1][2];
                inverse[8] = matrix->m[2][2];
                PushMatrix();
                x = inverse[0] | ((uint32)inverse[1] << 16);
                y = inverse[2] | ((uint32)inverse[3] << 16);
                xport_gte_write_control(0, x);
                xport_gte_write_control(1, y);
                x = inverse[4] | ((uint32)inverse[5] << 16);
                y = inverse[6] | ((uint32)inverse[7] << 16);
                // Control register 4 consumes only the ninth halfword
                z = inverse[8];
                xport_gte_write_control(2, x);
                xport_gte_write_control(3, y);
                xport_gte_write_control(4, z);
                {
                    VECTOR rotation_output;
                    {
                        VECTOR sdk_input = {sample.x, sample.y, sample.z, 0};
                        ApplyRotMatrixLV(&sdk_input, &rotation_output);
                    }
                    placed_view->pos.x = rotation_output.vx;
                    placed_view->pos.y = rotation_output.vy;
                    placed_view->pos.z = rotation_output.vz;
                }
                PopMatrix();
            }
            placed_view->flags = placed_view->flags | 0x400;
        }
    }
    if ((placed_view->path_definition->flags & 0x11) != 0)
        placed_view->flags = placed_view->flags | 0x400;
    path_view->SampleOffset(camera_motion.frame, camera_motion.pos, dir);
    path_view->SamplePos(camera_motion.frame, camera_motion.focus_pos, dir);
    placed_view->spheres[3].radius = 0xFFFFFFFF;
    placed_view->spheres[2].radius = 0xFFFFFFFF;
    placed_view->spheres[1].radius = 0xFFFFFFFF;
    placed_view->spheres[0].radius = 0xFFFFFFFF;
    (placed != 0 ? &placed_view->node : 0)->Register();
    if ((sint32)character_pad.flags < 0)
    {
        placed_view->flags = placed_view->flags | 0x40;
        placed_view->ComputePathMotion(24576, 0, NULL);
        placed_view->flags = placed_view->flags & 0xFFFFFFBF;
        captured_definition = path->definition;
        definition = captured_definition;
        if (captured_definition != 0 && definition->transform != 0)
        {
            matrix = definition->transform != 0 ? &definition->transform->matrix : NULL;
            inverse[0] = matrix->m[0][0];
            inverse[1] = matrix->m[1][0];
            inverse[2] = matrix->m[2][0];
            inverse[3] = matrix->m[0][1];
            inverse[4] = matrix->m[1][1];
            inverse[5] = matrix->m[2][1];
            inverse[6] = matrix->m[0][2];
            inverse[7] = matrix->m[1][2];
            inverse[8] = matrix->m[2][2];

            sample = placed_view->velocity;
            PushMatrix();
            x = inverse[0] | ((uint32)inverse[1] << 16);
            y = inverse[2] | ((uint32)inverse[3] << 16);
            xport_gte_write_control(0, x);
            xport_gte_write_control(1, y);
            x = inverse[4] | ((uint32)inverse[5] << 16);
            y = inverse[6] | ((uint32)inverse[7] << 16);
            z = inverse[8];
            xport_gte_write_control(2, x);
            xport_gte_write_control(3, y);
            xport_gte_write_control(4, z);
            {
                VECTOR sdk_input = {sample.x, sample.y, sample.z, 0};
                VECTOR sdk_output;
                ApplyRotMatrixLV(&sdk_input, &sdk_output);
                pos.x = sdk_output.vx;
                pos.y = sdk_output.vy;
                pos.z = sdk_output.vz;
            }
            PopMatrix();
        }
        else
        {
            pos = placed_view->velocity;
        }
        placed_view->pos.x = placed_view->pos.x + (uint32)pos.x;
        placed_view->pos.y = placed_view->pos.y + (uint32)pos.y;
        placed_view->pos.z = placed_view->pos.z + (uint32)pos.z;
        placed_view->velocity.z = 0;
        placed_view->velocity.y = 0;
        placed_view->velocity.x = 0;
        captured_definition = path->definition;
        definition = captured_definition;
        if (captured_definition != 0 && definition->transform != 0)
        {
            world = placed_view->pos;
            PushMatrix();
            matrix = placed_view->path_definition->transform != 0 ? &placed_view->path_definition->transform->matrix : NULL;
            SetRotMatrix(matrix);
            {
                VECTOR rotation_output;
                {
                    VECTOR sdk_input = {world.x, world.y, world.z, 0};
                    ApplyRotMatrixLV(&sdk_input, &rotation_output);
                }
                camera_motion.focus_pos.x = rotation_output.vx;
                camera_motion.focus_pos.y = rotation_output.vy;
                camera_motion.focus_pos.z = rotation_output.vz;
            }
            PopMatrix();
            matrix = placed_view->path_definition->transform != 0 ? &placed_view->path_definition->transform->matrix : NULL;
            camera_motion.focus_pos.x = (uint32)camera_motion.focus_pos.x + matrix->t[0];
            matrix = placed_view->path_definition->transform != 0 ? &placed_view->path_definition->transform->matrix : NULL;
            camera_motion.focus_pos.y = (uint32)camera_motion.focus_pos.y + matrix->t[1];
            matrix = placed_view->path_definition->transform != 0 ? &placed_view->path_definition->transform->matrix : NULL;
            camera_motion.focus_pos.z = (uint32)camera_motion.focus_pos.z + matrix->t[2];
        }
        else
        {
            camera_motion.focus_pos = placed_view->pos;
        }
        sample_frame = placed_view->path_frame;
        camera_motion.frame = sample_frame;
        camera_motion.path->ProjectOffset(sample_frame, camera_motion.pos, placed_view->flags & 0x10);
        {
            uint32 model_idx = 1 - ((uint16)(character_pad.motion.form_flags >> 16) & 0xF);
            CharacterModelRecord *model_binding;
            model_binding = character_pad.model_slots[model_idx];
            other_model_rec = model_binding;
        }
        other_object = other_model_rec->object;
        alternate = other_object;
        alternate->node.flags = placed_view->node.flags;

        alternate->pos = placed_view->pos;
        alternate->flags = placed_view->flags;
        alternate->path_flags = placed_view->path_flags;

        alternate->world_pos = placed_view->world_pos;
        x = placed_view->bounds.radius;
        y = placed_view->bounds.height;
        z = placed_view->bounds.depth;
        alternate->bounds.radius = x;
        alternate->bounds.height = y;
        alternate->bounds.depth = z;
        alternate->rotation = placed_view->rotation;

        alternate->velocity = placed_view->velocity;
        alternate->event_word = placed_view->event_word;
        alternate->behavior_data = placed_view->behavior_data;
        alternate->state = placed_view->state;
        alternate->movement_mode = placed_view->movement_mode;
        alternate->behavior = placed_view->behavior;
        alternate->contact_kind = placed_view->contact_kind;
        alternate->behavior_suspended = placed_view->behavior_suspended;
        alternate->ticks = placed_view->ticks;
        alternate->frame = placed_view->frame;
        alternate->path_definition = placed_view->path_definition;
        alternate->path_frame = placed_view->path_frame;
        alternate->yaw = placed_view->yaw;
        alternate->target_yaw = placed_view->target_yaw;
        alternate->pitch = placed_view->pitch;
        alternate->pitch_scale = placed_view->pitch_scale;
        alternate->path_motion_scale = placed_view->path_motion_scale;
        for (uint32 sphere = 0; sphere < 4; ++sphere)
            alternate->spheres[sphere] = placed_view->spheres[sphere];
        alternate->model_contacts = placed_view->model_contacts;
        alternate->movement_mode = 0;
        alternate->contact_kind = 0;
        alternate->behavior_suspended = 0;
        alternate->behavior = 33;
        flags = alternate->flags;
        alternate->scene_path = path;
        alternate->follow_target = placed;
        AnimationHeader *initial_animation = other_model_rec->animations[0];
        alternate->flags = flags | 0x20000;
        alternate->animation = initial_animation;
        alternate->unbound_animation_word = 0;
        (other_object != 0 ? &alternate->node : 0)->Register();
        flags = alternate->node.flags;
        val = alternate->flags & 0xFFFFFFBF;
        alternate->contact_kind = 1;
        alternate->node.flags = flags | 0x41;
        alternate->flags = val;
        alternate->ComputePathMotion(45056, 0, NULL);
        alternate->contact_kind = 0;
        captured_definition = path->definition;
        definition = captured_definition;
        if (captured_definition != 0 && definition->transform != 0)
        {
            matrix = definition->transform != 0 ? &definition->transform->matrix : NULL;
            other_inverse[0] = matrix->m[0][0];
            other_inverse[1] = matrix->m[1][0];
            other_inverse[2] = matrix->m[2][0];
            other_inverse[3] = matrix->m[0][1];
            other_inverse[4] = matrix->m[1][1];
            other_inverse[5] = matrix->m[2][1];
            other_inverse[6] = matrix->m[0][2];
            other_inverse[7] = matrix->m[1][2];
            other_inverse[8] = matrix->m[2][2];

            other_velocity = alternate->velocity;
            PushMatrix();
            x = other_inverse[0] | ((uint32)other_inverse[1] << 16);
            y = other_inverse[2] | ((uint32)other_inverse[3] << 16);
            xport_gte_write_control(0, x);
            xport_gte_write_control(1, y);
            x = other_inverse[4] | ((uint32)other_inverse[5] << 16);
            y = other_inverse[6] | ((uint32)other_inverse[7] << 16);
            z = other_inverse[8];
            xport_gte_write_control(2, x);
            xport_gte_write_control(3, y);
            xport_gte_write_control(4, z);
            {
                VECTOR sdk_input = {other_velocity.x, other_velocity.y, other_velocity.z, 0};
                VECTOR sdk_output;
                ApplyRotMatrixLV(&sdk_input, &sdk_output);
                pos.x = sdk_output.vx;
                pos.y = sdk_output.vy;
                pos.z = sdk_output.vz;
            }
            PopMatrix();
        }
        else
        {
            pos = alternate->velocity;
        }
        alternate->pos.x = alternate->pos.x + (uint32)pos.x;
        alternate->pos.y = alternate->pos.y + (uint32)pos.y;
        alternate->pos.z = alternate->pos.z + (uint32)pos.z;
        pos.x = -(uint32)alternate->velocity.x;
        pos.y = -(uint32)alternate->velocity.y;
        pos.z = -(uint32)alternate->velocity.z;
        alternate->velocity.z = 0;
        alternate->velocity.y = 0;
        alternate->velocity.x = 0;
        alternate->TurnObjectFacing(pos, 4096);
    }
    {
        MATRIX *root_matrix = &render_view.matrix_cursor->matrix;
        memcpy(root_matrix->m, psx_identity_matrix.m, sizeof(root_matrix->m));
        root_matrix = &render_view.matrix_cursor->matrix;
        SetRotMatrix(root_matrix);
        root_matrix = &render_view.matrix_cursor->matrix;
        root_matrix->t[2] = 0;
        root_matrix->t[1] = 0;
        root_matrix->t[0] = 0;
    }
    placed_view->node.flags = placed_view->node.flags | 0x41;
    CameraProjection::UpdateAndDrawRegisteredObjects();
    placed_view->node.flags = placed_view->node.flags & 0xFE;
    val = character_pad.weapon_state;
    if ((val & 0xFFFF) != 0)
    {
        idx = (uint16)(character_pad.motion.form_flags >> 16) & 0xF;
        if (idx != 5 && idx - 10 >= 2)
        {
            val >>= 16;
            AnimatedObject::SpawnActionEffect(placed, character != 0 ? (const void *)&character_pad : 0, val == 0 ? 3 : val - 7);
        }
    }
    if ((character_pad.secondary_flags & 0x40) != 0)
    {
        model_rec = character_pad.model;
        AnimationHeader *animation22 = model_rec->animations[22];
        AnimationHeader *animation2 = model_rec->animations[2];
        model_rec->animations[2] = animation22;
        model_rec = character_pad.model;
        model_rec->animations[22] = animation2;
        AnimatedObject::SpawnActionEffect(placed, character != 0 ? (const void *)&character_pad : 0, 7);
    }
    val = camera_motion.focus_pos.y;
    val += (sint32)character_pad.flags < 0 ? 0x8000 : 14336;
    camera_motion.focus_pos.y = val;
    camera_motion.flags = camera_motion.flags & 0xFE;
    placed_view->path_definition->SampleTangent(&character_pad.path_tangent, dir);
    val = game_flow.mode_flags;
    player_interaction.flags = 0;
    if ((val & 0x200) == 0 || ((uint16)(character_pad.motion.form_flags >> 16) & 0xF) != 0)
    {
        flags = character_pad.secondary_flags;
        result = flags & 0x120;
        if (result == 0)
            goto finish;
        result = flags | 0x20;
        if (((uint16)(character_pad.motion.form_flags >> 16) & 0xF) == 1)
        {
            character_pad.secondary_flags = result;
            goto finish;
        }
        AnimatedObject::SpawnActionEffect(placed, character != 0 ? (const void *)&character_pad : 0, 6);
    }
    AnimatedObject::BeginCharacterInteraction(placed);
finish:
    return;
}

uint32 CharacterController::GetSceneHealthPickupFlag(uint32 scene)
{
    FUNCTION_MARKER(0x80055848, "SLUS_005.78");
    if (scene == 20)
        return character_pad.vitals.health_pickups & 4;
    if (scene == 75)
        return character_pad.vitals.health_pickups & 1;
    if (scene == 10)
        return character_pad.vitals.health_pickups & 2;
    return 1;
}

uint32 CharacterController::SelectCharacterForm(uint32 mode)
{
    FUNCTION_MARKER(0x800558AC, "SLUS_005.78");
    uint32 flags = character_pad.motion.form_flags;
    uint32 val = ((flags & 0xFFF0FFFF) | (mode << 16)) & 0xFF0FFFFF;
    character_pad.motion.form_flags = val | (mode << 20);
    return (flags >> 20) & 0xF;
}

void CharacterController::ResetNewGameCharacter(void)
{
    uint32 idx = 0;
    uint32 offset = 0;

    uint32 flags;
    uint32 kind;
    AnimatedObject *model;

    uint32 val;
    FUNCTION_MARKER(0x800558F0, "SLUS_005.78");
    if ((game_flow.mode_flags & 1) == 0)
        game_flow.progress.unlocked_stage = 0;
    game_flow.progress.sel_stage = 0;
    character_pad.vitals.health_pickups = 0;
    scr_render.fade_level = 0;
    do
    {
        character_pad.vitals.lives = (game_flow.mode_flags & 8) ? 32 : 3;
        val = (game_flow.mode_flags & 4) ? 24 : 4;
        character_pad.vitals.health_limit = val;
        character_pad.vitals.health = val;
        flags = character_pad.motion.form_flags;
        character_pad.weapon_state = 0;
        character_pad.vitals.score = 0;
        character_pad.timers.idle_ticks = 900;
        flags &= 0xFFF0FFFF;
        flags |= ((flags >> 20) & 15) << 16;
        flags &= 0xF0FFFFFF;
        flags |= 0x0F000000;
        character_pad.motion.form_flags = flags;
        character_pad.object = 0;
        kind = (character_pad.motion.form_flags >> 20) & 15;
        if (character_pad.loaded_model_mask & (kind == 0 ? 1 : 2))
        {
            {
                CharacterModelRecord *sel_model;
                {
                    uint32 model_idx = kind;
                    CharacterModelRecord *model_binding;
                    model_binding = character_pad.model_slots[model_idx];
                    sel_model = model_binding;
                }
                model = sel_model->object;
            }
            model->event_word = idx;
        }
        if (character_pad.secondary_flags & 0x100)
        {
            character_pad.action_effects[6] = 0;
            character_pad.EndInteraction();
        }
        if (character_pad.secondary_flags & 0x20)
        {
            flags = player_interaction.flags;
            val = character_pad.secondary_flags;
            player_interaction.flags = flags & 0xFFFFFFDB;
            character_pad.secondary_flags = val & 0xFFFFFFDF;
        }
        val = character_pad.weapon_state;

        if ((val & 0xFFFF) != 0)
        {
            character_pad.action_effects[val >> 16] = 0;
        }
        character_pad.weapon_state = 0;
        ++idx;
        offset += 928;
    } while ((sint32)idx <= 0);
}

uint32 CharacterController::CanContinuePathStep(void)
{
    FUNCTION_MARKER(0x80056328, "SLUS_005.78");
    uint32 limit, count;
    limit = path_transition_rules.early_movement_ticks;
    count = character_pad.path_transition.movement_ticks;
    return count < limit;
}

uint32 CharacterController::IsCharacterAnchorPathActive(void)
{
    FUNCTION_MARKER(0x8005A3FC, "SLUS_005.78");
    const ScenePath *sel = character_pad.path_transition.anchor_path;
    const ScenePath *path = camera_motion.path;
    return sel == path;
}

const ScenePath *CharacterController::ReadCharacterConstrainedPath(void)
{
    FUNCTION_MARKER(0x8005A430, "SLUS_005.78");
    return character_pad.path_transition.constrained_path;
}

uint32 CharacterController::CancelCountedActions(uint32 incoming_flags)
{
    FUNCTION_MARKER(0x8005A480, "SLUS_005.78");
    sint32 idx;
    if ((incoming_flags & 0x10) != 0)
    {
        for (idx = 0; idx < action13_count; ++idx)
            RemoveAction(13);
        action13_count = 0;
    }
    if ((character_action_flags & 0x20) != 0)
    {
        for (idx = 0; idx < action14_count; ++idx)
            RemoveAction(14);
        action14_count = 0;
    }
    incoming_flags = character_action_flags & 0xFFFFFFCF;
    character_action_flags = incoming_flags;
    return incoming_flags;
}

void CharacterController::ClearInteractionActions()
{
    FUNCTION_MARKER(0x8005A540, "SLUS_005.78");
    uint32 flags;
    sint32 idx;
    if (character_action_flags != 0)
    {
        AnimatedObject *active = character_action_target;
        if (active != 0)
        {
            ActorBinding *binding = (ActorBinding *)(((const ActorPlacement *)active->path_definition)->state);
            binding->flags &= 0xF7;
        }
    }
    if ((character_action_flags & 8) != 0)
        RemoveAction(12);
    flags = character_action_flags;
    character_action_flags = flags & 0xFFFFEFF7;
    if ((flags & 16) != 0)
    {
        if (action13_count > 0)
        {
            idx = 0;
            do
            {
                RemoveAction(13);
                ++idx;
            } while (idx < action13_count);
        }
        action13_count = 0;
    }
    if ((character_action_flags & 32) != 0)
    {
        if (action14_count > 0)
        {
            idx = 0;
            do
            {
                RemoveAction(14);
                ++idx;
            } while (idx < action14_count);
        }
        action14_count = 0;
    }
    character_action_flags = character_action_flags & ~48u;
}

void CharacterController::EndInteraction()
{
    FUNCTION_MARKER(0x8005A668, "SLUS_005.78");
    uint32 flags;
    sint32 idx;
    if (character_action_flags != 0)
    {
        AnimatedObject *active = character_action_target;
        if (active != 0)
        {
            ActorBinding *binding = (ActorBinding *)(((const ActorPlacement *)active->path_definition)->state);

            binding->flags &= 0xF7;
        }
    }
    if ((character_action_flags & 8) != 0)
        RemoveAction(12);
    flags = character_action_flags;
    character_action_flags = flags & 0xFFFFEFF7;
    if ((flags & 16) != 0)
    {
        if (action13_count > 0)
        {
            idx = 0;
            do
            {
                RemoveAction(13);
                ++idx;
            } while (idx < action13_count);
        }
        action13_count = 0;
    }
    if ((character_action_flags & 32) != 0)
    {
        if (action14_count > 0)
        {
            idx = 0;
            do
            {
                RemoveAction(14);
                ++idx;
            } while (idx < action14_count);
        }
        action14_count = 0;
    }
    flags = secondary_flags;
    character_action_flags = 0;
    flags &= ~256u;
    secondary_flags = flags;
}
