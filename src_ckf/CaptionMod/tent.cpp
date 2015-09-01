#include <metahook.h>
#include "exportfuncs.h"
#include "engfuncs.h"
#include "StudioModelRenderer.h"
#include "GameStudioModelRenderer.h"
#include "tent.h"
#include "client.h"
#include "util.h"
#include "weapon.h"
#include "qgl.h"

char *g_szPlayerModel[] = {"ckf_scout", "ckf_heavy", "ckf_soldier", "ckf_pyro", "ckf_sniper", "ckf_medic", "ckf_engineer", "ckf_demoman", "ckf_spy"};

model_t *g_mdlPlayer[10];
model_t *g_mdlHats;
model_t *g_mdlWatch;
model_t *g_mdlSapper;
model_t *g_mdlSpyMask;

void CL_SetupPMTrace(int idx);
void CL_FinishPMTrace(void);

BOOL UTIL_IsHullDefaultEx(vec3_t vecOrigin, float size, float height);
BOOL UTIL_IsHullInZone(physent_t *pe, vec3_t vecMins, vec3_t vecMaxs);

TE_BluePrint_t g_BluePrint;
TE_SpyWatch_t g_SpyWatch;

void T_Sapper_VidInit(void)
{
	g_mdlSapper = CL_LoadTentModel("models\\CKF_III\\w_sapper.mdl");
}

void T_PlayerModel_VidInit(void)
{
	char szModelPath[64];
	for(int i = 0; i < 9; ++i)
	{
		sprintf(szModelPath, "models/player/%s/%s.mdl", g_szPlayerModel[i], g_szPlayerModel[i]);
		g_mdlPlayer[i] = CL_LoadTentModel(szModelPath);
	}
	g_mdlSpyMask = CL_LoadTentModel("models\\CKF_III\\p_spymask.mdl");
	//g_mdlHats = CL_LoadTentModel("models\\CKF_III\\wp_hats.mdl");
	g_mdlPlayer[9] = NULL;
}

//Blue Print

void CL_BluePrint(int bp)
{
	if(bp >= BUILDABLE_SENTRY && bp <= BUILDABLE_EXIT)
	{
		g_BluePrint.ent.model = g_BluePrint.mdl[min(bp-1, 2)];
		g_BluePrint.show = 1;
		g_BluePrint.animtime = g_flClientTime;
		g_BluePrint.anim = 0;
		g_BluePrint.ent.curstate.frame = 0;
		g_BluePrint.ent.curstate.animtime = gEngfuncs.GetClientTime();
		g_BluePrint.build = bp;
	}
	else if(bp >= 10)
	{
		g_BluePrint.ent.curstate.frame = 0;
		g_BluePrint.ent.curstate.animtime = gEngfuncs.GetClientTime();
		g_BluePrint.animtime = gEngfuncs.GetClientTime();
		g_BluePrint.anim = (bp - 10) * 2 + 1;
	}
}

void T_BluePrint_VidInit(void)
{
	g_BluePrint.mdl[0] = CL_LoadTentModel("models/CKF_III/w_blueprint_sen.mdl");
	g_BluePrint.mdl[1] = CL_LoadTentModel("models/CKF_III/w_blueprint_dis.mdl");
	g_BluePrint.mdl[2] = CL_LoadTentModel("models/CKF_III/w_blueprint_tel.mdl");
	CL_CreateTempEntity(&g_BluePrint.ent, NULL);
	g_BluePrint.show = 0;
	g_BluePrint.anim = 0;
	g_BluePrint.animtime = 0;
	g_BluePrint.build = 0;
}

int T_BluePrint_IgnoreEnt(physent_t *pe)
{
	if(pe->info == gEngfuncs.GetLocalPlayer()->index)//ignore self
		return 1;

	cl_entity_t *ent = gEngfuncs.GetEntityByIndex(pe->info);
	if(pe->solid == SOLID_BSP && pe->team != 0)
	{
		if(g_iTeam == pe->team)
			return 1;
		if(pe->team == 3)
			return 1;
	}
	if(pe->info && pe->info < gEngfuncs.GetMaxClients())
	{
		if(g_PlayerInfo[ent->index].iTeam == g_iTeam)//ignore teammate
			return 1;
	}
	return 0;
}

void T_BluePrint_Update(void)
{
	if(!g_BluePrint.show)
		return;

	vec3_t vecAng, vecSrc, vecSrc2, vecDst, vecForward, vecRight, vecUp;
	vec3_t vecMins, vecMaxs;
	float ground;
	pmtrace_t tr;
	int checkhull = 0;
	int hullok = 1;
	int hitent[9];

	cl_entity_t *pEnt = &g_BluePrint.ent;

	if(CL_CanDrawViewModel())
	{
		VectorCopy(refdef->vieworg, vecSrc);
		VectorCopy(refdef->viewangles, vecAng);
	}
	else
	{
		VectorCopy(gEngfuncs.GetLocalPlayer()->origin, vecSrc);
		VectorCopy(gEngfuncs.GetLocalPlayer()->angles, vecAng);
	}
	vecAng[0] = 0;
	gEngfuncs.pfnAngleVectors(vecAng, vecForward, vecRight, vecUp);
	VectorMA(vecSrc, 60, vecForward, vecSrc);

	VectorCopy(vecAng, pEnt->angles);

	if(!(cl_pmove->flags & FL_ONGROUND))
	{
		hullok = 0;
	}
	if(g_iUser3 && (g_iUser3 & CDFLAG_NOBUILD))
	{
		hullok = 0;
	}

	float height;
	if(g_BluePrint.build == BUILDABLE_SENTRY)
		height = 24;
	else if(g_BluePrint.build == BUILDABLE_DISPENSER)
		height = 26;
	else
		height = 6;

	//setup pm
	CL_SetupPMTrace(-1);

	const static int vecDir[8][2] = {{1, 1}, {1, -1}, {-1, -1}, {-1, 1}, {1, 0}, {-1, 0}, {0, 1}, {0, -1}};
	ground = -9999;

	for(int i = 0; i < 8; ++i)
	{
		VectorCopy(vecSrc, vecSrc2);
		vecSrc2[0] += vecDir[i][0]*16;
		vecSrc2[1] += vecDir[i][1]*16;
		VectorCopy(vecSrc2, vecDst);
		vecDst[2] -= 9999;
		tr = cl_pmove->PM_PlayerTraceEx(vecSrc2, vecDst, PM_STUDIO_BOX, T_BluePrint_IgnoreEnt);
		if(tr.endpos[2] > ground)
			ground = tr.endpos[2];
		hitent[i] = tr.ent;
	}
	VectorCopy(vecSrc, vecDst);
	vecDst[2] -= 9999;
	tr = cl_pmove->PM_PlayerTraceEx(vecSrc, vecDst, PM_STUDIO_BOX, T_BluePrint_IgnoreEnt);
	if(tr.endpos[2] > ground)
		ground = tr.endpos[2];
	hitent[8] = tr.ent;

	if(hullok)
	{
		int maxclients = gEngfuncs.GetMaxClients();
		for(int i = 0; i < 9; ++i)
		{
			if(!hitent[i])
				continue;
			physent_t *physent = gEngfuncs.pEventAPI->EV_GetPhysent(hitent[i]);
			if(!physent)
				continue;
			if(physent->solid == SOLID_BSP)
				continue;
			if(physent->info >= 1 && physent->info <= maxclients)
			{
				hullok = 0;
				break;
			}
			if(physent->studiomodel)
			{
				if(strstr(physent->studiomodel->name, "w_"))
				{
					hullok = 0;
					break;
				}
			}
		}
	}

	if(hullok && vecSrc[2] - (ground + height) > 64)
	{
		hullok = 0;
	}
	if(hullok && tr.plane.normal[2] < 0.866)
	{
		hullok = 0;
	}

	if(!hullok)
	{
		float vFeet = cl_pmove->origin[2];
		vFeet += (cl_pmove->bInDuck) ? cl_pmove->player_mins[1][2] : cl_pmove->player_mins[0][2];
		
		VectorCopy(tr.endpos, vecSrc);
		vecSrc[2] = max(vFeet, ground);
	}
	else
	{
		VectorCopy(tr.endpos, vecSrc);
		vecSrc[2] = ground;
	}

	VectorCopy(vecSrc, pEnt->origin);

	vecSrc[2] += height;
	if(hullok && !UTIL_IsHullDefaultEx(vecSrc, 16, height))
	{
		hullok = 0;
	}
	if(hullok && !g_NoBuildZones.empty())
	{
		vecMins[0] = vecSrc[0] - 16;
		vecMins[1] = vecSrc[1] - 16;
		vecMins[2] = vecSrc[2] - height;
		vecMaxs[0] = vecSrc[0] + 16;
		vecMaxs[1] = vecSrc[1] + 16;
		vecMaxs[2] = vecSrc[2] + height;
		int size = g_NoBuildZones.size();
		for(int i = 0; i < size ; ++i)
		{
			physent_t *pe = &g_NoBuildZones[i];
			if(pe->team != 0 && pe->team != g_iTeam)
				continue;
			if(UTIL_IsHullInZone(pe, vecMins, vecMaxs))
			{
				hullok = 0;
				break;
			}
		}
	}
	//reset pm
	CL_FinishPMTrace();

	if(!hullok)
	{
		g_BluePrint.ent.curstate.sequence = 8;
	}
	else
	{
		g_BluePrint.ent.curstate.sequence = g_BluePrint.anim;
	}

	if(g_BluePrint.anim % 2 == 1)
	{
		if(g_flClientTime - g_BluePrint.animtime >= 0.5)
		{
			g_BluePrint.anim ++;
			if(g_BluePrint.anim == 8)
				g_BluePrint.anim = 0;

			if(g_BluePrint.ent.curstate.sequence != 8)
				g_BluePrint.ent.curstate.sequence = g_BluePrint.anim;
		}
	}
	if(g_iWeaponID != WEAPON_BUILDPDA)
	{
		g_BluePrint.show = 0;
	}
}

void T_BluePrint_Draw(void)
{
	if(!g_BluePrint.show)
		return;
	if(!g_BluePrint.ent.model)
		return;
	if(g_iClass != CLASS_ENGINEER)
	{
		g_BluePrint.show = 0;
		return;
	}
	*CurrentEntity = &g_BluePrint.ent;
	g_StudioRenderer.StudioDrawModel(STUDIO_RENDER);
}

//SpyWatch

void CL_SpyWatch(int action)
{
	if(action == CLOAK_BEGIN)
	{
		g_SpyWatch.ent.model = g_SpyWatch.mdl;
		g_SpyWatch.ent.curstate.skin = (g_iTeam == 1) ? 0 : 1;
		g_SpyWatch.show = 1;
		
		g_SpyWatch.anim = 0;
		g_SpyWatch.animtime = g_flClientTime + CloakBegin_Duration;
		g_SpyWatch.ent.curstate.sequence = 0;
		g_SpyWatch.ent.curstate.frame = 0;
		g_SpyWatch.ent.curstate.framerate = 1;
		g_SpyWatch.ent.curstate.animtime = g_flClientTime;
	}
	else if(action == CLOAK_STOP)
	{
		g_SpyWatch.show = 1;
		
		g_SpyWatch.ent.curstate.framerate = -1;
		g_SpyWatch.ent.curstate.sequence = 0;
		if(g_SpyWatch.anim == 1)
		{
			g_SpyWatch.ent.curstate.frame = 255;
			g_SpyWatch.ent.curstate.animtime = g_flClientTime;
			g_SpyWatch.animtime = g_flClientTime + CloakStop_Duration;
		}
		else
		{
			g_SpyWatch.ent.curstate.frame = 255 - 255*(g_flClientTime - g_SpyWatch.animtime)/CloakStop_Duration;
			g_SpyWatch.ent.curstate.animtime = g_flClientTime;
			g_SpyWatch.animtime = g_flClientTime + CloakStop_Duration;
		}
		g_SpyWatch.anim = 2;
	}
}

void T_SpyWatch_VidInit(void)
{
	g_mdlWatch = CL_LoadTentModel("models\\CKF_III\\v_inviswatch.mdl");
	g_SpyWatch.mdl = g_mdlWatch;
	CL_CreateTempEntity(&g_SpyWatch.ent, NULL);
	g_SpyWatch.show = 0;
	g_SpyWatch.anim = 0;
	g_SpyWatch.ent.curstate.renderfx = kRenderFxCloak;
}

void T_SpyWatch_Update(void)
{
	if(!g_SpyWatch.show)
		return;

	if(g_iClass != CLASS_SPY || !CL_IsAlive())
	{
		g_SpyWatch.show = 0;
		return;
	}

	if(g_SpyWatch.anim == 0)
	{
		g_SpyWatch.ent.curstate.renderamt = SPY_MIN_ALPHA + (255-SPY_MIN_ALPHA) * max(min((g_SpyWatch.animtime - g_flClientTime) / CloakBegin_Duration, 1), 0);
		g_SpyWatch.ent.curstate.framerate = ((23.0f/30) / CloakBegin_Duration);
		if(g_flClientTime > g_SpyWatch.animtime)
		{
			g_SpyWatch.ent.curstate.sequence = 2;
			g_SpyWatch.ent.curstate.renderamt = SPY_MIN_ALPHA;
			g_SpyWatch.anim = 1;
		}
	}
	else if(g_SpyWatch.anim == 2)
	{
		g_SpyWatch.ent.curstate.renderamt = 255 - (255-SPY_MIN_ALPHA) * max(min((g_SpyWatch.animtime - g_flClientTime) / CloakStop_Duration, 1), 0);
		g_SpyWatch.ent.curstate.framerate = -((23.0f/30) / CloakStop_Duration);
		if(g_flClientTime > g_SpyWatch.animtime)
		{
			g_SpyWatch.show = 0;
			g_SpyWatch.ent.curstate.renderamt = 255;
		}
	}
}

void SpyWatch_Draw(void)
{
	if(!g_SpyWatch.show)
		return;
	if(!g_SpyWatch.ent.model)
		return;

	VectorCopy(cl_viewent->angles, g_SpyWatch.ent.angles);
	VectorCopy(cl_viewent->origin, g_SpyWatch.ent.origin);

	qglDepthRange(0, 0.3);
	
	*CurrentEntity = &g_SpyWatch.ent;
	g_StudioRenderer.StudioDrawModel(STUDIO_RENDER|STUDIO_EVENTS);

	qglDepthRange(0, 1);
}

void CL_ControlPointUpdate(void)
{
	g_iCapPointIndex = 0;
	vec3_t vecSrc, vecMins, vecMaxs;

	VectorCopy(gEngfuncs.GetLocalPlayer()->origin, vecSrc);
	vecMins[0] = vecSrc[0] - 16;
	vecMins[1] = vecSrc[1] - 16;
	vecMins[2] = vecSrc[2] - 16;
	vecMaxs[0] = vecSrc[0] + 16;
	vecMaxs[1] = vecSrc[1] + 16;
	vecMaxs[2] = vecSrc[2] + 48;
	int size = g_ControlPoints.size();
	for(int i = 0; i < size ; ++i)
	{
		physent_t *pe = &g_ControlPoints[i].physent;

		if(UTIL_IsHullInZone(pe, vecMins, vecMaxs))
		{
			g_iCapPointIndex = i + 1;
			break;
		}
	}
}

//common

void T_VidInit(void)
{
	T_PlayerModel_VidInit();
	T_BluePrint_VidInit();
	T_SpyWatch_VidInit();
	T_Sapper_VidInit();
}

void T_DrawTEnts(void)
{
	T_BluePrint_Draw();
}

void T_UpdateTEnts(void)
{
	T_BluePrint_Update();
	T_SpyWatch_Update();
	CL_ControlPointUpdate();
}

