#include "game.h"
#include "original_defaults.h"

CharacterContactPool character_contact_pool = {0};

static const sint8 pickup_scores[3] = {1, 5, 25};

CharacterContactNode *InitializeContactPool(void)
{
    FUNCTION_MARKER(0x8005BA34, "SLUS_005.78");
    CharacterContactNode *pointer = character_contact_pool.nodes + CharacterContactPool::Capacity - 1;
    sint32 index;
    for (index = CharacterContactPool::Capacity - 2; index >= 0; --index)
    {
        character_contact_pool.nodes[index].next = pointer;
        --pointer;
    }
    character_contact_pool.nodes[CharacterContactPool::Capacity - 1].next = 0;
    character_contact_pool.free_head = character_contact_pool.nodes;
    return character_contact_pool.nodes;
}

uint32 ContainsContact(const CharacterContactNode *head, const SceneNode *object)
{
    FUNCTION_MARKER(0x8005BAAC, "SLUS_005.78");
    while (head != 0)
    {
        if (head->object == object)
            return 1;
        head = head->next;
    }
    return 0;
}

CharacterContactNode *QueueContact(CharacterContactNode **head, SceneNode *object)
{
    FUNCTION_MARKER(0x8005BADC, "SLUS_005.78");
    CharacterContactNode *node = character_contact_pool.free_head;
    CharacterContactNode *result;
    if (node == 0)
        return *head;
    character_contact_pool.free_head = node->next;
    node->object = object;
    result = *head;
    node->next = result;
    *head = node;
    return result;
}

void ReleaseContacts(CharacterContactNode **head)
{
    FUNCTION_MARKER(0x8005BB14, "SLUS_005.78");
    CharacterContactNode *node = *head;
    CharacterContactNode *next, *free_head;
    while (node != 0)
    {
        next = node->next;
        free_head = character_contact_pool.free_head;
        character_contact_pool.free_head = node;
        node->next = free_head;
        node = next;
    }
    *head = 0;
}

CharacterContactNode *FinishCharacterContacts(void)
{
    FUNCTION_MARKER(0x80047C1C, "SLUS_005.78");
    CharacterContactNode *node, *result, *value;
    SceneNode *object;
    sint32 kind;
    for (node = character_contact_pool.prev; node != 0; node = node->next)
    {
        value = character_contact_pool.cur;
        object = node->object;
        if (ContainsContact(value, object) == 0)
        {
            object = node->object;
            kind = (sint8)(object->state_flags >> 8);
            if (kind == 53)
                QueueSceneEvent((xport_load_le32((const uint8 *)object + 68) & 0xFFF) | 0x4000, 4);
            else if (kind == 54)
                QueueSceneEvent((xport_load_le32((const uint8 *)object + 68) & 0xFFF) | 0x3000, 4);
        }
    }
    ReleaseContacts(&character_contact_pool.prev);
    result = character_contact_pool.cur;
    character_contact_pool.cur = 0;
    character_contact_pool.prev = result;
    return result;
}

uint32 AnimatedObject::CanCharacterContact(const SceneNode *other_node) const
{
    const ScenePath *trigger_path;
    const PathDefinition *parent;
    const PathDefinition *trigger_definition, *controller_definition;
    const PathTransform *captured_path_transform;
    FUNCTION_MARKER(0x80047524, "SLUS_005.78");
    const SceneNode *other = (const SceneNode *)(other_node);
    const ScenePath *controller_path;
    uint32 state = 0x800ABD68;
    uint32 category, event, mode, flags, value, x, y, z;
    const PathTransform *path_transform;
    sint32 kind;
    if ((character_controller.flags & 0x4000000) != 0)
        return 0;
    category = other->kind;
    if (category == 2)
    {
        event = (other->state_flags >> 8);
        switch (event)
        {
            case 2:
            case 4:
            case 8:
            case 9:
            case 18:
            case 19:
            case 20:
            case 22:
            case 23:
            case 26:
            case 27:
            case 28:
            case 30:
                return 1;
            case 3:
                return (HasPendingSceneSection() & 0xFF) != 0;
            case 5:
                flags = this->flags;
                if ((flags & 0x22000000) != 0)
                    return 0;
                if ((flags & 0x8000000) != 0 && ((uint16)(character_controller.motion.form_flags >> 16) & 0xF) == 4)
                    return 0;
                value = frame_clock.render_tick;
                return value >= ((const SceneTrigger *)other)->parameters.words[0];
            case 6:
                return (this->flags & 0x400) != 0 && character_controller.motion.vertical_speed < 0;
            case 7:
                x = this->velocity[0];
                y = this->velocity[1];
                z = this->velocity[2];
                parent = (this->path_definition);
                if (parent != 0)
                {
                    path_transform = ((captured_path_transform = ((parent)->transform)) != 0 ? (const PathTransform *)((PathTransform *)(captured_path_transform)) : 0);
                    if (path_transform != 0 && (this->flags & 0x2000) == 0)
                    {
                        x -= path_transform->displacement[0];
                        path_transform = (const PathTransform *)((((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) != 0 ? (((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) : (PathTransform *)0);
                        y -= path_transform->displacement[1];
                        path_transform = (const PathTransform *)((((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) != 0 ? (((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) : (PathTransform *)0);
                        z -= path_transform->displacement[2];
                    }
                }
                trigger_path = (((const SceneTrigger *)other)->parameters.path.target);
                trigger_path = ((const ScenePath *)(trigger_path));
                controller_path = character_controller.path;
                trigger_definition = (trigger_path->definition);
                controller_definition = controller_path->definition;
                return trigger_definition != controller_definition || x != 0 || y != 0 || z != 0;
            case 10:
            case 11:
                return character_controller.timers.contact_impulse_ticks == 0;
            case 15:
                return (this->node.flags & 1) == 0;
            case 16:
                return (this->node.flags & 1) != 0;
            case 17:
                if (screen_render.transition_ticks != 0)
                    return 0;
                return sequence_playback.current_handle != 0;
            case 21:
                value = ((const SceneTrigger *)other)->parameters.words[0] + 1;
                if ((sint32)value < 2)
                    value = (character_controller.motion.form_flags >> 20) & 0xF;
                value = ResolveCharacterFormVariant((sint32)value);
                return value != ((uint16)(character_controller.motion.form_flags >> 16) & 0xF);
            default:
                mode = (((const SceneTrigger *)other)->parameters.words[1] >> 12) & 0x1F;
                if ((sint32)mode - 1 < 3 || (uint32)(other->state_flags >> 8) - 40 >= 16)
                    return 0;
                if ((other->flags & 0x80) != 0 || ((((const SceneTrigger *)other)->parameters.words[0] >> 24) & 0xFE) != 0)
                    return 0;
                switch (mode)
                {
                    case 4:
                        return character_controller.secondary_flags & 0x40;
                    case 5:
                    case 6:
                    case 15:
                    case 16:
                        return 1;
                    case 7:
                        return character_controller.motion.horizontal_speed != 0;
                    case 8:
                        return (gameplay_input.buttons & 0x8000) != 0 && (this->flags & 0x8000000) == 0;
                    case 9:
                        return (gameplay_input.buttons & 0x2000) != 0 && (this->flags & 0x8000000) == 0;
                    case 10:
                        if ((sint32)character_controller.flags >= 0)
                        {
                            value = (*control_bindings.active_scheme)[1];
                            return (gameplay_input.buttons & value) != 0;
                        }
                        return (gameplay_input.buttons & 0x40) != 0 && (gameplay_input.prev_buttons & 0x40) == 0;
                    case 11:
                        if ((sint32)character_controller.flags < 0)
                            return 0;
                        return (gameplay_input.buttons >> 14) & 1;
                    case 12:
                        return (gameplay_input.buttons >> 12) & 1;
                    case 13:
                        return (gameplay_input.buttons >> 2) & 1;
                    case 14:
                        return (gameplay_input.buttons >> 3) & 1;
                    case 17:
                        value = (*control_bindings.active_scheme)[0];
                        return (gameplay_input.buttons & value) != 0;
                    case 18:
                        if (((uint16)(character_controller.motion.form_flags >> 16) & 0xF) != 1)
                            return 0;
                        return (sint8)character_controller.motion.state_code == 9;
                    default:
                        return 0;
                }
        }
    }
    if (category == 0)
    {
        kind = (sint8)((const AnimatedObject *)other)->metadata126[0];
        if (kind == 2)
        {
            if ((sint8)((const AnimatedObject *)other)->behavior != 24)
                return 1;
            if ((((((const AnimatedObject *)other)->path_definition) != 0 ? (((const AnimatedObject *)other)->path_definition) : (const PathDefinition *)0)->flags & 0x40) != 0)
                return 1;
        }
        if (kind == 3)
            return (sint8)((const AnimatedObject *)other)->animation_binding.word != -1;
        return 0;
    }
    if (category != 8)
        return 0;
    if ((character_controller.secondary_flags & 0x240) != 0)
    {
        if (character_controller.pickup_count >= 12)
            return 0;
        value = ((const BillboardNode *)other)->playback_flags & 0x7F;
        return value < 6 || value == 8;
    }
    if ((this->flags & 0x8000000) != 0 && ((uint16)(character_controller.motion.form_flags >> 16) & 0xF) == 4)
        return 0;
    return character_controller.pickup_count < 12;
}

void QueueCharacterPickup(SceneNode *object, uint32 context_offset)
{
    FUNCTION_MARKER(0x80047D18, "SLUS_005.78");
    sint32 index = character_controller.pickup_count;
    if (index < 12)
    {
        uint8 old_count;
        character_controller.pickup_objects[index] = object;
        old_count = (uint8)character_controller.pickup_count;
        character_controller.pickup_count = old_count + 1;
        character_controller.pickup_fractions[(sint8)old_count] = context_offset;
    }
}

uint32 AnimatedObject::ReadObjectImpulseScale() const
{
    FUNCTION_MARKER(0x80030A34, "SLUS_005.78");
    void *data;
    const ActorModelDefinition *bound_model;
    const PathDefinition *parent;
    AnimationHeader *animation;
    parent = (this->path_definition);
    if (parent == 0)
        return 0;
    data = ((const ActorPlacement *)parent)->state;
    if ((((const ActorBinding *)(data))->flags & 2) == 0)
        return 0;
    bound_model = ((const ActorBinding *)(data))->model;
    animation = this->animation;
    return animation == (bound_model->animations[4]) ? 5324 : 5734;
}

uint32 AnimatedObject::CollectCharacterContact(SceneNode *other_node, CollisionContact *contact_data)
{
    const ScenePath *trigger_path;
    const PathDefinition *parent;
    const PathDefinition *object_definition;
    const PathTransform *captured_path_transform;
    FUNCTION_MARKER(0x80047D6C, "SLUS_005.78");
    SceneNode *other = (SceneNode *)(other_node);
    CollisionTransform descriptor = {0};
    uint32 state = 0x800ABD68;
    uint32 contact, category, event, flags, value, mask, x, y, z, index, count, result = 0;
    const PathTransform *path_transform;
    sint32 status, kind;
    if (contact_data == 0)
        return (uint32)(sint32)(sint8)this->CanCharacterContact(other_node);
    category = other->kind;
    contact = contact_data->fraction;
    if (category == 1)
        goto surface_descriptor;
    if (category == 0)
        goto ordinary_contact;
    if (category == 8)
    {
        index = (uint32)(sint32)character_controller.pickup_count;
        if ((sint32)index < 12)
        {
            character_controller.pickup_objects[(sint32)index] = other_node;
            count = (uint8)character_controller.pickup_count;
            character_controller.pickup_count = count + 1;
            character_controller.pickup_fractions[(sint8)count] = contact;
        }
        goto done;
    }
    if (category != 2)
        goto done;
    QueueContact(&character_contact_pool.cur, other_node);
    if (ContainsContact(character_contact_pool.prev, other_node) != 0 && ((((const SceneTrigger *)other)->parameters.words[1] >> 12) & 0x1F) == 6)
        goto done;
    event = (other->state_flags >> 8);
    switch (event)
    {
        case 1:
        case 24:
            if ((this->TestCharacterSurfaceContact((const SceneTrigger *)other_node, contact) & 0xFF) == 0)
                goto done;
            flags = character_controller.flags;
            if ((flags & 1) != 0 && (sint32)contact < (sint32)character_controller.support.fraction)
                goto done;
            value = character_controller.secondary_flags;
            character_controller.flags = flags | 1;
            character_controller.support.candidate = (SceneTrigger *)other_node;
            character_controller.support.fraction = contact;
            if (event == 1)
                character_controller.secondary_flags = value & 0xFFFFFFF6;
            else
            {
                character_controller.secondary_flags = (value | 1) & 0xFFFFFFF7;
                if ((this->path_flags & 6) != 2)
                    character_controller.surface_interaction.surface = (SceneTrigger *)other_node;
                if ((character_controller.secondary_flags & 4) != 0)
                    character_controller.surface_interaction.prev_surface_step = character_controller.surface_interaction.surface_step;
            }
            goto done;
        case 2:
            flags = character_controller.flags;
            character_controller.contacts.section_trigger = (SceneTrigger *)other_node;
            character_controller.contacts.section_fraction = contact;
            character_controller.flags = flags | 0x4000;
            goto done;
        case 3:
        case 17:
            value = ((const SceneTrigger *)other)->control_word;
            if (value == 87)
            {
                screen_render.brighten_duration = 30;
                screen_render.brighten_ticks = 15;
            }
            else if (value == 66)
            {
                screen_render.darken_duration = 30;
                screen_render.darken_ticks = 15;
            }
            flags = character_controller.flags | 2;
            mask = 0xFCC5F45B;
            goto moving_event;
        case 4:
            flags = character_controller.flags;
            character_controller.surface_interaction.environment = (SceneTrigger *)other_node;
            character_controller.motion.environment_acceleration = contact;
            character_controller.flags = flags | 8;
            goto done;
        case 5:
        case 18:
            flags = character_controller.flags;
            character_controller.support.contact = (SceneNode *)other_node;
            character_controller.contacts.damage_fraction = contact;
            character_controller.flags = flags | 0x400;
            goto done;
        case 6:
            character_controller.flags = (character_controller.flags | 0x10) & 0xFCC5F459;
            character_controller.support.impulse_surface = (SceneTrigger *)other_node;
            value = contact_data->selection & 0x8FFF;
            goto indexed_event;
        case 7:
            flags = character_controller.flags | 0x20;
            mask = 0xFCC5F479;
            goto moving_event;
        case 8:
            QueueCutsceneTrigger((SceneTrigger *)other_node, contact);
            goto done;
        case 9:
            flags = character_controller.flags;
            if (contact == 0 && (flags & 0x40000) != 0)
            {
                character_controller.flags = flags | 0x20000;
                goto done;
            }
            character_controller.flags = ((flags | 0x20000) & 0xFCC7F459) & 0xFFFBFFFF;
            character_controller.surface_interaction.selected = other_node;
            value = contact_data->selection & 0x8FFF;
            goto indexed_event;
        case 10:
            flags = character_controller.flags;
            character_controller.surface_interaction.jump_scale = (SceneTrigger *)other_node;
            character_controller.contacts.jump_scale_fraction = contact;
            character_controller.flags = flags | 0x800000;
            goto done;
        case 11:
            flags = character_controller.flags;
            value = character_controller.secondary_flags;
            character_controller.surface_interaction.jump_scale = (SceneTrigger *)other_node;
            character_controller.contacts.jump_scale_fraction = contact;
            character_controller.flags = flags | 0x800000;
            character_controller.secondary_flags = value | 0x80;
            goto done;
        case 15:
            flags = character_controller.flags;
            character_controller.contacts.hide_trigger = (SceneTrigger *)other_node;
            character_controller.contacts.hide_fraction = contact;
            character_controller.flags = flags | 0x1000000;
            goto done;
        case 16:
            flags = character_controller.flags;
            character_controller.contacts.show_trigger = (SceneTrigger *)other_node;
            character_controller.contacts.show_fraction = contact;
            character_controller.flags = flags | 0x2000000;
            goto done;
        case 19:
            if (contact != 0)
                goto queue_event;
            value = ((const SceneTrigger *)other)->parameters.words[0];
            camera_projection.far_distance_copy = value;
            camera_projection.far_distance = value;
            goto done;
        case 20:
            if (contact != 0)
                goto queue_event;
            character_controller.motion.trail_effect = ((const SceneTrigger *)other)->parameters.words[0];
            goto done;
        case 21:
            if (contact != 0)
                goto queue_event;
            value = ((const SceneTrigger *)other)->parameters.words[0] + 1;
            if ((sint32)value < 2)
                value = (character_controller.motion.form_flags >> 20) & 0xF;
            value = ResolveCharacterFormVariant((sint32)value) << 24;
            character_controller.motion.form_flags = (character_controller.motion.form_flags & 0xF0FFFFFF) | value;
            goto done;
        case 22:
            if (contact == 0)
                this->flags = this->flags | 0x40000;
            goto done;
        case 23:
            if (contact != 0)
                goto queue_event;
            this->node.state_flags = ((const SceneTrigger *)other)->parameters.words[0];
            goto done;
        case 26:
        case 27:
            if (character_controller.path_transition.reentry_ticks != 0)
                goto done;
            flags = this->path_flags;
            if ((flags & 0x80) != 0 || (character_controller.flags & 0x60000) != 0)
                goto done;
            if (((this->flags & 0x400) == 0 || character_controller.path_transition.movement_ticks < 10) && (flags & 1) == 0)
                goto done;
            trigger_path = (((const SceneTrigger *)other)->parameters.path.target);
            trigger_path = ((const ScenePath *)(trigger_path));
            parent = (trigger_path->definition);
            object_definition = (this->path_definition);
            if (parent == object_definition && (this->flags & 0x2000) == 0)
                goto done;
            flags = this->path_flags;
            if (event == 26)
                flags = (flags | 0x80) & 0xFFFFFBFF;
            else
                flags |= 0x480;
            this->path_flags = flags;
            character_controller.path_transition.anchor = (SceneTrigger *)other_node;
            character_controller.path_transition.anchor_fraction = contact;
            character_controller.path_transition.phase = 0;
            goto done;
        case 28:
            goto queue_event;
        case 30:
        {
            flags = character_controller.secondary_flags | 0x400;
            character_controller.secondary_flags = flags;
            value = ((const SceneTrigger *)other)->parameters.words[0];
            const CharacterModelRecord *event_model = character_controller.model;
            AnimationHeader *active_animation = this->animation;
            if (active_animation == event_model->animations[value])
                this->frame = 0;
            else if ((flags & 0x80) == 0)
                character_controller.motion.state_code = value;
            goto done;
        }
        default:
            if (contact != 0)
                goto queue_event;
            other->state_flags = (other->state_flags & 0xFF00) | (uint8)((other->state_flags & 0xFF) | 8);
            goto done;
    }
moving_event:
    character_controller.flags = flags & mask;
    character_controller.surface_interaction.selected = other_node;
    x = ((const SceneTrigger *)other)->delta[0];
    y = ((const SceneTrigger *)other)->delta[1];
    z = ((const SceneTrigger *)other)->delta[2];
    character_controller.support.motion[0] = x;
    character_controller.support.motion[1] = y;
    character_controller.support.motion[2] = z;
    result = 1;
    goto done;
indexed_event:
    character_controller.support.contact_direction = value;
    x = ((const SceneTrigger *)other)->delta[0];
    y = ((const SceneTrigger *)other)->delta[1];
    z = ((const SceneTrigger *)other)->delta[2];
    character_controller.support.motion[0] = x;
    character_controller.support.motion[1] = y;
    character_controller.support.motion[2] = z;
    result = 1;
    goto done;
queue_event:
    index = (uint32)(sint32)character_controller.trigger_count;
    if ((sint32)index < 12)
    {
        character_controller.trigger_objects[(sint32)index] = other_node;
        count = (uint8)character_controller.trigger_count;
        character_controller.trigger_count = count + 1;
        character_controller.trigger_fractions[(sint8)count] = contact;
    }
    goto done;
surface_descriptor:
    SelectCollisionFace(other_node, contact_data->selection & 0xFFFF8FFF, &descriptor);
    if ((this->AcceptVerticalFaceContact(&descriptor, contact_data->selection) & 0xFF) != 0)
        goto done;
    character_controller.flags = (character_controller.flags | 4) & 0xFCC5F45D;
    character_controller.surface_interaction.selected = other_node;
    this->behavior_word = contact_data->selection;
    x = descriptor.fields.motion[0];
    y = descriptor.fields.motion[1];
    z = descriptor.fields.motion[2];
    character_controller.support.motion[0] = x;
    character_controller.support.motion[1] = y;
    character_controller.support.motion[2] = z;
    result = 1;
    goto done;
ordinary_contact:
    flags = ((const AnimatedObject *)other)->flags;
    if ((flags & 0x100000) != 0)
        goto surface_descriptor;
    if ((flags & 0x200) != 0 && (sint8)((const AnimatedObject *)other)->metadata126[0] == 2)
        goto done;
    if ((sint8)((const AnimatedObject *)other)->behavior == 17)
        goto moving_platform;
    kind = (sint8)((const AnimatedObject *)other)->metadata126[0];
    if (kind == 2)
    {
        if ((((const AnimatedObject *)other)->flags & 0x30000000) != 0)
            goto moving_platform;
        kind = (sint8)((const AnimatedObject *)other)->metadata126[0];
    }
    if (kind == 3 && ((const AnimatedObject *)other)->state == 30)
    {
        character_controller.motion.slope_scale = 4096;
        goto done;
    }
    status = this->ClassifyCharacterContactResponse();
    kind = (sint8)((const AnimatedObject *)other)->metadata126[0];
    if (kind != 2)
        goto fallback;
    if (status == 0 && (character_controller.secondary_flags & 0x100) != 0 && character_action_target == (const AnimatedObject *)other_node && (character_action_flags & 8) != 0)
    {
        status = -1;
        kind = (sint8)((const AnimatedObject *)other)->metadata126[0];
    }
    if (kind != 2)
        goto fallback;
    if (status == 0 && (this->path_flags & 1) != 0)
        status = this->ClassifyConstrainedContact((const AnimatedObject *)other);
    if ((sint8)((const AnimatedObject *)other)->metadata126[0] != 2)
        goto fallback;
    if (status == 0 && ((const AnimatedObject *)other)->ReadObjectImpulseScale() != 0)
        status = -1;
    if (contact == 0 && (this->flags & 0x2000000) != 0)
        goto fallback;
    if ((contact_data->flags & 1) == 0 && ((sint8)((const AnimatedObject *)other)->behavior != 20 || (((const AnimatedObject *)other)->flags & 0x40) != 0))
        goto fallback;
    flags = this->flags;
    if ((flags & 0x8000000) != 0 && ((uint16)(character_controller.motion.form_flags >> 16) & 0xF) == 4)
        goto fallback;
    if (status > 0)
        goto slide_contact;
    if (contact_data->other_selection < 0x800 && status == 0)
        goto attach_contact;
    if ((flags & 0x10000000) != 0 || (((const AnimatedObject *)other)->flags & 0x4400000) == 0x400000)
        goto fallback;
slide_contact:
    x = ((const AnimatedObject *)other)->velocity[0];
    y = ((const AnimatedObject *)other)->velocity[1];
    z = ((const AnimatedObject *)other)->velocity[2];
    character_controller.support.motion[0] = x;
    character_controller.support.motion[1] = y;
    character_controller.support.motion[2] = z;
    if ((((const AnimatedObject *)other)->flags & 0x2000) == 0)
    {
        parent = (((const AnimatedObject *)other)->path_definition);
        if (parent != 0)
        {
            path_transform = ((captured_path_transform = ((parent)->transform)) != 0 ? (const PathTransform *)((PathTransform *)(captured_path_transform)) : 0);
            if (path_transform != 0)
            {
                character_controller.support.motion[0] = character_controller.support.motion[0] + path_transform->displacement[0];
                path_transform = ((const AnimatedObject *)other)->path_definition->transform;
                character_controller.support.motion[1] = character_controller.support.motion[1] + path_transform->displacement[1];
                path_transform = ((const AnimatedObject *)other)->path_definition->transform;
                character_controller.support.motion[2] = character_controller.support.motion[2] + path_transform->displacement[2];
            }
        }
    }
    if ((this->flags & 0x400) != 0)
    {
        character_controller.flags = (character_controller.flags | 0x800) & 0xFCC5FC59;
        character_controller.support.impulse_scale = ((const AnimatedObject *)other)->ReadObjectImpulseScale();
        result = contact != 0;
        goto done;
    }
    if (((const AnimatedObject *)other)->ReadObjectImpulseScale() == 0)
    {
        if ((sint8)character_controller.path_transition.contact_delay <= 0)
            QueueCharacterActionSound(7, 3);
        if (status >= 0 || (sint16)((const AnimatedObject *)other)->ticks == 1)
        {
            result = status == 0;
            goto done;
        }
    }
    contact_data->flags = contact_data->flags & 0xFFFFFFFD;
fallback:
    if (status > 0)
        goto done;
attach_contact:
    if ((contact_data->flags & 2) == 0)
        goto ordinary_attachment;
    flags = this->flags;
    if ((flags & 0x2000000) != 0 || ((flags & 0x8000000) != 0 && ((uint16)(character_controller.motion.form_flags >> 16) & 0xF) == 4))
        goto ordinary_attachment;
    character_controller.flags = (character_controller.flags | 0x400) & 0xFCC5F459;
    character_controller.support.contact = (SceneNode *)other_node;
    kind = (sint8)((const AnimatedObject *)other)->metadata126[0];
    if (kind == 2)
        character_controller.vitals.damage_amount = ((const AnimatedObject *)other)->GetActorContactDamage();
    else if (kind == 3)
        character_controller.vitals.damage_amount = GetModelGroupContactType(((const AnimatedObject *)other)->state);
    goto copy_attachment;
ordinary_attachment:
    flags = character_controller.flags;
    if ((flags & 0x40000) != 0 || (this->flags & 0x2000000) != 0)
        goto done;
    character_controller.flags = ((flags | 0x20000) & 0xFCC7F459) & 0xFFFBFFFF;
    character_controller.support.contact = (SceneNode *)other_node;
copy_attachment:
    x = ((const AnimatedObject *)other)->velocity[0];
    y = ((const AnimatedObject *)other)->velocity[1];
    z = ((const AnimatedObject *)other)->velocity[2];
    character_controller.support.motion[0] = x;
    character_controller.support.motion[1] = y;
    character_controller.support.motion[2] = z;
    parent = (((const AnimatedObject *)other)->path_definition);
    if (parent != 0)
    {
        path_transform = ((captured_path_transform = ((parent)->transform)) != 0 ? (const PathTransform *)((PathTransform *)(captured_path_transform)) : 0);
        if (path_transform != 0 && (((const AnimatedObject *)other)->flags & 0x2000) == 0)
        {
            character_controller.support.motion[0] = character_controller.support.motion[0] + path_transform->displacement[0];
            path_transform = ((const AnimatedObject *)other)->path_definition->transform;
            character_controller.support.motion[1] = character_controller.support.motion[1] + path_transform->displacement[1];
            path_transform = ((const AnimatedObject *)other)->path_definition->transform;
            character_controller.support.motion[2] = character_controller.support.motion[2] + path_transform->displacement[2];
        }
    }
    result = (sint8)((const AnimatedObject *)other)->metadata126[0] != 3;
    goto done;
moving_platform:
    if ((this->flags & 0x400) == 0)
        goto done;
    character_controller.flags = (character_controller.flags | 0x200800) & 0xFCE5FC59;
    path_transform = ((const AnimatedObject *)other)->path_definition->transform;
    result = 1;
    if (path_transform != 0)
    {
        character_controller.support.motion[0] = character_controller.support.motion[0] + path_transform->displacement[0];
        path_transform = ((const AnimatedObject *)other)->path_definition->transform;
        character_controller.support.motion[1] = character_controller.support.motion[1] + path_transform->displacement[1];
        path_transform = ((const AnimatedObject *)other)->path_definition->transform;
        character_controller.support.motion[2] = character_controller.support.motion[2] + path_transform->displacement[2];
    }
done:
    return result;
}

uint32 AnimatedObject::ProcessCharacterPickups()
{
    SceneSection *section;
    CharacterModelRecord *state_record;
    CharacterController *character = &character_controller;
    FUNCTION_MARKER(0x800506D4, "SLUS_005.78");
    SceneNode *event;
    uint32 kind, marker, result, index, value, flags, total, action, previous, form;
    sint32 amount, first, second;
    first = character_controller.pickup_count;
    value = (uint8)character_controller.pickup_count;
    if (first == 0)
        return 0;
    result = value - 1;
    do
    {
        character_controller.pickup_count = result;
        index = (uint32)(sint32)(sint8)result;
        value = character_controller.pickup_fractions[(sint32)index];
        if ((sint32)this->path_motion_scale < (sint32)value)
            goto next_event;
        index = (uint32)(sint32)character_controller.pickup_count;
        event = character_controller.pickup_objects[(sint32)index];
        kind = ((const uint8 *)event)[18] & 0x7F;
        value = event->flags;
        event->flags = value | 0x80;
        marker = kind;
        switch (kind)
        {
            case 0:
                value = character_controller.vitals.lives;
                if (value < 32)
                    character_controller.vitals.lives = value + 1;
                break;
            case 1:
                first = (sint8)character_controller.vitals.health;
                second = (sint8)character_controller.vitals.health_limit;
                if (first < second)
                    character_controller.vitals.health = first + 1;
                break;
            case 2:
                if ((sint8)character_controller.vitals.health_limit < 21)
                {
                    value = level_resource.level_id;
                    if (value == 75)
                        character_controller.vitals.health_pickups = character_controller.vitals.health_pickups | 1;
                    else if (value == 10)
                        character_controller.vitals.health_pickups = character_controller.vitals.health_pickups | 2;
                    else if (value == 20)
                        character_controller.vitals.health_pickups = character_controller.vitals.health_pickups | 4;
                    character_controller.vitals.health_limit = character_controller.vitals.health_limit + 4;
                }
                first = (sint8)character_controller.vitals.health;
                second = (sint8)character_controller.vitals.health_limit;
                if (second - 4 >= first)
                    character_controller.vitals.health = first + 4;
                break;
            case 3:
            case 4:
            case 5:
                index = xport_load_le16((const uint8 *)event + 18) & 0x7F;
                section = scene_stream.active_section;
                amount = pickup_scores[index - 3];
                section->collected_count = (uint16)(section->collected_count + amount);
                flags = character_controller.flags;
                total = character_controller.vitals.score;
                total += (flags & 0x400000) != 0 ? (uint32)amount * 2 : (uint32)amount;
                index = (uint32)(sint32)character_controller.pickup_count;
                character_controller.vitals.score = total;
                if (character_controller.pickup_fractions[(sint32)index] == 0 && (player_interaction.flags & 1) != 0)
                {
                    flags = character_controller.flags;
                    total += (flags & 0x400000) != 0 ? (uint32)amount * 2 : (uint32)amount;
                    character_controller.vitals.score = total;
                }
                break;
            case 6:
                if (((uint16)(character_controller.motion.form_flags >> 16) & 15) == 1)
                {
                    state_record = character_controller.model;
                    AnimationHeader *animation22 = state_record->animations[22];
                    AnimationHeader *animation2 = state_record->animations[2];
                    state_record->animations[2] = animation22;
                    state_record = character_controller.model;
                    state_record->animations[22] = animation2;
                    SpawnActionEffect(this, &character_controller, 7);
                    character_controller.secondary_flags = character_controller.secondary_flags | 0x40;
                    this->flags = this->flags | 0x20000000;
                    character_controller.timers.model_swap_ticks = 450;
                    break;
                }
                goto change_form;
            case 7:
                if (((uint16)(character_controller.motion.form_flags >> 16) & 15) == 1)
                {
                    character_controller.secondary_flags = character_controller.secondary_flags | 0x200;
                    this->flags = this->flags | 0x20000000;
                    character_controller.timers.color_effect_ticks = 450;
                    break;
                }
            change_form:
                kind = (uint16)(character_controller.motion.form_flags >> 16) & 15;
                if (kind == 4 || kind == 5 || kind - 10 < 2)
                {
                    marker = 0xFFFFFFFF;
                    break;
                }
                form = marker == 7 ? 11 : 0;
                marker = 6;
                if ((character_controller.secondary_flags & 0x100) != 0)
                {
                    character->EndInteraction();
                    character_controller.RemoveAction(6);
                }
                else
                {
                    value = character_controller.weapon_state;
                    if ((value & 0xFFFF) != 0)
                    {
                        previous = value >> 16;
                        if (previous == form)
                            goto store_form;
                        character_controller.RemoveAction(previous == 0 ? 3 : previous - 7);
                    }
                }
                SpawnActionEffect(this, &character_controller, form == 0 ? 3 : form - 7);
            store_form:
                character_controller.weapon_state = (form << 16) | 1;
                break;
            case 8:
                marker -= 2;
                break;
            case 10:
                flags = character_controller.secondary_flags;
                if ((flags & 0x120) == 0)
                {
                    if (((uint16)(character_controller.motion.form_flags >> 16) & 15) == 1)
                        character_controller.secondary_flags = flags | 0x20;
                    else
                    {
                        SpawnActionEffect(this, &character_controller, 6);
                        BeginCharacterInteraction(this);
                        value = character_controller.weapon_state;
                        if ((value & 0xFFFF) != 0)
                        {
                            previous = value >> 16;
                            character_controller.RemoveAction(previous == 0 ? 3 : previous - 7);
                        }
                        character_controller.weapon_state = 0;
                    }
                }
                break;
            case 11:
                SpawnActionEffect(this, &character_controller, 3);
                character_controller.weapon_state = 1;
                break;
        }
        while (character_controller.vitals.score >= 500)
        {
            value = character_controller.vitals.lives;
            if (value < 32)
                character_controller.vitals.lives = value + 1;
            value = character_controller.vitals.score;
            character_controller.vitals.score = value - 500;
            QueueCharacterActionSound(17, 3);
        }
        SpawnEffectSprite((const uint32 *)((const uint8 *)event + 40), xport_load_le32((const uint8 *)event + 24), 6);
        if ((sint8)marker != -1)
        {
            kind = xport_load_le16((const uint8 *)event + 18) & 0x7F;
            switch (kind)
            {
                case 0:
                    action = 17;
                    break;
                case 1:
                case 2:
                    action = 18;
                    break;
                case 3:
                case 4:
                case 5:
                    action = 19;
                    break;
                case 6:
                case 7:
                    action = 20;
                    break;
                case 8:
                    action = 21;
                    break;
                default:
                    action = ((uint16)(character_controller.motion.form_flags >> 16) & 15) == 1 ? 11 : 15;
                    break;
            }
            QueueCharacterActionSound(action, 3);
        }
        if (xport_load_le16((const uint8 *)event + 16) != 0)
            xport_store_le16((uint8 *)event + 18, xport_load_le16((const uint8 *)event + 18) | 0x8000);
    next_event:
        first = character_controller.pickup_count;
        value = (uint8)character_controller.pickup_count;
        result = value - 1;
    } while (first != 0);
    return result;
}

void AnimatedObject::ProcessCharacterTriggers()
{
    SceneSection *section;
    CharacterController *character = &character_controller;
    FUNCTION_MARKER(0x80050F60, "SLUS_005.78");
    const ActorModelDefinition *bound_model;
    SceneNode *event;
    uint32 flags, object_flags, value, previous, index, count, scene, result;
    sint32 kind, health, active;
    flags = character_action_flags;
    if ((flags & 0x100) != 0)
    {
        const PathDefinition *hit_definition = character_action_target->path_definition;
        const void *hit_state = ((const ActorPlacement *)((const PathDefinition *)(hit_definition)))->state;
        value = character_controller.flags;
        bound_model = ((const ActorBinding *)(hit_state))->model;
        character_controller.flags = value | 0x400;
        value = bound_model->action_damage;
        character_action_flags = flags & 0xFFFFFEFF;
        character_controller.vitals.damage_amount = value;
        goto take_damage;
    }
    if ((character_controller.flags & 0x80000) != 0)
    {
        object_flags = this->flags;
        if ((object_flags & 0x2000000) == 0 && (object_flags & 0x4400) != 0x400)
        {
            character_controller.vitals.damage_amount = 1;
            goto take_damage;
        }
    }
    flags = character_controller.flags;
    if ((flags & 0x400) == 0)
        goto timed_flags;
    event = (SceneNode *)(character_controller.support.contact);
    if (event->kind != 0)
    {
        value = character_controller.contacts.damage_fraction;
        if ((sint32)this->path_motion_scale < (sint32)value)
            goto timed_flags;
        if ((sint8)(event->state_flags >> 8) == 18)
        {
            flags = character_controller.flags;
            character_controller.vitals.health = 0;
            character_controller.flags = flags | 0x80;
        }
        else
        {
            value = xport_load_le32((const uint8 *)event + 76);
            event = (SceneNode *)(character_controller.support.contact);
            character_controller.vitals.damage_amount = value;
            value = xport_load_le16((const uint8 *)event + 78);
            value += frame_clock.render_tick;
            xport_store_le32((uint8 *)event + 68, value);
        }
    }
    else if ((this->flags & 0x2000400) != 0x2000000)
        character_controller.flags = flags | 0x20000;
    if ((sint8)character_controller.vitals.health <= 0 || (this->flags & 0x2000000) != 0)
        goto timed_flags;
take_damage:
    if ((game_flow.transition_flags & 0x10000) != 0 && (uint32)level_resource.level_id - 96 >= 2 && (this->flags & 0x22000000) == 0)
    {
        value = character_controller.vitals.damage_amount;
        health = (sint8)character_controller.vitals.health;
        if ((sint32)value >= health)
        {
            value = (uint32)health;
            if (health >= 2)
                --value;
        }
        character_controller.vitals.health = character_controller.vitals.health - value;
    }
    QueueCharacterActionSound(2, 3);
    value = character_controller.movement_flags;
    flags = game_flow.mode_flags;
    character_controller.movement_flags = value | 0x4000000;
    if ((flags & 0x10) == 0)
    {
        if ((character_controller.secondary_flags & 0x20) != 0)
        {
            value = player_interaction.flags;
            flags = character_controller.secondary_flags;
            player_interaction.flags = value & 0xFFFFFFDB;
            character_controller.secondary_flags = flags & 0xFFFFFFDF;
        }
        if (((game_flow.mode_flags & 0x200) == 0 || ((uint16)(character_controller.motion.form_flags >> 16) & 15) != 0) && (character_controller.secondary_flags & 0x100) != 0)
        {
            character_controller.RemoveAction(6);
            character->EndInteraction();
        }
        value = character_controller.weapon_state;
        if ((value & 0xFFFF) != 0)
        {
            previous = value >> 16;
            character_controller.RemoveAction(previous == 0 ? 3 : previous - 7);
        }
        character_controller.weapon_state = 0;
    }
    if ((this->flags & 0x2000000) == 0)
    {
        character_controller.vitals.damage_ticks = 60;
        this->flags = this->flags | 0x2080000;
    }
    if ((character_controller.flags & 0x80000) == 0)
    {
        SpawnActionEffect(this, &character_controller, 0);
        if ((this->path_flags & 0x101) == 0 && (character_controller.secondary_flags & 0x80) == 0)
            character_controller.motion.state_code = 4;
    }
    character_controller.flags = character_controller.flags & 0xEFFFFFFF;
timed_flags:
    if ((character_controller.flags & 0x800000) != 0)
    {
        value = character_controller.contacts.jump_scale_fraction;
        if ((sint32)this->path_motion_scale < (sint32)value)
        {
            value = character_controller.secondary_flags;
            flags = character_controller.flags;
            character_controller.secondary_flags = value & 0xFFFFFF7F;
            character_controller.flags = flags & 0xFF7FFFFF;
        }
    }
    if ((character_controller.flags & 8) != 0)
    {
        value = character_controller.motion.environment_acceleration;
        if ((sint32)this->path_motion_scale < (sint32)value)
        {
            flags = character_controller.flags;
            character_controller.motion.environment_acceleration = 0;
            character_controller.flags = flags & 0xFFFFFFF7;
        }
    }
    flags = character_controller.flags;
    if ((flags & 0x3000000) == 0x3000000)
    {
        value = character_controller.contacts.hide_fraction;
        previous = character_controller.contacts.show_fraction;
        if ((sint32)previous >= (sint32)value && (sint32)this->path_motion_scale >= (sint32)value)
            character_controller.flags = flags & 0xFDFFFFFF;
        else
        {
            previous = character_controller.contacts.show_fraction;
            value = character_controller.contacts.hide_fraction;
            if ((sint32)value >= (sint32)previous && (sint32)this->path_motion_scale >= (sint32)previous)
                character_controller.flags = character_controller.flags & 0xFEFFFFFF;
        }
    }
    if ((character_controller.flags & 0x1000000) != 0)
    {
        value = character_controller.contacts.hide_fraction;
        if ((sint32)this->path_motion_scale >= (sint32)value)
        {
            this->node.flags = this->node.flags | 1;
            goto timed_scene;
        }
    }
    if ((character_controller.flags & 0x2000000) != 0)
    {
        value = character_controller.contacts.show_fraction;
        if ((sint32)this->path_motion_scale >= (sint32)value)
            this->node.flags = this->node.flags & 0xFE;
    }
timed_scene:
    if ((character_controller.flags & 0x4000) != 0)
    {
        value = character_controller.contacts.section_fraction;
        if ((sint32)this->path_motion_scale >= (sint32)value && (RequestSceneSection(((const SceneTrigger *)(character_controller.contacts.section_trigger))->parameters.words[0]) << 24) != 0)
        {
            section = scene_stream.active_section;
            section->state_flags = section->state_flags & 0xFFEF;
        }
    }
    active = character_controller.trigger_count;
    count = (uint8)character_controller.trigger_count;
    while (active != 0)
    {
        value = count - 1;
        character_controller.trigger_count = value;
        index = (uint32)(sint32)(sint8)value;
        value = character_controller.trigger_fractions[(sint32)index];
        if ((sint32)this->path_motion_scale >= (sint32)value)
        {
            event = character_controller.trigger_objects[(sint32)index];
            if (event != 0)
            {
                kind = (sint8)(event->state_flags >> 8);
                if (kind >= 40)
                    event->state_flags = event->state_flags | 8;
                else
                {
                    switch (kind)
                    {
                        case 19:
                            scene = OriginalDefaults::SceneContextIdentity;
                            value = xport_load_le32((const uint8 *)event + 68);
                            camera_projection.far_distance_copy = value;
                            camera_projection.far_distance = value;
                            break;
                        case 20:
                            character_controller.motion.trail_effect = xport_load_le32((const uint8 *)event + 68);
                            break;
                        case 23:
                            this->node.state_flags = xport_load_le32((const uint8 *)event + 68);
                            break;
                        case 21:
                            value = xport_load_le32((const uint8 *)event + 68) + 1;
                            if ((sint32)value < 2)
                                value = (character_controller.motion.form_flags >> 20) & 15;
                            result = ResolveCharacterFormVariant((sint32)value);
                            value = character_controller.motion.form_flags;
                            character_controller.motion.form_flags = (value & 0xF0FFFFFF) | (result << 24);
                            break;
                    }
                }
            }
        }
        active = character_controller.trigger_count;
        count = (uint8)character_controller.trigger_count;
    }
    StartPendingCutscene(this);
}

uint32 AnimatedObject::TestCharacterSurfaceContact(const SceneTrigger *trigger, uint32 contact) const
{
    const PathTransform *captured_path_transform;
    FUNCTION_MARKER(0x800471F4, "SLUS_005.78");
    const SceneTrigger *other = (const SceneTrigger *)(trigger);
    CollisionSphere bounds;
    uint32 delta[3], extent;
    const PathDefinition *parent;
    uint32 x, y, z, result, value;
    const PathTransform *path_transform;
    if ((sint8)(other->node.state_flags >> 8) == 24 && (this->path_flags & 4) != 0 && character_controller.surface_interaction.prev_surface == trigger)
    {
        value = path_transition_rules.early_movement_ticks;
        if (character_controller.path_transition.movement_ticks < value)
            return 0;
    }
    if (contact != 0)
        return 1;
    if ((this->flags & 0x2800) != 0)
        return 1;
    x = 0 - other->delta[0];
    y = 0 - other->delta[1];
    z = 0 - other->delta[2];
    parent = (this->path_definition);
    if (parent != 0)
    {
        path_transform = ((captured_path_transform = ((((const PathDefinition *)(parent)))->transform)) != 0 ? (const PathTransform *)((PathTransform *)(captured_path_transform)) : 0);
        if (path_transform != 0 && (this->flags & 0x2000) == 0)
        {
            x += path_transform->displacement[0];
            path_transform = (const PathTransform *)((((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) != 0 ? (((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) : (PathTransform *)0);
            y += path_transform->displacement[1];
            path_transform = (const PathTransform *)((((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) != 0 ? (((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) : (PathTransform *)0);
            z += path_transform->displacement[2];
        }
    }
    if (x != 0 || y != 0 || z != 0)
        return 1;
    extent = contact;
    x = this->spheres[0].position[0];
    y = this->spheres[0].position[1];
    z = this->spheres[0].position[2];
    bounds.position[0] = x;
    bounds.position[1] = y;
    bounds.position[2] = z;
    bounds.radius = 0;
    x = this->velocity[0];
    x -= other->delta[0];
    delta[0] = x;
    y = this->velocity[1];
    y -= other->delta[1];
    delta[1] = y;
    z = this->velocity[2];
    z -= other->delta[2];
    delta[2] = z;
    result = SweepSphereAgainstObject(&bounds, &other->node, delta, &extent, 0) != 0;
    return result;
}

uint32 ResolveCharacterFormVariant(sint32 kind)
{
    FUNCTION_MARKER(0x800473F0, "SLUS_005.78");
    uint32 current;
    uint32 compatible;
    if (kind != 6 && kind != 7 && kind != 8 && kind != 13)
        return (uint32)kind;
    current = (character_controller.motion.form_flags >> 16) & 15;
    compatible = current == 0 || current == 6 || current == 8 || current == 13 || current == 10;
    switch (kind)
    {
        case 6:
            return compatible ? 6 : 7;
        case 7:
            return compatible ? 8 : 9;
        case 8:
            return compatible ? 10 : 11;
        default:
            return compatible ? 13 : 14;
    }
}

sint32 AnimatedObject::ClassifyCharacterContactResponse() const
{
    CharacterController *character = &character_controller;
    FUNCTION_MARKER(0x80047B1C, "SLUS_005.78");
    uint32 flags = this->flags;
    uint32 character_flags, value;
    sint32 timer;
    if ((flags & 0x20000000) != 0 || (character->secondary_flags & 0x240) != 0)
    {
        character_flags = character->secondary_flags;
        if ((character_flags & 0x40) != 0)
            return 4;
        if ((character_flags & 0x200) != 0)
            return 5;
        timer = (sint16)character_controller.vitals.damage_ticks;
        if (timer >= 150)
            return 3;
        if (timer >= 75)
            value = (uint32)timer & 7;
        else if (timer >= 37)
            value = (uint32)timer & 3;
        else
            value = (uint32)timer & 1;
        return value == 0 ? 2 : 3;
    }
    if ((flags & 0x4000000) != 0 && ((character_controller.motion.form_flags >> 16) & 0xF) == 1)
        return -1;
    if ((sint8)character_controller.motion.prev_state_code == 9)
        return ((character_controller.motion.form_flags >> 16) & 0xF) == 1 ? -1 : 0;
    return 0;
}

union ContactSceneVector
{
    SceneSection *next_section, *current_section;
    VECTOR vector;
    uint32 words[4];
};

uint32 AnimatedObject::ProcessCharacterSceneEvent()
{
    CharacterModelRecord *parent_record;
    FUNCTION_MARKER(0x80050C94, "SLUS_005.78");
    SceneSection *next_section, *current_section;
    ContactSceneVector position, floor, input;
    const PathDefinition *parent;
    uint32 flags, value, x, y, z;
    const PathTransform *path_transform;
    const SceneTrigger *event;
    sint32 frame, index;
    if ((character_controller.flags & 2) == 0)
        return 0;
    event = character_controller.surface_interaction.selected != 0 ? (const SceneTrigger *)character_controller.surface_interaction.selected : (const SceneTrigger *)0;
    if ((sint8)(event->node.state_flags >> 8) == 17)
    {
        this->ResetCharacterForScene(0);
        event = character_controller.surface_interaction.selected != 0 ? (const SceneTrigger *)character_controller.surface_interaction.selected : (const SceneTrigger *)0;
        value = (uint32)MapLevelProgression(event->parameters.words[0], 0);
        flags = game_flow.transition_flags;
        game_flow.requested_level = value;
        game_flow.transition_flags = flags | 8;
        goto finish;
    }
    character_controller.section_height_offset = 0;
    if ((this->flags & 0x2400) == 0x400)
    {
        parent = (this->path_definition);
        if (parent != 0 && ((((const PathDefinition *)(parent)))->transform) != 0)
        {
            x = this->position[0];
            y = this->position[1];
            z = this->position[2];
            input.words[0] = x;
            input.words[1] = y;
            input.words[2] = z;
            PushMatrix();
            path_transform = (const PathTransform *)((((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) != 0 ? (((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) : (PathTransform *)0);
            x = xport_load_le32((const uint8 *)&path_transform->matrix);
            y = xport_load_le32((const uint8 *)&path_transform->matrix + 4);
            xport_gte_write_control(0, x);
            xport_gte_write_control(1, y);
            x = xport_load_le32((const uint8 *)&path_transform->matrix + 8);
            y = xport_load_le32((const uint8 *)&path_transform->matrix + 12);
            z = xport_load_le32((const uint8 *)&path_transform->matrix + 16);
            xport_gte_write_control(2, x);
            xport_gte_write_control(3, y);
            xport_gte_write_control(4, z);
            ApplyRotMatrixLV(&input.vector, &position.vector);
            PopMatrix();
            path_transform = (const PathTransform *)((((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) != 0 ? (((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) : (PathTransform *)0);
            x = position.words[0];
            position.words[0] = x + (uint32)path_transform->matrix.t[0];
            path_transform = (const PathTransform *)((((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) != 0 ? (((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) : (PathTransform *)0);
            y = position.words[1];
            position.words[1] = y + (uint32)path_transform->matrix.t[1];
            path_transform = (const PathTransform *)((((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) != 0 ? (((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) : (PathTransform *)0);
            z = position.words[2];
            position.words[2] = z + (uint32)path_transform->matrix.t[2];
        }
        else
        {
            x = this->position[0];
            y = this->position[1];
            z = this->position[2];
            position.words[0] = x;
            position.words[1] = y;
            position.words[2] = z;
        }
        frame = this->path_frame;
        flags = this->flags;
        parent = (this->path_definition);
        if ((((const PathDefinition *)(parent)))->ProjectFrame(frame, position.words, floor.words, flags & 0x10, 4096) >= 0)
            character_controller.section_height_offset = position.words[1] - floor.words[1];
    }
    event = character_controller.surface_interaction.selected != 0 ? (const SceneTrigger *)character_controller.surface_interaction.selected : (const SceneTrigger *)0;
    if (event->parameters.words[2] != 0)
    {
        while (PollSceneResourceLoad() != 0)
        {
        }
        next_section = SwapSceneSections();
        event = character_controller.surface_interaction.selected != 0 ? (const SceneTrigger *)character_controller.surface_interaction.selected : (const SceneTrigger *)0;
        index = next_section->FindObjectId((const SceneObjectName *)&event->parameters.words[0]);
        event = character_controller.surface_interaction.selected != 0 ? (const SceneTrigger *)character_controller.surface_interaction.selected : (const SceneTrigger *)0;
        current_section = scene_stream.active_section;
        value = event->parameters.words[1];
        SetSceneTransition(next_section, current_section, index, value);
    }
    else
    {
        parent_record = character_controller.model;
        flags = this->flags;
        AnimationHeader *selected_animation = parent_record->animations[2];
        this->frame = -1;
        this->flags = flags | 0x10000;
        this->animation = selected_animation;
        game_flow.transition_flags = game_flow.transition_flags | 0x1000;
    }
finish:
    character_controller.ClearTransientFlags();
    character_controller.flags = character_controller.flags & 0xFFFFFFDD;
    return 1;
}

sint32 AnimatedObject::ClassifyConstrainedContact(const AnimatedObject *other) const
{
    FUNCTION_MARKER(0x8005B604, "SLUS_005.78");

    union
    {
        VECTOR vector;
        uint32 words[4];
    } positions[2], input;

    MATRIX inverse;
    uint32 side, index;
    sint32 result = 0;
    for (side = 0; side < 2; ++side)
    {
        const AnimatedObject *object = side == 0 ? this : other;
        uint32 *destination = positions[side].words;
        const PathDefinition *definition = object->path_definition;
        const PathTransform *path_transform;
        if (definition != 0 && (definition->transform) != 0)
        {
            for (index = 0; index < 3; ++index)
            {
                path_transform = (const PathTransform *)((((const PathDefinition *)((const PathDefinition *)((object->path_definition))))->transform) != 0 ? (((const PathDefinition *)((const PathDefinition *)((object->path_definition))))->transform) : (PathTransform *)0);
                input.words[index] = object->position[index] - (uint32)path_transform->matrix.t[index];
            }
            for (index = 0; index < 9; ++index)
            {
                path_transform = (const PathTransform *)((((const PathDefinition *)((const PathDefinition *)((object->path_definition))))->transform) != 0 ? (((const PathDefinition *)((const PathDefinition *)((object->path_definition))))->transform) : (PathTransform *)0);
                inverse.m[index / 3][index % 3] = path_transform->matrix.m[index % 3][index / 3];
            }
            PushMatrix();
            for (index = 0; index < 4; ++index)
                xport_gte_write_control(index, xport_load_le32((const uint8 *)&inverse + index * 4));
            xport_gte_write_control(4, (uint16)inverse.m[2][2]);
            ApplyRotMatrixLV(&input.vector, &positions[side].vector);
            PopMatrix();
        }
        else
        {
            uint32 position[3];
            for (index = 0; index < 3; ++index)
                position[index] = object->position[index];
            for (index = 0; index < 3; ++index)
                destination[index] = position[index];
        }
    }
    if ((sint32)this->velocity[1] < 0 && (sint32)positions[1].words[1] < (sint32)positions[0].words[1] && positions[0].words[0] - positions[1].words[0] + 6144u < 0x3001 && positions[0].words[2] - positions[1].words[2] + 6144u < 0x3001)
        result = -1;
    return result;
}
