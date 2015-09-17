#include <metahook.h>
#include "plugins.h"
#include "exportfuncs.h"
#include "engfuncs.h"
#include "client.h"
#include "util.h"
#include "cvar.h"
#include "event.h"

extern bool g_bGameUIActivate;

vec3_t g_vecZero = {0,0,0};
vec3_t g_vecHullMin = {-16, -16, -18};
vec3_t g_vecHullMax = {16, 16, 32};

bool HudBase_IsFullScreenMenu(void);
void HUD_InitWeapons(void);

hud_player_info_t g_HudPlayerInfo[33];
PlayerInfo g_PlayerInfo[33];
WeaponInfo g_WeaponInfo[MAX_WEAPONS];
BuildInfo g_Build;
PlayerStats g_PlayerStats;
zonevector g_NoBuildZones;
cpvector g_ControlPoints;

int g_iHealth;
int g_iMaxHealth;
int g_iClass;
int g_iDesiredClass;
int g_iTeam;
int g_iWeaponID;
int g_iHideHUD;
int g_iForceFOV;
int g_iDefaultFOV;

int g_iMenu;
int g_iMenuKeys;

int g_iHudMenu;
int g_iHudMenuKeys;

int g_iDisgMenuTeam;

int g_iRedTeamScore;
int g_iBlueTeamScore;
int g_iRedPlayerNum;
int g_iBluePlayerNum;
int g_iSpectatorNum;

int g_iLocalPlayerNum;

int g_iLimitTeams;

char g_szServerName[64];

int g_iRoundStatus;
int g_iLastRoundStatus;
int g_iMaxRoundTime;
float g_flRoundEndTime;
float g_flRoundStatusChangeTime;

int g_iCapPointIndex;

qboolean CL_IsValidPlayer(int team, int playerclass)
{
	if(team != 1 && team != 2)
		return false;
	if(playerclass < CLASS_SCOUT || playerclass > CLASS_SPY)
		return false;
	return true;
}

qboolean CL_IsAlive(void)
{
	if(!g_iClass || g_iTeam == 0 || g_iTeam > 2 || g_iHealth <= 0)
		return false;

	return true;
}

void CL_InitVars(void)
{
	g_iHealth = 0;
	g_iMaxHealth = 0;
	g_iClass = 0;
	g_iDesiredClass = 0;
	g_iTeam = 0;
	g_iWeaponID = 0;
	g_iForceFOV = 0;
	g_iViewModelSkin = 0;
	g_iViewModelBody = 0;
	g_iHookSetupBones = 0;
	g_iMenu = 0;
	g_iHudMenu = 0;
	g_flTraceDistance = 0;
	g_iDefaultFOV = 90;
	g_pTraceEntity = NULL;
	g_iLocalPlayerNum = 0;
	g_iRedTeamScore = 0;
	g_iBlueTeamScore = 0;
	g_iRedPlayerNum = 0;
	g_iBluePlayerNum = 0;
	g_iSpectatorNum = 0;
	g_iLimitTeams = 0;
	g_szServerName[0] = 0;
	g_iRoundStatus = 0;
	g_iMaxRoundTime = 0;
	g_flRoundEndTime = 0;
	g_flRoundStatusChangeTime = 0;
	g_iCapPointIndex = 0;

	memset(g_WeaponInfo, 0, sizeof(g_WeaponInfo));
	memset(g_PlayerInfo, 0, sizeof(g_PlayerInfo));
	memset(&g_PlayerStats, 0, sizeof(g_PlayerStats));
	memset(&g_Build, 0, sizeof(g_Build));
	g_NoBuildZones.clear();
	g_ControlPoints.clear();

	HUD_InitWeapons();
}

void CL_BottleBroken(cl_entity_t *pEntity)
{
	if(pEntity == gEngfuncs.GetLocalPlayer())
	{
		if(*cls_viewmodel_sequence == BOTTLE_SLASH1)
			HudWeaponAnimEx(BOTTLE_SLASH1_BROKEN, 0, 0, *cls_viewmodel_starttime);
		else if(*cls_viewmodel_sequence == BOTTLE_SLASH2)
			HudWeaponAnimEx(BOTTLE_SLASH2_BROKEN, 0, 0, *cls_viewmodel_starttime);
	}
	gEngfuncs.pfnPlaySoundByNameAtLocation( "CKF_III/bottle_break.wav", 1.0f, pEntity->origin );
}

int CL_GetViewSkin(void)
{
	if(g_iViewModelSkin)
		return g_iViewModelSkin;
	if(g_WeaponInfo[g_iWeaponID].iSkin)
		return g_WeaponInfo[g_iWeaponID].iSkin;
	return 0;
}

int CL_GetViewBody(void)
{
	if(g_iViewModelBody)
		return g_iViewModelBody;
	if(g_WeaponInfo[g_iWeaponID].iBody)
		return g_WeaponInfo[g_iWeaponID].iBody;
	return 0;
}

qboolean CL_IsFirstPersonSpec(void)
{
	return (g_iUser1 == OBS_IN_EYE || (g_iUser1 && (gHUD_m_pip && gHUD_m_pip->value == INSET_IN_EYE)));
}

qboolean CL_IsThirdPerson(void)
{
	if(gExportfuncs.CL_IsThirdPerson() || chase_active->value != 0 || *envmap || refparams.viewentity > refparams.maxclients)
		return true;
	return false;
}

qboolean CL_CanDrawViewModel(void)
{
	if(gExportfuncs.CL_IsThirdPerson() || chase_active->value != 0 || *envmap || !r_drawentities->value || refparams.health <= 0 || refparams.viewentity > refparams.maxclients)
		return false;
	if(gRefExports.R_GetDrawPass() != r_draw_normal)
		return false;
	if(g_pBTEClient->GetHUDFov() < 60)
		return false;
	return true;
}

int CL_TraceEntity_Ignore(physent_t *pe)
{
	if(pe->info == gEngfuncs.GetLocalPlayer()->index)
		return 1;
	if(pe->solid == SOLID_BSP && pe->team != 0)
	{
		if(g_iTeam == pe->team)
			return 1;
		if(pe->team == 3)
			return 1;
	}
	return 0;
}

void CL_SetupPlayerStudio(int startnum)
{
	int i;
	cl_entity_t *pe;

	int maxClients = gEngfuncs.GetMaxClients();
	for(i = startnum; i <= cl_pmove->numphysent; ++i)
	{
		if(cl_pmove->physents[i].info < 1 || cl_pmove->physents[i].info > maxClients)// || strncmp(cl_pmove->physents[i].name, "player", 6))
			continue;
		pe = gEngfuncs.GetEntityByIndex(cl_pmove->physents[i].info);
		if(!pe || !pe->player)
			continue;
		cl_pmove->physents[i].sequence = cl_pmove->physents[i].info;
		cl_pmove->physents[i].studiomodel = pe->model;
	}
}

void CL_SetupPMTrace(int idx)
{
	gEngfuncs.pEventAPI->EV_SetUpPlayerPrediction(false, true);
	gEngfuncs.pEventAPI->EV_PushPMStates();
	int startnum = cl_pmove->numphysent;
	gEngfuncs.pEventAPI->EV_SetSolidPlayers(idx - 1);
	gEngfuncs.pEventAPI->EV_SetTraceHull(2);
	CL_SetupPlayerStudio(startnum);
	g_iHookSetupBones = 1;
}

void CL_FinishPMTrace(void)
{
	g_iHookSetupBones = 0;
	gEngfuncs.pEventAPI->EV_PopPMStates();
}

void CL_TraceEntity(void)
{
	vec3_t vecForward, vecRight, vecUp;
	vec3_t vecSrc, vecDest;

	gEngfuncs.pfnAngleVectors(refparams.viewangles, vecForward, vecRight, vecUp);	
	
	VectorCopy(refparams.vieworg, vecSrc);
	VectorMA(vecSrc, 8192, vecForward, vecDest);

	pmtrace_t *tr;

	CL_SetupPMTrace(0);

	//call PM_PlayerTrace
	tr = cl_pmove->PM_TraceLineEx(vecSrc, vecDest, PM_NORMAL, 2, CL_TraceEntity_Ignore );

	g_pTraceEntity = NULL;
	if(tr->ent)
	{
		physent_t *physent = gEngfuncs.pEventAPI->EV_GetPhysent(tr->ent);
		if(physent)
		{
			g_pTraceEntity = gEngfuncs.GetEntityByIndex(physent->info);
		}
	}

	CL_FinishPMTrace();

	VectorSubtract(tr->endpos, vecSrc, vecDest);

	float dist = VectorLength(vecDest);

	g_flTraceDistance = dist;
}

void CL_CreateTempEntity(cl_entity_t *pEntity, model_t *mod)
{
	memset(pEntity, 0, sizeof(cl_entity_t));
	pEntity->curstate.rendermode = kRenderNormal;
	pEntity->curstate.renderfx = kRenderFxNone;
	pEntity->curstate.renderamt = 255;	
	pEntity->curstate.framerate = 1;
	pEntity->curstate.frame = 0;
	pEntity->curstate.skin = 0;
	pEntity->curstate.body = 0;
	pEntity->curstate.sequence = 0;
	pEntity->curstate.solid = SOLID_NOT;
	pEntity->curstate.movetype = MOVETYPE_NOCLIP;	
	pEntity->curstate.entityType = ET_NORMAL;
	VectorClear(pEntity->curstate.vuser1);
	pEntity->model = mod;
}

model_t *CL_LoadTentModel(const char *pName)
{
	model_t *pModel;

	pModel = IEngineStudio.Mod_ForName(pName, true);
	pModel->needload = NL_CLIENT;
	return pModel;
}

void ShowHudMenu(int type, int keys)
{
	g_iHudMenu = type;
	g_iHudMenuKeys = keys;

	if(g_iHudMenu == HUDMENU_DISGUISE)
	{
		g_iDisgMenuTeam = 3-g_iTeam;
	}
}

extern "C"
{
	int PM_NoCollision(int iPlayerIndex, int iEntIndex)
	{
		cl_entity_t *pEnt = gEngfuncs.GetEntityByIndex(iEntIndex);
		if(!pEnt)
			return 0;

		if(pEnt->player && iEntIndex >= 1 && iEntIndex <= gEngfuncs.GetMaxClients())
		{
			if(g_iTeam == g_PlayerInfo[iEntIndex].iTeam)
				return 1;
			return 0;
		}
		if(pEnt->curstate.solid == SOLID_BSP && pEnt->curstate.team != 0)
		{
			if(g_iTeam == pEnt->curstate.team)
				return 1;
			if(pEnt->curstate.team == 3)
				return 1;
			return 0;
		}
		if(pEnt->curstate.solid == SOLID_SLIDEBOX && pEnt->model)
		{
			if(pEnt->curstate.team == g_iTeam && pEnt->curstate.iuser1 != gEngfuncs.GetLocalPlayer()->index)
			{
				if(!strncmp(&pEnt->model->name[15], "w_dis", 3) || !strncmp(&pEnt->model->name[15], "w_sen", 3))
					return 1;
			}
			return 0;
		}
		return 0;
	}
}

void pfnGetPlayerInfo( int ent_num, struct hud_player_info_s *pinfo )
{
	gEngfuncs.pfnGetPlayerInfo(ent_num, pinfo);
	if(pinfo->name && pinfo->name[0])
	{
		strncpy(pinfo->name, UnicodeToANSI(UTF8ToUnicode(pinfo->name)), 31);
		pinfo->name[31] = 0;
	}
}