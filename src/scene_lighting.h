#ifndef SCENE_LIGHTING_H
#define SCENE_LIGHTING_H

#include <stddef.h>

#include "psx.h"
#include "object_links.h"

union SceneLightVector
{
    uint32 position[3];
    SVECTOR direction;
};

// Mutable light record with native scene links
struct SceneLight
{
    ObjectLinks links;
    uint8 kind, flags, event_flags, id;
    uint32 event_id;
    uint32 position[3];
    SceneLightVector source;
    uint32 prev_position[3];
    uint32 relative_position[3];
    uint32 color[3];
    uint32 radius, attenuation_range, metadata_word;
    uint32 color_scale[3];
};

typedef char SceneLightBinarySize[(sizeof(SceneLight) == 104) ? 1 : -1];
typedef char SceneLightSourceOffset[(offsetof(SceneLight, source) == 32) ? 1 : -1];
typedef char SceneLightColorOffset[(offsetof(SceneLight, color) == 68) ? 1 : -1];

// Keep the original overlapping light-direction and matrix workspace
struct SceneLightTransformFields
{
    uint16 directions[10];
    sint16 inverse_rotation[10];
    uint32 transformed_origin[3];
};

union SceneLightTransformWorkspace
{
    SceneLightTransformFields fields;
    uint16 direction_output[26];
};

typedef char SceneLightTransformSize[(sizeof(SceneLightTransformWorkspace) == 52) ? 1 : -1];
typedef char SceneLightInverseOffset[(offsetof(SceneLightTransformFields, inverse_rotation) == 20) ? 1 : -1];
typedef char SceneLightOriginOffset[(offsetof(SceneLightTransformFields, transformed_origin) == 40) ? 1 : -1];

struct SceneLighting
{
    SceneLight *lights;
    const uint32 *bounds_origin;
    sint16 bounds_radius;
    SceneLight *point_lights[4];
    uint32 flags;
    uint8 directional_count, skip_transform;
    uint16 ambient[4];
    uint16 colors[10];
    SceneLightTransformWorkspace transforms;
};

extern SceneLighting scene_lighting;

void MarkSceneLightingDirty(const SceneLight *light);
void UpdateSceneLights();
uint32 SetLightingBounds(const uint32 *origin, sint32 radius);
uint32 SelectObjectLights();
uint32 TransformObjectLights(uint32 mode);
uint32 ApplyVertexLights(const SVECTOR *vertex, uint32 flags);

#endif
