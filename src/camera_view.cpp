#include <stdlib.h>

#include "game.h"
#include "original_defaults.h"

CameraProjection camera_projection = {OriginalDefaults::CameraProjectionScale};
CameraMotion camera_motion = {0};

static const uint16 camera_transition_factors[45] = {2048, 1365, 1024, 819, 682, 585, 512, 455, 409, 372, 341, 315, 292, 273, 256, 240, 227, 215, 204, 195, 186, 178, 170, 163, 157, 151, 146, 141, 136, 132, 128, 124, 120, 117, 113, 110, 107, 105, 102, 99, 97, 95, 93, 91, 89};

uint32 SetCameraProjection(sint32 scale)
{
    FUNCTION_MARKER(0x8002417C, "SLUS_005.78");
    uint32 point;
    uint32 ax;
    uint32 ay;
    uint32 az;
    uint32 bx;
    uint32 by;
    uint32 bz;
    uint32 first;
    uint32 second;
    uint32 vertices[4][3];
    VECTOR normal;
    sint32 projection = scale >> 12;
    camera_projection.view_normals[0].vz = 4096;
    camera_projection.view_normals[0].vx = 0;
    camera_projection.view_normals[0].vy = 0;
    vertices[0][0] = 0;
    vertices[0][1] = 0;
    vertices[0][2] = 0;
    vertices[1][0] = (uint32)-40960;
    vertices[1][1] = 30720;
    vertices[1][2] = (uint32)(scale >> 4);
    vertices[2][0] = (uint32)-40960;
    vertices[2][1] = (uint32)-30720;
    vertices[2][2] = (uint32)(scale >> 4);
    for (point = 0; point < 4; ++point)
    {
        if (point == 1)
        {
            vertices[1][0] = 40960;
            vertices[1][1] = (uint32)-30720;
        }
        else if (point == 2)
        {
            vertices[2][0] = 40960;
            vertices[2][1] = 30720;
        }
        else if (point == 3)
        {
            vertices[1][0] = (uint32)-40960;
            vertices[1][1] = 30720;
        }
        first = (point & 1) != 0 ? 2 : 1;
        second = (point & 1) != 0 ? 1 : 2;
        ax = vertices[first][0] - vertices[0][0];
        ay = vertices[first][1] - vertices[0][1];
        az = vertices[first][2] - vertices[0][2];
        bx = vertices[second][0] - vertices[first][0];
        by = vertices[second][1] - vertices[first][1];
        bz = vertices[second][2] - vertices[first][2];
        first = (uint32)(((sint64)(sint32)ay * (sint32)bz) >> 12);
        second = (uint32)(((sint64)(sint32)by * (sint32)az) >> 12);
        normal.vx = (sint32)(first - second);
        first = (uint32)(((sint64)(sint32)az * (sint32)bx) >> 12);
        second = (uint32)(((sint64)(sint32)bz * (sint32)ax) >> 12);
        normal.vy = (sint32)(first - second);
        first = (uint32)(((sint64)(sint32)ax * (sint32)by) >> 12);
        second = (uint32)(((sint64)(sint32)bx * (sint32)ay) >> 12);
        normal.vz = (sint32)(first - second);
        while ((uint32)normal.vx + 26754 > 0xD103 || (uint32)normal.vy + 26754 > 0xD103 || (uint32)normal.vz + 26754 > 0xD103)
        {
            normal.vx >>= 1;
            normal.vy >>= 1;
            normal.vz >>= 1;
        }
        VectorNormal(&normal, &normal);
        camera_projection.view_normals[point + 1].vx = (sint16)normal.vx;
        camera_projection.view_normals[point + 1].vy = (sint16)normal.vy;
        camera_projection.view_normals[point + 1].vz = (sint16)normal.vz;
    }
    xport_gte_write_control(26, (uint32)projection);
    xport_gte_write_control(21, 0);
    xport_gte_write_control(22, 0);
    xport_gte_write_control(23, 0);

    return SetFogNearFar(0x8000, 0xFFFF, projection);
}

void BuildCameraView(SceneSection *scene)
{
    MATRIX identity = psx_identity_matrix;
    MATRIX first_matrix, second_matrix;
    MATRIX *left, *right;

    FrustumPlane *plane;
    uint32 index;
    uint32 point;
    uint32 distance;
    uint32 old_distance;
    uint32 difference;
    uint32 sum;
    uint32 x;
    uint32 y;
    uint32 z;
    uint32 words[3];
    uint16 transpose[9];
    sint32 cosine;
    sint32 sine;
    sint32 quotient;
    sint32 coefficient;
    sint64 product;
    uint8 flags;
    FUNCTION_MARKER(0x800165C8, "SLUS_005.78");
    UpdateResourceMaterialAnimations();
    UpdateSceneLights();
    flags = camera_motion.flags;
    if ((flags & 1) == 0)
    {
        camera_motion.flags = (uint8)(flags | 1);
        UpdateCameraAim();
    }
    cosine = SinQ14((sint16)(camera_projection.rotation[0] + 1024)) >> 2;
    sine = SinQ14((sint16)(0 - camera_projection.rotation[0])) >> 2;
    rotation_x.m[1][1] = (sint16)((uint16)cosine);
    rotation_x.m[2][1] = (sint16)((uint16)sine);
    rotation_x.m[1][2] = (sint16)((uint16)(0 - (uint32)sine));
    rotation_x.m[2][2] = (sint16)((uint16)cosine);
    MulMatrix0(&identity, &rotation_x, &first_matrix);
    cosine = SinQ14((sint16)(camera_projection.rotation[1] + 1024)) >> 2;
    sine = SinQ14((sint16)(0 - camera_projection.rotation[1])) >> 2;
    rotation_y.m[0][0] = (sint16)((uint16)cosine);
    rotation_y.m[2][0] = (sint16)((uint16)(0 - (uint32)sine));
    rotation_y.m[0][2] = (sint16)((uint16)sine);
    rotation_y.m[2][2] = (sint16)((uint16)cosine);
    right = &rotation_y;
    if ((sint16)camera_projection.rotation[2] == 2048)
        left = &first_matrix;
    else
    {
        MulMatrix0(&first_matrix, right, &second_matrix);
        cosine = SinQ14((sint16)(camera_projection.rotation[2] - 1024)) >> 2;
        sine = SinQ14((sint16)(2048 - camera_projection.rotation[2])) >> 2;
        rotation_z.m[0][0] = (sint16)((uint16)cosine);
        rotation_z.m[1][0] = (sint16)((uint16)sine);
        rotation_z.m[0][1] = (sint16)((uint16)(0 - (uint32)sine));
        rotation_z.m[1][1] = (sint16)((uint16)cosine);
        left = &rotation_z;
        right = &second_matrix;
    }
    MulMatrix0(left, right, &render_view.camera_transform);
    x = xport_load_le32((const uint8 *)&render_view.camera_transform);
    y = xport_load_le32((const uint8 *)&render_view.camera_transform + 4);
    xport_gte_write_control(0, x);
    xport_gte_write_control(1, y);
    x = xport_load_le32((const uint8 *)&render_view.camera_transform + 8);
    y = xport_load_le32((const uint8 *)&render_view.camera_transform + 12);
    z = xport_load_le32((const uint8 *)&render_view.camera_transform + 16);
    xport_gte_write_control(2, x);
    xport_gte_write_control(3, y);
    xport_gte_write_control(4, z);
    ApplyRotMatrixLV((const VECTOR *)camera_motion.position, (VECTOR *)render_view.camera_transform.t);
    x = (uint32)render_view.camera_transform.t[0];
    y = (uint32)render_view.camera_transform.t[1];
    z = (uint32)render_view.camera_transform.t[2];
    render_view.camera_transform.t[0] = (sint32)(0u - x);
    render_view.camera_transform.t[1] = (sint32)(0u - y);
    render_view.camera_transform.t[2] = (sint32)(0u - z);
    transpose[0] = (uint16)render_view.camera_transform.m[0][0];
    transpose[1] = (uint16)render_view.camera_transform.m[1][0];
    transpose[2] = (uint16)render_view.camera_transform.m[2][0];
    transpose[3] = (uint16)render_view.camera_transform.m[0][1];
    transpose[4] = (uint16)render_view.camera_transform.m[1][1];
    transpose[5] = (uint16)render_view.camera_transform.m[2][1];
    transpose[6] = (uint16)render_view.camera_transform.m[0][2];
    transpose[7] = (uint16)render_view.camera_transform.m[1][2];
    transpose[8] = (uint16)render_view.camera_transform.m[2][2];
    for (index = 0; index < 9; ++index)
        render_view.inverse_transform.m[index / 3][index % 3] = (sint16)transpose[index];
    for (index = 0; index < 3; ++index)
        words[index] = xport_load_le32((const uint8 *)&render_view.inverse_transform + 4 * index);
    for (index = 0; index < 3; ++index)
        xport_store_le32((uint8 *)&render_view.billboard_transform + 4 * index, words[index]);
    x = xport_load_le32((const uint8 *)&render_view.inverse_transform + 12);
    y = (uint32)(sint32)render_view.inverse_transform.m[2][2];
    xport_store_le32((uint8 *)&render_view.billboard_transform + 12, x);
    render_view.billboard_transform.m[2][2] = (sint16)y;
    x = xport_load_le32((const uint8 *)&render_view.inverse_transform);
    y = xport_load_le32((const uint8 *)&render_view.inverse_transform + 4);
    xport_gte_write_control(0, x);
    xport_gte_write_control(1, y);
    x = xport_load_le32((const uint8 *)&render_view.inverse_transform + 8);
    y = xport_load_le32((const uint8 *)&render_view.inverse_transform + 12);
    z = xport_load_le32((const uint8 *)&render_view.inverse_transform + 16);
    xport_gte_write_control(2, x);
    xport_gte_write_control(3, y);
    xport_gte_write_control(4, z);
    for (point = 0; point < 5; ++point)
    {
        xport_gte_write_data(0, xport_load_le32((const uint8 *)&camera_projection.view_normals[point]));
        xport_gte_write_data(1, xport_load_le32((const uint8 *)&camera_projection.view_normals[point] + 4));
        xport_gte_mvmva(0x486012);
        render_view.transform_result.vx = xport_gte_read_data(25);
        render_view.transform_result.vy = xport_gte_read_data(26);
        render_view.transform_result.vz = xport_gte_read_data(27);
        plane = &render_view.planes[point];
        plane->normal.vx = (sint16)render_view.transform_result.vx;
        plane->normal.vy = (sint16)render_view.transform_result.vy;
        z = render_view.transform_result.vz;
        plane->normal.vz = (sint16)z;
        x = camera_motion.position[0];
        coefficient = plane->normal.vx;
        sum = (uint32)((sint64)((sint32)x >> 6) * coefficient);
        y = camera_motion.position[1];
        coefficient = plane->normal.vy;
        sum += (uint32)((sint64)((sint32)y >> 6) * coefficient);
        x = camera_motion.position[2];
        sum += (uint32)((sint64)((sint32)x >> 6) * (sint16)z);
        plane->bias = 0 - (uint32)((sint32)sum >> 6);
    }
    x = psx_identity_rotation_words[0];
    y = psx_identity_rotation_words[1];
    xport_gte_write_control(0, x);
    xport_gte_write_control(1, y);
    x = psx_identity_rotation_words[2];
    y = psx_identity_rotation_words[3];
    z = psx_identity_rotation_words[4];
    xport_gte_write_control(2, x);
    xport_gte_write_control(3, y);
    xport_gte_write_control(4, z);
    {
        MATRIX *root_matrix = &render_view.matrix_cursor->matrix;
        for (index = 0; index < 3; ++index)
            words[index] = psx_identity_rotation_words[index];
        for (index = 0; index < 3; ++index)
            xport_store_le32((uint8 *)root_matrix + 4 * index, words[index]);
        x = psx_identity_rotation_words[3];
        y = (uint32)(sint32)psx_identity_matrix.m[2][2];
        xport_store_le32((uint8 *)root_matrix + 12, x);
        root_matrix->m[2][2] = (sint16)y;
        root_matrix = &render_view.matrix_cursor->matrix;
        root_matrix->t[2] = 0;
        root_matrix->t[1] = 0;
        root_matrix->t[0] = 0;
    }
    distance = camera_projection.far_distance;
    old_distance = render_view.far_distance;
    if (distance != old_distance)
    {
        render_view.far_distance = distance;
        if (distance == 0)
            xport_mips_break(7);
        quotient = distance == 0 ? -1 : 0x40000000 / (sint32)distance;
        product = (sint64)0x400000 * quotient;
        coefficient = (sint32)(product >> 18);
        fixed_math_result = (uint32)coefficient;
        render_view.fog_scale = (uint32)coefficient;
    }
    if (scene != 0)
    {
        distance = scene->origin[2];
        if (distance != 0)
        {
            old_distance = scene->origin[1];
            difference = distance - old_distance;
            if (difference == 0)
                xport_mips_break(7);
            quotient = difference == 0 ? -1 : 0x40000000 / (sint32)difference;
            product = (sint64)0x100000 * quotient;
            coefficient = (sint32)(product >> 18);
            fixed_math_result = (uint32)coefficient;
            render_view.depth_scale = (uint32)coefficient;
        }
        else
            xport_gte_write_data(8, 0);
        if (screen_render.fade_level < 4096)
        {
            render_view.lighting_enabled = true;
            render_view.lighting_cache = NULL;
            TraverseSceneObjects(scene, 0);
            DrawSceneGeometry();
        }
    }
}

uint32 UpdateCameraAim(void)
{
    uint32 x;
    uint32 y;
    uint32 z;
    uint32 base;
    uint32 sum;
    uint32 length;
    uint32 mode;
    uint32 angle;
    uint32 result;
    sint32 component;
    FUNCTION_MARKER(0x80022D38, "SLUS_005.78");
    x = camera_motion.focus_position[0];
    base = camera_motion.position[0];
    x -= base;
    y = camera_motion.focus_position[1];
    base = camera_motion.position[1];
    y -= base;
    z = camera_motion.focus_position[2];
    base = camera_motion.position[2];
    z -= base;
    mode = GetCutsceneMode();
    component = (sint32)x >> 6;
    sum = (uint32)((sint64)component * component);
    component = (sint32)y >> 6;
    sum += (uint32)((sint64)component * component);
    component = (sint32)z >> 6;
    sum += (uint32)((sint64)component * component);
    if (mode != 0)
        length = SquareRootQ12Exact(sum);
    else if ((sint32)sum >= 0)
        length = SquareRoot12((sint32)sum);
    else
        length = SquareRoot12((sint32)(sum >> 2)) << 1;
    angle = Atan2Angle((sint32)z, (sint32)x);
    if ((sint16)angle != 4096)
        camera_projection.rotation[1] = (uint16)angle;
    angle = (uint32)AsinAngle((sint32)y, (sint32)length);
    result = 0 - angle;
    if ((sint16)angle != 4096)
    {
        result &= 0xFFF;
        camera_projection.rotation[0] = (uint16)result;
    }
    return result;
}

uint32 IsSphereVisible(const uint32 *position, uint32 radius)
{
    sint32 x;
    sint32 y;
    sint32 z;
    sint32 coefficient;
    uint32 product_x;
    uint32 product_y;
    uint32 product_z;
    uint32 sum;
    uint32 bias;
    uint32 distance;
    uint32 limit = 0 - radius;
    uint32 index;
    const FrustumPlane *plane;
    const uint32 plane_order[4] = {1, 3, 2, 4};
    FUNCTION_MARKER(0x80011338, "SLUS_005.78");
    sum = position[0];
    coefficient = render_view.planes[0].normal.vx;
    x = (sint32)sum >> 6;
    product_x = (uint32)((sint64)x * coefficient);
    sum = position[1];
    coefficient = render_view.planes[0].normal.vy;
    y = (sint32)sum >> 6;
    product_y = (uint32)((sint64)y * coefficient);
    sum = position[2];
    coefficient = render_view.planes[0].normal.vz;
    z = (sint32)sum >> 6;
    product_z = (uint32)((sint64)z * coefficient);
    bias = render_view.planes[0].bias;
    sum = product_x + product_y + product_z;
    distance = (uint32)((sint32)sum >> 6) + bias;
    if ((sint32)limit >= (sint32)distance)
        return 0;
    sum = render_view.far_distance - distance;
    if ((sint32)limit >= (sint32)sum)
        return 0;
    for (index = 0; index < 4; ++index)
    {
        plane = &render_view.planes[plane_order[index]];
        coefficient = plane->normal.vx;
        product_x = (uint32)((sint64)x * coefficient);
        coefficient = plane->normal.vy;
        product_y = (uint32)((sint64)y * coefficient);
        coefficient = plane->normal.vz;
        product_z = (uint32)((sint64)z * coefficient);
        bias = plane->bias;
        sum = product_x + product_y + product_z;
        distance = (uint32)((sint32)sum >> 6) + bias;
        if ((sint32)limit >= (sint32)distance)
            return 0;
    }
    return 1;
}

uint32 ResetSceneRendering(void)
{
    FUNCTION_MARKER(0x80060EEC, "SLUS_005.78");
    ResetMaterialRegistry();
    InitializeSpritePool();
    {
        ObjectCallback operation = {NoOp80061174};
        object_operations.behaviors[0] = operation;
    }
    {
        ObjectCallback operation = {0, &AnimatedObject::UpdateSceneAnimation};
        object_operations.behaviors[3] = operation;
    }
    {
        ObjectCallback operation = {0, &AnimatedObject::UpdateAnimatedObjectCommands};
        object_operations.behaviors[4] = operation;
    }
    {
        ObjectCallback operation = {0, &AnimatedObject::FollowSceneAnimation};
        object_operations.behaviors[31] = operation;
    }
    {
        ObjectCallback operation = {0, &AnimatedObject::UpdateSceneScriptVariable};
        object_operations.behaviors[32] = operation;
    }
    camera_projection.view_normals[0].vz = 0;
    return 1;
}

uint32 UpdateCameraPosition(void)
{
    FUNCTION_MARKER(0x80024890, "SLUS_005.78");
    uint32 difference[3];
    uint32 count, value, factor, divisor, result;
    value = camera_motion.desired_position[0] - camera_motion.position[0];
    difference[0] = value;
    value = camera_motion.desired_position[1] - camera_motion.position[1];
    difference[1] = value;
    value = camera_motion.desired_position[2] - camera_motion.position[2];
    difference[2] = value;
    count = camera_motion.smooth_ticks[0];
    if (count != 0)
    {
        count = (count - 1) & 0xFF;
        camera_motion.smooth_ticks[0] = (uint8)(count);
        if (count < 45)
        {
            count = camera_motion.smooth_ticks[0];
            factor = camera_transition_factors[count];
        }
        else
        {
            count = camera_motion.smooth_ticks[0];
            divisor = (count + 2) << 12;
            factor = 0x1000000 / divisor;
        }
        value = difference[0];
        difference[0] = (uint32)(((sint64)(sint32)value * (sint32)factor) >> 12);
    }
    count = camera_motion.smooth_ticks[1];
    if (count != 0)
    {
        count = (count - 1) & 0xFF;
        camera_motion.smooth_ticks[1] = (uint8)(count);
        if (count < 45)
        {
            count = camera_motion.smooth_ticks[1];
            factor = camera_transition_factors[count];
        }
        else
        {
            /* Each axis derives its own long transition coefficient */
            count = camera_motion.smooth_ticks[1];
            divisor = (count + 2) << 12;
            factor = 0x1000000 / divisor;
        }
        value = difference[1];
        difference[1] = (uint32)(((sint64)(sint32)value * (sint32)factor) >> 12);
    }
    count = camera_motion.smooth_ticks[2];
    if (count != 0)
    {
        count = (count - 1) & 0xFF;
        camera_motion.smooth_ticks[2] = (uint8)(count);
        if (count < 45)
        {
            count = camera_motion.smooth_ticks[2];
            factor = camera_transition_factors[count];
        }
        else
        {
            count = camera_motion.smooth_ticks[2];
            divisor = (count + 2) << 12;
            factor = 0x1000000 / divisor;
        }
        value = difference[2];
        difference[2] = (uint32)(((sint64)(sint32)value * (sint32)factor) >> 12);
    }
    value = difference[0];
    camera_motion.position[0] = camera_motion.position[0] + value;
    value = camera_motion.position[1];
    value += difference[1];
    camera_motion.position[1] = value;
    value = camera_motion.position[2];
    value += difference[2];
    camera_motion.position[2] = value;
    result = camera_motion.flags & 0xFE;
    camera_motion.flags = (uint8)result;
    return result;
}

uint32 SetCameraTransition(const uint32 *delta)
{
    FUNCTION_MARKER(0x80024B3C, "SLUS_005.78");
    uint32 axis, value, state;
    sint32 count;
    for (axis = 0; axis < 3; ++axis)
    {
        value = delta[axis];
        if ((sint32)value < 0)
            value = 0 - value;
        count = (sint32)value >> 12;
        if (count != 0)
        {
            count = (count >> 1) + 1;
            if (count >= 46)
                count = 45;
        }
        else
        {
            state = OriginalDefaults::SceneContextIdentity;
            if (camera_motion.smooth_ticks[axis] != 0)
                count = 1;
        }
        state = OriginalDefaults::SceneContextIdentity;
        camera_motion.smooth_ticks[axis] = (uint8)count;
    }
    return state;
}

uint32 UpdateCameraTarget(void)
{
    FUNCTION_MARKER(0x80024C44, "SLUS_005.78");
    uint32 x, y, z, count, factor, result;
    x = camera_motion.target[0];
    x -= camera_motion.focus_position[0];
    y = camera_motion.target[1];
    y -= camera_motion.focus_position[1];
    z = camera_motion.target[2];
    z -= camera_motion.focus_position[2];
    count = camera_motion.target_ticks;
    if (count)
    {
        --count;
        factor = camera_transition_factors[count];
        camera_motion.target_ticks = count;
        x = (uint32)(((sint64)(sint32)x * (sint32)factor) >> 12);
        y = (uint32)(((sint64)(sint32)y * (sint32)factor) >> 12);
        z = (uint32)(((sint64)(sint32)z * (sint32)factor) >> 12);
    }
    camera_motion.focus_position[0] = camera_motion.focus_position[0] + x;
    camera_motion.focus_position[1] = camera_motion.focus_position[1] + y;
    camera_motion.focus_position[2] = camera_motion.focus_position[2] + z;
    if (!camera_motion.target_ticks)
    {
        HideAlternateCharacterModel();
        game_flow.transition_flags = game_flow.transition_flags & 0xFFFFFEFF;
    }
    result = camera_motion.flags & 0xFE;
    camera_motion.flags = (uint8)result;
    return result;
}

uint32 AnimatedObject::UpdateCharacterCameraPath()
{
    const PathDefinition *parent;
    FUNCTION_MARKER(0x800457C8, "SLUS_005.78");

    union
    {
        VECTOR vector;
        uint32 words[4];
    } delta, position;

    uint32 scene, direction, flags, x, y, z, result;
    const PathTransform *path_transform;
    const ScenePath *current;
    uint32 *destination;
    sint32 frame;
    scene = OriginalDefaults::SceneContextIdentity;
    flags = this->flags;
    direction = flags & 0x10;
    camera_motion.flags = camera_motion.flags & 0xFE;
    if (GetCutsceneMode() != 0)
    {
        if (UpdateCutscene() == 0)
        {
            character_controller.path_velocity.vz = 0;
            character_controller.path_velocity.vy = 0;
            character_controller.path_velocity.vx = 0;
            return 1;
        }
        character_controller.motion.position_changed = 1;
    }
    flags = this->path_flags;
    if (((flags & 0x100) != 0 && (sint8)path_transition_rules.camera_duration != 0) || (flags & 0x40000) != 0)
    {
        scene = OriginalDefaults::SceneContextIdentity;
        flags = character_controller.flags;
        y = camera_motion.focus_position[1];
        camera_motion.focus_position[1] = y - ((sint32)flags >= 0 ? 14336 : 32768);
        UpdateCharacterCameraTarget(position.words, this);
        scene = OriginalDefaults::SceneContextIdentity;
        x = position.words[0];
        y = position.words[1];
        z = position.words[2];
        camera_motion.focus_position[0] = x;
        camera_motion.focus_position[1] = y;
        camera_motion.focus_position[2] = z;
        goto select_position;
    }
    if ((this->flags & 0x2000) != 0)
        goto local_position;
    parent = (this->path_definition);
    if (parent == 0 || ((((const PathDefinition *)(parent)))->transform) == 0)
        goto local_position;
    x = this->position[0];
    y = this->position[1];
    z = this->position[2];
    position.words[0] = x;
    position.words[1] = y;
    position.words[2] = z;
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
    scene = OriginalDefaults::SceneContextIdentity;
    ApplyRotMatrixLV(&position.vector, (VECTOR *)camera_motion.focus_position);
    PopMatrix();
    scene = OriginalDefaults::SceneContextIdentity;
    path_transform = (const PathTransform *)((((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) != 0 ? (((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) : (PathTransform *)0);
    x = camera_motion.focus_position[0];
    camera_motion.focus_position[0] = x + (uint32)path_transform->matrix.t[0];
    path_transform = (const PathTransform *)((((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) != 0 ? (((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) : (PathTransform *)0);
    y = camera_motion.focus_position[1];
    camera_motion.focus_position[1] = y + (uint32)path_transform->matrix.t[1];
    path_transform = (const PathTransform *)((((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) != 0 ? (((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) : (PathTransform *)0);
    z = camera_motion.focus_position[2];
    camera_motion.focus_position[2] = z + (uint32)path_transform->matrix.t[2];
    goto select_position;
local_position:
    scene = OriginalDefaults::SceneContextIdentity;
    x = this->position[0];
    y = this->position[1];
    z = this->position[2];
    camera_motion.focus_position[0] = x;
    camera_motion.focus_position[1] = y;
    camera_motion.focus_position[2] = z;
select_position:
    scene = OriginalDefaults::SceneContextIdentity;
    destination = (camera_motion.smooth_ticks[0] | camera_motion.smooth_ticks[1] | camera_motion.smooth_ticks[2]) != 0 ? camera_motion.desired_position : camera_motion.position;
    if ((sint16)character_controller.motion.position_changed != 0)
        goto update_path;
    scene = OriginalDefaults::SceneContextIdentity;
    if (destination == camera_motion.desired_position)
        goto update_path;
    current = (const ScenePath *)(camera_motion.path);
    if (current->offset_matrix != 0)
        goto update_path;
    if ((current->node.state_flags & 1) != 0 && (this->flags & 0x400) != 0)
        goto update_path;
    if ((this->path_flags & 0x212) == 0)
        goto compute_delta;
update_path:
    scene = OriginalDefaults::SceneContextIdentity;
    current = (const ScenePath *)(camera_motion.path);
    frame = camera_motion.frame;
    current->ProjectOffset(frame, destination, direction);
compute_delta:
    if ((this->flags & 0x2000) != 0)
    {
        flags = character_controller.support.speed[0];
        x = this->velocity[0];
        y = this->velocity[1];
        z = this->velocity[2];
        delta.words[0] = x;
        delta.words[1] = y;
        delta.words[2] = z;
        if ((sint32)flags < 0)
        {
            x = delta.words[0];
            y = delta.words[1];
            z = delta.words[2];
            delta.words[0] = 0 - x;
            delta.words[1] = 0 - y;
            delta.words[2] = 0 - z;
        }
    }
    else
    {
        scene = OriginalDefaults::SceneContextIdentity;
        current = (const ScenePath *)(camera_motion.path);
        frame = camera_motion.frame;
        current->SamplePosition(frame, position.words, direction);
        scene = OriginalDefaults::SceneContextIdentity;
        frame = camera_motion.frame;
        if (frame == 0)
        {
            x = position.words[0];
            y = position.words[1];
            z = position.words[2];
            delta.words[0] = x;
            delta.words[1] = y;
            delta.words[2] = z;
            frame = camera_motion.frame;
            current = (const ScenePath *)(camera_motion.path);
            current->SamplePosition(frame + 1, position.words, direction);
        }
        else
        {
            current = (const ScenePath *)(camera_motion.path);
            current->SamplePosition(frame - 1, delta.words, direction);
        }
        x = position.words[0] - delta.words[0];
        y = position.words[1] - delta.words[1];
        z = position.words[2] - delta.words[2];
        delta.words[0] = x;
        delta.words[1] = y;
        delta.words[2] = z;
    }
    if ((character_controller.flags & 0x180000) != 0)
    {
        x = (uint16)this->yaw;
        flags = this->flags;
        x = (x + 1024) & 0xFFF;
        this->yaw = x;
        this->orientation[2] = (x + 2048) & 0xFFF;
        this->flags = flags | 1;
    }
    else if ((this->path_flags & 1) != 0)
        this->orientation[2] = ((uint16)this->yaw + 2048) & 0xFFF;
    else
    {
        x = delta.words[0];
        y = delta.words[1];
        z = delta.words[2];
        character_controller.path_velocity.vx = 0 - x;
        character_controller.path_velocity.vy = 0 - y;
        character_controller.path_velocity.vz = 0 - z;
        this->TurnObjectFacing(delta.words, (sint16)character_controller.turn_step);
    }
    if ((this->path_flags & 0x100) != 0)
        NoOp80059AE4();
    scene = OriginalDefaults::SceneContextIdentity;
    flags = character_controller.flags;
    y = camera_motion.focus_position[1];
    result = y + ((sint32)flags >= 0 ? 14336 : 32768);
    camera_motion.focus_position[1] = result;
    return result;
}

uint32 AnimatedObject::FinishCharacterCameraTransition()
{
    const PathDefinition *parent;
    FUNCTION_MARKER(0x80045D0C, "SLUS_005.78");

    union
    {
        VECTOR value;
        uint32 words[4];
    } vector;

    uint32 x, y, z, result;
    const PathTransform *path_transform;
    x = camera_motion.desired_position[0];
    y = camera_motion.desired_position[1];
    z = camera_motion.desired_position[2];
    camera_motion.position[0] = x;
    camera_motion.position[1] = y;
    camera_motion.position[2] = z;
    parent = (this->path_definition);
    if (parent != 0 && ((((const PathDefinition *)(parent)))->transform) != 0)
    {
        x = this->position[0];
        y = this->position[1];
        z = this->position[2];
        vector.words[0] = x;
        vector.words[1] = y;
        vector.words[2] = z;
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
        ApplyRotMatrixLV(&vector.value, (VECTOR *)camera_motion.focus_position);
        PopMatrix();
        path_transform = (const PathTransform *)((((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) != 0 ? (((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) : (PathTransform *)0);
        camera_motion.focus_position[0] = camera_motion.focus_position[0] + (uint32)path_transform->matrix.t[0];
        path_transform = (const PathTransform *)((((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) != 0 ? (((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) : (PathTransform *)0);
        camera_motion.focus_position[1] = camera_motion.focus_position[1] + (uint32)path_transform->matrix.t[1];
        path_transform = (const PathTransform *)((((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) != 0 ? (((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) : (PathTransform *)0);
        camera_motion.focus_position[2] = camera_motion.focus_position[2] + (uint32)path_transform->matrix.t[2];
    }
    else
    {
        x = this->position[0];
        y = this->position[1];
        z = this->position[2];
        camera_motion.focus_position[0] = x;
        camera_motion.focus_position[1] = y;
        camera_motion.focus_position[2] = z;
    }
    result = character_controller.flags;
    y = camera_motion.focus_position[1];
    camera_motion.focus_position[1] = y + ((sint32)result < 0 ? 0x8000 : 0x3800);
    character_controller.motion.jump_phase = 0;
    character_controller.motion.vertical_speed = 0;
    character_controller.motion.horizontal_speed = 0;
    this->velocity[2] = 0;
    this->velocity[1] = 0;
    this->velocity[0] = 0;
    this->movement_mode = 1;
    result = character_controller.flags & 0xF7FFFFFF;
    character_controller.flags = result;
    return result;
}

uint32 AnimatedObject::UpdateCharacterCameraTransition()
{
    const PathDefinition *parent;
    FUNCTION_MARKER(0x80045ED8, "SLUS_005.78");

    union
    {
        VECTOR value;
        uint32 words[4];
    } vector;

    uint32 value, index, result;
    const PathTransform *path_transform;
    uint32 x, y, z;
    sint32 divisor, coefficient;
    ((const ScenePath *)(camera_motion.path))->SampleOffset(camera_motion.frame, (uint32 *)camera_motion.desired_position, this->flags & 0x10);
    value = character_controller.timers.transition_ticks - 1;
    character_controller.timers.transition_ticks = value;
    if ((value & 0xFFFF) == 0)
        this->FinishCharacterCameraTransition();
    else
    {
        parent = (this->path_definition);
        if (parent != 0 && ((((const PathDefinition *)(parent)))->transform) != 0)
        {
            x = this->position[0];
            y = this->position[1];
            z = this->position[2];
            vector.words[0] = x;
            vector.words[1] = y;
            vector.words[2] = z;
            PushMatrix();
            path_transform = (const PathTransform *)((((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) != 0 ? (((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) : (PathTransform *)0);
            for (index = 0; index < 5; ++index)
                xport_gte_write_control(index, xport_load_le32((const uint8 *)&path_transform->matrix + index * 4));
            ApplyRotMatrixLV(&vector.value, (VECTOR *)this->velocity);
            PopMatrix();
            for (index = 0; index < 3; ++index)
            {
                path_transform = (const PathTransform *)((((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) != 0 ? (((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) : (PathTransform *)0);
                value = this->velocity[index] + (uint32)path_transform->matrix.t[index];
                this->velocity[index] = value;
            }
        }
        else
        {
            x = this->position[0];
            y = this->position[1];
            z = this->position[2];
            this->velocity[0] = x;
            this->velocity[1] = y;
            this->velocity[2] = z;
        }
        value = this->velocity[1];
        value += (sint32)character_controller.flags < 0 ? 0x8000 : 14336;
        this->velocity[1] = value;
        for (index = 0; index < 3; ++index)
        {
            value = this->velocity[index] - camera_motion.focus_position[index];
            this->velocity[index] = value;
        }
        for (index = 0; index < 3; ++index)
        {
            divisor = (sint16)character_controller.timers.transition_ticks * 4096;
            if (divisor == 0)
            {
                trap(7);
                abort();
            }
            coefficient = 0x1000000 / divisor;
            value = (uint32)(((sint64)(sint32)this->velocity[index] * coefficient) >> 12);
            this->velocity[index] = value;
        }
        for (index = 0; index < 3; ++index)
        {
            value = camera_motion.focus_position[index] + this->velocity[index];
            camera_motion.focus_position[index] = value;
        }
    }
    result = camera_motion.flags & 0xFE;
    camera_motion.flags = result;
    return result;
}

void AnimatedObject::BeginCharacterCameraTransition(const ScenePath *input_path, uint32 frame_word, uint32 duration_ticks, uint32 kind)
{
    FUNCTION_MARKER(0x80046628, "SLUS_005.78");

    union
    {
        VECTOR value;
        uint32 words[4];
    } initial, target;

    const PathDefinition *parent, *definition;
    uint32 x, y, z, value, direction, frame;
    const PathTransform *path_transform;
    const ScenePath *path, *path_view;
    uint32 flags, duration, denominator;
    sint32 coefficient, sample_frame, reciprocal;
    sint64 product;
    if (kind == 87)
    {
        screen_render.brighten_duration = 30;
        screen_render.brighten_ticks = 15;
    }
    else if (kind == 66)
    {
        screen_render.darken_duration = 30;
        screen_render.darken_ticks = 15;
    }
    value = player_interaction.flags;
    if (value != 0)
        player_interaction.flags = value | 0x40;
    character_controller.ClearTransientFlags();
    if (character_action_flags != 0)
    {
        character_controller.interaction_phase = 0;
        character_controller.ClearInteractionActions();
    }
    if ((this->flags & 0x2000) == 0 && (parent = (this->path_definition)) != 0 && ((((const PathDefinition *)(parent)))->transform) != 0)
    {
        x = this->position[0];
        y = this->position[1];
        z = this->position[2];
        initial.words[0] = x;
        initial.words[1] = y;
        initial.words[2] = z;
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
        ApplyRotMatrixLV(&initial.value, (VECTOR *)camera_motion.focus_position);
        PopMatrix();
        path_transform = (const PathTransform *)((((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) != 0 ? (((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) : (PathTransform *)0);
        camera_motion.focus_position[0] = camera_motion.focus_position[0] + (uint32)path_transform->matrix.t[0];
        path_transform = (const PathTransform *)((((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) != 0 ? (((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) : (PathTransform *)0);
        camera_motion.focus_position[1] = camera_motion.focus_position[1] + (uint32)path_transform->matrix.t[1];
        path_transform = (const PathTransform *)((((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) != 0 ? (((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->transform) : (PathTransform *)0);
        camera_motion.focus_position[2] = camera_motion.focus_position[2] + (uint32)path_transform->matrix.t[2];
    }
    else
    {
        x = this->position[0];
        y = this->position[1];
        z = this->position[2];
        camera_motion.focus_position[0] = x;
        camera_motion.focus_position[1] = y;
        camera_motion.focus_position[2] = z;
    }
    this->flags = this->flags & 0xEFFF93EF;
    x = character_controller.motion.vertical_speed;
    y = (uint32)character_controller.movement.fall_speed_limit;
    if ((sint32)y >= (sint32)x)
        character_controller.timers.state_ticks = 0;
    path = input_path;
    path_view = (const ScenePath *)(path);
    frame = frame_word;
    duration = duration_ticks;
    flags = path_view->node.state_flags;
    direction = flags & 0x10;
    if ((sint32)frame < 0)
    {
        frame = ~frame;
        if ((flags & 8) != 0)
        {
            path = (path_view->adjacent[0]);
            path_view = (const ScenePath *)(path);
        }
        direction ^= 0x10;
    }
    this->flags = this->flags | 0x20000 | direction;
    if ((((const PathDefinition *)((path_view->definition)))->flags & 0x11) != 0)
    {
        x = character_controller.movement.jump_ticks;
        flags = character_controller.flags;
        character_controller.motion.jump_phase = -1;
        character_controller.motion.vertical_speed = 0;
        character_controller.timers.state_ticks = 0;
        character_controller.timers.jump_duration_ticks = x;
        character_controller.flags = flags | 0x08000000;
        if ((character_controller.secondary_flags & 0x80) == 0)
            character_controller.motion.state_code = 6;
        this->flags = (this->flags | 0x400) & 0xF7FFFFFF;
    }
    this->path_definition = (path_view->definition);
    coefficient = (sint16)((const PathDefinition *)((path_view->definition)))->metadata36;
    if (coefficient != 0)
        frame = (uint32)((sint32)((uint32)coefficient * frame) >> 12);
    camera_motion.frame = frame;
    this->path_frame = frame;
    this->yaw = 4096;
    character_controller.flags = character_controller.flags & 0xFCC5F459;
    SelectScenePath(path);
    path_view->SampleOffset((sint32)frame, (uint32 *)camera_motion.desired_position, direction);
    definition = (camera_motion.path->definition);
    if (direction != 0)
        sample_frame = camera_motion.frame;
    else
        sample_frame = (sint32)((uint32)(sint32)(sint16)((const PathDefinition *)(definition))->positions.metadata6 - (uint32)(sint32)camera_motion.frame - 1);
    SampleVectorTrack(sample_frame, &((const PathDefinition *)(definition))->positions, this->position);
    this->path_frame = (uint16)camera_motion.frame;
    parent = (this->path_definition);
    if (parent != 0 && ((((const PathDefinition *)(parent)))->transform) != 0)
    {
        x = this->position[0];
        y = this->position[1];
        z = this->position[2];
        target.words[0] = x;
        target.words[1] = y;
        target.words[2] = z;
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
        ApplyRotMatrixLV(&target.value, (VECTOR *)this->velocity);
        PopMatrix();
        parent = (this->path_definition);
        path_transform = (const PathTransform *)(((((const PathDefinition *)(parent)))->transform) != 0 ? ((((const PathDefinition *)(parent)))->transform) : (PathTransform *)0);
        x = this->velocity[0];
        x += (uint32)path_transform->matrix.t[0];
        parent = (this->path_definition);
        this->velocity[0] = x;
        path_transform = (const PathTransform *)(((((const PathDefinition *)(parent)))->transform) != 0 ? ((((const PathDefinition *)(parent)))->transform) : (PathTransform *)0);
        y = this->velocity[1];
        y += (uint32)path_transform->matrix.t[1];
        parent = (this->path_definition);
        this->velocity[1] = y;
        path_transform = (const PathTransform *)(((((const PathDefinition *)(parent)))->transform) != 0 ? ((((const PathDefinition *)(parent)))->transform) : (PathTransform *)0);
        this->velocity[2] = this->velocity[2] + (uint32)path_transform->matrix.t[2];
    }
    else
    {
        x = this->position[0];
        y = this->position[1];
        z = this->position[2];
        this->velocity[0] = x;
        this->velocity[1] = y;
        this->velocity[2] = z;
    }
    this->velocity[0] = this->velocity[0] - camera_motion.focus_position[0];
    this->velocity[1] = this->velocity[1] - camera_motion.focus_position[1];
    this->velocity[2] = this->velocity[2] - camera_motion.focus_position[2];
    camera_motion.smooth_ticks[2] = (uint8)(duration);
    camera_motion.smooth_ticks[1] = (uint8)(duration);
    camera_motion.smooth_ticks[0] = (uint8)(duration);
    if (duration == 0)
    {
        x = this->velocity[0] << 1;
        y = this->velocity[1] << 1;
        z = this->velocity[2] << 1;
        this->world_position[0] = x;
        this->world_position[1] = y;
        this->world_position[2] = z;
        this->FinishCharacterCameraTransition();
        camera_motion.flags = camera_motion.flags & 0xFE;
    }
    else
    {
        character_controller.timers.transition_ticks = duration;
        x = this->velocity[0];
        denominator = duration << 12;
        if (denominator == 0)
            xport_mips_break(7);
        reciprocal = 0x01000000 / (sint32)denominator;
        product = (sint64)(sint32)x * reciprocal;
        x = (uint32)((uint64)product >> 12);
        y = this->velocity[1];
        this->velocity[0] = x;
        product = (sint64)(sint32)y * reciprocal;
        y = (uint32)((uint64)product >> 12);
        z = this->velocity[2];
        this->velocity[1] = y;
        product = (sint64)(sint32)z * reciprocal;
        this->velocity[2] = (uint32)((uint64)product >> 12);
        flags = character_controller.flags;
        y = camera_motion.focus_position[1];
        camera_motion.focus_position[1] = y + ((sint32)flags < 0 ? 0x8000 : 0x3800);
        this->node.flags = this->node.flags | 1;
        this->movement_mode = 0;
        character_controller.flags = character_controller.flags | 0x04000000;
    }
    ((const PathDefinition *)((const PathDefinition *)((this->path_definition))))->SampleTangent(&character_controller.path_tangent, (sint32)direction);
    return;
}

void NoOp80059AE4()
{
    FUNCTION_MARKER(0x80059AE4, "SLUS_005.78");
}

void NoOp80061174(void)
{
    FUNCTION_MARKER(0x80061174, "SLUS_005.78");
}
