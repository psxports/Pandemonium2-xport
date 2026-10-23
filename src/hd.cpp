#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hd.h"
#include "psx.h"
#include "psx_gpu.h"
#include "texture_descriptor.h"
#include "camera_projection.h"
#include "vector3.h"
#include "mesh_geometry_node.h"
#include "scene_lighting.h"
#include "scene_section.h"
#include "animated_object.h"
#include "animation_header.h"
#include "animation_track.h"
#include "background_resource.h"
#include "sprite_record.h"
#include "global.h"
#include "path_definition.h"
#include "scene_trigger.h"
#include "global.h"
#include "render_commands.h"
#include "screen_render_state.h"
#include "original_defaults.h"

enum
{
    HD_WORLD_UNIFORMS = 21,
    HD_NO_POSE = 0xFFFFFFFFu,
    HD_POSE_PARENT_POS = 1,
    HD_POSE_PARENT_ROT = 2,
    HD_POOL_SLOTS = SpritePoolState::EntryCount
};

struct HdMeshPass
{
    uint32 first, count, blend, mask;
    float alpha;
};

struct HdPoolRange
{
    HdPoolRange *next;
    const MeshVertex *vertices;
    const MeshFace *faces;
    uint32 first, count, refs, first_vertex, vertex_count;
};

struct HdScrollClock
{
    HdScrollClock *next;
    const TextureDescriptor *texture;
    uint32 frame;
    sint32 rate;
    uint64 epoch;
};

struct HdScroll
{
    const TextureDescriptor *texture, *source;
    TextureMapping mapping, source_mapping;
    uint32 flags, phase, frame;
    sint32 rate;
    uint64 epoch;
    uint16 fraction, period_u, period_v;
};

struct HdOgGeometry
{
    uint32 refs, vertex_count, face_count;
    MeshVertex *vertices;
    MeshFace *faces;
};

struct HdOgMaterial
{
    TextureDescriptor texture, source;
    bool present;
};

struct HdMesh
{
    HdMesh *next;
    const MeshVertex *vertices;
    const MeshFace *faces;
    const TextureDescriptor *material;
    uint32 vertex_count, first_vertex, face_count;
    HdOgGeometry *og_geometry;
    HdOgMaterial *og_materials;
    HdOgMaterial og_override;
    VID_GEOMETRY *geometry;
    VID_INDEX *indices;
    VID_INDEX *opaque_indices;
    uint32 opaque_count;
    HdMeshPass *passes;
    HdMeshPass *staging_passes;
    uint32 *source_indices;
    HdScroll *scrolls;
    VID_TEXTURE *morph_texture;
    VID_MATERIAL *morph_materials[4];
    uint32 morph_width;
    uint32 pass_count;
    uint32 sample_refs;
    uint32 pool_first;
    HdPoolRange *pool_range;
    bool queued, pooled, pool_valid;
};

struct HdWorldDraw
{
    HdMesh *mesh;
    HdMesh *morph;
    HdMesh *scroll;
    float scroll_fract;
    uint32 first, count, blend;
    uint32 pose;
    MATRIXF model;
    HdLighting world_lights;
    bool relight;
    float uniforms[HD_WORLD_UNIFORMS * 4];
    VID_FOG fog;
    const SceneNode *group;
};

struct HdVisibleDraw
{
    uint32 first, count, material_first;
    bool eligible, duplicate, pooled;
};

struct HdUiPass
{
    uint32 first;
    uint32 count;
    uint32 blend;
    HdUiLayer layer;
};

struct HdPose
{
    MATRIXF model;
    const SceneNode *node;
    const AnimatedObject *owner;
    uint32 sample;
    bool valid;
};

struct HdIdentity
{
    const SceneNode *node;
    uint64 id;
};

struct HdGraphWork
{
    const SceneNode *node;
    uint32 work;
};

struct HdPoseSample
{
    uint64 id, owner;
    uint32 parent, space;
    MATRIXF local, world;
    QUATF rotation;
};

struct HdPoseIndex
{
    uint64 id;
    uint32 index;
};

struct HdMixedPose
{
    MATRIXF model;
    uint32 previous;
};

struct HdPathPose
{
    const PathTransform *path;
    MATRIX source;
    MATRIXF model;
};

struct HdBackdrop
{
    const BackgroundResource *source;
    float pitch, yaw, width;
    float offset[2];
};

struct HdOgQuad
{
    uint32 pos, extent, blend;
    TextureDescriptor texture;
    HdOgQuadEmit emit;
};

struct HdOgRect
{
    uint32 pos, extent, color;
    sint32 fraction;
    HdOgRectEmit emit;
};

struct HdOgEffect
{
    sint32 intensity;
    HdOgEffectEmit emit;
};

struct HdOgSceneFade
{
    uint32 level;
    HdOgFadeEmit emit;
};

struct HdOgBackgroundRect
{
    uint32 pos, extent, color;
    HdOgBackgroundRectEmit emit;
};

struct HdOgBackgroundTile
{
    uint32 tile, xy, next_xy;
    BackgroundTile definition;
};

struct HdOgBackground
{
    uint32 refs, count;
    HdOgBackgroundTile *tiles;
};

enum HdOgKind
{
    HD_OG_QUAD, HD_OG_RECT, HD_OG_EFFECT, HD_OG_SCENE_FADE, HD_OG_BACKGROUND_RECT, HD_OG_BACKGROUND_TILES
};

struct HdOgDraw
{
    HdOgKind kind;
    union
    {
        HdOgQuad quad;
        HdOgRect rect;
        HdOgEffect effect;
        HdOgSceneFade fade;
        HdOgBackgroundRect background_rect;
        HdOgBackground *background;
    };
};

struct HdSnapshot
{
    HdOgDraw *og_draws;
    uint32 og_count, og_capacity;
    HdWorldDraw *world_draws;
    uint32 world_count, world_capacity;
    VID_VERTEX2D *ui_vertices, *sprite_vertices;
    uint32 ui_count, ui_capacity, sprite_count, sprite_capacity;
    HdUiPass *ui_passes;
    uint32 pass_count, pass_capacity;
    uint32 clear_color, vram_version, image_version;
    uint64 content_version, sprite_content_version;
    float ui_half_width;
    bool valid, restore_image;
    HdPoseSample *nodes;
    uint32 node_count, node_capacity;
    HdCamera camera;
    const SceneSection *scene;
    uint64 epoch, camera_epoch, collection, clock_index;
    bool camera_valid;
    bool pool_valid[HD_POOL_SLOTS];
    HdBackdrop backdrop;
};

struct HdState
{
    HdOgDraw *og_draws;
    uint32 og_count, og_capacity;
    bool og_background_skipped;
    HdOgDraw *frozen_og_draws;
    uint32 frozen_og_count, frozen_og_capacity;
    bool frozen_og_ui_valid;
    HdScrollClock *scroll_clocks;
    uint64 scroll_epoch;
    VID_TARGET *live_image, *saved_image;
    VID_GEOMETRY *image_mesh;
    VID_MATERIAL *image_material;
    VID_SHADER *image_shader;
    bool last_hd, image_valid, restore_image, paused;
    SceneSection *pause_scene;
    HdSceneRedraw pause_redraw;
    sint32 pause_width, pause_height;
    bool pause_valid, pause_hd, pause_collect;
    VID_SHADER *ui_shader;
    VID_SHADER *world_shader;
    VID_SHADER *sprite_shader;
    VID_TEXTURE *vram;
    VID_TEXTURE *empty_morph;
    VID_MATERIAL *ui_materials[4];
    VID_MATERIAL *world_materials[4];
    VID_MATERIAL *sprite_materials[4];
    VID_GEOMETRY *sprite_mesh;
    VID_VERTEX2D *sprite_vertices;
    uint32 sprite_count, sprite_capacity, sprite_mesh_capacity;
    HdMesh *meshes, *retired_meshes;
    VID_GEOMETRY *world_mesh;
    VID_INDEX *world_indices;
    uint32 world_mesh_capacity, world_index_count, world_vertex_count;
    HdPoolRange *pool_ranges;
    bool world_pool_dirty;
    VID_INDEX *visible_indices;
    uint32 visible_capacity, visible_count;
    VID_TEXTURE *world_params, *world_groups;
    uint32 world_param_height, world_group_height;
    VID_SHADER *pooled_shader;
    VID_MATERIAL *pooled_material;
    VID_SHADER *world_pool_shader;
    VID_MATERIAL *world_pool_materials[4];
    HdVisibleDraw *visible_draws;
    uint32 visible_draw_capacity;
    HdWorldDraw *world_draws;
    uint32 world_count, world_capacity;
    VID_GEOMETRY *ui_mesh;
    VID_VERTEX2D *ui_vertices;
    HdUiPass *ui_passes;
    uint32 ui_count;
    uint32 ui_capacity;
    uint32 pass_count;
    uint32 pass_capacity;
    uint32 mesh_capacity;
    uint32 vram_version;
    uint32 clear_color, image_version, latest_sample;
    uint64 next_content_version, content_version, ui_version, sprite_version;
    uint64 live_collection;
    uint64 sprite_content_version;
    HdSnapshot samples[2];
    HdBackdrop backdrop;
    HdPose *poses;
    uint32 pose_depth, pose_capacity;
    const AnimatedObject *pose_owner;
    HdPathPose *paths;
    uint32 path_count, path_capacity, graph_depth;
    bool pose_active;
    HdGraphWork *graph_work;
    uint32 graph_work_count, graph_work_capacity;
    HdIdentity *identities;
    uint32 identity_count, identity_capacity;
    uint64 identity_serial, pose_epoch, collection;
    HdPoseSample *nodes;
    uint32 node_count, node_capacity;
    const SceneSection *scene;
    uint32 draw_pose;
    uint64 clock_index;
    HdWorldDraw *mixed_draws;
    uint32 mixed_draw_capacity;
    VID_VERTEX2D *mixed_sprites;
    uint32 mixed_sprite_capacity;
    HdMixedPose *mixed_poses;
    uint32 mixed_pose_capacity;
    HdPoseIndex *previous_index, *current_index;
    uint32 index_capacity;
    const SpriteRecord *sprite_pool;
    bool pool_valid[HD_POOL_SLOTS];
    uint64 camera_epoch;
    uint64 camera_sample_epoch;
    float ui_half_width;
    bool vram_valid;
    bool frame_active;
};

static HdState hd_state;
HdFrameState hd_frame;


static const char hd_ui_vertex[] =
    "layout(location=0) in vec4 aPos;\n"
    "layout(location=1) in vec2 aUV;\n"
    "layout(location=2) in vec4 aColor;\n"
    "layout(location=4) in float aAlpha;\n"
    "layout(location=5) in uvec4 aTex;\n"
    "layout(location=6) in uvec2 aRect;\n"
    "out vec2 vUV; out vec4 vColor; out float vDepth; out vec2 vFog;\n"
    "flat out uvec4 vTex;\n"
    "uniform vec4 uDraw[1];flat out vec4 vZBias;\n"
    "void main(){\n"
    " gl_Position=aPos; vUV=aUV; vColor=vec4(aColor.rgb/255.0,aAlpha);\n"
    " if((aTex.z&16u)!=0u){\n"
    "  float x=aPos.x-160.0;\n"
    "  if((aTex.z&32u)!=0u)x=aPos.x-uDraw[0].x;\n"
    "  else if((aTex.z&64u)!=0u)x=aPos.x-320.0+uDraw[0].x;\n"
    "  else if((aTex.z&128u)!=0u)x*=uDraw[0].x/160.0;\n"
    "  if((aTex.z&128u)==0u)x+=uDraw[0].y;\n"
    "  gl_Position=vec4(x/uDraw[0].x,1.0-(aPos.y+uDraw[0].z)/120.0,aPos.z,1.0);\n"
    " }\n"
    " vDepth=0.0; vFog=vec2(0.0); vTex=aTex;vZBias=vec4(0.0);\n"
    "}\n";

// Model/view, projection, lights, morph, local camera and original depth offsets
static const char hd_world_vertex[] =
    "layout(location=0) in vec4 aPos; layout(location=1) in vec2 aUV;\n"
    "layout(location=2) in vec4 aColor; layout(location=3) in vec4 aNormal;\n"
    "layout(location=4) in vec4 aExtra; layout(location=5) in uvec4 aTex;\n"
    "layout(location=6) in uvec2 aRect;\n"
    "uniform vec4 uDraw[21]; uniform vec4 uFog[3];flat out vec4 vZBias;\n"
    "uniform highp usampler2D uAux0;\n"
    "out vec2 vUV; out vec4 vColor; out float vDepth; out vec2 vFog; flat out uvec4 vTex;\n"
    "#if defined(HD_POOL)||defined(HD_POOL_MATERIAL)||defined(HD_LOCAL_MATERIAL)\n"
    "uniform highp usampler2D uAux1;\n"
    "uvec4 materialWord(uint index){return texelFetch(uAux1,ivec2(index%1024u,index/1024u),0);}\n"
    "#endif\n#ifdef HD_POOL\nuint hd_group;\n"
    "vec4 hd_draw(int row){\n"
    " uint index=hd_group*42u+uint(row)*2u;\n"
    " uvec4 a=texelFetch(uAux0,ivec2(index%1024u,index/1024u),0);++index;\n"
    " uvec4 b=texelFetch(uAux0,ivec2(index%1024u,index/1024u),0);\n"
    " return uintBitsToFloat(uvec4(a.x|(a.y<<16u),a.z|(a.w<<16u),b.x|(b.y<<16u),b.z|(b.w<<16u)));\n"
    "}\n#define D(i) hd_draw(i)\n"
    "#else\n#define D(i) uDraw[i]\n#endif\n"
    "float lightDistance(vec3 v){\n"
    " vec3 a=abs(v); float hi=max(a.x,max(a.y,a.z)); float lo=min(a.x,min(a.y,a.z));\n"
    " float mid=a.x+a.y+a.z-hi-lo;\n"
    " if(lo>0.0)return (36.0*lo+48.0*mid+120.0*hi)/128.0;\n"
    " return mid>0.0?(51.0*mid+123.0*hi)/128.0:hi;\n"
    "}\n"
    "float signed16(uint v){return v<32768u?float(v):float(int(v)-65536);}\n"
    "#if defined(HD_POOL)||defined(HD_POOL_MATERIAL)||defined(HD_LOCAL_MATERIAL)\n"
    "float scrollOffset(uint oldPhase,uint phase,uint period,bool forward,vec2 fraction,float fract){\n"
    " if(period==0u)return 0.0;float width=float(period);\n"
    " int delta=int(phase-oldPhase);float step=float(delta)+fraction.y-fraction.x;\n"
    " float advance=fraction.x+step*fract,value=float(oldPhase%period)+advance;\n"
    " float wrap=float((4294967295u%period+1u)%period);\n"
    " if(delta>0&&phase<oldPhase&&advance>=float(0u-oldPhase))value-=wrap;\n"
    " if(delta<0&&phase>oldPhase&&advance < -float(oldPhase))value+=wrap;\n"
    " value=mod(value,width);\n"
    " float current=float(phase%period);return forward?value-current:current-value;\n"
    "}\n#endif\n"
    "vec3 signed3(uvec3 v){return vec3(signed16(v.x),signed16(v.y),signed16(v.z));}\n"
    "uvec4 morphWord(uint index){uint width=uint(D(17).w);return texelFetch(uAux0,ivec2(index%width,index/width),0);}\n"
    "void main(){\n"
    " vec2 uv=aUV;vec4 vertexColor=aColor;uvec4 tex=aTex;\n"
    "#if defined(HD_POOL)||defined(HD_POOL_MATERIAL)||defined(HD_LOCAL_MATERIAL)\n"
    " uint face=uint(gl_VertexID)/4u,corner=uint(gl_VertexID)&3u;\n"
    "#ifdef HD_LOCAL_MATERIAL\n"
    " face+=uint(D(20).x)|(uint(D(20).y)<<16u);\n"
    "#endif\n"
    " uint base=face*7u;\n"
    "#ifdef HD_POOL\n"
    " uvec4 group=materialWord(base);hd_group=group.x|(group.y<<16u);\n"
    "#endif\n"
    " uvec4 coords=materialWord(base+1u+corner/2u);uvec2 pair=(corner&1u)==0u?coords.xy:coords.zw;\n"
    " uv=vec2(signed16(pair.x),signed16(pair.y));tex.xyz=materialWord(base+3u).xyz;vertexColor=vec4(materialWord(base+4u));\n"
    " uvec4 scroll=materialWord(base),phases=materialWord(base+5u),times=materialWord(base+6u);\n"
    " uint oldPhase=phases.x|(phases.y<<16u),phase=phases.z|(phases.w<<16u);\n"
    " vec2 fractions=vec2(times.xy)/65536.0;float fract=uintBitsToFloat(times.z|(times.w<<16u));\n"
    " uv+=vec2(scrollOffset(oldPhase,phase,scroll.w&255u,(scroll.z&1u)!=0u,fractions,fract),\n"
    "          scrollOffset(oldPhase,phase,scroll.w>>8u,(scroll.z&4u)!=0u,fractions,fract));\n"
    "#endif\n"
    " vec4 p=vec4(aPos.xyz,1.0);\n"
    " vec3 normal=aNormal.xyz/4096.0; vec3 lightOrigin=aExtra.xyz;\n"
    " vec3 faceNormal=vec3(signed16(aTex.w),signed16(aRect.x),signed16(aRect.y))/4096.0;\n"
    " uint planeWord=(uint(int(aPos.w))&65535u)|((uint(int(aNormal.w))&65535u)<<16u);\n"
    " int plane=int(planeWord);float planeValue=float(plane);\n"
    " float reverseLimit=plane>=0?float(1073739776-plane):1073739776.0-float(plane);\n"
    " if(D(17).w>0.0){\n"
    "  uint index=uint(gl_VertexID)*3u;uvec4 a=morphWord(index),b=morphWord(index+1u),c=morphWord(index+2u);\n"
    "  float phase=D(17).z;p.xyz=mix(signed3(a.xyz),p.xyz,phase);\n"
    "  normal=mix(signed3(uvec3(a.w,b.xy))/4096.0,normal,phase);\n"
    "  lightOrigin=mix(signed3(morphWord((uint(gl_VertexID)&~3u)*3u).xyz),lightOrigin,phase);\n"
    "  faceNormal=mix(signed3(c.xyz)/4096.0,faceNormal,phase);\n"
    "  int oldPlane=int(b.z|(b.w<<16u));planeValue=mix(float(oldPlane),planeValue,phase);\n"
    "  float oldLimit=oldPlane>=0?float(1073739776-oldPlane):1073739776.0-float(oldPlane);\n"
    "  reverseLimit=mix(oldLimit,reverseLimit,phase);\n"
    " }\n"
    " vec3 view=vec3(dot(D(0),p),dot(D(1),p),dot(D(2),p));\n"
    " gl_Position=vec4(view.x*D(3).x,-view.y*D(3).y,view.z*D(3).z+D(3).w,view.z);\n"
    " vZBias=vec4(aExtra.w+D(19).x,D(19).y,D(3).zw);\n"
    " float side=dot(D(18).xyz,faceNormal);\n"
    " if(D(18).w!=0.0&&side<=-planeValue)gl_Position=vec4(2.0,2.0,2.0,1.0);\n"
    " if((tex.z&256u)!=0u&&side<=reverseLimit)normal=-normal;\n"
    " vec3 rgb=vertexColor.rgb;\n"
    " if(D(16).w!=0.0){\n"
    "  vec3 light=D(16).rgb+vec3(vertexColor.a/256.0);\n"
    "  for(int i=0;i<3;i++){\n"
    "   vec3 color=D(7+i).rgb;\n"
    "   if(D(4+i).w!=0.0){\n"
    "    float attenuation=D(7+i).w-lightDistance(D(10+i).xyz-lightOrigin);\n"
    "    if(attenuation<=0.0)color=vec3(0.0);\n"
    "    else if(attenuation<D(10+i).w)color=attenuation*D(13+i).rgb;\n"
    "   }\n"
    "   light+=color*clamp(dot(D(4+i).xyz,normal),0.0,32767.0/4096.0);\n"
    "  }\n"
    "  rgb=floor(clamp(rgb*clamp(light,vec3(0.0),vec3(32767.0/4096.0)),vec3(0.0),vec3(255.0)));\n"
    " }\n"
    " vUV=uv; vColor=vec4(rgb/255.0,D(17).x); vTex=tex;\n"
    " if(D(17).y!=0.0)vTex.z=(vTex.z&~12u)|uint(D(17).y);\n"
    " if(D(19).z!=0.0){uint color=uint(D(19).w);vColor=vec4(vec3(color&255u,(color>>8u)&255u,(color>>16u)&255u)/255.0,1.0);vTex.z=0u;}\n"
    " vDepth=view.z; vFog=vec2(0.0); float distance=length(view);\n"
    " if(uFog[0].y>uFog[0].x){\n"
    "  vFog.x=clamp((distance-uFog[0].x)/(uFog[0].y-uFog[0].x),0.0,1.0);\n"
    "  if(uFog[0].z>uFog[0].y)vFog.y=clamp((distance-uFog[0].y)/(uFog[0].z-uFog[0].y),0.0,1.0);\n"
    " }\n"
    "}\n";

// Centers stay float; signed extents expand in the shader without rounding world positions
static const char hd_sprite_vertex[] =
    "layout(location=0) in vec4 aPos; layout(location=1) in vec2 aUV;\n"
    "layout(location=2) in vec4 aColor; layout(location=4) in float aHeight;\n"
    "layout(location=5) in uvec4 aTex; layout(location=6) in uvec2 aRect;\n"
    "uniform vec4 uDraw[21]; uniform vec4 uFog[3];flat out vec4 vZBias;\n"
    "out vec2 vUV; out vec4 vColor; out float vDepth; out vec2 vFog; flat out uvec4 vTex;\n"
    "float lightDistance(vec3 v){\n"
    " vec3 a=abs(v);float hi=max(a.x,max(a.y,a.z));float lo=min(a.x,min(a.y,a.z));\n"
    " float mid=a.x+a.y+a.z-hi-lo;\n"
    " if(lo>0.0)return (36.0*lo+48.0*mid+120.0*hi)/128.0;\n"
    " return mid>0.0?(51.0*mid+123.0*hi)/128.0:hi;\n"
    "}\n"
    "void main(){\n"
    " mat3 rotation=transpose(mat3(uDraw[0].xyz,uDraw[1].xyz,uDraw[2].xyz));\n"
    " vec3 center=rotation*aPos.xyz+vec3(uDraw[0].w,uDraw[1].w,uDraw[2].w);\n"
    " vec3 relative=transpose(rotation)*center; vec3 delta=vec3(aPos.w,aHeight,0.0);\n"
    " if(aRect.y==1u){\n"
    "  float pitch=(float(aTex.w)+uDraw[20].x)*0.00153398078789, yaw=(float(aRect.x)+uDraw[20].y)*0.00153398078789;\n"
    "  float cy=cos(yaw),sy=sin(yaw),cx=cos(pitch),sx=sin(pitch);\n"
    "  delta=rotation*vec3(cy*aPos.w+sy*cx*aHeight,-sx*aHeight,-sy*aPos.w+cy*cx*aHeight);\n"
    " }else if(aRect.y==2u){\n"
    "  float yaw=atan(relative.x,relative.z);\n"
    "  delta=rotation*vec3(cos(yaw)*aPos.w,aHeight,-sin(yaw)*aPos.w);\n"
    " }\n"
    " vec3 view=center+delta;\n"
    " gl_Position=vec4(view.x*uDraw[3].x,-view.y*uDraw[3].y,view.z*uDraw[3].z+uDraw[3].w,view.z);\n"
    " vZBias=vec4(uDraw[19].xy,uDraw[3].zw);\n"
    " if(aRect.y==1u&&relative.y>=0.0)gl_Position=vec4(2.0,2.0,2.0,1.0);\n"
    " vec3 rgb=aColor.rgb+vec3(uDraw[20].zw,uDraw[19].z);\n"
    " if(uDraw[5].w!=0.0){\n"
    "  vec3 light=uDraw[18].rgb+vec3((aColor.a+uDraw[19].w)/256.0);\n"
    "  for(int i=0;i<3;i++){\n"
    "   vec3 color=uDraw[9+i].rgb;\n"
    "   if(uDraw[6+i].w!=0.0){\n"
    "    float attenuation=uDraw[9+i].w-lightDistance(uDraw[12+i].xyz);\n"
    "    if(attenuation<=0.0)color=vec3(0.0);\n"
    "    else if(attenuation<uDraw[12+i].w)color=attenuation*uDraw[15+i].rgb;\n"
    "   }\n"
    "   light+=color*clamp(dot(uDraw[6+i].xyz,uDraw[5].xyz),0.0,32767.0/4096.0);\n"
    "  }\n"
    "  rgb=floor(clamp(rgb*clamp(light,vec3(0.0),vec3(32767.0/4096.0)),vec3(0.0),vec3(255.0)));\n"
    " }\n"
    " vUV=aUV;vColor=vec4(rgb/255.0,uDraw[4].x);vTex=aTex;vTex.z|=uint(uDraw[4].y);\n"
    " vDepth=view.z;vFog=vec2(0.0);float distance=length(view);\n"
    " if(uFog[0].y>uFog[0].x){\n"
    "  vFog.x=clamp((distance-uFog[0].x)/(uFog[0].y-uFog[0].x),0.0,1.0);\n"
    "  if(uFog[0].z>uFog[0].y)vFog.y=clamp((distance-uFog[0].y)/(uFog[0].z-uFog[0].y),0.0,1.0);\n"
    " }\n"
    "}\n";

// Raw VRAM words use RGB5_A1 transport bits; UVs and metadata retain PS1 addressing
static const char hd_texture_fragment[] =
    "flat in uvec4 vTex;flat in vec4 vZBias;\n"
    "uint vramWord(ivec2 p){\n"
    " uvec4 c=uvec4(round(texelFetch(uTex,p&ivec2(1023,511),0)*vec4(31.0,31.0,31.0,1.0)));\n"
    " return (c.r<<11u)|(c.g<<6u)|(c.b<<1u)|c.a;\n"
    "}\n"
    "uint textureWord(){\n"
    " uvec2 uv=uvec2(ivec2(floor(vUV)))&uvec2(255u);\n"
    " uint depth=(vTex.x>>7u)&3u;\n"
    " ivec2 page=ivec2(int(vTex.x&15u)*64,int((vTex.x>>4u)&1u)*256);\n"
    " uint value;\n"
    " if(depth==0u){\n"
    "  uint texel_word=vramWord(page+ivec2(int(uv.x>>2u),int(uv.y)));\n"
    "  value=(texel_word>>((uv.x&3u)*4u))&15u;\n"
    " }else if(depth==1u){\n"
    "  uint texel_word=vramWord(page+ivec2(int(uv.x>>1u),int(uv.y)));\n"
    "  value=(texel_word>>((uv.x&1u)*8u))&255u;\n"
    " }else return vramWord(page+ivec2(uv));\n"
    " return vramWord(ivec2(int((vTex.y&63u)*16u+value),int(vTex.y>>6u)));\n"
    "}\n"
    "void main(){\n"
    " vec4 color=vColor;\n"
    " if((vTex.z&1u)!=0u){\n"
    "  uint word=textureWord(); if(word==0u&&(vTex.z&512u)==0u)discard;\n"
    "  bool stp=(word&32768u)!=0u;\n"
    "  if(((vTex.z&4u)!=0u&&!stp)||((vTex.z&8u)!=0u&&stp))discard;\n"
    "  uvec3 channels=uvec3(word,word>>5u,word>>10u)&uvec3(31u);\n"
    "  vec3 rgb=vec3((channels<<3u)|(channels>>2u));\n"
    "  if((vTex.z&2u)==0u)rgb=min(vec3(255.0),floor(rgb*(vColor.rgb*255.0)/128.0));\n"
    "  color.rgb=rgb/255.0;\n"
    " }\n"
    " if(color.a<=0.0)discard; outColor=vidFog(color);\n"
    " gl_FragDepth=gl_FragCoord.z;\n"
    " if(vZBias.x!=0.0)gl_FragDepth=(vZBias.z+vZBias.w/max(vZBias.y,vDepth+vZBias.x))*0.5+0.5;\n"
    "}\n";

bool hd_camera(const Vec3 *pos, const CameraProjection *projection, float near_z, HdCamera *camera)
{
    float focal = projection->scale / 4096.0f;
    float far_z = projection->far_distance / 32.0f;
    if (near_z <= 0.0f || far_z <= near_z || focal <= 0.0f)
        return false;
    MATRIXF x_basis, y_basis;
    IdentityMatrixf(&x_basis);
    IdentityMatrixf(&y_basis);
    RotMatrixXf(-projection->rotation[0] * RAD12, &x_basis);
    RotMatrixYf(-projection->rotation[1] * RAD12, &y_basis);
    MulMatrix0f(&x_basis, &y_basis, &camera->view);
    RotMatrixZf((2048 - projection->rotation[2]) * RAD12, &camera->view);
    // World positions use 32 units per mesh coordinate; rotation coefficients do not scale translation
    VECTORF position = {pos->x / 32.0f, pos->y / 32.0f, pos->z / 32.0f};
    VECTORF rotated;
    ApplyMatrixf(&camera->view, &position, &rotated);
    camera->view.t[0] = -rotated.vx;
    camera->view.t[1] = -rotated.vy;
    camera->view.t[2] = -rotated.vz;
    camera->focal = focal;
    camera->near_z = near_z;
    camera->far_z = far_z;
    return true;
}

bool hd_visible(const HdCamera *camera, const VECTORF *pos, float radius, float aspect)
{
    if (aspect <= 0.0f || radius < 0.0f)
        return false;
    VECTORF point;
    RotTransf(&camera->view, pos, &point);
    if (point.vz + radius < camera->near_z || point.vz - radius > camera->far_z)
        return false;
    float y_focal = camera->focal / 120.0f;
    float x_focal = y_focal / aspect;
    float x_extent = radius * sqrtf(x_focal * x_focal + 1.0f);
    float y_extent = radius * sqrtf(y_focal * y_focal + 1.0f);
    return fabsf(point.vx * x_focal) <= point.vz + x_extent && fabsf(point.vy * y_focal) <= point.vz + y_extent;
}

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

static VID_SHADER *hd_variant_shader(const char *prefix)
{
    size_t prefix_bytes = strlen(prefix);
    char *source = (char *)malloc(prefix_bytes + sizeof(hd_world_vertex));
    if (source == NULL)
        return NULL;
    memcpy(source, prefix, prefix_bytes);
    memcpy(source + prefix_bytes, hd_world_vertex, sizeof(hd_world_vertex));
    VID_SHADER *shader = vid_shader_create(source, hd_texture_fragment);
    free(source);
    return shader;
}

static bool hd_reserve_ui(uint32 count, uint32 passes)
{
    if (count > hd_state.ui_capacity)
    {
        uint32 capacity = hd_state.ui_capacity != 0 ? hd_state.ui_capacity : 1536;
        while (capacity < count)
        {
            if (capacity > 0x7FFFFFFFu)
                return false;
            capacity *= 2;
        }
        if (capacity > (size_t)-1 / sizeof(VID_VERTEX2D))
            return false;
        VID_VERTEX2D *vertices = (VID_VERTEX2D *)realloc(hd_state.ui_vertices, (size_t)capacity * sizeof(VID_VERTEX2D));
        if (vertices == NULL)
            return false;
        hd_state.ui_vertices = vertices;
        hd_state.ui_capacity = capacity;
    }
    if (passes > hd_state.pass_capacity)
    {
        uint32 capacity = hd_state.pass_capacity != 0 ? hd_state.pass_capacity : 32;
        while (capacity < passes)
        {
            if (capacity > 0x7FFFFFFFu)
                return false;
            capacity *= 2;
        }
        if (capacity > (size_t)-1 / sizeof(HdUiPass))
            return false;
        HdUiPass *commands = (HdUiPass *)realloc(hd_state.ui_passes, (size_t)capacity * sizeof(HdUiPass));
        if (commands == NULL)
            return false;
        hd_state.ui_passes = commands;
        hd_state.pass_capacity = capacity;
    }
    return true;
}

static void hd_upload_vram(void)
{
    uint32 version = gpu_vram_version();
    if (hd_state.vram_valid && version == hd_state.vram_version)
        return;
    memcpy(vid_tex_data(hd_state.vram), VRAM, sizeof(VRAM));
    vid_tex_commit(hd_state.vram);
    hd_state.vram_version = version;
    hd_state.vram_valid = true;
}

static bool hd_og_material_equal(const HdOgMaterial *copy, const TextureDescriptor *texture)
{
    if (texture == NULL)
        return !copy->present;
    if (!copy->present || copy->texture.flags != texture->flags ||
        memcmp(&copy->texture.mapping, &texture->mapping, sizeof(TextureMapping)) != 0)
        return false;
    if ((texture->flags & 0x1400) != 0x1000)
        return true;
    if (texture->src_tex == NULL)
        return copy->texture.src_tex == NULL;
    return copy->texture.src_tex != NULL && copy->source.flags == texture->src_tex->flags &&
        memcmp(&copy->source.mapping, &texture->src_tex->mapping, sizeof(TextureMapping)) == 0;
}

static void hd_og_material_copy(HdOgMaterial *copy, const TextureDescriptor *texture)
{
    memset(copy, 0, sizeof(*copy));
    if (texture == NULL)
        return;
    copy->present = true;
    copy->texture = *texture;
    copy->texture.src_tex = NULL;
    if ((texture->flags & 0x1400) == 0x1000 && texture->src_tex != NULL)
    {
        copy->source = *texture->src_tex;
        copy->source.src_tex = NULL;
        copy->texture.src_tex = &copy->source;
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

static void hd_mesh_free(HdMesh *mesh)
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
    hd_og_geometry_release(mesh->og_geometry);
    free(mesh->og_materials);
    free(mesh);
}

static void hd_free_retired(void)
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

static void hd_reset_sprite_track(const SceneNode *node, const AnimatedObject *owner, const AnimationTrack *track)
{
    for (; node != NULL; node = node->links.sibling_next)
    {
        // Nested actors own their animation bindings
        if (node->kind == 0)
            continue;
        if (node->kind == 7)
        {
            sint32 binding = ((const SpriteRecord *)node)->animation_binding.tracks[0];
            if (binding >= 0 && binding < owner->animation->counts[0] && &owner->animation->tracks[0].vectors[binding].track == track)
                hd_forget_pose(node, false);
        }
        hd_reset_sprite_track(node->links.first_child, owner, track);
    }
}

void hd_sprite_track_loop(const AnimatedObject *owner, const AnimationTrack *track)
{
    if (owner != NULL && owner->animation != NULL)
        hd_reset_sprite_track(owner->node.links.first_child, owner, track);
}

static void hd_clear_identities(void)
{
    if (hd_state.identity_capacity != 0)
        memset(hd_state.identities, 0, hd_state.identity_capacity * sizeof(HdIdentity));
    hd_state.identity_count = 0;
    ++hd_state.pose_epoch;
}

static void hd_og_release(HdOgDraw *draws, uint32 count)
{
    for (uint32 idx = 0; idx < count; ++idx)
    {
        if (draws[idx].kind == HD_OG_BACKGROUND_TILES && --draws[idx].background->refs == 0)
            free(draws[idx].background);
    }
}

static void hd_og_retain(HdOgDraw *draws, uint32 count)
{
    for (uint32 idx = 0; idx < count; ++idx)
        if (draws[idx].kind == HD_OG_BACKGROUND_TILES)
            ++draws[idx].background->refs;
}

static void hd_sample_clear(HdSnapshot *sample)
{
    hd_og_release(sample->og_draws, sample->og_count);
    sample->og_count = 0;
    for (uint32 idx = 0; idx < sample->world_count; ++idx)
        if (sample->world_draws[idx].mesh != NULL)
            --sample->world_draws[idx].mesh->sample_refs;
    sample->world_count = 0;
    sample->ui_count = 0;
    sample->sprite_count = 0;
    sample->pass_count = 0;
    sample->node_count = 0;
    sample->valid = false;
    memset(sample->pool_valid, 0, sizeof(sample->pool_valid));
}

static void hd_clear_samples(void)
{
    hd_clear_identities();
    hd_sample_clear(hd_state.samples);
    hd_sample_clear(hd_state.samples + 1);
    for (uint32 idx = 0; idx < hd_state.pose_capacity; ++idx)
        hd_state.poses[idx].valid = false;
    hd_state.pose_owner = NULL;
    hd_state.pose_active = false;
    hd_state.graph_depth = 0;
    hd_state.path_count = 0;
    hd_free_retired();
}

void hd_sprite_pool(const SpriteRecord *records)
{
    if (records != hd_state.sprite_pool)
        hd_clear_samples();
    hd_state.sprite_pool = records;
    memset(hd_state.pool_valid, 0, sizeof(hd_state.pool_valid));
    memset(hd_state.samples[0].pool_valid, 0, sizeof(hd_state.samples[0].pool_valid));
    memset(hd_state.samples[1].pool_valid, 0, sizeof(hd_state.samples[1].pool_valid));
}

void hd_camera_cut(void)
{
    ++hd_state.camera_epoch;
}

static void *hd_grow_sample(void *data, uint32 *capacity, uint32 count, size_t stride)
{
    if (count <= *capacity)
        return data;
    uint32 grown = *capacity != 0 ? *capacity : 64;
    while (grown < count && grown <= 0x7FFFFFFFu)
        grown *= 2;
    if (grown < count)
        grown = count;
    if (grown > (size_t)-1 / stride)
        return NULL;
    void *result = realloc(data, grown * stride);
    if (result != NULL)
        *capacity = grown;
    return result;
}

static HdOgDraw *hd_og_append(void)
{
    if (!hd_frame.collecting || hd_state.og_count == 0xFFFFFFFFu)
        return NULL;
    HdOgDraw *draws = (HdOgDraw *)hd_grow_sample(hd_state.og_draws, &hd_state.og_capacity, hd_state.og_count + 1, sizeof(HdOgDraw));
    if (draws == NULL)
        return NULL;
    hd_state.og_draws = draws;
    HdOgDraw *draw = draws + hd_state.og_count++;
    memset(draw, 0, sizeof(*draw));
    return draw;
}

bool hd_og_textured_quad(uint32 pos, uint32 extent, const TextureDescriptor *texture, uint32 blend, HdOgQuadEmit emit)
{
    if (texture == NULL || emit == NULL)
        return false;
    HdOgDraw *draw = hd_og_append();
    if (draw == NULL)
        return false;
    draw->kind = HD_OG_QUAD;
    HdOgQuad *quad = &draw->quad;
    quad->pos = pos;
    quad->extent = extent;
    quad->blend = blend;
    quad->texture = *texture;
    // The original UI emitter reads the copied mapping without following src_tex
    quad->texture.src_tex = NULL;
    quad->emit = emit;
    return true;
}

bool hd_og_rect(uint32 pos, uint32 extent, sint32 fraction, uint32 color, HdOgRectEmit emit)
{
    if (emit == NULL)
        return false;
    HdOgDraw *draw = hd_og_append();
    if (draw == NULL)
        return false;
    draw->kind = HD_OG_RECT;
    draw->rect.pos = pos;
    draw->rect.extent = extent;
    draw->rect.fraction = fraction;
    draw->rect.color = color;
    draw->rect.emit = emit;
    return true;
}

bool hd_og_effect(sint32 intensity, HdOgEffectEmit emit)
{
    if (emit == NULL)
        return false;
    HdOgDraw *draw = hd_og_append();
    if (draw == NULL)
        return false;
    draw->kind = HD_OG_EFFECT;
    draw->effect.intensity = intensity;
    draw->effect.emit = emit;
    return true;
}

bool hd_og_scene_fade(uint32 level, HdOgFadeEmit emit)
{
    if (emit == NULL)
        return false;
    HdOgDraw *draw = hd_og_append();
    if (draw == NULL)
        return false;
    draw->kind = HD_OG_SCENE_FADE;
    draw->fade.level = level;
    draw->fade.emit = emit;
    return true;
}

bool hd_og_background_rect(uint32 pos, uint32 extent, uint32 color, HdOgBackgroundRectEmit emit)
{
    if (emit == NULL)
        return false;
    HdOgDraw *draw = hd_og_append();
    if (draw == NULL)
        return false;
    draw->kind = HD_OG_BACKGROUND_RECT;
    draw->background_rect.pos = pos;
    draw->background_rect.extent = extent;
    draw->background_rect.color = color;
    draw->background_rect.emit = emit;
    return true;
}

void hd_og_clear_background(void)
{
    if (!hd_frame.collecting)
        return;
    hd_state.og_background_skipped = false;
    uint32 count = 0;
    for (uint32 idx = 0; idx < hd_state.og_count; ++idx)
    {
        HdOgDraw *draw = hd_state.og_draws + idx;
        if (draw->kind == HD_OG_BACKGROUND_TILES || draw->kind == HD_OG_BACKGROUND_RECT)
            hd_og_release(draw, 1);
        else
            hd_state.og_draws[count++] = *draw;
    }
    hd_state.og_count = count;
}

bool hd_og_skip_background(void)
{
    if (!hd_frame.active || !hd_frame.collecting || hd_frame.failed)
        return false;
    hd_state.og_background_skipped = true;
    return true;
}

bool hd_og_background_pending(void)
{
    return hd_state.og_background_skipped;
}

void hd_og_background_state(void)
{
    if (!hd_state.og_background_skipped)
        return;
    uint32 last = 0xFFFFFFFF, last_mode = 0xFFFFFFFF;
    uint16 page = 0, mode = 0;
    // Reverse OT traversal ends with the earliest insertion in the lowest bucket
    for (uint32 idx = 0; idx < hd_state.og_count; ++idx)
    {
        const HdOgDraw *draw = hd_state.og_draws + idx;
        if (draw->kind != HD_OG_BACKGROUND_TILES)
            continue;
        const HdOgBackground *input = draw->background;
        for (uint32 cell = 0; cell < input->count; ++cell)
        {
            const HdOgBackgroundTile *tile = input->tiles + cell;
            uint32 bucket = tile->tile & 0x1FFF;
            if (bucket < last)
            {
                last = bucket;
                page = tile->definition.attributes;
            }
            if (!(tile->tile & 0xC000) && bucket < last_mode)
            {
                last_mode = bucket;
                mode = tile->definition.attributes;
            }
        }
    }
    if (last_mode != 0xFFFFFFFF)
    {
        DR_MODE packet;
        packet.tag = 0x02FFFFFF;
        packet.code[0] = 0xE1000000 | mode;
        packet.code[1] = 0;
        gpu_packet(&packet);
    }
    if (last != 0xFFFFFFFF)
        gpu_set_polygon_tpage(page);
}

bool hd_og_background_tiles(const BackgroundResource *background, const uint8 *map, sint32 row, sint32 column, sint32 x, uint32 y, sint32 rows, sint32 columns)
{
    if (background == NULL || map == NULL || rows <= 0 || columns <= 0 || row < 0 || column < 0 ||
        background->width == 0 || background->height == 0 || column >= background->width || row >= background->height || rows > background->height - row)
        return false;
    uint64 count = (uint64)rows * columns;
    if (count > 0xFFFFFFFFu || count > ((size_t)-1 - sizeof(HdOgBackground)) / sizeof(HdOgBackgroundTile))
        return false;
    HdOgBackground *input = (HdOgBackground *)malloc(sizeof(HdOgBackground) + (size_t)count * sizeof(HdOgBackgroundTile));
    if (input == NULL)
        return false;
    input->refs = 1;
    input->count = (uint32)count;
    input->tiles = (HdOgBackgroundTile *)(input + 1);
    uint32 cursor = 0;
    for (sint32 line = 0; line < rows; ++line)
    {
        sint32 current = column;
        for (sint32 cell = 0; cell < columns; ++cell)
        {
            if (current == background->width)
                current = 0;
            uint16 tile = ((const uint16 *)map)[(uint32)current * background->height + row + line];
            if ((tile & 0x1FFF) >= background->tile_count)
            {
                free(input);
                return false;
            }
            HdOgBackgroundTile *dst = input->tiles + cursor++;
            dst->tile = tile;
            dst->definition = background->tiles[tile & 0x1FFF];
            dst->xy = (((uint32)x + (uint32)cell * 16) & 0xFFFF) | ((y + (uint32)line * 16) << 16);
            dst->next_xy = (((uint32)x + (uint32)cell * 16 + 16) & 0xFFFF) | ((y + (uint32)line * 16 + 16) << 16);
            ++current;
        }
    }
    HdOgDraw *draw = hd_og_append();
    if (draw == NULL)
    {
        free(input);
        return false;
    }
    draw->kind = HD_OG_BACKGROUND_TILES;
    draw->background = input;
    return true;
}

void hd_og_emit_background_tile(uint32 tile, uint32 xy, uint32 next_xy, const BackgroundTile *definition)
{
    uint32 uv0, uv1, attributes, tmp, pal, packet, dst;
    uint32 ordering = render_commands.scene_ordering_table_offset + (tile & 0x1FFF) * 4;
    uint32 *ot_entry = (uint32 *)psx_addr(ordering + 4, sizeof(uint32));
    if ((tile & 0xC000) != 0)
    {
        uv0 = definition->uv;
        attributes = definition->attributes;
        uv1 = uv0 >> 16;
        if ((tile & 0x4000) != 0)
        {
            tmp = uv1 & 0xFF;
            uv1 = (uv1 & 0xFFFFFF00) | (uv0 & 0xFF);
            uv0 = (uv0 & 0xFFFFFF00) | tmp;
        }
        if ((tile & 0x8000) != 0)
        {
            tmp = uv1 & 0xFF00;
            uv1 = (uv1 & 0xFFFF00FF) | (uv0 & 0xFF00);
            uv0 = (uv0 & 0xFFFF00FF) | tmp;
        }
        uv0 = (uv0 & 0xFFFF) | (attributes & 0xFFFF0000);
        pal = (uv1 & 0xFF00) | (uv0 & 0xFF);
        tmp = *ot_entry;
        packet = render_commands.aux_packet_offset;
        POLY_FT4 *poly = (POLY_FT4 *)psx_addr(packet, sizeof(POLY_FT4));
        setPolyFT4(poly);
        poly->rgbc.rgbc = 0x2D000000;
        poly->tag = (tmp & 0xFFFFFF) | 0x09000000;
        poly->p[1].xy.xy = (next_xy & 0xFFFF) | (xy & 0xFFFF0000);
        poly->p[1].uv_attr = (uv0 & 0xFF00) | (uv1 & 0xFF) | (attributes << 16);
        poly->p[0].xy.xy = xy;
        poly->p[0].uv_attr = uv0;
        poly->p[2].xy.xy = (xy & 0xFFFF) | (next_xy & 0xFFFF0000);
        poly->p[2].uv_attr = pal;
        poly->p[3].xy.xy = next_xy;
        poly->p[3].uv_attr = uv1;
        tmp = *ot_entry;
        dst = packet + 40;
    }
    else
    {
        uv0 = definition->uv;
        attributes = definition->attributes;
        packet = render_commands.aux_packet_offset;
        DR_MODE *mode = (DR_MODE *)psx_addr(packet, sizeof(DR_MODE));
        SPRT_16 *spr = (SPRT_16 *)psx_addr(packet + 12, sizeof(SPRT_16));
        setSprt16(spr);
        mode->tag = ((packet + 12) & 0xFFFFFF) | 0x02000000;
        mode->code[0] = (attributes & 0xFFFF) | 0xE1000000;
        mode->code[1] = 0;
        spr->tag = (*ot_entry & 0xFFFFFF) | 0x03000000;
        spr->rgbc.rgbc = 0x7D000000;
        spr->p[0].xy.xy = xy;
        spr->p[0].uv_attr = (uv0 & 0xFFFF) | (attributes & 0xFFFF0000);
        tmp = *ot_entry;
        dst = packet + 28;
    }
    *ot_entry = (tmp & 0xFF000000) | (packet & 0xFFFFFF);
    render_commands.aux_packet_offset = dst;
}

static bool hd_og_rebuild_background(const HdSnapshot *sample, uint32 base, uint32 limit)
{
    if (!sample->valid || hd_state.frame_active || base >= limit)
        return false;
    uint64 size = 0;
    for (uint32 idx = 0; idx < sample->og_count; ++idx)
    {
        const HdOgDraw *draw = sample->og_draws + idx;
        if (draw->kind == HD_OG_BACKGROUND_RECT)
            size += sizeof(TILE);
        else if (draw->kind == HD_OG_BACKGROUND_TILES)
        {
            const HdOgBackground *input = draw->background;
            for (uint32 cell = 0; cell < input->count; ++cell)
            {
                uint32 tile = input->tiles[cell].tile;
                if ((tile & 0x1FFF) >= 600)
                    return false;
                size += (tile & 0xC000) ? sizeof(POLY_FT4) : sizeof(DR_MODE) + sizeof(SPRT_16);
            }
        }
    }
    // Validate the entire replacement before changing packets or ordering links
    if (size == 0 || size > limit - base)
        return false;
    uint32 table = render_commands.scene_ordering_table_offset;
    uint32 *tail = (uint32 *)psx_addr(table, sizeof(uint32));
    uint32 next = *tail;
    for (uint32 bucket = 1; bucket <= 600; ++bucket)
        *(uint32 *)psx_addr(table + bucket * 4, sizeof(uint32)) = (table + (bucket - 1) * 4) & 0xFFFFFF;
    render_commands.aux_packet_offset = base;
    bool collecting = hd_frame.collecting;
    hd_frame.collecting = false;
    for (uint32 idx = 0; idx < sample->og_count; ++idx)
    {
        const HdOgDraw *draw = sample->og_draws + idx;
        if (draw->kind == HD_OG_BACKGROUND_RECT)
        {
            const HdOgBackgroundRect *rect = &draw->background_rect;
            rect->emit(rect->pos, rect->extent, rect->color);
        }
        else if (draw->kind == HD_OG_BACKGROUND_TILES)
        {
            const HdOgBackground *input = draw->background;
            for (uint32 cell = 0; cell < input->count; ++cell)
            {
                const HdOgBackgroundTile *tile = input->tiles + cell;
                hd_og_emit_background_tile(tile->tile, tile->xy, tile->next_xy, &tile->definition);
            }
        }
    }
    *tail = next;
    hd_frame.collecting = collecting;
    return true;
}

static bool hd_og_replay_ui(const HdSnapshot *sample)
{
    if (!sample->valid || hd_state.frame_active)
        return false;
    bool collecting = hd_frame.collecting;
    hd_frame.collecting = false;
    bool result = true;
    for (uint32 idx = 0; idx < sample->og_count; ++idx)
    {
        const HdOgDraw *draw = sample->og_draws + idx;
        if (draw->kind == HD_OG_BACKGROUND_TILES)
        {
            const HdOgBackground *input = draw->background;
            for (uint32 cell = 0; cell < input->count; ++cell)
            {
                const HdOgBackgroundTile *tile = input->tiles + cell;
                hd_og_emit_background_tile(tile->tile, tile->xy, tile->next_xy, &tile->definition);
            }
            continue;
        }
        if (draw->kind == HD_OG_SCENE_FADE)
        {
            uint32 level = scr_render.fade_level;
            scr_render.fade_level = draw->fade.level;
            draw->fade.emit();
            scr_render.fade_level = level;
            continue;
        }
        if (draw->kind == HD_OG_EFFECT)
        {
            draw->effect.emit(draw->effect.intensity);
            continue;
        }
        if (draw->kind == HD_OG_BACKGROUND_RECT)
        {
            const HdOgBackgroundRect *rect = &draw->background_rect;
            rect->emit(rect->pos, rect->extent, rect->color);
            continue;
        }
        if (draw->kind == HD_OG_RECT)
        {
            const HdOgRect *rect = &draw->rect;
            rect->emit(rect->pos, rect->extent, rect->fraction, rect->color);
            continue;
        }
        const HdOgQuad *quad = &draw->quad;
        if (quad->emit(quad->pos, quad->extent, &quad->texture, quad->blend) == 0)
        {
            result = false;
            break;
        }
    }
    hd_frame.collecting = collecting;
    return result;
}

static void hd_og_save_ui(const HdSnapshot *sample)
{
    hd_og_release(hd_state.frozen_og_draws, hd_state.frozen_og_count);
    hd_state.frozen_og_ui_valid = false;
    hd_state.frozen_og_count = 0;
    if (!sample->valid || sample->restore_image || hd_state.frame_active || sample->collection != hd_state.collection)
        return;
    HdOgDraw *draws = (HdOgDraw *)hd_grow_sample(hd_state.frozen_og_draws, &hd_state.frozen_og_capacity, sample->og_count, sizeof(HdOgDraw));
    if (sample->og_count != 0 && draws == NULL)
        return;
    hd_state.frozen_og_draws = draws;
    hd_state.frozen_og_count = sample->og_count;
    if (sample->og_count != 0)
    {
        memcpy(draws, sample->og_draws, sample->og_count * sizeof(HdOgDraw));
        hd_og_retain(draws, sample->og_count);
    }
    hd_state.frozen_og_ui_valid = true;
}

static bool hd_og_restore_ui(void)
{
    HdSnapshot view;
    memset(&view, 0, sizeof(view));
    view.og_draws = hd_state.frozen_og_draws;
    view.og_count = hd_state.frozen_og_count;
    view.valid = hd_state.frozen_og_ui_valid;
    return hd_og_replay_ui(&view);
}

void hd_scroll_reset(void)
{
    ++hd_state.scroll_epoch;
}

void hd_scroll_update(const TextureDescriptor *texture, uint32 frame, sint32 rate)
{
    HdScrollClock *clock = hd_state.scroll_clocks;
    while (clock != NULL && clock->texture != texture)
        clock = clock->next;
    if (clock == NULL)
    {
        clock = (HdScrollClock *)malloc(sizeof(HdScrollClock));
        if (clock == NULL)
            return;
        clock->texture = texture;
        clock->next = hd_state.scroll_clocks;
        hd_state.scroll_clocks = clock;
    }
    clock->frame = frame;
    clock->rate = rate;
    clock->epoch = hd_state.scroll_epoch;
}

static HdScroll hd_scroll_source(const TextureDescriptor *texture)
{
    HdScroll scroll;
    memset(&scroll, 0, sizeof(scroll));
    if ((texture->flags & 4) == 0 || (texture->flags & 0x1E000) == 0)
        return scroll;
    bool explicit_uv = (texture->flags & 0x1400) == 0x1000;
    const TextureDescriptor *source = explicit_uv ? texture->src_tex : texture;
    if (source == NULL)
        return scroll;
    const HdScrollClock *clock = hd_state.scroll_clocks;
    while (clock != NULL && clock->texture != source)
        clock = clock->next;
    if (clock == NULL)
        return scroll;
    scroll.texture = texture;
    scroll.source = source;
    scroll.mapping = texture->mapping;
    scroll.source_mapping = source->mapping;
    if (!explicit_uv)
        scroll.mapping.scroll_phase = 0;
    scroll.source_mapping.scroll_phase = 0;
    scroll.flags = texture->flags;
    scroll.phase = source->mapping.scroll_phase;
    scroll.frame = clock->frame;
    scroll.rate = clock->rate;
    scroll.epoch = clock->epoch;
    sint64 phase = (sint64)(sint32)clock->frame * clock->rate;
    // Reject a stale clock when a descriptor was changed without its animation owner
    if ((uint32)(phase >> 16) != scroll.phase)
    {
        memset(&scroll, 0, sizeof(scroll));
        return scroll;
    }
    scroll.fraction = (uint16)phase;
    if ((scroll.flags & 0x6000) != 0)
        scroll.period_u = (source->mapping.right - source->mapping.left + 1) >> 1;
    if ((scroll.flags & 0x18000) != 0)
        scroll.period_v = (source->mapping.bottom - source->mapping.top + 1) >> 1;
    return scroll;
}

static bool hd_scroll_pair(const HdScroll *previous, const HdScroll *current)
{
    if (current->texture == NULL || previous->texture != current->texture || previous->source != current->source ||
        previous->flags != current->flags || previous->rate != current->rate || previous->epoch != current->epoch ||
        current->frame < previous->frame || current->frame > 0x7FFFFFFFu ||
        previous->period_u != current->period_u || previous->period_v != current->period_v ||
        memcmp(&previous->mapping, &current->mapping, sizeof(current->mapping)) != 0 ||
        memcmp(&previous->source_mapping, &current->source_mapping, sizeof(current->source_mapping)) != 0)
        return false;
    sint64 old_phase = ((sint64)(sint32)previous->frame * previous->rate) >> 16;
    sint64 phase = ((sint64)(sint32)current->frame * current->rate) >> 16;
    sint64 delta = phase - old_phase;
    return delta >= -(sint64)2147483648u && delta <= 2147483647;
}

static bool hd_capture_sample(HdSnapshot *sample)
{
    if (sample->og_capacity < hd_state.og_count)
    {
        HdOgDraw *quads = (HdOgDraw *)hd_grow_sample(sample->og_draws, &sample->og_capacity, hd_state.og_count, sizeof(HdOgDraw));
        if (quads == NULL)
            return false;
        sample->og_draws = quads;
    }
    if (sample->node_capacity < hd_state.node_count)
    {
        HdPoseSample *nodes = (HdPoseSample *)hd_grow_sample(sample->nodes, &sample->node_capacity, hd_state.node_count, sizeof(HdPoseSample));
        if (nodes == NULL)
            return false;
        sample->nodes = nodes;
    }
    if (sample->world_capacity < hd_state.world_count)
    {
        HdWorldDraw *draws = (HdWorldDraw *)hd_grow_sample(sample->world_draws, &sample->world_capacity, hd_state.world_count, sizeof(HdWorldDraw));
        if (draws == NULL)
            return false;
        sample->world_draws = draws;
    }
    if (sample->ui_capacity < hd_state.ui_count)
    {
        VID_VERTEX2D *vertices = (VID_VERTEX2D *)hd_grow_sample(sample->ui_vertices, &sample->ui_capacity, hd_state.ui_count, sizeof(VID_VERTEX2D));
        if (vertices == NULL)
            return false;
        sample->ui_vertices = vertices;
    }
    if (sample->sprite_capacity < hd_state.sprite_count)
    {
        VID_VERTEX2D *vertices = (VID_VERTEX2D *)hd_grow_sample(sample->sprite_vertices, &sample->sprite_capacity, hd_state.sprite_count, sizeof(VID_VERTEX2D));
        if (vertices == NULL)
            return false;
        sample->sprite_vertices = vertices;
    }
    if (sample->pass_capacity < hd_state.pass_count)
    {
        HdUiPass *passes = (HdUiPass *)hd_grow_sample(sample->ui_passes, &sample->pass_capacity, hd_state.pass_count, sizeof(HdUiPass));
        if (passes == NULL)
            return false;
        sample->ui_passes = passes;
    }
    hd_sample_clear(sample);
    sample->og_count = hd_state.og_count;
    if (sample->og_count != 0)
    {
        memcpy(sample->og_draws, hd_state.og_draws, sample->og_count * sizeof(HdOgDraw));
        hd_og_retain(sample->og_draws, sample->og_count);
    }
    memcpy(sample->pool_valid, hd_state.pool_valid, sizeof(sample->pool_valid));
    sample->node_count = hd_state.node_count;
    if (sample->node_count != 0)
        memcpy(sample->nodes, hd_state.nodes, sample->node_count * sizeof(HdPoseSample));
    sample->camera = hd_frame.camera;
    sample->camera_valid = hd_frame.camera_valid;
    sample->scene = hd_state.scene;
    sample->epoch = hd_state.pose_epoch;
    sample->camera_epoch = hd_state.camera_sample_epoch;
    sample->collection = hd_state.collection;
    sample->clock_index = hd_state.clock_index;
    sample->world_count = hd_state.world_count;
    sample->ui_count = hd_state.ui_count;
    sample->sprite_count = hd_state.sprite_count;
    sample->pass_count = hd_state.pass_count;
    if (sample->world_count != 0)
        memcpy(sample->world_draws, hd_state.world_draws, sample->world_count * sizeof(HdWorldDraw));
    if (sample->ui_count != 0)
        memcpy(sample->ui_vertices, hd_state.ui_vertices, sample->ui_count * sizeof(VID_VERTEX2D));
    if (sample->sprite_count != 0)
        memcpy(sample->sprite_vertices, hd_state.sprite_vertices, sample->sprite_count * sizeof(VID_VERTEX2D));
    if (sample->pass_count != 0)
        memcpy(sample->ui_passes, hd_state.ui_passes, sample->pass_count * sizeof(HdUiPass));
    for (uint32 idx = 0; idx < sample->world_count; ++idx)
        if (sample->world_draws[idx].mesh != NULL)
            ++sample->world_draws[idx].mesh->sample_refs;
    sample->clear_color = hd_state.clear_color;
    sample->vram_version = gpu_vram_version();
    sample->image_version = hd_state.image_version;
    sample->content_version = hd_state.content_version;
    sample->sprite_content_version = hd_state.sprite_content_version;
    sample->ui_half_width = hd_state.ui_half_width;
    sample->restore_image = hd_state.restore_image;
    sample->backdrop = hd_state.backdrop;
    return true;
}

void hd_invalidate_geometry(void)
{
    // Geometry replacement does not end the lifetime of live scene objects
    while (hd_state.meshes != NULL)
    {
        HdMesh *mesh = hd_state.meshes;
        hd_state.meshes = mesh->next;
        if (mesh->queued || mesh->sample_refs != 0)
        {
            mesh->next = hd_state.retired_meshes;
            hd_state.retired_meshes = mesh;
        }
        else
            hd_mesh_free(mesh);
    }
}

void hd_reset(void)
{
    hd_frame.collecting = false;
    if (hd_state.frame_active)
        vid_cancel();
    hd_state.frame_active = false;
    hd_state.pose_owner = NULL;
    hd_state.pose_active = false;
    hd_state.graph_depth = 0;
    hd_state.restore_image = false;
    hd_state.ui_count = 0;
    hd_og_release(hd_state.og_draws, hd_state.og_count);
    hd_state.og_count = 0;
    hd_state.og_background_skipped = false;
    hd_state.pass_count = 0;
    memset(&hd_state.backdrop, 0, sizeof(hd_state.backdrop));
    hd_state.sprite_count = 0;
    for (uint32 idx = 0; idx < hd_state.world_count; ++idx)
        if (hd_state.world_draws[idx].mesh != NULL)
            hd_state.world_draws[idx].mesh->queued = false;
    hd_state.world_count = 0;
    hd_state.node_count = 0;
    hd_free_retired();
    hd_state.vram_valid = false;
}

void hd_level_reset(void)
{
    hd_state.pause_scene = NULL;
    hd_state.pause_redraw = NULL;
    hd_state.pause_valid = false;
    hd_state.paused = false;
    hd_state.pause_collect = false;
    if (hd_state.graph_work_capacity != 0)
        memset(hd_state.graph_work, 0, hd_state.graph_work_capacity * sizeof(HdGraphWork));
    hd_state.graph_work_count = 0;
    hd_reset();
    hd_clear_samples();
    while (hd_state.scroll_clocks != NULL)
    {
        HdScrollClock *clock = hd_state.scroll_clocks;
        hd_state.scroll_clocks = clock->next;
        free(clock);
    }
    hd_scroll_reset();
    hd_invalidate_geometry();
    while (hd_state.pool_ranges != NULL)
    {
        HdPoolRange *range = hd_state.pool_ranges;
        hd_state.pool_ranges = range->next;
        free(range);
    }
    hd_state.world_vertex_count = 0;
    hd_state.world_pool_dirty = true;
}

void hd_free(void)
{
    hd_level_reset();
    vid_mesh_free(hd_state.world_mesh);
    vid_index_free(hd_state.world_indices);
    vid_index_free(hd_state.visible_indices);
    vid_mat_free(hd_state.pooled_material);
    vid_shader_free(hd_state.pooled_shader);
    vid_tex_free(hd_state.world_params);
    vid_tex_free(hd_state.world_groups);
    vid_shader_free(hd_state.world_pool_shader);
    for (uint32 idx = 0; idx < 4; ++idx)
        vid_mat_free(hd_state.world_pool_materials[idx]);
    free(hd_state.visible_draws);
    for (uint32 idx = 0; idx < 4; ++idx)
    {
        if (hd_state.ui_materials[idx] != NULL)
            vid_mat_free(hd_state.ui_materials[idx]);
        if (hd_state.world_materials[idx] != NULL)
            vid_mat_free(hd_state.world_materials[idx]);
        if (hd_state.sprite_materials[idx] != NULL)
            vid_mat_free(hd_state.sprite_materials[idx]);
    }
    if (hd_state.ui_mesh != NULL)
        vid_mesh_free(hd_state.ui_mesh);
    if (hd_state.vram != NULL)
        vid_tex_free(hd_state.vram);
    vid_tex_free(hd_state.empty_morph);
    if (hd_state.ui_shader != NULL)
        vid_shader_free(hd_state.ui_shader);
    if (hd_state.world_shader != NULL)
        vid_shader_free(hd_state.world_shader);
    if (hd_state.sprite_shader != NULL)
        vid_shader_free(hd_state.sprite_shader);
    if (hd_state.sprite_mesh != NULL)
        vid_mesh_free(hd_state.sprite_mesh);
    if (hd_state.image_material != NULL)
        vid_mat_free(hd_state.image_material);
    if (hd_state.image_shader != NULL)
        vid_shader_free(hd_state.image_shader);
    if (hd_state.image_mesh != NULL)
        vid_mesh_free(hd_state.image_mesh);
    vid_target_free(hd_state.live_image);
    vid_target_free(hd_state.saved_image);
    free(hd_state.sprite_vertices);
    free(hd_state.ui_vertices);
    free(hd_state.og_draws);
    hd_og_release(hd_state.frozen_og_draws, hd_state.frozen_og_count);
    free(hd_state.frozen_og_draws);
    free(hd_state.ui_passes);
    free(hd_state.world_draws);
    free(hd_state.poses);
    free(hd_state.paths);
    free(hd_state.nodes);
    free(hd_state.identities);
    free(hd_state.graph_work);
    free(hd_state.mixed_draws);
    free(hd_state.mixed_sprites);
    free(hd_state.mixed_poses);
    free(hd_state.previous_index);
    free(hd_state.current_index);
    for (uint32 idx = 0; idx < 2; ++idx)
    {
        free(hd_state.samples[idx].world_draws);
        free(hd_state.samples[idx].ui_vertices);
        free(hd_state.samples[idx].og_draws);
        free(hd_state.samples[idx].sprite_vertices);
        free(hd_state.samples[idx].ui_passes);
        free(hd_state.samples[idx].nodes);
    }
    memset(&hd_state, 0, sizeof(hd_state));
}

bool hd_init(void)
{
    if (hd_state.ui_shader != NULL)
        return true;
    hd_state.ui_shader = vid_shader_create(hd_ui_vertex, hd_texture_fragment);
    if (hd_state.ui_shader == NULL)
        return false;
    hd_state.world_shader = hd_variant_shader("#define HD_LOCAL_MATERIAL\n");
    if (hd_state.world_shader == NULL)
    {
        hd_free();
        return false;
    }
    hd_state.pooled_shader = hd_variant_shader("#define HD_POOL\n");
    hd_state.world_pool_shader = hd_variant_shader("#define HD_POOL_MATERIAL\n");
    if (hd_state.pooled_shader == NULL || hd_state.world_pool_shader == NULL)
    {
        hd_free();
        return false;
    }
    hd_state.sprite_shader = vid_shader_create(hd_sprite_vertex, hd_texture_fragment);
    if (hd_state.sprite_shader == NULL)
    {
        hd_free();
        return false;
    }
    hd_state.vram = vid_tex_create_fmt(1024, 512, VID_RGBA5551);
    if (hd_state.vram == NULL)
    {
        hd_free();
        return false;
    }
    hd_state.empty_morph = vid_tex_create_fmt(1, 1, VID_RGBA16UI);
    if (hd_state.empty_morph == NULL)
    {
        hd_free();
        return false;
    }
    memset(vid_tex_data(hd_state.empty_morph), 0, 8);
    vid_tex_commit(hd_state.empty_morph);
    for (uint32 idx = 0; idx < 4; ++idx)
    {
        hd_state.ui_materials[idx] = vid_mat_create();
        if (hd_state.ui_materials[idx] == NULL)
        {
            hd_free();
            return false;
        }
        VID_MATERIAL_DATA *material = vid_mat_data(hd_state.ui_materials[idx]);
        material->texture = hd_state.vram;
        material->shader = hd_state.ui_shader;
        material->blend = idx;
        material->depth_test = 0;
        material->depth_write = 0;
        material->filter = 0;
        material->repeat = 0;
        material->cull = 0;
        vid_mat_commit(hd_state.ui_materials[idx]);
        hd_state.world_materials[idx] = vid_mat_create();
        if (hd_state.world_materials[idx] == NULL)
        {
            hd_free();
            return false;
        }
        material = vid_mat_data(hd_state.world_materials[idx]);
        material->aux[0] = hd_state.empty_morph;
        material->texture = hd_state.vram;
        material->shader = hd_state.world_shader;
        material->blend = idx;
        material->depth_test = 1;
        material->depth_write = idx == VID_BLEND_NONE;
        material->filter = 0;
        material->repeat = 0;
        material->cull = 0;
        vid_mat_commit(hd_state.world_materials[idx]);
        hd_state.sprite_materials[idx] = vid_mat_create();
        if (hd_state.sprite_materials[idx] == NULL)
        {
            hd_free();
            return false;
        }
        material = vid_mat_data(hd_state.sprite_materials[idx]);
        *material = *vid_mat_data(hd_state.world_materials[idx]);
        material->shader = hd_state.sprite_shader;
        vid_mat_commit(hd_state.sprite_materials[idx]);
    }
    return true;
}

bool hd_begin(uint32 color)
{
    if (!hd_init())
        return false;
    if (hd_state.frame_active)
        vid_cancel();
    for (uint32 idx = 0; idx < hd_state.world_count; ++idx)
        if (hd_state.world_draws[idx].mesh != NULL)
            hd_state.world_draws[idx].mesh->queued = false;
    hd_state.world_count = 0;
    hd_state.node_count = 0;
    hd_state.scene = NULL;
    hd_state.draw_pose = HD_NO_POSE;
    memset(hd_state.pool_valid, 1, sizeof(hd_state.pool_valid));
    hd_state.pose_owner = NULL;
    hd_state.pose_active = false;
    hd_state.graph_depth = 0;
    hd_free_retired();
    hd_state.sprite_count = 0;
    sint32 width, height;
    vid_resize(&width, &height);
    hd_state.ui_half_width = height > 0 ? 120.0f * width / height : 160.0f;
    hd_state.ui_count = 0;
    hd_og_release(hd_state.og_draws, hd_state.og_count);
    hd_state.og_count = 0;
    hd_state.pass_count = 0;
    hd_state.og_background_skipped = false;
    memset(&hd_state.backdrop, 0, sizeof(hd_state.backdrop));
    hd_state.restore_image = false;
    hd_state.clear_color = color;
    hd_state.content_version = ++hd_state.next_content_version;
    hd_state.sprite_content_version = hd_state.content_version;
    hd_state.frame_active = vid_begin((color & 255) / 255.0f, ((color >> 8) & 255) / 255.0f, ((color >> 16) & 255) / 255.0f) != 0;
    return hd_state.frame_active;
}

static bool hd_append_ui(const VID_VERTEX2D *vertices, uint32 count, uint32 blend, HdUiLayer layer)
{
    if (!hd_state.frame_active || vertices == NULL || count % 3 != 0 || blend > VID_BLEND_SUBTRACT || layer < HD_BACKGROUND || layer >= HD_UI_LAYERS)
        return false;
    if (count == 0)
        return true;
    if (count > 0xFFFFFFFFu - hd_state.ui_count || hd_state.pass_count == 0xFFFFFFFFu)
        return false;
    if (!hd_reserve_ui(hd_state.ui_count + count, hd_state.pass_count + 1))
        return false;
    memcpy(hd_state.ui_vertices + hd_state.ui_count, vertices, (size_t)count * sizeof(VID_VERTEX2D));
    HdUiPass *pass;
    if (hd_state.pass_count != 0 && hd_state.ui_passes[hd_state.pass_count - 1].blend == blend && hd_state.ui_passes[hd_state.pass_count - 1].layer == layer)
        pass = hd_state.ui_passes + hd_state.pass_count - 1;
    else
    {
        pass = hd_state.ui_passes + hd_state.pass_count++;
        pass->first = hd_state.ui_count;
        pass->count = 0;
        pass->blend = blend;
        pass->layer = layer;
    }
    pass->count += count;
    hd_state.ui_count += count;
    return true;
}

bool hd_ui(const VID_VERTEX2D *vertices, uint32 count, uint32 blend)
{
    return hd_append_ui(vertices, count, blend, HD_OVERLAY);
}

static void hd_ui_corners(VID_VERTEX2D *vertices, float x, float y, float width, float height, uint32 color, HdUiAnchor anchor)
{
    static const uint8 order[6] = {0, 1, 2, 2, 1, 3};
    static const uint16 flags[4] = {HD_UI_COORDS, HD_UI_COORDS | HD_UI_LEFT, HD_UI_COORDS | HD_UI_RIGHT, HD_UI_COORDS | HD_UI_FILL};
    memset(vertices, 0, 6 * sizeof(VID_VERTEX2D));
    for (uint32 idx = 0; idx < 6; ++idx)
    {
        VID_VERTEX2D *vertex = vertices + idx;
        uint32 corner = order[idx];
        vertex->pos[0] = x + ((corner & 1) != 0 ? width : 0.0f);
        vertex->pos[1] = y + ((corner & 2) != 0 ? height : 0.0f);
        vertex->pos[3] = 1.0f;
        vertex->alpha = 1.0f;
        vertex->color[0] = color;
        vertex->color[1] = color >> 8;
        vertex->color[2] = color >> 16;
        vertex->tex[2] = flags[anchor];
    }
}

bool hd_textured_quad(uint32 pos, uint32 extent, const TextureDescriptor *texture, uint32 blend, HdUiAnchor anchor, HdUiLayer layer)
{
    if (texture == NULL || blend > 4 || anchor < HD_ANCHOR_CENTER || anchor > HD_ANCHOR_FILL)
        return false;
    const TextureMapping *mapping = &texture->mapping;
    uint32 width = extent & 0xFFFF;
    uint32 height = extent >> 16;
    float left = mapping->left;
    float top = mapping->top;
    float right = mapping->right;
    float bottom = mapping->bottom;
    if (width == 0)
    {
        width = mapping->right - mapping->left + 1;
        right += 1.0f;
    }
    if (height == 0)
    {
        height = mapping->bottom - mapping->top + 1;
        bottom += 1.0f;
    }
    VID_VERTEX2D vertices[6];
    static const uint8 order[6] = {0, 1, 2, 2, 1, 3};
    hd_ui_corners(vertices, (sint16)pos, (sint16)(pos >> 16), width, height, 0x808080, anchor);
    for (uint32 idx = 0; idx < 6; ++idx)
    {
        vertices[idx].uv[0] = (order[idx] & 1) != 0 ? right : left;
        vertices[idx].uv[1] = (order[idx] & 2) != 0 ? bottom : top;
        vertices[idx].tex[0] = mapping->tpage;
        vertices[idx].tex[1] = mapping->clut;
        vertices[idx].tex[2] |= HD_TEXTURED | HD_RAW_TEXTURE;
        if (blend != 0)
            vertices[idx].tex[2] |= HD_OPAQUE_ONLY;
    }
    if (!hd_append_ui(vertices, 6, VID_BLEND_NONE, layer))
        return false;
    if (blend == 0)
        return true;
    static const uint32 blends[4] = {VID_BLEND_ALPHA, VID_BLEND_ADD, VID_BLEND_SUBTRACT, VID_BLEND_ADD};
    static const float alpha[4] = {0.5f, 1.0f, 1.0f, 0.25f};
    for (uint32 idx = 0; idx < 6; ++idx)
    {
        vertices[idx].tex[2] = (vertices[idx].tex[2] & ~HD_OPAQUE_ONLY) | HD_STP_ONLY;
        vertices[idx].alpha = alpha[blend - 1];
    }
    return hd_append_ui(vertices, 6, blends[blend - 1], layer);
}

bool hd_rect(float x, float y, float width, float height, uint32 color, uint32 blend, HdUiAnchor anchor, HdUiLayer layer)
{
    if (anchor < HD_ANCHOR_CENTER || anchor > HD_ANCHOR_FILL)
        return false;
    VID_VERTEX2D vertices[6];
    hd_ui_corners(vertices, x, y, width, height, color, anchor);
    return hd_append_ui(vertices, 6, blend, layer);
}

bool hd_vram_image(const PSX_RECT *source)
{
    if (source == NULL || source->x < 0 || source->y < 0 || source->w <= 0 || source->h <= 0 ||
        source->x + source->w > 1024 || source->y + source->h > 512)
        return false;
    static const uint8 order[6] = {0, 1, 2, 2, 1, 3};
    sint32 top = 0;
    while (top < source->h)
    {
        sint32 v = (source->y + top) & 255;
        sint32 height = source->h - top;
        if (height > 256 - v)
            height = 256 - v;
        sint32 left = 0;
        while (left < source->w)
        {
            sint32 x = source->x + left;
            sint32 u = x & 63;
            sint32 width = source->w - left;
            if (width > 256 - u)
                width = 256 - u;
            VID_VERTEX2D vertices[6];
            hd_ui_corners(vertices, left, top, width, height, 0x808080, HD_ANCHOR_CENTER);
            for (uint32 idx = 0; idx < 6; ++idx)
            {
                vertices[idx].uv[0] = u + ((order[idx] & 1) != 0 ? width : 0);
                vertices[idx].uv[1] = v + ((order[idx] & 2) != 0 ? height : 0);
                vertices[idx].tex[0] = (x >> 6) | (((source->y + top) >> 8) << 4) | (2 << 7);
                vertices[idx].tex[2] |= HD_TEXTURED | HD_RAW_TEXTURE | HD_IMAGE_OPAQUE;
            }
            if (!hd_append_ui(vertices, 6, VID_BLEND_NONE, HD_BACKGROUND))
                return false;
            left += width;
        }
        top += height;
    }
    return true;
}

static float hd_angle_delta(float from, float to)
{
    float angle = fmodf(to - from + 2048.0f, 4096.0f);
    if (angle < 0.0f)
        angle += 4096.0f;
    return angle - 2048.0f;
}

bool hd_background(const BackgroundResource *background, const uint16 *map, sint32 pitch, sint32 yaw, sint32 pitch_offset, uint32 scroll, uint32 top_color, uint32 bottom_color)
{
    if (!hd_state.frame_active)
        return false;
    if (background == NULL)
        return hd_rect(0, 0, 320, 240, top_color, VID_BLEND_NONE, HD_ANCHOR_FILL, HD_BACKGROUND);
    if (background->width == 0 || background->height == 0 || map == NULL || background->tiles == NULL)
        return false;
    HdBackdrop *backdrop = &hd_state.backdrop;
    backdrop->source = background;
    backdrop->pitch = pitch & 4095;
    backdrop->yaw = yaw & 4095;
    backdrop->width = background->width;
    float margin_x = 16.0f, margin_y = 16.0f;
    const HdSnapshot *previous = hd_state.samples + hd_state.latest_sample;
    if (previous->valid && previous->backdrop.source == background)
    {
        margin_x += fabsf(hd_angle_delta(previous->backdrop.yaw, backdrop->yaw)) * background->width / 256.0f;
        margin_y += fabsf(hd_angle_delta(previous->backdrop.pitch, backdrop->pitch)) * 640.0f / 2048.0f;
    }
    float angle = (float)((1024 + pitch + pitch_offset) & 4095);
    if (angle > 2048.0f)
        angle -= 4096.0f;
    float vertical = 640.0f * angle / 2048.0f;
    float top = ((40 - (sint32)background->height) >> 1) * 16.0f - vertical;
    float bottom = top + background->height * 16.0f;
    if (top >= -margin_y && !hd_rect(0, -margin_y, 320, top + margin_y, top_color, VID_BLEND_NONE, HD_ANCHOR_FILL, HD_BACKGROUND))
        return false;
    if (bottom < 240.0f + margin_y && !hd_rect(0, bottom, 320, 240.0f + margin_y - bottom, bottom_color, VID_BLEND_NONE, HD_ANCHOR_FILL, HD_BACKGROUND))
        return false;
    float horizontal = ((-yaw) & 4095) * background->width / 256.0f;
    sint32 column = (sint32)floorf(horizontal / 16.0f) + scroll;
    float x = -fmodf(horizontal, 16.0f);
    sint32 extra = (sint32)floorf((160.0f - hd_state.ui_half_width - margin_x - x) / 16.0f);
    column += extra;
    x += extra * 16.0f;
    column %= background->width;
    if (column < 0)
        column += background->width;
    sint32 row = (sint32)floorf((-margin_y - top) / 16.0f);
    if (row < 0)
        row = 0;
    float y = top + row * 16.0f;
    for (; row < background->height && y < 240.0f + margin_y; ++row, y += 16.0f)
    {
        sint32 current = column;
        for (float left = x; left < 160.0f + hd_state.ui_half_width + margin_x; left += 16.0f)
        {
            uint32 tile = map[current * background->height + row];
            if ((tile & 8191) >= background->tile_count)
                return false;
            const BackgroundTile *source = background->tiles + (tile & 8191);
            float u0 = source->uv & 255, v0 = source->uv >> 8 & 255;
            float u1 = u0 + 16.0f, v1 = v0 + 16.0f;
            if ((tile & 0xC000) != 0)
            {
                u1 = (source->uv >> 16 & 255) + 1.0f;
                v1 = (source->uv >> 24) + 1.0f;
                if ((tile & 0x4000) != 0)
                {
                    float swap = u0;
                    u0 = u1;
                    u1 = swap;
                }
                if ((tile & 0x8000) != 0)
                {
                    float swap = v0;
                    v0 = v1;
                    v1 = swap;
                }
            }
            VID_VERTEX2D vertices[6];
            static const uint8 order[6] = {0, 1, 2, 2, 1, 3};
            hd_ui_corners(vertices, left, y, 16, 16, 0x808080, HD_ANCHOR_CENTER);
            for (uint32 idx = 0; idx < 6; ++idx)
            {
                vertices[idx].uv[0] = (order[idx] & 1) != 0 ? u1 : u0;
                vertices[idx].uv[1] = (order[idx] & 2) != 0 ? v1 : v0;
                vertices[idx].tex[0] = source->attributes;
                vertices[idx].tex[1] = source->attributes >> 16;
                vertices[idx].tex[2] |= HD_TEXTURED | HD_RAW_TEXTURE;
            }
            if (!hd_append_ui(vertices, 6, VID_BLEND_NONE, HD_BACKGROUND))
                return false;
            if (++current == background->width)
                current = 0;
        }
    }
    return true;
}

bool hd_texture(const TextureDescriptor *texture, HdTexture *result)
{
    uint32 flags = texture->flags;
    const TextureMapping *mapping = &texture->mapping;
    uint32 uv0, uv1, uv2, uv3;
    if ((flags & 0x1400) == 0x1000)
    {
        if (texture->src_tex == NULL)
            return false;
        uv0 = mapping->uv01;
        uv1 = uv0 >> 16;
        uv2 = mapping->uv32 >> 16;
        uv3 = mapping->uv32 & 0xFFFF;
        mapping = &texture->src_tex->mapping;
        uint32 width = (sint32)(mapping->right - mapping->left + 1) >> 1;
        uint32 height = (sint32)(mapping->bottom - mapping->top + 1) >> 1;
        uint32 u = 0, v = 0;
        if ((flags & 0x6000) != 0)
        {
            if (width == 0)
                return false;
            uint32 remainder = mapping->scroll_phase % width;
            u = (flags & 0x2000) != 0 ? remainder : width - remainder;
        }
        if ((flags & 0x18000) != 0)
        {
            if (height == 0)
                return false;
            uint32 remainder = mapping->scroll_phase % height;
            v = (flags & 0x8000) != 0 ? remainder : height - remainder;
        }
        uint32 offset = u | (v << 8);
        uv0 += offset;
        uv1 += offset;
        uv2 += offset;
        uv3 += offset;
    }
    else
    {
        uv0 = mapping->uv_rect;
        if ((flags & 0x1E000) != 0)
        {
            uint32 left = mapping->left, top = mapping->top;
            uint32 width = mapping->right - left + 1;
            uint32 height = mapping->bottom - top + 1;
            if ((flags & 0x6000) != 0)
            {
                width >>= 1;
                if (width == 0)
                    return false;
                uint32 remainder = mapping->scroll_phase % width;
                left += (flags & 0x2000) != 0 ? remainder : width - remainder;
            }
            if ((flags & 0x18000) != 0)
            {
                height >>= 1;
                if (height == 0)
                    return false;
                uint32 remainder = mapping->scroll_phase % height;
                top += (flags & 0x8000) != 0 ? remainder : height - remainder;
            }
            uv0 = left | (top << 8) | ((left + width - 1) << 16) | ((top + height - 1) << 24);
        }
        uv3 = uv0 >> 16;
        if ((flags & 8) != 0)
        {
            uint32 right = uv3 & 255;
            uv3 = (uv3 & 0xFFFFFF00) | (uv0 & 255);
            uv0 = (uv0 & 0xFFFFFF00) | right;
        }
        if ((flags & 16) != 0)
        {
            uint32 bottom = uv3 & 0xFF00;
            uv3 = (uv3 & 0xFFFF00FF) | (uv0 & 0xFF00);
            uv0 = (uv0 & 0xFFFF00FF) | bottom;
        }
        uv1 = (uv0 & 0xFF00) | (uv3 & 255);
        uv2 = (uv3 & 0xFF00) | (uv0 & 255);
    }
    result->uv[0].u = uv0;
    result->uv[0].v = uv0 >> 8;
    result->uv[1].u = uv1;
    result->uv[1].v = uv1 >> 8;
    result->uv[2].u = uv2;
    result->uv[2].v = uv2 >> 8;
    result->uv[3].u = uv3;
    result->uv[3].v = uv3 >> 8;
    result->page = mapping->tpage | (flags & 0x60) | (uv1 >> 16);
    result->clut = mapping->clut;
    return true;
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
            vertex.tex[2] = textured ? HD_TEXTURED : 0;
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
    if (mesh->geometry != NULL && mesh->indices != NULL && mesh->passes != NULL && mesh->staging_passes != NULL && mesh->source_indices != NULL && mesh->scrolls != NULL && hd_mesh_update(mesh))
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
    hd_og_geometry_release(mesh->og_geometry);
    free(mesh->og_materials);
    free(mesh);
    return NULL;
}

static HdWorldDraw *hd_new_world_draw(void)
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

static void hd_source_clip(const SceneSection *scene, HdCamera *camera)
{
    // A reversed original interval rejects everything beyond fog_near without depth cue
    if (scene != NULL && scene->fog_far != 0 && scene->fog_far <= scene->fog_near)
        camera->far_z = fminf(camera->far_z, scene->fog_near / 32.0f);
}

static const VID_FOG *hd_source_fog(const SceneSection *scene, const HdCamera *camera, VID_FOG *fog)
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

static void hd_select_lights(const SceneLighting *lighting, const VECTORF *origin, float radius, SceneLighting *selected)
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

static void hd_draw_lights(uint32 first, const SceneLighting *source, const MATRIXF *model)
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

static bool hd_sprite_pass(const HdCamera *camera, const VID_FOG *fog, uint32 first, uint32 blend, uint32 mask, float alpha, const HdLighting *lighting, const VECTORF *normal)
{
    HdWorldDraw source;
    memset(&source, 0, sizeof(source));
    source.pose = hd_state.draw_pose;
    source.first = first;
    source.count = 6;
    source.blend = blend;
    for (uint32 row = 0; row < 3; ++row)
    {
        memcpy(source.uniforms + row * 4, camera->view.m[row], sizeof(camera->view.m[row]));
        source.uniforms[row * 4 + 3] = camera->view.t[row];
    }
    source.uniforms[12] = camera->focal / hd_state.ui_half_width;
    source.uniforms[13] = camera->focal / 120.0f;
    source.uniforms[14] = (camera->far_z + camera->near_z) / (camera->far_z - camera->near_z);
    source.uniforms[15] = -2.0f * camera->far_z * camera->near_z / (camera->far_z - camera->near_z);
    source.uniforms[16] = alpha;
    source.uniforms[17] = mask;
    source.uniforms[19 * 4 + 1] = camera->near_z;
    if (lighting != NULL)
    {
        memcpy(source.uniforms + 5 * 4, normal, sizeof(*normal));
        source.uniforms[5 * 4 + 3] = 1.0f;
        for (uint32 idx = 0; idx < 3; ++idx)
        {
            const HdLight *light = lighting->lights + idx;
            memcpy(source.uniforms + (6 + idx) * 4, &light->dir, sizeof(light->dir));
            source.uniforms[(6 + idx) * 4 + 3] = light->point ? 1.0f : 0.0f;
            memcpy(source.uniforms + (9 + idx) * 4, &light->color, sizeof(light->color));
            source.uniforms[(9 + idx) * 4 + 3] = light->radius;
            memcpy(source.uniforms + (12 + idx) * 4, &light->pos, sizeof(light->pos));
            source.uniforms[(12 + idx) * 4 + 3] = light->attenuation_range;
            memcpy(source.uniforms + (15 + idx) * 4, &light->color_scale, sizeof(light->color_scale));
        }
        memcpy(source.uniforms + 18 * 4, &lighting->ambient, sizeof(lighting->ambient));
    }
    if (fog != NULL)
        source.fog = *fog;
    if (hd_state.world_count != 0)
    {
        HdWorldDraw *prev = hd_state.world_draws + hd_state.world_count - 1;
        if (source.pose == HD_NO_POSE && prev->mesh == NULL && prev->pose == source.pose && prev->first + prev->count == first && prev->blend == blend &&
            memcmp(prev->uniforms, source.uniforms, sizeof(source.uniforms)) == 0 && memcmp(&prev->fog, &source.fog, sizeof(source.fog)) == 0)
        {
            prev->count += 6;
            return true;
        }
    }
    HdWorldDraw *draw = hd_new_world_draw();
    if (draw == NULL)
        return false;
    *draw = source;
    return true;
}

bool hd_sprite(const HdSprite *sprite, const TextureDescriptor *texture, const HdCamera *camera, const VID_FOG *fog, const HdLighting *lighting, const VECTORF *normal)
{
    if (!hd_state.frame_active || sprite == NULL || texture == NULL || camera == NULL || camera->near_z <= 0 || camera->far_z <= camera->near_z ||
        sprite->mode < HD_BILLBOARD || sprite->mode > HD_FACING_SPRITE || hd_state.sprite_count > 0xFFFFFFFFu - 6 || (lighting != NULL && normal == NULL))
        return false;
    if (fog != NULL && !(fog->near_dist > 0 && fog->far_dist > fog->near_dist && fog->horizon_dist > fog->far_dist && fog->focal[0] > 0 && fog->focal[1] > 0))
        return false;
    uint32 count = hd_state.sprite_count + 6;
    if (count > hd_state.sprite_capacity)
    {
        uint32 capacity = hd_state.sprite_capacity != 0 ? hd_state.sprite_capacity * 2 : 1536;
        if (capacity < count || capacity > (size_t)-1 / sizeof(VID_VERTEX2D))
            return false;
        VID_VERTEX2D *vertices = (VID_VERTEX2D *)realloc(hd_state.sprite_vertices, capacity * sizeof(VID_VERTEX2D));
        if (vertices == NULL)
            return false;
        hd_state.sprite_vertices = vertices;
        hd_state.sprite_capacity = capacity;
    }
    const TextureMapping *mapping = &texture->mapping;
    static const uint8 order[6] = {0, 1, 2, 2, 1, 3};
    static const uint8 corners[4][4] = {{0,1,2,3},{2,0,3,1},{3,2,1,0},{1,3,0,2}};
    for (uint32 idx = 0; idx < 6; ++idx)
    {
        uint32 corner = sprite->mode != HD_GROUND_SPRITE ? corners[sprite->orientation & 3][order[idx]] : order[idx];
        if (sprite->mode == HD_GROUND_SPRITE)
            corner = ((corner & 1) << 1) | ((corner & 2) >> 1);
        VID_VERTEX2D *vertex = hd_state.sprite_vertices + hd_state.sprite_count + idx;
        memset(vertex, 0, sizeof(*vertex));
        memcpy(vertex->pos, &sprite->pos, sizeof(sprite->pos));
        vertex->pos[3] = (corner & 1) != 0 ? sprite->width : -sprite->width;
        vertex->alpha = (corner & 2) != 0 ? sprite->height : -sprite->height;
        if (sprite->mode == HD_BILLBOARD)
            vertex->alpha = -vertex->alpha;
        vertex->uv[0] = (order[idx] & 1) != 0 ? mapping->right : mapping->left;
        vertex->uv[1] = (order[idx] & 2) != 0 ? mapping->bottom : mapping->top;
        memcpy(vertex->color, &mapping->color, sizeof(vertex->color));
        vertex->color[3] = texture->flags >> 24;
        vertex->tex[0] = mapping->tpage | ((texture->flags & 0x80) != 0 ? texture->flags & 0x60 : 0);
        vertex->tex[1] = mapping->clut;
        vertex->tex[2] = HD_TEXTURED | ((mapping->color.code & 1) != 0 ? HD_RAW_TEXTURE : 0);
        vertex->tex[3] = sprite->pitch & 4095;
        vertex->rect[0] = sprite->yaw & 4095;
        vertex->rect[1] = sprite->mode;
    }
    uint32 first = hd_state.sprite_count;
    hd_state.sprite_count = count;
    if ((mapping->color.code & 2) == 0)
        return hd_sprite_pass(camera, fog, first, VID_BLEND_NONE, 0, 1.0f, lighting, normal);
    uint32 mode = (hd_state.sprite_vertices[first].tex[0] >> 5) & 3;
    static const uint32 blends[4] = {VID_BLEND_ALPHA, VID_BLEND_ADD, VID_BLEND_SUBTRACT, VID_BLEND_ADD};
    static const float alpha[4] = {0.5f, 1.0f, 1.0f, 0.25f};
    return hd_sprite_pass(camera, fog, first, VID_BLEND_NONE, HD_OPAQUE_ONLY, 1.0f, lighting, normal) &&
        hd_sprite_pass(camera, fog, first, blends[mode], HD_STP_ONLY, alpha[mode], lighting, normal);
}

bool hd_sprite_record(const SpriteRecord *source, const MATRIXF *model, const HdCamera *camera, float scale, const TextureDescriptor *material, const VID_FOG *fog, const SceneLighting *lighting)
{
    HdSprite sprite;
    memset(&sprite, 0, sizeof(sprite));
    sprite.pos.vx = model->t[0];
    sprite.pos.vy = model->t[1];
    sprite.pos.vz = model->t[2];
    sprite.width = source->extent[0];
    sprite.height = source->extent[1];
    uint32 flags = source->node.state_flags;
    const TextureDescriptor *texture = source->tex;
    if ((flags & 0x200) != 0)
    {
        sprite.mode = HD_GROUND_SPRITE;
        sprite.pitch = source->angles[0];
        sprite.yaw = source->angles[1];
    }
    else
    {
        VECTORF offset = {(sint16)source->angles[0] * sprite.width / 128.0f, (sint16)source->angles[1] * sprite.height / 128.0f, 0.0f};
        MATRIXF inverse;
        TransposeMatrixf(&camera->view, &inverse);
        ApplyMatrixf(&inverse, &offset, &offset);
        sprite.pos.vx += offset.vx;
        sprite.pos.vy += offset.vy;
        sprite.pos.vz += offset.vz;
        sprite.orientation = (flags & 0x180) >> 7;
        if ((flags & 4) != 0)
        {
            sprite.mode = HD_FACING_SPRITE;
            sprite.height += 10.0f;
        }
        else
        {
            sprite.mode = HD_BILLBOARD;
            if (scale != 0.0f)
            {
                sprite.width *= scale;
                sprite.height *= scale;
            }
            if (material != NULL)
            {
                texture = material;
                sprite.orientation = 0;
            }
        }
    }
    if ((flags & 0x205) == 0)
    {
        if (lighting == NULL)
            return false;
        VECTORF origin = {model->t[0], model->t[1], model->t[2]};
        float radius = sprite.width > sprite.height ? sprite.width : sprite.height;
        SceneLighting selected;
        hd_select_lights(lighting, &origin, radius, &selected);
        HdLighting lights;
        hd_lighting(&selected, model, NULL, &lights);
        VECTORF normal = {-camera->view.m[2][0], -camera->view.m[2][1], -camera->view.m[2][2]};
        MATRIXF inverse;
        TransposeMatrixf(model, &inverse);
        ApplyMatrixf(&inverse, &normal, &normal);
        uint32 first = hd_state.world_count;
        bool result = hd_sprite(&sprite, texture, camera, fog, &lights, &normal);
        if (result)
            hd_draw_lights(first, &selected, model);
        return result;
    }
    return hd_sprite(&sprite, texture, camera, fog);
}

bool hd_billboard_node(const BillboardNode *source, const VECTORF *pos, const HdCamera *camera, const VID_FOG *fog)
{
    HdSprite sprite;
    memset(&sprite, 0, sizeof(sprite));
    sprite.pos = *pos;
    sprite.width = source->extent / 32.0f;
    sprite.height = sprite.width;
    sprite.mode = HD_BILLBOARD;
    return hd_sprite(&sprite, source->tex, camera, fog);
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
        !hd_world_table(&hd_state.world_groups, &hd_state.world_group_height, material_faces * 7))
        return false;
    if (hd_state.pooled_material == NULL)
    {
        hd_state.pooled_material = vid_mat_create();
        if (hd_state.pooled_material == NULL)
            return false;
    }
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
            uint16 *metadata = groups + (size_t)(visible->material_first + face) * 28;
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

struct HdWorldPass
{
    VID_MATERIAL *material;
    VID_FOG fog;
};

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

static bool hd_submit_world(void)
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

bool hd_end(void)
{
    if (!hd_state.frame_active)
        return false;
    hd_upload_vram();
    if (hd_state.sprite_count != 0)
    {
        if (hd_state.sprite_mesh_capacity < hd_state.sprite_count)
        {
            VID_GEOMETRY *mesh = vid_mesh_create_2d(hd_state.sprite_capacity);
            if (mesh == NULL)
            {
                hd_reset();
                return false;
            }
            if (hd_state.sprite_mesh != NULL)
                vid_mesh_free(hd_state.sprite_mesh);
            hd_state.sprite_mesh = mesh;
            hd_state.sprite_mesh_capacity = hd_state.sprite_capacity;
        }
        if (hd_state.sprite_version != hd_state.sprite_content_version)
        {
            memcpy(vid_mesh_data_2d(hd_state.sprite_mesh), hd_state.sprite_vertices, hd_state.sprite_count * sizeof(VID_VERTEX2D));
            vid_mesh_commit(hd_state.sprite_mesh);
            hd_state.sprite_version = hd_state.sprite_content_version;
        }
    }
    if (hd_state.ui_count != 0)
    {
        if (hd_state.mesh_capacity < hd_state.ui_count)
        {
            VID_GEOMETRY *mesh = vid_mesh_create_2d(hd_state.ui_capacity);
            if (mesh == NULL)
            {
                hd_reset();
                return false;
            }
            if (hd_state.ui_mesh != NULL)
                vid_mesh_free(hd_state.ui_mesh);
            hd_state.ui_mesh = mesh;
            hd_state.mesh_capacity = hd_state.ui_capacity;
        }
        if (hd_state.ui_version != hd_state.content_version)
        {
            memcpy(vid_mesh_data_2d(hd_state.ui_mesh), hd_state.ui_vertices, (size_t)hd_state.ui_count * sizeof(VID_VERTEX2D));
            vid_mesh_commit(hd_state.ui_mesh);
            hd_state.ui_version = hd_state.content_version;
        }
    }
    for (uint32 layer = HD_BACKGROUND; layer < HD_UI_LAYERS; ++layer)
    {
        float layout[4] = {hd_state.ui_half_width, 0.0f, 0.0f, 0.0f};
        if (layer == HD_BACKGROUND)
        {
            layout[1] = hd_state.backdrop.offset[0];
            layout[2] = hd_state.backdrop.offset[1];
        }
        vid_uniforms(layout, 1);
        for (uint32 idx = 0; idx < hd_state.pass_count; ++idx)
        {
            const HdUiPass *pass = hd_state.ui_passes + idx;
            if (pass->layer != layer)
                continue;
            vid_pass(hd_state.ui_materials[pass->blend]);
            if (!vid_submit(hd_state.ui_mesh, NULL, pass->first, pass->count))
            {
                hd_reset();
                return false;
            }
        }
        if (layer == HD_BACKGROUND && hd_state.restore_image)
        {
            vid_uniforms(NULL, 0);
            if (!vid_draw(hd_state.image_mesh, 0, 6, hd_state.image_material))
            {
                hd_reset();
                return false;
            }
        }
        if (layer == HD_BACKGROUND && !hd_submit_world())
        {
            hd_reset();
            return false;
        }
    }
    hd_state.frame_active = false;
    bool result = vid_end() != 0;
    for (uint32 idx = 0; idx < hd_state.world_count; ++idx)
        if (hd_state.world_draws[idx].mesh != NULL)
            hd_state.world_draws[idx].mesh->queued = false;
    hd_state.world_count = 0;
    hd_free_retired();
    return result;
}

void hd_frame_begin(void)
{
    bool requested = xport_hd != 0;
    if (requested != hd_state.last_hd)
        hd_clear_samples();
    hd_frame.camera_valid = false;
    hd_frame.failed = false;
    hd_frame.collecting = hd_begin(0);
    hd_frame.active = requested && hd_frame.collecting;
    hd_state.restore_image = false;
    if (hd_frame.collecting)
    {
        ++hd_state.collection;
        hd_state.clock_index = game_field & ~(uint64)1;
    }
    if (requested && !hd_frame.active)
        fprintf(stderr, "HD frame initialization failed\n");
    hd_frame.returning_og = hd_state.last_hd && !hd_frame.active;
}

static void hd_work_view(HdSnapshot *view)
{
    view->og_draws = hd_state.og_draws;
    view->og_count = hd_state.og_count;
    view->og_capacity = hd_state.og_capacity;
    view->world_draws = hd_state.world_draws;
    view->world_count = hd_state.world_count;
    view->world_capacity = hd_state.world_capacity;
    view->ui_vertices = hd_state.ui_vertices;
    view->ui_count = hd_state.ui_count;
    view->ui_capacity = hd_state.ui_capacity;
    view->sprite_vertices = hd_state.sprite_vertices;
    view->sprite_count = hd_state.sprite_count;
    view->sprite_capacity = hd_state.sprite_capacity;
    view->ui_passes = hd_state.ui_passes;
    view->pass_count = hd_state.pass_count;
    view->pass_capacity = hd_state.pass_capacity;
    view->clear_color = hd_state.clear_color;
    view->content_version = hd_state.content_version;
    view->sprite_content_version = hd_state.sprite_content_version;
    view->ui_half_width = hd_state.ui_half_width;
    view->restore_image = hd_state.restore_image;
    view->backdrop = hd_state.backdrop;
}

static void hd_use_sample(const HdSnapshot *view)
{
    // OG sources remain owned by the collector while HD consumes a borrowed view
    hd_state.world_draws = view->world_draws;
    hd_state.world_count = view->world_count;
    hd_state.world_capacity = view->world_capacity;
    hd_state.ui_vertices = view->ui_vertices;
    hd_state.ui_count = view->ui_count;
    hd_state.ui_capacity = view->ui_capacity;
    hd_state.sprite_vertices = view->sprite_vertices;
    hd_state.sprite_count = view->sprite_count;
    hd_state.sprite_capacity = view->sprite_capacity;
    hd_state.ui_passes = view->ui_passes;
    hd_state.pass_count = view->pass_count;
    hd_state.pass_capacity = view->pass_capacity;
    hd_state.clear_color = view->clear_color;
    hd_state.content_version = view->content_version;
    hd_state.sprite_content_version = view->sprite_content_version;
    hd_state.ui_half_width = view->ui_half_width;
    hd_state.restore_image = view->restore_image;
    hd_state.backdrop = view->backdrop;
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

static bool hd_mix_poses(const HdSnapshot *previous, const HdSnapshot *current, float fract)
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
        mixed->model = node->world;
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

static HdCamera hd_mix_camera(const HdCamera *previous, const HdCamera *current, float fract)
{
    if (fract <= 0.0f)
        return *previous;
    if (fract >= 1.0f)
        return *current;
    HdCamera camera = *current;
    MATRIXF old_world, new_world, world;
    TransposeMatrixf(&previous->view, &old_world);
    TransposeMatrixf(&current->view, &new_world);
    QUATF old_rotation, new_rotation, rotation;
    MatrixQuatf(&old_world, &old_rotation);
    MatrixQuatf(&new_world, &new_rotation);
    SlerpQuatf(&old_rotation, &new_rotation, fract, &rotation);
    QuatMatrixf(&rotation, &world);
    TransposeMatrixf(&world, &camera.view);
    VECTORF old_pos = {-previous->view.t[0], -previous->view.t[1], -previous->view.t[2]};
    VECTORF new_pos = {-current->view.t[0], -current->view.t[1], -current->view.t[2]};
    ApplyMatrixf(&old_world, &old_pos, &old_pos);
    ApplyMatrixf(&new_world, &new_pos, &new_pos);
    VECTORF pos = {old_pos.vx + (new_pos.vx - old_pos.vx) * fract,
        old_pos.vy + (new_pos.vy - old_pos.vy) * fract, old_pos.vz + (new_pos.vz - old_pos.vz) * fract};
    ApplyMatrixf(&camera.view, &pos, &pos);
    camera.view.t[0] = -pos.vx;
    camera.view.t[1] = -pos.vy;
    camera.view.t[2] = -pos.vz;
    return camera;
}

static void hd_mix_draw(HdWorldDraw *draw, const MATRIXF *model, const HdCamera *camera)
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

static bool hd_prepare_morph(HdMesh *mesh)
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

static HdMesh *hd_previous_source(const HdSnapshot *previous, const HdWorldDraw *draw, uint32 pose)
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

static HdMesh *hd_previous_mesh(const HdSnapshot *previous, const HdWorldDraw *draw, uint32 pose)
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

static const HdWorldDraw *hd_previous_sprite(const HdSnapshot *previous, const HdWorldDraw *draw, uint32 pose)
{
    const HdWorldDraw *match = NULL;
    for (uint32 idx = 0; idx < previous->world_count; ++idx)
    {
        const HdWorldDraw *candidate = previous->world_draws + idx;
        if (candidate->mesh == NULL && candidate->pose == pose && candidate->count == draw->count &&
            candidate->blend == draw->blend && candidate->uniforms[17] == draw->uniforms[17])
        {
            if (match != NULL)
                return NULL;
            match = candidate;
        }
    }
    return match;
}

static bool hd_mix_sprites(const HdSnapshot *previous, const HdSnapshot *current, float fract, HdSnapshot *view)
{
    if (current->sprite_count == 0)
        return true;
    if (hd_state.mixed_sprite_capacity < current->sprite_count)
    {
        VID_VERTEX2D *vertices = (VID_VERTEX2D *)hd_grow_sample(hd_state.mixed_sprites, &hd_state.mixed_sprite_capacity, current->sprite_count, sizeof(VID_VERTEX2D));
        if (vertices == NULL)
            return false;
        hd_state.mixed_sprites = vertices;
    }
    memcpy(hd_state.mixed_sprites, current->sprite_vertices, current->sprite_count * sizeof(VID_VERTEX2D));
    for (uint32 idx = 0; idx < current->world_count; ++idx)
    {
        const HdWorldDraw *draw = current->world_draws + idx;
        if (draw->mesh != NULL || draw->pose >= current->node_count || draw->first > current->sprite_count || draw->count > current->sprite_count - draw->first)
            continue;
        const HdMixedPose *pose = hd_state.mixed_poses + draw->pose;
        const HdWorldDraw *old = pose->previous != HD_NO_POSE ? hd_previous_sprite(previous, draw, pose->previous) : NULL;
        if (old != NULL && (old->first > previous->sprite_count || old->count > previous->sprite_count - old->first))
            old = NULL;
        if (old != NULL && draw->count != 0)
        {
            const VID_VERTEX2D *src = current->sprite_vertices + draw->first;
            const VID_VERTEX2D *prev = previous->sprite_vertices + old->first;
            HdWorldDraw *mixed = view->world_draws + idx;
            if (prev->rect[1] == HD_GROUND_SPRITE && src->rect[1] == HD_GROUND_SPRITE)
            {
                // Fractional angular offsets preserve the compact source vertex layout
                mixed->uniforms[80] = -hd_angle_delta(prev->tex[3], src->tex[3]) * (1.0f - fract);
                mixed->uniforms[81] = -hd_angle_delta(prev->rect[0], src->rect[0]) * (1.0f - fract);
            }
            if (prev->rect[1] == src->rect[1] && prev->tex[2] == src->tex[2] && old->uniforms[23] == draw->uniforms[23])
            {
                mixed->uniforms[82] = (prev->color[0] - src->color[0]) * (1.0f - fract);
                mixed->uniforms[83] = (prev->color[1] - src->color[1]) * (1.0f - fract);
                mixed->uniforms[78] = (prev->color[2] - src->color[2]) * (1.0f - fract);
                mixed->uniforms[79] = (prev->color[3] - src->color[3]) * (1.0f - fract);
            }
        }
        // STP passes share vertices but retain their own interpolation uniforms
        if (idx != 0 && current->world_draws[idx - 1].mesh == NULL && current->world_draws[idx - 1].first == draw->first)
            continue;
        for (uint32 vertex = 0; vertex < draw->count; ++vertex)
        {
            VID_VERTEX2D *dst = hd_state.mixed_sprites + draw->first + vertex;
            const VID_VERTEX2D *src = current->sprite_vertices + draw->first + vertex;
            const VID_VERTEX2D *prev = old != NULL ? previous->sprite_vertices + old->first + vertex : NULL;
            for (uint32 axis = 0; axis < 3; ++axis)
            {
                float offset = src->pos[axis] - current->nodes[draw->pose].world.t[axis];
                if (prev != NULL)
                {
                    float old_offset = prev->pos[axis] - previous->nodes[pose->previous].world.t[axis];
                    offset = old_offset + (offset - old_offset) * fract;
                }
                dst->pos[axis] = pose->model.t[axis] + offset;
            }
            if (prev != NULL && memcmp(prev->tex, src->tex, 3 * sizeof(src->tex[0])) == 0 &&
                prev->rect[1] == src->rect[1] && memcmp(prev->uv, src->uv, sizeof(src->uv)) == 0)
            {
                dst->pos[3] = prev->pos[3] + (src->pos[3] - prev->pos[3]) * fract;
                dst->alpha = prev->alpha + (src->alpha - prev->alpha) * fract;
            }
        }
    }
    if (memcmp(hd_state.mixed_sprites, current->sprite_vertices, current->sprite_count * sizeof(VID_VERTEX2D)) != 0)
    {
        view->sprite_vertices = hd_state.mixed_sprites;
        view->sprite_capacity = hd_state.mixed_sprite_capacity;
        view->sprite_content_version = ++hd_state.next_content_version;
    }
    return true;
}

static bool hd_mixed_view(const HdSnapshot *previous, const HdSnapshot *current, float fract, HdSnapshot *view)
{
    *view = *current;
    if (fract >= 1.0f)
        return true;
    if (!hd_mix_poses(previous, current, fract))
        return false;
    if (hd_state.mixed_draw_capacity < current->world_count)
    {
        HdWorldDraw *draws = (HdWorldDraw *)hd_grow_sample(hd_state.mixed_draws, &hd_state.mixed_draw_capacity, current->world_count, sizeof(HdWorldDraw));
        if (draws == NULL)
            return false;
        hd_state.mixed_draws = draws;
    }
    view->world_draws = hd_state.mixed_draws;
    view->world_capacity = hd_state.mixed_draw_capacity;
    if (current->world_count != 0)
        memcpy(view->world_draws, current->world_draws, current->world_count * sizeof(HdWorldDraw));
    if (current->backdrop.source != NULL && previous->backdrop.source == current->backdrop.source)
    {
        view->backdrop.offset[0] = -hd_angle_delta(previous->backdrop.yaw, current->backdrop.yaw) * current->backdrop.width / 256.0f * (1.0f - fract);
        view->backdrop.offset[1] = hd_angle_delta(previous->backdrop.pitch, current->backdrop.pitch) * 640.0f / 2048.0f * (1.0f - fract);
    }
    HdCamera camera = hd_mix_camera(&previous->camera, &current->camera, fract);
    for (uint32 idx = 0; idx < current->world_count; ++idx)
    {
        HdWorldDraw *draw = view->world_draws + idx;
        const MATRIXF *model = draw->pose < current->node_count ? &hd_state.mixed_poses[draw->pose].model : &draw->model;
        hd_mix_draw(draw, model, &camera);
        if (draw->mesh != NULL)
        {
            uint32 old_pose = draw->pose < current->node_count ? hd_state.mixed_poses[draw->pose].previous : HD_NO_POSE;
            if (draw->pose == HD_NO_POSE || old_pose != HD_NO_POSE)
            {
                draw->scroll = hd_previous_source(previous, draw, old_pose);
                draw->morph = hd_previous_mesh(previous, draw, old_pose);
            }
            draw->scroll_fract = fract;
            if (draw->morph != NULL)
            {
                if (!hd_prepare_morph(draw->morph))
                    return false;
                draw->uniforms[70] = fract;
                draw->uniforms[71] = draw->morph->morph_width;
            }
        }
    }
    return hd_mix_sprites(previous, current, fract, view);
}

static bool hd_render_sample(const HdSnapshot *sample, const HdSnapshot *previous)
{
    HdSnapshot view = *sample;
    bool paired = previous->valid && previous->collection + 1 == sample->collection &&
        previous->epoch == sample->epoch && sample->epoch == hd_state.pose_epoch && previous->scene == sample->scene &&
        previous->camera_epoch == sample->camera_epoch && sample->camera_epoch == hd_state.camera_epoch &&
        previous->camera_valid && sample->camera_valid && !previous->restore_image && !sample->restore_image && !hd_state.paused &&
        previous->ui_half_width == sample->ui_half_width &&
        previous->camera.focal == sample->camera.focal && previous->camera.near_z == sample->camera.near_z && previous->camera.far_z == sample->camera.far_z;
    uint64 field;
    float fract = xport_frame_fract(&field);
    paired = paired && previous->clock_index + 2 == sample->clock_index && sample->clock_index == (field & ~(uint64)1);
    float phase = paired ? ((field & 1) + fract) * 0.5f : 1.0f;
    if (paired && !hd_mixed_view(previous, sample, phase, &view))
        return false;
    HdSnapshot work;
    hd_work_view(&work);
    hd_use_sample(&view);
    for (uint32 idx = 0; idx < view.world_count; ++idx)
        if (view.world_draws[idx].mesh != NULL)
            view.world_draws[idx].mesh->queued = true;
    bool result = hd_end();
    if (result)
        hd_state.live_collection = sample->collection;
    hd_use_sample(&work);
    return result;
}

bool hd_frame_end(void)
{
    if (hd_state.pause_collect && hd_frame.active)
        return hd_frame.collecting && !hd_frame.failed;
    bool active = hd_frame.active;
    if (!hd_frame.collecting)
    {
        hd_state.last_hd = false;
        return true;
    }
    hd_frame.active = false;
    hd_frame.collecting = false;
    uint32 incoming = 1 - hd_state.latest_sample;
    HdSnapshot *sample = hd_state.samples + incoming;
    bool captured = !hd_frame.failed && hd_capture_sample(sample);
    bool result = captured;
    if (active)
        result = captured && hd_render_sample(sample, hd_state.samples + hd_state.latest_sample);
    else
    {
        // Keep the collected source available for OG-to-HD transitions
        vid_cancel();
        hd_state.frame_active = false;
        for (uint32 idx = 0; idx < hd_state.world_count; ++idx)
            if (hd_state.world_draws[idx].mesh != NULL)
                hd_state.world_draws[idx].mesh->queued = false;
    }
    if (result)
    {
        sample->valid = true;
        hd_state.latest_sample = incoming;
        hd_state.world_count = 0;
    }
    else if (captured)
        hd_sample_clear(sample);
    hd_state.last_hd = active && result;
    if (!result)
    {
        hd_reset();
        if (active)
            fprintf(stderr, "HD frame collection or submission failed\n");
    }
    hd_free_retired();
    if (active && result)
        xport_render_wait_until(~(uint64)0);
    return active ? result : true;
}

static const HdSnapshot *hd_ready_sample(void)
{
    const HdSnapshot *sample = hd_state.samples + hd_state.latest_sample;
    if (hd_frame.active || hd_state.frame_active || !hd_state.last_hd || !sample->valid ||
        sample->vram_version != gpu_vram_version() ||
        (sample->restore_image && (sample->image_version != hd_state.image_version || !hd_state.image_valid)))
        return NULL;
    return sample;
}

bool hd_replay(void)
{
    const HdSnapshot *sample = hd_ready_sample();
    if (sample == NULL)
        return false;
    sint32 width, height;
    vid_resize(&width, &height);
    if (hd_state.paused && hd_state.pause_redraw != NULL &&
        (!hd_state.pause_valid || !xport_hd || width != hd_state.pause_width || height != hd_state.pause_height))
        return false;
    float half_width = height > 0 ? 120.0f * width / height : 160.0f;
    if (half_width != sample->ui_half_width || !hd_begin(sample->clear_color))
        return false;
    bool result = hd_render_sample(sample, hd_state.samples + 1 - hd_state.latest_sample);
    if (result)
        xport_render_wait_until(~(uint64)0);
    return result;
}

void hd_wait(uint64 deadline)
{
    for (;;)
    {
        psx_native_poll_events();
        if (CurrentField() >= deadline)
            break;
        if (hd_ready_sample() != NULL)
            hd_replay();
    }
}

void hd_save_image(void)
{
    const HdSnapshot *completed = hd_state.samples + hd_state.latest_sample;
    if (hd_state.last_hd && completed->valid && completed->collection == hd_state.collection &&
        !hd_state.frame_active && render_commands.active_buffer < 2)
    {
        const RenderBuffer *buffer = render_buffers + render_commands.active_buffer;
        if (render_commands.scene_ordering_table_offset == buffer->scene_ordering_table_offset &&
            render_commands.submitted_chain_offset == buffer->scene_ordering_table_offset + 2400)
            hd_og_rebuild_background(completed, buffer->aux_packet_base, buffer->scene_ordering_table_offset);
    }
    hd_og_save_ui(hd_state.samples + hd_state.latest_sample);
    ++hd_state.image_version;
    hd_state.image_valid = false;
    if (hd_state.paused)
        return;
    {
        const HdSnapshot *sample = hd_state.samples + hd_state.latest_sample;
        if (!sample->valid || sample->collection != hd_state.collection || sample->vram_version != gpu_vram_version() || hd_state.frame_active)
            return;
        if (hd_state.live_image == NULL)
            hd_state.live_image = vid_target_create();
        if (hd_state.live_image == NULL || !hd_begin(sample->clear_color))
            return;
        if (!vid_target_select_offscreen(hd_state.live_image) || !hd_render_sample(sample, sample))
        {
            hd_reset();
            return;
        }
    }
    if (hd_state.live_image == NULL)
        return;
    if (hd_state.saved_image == NULL)
        hd_state.saved_image = vid_target_create();
    if (hd_state.saved_image == NULL || !vid_target_copy(hd_state.saved_image, hd_state.live_image))
        return;
    if (hd_state.image_mesh == NULL)
    {
        hd_state.image_mesh = vid_mesh_create(6);
        if (hd_state.image_mesh == NULL)
            return;
        static const uint8 corners[6] = {0, 1, 2, 2, 1, 3};
        VID_VERTEX *vertices = vid_mesh_data(hd_state.image_mesh);
        for (uint32 idx = 0; idx < 6; ++idx)
        {
            uint8 corner = corners[idx];
            vertices[idx].pos[0] = corner & 1 ? 1.0f : -1.0f;
            vertices[idx].pos[1] = corner & 2 ? -1.0f : 1.0f;
            vertices[idx].pos[3] = 1.0f;
            vertices[idx].uv[0] = corner & 1 ? 1.0f : 0.0f;
            vertices[idx].uv[1] = corner & 2 ? 0.0f : 1.0f;
            for (uint32 channel = 0; channel < 4; ++channel)
                vertices[idx].color[channel] = 1.0f;
        }
        vid_mesh_commit(hd_state.image_mesh);
    }
    if (hd_state.image_material == NULL)
    {
        if (hd_state.image_shader == NULL)
            hd_state.image_shader = vid_shader_create(NULL, "void main(){outColor=vec4(texture(uTex,vUV).rgb,1.0);}\n");
        if (hd_state.image_shader == NULL)
            return;
        hd_state.image_material = vid_mat_create();
        if (hd_state.image_material == NULL)
            return;
        vid_mat_data(hd_state.image_material)->texture = vid_target_texture(hd_state.saved_image);
        // Display captures stay opaque after additive or subtractive overlays
        vid_mat_data(hd_state.image_material)->shader = hd_state.image_shader;
        vid_mat_commit(hd_state.image_material);
    }
    hd_state.image_valid = true;
}

void hd_restore_image(const PSX_RECT *source)
{
    if (hd_frame.collecting && hd_state.image_valid)
        hd_state.restore_image = true;
    else if (hd_frame.collecting && source != NULL && !hd_vram_image(source))
        hd_frame.failed = true;
}

void hd_menu_image_changed(void)
{
    hd_og_release(hd_state.frozen_og_draws, hd_state.frozen_og_count);
    hd_state.frozen_og_ui_valid = false;
    hd_state.frozen_og_count = 0;
    ++hd_state.image_version;
    hd_state.image_valid = false;
}

static void hd_drop_image(void)
{
    vid_mat_free(hd_state.image_material);
    vid_shader_free(hd_state.image_shader);
    vid_mesh_free(hd_state.image_mesh);
    vid_target_free(hd_state.saved_image);
    vid_target_free(hd_state.live_image);
    hd_state.image_material = NULL;
    hd_state.image_shader = NULL;
    hd_state.image_mesh = NULL;
    hd_state.saved_image = NULL;
    hd_state.live_image = NULL;
    hd_state.image_valid = false;
    ++hd_state.image_version;
}

void hd_pause_begin(SceneSection *scene, HdSceneRedraw redraw)
{
    hd_clear_samples();
    hd_drop_image();
    hd_state.paused = true;
    hd_state.pause_scene = scene;
    hd_state.pause_redraw = redraw;
    hd_state.pause_valid = false;
    hd_state.pause_collect = false;
}

void hd_pause_end(void)
{
    hd_state.paused = false;
    hd_state.pause_scene = NULL;
    hd_state.pause_redraw = NULL;
    hd_state.pause_valid = false;
    hd_state.pause_collect = false;
    // Keep the displayed frame available for the exit fade after scene teardown
    hd_clear_identities();
    for (uint32 idx = 0; idx < hd_state.pose_capacity; ++idx)
        hd_state.poses[idx].valid = false;
    hd_state.pose_owner = NULL;
    hd_state.pose_active = false;
    hd_state.graph_depth = 0;
    hd_state.path_count = 0;
}

static bool hd_rebuild_paused_scene(sint32 width, sint32 height)
{
    bool requested = xport_hd != 0;
    hd_state.image_valid = false;
    ++hd_state.image_version;
    hd_state.pause_collect = requested;
    hd_frame.capture_only = true;
    hd_state.pause_redraw(hd_state.pause_scene, false);
    hd_frame.capture_only = false;
    hd_state.pause_collect = false;
    DrawSync(0);
    render_commands.pending_disp_env = NULL;
    render_commands.submitted_disp_env = NULL;
    if (requested)
    {
        if (!hd_frame.active || !hd_frame.collecting || hd_frame.failed)
            return false;
    }
    hd_state.pause_hd = requested;
    hd_state.pause_width = width;
    hd_state.pause_height = height;
    hd_state.pause_valid = true;
    return true;
}

void hd_overlay_begin(void)
{
    bool scene_collected = false;
    if (hd_state.paused && hd_state.pause_redraw != NULL)
    {
        sint32 width, height;
        vid_resize(&width, &height);
        bool requested = xport_hd != 0;
        if (width > 0 && height > 0 && (requested || !hd_state.pause_valid || requested != hd_state.pause_hd))
        {
            hd_state.pause_valid = false;
            if (!hd_rebuild_paused_scene(width, height))
                fprintf(stderr, "Paused scene reconstruction failed\n");
            else
                scene_collected = requested;
        }
    }
    if (!scene_collected)
        hd_frame_begin();
    if (hd_frame.collecting)
    {
        // Scene redraw selected its packet arena; the overlay uses the other arena
        render_commands.packet_limit = render_commands.other_packet_base + 41600;
        gpu_frame_begin();
        gpu_set_raster(!hd_frame.active);
        if (!scene_collected)
            hd_restore_image();
    }
}

void hd_frame_camera(const Vec3 *pos, const CameraProjection *projection, const SceneSection *scene)
{
    if (!hd_frame.collecting || hd_frame.failed)
        return;
    hd_frame.camera_valid = hd_camera(pos, projection, 1.0f, &hd_frame.camera);
    hd_frame.aspect = hd_state.ui_half_width / 120.0f;
    if (hd_frame.camera_valid)
        hd_source_clip(scene, &hd_frame.camera);
    hd_state.camera_sample_epoch = hd_state.camera_epoch;
    hd_state.scene = scene;
    if (!hd_frame.camera_valid)
        hd_frame.failed = true;
}

static void hd_source_model(const MATRIX *source, MATRIXF *model)
{
    for (uint32 row = 0; row < 3; ++row)
    {
        for (uint32 column = 0; column < 3; ++column)
            model->m[row][column] = source->m[row][column] / 4096.0f;
        model->t[row] = source->t[row] / 32.0f;
    }
}

static void hd_quat_pose(const sint16 *rotation, MATRIXF *model)
{
    QUATF quat;
    quat.w = rotation[0] / 16384.0f;
    quat.x = -rotation[1] / 16384.0f;
    quat.y = -rotation[2] / 16384.0f;
    quat.z = -rotation[3] / 16384.0f;
    float norm = quat.w * quat.w + quat.x * quat.x + quat.y * quat.y + quat.z * quat.z;
    QuatMatrixf(&quat, model);
    // Preserve the source polynomial for nonunit Q14 samples
    for (uint32 row = 0; row < 3; ++row)
        for (uint32 column = 0; column < 3; ++column)
            model->m[row][column] = model->m[row][column] * norm + (row == column ? 1.0f - norm : 0.0f);
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
    VECTORF pos = {actor->pos.x / 32.0f, actor->pos.y / 32.0f, actor->pos.z / 32.0f};
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
        hd_quat_pose(actor->orientation, model);
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
            model->t[0] = part->pos.x / 32.0f;
            model->t[1] = part->pos.y / 32.0f;
            model->t[2] = part->pos.z / 32.0f;
        }
        else
        {
            if (part->rotation[0] != 0x4000)
                hd_quat_pose(part->rotation, &local);
            local.t[0] = part->pos.x / 32.0f;
            local.t[1] = part->pos.y / 32.0f;
            local.t[2] = part->pos.z / 32.0f;
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
        VECTORF offset = {pos->x / 32.0f, pos->y / 32.0f, pos->z / 32.0f};
        RotTransf(parent, &offset, &offset);
        TransMatrixf(model, &offset);
    }
}

static bool hd_reserve_poses(uint32 depth)
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
    sample->world = pose->model;
    if (rotation != NULL)
    {
        QUATF source = {-rotation[1] / 16384.0f, -rotation[2] / 16384.0f, -rotation[3] / 16384.0f, rotation[0] / 16384.0f};
        NormalizeQuatf(&source, &sample->rotation);
    }
    else
        MatrixQuatf(local, &sample->rotation);
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
        sample->world = pose->model;
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
            if (part->rotation[0] != 0x4000)
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
            hd_quat_pose(actor->orientation, model);
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
        if (rotation[0] != 0x4000)
            hd_quat_pose(rotation, &local);
        MulMatrix0f(parent, &local, model);
    }
    if (pos != NULL)
    {
        VECTORF offset = {pos->x / 32.0f, pos->y / 32.0f, pos->z / 32.0f};
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
        if (actor->behavior == 22 || actor->orientation[0] != 0x4000)
            rotation = actor->orientation;
    }
    else if (node->kind == 5)
    {
        space = HD_POSE_PARENT_POS | HD_POSE_PARENT_ROT;
        const MeshPartNode *part = (const MeshPartNode *)node;
        if (part->rotation[0] != 0x4000)
            rotation = part->rotation;
    }
    else if (node->kind == 2)
    {
        space = HD_POSE_PARENT_POS | (heading == NULL ? HD_POSE_PARENT_ROT : 0);
        const SceneTrigger *trigger = (const SceneTrigger *)node;
        if (heading == NULL && trigger->rotation[0] != 0x4000)
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

static void hd_pose_camera_scratch(const MATRIXF *model)
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

void hd_frame_sprite(const SpriteRecord *sprite, float scale, const TextureDescriptor *material, const SceneLighting *lighting, const MATRIX *scratch, float depth_bias)
{
    if (!hd_frame.collecting || hd_frame.failed)
        return;
    bool native_pose = hd_state.pose_active && hd_state.poses[hd_state.pose_depth].node == &sprite->node;
    if (!native_pose)
    {
        hd_frame.failed = true;
        return;
    }
    MATRIXF model = hd_state.poses[hd_state.pose_depth].model;
    hd_state.draw_pose = hd_state.poses[hd_state.pose_depth].sample;
    uint32 first = hd_state.world_count;
    VID_FOG source_fog;
    const VID_FOG *fog = hd_source_fog(hd_state.scene, &hd_frame.camera, &source_fog);
    if (!hd_frame.camera_valid || !hd_sprite_record(sprite, &model, &hd_frame.camera, scale, material, fog, lighting))
        hd_frame.failed = true;
    for (uint32 idx = first; idx < hd_state.world_count; ++idx)
        hd_state.world_draws[idx].uniforms[76] = depth_bias;
    hd_state.draw_pose = HD_NO_POSE;
    if (!native_pose || hd_frame.failed)
        return;
    uint32 depth = hd_state.pose_depth + 1;
    if (!hd_reserve_poses(depth))
    {
        hd_frame.failed = true;
        return;
    }
    HdPose *pose = hd_state.poses + depth;
    if (!pose->valid)
        hd_source_model(scratch, &pose->model);
    VECTORF pos = {model.t[0], model.t[1], model.t[2]};
    RotTransf(&hd_frame.camera.view, &pos, &pos);
    TransMatrixf(&pose->model, &pos);
    if ((sprite->node.state_flags & 0x200) != 0)
    {
        MATRIXF rotation;
        IdentityMatrixf(&rotation);
        RotMatrixXf((sprite->angles[0] & 4095) * RAD12, &rotation);
        RotMatrixYf((sprite->angles[1] & 4095) * RAD12, &rotation);
        MulMatrix0f(&hd_frame.camera.view, &rotation, &pose->model);
    }
    else
    {
        pose->model.t[0] += (sint16)sprite->angles[0] * sprite->extent[0] / 128.0f;
        pose->model.t[1] += (sint16)sprite->angles[1] * sprite->extent[1] / 128.0f;
        if ((sprite->node.state_flags & 4) != 0)
        {
            MATRIXF inverse, rotation;
            TransposeMatrixf(&hd_frame.camera.view, &inverse);
            pos.vx = pose->model.t[0];
            pos.vy = pose->model.t[1];
            pos.vz = pose->model.t[2];
            ApplyMatrixf(&inverse, &pos, &pos);
            IdentityMatrixf(&rotation);
            RotMatrixYf(atan2f(pos.vz, pos.vx), &rotation);
            MulMatrix0f(&hd_frame.camera.view, &rotation, &pose->model);
        }
    }
    pose->valid = true;
}

void hd_frame_billboard(const BillboardNode *billboard, const MATRIX *view, float depth_bias)
{
    if (!hd_frame.collecting || hd_frame.failed)
        return;
    if (!hd_frame.camera_valid)
    {
        hd_frame.failed = true;
        return;
    }
    bool native_pose = hd_state.pose_active && hd_state.poses[hd_state.pose_depth].node == &billboard->node;
    VECTORF pos;
    if (native_pose)
    {
        const MATRIXF *model = &hd_state.poses[hd_state.pose_depth].model;
        pos.vx = model->t[0];
        pos.vy = model->t[1];
        pos.vz = model->t[2];
    }
    else
    {
        pos.vx = view->t[0] / 32.0f - hd_frame.camera.view.t[0];
        pos.vy = view->t[1] / 32.0f - hd_frame.camera.view.t[1];
        pos.vz = view->t[2] / 32.0f - hd_frame.camera.view.t[2];
        MATRIXF inverse;
        TransposeMatrixf(&hd_frame.camera.view, &inverse);
        ApplyMatrixf(&inverse, &pos, &pos);
    }
    hd_state.draw_pose = native_pose ? hd_state.poses[hd_state.pose_depth].sample : HD_NO_POSE;
    uint32 first = hd_state.world_count;
    VID_FOG source_fog;
    const VID_FOG *fog = hd_source_fog(hd_state.scene, &hd_frame.camera, &source_fog);
    if (!hd_frame.camera_valid || !hd_billboard_node(billboard, &pos, &hd_frame.camera, fog))
        hd_frame.failed = true;
    for (uint32 idx = first; idx < hd_state.world_count; ++idx)
        hd_state.world_draws[idx].uniforms[76] = depth_bias;
    hd_state.draw_pose = HD_NO_POSE;
    if (native_pose && !hd_frame.failed)
    {
        RotTransf(&hd_frame.camera.view, &pos, &pos);
        TransMatrixf(&hd_state.poses[hd_state.pose_depth].model, &pos);
    }
}

void hd_frame_rect(uint32 pos, uint32 extent, uint32 color, uint32 blend, HdUiLayer layer)
{
    if (!hd_frame.collecting || hd_frame.failed)
        return;
    HdUiAnchor anchor = (extent & 65535) == 320 ? HD_ANCHOR_FILL : HD_ANCHOR_CENTER;
    if (!hd_rect((sint16)pos, (sint16)(pos >> 16), extent & 65535, extent >> 16, color, blend, anchor, layer))
        hd_frame.failed = true;
}
