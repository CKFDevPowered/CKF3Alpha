#include "hud.h"
#include "cl_util.h"
#include "const.h"
#include "entity_state.h"
#include "cl_entity.h"

#include "r_efx.h"

#include "eventscripts.h"
#include "event_api.h"
#include "pm_shared.h"

#define IS_FIRSTPERSON_SPEC (g_iUser1 == OBS_IN_EYE || (g_iUser1 && (gHUD.m_Spectator.m_pip->value == INSET_IN_EYE)))

struct cl_entity_s *GetEntity(int idx)
{
	return gEngfuncs.GetEntityByIndex(idx);
}

struct cl_entity_s *GetViewEntity(void)
{
	return gEngfuncs.GetViewModel();
}

void EV_CreateTracer(float *start, float *end)
{
	gEngfuncs.pEfxAPI->R_TracerEffect(start, end);
}

qboolean EV_IsPlayer(int idx)
{
	if (idx >= 1 && idx <= gEngfuncs.GetMaxClients())
		return true;

	return false;
}

qboolean EV_IsLocal(int idx)
{
	if (IS_FIRSTPERSON_SPEC)
		return (g_iUser2 == idx);
	else
		return gEngfuncs.pEventAPI->EV_IsLocal(idx - 1) ? true : false;
}

void EV_GetGunPosition(event_args_t *args, float *pos, float *origin)
{
	int idx;
	vec3_t view_ofs;

	idx = args->entindex;

	VectorClear(view_ofs);
	view_ofs[2] = DEFAULT_VIEWHEIGHT;

	if (EV_IsPlayer(idx))
	{
		if (EV_IsLocal(idx) && !IS_FIRSTPERSON_SPEC)
			gEngfuncs.pEventAPI->EV_LocalPlayerViewheight(view_ofs);
		else if (args->ducking == 1)
			view_ofs[2] = VEC_DUCK_VIEW;
	}

	VectorAdd(origin, view_ofs, pos);
}

void EV_EjectBrass(float *origin, float *velocity, float rotation, int model, int soundtype, int idx, int angle_velocity)
{
	g_pfnEV_EjectBrass(origin, velocity, rotation, model, soundtype, idx, angle_velocity);
}

void EV_EjectBrass(float *origin, float *velocity, float rotation, int model, int soundtype)
{
	vec3_t endpos;
	VectorClear(endpos);
	endpos[1] = rotation;
	gEngfuncs.pEfxAPI->R_TempModel(origin, velocity, endpos, 2.5, model, soundtype);
}

void EV_GetDefaultShellInfo(event_args_t *args, float *origin, float *velocity, float *ShellVelocity, float *ShellOrigin, float *forward, float *right, float *up, float forwardScale, float upScale, float rightScale, bool bReverseDirection)
{
	int i;
	vec3_t view_ofs;
	float fR, fU;

	int idx;

	idx = args->entindex;

	VectorClear(view_ofs);
	view_ofs[2] = DEFAULT_VIEWHEIGHT;

	if (EV_IsPlayer(idx))
	{
		if (EV_IsLocal(idx))
			gEngfuncs.pEventAPI->EV_LocalPlayerViewheight(view_ofs);
		else if (args->ducking == 1)
			view_ofs[2] = VEC_DUCK_VIEW;
	}

	fR = gEngfuncs.pfnRandomFloat(50, 70);
	fU = gEngfuncs.pfnRandomFloat(100, 150);

	VectorScale(velocity, 0.5, velocity);

	if (bReverseDirection)
	{
		for (i = 0; i < 3; i++)
		{
			ShellVelocity[i] = velocity[i] - right[i] * fR + up[i] * fU + forward[i];
			ShellOrigin[i] = origin[i] + view_ofs[i] + up[i] * upScale + forward[i] * forwardScale + right[i] * rightScale + (velocity[i] / 5.0);
		}
	}
	else
	{
		for (i = 0; i < 3; i++)
		{
			ShellVelocity[i] = velocity[i] + right[i] * fR + up[i] * fU + forward[i];
			ShellOrigin[i] = origin[i] + view_ofs[i] + up[i] * upScale + forward[i] * forwardScale + right[i] * rightScale + (velocity[i] / 5.0);
		}
	}
}

void EV_GetDefaultShellInfo(event_args_t *args, float *origin, float *velocity, float *ShellVelocity, float *ShellOrigin, float *forward, float *right, float *up, float forwardScale, float upScale, float rightScale)
{
	int i;
	vec3_t view_ofs;
	float fR, fU;

	int idx;

	idx = args->entindex;

	VectorClear(view_ofs);
	view_ofs[2] = DEFAULT_VIEWHEIGHT;

	if (EV_IsPlayer(idx))
	{
		if (EV_IsLocal(idx))
			gEngfuncs.pEventAPI->EV_LocalPlayerViewheight(view_ofs);
		else if (args->ducking == 1)
			view_ofs[2] = VEC_DUCK_VIEW;
	}

	fR = gEngfuncs.pfnRandomFloat(50, 70);
	fU = gEngfuncs.pfnRandomFloat(100, 150);
#if 0
	gEngfuncs.Con_Printf("fR:%f, fU:%f\n", fR, fU);
	gEngfuncs.Con_Printf("velocity.x:%f, velocity.y:%f, velocity.z:%f\n", velocity[0], velocity[1], velocity[2]);
	gEngfuncs.Con_Printf("forward.x:%f, forward.y:%f, forward.z:%f\n", forward[0], forward[1], forward[2]);
	gEngfuncs.Con_Printf("right.x:%f, right.y:%f, right.z:%f\n", right[0], right[1], right[2]);
	gEngfuncs.Con_Printf("up.x:%f, up.y:%f, up.z:%f\n", up[0], up[1], up[2]);
#endif
	for (i = 0; i < 3; i++)
	{
		ShellVelocity[i] = velocity[i] + right[i] * fR + up[i] * fU + forward[i] * 25;
		ShellOrigin[i] = origin[i] + view_ofs[i] + up[i] * upScale + forward[i] * forwardScale + right[i] * rightScale;
	}
#if 0
	gEngfuncs.Con_Printf("ShellVelocity.x:%f, ShellVelocity.y:%f, ShellVelocity.z:%f\n", ShellVelocity[0], ShellVelocity[1], ShellVelocity[2]);
#endif
}

void EV_MuzzleFlash(void)
{
	cl_entity_t *ent = GetViewEntity();

	if (!ent)
		return;

	ent->curstate.effects |= EF_MUZZLEFLASH;
}