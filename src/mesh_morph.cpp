#include "game.h"

struct MorphBlendContext
{
    const AnimatedObject *owner;
    const MorphSpan *span;
    MeshVertex *destination;
    const SVECTOR *source0;
    const SVECTOR *source1;
    uint32 weight;

    void Apply() const;
};

void SampleMorphTrack(sint32 frame, const MorphAnimationTrack *track, const AnimatedObject *owner, const MorphSpan *span)
{
    FUNCTION_MARKER(0x80019684, "SLUS_005.78");
    MorphBlendContext context;
    const MorphAnimationKey *keys, *previous, *next;
    uint32 frame_value, end_value;
    const SVECTOR *source;
    MeshVertex *destination;
    uint32 remaining;
    uint32 first, second;
    if ((owner->flags & 0x200) != 0 && (track->track.flags & 1) != 0)
    {
        return;
    }
    context.owner = owner;
    context.span = span;
    context.destination = (((MeshVertex *)(owner->vertices))) + (uint32)span->first_vertex;
    keys = (const MorphAnimationKey *)track->track.keys;
    next = keys;
    previous = keys + track->track.key_count - 1;
    end_value = previous->time;
    frame_value = ((uint32)frame << 20) | 0xFFFFF;
    ++next;
    if (frame_value < end_value)
    {
        do
        {
            end_value = next->time;
            ++next;
        } while (frame_value >= end_value);
        --next;
        previous = next - 1;
        goto interpolate;
    }
    --next;
    if (frame_value == (end_value | 0xFFFFF))
    {
        uint32 stride = (uint32)track->vertex_stride;
        uint32 index = previous->frame_index;
        destination = context.destination;
        remaining = (uint32)span->vertex_count;
        source = (const SVECTOR *)((const uint8 *)((const SVECTOR *)(track->vertices)) + ((index * stride) << 3));
        goto copy_vertices;
    }
    end_value = ((uint32)track->track.frame_limit << 20) | 0xFFFFF;
    if (frame_value < end_value)
        goto interpolate;
    {
        const MorphAnimationKey *key = ((track->track.flags & 3) == 3) ? next : previous;
        uint32 stride = (uint32)track->vertex_stride;
        uint32 index = key->frame_index;
        source = (const SVECTOR *)((const uint8 *)((const SVECTOR *)(track->vertices)) + ((index * stride) << 3));
    }
    context.source0 = source;
    span = context.span;
    owner = context.owner;
    destination = context.destination;
    source = context.source0;
    remaining = (uint32)span->vertex_count;
copy_vertices:
    do
    {
        destination->normal.vz = 0;
        destination->normal.vy = 0;
        destination->normal.vx = 0;
        first = xport_load_le32(source);
        second = xport_load_le32((const uint8 *)source + 4);
        xport_store_le32(&destination->position, first);
        xport_store_le32((uint8 *)&destination->position + 4, second);
        ++source;
        ++destination;
        --remaining;
    } while (remaining != 0);
    owner->RebuildMorphNormals(span);
    return;
interpolate:
{
    const uint8 *key_data = track->track.keys;
    uint32 value = InterpolateKeyTime(frame_value, end_value, previous->time, next->time, key_data);
    uint32 stride, index;
    context.weight = 4096u - (uint32)((sint32)value >> 8);
    stride = (uint32)track->vertex_stride;
    index = previous->frame_index;
    context.source0 = (const SVECTOR *)((const uint8 *)((const SVECTOR *)(track->vertices)) + ((index * stride) << 3));
    stride = (uint32)track->vertex_stride;
    index = next->frame_index;
    context.source1 = (const SVECTOR *)((const uint8 *)((const SVECTOR *)(track->vertices)) + ((index * stride) << 3));
    context.Apply();
}
}

void MorphBlendContext::Apply() const
{
    FUNCTION_MARKER(0x8001BC44, "SLUS_005.78");
    const SVECTOR *source0 = this->source0;
    const MorphSpan *span = this->span;
    const AnimatedObject *owner = this->owner;
    const SVECTOR *source1 = this->source1;
    uint32 weight = this->weight;
    MeshVertex *destination = this->destination;
    uint32 inverse = 4096u - weight;
    uint32 remaining = (uint32)span->vertex_count;
    uint32 x, y, z;
    do
    {
        destination->normal.vz = 0;
        destination->normal.vy = 0;
        destination->normal.vx = 0;
        xport_gte_write_data(8, weight);
        x = (uint16)source0->vx;
        y = (uint16)source0->vy;
        z = (uint16)source0->vz;
        xport_gte_write_data(9, x);
        xport_gte_write_data(10, y);
        xport_gte_write_data(11, z);
        xport_gte_execute(0x198003D);
        xport_gte_write_data(8, inverse);
        x = (uint16)source1->vx;
        y = (uint16)source1->vy;
        z = (uint16)source1->vz;
        xport_gte_write_data(9, x);
        xport_gte_write_data(10, y);
        xport_gte_write_data(11, z);
        xport_gte_execute(0x1A8003E);
        x = xport_gte_read_data(9);
        y = xport_gte_read_data(10);
        z = xport_gte_read_data(11);
        destination->position.vx = x;
        destination->position.vy = y;
        destination->position.vz = z;
        ++destination;
        ++source0;
        --remaining;
        ++source1;
    } while (remaining != 0);
    owner = this->owner;
    span = this->span;
    owner->RebuildMorphNormals(span);
}

void AnimatedObject::RebuildMorphNormals(const MorphSpan *span) const
{
    FUNCTION_MARKER(0x80019260, "SLUS_005.78");
    uint32 first_vertex = (uint32)span->first_vertex;
    MeshVertex *vertices;
    MeshFace *first_polygon = ((MeshFace *)(polygons)) + span->first_polygon;
    MeshFace *polygon;
    uint32 remaining = (uint32)span->polygon_count;
    uint32 edge0[3], edge1[3], normal[3];
    MeshVertex *vertex0, *vertex1, *vertex2, *vertex3;
    uint32 x, y, z, dot, plane, i;
    const MATRIX *parent_matrix;
    vertices = (((MeshVertex *)(this->vertices))) + first_vertex;
    if (remaining != 0)
    {
        polygon = first_polygon;
        do
        {
            vertex0 = vertices + (polygon->vertex_indices[0] - first_vertex);
            vertex1 = vertices + (polygon->vertex_indices[1] - first_vertex);
            vertex3 = vertices + (polygon->vertex_indices[3] - first_vertex);
            vertex2 = vertices + (polygon->vertex_indices[2] - first_vertex);
            edge0[0] = (uint32)vertex2->position.vx - (uint32)vertex0->position.vx;
            edge0[1] = (uint32)vertex2->position.vy - (uint32)vertex0->position.vy;
            edge0[2] = (uint32)vertex2->position.vz - (uint32)vertex0->position.vz;
            edge1[0] = (uint32)vertex3->position.vx - (uint32)vertex1->position.vx;
            edge1[1] = (uint32)vertex3->position.vy - (uint32)vertex1->position.vy;
            edge1[2] = (uint32)vertex3->position.vz - (uint32)vertex1->position.vz;
            xport_gte_write_control(0, edge0[0]);
            xport_gte_write_control(2, edge0[1]);
            xport_gte_write_control(4, edge0[2]);
            xport_gte_write_data(11, edge1[2]);
            xport_gte_write_data(9, edge1[0]);
            xport_gte_write_data(10, edge1[1]);
            xport_gte_execute(0x178000C);
            normal[0] = xport_gte_read_data(25);
            normal[1] = xport_gte_read_data(26);
            normal[2] = xport_gte_read_data(27);
            while (normal[0] + 26754u > 0xD103u || normal[1] + 26754u > 0xD103u || normal[2] + 26754u > 0xD103u)
            {
                x = (uint32)((sint32)normal[0] >> 1);
                y = (uint32)((sint32)normal[1] >> 1);
                z = (uint32)((sint32)normal[2] >> 1);
                normal[0] = x;
                normal[1] = y;
                normal[2] = z;
            }
            VectorNormalS((VECTOR *)normal, &polygon->normal.vector);
            dot = (uint32)(vertex0->position.vx * polygon->normal.vector.vx);
            dot += (uint32)(vertex0->position.vy * polygon->normal.vector.vy);
            dot += (uint32)(vertex0->position.vz * polygon->normal.vector.vz);
            plane = 0u - (uint32)((sint32)dot >> 7);
            polygon->plane = plane;
            if ((((const TextureDescriptor *)(polygon->texture))->flags & 1) != 0)
                polygon->plane = plane + 0x3FFFF800;
            for (i = 0; i < 4; ++i)
            {
                MeshVertex *vertex = i == 0 ? vertex0 : i == 1 ? vertex1 : i == 2 ? vertex2 : vertex3;
                vertex->normal.vx = (uint16)vertex->normal.vx + (uint16)polygon->normal.vector.vx;
                vertex->normal.vy = (uint16)vertex->normal.vy + (uint16)polygon->normal.vector.vy;
                vertex->normal.vz = (uint16)vertex->normal.vz + (uint16)polygon->normal.vector.vz;
            }
            ++polygon;
            --remaining;
        } while (remaining != 0);
    }
    remaining = (uint32)span->vertex_count;
    if (remaining != 0)
    {
        MeshVertex *vertex = vertices;
        do
        {
            VectorNormalSS(&vertex->normal, &vertex->normal);
            ++vertex;
            --remaining;
        } while (remaining != 0);
    }
    parent_matrix = (render_view.matrix_cursor->prev != 0 ? &render_view.matrix_cursor->prev->matrix : 0);
    for (i = 0; i < 5; ++i)
        xport_gte_write_control(i, xport_load_le32((const uint8 *)parent_matrix + i * 4));
}
