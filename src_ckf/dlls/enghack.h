#pragma once

#include "hook.h"
#include <com_model.h>

typedef struct areanode_s
{
	int axis;
	float dist;
	struct areanode_s *children[2];
	link_t trigger_edicts;
	link_t solid_edicts;
}areanode_t;

typedef struct
{
	vec3_t boxmins;
	vec3_t boxmaxs;
	float *mins;
	float *maxs;
	vec3_t mins2;
	vec3_t maxs2;
	float *start;
	float *end;
	trace_t trace;
	short type;
	short ignoretrans;
	edict_t *passedict;
	qboolean monsterClipBrush;
}moveclip_t;

#define MOVE_NORMAL 0
#define MOVE_NOMONSTERS 1
#define MOVE_MISSILE 2

#define GetCallAddress(addr) (addr + (*(DWORD *)(addr+1)) + 5)

extern model_t **sv_models;
extern model_t **sv_worldmodel;

typedef struct
{
	int (*buildnum)(void);
	void (*SV_ClipToLinks)(areanode_t *node, moveclip_t *clip);
	int (*SV_HullPointContents)(hull_t *hull, int num, const vec_t *p);
	void (*SV_AddToFatPVS)(float *org, mnode_t *node);
	void (*SV_AddToFatPAS)(float *org, mnode_t *node);
}sv_funcs_t;

extern sv_funcs_t gSVFuncs;