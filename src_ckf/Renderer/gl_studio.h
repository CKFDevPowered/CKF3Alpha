#pragma once

#include "gl_draw.h"

#define STUDIO_RENDER 1
#define STUDIO_EVENTS 2

// lighting options
#define STUDIO_NF_FLATSHADE		0x0001
#define STUDIO_NF_CHROME		0x0002
#define STUDIO_NF_FULLBRIGHT	0x0004
#define STUDIO_NF_NOMIPS        0x0008
#define STUDIO_NF_ALPHA         0x0010
#define STUDIO_NF_ADDITIVE      0x0020
#define STUDIO_NF_MASKED        0x0040

typedef struct
{
	int program;
	int lightpos;
	int eyepos;
	int ambient;
	int diffuse;
	int specular;
	int shiness;
	int basemap;
	int normalmap;
	int tangent;
	int binormal;
}studio_program_t;

typedef struct
{
	int program;
	int basemap;
	int normalmap;
	int time;
}invuln_program_t;

typedef struct
{
	char name[64];
	int index;
	int w, h;
	gltexture_t *glt;
}studio_texentry_t;

typedef struct
{
	studio_texentry_t base;
	studio_texentry_t replace;
	studio_texentry_t normal;
	float ambient;
	float diffuse;
	float specular;
	float shiness;
}studio_texture_t;

typedef struct
{
	char modelname[64];
	int numtextures;
	studio_texture_t *textures;
}studio_texarray_t;

typedef struct
{
	studio_texarray_t *pTexArray;
	int iNumTexArray;
}studio_texarray_mgr_t;

//engine
extern mstudiomodel_t **psubmodel;
extern studiohdr_t **pstudiohdr;
extern model_t **r_model;
extern float *r_blend;
extern float (*pbonetransform)[MAXSTUDIOBONES][3][4];
extern float (*plighttransform)[MAXSTUDIOBONES][3][4];
extern int (*g_NormalIndex)[MAXSTUDIOVERTS];
extern int (*chromeage)[MAXSTUDIOBONES];
extern cl_entity_t *cl_viewent;
extern int *g_ForcedFaceFlags;
extern int (*lightgammatable)[1024];

//renderer

void R_LoadStudioTextures(qboolean loadmap);
void R_InitStudio(void);

void R_GLStudioDrawPointsEx(void);
void R_StudioRenderFinal(void);
void R_GLStudioDrawPointsShadow(void);
void R_StudioDrawPoints(void);

void studioapi_StudioDrawPoints(void);
void studioapi_StudioSetupLighting(alight_t *plighting);
void studioapi_SetupRenderer(int rendermode);
void studioapi_RestoreRenderer(void);

int studiointerface_StudioDrawModel(int flags);
int studiointerface_StudioDrawPlayer(int flags, entity_state_t *pplayer);

extern engine_studio_api_t IEngineStudio;
extern r_studio_interface_t **gpStudioInterface;