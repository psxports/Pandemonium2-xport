#include "game.h"
#include "original_defaults.h"

union CollisionVector
{
    VECTOR vector;
    uint32 words[4];
};

uint32 AnimatedObject::ComputeCollisionResponse(const SceneNode *other_node, uint32 mode, uint32 value)
{
    FUNCTION_MARKER(0x80040A1C, "SLUS_005.78");
    const SceneNode *other = (const SceneNode *)(other_node);
    CollisionVector delta, normal, other_velocity;
    MATRIX rotation = {0};
    CollisionTransform descriptor = {0};
    const MeshFace *polygon;
    uint32 *response_velocity;
    AnimatedPathState *path_state;
    uint32 index, x, y, z, first, second, sum, magnitude, length, factor, result;
    sint32 quotient;
    xport_store_le16((uint8 *)&rotation + 18, 0);
    if (value == 0xFFFFFFFF)
    {
        x = ((const AnimatedObject *)other)->velocity[0];
        y = ((const AnimatedObject *)other)->velocity[1];
        z = ((const AnimatedObject *)other)->velocity[2];
        other_velocity.words[0] = x;
        other_velocity.words[1] = y;
        other_velocity.words[2] = z;
        for (index = 0; index < 3; ++index)
        {
            first = (uint32)(((sint64)(sint32)this->velocity[index] * (sint32)mode) >> 12);
            delta.words[index] = first;
        }
        for (index = 0; index < 3; ++index)
            delta.words[index] = delta.words[index] + this->spheres[0].position[index];
        for (index = 0; index < 3; ++index)
        {
            first = (uint32)(((sint64)(sint32)other_velocity.words[index] * (sint32)mode) >> 12);
            normal.words[index] = first;
        }
        for (index = 0; index < 3; ++index)
            normal.words[index] = normal.words[index] + ((const AnimatedObject *)other)->spheres[0].position[index];
        x = delta.words[0] - normal.words[0];
        y = delta.words[1] - normal.words[1];
        z = delta.words[2] - normal.words[2];
        normal.words[0] = x;
        normal.words[1] = y;
        normal.words[2] = z;
        while (x + 26754 > 0xD103 || y + 26754 > 0xD103 || z + 26754 > 0xD103)
        {
            x = (uint32)((sint32)x >> 1);
            y = (uint32)((sint32)y >> 1);
            z = (uint32)((sint32)z >> 1);
            normal.words[0] = x;
            normal.words[1] = y;
            normal.words[2] = z;
        }
        VectorNormal(&normal.vector, &normal.vector);
    }
    else
    {
        polygon = SelectCollisionFace(other_node, value & 0xFFFF8FFF, &descriptor);
        x = descriptor.fields.motion[0];
        y = descriptor.fields.motion[1];
        z = descriptor.fields.motion[2];
        other_velocity.words[0] = x;
        other_velocity.words[1] = y;
        other_velocity.words[2] = z;
        for (index = 0; index < 9; ++index)
            rotation.m[index / 3][index % 3] = descriptor.fields.rotation[(index % 3) * 3 + index / 3];
        PushMatrix();
        for (index = 0; index < 5; ++index)
            xport_gte_write_control(index, xport_load_le32((const uint8 *)&rotation + index * 4));
        xport_gte_write_data(0, xport_load_le32(&polygon->normal.vector));
        xport_gte_write_data(1, xport_load_le32((const uint8 *)&polygon->normal.vector + 4));
        xport_gte_mvmva(0x486012);
        normal.words[0] = xport_gte_read_data(25);
        normal.words[1] = xport_gte_read_data(26);
        normal.words[2] = xport_gte_read_data(27);
        PopMatrix();
    }
    for (index = 0; index < 3; ++index)
        delta.words[index] = this->velocity[index] - other_velocity.words[index];
    if (value == 0xFFFFFFFF)
    {
        sum = 0;
        for (index = 0; index < 3; ++index)
            sum += (uint32)(((sint64)(sint32)delta.words[index] * (sint32)normal.words[index]) >> 12);
        magnitude = 0 - sum;
        if (magnitude != 0)
        {
            x = (uint32)((sint32)delta.words[0] >> 6);
            y = (uint32)((sint32)delta.words[1] >> 6);
            z = (uint32)((sint32)delta.words[2] >> 6);
            sum = x * x + y * y + z * z;
            if ((sint32)sum < 0)
                length = SquareRoot12((sint32)(sum >> 2)) << 1;
            else
                length = SquareRoot12((sint32)sum);
            if (length != 0)
            {
                quotient = 0x40000000 / (sint32)length;
                factor = (uint32)(((sint64)(sint32)magnitude * quotient) >> 18);
                fixed_math_result = factor;
                factor = 4096 - factor;
                for (index = 0; index < 3; ++index)
                {
                    first = (uint32)(((sint64)(sint32)delta.words[index] * (sint32)factor) >> 12);
                    delta.words[index] = first;
                }
            }
        }
    }
    sum = 0;
    for (index = 0; index < 3; ++index)
        sum += (uint32)(((sint64)(sint32)delta.words[index] * (sint32)normal.words[index]) >> 12);
    path_state = (AnimatedPathState *)(((const ActorPlacement *)((const PathDefinition *)((this->path_definition))))->state != 0 ? ((const ActorPlacement *)((const PathDefinition *)((this->path_definition))))->state : 0);
    response_velocity = path_state->direction;
    factor = 0 - (sum << 1);
    for (index = 0; index < 3; ++index)
    {
        first = (uint32)(((sint64)(sint32)normal.words[index] * (sint32)factor) >> 12);
        response_velocity[index] = first;
    }
    for (index = 0; index < 3; ++index)
    {
        first = response_velocity[index];
        second = delta.words[index];
        response_velocity[index] = first + second;
    }
    for (index = 0; index < 3; ++index)
    {
        first = response_velocity[index];
        second = other_velocity.words[index];
        result = first + second;
        response_velocity[index] = result;
    }
    return result;
}

uint32 AnimatedObject::HandleModelCollision(SceneNode *other_node, CollisionContact *contact_data)
{
    FUNCTION_MARKER(0x80041060, "SLUS_005.78");
    SceneNode *other = (SceneNode *)(other_node);
    const AnimatedModelDefinition *rule;
    uint32 *response_velocity;
    AnimatedModelDefinition *const *cursor = animated_models.definitions;
    uint32 count = animated_models.count;
    const AnimatedModelDefinition *entry = 0;
    uint32 kind_id = this->state, flags, mask, mode, value;
    AnimatedPathState *path_state;
    uint32 x, y, z, distance, speed, magnitude, scale, index;
    sint32 kind, quotient;
    while (count != 0)
    {
        entry = *cursor;
        ++cursor;
        rule = entry;
        if ((uint32)(sint32)rule->kind_id == kind_id)
            break;
        --count;
    }
    if (count == 0)
        entry = 0;
    rule = entry;
    if (contact_data == 0)
    {
        if (other_node == 0)
            return (rule->flags & 0x180) != 0;
        if (other->kind != 0)
            return 0;
        kind = (sint8)((const AnimatedObject *)other)->metadata126[0];
        switch (kind)
        {
            case 1:
                mask = 0x18;
                break;
            case 2:
                mask = 0x60;
                break;
            case 3:
                mask = 0x600;
                break;
            case 4:
                mask = 0x1800;
                break;
            default:
                return 0;
        }
        return (rule->flags & mask) != 0;
    }
    flags = this->flags & 0xFF3FFFFF;
    this->flags = flags;
    value = contact_data->flags;
    mode = contact_data->fraction;
    if (value == 0x7FFFF000)
    {
        mask = (rule->flags & 0x80) != 0 ? 0x400000 : 0x800000;
        this->flags = flags | mask;
        value = contact_data->selection;
    }
    else
    {
        kind = (sint8)((const AnimatedObject *)other)->metadata126[0];
        switch (kind)
        {
            case 1:
                mask = 8;
                break;
            case 2:
                mask = 0x20;
                break;
            case 3:
                mask = 0x200;
                break;
            case 4:
                mask = 0x800;
                break;
            default:
                return 0;
        }
        flags |= (rule->flags & mask) != 0 ? 0x400000 : 0x800000;
        this->flags = flags;
        value = 0xFFFFFFFF;
    }
    flags = this->flags;
    if ((flags & 0x400000) == 0)
        return 1;
    if (mode != 0 || value != 0xFFFFFFFF)
    {
        this->ComputeCollisionResponse(other_node, mode, value);
        if (mode != 0 || (this->flags & 0x1000000) == 0)
            return 1;
        return 0;
    }
    if ((flags & 0x1000000) == 0)
        return 0;
    path_state = (AnimatedPathState *)(((const ActorPlacement *)((const PathDefinition *)((this->path_definition))))->state != 0 ? ((const ActorPlacement *)((const PathDefinition *)((this->path_definition))))->state : 0);
    response_velocity = path_state->direction;
    x = this->spheres[0].position[0] - ((const AnimatedObject *)other)->spheres[0].position[0];
    response_velocity[0] = x;
    y = this->spheres[0].position[1] - ((const AnimatedObject *)other)->spheres[0].position[1];
    response_velocity[1] = y;
    z = this->spheres[0].position[2] - ((const AnimatedObject *)other)->spheres[0].position[2];
    response_velocity[2] = z;
    x = (uint32)((sint32)response_velocity[0] >> 6);
    y = (uint32)((sint32)response_velocity[1] >> 6);
    distance = x * x + y * y;
    x = (uint32)((sint32)((const AnimatedObject *)other)->velocity[0] >> 6);
    y = (uint32)((sint32)((const AnimatedObject *)other)->velocity[1] >> 6);
    speed = x * x + y * y;
    x = (uint32)((sint32)((const AnimatedObject *)other)->velocity[2] >> 6);
    z = (uint32)((sint32)z >> 6);
    speed += x * x;
    distance += z * z;
    if (distance == 0)
        xport_mips_break(7);
    quotient = 0x40000000 / (sint32)distance;
    magnitude = (uint32)(((sint64)(sint32)speed * quotient) >> 18);
    fixed_math_result = magnitude;
    if ((sint32)magnitude < 0)
        scale = SquareRoot12((sint32)(magnitude >> 2)) << 1;
    else
        scale = SquareRoot12((sint32)magnitude);
    if ((sint32)scale < 64)
        scale = 64;
    for (index = 0; index < 3; ++index)
    {
        value = (uint32)(((sint64)(sint32)response_velocity[index] * (sint32)scale) >> 12);
        response_velocity[index] = value;
    }
    return 0;
}

uint32 AnimatedObject::CollectActorContact(SceneNode *other_node, CollisionContact *contact_data)
{
    FUNCTION_MARKER(0x80032DDC, "SLUS_005.78");
    SceneNode *other = (SceneNode *)(other_node);
    uint32 value, flags, contact_distance, category, resource, movement, angle, x, z, timeout;
    ActorBinding *binding;
    sint32 kind, variant, result, auxiliary;
    if (!contact_data)
    {
        category = other->kind;
        if (category)
        {
            if (category != 2)
                return 0;
            kind = (sint8)(other->state_flags >> 8);
            if (kind == 13)
            {
                value = frame_clock.render_tick;
                timeout = ((const SceneTrigger *)other)->parameters.words[0];
                if (value >= timeout)
                    return 1;
            }
            if (kind == 14 || (uint8)(kind - 22) < 2)
                return 1;
            if ((uint8)(kind - 40) >= 16 || (other->flags & 0x80))
                return 0;
            if ((((const SceneTrigger *)other)->parameters.words[0] >> 24) & 0xFE)
                return 0;
            return ((((const SceneTrigger *)other)->parameters.words[1] >> 12) & 31) == 2;
        }
        kind = (sint8)((const AnimatedObject *)other)->metadata126[0];
        if (kind == 3)
        {
            variant = (sint8)((const AnimatedObject *)other)->animation_binding.word;
            if (variant == -2)
                return 0;
            binding = (ActorBinding *)(((const ActorPlacement *)((const PathDefinition *)((this->path_definition))))->state != 0 ? ((const ActorPlacement *)((const PathDefinition *)((this->path_definition))))->state : 0);
            if ((sint8)binding->model_kind < 47 || variant != -1)
                return 1;
        }
        if (kind == 1)
        {
            if (this->behavior != 24)
                return 1;
            if (((const PathDefinition *)((this->path_definition)))->flags & 0x40)
                return 1;
        }
        if (kind != 2 || (sint8)((const AnimatedObject *)other)->behavior != 19)
            return 0;
        if (this->behavior == 19)
            return 0;
        return (((const AnimatedObject *)other)->flags & 0x40) != 0;
    }
    binding = (ActorBinding *)(((const ActorPlacement *)((const PathDefinition *)((this->path_definition))))->state != 0 ? ((const ActorPlacement *)((const PathDefinition *)((this->path_definition))))->state : 0);
    category = other->kind;
    contact_distance = contact_data->fraction;
    if (category)
    {
        kind = (sint8)(other->state_flags >> 8);
        if (kind == 13)
        {
            flags = binding->reaction_flags | 8;
            binding->reaction_flags = flags;
            value = ((const SceneTrigger *)other)->parameters.words[2];
            binding->reaction_flags = (flags & 0xFFFF01FF) | ((value & 0x7F) << 9);
            timeout = (((const SceneTrigger *)other)->parameters.words[2] >> 16);
            value = frame_clock.render_tick;
            ((SceneTrigger *)other)->parameters.words[0] = value + timeout;
        }
        else if (kind == 14)
        {
            flags = binding->reaction_flags | 0x10;
            binding->reaction_flags = flags;
            return (flags & 0x80) == 0;
        }
        else if (kind == 22)
        {
            if (contact_distance)
                return 0;
            this->flags = this->flags | 0x40000;
        }
        else if (kind == 23)
        {
            if (contact_distance)
                return 0;
            this->node.state_flags = ((const SceneTrigger *)other)->parameters.words[0];
        }
        else if (!contact_distance)
            other->state_flags = (other->state_flags & 0xFF00) | (uint8)((other->state_flags & 0xFF) | 8);
        else
        {
            if ((binding->reaction_flags & 4) && (sint32)contact_distance >= (sint32)binding->respawn_ticks)
                return 0;
            flags = binding->reaction_flags;
            binding->event_node = other_node;
            binding->respawn_ticks = contact_distance;
            binding->reaction_flags = flags | 4;
        }
        return 0;
    }
    kind = (sint8)((const AnimatedObject *)other)->metadata126[0];
    if (kind == 3)
    {
        resource = ((const AnimatedObject *)other)->state;
        if (resource == 19)
        {
            if ((player_interaction.flags & 0x20) && (binding->model->contact_mode))
            {
                value = binding->flags;
                if (!(value & 2))
                {
                    binding->flags = value | 2;
                    binding->interaction_timer = 0;
                }
                player_interaction.target = binding;
                return 1;
            }
        }
        else if (resource == 11)
        {
            if (this->flags & 0x30000000)
                return 1;
            binding->reaction_flags = binding->reaction_flags | 1;
            return 1;
        }
        else if (this->behavior == 17 || (this->flags & 0x30000000))
        {
            binding->reaction_flags = binding->reaction_flags | 0x20;
            return 1;
        }
        movement = this->flags & 0x05800000;
        if (movement != 0x01800000)
        {
            if (!movement || (movement & 0x04000000))
                goto mark_push_80032ddc;
            z = ((const AnimatedObject *)other)->velocity[2];
            x = ((const AnimatedObject *)other)->velocity[0];
            angle = Atan2Angle((sint32)z, (sint32)x);
            value = (uint16)this->yaw - angle;
            if (movement & 0x00800000)
            {
                if ((uint16)(value + 1024) >= 2049)
                    goto mark_push_80032ddc;
            }
            else if ((movement & 0x01000000) && (uint16)(value + 1023) < 2047)
                goto mark_push_80032ddc;
        }
        goto finish_contact_80032ddc;
    }
    if (kind != 1)
    {
        binding->reaction_flags = binding->reaction_flags | 0x60;
        goto finish_contact_80032ddc;
    }
    if (this->behavior == 17 || (this->flags & 0x10000000))
    {
        binding->reaction_flags = binding->reaction_flags | 0x22;
        return 1;
    }
    if (!contact_distance && (((const AnimatedObject *)other)->flags & 0x02000000))
        goto mark_contact_80032ddc;
    result = ((const AnimatedObject *)other)->ClassifyCharacterContactResponse();
    value = ((const AnimatedObject *)other)->path_flags;
    auxiliary = 0;
    if (value & 1)
        auxiliary = ((const AnimatedObject *)other)->ClassifyConstrainedContact(this);
    if (result <= 0 && ((this->behavior == 20 && (this->flags & 0x40)) || (contact_data->selection < 2048 && result == 0) || (((const AnimatedObject *)other)->flags & 0x18000000) || (this->flags & 0x04400000) == 0x00400000))
    {
        if (auxiliary && this->behavior != 20)
            binding->reaction_flags = binding->reaction_flags | 8;
    }
    else
    {
        flags = binding->reaction_flags;
        binding->reaction_flags = flags | (result > 0 ? 0x20 : 8);
    }
    if ((sint8)character_controller.path_transition.contact_delay > 0)
        binding->reaction_flags = binding->reaction_flags & 0xFFFFFFD7;
mark_contact_80032ddc:
    flags = character_controller.secondary_flags;
    if (flags & 0x40)
        binding->reaction_flags = binding->reaction_flags | 0x20000;
    else if (flags & 0x200)
        binding->reaction_flags = (binding->reaction_flags | 1) & 0xFFFFFFDF;
    binding->reaction_flags = binding->reaction_flags | 2;
    goto finish_contact_80032ddc;
mark_push_80032ddc:
    binding->reaction_flags = binding->reaction_flags | 8;
finish_contact_80032ddc:
    flags = binding->reaction_flags;
    if (flags & 8)
    {
        if ((contact_data->flags & 1) || (this->behavior == 20 && !(this->flags & 0x40)))
        {
            value = binding->flags;
            flags = binding->reaction_flags;
            flags = (value & 2) ? flags & 0xFFFF01F7 : (flags & 0xFFFF01FF) | 0x200;
        }
        else
            flags &= 0xFFFFFFF7;
        binding->reaction_flags = flags;
        if ((binding->reaction_flags & 8) && !(character_controller.secondary_flags & 0x40))
            character_controller.path_transition.contact_delay = 5;
    }
    return kind != 1;
}

uint32 AnimatedObject::CollectModelGroupContact(SceneNode *other_node, CollisionContact *contact_data)
{
    FUNCTION_MARKER(0x8002F944, "SLUS_005.78");
    SceneNode *other = (SceneNode *)(other_node);
    SceneTrigger *previous;
    uint32 kind, contact, value, variant;
    sint32 subtype;
    if (contact_data == 0)
        return (uint32)(sint32)(sint8)this->CanModelGroupContact(other_node);
    kind = other->kind;
    contact = contact_data->fraction;
    if (kind == 2)
    {
        subtype = (sint8)(other->state_flags >> 8);
        if (subtype == 9 || subtype == 14 || subtype == 29)
        {
            this->model_contacts.blocker = other_node;
            return 1;
        }
        if (subtype == 1)
        {
            previous = this->model_contacts.surface0;
            if (previous != 0 && (sint8)(previous->node.state_flags >> 8) != 1)
            {
                if (this->model_contacts.surface1 == 0)
                {
                    value = this->model_contacts.fraction0;
                    this->model_contacts.surface1 = previous;
                    this->model_contacts.fraction1 = value;
                }
                else if (this->model_contacts.surface2 == 0)
                {
                    value = this->model_contacts.fraction0;
                    this->model_contacts.surface2 = previous;
                    this->model_contacts.fraction2 = value;
                }
                this->model_contacts.surface0 = 0;
            }
            if ((this->TestCharacterSurfaceContact((const SceneTrigger *)other_node, contact) & 0xFF) == 0)
                return 0;
            previous = this->model_contacts.surface0;
            if (previous != 0 && (sint8)(previous->node.state_flags >> 8) == 1 && (sint32)contact < (sint32)this->model_contacts.fraction0)
                return 0;
            this->model_contacts.surface0 = (SceneTrigger *)other_node;
            this->model_contacts.fraction0 = contact;
        }
        else
        {
            if ((uint8)((uint32)subtype - 40) >= 16)
                return 0;
            if (this->model_contacts.surface0 == 0)
            {
                this->model_contacts.surface0 = (SceneTrigger *)other_node;
                this->model_contacts.fraction0 = contact;
            }
            else if (this->model_contacts.surface1 == 0)
            {
                this->model_contacts.surface1 = (SceneTrigger *)other_node;
                this->model_contacts.fraction1 = contact;
            }
            else
            {
                if (this->model_contacts.surface2 != 0)
                    return 0;
                this->model_contacts.surface2 = (SceneTrigger *)other_node;
                this->model_contacts.fraction2 = contact;
            }
        }
        return 0;
    }
    if (kind < 3)
    {
        this->model_contacts.blocker = other_node;
        return 1;
    }
    if (kind != 8)
        return 0;
    QueueCharacterPickup(other_node, 0);
    variant = ((const BillboardNode *)other)->playback_flags & 0x7F;
    if (variant - 3 < 3)
        this->ticks = (uint16)this->ticks + 30;
    if (variant - 6 < 2)
        return 1;
    return variant == 10;
}

uint32 AnimatedObject::CanModelGroupContact(const SceneNode *other_node) const
{
    FUNCTION_MARKER(0x8002F75C, "SLUS_005.78");
    const SceneNode *other = (const SceneNode *)(other_node);
    uint32 kind, variant, scene;
    const ScenePath *context;
    sint32 subtype, state, facing, other_facing;
    kind = other->kind;
    if (kind == 2)
    {
        subtype = (sint8)(other->state_flags >> 8);
        if (subtype == 9 || subtype == 14 || subtype == 29)
            return 1;
        if ((uint8)((uint32)subtype - 40) < 16 && (other->flags & 0x80) == 0 && ((((const SceneTrigger *)other)->parameters.words[0] >> 24) & 0xFE) == 0)
        {
            if (((((const SceneTrigger *)other)->parameters.words[1] >> 12) & 31) == 4)
                return 1;
            subtype = (sint8)(other->state_flags >> 8);
        }
        if (subtype != 1)
            return 0;
        state = (sint8)this->metadata126[0];
        if (state == 1)
        {
            if (this->behavior == 33)
                context = (this->scene_path);
            else
            {
                scene = OriginalDefaults::SceneContextIdentity;
                context = camera_motion.path;
            }
        }
        else
        {
            context = 0;
            if (state == 3 && this->animation_binding.tracks[0] == -1)
                context = (this->scene_path);
        }
        return (this->CanObjectChangePath((const SceneTrigger *)other_node, context) & 0xFF) != 0;
    }
    if (kind >= 3)
        return kind == 8;
    if (kind != 0)
        return 0;
    state = this->animation_binding.tracks[1];
    variant = ((const AnimatedObject *)other)->metadata126[0];
    if (state != -3)
        return 0;
    if (variant - 1 >= 2 || this->animation_binding.tracks[0] == -(sint32)(sint8)variant)
    {
        if ((sint8)variant != 3 || (sint8)(((const AnimatedObject *)other)->animation_binding.word >> 8) != state)
            return 0;
        facing = this->animation_binding.tracks[0];
        other_facing = (sint8)((const AnimatedObject *)other)->animation_binding.word;
        if (facing == other_facing)
            return 0;
    }
    if ((sint8)variant == 2 && this->state == 11)
        return (sint8)((const AnimatedObject *)other)->behavior == 17;
    return 1;
}
