#pragma once

typedef struct
{
	int show;
	int anim;
	float animtime;
	cl_entity_t ent;
}TE_Default_t;

typedef struct
{
	int show;
	int anim;
	float animtime;
	cl_entity_t ent;

	int build;
	model_t *mdl[3];	
}TE_BluePrint_t;

typedef struct
{
	int show;
	int anim;
	float animtime;
	cl_entity_t ent;
	
	model_t *mdl;	
}TE_SpyWatch_t;

extern TE_BluePrint_t g_BluePrint;
extern TE_SpyWatch_t g_SpyWatch;