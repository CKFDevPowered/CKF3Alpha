#include <metahook.h>
#include "exportfuncs.h"
#include "engfuncs.h"
#include "parsemsg.h"
#include "message.h"
#include "client.h"
#include "util.h"
#include "weapon.h"
#include "hud_localize.h"

pfnUserMsgHook pfnMsgFunc_Health;
pfnUserMsgHook pfnMsgFunc_ResetHUD;
pfnUserMsgHook pfnMsgFunc_TeamInfo;
pfnUserMsgHook pfnMsgFunc_TeamScore;
pfnUserMsgHook pfnMsgFunc_InitHUD;
pfnUserMsgHook pfnMsgFunc_Ammo;
pfnUserMsgHook pfnMsgFunc_VGUIMenu;
pfnUserMsgHook pfnMsgFunc_HideWeapon;
pfnUserMsgHook pfnMsgFunc_ServerName;
pfnUserMsgHook pfnMsgFunc_DrawFX;

void HudBase_ShowMenu(void);
void HudBase_DeactivateMouse(void);
void HudBase_ActivateMouse(void);
void CL_InitVars(void);
void HudIntroMenu_SetPage(int page);

void Motd_ParseBuf(void);
void Motd_ClearBuf(void);
void Motd_AddToBuf(const char *str);
void MapInfo_ParseBuf(void);
void MapInfo_ClearBuf(void);
void MapInfo_AddToBuf(const char *str);
void ClassIntro_ParseBuf(void);
void ClassIntro_ClearBuf(void);
void ClassIntro_AddToBuf(const char *str);
void MapInfo_LoadFromFile(void);
void R_GrenadeTrail(cl_entity_t *pEntity);
void R_StickyTrail(cl_entity_t *pEntity);
void R_KillPartSystem(cl_entity_t *pEntity, int instant);
void CL_BottleBroken(cl_entity_t *pEntity);
void R_MultiJumpTrail(cl_entity_t *pEntity);
void R_RocketTrail(cl_entity_t *pEntity);
void R_ExplosionMidAir(vec3_t vecStart);
void R_ExplosionWall(vec3_t vecStart, vec3_t vecNormal);
void R_StickyKill(vec3_t origin, int skin);
void R_MediBeam(cl_entity_t *pEntity, cl_entity_t *pTarget, int iColor);
void R_KillMediBeam(cl_entity_t *pStart, cl_entity_t *pTarget);
void CL_BluePrint(int bp);
void R_DisguiseSmoke(cl_entity_t *pEntity, int iTeam);
void CL_SpyWatch(int action);
void R_FlameThrow(cl_entity_t *pEntity, int iTeam);
void HudDeathMsg_AddPanel(const char *killer, const char *assister, int killerteam, const char *victim, int victimteam, const char *iconname, int relative, int critkill);

void R_BeginEntityTracer(int iTracerColor, int iNumTracer, float flDelay, float flSpeed, int iEntityIndex, int iAttachIndex);
void R_BeginCoordTracer(int iTracerColor, int iNumTracer, float flDelay, float flSpeed, vec3_t vecSrc);
void R_EmitTracer(vec3_t vecDst);

void R_CritText(vec3_t vecSrc);
void R_MiniCritText(vec3_t vecSrc);
void R_HitDamageText(int damage, vec3_t vecSrc, int crit);
void ShowHudMenu(int type, int keys);
void HudFloatText_AddHealth(int iHealth);
void HudFloatText_AddMetal(int iMetal);
void R_BurningPlayer(cl_entity_t *pEntity, int iTeam, float flDuration);
void R_KillAllEntityPartSystem(int instant);
void R_CritPlayerWeapon(cl_entity_t *pEntity, int iTeam, float flDuration);
void R_AirBlast(cl_entity_t *pEntity);
qboolean EV_IsLocal( int idx );

int MsgFunc_HideWeapon(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ(pbuf, iSize);

	g_iHideHUD = READ_SHORT();
	
	return pfnMsgFunc_HideWeapon(pszName, iSize, pbuf);
}

int MsgFunc_Dominate(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ(pbuf, iSize);
	int killerindex = READ_BYTE();
	int victimindex = READ_BYTE();
	int relative = 0;

	hud_player_info_t killer_pi;
	hud_player_info_t victim_pi;

	if(killerindex == victimindex)
		return 1;

	gEngfuncs.pfnGetPlayerInfo(killerindex, &killer_pi);
	if(!killerindex || !killer_pi.name || !killer_pi.name[0])
	{
		gEngfuncs.Con_DPrintf("Bad killer id (%d) in MsgFunc_Dominate\n", killerindex);
		return 1;
	}

	int killerteam = g_PlayerInfo[killerindex].iTeam;

	if(killerindex == gEngfuncs.GetLocalPlayer()->index)
		relative = 1;

	gEngfuncs.pfnGetPlayerInfo(victimindex, &victim_pi);
	if(!victimindex || !victim_pi.name || !victim_pi.name[0])
	{
		gEngfuncs.Con_DPrintf("Bad victim id (%d) in MsgFunc_Dominate\n", victimindex);
		return 1;
	}

	int victimteam = g_PlayerInfo[victimindex].iTeam;

	if(victimindex == gEngfuncs.GetLocalPlayer()->index)
		relative = 1;

	char victim_name[128];

	sprintf(victim_name, "%s %s", g_szMsgDominating, victim_pi.name);

	HudDeathMsg_AddPanel(killer_pi.name, NULL, killerteam, victim_name, victimteam, "dom", relative, false);

	g_PlayerInfo[killerindex].iDominateList[victimteam] = 1;

	return 1;
}

int MsgFunc_Revenge(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ(pbuf, iSize);
	int killerindex = READ_BYTE();
	int victimindex = READ_BYTE();
	int relative = 0;

	hud_player_info_t killer_pi;
	hud_player_info_t victim_pi;

	if(killerindex == victimindex)
		return 1;

	gEngfuncs.pfnGetPlayerInfo(killerindex, &killer_pi);
	if(!killerindex || !killer_pi.name || !killer_pi.name[0])
	{
		gEngfuncs.Con_DPrintf("Bad killer id (%d) in MsgFunc_Revenge\n", killerindex);
		return 1;
	}

	int killerteam = g_PlayerInfo[killerindex].iTeam;

	if(killerindex == gEngfuncs.GetLocalPlayer()->index)
		relative = 1;

	gEngfuncs.pfnGetPlayerInfo(victimindex, &victim_pi);
	if(!victimindex || !victim_pi.name || !victim_pi.name[0])
	{
		gEngfuncs.Con_DPrintf("Bad victim id (%d) in MsgFunc_Revenge\n", victimindex);
		return 1;
	}

	int victimteam = g_PlayerInfo[victimindex].iTeam;

	if(victimindex == gEngfuncs.GetLocalPlayer()->index)
		relative = 1;

	char final_victim_name[128];

	sprintf(final_victim_name, "%s %s", g_szMsgRevenge, victim_pi.name);

	HudDeathMsg_AddPanel(killer_pi.name, NULL, killerteam, final_victim_name, victimteam, "dom", relative, false);

	g_PlayerInfo[victimteam].iDominateList[killerindex] = 0;

	return 1;
}

int MsgFunc_DeathMsg(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ(pbuf, iSize);
	int killerindex = READ_BYTE();
	int assisterindex = READ_BYTE();
	int victimindex = READ_BYTE();
	int critkill = READ_BYTE();
	char *iconname = READ_STRING();

	int relative = 0;
	int killerteam, victimteam;

	hud_player_info_t killer_pi;
	hud_player_info_t victim_pi;

	gEngfuncs.pfnGetPlayerInfo(victimindex, &victim_pi);
	if(!victimindex || !victim_pi.name || !victim_pi.name[0])
	{
		gEngfuncs.Con_DPrintf("Bad victim id (%d) in MsgFunc_DeathMsg\n", victimindex);
		return 1;
	}
	killerteam = 0;
	victimteam = g_PlayerInfo[victimindex].iTeam;

	char *killer_name = NULL;
	char *assister_name = NULL;

	bool suicide = false;

	if(killerindex == victimindex || !killerindex)
	{
		suicide = true;
	}
	else
	{
		gEngfuncs.pfnGetPlayerInfo(killerindex, &killer_pi);
		if(!killer_pi.name || !killer_pi.name[0])
		{
			suicide = true;
		}
		else
		{
			killer_name = killer_pi.name;
			killerteam = g_PlayerInfo[killerindex].iTeam;
			if(g_iTeam == killerteam)
				relative = 1;			
		}
	}

	if(g_iTeam == g_PlayerInfo[victimindex].iTeam)
		relative = 1;

	if(suicide)
	{
		killer_name = NULL;
		assister_name = NULL;
	}
	else
	{
		if(assisterindex != 0)
		{
			hud_player_info_t assister_pi;
			gEngfuncs.pfnGetPlayerInfo(assisterindex, &assister_pi);
			if(!assister_pi.name || !assister_pi.name[0])
			{
				assister_name = NULL;
				gEngfuncs.Con_DPrintf("Warning: Bad assister index (%d) in MsgFunc_DeathMsg", assisterindex);
			}
			else
			{
				assister_name = assister_pi.name;
				if(g_iTeam == g_PlayerInfo[assisterindex].iTeam)
					relative = 1;
			}
		}
		else
		{
			assister_name = NULL;
		}
	}

	HudDeathMsg_AddPanel(killer_name, assister_name, killerteam, victim_pi.name, victimteam, iconname, relative, critkill);

	return 1;
}

int MsgFunc_BuildDeath(const char *pszName, int iSize, void *pbuf)
{
	/*BEGIN_READ(pbuf, iSize);
	int killerindex = READ_BYTE();
	int assisterindex = READ_BYTE();
	int victimindex = READ_BYTE();
	int buildclass = READ_BYTE();
	char *iconname = READ_STRING();

	int relative = 0;
	int killerteam, victimteam;

	hud_player_info_t killer_pi;
	hud_player_info_t victim_pi;

	char victim_name[128];

	if(buildclass < 1 || buildclass > 5)
	{
		gEngfuncs.Con_DPrintf("Warning: Bad classindex (%d) in MsgFunc_BuildDeath", buildclass);
		return 1;
	}

	gEngfuncs.pfnGetPlayerInfo(victimindex, &victim_pi);
	if(!victimindex || !victim_pi.name || !victim_pi.name[0])
	{
		gEngfuncs.Con_DPrintf("Warning: Bad victim index (%d) in MsgFunc_BuildDeath", victimindex);
		return 1;
	}
	killerteam = 0;
	victimteam = g_PlayerInfo[victimindex].iTeam;

	char *killer_name = NULL;
	char *assister_name = NULL;

	bool suicide = false;

	if(killerindex == victimindex || !killerindex)
	{
		suicide = true;
	}
	else
	{
		gEngfuncs.pfnGetPlayerInfo(killerindex, &killer_pi);
		if(!killer_pi.name || !killer_pi.name[0])
		{
			suicide = true;
		}
		else
		{
			killer_name = killer_pi.name;
			killerteam = g_PlayerInfo[killerindex].iTeam;
			if(g_iTeam == killerteam)
				relative = 1;			
		}
	}

	if(g_iTeam == g_PlayerInfo[victimindex].iTeam)
		relative = 1;

	if(suicide)
	{
		killer_name = NULL;
		assister_name = NULL;
	}
	else
	{
		if(assisterindex != 0)
		{
			hud_player_info_t assister_pi;
			gEngfuncs.pfnGetPlayerInfo(assisterindex, &assister_pi);
			if(!assister_pi.name || !assister_pi.name[0])
			{
				assister_name = NULL;
				gEngfuncs.Con_DPrintf("Warning: Bad assister index (%d) in MsgFunc_BuildDeath", assisterindex);
			}
			else
			{
				assister_name = assister_pi.name;
				if(g_iTeam == g_PlayerInfo[assisterindex].iTeam)
					relative = 1;
			}
		}
		else
		{
			assister_name = NULL;
		}
	}

	sprintf(victim_name, "%s (%s)", g_szBuildables[buildclass-1], victim_pi.name);

	HudDeathMsg_AddPanel(killer_name, assister_name, killerteam, victim_name, victimteam, iconname, relative, 0);

	return 1;*/
	return 1;
}

int MsgFunc_ObjectMsg(const char *pszName, int iSize, void *pbuf)
{
	/*BEGIN_READ(pbuf, iSize);

	int killerteam = READ_BYTE();
	int killerindex = READ_BYTE();

	std::vector<int> killer_vector;
	if(killerindex == 0)
	{
		int killercount = READ_BYTE();
		for(int i = 0; i < killercount; ++i)
		{
			killer_vector.push_back(READ_BYTE());
		}		
	}
	else
	{
		killer_vector.push_back(killerindex);
	}

	int victimteam = READ_BYTE();
	std::string iconname = READ_STRING();
	std::string victimname = READ_STRING();

	int relative = 0;

	hud_player_info_t killer_pi;

	char killer_name[128];

	killer_name[0] = '\0';

	for(int i = 0; i < killer_vector.size(); ++i)
	{
		if(i != 0) strcat(killer_name, ", ");

		gEngfuncs.pfnGetPlayerInfo(killer_vector[i], &killer_pi);

		if(killer_pi.name && killer_pi.name[0])
			strcat(killer_name, killer_pi.name);

		if(killer_pi.thisplayer)
			relative = 1;
	}

	char victim_name[128];

	if(iconname.find("def") != std::string::npos)
		sprintf(victim_name, "%s %s", g_szMsgDefended, victimname.c_str());
	else
		sprintf(victim_name, "%s %s", g_szMsgCaptured, victimname.c_str());

	if(g_iTeam == victimteam)
		relative = 1;

	HudDeathMsg_AddPanel(killer_name, NULL, killerteam, victim_name, victimteam, iconname.c_str(), relative, 0);

	return 1;*/
	return 1;
}

int MsgFunc_MapObject(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ(pbuf, iSize);

	int type = READ_BYTE();
	if(type == MAP_NOBUILDABLE)
	{
		vec3_t mins, maxs;
		char modelname[32];
		int team;

		team = READ_BYTE();
		mins[0] = READ_COORD();
		mins[1] = READ_COORD();
		mins[2] = READ_COORD();
		maxs[0] = READ_COORD();
		maxs[1] = READ_COORD();
		maxs[2] = READ_COORD();
		strcpy(modelname, READ_STRING());

		model_t *mod = IEngineStudio.Mod_ForName(modelname, true);

		physent_t *physent = &g_NoBuildZones[g_NoBuildZones.AddToTail()];
		memset(physent, 0, sizeof(physent_t));

		VectorCopy(maxs, physent->maxs);
		VectorCopy(mins, physent->mins);
		VectorAdd(physent->maxs, physent->mins, physent->origin);
		VectorMultiply(physent->origin, 0.5, physent->origin);
		physent->team = team;
		physent->model = mod;
		physent->solid = SOLID_TRIGGER;
		physent->movetype = MOVETYPE_NONE;
		physent->skin = CONTENT_SOLID;
	}
	return 1;
}

int MF_DrawFX_StickyTrail(void)
{
	READ_ENTITY();
	R_StickyTrail(pEntity);
	return 1;
}

int MF_DrawFX_RocketTrail(void)
{
	READ_ENTITY();
	R_RocketTrail(pEntity);
	return 1;
}

int MF_DrawFX_GrenadeTrail(void)
{
	READ_ENTITY();
	R_GrenadeTrail(pEntity);
	return 1;
}

int MF_DrawFX_KillTrail(void)
{
	READ_ENTITY();
	R_KillPartSystem(pEntity, 0);
	return 1;
}

int MF_DrawFX_KillTrailInst(void)
{
	READ_ENTITY();
	R_KillPartSystem(pEntity, 1);
	return 1;
}

int MF_DrawFX_BottleBroken(void)
{
	READ_ENTITY();
	CL_BottleBroken(pEntity);
	return 1;
}

int MF_DrawFX_MultiJumpTrail(void)
{
	READ_ENTITY();
	R_MultiJumpTrail(pEntity);
	return 1;
}

int MF_DrawFX_ExplosionMidAir(void)
{
	vec3_t vecSrc;
	vecSrc[0] = READ_COORD();
	vecSrc[1] = READ_COORD();
	vecSrc[2] = READ_COORD();
	R_ExplosionMidAir(vecSrc);
	return 1;
}

int MF_DrawFX_ExplosionWall(void)
{
	vec3_t vecSrc;
	vec3_t vecNormal;
	vecSrc[0] = READ_COORD();
	vecSrc[1] = READ_COORD();
	vecSrc[2] = READ_COORD();
	vecNormal[0] = READ_COORD() / 1000.0;
	vecNormal[1] = READ_COORD() / 1000.0;
	vecNormal[2] = READ_COORD() / 1000.0;
	R_ExplosionWall(vecSrc, vecNormal);
	return 1;
}

int MF_DrawFX_StickyKill(void)
{
	vec3_t vecSrc;
	vecSrc[0] = READ_COORD();
	vecSrc[1] = READ_COORD();
	vecSrc[2] = READ_COORD();
	int iSkin = READ_BYTE();
	R_StickyKill(vecSrc, iSkin);
	return 1;
}

int MF_DrawFX_CoordTracer(void)
{
	vec3_t vecSrc, vecDst;
	vecSrc[0] = READ_COORD();
	vecSrc[1] = READ_COORD();
	vecSrc[2] = READ_COORD();
	vecDst[0] = READ_COORD();
	vecDst[1] = READ_COORD();
	vecDst[2] = READ_COORD();
	int col = READ_BYTE();
	R_BeginCoordTracer(col, 1, 0.0f, 5000, vecSrc);
	R_EmitTracer(vecDst);
	return 1;
}

int MF_DrawFX_AttachTracer(void)
{
	vec3_t vecDst;

	READ_ENTITY();

	int attachment = READ_BYTE();
	if(attachment < 0 || attachment > 3)
		return 1;

	vecDst[0] = READ_COORD();
	vecDst[1] = READ_COORD();
	vecDst[2] = READ_COORD();
	int col = READ_BYTE();
	R_BeginEntityTracer(col, 1, 0.0f, 5000, entindex, attachment);
	R_EmitTracer(vecDst);
	return 1;
}

int MF_DrawFX_MediBeam(void)
{
	READ_ENTITY();
	READ_TARGETENT();
	int iColor = READ_BYTE();
	R_MediBeam(pEntity, pTarget, iColor);
	return 1;
}

int MF_DrawFX_KillMediBeam(void)
{
	READ_ENTITY();
	READ_TARGETENT();
	R_KillMediBeam(pEntity, pTarget);
	return 1;
}

int MF_DrawFX_BluePrint(void)
{
	int bp = READ_BYTE();
	CL_BluePrint(bp);
	return 1;
}

int MF_DrawFX_DisguiseSmoke(void)
{
	READ_ENTITY();
	if(entindex == gEngfuncs.GetLocalPlayer()->index && !CL_IsThirdPerson())
		return 1;
	int iTeam = READ_BYTE();
	R_DisguiseSmoke(pEntity, iTeam);
	return 1;
}

int MF_DrawFX_SpyWatch(void)
{
	int action = READ_BYTE();
	CL_SpyWatch(action);
	return 1;
}

int MF_DrawFX_FlameThrow(void)
{
	READ_ENTITY();
	int iTeam = READ_BYTE();
	R_FlameThrow(pEntity, iTeam);
	return 1;
}

int MF_DrawFX_CritHit(void)
{
	READ_ENTITY();

	vec3_t vecSrc;

	VectorCopy(pEntity->origin, vecSrc);
	vecSrc[2] += 32;

	int bPackedInt = READ_BYTE();
	int bShowEffect = (bPackedInt & 1);
	int bShowSound = (bPackedInt >> 1) & 1;

	if(bShowEffect)
		R_CritText(vecSrc);

	char sound[64];

	if(EV_IsLocal(entindex))
	{
		sprintf(sound, "CKF_III/crit_received%d.wav", RANDOM_LONG(1, 3));
		gEngfuncs.pfnPlaySoundByName(sound, 1);
	}

	if(bShowSound)
	{
		sprintf(sound, "CKF_III/crit_hit%d.wav", RANDOM_LONG(1, 5));
		gEngfuncs.pEventAPI->EV_PlaySound(refparams.viewentity, vecSrc, CHAN_STATIC, sound, 1, 1, 0, 100);
	}

	return 1;
}

int MF_DrawFX_MiniCritHit(void)
{
	READ_ENTITY();

	vec3_t vecSrc;

	VectorCopy(pEntity->origin, vecSrc);
	vecSrc[2] += 32;

	int bPackedInt = READ_BYTE();
	int bShowEffect = (bPackedInt & 1);
	int bShowSound = (bPackedInt >> 1) & 1;

	if(bShowEffect)
		R_MiniCritText(vecSrc);

	char sound[64];

	if(bShowSound)
	{
		sprintf(sound, "CKF_III/crit_hit_mini%d.wav", RANDOM_LONG(1, 5));
		gEngfuncs.pEventAPI->EV_PlaySound(refparams.viewentity, vecSrc, CHAN_STATIC, sound, 1, 1, 0, 100);
	}

	return 1;
}

int MF_DrawFX_HitDamage(void)
{
	READ_ENTITY();

	if(!g_Player.m_bHitDamage)
		return 1;

	int damage = READ_SHORT();
	vec3_t vecSrc;

	VectorCopy(pEntity->origin, vecSrc);
	vecSrc[2] += 32;

	int crit = READ_BYTE();

	R_HitDamageText(damage, vecSrc, crit);

	gEngfuncs.pfnPlaySoundByName("CKF_III/hitsound.wav", 1);

	return 1;
}

int MF_DrawFX_AddHealth(void)
{
	HudFloatText_AddHealth(READ_SHORT());

	return 1;
}

int MF_DrawFX_AddTime(void)
{
	//HudFloatText_AddTime(READ_SHORT());

	return 1;
}

int MF_DrawFX_AddMetal(void)
{
	HudFloatText_AddMetal(READ_SHORT());

	return 1;
}

int MF_DrawFX_BurningPlayer(void)
{
	READ_ENTITY();

	int iTeam = READ_BYTE();
	float flTime = READ_COORD();

	R_BurningPlayer(pEntity, iTeam, flTime);

	return 1;
}

int MF_DrawFX_CloakBegin(void)
{
	CL_SpyWatch(CLOAK_BEGIN);

	return 1;
}

int MF_DrawFX_CloakStop(void)
{
	CL_SpyWatch(CLOAK_STOP);

	return 1;
}

int MF_DrawFX_DisguiseHint(void)
{
	return 0;
}

int MF_DrawFX_KillAllTrail(void)
{
	R_KillAllEntityPartSystem(true);

	return 1;
}

int MF_DrawFX_CritPlayerWeapon(void)
{
	READ_ENTITY();
	int iTeam = READ_BYTE();
	float flDuration = READ_COORD();

	R_CritPlayerWeapon(pEntity, iTeam, flDuration);

	return 1;
}

int MF_DrawFX_AirBlast(void)
{
	READ_ENTITY();

	R_AirBlast(pEntity);

	return 1;
}

pfnMF_DrawFX gDrawFXList[] = {
	MF_DrawFX_StickyTrail,
	MF_DrawFX_RocketTrail,
	MF_DrawFX_GrenadeTrail,
	MF_DrawFX_KillTrail,
	MF_DrawFX_KillTrailInst,
	MF_DrawFX_BottleBroken,
	MF_DrawFX_MultiJumpTrail,
	MF_DrawFX_ExplosionMidAir,
	MF_DrawFX_ExplosionWall,
	MF_DrawFX_StickyKill,
	MF_DrawFX_CoordTracer,
	MF_DrawFX_AttachTracer,
	MF_DrawFX_MediBeam,
	MF_DrawFX_KillMediBeam,
	MF_DrawFX_BluePrint,
	MF_DrawFX_DisguiseSmoke,
	MF_DrawFX_SpyWatch,
	MF_DrawFX_FlameThrow,
	MF_DrawFX_CritHit,
	MF_DrawFX_MiniCritHit,
	MF_DrawFX_HitDamage,
	MF_DrawFX_AddHealth,
	MF_DrawFX_AddTime,
	MF_DrawFX_AddMetal,
	MF_DrawFX_BurningPlayer,
	MF_DrawFX_CloakBegin,
	MF_DrawFX_CloakStop,
	MF_DrawFX_DisguiseHint,
	MF_DrawFX_KillAllTrail,
	MF_DrawFX_CritPlayerWeapon,
	NULL,
	MF_DrawFX_AirBlast
};

int MsgFunc_DrawFX(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ(pbuf, iSize);

	if(pfnMsgFunc_DrawFX && pfnMsgFunc_DrawFX(pszName, iSize, pbuf))
	{
		return 1;
	}

	int fxtype = READ_BYTE();
	if(fxtype >= 0 && fxtype < ARRAYSIZE(gDrawFXList) )
	{
		if(gDrawFXList[fxtype] != NULL)
			return (gDrawFXList[fxtype])();
	}

	return 1;
}

//int MsgFunc_MGUIPrint(const char *pszName, int iSize, void *pbuf)
//{
//	BEGIN_READ(pbuf, iSize);
//	int buftype = READ_BYTE();
//	int part = READ_BYTE();
//	char *str = READ_STRING();
//	if(part == MESSAGEPART_START)
//	{
//		if(buftype == MENUBUF_MOTD)
//		{
//			Motd_ClearBuf();
//		}
//		else if(buftype == MENUBUF_MAPINFO)
//		{
//			MapInfo_ClearBuf();
//		}
//		else if(buftype == MENUBUF_CLASSINTRO)
//		{
//			ClassIntro_ClearBuf();
//		}
//	}
//	if(buftype == MENUBUF_MOTD)
//	{
//		Motd_AddToBuf(str);
//	}
//	else if(buftype == MENUBUF_MAPINFO)
//	{
//		MapInfo_AddToBuf(str);
//	}
//	else if(buftype == MENUBUF_CLASSINTRO)
//	{
//		ClassIntro_AddToBuf(str);
//	}
//	if(part == MESSAGEPART_END)
//	{
//		if(buftype == MENUBUF_MOTD)
//		{
//			Motd_ParseBuf();
//		}
//		else if(buftype == MENUBUF_MAPINFO)
//		{
//			MapInfo_ParseBuf();
//		}
//		else if(buftype == MENUBUF_CLASSINTRO)
//		{
//			ClassIntro_ParseBuf();
//		}
//	}
//	return 1;
//}

int MsgFunc_WeaponAnimEx(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ(pbuf, iSize);

	int anim = READ_BYTE();
	int body = READ_BYTE();
	int skin = READ_BYTE();

	if(anim != 0xFF)
	{
		*cls_viewmodel_sequence = anim;
		*cls_viewmodel_starttime = 0;
	}
	if(body)
		g_iViewModelBody = body;
	if(skin)
		g_iViewModelSkin = skin;
	return 1;
}

int MsgFunc_VGUIMenu(const char *pszName, int iSize, void *pbuf)
{
	//BEGIN_READ(pbuf, iSize);
	//*(byte *)pbuf = 2;
	//return pfnMsgFunc_VGUIMenu(pszName, iSize, pbuf);
	return 1;
}

int MsgFunc_HudMenu(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ(pbuf, iSize);
	int iHudMenu = READ_BYTE();
	int iHudMenuKeys = READ_SHORT();

	ShowHudMenu(iHudMenu, iHudMenuKeys);
	return 1;
}

//int MsgFunc_MGUIMenu(const char *pszName, int iSize, void *pbuf)
//{
//	BEGIN_READ(pbuf, iSize);
//	g_iMenu = READ_BYTE();
//	g_iMenuKeys = READ_SHORT();
//	if(g_iMenu == MENU_CHOOSETEAM)
//	{
//		HudBase_DeactivateMouse();
//	}
//	if(g_iMenu == MENU_CHOOSECLASS)
//	{
//		HudBase_DeactivateMouse();
//	}
//	if(g_iMenu == MENU_INTRO)
//	{
//		HudIntroMenu_SetPage(0);
//		HudBase_DeactivateMouse();
//	}
//	if(g_iMenu == MENU_CLOSE)
//	{
//		HudBase_ActivateMouse();
//	}
//	return 1;
//}

int MsgFunc_WeaponInfo(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ(pbuf, iSize);
	int weaponid = READ_BYTE();
	g_WeaponInfo[weaponid].iSlot = READ_BYTE();
	g_WeaponInfo[weaponid].iMaxClip = READ_BYTE();
	g_WeaponInfo[weaponid].iMaxAmmo = READ_BYTE();
	g_WeaponInfo[weaponid].iBody = READ_BYTE();
	g_WeaponInfo[weaponid].iSkin = READ_BYTE();	
	return 1;
}

int MsgFunc_Health(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ(pbuf, iSize);
	g_iHealth = READ_SHORT();

	*(byte *)pbuf = (g_iHealth > 0) ? 100 : 0;
	return pfnMsgFunc_Health(pszName, 1, pbuf);
}

int MsgFunc_InitHUD(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ(pbuf, iSize);

	CL_InitVars();
	//MapInfo_LoadFromFile();

	return pfnMsgFunc_InitHUD(pszName, 0, pbuf);
}

int MsgFunc_ResetHUD(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ(pbuf, iSize);

	g_Player.m_pLastItem = NULL;
	g_Player.m_pActiveItem = NULL;
	g_Player.m_iHealth = 0;
	g_Player.m_iClass = 0;

	return pfnMsgFunc_ResetHUD(pszName, 0, pbuf);
}

int MsgFunc_TeamInfo(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ(pbuf, iSize);

	int entindex = READ_BYTE();
	char *teamname = READ_STRING();

	g_PlayerInfo[entindex].iTeam = 0;
	if(!strcmp(teamname, "TERRORIST"))
		g_PlayerInfo[entindex].iTeam = 1;
	else if(!strcmp(teamname, "CT"))
		g_PlayerInfo[entindex].iTeam = 2;
	else if(!strcmp(teamname, "SPECTATOR"))
		g_PlayerInfo[entindex].iTeam = 3;

	if(entindex == gEngfuncs.GetLocalPlayer()->index)
	{
		g_iTeam = g_PlayerInfo[entindex].iTeam;
	}

	return pfnMsgFunc_TeamInfo(pszName, iSize, pbuf);
}

int MsgFunc_HUDBuild(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ(pbuf, iSize);
	int buildtype = READ_BYTE();
	if(buildtype == HUDBUILD_RESET)
	{
		g_Build.sentry.level = 0;
		g_Build.dispenser.level = 0;
		g_Build.telein.level = 0;
		g_Build.teleout.level = 0;
	}
	if(buildtype == BUILDABLE_SENTRY)
	{
		g_Build.sentry.level = READ_BYTE();
		if(g_Build.sentry.level == 0)
			return 1;
		g_Build.sentry.flags = READ_BYTE();
		g_Build.sentry.health = (float)READ_BYTE();
		g_Build.sentry.maxhealth = (float)READ_BYTE();
		g_Build.sentry.ammo = READ_BYTE();
		g_Build.sentry.maxammo = READ_BYTE();
		g_Build.sentry.killcount = READ_BYTE();
		if(g_Build.sentry.flags & BUILD_BUILDING)
		{
			g_Build.sentry.progress = (float)READ_BYTE() * 100.0 / 255;
		}
		if(g_Build.sentry.level < 3)
		{
			g_Build.sentry.upgrade = READ_BYTE();
		}
		else
		{
			g_Build.sentry.rocket = READ_BYTE();
			g_Build.sentry.maxrocket = READ_BYTE();
		}
		g_Build.sentry.updatetime = gEngfuncs.GetClientTime();
	}
	else if(buildtype == BUILDABLE_DISPENSER)
	{
		g_Build.dispenser.level = READ_BYTE();
		if(g_Build.dispenser.level == 0)
			return 1;
		g_Build.dispenser.flags = READ_BYTE();
		g_Build.dispenser.health = (float)READ_BYTE();
		g_Build.dispenser.maxhealth = (float)READ_BYTE();
		g_Build.dispenser.metal = READ_BYTE();
		g_Build.dispenser.maxmetal = READ_BYTE();
		if(g_Build.dispenser.flags  & BUILD_BUILDING)
			g_Build.dispenser.progress = (float)READ_BYTE() * 100.0 / 255;
		if(g_Build.dispenser.level < 3)
			g_Build.dispenser.upgrade = READ_BYTE();
		g_Build.dispenser.updatetime = gEngfuncs.GetClientTime();
	}
	else if(buildtype == BUILDABLE_ENTRANCE)
	{
		g_Build.telein.level = READ_BYTE();
		if(g_Build.telein.level == 0)
			return 1;
		g_Build.telein.flags = READ_BYTE();
		g_Build.telein.health = (float)READ_BYTE();
		g_Build.telein.maxhealth = (float)READ_BYTE(); 
		if(g_Build.telein.flags & BUILD_BUILDING)
		{
			g_Build.telein.progress = (float)READ_BYTE() * 100.0 / 255;
		}
		g_Build.telein.charge = READ_COORD();
		g_Build.telein.chargerate = READ_COORD();
		g_Build.telein.frags = READ_BYTE();
		if(g_Build.telein.level<3)
			g_Build.telein.upgrade = READ_BYTE();
		g_Build.telein.chargetime = g_Build.telein.updatetime = gEngfuncs.GetClientTime();
	}
	else if(buildtype == BUILDABLE_EXIT)
	{
		g_Build.teleout.level = READ_BYTE();
		if(g_Build.teleout.level == 0)
			return 1;
		g_Build.teleout.flags = READ_BYTE();
		g_Build.teleout.health = (float)READ_BYTE();
		g_Build.teleout.maxhealth = (float)READ_BYTE();
		if(g_Build.teleout.flags & BUILD_BUILDING)
		{
			g_Build.teleout.progress = (float)READ_BYTE() * 100.0 / 255;
		}
		if(g_Build.teleout.level<3)
			g_Build.teleout.upgrade = READ_BYTE();
		g_Build.teleout.updatetime = gEngfuncs.GetClientTime();
	}
	return 1;
}

int MsgFunc_StatsInfo(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ(pbuf, iSize);

	int iType = READ_BYTE();

	switch(iType)
	{
	case STATS_DEMOLISH:
		g_PlayerStats.iDemolish = READ_SHORT();
		break;
	case STATS_CAPTURE:
		g_PlayerStats.iCapture = READ_SHORT();
		break;
	case STATS_DEFENCE:
		g_PlayerStats.iDefence = READ_SHORT();
		break;
	case STATS_DOMINATE:
		g_PlayerStats.iDominate = READ_SHORT();
		break;
	case STATS_REVENGE:
		g_PlayerStats.iRevenge = READ_SHORT();
		break;
	case STATS_UBERCHARGE:
		g_PlayerStats.iUbercharge = READ_SHORT();
		break;
	case STATS_HEADSHOT:
		g_PlayerStats.iHeadshot = READ_SHORT();
		break;
	case STATS_TELEPORT:
		g_PlayerStats.iTeleport = READ_SHORT();
		break;
	case STATS_HEALING:
		g_PlayerStats.iHealing = READ_SHORT();
		break;
	case STATS_BACKSTAB:
		g_PlayerStats.iBackstab = READ_SHORT();
		break;
	case STATS_BONUS:
		g_PlayerStats.iBonus = READ_SHORT();
		break;
	case STATS_KILL:
		g_PlayerStats.iKill = READ_SHORT();
		break;
	case STATS_DEATH:
		g_PlayerStats.iDeath = READ_SHORT();
		break;
	case STATS_ASSIST:
		g_PlayerStats.iAssist = READ_SHORT();
		break;
	case STATS_CLEAR:
		memset(&g_PlayerStats, 0, sizeof(g_PlayerStats));
		break;
	}
	return 1;
}

int MsgFunc_TeamScore(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ(pbuf, iSize);

	char *teamname = READ_STRING();

	if(!strcmp(teamname, "TERRORIST"))
		g_iRedTeamScore = READ_SHORT();
	else if(!strcmp(teamname, "CT"))
		g_iBlueTeamScore = READ_SHORT();

	return pfnMsgFunc_TeamScore(pszName, iSize, pbuf);
}

int MsgFunc_ScoreAttrib(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ(pbuf, iSize);

	int entindex = READ_BYTE();

	int flags = READ_BYTE();

	g_PlayerInfo[entindex].bIsDead = (flags & SCOREATTRIB_DEAD) ? true : false;
	g_PlayerInfo[entindex].iClass = READ_BYTE();

	return 1;
}

int MsgFunc_ScoreInfo(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ(pbuf, iSize);
	int entindex = READ_BYTE();
	g_PlayerInfo[entindex].iFrags = READ_SHORT();
	g_PlayerInfo[entindex].iDeaths = READ_SHORT();
	g_PlayerInfo[entindex].iDominates = READ_BYTE();
	return 1;
}

int MsgFunc_ServerName(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ(pbuf, iSize);
	strncpy(g_szServerName, READ_STRING(), 63);
	g_szServerName[63] = 0;

	return pfnMsgFunc_ServerName(pszName, iSize, pbuf);
}

int MsgFunc_RoundTime(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ(pbuf, iSize);
	
	int iStatus = READ_BYTE();
	if(iStatus != g_iRoundStatus)
	{
		g_flRoundStatusChangeTime = g_flClientTime;
		g_iLastRoundStatus = g_iRoundStatus;
		g_iRoundStatus = iStatus;
	}

	int iRoundTime = READ_SHORT();
	g_iMaxRoundTime = READ_SHORT();

	g_flRoundEndTime = g_flClientTime + iRoundTime;

	return 1;
}

int MsgFunc_CPState(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ(pbuf, iSize);

	int iIndex = READ_BYTE();

	if(iIndex >= 0 && iIndex <= g_ControlPoints.Count())
	{
		g_ControlPoints[iIndex].bLocked = READ_BYTE();
		g_ControlPoints[iIndex].bDisabled = READ_BYTE();
		g_ControlPoints[iIndex].iState = READ_BYTE();
		int iTeam = READ_BYTE();
		g_ControlPoints[iIndex].iCapTeam = (iTeam & 3);
		g_ControlPoints[iIndex].iTeam = (iTeam >> 2) & 3;
		g_ControlPoints[iIndex].iCapPlayers = READ_BYTE();
		g_ControlPoints[iIndex].flProgress = READ_COORD();
		g_ControlPoints[iIndex].flCapRate = READ_COORD();
		g_ControlPoints[iIndex].flMessageTime = g_flClientTime;
	}

	return 1;
}

int MsgFunc_CPInit(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ(pbuf, iSize);

	int iNumPoints = READ_BYTE();

	g_ControlPoints.RemoveAll();
	g_ControlPoints.AddMultipleToTail(iNumPoints);

	for(int i = 0; i < iNumPoints; ++i)
	{
		g_ControlPoints[i].iHudPosition = READ_CHAR();
		strncpy(g_ControlPoints[i].szName, READ_STRING(), 31);
		g_ControlPoints[i].szName[31] = '\0';

		char modelname[32];
		strncpy(modelname, READ_STRING(), 31);
		modelname[31] = '\0';

		model_t *mod = IEngineStudio.Mod_ForName(modelname, true);

		physent_t *physent = &g_ControlPoints[i].physent;
		memset(physent, 0, sizeof(physent_t));

		VectorCopy(mod->maxs, physent->maxs);
		VectorCopy(mod->mins, physent->mins);
		VectorAdd(physent->maxs, physent->mins, physent->origin);
		VectorMultiply(physent->origin, 0.5, physent->origin);
		physent->model = mod;
		physent->solid = SOLID_TRIGGER;
		physent->movetype = MOVETYPE_NONE;
		physent->skin = CONTENT_SOLID;
	}

	return 1;
}

int MsgFunc_CPZone(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ(pbuf, iSize);

	g_iCapPointIndex = READ_BYTE();

	return 1;
}

int MsgFunc_RTInit(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ(pbuf, iSize);

	int iNumTimers = READ_BYTE();

	g_RoundTimers.RemoveAll();
	g_RoundTimers.AddMultipleToTail(iNumTimers);

	for (int i = 0; i < iNumTimers; ++i)
	{
		g_RoundTimers[i].iTeam = READ_CHAR();
		g_RoundTimers[i].iHudPosition = READ_CHAR();
	}

	return 1;
}

int MsgFunc_RTState(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ(pbuf, iSize);

	int iIndex = READ_BYTE();

	if (iIndex >= 0 && iIndex <= g_RoundTimers.Count())
	{
		BOOL bLocked = READ_BYTE();
		BOOL bDisabled = READ_BYTE();
		BOOL bOvertime = READ_BYTE();
		float flTime = READ_SHORT();
		float flTotalTime = READ_SHORT();

		g_RoundTimers[iIndex].bLocked = bLocked;
		g_RoundTimers[iIndex].bDisabled = bDisabled;
		g_RoundTimers[iIndex].bOvertime = bOvertime;
		g_RoundTimers[iIndex].flTime = flTime;
		g_RoundTimers[iIndex].flEndTime = bLocked || bDisabled ? 0.0f : g_flClientTime + flTime;
		g_RoundTimers[iIndex].flTotalTime = flTotalTime;
	}

	return 1;
}

int MsgFunc_PlayerVars(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ(pbuf, iSize);

	int iVarType = READ_BYTE();
	switch(iVarType)
	{
	case PV_iDesiredClass:
		g_iDesiredClass = READ_BYTE();
		break;
	case PV_iLimitTeams:
		g_iLimitTeams = READ_BYTE();
	default:break;
	}
	return 1;
}

int MsgFunc_SpawnInit(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ(pbuf, iSize);

	g_iMaxHealth = READ_SHORT();
	g_iClass = READ_BYTE();

	//clear all sounds
	gEngfuncs.pEventAPI->EV_StopAllSounds(gEngfuncs.GetLocalPlayer()->index, CHAN_WEAPON);
	gEngfuncs.pEventAPI->EV_StopAllSounds(gEngfuncs.GetLocalPlayer()->index, CHAN_ITEM);

	//uncloak me instantly
	CL_SpyWatch(CLOAK_NO);

	//remove the blueprint
	CL_BluePrint(0);

	R_KillPartSystem(gEngfuncs.GetLocalPlayer(), true);
	
	return 1;
}

void UserMsg_InstallHook(void)
{
#define HOOK_USERMSG(x) g_pBTEClient->HookUserMsg(#x, MsgFunc_##x);

	pfnMsgFunc_Health = HOOK_USERMSG(Health);
	pfnMsgFunc_ResetHUD = HOOK_USERMSG(ResetHUD);
	pfnMsgFunc_InitHUD = HOOK_USERMSG(InitHUD);
	pfnMsgFunc_TeamInfo = HOOK_USERMSG(TeamInfo);
	pfnMsgFunc_TeamScore = HOOK_USERMSG(TeamScore);
	//HOOK_USERMSG(DeathMsg);
	//pfnMsgFunc_VGUIMenu = HOOK_USERMSG(VGUIMenu);
	pfnMsgFunc_HideWeapon = HOOK_USERMSG(HideWeapon);
	HOOK_USERMSG(ScoreAttrib);
	HOOK_USERMSG(ScoreInfo);
	pfnMsgFunc_ServerName = HOOK_USERMSG(ServerName);
	HOOK_USERMSG(RoundTime);

	HOOK_USERMSG(WeaponInfo);
	HOOK_USERMSG(HudMenu);
	//HOOK_USERMSG(MGUIPrint);
	//HOOK_USERMSG(MGUIMenu);
	pfnMsgFunc_DrawFX = HOOK_USERMSG(DrawFX);
	HOOK_USERMSG(WeaponAnimEx);
	HOOK_USERMSG(MapObject);
	//HOOK_USERMSG(BuildDeath);
	HOOK_USERMSG(HUDBuild);
	//HOOK_USERMSG(ObjectMsg);
	HOOK_USERMSG(StatsInfo);
	HOOK_USERMSG(CPState);
	HOOK_USERMSG(CPInit);
	HOOK_USERMSG(RTInit);
	HOOK_USERMSG(RTState);
	//HOOK_USERMSG(CPZone);
	HOOK_USERMSG(SpawnInit);
	HOOK_USERMSG(PlayerVars);
	//HOOK_USERMSG(Dominate);
	//HOOK_USERMSG(Revenge);
}