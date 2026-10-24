#include "hd_internal.h"

static float hd_animation_key_time(float frame, float end, uint32 first, uint32 last, const uint8 *data)
{
    float duration = end - (first >> 20);
    if (duration <= 0.0f)
        return 0.0f;
    float u = (frame - (first >> 20)) / duration;
    uint32 left = first & 255, right = last & 255;
    if ((left | right) == 0)
        return u;
    float amplitude = (((first >> 8) & 4095) + 2048) / 4096.0f;
    const AnimationEaseCurve *incoming = left != 0 ? (const AnimationEaseCurve *)(data - left * sizeof(AnimationEaseCurve)) : NULL;
    const AnimationEaseCurve *outgoing = right != 0 ? (const AnimationEaseCurve *)(data - right * sizeof(AnimationEaseCurve)) : NULL;
    if (incoming != NULL && u < incoming->incoming_end / 4096.0f)
        return amplitude * (incoming->incoming_coefficient / 4096.0f) * u * u;
    if (outgoing != NULL && u >= outgoing->outgoing_start / 4096.0f)
        return 1.0f - amplitude * (outgoing->outgoing_coefficient / 4096.0f) * (1.0f - u) * (1.0f - u);
    if (incoming != NULL && outgoing == NULL && u >= 1.0f)
        return 1.0f;
    return amplitude * (2.0f * u - (incoming != NULL ? incoming->incoming_end / 4096.0f : 0.0f));
}

VECTORF hd_animation_vector(const AnimationTrack *track, float frame)
{
    const VectorAnimationKey *keys = (const VectorAnimationKey *)track->keys;
    if (keys == NULL || track->keys == (const uint8 *)-1 || track->key_count == 0)
    {
        VECTORF value = {track->constant.x / 32.0f, track->constant.y / 32.0f, track->constant.z / 32.0f};
        return value;
    }
    const VectorAnimationKey *prev = keys + track->key_count - 1;
    const VectorAnimationKey *next = keys;
    float end = (float)(prev->time >> 20);
    if (frame < end && track->key_count > 1)
    {
        next = keys + 1;
        while (next < keys + track->key_count - 1 && frame >= (next->time >> 20))
            ++next;
        prev = next - 1;
        end = (float)(next->time >> 20);
    }
    else if (frame == end || frame >= track->frame_limit || track->key_count == 1)
    {
        if (frame > end && frame >= track->frame_limit && (track->flags & 3) == 3)
            prev = keys;
        VECTORF value = {prev->pos.x / 32.0f, prev->pos.y / 32.0f, prev->pos.z / 32.0f};
        return value;
    }
    else
        end = (float)track->frame_limit;
    float u = hd_animation_key_time(frame, end, prev->time, next->time, track->keys);
    float sq = u * u, cube = sq * u;
    float next_weight = 3.0f * sq - 2.0f * cube;
    float prev_weight = 1.0f - next_weight;
    float prev_tangent = cube - 2.0f * sq + u;
    float next_tangent = cube - sq;
    VECTORF value = {
        (prev->pos.x / 32.0f) * prev_weight + (next->pos.x / 32.0f) * next_weight
            + (prev->outgoing_tangent.x / 32.0f) * prev_tangent + (next->incoming_tangent.x / 32.0f) * next_tangent,
        (prev->pos.y / 32.0f) * prev_weight + (next->pos.y / 32.0f) * next_weight
            + (prev->outgoing_tangent.y / 32.0f) * prev_tangent + (next->incoming_tangent.y / 32.0f) * next_tangent,
        (prev->pos.z / 32.0f) * prev_weight + (next->pos.z / 32.0f) * next_weight
            + (prev->outgoing_tangent.z / 32.0f) * prev_tangent + (next->incoming_tangent.z / 32.0f) * next_tangent};
    return value;
}

static QUATF hd_animation_source_quat(const sint16 *source)
{
    QUATF value = {-source[1] / 16384.0f, -source[2] / 16384.0f, -source[3] / 16384.0f, source[0] / 16384.0f};
    return value;
}

static QUATF hd_animation_mix_quat(const QUATF *first, const QUATF *last, float left, float right)
{
    QUATF value = {first->x * left + last->x * right, first->y * left + last->y * right,
        first->z * left + last->z * right, first->w * left + last->w * right};
    return value;
}

static QUATF hd_animation_quat(const AnimationTrack *track, float frame)
{
    const QuaternionAnimationKey *keys = (const QuaternionAnimationKey *)track->keys;
    if (keys == NULL || track->keys == (const uint8 *)-1 || track->key_count == 0)
        return hd_animation_source_quat(track->constant_quat.comp);
    const QuaternionAnimationKey *prev = keys + track->key_count - 1;
    const QuaternionAnimationKey *next = keys;
    float end = (float)(prev->time >> 20);
    if (frame < end && track->key_count > 1)
    {
        next = keys + 1;
        while (next < keys + track->key_count - 1 && frame >= (next->time >> 20))
            ++next;
        prev = next - 1;
        end = (float)(next->time >> 20);
    }
    else if (frame == end || frame >= track->frame_limit || track->key_count == 1)
    {
        if (frame > end && frame >= track->frame_limit && (track->flags & 3) == 3)
            prev = keys;
        return hd_animation_source_quat(prev->rotation.comp);
    }
    else
        end = (float)track->frame_limit;
    float u = hd_animation_key_time(frame, end, prev->time, next->time, track->keys);
    QUATF first = hd_animation_source_quat(prev->rotation.comp);
    QUATF last = hd_animation_source_quat(next->rotation.comp);
    if (next->turns == 0)
        return hd_animation_mix_quat(&first, &last, 1.0f - u, u);
    float angle = next->angle * RAD12;
    sint32 extra = (sint32)((uint32)(next->turns - 1) << 27) >> 16;
    float part = (next->angle + extra) * RAD12 * u;
    float divisor = sinf(angle);
    if (fabsf(divisor) < 0.000001f)
        return hd_animation_mix_quat(&first, &last, 1.0f - u, u);
    QUATF value = hd_animation_mix_quat(&first, &last, sinf(angle - part) / divisor, sinf(part) / divisor);
    float norm = value.x * value.x + value.y * value.y + value.z * value.z + value.w * value.w;
    if (1.0f / divisor > 20.0f && norm < 16368.0f / 16384.0f)
        NormalizeQuatf(&value, &value);
    return value;
}

static QUATF hd_animation_transition(const QUATF *current, const QUATF *target, float frame)
{
    // Preserve the source transition direction and its three-frame schedule
    float weight = 1.0f - (frame + 1.0f) / 3.0f;
    float dot = current->x * target->x + current->y * target->y + current->z * target->z + current->w * target->w;
    float left = 1.0f - weight, right = weight;
    if (fabsf(dot) < 16368.0f / 16384.0f)
    {
        float angle = acosf(fabsf(dot));
        left = sinf((1.0f - weight) * angle) / sinf(angle);
        right = sinf(weight * angle) / sinf(angle);
    }
    if (dot < 0.0f)
        right = -right;
    return hd_animation_mix_quat(current, target, left, right);
}

static void hd_animation_quat_matrix(const QUATF *quat, MATRIXF *model)
{
    float norm = quat->w * quat->w + quat->x * quat->x + quat->y * quat->y + quat->z * quat->z;
    QuatMatrixf(quat, model);
    // Preserve the original polynomial for nonunit quaternion samples
    for (uint32 row = 0; row < 3; ++row)
        for (uint32 column = 0; column < 3; ++column)
            model->m[row][column] = model->m[row][column] * norm + (row == column ? 1.0f - norm : 0.0f);
}

void hd_animation_release(HdAnimationChannel *channel)
{
    if (channel->curve != NULL && --channel->curve->refs == 0)
        free(channel->curve);
    memset(channel, 0, sizeof(*channel));
}

void hd_animation_retain(HdAnimationChannel *channel)
{
    if (channel->curve != NULL)
        ++channel->curve->refs;
}

void hd_animation_nodes_release(HdPoseSample *nodes, uint32 count)
{
    for (uint32 idx = 0; idx < count; ++idx)
    {
        hd_animation_release(&nodes[idx].position_channel);
        hd_animation_release(&nodes[idx].rotation_channel);
    }
}

static bool hd_animation_curve_matches(const HdAnimationCurve *curve, const AnimationTrack *track, bool quaternion)
{
    if (curve == NULL || curve->source != track->keys || curve->is_rotation != quaternion)
        return false;
    if (track->keys == (const uint8 *)-1)
        return quaternion ? memcmp(&curve->track.constant_quat, &track->constant_quat, sizeof(Quat)) == 0
            : memcmp(&curve->track.constant, &track->constant, sizeof(Vec3)) == 0;
    return curve->track.key_count == track->key_count && curve->track.frame_limit == track->frame_limit
        && curve->track.frame_count == track->frame_count && curve->track.flags == (track->flags & ~8);
}

static void hd_animation_channel(HdAnimationChannel *channel, const AnimationTrack *track, const AnimationHeader *clip,
    sint32 frame, bool per_track, bool quaternion)
{
    if (channel->clip != clip || !hd_animation_curve_matches(channel->curve, track, quaternion))
    {
        hd_animation_release(channel);
        size_t prefix = 0, bytes = 0;
        bool keyed = track->keys != NULL && track->keys != (const uint8 *)-1;
        if (keyed)
        {
            size_t stride = quaternion ? sizeof(QuaternionAnimationKey) : sizeof(VectorAnimationKey);
            bytes = track->key_count * stride;
            for (uint32 idx = 0; idx < track->key_count; ++idx)
            {
                uint32 time;
                memcpy(&time, track->keys + idx * stride, sizeof(time));
                size_t ease_bytes = (time & 255) * sizeof(AnimationEaseCurve);
                if (ease_bytes > prefix)
                    prefix = ease_bytes;
            }
        }
        HdAnimationCurve *curve = (HdAnimationCurve *)malloc(sizeof(HdAnimationCurve) + prefix + bytes);
        if (curve == NULL)
            return;
        curve->track = *track;
        curve->source = track->keys;
        curve->is_rotation = quaternion;
        curve->refs = 1;
        if (keyed)
        {
            uint8 *data = (uint8 *)(curve + 1);
            memcpy(data, track->keys - prefix, prefix + bytes);
            curve->track.keys = data + prefix;
            curve->track.flags &= ~8;
        }
        channel->curve = curve;
    }
    channel->clip = clip;
    channel->frame = frame;
    channel->per_track = per_track;
    channel->period = per_track ? ((track->flags & 2) != 0 ? track->frame_limit : 0) : clip->frame_count;
}

static uint32 hd_identity_slot(const SceneNode *node, HdIdentity *entries, uint32 capacity)
{
    size_t hash = (size_t)node >> 4;
    hash ^= hash >> 11;
    uint32 slot = hash & (capacity - 1);
    while (entries[slot].node != NULL && entries[slot].node != node)
        slot = (slot + 1) & (capacity - 1);
    return slot;
}

static uint32 hd_pool_slot(const SceneNode *node)
{
    if (hd_state.sprite_pool == NULL || (size_t)node < (size_t)hd_state.sprite_pool)
        return HD_NO_POSE;
    size_t offset = (size_t)node - (size_t)hd_state.sprite_pool;
    if (offset >= HD_POOL_SLOTS * sizeof(SpriteRecord) || offset % sizeof(SpriteRecord) != 0)
        return HD_NO_POSE;
    return (uint32)(offset / sizeof(SpriteRecord));
}

static uint32 hd_pool_key(uint64 key)
{
    uint64 slot = key & ~((uint64)1 << 63);
    return (key >> 63) != 0 && slot != 0 && slot <= HD_POOL_SLOTS ? (uint32)slot - 1 : HD_NO_POSE;
}

static uint64 hd_identity(const SceneNode *node)
{
    if (node == NULL)
        return 0;
    uint32 pool_slot = hd_pool_slot(node);
    if (pool_slot != HD_NO_POSE)
        return ((uint64)1 << 63) | (pool_slot + 1);
    if (hd_state.identity_count * 2 >= hd_state.identity_capacity)
    {
        uint32 capacity = hd_state.identity_capacity != 0 ? hd_state.identity_capacity * 2 : 128;
        if (capacity <= hd_state.identity_capacity || capacity > (size_t)-1 / sizeof(HdIdentity))
            return 0;
        HdIdentity *entries = (HdIdentity *)calloc(capacity, sizeof(HdIdentity));
        if (entries == NULL)
            return 0;
        for (uint32 idx = 0; idx < hd_state.identity_capacity; ++idx)
            if (hd_state.identities[idx].node != NULL)
                entries[hd_identity_slot(hd_state.identities[idx].node, entries, capacity)] = hd_state.identities[idx];
        free(hd_state.identities);
        hd_state.identities = entries;
        hd_state.identity_capacity = capacity;
    }
    HdIdentity *entry = hd_state.identities + hd_identity_slot(node, hd_state.identities, hd_state.identity_capacity);
    if (entry->node == NULL)
    {
        entry->node = node;
        entry->id = ++hd_state.identity_serial;
        ++hd_state.identity_count;
    }
    return entry->id;
}

static uint32 hd_animation_slot(const SceneNode *node, HdAnimationEntry *entries, uint32 capacity)
{
    size_t hash = (size_t)node >> 4;
    hash ^= hash >> 11;
    uint32 slot = hash & (capacity - 1);
    while (entries[slot].node != NULL && entries[slot].node != node)
        slot = (slot + 1) & (capacity - 1);
    return slot;
}

static HdAnimationPose *hd_animation_entry(const SceneNode *node, bool create)
{
    if (node == NULL)
        return NULL;
    if (create && hd_state.animation_count * 2 >= hd_state.animation_capacity)
    {
        uint32 capacity = hd_state.animation_capacity != 0 ? hd_state.animation_capacity * 2 : 128;
        if (capacity <= hd_state.animation_capacity || capacity > (size_t)-1 / sizeof(HdAnimationEntry))
            return NULL;
        HdAnimationEntry *entries = (HdAnimationEntry *)calloc(capacity, sizeof(HdAnimationEntry));
        if (entries == NULL)
            return NULL;
        for (uint32 idx = 0; idx < hd_state.animation_capacity; ++idx)
            if (hd_state.animations[idx].node != NULL)
                entries[hd_animation_slot(hd_state.animations[idx].node, entries, capacity)] = hd_state.animations[idx];
        free(hd_state.animations);
        hd_state.animations = entries;
        hd_state.animation_capacity = capacity;
    }
    if (hd_state.animation_capacity == 0)
        return NULL;
    HdAnimationEntry *entry = hd_state.animations + hd_animation_slot(node, hd_state.animations, hd_state.animation_capacity);
    if (entry->node == NULL && create)
    {
        memset(entry, 0, sizeof(*entry));
        entry->node = node;
        ++hd_state.animation_count;
    }
    return entry->node != NULL ? &entry->pose : NULL;
}

void hd_forget_animation(const SceneNode *node)
{
    if (hd_state.animation_count == 0)
        return;
    uint32 slot = hd_animation_slot(node, hd_state.animations, hd_state.animation_capacity);
    if (hd_state.animations[slot].node == NULL)
        return;
    hd_animation_release(&hd_state.animations[slot].pose.position_channel);
    hd_animation_release(&hd_state.animations[slot].pose.rotation_channel);
    hd_state.animations[slot].node = NULL;
    --hd_state.animation_count;
    uint32 next = (slot + 1) & (hd_state.animation_capacity - 1);
    while (hd_state.animations[next].node != NULL)
    {
        HdAnimationEntry entry = hd_state.animations[next];
        hd_state.animations[next].node = NULL;
        hd_state.animations[hd_animation_slot(entry.node, hd_state.animations, hd_state.animation_capacity)] = entry;
        next = (next + 1) & (hd_state.animation_capacity - 1);
    }
}

static const Vec3 *hd_animation_fields(const SceneNode *node, const sint16 **rotation, const sint8 **binding)
{
    *rotation = NULL;
    *binding = NULL;
    if (node->kind == 0)
    {
        const AnimatedObject *actor = (const AnimatedObject *)node;
        *rotation = actor->orientation;
        *binding = actor->animation_binding.tracks;
        return &actor->pos;
    }
    if (node->kind == 5)
    {
        const MeshPartNode *part = (const MeshPartNode *)node;
        *rotation = part->rotation;
        *binding = part->animation_binding.tracks;
        return &part->pos;
    }
    if (node->kind == 2)
    {
        const SceneTrigger *trigger = (const SceneTrigger *)node;
        *rotation = trigger->rotation;
        *binding = trigger->animation_binding.tracks;
        return &trigger->pos;
    }
    if (node->kind == 6)
    {
        *binding = (const sint8 *)&node->state_flags;
        return &((const SphereNode *)node)->pos;
    }
    if (node->kind == 7)
    {
        const SpriteRecord *sprite = (const SpriteRecord *)node;
        *binding = sprite->animation_binding.tracks;
        return &sprite->pos;
    }
    return NULL;
}

static bool hd_animation_has_rotation(const SceneNode *node, const sint16 *source)
{
    const HdAnimationPose *pose = hd_animation_entry(node, false);
    return pose != NULL && pose->rotation_valid && !pose->pending
        && memcmp(source, &pose->source_rotation, sizeof(Quat)) == 0;
}

static QUATF hd_animation_rotation(const SceneNode *node, const sint16 *source)
{
    if (hd_animation_has_rotation(node, source))
        return hd_animation_entry(node, false)->rotation;
    return hd_animation_source_quat(source);
}

static VECTORF hd_animation_position(const SceneNode *node, const Vec3 *source)
{
    const HdAnimationPose *pose = hd_animation_entry(node, false);
    if (pose != NULL && pose->pos_valid && !pose->pending && memcmp(source, &pose->source_pos, sizeof(*source)) == 0)
        return pose->pos;
    VECTORF value = {source->x / 32.0f, source->y / 32.0f, source->z / 32.0f};
    return value;
}

void hd_animation_begin(const AnimatedObject *owner, const SceneNode *node, bool tracks)
{
    if (owner == NULL || owner->animation == NULL || (!tracks && node->kind == 0))
        return;
    const sint16 *rotation;
    const sint8 *binding;
    const Vec3 *pos = hd_animation_fields(node, &rotation, &binding);
    if (pos == NULL || (tracks && (node->kind == 6 || (node->kind == 0 && ((const AnimatedObject *)node)->animation_binding.word == 0xFFFFFFFF))))
        return;
    HdAnimationPose *pose = hd_animation_entry(node, true);
    if (pose == NULL)
        return;
    bool same_owner = pose->owner == owner;
    pose->pos_valid = same_owner && pose->pos_valid && memcmp(pos, &pose->source_pos, sizeof(*pos)) == 0;
    pose->rotation_valid = same_owner && pose->rotation_valid && rotation != NULL
        && memcmp(rotation, &pose->source_rotation, sizeof(Quat)) == 0;
    if (!pose->pos_valid)
    {
        hd_animation_release(&pose->position_channel);
        VECTORF value = {pos->x / 32.0f, pos->y / 32.0f, pos->z / 32.0f};
        pose->pos = value;
    }
    if (!pose->rotation_valid)
    {
        hd_animation_release(&pose->rotation_channel);
        if (rotation != NULL)
            pose->rotation = hd_animation_source_quat(rotation);
    }
    pose->owner = owner;
    pose->pending = true;
    const AnimationHeader *clip = owner->animation;
    sint32 idx = binding[0];
    if (idx >= 0 && idx < clip->counts[0])
    {
        const AnimationTrack *track = &clip->tracks[0].vectors[idx].track;
        if (track->keys != NULL && (!tracks || (track->flags & 8) != 0))
        {
            if (!tracks && (owner->flags & 0x10000) != 0)
            {
                hd_animation_release(&pose->position_channel);
                const Vec3 *target = track->keys == (const uint8 *)-1 ? &track->constant : &((const VectorAnimationKey *)track->keys)->pos;
                float weight = (owner->frame + 1.0f) / 3.0f;
                pose->pos.vx += (target->x / 32.0f - pose->pos.vx) * weight;
                pose->pos.vy += (target->y / 32.0f - pose->pos.vy) * weight;
                pose->pos.vz += (target->z / 32.0f - pose->pos.vz) * weight;
            }
            else
            {
                hd_animation_channel(&pose->position_channel, track, clip, tracks ? track->frame : owner->frame, tracks, false);
                pose->pos = hd_animation_vector(track, tracks ? (float)track->frame : (float)owner->frame);
            }
            pose->pos_valid = true;
        }
    }
    if (rotation == NULL)
        return;
    idx = binding[1];
    if (idx < 0 || idx >= clip->counts[1])
        return;
    const AnimationTrack *track = clip->tracks[1].recs + idx;
    if (track->keys == NULL)
        return;
    if (!tracks && gameplay_input.rotation_track_shift != 0)
    {
        idx = (idx + gameplay_input.rotation_track_shift) % clip->counts[1];
        track = clip->tracks[1].recs + idx;
    }
    if (track->keys == NULL || (tracks && (track->flags & 8) == 0))
        return;
    if (!tracks && (owner->flags & 0x10000) != 0)
    {
        hd_animation_release(&pose->rotation_channel);
        const sint16 *target = track->keys == (const uint8 *)-1 ? track->constant_quat.comp : ((const QuaternionAnimationKey *)track->keys)->rotation.comp;
        QUATF value = hd_animation_source_quat(target);
        pose->rotation = hd_animation_transition(&pose->rotation, &value, (float)owner->frame);
    }
    else
    {
        hd_animation_channel(&pose->rotation_channel, track, clip, tracks ? track->frame : owner->frame, tracks, true);
        pose->rotation = hd_animation_quat(track, tracks ? (float)track->frame : (float)owner->frame);
    }
    pose->rotation_valid = true;
}

void hd_animation_end(const AnimatedObject *owner, const SceneNode *node)
{
    HdAnimationPose *pose = hd_animation_entry(node, false);
    if (pose == NULL || pose->owner != owner || !pose->pending)
        return;
    const sint16 *rotation;
    const sint8 *binding;
    const Vec3 *pos = hd_animation_fields(node, &rotation, &binding);
    pose->source_pos = *pos;
    if (rotation != NULL)
        memcpy(&pose->source_rotation, rotation, sizeof(Quat));
    pose->pending = false;
}

static uint32 hd_graph_work_slot(const SceneNode *node, HdGraphWork *entries, uint32 capacity)
{
    size_t hash = (size_t)node >> 4;
    hash ^= hash >> 11;
    uint32 slot = hash & (capacity - 1);
    while (entries[slot].node != NULL && entries[slot].node != node)
        slot = (slot + 1) & (capacity - 1);
    return slot;
}

uint32 hd_graph_work(const SceneNode *node, uint32 work, bool advance)
{
    if (node == NULL)
        return 0;
    if (advance && !hd_frame.collecting)
        return work;
    if (advance && hd_state.graph_work_count * 2 >= hd_state.graph_work_capacity)
    {
        uint32 capacity = hd_state.graph_work_capacity != 0 ? hd_state.graph_work_capacity * 2 : 128;
        if (capacity <= hd_state.graph_work_capacity || capacity > (size_t)-1 / sizeof(HdGraphWork))
        {
            hd_frame.failed = true;
            return work;
        }
        HdGraphWork *entries = (HdGraphWork *)calloc(capacity, sizeof(HdGraphWork));
        if (entries == NULL)
        {
            hd_frame.failed = true;
            return work;
        }
        for (uint32 idx = 0; idx < hd_state.graph_work_capacity; ++idx)
            if (hd_state.graph_work[idx].node != NULL)
                entries[hd_graph_work_slot(hd_state.graph_work[idx].node, entries, capacity)] = hd_state.graph_work[idx];
        free(hd_state.graph_work);
        hd_state.graph_work = entries;
        hd_state.graph_work_capacity = capacity;
    }
    if (hd_state.graph_work_capacity == 0)
        return 0;
    HdGraphWork *entry = hd_state.graph_work + hd_graph_work_slot(node, hd_state.graph_work, hd_state.graph_work_capacity);
    if (!advance)
        return entry->node != NULL ? entry->work : 0;
    if (entry->node == NULL)
    {
        entry->node = node;
        ++hd_state.graph_work_count;
    }
    entry->work = work;
    return work;
}

static void hd_forget_graph_work(const SceneNode *node)
{
    if (hd_state.graph_work_count == 0)
        return;
    uint32 slot = hd_graph_work_slot(node, hd_state.graph_work, hd_state.graph_work_capacity);
    if (hd_state.graph_work[slot].node == NULL)
        return;
    hd_state.graph_work[slot].node = NULL;
    --hd_state.graph_work_count;
    uint32 next = (slot + 1) & (hd_state.graph_work_capacity - 1);
    while (hd_state.graph_work[next].node != NULL)
    {
        HdGraphWork entry = hd_state.graph_work[next];
        hd_state.graph_work[next].node = NULL;
        hd_state.graph_work[hd_graph_work_slot(entry.node, hd_state.graph_work, hd_state.graph_work_capacity)] = entry;
        next = (next + 1) & (hd_state.graph_work_capacity - 1);
    }
}

void hd_forget_pose(const SceneNode *node, bool children)
{
    if (node == NULL)
        return;
    hd_forget_animation(node);
    hd_forget_graph_work(node);
    uint32 pool_slot = hd_pool_slot(node);
    if (pool_slot != HD_NO_POSE)
    {
        hd_state.pool_valid[pool_slot] = false;
        hd_state.samples[0].pool_valid[pool_slot] = false;
        hd_state.samples[1].pool_valid[pool_slot] = false;
    }
    else if (hd_state.identity_count != 0)
    {
        uint32 slot = hd_identity_slot(node, hd_state.identities, hd_state.identity_capacity);
        if (hd_state.identities[slot].node != NULL)
        {
            hd_state.identities[slot].node = NULL;
            --hd_state.identity_count;
            // Close the probe chain without retaining keys for released allocations
            uint32 next = (slot + 1) & (hd_state.identity_capacity - 1);
            while (hd_state.identities[next].node != NULL)
            {
                HdIdentity entry = hd_state.identities[next];
                hd_state.identities[next].node = NULL;
                hd_state.identities[hd_identity_slot(entry.node, hd_state.identities, hd_state.identity_capacity)] = entry;
                next = (next + 1) & (hd_state.identity_capacity - 1);
            }
        }
    }
    if (children)
        for (const SceneNode *child = node->links.first_child; child != NULL; child = child->links.sibling_next)
            hd_forget_pose(child, true);
}

void hd_clear_identities(void)
{
    if (hd_state.identity_capacity != 0)
        memset(hd_state.identities, 0, hd_state.identity_capacity * sizeof(HdIdentity));
    hd_state.identity_count = 0;
    ++hd_state.pose_epoch;
}

static bool hd_animation_cycle(const HdAnimationChannel *channel)
{
    const AnimationTrack *track = &channel->curve->track;
    if (track->keys == (const uint8 *)-1)
        return true;
    if (track->frame_limit < channel->period)
        return false;
    if (!channel->curve->is_rotation)
    {
        VECTORF start = hd_animation_vector(track, 0.0f);
        VECTORF end = hd_animation_vector(track, (float)channel->period);
        return fabsf(start.vx - end.vx) < 0.00001f && fabsf(start.vy - end.vy) < 0.00001f && fabsf(start.vz - end.vz) < 0.00001f;
    }
    QUATF first = hd_animation_quat(track, 0.0f), last = hd_animation_quat(track, (float)channel->period);
    MATRIXF start, end;
    IdentityMatrixf(&start);
    IdentityMatrixf(&end);
    hd_animation_quat_matrix(&first, &start);
    hd_animation_quat_matrix(&last, &end);
    for (uint32 row = 0; row < 3; ++row)
        for (uint32 column = 0; column < 3; ++column)
            if (fabsf(start.m[row][column] - end.m[row][column]) > 0.00001f)
                return false;
    return true;
}

static bool hd_animation_phase(const HdAnimationChannel *previous, const HdAnimationChannel *current, float fract, float *frame)
{
    if (previous->curve == NULL || previous->curve != current->curve || previous->clip != current->clip
        || previous->period != current->period || previous->per_track != current->per_track)
        return false;
    if (current->curve->track.keys == (const uint8 *)-1)
    {
        *frame = 0.0f;
        return true;
    }
    sint32 delta = current->frame - previous->frame;
    bool wrapped = false;
    if (current->period > 1 && previous->frame == current->period - 1 && current->frame == 0)
    {
        delta = 1;
        wrapped = true;
    }
    else if (current->per_track && current->period > 1 && previous->frame == 0 && current->frame == current->period - 1)
    {
        delta = -1;
        wrapped = true;
    }
    if (delta < -1 || delta > 1 || (wrapped && !hd_animation_cycle(current)))
        return false;
    *frame = previous->frame + delta * fract;
    if (*frame < 0.0f && current->period > 0)
        *frame += current->period;
    return true;
}

static void hd_mix_matrix(const HdPoseSample *previous, const HdPoseSample *current, float fract, MATRIXF *mixed)
{
    if (fract <= 0.0f)
    {
        *mixed = previous->local;
        return;
    }
    if (fract >= 1.0f)
    {
        *mixed = current->local;
        return;
    }
    float frame;
    if (hd_animation_phase(&previous->rotation_channel, &current->rotation_channel, fract, &frame))
    {
        QUATF quat = hd_animation_quat(&current->rotation_channel.curve->track, frame);
        hd_animation_quat_matrix(&quat, mixed);
        VECTORF scale = {
            previous->animation_scale.vx + (current->animation_scale.vx - previous->animation_scale.vx) * fract,
            previous->animation_scale.vy + (current->animation_scale.vy - previous->animation_scale.vy) * fract,
            previous->animation_scale.vz + (current->animation_scale.vz - previous->animation_scale.vz) * fract};
        ScaleMatrixf(mixed, &scale);
    }
    else
    {
        QUATF quat;
        MATRIXF old_rotation, new_rotation, rotation, inverse, old_scale, new_scale, scale;
        QuatMatrixf(&previous->rotation, &old_rotation);
        QuatMatrixf(&current->rotation, &new_rotation);
        SlerpQuatf(&previous->rotation, &current->rotation, fract, &quat);
        QuatMatrixf(&quat, &rotation);
        TransposeMatrixf(&old_rotation, &inverse);
        MulMatrix0f(&inverse, &previous->local, &old_scale);
        TransposeMatrixf(&new_rotation, &inverse);
        MulMatrix0f(&inverse, &current->local, &new_scale);
        for (uint32 row = 0; row < 3; ++row)
            for (uint32 col = 0; col < 3; ++col)
                scale.m[row][col] = old_scale.m[row][col] + (new_scale.m[row][col] - old_scale.m[row][col]) * fract;
        MulMatrix0f(&rotation, &scale, mixed);
    }
    if (hd_animation_phase(&previous->position_channel, &current->position_channel, fract, &frame))
    {
        VECTORF pos = hd_animation_vector(&current->position_channel.curve->track, frame);
        TransMatrixf(mixed, &pos);
    }
    else
        for (uint32 idx = 0; idx < 3; ++idx)
            mixed->t[idx] = previous->local.t[idx] + (current->local.t[idx] - previous->local.t[idx]) * fract;
}

static uint32 hd_pose_index_slot(uint64 id, HdPoseIndex *entries)
{
    uint32 slot = (uint32)(id ^ (id >> 32)) & (hd_state.index_capacity - 1);
    while (entries[slot].id != 0 && entries[slot].id != id)
        slot = (slot + 1) & (hd_state.index_capacity - 1);
    return slot;
}

static void hd_index_poses(const HdSnapshot *sample, HdPoseIndex *entries)
{
    memset(entries, 0, hd_state.index_capacity * sizeof(HdPoseIndex));
    for (uint32 idx = 0; idx < sample->node_count; ++idx)
    {
        uint64 id = sample->nodes[idx].id;
        HdPoseIndex *entry = entries + hd_pose_index_slot(id, entries);
        // A repeated node needs an instance key before it can be paired safely
        entry->index = entry->id != 0 ? HD_NO_POSE : idx;
        entry->id = id;
    }
}

static void hd_compose_pose(const MATRIXF *parent, const HdPoseSample *node, MATRIXF *world)
{
    *world = node->local;
    if (parent == NULL)
        return;
    if ((node->space & HD_POSE_PARENT_ROT) != 0)
        MulMatrix0f(parent, &node->local, world);
    if ((node->space & HD_POSE_PARENT_POS) != 0)
    {
        VECTORF pos = {node->local.t[0], node->local.t[1], node->local.t[2]};
        RotTransf(parent, &pos, &pos);
        TransMatrixf(world, &pos);
    }
}

bool hd_pose_world(const HdSnapshot *sample, uint32 index, MATRIXF *world)
{
    if (index >= sample->node_count)
        return false;
    const HdPoseSample *node = sample->nodes + index;
    if (node->parent == HD_NO_POSE)
        hd_compose_pose(NULL, node, world);
    else
    {
        MATRIXF parent;
        if (node->parent >= index || !hd_pose_world(sample, node->parent, &parent))
            return false;
        hd_compose_pose(&parent, node, world);
    }
    return true;
}

bool hd_mix_poses(const HdSnapshot *previous, const HdSnapshot *current, float fract)
{
    if (hd_state.mixed_pose_capacity < current->node_count)
    {
        HdMixedPose *poses = (HdMixedPose *)hd_grow_sample(hd_state.mixed_poses, &hd_state.mixed_pose_capacity, current->node_count, sizeof(HdMixedPose));
        if (poses == NULL)
            return false;
        hd_state.mixed_poses = poses;
    }
    uint32 count = previous->node_count > current->node_count ? previous->node_count : current->node_count;
    if (count > 0x3FFFFFFFu)
        return false;
    if (hd_state.index_capacity < count * 2 + 1)
    {
        uint32 capacity = 128;
        while (capacity < count * 2 + 1)
            capacity *= 2;
        if (capacity > (size_t)-1 / sizeof(HdPoseIndex))
            return false;
        HdPoseIndex *old_index = (HdPoseIndex *)calloc(capacity, sizeof(HdPoseIndex));
        HdPoseIndex *new_index = (HdPoseIndex *)calloc(capacity, sizeof(HdPoseIndex));
        if (old_index == NULL || new_index == NULL)
        {
            free(old_index);
            free(new_index);
            return false;
        }
        free(hd_state.previous_index);
        free(hd_state.current_index);
        hd_state.previous_index = old_index;
        hd_state.current_index = new_index;
        hd_state.index_capacity = capacity;
    }
    hd_index_poses(previous, hd_state.previous_index);
    hd_index_poses(current, hd_state.current_index);
    for (uint32 idx = 0; idx < current->node_count; ++idx)
    {
        const HdPoseSample *node = current->nodes + idx;
        HdMixedPose *mixed = hd_state.mixed_poses + idx;
        if (node->parent != HD_NO_POSE && node->parent >= idx)
            return false;
        const MATRIXF *parent = node->parent != HD_NO_POSE ? &hd_state.mixed_poses[node->parent].source : NULL;
        hd_compose_pose(parent, node, &mixed->source);
        mixed->model = mixed->source;
        mixed->previous = HD_NO_POSE;
        uint32 pool_slot = hd_pool_key(node->id);
        if (pool_slot != HD_NO_POSE && (!previous->pool_valid[pool_slot] || !current->pool_valid[pool_slot] || !hd_state.pool_valid[pool_slot]))
            continue;
        const HdPoseIndex *old_entry = hd_state.previous_index + hd_pose_index_slot(node->id, hd_state.previous_index);
        const HdPoseIndex *new_entry = hd_state.current_index + hd_pose_index_slot(node->id, hd_state.current_index);
        if (old_entry->id == 0 || old_entry->index == HD_NO_POSE || new_entry->index == HD_NO_POSE)
            continue;
        const HdPoseSample *old = previous->nodes + old_entry->index;
        if (old->owner != node->owner || old->space != node->space || (old->parent == HD_NO_POSE) != (node->parent == HD_NO_POSE))
            continue;
        if (node->parent != HD_NO_POSE &&
            (node->parent >= idx || old->parent >= old_entry->index || hd_state.mixed_poses[node->parent].previous != old->parent))
            continue;
        hd_mix_matrix(old, node, fract, &mixed->model);
        if (node->parent != HD_NO_POSE)
        {
            MATRIXF local = mixed->model;
            const MATRIXF *parent = &hd_state.mixed_poses[node->parent].model;
            if ((node->space & HD_POSE_PARENT_ROT) != 0)
                MulMatrix0f(parent, &local, &mixed->model);
            if ((node->space & HD_POSE_PARENT_POS) != 0)
            {
                VECTORF pos = {local.t[0], local.t[1], local.t[2]};
                RotTransf(parent, &pos, &pos);
                TransMatrixf(&mixed->model, &pos);
            }
        }
        mixed->previous = old_entry->index;
    }
    return true;
}

void hd_source_model(const MATRIX *source, MATRIXF *model)
{
    for (uint32 row = 0; row < 3; ++row)
    {
        for (uint32 column = 0; column < 3; ++column)
            model->m[row][column] = source->m[row][column] / 4096.0f;
        model->t[row] = source->t[row] / 32.0f;
    }
}

static void hd_quat_pose(const SceneNode *node, const sint16 *rotation, MATRIXF *model)
{
    QUATF quat = hd_animation_rotation(node, rotation);
    hd_animation_quat_matrix(&quat, model);
}

static void hd_path_model(const PathTransform *path, MATRIXF *model)
{
    for (uint32 idx = 0; idx < hd_state.path_count; ++idx)
    {
        const HdPathPose *pose = hd_state.paths + idx;
        if (pose->path == path && memcmp(&pose->source, &path->matrix, sizeof(MATRIX)) == 0)
        {
            *model = pose->model;
            return;
        }
    }
    hd_source_model(&path->matrix, model);
}

static VECTORF hd_actor_position(const AnimatedObject *actor)
{
    VECTORF pos = hd_animation_position(&actor->node, &actor->pos);
    if ((actor->flags & 0x2000) == 0 && actor->path_definition != NULL && actor->path_definition->transform != NULL)
    {
        MATRIXF path;
        hd_path_model(actor->path_definition->transform, &path);
        RotTransf(&path, &pos, &pos);
    }
    return pos;
}

void hd_root_pose(const AnimatedObject *actor, MATRIXF *model)
{
    IdentityMatrixf(model);
    if (actor->orientation[0] == 0x7FFF)
    {
        RotMatrixXf((actor->orientation[1] & 4095) * RAD12, model);
        RotMatrixYf((actor->orientation[2] & 4095) * RAD12, model);
    }
    else
        hd_quat_pose(&actor->node, actor->orientation, model);
    VECTORF pos = hd_actor_position(actor);
    TransMatrixf(model, &pos);
    bool special = actor->behavior == 17 && actor->behavior_suspended == 2;
    if (!special && (actor->movement_mode >= 4 || actor->behavior_suspended == 2) && (actor->flags & 0x30000000) != 0)
    {
        float scale = (uint16)actor->state / 4096.0f;
        VECTORF factors = {scale, scale, scale};
        ScaleMatrixf(model, &factors);
    }
}

void hd_node_pose(const SceneNode *node, const AnimatedObject *owner, const MATRIXF *parent, MATRIXF *model)
{
    bool special = owner->behavior == 17 && owner->behavior_suspended == 2;
    if (node->kind == 5)
    {
        const MeshPartNode *part = (const MeshPartNode *)node;
        MATRIXF local;
        IdentityMatrixf(&local);
        if (special)
        {
            uint32 rate = (15u - owner->path_motion_scale) * 16u;
            uint32 packed = (uint16)part->rotation[3];
            MATRIXF y_basis;
            IdentityMatrixf(&y_basis);
            RotMatrixXf(((packed & 31) * rate & 4095) * RAD12, &local);
            RotMatrixYf((((packed >> 5) & 31) * rate & 4095) * RAD12, &y_basis);
            MulMatrix0f(&local, &y_basis, model);
            RotMatrixZf((((packed >> 10) & 31) * rate & 4095) * RAD12, model);
            VECTORF pos = hd_animation_position(node, &part->pos);
            TransMatrixf(model, &pos);
        }
        else
        {
            if (part->rotation[0] != 0x4000 || hd_animation_has_rotation(node, part->rotation))
                hd_quat_pose(node, part->rotation, &local);
            VECTORF pos = hd_animation_position(node, &part->pos);
            TransMatrixf(&local, &pos);
            CompMatrixf(parent, &local, model);
        }
        return;
    }
    const Vec3 *pos = NULL;
    if (node->kind == 7)
    {
        pos = &((const SpriteRecord *)node)->pos;
        if ((node->flags & 1) == 0 && (node->state_flags & 0x200) == 0)
            memcpy(model->m, parent->m, sizeof(model->m));
        if (special)
        {
            memcpy(model->t, parent->t, sizeof(model->t));
            return;
        }
    }
    else if (node->kind == 6)
        pos = &((const SphereNode *)node)->pos;
    if (pos != NULL)
    {
        VECTORF offset = hd_animation_position(node, pos);
        RotTransf(parent, &offset, &offset);
        TransMatrixf(model, &offset);
    }
}

bool hd_reserve_poses(uint32 depth)
{
    if (depth < hd_state.pose_capacity)
        return true;
    uint32 capacity = hd_state.pose_capacity != 0 ? hd_state.pose_capacity * 2 : 16;
    if (capacity <= depth || capacity > (size_t)-1 / sizeof(HdPose))
        return false;
    HdPose *poses = (HdPose *)realloc(hd_state.poses, capacity * sizeof(HdPose));
    if (poses == NULL)
        return false;
    memset(poses + hd_state.pose_capacity, 0, (capacity - hd_state.pose_capacity) * sizeof(HdPose));
    hd_state.poses = poses;
    hd_state.pose_capacity = capacity;
    return true;
}

static bool hd_animation_scale(const MATRIXF *local, const QUATF *rotation, VECTORF *scale)
{
    MATRIXF basis;
    IdentityMatrixf(&basis);
    hd_animation_quat_matrix(rotation, &basis);
    float factors[3];
    for (uint32 column = 0; column < 3; ++column)
    {
        float sq = 0.0f, dot = 0.0f;
        for (uint32 row = 0; row < 3; ++row)
        {
            sq += basis.m[row][column] * basis.m[row][column];
            dot += basis.m[row][column] * local->m[row][column];
        }
        if (sq < 0.00000001f)
            return false;
        factors[column] = dot / sq;
        for (uint32 row = 0; row < 3; ++row)
            if (fabsf(local->m[row][column] - basis.m[row][column] * factors[column])
                > 0.00001f * (1.0f + fabsf(local->m[row][column])))
                return false;
    }
    scale->vx = factors[0];
    scale->vy = factors[1];
    scale->vz = factors[2];
    return true;
}

static void hd_animation_capture(HdPoseSample *sample, const HdPose *pose, const sint16 *rotation)
{
    HdAnimationPose *animation = hd_animation_entry(pose->node, false);
    if (animation == NULL || animation->pending || animation->owner != pose->owner)
        return;
    const sint16 *source_rotation;
    const sint8 *binding;
    const Vec3 *source_pos = hd_animation_fields(pose->node, &source_rotation, &binding);
    if (source_pos != NULL && animation->pos_valid && animation->position_channel.curve != NULL
        && memcmp(source_pos, &animation->source_pos, sizeof(Vec3)) == 0
        && sample->local.t[0] == animation->pos.vx && sample->local.t[1] == animation->pos.vy && sample->local.t[2] == animation->pos.vz)
    {
        sample->position_channel = animation->position_channel;
        hd_animation_retain(&sample->position_channel);
    }
    if (rotation != NULL && animation->rotation_valid && animation->rotation_channel.curve != NULL
        && memcmp(rotation, &animation->source_rotation, sizeof(Quat)) == 0
        && hd_animation_scale(&sample->local, &animation->rotation, &sample->animation_scale))
    {
        sample->rotation_channel = animation->rotation_channel;
        hd_animation_retain(&sample->rotation_channel);
    }
}

static void hd_bind_pose(HdPose *pose, const MATRIXF *local, uint32 space, const sint16 *rotation)
{
    pose->sample = HD_NO_POSE;
    if (hd_state.node_count == hd_state.node_capacity)
    {
        HdPoseSample *nodes = (HdPoseSample *)hd_grow_sample(hd_state.nodes, &hd_state.node_capacity, hd_state.node_count + 1, sizeof(HdPoseSample));
        if (nodes == NULL)
        {
            hd_frame.failed = true;
            return;
        }
        hd_state.nodes = nodes;
    }
    HdPoseSample *sample = hd_state.nodes + hd_state.node_count;
    memset(sample, 0, sizeof(*sample));
    sample->id = hd_identity(pose->node);
    sample->owner = hd_identity(pose->owner != NULL ? &pose->owner->node : NULL);
    if (sample->id == 0 || (pose->owner != NULL && sample->owner == 0))
    {
        hd_frame.failed = true;
        return;
    }
    sample->parent = hd_state.pose_depth != 0 ? (pose - 1)->sample : HD_NO_POSE;
    sample->space = space;
    sample->local = *local;
    if (rotation != NULL)
    {
        QUATF source = hd_animation_rotation(pose->node, rotation);
        NormalizeQuatf(&source, &sample->rotation);
    }
    else
        MatrixQuatf(local, &sample->rotation);
    hd_animation_capture(sample, pose, rotation);
    pose->sample = hd_state.node_count++;
    uint32 pool_slot = hd_pool_key(sample->id);
    if (pool_slot != HD_NO_POSE)
        hd_state.pool_valid[pool_slot] = true;
}

void hd_frame_root(const AnimatedObject *actor)
{
    if (!hd_frame.collecting || hd_frame.failed)
        return;
    if (!hd_reserve_poses(1))
    {
        hd_frame.failed = true;
        return;
    }
    hd_state.pose_depth = 1;
    hd_state.pose_owner = actor;
    hd_state.pose_active = true;
    IdentityMatrixf(&hd_state.poses[0].model);
    hd_state.poses[0].owner = NULL;
    hd_state.poses[0].node = NULL;
    hd_state.poses[0].valid = true;
    hd_state.poses[0].sample = HD_NO_POSE;
    HdPose *pose = hd_state.poses + 1;
    hd_root_pose(actor, &pose->model);
    pose->node = &actor->node;
    pose->owner = actor;
    pose->valid = true;
    hd_bind_pose(pose, &pose->model, 0, actor->orientation[0] != 0x7FFF ? actor->orientation : NULL);
    if (!hd_frame.failed && actor->orientation[0] == 0x7FFF)
    {
        MATRIXF rotation;
        IdentityMatrixf(&rotation);
        RotMatrixXf((actor->orientation[1] & 4095) * RAD12, &rotation);
        RotMatrixYf((actor->orientation[2] & 4095) * RAD12, &rotation);
        MatrixQuatf(&rotation, &hd_state.nodes[pose->sample].rotation);
    }
}

void hd_frame_root_end(void)
{
    hd_state.pose_owner = NULL;
    hd_state.pose_active = false;
}

void hd_frame_scale(float scale)
{
    if (!hd_state.pose_active || hd_frame.failed)
        return;
    VECTORF factors = {scale, scale, scale};
    ScaleMatrixf(&hd_state.poses[hd_state.pose_depth].model, &factors);
    HdPose *pose = hd_state.poses + hd_state.pose_depth;
    if (pose->sample != HD_NO_POSE)
    {
        HdPoseSample *sample = hd_state.nodes + pose->sample;
        ScaleMatrixf(&sample->local, &factors);
        }
}

void hd_frame_push(const MATRIX *initial)
{
    if (!hd_state.pose_active || hd_frame.failed)
        return;
    uint32 depth = hd_state.pose_depth + 1;
    if (!hd_reserve_poses(depth))
    {
        hd_frame.failed = true;
        return;
    }
    HdPose *pose = hd_state.poses + depth;
    pose->sample = HD_NO_POSE;
    if (!pose->valid)
    {
        // Seed inherited scratch rotation before this slot is first overwritten
        hd_source_model(initial, &pose->model);
        pose->valid = true;
    }
    hd_state.pose_depth = depth;
}

void hd_frame_pop(void)
{
    if (hd_state.pose_active && !hd_frame.failed && hd_state.pose_depth != 0)
    {
        --hd_state.pose_depth;
        hd_state.pose_owner = hd_state.poses[hd_state.pose_depth].owner;
    }
}

void hd_frame_node(const SceneNode *node, const AnimatedObject *owner)
{
    if (hd_state.pose_owner != owner || hd_frame.failed || hd_state.pose_depth == 0)
        return;
    HdPose *pose = hd_state.poses + hd_state.pose_depth;
    hd_node_pose(node, owner, &(pose - 1)->model, &pose->model);
    pose->node = node;
    pose->owner = owner;
    MATRIXF identity, local = pose->model;
    IdentityMatrixf(&identity);
    hd_node_pose(node, owner, &identity, &local);
    uint32 space = 0;
    const sint16 *rotation = NULL;
    if (node->kind == 5)
    {
        if (!(owner->behavior == 17 && owner->behavior_suspended == 2))
        {
            space = HD_POSE_PARENT_POS | HD_POSE_PARENT_ROT;
            const MeshPartNode *part = (const MeshPartNode *)node;
            if (part->rotation[0] != 0x4000 || hd_animation_has_rotation(node, part->rotation))
                rotation = part->rotation;
        }
    }
    else if (node->kind == 7)
        space = HD_POSE_PARENT_POS | (((node->flags & 1) == 0 && (node->state_flags & 0x200) == 0) ? HD_POSE_PARENT_ROT : 0);
    else if (node->kind == 6)
        space = HD_POSE_PARENT_POS;
    hd_bind_pose(pose, &local, space, rotation);
}

void hd_graph_pose(const SceneNode *node, const MATRIXF *parent, bool inherit_sprite, const Vec3 *heading, MATRIXF *model)
{
    const Vec3 *pos = NULL;
    const sint16 *rotation = NULL;
    if (node->kind == 0)
    {
        const AnimatedObject *actor = (const AnimatedObject *)node;
        if (actor->behavior == 22)
        {
            hd_quat_pose(node, actor->orientation, model);
            VECTORF world = hd_actor_position(actor);
            TransMatrixf(model, &world);
            return;
        }
        pos = &actor->pos;
        rotation = actor->orientation;
    }
    else if (node->kind == 5)
    {
        const MeshPartNode *part = (const MeshPartNode *)node;
        pos = &part->pos;
        rotation = part->rotation;
    }
    else if (node->kind == 2)
    {
        const SceneTrigger *trigger = (const SceneTrigger *)node;
        pos = &trigger->pos;
        if (heading != NULL)
        {
            IdentityMatrixf(model);
            float yaw = atan2f(heading->z, heading->x);
            if ((sint32)trigger->parameters.path_pair.tangent_dir >= 0)
                RotMatrixYf(yaw - 2048 * RAD12, model);
            else
            {
                float pitch = asinf(fmaxf(-1.0f, fminf(1.0f, -heading->y / 4096.0f)));
                RotMatrixXf(pitch, model);
                RotMatrixYf(yaw, model);
            }
        }
        else
            rotation = trigger->rotation;
    }
    else if (node->kind == 7)
    {
        pos = &((const SpriteRecord *)node)->pos;
        if (inherit_sprite)
            memcpy(model->m, parent->m, sizeof(model->m));
    }
    else if (node->kind == 6)
        pos = &((const SphereNode *)node)->pos;
    else if (node->kind == 8)
        pos = &((const PositionedBillboardNode *)node)->pos;
    if (rotation != NULL)
    {
        MATRIXF local;
        IdentityMatrixf(&local);
        if (rotation[0] != 0x4000 || hd_animation_has_rotation(node, rotation))
            hd_quat_pose(node, rotation, &local);
        MulMatrix0f(parent, &local, model);
    }
    if (pos != NULL)
    {
        VECTORF offset = hd_animation_position(node, pos);
        RotTransf(parent, &offset, &offset);
        TransMatrixf(model, &offset);
    }
}

void hd_frame_graph_begin(const MATRIX *initial)
{
    if (!hd_frame.collecting || hd_frame.failed)
        return;
    if (hd_state.graph_depth == 0)
    {
        if (!hd_reserve_poses(0))
        {
            hd_frame.failed = true;
            return;
        }
        HdPose *root = hd_state.poses;
        IdentityMatrixf(&root->model);
        root->node = NULL;
        root->owner = NULL;
        root->valid = true;
        root->sample = HD_NO_POSE;
        hd_state.pose_depth = 0;
        hd_state.pose_owner = NULL;
        hd_state.pose_active = true;
    }
    ++hd_state.graph_depth;
    hd_frame_push(initial);
}

void hd_frame_graph_node(const SceneNode *node, const AnimatedObject *parent, uint32 work, const Vec3 *heading)
{
    if (!hd_state.pose_active || hd_frame.failed || hd_state.graph_depth == 0)
        return;
    bool draw = (work & 0x14) == 0x10;
    bool children = node->links.first_child != NULL;
    bool inherit = parent != NULL ? draw || children : !draw && children;
    HdPose *pose = hd_state.poses + hd_state.pose_depth;
    hd_graph_pose(node, &(pose - 1)->model, inherit, heading, &pose->model);
    pose->node = node;
    pose->owner = node->kind == 0 ? (const AnimatedObject *)node : parent;
    hd_state.pose_owner = pose->owner;
    MATRIXF identity, local = pose->model;
    IdentityMatrixf(&identity);
    hd_graph_pose(node, &identity, inherit, heading, &local);
    uint32 space = 0;
    const sint16 *rotation = NULL;
    if (node->kind == 0)
    {
        const AnimatedObject *actor = (const AnimatedObject *)node;
        space = actor->behavior == 22 ? 0 : HD_POSE_PARENT_POS | HD_POSE_PARENT_ROT;
        if (actor->behavior == 22 || actor->orientation[0] != 0x4000 || hd_animation_has_rotation(node, actor->orientation))
            rotation = actor->orientation;
    }
    else if (node->kind == 5)
    {
        space = HD_POSE_PARENT_POS | HD_POSE_PARENT_ROT;
        const MeshPartNode *part = (const MeshPartNode *)node;
        if (part->rotation[0] != 0x4000 || hd_animation_has_rotation(node, part->rotation))
            rotation = part->rotation;
    }
    else if (node->kind == 2)
    {
        space = HD_POSE_PARENT_POS | (heading == NULL ? HD_POSE_PARENT_ROT : 0);
        const SceneTrigger *trigger = (const SceneTrigger *)node;
        if (heading == NULL && (trigger->rotation[0] != 0x4000 || hd_animation_has_rotation(node, trigger->rotation)))
            rotation = trigger->rotation;
    }
    else if (node->kind == 7)
        space = HD_POSE_PARENT_POS | (inherit ? HD_POSE_PARENT_ROT : 0);
    else if (node->kind == 6 || node->kind == 8)
        space = HD_POSE_PARENT_POS;
    hd_bind_pose(pose, &local, space, rotation);
}

void hd_frame_graph_path(const PathTransform *path)
{
    if (!hd_state.pose_active || hd_frame.failed || hd_state.graph_depth == 0 || path == NULL)
        return;
    uint32 idx = 0;
    while (idx < hd_state.path_count && hd_state.paths[idx].path != path)
        ++idx;
    if (idx == hd_state.path_capacity)
    {
        uint32 capacity = hd_state.path_capacity != 0 ? hd_state.path_capacity * 2 : 32;
        if (capacity <= idx || capacity > (size_t)-1 / sizeof(HdPathPose))
        {
            hd_frame.failed = true;
            return;
        }
        HdPathPose *paths = (HdPathPose *)realloc(hd_state.paths, capacity * sizeof(HdPathPose));
        if (paths == NULL)
        {
            hd_frame.failed = true;
            return;
        }
        hd_state.paths = paths;
        hd_state.path_capacity = capacity;
    }
    if (idx == hd_state.path_count)
        ++hd_state.path_count;
    HdPathPose *pose = hd_state.paths + idx;
    pose->path = path;
    pose->source = path->matrix;
    pose->model = hd_state.poses[hd_state.pose_depth - 1].model;
}

void hd_invalidate_paths(void)
{
    hd_state.path_count = 0;
}

void hd_frame_graph_end(void)
{
    if (hd_state.graph_depth == 0)
        return;
    hd_frame_pop();
    if (--hd_state.graph_depth == 0)
    {
        hd_state.pose_owner = NULL;
        hd_state.pose_active = false;
    }
}

void hd_pose_camera_scratch(const MATRIXF *model)
{
    uint32 depth = hd_state.pose_depth + 1;
    if (!hd_reserve_poses(depth))
    {
        hd_frame.failed = true;
        return;
    }
    HdPose *pose = hd_state.poses + depth;
    CompMatrixf(&hd_frame.camera.view, model, &pose->model);
    pose->valid = true;
}
