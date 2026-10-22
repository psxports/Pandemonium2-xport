#include "actor_binding.h"
#include "game.h"

uint32 ActorBinding::StopSound()
{
    FUNCTION_MARKER(0x80030BD4, "SLUS_005.78");
    const SoundSample *sample = sound;
    uint32 voice = sound_voice;
    uint32 result = sound_effects.IsPlaying(sample, voice);
    if ((result << 24) != 0)
        SoundEffects::StopSoundVoice(sound_voice);
    sound = 0;
    sound_voice = -1;
    return 0xFFFFFFFF;
}

uint32 ActorBinding::UpdateSound(AnimatedObject *object, const SoundSample *sample, uint32 mode)
{
    FUNCTION_MARKER(0x80032048, "SLUS_005.78");
    const SoundSample *prev = sound;
    uint32 pitch, rate, random, result;
    sint32 scale = 4096;
    if (sample != prev && prev != 0)
    {
        if ((sound_effects.IsPlaying(prev, sound_voice) & 255) != 0)
            SoundEffects::StopSoundVoice(sound_voice);
        sound = 0;
        sound_voice = -1;
    }
    if (sound_voice < 0)
    {
        rate = sample->pitch;
        if (rate == 0)
            return PrintSdkDiagnostic("Monster type %d bad SFX rate.\n", model_kind);
        random = random_generator.Next();
        if ((rate >> 2) == 0)
            xport_mips_break(7);
        pitch = rate + random % (rate >> 2) - (rate >> 3);
        sound_pitch = pitch;
    }
    else
        pitch = sound_pitch;
    if ((object->flags & 0x30000000) != 0)
    {
        rate = (uint16)object->state;
        if (rate < 2048)
            rate = 2048;
        scale = rate;
        pitch = mul12((sint32)pitch, (0x1000000 / (sint32)rate));
    }
    sound = sample;
    result = SoundEffects::PlaySpatialSound(sample, scale + 4096, 0, pitch, object->world_pos.comp, (uint32)model->sound_radius << 5, sound_voice, mode, mode != 0);
    sound_voice = result;
    return result;
}

uint32 ActorBinding::ComputeInteractionDistance()
{
    FUNCTION_MARKER(0x800343F0, "SLUS_005.78");

    Vec3 pos, input;

    uint32 x, y, z;
    const ActorPlacement *definition_val;
    const PathDefinition *path;
    MATRIX *matrix;
    const AnimatedObject *actor;
    distance_sq = 0x7FFFF000;
    if (CharacterController::FindPlayerObject(0) == 0)
        return distance_sq;
    actor = instance;
    if (actor != 0)
    {
        definition_val = definition;
        path = definition_val != 0 ? &definition_val->path : 0;
        if (path != 0 && path->transform != 0)
        {
            input = actor->pos;
            PushMatrix();
            path = definition != 0 ? &definition->path : NULL;
            matrix = path->transform != 0 ? &path->transform->matrix : NULL;
            SetRotMatrix(matrix);
            {
                VECTOR sdk_input = {input.x, input.y, input.z, 0};
                VECTOR sdk_output;
                ApplyRotMatrixLV(&sdk_input, &sdk_output);
                pos.x = sdk_output.vx;
                pos.y = sdk_output.vy;
                pos.z = sdk_output.vz;
            }
            PopMatrix();
            path = definition != 0 ? &definition->path : NULL;
            matrix = path->transform != 0 ? &path->transform->matrix : NULL;
            x = pos.x;
            x += (uint32)matrix->t[0];
            pos.x = x;
            path = definition != 0 ? &definition->path : NULL;
            matrix = path->transform != 0 ? &path->transform->matrix : NULL;
            y = pos.y;
            y += (uint32)matrix->t[1];
            pos.y = y;
            path = definition != 0 ? &definition->path : NULL;
            matrix = path->transform != 0 ? &path->transform->matrix : NULL;
            z = pos.z;
            z += (uint32)matrix->t[2];
            pos.z = z;
        }
        else
        {
            actor = instance;

            pos = actor->pos;
        }
    }
    else
    {
        definition_val = definition;
        path = definition_val != 0 ? &definition_val->path : NULL;
        if (path->pos_track.keys != 0)
            path->SamplePos(path_frame, pos, 1);
        else
        {
            PathTransform *captured_path_matrix = path->transform;
            if (captured_path_matrix != 0)
            {
                x = captured_path_matrix->matrix.t[0];
                y = captured_path_matrix->matrix.t[1];
                z = captured_path_matrix->matrix.t[2];
            }
            else
            {
                x = path->static_pos.x;
                y = path->static_pos.y;
                z = path->static_pos.z;
            }
            pos.x = x;
            pos.y = y;
            pos.z = z;
        }
    }

    x = pos.x;
    x -= (uint32)camera_motion.focus_pos.x;
    y = pos.y;
    y -= (uint32)camera_motion.focus_pos.y;
    z = pos.z;
    z -= (uint32)camera_motion.focus_pos.z;
    x = (sint32)x >> 12;
    y = (sint32)y >> 12;
    z = (sint32)z >> 12;
    distance_sq = x * x + y * y + z * z;
    return distance_sq;
}
