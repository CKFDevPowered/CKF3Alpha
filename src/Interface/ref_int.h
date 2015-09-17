#pragma once

#include <gl/gl.h>
#include <gl/glext.h>

#include <com_model.h>
#include <studio.h>

#define TEX_TYPE_NONE 0
#define TEX_TYPE_ALPHA 1
#define TEX_TYPE_LUM 2
#define TEX_TYPE_ALPHA_GRADIENT 3
#define TEX_TYPE_RGBA 4

#define NL_PRESENT 0
#define NL_NEEDS_LOADED 1
#define NL_UNREFERENCED 2
#define NL_CLIENT 3

#pragma pack(1)
typedef struct gltexture_s
{
	int texnum;
	short servercount;
	short paletteIndex;
	int width;
	int height;
	qboolean mipmap;
	char identifier[64];
}gltexture_t;
#pragma pack()

typedef enum
{
	GLT_SYSTEM,
	GLT_DECAL,
	GLT_HUDSPRITE,
	GLT_STUDIO,
	GLT_WORLD,
	GLT_SPRITE,
}
GL_TEXTURETYPE;

typedef struct
{
	qboolean fog;
	vec4_t color;
	float start;
	float end;
	float density;
	float fresnel;
}water_parm_t;

typedef struct
{
	int r, g, b;
}mcolor24_t;

#ifndef BASETYPES_H
struct vrect_t
{
	int				x,y,width,height;
	vrect_t			*pnext;
};
#endif

typedef struct refdef_s
{
	vrect_t vrect;
	vrect_t aliasvrect;
	int vrectright;
	int vrectbottom;
	int aliasvrectright;
	int aliasvrectbottom;
	float vrectrightedge;
	float fvrectx;
	float fvrecty;
	float fvrectx_adj;
	float fvrecty_adj;
	int vrect_x_adj_shift20;
	int vrectright_adj_shift20;
	float fvrectright_adj;
	float fvrectbottom_adj;
	float fvrectright;
	float fvrectbottom;
	float horizontalFieldOfView;
	float xOrigin;
	float yOrigin;
	vec3_t vieworg;
	vec3_t viewangles;
	color24 ambientlight;
	qboolean onlyClientDraws;
}refdef_t;

typedef struct
{
	GLuint (*R_CompileShader)(const char *vscode, const char *fscode, const char *vsfile, const char *fsfile);
	void (*GL_UseProgram)(GLuint program);
	void (*GL_EndProgram)(void);
	GLuint (*GL_GetUniformLoc)(GLuint program, const char *name);
	GLuint (*GL_GetAttribLoc)(GLuint program, const char *name);
	void (*GL_Uniform1i)(GLuint loc, int v0);
	void (*GL_Uniform2i)(GLuint loc, int v0, int v1);
	void (*GL_Uniform3i)(GLuint loc, int v0, int v1, int v2);
	void (*GL_Uniform4i)(GLuint loc, int v0, int v1, int v2, int v3);
	void (*GL_Uniform1f)(GLuint loc, float v0);
	void (*GL_Uniform2f)(GLuint loc, float v0, float v1);
	void (*GL_Uniform3f)(GLuint loc, float v0, float v1, float v2);
	void (*GL_Uniform4f)(GLuint loc, float v0, int v1, int v2, int v3);
	void (*GL_VertexAttrib3f)(GLuint index, float x, float y, float z);
	void (*GL_VertexAttrib3fv)(GLuint index, float *v);
	void (*GL_MultiTexCoord2f)(GLenum target, float s, float t);
	void (*GL_MultiTexCoord3f)(GLenum target, float s, float t, float r);
}shaderapi_t;

typedef struct
{
	void (*GL_Bind)(int texnum);
	void (*GL_SelectTexture)(GLenum target);
	void (*GL_DisableMultitexture)(void);
	void (*GL_EnableMultitexture)(void);
	void (*R_DrawBrushModel)(cl_entity_t *entity);
	void (*R_DrawSpriteModel)(cl_entity_t *entity);
	void (*R_GetSpriteAxes)(cl_entity_t *entity, int type, float *vforwrad, float *vright, float *vup);
	void (*R_SpriteColor)(mcolor24_t *col, cl_entity_t *entity, int renderamt);
	float (*GlowBlend)(cl_entity_t *entity);
	int (*CL_FxBlend)(cl_entity_t *entity);
	int (*R_CullBox)(vec3_t mins, vec3_t maxs);
	void (*GL_SwapBuffer)(void);
}engrefapi_t;

typedef struct
{
	//common
	int (*R_GetDrawPass)(void);
	int (*R_GetSupportExtension)(void);
	//water
	void (*R_SetWaterParm)(water_parm_t *parm);
	//studio
	void (*R_GLStudioDrawPointsEx)(void);
	entity_state_t *(*R_GetPlayerState)(int index);
	//refdef
	void (*R_PushRefDef)(void);
	void (*R_UpdateRefDef)(void);
	void (*R_PopRefDef)(void);
	float *(*R_GetSavedViewOrg)(void);
	refdef_t *(*R_GetRefDef)(void);
	//shadow
	void (*R_CreateShadowLight)(cl_entity_t *entity, vec3_t angles, float radius, float fard, float scale, int texscale);
	//texture
	GLuint (*R_GLGenTexture)(int w, int h);
	byte *(*R_GetTexLoaderBuffer)(int *bufsize);
	int (*R_LoadTextureEx)(const char *path, const char *name, int *width, int *height, GL_TEXTURETYPE type, qboolean mipmap, qboolean ansio);
	int (*GL_LoadTextureEx)(const char *identifier, GL_TEXTURETYPE textureType, int width, int height, byte *data, qboolean mipmap, qboolean ansio);
	gltexture_t *(*R_GetCurrentGLTexture)(void);
	int (*LoadBMP)(const char *szFilename, byte *buffer, int bufferSize, int *width, int *height);
	int (*LoadTGA)(const char *szFilename, byte *buffer, int bufferSize, int *width, int *height);
	int (*LoadPNG)(const char *szFilename, byte *buffer, int bufferSize, int *width, int *height);
	int (*LoadDDS)(const char *szFilename, byte *buffer, int bufferSize, int *width, int *height);
	int (*SaveBMP)(const char *file_name, int width, int height, byte *data);
	int (*SaveTGA)(const char *file_name, int width, int height, byte *data);
	int (*SavePNG)(const char *file_name, int width, int height, byte *data);
	//capture screen
	byte *(*R_GetSCRCaptureBuffer)(int *bufsize);
	//3dsky
	void (*R_Add3DSkyEntity)(cl_entity_t *ent);
	void (*R_Setup3DSkyModel)(void);
	void (*R_Finish3DSkyModel)(void);
	//2d postprocess
	void (*R_BeginFXAA)(int w, int h);
	void (*R_BeginDrawRoundRect)(int centerX, int centerY, float radius, float blurdist);
	void (*R_BeginDrawHudMask)(int r, int g, int b);
	//cloak
	void (*R_RenderCloakTexture)(void);
	//3dhud
	int (*R_Get3DHUDTexture)(void);
	void (*R_Draw3DHUDQuad)(int x, int y, int left, int top);
	void (*R_BeginDrawTrianglesInHUD_Direct)(int x, int y);
	void (*R_BeginDrawTrianglesInHUD_FBO)(int x, int y, int left, int top);
	void (*R_FinishDrawTrianglesInHUD)(void);
	void (*R_BeginDrawHUDInWorld)(int texid, int w, int h);
	void (*R_FinishDrawHUDInWorld)(void);
	//shader
	shaderapi_t ShaderAPI;
	engrefapi_t RefAPI;
}ref_export_t;

extern ref_export_t gRefExports;

#define r_draw_normal 0
#define r_draw_reflect 1
#define r_draw_refract 2
#define r_draw_shadow 3
#define r_draw_3dhud 4
#define r_draw_shadowscene 5
#define r_draw_3dsky 6
#define r_draw_hudinworld 7

#define r_ext_fbo (1<<0)
#define r_ext_msaa (1<<1)
#define r_ext_water (1<<2)
#define r_ext_shader (1<<3)
#define r_ext_shadow (1<<4)

#define META_RENDERER_VERSION "Meta Renderer 1.4b"