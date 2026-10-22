#include "actor_placement.h"
#include "game.h"

ActorModelDefinition *ActorPlacement::InitializeBinding()
{
    FUNCTION_MARKER(0x80034658, "SLUS_005.78");
    void *data;
    ActorModelDefinition *table;
    ActorModelDefinition *sel = 0;
    uint32 val, count;
    sint32 kind;
    sint16 frame;
    val = path.node.flags;
    data = state;
    ActorBinding *binding = (ActorBinding *)data;
    path.node.flags = val & 0x7F;
    count = actor_models.count;
    frame = binding->initial_path_frame;
    kind = binding->model_kind;
    binding->instance = 0;
    binding->distance_sq = 0x7FFFF000;
    binding->reaction_flags = 0;
    binding->path_frame = frame;
    table = actor_models.definitions;
    while (count != 0)
    {
        if (table->kind == kind)
        {
            sel = table;
            break;
        }
        --count;
        ++table;
    }
    binding->model = sel;
    return sel;
}

void ActorPlacement::CreateBoundActor()
{
    FUNCTION_MARKER(0x800346C8, "SLUS_005.78");
    void *data;
    const ActorModelDefinition *resource_model;
    uint32 val;
    AnimatedObject *prototype;
    SpriteRecord *spr;
    SceneSection *section;
    AnimatedObject *actor;
    data = state;
    ActorBinding *binding = (ActorBinding *)data;
    resource_model = binding->model;
    prototype = resource_model->prototype;
    if (prototype == 0 || binding->instance != 0)
        return;
    if ((prototype->node.flags & 0x80) != 0)
    {
        actor = prototype;
        prototype->node.links.sibling_next = 0;
    }
    else
    {
        AnimatedObject *created = AnimatedObject::CloneObjectResource(prototype);
        if (created == 0)
            return;
        actor = created;
        created->flags &= 0xFFFFFFFB;
    }
    if ((actor->flags & 0x8000) == 0)
    {
        if ((((const ActorBinding *)state)->flags & 1) != 0)
        {
            spr = actor->spr;
            spr->extent[1] = 0;
            spr->extent[0] = 0;
            spr = actor->spr;
            val = spr->node.flags | 1;
        }
        else
        {
            val = resource_model->spr_extent[0];
            spr = actor->spr;
            spr->extent[0] = (sint32)val >> 5;
            val = resource_model->spr_extent[1];
            spr = actor->spr;
            spr->extent[1] = (sint32)val >> 5;
            spr = actor->spr;
            val = spr->node.flags & 0xFE;
        }
        spr->node.flags = val;
    }
    actor->path_definition = &path;
    actor->ResetActorInstance();
    binding->instance = actor;
    binding->sound_voice = -1;
    section = scene_stream.active_section;
    actor->node.links.sibling_next = section->transient_links.first_child;
    section->transient_links.first_child = &actor->node;
}

void ActorPlacement::ReleaseBoundActor()
{
    FUNCTION_MARKER(0x80034848, "SLUS_005.78");
    void *data;
    const SoundSample *sound;
    sint32 frame, count;
    AnimatedObject *actor;
    data = state;
    ActorBinding *binding = (ActorBinding *)data;
    actor = binding->instance;
    if (actor == 0)
        return;
    sound = binding->sound;
    if ((sound_effects.IsPlaying(sound, binding->sound_voice) & 0xFF) != 0)
        SoundEffects::StopSoundVoice(binding->sound_voice);
    binding->sound = 0;
    binding->sound_voice = -1;
    binding->instance = 0;
    if (actor->flags & 0x10)
        binding->path_frame = actor->path_frame;
    else
    {
        count = (uint16)path.pos_track.frame_count;
        frame = (uint16)actor->path_frame;
        binding->path_frame = (uint32)count - frame - 1;
    }
    frame = binding->path_frame;
    if (frame < 0)
        binding->path_frame = 0;
    else
    {
        count = path.pos_track.frame_count;
        if (frame >= count)
            binding->path_frame = (uint32)count - 1;
    }
    binding->distance_sq = 0x7FFFF000;
    actor->node.flags |= 2;
}

void ActorPlacement::EmptyActorLifecycleOperation(void)
{
    FUNCTION_MARKER(0x80034BA0, "SLUS_005.78");
    /* Original function returns without side effects */
}

uint32 ActorPlacement::LoadMonsterScriptEntry(void)
{
    FUNCTION_MARKER(0x800352CC, "SLUS_005.78");
    return PrintSdkDiagnostic("LSC code should not be calling load_monster_script_entry\n");
}

uint32 ActorPlacement::UpdateEvents()
{
    FUNCTION_MARKER(0x8005BCFC, "SLUS_005.78");
    void *data;
    const uint32 *events;
    AnimatedObject *target;
    uint32 flags, command, kind, val, found, key, count, match;
    flags = path.flags;
    data = state;
    ActorBinding *binding = (ActorBinding *)data;
    if ((flags & 4) != 0)
    {
        path.flags = flags & 0xFFFFFFFB;
        command = binding->event_command;
        if ((command & 0xFFF) != 0)
        {
            kind = (command >> 12) & 31;
            if ((binding->action_effect_flags & 0x100) != 0)
                binding->pending_event = kind + 39;
            switch (kind)
            {
                case 1:
                    path.flags = path.flags | 0x40;
                    break;
                case 3:
                    if ((path.node.flags & 1) == 0)
                    {
                        ReleaseBoundActor();
                        val = path.node.flags;
                        flags = path.flags;
                        path.node.flags = val | 1;
                        path.flags = flags | 1;
                    }
                    break;
                case 5:
                    path.flags = path.flags & 0xFFFFFFBF;
                    break;
                case 8:
                    binding->path_frame = binding->initial_path_frame;
                    ReleaseBoundActor();
                    val = path.node.flags;
                    flags = path.flags;
                    path.node.flags = val | 1;
                    if ((flags & 0x20) != 0)
                    {
                        path.flags = flags | 1;
                        break;
                    }
                case 4:
                    if ((path.node.flags & 1) != 0)
                    {
                        CreateBoundActor();
                        if (kind == 8)
                        {
                            target = binding->instance;
                            if (target != 0)
                            {
                                target->scale = 4096;
                                target = binding->instance;
                                target->orientation[0] = 0x4000;
                                target = binding->instance;
                                target->orientation[3] = 0;
                                target->orientation[2] = 0;
                                target->orientation[1] = 0;
                            }
                        }
                        val = path.node.flags;
                        flags = path.flags;
                        path.node.flags = val & 0xFE;
                        path.flags = flags & 0xFFFFFFFE;
                    }
                    break;
            }
        }
        return 4;
    }
    found = 0;
    if (scene_events.count != 0)
    {
        key = binding->event_command & 0xFFF;
        if (key != 0)
        {
            binding->event_command = key;
            command = scene_events.entries[0];
            count = scene_events.count;
            events = scene_events.entries + 1;
            for (;;)
            {
                match = command & 0xFFF;
                --count;
                if (key == match)
                {
                    binding->event_command = command;
                    found = 1;
                    break;
                }
                if (count == 0)
                    break;
                command = *events;
                ++events;
            }
        }
    }
    if (found != 0)
        path.flags = path.flags | 4;
    else if ((path.flags & 1) != 0 || (sint32)(-(binding->distance_sq + 2)) < 0)
    {
        if (binding->instance != 0)
            ReleaseBoundActor();
    }
    else if (binding->instance == 0)
        CreateBoundActor();
    binding->distance_sq = 0x7FFFF000;
    return 0x7FFFF000;
}
