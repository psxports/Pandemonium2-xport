#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "morph_blend_context.h"
#include "game.h"
#include "psx_gte.h"

void MorphBlendContext::Apply() const
{
    FUNCTION_MARKER(0x8001BC44, "SLUS_005.78");
    const SVECTOR *source0_val = source0;
    const MorphSpan *span_val = span;
    const AnimatedObject *owner_val = owner;
    const SVECTOR *source1_val = source1;
    uint32 weight_val = weight;
    MeshVertex *dst_val = dst;
    uint32 inverse = 4096u - weight_val;
    uint32 remaining = span_val->vertex_count;
    uint32 x, y, z;
    do
    {
        dst_val->n.vz = 0;
        dst_val->n.vy = 0;
        dst_val->n.vx = 0;
        xport_gte_write_data(8, weight_val);
        x = (uint16)source0_val->vx;
        y = (uint16)source0_val->vy;
        z = (uint16)source0_val->vz;
        xport_gte_write_data(9, x);
        xport_gte_write_data(10, y);
        xport_gte_write_data(11, z);
        xport_gte_execute(0x198003D);
        xport_gte_write_data(8, inverse);
        x = (uint16)source1_val->vx;
        y = (uint16)source1_val->vy;
        z = (uint16)source1_val->vz;
        xport_gte_write_data(9, x);
        xport_gte_write_data(10, y);
        xport_gte_write_data(11, z);
        xport_gte_execute(0x1A8003E);
        x = xport_gte_read_data(9);
        y = xport_gte_read_data(10);
        z = xport_gte_read_data(11);
        dst_val->pos.vx = x;
        dst_val->pos.vy = y;
        dst_val->pos.vz = z;
        ++dst_val;
        ++source0_val;
        --remaining;
        ++source1_val;
    } while (remaining != 0);
    owner_val = owner;
    span_val = span;
    owner_val->RebuildMorphNormals(span_val);
}
