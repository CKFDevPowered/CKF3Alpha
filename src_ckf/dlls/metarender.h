#pragma once

#define METARENDER_SKYCAMERA 1
#define METARENDER_SHADOWMGR 2

typedef struct
{
	char affectmodel[64];
	vec3_t angles;
	float radius;
	float fard;
	float scale;
	int texsize;
}shadow_manager_t;

typedef struct
{
	qboolean enable;
	vec3_t origin;
	vec3_t center;
	char model[32];
}skycamera_t;