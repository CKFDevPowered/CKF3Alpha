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
}skycamera_t;

extern skycamera_t g_SkyCamera;