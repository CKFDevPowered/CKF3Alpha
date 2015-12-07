#pragma once

#define MAX_WATERS 32

typedef struct
{
	int waterfogcolor;
	int eyepos;
	int time;
	int fresnel;
	int abovewater;
	int reflectmap;
	int refractmap;
	int normalmap;
}water_uniform_t;

typedef struct
{
	int tangent;
	int binormal;
}water_attrib_t;

typedef struct r_water_s
{
	GLuint refractmap;
	GLuint reflectmap;
	vec3_t vecs;
	float distances;
	cl_entity_t *ent;
	vec3_t org;
	struct r_water_s *next;
	int is3dsky;
}r_water_t;

//renderer
extern qboolean drawreflect;
extern qboolean drawrefract;
extern mplane_t custom_frustum[4];
extern int water_update_counter;
extern int water_texture_size;
//water
extern r_water_t water[MAX_WATERS];
extern r_water_t *curwater;
extern r_water_t *waters_free;
extern r_water_t *waters_active;

//shader
extern int water_program;
extern water_uniform_t water_uniform;
extern water_attrib_t water_attrib;
extern int water_normalmap;
extern int water_normalmap_default;

//water fog
extern int *g_bUserFogOn;
extern int save_userfogon;
extern int waterfog_on;

typedef struct
{
	qboolean fog;
	vec4_t color;
	float start;
	float end;
	float density;
	float fresnel;
	qboolean active;
}water_parm_t;

extern water_parm_t water_parm;

//cvar
extern cvar_t *r_water;
extern cvar_t *r_water_debug;
extern cvar_t *r_water_fresnel;

void R_AddWater(cl_entity_t *ent, vec3_t p);
void R_InitWater(void);
void R_ClearWater(void);
void R_SetupReflect(void);
void R_FinishReflect(void);
void R_SetupRefract(void);
void R_FinishRefract(void);
void R_UpdateWater(void);
void R_SetupClip(qboolean isdrawworld);
void R_SetWaterParm(water_parm_t *parm);
void R_SetCustomFrustum(void);