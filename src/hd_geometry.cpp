#include "hd_internal.h"

void hd_lighting(const SceneLighting *source, const MATRIXF *model, const VECTORF *origin, HdLighting *lighting)
{
    memset(lighting, 0, sizeof(*lighting));
    lighting->ambient.vx = source->ambient[0] / 256.0f;
    lighting->ambient.vy = source->ambient[1] / 256.0f;
    lighting->ambient.vz = source->ambient[2] / 256.0f;
    if (source->skip_transform != 0)
        return;
    MATRIXF inverse;
    TransposeMatrixf(model, &inverse);
    uint32 row = 0;
    const SceneLight *light = source->lights;
    while (light != NULL && (light->event_flags & 2) == 0 && row < 3)
    {
        if ((light->flags & 1) == 0)
        {
            HdLight *output = lighting->lights + row++;
            VECTORF direction = {light->src.dir.vx / 4096.0f, light->src.dir.vy / 4096.0f, light->src.dir.vz / 4096.0f};
            ApplyMatrixf(&inverse, &direction, &output->dir);
            output->color.vx = (sint16)light->color[0] / 4096.0f;
            output->color.vy = (sint16)light->color[1] / 4096.0f;
            output->color.vz = (sint16)light->color[2] / 4096.0f;
        }
        light = (const SceneLight *)light->links.registry_next;
    }
    for (uint32 idx = 0; row < 3 && source->point_lights[idx] != NULL; ++idx)
    {
        light = source->point_lights[idx];
        HdLight *output = lighting->lights + row++;
        VECTORF position = {light->src.pos.x / 32.0f, light->src.pos.y / 32.0f, light->src.pos.z / 32.0f};
        VECTORF relative = {position.vx - model->t[0], position.vy - model->t[1], position.vz - model->t[2]};
        ApplyMatrixf(&inverse, &relative, &output->pos);
        if (origin != NULL)
        {
            relative.vx = position.vx - origin->vx;
            relative.vy = position.vy - origin->vy;
            relative.vz = position.vz - origin->vz;
        }
        ApplyMatrixf(&inverse, &relative, &relative);
        NormalizeVectorf(&relative, &output->dir);
        output->point = true;
        output->color.vx = (sint16)light->color[0] / 4096.0f;
        output->color.vy = (sint16)light->color[1] / 4096.0f;
        output->color.vz = (sint16)light->color[2] / 4096.0f;
        output->color_scale.vx = (sint32)light->color_scale[0] / 16384.0f;
        output->color_scale.vy = (sint32)light->color_scale[1] / 16384.0f;
        output->color_scale.vz = (sint32)light->color_scale[2] / 16384.0f;
        output->radius = light->radius;
        output->attenuation_range = light->attenuation_range;
    }
}

static void hd_og_geometry_release(HdOgGeometry *geometry)
{
    if (geometry != NULL && --geometry->refs == 0)
        free(geometry);
}

static bool hd_og_geometry_equal(const HdOgGeometry *geometry, const HdMesh *mesh)
{
    if (geometry == NULL || geometry->vertex_count != mesh->vertex_count || geometry->face_count != mesh->face_count ||
        memcmp(geometry->vertices, mesh->vertices, mesh->vertex_count * sizeof(MeshVertex)) != 0)
        return false;
    for (uint32 idx = 0; idx < mesh->face_count; ++idx)
    {
        const MeshFace *copy = geometry->faces + idx, *face = mesh->faces + idx;
        if (copy->plane != face->plane || memcmp(copy->vertex_idxs, face->vertex_idxs, sizeof(face->vertex_idxs)) != 0 ||
            memcmp(&copy->n, &face->n, sizeof(MeshFaceNormal)) != 0)
            return false;
    }
    return true;
}

static bool hd_og_mesh_equal(const HdMesh *mesh)
{
    if (!hd_og_geometry_equal(mesh->og_geometry, mesh) || mesh->og_materials == NULL ||
        !hd_og_material_equal(&mesh->og_override, mesh->material))
        return false;
    for (uint32 idx = 0; idx < mesh->face_count; ++idx)
        if (!hd_og_material_equal(mesh->og_materials + idx, mesh->faces[idx].tex))
            return false;
    return true;
}

static bool hd_og_mesh_capture(HdMesh *mesh)
{
    if (hd_og_mesh_equal(mesh))
        return true;
    HdOgGeometry *geometry = mesh->og_geometry;
    if (!hd_og_geometry_equal(geometry, mesh))
    {
        geometry = NULL;
        for (HdMesh *other = hd_state.meshes; other != NULL; other = other->next)
            if (other != mesh && other->vertices == mesh->vertices && other->faces == mesh->faces &&
                other->first_vertex == mesh->first_vertex && hd_og_geometry_equal(other->og_geometry, mesh))
            {
                geometry = other->og_geometry;
                ++geometry->refs;
                break;
            }
        if (geometry == NULL)
        {
            uint64 bytes = sizeof(HdOgGeometry) + (uint64)mesh->vertex_count * sizeof(MeshVertex) + (uint64)mesh->face_count * sizeof(MeshFace);
            if (bytes > (size_t)-1)
                return false;
            geometry = (HdOgGeometry *)malloc((size_t)bytes);
            if (geometry == NULL)
                return false;
            geometry->refs = 1;
            geometry->vertex_count = mesh->vertex_count;
            geometry->face_count = mesh->face_count;
            geometry->vertices = (MeshVertex *)(geometry + 1);
            geometry->faces = (MeshFace *)(geometry->vertices + mesh->vertex_count);
            memcpy(geometry->vertices, mesh->vertices, mesh->vertex_count * sizeof(MeshVertex));
            for (uint32 idx = 0; idx < mesh->face_count; ++idx)
            {
                geometry->faces[idx] = mesh->faces[idx];
                geometry->faces[idx].tex = NULL;
            }
        }
    }
    HdOgMaterial *materials = mesh->og_materials;
    if (materials == NULL)
        materials = (HdOgMaterial *)calloc(mesh->face_count, sizeof(HdOgMaterial));
    if (materials == NULL)
    {
        if (geometry != mesh->og_geometry)
            hd_og_geometry_release(geometry);
        return false;
    }
    if (geometry != mesh->og_geometry)
        hd_og_geometry_release(mesh->og_geometry);
    mesh->og_geometry = geometry;
    mesh->og_materials = materials;
    for (uint32 idx = 0; idx < mesh->face_count; ++idx)
        if (!hd_og_material_equal(materials + idx, mesh->faces[idx].tex))
            hd_og_material_copy(materials + idx, mesh->faces[idx].tex);
    if (!hd_og_material_equal(&mesh->og_override, mesh->material))
        hd_og_material_copy(&mesh->og_override, mesh->material);
    return true;
}

static void hd_clear_morph(HdMesh *mesh)
{
    for (uint32 idx = 0; idx < 4; ++idx)
    {
        vid_mat_free(mesh->morph_materials[idx]);
        mesh->morph_materials[idx] = NULL;
    }
    vid_tex_free(mesh->morph_texture);
    mesh->morph_texture = NULL;
    mesh->morph_width = 0;
}

void hd_mesh_free(HdMesh *mesh)
{
    if (mesh->pool_range != NULL)
        --mesh->pool_range->refs;
    if (mesh->pooled)
        hd_state.world_pool_dirty = true;
    hd_clear_morph(mesh);
    vid_mesh_free(mesh->geometry);
    vid_index_free(mesh->indices);
    if (mesh->opaque_indices != NULL)
        vid_index_free(mesh->opaque_indices);
    free(mesh->passes);
    free(mesh->staging_passes);
    free(mesh->source_indices);
    free(mesh->scrolls);
    free(mesh->regions);
    hd_og_geometry_release(mesh->og_geometry);
    free(mesh->og_materials);
    free(mesh);
}

void hd_free_retired(void)
{
    HdMesh **link = &hd_state.retired_meshes;
    while (*link != NULL)
    {
        HdMesh *mesh = *link;
        if (!mesh->queued && mesh->sample_refs == 0)
        {
            *link = mesh->next;
            hd_mesh_free(mesh);
        }
        else
            link = &mesh->next;
    }
}

static void hd_mesh_pass(HdMesh *mesh, uint32 first, uint32 blend, uint32 mask, float alpha)
{
    HdMeshPass *pass;
    if (mesh->pass_count != 0)
    {
        pass = mesh->passes + mesh->pass_count - 1;
        if (pass->first + pass->count == first && pass->blend == blend && pass->mask == mask && pass->alpha == alpha)
        {
            pass->count += 6;
            return;
        }
    }
    pass = mesh->passes + mesh->pass_count++;
    pass->first = first;
    pass->count = 6;
    pass->blend = blend;
    pass->mask = mask;
    pass->alpha = alpha;
}

static bool hd_mesh_opaque_indices(HdMesh *mesh)
{
    uint32 count = 0;
    for (uint32 idx = 0; idx < mesh->pass_count; ++idx)
        if (mesh->passes[idx].blend == VID_BLEND_NONE)
            count += mesh->passes[idx].count;
    VID_INDEX *indices = count != 0 ? vid_index_create(count) : NULL;
    if (count != 0 && indices == NULL)
        return false;
    if (indices != NULL)
    {
        uint32 *output = vid_index_data(indices);
        uint32 cursor = 0;
        static const uint8 order[6] = {0, 1, 2, 2, 1, 3};
        for (uint32 idx = 0; idx < mesh->pass_count; ++idx)
        {
            const HdMeshPass *pass = mesh->passes + idx;
            if (pass->blend != VID_BLEND_NONE)
                continue;
            for (uint32 index = pass->first; index < pass->first + pass->count; ++index)
                output[cursor++] = index / 6 * 4 + order[index % 6];
        }
        vid_index_commit(indices, count);
        vid_index_static(indices);
    }
    if (mesh->opaque_indices != NULL)
        vid_index_free(mesh->opaque_indices);
    mesh->opaque_indices = indices;
    mesh->opaque_count = count;
    return true;
}

static VID_VERTEX16 hd_pool_vertex(const VID_VERTEX16 *source)
{
    VID_VERTEX16 vertex = *source;
    memset(vertex.uv, 0, sizeof(vertex.uv));
    memset(vertex.color, 0, sizeof(vertex.color));
    vertex.tex[0] = vertex.tex[1] = vertex.tex[2] = 0;
    return vertex;
}

static bool hd_mesh_update_source(HdMesh *mesh, bool compare_only)
{
    if (mesh == NULL)
        return false;
    if (compare_only ? !hd_og_mesh_equal(mesh) : !hd_og_mesh_capture(mesh))
        return false;
    VID_VERTEX16 *output = vid_mesh_data_16(mesh->geometry);
    if (output == NULL)
        return false;
    HdMesh pending = *mesh;
    pending.passes = mesh->staging_passes;
    pending.pass_count = 0;
    bool changed = false;
    static const uint8 order[4] = {0, 1, 3, 2};
    static const uint32 blends[4] = {VID_BLEND_ALPHA, VID_BLEND_ADD, VID_BLEND_SUBTRACT, VID_BLEND_ADD};
    static const float alpha[4] = {0.5f, 1.0f, 1.0f, 0.25f};
    for (uint32 idx = 0; idx < mesh->face_count; ++idx)
    {
        const MeshFace *face = mesh->faces + idx;
        const TextureDescriptor *texture = mesh->material != NULL ? mesh->material : face->tex;
        if (texture == NULL)
            return false;
        for (uint32 corner = 0; corner < 4; ++corner)
            if (face->vertex_idxs[corner] < mesh->first_vertex || face->vertex_idxs[corner] - mesh->first_vertex >= mesh->vertex_count)
                return false;
        HdTexture mapping;
        memset(&mapping, 0, sizeof(mapping));
        uint32 flags = texture->flags;
        bool textured = (flags & 4) != 0;
        bool semi = (texture->mapping.color.code & 2) != 0;
        if (textured && !hd_texture(texture, &mapping))
            return false;
        HdTexRegion region = {mapping.left, mapping.top, mapping.width, mapping.height};
        if (memcmp(mesh->regions + idx, &region, sizeof(region)) != 0)
        {
            if (compare_only)
                return false;
            mesh->regions[idx] = region;
            changed = true;
        }
        HdScroll scroll = hd_scroll_source(texture);
        if (memcmp(mesh->scrolls + idx, &scroll, sizeof(scroll)) != 0)
        {
            if (compare_only)
                return false;
            mesh->scrolls[idx] = scroll;
        }
        const MeshVertex *first = mesh->vertices + (face->vertex_idxs[0] - mesh->first_vertex);
        for (uint32 corner = 0; corner < 4; ++corner)
        {
            uint32 source_index = face->vertex_idxs[order[corner]] - mesh->first_vertex;
            if (compare_only && mesh->source_indices[idx * 4 + corner] != source_index)
                return false;
            if (!compare_only)
                mesh->source_indices[idx * 4 + corner] = source_index;
            const MeshVertex *source = mesh->vertices + (face->vertex_idxs[order[corner]] - mesh->first_vertex);
            VID_VERTEX16 vertex;
            memset(&vertex, 0, sizeof(vertex));
            vertex.pos[0] = source->pos.vx;
            vertex.pos[1] = source->pos.vy;
            vertex.pos[2] = source->pos.vz;
            vertex.pos[3] = face->plane;
            vertex.uv[0] = mapping.uv[corner].u;
            vertex.uv[1] = mapping.uv[corner].v;
            vertex.color[0] = texture->mapping.color.r;
            vertex.color[1] = texture->mapping.color.g;
            vertex.color[2] = texture->mapping.color.b;
            vertex.color[3] = flags >> 24;
            vertex.n[0] = (flags & 2) != 0 ? source->n.vx : face->n.x;
            vertex.n[1] = (flags & 2) != 0 ? source->n.vy : face->n.y;
            vertex.n[2] = (flags & 2) != 0 ? source->n.vz : face->n.z;
            vertex.n[3] = face->plane >> 16;
            vertex.extra[0] = first->pos.vx;
            vertex.extra[1] = first->pos.vy;
            vertex.extra[2] = first->pos.vz;
            vertex.extra[3] = face->n.depth_bias >> 1;
            vertex.tex[0] = mapping.page;
            vertex.tex[1] = mapping.clut;
            vertex.tex[2] = textured ? HD_TEXTURED | mapping.flags : 0;
            if (textured && semi)
                vertex.tex[2] |= HD_OPAQUE_ONLY;
            if ((texture->mapping.color.code & 1) != 0)
                vertex.tex[2] |= HD_RAW_TEXTURE;
            if ((flags & 1) != 0)
                vertex.tex[2] |= HD_TWO_SIDED;
            vertex.tex[3] = face->n.x;
            vertex.rect[0] = face->n.y;
            vertex.rect[1] = face->n.z;
            VID_VERTEX16 *destination = output + idx * 4 + corner;
            if (memcmp(destination, &vertex, sizeof(vertex)) != 0)
            {
                if (compare_only)
                    return false;
                if (mesh->pool_valid)
                {
                    VID_VERTEX16 before = hd_pool_vertex(destination);
                    VID_VERTEX16 after = hd_pool_vertex(&vertex);
                    if (memcmp(&before, &after, sizeof(before)) != 0)
                        mesh->pool_valid = false;
                }
                *destination = vertex;
                changed = true;
            }
        }
        if (!semi)
            hd_mesh_pass(&pending, idx * 6, VID_BLEND_NONE, 0, 1.0f);
        else
        {
            uint32 mode = (textured ? mapping.page : flags) >> 5 & 3;
            if (textured)
                hd_mesh_pass(&pending, idx * 6, VID_BLEND_NONE, HD_OPAQUE_ONLY, 1.0f);
            hd_mesh_pass(&pending, idx * 6, blends[mode], textured ? HD_STP_ONLY : 0, alpha[mode]);
        }
    }
    if (compare_only)
        return mesh->pass_count == pending.pass_count && memcmp(mesh->passes, pending.passes, pending.pass_count * sizeof(HdMeshPass)) == 0;
    bool indices_changed = mesh->pass_count != pending.pass_count || memcmp(mesh->passes, pending.passes, pending.pass_count * sizeof(HdMeshPass)) != 0;
    mesh->pass_count = pending.pass_count;
    memcpy(mesh->passes, pending.passes, pending.pass_count * sizeof(HdMeshPass));
    if (indices_changed && !hd_mesh_opaque_indices(mesh))
        return false;
    if (changed)
    {
        hd_clear_morph(mesh);
        vid_mesh_commit(mesh->geometry);
    }
    if (mesh->pooled && !mesh->pool_valid)
        hd_state.world_pool_dirty = true;
    return true;
}

bool hd_mesh_update(HdMesh *mesh)
{
    return mesh != NULL && !mesh->queued && mesh->sample_refs == 0 && hd_mesh_update_source(mesh, false);
}

HdMesh *hd_mesh(const MeshVertex *vertices, uint32 vertex_count, uint32 first_vertex, const MeshFace *faces, uint32 face_count, const TextureDescriptor *material)
{
    if (vertices == NULL || faces == NULL || vertex_count == 0 || face_count == 0 || face_count > 0xFFFFFFFFu / 6 ||
        face_count > (size_t)-1 / (2 * sizeof(HdMeshPass)) || face_count > (size_t)-1 / (4 * sizeof(uint32)) || face_count > (size_t)-1 / sizeof(HdScroll) || face_count > (size_t)-1 / sizeof(HdOgMaterial))
        return NULL;
    HdMesh *mesh = hd_state.meshes;
    HdMesh *available = NULL;
    while (mesh != NULL)
    {
        if (mesh->vertices == vertices && mesh->faces == faces && mesh->vertex_count == vertex_count &&
            mesh->first_vertex == first_vertex && mesh->face_count == face_count && mesh->material == material)
        {
            if (mesh->queued || mesh->sample_refs != 0)
            {
                // Shared source arrays may change between instances in one frame
                if (hd_mesh_update_source(mesh, true))
                    return mesh;
            }
            else if (available == NULL)
                available = mesh;
        }
        mesh = mesh->next;
    }
    if (available != NULL)
        return hd_mesh_update(available) ? available : NULL;
    mesh = (HdMesh *)calloc(1, sizeof(HdMesh));
    if (mesh == NULL)
        return NULL;
    mesh->vertices = vertices;
    mesh->faces = faces;
    mesh->vertex_count = vertex_count;
    mesh->first_vertex = first_vertex;
    mesh->face_count = face_count;
    mesh->material = material;
    mesh->geometry = vid_mesh_create_16(face_count * 4);
    mesh->indices = vid_index_create(face_count * 6);
    mesh->passes = (HdMeshPass *)malloc(face_count * 2 * sizeof(HdMeshPass));
    mesh->staging_passes = (HdMeshPass *)malloc(face_count * 2 * sizeof(HdMeshPass));
    mesh->source_indices = (uint32 *)calloc(face_count * 4, sizeof(uint32));
    mesh->scrolls = (HdScroll *)calloc(face_count, sizeof(HdScroll));
    mesh->regions = (HdTexRegion *)calloc(face_count, sizeof(HdTexRegion));
    if (mesh->geometry != NULL && mesh->indices != NULL && mesh->passes != NULL && mesh->staging_passes != NULL && mesh->source_indices != NULL && mesh->scrolls != NULL && mesh->regions != NULL && hd_mesh_update(mesh))
    {
        uint32 *indices = vid_index_data(mesh->indices);
        static const uint8 order[6] = {0, 1, 2, 2, 1, 3};
        for (uint32 idx = 0; idx < face_count; ++idx)
            for (uint32 corner = 0; corner < 6; ++corner)
                indices[idx * 6 + corner] = idx * 4 + order[corner];
        vid_index_commit(mesh->indices, face_count * 6);
        vid_index_static(mesh->indices);
        mesh->next = hd_state.meshes;
        hd_state.meshes = mesh;
        return mesh;
    }
    if (mesh->geometry != NULL)
        vid_mesh_free(mesh->geometry);
    if (mesh->indices != NULL)
        vid_index_free(mesh->indices);
    if (mesh->opaque_indices != NULL)
        vid_index_free(mesh->opaque_indices);
    free(mesh->passes);
    free(mesh->staging_passes);
    free(mesh->source_indices);
    free(mesh->scrolls);
    free(mesh->regions);
    hd_og_geometry_release(mesh->og_geometry);
    free(mesh->og_materials);
    free(mesh);
    return NULL;
}

HdWorldDraw *hd_new_world_draw(void)
{
    if (hd_state.world_count == hd_state.world_capacity)
    {
        uint32 capacity = hd_state.world_capacity != 0 ? hd_state.world_capacity * 2 : 32;
        if (capacity <= hd_state.world_capacity || capacity > (size_t)-1 / sizeof(HdWorldDraw))
            return NULL;
        HdWorldDraw *draws = (HdWorldDraw *)realloc(hd_state.world_draws, (size_t)capacity * sizeof(HdWorldDraw));
        if (draws == NULL)
            return NULL;
        hd_state.world_draws = draws;
        hd_state.world_capacity = capacity;
    }
    HdWorldDraw *draw = hd_state.world_draws + hd_state.world_count++;
    memset(draw, 0, sizeof(*draw));
    draw->pose = hd_state.draw_pose;
    draw->scroll_fract = 1.0f;
    return draw;
}

bool hd_world(HdMesh *mesh, const MATRIXF *model, const HdCamera *camera, const HdLighting *lighting, const VID_FOG *fog, bool original_planes)
{
    if (!hd_state.frame_active || mesh == NULL || model == NULL || camera == NULL || camera->near_z <= 0 || camera->far_z <= camera->near_z)
        return false;
    if (fog != NULL && !(fog->near_dist > 0 && fog->far_dist > fog->near_dist && fog->horizon_dist > fog->far_dist && fog->focal[0] > 0 && fog->focal[1] > 0))
        return false;
    HdWorldDraw *draw = hd_new_world_draw();
    if (draw == NULL)
        return false;
    draw->mesh = mesh;
    draw->model = *model;
    mesh->queued = true;
    MATRIXF transform;
    CompMatrixf(&camera->view, model, &transform);
    float *uniforms = draw->uniforms;
    for (uint32 row = 0; row < 3; ++row)
    {
        memcpy(uniforms + row * 4, transform.m[row], sizeof(transform.m[row]));
        uniforms[row * 4 + 3] = transform.t[row];
    }
    uniforms[12] = camera->focal / hd_state.ui_half_width;
    uniforms[13] = camera->focal / 120.0f;
    uniforms[14] = (camera->far_z + camera->near_z) / (camera->far_z - camera->near_z);
    uniforms[15] = -2.0f * camera->far_z * camera->near_z / (camera->far_z - camera->near_z);
    MATRIXF inverse;
    TransposeMatrixf(&transform, &inverse);
    VECTORF translation = {transform.t[0], transform.t[1], transform.t[2]};
    ApplyMatrixf(&inverse, &translation, &translation);
    uniforms[18 * 4] = -translation.vx * 32.0f;
    uniforms[18 * 4 + 1] = -translation.vy * 32.0f;
    uniforms[18 * 4 + 2] = -translation.vz * 32.0f;
    uniforms[18 * 4 + 3] = original_planes ? 1.0f : 0.0f;
    uniforms[19 * 4 + 1] = camera->near_z;
    if (lighting != NULL)
    {
        for (uint32 idx = 0; idx < 3; ++idx)
        {
            const HdLight *light = lighting->lights + idx;
            memcpy(uniforms + (4 + idx) * 4, &light->dir, sizeof(light->dir));
            uniforms[(4 + idx) * 4 + 3] = light->point ? 1.0f : 0.0f;
            memcpy(uniforms + (7 + idx) * 4, &light->color, sizeof(light->color));
            uniforms[(7 + idx) * 4 + 3] = light->radius;
            memcpy(uniforms + (10 + idx) * 4, &light->pos, sizeof(light->pos));
            uniforms[(10 + idx) * 4 + 3] = light->attenuation_range;
            memcpy(uniforms + (13 + idx) * 4, &light->color_scale, sizeof(light->color_scale));
        }
        memcpy(uniforms + 16 * 4, &lighting->ambient, sizeof(lighting->ambient));
        uniforms[16 * 4 + 3] = 1.0f;
    }
    if (fog != NULL)
        draw->fog = *fog;
    return true;
}

void hd_source_clip(const SceneSection *scene, HdCamera *camera)
{
    // A reversed original interval rejects everything beyond fog_near without depth cue
    if (scene != NULL && scene->fog_far != 0 && scene->fog_far <= scene->fog_near)
        camera->far_z = fminf(camera->far_z, scene->fog_near / 32.0f);
}

const VID_FOG *hd_source_fog(const SceneSection *scene, const HdCamera *camera, VID_FOG *fog)
{
    if (scene == NULL || scene->fog_far == 0 || scene->fog_far <= scene->fog_near)
        return NULL;
    memset(fog, 0, sizeof(*fog));
    fog->near_dist = fmaxf(camera->near_z, scene->fog_near / 32.0f);
    fog->far_dist = scene->fog_far / 32.0f;
    if (fog->far_dist <= fog->near_dist)
        return NULL;
    // Original SetCameraProj selects a black GTE far colour
    fog->horizon_dist = fmaxf(camera->far_z, fog->far_dist) + camera->near_z;
    fog->focal[0] = camera->focal / hd_state.ui_half_width;
    fog->focal[1] = camera->focal / 120.0f;
    return fog;
}

void hd_select_lights(const SceneLighting *lighting, const VECTORF *origin, float radius, SceneLighting *selected)
{
    memset(selected, 0, sizeof(*selected));
    memcpy(selected->ambient, lighting->ambient, sizeof(selected->ambient));
    selected->skip_transform = lighting->skip_transform;
    if (lighting->skip_transform != 0)
        return;
    selected->lights = lighting->lights;
    uint32 row = 0, count = 0;
    const SceneLight *light = selected->lights;
    while (light != NULL && row < 3)
    {
        if ((light->flags & 1) == 0)
        {
            if ((light->event_flags & 2) != 0)
            {
                float extent = radius + light->radius;
                float x = light->src.pos.x / 32.0f - origin->vx;
                float y = light->src.pos.y / 32.0f - origin->vy;
                float z = light->src.pos.z / 32.0f - origin->vz;
                if (fabsf(x) > extent || fabsf(y) > extent || fabsf(z) > extent || x*x + y*y + z*z > extent*extent)
                {
                    light = (const SceneLight *)light->links.registry_next;
                    continue;
                }
                selected->point_lights[count++] = (SceneLight *)light;
            }
            ++row;
        }
        light = (const SceneLight *)light->links.registry_next;
    }
}

bool hd_scene(const SceneSection *scene, const HdCamera *camera, const SceneLighting *lighting, const VID_FOG *fog)
{
    HdCamera clipped = *camera;
    hd_source_clip(scene, &clipped);
    camera = &clipped;
    VID_FOG source_fog;
    if (fog == NULL)
        fog = hd_source_fog(scene, camera, &source_fog);
    const SceneNode *node = scene->object_heads[2];
    while (node != NULL)
    {
        const MeshGeometryNode *mesh = (const MeshGeometryNode *)node;
        node = node->links.registry_next;
        if ((mesh->node.flags & 1) != 0 || (mesh->flags & 0x80) != 0 || mesh->polygon_count <= 0)
            continue;
        const MeshGeometryNode *source = mesh->vertex_count > 0 ? mesh : mesh->src_mesh;
        if ((source->node.flags & 1) != 0 || (source == mesh && (mesh->flags & 0x2000) != 0))
            continue;
        VECTORF origin = {mesh->pos.x / 32.0f, mesh->pos.y / 32.0f, mesh->pos.z / 32.0f};
        float radius = mesh->node.state_flags;
        bool visible = hd_visible(camera, &origin, radius, hd_state.ui_half_width / 120.0f);
        uint32 first_vertex = (uint16)source->first_vertex;
        HdMesh *geometry = hd_mesh(source->vertices + first_vertex, source->vertex_count, first_vertex, mesh->polygons, mesh->polygon_count, NULL);
        if (geometry == NULL)
            return false;
        if (!geometry->pooled)
        {
            geometry->pooled = true;
            hd_state.world_pool_dirty = true;
        }
        if (!visible)
            continue;
        MATRIXF model;
        IdentityMatrixf(&model);
        model.t[0] = source->pos.x / 32.0f;
        model.t[1] = source->pos.y / 32.0f;
        model.t[2] = source->pos.z / 32.0f;
        SceneLighting selected;
        hd_select_lights(lighting, &origin, radius, &selected);
        HdLighting lights;
        hd_lighting(&selected, &model, &origin, &lights);
        if (!hd_world(geometry, &model, camera, &lights, fog, true))
            return false;
        hd_state.world_draws[hd_state.world_count - 1].group = &mesh->node;
    }
    return true;
}

void hd_draw_lights(uint32 first, const SceneLighting *source, const MATRIXF *model)
{
    MATRIXF identity;
    IdentityMatrixf(&identity);
    HdLighting lights;
    hd_lighting(source, &identity, NULL, &lights);
    for (uint32 idx = first; idx < hd_state.world_count; ++idx)
    {
        HdWorldDraw *draw = hd_state.world_draws + idx;
        draw->world_lights = lights;
        draw->model = *model;
        draw->relight = true;
    }
}

bool hd_actor(const AnimatedObject *actor, const MorphSpan *span, const MATRIXF *model, const HdCamera *camera, const SceneLighting *lighting, const TextureDescriptor *material, const VID_FOG *fog)
{
    if (span->polygon_count == 0)
        return true;
    if (span->vertex_count <= 0 || span->polygon_count < 0 || span->first_vertex < 0 || span->first_polygon < 0)
        return false;
    HdMesh *mesh = hd_mesh(actor->vertices + span->first_vertex, span->vertex_count, span->first_vertex,
        actor->polygons + span->first_polygon, span->polygon_count, material);
    if (mesh == NULL)
        return false;
    HdLighting lights;
    const SceneLighting *source = lighting;
    SceneLighting selected;
    if (lighting != NULL)
    {
        if (lighting->bounds_origin != NULL)
        {
            VECTORF origin = {lighting->bounds_origin[0] / 32.0f, lighting->bounds_origin[1] / 32.0f, lighting->bounds_origin[2] / 32.0f};
            hd_select_lights(lighting, &origin, lighting->bounds_radius, &selected);
            source = &selected;
        }
        hd_lighting(source, model, NULL, &lights);
    }
    uint32 first = hd_state.world_count;
    bool result = hd_world(mesh, model, camera, lighting != NULL ? &lights : NULL, fog, true);
    if (result && source != NULL)
        hd_draw_lights(first, source, model);
    return result;
}

static bool hd_pool_matches(const HdPoolRange *range, const HdMesh *mesh, const VID_VERTEX16 *source)
{
    if (range->vertices != mesh->vertices || range->faces != mesh->faces || range->count != mesh->face_count * 4 ||
        range->first_vertex != mesh->first_vertex || range->vertex_count != mesh->vertex_count)
        return false;
    const VID_VERTEX16 *stored = vid_mesh_data_16(hd_state.world_mesh);
    for (uint32 idx = 0; idx < range->count; ++idx)
    {
        VID_VERTEX16 vertex = hd_pool_vertex(source + idx);
        if (memcmp(stored + range->first + idx, &vertex, sizeof(vertex)) != 0)
            return false;
    }
    return true;
}

static bool hd_prepare_world_pool(void)
{
    if (!hd_state.world_pool_dirty)
        return true;
    bool changed = false;
    for (uint32 list = 0; list < 2; ++list)
    {
        for (HdMesh *mesh = list == 0 ? hd_state.meshes : hd_state.retired_meshes; mesh != NULL; mesh = mesh->next)
        {
            if (!mesh->pooled || mesh->pool_valid)
                continue;
            const VID_VERTEX16 *source = vid_mesh_data_16(mesh->geometry);
            if (source == NULL)
                return false;
            if (mesh->pool_range != NULL && hd_pool_matches(mesh->pool_range, mesh, source))
            {
                mesh->pool_valid = true;
                continue;
            }
            if (mesh->pool_range != NULL)
            {
                --mesh->pool_range->refs;
                mesh->pool_range = NULL;
            }
            HdPoolRange *range = hd_state.pool_ranges, *available = NULL;
            while (range != NULL)
            {
                if (hd_pool_matches(range, mesh, source))
                    break;
                if (range->refs == 0 && range->count == mesh->face_count * 4)
                    available = range;
                range = range->next;
            }
            if (range == NULL)
            {
                range = available;
                if (range == NULL)
                {
                    uint64 needed = (uint64)hd_state.world_vertex_count + (uint64)mesh->face_count * 4;
                    if (needed > 0xFFFFFFFFu / 6 * 4 || needed > (size_t)-1 / sizeof(VID_VERTEX16))
                        return false;
                    if (needed > hd_state.world_mesh_capacity)
                    {
                        uint32 capacity = hd_state.world_mesh_capacity != 0 ? hd_state.world_mesh_capacity : 4096;
                        while (capacity < needed && capacity <= 0xFFFFFFFFu / 2)
                            capacity *= 2;
                        if (capacity < needed || capacity > (size_t)-1 / sizeof(VID_VERTEX16) || capacity > 0xFFFFFFFFu / 6 * 4)
                            return false;
                        VID_GEOMETRY *geometry = vid_mesh_create_16(capacity);
                        VID_INDEX *indices = vid_index_create(capacity / 4 * 6);
                        if (geometry == NULL || indices == NULL)
                        {
                            vid_mesh_free(geometry);vid_index_free(indices);
                            return false;
                        }
                        if (hd_state.world_vertex_count != 0)
                            memcpy(vid_mesh_data_16(geometry), vid_mesh_data_16(hd_state.world_mesh), hd_state.world_vertex_count * sizeof(VID_VERTEX16));
                        vid_mesh_free(hd_state.world_mesh);vid_index_free(hd_state.world_indices);
                        hd_state.world_mesh = geometry;hd_state.world_indices = indices;
                        hd_state.world_mesh_capacity = capacity;hd_state.world_index_count = 0;
                        changed = true;
                    }
                    range = (HdPoolRange *)calloc(1, sizeof(HdPoolRange));
                    if (range == NULL)
                        return false;
                    range->first = hd_state.world_vertex_count;
                    range->count = mesh->face_count * 4;
                    hd_state.world_vertex_count += range->count;
                    range->next = hd_state.pool_ranges;hd_state.pool_ranges = range;
                }
                range->vertices = mesh->vertices;range->faces = mesh->faces;
                range->first_vertex = mesh->first_vertex;range->vertex_count = mesh->vertex_count;
                VID_VERTEX16 *output = vid_mesh_data_16(hd_state.world_mesh);
                for (uint32 idx = 0; idx < range->count; ++idx)
                {
                    VID_VERTEX16 vertex = hd_pool_vertex(source + idx);
                    if (memcmp(output + range->first + idx, &vertex, sizeof(vertex)) != 0)
                    {
                        output[range->first + idx] = vertex;
                        changed = true;
                    }
                }
            }
            ++range->refs;mesh->pool_range = range;mesh->pool_first = range->first;
            mesh->pool_valid = true;
        }
    }
    if (changed)
        vid_mesh_commit(hd_state.world_mesh);
    uint32 count = hd_state.world_vertex_count / 4 * 6;
    if (count != hd_state.world_index_count)
    {
        uint32 *indices = vid_index_data(hd_state.world_indices);
        static const uint8 order[6] = {0, 1, 2, 2, 1, 3};
        for (uint32 idx = 0; idx < count; ++idx)
            indices[idx] = idx / 6 * 4 + order[idx % 6];
        if (hd_state.world_indices != NULL)
            vid_index_commit(hd_state.world_indices, count);
        hd_state.world_index_count = count;
    }
    hd_state.world_pool_dirty = false;
    return true;
}

static bool hd_world_table(VID_TEXTURE **texture, uint32 *height, uint64 texels)
{
    uint64 rows = (texels + 1023) / 1024;
    if (rows <= *height)
        return true;
    uint32 capacity = *height != 0 ? *height : 1;
    while (capacity < rows && capacity <= 0xFFFFFFFFu / 2)
        capacity *= 2;
    if (capacity < rows || capacity > (size_t)-1 / 8192)
        return false;
    VID_TEXTURE *next = vid_tex_create_fmt(1024, capacity, VID_RGBA16UI);
    if (next == NULL)
        return false;
    memset(vid_tex_data(next), 0, (size_t)capacity * 8192);
    vid_tex_commit(next);
    vid_tex_free(*texture);
    *texture = next;
    *height = capacity;
    return true;
}

static bool hd_pool_material_equal(const HdMesh *left, const HdMesh *right)
{
    if (left == right)
        return true;
    if (memcmp(left->scrolls, right->scrolls, left->face_count * sizeof(HdScroll)) != 0)
        return false;
    const VID_VERTEX16 *a = vid_mesh_data_16(left->geometry), *b = vid_mesh_data_16(right->geometry);
    for (uint32 idx = 0; idx < left->face_count * 4; ++idx)
        if (memcmp(a[idx].uv, b[idx].uv, sizeof(a[idx].uv)) != 0 || memcmp(a[idx].color, b[idx].color, sizeof(a[idx].color)) != 0 ||
            memcmp(a[idx].tex, b[idx].tex, 3 * sizeof(uint16)) != 0)
            return false;
    return true;
}

static bool hd_prepare_visible_world(void)
{
    uint32 count = hd_state.world_count;
    if (count == 0)
        return true;
    if (count >= 0xFFFFFFFFu / (HD_WORLD_UNIFORMS * 8))
        return false;
    if (count > hd_state.visible_draw_capacity)
    {
        uint32 capacity = hd_state.visible_draw_capacity != 0 ? hd_state.visible_draw_capacity * 2 : 32;
        if (capacity < count)
            capacity = count;
        if (capacity > (size_t)-1 / sizeof(HdVisibleDraw))
            return false;
        HdVisibleDraw *draws = (HdVisibleDraw *)realloc(hd_state.visible_draws, capacity * sizeof(HdVisibleDraw));
        if (draws == NULL)
            return false;
        hd_state.visible_draws = draws;
        hd_state.visible_draw_capacity = capacity;
    }
    memset(hd_state.visible_draws, 0, count * sizeof(HdVisibleDraw));
    for (uint32 idx = 0; idx < count; ++idx)
    {
        const HdWorldDraw *draw = hd_state.world_draws + idx;
        HdVisibleDraw *visible = hd_state.visible_draws + idx;
        visible->pooled = draw->mesh != NULL && draw->mesh->pooled && draw->morph == NULL;
        visible->eligible = draw->group != NULL && visible->pooled &&
            draw->pose == HD_NO_POSE && draw->uniforms[78] == 0.0f && draw->mesh->opaque_count != 0;
        if (!visible->eligible)
            continue;
        for (uint32 prev = 0; prev < idx; ++prev)
        {
            const HdWorldDraw *old = hd_state.world_draws + prev;
            if (draw->group == old->group && draw->mesh == old->mesh && old->morph == NULL &&
                draw->scroll == old->scroll && draw->scroll_fract == old->scroll_fract &&
                memcmp(draw->uniforms, old->uniforms, sizeof(draw->uniforms)) == 0 && memcmp(&draw->fog, &old->fog, sizeof(draw->fog)) == 0)
            {
                visible->eligible = false;
                visible->duplicate = true;
                break;
            }
        }
    }
    uint64 indices = 0;
    bool any_mesh = false;
    for (uint32 idx = 0; idx < count; ++idx)
    {
        HdVisibleDraw *visible = hd_state.visible_draws + idx;
        if (!visible->pooled || visible->duplicate)
            continue;
        for (uint32 other = 0; other < count; ++other)
        {
            // One source range cannot select two different parameter rows in the same frame
            const HdMesh *mesh = hd_state.world_draws[idx].mesh;
            const HdMesh *alias = hd_state.world_draws[other].mesh;
            if (other != idx && !hd_state.visible_draws[other].duplicate && alias != NULL && alias->pooled &&
                alias->pool_range == mesh->pool_range && hd_state.world_draws[other].morph == NULL)
            {
                if (hd_state.world_draws[other].group != NULL)
                    visible->eligible = false;
                if (!hd_pool_material_equal(mesh, alias) ||
                    hd_state.world_draws[idx].scroll != hd_state.world_draws[other].scroll ||
                    hd_state.world_draws[idx].scroll_fract != hd_state.world_draws[other].scroll_fract)
                    visible->pooled = visible->eligible = false;
            }
        }
        if (visible->pooled)
        {
            indices += hd_state.world_draws[idx].mesh->opaque_count;
        }
    }
    uint64 material_faces = hd_state.world_index_count / 6;
    for (uint32 idx = 0; idx < count; ++idx)
    {
        const HdMesh *mesh = hd_state.world_draws[idx].mesh;
        HdVisibleDraw *visible = hd_state.visible_draws + idx;
        if (mesh == NULL || visible->duplicate)
            continue;
        any_mesh = true;
        if (visible->pooled)
            visible->material_first = mesh->pool_first / 4;
        else
        {
            if (material_faces > 0xFFFFFFFFu || mesh->face_count > 0xFFFFFFFFu - material_faces)
                return false;
            visible->material_first = (uint32)material_faces;
            material_faces += mesh->face_count;
        }
    }
    if (!any_mesh)
        return true;
    if (indices > 0xFFFFFFFFu || indices > (size_t)-1 / sizeof(uint32))
        return false;
    if (indices > hd_state.visible_capacity)
    {
        uint32 capacity = hd_state.visible_capacity != 0 ? hd_state.visible_capacity : 4096;
        while (capacity < indices && capacity <= 0xFFFFFFFFu / 2)
            capacity *= 2;
        if (capacity < indices || capacity > (size_t)-1 / sizeof(uint32))
            return false;
        VID_INDEX *next = vid_index_create(capacity);
        if (next == NULL)
            return false;
        vid_index_free(hd_state.visible_indices);
        hd_state.visible_indices = next;
        hd_state.visible_capacity = capacity;
        hd_state.visible_count = 0;
    }
    if (!hd_world_table(&hd_state.world_params, &hd_state.world_param_height, ((uint64)count + 1) * (HD_WORLD_UNIFORMS * 2)) ||
        !hd_world_table(&hd_state.world_groups, &hd_state.world_group_height, material_faces * 8))
        return false;
    if (hd_state.pooled_material == NULL)
    {
        hd_state.pooled_material = vid_mat_create();
        if (hd_state.pooled_material == NULL)
            return false;
    }
    hd_filter_material(hd_state.pooled_material);
    VID_MATERIAL_DATA *material = vid_mat_data(hd_state.pooled_material);
    if (material->aux[0] != hd_state.world_params || material->aux[1] != hd_state.world_groups)
    {
        *material = *vid_mat_data(hd_state.world_materials[VID_BLEND_NONE]);
        material->shader = hd_state.pooled_shader;
        material->aux[0] = hd_state.world_params;
        material->aux[1] = hd_state.world_groups;
        vid_mat_commit(hd_state.pooled_material);
    }
    for (uint32 idx = 0; idx < 4; ++idx)
    {
        VID_MATERIAL_DATA *local = vid_mat_data(hd_state.world_materials[idx]);
        if (local->aux[1] != hd_state.world_groups)
        {
            local->aux[1] = hd_state.world_groups;
            vid_mat_commit(hd_state.world_materials[idx]);
        }
        if (hd_state.world_pool_materials[idx] == NULL)
            hd_state.world_pool_materials[idx] = vid_mat_create();
        if (hd_state.world_pool_materials[idx] == NULL)
            return false;
        hd_filter_material(hd_state.world_pool_materials[idx]);
        VID_MATERIAL_DATA *data = vid_mat_data(hd_state.world_pool_materials[idx]);
        if (data->aux[1] != hd_state.world_groups)
        {
            *data = *vid_mat_data(hd_state.world_materials[idx]);
            data->shader = hd_state.world_pool_shader;data->aux[1] = hd_state.world_groups;
            vid_mat_commit(hd_state.world_pool_materials[idx]);
        }
    }
    uint32 *output = hd_state.visible_indices != NULL ? vid_index_data(hd_state.visible_indices) : NULL;
    uint16 *params = (uint16 *)vid_tex_data(hd_state.world_params);
    uint16 *groups = (uint16 *)vid_tex_data(hd_state.world_groups);
    bool indices_changed = false, params_changed = false, groups_changed = false;
    uint32 cursor = 0;
    for (uint32 idx = 0; idx < count; ++idx)
    {
        HdVisibleDraw *visible = hd_state.visible_draws + idx;
        if (hd_state.world_draws[idx].mesh == NULL || visible->duplicate)
            continue;
        const HdWorldDraw *draw = hd_state.world_draws + idx;
        const HdMesh *mesh = draw->mesh;
        if (draw->morph != NULL)
        {
            for (uint32 blend = 0; blend < 4; ++blend)
            {
                hd_filter_material(draw->morph->morph_materials[blend]);
                VID_MATERIAL_DATA *morph = vid_mat_data(draw->morph->morph_materials[blend]);
                if (morph->aux[1] != hd_state.world_groups)
                {
                    morph->aux[1] = hd_state.world_groups;
                    vid_mat_commit(draw->morph->morph_materials[blend]);
                }
            }
        }
        visible->first = cursor;
        visible->count = mesh->opaque_count;
        static const uint8 order[6] = {0, 1, 2, 2, 1, 3};
        for (uint32 part = 0; visible->pooled && part < mesh->pass_count; ++part)
        {
            const HdMeshPass *pass = mesh->passes + part;
            if (pass->blend != VID_BLEND_NONE)
                continue;
            for (uint32 index = pass->first; index < pass->first + pass->count; ++index)
            {
                uint32 value = mesh->pool_first + index / 6 * 4 + order[index % 6];
                if (output[cursor] != value)
                {
                    output[cursor] = value;
                    indices_changed = true;
                }
                ++cursor;
            }
        }
        uint32 group = idx + 1;
        const VID_VERTEX16 *packed = vid_mesh_data_16(mesh->geometry);
        for (uint32 face = 0; face < mesh->face_count; ++face)
        {
            uint16 *metadata = groups + (size_t)(visible->material_first + face) * 32;
            if (visible->eligible)
            {
                uint16 low = group & 0xFFFF, high = group >> 16;
                if (metadata[0] != low || metadata[1] != high)
                {
                    metadata[0] = low;metadata[1] = high;groups_changed = true;
                }
            }
            const VID_VERTEX16 *source = packed + face * 4;
            const HdScroll *scroll = mesh->scrolls + face;
            const HdScroll *old_scroll = draw->scroll != NULL ? draw->scroll->scrolls + face : scroll;
            float fract = draw->scroll_fract;
            if (!hd_scroll_pair(old_scroll, scroll))
            {
                old_scroll = scroll;
                fract = 1.0f;
            }
            uint16 scroll_flags = (scroll->flags >> 13) & 15;
            uint16 periods = scroll->period_u | (scroll->period_v << 8);
            if (metadata[2] != scroll_flags || metadata[3] != periods)
            {
                metadata[2] = scroll_flags;metadata[3] = periods;groups_changed = true;
            }
            uint32 fract_bits;
            memcpy(&fract_bits, &fract, sizeof(fract_bits));
            uint16 clock_values[8] = {(uint16)old_scroll->phase, (uint16)(old_scroll->phase >> 16),
                (uint16)scroll->phase, (uint16)(scroll->phase >> 16), old_scroll->fraction, scroll->fraction,
                (uint16)fract_bits, (uint16)(fract_bits >> 16)};
            if (memcmp(metadata + 20, clock_values, sizeof(clock_values)) != 0)
            {
                memcpy(metadata + 20, clock_values, sizeof(clock_values));groups_changed = true;
            }
            if (memcmp(metadata + 28, mesh->regions + face, sizeof(HdTexRegion)) != 0)
            {
                memcpy(metadata + 28, mesh->regions + face, sizeof(HdTexRegion));
                groups_changed = true;
            }
            uint16 values[16] = {0};
            for (uint32 corner = 0; corner < 4; ++corner)
            {
                values[corner * 2] = (uint16)source[corner].uv[0];
                values[corner * 2 + 1] = (uint16)source[corner].uv[1];
            }
            memcpy(values + 8, source->tex, 3 * sizeof(uint16));
            for (uint32 channel = 0; channel < 4; ++channel)
                values[12 + channel] = source->color[channel];
            if (memcmp(metadata + 4, values, sizeof(values)) != 0)
            {
                memcpy(metadata + 4, values, sizeof(values));groups_changed = true;
            }
        }
        if (!visible->eligible)
            continue;
        float uniforms[HD_WORLD_UNIFORMS * 4];
        memcpy(uniforms, draw->uniforms, sizeof(uniforms));
        uniforms[68] = 1.0f;
        uniforms[69] = uniforms[71] = 0.0f;
        for (uint32 word = 0; word < HD_WORLD_UNIFORMS * 4; ++word)
        {
            uint32 bits;
            memcpy(&bits, uniforms + word, sizeof(bits));
            uint16 low = bits & 0xFFFF, high = bits >> 16;
            uint16 *value = params + group * (HD_WORLD_UNIFORMS * 8) + word * 2;
            if (value[0] != low || value[1] != high)
            {
                value[0] = low;
                value[1] = high;
                params_changed = true;
            }
        }
    }
    if (hd_state.visible_indices != NULL && (indices_changed || cursor != hd_state.visible_count))
        vid_index_commit(hd_state.visible_indices, cursor);
    if (params_changed)
        vid_tex_commit(hd_state.world_params);
    if (groups_changed)
        vid_tex_commit(hd_state.world_groups);
    hd_state.visible_count = cursor;
    return true;
}

static void hd_world_pass(HdWorldPass *pass, VID_MATERIAL *material, const VID_FOG *fog, const float *uniforms)
{
    vid_uniforms(uniforms, HD_WORLD_UNIFORMS);
    if (pass->material != material || memcmp(&pass->fog, fog, sizeof(*fog)) != 0)
    {
        vid_fog(fog->near_dist > 0.0f ? fog : NULL);
        vid_pass(material);
        pass->material = material;
        pass->fog = *fog;
    }
}

bool hd_submit_world(void)
{
    if (!hd_prepare_world_pool())
        return false;
    if (!hd_prepare_visible_world())
        return false;
    HdWorldPass current = {0};
    // Transparent surfaces must see the completed opaque depth buffer
    for (uint32 transparent = 0; transparent < 2; ++transparent)
    {
        for (uint32 idx = 0; idx < hd_state.world_count; ++idx)
        {
            const HdVisibleDraw *visible = hd_state.visible_draws + idx;
            if (visible->duplicate)
                continue;
            const HdWorldDraw *draw = hd_state.world_draws + idx;
            if (transparent == 0 && visible->eligible)
            {
                uint32 count = visible->count;
                uint32 last = idx;
                while (last + 1 < hd_state.world_count)
                {
                    const HdVisibleDraw *next = hd_state.visible_draws + last + 1;
                    if (next->duplicate)
                    {
                        ++last;
                        continue;
                    }
                    if (!next->eligible || memcmp(&draw->fog, &hd_state.world_draws[last + 1].fog, sizeof(draw->fog)) != 0)
                        break;
                    count += next->count;
                    ++last;
                }
                float uniforms[HD_WORLD_UNIFORMS * 4] = {0};
                hd_world_pass(&current, hd_state.pooled_material, &draw->fog, uniforms);
                if (!vid_submit(hd_state.world_mesh, hd_state.visible_indices, visible->first, count))
                    return false;
                idx = last;
                continue;
            }
            const HdMesh *mesh = draw->mesh;
            float uniforms[HD_WORLD_UNIFORMS * 4];
            memcpy(uniforms, draw->uniforms, sizeof(uniforms));
            if (mesh == NULL)
            {
                if ((draw->blend != VID_BLEND_NONE) != (transparent != 0))
                    continue;
                hd_world_pass(&current, hd_state.sprite_materials[draw->blend], &draw->fog, uniforms);
                if (!vid_submit(hd_state.sprite_mesh, NULL, draw->first, draw->count))
                    return false;
                continue;
            }
            bool pooled = visible->pooled;
            uniforms[80] = (float)(visible->material_first & 0xFFFF);
            uniforms[81] = (float)(visible->material_first >> 16);
            VID_GEOMETRY *geometry = pooled ? hd_state.world_mesh : mesh->geometry;
            VID_INDEX *indices = pooled ? hd_state.world_indices : mesh->indices;
            uint32 first = pooled ? mesh->pool_first / 4 * 6 : 0;
            if (uniforms[78] != 0.0f)
            {
                if (transparent != 0)
                    continue;
                hd_world_pass(&current, draw->morph != NULL ? draw->morph->morph_materials[VID_BLEND_NONE] : pooled ? hd_state.world_pool_materials[VID_BLEND_NONE] : hd_state.world_materials[VID_BLEND_NONE], &draw->fog, uniforms);
                if (!vid_submit(geometry, indices, first, mesh->face_count * 6))
                    return false;
                continue;
            }
            if (transparent == 0)
            {
                if (mesh->opaque_count == 0)
                    continue;
                uniforms[17 * 4] = 1.0f;
                uniforms[17 * 4 + 1] = 0.0f;
                hd_world_pass(&current, draw->morph != NULL ? draw->morph->morph_materials[VID_BLEND_NONE] : pooled ? hd_state.world_pool_materials[VID_BLEND_NONE] : hd_state.world_materials[VID_BLEND_NONE], &draw->fog, uniforms);
                if (!vid_submit(geometry, pooled ? hd_state.visible_indices : mesh->opaque_indices, pooled ? visible->first : 0, mesh->opaque_count))
                    return false;
                continue;
            }
            for (uint32 part = 0; part < mesh->pass_count; ++part)
            {
                const HdMeshPass *pass = mesh->passes + part;
                if ((pass->blend != VID_BLEND_NONE) != (transparent != 0))
                    continue;
                uniforms[17 * 4] = pass->alpha;
                uniforms[17 * 4 + 1] = pass->mask;
                hd_world_pass(&current, draw->morph != NULL ? draw->morph->morph_materials[pass->blend] : pooled ? hd_state.world_pool_materials[pass->blend] : hd_state.world_materials[pass->blend], &draw->fog, uniforms);
                if (!vid_submit(geometry, indices, first + pass->first, pass->count))
                    return false;
            }
        }
    }
    vid_fog(NULL);
    return true;
}

void hd_mix_draw(HdWorldDraw *draw, const MATRIXF *model, const HdCamera *camera)
{
    MATRIXF transform;
    if (draw->mesh != NULL)
        CompMatrixf(&camera->view, model, &transform);
    else
        transform = camera->view;
    for (uint32 row = 0; row < 3; ++row)
    {
        memcpy(draw->uniforms + row * 4, transform.m[row], sizeof(transform.m[row]));
        draw->uniforms[row * 4 + 3] = transform.t[row];
    }
    if (draw->mesh != NULL)
    {
        MATRIXF inverse;
        TransposeMatrixf(&transform, &inverse);
        VECTORF pos = {transform.t[0], transform.t[1], transform.t[2]};
        ApplyMatrixf(&inverse, &pos, &pos);
        draw->uniforms[72] = -pos.vx * 32.0f;
        draw->uniforms[73] = -pos.vy * 32.0f;
        draw->uniforms[74] = -pos.vz * 32.0f;
    }
    if (!draw->relight)
        return;
    MATRIXF inverse;
    TransposeMatrixf(model, &inverse);
    if (draw->mesh == NULL)
    {
        VECTORF normal = {-camera->view.m[2][0], -camera->view.m[2][1], -camera->view.m[2][2]};
        ApplyMatrixf(&inverse, &normal, &normal);
        memcpy(draw->uniforms + 20, &normal, sizeof(normal));
    }
    for (uint32 idx = 0; idx < 3; ++idx)
    {
        const HdLight *light = draw->world_lights.lights + idx;
        VECTORF direction;
        uint32 dir_slot = draw->mesh != NULL ? 4 : 6;
        uint32 pos_slot = draw->mesh != NULL ? 10 : 12;
        if (light->point)
        {
            VECTORF relative = {light->pos.vx - model->t[0], light->pos.vy - model->t[1], light->pos.vz - model->t[2]};
            ApplyMatrixf(&inverse, &relative, &relative);
            memcpy(draw->uniforms + (pos_slot + idx) * 4, &relative, sizeof(relative));
            NormalizeVectorf(&relative, &direction);
        }
        else
            ApplyMatrixf(&inverse, &light->dir, &direction);
        memcpy(draw->uniforms + (dir_slot + idx) * 4, &direction, sizeof(direction));
    }
}

bool hd_prepare_morph(HdMesh *mesh)
{
    if (mesh->morph_texture != NULL)
        return true;
    const VID_VERTEX16 *source = vid_mesh_data_16(mesh->geometry);
    if (source == NULL)
        return false;
    uint64 texels = (uint64)mesh->face_count * 12;
    uint32 width = 1;
    while (width < texels && width < 1024)
        width *= 2;
    uint64 height = (texels + width - 1) / width;
    if (height > 0xFFFFFFFFu || height * width * 8 > (size_t)-1)
        return false;
    mesh->morph_texture = vid_tex_create_fmt(width, (uint32)height, VID_RGBA16UI);
    if (mesh->morph_texture == NULL)
        return false;
    mesh->morph_width = width;
    uint16 *data = (uint16 *)vid_tex_data(mesh->morph_texture);
    for (uint32 idx = 0; idx < mesh->face_count * 4; ++idx)
    {
        const VID_VERTEX16 *vertex = source + idx;
        uint16 *dst = data + idx * 12;
        dst[0] = (uint16)vertex->pos[0];
        dst[1] = (uint16)vertex->pos[1];
        dst[2] = (uint16)vertex->pos[2];
        dst[3] = (uint16)vertex->n[0];
        dst[4] = (uint16)vertex->n[1];
        dst[5] = (uint16)vertex->n[2];
        dst[6] = (uint16)vertex->pos[3];
        dst[7] = (uint16)vertex->n[3];
        dst[8] = vertex->tex[3];
        dst[9] = vertex->rect[0];
        dst[10] = vertex->rect[1];
        dst[11] = 0;
    }
    vid_tex_commit(mesh->morph_texture);
    for (uint32 idx = 0; idx < 4; ++idx)
    {
        mesh->morph_materials[idx] = vid_mat_create();
        if (mesh->morph_materials[idx] == NULL)
        {
            hd_clear_morph(mesh);
            return false;
        }
        VID_MATERIAL_DATA *material = vid_mat_data(mesh->morph_materials[idx]);
        *material = *vid_mat_data(hd_state.world_materials[idx]);
        material->aux[0] = mesh->morph_texture;
        vid_mat_commit(mesh->morph_materials[idx]);
    }
    return true;
}

HdMesh *hd_previous_source(const HdSnapshot *previous, const HdWorldDraw *draw, uint32 pose)
{
    HdMesh *match = NULL;
    const HdMesh *mesh = draw->mesh;
    for (uint32 idx = 0; idx < previous->world_count; ++idx)
    {
        HdMesh *old = previous->world_draws[idx].mesh;
        if (previous->world_draws[idx].pose == pose && previous->world_draws[idx].group == draw->group && old != NULL && old->vertices == mesh->vertices &&
            old->faces == mesh->faces && old->vertex_count == mesh->vertex_count && old->first_vertex == mesh->first_vertex &&
            old->face_count == mesh->face_count && old->material == mesh->material &&
            memcmp(old->source_indices, mesh->source_indices, mesh->face_count * 4 * sizeof(uint32)) == 0)
        {
            if (match != NULL)
                return NULL;
            match = old;
        }
    }
    return match;
}

HdMesh *hd_previous_mesh(const HdSnapshot *previous, const HdWorldDraw *draw, uint32 pose)
{
    const HdMesh *mesh = draw->mesh;
    HdMesh *match = hd_previous_source(previous, draw, pose);
    if (match == NULL || match == mesh)
        return NULL;
    const VID_VERTEX16 *old_vertices = vid_mesh_data_16(match->geometry);
    const VID_VERTEX16 *vertices = vid_mesh_data_16(mesh->geometry);
    if (old_vertices == NULL || vertices == NULL)
        return NULL;
    for (uint32 idx = 0; idx < mesh->face_count * 4; ++idx)
    {
        const VID_VERTEX16 *a = old_vertices + idx, *b = vertices + idx;
        if (memcmp(a->pos, b->pos, sizeof(a->pos)) != 0 || memcmp(a->n, b->n, sizeof(a->n)) != 0 ||
            a->tex[3] != b->tex[3] || memcmp(a->rect, b->rect, sizeof(a->rect)) != 0)
            return match;
    }
    return NULL;
}

void hd_frame_actor(const AnimatedObject *actor, const MorphSpan *span, const SceneLighting *lighting, const TextureDescriptor *material, float depth_bias, bool flat_color, uint32 color)
{
    if (!hd_frame.collecting || hd_frame.failed)
        return;
    if (!hd_state.pose_active || hd_state.pose_owner != actor)
    {
        hd_frame.failed = true;
        return;
    }
    MATRIXF model = hd_state.poses[hd_state.pose_depth].model;
    hd_state.draw_pose = hd_state.poses[hd_state.pose_depth].sample;
    uint32 first = hd_state.world_count;
    VID_FOG source_fog;
    const VID_FOG *fog = hd_source_fog(hd_state.scene, &hd_frame.camera, &source_fog);
    if (!hd_frame.camera_valid || !hd_actor(actor, span, &model, &hd_frame.camera, flat_color ? NULL : lighting, material, fog))
        hd_frame.failed = true;
    for (uint32 idx = first; idx < hd_state.world_count; ++idx)
    {
        hd_state.world_draws[idx].uniforms[76] = depth_bias;
        hd_state.world_draws[idx].uniforms[78] = flat_color ? 1.0f : 0.0f;
        hd_state.world_draws[idx].uniforms[79] = color & 0xFFFFFF;
    }
    hd_state.draw_pose = HD_NO_POSE;
    if (hd_state.pose_active && hd_state.pose_owner == actor && !hd_frame.failed)
        hd_pose_camera_scratch(&model);
}
