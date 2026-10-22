#ifndef SCENE_LIGHTING_H
#define SCENE_LIGHTING_H
#include <stddef.h>

#include "psx.h"
#include "object_links.h"
#include "vector3.h"

union SceneLightVector;
struct EncodedSceneLight;
struct SceneLight;
struct SceneLighting;

union SceneLightVector
{
    Vec3 pos;
    SVECTOR dir;
};

struct EncodedSceneLight
{
    uint32 registry_offset;
    uint32 sibling_offset;
    uint32 child_offset;
    uint8 kind;
    uint8 flags;
    uint8 event_flags;
    uint8 id;
    uint32 event_id;
    Vec3 pos;
    SceneLightVector src;
    Vec3 prev_pos;
    Vec3 relative_pos;
    uint32 color[3];
    uint32 radius;
    uint32 attenuation_range;
    uint32 metadata_word;
    uint32 color_scale[3];
};

typedef char EncodedSceneLightSize[(sizeof(EncodedSceneLight) == 104) ? 1 : -1];

// Mutable light record with native scene links
struct SceneLight
{
    ObjectLinks links;
    uint8 kind, flags, event_flags, id;
    uint32 event_id;
    Vec3 pos;
    SceneLightVector src;
    Vec3 prev_pos;
    Vec3 relative_pos;
    uint32 color[3];
    uint32 radius, attenuation_range, metadata_word;
    uint32 color_scale[3];
};

typedef char SceneLightBinarySize[(sizeof(SceneLight) == 104) ? 1 : -1];

typedef char SceneLightSourceOffset[(offsetof(SceneLight, src) == 32) ? 1 : -1];

typedef char SceneLightColorOffset[(offsetof(SceneLight, color) == 68) ? 1 : -1];

struct SceneLighting
{
    SceneLight *lights;
    const sint32 *bounds_origin;
    sint16 bounds_radius;
    SceneLight *point_lights[4];
    uint32 flags;
    uint8 directional_count, skip_transform;
    uint16 ambient[4];
    MATRIX color_matrix;
    MATRIX light_matrix;
    MATRIX inverse_matrix;
    Vec3 origin;

    static void MarkSceneLightingDirty(const SceneLight *light);
    static void UpdateSceneLights();
    static void SelectObjectLights();
    static void SetLightingBounds(const sint32 *origin, sint32 radius);
    static void TransformObjectLights(uint32 mode);
    static void ApplyVertexLights(const SVECTOR *vertex, uint32 flags);
};

extern SceneLighting scene_lighting;

#endif
