#ifndef MODEL_GROUP_REGISTRY_H
#define MODEL_GROUP_REGISTRY_H
#include <stddef.h>

#include "psx.h"
#include "model_group_definition.h"

struct AnimatedObject;
struct ModelGroupDefinition;

struct AnimatedObject;

struct ModelGroupRegistry;

struct ModelGroupRegistry
{
    enum
    {
        CommonCount = 3,
        Capacity = 12
    };

    uint32 count;
    ModelGroupDefinition recs[Capacity];
    ModelGroupDefinition *definitions[Capacity];
    // File identities preserve aliases within each package
    uint32 src_packages[Capacity];
    uint32 src_offsets[Capacity];

    static void LoadModelGrp(uint8 kind);
    static AnimatedObject *UpdateModelGrp(AnimatedObject *animated_object);
    static void RetainCommonModelGrps(void);
    static void LoadCommonModelGrps(void);
    static void ClearModelGrps(void);
    static ModelGroupDefinition *FindRelatedModelGrp(uint32 kind);
    static ModelGroupDefinition *SetRelatedModelGrpContactType(uint32 kind, uint8 val);
    static uint32 GetModelGrpContactType(uint32 kind);
    static uint32 GetModelGrpLifetime(uint32 kind);
};

extern ModelGroupRegistry model_grps;

#endif
